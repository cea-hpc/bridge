
AC_DEFUN([AX_TORQUE],
[

	saved_CPPFLAGS="$CPPFLAGS"
	saved_LDFLAGS="$LDFLAGS"

	ac_have_torque=no

	AC_ARG_WITH(torque,
		AS_HELP_STRING(--with-torque@<:@=PATH@:>@  ,Specify TORQUE support and path),
		[ ac_have_torque=yes; TORQUEDIR="$withval" ]
	)
	AC_ARG_WITH(torque-lib,
		AS_HELP_STRING(--with-torque-lib@<:@=PATH@:>@,Specify TORQUE libraries path),
		[ TORQUELIBDIR="$withval" ]
	)

	if test "x$ac_have_torque" == "xyes" ; then

		dnl extend CPPFLAGS and LDFLAGS if required
		if test "x$TORQUEDIR" != "x" && test "x$TORQUEDIR" != "xyes" ; then
			TORQUE_CPPFLAGS="-I$TORQUEDIR/include"
			CPPFLAGS="${CPPFLAGS} ${TORQUE_CPPFLAGS}"
			TORQUE_LDFLAGS="-L$TORQUEDIR/lib"
                        LDFLAGS="$LDFLAGS $TORQUE_LDFLAGS"
		fi
		if test "x$TORQUELIBDIR" != "x" && test "x$TORQUELIBDIR" != "xyes" ; then
			TORQUE_LDFLAGS="-L$TORQUELIBDIR"
                        LDFLAGS="$LDFLAGS $TORQUE_LDFLAGS"
                fi

		dnl check for headers	
		AC_CHECK_HEADERS([pbs_ifl.h],[has_torque_header="true"])
		if test "x$has_torque_header" != "xtrue" ; then
			AC_MSG_ERROR([unable to use TORQUE without pbs_ifl.h])
		fi
		dnl check for headers	
		AC_CHECK_HEADERS([pbs_error.h],[has_torque_header="true"])
		if test "x$has_torque_header" != "xtrue" ; then
			AC_MSG_ERROR([unable to use TORQUE without pbs_error.h])
		fi

		dnl check for libraries
		AC_CHECK_LIB([torque],[pbs_connect],[has_torque_lib="yes"],[has_torque_lib="no"])
		if test "x$has_torque_lib" != "xyes" ; then
                        AC_MSG_ERROR([unable to use TORQUE without libtorque])
		else
                        TORQUE_LDFLAGS="${TORQUE_LDFLAGS} -ltorque"
			LDFLAGS="$LDFLAGS -ltorque"
                fi

		dnl set batch system name and plugin
		dnl then set batch_system<->rm_system binding method
		dnl (used during configuration files creation)
		BRIDGE_BATCH_SYSTEM=torque
		AC_SUBST([BRIDGE_BATCH_SYSTEM])
		BRIDGE_BATCH_PLUGIN=libbridge_bs_torque.so
		AC_SUBST([BRIDGE_BATCH_PLUGIN])
		BRIDGE_BINDING=none
		AC_SUBST([BRIDGE_BINDING])

		dnl set resource management system name
		BRIDGE_RM_SYSTEM=torque
		AC_SUBST([BRIDGE_RM_SYSTEM])

	fi

	CPPFLAGS="$saved_CPPFLAGS"
	LDFLAGS="$saved_LDFLAGS"

	AC_SUBST([TORQUE_CPPFLAGS])
	AC_SUBST([TORQUE_LDFLAGS])

	AM_CONDITIONAL(HAVE_TORQUE, test "x$ac_have_torque" = "xyes")
	AC_SUBST(HAVE_TORQUE)

])

