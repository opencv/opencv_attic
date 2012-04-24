"""
This script will test highgui's cvQueryFrame() function
for different video formats
"""

# import the necessary things for OpenCV and comparson routine
import os
import python
from python.highgui import *
from python.cv import *
import match

# path to videos and images we need
PREFIX=os.environ["top_srcdir"]+"/tests/python/testdata/"

# this is the folder with the videos and images
# and name of output window
IMAGES		= PREFIX+"images/"
VIDEOS		= PREFIX+"videos/"

# testing routine, called for each entry in FILENAMES
# and compares each frame with corresponding frame in COMPARISON
def query_ok(FILENAME,THRESHOLD):

    # create a video reader using the tiny videofile VIDEOS+FILENAME
    video=cvCreateFileCapture(VIDEOS+FILENAME)

    if video is None:
	# couldn't open video (FAIL)
	return 1

    # call cvQueryFrame for 30 frames and check if the returned image is ok
    for k in range(0,29):
    	image=cvQueryFrame(video)

	if image is None:
	# returned image is NULL (FAIL)
		return 1

	result=match.match(image,k,THRESHOLD)
	if not result:
		return 1
	
	# ATTENTION: We do not release the video reader, window or any image.
	# This is bad manners, but Python and OpenCV don't care,
	# the whole memory segment will be freed on finish anyway...
	
    del video
    # everything is fine (PASS)
    return 0
