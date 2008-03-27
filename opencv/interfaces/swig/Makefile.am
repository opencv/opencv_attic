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

SUBDIRS = filtered general python octave

EXTRA_DIST = README
