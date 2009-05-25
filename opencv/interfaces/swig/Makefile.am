# opencv/interfaces/swig/Makefile.am
# 2005-01-23, Mark Asbach <asbach@ient.rwth-aachen.de>

# depending on the Automake conditionals set by configure, we will build the various
# script language interfaces that SWIG is capable of generating wrappers for

CV_INCLUDES = \
  -I$(top_srcdir)/include/opencv

AM_CPPFLAGS = \
  $(CV_INCLUDES)                     \
  -DCV_NO_BACKWARD_COMPATIBILITY

if BUILD_OCTAVE_WRAPPERS
  TMP_OCTAVE_DIR = octave
else
  TMP_OCTAVE_DIR =
endif

SUBDIRS = filtered general python $(TMP_OCTAVE_DIR)

EXTRA_DIST = README CMakeLists.txt
