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

static char *cTestName[] = 
{
    "Calculating the Contour's moments",
    "Calculating the Contour's area",
    "Calculating the Contour's section area"
};

static char cTestClass[] = "Algorithm";

static char *cFuncName[] = 
{
    "cvContourMoments",
    "cvContourArea",
    "cvContourSecArea"
};

static int aContourMoments(void *arg)
{

    CvSeqBlock contour_blk;
    CvContour contour_h;
    CvMoments r_moments,moments;
    CvMoments mState;
    CvPoint *cp;
/*    CvPoint cp[] ={0,0, 5,5, 5,0, 0,5};*/
/*    CvPoint cp[] ={5,0, 10,5, 5,10, 0,5};*/
/*	CvPoint cp[] ={0,0, 5,5, 5,0, 10,5, 10,0, 15,5, 15,0};*/
    int algr = (int)(size_t)arg;
    int width = 128;
    int height = 128;
    int kp = 5;
    int nPoints2 = 20;
    int color1 = 0, color2 = 0, bkcolor = 255;
    int n1 = 0, n2 = 19;
    int seq_type = 0;
    int fi = 0;
    int a2 = 20;
    int b2 = 25,xc,yc;
    double pi = 3.1415926;
    double eps = 1.e-5, eps_rez = 1.0;
    double error_test = 0., error_area = 0., error_sec_area = 0.;
    double area = 0, sec_area = 0;
	double H;
    int l,i,i1;
    IplImage *Iplimage;
    CvSize size;
    int xmin,ymin,xmax,ymax;
    int code = TRS_OK;

/* read tests params */
    n1 = 0;
    n2 = nPoints2 -1;
    if(!trsiRead( &width, "128", "image width" ))
        return TRS_UNDEF;
    if(!trsiRead( &height, "128", "image height" ))
        return TRS_UNDEF;

    if (!trsiRead(&a2,"20","first radius of the second elipse"))
        return TRS_UNDEF;
    if (!trsiRead(&b2,"25","second radius of the second elipse"))
        return TRS_UNDEF;
    if (!trsiRead(&fi,"0","second radius of the second elipse"))
        return TRS_UNDEF;

    if (!trsiRead(&nPoints2,"20","number of contour's points"))
        return TRS_UNDEF;
    if(algr == 2)
    {
        if (!trsiRead(&n1,"0","number of the first point contour's point"))
            return TRS_UNDEF;
        if (!trsiRead(&n2,"19","number of the last point contour's point"))
            return TRS_UNDEF;
        if(n2 >= nPoints2) n2 = nPoints2 - 1;
    }
/*  initialized image*/
    l = width*height;

    cp = (CvPoint*) trsmAlloc(nPoints2*sizeof(CvPoint));

    xc = (int)( width/3.);
    yc = (int)( height/4.);
    kp = nPoints2;
    size.width = width;
    size.height = height;
    xmin = width;
    ymin = height;
    xmax = 0;
    ymax = 0;

    for(i=0;i<nPoints2;i++)
    {
        cp[i].x = (int)(a2*cos(2*pi*i/nPoints2)*cos(2*pi*fi/360.))-
        (int)(b2*sin(2*pi*i/nPoints2)*sin(2*pi*fi/360.))+xc;
        if(xmin> cp[i].x) xmin = cp[i].x;
        if(xmax< cp[i].x) xmax = cp[i].x;
        cp[i].y = (int)(a2*cos(2*pi*i/nPoints2)*sin(2*pi*fi/360.))+
                    (int)(b2*sin(2*pi*i/nPoints2)*cos(2*pi*fi/360.))+yc;
        if(ymin> cp[i].y) ymin = cp[i].y;
        if(ymax< cp[i].y) ymax = cp[i].y;
    }

    if(xmax>width||xmin<0||ymax>height||ymin<0) return TRS_FAIL;
    
/*  IPL image moment calculation  */
/*  create image  */

    Iplimage = cvCreateImage( size, IPL_DEPTH_8U, 1 );

    memset(Iplimage->imageData,bkcolor,l);
//    CVL_CHECK(ippiFillPoly8uC1R((uchar*)Iplimage->imageData, Iplimage->widthStep, size, cp, kp, color1));

    cvFillPoly(Iplimage, &cp, &kp, 1, cvScalar(color1));

    for(i=0;i<kp;i++)
    {
         if(i<kp-1) i1 = i+1;
         else i1 = 0;
//         CVL_CHECK(ippiLine8uC1R((uchar*)Iplimage->imageData, Iplimage->widthStep, size, cp[i], cp[i1],
//                                 color2));
         cvLine(Iplimage, cp[i], cp[i1], cvScalar(color2));
    }
    cvMoments(Iplimage, &mState);

    r_moments.m00 = cvGetSpatialMoment(&mState, 0, 0)*0.5;
    r_moments.m10 = cvGetSpatialMoment(&mState, 1, 0)*0.5;
    r_moments.m01 = cvGetSpatialMoment(&mState, 0, 1)*0.5;
    r_moments.m20 = cvGetSpatialMoment(&mState, 2, 0)*0.5;
    r_moments.m11 = cvGetSpatialMoment(&mState, 1, 1)*0.5;
    r_moments.m02 = cvGetSpatialMoment(&mState, 0, 2)*0.5;
    r_moments.m30 = cvGetSpatialMoment(&mState, 3, 0)*0.5;
    r_moments.m21 = cvGetSpatialMoment(&mState, 2, 1)*0.5;
    r_moments.m12 = cvGetSpatialMoment(&mState, 1, 2)*0.5;
    r_moments.m03 = cvGetSpatialMoment(&mState, 0, 3)*0.5;

    r_moments.mu20 = cvGetCentralMoment(&mState, 2, 0)*0.5;
    r_moments.mu11 = cvGetCentralMoment(&mState, 1, 1)*0.5;
    r_moments.mu02 = cvGetCentralMoment(&mState, 0, 2)*0.5;
    r_moments.mu30 = cvGetCentralMoment(&mState, 3, 0)*0.5;
    r_moments.mu21 = cvGetCentralMoment(&mState, 2, 1)*0.5;
    r_moments.mu12 = cvGetCentralMoment(&mState, 1, 2)*0.5;
    r_moments.mu03 = cvGetCentralMoment(&mState, 0, 3)*0.5;
    r_moments.inv_sqrt_m00 = r_moments.m00 ? 1./sqrt(r_moments.m00) : 0;

//	H = sqrt((r_moments.mu20 - r_moments.mu02) * (r_moments.mu20 - r_moments.mu02) + 
//		          4 * r_moments.mu11 * r_moments.mu11) / 4;

    cvReleaseImage(&Iplimage);

    seq_type = CV_SEQ_POLYGON;


    cvMakeSeqHeaderForArray(seq_type, sizeof(CvContour), sizeof(CvPoint),
                            (char*)cp, nPoints2, (CvSeq*)&contour_h, &contour_blk);
//    cvMakeSeqHeaderForArray(seq_type, sizeof(CvSeq), sizeof(CvPoint),
//                            (char*)cp, nPoints2, &contour_h, &contour_blk));

/*  countour moments calculation  */
    if(algr == 0) 
    {
        cvContourMoments (&contour_h, &moments);
		H = sqrt((moments.mu20 - moments.mu02) * (moments.mu20 - moments.mu02)/4 + 
		          moments.mu11 * moments.mu11);

        error_test = 0.;
        for(i=0;i<=12;i++)
        {
            if( fabs((&(r_moments.m00))[i]) > eps )
                error_test += fabs((&(moments.m00))[i] - (&(r_moments.m00))[i])/
                              (&(r_moments.m00))[i];
        }
        error_test = error_test/13.;
        if(error_test > eps_rez ) code = TRS_FAIL;
        else code = TRS_OK;

    }
    else 
    {
/*  countour's area calculation  */
        area = cvContourArea (&contour_h);
        if(algr == 2)
            sec_area = cvContourArea (&contour_h, cvSlice(n1,n2));
        if(algr == 1)
        {
            error_area = fabs(r_moments.m00 - area)/r_moments.m00;
            if(error_area > eps_rez ) code = TRS_FAIL;
            else code = TRS_OK;
        }
        else 
        {
            error_sec_area = fabs(sec_area - area)/fabs(area);
            if(error_sec_area > eps_rez ) code = TRS_FAIL;
            else code = TRS_OK;
        }
    }

    switch(algr)
    {
    case 0:
        {trsWrite( ATS_CON | ATS_LST | ATS_SUM, "error_test_moments =%f \n",error_test); break;}
    case 1:
        {trsWrite( ATS_CON | ATS_LST | ATS_SUM, "area =%f error_test_area =%f \n",area,error_area); break;}
    case 2:
        {trsWrite( ATS_CON | ATS_LST | ATS_SUM, "sec_area =%f error_test_seq_area =%f \n",sec_area,error_sec_area);
         break;
        }
    }

    trsFree(cp);
/*    _getch();    */
    return code;
}

#define _MOMENTS    0
#define _AREA       1
#define _SEQAREA    2

void InitAContourMoments( void )
{
/* Test Registartion */
    trsRegArg(cFuncName[0],cTestName[0],cTestClass,aContourMoments, _MOMENTS); 
    trsRegArg(cFuncName[1],cTestName[1],cTestClass,aContourMoments, _AREA); 
    trsRegArg(cFuncName[2],cTestName[2],cTestClass,aContourMoments, _SEQAREA); 
} /* InitAContourMoments */

/* End of file. */

