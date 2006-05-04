EXTRA_DIST = cxts.dsp cxts.vcproj precomp.cpp

INCLUDES = -I. -I$(top_srcdir)/cxcore/include -I$(top_srcdir)/tests
AM_CFLAGS = @CFLAGS@ @DEBUG@

check_LIBRARIES = libcxts.a

libcxts_a_SOURCES = _cxts.h cxts.h cxts.cpp cxts_arrtest.cpp cxts_math.cpp
libcxts_a_LIBADD  = $(top_builddir)/cxcore/src/libcxcore.la
