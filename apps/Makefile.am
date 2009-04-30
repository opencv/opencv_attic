AM_CPPFLAGS = @OPTIMIZATION_CPPFLAGS@ @DEBUG_CPPFLAGS@ -I$(top_srcdir)/include/opencv
AM_CXXFLAGS = @OPTIMIZATION_CXXFLAGS@ @DEBUG_CXXFLAGS@ @SIMD_FLAGS@ @OPENMP_CXXFLAGS@
AM_LDFLAGS  = @OPENMP_CXXFLAGS@

EXTRA_DIST = \
  CMakeLists.txt \
  haartraining/CMakeLists.txt 

noinst_LTLIBRARIES = libcvhaartraining.la

bin_PROGRAMS = opencv-haartraining opencv-createsamples opencv-performance

ALL_CV_LIBS = \
  $(top_builddir)/src/libhighgui.la       \
  $(top_builddir)/src/libcv.la            \
  $(top_builddir)/src/libcxcore.la

libcvhaartraining_la_SOURCES = \
  haartraining/cvboost.cpp           \
  haartraining/cvhaarclassifier.cpp  \
  haartraining/cvhaartraining.cpp    \
  haartraining/cvcommon.cpp          \
  haartraining/cvsamples.cpp
libcvhaartraining_la_LIBADD = @LTLIBOBJS@

noinst_HEADERS = \
  haartraining/cvclassifier.h    \
  haartraining/cvhaartraining.h  \
  haartraining/_cvhaartraining.h \
  haartraining/_cvcommon.h

opencv_haartraining_SOURCES = haartraining/haartraining.cpp
opencv_haartraining_LDADD = libcvhaartraining.la $(ALL_CV_LIBS)

opencv_createsamples_SOURCES = haartraining/createsamples.cpp
opencv_createsamples_LDADD = libcvhaartraining.la $(ALL_CV_LIBS)

opencv_performance_SOURCES = haartraining/performance.cpp
opencv_performance_LDADD = libcvhaartraining.la $(ALL_CV_LIBS)

