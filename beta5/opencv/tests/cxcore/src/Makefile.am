INCLUDES =                             \
	-I$(top_srcdir)/cxcore/include \
	-I$(top_srcdir)/tests/cxts

check_PROGRAMS = cxcoretest

cxcoretest_SOURCES =        \
	cxcoretest.h        \
	aarithm.cpp         \
	adatastruct.cpp     \
	adxt.cpp            \
	amath.cpp           \
	cxcoretest_main.cpp
cxcoretest_LDADD =                              \
	$(top_builddir)/cxcore/src/libcxcore.la \
	$(top_builddir)/tests/cxts/libcxts.a

EXTRA_DIST = precomp.cpp cxcoretest.dsp cxcoretest.vcproj
