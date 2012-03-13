AUTOMAKE_OPTIONS = foreign

# TORQUE_*FLAGS are got from m4 macro

if HAVE_TORQUE

PLUGIN_FLAGS = 

AM_CPPFLAGS = $(TORQUE_CPPFLAGS) -I$(top_srcdir)/lib -g

lib_LTLIBRARIES = libbridge_bs_torque.la
libbridge_bs_torque_la_SOURCES = 	batch_manager.c  batch_node.c  batch_queue.c  \
					batch_session.c batch_common.c batch_common.h
libbridge_bs_torque_la_LDFLAGS = 	-module -avoid-version \
					--export-dynamic $(TORQUE_LDFLAGS)

endif # HAVE_TORQUE
