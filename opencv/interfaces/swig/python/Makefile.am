# opencv/interfaces/swig/python/Makefile.am
# 2005-05-07, Mark Asbach <asbach@ient.rwth-aachen.de>

# depending on the Automake conditionals set by configure, we will build the various
# script language interfaces that SWIG is capable of generating wrappers for

CV_INCLUDES = \
  -I$(top_srcdir)/cv/include         \
  -I$(top_srcdir)/cxcore/include     \
  -I$(top_srcdir)/ml/include         \
  -I$(top_srcdir)/otherlibs/highgui

AM_CPPFLAGS = \
  $(CV_INCLUDES)                     \
  -DCV_NO_BACKWARD_COMPATIBILITY

AM_CXXFLAGS = @PY_CXXFLAGS@

noinst_PYTHON = setup-for-win.py

EXTRA_DIST = \
  __init__.py      \
  \
  cv.py            \
  adaptors.py      \
  highgui.py       \
  matlab_syntax.py \
  \
  cv.i             \
  cvaliases.i      \
  cvarr.i          \
  cvseq.i          \
  cvshadow.i       \
  ml.i             \
  highgui.i        \
  imagedata.i      \
  nointpb.i        \
  pyhelpers.i      \
  pytypemaps.i     \
  \
  cvshadow.h       \
  error.h          \
  pycvseq.hpp      \
  pyhelpers.h      \
  \
  _cv.cpp          \
  _ml.cpp          \
  _highgui.cpp     \
  cvshadow.cpp     \
  error.cpp        \
  pyhelpers.cpp


# SWIG #######################################################################

# the following rules keep wrappers up to date, if SWIG is present
if UPDATE_SWIG_WRAPPERS

# this rule creates the Python wrapper sources for cxcore and cv
_cv.cpp: cv.i imagedata.i cvarr.i cvaliases.i pyhelpers.i       \
         nointpb.i pytypemaps.i cvshadow.i cvseq.i              \
         error.cpp error.h                                      \
         pyhelpers.h cvshadow.h pycvseq.hpp                     \
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
	$(SWIG) $(SWIG_PYTHON_OPT) $(CV_INCLUDES) -o $@ $<

# this rule creates the Python wrapper sources for ml
_ml.cpp: ml.i nointpb.i pytypemaps.i                            \
              $(top_srcdir)/interfaces/swig/general/typemaps.i  \
              $(top_srcdir)/interfaces/swig/general/memory.i    \
              $(top_srcdir)/ml/include/ml.h                     \
              Makefile.in
	$(SWIG) $(SWIG_PYTHON_OPT) $(CV_INCLUDES) -DSKIP_INCLUDES -o $@ $<

# this rule creates the Python wrapper sources for highgui
_highgui.cpp: highgui.i nointpb.i pytypemaps.i                  \
              $(top_srcdir)/interfaces/swig/general/highgui.i   \
              $(top_srcdir)/interfaces/swig/general/typemaps.i  \
              $(top_srcdir)/interfaces/swig/general/memory.i    \
              $(top_srcdir)/otherlibs/highgui/highgui.h         \
              Makefile.in
	$(SWIG) $(SWIG_PYTHON_OPT) $(CV_INCLUDES) -DSKIP_INCLUDES -o $@ $<

CV_HEADER_FILES = \
  $(top_srcdir)/cxcore/include/cxtypes.h \
  $(top_srcdir)/cxcore/include/cxcore.h  \
  $(top_srcdir)/cv/include/cvtypes.h \
  $(top_srcdir)/cv/include/cv.h

cvaliases.i: $(CV_HEADER_FILES) Makefile.in
	$(PYTHON) $(top_srcdir)/utils/extract_aliases.py $(CV_HEADER_FILES) > $@

endif


# Conditional targets #######################################################

if BUILD_PYTHON_WRAPPERS

#pkgpython_LTLIBRARIES = _cv.la _highgui.la
pkgpython_LTLIBRARIES = _cv.la _ml.la _highgui.la

pkgpython_PYTHON = \
  __init__.py    \
  cv.py          \
  ml.py          \
  highgui.py     \
  adaptors.py    \
  matlab_syntax.py

_cv_la_SOURCES    = _cv.cpp error.cpp error.h pyhelpers.cpp pyhelpers.h \
                    pycvseq.hpp cvshadow.cpp cvshadow.h \
                    cv.i imagedata.i cvarr.i nointpb.i pytypemaps.i cvseq.i cvshadow.i
_cv_la_CXXFLAGS   = $(PYTHON_CSPEC)
_cv_la_LIBADD     = $(top_builddir)/cv/src/libcv.la
_cv_la_LDFLAGS    = -module -avoid-version -no-undefined $(PYTHON_LSPEC) \
                    -L$(SWIG_PYTHON_LIBS)

_ml_la_SOURCES    = _ml.cpp pyhelpers.cpp pyhelpers.h
_ml_la_CXXFLAGS   = $(PYTHON_CSPEC)
_ml_la_LIBADD     = $(top_builddir)/ml/src/libml.la
_ml_la_LDFLAGS    = -module -avoid-version -no-undefined $(PYTHON_LSPEC) \
                    -L$(SWIG_PYTHON_LIBS)

_highgui_la_SOURCES    = _highgui.cpp pyhelpers.cpp pyhelpers.h \
                         highgui.i
_highgui_la_CXXFLAGS     = $(PYTHON_CSPEC)
_highgui_la_LIBADD       = $(top_builddir)/otherlibs/highgui/libhighgui.la
_highgui_la_LDFLAGS      = -module -avoid-version -no-undefined $(PYTHON_LSPEC) \
                           -L$(SWIG_PYTHON_LIBS)

endif
