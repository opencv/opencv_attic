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

#include "cvtest.h"

static char* FuncName = "cvThreshold";
static char TestName[]    = "Thresholding";
static char TestClass[]   = "Algorithm";

static long  Len;
static float Thresh;

#define ATS_8U  3
#define ATS_32F 5

#define BINARY_8U      (CV_THRESH_BINARY     << 4) + ATS_8U
#define BINARY_INV_8U  (CV_THRESH_BINARY_INV << 4) + ATS_8U
#define TRUNC_8U       (CV_THRESH_TRUNC      << 4) + ATS_8U
#define TOZERO_8U      (CV_THRESH_TOZERO     << 4) + ATS_8U
#define TOZERO_INV_8U  (CV_THRESH_TOZERO_INV << 4) + ATS_8U

#define BINARY_8S      (CV_THRESH_BINARY     << 4) + ATS_8S
#define BINARY_INV_8S  (CV_THRESH_BINARY_INV << 4) + ATS_8S
#define TRUNC_8S       (CV_THRESH_TRUNC      << 4) + ATS_8S
#define TOZERO_8S      (CV_THRESH_TOZERO     << 4) + ATS_8S
#define TOZERO_INV_8S  (CV_THRESH_TOZERO_INV << 4) + ATS_8S

#define BINARY_32F     (CV_THRESH_BINARY     << 4) + ATS_32F
#define BINARY_INV_32F (CV_THRESH_BINARY_INV << 4) + ATS_32F
#define TRUNC_32F      (CV_THRESH_TRUNC      << 4) + ATS_32F
#define TOZERO_32F     (CV_THRESH_TOZERO     << 4) + ATS_32F
#define TOZERO_INV_32F (CV_THRESH_TOZERO_INV << 4) + ATS_32F


static int myThresh( uchar*        Src8u,
                     float*        Src32f,
                     float         Thresh,
                     float         Max,
                     int           Len,
                     CvThreshType  Type )
{
    int i;
    for( i = 0; i < Len; i++ )
    {
        switch( Type )
        {
        case CV_THRESH_BINARY:
            Src8u[i]  = (uchar)(Src8u[i]  > (uchar)Thresh ? Max : 0);
            Src32f[i] =        (Src32f[i] >        Thresh ? Max : 0);
            break;
        case CV_THRESH_BINARY_INV:
            Src8u[i]  = (uchar)(Src8u[i]  > (uchar)Thresh ? 0 : Max);
            Src32f[i] =        (Src32f[i] >        Thresh ? 0 : Max);
            break;
        case CV_THRESH_TRUNC:
            Src8u[i]  = (uchar)(Src8u[i]  > (uchar)Thresh ? Thresh : Src8u[i] );
            Src32f[i] =        (Src32f[i] >        Thresh ? Thresh : Src32f[i]);
            break;
        case CV_THRESH_TOZERO:
            Src8u[i]  = (uchar)(Src8u[i]  > (uchar)Thresh ? Src8u[i]  : 0);
            Src32f[i] =        (Src32f[i] >        Thresh ? Src32f[i] : 0);
            break;
        case CV_THRESH_TOZERO_INV:
            Src8u[i]  = (uchar)(Src8u[i]  > (uchar)Thresh ? 0 : Src8u[i] );
            Src32f[i] =        (Src32f[i] >        Thresh ? 0 : Src32f[i]);
            break;
        default:
            assert( 0 );
        }
    }
    return 0;
} /* myThresh */


static int myThreshR( IplImage*     Src8u,
                      IplImage*     Src32f,
                      float         Thresh,
                      float         Max,
                      CvThreshType  Type )
{
    char* s8u;
    char* s32f;
    int step8u, step32f;
    CvSize size;

    cvGetImageRawData(Src8u, (uchar**)&s8u, &step8u, &size);
    cvGetImageRawData(Src32f, (uchar**)&s32f, &step32f, 0);

    int y;
    for( y = 0; y < size.height; y++, s8u += step8u, s32f += step32f )
        myThresh( (uchar*)s8u, (float*)s32f, Thresh, Max, size.width, Type );
    return 0;
}


static int foaThreshold( void* prm )
{
    long          lParam  = (long)prm;
    int           Flavour = lParam & 0xf;
    CvThreshType  Type    = (CvThreshType)((lParam >> 4) & 0xf);

    IplImage* Src8uR;
    IplImage* Src32fR;
    IplImage* Src8uControlR;
    IplImage* Src32fControlR;

    int    height;
    int    width;

    long   Errors = 0;

    float  ThreshMax;
    float  ThreshMin;
    float  _Thresh;

    int    i;

    static int  read_param = 0;

    /* Initialization global parameters */
    if( !read_param )
    {
        read_param = 1;
        trslRead( &Len,    "106", "Size of sourse array" );
        trssRead( &Thresh, "125", "Threshold value" );
    }

    width = Len;
    height = Len / 2 + 1;
    
    Src8uR         = cvCreateImage(cvSize(width, height), IPL_DEPTH_8U, 1);
    Src32fR        = cvCreateImage(cvSize(width, height), IPL_DEPTH_32F, 1);
    Src8uControlR  = cvCreateImage(cvSize(width, height), IPL_DEPTH_8U, 1);
    Src32fControlR = cvCreateImage(cvSize(width, height), IPL_DEPTH_32F, 1);
    
    assert( Src8uR );
    assert( Src32fR );
    assert( Src8uControlR );
    assert( Src32fControlR );

    ThreshMin = (Flavour == ATS_8U ? MAX( -Thresh, 0 ) : -Thresh);
    ThreshMax = Thresh;

    for( _Thresh = ThreshMin; _Thresh <= ThreshMax; _Thresh++ )
    {
        CvSize size = cvSize( width, height );
        
        for( i = 0; i < height; i++ )
        {
            ats1bInitRandom( 0, 255, (uchar*)Src8uControlR->imageData + i * Src8uControlR->widthStep, Len );
            ats1flInitRandom( -255, 255, (float*)(Src32fControlR->imageData + i * Src32fControlR->widthStep), Len );
        }

        /* Run CVL function comparing results */
        switch( Flavour )
        {
        case ATS_8U:
            cvThreshold( Src8uControlR, Src8uR, _Thresh, 250, Type );
            /* Run my function */
            myThreshR( Src8uControlR,
                       Src32fControlR,
                       _Thresh, 250, Type );
            Errors += atsCompare2Db( (uchar*)Src8uR->imageData, (uchar*)Src8uControlR->imageData,
                                     size, Src8uR->widthStep, 0 );
            break;
        case ATS_32F:
            cvThreshold( Src32fControlR, Src32fR, _Thresh, 250, Type );
            /* Run my function */
            myThreshR( Src8uControlR,
                       Src32fControlR,
                       _Thresh, 250, Type );
            Errors += atsCompare2Dfl( (float*)Src32fR->imageData, (float*)Src32fControlR->imageData,
                                      size, Src32fR->widthStep, 0 );
            break;
        default:
            assert( 0 );
        }
    }
    cvReleaseImage( &Src8uR );
    cvReleaseImage( &Src32fR );
    cvReleaseImage( &Src8uControlR );
    cvReleaseImage( &Src32fControlR );

    return Errors == 0 ? TRS_OK : trsResult( TRS_FAIL, "Fixed %d errors", Errors );
}

void InitAThreshold()
{
    trsRegArg( FuncName,  TestName, TestClass, foaThreshold, BINARY_8U    );
    trsRegArg( FuncName,  TestName, TestClass, foaThreshold, BINARY_INV_8U);
    trsRegArg( FuncName,  TestName, TestClass, foaThreshold, TRUNC_8U     );
    trsRegArg( FuncName,  TestName, TestClass, foaThreshold, TOZERO_8U    );
    trsRegArg( FuncName,  TestName, TestClass, foaThreshold, TOZERO_INV_8U);

    trsRegArg( FuncName, TestName, TestClass, foaThreshold, BINARY_32F   );
    trsRegArg( FuncName, TestName, TestClass, foaThreshold, BINARY_INV_32F);
    trsRegArg( FuncName, TestName, TestClass, foaThreshold, TRUNC_32F    );
    trsRegArg( FuncName, TestName, TestClass, foaThreshold, TOZERO_32F   );
    trsRegArg( FuncName, TestName, TestClass, foaThreshold, TOZERO_INV_32F);
} /* InitAThresholdC1S */

/* End of file. */
