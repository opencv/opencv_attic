lib_LTLIBRARIES = libhighgui.la

libhighgui_la_SOURCES = bitstrm.cpp cvcap.cpp cvcap_dc1394.cpp \
    cvcap_v4l.cpp grfmt_base.cpp grfmt_bmp.cpp grfmt_jpeg.cpp \
    grfmt_png.cpp grfmt_pxm.cpp grfmt_sunras.cpp grfmt_tiff.cpp \
    image.cpp loadsave.cpp precomp.cpp utils.cpp window_lnx.cpp \
    window_w32.cpp

EXTRA_DIST = highgui.dsp highgui.vcproj highgui.rc resource.h

libhighguiincludedir = $(includedir)/opencv

libhighguiinclude_HEADERS = highgui.h

noinst_HEADERS = _highgui.h bitstrm.h grfmt_base.h grfmt_bmp.h grfmt_jpeg.h grfmt_pxm.h grfmt_png.h grfmt_sunras.h grfmt_tiff.h grfmts.h utils.h

libhighgui_la_LDFLAGS = -no-undefined -version-info @LT_VERSION@ @LDFLAGS@

libhighgui_la_LIBADD  = ../../cv/src/libcv.la @GTK_LIBS@ @IMAGELIBS@ @FFMPEGLIBS@ @IEEE1394LIBS@

INCLUDES = -I. -I../../cxcore/include -I../../cv/include @GTK_CFLAGS@
