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
*                                        S O B E L                                       *
\****************************************************************************************/

/* This function calculates generalized Sobel kernel */
static int icvCalcKer( char *kernel, int order, int size,
                       CvDataType datatype, int origin )
{
    int i, j;
    int* kerI = (int*)kernel;
    int type = -1;
    
    if( size != CV_SCHARR )
    {
        if( size == 3 )
        {
            switch( order )
            {
            case 0:
                kerI[0] = 1;
                kerI[1] = 2;
                kerI[2] = 1;
                type = ICV_1_2_1_KERNEL;
                break;
            case 1:
                kerI[0] =-1;
                kerI[1] = 0;
                kerI[2] = 1;
                type = ICV_m1_0_1_KERNEL;
                break;
            case 2:
                kerI[0] = 1;
                kerI[1] =-2;
                kerI[2] = 1;
                type = ICV_1_m2_1_KERNEL;
                break;
            default:
                return CV_BADARG_ERR;
            }
        }
        else
        {
            int oldval, newval;

            memset( kerI + 1, 0, size * sizeof(kerI[0]));
            kerI[0] = 1;

            for( i = 0; i < size - order - 1; i++ )
            {
                oldval = kerI[0];
                for( j = 1; j <= size; j++ )
                {
                    newval = kerI[j]+kerI[j-1];
                    kerI[j-1] = oldval;
                    oldval = newval;
                }
            }

            for( i = 0; i < order; i++ )
            {
                oldval = -kerI[0];
                for( j = 1; j <= size; j++ )
                {
                    newval = kerI[j-1] - kerI[j];
                    kerI[j-1] = oldval;
                    oldval = newval;
                }
            }

            type = order & 1 ? ICV_ASYMMETRIC_KERNEL : ICV_SYMMETRIC_KERNEL;
        }
    }
    else
    {
        size = 3;
        if( order == 1 )
        {
            kerI[0] = -1;
            kerI[1] = 0;
            kerI[2] = 1;
            type = ICV_m1_0_1_KERNEL;
        }
        else
        {
            assert( order == 0 );
            kerI[0] = kerI[2] = 3;
            kerI[1] = 10;
            type = ICV_3_10_3_KERNEL;
        }
    }

    if( origin && (order & 1) )
        for( j = 0; j < size; j++ )
            kerI[j] = -kerI[j];

    if( datatype == cv32f )
        for( j = 0; j < size; j++ )
            ((float*)kerI)[j] = (float)kerI[j];

    return type;
}


CvStatus CV_STDCALL icvSobelInitAlloc( int roiwidth, int datatype, int size,
                                       int origin, int dx, int dy, CvFilterState** state )
{
    #define MAX_KERNEL_SIZE  7
    int ker[MAX_KERNEL_SIZE*2+1];
    CvDataType worktype = datatype != cv32f ? cv32s : cv32f;
    CvStatus status;
    int x_filter_type, y_filter_type;
    int x_size = size, y_size = size;
    
    if( !state )
        return CV_NULLPTR_ERR;

    if( size == CV_SCHARR )
    {
        if( dx + dy != 1 )
            return CV_BADRANGE_ERR;
        x_size = y_size = 3;
    }
    else
    {
        if( (size&1) == 0 || size < 1 || size > MAX_KERNEL_SIZE )
            return CV_BADRANGE_ERR;

        if( (unsigned)dx > 2 || (unsigned)dy > 2 )
            return CV_BADRANGE_ERR;

        if( size == 1 )
        {
            if( dy == 0 )
                x_size = 3, y_size = 1;
            else if( dx == 0 )
                x_size = 1, y_size = 3;
            else
                return CV_BADARG_ERR;
        }
    }

    x_filter_type = icvCalcKer( (char*)ker, dx, size < 0 ? size : x_size, worktype, 0 );
    y_filter_type = icvCalcKer( (char*)(ker + x_size), dy, size < 0 ? size : y_size,
                                worktype, origin != 0 );

    {
    CvSize element_size = { x_size, y_size };
    CvPoint element_anchor = { x_size/2, y_size/2 };
    
    status = icvFilterInitAlloc( roiwidth, worktype, 1, element_size, element_anchor, ker,
                                 ICV_MAKE_SEPARABLE_KERNEL(x_filter_type, y_filter_type), state );
    }
    if( status < 0 )
        return status;

    (*state)->origin = origin != 0;

    return CV_OK;
}


CvStatus CV_STDCALL icvSobel_8u16s_C1R( const uchar* pSrc, int srcStep,
                                        short* dst, int dstStep, CvSize* roiSize,
                                        CvFilterState* state, int stage )
{
    uchar* src = (uchar*)pSrc;
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
    short *tbufw = (short*)(state->tbuf);
    int *trow = 0;

    int* fmaskX = (int*)(state->ker0) + ker_x;
    int* fmaskY = (int*)(state->ker1) + ker_y;
    int fmX0 = fmaskX[0], fmY0 = fmaskY[0];

    int is_small_width = width < MAX( ker_x, ker_right );
    int starting_flag = 0;
    int width_rest = width & (CV_MORPH_ALIGN - 1);
    int origin = state->origin;
    int x_type = ICV_X_KERNEL_TYPE(state->kerType),
        y_type = ICV_Y_KERNEL_TYPE(state->kerType);
    int x_asymm = (x_type & 3) - 1, /* <0 - general kind (not used),
                                       0-symmetric, 1-asymmetric*/
        y_asymm = (y_type & 3) - 1;

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

    dstStep /= sizeof(dst[0]);

    do
    {
        int need_copy = is_small_width | (y == 0);
        uchar *tsrc;
        int   *tdst;
        short *tdst2;
        int   *saved_row = rows[ker_y];

        /* fill cyclic buffer - horizontal filtering */
        for( ; crows < ker_height; crows++ )
        {
            tsrc = src - ker_x;
            tdst = rows[crows];

            if( src_height-- <= 0 )
            {
                if( stage != CV_WHOLE && stage != CV_END )
                    break;
                /* duplicate last row */
                trow = rows[crows - 1];
                CV_COPY( tdst, trow, width, x );
                continue;
            }

            need_copy |= src_height == 1;

            if( ker_width > 1 )
            {
                uchar* tbufc = (uchar*)tbufw;

                if( need_copy )
                {
                    tsrc = tbufc - ker_x;
                    CV_COPY( tbufc, src, width, x );
                }
                else
                {
                    CV_COPY( tbufc - ker_x, src - ker_x, ker_x, x );
                    CV_COPY( tbufc, src + width, ker_right, x );
                }

                /* make replication borders */
                {
                uchar pix = tsrc[ker_x];
                CV_SET( tsrc, pix, ker_x, x );

                pix = tsrc[width + ker_x - 1];
                CV_SET( tsrc + width + ker_x, pix, ker_right, x );
                }

                if( x_asymm )
                {
                    /* horizontal filter: asymmetric case */
                    if( x_type == ICV_m1_0_1_KERNEL )
                    {
                        for( i = 0; i < width; i++ )
                            tdst[i] = tsrc[i+2] - tsrc[i];
                    }
                    else
                    {
                        for( i = 0; i < width; i++ )
                        {
                            int j;
                            int t0 = tsrc[i + ker_x]*fmX0;

                            for( j = 1; j <= ker_x; j++ )
                                t0 += (tsrc[i+ker_x+j] - tsrc[i+ker_x-j])*fmaskX[j];

                            tdst[i] = t0;
                        }
                    }
                }
                else
                {
                    if( x_type == ICV_1_2_1_KERNEL )
                    {
                        for( i = 0; i < width; i++ )
                            tdst[i] = tsrc[i+1]*2 + tsrc[i] + tsrc[i+2];
                    }
                    else if( x_type == ICV_3_10_3_KERNEL )
                    {
                        for( i = 0; i < width; i++ )
                            tdst[i] = tsrc[i+1]*10 + (tsrc[i] + tsrc[i+2])*3;
                    }
                    else
                    {
                        /* horizontal filter: symmetric case */
                        for( i = 0; i < width; i++ )
                        {
                            int j;
                            int t0 = tsrc[i + ker_x]*fmX0;

                            for( j = 1; j <= ker_x; j++ )
                                t0 += (tsrc[i+ker_x+j] + tsrc[i+ker_x-j])*fmaskX[j];

                            tdst[i] = t0;
                        }
                    }
                }

                if( !need_copy )
                {
                    /* restore borders */
                    CV_COPY( src - ker_x, tbufc - ker_x, ker_x, x );
                    CV_COPY( src + width, tbufc, ker_right, x );
                }
            }
            else
            {
                CV_COPY( tdst, tsrc + ker_x, width, x );
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
                CV_COPY( tdst, trow, width, x );
            }
        }

        /* vertical convolution */
        if( crows != ker_height )
            break;

        tdst2 = dst;
        trow = rows[ker_y];

        if( ker_height > 1 )
        {
            if( width_rest )
            {
                need_copy = width < CV_MORPH_ALIGN || y == dst_height - 1;

                if( need_copy )
                    tdst2 = tbufw;
                else
                    CV_COPY( tbufw + width, dst + width, CV_MORPH_ALIGN, x );
            }

            if( y_asymm )
            {
                if( y_type == ICV_m1_0_1_KERNEL )
                {
                    int *trow1 = rows[origin*2], *trow2 = rows[(origin^1)*2];

                    for( x = 0; x < width; x += 4 )
                    {
                        int val0, val1;
                        val0 = trow2[x] - trow1[x];
                        val1 = trow2[x + 1] - trow1[x + 1];
                
                        tdst2[x + 0] = (short)val0;
                        tdst2[x + 1] = (short)val1;
                
                        val0 = trow2[x + 2] - trow1[x + 2];
                        val1 = trow2[x + 3] - trow1[x + 3];
                
                        tdst2[x + 2] = (short)val0;
                        tdst2[x + 3] = (short)val1;
                    }
                }
                else
                {
                    for( x = 0; x < width; x += 4 )
                    {
                        int val0, val1, val2, val3;

                        val0 = trow[x]*fmY0;
                        val1 = trow[x + 1]*fmY0;
                        val2 = trow[x + 2]*fmY0;
                        val3 = trow[x + 3]*fmY0;

                        for( i = 1; i <= ker_y; i++ )
                        {
                            int *trow1, *trow2;
                            int m = fmaskY[i];
                            trow1 = rows[ker_y - i];
                            trow2 = rows[ker_y + i];
                            val0 += (trow2[x] - trow1[x])*m;
                            val1 += (trow2[x+1] - trow1[x+1])*m;
                            val2 += (trow2[x+2] - trow1[x+2])*m;
                            val3 += (trow2[x+3] - trow1[x+3])*m;
                        }

                        tdst2[x + 0] = CV_CAST_16S(val0);
                        tdst2[x + 1] = CV_CAST_16S(val1);
                        tdst2[x + 2] = CV_CAST_16S(val2);
                        tdst2[x + 3] = CV_CAST_16S(val3);
                    }
                }
            }
            else
            {
                if( y_type == ICV_1_2_1_KERNEL )
                {
                    int *trow1 = rows[0], *trow2 = rows[2];

                    for( x = 0; x < width; x += 4 )
                    {
                        int val0, val1;
                        val0 = trow[x]*2 + trow1[x] + trow2[x];
                        val1 = trow[x + 1]*2 + trow1[x+1] + trow2[x+1];
                
                        tdst2[x + 0] = (short)val0;
                        tdst2[x + 1] = (short)val1;
                
                        val0 = trow[x + 2]*2 + trow1[x+2] + trow2[x+2];
                        val1 = trow[x + 3]*2 + trow1[x+3] + trow2[x+3];
                
                        tdst2[x + 2] = (short)val0;
                        tdst2[x + 3] = (short)val1;
                    }
                }
                else if( y_type == ICV_3_10_3_KERNEL )
                {
                    int *trow1 = rows[0], *trow2 = rows[2];

                    for( x = 0; x < width; x += 4 )
                    {
                        int val0, val1;
                        val0 = trow[x]*10 + (trow1[x] + trow2[x])*3;
                        val1 = trow[x + 1]*10 + (trow1[x+1] + trow2[x+1])*3;
                
                        tdst2[x + 0] = (short)val0;
                        tdst2[x + 1] = (short)val1;
                
                        val0 = trow[x + 2]*10 + (trow1[x+2] + trow2[x+2])*3;
                        val1 = trow[x + 3]*10 + (trow1[x+3] + trow2[x+3])*3;
                
                        tdst2[x + 2] = (short)val0;
                        tdst2[x + 3] = (short)val1;
                    }
                }
                else
                {
                    for( x = 0; x < width; x += 4 )
                    {
                        int val0, val1, val2, val3;

                        val0 = trow[x]*fmY0;
                        val1 = trow[x + 1]*fmY0;
                        val2 = trow[x + 2]*fmY0;
                        val3 = trow[x + 3]*fmY0;

                        for( i = 1; i <= ker_y; i++ )
                        {
                            int *trow1, *trow2;
                            int m = fmaskY[i];
                            trow1 = rows[ker_y - i];
                            trow2 = rows[ker_y + i];
                            val0 += (trow2[x] + trow1[x])*m;
                            val1 += (trow2[x+1] + trow1[x+1])*m;
                            val2 += (trow2[x+2] + trow1[x+2])*m;
                            val3 += (trow2[x+3] + trow1[x+3])*m;
                        }

                        tdst2[x + 0] = CV_CAST_16S(val0);
                        tdst2[x + 1] = CV_CAST_16S(val1);
                        tdst2[x + 2] = CV_CAST_16S(val2);
                        tdst2[x + 3] = CV_CAST_16S(val3);
                    }
                }
            }

            if( width_rest )
            {
                if( need_copy )
                    CV_COPY( dst, tbufw, width, x );
                else
                    CV_COPY( dst + width, tbufw + width, CV_MORPH_ALIGN, x );
            }
        }
        else
        {
            for( x = 0; x < width; x++ )
                dst[x] = (short)trow[x];
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


CvStatus CV_STDCALL icvSobel_32f_C1R( const float* pSrc, int srcStep,
                                      float* dst, int dstStep, CvSize* roiSize,
                                      CvFilterState* state, int stage )
{
    float* src = (float*)pSrc;
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
    float *tbufw = (float*)(state->tbuf);
    float *trow = 0;

    float* fmaskX = (float*)(state->ker0) + ker_x;
    float* fmaskY = (float*)(state->ker1) + ker_y;
    float fmX0 = fmaskX[0], fmY0 = fmaskY[0];

    int is_small_width = width < MAX( ker_x, ker_right );
    int starting_flag = 0;
    int width_rest = width & (CV_MORPH_ALIGN - 1);
    int origin = state->origin;
    int x_type = ICV_X_KERNEL_TYPE(state->kerType),
        y_type = ICV_Y_KERNEL_TYPE(state->kerType);
    int x_asymm = (x_type & 3) - 1, /* <0 - general kind (not used),
                                       0-symmetric, 1-asymmetric*/
        y_asymm = (y_type & 3) - 1;

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

    srcStep /= sizeof(src[0]);
    dstStep /= sizeof(dst[0]);

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
            tsrc = src - ker_x;
            tdst = rows[crows];

            if( src_height-- <= 0 )
            {
                if( stage != CV_WHOLE && stage != CV_END )
                    break;
                /* duplicate last row */
                trow = rows[crows - 1];
                CV_COPY( tdst, trow, width, x );
                continue;
            }

            need_copy |= src_height == 1;

            if( ker_width > 1 )
            {
                float* tbufc = (float*)tbufw;

                if( need_copy )
                {
                    tsrc = tbufc - ker_x;
                    CV_COPY( tbufc, src, width, x );
                }
                else
                {
                    CV_COPY( tbufc - ker_x, src - ker_x, ker_x, x );
                    CV_COPY( tbufc, src + width, ker_right, x );
                }

                /* make replication borders */
                {
                float pix = tsrc[ker_x];
                CV_SET( tsrc, pix, ker_x, x );

                pix = tsrc[width + ker_x - 1];
                CV_SET( tsrc + width + ker_x, pix, ker_right, x );
                }

                if( x_asymm )
                {
                    /* horizontal filter: asymmetric case */
                    if( x_type == ICV_m1_0_1_KERNEL )
                    {
                        for( i = 0; i < width; i++ )
                            tdst[i] = tsrc[i+2] - tsrc[i];
                    }
                    else
                    {
                        for( i = 0; i < width; i++ )
                        {
                            int j;
                            float t0 = tsrc[i + ker_x]*fmX0;

                            for( j = 1; j <= ker_x; j++ )
                                t0 += (tsrc[i+ker_x+j] - tsrc[i+ker_x-j])*fmaskX[j];

                            tdst[i] = t0;
                        }
                    }
                }
                else
                {
                    if( x_type == ICV_1_2_1_KERNEL )
                    {
                        for( i = 0; i < width; i++ )
                            tdst[i] = tsrc[i+1]*2 + tsrc[i] + tsrc[i+2];
                    }
                    else if( x_type == ICV_3_10_3_KERNEL )
                    {
                        for( i = 0; i < width; i++ )
                            tdst[i] = tsrc[i+1]*10 + (tsrc[i] + tsrc[i+2])*3;
                    }
                    else
                    {
                        /* horizontal filter: symmetric case */
                        for( i = 0; i < width; i++ )
                        {
                            int j;
                            double t0 = tsrc[i + ker_x]*fmX0;

                            for( j = 1; j <= ker_x; j++ )
                                t0 += (tsrc[i+ker_x+j] + tsrc[i+ker_x-j])*fmaskX[j];

                            tdst[i] = (float)t0;
                        }
                    }
                }

                if( !need_copy )
                {
                    /* restore borders */
                    CV_COPY( src - ker_x, tbufc - ker_x, ker_x, x );
                    CV_COPY( src + width, tbufc, ker_right, x );
                }
            }
            else
            {
                CV_COPY( tdst, tsrc + ker_x, width, x );
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
                CV_COPY( tdst, trow, width, x );
            }
        }

        /* vertical convolution */
        if( crows != ker_height )
            break;

        tdst2 = dst;
        trow = rows[ker_y];

        if( ker_height > 1 )
        {
            if( width_rest )
            {
                need_copy = width < CV_MORPH_ALIGN || y == dst_height - 1;

                if( need_copy )
                    tdst2 = tbufw;
                else
                    CV_COPY( tbufw + width, dst + width, CV_MORPH_ALIGN, x );
            }

            if( y_asymm )
            {
                if( y_type == ICV_m1_0_1_KERNEL )
                {
                    float *trow1 = rows[origin*2], *trow2 = rows[(origin^1)*2];

                    for( x = 0; x < width; x += 4 )
                    {
                        double val0, val1;
                        val0 = trow2[x] - trow1[x];
                        val1 = trow2[x + 1] - trow1[x + 1];
                
                        tdst2[x + 0] = (float)val0;
                        tdst2[x + 1] = (float)val1;
                
                        val0 = trow2[x + 2] - trow1[x + 2];
                        val1 = trow2[x + 3] - trow1[x + 3];
                
                        tdst2[x + 2] = (float)val0;
                        tdst2[x + 3] = (float)val1;
                    }
                }
                else
                {
                    for( x = 0; x < width; x += 4 )
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
                            val0 += (trow2[x] - trow1[x])*m;
                            val1 += (trow2[x+1] - trow1[x+1])*m;
                            val2 += (trow2[x+2] - trow1[x+2])*m;
                            val3 += (trow2[x+3] - trow1[x+3])*m;
                        }

                        tdst2[x + 0] = (float)val0;
                        tdst2[x + 1] = (float)val1;
                        tdst2[x + 2] = (float)val2;
                        tdst2[x + 3] = (float)val3;
                    }
                }
            }
            else
            {
                if( y_type == ICV_1_2_1_KERNEL )
                {
                    float *trow1 = rows[0], *trow2 = rows[2];

                    for( x = 0; x < width; x += 4 )
                    {
                        double val0, val1;
                        val0 = trow[x]*2 + trow1[x] + trow2[x];
                        val1 = trow[x + 1]*2 + trow1[x+1] + trow2[x+1];
                
                        tdst2[x + 0] = (float)val0;
                        tdst2[x + 1] = (float)val1;
                
                        val0 = trow[x + 2]*2 + trow1[x+2] + trow2[x+2];
                        val1 = trow[x + 3]*2 + trow1[x+3] + trow2[x+3];
                
                        tdst2[x + 2] = (float)val0;
                        tdst2[x + 3] = (float)val1;
                    }
                }
                else if( y_type == ICV_3_10_3_KERNEL )
                {
                    float *trow1 = rows[0], *trow2 = rows[2];

                    for( x = 0; x < width; x += 4 )
                    {
                        double val0, val1;
                        val0 = trow[x]*10 + (trow1[x] + trow2[x])*3;
                        val1 = trow[x + 1]*10 + (trow1[x+1] + trow2[x+1])*3;
                
                        tdst2[x + 0] = (float)val0;
                        tdst2[x + 1] = (float)val1;
                
                        val0 = trow[x + 2]*10 + (trow1[x+2] + trow2[x+2])*3;
                        val1 = trow[x + 3]*10 + (trow1[x+3] + trow2[x+3])*3;
                
                        tdst2[x + 2] = (float)val0;
                        tdst2[x + 3] = (float)val1;
                    }
                }
                else
                {
                    for( x = 0; x < width; x += 4 )
                    {
                        double val0, val1, val2, val3;

                        val0 = trow[x]*fmY0;
                        val1 = trow[x + 1]*fmY0;
                        val2 = trow[x + 2]*fmY0;
                        val3 = trow[x + 3]*fmY0;

                        for( i = 1; i <= ker_y; i++ )
                        {
                            float *trow1, *trow2;
                            float m = fmaskY[i];
                            trow1 = rows[ker_y - i];
                            trow2 = rows[ker_y + i];
                            val0 += (trow2[x] + trow1[x])*m;
                            val1 += (trow2[x+1] + trow1[x+1])*m;
                            val2 += (trow2[x+2] + trow1[x+2])*m;
                            val3 += (trow2[x+3] + trow1[x+3])*m;
                        }

                        tdst2[x + 0] = (float)val0;
                        tdst2[x + 1] = (float)val1;
                        tdst2[x + 2] = (float)val2;
                        tdst2[x + 3] = (float)val3;
                    }
                }
            }

            if( width_rest )
            {
                if( need_copy )
                    CV_COPY( dst, tbufw, width, x );
                else
                    CV_COPY( dst + width, tbufw + width, CV_MORPH_ALIGN, x );
            }
        }
        else
        {
            for( x = 0; x < width; x++ )
                dst[x] = (float)trow[x];
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
*                                      S C H A R R                                       *
\****************************************************************************************/

static CvStatus CV_STDCALL
icvScharrInitAlloc( int roiwidth, int datatype, int origin,
                    int dx, int dy, CvFilterState** state )
{
    return icvSobelInitAlloc( roiwidth, datatype, CV_SCHARR, origin, dx, dy, state );
}

static CvStatus CV_STDCALL
icvScharr_8u16s_C1R( const uchar* pSrc, int srcStep,
                     short* dst, int dstStep, CvSize* roiSize,
                     CvFilterState* state, int stage )
{
    assert( state->kerType == ICV_MAKE_SEPARABLE_KERNEL( ICV_m1_0_1_KERNEL, ICV_3_10_3_KERNEL ) ||
            state->kerType == ICV_MAKE_SEPARABLE_KERNEL( ICV_3_10_3_KERNEL, ICV_m1_0_1_KERNEL ));
    return icvSobel_8u16s_C1R( pSrc, srcStep, dst, dstStep, roiSize, state, stage );
}


static CvStatus CV_STDCALL
icvScharr_32f_C1R( const float* pSrc, int srcStep,
                   float* dst, int dstStep, CvSize* roiSize,
                   CvFilterState* state, int stage )
{
    assert( state->kerType == ICV_MAKE_SEPARABLE_KERNEL( ICV_m1_0_1_KERNEL, ICV_3_10_3_KERNEL ) ||
            state->kerType == ICV_MAKE_SEPARABLE_KERNEL( ICV_3_10_3_KERNEL, ICV_m1_0_1_KERNEL ));
    return icvSobel_32f_C1R( pSrc, srcStep, dst, dstStep, roiSize, state, stage );
}

/****************************************************************************************\
*                                      L A P L A C E                                     *
\****************************************************************************************/

static CvStatus CV_STDCALL
icvLaplaceInitAlloc( int roiwidth, int datatype,
                     int size, CvFilterState** state )
{
    #define MAX_KERNEL_SIZE  7
    int ker[MAX_KERNEL_SIZE*2+1];
    CvDataType worktype = datatype != cv32f ? cv32s : cv32f;
    CvStatus status;
    int x_filter_type, y_filter_type;
    int x_size = size;
    
    if( !state )
        return CV_NULLPTR_ERR;

    if( (size&1) == 0 || size < 1 || size > MAX_KERNEL_SIZE )
        return CV_BADRANGE_ERR;

    if( size == 1 )
        x_size = 3;

    x_filter_type = icvCalcKer( (char*)ker, 2, x_size, worktype, 0 );
    y_filter_type = icvCalcKer( (char*)(ker + x_size), 0, size, worktype, 0 );

    {
    CvSize element_size = { x_size, x_size };
    CvPoint element_anchor = { x_size/2, x_size/2 };

    status = icvFilterInitAlloc( roiwidth, worktype, 2, element_size, element_anchor, ker,
                                 ICV_MAKE_SEPARABLE_KERNEL(x_filter_type, y_filter_type), state );
    }
    if( status < 0 )
        return status;

    (*state)->origin = 0;

    return CV_OK;
}


static CvStatus CV_STDCALL
icvLaplace_8u16s_C1R( const uchar* pSrc, int srcStep,
                      short* dst, int dstStep, CvSize* roiSize,
                      CvFilterState* state, int stage )
{
    uchar* src = (uchar*)pSrc;
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
    short *tbufw = (short*)(state->tbuf);
    int *trow = 0;

    int* fmaskX = (int*)(state->ker0) + ker_x;
    int* fmaskY = (int*)(state->ker1) + ker_y;
    int fmX0 = fmaskX[0], fmY0 = fmaskY[0];

    int is_small_width = width < MAX( ker_x, ker_right );
    int starting_flag = 0;
    int width_rest = width & (CV_MORPH_ALIGN - 1);
    int y_type = ICV_Y_KERNEL_TYPE(state->kerType);

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

    dstStep /= sizeof(dst[0]);

    do
    {
        int need_copy = is_small_width | (y == 0);
        uchar *tsrc;
        int   *tdst;
        short *tdst2;
        int   *saved_row = rows[ker_y];

        /* fill cyclic buffer - horizontal filtering */
        for( ; crows < ker_height; crows++ )
        {
            tsrc = src - ker_x;
            tdst = rows[crows];

            if( src_height-- <= 0 )
            {
                if( stage != CV_WHOLE && stage != CV_END )
                    break;
                /* duplicate last row */
                trow = rows[crows - 1];
                CV_COPY( tdst, trow, width*2, x );
                continue;
            }

            need_copy |= src_height == 1;

            {
                uchar* tbufc = (uchar*)tbufw;

                if( need_copy )
                {
                    tsrc = tbufc - ker_x;
                    CV_COPY( tbufc, src, width, x );
                }
                else
                {
                    CV_COPY( tbufc - ker_x, src - ker_x, ker_x, x );
                    CV_COPY( tbufc, src + width, ker_right, x );
                }

                /* make replication borders */
                {
                uchar pix = tsrc[ker_x];
                CV_SET( tsrc, pix, ker_x, x );

                pix = tsrc[width + ker_x - 1];
                CV_SET( tsrc + width + ker_x, pix, ker_right, x );
                }

                if( ker_width == 3 )
                {
                    if( y_type == ICV_1_2_1_KERNEL )
                    {
                        for( i = 0; i < width; i++ )
                        {
                            int t0 = tsrc[i] + tsrc[i+2] - tsrc[i+1]*2;
                            int t1 = tsrc[i] + tsrc[i+2] + tsrc[i+1]*2;
                            tdst[i] = t0;
                            tdst[i+width] = t1;
                        }
                    }
                    else
                    {
                        for( i = 0; i < width; i++ )
                        {
                            int t0 = tsrc[i] + tsrc[i+2] - tsrc[i+1]*2;
                            int t1 = tsrc[i+1];
                            tdst[i] = t0;
                            tdst[i+width] = t1;
                        }
                    }
                }
                else if( ker_width == 5 )
                {
                    for( i = 0; i < width; i++ )
                    {
                        int t0 = tsrc[i] + tsrc[i+4] - tsrc[i+2]*2;
                        int t1 = tsrc[i] + tsrc[i+4] + tsrc[i+2]*6 +
                                 (tsrc[i+1] + tsrc[i+3])*4;
                        tdst[i] = t0;
                        tdst[i+width] = t1;
                    }
                }
                else
                {
                    for( i = 0; i < width; i++ )
                    {
                        int j;
                        int t0 = tsrc[i + ker_x]*fmX0;
                        int t1 = tsrc[i + ker_x]*fmY0;

                        for( j = 1; j <= ker_x; j++ )
                        {
                            t0 += (tsrc[i+ker_x+j] + tsrc[i+ker_x-j])*fmaskX[j];
                            t1 += (tsrc[i+ker_x+j] + tsrc[i+ker_x-j])*fmaskY[j];
                        }

                        tdst[i] = t0;
                        tdst[i+width] = t1;
                    }
                }

                if( !need_copy )
                {
                    /* restore borders */
                    CV_COPY( src - ker_x, tbufc - ker_x, ker_x, x );
                    CV_COPY( src + width, tbufc, ker_right, x );
                }
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
                CV_COPY( tdst, trow, width*2, x );
            }
        }

        /* vertical convolution */
        if( crows != ker_height )
            break;

        tdst2 = dst;

        if( width_rest )
        {
            need_copy = width < CV_MORPH_ALIGN || y == dst_height - 1;

            if( need_copy )
                tdst2 = tbufw;
            else
                CV_COPY( tbufw + width, dst + width, CV_MORPH_ALIGN, x );
        }

        trow = rows[ker_y];

        if( ker_height == 3 )
        {
            int *trow1 = rows[0], *trow2 = rows[2];

            if( y_type == ICV_1_2_1_KERNEL )
            {
                for( x = 0; x < width; x += 4 )
                {
                    int val0, val1;
                    val0 = trow[x]*2 + trow1[x] + trow2[x] -
                           trow[x+width]*2 + trow1[x+width] + trow2[x+width];
                    val1 = trow[x+1]*2 + trow1[x+1] + trow2[x+1] -
                           trow[x+1+width]*2 + trow1[x+1+width] + trow2[x+1+width];
            
                    tdst2[x + 0] = (short)val0;
                    tdst2[x + 1] = (short)val1;
            
                    val0 = trow[x+2]*2 + trow1[x+2] + trow2[x+2] -
                           trow[x+2+width]*2 + trow1[x+2+width] + trow2[x+2+width];
                    val1 = trow[x+3]*2 + trow1[x+3] + trow2[x+3] -
                           trow[x+3+width]*2 + trow1[x+3+width] + trow2[x+3+width];
            
                    tdst2[x + 2] = (short)val0;
                    tdst2[x + 3] = (short)val1;
                }
            }
            else
            {
                for( x = 0; x < width; x += 4 )
                {
                    int val0, val1;
                    val0 = trow[x] -
                           trow[x+width]*2 + trow1[x+width] + trow2[x+width];
                    val1 = trow[x+1] -
                           trow[x+1+width]*2 + trow1[x+1+width] + trow2[x+1+width];
            
                    tdst2[x + 0] = (short)val0;
                    tdst2[x + 1] = (short)val1;
            
                    val0 = trow[x+2] -
                           trow[x+2+width]*2 + trow1[x+2+width] + trow2[x+2+width];
                    val1 = trow[x+3] -
                           trow[x+3+width]*2 + trow1[x+3+width] + trow2[x+3+width];
            
                    tdst2[x + 2] = (short)val0;
                    tdst2[x + 3] = (short)val1;
                }
            }
        }
        else if( ker_height == 5 )
        {
            int *trow0 = rows[0], *trow1 = rows[1], *trow3 = rows[3], *trow4 = rows[4];

            for( x = 0; x < width; x += 4 )
            {
                int val0, val1;
                val0 = trow0[x] + trow4[x] + (trow1[x] + trow3[x])*4 + trow[x]*6 +
                       trow0[x+width] + trow4[x+width] - 2*trow[x+width];
                val1 = trow0[x+1] + trow4[x+1] + (trow1[x+1] + trow3[x+1])*4 + trow[x+1]*6 +
                       trow0[x+1+width] + trow4[x+1+width] - 2*trow[x+1+width];
            
                tdst2[x + 0] = (short)val0;
                tdst2[x + 1] = (short)val1;
            
                val0 = trow0[x+2] + trow4[x+2] + (trow1[x+2] + trow3[x+2])*4 + trow[x+2]*6 +
                       trow0[x+2+width] + trow4[x+2+width] - 2*trow[x+2+width];
                val1 = trow0[x+3] + trow4[x+3] + (trow1[x+3] + trow3[x+3])*4 + trow[x+3]*6 +
                       trow0[x+3+width] + trow4[x+3+width] - 2*trow[x+3+width];
            
                tdst2[x + 2] = (short)val0;
                tdst2[x + 3] = (short)val1;
            }
        }
        else
        {
            for( x = 0; x < width; x += 4 )
            {
                int val0, val1, val2, val3;

                val0 = trow[x]*fmY0 + trow[x + width]*fmX0;
                val1 = trow[x + 1]*fmY0 + trow[x + 1 + width]*fmX0;
                val2 = trow[x + 2]*fmY0 + trow[x + 2 + width]*fmX0;
                val3 = trow[x + 3]*fmY0 + trow[x + 3 + width]*fmX0;

                for( i = 1; i <= ker_y; i++ )
                {
                    int *trow1, *trow2;
                    int m0 = fmaskY[i], m1 = fmaskX[i];
                    trow1 = rows[ker_y - i];
                    trow2 = rows[ker_y + i];
                    val0 += (trow2[x] + trow1[x])*m0 +
                            (trow2[x+width] + trow1[x+width])*m1;
                    val1 += (trow2[x+1] + trow1[x+1])*m0 +
                            (trow2[x+1+width] + trow1[x+1+width])*m1;
                    val2 += (trow2[x+2] + trow1[x+2])*m0 +
                            (trow2[x+2+width] + trow1[x+2+width])*m1;
                    val3 += (trow2[x+3] + trow1[x+3])*m0 +
                            (trow2[x+3+width] + trow1[x+3+width])*m1;
                }

                tdst2[x + 0] = CV_CAST_16S(val0);
                tdst2[x + 1] = CV_CAST_16S(val1);
                tdst2[x + 2] = CV_CAST_16S(val2);
                tdst2[x + 3] = CV_CAST_16S(val3);
            }
        }

        if( width_rest )
        {
            if( need_copy )
                CV_COPY( dst, tbufw, width, x );
            else
                CV_COPY( dst + width, tbufw + width, CV_MORPH_ALIGN, x );
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
icvLaplace_32f_C1R( const float* pSrc, int srcStep,
                    float* dst, int dstStep, CvSize* roiSize,
                    CvFilterState* state, int stage )
{
    float* src = (float*)pSrc;
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
    float**rows = (float**)(state->rows);
    float *tbufw = (float*)(state->tbuf);
    float*trow = 0;

    float* fmaskX = (float*)(state->ker0) + ker_x;
    float* fmaskY = (float*)(state->ker1) + ker_y;
    float fmX0 = fmaskX[0], fmY0 = fmaskY[0];

    int is_small_width = width < MAX( ker_x, ker_right );
    int starting_flag = 0;
    int width_rest = width & (CV_MORPH_ALIGN - 1);
    int y_type = ICV_Y_KERNEL_TYPE(state->kerType);

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

    srcStep /= sizeof(src[0]);
    dstStep /= sizeof(dst[0]);

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
            tsrc = src - ker_x;
            tdst = rows[crows];

            if( src_height-- <= 0 )
            {
                if( stage != CV_WHOLE && stage != CV_END )
                    break;
                /* duplicate last row */
                trow = rows[crows - 1];
                CV_COPY( tdst, trow, width*2, x );
                continue;
            }

            need_copy |= src_height == 1;

            {
                float* tbufc = (float*)tbufw;

                if( need_copy )
                {
                    tsrc = tbufc - ker_x;
                    CV_COPY( tbufc, src, width, x );
                }
                else
                {
                    CV_COPY( tbufc - ker_x, src - ker_x, ker_x, x );
                    CV_COPY( tbufc, src + width, ker_right, x );
                }

                /* make replication borders */
                {
                float pix = tsrc[ker_x];
                CV_SET( tsrc, pix, ker_x, x );

                pix = tsrc[width + ker_x - 1];
                CV_SET( tsrc + width + ker_x, pix, ker_right, x );
                }

                if( ker_width == 3 )
                {
                    if( y_type == ICV_1_2_1_KERNEL )
                    {
                        for( i = 0; i < width; i++ )
                        {
                            float t0 = tsrc[i] + tsrc[i+2] - tsrc[i+1]*2;
                            float t1 = tsrc[i] + tsrc[i+2] + tsrc[i+1]*2;
                            tdst[i] = t0;
                            tdst[i+width] = t1;
                        }
                    }
                    else
                    {
                        for( i = 0; i < width; i++ )
                        {
                            float t0 = tsrc[i] + tsrc[i+2] - tsrc[i+1]*2;
                            float t1 = tsrc[i+1];
                            tdst[i] = t0;
                            tdst[i+width] = t1;
                        }
                    }
                }
                else if( ker_width == 5 )
                {
                    for( i = 0; i < width; i++ )
                    {
                        float t0 = tsrc[i] + tsrc[i+4] - tsrc[i+2]*2;
                        float t1 = tsrc[i] + tsrc[i+4] + tsrc[i+2]*6 +
                                   (tsrc[i+1] + tsrc[i+3])*4;
                        tdst[i] = t0;
                        tdst[i+width] = t1;
                    }
                }
                else
                {
                    for( i = 0; i < width; i++ )
                    {
                        int j;
                        double t0 = tsrc[i + ker_x]*fmX0;
                        double t1 = tsrc[i + ker_x]*fmY0;

                        for( j = 1; j <= ker_x; j++ )
                        {
                            t0 += (tsrc[i+ker_x+j] + tsrc[i+ker_x-j])*fmaskX[j];
                            t1 += (tsrc[i+ker_x+j] + tsrc[i+ker_x-j])*fmaskY[j];
                        }

                        tdst[i] = (float)t0;
                        tdst[i+width] = (float)t1;
                    }
                }

                if( !need_copy )
                {
                    /* restore borders */
                    CV_COPY( src - ker_x, tbufc - ker_x, ker_x, x );
                    CV_COPY( src + width, tbufc, ker_right, x );
                }
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
                CV_COPY( tdst, trow, width*2, x );
            }
        }

        /* vertical convolution */
        if( crows != ker_height )
            break;

        tdst2 = dst;

        if( width_rest )
        {
            need_copy = width < CV_MORPH_ALIGN || y == dst_height - 1;

            if( need_copy )
                tdst2 = tbufw;
            else
                CV_COPY( tbufw + width, dst + width, CV_MORPH_ALIGN, x );
        }

        trow = rows[ker_y];

        if( ker_height == 3 )
        {
            float* trow1 = rows[0], *trow2 = rows[2];

            if( y_type == ICV_1_2_1_KERNEL )
            {
                for( x = 0; x < width; x += 4 )
                {
                    float val0, val1;
                    val0 = trow[x]*2 + trow1[x] + trow2[x] -
                           trow[x+width]*2 + trow1[x+width] + trow2[x+width];
                    val1 = trow[x+1]*2 + trow1[x+1] + trow2[x+1] -
                           trow[x+1+width]*2 + trow1[x+1+width] + trow2[x+1+width];
            
                    tdst2[x + 0] = (float)val0;
                    tdst2[x + 1] = (float)val1;
            
                    val0 = trow[x+2]*2 + trow1[x+2] + trow2[x+2] -
                           trow[x+2+width]*2 + trow1[x+2+width] + trow2[x+2+width];
                    val1 = trow[x+3]*2 + trow1[x+3] + trow2[x+3] -
                           trow[x+3+width]*2 + trow1[x+3+width] + trow2[x+3+width];
            
                    tdst2[x + 2] = (float)val0;
                    tdst2[x + 3] = (float)val1;
                }
            }
            else
            {
                for( x = 0; x < width; x += 4 )
                {
                    float val0, val1;
                    val0 = trow[x] -
                           trow[x+width]*2 + trow1[x+width] + trow2[x+width];
                    val1 = trow[x+1] -
                           trow[x+1+width]*2 + trow1[x+1+width] + trow2[x+1+width];
            
                    tdst2[x + 0] = (float)val0;
                    tdst2[x + 1] = (float)val1;
            
                    val0 = trow[x+2] -
                           trow[x+2+width]*2 + trow1[x+2+width] + trow2[x+2+width];
                    val1 = trow[x+3] -
                           trow[x+3+width]*2 + trow1[x+3+width] + trow2[x+3+width];
            
                    tdst2[x + 2] = (float)val0;
                    tdst2[x + 3] = (float)val1;
                }
            }
        }
        else if( ker_height == 5 )
        {
            float*trow0 = rows[0], *trow1 = rows[1], *trow3 = rows[3], *trow4 = rows[4];

            for( x = 0; x < width; x += 4 )
            {
                float val0, val1;
                val0 = trow0[x] + trow4[x] + (trow1[x] + trow3[x])*4 + trow[x]*6 +
                       trow0[x+width] + trow4[x+width] - 2*trow[x+width];
                val1 = trow0[x+1] + trow4[x+1] + (trow1[x+1] + trow3[x+1])*4 + trow[x+1]*6 +
                       trow0[x+1+width] + trow4[x+1+width] - 2*trow[x+1+width];
            
                tdst2[x + 0] = (float)val0;
                tdst2[x + 1] = (float)val1;
            
                val0 = trow0[x+2] + trow4[x+2] + (trow1[x+2] + trow3[x+2])*4 + trow[x+2]*6 +
                       trow0[x+2+width] + trow4[x+2+width] - 2*trow[x+2+width];
                val1 = trow0[x+3] + trow4[x+3] + (trow1[x+3] + trow3[x+3])*4 + trow[x+3]*6 +
                       trow0[x+3+width] + trow4[x+3+width] - 2*trow[x+3+width];
            
                tdst2[x + 2] = (float)val0;
                tdst2[x + 3] = (float)val1;
            }
        }
        else
        {
            for( x = 0; x < width; x += 4 )
            {
                double val0, val1, val2, val3;

                val0 = trow[x]*fmY0 + trow[x + width]*fmX0;
                val1 = trow[x + 1]*fmY0 + trow[x + 1 + width]*fmX0;
                val2 = trow[x + 2]*fmY0 + trow[x + 2 + width]*fmX0;
                val3 = trow[x + 3]*fmY0 + trow[x + 3 + width]*fmX0;

                for( i = 1; i <= ker_y; i++ )
                {
                    float *trow1, *trow2;
                    double m0 = fmaskY[i], m1 = fmaskX[i];
                    trow1 = rows[ker_y - i];
                    trow2 = rows[ker_y + i];
                    val0 += (trow2[x] + trow1[x])*m0 +
                            (trow2[x+width] + trow1[x+width])*m1;
                    val1 += (trow2[x+1] + trow1[x+1])*m0 +
                            (trow2[x+1+width] + trow1[x+1+width])*m1;
                    val2 += (trow2[x+2] + trow1[x+2])*m0 +
                            (trow2[x+2+width] + trow1[x+2+width])*m1;
                    val3 += (trow2[x+3] + trow1[x+3])*m0 +
                            (trow2[x+3+width] + trow1[x+3+width])*m1;
                }

                tdst2[x + 0] = (float)val0;
                tdst2[x + 1] = (float)val1;
                tdst2[x + 2] = (float)val2;
                tdst2[x + 3] = (float)val3;
            }
        }

        if( width_rest )
        {
            if( need_copy )
                CV_COPY( dst, tbufw, width, x );
            else
                CV_COPY( dst + width, tbufw + width, CV_MORPH_ALIGN, x );
        }

        rows[ker_y] = saved_row;

        /* rotate buffer */
        {
            float*t = rows[0];

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


/****************************************************************************************/

/* lightweight convolution with 3x3 kernel */
void icvSepConvSmall3_32f( float*  src, int src_step,
                           float*  dst, int dst_step,
                           CvSize src_size,
                           const float* kx, const float* ky,
                           float*  buffer )
{
    int  dst_width, buffer_step = 0;
    int  x, y;
    
    assert( src && dst && src_size.width > 2 && src_size.height > 2 &&
            (src_step & 3) == 0 && (dst_step & 3) == 0 &&
            (kx || ky) && (buffer || !kx || !ky));

    src_step >>= 2;
    dst_step >>= 2;

    dst_width = src_size.width - 2;

    if( !kx )
    {
        /* set vars, so that vertical convolution 
           will write results into destination ROI and
           horizontal convolution won't run */
        src_size.width = dst_width;
        buffer_step = dst_step;
        buffer = dst;
        dst_width = 0;
    }

    assert( src_step >= src_size.width && dst_step >= dst_width );
    
    src_size.height -= 3;
    if( !ky )
    {
        /* set vars, so that vertical convolution won't run and 
           horizontal convolution will write results into destination ROI and */
        src_size.height += 3;
        buffer_step = src_step;
        buffer = src;
        src_size.width = 0;
    }

    for( y = 0; y <= src_size.height; y++, src += src_step,
                                           dst += dst_step,
                                           buffer += buffer_step )
    {
        float* src2 = src + src_step;
        float* src3 = src + src_step*2;
        for( x = 0; x < src_size.width; x++ )
        {
            buffer[x] = (float)(ky[0]*src[x] + ky[1]*src2[x] + ky[2]*src3[x]);
        }

        for( x = 0; x < dst_width; x++ )
        {
            dst[x] = (float)(kx[0]*buffer[x] + kx[1]*buffer[x+1] + kx[2]*buffer[x+2]);
        }
    }
}


/****************************************************************************************\
*                                   External Functions                                   *
\****************************************************************************************/

#define  ICV_DEF_INIT_DERIV_TAB( FUNCNAME )                \
static  void  icvInit##FUNCNAME##Table( CvFuncTable* tab ) \
{                                                          \
    tab->fn_2d[CV_8U] = (void*)icv##FUNCNAME##_8u16s_C1R;  \
    tab->fn_2d[CV_8S] = 0;                                 \
    tab->fn_2d[CV_32F] = (void*)icv##FUNCNAME##_32f_C1R;   \
}

ICV_DEF_INIT_DERIV_TAB( Sobel )
ICV_DEF_INIT_DERIV_TAB( Scharr )
ICV_DEF_INIT_DERIV_TAB( Laplace )


////////////////////////////////// IPP derivative filters ////////////////////////////////

icvFilterSobelVert_8u16s_C1R_t icvFilterSobelVert_8u16s_C1R_p = 0;
icvFilterSobelHoriz_8u16s_C1R_t icvFilterSobelHoriz_8u16s_C1R_p = 0;
icvFilterSobelVertSecond_8u16s_C1R_t icvFilterSobelVertSecond_8u16s_C1R_p = 0;
icvFilterSobelHorizSecond_8u16s_C1R_t icvFilterSobelHorizSecond_8u16s_C1R_p = 0;
icvFilterSobelCross_8u16s_C1R_t icvFilterSobelCross_8u16s_C1R_p = 0;

icvFilterSobelVert_32f_C1R_t icvFilterSobelVert_32f_C1R_p = 0;
icvFilterSobelHoriz_32f_C1R_t icvFilterSobelHoriz_32f_C1R_p = 0;
icvFilterSobelVertSecond_32f_C1R_t icvFilterSobelVertSecond_32f_C1R_p = 0;
icvFilterSobelHorizSecond_32f_C1R_t icvFilterSobelHorizSecond_32f_C1R_p = 0;
icvFilterSobelCross_32f_C1R_t icvFilterSobelCross_32f_C1R_p = 0;

icvFilterScharrVert_8u16s_C1R_t icvFilterScharrVert_8u16s_C1R_p = 0;
icvFilterScharrHoriz_8u16s_C1R_t icvFilterScharrHoriz_8u16s_C1R_p = 0;
icvFilterScharrVert_32f_C1R_t icvFilterScharrVert_32f_C1R_p = 0;
icvFilterScharrHoriz_32f_C1R_t icvFilterScharrHoriz_32f_C1R_p = 0;

//////////////////////////////////////////////////////////////////////////////////////////

CV_IMPL void
cvSobel( const void* srcarr, void* dstarr, int dx, int dy, int aperture_size )
{
    static CvFuncTable sobel_tab, scharr_tab;
    static int inittab = 0;
    CvMat* temp = 0;

    CvFilterState *state = 0;
    CV_FUNCNAME( "cvSobel" );

    __BEGIN__;

    int origin = 0;
    int depth;
    CvMat srcstub, *src = (CvMat*)srcarr;
    CvMat dststub, *dst = (CvMat*)dstarr;
    CvFilterFunc func = 0;
    CvSize size;
    int datatype;

    if( !inittab )
    {
        icvInitSobelTable( &sobel_tab );
        icvInitScharrTable( &scharr_tab );
        inittab = 1;
    }

    CV_CALL( src = cvGetMat( src, &srcstub ));
    CV_CALL( dst = cvGetMat( dst, &dststub ));

    if( CV_IS_IMAGE_HDR( srcarr ))
        origin = ((IplImage*)srcarr)->origin;

    if( CV_MAT_CN( src->type ) != 1 || CV_MAT_CN( dst->type ) != 1 )
        CV_ERROR( CV_BadNumChannels, cvUnsupportedFormat );

    if( CV_MAT_DEPTH( src->type ) == CV_8U )
    {
        if( CV_MAT_DEPTH( dst->type ) != CV_16S )
            CV_ERROR( CV_StsUnmatchedFormats,
            "Destination array should be 16-bit signed when the source is 8-bit" );
    }
    else if( CV_MAT_DEPTH( src->type ) == CV_32F )
    {
        if( CV_MAT_DEPTH( dst->type ) != CV_32F )
            CV_ERROR( CV_StsUnmatchedFormats,
            "Destination array should be 32-bit floating point "
            "when the source is 32-bit floating point" );
    }
    else
        CV_ERROR( CV_StsUnsupportedFormat, "Unsupported source array format" );

    depth = CV_MAT_DEPTH( src->type );
    datatype = icvDepthToDataType(depth);

    if( !CV_ARE_SIZES_EQ( src, dst ))
        CV_ERROR( CV_StsBadArg, "src and dst have different sizes" );

    size = cvGetMatSize(src);

    if( ((aperture_size == CV_SCHARR || aperture_size == 3 || aperture_size == 5) &&
        dx <= 1 && dy <= 1 && icvFilterSobelVert_8u16s_C1R_p))
    {
        CvSobelFixedIPPFunc ipp_sobel_func = 0;
        CvFilterFixedIPPFunc ipp_scharr_func = 0;

        if( dx == 1 && dy == 0 && aperture_size == CV_SCHARR )
            ipp_scharr_func = depth == CV_8U ?
                icvFilterScharrVert_8u16s_C1R_p : icvFilterScharrVert_32f_C1R_p;
        else if( dx == 0 && dy == 1 && aperture_size == CV_SCHARR )
            ipp_scharr_func = depth == CV_8U ?
                icvFilterScharrHoriz_8u16s_C1R_p : icvFilterScharrHoriz_32f_C1R_p;
        else if( dx == 1 && dy == 0 )
            ipp_sobel_func = depth == CV_8U ?
                icvFilterSobelVert_8u16s_C1R_p : icvFilterSobelVert_32f_C1R_p;
        else if( dx == 0 && dy == 1 )
            ipp_sobel_func = depth == CV_8U ?
                icvFilterSobelHoriz_8u16s_C1R_p : icvFilterSobelHoriz_32f_C1R_p;
        else if( dx == 2 && dy == 0 )
            ipp_sobel_func = depth == CV_8U ?
                icvFilterSobelVertSecond_8u16s_C1R_p : icvFilterSobelVertSecond_32f_C1R_p;
        else if( dx == 1 && dy == 1 )
            ipp_sobel_func = depth == CV_8U ?
                icvFilterSobelCross_8u16s_C1R_p : icvFilterSobelCross_32f_C1R_p;
        else if( dx == 0 && dy == 2 )
            ipp_sobel_func = depth == CV_8U ?
                icvFilterSobelHorizSecond_8u16s_C1R_p : icvFilterSobelHorizSecond_32f_C1R_p;

        if( ipp_sobel_func || ipp_scharr_func )
        {
            int need_to_negate = (dx == 1 && aperture_size != CV_SCHARR) ^ ((dy == 1) && origin);
            aperture_size = aperture_size == CV_SCHARR ? 3 : aperture_size;
            CvSize el_size = { aperture_size, aperture_size };
            CvPoint el_anchor = { aperture_size/2, aperture_size/2 };
            int stripe_buf_size = 1 << 15; // the optimal value may depend on CPU cache,
                                           // overhead of current IPP code etc.
            const uchar* shifted_ptr;
            int y, dy = 0;
            int temp_step;
            int dst_step = dst->step ? dst->step : CV_STUB_STEP;
            CvSize stripe_size;

            CV_CALL( temp = icvIPPFilterInit( src, stripe_buf_size, el_size ));
            
            shifted_ptr = temp->data.ptr +
                el_anchor.y*temp->step + el_anchor.x*CV_ELEM_SIZE(depth);
            temp_step = temp->step ? temp->step : CV_STUB_STEP;

            for( y = 0; y < src->rows; y += dy )
            {
                dy = icvIPPFilterNextStripe( src, temp, y, el_size, el_anchor );
                stripe_size.width = size.width;
                stripe_size.height = dy;

                if( ipp_sobel_func )
                {
                    IPPI_CALL( ipp_sobel_func( shifted_ptr, temp_step,
                            dst->data.ptr + y*dst_step, dst_step,
                            stripe_size, aperture_size*10 + aperture_size ));
                }
                else
                {
                    IPPI_CALL( ipp_scharr_func( shifted_ptr, temp_step,
                            dst->data.ptr + y*dst_step, dst_step, stripe_size ));
                }
            }

            if( need_to_negate )
                cvSubRS( dst, cvScalarAll(0), dst );
            EXIT;
        }
    }

    if( aperture_size == CV_SCHARR )
    {
        IPPI_CALL( icvScharrInitAlloc( src->width, datatype, origin, dx, dy, &state ));
        func = (CvFilterFunc)(scharr_tab.fn_2d[depth]);
    }
    else
    {
        IPPI_CALL( icvSobelInitAlloc( src->width, datatype, aperture_size,
                                      origin, dx, dy, &state ));
        func = (CvFilterFunc)(sobel_tab.fn_2d[depth]);
    }

    if( !func )
        CV_ERROR( CV_StsUnsupportedFormat, "" );

    IPPI_CALL( func( src->data.ptr, src->step, dst->data.ptr,
                     dst->step, &size, state, 0 ));

    __END__;

    cvReleaseMat( &temp );
    icvFilterFree( &state );
}


CV_IMPL void
cvLaplace( const void* srcarr, void* dstarr, int aperture_size )
{
    static CvFuncTable laplace_tab;
    static int inittab = 0;
    
    CV_FUNCNAME( "cvLaplace" );

    CvFilterState *state = 0;

    __BEGIN__;

    int depth;
    CvMat srcstub, *src = (CvMat*)srcarr;
    CvMat dststub, *dst = (CvMat*)dstarr;
    CvFilterFunc func = 0;
    CvSize size;

    if( !inittab )
    {
        icvInitLaplaceTable( &laplace_tab );
        inittab = 1;
    }

    CV_CALL( src = cvGetMat( src, &srcstub ));
    CV_CALL( dst = cvGetMat( dst, &dststub ));

    if( CV_MAT_CN( src->type ) != 1 || CV_MAT_CN( dst->type ) != 1 )
        CV_ERROR( CV_BadNumChannels, cvUnsupportedFormat );

    if( CV_MAT_DEPTH( src->type ) == CV_8U )
    {
        if( CV_MAT_DEPTH( dst->type ) != CV_16S )
            CV_ERROR( CV_StsUnmatchedFormats, "Destination should have 16s format"
                                              " when source has 8u or 8s format" );
    }
    else if( CV_MAT_DEPTH( src->type ) == CV_32F )
    {
        if( CV_MAT_DEPTH( dst->type ) != CV_32F )
            CV_ERROR( CV_StsUnmatchedFormats, "Destination should have 16s format"
                                              " when source has 8u or 8s format" );
    }
    else
    {
        CV_ERROR( CV_StsUnsupportedFormat, "Unsupported source array format" );
    }

    depth = CV_MAT_DEPTH( src->type );

    if( !CV_ARE_SIZES_EQ( src, dst ))
        CV_ERROR( CV_StsBadArg, "src and dst have different sizes" );

    IPPI_CALL( icvLaplaceInitAlloc( src->width, icvDepthToDataType(depth), aperture_size, &state ));
    func = (CvFilterFunc)(laplace_tab.fn_2d[depth]);

    if( !func )
        CV_ERROR( CV_StsUnsupportedFormat, "" );

    size = cvGetMatSize(src);
    IPPI_CALL( func( src->data.ptr, src->step, dst->data.ptr,
                     dst->step, &size, state, 0 ));

    __END__;

    icvFilterFree( &state );
}

/* End of file. */


