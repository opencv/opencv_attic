#! /usr/bin/env python
"""
This script will test highgui's video reading functionality
"""

# name of this test and it's requirements
TESTNAME = "cvCaptureFromFile"
REQUIRED = []

 
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


# create a video reader using the tiny video 'vd_uncompressed.avi'
video = cvCaptureFromFile("/home/asbach/Data/video_test/vd_uncompressed.avi")

if video is None:
	print "(ERROR) Couldn't create reader object."
	sys.exit(1)

# ATTENTION: We do not release the video reader.
# This is bad manners, but Python and OpenCV don't care...
	
# create flag file for following tests
works.set_file(TESTNAME)

# return 0 ('PASS')
sys.exit(0)
