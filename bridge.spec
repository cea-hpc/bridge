# evaluate systemd availability

%undefine _enable_debug_packages
%define debug_package %{nil}

# Should unpackaged files in a build root terminate a build?
# Note: The default value should be 0 for legacy compatibility.
%define _unpackaged_files_terminate_build      0

# Remove the space between % and trace to activate RPM debug traces
# % trace

# By default :
# > program prefix is ccc_
# > scripts and apps in /usr
# > configuration files in /etc
%define _program_prefix ccc_
%define prefix /usr
%define sysconfdir /etc

# By default compiling with slurm.
# To compile without slurm you can use --without slurm as argument to rpmbuild
%bcond_without slurm

# By default creating flux rpm
# To not create the package use rpmbuild with --without flux as argument
%bcond_without flux

Summary: Bridge CCC In-House Batch Environment
Name: bridge
Version: 1.5.14
Release: 3%{?dist}
License: GPL License
Group: System Environment/Base
URL: https://github.com/cea-hpc/bridge
Source0: %{name}-%{version}.tar.gz

BuildRequires: autoconf
BuildRequires: automake
BuildRequires: libtool

%if 0%{?fedora} >= 28 || 0%{?rhel} >= 8
BuildRequires:  libtirpc-devel
%endif

Requires: clustershell

# Required for %%post, %%preun, %%postun
Requires:       systemd
BuildRequires:  systemd

# by default program prefix is ccc_
# you can alter it using --define "_program_prefix foo_" or remove it
# using --define "_program_prefix %%{nil}"
%{!?_program_prefix: %global _program_prefix "ccc_"}

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

%define _prefix     %{prefix}
%define _bindir     %{prefix}/bin
%define _sbindir    %{prefix}/sbin
%define _sysconfdir %{sysconfdir}
%define _datadir    %{prefix}/share
%define _includedir %{prefix}/include
%define _sharedstatedir %{prefix}/share
%define _mandir     %{_datadir}/man
%define _infodir    %{_datadir}/info
%define _docdir     %{_datadir}/doc
%define _libexecdir /usr/libexec

# nqsII and LSF not yet supported
%bcond_with nqsII
%bcond_with lsf

%description
Bridge CCC In-House Batch Environment gives a uniform way to access external
Batch scheduling systems.
It currently provides plugins for slurm.

%package devel
Summary: Development package for Bridge
Group: Development/System
Requires: bridge
%description devel
Development package for bridge. This package includes the header files
for the bridge API as well as the material to link against the dynamic library.

%if %{with slurm}
%package slurm
Summary: Slurm plugins for Bridge CCC In-House Batch Environment
Group: System Environment/Base
Requires: slurm >= 22.05.3
Requires: bridge >= %{version}
Requires: clustershell
BuildRequires: slurm-devel >= 22.05.3

%description slurm
Plugin that provides Slurm access across the CCC Batch systems Bridge
%endif

%prep
%setup -q


%build
autoreconf -fvi
%configure %{?with_slurm:--with-slurm}

%{__make} %{?_smp_mflags}

%install
%{make_install} DESTDIR=%{buildroot}
%if %{with slurm}
chmod 755 %{buildroot}/%{_prefix}/share/scripts/resource_manager/addons/get_task_info
%endif

install -d -m0755  %{buildroot}%{_unitdir}
install -Dp -m0644 etc/init.d/bridged.service %{buildroot}%{_unitdir}/%{_program_prefix}bridged.service
rm %{buildroot}%{_sysconfdir}/init.d/%{_program_prefix}bridged
rm %{buildroot}%{_sysconfdir}/logrotate.d/%{_program_prefix}bridged
mv %{buildroot}%{_sysconfdir}/logrotate.d/%{_program_prefix}bridged.systemd %{buildroot}%{_sysconfdir}/logrotate.d/%{_program_prefix}bridged
chmod 0644 %{buildroot}%{_sysconfdir}/logrotate.d/%{_program_prefix}bridged

# ensure bridged existence as it is used by systemd
pushd %{buildroot}%{_sbindir}
ln -s %{_program_prefix}bridged bridged
popd

%post
if [[ -f %{_sysconfdir}/logrotate.d/%{_program_prefix}bridged.rpmsave ]]
then
    rm -f %{_sysconfdir}/logrotate.d/%{_program_prefix}bridged.rpmsave
fi

%files
%defattr(-,root,root,-)
%dir %{_prefix}/share/scripts
%dir %{_prefix}/share/scripts/common
%dir %{_prefix}/share/scripts/addons
%dir %{_prefix}/share/scripts/batch_system
%dir %{_prefix}/share/scripts/batch_system/profiles
%dir %{_prefix}/share/scripts/batch_system/plugins
%dir %{_prefix}/share/scripts/resource_manager
%dir %{_prefix}/share/scripts/resource_manager/profiles
%dir %{_prefix}/share/scripts/resource_manager/addons
%dir %{_prefix}/share/scripts/resource_manager/plugins
%config (noreplace) %{_sysconfdir}/bridge.conf
%config (noreplace) %{_sysconfdir}/bridge_bs.conf
%config (noreplace) %{_sysconfdir}/bridge_rm.conf
%{_bindir}/%{_program_prefix}*
%{_libdir}/libbridge.so.*
%{_prefix}/share/scripts/common/*
%{_prefix}/share/scripts/addons/*
%{_prefix}/share/scripts/batch_system/plugins/generic
%{_prefix}/share/scripts/resource_manager/plugins/generic
%{_prefix}/share/scripts/resource_manager/plugins/ws
%{_sbindir}/%{_program_prefix}bridged
%{_sbindir}/bridged
%config (noreplace) %{_sysconfdir}/bridged.conf
%config (noreplace) %{_sysconfdir}/bridgedapi.conf
%{_unitdir}/%{_program_prefix}bridged.service
%{_sysconfdir}/logrotate.d/%{_program_prefix}bridged
%{_includedir}/bridgedapi.h
%{_libdir}/libbridgedapi.*

%files devel
%defattr(-,root,root,-)
%{_includedir}/bridge.h
%{_includedir}/bridge_common.h
%{_libdir}/libbridge.so

%if %{with slurm}
%files slurm
%defattr(-,root,root,-)
%{_prefix}/share/scripts/batch_system/profiles/slurm.sh
%{_prefix}/share/scripts/batch_system/plugins/slurm
%{_prefix}/share/scripts/resource_manager/profiles/slurm.sh
%{_prefix}/share/scripts/resource_manager/plugins/slurm
%{_prefix}/share/scripts/resource_manager/addons/get_task_info
%{_libdir}/libbridge_*_slurm*
%endif

%if %{with flux}
%package flux
Summary: Flux plugins for Bridge CCC In-House Batch Environment
Group: System Environment/Base
Requires: slurm >= 22.05.3
Requires: bridge >= %{version}
Requires: clustershell
BuildRequires: slurm-devel >= 22.05.3

%description flux
Plugin that provides Flux access across the CCC Batch systems Bridge

%files flux
%defattr(-,root,root,-)
%{_prefix}/share/scripts/batch_system/plugins/flux
%{_prefix}/share/scripts/resource_manager/plugins/flux
%endif

%changelog
* Fri Oct 18 2024 Olivier Delhomme <olivier.delhomme@cea.fr> - 1.5.14-3
- fix bridge.spec to avoid requiring a specific version of clustershell
  in all packages: bridge-slurm and bridge-flux.

* Thu Oct 17 2024 Olivier Delhomme <olivier.delhomme@cea.fr> - 1.5.14-2
- fix bridge.spec to avoid requiring a specific version of clustershell
  as any decent version will provide a functional nodeset program

* Fri Sep 27 2024 Olivier Delhomme <olivier.delhomme@cea.fr> - 1.5.14-1
- Fix for mpmd and Slurm 23
- bridge.spec file now requires clustershell as nodeset is used
  in the project. systemd availability check removed (presumed
  always available)
- Boundary check before usage in xstream.c
- Building bridge in silent mode by default
- Corrects a pointer comparison against a value
- Avoiding a compilation warning because of a too short destination
- Code maintenance: new headers and put some function declarations
  into them
- Corrects a hardcoded PATH into something more generic:
  "$HOME/.local/share/bridge"
- Corrects a bug that prevented -a usage in #MSUB directives
- Array addon added
- flux: adaptation for slurm 23.11 and stdio append mode. Removed
  mini command used in flux invocation since it is deprecated
  since flux 0.48.0 version.


* Wed Jan 10 2024 Olivier Delhomme <olivier.delhomme@cea.fr> - 1.5.13-1
- Adds flux plugins and flux addons and mpmd-cluster-heterogenous.ad
  and env-cleaner.ad addons to installation and packaging systems.

* Wed Jan 10 2024 Olivier Delhomme <olivier.delhomme@cea.fr> - 1.5.12-1
- Removes cea_ compatibility layer with associated package. One
  should pay attention to configuration files and external scripts
  to change names from cea_ to ccc_ or used prefix.
- Usage of $$ in scripts has been replaced with mktemp or shuf
- slurm23 integration: this version compiles with either version
  22 or 23 of slurm.

* Mon Nov 27 2023 Olivier Delhomme <olivier.delhomme@cea.fr> - 1.5.11-1
- Allow overlapping (option --overlap) when using shell.ad addon
- Adds env-cleaner.ad to allow one  to clean it's environment
- Adds the ability to use -f flag along with -H all in mstat
- Adds flux mprun plugin and use wait in mprun -B
- Adds stdio-flux.ad addon to allow usage of --output (-o), --error (-e)
  and -l (-l) options of flux (bridge equivalent)
- Allows usage of -D for debugging in msub as stated in documentation
- Restore inheritance in mprun when no option is set
- Adds -r usage when calling bsstat
- typo corrections

* Fri Jun 23 2023 Olivier Delhomme <olivier.delhomme@cea.fr> - 1.5.10-1
- Improves spec file by removing or updating old parameters.
- Adds libtirpc autodetection in configure scripts.
- Adds autogen.sh file to regenerate configure script from configure.ac file
- Adds mpmd cluster heterogeneous addon.

* Wed Apr 19 2023 Olivier Delhomme <olivier.delhomme@cea.fr> - 1.5.9-1.ocean1
- Removing libtirpc from spec file, now detection is done in configure script

* Wed Feb 09 2022 Olivier Delhomme <olivier.delhomme@cea.fr> - 1.5.7-1.ocean1
- Integrating patches and making 1.5.7 version

* Wed Sep 02 2020 Regine Gaudin <regine.gaudin@cea.fr> - 1.5.6-6.ocean7
- Change spooldir management in dirige.ad:
  bridge-1.5.6-modif-spooldir-dirige-addon.patch

* Fri Jul 24 2020 Regine Gaudin <regine.gaudin@cea.fr> - 1.5.6-6.ocean6
- Fix mismatching test in bridge-1.5.6-m-option-extra-param-erased.patch

* Mon Jul 20 2020 Regine Gaudin <regine.gaudin@cea.fr> - 1.5.6-6.ocean5
- Fix print format for ccc_msub -S option : bridge-1.5.6.msub-S.patch

* Wed Jul 01 2020 Regine Gaudin <regine.gaudin@cea.fr> - 1.5.6-6.ocean4
- Increase to 12 char username display of rmastat:
  bridge-1.5.6-rmastat-increase-username-display-to-12-char.patch
- Clean old format use in mstat, keeping only new format:
  bridge-1.5.6-mstat-supports-only-new-format.patch
- Rename dirige.ad and adapt the corresponding calls:
  bridge-1.5.6-rename-dirige-addon.patch,
  bridge-1.5.6-change-dirige-addon-name-calls.patch

* Wed Jun 24 2020 Regine Gaudin <regine.gaudin@cea.fr> - 1.5.6-6.ocean3
- Fix erased -m option filesystem extra parameter:
  bridge-1.5.6-m-option-extra-param-erased.patch

* Tue Jun 23 2020 Regine Gaudin <regine.gaudin@cea.fr> - 1.5.6-6.ocean2
- Link plugins slurm with bridge for dlopen failure:
  bridge-1.5.6-dlopen-failure.patch

* Mon Nov 25 2019  Regine Gaudin <regine.gaudin@cea.fr> - 1.5.6-6.ocean1
- Rebuild with slurm19 on rhel8, missing rpc/types.h header found in
  libtirpc-devel: bridge-1.5.6.rpc-headers-missing.patch

* Tue Jun 25 2019  Regine Gaudin <regine.gaudin@cea.fr> - 1.5.6-5.ocean4
- Add war in ksh mpmd-cluster.ad for printf bug with new set LANG=en_US.utf8

* Thu Jun 13 2019 Regine Gaudin <regine.gaudin@cea.fr> - 1.5.6-5.ocean3
- Suppress incompatibility -m option between -m affinity and -m unshare
  mounted filesystems:
  suppress bridge-1.5.6-slurm-affinity-addon.patch
  add bridge-1.5.6.option_m_suppress.patch for suppressing -m in the help
  option used for unshare mounted filesystems
  bridge-1.5.6.filesystems_addon.patch
  adapt without -m keeping -M bridge-1.5.4.msub_option_m_default.patch:
  bridge-1.5.6.msub_option_M_default.patch
  add compilation of filesystems.ad in Makefile:
  bridge-1.5.6.filesystems_addon.patch

* Fri May 24 2019 Regine Gaudin <regine.gaudin@cea.fr> - 1.5.6-5.ocean2
- Rebuild with fixed Slurm 18.08.6.ocean2

* Thu Apr 11 2019 Regine Gaudin <regine.gaudin@cea.fr>  - 1.5.6-5.ocean1
- Rebuild with slurm-18.08.06 / ocean v2.7
  add support of slurm-18 feature with multiple backup controller:
  bridge-1.5.6-multi-backup-controller.patch
  rename bridge-1.5.6-partition_info_t_priority.patch for change of
  partition_info_t priority
  fix segv in cluster name algorithm bridge-1.5.6-fix-cluster-name-segv.patch
  add slurm level 18 requirement because of char **control_machine list in
  slurm_ctl_conf_t instead of two char* master and backup

* Fri Sep 07 2018 Regine Gaudin <regine.gaudin@cea.fr> - 1.5.6-4.ocean1
-- Rebuild with slurm.17.11.6 for ocean
-- merge: bridge-1.5.6-env-variables-autoswitch.patch fix slurm environment
   variables with bridge-1.5.6.addon-autoswitch.patch:
   bridge-1.5.6-addon-autoswitch-and-env-slurm-variable-fix.patch
-- patch: bridge-1.5.6.filesystems_addon.patch
   add -m option for filesystem use description

* Fri Jun 15 2018 Regine Gaudin <regine.gaudin@cea.fr> - 1.5.6-3.ocean18
- Add / character to assure that pattern can use more than 2 definitions
   per partition
   patch: bridge-1.5.6.addon-autoswitch.patch
- Add -F foreground launch option because of several pid problem
   leading to logrotate/bridge relaunch failure
   patch: bridged-foreground-option.patch

* Fri May 18 2018 Regine Gaudin <regine.gaudin@cea.fr> - 1.5.6-3.ocean17
- Rebuild with slurm.17.11.6

* Tue Apr 17 2018 Regine Gaudin <regine.gaudin@cea.fr> and Roger Brel - 1.5.6-3.ocean16.1
- Add DEBUG_MODE in bridge_addon_msub_alterscript function on preload.ad
  bridge-1.5.6.addons-preload.patch

* Wed Jan 03 2018 Regine Gaudin <regine.gaudin@cea.fr> - 1.5.6-3.ocean16
- Rebuild  with slurm 17.11.1-1.xcea5 + cve slurmdbd

* Wed Jan 03 2018 Regine Gaudin <regine.gaudin@cea.fr> - 1.5.6-3.ocean15
- Rebuild  with slurm 17.11.1

* Fri Nov 17 2017 Regine Gaudin <regine.gaudin@cea.fr> - 1.5.6-3.ocean14
- Fix false IntelMPI  mpicc is the first in the PATH in IntelMPI addon

* Mon Nov 13 2017 Regine Gaudin <regine.gaudin@cea.fr> - 1.5.6-3.ocean13
- Rebuild with slurm 17.11

* Fri May 5 2017 Francis BELOT <francis.belot@cea.fr> - 1.5.6-3.ocean12
 -Patch13: bridge-1.5.6-mpstat-option-t.patch
     use clustack instead of padb
  Patch14: bridge-1.5.6-addons-totalview.patch
      possibility to specify less processors in embedded ccc_mprun than the
      number of processors specified in ccc_msub command
  Patch15: bridge-1.5.6-mpstat-option-p.patch
     -p option was not working

* Wed Feb 15 2017 Matthieu Hautreux <matthieu.hautreux@cea.fr> - 1.5.6-3.ocean11
- add support of /etc/ccc_nodeinfo/* as an override for part/node info in
  mpinfo/slurm. patch: bridge-1.5.6-ccc_nodeinfo.patch

* Wed Feb 15 2017 Matthieu Hautreux <matthieu.hautreux@cea.fr> - 1.5.6-3.ocean11
- add support of /etc/ccc_nodeinfo/* as an override for part/node info in
  mpinfo/slurm. patch: bridge-1.5.6-ccc_nodeinfo.patch

* Tue Feb 14 2017 Matthieu Hautreux <matthieu.hautreux@cea.fr> - 1.5.6-3.ocean10
- allow inheritance of MSUB QOS from BRIDGE_MSUB_ADDON_QOS env variable when
  BRIDGE_MSUB_ADDON_QOS_INHERITANCE env var is set to 1.
  patch: bridge-1.5.6-qos_addon_inheritance.patch

* Mon Jan 16 2017 Matthieu Hautreux <matthieu.hautreux@cea.fr> - 1.5.6-3.ocean9
- correct a regression in upgrade because of directories provided by both
  bridge and bridge-slurm

* Mon Jan 16 2017 Matthieu Hautreux <matthieu.hautreux@cea.fr> - 1.5.6-3.ocean8
- rebuild for slurm-16.05
- include slurm-16.05 prototypes changes support
  patch:bridge-1.5.6.slurm-16.05.patch

* Wed Aug 24 2016 Matthieu Hautreux <matthieu.hautreux@cea.fr> - 1.5.6-3.ocean7
- ensure that -a option of msub generates an error when -w is activated.
  patch: bridge-1.5.6.-a_vs_-w.patch

* Mon Jun 13 2016 Matthieu Hautreux <matthieu.hautreux@cea.fr> - 1.5.6-3.ocean6
- make logrotate conf work with systemd bridged service
  note that /etc/logrotate.d/%%{_program_prefix}bridged is no longer considered
  as a config file and overwritten by RPM during installation.
  Older /etc/logrotate.d/%%{_program_prefix}bridged.rpmsave file is removed in
  %%post to avoid issue with logrotate.

* Wed Jun 01 2016 Matthieu Hautreux <matthieu.hautreux@cea.fr> - 1.5.6-3.ocean5
- add systemd support in SPEC file and in the code
  patch: bridge-1.5.6.systemd.patch
- ensure that reservation addon add SLURM_RESERVATION env var
  patch: bridge-1.5.6.slurm_reservation.patch
- correct a bug in pstat args processing
  patch: bridge-1.5.6.mpstat_args.patch

* Mon Mar 21 2016 Aurelien Cedeyn <aurelien.cedeyn@cea.fr> - 1.5.6-3.ocean4
- local build with slurm resource patch (bug if empty partition in ccc_mpinfo).

* Thu Mar 17 2016 Matthieu Hautreux <matthieu.hautreux@cea.fr> - 1.5.6-1.ocean4
- local rebuild to confirm to SCS5 bullxbm-slurm-15.08 flavor (not API
  compatible)

* Fri Feb 12 2016 Matthieu Hautreux <matthieu.hautreux@cea.fr> - 1.5.6-1.ocean3
- allow to use a 'default' pragma in -M/-m of msub to override '#MSUB -M/-m'
  pragmas specified in the submission script with the system default values.
  patch: bridge-1.5.4.msub_option_m_default.patch
- add support of BRIDGE_DDT_PARAMS env var to specify additional DDT params
  when '-d ddt' is used.
  patch: bridge-1.5.4.addon_ddt_params.patch

* Fri Dec 04 2015 Matthieu Hautreux <matthieu.hautreux@cea.fr> - 1.5.6-1.ocean2
- add slurm-affinity addons (bridge-1.5.6-slurm-affinity-addon.patch)

* Fri Nov 27 2015 Matthieu Hautreux <matthieu.hautreux@cea.fr> - 1.5.6-1.ocean1
- backport following master branch commit :
  d19f096d553c217d7d360361aaefe988e434ce26

* Wed Nov 18 2015 Matthieu Hautreux <matthieu.hautreux@cea.fr> - 1.5.6-1
-- switch to bridge-1.5.6, see NEWS file for changes

* Tue Feb 17 2015 Matthieu Hautreux <matthieu.hautreux@cea.fr> - 1.5.5-3
-- more spec cleanup

* Mon Feb 02 2015 Matthieu Hautreux <matthieu.hautreux@cea.fr> - 1.5.5-2
-- more spec file cleaning
-- move devel files to a dedicated bridge-devel package

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
-- addons/mpmd-cluster-wrapper : ensure correct behavior of the plugin when
   args are passed using shell arrays

* Thu Dec 19 2013 Matthieu Hautreux <matthieu.hautreux@cea.fr> - 1.5.4-4
-- rm/plugins/ws|openmpi: correct separator management in mprun for openmpi
   and ws plugin

* Thu Dec 19 2013 Matthieu Hautreux <matthieu.hautreux@cea.fr> - 1.5.4-3
-- addons/ddt: correct a bug introduced when switching to arrays when passing
   args

* Tue Dec 17 2013 Matthieu Hautreux <matthieu.hautreux@cea.fr> - 1.5.4-2
- tag release 1.5.4-2
-- modify spec file to automate the generation of different styles of packaging:
   -- define "ccc_tera 1" : generate with cea_ prog prefix in /usr/local/bridge
      compile with slurm support by default, add a bridge-ccc_compat for ccc_
      links
   -- define "ccc_tgcc 1" : generate with ccc_ prog prefix in /usr, compile with
      slurm support by default add a bridge-cea_compat for cea_ links
   -- default : generate with ccc_ prog prefix in /usr, compile without slurm
      support, add a bridge-cea_compat for cea_ links
-- in tgcc mode (the default), create a bridge-cea_compat package providing
   the cea_* compatibility links to the ccc_* equivalents

* Tue Dec 17 2013 Matthieu Hautreux <matthieu.hautreux@cea.fr> - 1.5.4-1
- tag release 1.5.4-1
-- addons/autompi: bind /usr/bin/mpirun to mpich2 mpi type (RHEL6 MPI package)

* Wed Nov 27 2013 Francois Diakhate <francois.diakhate@cea.fr> - 1.5.4-0
- tag release 1.5.4-0
-- addons: add missing shell.ad

* Wed Nov 27 2013 Francois Diakhate <francois.diakhate@cea.fr>  - 1.5.3-12
- tag release 1.5.3-12
-- mprun: enhance BRIDGE_MPRUN_EXTRA_(ARRAY_)PARAMETERS support
-- msub/slurm: correct a reg in '-a job' management which appeared in 1.5.3-8

* Wed Nov 13 2013 Francois Diakhate <francois.diakhate@cea.fr>  - 1.5.3-11
- tag release 1.5.3-11
-- mprun: re-fix special character handling in arguments which was broken
   since 1.5.3-8
-- slurm: fix special character handling in extra-parameters for allocations

* Wed Nov 13 2013 Francois Diakhate <francois.diakhate@cea.fr>  - 1.5.3-10
- tag release 1.5.3-10
-- vtune: fix for invalid cpuset argument with slurm cgroups
-- addons: fix autodefmem which would choose the wrong value
-- mpp: optimizations for loaded machines
-- intelmpi-slurm.ad: add a new BRIDGE_SLURM_INTELMPI_CMD_DEFAULT_OPTIONS env
   var
-- addons: fix mpmd bcast if the same name is used for different binaries
-- addons: add shell addon for interactive usage of a node

* Tue Jun 25 2013 Matthieu Hautreux <matthieu.hautreux@cea.fr> - 1.5.3-9
- tag release 1.5.3-8
-- addons: fixes for incorrect construction of compounded parameters
-- mprun: unify extra parameters management using array struct and
  introduce a new option -Z "a b c" to allow to pass parameter containing
  spaces as single block parameters to the underlying resource manager command
-- msub: add support for whitespaces in submission parameters with SLURM

* Fri Apr 19 2013 Matthieu Hautreux <matthieu.hautreux@cea.fr> - 1.5.3-8
- tag release 1.5.3-8
-- autopacking: remove useless export which caused a bug in recursive
                submissions
-- rm/ws: add spmd_run_separator support in ws resource manager plugin
-- rm/ws: fixes for whitespaces and special characters handling
-- mprun: fixes for whitespaces and special characters handling in extra
          parameters management
-- rm/slurm: fixes for whitespaces and special characters handling when using
             -E "extra_params"
-- rm/*: fixes for whitespaces and special characters handling when using
         -E "extra_params"
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
* Tue Jul 17 2012 Francois Diakhate <francois.diakhate@cea.fr> - 1.4.15-4
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
* Wed Jul 06 2011 Matthieu Hautreux <matthieu.hautreux@cea.fr> - 1.4.8-1
- tag release 1.4.8

* Wed Aug 25 2010 Matthieu Hautreux <matthieu.hautreux@cea.fr> - 1.4.5-1
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
* Thu Jul 2 2009 Matthieu Hautreux <matthieu.hautreux@cea.fr> - 1.3.24-2
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
* Mon Dec 15 2008 Matthieu Hautreux <matthieu.hautreux@cea.fr> - 1.3.11-2
- Initial build.
