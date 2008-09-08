# use the default c++ flags
AM_CXXFLAGS=@DEF_CXXFLAGS@

INCLUDES = -I. -I$(top_srcdir)/cxcore/include -I$(top_srcdir)

noinst_HEADERS     = _cxcore.h _cxipp.h
noinst_LTLIBRARIES = lib_cxcore.la
lib_LTLIBRARIES    = libcxcore.la

# convenience library
lib_cxcore_la_SOURCES = cxalloc.cpp cxarithm.cpp cxarray.cpp cxcmp.cpp \
    cxconvert.cpp cxcopy.cpp cxdatastructs.cpp cxdrawing.cpp \
    cxdxt.cpp cxerror.cpp cximage.cpp cxjacobieigens.cpp cxlogic.cpp cxlut.cpp \
    cxmathfuncs.cpp cxmatmul.cpp cxmatrix.cpp cxmean.cpp \
    cxmeansdv.cpp cxminmaxloc.cpp cxnorm.cpp cxouttext.cpp \
    cxpersistence.cpp cxprecomp.cpp cxrand.cpp cxsumpixels.cpp \
    cxsvd.cpp cxswitcher.cpp cxtables.cpp cxutils.cpp
lib_cxcore_la_LDFLAGS = -no-undefined @LDFLAGS@

# real library
libcxcore_la_SOURCES = dummy.cpp
libcxcore_la_LDFLAGS = -no-undefined -version-info @LT_VERSION@ @LDFLAGS@
libcxcore_la_LIBADD  = lib_cxcore.la @LTLIBOBJS@
