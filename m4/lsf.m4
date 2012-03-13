
AC_DEFUN([AX_LSF],
[

	saved_CPPFLAGS="$CPPFLAGS"
	saved_LDFLAGS="$LDFLAGS"

	ac_have_lsf=no

	AC_ARG_WITH(lsf,
		AS_HELP_STRING(--with-lsf@<:@=PATH@:>@  ,Specify LSF support and path),
		[ ac_have_lsf=yes; LSFDIR="$withval" ]
	)
	AC_ARG_WITH(lsf-lib,
		AS_HELP_STRING(--with-lsf-lib@<:@=PATH@:>@,Specify LSF libraries path),
		[ LSFLIBDIR="$withval" ]
	)

	if test "x$ac_have_lsf" == "xyes" ; then

		dnl extend CPPFLAGS and LDFLAGS if required
		if test "x$LSFDIR" != "x" && test "x$LSFDIR" != "xyes" ; then
			LSF_CPPFLAGS="-I$LSFDIR/include"
			CPPFLAGS="${CPPFLAGS} ${LSF_CPPFLAGS}"
			LSF_LDFLAGS="-L$LSFDIR/lib -lnsl"
                        LDFLAGS="$LDFLAGS $LSF_LDFLAGS"
		fi
		if test "x$LSFLIBDIR" != "x" && test "x$LSFLIBDIR" != "xyes" ; then
			LSF_LDFLAGS="-L$LSFLIBDIR -lnsl"
                        LDFLAGS="$LDFLAGS $LSF_LDFLAGS"
                fi

		dnl check for headers	
		AC_CHECK_HEADERS([lsf/lsf.h],[has_lsf_header="true"])
		if test "x$has_lsf_header" != "xtrue" ; then
			AC_MSG_ERROR([unable to use LSF without lsf/lsf.h])
		fi
		AC_CHECK_HEADERS([lsf/lsbatch.h],[has_lsbatch_header="true"])
		if test "x$has_lsbatch_header" != "xtrue" ; then
			AC_MSG_ERROR([unable to use LSF without lsf/lsbatch.h])
		fi

		dnl check for libraries
		AC_CHECK_LIB([lsf],[ls_connect],[has_lsf_lib="yes"],[has_lsf_lib="no"])
		if test "x$has_lsf_lib" != "xyes" ; then
                        AC_MSG_ERROR([unable to use LSF without liblsf])
		else
                        LSF_LDFLAGS="${LSF_LDFLAGS} -llsf -lbat"
			LDFLAGS="$LDFLAGS -llsf -lbat"
                fi

		dnl set batch system name and plugin
		dnl then set batch_system<->rm_system binding method
		dnl (used during configuration files creation)
		BRIDGE_BATCH_SYSTEM=lsf
		AC_SUBST([BRIDGE_BATCH_SYSTEM])
		BRIDGE_BATCH_PLUGIN=libbridge_bs_lsf.so
		AC_SUBST([BRIDGE_BATCH_PLUGIN])
		BRIDGE_BINDING=none
		AC_SUBST([BRIDGE_BINDING])

		dnl set resource management system if not already done
		if test "x$BRIDGE_RM_SYSTEM" != "x" ; then
			BRIDGE_RM_SYSTEM=lsf
			AC_SUBST([BRIDGE_RM_SYSTEM])
		fi

	fi

	CPPFLAGS="$saved_CPPFLAGS"
	LDFLAGS="$saved_LDFLAGS"

	AC_SUBST([LSF_CPPFLAGS])
	AC_SUBST([LSF_LDFLAGS])

	AM_CONDITIONAL(HAVE_LSF, test "x$ac_have_lsf" = "xyes")
	AC_SUBST(HAVE_LSF)

])

AC_DEFUN([AX_LSF_SLURM],
[
	saved_CPPFLAGS="$CPPFLAGS"
	saved_LDFLAGS="$LDFLAGS"

	ac_have_lsf_slurm=no

        AC_ARG_WITH(lsf-slurm,
                AS_HELP_STRING(--with-lsf-slurm,Specify LSF with Slurm binding support),
                [ ac_have_lsf_slurm=yes ]
        )

	if test "x$ac_have_lsf_slurm" == "xyes" ; then

		if test "x$ac_have_lsf" != "xyes" ; then
			AX_LSF
		fi
		if test "x$ac_have_lsf" != "xyes" ; then
			AC_MSG_ERROR([unable to use LSF-Slurm without LSF])
		else
			CPPFLAGS="${CPPFLAGS} ${LSF_CPPFLAGS}"
			LDFLAGS="${LDFLAGS} ${LSF_LDFLAGS}"

			dnl check for headers
			AC_CHECK_HEADERS([lsf/openlsfslurm.h],[has_openlsfslurm_header="true"])
			if test "x$has_openlsfslurm_header" != "xtrue" ; then
				AC_MSG_ERROR([unable to use LSF-Slurm without lsf/openlsfslurm.h])
			fi

			dnl check for libraries
			AC_CHECK_LIB([openlsfslurm],[slurm_run],[has_openlsfslurm_lib="yes"],[has_openlsfslurm_lib="no"])
			if test "x$has_openlsfslurm_lib" != "xyes" ; then
	                        AC_MSG_ERROR([unable to use LSF-Slurm without libopenlsfslurm])
			else
				ac_have_lsf_slurm=yes
	                fi

			dnl with lsf-slurm, we use lsf rmid value for Batch_system<->Rm_system binding
			dnl we also use a different batch system plugin
			dnl (used during configuration files creation)
			BRIDGE_BINDING=rmid
			AC_SUBST([BRIDGE_BINDING])
			BRIDGE_BATCH_PLUGIN=libbridge_bs_lsf_slurm.so
			AC_SUBST([BRIDGE_BATCH_PLUGIN])

		fi

	fi

	CPPFLAGS="$saved_CPPFLAGS"
	LDFLAGS="$saved_LDFLAGS"

	AM_CONDITIONAL(HAVE_LSF_SLURM, test "x$ac_have_lsf_slurm" = "xyes")
	AC_SUBST(HAVE_LSF_SLURM)

])
