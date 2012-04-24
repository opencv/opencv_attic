EXTRA_DIST = highgui.dsp highgui.vcproj highgui.rc resource.h    \
    cvcap_cmu.cpp cvcap_tyzx.cpp cvcap_vfw.cpp cvcap_mil.cpp     \
    cvcap_qt.cpp cvcap_dc1394.cpp cvcap_ffmpeg.cpp               \
    cvcap_v4l.cpp cvcap_xine.cpp                                 \
    window_carbon.cpp window_gtk.cpp window_w32.cpp

INCLUDES = -I. -I$(top_srcdir)/cxcore/include \
    -I$(top_srcdir)/cv/include @GTHREAD_CFLAGS@ @GTK_CFLAGS@ \
    @QUICKTIME_CFLAGS@ @CARBON_CFLAGS@

noinst_HEADERS     = \
    _highgui.h bitstrm.h grfmt_base.h grfmt_bmp.h \
    grfmt_jpeg.h grfmt_jpeg2000.h grfmt_exr.h \
    grfmt_pxm.h grfmt_png.h grfmt_sunras.h grfmt_tiff.h grfmts.h utils.h
noinst_LTLIBRARIES = lib_highgui.la
lib_LTLIBRARIES    = libhighgui.la

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

if BUILD_CARBON
TMP_CARBON=window_carbon.cpp
else
TMP_CARBON=
endif

if BUILD_GTK
TMP_GTK=window_gtk.cpp
else
TMP_GTK=
endif


# convenience library
lib_highgui_la_SOURCES = \
    bitstrm.cpp grfmt_base.cpp grfmt_bmp.cpp grfmt_jpeg.cpp      \
    grfmt_png.cpp grfmt_pxm.cpp grfmt_jpeg2000.cpp grfmt_exr.cpp \
    grfmt_sunras.cpp grfmt_tiff.cpp image.cpp loadsave.cpp       \
    precomp.cpp utils.cpp cvcap.cpp window.cpp                   \
    $(TMP_DC1394) $(TMP_V4L) $(TMP_FFMPEG) $(TMP_QT)             \
    $(TMP_XINE) $(TMP_GTK) $(TMP_CARBON)
lib_highgui_la_LDFLAGS = -no-undefined @LDFLAGS@

# real library
libhighguiincludedir = $(includedir)/opencv
libhighguiinclude_HEADERS = highgui.h
libhighgui_la_SOURCES = dummy.cpp
libhighgui_la_LDFLAGS = -no-undefined -version-info @LT_VERSION@ @LDFLAGS@
libhighgui_la_LIBADD  = \
    lib_highgui.la \
    $(top_builddir)/cxcore/src/libcxcore.la \
    $(top_builddir)/cv/src/libcv.la         \
    @GTHREAD_LIBS@ @GTK_LIBS@ @IMAGELIBS@ @FFMPEGLIBS@ @IEEE1394LIBS@ \
    @QUICKTIME_LIBS@ @CARBON_LIBS@ @XINE_LIBS@ @LTLIBOBJS@
