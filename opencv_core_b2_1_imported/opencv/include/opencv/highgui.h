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

#define CVVAPI HIGHGUI_API

typedef IplImage* IPLIMAGE;

/* Errors */
#define CVV_OK          0 /* Don't bet on it! */
#define CVV_BADNAME    -1 /* Bad window or file name */
#define CVV_INITFAILED -2 /* Can't initialize HigCVVUI. Possibly, can't find vlgrfmts.dll */
#define CVV_WCFAILED   -3 /* Can't create a window */
#define CVV_NULLPTR    -4 /* The null pointer where it should not appear */
#define CVV_BADPARAM   -5 

#define HG_OK         CVV_OK
#define HG_BADNAME    CVV_BADNAME
#define HG_INITFAILED CVV_INITFAILED
#define HG_WCFAILED   CVV_WCFAILED
#define HG_NULLPTR    CVV_NULLPTR
#define HG_BADPARAM   CVV_BADPARAM

/* this function is used to set graphical library some extern parameters */
HIGHGUI_API int cvvInitSystem( int argc, char** argv );

HIGHGUI_API int cvvNamedWindow( const char* name, unsigned long flags );

HIGHGUI_API void cvvShowImage( const char* name, const CvArr* image );

HIGHGUI_API int cvvResizeWindow( const char* name, int width, int height );

HIGHGUI_API int cvvDestroyWindow( const char* name );

CV_EXTERN_C_FUNCPTR( void (*HG_on_notify)(int) );

HIGHGUI_API int cvvCreateTrackbar( const char* name, const char* window_name,
                                   int* value, int count, HG_on_notify on_notify );

HIGHGUI_API IplImage* cvvLoadImage( const char* filename );

HIGHGUI_API int cvvSaveImage( const char* filename, const CvArr* image );

HIGHGUI_API void cvvAddSearchPath( const char* path );

HIGHGUI_API int cvvWaitKey( const char* name );

HIGHGUI_API int cvvWaitKeyEx( const char* name, int delay );

HIGHGUI_API void cvvConvertImage( const CvArr* src, CvArr* dst, int flip CV_DEFAULT(0));


#ifndef HIGHGUI_NO_BACKWARD_COMPATIBILITY
    #define HIGHGUI_BACKWARD_COMPATIBILITY
#endif

#ifdef HIGHGUI_BACKWARD_COMPATIBILITY

#define HGInitialize() cvvInitSystem( 0, 0 )
#define HGExit()
#define add_search_path cvvAddSearchPath
#define named_window cvvNamedWindow
#define show_iplimage cvvShowImage
#define resize cvvResizeWindow
#define destroy_window cvvDestroyWindow
#define create_trackbar cvvCreateTrackbar
#define create_slider create_trackbar
#define load_iplimage cvvLoadImage
#define save_iplimage cvvSaveImage
#define wait_key cvvWaitKey
#define wait_key_ex cvvWaitKeyEx
#define CImage CvvImage /* see below */

#endif

/* For use with Win32 */

#ifdef WIN32
HIGHGUI_API void SetInstance(void* curInstance);
HIGHGUI_API int  check_key(const char* window_name);
HIGHGUI_API int  reset_key(const char* window_name);
HIGHGUI_API int  create_toolbar(const char* window_name/*, int pos*/);
HIGHGUI_API void destroy_all();
HIGHGUI_API void detach_all_controls();

HIGHGUI_API HWND get_hwnd_byname(const char* name);
HIGHGUI_API const char* get_name_byhwnd(HWND hwnd);
HIGHGUI_API void set_preprocess_func(int (__cdecl * on_preprocess)(HWND, UINT, WPARAM, LPARAM, int*));
HIGHGUI_API void set_postprocess_func(int (__cdecl *on_postprocess)(HWND, UINT, WPARAM, LPARAM, int*));

__inline int iplWidth( const IplImage* img )
{
    return !img ? 0 : !img->roi ? img->width : img->roi->width; 
}

__inline int iplHeight( const IplImage* img )
{
    return !img ? 0 : !img->roi ? img->height : img->roi->height; 
}

__inline RECT NormalizeRect( RECT r )
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

__inline CvRect RectToCvRect( RECT sr )
{
    sr = NormalizeRect( sr );
    return cvRect( sr.left, sr.top, sr.right - sr.left, sr.bottom - sr.top );
}

__inline RECT CvRectToRect( CvRect sr )
{
    RECT dr;
    dr.left = sr.x;
    dr.top = sr.y;
    dr.right = sr.x + sr.width;
    dr.bottom = sr.y + sr.height;

    return dr;
}

__inline IplROI RectToROI( RECT r )
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

/* Window flags */
#define HG_AUTOSIZE 1

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
