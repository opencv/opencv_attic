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

/* calculate sizes of temporary buffers */
static void
icvCalculateBufferSizes( CvSize roiSize, CvSize templSize,
                         CvDataType dataType,
                         int is_centered, int is_normed,
                         int *imgBufSize, int *templBufSize,
                         int *sumBufSize, int *sqsumBufSize,
                         int *resNumBufSize, int *resDenomBufSize )
{
    int depth = dataType == cv32f ? sizeof_float : 1;

#define align(size) icvAlign((int)(size) + 16, 32)

    *imgBufSize = align( (templSize.width * roiSize.height + roiSize.width) * depth );
    *templBufSize = align( templSize.width * templSize.height * depth );
    *resNumBufSize = align( (roiSize.height - templSize.height + 1) * sizeof( double ));

    if( is_centered || is_normed )
    {
        *sumBufSize = align( roiSize.height * sizeof( double ));
        *sqsumBufSize = align( roiSize.height * sizeof( double ));

        *resDenomBufSize = align( (roiSize.height - templSize.height + 1) *
                                  (is_centered + is_normed) * sizeof( double ));
    }

#undef align
}


static CvStatus
icvMatchTemplateEntry( const void *pImage, int imageStep,
                       CvSize roiSize,
                       const void *pTemplate, int templStep,
                       CvSize templSize,
                       float *pResult, int resultStep,
                       void *pBuffer, CvDataType dataType,
                       int is_centered, int is_normed,
                       void **imgBuf, void **templBuf,
                       void **sumBuf, void **sqsumBuf, void **resNum, void **resDenom )
{
    int templBufSize = 0,
        imgBufSize = 0,

        sumBufSize = 0, sqsumBufSize = 0, resNumBufSize = 0, resDenomBufSize = 0;
    int depth = dataType == cv32f ? 4 : 1;
    int i;
    char *buffer = (char *) pBuffer;

    if( !pImage || !pTemplate || !pResult || !pBuffer )
        return CV_NULLPTR_ERR;

    if( templSize.width <= 0 || templSize.height <= 0 ||
        roiSize.width < templSize.width || roiSize.height < templSize.height )
        return CV_BADSIZE_ERR;

    if( templStep < templSize.width * depth ||
        imageStep < roiSize.width * depth ||
        resultStep < (roiSize.width - templSize.width + 1) * sizeof_float )
        return CV_BADSIZE_ERR;

    if( (templStep & (depth - 1)) != 0 ||
        (imageStep & (depth - 1)) != 0 || (resultStep & (sizeof_float - 1)) != 0 )
        return CV_BADSIZE_ERR;

    icvCalculateBufferSizes( roiSize, templSize, dataType,
                             is_centered, is_normed,
                             &imgBufSize, &templBufSize,
                             &sumBufSize, &sqsumBufSize, &resNumBufSize, &resDenomBufSize );

    *templBuf = buffer;
    buffer += templBufSize;

    *imgBuf = buffer;
    buffer += imgBufSize;

    *resNum = buffer;
    buffer += resNumBufSize;

    if( is_centered || is_normed )
    {
        if( sumBuf )
            *sumBuf = buffer;
        buffer += sumBufSize;

        if( sqsumBuf )
            *sqsumBuf = buffer;
        buffer += sqsumBufSize;

        if( resDenom )
            *resDenom = buffer;
        buffer += resDenomBufSize;
    }

    for( i = 0; i < roiSize.height; i++ )
    {
        memcpy( (char *) *imgBuf + i * templSize.width * depth,
                (char *) pImage + i * imageStep, templSize.width * depth );
    }

    for( i = 0; i < templSize.height; i++ )
    {
        memcpy( (char *) *templBuf + i * templSize.width * depth,
                (char *) pTemplate + i * templStep, templSize.width * depth );
    }

    return CV_OK;
}


/****************************************************************************************\
*                          External functions' implementation                            *
\****************************************************************************************/

/****************************************************************************************\
*                                      8u flavor                                         *
\****************************************************************************************/

/* --------------------------------------- SqDiff ------------------------------------- */

IPCVAPI_IMPL( CvStatus, icvMatchTemplate_SqDiff_8u32f_C1R,
              (const uchar * pImage, int imageStep, CvSize roiSize,
               const uchar * pTemplate, int templStep, CvSize templSize,
               float *pResult, int resultStep, void *pBuffer) )
{
    uchar *imgBuf = 0;
    uchar *templBuf = 0;
    int64 *resNum = 0;
    int winLen = templSize.width * templSize.height;
    CvSize resultSize = cvSize( roiSize.width - templSize.width + 1,
                                roiSize.height - templSize.height + 1 );
    int x, y;

    CvStatus result = icvMatchTemplateEntry( pImage, imageStep, roiSize,
                                             pTemplate, templStep, templSize,
                                             pResult, resultStep, pBuffer,
                                             cv8u, 0, 0,
                                             (void **) &imgBuf, (void **) &templBuf,
                                             0, 0, (void **) &resNum, 0 );

    if( result != CV_OK )
        return result;

    resultStep /= sizeof_float;

    /* main loop - through x coordinate of the result */
    for( x = 0; x < resultSize.width; x++ )
    {
        uchar *imgPtr = imgBuf + x;

        /* update sums and image band buffer */
        if( x > 0 )
        {
            const uchar *src = pImage + x + templSize.width - 1;
            uchar *dst = imgPtr - 1;

            dst += templSize.width;

            for( y = 0; y < roiSize.height; y++, src += imageStep, dst += templSize.width )
            {
                dst[0] = src[0];
            }
        }

        for( y = 0; y < resultSize.height; y++, imgPtr += templSize.width )
        {
            int64 res = icvCmpBlocksL2_8u_C1( imgPtr, templBuf, winLen );

            resNum[y] = res;
        }

        for( y = 0; y < resultSize.height; y++ )
        {
            pResult[x + y * resultStep] = (float) resNum[y];
        }
    }

    return CV_OK;
}


/* ----------------------------------- SqDiffNormed ----------------------------------- */

IPCVAPI_IMPL( CvStatus, icvMatchTemplate_SqDiffNormed_8u32f_C1R,
              (const uchar * pImage, int imageStep, CvSize roiSize,
               const uchar * pTemplate, int templStep, CvSize templSize,
               float *pResult, int resultStep, void *pBuffer) )
{
    uchar *imgBuf = 0;
    uchar *templBuf = 0;
    int64 *sqsumBuf = 0;
    int64 *resNum = 0;
    int64 *resDenom = 0;
    double templCoeff = 0;

    int winLen = templSize.width * templSize.height;
    CvSize resultSize = cvSize( roiSize.width - templSize.width + 1,
                                roiSize.height - templSize.height + 1 );
    int x, y;

    CvStatus result = icvMatchTemplateEntry( pImage, imageStep, roiSize,
                                             pTemplate, templStep, templSize,
                                             pResult, resultStep, pBuffer,
                                             cv8u, 0, 1,
                                             (void **) &imgBuf, (void **) &templBuf,
                                             0, (void **) &sqsumBuf,
                                             (void **) &resNum, (void **) &resDenom );

    if( result != CV_OK )
        return result;

    resultStep /= sizeof_float;

    /* calc common statistics for template and image */
    {
        const uchar *rowPtr = (const uchar *) imgBuf;
        int64 templSqsum = icvCrossCorr_8u_C1( templBuf, templBuf, winLen );

        templCoeff = (double) templSqsum;
        templCoeff = icvInvSqrt64d( fabs( templCoeff ) + FLT_EPSILON );

        for( y = 0; y < roiSize.height; y++, rowPtr += templSize.width )
        {
            sqsumBuf[y] = icvCrossCorr_8u_C1( rowPtr, rowPtr, templSize.width );
        }
    }

    /* main loop - through x coordinate of the result */
    for( x = 0; x < resultSize.width; x++ )
    {
        int64 sqsum = 0;
        uchar *img_ptr = imgBuf + x;

        /* update sums and image band buffer */
        if( x > 0 )
        {
            const uchar *src = pImage + x + templSize.width - 1;
            uchar *dst = img_ptr - 1;
            int out_val = dst[0];

            dst += templSize.width;

            for( y = 0; y < roiSize.height; y++, src += imageStep, dst += templSize.width )
            {
                int in_val = src[0];

                sqsumBuf[y] += (in_val - out_val) * (in_val + out_val);
                out_val = dst[0];
                dst[0] = (uchar) in_val;
            }
        }

        for( y = 0; y < templSize.height; y++ )
        {
            sqsum += sqsumBuf[y];
        }

        for( y = 0; y < resultSize.height; y++, img_ptr += templSize.width )
        {
            int64 res = icvCmpBlocksL2_8u_C1( img_ptr, templBuf, winLen );

            if( y > 0 )
            {
                sqsum -= sqsumBuf[y - 1];
                sqsum += sqsumBuf[y + templSize.height - 1];
            }
            resNum[y] = res;
            resDenom[y] = sqsum;
        }



        for( y = 0; y < resultSize.height; y++ )
        {
            double res = ((double) resNum[y]) * templCoeff *
                icvInvSqrt64d( fabs( (double) resDenom[y] ) + FLT_EPSILON );

            pResult[x + y * resultStep] = (float) res;
        }
    }

    return CV_OK;
}

/* -------------------------------------- Corr ---------------------------------------- */

IPCVAPI_IMPL( CvStatus, icvMatchTemplate_Corr_8u32f_C1R,
              (const uchar * pImage, int imageStep, CvSize roiSize,
               const uchar * pTemplate, int templStep, CvSize templSize,
               float *pResult, int resultStep, void *pBuffer) )
{
    uchar *imgBuf = 0;
    uchar *templBuf = 0;
    int64 *resNum = 0;
    int winLen = templSize.width * templSize.height;
    CvSize resultSize = cvSize( roiSize.width - templSize.width + 1,
                                roiSize.height - templSize.height + 1 );
    int x, y;

    CvStatus result = icvMatchTemplateEntry( pImage, imageStep, roiSize,
                                             pTemplate, templStep, templSize,
                                             pResult, resultStep, pBuffer,
                                             cv8u, 0, 0,
                                             (void **) &imgBuf, (void **) &templBuf,
                                             0, 0, (void **) &resNum, 0 );

    if( result != CV_OK )
        return result;

    resultStep /= sizeof_float;

    /* main loop - through x coordinate of the result */
    for( x = 0; x < resultSize.width; x++ )
    {
        uchar *imgPtr = imgBuf + x;

        /* update sums and image band buffer */
        if( x > 0 )
        {
            const uchar *src = pImage + x + templSize.width - 1;
            uchar *dst = imgPtr - 1;

            dst += templSize.width;

            for( y = 0; y < roiSize.height; y++, src += imageStep, dst += templSize.width )
            {
                dst[0] = src[0];
            }
        }

        for( y = 0; y < resultSize.height; y++, imgPtr += templSize.width )
        {
            int64 res = icvCrossCorr_8u_C1( imgPtr, templBuf, winLen );

            resNum[y] = res;
        }



        for( y = 0; y < resultSize.height; y++ )
        {
            pResult[x + y * resultStep] = (float) resNum[y];
        }
    }

    return CV_OK;
}

/* ------------------------------------ CorrNormed ------------------------------------ */

IPCVAPI_IMPL( CvStatus, icvMatchTemplate_CorrNormed_8u32f_C1R,
              (const uchar * pImage, int imageStep, CvSize roiSize,
               const uchar * pTemplate, int templStep, CvSize templSize,
               float *pResult, int resultStep, void *pBuffer) )
{
    uchar *imgBuf = 0;
    uchar *templBuf = 0;
    int64 *sqsumBuf = 0;
    int64 *resNum = 0;
    int64 *resDenom = 0;
    double templCoeff = 0;

    int winLen = templSize.width * templSize.height;
    CvSize resultSize = cvSize( roiSize.width - templSize.width + 1,
                                roiSize.height - templSize.height + 1 );
    int x, y;

    CvStatus result = icvMatchTemplateEntry( pImage, imageStep, roiSize,
                                             pTemplate, templStep, templSize,
                                             pResult, resultStep, pBuffer,
                                             cv8u, 0, 1,
                                             (void **) &imgBuf, (void **) &templBuf,
                                             0, (void **) &sqsumBuf,
                                             (void **) &resNum, (void **) &resDenom );

    if( result != CV_OK )
        return result;

    resultStep /= sizeof_float;

    /* calc common statistics for template and image */
    {
        const uchar *rowPtr = (const uchar *) imgBuf;
        int64 templSqsum = icvCrossCorr_8u_C1( templBuf, templBuf, winLen );


        templCoeff = (double) templSqsum;
        templCoeff = icvInvSqrt64d( fabs( templCoeff ) + FLT_EPSILON );

        for( y = 0; y < roiSize.height; y++, rowPtr += templSize.width )
        {
            sqsumBuf[y] = icvCrossCorr_8u_C1( rowPtr, rowPtr, templSize.width );
        }

    }

    /* main loop - through x coordinate of the result */
    for( x = 0; x < resultSize.width; x++ )
    {
        int64 sqsum = 0;
        uchar *imgPtr = imgBuf + x;

        /* update sums and image band buffer */
        if( x > 0 )
        {
            const uchar *src = pImage + x + templSize.width - 1;
            uchar *dst = imgPtr - 1;
            int out_val = dst[0];

            dst += templSize.width;

            for( y = 0; y < roiSize.height; y++, src += imageStep, dst += templSize.width )
            {
                int in_val = src[0];

                sqsumBuf[y] += (in_val - out_val) * (in_val + out_val);
                out_val = dst[0];
                dst[0] = (uchar) in_val;
            }
        }

        for( y = 0; y < templSize.height; y++ )
        {
            sqsum += sqsumBuf[y];
        }

        for( y = 0; y < resultSize.height; y++, imgPtr += templSize.width )
        {
            int64 res = icvCrossCorr_8u_C1( imgPtr, templBuf, winLen );

            if( y > 0 )
            {
                sqsum -= sqsumBuf[y - 1];
                sqsum += sqsumBuf[y + templSize.height - 1];
            }
            resNum[y] = res;
            resDenom[y] = sqsum;
        }



        for( y = 0; y < resultSize.height; y++ )
        {
            double res = ((double) resNum[y]) * templCoeff *
                icvInvSqrt64d( fabs( (double) resDenom[y] ) + FLT_EPSILON );

            pResult[x + y * resultStep] = (float) res;
        }
    }

    return CV_OK;
}


/* -------------------------------------- Coeff --------------------------------------- */

IPCVAPI_IMPL( CvStatus, icvMatchTemplate_Coeff_8u32f_C1R,
              (const uchar * pImage, int imageStep, CvSize roiSize,
               const uchar * pTemplate, int templStep, CvSize templSize,
               float *pResult, int resultStep, void *pBuffer) )
{
    uchar *imgBuf = 0;
    uchar *templBuf = 0;
    int64 *resNum = 0;
    int64 *resDenom = 0;
    int64 *sumBuf = 0;
    int winLen = templSize.width * templSize.height;
    CvSize resultSize = cvSize( roiSize.width - templSize.width + 1,
                                roiSize.height - templSize.height + 1 );
    int64 templSum = 0;
    double winCoeff = 1. / (winLen + DBL_EPSILON);
    int x, y;

    CvStatus result = icvMatchTemplateEntry( pImage, imageStep, roiSize,
                                             pTemplate, templStep, templSize,
                                             pResult, resultStep, pBuffer,
                                             cv8u, 1, 0,
                                             (void **) &imgBuf, (void **) &templBuf,
                                             (void **) &sumBuf, 0,
                                             (void **) &resNum, (void **) &resDenom );

    if( result != CV_OK )
        return result;

    resultStep /= sizeof_float;

    /* calc common statistics for template and image */
    {
        const uchar *rowPtr = (const uchar *) imgBuf;

        templSum = icvSumPixels_8u_C1( templBuf, winLen );

        for( y = 0; y < roiSize.height; y++, rowPtr += templSize.width )
        {
            sumBuf[y] = icvSumPixels_8u_C1( rowPtr, templSize.width );
        }

    }

    /* main loop - through x coordinate of the result */
    for( x = 0; x < resultSize.width; x++ )
    {
        uchar *imgPtr = imgBuf + x;
        int64 sum = 0;

        /* update sums and image band buffer */
        if( x > 0 )
        {
            const uchar *src = pImage + x + templSize.width - 1;
            uchar *dst = imgPtr - 1;
            int out_val = dst[0];

            dst += templSize.width;

            for( y = 0; y < roiSize.height; y++, src += imageStep, dst += templSize.width )
            {
                int in_val = src[0];

                sumBuf[y] += in_val - out_val;
                out_val = dst[0];
                dst[0] = (uchar) in_val;
            }
        }

        for( y = 0; y < templSize.height; y++ )
        {
            sum += sumBuf[y];
        }

        for( y = 0; y < resultSize.height; y++, imgPtr += templSize.width )
        {
            int64 res = icvCrossCorr_8u_C1( imgPtr, templBuf, winLen );

            if( y > 0 )
            {
                sum -= sumBuf[y - 1];
                sum += sumBuf[y + templSize.height - 1];
            }
            resNum[y] = res;
            resDenom[y] = sum;
        }



        for( y = 0; y < resultSize.height; y++ )
        {
            double res = ((double) resNum[y]) - winCoeff * templSum * ((double) resDenom[y]);

            pResult[x + y * resultStep] = (float) res;
        }
    }

    return CV_OK;
}


/* ------------------------------------ CoeffNormed ----------------------------------- */

IPCVAPI_IMPL( CvStatus, icvMatchTemplate_CoeffNormed_8u32f_C1R,
              (const uchar * pImage, int imageStep, CvSize roiSize,
               const uchar * pTemplate, int templStep, CvSize templSize,
               float *pResult, int resultStep, void *pBuffer) )
{
    uchar *imgBuf = 0;
    uchar *templBuf = 0;
    int64 *sumBuf = 0;
    int64 *sqsumBuf = 0;
    int64 *resNum = 0;
    int64 *resDenom = 0;
    int64 templSum = 0;
    double templCoeff = 0;

    int winLen = templSize.width * templSize.height;
    double winCoeff = 1. / (winLen + DBL_EPSILON);

    CvSize resultSize = cvSize( roiSize.width - templSize.width + 1,
                                roiSize.height - templSize.height + 1 );
    int x, y;

    CvStatus result = icvMatchTemplateEntry( pImage, imageStep, roiSize,
                                             pTemplate, templStep, templSize,
                                             pResult, resultStep, pBuffer,
                                             cv8u, 1, 1,
                                             (void **) &imgBuf, (void **) &templBuf,
                                             (void **) &sumBuf, (void **) &sqsumBuf,
                                             (void **) &resNum, (void **) &resDenom );

    if( result != CV_OK )
        return result;

    resultStep /= sizeof_float;

    /* calc common statistics for template and image */
    {
        const uchar *rowPtr = (const uchar *) imgBuf;
        int64 templSqsum = icvCrossCorr_8u_C1( templBuf, templBuf, winLen );

        templSum = icvSumPixels_8u_C1( templBuf, winLen );


        templCoeff = (double) templSqsum - ((double) templSum) * templSum * winCoeff;
        templCoeff = icvInvSqrt64d( fabs( templCoeff ) + FLT_EPSILON );

        for( y = 0; y < roiSize.height; y++, rowPtr += templSize.width )
        {
            sumBuf[y] = icvSumPixels_8u_C1( rowPtr, templSize.width );
            sqsumBuf[y] = icvCrossCorr_8u_C1( rowPtr, rowPtr, templSize.width );
        }

    }

    /* main loop - through x coordinate of the result */
    for( x = 0; x < resultSize.width; x++ )
    {
        int64 sum = 0;
        int64 sqsum = 0;
        uchar *imgPtr = imgBuf + x;

        /* update sums and image band buffer */
        if( x > 0 )
        {
            const uchar *src = pImage + x + templSize.width - 1;
            uchar *dst = imgPtr - 1;
            int out_val = dst[0];

            dst += templSize.width;

            for( y = 0; y < roiSize.height; y++, src += imageStep, dst += templSize.width )
            {
                int in_val = src[0];

                sumBuf[y] += in_val - out_val;
                sqsumBuf[y] += (in_val - out_val) * (in_val + out_val);
                out_val = dst[0];
                dst[0] = (uchar) in_val;
            }
        }

        for( y = 0; y < templSize.height; y++ )
        {
            sum += sumBuf[y];
            sqsum += sqsumBuf[y];
        }

        for( y = 0; y < resultSize.height; y++, imgPtr += templSize.width )
        {
            int64 res = icvCrossCorr_8u_C1( imgPtr, templBuf, winLen );

            if( y > 0 )
            {
                sum -= sumBuf[y - 1];
                sum += sumBuf[y + templSize.height - 1];
                sqsum -= sqsumBuf[y - 1];
                sqsum += sqsumBuf[y + templSize.height - 1];
            }
            resNum[y] = res;
            resDenom[y] = sum;
            resDenom[y + resultSize.height] = sqsum;
        }



        for( y = 0; y < resultSize.height; y++ )
        {
            double sum = ((double) resDenom[y]);
            double wsum = winCoeff * sum;
            double res = ((double) resNum[y]) - wsum * templSum;
            double nrm_s = ((double) resDenom[y + resultSize.height]) - wsum * sum;

            res *= templCoeff * icvInvSqrt64d( fabs( nrm_s ) + FLT_EPSILON );
            pResult[x + y * resultStep] = (float) res;
        }
    }

    return CV_OK;
}


/****************************************************************************************\
*                                      8s flavor                                         *
\****************************************************************************************/

/* --------------------------------------- SqDiff ------------------------------------- */

IPCVAPI_IMPL( CvStatus, icvMatchTemplate_SqDiff_8s32f_C1R,
              (const char *pImage, int imageStep, CvSize roiSize,
               const char *pTemplate, int templStep, CvSize templSize,
               float *pResult, int resultStep, void *pBuffer) )
{
    char *imgBuf = 0;
    char *templBuf = 0;
    int64 *resNum = 0;
    int winLen = templSize.width * templSize.height;
    CvSize resultSize = cvSize( roiSize.width - templSize.width + 1,
                                roiSize.height - templSize.height + 1 );
    int x, y;

    CvStatus result = icvMatchTemplateEntry( pImage, imageStep, roiSize,
                                             pTemplate, templStep, templSize,
                                             pResult, resultStep, pBuffer,
                                             cv8s, 0, 0,
                                             (void **) &imgBuf, (void **) &templBuf,
                                             0, 0, (void **) &resNum, 0 );

    if( result != CV_OK )
        return result;

    resultStep /= sizeof_float;

    /* main loop - through x coordinate of the result */
    for( x = 0; x < resultSize.width; x++ )
    {
        char *imgPtr = imgBuf + x;

        /* update sums and image band buffer */
        if( x > 0 )
        {
            const char *src = pImage + x + templSize.width - 1;
            char *dst = imgPtr - 1;

            dst += templSize.width;

            for( y = 0; y < roiSize.height; y++, src += imageStep, dst += templSize.width )
            {
                dst[0] = src[0];
            }
        }

        for( y = 0; y < resultSize.height; y++, imgPtr += templSize.width )
        {
            int64 res = icvCmpBlocksL2_8s_C1( imgPtr, templBuf, winLen );

            resNum[y] = res;
        }



        for( y = 0; y < resultSize.height; y++ )
        {
            pResult[x + y * resultStep] = (float) resNum[y];
        }
    }

    return CV_OK;
}


/* ----------------------------------- SqDiffNormed ----------------------------------- */

IPCVAPI_IMPL( CvStatus, icvMatchTemplate_SqDiffNormed_8s32f_C1R,
              (const char *pImage, int imageStep, CvSize roiSize,
               const char *pTemplate, int templStep, CvSize templSize,
               float *pResult, int resultStep, void *pBuffer) )
{
    char *imgBuf = 0;
    char *templBuf = 0;
    int64 *sqsumBuf = 0;
    int64 *resNum = 0;
    int64 *resDenom = 0;
    double templCoeff = 0;

    int winLen = templSize.width * templSize.height;
    CvSize resultSize = cvSize( roiSize.width - templSize.width + 1,
                                roiSize.height - templSize.height + 1 );
    int x, y;

    CvStatus result = icvMatchTemplateEntry( pImage, imageStep, roiSize,
                                             pTemplate, templStep, templSize,
                                             pResult, resultStep, pBuffer,
                                             cv8s, 0, 1,
                                             (void **) &imgBuf, (void **) &templBuf,
                                             0, (void **) &sqsumBuf,
                                             (void **) &resNum, (void **) &resDenom );

    if( result != CV_OK )
        return result;

    resultStep /= sizeof_float;

    /* calc common statistics for template and image */
    {
        const char *rowPtr = (const char *) imgBuf;
        int64 templSqsum = icvCrossCorr_8s_C1( templBuf, templBuf, winLen );


        templCoeff = (double) templSqsum;
        templCoeff = icvInvSqrt64d( fabs( templCoeff ) + FLT_EPSILON );

        for( y = 0; y < roiSize.height; y++, rowPtr += templSize.width )
        {
            sqsumBuf[y] = icvCrossCorr_8s_C1( rowPtr, rowPtr, templSize.width );
        }

    }

    /* main loop - through x coordinate of the result */
    for( x = 0; x < resultSize.width; x++ )
    {
        int64 sqsum = 0;
        char *img_ptr = imgBuf + x;

        /* update sums and image band buffer */
        if( x > 0 )
        {
            const char *src = pImage + x + templSize.width - 1;
            char *dst = img_ptr - 1;
            int out_val = dst[0];

            dst += templSize.width;

            for( y = 0; y < roiSize.height; y++, src += imageStep, dst += templSize.width )
            {
                int in_val = src[0];

                sqsumBuf[y] += (in_val - out_val) * (in_val + out_val);
                out_val = dst[0];
                dst[0] = (char) in_val;
            }
        }

        for( y = 0; y < templSize.height; y++ )
        {
            sqsum += sqsumBuf[y];
        }

        for( y = 0; y < resultSize.height; y++, img_ptr += templSize.width )
        {
            int64 res = icvCmpBlocksL2_8s_C1( img_ptr, templBuf, winLen );

            if( y > 0 )
            {
                sqsum -= sqsumBuf[y - 1];
                sqsum += sqsumBuf[y + templSize.height - 1];
            }
            resNum[y] = res;
            resDenom[y] = sqsum;
        }



        for( y = 0; y < resultSize.height; y++ )
        {
            double res = ((double) resNum[y]) * templCoeff *
                icvInvSqrt64d( fabs( (double) resDenom[y] ) + FLT_EPSILON );

            pResult[x + y * resultStep] = (float) res;
        }
    }

    return CV_OK;
}

/* -------------------------------------- Corr ---------------------------------------- */

IPCVAPI_IMPL( CvStatus, icvMatchTemplate_Corr_8s32f_C1R,
              (const char *pImage, int imageStep, CvSize roiSize,
               const char *pTemplate, int templStep, CvSize templSize,
               float *pResult, int resultStep, void *pBuffer) )
{
    char *imgBuf = 0;
    char *templBuf = 0;
    int64 *resNum = 0;
    int winLen = templSize.width * templSize.height;
    CvSize resultSize = cvSize( roiSize.width - templSize.width + 1,
                                roiSize.height - templSize.height + 1 );
    int x, y;

    CvStatus result = icvMatchTemplateEntry( pImage, imageStep, roiSize,
                                             pTemplate, templStep, templSize,
                                             pResult, resultStep, pBuffer,
                                             cv8s, 0, 0,
                                             (void **) &imgBuf, (void **) &templBuf,
                                             0, 0, (void **) &resNum, 0 );

    if( result != CV_OK )
        return result;

    resultStep /= sizeof_float;

    /* main loop - through x coordinate of the result */
    for( x = 0; x < resultSize.width; x++ )
    {
        char *imgPtr = imgBuf + x;

        /* update sums and image band buffer */
        if( x > 0 )
        {
            const char *src = pImage + x + templSize.width - 1;
            char *dst = imgPtr - 1;

            dst += templSize.width;

            for( y = 0; y < roiSize.height; y++, src += imageStep, dst += templSize.width )
            {
                dst[0] = src[0];
            }
        }

        for( y = 0; y < resultSize.height; y++, imgPtr += templSize.width )
        {
            int64 res = icvCrossCorr_8s_C1( imgPtr, templBuf, winLen );

            resNum[y] = res;
        }



        for( y = 0; y < resultSize.height; y++ )
        {
            pResult[x + y * resultStep] = (float) resNum[y];
        }
    }

    return CV_OK;
}

/* ------------------------------------ CorrNormed ------------------------------------ */

IPCVAPI_IMPL( CvStatus, icvMatchTemplate_CorrNormed_8s32f_C1R,
              (const char *pImage, int imageStep, CvSize roiSize,
               const char *pTemplate, int templStep, CvSize templSize,
               float *pResult, int resultStep, void *pBuffer) )
{
    char *imgBuf = 0;
    char *templBuf = 0;
    int64 *sqsumBuf = 0;
    int64 *resNum = 0;
    int64 *resDenom = 0;
    double templCoeff = 0;

    int winLen = templSize.width * templSize.height;
    CvSize resultSize = cvSize( roiSize.width - templSize.width + 1,
                                roiSize.height - templSize.height + 1 );
    int x, y;

    CvStatus result = icvMatchTemplateEntry( pImage, imageStep, roiSize,
                                             pTemplate, templStep, templSize,
                                             pResult, resultStep, pBuffer,
                                             cv8s, 0, 1,
                                             (void **) &imgBuf, (void **) &templBuf,
                                             0, (void **) &sqsumBuf,
                                             (void **) &resNum, (void **) &resDenom );

    if( result != CV_OK )
        return result;

    resultStep /= sizeof_float;

    /* calc common statistics for template and image */
    {
        const char *rowPtr = (const char *) imgBuf;
        int64 templSqsum = icvCrossCorr_8s_C1( templBuf, templBuf, winLen );


        templCoeff = (double) templSqsum;
        templCoeff = icvInvSqrt64d( fabs( templCoeff ) + FLT_EPSILON );

        for( y = 0; y < roiSize.height; y++, rowPtr += templSize.width )
        {
            sqsumBuf[y] = icvCrossCorr_8s_C1( rowPtr, rowPtr, templSize.width );
        }

    }

    /* main loop - through x coordinate of the result */
    for( x = 0; x < resultSize.width; x++ )
    {
        int64 sqsum = 0;
        char *imgPtr = imgBuf + x;

        /* update sums and image band buffer */
        if( x > 0 )
        {
            const char *src = pImage + x + templSize.width - 1;
            char *dst = imgPtr - 1;
            int out_val = dst[0];

            dst += templSize.width;

            for( y = 0; y < roiSize.height; y++, src += imageStep, dst += templSize.width )
            {
                int in_val = src[0];

                sqsumBuf[y] += (in_val - out_val) * (in_val + out_val);
                out_val = dst[0];
                dst[0] = (char) in_val;
            }
        }

        for( y = 0; y < templSize.height; y++ )
        {
            sqsum += sqsumBuf[y];
        }

        for( y = 0; y < resultSize.height; y++, imgPtr += templSize.width )
        {
            int64 res = icvCrossCorr_8s_C1( imgPtr, templBuf, winLen );

            if( y > 0 )
            {
                sqsum -= sqsumBuf[y - 1];
                sqsum += sqsumBuf[y + templSize.height - 1];
            }
            resNum[y] = res;
            resDenom[y] = sqsum;
        }



        for( y = 0; y < resultSize.height; y++ )
        {
            double res = ((double) resNum[y]) * templCoeff *
                icvInvSqrt64d( fabs( (double) resDenom[y] ) + FLT_EPSILON );

            pResult[x + y * resultStep] = (float) res;
        }
    }

    return CV_OK;
}


/* -------------------------------------- Coeff --------------------------------------- */

IPCVAPI_IMPL( CvStatus, icvMatchTemplate_Coeff_8s32f_C1R,
              (const char *pImage, int imageStep, CvSize roiSize,
               const char *pTemplate, int templStep, CvSize templSize,
               float *pResult, int resultStep, void *pBuffer) )
{
    char *imgBuf = 0;
    char *templBuf = 0;
    int64 *resNum = 0;
    int64 *resDenom = 0;
    int64 *sumBuf = 0;
    int winLen = templSize.width * templSize.height;
    CvSize resultSize = cvSize( roiSize.width - templSize.width + 1,
                                roiSize.height - templSize.height + 1 );
    int64 templSum = 0;
    double winCoeff = 1. / (winLen + DBL_EPSILON);
    int x, y;

    CvStatus result = icvMatchTemplateEntry( pImage, imageStep, roiSize,
                                             pTemplate, templStep, templSize,
                                             pResult, resultStep, pBuffer,
                                             cv8s, 1, 0,
                                             (void **) &imgBuf, (void **) &templBuf,
                                             (void **) &sumBuf, 0,
                                             (void **) &resNum, (void **) &resDenom );

    if( result != CV_OK )
        return result;

    resultStep /= sizeof_float;

    /* calc common statistics for template and image */
    {
        const char *rowPtr = (const char *) imgBuf;

        templSum = icvSumPixels_8s_C1( templBuf, winLen );

        for( y = 0; y < roiSize.height; y++, rowPtr += templSize.width )
        {
            sumBuf[y] = icvSumPixels_8s_C1( rowPtr, templSize.width );
        }

    }

    /* main loop - through x coordinate of the result */
    for( x = 0; x < resultSize.width; x++ )
    {
        char *imgPtr = imgBuf + x;
        int64 sum = 0;

        /* update sums and image band buffer */
        if( x > 0 )
        {
            const char *src = pImage + x + templSize.width - 1;
            char *dst = imgPtr - 1;
            int out_val = dst[0];

            dst += templSize.width;

            for( y = 0; y < roiSize.height; y++, src += imageStep, dst += templSize.width )
            {
                int in_val = src[0];

                sumBuf[y] += in_val - out_val;
                out_val = dst[0];
                dst[0] = (char) in_val;
            }
        }

        for( y = 0; y < templSize.height; y++ )
        {
            sum += sumBuf[y];
        }

        for( y = 0; y < resultSize.height; y++, imgPtr += templSize.width )
        {
            int64 res = icvCrossCorr_8s_C1( imgPtr, templBuf, winLen );

            if( y > 0 )
            {
                sum -= sumBuf[y - 1];
                sum += sumBuf[y + templSize.height - 1];
            }
            resNum[y] = res;
            resDenom[y] = sum;
        }



        for( y = 0; y < resultSize.height; y++ )
        {
            double res = ((double) resNum[y]) - winCoeff * templSum * ((double) resDenom[y]);

            pResult[x + y * resultStep] = (float) res;
        }
    }

    return CV_OK;
}


/* ------------------------------------ CoeffNormed ----------------------------------- */

IPCVAPI_IMPL( CvStatus, icvMatchTemplate_CoeffNormed_8s32f_C1R,
              (const char *pImage, int imageStep, CvSize roiSize,
               const char *pTemplate, int templStep, CvSize templSize,
               float *pResult, int resultStep, void *pBuffer) )
{
    char *imgBuf = 0;
    char *templBuf = 0;
    int64 *sumBuf = 0;
    int64 *sqsumBuf = 0;
    int64 *resNum = 0;
    int64 *resDenom = 0;
    int64 templSum = 0;
    double templCoeff = 0;

    int winLen = templSize.width * templSize.height;
    double winCoeff = 1. / (winLen + DBL_EPSILON);

    CvSize resultSize = cvSize( roiSize.width - templSize.width + 1,
                                roiSize.height - templSize.height + 1 );
    int x, y;

    CvStatus result = icvMatchTemplateEntry( pImage, imageStep, roiSize,
                                             pTemplate, templStep, templSize,
                                             pResult, resultStep, pBuffer,
                                             cv8s, 1, 1,
                                             (void **) &imgBuf, (void **) &templBuf,
                                             (void **) &sumBuf, (void **) &sqsumBuf,
                                             (void **) &resNum, (void **) &resDenom );

    if( result != CV_OK )
        return result;

    resultStep /= sizeof_float;

    /* calc common statistics for template and image */
    {
        const char *rowPtr = (const char *) imgBuf;
        int64 templSqsum = icvCrossCorr_8s_C1( templBuf, templBuf, winLen );

        templSum = icvSumPixels_8s_C1( templBuf, winLen );


        templCoeff = (double) templSqsum - ((double) templSum) * templSum * winCoeff;
        templCoeff = icvInvSqrt64d( fabs( templCoeff ) + FLT_EPSILON );

        for( y = 0; y < roiSize.height; y++, rowPtr += templSize.width )
        {
            sumBuf[y] = icvSumPixels_8s_C1( rowPtr, templSize.width );
            sqsumBuf[y] = icvCrossCorr_8s_C1( rowPtr, rowPtr, templSize.width );
        }

    }

    /* main loop - through x coordinate of the result */
    for( x = 0; x < resultSize.width; x++ )
    {
        int64 sum = 0;
        int64 sqsum = 0;
        char *imgPtr = imgBuf + x;

        /* update sums and image band buffer */
        if( x > 0 )
        {
            const char *src = pImage + x + templSize.width - 1;
            char *dst = imgPtr - 1;
            int out_val = dst[0];

            dst += templSize.width;

            for( y = 0; y < roiSize.height; y++, src += imageStep, dst += templSize.width )
            {
                int in_val = src[0];

                sumBuf[y] += in_val - out_val;
                sqsumBuf[y] += (in_val - out_val) * (in_val + out_val);
                out_val = dst[0];
                dst[0] = (char) in_val;
            }
        }

        for( y = 0; y < templSize.height; y++ )
        {
            sum += sumBuf[y];
            sqsum += sqsumBuf[y];
        }

        for( y = 0; y < resultSize.height; y++, imgPtr += templSize.width )
        {
            int64 res = icvCrossCorr_8s_C1( imgPtr, templBuf, winLen );

            if( y > 0 )
            {
                sum -= sumBuf[y - 1];
                sum += sumBuf[y + templSize.height - 1];
                sqsum -= sqsumBuf[y - 1];
                sqsum += sqsumBuf[y + templSize.height - 1];
            }
            resNum[y] = res;
            resDenom[y] = sum;
            resDenom[y + resultSize.height] = sqsum;
        }



        for( y = 0; y < resultSize.height; y++ )
        {
            double sum = ((double) resDenom[y]);
            double wsum = winCoeff * sum;
            double res = ((double) resNum[y]) - wsum * templSum;
            double nrm_s = ((double) resDenom[y + resultSize.height]) - wsum * sum;

            res *= templCoeff * icvInvSqrt64d( fabs( nrm_s ) + FLT_EPSILON );
            pResult[x + y * resultStep] = (float) res;
        }
    }

    return CV_OK;
}


/****************************************************************************************\
*                                      32f flavor                                        *
\****************************************************************************************/

/* --------------------------------------- SqDiff ------------------------------------- */

IPCVAPI_IMPL( CvStatus, icvMatchTemplate_SqDiff_32f_C1R,
              (const float *pImage, int imageStep, CvSize roiSize,
               const float *pTemplate, int templStep, CvSize templSize,
               float *pResult, int resultStep, void *pBuffer) )
{
    float *imgBuf = 0;
    float *templBuf = 0;
    double *resNum = 0;
    int winLen = templSize.width * templSize.height;
    CvSize resultSize = cvSize( roiSize.width - templSize.width + 1,
                                roiSize.height - templSize.height + 1 );
    int x, y;

    CvStatus result = icvMatchTemplateEntry( pImage, imageStep, roiSize,
                                             pTemplate, templStep, templSize,
                                             pResult, resultStep, pBuffer,
                                             cv32f, 0, 0,
                                             (void **) &imgBuf, (void **) &templBuf,
                                             0, 0, (void **) &resNum, 0 );

    if( result != CV_OK )
        return result;

    imageStep /= sizeof_float;
    templStep /= sizeof_float;
    resultStep /= sizeof_float;

    /* main loop - through x coordinate of the result */
    for( x = 0; x < resultSize.width; x++ )
    {
        float *imgPtr = imgBuf + x;

        /* update sums and image band buffer */
        if( x > 0 )
        {
            const float *src = pImage + x + templSize.width - 1;
            float *dst = imgPtr - 1;

            dst += templSize.width;

            for( y = 0; y < roiSize.height; y++, src += imageStep, dst += templSize.width )
            {
                dst[0] = src[0];
            }
        }

        for( y = 0; y < resultSize.height; y++, imgPtr += templSize.width )
        {
            double res = icvCmpBlocksL2_32f_C1( imgPtr, templBuf, winLen );

            resNum[y] = res;
        }

        for( y = 0; y < resultSize.height; y++ )
        {
            pResult[x + y * resultStep] = (float) resNum[y];
        }
    }

    return CV_OK;
}


/* ----------------------------------- SqDiffNormed ----------------------------------- */

IPCVAPI_IMPL( CvStatus, icvMatchTemplate_SqDiffNormed_32f_C1R,
              (const float *pImage, int imageStep, CvSize roiSize,
               const float *pTemplate, int templStep, CvSize templSize,
               float *pResult, int resultStep, void *pBuffer) )
{
    float *imgBuf = 0;
    float *templBuf = 0;
    double *sqsumBuf = 0;
    double *resNum = 0;
    double *resDenom = 0;
    double templCoeff = 0;

    int winLen = templSize.width * templSize.height;
    CvSize resultSize = cvSize( roiSize.width - templSize.width + 1,
                                roiSize.height - templSize.height + 1 );
    int x, y;

    CvStatus result = icvMatchTemplateEntry( pImage, imageStep, roiSize,
                                             pTemplate, templStep, templSize,
                                             pResult, resultStep, pBuffer,
                                             cv32f, 0, 1,
                                             (void **) &imgBuf, (void **) &templBuf,
                                             0, (void **) &sqsumBuf,
                                             (void **) &resNum, (void **) &resDenom );

    if( result != CV_OK )
        return result;

    imageStep /= sizeof_float;
    templStep /= sizeof_float;
    resultStep /= sizeof_float;

    /* calc common statistics for template and image */
    {
        const float *rowPtr = (const float *) imgBuf;
        double templSqsum = icvCrossCorr_32f_C1( templBuf, templBuf, winLen );


        templCoeff = (double) templSqsum;
        templCoeff = icvInvSqrt64d( fabs( templCoeff ) + FLT_EPSILON );

        for( y = 0; y < roiSize.height; y++, rowPtr += templSize.width )
        {
            sqsumBuf[y] = icvCrossCorr_32f_C1( rowPtr, rowPtr, templSize.width );
        }

    }

    /* main loop - through x coordinate of the result */
    for( x = 0; x < resultSize.width; x++ )
    {
        double sqsum = 0;
        float *img_ptr = imgBuf + x;

        /* update sums and image band buffer */
        if( x > 0 )
        {
            const float *src = pImage + x + templSize.width - 1;
            float *dst = img_ptr - 1;
            float out_val = dst[0];

            dst += templSize.width;

            for( y = 0; y < roiSize.height; y++, src += imageStep, dst += templSize.width )
            {
                float in_val = src[0];

                sqsumBuf[y] += (in_val - out_val) * (in_val + out_val);
                out_val = dst[0];
                dst[0] = (float) in_val;
            }
        }

        for( y = 0; y < templSize.height; y++ )
        {
            sqsum += sqsumBuf[y];
        }

        for( y = 0; y < resultSize.height; y++, img_ptr += templSize.width )
        {
            double res = icvCmpBlocksL2_32f_C1( img_ptr, templBuf, winLen );

            if( y > 0 )
            {
                sqsum -= sqsumBuf[y - 1];
                sqsum += sqsumBuf[y + templSize.height - 1];
            }
            resNum[y] = res;
            resDenom[y] = sqsum;
        }

        for( y = 0; y < resultSize.height; y++ )
        {
            double res = ((double) resNum[y]) * templCoeff *
                icvInvSqrt64d( fabs( (double) resDenom[y] ) + FLT_EPSILON );

            pResult[x + y * resultStep] = (float) res;
        }
    }

    return CV_OK;
}

/* -------------------------------------- Corr ---------------------------------------- */

IPCVAPI_IMPL( CvStatus, icvMatchTemplate_Corr_32f_C1R,
              (const float *pImage, int imageStep, CvSize roiSize,
               const float *pTemplate, int templStep, CvSize templSize,
               float *pResult, int resultStep, void *pBuffer) )
{
    float *imgBuf = 0;
    float *templBuf = 0;
    double *resNum = 0;
    int winLen = templSize.width * templSize.height;
    CvSize resultSize = cvSize( roiSize.width - templSize.width + 1,
                                roiSize.height - templSize.height + 1 );
    int x, y;

    CvStatus result = icvMatchTemplateEntry( pImage, imageStep, roiSize,
                                             pTemplate, templStep, templSize,
                                             pResult, resultStep, pBuffer,
                                             cv32f, 0, 0,
                                             (void **) &imgBuf, (void **) &templBuf,
                                             0, 0, (void **) &resNum, 0 );

    if( result != CV_OK )
        return result;

    imageStep /= sizeof_float;
    templStep /= sizeof_float;
    resultStep /= sizeof_float;

    /* main loop - through x coordinate of the result */
    for( x = 0; x < resultSize.width; x++ )
    {
        float *imgPtr = imgBuf + x;

        /* update sums and image band buffer */
        if( x > 0 )
        {
            const float *src = pImage + x + templSize.width - 1;
            float *dst = imgPtr - 1;

            dst += templSize.width;

            for( y = 0; y < roiSize.height; y++, src += imageStep, dst += templSize.width )
            {
                dst[0] = src[0];
            }
        }

        for( y = 0; y < resultSize.height; y++, imgPtr += templSize.width )
        {
            double res = icvCrossCorr_32f_C1( imgPtr, templBuf, winLen );

            resNum[y] = res;
        }

        for( y = 0; y < resultSize.height; y++ )
        {
            pResult[x + y * resultStep] = (float) resNum[y];
        }
    }

    return CV_OK;
}

/* ------------------------------------ CorrNormed ------------------------------------ */

IPCVAPI_IMPL( CvStatus, icvMatchTemplate_CorrNormed_32f_C1R,
              (const float *pImage, int imageStep, CvSize roiSize,
               const float *pTemplate, int templStep, CvSize templSize,
               float *pResult, int resultStep, void *pBuffer) )
{
    float *imgBuf = 0;
    float *templBuf = 0;
    double *sqsumBuf = 0;
    double *resNum = 0;
    double *resDenom = 0;
    double templCoeff = 0;

    int winLen = templSize.width * templSize.height;
    CvSize resultSize = cvSize( roiSize.width - templSize.width + 1,
                                roiSize.height - templSize.height + 1 );
    int x, y;

    CvStatus result = icvMatchTemplateEntry( pImage, imageStep, roiSize,
                                             pTemplate, templStep, templSize,
                                             pResult, resultStep, pBuffer,
                                             cv32f, 0, 1,
                                             (void **) &imgBuf, (void **) &templBuf,
                                             0, (void **) &sqsumBuf,
                                             (void **) &resNum, (void **) &resDenom );

    if( result != CV_OK )
        return result;

    imageStep /= sizeof_float;
    templStep /= sizeof_float;
    resultStep /= sizeof_float;

    /* calc common statistics for template and image */
    {
        const float *rowPtr = (const float *) imgBuf;
        double templSqsum = icvCrossCorr_32f_C1( templBuf, templBuf, winLen );


        templCoeff = (double) templSqsum;
        templCoeff = icvInvSqrt64d( fabs( templCoeff ) + FLT_EPSILON );

        for( y = 0; y < roiSize.height; y++, rowPtr += templSize.width )
        {
            sqsumBuf[y] = icvCrossCorr_32f_C1( rowPtr, rowPtr, templSize.width );
        }

    }

    /* main loop - through x coordinate of the result */
    for( x = 0; x < resultSize.width; x++ )
    {
        double sqsum = 0;
        float *imgPtr = imgBuf + x;

        /* update sums and image band buffer */
        if( x > 0 )
        {
            const float *src = pImage + x + templSize.width - 1;
            float *dst = imgPtr - 1;
            float out_val = dst[0];

            dst += templSize.width;

            for( y = 0; y < roiSize.height; y++, src += imageStep, dst += templSize.width )
            {
                float in_val = src[0];

                sqsumBuf[y] += (in_val - out_val) * (in_val + out_val);
                out_val = dst[0];
                dst[0] = (float) in_val;
            }
        }

        for( y = 0; y < templSize.height; y++ )
        {
            sqsum += sqsumBuf[y];
        }

        for( y = 0; y < resultSize.height; y++, imgPtr += templSize.width )
        {
            double res = icvCrossCorr_32f_C1( imgPtr, templBuf, winLen );

            if( y > 0 )
            {
                sqsum -= sqsumBuf[y - 1];
                sqsum += sqsumBuf[y + templSize.height - 1];
            }
            resNum[y] = res;
            resDenom[y] = sqsum;
        }

        for( y = 0; y < resultSize.height; y++ )
        {
            double res = ((double) resNum[y]) * templCoeff *
                icvInvSqrt64d( fabs( (double) resDenom[y] ) + FLT_EPSILON );

            pResult[x + y * resultStep] = (float) res;
        }
    }

    return CV_OK;
}


/* -------------------------------------- Coeff --------------------------------------- */

IPCVAPI_IMPL( CvStatus, icvMatchTemplate_Coeff_32f_C1R,
              (const float *pImage, int imageStep, CvSize roiSize,
               const float *pTemplate, int templStep, CvSize templSize,
               float *pResult, int resultStep, void *pBuffer) )
{
    float *imgBuf = 0;
    float *templBuf = 0;
    double *resNum = 0;
    double *resDenom = 0;
    double *sumBuf = 0;
    int winLen = templSize.width * templSize.height;
    CvSize resultSize = cvSize( roiSize.width - templSize.width + 1,
                                roiSize.height - templSize.height + 1 );
    double templSum = 0;
    double winCoeff = 1. / (winLen + DBL_EPSILON);
    int x, y;

    CvStatus result = icvMatchTemplateEntry( pImage, imageStep, roiSize,
                                             pTemplate, templStep, templSize,
                                             pResult, resultStep, pBuffer,
                                             cv32f, 1, 0,
                                             (void **) &imgBuf, (void **) &templBuf,
                                             (void **) &sumBuf, 0,
                                             (void **) &resNum, (void **) &resDenom );

    if( result != CV_OK )
        return result;

    imageStep /= sizeof_float;
    templStep /= sizeof_float;
    resultStep /= sizeof_float;

    /* calc common statistics for template and image */
    {
        const float *rowPtr = (const float *) imgBuf;

        templSum = icvSumPixels_32f_C1( templBuf, winLen );

        for( y = 0; y < roiSize.height; y++, rowPtr += templSize.width )
        {
            sumBuf[y] = icvSumPixels_32f_C1( rowPtr, templSize.width );
        }
    }

    /* main loop - through x coordinate of the result */
    for( x = 0; x < resultSize.width; x++ )
    {
        float *imgPtr = imgBuf + x;
        double sum = 0;

        /* update sums and image band buffer */
        if( x > 0 )
        {
            const float *src = pImage + x + templSize.width - 1;
            float *dst = imgPtr - 1;
            float out_val = dst[0];

            dst += templSize.width;

            for( y = 0; y < roiSize.height; y++, src += imageStep, dst += templSize.width )
            {
                float in_val = src[0];

                sumBuf[y] += in_val - out_val;
                out_val = dst[0];
                dst[0] = (float) in_val;
            }
        }

        for( y = 0; y < templSize.height; y++ )
        {
            sum += sumBuf[y];
        }

        for( y = 0; y < resultSize.height; y++, imgPtr += templSize.width )
        {
            double res = icvCrossCorr_32f_C1( imgPtr, templBuf, winLen );

            if( y > 0 )
            {
                sum -= sumBuf[y - 1];
                sum += sumBuf[y + templSize.height - 1];
            }
            resNum[y] = res;
            resDenom[y] = sum;
        }

        for( y = 0; y < resultSize.height; y++ )
        {
            double res = ((double) resNum[y]) - winCoeff * templSum * ((double) resDenom[y]);

            pResult[x + y * resultStep] = (float) res;
        }
    }

    return CV_OK;
}


/* ------------------------------------ CoeffNormed ----------------------------------- */

IPCVAPI_IMPL( CvStatus, icvMatchTemplate_CoeffNormed_32f_C1R,
              (const float *pImage, int imageStep, CvSize roiSize,
               const float *pTemplate, int templStep, CvSize templSize,
               float *pResult, int resultStep, void *pBuffer) )
{
    float *imgBuf = 0;
    float *templBuf = 0;
    double *sumBuf = 0;
    double *sqsumBuf = 0;
    double *resNum = 0;
    double *resDenom = 0;
    double templCoeff = 0;
    double templSum = 0;

    int winLen = templSize.width * templSize.height;
    double winCoeff = 1. / (winLen + DBL_EPSILON);

    CvSize resultSize = cvSize( roiSize.width - templSize.width + 1,
                                roiSize.height - templSize.height + 1 );
    int x, y;

    CvStatus result = icvMatchTemplateEntry( pImage, imageStep, roiSize,
                                             pTemplate, templStep, templSize,
                                             pResult, resultStep, pBuffer,
                                             cv32f, 1, 1,
                                             (void **) &imgBuf, (void **) &templBuf,
                                             (void **) &sumBuf, (void **) &sqsumBuf,
                                             (void **) &resNum, (void **) &resDenom );

    if( result != CV_OK )
        return result;

    imageStep /= sizeof_float;
    templStep /= sizeof_float;
    resultStep /= sizeof_float;

    /* calc common statistics for template and image */
    {
        const float *rowPtr = (const float *) imgBuf;
        double templSqsum = icvCrossCorr_32f_C1( templBuf, templBuf, winLen );


        templSum = icvSumPixels_32f_C1( templBuf, winLen );
        templCoeff = (double) templSqsum - ((double) templSum) * templSum * winCoeff;
        templCoeff = icvInvSqrt64d( fabs( templCoeff ) + FLT_EPSILON );

        for( y = 0; y < roiSize.height; y++, rowPtr += templSize.width )
        {
            sumBuf[y] = icvSumPixels_32f_C1( rowPtr, templSize.width );
            sqsumBuf[y] = icvCrossCorr_32f_C1( rowPtr, rowPtr, templSize.width );
        }

    }

    /* main loop - through x coordinate of the result */
    for( x = 0; x < resultSize.width; x++ )
    {
        double sum = 0;
        double sqsum = 0;
        float *imgPtr = imgBuf + x;

        /* update sums and image band buffer */
        if( x > 0 )
        {
            const float *src = pImage + x + templSize.width - 1;
            float *dst = imgPtr - 1;
            float out_val = dst[0];

            dst += templSize.width;

            for( y = 0; y < roiSize.height; y++, src += imageStep, dst += templSize.width )
            {
                float in_val = src[0];

                sumBuf[y] += in_val - out_val;
                sqsumBuf[y] += (in_val - out_val) * (in_val + out_val);
                out_val = dst[0];
                dst[0] = (float) in_val;
            }
        }

        for( y = 0; y < templSize.height; y++ )
        {
            sum += sumBuf[y];
            sqsum += sqsumBuf[y];
        }

        for( y = 0; y < resultSize.height; y++, imgPtr += templSize.width )
        {
            double res = icvCrossCorr_32f_C1( imgPtr, templBuf, winLen );

            if( y > 0 )
            {
                sum -= sumBuf[y - 1];
                sum += sumBuf[y + templSize.height - 1];
                sqsum -= sqsumBuf[y - 1];
                sqsum += sqsumBuf[y + templSize.height - 1];
            }
            resNum[y] = res;
            resDenom[y] = sum;
            resDenom[y + resultSize.height] = sqsum;
        }

        for( y = 0; y < resultSize.height; y++ )
        {
            double sum = ((double) resDenom[y]);
            double wsum = winCoeff * sum;
            double res = ((double) resNum[y]) - wsum * templSum;
            double nrm_s = ((double) resDenom[y + resultSize.height]) - wsum * sum;

            res *= templCoeff * icvInvSqrt64d( fabs( nrm_s ) + FLT_EPSILON );
            pResult[x + y * resultStep] = (float) res;
        }
    }

    return CV_OK;
}


/****************************************************************************************\
*                          Calculation of buffer sizes                                   *
\****************************************************************************************/

static CvStatus
icvMatchTemplateGetBufSize( CvSize roiSize, CvSize templSize,
                            CvDataType dataType, int *bufferSize,
                            int is_centered, int is_normed )
{
    int imgBufSize = 0, templBufSize = 0, sumBufSize = 0, sqsumBufSize = 0,
        resNumBufSize = 0, resDenomBufSize = 0;

    if( !bufferSize )
        return CV_NULLPTR_ERR;
    *bufferSize = 0;

    if( templSize.width <= 0 || templSize.height <= 0 ||
        roiSize.width < templSize.width || roiSize.height < templSize.height )
        return CV_BADSIZE_ERR;

    if( dataType != cv8u && dataType != cv8s && dataType != cv32f )
        return CV_BADDEPTH_ERR;

    icvCalculateBufferSizes( roiSize, templSize, dataType,
                             is_centered, is_normed,
                             &imgBufSize, &templBufSize,
                             &sumBufSize, &sqsumBufSize, &resNumBufSize, &resDenomBufSize );

    *bufferSize = imgBufSize + templBufSize + sumBufSize + sqsumBufSize +
        resNumBufSize + resDenomBufSize;

    return CV_OK;
}


IPCVAPI_IMPL( CvStatus, icvMatchTemplateGetBufSize_SqDiff, (CvSize roiSize, CvSize templSize,
                                                            CvDataType dataType,
                                                            int *bufferSize) )
{
    return icvMatchTemplateGetBufSize( roiSize, templSize, dataType, bufferSize, 0, 0 );
}


IPCVAPI_IMPL( CvStatus, icvMatchTemplateGetBufSize_SqDiffNormed,
              (CvSize roiSize, CvSize templSize, CvDataType dataType, int *bufferSize) )
{
    return icvMatchTemplateGetBufSize( roiSize, templSize, dataType, bufferSize, 0, 1 );
}


IPCVAPI_IMPL( CvStatus, icvMatchTemplateGetBufSize_Corr, (CvSize roiSize, CvSize templSize,
                                                          CvDataType dataType,
                                                          int *bufferSize) )
{
    return icvMatchTemplateGetBufSize( roiSize, templSize, dataType, bufferSize, 0, 0 );
}


IPCVAPI_IMPL( CvStatus, icvMatchTemplateGetBufSize_CorrNormed,
              (CvSize roiSize, CvSize templSize, CvDataType dataType, int *bufferSize) )
{
    return icvMatchTemplateGetBufSize( roiSize, templSize, dataType, bufferSize, 0, 1 );
}


IPCVAPI_IMPL( CvStatus, icvMatchTemplateGetBufSize_Coeff, (CvSize roiSize, CvSize templSize,
                                                           CvDataType dataType,
                                                           int *bufferSize) )
{
    return icvMatchTemplateGetBufSize( roiSize, templSize, dataType, bufferSize, 1, 0 );
}


IPCVAPI_IMPL( CvStatus, icvMatchTemplateGetBufSize_CoeffNormed,
              (CvSize roiSize, CvSize templSize, CvDataType dataType, int *bufferSize) )
{
    return icvMatchTemplateGetBufSize( roiSize, templSize, dataType, bufferSize, 1, 1 );
}


/****************************************************************************************\
*                                Comparison/Matching                                     *
\****************************************************************************************/

typedef CvStatus( CV_STDCALL * CvMatchBufSizeFunc ) (CvSize roiSize,
                                                      CvSize templSize,
                                                      CvDataType dataType, int *bufferSize);

typedef CvStatus( CV_STDCALL * CvMatchTemplFunc ) (const void *pImage,
                                                   int imageStep, CvSize roiSize,
                                                   const void *pTemplate,
                                                   int templStep, CvSize templSize,
                                                   void *pResult, int resultStep,
                                                   void *pBuffer);


static  void  icvInitMatchTemplTable( void** bufSizeTab, void** funcTab )
{
    bufSizeTab[0] = (void*)icvMatchTemplateGetBufSize_SqDiff;
    bufSizeTab[1] = (void*)icvMatchTemplateGetBufSize_SqDiffNormed;
    bufSizeTab[2] = (void*)icvMatchTemplateGetBufSize_Corr;
    bufSizeTab[3] = (void*)icvMatchTemplateGetBufSize_CorrNormed;
    bufSizeTab[4] = (void*)icvMatchTemplateGetBufSize_Coeff;
    bufSizeTab[5] = (void*)icvMatchTemplateGetBufSize_CoeffNormed;

    funcTab[0]  = (void*)icvMatchTemplate_SqDiff_8u32f_C1R;
    funcTab[1]  = (void*)icvMatchTemplate_SqDiffNormed_8u32f_C1R;
    funcTab[2]  = (void*)icvMatchTemplate_Corr_8u32f_C1R;
    funcTab[3]  = (void*)icvMatchTemplate_CorrNormed_8u32f_C1R;
    funcTab[4]  = (void*)icvMatchTemplate_Coeff_8u32f_C1R;
    funcTab[5]  = (void*)icvMatchTemplate_CoeffNormed_8u32f_C1R;

    funcTab[6]  = (void*)icvMatchTemplate_SqDiff_8s32f_C1R;
    funcTab[7]  = (void*)icvMatchTemplate_SqDiffNormed_8s32f_C1R;
    funcTab[8]  = (void*)icvMatchTemplate_Corr_8s32f_C1R;
    funcTab[9]  = (void*)icvMatchTemplate_CorrNormed_8s32f_C1R;
    funcTab[10] = (void*)icvMatchTemplate_Coeff_8s32f_C1R;
    funcTab[11] = (void*)icvMatchTemplate_CoeffNormed_8s32f_C1R;

    funcTab[12] = (void*)icvMatchTemplate_SqDiff_32f_C1R;
    funcTab[13] = (void*)icvMatchTemplate_SqDiffNormed_32f_C1R;
    funcTab[14] = (void*)icvMatchTemplate_Corr_32f_C1R;
    funcTab[15] = (void*)icvMatchTemplate_CorrNormed_32f_C1R;
    funcTab[16] = (void*)icvMatchTemplate_Coeff_32f_C1R;
    funcTab[17] = (void*)icvMatchTemplate_CoeffNormed_32f_C1R;
}


CV_IMPL void
cvMatchTemplate( const void* arr, const void* templarr, void* resultarr,
                 CvTemplMatchMethod method )
{
    static void* bufSizeFuncs[6];
    static void* funcs[18];
    static int inittab = 0;

    void* buffer = 0;
    CV_FUNCNAME( "cvMatchTemplate" );

    __BEGIN__;

    int coi1 = 0, coi2 = 0;
    CvMat stub, *img = (CvMat*)arr;
    CvMat tstub, *templ = (CvMat*)templarr;
    CvMat resstub, *result = (CvMat*)resultarr;

    CvSize imgSize, templSize;
    CvDataType dataType = cv16s;
    int dataOffset = -1;
    int bufferSize;
    int imethod = (int) method;
    int img_step, templ_step, result_step;

    CvMatchBufSizeFunc bufSizeFunc = 0;
    CvMatchTemplFunc func = 0;

    if( !inittab )
    {
        icvInitMatchTemplTable( bufSizeFuncs, funcs );
        inittab = 1;
    }

    CV_CALL( img = cvGetMat( img, &stub, &coi1 ));
    CV_CALL( templ = cvGetMat( templ, &tstub, &coi2 ));
    CV_CALL( result = cvGetMat( result, &resstub ));

    if( CV_MAT_CN( img->type ) != 1 ||
        CV_MAT_DEPTH( img->type ) != CV_8U &&
        CV_MAT_DEPTH( img->type ) != CV_8S &&
        CV_MAT_DEPTH( img->type ) != CV_32F )
        CV_ERROR( CV_StsUnsupportedFormat, "" );

    if( !CV_ARE_TYPES_EQ( img, templ ))
        CV_ERROR( CV_StsUnmatchedSizes, "" );

    if( CV_MAT_TYPE( result->type ) != CV_32FC1 )
        CV_ERROR( CV_StsUnsupportedFormat, "" );

    if( result->width != img->width - templ->width + 1 ||
        result->height != img->height - templ->height + 1 )
        CV_ERROR( CV_StsUnmatchedSizes, "" );

    if( method < CV_TM_SQDIFF || method > CV_TM_CCOEFF_NORMED )
        CV_ERROR( CV_StsBadArg, "unknown comparison method" );

    switch( CV_MAT_DEPTH( img->type ))
    {
    case CV_8U:
        dataType = cv8u;
        dataOffset = 0;
        break;
    case CV_8S:
        dataType = cv8s;
        dataOffset = 1;
        break;
    case CV_32F:
        dataType = cv32f;
        dataOffset = 2;
        break;
    default:
        CV_ERROR( CV_StsBadArg, icvUnsupportedFormat );
    }

    imgSize = icvGetMatSize( img );
    templSize = icvGetMatSize( templ );

    bufSizeFunc = (CvMatchBufSizeFunc)(bufSizeFuncs[imethod]);
    IPPI_CALL( bufSizeFunc( imgSize, templSize, dataType, &bufferSize ));

    CV_CALL( buffer = icvAlloc( bufferSize ));

    func = (CvMatchTemplFunc)(funcs[imethod + dataOffset * 6]);

    if( !func )
        CV_ERROR( CV_StsUnsupportedFormat, "" );

    img_step = img->step;
    templ_step = templ->step;
    result_step = result->step;

    if( img_step == 0 )
        img_step = img->width*icvPixSize[CV_MAT_TYPE(img->type)];

    if( templ_step == 0 )
        templ_step = templ->width*icvPixSize[CV_MAT_TYPE(templ->type)];

    if( result_step == 0 )
        result_step = result->width*icvPixSize[CV_MAT_TYPE(result->type)];

    IPPI_CALL(  func( img->data.ptr, img_step, imgSize,
                      templ->data.ptr, templ_step, templSize,
                      result->data.ptr, result_step, buffer ));

    __END__;

    cvFree( &buffer );
}

/* End of file. */
