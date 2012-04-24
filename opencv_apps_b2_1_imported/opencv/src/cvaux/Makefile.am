lib_LTLIBRARIES = libcvaux.la

libcvaux_la_SOURCES = correspond.cpp cvaux.cpp cvbilateralfiltering.cpp cvcalibfilter.cpp cvclique.cpp cvcontourex.cpp cvepilines.cpp cvgraphex.cpp cvhmm1d.cpp cvsegment.cpp cvsubdiv2.cpp cvtexture.cpp cvvideo.cpp decomppoly.cpp enmin.cpp extendededges.cpp precomp.cpp

EXTRA_DIST = 

SOMAJOR = @MMAJOR@
SOMINOR = @MMINOR@
SOSUBMINOR = @MSUBMINOR@

# here you should add all header files, which contains the functionality
# of your shared lib (published for the users)
include_HEADERS =

# here you should add all header files, which shouldn't be installed
noinst_HEADERS = _cvaux.h

libcvaux_la_LDFLAGS = $(all_libraries) -no-undefined -version-info $(SOMAJOR):$(SOMINOR):$(SOSUBMINOR) $(LDFLAGS)

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

INCLUDES += -I../include -I. -I../../cv/include -I../../cv/src
CFLAGS += -Wall @DEBUG@
CXXFLAGS += -Wall @DEBUG@
