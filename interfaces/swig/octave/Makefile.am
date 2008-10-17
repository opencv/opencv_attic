CV_INCLUDES = \
  -I$(top_srcdir)/cv/include         \
  -I$(top_srcdir)/cxcore/include     \
  -I$(top_srcdir)/ml/include         \
  -I$(top_srcdir)/otherlibs/highgui	 \
  -I$(srcdir)

AM_CPPFLAGS = \
  $(CV_INCLUDES)                     \
  -DCV_NO_BACKWARD_COMPATIBILITY

AM_CXXFLAGS = @OCT_CXXFLAGS@

EXTRA_DIST = \
  opencv.m         \
  \
  adapters.i       \
  cvaliases.i      \
  cvarr.i          \
  cv.i             \
  cvseq.i          \
  cvshadow.i       \
  highgui.i        \
  imagedata.i      \
  ml.i             \
  octhelpers.i     \
  octtypemaps.i    \
  \
  cvshadow.h       \
  octerror.h       \
  octhelpers.h     \
  \
  cv_wrap.cpp      \
  ml_wrap.cpp      \
  highgui_wrap.cpp \
  cvshadow.cpp     \
  octerror.cpp     \
  octhelpers.cpp


# SWIG #######################################################################

# the following rules keep wrappers up to date, if SWIG is present
if UPDATE_SWIG_WRAPPERS

# this rule creates the octave wrapper sources for cxcore and cv
cv_wrap.cpp: cv.i imagedata.i cvarr.i octhelpers.i              \
         octtypemaps.i cvshadow.i cvseq.i                       \
         octerror.cpp octerror.h                                \
         octhelpers.h cvshadow.h octcvseq.hpp                   \
         $(top_srcdir)/interfaces/swig/general/cv.i             \
         $(top_srcdir)/interfaces/swig/general/memory.i         \
         $(top_srcdir)/interfaces/swig/general/typemaps.i       \
         $(top_srcdir)/interfaces/swig/general/extensions.i     \
         $(top_srcdir)/interfaces/swig/general/doublepointers.i \
         $(top_srcdir)/interfaces/swig/general/sizeof.i         \
         $(top_srcdir)/interfaces/swig/general/cvmacros.i       \
         ../filtered/cv.h            \
         ../filtered/constants.h     \
         Makefile.in
	$(SWIG) $(SWIG_OCTAVE_OPT) $(CV_INCLUDES) -o $@ $<

# this rule creates the octave wrapper sources for ml
ml_wrap.cpp: ml.i octtypemaps.i                                 \
              $(top_srcdir)/interfaces/swig/general/typemaps.i  \
              $(top_srcdir)/interfaces/swig/general/memory.i    \
              $(top_srcdir)/ml/include/ml.h                     \
              Makefile.in
	$(SWIG) $(SWIG_OCTAVE_OPT) $(CV_INCLUDES) -DSKIP_INCLUDES -o $@ $<

# this rule creates the octave wrapper sources for highgui
highgui_wrap.cpp: highgui.i octtypemaps.i adapters.i            \
              $(top_srcdir)/interfaces/swig/general/highgui.i   \
              $(top_srcdir)/interfaces/swig/general/typemaps.i  \
              $(top_srcdir)/interfaces/swig/general/memory.i    \
              $(top_srcdir)/otherlibs/highgui/highgui.h         \
              Makefile.in
	$(SWIG) $(SWIG_OCTAVE_OPT) $(CV_INCLUDES) -DSKIP_INCLUDES -o $@ $<

endif


# Conditional targets #######################################################

if BUILD_OCTAVE_WRAPPERS

OCTAVE_PKG_VERSION=1.0 #$(shell egrep '^Version: ' $(top_srcdir)/interfaces/swig/octave/DESCRIPTION|sed -e 's/Version: //')

PKG_NAME=opencv-1.1.0
PKG_OCT_FILES = cv.oct ml.oct highgui.oct
PKG_OTHER_FILES = $(srcdir)/COPYING $(srcdir)/DESCRIPTION $(srcdir)/INDEX $(srcdir)/PKG_ADD_template

octavedir=$(pkgdatadir)/octave
dist_octave_DATA = opencv-1.1.0.tar.gz

opencv-1.1.0.tar.gz: $(PKG_OCT_FILES) $(PKG_OTHER_FILES)
	mkdir -p $(PKG_NAME)
	mkdir -p $(PKG_NAME)/inst/$(OCTAVE_ARCH)
	cp -rf $(PKG_OTHER_FILES) $(PKG_NAME)
	mv $(PKG_NAME)/PKG_ADD_template $(PKG_NAME)/PKG_ADD
	cp -rf $(PKG_OCT_FILES) $(PKG_NAME)/inst/$(OCTAVE_ARCH)
	tar cpzvf $@ $(PKG_NAME)
	rm -rf $(PKG_NAME)

# seems that mkoctfile has some troubles compiling the wrapper... it takes forever
# workaround is to use the g++ directly
MY_MKOCTFILE=$(CXX) -O0 -g -shared $(OCTAVE_INCFLAGS)

cv.oct: cv_wrap.cpp cvshadow.cpp octerror.cpp octhelpers.cpp
	$(MY_MKOCTFILE) -o $@ $(AM_CPPFLAGS) -L$(top_builddir)/cv/src/.libs -lcv $^
ml.oct: ml_wrap.cpp octhelpers.cpp
	$(MY_MKOCTFILE) -o $@ $(AM_CPPFLAGS) -L$(top_builddir)/ml/src/.libs -lml $^
highgui.oct: highgui_wrap.cpp octhelpers.cpp
	$(MY_MKOCTFILE) -o $@ $(AM_CPPFLAGS) -L$(top_builddir)/otherlibs/highgui/.libs -lhighgui $^

endif

clean-local:
	rm -rf opencv-1.1.0.tar.gz *.oct
