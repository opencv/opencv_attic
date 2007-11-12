#!/usr/bin/env python

"""A simple TestCase class for testing the adaptors.py module.

2007-11-xx, Vicent Mas <vmas@carabos.com> Carabos Coop. V.
2007-11-08, minor modifications for distribution, Mark Asbach <asbach@ient.rwth-aachen.de>
"""

import unittest
import os

import PIL.Image
import numpy

import python
from python import cv
from python import highgui
from python import adaptors

class AdaptorsTestCase(unittest.TestCase):
    def test00_array_interface(self):
        """Check if PIL supports the array interface."""
        self.assert_(PIL.Image.VERSION>='1.1.6',
            """The installed PIL library doesn't support the array """
            """interface. Please, update to version 1.1.6b2 or higher.""")


    def test01_PIL2NumPy(self):
        """Test the adaptors.PIL2NumPy function."""

        a = adaptors.PIL2NumPy(pil_image)
        self.assert_(a.flags['WRITEABLE'] == True,
            'PIL2NumPy should return a writeable array.')
        b = numpy.asarray(pil_image)
        self.assert_((a == b).all() == True,
            'The returned numpy array has not been properly constructed.')


    def test02_NumPy2PIL(self):
        """Test the adaptors.NumPy2PIL function."""

        a = numpy.asarray(pil_image)
        b = adaptors.NumPy2PIL(a)
        self.assert_(pil_image.tostring() == b.tostring(),
            'The returned image has not been properly constructed.')


    def test03_Ipl2PIL(self):
        """Test the adaptors.Ipl2PIL function."""
    
        i = adaptors.Ipl2PIL(ipl_image)
        self.assert_(pil_image.tostring() == i.tostring(),
            'The returned image has not been properly constructed.')


    def test04_PIL2Ipl(self):
        """Test the adaptors.PIL2Ipl function."""

        i = adaptors.PIL2Ipl(pil_image)
        self.assert_(ipl_image.imageData == i.imageData,
            'The returned image has not been properly constructed.')


    def test05_Ipl2NumPy(self):
        """Test the adaptors.Ipl2NumPy function."""
    
        a = adaptors.Ipl2NumPy(ipl_image)
        a_1d = numpy.reshape(a, (a.size, ))
        # For 3-channel IPL images  the order of channels will be BGR
        # but NumPy array order of channels will be RGB so a conversion
        # is needed before we can compare both images
        if ipl_image.nChannels == 3:
            rgb = cv.cvCreateImage(cv.cvSize(ipl_image.width, ipl_image.height), ipl_image.depth, 3)
            cv.cvCvtColor(ipl_image, rgb, cv.CV_BGR2RGB)
            self.assert_(a_1d.tostring() == rgb.imageData,
                'The returned image has not been properly constructed.')
        else:
            self.assert_(a_1d.tostring() == ipl_image.imageData,
                'The returned image has not been properly constructed.')


    def test06_NumPy2Ipl(self):
        """Test the adaptors.NumPy2Ipl function."""

        a = adaptors.Ipl2NumPy(ipl_image)
        b = adaptors.NumPy2Ipl(a)
        self.assert_(ipl_image.imageData == b.imageData,
            'The returned image has not been properly constructed.')


if __name__ == '__main__':
    global pil_image, ipl_image
    gray_sample = os.environ['top_srcdir']+'/tests/python/testdata/images/cvSetMouseCallback.jpg'
    rgb_sample  = os.environ['top_srcdir']+'/tests/python/testdata/images/baboon.jpg'
    for sample in (gray_sample, rgb_sample):
        print sample
        ipl_image = highgui.cvLoadImage(sample, 4|2)
        pil_image = PIL.Image.open(sample, 'r')
        suite = unittest.TestLoader().loadTestsFromTestCase(AdaptorsTestCase)
        unittest.TextTestRunner().run(suite)

