Introduction
------------

Here is a small collection of code samples demonstrating some features
of the OpenCV Python bindings.

Convert an image from png to jpg
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

::

    import cv
    cv.SaveImage("foo.png", cv.LoadImage("foo.jpg"))

Compute the Laplacian
^^^^^^^^^^^^^^^^^^^^^

::

    im = cv.LoadImage("foo.png", 1)
    dst = cv.CreateImage(cv.GetSize(im), cv.IPL_DEPTH_16S, 3);
    laplace = cv.Laplace(im, dst)
    cv.SaveImage("foo-laplace.png", dst)


Using cvGoodFeaturesToTrack
^^^^^^^^^^^^^^^^^^^^^^^^^^^

::

    img = cv.LoadImage("foo.jpg")
    eig_image = cv.CreateImage(cv.GetSize(img), cv.IPL_DEPTH_32F, 1)
    temp_image = cv.CreateImage(cv.GetSize(img), cv.IPL_DEPTH_32F, 1)
    # Find up to 300 corners using Harris
    for (x,y) in cv.GoodFeaturesToTrack(img, eig_image, temp_image, 300, None, 1.0, use_harris = True):
        print "good feature at", x,y

Using GetSubRect
^^^^^^^^^^^^^^^^

GetSubRect returns a rectangular part of another image.  It does this without copying any data.

::

    img = cv.LoadImage("foo.jpg")
    sub = cv.GetSubRect(img, (0, 0, 32, 32))  # sub is 32x32 patch from img top-left
    cv.SetZero(sub)                           # clear sub to zero, which also clears 32x32 pixels in img

Using CreateMat, and accessing an element
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

::

    mat = cv.CreateMat(5, 5, cv.CV_32FC1)
    mat[3,2] += 0.787


ROS image message to OpenCV
^^^^^^^^^^^^^^^^^^^^^^^^^^^

See this tutorial: http://www.ros.org/wiki/cv_bridge/Tutorials/UsingCvBridgeToConvertBetweenROSImagesAndOpenCVImages

PIL Image to OpenCV
^^^^^^^^^^^^^^^^^^^

(For details on PIL see the `PIL manual <http://www.pythonware.com/library/pil/handbook/image.htm>`_).

::

    import Image
    import cv
    pi = Image.open('foo.png')       # PIL image
    cv_im = cv.CreateImageHeader(pi.size, cv.IPL_DEPTH_8U, 1)
    cv.SetData(cv_im, pi.tostring())

OpenCV to PIL Image
^^^^^^^^^^^^^^^^^^^

::

    cv_im = cv.CreateImage((320,200), cv.IPL_DEPTH_8U, 1)
    pi = Image.fromstring("L", cv.GetSize(cv_im), cv_im.tostring())

NumPy and OpenCV
^^^^^^^^^^^^^^^^

Using the array interface, here is how you use an OpenCV Mat in NumPy::

    mat = cv.CreateMat(5, 5, cv.CV_32FC1)
    a = numpy.asarray(mat)

and here is how you use a NumPy array in OpenCV:

    a = numpy.array([1, 2, 3, 4, 5])
    mat = cv.fromarray(a)

These conversions work for CvMat and CvMatND.  N-channel images get
turned into NumPy arrays with N as the smallest dimension.  For example
a 640x480 RGB image would become a NumPy array of shape (640, 480, 3).
When converting in the other direction, from NumPy arrays to OpenCV,
the result is always a single channel CvMat.
