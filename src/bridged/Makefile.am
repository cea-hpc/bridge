AM_CPPFLAGS             =       -D_THREAD_SAFE  -I$(top_srcdir)/lib -pthread \
				-DBRIDGED_CONF=\"$(sysconfdir)/bridged.conf\"
AM_LDFLAGS		=	-pthread

LIBBRIDGEDAPI		=	$(top_srcdir)/lib/bridged/libbridgedapi.la

sbin_PROGRAMS           =       bridged

noinst_HEADERS		=	bridged_engine.h \
				worker.h

bridged_SOURCES		=	bridged_engine.c bridged.c \
				worker.c

bridged_LDADD		=	$(LIBBRIDGEDAPI)
