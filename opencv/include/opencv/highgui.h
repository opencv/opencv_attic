/*M///////////////////////////////////////////////////////////////////////////////////////
//
//  IMPORTANT: READ BEFORE DOWNLOADING, COPYING, INSTALLING OR USING.
//
//  By downloading, copying, installing or using the software you agree to this license.
//  If you do not agree to this license, do not download, install,
//  copy or use the software.
//
//
//                        Intel License Agreement
//                For Open Source Computer Vision Library
//
// Copyright (C) 2000, Intel Corporation, all rights reserved.
// Third party copyrights are property of their respective owners.
//
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
//
//   * Redistribution's of source code must retain the above copyright notice,
//     this list of conditions and the following disclaimer.
//
//   * Redistribution's in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//
//   * The name of Intel Corporation may not be used to endorse or promote products
//     derived from this software without specific prior written permission.
//
// This software is provided by the copyright holders and contributors "as is" and
// any express or implied warranties, including, but not limited to, the implied
// warranties of merchantability and fitness for a particular purpose are disclaimed.
// In no event shall the Intel Corporation or contributors be liable for any direct,
// indirect, incidental, special, exemplary, or consequential damages
// (including, but not limited to, procurement of substitute goods or services;
// loss of use, data, or profits; or business interruption) however caused
// and on any theory of liability, whether in contract, strict liability,
// or tort (including negligence or otherwise) arising in any way out of
// the use of this software, even if advised of the possibility of such damage.
//
//M*/

#ifndef _HIGH_GUI_
#define _HIGH_GUI_

#include "cv.h"
#ifdef WIN32
#include <windows.h>
#endif

#if defined(_CH_)
#include <cvch.h>
CV_CH_LOAD_CODE(highgui,Highgui)
#endif

#ifdef __cplusplus
#define HIGHGUI_EXTERN_C extern "C"
extern "C" {
#else
#define HIGHGUI_EXTERN_C
#endif /* __cplusplus */

#if defined HIGHGUI_EXPORTS && defined WIN32
    #define HIGHGUI_DLL_ENTRY __declspec(dllexport)
#else
    #define HIGHGUI_DLL_ENTRY
#endif

#ifdef HIGHGUI_EXPORTS
#define HIGHGUI_API HIGHGUI_EXTERN_C HIGHGUI_DLL_ENTRY
#else
#define HIGHGUI_API
#endif /* HIGHGUI_DLLEXPORT */


/****************************************************************************************\
*                                Basic HighGUI functionality                             *
\****************************************************************************************/

/* this function is used to set some external parameters in case of X Window */
HIGHGUI_API int cvInitSystem( int argc, char** argv );

#define CV_WINDOW_AUTOSIZE  1
/* create window */
HIGHGUI_API int cvNamedWindow( const char* name, int flags );

/* display image within window (highgui windows remember their content) */
HIGHGUI_API void cvShowImage( const char* name, const CvArr* image );

/* resize/move window */
HIGHGUI_API void cvResizeWindow( const char* name, int width, int height );

/* destroy window and all the trackers associated with it */
HIGHGUI_API void cvDestroyWindow( const char* name );

HIGHGUI_API void cvDestroyAllWindows(void);

/* get native window handle (HWND in case of Win32 and Widget in case of X Window) */
HIGHGUI_API void* cvGetWindowHandle( const char* name );

/* get name of highgui window given its native handle */
HIGHGUI_API const char* cvGetWindowName( void* window_handle );

CV_EXTERN_C_FUNCPTR( void (*CvTrackbarCallback)(int pos) );

/* create trackbar and display it on top of given window, set callback */
HIGHGUI_API int cvCreateTrackbar( const char* trackbar_name, const char* window_name,
                                  int* value, int count, CvTrackbarCallback on_change );

/* retrieve or set trackbar position */
HIGHGUI_API int cvGetTrackbarPos( const char* trackbar_name, const char* window_name );
HIGHGUI_API void cvSetTrackbarPos( const char* trackbar_name, const char* window_name, int pos );

#define CV_EVENT_MOUSEMOVE      0
#define CV_EVENT_LBUTTONDOWN    1
#define CV_EVENT_RBUTTONDOWN    2
#define CV_EVENT_MBUTTONDOWN    3
#define CV_EVENT_LBUTTONUP      4
#define CV_EVENT_RBUTTONUP      5
#define CV_EVENT_MBUTTONUP      6
#define CV_EVENT_LBUTTONDBLCLK  7
#define CV_EVENT_RBUTTONDBLCLK  8
#define CV_EVENT_MBUTTONDBLCLK  9

#define CV_EVENT_FLAG_LBUTTON   1
#define CV_EVENT_FLAG_RBUTTON   2
#define CV_EVENT_FLAG_MBUTTON   4
#define CV_EVENT_FLAG_CTRLKEY   8
#define CV_EVENT_FLAG_SHIFTKEY  16
#define CV_EVENT_FLAG_ALTKEY    32

CV_EXTERN_C_FUNCPTR( void (*CvMouseCallback )(int event, int x, int y, int flags) );
/* assign callback for mouse events */
HIGHGUI_API void cvSetMouseCallback( const char* window_name, CvMouseCallback on_mouse );

/*load image from file 
  iscolor: >0 - output image is always color,
            0 - output image is always grayscale,
           <0 - output image is color or grayscale dependending on the file */
HIGHGUI_API IplImage* cvLoadImage( const char* filename, int iscolor CV_DEFAULT(1));

/* save image to file */
HIGHGUI_API int cvSaveImage( const char* filename, const CvArr* image );

/* add folder to the image search path (used by cvLoadImage) */
HIGHGUI_API void cvAddSearchPath( const char* path );

/* utility function: convert one image to another with optional vertical flip */
HIGHGUI_API void cvConvertImage( const CvArr* src, CvArr* dst, int flip CV_DEFAULT(0));

/* wait for key event infinitely (delay<=0) or for "delay" milliseconds */
HIGHGUI_API int cvWaitKey(int delay CV_DEFAULT(0));


/****************************************************************************************\
*                             Working with AVIs and Cameras                              *
\****************************************************************************************/

/* "black box" capture structure */
typedef struct CvCapture CvCapture;

/* start capturing frames from AVI */
HIGHGUI_API CvCapture* cvCaptureFromAVI( const char* filename );

/* start capturing frames from camera */
HIGHGUI_API CvCapture* cvCaptureFromCAM( int index );

/*just grab frame, return 1 if success, 0 if fail 
  this function is thought to be fast               */  
HIGHGUI_API int cvGrabFrame( CvCapture* capture );

/*get frame grabbed with cvGrabFrame(..) 
  This function may apply some frame processing like 
  frame decompression, flipping etc.
  !!!DO NOT RELEASE or MODIFY the retrieved frame!!! */
HIGHGUI_API IplImage* cvRetrieveFrame( CvCapture* capture );

/* Just successive call of cvGrabFrame and cvRetrieveFrame
   !!!DO NOT RELEASE or MODIFY the retrieved frame!!!      */
HIGHGUI_API IplImage* cvQueryFrame( CvCapture* capture );

/* stop capturing/reading and free resources */
HIGHGUI_API void cvReleaseCapture( CvCapture** capture );

#define CV_CAP_PROP_POS_MSEC       0
#define CV_CAP_PROP_POS_FRAMES     1
#define CV_CAP_PROP_POS_AVI_RATIO  2
#define CV_CAP_PROP_FRAME_WIDTH    3
#define CV_CAP_PROP_FRAME_HEIGHT   4
#define CV_CAP_PROP_FPS            5
#define CV_CAP_PROP_FOURCC         6

/* retrieve or set capture properties */
HIGHGUI_API double cvGetCaptureProperty( CvCapture* capture, int property_id );
HIGHGUI_API int    cvSetCaptureProperty( CvCapture* capture, int property_id, double value );

/* "black box" AVI writer structure */
typedef struct CvAVIWriter CvAVIWriter;

#define CV_FOURCC(c1,c2,c3,c4)  \
    (((c1)&255) + (((c2)&255)<<8) + (((c3)&255)<<16) + (((c4)&255)<<24))

/* initialize AVI writer */
HIGHGUI_API CvAVIWriter* cvCreateAVIWriter( const char* filename, int fourcc,
                                            double fps, CvSize frameSize );

/* write frame to AVI */
HIGHGUI_API int cvWriteToAVI( CvAVIWriter* writer, const IplImage* image );

/* close AVI writer */
HIGHGUI_API void cvReleaseAVIWriter( CvAVIWriter** writer );

/****************************************************************************************\
*                              Obsolete functions/synonyms                               *
\****************************************************************************************/

#ifndef HIGHGUI_NO_BACKWARD_COMPATIBILITY
    #define HIGHGUI_BACKWARD_COMPATIBILITY
#endif

#ifdef HIGHGUI_BACKWARD_COMPATIBILITY

#define cvvInitSystem cvInitSystem
#define cvvNamedWindow cvNamedWindow
#define cvvShowImage cvShowImage
#define cvvResizeWindow cvResizeWindow
#define cvvDestroyWindow cvDestroyWindow
#define cvvCreateTrackbar cvCreateTrackbar
#define cvvLoadImage(name) cvLoadImage((name),1)
#define cvvSaveImage cvSaveImage
#define cvvAddSearchPath cvAddSearchPath
#define cvvWaitKey(name) cvWaitKey(0)
#define cvvWaitKeyEx(name,delay) cvWaitKey(delay)
#define cvvConvertImage cvConvertImage
#define HG_AUTOSIZE CV_WINDOW_AUTOSIZE

#ifdef WIN32

CV_EXTERN_C_FUNCPTR( int (CV_CDECL * CvWin32WindowCallback)
                     (HWND, UINT, WPARAM, LPARAM, int*));
HIGHGUI_API void set_preprocess_func( CvWin32WindowCallback on_preprocess );
HIGHGUI_API void set_postprocess_func( CvWin32WindowCallback on_postprocess );

CV_INLINE int iplWidth( const IplImage* img );
CV_INLINE int iplWidth( const IplImage* img )
{
    return !img ? 0 : !img->roi ? img->width : img->roi->width;
}

CV_INLINE int iplHeight( const IplImage* img );
CV_INLINE int iplHeight( const IplImage* img )
{
    return !img ? 0 : !img->roi ? img->height : img->roi->height;
}

#endif

#endif /* obsolete functions */

/* For use with Win32 */
#ifdef WIN32

CV_INLINE RECT NormalizeRect( RECT r );
CV_INLINE RECT NormalizeRect( RECT r )
{
    int t;

    if( r.left > r.right )
    {
        t = r.left;
        r.left = r.right;
        r.right = t;
    }

    if( r.top > r.bottom )
    {
        t = r.top;
        r.top = r.bottom;
        r.bottom = t;
    }

    return r;
}

CV_INLINE CvRect RectToCvRect( RECT sr );
CV_INLINE CvRect RectToCvRect( RECT sr )
{
    sr = NormalizeRect( sr );
    return cvRect( sr.left, sr.top, sr.right - sr.left, sr.bottom - sr.top );
}

CV_INLINE RECT CvRectToRect( CvRect sr );
CV_INLINE RECT CvRectToRect( CvRect sr )
{
    RECT dr;
    dr.left = sr.x;
    dr.top = sr.y;
    dr.right = sr.x + sr.width;
    dr.bottom = sr.y + sr.height;

    return dr;
}

CV_INLINE IplROI RectToROI( RECT r );
CV_INLINE IplROI RectToROI( RECT r )
{
    IplROI roi;
    r = NormalizeRect( r );
    roi.xOffset = r.left;
    roi.yOffset = r.top;
    roi.width = r.right - r.left;
    roi.height = r.bottom - r.top;
    roi.coi = 0;

    return roi;
}

#endif /* WIN32 */

#ifdef __cplusplus
}  /* end of extern "C" */
#endif /* __cplusplus */


#ifdef __cplusplus

#define CImage CvvImage

/* CvvImage class definition */
class CvvImage
{
public:
    HIGHGUI_DLL_ENTRY CvvImage();
    HIGHGUI_DLL_ENTRY virtual ~CvvImage();

    /* Create image (BGR or grayscale) */
    HIGHGUI_DLL_ENTRY virtual bool  Create( int w, int h, int bpp, int origin = 0 );

    /* Load image from specified file */
    HIGHGUI_DLL_ENTRY virtual bool  Load( const char* filename, int desired_color = 1 );

    /* Load rectangle from the file */
    HIGHGUI_DLL_ENTRY virtual bool  LoadRect( const char* filename,
                                              int desired_color, CvRect r );

#ifdef WIN32
    HIGHGUI_DLL_ENTRY virtual bool  LoadRect( const char* filename,
                                              int desired_color, RECT r )
    {
        return LoadRect( filename, desired_color,
                         cvRect( r.left, r.top, r.right - r.left, r.bottom - r.top ));
    }
#endif

    /* Save entire image to specified file. */
    HIGHGUI_DLL_ENTRY virtual bool  Save( const char* filename );

    /* Get copy of input image ROI */
    HIGHGUI_DLL_ENTRY virtual void  CopyOf( CvvImage& image, int desired_color = -1 );
    HIGHGUI_DLL_ENTRY virtual void  CopyOf( IplImage* img, int desired_color = -1 );

    IplImage* GetImage() { return m_img; };
    HIGHGUI_DLL_ENTRY virtual void  Destroy(void);

    /* width and height of ROI */
    int Width() { return !m_img ? 0 : !m_img->roi ? m_img->width : m_img->roi->width; };
    int Height() { return !m_img ? 0 : !m_img->roi ? m_img->height : m_img->roi->height;};
    int Bpp() { return m_img ? (m_img->depth & 255)*m_img->nChannels : 0; };

    HIGHGUI_DLL_ENTRY virtual void  Fill( int color );

    /* draw to highgui window */
    HIGHGUI_DLL_ENTRY virtual void  Show( const char* window );

#ifdef WIN32
    /* draw part of image to the specified DC */
    HIGHGUI_DLL_ENTRY virtual void  Show( HDC dc, int x, int y, int w, int h,
                                          int from_x = 0, int from_y = 0 );
    /* draw the current image ROI to the specified rectangle of the destination DC */
    HIGHGUI_DLL_ENTRY virtual void  DrawToHDC( HDC hDCDst, RECT* pDstRect );
#endif

protected:

    IplImage*  m_img;
};

#endif /* __cplusplus */

#endif /* _HIGH_GUI_ */
