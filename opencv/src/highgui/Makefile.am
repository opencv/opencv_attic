lib_LTLIBRARIES = libhighgui.la

libhighgui_la_SOURCES = bitstrm.cpp grfmt_base.cpp grfmt_bmp.cpp grfmt_jpeg.cpp grfmt_pxm.cpp grfmt_sunras.cpp grfmt_tiff.cpp image.cpp loadsave.cpp precomp.cpp utils.cpp window_lnx.cpp window_w32.cpp

EXTRA_DIST = highgui.dsp makefile.bcc makefile.gcc makefile.icl makefile.vc

# here you should add all header files, which contains the functionality
# of your shared lib (published for the users)
include_HEADERS = highgui.h

# here you should add all header files, which shouldn't be installed
noinst_HEADERS = _highgui.h bitstrm.h grfmt_base.h grfmt_bmp.h grfmt_jpeg.h grfmt_pxm.h grfmt_sunras.h grfmt_tiff.h grfmts.h utils.h

SOMAJOR = @MMAJOR@
SOMINOR = @MMINOR@
SOSUBMINOR = @MSUBMINOR@

libhighgui_la_LDFLAGS = $(all_libraries) -no-undefined -version-info $(SOMAJOR):$(SOMINOR):$(SOSUBMINOR) $(LDFLAGS) $(X_LIBS) -L../../cv/src/.libs

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
libhighgui_la_LIBADD = -lXt -lXaw

INCLUDES = $(all_includes) -I../../cv/include $(X_CFLAGS)

CFLAGS += @DEBUG@
CXXFLAGS += @DEBUG@

