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
  -DCV_NO_BACKWARD_COMPATIBILITY

EXTRA_DIST            = filtered python


# Conditional targets ################################################################

if BUILD_PYTHON_WRAPPERS

  pkgpython_LTLIBRARIES = _cv.la _highgui.la

  pkgpython_PYTHON      = \
    python/__init__.py    \
    python/cv.py          \
    python/adaptors.py    \
    python/highgui.py

endif


# Python #############################################################################

_cv_la_SOURCES    = python/_cv.cpp python/error.cpp python/error.h filtered/cv.h filtered/constants.h
_cv_la_CXXFLAGS   = $(PYTHON_CSPEC)
_cv_la_LIBADD     = $(top_builddir)/cv/src/libcv.la
_cv_la_LDFLAGS    = -module -avoid-version -no-undefined $(PYTHON_LSPEC) -L$(SWIG_PYTHON_LIBS)

_highgui_la_SOURCES    = python/_highgui.cpp $(top_srcdir)/otherlibs/highgui/highgui.h
_highgui_la_CXXFLAGS   = $(PYTHON_CSPEC)
_highgui_la_LIBADD     = $(top_builddir)/otherlibs/highgui/libhighgui.la
_highgui_la_LDFLAGS    = -module -avoid-version -no-undefined $(PYTHON_LSPEC) -L$(SWIG_PYTHON_LIBS)


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

# this rule creates the Python wrapper sources for cxcore and cv
python/_cv.cpp: python/cv.i python/typemaps.i python/imagedata.i python/error.h python/error.cpp filtered/cv.h filtered/constants.h Makefile
	$(SWIG) $(SWIG_PYTHON_OPT) $(CV_INCLUDES) -outdir python -o $@ $<

# this rule creates the Python wrapper sources for highgui
python/_highgui.cpp: python/highgui.i python/typemaps.i $(top_srcdir)/otherlibs/highgui/highgui.h Makefile
	$(SWIG) $(SWIG_PYTHON_OPT) $(CV_INCLUDES) -DSKIP_INCLUDES -outdir python -o $@ $<

endif
