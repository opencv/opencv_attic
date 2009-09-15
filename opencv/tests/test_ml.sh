#! /bin/sh
if [ -n "${TESTDATA_DIR}" ] ; then
  ./mltest -d $TESTDATA_DIR/ml -f $TESTDATA_DIR/ml/validation.xml
else
  ./mltest -d $srcdir/../../opencv_extra/testdata/ml -f $srcdir/../../opencv_extra/testdata/ml/validation.xml
fi
