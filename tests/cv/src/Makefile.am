noinst_PROGRAMS = cvtest

cvtest_SOURCES= \
aabsdiff.cpp            adistancetransform.cpp   amatchcontourtrees.cpp  apyramids.cpp        \
aaccum.cpp              adrawing.cpp             amathutils.cpp          apyrsegmentation.cpp \
aadaptthresh.cpp        adrawing_regress.cpp     amatrix2.cpp            asamplers.cpp        \
aapproxpoly.cpp         aeigenobjects.cpp        amatrix.cpp             asequence.cpp        \
aarithm.cpp             aemd.cpp                 ameanshift.cpp          asnakes.cpp          \
aarrayiterator.cpp      aerrhandle.cpp           amemorytest.cpp         asobel.cpp           \
abackproject.cpp        afitellipse.cpp          aminarearect.cpp        astoragearray.cpp    \
acalccontrasthist.cpp   afitlines.cpp            amineval.cpp            asubdivisions.cpp    \
acalculate.cpp          afloodfill8.cpp          amoments.cpp            asvd.cpp             \
acameracalibration.cpp  afloodfill.cpp           amorphology.cpp         atemplmatch.cpp      \
acamshift.cpp           amotiontemplates.cpp     athresh.cpp                                  \
acanny.cpp              ahaar.cpp                amotseg.cpp             atree.cpp            \
achesscorners.cpp       ahistogram.cpp           anodeiterator.cpp       atreeiterator.cpp    \
acondens.cpp            ahistograms.cpp          anorm.cpp               ats.cpp              \
acontourcollection.cpp  ahmmobs.cpp              anormmask.cpp           aundistort.cpp       \
acontourmoments.cpp     ahoughtransform.cpp      aoperations.cpp         canny.cpp            \
acontours.cpp           aimage.cpp               aoptflowhs.cpp          cvtest.cpp           \
acontoursmatch.cpp      aimagestatistics.cpp     aoptflowlk.cpp          tsysa.cpp            \
aconvert.cpp            akalman.cpp              aoptflowpyrlk.cpp       ucontours.cpp        \
aconvhull.cpp           akmeans.cpp              apixelaccess.cpp        ugraphics.cpp        \
acorner.cpp             alogic.cpp               aposit.cpp              uiplutils.cpp        \
acreatecontourtree.cpp  amaskaccum.cpp           aprecorner.cpp          ureadfile.cpp

# here you should add all header files, which contains the functionality
# of your shared lib (published for the users)
include_HEADERS =

# here you should add all header files, which shouldn't be installed
noinst_HEADERS = cvtest.h

EXTRA_DIST = aeigenobjects.inc  amatrix.inc  apixelaccess.inc makefile.vc makefile.icl makefile.bcc makefile.gcc cvtest.dsp

INCLUDES = -I. -I../include -I../../../otherlibs/highgui -I../../trs/include -I../../../cv/include -I../../../cv/src -I../../../cvaux/include

cvtest_LDADD = -L../../../cv/src -lopencv -L../../../otherlibs/highgui -lhighgui -L../../trs/src -ltrs -L../../../cvaux/src -lcvaux

CFLAGS = @CFLAGS@ @DEBUG@
CXXFLAGS = @CXXFLAGS@ @DEBUG@
