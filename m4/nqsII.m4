
AC_DEFUN([AX_NQSII],
[

	saved_CPPFLAGS="$CPPFLAGS"
	saved_LDFLAGS="$LDFLAGS"

	ac_have_nqsII=no

	AC_ARG_WITH(nqsII,
		AS_HELP_STRING(--with-nqsII@<:@=PATH@:>@,Specify NQSII support and path),
		[ ac_have_nqsII=yes; NQSIIDIR="$withval" ]
	)
	AC_ARG_WITH(nqsII-lib,
		AS_HELP_STRING(--with-nqsII-lib@<:@=PATH@:>@,Specify NQSII libraries path),
		[ NQSIILIBDIR="$withval" ]
	)

	if test "x$ac_have_nqsII" == "xyes" ; then

		dnl extend CPPFLAGS and LDFLAGS if required
		if test "x$NQSIIDIR" != "x" && test "x$NQSIIDIR" != "xyes" ; then
			NQSII_CPPFLAGS="-I$NQSIIDIR/include"
			CPPFLAGS="${CPPFLAGS} ${NQSII_CPPFLAGS}"
			NQSII_LDFLAGS="-L$NQSIIDIR/lib"
                        LDFLAGS="$LDFLAGS $NQSII_LDFLAGS"
		fi
		if test "x$NQSIILIBDIR" != "x" && test "x$NQSIILIBDIR" != "xyes" ; then
			NQSII_LDFLAGS="-L$NQSIILIBDIR"
                        LDFLAGS="$LDFLAGS $NQSII_LDFLAGS"
                fi

		dnl check for headers	
		AC_CHECK_HEADERS([nqsII.h],[has_nqsII_header="true"])
		if test "x$has_nqsII_header" != "xtrue" ; then
			AC_MSG_ERROR([unable to use NQSII without nqsII.h])
		fi

		dnl check for libraries
		AC_CHECK_LIB([nqsII],[NQSconnect],[has_nqsII_lib="yes"],[has_nqsII_lib="no"])
		if test "x$has_nqsII_lib" != "xyes" ; then
                        AC_MSG_ERROR([unable to use NQSII without libnqsII])
		else
                        NQSII_LDFLAGS="${NQSII_LDFLAGS} -lnqsII"
			LDFLAGS="$LDFLAGS -lnqsII"
                fi

		dnl set batch system name and plugin
		dnl then set batch_system<->rm_system binding method
		dnl (used during configuration files creation)
		BRIDGE_BATCH_SYSTEM=nqsII
		AC_SUBST([BRIDGE_BATCH_SYSTEM])
		BRIDGE_BATCH_PLUGIN=libbridge_bs_nqsII.so
		AC_SUBST([BRIDGE_BATCH_PLUGIN])
		BRIDGE_RM_SYSTEM=nqsII
		AC_SUBST([BRIDGE_RM_SYSTEM])
		BRIDGE_RM_PLUGIN=libbridge_bs_nqsII.so
		AC_SUBST([BRIDGE_RM_PLUGIN])
		BRIDGE_BINDING=none
		AC_SUBST([BRIDGE_BINDING])

	fi

	CPPFLAGS="$saved_CPPFLAGS"
	LDFLAGS="$saved_LDFLAGS"

	AC_SUBST([NQSII_CPPFLAGS])
	AC_SUBST([NQSII_LDFLAGS])

	AM_CONDITIONAL(HAVE_NQSII, test "x$ac_have_nqsII" = "xyes")
	AC_SUBST(HAVE_NQSII)

])
