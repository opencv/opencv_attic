# use the default c++ flags
AM_CXXFLAGS=@DEF_CXXFLAGS@

EXTRA_DIST = precomp.cpp cxcoretest.dsp cxcoretest.vs2005.vcproj

INCLUDES =                             \
	-I$(top_srcdir)/cxcore/include \
	-I$(top_srcdir)/tests/cxts

check_PROGRAMS = cxcoretest
TESTS          = $(check_PROGRAMS)

cxcoretest_SOURCES =        \
	cxcoretest.h        \
	aarithm.cpp         \
	adatastruct.cpp     \
	adxt.cpp            \
	amath.cpp           \
	asolvepoly.cpp      \
	cxcoretest_main.cpp
cxcoretest_LDADD = \
	$(top_builddir)/tests/cxts/libcxts.la \
	$(top_builddir)/cxcore/src/libcxcore.la
