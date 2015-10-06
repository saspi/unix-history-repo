//===-- FileAction.cpp ------------------------------------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include <fcntl.h>

#if defined(_WIN32)
#include "lldb/Host/Windows/win32.h" // For O_NOCTTY
#endif

#include "lldb/Core/Stream.h"
#include "lldb/Target/FileAction.h"

using namespace lldb_private;

//----------------------------------------------------------------------------
// FileAction member functions
//----------------------------------------------------------------------------

FileAction::FileAction() : 
    m_action(eFileActionNone),
    m_fd(-1),
    m_arg(-1),
    m_file_spec()
{
}

void
FileAction::Clear()
{
    m_action = eFileActionNone;
    m_fd = -1;
    m_arg = -1;
    m_file_spec.Clear();
}

const char *
FileAction::GetPath() const
{
    return m_file_spec.GetCString();
}

const FileSpec &
FileAction::GetFileSpec() const
{
    return m_file_spec;
}

bool
FileAction::Open(int fd, const FileSpec &file_spec, bool read, bool write)
{
    if ((read || write) && fd >= 0 && file_spec)
    {
        m_action = eFileActionOpen;
        m_fd = fd;
        if (read && write)
            m_arg = O_NOCTTY | O_CREAT | O_RDWR;
        else if (read)
            m_arg = O_NOCTTY | O_RDONLY;
        else
            m_arg = O_NOCTTY | O_CREAT | O_WRONLY;
        m_file_spec = file_spec;
        return true;
    }
    else
    {
        Clear();
    }
    return false;
}

bool
FileAction::Close(int fd)
{
    Clear();
    if (fd >= 0)
    {
        m_action = eFileActionClose;
        m_fd = fd;
    }
    return m_fd >= 0;
}

bool
FileAction::Duplicate(int fd, int dup_fd)
{
    Clear();
    if (fd >= 0 && dup_fd >= 0)
    {
        m_action = eFileActionDuplicate;
        m_fd = fd;
        m_arg = dup_fd;
    }
    return m_fd >= 0;
}

void
FileAction::Dump(Stream &stream) const
{
    stream.PutCString("file action: ");
    switch (m_action)
    {
        case eFileActionClose:
            stream.Printf("close fd %d", m_fd);
            break;
        case eFileActionDuplicate:
            stream.Printf("duplicate fd %d to %d", m_fd, m_arg);
            break;
        case eFileActionNone:
            stream.PutCString("no action");
            break;
        case eFileActionOpen:
            stream.Printf("open fd %d with '%s', OFLAGS = 0x%x",
                    m_fd, m_file_spec.GetCString(), m_arg);
            break;
    }
}
