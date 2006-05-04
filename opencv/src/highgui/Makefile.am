EXTRA_DIST = highgui.dsp highgui.vcproj highgui.rc resource.h    \
    cvcap_cmu.cpp cvcap_tyzx.cpp cvcap_vfw.cpp cvcap_mil.cpp     \
    cvcap_qt.cpp cvcap_dc1394.cpp cvcap_ffmpeg.cpp               \
    cvcap_v4l.cpp cvcap_xine.cpp

INCLUDES = -I. -I$(top_srcdir)/cxcore/include \
	-I$(top_srcdir)/cv/include @GTHREAD_CFLAGS@ @GTK_CFLAGS@ \
	@QUICKTIME_CFLAGS@

lib_LTLIBRARIES = libhighgui.la

if BUILD_DC1394
TMP_DC1394=cvcap_dc1394.cpp
else
TMP_DC1394=
endif

if BUILD_V4L
TMP_V4L=cvcap_v4l.cpp
else
TMP_V4L= 
endif

if BUILD_FFMPEG
TMP_FFMPEG=cvcap_ffmpeg.cpp
else
TMP_FFMPEG=
endif

if BUILD_XINE
TMP_XINE=cvcap_xine.cpp
else
TMP_XINE=
endif

if BUILD_QUICKTIME
TMP_QT=cvcap_qt.cpp
else
TMP_QT=
endif

libhighgui_la_SOURCES = \
    bitstrm.cpp grfmt_base.cpp grfmt_bmp.cpp grfmt_jpeg.cpp      \
    grfmt_png.cpp grfmt_pxm.cpp grfmt_sunras.cpp grfmt_tiff.cpp  \
    image.cpp loadsave.cpp precomp.cpp utils.cpp window_lnx.cpp  \
    cvcap.cpp $(TMP_DC1394) $(TMP_V4L) $(TMP_FFMPEG) $(TMP_QT)   \
    $(TMP_XINE)

libhighguiincludedir = $(includedir)/opencv

libhighguiinclude_HEADERS = highgui.h

noinst_HEADERS = _highgui.h bitstrm.h grfmt_base.h grfmt_bmp.h \
	grfmt_jpeg.h grfmt_pxm.h grfmt_png.h grfmt_sunras.h grfmt_tiff.h grfmts.h utils.h

libhighgui_la_LDFLAGS = -no-undefined -version-info @LT_VERSION@ @LDFLAGS@

libhighgui_la_LIBADD  = $(top_builddir)/cxcore/src/libcxcore.la $(top_builddir)/cv/src/libcv.la \
	@GTHREAD_LIBS@ @GTK_LIBS@ @IMAGELIBS@ @FFMPEGLIBS@ @IEEE1394LIBS@ \
	@QUICKTIME_LIBS@ @CARBON_LIBS@ @XINE_LIBS@

