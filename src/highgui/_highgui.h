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

#ifndef __HIGHGUI_H_
#define __HIGHGUI_H_

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <ctype.h>
#include <assert.h>

#include "highgui.h"

#ifdef WIN32

#define WIN32_LEAN_AND_MEAN  // Exclude rarely-used stuff from Windows headers

#include "windows.h"
#include "commctrl.h"

typedef HWND CvvWidget;
typedef WNDPROC CvvWidgetProc;
typedef void* CvvCanvas;

#else

#include <X11/Xlib.h>
#include <X11/Intrinsic.h>

typedef Widget CvvWidget;
typedef int (*CvvWidgetProc)( void* );
typedef XImage* CvvCanvas;

#endif

#define HIGHGUI_IMPL extern "C"

#ifdef WIN32

struct CvvControl
{
    CvvWidget m_control;
    void* m_data;
    void (*m_notify)(int);
    int m_id;
    char* m_name;
    CvvControl* m_next;
};

struct CvvToolbar
{
    CvvWidget m_toolbar;
    int m_pos;
    int m_rows;
    CvvWidgetProc m_toolBarProc;
    CvvControl* m_first;
};

struct CvvWindow
{
    CvvWidget m_window;
    CvvWidget m_main;
    CvvCanvas m_image;
    char* m_name;
    HDC m_dc;
    int m_last_key;
    CvvToolbar m_toolbar;
    unsigned long m_flags;
    CvvWindow* m_prev;
    CvvWindow* m_next;
};

void  FillBitmapInfo( BITMAPINFO* bmi, int width, int height, int bpp, int origin );

#else

typedef struct _CvvTrackbar
{
    char    name[100];
    int    count;
    int*   value;
    void   (*on_notify)(int);
    struct _CvvTrackbar* next;
} CvvTrackbar;

struct CvvWindow
{
    char    name[100];
    int     created;
    int     flags;

    CvvWidget  window;
    CvvWidget  paned;
    CvvWidget  frame;
    CvvCanvas  image;
    CvvCanvas  dst_image;
    int        converted;

    struct  CvvWindow* next;
    struct  CvvWindow* prev;
    CvvTrackbar*  trackbar;
};

#endif /* WIN32 */

#endif /* __HIGHGUI_H_ */

