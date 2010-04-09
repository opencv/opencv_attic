#!/usr/bin/python
import urllib2
import sys
import cv
import numpy
import time

# SRGB-linear conversions using NumPy - see http://en.wikipedia.org/wiki/SRGB

def srgb2lin(x):
    a = 0.055
    return numpy.where(x <= 0.04045,
                       x * (1.0 / 12.92),
                       numpy.power((x + a) * (1.0 / (1 + a)), 2.4))

def lin2srgb(x):
    a = 0.055
    return numpy.where(x <= 0.0031308,
                       x * 12.92,
                       (1 + a) * numpy.power(x, 1 / 2.4) - a)

if __name__ == "__main__":
    cv.NamedWindow("camera", 1)

    capture = cv.CaptureFromCAM(0)

    paste = cv.CreateMat(960, 1280, cv.CV_8UC3)
    topleft = numpy.asarray(cv.GetSubRect(paste, (0, 0, 640, 480)))
    topright = numpy.asarray(cv.GetSubRect(paste, (640, 0, 640, 480)))
    bottomleft = numpy.asarray(cv.GetSubRect(paste, (0, 480, 640, 480)))
    bottomright = numpy.asarray(cv.GetSubRect(paste, (640, 480, 640, 480)))

    started = time.time()
    for i in range(100):
        img = cv.GetMat(cv.QueryFrame(capture))

        n = (numpy.asarray(img)).astype(numpy.uint8)

        red = n[:,:,0]
        grn = n[:,:,1]
        blu = n[:,:,2]

        topleft[:,:,0] = 255 - grn
        topleft[:,:,1] = red
        topleft[:,:,2] = blu

        topright[:,:,0] = blu
        topright[:,:,1] = 255 - red
        topright[:,:,2] = grn

        bottomleft[:,:,0] = red
        bottomleft[:,:,1] = grn
        bottomleft[:,:,2] = 255 - blu

        fgrn = grn.astype(numpy.float32)
        fred = red.astype(numpy.float32)
        bottomright[:,:,0] = blu
        bottomright[:,:,1] = (abs(fgrn - fred)).astype(numpy.uint8)
        bottomright[:,:,2] = red

        cv.ShowImage("camera", paste)
        if cv.WaitKey(6) == 27:
            break
    print "took", time.time() - started
