# on autotools setups, build cLapack only
# TODO: let ./configure find platform installed cLapack and only use this as fallback

AM_CPPFLAGS = -I$(top_srcdir)/3rdparty/include @OPTIMIZATION_CPPFLAGS@ -I$(top_srcdir)/3rdparty/flann -I$(top_srcdir)/3rdparty/flann/algorithms -I$(top_srcdir)/3rdparty/flann/nn -I$(top_srcdir)/3rdparty/flann/util
AM_CXXFLAGS = @OPTIMIZATION_CXXFLAGS@ @SIMD_FLAGS@ @OPENMP_CXXFLAGS@
AM_CFLAGS   = $(AM_CXXFLAGS) $(CXXFLAGS)

# =========================================================================

# sources for other platforms and conditionally compiled sources
EXTRA_DIST = \
  CMakeLists.txt	       \
  lapack/CMakeLists.txt  \
  \
  readme.txt             \
  \
  ilmimf                 \
  libjasper              \
  libjpeg                \
  libpng                 \
  libtiff                \
  zlib                   \
  \
  lib                    \
  \
  include/OpenEXR        \
  include/ffmpeg_        \
  include/jasper         \
  include/jconfig.h      \
  include/jmorecfg.h     \
  include/jpeglib.h      \
  include/msc_inttypes.h \
  include/msc_stdint.h   \
  include/png.h          \
  include/pngconf.h      \
  include/tiff.h         \
  include/tiffio.h       \
  include/tiffvers.h     \
  include/videoInput.h   \
  include/zconf.h        \
  include/zlib.h         \
  \
  lapack/COPYING

# =========================================================================

# headers that are needed at compile time of OpenCV only
noinst_HEADERS     = \
   include/cblas.h   \
   include/clapack.h \
   include/f2c.h     \
   \
   flann/algorithms/autotuned_index.h  \
   flann/algorithms/composite_index.h  \
   flann/algorithms/dist.h             \
   flann/algorithms/kdtree_index.h     \
   flann/algorithms/kmeans_index.h     \
   flann/algorithms/linear_index.h     \
   flann/algorithms/nn_index.h         \
   \
   flann/nn/ground_truth.h      \
   flann/nn/index_testing.h     \
   flann/nn/simplex_downhill.h  \
   \
   flann/constants.h  \
   flann/flann.h      \
   flann/flann.hpp    \
   \
   flann/util/allocator.h       \
   flann/util/common.h          \
   flann/util/heap.h            \
   flann/util/logger.h          \
   flann/util/matrix.h          \
   flann/util/object_factory.h  \
   flann/util/random.h          \
   flann/util/result_set.h      \
   flann/util/saving.h          \
   flann/util/timer.h

# all we want to build here is the single library
if NATIVE_LAPACK
  TMP_LAPACK = 
else
  TMP_LAPACK = lib_clapack.la lib_clapack_floatstore.la
endif

noinst_LTLIBRARIES = $(TMP_LAPACK) lib_flann.la

# sources of the cLapack convenience library part with special flags
lib_clapack_floatstore_la_SOURCES = \
  lapack/dlamch.c   lapack/slamch.c   
lib_clapack_floatstore_la_CFLAGS  = $(AM_CFLAGS) @FLOAT_STORE_CFLAGS@
lib_clapack_floatstore_la_LDFLAGS = -no-undefined -Wc,@OPENMP_CXXFLAGS@ @LDFLAGS@

# sources of the cLapack convenience library
lib_clapack_la_SOURCES = \
  lapack/dlasyf.c   lapack/dsyr.c     lapack/dsytf2.c   lapack/dsytrf.c   lapack/dsytri.c   lapack/dsytrs.c \
  lapack/dasum.c    lapack/daxpy.c    lapack/dbdsdc.c   lapack/dbdsqr.c   lapack/dcopy.c    lapack/ddot.c     lapack/dgebd2.c   lapack/dgebrd.c   \
  lapack/dgelq2.c   lapack/dgelqf.c   lapack/dgels.c    lapack/dgelsd.c   lapack/dgemm.c    lapack/dgemv.c    lapack/dgeqr2.c   lapack/dgeqrf.c   \
  lapack/dger.c     lapack/dgesdd.c   lapack/dgesv.c    lapack/dgetf2.c   lapack/dgetrf.c   lapack/dgetri.c   lapack/dgetrs.c   lapack/dlabad.c   \
  lapack/dlabrd.c   lapack/dlacpy.c   lapack/dlae2.c    lapack/dlaebz.c   lapack/dlaed0.c   lapack/dlaed1.c   lapack/dlaed2.c   lapack/dlaed3.c   \
  lapack/dlaed4.c   lapack/dlaed5.c   lapack/dlaed6.c   lapack/dlaed7.c   lapack/dlaed8.c   lapack/dlaed9.c   lapack/dlaeda.c   lapack/dlaev2.c   \
  lapack/dlagtf.c   lapack/dlagts.c   lapack/dlaisnan.c lapack/dlals0.c   lapack/dlalsa.c   lapack/dlalsd.c   lapack/dlamrg.c   \
  lapack/dlaneg.c   lapack/dlange.c   lapack/dlanst.c   lapack/dlansy.c   lapack/dlapy2.c   lapack/dlar1v.c   lapack/dlarf.c    lapack/dlarfb.c   \
  lapack/dlarfg.c   lapack/dlarft.c   lapack/dlarnv.c   lapack/dlarra.c   lapack/dlarrb.c   lapack/dlarrc.c   lapack/dlarrd.c   lapack/dlarre.c   \
  lapack/dlarrf.c   lapack/dlarrj.c   lapack/dlarrk.c   lapack/dlarrr.c   lapack/dlarrv.c   lapack/dlartg.c   lapack/dlaruv.c   lapack/dlas2.c    \
  lapack/dlascl.c   lapack/dlasd0.c   lapack/dlasd1.c   lapack/dlasd2.c   lapack/dlasd3.c   lapack/dlasd4.c   lapack/dlasd5.c   lapack/dlasd6.c   \
  lapack/dlasd7.c   lapack/dlasd8.c   lapack/dlasda.c   lapack/dlasdq.c   lapack/dlasdt.c   lapack/dlaset.c   lapack/dlasq1.c   lapack/dlasq2.c   \
  lapack/dlasq3.c   lapack/dlasq4.c   lapack/dlasq5.c   lapack/dlasq6.c   lapack/dlasr.c    lapack/dlasrt.c   lapack/dlassq.c   lapack/dlasv2.c   \
  lapack/dlaswp.c   lapack/dlatrd.c   lapack/dlauu2.c   lapack/dlauum.c   lapack/dlazq3.c   lapack/dlazq4.c   lapack/dnrm2.c    lapack/dorg2r.c   \
  lapack/dorgbr.c   lapack/dorgl2.c   lapack/dorglq.c   lapack/dorgqr.c   lapack/dorm2l.c   lapack/dorm2r.c   lapack/dormbr.c   lapack/dorml2.c   \
  lapack/dormlq.c   lapack/dormql.c   lapack/dormqr.c   lapack/dormtr.c   lapack/dpotf2.c   lapack/dpotrf.c   lapack/dpotri.c   lapack/dpotrs.c   \
  lapack/drot.c     lapack/dscal.c    lapack/dstebz.c   lapack/dstein.c   lapack/dstemr.c   lapack/dsteqr.c   lapack/dsterf.c   lapack/dswap.c    \
  lapack/dsyevr.c   lapack/dsymv.c    lapack/dsyr2.c    lapack/dsyr2k.c   lapack/dsyrk.c    lapack/dsytd2.c   lapack/dsytrd.c   lapack/dtrmm.c    \
  lapack/dtrmv.c    lapack/dtrsm.c    lapack/dtrti2.c   lapack/dtrtri.c   lapack/dtrtrs.c   lapack/f77_aloc.c lapack/idamax.c   lapack/ieeeck.c   \
  lapack/ilaenv.c   lapack/iparmq.c   lapack/isamax.c   lapack/pow_di.c   lapack/pow_ii.c   lapack/pow_ri.c   lapack/precomp.c  lapack/s_cat.c    \
  lapack/s_cmp.c    lapack/s_copy.c   lapack/sasum.c    lapack/saxpy.c    lapack/sbdsdc.c   lapack/sbdsqr.c   lapack/scopy.c    lapack/sdot.c     \
  lapack/sgebd2.c   lapack/sgebrd.c   lapack/sgelq2.c   lapack/sgelqf.c   lapack/sgels.c    lapack/sgelsd.c   lapack/sgemm.c    lapack/sgemv.c    \
  lapack/sgeqr2.c   lapack/sgeqrf.c   lapack/sger.c     lapack/sgesdd.c   lapack/sgesv.c    lapack/sgetf2.c   lapack/sgetrf.c   lapack/sgetri.c   \
  lapack/sgetrs.c   lapack/slabad.c   lapack/slabrd.c   lapack/slacpy.c   lapack/slae2.c    lapack/slaebz.c   lapack/slaed0.c   lapack/slaed1.c   \
  lapack/slaed2.c   lapack/slaed3.c   lapack/slaed4.c   lapack/slaed5.c   lapack/slaed6.c   lapack/slaed7.c   lapack/slaed8.c   lapack/slaed9.c   \
  lapack/slaeda.c   lapack/slaev2.c   lapack/slagtf.c   lapack/slagts.c   lapack/slaisnan.c lapack/slals0.c   lapack/slalsa.c   lapack/slalsd.c   \
  lapack/slamrg.c   lapack/slaneg.c   lapack/slange.c   lapack/slanst.c   lapack/slansy.c   lapack/slapy2.c   lapack/slar1v.c   \
  lapack/slarf.c    lapack/slarfb.c   lapack/slarfg.c   lapack/slarft.c   lapack/slarnv.c   lapack/slarra.c   lapack/slarrb.c   lapack/slarrc.c   \
  lapack/slarrd.c   lapack/slarre.c   lapack/slarrf.c   lapack/slarrj.c   lapack/slarrk.c   lapack/slarrr.c   lapack/slarrv.c   lapack/slartg.c   \
  lapack/slaruv.c   lapack/slas2.c    lapack/slascl.c   lapack/slasd0.c   lapack/slasd1.c   lapack/slasd2.c   lapack/slasd3.c   lapack/slasd4.c   \
  lapack/slasd5.c   lapack/slasd6.c   lapack/slasd7.c   lapack/slasd8.c   lapack/slasda.c   lapack/slasdq.c   lapack/slasdt.c   lapack/slaset.c   \
  lapack/slasq1.c   lapack/slasq2.c   lapack/slasq3.c   lapack/slasq4.c   lapack/slasq5.c   lapack/slasq6.c   lapack/slasr.c    lapack/slasrt.c   \
  lapack/slassq.c   lapack/slasv2.c   lapack/slaswp.c   lapack/slatrd.c   lapack/slauu2.c   lapack/slauum.c   lapack/slazq3.c   lapack/slazq4.c   \
  lapack/snrm2.c    lapack/sorg2r.c   lapack/sorgbr.c   lapack/sorgl2.c   lapack/sorglq.c   lapack/sorgqr.c   lapack/sorm2l.c   lapack/sorm2r.c   \
  lapack/sormbr.c   lapack/sorml2.c   lapack/sormlq.c   lapack/sormql.c   lapack/sormqr.c   lapack/sormtr.c   lapack/spotf2.c   lapack/spotrf.c   \
  lapack/spotri.c   lapack/spotrs.c   lapack/srot.c     lapack/sscal.c    lapack/sstebz.c   lapack/sstein.c   lapack/sstemr.c   lapack/ssteqr.c   \
  lapack/ssterf.c   lapack/sswap.c    lapack/ssyevr.c   lapack/ssymv.c    lapack/ssyr2.c    lapack/ssyr2k.c   lapack/ssyrk.c    lapack/ssytd2.c   \
  lapack/ssytrd.c   lapack/strmm.c    lapack/strmv.c    lapack/strsm.c    lapack/strti2.c   lapack/strtri.c   lapack/strtrs.c   lapack/xerbla.c

# the cLapack convenience library
lib_clapack_la_LDFLAGS = -no-undefined -Wc,@OPENMP_CXXFLAGS@ @LDFLAGS@
lib_clapack_la_LIBADD  = lib_clapack_floatstore.la


# sources of the FLANN convenience library
lib_flann_la_SOURCES = \
  flann/algorithms/dist.cpp   \
  flann/nn/index_testing.cpp  \
  flann/flann.cpp             \
  flann/util/logger.cpp       \
  flann/util/random.cpp       \
  flann/util/saving.cpp
