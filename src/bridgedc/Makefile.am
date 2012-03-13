AM_CPPFLAGS             =       -D_THREAD_SAFE  -I$(top_srcdir)/lib -pthread \
				-DBRIDGED_CONF=\"$(sysconfdir)/bridged.conf\"
AM_LDFLAGS		=	-pthread

LIBBRIDGEDAPI		=	$(top_srcdir)/lib/bridged/libbridgedapi.la

bin_PROGRAMS            =       bridgedc

bridgedc_SOURCES	=	bridgedc.c

bridgedc_LDADD		=	$(LIBBRIDGEDAPI)
