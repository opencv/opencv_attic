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

/////////////////////// common functions for working with IPP filters ////////////////////

CvMat* icvIPPFilterInit( const CvMat* src, int stripe_size, CvSize ksize )
{
    CvSize temp_size;
    int pix_size = CV_ELEM_SIZE(src->type);
    temp_size.width = cvAlign(src->cols + ksize.width - 1,8/CV_ELEM_SIZE(src->type & CV_MAT_DEPTH_MASK));
    //temp_size.width = src->cols + ksize.width - 1;
    temp_size.height = (stripe_size*2 + temp_size.width*pix_size) / (temp_size.width*pix_size*2);
    temp_size.height = MAX( temp_size.height, ksize.height );
    temp_size.height = MIN( temp_size.height, src->rows + ksize.height - 1 );
    
    return cvCreateMat( temp_size.height, temp_size.width, src->type );
}


int icvIPPFilterNextStripe( const CvMat* src, CvMat* temp, int y,
                            CvSize ksize, CvPoint anchor )
{
    int pix_size = CV_ELEM_SIZE(src->type);
    int src_step = src->step ? src->step : CV_STUB_STEP;
    int temp_step = temp->step ? temp->step : CV_STUB_STEP;
    int i, dy, src_y1 = 0, src_y2;
    int temp_rows;
    uchar* temp_ptr = temp->data.ptr;
    CvSize stripe_size, temp_size;
    CvCopyNonConstBorderFunc copy_border_func =
        icvGetCopyNonConstBorderFunc( pix_size, IPL_BORDER_REPLICATE );

    dy = MIN( temp->rows - ksize.height + 1, src->rows - y );
    if( y > 0 )
    {
        int temp_ready = ksize.height - 1;
        
        for( i = 0; i < temp_ready; i++ )
            memcpy( temp_ptr + temp_step*i, temp_ptr +
                    temp_step*(temp->rows - temp_ready + i), temp_step );

        temp_ptr += temp_ready*temp_step;
        temp_rows = dy;
        src_y1 = y + temp_ready - anchor.y;
        src_y2 = src_y1 + dy;
        if( src_y1 >= src->rows )
        {
            src_y1 = src->rows - 1;
            src_y2 = src->rows;
        }
    }
    else
    {
        temp_rows = dy + ksize.height - 1;
        src_y2 = temp_rows - anchor.y;
    }

    src_y2 = MIN( src_y2, src->rows );

    stripe_size = cvSize(src->cols, src_y2 - src_y1);
    temp_size = cvSize(temp->cols, temp_rows);
    copy_border_func( src->data.ptr + src_y1*src_step, src_step,
                      stripe_size, temp_ptr, temp_step, temp_size,
                      (y == 0 ? anchor.y : 0), anchor.x );
    return dy;
}


/////////////////////////////// IPP separable filter functions ///////////////////////////

icvFilterRow_8u_C1R_t icvFilterRow_8u_C1R_p = 0;
icvFilterRow_8u_C3R_t icvFilterRow_8u_C3R_p = 0;
icvFilterRow_8u_C4R_t icvFilterRow_8u_C4R_p = 0;
icvFilterRow_16s_C1R_t icvFilterRow_16s_C1R_p = 0;
icvFilterRow_16s_C3R_t icvFilterRow_16s_C3R_p = 0;
icvFilterRow_16s_C4R_t icvFilterRow_16s_C4R_p = 0;
icvFilterRow_32f_C1R_t icvFilterRow_32f_C1R_p = 0;
icvFilterRow_32f_C3R_t icvFilterRow_32f_C3R_p = 0;
icvFilterRow_32f_C4R_t icvFilterRow_32f_C4R_p = 0;

icvFilterColumn_8u_C1R_t icvFilterColumn_8u_C1R_p = 0;
icvFilterColumn_8u_C3R_t icvFilterColumn_8u_C3R_p = 0;
icvFilterColumn_8u_C4R_t icvFilterColumn_8u_C4R_p = 0;
icvFilterColumn_16s_C1R_t icvFilterColumn_16s_C1R_p = 0;
icvFilterColumn_16s_C3R_t icvFilterColumn_16s_C3R_p = 0;
icvFilterColumn_16s_C4R_t icvFilterColumn_16s_C4R_p = 0;
icvFilterColumn_32f_C1R_t icvFilterColumn_32f_C1R_p = 0;
icvFilterColumn_32f_C3R_t icvFilterColumn_32f_C3R_p = 0;
icvFilterColumn_32f_C4R_t icvFilterColumn_32f_C4R_p = 0;

//////////////////////////////////////////////////////////////////////////////////////////

typedef CvStatus (CV_STDCALL * CvIPPSepFilterFunc)
    ( const void* src, int srcstep, void* dst, int dststep,
      CvSize size, const float* kernel, int ksize, int anchor );

int icvIPPSepFilter( const CvMat* src, CvMat* dst, const CvMat* kernelX,
                     const CvMat* kernelY, CvPoint anchor )
{
    int result = 0;
    
    CvMat* top_bottom = 0;
    CvMat* vout_hin = 0;
    CvMat* dst_buf = 0;
    
    CV_FUNCNAME( "icvIPPSepFilter" );

    __BEGIN__;

    CvSize ksize;
    CvPoint el_anchor;
    CvSize size;
    int type, depth, pix_size;
    int i, x, y, dy = 0, prev_dy = 0, max_dy;
    CvMat vout;
    CvCopyNonConstBorderFunc copy_border_func;
    CvIPPSepFilterFunc x_func = 0, y_func = 0;
    int src_step, top_bottom_step;
    float *kx, *ky;
    int align, stripe_size;

    if( !icvFilterRow_8u_C1R_p )
        EXIT;

    if( !CV_ARE_TYPES_EQ( src, dst ) || !CV_ARE_SIZES_EQ( src, dst ) ||
        !CV_IS_MAT_CONT(kernelX->type & kernelY->type) ||
        CV_MAT_TYPE(kernelX->type) != CV_32FC1 ||
        CV_MAT_TYPE(kernelY->type) != CV_32FC1 ||
        kernelX->cols != 1 && kernelX->rows != 1 ||
        kernelY->cols != 1 && kernelY->rows != 1 ||
        (unsigned)anchor.x >= (unsigned)(kernelX->cols + kernelX->rows - 1) ||
        (unsigned)anchor.y >= (unsigned)(kernelY->cols + kernelY->rows - 1) )
        CV_ERROR( CV_StsError, "Internal Error: incorrect parameters" );

    ksize.width = kernelX->cols + kernelX->rows - 1;
    ksize.height = kernelY->cols + kernelY->rows - 1;

    /*if( ksize.width <= 5 && ksize.height <= 5 )
    {
        float* ker = (float*)cvStackAlloc( ksize.width*ksize.height*sizeof(ker[0]));
        CvMat kernel = cvMat( ksize.height, ksize.width, CV_32F, ker );
        for( y = 0, i = 0; y < ksize.height; y++ )
            for( x = 0; x < ksize.width; x++, i++ )
                ker[i] = kernelY->data.fl[y]*kernelX->data.fl[x];

        CV_CALL( cvFilter2D( src, dst, &kernel, anchor ));
        EXIT;
    }*/

    type = CV_MAT_TYPE(src->type);
    depth = CV_MAT_DEPTH(type);
    pix_size = CV_ELEM_SIZE(type);

    if( type == CV_8UC1 )
        x_func = icvFilterRow_8u_C1R_p, y_func = icvFilterColumn_8u_C1R_p;
    else if( type == CV_8UC3 )
        x_func = icvFilterRow_8u_C3R_p, y_func = icvFilterColumn_8u_C3R_p;
    else if( type == CV_8UC4 )
        x_func = icvFilterRow_8u_C4R_p, y_func = icvFilterColumn_8u_C4R_p;
    else if( type == CV_16SC1 )
        x_func = icvFilterRow_16s_C1R_p, y_func = icvFilterColumn_16s_C1R_p;
    else if( type == CV_16SC3 )
        x_func = icvFilterRow_16s_C3R_p, y_func = icvFilterColumn_16s_C3R_p;
    else if( type == CV_16SC4 )
        x_func = icvFilterRow_16s_C4R_p, y_func = icvFilterColumn_16s_C4R_p;
    else if( type == CV_32FC1 )
        x_func = icvFilterRow_32f_C1R_p, y_func = icvFilterColumn_32f_C1R_p;
    else if( type == CV_32FC3 )
        x_func = icvFilterRow_32f_C3R_p, y_func = icvFilterColumn_32f_C3R_p;
    else if( type == CV_32FC4 )
        x_func = icvFilterRow_32f_C4R_p, y_func = icvFilterColumn_32f_C4R_p;
    else
        EXIT;

    size = cvGetMatSize(src);
    stripe_size = src->data.ptr == dst->data.ptr ? 1 << 15 : 1 << 16;
    max_dy = MAX( ksize.height - 1, stripe_size/(size.width + ksize.width - 1));
    
    align = 8/CV_ELEM_SIZE(depth);

    CV_CALL( top_bottom = cvCreateMat( ksize.height - 1 +
        MAX(ksize.height - anchor.y - 1, anchor.y), cvAlign(size.width,align), type ));

    CV_CALL( vout_hin = cvCreateMat( max_dy, cvAlign(size.width + ksize.width - 1, align), type ));
    
    if( src->data.ptr == dst->data.ptr && size.height )
        CV_CALL( dst_buf = cvCreateMat( max_dy, cvAlign(size.width, align), type ));

    kx = (float*)cvStackAlloc( ksize.width*sizeof(kx[0]) );
    ky = (float*)cvStackAlloc( ksize.height*sizeof(ky[0]) );

    // mirror the kernels
    for( i = 0; i < ksize.width; i++ )
        kx[i] = kernelX->data.fl[ksize.width - i - 1];

    for( i = 0; i < ksize.height; i++ )
        ky[i] = kernelY->data.fl[ksize.height - i - 1];

    el_anchor = cvPoint( ksize.width - anchor.x - 1, ksize.height - anchor.y - 1 );

    cvGetCols( vout_hin, &vout, anchor.x, anchor.x + size.width );
    copy_border_func = icvGetCopyNonConstBorderFunc( pix_size, IPL_BORDER_REPLICATE );

    src_step = src->step ? src->step : CV_STUB_STEP;
    top_bottom_step = top_bottom->step ? top_bottom->step : CV_STUB_STEP;
    vout.step = vout.step ? vout.step : CV_STUB_STEP;

    for( y = 0; y < size.height; y += dy )
    {
        const CvMat *vin = src, *hout = dst;
        int src_y = y, dst_y = y;
        dy = MIN( max_dy, size.height - (ksize.height - anchor.y - 1) - y );

        if( y < anchor.y || dy <= 0 )
        {
            int ay = anchor.y;
            CvSize src_stripe_size = size;
            
            if( y < anchor.y )
            {
                src_y = 0;
                dy = MIN( anchor.y, size.height );
                src_stripe_size.height = MIN( dy + ksize.height - anchor.y - 1, size.height );
            }
            else
            {
                src_y = MAX( y - anchor.y, 0 );
                dy = size.height - y;
                src_stripe_size.height = MIN( dy + anchor.y, size.height );
                ay = MAX( anchor.y - y, 0 );
            }

            copy_border_func( src->data.ptr + src_y*src_step, src_step, src_stripe_size,
                              top_bottom->data.ptr, top_bottom_step,
                              cvSize(size.width, dy + ksize.height - 1),
                              ay, 0 );
            vin = top_bottom;
            src_y = anchor.y;            
        }

        // do vertical convolution
        IPPI_CALL( y_func( vin->data.ptr + src_y*vin->step, vin->step ? vin->step : CV_STUB_STEP,
                           vout.data.ptr, vout.step, cvSize(size.width, dy),
                           ky, ksize.height, el_anchor.y ));

        // now it's time to copy the previously processed stripe to the input/output image
        if( src->data.ptr == dst->data.ptr )
        {
            for( i = 0; i < prev_dy; i++ )
                memcpy( dst->data.ptr + (y - prev_dy + i)*dst->step,
                        dst_buf->data.ptr + i*dst_buf->step, size.width*pix_size );
            if( y + dy < size.height )
            {
                hout = dst_buf;
                dst_y = 0;
            }
        }

        // create a border for every line by replicating the left-most/right-most elements
        for( i = 0; i < dy; i++ )
        {
            uchar* ptr = vout.data.ptr + i*vout.step;
            for( x = -1; x >= -anchor.x*pix_size; x-- )
                ptr[x] = ptr[x + pix_size];
            for( x = size.width*pix_size; x < (size.width+ksize.width-anchor.x-1)*pix_size; x++ )
                ptr[x] = ptr[x - pix_size];
        }

        // do horizontal convolution
        IPPI_CALL( x_func( vout.data.ptr, vout.step, hout->data.ptr + dst_y*hout->step,
                           hout->step ? hout->step : CV_STUB_STEP,
                           cvSize(size.width, dy), kx, ksize.width, el_anchor.x ));
        prev_dy = dy;
    }

    result = 1;

    __END__;

    cvReleaseMat( &vout_hin );
    cvReleaseMat( &dst_buf );
    cvReleaseMat( &top_bottom );

    return result;
}


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
    srcstep /= sizeof(src[0]);                                              \
    dststep /= sizeof(dst[0]);                                              \
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


//////////////////////////////// IPP generic filter functions ////////////////////////////

icvFilter_8u_C1R_t icvFilter_8u_C1R_p = 0;
icvFilter_8u_C3R_t icvFilter_8u_C3R_p = 0;
icvFilter_8u_C4R_t icvFilter_8u_C4R_p = 0;
icvFilter_16s_C1R_t icvFilter_16s_C1R_p = 0;
icvFilter_16s_C3R_t icvFilter_16s_C3R_p = 0;
icvFilter_16s_C4R_t icvFilter_16s_C4R_p = 0;
icvFilter_32f_C1R_t icvFilter_32f_C1R_p = 0;
icvFilter_32f_C3R_t icvFilter_32f_C3R_p = 0;
icvFilter_32f_C4R_t icvFilter_32f_C4R_p = 0;

//////////////////////////////////////////////////////////////////////////////////////////

typedef CvStatus (CV_STDCALL * CvFilterIPPFunc)
( const void* src, int srcstep, void* dst, int dststep, CvSize size,
  const float* kernel, CvSize ksize, CvPoint anchor );

CV_IMPL void
cvFilter2D( const CvArr* _src, CvArr* _dst, const CvMat* _kernel, CvPoint anchor )
{
    // below that approximate size OpenCV is faster
    const int ipp_lower_limit = 20;
    
    static CvFuncTable filter_tab;
    static int inittab = 0;
    CvFilterState *state = 0;
    float* kernel_data = 0;
    int local_alloc = 1;
    CvMat* temp = 0;

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

    if( CV_MAT_TYPE(kernel->type) != CV_32FC1 || !CV_IS_MAT_CONT(kernel->type) || icvFilter_8u_C1R_p )
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
        if( CV_MAT_TYPE(kernel->type) == CV_32FC1 )
            cvCopy( kernel, &kernel_hdr );
        else
            cvConvertScale( kernel, &kernel_hdr, 1, 0 );
        kernel = &kernel_hdr;
    }

    size = cvGetMatSize( src );
    depth = CV_MAT_DEPTH(type);

    if( icvFilter_8u_C1R_p && (src->rows >= ipp_lower_limit || src->cols >= ipp_lower_limit) )
    {
        CvFilterIPPFunc ipp_func = 
                type == CV_8UC1 ? (CvFilterIPPFunc)icvFilter_8u_C1R_p :
                type == CV_8UC3 ? (CvFilterIPPFunc)icvFilter_8u_C3R_p :
                type == CV_8UC4 ? (CvFilterIPPFunc)icvFilter_8u_C4R_p :
                type == CV_16SC1 ? (CvFilterIPPFunc)icvFilter_16s_C1R_p :
                type == CV_16SC3 ? (CvFilterIPPFunc)icvFilter_16s_C3R_p :
                type == CV_16SC4 ? (CvFilterIPPFunc)icvFilter_16s_C4R_p :
                type == CV_32FC1 ? (CvFilterIPPFunc)icvFilter_32f_C1R_p :
                type == CV_32FC3 ? (CvFilterIPPFunc)icvFilter_32f_C3R_p :
                type == CV_32FC4 ? (CvFilterIPPFunc)icvFilter_32f_C4R_p : 0;
        
        if( ipp_func )
        {
            CvSize el_size = { kernel->cols, kernel->rows };
            CvPoint el_anchor = { el_size.width - anchor.x - 1, el_size.height - anchor.y - 1 };
            int stripe_size = 1 << 16; // the optimal value may depend on CPU cache,
                                       // overhead of current IPP code etc.
            const uchar* shifted_ptr;
            int i, j, y, dy = 0;
            int temp_step;
            int dst_step = dst->step ? dst->step : CV_STUB_STEP;

            // mirror the kernel around the center
            for( i = 0; i < (el_size.height+1)/2; i++ )
            {
                float* top_row = kernel->data.fl + el_size.width*i;
                float* bottom_row = kernel->data.fl + el_size.width*(el_size.height - i - 1);

                for( j = 0; j < (el_size.width+1)/2; j++ )
                {
                    float a = top_row[j], b = top_row[el_size.width - j - 1];
                    float c = bottom_row[j], d = bottom_row[el_size.width - j - 1];
                    top_row[j] = d;
                    top_row[el_size.width - j - 1] = c;
                    bottom_row[j] = b;
                    bottom_row[el_size.width - j - 1] = a;
                }
            }

            CV_CALL( temp = icvIPPFilterInit( src, stripe_size, el_size ));
            
            shifted_ptr = temp->data.ptr +
                anchor.y*temp->step + anchor.x*CV_ELEM_SIZE(type);
            temp_step = temp->step ? temp->step : CV_STUB_STEP;

            for( y = 0; y < src->rows; y += dy )
            {
                dy = icvIPPFilterNextStripe( src, temp, y, el_size, anchor );
                IPPI_CALL( ipp_func( shifted_ptr, temp_step,
                    dst->data.ptr + y*dst_step, dst_step, cvSize(src->cols, dy),
                    kernel->data.fl, el_size, el_anchor ));
            }
            EXIT;
        }
    }

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

    cvReleaseMat( &temp );
    icvFilterFree( &state );
    if( !local_alloc )
        cvFree( (void**)&kernel_data );
}

/* End of file. */
