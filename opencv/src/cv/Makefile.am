lib_LTLIBRARIES = libopencv.la

libopencv_la_SOURCES = camshift.cpp cvabsdiff.cpp cvaccum.cpp cvadapthresh.cpp cvaffine.cpp cvalloc.cpp cvapprox.cpp cvarithm.cpp cvarray.cpp cvblur.cpp cvcalccontrasthistogram.cpp cvcalccovarmatrix.cpp cvcalccovarmatrixex.cpp cvcalceigenobjects.cpp cvcalcimagehomography.cpp cvcalcprobdensity.cpp cvcalibinit.cpp cvcalibration.cpp cvcamshift.cpp cvcanny.cpp cvcolor.cpp cvcondens.cpp cvcontourarea.cpp cvcontours.cpp cvcontoursmoments.cpp cvcontourtree.cpp cvconvert.cpp cvconvgen.cpp cvconvhull.cpp cvcopy.cpp cvcorner.cpp cvcornersubpix.cpp cvcorrespond.cpp cvcreatehandmask.cpp cvdatastructs.cpp cvdetectwr.cpp cvdistransform.cpp cvdominants.cpp cvdrawing.cpp cveigendecomposite.cpp cveigenobjectswrap.cpp cveigenprojection.cpp cvellipsefit.cpp cvemd.cpp cverror.cpp cvfeatureselect.cpp cvfilterwr.cpp cvfindhandregion.cpp cvfindhandregiona.cpp cvfloodfill.cpp cvfundam.cpp cvgeometry.cpp cvgeometrywrap.cpp cvgesturerecognition.cpp cvhistogram.cpp cvhmm.cpp cvhmmobs.cpp cvhmmwrap.cpp cvhough.cpp cvhoughdiv.cpp cvhoughp.cpp cvhoughwrap.cpp cvimage.cpp cvinitconvol.cpp cvjacobieigens.cpp cvkalman.cpp cvlaplace.cpp cvlinefit.cpp cvlines.cpp cvlkpyramid.cpp cvlmeds.cpp cvlogic.cpp cvmatchcontours.cpp cvmathfuncs.cpp cvmatmul.cpp cvmatrix.cpp cvmatrix64d.cpp cvmaxrect.cpp cvmean.cpp cvmeansdv.cpp cvmineval.cpp cvminmaxloc.cpp cvmoments.cpp cvmorph.cpp cvmorphing.cpp cvmorphwrap.cpp cvmotempl.cpp cvmotseg.cpp cvnorm.cpp cvoptflowbm.cpp cvoptflowhs.cpp cvoptflowlk.cpp cvouttext.cpp cvpgh.cpp cvposit.cpp cvprecomp.cpp cvprecorner.cpp cvprewarp.cpp cvprint.cpp cvproject.cpp cvpyramids.cpp cvpyrsegmentation.cpp cvrand.cpp cvrotcalipers.cpp cvsamplers.cpp cvscanlines.cpp cvscharr.cpp cvshapedescr.cpp cvsnakes.cpp cvsubdivision2d.cpp cvsumpixels.cpp cvsvd.cpp cvswitcher.cpp cvtables.cpp cvtemplmatch.cpp cvthresh.cpp cvundistort.cpp cvutilitieswrap.cpp cvutils.cpp

SOMAJOR = @MMAJOR@
SOMINOR = @MMINOR@
SOSUBMINOR = @MSUBMINOR@

# here you should add all header files, which contains the functionality
# of your shared lib (published for the users)
include_HEADERS =

# here you should add all header files, which shouldn't be installed
noinst_HEADERS = _cv.h _cvarr.h _cvdatastructs.h _cverror.h _cvfuncn.h _cvgeom.h _cvlist.h _cvmatrix.h _cvmorphology.h _cvoptions.h _cvtables.h _cvutils.h _cvvm.h _cvwrap.h _ipcv.h _optcv.h

libopencv_la_LDFLAGS = $(all_libraries) -no-undefined -version-info $(SOMAJOR):$(SOMINOR):$(SOSUBMINOR) $(LDFLAGS)

# add here the libraries which have to be include
# for building the shared lib
#
#  please consider also the usage of the make environment variables
#  like
#  $(LIB_KDEUI), $(LIB_QT), $(LIB_HTML), etc.
#  instead of the hardcoded entries, which have placed
#  in here.
#  The following line is only a suggestion!
#
libopencv_la_LIBADD = 

LIBS += -ldl -lpthread -lm

INCLUDES += -I../include -I../_include
CFLAGS += -Wall @DEBUG@
CXXFLAGS += -Wall @DEBUG@

