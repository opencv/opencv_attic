# use the default c++ flags
AM_CXXFLAGS=@DEF_CXXFLAGS@

INCLUDES = -I. -I$(top_srcdir)/cvaux/include -I$(top_srcdir)/cxcore/include -I$(top_srcdir)/cv/include -I$(top_srcdir)/cv/src -I$(top_srcdir)

noinst_HEADERS     = _cvaux.h _cvfacedetection.h _cvvectrack.h _cvvm.h
noinst_LTLIBRARIES = lib_cvaux.la
lib_LTLIBRARIES    = libcvaux.la

# convenience library
lib_cvaux_la_SOURCES = \
    camshift.cpp cv3dtracker.cpp cvaux.cpp cvauxutils.cpp \
    cvbgfg_acmmm2003.cpp cvbgfg_codebook.cpp cvbgfg_common.cpp cvbgfg_gaussmix.cpp \
    cvcalibfilter.cpp cvclique.cpp cvcorrespond.cpp cvcorrimages.cpp \
    cvcreatehandmask.cpp cvdpstereo.cpp cveigenobjects.cpp \
    cvepilines.cpp cvface.cpp cvfacedetection.cpp cvfacetemplate.cpp \
    cvfindface.cpp cvfindhandregion.cpp cvhmm1d.cpp cvhmm.cpp \
    cvhmmobs.cpp cvlcm.cpp cvlee.cpp cvlevmar.cpp cvlevmarprojbandle.cpp \
    cvlevmartrif.cpp cvlines.cpp cvlmeds.cpp cvmat.cpp cvmorphcontours.cpp \
    cvmorphing.cpp cvprewarp.cpp cvscanlines.cpp cvsegment.cpp \
    cvsubdiv2.cpp cvtexture.cpp cvtrifocal.cpp cvvecfacetracking.cpp \
    cvvideo.cpp decomppoly.cpp enmin.cpp extendededges.cpp \
    precomp.cpp \
    \
    vs/bgfg_estimation.cpp              \
    vs/blobtrackanalysis.cpp            \
    vs/blobtrackanalysishist.cpp        \
    vs/blobtrackanalysisior.cpp         \
    vs/blobtrackanalysistrackdist.cpp   \
    vs/blobtrackgen1.cpp                \
    vs/blobtrackgenyml.cpp              \
    vs/blobtrackingauto.cpp             \
    vs/blobtrackingcc.cpp               \
    vs/blobtrackingccwithcr.cpp         \
    vs/blobtrackingkalman.cpp           \
    vs/blobtrackinglist.cpp             \
    vs/blobtrackingmsfg.cpp             \
    vs/blobtrackingmsfgs.cpp            \
    vs/blobtrackpostprockalman.cpp      \
    vs/blobtrackpostproclinear.cpp      \
    vs/blobtrackpostproclist.cpp        \
    vs/enteringblobdetection.cpp        \
    vs/enteringblobdetectionreal.cpp    \
    vs/testseq.cpp

lib_cvaux_la_LDFLAGS = -no-undefined @LDFLAGS@

# real library
libcvaux_la_SOURCES = dummy.cpp
libcvaux_la_LDFLAGS = -no-undefined -version-info @LT_VERSION@ @LDFLAGS@
libcvaux_la_LIBADD  = \
    lib_cvaux.la \
    $(top_builddir)/cxcore/src/libcxcore.la \
    $(top_builddir)/cv/src/libcv.la         \
    @LTLIBOBJS@
