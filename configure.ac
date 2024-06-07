dnl Package initialisation
AC_INIT([bridge], [1.5.13], [])

dnl set auxiliary scripts directory (install-sh,...)
AC_CONFIG_AUX_DIR([build-aux])

dnl set in-house m4 macro directory
AC_CONFIG_MACRO_DIR([m4])

dnl foreign package
AM_INIT_AUTOMAKE([-Wall -Werror foreign])

dnl build configure.h
AC_CONFIG_HEADERS([config.h])

dnl check ar for static libraries
dnl AM_PROG_AR was introduced in automake 1.11.2 so we cannot use it directly
m4_ifdef([AM_PROG_AR], [AM_PROG_AR])

dnl build lib using autotools .la format
AC_PROG_LIBTOOL

dnl C code
AC_PROG_CC
AM_PROG_CC_C_O

dnl lex an yacc support
AC_PROG_LEX
AM_PROG_LEX
AC_PROG_YACC

dnl set bridge default parameters
AX_BRIDGE

dnl look for Platform-LSF Batch system support
AX_LSF

dnl look for Platform-LSF Batch system with Slurm binding support
AX_LSF_SLURM

dnl look for NQSII Batch system support
AX_NQSII

dnl look for Torque Batch system support
AX_TORQUE

dnl look for slurm RM system support
AX_SLURM

dnl libraries requirements
PKG_PROG_PKG_CONFIG([0.23])

dnl checking for rpc/types.h headers and if
dnl not found it checks for libtirpc
AC_CHECK_HEADERS([rpc/types.h], [],
 [
  LIBTIRPC_VERSION=1.1.4
  AC_SUBST(LIBTIRPC_VERSION)
  PKG_CHECK_MODULES(LIBTIRPC, [libtirpc >= $LIBTIRPC_VERSION])
  AC_SUBST(LIBTIRPC_CFLAGS)
  AC_SUBST(LIBTIRPC_LIBS)
 ]
)

dnl ***********************************************************************
dnl * If we are running automake >= 1.11, we can                          *
dnl * have nicer compilation output. silent option                        *
dnl ***********************************************************************
AC_ARG_ENABLE([silent],
     [  --disable-silent        Disable quiet compilation],
     [case "${enableval}" in
       yes) silent=false ;;
       no)  silent=true ;;
       *) AC_MSG_ERROR([bad value ${enableval} for --disable-silent]) ;;
     esac],[silent=false])
if test x$silent = xtrue
then
    AM_SILENT_RULES([no])
else
    AM_SILENT_RULES([yes])
fi



dnl build these conf files
AC_CONFIG_FILES([Makefile
		lib/Makefile
		lib/confparse/Makefile
		lib/xternal/Makefile
		lib/bridge/Makefile
		lib/bridged/Makefile
		plugins/Makefile
		plugins/batch/Makefile
		plugins/batch/lsf/Makefile
		plugins/batch/lsf_slurm/Makefile
		plugins/batch/torque/Makefile
		plugins/batch/slurm/Makefile
		plugins/rm/Makefile
		plugins/rm/slurm/Makefile
		src/Makefile
		src/bridge/Makefile
		src/bridged/Makefile
		src/bridgedc/Makefile
		src/helpers/Makefile
		scripts/Makefile
		scripts/common/Makefile
		scripts/common/bridge_command:scripts/common/bridge_command.in
		scripts/batch_system/Makefile
		scripts/batch_system/msub:scripts/batch_system/msub.in
		scripts/batch_system/mstat:scripts/batch_system/mstat.in
		scripts/batch_system/mpp:scripts/batch_system/mpp.in
		scripts/batch_system/mdel:scripts/batch_system/mdel.in
		scripts/resource_manager/Makefile
		scripts/resource_manager/mprun:scripts/resource_manager/mprun.in
		scripts/resource_manager/mpstat:scripts/resource_manager/mpstat.in
		scripts/resource_manager/mpinfo:scripts/resource_manager/mpinfo.in
		scripts/addons/Makefile
		scripts/addons/vtune.sh:scripts/addons/vtune.sh.in
		etc/Makefile
		etc/bridge.conf:etc/bridge.conf.in
		etc/bridge_bs.conf:etc/bridge_bs.conf.in
		etc/bridge_rm.conf:etc/bridge_rm.conf.in
		etc/bridged.conf:etc/bridged.conf.in
		etc/bridgedapi.conf:etc/bridgedapi.conf.in
		etc/init.d/bridged:etc/init.d/bridged.in
		etc/init.d/Makefile
		etc/logrotate.d/bridged.systemd
		etc/logrotate.d/Makefile
		etc/sysconfig/Makefile])

dnl output all declared files
AC_OUTPUT

AC_MSG_NOTICE([
 *** Flags that will be used to compile ***
 CFLAGS              : ${CFLAGS}
 LDFLAGS             : ${LDFLAGS}
 LIBTIRPC CFLAGS     : ${LIBTIRPC_CFLAGS}
 LIBTIRPC LIBS       : ${LIBTIRPC_LIBS}
 *** Dumping configuration ***
     - Host          : $host
     - Target        : $target
     - Build For OS  : $build_os
     - Compiler      : ${CC}
     - Prefix        : $prefix
     - Exec prefix   : ${exec_prefix}
You can now run 'make' to compile bridge.
])
