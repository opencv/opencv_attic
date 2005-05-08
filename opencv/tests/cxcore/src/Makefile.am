INCLUDES = -I$(top_srcdir)/cxcore/include -I$(top_srcdir)/tests/cxts

check_PROGRAMS = cxcoretest

cxcoretest_SOURCES = cxcoretest.h aarithm.cpp adatastruct.cpp adxt.cpp amath.cpp cxcoretest_main.cpp
cxcoretest_LDADD = -L$(top_builddir)/cxcore/src -lcxcore -L$(top_builddir)/tests/cxts -lcxts

EXTRA_DIST = precomp.cpp cxcoretest.dsp cxcoretest.vcproj
