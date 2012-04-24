"""
This script will test HighGUI's cvGetCaptureProperty functionality
for correct returnvalues of width and height information for different video formats
"""

# import the necessary things for OpenCV and comparson routine
import os
import python
from python.highgui import *


# path to images and videos  we need
PREFIX		=os.environ["top_srcdir"]+"/tests/python/testdata/"


# this is the folder with the videos and images
# and name of output window
IMAGES		= PREFIX+"images/"
VIDEOS		= PREFIX+"videos/"


# testing routine, seeks through file and compares read images with frames in COMPARISON
def size_ok(FILENAME):
  # create a video reader using the tiny videofile VIDEOS+FILENAME
  video=cvCreateFileCapture(VIDEOS+FILENAME)

  if video is None:
    # couldn't open video (FAIL)
    return 1

  # get width and height information via HighGUI's cvGetCaptureProperty function
  w=cvGetCaptureProperty(video,CV_CAP_PROP_FRAME_WIDTH)
  h=cvGetCaptureProperty(video,CV_CAP_PROP_FRAME_HEIGHT)

  # get an image to compare
  image=cvQueryFrame(video)

  if (w!=image.width) or (h!=image.height):
    # dimensions don't match parameters (FAIL)
    return 1

  del video
  del image
  # everything is fine (PASS)
  return 0


