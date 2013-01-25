Summary: Bridge CEA In-House Batch Environment
Name: bridge
Version: 1.5.3
Release: 1
License: GPL License
Group: System Environment/Base
URL: http://
Source0: %{name}-%{version}.tar.gz
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-root

### Thanks to slurm packagers
%define bridge_with_opt() %{expand:%%{!?_without_%{1}:%%global bridge_with_%{1} 1}}
%define bridge_without_opt() %{expand:%%{?_with_%{1}:%%global bridge_with_%{1} 1}}
%define bridge_with() %{expand:%%{?bridge_with_%{1}:1}%%{!?bridge_with_%{1}:0}}
#
# Never allow rpm to strip binaries as this will break
#  parallel debugging capability
#
%define __os_install_post /usr/lib/rpm/brp-compress
%define debug_package %{nil}
#
# Should unpackaged files in a build root terminate a build?
#
# Note: The default value should be 0 for legacy compatibility.
%define _unpackaged_files_terminate_build      0
### Thanks again

#
# By default, compile and package in Tera style
# Set tgcc to one to do it for Tgcc or tera to 0
# to do it in the default way
%if %{!?tgcc:1}%{?tgcc:0}
%if %{!?tera:1}%{?tera:0}
%define tera 1
%endif
%endif

#
# Revert to using TGCC preferences by default
#
%define tgcc 1

#
# Set Tera options if requested
# > program prefix is cea_
# > scripts and apps in /usr/local/bridge
# > configuration files in /etc
%if %{?tera}0
%define _program_prefix cea_
%define prefix /usr/local/bridge
%define sysconfdir /etc
%bridge_with_opt slurm
%endif

#
# Set Tgcc options if requested
# > program prefix is ccc_
# > scripts and apps in /usr
# > configuration files in /etc
%if %{?tgcc}0
%define _program_prefix ccc_
%define prefix /usr
%define sysconfdir /etc
%bridge_with_opt slurm
%endif


# by default program prefix is cea_
# you can alter it using --define "_program_prefix foo_" or remove it
# using --define "_program_prefix %{nil}"
%if %{!?_program_prefix:1}%{?_program_prefix:0}
%define _program_prefix cea_ 
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

# Compiled with slurm plugin as default (disable using --without slurm)
%bridge_with_opt slurm

# nqsII and LSF not yet supported
%bridge_without_opt nqsII
%bridge_without_opt lsf

%description
Bridge CEA In-House Batch Environment gives a uniform way to access external 
Batch scheduling systems.
It currently provides plugins for slurm.

%if %{bridge_with slurm}
%package slurm
Summary: Slurm plugins for Bridge CEA In-House Batch Environment
Group: System Environment/Base
Requires: slurm >= 1.3.6 bridge >= 1.3.12
%description slurm
Plugins that provides Slurm access accross the CEA Batch systems Bridge
%endif

%prep
%setup -n %{name}-%{version}

%build
autoreconf -fvi
%configure --prefix=%{_prefix} --sysconfdir=%{_sysconfdir} \
	   --program-prefix=%{?_program_prefix:%{_program_prefix}} \
	   %{?bridge_with_slurm:--with-slurm}

make %{?_smp_mflags} 

%install
rm -rf $RPM_BUILD_ROOT
mkdir -p "$RPM_BUILD_ROOT"
DESTDIR="$RPM_BUILD_ROOT" make install
%if %{bridge_with slurm}
chmod 755 ${RPM_BUILD_ROOT}/%{_prefix}/share/scripts/resource_manager/addons/get_task_info
%endif

%clean
rm -rf $RPM_BUILD_ROOT

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
%{_bindir}/*
%{_includedir}/bridge.h
%{_includedir}/bridge_common.h
%{_libdir}/libbridge.*
%{_prefix}/share/scripts/common/*
%{_prefix}/share/scripts/addons/*
%{_prefix}/share/scripts/batch_system/plugins/generic
%{_prefix}/share/scripts/resource_manager/plugins/generic
%{_sbindir}/%{_program_prefix}bridged
%config (noreplace) %{_sysconfdir}/bridged.conf
%config (noreplace) %{_sysconfdir}/bridgedapi.conf
%config %{_sysconfdir}/init.d/%{_program_prefix}bridged
%config (noreplace) %{_sysconfdir}/sysconfig/%{_program_prefix}bridged
%config %{_sysconfdir}/logrotate.d/%{_program_prefix}bridged
%{_includedir}/bridgedapi.h
%{_libdir}/libbridgedapi.*

%if %{bridge_with slurm}
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
