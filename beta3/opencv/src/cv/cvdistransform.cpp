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

#define MIN2(a, b) MIN((a),(b))
#define CALC_MIN(a, b) if((a) > (b)) (a) = (b)

#define _CV_DIST_SHIFT  18

static int
_MINn( const int *values, int n )
{
    int i;
    int ret;

    ret = values[0];

    for( i = 1; i < n; i++ )
    {
        int t = values[i];

        CALC_MIN( ret, t );
    }

    return ret;
}


static void
init_distances_8uC1( const uchar * pSrc, int srcStep, int *pDst, int dstStep, CvSize roiSize )
{
    int ri, ci;

    for( ri = 0; ri < roiSize.height; ri++, pSrc += srcStep, pDst += dstStep )
    {
        /* Convert the feature color to zero and non-feature -- to infinity */
        for( ci = 0; ci <= roiSize.width - 4; ci += 4 )
        {
            int t0 = pSrc[ci] ? (INT_MAX >> 1) : 0;
            int t1 = pSrc[ci + 1] ? (INT_MAX >> 1) : 0;
            int t2 = pSrc[ci + 2] ? (INT_MAX >> 1) : 0;
            int t3 = pSrc[ci + 3] ? (INT_MAX >> 1) : 0;

            pDst[ci] = t0;
            pDst[ci + 1] = t1;
            pDst[ci + 2] = t2;
            pDst[ci + 3] = t3;
        }

        for( ; ci < roiSize.width; ci++ )
        {
            pDst[ci] = pSrc[ci] > 0 ? (INT_MAX >> 1) : 0;
        }
    }
}


static void
icvCvtIntTofloat( int *idist, int step, CvSize roiSize, float scale )
{
    int ri, ci;

    for( ri = 0; ri < roiSize.height; ri++, idist += step )
    {
        for( ci = 0; ci <= roiSize.width - 4; ci += 4 )
        {
            float ft0 = scale * idist[ci];
            float ft1 = scale * idist[ci + 1];
            float ft2 = scale * idist[ci + 2];
            float ft3 = scale * idist[ci + 3];

            ((float *) idist)[ci] = (float) ft0;
            ((float *) idist)[ci + 1] = (float) ft1;
            ((float *) idist)[ci + 2] = (float) ft2;
            ((float *) idist)[ci + 3] = (float) ft3;
        }

        for( ; ci < roiSize.width; ci++ )
        {
            ((float *) idist)[ci] = (float) (scale * idist[ci]);
        }
    }
}

#define _IS_VALID(roiSize, row, column)\
    (row < 0 || column < 0 || row > roiSize.height - 1\
    || column > roiSize.width - 1) ? 0 : 1

#define VALIDATE_INPUT(img, img_step, dist, step, roiSize)\
    if( !img || !dist ) return CV_NULLPTR_ERR;\
    if( img_step < roiSize.width || step < roiSize.width*sizeof_float ||\
        (step & (sizeof_float-1)) != 0 || roiSize.width < 0 || roiSize.height < 0) \
        return CV_BADSIZE_ERR;


IPCVAPI_IMPL( CvStatus, icvDistanceTransform_3x3_8u32f_C1R,
              (const uchar * pSrc, int srcStep,
               float *pDst, int dstStep, CvSize roiSize, float *pMetrics) )
{
    int w = roiSize.width;
    int h = roiSize.height;
    int *idist = (int *) pDst;
    int ri;
    int t1, t2, t3, t4;
    int mask0, mask1;
    float scale;

    /* Test input data for validness */
    if( !pSrc || !pDst || !pMetrics )
        return CV_NULLPTR_ERR;
    if( roiSize.width < 0 || roiSize.height < 0 ||
        srcStep < roiSize.width || dstStep < roiSize.width * sizeof_float ||
        (dstStep & (sizeof_float - 1)) != 0 )
        return CV_BADSIZE_ERR;

    dstStep /= sizeof_float;

    mask0 = cvRound( pMetrics[0] * (1 << _CV_DIST_SHIFT) );
    mask1 = cvRound( pMetrics[1] * (1 << _CV_DIST_SHIFT) );
    scale = (float) (1. / (1 << _CV_DIST_SHIFT));

    init_distances_8uC1( pSrc, srcStep, idist, dstStep, roiSize );

    if( w == 1 )
    {
        int ri;

        /* Forward mask */
        /* ri = 0: do nothing */

        /* 0 < ri < h */
        for( ri = 1, idist += dstStep; ri < h; ri++, idist += dstStep )
        {
            t1 = idist[0];
            t2 = idist[-dstStep] + mask0;
            CALC_MIN( t1, t2 );
            idist[0] = t1;
        }

        /* Backward mask */
        /* ri = h - 1: do nothing */

        /* h - 1 > ri >= 0 */
        for( ri = h - 2, idist -= 2 * dstStep; ri >= 0; ri--, idist -= dstStep )
        {
            t1 = idist[0];
            t2 = idist[dstStep] + mask0;
            CALC_MIN( t1, t2 );
            idist[0] = t1;
        }

        idist += dstStep;
    }
    else
    {
        int ci;

/*  ____________ Forward mask _______________  */

        /* ci = 0: do nothing */

        /* 0 < ci < w */
        for( ci = 1; ci < w; ci++ )
        {
            t1 = idist[ci];
            t2 = idist[ci - 1] + mask0;
            CALC_MIN( t1, t2 );
            idist[ci] = t1;
        }

        if( h > 1 )
        {
            for( ri = 1, idist += dstStep; ri < h; ri++, idist += dstStep )
            {
                int *idist2 = idist - dstStep;

                /* ci = 0 */
                t1 = idist[0];
                t2 = idist2[0] + mask0;
                CALC_MIN( t1, t2 );
                t2 = idist2[1] + mask1;
                CALC_MIN( t1, t2 );
                idist[0] = t1;

                /* 0 < ci < w - 1 */
                for( ci = 1; ci < w - 1; ci++ )
                {
                    t1 = idist[ci];
                    t2 = idist[ci - 1] + mask0;
                    CALC_MIN( t1, t2 );
                    t3 = idist2[ci] + mask0;
                    t4 = idist2[ci - 1] + mask1;
                    CALC_MIN( t3, t4 );
                    t2 = idist2[ci + 1] + mask1;
                    CALC_MIN( t1, t3 );
                    CALC_MIN( t1, t2 );
                    idist[ci] = t1;
                }

                /* ci = w - 1 */
                t1 = idist[ci];

                t2 = idist[ci - 1] + mask0;
                CALC_MIN( t1, t2 );

                t2 = idist2[ci - 1] + mask1;
                CALC_MIN( t1, t2 );

                t2 = idist2[ci] + mask0;
                CALC_MIN( t1, t2 );

                idist[ci] = t1;
            }

            idist -= dstStep;
        }

/*  ____________ Backward mask _______________ */
        /* ci = w - 1: do nothing */

        /* w - 1 > ci >= 0 */
        for( ci = w - 2; ci >= 0; ci-- )
        {
            t1 = idist[ci];
            t2 = idist[ci + 1] + mask0;
            CALC_MIN( t1, t2 );
            idist[ci] = t1;
        }

        if( h > 1 )
        {
            /* 0 <= ri < h - 1 */
            for( ri = h - 2, idist -= dstStep; ri >= 0; ri--, idist -= dstStep )
            {
                int *idist2 = idist + dstStep;

                /* ci = w - 1 */
                t1 = idist[w - 1];
                t2 = idist2[w - 1] + mask0;
                CALC_MIN( t1, t2 );
                t2 = idist2[w - 2] + mask1;
                CALC_MIN( t1, t2 );
                idist[w - 1] = t1;

                /* 0 < ci < w - 1 */
                for( ci = w - 2; ci > 0; ci-- )
                {
                    t1 = idist[ci];
                    t2 = idist[ci + 1] + mask0;
                    CALC_MIN( t1, t2 );
                    t3 = idist2[ci] + mask0;
                    t4 = idist2[ci - 1] + mask1;
                    CALC_MIN( t3, t4 );
                    t2 = idist2[ci + 1] + mask1;
                    CALC_MIN( t1, t3 );
                    CALC_MIN( t1, t2 );
                    idist[ci] = t1;
                }

                /* ci = 0 */
                t1 = idist[0];

                t2 = idist[1] + mask0;
                CALC_MIN( t1, t2 );

                t2 = idist2[0] + mask0;
                CALC_MIN( t1, t2 );

                t2 = idist2[1] + mask1;
                CALC_MIN( t1, t2 );
                idist[0] = t1;
            }
            idist += dstStep;
        }
    }

    assert( idist == (int *) pDst );

    icvCvtIntTofloat( idist, dstStep, roiSize, scale );

    return CV_OK;
}


#define _F2I(i, f) (i) = (f);
#define _I2F(f, i) (f) = (i);

#define _POINT(row, column)  (((int*)pDst)[(row)*dstStep+(column)])
#define _ADD_POINT(row, column, m)\
    buffer[length++] = ((int*)pDst)[dstStep*(row) + (column)] + (m)


IPCVAPI_IMPL( CvStatus, icvDistanceTransform_5x5_8u32f_C1R,
              (const uchar * pSrc, int srcStep,
               float *pDst, int dstStep, CvSize roiSize, float *pMetrics) )
{
    int ri;                     /* Row index */
    int ci;                     /* Column index */
    int w = roiSize.width;
    int h = roiSize.height;
    int t1, t2;
    int *d;
    int buffer[13];
    int length;
    int offset;
    int mask0, mask1, mask2;
    float scale;

    /* Test input data for validness */
    if( !pSrc || !pDst || !pMetrics )
        return CV_NULLPTR_ERR;
    if( roiSize.width < 0 || roiSize.height < 0 ||
        srcStep < roiSize.width || dstStep < roiSize.width * sizeof_float ||
        (dstStep & (sizeof_float - 1)) != 0 )
        return CV_BADSIZE_ERR;

    dstStep /= sizeof_float;

    mask0 = cvRound( pMetrics[0] * (1 << _CV_DIST_SHIFT) );
    mask1 = cvRound( pMetrics[1] * (1 << _CV_DIST_SHIFT) );
    mask2 = cvRound( pMetrics[2] * (1 << _CV_DIST_SHIFT) );
    scale = (float) (1. / (1 << _CV_DIST_SHIFT));

    init_distances_8uC1( pSrc, srcStep, (int *) pDst, dstStep, roiSize );

    if( w < 4 || h < 4 )
    {
        /*  ____________ Forward mask _______________ */

        for( ri = 0; ri < h; ri++ )
        {
            for( ci = 0; ci < w; ci++ )
            {
                length = 1;
                offset = ri * dstStep + ci;
                d = (int *) pDst + offset;
                buffer[0] = d[0];

                if( ri == 0 )
                {
                    if( ci == 1 )
                    {
                        _ADD_POINT( ri, ci - 1, mask0 );
                    }
                    else if( ci > 1 )
                    {
                        _ADD_POINT( ri, ci - 1, mask0 );
                        _ADD_POINT( ri, ci - 2, 2 * mask0 );
                    }
                }
                else if( ri == 1 )
                {
                    if( ci == 0 )
                    {
                        _ADD_POINT( ri - 1, ci, mask0 );
                    }
                    else if( ci == 1 )
                    {
                        _ADD_POINT( ri, ci - 1, mask0 );
                        _ADD_POINT( ri - 1, ci, mask0 );
                        _ADD_POINT( ri - 1, ci - 1, mask1 );
                    }
                    else
                    {
                        _ADD_POINT( ri, ci - 1, mask0 );
                        _ADD_POINT( ri, ci - 2, 2 * mask0 );
                        _ADD_POINT( ri - 1, ci, mask0 );
                        _ADD_POINT( ri - 1, ci - 1, mask1 );
                        _ADD_POINT( ri - 1, ci - 2, mask2 );
                    }

                    if( ci == w - 2 )
                    {
                        _ADD_POINT( ri - 1, ci + 1, mask1 );
                    }
                    else if( ci < w - 2 )
                    {
                        _ADD_POINT( ri - 1, ci + 1, mask1 );
                        _ADD_POINT( ri - 1, ci + 2, mask2 );
                    }
                }
                else
                {
                    if( ci == 0 )
                    {
                        _ADD_POINT( ri - 1, ci, mask0 );
                        _ADD_POINT( ri - 2, ci, 2 * mask0 );
                    }
                    else if( ci == 1 )
                    {
                        _ADD_POINT( ri, ci - 1, mask0 );
                        _ADD_POINT( ri - 1, ci, mask0 );
                        _ADD_POINT( ri - 1, ci - 1, mask1 );
                        _ADD_POINT( ri - 2, ci, 2 * mask0 );
                        _ADD_POINT( ri - 2, ci - 1, mask2 );
                    }
                    else
                    {
                        _ADD_POINT( ri, ci - 1, mask0 );
                        _ADD_POINT( ri, ci - 2, 2 * mask0 );
                        _ADD_POINT( ri - 1, ci, mask0 );
                        _ADD_POINT( ri - 1, ci - 1, mask1 );
                        _ADD_POINT( ri - 1, ci - 2, mask2 );
                        _ADD_POINT( ri - 2, ci, 2 * mask0 );
                        _ADD_POINT( ri - 2, ci - 1, mask2 );
                        _ADD_POINT( ri - 2, ci - 2, 2 * mask1 );
                    }

                    if( ci == w - 2 )
                    {
                        _ADD_POINT( ri - 1, ci + 1, mask1 );
                        _ADD_POINT( ri - 2, ci + 1, mask2 );
                    }
                    else if( ci < w - 2 )
                    {
                        _ADD_POINT( ri - 1, ci + 1, mask1 );
                        _ADD_POINT( ri - 1, ci + 2, mask2 );
                        _ADD_POINT( ri - 2, ci + 1, mask2 );
                        _ADD_POINT( ri - 2, ci + 2, 2 * mask1 );
                    }
                }

                d[0] = _MINn( buffer, length );
            }
        }

        /*  ____________ Backward mask _______________ */

        for( ri = h - 1; ri >= 0; ri-- )
        {
            for( ci = w - 1; ci >= 0; ci-- )
            {
                length = 1;
                offset = ri * dstStep + ci;
                d = (int *) pDst + offset;
                buffer[0] = d[0];

                if( ri == h - 1 )
                {
                    if( ci == w - 2 )
                    {
                        _ADD_POINT( ri, ci + 1, mask0 );
                    }
                    else if( ci < w - 2 )
                    {
                        _ADD_POINT( ri, ci + 1, mask0 );
                        _ADD_POINT( ri, ci + 2, 2 * mask0 );
                    }
                }
                else if( ri == h - 2 )
                {
                    if( ci == w - 1 )
                    {
                        _ADD_POINT( ri + 1, ci, mask0 );
                    }
                    else if( ci == w - 2 )
                    {
                        _ADD_POINT( ri, ci + 1, mask0 );
                        _ADD_POINT( ri + 1, ci, mask0 );
                        _ADD_POINT( ri + 1, ci + 1, mask1 );
                    }
                    else
                    {
                        _ADD_POINT( ri, ci + 1, mask0 );
                        _ADD_POINT( ri, ci + 2, 2 * mask0 );
                        _ADD_POINT( ri + 1, ci, mask0 );
                        _ADD_POINT( ri + 1, ci + 1, mask1 );
                        _ADD_POINT( ri + 1, ci + 2, mask2 );
                    }

                    if( ci == 1 )
                    {
                        _ADD_POINT( ri + 1, ci - 1, mask1 );
                    }
                    else if( ci > 1 )
                    {
                        _ADD_POINT( ri + 1, ci - 1, mask1 );
                        _ADD_POINT( ri + 1, ci - 2, mask2 );
                    }
                }
                else
                {
                    if( ci == w - 1 )
                    {
                        _ADD_POINT( ri + 1, ci, mask0 );
                        _ADD_POINT( ri + 2, ci, 2 * mask0 );
                    }
                    else if( ci == w - 2 )
                    {
                        _ADD_POINT( ri, ci + 1, mask0 );
                        _ADD_POINT( ri + 1, ci, mask0 );
                        _ADD_POINT( ri + 1, ci + 1, mask1 );
                        _ADD_POINT( ri + 2, ci, 2 * mask0 );
                        _ADD_POINT( ri + 2, ci + 1, mask2 );
                    }
                    else
                    {
                        _ADD_POINT( ri, ci + 1, mask0 );
                        _ADD_POINT( ri, ci + 2, 2 * mask0 );
                        _ADD_POINT( ri + 1, ci, mask0 );
                        _ADD_POINT( ri + 1, ci + 1, mask1 );
                        _ADD_POINT( ri + 1, ci + 2, mask2 );
                        _ADD_POINT( ri + 2, ci, 2 * mask0 );
                        _ADD_POINT( ri + 2, ci + 1, mask2 );
                        _ADD_POINT( ri + 2, ci + 2, 2 * mask1 );
                    }

                    if( ci == 1 )
                    {
                        _ADD_POINT( ri + 1, ci - 1, mask1 );
                        _ADD_POINT( ri + 2, ci - 1, mask2 );
                    }
                    else if( ci > 1 )
                    {
                        _ADD_POINT( ri + 1, ci - 1, mask1 );
                        _ADD_POINT( ri + 1, ci - 2, mask2 );
                        _ADD_POINT( ri + 2, ci - 1, mask2 );
                        _ADD_POINT( ri + 2, ci - 2, 2 * mask1 );
                    }
                }

                d[0] = _MINn( buffer, length );
            }
        }

        goto func_exit;
    }

/*  ____________ Forward mask _______________ */

    /* ri = 0, ci = 0: do nothing */

    /* ri = 0, ci = 1 */
    t1 = *(int *) &_POINT( 0, 1 );
    _F2I( t2, _POINT( 0, 0 ) + mask0 );
    CALC_MIN( t1, t2 );
    _I2F( _POINT( 0, 1 ), t1 );

    /* ri = 0, 1 < ci < w */
    for( ci = 2; ci < w; ci++ )
    {
        t1 = *(int *) &_POINT( 0, ci );
        _F2I( t2, _POINT( 0, ci - 1 ) + mask0 );
        CALC_MIN( t1, t2 );
        _I2F( _POINT( 0, ci ), t1 );
    }

    /* ri = 1, ci = 0 */
    t1 = *(int *) &_POINT( 1, 0 );
    _F2I( t2, _POINT( 0, 0 ) + mask0 );
    CALC_MIN(t1, t2);
	_F2I( t2, _POINT( 0, 1 ) + mask1 );
    CALC_MIN( t1, t2 );
    _F2I( t2, _POINT( 0, 2 ) + mask2 );
    CALC_MIN( t1, t2 );
    _I2F( _POINT( 1, 0 ), t1 );

    /* ri = 1, ci = 1 */
    t1 = *(int *) &_POINT( 1, 1 );
    _F2I( t2, _POINT( 0, 1 ) + mask0 );
    CALC_MIN( t1, t2 );
    _F2I( t2, _POINT( 0, 2 ) + mask1 );
    CALC_MIN( t1, t2 );
    _F2I( t2, _POINT( 0, 3 ) + mask2 );
    CALC_MIN( t1, t2 );
    _F2I( t2, _POINT( 0, 0 ) + mask1 );
    CALC_MIN( t1, t2 );
    _F2I( t2, _POINT( 1, 0 ) + mask0 );
    CALC_MIN( t1, t2 );
    _I2F( _POINT( 1, 1 ), t1 );


    /* ri = 1, 1 < ci < w - 2 */
    for( ci = 2; ci < w - 2; ci++ )
    {
        t1 = *(int *) &_POINT( 1, ci );
        _F2I( t2, _POINT( 0, ci ) + mask0 );
        CALC_MIN( t1, t2 );
        _F2I( t2, _POINT( 0, ci + 1 ) + mask1 );
        CALC_MIN( t1, t2 );
        _F2I( t2, _POINT( 0, ci + 2 ) + mask2 );
        CALC_MIN( t1, t2 );
        _F2I( t2, _POINT( 0, ci - 1 ) + mask1 );
        CALC_MIN( t1, t2 );
        _F2I( t2, _POINT( 1, ci - 1 ) + mask0 );
        CALC_MIN( t1, t2 );
        _F2I( t2, _POINT( 0, ci - 2 ) + mask2 );
        CALC_MIN( t1, t2 );
        _I2F( _POINT( 1, ci ), t1 );
    }

    /* ri = 1, ci = w - 2 */
    t1 = *(int *) &_POINT( 1, w - 2 );
    _F2I( t2, _POINT( 0, w - 2 ) + mask0 );
    CALC_MIN( t1, t2 );
    _F2I( t2, _POINT( 0, w - 1 ) + mask1 );
    CALC_MIN( t1, t2 );
    _F2I( t2, _POINT( 0, w - 3 ) + mask1 );
    CALC_MIN( t1, t2 );
    _F2I( t2, _POINT( 1, w - 3 ) + mask0 );
    CALC_MIN( t1, t2 );
    _F2I( t2, _POINT( 0, w - 4 ) + mask2 );
    CALC_MIN( t1, t2 );
    _I2F( _POINT( 1, w - 2 ), t1 );

    /* ri = 1, ci = w - 1 */
    t1 = *(int *) &_POINT( 1, w - 1 );
    _F2I( t2, _POINT( 0, w - 1 ) + mask0 );
    CALC_MIN( t1, t2 );
    _F2I( t2, _POINT( 0, w - 2 ) + mask1 );
    CALC_MIN( t1, t2 );
    _F2I( t2, _POINT( 1, w - 2 ) + mask0 );
    CALC_MIN( t1, t2 );
    _F2I( t2, _POINT( 0, w - 3 ) + mask2 );
    CALC_MIN( t1, t2 );
    _I2F( _POINT( 1, w - 1 ), t1 )
        /* 1 < ri < h */
        for( ri = 2; ri < h; ri++ )
    {
        /* ci = 0 */
        t1 = *(int *) &_POINT( ri, 0 );
        _F2I( t2, _POINT( ri - 1, 0 ) + mask0 );
        CALC_MIN( t1, t2 );
        _F2I( t2, _POINT( ri - 1, 1 ) + mask1 );
        CALC_MIN( t1, t2 );
        _F2I( t2, _POINT( ri - 1, 2 ) + mask2 );
        CALC_MIN( t1, t2 );
        _F2I( t2, _POINT( ri - 2, 1 ) + mask2 );
        CALC_MIN( t1, t2 );
        _I2F( _POINT( ri, 0 ), t1 );

        /* ci = 1 */
        t1 = *(int *) &_POINT( ri, 1 );
        _F2I( t2, _POINT( ri - 1, 1 ) + mask0 );
        CALC_MIN( t1, t2 );
        _F2I( t2, _POINT( ri - 1, 2 ) + mask1 );
        CALC_MIN( t1, t2 );
        _F2I( t2, _POINT( ri - 1, 3 ) + mask2 );
        CALC_MIN( t1, t2 );
        _F2I( t2, _POINT( ri - 2, 2 ) + mask2 );
        CALC_MIN( t1, t2 );
        _F2I( t2, _POINT( ri, 0 ) + mask0 );
        CALC_MIN( t1, t2 );
        _F2I( t2, _POINT( ri - 1, 0 ) + mask1 );
        CALC_MIN( t1, t2 );
        _F2I( t2, _POINT( ri - 2, 0 ) + mask2 );
        CALC_MIN( t1, t2 );
        _I2F( _POINT( ri, 1 ), t1 );


        /* 1 < ci < w - 2 */
        for( ci = 2; ci < w - 2; ci++ )
        {
            t1 = *(int *) &_POINT( ri, ci );
            _F2I( t2, _POINT( ri, ci - 1 ) + mask0 );
            CALC_MIN( t1, t2 );
            _F2I( t2, _POINT( ri - 1, ci ) + mask0 );
            CALC_MIN( t1, t2 );
            _F2I( t2, _POINT( ri - 1, ci + 1 ) + mask1 );
            CALC_MIN( t1, t2 );
            _F2I( t2, _POINT( ri - 1, ci + 2 ) + mask2 );
            CALC_MIN( t1, t2 );
            _F2I( t2, _POINT( ri - 2, ci + 1 ) + mask2 );
            CALC_MIN( t1, t2 );
            _F2I( t2, _POINT( ri - 1, ci - 1 ) + mask1 );
            CALC_MIN( t1, t2 );
            _F2I( t2, _POINT( ri - 1, ci - 2 ) + mask2 );
            CALC_MIN( t1, t2 );
            _F2I( t2, _POINT( ri - 2, ci - 1 ) + mask2 );
            CALC_MIN( t1, t2 );
        _I2F( _POINT( ri, ci ), t1 )}


        /* ci = w - 2 */
        t1 = *(int *) &_POINT( ri, w - 2 );
        _F2I( t2, _POINT( ri - 1, w - 2 ) + mask0 );
        CALC_MIN( t1, t2 );
        _F2I( t2, _POINT( ri - 1, w - 1 ) + mask1 );
        CALC_MIN( t1, t2 );
        _F2I( t2, _POINT( ri - 2, w - 1 ) + mask2 );
        CALC_MIN( t1, t2 );
        _F2I( t2, _POINT( ri, w - 2 ) + mask0 );
        CALC_MIN( t1, t2 );
        _F2I( t2, _POINT( ri - 1, w - 3 ) + mask1 );
        CALC_MIN( t1, t2 );
        _F2I( t2, _POINT( ri - 1, w - 4 ) + mask2 );
        CALC_MIN( t1, t2 );
        _F2I( t2, _POINT( ri - 2, w - 3 ) + mask2 );
        CALC_MIN( t1, t2 );
        _I2F( _POINT( ri, w - 2 ), t1 );


        /* ci = w - 1 */
        t1 = *(int *) &_POINT( ri, w - 1 );
        _F2I( t2, _POINT( ri - 1, w - 1 ) + mask0 );
        CALC_MIN( t1, t2 );
        _F2I( t2, _POINT( ri, w - 2 ) + mask0 );
        CALC_MIN( t1, t2 );
        _F2I( t2, _POINT( ri - 1, w - 2 ) + mask1 );
        CALC_MIN( t1, t2 );
        _F2I( t2, _POINT( ri - 1, w - 3 ) + mask2 );
        CALC_MIN( t1, t2 );
        _F2I( t2, _POINT( ri - 2, w - 2 ) + mask2 );
        CALC_MIN( t1, t2 );
        _I2F( _POINT( ri, w - 1 ), t1 );
    }


/*  ____________ Backward mask _______________ */

    /* ri = h - 1, ci = w - 1: do nothing */

    /* ri = h - 1, ci = w - 2 */
    t1 = *(int *) &_POINT( h - 1, w - 2 );
    _F2I( t2, _POINT( h - 1, w - 1 ) + mask0 );
    CALC_MIN( t1, t2 );
    _I2F( _POINT( h - 1, w - 2 ), t1 );

    /* ri = h - 1, w - 2 > ci >= 0 */
    for( ci = w - 3; ci >= 0; ci-- )
    {
        t1 = *(int *) &_POINT( h - 1, ci );
        _F2I( t2, _POINT( h - 1, ci + 1 ) + mask0 );
        CALC_MIN( t1, t2 );
        _I2F( _POINT( h - 1, ci ), t1 );
    }


    /* ri = h - 2, ci = w - 1 */
    t1 = *(int *) &_POINT( h - 2, w - 1 );
    _F2I( t2, _POINT( h - 1, w - 1 ) + mask0 );
    CALC_MIN( t1, t2 );
    _F2I( t2, _POINT( h - 1, w - 2 ) + mask1 );
    CALC_MIN( t1, t2 );
    _F2I( t2, _POINT( h - 1, w - 3 ) + mask2 );
    CALC_MIN( t1, t2 );
    _I2F( _POINT( h - 2, w - 1 ), t1 );

    /* ri = h - 2, ci = w - 2 */
    t1 = *(int *) &_POINT( h - 2, w - 2 );
    _F2I( t2, _POINT( h - 1, w - 2 ) + mask0 );
    CALC_MIN( t1, t2 );
    _F2I( t2, _POINT( h - 1, w - 3 ) + mask1 );
    CALC_MIN( t1, t2 );
    _F2I( t2, _POINT( h - 1, w - 4 ) + mask2 );
    CALC_MIN( t1, t2 );
    _F2I( t2, _POINT( h - 1, w - 1 ) + mask1 );
    CALC_MIN( t1, t2 );
    _F2I( t2, _POINT( h - 2, w - 1 ) + mask0 );
    CALC_MIN( t1, t2 );
    _I2F( _POINT( h - 2, w - 2 ), t1 );

    /* ri = h - 2, w - 2 > ci > 1 */
    for( ci = w - 3; ci > 1; ci-- )
    {
        t1 = *(int *) &_POINT( h - 2, ci );
        _F2I( t2, _POINT( h - 2, ci + 1 ) + mask0 );
        CALC_MIN( t1, t2 );
        _F2I( t2, _POINT( h - 1, ci ) + mask0 );
        CALC_MIN( t1, t2 );
        _F2I( t2, _POINT( h - 1, ci + 1 ) + mask1 );
        CALC_MIN( t1, t2 );
        _F2I( t2, _POINT( h - 1, ci + 2 ) + mask2 );
        CALC_MIN( t1, t2 );
        _F2I( t2, _POINT( h - 1, ci - 1 ) + mask1 );
        CALC_MIN( t1, t2 );
        _F2I( t2, _POINT( h - 1, ci - 2 ) + mask2 );
        CALC_MIN( t1, t2 );
        _I2F( _POINT( h - 2, ci ), t1 );
    }

    /* ri = h - 2, ci = 1 */
    t1 = *(int *) &_POINT( h - 2, 1 );
    _F2I( t2, _POINT( h - 2, 2 ) + mask0 );
    CALC_MIN( t1, t2 );
    _F2I( t2, _POINT( h - 1, 2 ) + mask1 );
    CALC_MIN( t1, t2 );
    _F2I( t2, _POINT( h - 1, 3 ) + mask2 );
    CALC_MIN( t1, t2 );
    _F2I( t2, _POINT( h - 1, 1 ) + mask0 );
    CALC_MIN( t1, t2 );
    _F2I( t2, _POINT( h - 1, 0 ) + mask1 );
    CALC_MIN( t1, t2 );
    _I2F( _POINT( h - 2, 1 ), t1 );

    /* ri = h - 2, ci = 0 */
    t1 = *(int *) &_POINT( h - 2, 0 );
    _F2I( t2, _POINT( h - 2, 1 ) + mask0 );
    CALC_MIN( t1, t2 );
    _F2I( t2, _POINT( h - 1, 1 ) + mask1 );
    CALC_MIN( t1, t2 );
    _F2I( t2, _POINT( h - 1, 2 ) + mask2 );
    CALC_MIN( t1, t2 );
    _F2I( t2, _POINT( h - 1, 0 ) + mask0 );
    CALC_MIN( t1, t2 );
    _I2F( _POINT( h - 2, 0 ), t1 );


    /* h - 2 > ri >= 0 */
    for( ri = h - 3; ri >= 0; ri-- )
    {
        /* ci = w - 1 */
        t1 = *(int *) &_POINT( ri, w - 1 );
        _F2I( t2, _POINT( ri + 1, w - 1 ) + mask0 );
        CALC_MIN( t1, t2 );
        _F2I( t2, _POINT( ri + 1, w - 2 ) + mask1 );
        CALC_MIN( t1, t2 );
        _F2I( t2, _POINT( ri + 2, w - 2 ) + mask2 );
        CALC_MIN( t1, t2 );
        _F2I( t2, _POINT( ri + 1, w - 3 ) + mask2 );
        CALC_MIN( t1, t2 );
        _I2F( _POINT( ri, w - 1 ), t1 );

        /* ci = w - 2 */
        t1 = *(int *) &_POINT( ri, w - 2 );
        _F2I( t2, _POINT( ri, w - 1 ) + mask0 );
        CALC_MIN( t1, t2 );
        _F2I( t2, _POINT( ri + 1, w - 1 ) + mask1 );
        CALC_MIN( t1, t2 );
        _F2I( t2, _POINT( ri + 2, w - 1 ) + mask2 );
        CALC_MIN( t1, t2 );
        _F2I( t2, _POINT( ri + 1, w - 2 ) + mask0 );
        CALC_MIN( t1, t2 );
        _F2I( t2, _POINT( ri + 1, w - 3 ) + mask1 );
        CALC_MIN( t1, t2 );
        _F2I( t2, _POINT( ri + 2, w - 3 ) + mask2 );
        CALC_MIN( t1, t2 );
        _F2I( t2, _POINT( ri + 1, w - 4 ) + mask2 );
        CALC_MIN( t1, t2 );
        _I2F( _POINT( ri, w - 2 ), t1 );

        /* w - 2 > ci > 1 */
        for( ci = w - 3; ci > 1; ci-- )
        {
            t1 = *(int *) &_POINT( ri, ci );
            _F2I( t2, _POINT( ri, ci + 1 ) + mask0 );
            CALC_MIN( t1, t2 );
            _F2I( t2, _POINT( ri + 1, ci ) + mask0 );
            CALC_MIN( t1, t2 );
            _F2I( t2, _POINT( ri + 1, ci + 1 ) + mask1 );
            CALC_MIN( t1, t2 );
            _F2I( t2, _POINT( ri + 1, ci + 2 ) + mask2 );
            CALC_MIN( t1, t2 );
            _F2I( t2, _POINT( ri + 2, ci + 1 ) + mask2 );
            CALC_MIN( t1, t2 );
            _F2I( t2, _POINT( ri + 1, ci - 1 ) + mask1 );
            CALC_MIN( t1, t2 );
            _F2I( t2, _POINT( ri + 1, ci - 2 ) + mask2 );
            CALC_MIN( t1, t2 );
            _F2I( t2, _POINT( ri + 2, ci - 1 ) + mask2 );
            CALC_MIN( t1, t2 );
            _I2F( _POINT( ri, ci ), t1 );
        }

        /* ci = 1 */
        t1 = *(int *) &_POINT( ri, 1 );
        _F2I( t2, _POINT( ri, 2 ) + mask0 );
        CALC_MIN( t1, t2 );
        _F2I( t2, _POINT( ri + 1, 1 ) + mask0 );
        CALC_MIN( t1, t2 );
        _F2I( t2, _POINT( ri + 1, 2 ) + mask1 );
        CALC_MIN( t1, t2 );
        _F2I( t2, _POINT( ri + 1, 3 ) + mask2 );
        CALC_MIN( t1, t2 );
        _F2I( t2, _POINT( ri + 2, 2 ) + mask2 );
        CALC_MIN( t1, t2 );
        _F2I( t2, _POINT( ri + 1, 0 ) + mask1 );
        CALC_MIN( t1, t2 );
        _F2I( t2, _POINT( ri + 2, 0 ) + mask2 );
        CALC_MIN( t1, t2 );
        _I2F( _POINT( ri, 1 ), t1 );

        /* ci = 0 */
        t1 = *(int *) &_POINT( ri, 0 );
        _F2I( t2, _POINT( ri, 1 ) + mask0 );
        CALC_MIN( t1, t2 );
        _F2I( t2, _POINT( ri + 1, 0 ) + mask0 );
        CALC_MIN( t1, t2 );
        _F2I( t2, _POINT( ri + 1, 1 ) + mask1 );
        CALC_MIN( t1, t2 );
        _F2I( t2, _POINT( ri + 1, 2 ) + mask2 );
        CALC_MIN( t1, t2 );
        _F2I( t2, _POINT( ri + 2, 1 ) + mask2 );
        CALC_MIN( t1, t2 );
        _I2F( _POINT( ri, 0 ), t1 );
    }

  func_exit:

    icvCvtIntTofloat( (int *) pDst, dstStep, roiSize, scale );
    return CV_OK;
}


IPCVAPI_IMPL( CvStatus, icvGetDistanceTransformMask, (int maskType, float *pMetrics) )
{
    if( !pMetrics )
        return CV_NULLPTR_ERR;

    switch (maskType)
    {
    case 30:
        pMetrics[0] = 1.0f;
        pMetrics[1] = 1.0f;
        break;

    case 31:
        pMetrics[0] = 1.0f;
        pMetrics[1] = 2.0f;
        break;

    case 32:
        pMetrics[0] = 0.955f;
        pMetrics[1] = 1.3693f;
        break;

    case 52:
        pMetrics[0] = 1.0f;
        pMetrics[1] = 1.4f;
        pMetrics[2] = 2.1969f;
        break;
    default:
        return CV_BADRANGE_ERR;
    }
    return CV_OK;
}


/* Wrapper function for distance transform group */
CV_IMPL void
cvDistTransform( const void* srcarr, void* dstarr,
                 CvDisType distType, int maskSize,
                 const float *mask )
{
    CV_FUNCNAME( "cvDistTransform" );

    __BEGIN__;

    float _mask[5];
    CvMat srcstub, *src = (CvMat*)srcarr;
    CvMat dststub, *dst = (CvMat*)dstarr;

    CV_CALL( src = cvGetMat( src, &srcstub ));
    CV_CALL( dst = cvGetMat( dst, &dststub ));

    if( !CV_IS_MASK_ARR( src ) || CV_MAT_TYPE( dst->type ) != CV_32FC1 )
        CV_ERROR( CV_StsUnsupportedFormat, "" );

    if( !CV_ARE_SIZES_EQ( src, dst ))
        CV_ERROR( CV_StsUnmatchedSizes, "" );

    if( maskSize != CV_DIST_MASK_3 && maskSize != CV_DIST_MASK_5 )
        CV_ERROR( CV_StsBadSize, "" );

    if( distType == CV_DIST_C || distType == CV_DIST_L1 )
        maskSize = CV_DIST_MASK_3;

    if( distType == CV_DIST_C || distType == CV_DIST_L1 || distType == CV_DIST_L2 )
    {
        IPPI_CALL( icvGetDistanceTransformMask( distType == CV_DIST_C ? 30 :
                                                distType == CV_DIST_L1 ? 31 :
                                                maskSize*10 + 2, _mask ));
    }
    else if( distType == CV_DIST_USER )
    {
        if( !mask )
            CV_ERROR( CV_StsNullPtr, "" );

        memcpy( _mask, mask, (maskSize/2 + 1)*sizeof(float));
    }

    if( maskSize == CV_DIST_MASK_3 )
    {
        IPPI_CALL( icvDistanceTransform_3x3_8u32f_C1R
                   ( src->data.ptr, src->step,
                     dst->data.fl, dst->step,
                     icvGetMatSize(src), _mask ));
    }
    else
    {
        IPPI_CALL( icvDistanceTransform_5x5_8u32f_C1R
                   ( src->data.ptr, src->step,
                     dst->data.fl, dst->step,
                     icvGetMatSize(src), _mask ));
    }

    __END__;
}

