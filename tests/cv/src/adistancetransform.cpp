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

#include <stdlib.h>
#include <math.h>
#include <float.h>

static char* func_names[] = {"cvDistTransform"};
static char* test_desc[] = { "Measure the deviation of the distance from the euclidean, 3x3 mask", 
    "Measure the deviation of the distance from the euclidean, 5x5 mask"};

int test_dt(void* arg);
int read_image_dims(void);
int read_gen_type(void);

int read_image_gap_size(void)
{
    int RoiWidth;

    trsCaseRead(&RoiWidth, "/r/e", "r",
        "Enter the width of ROI: \n"
        "r - Random\n"
        "e - Equal to the image's width\n");
    return RoiWidth;
}

int read_gen_type(void)
{
    int fpGen;

    trsCaseRead(&fpGen, "/r/d", "r",
        "Method for generating feature points: \n"
        "r - Random generation\n"
        "d - Feature points are diagonal\n");
    return fpGen;
}

int test_dt(void* arg)
{
    int ip;
    int npoints = 100;
    int w = 100; /* width and height of the rect */
    int h = 100;
    int width = w; /* width and height of the source image */
    int height = 100;
    int length;
    CvSize size;
    int* x; /* Coordinates of feature points */
    int* y;
    IplImage* image; /* Source and destination images */
    IplImage* dist;
    uchar* _image;
    float* _dist;
    int im_step, dist_step;
    CvSize im_size, dist_size;
    float dev = 0; /* The maximum deviation from the Euclidean distance */
    float euclid; /* The minimum Euclidean distance */
    float distance;
    int xi, yi, xm = 0, ym = 0;
    int genType;
    CvDisMaskType maskType = CvDisMaskType(int(arg));

    size.width = w;
    size.height = h;

    switch(read_image_gap_size())
    {
    case 0:
        width = (int)cvFloor(atsInitRandom(100, 1000));
        break;

    case 1:
        width = w;
        break;
    }
    length = width*height;

    image = cvCreateImage( cvSize(w, h), 8, 1 );
    dist = cvCreateImage( cvSize(w, h), 32, 1);
    cvGetImageRawData( image, &_image, &im_step, &im_size );
    cvGetImageRawData( dist, (uchar**)&_dist, &dist_step, &dist_size );
    
    x = (int*)malloc(npoints*sizeof(int));
    y = (int*)malloc(npoints*sizeof(int));

    if(image == NULL || dist == NULL || x == 0 || y == 0)
    {
        return trsResult(TRS_FAIL, "Not enough memory to perform the test");
    }

    /* Insert random feature points */
    cvSet(image, cvScalarAll(1));
    switch(genType = read_gen_type())
    {
    case 0:
        for(ip = 0; ip < npoints; ip++)
        {
            x[ip] = (int)floor(atsInitRandom(0, w-1));
            y[ip] = (int)floor(atsInitRandom(0, h-1));
            _image[x[ip] + y[ip]*im_step] = 0;
        }
        break;

    case 1:
        for(ip = 0; ip < npoints; ip++)
        {
            x[ip] = ip%w;
            y[ip] = ip%h;
            _image[x[ip] + y[ip]*im_step] = 0;
        }
    }

    /* Run the distance transformation function */
    cvDistTransform(image, dist, CV_DIST_L2, maskType, 0);
    if(cvGetErrStatus() < 0)
    {
        cvReleaseImage( &image );
        cvReleaseImage( &dist );
        free( x );
        free( y );
        return trsResult(TRS_FAIL, "Function returned 'bad argument'");
    }



    /* Checking the maximum deviation */
    for(xi = 0; xi < w; xi++)
    {
        for(yi = 0; yi < h; yi++)
        {
            euclid = FLT_MAX;
            for(ip = 0; ip < npoints; ip++)
            {
                distance = (float)sqrt((xi-x[ip])*(xi-x[ip])+(yi-y[ip])*(yi-y[ip]));
                if(distance < euclid)
                {
                    euclid = distance;
                }
            }

            if(fabs(dev) < fabs(*(float*)((uchar*)&_dist[xi] + yi*dist_step) - euclid))
            {
                dev = *(float*)((uchar*)&_dist[xi] + yi*dist_step) - euclid;
                xm = xi;
                ym = yi;
            }
        }
    }

    cvReleaseImage( &image );
    cvReleaseImage( &dist );
    free( x );
    free( y );
    
    trsWrite(ATS_LST | ATS_CON, "Image size: %dx%d\n", w, h);
    trsWrite(ATS_LST | ATS_CON, "Maximum deviation from the Euclidean distance: %f\n", dev);
    trsWrite(ATS_LST | ATS_CON, "x = %d, y = %d\n", xm, ym);

    if(fabs(dev) > ((maskType == CV_DIST_MASK_3) ? 0.07f*w : 0.02f*w))
    {
        return trsResult(TRS_FAIL, "Bad accuracy");
    }
    else
    {
        return trsResult(TRS_OK, "No errors");
    }
}

void InitADistanceTransform(void)
{
    /* Registering test functions */
    trsRegArg(func_names[0], test_desc[0], atsAlgoClass, test_dt, CV_DIST_MASK_3);
    trsRegArg(func_names[0], test_desc[1], atsAlgoClass, test_dt, CV_DIST_MASK_5);

} /* InitADistanceTransform*/

