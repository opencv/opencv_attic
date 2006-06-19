#! /usr/bin/env python
"""
This script will test highgui's window functionality
"""

# name of this test and it's requirements
TESTNAME = "cvMoveResizeWindow"
REQUIRED = ["cvNamedWindow"]

 
# needed for sys.exit(int) and .works file handling
import sys
import works

# check requirements and delete old flag file, if it exists
if not works.check_files(REQUIRED,TESTNAME):
	sys.exit(77)

# import the necessary things for OpenCV
import opencv
from opencv.highgui import *
from opencv.cv import *

# some definitions
win_name = "testing..."

# create a window
cvNamedWindow(win_name,CV_WINDOW_AUTOSIZE)

# move the window around
cvMoveWindow(win_name,   0,   0)
cvWaitKey(100)
cvMoveWindow(win_name, 100,   0)
cvWaitKey(100)
cvMoveWindow(win_name, 100, 100)
cvWaitKey(100)
cvMoveWindow(win_name,   0, 100)
cvWaitKey(100)

# resize the window
for i in range(1,10):
	cvResizeWindow(win_name, i*100, i*100)
	cvWaitKey(100)

# destroy the window
cvDestroyWindow( win_name )

# create flag file for following tests
works.set_file(TESTNAME)

# return 0 (success)
sys.exit(0)
