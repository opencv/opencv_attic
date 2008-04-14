# test/python/Makefile.am contains partially interactive tests of HighGUI and the python interface
# 2006-07-17, asbach@ient.rwth-aachen.de


EXTRA_DIST = testdata

MOSTLYCLEANFILES = *.works

check_SCRIPTS = \
    test_adaptors.py                              \
    highgui/cvInitSystem.py                       \
    highgui/cvLoadImageBMP.py                     \
    highgui/cvLoadImageJPG.py                     \
    highgui/cvLoadImageTIFF.py                    \
    highgui/cvLoadImagePPM.py                     \
    highgui/cvLoadImagePNG.py                     \
    highgui/cvLoadImageSR.py                      \
    highgui/cvSaveImage.py                        \
    highgui/cvNamedWindow.py                      \
    highgui/cvShowImage.py                        \
    highgui/cvWaitKey.py                          \
    highgui/cvCreateTrackbar.py                   \
    highgui/cvMoveResizeWindow.py                 \
    highgui/cvGetWindowHandleName.py              \
    highgui/cvGetSetTrackbarPos.py                \
    highgui/cvSetMouseCallback.py                 \
    highgui/cvCreateFileCaptureRGB.py             \
    highgui/cvCreateFileCaptureRGBA.py            \
    highgui/cvCreateFileCaptureUYVY.py            \
    highgui/cvCreateFileCaptureY8.py              \
    highgui/cvCreateFileCaptureYUY2.py            \
    highgui/cvCreateFileCaptureYV12.py            \
    highgui/cvCreateFileCaptureYV16.py            \
    highgui/cvCreateFileCaptureYVU9.py            \
    highgui/cvGrabFrame.py                        \
    highgui/cvRetrieveFrame.py                    \
    highgui/cvQueryFrame.py                       \
    highgui/query_3gp.py                          \
    highgui/query_bmp24.py                        \
    highgui/query_bmp32.py                        \
    highgui/query_cinepak.py                      \
    highgui/query_divx.py                         \
    highgui/query_dv_pal_progressive_avi.py       \
    highgui/query_dv_pal_progressive_dv.py        \
    highgui/query_huffyuv.py                      \
    highgui/query_indeo.py                        \
    highgui/query_mpeg4.py                        \
    highgui/query_uncompressed.py                 \
    highgui/query_wmv9.py                         \
    highgui/seek_frame_3gp.py                     \
    highgui/seek_frame_bmp24.py                   \
    highgui/seek_frame_bmp32.py                   \
    highgui/seek_frame_cinepak.py                 \
    highgui/seek_frame_divx.py                    \
    highgui/seek_frame_dv_pal_progressive_avi.py  \
    highgui/seek_frame_dv_pal_progressive_dv.py   \
    highgui/seek_frame_huffyuv.py                 \
    highgui/seek_frame_indeo.py                   \
    highgui/seek_frame_mpeg4.py                   \
    highgui/seek_frame_uncompressed.py            \
    highgui/seek_frame_wmv9.py                    \
    highgui/seek_time_3gp.py                      \
    highgui/seek_time_bmp24.py                    \
    highgui/seek_time_bmp32.py                    \
    highgui/seek_time_cinepak.py                  \
    highgui/seek_time_divx.py                     \
    highgui/seek_time_dv_pal_progressive_avi.py	  \
    highgui/seek_time_dv_pal_progressive_dv.py	  \
    highgui/seek_time_huffyuv.py                  \
    highgui/seek_time_indeo.py                    \
    highgui/seek_time_mpeg4.py                    \
    highgui/seek_time_uncompressed.py             \
    highgui/seek_time_wmv9.py                     \
    highgui/size_3gp.py                           \
    highgui/size_bmp24.py                         \
    highgui/size_bmp32.py                         \
    highgui/size_cinepak.py                       \
    highgui/size_divx.py                          \
    highgui/size_dv_pal_progressive_avi.py        \
    highgui/size_dv_pal_progressive_dv.py         \
    highgui/size_huffyuv.py                       \
    highgui/size_indeo.py                         \
    highgui/size_mpeg4.py                         \
    highgui/size_uncompressed.py                  \
    highgui/size_wmv9.py

noinst_PYTHON = \
    $(check_SCRIPTS)                \
    highgui/works.py                \
    highgui/frames.py               \
    highgui/match.py                \
    highgui/query_test.py           \
    highgui/seek_test.py            \
    highgui/size_test.py            \
    highgui/cvCreateFileCapture.py  \
    highgui/cvLoadImage.py

TESTS_ENVIRONMENT = \
    top_srcdir=$(top_srcdir) \
    top_builddir=$(top_builddir) \
    LD_LIBRARY_PATH=$(top_builddir)/cxcore/src/.libs:$(top_builddir)/cv/src/.libs:$(top_builddir)/otherlibs/highgui/.libs:$(LD_LIBRARY_PATH) \
    DYLD_LIBRARY_PATH=$(top_builddir)/cxcore/src/.libs:$(top_builddir)/cv/src/.libs:$(top_builddir)/otherlibs/highgui/.libs:$(LD_LIBRARY_PATH) \
    PYTHONPATH=$(top_srcdir)/interfaces/swig:$(top_builddir)/interfaces/swig/python/.libs

# do tests only if we ask for python

if BUILD_PYTHON_WRAPPERS

TESTS = $(check_SCRIPTS)

endif

