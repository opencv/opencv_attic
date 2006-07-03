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

#define Pi 3.14159265358979f

/* Testing parameters */
static char* FuncName = "cvCamShift";

static char TestName[]    = "Calculating CamShift";
static char TestClass[]   = "Algorithm";

static int height;
static int width;
static int Length;
static int Width;
static int iter;
static float epsilon;
static int steps;

#define ATS_8U  0
#define ATS_32F 1

#define EPS_8U      (ATS_8U << 4) | CV_TERMCRIT_EPS
#define ITER_8U     (ATS_8U << 4) | CV_TERMCRIT_ITER
#define EPS_ITER_8U (ATS_8U << 4) | CV_TERMCRIT_ITER | CV_TERMCRIT_EPS

#define EPS_32F      (ATS_32F << 4) | CV_TERMCRIT_EPS
#define ITER_32F     (ATS_32F << 4) | CV_TERMCRIT_ITER
#define EPS_ITER_32F (ATS_32F << 4) | CV_TERMCRIT_ITER | CV_TERMCRIT_EPS

static int foaCamShiftC1R( void* prm )
{
    /* Some variables */
    long       lParam  = (long)(size_t)prm;
    int        Flvr = (lParam >> 4) & 0xf;
    int        depth = (Flvr == ATS_8U ? IPL_DEPTH_8U : IPL_DEPTH_32F);
    int        Type = lParam & 0xf;
    int        Errors = 0;

    CvTermCriteria criteria;
    CvRect     Window;
    CvSize     roi;

    IplImage*  src;

    float      alpha = 0;
    int        i;
    int        x, y;

    float      destOrientation = 0;
    float      destLen = 0;
    float      destWidth = 0;
    float      destArea = 0;
    int        destIters = 0;

    static int  read_param = 0;

    /* Initialization global parameters */
    if( !read_param )
    {
        read_param = 1;
        trsiRead( &height, "512", "source array length" );
        trsiRead( &width, "512", "source array width" );
        trsiRead( &Length, "68", "oval length" );
        trsiRead( &Width, "15", "oval width" );
        trsiRead( &iter, "10", "iterations" );
        trsiRead( &steps, "10", "steps" );
        trssRead( &epsilon, "1", "epsilon" );
    }

    /* Initilization */
    Window.x = width / 4;
    Window.y = height / 4;
    Window.width = width / 2;
    Window.height = height / 2;

    roi.width = width;
    roi.height = height;

    criteria.type = Type;
    criteria.epsilon = epsilon;
    criteria.max_iter = iter;

    /* Allocating source arrays; */
    src = cvCreateImage(roi, depth, 1);
    assert(src);

    for( alpha = -Pi / 2; alpha < Pi / 2; alpha += Pi / steps )
    {
        x = (int)(width  / 2 + width / 8 * cos(alpha));
        y = (int)(height / 2 + height / 8 * sin(alpha));

        switch( Flvr )
        {
        case ATS_8U:
            atsbInitEllipse( (uchar*)src->imageData,
                             roi.width,
                             roi.height,
                             src->widthStep,
                             x,
                             y,
                             Length,
                             Width,
                             alpha,
                             10 );
            break;
        case ATS_32F:
            atsfInitEllipse( (float*)src->imageData,
                             roi.width,
                             roi.height,
                             src->widthStep,
                             x,
                             y,
                             Length,
                             Width,
                             alpha,
                             10 );
            break;
        } /* switch( Flvr ) */

        for( i = 0; i < steps; i++ )
        {
            CvConnectedComp comp;
            CvBox2D box;
            destIters = cvCamShift( src, Window, criteria, &comp, &box );
            Window = comp.rect;
            destArea = (float) comp.area;
            destOrientation = box.angle*Pi/180;
            destLen = box.size.height;
            destWidth = box.size.width;
        }
        
        /* Checking results */
        /* Checking orientation */
        if( fabs( alpha - destOrientation ) > 0.01 &&
            fabs( alpha + Pi - destOrientation ) > 0.01 )
        {
            Errors++;
            trsWrite( ATS_LST,
                      "orientation: act: %f,  exp: %f\n",
                      destOrientation,
                      alpha );
        }
        /* Checking length */
        if( fabs( destLen - Length * 2 ) > epsilon )
        {
            Errors++;
            trsWrite( ATS_LST,
                      "length: act: %f,  exp: %d\n",
                      destLen,
                      Length );
        }
        /* Checking width */
        if( fabs( destWidth - Width * 2 ) > epsilon )
        {
            Errors++;
            trsWrite( ATS_LST,
                      "width: act: %f,  exp: %d\n",
                      destWidth,
                      Width );
        }
    }

    cvReleaseImage(&src);

    return Errors == 0 ? TRS_OK : trsResult( TRS_FAIL, "Fixed %d errors", Errors );

} /* foaCamShiftC1R */


void InitACamShift()
{
    trsRegArg( FuncName, TestName, TestClass, foaCamShiftC1R, EPS_8U );
    trsRegArg( FuncName, TestName, TestClass, foaCamShiftC1R, ITER_8U );
    trsRegArg( FuncName, TestName, TestClass, foaCamShiftC1R, EPS_ITER_8U );

    trsRegArg( FuncName, TestName, TestClass, foaCamShiftC1R, EPS_32F );
    trsRegArg( FuncName, TestName, TestClass, foaCamShiftC1R, ITER_32F );
    trsRegArg( FuncName, TestName, TestClass, foaCamShiftC1R, EPS_ITER_32F );
} /* InitACamShiftC1R */


/* End of file. */
