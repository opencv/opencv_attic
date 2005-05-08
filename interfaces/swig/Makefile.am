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

SUBDIRS = filtered python
EXTRA_DIST = general


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

