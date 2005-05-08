lib_LTLIBRARIES = libcvaux.la

libcvaux_la_SOURCES = camshift.cpp cv3dtracker.cpp cvaux.cpp \
    cvauxutils.cpp cvcalibfilter.cpp cvcorrimages.cpp \
    cvcreatehandmask.cpp cvdpstereo.cpp cveigenobjects.cpp cvepilines.cpp \
    cvface.cpp cvfacedetection.cpp cvfacetemplate.cpp cvfindface.cpp \
    cvfindhandregion.cpp cvhmm.cpp cvhmm1d.cpp cvhmmobs.cpp cvlcm.cpp \
    cvlee.cpp cvlevmar.cpp cvlevmarprojbandle.cpp cvlevmartrif.cpp \
    cvlines.cpp cvlmeds.cpp cvmat.cpp cvmorphcontours.cpp cvmorphing.cpp \
    cvprewarp.cpp cvscanlines.cpp cvsegment.cpp cvsubdiv2.cpp cvtexture.cpp \
    cvtrifocal.cpp cvvecfacetracking.cpp cvvideo.cpp precomp.cpp

EXTRA_DIST = cvaux.dsp cvaux.vcproj cvaux.rc resource.h

include_HEADERS =
noinst_HEADERS = _cvaux.h _cvfacedetection.h _cvvectrack.h _cvvm.h

libcvaux_la_LDFLAGS = -no-undefined -version-info @LT_VERSION@ @LDFLAGS@
libcvaux_la_LIBADD  = $(top_builddir)/cv/src/libcv.la

INCLUDES = -I. -I$(top_srcdir)/cvaux/include -I$(top_srcdir)/cxcore/include -I$(top_srcdir)/cv/include -I$(top_srcdir)/cv/src -I$(top_srcdir)
