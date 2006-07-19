# depending on the Automake conditionals set by configure, we will build the various
# script language interfaces that SWIG is capable of generating wrappers for

BUILT_SOURCES = cvmacros.i

# the following rules keep wrappers up to date, if SWIG is present
if UPDATE_SWIG_WRAPPERS

CV_HEADER_FILES = \
		   $(top_srcdir)/cxcore/include/cxtypes.h  $(top_srcdir)/cxcore/include/cxcore.h  \
		  $(top_srcdir)/cv/include/cvtypes.h $(top_srcdir)/cv/include/cv.h          

cvmacros.i: $(CV_HEADER_FILES) Makefile
	$(PYTHON) $(top_srcdir)/utils/extract_macros.py $(CV_HEADER_FILES) > $@

endif
