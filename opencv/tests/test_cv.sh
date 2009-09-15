#! /bin/sh
if [ -n "${TESTDATA_DIR}" ] ; then
  ./cvtest -d $TESTDATA_DIR/cv
else
  ./cvtest -d $srcdir/../../opencv_extra/testdata/cv
fi
