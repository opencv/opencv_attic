#!/usr/bin/python 

import urllib2
import cv
from random import randint

def minarea_array(img, count):
    pointMat = cv.CreateMat(count, 1, cv.CV_32SC2)
    for i in range(count):
        pointMat[i] = (randint(img.width/4, img.width*3/4),
                               randint(img.height/4, img.height*3/4))

    box = cv.MinAreaRect2(pointMat)
    box_vtx = cv.BoxPoints(box)
    success, center, radius = cv.MinEnclosingCircle(pointMat)
    cv.Zero(img)
    for i in range(count):
        cv.Circle(img, cv.Get1D(pointMat, i), 2, cv.CV_RGB(255, 0, 0), cv. CV_FILLED, cv. CV_AA, 0)

    box_vtx = [From32f(box_vtx[0]),
               From32f(box_vtx[1]),
               From32f(box_vtx[2]),
               From32f(box_vtx[3])]
    cv.Circle(img, From32f(center), cv.Round(radius), cv.CV_RGB(255, 255, 0), 1, cv. CV_AA, 0)
    cv.PolyLine(img, [box_vtx], 1, cv.CV_RGB(0, 255, 255), 1, cv. CV_AA) 
    

    
def minarea_seq(img, count, storage):
    ptseq = cv.CreateSeq(cv.CV_SEQ_KIND_GENERIC | cv. CV_32SC2, sizeof_CvContour, sizeof_CvPoint, storage)
    ptseq = CvSeq_CvPoint.cast(ptseq)
    for i in range(count):
        pt0 = (randint(img.width/4, img.width*3/4),
                       randint(img.height/4, img.height*3/4))
        cv.SeqPush(ptseq, pt0)
    box = cv.MinAreaRect2(ptseq)
    box_vtx = cv.BoxPoints(box)
    success, center, radius = cv.MinEnclosingCircle(ptseq)
    cv.Zero(img)
    for pt in ptseq: 
        cv.Circle(img, pt, 2, cv.CV_RGB(255, 0, 0), cv. CV_FILLED, cv. CV_AA, 0)

    box_vtx = [From32f(box_vtx[0]),
               From32f(box_vtx[1]),
               From32f(box_vtx[2]),
               From32f(box_vtx[3])]
    cv.Circle(img, From32f(center), cv.Round(radius), cv.CV_RGB(255, 255, 0), 1, cv. CV_AA, 0)
    cv.PolyLine(img, [box_vtx], 1, cv.CV_RGB(0, 255, 255), 1, cv. CV_AA) 
    cv.ClearMemStorage(storage)

if __name__ == "__main__":
    img = cv.CreateImage(cv.Size(500, 500), 8, 3)
    storage = cv.CreateMemStorage(0)

    cv.NamedWindow("rect & circle", 1)
        
    use_seq=True

    while True: 
        count = randint(1, 100)
        if use_seq:
            minarea_seq(img, count, storage)
        else:
            minarea_array(img, count)

        cv.ShowImage("rect & circle", img)
        key = cv.WaitKey()
        if(key == '\x1b'):
            break

        use_seq = not use_seq
