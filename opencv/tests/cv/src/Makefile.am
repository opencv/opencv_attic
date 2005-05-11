INCLUDES =                                \
	-I.                               \
	-I$(top_srcdir)/tests/trs         \
	-I$(top_srcdir)/tests/cxts        \
	-I$(top_srcdir)/otherlibs/highgui \
	-I$(top_srcdir)/cxcore/include    \
	-I$(top_srcdir)/cv/include        \
	-I$(top_srcdir)/cvaux/include

check_PROGRAMS = cvtest

cvtest_SOURCES = \
    aeigenobjects.inc apixelaccess.inc cvtest.h \
    aaccum.cpp aadaptthresh.cpp aapproxpoly.cpp acalccontrasthist.cpp \
    acameracalibration.cpp acamshift.cpp acanny.cpp achesscorners.cpp \
    acolor.cpp acondens.cpp acontourcollection.cpp acontourmoments.cpp \
    acontours.cpp acontoursmatch.cpp aconvhull.cpp acreatecontourtree.cpp \
    adistancetransform.cpp adrawing.cpp adrawing_regress.cpp \
    aeigenobjects.cpp aemd.cpp aerrhandle.cpp afilter.cpp afitellipse.cpp \
    afitlines.cpp afloodfill.cpp agesturerecognition.cpp \
    ahistograms.cpp ahmmobs.cpp ahoughtransform.cpp aimage.cpp \
    aimgwarp.cpp akalman.cpp akmeans.cpp amatchcontourtrees.cpp \
    ameanshift.cpp amemorytest.cpp aminarearect.cpp amoments.cpp \
    amotiontemplates.cpp amotseg.cpp aoptflowhs.cpp aoptflowlk.cpp \
    aoptflowpyrlk.cpp apixelaccess.cpp aposit.cpp apyrsegmentation.cpp \
    asamplers.cpp asnakes.cpp asubdivisions.cpp atemplmatch.cpp \
    athresh.cpp ats.cpp aundistort.cpp canny.cpp cvtest.cpp \
    tsysa.cpp ucontours.cpp ugraphics.cpp uiplutils.cpp ureadfile.cpp

cvtest_LDADD =                                          \
	$(top_builddir)/cv/src/libcv.la                 \
	$(top_builddir)/cxcore/src/libcxcore.la         \
	$(top_builddir)/tests/trs/libtrs.a              \
	$(top_builddir)/tests/cxts/libcxts.a            \
	$(top_builddir)/cvaux/src/libcvaux.la           \
	$(top_builddir)/otherlibs/highgui/libhighgui.la

EXTRA_DIST = cvtest.dsp cvtest.vcproj
