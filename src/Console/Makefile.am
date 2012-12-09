AUTOMAKE_OPTIONS = gnu

bin_PROGRAMS = mac

mac_CPPFLAGS = -I$(top_srcdir)/src/Shared -I$(top_srcdir)/src/MACLib
mac_LDFLAGS = -export-dynamic
mac_LDADD = $(top_builddir)/src/MACLib/libmac.la

mac_SOURCES = Console.cpp
