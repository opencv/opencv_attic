# opencv/interfaces/swig/python/Makefile.am
# 2005-05-07, Mark Asbach <asbach@ient.rwth-aachen.de>

# depending on the Automake conditionals set by configure, we will build the various
# script language interfaces that SWIG is capable of generating wrappers for

CV_INCLUDES = -I$(top_srcdir)/include/opencv -I$(srcdir)

AM_CPPFLAGS = \
  $(CV_INCLUDES)                     \
  -DCV_NO_BACKWARD_COMPATIBILITY

noinst_PYTHON = setup-for-win.py

EXTRA_DIST = \
  \
  CMakeLists.txt   \
  \
  __init__.py      \
  \
  cv.py            \
  adaptors.py      \
  highgui.py       \
  matlab_syntax.py \
  \
  cv.i             \
  cvaliases.i      \
  cvaliases_autogen.i \
  cvarr.i          \
  cvseq.i          \
  cvshadow.i       \
  cvswigmacros.i   \
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
_cv.cpp: cv.i imagedata.i cvarr.i cvaliases.i cvaliases_autogen.i pyhelpers.i \
         nointpb.i pytypemaps.i cvshadow.i cvseq.i cvswigmacros.i \
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
_ml.cpp: ml.i nointpb.i pytypemaps.i cvswigmacros.i             \
              $(top_srcdir)/interfaces/swig/general/typemaps.i  \
              $(top_srcdir)/interfaces/swig/general/memory.i    \
              $(top_srcdir)/include/opencv/ml.h                 \
              Makefile.in
	$(SWIG) $(SWIG_PYTHON_OPT) $(CV_INCLUDES) -DSKIP_INCLUDES -o $@ $<

# this rule creates the Python wrapper sources for highgui
_highgui.cpp: highgui.i nointpb.i pytypemaps.i cvswigmacros.i   \
              $(top_srcdir)/interfaces/swig/general/highgui.i   \
              $(top_srcdir)/interfaces/swig/general/typemaps.i  \
              $(top_srcdir)/interfaces/swig/general/memory.i    \
              $(top_srcdir)/include/opencv/highgui.h            \
              Makefile.in
	$(SWIG) $(SWIG_PYTHON_OPT) $(CV_INCLUDES) -DSKIP_INCLUDES -o $@ $<

CV_HEADER_FILES = \
  $(top_srcdir)/include/opencv/cxtypes.h \
  $(top_srcdir)/include/opencv/cxcore.h  \
  $(top_srcdir)/include/opencv/cvtypes.h \
  $(top_srcdir)/include/opencv/cv.h

cvaliases_autogen.i: $(CV_HEADER_FILES) Makefile.in
	$(PYTHON) $(top_srcdir)/utils/extract_aliases.py $(CV_HEADER_FILES) > $@

endif


# Conditional targets #######################################################

if BUILD_PYTHON_WRAPPERS

pkgpyexec_LTLIBRARIES = _cv.la _ml.la _highgui.la

pkgpyexec_PYTHON = \
  __init__.py    \
  cv.py          \
  ml.py          \
  highgui.py     \
  adaptors.py    \
  matlab_syntax.py

_cv_la_SOURCES    = _cv.cpp error.cpp error.h pyhelpers.cpp pyhelpers.h \
                    pycvseq.hpp cvshadow.cpp cvshadow.h \
                    cv.i imagedata.i cvarr.i nointpb.i pytypemaps.i cvseq.i cvshadow.i
_cv_la_CXXFLAGS   = $(PYTHON_CSPEC) $(AM_CXXFLAGS)
_cv_la_LIBADD     = $(top_builddir)/src/libcv.la $(top_builddir)/src/libcxcore.la
_cv_la_LDFLAGS    = -module -avoid-version -no-undefined $(PYTHON_LSPEC) $(SWIG_PYTHON_LIBS) @LDFLAGS@

_ml_la_SOURCES    = _ml.cpp pyhelpers.cpp pyhelpers.h
_ml_la_CXXFLAGS   = $(PYTHON_CSPEC) $(AM_CXXFLAGS)
_ml_la_LIBADD     = $(top_builddir)/src/libml.la
_ml_la_LDFLAGS    = -module -avoid-version -no-undefined $(PYTHON_LSPEC) $(SWIG_PYTHON_LIBS) @LDFLAGS@

_highgui_la_SOURCES    = _highgui.cpp pyhelpers.cpp pyhelpers.h \
                         highgui.i
_highgui_la_CXXFLAGS     = $(PYTHON_CSPEC) $(AM_CXXFLAGS)
_highgui_la_LIBADD       = $(top_builddir)/src/libhighgui.la
_highgui_la_LDFLAGS      = -module -avoid-version -no-undefined $(PYTHON_LSPEC) $(SWIG_PYTHON_LIBS) @LDFLAGS@

endif
