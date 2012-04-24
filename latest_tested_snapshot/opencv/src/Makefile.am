# This Makefile now builds everything in the src directory

# use platform native lapack libraries or the one shipped with OpenCV
if NATIVE_LAPACK
  LAPACK = @LAPACK_LIBS@
  LAPACK_INCL =
else
  LAPACK = $(top_builddir)/3rdparty/lib_clapack.la
  LAPACK_INCL = -I$(top_srcdir)/3rdparty/include
endif

AM_CPPFLAGS = @OPTIMIZATION_CPPFLAGS@ @DEBUG_CPPFLAGS@ -I$(top_srcdir)/include/opencv
AM_CXXFLAGS = @OPTIMIZATION_CXXFLAGS@ @DEBUG_CXXFLAGS@

# =========================================================================

# sources for other platforms and conditionally compiled sources
EXTRA_DIST = \
  \
  CMakeLists.txt               \
  cv/CMakeLists.txt            \
  cvaux/CMakeLists.txt         \
  cxcore/CMakeLists.txt        \
  highgui/CMakeLists.txt       \
  ml/CMakeLists.txt            \
  \
  highgui/makeswig.sh          \
  \
  highgui/cvcap_cmu.cpp        \
  highgui/cvcap_tyzx.cpp       \
  highgui/cvcap_vfw.cpp        \
  highgui/cvcap_mil.cpp        \
  highgui/cvcap_images.cpp     \
  highgui/window_w32.cpp       \
  \
  highgui/cvcap_dc1394.cpp     \
  highgui/cvcap_dc1394_v2.cpp  \
  highgui/cvcap_dshow.cpp      \
  highgui/cvcap_ffmpeg.cpp     \
  highgui/cvcap_gstreamer.cpp  \
  highgui/cvcap_libv4l.cpp     \
  highgui/cvcap_qt.cpp         \
  highgui/cvcap_unicap.cpp     \
  highgui/cvcap_v4l.cpp        \
  highgui/cvcap_w32.cpp        \
  highgui/cvcap_xine.cpp       \
  highgui/gstappsink.cpp       \
  highgui/window_carbon.cpp    \
  highgui/window_gtk.cpp       \
  highgui/grfmt_imageio.cpp


# =========================================================================

# headers that are needed at compile time of OpenCV only
noinst_HEADERS     = \
  cxcore/_cxcore.h         \
  \
  cv/_cv.h                 \
  cv/_cvfeaturetree.h      \
  cv/_cvgeom.h             \
  cv/_cvimgproc.h          \
  cv/_cvipp.h              \
  cv/_cvkdtree.hpp         \
  cv/_cvlist.h             \
  cv/_cvmatrix.h           \
  cv/_cvmodelest.h         \
  \
  cvaux/_cvaux.h           \
  cvaux/_cvfacedetection.h \
  cvaux/_cvvectrack.h      \
  cvaux/_cvvm.h            \
  \
  cvaux/cvface.h           \
  cvaux/cvfacedetection.h  \
  cvaux/cvfacetemplate.h   \
  \
  ml/_ml.h                 \
  \
  highgui/_highgui.h       \
  highgui/bitstrm.h        \
  highgui/grfmt_base.h     \
  highgui/grfmt_bmp.h      \
  highgui/grfmt_exr.h      \
  highgui/grfmt_imageio.h  \
  highgui/grfmt_jpeg.h     \
  highgui/grfmt_jpeg2000.h \
  highgui/grfmt_png.h      \
  highgui/grfmt_pxm.h      \
  highgui/grfmt_sunras.h   \
  highgui/grfmt_tiff.h     \
  highgui/grfmts.h         \
  highgui/gstappsink.h	   \
  highgui/utils.h

noinst_LTLIBRARIES = lib_cxcore.la lib_cv.la lib_cvaux.la lib_ml.la lib_highgui.la

lib_LTLIBRARIES    = libcxcore.la  libcv.la  libcvaux.la  libml.la  libhighgui.la

# =========================================================================

# convenience library for cxcore
lib_cxcore_la_SOURCES = \
  cxcore/cxalloc.cpp \
  cxcore/cxarithm.cpp \
  cxcore/cxarray.cpp \
  cxcore/cxconvert.cpp \
  cxcore/cxcopy.cpp \
  cxcore/cxdatastructs.cpp \
  cxcore/cxdrawing.cpp \
  cxcore/cxdxt.cpp \
  cxcore/cximage.cpp \
  cxcore/cxlapack.cpp \
  cxcore/cxmathfuncs.cpp \
  cxcore/cxmatmul.cpp \
  cxcore/cxmatrix.cpp \
  cxcore/cxpersistence.cpp \
  cxcore/cxrand.cpp \
  cxcore/cxstat.cpp \
  cxcore/cxsystem.cpp \
  cxcore/cxtables.cpp
lib_cxcore_la_CPPFLAGS = $(LAPACK_INCL) $(AM_CPPFLAGS)
lib_cxcore_la_CXXFLAGS = @SIMD_FLAGS@ @OPENMP_CXXFLAGS@ $(AM_CXXFLAGS)
lib_cxcore_la_LDFLAGS  = -no-undefined @LDFLAGS@

# real library for cxcore
libcxcore_la_SOURCES = cxcore/cxprecomp.cpp
libcxcore_la_LDFLAGS = -no-undefined -Wc,@OPENMP_CXXFLAGS@ -version-info @LT_VERSION@ @LDFLAGS@
libcxcore_la_LIBADD  = \
  lib_cxcore.la \
  $(LAPACK) \
  @LTLIBOBJS@

# =========================================================================

# convenience library for cv
lib_cv_la_SOURCES = \
  cv/cvaccum.cpp \
  cv/cvapprox.cpp \
  cv/cvcalccontrasthistogram.cpp \
  cv/cvcalcimagehomography.cpp \
  cv/cvcalibinit.cpp \
  cv/cvcalibration.cpp \
  cv/cvcamshift.cpp \
  cv/cvcanny.cpp \
  cv/cvcascadedetect.cpp \
  cv/cvcolor.cpp \
  cv/cvcondens.cpp \
  cv/cvcontours.cpp \
  cv/cvcontourtree.cpp \
  cv/cvconvhull.cpp \
  cv/cvcorner.cpp \
  cv/cvcornersubpix.cpp \
  cv/cvderiv.cpp \
  cv/cvdistransform.cpp \
  cv/cvdominants.cpp \
  cv/cvemd.cpp \
  cv/cvfeatureselect.cpp \
  cv/cvfeaturetree.cpp \
  cv/cvfilter.cpp \
  cv/cvfloodfill.cpp \
  cv/cvfundam.cpp \
  cv/cvgeometry.cpp \
  cv/cvhaar.cpp \
  cv/cvhistogram.cpp \
  cv/cvhough.cpp \
  cv/cvimgwarp.cpp \
  cv/cvinpaint.cpp \
  cv/cvkalman.cpp \
  cv/cvkdtree.cpp \
  cv/cvlinefit.cpp \
  cv/cvlkpyramid.cpp \
  cv/cvlsh.cpp \
  cv/cvmatchcontours.cpp \
  cv/cvmodelest.cpp \
  cv/cvmoments.cpp \
  cv/cvmorph.cpp \
  cv/cvmotempl.cpp \
  cv/cvmser.cpp \
  cv/cvoptflowbm.cpp \
  cv/cvoptflowgf.cpp \
  cv/cvoptflowhs.cpp \
  cv/cvoptflowlk.cpp \
  cv/cvpgh.cpp \
  cv/cvposit.cpp \
  cv/cvpyramids.cpp \
  cv/cvpyrsegmentation.cpp \
  cv/cvrotcalipers.cpp \
  cv/cvsamplers.cpp \
  cv/cvsegmentation.cpp \
  cv/cvshapedescr.cpp \
  cv/cvsmooth.cpp \
  cv/cvsnakes.cpp \
  cv/cvspilltree.cpp \
  cv/cvstardetector.cpp \
  cv/cvstereobm.cpp \
  cv/cvstereogc.cpp \
  cv/cvsubdivision2d.cpp \
  cv/cvsumpixels.cpp \
  cv/cvsurf.cpp \
  cv/cvtables.cpp \
  cv/cvtemplmatch.cpp \
  cv/cvthresh.cpp \
  cv/cvtriangulate.cpp \
  cv/cvundistort.cpp \
  cv/cvutils.cpp
lib_cv_la_CXXFLAGS = @SIMD_FLAGS@ @OPENMP_CXXFLAGS@ $(AM_CXXFLAGS)
lib_cv_la_LDFLAGS = -no-undefined @LDFLAGS@

# real library for cv
libcv_la_SOURCES = cv/cvprecomp.cpp
libcv_la_LDFLAGS = -no-undefined -Wc,@OPENMP_CXXFLAGS@ -version-info @LT_VERSION@ @LDFLAGS@
libcv_la_LIBADD = \
  lib_cv.la \
  libcxcore.la \
  @LTLIBOBJS@

# =========================================================================

# convenience library for cvaux
lib_cvaux_la_SOURCES = \
  cvaux/camshift.cpp \
  cvaux/cv3dtracker.cpp \
  cvaux/cvadaptiveskindetector.cpp \
  cvaux/cvaux.cpp \
  cvaux/cvauxutils.cpp \
  cvaux/cvba.cpp \
  cvaux/cvbgfg_acmmm2003.cpp \
  cvaux/cvbgfg_codebook.cpp \
  cvaux/cvbgfg_common.cpp \
  cvaux/cvbgfg_gaussmix.cpp \
  cvaux/cvcalibfilter.cpp \
  cvaux/cvclique.cpp \
  cvaux/cvcorrespond.cpp \
  cvaux/cvcorrimages.cpp \
  cvaux/cvcreatehandmask.cpp \
  cvaux/cvdpstereo.cpp \
  cvaux/cveigenobjects.cpp \
  cvaux/cvepilines.cpp \
  cvaux/cvface.cpp \
  cvaux/cvfacedetection.cpp \
  cvaux/cvfacetemplate.cpp \
  cvaux/cvfindface.cpp \
  cvaux/cvfindhandregion.cpp \
  cvaux/cvfuzzymeanshifttracker.cpp \
  cvaux/cvhmm.cpp \
  cvaux/cvhmm1d.cpp \
  cvaux/cvhmmobs.cpp \
  cvaux/cvhog.cpp \
  cvaux/cvlcm.cpp \
  cvaux/cvlee.cpp \
  cvaux/cvlevmar.cpp \
  cvaux/cvlevmarprojbandle.cpp \
  cvaux/cvlevmartrif.cpp \
  cvaux/cvlines.cpp \
  cvaux/cvlmeds.cpp \
  cvaux/cvmat.cpp \
  cvaux/cvmorphcontours.cpp \
  cvaux/cvmorphing.cpp \
  cvaux/cvoctree.cpp \
  cvaux/cvplanardetect.cpp \
  cvaux/cvprewarp.cpp \
  cvaux/cvscanlines.cpp \
  cvaux/cvsegment.cpp \
  cvaux/cvselfsimilarity.cpp \
  cvaux/cvspinimages.cpp \
  cvaux/cvsubdiv2.cpp \
  cvaux/cvtexture.cpp \
  cvaux/cvtrifocal.cpp \
  cvaux/cvvecfacetracking.cpp \
  cvaux/cvvideo.cpp \
  cvaux/decomppoly.cpp \
  cvaux/enmin.cpp \
  cvaux/extendededges.cpp \
  \
  cvaux/vs/bgfg_estimation.cpp \
  cvaux/vs/blobtrackanalysis.cpp \
  cvaux/vs/blobtrackanalysishist.cpp \
  cvaux/vs/blobtrackanalysisior.cpp \
  cvaux/vs/blobtrackanalysistrackdist.cpp \
  cvaux/vs/blobtrackgen1.cpp \
  cvaux/vs/blobtrackgenyml.cpp \
  cvaux/vs/blobtrackingauto.cpp \
  cvaux/vs/blobtrackingcc.cpp \
  cvaux/vs/blobtrackingccwithcr.cpp \
  cvaux/vs/blobtrackingkalman.cpp \
  cvaux/vs/blobtrackinglist.cpp \
  cvaux/vs/blobtrackingmsfg.cpp \
  cvaux/vs/blobtrackingmsfgs.cpp \
  cvaux/vs/blobtrackpostprockalman.cpp \
  cvaux/vs/blobtrackpostproclinear.cpp \
  cvaux/vs/blobtrackpostproclist.cpp \
  cvaux/vs/enteringblobdetection.cpp \
  cvaux/vs/enteringblobdetectionreal.cpp \
  cvaux/vs/testseq.cpp
lib_cvaux_la_CPPFLAGS = -I$(top_srcdir)/src/cvaux -I$(top_srcdir)/src/cv $(AM_CPPFLAGS)
lib_cvaux_la_CXXFLAGS = @SIMD_FLAGS@ @OPENMP_CXXFLAGS@ $(AM_CXXFLAGS)
lib_cvaux_la_LDFLAGS  = -no-undefined @LDFLAGS@

# real library for cvaux
libcvaux_la_CPPFLAGS = -I$(top_srcdir)/src/cv $(AM_CPPFLAGS)
libcvaux_la_SOURCES  = cvaux/cvauxprecomp.cpp
libcvaux_la_LDFLAGS  = -no-undefined -Wc,@OPENMP_CXXFLAGS@ -version-info @LT_VERSION@ @LDFLAGS@
libcvaux_la_LIBADD   = \
  lib_cvaux.la \
  libcxcore.la \
  libcv.la \
  @LTLIBOBJS@

# =========================================================================

# convenience library for ml
lib_ml_la_SOURCES = \
  ml/ml_inner_functions.cpp \
  ml/ml.cpp \
  ml/mlann_mlp.cpp \
  ml/mlboost.cpp \
  ml/mlcnn.cpp \
  ml/mldata.cpp \
  ml/mlem.cpp \
  ml/mlertrees.cpp \
  ml/mlestimate.cpp \
  ml/mlknearest.cpp \
  ml/mlnbayes.cpp \
  ml/mlrtrees.cpp \
  ml/mlsvm.cpp \
  ml/mltestset.cpp \
  ml/mltree.cpp
lib_ml_la_CXXFLAGS = @SIMD_FLAGS@ @OPENMP_CXXFLAGS@ $(AM_CXXFLAGS)
lib_ml_la_LDFLAGS = -no-undefined @LDFLAGS@

# real library for ml
libml_la_SOURCES = dummy.cpp
libml_la_LDFLAGS = -no-undefined -Wc,@OPENMP_CXXFLAGS@ -version-info @LT_VERSION@ @LDFLAGS@
libml_la_LIBADD = \
  lib_ml.la \
  libcxcore.la \
  $(top_builddir)/3rdparty/lib_flann.la \
  @LTLIBOBJS@

# =========================================================================

# conditional sources

if BUILD_DC1394
TMP_DC1394=highgui/cvcap_dc1394.cpp
else
TMP_DC1394=
endif

if BUILD_DC1394_2
TMP_DC1394=highgui/cvcap_dc1394_v2.cpp
else
TMP_DC1394=
endif

if BUILD_V4L
TMP_V4L=highgui/cvcap_v4l.cpp
else
TMP_V4L=
endif

if BUILD_UNICAP
TMP_UNICAP=highgui/cvcap_unicap.cpp
else
TMP_UNICAP=
endif

if BUILD_FFMPEG
TMP_FFMPEG=highgui/cvcap_ffmpeg.cpp
else
TMP_FFMPEG=
endif

if BUILD_XINE
TMP_XINE=highgui/cvcap_xine.cpp
else
TMP_XINE=
endif

if BUILD_GSTREAMER
TMP_GSTREAMER=highgui/cvcap_gstreamer.cpp highgui/gstappsink.cpp
else
TMP_GSTREAMER=
endif

if BUILD_QUICKTIME
TMP_QT=highgui/cvcap_qt.cpp
else
TMP_QT=
endif

if BUILD_CARBON
TMP_CARBON=highgui/window_carbon.cpp
else
TMP_CARBON=
endif

if BUILD_GTK
TMP_GTK=highgui/window_gtk.cpp
else
TMP_GTK=
endif

if BUILD_IMAGEIO
TMP_IMAGEIO=highgui/grfmt_imageio.cpp
else
TMP_IMAGEIO=
endif

# -------------------------------------------------------------------------

# convenience library
lib_highgui_la_SOURCES = \
  highgui/bitstrm.cpp        \
  highgui/grfmt_base.cpp     \
  highgui/grfmt_bmp.cpp      \
  highgui/grfmt_exr.cpp      \
  highgui/grfmt_jpeg.cpp     \
  highgui/grfmt_jpeg2000.cpp \
  highgui/grfmt_png.cpp      \
  highgui/grfmt_pxm.cpp      \
  highgui/grfmt_sunras.cpp   \
  highgui/grfmt_tiff.cpp     \
  highgui/image.cpp          \
  highgui/loadsave.cpp       \
  highgui/utils.cpp          \
  highgui/cvcap.cpp          \
  highgui/cvcap_images.cpp   \
  highgui/window.cpp         \
  $(TMP_DC1394)     \
  $(TMP_V4L)        \
  $(TMP_UNICAP)     \
  $(TMP_FFMPEG)     \
  $(TMP_XINE)       \
  $(TMP_GSTREAMER)  \
  $(TMP_QT)         \
  $(TMP_CARBON)     \
  $(TMP_GTK)        \
  $(TMP_IMAGEIO)

lib_highgui_la_CXXFLAGS = \
  @GTHREAD_CFLAGS@ @GTK_CFLAGS@ \
  @GSTREAMER_CFLAGS@ @QUICKTIME_CFLAGS@ @CARBON_CFLAGS@    \
  @UNICAP_PACKAGE_CFLAGS@ @UCIL_PACKAGE_CFLAGS@ $(AM_CXXFLAGS)
lib_highgui_la_LDFLAGS = -no-undefined @LDFLAGS@

# real library
libhighgui_la_SOURCES = highgui/precomp.cpp
libhighgui_la_LDFLAGS = -no-undefined -Wc,@OPENMP_CXXFLAGS@ -version-info @LT_VERSION@ @LDFLAGS@
libhighgui_la_LIBADD  = \
    lib_highgui.la \
    libcxcore.la   \
    libcv.la \
    @GTHREAD_LIBS@ @GTK_LIBS@ @GSTREAMER_LIBS@ @IMAGELIBS@ @FFMPEGLIBS@   \
    @IEEE1394LIBS@ @QUICKTIME_LIBS@ @CARBON_LIBS@ @XINE_LIBS@ @LTLIBOBJS@ \
    @UNICAP_PACKAGE_LIBS@ @UCIL_PACKAGE_LIBS@
