bin_PROGRAMS = inspect_mon		
inspect_mon_SOURCES = inspect_mon.c
inspect_mon_CFLAGS = -I$(top_srcdir)/include

# todo: this is a hack... and seems like it should come from 
#       from the top level autoconf.ac config... 
AM_LDFLAGS        = -L$(top_srcdir)/src/lib -linsp_tools
