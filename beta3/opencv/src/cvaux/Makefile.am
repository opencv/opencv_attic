lib_LTLIBRARIES = libcvaux.la

libcvaux_la_SOURCES = \
cv3dtracker.cpp    cvface.cpp             cvhmm1d.cpp          \
cvaux.cpp          cvfacedetection.cpp    cvlcm.cpp            cvsubdiv2.cpp \
cvauxutils.cpp     cvfacetemplate.cpp     cvlee.cpp            cvtexture.cpp \
cvcalibfilter.cpp  cvfindface.cpp         cvmat.cpp            cvvecfacetracking.cpp \
cvdpstereo.cpp     cvhaar.cpp             cvmorphcontours.cpp  cvvideo.cpp \
cvepilines.cpp     cvhaarfacecascade.cpp  cvsegment.cpp        precomp.cpp

EXTRA_DIST = cvaux.rc resource.h

SOMAJOR = @MMAJOR@
SOMINOR = @MMINOR@
SOSUBMINOR = @MSUBMINOR@

# here you should add all header files, which contains the functionality
# of your shared lib (published for the users)
include_HEADERS =

# here you should add all header files, which shouldn't be installed
noinst_HEADERS = _cvaux.h  _cvfacedetection.h  _cvvectrack.h

libcvaux_la_LDFLAGS = -no-undefined -version-info $(LT_CURRENT):$(LT_REVISION):$(LT_AGE) -release $(LT_RELEASE) @LDFLAGS@

# add here the libraries which have to be include
# for building the shared lib
#
#  please consider also the usage of the make environment variables
#  like
#  $(LIB_KDEUI), $(LIB_QT), $(LIB_HTML), etc.
#  instead of the hardcoded entries, which have placed
#  in here.
#  The following line is only a suggestion!
#
libcvaux_la_LIBADD = 

INCLUDES = -I. -I../include -I../../cv/include -I../../cv/src -I../..
CXXFLAGS = @CXXFLAGS@ -Wall
