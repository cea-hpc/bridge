# SLURM_*FLAGS are got from m4 macro during configure

if HAVE_SLURM

AM_CPPFLAGS = $(SLURM_CPPFLAGS) -I$(top_srcdir)/lib

lib_LTLIBRARIES = libbridge_rm_slurm.la
libbridge_rm_slurm_la_SOURCES = slurm_rm_manager.c
libbridge_rm_slurm_la_LDFLAGS = -module -avoid-version --export-dynamic $(SLURM_LDFLAGS)

endif # HAVE_SLURM
