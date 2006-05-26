# This file was created automatically by SWIG 1.3.27.
# Don't modify this file, modify the SWIG interface instead.

import _highgui

# This file is compatible with both classic and new-style classes.
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



def cvInitSystem(*args):
    """cvInitSystem(int argc, char argv) -> int"""
    return _highgui.cvInitSystem(*args)

def cvStartWindowThread(*args):
    """cvStartWindowThread() -> int"""
    return _highgui.cvStartWindowThread(*args)
CV_WINDOW_AUTOSIZE = _highgui.CV_WINDOW_AUTOSIZE

def cvNamedWindow(*args):
    """cvNamedWindow(char name, int flags) -> int"""
    return _highgui.cvNamedWindow(*args)

def cvShowImage(*args):
    """cvShowImage(char name, CvArr image)"""
    return _highgui.cvShowImage(*args)

def cvResizeWindow(*args):
    """cvResizeWindow(char name, int width, int height)"""
    return _highgui.cvResizeWindow(*args)

def cvMoveWindow(*args):
    """cvMoveWindow(char name, int x, int y)"""
    return _highgui.cvMoveWindow(*args)

def cvDestroyWindow(*args):
    """cvDestroyWindow(char name)"""
    return _highgui.cvDestroyWindow(*args)

def cvDestroyAllWindows(*args):
    """cvDestroyAllWindows()"""
    return _highgui.cvDestroyAllWindows(*args)

def cvGetWindowHandle(*args):
    """cvGetWindowHandle(char name) -> void"""
    return _highgui.cvGetWindowHandle(*args)

def cvGetWindowName(*args):
    """cvGetWindowName(void window_handle) -> char"""
    return _highgui.cvGetWindowName(*args)

def cvCreateTrackbar(*args):
    """
    cvCreateTrackbar(char trackbar_name, char window_name, int value, int count, 
        CvTrackbarCallback on_change) -> int
    """
    return _highgui.cvCreateTrackbar(*args)

def cvGetTrackbarPos(*args):
    """cvGetTrackbarPos(char trackbar_name, char window_name) -> int"""
    return _highgui.cvGetTrackbarPos(*args)

def cvSetTrackbarPos(*args):
    """cvSetTrackbarPos(char trackbar_name, char window_name, int pos)"""
    return _highgui.cvSetTrackbarPos(*args)
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

def cvSetMouseCallback(*args):
    """cvSetMouseCallback(char window_name, CvMouseCallback on_mouse, void param=None)"""
    return _highgui.cvSetMouseCallback(*args)
CV_LOAD_IMAGE_COLOR = _highgui.CV_LOAD_IMAGE_COLOR
CV_LOAD_IMAGE_GRAYSCALE = _highgui.CV_LOAD_IMAGE_GRAYSCALE
CV_LOAD_IMAGE_UNCHANGED = _highgui.CV_LOAD_IMAGE_UNCHANGED

def cvLoadImage(*args):
    """cvLoadImage(char filename, int iscolor=1)"""
    return _highgui.cvLoadImage(*args)

def cvLoadImageM(*args):
    """cvLoadImageM(char filename, int iscolor=1) -> CvMat"""
    return _highgui.cvLoadImageM(*args)

def cvSaveImage(*args):
    """cvSaveImage(char filename, CvArr image) -> int"""
    return _highgui.cvSaveImage(*args)
CV_CVTIMG_FLIP = _highgui.CV_CVTIMG_FLIP
CV_CVTIMG_SWAP_RB = _highgui.CV_CVTIMG_SWAP_RB

def cvConvertImage(*args):
    """cvConvertImage(CvArr src, CvArr dst, int flags=0)"""
    return _highgui.cvConvertImage(*args)

def cvWaitKey(*args):
    """cvWaitKey(int delay=0) -> int"""
    return _highgui.cvWaitKey(*args)

def cvCaptureFromFile(*args):
    """cvCaptureFromFile(char filename) -> CvCapture"""
    return _highgui.cvCaptureFromFile(*args)
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
CV_CAP_TYZX = _highgui.CV_CAP_TYZX
CV_TYZX_LEFT = _highgui.CV_TYZX_LEFT
CV_TYZX_RIGHT = _highgui.CV_TYZX_RIGHT
CV_TYZX_COLOR = _highgui.CV_TYZX_COLOR
CV_TYZX_Z = _highgui.CV_TYZX_Z
CV_CAP_QT = _highgui.CV_CAP_QT

def cvCaptureFromCAM(*args):
    """cvCaptureFromCAM(int index) -> CvCapture"""
    return _highgui.cvCaptureFromCAM(*args)

def cvGrabFrame(*args):
    """cvGrabFrame(CvCapture capture) -> int"""
    return _highgui.cvGrabFrame(*args)

def cvRetrieveFrame(*args):
    """cvRetrieveFrame(CvCapture capture)"""
    return _highgui.cvRetrieveFrame(*args)

def cvQueryFrame(*args):
    """cvQueryFrame(CvCapture capture)"""
    return _highgui.cvQueryFrame(*args)

def cvReleaseCapture(*args):
    """cvReleaseCapture(CvCapture capture)"""
    return _highgui.cvReleaseCapture(*args)
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

def cvGetCaptureProperty(*args):
    """cvGetCaptureProperty(CvCapture capture, int property_id) -> double"""
    return _highgui.cvGetCaptureProperty(*args)

def cvSetCaptureProperty(*args):
    """cvSetCaptureProperty(CvCapture capture, int property_id, double value) -> int"""
    return _highgui.cvSetCaptureProperty(*args)

def cvCreateVideoWriter(*args):
    """
    cvCreateVideoWriter(char filename, int fourcc, double fps, CvSize frame_size, 
        int is_color=1) -> CvVideoWriter
    """
    return _highgui.cvCreateVideoWriter(*args)

def cvWriteFrame(*args):
    """cvWriteFrame(CvVideoWriter writer,  image) -> int"""
    return _highgui.cvWriteFrame(*args)

def cvReleaseVideoWriter(*args):
    """cvReleaseVideoWriter(CvVideoWriter writer)"""
    return _highgui.cvReleaseVideoWriter(*args)
HG_AUTOSIZE = _highgui.HG_AUTOSIZE
class CvvImage(_object):
    """Proxy of C++ CvvImage class"""
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, CvvImage, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, CvvImage, name)
    def __repr__(self):
        return "<%s.%s; proxy of C++ CvvImage instance at %s>" % (self.__class__.__module__, self.__class__.__name__, self.this,)
    def __init__(self, *args):
        """__init__(self) -> CvvImage"""
        _swig_setattr(self, CvvImage, 'this', _highgui.new_CvvImage(*args))
        _swig_setattr(self, CvvImage, 'thisown', 1)
    def __del__(self, destroy=_highgui.delete_CvvImage):
        """__del__(self)"""
        try:
            if self.thisown: destroy(self)
        except: pass

    def Create(*args):
        """
        Create(self, int width, int height, int bits_per_pixel, int image_origin=0) -> bool
        Create(self, int width, int height, int bits_per_pixel) -> bool
        """
        return _highgui.CvvImage_Create(*args)

    def Load(*args):
        """
        Load(self, char filename, int desired_color=1) -> bool
        Load(self, char filename) -> bool
        """
        return _highgui.CvvImage_Load(*args)

    def LoadRect(*args):
        """LoadRect(self, char filename, int desired_color, CvRect r) -> bool"""
        return _highgui.CvvImage_LoadRect(*args)

    def Save(*args):
        """Save(self, char filename) -> bool"""
        return _highgui.CvvImage_Save(*args)

    def CopyOf(*args):
        """
        CopyOf(self, CvvImage image, int desired_color=-1)
        CopyOf(self, CvvImage image)
        CopyOf(self,  img, int desired_color=-1)
        CopyOf(self,  img)
        """
        return _highgui.CvvImage_CopyOf(*args)

    def GetImage(*args):
        """GetImage(self)"""
        return _highgui.CvvImage_GetImage(*args)

    def Destroy(*args):
        """Destroy(self)"""
        return _highgui.CvvImage_Destroy(*args)

    def Width(*args):
        """Width(self) -> int"""
        return _highgui.CvvImage_Width(*args)

    def Height(*args):
        """Height(self) -> int"""
        return _highgui.CvvImage_Height(*args)

    def Bpp(*args):
        """Bpp(self) -> int"""
        return _highgui.CvvImage_Bpp(*args)

    def Fill(*args):
        """Fill(self, int color)"""
        return _highgui.CvvImage_Fill(*args)

    def Show(*args):
        """Show(self, char window)"""
        return _highgui.CvvImage_Show(*args)


class CvvImagePtr(CvvImage):
    def __init__(self, this):
        _swig_setattr(self, CvvImage, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, CvvImage, 'thisown', 0)
        self.__class__ = CvvImage
_highgui.CvvImage_swigregister(CvvImagePtr)

__doc__ = """HighGUI provides minimalistic user interface parts and video input/output.

Dependent on the platform it was compiled on, this library provides methods
to draw a window for image display, capture video from a camera or framegrabber
or read/write video streams from/to the file system.

This wrapper was semi-automatically created from the C/C++ headers and therefore
contains no Python documentation. Because all identifiers are identical to their
C/C++ counterparts, you can consult the standard manuals that come with OpenCV.
"""




