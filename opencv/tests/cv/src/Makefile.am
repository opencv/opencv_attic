INCLUDES = -I. -I../../trs -I../../cxts -I../../../otherlibs/highgui -I../../../cxcore/include -I../../../cv/include -I../../../cv/src -I../../../cvaux/include

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
cvtest_LDADD = \
    -L$(top_builddir)/cv/src -lcv \
    -L$(top_builddir)/cxcore/src -lcxcore \
    -L$(top_builddir)/tests/trs -ltrs \
    -L$(top_builddir)/tests/cxts -lcxts \
    -L$(top_builddir)/cvaux/src -lcvaux \
    -L$(top_builddir)/otherlibs/highgui -lhighgui

EXTRA_DIST = cvtest.dsp cvtest.vcproj
