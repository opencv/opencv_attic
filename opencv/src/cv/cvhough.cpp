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

#include <math.h>
#include "_cv.h"

/*=====================================================================================*/
/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:    
//    Purpose:
//     
//    Context:
//    Parameters:
//      
//      
//      
//    Returns:
//    Notes:
//      
//F*/

IPCVAPI_IMPL( int, icvHoughLines_8uC1R, (uchar * image,
                                         int step,
                                         CvSize size,
                                         float rho,
                                         float theta,
                                         int threshold, float *lines, int linesNumber) )
/*
Here image is an input raster;
step is it's step; size characterizes it's ROI;
rho and theta are discretization steps (in pixels and radians correspondingly).
threshold is the minimum number of pixels in the feature for it
to be a candidate for line. lines is the output
array of (rho, theta) pairs. linesNumber is the buffer size (number of pairs).
Functions return the actual number of found lines.

*/
{

    int width, height;
    int numangle;
    int longrho;
    int numrho;
    int *accum;
    float *tabSin;
    float *tabCos;
    int n;
    float ang;
    int i, j;
    int r;
    int currMax;
    int t;
    int base;

    float irho = 1 / rho;


    /*--- For no warnings ---*/
/*  image       = image;
    step        = step;
    size        = size;
    rho         = rho;
    theta       = theta; */
    threshold = threshold;
    lines = lines;
    linesNumber = linesNumber;
    /*-----------------------*/

    width = size.width;
    height = size.height;

    if( image == NULL )
    {
        return CV_NULLPTR_ERR;
    }

    if( width < 0 || height < 0 || width > step )
    {
        return CV_BADSIZE_ERR;
    }

    numangle = (int) (CV_PI / theta);
    longrho = (width + height) * 2 + 1; /*    -(i+j) <= rho <= (i+j) */
    numrho = (int) (longrho / rho);

    accum = (int *) icvAlloc( sizeof( int ) * numangle * numrho );
    tabSin = (float *) icvAlloc( sizeof( float ) * numangle );
    tabCos = (float *) icvAlloc( sizeof( float ) * numangle );
    memset( accum, 0, sizeof( int ) * numangle * numrho );


    if( tabSin == 0 || tabCos == 0 || accum == 0 )
    {
        if( tabSin != 0 )
            icvFree( tabSin );
        if( tabCos != 0 )
            icvFree( tabCos );
        if( accum != 0 )
            icvFree( accum );

        return 0;
    }

    /* May change using mirroring */
    for( ang = 0, n = 0; n < numangle; ang += theta, n++ )
    {
        tabSin[n] = (float) sin( ang );
        tabCos[n] = (float) cos( ang );
    }

    /* May be optimized ! */
    for( i = 0; i < width; i++ )
    {
        for( j = 0; j < height; j++ )
        {

            /* Get (i,j) pixel from image */

            if( image[j * step + i] != 0 )
            {

                for( n = 0; n < numangle; n++ )
                {

                    r = cvRound( (i * tabCos[n] + j * tabSin[n]) * irho );
                    r += (numrho - 1) / 2;
                    accum[n * numrho + r]++;
                }
            }
        }
    }




    /* Now find local maximums */

    currMax = 0;
    for( r = 1; r < numrho - 1; r++ )
    {
        for( t = 1; t < numangle - 1; t++ )
        {
            base = t * numrho + r;
            if( accum[base] > threshold )
            {                   /* candidate to maximum */
                if( accum[base] > accum[base - 1] &&
                    accum[base] > accum[base + 1] &&
                    accum[base] > accum[base - numrho] && accum[base] > accum[base + numrho] )
                {               /* is it a local maximum */

                    lines[currMax++] = (r - (numrho - 1) / 2) * rho;
                    lines[currMax++] = t * theta;

                    if( (currMax / 2) >= linesNumber )
                    {
                        icvFree( &tabSin );
                        icvFree( &tabCos );
                        icvFree( &accum );
                        return currMax / 2;

                    }           /* buffer is end */
                }               /* if maximum */
            }                   /* if accum is candidate */
        }                       /* for tbetas */
    }                           /* for rhos */

    icvFree( &tabSin );
    icvFree( &tabCos );
    icvFree( &accum );

    return currMax / 2;

}
