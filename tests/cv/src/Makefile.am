# use the default c++ flags
AM_CXXFLAGS=@DEF_CXXFLAGS@

INCLUDES =                            \
    -I.                               \
    -I$(top_srcdir)/tests/cxts        \
    -I$(top_srcdir)/include/opencv

check_PROGRAMS   = cvtest
TESTS            = $(check_PROGRAMS)
MOSTLYCLEANFILES = .lst .sum

noinst_HEADERS = \
    cvtest.h
cvtest_SOURCES = \
    aaccum.cpp               aadaptthresh.cpp         aapproxpoly.cpp \
    acameracalibration.cpp   acamshift.cpp            acanny.cpp \
    achesscorners.cpp        acolor.cpp               acondens.cpp \
    acontourmoments.cpp      acontours.cpp            acontoursmatch.cpp \
    aconvhull.cpp            acreatecontourtree.cpp   adistancetransform.cpp \
    adrawing_regress.cpp     aeigenobjects.cpp        aemd.cpp \
    afilter.cpp              afloodfill.cpp           afundam.cpp \
    ahistograms.cpp          ahmmobs.cpp              ahoughtransform.cpp \
    aimage.cpp               aimgwarp.cpp             akalman.cpp \
    akmeans.cpp              amatchcontourtrees.cpp   amoments.cpp \
    amotiontemplates.cpp     amotseg.cpp              aoptflowhs.cpp \
    aoptflowlk.cpp           aoptflowpyrlk.cpp        aposit.cpp \
    apyrsegmentation.cpp     asnakes.cpp              asubdivisions.cpp \
    akdtree.cpp \
    atemplmatch.cpp          athresh.cpp      cvtest.cpp    tsysa.cpp

cvtest_LDADD = \
    $(top_builddir)/tests/cxts/libcxts.la  \
    $(top_builddir)/src/libcxcore.la       \
    $(top_builddir)/src/libcv.la           \
    $(top_builddir)/src/libcvaux.la        \
    $(top_builddir)/src/libhighgui.la
