# use the default c++ flags
AM_CXXFLAGS=@DEF_CXXFLAGS@

EXTRA_DIST = cxts.dsp cxts.vs2005.vcproj precomp.cpp

INCLUDES = -I. -I$(top_srcdir)/cxcore/include -I$(top_srcdir)/tests
AM_CFLAGS = @CFLAGS@ @DEBUG@

check_LTLIBRARIES = libcxts.la

libcxts_la_SOURCES = _cxts.h cxts.h cxts.cpp cxts_arrtest.cpp cxts_math.cpp
libcxts_la_LIBADD  = $(top_builddir)/cxcore/src/libcxcore.la
