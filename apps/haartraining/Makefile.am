lib_LIBRARIES = libcvhaartraining.a

bin_PROGRAMS = opencv-haartraining opencv-createsamples opencv-performance

libcvhaartraining_a_SOURCES = cvboost.cpp cvhaarclassifier.cpp cvhaartraining.cpp cvcommon.cpp cvsamples.cpp
opencv_haartraining_SOURCES = haartraining.cpp
opencv_createsamples_SOURCES = createsamples.cpp
opencv_performance_SOURCES = performance.cpp

noinst_HEADERS = cvclassifier.h _cvhaartraining.h _cvcommon.h

EXTRA_DIST =

INCLUDES = -I../../../cxcore/include -I../../../cv/include -I../../../otherlibs/highgui -I. -I../include
LIBS = -L../../../otherlibs/highgui -lhighgui -L../../../cv/src -lcv -L../../../cxcore/src -lcxcore 

opencv_haartraining_LDFLAGS = -L. -lcvhaartraining
opencv_createsamples_LDFLAGS = -L. -lcvhaartraining
opencv_performance_LDFLAGS = -L. -lcvhaartraining

#CXX=@CXXOPENMP@
