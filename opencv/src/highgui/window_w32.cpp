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

#include "_highgui.h"

#ifdef WIN32

#include "HighGUI.h"
#include "GrFmts.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>


#ifndef TBIF_SIZE
    #define TBIF_SIZE  0x40
#endif

#ifndef TB_SETBUTTONINFO
    #define TB_SETBUTTONINFO (WM_USER + 66)
#endif

#ifndef TBM_GETTOOLTIPS
    #define TBM_GETTOOLTIPS  (WM_USER + 30)
#endif


static LRESULT CALLBACK HighGUIProc(  HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
static LRESULT CALLBACK WindowProc(  HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

typedef CvvWindow _WINDOW_INFO;
typedef CvvToolbar _TOOLBAR_INFO;
typedef CvvControl _CONTROL_INFO;

static int storage_count = 0;

static _WINDOW_INFO* FIRST = 0;
static _WINDOW_INFO* LAST = 0;

static int (__cdecl *hg_on_preprocess)(HWND, UINT, WPARAM, LPARAM, int*) = 0;
static int (__cdecl *hg_on_postprocess)(HWND, UINT, WPARAM, LPARAM, int*) = 0;

static _WINDOW_INFO* _add_element()
{
    _WINDOW_INFO* pn = (_WINDOW_INFO*)malloc(sizeof(_WINDOW_INFO));
    if(storage_count++ == 0)
    {
        FIRST = LAST = pn;
        pn->m_prev = 0;
    }
    else
    {
        LAST->m_next = pn;
        pn->m_prev = LAST;
        LAST = pn;
    }

    pn->m_next = 0;
    return pn;
}

static void _remove_element(_WINDOW_INFO* el)
{
    assert(storage_count);
    assert(el);
    if(el->m_next)
    {
        el->m_next->m_prev = el->m_prev;
    }
    else
    {
        LAST = el->m_prev;
    }

    if(el->m_prev)
    {
        el->m_prev->m_next = el->m_next;
    }
    else
    {
        FIRST = el->m_next;
    }
    
    if(--storage_count == 0)
    {
        FIRST = 0;
        LAST = 0;
    }

    free(el->m_name);
    free(el);
}

// Puts a new record to the STORAGE
static _WINDOW_INFO* _add_window(HWND hWnd, HWND mainhWnd, const char* name, unsigned long flags)
{
    _WINDOW_INFO* info = _add_element();

    info->m_window = hWnd;
    info->m_main = mainhWnd;
    info->m_name = strdup(name);
    info->m_flags = flags;
    info->m_image = 0;
    info->m_dc = CreateCompatibleDC(0);
    info->m_last_key = 0;
    info->m_toolbar.m_toolbar = 0;
    info->m_toolbar.m_first = 0;
    return info;
}

static void _remove_window(_WINDOW_INFO* info)
{
    if( info->m_image )
        DeleteObject(SelectObject(info->m_dc,info->m_image));

    if( info->m_dc )
        DeleteDC(info->m_dc);

    _remove_element(info);
}

static _WINDOW_INFO* _find_window_byname(const char* name)
{
    _WINDOW_INFO* info = FIRST;

    while(info != 0 && strcmp(name, info->m_name))
    {
        info = info->m_next;
    }

    return info;
}

static _WINDOW_INFO* _find_window_byhwnd(HWND hWnd)
{
    _WINDOW_INFO* info = FIRST;

    while(info != 0 && hWnd != info->m_window)
    {
        info = info->m_next;
    }

    return info;
}

static _WINDOW_INFO* _find_window_bytoolbar(HWND hWnd)
{
    _WINDOW_INFO* info = FIRST;

    while(info != 0 && hWnd != info->m_toolbar.m_toolbar)
    {
        info = info->m_next;
    }

    return info;
}

static _WINDOW_INFO* _find_mainwindow_byhwnd(HWND hWnd)
{
    _WINDOW_INFO* info = FIRST;

    while(info != 0 && hWnd != info->m_main)
    {
        info = info->m_next;
    }

    return info;
}

static _CONTROL_INFO* _find_control_byhwnd(_WINDOW_INFO* info, HWND hWnd)
{
    _CONTROL_INFO* control;

    assert(info);
    for(control = info->m_toolbar.m_first; control != 0; control = control->m_next)
    {
        if(control->m_control == hWnd)
        {
            // Control identified...
            return control;
        }
    }

    // Control not identified...
    return 0;
}

static _CONTROL_INFO* _find_control_byname(_WINDOW_INFO* info, const char* name)
{
    _CONTROL_INFO* control;

    assert(info);
    for(control = info->m_toolbar.m_first; control != 0; control = control->m_next)
    {
        if(!strcmp(control->m_name, name))
        {
            // Control identified...
            return control;
        }
    }

    // Control not identified...
    return 0;
}

static void _add_control(_WINDOW_INFO* info, _CONTROL_INFO* control)
{
    _CONTROL_INFO* it;

    assert(info);
    assert(control);

    if(info->m_toolbar.m_first != 0)
    {
        for(it = info->m_toolbar.m_first; it->m_next != 0; it = it->m_next);
        it->m_next = control;
    }
    else
    {
        info->m_toolbar.m_first = control;
    }
}

// Needed for wait_key()
static int _update_wait_list(const char* name, _WINDOW_INFO** pfirst, _WINDOW_INFO** plast)
{
    _WINDOW_INFO* info;
    if(name)
    {
        info = _find_window_byname(name);
        if(info)
        {
            *pfirst = info;
            *plast = info->m_next;
        }
        else
        {
            *pfirst = 0;
            *plast = 0;
            return HG_BADNAME;
        }
    }
    else
    {
        if(!FIRST || !LAST)
        {
            *pfirst = 0;
            *plast = 0;
            return HG_BADNAME;
        }
        *pfirst = FIRST;
        *plast = LAST->m_next;
    }

    return HG_OK;
}


static void ScreenToClientR(HWND hwnd, LPRECT rect)
{
    POINT p;
    p.x = rect->left;
    p.y = rect->top;
    ScreenToClient(hwnd, &p);
    OffsetRect(rect, p.x - rect->left, p.y - rect->top);
}

/* Calculatess the window coordinates relative to the upper left corner of the main window */
static void _calc_window_rect(_WINDOW_INFO* info, LPRECT rect)
{
    RECT crect, trect;

    assert(info);
    assert(rect);

    GetClientRect(info->m_main, &crect);
    if(info->m_toolbar.m_toolbar)
    {
        GetWindowRect(info->m_toolbar.m_toolbar, &trect);
        ScreenToClientR(info->m_main, &trect);
        SubtractRect(rect, &crect, &trect);
    }
    else
    {
        *rect = crect;
    }

    rect->top = rect->top + 2;
    rect->left = rect->left + 2;
    rect->bottom = rect->bottom - 2;
    rect->right = rect->right - 2;
}

static int _update_window_pos(_WINDOW_INFO* info, int update)
{
    RECT rect;
    int error;
    assert(info);

    if(info->m_flags & HG_AUTOSIZE && info->m_image)
    {
        int i;
        RECT rmw, rw;
        BITMAPINFO binfo;
        HDC dc = info->m_dc;
        HBITMAP hbitmap = (HBITMAP)GetCurrentObject( dc, OBJ_BITMAP );
        binfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        binfo.bmiHeader.biBitCount = 0;
        assert( dc );

        error = GetDIBits( dc, hbitmap, 0, 0, 0, &binfo, DIB_RGB_COLORS);

        // Repeat two times because after the first resizing of the main window 
        // toolbar may resize too
        for(i = 0; i < (info->m_toolbar.m_toolbar ? 2 : 1); i++)
        {
            _calc_window_rect(info, &rw);
            MoveWindow(info->m_window, rw.left, rw.top, 
                rw.right - rw.left + 1, rw.bottom - rw.top + 1, FALSE);
            GetClientRect(info->m_window, &rw);
            GetWindowRect(info->m_main, &rmw);
            // Resize the main window in order to make the bitmap fit into the child window
            error = MoveWindow(info->m_main, rmw.left, rmw.top, 
                rmw.right - rmw.left + binfo.bmiHeader.biWidth - rw.right + rw.left, 
                rmw.bottom  - rmw.top + binfo.bmiHeader.biHeight - rw.bottom + rw.top, update);
            assert(error);
        }
    }
    
    _calc_window_rect(info, &rect);
    error = MoveWindow(info->m_window, rect.left, rect.top, 
        rect.right - rect.left + 1, rect.bottom - rect.top + 1, update);
    assert(error);
    error = GetLastError();
    return error;
}

void _update_toolbar_controls(_WINDOW_INFO* info)
{
    // Recurse through all controls to move them with buttons
    _CONTROL_INFO* cinfo;
    SendMessage(info->m_toolbar.m_toolbar, TB_BUTTONCOUNT, 0, 0);
    assert(info);
    cinfo = info->m_toolbar.m_first;

    while(cinfo)
    {
        RECT rect;
        SendMessage(info->m_toolbar.m_toolbar, TB_GETITEMRECT, 
            (WPARAM)cinfo->m_id, (LPARAM)&rect);
        MoveWindow(cinfo->m_control, rect.left, rect.top, 
            rect.right - rect.left, rect.bottom - rect.top, FALSE);
        cinfo = cinfo->m_next;
    }
}

void _release_tooltips(HWND slider)
{
    int count, i;
    HWND tooltips = (HWND)SendMessage(slider, TB_GETTOOLTIPS, 0, 0);
    if(!tooltips)
        return;

    count = SendMessage(tooltips, TTM_GETTOOLCOUNT, 0, 0);
    for(i = 0; i < count - 1; i++)
    {
        TOOLINFO tinfo;
        tinfo.cbSize = sizeof(TOOLINFO);
        SendMessage(tooltips, TTM_ENUMTOOLS, i, (LPARAM)&tinfo);
        SendMessage(tooltips, TTM_DELTOOL, 0, (LPARAM)&tinfo);
    }
}


static HINSTANCE hInstance;

static LRESULT CALLBACK MainWindowProc(  HWND hwnd,      // handle to main window
  UINT uMsg,      // message identifier
  WPARAM wParam,  // first message parameter
  LPARAM lParam)   // second message parameter
{
    _WINDOW_INFO* info = _find_mainwindow_byhwnd(hwnd);
    if(!info)
    {
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }

    switch(uMsg)
    {
    case WM_WINDOWPOSCHANGED:
        {
            WINDOWPOS* pos = (WINDOWPOS*)lParam;
            RECT rc;

            // Update the toolbar position/size
            GetClientRect(info->m_main, &rc);
            if(info->m_toolbar.m_toolbar)
            {
                RECT rect;
                GetWindowRect(info->m_toolbar.m_toolbar, &rect);
                MoveWindow(info->m_toolbar.m_toolbar, 0, 0, pos->cx, rect.bottom - rect.top, TRUE);
            }

            if(!(info->m_flags & HG_AUTOSIZE))
            {
                // Update window position/size
                _update_window_pos(info, TRUE);
            }
            
            break;
        }

    case WM_ACTIVATE:
        if(LOWORD(wParam) == WA_ACTIVE || LOWORD(wParam) == WA_CLICKACTIVE)
        {
            _WINDOW_INFO* info = _find_window_byhwnd(hwnd);
            if(info)
            {
                SetFocus(info->m_window);
            }
        }
        break;

    case WM_ERASEBKGND:
        {
            RECT cr, tr, wrc;
            HRGN rgn, rgn1, rgn2;
            int ret;
            HDC hdc = (HDC)wParam;
            GetWindowRect(info->m_window, &cr);
            ScreenToClientR(info->m_main, &cr);
            if(info->m_toolbar.m_toolbar)
            {
                GetWindowRect(info->m_toolbar.m_toolbar, &tr);
                ScreenToClientR(info->m_main, &tr);
            }
            else
            {
                tr.left = tr.top = tr.right = tr.bottom = 0;
            }

            GetClientRect(info->m_main, &wrc);

            rgn = CreateRectRgn(0, 0, wrc.right, wrc.bottom);
            rgn1 = CreateRectRgn(cr.left, cr.top, cr.right, cr.bottom);
            rgn2 = CreateRectRgn(tr.left, tr.top, tr.right, tr.bottom);
            ret = CombineRgn(rgn, rgn, rgn1, RGN_DIFF);
            ret = CombineRgn(rgn, rgn, rgn2, RGN_DIFF);

            if(ret != NULLREGION && ret != ERROR)
            {
                // Retrieve the bkgrnd brush
                FillRgn(hdc, rgn, (HBRUSH)GetClassLong(hwnd, GCL_HBRBACKGROUND));
            }
        }
        return 1;
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}


static LRESULT CALLBACK WindowProc(  HWND hwnd,      // handle to window
  UINT uMsg,      // message identifier
  WPARAM wParam,  // first message parameter
  LPARAM lParam)   // second message parameter
{
    int ret;
    
    if(hg_on_preprocess)
    {
        int was_processed;
        int ret = (*hg_on_preprocess)(hwnd, uMsg, wParam, lParam, &was_processed);
        if(was_processed)
        {
            return ret;
        }
    }
    
    ret = HighGUIProc(hwnd, uMsg, wParam, lParam);
    if(hg_on_postprocess)
    {
        int was_processed;
        int ret = (*hg_on_postprocess)(hwnd, uMsg, wParam, lParam, &was_processed);
        if(was_processed)
        {
            return ret;
        }
    }
    
    return ret;
}

static LRESULT CALLBACK HighGUIProc(  HWND hwnd,      // handle to window
  UINT uMsg,      // message identifier
  WPARAM wParam,  // first message parameter
  LPARAM lParam   // second message parameter
)
{
    _WINDOW_INFO* info = 0;
    // Find the window's info
    info = _find_window_byhwnd(hwnd);
    if(info == 0)
    {
        // This window is not mentioned in HighGUI storage
        // Actually, this should be error except for the case of calls to CreateWindow
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }

    // Process the message
    switch(uMsg)
    {

    case WM_WINDOWPOSCHANGING:
        {
            LPWINDOWPOS pos = (LPWINDOWPOS)lParam;
            RECT rect;
            _calc_window_rect(info, &rect);
            pos->x = rect.left;
            pos->y = rect.top;
            pos->cx = rect.right - rect.left + 1;
            pos->cy = rect.bottom - rect.top + 1;

        }
        break;
    case WM_PAINT:
        if(info->m_image != 0)
        {
            HBITMAP hbitmap;
            BITMAPINFO binfo;
            PAINTSTRUCT paint;
            HDC hdc;
            RECT rect;
            int width, height;
            int nchannels;
            RGBQUAD table[256];

            GetClientRect(info->m_window, &rect);
            width = rect.right - rect.left;// + 1;
            height = rect.bottom - rect.top;// + 1;

            BeginPaint(hwnd, &paint);
            hdc = paint.hdc;
            assert(info->m_dc != 0);
            hbitmap = (HBITMAP)GetCurrentObject(info->m_dc, OBJ_BITMAP);
            SetStretchBltMode(hdc, COLORONCOLOR);

            // Determine the bitmap's dimensions
            binfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
            binfo.bmiHeader.biBitCount = 0;
            GetDIBits(info->m_dc, hbitmap, 0, 0, 0, &binfo, DIB_PAL_COLORS);

            nchannels = binfo.bmiHeader.biBitCount/8;
            assert(nchannels == 1 || nchannels == 3);

            if(nchannels == 1)
            {
                int i;
                for(i = 0; i < 256; i++)
                {
                    table[i].rgbBlue = (unsigned char)i;
                    table[i].rgbGreen = (unsigned char)i;
                    table[i].rgbRed = (unsigned char)i;
                }
                SetDIBColorTable(info->m_dc, 0, 255, table);
            }

            if(info->m_flags & HG_AUTOSIZE)
            {
                BitBlt(hdc, 0, 0, binfo.bmiHeader.biWidth, binfo.bmiHeader.biHeight,
                                info->m_dc, 0, 0, SRCCOPY);
            }
            else
            {
                StretchBlt(hdc, 0, 0, width, height,
                                info->m_dc, 0, 0, binfo.bmiHeader.biWidth, 
                                binfo.bmiHeader.biHeight, SRCCOPY);
            }
            DeleteDC(hdc);
        }
        else
        {
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
        }
        return 0;

    case WM_LBUTTONDOWN:
        SetFocus(hwnd);
        return 0;

    case WM_ERASEBKGND:
        if(info->m_image)
            return 0;
        break;
        
    case WM_DESTROY:
        assert(uMsg == WM_DESTROY);
        _remove_window(info);
        // Do nothing!!!
        //PostQuitMessage(0);
        break;

    case WM_SETCURSOR:
        SetCursor((HCURSOR)GetClassLong(hwnd, GCL_HCURSOR));
        return 0;

    case WM_KEYDOWN:
        info->m_last_key = (int)wParam;
        return 0;
    }
    
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
};

static LRESULT CALLBACK HGToolbarProc(  HWND hwnd,      // handle to window
  UINT uMsg,      // message identifier
  WPARAM wParam,  // first message parameter
  LPARAM lParam   // second message parameter
)
{
    _WINDOW_INFO* info = _find_window_bytoolbar(hwnd);
    if(!info)
    {
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }

    switch(uMsg)
    {
    // Control messages processing
        // Slider processing
    case WM_HSCROLL:
        {
            HWND slider = (HWND)lParam;
            int pos = SendMessage(slider, TBM_GETPOS, 0, 0);
            int update = 0;
            _CONTROL_INFO* sinfo;
            assert(info);

            // Find the slider
            sinfo = _find_control_byhwnd(info, slider);
            assert(sinfo);
            if(sinfo->m_data)
            {
                if(*(int*)sinfo->m_data != pos)
                {
                    // Need an update
                    update = 1;
                    _release_tooltips(slider);
                }
                *(int*)sinfo->m_data = pos;
            }
            if(update && sinfo->m_notify)
            {
                sinfo->m_notify((int)slider);
            }

            return 0;
        }

    case WM_NCCALCSIZE:
        {
            LRESULT ret = CallWindowProc(info->m_toolbar.m_toolBarProc, hwnd, uMsg, wParam, lParam);
            int rows = SendMessage(hwnd, TB_GETROWS, 0, 0);

            if(info->m_toolbar.m_rows != rows)    
            {
                _update_toolbar_controls(info);

                info->m_toolbar.m_rows = rows;
            }

            return ret;
        }
        
    }

    return CallWindowProc(info->m_toolbar.m_toolBarProc, hwnd, uMsg, wParam, lParam);
}


void SetInstance(void* curInstance)
{
    hInstance = (HINSTANCE)curInstance;
}


static bool RegisterHighGUIClass()
{
    WNDCLASS wndc;
    wndc.style = CS_OWNDC | CS_VREDRAW | CS_HREDRAW;
    wndc.lpfnWndProc = WindowProc;
    wndc.cbClsExtra = 0;
    wndc.cbWndExtra = 0;
    wndc.hInstance = hInstance;
    wndc.lpszClassName = "HighGUI class";
    wndc.lpszMenuName = "HighGUI class";
    wndc.hIcon = LoadIcon(0, IDI_APPLICATION); 
    wndc.hCursor = LoadCursor(0, MAKEINTRESOURCE(IDC_ARROW));
    wndc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);  

    if(!RegisterClass(&wndc))
        return false;

    wndc.lpszClassName = "Main HighGUI class";
    wndc.lpszMenuName = "Main HighGUI class";
    wndc.hbrBackground = (HBRUSH)GetStockObject(GRAY_BRUSH);
    wndc.lpfnWndProc = MainWindowProc;

    if(!RegisterClass(&wndc))
        return false;

    return true;
}


int cvvInitSystem( int, char** )
{
    static int wasInitialized = 0;    
    bool isok = true;

    // check initialization status
    if( !wasInitialized )
    {
        // Initialize the stogare
        storage_count = 0;
        FIRST = LAST = 0;
        
        // Register the class
        isok = RegisterHighGUIClass();

        if( isok )
            wasInitialized = 1;
    }

    return isok ? HG_OK : HG_INITFAILED;
}

HIGHGUI_IMPL int
cvvNamedWindow(const char* name, unsigned long flags)
{
    HWND hWnd, mainhWnd;
    _WINDOW_INFO* info;
    DWORD defStyle;
    int result = HGInitialize();

    if( result < 0 )
    {
        return result;
    }

    if(!name)
    {
        return HG_NULLPTR;
    }

    // Check the name in the storage
    if(_find_window_byname(name))
    {
        return HG_BADNAME;
    }

    defStyle = WS_VISIBLE | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_SYSMENU;
    if(!(flags & HG_AUTOSIZE))
    {
        defStyle |= WS_SIZEBOX;
    }

    mainhWnd = CreateWindow("Main HighGUI class", name, defStyle  | WS_OVERLAPPED,
                CW_USEDEFAULT, 0, 320, 320, 0, 0, hInstance, 0);
    ShowWindow(mainhWnd, SW_SHOW);
    if(mainhWnd == 0)
    {
        return HG_WCFAILED;
    }

    hWnd = CreateWindow("HighGUI class", "", defStyle | WS_CHILD | WS_SIZEBOX, 
                CW_USEDEFAULT, 0, 320, 320, mainhWnd, 0, hInstance, 0);
    ShowWindow(hWnd, SW_SHOW);
    if(hWnd == 0)
    {
        return HG_WCFAILED;
    }

    info = _add_window(hWnd, mainhWnd, name, flags);

    // Recalculate window position
    _update_window_pos(info, TRUE);

    return 0;
}

HIGHGUI_IMPL int
cvvDestroyWindow(const char* name)
{
    _WINDOW_INFO* info;
    HWND main;

    if(!name)
    {
        return HG_NULLPTR;
    }

    info = _find_window_byname(name);
    if(info == 0)
    {
        // Window is not mentioned in the HighGUI's storage
        return HG_BADNAME;
    }

    main = info->m_main;
    SendMessage(info->m_window, WM_CLOSE, 0, 0);
    SendMessage(main, WM_CLOSE, 0, 0);
    // Do NOT call _remove_window -- _WINDOW_INFO list will be updated automatically ...
    return 0;    
}

void destroy_all()
{
    HWND main, child;
    _WINDOW_INFO* info = FIRST;

    while(info)
    {
        main = info->m_main;
        child = info->m_window;
        info = info->m_next;

        SendMessage(child, WM_CLOSE, 0, 0);
        SendMessage(main, WM_CLOSE, 0, 0);
    }
}


HIGHGUI_IMPL void
cvvShowImage( const char* window_name, const CvArr* arr )
{
    CV_FUNCNAME( "cvvShowImage" );

    __BEGIN__;
    
    _WINDOW_INFO* info = _find_window_byname(window_name);
    int width = 0, height = 0, bpp = 0;
    const int bpp0 = 24;
    void* dst_ptr = 0;
    int dst_step, origin = 0;
    CvMat stub, dst, *image;

    if( !info || !arr )
        EXIT; // keep silence here.

    if( _CV_IS_IMAGE( arr ))
        origin = ((IplImage*)arr)->origin;

    CV_CALL( image = cvGetMat( arr, &stub ));

    if( info->m_image )
    {
        HBITMAP hbitmap = (HBITMAP)GetCurrentObject( info->m_dc, OBJ_BITMAP );
        BITMAP bmp;
        memset( &bmp, 0, sizeof(bmp));
        GetObject( hbitmap, sizeof(bmp), &bmp );
        width = bmp.bmWidth;
        height = bmp.bmHeight;
        bpp = bmp.bmBitsPixel;
        dst_ptr = bmp.bmBits;
    }

    if( width != image->width || height != image->height || bpp != bpp0 )
    {
        uchar buffer[sizeof(BITMAPINFO) + 255*sizeof(RGBQUAD)];
        BITMAPINFO* binfo = (BITMAPINFO*)buffer;
        
        DeleteObject( SelectObject( info->m_dc, info->m_image ));
        info->m_image = 0;

        width = image->width;
        height = image->height;
        bpp = bpp0;

        FillBitmapInfo( binfo, width, height, bpp, 1 );

        info->m_image = SelectObject( info->m_dc, CreateDIBSection(info->m_dc, binfo,
                                      DIB_RGB_COLORS, &dst_ptr, 0, 0));
    }

    dst_step = (width * (bpp >> 3) + 3) & -4;
    cvInitMatHeader( &dst, height, width, CV_8UC3, dst_ptr, dst_step );
    cvvConvertImage( image, &dst, origin == 0 );
    
    _update_window_pos(info, TRUE);
    InvalidateRect(info->m_window, 0, 0);
    UpdateWindow(info->m_window);

    __END__;
}


HIGHGUI_IMPL int
cvvWaitKey(const char* name)
{
    _WINDOW_INFO* info;
    MSG message;
    _WINDOW_INFO *first, *last;
    int is_processed;

    for(;;)
    {
        if(_update_wait_list(name, &first, &last))
        {
            return 0;
        }

        GetMessage(&message, 0, 0, 0);
        is_processed = 0;
        for(info = first; info != last && is_processed == 0; info = info->m_next)
        {
            if(info->m_window == message.hwnd || info->m_main == message.hwnd)
            {
                is_processed = 1;
                switch(message.message)
                {
                case WM_DESTROY:
                case WM_CHAR:
                    DispatchMessage(&message);
                    return (int)message.wParam;

                case WM_KEYDOWN:
                    TranslateMessage(&message);
                default:
                    DispatchMessage(&message);
                    is_processed = 1;
                    break;
                }
            }
        }
        if(!is_processed)
        {
            TranslateMessage(&message);
            DispatchMessage(&message);
        }

        /* If the window has been destroyed during message processing, return! */
        if(_update_wait_list(name, &first, &last))
        {
            return 0;
        }
    }
}

HIGHGUI_IMPL int
cvvWaitKeyEx(const char* name, int delay)
{
    _WINDOW_INFO* info;
    MSG message;
    _WINDOW_INFO *first, *last;
    int is_processed;
    int time = GetTickCount();

    for(;;)
    {
        if(abs(GetTickCount() - time) >= delay)
            return -1;

        if(_update_wait_list(name, &first, &last))
        {
            return HG_BADNAME;
        }

        if(PeekMessage(&message, 0, 0, 0, PM_REMOVE) == FALSE)
        {
            Sleep(1);
            continue;
        }
        is_processed = 0;
        for(info = first; info != last && is_processed == 0; info = info->m_next)
        {
            if(info->m_window == message.hwnd || info->m_main == message.hwnd)
            {
                is_processed = 1;
                switch(message.message)
                {
                case WM_DESTROY:
                case WM_CHAR:
                    DispatchMessage(&message);
                    return (int)message.wParam;

                case WM_KEYDOWN:
                    TranslateMessage(&message);
                default:
                    DispatchMessage(&message);
                    is_processed = 1;
                    break;
                }
            }
        }
        if(!is_processed)
        {
            TranslateMessage(&message);
            DispatchMessage(&message);
        }

        /* If the window has been destroyed during message processing, return! */
        if(_update_wait_list(name, &first, &last))
        {
            return HG_BADNAME;
        }
    }
}

int create_toolbar(const char* window_name/*, int pos*/)
{
    _WINDOW_INFO* info;
    HWND toolbar;
    _TOOLBAR_INFO tinfo;
    RECT rect;
    const int default_height = 30;

    if(!window_name)
    {
        return HG_NULLPTR;
    }

    info = _find_window_byname(window_name);
    if(!info)
    {
        return HG_BADNAME;
    }

    toolbar = CreateToolbarEx(info->m_main, WS_CHILD | CCS_TOP |
        TBSTYLE_TOOLTIPS | TBSTYLE_WRAPABLE,
        1, 0, 0, 0, 0, 0, 16, 20, 16, 16, sizeof(TBBUTTON));
    GetClientRect(info->m_main, &rect);
    MoveWindow(toolbar, 0, 0, rect.right - rect.left, default_height, TRUE);

    SendMessage(toolbar, TB_AUTOSIZE, 0, 0);
    ShowWindow(toolbar, SW_SHOW);

    tinfo.m_toolbar = toolbar;
    tinfo.m_first = 0;
    tinfo.m_toolBarProc = (WNDPROC)GetWindowLong(toolbar, GWL_WNDPROC);
    info->m_toolbar = tinfo;

//    SendMessage(toolbar, TB_SETROWS, MAKEWPARAM(2, TRUE), (LPARAM)&rect);

/*    _calc_window_rect(info, &rect);
    MoveWindow(info->m_window, rect.left, rect.top,
        rect.right - rect.left + 1, rect.bottom - rect.top + 1, TRUE);*/
    _update_window_pos(info, TRUE);

    // Subclassing from toolbar
    SetWindowLong(toolbar, GWL_WNDPROC, (LONG)HGToolbarProc);
    return HG_OK;
}


typedef struct
{
    UINT cbSize;
    DWORD dwMask;
    int idCommand;
    int iImage;
    BYTE fsState;
    BYTE fsStyle;
    WORD cx;
    DWORD lParam;
    LPSTR pszText;
    int cchText;
}
ButtonInfo;


int cvvCreateTrackbar(const char* trackbar_name, const char* window_name, int* val,
                    int count, void (*ON_NOTIFY)(int))
{
    char slider_name[] = "Slider12345678";
    DWORD style = WS_CHILD | WS_VISIBLE | TBS_AUTOTICKS |
        TBS_FIXEDLENGTH/* | TBS_TOOLTIPS*/;
    _WINDOW_INFO* info = 0;
    int w; //, x = 0, y = 0, h = 30;
    RECT rect;
    HWND slider_hwnd = 0;
    int tic_freq;
    TBBUTTON tbs;
    ButtonInfo tbis;
    int bcount;
    _CONTROL_INFO* slider;
    int min = 0, max = count;

    if(!window_name)
    {
        return HG_NULLPTR;
    }

    if(count < 0 || max <= min)
    {
        return HG_BADPARAM;
    }

    info = _find_window_byname(window_name);
    if(info == 0)
    {
        return HG_BADNAME;
    }

    slider = _find_control_byname(info, trackbar_name);
    if(!slider)
    {
        // Create a slider
        if(!info->m_toolbar.m_toolbar)
        {
            // Create a toolbar
            int ret = create_toolbar(window_name/*, 0*/);
            assert(ret == HG_OK);
            if(ret != HG_OK)
            {
                return ret;
            }
        }

        GetClientRect(info->m_window, &rect);
        w = rect.right - rect.left;
        //h = slider_height;
        style |= TBS_HORZ | TBS_BOTTOM;

        /* Retrieve current buttons count */
        bcount = SendMessage(info->m_toolbar.m_toolbar, TB_BUTTONCOUNT, 0, 0);

        if(bcount > 1)
        {
            /* If this is not the first button then we need to
            separate it from the previous one */
            tbs.iBitmap = 0;
            tbs.idCommand = bcount; // Set button id to it's number
            tbs.iString = 0;
            tbs.fsStyle = TBSTYLE_SEP;
            tbs.fsState = TBSTATE_ENABLED;
            SendMessage(info->m_toolbar.m_toolbar, TB_ADDBUTTONS, 1, (LPARAM)&tbs);

            // Retrieve current buttons count
            bcount = SendMessage(info->m_toolbar.m_toolbar, TB_BUTTONCOUNT, 0, 0);
        }

        /* Add a button which we're going to cover with the slider */
        tbs.iBitmap = 0;
        tbs.idCommand = bcount; // Set button id to it's number
        tbs.iString = 0;
        tbs.fsStyle = TBSTYLE_GROUP;
        tbs.fsState = TBSTATE_ENABLED;
        SendMessage(info->m_toolbar.m_toolbar, TB_ADDBUTTONS, 1, (LPARAM)&tbs);

        /* Adjust button size to the slider */
        tbis.cbSize = sizeof(tbis);
        tbis.dwMask = TBIF_SIZE;
        tbis.cx = (unsigned short)w;
        SendMessage(info->m_toolbar.m_toolbar, TB_SETBUTTONINFO,
            (WPARAM)tbs.idCommand, (LPARAM)&tbis);

        /* Get button position */
        SendMessage(info->m_toolbar.m_toolbar, TB_GETITEMRECT,
            (WPARAM)tbs.idCommand, (LPARAM)&rect);

        /* Create a slider */
        sprintf(slider_name + 6, "%p", val);
        slider_hwnd = CreateWindowEx(0, TRACKBAR_CLASS, slider_name, style,
            rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top,
            info->m_toolbar.m_toolbar, (HMENU)bcount, hInstance, 0);
        assert(slider_hwnd);

        /* Adjust slider parameters */
        SendMessage(slider_hwnd, TBM_SETRANGE,
            (WPARAM) TRUE,                   // redraw flag
            (LPARAM) MAKELONG(min, max));  // min. & max. positions

        tic_freq = max - min - 1 > count ? (max - min - 1)/count : 1;
          SendMessage(slider_hwnd, TBM_SETTICFREQ,
            (WPARAM) tic_freq,
            (LPARAM) 0);

        SendMessage(slider_hwnd, TBM_SETPOS,
            (WPARAM) TRUE,                   // redraw flag
            (LPARAM) *val);

        SendMessage(info->m_toolbar.m_toolbar, TB_AUTOSIZE, 0, 0);

        /* Remember slider parameters */
        slider = (_CONTROL_INFO*)malloc(sizeof(_CONTROL_INFO));
        slider->m_name = strdup(trackbar_name);
        slider->m_control = slider_hwnd;
        slider->m_id = bcount;
        slider->m_next = 0;
        _add_control(info, slider);
    }
    else
    {
        // Initialize the variable with the slider's current position
        *val = SendMessage(slider->m_control, TBM_GETPOS, 0, 0);
    }

    slider->m_data = (void*)val;
    slider->m_notify = ON_NOTIFY;

    /* Minimize the number of rows */
    SendMessage(info->m_toolbar.m_toolbar, TB_SETROWS, MAKEWPARAM(1, FALSE), (LPARAM)&rect);

    /* Resize the window to reflect the toolbar resizing*/
    _update_window_pos(info, TRUE);

    return (int)slider->m_control;
}


int check_key(const char* window_name)
{
    _WINDOW_INFO* info = _find_window_byname(window_name);
    if(info == 0)
        return HG_BADNAME;

    return info->m_last_key;
}

int reset_key(const char* window_name)
{
    _WINDOW_INFO* info = _find_window_byname(window_name);
    if(info == 0)
        return HG_BADNAME;

    info->m_last_key = 0;
    return HG_OK;
}

void detach_all_controls()
{
    _WINDOW_INFO* info = FIRST;

    while(info)
    {
        _CONTROL_INFO* control = info->m_toolbar.m_first;
        while(control)
        {
            control->m_data = 0;
            control->m_notify = 0;
            /*EnableWindow(control->m_control, FALSE);*/
            control = control->m_next;
        }

        info = info->m_next;
    }
}

int cvvResizeWindow(const char* window_name, int width, int height)
{
    int i;
    _WINDOW_INFO* info;
    RECT rmw, rw, rect;

    info = _find_window_byname(window_name);
    if(!info)
        return HG_BADNAME;

    // Repeat two times because after the first resizing of the main window
    // toolbar may resize too
    for(i = 0; i < (info->m_toolbar.m_toolbar ? 2 : 1); i++)
    {
        _calc_window_rect(info, &rw);
        MoveWindow(info->m_window, rw.left, rw.top,
            rw.right - rw.left + 1, rw.bottom - rw.top + 1, FALSE);
        GetClientRect(info->m_window, &rw);
        GetWindowRect(info->m_main, &rmw);
        // Resize the main window in order to make the bitmap fit into the child window
        MoveWindow(info->m_main, rmw.left, rmw.top,
            rmw.right - rmw.left + width - rw.right + rw.left,
            rmw.bottom  - rmw.top + height - rw.bottom + rw.top, TRUE);
    }

    _calc_window_rect(info, &rect);
    MoveWindow(info->m_window, rect.left, rect.top,
        rect.right - rect.left + 1, rect.bottom - rect.top + 1, TRUE);
    return HG_OK;
}

HWND get_hwnd_byname(const char* name)
{
    _WINDOW_INFO* info;
    info = _find_window_byname(name);
    if(info == 0)
    {
        return 0;
    }
    else
    {
        return info->m_window;
    }
}

const char* get_name_byhwnd(HWND hwnd)
{
    _WINDOW_INFO* info = FIRST;

    while(info != 0 && info->m_window != hwnd)
    {
        info = info->m_next;
    }

    return info == 0 ? 0 : info->m_name;
}

void set_preprocess_func(int (__cdecl *on_preprocess)(HWND, UINT, WPARAM, LPARAM, int*))
{
    if(on_preprocess)
        hg_on_preprocess = on_preprocess;
    else
        assert(on_preprocess);
}

void set_postprocess_func(int (__cdecl *on_postprocess)(HWND, UINT, WPARAM, LPARAM, int*))
{
    if(on_postprocess)
        hg_on_postprocess = on_postprocess;
    else
        assert(on_postprocess);
}

#endif //WIN32
