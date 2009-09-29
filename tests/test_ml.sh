#! /bin/sh
if [ -n "${TESTDATA_DIR}" ] ; then
  ./mltest -d $TESTDATA_DIR/ml
else
  ./mltest -d $srcdir/../../opencv_extra/testdata/ml
fi
