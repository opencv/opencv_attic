#! /usr/bin/env python

"""
This script checks HighGUI's frame seeking functionality
on a 3GP-compressed .3gp file.
"""

# name if this test and it's requirements
TESTNAME = "seek_frame_3gp"
REQUIRED = []

# needed for sys.exit(int), .works file handling and check routine
import sys
import works
import seek_test

# check requirements and delete old flag file, if it exists
if not works.check_files(REQUIRED,TESTNAME):
	sys.exit(77)

# name of file we check here
FILENAME='3gp.3gp'

# run check routine
result=seek_test.seek_frame_ok(FILENAME,0.25)

# create flag file for following tests
works.set_file(TESTNAME)

 # return result of test routine
sys.exit(result)
