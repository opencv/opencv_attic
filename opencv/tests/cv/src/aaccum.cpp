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

/* Testing parameters */
static char* FuncName[] = 
{
    "cvAcc",
    "cvSquareAcc",
    "cvMultiplyAcc",
    "cvRunningAvg"
};
static char* TestName[]    = 
{
    "Linear Accumulating",
    "Accumulating of Squares",
    "Accumulating of Products",
    "Running Average"
};
static char TestClass[]   = "Algorithm";


static long lImageWidth;
static long lImageHeight;

#define EPSILON 0.01

static int fcaLinAcc( void )
{
    /* Some Variables */
    
    AtsRandState      state;
    IplImage*         pSrc8u;
    IplImage*         pSrc8s;
    IplImage*         pSrc32f;
    
    IplImage*         pDst;
    IplImage*         pTest;
    IplImage*         pTemp;
    
    
    double Error;
    
    static int  read_param = 0;
    
    /* Initialization global parameters */
    if( !read_param )
    {
        read_param = 1;
        /* Reading test-parameters */
        trslRead( &lImageHeight, "256", "Image Height" );
        trslRead( &lImageWidth, "256", "Image width" );
    }
    atsRandInit(&state,0,255,127);
    pSrc8u         = cvCreateImage(cvSize(lImageWidth, lImageHeight), IPL_DEPTH_8U, 3);
    pSrc8s         = cvCreateImage(cvSize(lImageWidth, lImageHeight), IPL_DEPTH_8S, 3);
    pSrc32f        = cvCreateImage(cvSize(lImageWidth, lImageHeight), IPL_DEPTH_32F, 3);
    pDst           = cvCreateImage(cvSize(lImageWidth, lImageHeight), IPL_DEPTH_32F, 3);
    pTemp          = cvCreateImage(cvSize(lImageWidth, lImageHeight), IPL_DEPTH_32F, 3);	
    atsFillRandomImageEx(pSrc8u, &state );
    atsFillRandomImageEx(pSrc8s, &state );
    atsFillRandomImageEx(pSrc32f, &state );
    atsFillRandomImageEx(pDst, &state );
    pTest = cvCloneImage(pDst);
    
    
    cvAcc(pSrc8u,pTest);
    atsConvert(pSrc8u,pTemp);
    cvAdd(pDst,pTemp,pDst);
    cvAcc(pSrc8s,pTest);
    atsConvert(pSrc8s,pTemp);
    cvAdd(pDst,pTemp,pDst);
    cvAcc(pSrc32f,pTest);
    cvAdd(pDst,pSrc32f,pDst);
    Error = (long)cvNorm(pTest,pDst,CV_C);
    
    trsWrite(ATS_SUM, "\nAccuracy   %e\n",EPSILON);
    /*************************************************************************************/
    /*    check 8u                                                                       */
    /*************************************************************************************/
    
    
    cvReleaseImage( &pSrc8u );
    cvReleaseImage( &pSrc8s );
    cvReleaseImage( &pSrc32f );
    cvReleaseImage( &pDst );
    cvReleaseImage( &pTest);
    cvReleaseImage( &pTemp );
    
    if( Error < EPSILON  ) return trsResult( TRS_OK, "No errors fixed for this text" );
    else return trsResult( TRS_FAIL,"Total fixed %d errors", 1);
} /* fmaAcc */

static int fcaSqrAcc( void )
{
    /* Some Variables */
    
    AtsRandState      state;
    IplImage*         pSrc8u;
    IplImage*         pSrc8s;
    IplImage*         pSrc32f;
    
    IplImage*         pDst;
    IplImage*         pTest;
    IplImage*         pTemp;
    
    
    double Error;
    
    static int  read_param = 0;
    
    /* Initialization global parameters */
    if( !read_param )
    {
        read_param = 1;
        /* Reading test-parameters */
        trslRead( &lImageHeight, "177", "Image Height" );
        trslRead( &lImageWidth, "177", "Image width" );
    }
    atsRandInit(&state,0,255,127);
    pSrc8u         = cvCreateImage(cvSize(lImageWidth, lImageHeight), IPL_DEPTH_8U, 1);
    pSrc8s         = cvCreateImage(cvSize(lImageWidth, lImageHeight), IPL_DEPTH_8S, 1);
    pSrc32f        = cvCreateImage(cvSize(lImageWidth, lImageHeight), IPL_DEPTH_32F, 1);
    pDst           = cvCreateImage(cvSize(lImageWidth, lImageHeight), IPL_DEPTH_32F, 1);
    pTemp          = cvCreateImage(cvSize(lImageWidth, lImageHeight), IPL_DEPTH_32F, 1);	
    atsFillRandomImageEx(pSrc8u, &state );
    atsFillRandomImageEx(pSrc8s, &state );
    atsFillRandomImageEx(pSrc32f, &state );
    atsFillRandomImageEx(pDst, &state );
    pTest = cvCloneImage(pDst);
    
    
    cvSquareAcc(pSrc8u,pTest);
    atsConvert(pSrc8u,pTemp);
    cvMul(pTemp,pTemp,pTemp);
    cvAdd(pDst,pTemp,pDst);
    cvSquareAcc(pSrc8s,pTest);
    atsConvert(pSrc8s,pTemp);
    cvMul(pTemp,pTemp,pTemp);
    cvAdd(pDst,pTemp,pDst);
    cvSquareAcc(pSrc32f,pTest);
    cvMul(pSrc32f,pSrc32f,pSrc32f);
    cvAdd(pDst,pSrc32f,pDst);
    Error = (long)cvNorm(pTest,pDst,CV_C);
    
    trsWrite(ATS_SUM, "\nAccuracy   %e\n",EPSILON);
    /*************************************************************************************/
    /*    check 8u                                                                       */
    /*************************************************************************************/
    
    
    cvReleaseImage( &pSrc8u );
    cvReleaseImage( &pSrc8s );
    cvReleaseImage( &pSrc32f );
    cvReleaseImage( &pDst );
    cvReleaseImage( &pTest );
    cvReleaseImage( &pTemp );
    
    if( Error < EPSILON  ) return trsResult( TRS_OK, "No errors fixed for this text" );
    else return trsResult( TRS_FAIL,"Total fixed %d errors", 1);
} /* fmaAcc */

static int fcaMultAcc( void )
{
    /* Some Variables */
    
    AtsRandState      state;
    IplImage*         pSrcA8u;
    IplImage*         pSrcA8s;
    IplImage*         pSrcA32f;
    IplImage*         pSrcB8u;
    IplImage*         pSrcB8s;
    IplImage*         pSrcB32f;
    
    IplImage*         pDst;
    IplImage*         pTest;
    IplImage*         pTempA;
    IplImage*         pTempB;
    
    
    double Error;
    
    static int  read_param = 0;
    
    /* Initialization global parameters */
    if( !read_param )
    {
        read_param = 1;
        /* Reading test-parameters */
        trslRead( &lImageHeight, "177", "Image Height" );
        trslRead( &lImageWidth, "177", "Image width" );
    }
    atsRandInit(&state,0,255,127);
    pSrcA8u         = cvCreateImage(cvSize(lImageWidth, lImageHeight), IPL_DEPTH_8U, 1);
    pSrcA8s         = cvCreateImage(cvSize(lImageWidth, lImageHeight), IPL_DEPTH_8S, 1);
    pSrcA32f        = cvCreateImage(cvSize(lImageWidth, lImageHeight), IPL_DEPTH_32F, 1);
    pSrcB8u         = cvCreateImage(cvSize(lImageWidth, lImageHeight), IPL_DEPTH_8U, 1);
    pSrcB8s         = cvCreateImage(cvSize(lImageWidth, lImageHeight), IPL_DEPTH_8S, 1);
    pSrcB32f        = cvCreateImage(cvSize(lImageWidth, lImageHeight), IPL_DEPTH_32F, 1);
    pDst            = cvCreateImage(cvSize(lImageWidth, lImageHeight), IPL_DEPTH_32F, 1);
    pTempA          = cvCreateImage(cvSize(lImageWidth, lImageHeight), IPL_DEPTH_32F, 1);
    pTempB          = cvCreateImage(cvSize(lImageWidth, lImageHeight), IPL_DEPTH_32F, 1);
    atsFillRandomImageEx(pSrcA8u, &state );
    atsFillRandomImageEx(pSrcA8s, &state );
    atsFillRandomImageEx(pSrcA32f, &state );
    atsFillRandomImageEx(pSrcB8u, &state );
    atsFillRandomImageEx(pSrcB8s, &state );
    atsFillRandomImageEx(pSrcB32f, &state );
    atsFillRandomImageEx(pDst, &state );
    pTest = cvCloneImage(pDst);
    
    
    cvMultiplyAcc(pSrcA8u,pSrcB8u,pTest);
    atsConvert(pSrcA8u,pTempA);
    atsConvert(pSrcB8u,pTempB);
    cvMul(pTempA,pTempB,pTempA);
    cvAdd(pDst,pTempA,pDst);
    //Error = (long)cvNorm(pTest,pDst,CV_C);
    cvMultiplyAcc(pSrcA8s,pSrcB8s,pTest);
    atsConvert(pSrcA8s,pTempA);
    atsConvert(pSrcB8s,pTempB);
    cvMul(pTempA,pTempB,pTempA);
    cvAdd(pDst,pTempA,pDst);
    //Error = (long)cvNorm(pTest,pDst,CV_C);
    cvMultiplyAcc(pSrcA32f,pSrcB32f,pTest);
    cvMul(pSrcA32f,pSrcB32f,pSrcA32f);
    cvAdd(pDst,pSrcA32f,pDst);
    Error = cvNorm(pTest,pDst,CV_C);
    
    trsWrite(ATS_SUM, "\nAccuracy   %e\n",Error);
    /*************************************************************************************/
    /*    check 8u                                                                       */
    /*************************************************************************************/
    
    
    cvReleaseImage( &pSrcA8u );
    cvReleaseImage( &pSrcA8s );
    cvReleaseImage( &pSrcA32f );
    cvReleaseImage( &pSrcB8u );
    cvReleaseImage( &pSrcB8s );
    cvReleaseImage( &pSrcB32f );
    cvReleaseImage( &pDst );
    cvReleaseImage( &pTest);
    cvReleaseImage( &pTempA );
    cvReleaseImage( &pTempB );
    
    if( Error < EPSILON  ) return trsResult( TRS_OK, "No errors fixed for this text" );
    else return trsResult( TRS_FAIL,"Total fixed %d errors", 1);
} /* fmaAcc */

static int fcaRunAvg( void )
{
    /* Some Variables */

    AtsRandState      state;
    IplImage*         pSrc8u;
    IplImage*         pSrc8s;
    IplImage*         pSrc32f;
    
    IplImage*         pDst;
    IplImage*         pTest;
    IplImage*         pTemp;
    
    float alpha =0.05f; 
    double Error;
    
    static int  read_param = 0;
    
    /* Initialization global parameters */
    if( !read_param )
    {
        read_param = 1;
        /* Reading test-parameters */
        trslRead( &lImageHeight, "177", "Image Height" );
        trslRead( &lImageWidth, "177", "Image width" );
    }
    atsRandInit(&state,0,255,127);
    pSrc8u         = cvCreateImage(cvSize(lImageWidth, lImageHeight), IPL_DEPTH_8U, 1);
    pSrc8s         = cvCreateImage(cvSize(lImageWidth, lImageHeight), IPL_DEPTH_8S, 1);
    pSrc32f        = cvCreateImage(cvSize(lImageWidth, lImageHeight), IPL_DEPTH_32F, 1);
    pDst           = cvCreateImage(cvSize(lImageWidth, lImageHeight), IPL_DEPTH_32F, 1);
    pTemp          = cvCreateImage(cvSize(lImageWidth, lImageHeight), IPL_DEPTH_32F, 1);	
    atsFillRandomImageEx(pSrc8u, &state );
    atsFillRandomImageEx(pSrc8s, &state );
    atsFillRandomImageEx(pSrc32f, &state );
    atsFillRandomImageEx(pDst, &state );
    pTest = cvCloneImage(pDst);
    
    cvRunningAvg(pSrc8u,pTest,alpha);
    atsConvert(pSrc8u,pTemp);
    cvScale(pDst,pDst,(1.f-alpha));
    cvScale(pTemp,pTemp,alpha);
    cvAdd(pDst,pTemp,pDst);
    Error = (long)cvNorm(pTest,pDst,CV_C);
    cvRunningAvg(pSrc8s,pTest,alpha);
    atsConvert(pSrc8s,pTemp);
    cvScale(pDst,pDst,(1.f-alpha));
    cvScale(pTemp,pTemp,alpha);
    cvAdd(pDst,pTemp,pDst);
    Error = (long)cvNorm(pTest,pDst,CV_C);
    cvRunningAvg(pSrc32f,pTest,alpha);
    cvScale(pDst,pDst,(1.f-alpha));
    cvScale(pSrc32f,pTemp,alpha);
    cvAdd(pDst,pTemp,pDst);
    Error = (long)cvNorm(pTest,pDst,CV_C);
    
    trsWrite(ATS_SUM, "\nAccuracy   %e\n",EPSILON);
    /*************************************************************************************/
    /*    check 8u                                                                       */
    /*************************************************************************************/
   

    cvReleaseImage( &pSrc8u );
    cvReleaseImage( &pSrc8s );
    cvReleaseImage( &pSrc32f );
    cvReleaseImage( &pDst );
    cvReleaseImage( &pTest);
    cvReleaseImage( &pTemp );

    if( Error < EPSILON  ) return trsResult( TRS_OK, "No errors fixed for this text" );
    else return trsResult( TRS_FAIL,"Total fixed %d errors", 1);
} /* fmaAcc */





void InitAAcc( void )
{
    /* Registering test function */
    trsReg( FuncName[0], TestName[0], TestClass, fcaLinAcc );
    trsReg( FuncName[1], TestName[1], TestClass, fcaSqrAcc );
    trsReg( FuncName[2], TestName[2], TestClass, fcaMultAcc );
    trsReg( FuncName[3], TestName[3], TestClass, fcaRunAvg );
} /* InitAAcc */


/* End of file. */
