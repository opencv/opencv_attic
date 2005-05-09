lib_LIBRARIES = libcvhaartraining.a

bin_PROGRAMS = opencv-haartraining opencv-createsamples opencv-performance

libcvhaartraining_a_SOURCES = cvboost.cpp cvhaarclassifier.cpp cvhaartraining.cpp cvcommon.cpp cvsamples.cpp
opencv_haartraining_SOURCES = haartraining.cpp
opencv_createsamples_SOURCES = createsamples.cpp
opencv_performance_SOURCES = performance.cpp

noinst_HEADERS = cvclassifier.h _cvhaartraining.h _cvcommon.h

EXTRA_DIST =

INCLUDES = -I$(top_srcdir)/cxcore/include -I$(top_srcdir)/cv/include \
           -I$(top_srcdir)/otherlibs/highgui -I. -I$(top_srcdir)/apps/haartraining/include

LIBS = -L$(top_builddir)/otherlibs/highgui -lhighgui \
       -L$(top_builddir)/cv/src -lcv \
       -L$(top_builddir)/cxcore/src -lcxcore

opencv_haartraining_LDFLAGS = -L. -lcvhaartraining
opencv_createsamples_LDFLAGS = -L. -lcvhaartraining
opencv_performance_LDFLAGS = -L. -lcvhaartraining

#CXX=@CXXOPENMP@
