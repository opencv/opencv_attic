#!/usr/bin/python
"""
This program is demonstration for ellipse fitting. Program finds 
contours and approximate it by ellipses.

Trackbar specify threshold parametr.

White lines is contours. Red lines is fitting ellipses.

Original C implementation by:  Denis Burenkov.
Python implementation by: Roman Stanchak
"""

import sys
import urllib2
import cv

image02 = None
image03 = None
image04 = None

def process_image( slider_pos ): 
    """
    Define trackbar callback functon. This function find contours,
    draw it and approximate it by ellipses.
    """
    stor = cv.CreateMemStorage()
    
    # Threshold the source image. This needful for cv.FindContours().
    cv.Threshold( image03, image02, slider_pos, 255, cv.CV_THRESH_BINARY )
    
    # Find all contours.
    cont = cv.FindContours (image02,
            stor,
            cv.CV_RETR_LIST,
            cv.CV_CHAIN_APPROX_NONE,
            (0, 0))
    
    # Clear images. IPL use.
    cv.Zero(image02)
    cv.Zero(image04)
    
    print 'h_next', cont.h_next()
    print 'v_next', cont.v_next()

    # This cycle draw all contours and approximate it by ellipses.
    for (count, c) in enumerate(cont):

        # Number point must be more than or equal to 6 (for cv.FitEllipse_32f).        
        if( count < 6 ):
            continue
        
        # Alloc memory for contour point set.    
        PointArray = cv.CreateMat(1, count, cv.CV_32SC2)
        PointArray2D32f= cv.CreateMat( 1, count, cv.CV_32FC2)
        
        # Get contour point set.
        cv.CvtSeqToArray(c, PointArray, cv.Slice(0, cv.CV_WHOLE_SEQ_END_INDEX))
        
        # Convert CvPoint set to CvBox2D32f set.
        cv.Convert( PointArray, PointArray2D32f )
        
        box = cv.CvBox2D()

        # Fits ellipse to current contour.
        box = cv.FitEllipse2(PointArray2D32f)
        
        # Draw current contour.
        cv.DrawContours(image04, c, cv.CV_RGB(255,255,255), cv.CV_RGB(255,255,255),0,1,8,(0,0))
        
        # Convert ellipse data from float to integer representation.
        center = cv.CvPoint()
        size = cv.CvSize()
        center.x = cv.Round(box.center.x)
        center.y = cv.Round(box.center.y)
        size.width = cv.Round(box.size.width*0.5)
        size.height = cv.Round(box.size.height*0.5)
        box.angle = -box.angle
        
        # Draw ellipse.
        cv.Ellipse(image04, center, size,
                  box.angle, 0, 360,
                  cv.CV_RGB(0,0,255), 1, cv.CV_AA, 0)
    
    # Show image. HighGUI use.
    cv.ShowImage( "Result", image04 )


if __name__ == '__main__':
    if len(sys.argv) > 1:
        image03 = cv.LoadImage(sys.argv[1], cv.CV_LOAD_IMAGE_GRAYSCALE)
    else:
        url = 'https://code.ros.org/svn/opencv/trunk/opencv/samples/c/stuff.jpg'
        filedata = urllib2.urlopen(url).read()
        imagefiledata = cv.CreateMatHeader(1, len(filedata), cv.CV_8UC1)
        cv.SetData(imagefiledata, filedata, len(filedata))
        image03 = cv.DecodeImage(imagefiledata, cv.CV_LOAD_IMAGE_GRAYSCALE)
    
    slider_pos = 70

    # Create the destination images
    image02 = cv.CloneImage( image03 )
    image04 = cv.CloneImage( image03 )

    # Create windows.
    cv.NamedWindow("Source", 1)
    cv.NamedWindow("Result", 1)

    # Show the image.
    cv.ShowImage("Source", image03)

    # Create toolbars. HighGUI use.
    cv.CreateTrackbar( "Threshold", "Result", slider_pos, 255, process_image )


    process_image( 1 )

    #Wait for a key stroke; the same function arranges events processing                
    print "Press any key to exit"
    cv.WaitKey(0)

    cv.DestroyWindow("Source")
    cv.DestroyWindow("Result")

