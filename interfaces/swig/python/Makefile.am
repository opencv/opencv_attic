# opencv/interfaces/swig/python/Makefile.am
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


# SWIG ###############################################################################

# the following rules keep wrappers up to date, if SWIG is present
if UPDATE_SWIG_WRAPPERS

# this rule creates the Python wrapper sources for cxcore and cv
_cv.cpp: cv.i imagedata.i error.h error.cpp         \
         ../general/cv.i ../general/memory.i ../general/typemaps.i ../general/extensions.i ../general/doublepointers.i \
         ../filtered/cv.h ../filtered/constants.h Makefile
	$(SWIG) $(SWIG_PYTHON_OPT) $(CV_INCLUDES) -o $@ $<

# this rule creates the Python wrapper sources for highgui
_highgui.cpp: highgui.i ../general/highgui.i ../general/typemaps.i ../general/memory.i \
              $(top_srcdir)/otherlibs/highgui/highgui.h Makefile python
	$(SWIG) $(SWIG_PYTHON_OPT) $(CV_INCLUDES) -DSKIP_INCLUDES -o $@ $<

endif
