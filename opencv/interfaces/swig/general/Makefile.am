# depending on the Automake conditionals set by configure, we will build the various
# script language interfaces that SWIG is capable of generating wrappers for

BUILT_SOURCES = cvmacros.i

EXTRA_DIST = \
  cv.i \
  cvarr_operators.i \
  cvmacros.i \
  doublepointers.i \
  extensions.i \
  highgui.i \
  memory.i \
  sizeof.i \
  typemaps.i

# the following rules keep wrappers up to date, if SWIG is present
if UPDATE_SWIG_WRAPPERS

CV_HEADER_FILES = \
  $(top_srcdir)/include/opencv/cxtypes.h \
  $(top_srcdir)/include/opencv/cxcore.h  \
  $(top_srcdir)/include/opencv/cvtypes.h \
  $(top_srcdir)/include/opencv/cv.h          

cvmacros.i: $(CV_HEADER_FILES) Makefile.in
	$(PYTHON) $(top_srcdir)/utils/extract_macros.py $(CV_HEADER_FILES) > $@

endif
