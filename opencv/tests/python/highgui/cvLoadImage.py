#! /usr/bin/env python
"""
This script will test highgui's image loading functionality
"""

# name of this test and it's requirements
TESTNAME = "cvLoadImage"
REQUIRED = []

# needed for sys.exit(int) and .works file handling
import sys
import works

# check requirements and delete old flag file, if it exists
if not works.check_files(REQUIRED, TESTNAME):
	sys.exit(77)


# import the necessary things for OpenCV
import opencv
from opencv import highgui
from opencv import cv


# try to load an image from a file
image = highgui.cvLoadImage("../../samples/c/baboon.jpg")

# if the returned object is a valid IplImage (pointer)
# loading was successful.
if isinstance(image,cv.IplImagePtr):
	# create flag file for the following tests
	works.set_file(TESTNAME)
	# return 0 ('PASS')
	sys.exit(0)
# otherwise, it obviously failed.
else:
	sys.exit(1)

# ATTENTION: We do not release the image.
# This is bad manners, but Python and OpenCV don't care.
