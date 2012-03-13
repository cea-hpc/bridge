lib_LTLIBRARIES			=	libbridge.la

include_HEADERS			=	bridge.h bridge_common.h

libbridge_la_SOURCES		=	bridge_common.c  bridge_idlist.c  bridge_manager.c  bridge_nodelist.c \
					bridge_range.c bridge_common.h  bridge.h

libbridge_la_CPPFLAGS		=       -D_THREAD_SAFE -I$(top_srcdir) -I../ -pthread \
					-DBRIDGE_CONF=\"$(sysconfdir)/bridge.conf\"

libbridge_la_LDFLAGS		=	$(LIB_LDFLAGS) -pthread

libbridge_la_LIBADD		=	../confparse/libconfig_parsing.la ../xternal/libxternal.la
