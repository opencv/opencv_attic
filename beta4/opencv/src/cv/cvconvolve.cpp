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

#define ICV_DEF_FILTER_FUNC( flavor, arrtype, worktype,                     \
                             load_macro, cast_macro1, cast_macro2 )         \
static CvStatus CV_STDCALL                                                  \
icvFilter_##flavor##_CnR( arrtype* src, int srcstep,                        \
                          arrtype* dst, int dststep, CvSize* roi,           \
                          CvFilterState* state, int stage )                 \
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
    const float *ker_data = (const float*)state->ker0;                      \
                                                                            \
    int crows = state->crows;                                               \
    arrtype **rows = (arrtype**) (state->rows);                             \
    arrtype* tbuf = (arrtype*)(state->tbuf);                                \
                                                                            \
    int channels = state->channels;                                         \
    int ker_x_n = ker_x * channels;                                         \
    int ker_width_n = ker_width * channels;                                 \
    int width_n = width * channels;                                         \
                                                                            \
    int starting_flag = 0;                                                  \
    int width_rest = width_n & (CV_MORPH_ALIGN - 1);                        \
    arrtype **ker_ptr, **ker = (arrtype**)cvStackAlloc(                     \
                ker_width*ker_height*sizeof(ker[0]) );                      \
    float* ker_coeffs0 = (float*)cvStackAlloc(                              \
                ker_width*ker_height*sizeof(ker_coeffs0[0]) );              \
    float* ker_coeffs = ker_coeffs0;                                        \
                                                                            \
    for( i = 0; i < ker_height; i++ )                                       \
        for( x = 0; x < ker_width; x++ )                                    \
        {                                                                   \
            int t = ((int*)ker_data)[i*ker_width + x];                      \
            if( t )                                                         \
            {                                                               \
                *(int*)ker_coeffs = t;                                      \
                ker_coeffs++;                                               \
            }                                                               \
        }                                                                   \
                                                                            \
    /* initialize cyclic buffer when starting */                            \
    if( stage == CV_WHOLE || stage == CV_START )                            \
    {                                                                       \
        for( i = 0; i < ker_height; i++ )                                   \
        {                                                                   \
            rows[i] = (arrtype*)(state->buffer + state->buffer_step * i);   \
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
    do                                                                      \
    {                                                                       \
        arrtype *tsrc, *tdst;                                               \
        int need_copy = 0;                                                  \
                                                                            \
        /* fill cyclic buffer */                                            \
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
                                                                            \
            CV_COPY( tdst + ker_x_n, tsrc, width_n, x );                    \
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
        /* do convolution */                                                \
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
                if( ((int*)ker_data)[i*ker_width + x] )                     \
                    *ker_ptr++ = rows[i] + x*channels;                      \
                                                                            \
        if( channels == 3 )                                                 \
        {                                                                   \
            for( x = 0; x < width_n; x += 3 )                               \
            {                                                               \
                float sum0 = 0, sum1 = 0, sum2 = 0;                         \
                worktype t0, t1, t2;                                        \
                arrtype** kp = ker;                                         \
                ker_coeffs = ker_coeffs0;                                   \
                while( kp != ker_ptr )                                      \
                {                                                           \
                    arrtype* tp = *kp++;                                    \
                    float f = *ker_coeffs++;                                \
                    sum0 += load_macro(tp[x])*f;                            \
                    sum1 += load_macro(tp[x+1])*f;                          \
                    sum2 += load_macro(tp[x+2])*f;                          \
                }                                                           \
                t0 = cast_macro1(sum0);                                     \
                t1 = cast_macro1(sum1);                                     \
                t2 = cast_macro1(sum2);                                     \
                tdst[x] = cast_macro2(t0);                                  \
                tdst[x+1] = cast_macro2(t1);                                \
                tdst[x+2] = cast_macro2(t2);                                \
            }                                                               \
        }                                                                   \
        else                                                                \
        {                                                                   \
            for( x = 0; x < width_n; x += 4 )                               \
            {                                                               \
                float sum0 = 0, sum1 = 0, sum2 = 0, sum3 = 0;               \
                worktype t0, t1;                                            \
                arrtype** kp = ker;                                         \
                ker_coeffs = ker_coeffs0;                                   \
                while( kp != ker_ptr )                                      \
                {                                                           \
                    arrtype* tp = *kp++;                                    \
                    float f = *ker_coeffs++;                                \
                    sum0 += load_macro(tp[x])*f;                            \
                    sum1 += load_macro(tp[x+1])*f;                          \
                    sum2 += load_macro(tp[x+2])*f;                          \
                    sum3 += load_macro(tp[x+3])*f;                          \
                }                                                           \
                t0 = cast_macro1(sum0);                                     \
                t1 = cast_macro1(sum1);                                     \
                tdst[x] = cast_macro2(t0);                                  \
                tdst[x+1] = cast_macro2(t1);                                \
                t0 = cast_macro1(sum2);                                     \
                t1 = cast_macro1(sum3);                                     \
                tdst[x+2] = cast_macro2(t0);                                \
                tdst[x+3] = cast_macro2(t1);                                \
            }                                                               \
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


ICV_DEF_FILTER_FUNC( 8u, uchar, int, CV_8TO32F, cvRound, CV_CAST_8U )
ICV_DEF_FILTER_FUNC( 16u, ushort, int, CV_NOP, cvRound, CV_CAST_16U )
ICV_DEF_FILTER_FUNC( 32f, float, float, CV_NOP, CV_NOP, CV_NOP )


static void icvInitFilterTab( CvFuncTable* tab )
{
    tab->fn_2d[CV_8U] = (void*)icvFilter_8u_CnR;
    tab->fn_2d[CV_16U] = (void*)icvFilter_16u_CnR;
    tab->fn_2d[CV_32F] = (void*)icvFilter_32f_CnR;
}


CV_IMPL void
cvFilter2D( const CvArr* _src, CvArr* _dst, const CvMat* _kernel, CvPoint anchor )
{
    static CvFuncTable filter_tab;
    static int inittab = 0;
    CvFilterState *state = 0;
    float* kernel_data = 0;
    int local_alloc = 1;

    CV_FUNCNAME( "cvFilter2D" );

    __BEGIN__;

    CvFilterFunc func = 0;
    int coi1 = 0, coi2 = 0;
    CvMat srcstub, *src = (CvMat*)_src;
    CvMat dststub, *dst = (CvMat*)_dst;
    CvSize size;
    int type, depth;
    int src_step, dst_step;
    CvMat kernel_hdr;
    const CvMat* kernel = _kernel;

    if( !inittab )
    {
        icvInitFilterTab( &filter_tab );
        inittab = 1;
    }

    CV_CALL( src = cvGetMat( src, &srcstub, &coi1 ));
    CV_CALL( dst = cvGetMat( dst, &dststub, &coi2 ));

    if( coi1 != 0 || coi2 != 0 )
        CV_ERROR( CV_BadCOI, "" );

    type = CV_MAT_TYPE( src->type );

    if( !CV_ARE_SIZES_EQ( src, dst ))
        CV_ERROR( CV_StsUnmatchedSizes, "" );

    if( !CV_ARE_TYPES_EQ( src, dst ))
        CV_ERROR( CV_StsUnmatchedFormats, "" );

    if( !CV_IS_MAT(kernel) ||
        (CV_MAT_TYPE(kernel->type) != CV_32F &&
        CV_MAT_TYPE(kernel->type) != CV_64F ))
        CV_ERROR( CV_StsBadArg, "kernel must be single-channel floating-point matrix" );

    if( anchor.x == -1 && anchor.y == -1 )
        anchor = cvPoint(kernel->cols/2,kernel->rows/2);

    if( (unsigned)anchor.x >= (unsigned)kernel->cols ||
        (unsigned)anchor.y >= (unsigned)kernel->rows )
        CV_ERROR( CV_StsOutOfRange, "anchor point is out of kernel" );

    if( CV_MAT_TYPE(kernel->type) != CV_32F )
    {
        int sz = kernel->rows*kernel->cols*sizeof(kernel_data[0]);
        if( sz < CV_MAX_LOCAL_SIZE )
            kernel_data = (float*)cvStackAlloc( sz );
        else
        {
            CV_CALL( kernel_data = (float*)cvAlloc( sz ));
            local_alloc = 0;
        }
        kernel_hdr = cvMat( kernel->rows, kernel->cols, CV_32F, kernel_data );
        cvConvertScale( kernel, &kernel_hdr, 1, 0 );
        kernel = &kernel_hdr;
    }

    size = cvGetMatSize( src );
    
    depth = CV_MAT_DEPTH(type);
    IPPI_CALL( icvFilterInitAlloc( src->cols, cv32f, CV_MAT_CN(type),
                                   cvSize(kernel->cols, kernel->rows), anchor,
                                   kernel->data.ptr, ICV_GENERIC_KERNEL, &state ));

    if( CV_MAT_CN(type) == 2 )
        CV_ERROR( CV_BadNumChannels, "Unsupported number of channels" );

    func = (CvFilterFunc)(filter_tab.fn_2d[depth]);

    if( !func )
        CV_ERROR( CV_StsUnsupportedFormat, "" );

    src_step = src->step;
    dst_step = dst->step;

    if( size.height == 1 )
        src_step = dst_step = CV_AUTOSTEP;

    IPPI_CALL( func( src->data.ptr, src_step, dst->data.ptr,
                     dst_step, &size, state, 0 ));

    __END__;

    icvFilterFree( &state );
    if( !local_alloc )
        cvFree( (void**)&kernel_data );
}

/* End of file. */
