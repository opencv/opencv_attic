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

/****************************************************************************************\
                                    Initialization
\****************************************************************************************/

CvStatus CV_STDCALL icvFilterInitAlloc(
             int roiWidth, CvDataType dataType, int channels,
             CvSize elSize, CvPoint elAnchor,
             const void* elData, int elementType,
             struct CvFilterState ** filterState )
{
    CvFilterState *state = 0;
    const int align = sizeof(size_t);
    int ker_size = 0;
    int buffer_step = 0;
    int buffer_size;
    int bt_pix, bt_pix_n;
    int aligned_hdr_size = cvAlign((int)sizeof(*state),align);
    int temp_lines = 2;
    int binaryElement = ICV_KERNEL_TYPE(elementType) == ICV_BINARY_KERNEL;
    int separableElement = ICV_KERNEL_TYPE(elementType) == ICV_SEPARABLE_KERNEL;
    char *ptr;

    if( !filterState )
        return CV_NULLPTR_ERR;
    *filterState = 0;

    if( roiWidth <= 0 )
        return CV_BADSIZE_ERR;
    if( dataType != cv8u && dataType != cv32s && dataType != cv32f )
        return CV_BADDEPTH_ERR;

    if( channels < 1 || channels > 4 )
        return CV_UNSUPPORTED_CHANNELS_ERR;
    if( elSize.width <= 0 || elSize.height <= 0 )
        return CV_BADSIZE_ERR;
    if( (unsigned) elAnchor.x >= (unsigned) elSize.width ||
        (unsigned) elAnchor.y >= (unsigned) elSize.height )
        return CV_BADRANGE_ERR;

    bt_pix = dataType == cv8u ? 1 : dataType == cv32s ? sizeof(int) : sizeof(float);
    bt_pix_n = bt_pix * channels;

    if( elData )
        ker_size = binaryElement ? (elSize.width + 1) * elSize.height * 2 :
                   separableElement ? (elSize.width + elSize.height)*bt_pix :
                                      elSize.width * elSize.height * bt_pix;

    buffer_step = cvAlign((roiWidth + elSize.width+1 + CV_MORPH_ALIGN*4)*bt_pix_n, align);

    buffer_size = (elSize.height + temp_lines) * (buffer_step + sizeof(void*)) +
                  ker_size + aligned_hdr_size + elSize.width * bt_pix_n + align*4;

    buffer_size = cvAlign(buffer_size + align, align);

    state = (CvFilterState*)cvAlloc( buffer_size );
    if( !state )
        return CV_OUTOFMEM_ERR;

    ptr = (char*)state;
    state->buffer_step = buffer_step;
    state->crows = 0;

    state->ker_x = elAnchor.x;
    state->ker_y = elAnchor.y;
    state->ker_height = elSize.height;
    state->ker_width = elSize.width;
    state->kerType = elementType;
    state->divisor = 1;

    state->max_width = roiWidth;
    state->dataType = dataType;
    state->channels = channels;
    state->origin = 0;
    ptr += aligned_hdr_size + align;

    state->buffer = ptr;
    ptr += buffer_step * elSize.height;

    state->tbuf = ptr;
    ptr += buffer_step * temp_lines;

    state->rows = (char**)ptr;
    ptr += sizeof(state->rows[0])*elSize.height;

    state->ker0 = state->ker1 = 0;

    if( elData )
    {
        state->ker0 = (uchar*)ptr;
        
        if( binaryElement )
        {
            int i, mask_size = elSize.width * elSize.height;
            state->ker1 = 0;

            for( i = 0; i < mask_size; i++ )
            {
                int t = ((int*)elData)[i] ? -1 : 0;
                state->ker0[i] = (uchar)t;
            }
        }
        else
        {
            memcpy( state->ker0, elData, ker_size );
            state->ker1 = (uchar*)(separableElement ? ptr + elSize.width*bt_pix : 0);
        }
    }

    *filterState = state;
    return CV_OK;
}


CvStatus CV_STDCALL icvFilterFree( CvFilterState ** filterState )
{
    if( !filterState )
        return CV_NULLPTR_ERR;
    cvFree( (void **) filterState );
    return CV_OK;
}


#define SMALL_GAUSSIAN_SIZE  7

static void
icvCalcGaussianKernel( int n, double sigma, float* kernel )
{
    static const float small_gaussian_tab[][SMALL_GAUSSIAN_SIZE] = 
    {
        {1.f},
        {0.25f, 0.5f, 0.25f},
        {0.0625f, 0.25f, 0.375f, 0.25f, 0.0625f},
        {0.03125, 0.109375, 0.21875, 0.28125, 0.21875, 0.109375, 0.03125}
    };

    if( n <= SMALL_GAUSSIAN_SIZE && sigma <= 0 )
    {
        assert( n%2 == 1 );
        memcpy( kernel, small_gaussian_tab[n>>1], n*sizeof(kernel[0]));
    }
    else
    {
        double sigmaX = sigma > 0 ? sigma : (n/2 - 1)*0.3 + 0.8;
        double scale2X = -0.5/(sigmaX*sigmaX);
        double sum = 1.;
        int i;
        sum = kernel[n/2] = 1.f;
    
        for( i = 1; i <= n/2; i++ )
        {
            kernel[n/2+i] = kernel[n/2-i] = (float)exp(scale2X*i*i);
            sum += kernel[n/2+i]*2;
        }

        sum = 1./sum;
        for( i = 0; i <= n/2; i++ )
            kernel[n/2+i] = kernel[n/2-i] = (float)(kernel[n/2+i]*sum);
    }
}


static CvStatus
icvSmoothInitAlloc( int width, CvDataType /*dataType*/, int channels,
                    CvSize el_size, int smoothtype, double sigma,
                    struct CvFilterState** filterState )
{
    CvPoint el_anchor = { el_size.width/2, el_size.height/2 };
    float* kx = 0;
    CvStatus status;

    if( smoothtype == CV_GAUSSIAN )
    {
        kx = (float*)cvStackAlloc( (el_size.width + el_size.height)*sizeof(kx[0]));
        float* ky = kx + el_size.width;

        icvCalcGaussianKernel( el_size.width, sigma, kx );
        if( el_size.width == el_size.height )
            memcpy( ky, kx, el_size.width*sizeof(kx[0]) );
        else
            icvCalcGaussianKernel( el_size.height, sigma, ky );
    }

    status = icvFilterInitAlloc( width, cv32f, channels, el_size, el_anchor,
                                 kx, ICV_SEPARABLE_KERNEL, filterState );
    if( status < 0 )
        return status;

    if( filterState && *filterState )
    {
        if( smoothtype == CV_BLUR )
            (*filterState)->divisor = el_size.width * el_size.height;
        else if( smoothtype == CV_GAUSSIAN )
        {
            (*filterState)->divisor = (double)(1 << el_size.width) * (1 << el_size.height);
            (*filterState)->kerType = ((*filterState)->kerType & ~255) |
                (sigma > 0 ? ICV_CUSTOM_GAUSSIAN_KERNEL : ICV_DEFAULT_GAUSSIAN_KERNEL);
        }
    }

    return filterState != 0 ? CV_OK : CV_NOTDEFINED_ERR;
}


static CvStatus
icvSmoothFree( CvFilterState ** filterState )
{
    return icvFilterFree( filterState );
}


/****************************************************************************************\
                                      Simple Blur
\****************************************************************************************/

static CvStatus CV_STDCALL
icvBlur_8u_CnR( uchar* src, int srcStep,
                uchar* dst, int dstStep, CvSize * roiSize,
                CvFilterState* state, int stage )
{
    int width = roiSize->width;
    int src_height = roiSize->height;
    int dst_height = src_height;
    int x, y = 0, i;

    int ker_x = state->ker_x;
    int ker_y = state->ker_y;
    int ker_width = state->ker_width;
    int ker_height = state->ker_height;
    int ker_right = ker_width - ker_x;

    int crows = state->crows;
    int **rows = (int**)(state->rows);
    uchar *tbuf = (uchar*)(state->tbuf);
    int   *sumbuf = (int*)((char*)state->tbuf + state->buffer_step);
    int *trow, *trow2;

    int channels = state->channels;
    int ker_x_n = ker_x * channels;
    int ker_width_n = ker_width * channels;
    int ker_right_n = ker_right * channels;
    int width_n = width * channels;

    int is_small_width = width <= ker_width;
    int starting_flag = 0;
    int width_rest = width_n & (CV_MORPH_ALIGN - 1);
    int divisor = cvRound(state->divisor);
    double inv_divisor = divisor ? 1./divisor : 0.;

    if( stage == CV_START + CV_END )
        stage = CV_WHOLE;

    /* initialize cyclic buffer when starting */
    if( stage == CV_WHOLE || stage == CV_START )
    {
        for( i = 0; i < ker_height; i++ )
            rows[i] = (int*)(state->buffer + state->buffer_step * i);

        crows = ker_y;
        if( stage != CV_WHOLE )
            dst_height -= ker_height - ker_y - 1;
        starting_flag = 1;
    }

    if( stage == CV_END )
        dst_height += ker_height - ker_y - 1;

    do
    {
        int need_copy = is_small_width | (y == 0);
        uchar *tsrc;
        int *tdst;
        uchar *tdst2;
        int *saved_row = rows[ker_y];

        /* fill cyclic buffer - horizontal filtering */
        for( ; crows < ker_height; crows++ )
        {
            tsrc = src - ker_x_n;
            tdst = rows[crows];

            if( src_height-- <= 0 )
            {
                if( stage != CV_WHOLE && stage != CV_END )
                    break;
                /* duplicate last row */
                trow = rows[crows - 1];
                CV_COPY( tdst, trow, width_n, x );
                continue;
            }

            need_copy |= src_height == 1;

            if( ker_width > 1 )
            {
                if( need_copy )
                {
                    tsrc = tbuf - ker_x_n;
                    CV_COPY( tbuf, src, width_n, x );
                }
                else
                {
                    CV_COPY( tbuf - ker_x_n, src - ker_x_n, ker_x_n, x );
                    CV_COPY( tbuf, src + width_n, ker_right_n, x );
                }

                /* make replication borders */
                for( i = ker_x_n - 1; i >= 0; i-- )
                    tsrc[i] = tsrc[i + channels];
                for( i = width_n + ker_x_n; i < width_n + ker_width_n; i++ )
                    tsrc[i] = tsrc[i - channels];

                if( channels == 1 )
                {
                    int t0 = 0;

                    for( i = 0; i < ker_width_n - 1; i++ )
                        t0 += tsrc[i];

                    /* horizontal blurring */
                    for( i = ker_width_n; i < width_n + ker_width_n; i++ )
                    {
                        t0 += tsrc[i-1];
                        tdst[i-ker_width_n] = t0;
                        t0 -= tsrc[i - ker_width_n];
                    }
                }
                else if( channels == 3 )
                {
                    int t0 = 0, t1 = 0, t2 = 0;

                    for( i = 0; i < ker_width_n - 3; i += 3 )
                    {
                        t0 += tsrc[i];
                        t1 += tsrc[i+1];
                        t2 += tsrc[i+2];
                    }

                    /* horizontal blurring */
                    for( i = ker_width_n; i < width_n + ker_width_n; i += 3 )
                    {
                        t0 += tsrc[i-3];
                        t1 += tsrc[i-2];
                        t2 += tsrc[i-1];
                        tdst[i-ker_width_n] = t0;
                        tdst[i-ker_width_n+1] = t1;
                        tdst[i-ker_width_n+2] = t2;
                        t0 -= tsrc[i - ker_width_n];
                        t1 -= tsrc[i - ker_width_n + 1];
                        t2 -= tsrc[i - ker_width_n + 2];
                    }
                }
                else            /* channels == 4 */
                {
                    int t0 = 0, t1 = 0, t2 = 0, t3 = 0;
                    
                    for( i = 0; i < ker_width_n - 4; i += 4 )
                    {
                        t0 += tsrc[i];
                        t1 += tsrc[i+1];
                        t2 += tsrc[i+2];
                        t3 += tsrc[i+3];
                    }

                    /* horizontal blurring */
                    for( i = ker_width_n; i < width_n + ker_width_n; i += 4 )
                    {
                        t0 += tsrc[i-4];
                        t1 += tsrc[i-3];
                        t2 += tsrc[i-2];
                        t3 += tsrc[i-1];
                        tdst[i-ker_width_n] = t0;
                        tdst[i-ker_width_n+1] = t1;
                        tdst[i-ker_width_n+2] = t2;
                        tdst[i-ker_width_n+3] = t3;
                        t0 -= tsrc[i - ker_width_n];
                        t1 -= tsrc[i - ker_width_n + 1];
                        t2 -= tsrc[i - ker_width_n + 2];
                        t3 -= tsrc[i - ker_width_n + 3];
                    }
                }

                if( !need_copy )
                {
                    /* restore borders */
                    CV_COPY( src - ker_x_n, tbuf - ker_x_n, ker_x_n, x );
                    CV_COPY( src + width_n, tbuf, ker_right_n, x );
                }
            }
            else
            {
                CV_COPY( tdst, tsrc + ker_x_n, width_n, x );
            }

            if( crows < ker_height )
                src += srcStep;
        }

        if( starting_flag )
        {
            starting_flag = 0;

            for( x = 0; x < width_n; x++ )
            {
                int t = rows[ker_y][x];
                for( i = 0; i < ker_y; i++ ) // copy border
                    rows[i][x] = t;

                t *= ker_y;
                for( i = ker_y; i < ker_height - 1; i++ ) // accumulate initial sum
                    t += rows[i][x];
                sumbuf[x] = t;
            }
        }

        /* vertical convolution */
        if( crows != ker_height )
            break;

        tdst2 = dst;

        if( width_rest )
        {
            need_copy = width_n < CV_MORPH_ALIGN || y == dst_height - 1;

            if( need_copy )
                tdst2 = tbuf;
            else
                CV_COPY( tbuf + width_n, dst + width_n, CV_MORPH_ALIGN, x );
        }

        trow = rows[0];
        trow2 = rows[ker_height-1];

        for( x = 0; x < width_n; x += 4 )
        {
            int val0, val1, t0, t1;

            val0 = sumbuf[x];
            val1 = sumbuf[x+1];
            
            val0 += trow2[x];
            val1 += trow2[x+1];

            t0 = cvRound(val0*inv_divisor);
            t1 = cvRound(val1*inv_divisor);

            tdst2[x] = (uchar)t0;
            tdst2[x+1] = (uchar)t1;

            sumbuf[x] = val0 - trow[x];
            sumbuf[x+1] = val1 - trow[x+1];

            val0 = sumbuf[x+2];
            val1 = sumbuf[x+3];
            
            val0 += trow2[x+2];
            val1 += trow2[x+3];

            t0 = cvRound(val0*inv_divisor);
            t1 = cvRound(val1*inv_divisor);

            tdst2[x+2] = (uchar)t0;
            tdst2[x+3] = (uchar)t1;

            sumbuf[x+2] = val0 - trow[x+2];
            sumbuf[x+3] = val1 - trow[x+3];
        }

        if( width_rest )
        {
            if( need_copy )
                CV_COPY( dst, tbuf, width_n, x );
            else
                CV_COPY( dst + width_n, tbuf + width_n, CV_MORPH_ALIGN, x );
        }

        rows[ker_y] = saved_row;

        /* rotate buffer */
        {
            int *t = rows[0];

            CV_COPY( rows, rows + 1, ker_height - 1, i );
            rows[i] = t;
            crows--;
            dst += dstStep;
        }
    }
    while( ++y < dst_height );

    roiSize->height = y;
    state->crows = crows;

    return CV_OK;
}


CvStatus CV_STDCALL
icvBlur_8u16s_C1R( const uchar* pSrc, int srcStep,
                   short* dst, int dstStep, CvSize * roiSize,
                   CvFilterState* state, int stage )
{
    uchar* src = (uchar*)pSrc;
    int width = roiSize->width;
    int src_height = roiSize->height;
    int dst_height = src_height;
    int x, y = 0, i;

    int ker_x = state->ker_x;
    int ker_y = state->ker_y;
    int ker_width = state->ker_width;
    int ker_height = state->ker_height;
    int ker_right = ker_width - ker_x;

    int crows = state->crows;
    int **rows = (int**)(state->rows);
    short *tbufw = (short*)(state->tbuf);
    int   *sumbuf = (int*)((char*)state->tbuf + state->buffer_step);
    int *trow, *trow2;

    int channels = state->channels;
    int ker_x_n = ker_x * channels;
    int ker_width_n = ker_width * channels;
    int ker_right_n = ker_right * channels;
    int width_n = width * channels;

    int is_small_width = width <= ker_width;
    int starting_flag = 0;
    int width_rest = width_n & (CV_MORPH_ALIGN - 1);

    /*if( stage == CV_START + CV_END )
        stage = CV_WHOLE;*/

    /* initialize cyclic buffer when starting */
    if( stage == CV_WHOLE || stage == CV_START )
    {
        for( i = 0; i < ker_height; i++ )
            rows[i] = (int*)(state->buffer + state->buffer_step * i);

        crows = ker_y;
        if( stage != CV_WHOLE )
            dst_height -= ker_height - ker_y - 1;
        starting_flag = 1;
    }

    if( stage == CV_END )
        dst_height += ker_height - ker_y - 1;

    dstStep /= sizeof(dst[0]);

    do
    {
        int need_copy = is_small_width | (y == 0);
        uchar *tsrc;
        int *tdst;
        short *tdst2;
        int *saved_row = rows[ker_y];

        /* fill cyclic buffer - horizontal filtering */
        for( ; crows < ker_height; crows++ )
        {
            tsrc = src - ker_x_n;
            tdst = rows[crows];

            if( src_height-- <= 0 )
            {
                if( stage != CV_WHOLE && stage != CV_END )
                    break;
                /* duplicate last row */
                trow = rows[crows - 1];
                CV_COPY( tdst, trow, width_n, x );
                continue;
            }

            need_copy |= src_height == 1;

            if( ker_width > 1 )
            {
                uchar* tbuf = (uchar*)tbufw;

                if( need_copy )
                {
                    tsrc = tbuf - ker_x_n;
                    CV_COPY( tbuf, src, width_n, x );
                }
                else
                {
                    CV_COPY( tbuf - ker_x_n, src - ker_x_n, ker_x_n, x );
                    CV_COPY( tbuf, src + width_n, ker_right_n, x );
                }

                {
                    /* make replication borders */
                    uchar pix = tsrc[ker_x];
                    int t0 = 0;

                    CV_SET( tsrc, pix, ker_x, x );

                    pix = tsrc[width + ker_x - 1];
                    CV_SET( tsrc + width + ker_x, pix, ker_right, x );

                    for( i = 0; i < ker_width_n - 1; i++ )
                        t0 += tsrc[i];

                    /* horizontal blurring */
                    for( i = ker_width_n; i < width_n + ker_width_n; i++ )
                    {
                        t0 += tsrc[i-1];
                        tdst[i-ker_width_n] = t0;
                        t0 -= tsrc[i - ker_width_n];
                    }
                }

                if( !need_copy )
                {
                    /* restore borders */
                    CV_COPY( src - ker_x_n, tbuf - ker_x_n, ker_x_n, x );
                    CV_COPY( src + width_n, tbuf, ker_right_n, x );
                }
            }
            else
            {
                CV_COPY( tdst, tsrc + ker_x_n, width_n, x );
            }

            if( crows < ker_height )
                src += srcStep;
        }

        if( starting_flag )
        {
            starting_flag = 0;

            for( x = 0; x < width_n; x++ )
            {
                int t = rows[ker_y][x];
                for( i = 0; i < ker_y; i++ ) // copy border
                    rows[i][x] = t;

                t *= ker_y;
                for( i = ker_y; i < ker_height - 1; i++ ) // accumulate initial sum
                    t += rows[i][x];
                sumbuf[x] = t;
            }
        }

        /* vertical convolution */
        if( crows != ker_height )
            break;

        tdst2 = dst;

        if( width_rest )
        {
            need_copy = width_n < CV_MORPH_ALIGN || y == dst_height - 1;

            if( need_copy )
                tdst2 = tbufw;
            else
                CV_COPY( tbufw + width_n, dst + width_n, CV_MORPH_ALIGN, x );
        }

        trow = rows[0];
        trow2 = rows[ker_height-1];

        for( x = 0; x < width_n; x += 4 )
        {
            int val0, val1;

            val0 = sumbuf[x];
            val1 = sumbuf[x+1];
            
            val0 += trow2[x];
            val1 += trow2[x+1];

            tdst2[x] = CV_CAST_16S(val0);
            tdst2[x+1] = CV_CAST_16S(val1);

            sumbuf[x] = val0 - trow[x];
            sumbuf[x+1] = val1 - trow[x+1];

            val0 = sumbuf[x+2];
            val1 = sumbuf[x+3];
            
            val0 += trow2[x+2];
            val1 += trow2[x+3];

            tdst2[x+2] = CV_CAST_16S(val0);
            tdst2[x+3] = CV_CAST_16S(val1);

            sumbuf[x+2] = val0 - trow[x+2];
            sumbuf[x+3] = val1 - trow[x+3];
        }

        if( width_rest )
        {
            if( need_copy )
                CV_COPY( dst, tbufw, width_n, x );
            else
                CV_COPY( dst + width_n, tbufw + width_n, CV_MORPH_ALIGN, x );
        }

        rows[ker_y] = saved_row;

        /* rotate buffer */
        {
            int *t = rows[0];

            CV_COPY( rows, rows + 1, ker_height - 1, i );
            rows[i] = t;
            crows--;
            dst += dstStep;
        }
    }
    while( ++y < dst_height );

    roiSize->height = y;
    state->crows = crows;

    return CV_OK;
}


CvStatus CV_STDCALL
icvBlur_32f_CnR( const float* pSrc, int srcStep,
                 float* dst, int dstStep, CvSize * roiSize,
                 CvFilterState* state, int stage )
{
    float* src = (float*)pSrc;
    int width = roiSize->width;
    int src_height = roiSize->height;
    int dst_height = src_height;
    int x, y = 0, i;

    int ker_x = state->ker_x;
    int ker_y = state->ker_y;
    int ker_width = state->ker_width;
    int ker_height = state->ker_height;
    int ker_right = ker_width - ker_x;

    int crows = state->crows;
    float **rows = (float**)(state->rows);
    float *tbuf = (float*)(state->tbuf);
    float *sumbuf = (float*)((char*)state->tbuf + state->buffer_step);
    float *trow, *trow2;

    int channels = state->channels;
    int ker_x_n = ker_x * channels;
    int ker_width_n = ker_width * channels;
    int ker_right_n = ker_right * channels;
    int width_n = width * channels;

    int is_small_width = width <= ker_width;
    int starting_flag = 0;
    int width_rest = width_n & (CV_MORPH_ALIGN - 1);
    float scale = (float)(1./state->divisor);
    int no_scale = state->divisor == 1;

    srcStep /= sizeof(float);
    dstStep /= sizeof(float);

    if( stage == CV_START + CV_END )
        stage = CV_WHOLE;

    /* initialize cyclic buffer when starting */
    if( stage == CV_WHOLE || stage == CV_START )
    {
        for( i = 0; i < ker_height; i++ )
            rows[i] = (float*)(state->buffer + state->buffer_step * i);

        crows = ker_y;
        if( stage != CV_WHOLE )
            dst_height -= ker_height - ker_y - 1;
        starting_flag = 1;
    }

    if( stage == CV_END )
        dst_height += ker_height - ker_y - 1;

    do
    {
        int need_copy = is_small_width | (y == 0);
        float *tsrc;
        float *tdst;
        float *tdst2;
        float *saved_row = rows[ker_y];

        /* fill cyclic buffer - horizontal filtering */
        for( ; crows < ker_height; crows++ )
        {
            tsrc = src - ker_x_n;
            tdst = rows[crows];

            if( src_height-- <= 0 )
            {
                if( stage != CV_WHOLE && stage != CV_END )
                    break;
                /* duplicate last row */
                trow = rows[crows - 1];
                CV_COPY( tdst, trow, width_n, x );
                continue;
            }

            need_copy |= src_height == 1;

            if( ker_width > 1 )
            {
                if( need_copy )
                {
                    tsrc = tbuf - ker_x_n;
                    CV_COPY( tbuf, src, width_n, x );
                }
                else
                {
                    CV_COPY( tbuf - ker_x_n, src - ker_x_n, ker_x_n, x );
                    CV_COPY( tbuf, src + width_n, ker_right_n, x );
                }

                /* make replication borders */
                for( i = ker_x_n - 1; i >= 0; i-- )
                    tsrc[i] = tsrc[i + channels];
                for( i = width_n + ker_x_n; i < width_n + ker_width_n; i++ )
                    tsrc[i] = tsrc[i - channels];

                /* horizontal blurring */
                if( channels == 1 )
                {
                    float t0 = 0;
                    for( i = 0; i < ker_width_n - 1; i++ )
                        t0 += tsrc[i];

                    for( i = ker_width_n; i < width_n + ker_width_n; i++ )
                    {
                        t0 += tsrc[i-1];
                        tdst[i-ker_width_n] = t0;
                        t0 -= tsrc[i - ker_width_n];
                    }
                }
                else if( channels == 3 )
                {
                    float t0 = 0, t1 = 0, t2 = 0;
                    for( i = 0; i < ker_width_n - 3; i += 3 )
                    {
                        t0 += tsrc[i];
                        t1 += tsrc[i+1];
                        t2 += tsrc[i+2];
                    }

                    for( i = ker_width_n; i < width_n + ker_width_n; i += 3 )
                    {
                        t0 += tsrc[i-3];
                        t1 += tsrc[i-2];
                        t2 += tsrc[i-1];
                        tdst[i-ker_width_n] = t0;
                        tdst[i-ker_width_n+1] = t1;
                        tdst[i-ker_width_n+2] = t2;
                        t0 -= tsrc[i - ker_width_n];
                        t1 -= tsrc[i - ker_width_n + 1];
                        t2 -= tsrc[i - ker_width_n + 2];
                    }
                }
                else            /* channels == 4 */
                {
                    float t0 = 0, t1 = 0, t2 = 0, t3 = 0;
                    for( i = 0; i < ker_width_n - 4; i += 4 )
                    {
                        t0 += tsrc[i];
                        t1 += tsrc[i+1];
                        t2 += tsrc[i+2];
                        t3 += tsrc[i+3];
                    }

                    /* horizontal blurring */
                    for( i = ker_width_n; i < width_n + ker_width_n; i += 4 )
                    {
                        t0 += tsrc[i-4];
                        t1 += tsrc[i-3];
                        t2 += tsrc[i-2];
                        t3 += tsrc[i-1];
                        tdst[i-ker_width_n] = t0;
                        tdst[i-ker_width_n+1] = t1;
                        tdst[i-ker_width_n+2] = t2;
                        tdst[i-ker_width_n+3] = t3;
                        t0 -= tsrc[i - ker_width_n];
                        t1 -= tsrc[i - ker_width_n + 1];
                        t2 -= tsrc[i - ker_width_n + 2];
                        t3 -= tsrc[i - ker_width_n + 3];
                    }
                }

                if( !need_copy )
                {
                    /* restore borders */
                    CV_COPY( src - ker_x_n, tbuf - ker_x_n, ker_x_n, x );
                    CV_COPY( src + width_n, tbuf, ker_right_n, x );
                }
            }
            else
            {
                CV_COPY( tdst, tsrc + ker_x_n, width_n, x );
            }

            if( crows < ker_height )
                src += srcStep;
        }

        if( starting_flag )
        {
            starting_flag = 0;

            for( x = 0; x < width_n; x++ )
            {
                float t = rows[ker_y][x];
                for( i = 0; i < ker_y; i++ ) // copy border
                    rows[i][x] = t;

                t *= ker_y;
                for( i = ker_y; i < ker_height - 1; i++ ) // accumulate initial sum
                    t += rows[i][x];
                sumbuf[x] = t;
            }
        }

        /* vertical convolution */
        if( crows != ker_height )
            break;

        tdst2 = dst;

        if( width_rest )
        {
            need_copy = width_n < CV_MORPH_ALIGN || y == dst_height - 1;

            if( need_copy )
                tdst2 = tbuf;
            else
                CV_COPY( tbuf + width_n, dst + width_n, CV_MORPH_ALIGN, x );
        }

        trow = rows[0];
        trow2 = rows[ker_height-1];

        if( no_scale )
        {
            for( x = 0; x < width_n; x += 4 )
            {
                float val0, val1;

                val0 = sumbuf[x];
                val1 = sumbuf[x+1];
            
                val0 += trow2[x];
                val1 += trow2[x+1];

                tdst2[x] = (float)val0;
                tdst2[x+1] = (float)val1;

                sumbuf[x] = val0 - trow[x];
                sumbuf[x+1] = val1 - trow[x+1];

                val0 = sumbuf[x+2];
                val1 = sumbuf[x+3];
            
                val0 += trow2[x+2];
                val1 += trow2[x+3];

                tdst2[x+2] = (float)val0;
                tdst2[x+3] = (float)val1;

                sumbuf[x+2] = val0 - trow[x+2];
                sumbuf[x+3] = val1 - trow[x+3];
            }
        }
        else
        {
            for( x = 0; x < width_n; x += 4 )
            {
                float val0, val1, t0, t1;

                val0 = sumbuf[x];
                val1 = sumbuf[x+1];
            
                val0 += trow2[x];
                val1 += trow2[x+1];

                t0 = val0 * scale;
                t1 = val1 * scale;

                tdst2[x] = (float)t0;
                tdst2[x+1] = (float)t1;

                sumbuf[x] = val0 - trow[x];
                sumbuf[x+1] = val1 - trow[x+1];

                val0 = sumbuf[x+2];
                val1 = sumbuf[x+3];
            
                val0 += trow2[x+2];
                val1 += trow2[x+3];

                t0 = val0 * scale;
                t1 = val1 * scale;

                tdst2[x+2] = (float)t0;
                tdst2[x+3] = (float)t1;

                sumbuf[x+2] = val0 - trow[x+2];
                sumbuf[x+3] = val1 - trow[x+3];
            }
        }

        if( width_rest )
        {
            if( need_copy )
                CV_COPY( dst, tbuf, width_n, x );
            else
                CV_COPY( dst + width_n, tbuf + width_n, CV_MORPH_ALIGN, x );
        }

        rows[ker_y] = saved_row;

        /* rotate buffer */
        {
            float *t = rows[0];

            CV_COPY( rows, rows + 1, ker_height - 1, i );
            rows[i] = t;
            crows--;
            dst += dstStep;
        }
    }
    while( ++y < dst_height );

    roiSize->height = y;
    state->crows = crows;

    return CV_OK;
}


CvStatus CV_STDCALL icvBlurInitAlloc(
    int roiWidth, int depth, int channels, int size,
    struct CvFilterState** filterState )
{
    return icvSmoothInitAlloc( roiWidth, (CvDataType)depth, channels, cvSize(size,size),
                               CV_BLUR_NO_SCALE, 0, filterState );
}


/****************************************************************************************\
                                    Gaussian Blur
\****************************************************************************************/

static CvStatus CV_STDCALL
icvGaussianBlur_small_8u_CnR( uchar* src, int srcStep,
                              uchar* dst, int dstStep, CvSize * roiSize,
                              CvFilterState* state, int stage )
{
    int width = roiSize->width;
    int src_height = roiSize->height;
    int dst_height = src_height;
    int x, y = 0, i;

    int ker_width = state->ker_width;
    int ker_height = state->ker_height;
    int ker_x = ker_width/2;
    int ker_y = ker_height/2;
    int ker_right = ker_width - ker_x;

    int crows = state->crows;
    int **rows = (int**)(state->rows);
    uchar *tbuf = (uchar*)(state->tbuf);
    int *trow = 0;

    int channels = state->channels;
    int ker_x_n = ker_x * channels;
    int ker_right_n = ker_right * channels;
    int ker_width_n = ker_width * channels;
    int width_n = width * channels;

    int is_small_width = width <= ker_width;
    int starting_flag = 0;
    int width_rest = width_n & (CV_MORPH_ALIGN - 1);
    int rshift = ker_width + ker_height - 2;
    int delta = 1 << (rshift - 1);

    assert( ker_width <= SMALL_GAUSSIAN_SIZE &&
            ker_height <= SMALL_GAUSSIAN_SIZE );

    if( stage == CV_START + CV_END )
        stage = CV_WHOLE;

    /* initialize cyclic buffer when starting */
    if( stage == CV_WHOLE || stage == CV_START )
    {
        for( i = 0; i < ker_height; i++ )
            rows[i] = (int*)(state->buffer + state->buffer_step * i);

        crows = ker_y;
        if( stage != CV_WHOLE )
            dst_height -= ker_height - ker_y - 1;
        starting_flag = 1;
    }

    if( stage == CV_END )
        dst_height += ker_height - ker_y - 1;

    do
    {
        int need_copy = is_small_width | (y == 0);
        uchar *tsrc;
        int *tdst;
        uchar *tdst2;
        int *saved_row = rows[ker_y];

        /* fill cyclic buffer - horizontal filtering */
        for( ; crows < ker_height; crows++ )
        {
            tsrc = src - ker_x_n;
            tdst = rows[crows];

            if( src_height-- <= 0 )
            {
                if( stage != CV_WHOLE && stage != CV_END )
                    break;
                /* duplicate last row */
                trow = rows[crows - 1];
                CV_COPY( tdst, trow, width_n, x );
                continue;
            }

            need_copy |= src_height == 1;

            if( ker_width > 1 )
            {
                if( need_copy )
                {
                    tsrc = tbuf - ker_x_n;
                    CV_COPY( tbuf, src, width_n, x );
                }
                else
                {
                    CV_COPY( tbuf - ker_x_n, src - ker_x_n, ker_x_n, x );
                    CV_COPY( tbuf, src + width_n, ker_right_n, x );
                }

                /* make replication borders */
                for( i = ker_x_n - 1; i >= 0; i-- )
                    tsrc[i] = tsrc[i + channels];
                for( i = width_n + ker_x_n; i < width_n + ker_width_n; i++ )
                    tsrc[i] = tsrc[i - channels];

                if( channels == 1 )
                {
                    /* horizontal blurring */
                    if( ker_width == 3 )
                    {
                        for( i = 0; i < width_n; i++ )
                            tdst[i] = tsrc[i+1]*2 + tsrc[i] + tsrc[i + 2];
                    }
                    else if( ker_width == 5 )
                    {
                        for( i = 0; i < width_n; i++ )
                            tdst[i] = tsrc[i+2]*6 + (tsrc[i+1] + tsrc[i+3])*4 + 
                                      tsrc[i] + tsrc[i+4];
                    }
                    else if( ker_width == 7 )
                    {
                        for( i = 0; i < width_n; i++ )
                            tdst[i] = tsrc[i+3]*18 + (tsrc[i+2] + tsrc[i+4])*14 + 
                                (tsrc[i+1] + tsrc[i+5])*7 + (tsrc[i] + tsrc[i+6])*2;
                    }
                }
                else if( channels == 3 )
                {
                    /* horizontal blurring */
                    if( ker_width == 3 )
                    {
                        for( i = 0; i < width_n; i += 3 )
                        {
                            int t0 = tsrc[i + 3]*2 + tsrc[i] + tsrc[i+6];
                            int t1 = tsrc[i + 4]*2 + tsrc[i+1] + tsrc[i+7];
                            int t2 = tsrc[i + 5]*2 + tsrc[i+2] + tsrc[i+8];

                            tdst[i] = t0;
                            tdst[i+1] = t1;
                            tdst[i+2] = t2;
                        }
                    }
                    else if( ker_width == 5 )
                    {
                        for( i = 0; i < width_n; i += 3 )
                        {
                            int t0 = tsrc[i+6]*6 + (tsrc[i+3] + tsrc[i+9])*4 +
                                     tsrc[i]+tsrc[i+12];
                            int t1 = tsrc[i+7]*6 + (tsrc[i+4] + tsrc[i+10])*4 +
                                     tsrc[i+1]+tsrc[i+13];
                            int t2 = tsrc[i+8]*6 + (tsrc[i+5] + tsrc[i+11])*4 +
                                     tsrc[i+2]+tsrc[i+14];

                            tdst[i] = t0;
                            tdst[i+1] = t1;
                            tdst[i+2] = t2;
                        }
                    }
                    else
                    {
                        assert( ker_width == 7 );
                        for( i = 0; i < width_n; i += 3 )
                        {
                            int t0 = tsrc[i+9]*18 + (tsrc[i+6] + tsrc[i+12])*14 +
                                     (tsrc[i+3]+tsrc[i+15])*7 + (tsrc[i] + tsrc[i+18])*2;
                            int t1 = tsrc[i+10]*18 + (tsrc[i+7] + tsrc[i+13])*14 +
                                     (tsrc[i+4]+tsrc[i+16])*7 + (tsrc[i+1] + tsrc[i+19])*2;
                            int t2 = tsrc[i+11]*18 + (tsrc[i+8] + tsrc[i+14])*14 +
                                     (tsrc[i+5]+tsrc[i+17])*7 + (tsrc[i+2] + tsrc[i+20])*2;

                            tdst[i] = t0;
                            tdst[i+1] = t1;
                            tdst[i+2] = t2;
                        }
                    }
                }
                else   /* channels == 4 */
                {
                    /* horizontal blurring */
                    if( ker_width == 3 )
                    {
                        for( i = 0; i < width_n; i += 4 )
                        {
                            int t0 = tsrc[i + 4]*2 + tsrc[i] + tsrc[i+8];
                            int t1 = tsrc[i + 5]*2 + tsrc[i+1] + tsrc[i+9];
                            int t2 = tsrc[i + 6]*2 + tsrc[i+2] + tsrc[i+10];
                            int t3 = tsrc[i + 7]*2 + tsrc[i+3] + tsrc[i+11];

                            tdst[i] = t0;
                            tdst[i+1] = t1;
                            tdst[i+2] = t2;
                            tdst[i+3] = t3;
                        }
                    }
                    else if( ker_width == 5 )
                    {
                        for( i = 0; i < width_n; i += 4 )
                        {
                            int t0 = tsrc[i+8]*6 + (tsrc[i+4] + tsrc[i+12])*4 +
                                     tsrc[i]+tsrc[i+16];
                            int t1 = tsrc[i+9]*6 + (tsrc[i+5] + tsrc[i+13])*4 +
                                     tsrc[i+1]+tsrc[i+17];
                            int t2 = tsrc[i+10]*6 + (tsrc[i+6] + tsrc[i+14])*4 +
                                     tsrc[i+2]+tsrc[i+18];
                            int t3 = tsrc[i+11]*6 + (tsrc[i+7] + tsrc[i+15])*4 +
                                     tsrc[i+2]+tsrc[i+19];

                            tdst[i] = t0;
                            tdst[i+1] = t1;
                            tdst[i+2] = t2;
                            tdst[i+3] = t3;
                        }
                    }
                    else
                    {
                        assert( ker_width == 7 );
                        for( i = 0; i < width_n; i += 4 )
                        {
                            int t0 = tsrc[i+12]*18 + (tsrc[i+8] + tsrc[i+16])*14 +
                                     (tsrc[i+4]+tsrc[i+20])*7 + (tsrc[i] + tsrc[i+24])*2;
                            int t1 = tsrc[i+13]*18 + (tsrc[i+9] + tsrc[i+17])*14 +
                                     (tsrc[i+5]+tsrc[i+21])*7 + (tsrc[i+1] + tsrc[i+25])*2;
                            int t2 = tsrc[i+14]*18 + (tsrc[i+10] + tsrc[i+18])*14 +
                                     (tsrc[i+6]+tsrc[i+22])*7 + (tsrc[i+2] + tsrc[i+26])*2;
                            int t3 = tsrc[i+15]*18 + (tsrc[i+11] + tsrc[i+19])*14 +
                                     (tsrc[i+7]+tsrc[i+23])*7 + (tsrc[i+3] + tsrc[i+27])*2;

                            tdst[i] = t0;
                            tdst[i+1] = t1;
                            tdst[i+2] = t2;
                            tdst[i+3] = t3;
                        }
                    }
                }

                if( !need_copy )
                {
                    /* restore borders */
                    CV_COPY( src - ker_x_n, tbuf - ker_x_n, ker_x_n, x );
                    CV_COPY( src + width_n, tbuf, ker_right_n, x );
                }
            }
            else
            {
                CV_COPY( tdst, tsrc + ker_x_n, width_n, x );
            }

            if( crows < ker_height )
                src += srcStep;
        }

        if( starting_flag )
        {
            starting_flag = 0;
            trow = rows[ker_y];

            for( i = 0; i < ker_y; i++ )
            {
                tdst = rows[i];
                CV_COPY( tdst, trow, width_n, x );
            }
        }

        /* vertical convolution */
        if( crows != ker_height )
            break;

        tdst2 = dst;

        if( width_rest )
        {
            need_copy = width_n < CV_MORPH_ALIGN || y == dst_height - 1;

            if( need_copy )
                tdst2 = tbuf;
            else
                CV_COPY( tbuf + width_n, dst + width_n, CV_MORPH_ALIGN, x );
        }

        trow = rows[ker_y];

        if( ker_height == 1 )
        {
            for( x = 0; x < width_n; x += 4 )
            {
                int val0, val1, val2, val3;

                val0 = trow[x];
                val1 = trow[x + 1];
                val2 = trow[x + 2];
                val3 = trow[x + 3];

                tdst2[x + 0] = (uchar)((val0 + delta) >> rshift);
                tdst2[x + 1] = (uchar)((val1 + delta) >> rshift);
                tdst2[x + 2] = (uchar)((val2 + delta) >> rshift);
                tdst2[x + 3] = (uchar)((val3 + delta) >> rshift);
            }
        }
        else if( ker_height == 3 )
        {
            int *trow1 = rows[0], *trow2 = rows[2];
            
            for( x = 0; x < width_n; x += 4 )
            {
                int val0, val1;
                val0 = (trow[x]*2 + trow1[x] + trow2[x] + delta) >> rshift;
                val1 = (trow[x + 1]*2 + trow1[x+1] + trow2[x+1] + delta) >> rshift;
                
                tdst2[x + 0] = (uchar)val0;
                tdst2[x + 1] = (uchar)val1;
                
                val0 = (trow[x + 2]*2 + trow1[x+2] + trow2[x+2] + delta) >> rshift;
                val1 = (trow[x + 3]*2 + trow1[x+3] + trow2[x+3] + delta) >> rshift;
                
                tdst2[x + 2] = (uchar)val0;
                tdst2[x + 3] = (uchar)val1;
            }
        }
        else if( ker_height == 5 )
        {
            int *trow1 = rows[0], *trow2 = rows[1],
                *trow3 = rows[3], *trow4 = rows[4];
            
            for( x = 0; x < width_n; x++ )
            {
                int val0 = (trow[x]*6 + (trow2[x] + trow3[x])*4 +
                            trow1[x] + trow4[x] + delta) >> rshift;
                tdst2[x] = (uchar)val0;
            }
        }
        else
        {
            int *trow1 = rows[0], *trow2 = rows[1],
                *trow3 = rows[2], *trow4 = rows[4],
                *trow5 = rows[5], *trow6 = rows[6];
            
            for( x = 0; x < width_n; x++ )
            {
                int val0 = (trow[x]*18 + (trow3[x] + trow4[x])*14 +
                    (trow2[x] + trow5[x])*7 + (trow1[x] + trow6[x])*2 + delta) >> rshift;
                tdst2[x] = (uchar)val0;
            }
        }

        if( width_rest )
        {
            if( need_copy )
                CV_COPY( dst, tbuf, width_n, x );
            else
                CV_COPY( dst + width_n, tbuf + width_n, CV_MORPH_ALIGN, x );
        }

        rows[ker_y] = saved_row;

        /* rotate buffer */
        {
            int *t = rows[0];

            CV_COPY( rows, rows + 1, ker_height - 1, i );
            rows[i] = t;
            crows--;
            dst += dstStep;
        }
    }
    while( ++y < dst_height );

    roiSize->height = y;
    state->crows = crows;

    return CV_OK;
}


static CvStatus CV_STDCALL
icvGaussianBlur_8u_CnR( uchar* src, int srcStep,
                        uchar* dst, int dstStep, CvSize * roiSize,
                        CvFilterState* state, int stage )
{
    if( state->ker_width <= SMALL_GAUSSIAN_SIZE &&
        state->ker_height <= SMALL_GAUSSIAN_SIZE &&
        ICV_X_KERNEL_TYPE(state->kerType) == ICV_DEFAULT_GAUSSIAN_KERNEL )
    {
        return icvGaussianBlur_small_8u_CnR( src, srcStep, dst, dstStep,
                                             roiSize, state, stage );
    }
    else
    {
        int width = roiSize->width;
        int src_height = roiSize->height;
        int dst_height = src_height;
        int x, y = 0, i;

        int ker_width = state->ker_width;
        int ker_height = state->ker_height;
        int ker_x = ker_width/2;
        int ker_y = ker_height/2;
        int ker_right = ker_width - ker_x;

        int crows = state->crows;
        float **rows = (float**)(state->rows);
        uchar *tbuf = (uchar*)(state->tbuf);
        float *trow = 0;

        int channels = state->channels;
        int ker_x_n = ker_x * channels;
        int ker_right_n = ker_right * channels;
        int width_n = width * channels;
        int ker_width_n = ker_width * channels;
    
        float* fmaskX = (float*)(state->ker0) + ker_x;
        float* fmaskY = (float*)(state->ker1) + ker_y;
        double fmX0 = fmaskX[0], fmY0 = fmaskY[0];

        int is_small_width = width <= ker_width;
        int starting_flag = 0;
        int width_rest = width_n & (CV_MORPH_ALIGN - 1);

        if( stage == CV_START + CV_END )
            stage = CV_WHOLE;

        /* initialize cyclic buffer when starting */
        if( stage == CV_WHOLE || stage == CV_START )
        {
            for( i = 0; i < ker_height; i++ )
                rows[i] = (float*)(state->buffer + state->buffer_step * i);

            crows = ker_y;
            if( stage != CV_WHOLE )
                dst_height -= ker_height - ker_y - 1;
            starting_flag = 1;
        }

        if( stage == CV_END )
            dst_height += ker_height - ker_y - 1;

        do
        {
            int need_copy = is_small_width | (y == 0);
            uchar *tsrc;
            float *tdst;
            uchar *tdst2;
            float *saved_row = rows[ker_y];

            /* fill cyclic buffer - horizontal filtering */
            for( ; crows < ker_height; crows++ )
            {
                tsrc = src - ker_x_n;
                tdst = rows[crows];

                if( src_height-- <= 0 )
                {
                    if( stage != CV_WHOLE && stage != CV_END )
                        break;
                    /* duplicate last row */
                    trow = rows[crows - 1];
                    CV_COPY( tdst, trow, width_n, x );
                    continue;
                }

                need_copy |= src_height == 1;

                if( ker_width > 1 )
                {
                    if( need_copy )
                    {
                        tsrc = tbuf - ker_x_n;
                        CV_COPY( tbuf, src, width_n, x );
                    }
                    else
                    {
                        CV_COPY( tbuf - ker_x_n, src - ker_x_n, ker_x_n, x );
                        CV_COPY( tbuf, src + width_n, ker_right_n, x );
                    }

                    /* make replication borders */
                    for( i = ker_x_n - 1; i >= 0; i-- )
                        tsrc[i] = tsrc[i + channels];
                    for( i = width_n + ker_x_n; i < width_n + ker_width_n; i++ )
                        tsrc[i] = tsrc[i - channels];

                    /* horizontal blurring */
                    if( channels == 1 )
                    {
                        for( i = 0; i < width_n; i++ )
                        {
                            int j;
                            double t0 = CV_8TO32F(tsrc[i + ker_x_n])*fmX0;

                            for( j = 1; j <= ker_x; j++ )
                                t0 += (CV_8TO32F(tsrc[i+ker_x_n+j]) +
                                       CV_8TO32F(tsrc[i+ker_x_n-j]))*(double)fmaskX[j];

                            ((float*)tdst)[i] = (float)t0;
                        }
                    }
                    else if( channels == 3 )
                    {
                        for( i = 0; i < width_n; i += 3 )
                        {
                            int j;
                            double t0 = CV_8TO32F(tsrc[i + ker_x_n])*fmX0;
                            double t1 = CV_8TO32F(tsrc[i + ker_x_n + 1])*fmX0;
                            double t2 = CV_8TO32F(tsrc[i + ker_x_n + 2])*fmX0;

                            for( j = 1; j <= ker_x; j++ )
                            {
                                int j3 = j*3;
                                double m = fmaskX[j];
                                t0 += (CV_8TO32F(tsrc[i+ker_x_n+j3]) +
                                       CV_8TO32F(tsrc[i+ker_x_n-j3]))*m;
                                t1 += (CV_8TO32F(tsrc[i+ker_x_n+j3+1]) +
                                       CV_8TO32F(tsrc[i+ker_x_n-j3+1]))*m;
                                t2 += (CV_8TO32F(tsrc[i+ker_x_n+j3+2]) +
                                       CV_8TO32F(tsrc[i+ker_x_n-j3+2]))*m;
                            }

                            tdst[i] = (float)t0;
                            tdst[i+1] = (float)t1;
                            tdst[i+2] = (float)t2;
                        }
                    }
                    else    /* channels == 4 */
                    {
                        for( i = 0; i < width_n; i += 4 )
                        {
                            int j;
                            double t0 = CV_8TO32F(tsrc[i + ker_x_n])*fmX0;
                            double t1 = CV_8TO32F(tsrc[i + ker_x_n + 1])*fmX0;
                            double t2 = CV_8TO32F(tsrc[i + ker_x_n + 2])*fmX0;
                            double t3 = CV_8TO32F(tsrc[i + ker_x_n + 3])*fmX0;

                            for( j = 1; j <= ker_x; j++ )
                            {
                                int j4 = j*4;
                                double m = fmaskX[j];
                                t0 += (CV_8TO32F(tsrc[i+ker_x_n+j4]) +
                                       CV_8TO32F(tsrc[i+ker_x_n-j4]))*m;
                                t1 += (CV_8TO32F(tsrc[i+ker_x_n+j4+1]) +
                                       CV_8TO32F(tsrc[i+ker_x_n-j4+1]))*m;
                                t2 += (CV_8TO32F(tsrc[i+ker_x_n+j4+2]) +
                                       CV_8TO32F(tsrc[i+ker_x_n-j4+2]))*m;
                                t3 += (CV_8TO32F(tsrc[i+ker_x_n+j4+3]) +
                                       CV_8TO32F(tsrc[i+ker_x_n-j4+3]))*m;
                            }

                            tdst[i] = (float)t0;
                            tdst[i+1] = (float)t1;
                            tdst[i+2] = (float)t2;
                            tdst[i+3] = (float)t3;
                        }
                    }

                    if( !need_copy )
                    {
                        /* restore borders */
                        CV_COPY( src - ker_x_n, tbuf - ker_x_n, ker_x_n, x );
                        CV_COPY( src + width_n, tbuf, ker_right_n, x );
                    }
                }
                else
                {
                    CV_COPY( tdst, tsrc + ker_x_n, width_n, x );
                }

                if( crows < ker_height )
                    src += srcStep;
            }

            if( starting_flag )
            {
                starting_flag = 0;
                trow = rows[ker_y];

                for( i = 0; i < ker_y; i++ )
                {
                    tdst = rows[i];
                    CV_COPY( tdst, trow, width_n, x );
                }
            }

            /* vertical convolution */
            if( crows != ker_height )
                break;

            tdst2 = dst;

            if( width_rest )
            {
                need_copy = width_n < CV_MORPH_ALIGN || y == dst_height - 1;

                if( need_copy )
                    tdst2 = tbuf;
                else
                    CV_COPY( tbuf + width_n, dst + width_n, CV_MORPH_ALIGN, x );
            }

            trow = rows[ker_y];

            for( x = 0; x < width_n; x += 4 )
            {
                double val0, val1, val2, val3;

                val0 = trow[x]*fmY0;
                val1 = trow[x + 1]*fmY0;
                val2 = trow[x + 2]*fmY0;
                val3 = trow[x + 3]*fmY0;

                for( i = 1; i <= ker_y; i++ )
                {
                    float *trow1, *trow2;
                    double m = fmaskY[i];
                    trow1 = rows[ker_y - i];
                    trow2 = rows[ker_y + i];
                    val0 += (trow1[x] + trow2[x])*m;
                    val1 += (trow1[x+1] + trow2[x+1])*m;
                    val2 += (trow1[x+2] + trow2[x+2])*m;
                    val3 += (trow1[x+3] + trow2[x+3])*m;
                }

                tdst2[x + 0] = (uchar)cvRound(val0);
                tdst2[x + 1] = (uchar)cvRound(val1);
                tdst2[x + 2] = (uchar)cvRound(val2);
                tdst2[x + 3] = (uchar)cvRound(val3);
            }

            if( width_rest )
            {
                if( need_copy )
                    CV_COPY( dst, tbuf, width_n, x );
                else
                    CV_COPY( dst + width_n, tbuf + width_n, CV_MORPH_ALIGN, x );
            }

            rows[ker_y] = saved_row;

            /* rotate buffer */
            {
                float *t = rows[0];

                CV_COPY( rows, rows + 1, ker_height - 1, i );
                rows[i] = t;
                crows--;
                dst += dstStep;
            }
        }
        while( ++y < dst_height );

        roiSize->height = y;
        state->crows = crows;

        return CV_OK;
    }
}


static CvStatus CV_STDCALL
icvGaussianBlur_32f_CnR( float* src, int srcStep,
                         float* dst, int dstStep, CvSize * roiSize,
                         CvFilterState* state, int stage )
{
    int width = roiSize->width;
    int src_height = roiSize->height;
    int dst_height = src_height;
    int x, y = 0, i;

    int ker_width = state->ker_width;
    int ker_height = state->ker_height;
    int ker_x = ker_width/2;
    int ker_y = ker_height/2;
    int ker_right = ker_width - ker_x;

    int crows = state->crows;
    float **rows = (float**)(state->rows);
    float *tbuf = (float*)(state->tbuf);
    float *trow = 0;

    int channels = state->channels;
    int ker_x_n = ker_x * channels;
    int ker_right_n = ker_right * channels;
    int width_n = width * channels;
    int ker_width_n = ker_width * channels;

    float* fmaskX = (float*)(state->ker0) + ker_x;
    float* fmaskY = (float*)(state->ker1) + ker_y;
    double fmX0 = fmaskX[0], fmY0 = fmaskY[0];

    int is_small_width = width <= ker_width;
    int starting_flag = 0;
    int width_rest = width_n & (CV_MORPH_ALIGN - 1);

    srcStep /= sizeof(src[0]);
    dstStep /= sizeof(dst[0]);

    if( stage == CV_START + CV_END )
        stage = CV_WHOLE;

    /* initialize cyclic buffer when starting */
    if( stage == CV_WHOLE || stage == CV_START )
    {
        for( i = 0; i < ker_height; i++ )
            rows[i] = (float*)(state->buffer + state->buffer_step * i);

        crows = ker_y;
        if( stage != CV_WHOLE )
            dst_height -= ker_height - ker_y - 1;
        starting_flag = 1;
    }

    if( stage == CV_END )
        dst_height += ker_height - ker_y - 1;

    do
    {
        int need_copy = is_small_width | (y == 0);
        float *tsrc;
        float *tdst;
        float *tdst2;
        float *saved_row = rows[ker_y];

        /* fill cyclic buffer - horizontal filtering */
        for( ; crows < ker_height; crows++ )
        {
            tsrc = src - ker_x_n;
            tdst = rows[crows];

            if( src_height-- <= 0 )
            {
                if( stage != CV_WHOLE && stage != CV_END )
                    break;
                /* duplicate last row */
                trow = rows[crows - 1];
                CV_COPY( tdst, trow, width_n, x );
                continue;
            }

            need_copy |= src_height == 1;

            if( ker_width > 1 )
            {
                if( need_copy )
                {
                    tsrc = tbuf - ker_x_n;
                    CV_COPY( tbuf, src, width_n, x );
                }
                else
                {
                    CV_COPY( tbuf - ker_x_n, src - ker_x_n, ker_x_n, x );
                    CV_COPY( tbuf, src + width_n, ker_right_n, x );
                }

                /* make replication borders */
                for( i = ker_x_n - 1; i >= 0; i-- )
                    tsrc[i] = tsrc[i + channels];
                for( i = width_n + ker_x_n; i < width_n + ker_width_n; i++ )
                    tsrc[i] = tsrc[i - channels];

                /* horizontal blurring */
                if( channels == 1 )
                {
                    for( i = 0; i < width_n; i++ )
                    {
                        int j;
                        double t0 = (tsrc[i + ker_x_n])*fmX0;

                        for( j = 1; j <= ker_x; j++ )
                            t0 += ((tsrc[i+ker_x_n+j]) +
                                   (tsrc[i+ker_x_n-j]))*(double)fmaskX[j];

                        ((float*)tdst)[i] = (float)t0;
                    }
                }
                else if( channels == 3 )
                {
                    for( i = 0; i < width_n; i += 3 )
                    {
                        int j;
                        double t0 = (tsrc[i + ker_x_n])*fmX0;
                        double t1 = (tsrc[i + ker_x_n + 1])*fmX0;
                        double t2 = (tsrc[i + ker_x_n + 2])*fmX0;

                        for( j = 1; j <= ker_x; j++ )
                        {
                            int j3 = j*3;
                            double m = fmaskX[j];
                            t0 += ((tsrc[i+ker_x_n+j3]) +
                                   (tsrc[i+ker_x_n-j3]))*m;
                            t1 += ((tsrc[i+ker_x_n+j3+1]) +
                                   (tsrc[i+ker_x_n-j3+1]))*m;
                            t2 += ((tsrc[i+ker_x_n+j3+2]) +
                                   (tsrc[i+ker_x_n-j3+2]))*m;
                        }

                        tdst[i] = (float)t0;
                        tdst[i+1] = (float)t1;
                        tdst[i+2] = (float)t2;
                    }
                }
                else    /* channels == 4 */
                {
                    for( i = 0; i < width_n; i += 4 )
                    {
                        int j;
                        double t0 = (tsrc[i + ker_x_n])*fmX0;
                        double t1 = (tsrc[i + ker_x_n + 1])*fmX0;
                        double t2 = (tsrc[i + ker_x_n + 2])*fmX0;
                        double t3 = (tsrc[i + ker_x_n + 3])*fmX0;

                        for( j = 1; j <= ker_x; j++ )
                        {
                            int j4 = j*4;
                            double m = fmaskX[j];
                            t0 += ((tsrc[i+ker_x_n+j4]) +
                                   (tsrc[i+ker_x_n-j4]))*m;
                            t1 += ((tsrc[i+ker_x_n+j4+1]) +
                                   (tsrc[i+ker_x_n-j4+1]))*m;
                            t2 += ((tsrc[i+ker_x_n+j4+2]) +
                                   (tsrc[i+ker_x_n-j4+2]))*m;
                            t3 += ((tsrc[i+ker_x_n+j4+3]) +
                                   (tsrc[i+ker_x_n-j4+3]))*m;
                        }

                        tdst[i] = (float)t0;
                        tdst[i+1] = (float)t1;
                        tdst[i+2] = (float)t2;
                        tdst[i+3] = (float)t3;
                    }
                }

                if( !need_copy )
                {
                    /* restore borders */
                    CV_COPY( src - ker_x_n, tbuf - ker_x_n, ker_x_n, x );
                    CV_COPY( src + width_n, tbuf, ker_right_n, x );
                }
            }
            else
            {
                CV_COPY( tdst, tsrc + ker_x_n, width_n, x );
            }

            if( crows < ker_height )
                src += srcStep;
        }

        if( starting_flag )
        {
            starting_flag = 0;
            trow = rows[ker_y];

            for( i = 0; i < ker_y; i++ )
            {
                tdst = rows[i];
                CV_COPY( tdst, trow, width_n, x );
            }
        }

        /* vertical convolution */
        if( crows != ker_height )
            break;

        tdst2 = dst;

        if( width_rest )
        {
            need_copy = width_n < CV_MORPH_ALIGN || y == dst_height - 1;

            if( need_copy )
                tdst2 = tbuf;
            else
                CV_COPY( tbuf + width_n, dst + width_n, CV_MORPH_ALIGN, x );
        }

        trow = rows[ker_y];

        for( x = 0; x < width_n; x += 4 )
        {
            double val0, val1, val2, val3;

            val0 = trow[x]*fmY0;
            val1 = trow[x + 1]*fmY0;
            val2 = trow[x + 2]*fmY0;
            val3 = trow[x + 3]*fmY0;

            for( i = 1; i <= ker_y; i++ )
            {
                float *trow1, *trow2;
                double m = fmaskY[i];
                trow1 = rows[ker_y - i];
                trow2 = rows[ker_y + i];
                val0 += (trow1[x] + trow2[x])*m;
                val1 += (trow1[x+1] + trow2[x+1])*m;
                val2 += (trow1[x+2] + trow2[x+2])*m;
                val3 += (trow1[x+3] + trow2[x+3])*m;
            }

            tdst2[x + 0] = (float)val0;
            tdst2[x + 1] = (float)val1;
            tdst2[x + 2] = (float)val2;
            tdst2[x + 3] = (float)val3;
        }

        if( width_rest )
        {
            if( need_copy )
                CV_COPY( dst, tbuf, width_n, x );
            else
                CV_COPY( dst + width_n, tbuf + width_n, CV_MORPH_ALIGN, x );
        }

        rows[ker_y] = saved_row;

        /* rotate buffer */
        {
            float *t = rows[0];

            CV_COPY( rows, rows + 1, ker_height - 1, i );
            rows[i] = t;
            crows--;
            dst += dstStep;
        }
    }
    while( ++y < dst_height );

    roiSize->height = y;
    state->crows = crows;

    return CV_OK;
}


/****************************************************************************************\
                                      Median Filter
\****************************************************************************************/

#define CV_MINMAX_8U(a,b) \
    (t = CV_FAST_CAST_8U((a) - (b)), (b) += t, a -= t)

static CvStatus CV_STDCALL
icvMedianBlur_8u_CnR( uchar* src, int src_step, uchar* dst, int dst_step,
                      CvSize* roiSize, int* param, int /*stub */ )
{
    #define N  16
    int     zone0[4][N];
    int     zone1[4][N*N];
    int     x, y;
    int     cn = param[0];
    int     m = param[1], n2 = m*m/2;
    int     nx = (m + 1)/2 - 1;
    CvSize  size = *roiSize;
    uchar*  src_max = src + size.height*src_step;
    uchar*  src_right = src + size.width*cn;

    #define UPDATE_ACC01( pix, cn, op ) \
    {                                   \
        int p = (pix);                  \
        zone1[cn][p] op;                \
        zone0[cn][p >> 4] op;           \
    }

    if( size.height < nx || size.width < nx )
        return CV_BADSIZE_ERR;

    if( m == 3 )
    {
        size.width *= cn;
        
        for( y = 0; y < size.height; y++, dst += dst_step )
        {
            const uchar* src0 = src + src_step*(y-1);
            const uchar* src1 = src0 + src_step;
            const uchar* src2 = src1 + src_step;
            if( y == 0 )
                src0 = src1;
            else if( y == size.height - 1 )
                src2 = src1;

            for( x = 0; x < 2*cn; x++ )
            {
                int x0 = x < cn ? x : size.width - 3*cn + x;
                int x2 = x < cn ? x + cn : size.width - 2*cn + x;
                int x1 = x < cn ? x0 : x2, t;

                int p0 = src0[x0], p1 = src0[x1], p2 = src0[x2];
                int p3 = src1[x0], p4 = src1[x1], p5 = src1[x2];
                int p6 = src2[x0], p7 = src2[x1], p8 = src2[x2];
            
                CV_MINMAX_8U(p1, p2);
                CV_MINMAX_8U(p4, p5);
                CV_MINMAX_8U(p7, p8);
                CV_MINMAX_8U(p0, p1);
                CV_MINMAX_8U(p3, p4);
                CV_MINMAX_8U(p6, p7);
                CV_MINMAX_8U(p1, p2);
                CV_MINMAX_8U(p4, p5);
                CV_MINMAX_8U(p7, p8);
                CV_MINMAX_8U(p0, p3);
                CV_MINMAX_8U(p5, p8);
                CV_MINMAX_8U(p4, p7);
                CV_MINMAX_8U(p3, p6);
                CV_MINMAX_8U(p1, p4);
                CV_MINMAX_8U(p2, p5);
                CV_MINMAX_8U(p4, p7);
                CV_MINMAX_8U(p4, p2);
                CV_MINMAX_8U(p6, p4);
                CV_MINMAX_8U(p4, p2);
                dst[x1] = (uchar)p4;
            }

            for( x = cn; x < size.width - cn; x++ )
            {
                int p0 = src0[x-cn], p1 = src0[x], p2 = src0[x+cn];
                int p3 = src1[x-cn], p4 = src1[x], p5 = src1[x+cn];
                int p6 = src2[x-cn], p7 = src2[x], p8 = src2[x+cn];
                int t;
            
                CV_MINMAX_8U(p1, p2);
                CV_MINMAX_8U(p4, p5);
                CV_MINMAX_8U(p7, p8);
                CV_MINMAX_8U(p0, p1);
                CV_MINMAX_8U(p3, p4);
                CV_MINMAX_8U(p6, p7);
                CV_MINMAX_8U(p1, p2);
                CV_MINMAX_8U(p4, p5);
                CV_MINMAX_8U(p7, p8);
                CV_MINMAX_8U(p0, p3);
                CV_MINMAX_8U(p5, p8);
                CV_MINMAX_8U(p4, p7);
                CV_MINMAX_8U(p3, p6);
                CV_MINMAX_8U(p1, p4);
                CV_MINMAX_8U(p2, p5);
                CV_MINMAX_8U(p4, p7);
                CV_MINMAX_8U(p4, p2);
                CV_MINMAX_8U(p6, p4);
                CV_MINMAX_8U(p4, p2);

                dst[x] = (uchar)p4;
            }
        }
        
        return CV_OK;
    }

    for( x = 0; x < size.width; x++, dst += cn )
    {
        uchar* dst_cur = dst;
        uchar* src_top = src;
        uchar* src_bottom = src;
        int    k, c;
        int    x0 = -1;

        if( x <= m/2 )
            nx++;

        if( nx < m )
            x0 = x < m/2 ? 0 : (nx-1)*cn;

        // init accumulator
        memset( zone0, 0, sizeof(zone0[0])*cn );
        memset( zone1, 0, sizeof(zone1[0])*cn );

        for( y = -m/2; y < m/2; y++ )
        {
            for( c = 0; c < cn; c++ )
            {
                if( x0 >= 0 )
                    UPDATE_ACC01( src_bottom[x0+c], c, += (m - nx) );
                for( k = 0; k < nx*cn; k += cn )
                    UPDATE_ACC01( src_bottom[k+c], c, ++ );
            }
            
            if( (unsigned)y < (unsigned)(size.height-1) )
                src_bottom += src_step;
        }

        for( y = 0; y < size.height; y++, dst_cur += dst_step )
        {
            if( cn == 1 )
            {
                for( k = 0; k < nx; k++ )
                    UPDATE_ACC01( src_bottom[k], 0, ++ );
            }
            else if( cn == 3 )
            {
                for( k = 0; k < nx*3; k += 3 )
                {
                    UPDATE_ACC01( src_bottom[k], 0, ++ );
                    UPDATE_ACC01( src_bottom[k+1], 1, ++ );
                    UPDATE_ACC01( src_bottom[k+2], 2, ++ );
                }
            }
            else
            {
                assert( cn == 4 );
                for( k = 0; k < nx*4; k += 4 )
                {
                    UPDATE_ACC01( src_bottom[k], 0, ++ );
                    UPDATE_ACC01( src_bottom[k+1], 1, ++ );
                    UPDATE_ACC01( src_bottom[k+2], 2, ++ );
                    UPDATE_ACC01( src_bottom[k+3], 3, ++ );
                }
            }

            if( x0 >= 0 )
            {
                for( c = 0; c < cn; c++ )
                    UPDATE_ACC01( src_bottom[x0+c], c, += (m - nx) );
            }

            if( src_bottom + src_step < src_max )
                src_bottom += src_step;

            // find median
            for( c = 0; c < cn; c++ )
            {
                int s = 0;
                for( k = 0; ; k++ )
                {
                    int t = s + zone0[c][k];
                    if( t > n2 ) break;
                    s = t;
                }

                for( k *= N; ;k++ )
                {
                    s += zone1[c][k];
                    if( s > n2 ) break;
                }

                dst_cur[c] = (uchar)k;
            }

            if( cn == 1 )
            {
                for( k = 0; k < nx; k++ )
                    UPDATE_ACC01( src_top[k], 0, -- );
            }
            else if( cn == 3 )
            {
                for( k = 0; k < nx*3; k += 3 )
                {
                    UPDATE_ACC01( src_top[k], 0, -- );
                    UPDATE_ACC01( src_top[k+1], 1, -- );
                    UPDATE_ACC01( src_top[k+2], 2, -- );
                }
            }
            else
            {
                assert( cn == 4 );
                for( k = 0; k < nx*4; k += 4 )
                {
                    UPDATE_ACC01( src_top[k], 0, -- );
                    UPDATE_ACC01( src_top[k+1], 1, -- );
                    UPDATE_ACC01( src_top[k+2], 2, -- );
                    UPDATE_ACC01( src_top[k+3], 3, -- );
                }
            }

            if( x0 >= 0 )
            {
                for( c = 0; c < cn; c++ )
                    UPDATE_ACC01( src_top[x0+c], c, -= (m - nx) );
            }

            if( y >= m/2 )
                src_top += src_step;
        }

        if( x >= m/2 )
            src += cn;
        if( src + nx*cn > src_right ) nx--;
    }
#undef N
#undef UPDATE_ACC
    return CV_OK;
}


/****************************************************************************************\
                                   Bilateral Filtering
\****************************************************************************************/

static CvStatus CV_STDCALL
icvBilateralFiltering_8u_CnR( uchar* src, int srcStep,
                              uchar* dst, int dstStep,
                              CvSize* roiSize, int* param, int /*stub*/ )
{
    CvSize size = *roiSize;
    
    int channels = param[0];
    double sigma_color = param[1]; 
    double sigma_space = param[2];

    double i2sigma_color = 1./(sigma_color*sigma_color);
    double i2sigma_space = 1./(sigma_space*sigma_space); 

    double mean1[3];
    double mean0;
    double w;
    int deltas[8];
    double weight_tab[8];
    
    int i, j;

#define INIT_C1\
            color = src[0]; \
            mean0 = 1; mean1[0] = color;

#define COLOR_DISTANCE_C1(c1, c2)\
            (c1 - c2)*(c1 - c2)
#define KERNEL_ELEMENT_C1(k)\
            temp_color = src[deltas[k]];\
            w = weight_tab[k] + COLOR_DISTANCE_C1(color, temp_color)*i2sigma_color;\
            w = 1./(w*w + 1); \
            mean0 += w;\
            mean1[0] += temp_color*w;

#define INIT_C3\
            mean0 = 1; mean1[0] = src[0];mean1[1] = src[1];mean1[2] = src[2];

#define UPDATE_OUTPUT_C1                   \
            dst[i] = (uchar)cvRound(mean1[0]/mean0);

#define COLOR_DISTANCE_C3(c1, c2)\
            ((c1[0] - c2[0])*(c1[0] - c2[0]) + \
             (c1[1] - c2[1])*(c1[1] - c2[1]) + \
             (c1[2] - c2[2])*(c1[2] - c2[2]))
#define KERNEL_ELEMENT_C3(k)\
            temp_color = src + deltas[k];\
            w = weight_tab[k] + COLOR_DISTANCE_C3(src, temp_color)*i2sigma_color;\
            w = 1./(w*w + 1); \
            mean0 += w;\
            mean1[0] += temp_color[0]*w; \
            mean1[1] += temp_color[1]*w; \
            mean1[2] += temp_color[2]*w;

#define UPDATE_OUTPUT_C3\
            mean0 = 1./mean0;\
            dst[i*3 + 0] = (uchar)cvRound(mean1[0]*mean0); \
            dst[i*3 + 1] = (uchar)cvRound(mean1[1]*mean0); \
            dst[i*3 + 2] = (uchar)cvRound(mean1[2]*mean0);

    CV_INIT_3X3_DELTAS( deltas, srcStep, channels );

    weight_tab[0] = weight_tab[2] = weight_tab[4] = weight_tab[6] = i2sigma_space;
    weight_tab[1] = weight_tab[3] = weight_tab[5] = weight_tab[7] = i2sigma_space*2;

    if( channels == 1 )
    {
        int color, temp_color;

        for( i = 0; i < size.width; i++, src++ )
        {
            INIT_C1;
            KERNEL_ELEMENT_C1(6);
            if( i > 0 )
            {
                KERNEL_ELEMENT_C1(5);
                KERNEL_ELEMENT_C1(4);
            }
            if( i < size.width - 1 )
            {
                KERNEL_ELEMENT_C1(7);
                KERNEL_ELEMENT_C1(0);
            }
            UPDATE_OUTPUT_C1;
        }

        src += srcStep - size.width;
        dst += dstStep;
    
        for( j = 1; j < size.height - 1; j++, dst += dstStep )
        {
            i = 0;
            INIT_C1;
            KERNEL_ELEMENT_C1(0);
            KERNEL_ELEMENT_C1(1);
            KERNEL_ELEMENT_C1(2);
            KERNEL_ELEMENT_C1(6);
            KERNEL_ELEMENT_C1(7);
            UPDATE_OUTPUT_C1;

            for( i = 1, src++; i < size.width - 1; i++, src++ )
            {
                INIT_C1;
                KERNEL_ELEMENT_C1(0);
                KERNEL_ELEMENT_C1(1);
                KERNEL_ELEMENT_C1(2);
                KERNEL_ELEMENT_C1(3);
                KERNEL_ELEMENT_C1(4);
                KERNEL_ELEMENT_C1(5);
                KERNEL_ELEMENT_C1(6);
                KERNEL_ELEMENT_C1(7);
                UPDATE_OUTPUT_C1;
            }

            INIT_C1;
            KERNEL_ELEMENT_C1(2);
            KERNEL_ELEMENT_C1(3);
            KERNEL_ELEMENT_C1(4);
            KERNEL_ELEMENT_C1(5);
            KERNEL_ELEMENT_C1(6);
            UPDATE_OUTPUT_C1;

            src += srcStep + 1 - size.width;
        }

        for( i = 0; i < size.width; i++, src++ )
        {
            INIT_C1;
            KERNEL_ELEMENT_C1(2);
            if( i > 0 )
            {
                KERNEL_ELEMENT_C1(3);
                KERNEL_ELEMENT_C1(4);
            }
            if( i < size.width - 1 )
            {
                KERNEL_ELEMENT_C1(1);
                KERNEL_ELEMENT_C1(0);
            }
            UPDATE_OUTPUT_C1;
        }
    }
    else
    {
        uchar* temp_color;

        if( channels != 3 )
            return CV_UNSUPPORTED_CHANNELS_ERR;
        
        for( i = 0; i < size.width; i++, src += 3 )
        {
            INIT_C3;
            KERNEL_ELEMENT_C3(6);
            if( i > 0 )
            {
                KERNEL_ELEMENT_C3(5);
                KERNEL_ELEMENT_C3(4);
            }
            if( i < size.width - 1 )
            {
                KERNEL_ELEMENT_C3(7);
                KERNEL_ELEMENT_C3(0);
            }
            UPDATE_OUTPUT_C3;
        }

        src += srcStep - size.width*3;
        dst += dstStep;
    
        for( j = 1; j < size.height - 1; j++, dst += dstStep )
        {
            i = 0;
            INIT_C3;
            KERNEL_ELEMENT_C3(0);
            KERNEL_ELEMENT_C3(1);
            KERNEL_ELEMENT_C3(2);
            KERNEL_ELEMENT_C3(6);
            KERNEL_ELEMENT_C3(7);
            UPDATE_OUTPUT_C3;

            for( i = 1, src += 3; i < size.width - 1; i++, src += 3 )
            {
                INIT_C3;
                KERNEL_ELEMENT_C3(0);
                KERNEL_ELEMENT_C3(1);
                KERNEL_ELEMENT_C3(2);
                KERNEL_ELEMENT_C3(3);
                KERNEL_ELEMENT_C3(4);
                KERNEL_ELEMENT_C3(5);
                KERNEL_ELEMENT_C3(6);
                KERNEL_ELEMENT_C3(7);
                UPDATE_OUTPUT_C3;
            }

            INIT_C3;
            KERNEL_ELEMENT_C3(2);
            KERNEL_ELEMENT_C3(3);
            KERNEL_ELEMENT_C3(4);
            KERNEL_ELEMENT_C3(5);
            KERNEL_ELEMENT_C3(6);
            UPDATE_OUTPUT_C3;

            src += srcStep + 3 - size.width*3;
        }

        for( i = 0; i < size.width; i++, src += 3 )
        {
            INIT_C3;
            KERNEL_ELEMENT_C3(2);
            if( i > 0 )
            {
                KERNEL_ELEMENT_C3(3);
                KERNEL_ELEMENT_C3(4);
            }
            if( i < size.width - 1 )
            {
                KERNEL_ELEMENT_C3(1);
                KERNEL_ELEMENT_C3(0);
            }
            UPDATE_OUTPUT_C3;
        }
    }

    return CV_OK;
#undef INIT_C1
#undef KERNEL_ELEMENT_C1
#undef UPDATE_OUTPUT_C1
#undef INIT_C3
#undef KERNEL_ELEMENT_C3
#undef UPDATE_OUTPUT_C3
#undef COLOR_DISTANCE_C3
}


static void icvInitSmoothTab( CvFuncTable* blur_no_scale_tab, 
                              CvFuncTable* blur_tab, CvFuncTable* gaussian_tab,
                              CvFuncTable* median_tab, CvFuncTable* bilateral_tab )
{
    blur_no_scale_tab->fn_2d[CV_8U] = (void*)icvBlur_8u16s_C1R;
    blur_no_scale_tab->fn_2d[CV_32F] = (void*)icvBlur_32f_CnR;
    
    blur_tab->fn_2d[CV_8U] = (void*)icvBlur_8u_CnR;
    blur_tab->fn_2d[CV_32F] = (void*)icvBlur_32f_CnR;

    gaussian_tab->fn_2d[CV_8U] = (void*)icvGaussianBlur_8u_CnR;
    gaussian_tab->fn_2d[CV_32F] = (void*)icvGaussianBlur_32f_CnR;

    median_tab->fn_2d[CV_8U] = (void*)icvMedianBlur_8u_CnR;

    bilateral_tab->fn_2d[CV_8U] = (void*)icvBilateralFiltering_8u_CnR;
}


//////////////////////////////// IPP smoothing functions /////////////////////////////////

icvFilterMedian_8u_C1R_t icvFilterMedian_8u_C1R_p = 0;
icvFilterMedian_8u_C3R_t icvFilterMedian_8u_C3R_p = 0;
icvFilterMedian_8u_C4R_t icvFilterMedian_8u_C4R_p = 0;

icvFilterBox_8u_C1R_t icvFilterBox_8u_C1R_p = 0;
icvFilterBox_8u_C3R_t icvFilterBox_8u_C3R_p = 0;
icvFilterBox_8u_C4R_t icvFilterBox_8u_C4R_p = 0;
icvFilterBox_32f_C1R_t icvFilterBox_32f_C1R_p = 0;
icvFilterBox_32f_C3R_t icvFilterBox_32f_C3R_p = 0;
icvFilterBox_32f_C4R_t icvFilterBox_32f_C4R_p = 0;

typedef CvStatus (CV_STDCALL * CvSmoothFixedIPPFunc)
( const void* src, int srcstep, void* dst, int dststep,
  CvSize size, CvSize ksize, CvPoint anchor );

//////////////////////////////////////////////////////////////////////////////////////////

CV_IMPL void
cvSmooth( const void* srcarr, void* dstarr, int smoothtype,
          int param1, int param2, double param3 )
{
    static CvFuncTable smooth_tab[5];
    static int inittab = 0;
    CvFilterState *state = 0;
    CvMat* temp = 0;

    CV_FUNCNAME( "cvSmooth" );

    __BEGIN__;

    CvFilterFunc func = 0;
    int coi1 = 0, coi2 = 0;
    CvMat srcstub, *src = (CvMat*)srcarr;
    CvMat dststub, *dst = (CvMat*)dstarr;
    CvSize size;
    int type, depth, dsttype;
    int src_step, dst_step;
    int nonlin_param[] = { 0, param1, param2 };
    void* ptr = nonlin_param;
    double sigma = 0;

    if( !inittab )
    {
        icvInitSmoothTab( smooth_tab + CV_BLUR_NO_SCALE, smooth_tab + CV_BLUR,
                          smooth_tab + CV_GAUSSIAN, smooth_tab + CV_MEDIAN,
                          smooth_tab + CV_BILATERAL );
        inittab = 1;
    }

    CV_CALL( src = cvGetMat( src, &srcstub, &coi1 ));
    CV_CALL( dst = cvGetMat( dst, &dststub, &coi2 ));

    if( coi1 != 0 || coi2 != 0 )
        CV_ERROR( CV_BadCOI, "" );

    type = CV_MAT_TYPE( src->type );
    dsttype = CV_MAT_TYPE( dst->type );

    if( smoothtype > 0 && !CV_ARE_TYPES_EQ( src, dst ) ||
        smoothtype == 0 &&
        !(type == CV_8UC1 && dsttype == CV_16SC1 ||
          type == CV_32FC1 && dsttype == CV_32FC1 ))
        CV_ERROR( CV_StsUnmatchedFormats, "" );

    if( !CV_ARE_SIZES_EQ( src, dst ))
        CV_ERROR( CV_StsUnmatchedSizes, "" );

    size = cvGetMatSize( src );
    
    depth = CV_MAT_DEPTH(type);
    nonlin_param[0] = CV_MAT_CN(type);

    if( (unsigned)smoothtype > CV_BILATERAL )
        CV_ERROR( CV_StsBadArg, "Unsupported smoothing type" );

    if( (smoothtype == CV_MEDIAN || smoothtype == CV_BILATERAL) &&
        src->data.ptr == dst->data.ptr )
        CV_ERROR( CV_StsBadArg,
        "Inplace operation is not supported for that type of smoothing" );

    if( smoothtype == CV_BILATERAL )
    {
        if( param1 < 0 || param2 < 0 )
            CV_ERROR( CV_StsOutOfRange,
            "Thresholds in bilaral filtering should not bee negative" );
        param1 += param1 == 0;
        param2 += param2 == 0;
    }
    else /* simple blurring, gaussian, median */
    {
        sigma = param3;
        
        // automatic detection of kernel size from sigma
        if( smoothtype == CV_GAUSSIAN && param1 == 0 && sigma > 0 )
            param1 = cvRound(sigma*(depth == CV_8U ? 3 : 4)*2 + 1)|1;
        
        if( param2 == 0 )
            param2 = size.height == 1 ? 1 : param1;
        if( param1 < 1 || (param1 & 1) == 0 || param2 < 1 || (param2 & 1) == 0 )
            CV_ERROR( CV_StsOutOfRange,
                "One of aperture dimensions is incorrect (should be >=1 and odd)" );

        if( param1 == 1 && param2 == 1 && CV_ARE_TYPES_EQ(src,dst) )
        {
            cvCopy( src, dst );
            EXIT;
        }
    }

    src_step = src->step;
    dst_step = dst->step;
    if( size.height == 1 )
        src_step = dst_step = CV_AUTOSTEP;

    if( CV_MAT_CN(type) == 2 )
        CV_ERROR( CV_BadNumChannels, "Unsupported number of channels" );

    if( (smoothtype == CV_BLUR || smoothtype == CV_MEDIAN) && icvFilterBox_8u_C1R_p &&
        size.width >= param1 && size.height >= param2 && param1 > 1 && param2 > 1 )
    {
        CvSmoothFixedIPPFunc ipp_median_box_func = 0;

        if( smoothtype == CV_BLUR )
        {
            ipp_median_box_func =
                type == CV_8UC1 ? icvFilterBox_8u_C1R_p :
                type == CV_8UC3 ? icvFilterBox_8u_C3R_p :
                type == CV_8UC4 ? icvFilterBox_8u_C4R_p :
                type == CV_32FC1 ? icvFilterBox_32f_C1R_p :
                type == CV_32FC3 ? icvFilterBox_32f_C3R_p :
                type == CV_32FC4 ? icvFilterBox_32f_C4R_p : 0;
        }
        else if( smoothtype == CV_MEDIAN )
        {
            ipp_median_box_func =
                type == CV_8UC1 ? icvFilterMedian_8u_C1R_p :
                type == CV_8UC3 ? icvFilterMedian_8u_C3R_p :
                type == CV_8UC4 ? icvFilterMedian_8u_C4R_p : 0;
        }
        
        if( ipp_median_box_func )
        {
            CvSize el_size = { param1, param2 };
            CvPoint el_anchor = { param1/2, param2/2 };
            int stripe_size = 1 << 14; // the optimal value may depend on CPU cache,
                                       // overhead of current IPP code etc.
            const uchar* shifted_ptr;
            int y, dy = 0;
            int temp_step;
            int dst_step = dst->step ? dst->step : CV_STUB_STEP;

            CV_CALL( temp = icvIPPFilterInit( src, stripe_size, el_size ));
            
            shifted_ptr = temp->data.ptr +
                el_anchor.y*temp->step + el_anchor.x*CV_ELEM_SIZE(type);
            temp_step = temp->step ? temp->step : CV_STUB_STEP;

            for( y = 0; y < src->rows; y += dy )
            {
                dy = icvIPPFilterNextStripe( src, temp, y, el_size, el_anchor );
                IPPI_CALL( ipp_median_box_func( shifted_ptr, temp_step,
                    dst->data.ptr + y*dst_step, dst_step, cvSize(src->cols, dy),
                    el_size, el_anchor ));
            }
            EXIT;
        }
    }
    else if( smoothtype == CV_GAUSSIAN &&
             icvFilterBox_8u_C1R_p /* this is to check that IPP is loaded */ &&
             size.width >= param1*3 && size.height >= param2 && param1 > 1 && param2 > 1 )
    {
        CvSize ksize = { param1, param2 };
        float* kx = (float*)cvStackAlloc( ksize.width*sizeof(kx[0]) );
        float* ky = (float*)cvStackAlloc( ksize.height*sizeof(ky[0]) );
        CvMat KX = cvMat( 1, ksize.width, CV_32F, kx );
        CvMat KY = cvMat( 1, ksize.height, CV_32F, ky );
        int done = 0;

        icvCalcGaussianKernel( ksize.width, sigma, kx );
        if( ksize.width != ksize.height )
            icvCalcGaussianKernel( ksize.height, sigma, ky );
        else
            KY.data.fl = kx;

        CV_CALL( done = icvIPPSepFilter( src, dst, &KX, &KY,
                        cvPoint(ksize.width/2,ksize.height/2)));
        if( done )
            EXIT;
    }

    if( smoothtype <= CV_GAUSSIAN )
    {
        IPPI_CALL( icvSmoothInitAlloc( src->width, depth < CV_32F ? cv32s : cv32f,
            CV_MAT_CN(type), cvSize(param1, smoothtype <= CV_GAUSSIAN ? param2 : param1),
            smoothtype, sigma, &state ));
        ptr = state;
    }

    func = (CvFilterFunc)(smooth_tab[smoothtype].fn_2d[depth]);

    if( !func )
        CV_ERROR( CV_StsUnsupportedFormat, "" );

    IPPI_CALL( func( src->data.ptr, src_step, dst->data.ptr,
                     dst_step, &size, (CvFilterState*)ptr, 0 ));

    __END__;

    cvReleaseMat( &temp );
    icvSmoothFree( &state );
}

/* End of file. */
