#! /usr/bin/env python

import urllib2
import cv

cap = highgui.cv.CreateFileCapture("../c/tree.avi")
img = highgui.cv.QueryFrame(cap)
print "Got frame of dimensions (", img.width, " x ", img.height, ")"

highgui.cv.NamedWindow("win", highgui.cv.CV_WINDOW_AUTOSIZE)
highgui.cv.ShowImage("win", img)
highgui.cv.MoveWindow("win", 200, 200)
highgui.cv.WaitKey(0)

