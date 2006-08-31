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
#include <float.h>
#include <stdio.h>

static void
intersect( CvPoint2D32f pt, CvSize win_size, CvSize img_size,
           CvPoint* min_pt, CvPoint* max_pt )
{
    CvPoint ipt;

    ipt.x = cvFloor( pt.x );
    ipt.y = cvFloor( pt.y );

    ipt.x -= win_size.width;
    ipt.y -= win_size.height;

    win_size.width = win_size.width * 2 + 1;
    win_size.height = win_size.height * 2 + 1;

    min_pt->x = MAX( 0, -ipt.x );
    min_pt->y = MAX( 0, -ipt.y );
    max_pt->x = MIN( win_size.width, img_size.width - ipt.x );
    max_pt->y = MIN( win_size.height, img_size.height - ipt.y );
}


static CvStatus
icvInitPyramidalAlgorithm( const uchar * imgA, const uchar * imgB,
                           int imgStep, CvSize imgSize,
                           uchar * pyrA, uchar * pyrB,
                           int level,
                           CvTermCriteria * criteria,
                           int max_iters, int flags,
                           uchar *** imgI, uchar *** imgJ,
                           int **step, CvSize** size,
                           double **scale, uchar ** buffer )
{
    int pyrBytes, bufferBytes = 0;
    int level1 = level + 1;

    int i;
    CvSize levelSize;

    *buffer = 0;
    *imgI = *imgJ = 0;
    *step = 0;
    *scale = 0;
    *size = 0;

    /* check input arguments */
    if( !imgA || !imgB )
        return CV_NULLPTR_ERR;

    if( (flags & CV_LKFLOW_PYR_A_READY) != 0 && !pyrA ||
        (flags & CV_LKFLOW_PYR_B_READY) != 0 && !pyrB )
        return CV_BADFLAG_ERR;

    if( level < 0 )
        return CV_BADRANGE_ERR;

    switch (criteria->type)
    {
    case CV_TERMCRIT_ITER:
        criteria->epsilon = 0.f;
        break;
    case CV_TERMCRIT_EPS:
        criteria->max_iter = max_iters;
        break;
    case CV_TERMCRIT_ITER | CV_TERMCRIT_EPS:
        break;
    default:
        assert( 0 );
        return CV_BADFLAG_ERR;
    }

    /* compare squared values */
    criteria->epsilon *= criteria->epsilon;

    /* set pointers and step for every level */
    pyrBytes = 0;

#define ALIGN 8

    levelSize = imgSize;

    for( i = 1; i < level1; i++ )
    {
        levelSize.width = (levelSize.width + 1) >> 1;
        levelSize.height = (levelSize.height + 1) >> 1;

        int tstep = cvAlign(levelSize.width,ALIGN) * sizeof( imgA[0] );
        pyrBytes += tstep * levelSize.height;
    }

    assert( pyrBytes <= imgSize.width * imgSize.height * (int) sizeof( imgA[0] ) * 4 / 3 );

    /* buffer_size = <size for patches> + <size for pyramids> */
    bufferBytes = (int)((level1 >= 0) * ((pyrA == 0) + (pyrB == 0)) * pyrBytes +
        (sizeof( imgI[0][0] ) * 2 + sizeof( step[0][0] ) +
         sizeof(size[0][0]) + sizeof( scale[0][0] )) * level1);

    *buffer = (uchar *)cvAlloc( bufferBytes );
    if( !buffer[0] )
        return CV_OUTOFMEM_ERR;

    *imgI = (uchar **) buffer[0];
    *imgJ = *imgI + level1;
    *step = (int *) (*imgJ + level1);
    *scale = (double *) (*step + level1);
    *size = (CvSize *)(*scale + level1);

    imgI[0][0] = (uchar*)imgA;
    imgJ[0][0] = (uchar*)imgB;
    step[0][0] = imgStep;
    scale[0][0] = 1;
    size[0][0] = imgSize;

    if( level > 0 )
    {
        uchar *bufPtr = (uchar *) (*size + level1);
        uchar *ptrA = pyrA;
        uchar *ptrB = pyrB;

        if( !ptrA )
        {
            ptrA = bufPtr;
            bufPtr += pyrBytes;
        }

        if( !ptrB )
            ptrB = bufPtr;

        levelSize = imgSize;

        /* build pyramids for both frames */
        for( i = 1; i <= level; i++ )
        {
            int levelBytes;
            CvMat prev_level, next_level;

            levelSize.width = (levelSize.width + 1) >> 1;
            levelSize.height = (levelSize.height + 1) >> 1;

            size[0][i] = levelSize;
            step[0][i] = cvAlign( levelSize.width, ALIGN ) * sizeof( imgA[0] );
            scale[0][i] = scale[0][i - 1] * 0.5;

            levelBytes = step[0][i] * levelSize.height;
            imgI[0][i] = (uchar *) ptrA;
            ptrA += levelBytes;

            if( !(flags & CV_LKFLOW_PYR_A_READY) )
            {
                prev_level = cvMat( size[0][i-1].height, size[0][i-1].width, CV_8UC1 );
                next_level = cvMat( size[0][i].height, size[0][i].width, CV_8UC1 );
                cvSetData( &prev_level, imgI[0][i-1], step[0][i-1] );
                cvSetData( &next_level, imgI[0][i], step[0][i] );
                cvPyrDown( &prev_level, &next_level );
            }

            imgJ[0][i] = (uchar *) ptrB;
            ptrB += levelBytes;

            if( !(flags & CV_LKFLOW_PYR_B_READY) )
            {
                prev_level = cvMat( size[0][i-1].height, size[0][i-1].width, CV_8UC1 );
                next_level = cvMat( size[0][i].height, size[0][i].width, CV_8UC1 );
                cvSetData( &prev_level, imgJ[0][i-1], step[0][i-1] );
                cvSetData( &next_level, imgJ[0][i], step[0][i] );
                cvPyrDown( &prev_level, &next_level );
            }
        }
    }

    return CV_OK;
}


/* compute dI/dx and dI/dy */
static void
icvCalcIxIy_32f( const float* src, int src_step, float* dstX, float* dstY, int dst_step,
                 CvSize src_size, const float* smooth_k, float* buffer0 )
{
    int src_width = src_size.width, dst_width = src_size.width-2;
    int x, height = src_size.height - 2;
    float* buffer1 = buffer0 + src_width;

    src_step /= sizeof(src[0]);
    dst_step /= sizeof(dstX[0]);

    for( ; height--; src += src_step, dstX += dst_step, dstY += dst_step )
    {
        const float* src2 = src + src_step;
        const float* src3 = src + src_step*2;

        for( x = 0; x < src_width; x++ )
        {
            float t0 = (src3[x] + src[x])*smooth_k[0] + src2[x]*smooth_k[1];
            float t1 = src3[x] - src[x];
            buffer0[x] = t0; buffer1[x] = t1;
        }

        for( x = 0; x < dst_width; x++ )
        {
            float t0 = buffer0[x+2] - buffer0[x];
            float t1 = (buffer1[x] + buffer1[x+2])*smooth_k[0] + buffer1[x+1]*smooth_k[1];
            dstX[x] = t0; dstY[x] = t1;
        }
    }
}

icvOpticalFlowPyrLKInitAlloc_8u_C1R_t icvOpticalFlowPyrLKInitAlloc_8u_C1R_p = 0;
icvOpticalFlowPyrLKFree_8u_C1R_t icvOpticalFlowPyrLKFree_8u_C1R_p = 0;
icvOpticalFlowPyrLK_8u_C1R_t icvOpticalFlowPyrLK_8u_C1R_p = 0;

static CvStatus
icvCalcOpticalFlowPyrLK_8uC1R( const uchar* imgA, const uchar* imgB,
                               int imgStep, CvSize imgSize,
                               uchar* pyrA, uchar* pyrB,
                               const CvPoint2D32f* featuresA,
                               CvPoint2D32f* featuresB,
                               int count, CvSize winSize,
                               int level, char* status,
                               float *error, CvTermCriteria criteria, int flags )
{
#define MAX_SIZE  100
#define MAX_LEVEL 10
#define MAX_ITERS 100

    static const float smoothKernel[] = { 0.09375, 0.3125, 0.09375 };  /* 3/32, 10/32, 3/32 */

    uchar *pyrBuffer = 0;
    uchar *buffer = 0;
    int bufferBytes = 0;
    float* _error = 0;
    char* _status = 0;

    uchar **imgI = 0;
    uchar **imgJ = 0;
    int *step = 0;
    double *scale = 0;
    CvSize* size = 0;

    int threadCount = cvGetNumThreads();
    float* _patchI[CV_MAX_THREADS];
    float* _patchJ[CV_MAX_THREADS];
    float* _Ix[CV_MAX_THREADS];
    float* _Iy[CV_MAX_THREADS];

    int i, l;

    CvSize patchSize = cvSize( winSize.width * 2 + 1, winSize.height * 2 + 1 );
    int patchLen = patchSize.width * patchSize.height;
    int srcPatchLen = (patchSize.width + 2)*(patchSize.height + 2);

    CvStatus result = CV_OK;
    void* ipp_optflow_state = 0;

    /* check input arguments */
    if( !featuresA || !featuresB )
        return CV_NULLPTR_ERR;
    if( winSize.width <= 1 || winSize.height <= 1 )
        return CV_BADSIZE_ERR;

    if( (flags & ~7) != 0 )
        return CV_BADFLAG_ERR;
    if( count <= 0 )
        return CV_BADRANGE_ERR;

    for( i = 0; i < threadCount; i++ )
        _patchI[i] = _patchJ[i] = _Ix[i] = _Iy[i] = 0;

    result = icvInitPyramidalAlgorithm( imgA, imgB, imgStep, imgSize,
                                        pyrA, pyrB, level, &criteria, MAX_ITERS, flags,
                                        &imgI, &imgJ, &step, &size, &scale, &pyrBuffer );

    if( result < 0 )
        goto func_exit;

    if( !status )
    {
        _status = (char*)cvAlloc( count*sizeof(_status[0]) );
        if( !_status )
        {
            result = CV_OUTOFMEM_ERR;
            goto func_exit;
        }
        status = _status;
    }

#if 0
    if( icvOpticalFlowPyrLKInitAlloc_8u_C1R_p &&
        icvOpticalFlowPyrLKFree_8u_C1R_p &&
        icvOpticalFlowPyrLK_8u_C1R_p &&
        winSize.width == winSize.height &&
        icvOpticalFlowPyrLKInitAlloc_8u_C1R_p( &ipp_optflow_state, imgSize,
                                               winSize.width*2+1, cvAlgHintAccurate ) >= 0 )
    {
        CvPyramid ipp_pyrA, ipp_pyrB;
        static const double rate[] = { 1, 0.5, 0.25, 0.125, 0.0625, 0.03125, 0.015625, 0.0078125,
                                       0.00390625, 0.001953125, 0.0009765625, 0.00048828125, 0.000244140625,
                                       0.0001220703125 };
        // initialize pyramid structures
        assert( level < 14 );
        ipp_pyrA.ptr = imgI;
        ipp_pyrB.ptr = imgJ;
        ipp_pyrA.sz = ipp_pyrB.sz = size;
        ipp_pyrA.rate = ipp_pyrB.rate = (double*)rate;
        ipp_pyrA.step = ipp_pyrB.step = step;
        ipp_pyrA.state = ipp_pyrB.state = 0;
        ipp_pyrA.level = ipp_pyrB.level = level;

        if( !error )
        {
            _error = (float*)cvAlloc( count*sizeof(_error[0]) );
            if( !_error )
            {
                result = CV_OUTOFMEM_ERR;
                goto func_exit;
            }
            error = _error;
        }

        for( i = 0; i < count; i++ )
            featuresB[i] = featuresA[i];

        if( icvOpticalFlowPyrLK_8u_C1R_p( &ipp_pyrA, &ipp_pyrB,
            (const float*)featuresA, (float*)featuresB, status, error, count,
            winSize.width*2 + 1, level, criteria.max_iter,
            (float)criteria.epsilon, ipp_optflow_state ) >= 0 )
        {
            for( i = 0; i < count; i++ )
                status[i] = status[i] == 0;
            goto func_exit;
        }
    }
#endif

    /* buffer_size = <size for patches> + <size for pyramids> */
    bufferBytes = (srcPatchLen + patchLen * 3) * sizeof( _patchI[0][0] ) * threadCount;

    buffer = (uchar*)cvAlloc( bufferBytes );
    if( !buffer )
    {
        result = CV_OUTOFMEM_ERR;
        goto func_exit;
    }

    for( i = 0; i < threadCount; i++ )
    {
        _patchI[i] = i == 0 ? (float*)buffer : _Iy[i-1] + patchLen;
        _patchJ[i] = _patchI[i] + srcPatchLen;
        _Ix[i] = _patchJ[i] + patchLen;
        _Iy[i] = _Ix[i] + patchLen;
    }

    memset( status, 1, count );
    if( error )
        memset( error, 0, count*sizeof(error[0]) );

    if( !(flags & CV_LKFLOW_INITIAL_GUESSES) )
        memcpy( featuresB, featuresA, count*sizeof(featuresA[0]));

    /* do processing from top pyramid level (smallest image)
       to the bottom (original image) */
    for( l = level; l >= 0; l-- )
    {
        CvSize levelSize = size[l];
        int levelStep = step[l];

        {
#ifdef _OPENMP
        #pragma omp parallel for num_threads(threadCount), schedule(dynamic) 
#endif // _OPENMP
        /* find flow for each given point */
        for( i = 0; i < count; i++ )
        {
            CvPoint2D32f v;
            CvPoint minI, maxI, minJ, maxJ;
            CvSize isz, jsz;
            int pt_status;
            CvPoint2D32f u;
            CvPoint prev_minJ = { -1, -1 }, prev_maxJ = { -1, -1 };
            double Gxx = 0, Gxy = 0, Gyy = 0, D = 0;
            float prev_mx = 0, prev_my = 0;
            int j, x, y;
            int threadIdx = cvGetThreadNum();
            float* patchI = _patchI[threadIdx];
            float* patchJ = _patchJ[threadIdx];
            float* Ix = _Ix[threadIdx];
            float* Iy = _Iy[threadIdx];

            v.x = featuresB[i].x;
            v.y = featuresB[i].y;
            if( l < level )
            {
                v.x += v.x;
                v.y += v.y;
            }
            else
            {
                v.x = (float)(v.x * scale[l]);
                v.y = (float)(v.y * scale[l]);
            }

            pt_status = status[i];
            if( !pt_status )
                continue;

            minI = maxI = minJ = maxJ = cvPoint( 0, 0 );

            u.x = (float) (featuresA[i].x * scale[l]);
            u.y = (float) (featuresA[i].y * scale[l]);

            intersect( u, winSize, levelSize, &minI, &maxI );
            isz = jsz = cvSize(maxI.x - minI.x + 2, maxI.y - minI.y + 2);
            u.x += (minI.x - (patchSize.width - maxI.x + 1))*0.5f;
            u.y += (minI.y - (patchSize.height - maxI.y + 1))*0.5f;

            if( isz.width < 3 || isz.height < 3 ||
                icvGetRectSubPix_8u32f_C1R( imgI[l], levelStep, levelSize,
                    patchI, isz.width*sizeof(patchI[0]), isz, u ) < 0 )
            {
                /* point is outside the image. take the next */
                status[i] = 0;
                continue;
            }

            icvCalcIxIy_32f( patchI, isz.width*sizeof(patchI[0]), Ix, Iy,
                (isz.width-2)*sizeof(patchI[0]), isz, smoothKernel, patchJ );

            for( j = 0; j < criteria.max_iter; j++ )
            {
                double bx = 0, by = 0;
                float mx, my;
                CvPoint2D32f _v;

                intersect( v, winSize, levelSize, &minJ, &maxJ );

                minJ.x = MAX( minJ.x, minI.x );
                minJ.y = MAX( minJ.y, minI.y );

                maxJ.x = MIN( maxJ.x, maxI.x );
                maxJ.y = MIN( maxJ.y, maxI.y );

                jsz = cvSize(maxJ.x - minJ.x, maxJ.y - minJ.y);

                _v.x = v.x + (minJ.x - (patchSize.width - maxJ.x + 1))*0.5f;
                _v.y = v.y + (minJ.y - (patchSize.height - maxJ.y + 1))*0.5f;

                if( jsz.width < 1 || jsz.height < 1 ||
                    icvGetRectSubPix_8u32f_C1R( imgJ[l], levelStep, levelSize, patchJ,
                                                jsz.width*sizeof(patchJ[0]), jsz, _v ) < 0 )
                {
                    /* point is outside image. take the next */
                    pt_status = 0;
                    break;
                }

                if( maxJ.x == prev_maxJ.x && maxJ.y == prev_maxJ.y &&
                    minJ.x == prev_minJ.x && minJ.y == prev_minJ.y )
                {
                    for( y = 0; y < jsz.height; y++ )
                    {
                        const float* pi = patchI +
                            (y + minJ.y - minI.y + 1)*isz.width + minJ.x - minI.x + 1;
                        const float* pj = patchJ + y*jsz.width;
                        const float* ix = Ix +
                            (y + minJ.y - minI.y)*(isz.width-2) + minJ.x - minI.x;
                        const float* iy = Iy + (ix - Ix);

                        for( x = 0; x < jsz.width; x++ )
                        {
                            double t0 = pi[x] - pj[x];
                            bx += t0 * ix[x];
                            by += t0 * iy[x];
                        }
                    }
                }
                else
                {
                    Gxx = Gyy = Gxy = 0;
                    for( y = 0; y < jsz.height; y++ )
                    {
                        const float* pi = patchI +
                            (y + minJ.y - minI.y + 1)*isz.width + minJ.x - minI.x + 1;
                        const float* pj = patchJ + y*jsz.width;
                        const float* ix = Ix +
                            (y + minJ.y - minI.y)*(isz.width-2) + minJ.x - minI.x;
                        const float* iy = Iy + (ix - Ix);

                        for( x = 0; x < jsz.width; x++ )
                        {
                            double t = pi[x] - pj[x];
                            bx += (double) (t * ix[x]);
                            by += (double) (t * iy[x]);
                            Gxx += ix[x] * ix[x];
                            Gxy += ix[x] * iy[x];
                            Gyy += iy[x] * iy[x];
                        }
                    }

                    D = Gxx * Gyy - Gxy * Gxy;
                    if( D < DBL_EPSILON )
                    {
                        pt_status = 0;
                        break;
                    }
                    D = 1. / D;

                    prev_minJ = minJ;
                    prev_maxJ = maxJ;
                }

                mx = (float) ((Gyy * bx - Gxy * by) * D);
                my = (float) ((Gxx * by - Gxy * bx) * D);

                v.x += mx;
                v.y += my;

                if( mx * mx + my * my < criteria.epsilon )
                    break;

                if( j > 0 && fabs(mx + prev_mx) < 0.01 && fabs(my + prev_my) < 0.01 )
                {
                    v.x -= mx*0.5f;
                    v.y -= my*0.5f;
                    break;
                }
                prev_mx = mx;
                prev_my = my;
            }

            featuresB[i] = v;
            status[i] = (char)pt_status;
            if( l == 0 && error && pt_status )
            {
                /* calc error */
                double err = 0;

                for( y = 0; y < jsz.height; y++ )
                {
                    const float* pi = patchI +
                        (y + minJ.y - minI.y + 1)*isz.width + minJ.x - minI.x + 1;
                    const float* pj = patchJ + y*jsz.width;

                    for( x = 0; x < jsz.width; x++ )
                    {
                        double t = pi[x] - pj[x];
                        err += t * t;
                    }
                }
                error[i] = (float)sqrt(err);
            }
        } // end of point processing loop (i)
        }
    } // end of pyramid levels loop (l)

func_exit:

    if( ipp_optflow_state )
        icvOpticalFlowPyrLKFree_8u_C1R_p( ipp_optflow_state );

    cvFree( &pyrBuffer );
    cvFree( &buffer );
    cvFree( &_error );
    cvFree( &_status );

    return result;
#undef MAX_LEVEL
}

#if 0
/* Affine tracking algorithm */
static  CvStatus  icvCalcAffineFlowPyrLK_8uC1R( uchar * imgA, uchar * imgB,
                                                int imgStep, CvSize imgSize,
                                                uchar * pyrA, uchar * pyrB,
                                                CvPoint2D32f * featuresA,
                                                CvPoint2D32f * featuresB,
                                                float *matrices, int count,
                                                CvSize winSize, int level,
                                                char *status, float *error,
                                                CvTermCriteria criteria, int flags )
{
#define MAX_LEVEL 10
#define MAX_ITERS 100

    static const float kerX[] = { -1, 0, 1 }, kerY[] =
    {
    0.09375, 0.3125, 0.09375};  /* 3/32, 10/32, 3/32 */

    uchar *buffer = 0;
    uchar *pyr_buffer = 0;
    int bufferBytes = 0;

    uchar **imgI = 0;
    uchar **imgJ = 0;
    int *step = 0;
    double *scale = 0;
    CvSize* size = 0;

    float *patchI;
    float *patchJ;
    float *Ix;
    float *Iy;

    int i, j, k;
    int x, y;

    CvSize patchSize = cvSize( winSize.width * 2 + 1, winSize.height * 2 + 1 );
    int patchLen = patchSize.width * patchSize.height;
    int patchStep = patchSize.width * sizeof( patchI[0] );

    CvSize srcPatchSize = cvSize( patchSize.width + 2, patchSize.height + 2 );
    int srcPatchLen = srcPatchSize.width * srcPatchSize.height;
    int srcPatchStep = srcPatchSize.width * sizeof( patchI[0] );

    CvStatus result = CV_OK;

    /* check input arguments */
    if( !featuresA || !featuresB || !matrices )
        return CV_NULLPTR_ERR;
    if( winSize.width <= 1 || winSize.height <= 1 )
        return CV_BADSIZE_ERR;

    if( (flags & ~7) != 0 )
        return CV_BADFLAG_ERR;
    if( count <= 0 )
        return CV_BADRANGE_ERR;

    result = icvInitPyramidalAlgorithm( imgA, imgB, imgStep, imgSize,
                                        pyrA, pyrB, level, &criteria, MAX_ITERS, flags,
                                        &imgI, &imgJ, &step, &size, &scale, &pyr_buffer );

    if( result < 0 )
        goto func_exit;

    /* buffer_size = <size for patches> + <size for pyramids> */
    bufferBytes = (srcPatchLen + patchLen * 3) * sizeof( patchI[0] ) +

        (36 * 2 + 6) * sizeof( double );

    buffer = (uchar *) cvAlloc( bufferBytes );
    if( !buffer )
    {
        result = CV_OUTOFMEM_ERR;
        goto func_exit;
    }

    patchI = (float *) buffer;
    patchJ = patchI + srcPatchLen;
    Ix = patchJ + patchLen;
    Iy = Ix + patchLen;

    if( status )
        memset( status, 1, count );

    if( !(flags & CV_LKFLOW_INITIAL_GUESSES) )
    {
        memcpy( featuresB, featuresA, count * sizeof( featuresA[0] ));
        for( i = 0; i < count * 4; i += 4 )
        {
            matrices[i] = matrices[i + 2] = 1.f;
            matrices[i + 1] = matrices[i + 3] = 0.f;
        }
    }

    /* find flow for each given point */
    for( i = 0; i < count; i++ )
    {
        CvPoint2D32f v;
        float A[4];
        double G[36];
        int l;
        int pt_status = 1;

        memcpy( A, matrices + i * 4, sizeof( A ));

        v.x = (float) (featuresB[i].x * scale[level] * 0.5);
        v.y = (float) (featuresB[i].y * scale[level] * 0.5);

        /* do processing from top pyramid level (smallest image)
           to the bottom (original image) */
        for( l = level; l >= 0; l-- )
        {
            CvPoint2D32f u;
            CvSize levelSize = size[l];
            int x, y;

            v.x += v.x;
            v.y += v.y;

            u.x = (float) (featuresA[i].x * scale[l]);
            u.y = (float) (featuresA[i].y * scale[l]);

            if( icvGetRectSubPix_8u32f_C1R( imgI[l], step[l], levelSize,
                                             patchI, srcPatchStep, srcPatchSize, u ) < 0 )
            {
                /* point is outside the image. take the next */
                pt_status = 0;
                break;
            }

            /* calc Ix */
            icvSepConvSmall3_32f( patchI, srcPatchStep, Ix, patchStep,
                                  srcPatchSize, kerX, kerY, patchJ );

            /* calc Iy */
            icvSepConvSmall3_32f( patchI, srcPatchStep, Iy, patchStep,
                                  srcPatchSize, kerY, kerX, patchJ );

            /* repack patchI (remove borders) */
            for( k = 0; k < patchSize.height; k++ )
                memcpy( patchI + k * patchSize.width,
                        patchI + (k + 1) * srcPatchSize.width + 1, patchStep );

            memset( G, 0, sizeof( G ));

            /* calculate G matrix */
            for( y = -winSize.height, k = 0; y <= winSize.height; y++ )
            {
                for( x = -winSize.width; x <= winSize.width; x++, k++ )
                {
                    double ixix = ((double) Ix[k]) * Ix[k];
                    double ixiy = ((double) Ix[k]) * Iy[k];
                    double iyiy = ((double) Iy[k]) * Iy[k];

                    double xx, xy, yy;

                    G[0] += ixix;
                    G[1] += ixiy;
                    G[2] += x * ixix;
                    G[3] += y * ixix;
                    G[4] += x * ixiy;
                    G[5] += y * ixiy;

                    // G[6] == G[1]
                    G[7] += iyiy;
                    // G[8] == G[4]
                    // G[9] == G[5]
                    G[10] += x * iyiy;
                    G[11] += y * iyiy;

                    xx = x * x;
                    xy = x * y;
                    yy = y * y;

                    // G[12] == G[2]
                    // G[13] == G[8] == G[4]
                    G[14] += xx * ixix;
                    G[15] += xy * ixix;
                    G[16] += xx * ixiy;
                    G[17] += xy * ixiy;

                    // G[18] == G[3]
                    // G[19] == G[9]
                    // G[20] == G[15]
                    G[21] += yy * ixix;
                    // G[22] == G[17]
                    G[23] += yy * ixiy;

                    // G[24] == G[4]
                    // G[25] == G[10]
                    // G[26] == G[16]
                    // G[27] == G[22]
                    G[28] += xx * iyiy;
                    G[29] += xy * iyiy;

                    // G[30] == G[5]
                    // G[31] == G[11]
                    // G[32] == G[17]
                    // G[33] == G[23]
                    // G[34] == G[29]
                    G[35] += yy * iyiy;
                }
            }

            G[8] = G[4];
            G[9] = G[5];
            G[22] = G[17];

            // fill part of G below its diagonal
            for( y = 1; y < 6; y++ )
                for( x = 0; x < y; x++ )
                    G[y * 6 + x] = G[x * 6 + y];

            CvMat mat;
            cvInitMatHeader( &mat, 6, 6, CV_64FC1, G );

            if( cvInvert( &mat, &mat, CV_SVD ) < 1e-3 )
            {
                /* bad matrix. take the next point */
                pt_status = 0;
            }
            else
            {
                for( j = 0; j < criteria.max_iter; j++ )
                {
                    double b[6], eta[6];
                    double t0, t1, s = 0;

                    if( icvGetQuadrangleSubPix_8u32f_C1R( imgJ[l], step[l], levelSize,
                                                          patchJ, patchStep, patchSize, A,
                                                          0, 0 ) < 0 )
                    {
                        pt_status = 0;
                        break;
                    }

                    memset( b, 0, sizeof( b ));

                    for( y = -winSize.height, k = 0; y <= winSize.height; y++ )
                    {
                        for( x = -winSize.width; x <= winSize.width; x++, k++ )
                        {
                            double t = patchI[k] - patchJ[k];
                            double ixt = Ix[k] * t;
                            double iyt = Iy[k] * t;

                            s += t;

                            b[0] += ixt;
                            b[1] += iyt;
                            b[2] += x * ixt;
                            b[3] += y * ixt;
                            b[4] += x * iyt;
                            b[5] += y * iyt;
                        }
                    }

                    icvTransformVector_64d( G, b, eta, 6, 6 );

                    t0 = v.x + A[0] * eta[0] + A[1] * eta[1];
                    t1 = v.y + A[2] * eta[0] + A[3] * eta[1];

                    assert( fabs( t0 ) < levelSize.width * 2 );
                    assert( fabs( t1 ) < levelSize.height * 2 );

                    v.x = (float) t0;
                    v.y = (float) t1;

                    t0 = A[0] * (1 + eta[2]) + A[1] * eta[4];
                    t1 = A[0] * eta[3] + A[1] * (1 + eta[5]);
                    A[0] = (float) t0;
                    A[1] = (float) t1;

                    t0 = A[2] * (1 + eta[2]) + A[3] * eta[4];
                    t1 = A[2] * eta[3] + A[3] * (1 + eta[5]);
                    A[2] = (float) t0;
                    A[3] = (float) t1;

                    /*t0 = 4./(fabs(A[0]) + fabs(A[1]) + fabs(A[2]) + fabs(A[3]) + DBL_EPSILON);
                       A[0] = (float)(A[0]*t0);
                       A[1] = (float)(A[1]*t0);
                       A[2] = (float)(A[2]*t0);
                       A[3] = (float)(A[3]*t0);

                       t0 = fabs(A[0]*A[2] - A[1]*A[3]);
                       if( t0 >
                       A[0] = (float)(A[0]*t0);
                       A[1] = (float)(A[1]*t0);
                       A[2] = (float)(A[2]*t0);
                       A[3] = (float)(A[3]*t0); */

                    if( eta[0] * eta[0] + eta[1] * eta[1] < criteria.epsilon )
                        break;
                }
            }

            if( pt_status == 0 )
                break;
        }

        if( pt_status )
        {
            featuresB[i] = v;
            memcpy( matrices + i * 4, A, sizeof( A ));

            if( error )
            {
                /* calc error */
                double err = 0;

                for( y = 0, k = 0; y < patchSize.height; y++ )
                {
                    for( x = 0; x < patchSize.width; x++, k++ )
                    {
                        double t = patchI[k] - patchJ[k];
                        err += t * t;
                    }
                }
                error[i] = (float) sqrt( err );
            }
        }

        if( status )
            status[i] = (char) pt_status;
    }

  func_exit:

    cvFree( &pyr_buffer );
    cvFree( &buffer );

    return result;
#undef MAX_LEVEL
}
#endif

static int icvMinimalPyramidSize( CvSize img_size )
{
    return cvAlign(img_size.width,8) * img_size.height / 3;
}


CV_IMPL void
cvCalcOpticalFlowPyrLK( const void* arrA, const void* arrB,
                        void* pyrarrA, void* pyrarrB,
                        const CvPoint2D32f * featuresA,
                        CvPoint2D32f * featuresB,
                        int count, CvSize winSize, int level,
                        char *status, float *error,
                        CvTermCriteria criteria, int flags )
{
    CV_FUNCNAME( "cvCalcOpticalFlowPyrLK" );

    __BEGIN__;

    CvMat stubA, *imgA = (CvMat*)arrA;
    CvMat stubB, *imgB = (CvMat*)arrB;
    CvMat pstubA, *pyrA = (CvMat*)pyrarrA;
    CvMat pstubB, *pyrB = (CvMat*)pyrarrB;
    CvSize img_size;

    CV_CALL( imgA = cvGetMat( imgA, &stubA ));
    CV_CALL( imgB = cvGetMat( imgB, &stubB ));

    if( CV_MAT_TYPE( imgA->type ) != CV_8UC1 )
        CV_ERROR( CV_StsUnsupportedFormat, "" );

    if( !CV_ARE_TYPES_EQ( imgA, imgB ))
        CV_ERROR( CV_StsUnmatchedFormats, "" );

    if( !CV_ARE_SIZES_EQ( imgA, imgB ))
        CV_ERROR( CV_StsUnmatchedSizes, "" );

    if( imgA->step != imgB->step )
        CV_ERROR( CV_StsUnmatchedSizes, "imgA and imgB must have equal steps" );

    img_size = cvGetMatSize( imgA );

    if( pyrA )
    {
        CV_CALL( pyrA = cvGetMat( pyrA, &pstubA ));

        if( pyrA->step*pyrA->height < icvMinimalPyramidSize( img_size ) )
            CV_ERROR( CV_StsBadArg, "pyramid A has insufficient size" );
    }
    else
    {
        pyrA = &pstubA;
        pyrA->data.ptr = 0;
    }


    if( pyrB )
    {
        CV_CALL( pyrB = cvGetMat( pyrB, &pstubB ));

        if( pyrB->step*pyrB->height < icvMinimalPyramidSize( img_size ) )
            CV_ERROR( CV_StsBadArg, "pyramid B has insufficient size" );
    }
    else
    {
        pyrB = &pstubB;
        pyrB->data.ptr = 0;
    }

    IPPI_CALL( icvCalcOpticalFlowPyrLK_8uC1R( imgA->data.ptr, imgB->data.ptr, imgA->step,
                                              img_size, pyrA->data.ptr, pyrB->data.ptr,
                                              featuresA, featuresB,
                                              count, winSize, level, status,
                                              error, criteria, flags ));

    __END__;
}

#if 0
CV_IMPL void
cvCalcAffineFlowPyrLK( const void* arrA, const void* arrB,
                       void* pyrarrA, void* pyrarrB,
                       CvPoint2D32f * featuresA,
                       CvPoint2D32f * featuresB,
                       float *matrices, int count,
                       CvSize winSize, int level,
                       char *status, float *error,
                       CvTermCriteria criteria, int flags )
{
    CV_FUNCNAME( "cvCalcAffineFlowPyrLK" );

    __BEGIN__;

    CvMat stubA, *imgA = (CvMat*)arrA;
    CvMat stubB, *imgB = (CvMat*)arrB;
    CvMat pstubA, *pyrA = (CvMat*)pyrarrA;
    CvMat pstubB, *pyrB = (CvMat*)pyrarrB;
    CvSize img_size;

    CV_CALL( imgA = cvGetMat( imgA, &stubA ));
    CV_CALL( imgB = cvGetMat( imgB, &stubB ));

    if( CV_MAT_TYPE( imgA->type ) != CV_8UC1 )
        CV_ERROR( CV_StsUnsupportedFormat, "" );

    if( !CV_ARE_TYPES_EQ( imgA, imgB ))
        CV_ERROR( CV_StsUnmatchedFormats, "" );

    if( !CV_ARE_SIZES_EQ( imgA, imgB ))
        CV_ERROR( CV_StsUnmatchedSizes, "" );

    if( imgA->step != imgB->step )
        CV_ERROR( CV_StsUnmatchedSizes, "imgA and imgB must have equal steps" );

    if( !matrices )
        CV_ERROR( CV_StsNullPtr, "" );

    img_size = cvGetMatSize( imgA );

    if( pyrA )
    {
        CV_CALL( pyrA = cvGetMat( pyrA, &pstubA ));

        if( pyrA->step*pyrA->height < icvMinimalPyramidSize( img_size ) )
            CV_ERROR( CV_StsBadArg, "pyramid A has insufficient size" );
    }
    else
    {
        pyrA = &pstubA;
        pyrA->data.ptr = 0;
    }


    if( pyrB )
    {
        CV_CALL( pyrB = cvGetMat( pyrB, &pstubB ));

        if( pyrB->step*pyrB->height < icvMinimalPyramidSize( img_size ) )
            CV_ERROR( CV_StsBadArg, "pyramid B has insufficient size" );
    }
    else
    {
        pyrB = &pstubB;
        pyrB->data.ptr = 0;
    }

    IPPI_CALL( icvCalcAffineFlowPyrLK_8uC1R( imgA->data.ptr, imgB->data.ptr, imgA->step,
                                             img_size, pyrA->data.ptr, pyrB->data.ptr,
                                             featuresA, featuresB, matrices,
                                             count, winSize, level, status,
                                             error, criteria, flags ));

    __END__;
}
#endif


/* End of file. */
