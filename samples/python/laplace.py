#!/usr/bin/python
import urllib2
import cv
import sys

if __name__ == "__main__":
    laplace = None
    colorlaplace = None
    planes = [ None, None, None ]
    capture = None
    
    if len(sys.argv)==1:
        capture = cv.CreateCameraCapture(0)
    elif len(sys.argv)==2 and sys.argv[1].isdigit():
        capture = cv.CreateCameraCapture(int(sys.argv[1]))
    elif len(sys.argv)==2:
        capture = cv.CreateFileCapture(sys.argv[1]) 

    if not capture:
        print "Could not initialize capturing..."
        sys.exit(-1)
        
    cv.NamedWindow("Laplacian", 1)

    while True:
        frame = cv.QueryFrame(capture)
        if not frame:
            cv.WaitKey(0)
            break

        if not laplace:
            for i in range(len(planes)):
                planes[i] = cv.CreateImage(cv.Size(frame.width, frame.height), 8, 1)
            laplace = cv.CreateImage(cv.Size(frame.width, frame.height), IPL_DEPTH_16S, 1)
            colorlaplace = cv.CreateImage(cv.Size(frame.width, frame.height), 8, 3)

        cv.Split(frame, planes[0], planes[1], planes[2], None)
        for plane in planes:
            cv.Laplace(plane, laplace, 3)
            cv.ConvertScaleAbs(laplace, plane, 1, 0)

        cv.Merge(planes[0], planes[1], planes[2], None, colorlaplace)

        cv.ShowImage("Laplacian", colorlaplace)

        if cv.WaitKey(10) != -1:
            break

    cv.DestroyWindow("Laplacian")
