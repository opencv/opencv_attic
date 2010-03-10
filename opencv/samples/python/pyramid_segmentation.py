#!/usr/bin/python
import sys
import urllib2
import cv

image =  [None, None]
image0 = None
image1 = None
threshold1 = 255
threshold2 = 30
l = level = 4
block_size = 1000
filter = cv.CV_GAUSSIAN_5x5
storage = None
min_comp = CvConnectedComp()

def set_thresh1(val):
    global threshold1
    threshold1 = val
    ON_SEGMENT()

def set_thresh2(val):
    global threshold2
    threshold2 = val
    ON_SEGMENT()

def ON_SEGMENT():
    global storage
    global min_comp
    comp = cv.PyrSegmentation(image0, image1, storage, level, threshold1+1, threshold2+1)
    cv.ShowImage("Segmentation", image1)

if __name__ == "__main__":
    filename = "../c/fruits.jpg"
    if len(sys.argv) == 2:
        filename = sys.argv[1]
    image[0] = cv.LoadImage(filename, 1)
    if not image[0]:
        print "Error opening %s" % filename
        sys.exit(-1)

    cv.NamedWindow("Source", 0)
    cv.ShowImage("Source", image[0])
    cv.NamedWindow("Segmentation", 0)
    storage = cv.CreateMemStorage (block_size)
    image[0].width &= -(1<<level)
    image[0].height &= -(1<<level)
    image0 = cv.CloneImage(image[0])
    image1 = cv.CloneImage(image[0])
    # segmentation of the color image
    l = 1
    threshold1 =255
    threshold2 =30
    ON_SEGMENT()
    sthreshold1 = cv.CreateTrackbar("Threshold1", "Segmentation", threshold1, 255, set_thresh1)
    sthreshold2 = cv.CreateTrackbar("Threshold2", "Segmentation",  threshold2, 255, set_thresh2)
    cv.ShowImage("Segmentation", image1)
    cv.WaitKey(0)
    cv.DestroyWindow("Segmentation")
    cv.DestroyWindow("Source")
