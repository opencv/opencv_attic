#! /usr/bin/env python

"""
This script checks HighGUI's cvQueryFrame function
on a DV-compressed .dv file.
"""

# name if this test and it's requirements
TESTNAME = "query_dv_pal_progressive_dv"
REQUIRED = []

# needed for sys.exit(int), .works file handling and check routine
import sys
import works
import query_test

# check requirements and delete old flag file, if it exists
if not works.check_files(REQUIRED,TESTNAME):
	sys.exit(77)

# name of file we check here
FILENAME='dv_pal_progressive.dv'

# run check routine
result=query_test.query_ok(FILENAME,0.37)

# create flag file for following tests
works.set_file(TESTNAME)

 # return result of test routine
sys.exit(result)
