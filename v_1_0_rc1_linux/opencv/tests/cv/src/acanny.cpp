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

#define DEPTH_8U 0

/* Testing parameters */
static char test_desc[] = "Canny Edge Detector";
static int lImageWidth;
static int lImageHeight;
static int  Sobord;
static float flLow, flHigh;
static char* func_name[] = 
{
    "cvCanny",
    "cvCannyBegin, cvCannyEnd"
};

static int data_type = 0;

static int fmaCanny( void* arg )
{
    int Components = 0;

    /* source image */
    IplImage* src8u;
	IplImage* dst8u;
	IplImage* test8u;
	AtsRandState state;

    CvSize roi;
    long lErrors = 0;

    static int  read_param = 0;

    /* Initialization global parameters */
    if( !read_param )
    {
        read_param = 1;

        /* Read test-parameters */
        trsiRead( &lImageWidth, "5", "width of the image" );
        trsiRead( &lImageHeight, "5", "height of the image" );
        trsiRead( &Sobord,"3","Size of Sobel operator");
        trssRead( &flLow, "40", "low threshold" );
        trssRead( &flHigh, "100", "high threshold" );

    }

    if( (int)(size_t)arg != data_type && (int)data_type != 2 ) return TRS_UNDEF;

    roi.height = lImageHeight;
    roi.width  = lImageWidth;

    src8u= cvCreateImage(cvSize(lImageWidth, lImageHeight), IPL_DEPTH_8U, 1);
	dst8u= cvCreateImage(cvSize(lImageWidth, lImageHeight), IPL_DEPTH_8U, 1);
	test8u = cvCreateImage(cvSize(lImageWidth, lImageHeight), IPL_DEPTH_8U, 1);
	
	atsRandInit(&state,0,255,127);
	atsFillRandomImageEx(src8u, &state );
    
    /* run alternative canny function */
    atsCannyStatistics( (uchar*)src8u->imageData, roi,src8u->widthStep, (uchar*)dst8u->imageData,dst8u->widthStep, Sobord, flLow,
                        flHigh, 0,0,0,0,&Components,0 );
    trsWrite( ATS_CON, " %d connected components were found" , Components );
    /* Run CVL function to check it */

    switch ( (int)(size_t)arg )
    {
    case DEPTH_8U:
        {
            cvCanny(src8u,test8u,flLow,flHigh,Sobord);
            break;
        }
    }
    /*
    // clear boundaries
    for( i = 0; i < src8u->height; i++ )
    {
        dst8u->imageData[i*dst8u->widthStep] =
        dst8u->imageData[i*dst8u->widthStep + dst8u->width-1] = 0;

        test8u->imageData[i*test8u->widthStep] =
        test8u->imageData[i*test8u->widthStep + test8u->width-1] = 0;
    }

    for( i = 0; i < src8u->width; i++ )
    {
        dst8u->imageData[i] =
        dst8u->imageData[(src8u->height-1)*dst8u->widthStep + i] = 0;

        test8u->imageData[i] =
        test8u->imageData[(src8u->height-1)*test8u->widthStep + i] = 0;
    }*/
    lErrors = (long)cvNorm(dst8u,test8u,CV_C);

    cvReleaseImage( &src8u );
    cvReleaseImage( &dst8u );
    cvReleaseImage( &test8u );

   if( lErrors == 0 ) return trsResult( TRS_OK, "No errors fixed for this text" );
    else return trsResult( TRS_FAIL, "Bad Accuracy %d", lErrors );

}



void InitACanny()
{
    /* Register test function */
    trsRegArg( func_name[0], test_desc, atsAlgoClass, fmaCanny, DEPTH_8U );
    
} /* InitACanny */
