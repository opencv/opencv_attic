#! /bin/sh
if [ -n "${TESTDATA_DIR}" ] ; then
  ./cxcoretest -d $TESTDATA_DIR/cxcore
else
  ./cxcoretest -d $srcdir/../../opencv_extra/testdata/cxcore
fi
