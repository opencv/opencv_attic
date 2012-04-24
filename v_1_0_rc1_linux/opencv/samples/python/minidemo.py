import opencv
from opencv import highgui

highgui.cvNamedWindow("win", highgui.CV_WINDOW_AUTOSIZE)
cap = highgui.cvCaptureFromFile("/home/asbach/Source/ObjectDetection/older/avi/table.avi")
img = highgui.cvQueryFrame(cap)

print img
print "Got frame of dimensions (", img.width, " x ", img.height, " )"

highgui.cvShowImage("win", img)
highgui.cvMoveWindow("win", 200, 200)
highgui.cvWaitKey(0)

pilimg = opencv.Ipl2PIL(img)
print pilimg

