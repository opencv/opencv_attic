#! /usr/bin/env octave -q
highgui

highgui.cvNamedWindow("win", highgui.CV_WINDOW_AUTOSIZE)
cap = highgui.cvCaptureFromFile("/home/asbach/Source/ObjectDetection/older/avi/table.avi")
img = highgui.cvQueryFrame(cap)

img
printf("Got frame of dimensions (%i x %i)",img.width,img.height);

highgui.cvShowImage("win", img)
highgui.cvMoveWindow("win", 200, 200)
highgui.cvWaitKey(0)

pilimg = opencv.Ipl2PIL(img)
pilimg

