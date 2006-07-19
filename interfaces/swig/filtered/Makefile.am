# opencv/interfaces/swig/filtered/Makefile.am
# 2005-05-07, Mark Asbach <asbach@ient.rwth-aachen.de>

# depending on the Automake conditionals set by configure, we will build the various
# script language interfaces that SWIG is capable of generating wrappers for

CV_INCLUDES = \
  -I$(top_srcdir)/cv/include         \
  -I$(top_srcdir)/cxcore/include     \
  -I$(top_srcdir)/otherlibs/highgui

AM_CPPFLAGS = \
  $(CV_INCLUDES)                     \
  -DCV_NO_BACKWARD_COMPATIBILITY

EXTRA_DIST = cv.h constants.h

distcleancheck_listfiles = \
  find -type f -exec sh -c 'test -f $(scrdir)/{} || echo {}'

# SWIG ###############################################################################

# the following rules keep wrappers up to date, if SWIG is present
if UPDATE_SWIG_WRAPPERS

# a work-around to force the build of cv.h and constants.h
# maybe there is a better way...
noinst_SCRIPTS = headers_generated
headers_generated: cv.h constants.h
	touch headers_generated
MOSTLYCLEANFILES = headers_generated

# this rule creates a single SWIG parseable header from cv.h and cxcore.h, etc.
cv.h:
	$(top_srcdir)/cv/include/cv.h \
	$(top_builddir)/cvver.h \
	$(top_srcdir)/cv/include/cvtypes.h \
	$(top_srcdir)/cxcore/include/cxcore.h \
	$(top_srcdir)/cxcore/include/cxtypes.h \
	$(top_srcdir)/cxcore/include/cxerror.h Makefile
	$(CPP) -x c++ -I$(top_builddir) $(AM_CPPFLAGS) $(CPPFLAGS) -DSKIP_INCLUDES -o $@ $(top_srcdir)/cv/include/cv.h

# this rule creates a single SWIG parseable header from cv.h and cxcore.h, etc.
constants.h: $(top_srcdir)/cv/include/cv.h $(top_srcdir)/cv/include/cvtypes.h \
             $(top_srcdir)/cxcore/include/cxcore.h $(top_srcdir)/cxcore/include/cxtypes.h \
             $(top_srcdir)/cxcore/include/cxerror.h Makefile
	$(PYTHON) $(top_srcdir)/utils/extract_constants.py $(top_srcdir)/cxcore/include/cxcore.h   > $@
	$(PYTHON) $(top_srcdir)/utils/extract_constants.py $(top_srcdir)/cxcore/include/cxtypes.h >> $@
	$(PYTHON) $(top_srcdir)/utils/extract_constants.py $(top_srcdir)/cxcore/include/cxerror.h >> $@
	$(PYTHON) $(top_srcdir)/utils/extract_constants.py $(top_srcdir)/cv/include/cv.h          >> $@
	$(PYTHON) $(top_srcdir)/utils/extract_constants.py $(top_srcdir)/cv/include/cvtypes.h     >> $@
	$(PYTHON) $(top_srcdir)/utils/extract_constants.py $(top_builddir)/cvver.h                >> $@

endif
