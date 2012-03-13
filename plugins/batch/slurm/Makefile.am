AUTOMAKE_OPTIONS = foreign

# SLURM_*FLAGS are got from m4 macro

if !HAVE_LSF_SLURM
if HAVE_SLURM

PLUGIN_FLAGS = 

AM_CPPFLAGS = $(SLURM_CPPFLAGS) -I$(top_srcdir)/lib -g

lib_LTLIBRARIES = libbridge_bs_slurm.la
libbridge_bs_slurm_la_SOURCES = 	batch_manager.c  batch_node.c  batch_queue.c  \
					batch_session.c batch_common.c batch_common.h
libbridge_bs_slurm_la_LDFLAGS = 	-module -avoid-version \
					--export-dynamic $(SLURM_LDFLAGS)

endif # HAVE_SLURM
endif # !HAVE_LSF_SLURM
