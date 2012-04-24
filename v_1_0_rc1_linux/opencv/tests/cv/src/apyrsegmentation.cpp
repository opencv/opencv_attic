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
/*#include "conio.h"   */

static char cTestName[] = "Image segmentation by pyramids";

static char cTestClass[] = "Algorithm";

static char *cFuncName[] = 
{
    "cvPyrSegmentation"
   
};

static int aPyrSegmentation(void* agr)
{
    CvPoint _cp[] ={{33,33}, {43,33}, {43,43}, {33,43}}; 
    CvPoint _cp2[] ={{50,50}, {70,50}, {70,70}, {50,70}};  
    CvPoint* cp = _cp;
    CvPoint* cp2 = _cp2;
    CvConnectedComp *dst_comp[3];
    CvRect rect[3] = {{50,50,21,21}, {0,0,128,128}, {33,33,11,11}};
    double a[3] = {441.0, 15822.0, 121.0};

/*    ippiPoint cp3[] ={130,130, 150,130, 150,150, 130,150};  */
/*	CvPoint cp[] ={0,0, 5,5, 5,0, 10,5, 10,0, 15,5, 15,0};  */
    int chanels = (int)(size_t)agr;    /* number of the color chanels  */
    int width = 128;
    int height = 128;
    int nPoints = 4;
    int block_size = 1000;
    int color1 = 30, color2 = 110, color3 = 180;
    int level = 5;
    long diff, l;
    int code;

    CvMemStorage *storage;   /*   storage for connected component writing  */
    CvSeq *comp;

    double lower, upper;
    unsigned seed;
    char rand;
    AtsRandState state;
    int i,j;

    IplImage *image, *image_f, *image_s;
    CvSize size;
    uchar *f_cur, *f_row;
    uchar *row;
    uchar *cur;
    int threshold1, threshold2;

    code = TRS_OK;

    if(chanels != 1 && chanels != 3)
        return TRS_UNDEF;
/* read tests params */

    if(!trsiRead( &width, "128", "image width" ))
        return TRS_UNDEF;
    if(!trsiRead( &height, "128", "image height" ))
        return TRS_UNDEF;
    if(!trsiRead( &level, "5", "pyramid level" ))
        return TRS_UNDEF;


/*  create Image   */
    l = width*height;
    size.width = width;
    size.height = height;

    rect[1].height = height;
    rect[1].width = width;
    a[1] = l - a[0] - a[2];

    image = cvCreateImage(cvSize(size.width, size.height), IPL_DEPTH_8U, chanels); 
    image_s = cvCreateImage(cvSize(size.width, size.height), IPL_DEPTH_8U, chanels); 

    memset(image->imageData, color1, chanels*l);

    image_f = cvCreateImage(cvSize(size.width, size.height), IPL_DEPTH_8U, chanels); 

    OPENCV_CALL( storage = cvCreateMemStorage( block_size ) );

/*  do noise   */
    upper = 20;
    lower = -upper;
    seed = 345753;
    atsRandInit( &state, lower, upper, seed );

/*   segmentation by pyramid     */    
    threshold1 = 50;
    threshold2 = 50;

    switch(chanels)
    {
        case 1:
        {
            cvFillPoly( image, &cp, &nPoints, 1, cvScalar(color2));
            cvFillPoly( image, &cp2, &nPoints, 1, cvScalar(color3)); 

            row = (uchar*)image->imageData;
            f_row = (uchar*)image_f->imageData;
            for(i = 0; i<size.height; i++)
            {
                cur = row;
                f_cur = f_row;
                for(j = 0; j<size.width; j++)
                {
                    atsbRand8s( &state, &rand, 1);
                    *(f_cur)=(uchar)((*cur) + rand);
                    cur++;
                    f_cur++;
                }
                row+=image->widthStep;
                f_row+=image_f->widthStep;
            }

            cvPyrSegmentation( image_f, image_s,
                               storage, &comp, 
                               level, threshold1, threshold2 );

            //if(comp->total != 3) { code = TRS_FAIL; goto exit; }
/*  read the connected components     */
            /*dst_comp[0] = (CvConnectedComp*)CV_GET_SEQ_ELEM( CvConnectedComp, comp, 0 );
            dst_comp[1] = (CvConnectedComp*)CV_GET_SEQ_ELEM( CvConnectedComp, comp, 1 );
            dst_comp[2] = (CvConnectedComp*)CV_GET_SEQ_ELEM( CvConnectedComp, comp, 2 );*/
            break;
        }
        case 3:
        {
            cvFillPoly( image, &cp, &nPoints, 1, CV_RGB(color2,color2,color2));
            cvFillPoly( image, &cp2, &nPoints, 1, CV_RGB(color3,color3,color3)); 

            row = (uchar*)image->imageData;
            f_row = (uchar*)image_f->imageData;
            for(i = 0; i<size.height; i++)
            {
                cur = row;
                f_cur = f_row;
                for(j = 0; j<size.width; j++)
                {
                    atsbRand8s( &state, &rand, 1);
                    *(f_cur)=(uchar)((*cur) + rand);
                    atsbRand8s( &state, &rand, 1);
                    *(f_cur+1)=(uchar)(*(cur+1) + rand);
                    atsbRand8s( &state, &rand, 1);
                    *(f_cur+2)=(uchar)(*(cur+2) + rand);
                    cur+=3;
                    f_cur+=3;
                }
                row+=image->widthStep;
                f_row+=image_f->widthStep;
            }

            cvPyrSegmentation(image_f, image_s, storage, &comp, level,
                              threshold1, threshold2);   
/*  read the connected components     */
            if(comp->total != 3) { code = TRS_FAIL; goto exit; }
            dst_comp[0] = (CvConnectedComp*)CV_GET_SEQ_ELEM( CvConnectedComp, comp, 0 );
            dst_comp[1] = (CvConnectedComp*)CV_GET_SEQ_ELEM( CvConnectedComp, comp, 1 );
            dst_comp[2] = (CvConnectedComp*)CV_GET_SEQ_ELEM( CvConnectedComp, comp, 2 );
            break;
        }
    }
 
    diff = 0;
    /*diff = atsCompare1Db( (uchar*)image->imageData, (uchar*)image_s->imageData, chanels*l, 4);
 
    for(i = 0; i < 3; i++)
    {
        if(dst_comp[i]->area != a[i]) diff++;
        if(dst_comp[i]->rect.x != rect[i].x) diff++;
        if(dst_comp[i]->rect.y != rect[i].y) diff++;
        if(dst_comp[i]->rect.width != rect[i].width) diff++;
        if(dst_comp[i]->rect.height != rect[i].height) diff++;
    }*/

    trsWrite( ATS_CON | ATS_LST | ATS_SUM, "upper =%f diff =%ld \n",upper, diff);

    if(diff > 0 )
        code = TRS_FAIL;
    else
        code = TRS_OK;

exit:

    cvReleaseMemStorage( &storage );
    cvReleaseImage(&image_f);
    cvReleaseImage(&image);
    cvReleaseImage(&image_s);

   

/*    trsFree(cp);  */
/*    _getch();     */
    return code;
 
    
}

#define _8U_C1    1
#define _8U_C3    3

void InitAPyrSegmentation( void )
{
/* Test Registartion */
    trsRegArg(cFuncName[0],cTestName,cTestClass,aPyrSegmentation, _8U_C1); 
    trsRegArg(cFuncName[0],cTestName,cTestClass,aPyrSegmentation, _8U_C3); 

} /* InitAContourMoments */

/* End of file. */
