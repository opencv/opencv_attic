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
#include <stdio.h>

#define IPCV_MORPHOLOGY_PTRS( morphtype, flavor )                                   \
icv##morphtype##Rect_##flavor##_C1R_t icv##morphtype##Rect_##flavor##_C1R_p = 0;    \
icv##morphtype##Rect_##flavor##_C3R_t icv##morphtype##Rect_##flavor##_C3R_p = 0;    \
icv##morphtype##Rect_##flavor##_C4R_t icv##morphtype##Rect_##flavor##_C4R_p = 0;    \
icv##morphtype##Any_##flavor##_C1R_t icv##morphtype##Any_##flavor##_C1R_p = 0;      \
icv##morphtype##Any_##flavor##_C3R_t icv##morphtype##Any_##flavor##_C3R_p = 0;      \
icv##morphtype##Any_##flavor##_C4R_t icv##morphtype##Any_##flavor##_C4R_p = 0;

IPCV_MORPHOLOGY_PTRS( Erode, 8u )
IPCV_MORPHOLOGY_PTRS( Erode, 32f )
IPCV_MORPHOLOGY_PTRS( Dilate, 8u )
IPCV_MORPHOLOGY_PTRS( Dilate, 32f )


static CvFilterState* CV_STDCALL
icvMorphologyInitAlloc( int roiWidth, CvDataType dataType, int channels,
                        CvSize elSize, CvPoint elAnchor, int elShape, int* elData )
{
    CvFilterState* morphState = 0;

    CV_FUNCNAME( "icvMorphologyInitAlloc" );

    __BEGIN__;
    
    switch( elShape )
    {
    case CV_SHAPE_RECT:
        break;
    case CV_SHAPE_CROSS:
    case CV_SHAPE_ELLIPSE:
    case CV_SHAPE_CUSTOM:
        if( elData == 0 )
            CV_ERROR( CV_StsNullPtr,
            "For non-rectangular strucuring element coefficient should be specified" );
        break;
    default:
        CV_ERROR( CV_StsBadFlag, "Unknown/unsupported type of structuring element" );
    }

    morphState = icvFilterInitAlloc( roiWidth, dataType, channels,
        cvSize(elSize.width, elSize.height + (elShape == CV_SHAPE_RECT)),
        elAnchor, elShape != CV_SHAPE_RECT ? elData : 0,
        ICV_MAKE_BINARY_KERNEL(elShape) );

    if( morphState )
        morphState->ker_height = elSize.height;

    __END__;

    return morphState;
}


static void CV_STDCALL
icvMorphologyFree( CvFilterState** morphState )
{
    icvFilterFree( morphState );
}


/****************************************************************************************\
*                 Erode/Dilate with rectangular element for integer types                *
\****************************************************************************************/

#define ICV_DEF_MORPH_RECT_INT_FUNC( name, flavor, arrtype,                 \
                                     update_extr_macro )                    \
static CvStatus CV_STDCALL                                                  \
icv##name##Rect_##flavor( arrtype* src, int srcstep, arrtype* dst, int dststep,\
                          CvSize* roi, CvFilterState* state, int /*stage*/ ) \
{                                                                           \
    int src_height = roi->height;                                           \
    int dst_height = src_height;                                            \
    int x, y = 0, dy = 0, i;                                                \
                                                                            \
    int row_count, rows_wanted;                                             \
    arrtype** rows = (arrtype**)(state->rows);                              \
    arrtype *tbuf = (arrtype*)(state->tbuf), *tdst, *tsrc;                  \
                                                                            \
    int channels = state->channels;                                         \
    int ker_x_n = state->ker_x * channels;                                  \
    int ker_width_n = state->ker_width * channels;                          \
    int ker_y = state->ker_y;                                               \
    int ker_height = state->ker_height;                                     \
    int width_n = roi->width * channels;                                    \
                                                                            \
    /* initialize cyclic buffer when starting */                            \
    for( i = 0; i <= ker_height; i++ )                                      \
        rows[i] = (arrtype*)(state->buffer + state->buffer_step * i);       \
    row_count = 0;                                                          \
    rows_wanted = ker_height - ker_y + 1;                                   \
                                                                            \
    srcstep /= sizeof(src[0]);                                              \
    dststep /= sizeof(dst[0]);                                              \
                                                                            \
    for( y = 0; y < dst_height; y += dy )                                   \
    {                                                                       \
        int val0, val1, val2, val3, t;                                      \
        dy = MIN( dst_height - y, 2 );                                      \
                                                                            \
        /* fill cyclic buffer - apply horizontal filter */                  \
        for( ; row_count < rows_wanted; row_count++, src += srcstep )       \
        {                                                                   \
            tdst = rows[row_count];                                         \
            if( ker_height == 1 )                                           \
            {                                                               \
                tdst = dst;                                                 \
                dst += dststep;                                             \
            }                                                               \
                                                                            \
            if( src_height-- <= 0 )                                         \
                break;                                                      \
                                                                            \
            if( ker_width_n == channels )                                   \
            {                                                               \
                CV_COPY( tdst, src, width_n, x );                           \
                continue;                                                   \
            }                                                               \
                                                                            \
            CV_COPY( tbuf + ker_x_n, src, width_n, x );                     \
                                                                            \
            /* make replication borders */                                  \
            for( i = ker_x_n - 1; i >= 0; i-- )                             \
                tbuf[i] = tbuf[i + channels];                               \
            for( i = width_n + ker_x_n; i < width_n + ker_width_n; i++ )    \
                tbuf[i] = tbuf[i - channels];                               \
                                                                            \
            /* row processing */                                            \
            if( channels == 1 )                                             \
            {                                                               \
                for( x = 0; x <= width_n - 2; x += 2 )                      \
                {                                                           \
                    tsrc = tbuf + x;                                        \
                    val0 = tsrc[1];                                         \
                                                                            \
                    for( i = 2; i < ker_width_n; i++ )                      \
                    {                                                       \
                        t = tsrc[i]; update_extr_macro( val0, t );          \
                    }                                                       \
                    t = tsrc[0]; update_extr_macro( t, val0 ); tdst[x] = (arrtype)t;\
                    t = tsrc[i]; update_extr_macro( t, val0 ); tdst[x+1] = (arrtype)t;\
                }                                                           \
            }                                                               \
            else if( channels == 3 )                                        \
            {                                                               \
                for( x = 0; x <= width_n - 6; x += 6 )                      \
                {                                                           \
                    tsrc = tbuf + x;                                        \
                    val0 = tsrc[3]; val1 = tsrc[4]; val2 = tsrc[5];         \
                                                                            \
                    for( i = 6; i < ker_width_n; i += 3 )                   \
                    {                                                       \
                        t = tsrc[i]; update_extr_macro( val0, t );          \
                        t = tsrc[i+1]; update_extr_macro( val1, t );        \
                        t = tsrc[i+2]; update_extr_macro( val2, t );        \
                    }                                                       \
                    t = tsrc[0]; update_extr_macro( t, val0 ); tdst[x] = (arrtype)t;\
                    t = tsrc[i]; update_extr_macro( t, val0 ); tdst[x+3] = (arrtype)t;\
                                                                            \
                    t = tsrc[1]; update_extr_macro( t, val1 ); tdst[x+1] = (arrtype)t;\
                    t = tsrc[i+1]; update_extr_macro( t, val1 ); tdst[x+4] = (arrtype)t;\
                                                                            \
                    t = tsrc[2]; update_extr_macro( t, val2 ); tdst[x+2] = (arrtype)t;\
                    t = tsrc[i+2]; update_extr_macro( t, val2 ); tdst[x+5] = (arrtype)t;\
                }                                                           \
            }                                                               \
            else                                                            \
            {                                                               \
                assert( channels == 4 );                                    \
                for( x = 0; x <= width_n - 8; x += 8 )                      \
                {                                                           \
                    tsrc = tbuf + x;                                        \
                    val0 = tsrc[4]; val1 = tsrc[5];                         \
                    val2 = tsrc[6]; val3 = tsrc[7];                         \
                                                                            \
                    for( i = 8; i < ker_width_n; i += 4 )                   \
                    {                                                       \
                        t = tsrc[i]; update_extr_macro( val0, t );          \
                        t = tsrc[i+1]; update_extr_macro( val1, t );        \
                        t = tsrc[i+2]; update_extr_macro( val2, t );        \
                        t = tsrc[i+3]; update_extr_macro( val3, t );        \
                    }                                                       \
                    t = tsrc[0]; update_extr_macro( t, val0 ); tdst[x] = (arrtype)t;\
                    t = tsrc[i]; update_extr_macro( t, val0 ); tdst[x+4] = (arrtype)t;\
                                                                            \
                    t = tsrc[1]; update_extr_macro( t, val1 ); tdst[x+1] = (arrtype)t;\
                    t = tsrc[i+1]; update_extr_macro( t, val1 ); tdst[x+5] = (arrtype)t;\
                                                                            \
                    t = tsrc[2]; update_extr_macro( t, val2 ); tdst[x+2] = (arrtype)t;\
                    t = tsrc[i+2]; update_extr_macro( t, val2 ); tdst[x+6] = (arrtype)t;\
                                                                            \
                    t = tsrc[3]; update_extr_macro( t, val3 ); tdst[x+3] = (arrtype)t;\
                    t = tsrc[i+3]; update_extr_macro( t, val3 ); tdst[x+7] = (arrtype)t;\
                }                                                           \
            }                                                               \
                                                                            \
            for( ; x < width_n; x++ )                                       \
            {                                                               \
                val0 = tbuf[x];                                             \
                for( i = channels; i < ker_width_n; i += channels )         \
                {                                                           \
                    t = tbuf[x + i]; update_extr_macro( val0, t );          \
                }                                                           \
                tdst[x] = (arrtype)val0;                                    \
            }                                                               \
        }                                                                   \
                                                                            \
        /* apply vertical filter */                                         \
        if( ker_height == 1 )                                               \
        {                                                                   \
            row_count -= 2;                                                 \
            continue;                                                       \
        }                                                                   \
                                                                            \
        if( dy == 2 )                                                       \
        {                                                                   \
            int second_row = MIN((int)(y >= ker_y), row_count-1);           \
            int last_row = row_count - (src_height >= 0);                   \
                                                                            \
            for( x = 0; x <= width_n - 4; x += 4, dst += 4 )                \
            {                                                               \
                tsrc = rows[second_row] + x;                                \
                val0 = tsrc[0]; val1 = tsrc[1];                             \
                val2 = tsrc[2]; val3 = tsrc[3];                             \
                                                                            \
                for( i = second_row + 1; i < last_row; i++ )                \
                {                                                           \
                    tsrc = rows[i] + x;                                     \
                    t = tsrc[0]; update_extr_macro(val0, t);                \
                    t = tsrc[1]; update_extr_macro(val1, t);                \
                    t = tsrc[2]; update_extr_macro(val2, t);                \
                    t = tsrc[3]; update_extr_macro(val3, t);                \
                }                                                           \
                                                                            \
                tsrc = rows[0] + x;                                         \
                t = tsrc[0]; update_extr_macro(t, val0); dst[0] = (arrtype)t;\
                t = tsrc[1]; update_extr_macro(t, val1); dst[1] = (arrtype)t;\
                t = tsrc[2]; update_extr_macro(t, val2); dst[2] = (arrtype)t;\
                t = tsrc[3]; update_extr_macro(t, val3); dst[3] = (arrtype)t;\
                                                                            \
                tsrc = rows[row_count - 1] + x;                             \
                t = tsrc[0]; update_extr_macro(t, val0); dst[dststep] = (arrtype)t;\
                t = tsrc[1]; update_extr_macro(t, val1); dst[dststep+1] = (arrtype)t;\
                t = tsrc[2]; update_extr_macro(t, val2); dst[dststep+2] = (arrtype)t;\
                t = tsrc[3]; update_extr_macro(t, val3); dst[dststep+3] = (arrtype)t;\
            }                                                               \
                                                                            \
            for( ; x < width_n; x++, dst++ )                                \
            {                                                               \
                tsrc = rows[second_row] + x;                                \
                val0 = tsrc[0];                                             \
                                                                            \
                for( i = second_row + 1; i < last_row; i++ )                \
                {                                                           \
                    tsrc = rows[i] + x;                                     \
                    t = tsrc[0]; update_extr_macro(val0, t);                \
                }                                                           \
                                                                            \
                tsrc = rows[0] + x;                                         \
                t = tsrc[0]; update_extr_macro(t, val0); dst[0] = (arrtype)t;\
                tsrc = rows[row_count - 1] + x;                             \
                t = tsrc[0]; update_extr_macro(t, val0); dst[dststep] = (arrtype)t;\
            }                                                               \
            dst -= width_n;                                                 \
        }                                                                   \
        else                                                                \
        {                                                                   \
            for( x = 0; x < width_n; x++ )                                  \
            {                                                               \
                tsrc = rows[0] + x;                                         \
                val0 = tsrc[0];                                             \
                                                                            \
                for( i = 1; i < row_count; i++ )                            \
                {                                                           \
                    tsrc = rows[i] + x;                                     \
                    t = tsrc[0]; update_extr_macro(val0, t);                \
                }                                                           \
                                                                            \
                dst[x] = (arrtype)val0;                                     \
            }                                                               \
        }                                                                   \
                                                                            \
        if( y+dy > ker_y && row_count > 1 )                                 \
        {                                                                   \
            /* rotate buffer */                                             \
            int shift = dy - (y < ker_y);                                   \
            arrtype* row0 = rows[0];                                        \
            arrtype* row1 = rows[1];                                        \
            if( row_count <= shift && src_height <= 0 )                     \
                shift = 1;                                                  \
                                                                            \
            CV_COPY( rows, rows + shift, ker_height + 1 - shift, i );       \
                                                                            \
            if( shift == 2 )                                                \
            {                                                               \
                rows[ker_height - 1] = row0;                                \
                rows[ker_height] = row1;                                    \
            }                                                               \
            else                                                            \
                rows[ker_height] = row0;                                    \
            row_count -= shift;                                             \
        }                                                                   \
                                                                            \
        rows_wanted = MIN(rows_wanted + 2, ker_height + 1);                 \
        dst += dststep*dy;                                                  \
    }                                                                       \
                                                                            \
    return CV_OK;                                                           \
}


/****************************************************************************************\
*                    Erode/Dilate with rectangular element for 32f type                  *
\****************************************************************************************/

#define ICV_DEF_MORPH_RECT_FLT_FUNC( name, update_extr_macro )              \
static CvStatus CV_STDCALL                                                  \
icv##name##Rect_32f( int* src, int srcstep, int* dst, int dststep,          \
                     CvSize* roi, CvFilterState* state, int /*stage*/ )      \
{                                                                           \
    int src_height = roi->height;                                           \
    int dst_height = src_height;                                            \
    int x, y = 0, dy = 0, i;                                                \
                                                                            \
    int row_count, rows_wanted;                                             \
    int** rows = (int**)(state->rows);                                      \
    int *tbuf = (int*)(state->tbuf), *tdst, *tsrc;                          \
                                                                            \
    int channels = state->channels;                                         \
    int ker_x_n = state->ker_x * channels;                                  \
    int ker_width_n = state->ker_width * channels;                          \
    int ker_y = state->ker_y;                                               \
    int ker_height = state->ker_height;                                     \
    int width_n = roi->width * channels;                                    \
                                                                            \
    /* initialize cyclic buffer when starting */                            \
    for( i = 0; i <= ker_height; i++ )                                      \
        rows[i] = (int*)(state->buffer + state->buffer_step * i);           \
    row_count = 0;                                                          \
    rows_wanted = ker_height - ker_y + 1;                                   \
                                                                            \
    srcstep /= sizeof(src[0]);                                              \
    dststep /= sizeof(dst[0]);                                              \
                                                                            \
    for( y = 0; y < dst_height; y += dy )                                   \
    {                                                                       \
        int val0, val1, val2, val3, t;                                      \
        dy = MIN( dst_height - y, 2 );                                      \
                                                                            \
        /* fill cyclic buffer - apply horizontal filter */                  \
        for( ; row_count < rows_wanted; row_count++, src += srcstep )       \
        {                                                                   \
            tdst = rows[row_count];                                         \
            if( ker_height == 1 )                                           \
            {                                                               \
                tdst = dst;                                                 \
                dst += dststep;                                             \
            }                                                               \
                                                                            \
            if( src_height-- <= 0 )                                         \
                break;                                                      \
                                                                            \
            if( ker_width_n == channels )                                   \
            {                                                               \
                if( ker_height == 1 )                                       \
                {                                                           \
                    CV_COPY( tdst, src, width_n, x );                       \
                }                                                           \
                else                                                        \
                {                                                           \
                    for( x = 0; x < width_n; x++ )                          \
                        t = src[x], tdst[x] = CV_TOGGLE_FLT(t);             \
                }                                                           \
                continue;                                                   \
            }                                                               \
                                                                            \
            for( x = 0; x < width_n; x++ )                                  \
            {                                                               \
                t = src[x];                                                 \
                tbuf[ker_x_n + x] = CV_TOGGLE_FLT(t);                       \
            }                                                               \
                                                                            \
            /* make replication borders */                                  \
            for( i = ker_x_n - 1; i >= 0; i-- )                             \
                tbuf[i] = tbuf[i + channels];                               \
            for( i = width_n + ker_x_n; i < width_n + ker_width_n; i++ )    \
                tbuf[i] = tbuf[i - channels];                               \
                                                                            \
            /* row processing */                                            \
            if( channels == 1 )                                             \
            {                                                               \
                for( x = 0; x <= width_n - 2; x += 2 )                      \
                {                                                           \
                    tsrc = tbuf + x;                                        \
                    val0 = tsrc[1];                                         \
                                                                            \
                    for( i = 2; i < ker_width_n; i++ )                      \
                    {                                                       \
                        t = tsrc[i]; update_extr_macro( val0, t );          \
                    }                                                       \
                    t = tsrc[0]; update_extr_macro( t, val0 ); tdst[x] = t; \
                    t = tsrc[i]; update_extr_macro( t, val0 ); tdst[x+1]= t;\
                }                                                           \
            }                                                               \
            else if( channels == 3 )                                        \
            {                                                               \
                for( x = 0; x <= width_n - 6; x += 6 )                      \
                {                                                           \
                    tsrc = tbuf + x;                                        \
                    val0 = tsrc[3]; val1 = tsrc[4]; val2 = tsrc[5];         \
                                                                            \
                    for( i = 6; i < ker_width_n; i += 3 )                   \
                    {                                                       \
                        t = tsrc[i]; update_extr_macro( val0, t );          \
                        t = tsrc[i+1]; update_extr_macro( val1, t );        \
                        t = tsrc[i+2]; update_extr_macro( val2, t );        \
                    }                                                       \
                    t = tsrc[0]; update_extr_macro( t, val0 ); tdst[x] = t; \
                    t = tsrc[i]; update_extr_macro( t, val0 ); tdst[x+3] = t;\
                                                                            \
                    t = tsrc[1]; update_extr_macro( t, val1 ); tdst[x+1] = t;\
                    t = tsrc[i+1]; update_extr_macro( t, val1 ); tdst[x+4] = t;\
                                                                            \
                    t = tsrc[2]; update_extr_macro( t, val2 ); tdst[x+2] = t;\
                    t = tsrc[i+2]; update_extr_macro( t, val2 ); tdst[x+5] = t;\
                }                                                           \
            }                                                               \
            else                                                            \
            {                                                               \
                assert( channels == 4 );                                    \
                for( x = 0; x <= width_n - 8; x += 8 )                      \
                {                                                           \
                    tsrc = tbuf + x;                                        \
                    val0 = tsrc[4]; val1 = tsrc[5];                         \
                    val2 = tsrc[6]; val3 = tsrc[7];                         \
                                                                            \
                    for( i = 8; i < ker_width_n; i += 4 )                   \
                    {                                                       \
                        t = tsrc[i]; update_extr_macro( val0, t );          \
                        t = tsrc[i+1]; update_extr_macro( val1, t );        \
                        t = tsrc[i+2]; update_extr_macro( val2, t );        \
                        t = tsrc[i+3]; update_extr_macro( val3, t );        \
                    }                                                       \
                    t = tsrc[0]; update_extr_macro( t, val0 ); tdst[x] = t; \
                    t = tsrc[i]; update_extr_macro( t, val0 ); tdst[x+4] = t;\
                                                                            \
                    t = tsrc[1]; update_extr_macro( t, val1 ); tdst[x+1] = t;\
                    t = tsrc[i+1]; update_extr_macro( t, val1 ); tdst[x+5] = t;\
                                                                            \
                    t = tsrc[2]; update_extr_macro( t, val2 ); tdst[x+2] = t;\
                    t = tsrc[i+2]; update_extr_macro( t, val2 ); tdst[x+6] = t;\
                                                                            \
                    t = tsrc[3]; update_extr_macro( t, val3 ); tdst[x+3] = t;\
                    t = tsrc[i+3]; update_extr_macro( t, val3 ); tdst[x+7] = t;\
                }                                                           \
            }                                                               \
                                                                            \
            for( ; x < width_n; x++ )                                       \
            {                                                               \
                val0 = tbuf[x];                                             \
                for( i = channels; i < ker_width_n; i += channels )         \
                {                                                           \
                    t = tbuf[x + i]; update_extr_macro( val0, t );          \
                }                                                           \
                tdst[x] = val0;                                             \
            }                                                               \
                                                                            \
            if( ker_height == 1 )                                           \
            {                                                               \
                for( x = 0; x < width_n; x++ )                              \
                    t = tdst[x], tdst[x] = CV_TOGGLE_FLT(t);                \
            }                                                               \
        }                                                                   \
                                                                            \
        /* apply vertical filter */                                         \
        if( ker_height == 1 )                                               \
        {                                                                   \
            row_count -= 2;                                                 \
            continue;                                                       \
        }                                                                   \
                                                                            \
        if( dy == 2 )                                                       \
        {                                                                   \
            int second_row = MIN((int)(y >= ker_y), row_count-1);           \
            int last_row = row_count - (src_height >= 0);                   \
                                                                            \
            for( x = 0; x <= width_n - 4; x += 4, dst += 4 )                \
            {                                                               \
                tsrc = rows[second_row] + x;                                \
                val0 = tsrc[0]; val1 = tsrc[1];                             \
                val2 = tsrc[2]; val3 = tsrc[3];                             \
                                                                            \
                for( i = second_row + 1; i < last_row; i++ )                \
                {                                                           \
                    tsrc = rows[i] + x;                                     \
                    t = tsrc[0]; update_extr_macro(val0, t);                \
                    t = tsrc[1]; update_extr_macro(val1, t);                \
                    t = tsrc[2]; update_extr_macro(val2, t);                \
                    t = tsrc[3]; update_extr_macro(val3, t);                \
                }                                                           \
                                                                            \
                tsrc = rows[0] + x;                                         \
                t = tsrc[0]; update_extr_macro(t, val0); dst[0] = CV_TOGGLE_FLT(t);\
                t = tsrc[1]; update_extr_macro(t, val1); dst[1] = CV_TOGGLE_FLT(t);\
                t = tsrc[2]; update_extr_macro(t, val2); dst[2] = CV_TOGGLE_FLT(t);\
                t = tsrc[3]; update_extr_macro(t, val3); dst[3] = CV_TOGGLE_FLT(t);\
                                                                            \
                tsrc = rows[row_count - 1] + x;                             \
                t = tsrc[0]; update_extr_macro(t, val0); dst[dststep] = CV_TOGGLE_FLT(t);\
                t = tsrc[1]; update_extr_macro(t, val1); dst[dststep+1] = CV_TOGGLE_FLT(t);\
                t = tsrc[2]; update_extr_macro(t, val2); dst[dststep+2] = CV_TOGGLE_FLT(t);\
                t = tsrc[3]; update_extr_macro(t, val3); dst[dststep+3] = CV_TOGGLE_FLT(t);\
            }                                                               \
                                                                            \
            for( ; x < width_n; x++, dst++ )                                \
            {                                                               \
                tsrc = rows[second_row] + x;                                \
                val0 = tsrc[0];                                             \
                                                                            \
                for( i = second_row + 1; i < last_row; i++ )                \
                {                                                           \
                    tsrc = rows[i] + x;                                     \
                    t = tsrc[0]; update_extr_macro(val0, t);                \
                }                                                           \
                                                                            \
                tsrc = rows[0] + x;                                         \
                t = tsrc[0]; update_extr_macro(t, val0); dst[0] = CV_TOGGLE_FLT(t);\
                tsrc = rows[row_count - 1] + x;                             \
                t = tsrc[0]; update_extr_macro(t, val0); dst[dststep] = CV_TOGGLE_FLT(t);\
            }                                                               \
            dst -= width_n;                                                 \
        }                                                                   \
        else                                                                \
        {                                                                   \
            for( x = 0; x < width_n; x++ )                                  \
            {                                                               \
                tsrc = rows[0] + x;                                         \
                val0 = tsrc[0];                                             \
                                                                            \
                for( i = 1; i < row_count; i++ )                            \
                {                                                           \
                    tsrc = rows[i] + x;                                     \
                    t = tsrc[0]; update_extr_macro(val0, t);                \
                }                                                           \
                                                                            \
                dst[x] = CV_TOGGLE_FLT(val0);                               \
            }                                                               \
        }                                                                   \
                                                                            \
        if( y+dy > ker_y && row_count > 1 )                                 \
        {                                                                   \
            /* rotate buffer */                                             \
            int shift = dy - (y < ker_y);                                   \
            int* row0 = rows[0];                                            \
            int* row1 = rows[1];                                            \
            if( row_count <= shift && src_height <= 0 )                     \
                shift = 1;                                                  \
                                                                            \
            CV_COPY( rows, rows + shift, ker_height + 1 - shift, i );       \
                                                                            \
            if( shift == 2 )                                                \
            {                                                               \
                rows[ker_height - 1] = row0;                                \
                rows[ker_height] = row1;                                    \
            }                                                               \
            else                                                            \
                rows[ker_height] = row0;                                    \
            row_count -= shift;                                             \
        }                                                                   \
                                                                            \
        rows_wanted = MIN(rows_wanted + 2, ker_height + 1);                 \
        dst += dststep*dy;                                                  \
    }                                                                       \
                                                                            \
    return CV_OK;                                                           \
}


/****************************************************************************************\
*                 Erode/Dilate with arbitrary element and arbitrary type                 *
\****************************************************************************************/

#define ICV_DEF_MORPH_ARB_FUNC( name, flavor, arrtype,                      \
                                update_extr_macro, toggle_macro )           \
static CvStatus CV_STDCALL                                                  \
icv##name##Any_##flavor( arrtype* src, int srcstep,                         \
                         arrtype* dst, int dststep,                         \
                         CvSize* roi, CvFilterState* state, int stage )      \
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
    if( stage == CV_START + CV_END )                                        \
        stage = CV_WHOLE;                                                   \
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
        if( ker_ptr == ker )                                                \
            *ker_ptr++ = rows[ker_y] + ker_x_n;                             \
                                                                            \
        if( channels == 3 )                                                 \
            for( x = 0; x < width_n; x += 3 )                               \
            {                                                               \
                arrtype** kp = ker;                                         \
                arrtype* tp = *kp++;                                        \
                int t, val0 = tp[x], val1 = tp[x+1], val2 = tp[x+2];        \
                                                                            \
                while( kp != ker_ptr )                                      \
                {                                                           \
                    tp = *kp++;                                             \
                    t = tp[x]; update_extr_macro( val0, t );                \
                    t = tp[x + 1]; update_extr_macro( val1, t );            \
                    t = tp[x + 2]; update_extr_macro( val2, t );            \
                }                                                           \
                tdst[x] = (arrtype)toggle_macro(val0);                      \
                tdst[x + 1] = (arrtype)toggle_macro(val1);                  \
                tdst[x + 2] = (arrtype)toggle_macro(val2);                  \
            }                                                               \
        else                                                                \
            /* channels == 1 or channels == 4 */                            \
            for( x = 0; x < width_n; x += 4 )                               \
            {                                                               \
                arrtype** kp = ker;                                         \
                arrtype* tp = *kp++;                                        \
                int t, val0 = tp[x], val1 = tp[x+1],                        \
                       val2 = tp[x+2], val3 = tp[x+3];                      \
                                                                            \
                while( kp != ker_ptr )                                      \
                {                                                           \
                    tp = *kp++;                                             \
                    t = tp[x]; update_extr_macro( val0, t );                \
                    t = tp[x + 1]; update_extr_macro( val1, t );            \
                    t = tp[x + 2]; update_extr_macro( val2, t );            \
                    t = tp[x + 3]; update_extr_macro( val3, t );            \
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


ICV_DEF_MORPH_RECT_INT_FUNC( Erode, 8u, uchar, CV_CALC_MIN_8U )
ICV_DEF_MORPH_RECT_INT_FUNC( Dilate, 8u, uchar, CV_CALC_MAX_8U )
ICV_DEF_MORPH_RECT_FLT_FUNC( Erode, CV_CALC_MIN )
ICV_DEF_MORPH_RECT_FLT_FUNC( Dilate, CV_CALC_MAX )

ICV_DEF_MORPH_ARB_FUNC( Erode, 8u, uchar, CV_CALC_MIN, CV_NOP )
ICV_DEF_MORPH_ARB_FUNC( Dilate, 8u, uchar, CV_CALC_MAX, CV_NOP )
ICV_DEF_MORPH_ARB_FUNC( Erode, 32f, int, CV_CALC_MIN, CV_TOGGLE_FLT )
ICV_DEF_MORPH_ARB_FUNC( Dilate, 32f, int, CV_CALC_MAX, CV_TOGGLE_FLT )


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


static void icvInitMorphologyTab( CvFuncTable* rect_erode, CvFuncTable* rect_dilate,
                                  CvFuncTable* any_erode, CvFuncTable* any_dilate )
{
    rect_erode->fn_2d[CV_8U] = (void*)icvErodeRect_8u;
    rect_erode->fn_2d[CV_32F] = (void*)icvErodeRect_32f;
    any_erode->fn_2d[CV_8U] = (void*)icvErodeAny_8u;
    any_erode->fn_2d[CV_32F] = (void*)icvErodeAny_32f;

    rect_dilate->fn_2d[CV_8U] = (void*)icvDilateRect_8u;
    rect_dilate->fn_2d[CV_32F] = (void*)icvDilateRect_32f;
    any_dilate->fn_2d[CV_8U] = (void*)icvDilateAny_8u;
    any_dilate->fn_2d[CV_32F] = (void*)icvDilateAny_32f;
}


typedef CvStatus (CV_STDCALL * CvMorphRectFunc_IPP)
    ( const void* src, int srcstep, void* dst, int dststep,
      CvSize roi, CvSize elSize, CvPoint elAnchor );

typedef CvStatus (CV_STDCALL * CvMorphAnyFunc_IPP)
    ( const void* src, int srcstep, void* dst, int dststep,
      CvSize roi, const uchar* element, CvSize elSize, CvPoint elAnchor );

static void
icvMorphOp( const void* srcarr, void* dstarr, IplConvKernel* element,
            int iterations, int mop )
{
    static CvFuncTable morph_tab[4];
    static int inittab = 0;
    CvFilterState *state = 0;
    CvMat* temp = 0;

    CV_FUNCNAME( "icvMorphOp" );

    __BEGIN__;

    CvMorphFunc func = 0;
    int i, coi1 = 0, coi2 = 0;
    CvMat srcstub, *src = (CvMat*)srcarr;
    CvMat dststub, *dst = (CvMat*)dstarr;
    CvSize size, el_size;
    CvPoint el_anchor;
    int el_shape;
    int type, depth;

    if( !inittab )
    {
        icvInitMorphologyTab( morph_tab+0, morph_tab+1, morph_tab+2, morph_tab+3 );
        inittab = 1;
    }

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
    depth = CV_MAT_DEPTH( type );

    if( iterations == 0 || (element && element->nCols == 1 && element->nRows == 1))
    {
        if( src->data.ptr != dst->data.ptr )
            cvCopy( src, dst );
        EXIT;
    }

    if( element )
    {
        el_size = cvSize( element->nCols, element->nRows );
        el_anchor = cvPoint( element->anchorX, element->anchorY );
        el_shape = (int)(element->nShiftR);
        el_shape = el_shape < CV_SHAPE_CUSTOM ? el_shape : CV_SHAPE_CUSTOM;
    }
    else
    {
        el_size = cvSize( 1+iterations*2, 1+iterations*2 );
        el_anchor = cvPoint( iterations, iterations );
        el_shape = CV_SHAPE_RECT;
        iterations = 1;
    }

    if( icvErodeRect_8u_C1R_p )
    {
        CvMorphRectFunc_IPP rect_func = 0;
        CvMorphAnyFunc_IPP any_func = 0;

        if( el_shape == CV_SHAPE_RECT && mop == 0 )
        {
            rect_func = type == CV_8UC1 ? icvErodeRect_8u_C1R_p :
                        type == CV_8UC3 ? icvErodeRect_8u_C3R_p :
                        type == CV_8UC4 ? icvErodeRect_8u_C4R_p :
                        type == CV_32FC1 ? icvErodeRect_32f_C1R_p :
                        type == CV_32FC3 ? icvErodeRect_32f_C3R_p :
                        type == CV_32FC4 ? icvErodeRect_32f_C4R_p : 0;
        }
        else if( el_shape == CV_SHAPE_RECT && mop == 1 )
        {
            rect_func = type == CV_8UC1 ? icvDilateRect_8u_C1R_p :
                        type == CV_8UC3 ? icvDilateRect_8u_C3R_p :
                        type == CV_8UC4 ? icvDilateRect_8u_C4R_p :
                        type == CV_32FC1 ? icvDilateRect_32f_C1R_p :
                        type == CV_32FC3 ? icvDilateRect_32f_C3R_p :
                        type == CV_32FC4 ? icvDilateRect_32f_C4R_p : 0;
        }
        else if( mop == 0 )
        {
            any_func =  type == CV_8UC1 ? icvErodeAny_8u_C1R_p :
                        type == CV_8UC3 ? icvErodeAny_8u_C3R_p :
                        type == CV_8UC4 ? icvErodeAny_8u_C4R_p :
                        type == CV_32FC1 ? icvErodeAny_32f_C1R_p :
                        type == CV_32FC3 ? icvErodeAny_32f_C3R_p :
                        type == CV_32FC4 ? icvErodeAny_32f_C4R_p : 0;
        }
        else
        {
            any_func =  type == CV_8UC1 ? icvDilateAny_8u_C1R_p :
                        type == CV_8UC3 ? icvDilateAny_8u_C3R_p :
                        type == CV_8UC4 ? icvDilateAny_8u_C4R_p :
                        type == CV_32FC1 ? icvDilateAny_32f_C1R_p :
                        type == CV_32FC3 ? icvDilateAny_32f_C3R_p :
                        type == CV_32FC4 ? icvDilateAny_32f_C4R_p : 0;
        }

        
        if( rect_func || any_func )
        {
            int y, dy = 0;
            int el_len = el_size.width*el_size.height;
            int temp_step;
            uchar* el_mask = 0;
            const uchar* shifted_ptr;
            int stripe_size = 1 << 15; // the optimal value may depend on CPU cache,
                                       // overhead of current IPP code etc.
            if( any_func )
            {
                el_mask = (uchar*)cvStackAlloc( el_len );
                for( i = 0; i < el_len; i++ )
                    el_mask[i] = (uchar)(element->values[i] != 0);
            }

            CV_CALL( temp = icvIPPFilterInit( src, stripe_size, el_size ));
            
            shifted_ptr = temp->data.ptr +
                el_anchor.y*temp->step + el_anchor.x*CV_ELEM_SIZE(type);
            temp_step = temp->step ? temp->step : CV_STUB_STEP;

            for( i = 0; i < iterations; i++ )
            {
                int dst_step = dst->step ? dst->step : CV_STUB_STEP;

                for( y = 0; y < src->rows; y += dy )
                {
                    dy = icvIPPFilterNextStripe( src, temp, y, el_size, el_anchor );
                    if( rect_func )
                    {
                        IPPI_CALL( rect_func( shifted_ptr, temp_step,
                            dst->data.ptr + y*dst_step, dst_step, cvSize(src->cols, dy),
                            el_size, el_anchor ));
                    }
                    else
                    {
                        IPPI_CALL( any_func( shifted_ptr, temp_step,
                            dst->data.ptr + y*dst_step, dst_step, cvSize(src->cols, dy),
                            el_mask, el_size, el_anchor ));
                    }
                }
                src = dst;
            }
            EXIT;
        }
    }

    CV_CALL( state = icvMorphologyInitAlloc( src->width,
        depth == CV_8U ? cv8u : cv32f, CV_MAT_CN(type),
        el_size, el_anchor, el_shape, element ? element->values : 0 ));

    func = (CvMorphFunc)(morph_tab[(el_shape != CV_SHAPE_RECT)*2 + mop].fn_2d[depth]);
    if( !func )
        CV_ERROR( CV_StsUnsupportedFormat, "" );

    size = cvGetMatSize( src );
    for( i = 0; i < iterations; i++ )
    {
        IPPI_CALL( func( src->data.ptr, src->step, dst->data.ptr,
                         dst->step, &size, state, 0 ));
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
