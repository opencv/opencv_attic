# This file was created automatically by SWIG.
# Don't modify this file, modify the SWIG interface instead.
# This file is compatible with both classic and new-style classes.

import _highgui

def _swig_setattr_nondynamic(self,class_type,name,value,static=1):
    if (name == "this"):
        if isinstance(value, class_type):
            self.__dict__[name] = value.this
            if hasattr(value,"thisown"): self.__dict__["thisown"] = value.thisown
            del value.thisown
            return
    method = class_type.__swig_setmethods__.get(name,None)
    if method: return method(self,value)
    if (not static) or hasattr(self,name) or (name == "thisown"):
        self.__dict__[name] = value
    else:
        raise AttributeError("You cannot add attributes to %s" % self)

def _swig_setattr(self,class_type,name,value):
    return _swig_setattr_nondynamic(self,class_type,name,value,0)

def _swig_getattr(self,class_type,name):
    method = class_type.__swig_getmethods__.get(name,None)
    if method: return method(self)
    raise AttributeError,name

import types
try:
    _object = types.ObjectType
    _newclass = 1
except AttributeError:
    class _object : pass
    _newclass = 0
del types



cvInitSystem = _highgui.cvInitSystem

cvStartWindowThread = _highgui.cvStartWindowThread
CV_WINDOW_AUTOSIZE = _highgui.CV_WINDOW_AUTOSIZE

cvNamedWindow = _highgui.cvNamedWindow

cvShowImage = _highgui.cvShowImage

cvResizeWindow = _highgui.cvResizeWindow

cvMoveWindow = _highgui.cvMoveWindow

cvDestroyWindow = _highgui.cvDestroyWindow

cvDestroyAllWindows = _highgui.cvDestroyAllWindows

cvGetWindowHandle = _highgui.cvGetWindowHandle

cvGetWindowName = _highgui.cvGetWindowName

cvCreateTrackbar = _highgui.cvCreateTrackbar

cvGetTrackbarPos = _highgui.cvGetTrackbarPos

cvSetTrackbarPos = _highgui.cvSetTrackbarPos
CV_EVENT_MOUSEMOVE = _highgui.CV_EVENT_MOUSEMOVE
CV_EVENT_LBUTTONDOWN = _highgui.CV_EVENT_LBUTTONDOWN
CV_EVENT_RBUTTONDOWN = _highgui.CV_EVENT_RBUTTONDOWN
CV_EVENT_MBUTTONDOWN = _highgui.CV_EVENT_MBUTTONDOWN
CV_EVENT_LBUTTONUP = _highgui.CV_EVENT_LBUTTONUP
CV_EVENT_RBUTTONUP = _highgui.CV_EVENT_RBUTTONUP
CV_EVENT_MBUTTONUP = _highgui.CV_EVENT_MBUTTONUP
CV_EVENT_LBUTTONDBLCLK = _highgui.CV_EVENT_LBUTTONDBLCLK
CV_EVENT_RBUTTONDBLCLK = _highgui.CV_EVENT_RBUTTONDBLCLK
CV_EVENT_MBUTTONDBLCLK = _highgui.CV_EVENT_MBUTTONDBLCLK
CV_EVENT_FLAG_LBUTTON = _highgui.CV_EVENT_FLAG_LBUTTON
CV_EVENT_FLAG_RBUTTON = _highgui.CV_EVENT_FLAG_RBUTTON
CV_EVENT_FLAG_MBUTTON = _highgui.CV_EVENT_FLAG_MBUTTON
CV_EVENT_FLAG_CTRLKEY = _highgui.CV_EVENT_FLAG_CTRLKEY
CV_EVENT_FLAG_SHIFTKEY = _highgui.CV_EVENT_FLAG_SHIFTKEY
CV_EVENT_FLAG_ALTKEY = _highgui.CV_EVENT_FLAG_ALTKEY

cvSetMouseCallback = _highgui.cvSetMouseCallback

cvLoadImage = _highgui.cvLoadImage

cvSaveImage = _highgui.cvSaveImage
CV_CVTIMG_FLIP = _highgui.CV_CVTIMG_FLIP
CV_CVTIMG_SWAP_RB = _highgui.CV_CVTIMG_SWAP_RB

cvConvertImage = _highgui.cvConvertImage

cvWaitKey = _highgui.cvWaitKey

cvCaptureFromFile = _highgui.cvCaptureFromFile
CV_CAP_ANY = _highgui.CV_CAP_ANY
CV_CAP_MIL = _highgui.CV_CAP_MIL
CV_CAP_VFW = _highgui.CV_CAP_VFW
CV_CAP_V4L = _highgui.CV_CAP_V4L
CV_CAP_V4L2 = _highgui.CV_CAP_V4L2
CV_CAP_FIREWARE = _highgui.CV_CAP_FIREWARE
CV_CAP_IEEE1394 = _highgui.CV_CAP_IEEE1394
CV_CAP_DC1394 = _highgui.CV_CAP_DC1394
CV_CAP_CMU1394 = _highgui.CV_CAP_CMU1394
CV_CAP_STEREO = _highgui.CV_CAP_STEREO

cvCaptureFromCAM = _highgui.cvCaptureFromCAM

cvGrabFrame = _highgui.cvGrabFrame

cvRetrieveFrame = _highgui.cvRetrieveFrame

cvQueryFrame = _highgui.cvQueryFrame

cvReleaseCapture = _highgui.cvReleaseCapture
CV_CAP_PROP_POS_MSEC = _highgui.CV_CAP_PROP_POS_MSEC
CV_CAP_PROP_POS_FRAMES = _highgui.CV_CAP_PROP_POS_FRAMES
CV_CAP_PROP_POS_AVI_RATIO = _highgui.CV_CAP_PROP_POS_AVI_RATIO
CV_CAP_PROP_FRAME_WIDTH = _highgui.CV_CAP_PROP_FRAME_WIDTH
CV_CAP_PROP_FRAME_HEIGHT = _highgui.CV_CAP_PROP_FRAME_HEIGHT
CV_CAP_PROP_FPS = _highgui.CV_CAP_PROP_FPS
CV_CAP_PROP_FOURCC = _highgui.CV_CAP_PROP_FOURCC
CV_CAP_PROP_FRAME_COUNT = _highgui.CV_CAP_PROP_FRAME_COUNT
CV_CAP_PROP_FORMAT = _highgui.CV_CAP_PROP_FORMAT
CV_CAP_PROP_MODE = _highgui.CV_CAP_PROP_MODE
CV_CAP_PROP_BRIGHTNESS = _highgui.CV_CAP_PROP_BRIGHTNESS
CV_CAP_PROP_CONTRAST = _highgui.CV_CAP_PROP_CONTRAST
CV_CAP_PROP_SATURATION = _highgui.CV_CAP_PROP_SATURATION
CV_CAP_PROP_HUE = _highgui.CV_CAP_PROP_HUE
CV_CAP_PROP_GAIN = _highgui.CV_CAP_PROP_GAIN
CV_CAP_PROP_CONVERT_RGB = _highgui.CV_CAP_PROP_CONVERT_RGB

cvGetCaptureProperty = _highgui.cvGetCaptureProperty

cvSetCaptureProperty = _highgui.cvSetCaptureProperty

cvCreateVideoWriter = _highgui.cvCreateVideoWriter

cvWriteFrame = _highgui.cvWriteFrame

cvReleaseVideoWriter = _highgui.cvReleaseVideoWriter
HG_AUTOSIZE = _highgui.HG_AUTOSIZE
class CvvImage(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, CvvImage, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, CvvImage, name)
    def __repr__(self):
        return "<%s.%s; proxy of C++ CvvImage instance at %s>" % (self.__class__.__module__, self.__class__.__name__, self.this,)
    def __init__(self, *args):
        _swig_setattr(self, CvvImage, 'this', _highgui.new_CvvImage(*args))
        _swig_setattr(self, CvvImage, 'thisown', 1)
    def __del__(self, destroy=_highgui.delete_CvvImage):
        try:
            if self.thisown: destroy(self)
        except: pass

    def Create(*args): return _highgui.CvvImage_Create(*args)
    def Load(*args): return _highgui.CvvImage_Load(*args)
    def LoadRect(*args): return _highgui.CvvImage_LoadRect(*args)
    def Save(*args): return _highgui.CvvImage_Save(*args)
    def CopyOf(*args): return _highgui.CvvImage_CopyOf(*args)
    def GetImage(*args): return _highgui.CvvImage_GetImage(*args)
    def Destroy(*args): return _highgui.CvvImage_Destroy(*args)
    def Width(*args): return _highgui.CvvImage_Width(*args)
    def Height(*args): return _highgui.CvvImage_Height(*args)
    def Bpp(*args): return _highgui.CvvImage_Bpp(*args)
    def Fill(*args): return _highgui.CvvImage_Fill(*args)
    def Show(*args): return _highgui.CvvImage_Show(*args)

class CvvImagePtr(CvvImage):
    def __init__(self, this):
        _swig_setattr(self, CvvImage, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, CvvImage, 'thisown', 0)
        _swig_setattr(self, CvvImage,self.__class__,CvvImage)
_highgui.CvvImage_swigregister(CvvImagePtr)

__doc__ = """HighGUI provides minimalistic user interface parts and video input/output.

Dependent on the platform it was compiled on, this library provides methods
to draw a window for image display, capture video from a camera or framegrabber
or read/write video streams from/to the file system.

This wrapper was semi-automatically created from the C/C++ headers and therefore
contains no Python documentation. Because all identifiers are identical to their
C/C++ counterparts, you can consult the standard manuals that come with OpenCV.
"""



