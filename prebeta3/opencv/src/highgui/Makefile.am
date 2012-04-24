lib_LTLIBRARIES = libhighgui.la

libhighgui_la_SOURCES = bitstrm.cpp grfmt_base.cpp grfmt_bmp.cpp grfmt_jpeg.cpp grfmt_pxm.cpp grfmt_png.cpp grfmt_sunras.cpp grfmt_tiff.cpp image.cpp loadsave.cpp precomp.cpp utils.cpp window_lnx.cpp window_w32.cpp

EXTRA_DIST = highgui.dsp 

libhighguiincludedir = $(includedir)/cv

# here you should add all header files, which contains the functionality
# of your shared lib (published for the users)
libhighguiinclude_HEADERS = highgui.h

# here you should add all header files, which shouldn't be installed
noinst_HEADERS = _highgui.h bitstrm.h grfmt_base.h grfmt_bmp.h grfmt_jpeg.h grfmt_pxm.h grfmt_png.h grfmt_sunras.h grfmt_tiff.h grfmts.h utils.h

SOMAJOR = @MMAJOR@
SOMINOR = @MMINOR@
SOSUBMINOR = @MSUBMINOR@

libhighgui_la_LDFLAGS = -no-undefined -version-info $(LT_CURRENT):$(LT_REVISION):$(LT_AGE) -release $(LT_RELEASE) @LDFLAGS@ $(X_LIBS)

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
libhighgui_la_LIBADD = @MOTIFLIBS@ @IMAGELIBS@

INCLUDES = -I. -I../../cv/include -I../.. $(X_CFLAGS)

CXXFLAGS = @CXXFLAGS@ -Wall

