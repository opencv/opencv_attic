lib_LTLIBRARIES = libopencv.la

libopencv_la_SOURCES = \
camshift.cpp                 cveigendecomposite.cpp  cvmorphing.cpp        \
cvabsdiff.cpp                cveigenobjectswrap.cpp  cvmotempl.cpp         \
cvaccum.cpp                  cveigenprojection.cpp   cvnorm.cpp            \
cvadapthresh.cpp             cvellipsefit.cpp        cvoptflowbm.cpp       \
cvaffine.cpp                 cvemd.cpp               cvoptflowhs.cpp       \
cvalloc.cpp                  cverror.cpp             cvoptflowlk.cpp       \
cvapprox.cpp                 cvfeatureselect.cpp     cvouttext.cpp         \
cvarithm.cpp                 cvfindhandregion.cpp    cvpersistence.cpp     \
cvarray.cpp                  cvfloodfill.cpp         cvpgh.cpp             \
cvcalccontrasthistogram.cpp  cvfundam.cpp            cvposit.cpp           \
cvcalccovarmatrix.cpp        cvgeometry.cpp          cvprecomp.cpp         \
cvcalceigenobjects.cpp       cvhistogram.cpp         cvprecorner.cpp       \
cvcalcimagehomography.cpp    cvhmm.cpp               cvprewarp.cpp         \
cvcalibinit.cpp              cvhmmobs.cpp            cvprint.cpp           \
cvcalibration.cpp            cvhmmwrap.cpp           cvproject.cpp         \
cvcamshift.cpp               cvhough.cpp             cvpyramids.cpp        \
cvcanny.cpp                  cvimage.cpp             cvpyrsegmentation.cpp \
cvcmp.cpp                    cvjacobieigens.cpp      cvrand.cpp            \
cvcolor.cpp                  cvkalman.cpp            cvrotcalipers.cpp     \
cvcondens.cpp                cvlinefit.cpp           cvsamplers.cpp        \
cvcontours.cpp               cvlines.cpp             cvscanlines.cpp       \
cvcontourtree.cpp            cvlkpyramid.cpp         cvshapedescr.cpp      \
cvconvert.cpp                cvlmeds.cpp             cvsmooth.cpp          \
cvconvhull.cpp               cvlogic.cpp             cvsnakes.cpp          \
cvcopy.cpp                   cvmatchcontours.cpp     cvsubdivision2d.cpp   \
cvcorner.cpp                 cvmathfuncs.cpp         cvsumpixels.cpp       \
cvcornersubpix.cpp           cvmatmul.cpp            cvsvd.cpp             \
cvcorrespond.cpp             cvmatrix64d.cpp         cvswitcher.cpp        \
cvcreatehandmask.cpp         cvmatrix.cpp            cvtables.cpp          \
cvdatastructs.cpp            cvmean.cpp              cvtemplmatch.cpp      \
cvderiv.cpp                  cvmeansdv.cpp           cvthresh.cpp          \
cvdetectwr.cpp               cvmineval.cpp           cvundistort.cpp       \
cvdistransform.cpp           cvminmaxloc.cpp         cvutils.cpp           \
cvdominants.cpp              cvmoments.cpp                                 \
cvdrawing.cpp                cvmorph.cpp

SOMAJOR = @MMAJOR@
SOMINOR = @MMINOR@
SOSUBMINOR = @MSUBMINOR@

# here you should add all header files, which contains the functionality
# of your shared lib (published for the users)
include_HEADERS =

# here you should add all header files, which shouldn't be installed
noinst_HEADERS =  \
_cvarr.h          _cvfuncn.h  _cvlist.h     _cvtables.h  _ipcv.h                  \
_cvdatastructs.h  _cvgeom.h   _cvmatrix.h   _cvutils.h   _optcv.h                 \
_cverror.h        _cv.h       _cvoptions.h  _cvvm.h

EXTRA_DIST = cv.rc resource.h

#libopencv_la_LDFLAGS = $(all_libraries) -no-undefined -version-info $(LT_CURRENT):$(LT_REVISION):$(LT_AGE) -release $(LT_RELEASE) @LDFLAGS@
libopencv_la_LDFLAGS = -no-undefined -version-info $(LT_CURRENT):$(LT_REVISION):$(LT_AGE) -release $(LT_RELEASE)

LIBS = -ldl -lpthread -lm

INCLUDES = -I. -I../include -I../..
CXXFLAGS = @CXXFLAGS@ -Wall

