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
                              int elShape, int* elData,
                              struct CvMorphState** morphState ),
                              (roiWidth, dataType, channels, elSize, elAnchor,
                              elShape, elData, morphState) )
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

            elData = (int*)cvStackAlloc( elSize.width * elSize.height * sizeof(elData[0]));
            memset( elData, 0, elSize.width*elSize.height*sizeof(elData[0]));

            for( i = 0; i < r; i++ )
            {
                int y = r - i;
                int dx = cvRound( c * sqrt( ((double)r * r - y * y) * inv_r2 ));
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


IPCVAPI_IMPL( CvStatus, icvMorphologyFree, (CvMorphState ** morphState), (morphState) )
{
    return icvFilterFree( morphState );
}


/****************************************************************************************\
*         Erode/Dilate with rectangular or cross-shaped element for integer types        *
\****************************************************************************************/

#define ICV_DEF_MORPH_RECT_INT_FUNC( name, flavor, arrtype,                 \
                                     update_extr_macro )                    \
static CvStatus CV_STDCALL                                                  \
icv##name##RC_##flavor( arrtype* src, int srcstep,                          \
                        arrtype* dst, int dststep,                          \
                        CvSize* roi, CvMorphState* state, int stage )       \
{                                                                           \
    int width = roi->width;                                                 \
    int src_height = roi->height;                                           \
    int dst_height = src_height;                                            \
    int x, y = 0, i;                                                        \
                                                                            \
    int ker_x = state->ker_x;                                               \
    int ker_y = state->ker_y;                                               \
    int ker_width = state->ker_width;                                       \
    int ker_height = state->ker_height;                                     \
    int ker_right = ker_width - ker_x;                                      \
                                                                            \
    int crows = state->crows;                                               \
    arrtype** rows = (arrtype**)(state->rows);                              \
    arrtype* tbuf = (arrtype*)(state->tbuf);                                \
                                                                            \
    int channels = state->channels;                                         \
    int ker_x_n = ker_x * channels;                                         \
    int ker_width_n = ker_width * channels;                                 \
    int ker_right_n = ker_right * channels;                                 \
    int width_n = width * channels;                                         \
                                                                            \
    int is_small_width = width < MAX( ker_x, ker_right );                   \
    int starting_flag = 0;                                                  \
    int width_rest = width_n & (CV_MORPH_ALIGN - 1);                        \
                                                                            \
    int is_cross = ICV_BINARY_KERNEL_SHAPE(state->kerType)==CV_SHAPE_CROSS; \
                                                                            \
    /* initialize cyclic buffer when starting */                            \
    if( stage == CV_WHOLE || stage == CV_START )                            \
    {                                                                       \
        for( i = 0; i < ker_height; i++ )                                   \
            rows[i] = (arrtype*)(state->buffer + state->buffer_step * i);   \
                                                                            \
        crows = ker_y;                                                      \
        if( stage != CV_WHOLE )                                             \
            dst_height -= ker_height - ker_y - 1;                           \
        starting_flag = 1;                                                  \
    }                                                                       \
                                                                            \
    if( stage == CV_END )                                                   \
        dst_height += ker_height - ker_y - 1;                               \
                                                                            \
    do                                                                      \
    {                                                                       \
        int need_copy = is_small_width | (y == 0);                          \
        arrtype *tsrc, *tdst;                                               \
        arrtype *saved_row = rows[ker_y];                                   \
                                                                            \
        /* fill cyclic buffer - horizontal filtering */                     \
        for( ; crows < ker_height + is_cross; crows++ )                     \
        {                                                                   \
            if( crows < ker_height )                                        \
            {                                                               \
                tsrc = src - ker_x_n;                                       \
                tdst = rows[crows];                                         \
                                                                            \
                if( src_height-- <= 0 )                                     \
                {                                                           \
                    if( stage != CV_WHOLE && stage != CV_END )              \
                        break;                                              \
                    /* duplicate last row */                                \
                    tsrc = rows[crows - 1];                                 \
                    CV_COPY( tdst, tsrc, width_n, x );                      \
                    continue;                                               \
                }                                                           \
                                                                            \
                need_copy |= src_height == 1;                               \
            }                                                               \
            else                                                            \
            {                                                               \
                /* convolve center line for cross-shaped element */         \
                tsrc = rows[ker_y] - ker_x_n;                               \
                tdst = tbuf;                                                \
                                                                            \
                need_copy = 0;                                              \
            }                                                               \
                                                                            \
            if( ker_width > 1 && (!is_cross || crows == ker_height) )       \
            {                                                               \
                if( need_copy )                                             \
                {                                                           \
                    tsrc = tbuf - ker_x_n;                                  \
                    CV_COPY( tbuf, src, width_n, x );                       \
                }                                                           \
                else if( !is_cross )                                        \
                {                                                           \
                    CV_COPY( tbuf - ker_x_n, src - ker_x_n, ker_x_n, x );   \
                    CV_COPY( tbuf, src + width_n, ker_right_n, x );         \
                }                                                           \
                                                                            \
                /* make replication borders */                              \
                for( i = ker_x_n - 1; i >= 0; i-- )                         \
                    tsrc[i] = tsrc[i + channels];                           \
                for( i = width_n + ker_x_n; i < width_n + ker_width_n; i++ )\
                    tsrc[i] = tsrc[i - channels];                           \
                                                                            \
                /* row processing */                                        \
                if( channels == 1 )                                         \
                    for( i = 0; i < width_n; i += 2 )                       \
                    {                                                       \
                        int j;                                              \
                        int t, t0 = tsrc[i + 1];                            \
                                                                            \
                        for( j = 2; j < ker_width_n; j++ )                  \
                        {                                                   \
                            int t1 = tsrc[i + j];                           \
                            update_extr_macro( t0, t1 );                    \
                        }                                                   \
                                                                            \
                        t = tsrc[i];                                        \
                        update_extr_macro( t, t0 );                         \
                        tdst[i] = (arrtype) t;                              \
                                                                            \
                        t = tsrc[i + j];                                    \
                        update_extr_macro( t, t0 );                         \
                        tdst[i + 1] = (arrtype) t;                          \
                    }                                                       \
                else                                                        \
                    for( i = 0; i < width_n; i++ )                          \
                    {                                                       \
                        int j;                                              \
                        int t0 = tsrc[i];                                   \
                                                                            \
                        for( j = channels; j < ker_width_n; j += channels ) \
                        {                                                   \
                            int t1 = tsrc[i + j];                           \
                            update_extr_macro( t0, t1 );                    \
                        }                                                   \
                                                                            \
                        tdst[i] = (arrtype)t0;                              \
                    }                                                       \
                                                                            \
                if( !need_copy && !is_cross )                               \
                {                                                           \
                    /* restore borders */                                   \
                    CV_COPY( src - ker_x_n, tbuf - ker_x_n, ker_x_n, x );   \
                    CV_COPY( src + width_n, tbuf, ker_right_n, x );         \
                }                                                           \
            }                                                               \
            else                                                            \
                CV_COPY( tdst, tsrc + ker_x_n, width_n, x );                \
                                                                            \
            src += crows < ker_height ? srcstep : 0;                        \
        }                                                                   \
                                                                            \
        if( starting_flag )                                                 \
        {                                                                   \
            starting_flag = 0;                                              \
            tsrc = rows[ker_y];                                             \
                                                                            \
            for( i = 0; i < ker_y; i++ )                                    \
            {                                                               \
                tdst = rows[i];                                             \
                CV_COPY( tdst, tsrc, width_n, x );                          \
            }                                                               \
        }                                                                   \
                                                                            \
        /* vertical convolution */                                          \
        if( crows != ker_height )                                           \
        {                                                                   \
            if( crows < ker_height )                                        \
                break;                                                      \
            /* else it is cross-shaped element: change central line */      \
            rows[ker_y] = tbuf;                                             \
            crows--;                                                        \
        }                                                                   \
                                                                            \
        tdst = dst;                                                         \
                                                                            \
        if( width_rest )                                                    \
        {                                                                   \
            need_copy = width_n < CV_MORPH_ALIGN || y == dst_height - 1;    \
                                                                            \
            if( need_copy )                                                 \
                tdst = tbuf;                                                \
            else                                                            \
                CV_COPY( tbuf + width_n, dst + width_n, CV_MORPH_ALIGN, x );\
        }                                                                   \
                                                                            \
        for( x = 0; x < width_n; x += 4 )                                   \
        {                                                                   \
            int val0, val1, val2, val3;                                     \
                                                                            \
            tsrc = rows[0];                                                 \
                                                                            \
            val0 = tsrc[x];                                                 \
            val1 = tsrc[x + 1];                                             \
            val2 = tsrc[x + 2];                                             \
            val3 = tsrc[x + 3];                                             \
                                                                            \
            for( i = 1; i < ker_height; i++ )                               \
            {                                                               \
                tsrc = rows[i];                                             \
                int s = tsrc[x + 0];                                        \
                update_extr_macro( val0, s );                               \
                s = tsrc[x + 1];                                            \
                update_extr_macro( val1, s );                               \
                s = tsrc[x + 2];                                            \
                update_extr_macro( val2, s );                               \
                s = tsrc[x + 3];                                            \
                update_extr_macro( val3, s );                               \
            }                                                               \
                                                                            \
            tdst[x + 0] = (arrtype)val0;                                    \
            tdst[x + 1] = (arrtype)val1;                                    \
            tdst[x + 2] = (arrtype)val2;                                    \
            tdst[x + 3] = (arrtype)val3;                                    \
        }                                                                   \
                                                                            \
        if( width_rest )                                                    \
        {                                                                   \
            if( need_copy )                                                 \
                CV_COPY( dst, tbuf, width_n, x );                           \
            else                                                            \
                CV_COPY( dst + width_n, tbuf + width_n, CV_MORPH_ALIGN, x );\
        }                                                                   \
                                                                            \
        rows[ker_y] = saved_row;                                            \
                                                                            \
        /* rotate buffer */                                                 \
        {                                                                   \
            arrtype *t = rows[0];                                           \
                                                                            \
            CV_COPY( rows, rows + 1, ker_height - 1, i );                   \
            rows[i] = t;                                                    \
            crows--;                                                        \
            dst += dststep;                                                 \
        }                                                                   \
    }                                                                       \
    while( ++y < dst_height );                                              \
                                                                            \
    roi->height = y;                                                        \
    state->crows = crows;                                                   \
                                                                            \
    return CV_OK;                                                           \
}


/****************************************************************************************\
*         Erode/Dilate with rectangular or cross-shaped element for 32f type             *
\****************************************************************************************/

#define ICV_DEF_MORPH_RECT_FLT_FUNC( name, flavor, arrtype,                 \
                                     update_extr_macro )                    \
static CvStatus                                                             \
icv##name##RC_##flavor( arrtype* src, int srcstep,                          \
                        arrtype* dst, int dststep,                          \
                        CvSize* roi, CvMorphState* state, int stage )       \
{                                                                           \
    int width = roi->width;                                                 \
    int src_height = roi->height;                                           \
    int dst_height = src_height;                                            \
    int x, y = 0, i;                                                        \
                                                                            \
    int ker_x = state->ker_x;                                               \
    int ker_y = state->ker_y;                                               \
    int ker_width = state->ker_width;                                       \
    int ker_height = state->ker_height;                                     \
                                                                            \
    int crows = state->crows;                                               \
    int **rows = (int**)(state->rows);                                      \
    int *tbuf = (int*)(state->tbuf);                                        \
    int *tbuf2 = (int*)(state->tbuf + state->buffer_step);                  \
                                                                            \
    int channels = state->channels;                                         \
    int ker_x_n = ker_x * channels;                                         \
    int ker_width_n = ker_width * channels;                                 \
    int width_n = width * channels;                                         \
                                                                            \
    int starting_flag = 0;                                                  \
    int width_rest = width_n & (CV_MORPH_ALIGN - 1);                        \
                                                                            \
    int is_cross = ICV_BINARY_KERNEL_SHAPE(state->kerType)==CV_SHAPE_CROSS; \
                                                                            \
    /* initialize cyclic buffer when starting */                            \
    if( stage == CV_WHOLE || stage == CV_START )                            \
    {                                                                       \
        for( i = 0; i < ker_height; i++ )                                   \
        {                                                                   \
            rows[i] = (int *) (state->buffer + state->buffer_step * i);     \
        }                                                                   \
        crows = ker_y;                                                      \
        if( stage != CV_WHOLE )                                             \
            dst_height -= ker_height - ker_y - 1;                           \
        starting_flag = 1;                                                  \
    }                                                                       \
                                                                            \
    if( stage == CV_END )                                                   \
        dst_height += ker_height - ker_y - 1;                               \
                                                                            \
    srcstep /= sizeof(src[0]);                                              \
    dststep /= sizeof(dst[0]);                                              \
                                                                            \
    do                                                                      \
    {                                                                       \
        int need_copy = 0;                                                  \
        int *tsrc, *tdst;                                                   \
        int *saved_row = rows[ker_y];                                       \
                                                                            \
        /* fill cyclic buffer - horizontal filtering */                     \
        for( ; crows < ker_height + is_cross; crows++ )                     \
        {                                                                   \
            if( crows < ker_height )                                        \
            {                                                               \
                tsrc = (int *) src;                                         \
                tdst = rows[crows];                                         \
                                                                            \
                if( src_height-- <= 0 )                                     \
                {                                                           \
                    if( stage != CV_WHOLE && stage != CV_END )              \
                        break;                                              \
                    /* duplicate last row */                                \
                    tsrc = rows[crows - 1];                                 \
                    CV_COPY( tdst, tsrc, width_n, x );                      \
                    continue;                                               \
                }                                                           \
                                                                            \
                for( x = 0; x < width_n; x++ )                              \
                {                                                           \
                    int t = tsrc[x];                                        \
                                                                            \
                    tbuf2[x] = CV_TOGGLE_FLT( t );                          \
                }                                                           \
                tsrc = tbuf2 - ker_x_n;                                     \
            }                                                               \
            else                                                            \
            {                                                               \
                /* convolve center line for cross-shaped element */         \
                tsrc = rows[ker_y] - ker_x_n;                               \
                tdst = tbuf;                                                \
            }                                                               \
                                                                            \
            if( ker_width > 1 && (!is_cross || crows == ker_height) )       \
            {                                                               \
                /* make replication borders */                              \
                for( i = ker_x_n - 1; i >= 0; i-- )                         \
                    tsrc[i] = tsrc[i + channels];                           \
                for( i = width_n + ker_x_n; i < width_n + ker_width_n; i++ )\
                    tsrc[i] = tsrc[i - channels];                           \
                                                                            \
                /* row processing */                                        \
                if( channels == 1 )                                         \
                    for( i = 0; i < width_n; i += 2 )                       \
                    {                                                       \
                        int j;                                              \
                        int t, t0 = tsrc[i + 1];                            \
                                                                            \
                        for( j = 2; j < ker_width_n; j++ )                  \
                        {                                                   \
                            int t1 = tsrc[i + j];                           \
                            update_extr_macro( t0, t1 );                    \
                        }                                                   \
                                                                            \
                        t = tsrc[i];                                        \
                        update_extr_macro( t, t0 );                         \
                        tdst[i] = (int) t;                                  \
                                                                            \
                        t = tsrc[i + j];                                    \
                        update_extr_macro( t, t0 );                         \
                        tdst[i + 1] = (int) t;                              \
                    }                                                       \
                else                                                        \
                    for( i = 0; i < width_n; i++ )                          \
                    {                                                       \
                        int j;                                              \
                        int t0 = tsrc[i];                                   \
                                                                            \
                        for( j = channels; j < ker_width_n; j += channels ) \
                        {                                                   \
                            int t1 = tsrc[i + j];                           \
                            update_extr_macro( t0, t1 );                    \
                        }                                                   \
                                                                            \
                        tdst[i] = (int) t0;                                 \
                    }                                                       \
            }                                                               \
            else                                                            \
                CV_COPY( tdst, tsrc + ker_x_n, width_n, x );                \
                                                                            \
            if( crows < ker_height )                                        \
                src += srcstep;                                             \
        }                                                                   \
                                                                            \
        if( starting_flag )                                                 \
        {                                                                   \
            starting_flag = 0;                                              \
            tsrc = rows[ker_y];                                             \
                                                                            \
            for( i = 0; i < ker_y; i++ )                                    \
            {                                                               \
                tdst = rows[i];                                             \
                CV_COPY( tdst, tsrc, width_n, x );                          \
            }                                                               \
        }                                                                   \
                                                                            \
        /* vertical convolution */                                          \
        if( crows != ker_height )                                           \
        {                                                                   \
            if( crows < ker_height )                                        \
                break;                                                      \
            /* else it is cross-shaped element: change central line */      \
            rows[ker_y] = tbuf;                                             \
            crows--;                                                        \
        }                                                                   \
                                                                            \
        tdst = (int *) dst;                                                 \
                                                                            \
        if( width_rest )                                                    \
        {                                                                   \
            need_copy = width_n < CV_MORPH_ALIGN || y == dst_height - 1;    \
                                                                            \
            if( need_copy )                                                 \
                tdst = tbuf;                                                \
            else                                                            \
                CV_COPY(tbuf + width_n, tdst + width_n, CV_MORPH_ALIGN, x); \
        }                                                                   \
                                                                            \
        for( x = 0; x < width_n; x += 4 )                                   \
        {                                                                   \
            int val0, val1, val2, val3;                                     \
                                                                            \
            tsrc = rows[0];                                                 \
                                                                            \
            val0 = tsrc[x];                                                 \
            val1 = tsrc[x + 1];                                             \
            val2 = tsrc[x + 2];                                             \
            val3 = tsrc[x + 3];                                             \
                                                                            \
            for( i = 1; i < ker_height; i++ )                               \
            {                                                               \
                tsrc = rows[i];                                             \
                int s = tsrc[x + 0];                                        \
                update_extr_macro( val0, s );                               \
                s = tsrc[x + 1];                                            \
                update_extr_macro( val1, s );                               \
                s = tsrc[x + 2];                                            \
                update_extr_macro( val2, s );                               \
                s = tsrc[x + 3];                                            \
                update_extr_macro( val3, s );                               \
            }                                                               \
            tdst[x + 0] = CV_TOGGLE_FLT( val0 );                            \
            tdst[x + 1] = CV_TOGGLE_FLT( val1 );                            \
            tdst[x + 2] = CV_TOGGLE_FLT( val2 );                            \
            tdst[x + 3] = CV_TOGGLE_FLT( val3 );                            \
        }                                                                   \
                                                                            \
        if( width_rest )                                                    \
        {                                                                   \
            if( need_copy )                                                 \
                CV_COPY( (int *) dst, tbuf, width_n, x );                   \
            else                                                            \
                CV_COPY(tdst + width_n, tbuf + width_n, CV_MORPH_ALIGN, x); \
        }                                                                   \
                                                                            \
        rows[ker_y] = saved_row;                                            \
                                                                            \
        /* rotate buffer */                                                 \
        {                                                                   \
            int *t = rows[0];                                               \
                                                                            \
            CV_COPY( rows, rows + 1, ker_height - 1, i );                   \
            rows[i] = t;                                                    \
            crows--;                                                        \
            dst += dststep;                                                 \
        }                                                                   \
    }                                                                       \
    while( ++y < dst_height );                                              \
                                                                            \
    roi->height = y;                                                        \
    state->crows = crows;                                                   \
                                                                            \
    return CV_OK;                                                           \
}


/****************************************************************************************\
*         Erode/Dilate with arbitrary element and arbitrary type                         *
\****************************************************************************************/

#define ICV_DEF_MORPH_ARB_FUNC( name, flavor, arrtype, init_val,            \
                                update_extr_macro, toggle_macro )           \
static CvStatus                                                             \
icv##name##Arb_##flavor( arrtype* src, int srcstep,                         \
                         arrtype* dst, int dststep,                         \
                         CvSize* roi, CvMorphState* state, int stage )      \
{                                                                           \
    const int INIT_VAL = init_val;                                          \
    int width = roi->width;                                                 \
    int src_height = roi->height;                                           \
    int dst_height = src_height;                                            \
    int x, y = 0, i;                                                        \
                                                                            \
    int ker_x = state->ker_x;                                               \
    int ker_y = state->ker_y;                                               \
    int ker_width = state->ker_width;                                       \
    int ker_height = state->ker_height;                                     \
    uchar *ker_data = state->ker0;                                          \
                                                                            \
    int crows = state->crows;                                               \
    arrtype **rows = (arrtype **) (state->rows);                            \
    arrtype *tbuf = (arrtype *) (state->tbuf);                              \
                                                                            \
    int channels = state->channels;                                         \
    int ker_x_n = ker_x * channels;                                         \
    int ker_width_n = ker_width * channels;                                 \
    int width_n = width * channels;                                         \
                                                                            \
    int starting_flag = 0;                                                  \
    int width_rest = width_n & (CV_MORPH_ALIGN - 1);                        \
    arrtype **ker_ptr, **ker = (arrtype**)cvStackAlloc(                     \
        ker_width*ker_height*sizeof(ker[0]) );                              \
                                                                            \
    srcstep /= sizeof(src[0]);                                              \
    dststep /= sizeof(dst[0]);                                              \
                                                                            \
    /* initialize cyclic buffer when starting */                            \
    if( stage == CV_WHOLE || stage == CV_START )                            \
    {                                                                       \
        for( i = 0; i < ker_height; i++ )                                   \
            rows[i] = (arrtype *) (state->buffer + state->buffer_step * i); \
        crows = ker_y;                                                      \
        if( stage != CV_WHOLE )                                             \
            dst_height -= ker_height - ker_y - 1;                           \
        starting_flag = 1;                                                  \
    }                                                                       \
                                                                            \
    if( stage == CV_END )                                                   \
        dst_height += ker_height - ker_y - 1;                               \
                                                                            \
    do                                                                      \
    {                                                                       \
        arrtype *tsrc, *tdst;                                               \
        int need_copy = 0;                                                  \
                                                                            \
        /* fill cyclic buffer - horizontal filtering */                     \
        for( ; crows < ker_height; crows++ )                                \
        {                                                                   \
            tsrc = src;                                                     \
            tdst = rows[crows];                                             \
                                                                            \
            if( src_height-- <= 0 )                                         \
            {                                                               \
                if( stage != CV_WHOLE && stage != CV_END )                  \
                    break;                                                  \
                /* duplicate last row */                                    \
                tsrc = rows[crows - 1];                                     \
                CV_COPY( tdst, tsrc, width_n + ker_width_n, x );            \
                continue;                                                   \
            }                                                               \
                                                                            \
            src += srcstep;                                                 \
            for( x = 0; x < width_n; x++ )                                  \
            {                                                               \
                arrtype t = tsrc[x];                                        \
                tdst[ker_x_n + x] = toggle_macro(t);                        \
            }                                                               \
                                                                            \
            /* make replication borders */                                  \
            for( i = ker_x_n - 1; i >= 0; i-- )                             \
                tdst[i] = tdst[i + channels];                               \
            for( i = width_n + ker_x_n; i < width_n + ker_width_n; i++ )    \
                tdst[i] = tdst[i - channels];                               \
        }                                                                   \
                                                                            \
        if( starting_flag )                                                 \
        {                                                                   \
            starting_flag = 0;                                              \
            tsrc = rows[ker_y];                                             \
                                                                            \
            for( i = 0; i < ker_y; i++ )                                    \
            {                                                               \
                tdst = rows[i];                                             \
                CV_COPY( tdst, tsrc, width_n + ker_width_n, x );            \
            }                                                               \
        }                                                                   \
                                                                            \
        if( crows < ker_height )                                            \
            break;                                                          \
                                                                            \
        tdst = dst;                                                         \
        if( width_rest )                                                    \
        {                                                                   \
            need_copy = width_n < CV_MORPH_ALIGN || y == dst_height - 1;    \
                                                                            \
            if( need_copy )                                                 \
                tdst = tbuf;                                                \
            else                                                            \
                CV_COPY( tbuf + width_n, dst + width_n, CV_MORPH_ALIGN, x );\
        }                                                                   \
                                                                            \
        ker_ptr = ker;                                                      \
        for( i = 0; i < ker_height; i++ )                                   \
            for( x = 0; x < ker_width; x++ )                                \
            {                                                               \
                if( ker_data[i*ker_width + x] )                             \
                    *ker_ptr++ = rows[i] + x*channels;                      \
            }                                                               \
                                                                            \
        if( channels == 3 )                                                 \
            for( x = 0; x < width_n; x += 3 )                               \
            {                                                               \
                int val0 = INIT_VAL, val1 = INIT_VAL, val2 = INIT_VAL;      \
                arrtype** kp = ker;                                         \
                while( kp != ker_ptr )                                      \
                {                                                           \
                    arrtype* tp = *kp++;                                    \
                    int t = tp[x];                                          \
                    update_extr_macro( val0, t );                           \
                    t = tp[x+1];                                            \
                    update_extr_macro( val1, t );                           \
                    t = tp[x+2];                                            \
                    update_extr_macro( val2, t );                           \
                }                                                           \
                tdst[x] = (arrtype)toggle_macro(val0);                      \
                tdst[x + 1] = (arrtype)toggle_macro(val1);                  \
                tdst[x + 2] = (arrtype)toggle_macro(val2);                  \
            }                                                               \
        else                                                                \
            /* channels == 1 or channels == 4 */                            \
            for( x = 0; x < width_n; x += 4 )                               \
            {                                                               \
                int val0 = INIT_VAL, val1 = INIT_VAL,                       \
                    val2 = INIT_VAL, val3 = INIT_VAL;                       \
                arrtype** kp = ker;                                         \
                while( kp != ker_ptr )                                      \
                {                                                           \
                    arrtype* tp = *kp++;                                    \
                    int t = tp[x];                                          \
                    update_extr_macro( val0, t );                           \
                    t = tp[x+1];                                            \
                    update_extr_macro( val1, t );                           \
                    t = tp[x+2];                                            \
                    update_extr_macro( val2, t );                           \
                    t = tp[x+3];                                            \
                    update_extr_macro( val3, t );                           \
                }                                                           \
                                                                            \
                tdst[x] = (arrtype)toggle_macro(val0);                      \
                tdst[x + 1] = (arrtype)toggle_macro(val1);                  \
                tdst[x + 2] = (arrtype)toggle_macro(val2);                  \
                tdst[x + 3] = (arrtype)toggle_macro(val3);                  \
            }                                                               \
                                                                            \
        if( width_rest )                                                    \
        {                                                                   \
            if( need_copy )                                                 \
                CV_COPY( dst, tbuf, width_n, x );                           \
            else                                                            \
                CV_COPY( dst + width_n, tbuf + width_n, CV_MORPH_ALIGN, x );\
        }                                                                   \
                                                                            \
        /* rotate buffer */                                                 \
        {                                                                   \
            arrtype *t = rows[0];                                           \
                                                                            \
            CV_COPY( rows, rows + 1, ker_height - 1, i );                   \
            rows[i] = t;                                                    \
            crows--;                                                        \
            dst += dststep;                                                 \
        }                                                                   \
    }                                                                       \
    while( ++y < dst_height );                                              \
                                                                            \
    roi->height = y;                                                        \
    state->crows = crows;                                                   \
                                                                            \
    return CV_OK;                                                           \
}


ICV_DEF_MORPH_RECT_INT_FUNC( Erode, 8u, uchar, CV_CALC_MIN )
ICV_DEF_MORPH_RECT_INT_FUNC( Dilate, 8u, uchar, CV_CALC_MAX )
ICV_DEF_MORPH_RECT_FLT_FUNC( Erode, 32f, int, CV_CALC_MIN )
ICV_DEF_MORPH_RECT_FLT_FUNC( Dilate, 32f, int, CV_CALC_MAX )

ICV_DEF_MORPH_ARB_FUNC( Erode, 8u, uchar, 255, CV_CALC_MIN, CV_NOP )
ICV_DEF_MORPH_ARB_FUNC( Dilate, 8u, uchar, 0, CV_CALC_MAX, CV_NOP )
ICV_DEF_MORPH_ARB_FUNC( Erode, 32f, int, INT_MAX, CV_CALC_MIN, CV_TOGGLE_FLT )
ICV_DEF_MORPH_ARB_FUNC( Dilate, 32f, int, INT_MIN, CV_CALC_MAX, CV_TOGGLE_FLT )

static CvStatus
icvCheckMorphArgs( const void *pSrc, int srcstep,
                   void *pDst, int dststep,
                   CvSize * roi,
                   CvMorphState * state,
                   int stage, int shape, CvDataType dataType, int channels )
{
    int bt_pix = channels * (dataType != cv32f ? 1 : 4);

    if( !pSrc || !pDst || !state || !roi )
        return CV_NULLPTR_ERR;

    if( roi->width <= 0 || roi->width > state->max_width || roi->height < 0 )
        return CV_BADSIZE_ERR;

    if( state->dataType != dataType || state->channels != channels )
        return CV_UNMATCHED_FORMATS_ERR;

    if( ICV_BINARY_KERNEL_SHAPE(state->kerType) != shape )
        return CV_UNMATCHED_FORMATS_ERR;

    if( roi->width * bt_pix > srcstep || roi->width * bt_pix > dststep )
        return CV_BADSIZE_ERR;

    if( stage != CV_WHOLE && stage != CV_MIDDLE && stage != CV_START && stage != CV_END )
        return CV_BADRANGE_ERR;

    if( state->crows == 0 && stage > CV_START || roi->height == 0 && stage != CV_END )
    {
        roi->height = 0;
        return ( CvStatus ) 1;
    }

    return CV_OK;
}

/****************************************************************************************\
                              Internal IPP-like functions
\****************************************************************************************/

#define ICV_DEF_MORPH_FUNC( extname, intname, flavor, cn, arrtype, shape )          \
IPCVAPI_IMPL( CvStatus,                                                             \
icv##extname##_##flavor##_C##cn##R,( const arrtype* pSrc, int srcstep,              \
                                     arrtype* pDst, int dststep,                    \
                                     CvSize* roi,                                   \
                                     CvMorphState * state, int stage ),             \
                                     (pSrc, srcstep, pDst, dststep, roi, state, stage) )\
{                                                                                   \
    CvStatus status = icvCheckMorphArgs( pSrc, srcstep, pDst, dststep, roi,         \
                                         state, stage, shape, cv##flavor, cn );     \
                                                                                    \
    if( status == CV_OK )                                                           \
    {                                                                               \
        status = icv##intname##_##flavor( (arrtype*)pSrc, srcstep, pDst, dststep,   \
                                          roi, state, stage );                      \
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

ICV_DEF_MORPH_FUNC( ErodeStrip_Rect, ErodeRC, 32f, 1, int, CV_SHAPE_RECT )
ICV_DEF_MORPH_FUNC( ErodeStrip_Rect, ErodeRC, 32f, 3, int, CV_SHAPE_RECT )
ICV_DEF_MORPH_FUNC( ErodeStrip_Rect, ErodeRC, 32f, 4, int, CV_SHAPE_RECT )

ICV_DEF_MORPH_FUNC( ErodeStrip_Cross, ErodeRC, 32f, 1, int, CV_SHAPE_CROSS )
ICV_DEF_MORPH_FUNC( ErodeStrip_Cross, ErodeRC, 32f, 3, int, CV_SHAPE_CROSS )
ICV_DEF_MORPH_FUNC( ErodeStrip_Cross, ErodeRC, 32f, 4, int, CV_SHAPE_CROSS )

ICV_DEF_MORPH_FUNC( ErodeStrip, ErodeArb, 32f, 1, int, CV_SHAPE_CUSTOM )
ICV_DEF_MORPH_FUNC( ErodeStrip, ErodeArb, 32f, 3, int, CV_SHAPE_CUSTOM )
ICV_DEF_MORPH_FUNC( ErodeStrip, ErodeArb, 32f, 4, int, CV_SHAPE_CUSTOM )

ICV_DEF_MORPH_FUNC( DilateStrip_Rect, DilateRC, 8u, 1, uchar, CV_SHAPE_RECT )
ICV_DEF_MORPH_FUNC( DilateStrip_Rect, DilateRC, 8u, 3, uchar, CV_SHAPE_RECT )
ICV_DEF_MORPH_FUNC( DilateStrip_Rect, DilateRC, 8u, 4, uchar, CV_SHAPE_RECT )

ICV_DEF_MORPH_FUNC( DilateStrip_Cross, DilateRC, 8u, 1, uchar, CV_SHAPE_CROSS )
ICV_DEF_MORPH_FUNC( DilateStrip_Cross, DilateRC, 8u, 3, uchar, CV_SHAPE_CROSS )
ICV_DEF_MORPH_FUNC( DilateStrip_Cross, DilateRC, 8u, 4, uchar, CV_SHAPE_CROSS )

ICV_DEF_MORPH_FUNC( DilateStrip, DilateArb, 8u, 1, uchar, CV_SHAPE_CUSTOM )
ICV_DEF_MORPH_FUNC( DilateStrip, DilateArb, 8u, 3, uchar, CV_SHAPE_CUSTOM )
ICV_DEF_MORPH_FUNC( DilateStrip, DilateArb, 8u, 4, uchar, CV_SHAPE_CUSTOM )

ICV_DEF_MORPH_FUNC( DilateStrip_Rect, DilateRC, 32f, 1, int, CV_SHAPE_RECT )
ICV_DEF_MORPH_FUNC( DilateStrip_Rect, DilateRC, 32f, 3, int, CV_SHAPE_RECT )
ICV_DEF_MORPH_FUNC( DilateStrip_Rect, DilateRC, 32f, 4, int, CV_SHAPE_RECT )

ICV_DEF_MORPH_FUNC( DilateStrip_Cross, DilateRC, 32f, 1, int, CV_SHAPE_CROSS )
ICV_DEF_MORPH_FUNC( DilateStrip_Cross, DilateRC, 32f, 3, int, CV_SHAPE_CROSS )
ICV_DEF_MORPH_FUNC( DilateStrip_Cross, DilateRC, 32f, 4, int, CV_SHAPE_CROSS )

ICV_DEF_MORPH_FUNC( DilateStrip, DilateArb, 32f, 1, int, CV_SHAPE_CUSTOM )
ICV_DEF_MORPH_FUNC( DilateStrip, DilateArb, 32f, 3, int, CV_SHAPE_CUSTOM )
ICV_DEF_MORPH_FUNC( DilateStrip, DilateArb, 32f, 4, int, CV_SHAPE_CUSTOM )



/////////////////////////////////// External Interface /////////////////////////////////////


CV_IMPL IplConvKernel *
cvCreateStructuringElementEx( int cols, int rows,
                              int anchorX, int anchorY,
                              int shape, int *values )
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

    element_size = cvAlign(element_size,32);
    element = (IplConvKernel *)cvAlloc( element_size );
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
                int dx = cvRound( c * sqrt( ((double)r * r - y * y) * inv_r2 ));
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
        cvFree( (void**)&element );
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
    cvFree( (void**)element );

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
    CvMat* temp = 0;

    CV_FUNCNAME( "icvMorphOp" );

    __BEGIN__;

    CvMorphFunc func = 0;
    int shape;
    int i, coi1 = 0, coi2 = 0;
    CvMat srcstub, *src = (CvMat*)srcarr;
    CvMat dststub, *dst = (CvMat*)dstarr;
    CvSize size;
    int type;

    if( !inittab )
    {
        icvInitMorphologyTab( morph_tab + 0, morph_tab + 1, morph_tab + 2,
                              morph_tab + 3, morph_tab + 4, morph_tab + 5 );
        inittab = 1;
    }

    if( iterations <= 0 )
        CV_CALL( cvCopy( src, dst ));

    CV_CALL( src = cvGetMat( src, &srcstub, &coi1 ));
    
    if( src != &srcstub )
    {
        srcstub = *src;
        src = &srcstub;
    }

    if( dstarr == srcarr )
        dst = src;
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
        CvSize element_size = { element->nCols, element->nRows };
        CvPoint element_anchor = { element->anchorX, element->anchorY };
        IPPI_CALL(
            icvMorphologyInitAlloc( src->width, CV_MAT_DEPTH(type) == CV_8U ? cv8u : cv32f,
                                 CV_MAT_CN(type), element_size, element_anchor,
                                 (int) (element->nShiftR), element->values,
                                 &state ));
        shape = (int)(element->nShiftR);
        shape = shape < CV_SHAPE_ELLIPSE ? shape : CV_SHAPE_CUSTOM;
    }
    else
    {
        CvSize element_size = { 1+iterations*2, 1+iterations*2 };
        CvPoint element_anchor = { iterations, iterations };
        IPPI_CALL(
            icvMorphologyInitAlloc( src->width, CV_MAT_DEPTH(type) == CV_8U ? cv8u : cv32f,
                                  CV_MAT_CN(type), element_size, element_anchor,
                                  CV_SHAPE_RECT, 0, &state ));
        iterations = 1;
        shape = CV_SHAPE_RECT;
    }

    func = (CvMorphFunc)(morph_tab[(shape == CV_SHAPE_RECT ? 0 :
                         shape == CV_SHAPE_CROSS ? 1 : 2) * 2 + mop].fn_2d[type]);

    if( !func )
        CV_ERROR( CV_StsUnsupportedFormat, "" );

    size = cvGetMatSize( src );

    for( i = 0; i < iterations; i++ )
    {
        int src_step = src->step, dst_step = dst->step;
        if( src_step == 0 )
            src_step = dst_step = size.width * CV_ELEM_SIZE(type);

        IPPI_CALL( func( src->data.ptr, src_step, dst->data.ptr,
                         dst_step, &size, state, 0 ));
        src = dst;
    }

    __END__;

    cvReleaseMat( &temp );
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
                void* temp, IplConvKernel* element, int op, int iterations )
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
        if( src != dst )
            temp = dst;
        CV_CALL( cvErode( src, temp, element, iterations ));
        CV_CALL( cvDilate( temp, temp, element, iterations ));
        CV_CALL( cvSub( src, temp, dst ));
        break;
    case CV_MOP_BLACKHAT:
        if( src != dst )
            temp = dst;
        CV_CALL( cvDilate( src, temp, element, iterations ));
        CV_CALL( cvErode( temp, temp, element, iterations ));
        CV_CALL( cvSub( temp, src, dst ));
        break;
    default:
        CV_ERROR( CV_StsBadArg, "unknown morphological operation" );
    }

    __END__;
}

/* End of file. */
