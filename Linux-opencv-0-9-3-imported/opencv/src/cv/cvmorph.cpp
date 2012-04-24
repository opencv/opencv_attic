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

#define  CV_COPY( dst, src, len, idx )                            \
    for( (idx) = 0; (idx) < (len); (idx)++) (dst)[idx] = (src)[idx]

#define  CV_SET( dst, val, len, idx )                         \
    for( (idx) = 0; (idx) < (len); (idx)++) (dst)[idx] = (val)  \


typedef struct
{
    uchar  b, g, r;
}
CvRGB8u;


typedef struct
{
    uchar  b, g, r, a;
}
CvRGBA8u;


typedef struct
{
    int  b, g, r;
}
CvRGB32s;


typedef struct
{
    int  b, g, r, a;
}
CvRGBA32s;

#undef   CV_CALC_MAX
#undef   CV_CALC_MIN

#define  CV_CALC_MIN(a, b) (a) = CV_IMIN((a),(b))
#define  CV_CALC_MAX(a, b) (a) = CV_IMAX((a),(b))

typedef struct CvMorphState
{
    /* kernel data */
    int ker_width;
    int ker_height;
    int ker_x;
    int ker_y;
    CvElementShape shape;
    uchar *ker_erode_mask;
    uchar *ker_dilate_mask;

    /* image data */
    int max_width;
    CvDataType dataType;
    int channels;

    /* cyclic buffer */
    char *buffer;
    int buffer_step;
    int crows;
    char **rows;
    char *tbuf;
}
CvMorphState;


#define CV_MORPH_ALIGN  ((int)sizeof(long))

IPCVAPI_IMPL( CvStatus, icvMorphologyInitAlloc, (int roiWidth,
                                                 CvDataType dataType, int channels,
                                                 CvSize elSize, CvPoint elAnchor,
                                                 CvElementShape elShape, const int *elData,
                                                 struct CvMorphState ** morphState) )
{
    CvMorphState *state = 0;
    int ker_size = 0;
    int buffer_step = 0;
    int buffer_size;
    int bt_pix, bt_pix_n;
    int aligned_hdr_size = icvAlign((int)sizeof(*state),CV_MORPH_ALIGN);
    int temp_lines = dataType == cv32f ? 2 : 1;
    char *ptr;

    int i, mask_size;

    if( !morphState )
        return CV_NULLPTR_ERR;
    *morphState = 0;

    if( roiWidth <= 0 )
        return CV_BADSIZE_ERR;
    if( dataType != cv8u && dataType != cv32f )
        return CV_BADDEPTH_ERR;

    if( channels != 1 && channels != 3 && channels != 4 )
        return CV_UNSUPPORTED_CHANNELS_ERR;
    if( elSize.width <= 0 || elSize.height <= 0 )
        return CV_BADSIZE_ERR;
    if( (unsigned) elAnchor.x >= (unsigned) elSize.width ||
        (unsigned) elAnchor.y >= (unsigned) elSize.height )
        return CV_BADRANGE_ERR;

    switch (elShape)
    {
    case CV_SHAPE_RECT:
    case CV_SHAPE_CROSS:
        break;
    case CV_SHAPE_ELLIPSE:
    case CV_SHAPE_CUSTOM:
        if( elShape == CV_SHAPE_CUSTOM && elData == 0 )
            return CV_NULLPTR_ERR;
        ker_size = (elSize.width + 1) * elSize.height * 2;
        break;
    }

    bt_pix = dataType != cv32f ? 1 : 4;
    bt_pix_n = bt_pix * channels;

    buffer_step = (((roiWidth + elSize.width + 1 + CV_MORPH_ALIGN * 2) * bt_pix_n - 1)
                   & -CV_MORPH_ALIGN * bt_pix);

    buffer_size = (elSize.height + temp_lines) * (buffer_step + sizeof( void * )) +
        ker_size + aligned_hdr_size + elSize.width * bt_pix_n;

    buffer_size = icvAlign(buffer_size + CV_MORPH_ALIGN, CV_MORPH_ALIGN);

    state = (CvMorphState *) icvAlloc( buffer_size );
    if( !state )
        return CV_OUTOFMEM_ERR;

    ptr = (char *) state;
    state->buffer_step = buffer_step;
    state->crows = 0;

    state->ker_x = elAnchor.x;
    state->ker_y = elAnchor.y;
    state->ker_height = elSize.height;
    state->ker_width = elSize.width;
    state->shape = elShape;

    state->max_width = roiWidth;
    state->dataType = dataType;
    state->channels = channels;
    ptr += ((aligned_hdr_size + elAnchor.x * bt_pix_n + CV_MORPH_ALIGN * bt_pix - 1)
            & -CV_MORPH_ALIGN * bt_pix);

    state->buffer = ptr;
    ptr += buffer_step * elSize.height;

    state->tbuf = ptr;
    ptr += buffer_step * temp_lines;

    state->ker_dilate_mask = (uchar *) ptr;
    state->ker_erode_mask = (uchar *) (ptr + (ker_size >> 1));

    mask_size = elSize.width * elSize.height;

    if( elShape == CV_SHAPE_CUSTOM )
    {
        for( i = 0; i < mask_size; i++ )
        {
            int t = elData[i] ? -1 : 0;

            ptr[i] = (char) t;
            ptr[i + (ker_size >> 1)] = (char) (~t);
        }
    }
    else if( elShape == CV_SHAPE_ELLIPSE )
    {
        int r = elSize.height / 2;
        int c = elSize.width / 2;
        double inv_r2 = 1. / (((double) (r)) * (r));

        memset( ptr, 0, ker_size );

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
            x2 -= x1 - 1;
            memset( ptr + i * elSize.width + x1, -1, x2 );
            memset( ptr + (elSize.height - i - 1) * elSize.width + x1, -1, x2 );
        }

        for( i = 0; i < mask_size; i++ )
        {
            ptr[i + (ker_size >> 1)] = (char) (~ptr[i]);
        }
    }

    ptr += ker_size;

    state->rows = (char **) ptr;
    if( state->shape != CV_SHAPE_RECT && state->shape != CV_SHAPE_CROSS )
        state->shape = CV_SHAPE_CUSTOM;

    *morphState = state;
    return CV_OK;
}


IPCVAPI_IMPL( CvStatus, icvMorphologyFree, (CvMorphState ** morphState) )
{
    if( !morphState )
        return CV_NULLPTR_ERR;
    icvFree( (void **) morphState );
    return CV_OK;
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

    int is_cross = state->shape == CV_SHAPE_CROSS;

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
    uchar *ker_data = state->ker_erode_mask + ker_width;

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

    int is_cross = state->shape == CV_SHAPE_CROSS;

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
    char *ker_data = (char *) (state->ker_erode_mask + ker_width);

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
    char *ker_data = (char *) (state->ker_dilate_mask + ker_width);

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

    int is_cross = state->shape == CV_SHAPE_CROSS;

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
    uchar *ker_data = state->ker_dilate_mask + ker_width;

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

    int is_cross = state->shape == CV_SHAPE_CROSS;

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

    if( state->shape != shape )
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
                                Erosion, byte-depth format
\****************************************************************************************/

/********************************** Rectangular, 8u, C1 *********************************/

IPCVAPI_IMPL( CvStatus, icvErodeStrip_Rect_8u_C1R, (const uchar * pSrc, int srcStep,
                                                    uchar * pDst, int dstStep,
                                                    CvSize * roiSize,
                                                    CvMorphState * state, int stage) )
{
    CvStatus status = icvCheckMorphArgs( pSrc, srcStep, pDst, dstStep, roiSize,
                                         state, stage, CV_SHAPE_RECT, cv8u, 1 );

    if( status == CV_OK )
    {
        status = icvErodeRC_8u( (uchar *) pSrc, srcStep, pDst, dstStep,
                                roiSize, state, stage );
    }

    return status >= 0 ? CV_OK : status;
}


/********************************** Rectangular, 8u, C3 *********************************/

IPCVAPI_IMPL( CvStatus, icvErodeStrip_Rect_8u_C3R, (const uchar * pSrc, int srcStep,
                                                    uchar * pDst, int dstStep,
                                                    CvSize * roiSize,
                                                    CvMorphState * state, int stage) )
{
    CvStatus status = icvCheckMorphArgs( pSrc, srcStep, pDst, dstStep, roiSize,
                                         state, stage, CV_SHAPE_RECT, cv8u, 3 );

    if( status == CV_OK )
    {
        status = icvErodeRC_8u( (uchar *) pSrc, srcStep, pDst, dstStep,
                                roiSize, state, stage );
    }

    return status >= 0 ? CV_OK : status;
}


/********************************** Rectangular, 8u, C4 *********************************/

IPCVAPI_IMPL( CvStatus, icvErodeStrip_Rect_8u_C4R, (const uchar * pSrc, int srcStep,
                                                    uchar * pDst, int dstStep,
                                                    CvSize * roiSize,
                                                    CvMorphState * state, int stage) )
{
    CvStatus status = icvCheckMorphArgs( pSrc, srcStep, pDst, dstStep, roiSize,
                                         state, stage, CV_SHAPE_RECT, cv8u, 4 );

    if( status == CV_OK )
    {
        status = icvErodeRC_8u( (uchar *) pSrc, srcStep, pDst, dstStep,
                                roiSize, state, stage );
    }

    return status >= 0 ? CV_OK : status;
}


/********************************* Cross-shaped, 8u, C1 *********************************/

IPCVAPI_IMPL( CvStatus, icvErodeStrip_Cross_8u_C1R, (const uchar * pSrc, int srcStep,
                                                     uchar * pDst, int dstStep,
                                                     CvSize * roiSize,
                                                     CvMorphState * state, int stage) )
{
    CvStatus status = icvCheckMorphArgs( pSrc, srcStep, pDst, dstStep, roiSize,
                                         state, stage, CV_SHAPE_CROSS, cv8u, 1 );

    if( status == CV_OK )
    {
        status = icvErodeRC_8u( (uchar *) pSrc, srcStep, pDst, dstStep,
                                roiSize, state, stage );
    }

    return status >= 0 ? CV_OK : status;
}


/********************************** Cross-shaped, 8u, C3 ********************************/

IPCVAPI_IMPL( CvStatus, icvErodeStrip_Cross_8u_C3R, (const uchar * pSrc, int srcStep,
                                                     uchar * pDst, int dstStep,
                                                     CvSize * roiSize,
                                                     CvMorphState * state, int stage) )
{
    CvStatus status = icvCheckMorphArgs( pSrc, srcStep, pDst, dstStep, roiSize,
                                         state, stage, CV_SHAPE_CROSS, cv8u, 3 );

    if( status == CV_OK )
    {
        status = icvErodeRC_8u( (uchar *) pSrc, srcStep, pDst, dstStep,
                                roiSize, state, stage );
    }

    return status >= 0 ? CV_OK : status;
}


/********************************** Cross-shaped, 8u, C4 ********************************/

IPCVAPI_IMPL( CvStatus, icvErodeStrip_Cross_8u_C4R, (const uchar * pSrc, int srcStep,
                                                     uchar * pDst, int dstStep,
                                                     CvSize * roiSize,
                                                     CvMorphState * state, int stage) )
{
    CvStatus status = icvCheckMorphArgs( pSrc, srcStep, pDst, dstStep, roiSize,
                                         state, stage, CV_SHAPE_CROSS, cv8u, 4 );

    if( status == CV_OK )
    {
        status = icvErodeRC_8u( (uchar *) pSrc, srcStep, pDst, dstStep,
                                roiSize, state, stage );
    }

    return status >= 0 ? CV_OK : status;
}


/****************************************************************************************\
                                Dilatation, byte-depth format
\****************************************************************************************/

/********************************** Rectangular, 8u, C1 *********************************/

IPCVAPI_IMPL( CvStatus, icvDilateStrip_Rect_8u_C1R, (const uchar * pSrc, int srcStep,
                                                     uchar * pDst, int dstStep,
                                                     CvSize * roiSize,
                                                     CvMorphState * state, int stage) )
{
    CvStatus status = icvCheckMorphArgs( pSrc, srcStep, pDst, dstStep, roiSize,
                                         state, stage, CV_SHAPE_RECT, cv8u, 1 );

    if( status == CV_OK )
    {
        status = icvDilateRC_8u( (uchar *) pSrc, srcStep, pDst, dstStep,
                                 roiSize, state, stage );
    }

    return status >= 0 ? CV_OK : status;
}


/********************************** Rectangular, 8u, C3 *********************************/

IPCVAPI_IMPL( CvStatus, icvDilateStrip_Rect_8u_C3R, (const uchar * pSrc, int srcStep,
                                                     uchar * pDst, int dstStep,
                                                     CvSize * roiSize,
                                                     CvMorphState * state, int stage) )
{
    CvStatus status = icvCheckMorphArgs( pSrc, srcStep, pDst, dstStep, roiSize,
                                         state, stage, CV_SHAPE_RECT, cv8u, 3 );

    if( status == CV_OK )
    {
        status = icvDilateRC_8u( (uchar *) pSrc, srcStep, pDst, dstStep,
                                 roiSize, state, stage );
    }

    return status >= 0 ? CV_OK : status;
}


/********************************** Rectangular, 8u, C4 *********************************/

IPCVAPI_IMPL( CvStatus, icvDilateStrip_Rect_8u_C4R, (const uchar * pSrc, int srcStep,
                                                     uchar * pDst, int dstStep,
                                                     CvSize * roiSize,
                                                     CvMorphState * state, int stage) )
{
    CvStatus status = icvCheckMorphArgs( pSrc, srcStep, pDst, dstStep, roiSize,
                                         state, stage, CV_SHAPE_RECT, cv8u, 4 );

    if( status == CV_OK )
    {
        status = icvDilateRC_8u( (uchar *) pSrc, srcStep, pDst, dstStep,
                                 roiSize, state, stage );
    }

    return status >= 0 ? CV_OK : status;
}


/********************************* Cross-shaped, 8u, C1 *********************************/

IPCVAPI_IMPL( CvStatus, icvDilateStrip_Cross_8u_C1R, (const uchar * pSrc, int srcStep,
                                                      uchar * pDst, int dstStep,
                                                      CvSize * roiSize,
                                                      CvMorphState * state, int stage) )
{
    CvStatus status = icvCheckMorphArgs( pSrc, srcStep, pDst, dstStep, roiSize,
                                         state, stage, CV_SHAPE_CROSS, cv8u, 1 );

    if( status == CV_OK )
    {
        status = icvDilateRC_8u( (uchar *) pSrc, srcStep, pDst, dstStep,
                                 roiSize, state, stage );
    }

    return status >= 0 ? CV_OK : status;
}


/********************************** Cross-shaped, 8u, C3 ********************************/

IPCVAPI_IMPL( CvStatus, icvDilateStrip_Cross_8u_C3R, (const uchar * pSrc, int srcStep,
                                                      uchar * pDst, int dstStep,
                                                      CvSize * roiSize,
                                                      CvMorphState * state, int stage) )
{
    CvStatus status = icvCheckMorphArgs( pSrc, srcStep, pDst, dstStep, roiSize,
                                         state, stage, CV_SHAPE_CROSS, cv8u, 3 );

    if( status == CV_OK )
    {
        status = icvDilateRC_8u( (uchar *) pSrc, srcStep, pDst, dstStep,
                                 roiSize, state, stage );
    }

    return status >= 0 ? CV_OK : status;
}


/********************************** Cross-shaped, 8u, C4 ********************************/

IPCVAPI_IMPL( CvStatus, icvDilateStrip_Cross_8u_C4R, (const uchar * pSrc, int srcStep,
                                                      uchar * pDst, int dstStep,
                                                      CvSize * roiSize,
                                                      CvMorphState * state, int stage) )
{
    CvStatus status = icvCheckMorphArgs( pSrc, srcStep, pDst, dstStep, roiSize,
                                         state, stage, CV_SHAPE_CROSS, cv8u, 4 );

    if( status == CV_OK )
    {
        status = icvDilateRC_8u( (uchar *) pSrc, srcStep, pDst, dstStep,
                                 roiSize, state, stage );
    }

    return status >= 0 ? CV_OK : status;
}



/****************************************************************************************\
                                Erosion, floating-point format
\****************************************************************************************/

/********************************** Rectangular, 32f, C1 ********************************/

IPCVAPI_IMPL( CvStatus, icvErodeStrip_Rect_32f_C1R, (const float *pSrc, int srcStep,
                                                     float *pDst, int dstStep,
                                                     CvSize * roiSize,
                                                     CvMorphState * state, int stage) )
{
    CvStatus status = icvCheckMorphArgs( pSrc, srcStep, pDst, dstStep, roiSize,
                                         state, stage, CV_SHAPE_RECT, cv32f, 1 );

    if( status == CV_OK )
    {
        status = icvErodeRC_32f( (float *) pSrc, srcStep, pDst, dstStep,
                                 roiSize, state, stage );
    }

    return status >= 0 ? CV_OK : status;
}


/********************************** Rectangular, 32f, C3 ********************************/

IPCVAPI_IMPL( CvStatus, icvErodeStrip_Rect_32f_C3R, (const float *pSrc, int srcStep,
                                                     float *pDst, int dstStep,
                                                     CvSize * roiSize,
                                                     CvMorphState * state, int stage) )
{
    CvStatus status = icvCheckMorphArgs( pSrc, srcStep, pDst, dstStep, roiSize,
                                         state, stage, CV_SHAPE_RECT, cv32f, 3 );

    if( status == CV_OK )
    {
        status = icvErodeRC_32f( (float *) pSrc, srcStep, pDst, dstStep,
                                 roiSize, state, stage );
    }

    return status >= 0 ? CV_OK : status;
}


/********************************** Rectangular, 32f, C4 ********************************/

IPCVAPI_IMPL( CvStatus, icvErodeStrip_Rect_32f_C4R, (const float *pSrc, int srcStep,
                                                     float *pDst, int dstStep,
                                                     CvSize * roiSize,
                                                     CvMorphState * state, int stage) )
{
    CvStatus status = icvCheckMorphArgs( pSrc, srcStep, pDst, dstStep, roiSize,
                                         state, stage, CV_SHAPE_RECT, cv32f, 4 );

    if( status == CV_OK )
    {
        status = icvErodeRC_32f( (float *) pSrc, srcStep, pDst, dstStep,
                                 roiSize, state, stage );
    }

    return status >= 0 ? CV_OK : status;
}


/********************************* Cross-shaped, 32f, C1 ********************************/

IPCVAPI_IMPL( CvStatus, icvErodeStrip_Cross_32f_C1R, (const float *pSrc, int srcStep,
                                                      float *pDst, int dstStep,
                                                      CvSize * roiSize,
                                                      CvMorphState * state, int stage) )
{
    CvStatus status = icvCheckMorphArgs( pSrc, srcStep, pDst, dstStep, roiSize,
                                         state, stage, CV_SHAPE_CROSS, cv32f, 1 );

    if( status == CV_OK )
    {
        status = icvErodeRC_32f( (float *) pSrc, srcStep, pDst, dstStep,
                                 roiSize, state, stage );
    }

    return status >= 0 ? CV_OK : status;
}


/********************************** Cross-shaped, 32f, C3 *******************************/

IPCVAPI_IMPL( CvStatus, icvErodeStrip_Cross_32f_C3R, (const float *pSrc, int srcStep,
                                                      float *pDst, int dstStep,
                                                      CvSize * roiSize,
                                                      CvMorphState * state, int stage) )
{
    CvStatus status = icvCheckMorphArgs( pSrc, srcStep, pDst, dstStep, roiSize,
                                         state, stage, CV_SHAPE_CROSS, cv32f, 3 );

    if( status == CV_OK )
    {
        status = icvErodeRC_32f( (float *) pSrc, srcStep, pDst, dstStep,
                                 roiSize, state, stage );
    }

    return status >= 0 ? CV_OK : status;
}


/********************************** Cross-shaped, 32f, C4 *******************************/

IPCVAPI_IMPL( CvStatus, icvErodeStrip_Cross_32f_C4R, (const float *pSrc, int srcStep,
                                                      float *pDst, int dstStep,
                                                      CvSize * roiSize,
                                                      CvMorphState * state, int stage) )
{
    CvStatus status = icvCheckMorphArgs( pSrc, srcStep, pDst, dstStep, roiSize,
                                         state, stage, CV_SHAPE_CROSS, cv32f, 4 );

    if( status == CV_OK )
    {
        status = icvErodeRC_32f( (float *) pSrc, srcStep, pDst, dstStep,
                                 roiSize, state, stage );
    }

    return status >= 0 ? CV_OK : status;
}


/****************************************************************************************\
                                Dilatation, floating-point format
\****************************************************************************************/

/********************************** Rectangular, 32f, C1 ********************************/

IPCVAPI_IMPL( CvStatus, icvDilateStrip_Rect_32f_C1R, (const float *pSrc, int srcStep,
                                                      float *pDst, int dstStep,
                                                      CvSize * roiSize,
                                                      CvMorphState * state, int stage) )
{
    CvStatus status = icvCheckMorphArgs( pSrc, srcStep, pDst, dstStep, roiSize,
                                         state, stage, CV_SHAPE_RECT, cv32f, 1 );

    if( status == CV_OK )
    {
        status = icvDilateRC_32f( (float *) pSrc, srcStep, pDst, dstStep,
                                  roiSize, state, stage );
    }

    return status >= 0 ? CV_OK : status;
}


/********************************** Rectangular, 32f, C3 ********************************/

IPCVAPI_IMPL( CvStatus, icvDilateStrip_Rect_32f_C3R, (const float *pSrc, int srcStep,
                                                      float *pDst, int dstStep,
                                                      CvSize * roiSize,
                                                      CvMorphState * state, int stage) )
{
    CvStatus status = icvCheckMorphArgs( pSrc, srcStep, pDst, dstStep, roiSize,
                                         state, stage, CV_SHAPE_RECT, cv32f, 3 );

    if( status == CV_OK )
    {
        status = icvDilateRC_32f( (float *) pSrc, srcStep, pDst, dstStep,
                                  roiSize, state, stage );
    }

    return status >= 0 ? CV_OK : status;
}


/********************************** Rectangular, 32f, C4 ********************************/

IPCVAPI_IMPL( CvStatus, icvDilateStrip_Rect_32f_C4R, (const float *pSrc, int srcStep,
                                                      float *pDst, int dstStep,
                                                      CvSize * roiSize,
                                                      CvMorphState * state, int stage) )
{
    CvStatus status = icvCheckMorphArgs( pSrc, srcStep, pDst, dstStep, roiSize,
                                         state, stage, CV_SHAPE_RECT, cv32f, 4 );

    if( status == CV_OK )
    {
        status = icvDilateRC_32f( (float *) pSrc, srcStep, pDst, dstStep,
                                  roiSize, state, stage );
    }

    return status >= 0 ? CV_OK : status;
}


/********************************* Cross-shaped, 32f, C1 ********************************/

IPCVAPI_IMPL( CvStatus, icvDilateStrip_Cross_32f_C1R, (const float *pSrc, int srcStep,
                                                       float *pDst, int dstStep,
                                                       CvSize * roiSize,
                                                       CvMorphState * state, int stage) )
{
    CvStatus status = icvCheckMorphArgs( pSrc, srcStep, pDst, dstStep, roiSize,
                                         state, stage, CV_SHAPE_CROSS, cv32f, 1 );

    if( status == CV_OK )
    {
        status = icvDilateRC_32f( (float *) pSrc, srcStep, pDst, dstStep,
                                  roiSize, state, stage );
    }

    return status >= 0 ? CV_OK : status;
}


/********************************** Cross-shaped, 32f, C3 *******************************/

IPCVAPI_IMPL( CvStatus, icvDilateStrip_Cross_32f_C3R, (const float *pSrc, int srcStep,
                                                       float *pDst, int dstStep,
                                                       CvSize * roiSize,
                                                       CvMorphState * state, int stage) )
{
    CvStatus status = icvCheckMorphArgs( pSrc, srcStep, pDst, dstStep, roiSize,
                                         state, stage, CV_SHAPE_CROSS, cv32f, 3 );

    if( status == CV_OK )
    {
        status = icvDilateRC_32f( (float *) pSrc, srcStep, pDst, dstStep,
                                  roiSize, state, stage );
    }

    return status >= 0 ? CV_OK : status;
}


/********************************** Cross-shaped, 32f, C4 *******************************/

IPCVAPI_IMPL( CvStatus, icvDilateStrip_Cross_32f_C4R, (const float *pSrc, int srcStep,
                                                       float *pDst, int dstStep,
                                                       CvSize * roiSize,
                                                       CvMorphState * state, int stage) )
{
    CvStatus status = icvCheckMorphArgs( pSrc, srcStep, pDst, dstStep, roiSize,
                                         state, stage, CV_SHAPE_CROSS, cv32f, 4 );

    if( status == CV_OK )
    {
        status = icvDilateRC_32f( (float *) pSrc, srcStep, pDst, dstStep,
                                  roiSize, state, stage );
    }

    return status >= 0 ? CV_OK : status;
}


/********************************** Arbitrary, 8u, C1 ***********************************/

IPCVAPI_IMPL( CvStatus, icvErodeStrip_8u_C1R, (const uchar * pSrc, int srcStep,
                                               uchar * pDst, int dstStep,
                                               CvSize * roiSize,
                                               struct CvMorphState * state, int stage) )
{
    CvStatus status = icvCheckMorphArgs( pSrc, srcStep, pDst, dstStep, roiSize,
                                         state, stage, CV_SHAPE_CUSTOM, cv8u, 1 );

    if( status == CV_OK )
    {
        status = icvErodeArb_8u( (uchar *) pSrc, srcStep, pDst, dstStep,
                                 roiSize, state, stage );
    }

    return status >= 0 ? CV_OK : status;
}


IPCVAPI_IMPL( CvStatus, icvErodeStrip_8u_C3R, (const uchar * pSrc, int srcStep,
                                               uchar * pDst, int dstStep,
                                               CvSize * roiSize,
                                               struct CvMorphState * state, int stage) )
{
    CvStatus status = icvCheckMorphArgs( pSrc, srcStep, pDst, dstStep, roiSize,
                                         state, stage, CV_SHAPE_CUSTOM, cv8u, 3 );

    if( status == CV_OK )
    {
        status = icvErodeArb_8u( (uchar *) pSrc, srcStep, pDst, dstStep,
                                 roiSize, state, stage );
    }

    return status >= 0 ? CV_OK : status;
}


IPCVAPI_IMPL( CvStatus, icvErodeStrip_8u_C4R, (const uchar * pSrc, int srcStep,
                                               uchar * pDst, int dstStep,
                                               CvSize * roiSize,
                                               struct CvMorphState * state, int stage) )
{
    CvStatus status = icvCheckMorphArgs( pSrc, srcStep, pDst, dstStep, roiSize,
                                         state, stage, CV_SHAPE_CUSTOM, cv8u, 4 );

    if( status == CV_OK )
    {
        status = icvErodeArb_8u( (uchar *) pSrc, srcStep, pDst, dstStep,
                                 roiSize, state, stage );
    }

    return status >= 0 ? CV_OK : status;
}


IPCVAPI_IMPL( CvStatus, icvErodeStrip_32f_C1R, (const float *pSrc, int srcStep,
                                                float *pDst, int dstStep,
                                                CvSize * roiSize,
                                                struct CvMorphState * state, int stage) )
{
    CvStatus status = icvCheckMorphArgs( pSrc, srcStep, pDst, dstStep, roiSize,
                                         state, stage, CV_SHAPE_CUSTOM, cv32f, 1 );

    if( status == CV_OK )
    {
        status = icvErodeArb_32f( (float *) pSrc, srcStep, pDst, dstStep,
                                  roiSize, state, stage );
    }

    return status >= 0 ? CV_OK : status;
}


IPCVAPI_IMPL( CvStatus, icvErodeStrip_32f_C3R, (const float *pSrc, int srcStep,
                                                float *pDst, int dstStep,
                                                CvSize * roiSize,
                                                struct CvMorphState * state, int stage) )
{
    CvStatus status = icvCheckMorphArgs( pSrc, srcStep, pDst, dstStep, roiSize,
                                         state, stage, CV_SHAPE_CUSTOM, cv32f, 3 );

    if( status == CV_OK )
    {
        status = icvErodeArb_32f( (float *) pSrc, srcStep, pDst, dstStep,
                                  roiSize, state, stage );
    }

    return status >= 0 ? CV_OK : status;
}


IPCVAPI_IMPL( CvStatus, icvErodeStrip_32f_C4R, (const float *pSrc, int srcStep,
                                                float *pDst, int dstStep,
                                                CvSize * roiSize,
                                                struct CvMorphState * state, int stage) )
{
    CvStatus status = icvCheckMorphArgs( pSrc, srcStep, pDst, dstStep, roiSize,
                                         state, stage, CV_SHAPE_CUSTOM, cv32f, 4 );

    if( status == CV_OK )
    {
        status = icvErodeArb_32f( (float *) pSrc, srcStep, pDst, dstStep,
                                  roiSize, state, stage );
    }

    return status >= 0 ? CV_OK : status;
}


/* Dilate */
IPCVAPI_IMPL( CvStatus, icvDilateStrip_8u_C1R, (const uchar * pSrc, int srcStep,
                                                uchar * pDst, int dstStep,
                                                CvSize * roiSize,
                                                struct CvMorphState * state, int stage) )
{
    CvStatus status = icvCheckMorphArgs( pSrc, srcStep, pDst, dstStep, roiSize,
                                         state, stage, CV_SHAPE_CUSTOM, cv8u, 1 );

    if( status == CV_OK )
    {
        status = icvDilateArb_8u( (uchar *) pSrc, srcStep, pDst, dstStep,
                                  roiSize, state, stage );
    }

    return status >= 0 ? CV_OK : status;
}


IPCVAPI_IMPL( CvStatus, icvDilateStrip_8u_C3R, (const uchar * pSrc, int srcStep,
                                                uchar * pDst, int dstStep,
                                                CvSize * roiSize,
                                                struct CvMorphState * state, int stage) )
{
    CvStatus status = icvCheckMorphArgs( pSrc, srcStep, pDst, dstStep, roiSize,
                                         state, stage, CV_SHAPE_CUSTOM, cv8u, 3 );

    if( status == CV_OK )
    {
        status = icvDilateArb_8u( (uchar *) pSrc, srcStep, pDst, dstStep,
                                  roiSize, state, stage );
    }

    return status >= 0 ? CV_OK : status;
}


IPCVAPI_IMPL( CvStatus, icvDilateStrip_8u_C4R, (const uchar * pSrc, int srcStep,
                                                uchar * pDst, int dstStep,
                                                CvSize * roiSize,
                                                struct CvMorphState * state, int stage) )
{
    CvStatus status = icvCheckMorphArgs( pSrc, srcStep, pDst, dstStep, roiSize,
                                         state, stage, CV_SHAPE_CUSTOM, cv8u, 4 );

    if( status == CV_OK )
    {
        status = icvDilateArb_8u( (uchar *) pSrc, srcStep, pDst, dstStep,
                                  roiSize, state, stage );
    }

    return status >= 0 ? CV_OK : status;
}


IPCVAPI_IMPL( CvStatus, icvDilateStrip_32f_C1R, (const float *pSrc, int srcStep,
                                                 float *pDst, int dstStep,
                                                 CvSize * roiSize,
                                                 struct CvMorphState * state, int stage) )
{
    CvStatus status = icvCheckMorphArgs( pSrc, srcStep, pDst, dstStep, roiSize,
                                         state, stage, CV_SHAPE_CUSTOM, cv32f, 1 );

    if( status == CV_OK )
    {
        status = icvDilateArb_32f( (float *) pSrc, srcStep, pDst, dstStep,
                                   roiSize, state, stage );
    }

    return status >= 0 ? CV_OK : status;
}


IPCVAPI_IMPL( CvStatus, icvDilateStrip_32f_C3R, (const float *pSrc, int srcStep,
                                                 float *pDst, int dstStep,
                                                 CvSize * roiSize,
                                                 struct CvMorphState * state, int stage) )
{
    CvStatus status = icvCheckMorphArgs( pSrc, srcStep, pDst, dstStep, roiSize,
                                         state, stage, CV_SHAPE_CUSTOM, cv32f, 3 );

    if( status == CV_OK )
    {
        status = icvDilateArb_32f( (float *) pSrc, srcStep, pDst, dstStep,
                                   roiSize, state, stage );
    }

    return status >= 0 ? CV_OK : status;
}


IPCVAPI_IMPL( CvStatus, icvDilateStrip_32f_C4R, (const float *pSrc, int srcStep,
                                                 float *pDst, int dstStep,
                                                 CvSize * roiSize,
                                                 struct CvMorphState * state, int stage) )
{
    CvStatus status = icvCheckMorphArgs( pSrc, srcStep, pDst, dstStep, roiSize,
                                         state, stage, CV_SHAPE_CUSTOM, cv32f, 4 );

    if( status == CV_OK )
    {
        status = icvDilateArb_32f( (float *) pSrc, srcStep, pDst, dstStep,
                                   roiSize, state, stage );
    }

    return status >= 0 ? CV_OK : status;
}


/* End of file. */
