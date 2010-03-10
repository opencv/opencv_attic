#!/usr/bin/python
import urllib2
import sys
import cv

marker_mask = None
markers = None
img0 = None
img = None
img_gray = None 
wshed = None
prev_pt = (-1, -1)

def on_mouse(event, x, y, flags, param):
    global prev_pt
    if(not img):
        return
    if(event == cv.CV_EVENT_LBUTTONUP or not (flags & cv. CV_EVENT_FLAG_LBUTTON)):
        prev_pt = (-1, -1)
    elif(event == cv.CV_EVENT_LBUTTONDOWN):
        prev_pt = (x, y)
    elif(event == cv.CV_EVENT_MOUSEMOVE and (flags & cv. CV_EVENT_FLAG_LBUTTON)):
        pt = (x, y)
        if(prev_pt.x < 0):
            prev_pt = pt
        cv.Line(marker_mask, prev_pt, pt, cv.ScalarAll(255), 5, 8, 0)
        cv.Line(img, prev_pt, pt, cv.ScalarAll(255), 5, 8, 0)
        prev_pt = pt
        cv.ShowImage("image", img)

if __name__ == "__main__":
    filename = "../c/fruits.jpg"
    if len(sys.argv)>1:
        filename = sys.argv[1]

    rng = cv.RNG(-1)
    img0 = cv.LoadImage(filename, 1)
    if not img0:
        print "Error opening image '%s'" % filename
        sys.exit(-1)

    print "Hot keys:"
    print "\tESC - quit the program"
    print "\tr - restore the original image"
    print "\tw - run watershed algorithm"
    print "\t  (before that, roughly outline several markers on the image)"

    cv.NamedWindow("image", 1)
    cv.NamedWindow("watershed transform", 1)

    img = cv.CloneImage(img0)
    img_gray = cv.CloneImage(img0)
    wshed = cv.CloneImage(img0)
    marker_mask = cv.CreateImage(cv.GetSize(img), 8, 1)
    markers = cv.CreateImage(cv.GetSize(img), IPL_DEPTH_32S, 1)

    cv.CvtColor(img, marker_mask, cv.CV_BGR2GRAY)
    cv.CvtColor(marker_mask, img_gray, cv.CV_GRAY2BGR)

    cv.Zero(marker_mask)
    cv.Zero(wshed)

    cv.ShowImage("image", img)
    cv.ShowImage("watershed transform", wshed)

    cv.SetMouseCallback("image", on_mouse, None)
    while True:
        c = cv.WaitKey(0)
        if c=='\x1b':
            break
        if c == 'r':
            cv.Zero(marker_mask)
            cv.Copy(img0, img)
            cv.ShowImage("image", img)
        if c == 'w':
            storage = cv.CreateMemStorage(0)
            comp_count = 0
            #cv.SaveImage("wshed_mask.png", marker_mask)
            #marker_mask = cv.LoadImage("wshed_mask.png", 0)
            nb_cont, contours = cv.FindContours(marker_mask, storage, sizeof_CvContour,
                            cv.CV_RETR_CCOMP, cv. CV_CHAIN_APPROX_SIMPLE)
            cv.Zero(markers)
            while contours:
                cv.DrawContours(markers, contours, cv.ScalarAll(comp_count+1),
                                cv.ScalarAll(comp_count+1), -1, -1, 8, (0, 0))
                contours=contours.h_next
                comp_count+=1
            color_tab = cv.CreateMat(comp_count, 1, cv.CV_8UC3)
            for i in range(comp_count):
                color_tab[i] = cv.Scalar(cv.RandInt(rng)%180 + 50, 
                                 cv.RandInt(rng)%180 + 50, 
                                 cv.RandInt(rng)%180 + 50)
            t = cv.GetTickCount()
            cv.Watershed(img0, markers)
            t = cv.GetTickCount() - t
            #print "exec time = %f" % t/(cv.GetTickFrequency()*1000.)

            cv.Set(wshed, cv.ScalarAll(255))

            # paint the watershed image
            for j in range(markers.height):
                for i in range(markers.width):
                    idx = markers[j, i]
                    if idx==-1:
                        continue
                    idx = idx-1
                    wshed[j, i] = color_tab[idx, 0]

            cv.AddWeighted(wshed, 0.5, img_gray, 0.5, 0, wshed)
            cv.ShowImage("watershed transform", wshed)
            cv.WaitKey()
