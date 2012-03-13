AUTOMAKE_OPTIONS = foreign

# LSF_*FLAGS are got from m4 macro

if HAVE_LSF_SLURM

PLUGIN_FLAGS = 

AM_CPPFLAGS = $(LSF_CPPFLAGS) -I$(top_srcdir)/lib -g

lib_LTLIBRARIES = libbridge_bs_lsf_slurm.la
libbridge_bs_lsf_slurm_la_SOURCES = lsf_batch_manager.c  lsf_batch_node.c  lsf_batch_queue.c  lsf_batch_session.c
libbridge_bs_lsf_slurm_la_LIBADD = $(LSF_LDFLAGS) -lm
libbridge_bs_lsf_slurm_la_LDFLAGS = -module -avoid-version --export-dynamic


endif # HAVE_SLURM
