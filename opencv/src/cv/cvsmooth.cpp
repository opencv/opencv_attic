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
                                         Box Filter
\****************************************************************************************/

static void icvSumRow_8u32s( const uchar* src0, int* dst, void* params );
static void icvSumRow_32f64f( const float* src0, double* dst, void* params );
static void icvSumCol_32s8u( const int** src, uchar* dst, int dst_step,
                             int count, void* params );
static void icvSumCol_32s16s( const int** src, short* dst, int dst_step,
                             int count, void* params );
static void icvSumCol_32s32s( const int** src, int* dst, int dst_step,
                             int count, void* params );
static void icvSumCol_64f32f( const double** src, float* dst, int dst_step,
                              int count, void* params );

CvBoxFilter::CvBoxFilter()
{
    min_depth = CV_32S;
    sum = 0;
    sum_count = 0;
    normalized = false;
}


CvBoxFilter::CvBoxFilter( int _max_width, int _src_type, int _dst_type,
                          bool _normalized, CvSize _ksize,
                          CvPoint _anchor, int _border_mode,
                          CvScalar _border_value )
{
    min_depth = CV_32S;
    sum = 0;
    sum_count = 0;
    normalized = false;
    init( _max_width, _src_type, _dst_type, _normalized,
          _ksize, _anchor, _border_mode, _border_value );
}


CvBoxFilter::~CvBoxFilter()
{
    clear();
}


void CvBoxFilter::init( int _max_width, int _src_type, int _dst_type,
                        bool _normalized, CvSize _ksize,
                        CvPoint _anchor, int _border_mode,
                        CvScalar _border_value )
{
    CV_FUNCNAME( "CvBoxFilter::init" );

    __BEGIN__;
    
    sum = 0;
    normalized = _normalized;

    if( normalized && CV_MAT_TYPE(_src_type) != CV_MAT_TYPE(_dst_type) ||
        !normalized && CV_MAT_CN(_src_type) != CV_MAT_CN(_dst_type))
        CV_ERROR( CV_StsUnmatchedFormats,
        "In case of normalized box filter input and output must have the same type.\n"
        "In case of unnormalized box filter the number of input and output channels must be the same" );

    min_depth = CV_MAT_DEPTH(_src_type) == CV_8U ? CV_32S : CV_64F;

    CvBaseImageFilter::init( _max_width, _src_type, _dst_type, 1, _ksize,
                             _anchor, _border_mode, _border_value );
    
    scale = normalized ? 1./(ksize.width*ksize.height) : 1;

    if( CV_MAT_DEPTH(src_type) == CV_8U )
        x_func = (CvRowFilterFunc)icvSumRow_8u32s;
    else if( CV_MAT_DEPTH(src_type) == CV_32F )
        x_func = (CvRowFilterFunc)icvSumRow_32f64f;
    else
        CV_ERROR( CV_StsUnsupportedFormat, "Unknown/unsupported input image format" );

    if( CV_MAT_DEPTH(dst_type) == CV_8U )
    {
        if( !normalized )
            CV_ERROR( CV_StsBadArg, "Only normalized box filter can be used for 8u->8u transformation" );
        y_func = (CvColumnFilterFunc)icvSumCol_32s8u;
    }
    else if( CV_MAT_DEPTH(dst_type) == CV_16S )
    {
        if( normalized || CV_MAT_DEPTH(src_type) != CV_8U )
            CV_ERROR( CV_StsBadArg, "Only 8u->16s unnormalized box filter is supported in case of 16s output" );
        y_func = (CvColumnFilterFunc)icvSumCol_32s16s;
    }
	else if( CV_MAT_DEPTH(dst_type) == CV_32S )
	{
		if( normalized || CV_MAT_DEPTH(src_type) != CV_8U )
			CV_ERROR( CV_StsBadArg, "Only 8u->32s unnormalized box filter is supported in case of 32s output");

		y_func = (CvColumnFilterFunc)icvSumCol_32s32s;
	}
    else if( CV_MAT_DEPTH(dst_type) == CV_32F )
    {
        if( CV_MAT_DEPTH(src_type) != CV_32F )
            CV_ERROR( CV_StsBadArg, "Only 32f->32f box filter (normalized or not) is supported in case of 32f output" );
        y_func = (CvColumnFilterFunc)icvSumCol_64f32f;
    }
	else{
		CV_ERROR( CV_StsBadArg, "Unknown/unsupported destination image format" );
	}

    __END__;
}


void CvBoxFilter::start_process( CvSlice x_range, int width )
{
    CvBaseImageFilter::start_process( x_range, width );
    int i, psz = CV_ELEM_SIZE(work_type);
    uchar* s;
    buf_end -= buf_step;
    buf_max_count--;
    assert( buf_max_count >= max_ky*2 + 1 );
    s = sum = buf_end + cvAlign((width + ksize.width - 1)*CV_ELEM_SIZE(src_type), ALIGN);
    sum_count = 0;

    width *= psz;
    for( i = 0; i < width; i++ )
        s[i] = (uchar)0;
}


static void
icvSumRow_8u32s( const uchar* src, int* dst, void* params )
{
    const CvBoxFilter* state = (const CvBoxFilter*)params;
    int ksize = state->get_kernel_size().width;
    int width = state->get_width();
    int cn = CV_MAT_CN(state->get_src_type());
    int i, k;

    width = (width - 1)*cn; ksize *= cn;

    for( k = 0; k < cn; k++, src++, dst++ )
    {
        int s = 0;
        for( i = 0; i < ksize; i += cn )
            s += src[i];
        dst[0] = s;
        for( i = 0; i < width; i += cn )
        {
            s += src[i+ksize] - src[i];
            dst[i+cn] = s;
        }
    }
}


static void
icvSumRow_32f64f( const float* src, double* dst, void* params )
{
    const CvBoxFilter* state = (const CvBoxFilter*)params;
    int ksize = state->get_kernel_size().width;
    int width = state->get_width();
    int cn = CV_MAT_CN(state->get_src_type());
    int i, k;

    width = (width - 1)*cn; ksize *= cn;

    for( k = 0; k < cn; k++, src++, dst++ )
    {
        double s = 0;
        for( i = 0; i < ksize; i += cn )
            s += src[i];
        dst[0] = s;
        for( i = 0; i < width; i += cn )
        {
            s += (double)src[i+ksize] - src[i];
            dst[i+cn] = s;
        }
    }
}


static void
icvSumCol_32s8u( const int** src, uchar* dst,
                 int dst_step, int count, void* params )
{
#define BLUR_SHIFT 24
    CvBoxFilter* state = (CvBoxFilter*)params;
    int ksize = state->get_kernel_size().height;
    int i, width = state->get_width();
    int cn = CV_MAT_CN(state->get_src_type());
    double scale = state->get_scale();
    int iscale = cvFloor(scale*(1 << BLUR_SHIFT));
    int* sum = (int*)state->get_sum_buf();
    int* _sum_count = state->get_sum_count_ptr();
    int sum_count = *_sum_count;

    width *= cn;
    src += sum_count;
    count += ksize - 1 - sum_count;

    for( ; count--; src++ )
    {
        const int* sp = src[0];
        if( sum_count+1 < ksize )
        {
            for( i = 0; i <= width - 2; i += 2 )
            {
                int s0 = sum[i] + sp[i], s1 = sum[i+1] + sp[i+1];
                sum[i] = s0; sum[i+1] = s1;
            }

            for( ; i < width; i++ )
                sum[i] += sp[i];

            sum_count++;
        }
        else
        {
            const int* sm = src[-ksize+1];
            for( i = 0; i <= width - 2; i += 2 )
            {
                int s0 = sum[i] + sp[i], s1 = sum[i+1] + sp[i+1];
                int t0 = CV_DESCALE(s0*iscale, BLUR_SHIFT), t1 = CV_DESCALE(s1*iscale, BLUR_SHIFT);
                s0 -= sm[i]; s1 -= sm[i+1];
                sum[i] = s0; sum[i+1] = s1;
                dst[i] = (uchar)t0; dst[i+1] = (uchar)t1;
            }

            for( ; i < width; i++ )
            {
                int s0 = sum[i] + sp[i], t0 = CV_DESCALE(s0*iscale, BLUR_SHIFT);
                sum[i] = s0 - sm[i]; dst[i] = (uchar)t0;
            }
            dst += dst_step;
        }
    }

    *_sum_count = sum_count;
#undef BLUR_SHIFT
}


static void
icvSumCol_32s16s( const int** src, short* dst,
                  int dst_step, int count, void* params )
{
    CvBoxFilter* state = (CvBoxFilter*)params;
    int ksize = state->get_kernel_size().height;
    int ktotal = ksize*state->get_kernel_size().width;
    int i, width = state->get_width();
    int cn = CV_MAT_CN(state->get_src_type());
    int* sum = (int*)state->get_sum_buf();
    int* _sum_count = state->get_sum_count_ptr();
    int sum_count = *_sum_count;

    dst_step /= sizeof(dst[0]);
    width *= cn;
    src += sum_count;
    count += ksize - 1 - sum_count;

    for( ; count--; src++ )
    {
        const int* sp = src[0];
        if( sum_count+1 < ksize )
        {
            for( i = 0; i <= width - 2; i += 2 )
            {
                int s0 = sum[i] + sp[i], s1 = sum[i+1] + sp[i+1];
                sum[i] = s0; sum[i+1] = s1;
            }

            for( ; i < width; i++ )
                sum[i] += sp[i];

            sum_count++;
        }
        else if( ktotal < 128 )
        {
            const int* sm = src[-ksize+1];
            for( i = 0; i <= width - 2; i += 2 )
            {
                int s0 = sum[i] + sp[i], s1 = sum[i+1] + sp[i+1];
                dst[i] = (short)s0; dst[i+1] = (short)s1;
                s0 -= sm[i]; s1 -= sm[i+1];
                sum[i] = s0; sum[i+1] = s1;
            }

            for( ; i < width; i++ )
            {
                int s0 = sum[i] + sp[i];
                dst[i] = (short)s0;
                sum[i] = s0 - sm[i];
            }
            dst += dst_step;
        }
        else
        {
            const int* sm = src[-ksize+1];
            for( i = 0; i <= width - 2; i += 2 )
            {
                int s0 = sum[i] + sp[i], s1 = sum[i+1] + sp[i+1];
                dst[i] = CV_CAST_16S(s0); dst[i+1] = CV_CAST_16S(s1);
                s0 -= sm[i]; s1 -= sm[i+1];
                sum[i] = s0; sum[i+1] = s1;
            }

            for( ; i < width; i++ )
            {
                int s0 = sum[i] + sp[i];
                dst[i] = CV_CAST_16S(s0);
                sum[i] = s0 - sm[i];
            }
            dst += dst_step;
        }
    }

    *_sum_count = sum_count;
}

static void
icvSumCol_32s32s( const int** src, int * dst,
                  int dst_step, int count, void* params )
{
    CvBoxFilter* state = (CvBoxFilter*)params;
    int ksize = state->get_kernel_size().height;
    int i, width = state->get_width();
    int cn = CV_MAT_CN(state->get_src_type());
    int* sum = (int*)state->get_sum_buf();
    int* _sum_count = state->get_sum_count_ptr();
    int sum_count = *_sum_count;

    dst_step /= sizeof(dst[0]);
    width *= cn;
    src += sum_count;
    count += ksize - 1 - sum_count;

    for( ; count--; src++ )
    {
        const int* sp = src[0];
        if( sum_count+1 < ksize )
        {
            for( i = 0; i <= width - 2; i += 2 )
            {
                int s0 = sum[i] + sp[i], s1 = sum[i+1] + sp[i+1];
                sum[i] = s0; sum[i+1] = s1;
            }

            for( ; i < width; i++ )
                sum[i] += sp[i];

            sum_count++;
        }
        else
        {
            const int* sm = src[-ksize+1];
            for( i = 0; i <= width - 2; i += 2 )
            {
                int s0 = sum[i] + sp[i], s1 = sum[i+1] + sp[i+1];
                dst[i] = s0; dst[i+1] = s1;
                s0 -= sm[i]; s1 -= sm[i+1];
                sum[i] = s0; sum[i+1] = s1;
            }

            for( ; i < width; i++ )
            {
                int s0 = sum[i] + sp[i];
                dst[i] = s0;
                sum[i] = s0 - sm[i];
            }
            dst += dst_step;
        }
    }

    *_sum_count = sum_count;
}


static void
icvSumCol_64f32f( const double** src, float* dst,
                  int dst_step, int count, void* params )
{
    CvBoxFilter* state = (CvBoxFilter*)params;
    int ksize = state->get_kernel_size().height;
    int i, width = state->get_width();
    int cn = CV_MAT_CN(state->get_src_type());
    double scale = state->get_scale();
    bool normalized = state->is_normalized();
    double* sum = (double*)state->get_sum_buf();
    int* _sum_count = state->get_sum_count_ptr();
    int sum_count = *_sum_count;

    dst_step /= sizeof(dst[0]);
    width *= cn;
    src += sum_count;
    count += ksize - 1 - sum_count;

    for( ; count--; src++ )
    {
        const double* sp = src[0];
        if( sum_count+1 < ksize )
        {
            for( i = 0; i <= width - 2; i += 2 )
            {
                double s0 = sum[i] + sp[i], s1 = sum[i+1] + sp[i+1];
                sum[i] = s0; sum[i+1] = s1;
            }

            for( ; i < width; i++ )
                sum[i] += sp[i];

            sum_count++;
        }
        else
        {
            const double* sm = src[-ksize+1];
            if( normalized )
                for( i = 0; i <= width - 2; i += 2 )
                {
                    double s0 = sum[i] + sp[i], s1 = sum[i+1] + sp[i+1];
                    double t0 = s0*scale, t1 = s1*scale;
                    s0 -= sm[i]; s1 -= sm[i+1];
                    dst[i] = (float)t0; dst[i+1] = (float)t1;
                    sum[i] = s0; sum[i+1] = s1;
                }
            else
                for( i = 0; i <= width - 2; i += 2 )
                {
                    double s0 = sum[i] + sp[i], s1 = sum[i+1] + sp[i+1];
                    dst[i] = (float)s0; dst[i+1] = (float)s1;
                    s0 -= sm[i]; s1 -= sm[i+1];
                    sum[i] = s0; sum[i+1] = s1;
                }

            for( ; i < width; i++ )
            {
                double s0 = sum[i] + sp[i], t0 = s0*scale;
                sum[i] = s0 - sm[i]; dst[i] = (float)t0;
            }
            dst += dst_step;
        }
    }

    *_sum_count = sum_count;
}


/****************************************************************************************\
                                      Median Filter
\****************************************************************************************/

#define CV_MINMAX_8U(a,b) \
    (t = CV_FAST_CAST_8U((a) - (b)), (b) += t, a -= t)

static CvStatus CV_STDCALL
icvMedianBlur_8u_CnR( uchar* src, int src_step, uchar* dst, int dst_step,
                      CvSize size, int m, int cn )
{
    #define N  16
    int     zone0[4][N];
    int     zone1[4][N*N];
    int     x, y;
    int     n2 = m*m/2;
    int     nx = (m + 1)/2 - 1;
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

                CV_MINMAX_8U(p1, p2); CV_MINMAX_8U(p4, p5);
                CV_MINMAX_8U(p7, p8); CV_MINMAX_8U(p0, p1);
                CV_MINMAX_8U(p3, p4); CV_MINMAX_8U(p6, p7);
                CV_MINMAX_8U(p1, p2); CV_MINMAX_8U(p4, p5);
                CV_MINMAX_8U(p7, p8); CV_MINMAX_8U(p0, p3);
                CV_MINMAX_8U(p5, p8); CV_MINMAX_8U(p4, p7);
                CV_MINMAX_8U(p3, p6); CV_MINMAX_8U(p1, p4);
                CV_MINMAX_8U(p2, p5); CV_MINMAX_8U(p4, p7);
                CV_MINMAX_8U(p4, p2); CV_MINMAX_8U(p6, p4);
                CV_MINMAX_8U(p4, p2);
                dst[x1] = (uchar)p4;
            }

            for( x = cn; x < size.width - cn; x++ )
            {
                int p0 = src0[x-cn], p1 = src0[x], p2 = src0[x+cn];
                int p3 = src1[x-cn], p4 = src1[x], p5 = src1[x+cn];
                int p6 = src2[x-cn], p7 = src2[x], p8 = src2[x+cn];
                int t;

                CV_MINMAX_8U(p1, p2); CV_MINMAX_8U(p4, p5);
                CV_MINMAX_8U(p7, p8); CV_MINMAX_8U(p0, p1);
                CV_MINMAX_8U(p3, p4); CV_MINMAX_8U(p6, p7);
                CV_MINMAX_8U(p1, p2); CV_MINMAX_8U(p4, p5);
                CV_MINMAX_8U(p7, p8); CV_MINMAX_8U(p0, p3);
                CV_MINMAX_8U(p5, p8); CV_MINMAX_8U(p4, p7);
                CV_MINMAX_8U(p3, p6); CV_MINMAX_8U(p1, p4);
                CV_MINMAX_8U(p2, p5); CV_MINMAX_8U(p4, p7);
                CV_MINMAX_8U(p4, p2); CV_MINMAX_8U(p6, p4);
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
                              CvSize size, double sigma_color,
                              double sigma_space, int channels )
{
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
cvSmooth( const void* srcarr, void* dstarr, int smooth_type,
          int param1, int param2, double param3, double param4 )
{
    CvBoxFilter box_filter;
    CvSepFilter gaussian_filter;

    CvMat* temp = 0;

    CV_FUNCNAME( "cvSmooth" );

    __BEGIN__;

    int coi1 = 0, coi2 = 0;
    CvMat srcstub, *src = (CvMat*)srcarr;
    CvMat dststub, *dst = (CvMat*)dstarr;
    CvSize size;
    int src_type, dst_type, depth, cn;
    double sigma1 = 0, sigma2 = 0;
    bool have_ipp = icvFilterMedian_8u_C1R_p != 0;

    CV_CALL( src = cvGetMat( src, &srcstub, &coi1 ));
    CV_CALL( dst = cvGetMat( dst, &dststub, &coi2 ));

    if( coi1 != 0 || coi2 != 0 )
        CV_ERROR( CV_BadCOI, "" );

    src_type = CV_MAT_TYPE( src->type );
    dst_type = CV_MAT_TYPE( dst->type );
    depth = CV_MAT_DEPTH(src_type);
    cn = CV_MAT_CN(src_type);
    size = cvGetMatSize(src);

    if( !CV_ARE_SIZES_EQ( src, dst ))
        CV_ERROR( CV_StsUnmatchedSizes, "" );

    if( smooth_type != CV_BLUR_NO_SCALE && !CV_ARE_TYPES_EQ( src, dst ))
        CV_ERROR( CV_StsUnmatchedFormats,
        "The specified smoothing algorithm requires input and ouput arrays be of the same type" );

    if( smooth_type == CV_BLUR || smooth_type == CV_BLUR_NO_SCALE ||
        smooth_type == CV_GAUSSIAN || smooth_type == CV_MEDIAN )
    {
        // automatic detection of kernel size from sigma
        if( smooth_type == CV_GAUSSIAN )
        {
            sigma1 = param3;
            sigma2 = param4 ? param4 : param3;

            if( param1 == 0 && sigma1 > 0 )
                param1 = cvRound(sigma1*(depth == CV_8U ? 3 : 4)*2 + 1)|1;
            if( param2 == 0 && sigma2 > 0 )
                param2 = cvRound(sigma2*(depth == CV_8U ? 3 : 4)*2 + 1)|1;
        }

        if( param2 == 0 )
            param2 = size.height == 1 ? 1 : param1;
        if( param1 < 1 || (param1 & 1) == 0 || param2 < 1 || (param2 & 1) == 0 )
            CV_ERROR( CV_StsOutOfRange,
                "Both mask width and height must be >=1 and odd" );

        if( param1 == 1 && param2 == 1 )
        {
            cvConvert( src, dst );
            EXIT;
        }
    }

    if( have_ipp && (smooth_type == CV_BLUR || smooth_type == CV_MEDIAN) &&
        size.width >= param1 && size.height >= param2 && param1 > 1 && param2 > 1 )
    {
        CvSmoothFixedIPPFunc ipp_median_box_func = 0;

        if( smooth_type == CV_BLUR )
        {
            ipp_median_box_func =
                src_type == CV_8UC1 ? icvFilterBox_8u_C1R_p :
                src_type == CV_8UC3 ? icvFilterBox_8u_C3R_p :
                src_type == CV_8UC4 ? icvFilterBox_8u_C4R_p :
                src_type == CV_32FC1 ? icvFilterBox_32f_C1R_p :
                src_type == CV_32FC3 ? icvFilterBox_32f_C3R_p :
                src_type == CV_32FC4 ? icvFilterBox_32f_C4R_p : 0;
        }
        else if( smooth_type == CV_MEDIAN )
        {
            ipp_median_box_func =
                src_type == CV_8UC1 ? icvFilterMedian_8u_C1R_p :
                src_type == CV_8UC3 ? icvFilterMedian_8u_C3R_p :
                src_type == CV_8UC4 ? icvFilterMedian_8u_C4R_p : 0;
        }

        if( ipp_median_box_func )
        {
            CvSize el_size = { param1, param2 };
            CvPoint el_anchor = { param1/2, param2/2 };
            int stripe_size = 1 << 14; // the optimal value may depend on CPU cache,
                                       // overhead of the current IPP code etc.
            const uchar* shifted_ptr;
            int y, dy = 0;
            int temp_step, dst_step = dst->step;

            CV_CALL( temp = icvIPPFilterInit( src, stripe_size, el_size ));

            shifted_ptr = temp->data.ptr +
                el_anchor.y*temp->step + el_anchor.x*CV_ELEM_SIZE(src_type);
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

    if( smooth_type == CV_BLUR || smooth_type == CV_BLUR_NO_SCALE )
    {
        CV_CALL( box_filter.init( src->cols, src_type, dst_type,
            smooth_type == CV_BLUR, cvSize(param1, param2) ));
        CV_CALL( box_filter.process( src, dst ));
    }
    else if( smooth_type == CV_MEDIAN )
    {
        if( depth != CV_8U || cn != 1 && cn != 3 && cn != 4 )
            CV_ERROR( CV_StsUnsupportedFormat,
            "Median filter only supports 8uC1, 8uC3 and 8uC4 images" );

        IPPI_CALL( icvMedianBlur_8u_CnR( src->data.ptr, src->step,
            dst->data.ptr, dst->step, size, param1, cn ));
    }
    else if( smooth_type == CV_GAUSSIAN )
    {
        CvSize ksize = { param1, param2 };
        float* kx = (float*)cvStackAlloc( ksize.width*sizeof(kx[0]) );
        float* ky = (float*)cvStackAlloc( ksize.height*sizeof(ky[0]) );
        CvMat KX = cvMat( 1, ksize.width, CV_32F, kx );
        CvMat KY = cvMat( 1, ksize.height, CV_32F, ky );
        
        CvSepFilter::init_gaussian_kernel( &KX, sigma1 );
        if( ksize.width != ksize.height || fabs(sigma1 - sigma2) > FLT_EPSILON )
            CvSepFilter::init_gaussian_kernel( &KY, sigma2 );
        else
            KY.data.fl = kx;
        
        if( have_ipp && size.width >= param1*3 &&
            size.height >= param2 && param1 > 1 && param2 > 1 )
        {
            int done;
            CV_CALL( done = icvIPPSepFilter( src, dst, &KX, &KY,
                        cvPoint(ksize.width/2,ksize.height/2)));
            if( done )
                EXIT;
        }

        CV_CALL( gaussian_filter.init( src->cols, src_type, dst_type, &KX, &KY ));
        CV_CALL( gaussian_filter.process( src, dst ));
    }
    else if( smooth_type == CV_BILATERAL )
    {
        if( param1 < 0 || param2 < 0 )
            CV_ERROR( CV_StsOutOfRange,
            "Thresholds in bilaral filtering should not bee negative" );
        param1 += param1 == 0;
        param2 += param2 == 0;

        if( depth != CV_8U || cn != 1 && cn != 3 )
            CV_ERROR( CV_StsUnsupportedFormat,
            "Bilateral filter only supports 8uC1 and 8uC3 images" );

        IPPI_CALL( icvBilateralFiltering_8u_CnR( src->data.ptr, src->step,
            dst->data.ptr, dst->step, size, param1, param2, cn ));
    }

    __END__;

    cvReleaseMat( &temp );
}

/* End of file. */
