#! /usr/bin/env python

import urllib2
import cv

cap = cv.CreateFileCapture("../samples/c/tree.avi")
img = cv.QueryFrame(cap)
print "Got frame of dimensions (", img.width, " x ", img.height, ")"

cv.NamedWindow("win", highgui.cv.CV_WINDOW_AUTOSIZE)
cv.ShowImage("win", img)
cv.MoveWindow("win", 200, 200)
cv.WaitKey(0)
