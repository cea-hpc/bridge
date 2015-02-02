#
# Never allow rpm to strip binaries as this will break
#  parallel debugging capability
#
%define __os_install_post /usr/lib/rpm/brp-compress
%define debug_package %{nil}
#
# Should unpackaged files in a build root terminate a build?
# Note: The default value should be 0 for legacy compatibility.
%define _unpackaged_files_terminate_build      0

# Remove the space between % and trace to activate RPM debug traces
# (Commenting %blabla does not really disable it)
# % trace

#
# Set Tera options if requested
# > program prefix is cea_
# > scripts and apps in /usr/local/bridge
# > configuration files in /etc
%if %{?ccc_tera}0
%define _program_prefix cea_
%define prefix /usr/local/bridge
%define sysconfdir /etc
%bcond_without slurm
%define compat_target ccc
%define target tera
%endif

#
# Set Tgcc options if requested
# > program prefix is ccc_
# > scripts and apps in /usr
# > configuration files in /etc
%if %{?ccc_tgcc}0
%define _program_prefix ccc_
%define prefix /usr
%define sysconfdir /etc
%bcond_without slurm
%define compat_target cea
%define target tgcc
%endif

#
# If no target specified (neither tera nor tgcc),
# compile without SLURM but in TGCC style
# with cea_compat links
%if %{!?target:1}0
%define _program_prefix ccc_
%define prefix /usr
%define sysconfdir /etc
%define compat_target cea
%define target ws
%endif

Summary: Bridge CCC In-House Batch Environment
Name: bridge
Version: 1.5.5
Release: 1.%{?target}
License: GPL License
Group: System Environment/Base
URL: http://
Source0: %{name}-%{version}.tar.gz
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-root

# by default program prefix is ccc_
# you can alter it using --define "_program_prefix foo_" or remove it
# using --define "_program_prefix %{nil}"
%if %{!?_program_prefix:1}%{?_program_prefix:0}
%define _program_prefix ccc_ 
%endif

# by default prefix is /usr
# you can alter it using --define "prefix /usr/local/bridge"
%if %{!?prefix:1}%{?prefix:0}
%define prefix /usr
%endif

# by default sysconfdir is /etc
# you can alter it using --define "sysconfdir /usr/local/bridge/etc"
%if %{!?sysconfdir:1}%{?sysconfdir:0}
%define sysconfdir /etc
%endif

%define _prefix         %{prefix}
%define _bindir         %{prefix}/bin
%define _sbindir        %{prefix}/sbin
%define _sysconfdir     %{sysconfdir}
%define _datadir        %{prefix}/share
%define _includedir     %{prefix}/include
%define _sharedstatedir %{prefix}/share
%define _mandir         %{_datadir}/man
%define _infodir        %{_datadir}/info
%define _docdir         %{_datadir}/doc
%define _libexecdir     /usr/libexec

# # Compiled with slurm plugin as default (disable using --without slurm)
# % bridge_with_opt slurm

# nqsII and LSF not yet supported
%bcond_with nqsII
%bcond_with lsf

%description
Bridge CCC In-House Batch Environment gives a uniform way to access external 
Batch scheduling systems.
It currently provides plugins for slurm.

%if %{with slurm}
%package slurm
Summary: Slurm plugins for Bridge CCC In-House Batch Environment
Group: System Environment/Base
Requires: slurm >= 1.3.6
Requires: bridge >= %{version}
%description slurm
Plugins that provides Slurm access accross the CCC Batch systems Bridge
%endif

%prep
%setup -q

%build
autoreconf -fvi
%configure %{?with_slurm:--with-slurm}
make %{?_smp_mflags} 

%install
make install DESTDIR="$RPM_BUILD_ROOT"
%if %{with slurm}
chmod 755 ${RPM_BUILD_ROOT}/%{_prefix}/share/scripts/resource_manager/addons/get_task_info
%endif
%if %{?compat_target}0
# create cea_ compatibility links
for dr in %{buildroot}/%{_bindir} %{buildroot}/%{_sbindir} %{buildroot}/%{_sysconfdir}/init.d %{buildroot}/%{_sysconfdir}/sysconfig
do
	pushd $dr
      	for fl in %{_program_prefix}* ; do
      	    ln -s $fl %{?compat_target}_${fl##%{_program_prefix}}
      	done
	popd
done
%endif

%files
%defattr(-,root,root,-)
%dir %{_prefix}/share/scripts
%dir %{_prefix}/share/scripts/common
%dir %{_prefix}/share/scripts/addons
%dir %{_prefix}/share/scripts/batch_system
%dir %{_prefix}/share/scripts/batch_system/profiles
%dir %{_prefix}/share/scripts/batch_system/addons
%dir %{_prefix}/share/scripts/batch_system/plugins
%dir %{_prefix}/share/scripts/resource_manager
%dir %{_prefix}/share/scripts/resource_manager/profiles
%dir %{_prefix}/share/scripts/resource_manager/addons
%dir %{_prefix}/share/scripts/resource_manager/plugins
%config (noreplace) %{_sysconfdir}/bridge.conf
%config (noreplace) %{_sysconfdir}/bridge_bs.conf
%config (noreplace) %{_sysconfdir}/bridge_rm.conf
%{_bindir}/%{_program_prefix}*
%{_includedir}/bridge.h
%{_includedir}/bridge_common.h
%{_libdir}/libbridge.*
%{_prefix}/share/scripts/common/*
%{_prefix}/share/scripts/addons/*
%{_prefix}/share/scripts/batch_system/plugins/generic
%{_prefix}/share/scripts/resource_manager/plugins/generic
%{_prefix}/share/scripts/resource_manager/plugins/ws
%{_sbindir}/%{_program_prefix}bridged
%config (noreplace) %{_sysconfdir}/bridged.conf
%config (noreplace) %{_sysconfdir}/bridgedapi.conf
%config %{_sysconfdir}/init.d/%{_program_prefix}bridged
%config (noreplace) %{_sysconfdir}/sysconfig/%{_program_prefix}bridged
%config %{_sysconfdir}/logrotate.d/%{_program_prefix}bridged
%{_includedir}/bridgedapi.h
%{_libdir}/libbridgedapi.*

%if %{?compat_target}0
%package %{?compat_target}_compat
Summary: Compatibility links for alternative Bridge command prefix
Group: System Environment/Base
Requires: bridge
%description %{?compat_target}_compat
Additional package providing %{?compat_target}_* compatibility links to the 
%{_program_prefix}* commands provided by the standard flavor of Bridge.

%files %{?compat_target}_compat
%defattr(-,root,root,-)
%{_bindir}/%{?compat_target}_*
%{_sbindir}/%{?compat_target}_*
%config %{_sysconfdir}/init.d/%{?compat_target}_bridged
%config (noreplace) %{_sysconfdir}/sysconfig/%{?compat_target}_bridged
%endif

%if %{with slurm}
%files slurm
%defattr(-,root,root,-)
%dir %{_prefix}/share/scripts/batch_system
%dir %{_prefix}/share/scripts/batch_system/profiles
%dir %{_prefix}/share/scripts/batch_system/addons
%dir %{_prefix}/share/scripts/batch_system/plugins
%dir %{_prefix}/share/scripts/resource_manager
%dir %{_prefix}/share/scripts/resource_manager/profiles
%dir %{_prefix}/share/scripts/resource_manager/addons
%dir %{_prefix}/share/scripts/resource_manager/plugins
%{_prefix}/share/scripts/batch_system/profiles/slurm.sh
%{_prefix}/share/scripts/batch_system/plugins/slurm
%{_prefix}/share/scripts/resource_manager/profiles/slurm.sh
%{_prefix}/share/scripts/resource_manager/plugins/slurm
%{_prefix}/share/scripts/resource_manager/addons/get_task_info
%{_libdir}/libbridge_*_slurm*
%endif

%changelog
* Mon Feb 02 2015 Matthieu Hautreux <matthieu.hautreux@cea.fr> - 1.5.5-1
-- switch to bridge-1.5.5, see NEWS file for changes
-- simplify spec file

* Mon Feb 02 2015 Matthieu Hautreux <matthieu.hautreux@cea.fr> - 1.5.4-10
-- integrate recent patches from prod sites
   -- addons: translate usage in english and add support for ltrace, valgrind
   -- addons: ensure that bcast.ad uses which using /usr/bin/which
   -- addons: fix arguments passing in strace addons and a bug in strace.sh
   -- add -t|T [min-]max support in msub and mprun for Slurm backend
   -- correct a bug preventing from using -e with IntelMPI when used in
      salloc/mpirun
   -- ensure that BRIDGE_MSUB_QUEUE env var is used in msub when
      no -q option is set
   -- autoexclusive.ad: add BRIDGE_ADDON_AUTOEXCLUSIVE_FEATURES support
   -- totalview.ad: add BRIDGE_TOTALVIEW_CMD and BRIDGE_TOTALVIEW_PARAMS

* Fri Oct 10 2014 Matthieu Hautreux <matthieu.hautreux@cea.fr> - 1.5.4-9
-- mpp: ensure that scripts basedir is extracted from autotools prefix
-- autotools: add missing helpers Makefile.in file
-- fix multiple incorrect usage of getopt in different command line clients
   (bugs triggered when compiled with unsigned char, like on ARM processors)
-- check for AR in autotools
-- msub|bs/slurm: allow trailing script parameters in msub. The synopsis
   of msub with Slurm is now "msub [args] script [params]"

* Tue May 20 2014 Matthieu Hautreux <matthieu.hautreux@cea.fr> - 1.5.4-8
-- addons/reservation.ad: new addon to use a particular SLURM reservation
   as defined using the BRIDGE_RESERVATION env variable.
-- addons/autoswitches-slurm.ad: new addons to automatically specify the value
   of the max number of switches requested in SLURM

* Wed Apr 09 2014 Matthieu Hautreux <matthieu.hautreux@cea.fr> - 1.5.4-7
-- addons/intelmpi-slurm.ad: switch to intelmpi mode when intel|intel64 is
   detected in mpirun path

* Tue Jan 28 2014 Matthieu Hautreux <matthieu.hautreux@cea.fr> - 1.5.4-6
-- addons: add new strace addon for mprun
   (enabling to trace parallel apps using strace)
-- switch to properly defined integers when managing ksh internal
   debug/verbosity levels

* Mon Jan 13 2014 Matthieu Hautreux <matthieu.hautreux@cea.fr> - 1.5.4-5
-- addons/totalview: ensure that a debug step not using the job resources
   (i.e. lower -n ... value) works properly
-- addons/mpmd-cluster-wrapper : ensure correct behavior of the plugin when args are
   passed using shell arrays

* Thu Dec 19 2013 Matthieu Hautreux <matthieu.hautreux@cea.fr> - 1.5.4-4
-- rm/plugins/ws|openmpi: correct separator management in mprun for openmpi and ws plugin

* Thu Dec 19 2013 Matthieu Hautreux <matthieu.hautreux@cea.fr> - 1.5.4-3
-- addons/ddt: correct a bug introduced when switching to arrays when passing args

* Tue Dec 17 2013 Matthieu Hautreux <matthieu.hautreux@cea.fr> - 1.5.4-2
- tag release 1.5.4-2
-- modify spec file to automate the generation of different styles of packaging :
   --define "ccc_tera 1" : generate with cea_ prog prefix in /usr/local/bridge
   	    	           compile with slurm support by default
   	    	      	   add a bridge-ccc_compat for ccc_ links
   --define "ccc_tgcc 1" : generate with ccc_ prog prefix in /usr
   	    	       	   compile with slurm support by default
   	    	       	   add a bridge-cea_compat for cea_ links
   default : generate with ccc_ prog prefix in /usr
   	     compile without slurm support
	     add a bridge-cea_compat for cea_ links

-- in tgcc mode (the default), create a bridge-cea_compat package providing
   the cea_* compatibility links to the ccc_* equivalents

* Tue Dec 17 2013 Matthieu Hautreux <matthieu.hautreux@cea.fr> - 1.5.4-1
- tag release 1.5.4-1
-- addons/autompi: bind /usr/bin/mpirun to mpich2 mpi type (RHEL6 MPI package)

* Wed Nov 27 2013 Francois Diakhate <francois.diakhate@cea.fr>  - 1.5.4-0
- tag release 1.5.4-0
-- addons: add missing shell.ad

* Wed Nov 27 2013 Francois Diakhate <francois.diakhate@cea.fr>  - 1.5.3-12
- tag release 1.5.3-12
-- mprun: enhance BRIDGE_MPRUN_EXTRA_(ARRAY_)PARAMETERS support
-- msub/slurm: correct a reg in '-a job' management which appeared in 1.5.3-8

* Wed Nov 13 2013 Francois Diakhate <francois.diakhate@cea.fr>  - 1.5.3-11
- tag release 1.5.3-11
-- mprun: re-fix special character handling in arguments which was broken since 1.5.3-8
-- slurm: fix special character handling in extra-parameters for allocations

* Wed Nov 13 2013 Francois Diakhate <francois.diakhate@cea.fr>  - 1.5.3-10
- tag release 1.5.3-10
-- vtune: fix for invalid cpuset argument with slurm cgroups
-- addons: fix autodefmem which would choose the wrong value
-- mpp: optimizations for loaded machines
-- intelmpi-slurm.ad: add a new BRIDGE_SLURM_INTELMPI_CMD_DEFAULT_OPTIONS env var
-- addons: fix mpmd bcast if the same name is used for different binaries
-- addons: add shell addon for interactive usage of a node

* Tue Jun 25 2013 Matthieu Hautreux <matthieu.hautreux@cea.fr> - 1.5.3-9
- tag release 1.5.3-8
-- addons: fixes for incorrect construction of compounded parameters
-- mprun: unify extra parameters management using array struct and
  introduce a new option -Z "a b c" to allow to pass parameter containing
  spaces as single block parameters to the underlying resource manager command
-- msub: add support for whitespaces in submission parameters with SLURM

* Tue Apr 19 2013 Matthieu Hautreux <matthieu.hautreux@cea.fr> - 1.5.3-8
- tag release 1.5.3-8
-- autopacking: remove useless export which caused a bug in recursive submissions
-- rm/ws: add spmd_run_separator support in ws resource manager plugin
-- rm/ws: fixes for whitespaces and special characters handling
-- mprun: fixes for whitespaces and special characters handling in extra parameters management
-- rm/slurm: fixes for whitespaces and special characters handling when using -E "extra_params"
-- rm/*: fixes for whitespaces and special characters handling when using -E "extra_params"
* Tue Apr 16 2013 Francois Diakhate <francois.diakhate@cea.fr> - 1.5.3-7
- tag release 1.5.3-7
* Thu Mar 28 2013 Matthieu Hautreux <matthieu.hautreux@cea.fr> - 1.5.3-6
- tag release 1.5.3-6
- now add ws plugin in the default bridge RPM for standalone mode
* Thu Mar 28 2013 Matthieu Hautreux <matthieu.hautreux@cea.fr> - 1.5.3-5
- tag release 1.5.3-5
* Thu Mar 07 2013 Matthieu Hautreux <matthieu.hautreux@cea.fr> - 1.5.3-4
- tag release 1.5.3-4
* Thu Feb 14 2013 Francois Diakhate <francois.diakhate@cea.fr> - 1.4.15-10
- tag release 1.4.15-10
* Fri Feb 01 2013 Matthieu Hautreux <matthieu.hautreux@cea.fr> - 1.5.3-2
- tag release 1.5.3-2
* Tue Jan 29 2013 Francois Diakhate <francois.diakhate@cea.fr> - 1.4.15-9
- tag release 1.4.15-9
* Fri Jan 25 2013 Matthieu Hautreux <matthieu.hautreux@cea.fr> - 1.5.3-1
- tag release 1.5.3-1

* Wed Nov 28 2012 Matthieu Hautreux <matthieu.hautreux@cea.fr> - 1.5.2-1
- tag release 1.5.2-1
* Tue Nov 27 2012 Matthieu Hautreux <matthieu.hautreux@cea.fr> - 1.5.1-1
- tag release 1.5.1-1
* Mon Sep 17 2012 Matthieu Hautreux <matthieu.hautreux@cea.fr> - 1.5.0-1
- tag release 1.5.0-1 with GPL licensing information
* Mon Aug 20 2012 Francois Diakhate <francois.diakhate@cea.fr> - 1.4.15-6
- tag release 1.4.15-6
* Fri Jul 27 2012 Francois Diakhate <francois.diakhate@cea.fr> - 1.4.15-5
- tag release 1.4.15-5
* Mon Jul 17 2012 Francois Diakhate <francois.diakhate@cea.fr> - 1.4.15-4
- tag release 1.4.15-4
* Mon Jul 16 2012 Francois Diakhate <francois.diakhate@cea.fr> - 1.4.15-3
- tag release 1.4.15-3
* Wed Jun 27 2012 Matthieu Hautreux <matthieu.hautreux@cea.fr> - 1.4.15-1
- tag release 1.4.15-1 for usage on TGCC/TERA+ machines
* Wed Jun 27 2012 Matthieu Hautreux <matthieu.hautreux@cea.fr> - 1.4.14-1
- tag release 1.4.14-1

* Tue Nov 08 2011 Matthieu Hautreux <matthieu.hautreux@cea.fr> - 1.4.12-1
- tag release 1.4.12-1
* Tue Nov 08 2011 Matthieu Hautreux <matthieu.hautreux@cea.fr> - 1.4.10-1
- tag release 1.4.10
* Fri Jul 07 2011 Matthieu Hautreux <matthieu.hautreux@cea.fr> - 1.4.8-1
- tag release 1.4.8

* Wed Aug 27 2010 Matthieu Hautreux <matthieu.hautreux@cea.fr> - 1.4.5-1
- tag release 1.4.5
* Wed Jun 16 2010 Matthieu Hautreux <matthieu.hautreux@cea.fr> - 1.4.4-1
- tag release 1.4.4
* Wed Feb 10 2010 Matthieu Hautreux <matthieu.hautreux@cea.fr> - 1.4.2-1
- tag release 1.4.2
* Tue Jan 26 2010 Matthieu Hautreux <matthieu.hautreux@cea.fr> - 1.4.1-1
- tag release 1.4.1 for better handling of two distinct branches (CEA vs CCRT)
  that will have to be merge again in the future

* Thu Dec 03 2009 Matthieu Hautreux <matthieu.hautreux@cea.fr> - 1.3.24-10
- tag release 10
* Mon Nov 23 2009 Matthieu Hautreux <matthieu.hautreux@cea.fr> - 1.3.24-9
- tag release 9
* Wed Nov 18 2009 Matthieu Hautreux <matthieu.hautreux@cea.fr> - 1.3.24-8
- add patch v8
* Wed Oct 28 2009 Matthieu Hautreux <matthieu.hautreux@cea.fr> - 1.3.24-7
- add patch v7
* Wed Oct 28 2009 Matthieu Hautreux <matthieu.hautreux@cea.fr> - 1.3.24-6
- add patch to enhance batch support of Slurm
* Wed Oct 21 2009 Matthieu Hautreux <matthieu.hautreux@cea.fr> - 1.3.24-5
- add patch to enhance batch support of Slurm
* Fri Jul 24 2009 Matthieu Hautreux <matthieu.hautreux@cea.fr> - 1.3.24-3
- add patch1 to correctly set BRIDGE env variables when openmpi is used
  with slurm and the slurm.sh profile is sourced
* Mon Jul 2 2009 Matthieu Hautreux <matthieu.hautreux@cea.fr> - 1.3.24-2
- add patch0 to compile with surm-2.1
* Tue Jun 2 2009 Matthieu Hautreux <matthieu.hautreux@cea.fr> - 1.3.21-1
- 1.3.21 release (see Changelog)
* Fri Apr 24 2009 Matthieu Hautreux <matthieu.hautreux@cea.fr> - 1.3.17-1
- 1.3.17 release (english transcription of user commands)
* Thu Apr 16 2009 Matthieu Hautreux <matthieu.hautreux@cea.fr> - 1.3.16-1
- 1.3.16 release
* Thu Mar 12 2009 Matthieu Hautreux <matthieu.hautreux@cea.fr> - 1.3.13-1
- 1.3.13 release
* Wed Feb 11 2009 Matthieu Hautreux <matthieu.hautreux@cea.fr> - 1.3.12-2
- Modify openmpi-slurm addon to enable external configuration of the
  launcher and its options
* Mon Feb 9 2009 Matthieu Hautreux <matthieu.hautreux@cea.fr> - 1.3.12-1
- Add openmpi support in mprun for Slurm using external addon and some
  minor changes 
* Fri Jan 30 2009 Matthieu Hautreux <matthieu.hautreux@cea.fr> - 1.3.11-4
- Correct a bug in msub -W processing

* Thu Dec 18 2008 Matthieu Hautreux <matthieu.hautreux@cea.fr> - 1.3.11-3
- Move conf load in mprun to set addon in conf file
- Add a patch to use /etc as a default config directory
* Mon Dec 15 2008 Matthieu Hautreux <matthieu.hautreux@cea.fr> - 
- Initial build.
