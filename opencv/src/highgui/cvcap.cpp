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

#include <vfw.h>
#if _MSC_VER >= 1200
#pragma warning( disable: 4711 )
#endif

#else

extern "C" {
#include "ffmpeg/avcodec.h"
}

#endif

/************************* Reading AVIs & Camera data **************************/

/***************************** Common definitions ******************************/

#define CV_CAPTURE_BASE_API_COUNT 6

typedef void (CV_CDECL* CvCaptureCloseFunc)( CvCapture* capture );
typedef int (CV_CDECL* CvCaptureGrabFrameFunc)( CvCapture* capture );
typedef IplImage* (CV_CDECL* CvCaptureRetrieveFrameFunc)( CvCapture* capture );
typedef double (CV_CDECL* CvCaptureGetPropertyFunc)( CvCapture* capture, int id );
typedef int (CV_CDECL* CvCaptureSetPropertyFunc)( CvCapture* capture,
                                                  int id, double value );
typedef const char* (CV_CDECL* CvCaptureGetDescriptionFunc)( CvCapture* capture );

typedef struct CvCaptureVTable
{
    int     count;
    CvCaptureCloseFunc close;
    CvCaptureGrabFrameFunc grab_frame;
    CvCaptureRetrieveFrameFunc retrieve_frame;
    CvCaptureGetPropertyFunc get_property;
    CvCaptureSetPropertyFunc set_property;
    CvCaptureGetDescriptionFunc get_description;
}
CvCaptureVTable;

typedef struct CvCapture
{
    CvCaptureVTable* vtable;
}
CvCapture;


HIGHGUI_IMPL void cvReleaseCapture( CvCapture** pcapture )
{
    if( pcapture && *pcapture )
    {
        CvCapture* capture = *pcapture;
        if( capture && capture->vtable &&
            capture->vtable->count >= CV_CAPTURE_BASE_API_COUNT &&
            capture->vtable->close )
            capture->vtable->close( capture );
        cvFree( (void**)pcapture );
    }
}


HIGHGUI_IMPL IplImage* cvQueryFrame( CvCapture* capture )
{
    if( capture && capture->vtable &&
        capture->vtable->count >= CV_CAPTURE_BASE_API_COUNT &&
        capture->vtable->grab_frame && capture->vtable->retrieve_frame &&
        capture->vtable->grab_frame( capture ))
        return capture->vtable->retrieve_frame( capture );
    return 0;
}

HIGHGUI_IMPL int cvGrabFrame( CvCapture* capture )
{
    if( capture && capture->vtable &&
        capture->vtable->count >= CV_CAPTURE_BASE_API_COUNT &&
        capture->vtable->grab_frame )
        return capture->vtable->grab_frame( capture );
    return 0;
} 

HIGHGUI_IMPL IplImage* cvRetrieveFrame( CvCapture* capture )
{
    if( capture && capture->vtable &&
        capture->vtable->count >= CV_CAPTURE_BASE_API_COUNT &&
        capture->vtable->retrieve_frame )
        return capture->vtable->retrieve_frame( capture );
    return 0;
}                                       

HIGHGUI_IMPL double cvGetCaptureProperty( CvCapture* capture, int id )
{
    if( capture && capture->vtable &&
        capture->vtable->count >= CV_CAPTURE_BASE_API_COUNT &&
        capture->vtable->get_property )
        return capture->vtable->get_property( capture, id );
    return 0;
}


HIGHGUI_IMPL int cvSetCaptureProperty( CvCapture* capture, int id, double value )
{
    if( capture && capture->vtable &&
        capture->vtable->count >= CV_CAPTURE_BASE_API_COUNT &&
        capture->vtable->set_property )
        return capture->vtable->set_property( capture, id, value );
    return 0;
}


/********************* Capturing video from AVI via VFW ************************/

#ifdef WIN32

static BITMAPINFOHEADER
icvBitmapHeader( int width, int height, int bpp, int compression = BI_RGB )
{
    BITMAPINFOHEADER bmih;
    memset( &bmih, 0, sizeof(bmih));
    bmih.biSize = sizeof(bmih);
    bmih.biWidth = width;
    bmih.biHeight = height;
    bmih.biBitCount = (WORD)bpp;
    bmih.biCompression = compression;
    bmih.biPlanes = 1;

    return bmih;
}

static void icvInitCapture_VFW()
{
    static int isInitialized = 0;
    if( !isInitialized )
    {
        AVIFileInit();
        isInitialized = 1;
    }
}


typedef struct CvCaptureAVI_VFW
{
    CvCaptureVTable* vtable;
    PAVIFILE avifile;
    PAVISTREAM avistream;
    PGETFRAME getframe;
    AVISTREAMINFO aviinfo;
    BITMAPINFOHEADER* bmih;
    CvSlice film_range;
    double fps;
    int pos;
    IplImage frame;
}
CvCaptureAVI_VFW;


static void icvCloseAVI_VFW( CvCaptureAVI_VFW* capture )
{
    if( capture->getframe )
    {
        AVIStreamGetFrameClose( capture->getframe );
        capture->getframe = 0;
    }
    if( capture->avistream )
    {
        AVIStreamRelease( capture->avistream );
        capture->avistream = 0;
    }
    if( capture->avifile )
    {
        AVIFileRelease( capture->avifile );
        capture->avifile = 0;
    }
    capture->bmih = 0;
    capture->pos = 0;
    capture->film_range.startIndex = capture->film_range.endIndex = 0;
    memset( &capture->frame, 0, sizeof(capture->frame));
}


static int icvOpenAVI_VFW( CvCaptureAVI_VFW* capture, const char* filename )
{
    HRESULT hr;

    icvInitCapture_VFW();

    if( !capture )
        return 0;

    hr = AVIFileOpen( &capture->avifile, filename, OF_READ, NULL );
    if( SUCCEEDED(hr))
    {
        hr = AVIFileGetStream( capture->avifile, &capture->avistream, streamtypeVIDEO, 0 );
        if( SUCCEEDED(hr))
        {
            hr = AVIStreamInfo( capture->avistream, &capture->aviinfo,
                                    sizeof(capture->aviinfo));
            if( SUCCEEDED(hr))
            {
                BITMAPINFOHEADER bmih = icvBitmapHeader(
                    capture->aviinfo.rcFrame.right -
                    capture->aviinfo.rcFrame.left,
                    capture->aviinfo.rcFrame.bottom -
                    capture->aviinfo.rcFrame.top,
                    24 );
                capture->film_range.startIndex = (int)capture->aviinfo.dwStart;
                capture->film_range.endIndex = capture->film_range.startIndex +
                                                (int)capture->aviinfo.dwLength;
                capture->fps = ((double)capture->aviinfo.dwRate)/capture->aviinfo.dwScale;
                capture->pos = capture->film_range.startIndex;
                capture->getframe = AVIStreamGetFrameOpen( capture->avistream, &bmih );
                if( capture->getframe != 0 )
                    return 1;
            }
        }
    }

    icvCloseAVI_VFW( capture );
    return 0;
}


static int icvGrabFrameAVI_VFW( CvCaptureAVI_VFW* capture )
{
    if( capture->avistream )
    {
        capture->bmih = (BITMAPINFOHEADER*)
            AVIStreamGetFrame( capture->getframe, capture->pos++ );
    }

    return capture->bmih != 0;
}


static const IplImage* icvRetrieveFrameAVI_VFW( CvCaptureAVI_VFW* capture )
{
    if( capture->avistream && capture->bmih )
    {
        cvInitImageHeader( &capture->frame,
                           cvSize( capture->bmih->biWidth,
                                   capture->bmih->biHeight ),
                           IPL_DEPTH_8U, 3, IPL_ORIGIN_BL, 4 );
        capture->frame.imageData = capture->frame.imageDataOrigin =
            (char*)(capture->bmih + 1);
        return &capture->frame;
    }

    return 0;
}


static double icvGetPropertyAVI_VFW( CvCaptureAVI_VFW* capture, int property_id )
{
    switch( property_id )
    {
    case CV_CAP_PROP_POS_MSEC:
        return cvRound(capture->pos*1000./capture->fps);
        break;
    case CV_CAP_PROP_POS_FRAMES:
        return capture->pos;
    case CV_CAP_PROP_POS_AVI_RATIO:
        return (capture->pos - capture->film_range.startIndex)/
               (capture->film_range.endIndex - capture->film_range.startIndex + 1e-10);
    case CV_CAP_PROP_FRAME_WIDTH:
        return capture->frame.width;
    case CV_CAP_PROP_FRAME_HEIGHT:
        return capture->frame.height;
    case CV_CAP_PROP_FPS:
        return capture->fps;
    case CV_CAP_PROP_FOURCC:
        return capture->aviinfo.fccHandler;
    }
    return 0;
}


static int icvSetPropertyAVI_VFW( CvCaptureAVI_VFW* capture,
                                  int property_id, double value )
{
    switch( property_id )
    {
    case CV_CAP_PROP_POS_MSEC:
    case CV_CAP_PROP_POS_FRAMES:
    case CV_CAP_PROP_POS_AVI_RATIO:
        {
            int pos;
            switch( property_id )
            {
            case CV_CAP_PROP_POS_MSEC:
                pos = cvRound(value*capture->fps*0.001);
                break;
            case CV_CAP_PROP_POS_AVI_RATIO:
                pos = cvRound(value*(capture->film_range.endIndex - 
                                     capture->film_range.startIndex) +
                              capture->film_range.startIndex);
                break;
            default:
                pos = cvRound(value);
            }
            if( pos < capture->film_range.startIndex )
                pos = capture->film_range.startIndex;
            if( pos > capture->film_range.endIndex )
                pos = capture->film_range.endIndex;
            capture->pos = pos;
        }
        break;
    default:
        return 0;
    }

    return 1;
}

static CvCaptureVTable captureAVI_VFW_vtable = 
{
    6,
    (CvCaptureCloseFunc)icvCloseAVI_VFW,
    (CvCaptureGrabFrameFunc)icvGrabFrameAVI_VFW,
    (CvCaptureRetrieveFrameFunc)icvRetrieveFrameAVI_VFW,
    (CvCaptureGetPropertyFunc)icvGetPropertyAVI_VFW,
    (CvCaptureSetPropertyFunc)icvSetPropertyAVI_VFW,
    (CvCaptureGetDescriptionFunc)0
};


HIGHGUI_IMPL CvCapture* cvCaptureFromAVI( const char* filename )
{
    CvCaptureAVI_VFW* capture = (CvCaptureAVI_VFW*)cvAlloc( sizeof(*capture));
    memset( capture, 0, sizeof(*capture));

    capture->vtable = &captureAVI_VFW_vtable;

    if( !icvOpenAVI_VFW( capture, filename ))
        cvReleaseCapture( (CvCapture**)&capture );

    return (CvCapture*)capture;
}

/********************* Capturing video from camera via VFW *********************/

typedef struct CvCaptureCAM_VFW
{
    CvCaptureVTable* vtable;
    CAPDRIVERCAPS caps;
    HWND   capWnd;
    VIDEOHDR* hdr;
    DWORD  fourcc;
    HIC    hic;
    IplImage* rgb_frame;
    IplImage frame;
}
CvCaptureCAM_VFW;


static LRESULT PASCAL
FrameCallbackProc( HWND hWnd, VIDEOHDR* hdr ) 
{ 
    CvCaptureCAM_VFW* capture = 0;

    if (!hWnd) return FALSE;

    capture = (CvCaptureCAM_VFW*)capGetUserData(hWnd);
    capture->hdr = hdr;

    return (LRESULT)TRUE; 
} 


// Initialize camera input
static int icvOpenCAM_VFW( CvCaptureCAM_VFW* capture, int wIndex )
{
    char szDeviceName[80];
    char szDeviceVersion[80];
    HWND hWndC = 0;
    
    if( (unsigned)wIndex >= 10 )
        wIndex = 0;

    for( ; wIndex < 10; wIndex++ ) 
    {
        if( capGetDriverDescription( wIndex, szDeviceName, 
            sizeof (szDeviceName), szDeviceVersion, 
            sizeof (szDeviceVersion))) 
        {
            hWndC = capCreateCaptureWindow ( "My Own Capture Window", 
                WS_POPUP & WS_CHILD & WS_VISIBLE, 0, 0, 320, 240, 0, 0);
            if( capDriverConnect (hWndC, wIndex))
                break;
            DestroyWindow( hWndC );
            hWndC = 0;
        }
    }
    
    if( hWndC )
    {
        capture->capWnd = hWndC;
        capture->hdr = 0;
        capture->hic = 0;
        capture->fourcc = (DWORD)-1;
        capture->rgb_frame = 0;
        
        memset( &capture->caps, 0, sizeof(capture->caps));
        capDriverGetCaps( hWndC, &capture->caps, sizeof(&capture->caps));
        ::MoveWindow( hWndC, 0, 0, 320, 240, TRUE );
        capSetUserData( hWndC, (long)capture );
        capSetCallbackOnFrame( hWndC, FrameCallbackProc ); 
        CAPTUREPARMS p;
        capCaptureGetSetup(hWndC,&p,sizeof(CAPTUREPARMS));
        p.dwRequestMicroSecPerFrame = 66667/2;
        capCaptureSetSetup(hWndC,&p,sizeof(CAPTUREPARMS));
        //capPreview( hWndC, 1 );
        capPreviewScale(hWndC,FALSE);
        capPreviewRate(hWndC,1);
    }
    return capture->capWnd != 0;
}

static  void icvCloseCAM_VFW( CvCaptureCAM_VFW* capture )
{
    if( capture && capture->capWnd )
    {
        capSetCallbackOnFrame( capture->capWnd, NULL ); 
        capDriverDisconnect( capture->capWnd );
        DestroyWindow( capture->capWnd );
        cvReleaseImage( &capture->rgb_frame );
        if( capture->hic )
        {
            ICDecompressEnd( capture->hic );
            ICClose( capture->hic );
        }

        capture->capWnd = 0;
        capture->hic = 0;
        capture->hdr = 0;
        capture->fourcc = 0;
        capture->rgb_frame = 0;
        memset( &capture->frame, 0, sizeof(capture->frame));
    }
}


static int icvGrabFrameCAM_VFW( CvCaptureCAM_VFW* capture )
{
    if( capture->capWnd )
    {
        SendMessage( capture->capWnd, WM_CAP_GRAB_FRAME_NOSTOP, 0, 0 );
        return 1;
    }
    return 0;
}


static IplImage* icvRetrieveFrameCAM_VFW( CvCaptureCAM_VFW* capture )
{
    if( capture->capWnd )
    {
        BITMAPINFO vfmt;
        memset( &vfmt, 0, sizeof(vfmt));
        int sz = capGetVideoFormat( capture->capWnd, &vfmt, sizeof(vfmt));

        if( capture->hdr && capture->hdr->lpData && sz != 0 )
        {
            long code = ICERR_OK;
            char* frame_data = (char*)capture->hdr->lpData;

            if( vfmt.bmiHeader.biCompression != BI_RGB ||
                vfmt.bmiHeader.biBitCount != 24 )
            {
                BITMAPINFOHEADER& vfmt0 = vfmt.bmiHeader;
                BITMAPINFOHEADER vfmt1 = icvBitmapHeader( vfmt0.biWidth, vfmt0.biHeight, 24 );
                code = ICERR_ERROR;

                if( capture->hic == 0 ||
                    capture->fourcc != vfmt0.biCompression ||
                    capture->rgb_frame == 0 ||
                    vfmt0.biWidth != capture->rgb_frame->width ||
                    vfmt0.biHeight != capture->rgb_frame->height )
                {
                    if( capture->hic )
                    {
                        ICDecompressEnd( capture->hic );
                        ICClose( capture->hic );
                    }
                    capture->hic = ICOpen( MAKEFOURCC('V','I','D','C'),
                                            vfmt0.biCompression, ICMODE_DECOMPRESS );
                    if( capture->hic &&
                        ICDecompressBegin( capture->hic, &vfmt0, &vfmt1 ) == ICERR_OK )
                    {
                        cvReleaseImage( &capture->rgb_frame );
                        capture->rgb_frame = cvCreateImage(
                            cvSize( vfmt0.biWidth, vfmt0.biHeight ), IPL_DEPTH_8U, 3 );
                        capture->rgb_frame->origin = IPL_ORIGIN_BL;

                        code = ICDecompress( capture->hic, 0,
                                             &vfmt0, capture->hdr->lpData,
                                             &vfmt1, capture->rgb_frame->imageData );
                        frame_data = capture->rgb_frame->imageData;
                    }
                }
            }
        
            if( code == ICERR_OK )
            {
                cvInitImageHeader( &capture->frame,
                                   cvSize(vfmt.bmiHeader.biWidth,
                                          vfmt.bmiHeader.biHeight),
                                   IPL_DEPTH_8U, 3, IPL_ORIGIN_BL, 4 );
                capture->frame.imageData = capture->frame.imageDataOrigin = frame_data;
                return &capture->frame;
            }
        }
    }

    return 0;
}


static double icvGetPropertyCAM_VFW( CvCaptureCAM_VFW* capture, int property_id )
{
    switch( property_id )
    {
    case CV_CAP_PROP_FRAME_WIDTH:
        return capture->frame.width;
    case CV_CAP_PROP_FRAME_HEIGHT:
        return capture->frame.height;
    case CV_CAP_PROP_FOURCC:
        return capture->fourcc;
    }
    return 0;
}



static CvCaptureVTable captureCAM_VFW_vtable = 
{
    6,
    (CvCaptureCloseFunc)icvCloseCAM_VFW,
    (CvCaptureGrabFrameFunc)icvGrabFrameCAM_VFW,
    (CvCaptureRetrieveFrameFunc)icvRetrieveFrameCAM_VFW,
    (CvCaptureGetPropertyFunc)icvGetPropertyCAM_VFW,
    (CvCaptureSetPropertyFunc)0,
    (CvCaptureGetDescriptionFunc)0
};
   
/********************* Capturing video from camera via MIL *********************/
//#ifdef WIN32

/* Small patch to cope with automatically generated Makefiles */
#if !defined _MSC_VER || defined __ICL || defined CV_BATCH_MSVC
#undef USE_MIL 
#endif

#ifdef USE_MIL 
#include "mil.h" /* to build MIL-enabled version of HighGUI you
                    should have MIL installed */

struct 
{      
    MIL_ID MilApplication;
    int MilUser;
} g_Mil = {0,0}; //global structure for handling MIL application

typedef struct CvCaptureCAM_MIL
{
    CvCaptureVTable* vtable;
    MIL_ID 
    MilSystem,       /* System identifier.       */
    MilDisplay,      /* Display identifier.      */
    MilDigitizer,    /* Digitizer identifier.    */ 
    MilImage;        /* Image buffer identifier. */
    IplImage* rgb_frame;
}
CvCaptureCAM_MIL;

// Initialize camera input
static int icvOpenCAM_MIL( CvCaptureCAM_MIL* capture, int wIndex )
{
    if( g_Mil.MilApplication == M_NULL )
    {
        assert(g_Mil.MilUser == 0);
        MappAlloc(M_DEFAULT, &(g_Mil.MilApplication) );
        g_Mil.MilUser = 1;
    }
    else
    {
        assert(g_Mil.MilUser>0);
        g_Mil.MilUser++;
    }
    
    int dev_table[16] = { M_DEV0, M_DEV1, M_DEV2, M_DEV3,
                          M_DEV4, M_DEV5, M_DEV6, M_DEV7,
                          M_DEV8, M_DEV9, M_DEV10, M_DEV11,
                          M_DEV12, M_DEV13, M_DEV14, M_DEV15 };
    
    //set default window size
    int w = 320/*160*/;
    int h = 240/*120*/;
    
    
    if( (unsigned)wIndex < 116)
    {   
        wIndex -= 100;
    }
    if( wIndex < 0 ) wIndex = 0;

    for( ; wIndex < 16; wIndex++ ) 
    {
        MsysAlloc( M_SYSTEM_SETUP, //we use default system,
                                   //if this does not work 
                                   //try to define exact board 
                                   //e.g.M_SYSTEM_METEOR,M_SYSTEM_METEOR_II...
                   dev_table[wIndex], 
                   M_DEFAULT, 
                   &(capture->MilSystem) ); 

        if( capture->MilSystem != M_NULL )
            break;
    }
    if( capture->MilSystem != M_NULL )
    {
        MdigAlloc(capture->MilSystem,M_DEFAULT,
                  M_CAMERA_SETUP, //default. May be M_NTSC or other
                  M_DEFAULT,&(capture->MilDigitizer));
        
        capture->rgb_frame = cvCreateImage(cvSize(w,h), IPL_DEPTH_8U, 3 );
        MdigControl(capture->MilDigitizer, M_GRAB_SCALE,  1.0 / 2);
        
        /*below line enables getting image vertical orientation 
        consistent with VFW but it introduces some image corruption 
        on MeteorII, so we left the image as is*/  
        //MdigControl(capture->MilDigitizer, M_GRAB_DIRECTION_Y, M_REVERSE );

        capture->MilImage = MbufAllocColor(capture->MilSystem, 3, w, h,
                                                      8+M_UNSIGNED,
                                                      M_IMAGE + M_GRAB,                                                      
                                                      M_NULL);
    }
    
    return capture->MilSystem != M_NULL;
}

static  void icvCloseCAM_MIL( CvCaptureCAM_MIL* capture )
{
    if( capture->MilSystem != M_NULL )
    {
        MdigFree( capture->MilDigitizer );
        MbufFree( capture->MilImage );
        MsysFree( capture->MilSystem );
        cvReleaseImage(&capture->rgb_frame ); 
        capture->rgb_frame = 0;
        
        g_Mil.MilUser--;
        if(!g_Mil.MilUser)
            MappFree(g_Mil.MilApplication);

        capture->MilSystem = M_NULL;
        capture->MilDigitizer = M_NULL;
    }
}         

static int icvGrabFrameCAM_MIL( CvCaptureCAM_MIL* capture )
{
    if( capture->MilSystem )
    {
        MdigGrab(capture->MilDigitizer, capture->MilImage);
        return 1;
    }
    return 0;
}


static IplImage* icvRetrieveFrameCAM_MIL( CvCaptureCAM_MIL* capture )
{
    MbufGetColor(capture->MilImage, M_BGR24+M_PACKED, M_ALL_BAND, (void*)(capture->rgb_frame->imageData)); 
    //make image vertical orientation consistent with VFW
    //You can find some better way to do this
    capture->rgb_frame->origin = IPL_ORIGIN_BL;
    cvFlip(capture->rgb_frame,capture->rgb_frame,0);
    return capture->rgb_frame;
}

static double icvGetPropertyCAM_MIL( CvCaptureCAM_MIL* capture, int property_id )
{
    switch( property_id )
    {
    case CV_CAP_PROP_FRAME_WIDTH:
        if( capture->rgb_frame) return capture->rgb_frame->width;
    case CV_CAP_PROP_FRAME_HEIGHT:
        if( capture->rgb_frame) return capture->rgb_frame->height;
    } 
    return 0;
}
static CvCaptureVTable captureCAM_MIL_vtable = 
{
    6,
    (CvCaptureCloseFunc)icvCloseCAM_MIL,
    (CvCaptureGrabFrameFunc)icvGrabFrameCAM_MIL,
    (CvCaptureRetrieveFrameFunc)icvRetrieveFrameCAM_MIL,
    (CvCaptureGetPropertyFunc)icvGetPropertyCAM_MIL,
    (CvCaptureSetPropertyFunc)0,
    (CvCaptureGetDescriptionFunc)0
};

#endif //USE_MIL 
//#endif //WIN32

HIGHGUI_IMPL CvCapture* cvCaptureFromCAM( int index )
{
    if( index < 100 ) //try VFW 
    {
        CvCaptureCAM_VFW* capture = (CvCaptureCAM_VFW*)cvAlloc( sizeof(*capture));
        memset( capture, 0, sizeof(*capture));

        capture->vtable = &captureCAM_VFW_vtable;

        if( !icvOpenCAM_VFW( capture, index ))
            cvReleaseCapture( (CvCapture**)&capture );
        else return (CvCapture*)capture;
    }
#ifdef USE_MIL
    if( index >= 100 || index < 0 ) //try MIL 
    {
        CvCaptureCAM_MIL* capture = (CvCaptureCAM_MIL*)cvAlloc( sizeof(*capture));
        memset( capture, 0, sizeof(*capture));

        capture->vtable = &captureCAM_MIL_vtable;

        if( !icvOpenCAM_MIL( capture, index ))
            cvReleaseCapture( (CvCapture**)&capture );
        else return (CvCapture*)capture;
    }
#endif
    return 0;
}


/*************************** writing AVIs ******************************/

typedef struct CvAVIWriter
{
    PAVIFILE avifile;
    PAVISTREAM compressed;
    PAVISTREAM uncompressed;
    double fps;
    CvSize frameSize;
    IplImage* tempFrame;
    long pos;
    int fourcc;
}
CvAVIWriter;


static void icvCloseAVIWriter( CvAVIWriter* writer )
{
    if( writer )
    {
        if( writer->uncompressed )
            AVIStreamRelease( writer->uncompressed );
        if( writer->compressed )
            AVIStreamRelease( writer->compressed );
        if( writer->avifile )
            AVIFileRelease( writer->avifile );
        cvReleaseImage( &writer->tempFrame );
        memset( writer, 0, sizeof(*writer));
    }
}


static int icvInitAVIWriter( CvAVIWriter* writer, int fourcc,
                             double fps, CvSize frameSize )
{
    if( writer && writer->avifile )
    {
        AVICOMPRESSOPTIONS copts, *pcopts = &copts;
        AVISTREAMINFO aviinfo;

        assert( frameSize.width > 0 && frameSize.height > 0 );

        BITMAPINFOHEADER bmih = icvBitmapHeader( frameSize.width, frameSize.height, 24 );
        
        memset( &aviinfo, 0, sizeof(aviinfo));
        aviinfo.fccType = streamtypeVIDEO;
        aviinfo.fccHandler = 0;
        aviinfo.dwScale = 1;
        aviinfo.dwRate = cvRound(fps);
        aviinfo.rcFrame.top = aviinfo.rcFrame.left = 0;
        aviinfo.rcFrame.right = frameSize.width;
        aviinfo.rcFrame.bottom = frameSize.height;

        if( AVIFileCreateStream( writer->avifile,
            &writer->uncompressed, &aviinfo ) == AVIERR_OK )
        {
            copts.fccType = streamtypeVIDEO;
            copts.fccHandler = fourcc != -1 ? fourcc : 0; 
            copts.dwKeyFrameEvery = 1; 
            copts.dwQuality = 90; 
            copts.dwBytesPerSecond = 0; 
            copts.dwFlags = AVICOMPRESSF_VALID; 
            copts.lpFormat = &bmih; 
            copts.cbFormat = sizeof(bmih); 
            copts.lpParms = 0; 
            copts.cbParms = 0; 
            copts.dwInterleaveEvery = 0;

            if( fourcc != -1 ||
                AVISaveOptions( 0, 0, 1, &writer->uncompressed, &pcopts ) == TRUE )
            {
                if( AVIMakeCompressedStream( &writer->compressed,
                    writer->uncompressed, pcopts, 0 ) == AVIERR_OK &&
                    // check that the resolution was not changed
                    bmih.biWidth == frameSize.width &&
                    bmih.biHeight == frameSize.height &&
                    AVIStreamSetFormat( writer->compressed, 0, &bmih, sizeof(bmih)) == AVIERR_OK )
                    {
                        writer->fps = fps;
                        writer->fourcc = (int)copts.fccHandler;
                        writer->frameSize = frameSize;
                        writer->tempFrame = cvCreateImage( frameSize, 8, 3 );
                        return 1;
                    }
            }
        }
    }
    icvCloseAVIWriter( writer );
    return 0;
}


HIGHGUI_IMPL CvAVIWriter* cvCreateAVIWriter( const char* filename, int fourcc,
                                             double fps, CvSize frameSize )
{
    CvAVIWriter* writer = (CvAVIWriter*)cvAlloc( sizeof(CvAVIWriter));
    memset( writer, 0, sizeof(*writer));

    icvInitCapture_VFW();
    
    if( AVIFileOpen( &writer->avifile, filename, OF_CREATE | OF_WRITE, 0 ) == AVIERR_OK )
    {
        if( frameSize.width > 0 && frameSize.height > 0 )
        {
            if( !icvInitAVIWriter( writer, fourcc, fps, frameSize ))
                cvReleaseAVIWriter( &writer );
        }
        else if( fourcc == -1 )
        {
            icvCloseAVIWriter( writer );
        }
        else
        {
            /* postpone initialization until the first frame is written */
            writer->fourcc = fourcc;
            writer->fps = fps;
            writer->frameSize = frameSize;
        }
    }
    
    return writer;
}


HIGHGUI_IMPL int cvWriteToAVI( CvAVIWriter* writer, const IplImage* image )
{
    if( writer && (writer->compressed ||
        icvInitAVIWriter( writer, writer->fourcc, writer->fps, writer->frameSize )))
    {
        if( image->origin == 0 )
        {
            cvFlip( image, writer->tempFrame, 0 );
            image = (const IplImage*)writer->tempFrame;
        }
        if( AVIStreamWrite( writer->compressed, writer->pos++, 1, image->imageData,
                            image->imageSize, AVIIF_KEYFRAME, 0, 0 ) == AVIERR_OK )
        {
            return 1;
        }
    }
    return 0;
}


HIGHGUI_IMPL void cvReleaseAVIWriter( CvAVIWriter** writer )
{
    if( writer && *writer )
    {
        icvCloseAVIWriter( *writer );
        cvFree( (void**)writer );
    }
}

#else /* Linux version */

#ifdef HAVE_FFMPEG

typedef struct avi_entry
{
    int id;
    int flags;
    int offset;
    int size;
}
avi_entry;

typedef struct avi_entry_compact
{
    int flags;
    int offset;
}
avi_entry_compact;


typedef struct CvCaptureAVI_FFMPEG
{
    CvCaptureVTable* vtable;
    AVCodec *codec;
    AVCodecContext *avctx;
    AVPicture picture, rgb_picture;

    int codec_fourcc;
    int codec_sub_fourcc;

    CvSlice film_range;
    int pos;

    CvSize frame_size;
    double fps;
    FILE* file;
    int64 data_offset;
    int64 file_size;
    int64 offset;
    int stream_index;
    char* buffer;
    int buffer_size;
    avi_entry_compact* entries;
    IplImage frame;
}
CvCaptureAVI_FFMPEG;


#define fourcc(c1,c2,c3,c4) \
    (((c1)&255) + (((c2)&255)<<8) + (((c3)&255)<<16) + (((c4)&255)<<24))

static int read4( CvCaptureAVI_FFMPEG* capture )
{
    unsigned char buffer[4];
    if( fread( buffer, 1, 4, capture->file ) == 4 )
    {
        capture->offset += 4;
        return fourcc(buffer[0],buffer[1],buffer[2],buffer[3]);
    }
    else
        return -1;
}

static void skip( CvCaptureAVI_FFMPEG* capture, int bytes )
{
    if( bytes != 0 )
    {
        fseek( capture->file, bytes, SEEK_CUR );
        capture->offset += bytes;
    }
}

static void icvInitCapture_FFMPEG()
{
    static int isInitialized = 0;
    if( !isInitialized )
    {
        avcodec_init();
        avcodec_register_all();
        isInitialized = 1;
    }
}

static void icvCloseAVI_FFMPEG( CvCaptureAVI_FFMPEG* capture )
{
    cvFree( (void**)&(capture->entries) );
    
    if( capture->avctx )
    {
        avcodec_close( capture->avctx );
        cvFree( (void**)&capture->avctx );
        //capture->avctx = 0;
    }

    if( capture->file )
    {
        fclose( capture->file );
        capture->file = 0;
    }

    if( capture->rgb_picture.data[0] )
        cvFree( (void**)&capture->rgb_picture.data[0] );

    cvFree( (void**)&capture->buffer );

    capture->codec = 0;
    capture->pos = 0;
    capture->film_range.startIndex = capture->film_range.endIndex = 0;
    memset( &capture->frame, 0, sizeof(capture->frame));
}


static int icvOpenAVI_FFMPEG( CvCaptureAVI_FFMPEG* capture, const char* filename )
{
    int err, valid = 0;
    int val;
    icvInitCapture_FFMPEG();
    
    capture->file = fopen( filename, "rb" );
    if(!capture->file)
        return 0;

    capture->stream_index = -1;
    capture->file = fopen( filename, "rb" );
    if( !capture->file )
        goto exit_func;

    val = read4( capture );
    if( val != fourcc('R','I','F','F'))
        goto exit_func;

    capture->file_size = read4( capture );
    val = read4( capture );
    if( val != fourcc('A','V','I',' '))
        goto exit_func;

    for(;;)
    {
        int64 new_offset;
        val = read4( capture );

        if( val == -1 )
            break;

        new_offset = read4( capture );
        new_offset += capture->offset;

        if( val == fourcc('L','I','S','T'))
        {
            val = read4( capture );

            switch( val )
            {
            case fourcc('m','o','v','i'):
                capture->data_offset = capture->offset;
                break;
            case fourcc('h','d','r','l'):
            {
                int hdr_len, list_len, stream_index = -1;
                int i, streams;
                
                ///////////////  read AVI header  ////////////////
                val = read4( capture );
                hdr_len = read4( capture );
                
                capture->fps = (float)(1e6/read4(capture));
                read4( capture ); // bit-rate
                read4( capture ); // reserved
                read4( capture ); // flags
                capture->film_range.endIndex = read4( capture );
                read4( capture ); // initial frames
                streams = read4( capture ); // nstreams
                capture->buffer_size = read4( capture );
                skip( capture, hdr_len - 32 );

                for( i = 0; i < streams; i++ )
                {
                    /////////////// Streams' information ////////////////
                    if( read4(capture) != fourcc('L','I','S','T'))
                        goto exit_func;

                    list_len = read4(capture) - 4;
                    if( read4(capture) != fourcc('s','t','r','l'))
                        goto exit_func;

                    while( list_len > 0 )
                    {
                        val = read4(capture);
                        hdr_len = read4(capture);
                        list_len -= hdr_len + 8;

                        switch( val )
                        {
                        case fourcc('s','t','r','h'):
                            stream_index++;
                            val = read4(capture);
                            hdr_len -= 4;
                            if( val == fourcc('v','i','d','s') && capture->stream_index < 0 )
                            {
                                capture->stream_index = stream_index;
                                capture->codec_fourcc = read4( capture );
                                hdr_len -= 4;
                            }
                            break;
                        case fourcc('s','t','r','f'):
                            if( capture->stream_index == stream_index )
                            {
                                read4(capture); // header size
                                capture->frame_size.width = read4(capture);
                                capture->frame_size.height = read4(capture);
                                read4(capture); // planes & bit count
                                capture->codec_sub_fourcc = read4(capture);
                                hdr_len -= 20;
                            }
                            break;
                        }

                        skip( capture, hdr_len );
                    }
                }
            }
            break;
            }
        }
        else if( val == fourcc('i','d','x','1'))
        {
            int i, k = 0, size = (int)(new_offset - capture->offset);
            int fourcc1 = fourcc( '0', (capture->stream_index + '0'), 'd', 'b' );
            int fourcc2 = fourcc( '0', (capture->stream_index + '0'), 'd', 'c' );

            avi_entry* entries = (avi_entry*)cvAlloc( size );
            fread( entries, 1, size, capture->file );
            capture->offset = new_offset;
            size /= sizeof(entries[0]);

            capture->entries = (avi_entry_compact*)cvAlloc( capture->film_range.endIndex*
                                                            sizeof(capture->entries[0]) );
            for( i = 0; i < size; i++ )
            {
                if( entries[i].id == fourcc1 || entries[i].id == fourcc2 )
                {
                    int flags = entries[i].flags & 16;
                    int offset = entries[i].offset;
                    capture->entries[k].flags = flags;
                    capture->entries[k].offset = offset;
                    k++;
                }
            }
            cvFree( (void**)&entries );
        }
  
        skip( capture, (long)(new_offset - capture->offset) );
    }

    if( capture->codec_fourcc && capture->data_offset )
    {
        int fcc = capture->codec_sub_fourcc;
        CodecID id = fcc == fourcc('U','2','6','3') ? CODEC_ID_H263 :
                     fcc == fourcc('I','2','6','3') ? CODEC_ID_H263I :
                     fcc == fourcc('D', 'I', 'V', '3') ? CODEC_ID_MSMPEG4V3 :
                     fcc == fourcc('D', 'I', 'V', 'X') ? CODEC_ID_MPEG4 :
                     fcc == fourcc('D', 'X', '5', '0') ? CODEC_ID_MPEG4 :
                     fcc == fourcc('M', 'P', '4', '2') ? CODEC_ID_MSMPEG4V2 :
                     fcc == fourcc('M', 'J', 'P', 'G') ? CODEC_ID_MJPEG :
                     fcc == fourcc('P', 'I', 'M', '1') ? CODEC_ID_MPEG1VIDEO :
                     fcc == 0x50 ? CODEC_ID_MP2 :
                     fcc == 0x55 ? CODEC_ID_MP2 : CODEC_ID_NONE;

        capture->codec = avcodec_find_decoder( id );

        if( capture->codec )
        {
            capture->avctx = (AVCodecContext*)cvAlloc( sizeof(*capture->avctx));
            memset( capture->avctx, 0, sizeof(*capture->avctx));
            capture->avctx->width = capture->frame_size.width;
            capture->avctx->height = capture->frame_size.height;
            capture->avctx->codec_tag = capture->codec_sub_fourcc;
            capture->avctx->codec_type = CODEC_TYPE_VIDEO;
            capture->avctx->codec_id = id;
            
            err = avcodec_open( capture->avctx, capture->codec );
            if( err >= 0 )
            {
                capture->rgb_picture.data[0] = (uchar*)cvAlloc(
                                avpicture_get_size( PIX_FMT_RGB24,
                                capture->avctx->width, capture->avctx->height ));
                avpicture_fill( &capture->rgb_picture, capture->rgb_picture.data[0],
                                PIX_FMT_RGB24, capture->avctx->width, capture->avctx->height );

                cvInitImageHeader( &capture->frame, cvSize( capture->avctx->width,
                                   capture->avctx->height ), 8, 3, 0, 4 );
                cvSetData( &capture->frame, capture->rgb_picture.data[0],
                           capture->rgb_picture.linesize[0] );

                capture->pos = 0;
                capture->fps = capture->avctx->frame_rate*1e-4;
                capture->picture.data[0] = 0;
                fseek( capture->file, (long)capture->data_offset, SEEK_SET );
                capture->offset = capture->data_offset;
                valid = 1;
            }
        }
    }

exit_func:

    if( !valid )
        icvCloseAVI_FFMPEG( capture );

    return valid;
}


static int icvGrabFrameAVI_FFMPEG( CvCaptureAVI_FFMPEG* capture )
{
    if( capture && capture->avctx )
    {
        for(;;)
        {
            int val = read4( capture );
            if( val == -1 )
                return 0;

            if( isdigit((char)val) && isdigit((char)(val>>8)) &&
                ((char)(val>>16) == 'd' || (char)(val>>16) == 'w') &&
                ((char)(val>>24) == 'b' || (char)(val>>24) == 'c'))
            {
                int stream_index = ((char)(val>>8) - '0') + ((char)val - '0')*10;
                int size = (read4( capture ) + 1) & -2;

                if( stream_index == capture->stream_index )
                {
                    int got_picture = 0, ret;

                    if( !capture->buffer || capture->buffer_size < size )
                    {
                        cvFree( (void**)&capture->buffer );
                        if( capture->buffer_size < size )
                            capture->buffer_size = size;
                        capture->buffer = (char*)cvAlloc( capture->buffer_size );
                    }
                    fread( capture->buffer, 1, size, capture->file );
                    capture->offset += size;

                    ret = avcodec_decode_video( capture->avctx, &capture->picture,
                                                &got_picture, (uchar*)capture->buffer, size );
                    if( ret >= 0 && got_picture )
                    {
                        capture->pos++;
                        return 1;
                    }
                    return 0;
                }
                else
                    skip( capture, size );
            }
        }
    }

    return 0;
}


static const IplImage* icvRetrieveFrameAVI_FFMPEG( CvCaptureAVI_FFMPEG* capture )
{
    if( capture && capture->avctx && capture->picture.data[0] )
    {
        img_convert( &capture->rgb_picture, PIX_FMT_RGB24,
                     &capture->picture, capture->avctx->pix_fmt,
                     capture->avctx->width, capture->avctx->height );
        cvCvtColor( &capture->frame, &capture->frame, CV_RGB2BGR );
        return &capture->frame;
    }

    return 0;
}


static double icvGetPropertyAVI_FFMPEG( CvCaptureAVI_FFMPEG* capture, int property_id )
{
    switch( property_id )
    {
    case CV_CAP_PROP_POS_MSEC:
        return cvRound(capture->pos*1000./capture->fps);
        break;
    case CV_CAP_PROP_POS_FRAMES:
        return capture->pos;
    case CV_CAP_PROP_POS_AVI_RATIO:
        return ((double)capture->offset)/((double)capture->file_size);
    case CV_CAP_PROP_FRAME_WIDTH:
        return capture->frame.width;
    case CV_CAP_PROP_FRAME_HEIGHT:
        return capture->frame.height;
    case CV_CAP_PROP_FPS:
        return capture->fps;
    case CV_CAP_PROP_FOURCC:
        return capture->codec_sub_fourcc;
    }
    return 0;
}


static int icvSetPropertyAVI_FFMPEG( CvCaptureAVI_FFMPEG* capture,
                                     int property_id, double value )
{
    switch( property_id )
    {
    case CV_CAP_PROP_POS_MSEC:
    case CV_CAP_PROP_POS_FRAMES:
    case CV_CAP_PROP_POS_AVI_RATIO:
        if( capture->entries )
        {
            int pos;
            switch( property_id )
            {
            case CV_CAP_PROP_POS_MSEC:
                pos = cvRound(value*capture->fps*0.001);
                break;
            case CV_CAP_PROP_POS_AVI_RATIO:
                pos = cvRound(value*(capture->film_range.endIndex - 
                                     capture->film_range.startIndex) +
                              capture->film_range.startIndex);
                break;
            default:
                pos = cvRound(value);
            }
            pos -= 10; // to make sure the frame is updated properly
            if( pos < capture->film_range.startIndex )
                pos = capture->film_range.startIndex;
            if( pos > capture->film_range.endIndex )
                pos = capture->film_range.endIndex;
            for( ; pos > 0; pos-- )
                if( capture->entries[pos].flags & 16 )
                    break;
            capture->pos = pos;
            capture->offset = capture->entries[pos].offset + capture->data_offset;
            fseek( capture->file, (long)capture->offset, SEEK_SET );
        }
        break;
    default:
        return 0;
    }

    return 1;
}

static CvCaptureVTable captureAVI_FFMPEG_vtable = 
{
    6,
    (CvCaptureCloseFunc)icvCloseAVI_FFMPEG,
    (CvCaptureGrabFrameFunc)icvGrabFrameAVI_FFMPEG,
    (CvCaptureRetrieveFrameFunc)icvRetrieveFrameAVI_FFMPEG,
    (CvCaptureGetPropertyFunc)icvGetPropertyAVI_FFMPEG,
    (CvCaptureSetPropertyFunc)icvSetPropertyAVI_FFMPEG,
    (CvCaptureGetDescriptionFunc)0
};

HIGHGUI_IMPL CvCapture* cvCaptureFromAVI( const char* filename )
{
    CvCaptureAVI_FFMPEG* capture = (CvCaptureAVI_FFMPEG*)cvAlloc( sizeof(*capture));
    memset( capture, 0, sizeof(*capture));

    capture->vtable = &captureAVI_FFMPEG_vtable;

    if( !icvOpenAVI_FFMPEG( capture, filename ))
        cvReleaseCapture( (CvCapture**)&capture );

    return (CvCapture*)capture;
}

#else

HIGHGUI_IMPL CvCapture* cvCaptureFromAVI( const char* filename )
{
    return 0;
}

#endif

HIGHGUI_IMPL CvCapture* cvCaptureFromCAM( int /*index*/ )
{
    return 0;
}


HIGHGUI_IMPL CvAVIWriter* cvCreateAVIWriter( const char* /*filename*/, int /*fourcc*/,
                                             double /*fps*/, CvSize /*frameSize*/ )
{
    return 0;
}


HIGHGUI_IMPL int cvWriteToAVI( CvAVIWriter* /*writer*/, const IplImage* /*image*/ )
{
    return 0;
}


HIGHGUI_IMPL void cvReleaseAVIWriter( CvAVIWriter** /*writer*/ )
{
}


#endif


#if 0
int main( int argc, char** argv )
{
    CvCapture* capture;

    if( argc != 2 )
    {
        printf("Usage: %s <avifile>\n", argv[0] );
        return 0;
    }

    cvvNamedWindow( "window", 1 );
    capture = cvCaptureFromAVI( argv[1] );

    if( capture )
    {
        for( ;; )
        {
            IplImage* frame = cvQueryFrame( capture );
            if( frame )
                cvvShowImage( "window", frame );
            else
                break;
            int ch = cvvWaitKeyEx( 0, 10 );
            if( ch == '\x1b' )
                break;
        }

        cvReleaseCapture( &capture );
    }

    return 1;
}
#endif
