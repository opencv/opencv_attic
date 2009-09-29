# This Makefile compiles, links and invokes all tests
# Python and Octave tests are only run if python or octave resp. have been --enable[d] in ./configure


check_LTLIBRARIES  = libcxts.la
check_PROGRAMS     = cxcoretest cvtest mltest
dist_check_SCRIPTS = test_cxcore.sh test_cv.sh test_ml.sh $(TMP_OCTAVE_FILES) $(TMP_PYTHON_FILES)
TESTS              = $(dist_check_SCRIPTS)


clean-local:
	-rm -rf *.lst *.sum python-old/*.pyc *.works python-old/*.works python-old/highgui/*.pyc python-old/highgui/*.works


TESTS_ENVIRONMENT = \
  LD_LIBRARY_PATH=$(top_builddir)/src/.libs:$(LD_LIBRARY_PATH) \
  DYLD_LIBRARY_PATH=$(top_builddir)/src/.libs:$(LD_LIBRARY_PATH) \
  OCTAVEPATH=$(top_builddir)/interfaces/swig/octave \
  PYTHONPATH=$(top_srcdir)/tests/python:$(top_srcdir)/tests/python-old/highgui:$(top_srcdir)/interfaces/swig/python:$(top_builddir)/interfaces/swig/python-old/.libs

AM_CPPFLAGS = \
  -I$(top_srcdir)/tests/cxts        \
  -I$(top_srcdir)/include/opencv

AM_LDFLAGS = @OPENMP_CXXFLAGS@


EXTRA_DIST = \
  CMakeLists.txt \
  cv/CMakeLists.txt \
  cxcore/CMakeLists.txt \
  cxts/CMakeLists.txt \
  ml/CMakeLists.txt \
  \
  cxcore/src/precomp.cpp \
  cxts/precomp.cpp \
  \
  ml/src/precomp.cpp \
  \
  octave/frame.jpg \
  \
  python/test.py \
  python/tickets.py


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
  ml/src/amltests.cpp    \
  ml/src/mltest_main.cpp \
  ml/src/mltests.cpp     \
  ml/src/slmltests.cpp
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
TMP_PYTHON_FILES = 
#  python-old/testall.py            \
#  python-old/highgui/cvInitSystem.py                       \
#  python-old/highgui/cvLoadImageBMP.py                     \
#  python-old/highgui/cvLoadImageJPG.py                     \
#  python-old/highgui/cvLoadImageTIFF.py                    \
#  python-old/highgui/cvLoadImagePPM.py                     \
#  python-old/highgui/cvLoadImagePNG.py                     \
#  python-old/highgui/cvLoadImageSR.py                      \
#  python-old/highgui/cvSaveImage.py                        \
#  python-old/highgui/cvNamedWindow.py                      \
#  python-old/highgui/cvShowImage.py                        \
#  python-old/highgui/cvWaitKey.py                          \
#  python-old/highgui/cvCreateTrackbar.py                   \
#  python-old/highgui/cvMoveResizeWindow.py                 \
#  python-old/highgui/cvGetSetTrackbarPos.py                \
#  python-old/highgui/cvGetWindowHandleName.py              \
#  python-old/highgui/cvSetMouseCallback.py                 \
#  python-old/highgui/cvCreateFileCaptureRGB.py             \
#  python-old/highgui/cvCreateFileCaptureRGBA.py            \
#  python-old/highgui/cvCreateFileCaptureUYVY.py            \
#  python-old/highgui/cvCreateFileCaptureY8.py              \
#  python-old/highgui/cvCreateFileCaptureYUY2.py            \
#  python-old/highgui/cvCreateFileCaptureYV12.py            \
#  python-old/highgui/cvCreateFileCaptureYV16.py            \
#  python-old/highgui/cvCreateFileCaptureYVU9.py            \
#  python-old/highgui/cvGetCaptureProperty.py               \
#  python-old/highgui/cvGrabFrame.py                        \
#  python-old/highgui/cvRetrieveFrame.py                    \
#  python-old/highgui/cvQueryFrame.py                       \
#  python-old/highgui/query_3gp.py                          \
#  python-old/highgui/query_bmp24.py                        \
#  python-old/highgui/query_bmp32.py                        \
#  python-old/highgui/query_cinepak.py                      \
#  python-old/highgui/query_divx.py                         \
#  python-old/highgui/query_dv_pal_progressive_avi.py       \
#  python-old/highgui/query_dv_pal_progressive_dv.py        \
#  python-old/highgui/query_huffyuv.py                      \
#  python-old/highgui/query_indeo.py                        \
#  python-old/highgui/query_mpeg4.py                        \
#  python-old/highgui/query_uncompressed.py                 \
#  python-old/highgui/query_wmv9.py                         \
#  python-old/highgui/seek_frame_3gp.py                     \
#  python-old/highgui/seek_frame_bmp24.py                   \
#  python-old/highgui/seek_frame_bmp32.py                   \
#  python-old/highgui/seek_frame_cinepak.py                 \
#  python-old/highgui/seek_frame_divx.py                    \
#  python-old/highgui/seek_frame_dv_pal_progressive_avi.py  \
#  python-old/highgui/seek_frame_dv_pal_progressive_dv.py   \
#  python-old/highgui/seek_frame_huffyuv.py                 \
#  python-old/highgui/seek_frame_indeo.py                   \
#  python-old/highgui/seek_frame_mpeg4.py                   \
#  python-old/highgui/seek_frame_uncompressed.py            \
#  python-old/highgui/seek_frame_wmv9.py                    \
#  python-old/highgui/seek_time_3gp.py                      \
#  python-old/highgui/seek_time_bmp24.py                    \
#  python-old/highgui/seek_time_bmp32.py                    \
#  python-old/highgui/seek_time_cinepak.py                  \
#  python-old/highgui/seek_time_divx.py                     \
#  python-old/highgui/seek_time_dv_pal_progressive_avi.py	  \
#  python-old/highgui/seek_time_dv_pal_progressive_dv.py	  \
#  python-old/highgui/seek_time_huffyuv.py                  \
#  python-old/highgui/seek_time_indeo.py                    \
#  python-old/highgui/seek_time_mpeg4.py                    \
#  python-old/highgui/seek_time_uncompressed.py             \
#  python-old/highgui/seek_time_wmv9.py                     \
#  python-old/highgui/size_3gp.py                           \
#  python-old/highgui/size_bmp24.py                         \
#  python-old/highgui/size_bmp32.py                         \
#  python-old/highgui/size_cinepak.py                       \
#  python-old/highgui/size_divx.py                          \
#  python-old/highgui/size_dv_pal_progressive_avi.py        \
#  python-old/highgui/size_dv_pal_progressive_dv.py         \
#  python-old/highgui/size_huffyuv.py                       \
#  python-old/highgui/size_indeo.py                         \
#  python-old/highgui/size_mpeg4.py                         \
#  python-old/highgui/size_uncompressed.py                  \
#  python-old/highgui/size_wmv9.py
else
  TMP_PYTHON_FILES =
endif

noinst_PYTHON = 

#  $(TMP_PYTHON_FILES)                        \
#  python-old/cmp_tests.py                    \
#  python-old/cvtestutils.py                  \
#  python-old/feature_tree_tests.py           \
#  python-old/histogram_tests.py              \
#  python-old/homography_tests.py             \
#  python-old/lsh_tests.py                    \
#  python-old/moments_tests.py                \
#  python-old/roots_tests.py                  \
#  python-old/test_adaptors.py                \
#  python-old/highgui/works.py                \
#  python-old/highgui/frames.py               \
#  python-old/highgui/match.py                \
#  python-old/highgui/query_test.py           \
#  python-old/highgui/seek_test.py            \
#  python-old/highgui/size_test.py            \
#  python-old/highgui/cvCreateFileCapture.py  \
#  python-old/highgui/cvLoadImage.py
