INCLUDES = -I$(top_srcdir)/include/opencv -I.

# use the default c++ flags
AM_CXXFLAGS=@DEF_CXXFLAGS@

lib_LIBRARIES = libcvhaartraining.a

bin_PROGRAMS = opencv-haartraining opencv-createsamples opencv-performance

ALL_CV_LIBS = \
  libcvhaartraining.a                     \
  $(top_builddir)/src/libhighgui.la       \
  $(top_builddir)/src/libcv.la            \
  $(top_builddir)/src/libcxcore.la

libcvhaartraining_a_SOURCES = \
  haartraining/cvboost.cpp           \
  haartraining/cvhaarclassifier.cpp  \
  haartraining/cvhaartraining.cpp    \
  haartraining/cvcommon.cpp          \
  haartraining/cvsamples.cpp

noinst_HEADERS = \
  haartraining/cvclassifier.h    \
  haartraining/cvhaartraining.h  \
  haartraining/_cvhaartraining.h \
  haartraining/_cvcommon.h

opencv_haartraining_SOURCES = haartraining/haartraining.cpp
opencv_haartraining_LDADD = $(ALL_CV_LIBS)

opencv_createsamples_SOURCES = haartraining/createsamples.cpp
opencv_createsamples_LDADD = $(ALL_CV_LIBS)

opencv_performance_SOURCES = haartraining/performance.cpp
opencv_performance_LDADD = $(ALL_CV_LIBS)

