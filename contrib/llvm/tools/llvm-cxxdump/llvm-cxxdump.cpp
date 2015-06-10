//===- llvm-cxxdump.cpp - Dump C++ data in an Object File -------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// Dumps C++ data resident in object files and archives.
//
//===----------------------------------------------------------------------===//

#include "llvm-cxxdump.h"
#include "Error.h"
#include "llvm/ADT/ArrayRef.h"
#include "llvm/Object/Archive.h"
#include "llvm/Object/ObjectFile.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/Endian.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/ManagedStatic.h"
#include "llvm/Support/PrettyStackTrace.h"
#include "llvm/Support/Signals.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/raw_ostream.h"
#include <map>
#include <string>
#include <system_error>

using namespace llvm;
using namespace llvm::object;
using namespace llvm::support;

namespace opts {
cl::list<std::string> InputFilenames(cl::Positional,
                                     cl::desc("<input object files>"),
                                     cl::ZeroOrMore);
} // namespace opts

static int ReturnValue = EXIT_SUCCESS;

namespace llvm {

static bool error(std::error_code EC) {
  if (!EC)
    return false;

  ReturnValue = EXIT_FAILURE;
  outs() << "\nError reading file: " << EC.message() << ".\n";
  outs().flush();
  return true;
}

} // namespace llvm

static void reportError(StringRef Input, StringRef Message) {
  if (Input == "-")
    Input = "<stdin>";

  errs() << Input << ": " << Message << "\n";
  errs().flush();
  ReturnValue = EXIT_FAILURE;
}

static void reportError(StringRef Input, std::error_code EC) {
  reportError(Input, EC.message());
}

static SmallVectorImpl<SectionRef> &getRelocSections(const ObjectFile *Obj,
                                                     const SectionRef &Sec) {
  static bool MappingDone = false;
  static std::map<SectionRef, SmallVector<SectionRef, 1>> SectionRelocMap;
  if (!MappingDone) {
    for (const SectionRef &Section : Obj->sections()) {
      section_iterator Sec2 = Section.getRelocatedSection();
      if (Sec2 != Obj->section_end())
        SectionRelocMap[*Sec2].push_back(Section);
    }
    MappingDone = true;
  }
  return SectionRelocMap[Sec];
}

static bool collectRelocatedSymbols(const ObjectFile *Obj,
                                    const SectionRef &Sec, uint64_t SecAddress,
                                    uint64_t SymAddress, uint64_t SymSize,
                                    StringRef *I, StringRef *E) {
  uint64_t SymOffset = SymAddress - SecAddress;
  uint64_t SymEnd = SymOffset + SymSize;
  for (const SectionRef &SR : getRelocSections(Obj, Sec)) {
    for (const object::RelocationRef &Reloc : SR.relocations()) {
      if (I == E)
        break;
      const object::symbol_iterator RelocSymI = Reloc.getSymbol();
      if (RelocSymI == Obj->symbol_end())
        continue;
      StringRef RelocSymName;
      if (error(RelocSymI->getName(RelocSymName)))
        return true;
      uint64_t Offset;
      if (error(Reloc.getOffset(Offset)))
        return true;
      if (Offset >= SymOffset && Offset < SymEnd) {
        *I = RelocSymName;
        ++I;
      }
    }
  }
  return false;
}

static bool collectRelocationOffsets(
    const ObjectFile *Obj, const SectionRef &Sec, uint64_t SecAddress,
    uint64_t SymAddress, uint64_t SymSize, StringRef SymName,
    std::map<std::pair<StringRef, uint64_t>, StringRef> &Collection) {
  uint64_t SymOffset = SymAddress - SecAddress;
  uint64_t SymEnd = SymOffset + SymSize;
  for (const SectionRef &SR : getRelocSections(Obj, Sec)) {
    for (const object::RelocationRef &Reloc : SR.relocations()) {
      const object::symbol_iterator RelocSymI = Reloc.getSymbol();
      if (RelocSymI == Obj->symbol_end())
        continue;
      StringRef RelocSymName;
      if (error(RelocSymI->getName(RelocSymName)))
        return true;
      uint64_t Offset;
      if (error(Reloc.getOffset(Offset)))
        return true;
      if (Offset >= SymOffset && Offset < SymEnd)
        Collection[std::make_pair(SymName, Offset - SymOffset)] = RelocSymName;
    }
  }
  return false;
}

static void dumpCXXData(const ObjectFile *Obj) {
  struct CompleteObjectLocator {
    StringRef Symbols[2];
    ArrayRef<little32_t> Data;
  };
  struct ClassHierarchyDescriptor {
    StringRef Symbols[1];
    ArrayRef<little32_t> Data;
  };
  struct BaseClassDescriptor {
    StringRef Symbols[2];
    ArrayRef<little32_t> Data;
  };
  struct TypeDescriptor {
    StringRef Symbols[1];
    uint64_t AlwaysZero;
    StringRef MangledName;
  };
  struct ThrowInfo {
    uint32_t Flags;
  };
  struct CatchableTypeArray {
    uint32_t NumEntries;
  };
  struct CatchableType {
    uint32_t Flags;
    uint32_t NonVirtualBaseAdjustmentOffset;
    int32_t VirtualBasePointerOffset;
    uint32_t VirtualBaseAdjustmentOffset;
    uint32_t Size;
    StringRef Symbols[2];
  };
  std::map<std::pair<StringRef, uint64_t>, StringRef> VFTableEntries;
  std::map<std::pair<StringRef, uint64_t>, StringRef> TIEntries;
  std::map<std::pair<StringRef, uint64_t>, StringRef> CTAEntries;
  std::map<StringRef, ArrayRef<little32_t>> VBTables;
  std::map<StringRef, CompleteObjectLocator> COLs;
  std::map<StringRef, ClassHierarchyDescriptor> CHDs;
  std::map<std::pair<StringRef, uint64_t>, StringRef> BCAEntries;
  std::map<StringRef, BaseClassDescriptor> BCDs;
  std::map<StringRef, TypeDescriptor> TDs;
  std::map<StringRef, ThrowInfo> TIs;
  std::map<StringRef, CatchableTypeArray> CTAs;
  std::map<StringRef, CatchableType> CTs;

  std::map<std::pair<StringRef, uint64_t>, StringRef> VTableSymEntries;
  std::map<std::pair<StringRef, uint64_t>, int64_t> VTableDataEntries;
  std::map<std::pair<StringRef, uint64_t>, StringRef> VTTEntries;
  std::map<StringRef, StringRef> TINames;

  uint8_t BytesInAddress = Obj->getBytesInAddress();

  for (const object::SymbolRef &Sym : Obj->symbols()) {
    StringRef SymName;
    if (error(Sym.getName(SymName)))
      return;
    object::section_iterator SecI(Obj->section_begin());
    if (error(Sym.getSection(SecI)))
      return;
    // Skip external symbols.
    if (SecI == Obj->section_end())
      continue;
    const SectionRef &Sec = *SecI;
    // Skip virtual or BSS sections.
    if (Sec.isBSS() || Sec.isVirtual())
      continue;
    StringRef SecContents;
    if (error(Sec.getContents(SecContents)))
      return;
    uint64_t SymAddress;
    if (error(Sym.getAddress(SymAddress)))
      return;
    uint64_t SymSize = Sym.getSize();
    uint64_t SecAddress = Sec.getAddress();
    uint64_t SecSize = Sec.getSize();
    uint64_t SymOffset = SymAddress - SecAddress;
    StringRef SymContents = SecContents.substr(SymOffset, SymSize);

    // VFTables in the MS-ABI start with '??_7' and are contained within their
    // own COMDAT section.  We then determine the contents of the VFTable by
    // looking at each relocation in the section.
    if (SymName.startswith("??_7")) {
      // Each relocation either names a virtual method or a thunk.  We note the
      // offset into the section and the symbol used for the relocation.
      collectRelocationOffsets(Obj, Sec, SecAddress, SecAddress, SecSize,
                               SymName, VFTableEntries);
    }
    // VBTables in the MS-ABI start with '??_8' and are filled with 32-bit
    // offsets of virtual bases.
    else if (SymName.startswith("??_8")) {
      ArrayRef<little32_t> VBTableData(
          reinterpret_cast<const little32_t *>(SymContents.data()),
          SymContents.size() / sizeof(little32_t));
      VBTables[SymName] = VBTableData;
    }
    // Complete object locators in the MS-ABI start with '??_R4'
    else if (SymName.startswith("??_R4")) {
      CompleteObjectLocator COL;
      COL.Data = ArrayRef<little32_t>(
          reinterpret_cast<const little32_t *>(SymContents.data()), 3);
      StringRef *I = std::begin(COL.Symbols), *E = std::end(COL.Symbols);
      if (collectRelocatedSymbols(Obj, Sec, SecAddress, SymAddress, SymSize, I,
                                  E))
        return;
      COLs[SymName] = COL;
    }
    // Class hierarchy descriptors in the MS-ABI start with '??_R3'
    else if (SymName.startswith("??_R3")) {
      ClassHierarchyDescriptor CHD;
      CHD.Data = ArrayRef<little32_t>(
          reinterpret_cast<const little32_t *>(SymContents.data()), 3);
      StringRef *I = std::begin(CHD.Symbols), *E = std::end(CHD.Symbols);
      if (collectRelocatedSymbols(Obj, Sec, SecAddress, SymAddress, SymSize, I,
                                  E))
        return;
      CHDs[SymName] = CHD;
    }
    // Class hierarchy descriptors in the MS-ABI start with '??_R2'
    else if (SymName.startswith("??_R2")) {
      // Each relocation names a base class descriptor.  We note the offset into
      // the section and the symbol used for the relocation.
      collectRelocationOffsets(Obj, Sec, SecAddress, SymAddress, SymSize,
                               SymName, BCAEntries);
    }
    // Base class descriptors in the MS-ABI start with '??_R1'
    else if (SymName.startswith("??_R1")) {
      BaseClassDescriptor BCD;
      BCD.Data = ArrayRef<little32_t>(
          reinterpret_cast<const little32_t *>(SymContents.data()) + 1, 5);
      StringRef *I = std::begin(BCD.Symbols), *E = std::end(BCD.Symbols);
      if (collectRelocatedSymbols(Obj, Sec, SecAddress, SymAddress, SymSize, I,
                                  E))
        return;
      BCDs[SymName] = BCD;
    }
    // Type descriptors in the MS-ABI start with '??_R0'
    else if (SymName.startswith("??_R0")) {
      const char *DataPtr = SymContents.drop_front(BytesInAddress).data();
      TypeDescriptor TD;
      if (BytesInAddress == 8)
        TD.AlwaysZero = *reinterpret_cast<const little64_t *>(DataPtr);
      else
        TD.AlwaysZero = *reinterpret_cast<const little32_t *>(DataPtr);
      TD.MangledName = SymContents.drop_front(BytesInAddress * 2);
      StringRef *I = std::begin(TD.Symbols), *E = std::end(TD.Symbols);
      if (collectRelocatedSymbols(Obj, Sec, SecAddress, SymAddress, SymSize, I,
                                  E))
        return;
      TDs[SymName] = TD;
    }
    // Throw descriptors in the MS-ABI start with '_TI'
    else if (SymName.startswith("_TI") || SymName.startswith("__TI")) {
      ThrowInfo TI;
      TI.Flags = *reinterpret_cast<const little32_t *>(SymContents.data());
      collectRelocationOffsets(Obj, Sec, SecAddress, SymAddress, SymSize,
                               SymName, TIEntries);
      TIs[SymName] = TI;
    }
    // Catchable type arrays in the MS-ABI start with _CTA or __CTA.
    else if (SymName.startswith("_CTA") || SymName.startswith("__CTA")) {
      CatchableTypeArray CTA;
      CTA.NumEntries =
          *reinterpret_cast<const little32_t *>(SymContents.data());
      collectRelocationOffsets(Obj, Sec, SecAddress, SymAddress, SymSize,
                               SymName, CTAEntries);
      CTAs[SymName] = CTA;
    }
    // Catchable types in the MS-ABI start with _CT or __CT.
    else if (SymName.startswith("_CT") || SymName.startswith("__CT")) {
      const little32_t *DataPtr =
          reinterpret_cast<const little32_t *>(SymContents.data());
      CatchableType CT;
      CT.Flags = DataPtr[0];
      CT.NonVirtualBaseAdjustmentOffset = DataPtr[2];
      CT.VirtualBasePointerOffset = DataPtr[3];
      CT.VirtualBaseAdjustmentOffset = DataPtr[4];
      CT.Size = DataPtr[5];
      StringRef *I = std::begin(CT.Symbols), *E = std::end(CT.Symbols);
      if (collectRelocatedSymbols(Obj, Sec, SecAddress, SymAddress, SymSize, I,
                                  E))
        return;
      CTs[SymName] = CT;
    }
    // Construction vtables in the Itanium ABI start with '_ZTT' or '__ZTT'.
    else if (SymName.startswith("_ZTT") || SymName.startswith("__ZTT")) {
      collectRelocationOffsets(Obj, Sec, SecAddress, SymAddress, SymSize,
                               SymName, VTTEntries);
    }
    // Typeinfo names in the Itanium ABI start with '_ZTS' or '__ZTS'.
    else if (SymName.startswith("_ZTS") || SymName.startswith("__ZTS")) {
      TINames[SymName] = SymContents.slice(0, SymContents.find('\0'));
    }
    // Vtables in the Itanium ABI start with '_ZTV' or '__ZTV'.
    else if (SymName.startswith("_ZTV") || SymName.startswith("__ZTV")) {
      collectRelocationOffsets(Obj, Sec, SecAddress, SymAddress, SymSize,
                               SymName, VTableSymEntries);
      for (uint64_t SymOffI = 0; SymOffI < SymSize; SymOffI += BytesInAddress) {
        auto Key = std::make_pair(SymName, SymOffI);
        if (VTableSymEntries.count(Key))
          continue;
        const char *DataPtr =
            SymContents.substr(SymOffI, BytesInAddress).data();
        int64_t VData;
        if (BytesInAddress == 8)
          VData = *reinterpret_cast<const little64_t *>(DataPtr);
        else
          VData = *reinterpret_cast<const little32_t *>(DataPtr);
        VTableDataEntries[Key] = VData;
      }
    }
    // Typeinfo structures in the Itanium ABI start with '_ZTI' or '__ZTI'.
    else if (SymName.startswith("_ZTI") || SymName.startswith("__ZTI")) {
      // FIXME: Do something with these!
    }
  }
  for (const auto &VFTableEntry : VFTableEntries) {
    StringRef VFTableName = VFTableEntry.first.first;
    uint64_t Offset = VFTableEntry.first.second;
    StringRef SymName = VFTableEntry.second;
    outs() << VFTableName << '[' << Offset << "]: " << SymName << '\n';
  }
  for (const auto &VBTable : VBTables) {
    StringRef VBTableName = VBTable.first;
    uint32_t Idx = 0;
    for (little32_t Offset : VBTable.second) {
      outs() << VBTableName << '[' << Idx << "]: " << Offset << '\n';
      Idx += sizeof(Offset);
    }
  }
  for (const auto &COLPair : COLs) {
    StringRef COLName = COLPair.first;
    const CompleteObjectLocator &COL = COLPair.second;
    outs() << COLName << "[IsImageRelative]: " << COL.Data[0] << '\n';
    outs() << COLName << "[OffsetToTop]: " << COL.Data[1] << '\n';
    outs() << COLName << "[VFPtrOffset]: " << COL.Data[2] << '\n';
    outs() << COLName << "[TypeDescriptor]: " << COL.Symbols[0] << '\n';
    outs() << COLName << "[ClassHierarchyDescriptor]: " << COL.Symbols[1]
           << '\n';
  }
  for (const auto &CHDPair : CHDs) {
    StringRef CHDName = CHDPair.first;
    const ClassHierarchyDescriptor &CHD = CHDPair.second;
    outs() << CHDName << "[AlwaysZero]: " << CHD.Data[0] << '\n';
    outs() << CHDName << "[Flags]: " << CHD.Data[1] << '\n';
    outs() << CHDName << "[NumClasses]: " << CHD.Data[2] << '\n';
    outs() << CHDName << "[BaseClassArray]: " << CHD.Symbols[0] << '\n';
  }
  for (const auto &BCAEntry : BCAEntries) {
    StringRef BCAName = BCAEntry.first.first;
    uint64_t Offset = BCAEntry.first.second;
    StringRef SymName = BCAEntry.second;
    outs() << BCAName << '[' << Offset << "]: " << SymName << '\n';
  }
  for (const auto &BCDPair : BCDs) {
    StringRef BCDName = BCDPair.first;
    const BaseClassDescriptor &BCD = BCDPair.second;
    outs() << BCDName << "[TypeDescriptor]: " << BCD.Symbols[0] << '\n';
    outs() << BCDName << "[NumBases]: " << BCD.Data[0] << '\n';
    outs() << BCDName << "[OffsetInVBase]: " << BCD.Data[1] << '\n';
    outs() << BCDName << "[VBPtrOffset]: " << BCD.Data[2] << '\n';
    outs() << BCDName << "[OffsetInVBTable]: " << BCD.Data[3] << '\n';
    outs() << BCDName << "[Flags]: " << BCD.Data[4] << '\n';
    outs() << BCDName << "[ClassHierarchyDescriptor]: " << BCD.Symbols[1]
           << '\n';
  }
  for (const auto &TDPair : TDs) {
    StringRef TDName = TDPair.first;
    const TypeDescriptor &TD = TDPair.second;
    outs() << TDName << "[VFPtr]: " << TD.Symbols[0] << '\n';
    outs() << TDName << "[AlwaysZero]: " << TD.AlwaysZero << '\n';
    outs() << TDName << "[MangledName]: ";
    outs().write_escaped(TD.MangledName.rtrim(StringRef("\0", 1)),
                         /*UseHexEscapes=*/true)
        << '\n';
  }
  for (const auto &TIPair : TIs) {
    StringRef TIName = TIPair.first;
    const ThrowInfo &TI = TIPair.second;
    auto dumpThrowInfoFlag = [&](const char *Name, uint32_t Flag) {
      outs() << TIName << "[Flags." << Name
             << "]: " << (TI.Flags & Flag ? "true" : "false") << '\n';
    };
    auto dumpThrowInfoSymbol = [&](const char *Name, int Offset) {
      outs() << TIName << '[' << Name << "]: ";
      auto Entry = TIEntries.find(std::make_pair(TIName, Offset));
      outs() << (Entry == TIEntries.end() ? "null" : Entry->second) << '\n';
    };
    outs() << TIName << "[Flags]: " << TI.Flags << '\n';
    dumpThrowInfoFlag("Const", 1);
    dumpThrowInfoFlag("Volatile", 2);
    dumpThrowInfoSymbol("CleanupFn", 4);
    dumpThrowInfoSymbol("ForwardCompat", 8);
    dumpThrowInfoSymbol("CatchableTypeArray", 12);
  }
  for (const auto &CTAPair : CTAs) {
    StringRef CTAName = CTAPair.first;
    const CatchableTypeArray &CTA = CTAPair.second;

    outs() << CTAName << "[NumEntries]: " << CTA.NumEntries << '\n';

    unsigned Idx = 0;
    for (auto I = CTAEntries.lower_bound(std::make_pair(CTAName, 0)),
              E = CTAEntries.upper_bound(std::make_pair(CTAName, UINT64_MAX));
         I != E; ++I)
      outs() << CTAName << '[' << Idx++ << "]: " << I->second << '\n';
  }
  for (const auto &CTPair : CTs) {
    StringRef CTName = CTPair.first;
    const CatchableType &CT = CTPair.second;
    auto dumpCatchableTypeFlag = [&](const char *Name, uint32_t Flag) {
      outs() << CTName << "[Flags." << Name
             << "]: " << (CT.Flags & Flag ? "true" : "false") << '\n';
    };
    outs() << CTName << "[Flags]: " << CT.Flags << '\n';
    dumpCatchableTypeFlag("ScalarType", 1);
    dumpCatchableTypeFlag("VirtualInheritance", 4);
    outs() << CTName << "[TypeDescriptor]: " << CT.Symbols[0] << '\n';
    outs() << CTName << "[NonVirtualBaseAdjustmentOffset]: "
           << CT.NonVirtualBaseAdjustmentOffset << '\n';
    outs() << CTName
           << "[VirtualBasePointerOffset]: " << CT.VirtualBasePointerOffset
           << '\n';
    outs() << CTName << "[VirtualBaseAdjustmentOffset]: "
           << CT.VirtualBaseAdjustmentOffset << '\n';
    outs() << CTName << "[Size]: " << CT.Size << '\n';
    outs() << CTName
           << "[CopyCtor]: " << (CT.Symbols[1].empty() ? "null" : CT.Symbols[1])
           << '\n';
  }
  for (const auto &VTTPair : VTTEntries) {
    StringRef VTTName = VTTPair.first.first;
    uint64_t VTTOffset = VTTPair.first.second;
    StringRef VTTEntry = VTTPair.second;
    outs() << VTTName << '[' << VTTOffset << "]: " << VTTEntry << '\n';
  }
  for (const auto &TIPair : TINames) {
    StringRef TIName = TIPair.first;
    outs() << TIName << ": " << TIPair.second << '\n';
  }
  auto VTableSymI = VTableSymEntries.begin();
  auto VTableSymE = VTableSymEntries.end();
  auto VTableDataI = VTableDataEntries.begin();
  auto VTableDataE = VTableDataEntries.end();
  for (;;) {
    bool SymDone = VTableSymI == VTableSymE;
    bool DataDone = VTableDataI == VTableDataE;
    if (SymDone && DataDone)
      break;
    if (!SymDone && (DataDone || VTableSymI->first < VTableDataI->first)) {
      StringRef VTableName = VTableSymI->first.first;
      uint64_t Offset = VTableSymI->first.second;
      StringRef VTableEntry = VTableSymI->second;
      outs() << VTableName << '[' << Offset << "]: ";
      outs() << VTableEntry;
      outs() << '\n';
      ++VTableSymI;
      continue;
    }
    if (!DataDone && (SymDone || VTableDataI->first < VTableSymI->first)) {
      StringRef VTableName = VTableDataI->first.first;
      uint64_t Offset = VTableDataI->first.second;
      int64_t VTableEntry = VTableDataI->second;
      outs() << VTableName << '[' << Offset << "]: ";
      outs() << VTableEntry;
      outs() << '\n';
      ++VTableDataI;
      continue;
    }
  }
}

static void dumpArchive(const Archive *Arc) {
  for (const Archive::Child &ArcC : Arc->children()) {
    ErrorOr<std::unique_ptr<Binary>> ChildOrErr = ArcC.getAsBinary();
    if (std::error_code EC = ChildOrErr.getError()) {
      // Ignore non-object files.
      if (EC != object_error::invalid_file_type)
        reportError(Arc->getFileName(), EC.message());
      continue;
    }

    if (ObjectFile *Obj = dyn_cast<ObjectFile>(&*ChildOrErr.get()))
      dumpCXXData(Obj);
    else
      reportError(Arc->getFileName(), cxxdump_error::unrecognized_file_format);
  }
}

static void dumpInput(StringRef File) {
  // If file isn't stdin, check that it exists.
  if (File != "-" && !sys::fs::exists(File)) {
    reportError(File, cxxdump_error::file_not_found);
    return;
  }

  // Attempt to open the binary.
  ErrorOr<OwningBinary<Binary>> BinaryOrErr = createBinary(File);
  if (std::error_code EC = BinaryOrErr.getError()) {
    reportError(File, EC);
    return;
  }
  Binary &Binary = *BinaryOrErr.get().getBinary();

  if (Archive *Arc = dyn_cast<Archive>(&Binary))
    dumpArchive(Arc);
  else if (ObjectFile *Obj = dyn_cast<ObjectFile>(&Binary))
    dumpCXXData(Obj);
  else
    reportError(File, cxxdump_error::unrecognized_file_format);
}

int main(int argc, const char *argv[]) {
  sys::PrintStackTraceOnErrorSignal();
  PrettyStackTraceProgram X(argc, argv);
  llvm_shutdown_obj Y;

  // Initialize targets.
  llvm::InitializeAllTargetInfos();

  // Register the target printer for --version.
  cl::AddExtraVersionPrinter(TargetRegistry::printRegisteredTargetsForVersion);

  cl::ParseCommandLineOptions(argc, argv, "LLVM C++ ABI Data Dumper\n");

  // Default to stdin if no filename is specified.
  if (opts::InputFilenames.size() == 0)
    opts::InputFilenames.push_back("-");

  std::for_each(opts::InputFilenames.begin(), opts::InputFilenames.end(),
                dumpInput);

  return ReturnValue;
}
