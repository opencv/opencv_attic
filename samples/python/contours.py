#! /usr/bin/env python

print "OpenCV Python version of contours"

# import the necessary things for OpenCV
from opencv import cv
from opencv import highgui

_SIZE = 500

_red = cv.cvScalar (0, 0, 255, 0);
_green = cv.cvScalar (0, 255, 0, 0);

_white = cv.cvRealScalar (255)
_black = cv.cvRealScalar (0)

if __name__ == '__main__':

    # create the image where we want to display results
    image = cv.cvCreateImage (cv.cvSize (_SIZE, _SIZE), 8, 1)

    # start with an empty image
    cv.cvSetZero (image)

    # draw the original picture
    for i in range (6):
        dx = (i % 2) * 250 - 30
        dy = (i / 2) * 150
        

        cv.cvEllipse (image,
                      cv.cvPoint (dx + 150, dy + 100),
                      cv.cvSize (100, 70),
                      0, 0, 360, _white, -1, 8, 0)
        cv.cvEllipse (image,
                      cv.cvPoint (dx + 115, dy + 70),
                      cv.cvSize (30, 20),
                      0, 0, 360, _black, -1, 8, 0)
        cv.cvEllipse (image,
                      cv.cvPoint (dx + 185, dy + 70),
                      cv.cvSize (30, 20),
                      0, 0, 360, _black, -1, 8, 0)
        cv.cvEllipse (image,
                      cv.cvPoint (dx + 115, dy + 70),
                      cv.cvSize (15, 15),
                      0, 0, 360, _white, -1, 8, 0)
        cv.cvEllipse (image,
                      cv.cvPoint (dx + 185, dy + 70),
                      cv.cvSize (15, 15),
                      0, 0, 360, _white, -1, 8, 0)
        cv.cvEllipse (image,
                      cv.cvPoint (dx + 115, dy + 70),
                      cv.cvSize (5, 5),
                      0, 0, 360, _black, -1, 8, 0)
        cv.cvEllipse (image,
                      cv.cvPoint (dx + 185, dy + 70),
                      cv.cvSize (5, 5),
                      0, 0, 360, _black, -1, 8, 0)
        cv.cvEllipse (image,
                      cv.cvPoint (dx + 150, dy + 100),
                      cv.cvSize (10, 5),
                      0, 0, 360, _black, -1, 8, 0)
        cv.cvEllipse (image,
                      cv.cvPoint (dx + 150, dy + 150),
                      cv.cvSize (40, 10),
                      0, 0, 360, _black, -1, 8, 0)
        cv.cvEllipse (image,
                      cv.cvPoint (dx + 27, dy + 100),
                      cv.cvSize (20, 35),
                      0, 0, 360, _white, -1, 8, 0)
        cv.cvEllipse (image,
                      cv.cvPoint (dx + 273, dy + 100),
                      cv.cvSize (20, 35),
                      0, 0, 360, _white, -1, 8, 0)

    # create window and display the original picture in it
    highgui.cvNamedWindow ("image", 1)
    highgui.cvShowImage ("image", image)

    # find the contours
    nb_contours, contours = cv.cvFindContours (image, cv.sizeof_CvContour,
                                               cv.CV_RETR_TREE,
                                               cv.CV_CHAIN_APPROX_SIMPLE,
                                               cv.cvPoint (0,0))

    # create the image for putting in it the founded contours
    contours_image = cv.cvCreateImage (cv.cvSize (_SIZE, _SIZE), 8, 3)

    # create the window for the contours
    highgui.cvNamedWindow ("contours", 1)
    
    # initialisations
    k = 0
    i = 0

    # loop until escape is pressed
    while k != 27:

        # first, clear the image where we will draw contours
        cv.cvSetZero (contours_image)

        # draw contours in red and green
        cv.cvDrawContours (contours_image, contours,
                           _red, _green,
                           i, 3, cv.CV_AA,
                           cv.cvPoint (0, 0))

        # show the final result
        highgui.cvShowImage ("contours", contours_image)

        # wait a key pressed
        k = highgui.cvWaitKey (10)

        # next i, to change the number of displayed contours
        i = (i + 1) % 5
