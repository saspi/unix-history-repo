/* $FreeBSD$ */

/* Do not modify this file.  */
/* It is created automatically by the Makefile.  */
#include "defs.h"      /* For initialize_file_ftype.  */
#include "call-cmds.h" /* For initialize_all_files.  */
extern initialize_file_ftype _initialize_gdbtypes;
extern initialize_file_ftype _initialize_arm_tdep;
extern initialize_file_ftype _initialize_armfbsd_tdep;
extern initialize_file_ftype _initialize_corelow;
extern initialize_file_ftype _initialize_solib;
extern initialize_file_ftype _initialize_svr4_solib;
extern initialize_file_ftype _initialize_ser_hardwire;
extern initialize_file_ftype _initialize_ser_pipe;
extern initialize_file_ftype _initialize_ser_tcp;
#ifndef CROSS_DEBUGGER
extern initialize_file_ftype _initialize_kernel_u_addr;
extern initialize_file_ftype _initialize_infptrace;
extern initialize_file_ftype _initialize_inftarg;
#endif
extern initialize_file_ftype _initialize_arm_fbsdnat;
extern initialize_file_ftype _initialize_remote;
extern initialize_file_ftype _initialize_dcache;
extern initialize_file_ftype _initialize_sr_support;
extern initialize_file_ftype _initialize_tracepoint;
extern initialize_file_ftype _initialize_ax_gdb;
extern initialize_file_ftype _initialize_annotate;
extern initialize_file_ftype _initialize_auxv;
extern initialize_file_ftype _initialize_breakpoint;
extern initialize_file_ftype _initialize_regcache;
extern initialize_file_ftype _initialize_charset;
extern initialize_file_ftype _initialize_dummy_frame;
extern initialize_file_ftype _initialize_source;
extern initialize_file_ftype _initialize_values;
extern initialize_file_ftype _initialize_valops;
extern initialize_file_ftype _initialize_valarith;
extern initialize_file_ftype _initialize_valprint;
extern initialize_file_ftype _initialize_printcmd;
extern initialize_file_ftype _initialize_symtab;
extern initialize_file_ftype _initialize_symfile;
extern initialize_file_ftype _initialize_symmisc;
extern initialize_file_ftype _initialize_infcall;
extern initialize_file_ftype _initialize_infcmd;
extern initialize_file_ftype _initialize_infrun;
extern initialize_file_ftype _initialize_stack;
extern initialize_file_ftype _initialize_thread;
extern initialize_file_ftype _initialize_interpreter;
extern initialize_file_ftype _initialize_macrocmd;
extern initialize_file_ftype _initialize_gdbarch;
extern initialize_file_ftype _initialize_gdbarch_utils;
extern initialize_file_ftype _initialize_gdb_osabi;
extern initialize_file_ftype _initialize_copying;
extern initialize_file_ftype _initialize_mem;
extern initialize_file_ftype _initialize_parse;
extern initialize_file_ftype _initialize_language;
extern initialize_file_ftype _initialize_frame_reg;
extern initialize_file_ftype _initialize_signals;
extern initialize_file_ftype _initialize_kod;
extern initialize_file_ftype _initialize_gdb_events;
extern initialize_file_ftype _initialize_exec;
extern initialize_file_ftype _initialize_maint_cmds;
extern initialize_file_ftype _initialize_demangler;
extern initialize_file_ftype _initialize_dbxread;
extern initialize_file_ftype _initialize_coffread;
extern initialize_file_ftype _initialize_elfread;
extern initialize_file_ftype _initialize_mipsread;
extern initialize_file_ftype _initialize_stabsread;
extern initialize_file_ftype _initialize_core;
extern initialize_file_ftype _initialize_dwarf2_frame;
extern initialize_file_ftype _initialize_c_language;
extern initialize_file_ftype _initialize_f_language;
extern initialize_file_ftype _initialize_objc_language;
extern initialize_file_ftype _initialize_ui_out;
extern initialize_file_ftype _initialize_cli_out;
extern initialize_file_ftype _initialize_varobj;
extern initialize_file_ftype _initialize_java_language;
extern initialize_file_ftype _initialize_m2_language;
extern initialize_file_ftype _initialize_pascal_language;
extern initialize_file_ftype _initialize_pascal_valprint;
extern initialize_file_ftype _initialize_scheme_language;
extern initialize_file_ftype _initialize_complaints;
extern initialize_file_ftype _initialize_typeprint;
extern initialize_file_ftype _initialize_cp_valprint;
extern initialize_file_ftype _initialize_f_valprint;
extern initialize_file_ftype _initialize_nlmread;
extern initialize_file_ftype _initialize_serial;
extern initialize_file_ftype _initialize_mdebugread;
extern initialize_file_ftype _initialize_user_regs;
extern initialize_file_ftype _initialize_frame;
extern initialize_file_ftype _initialize_frame_unwind;
extern initialize_file_ftype _initialize_frame_base;
extern initialize_file_ftype _initialize_gnu_v2_abi;
extern initialize_file_ftype _initialize_gnu_v3_abi;
extern initialize_file_ftype _initialize_hpacc_abi;
extern initialize_file_ftype _initialize_cp_abi;
extern initialize_file_ftype _initialize_cp_support;
extern initialize_file_ftype _initialize_cp_namespace;
extern initialize_file_ftype _initialize_reggroup;
extern initialize_file_ftype _initialize_inflow;
extern initialize_file_ftype _initialize_cli_dump;
extern initialize_file_ftype _initialize_cli_logging;
extern initialize_file_ftype _initialize_cli_interp;
extern initialize_file_ftype _initialize_mi_out;
extern initialize_file_ftype _initialize_mi_cmds;
extern initialize_file_ftype _initialize_mi_cmd_env;
extern initialize_file_ftype _initialize_mi_interp;
extern initialize_file_ftype _initialize_mi_main;
extern initialize_file_ftype _initialize_thread_db;
extern initialize_file_ftype _initialize_tui_hooks;
extern initialize_file_ftype _initialize_tui_interp;
extern initialize_file_ftype _initialize_tui_layout;
extern initialize_file_ftype _initialize_tui_out;
extern initialize_file_ftype _initialize_tui_regs;
extern initialize_file_ftype _initialize_tui_stack;
extern initialize_file_ftype _initialize_tui_win;
void
initialize_all_files (void)
{
  _initialize_gdbtypes ();
  _initialize_arm_tdep ();
  _initialize_armfbsd_tdep ();
  _initialize_corelow ();
  _initialize_solib ();
  _initialize_svr4_solib ();
  _initialize_ser_hardwire ();
  _initialize_ser_pipe ();
  _initialize_ser_tcp ();
#ifndef CROSS_DEBUGGER
  _initialize_kernel_u_addr ();
  _initialize_infptrace ();
  _initialize_inftarg ();
  _initialize_thread_db ();
#endif
  _initialize_arm_fbsdnat ();
  _initialize_remote ();
  _initialize_dcache ();
  _initialize_sr_support ();
  _initialize_tracepoint ();
  _initialize_ax_gdb ();
  _initialize_annotate ();
  _initialize_auxv ();
  _initialize_breakpoint ();
  _initialize_regcache ();
  _initialize_charset ();
  _initialize_dummy_frame ();
  _initialize_source ();
  _initialize_values ();
  _initialize_valops ();
  _initialize_valarith ();
  _initialize_valprint ();
  _initialize_printcmd ();
  _initialize_symtab ();
  _initialize_symfile ();
  _initialize_symmisc ();
  _initialize_infcall ();
  _initialize_infcmd ();
  _initialize_infrun ();
  _initialize_stack ();
  _initialize_thread ();
  _initialize_interpreter ();
  _initialize_macrocmd ();
  _initialize_gdbarch ();
  _initialize_gdbarch_utils ();
  _initialize_gdb_osabi ();
  _initialize_copying ();
  _initialize_mem ();
  _initialize_parse ();
  _initialize_language ();
  _initialize_frame_reg ();
  _initialize_signals ();
  _initialize_kod ();
  _initialize_gdb_events ();
  _initialize_exec ();
  _initialize_maint_cmds ();
  _initialize_demangler ();
  _initialize_dbxread ();
  _initialize_coffread ();
  _initialize_elfread ();
  _initialize_mipsread ();
  _initialize_stabsread ();
  _initialize_core ();
  _initialize_dwarf2_frame ();
  _initialize_c_language ();
  _initialize_f_language ();
  _initialize_objc_language ();
  _initialize_ui_out ();
  _initialize_cli_out ();
  _initialize_varobj ();
  _initialize_java_language ();
  _initialize_m2_language ();
  _initialize_pascal_language ();
  _initialize_pascal_valprint ();
  _initialize_scheme_language ();
  _initialize_complaints ();
  _initialize_typeprint ();
  _initialize_cp_valprint ();
  _initialize_f_valprint ();
  _initialize_nlmread ();
  _initialize_serial ();
  _initialize_mdebugread ();
  _initialize_user_regs ();
  _initialize_frame ();
  _initialize_frame_unwind ();
  _initialize_frame_base ();
  _initialize_gnu_v2_abi ();
  _initialize_gnu_v3_abi ();
  _initialize_hpacc_abi ();
  _initialize_cp_abi ();
  _initialize_cp_support ();
  _initialize_cp_namespace ();
  _initialize_reggroup ();
  _initialize_inflow ();
  _initialize_cli_dump ();
  _initialize_cli_logging ();
  _initialize_cli_interp ();
  _initialize_mi_out ();
  _initialize_mi_cmds ();
  _initialize_mi_cmd_env ();
  _initialize_mi_interp ();
  _initialize_mi_main ();
  _initialize_tui_hooks ();
  _initialize_tui_interp ();
  _initialize_tui_layout ();
  _initialize_tui_out ();
  _initialize_tui_regs ();
  _initialize_tui_stack ();
  _initialize_tui_win ();
}
