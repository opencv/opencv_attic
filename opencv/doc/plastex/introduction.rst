Introduction
------------

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

See this tutorial: http://www.ros.org/wiki/opencv_latest/Tutorials/UsingCvBridgeToConvertBetweenROSImagesAndOpenCVImages

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

numpy and OpenCV
^^^^^^^^^^^^^^^^

Numpy and OpenCV have different orderings for dimensions, hence the tuple reversals here::

    def cv2array(im):
      depth2dtype = {
            cv.IPL_DEPTH_8U: 'uint8',
            cv.IPL_DEPTH_8S: 'int8',
            cv.IPL_DEPTH_16U: 'uint16',
            cv.IPL_DEPTH_16S: 'int16',
            cv.IPL_DEPTH_32S: 'int32',
            cv.IPL_DEPTH_32F: 'float32',
            cv.IPL_DEPTH_64F: 'float64',
        }
      
      arrdtype=im.depth
      a = np.fromstring(
             im.tostring(),
             dtype=depth2dtype[im.depth],
             count=im.width*im.height*im.nChannels)
      a.shape = (im.height,im.width,im.nChannels)
      return a
        
    def array2cv(a):
      dtype2depth = {
            'uint8':   cv.IPL_DEPTH_8U,
            'int8':    cv.IPL_DEPTH_8S,
            'uint16':  cv.IPL_DEPTH_16U,
            'int16':   cv.IPL_DEPTH_16S,
            'int32':   cv.IPL_DEPTH_32S,
            'float32': cv.IPL_DEPTH_32F,
            'float64': cv.IPL_DEPTH_64F,
        }
      try:
        nChannels = a.shape[2]
      except:
        nChannels = 1
      cv_im = cv.CreateImageHeader((a.shape[1],a.shape[0]), 
              dtype2depth[str(a.dtype)],
              nChannels)
      cv.SetData(cv_im, a.tostring(), 
                 a.dtype.itemsize*nChannels*a.shape[1])
      return cv_im

