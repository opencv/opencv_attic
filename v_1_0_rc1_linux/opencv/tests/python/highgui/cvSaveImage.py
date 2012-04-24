#! /usr/bin/env python
"""
This script will test highgui's image saving functionality
"""

# name if this test and it's requirements
TESTNAME = "cvSaveImage"
REQUIRED = ["cvLoadImagejpg"]

#needed for sys.exit(int), filehandling and .works file checks
import sys
import os
import works

# delete old .works file and check requirements
if not works.check_files(REQUIRED,TESTNAME):
	sys.exit(77)

# import the necessary things for OpenCV
import opencv
from opencv.highgui import *
from opencv.cv import *

# our temporary test file
file_name = "./highgui_testfile.bmp"

# try to load an image from a file
image = cvLoadImage("../../samples/c/baboon.jpg")

# if the returned object is a valid IplImage (pointer)
# loading was successful.
if not isinstance(image,opencv.cv.IplImagePtr):
	print "(INFO) Couldn't load test image. Skipping rest of this test."
	sys.exit(77)

res = cvSaveImage("./highgui_testfile.bmp", image)

if res == 0:
	print "(ERROR) Couldn't save image to '"+file_name+"'."
	sys.exit(1)

# remove temporary file
os.remove(file_name)

# create flag file
works.set_file(TESTNAME)

# return 0 ('PASS')
sys.exit(0)
