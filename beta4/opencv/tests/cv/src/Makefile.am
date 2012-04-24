noinst_PROGRAMS = cvtest

cvtest_SOURCES = aaccum.cpp aadaptthresh.cpp aapproxpoly.cpp \
acalccontrasthist.cpp acameracalibration.cpp acamshift.cpp \
acanny.cpp achesscorners.cpp acondens.cpp acontourcollection.cpp \
acontourmoments.cpp acontours.cpp acontoursmatch.cpp aconvhull.cpp \
acorner.cpp acreatecontourtree.cpp adistancetransform.cpp \
aeigenobjects.cpp aemd.cpp aerrhandle.cpp afitellipse.cpp \
afitlines.cpp afloodfill.cpp afloodfill8.cpp agesturerecognition.cpp \
ahaar.cpp ahistograms.cpp ahmmobs.cpp ahoughtransform.cpp aimage.cpp \
akalman.cpp akmeans.cpp amaskaccum.cpp amatchcontourtrees.cpp \
ameanshift.cpp amemorytest.cpp aminarearect.cpp amineval.cpp \
amoments.cpp amorphology.cpp amotiontemplates.cpp amotseg.cpp \
aoptflowhs.cpp aoptflowlk.cpp aoptflowpyrlk.cpp apixelaccess.cpp \
aposit.cpp aprecorner.cpp apyramids.cpp apyrsegmentation.cpp \
asamplers.cpp asnakes.cpp asobel.cpp asubdivisions.cpp \
atemplmatch.cpp athresh.cpp ats.cpp aundistort.cpp canny.cpp \
cvtest.cpp tsysa.cpp ucontours.cpp ugraphics.cpp uiplutils.cpp ureadfile.cpp

# here you should add all header files, which contains the functionality
# of your shared lib (published for the users)
include_HEADERS =

# here you should add all header files, which shouldn't be installed
noinst_HEADERS = aeigenobjects.inc apixelaccess.inc cvtest.h

EXTRA_DIST = cvtest.dsp cvtest.vcproj

INCLUDES = -I. -I../../trs -I../../../otherlibs/highgui -I../../../cxcore/include -I../../../cv/include -I../../../cv/src -I../../../cvaux/include

#cvtest_LDADD = -L../../../cv/src -lcv -L../../../otherlibs/highgui -lhighgui -L../../trs -ltrs -L../../../cvaux/src -lcvaux -L../../../cxcore/src -Lcxcore
cvtest_LDADD = -L../../../cv/src -lcv -L../../../cxcore/src -lcxcore -L../../trs -ltrs -L../../../cvaux/src -lcvaux -L../../../otherlibs/highgui -lhighgui



