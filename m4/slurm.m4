
AC_DEFUN([AX_SLURM],
[

	saved_CPPFLAGS="$CPPFLAGS"
	saved_LDFLAGS="$LDFLAGS"

	ac_have_slurm=no

	AC_ARG_WITH(slurm,
		AS_HELP_STRING(--with-slurm@<:@=PATH@:>@,Specify SLURM support and path),
		[ ac_have_slurm=yes; SLURMDIR="$withval" ]
	)
	AC_ARG_WITH(slurm-lib,
		AS_HELP_STRING(--with-slurm-lib@<:@=PATH@:>@,Specify SLURM libraries path),
		[ SLURMLIBDIR="$withval" ]
	)

	if test "x$ac_have_slurm" == "xyes" ; then

		SLURM_CPPFLAGS=
		SLURM_LDFLAGS=

		dnl extend CPPFLAGS and LDFLAGS if required
		if test "x$SLURMDIR" != "x" && test "x$SLURMDIR" != "xyes" ; then
			SLURM_CPPFLAGS="-I$SLURMDIR/include"
			CPPFLAGS="${CPPFLAGS} ${SLURM_CPPFLAGS}"
			SLURM_LDFLAGS="-L$SLURMDIR/lib"
                        LDFLAGS="$LDFLAGS $SLURM_LDFLAGS"
		fi
		if test "x$SLURMLIBDIR" != "x" && test "x$SLURMLIBDIR" != "xyes"  ; then
			SLURM_LDFLAGS="-L$SLURMLIBDIR"
                        LDFLAGS="$LDFLAGS $SLURM_LDFLAGS"
                fi

		dnl check for headers	
		AC_CHECK_HEADERS([slurm/slurm.h],[has_slurm_header="true"])
		if test "x$has_slurm_header" != "xtrue" ; then
			AC_MSG_ERROR([unable to use SLURM without slurm/slurm.h])
		fi

		dnl check for libraries
		AC_CHECK_LIB([slurm],[slurm_api_version],[has_slurm_lib="yes"],[has_slurm_lib="no"])
		if test "x$has_slurm_lib" != "xyes" ; then
                        AC_MSG_ERROR([unable to use SLURM without libslurm])
		else
                        SLURM_LDFLAGS="${SLURM_LDFLAGS} -lslurm -L../../../lib/bridge/.libs -lbridge"
			LDFLAGS="$LDFLAGS -lslurm -L../../../lib/bridge/.libs -lbridge"

			dnl set resource management system name and plugin
			dnl (used during configuration files creation)
			BRIDGE_RM_SYSTEM=slurm
			AC_SUBST([BRIDGE_RM_SYSTEM])
			BRIDGE_RM_PLUGIN=libbridge_rm_slurm.so
			AC_SUBST([BRIDGE_RM_PLUGIN])

			dnl set batch system name if not already set
			dnl (used during configuration files creation)
			if test "x$BRIDGE_BATCH_SYSTEM" == "xnone" ; then
				BRIDGE_BATCH_SYSTEM=slurm
				AC_SUBST([BRIDGE_BATCH_SYSTEM])
				BRIDGE_BATCH_PLUGIN=libbridge_bs_slurm.so
				AC_SUBST([BRIDGE_BATCH_PLUGIN])
			fi

                fi

	fi

	CPPFLAGS="$saved_CPPFLAGS"
	LDFLAGS="$saved_LDFLAGS"

	AC_SUBST([SLURM_CPPFLAGS])
	AC_SUBST([SLURM_LDFLAGS])

	AM_CONDITIONAL(HAVE_SLURM, test "x$ac_have_slurm" = "xyes")
	AC_SUBST(HAVE_SLURM)

])
