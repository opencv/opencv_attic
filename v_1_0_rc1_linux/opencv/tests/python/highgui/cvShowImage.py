#! /usr/bin/env python
"""
This script will test highgui's window functionality
"""

# name of this test and it's requirements
TESTNAME = "cvShowImage"
REQUIRED = ["cvLoadImagejpg", "cvNamedWindow"]

 
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

# defined window name
win_name = "testing..."

# we expect a window to be createable, thanks to 'cvNamedWindow.works'
cvNamedWindow(win_name, CV_WINDOW_AUTOSIZE)

# we expect the image to be loadable, thanks to 'cvLoadImage.works'
image = cvLoadImage("../../cvShowImage.jpg")

# try to show image in window
res = cvShowImage( win_name, image )

if res == 0:
	cvReleaseImage(image)
	cvDestroyWindow(win_name)
	sys.exit(1)
	
# destroy window
cvDestroyWindow(win_name)

# create flag file for following tests
works.set_file(TESTNAME)

# return 0 ('PASS')
sys.exit(0)
	
	
	
