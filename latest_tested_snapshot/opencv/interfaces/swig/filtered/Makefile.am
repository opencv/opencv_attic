
# opencv/interfaces/swig/filtered/Makefile.am
# 2005-05-07, Mark Asbach <asbach@ient.rwth-aachen.de>

# depending on the Automake conditionals set by configure, we will build the various
# script language interfaces that SWIG is capable of generating wrappers for

CV_INCLUDES = \
  -I$(top_srcdir)/include/opencv

AM_CPPFLAGS = \
  $(CV_INCLUDES)                     \
  -DCV_NO_BACKWARD_COMPATIBILITY

noinst_HEADERS = cv.h constants.h


# SWIG ###############################################################################

# the following rules keep wrappers up to date, if SWIG is present
if UPDATE_SWIG_WRAPPERS

# this rule creates a single SWIG parseable header from cv.h and cxcore.h, etc.
cv.h: \
  $(top_srcdir)/include/opencv/cv.h \
  $(top_srcdir)/include/opencv/cvtypes.h \
  $(top_srcdir)/include/opencv/cxcore.h \
  $(top_srcdir)/include/opencv/cxcore.hpp \
  $(top_srcdir)/include/opencv/cvver.h \
  $(top_srcdir)/include/opencv/cxtypes.h \
  $(top_srcdir)/include/opencv/cxerror.h \
  Makefile.in
	$(CPP) -x c++ -I$(top_builddir) $(AM_CPPFLAGS) $(CPPFLAGS) -DSKIP_INCLUDES -o $@ $(top_srcdir)/include/opencv/cv.h

# this rule creates a single SWIG parseable header from cv.h and cxcore.h, etc.
constants.h: \
  $(top_srcdir)/include/opencv/cv.h \
  $(top_srcdir)/include/opencv/cvtypes.h \
  $(top_srcdir)/include/opencv/cxcore.h \
  $(top_srcdir)/include/opencv/cxcore.hpp \
  $(top_srcdir)/include/opencv/cxtypes.h \
  $(top_srcdir)/include/opencv/cxerror.h \
  Makefile.in
	$(PYTHON) $(top_srcdir)/utils/extract_constants.py $(top_srcdir)/include/opencv/cxcore.h   > $@
	$(PYTHON) $(top_srcdir)/utils/extract_constants.py $(top_srcdir)/include/opencv/cvver.h   >> $@
	$(PYTHON) $(top_srcdir)/utils/extract_constants.py $(top_srcdir)/include/opencv/cxtypes.h >> $@
	$(PYTHON) $(top_srcdir)/utils/extract_constants.py $(top_srcdir)/include/opencv/cxerror.h >> $@
	$(PYTHON) $(top_srcdir)/utils/extract_constants.py $(top_srcdir)/include/opencv/cv.h      >> $@
	$(PYTHON) $(top_srcdir)/utils/extract_constants.py $(top_srcdir)/include/opencv/cvtypes.h >> $@

endif
