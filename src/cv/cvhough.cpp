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

#include "_cv.h"
#include "_cvlist.h"

#define halfPi ((float)(CV_PI*0.5))
#define Pi     ((float)CV_PI)
#define a0  0 /*-4.172325e-7f*/   /*(-(float)0x7)/((float)0x1000000); */
#define a1 1.000025f        /*((float)0x1922253)/((float)0x1000000)*2/Pi; */
#define a2 -2.652905e-4f    /*(-(float)0x2ae6)/((float)0x1000000)*4/(Pi*Pi); */
#define a3 -0.165624f       /*(-(float)0xa45511)/((float)0x1000000)*8/(Pi*Pi*Pi); */
#define a4 -1.964532e-3f    /*(-(float)0x30fd3)/((float)0x1000000)*16/(Pi*Pi*Pi*Pi); */
#define a5 1.02575e-2f      /*((float)0x191cac)/((float)0x1000000)*32/(Pi*Pi*Pi*Pi*Pi); */
#define a6 -9.580378e-4f    /*(-(float)0x3af27)/((float)0x1000000)*64/(Pi*Pi*Pi*Pi*Pi*Pi); */

#define _sin(x) ((((((a6*(x) + a5)*(x) + a4)*(x) + a3)*(x) + a2)*(x) + a1)*(x) + a0)
#define _cos(x) _sin(halfPi - (x))

/****************************************************************************************\
*                               Classical Hough Transform                                *
\****************************************************************************************/

typedef struct CvLinePolar
{
    float rho;
    float angle;
}
CvLinePolar;

/*=====================================================================================*/

#define hough_cmp_gt(l1,l2) (aux[l1] > aux[l2])

static CV_IMPLEMENT_QSORT_EX( icvHoughSortDescent32s, int, hough_cmp_gt, const int* );

/*
Here image is an input raster;
step is it's step; size characterizes it's ROI;
rho and theta are discretization steps (in pixels and radians correspondingly).
threshold is the minimum number of pixels in the feature for it
to be a candidate for line. lines is the output
array of (rho, theta) pairs. linesMax is the buffer size (number of pairs).
Functions return the actual number of found lines.
*/
static CvStatus
icvHoughLines_8uC1R( uchar* image, int step, CvSize size,
                     float rho, float theta, int threshold,
                     CvSeq *lines, int linesMax )
{
    int width, height;
    int numangle, numrho;
    int *accum = 0;
    int *sort_buf=0;
    int total = 0;
    float *tabSin = 0;
    float *tabCos = 0;
    float ang;
    int r, n;
    int i, j;
    float irho = 1 / rho;
    double scale;

    width = size.width;
    height = size.height;

    if( image == NULL )
        return CV_NULLPTR_ERR;

    if( width < 0 || height < 0 )
        return CV_BADSIZE_ERR;

    numangle = (int) (Pi / theta);
    numrho = (int) (((width + height) * 2 + 1) / rho);

    accum = (int*)cvAlloc( sizeof(accum[0]) * (numangle+2) * (numrho+2) );
    sort_buf = (int*)cvAlloc( sizeof(accum[0]) * numangle * numrho );
    tabSin = (float*)cvAlloc( sizeof(tabSin[0]) * numangle );
    tabCos = (float*)cvAlloc( sizeof(tabCos[0]) * numangle );
    memset( accum, 0, sizeof(accum[0]) * (numangle+2) * (numrho+2) );

    if( tabSin == 0 || tabCos == 0 || accum == 0 )
        goto func_exit;

    for( ang = 0, n = 0; n < numangle; ang += theta, n++ )
    {
        tabSin[n] = (float)(sin(ang) * irho);
        tabCos[n] = (float)(cos(ang) * irho);
    }

    // stage 1. fill accumulator
    for( j = 0; j < height; j++ )
        for( i = 0; i < width; i++ )
        {
            if( image[j * step + i] != 0 )
                for( n = 0; n < numangle; n++ )
                {
                    r = cvRound( i * tabCos[n] + j * tabSin[n] );
                    r += (numrho - 1) / 2;
                    accum[(n+1) * (numrho+2) + r+1]++;
                }
        }

    // stage 2. find local maximums 
    for( r = 0; r < numrho; r++ )
        for( n = 0; n < numangle; n++ )
        {
            int base = (n+1) * (numrho+2) + r+1;
            if( accum[base] > threshold &&
                accum[base] > accum[base - 1] && accum[base] >= accum[base + 1] &&
                accum[base] > accum[base - numrho - 2] && accum[base] >= accum[base + numrho + 2] )
                sort_buf[total++] = base;
        }

    // stage 3. sort the detected lines by accumulator value
    icvHoughSortDescent32s( sort_buf, total, accum );
    
    // stage 4. store the first min(total,linesMax) lines to the output buffer
    linesMax = MIN(linesMax, total);
    scale = 1./(numrho+2);
    for( i = 0; i < linesMax; i++ )
    {
        CvLinePolar line;
        int idx = sort_buf[i];
        int n = cvFloor(idx*scale) - 1;
        int r = idx - (n+1)*(numrho+2) - 1;
        line.rho = (r - (numrho - 1)*0.5f) * rho;
        line.angle = n * theta;
        cvSeqPush( lines, &line );
    }

func_exit:
    cvFree( (void**)&sort_buf );
    cvFree( (void**)&tabSin );
    cvFree( (void**)&tabCos );
    cvFree( (void**)&accum );

    return CV_OK;
}


/****************************************************************************************\
*                     Multi-Scale variant of Classical Hough Transform                   *
\****************************************************************************************/

#if defined _MSC_VER && _MSC_VER >= 1200
#pragma warning( disable: 4714 )
#endif

//DECLARE_AND_IMPLEMENT_LIST( _index, h_ );
IMPLEMENT_LIST( _index, h_ )

static  CvStatus  icvHoughLinesSDiv_8uC1R( uchar * image_src, int step, CvSize size,
                                           float rho, float theta, int threshold,
                                           int srn, int stn,
                                           CvSeq* lines, int linesMax )
{
#define _POINT(row, column)\
    (image_src[(row)*step+(column)])

    int rn, tn;                 /* number of rho and theta discrete values */

    uchar *mcaccum = 0;
    uchar *caccum = 0;
    uchar *buffer = 0;
    float *sinTable = 0;
    
    int *x = 0;
    int *y = 0;

    int index, i;
    int ri, ti, ti1, ti0;
    int row, col;
    float r, t;                 /* Current rho and theta */
    float rv;                   /* Some temporary rho value */
    float irho;
    float itheta;
    float srho, stheta;
    float isrho, istheta;

    int w = size.width;
    int h = size.height;

    int fn = 0;
    float xc, yc;

    const float d2r = (float)(Pi / 180);

    int sfn = srn * stn;
    int fi;
    int count;

    int cmax = 0;

    _CVLIST *list;
    CVPOS pos;
    _index *pindex;
    _index vi;

    if( image_src == NULL )
        return CV_NULLPTR_ERR;

    if( size.width < 0 || size.height < 0 )
        return CV_BADSIZE_ERR;

    if( linesMax == 0 || rho <= 0 || theta <= 0 )
        return CV_BADFACTOR_ERR;

    irho = 1 / rho;
    itheta = 1 / theta;
    srho = rho / srn;
    stheta = theta / stn;
    isrho = 1 / srho;
    istheta = 1 / stheta;

    rn = cvFloor( sqrt( (double)w * w + (double)h * h ) * irho );
    tn = cvFloor( 2 * Pi * itheta );

    list = h_create_list__index( linesMax < 1000 ? linesMax : 1000 );
    vi.value = threshold;
    vi.rho = -1;
    h_add_head__index( list, &vi );

    /* Precalculating sin */
    sinTable = (float*)cvAlloc( 5 * tn * stn * sizeof( float ));

    for( index = 0; index < 5 * tn * stn; index++ )
    {
        sinTable[index] = (float)cos( stheta * index * 0.2f );
    }

    /* Allocating memory for the accumulator ad initializing it */
    if( threshold > 255 )
        goto func_exit;

    caccum = (uchar*)cvAlloc( rn * tn * sizeof( caccum[0] ));
    memset( caccum, 0, rn * tn * sizeof( caccum[0] ));

    /* Counting all feature pixels */
    for( row = 0; row < h; row++ )
        for( col = 0; col < w; col++ )
            fn += _POINT( row, col ) != 0;

    x = (int*)cvAlloc( fn * sizeof(x[0]));
    y = (int*)cvAlloc( fn * sizeof(y[0]));

    /* Full Hough Transform (it's accumulator update part) */
    fi = 0;
    if( threshold < 256 )
    {
        for( row = 0; row < h; row++ )
        {
            for( col = 0; col < w; col++ )
            {
                if( _POINT( row, col ))
                {
                    int halftn;
                    float r0;
                    float scale_factor;
                    int iprev = -1;
                    float phi, phi1;
                    float theta_it;     /* Value of theta for iterating */

                    /* Remember the feature point */
                    x[fi] = col;
                    y[fi] = row;
                    fi++;

                    yc = (float) row + 0.5f;
                    xc = (float) col + 0.5f;

                    /* Update the accumulator */
                    t = (float) fabs( cvFastArctan( yc, xc ) * d2r );
                    r = (float) sqrt( (double)xc * xc + (double)yc * yc );
                    r0 = r * irho;
                    ti0 = cvFloor( (t + Pi / 2) * itheta );

                    caccum[ti0]++;

                    theta_it = rho / r;
                    theta_it = theta_it < theta ? theta_it : theta;
                    scale_factor = theta_it * itheta;
                    halftn = cvFloor( Pi / theta_it );
                    for( ti1 = 1, phi = theta_it - halfPi, phi1 = (theta_it + t) * itheta;
                         ti1 < halftn; ti1++, phi += theta_it, phi1 += scale_factor )
                    {
                        rv = r0 * _cos( phi );
                        i = cvFloor( rv ) * tn;
                        i += cvFloor( phi1 );
                        assert( i >= 0 );
                        assert( i < rn * tn );
                        caccum[i] = (unsigned char) (caccum[i] + ((i ^ iprev) != 0));
                        iprev = i;
                        if( cmax < caccum[i] )
                            cmax = caccum[i];
                    }
                }
            }
        }
    }
    else
    {
        cvClearSeq( lines );
        goto func_exit;
    }

    /* Starting additional analysis */

    count = 0;
    for( ri = 0; ri < rn; ri++ )
    {
        for( ti = 0; ti < tn; ti++ )
        {
            if( caccum[ri * tn + ti > threshold] )
            {
                count++;
            }
        }
    }

    if( count * 100 > rn * tn )
    {
        icvHoughLines_8uC1R( image_src, step, size, rho, theta,
                             threshold, lines, linesMax );
        goto func_exit;
    }

    buffer = (uchar *) cvAlloc( (srn * stn + 2) * sizeof( uchar ));
    mcaccum = buffer + 1;

    count = 0;
    for( ri = 0; ri < rn; ri++ )
    {
        for( ti = 0; ti < tn; ti++ )
        {
            if( caccum[ri * tn + ti] > threshold )
            {
                count++;
                memset( mcaccum, 0, sfn * sizeof( uchar ));

                for( index = 0; index < fn; index++ )
                {
                    int ti2;
                    float r0;

                    yc = (float) y[index] + 0.5f;
                    xc = (float) x[index] + 0.5f;

                    /* Update the accumulator */
                    t = (float) fabs( cvFastArctan( yc, xc ) * d2r );
                    r = (float) sqrt( (double)xc * xc + (double)yc * yc ) * isrho;
                    ti0 = cvFloor( (t + Pi * 0.5f) * istheta );
                    ti2 = (ti * stn - ti0) * 5;
                    r0 = (float) ri *srn;

                    for( ti1 = 0 /*, phi = ti*theta - Pi/2 - t */ ; ti1 < stn; ti1++, ti2 += 5
                         /*phi += stheta */  )
                    {
                        /*rv = r*_cos(phi) - r0; */
                        rv = r * sinTable[(int) (abs( ti2 ))] - r0;
                        i = cvFloor( rv ) * stn + ti1;

                        i = CV_IMAX( i, -1 );
                        i = CV_IMIN( i, sfn );
                        mcaccum[i]++;
                        assert( i >= -1 );
                        assert( i <= sfn );
                    }
                }

                /* Find peaks in maccum... */
                for( index = 0; index < sfn; index++ )
                {
                    i = 0;
                    pos = h_get_tail_pos__index( list );
                    if( h_get_prev__index( &pos )->value < mcaccum[index] )
                    {
                        vi.value = mcaccum[index];
                        vi.rho = index / stn * srho + ri * rho;
                        vi.theta = index % stn * stheta + ti * theta - halfPi;
                        while( h_is_pos__index( pos ))
                        {
                            if( h_get__index( pos )->value > mcaccum[index] )
                            {
                                h_insert_after__index( list, pos, &vi );
                                if( h_get_count__index( list ) > linesMax )
                                {
                                    h_remove_tail__index( list );
                                }
                                break;
                            }
                            h_get_prev__index( &pos );
                        }
                        if( !h_is_pos__index( pos ))
                        {
                            h_add_head__index( list, &vi );
                            if( h_get_count__index( list ) > linesMax )
                            {
                                h_remove_tail__index( list );
                            }
                        }
                    }
                }
            }
        }
    }

    pos = h_get_head_pos__index( list );
    if( h_get_count__index( list ) == 1 )
    {
        if( h_get__index( pos )->rho < 0 )
        {
            h_clear_list__index( list );
        }
    }
    else
    {
        while( h_is_pos__index( pos ))
        {
            CvLinePolar line;
            pindex = h_get__index( pos );
            if( pindex->rho < 0 )
            {
                /* This should be the last element... */
                h_get_next__index( &pos );
                assert( !h_is_pos__index( pos ));
                break;
            }
            line.rho = pindex->rho;
            line.angle = pindex->theta;
            cvSeqPush( lines, &line );

            if( lines->total >= linesMax )
                goto func_exit;
            h_get_next__index( &pos );
        }
    }

func_exit:
    h_destroy_list__index( list );

    cvFree( (void**)&sinTable );
    cvFree( (void**)&x );
    cvFree( (void**)&y );
    cvFree( (void**)&caccum );
    cvFree( (void**)&buffer );

    return CV_OK;
}


/****************************************************************************************\
*                              Probabilistic Hough Transform                             *
\****************************************************************************************/

#define _PHOUGH_SIN_TABLE

static  CvStatus  icvHoughLinesP_8uC1R( uchar * image_src, int step, CvSize size,
                                        float rho, float theta, int threshold,
                                        int lineLength, int lineGap,
                                        CvSeq *lines, int linesMax )
{
#define _POINT(row, column)\
    (image_src[(row)*step+(column)])

    int *map = 0;
    int rn, tn;                 /* number of rho and theta discrete values */

#define ROUNDR(x) cvFloor(x)
#define ROUNDT(x) cvFloor(x)

#ifdef _PHOUGH_SIN_TABLE
    #define SIN(a)  sinTable[a]
#else
    #define SIN(a)  sin((a)*theta)
#endif

    int *iaccum = 0;
    uchar *caccum = 0;
    int imaccum;
    uchar cmaccum;

    int *x = 0;
    int *y = 0;

    int index, i;
    int ri, ri1, ti, ti1, ti0;
    int halftn;
    int row, col;
    float r, t;                 /* Current rho and theta */
    float rv;                   /* Some temporary rho value */
    float irho;
    float itheta;

    int w = size.width;
    int h = size.height;

    int fn = 0;
    float xc, yc;

    const float d2r = (float)(Pi / 180);

    int fpn = 0;

    float *sinTable = 0;
    CvRNG state = CvRNG(0xffffffff);

    if( linesMax <= 0 )
        return CV_BADSIZE_ERR;

    if( rho <= 0 || theta <= 0 )
        return CV_BADARG_ERR;

    irho = 1 / rho;
    itheta = 1 / theta;

    rn = cvFloor( sqrt( (double)w * w + (double)h * h ) * irho );
    tn = cvFloor( 2 * Pi * itheta );
    halftn = cvFloor( Pi * itheta );

    /* Allocating memory for the accumulator ad initializing it */
    if( threshold > 255 )
    {
        iaccum = (int *) cvAlloc( rn * tn * sizeof( int ));
        memset( iaccum, 0, rn * tn * sizeof( int ));
    }
    else
    {
        caccum = (uchar *) cvAlloc( rn * tn * sizeof( uchar ));
        memset( caccum, 0, rn * tn * sizeof( uchar ));
    }


    /* Counting all feature pixels */

    for( row = 0; row < h; row++ )
    {
        for( col = 0; col < w; col++ )
        {
            fn += !!_POINT( row, col );
        }
    }

    x = (int *) cvAlloc( fn * sizeof( int ));
    y = (int *) cvAlloc( fn * sizeof( int ));
    map = (int *) cvAlloc( w * h * sizeof( int ));
    memset( map, -1, w * h * sizeof( int ));

#ifdef _PHOUGH_SIN_TABLE
    sinTable = (float *) cvAlloc( tn * sizeof( float ));

    for( ti = 0; ti < tn; ti++ )
    {
        sinTable[ti] = (float)sin( (double)(ti * theta) );
    }
#endif

    index = 0;
    for( row = 0; row < h; row++ )
    {
        for( col = 0; col < w; col++ )
        {
            if( _POINT( row, col ))
            {
                x[index] = col;
                y[index] = row;
                map[row * w + col] = index;
                index++;
            }
        }
    }

    /* Starting Hough Transform */
    while( fn != 0 )
    {
        int temp;
        int index0;
        int cl;                 /* Counter of length of lines of feature pixels */
        int cg;                 /* Counter of gaps length in lines of feature pixels */
        float dx = 1.0f, dy = 0.0f, ax, ay;
        float msx = 0, msy = 0, mex = 0, mey = 0, mdx = 1.0f, mdy = 0.0f;
        int ml;

        /* The x, y and length of a line (remember the maximum length) */
        float curx = 0, cury = 0;
        int ox, oy;             /* Rounded ax and ay */
        int ex, ey;

#define _EXCHANGE(x1, x2) temp = x1;x1 = x2;x2 = temp

        /* Select a pixel randomly */
        index0 = cvRandInt(&state) % fn;
        /* Remove the pixel from the feature points set */
        if( index0 != fn - 1 )
        {
            /* Exchange the point with the last one */
            _EXCHANGE( x[index0], x[fn - 1] );
            _EXCHANGE( y[index0], y[fn - 1] );
            _EXCHANGE( map[y[index0] * w + x[index0]], map[y[fn - 1] * w + x[fn - 1]] );
        }

        fn--;
        fpn++;

        yc = (float) y[fn] + 0.5f;
        xc = (float) x[fn] + 0.5f;

        /* Update the accumulator */
        t = (float) fabs( cvFastArctan( yc, xc ) * d2r );
        r = (float) sqrt( (double)xc * xc + (double)yc * yc );
        ti0 = ROUNDT( t * itheta );

        /* ti1 = 0 */
        if( threshold > 255 )
        {
            rv = 0.0f;
            ri1 = 0;
            i = ti0;
            iaccum[i]++;
            imaccum = iaccum[i];
            ri = ri1;
            ti = ti0;
            for( ti1 = 1; ti1 < halftn; ti1++ )
            {
                rv = r * SIN( ti1 );
                ri1 = ROUNDR( rv * irho );
                i = ri1 * tn + ti1 + ti0;
                iaccum[i]++;
                if( imaccum < iaccum[i] )
                {
                    imaccum = iaccum[i];
                    ri = ri1;
                    ti = ti1 + ti0;
                }
            }

            r = ri * rho + rho / 2;
            t = ti * theta + theta / 2;

            if( iaccum[ri * tn + ti] < threshold )
            {
                continue;
            }

            /* Unvote all the pixels from the detected line */
            iaccum[ri * tn + ti] = 0;
        }
        else
        {
            rv = 0.0f;
            ri1 = 0;
            i = ti0;
            caccum[i]++;
            cmaccum = caccum[i];
            ri = ri1;
            ti = ti0;
            for( ti1 = 1; ti1 < halftn; ti1++ )
            {
                rv = r * SIN( ti1 );
                ri1 = ROUNDR( rv * irho );
                i = ri1 * tn + ti1 + ti0;
                
                caccum[i]++;
                if( cmaccum < caccum[i] )
                {
                    cmaccum = caccum[i];
                    ri = ri1;
                    ti = ti1 + ti0;
                }
            }

            r = ri * rho + rho / 2;
            t = ti * theta + theta / 2;

            if( caccum[ri * tn + ti] < threshold )
            {
                continue;
            }

            /* Unvote all the pixels from the detected line */
            caccum[ri * tn + ti] = 0;
        }

        /* Find a longest segment representing the line */
        /* Use an algorithm like Bresenheim one        */
        ml = 0;

        for( i = 0; i < 7; i++ )
        {
            switch (i)
            {
            case 0:
                break;

            case 1:
                r = ri * rho;
                t = ti * theta - halfPi + 0.1f * theta;
                break;

            case 2:
                r = ri * rho;
                t = (ti + 1) * theta - halfPi - 0.1f * theta;
                break;

            case 3:
                r = (ri + 1) * rho - 0.1f * rho;
                t = ti * theta - halfPi + 0.1f * theta;
                break;

            case 4:
                r = (ri + 1) * rho - 0.1f * rho;
                t = (ti + 1) * theta - halfPi - 0.1f * theta;
                break;

            case 5:
                r = ri * rho + 0.1f * rho;
                t = ti * theta - halfPi + 0.5f * theta;
                break;

            case 6:
                r = (ri + 1) * rho - 0.1f * rho;
                t = ti * theta - halfPi + 0.5f * theta;
                break;

            }

            if( t > Pi )
            {
                t = t - 2 * Pi;
            }

            if( t >= 0 )
            {
                if( t <= Pi / 2 )
                {
                    dx = -(float) sin( t );
                    dy = (float) cos( t );

                    if( r < (w - 1) * fabs( dy ))
                    {
                        ax = (float) cvFloor( r / dy ) + 0.5f;
                        ay = 0.5f;
                    }
                    else
                    {
                        ax = (float) w - 0.5f;
                        ay = (float) cvFloor( (r - (w - 1) * dy) / (float) fabs( dx )) + 0.5f;
                    }
                }
                else
                {
                    /* Pi/2 < t < Pi */
                    dx = (float) sin( t );
                    dy = -(float) cos( t );

                    ax = 0.5f;
                    ay = (float) cvFloor( r / dx ) + 0.5f;
                }
            }
            else
            {
                /* -Pi/2 < t < 0 */
                dx = -(float) sin( t );
                dy = (float) cos( t );
                ax = (float) cvFloor( r / dy ) + 0.5f;
                ay = 0.5f;
            }

            cl = 0;
            cg = 0;

            ox = cvFloor( ax );
            oy = cvFloor( ay );
            while( ox >= 0 && ox < w && oy >= 0 && oy < h )
            {
                if( _POINT( oy, ox ))
                {
                    if( cl == 0 )
                    {
                        /* A line has started */
                        curx = ax;
                        cury = ay;
                    }

                    cl++;
                    cg = 0;     /* No gaps so far */
                }
                else if( cl )
                {
                    if( ++cg > lineGap )
                    {
                        /* This is not a gap, the line has finished */
                        /* Let us remember it's parameters */
                        if( ml < cl )
                        {
                            msx = curx;
                            msy = cury;
                            mex = ax;
                            mey = ay;
                            mdx = dx;
                            mdy = dy;
                            ml = cl;
                        }
                        cl = 0;
                        cg = 0;
                    }
                }

                ax += dx;
                ay += dy;
                ox = cvFloor( ax );
                oy = cvFloor( ay );
            }

            /* The last line if there was any... */
            if( ml < cl )
            {
                msx = curx;
                msy = cury;
                mex = ax;
                mey = ay;
                mdx = dx;
                mdy = dy;
                ml = cl;
            }
        }

        if( ml == 0 )
        {
            // no line...
            continue;
        }

        /* Now let's remove all the pixels in the segment from the input image */
        cl = 0;
        cg = 0;
        ax = msx;
        ay = msy;
        ox = cvFloor( msx );
        oy = cvFloor( msy );
        ex = cvFloor( mex );
        ey = cvFloor( mey );

        while( (ox != ex || oy != ey) && fn > 0 )
        {
            if( (unsigned)ox >= (unsigned)w || (unsigned)oy >= (unsigned)h )
                break;
            {
                image_src[oy * step + ox] = 0;
                index0 = map[oy * w + ox];
                if( index0 != -1 )
                {
                    if( index0 != fn - 1 )
                    {
                        /* Exchange the point with the last one */
                        _EXCHANGE( x[index0], x[fn - 1] );
                        _EXCHANGE( y[index0], y[fn - 1] );
                        _EXCHANGE( map[y[index0] * w + x[index0]],
                                   map[y[fn - 1] * w + x[fn - 1]] );
                    }
                    fn--;
                }
            }

            ax += mdx;
            ay += mdy;
            ox = cvFloor( ax );
            oy = cvFloor( ay );
        }

        if( ml >= lineLength )
        {
            CvRect line;
            line.x = cvFloor( msx );
            line.y = cvFloor( msy );
            line.width = cvFloor( mex );
            line.height = cvFloor( mey );
            cvSeqPush( lines, &line );

            if( lines->total >= linesMax || fn == 0 )
                goto func_exit;
        }
    }
func_exit:
    cvFree( (void**)&x );
    cvFree( (void**)&y );
    cvFree( (void**)&map );
    cvFree( (void**)&sinTable );
    cvFree( (void**)&iaccum );
    cvFree( (void**)&caccum );

    return CV_OK;
}


/* Wrapper function for standard hough transform */
CV_IMPL CvSeq*
cvHoughLines2( CvArr* src_image, void* lineStorage, int method,
               double rho, double theta, int threshold,
               double param1, double param2 )
{
    CvSeq* result = 0;

    CV_FUNCNAME( "cvHoughLines" );

    __BEGIN__;
    
    CvMat stub, *img = (CvMat*)src_image;
    CvMat* mat = 0;
    CvSeq* lines = 0;
    CvSeq lines_header;
    CvSeqBlock lines_block;
    int lineType, elemSize;
    int linesMax = INT_MAX;
    CvSize size;
    int iparam1, iparam2;

    CV_CALL( img = cvGetMat( img, &stub ));

    if( !CV_IS_MASK_ARR(img))
        CV_ERROR( CV_StsBadArg, "The source image must be 8-bit, single-channel" );

    if( !lineStorage )
        CV_ERROR( CV_StsNullPtr, "NULL destination" );

    if( rho <= 0 || theta <= 0 || threshold <= 0 )
        CV_ERROR( CV_StsOutOfRange, "rho, theta and threshold must be positive" );

    if( method != CV_HOUGH_PROBABILISTIC )
    {
        lineType = CV_32FC2;
        elemSize = sizeof(float)*2;
    }
    else
    {
        lineType = CV_32SC4;
        elemSize = sizeof(int)*4;
    }

    if( CV_IS_STORAGE( lineStorage ))
    {
        CV_CALL( lines = cvCreateSeq( lineType, sizeof(CvSeq), elemSize, (CvMemStorage*)lineStorage ));
    }
    else if( CV_IS_MAT( lineStorage ))
    {
        mat = (CvMat*)lineStorage;

        if( !CV_IS_MAT_CONT( mat->type ) || mat->rows != 1 && mat->cols != 1 )
            CV_ERROR( CV_StsBadArg,
            "The destination matrix should be continuous and have a single row or a single column" );

        if( CV_MAT_TYPE( mat->type ) != lineType )
            CV_ERROR( CV_StsBadArg,
            "The destination matrix data type is inappropriate, see the manual" );

        CV_CALL( lines = cvMakeSeqHeaderForArray( lineType, sizeof(CvSeq), elemSize, mat->data.ptr,
                                                  mat->rows + mat->cols - 1, &lines_header, &lines_block ));
        linesMax = lines->total;
        CV_CALL( cvClearSeq( lines ));
    }
    else
    {
        CV_ERROR( CV_StsBadArg, "Destination is not CvMemStorage* nor CvMat*" );
    }

    size = cvGetMatSize(img);
    iparam1 = cvRound(param1);
    iparam2 = cvRound(param2);

    switch( method )
    {
    case CV_HOUGH_STANDARD:
          IPPI_CALL( icvHoughLines_8uC1R( img->data.ptr, img->step, size, (float)rho,
                                          (float)theta, threshold, lines, linesMax ));
          break;
    case CV_HOUGH_MULTI_SCALE:
          IPPI_CALL( icvHoughLinesSDiv_8uC1R( img->data.ptr, img->step, size, (float)rho,
                                              (float)theta, threshold, iparam1,
                                              iparam2, lines, linesMax ));
          break;
    case CV_HOUGH_PROBABILISTIC:
          IPPI_CALL( icvHoughLinesP_8uC1R( img->data.ptr, img->step, size, (float)rho,
                                           (float)theta, threshold, iparam1,
                                           iparam2, lines, linesMax ));
          break;
    default:
        CV_ERROR( CV_StsBadArg, "Unrecognized method id" );
    }

    if( mat )
    {
        if( mat->cols > mat->rows )
            mat->cols = lines->total;
        else
            mat->rows = lines->total;
    }
    else
    {
        result = lines;
    }

    __END__;
    
    return result;    
}


/****************************************************************************************\
*                                     Circle Detection                                   *
\****************************************************************************************/

static void
icvHoughCirclesGradient( CvMat* img, float dp, float min_dist,
                         int canny_threshold, int acc_threshold,
                         CvSeq* circles, int circles_max )
{
    const int SHIFT = 10, ONE = 1 << SHIFT, R_THRESH = 30;
    CvMat *dx = 0, *dy = 0;
    CvMat *edges = 0;
    CvMat *accum = 0;
    int* sort_buf = 0;
    CvMat* dist_buf = 0;
    CvMemStorage* storage = 0;
    
    CV_FUNCNAME( "icvHoughCirclesGradient" );

    __BEGIN__;

    int x, y, i, j, center_count, nz_count;
    int rows, cols, arows, acols;
    int astep, *adata;
    float* ddata;
    CvSize asize;
    CvSeq *nz, *centers;
    float idp, dr;
    CvSeqReader reader;

    CV_CALL( edges = cvCreateMat( img->rows, img->cols, CV_8UC1 ));
    CV_CALL( cvCanny( img, edges, MAX(canny_threshold/2,1), canny_threshold, 3 ));

    CV_CALL( dx = cvCreateMat( img->rows, img->cols, CV_16SC1 ));
    CV_CALL( dy = cvCreateMat( img->rows, img->cols, CV_16SC1 ));
    CV_CALL( cvSobel( img, dx, 1, 0, 3 ));
    CV_CALL( cvSobel( img, dy, 0, 1, 3 ));

    if( dp < 1.f )
        dp = 1.f;
    idp = 1.f/dp;
    CV_CALL( accum = cvCreateMat( cvCeil(img->rows*idp)+2, cvCeil(img->cols*idp)+2, CV_32SC1 ));
    CV_CALL( cvZero(accum));
    asize = cvGetMatSize(accum);

    CV_CALL( storage = cvCreateMemStorage() );
    CV_CALL( nz = cvCreateSeq( CV_32SC2, sizeof(CvSeq), sizeof(CvPoint), storage ));
    CV_CALL( centers = cvCreateSeq( CV_32SC1, sizeof(CvSeq), sizeof(int), storage ));

    rows = img->rows;
    cols = img->cols;
    arows = accum->rows - 2;
    acols = accum->cols - 2;
    adata = accum->data.i;
    astep = accum->step/sizeof(adata[0]);

    for( y = 0; y < rows; y++ )
    {
        const uchar* edges_row = edges->data.ptr + y*edges->step;
        const short* dx_row = (const short*)(dx->data.ptr + y*dx->step);
        const short* dy_row = (const short*)(dy->data.ptr + y*dy->step);

        for( x = 0; x < cols; x++ )
        {
            float vx, vy;
            int sx, sy, x0, y0, x1, y1;
            CvPoint pt;
            
            vx = dx_row[x];
            vy = dy_row[x];

            if( !edges_row[x] || vx == 0 && vy == 0 )
                continue;
            
            if( fabs(vx) < fabs(vy) )
            {
                sx = cvRound(vx*ONE/fabs(vy));
                sy = vy < 0 ? -ONE : ONE;
            }
            else
            {
                assert( vx != 0 );
                sy = cvRound(vy*ONE/fabs(vx));
                sx = vx < 0 ? -ONE : ONE;
            }

            x0 = cvRound((x*idp)*ONE) + ONE + (ONE/2);
            y0 = cvRound((y*idp)*ONE) + ONE + (ONE/2);

            for( x1 = x0, y1 = y0;; x1 += sx, y1 += sy )
            {
                int x2 = x1 >> SHIFT, y2 = y1 >> SHIFT;
                if( (unsigned)x2 >= (unsigned)acols ||
                    (unsigned)y2 >= (unsigned)arows )
                    break;
                adata[y2*astep + x2]++;
            }

            sx = -sx; sy = -sy;
            for( x1 = x0 + sx, y1 = y0 + sy;; x1 += sx, y1 += sy )
            {
                int x2 = x1 >> SHIFT, y2 = y1 >> SHIFT;
                if( (unsigned)x2 >= (unsigned)acols ||
                    (unsigned)y2 >= (unsigned)arows )
                    break;
                adata[y2*astep + x2]++;
            }

            pt.x = x; pt.y = y;
            cvSeqPush( nz, &pt );
        }
    }

    nz_count = nz->total;
    if( !nz_count )
        EXIT;

    for( y = 1; y < arows - 1; y++ )
    {
        for( x = 1; x < acols - 1; x++ )
        {
            int base = y*(acols+2) + x;
            if( adata[base] > acc_threshold &&
                adata[base] > adata[base-1] && adata[base] > adata[base+1] &&
                adata[base] > adata[base-acols-2] && adata[base] > adata[base+acols+2] )
                cvSeqPush(centers, &base);
        }
    }

    center_count = centers->total;
    if( !center_count )
        EXIT;

    CV_CALL( sort_buf = (int*)cvAlloc( MAX(center_count,nz_count)*sizeof(sort_buf[0]) ));
    cvCvtSeqToArray( centers, sort_buf );

    icvHoughSortDescent32s( sort_buf, center_count, adata );
    cvClearSeq( centers );
    cvSeqPushMulti( centers, sort_buf, center_count );

    CV_CALL( dist_buf = cvCreateMat( 1, nz_count, CV_32FC1 ));
    ddata = dist_buf->data.fl;

    dr = dp;
    min_dist = MAX( min_dist, dp );
    min_dist *= min_dist;

    for( i = 0; i < centers->total; i++ )
    {
        int ofs = *(int*)cvGetSeqElem( centers, i );
        y = ofs/(acols+2) - 1;
        x = ofs - (y+1)*(acols+2) - 1;
        float cx = (float)(x*dp), cy = (float)(y*dp);
        int start_idx = nz_count - 1;
        float start_dist, prev_dist, dist_sum;
        float r_best = 0, c[3];
        int max_count = R_THRESH;

        for( j = 0; j < circles->total; j++ )
        {
            float* c = (float*)cvGetSeqElem( circles, j );
            if( (c[0] - cx)*(c[0] - cx) + (c[1] - cy)*(c[1] - cy) < min_dist )
                break;
        }

        if( j < circles->total )
            continue;
        
        cvStartReadSeq( nz, &reader );
        for( j = 0; j < nz_count; j++ )
        {
            CvPoint pt;
            float _dx, _dy;
            CV_READ_SEQ_ELEM( pt, reader );
            _dx = cx - pt.x; _dy = cy - pt.y;
            ddata[j] = _dx*_dx + _dy*_dy;
            sort_buf[j] = j;
        }

        cvPow( dist_buf, dist_buf, 0.5 );
        icvHoughSortDescent32s( sort_buf, nz_count, (int*)ddata );
        
        dist_sum = prev_dist = start_dist = ddata[sort_buf[nz_count-1]];
        for( j = nz_count - 2; j >= 0; j-- )
        {
            float d = ddata[sort_buf[j]];
            if( d - start_dist > dr )
            {
                if( start_idx - j >= max_count )
                {
                    r_best = ddata[sort_buf[(j + start_idx)/2]];
                    max_count = start_idx - j;
                }
                start_dist = d;
                start_idx = j;
                dist_sum = 0;
            }
            dist_sum += d;
            prev_dist = d;
        }

        if( max_count > R_THRESH )
        {
            c[0] = cx;
            c[1] = cy;
            c[2] = (float)r_best;
            cvSeqPush( circles, c );
            if( circles->total > circles_max )
                EXIT;
        }
    }

    __END__;

    cvReleaseMat( &dist_buf );
    cvFree( (void**)&sort_buf );
    cvReleaseMemStorage( &storage );
    cvReleaseMat( &edges );
    cvReleaseMat( &dx );
    cvReleaseMat( &dy );
    cvReleaseMat( &accum );
}

CV_IMPL CvSeq*
cvHoughCircles( CvArr* src_image, void* circle_storage,
                int method, double dp, double min_dist,
                double param1, double param2 )
{
    CvSeq* result = 0;

    CV_FUNCNAME( "cvHoughCircles" );

    __BEGIN__;
    
    CvMat stub, *img = (CvMat*)src_image;
    CvMat* mat = 0;
    CvSeq* circles = 0;
    CvSeq circles_header;
    CvSeqBlock circles_block;
    int circles_max = INT_MAX;
    int canny_threshold = cvRound(param1);
    int acc_threshold = cvRound(param2);
    CvSize size;

    CV_CALL( img = cvGetMat( img, &stub ));

    if( !CV_IS_MASK_ARR(img))
        CV_ERROR( CV_StsBadArg, "The source image must be 8-bit, single-channel" );

    if( !circle_storage )
        CV_ERROR( CV_StsNullPtr, "NULL destination" );

    if( dp <= 0 || min_dist <= 0 || canny_threshold <= 0 || acc_threshold <= 0 )
        CV_ERROR( CV_StsOutOfRange, "dp, min_dist, canny_threshold and acc_threshold must be all positive numbers" );

    if( CV_IS_STORAGE( circle_storage ))
    {
        CV_CALL( circles = cvCreateSeq( CV_32FC3, sizeof(CvSeq),
            sizeof(float)*3, (CvMemStorage*)circle_storage ));
    }
    else if( CV_IS_MAT( circle_storage ))
    {
        mat = (CvMat*)circle_storage;

        if( !CV_IS_MAT_CONT( mat->type ) || mat->rows != 1 && mat->cols != 1 ||
            CV_MAT_TYPE(mat->type) != CV_32FC3 )
            CV_ERROR( CV_StsBadArg,
            "The destination matrix should be continuous and have a single row or a single column" );

        CV_CALL( circles = cvMakeSeqHeaderForArray( CV_32FC3, sizeof(CvSeq), sizeof(float)*3,
                mat->data.ptr, mat->rows + mat->cols - 1, &circles_header, &circles_block ));
        circles_max = circles->total;
        CV_CALL( cvClearSeq( circles ));
    }
    else
    {
        CV_ERROR( CV_StsBadArg, "Destination is not CvMemStorage* nor CvMat*" );
    }

    size = cvGetMatSize(img);

    switch( method )
    {
    case CV_HOUGH_GRADIENT:
          CV_CALL( icvHoughCirclesGradient( img, (float)dp, (float)min_dist,
              canny_threshold, acc_threshold, circles, circles_max ));
          break;
    default:
        CV_ERROR( CV_StsBadArg, "Unrecognized method id" );
    }

    if( mat )
    {
        if( mat->cols > mat->rows )
            mat->cols = circles->total;
        else
            mat->rows = circles->total;
    }
    else
        result = circles;

    __END__;
    
    return result;    
}

/* End of file. */
