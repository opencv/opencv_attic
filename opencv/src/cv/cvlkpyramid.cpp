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
           CvPoint * min_pt, CvPoint * max_pt )
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
    bufferBytes = (level1 >= 0) * ((pyrA == 0) + (pyrB == 0)) * pyrBytes +
        (sizeof( imgI[0][0] ) * 2 + sizeof( step[0][0] ) +
         sizeof(size[0][0]) + sizeof( scale[0][0] )) * level1;

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

            //srcSize.width &= -2;
            //srcSize.height &= -2;

            if( !(flags & CV_LKFLOW_PYR_A_READY) )
            {
                prev_level = cvMat( size[0][i-1].height, size[0][i-1].width, CV_8UC1 );
                next_level = cvMat( size[0][i].height, size[0][i].width, CV_8UC1 );
                cvSetData( &prev_level, imgI[0][i-1], step[0][i-1] );
                cvSetData( &next_level, imgI[0][i], step[0][i] );
                cvPyrDown( &prev_level, &next_level );

                /*result = icvPyrDown_Gauss5x5_8u_C1R( imgI[0][i - 1], step[0][i - 1],
                                                     imgI[0][i], step[0][i],
                                                     srcSize, pyr_down_temp_buffer );
                if( result < 0 )
                    goto func_exit;
                icvPyrDownBorder_8u_CnR( imgI[0][i - 1], step[0][i - 1], size[0][i-1],
                                         imgI[0][i], step[0][i], size[0][i], 1 );*/
                
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
            
                /*result = icvPyrDown_Gauss5x5_8u_C1R( imgJ[0][i - 1], step[0][i - 1],
                                                     imgJ[0][i], step[0][i],
                                                     srcSize, pyr_down_temp_buffer );
                if( result < 0 )
                    goto func_exit;
                icvPyrDownBorder_8u_CnR( imgJ[0][i - 1], step[0][i - 1], size[0][i-1],
                                         imgJ[0][i], step[0][i], size[0][i], 1 );*/
            }
        }
    }

    return CV_OK;
}


/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name: icvCalcOpticalFlowPyrLK_8uC1R ( Lucas & Kanade method,
//                                           modification that uses pyramids )
//    Purpose:
//      Calculates optical flow between two images for certain set of points.
//    Context:
//    Parameters:
//            imgA     - pointer to first frame (time t)
//            imgB     - pointer to second frame (time t+1)
//            imgStep  - full width of the source images in bytes
//            imgSize  - size of the source images
//            pyrA     - buffer for pyramid for the first frame.
//                       if the pointer is not NULL, the buffer must have size enough to
//                       store pyramid (from level 1 to level #<level> (see below))
//                       (imgSize.width*imgSize.height/3 will be enough)).
//            pyrB     - similar to pyrA, but for the second frame.
//                       
//                       for both parameters above the following rules work:
//                           If pointer is 0, the function allocates the buffer internally,
//                           calculates pyramid and releases the buffer after processing.
//                           Else (it should be large enough then) the function calculates
//                           pyramid and stores it in the buffer unless the
//                           CV_LKFLOW_PYR_A[B]_READY flag is set. In both cases
//                           (flag is set or not) the subsequent calls may reuse the calculated
//                           pyramid by setting CV_LKFLOW_PYR_A[B]_READY.
//
//            featuresA - array of points, for which the flow needs to be found
//            count    - number of feature points 
//            winSize  - size of search window on each pyramid level
//            level    - maximal pyramid level number
//                         (if 0, pyramids are not used (single level),
//                          if 1, two levels are used etc.)
//
//            next parameters are arrays of <count> elements.
//            ------------------------------------------------------
//            featuresB - array of 2D points, containing calculated
//                       new positions of input features (in the second image).
//            status   - array, every element of which will be set to 1 if the flow for the
//                       corresponding feature has been found, 0 else.
//            error    - array of double numbers, containing difference between
//                       patches around the original and moved points
//                       (it is optional parameter, can be NULL).
//            ------------------------------------------------------
//            criteria   - specifies when to stop the iteration process of finding flow
//                         for each point on each pyramid level
//
//            flags      - miscellaneous flags:
//                            CV_LKFLOW_PYR_A_READY - pyramid for the first frame
//                                                      is precalculated before call
//                            CV_LKFLOW_PYR_B_READY - pyramid for the second frame
//                                                      is precalculated before call
//                            CV_LKFLOW_INITIAL_GUESSES - featuresB array holds initial
//                                                       guesses about new features'
//                                                       locations before function call.
//    Returns: CV_OK       - all ok
//             CV_OUTOFMEM_ERR - insufficient memory for function work
//             CV_NULLPTR_ERR  - if one of input pointers is NULL
//             CV_BADSIZE_ERR  - wrong input sizes interrelation
//
//    Notes:  For calculating spatial derivatives 3x3 Sobel operator is used.
//            The values of pixels beyond the image are determined using replication mode.
//F*/
static  CvStatus  icvCalcOpticalFlowPyrLK_8uC1R( const uchar * imgA,
                                                 const uchar * imgB,
                                                 int imgStep,
                                                 CvSize imgSize,
                                                 uchar * pyrA,
                                                 uchar * pyrB,
                                                 const CvPoint2D32f * featuresA,
                                                 CvPoint2D32f * featuresB,
                                                 int count,
                                                 CvSize winSize,
                                                 int level,
                                                 char *status,
                                                 float *error,
                                                 CvTermCriteria criteria, int flags )
{
#define MAX_LEVEL 10
#define MAX_ITERS 100

    static const float kerX[] = { -1, 0, 1 }, kerY[] =
    {
    0.09375, 0.3125, 0.09375};  /* 3/32, 10/32, 3/32 */

    uchar *pyr_buffer = 0;
    uchar *buffer = 0;
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
    if( !featuresA || !featuresB )
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
    bufferBytes = (srcPatchLen + patchLen * 3) * sizeof( patchI[0] );

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

    memset( status, 1, count );

    if( !(flags & CV_LKFLOW_INITIAL_GUESSES) )
    {
        memcpy( featuresB, featuresA, count * sizeof( featuresA[0] ));
    }

    /* find flow for each given point */
    for( i = 0; i < count; i++ )
    {
        CvPoint2D32f v;
        CvPoint minI, maxI, minJ, maxJ;
        int l, pt_status = 1;

        minI = maxI = minJ = maxJ = cvPoint( 0, 0 );

        v.x = (float) (featuresB[i].x * scale[level] * 0.5);
        v.y = (float) (featuresB[i].y * scale[level] * 0.5);

        /* do processing from top pyramid level (smallest image)
           to the bottom (original image) */
        for( l = level; l >= 0; l-- )
        {
            CvPoint2D32f u;
            CvSize levelSize = size[l];
            CvPoint prev_minJ = { -1, -1 }, prev_maxJ = { -1, -1 };
            double Gxx = 0, Gxy = 0, Gyy = 0, D = 0;
            float prev_mx = 0, prev_my = 0;

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

            intersect( u, winSize, levelSize, &minI, &maxI );

            for( j = 0; j < criteria.max_iter; j++ )
            {
                double bx = 0, by = 0;
                float mx, my;

                if( icvGetRectSubPix_8u32f_C1R( imgJ[l], step[l], levelSize,
                                                patchJ, patchStep, patchSize, v ) < 0 )
                {
                    /* point is outside image. take the next */
                    pt_status = 0;
                    break;
                }

                intersect( v, winSize, levelSize, &minJ, &maxJ );

                minJ.x = MAX( minJ.x, minI.x );
                minJ.y = MAX( minJ.y, minI.y );

                maxJ.x = MIN( maxJ.x, maxI.x );
                maxJ.y = MIN( maxJ.y, maxI.y );

                if( maxJ.x == prev_maxJ.x &&
                    maxJ.y == prev_maxJ.y &&
                    minJ.x == prev_minJ.x &&
                    minJ.y == prev_minJ.y )
                {
                    for( y = minJ.y; y < maxJ.y; y++ )
                    {
                        for( x = minJ.x; x < maxJ.x; x++ )
                        {
                            int idx = y * (winSize.width * 2 + 1) + x;
                            double t = patchI[idx] - patchJ[idx];

                            bx += (double) (t * Ix[idx]);
                            by += (double) (t * Iy[idx]);
                        }
                    }
                }
                else
                {
                    Gxx = Gyy = Gxy = 0;
                    
                    for( y = minJ.y; y < maxJ.y; y++ )
                    {
                        for( x = minJ.x; x < maxJ.x; x++ )
                        {
                            int idx = y * (winSize.width * 2 + 1) + x;
                            double t = patchI[idx] - patchJ[idx];

                            bx += (double) (t * Ix[idx]);
                            by += (double) (t * Iy[idx]);
                            Gxx += Ix[idx] * Ix[idx];
                            Gxy += Ix[idx] * Iy[idx];
                            Gyy += Iy[idx] * Iy[idx];
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

            if( pt_status == 0 )
                break;
        }

        if( pt_status )
        {
            featuresB[i] = v;

            if( error )
            {
                /* calc error */
                double err = 0;

                for( y = minJ.y; y < maxJ.y; y++ )
                {
                    for( x = minJ.x; x < maxJ.x; x++ )
                    {
                        int idx = y * (winSize.width * 2 + 1) + x;
                        double t = patchI[idx] - patchJ[idx];

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

    cvFree( (void**)&pyr_buffer );
    cvFree( (void**)&buffer );

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

    cvFree( (void**)&pyr_buffer );
    cvFree( (void**)&buffer );

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
