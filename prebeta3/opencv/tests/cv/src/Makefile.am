noinst_PROGRAMS = cvla

cvla_SOURCES = aabsdiff.cpp aaccum.cpp aadaptthresh.cpp aapproxpoly.cpp aarithm.cpp aarrayiterator.cpp abackproject.cpp acalccontrasthist.cpp acalculate.cpp acameracalibration.cpp acamshift.cpp acanny.cpp achesscorners.cpp acondens.cpp acontourcollection.cpp acontourmoments.cpp acontours.cpp acontoursmatch.cpp aconvert.cpp aconvhull.cpp acorner.cpp acreatecontourtree.cpp adistancetransform.cpp adrawing.cpp adrawing_regress.cpp aeigenobjects.cpp aemd.cpp aerrhandle.cpp afitellipse.cpp afitlines.cpp afloodfill.cpp afloodfill8.cpp agesturerecognition.cpp ahistogram.cpp ahistograms.cpp ahmmobs.cpp ahoughtransform.cpp aimage.cpp aimagestatistics.cpp akalman.cpp akmeans.cpp alaplace.cpp alogic.cpp amaskaccum.cpp amatchcontourtrees.cpp amathutils.cpp amatrix.cpp amatrix2.cpp ameanshift.cpp amemorytest.cpp aminarearect.cpp amineval.cpp amoments.cpp amorphology.cpp amotiontemplates.cpp amotseg.cpp anodeiterator.cpp anorm.cpp anormmask.cpp aoperations.cpp aoptflowhs.cpp aoptflowlk.cpp aoptflowpyrlk.cpp apixelaccess.cpp aposit.cpp aprecorner.cpp apyramids.cpp apyrsegmentation.cpp asamplers.cpp ascharr.cpp asequence.cpp asnakes.cpp asobel.cpp astoragearray.cpp asubdivisions.cpp asvd.cpp atemplmatch.cpp athresh.cpp atree.cpp atreeiterator.cpp aundistort.cpp canny.cpp tsysa.cpp ucontours.cpp ugraphics.cpp uiplutils.cpp ureadfile.cpp

# here you should add all header files, which contains the functionality
# of your shared lib (published for the users)
include_HEADERS =

# here you should add all header files, which shouldn't be installed
noinst_HEADERS = 

EXTRA_DIST =   aeigenobjects.inc amatrix.inc apixelaccess.inc aabsdiff.cpp aaccum.cpp aadaptthresh.cpp aapproxpoly.cpp aarithm.cpp aarrayiterator.cpp abackproject.cpp acalccontrasthist.cpp acalculate.cpp acameracalibration.cpp acamshift.cpp acanny.cpp achesscorners.cpp acondens.cpp acontourcollection.cpp acontourmoments.cpp acontours.cpp acontoursmatch.cpp aconvert.cpp aconvhull.cpp acorner.cpp acreatecontourtree.cpp adistancetransform.cpp adrawing.cpp adrawing_regress.cpp aeigenobjects.cpp aemd.cpp aerrhandle.cpp afitellipse.cpp afitlines.cpp afloodfill.cpp afloodfill8.cpp agesturerecognition.cpp ahistogram.cpp ahistograms.cpp ahmmobs.cpp ahoughtransform.cpp aimage.cpp aimagestatistics.cpp akalman.cpp akmeans.cpp alaplace.cpp alogic.cpp amaskaccum.cpp amatchcontourtrees.cpp amathutils.cpp amatrix.cpp amatrix2.cpp ameanshift.cpp amemorytest.cpp aminarearect.cpp amineval.cpp amoments.cpp amorphology.cpp amotiontemplates.cpp amotseg.cpp anodeiterator.cpp anorm.cpp anormmask.cpp aoperations.cpp aoptflowhs.cpp aoptflowlk.cpp aoptflowpyrlk.cpp apixelaccess.cpp aposit.cpp aprecorner.cpp apyramids.cpp apyrsegmentation.cpp asamplers.cpp ascharr.cpp asequence.cpp asnakes.cpp asobel.cpp astoragearray.cpp asubdivisions.cpp asvd.cpp atemplmatch.cpp athresh.cpp atree.cpp atreeiterator.cpp aundistort.cpp canny.cpp tsysa.cpp ucontours.cpp ugraphics.cpp uiplutils.cpp ureadfile.cpp

INCLUDES += -I. -I../include -I../../../otherlibs/highgui -I../../ats/include -I../../trs/include -I../../../cv/include -I../../../cv/src -I../../../cvaux/include -I../../../otherlibs/_Mkl/include

cvla_LDADD = -L../../../cv/src -lopencv -L../../../otherlibs/highgui -lhighgui -L../../trs/src -ltrs -L../../ats/src -lats -L../../../cvaux/src -lcvaux -liplpx

CFLAGS += @DEBUG@
CXXFLAGS += @DEBUG@

#cvla_LIBS = 
