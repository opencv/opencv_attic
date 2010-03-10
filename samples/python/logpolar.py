#!/usr/bin/python
import sys
import urllib2
import cv

src=None
dst=None
src2=None

def on_mouse(event, x, y, flags, param):

    if(not src):
        return

    if event==cv.CV_EVENT_LBUTTONDOWN:
        cv.LogPolar(src, dst, 2D32f(x, y), 40, cv.CV_INTER_LINEAR+CV_WARP_FILL_OUTLIERS)
        cv.LogPolar(dst, src2, 2D32f(x, y), 40, cv.CV_INTER_LINEAR+CV_WARP_FILL_OUTLIERS+CV_WARP_INVERSE_MAP)
        cv.ShowImage("log-polar", dst)
        cv.ShowImage("inverse log-polar", src2)

if __name__ == "__main__":
    
    filename = "../c/fruits.jpg"
    if len(sys.argv)>1:
        filename=argv[1]
    
    src = cv.LoadImage(filename, 1)
    if not src:
        print "Could not open %s" % filename
        sys.exit(-1)
        
    cv.NamedWindow("original", 1)
    cv.NamedWindow("log-polar", 1)
    cv.NamedWindow("inverse log-polar", 1)
  
    
    dst = cv.CreateImage(cv.Size(256, 256), 8, 3)
    src2 = cv.CreateImage(cv.GetSize(src), 8, 3)
    
    cv.SetMouseCallback("original", on_mouse)
    on_mouse(cv.CV_EVENT_LBUTTONDOWN, src.width/2, src.height/2, None, None)
    
    cv.ShowImage("original", src)
    cv.WaitKey()
