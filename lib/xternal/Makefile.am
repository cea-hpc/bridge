AM_CPPFLAGS		=       -I$(top_srcdir)

noinst_HEADERS		=	xerror.h xlogger.h \
				xstream.h xqueue.h \
				xmessage.h xfreelist.h \
				xlibrary.h

noinst_LTLIBRARIES	=       libxternal.la

libxternal_la_CPPFLAGS	=	-D_THREAD_SAFE -pthread
libxternal_la_SOURCES	=       xlogger.c xstream.c \
				xmessage.c xqueue.c \
				xfreelist.c xlibrary.c  
libxternal_la_LDFLAGS	=       $(LIB_LDFLAGS) -pthread
