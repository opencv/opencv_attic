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
#include <limits.h>

IPCVAPI_IMPL( CvStatus,
    icvMorphologyInitAlloc, ( int roiWidth, CvDataType dataType, int channels,
                              CvSize elSize, CvPoint elAnchor,
                              CvElementShape elShape, int* elData,
                              struct CvMorphState** morphState ))
{
    CvStatus status;
    
    switch( elShape )
    {
    case CV_SHAPE_RECT:
    case CV_SHAPE_CROSS:
        break;
    case CV_SHAPE_ELLIPSE:
        {
            int i, r = elSize.height / 2, c = elSize.width / 2;
            double inv_r2 = 1. / (((double) (r)) * (r));

            elData = (int*)alloca( elSize.width * elSize.height * sizeof(elData[0]));
            memset( elData, 0, elSize.width*elSize.height*sizeof(elData[0]));

            for( i = 0; i < r; i++ )
            {
                int y = r - i;
                int dx = cvRound( c * sqrt( (r * r - y * y) * inv_r2 ));
                int x1 = c - dx;
                int x2 = c + dx;

                if( x1 < 0 )
                    x1 = 0;
                if( x2 >= elSize.width )
                    x2 = elSize.width;
                x2 = (x2 - x1 - 1)*sizeof(elData[0]);

                memset( elData + i * elSize.width + x1, -1, x2 );
                memset( elData + (elSize.height - i - 1) * elSize.width + x1, -1, x2 );
            }

            elShape = CV_SHAPE_CUSTOM;
        }
        break;
    case CV_SHAPE_CUSTOM:
        if( elShape == CV_SHAPE_CUSTOM && elData == 0 )
            return CV_NULLPTR_ERR;
        break;
    default:
        return CV_BADFLAG_ERR;
    }

    status = icvFilterInitAlloc( roiWidth, dataType, channels, elSize, elAnchor,
                                 elShape == CV_SHAPE_CUSTOM ? elData : 0,
                                 ICV_MAKE_BINARY_KERNEL(elShape), morphState );

    if( status < 0 )
        return status;

    if( !morphState )
        return CV_NOTDEFINED_ERR;

    return CV_OK;
}


IPCVAPI_IMPL( CvStatus, icvMorphologyFree, (CvMorphState ** morphState) )
{
    return icvFilterFree( morphState );
}


static CvStatus
icvErodeRC_8u( uchar * src, int srcStep,
               uchar * dst, int dstStep, CvSize * roiSize, CvMorphState * state, int stage )
{
    int width = roiSize->width;
    int src_height = roiSize->height;
    int dst_height = src_height;
    int x, y = 0, i;

    int ker_x = state->ker_x;
    int ker_y = state->ker_y;
    int ker_width = state->ker_width;
    int ker_height = state->ker_height;
    int ker_right = ker_width - ker_x - ((width & 1) == 0);

    int crows = state->crows;
    uchar **rows = (uchar **) (state->rows);
    uchar *tbuf = (uchar *) (state->tbuf);

    int channels = state->channels;
    int ker_x_n = ker_x * channels;
    int ker_width_n = ker_width * channels;
    int ker_right_n = ker_right * channels;
    int width_n = width * channels;

    int is_small_width = width < MAX( ker_x, ker_right );
    int starting_flag = 0;
    int width_rest = width_n & (CV_MORPH_ALIGN - 1);

    int is_cross = ICV_BINARY_KERNEL_SHAPE(state->kerType) == CV_SHAPE_CROSS;

    /* initialize cyclic buffer when starting */
    if( stage == CV_WHOLE || stage == CV_START )
    {
        for( i = 0; i < ker_height; i++ )
        {
            rows[i] = (uchar *) (state->buffer + state->buffer_step * i);
        }
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
        uchar *tsrc, *tdst;
        uchar *saved_row = rows[ker_y];

        /* fill cyclic buffer - horizontal filtering */
        for( ; crows < ker_height + is_cross; crows++ )
        {
            if( crows < ker_height )
            {
                tsrc = src - ker_x_n;
                tdst = rows[crows];

                if( src_height-- <= 0 )
                {
                    if( stage != CV_WHOLE && stage != CV_END )
                        break;
                    /* duplicate last row */
                    tsrc = rows[crows - 1];
                    CV_COPY( tdst, tsrc, width_n, x );
                    continue;
                }

                need_copy |= src_height == 1;
            }
            else
            {
                /* convolve center line for cross-shaped element */
                tsrc = rows[ker_y] - ker_x_n;
                tdst = tbuf;

                need_copy = 0;
            }

            if( ker_width > 1 && (!is_cross || crows == ker_height) )
            {
                if( need_copy )
                {
                    tsrc = tbuf - ker_x_n;
                    CV_COPY( tbuf, src, width_n, x );
                }
                else if( !is_cross )
                {
                    CV_COPY( tbuf - ker_x_n, src - ker_x_n, ker_x_n, x );
                    CV_COPY( tbuf, src + width_n, ker_right_n, x );
                }

                if( channels == 1 )
                {
                    /* make replication borders */
                    uchar pix = tsrc[ker_x];

                    CV_SET( tsrc, pix, ker_x, x );

                    pix = tsrc[width + ker_x - 1];
                    CV_SET( tsrc + width + ker_x, pix, ker_right, x );

                    /* horizontal convolution loop */
                    for( i = 0; i < width_n; i += 2 )
                    {
                        int j;
                        int t, t0 = tsrc[i + 1];

                        for( j = 2; j < ker_width_n; j++ )
                        {
                            int t1 = tsrc[i + j];

                            CV_CALC_MIN( t0, t1 );
                        }

                        t = tsrc[i];
                        CV_CALC_MIN( t, t0 );
                        tdst[i] = (uchar) t;

                        t = tsrc[i + j];
                        CV_CALC_MIN( t, t0 );
                        tdst[i + 1] = (uchar) t;
                    }
                }
                else if( channels == 3 )
                {
                    /* make replication borders */
                    CvRGB8u pix = ((CvRGB8u *) tsrc)[ker_x];

                    CV_SET( (CvRGB8u *) tsrc, pix, ker_x, x );

                    pix = ((CvRGB8u *) tsrc)[width + ker_x - 1];
                    CV_SET( (CvRGB8u *) tsrc + width + ker_x, pix, ker_right, x );

                    /* horizontal convolution loop */
                    for( i = 0; i < width_n; i++ )
                    {
                        int j;
                        int t0 = tsrc[i];

                        for( j = 3; j < ker_width_n; j += 3 )
                        {
                            int t1 = tsrc[i + j];

                            CV_CALC_MIN( t0, t1 );
                        }

                        tdst[i] = (uchar) t0;
                    }
                }
                else            /* channels == 4 */
                {
                    /* make replication borders */
                    CvRGBA8u pix = ((CvRGBA8u *) tsrc)[ker_x];

                    CV_SET( (CvRGBA8u *) tsrc, pix, ker_x, x );

                    pix = ((CvRGBA8u *) tsrc)[width + ker_x - 1];
                    CV_SET( (CvRGBA8u *) tsrc + width + ker_x, pix, ker_right, x );

                    /* horizontal convolution loop */
                    for( i = 0; i < width_n; i++ )
                    {
                        int j;
                        int t0 = tsrc[i];

                        for( j = 4; j < ker_width_n; j += 4 )
                        {
                            int t1 = tsrc[i + j];

                            CV_CALC_MIN( t0, t1 );
                        }

                        tdst[i] = (uchar) t0;
                    }
                }

                if( !need_copy && !is_cross )
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

            src += crows < ker_height ? srcStep : 0;
        }

        if( starting_flag )
        {
            starting_flag = 0;
            tsrc = rows[ker_y];

            for( i = 0; i < ker_y; i++ )
            {
                tdst = rows[i];
                CV_COPY( tdst, tsrc, width_n, x );
            }
        }

        /* vertical convolution */
        if( crows != ker_height )
        {
            if( crows < ker_height )
                break;
            /* else it is cross-shaped element: change central line */
            rows[ker_y] = tbuf;
            crows--;
        }

        tdst = dst;

        if( width_rest )
        {
            need_copy = width_n < CV_MORPH_ALIGN || y == dst_height - 1;

            if( need_copy )
                tdst = tbuf;
            else
                CV_COPY( tbuf + width_n, dst + width_n, CV_MORPH_ALIGN, x );
        }

        for( x = 0; x < width_n; x += 4 )
        {
            int val0, val1, val2, val3;

            tsrc = rows[0];

            val0 = tsrc[x];
            val1 = tsrc[x + 1];
            val2 = tsrc[x + 2];
            val3 = tsrc[x + 3];

            for( i = 1; i < ker_height; i++ )
            {
                int s;

                tsrc = rows[i];

                s = tsrc[x + 0];
                CV_CALC_MIN( val0, s );
                s = tsrc[x + 1];
                CV_CALC_MIN( val1, s );
                s = tsrc[x + 2];
                CV_CALC_MIN( val2, s );
                s = tsrc[x + 3];
                CV_CALC_MIN( val3, s );
            }

            tdst[x + 0] = (uchar) val0;
            tdst[x + 1] = (uchar) val1;
            tdst[x + 2] = (uchar) val2;
            tdst[x + 3] = (uchar) val3;
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
            uchar *t = rows[0];

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


static CvStatus
icvErodeArb_8u( uchar * src, int srcStep,
                uchar * dst, int dstStep, CvSize * roiSize, CvMorphState * state, int stage )
{
#define INIT_VAL 255
    int width = roiSize->width;
    int src_height = roiSize->height;
    int dst_height = src_height;
    int x, y = 0, i;

    int ker_x = state->ker_x;
    int ker_y = state->ker_y;
    int ker_width = state->ker_width;
    int ker_height = state->ker_height;
    int ker_right = ker_width - ker_x - ((width & 1) == 0);
    uchar *ker_data = state->ker1 + ker_width;

    int crows = state->crows;
    uchar **rows = (uchar **) (state->rows);
    uchar *tbuf = (uchar *) (state->tbuf);

    int channels = state->channels;
    int ker_x_n = ker_x * channels;
    int ker_width_n = ker_width * channels;
    int width_n = width * channels;

    int starting_flag = 0;
    int width_rest = width_n & (CV_MORPH_ALIGN - 1);

    /* initialize cyclic buffer when starting */
    if( stage == CV_WHOLE || stage == CV_START )
    {
        for( i = 0; i < ker_height; i++ )
        {
            rows[i] = (uchar *) (state->buffer + state->buffer_step * i);
        }
        crows = ker_y;
        if( stage != CV_WHOLE )
            dst_height -= ker_height - ker_y - 1;
        starting_flag = 1;
    }

    if( stage == CV_END )
        dst_height += ker_height - ker_y - 1;

    do
    {
        uchar *tsrc, *tdst;
        int need_copy = 0;

        /* fill cyclic buffer - horizontal filtering */
        for( ; crows < ker_height; crows++ )
        {
            tsrc = src;
            tdst = rows[crows];

            if( src_height-- <= 0 )
            {
                if( stage != CV_WHOLE && stage != CV_END )
                    break;
                /* duplicate last row */
                tsrc = rows[crows - 1];
                CV_COPY( tdst, tsrc, width_n + ker_width_n, x );
                continue;
            }

            src += srcStep;

            CV_COPY( tdst + ker_x_n, tsrc, width_n, x );

            /* make replication borders */
            if( channels == 1 )
            {
                uchar pix = tdst[ker_x];

                CV_SET( tdst, pix, ker_x, x );

                pix = tdst[width + ker_x - 1];
                CV_SET( tdst + width + ker_x, pix, ker_right, x );
            }
            else if( channels == 3 )
            {
                CvRGB8u pix = ((CvRGB8u *) tdst)[ker_x];

                CV_SET( (CvRGB8u *) tdst, pix, ker_x, x );

                pix = ((CvRGB8u *) tdst)[width + ker_x - 1];
                CV_SET( (CvRGB8u *) tdst + width + ker_x, pix, ker_right, x );
            }
            else                /* channels == 4 */
            {
                /* make replication borders */
                CvRGBA8u pix = ((CvRGBA8u *) tdst)[ker_x];

                CV_SET( (CvRGBA8u *) tdst, pix, ker_x, x );

                pix = ((CvRGBA8u *) tdst)[width + ker_x - 1];
                CV_SET( (CvRGBA8u *) tdst + width + ker_x, pix, ker_right, x );
            }
        }

        if( starting_flag )
        {
            starting_flag = 0;
            tsrc = rows[ker_y];

            for( i = 0; i < ker_y; i++ )
            {
                tdst = rows[i];
                CV_COPY( tdst, tsrc, width_n + ker_width_n, x );
            }
        }

        /* vertical convolution */
        if( crows < ker_height )
            break;

        tdst = dst;
        if( width_rest )
        {
            need_copy = width_n < CV_MORPH_ALIGN || y == dst_height - 1;

            if( need_copy )
                tdst = tbuf;
            else
                CV_COPY( tbuf + width_n, dst + width_n, CV_MORPH_ALIGN, x );
        }

        if( channels == 1 )
        {
            for( x = 0; x < width_n; x += 4 )
            {
                int val0 = INIT_VAL, val1 = INIT_VAL, val2 = INIT_VAL, val3 = INIT_VAL;

                uchar *ker = ker_data;

                for( i = 0; i < ker_height; i++, ker += ker_width )
                {
                    int j = -ker_width;

                    tsrc = rows[i] + x - j;
                    do
                    {
                        int m = ker[j];
                        int t = tsrc[j] | m;

                        CV_CALC_MIN( val0, t );
                        t = tsrc[j + 1] | m;
                        CV_CALC_MIN( val1, t );
                        t = tsrc[j + 2] | m;
                        CV_CALC_MIN( val2, t );
                        t = tsrc[j + 3] | m;
                        CV_CALC_MIN( val3, t );
                    }
                    while( ++j < 0 );
                }

                tdst[x] = (uchar) val0;
                tdst[x + 1] = (uchar) val1;
                tdst[x + 2] = (uchar) val2;
                tdst[x + 3] = (uchar) val3;
            }
        }
        else if( channels == 3 )
        {
            for( x = 0; x < width_n; x += 3 )
            {
                int val0 = INIT_VAL, val1 = INIT_VAL, val2 = INIT_VAL;

                uchar *ker = ker_data;

                for( i = 0; i < ker_height; i++, ker += ker_width )
                {
                    int j = -ker_width;

                    tsrc = rows[i] + x - j * 3;
                    for( ; j < 0; j++ )
                    {
                        int m = ker[j];
                        int t = tsrc[j * 3] | m;

                        CV_CALC_MIN( val0, t );
                        t = tsrc[j * 3 + 1] | m;
                        CV_CALC_MIN( val1, t );
                        t = tsrc[j * 3 + 2] | m;
                        CV_CALC_MIN( val2, t );
                    }
                }

                tdst[x] = (uchar) val0;
                tdst[x + 1] = (uchar) val1;
                tdst[x + 2] = (uchar) val2;
            }
        }
        else
        {
            for( x = 0; x < width_n; x += 4 )
            {
                int val0 = INIT_VAL, val1 = INIT_VAL, val2 = INIT_VAL, val3 = INIT_VAL;

                uchar *ker = ker_data;

                for( i = 0; i < ker_height; i++, ker += ker_width )
                {
                    int j = -ker_width;

                    tsrc = rows[i] + x - j * 4;
                    for( ; j < 0; j++ )
                    {
                        int m = ker[j];
                        int t = tsrc[j * 4] | m;

                        CV_CALC_MIN( val0, t );
                        t = tsrc[j * 4 + 1] | m;
                        CV_CALC_MIN( val1, t );
                        t = tsrc[j * 4 + 2] | m;
                        CV_CALC_MIN( val2, t );
                        t = tsrc[j * 4 + 3] | m;
                        CV_CALC_MIN( val3, t );
                    }
                }

                tdst[x] = (uchar) val0;
                tdst[x + 1] = (uchar) val1;
                tdst[x + 2] = (uchar) val2;
                tdst[x + 3] = (uchar) val3;
            }
        }

        if( width_rest )
        {
            if( need_copy )
                CV_COPY( dst, tbuf, width_n, x );
            else
                CV_COPY( dst + width_n, tbuf + width_n, CV_MORPH_ALIGN, x );
        }

        /* rotate buffer */
        {
            uchar *t = rows[0];

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
#undef INIT_VAL
}


static CvStatus
icvErodeRC_32f( float *src, int srcStep,
                float *dst, int dstStep, CvSize * roiSize, CvMorphState * state, int stage )
{
    int width = roiSize->width;
    int src_height = roiSize->height;
    int dst_height = src_height;
    int x, y = 0, i;

    int ker_x = state->ker_x;
    int ker_y = state->ker_y;
    int ker_width = state->ker_width;
    int ker_height = state->ker_height;
    int ker_right = ker_width - ker_x - ((width & 1) == 0);

    int crows = state->crows;
    int **rows = (int **) (state->rows);
    int *tbuf = (int *) (state->tbuf);
    int *tbuf2 = (int *) (state->tbuf + state->buffer_step);

    int channels = state->channels;
    int ker_x_n = ker_x * channels;
    int ker_width_n = ker_width * channels;
    int width_n = width * channels;

    int starting_flag = 0;
    int width_rest = width_n & (CV_MORPH_ALIGN - 1);

    int is_cross = ICV_BINARY_KERNEL_SHAPE(state->kerType) == CV_SHAPE_CROSS;

    /* initialize cyclic buffer when starting */
    if( stage == CV_WHOLE || stage == CV_START )
    {
        for( i = 0; i < ker_height; i++ )
        {
            rows[i] = (int *) (state->buffer + state->buffer_step * i);
        }
        crows = ker_y;
        if( stage != CV_WHOLE )
            dst_height -= ker_height - ker_y - 1;
        starting_flag = 1;
    }

    if( stage == CV_END )
        dst_height += ker_height - ker_y - 1;

    srcStep /= sizeof_float;
    dstStep /= sizeof_float;

    do
    {
        int need_copy = 0;
        int *tsrc, *tdst;
        int *saved_row = rows[ker_y];

        /* fill cyclic buffer - horizontal filtering */
        for( ; crows < ker_height + is_cross; crows++ )
        {
            if( crows < ker_height )
            {
                tsrc = (int *) src;
                tdst = rows[crows];

                if( src_height-- <= 0 )
                {
                    if( stage != CV_WHOLE && stage != CV_END )
                        break;
                    /* duplicate last row */
                    tsrc = rows[crows - 1];
                    CV_COPY( tdst, tsrc, width_n, x );
                    continue;
                }

                for( x = 0; x < width_n; x++ )
                {
                    int t = tsrc[x];

                    tbuf2[x] = CV_TOGGLE_FLT( t );
                }
                tsrc = tbuf2 - ker_x_n;
            }
            else
            {
                /* convolve center line for cross-shaped element */
                tsrc = rows[ker_y] - ker_x_n;
                tdst = tbuf;
            }

            if( ker_width > 1 && (!is_cross || crows == ker_height) )
            {
                if( channels == 1 )
                {
                    /* make replication borders */
                    int pix = tsrc[ker_x];

                    CV_SET( tsrc, pix, ker_x, x );

                    pix = tsrc[width + ker_x - 1];
                    CV_SET( tsrc + width + ker_x, pix, ker_right, x );

                    /* horizontal convolution loop */
                    for( i = 0; i < width_n; i += 2 )
                    {
                        int j;
                        int t, t0 = tsrc[i + 1];

                        for( j = 2; j < ker_width_n; j++ )
                        {
                            int t1 = tsrc[i + j];

                            CV_CALC_MIN( t0, t1 );
                        }

                        t = tsrc[i];
                        CV_CALC_MIN( t, t0 );
                        tdst[i] = (int) t;

                        t = tsrc[i + j];
                        CV_CALC_MIN( t, t0 );
                        tdst[i + 1] = (int) t;
                    }
                }
                else if( channels == 3 )
                {
                    /* make replication borders */
                    CvRGB32s pix = ((CvRGB32s *) tsrc)[ker_x];

                    CV_SET( (CvRGB32s *) tsrc, pix, ker_x, x );

                    pix = ((CvRGB32s *) tsrc)[width + ker_x - 1];
                    CV_SET( (CvRGB32s *) tsrc + width + ker_x, pix, ker_right, x );

                    /* horizontal convolution loop */
                    for( i = 0; i < width_n; i++ )
                    {
                        int j;
                        int t0 = tsrc[i];

                        for( j = 3; j < ker_width_n; j += 3 )
                        {
                            int t1 = tsrc[i + j];

                            CV_CALC_MIN( t0, t1 );
                        }

                        tdst[i] = (int) t0;
                    }
                }
                else            /* channels == 4 */
                {
                    /* make replication borders */
                    CvRGBA32s pix = ((CvRGBA32s *) tsrc)[ker_x];

                    CV_SET( (CvRGBA32s *) tsrc, pix, ker_x, x );

                    pix = ((CvRGBA32s *) tsrc)[width + ker_x - 1];
                    CV_SET( (CvRGBA32s *) tsrc + width + ker_x, pix, ker_right, x );

                    /* horizontal convolution loop */
                    for( i = 0; i < width_n; i++ )
                    {
                        int j;
                        int t0 = tsrc[i];

                        for( j = 4; j < ker_width_n; j += 4 )
                        {
                            int t1 = tsrc[i + j];

                            CV_CALC_MIN( t0, t1 );
                        }

                        tdst[i] = (int) t0;
                    }
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
            tsrc = rows[ker_y];

            for( i = 0; i < ker_y; i++ )
            {
                tdst = rows[i];
                CV_COPY( tdst, tsrc, width_n, x );
            }
        }

        /* vertical convolution */
        if( crows != ker_height )
        {
            if( crows < ker_height )
                break;
            /* else it is cross-shaped element: change central line */
            rows[ker_y] = tbuf;
            crows--;
        }

        tdst = (int *) dst;

        if( width_rest )
        {
            need_copy = width_n < CV_MORPH_ALIGN || y == dst_height - 1;

            if( need_copy )
                tdst = tbuf;
            else
                CV_COPY( tbuf + width_n, tdst + width_n, CV_MORPH_ALIGN, x );
        }

        for( x = 0; x < width_n; x += 4 )
        {
            int val0, val1, val2, val3;

            tsrc = rows[0];

            val0 = tsrc[x];
            val1 = tsrc[x + 1];
            val2 = tsrc[x + 2];
            val3 = tsrc[x + 3];

            for( i = 1; i < ker_height; i++ )
            {
                int s;

                tsrc = rows[i];

                s = tsrc[x + 0];
                CV_CALC_MIN( val0, s );
                s = tsrc[x + 1];
                CV_CALC_MIN( val1, s );
                s = tsrc[x + 2];
                CV_CALC_MIN( val2, s );
                s = tsrc[x + 3];
                CV_CALC_MIN( val3, s );
            }
            tdst[x + 0] = CV_TOGGLE_FLT( val0 );
            tdst[x + 1] = CV_TOGGLE_FLT( val1 );
            tdst[x + 2] = CV_TOGGLE_FLT( val2 );
            tdst[x + 3] = CV_TOGGLE_FLT( val3 );
        }

        if( width_rest )
        {
            if( need_copy )
                CV_COPY( (int *) dst, tbuf, width_n, x );
            else
                CV_COPY( tdst + width_n, tbuf + width_n, CV_MORPH_ALIGN, x );
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


static CvStatus
icvErodeArb_32f( float *src, int srcStep,
                 float *dst, int dstStep, CvSize * roiSize, CvMorphState * state, int stage )
{
#define INIT_VAL  INT_MAX
    int width = roiSize->width;
    int src_height = roiSize->height;
    int dst_height = src_height;
    int x, y = 0, i;

    int ker_x = state->ker_x;
    int ker_y = state->ker_y;
    int ker_width = state->ker_width;
    int ker_height = state->ker_height;
    int ker_right = ker_width - ker_x - ((width & 1) == 0);
    char *ker_data = (char *) (state->ker1 + ker_width);

    int crows = state->crows;
    int **rows = (int **) (state->rows);
    int *tbuf = (int *) (state->tbuf);

    int channels = state->channels;
    int ker_x_n = ker_x * channels;
    int ker_width_n = ker_width * channels;
    int width_n = width * channels;

    int starting_flag = 0;
    int width_rest = width_n & (CV_MORPH_ALIGN - 1);

    /* initialize cyclic buffer when starting */
    if( stage == CV_WHOLE || stage == CV_START )
    {
        for( i = 0; i < ker_height; i++ )
        {
            rows[i] = (int *) (state->buffer + state->buffer_step * i);
        }
        crows = ker_y;
        if( stage != CV_WHOLE )
            dst_height -= ker_height - ker_y - 1;
        starting_flag = 1;
    }

    if( stage == CV_END )
        dst_height += ker_height - ker_y - 1;

    srcStep /= sizeof_float;
    dstStep /= sizeof_float;

    do
    {
        int *tsrc, *tdst;
        int need_copy = 0;

        /* fill cyclic buffer - horizontal filtering */
        for( ; crows < ker_height; crows++ )
        {
            tsrc = (int *) src;
            tdst = rows[crows];

            if( src_height-- <= 0 )
            {
                if( stage != CV_WHOLE && stage != CV_END )
                    break;
                /* duplicate last row */
                tsrc = rows[crows - 1];
                CV_COPY( tdst, tsrc, width_n + ker_width_n, x );
                continue;
            }

            src += srcStep;

            for( x = 0; x < width_n; x++ )
            {
                int t = tsrc[x];

                tdst[ker_x_n + x] = (int) CV_TOGGLE_FLT( t );
            }

            /* make replication borders */
            if( channels == 1 )
            {
                int pix = tdst[ker_x];

                CV_SET( tdst, pix, ker_x, x );

                pix = tdst[width + ker_x - 1];
                CV_SET( tdst + width + ker_x, pix, ker_right, x );
            }
            else if( channels == 3 )
            {
                CvRGB32s pix = ((CvRGB32s *) tdst)[ker_x];

                CV_SET( (CvRGB32s *) tdst, pix, ker_x, x );

                pix = ((CvRGB32s *) tdst)[width + ker_x - 1];
                CV_SET( (CvRGB32s *) tdst + width + ker_x, pix, ker_right, x );
            }
            else                /* channels == 4 */
            {
                /* make replication borders */
                CvRGBA32s pix = ((CvRGBA32s *) tdst)[ker_x];

                CV_SET( (CvRGBA32s *) tdst, pix, ker_x, x );

                pix = ((CvRGBA32s *) tdst)[width + ker_x - 1];
                CV_SET( (CvRGBA32s *) tdst + width + ker_x, pix, ker_right, x );
            }
        }

        if( starting_flag )
        {
            starting_flag = 0;
            tsrc = rows[ker_y];

            for( i = 0; i < ker_y; i++ )
            {
                tdst = rows[i];
                CV_COPY( tdst, tsrc, width_n + ker_width_n, x );
            }
        }

        /* vertical convolution */
        if( crows < ker_height )
            break;

        tdst = (int *) dst;
        if( width_rest )
        {
            need_copy = width_n < CV_MORPH_ALIGN || y == dst_height - 1;

            if( need_copy )
                tdst = tbuf;
            else
                CV_COPY( tbuf + width_n, (int *) (dst + width_n), CV_MORPH_ALIGN, x );
        }

        if( channels == 1 )
        {
            for( x = 0; x < width_n; x += 4 )
            {
                int val0 = INIT_VAL, val1 = INIT_VAL, val2 = INIT_VAL, val3 = INIT_VAL;

                char *ker = ker_data;

                for( i = 0; i < ker_height; i++, ker += ker_width )
                {
                    int j = -ker_width;

                    tsrc = rows[i] + x - j;
                    do
                    {
                        int m = ker[j];

                        if( !m )
                        {
                            int t = tsrc[j];

                            CV_CALC_MIN( val0, t );
                            t = tsrc[j + 1];
                            CV_CALC_MIN( val1, t );
                            t = tsrc[j + 2];
                            CV_CALC_MIN( val2, t );
                            t = tsrc[j + 3];
                            CV_CALC_MIN( val3, t );
                        }
                    }
                    while( ++j < 0 );
                }

                tdst[x] = (int) CV_TOGGLE_FLT( val0 );
                tdst[x + 1] = (int) CV_TOGGLE_FLT( val1 );
                tdst[x + 2] = (int) CV_TOGGLE_FLT( val2 );
                tdst[x + 3] = (int) CV_TOGGLE_FLT( val3 );
            }
        }
        else if( channels == 3 )
        {
            for( x = 0; x < width_n; x += 3 )
            {
                int val0 = INIT_VAL, val1 = INIT_VAL, val2 = INIT_VAL;

                char *ker = ker_data;

                for( i = 0; i < ker_height; i++, ker += ker_width )
                {
                    int j = -ker_width;

                    tsrc = rows[i] + x - j * 3;
                    for( ; j < 0; j++ )
                    {
                        int m = ker[j];

                        if( !m )
                        {
                            int t = tsrc[j * 3];

                            CV_CALC_MIN( val0, t );
                            t = tsrc[j * 3 + 1];
                            CV_CALC_MIN( val1, t );
                            t = tsrc[j * 3 + 2];
                            CV_CALC_MIN( val2, t );
                        }
                    }
                }

                tdst[x] = (int) CV_TOGGLE_FLT( val0 );
                tdst[x + 1] = (int) CV_TOGGLE_FLT( val1 );
                tdst[x + 2] = (int) CV_TOGGLE_FLT( val2 );
            }
        }
        else
        {
            for( x = 0; x < width_n; x += 4 )
            {
                int val0 = INIT_VAL, val1 = INIT_VAL, val2 = INIT_VAL, val3 = INIT_VAL;

                char *ker = ker_data;

                for( i = 0; i < ker_height; i++, ker += ker_width )
                {
                    int j = -ker_width;

                    tsrc = rows[i] + x - j * 4;
                    for( ; j < 0; j++ )
                    {
                        int m = ker[j];

                        if( !m )
                        {
                            int t = tsrc[j * 4];

                            CV_CALC_MIN( val0, t );
                            t = tsrc[j * 4 + 1];
                            CV_CALC_MIN( val1, t );
                            t = tsrc[j * 4 + 2];
                            CV_CALC_MIN( val2, t );
                            t = tsrc[j * 4 + 3];
                            CV_CALC_MIN( val3, t );
                        }
                    }
                }

                tdst[x] = (int) CV_TOGGLE_FLT( val0 );
                tdst[x + 1] = (int) CV_TOGGLE_FLT( val1 );
                tdst[x + 2] = (int) CV_TOGGLE_FLT( val2 );
                tdst[x + 3] = (int) CV_TOGGLE_FLT( val3 );
            }
        }

        if( width_rest )
        {
            if( need_copy )
                CV_COPY( (int *) dst, tbuf, width_n, x );
            else
                CV_COPY( (int *) (dst + width_n), tbuf + width_n, CV_MORPH_ALIGN, x );
        }

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
#undef INIT_VAL
}


static CvStatus
icvDilateArb_32f( float *src, int srcStep,
                  float *dst, int dstStep, CvSize * roiSize, CvMorphState * state, int stage )
{
#define INIT_VAL  INT_MIN
    int width = roiSize->width;
    int src_height = roiSize->height;
    int dst_height = src_height;
    int x, y = 0, i;

    int ker_x = state->ker_x;
    int ker_y = state->ker_y;
    int ker_width = state->ker_width;
    int ker_height = state->ker_height;
    int ker_right = ker_width - ker_x - ((width & 1) == 0);
    char *ker_data = (char *) (state->ker0 + ker_width);

    int crows = state->crows;
    int **rows = (int **) (state->rows);
    int *tbuf = (int *) (state->tbuf);

    int channels = state->channels;
    int ker_x_n = ker_x * channels;
    int ker_width_n = ker_width * channels;
    int width_n = width * channels;

    int starting_flag = 0;
    int width_rest = width_n & (CV_MORPH_ALIGN - 1);

    /* initialize cyclic buffer when starting */
    if( stage == CV_WHOLE || stage == CV_START )
    {
        for( i = 0; i < ker_height; i++ )
        {
            rows[i] = (int *) (state->buffer + state->buffer_step * i);
        }
        crows = ker_y;
        if( stage != CV_WHOLE )
            dst_height -= ker_height - ker_y - 1;
        starting_flag = 1;
    }

    if( stage == CV_END )
        dst_height += ker_height - ker_y - 1;

    srcStep /= sizeof_float;
    dstStep /= sizeof_float;

    do
    {
        int *tsrc, *tdst;
        int need_copy = 0;

        /* fill cyclic buffer - horizontal filtering */
        for( ; crows < ker_height; crows++ )
        {
            tsrc = (int *) src;
            tdst = rows[crows];

            if( src_height-- <= 0 )
            {
                if( stage != CV_WHOLE && stage != CV_END )
                    break;
                /* duplicate last row */
                tsrc = rows[crows - 1];
                CV_COPY( tdst, tsrc, width_n + ker_width_n, x );
                continue;
            }

            src += srcStep;

            for( x = 0; x < width_n; x++ )
            {
                int t = tsrc[x];

                tdst[ker_x_n + x] = (int) CV_TOGGLE_FLT( t );
            }

            /* make replication borders */
            if( channels == 1 )
            {
                int pix = tdst[ker_x];

                CV_SET( tdst, pix, ker_x, x );

                pix = tdst[width + ker_x - 1];
                CV_SET( tdst + width + ker_x, pix, ker_right, x );
            }
            else if( channels == 3 )
            {
                CvRGB32s pix = ((CvRGB32s *) tdst)[ker_x];

                CV_SET( (CvRGB32s *) tdst, pix, ker_x, x );

                pix = ((CvRGB32s *) tdst)[width + ker_x - 1];
                CV_SET( (CvRGB32s *) tdst + width + ker_x, pix, ker_right, x );
            }
            else                /* channels == 4 */
            {
                /* make replication borders */
                CvRGBA32s pix = ((CvRGBA32s *) tdst)[ker_x];

                CV_SET( (CvRGBA32s *) tdst, pix, ker_x, x );

                pix = ((CvRGBA32s *) tdst)[width + ker_x - 1];
                CV_SET( (CvRGBA32s *) tdst + width + ker_x, pix, ker_right, x );
            }
        }

        if( starting_flag )
        {
            starting_flag = 0;
            tsrc = rows[ker_y];

            for( i = 0; i < ker_y; i++ )
            {
                tdst = rows[i];
                CV_COPY( tdst, tsrc, width_n + ker_width_n, x );
            }
        }

        /* vertical convolution */
        if( crows < ker_height )
            break;

        tdst = (int *) dst;
        if( width_rest )
        {
            need_copy = width_n < CV_MORPH_ALIGN || y == dst_height - 1;

            if( need_copy )
                tdst = tbuf;
            else
                CV_COPY( tbuf + width_n, (int *) (dst + width_n), CV_MORPH_ALIGN, x );
        }

        if( channels == 1 )
        {
            for( x = 0; x < width_n; x += 4 )
            {
                int val0 = INIT_VAL, val1 = INIT_VAL, val2 = INIT_VAL, val3 = INIT_VAL;

                char *ker = ker_data;

                for( i = 0; i < ker_height; i++, ker += ker_width )
                {
                    int j = -ker_width;

                    tsrc = rows[i] + x - j;
                    do
                    {
                        int m = ker[j];

                        if( m )
                        {
                            int t = tsrc[j];

                            CV_CALC_MAX( val0, t );
                            t = tsrc[j + 1];
                            CV_CALC_MAX( val1, t );
                            t = tsrc[j + 2];
                            CV_CALC_MAX( val2, t );
                            t = tsrc[j + 3];
                            CV_CALC_MAX( val3, t );
                        }
                    }
                    while( ++j < 0 );
                }

                tdst[x] = (int) CV_TOGGLE_FLT( val0 );
                tdst[x + 1] = (int) CV_TOGGLE_FLT( val1 );
                tdst[x + 2] = (int) CV_TOGGLE_FLT( val2 );
                tdst[x + 3] = (int) CV_TOGGLE_FLT( val3 );
            }
        }
        else if( channels == 3 )
        {
            for( x = 0; x < width_n; x += 3 )
            {
                int val0 = INIT_VAL, val1 = INIT_VAL, val2 = INIT_VAL;

                char *ker = ker_data;

                for( i = 0; i < ker_height; i++, ker += ker_width )
                {
                    int j = -ker_width;

                    tsrc = rows[i] + x - j * 3;
                    for( ; j < 0; j++ )
                    {
                        int m = ker[j];

                        if( m )
                        {
                            int t = tsrc[j * 3];

                            CV_CALC_MAX( val0, t );
                            t = tsrc[j * 3 + 1];
                            CV_CALC_MAX( val1, t );
                            t = tsrc[j * 3 + 2];
                            CV_CALC_MAX( val2, t );
                        }
                    }
                }

                tdst[x] = (int) CV_TOGGLE_FLT( val0 );
                tdst[x + 1] = (int) CV_TOGGLE_FLT( val1 );
                tdst[x + 2] = (int) CV_TOGGLE_FLT( val2 );
            }
        }
        else
        {
            for( x = 0; x < width_n; x += 4 )
            {
                int val0 = INIT_VAL, val1 = INIT_VAL, val2 = INIT_VAL, val3 = INIT_VAL;

                char *ker = ker_data;

                for( i = 0; i < ker_height; i++, ker += ker_width )
                {
                    int j = -ker_width;

                    tsrc = rows[i] + x - j * 4;
                    for( ; j < 0; j++ )
                    {
                        int m = ker[j];

                        if( m )
                        {
                            int t = tsrc[j * 4];

                            CV_CALC_MAX( val0, t );
                            t = tsrc[j * 4 + 1];
                            CV_CALC_MAX( val1, t );
                            t = tsrc[j * 4 + 2];
                            CV_CALC_MAX( val2, t );
                            t = tsrc[j * 4 + 3];
                            CV_CALC_MAX( val3, t );
                        }
                    }
                }

                tdst[x] = (int) CV_TOGGLE_FLT( val0 );
                tdst[x + 1] = (int) CV_TOGGLE_FLT( val1 );
                tdst[x + 2] = (int) CV_TOGGLE_FLT( val2 );
                tdst[x + 3] = (int) CV_TOGGLE_FLT( val3 );
            }
        }

        if( width_rest )
        {
            if( need_copy )
                CV_COPY( (int *) dst, tbuf, width_n, x );
            else
                CV_COPY( (int *) (dst + width_n), tbuf + width_n, CV_MORPH_ALIGN, x );
        }

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
#undef INIT_VAL
}



static CvStatus
icvDilateRC_8u( uchar * src, int srcStep,
                uchar * dst, int dstStep, CvSize * roiSize, CvMorphState * state, int stage )
{
    int width = roiSize->width;
    int src_height = roiSize->height;
    int dst_height = src_height;
    int x, y = 0, i;

    int ker_x = state->ker_x;
    int ker_y = state->ker_y;
    int ker_width = state->ker_width;
    int ker_height = state->ker_height;
    int ker_right = ker_width - ker_x - ((width & 1) == 0);

    int crows = state->crows;
    uchar **rows = (uchar **) (state->rows);
    uchar *tbuf = (uchar *) (state->tbuf);

    int channels = state->channels;
    int ker_x_n = ker_x * channels;
    int ker_width_n = ker_width * channels;
    int ker_right_n = ker_right * channels;
    int width_n = width * channels;

    int is_small_width = width < MAX( ker_x, ker_right );
    int starting_flag = 0;
    int width_rest = width_n & (CV_MORPH_ALIGN - 1);

    int is_cross = ICV_BINARY_KERNEL_SHAPE(state->kerType) == CV_SHAPE_CROSS;

    /* initialize cyclic buffer when starting */
    if( stage == CV_WHOLE || stage == CV_START )
    {
        for( i = 0; i < ker_height; i++ )
        {
            rows[i] = (uchar *) (state->buffer + state->buffer_step * i);
        }
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
        uchar *tsrc, *tdst;
        uchar *saved_row = rows[ker_y];

        /* fill cyclic buffer - horizontal filtering */
        for( ; crows < ker_height + is_cross; crows++ )
        {
            if( crows < ker_height )
            {
                tsrc = src - ker_x_n;
                tdst = rows[crows];

                if( src_height-- <= 0 )
                {
                    if( stage != CV_WHOLE && stage != CV_END )
                        break;
                    /* duplicate last row */
                    tsrc = rows[crows - 1];
                    CV_COPY( tdst, tsrc, width_n, x );
                    continue;
                }

                need_copy |= src_height == 1;
            }
            else
            {
                /* convolve center line for cross-shaped element */
                tsrc = rows[ker_y] - ker_x_n;
                tdst = tbuf;

                need_copy = 0;
            }

            if( ker_width > 1 && (!is_cross || crows == ker_height) )
            {
                if( need_copy )
                {
                    tsrc = tbuf - ker_x_n;
                    CV_COPY( tbuf, src, width_n, x );
                }
                else if( !is_cross )
                {
                    CV_COPY( tbuf - ker_x_n, src - ker_x_n, ker_x_n, x );
                    CV_COPY( tbuf, src + width_n, ker_right_n, x );
                }

                if( channels == 1 )
                {
                    /* make replication borders */
                    uchar pix = tsrc[ker_x];

                    CV_SET( tsrc, pix, ker_x, x );

                    pix = tsrc[width + ker_x - 1];
                    CV_SET( tsrc + width + ker_x, pix, ker_right, x );

                    /* horizontal convolution loop */
                    for( i = 0; i < width_n; i += 2 )
                    {
                        int j;
                        int t, t0 = tsrc[i + 1];

                        for( j = 2; j < ker_width_n; j++ )
                        {
                            int t1 = tsrc[i + j];

                            CV_CALC_MAX( t0, t1 );
                        }

                        t = tsrc[i];
                        CV_CALC_MAX( t, t0 );
                        tdst[i] = (uchar) t;

                        t = tsrc[i + j];
                        CV_CALC_MAX( t, t0 );
                        tdst[i + 1] = (uchar) t;
                    }
                }
                else if( channels == 3 )
                {
                    /* make replication borders */
                    CvRGB8u pix = ((CvRGB8u *) tsrc)[ker_x];

                    CV_SET( (CvRGB8u *) tsrc, pix, ker_x, x );

                    pix = ((CvRGB8u *) tsrc)[width + ker_x - 1];
                    CV_SET( (CvRGB8u *) tsrc + width + ker_x, pix, ker_right, x );

                    /* horizontal convolution loop */
                    for( i = 0; i < width_n; i++ )
                    {
                        int j;
                        int t0 = tsrc[i];

                        for( j = 3; j < ker_width_n; j += 3 )
                        {
                            int t1 = tsrc[i + j];

                            CV_CALC_MAX( t0, t1 );
                        }

                        tdst[i] = (uchar) t0;
                    }
                }
                else            /* channels == 4 */
                {
                    /* make replication borders */
                    CvRGBA8u pix = ((CvRGBA8u *) tsrc)[ker_x];

                    CV_SET( (CvRGBA8u *) tsrc, pix, ker_x, x );

                    pix = ((CvRGBA8u *) tsrc)[width + ker_x - 1];
                    CV_SET( (CvRGBA8u *) tsrc + width + ker_x, pix, ker_right, x );

                    /* horizontal convolution loop */
                    for( i = 0; i < width_n; i++ )
                    {
                        int j;
                        int t0 = tsrc[i];

                        for( j = 4; j < ker_width_n; j += 4 )
                        {
                            int t1 = tsrc[i + j];

                            CV_CALC_MAX( t0, t1 );
                        }

                        tdst[i] = (uchar) t0;
                    }
                }

                if( !need_copy && !is_cross )
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
            tsrc = rows[ker_y];

            for( i = 0; i < ker_y; i++ )
            {
                tdst = rows[i];
                CV_COPY( tdst, tsrc, width_n, x );
            }
        }

        /* vertical convolution */
        if( crows != ker_height )
        {
            if( crows < ker_height )
                break;
            /* else it is cross-shaped element: change central line */
            rows[ker_y] = tbuf;
            crows--;
        }

        tdst = dst;

        if( width_rest )
        {
            need_copy = width_n < CV_MORPH_ALIGN || y == dst_height - 1;

            if( need_copy )
                tdst = tbuf;
            else
                CV_COPY( tbuf + width_n, dst + width_n, CV_MORPH_ALIGN, x );
        }

        for( x = 0; x < width_n; x += 4 )
        {
            int val0, val1, val2, val3;

            tsrc = rows[0];

            val0 = tsrc[x];
            val1 = tsrc[x + 1];
            val2 = tsrc[x + 2];
            val3 = tsrc[x + 3];

            for( i = 1; i < ker_height; i++ )
            {
                int s;

                tsrc = rows[i];

                s = tsrc[x + 0];
                CV_CALC_MAX( val0, s );
                s = tsrc[x + 1];
                CV_CALC_MAX( val1, s );
                s = tsrc[x + 2];
                CV_CALC_MAX( val2, s );
                s = tsrc[x + 3];
                CV_CALC_MAX( val3, s );
            }

            tdst[x + 0] = (uchar) val0;
            tdst[x + 1] = (uchar) val1;
            tdst[x + 2] = (uchar) val2;
            tdst[x + 3] = (uchar) val3;
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
            uchar *t = rows[0];

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


static CvStatus
icvDilateArb_8u( uchar * src, int srcStep,
                 uchar * dst, int dstStep, CvSize * roiSize, CvMorphState * state, int stage )
{
#define INIT_VAL 0
    int width = roiSize->width;
    int src_height = roiSize->height;
    int dst_height = src_height;
    int x, y = 0, i;

    int ker_x = state->ker_x;
    int ker_y = state->ker_y;
    int ker_width = state->ker_width;
    int ker_height = state->ker_height;
    int ker_right = ker_width - ker_x - ((width & 1) == 0);
    uchar *ker_data = state->ker0 + ker_width;

    int crows = state->crows;
    uchar **rows = (uchar **) (state->rows);
    uchar *tbuf = (uchar *) (state->tbuf);

    int channels = state->channels;
    int ker_x_n = ker_x * channels;
    int ker_width_n = ker_width * channels;
    int width_n = width * channels;

    int starting_flag = 0;
    int width_rest = width_n & (CV_MORPH_ALIGN - 1);

    /* initialize cyclic buffer when starting */
    if( stage == CV_WHOLE || stage == CV_START )
    {
        for( i = 0; i < ker_height; i++ )
        {
            rows[i] = (uchar *) (state->buffer + state->buffer_step * i);
        }
        crows = ker_y;
        if( stage != CV_WHOLE )
            dst_height -= ker_height - ker_y - 1;
        starting_flag = 1;
    }

    if( stage == CV_END )
        dst_height += ker_height - ker_y - 1;

    do
    {
        uchar *tsrc, *tdst;
        int need_copy = 0;

        /* fill cyclic buffer - horizontal filtering */
        for( ; crows < ker_height; crows++ )
        {
            tsrc = src;
            tdst = rows[crows];

            if( src_height-- <= 0 )
            {
                if( stage != CV_WHOLE && stage != CV_END )
                    break;
                /* duplicate last row */
                tsrc = rows[crows - 1];
                CV_COPY( tdst, tsrc, width_n + ker_width_n, x );
                continue;
            }

            src += srcStep;

            CV_COPY( tdst + ker_x_n, tsrc, width_n, x );

            /* make replication borders */
            if( channels == 1 )
            {
                uchar pix = tdst[ker_x];

                CV_SET( tdst, pix, ker_x, x );

                pix = tdst[width + ker_x - 1];
                CV_SET( tdst + width + ker_x, pix, ker_right, x );
            }
            else if( channels == 3 )
            {
                CvRGB8u pix = ((CvRGB8u *) tdst)[ker_x];

                CV_SET( (CvRGB8u *) tdst, pix, ker_x, x );

                pix = ((CvRGB8u *) tdst)[width + ker_x - 1];
                CV_SET( (CvRGB8u *) tdst + width + ker_x, pix, ker_right, x );
            }
            else                /* channels == 4 */
            {
                /* make replication borders */
                CvRGBA8u pix = ((CvRGBA8u *) tdst)[ker_x];

                CV_SET( (CvRGBA8u *) tdst, pix, ker_x, x );

                pix = ((CvRGBA8u *) tdst)[width + ker_x - 1];
                CV_SET( (CvRGBA8u *) tdst + width + ker_x, pix, ker_right, x );
            }
        }

        if( starting_flag )
        {
            starting_flag = 0;
            tsrc = rows[ker_y];

            for( i = 0; i < ker_y; i++ )
            {
                tdst = rows[i];
                CV_COPY( tdst, tsrc, width_n + ker_width_n, x );
            }
        }

        /* vertical convolution */
        if( crows < ker_height )
            break;

        tdst = dst;
        if( width_rest )
        {
            need_copy = width_n < CV_MORPH_ALIGN || y == dst_height - 1;

            if( need_copy )
                tdst = tbuf;
            else
                CV_COPY( tbuf + width_n, dst + width_n, CV_MORPH_ALIGN, x );
        }

        if( channels == 1 )
        {
            for( x = 0; x < width_n; x += 4 )
            {
                int val0 = INIT_VAL, val1 = INIT_VAL, val2 = INIT_VAL, val3 = INIT_VAL;

                uchar *ker = ker_data;

                for( i = 0; i < ker_height; i++, ker += ker_width )
                {
                    int j = -ker_width;

                    tsrc = rows[i] + x - j;
                    do
                    {
                        int m = ker[j];
                        int t = tsrc[j] & m;

                        CV_CALC_MAX( val0, t );
                        t = tsrc[j + 1] & m;
                        CV_CALC_MAX( val1, t );
                        t = tsrc[j + 2] & m;
                        CV_CALC_MAX( val2, t );
                        t = tsrc[j + 3] & m;
                        CV_CALC_MAX( val3, t );
                    }
                    while( ++j < 0 );
                }

                tdst[x] = (uchar) val0;
                tdst[x + 1] = (uchar) val1;
                tdst[x + 2] = (uchar) val2;
                tdst[x + 3] = (uchar) val3;
            }
        }
        else if( channels == 3 )
        {
            for( x = 0; x < width_n; x += 3 )
            {
                int val0 = INIT_VAL, val1 = INIT_VAL, val2 = INIT_VAL;

                uchar *ker = ker_data;

                for( i = 0; i < ker_height; i++, ker += ker_width )
                {
                    int j = -ker_width;

                    tsrc = rows[i] + x - j * 3;
                    for( ; j < 0; j++ )
                    {
                        int m = ker[j];
                        int t = tsrc[j * 3] & m;

                        CV_CALC_MAX( val0, t );
                        t = tsrc[j * 3 + 1] & m;
                        CV_CALC_MAX( val1, t );
                        t = tsrc[j * 3 + 2] & m;
                        CV_CALC_MAX( val2, t );
                    }
                }

                tdst[x] = (uchar) val0;
                tdst[x + 1] = (uchar) val1;
                tdst[x + 2] = (uchar) val2;
            }
        }
        else
        {
            for( x = 0; x < width_n; x += 4 )
            {
                int val0 = INIT_VAL, val1 = INIT_VAL, val2 = INIT_VAL, val3 = INIT_VAL;

                uchar *ker = ker_data;

                for( i = 0; i < ker_height; i++, ker += ker_width )
                {
                    int j = -ker_width;

                    tsrc = rows[i] + x - j * 4;
                    for( ; j < 0; j++ )
                    {
                        int m = ker[j];
                        int t = tsrc[j * 4] & m;

                        CV_CALC_MAX( val0, t );
                        t = tsrc[j * 4 + 1] & m;
                        CV_CALC_MAX( val1, t );
                        t = tsrc[j * 4 + 2] & m;
                        CV_CALC_MAX( val2, t );
                        t = tsrc[j * 4 + 3] & m;
                        CV_CALC_MAX( val3, t );
                    }
                }

                tdst[x] = (uchar) val0;
                tdst[x + 1] = (uchar) val1;
                tdst[x + 2] = (uchar) val2;
                tdst[x + 3] = (uchar) val3;
            }
        }

        if( width_rest )
        {
            if( need_copy )
                CV_COPY( dst, tbuf, width_n, x );
            else
                CV_COPY( dst + width_n, tbuf + width_n, CV_MORPH_ALIGN, x );
        }

        /* rotate buffer */
        {
            uchar *t = rows[0];

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
#undef INIT_VAL
}



static CvStatus
icvDilateRC_32f( float *src, int srcStep,
                 float *dst, int dstStep, CvSize * roiSize, CvMorphState * state, int stage )
{
    int width = roiSize->width;
    int src_height = roiSize->height;
    int dst_height = src_height;
    int x, y = 0, i;

    int ker_x = state->ker_x;
    int ker_y = state->ker_y;
    int ker_width = state->ker_width;
    int ker_height = state->ker_height;
    int ker_right = ker_width - ker_x - ((width & 1) == 0);

    int crows = state->crows;
    int **rows = (int **) (state->rows);
    int *tbuf = (int *) (state->tbuf);
    int *tbuf2 = (int *) (state->tbuf + state->buffer_step);

    int channels = state->channels;
    int ker_x_n = ker_x * channels;
    int ker_width_n = ker_width * channels;
    int width_n = width * channels;

    int starting_flag = 0;
    int width_rest = width_n & (CV_MORPH_ALIGN - 1);

    int is_cross = ICV_BINARY_KERNEL_SHAPE(state->kerType) == CV_SHAPE_CROSS;

    /* initialize cyclic buffer when starting */
    if( stage == CV_WHOLE || stage == CV_START )
    {
        for( i = 0; i < ker_height; i++ )
        {
            rows[i] = (int *) (state->buffer + state->buffer_step * i);
        }
        crows = ker_y;
        if( stage != CV_WHOLE )
            dst_height -= ker_height - ker_y - 1;
        starting_flag = 1;
    }

    if( stage == CV_END )
        dst_height += ker_height - ker_y - 1;

    srcStep /= sizeof_float;
    dstStep /= sizeof_float;

    do
    {
        int need_copy = 0;
        int *tsrc, *tdst;
        int *saved_row = rows[ker_y];

        /* fill cyclic buffer - horizontal filtering */
        for( ; crows < ker_height + is_cross; crows++ )
        {
            if( crows < ker_height )
            {
                tsrc = (int *) src;
                tdst = rows[crows];

                if( src_height-- <= 0 )
                {
                    if( stage != CV_WHOLE && stage != CV_END )
                        break;
                    /* duplicate last row */
                    tsrc = rows[crows - 1];
                    CV_COPY( tdst, tsrc, width_n, x );
                    continue;
                }

                for( x = 0; x < width_n; x++ )
                {
                    int t = tsrc[x];

                    tbuf2[x] = CV_TOGGLE_FLT( t );
                }

                tsrc = tbuf2 - ker_x_n;
            }
            else
            {
                /* convolve center line for cross-shaped element */
                tsrc = rows[ker_y] - ker_x_n;
                tdst = tbuf;
            }

            if( ker_width > 1 && (!is_cross || crows == ker_height) )
            {
                if( channels == 1 )
                {
                    /* make replication borders */
                    int pix = tsrc[ker_x];

                    CV_SET( tsrc, pix, ker_x, x );

                    pix = tsrc[width + ker_x - 1];
                    CV_SET( tsrc + width + ker_x, pix, ker_right, x );

                    /* horizontal convolution loop */
                    for( i = 0; i < width_n; i += 2 )
                    {
                        int j;
                        int t, t0 = tsrc[i + 1];

                        for( j = 2; j < ker_width_n; j++ )
                        {
                            int t1 = tsrc[i + j];

                            CV_CALC_MAX( t0, t1 );
                        }

                        t = tsrc[i];
                        CV_CALC_MAX( t, t0 );
                        tdst[i] = (int) t;

                        t = tsrc[i + j];
                        CV_CALC_MAX( t, t0 );
                        tdst[i + 1] = (int) t;
                    }
                }
                else if( channels == 3 )
                {
                    /* make replication borders */
                    CvRGB32s pix = ((CvRGB32s *) tsrc)[ker_x];

                    CV_SET( (CvRGB32s *) tsrc, pix, ker_x, x );

                    pix = ((CvRGB32s *) tsrc)[width + ker_x - 1];
                    CV_SET( (CvRGB32s *) tsrc + width + ker_x, pix, ker_right, x );

                    /* horizontal convolution loop */
                    for( i = 0; i < width_n; i++ )
                    {
                        int j;
                        int t0 = tsrc[i];

                        for( j = 3; j < ker_width_n; j += 3 )
                        {
                            int t1 = tsrc[i + j];

                            CV_CALC_MAX( t0, t1 );
                        }

                        tdst[i] = (int) t0;
                    }
                }
                else            /* channels == 4 */
                {
                    /* make replication borders */
                    CvRGBA32s pix = ((CvRGBA32s *) tsrc)[ker_x];

                    CV_SET( (CvRGBA32s *) tsrc, pix, ker_x, x );

                    pix = ((CvRGBA32s *) tsrc)[width + ker_x - 1];
                    CV_SET( (CvRGBA32s *) tsrc + width + ker_x, pix, ker_right, x );

                    /* horizontal convolution loop */
                    for( i = 0; i < width_n; i++ )
                    {
                        int j;
                        int t0 = tsrc[i];

                        for( j = 4; j < ker_width_n; j += 4 )
                        {
                            int t1 = tsrc[i + j];

                            CV_CALC_MAX( t0, t1 );
                        }

                        tdst[i] = (int) t0;
                    }
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
            tsrc = rows[ker_y];

            for( i = 0; i < ker_y; i++ )
            {
                tdst = rows[i];
                CV_COPY( tdst, tsrc, width_n, x );
            }
        }

        /* vertical convolution */
        if( crows != ker_height )
        {
            if( crows < ker_height )
                break;
            /* else it is cross-shaped element: change central line */
            rows[ker_y] = tbuf;
            crows--;
        }

        tdst = (int *) dst;

        if( width_rest )
        {
            need_copy = width_n < CV_MORPH_ALIGN || y == dst_height - 1;

            if( need_copy )
                tdst = tbuf;
            else
                CV_COPY( tbuf + width_n, tdst + width_n, CV_MORPH_ALIGN, x );
        }

        for( x = 0; x < width_n; x += 4 )
        {
            int val0, val1, val2, val3;

            tsrc = rows[0];

            val0 = tsrc[x];
            val1 = tsrc[x + 1];
            val2 = tsrc[x + 2];
            val3 = tsrc[x + 3];

            for( i = 1; i < ker_height; i++ )
            {
                int s;

                tsrc = rows[i];

                s = tsrc[x + 0];
                CV_CALC_MAX( val0, s );
                s = tsrc[x + 1];
                CV_CALC_MAX( val1, s );
                s = tsrc[x + 2];
                CV_CALC_MAX( val2, s );
                s = tsrc[x + 3];
                CV_CALC_MAX( val3, s );
            }
            tdst[x + 0] = CV_TOGGLE_FLT( val0 );
            tdst[x + 1] = CV_TOGGLE_FLT( val1 );
            tdst[x + 2] = CV_TOGGLE_FLT( val2 );
            tdst[x + 3] = CV_TOGGLE_FLT( val3 );
        }

        if( width_rest )
        {
            if( need_copy )
                CV_COPY( (int *) dst, tbuf, width_n, x );
            else
                CV_COPY( tdst + width_n, tbuf + width_n, CV_MORPH_ALIGN, x );
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



static CvStatus
icvCheckMorphArgs( const void *pSrc, int srcStep,
                   void *pDst, int dstStep,
                   CvSize * roiSize,
                   CvMorphState * state,
                   int stage, CvElementShape shape, CvDataType dataType, int channels )
{
    int bt_pix = channels * (dataType != cv32f ? 1 : 4);

    if( !pSrc || !pDst || !state || !roiSize )
        return CV_NULLPTR_ERR;

    if( roiSize->width <= 0 || roiSize->width > state->max_width || roiSize->height < 0 )
        return CV_BADSIZE_ERR;

    if( state->dataType != dataType || state->channels != channels )
        return CV_UNMATCHED_FORMATS_ERR;

    if( ICV_BINARY_KERNEL_SHAPE(state->kerType) != shape )
        return CV_UNMATCHED_FORMATS_ERR;

    if( roiSize->width * bt_pix > srcStep || roiSize->width * bt_pix > dstStep )
        return CV_BADSIZE_ERR;

    if( stage != CV_WHOLE && stage != CV_MIDDLE && stage != CV_START && stage != CV_END )
        return CV_BADRANGE_ERR;

    if( state->crows == 0 && stage > CV_START || roiSize->height == 0 && stage != CV_END )
    {
        roiSize->height = 0;
        return ( CvStatus ) 1;
    }

    return CV_OK;
}


/****************************************************************************************\
                              Internal IPP-like functions
\****************************************************************************************/

#define ICV_DEF_MORPH_FUNC( extname, intname, flavor, cn, arrtype, shape )          \
IPCVAPI_IMPL( CvStatus,                                                             \
icv##extname##_##flavor##_C##cn##R,( const arrtype* pSrc, int srcStep,              \
                                     arrtype* pDst, int dstStep,                    \
                                     CvSize* roiSize,                               \
                                     CvMorphState * state, int stage ))             \
{                                                                                   \
    CvStatus status = icvCheckMorphArgs( pSrc, srcStep, pDst, dstStep, roiSize,     \
                                         state, stage, shape, cv##flavor, cn );     \
                                                                                    \
    if( status == CV_OK )                                                           \
    {                                                                               \
        status = icv##intname##_##flavor( (arrtype*)pSrc, srcStep, pDst, dstStep,   \
                                          roiSize, state, stage );                  \
    }                                                                               \
                                                                                    \
    return status >= 0 ? CV_OK : status;                                            \
}


ICV_DEF_MORPH_FUNC( ErodeStrip_Rect, ErodeRC, 8u, 1, uchar, CV_SHAPE_RECT )
ICV_DEF_MORPH_FUNC( ErodeStrip_Rect, ErodeRC, 8u, 3, uchar, CV_SHAPE_RECT )
ICV_DEF_MORPH_FUNC( ErodeStrip_Rect, ErodeRC, 8u, 4, uchar, CV_SHAPE_RECT )

ICV_DEF_MORPH_FUNC( ErodeStrip_Cross, ErodeRC, 8u, 1, uchar, CV_SHAPE_CROSS )
ICV_DEF_MORPH_FUNC( ErodeStrip_Cross, ErodeRC, 8u, 3, uchar, CV_SHAPE_CROSS )
ICV_DEF_MORPH_FUNC( ErodeStrip_Cross, ErodeRC, 8u, 4, uchar, CV_SHAPE_CROSS )

ICV_DEF_MORPH_FUNC( ErodeStrip, ErodeArb, 8u, 1, uchar, CV_SHAPE_CUSTOM )
ICV_DEF_MORPH_FUNC( ErodeStrip, ErodeArb, 8u, 3, uchar, CV_SHAPE_CUSTOM )
ICV_DEF_MORPH_FUNC( ErodeStrip, ErodeArb, 8u, 4, uchar, CV_SHAPE_CUSTOM )

ICV_DEF_MORPH_FUNC( ErodeStrip_Rect, ErodeRC, 32f, 1, float, CV_SHAPE_RECT )
ICV_DEF_MORPH_FUNC( ErodeStrip_Rect, ErodeRC, 32f, 3, float, CV_SHAPE_RECT )
ICV_DEF_MORPH_FUNC( ErodeStrip_Rect, ErodeRC, 32f, 4, float, CV_SHAPE_RECT )

ICV_DEF_MORPH_FUNC( ErodeStrip_Cross, ErodeRC, 32f, 1, float, CV_SHAPE_CROSS )
ICV_DEF_MORPH_FUNC( ErodeStrip_Cross, ErodeRC, 32f, 3, float, CV_SHAPE_CROSS )
ICV_DEF_MORPH_FUNC( ErodeStrip_Cross, ErodeRC, 32f, 4, float, CV_SHAPE_CROSS )

ICV_DEF_MORPH_FUNC( ErodeStrip, ErodeArb, 32f, 1, float, CV_SHAPE_CUSTOM )
ICV_DEF_MORPH_FUNC( ErodeStrip, ErodeArb, 32f, 3, float, CV_SHAPE_CUSTOM )
ICV_DEF_MORPH_FUNC( ErodeStrip, ErodeArb, 32f, 4, float, CV_SHAPE_CUSTOM )

ICV_DEF_MORPH_FUNC( DilateStrip_Rect, DilateRC, 8u, 1, uchar, CV_SHAPE_RECT )
ICV_DEF_MORPH_FUNC( DilateStrip_Rect, DilateRC, 8u, 3, uchar, CV_SHAPE_RECT )
ICV_DEF_MORPH_FUNC( DilateStrip_Rect, DilateRC, 8u, 4, uchar, CV_SHAPE_RECT )

ICV_DEF_MORPH_FUNC( DilateStrip_Cross, DilateRC, 8u, 1, uchar, CV_SHAPE_CROSS )
ICV_DEF_MORPH_FUNC( DilateStrip_Cross, DilateRC, 8u, 3, uchar, CV_SHAPE_CROSS )
ICV_DEF_MORPH_FUNC( DilateStrip_Cross, DilateRC, 8u, 4, uchar, CV_SHAPE_CROSS )

ICV_DEF_MORPH_FUNC( DilateStrip, DilateArb, 8u, 1, uchar, CV_SHAPE_CUSTOM )
ICV_DEF_MORPH_FUNC( DilateStrip, DilateArb, 8u, 3, uchar, CV_SHAPE_CUSTOM )
ICV_DEF_MORPH_FUNC( DilateStrip, DilateArb, 8u, 4, uchar, CV_SHAPE_CUSTOM )

ICV_DEF_MORPH_FUNC( DilateStrip_Rect, DilateRC, 32f, 1, float, CV_SHAPE_RECT )
ICV_DEF_MORPH_FUNC( DilateStrip_Rect, DilateRC, 32f, 3, float, CV_SHAPE_RECT )
ICV_DEF_MORPH_FUNC( DilateStrip_Rect, DilateRC, 32f, 4, float, CV_SHAPE_RECT )

ICV_DEF_MORPH_FUNC( DilateStrip_Cross, DilateRC, 32f, 1, float, CV_SHAPE_CROSS )
ICV_DEF_MORPH_FUNC( DilateStrip_Cross, DilateRC, 32f, 3, float, CV_SHAPE_CROSS )
ICV_DEF_MORPH_FUNC( DilateStrip_Cross, DilateRC, 32f, 4, float, CV_SHAPE_CROSS )

ICV_DEF_MORPH_FUNC( DilateStrip, DilateArb, 32f, 1, float, CV_SHAPE_CUSTOM )
ICV_DEF_MORPH_FUNC( DilateStrip, DilateArb, 32f, 3, float, CV_SHAPE_CUSTOM )
ICV_DEF_MORPH_FUNC( DilateStrip, DilateArb, 32f, 4, float, CV_SHAPE_CUSTOM )



/////////////////////////////////// External Interface /////////////////////////////////////


CV_IMPL IplConvKernel *
cvCreateStructuringElementEx( int cols, int rows,
                              int anchorX, int anchorY,
                              CvElementShape shape, int *values )
{
    IplConvKernel *element = 0;
    int i, size = rows * cols;
    int *vals;
    int element_size = sizeof( *element ) + size * sizeof( element->values[0] );

    CV_FUNCNAME( "cvCreateStructuringElementEx" );

    __BEGIN__;

    if( !values && shape == CV_SHAPE_CUSTOM )
        CV_ERROR_FROM_STATUS( CV_NULLPTR_ERR );

    if( cols <= 0 || rows <= 0 ||
        (unsigned) anchorX >= (unsigned) cols || (unsigned) anchorY >= (unsigned) rows )
        CV_ERROR_FROM_STATUS( CV_BADSIZE_ERR );

    element_size = icvAlign(element_size,32);
    element = (IplConvKernel *) icvAlloc( element_size );
    if( !element )
        CV_ERROR_FROM_STATUS( CV_OUTOFMEM_ERR );

    element->nCols = cols;
    element->nRows = rows;
    element->anchorX = anchorX;
    element->anchorY = anchorY;
    element->nShiftR = shape < CV_SHAPE_ELLIPSE ? shape : CV_SHAPE_CUSTOM;
    element->values = vals = (int *) (element + 1);

    switch (shape)
    {
    case CV_SHAPE_RECT:
        memset( vals, -1, sizeof( vals[0] ) * cols * rows );
        break;
    case CV_SHAPE_CROSS:
        memset( vals, 0, sizeof( vals[0] ) * cols * rows );
        for( i = 0; i < cols; i++ )
        {
            vals[i + anchorY * cols] = -1;
        }

        for( i = 0; i < rows; i++ )
        {
            vals[anchorX + i * cols] = -1;
        }
        break;
    case CV_SHAPE_ELLIPSE:
        {
            int r = (rows + 1) / 2;
            int c = cols / 2;
            double inv_r2 = 1. / (((double) (r)) * (r));

            memset( vals, 0, sizeof( vals[0] ) * cols * rows );

            for( i = 0; i < r; i++ )
            {
                int y = r - i;
                int dx = cvRound( c * sqrt( (r * r - y * y) * inv_r2 ));
                int x1 = c - dx;
                int x2 = c + dx;

                if( x1 < 0 )
                    x1 = 0;
                if( x2 >= cols )
                    x2 = cols;
                x2 -= x1 - 1;
                memset( vals + i * cols + x1, -1, x2 * sizeof( int ));
                memset( vals + (rows - i - 1) * cols + x1, -1, x2 * sizeof( int ));
            }
        }
        break;
    case CV_SHAPE_CUSTOM:
        for( i = 0; i < size; i++ )
        {
            vals[i] = !values || values[i] ? -1 : 0;
        }
        break;
    default:
        icvFree( &element );
        CV_ERROR_FROM_STATUS( CV_BADFLAG_ERR );
    }

    __END__;

    return element;
}


CV_IMPL void
cvReleaseStructuringElement( IplConvKernel ** element )
{
    CV_FUNCNAME( "cvReleaseStructuringElement" );

    __BEGIN__;

    if( !element )
        CV_ERROR_FROM_STATUS( CV_NULLPTR_ERR );
    icvFree( element );

    __END__;
}


static void icvInitMorphologyTab( CvBigFuncTable* rect_erode, CvBigFuncTable* rect_dilate,
                                CvBigFuncTable* cross_erode, CvBigFuncTable* cross_dilate,
                                CvBigFuncTable* arb_erode, CvBigFuncTable* arb_dilate )
{
    rect_erode->fn_2d[CV_8UC1] = (void*)icvErodeStrip_Rect_8u_C1R;
    rect_erode->fn_2d[CV_8UC3] = (void*)icvErodeStrip_Rect_8u_C3R;
    rect_erode->fn_2d[CV_8UC4] = (void*)icvErodeStrip_Rect_8u_C4R;

    rect_erode->fn_2d[CV_32FC1] = (void*)icvErodeStrip_Rect_32f_C1R;
    rect_erode->fn_2d[CV_32FC3] = (void*)icvErodeStrip_Rect_32f_C3R;
    rect_erode->fn_2d[CV_32FC4] = (void*)icvErodeStrip_Rect_32f_C4R;

    cross_erode->fn_2d[CV_8UC1] = (void*)icvErodeStrip_Cross_8u_C1R;
    cross_erode->fn_2d[CV_8UC3] = (void*)icvErodeStrip_Cross_8u_C3R;
    cross_erode->fn_2d[CV_8UC4] = (void*)icvErodeStrip_Cross_8u_C4R;

    cross_erode->fn_2d[CV_32FC1] = (void*)icvErodeStrip_Cross_32f_C1R;
    cross_erode->fn_2d[CV_32FC3] = (void*)icvErodeStrip_Cross_32f_C3R;
    cross_erode->fn_2d[CV_32FC4] = (void*)icvErodeStrip_Cross_32f_C4R;

    arb_erode->fn_2d[CV_8UC1] = (void*)icvErodeStrip_8u_C1R;
    arb_erode->fn_2d[CV_8UC3] = (void*)icvErodeStrip_8u_C3R;
    arb_erode->fn_2d[CV_8UC4] = (void*)icvErodeStrip_8u_C4R;

    arb_erode->fn_2d[CV_32FC1] = (void*)icvErodeStrip_32f_C1R;
    arb_erode->fn_2d[CV_32FC3] = (void*)icvErodeStrip_32f_C3R;
    arb_erode->fn_2d[CV_32FC4] = (void*)icvErodeStrip_32f_C4R;

    rect_dilate->fn_2d[CV_8UC1] = (void*)icvDilateStrip_Rect_8u_C1R;
    rect_dilate->fn_2d[CV_8UC3] = (void*)icvDilateStrip_Rect_8u_C3R;
    rect_dilate->fn_2d[CV_8UC4] = (void*)icvDilateStrip_Rect_8u_C4R;

    rect_dilate->fn_2d[CV_32FC1] = (void*)icvDilateStrip_Rect_32f_C1R;
    rect_dilate->fn_2d[CV_32FC3] = (void*)icvDilateStrip_Rect_32f_C3R;
    rect_dilate->fn_2d[CV_32FC4] = (void*)icvDilateStrip_Rect_32f_C4R;

    cross_dilate->fn_2d[CV_8UC1] = (void*)icvDilateStrip_Cross_8u_C1R;
    cross_dilate->fn_2d[CV_8UC3] = (void*)icvDilateStrip_Cross_8u_C3R;
    cross_dilate->fn_2d[CV_8UC4] = (void*)icvDilateStrip_Cross_8u_C4R;

    cross_dilate->fn_2d[CV_32FC1] = (void*)icvDilateStrip_Cross_32f_C1R;
    cross_dilate->fn_2d[CV_32FC3] = (void*)icvDilateStrip_Cross_32f_C3R;
    cross_dilate->fn_2d[CV_32FC4] = (void*)icvDilateStrip_Cross_32f_C4R;

    arb_dilate->fn_2d[CV_8UC1] = (void*)icvDilateStrip_8u_C1R;
    arb_dilate->fn_2d[CV_8UC3] = (void*)icvDilateStrip_8u_C3R;
    arb_dilate->fn_2d[CV_8UC4] = (void*)icvDilateStrip_8u_C4R;

    arb_dilate->fn_2d[CV_32FC1] = (void*)icvDilateStrip_32f_C1R;
    arb_dilate->fn_2d[CV_32FC3] = (void*)icvDilateStrip_32f_C3R;
    arb_dilate->fn_2d[CV_32FC4] = (void*)icvDilateStrip_32f_C4R;
}

static void
icvMorphOp( const void* srcarr, void* dstarr, IplConvKernel* element,
            int iterations, int mop )
{
    static CvBigFuncTable morph_tab[6];
    static int inittab = 0;
    CvMorphState *state = 0;

    CV_FUNCNAME( "icvMorphOp" );

    __BEGIN__;

    CvMorphFunc func = 0;
    CvElementShape shape;
    int i, coi1 = 0, coi2 = 0;
    CvMat srcstub, *src = (CvMat*)srcarr;
    CvMat dststub, *dst = (CvMat*)dstarr;
    CvSize size;
    int type;
    int src_step, dst_step;

    if( !inittab )
    {
        icvInitMorphologyTab( morph_tab + 0, morph_tab + 1, morph_tab + 2,
                              morph_tab + 3, morph_tab + 4, morph_tab + 5 );
        inittab = 1;
    }

    CV_CALL( src = cvGetMat( src, &srcstub, &coi1 ));
    
    if( src != &srcstub )
    {
        srcstub = *src;
        src = &srcstub;
    }

    if( dstarr == srcarr )
    {
        dst = src;
    }
    else
    {
        CV_CALL( dst = cvGetMat( dst, &dststub, &coi2 ));

        if( !CV_ARE_TYPES_EQ( src, dst ))
            CV_ERROR( CV_StsUnmatchedFormats, "" );

        if( !CV_ARE_SIZES_EQ( src, dst ))
            CV_ERROR( CV_StsUnmatchedSizes, "" );
    }

    if( dst != &dststub )
    {
        dststub = *dst;
        dst = &dststub;
    }

    if( coi1 != 0 || coi2 != 0 )
        CV_ERROR( CV_BadCOI, "" );

    type = CV_MAT_TYPE( src->type );

    if( element )
    {
        IPPI_CALL(
            icvMorphologyInitAlloc( src->width, CV_MAT_DEPTH(type) == CV_8U ? cv8u : cv32f,
                                 CV_MAT_CN(type), cvSize( element->nCols, element->nRows),
                                 cvPoint( element->anchorX, element->anchorY ),
                                 (CvElementShape) (element->nShiftR), element->values,
                                 &state ));
        shape = (CvElementShape) (element->nShiftR);
        shape = shape < CV_SHAPE_ELLIPSE ? shape : CV_SHAPE_CUSTOM;
    }
    else
    {
        IPPI_CALL(
            icvMorphologyInitAlloc( src->width, CV_MAT_DEPTH(type) == CV_8U ? cv8u : cv32f,
                                  CV_MAT_CN(type), cvSize( 3, 3 ), cvPoint( 1, 1 ),
                                  CV_SHAPE_RECT, 0, &state ));
        shape = CV_SHAPE_RECT;
    }

    func = (CvMorphFunc)(morph_tab[(shape == CV_SHAPE_RECT ? 0 :
                         shape == CV_SHAPE_CROSS ? 1 : 2) * 2 + mop].fn_2d[type]);

    if( !func )
        CV_ERROR( CV_StsUnsupportedFormat, "" );

    size = icvGetMatSize( src );

    src_step = src->step;
    dst_step = dst->step;

    if( src_step == 0 )
        src_step = dst_step = size.width * icvPixSize[type];

    for( i = 0; i < iterations; i++ )
    {
        IPPI_CALL( func( src->data.ptr, src_step, dst->data.ptr,
                         dst_step, &size, state, 0 ));
        src = dst;
    }

    __END__;

    icvMorphologyFree( &state );
}



CV_IMPL void
cvErode( const void* src, void* dst, IplConvKernel* element, int iterations )
{
    icvMorphOp( src, dst, element, iterations, 0 );
}


CV_IMPL void
cvDilate( const void* src, void* dst, IplConvKernel* element, int iterations )
{
    icvMorphOp( src, dst, element, iterations, 1 );
}


CV_IMPL void
cvMorphologyEx( const void* src, void* dst,
                void* temp, IplConvKernel* element, CvMorphOp op, int iterations )
{
    CV_FUNCNAME( "cvMorhologyEx" );

    __BEGIN__;

    if( (op == CV_MOP_GRADIENT ||
        (op == CV_MOP_TOPHAT || op == CV_MOP_BLACKHAT) && src == dst) && temp == 0 )
        CV_ERROR( CV_HeaderIsNull, "temp image required" );

    if( temp == src || temp == dst )
        CV_ERROR( CV_HeaderIsNull, "temp image is equal to src or dst" );

    switch (op)
    {
    case CV_MOP_OPEN:
        CV_CALL( cvErode( src, dst, element, iterations ));
        CV_CALL( cvDilate( dst, dst, element, iterations ));
        break;
    case CV_MOP_CLOSE:
        CV_CALL( cvDilate( src, dst, element, iterations ));
        CV_CALL( cvErode( dst, dst, element, iterations ));
        break;
    case CV_MOP_GRADIENT:
        CV_CALL( cvErode( src, temp, element, iterations ));
        CV_CALL( cvDilate( src, dst, element, iterations ));
        CV_CALL( cvSub( dst, temp, dst ));
        break;
    case CV_MOP_TOPHAT:
        CV_CALL( cvErode( src, src != dst ? dst : temp, element, iterations ));
        CV_CALL( cvSub( src, src != dst ? dst : temp, dst ));
        break;
    case CV_MOP_BLACKHAT:
        CV_CALL( cvDilate( src, src != dst ? dst : temp, element, iterations ));
        CV_CALL( cvSub( src != dst ? dst : temp, src, dst ));
        break;
    default:
        CV_ERROR( CV_StsBadArg, "unknown morphological operation" );
    }

    __END__;
}

/* End of file. */
