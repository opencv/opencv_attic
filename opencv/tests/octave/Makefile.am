# test/octave/Makefile.am contains tests of the octave interface
# 2008-04-19, xavier.delacour@gmail.com


EXTRA_DIST = 

MOSTLYCLEANFILES = *.works

check_SCRIPTS = \
    test_adaptors.m

noinst_OCTAVE = \
    $(check_SCRIPTS)

TESTS_ENVIRONMENT = \
    top_srcdir=$(top_srcdir) \
    top_builddir=$(top_builddir) \
    LD_LIBRARY_PATH=$(top_builddir)/cxcore/src/.libs:$(top_builddir)/cv/src/.libs:$(top_builddir)/otherlibs/highgui/.libs:$(LD_LIBRARY_PATH) \
    DYLD_LIBRARY_PATH=$(top_builddir)/cxcore/src/.libs:$(top_builddir)/cv/src/.libs:$(top_builddir)/otherlibs/highgui/.libs:$(LD_LIBRARY_PATH) \
    OCTAVEPATH=$(top_builddir)/interfaces/swig/octave

# do tests only if we ask for octave

if BUILD_OCTAVE_WRAPPERS

TESTS = $(check_SCRIPTS)

endif

