#! /usr/bin/env python
"""
This script will test highgui's cvGetCaptureProperty() function
"""

# name of this test and it's requirements
TESTNAME = "cvGetCaptureProperty"
REQUIRED = ["cvCaptureFromFile"]

 
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
video = cvCaptureFromFile("/home/asbach/Data/video_test/qt_h263.mov")

# retrieve video dimensions and compare with known values
print str(cvGetCaptureProperty( video, CV_CAP_PROP_FOURCC ))

print "Checking image dimensions"
if cvGetCaptureProperty( video, CV_CAP_PROP_FRAME_WIDTH ) != 720:
	sys.exit(1)
if cvGetCaptureProperty( video, CV_CAP_PROP_FRAME_HEIGHT ) != 576:
	sys.exit(1)
print "pass"

print "Checking framerate"
if cvGetCaptureProperty( video, CV_CAP_PROP_FPS ) != 25:
	sys.exit(1)
print "pass"

print str(cvGetCaptureProperty( video, CV_CAP_PROP_FOURCC ) )

# ATTENTION: We do not release the video reader, window or any image.
# This is bad manners, but Python and OpenCV don't care...
	
# create flag file for sollowing tests
works.set_file(TESTNAME)

# return 0 ('PASS')
sys.exit(0)
