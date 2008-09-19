# use the default c++ flags
AM_CXXFLAGS=@DEF_CXXFLAGS@

INCLUDES = -I. -I$(top_srcdir)/cv/include -I$(top_srcdir)/cxcore/include -I$(top_srcdir)

noinst_HEADERS     = _cv.h _cvgeom.h _cvimgproc.h _cvipp.h _cvlist.h _cvmatrix.h \
    _cvkdtree.hpp

noinst_LTLIBRARIES = lib_cv.la
lib_LTLIBRARIES    = libcv.la

# convenience libraries
lib_cv_la_SOURCES = \
    cvaccum.cpp cvadapthresh.cpp cvapprox.cpp cvcalccontrasthistogram.cpp \
    cvcalcimagehomography.cpp cvcalibinit.cpp cvcalibration.cpp \
    cvcamshift.cpp cvcanny.cpp cvcolor.cpp cvcondens.cpp cvcontours.cpp \
    cvcontourtree.cpp cvconvhull.cpp cvcorner.cpp \
    cvcornersubpix.cpp cvderiv.cpp cvdistransform.cpp cvdominants.cpp \
    cvemd.cpp cvfeatureselect.cpp cvfilter.cpp cvfloodfill.cpp cvfundam.cpp \
    cvgeometry.cpp cvhaar.cpp cvhistogram.cpp cvhough.cpp cvimgwarp.cpp \
    cvinpaint.cpp cvkalman.cpp cvlinefit.cpp cvlkpyramid.cpp cvmatchcontours.cpp \
    cvmoments.cpp cvmorph.cpp cvmotempl.cpp cvoptflowbm.cpp cvoptflowhs.cpp \
    cvoptflowlk.cpp cvpgh.cpp cvposit.cpp cvprecomp.cpp cvpyramids.cpp \
    cvpyrsegmentation.cpp cvrotcalipers.cpp cvsamplers.cpp cvsegmentation.cpp \
    cvshapedescr.cpp cvsmooth.cpp cvsnakes.cpp cvstereobm.cpp cvstereogc.cpp \
    cvsubdivision2d.cpp cvsumpixels.cpp cvsurf.cpp cvswitcher.cpp cvtables.cpp \
    cvtemplmatch.cpp cvthresh.cpp cvundistort.cpp cvutils.cpp cvkdtree.cpp
lib_cv_la_LDFLAGS = -no-undefined @LDFLAGS@

# real library
libcv_la_SOURCES = dummy.cpp
libcv_la_LDFLAGS = -no-undefined -version-info @LT_VERSION@ @LDFLAGS@
libcv_la_LIBADD = lib_cv.la \
    $(top_builddir)/cxcore/src/libcxcore.la \
    @LTLIBOBJS@
