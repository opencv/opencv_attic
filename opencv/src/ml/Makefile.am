# use the default c++ flags
AM_CXXFLAGS=@DEF_CXXFLAGS@

INCLUDES = -I. -I$(top_srcdir)/ml/include -I$(top_srcdir)/cxcore/include -I$(top_srcdir)

noinst_HEADERS     = _ml.h
noinst_LTLIBRARIES = lib_ml.la
lib_LTLIBRARIES    = libml.la

# convenience libraries
lib_ml_la_SOURCES = \
    ml.cpp mlann_mlp.cpp mlboost.cpp mlcnn.cpp mlem.cpp \
    mlestimate.cpp mlknearest.cpp mlnbayes.cpp mlrtrees.cpp \
    mlsvm.cpp mltestset.cpp mltree.cpp ml_inner_functions.cpp   
lib_ml_la_LDFLAGS = -no-undefined @LDFLAGS@

# real library
libml_la_SOURCES = dummy.cpp
libml_la_LDFLAGS = -no-undefined -version-info @LT_VERSION@ @LDFLAGS@
libml_la_LIBADD = lib_ml.la \
    $(top_builddir)/cxcore/src/libcxcore.la \
    @LTLIBOBJS@
