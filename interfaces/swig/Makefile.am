# opencv/interfaces/swig/Makefile.am
# 2005-01-23, Mark Asbach <asbach@ient.rwth-aachen.de>

# depending on the Automake conditionals set by configure, we will build the various
# script language interfaces that SWIG is capable of generating wrappers for

CV_INCLUDES = \
  -I$(top_srcdir)/cv/include         \
  -I$(top_srcdir)/cxcore/include     \
  -I$(top_srcdir)/otherlibs/highgui

AM_CPPFLAGS = \
  $(CV_INCLUDES)                     \
  @PYTHON_CPPFLAGS@                  \
  -DCV_NO_BACKWARD_COMPATIBILITY

EXTRA_DIST            = filtered python


# Conditional targets ################################################################

if BUILD_PYTHON_WRAPPERS
  pkgpython_LTLIBRARIES = _opencv.la
  pkgpython_PYTHON      = python/opencv.py
endif


# Python #############################################################################

# python wrapper binary part
_opencv_la_SOURCES    = python/opencv_wrap.cpp filtered/cv.h filtered/constants.h
_opencv_la_CXXFLAGS   = $(SWIG_PYTHON_CPPFLAGS)
#_opencv_la_LIBADD     = ../../cv/src/libcv.la ../../cxcore/src/libcxcore.la
_opencv_la_LIBADD     = ../../cv/src/libcv.la 
_opencv_la_LDFLAGS    = -module -avoid-version -no-undefined -L$(SWIG_PYTHON_LIBS)


# SWIG ###############################################################################

# the following rules keep wrappers up to date, if SWIG is present
if UPDATE_SWIG_WRAPPERS

# this rule creates a single SWIG parseable header from cv.h and cxcore.h, etc.
filtered/cv.h: $(top_srcdir)/cv/include/cv.h $(top_srcdir)/cv/include/cvtypes.h \
               $(top_srcdir)/cxcore/include/cxcore.h $(top_srcdir)/cxcore/include/cxtypes.h \
               $(top_srcdir)/cxcore/include/cxerror.h Makefile
	$(CPP) $(AM_CPPFLAGS) $(CPPFLAGS) -DSKIP_INCLUDES -o $@ $(top_srcdir)/cv/include/cv.h

# this rule creates a single SWIG parseable header from cv.h and cxcore.h, etc.
filtered/constants.h: $(top_srcdir)/cv/include/cv.h $(top_srcdir)/cv/include/cvtypes.h \
               $(top_srcdir)/cxcore/include/cxcore.h $(top_srcdir)/cxcore/include/cxtypes.h \
               $(top_srcdir)/cxcore/include/cxerror.h Makefile
	$(PYTHON) $(top_srcdir)/utils/extract_constants.py $(top_srcdir)/cxcore/include/cxcore.h   > $@
	$(PYTHON) $(top_srcdir)/utils/extract_constants.py $(top_srcdir)/cxcore/include/cxtypes.h >> $@
	$(PYTHON) $(top_srcdir)/utils/extract_constants.py $(top_srcdir)/cxcore/include/cxerror.h >> $@
	$(PYTHON) $(top_srcdir)/utils/extract_constants.py $(top_srcdir)/cv/include/cv.h          >> $@
	$(PYTHON) $(top_srcdir)/utils/extract_constants.py $(top_srcdir)/cv/include/cvtypes.h     >> $@

# this rule creates the Python wrapper sources
python/opencv_wrap.cpp: python/opencv.i filtered/cv.h filtered/constants.h Makefile
	$(SWIG) $(SWIG_PYTHON_OPT) $(CV_INCLUDES) -outdir python -o $@ $<

endif
