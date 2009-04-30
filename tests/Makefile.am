# This Makefile compiles, links and invokes all tests
# Python and Octave tests are only run if python or octave resp. have been --enable[d] in ./configure


check_LTLIBRARIES = libcxts.la
check_PROGRAMS    = cxcoretest cvtest mltest
check_SCRIPTS     = $(TMP_OCTAVE_FILES) $(TMP_PYTHON_FILES)
TESTS             = $(check_PROGRAMS) $(check_SCRIPTS)

AM_LDFLAGS        = @OPENMP_CXXFLAGS@

clean-local:
	-rm -rf *.lst *.sum python/*.pyc python/*.works python/highgui/*.pyc python/highgui/*.works


TESTS_ENVIRONMENT = \
    top_srcdir=$(top_srcdir) \
    top_builddir=$(top_builddir) \
    LD_LIBRARY_PATH=$(top_builddir)/src/.libs:$(LD_LIBRARY_PATH) \
    DYLD_LIBRARY_PATH=$(top_builddir)/src/.libs:$(LD_LIBRARY_PATH) \
    OCTAVEPATH=$(top_builddir)/interfaces/swig/octave \
    PYTHONPATH=$(top_srcdir)/tests/python:$(top_srcdir)/tests/python/highgui:$(top_srcdir)/interfaces/swig/python:$(top_builddir)/interfaces/swig/python/.libs


AM_CPPFLAGS = \
    -I$(top_srcdir)/tests/cxts        \
    -I$(top_srcdir)/include/opencv


EXTRA_DIST = \
  CMakeLists.txt \
  cv/CMakeLists.txt \
  cxcore/CMakeLists.txt \
  cxts/CMakeLists.txt \
  ml/CMakeLists.txt \
  \
  cv/testdata \
  cxcore/src/precomp.cpp \
  cxts/precomp.cpp \
  \
  ml/src/precomp.cpp \
  ml/Readme.txt \
  \
  octave \
  python/testdata

noinst_HEADERS = \
  cxts/_cxts.h                  \
  cxts/cxts.h                   \
  \
	cxcore/src/cxcoretest.h       \
  \
  cv/src/aeigenobjects.inc      \
  cv/src/cvtest.h               \
  \
  ml/src/mltest.h


libcxts_la_SOURCES = \
  cxts/cxts.cpp \
  cxts/cxts_arrtest.cpp \
  cxts/cxts_math.cpp
libcxts_la_LIBADD = \
  $(top_builddir)/src/libcxcore.la


cxcoretest_SOURCES =    \
	cxcore/src/aarithm.cpp         \
	cxcore/src/adatastruct.cpp     \
	cxcore/src/adxt.cpp            \
	cxcore/src/amath.cpp           \
	cxcore/src/asolvepoly.cpp      \
	cxcore/src/cxcoretest_main.cpp
cxcoretest_LDADD = \
	libcxts.la \
	$(top_builddir)/src/libcxcore.la


cvtest_SOURCES = \
  cv/src/aaccum.cpp             \
  cv/src/aadaptthresh.cpp       \
  cv/src/aapproxpoly.cpp        \
  cv/src/acameracalibration.cpp \
  cv/src/acamshift.cpp          \
  cv/src/acanny.cpp             \
  cv/src/achesscorners.cpp      \
  cv/src/acolor.cpp             \
  cv/src/acondens.cpp           \
  cv/src/acontourmoments.cpp    \
  cv/src/acontours.cpp          \
  cv/src/acontoursmatch.cpp     \
  cv/src/aconvhull.cpp          \
  cv/src/acreatecontourtree.cpp \
  cv/src/adistancetransform.cpp \
  cv/src/adrawing_regress.cpp   \
  cv/src/aeigenobjects.cpp      \
  cv/src/aemd.cpp               \
  cv/src/afilter.cpp            \
  cv/src/afloodfill.cpp         \
  cv/src/afundam.cpp            \
  cv/src/ahistograms.cpp        \
  cv/src/ahmmobs.cpp            \
  cv/src/ahoughtransform.cpp    \
  cv/src/aimage.cpp             \
  cv/src/aimgwarp.cpp           \
  cv/src/akalman.cpp            \
  cv/src/akdtree.cpp            \
  cv/src/akmeans.cpp            \
  cv/src/amatchcontourtrees.cpp \
  cv/src/amoments.cpp           \
  cv/src/amotiontemplates.cpp   \
  cv/src/amotseg.cpp            \
  cv/src/aoptflowhs.cpp         \
  cv/src/aoptflowlk.cpp         \
  cv/src/aoptflowpyrlk.cpp      \
  cv/src/aposit.cpp             \
  cv/src/apyrsegmentation.cpp   \
  cv/src/asnakes.cpp            \
  cv/src/aspilltree.cpp         \
  cv/src/astereocorrespondencegc.cpp \
  cv/src/asubdivisions.cpp      \
  cv/src/atemplmatch.cpp        \
  cv/src/athresh.cpp            \
  cv/src/cvtest.cpp             \
  cv/src/tsysa.cpp
cvtest_LDADD = \
  libcxts.la  \
  $(top_builddir)/src/libcxcore.la       \
  $(top_builddir)/src/libcv.la           \
  $(top_builddir)/src/libcvaux.la        \
  $(top_builddir)/src/libhighgui.la


mltest_SOURCES = \
  ml/src/aboost.cpp      \
  ml/src/adtree.cpp      \
  ml/src/aertrees.cpp    \
  ml/src/artrees.cpp     \
  ml/src/mltest_main.cpp \
  ml/src/treesbase.cpp
mltest_LDADD = \
  libcxts.la \
  $(top_builddir)/src/libcxcore.la       \
  $(top_builddir)/src/libcv.la           \
  $(top_builddir)/src/libml.la


if BUILD_OCTAVE_WRAPPERS
  TMP_OCTAVE_FILES = octave/test_adaptors.m
else
  TMP_OCTAVE_FILES =
endif
noinst_OCTAVE    = $(TMP_OCTAVE_FILES)


if BUILD_PYTHON_WRAPPERS
TMP_PYTHON_FILES = \
  python/test_adaptors.py                              \
  python/highgui/cvInitSystem.py                       \
  python/highgui/cvLoadImageBMP.py                     \
  python/highgui/cvLoadImageJPG.py                     \
  python/highgui/cvLoadImageTIFF.py                    \
  python/highgui/cvLoadImagePPM.py                     \
  python/highgui/cvLoadImagePNG.py                     \
  python/highgui/cvLoadImageSR.py                      \
  python/highgui/cvSaveImage.py                        \
  python/highgui/cvNamedWindow.py                      \
  python/highgui/cvShowImage.py                        \
  python/highgui/cvWaitKey.py                          \
  python/highgui/cvCreateTrackbar.py                   \
  python/highgui/cvMoveResizeWindow.py                 \
  python/highgui/cvGetWindowHandleName.py              \
  python/highgui/cvGetSetTrackbarPos.py                \
  python/highgui/cvSetMouseCallback.py                 \
  python/highgui/cvCreateFileCaptureRGB.py             \
  python/highgui/cvCreateFileCaptureRGBA.py            \
  python/highgui/cvCreateFileCaptureUYVY.py            \
  python/highgui/cvCreateFileCaptureY8.py              \
  python/highgui/cvCreateFileCaptureYUY2.py            \
  python/highgui/cvCreateFileCaptureYV12.py            \
  python/highgui/cvCreateFileCaptureYV16.py            \
  python/highgui/cvCreateFileCaptureYVU9.py            \
  python/highgui/cvGrabFrame.py                        \
  python/highgui/cvRetrieveFrame.py                    \
  python/highgui/cvQueryFrame.py                       \
  python/highgui/query_3gp.py                          \
  python/highgui/query_bmp24.py                        \
  python/highgui/query_bmp32.py                        \
  python/highgui/query_cinepak.py                      \
  python/highgui/query_divx.py                         \
  python/highgui/query_dv_pal_progressive_avi.py       \
  python/highgui/query_dv_pal_progressive_dv.py        \
  python/highgui/query_huffyuv.py                      \
  python/highgui/query_indeo.py                        \
  python/highgui/query_mpeg4.py                        \
  python/highgui/query_uncompressed.py                 \
  python/highgui/query_wmv9.py                         \
  python/highgui/seek_frame_3gp.py                     \
  python/highgui/seek_frame_bmp24.py                   \
  python/highgui/seek_frame_bmp32.py                   \
  python/highgui/seek_frame_cinepak.py                 \
  python/highgui/seek_frame_divx.py                    \
  python/highgui/seek_frame_dv_pal_progressive_avi.py  \
  python/highgui/seek_frame_dv_pal_progressive_dv.py   \
  python/highgui/seek_frame_huffyuv.py                 \
  python/highgui/seek_frame_indeo.py                   \
  python/highgui/seek_frame_mpeg4.py                   \
  python/highgui/seek_frame_uncompressed.py            \
  python/highgui/seek_frame_wmv9.py                    \
  python/highgui/seek_time_3gp.py                      \
  python/highgui/seek_time_bmp24.py                    \
  python/highgui/seek_time_bmp32.py                    \
  python/highgui/seek_time_cinepak.py                  \
  python/highgui/seek_time_divx.py                     \
  python/highgui/seek_time_dv_pal_progressive_avi.py	  \
  python/highgui/seek_time_dv_pal_progressive_dv.py	  \
  python/highgui/seek_time_huffyuv.py                  \
  python/highgui/seek_time_indeo.py                    \
  python/highgui/seek_time_mpeg4.py                    \
  python/highgui/seek_time_uncompressed.py             \
  python/highgui/seek_time_wmv9.py                     \
  python/highgui/size_3gp.py                           \
  python/highgui/size_bmp24.py                         \
  python/highgui/size_bmp32.py                         \
  python/highgui/size_cinepak.py                       \
  python/highgui/size_divx.py                          \
  python/highgui/size_dv_pal_progressive_avi.py        \
  python/highgui/size_dv_pal_progressive_dv.py         \
  python/highgui/size_huffyuv.py                       \
  python/highgui/size_indeo.py                         \
  python/highgui/size_mpeg4.py                         \
  python/highgui/size_uncompressed.py                  \
  python/highgui/size_wmv9.py
else
  TMP_PYTHON_FILES =
endif

noinst_PYTHON = \
  $(TMP_PYTHON_FILES)                          \
  python/highgui/works.py                \
  python/highgui/frames.py               \
  python/highgui/match.py                \
  python/highgui/query_test.py           \
  python/highgui/seek_test.py            \
  python/highgui/size_test.py            \
  python/highgui/cvCreateFileCapture.py  \
  python/highgui/cvLoadImage.py

