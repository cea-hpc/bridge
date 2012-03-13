AM_CPPFLAGS		=	-D_THREAD_SAFE -I$(top_srcdir)/lib -pthread
AM_LDFLAGS		=	$(top_srcdir)/lib/bridge/libbridge.la -ldl -pthread

bin_PROGRAMS		=	idlist nodelist \
				bmstat bsstat bqstat bnstat \
				rmmstat rmpstat rmastat

idlist_SOURCES		=	idlist.c
nodelist_SOURCES	=	nodelist.c

bmstat_SOURCES		=	bmstat.c
bsstat_SOURCES		=	bsstat.c
bqstat_SOURCES		=	bqstat.c
bnstat_SOURCES		=	bnstat.c

rmmstat_SOURCES		=	rmmstat.c
rmpstat_SOURCES		=	rmpstat.c
rmastat_SOURCES		=	rmastat.c
rmastat_LDADD		=	-lm
