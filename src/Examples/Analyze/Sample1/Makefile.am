AUTOMAKE_OPTIONS = gnu

noinst_PROGRAMS = Sample1

Sample1_LDADD = $(top_builddir)/src/MACLib/libmac.la
Sample1_CPPFLAGS = -I$(top_srcdir)/src/Shared -I$(top_srcdir)/src/MACLib

Sample1_SOURCES = Sample1.cpp
