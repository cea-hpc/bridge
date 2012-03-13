lib_LTLIBRARIES			=	libbridgedapi.la

noinst_HEADERS			=	bridge_engine.h \
					bridge_request.h \
					bridge_rus.h

include_HEADERS			=	bridgedapi.h

libbridgedapi_la_SOURCES	=	bridge_engine.c \
					bridge_request.c \
					bridge_rus.c \
					bridgedapi.c

libbridgedapi_la_CPPFLAGS	=	-D_THREAD_SAFE -I$(top_srcdir)/lib -pthread -DBRIDGEDAPI_CONF=\"$(sysconfdir)/bridgedapi.conf\"
libbridgedapi_la_LDFLAGS	=	$(LIB_LDFLAGS) -version-info 0:1:0 -pthread
libbridgedapi_la_LIBADD		=	$(top_srcdir)/lib/confparse/libconfig_parsing.la \
					$(top_srcdir)/lib/xternal/libxternal.la

