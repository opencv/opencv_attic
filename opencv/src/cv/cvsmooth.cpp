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

/*
 * This file includes the code, contributed by Simon Perreault
 * (the function icvMedianBlur_8u_CnR_O1)
 *
 * Constant-time median filtering -- http://nomis80.org/ctmf.html
 * Copyright (C) 2006 Simon Perreault
 *
 * Contact:
 *  Laboratoire de vision et systemes numeriques
 *  Pavillon Adrien-Pouliot
 *  Universite Laval
 *  Sainte-Foy, Quebec, Canada
 *  G1K 7P4
 *
 *  perreaul@gel.ulaval.ca
 */

namespace cv
{

/****************************************************************************************\
                                         Box Filter
\****************************************************************************************/

template<typename T, typename ST> struct RowSum : public BaseRowFilter
{
    RowSum( int _ksize, int _anchor )
    {
        ksize = _ksize;
        anchor = _anchor;
    }
    
    void operator()(const uchar* src, uchar* dst, int width, int cn)
    {
        const T* S = (const T*)src;
        ST* D = (ST*)dst;
        int i = 0, k, ksz_cn = ksize*cn;
        
        width = (width - 1)*cn;
        for( k = 0; k < cn; k++, S++, D++ )
        {
            ST s = 0;
            for( i = 0; i < ksz_cn; i += cn )
                s += S[i];
            D[0] = s;
            for( i = 0; i < width; i += cn )
            {
                s += S[i + ksz_cn] - S[i];
                D[i+cn] = s;
            }
        }
    }
};


template<typename ST, typename T> struct ColumnSum : public BaseColumnFilter
{
    ColumnSum( int _ksize, int _anchor, double _scale )
    {
        ksize = _ksize;
        anchor = _anchor;
        scale = _scale;
        sumCount = 0;
    }

    void reset() { sumCount = 0; }
    
    void operator()(const uchar** src, uchar* dst, int dststep, int count, int width)
    {
        int i;
        ST* SUM;
        bool haveScale = scale != 1;
        double _scale = scale;

        if( width != (int)sum.size() )
        {
            sum.resize(width);
            sumCount = 0;
        }

        SUM = &sum[0];
        if( sumCount == 0 )
        {
            for( i = 0; i < width; i++ )
                SUM[i] = 0;
            for( ; sumCount < ksize - 1; sumCount++, src++ )
            {
                const ST* Sp = (const ST*)src[0];
                for( i = 0; i <= width - 2; i += 2 )
                {
                    ST s0 = SUM[i] + Sp[i], s1 = SUM[i+1] + Sp[i+1];
                    SUM[i] = s0; SUM[i+1] = s1;
                }

                for( ; i < width; i++ )
                    SUM[i] += Sp[i];
            }
        }
        else
        {
            CV_Assert( sumCount == ksize-1 );
            src += ksize-1;
        }

        for( ; count--; src++ )
        {
            const ST* Sp = (const ST*)src[0];
            const ST* Sm = (const ST*)src[1-ksize];
            T* D = (T*)dst;
            if( haveScale )
            {
                for( i = 0; i <= width - 2; i += 2 )
                {
                    ST s0 = SUM[i] + Sp[i], s1 = SUM[i+1] + Sp[i+1];
                    D[i] = saturate_cast<T>(s0*_scale);
                    D[i+1] = saturate_cast<T>(s1*_scale);
                    s0 -= Sm[i]; s1 -= Sm[i+1];
                    SUM[i] = s0; SUM[i+1] = s1;
                }

                for( ; i < width; i++ )
                {
                    ST s0 = SUM[i] + Sp[i];
                    D[i] = saturate_cast<T>(s0*_scale);
                    SUM[i] = s0 - Sm[i];
                }
            }
            else
            {
                for( i = 0; i <= width - 2; i += 2 )
                {
                    ST s0 = SUM[i] + Sp[i], s1 = SUM[i+1] + Sp[i+1];
                    D[i] = saturate_cast<T>(s0);
                    D[i+1] = saturate_cast<T>(s1);
                    s0 -= Sm[i]; s1 -= Sm[i+1];
                    SUM[i] = s0; SUM[i+1] = s1;
                }

                for( ; i < width; i++ )
                {
                    ST s0 = SUM[i] + Sp[i];
                    D[i] = saturate_cast<T>(s0);
                    SUM[i] = s0 - Sm[i];
                }
            }
            dst += dststep;
        }
    }

    double scale;
    int sumCount;
    Vector<ST> sum;
};


Ptr<BaseRowFilter> getRowSumFilter(int srcType, int sumType, int ksize, int anchor)
{
    int sdepth = CV_MAT_DEPTH(srcType), ddepth = CV_MAT_DEPTH(sumType);
    CV_Assert( CV_MAT_CN(sumType) == CV_MAT_CN(srcType) );

    if( anchor < 0 )
        anchor = ksize/2;

    if( sdepth == CV_8U && ddepth == CV_32S )
        return Ptr<BaseRowFilter>(new RowSum<uchar, int>(ksize, anchor));
    if( sdepth == CV_8U && ddepth == CV_64F )
        return Ptr<BaseRowFilter>(new RowSum<uchar, double>(ksize, anchor));
    if( sdepth == CV_16U && ddepth == CV_32S )
        return Ptr<BaseRowFilter>(new RowSum<ushort, int>(ksize, anchor));
    if( sdepth == CV_16U && ddepth == CV_64F )
        return Ptr<BaseRowFilter>(new RowSum<ushort, double>(ksize, anchor));
    if( sdepth == CV_16S && ddepth == CV_32S )
        return Ptr<BaseRowFilter>(new RowSum<short, int>(ksize, anchor));
    if( sdepth == CV_32S && ddepth == CV_32S )
        return Ptr<BaseRowFilter>(new RowSum<int, int>(ksize, anchor));
    if( sdepth == CV_16S && ddepth == CV_64F )
        return Ptr<BaseRowFilter>(new RowSum<short, double>(ksize, anchor));
    if( sdepth == CV_32F && ddepth == CV_64F )
        return Ptr<BaseRowFilter>(new RowSum<float, double>(ksize, anchor));
    if( sdepth == CV_64F && ddepth == CV_64F )
        return Ptr<BaseRowFilter>(new RowSum<double, double>(ksize, anchor));

    CV_Error_( CV_StsNotImplemented,
        ("Unsupported combination of source format (=%d), and buffer format (=%d)",
        srcType, sumType));

    return Ptr<BaseRowFilter>(0);
}


Ptr<BaseColumnFilter> getColumnSumFilter(int sumType, int dstType, int ksize,
                                         int anchor, double scale)
{
    int sdepth = CV_MAT_DEPTH(sumType), ddepth = CV_MAT_DEPTH(dstType);
    CV_Assert( CV_MAT_CN(sumType) == CV_MAT_CN(dstType) );

    if( anchor < 0 )
        anchor = ksize/2;

    if( ddepth == CV_8U && sdepth == CV_32S )
        return Ptr<BaseColumnFilter>(new ColumnSum<int, uchar>(ksize, anchor, scale));
    if( ddepth == CV_8U && sdepth == CV_64F )
        return Ptr<BaseColumnFilter>(new ColumnSum<double, uchar>(ksize, anchor, scale));
    if( ddepth == CV_16U && sdepth == CV_32S )
        return Ptr<BaseColumnFilter>(new ColumnSum<int, ushort>(ksize, anchor, scale));
    if( ddepth == CV_16U && sdepth == CV_64F )
        return Ptr<BaseColumnFilter>(new ColumnSum<double, ushort>(ksize, anchor, scale));
    if( ddepth == CV_16S && sdepth == CV_32S )
        return Ptr<BaseColumnFilter>(new ColumnSum<int, short>(ksize, anchor, scale));
    if( ddepth == CV_16S && sdepth == CV_64F )
        return Ptr<BaseColumnFilter>(new ColumnSum<double, short>(ksize, anchor, scale));
    if( ddepth == CV_32S && sdepth == CV_32S )
        return Ptr<BaseColumnFilter>(new ColumnSum<int, int>(ksize, anchor, scale));
    if( ddepth == CV_32F && sdepth == CV_32S )
        return Ptr<BaseColumnFilter>(new ColumnSum<int, float>(ksize, anchor, scale));
    if( ddepth == CV_32F && sdepth == CV_64F )
        return Ptr<BaseColumnFilter>(new ColumnSum<double, float>(ksize, anchor, scale));
    if( ddepth == CV_64F && sdepth == CV_32S )
        return Ptr<BaseColumnFilter>(new ColumnSum<int, double>(ksize, anchor, scale));
    if( ddepth == CV_64F && sdepth == CV_64F )
        return Ptr<BaseColumnFilter>(new ColumnSum<double, double>(ksize, anchor, scale));

    CV_Error_( CV_StsNotImplemented,
        ("Unsupported combination of sum format (=%d), and destination format (=%d)",
        sumType, dstType));

    return Ptr<BaseColumnFilter>(0);
}


Ptr<FilterEngine> createBoxFilter( int srcType, int dstType, Size ksize,
                    Point anchor, bool normalize, int borderType )
{
    int sdepth = CV_MAT_DEPTH(srcType);
    int cn = CV_MAT_CN(srcType), sumType = CV_64F;
    if( sdepth < CV_32S && (!normalize ||
        ksize.width*ksize.height <= (sdepth == CV_8U ? (1<<23) :
            sdepth == CV_16U ? (1 << 15) : (1 << 16))) )
        sumType = CV_32S;
    sumType = CV_MAKETYPE( sumType, cn );

    Ptr<BaseRowFilter> rowFilter = getRowSumFilter(srcType, sumType, ksize.width, anchor.x );
    Ptr<BaseColumnFilter> columnFilter = getColumnSumFilter(sumType,
        dstType, ksize.height, anchor.y, normalize ? 1./(ksize.width*ksize.height) : 1);

    return Ptr<FilterEngine>(new FilterEngine(Ptr<BaseFilter>(0), rowFilter, columnFilter,
           srcType, dstType, sumType, borderType ));
}


void boxFilter( const Mat& src, Mat& dst, int ddepth,
                Size ksize, Point anchor,
                bool normalize, int borderType )
{
    int sdepth = src.depth(), cn = src.channels();
    if( ddepth < 0 )
        ddepth = sdepth;
    dst.create( src.size(), CV_MAKETYPE(ddepth, cn) );
    Ptr<FilterEngine> f = createBoxFilter( src.type(), dst.type(),
                        ksize, anchor, normalize, borderType );
    f->apply( src, dst );
}

/****************************************************************************************\
                                     Gaussian Blur
\****************************************************************************************/

Mat getGaussianKernel( int n, double sigma, int ktype )
{
    const int SMALL_GAUSSIAN_SIZE = 7;
    static const float small_gaussian_tab[][SMALL_GAUSSIAN_SIZE/2+1] =
    {
        {1.f},
        {0.5f, 0.25f},
        {0.375f, 0.25f, 0.0625f},
        {0.28125f, 0.21875f, 0.109375f, 0.03125f}
    };

    const float* fixed_kernel = n <= SMALL_GAUSSIAN_SIZE && sigma <= 0 ?
        small_gaussian_tab[n>>1] : 0;

    CV_Assert( ktype == CV_32F || ktype == CV_64F );
    Mat kernel(n, 1, ktype);
    float* cf = (float*)kernel.data;
    double* cd = (double*)kernel.data;

    double sigmaX = sigma > 0 ? sigma : (n/2 - 1)*0.3 + 0.8;
    double scale2X = -0.5/(sigmaX*sigmaX);

    double sum = fixed_kernel ? -fixed_kernel[0] : -1.;

    int i;
    for( i = 0; i <= n/2; i++ )
    {
        double t = fixed_kernel ? (double)fixed_kernel[i] : std::exp(scale2X*i*i);
        if( ktype == CV_32F )
        {
            cf[n/2+i] = (float)t;
            sum += cf[n/2+i]*2;
        }
        else
        {
            cd[n/2+i] = t;
            sum += cd[n/2+i]*2;
        }
    }

    sum = 1./sum;
    for( i = 0; i <= n/2; i++ )
    {
        if( ktype == CV_32F )
            cf[n/2+i] = cf[n/2-i] = (float)(cf[n/2+i]*sum);
        else
            cd[n/2+i] = cd[n/2-i] = cd[n/2+i]*sum;
    }

    return kernel;
}


Ptr<FilterEngine> createGaussianFilter( int type, Size ksize,
                                        double sigma1, double sigma2,
                                        int borderType )
{
    int depth = CV_MAT_DEPTH(type);
    if( sigma2 <= 0 )
        sigma2 = sigma1;

    // automatic detection of kernel size from sigma
    if( ksize.width <= 0 && sigma1 > 0 )
        ksize.width = cvRound(sigma1*(depth == CV_8U ? 3 : 4)*2 + 1)|1;
    if( ksize.height <= 0 && sigma2 > 0 )
        ksize.height = cvRound(sigma2*(depth == CV_8U ? 3 : 4)*2 + 1)|1;

    CV_Assert( ksize.width > 0 && ksize.width % 2 == 1 &&
        ksize.height > 0 && ksize.height % 2 == 1 );

    sigma1 = std::max( sigma1, 0. );
    sigma2 = std::max( sigma2, 0. );

    Mat kx = getGaussianKernel( ksize.width, sigma1, std::max(depth, CV_32F) );
    Mat ky;
    if( ksize.height == ksize.width && std::abs(sigma1 - sigma2) < DBL_EPSILON )
        ky = kx;
    else
        ky = getGaussianKernel( ksize.height, sigma2, std::max(depth, CV_32F) );

    return createSeparableLinearFilter( type, type, kx, ky, Point(-1,-1), 0, borderType );
}


void GaussianBlur( const Mat& src, Mat& dst, Size ksize,
                   double sigma1, double sigma2,
                   int borderType )
{
    if( ksize.width == 1 && ksize.height == 1 )
    {
        src.copyTo(dst);
        return;
    }

    dst.create( src.size(), src.type() );
    Ptr<FilterEngine> f = createGaussianFilter( src.type(), ksize, sigma1, sigma2, borderType );
    f->apply( src, dst );
}


/****************************************************************************************\
                                      Median Filter
\****************************************************************************************/

#define CV_MINMAX_8U(a,b) \
    (t = CV_FAST_CAST_8U((a) - (b)), (b) += t, a -= t)

#if CV_SSE2 && !defined __SSE2__
#define __SSE2__ 1
#include "emmintrin.h"
#endif

#if defined(__VEC__) || defined(__ALTIVEC__)
#include <altivec.h>
#undef bool
#endif

#if defined(__GNUC__)
#define align(x) __attribute__ ((aligned (x)))
#elif CV_SSE2 && (defined(__ICL) || (_MSC_VER >= 1300))
#define align(x) __declspec(align(x))
#else
#define align(x)
#endif

#if _MSC_VER >= 1200
#pragma warning( disable: 4244 )
#endif

/**
 * This structure represents a two-tier histogram. The first tier (known as the
 * "coarse" level) is 4 bit wide and the second tier (known as the "fine" level)
 * is 8 bit wide. Pixels inserted in the fine level also get inserted into the
 * coarse bucket designated by the 4 MSBs of the fine bucket value.
 *
 * The structure is aligned on 16 bits, which is a prerequisite for SIMD
 * instructions. Each bucket is 16 bit wide, which means that extra care must be
 * taken to prevent overflow.
 */
typedef struct align(16)
{
    ushort coarse[16];
    ushort fine[16][16];
} Histogram;

/**
 * HOP is short for Histogram OPeration. This macro makes an operation \a op on
 * histogram \a h for pixel value \a x. It takes care of handling both levels.
 */
#define HOP(h,x,op) \
    h.coarse[x>>4] op; \
    *((ushort*) h.fine + x) op;

#define COP(c,j,x,op) \
    h_coarse[ 16*(n*c+j) + (x>>4) ] op; \
    h_fine[ 16 * (n*(16*c+(x>>4)) + j) + (x & 0xF) ] op;

#if CV_SSE2 || defined __MMX__ || defined __ALTIVEC__
#define MEDIAN_HAVE_SIMD 1
#else
#define MEDIAN_HAVE_SIMD 0
#endif

/**
 * Adds histograms \a x and \a y and stores the result in \a y. Makes use of
 * SSE2, MMX or Altivec, if available.
 */
#if CV_SSE2
static inline void histogram_add( const ushort x[16], ushort y[16] )
{
    _mm_store_si128( (__m128i*) &y[0], _mm_add_epi16(
        _mm_load_si128((__m128i*) &y[0]), _mm_load_si128((__m128i*) &x[0] )));
    _mm_store_si128( (__m128i*) &y[8], _mm_add_epi16(
        _mm_load_si128((__m128i*) &y[8]), _mm_load_si128((__m128i*) &x[8] )));
}
#elif defined(__MMX__)
static inline void histogram_add( const ushort x[16], ushort y[16] )
{
    *(__m64*) &y[0]  = _mm_add_pi16( *(__m64*) &y[0],  *(__m64*) &x[0]  );
    *(__m64*) &y[4]  = _mm_add_pi16( *(__m64*) &y[4],  *(__m64*) &x[4]  );
    *(__m64*) &y[8]  = _mm_add_pi16( *(__m64*) &y[8],  *(__m64*) &x[8]  );
    *(__m64*) &y[12] = _mm_add_pi16( *(__m64*) &y[12], *(__m64*) &x[12] );
}
#elif defined(__ALTIVEC__)
static inline void histogram_add( const ushort x[16], ushort y[16] )
{
    *(vector ushort*) &y[0] = vec_add( *(vector ushort*) &y[0], *(vector ushort*) &x[0] );
    *(vector ushort*) &y[8] = vec_add( *(vector ushort*) &y[8], *(vector ushort*) &x[8] );
}
#else
static inline void histogram_add( const ushort x[16], ushort y[16] )
{
    int i;
    for( i = 0; i < 16; ++i )
        y[i] = (ushort)(y[i] + x[i]);
}
#endif

/**
 * Subtracts histogram \a x from \a y and stores the result in \a y. Makes use
 * of SSE2, MMX or Altivec, if available.
 */
#if CV_SSE2
static inline void histogram_sub( const ushort x[16], ushort y[16] )
{
    _mm_store_si128( (__m128i*) &y[0], _mm_sub_epi16(
        _mm_load_si128((__m128i*) &y[0]), _mm_load_si128((__m128i*) &x[0] )));
    _mm_store_si128( (__m128i*) &y[8], _mm_sub_epi16(
        _mm_load_si128((__m128i*) &y[8]), _mm_load_si128((__m128i*) &x[8] )));
}
#elif defined(__MMX__)
static inline void histogram_sub( const ushort x[16], ushort y[16] )
{
    *(__m64*) &y[0]  = _mm_sub_pi16( *(__m64*) &y[0],  *(__m64*) &x[0]  );
    *(__m64*) &y[4]  = _mm_sub_pi16( *(__m64*) &y[4],  *(__m64*) &x[4]  );
    *(__m64*) &y[8]  = _mm_sub_pi16( *(__m64*) &y[8],  *(__m64*) &x[8]  );
    *(__m64*) &y[12] = _mm_sub_pi16( *(__m64*) &y[12], *(__m64*) &x[12] );
}
#elif defined(__ALTIVEC__)
static inline void histogram_sub( const ushort x[16], ushort y[16] )
{
    *(vector ushort*) &y[0] = vec_sub( *(vector ushort*) &y[0], *(vector ushort*) &x[0] );
    *(vector ushort*) &y[8] = vec_sub( *(vector ushort*) &y[8], *(vector ushort*) &x[8] );
}
#else
static inline void histogram_sub( const ushort x[16], ushort y[16] )
{
    int i;
    for( i = 0; i < 16; ++i )
        y[i] = (ushort)(y[i] - x[i]);
}
#endif

static inline void histogram_muladd( int a, const ushort x[16],
        ushort y[16] )
{
    int i;
    for ( i = 0; i < 16; ++i )
        y[i] = (ushort)(y[i] + a * x[i]);
}

static void
medianBlur_8u_CnR_O1( uchar* src, int src_step, uchar* dst, int dst_step,
                      Size size, int kernel_size, int cn,
                      int pad_left, int pad_right )
{
    int r = (kernel_size-1)/2;
    const int m = size.height, n = size.width;
    int i, j, k, c;
    const uchar *p, *q;
    Histogram H[4];
    ushort luc[4][16];

    CV_Assert( size.height >= r && size.width >= r );

    assert( src );
    assert( dst );
    assert( r >= 0 );
    assert( size.width >= 2*r+1 );
    assert( size.height >= 2*r+1 );
    assert( src_step != 0 );
    assert( dst_step != 0 );

    Vector<ushort> _h_coarse(1 * 16 * n * cn, (ushort)0);
    Vector<ushort> _h_fine(16 * 16 * n * cn, (ushort)0); 
    ushort* h_coarse = &_h_coarse[0];
    ushort* h_fine = &_h_fine[0];

    /* First row initialization */
    for ( j = 0; j < n; ++j ) {
        for ( c = 0; c < cn; ++c ) {
            COP( c, j, src[cn*j+c], += r+1 );
        }
    }
    for ( i = 0; i < r; ++i ) {
        for ( j = 0; j < n; ++j ) {
            for ( c = 0; c < cn; ++c ) {
                COP( c, j, src[src_step*i+cn*j+c], ++ );
            }
        }
    }

    for ( i = 0; i < m; ++i ) {

        /* Update column histograms for entire row. */
        p = src + src_step * MAX( 0, i-r-1 );
        q = p + cn * n;
        for ( j = 0; p != q; ++j ) {
            for ( c = 0; c < cn; ++c, ++p ) {
                COP( c, j, *p, -- );
            }
        }

        p = src + src_step * MIN( m-1, i+r );
        q = p + cn * n;
        for ( j = 0; p != q; ++j ) {
            for ( c = 0; c < cn; ++c, ++p ) {
                COP( c, j, *p, ++ );
            }
        }

        /* First column initialization */
        memset( H, 0, cn*sizeof(H[0]) );
        memset( luc, 0, cn*sizeof(luc[0]) );
        if ( pad_left ) {
            for ( c = 0; c < cn; ++c ) {
                histogram_muladd( r, &h_coarse[16*n*c], H[c].coarse );
            }
        }
        for ( j = 0; j < (pad_left ? r : 2*r); ++j ) {
            for ( c = 0; c < cn; ++c ) {
                histogram_add( &h_coarse[16*(n*c+j)], H[c].coarse );
            }
        }
        for ( c = 0; c < cn; ++c ) {
            for ( k = 0; k < 16; ++k ) {
                histogram_muladd( 2*r+1, &h_fine[16*n*(16*c+k)], &H[c].fine[k][0] );
            }
        }

        for ( j = pad_left ? 0 : r; j < (pad_right ? n : n-r); ++j ) {
            for ( c = 0; c < cn; ++c ) {
                int t = 2*r*r + 2*r, b, sum = 0;
                ushort* segment;

                histogram_add( &h_coarse[16*(n*c + MIN(j+r,n-1))], H[c].coarse );

                /* Find median at coarse level */
                for ( k = 0; k < 16 ; ++k ) {
                    sum += H[c].coarse[k];
                    if ( sum > t ) {
                        sum -= H[c].coarse[k];
                        break;
                    }
                }
                assert( k < 16 );

                /* Update corresponding histogram segment */
                if ( luc[c][k] <= j-r ) {
                    memset( &H[c].fine[k], 0, 16 * sizeof(ushort) );
                    for ( luc[c][k] = j-r; luc[c][k] < MIN(j+r+1,n); ++luc[c][k] ) {
                        histogram_add( &h_fine[16*(n*(16*c+k)+luc[c][k])], H[c].fine[k] );
                    }
                    if ( luc[c][k] < j+r+1 ) {
                        histogram_muladd( j+r+1 - n, &h_fine[16*(n*(16*c+k)+(n-1))], &H[c].fine[k][0] );
                        luc[c][k] = (ushort)(j+r+1);
                    }
                }
                else {
                    for ( ; luc[c][k] < j+r+1; ++luc[c][k] ) {
                        histogram_sub( &h_fine[16*(n*(16*c+k)+MAX(luc[c][k]-2*r-1,0))], H[c].fine[k] );
                        histogram_add( &h_fine[16*(n*(16*c+k)+MIN(luc[c][k],n-1))], H[c].fine[k] );
                    }
                }

                histogram_sub( &h_coarse[16*(n*c+MAX(j-r,0))], H[c].coarse );

                /* Find median in segment */
                segment = H[c].fine[k];
                for ( b = 0; b < 16 ; ++b ) {
                    sum += segment[b];
                    if ( sum > t ) {
                        dst[dst_step*i+cn*j+c] = (uchar)(16*k + b);
                        break;
                    }
                }
                assert( b < 16 );
            }
        }
    }

#if defined(__MMX__)
    _mm_empty();
#endif

#undef HOP
#undef COP
}


#if _MSC_VER >= 1200
#pragma warning( default: 4244 )
#endif


static void
medianBlur_8u_CnR_Om( uchar* src, int src_step, uchar* dst, int dst_step,
                      Size size, int m, int cn )
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

    CV_Assert( size.height >= nx && size.width >= nx );

    if( m == 3 )
    {
        size.width *= cn;

        // special case for 3x3 aperture, uses bitonic sort
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
        return;
    }

    for( x = 0; x < size.width; x++, dst += cn )
    {
        uchar* dst_cur = dst;
        uchar* src_top = src;
        uchar* src_bottom = src;
        int    k, c;
        int    x0 = -1;
        int    src_step1 = src_step, dst_step1 = dst_step;

        if( x % 2 != 0 )
        {
            src_bottom = src_top += src_step*(size.height-1);
            dst_cur += dst_step*(size.height-1);
            src_step1 = -src_step1;
            dst_step1 = -dst_step1;
        }

        if( x <= m/2 )
            nx++;

        if( nx < m )
            x0 = x < m/2 ? 0 : (nx-1)*cn;

        // init accumulator
        memset( zone0, 0, sizeof(zone0[0])*cn );
        memset( zone1, 0, sizeof(zone1[0])*cn );

        for( y = 0; y <= m/2; y++ )
        {
            for( c = 0; c < cn; c++ )
            {
                if( y > 0 )
                {
                    if( x0 >= 0 )
                        UPDATE_ACC01( src_bottom[x0+c], c, += (m - nx) );
                    for( k = 0; k < nx*cn; k += cn )
                        UPDATE_ACC01( src_bottom[k+c], c, ++ );
                }
                else
                {
                    if( x0 >= 0 )
                        UPDATE_ACC01( src_bottom[x0+c], c, += (m - nx)*(m/2+1) );
                    for( k = 0; k < nx*cn; k += cn )
                        UPDATE_ACC01( src_bottom[k+c], c, += m/2+1 );
                }
            }

            if( (src_step1 > 0 && y < size.height-1) ||
                (src_step1 < 0 && size.height-y-1 > 0) )
                src_bottom += src_step1;
        }

        for( y = 0; y < size.height; y++, dst_cur += dst_step1 )
        {
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

            if( y+1 == size.height )
                break;

            if( cn == 1 )
            {
                for( k = 0; k < nx; k++ )
                {
                    int p = src_top[k];
                    int q = src_bottom[k];
                    zone1[0][p]--;
                    zone0[0][p>>4]--;
                    zone1[0][q]++;
                    zone0[0][q>>4]++;
                }
            }
            else if( cn == 3 )
            {
                for( k = 0; k < nx*3; k += 3 )
                {
                    UPDATE_ACC01( src_top[k], 0, -- );
                    UPDATE_ACC01( src_top[k+1], 1, -- );
                    UPDATE_ACC01( src_top[k+2], 2, -- );

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
                    UPDATE_ACC01( src_top[k], 0, -- );
                    UPDATE_ACC01( src_top[k+1], 1, -- );
                    UPDATE_ACC01( src_top[k+2], 2, -- );
                    UPDATE_ACC01( src_top[k+3], 3, -- );

                    UPDATE_ACC01( src_bottom[k], 0, ++ );
                    UPDATE_ACC01( src_bottom[k+1], 1, ++ );
                    UPDATE_ACC01( src_bottom[k+2], 2, ++ );
                    UPDATE_ACC01( src_bottom[k+3], 3, ++ );
                }
            }

            if( x0 >= 0 )
            {
                for( c = 0; c < cn; c++ )
                {
                    UPDATE_ACC01( src_top[x0+c], c, -= (m - nx) );
                    UPDATE_ACC01( src_bottom[x0+c], c, += (m - nx) );
                }
            }

            if( (src_step1 > 0 && src_bottom + src_step1 < src_max) ||
                (src_step1 < 0 && src_bottom + src_step1 >= src) )
                src_bottom += src_step1;

            if( y >= m/2 )
                src_top += src_step1;
        }

        if( x >= m/2 )
            src += cn;
        if( src + nx*cn > src_right ) nx--;
    }
#undef N
#undef UPDATE_ACC
}


void medianBlur( const Mat& src, Mat& dst, int ksize )
{
    Size size = src.size();
    int cn = src.channels();
    double img_size_mp = (double)(size.width*size.height)/(1 << 20);

    dst.create( src.size(), src.type() );

    CV_Assert( src.depth() == CV_8U && (cn == 1 || cn == 3 || cn == 4) && src.data != dst.data );

    if( size.width < ksize*2 || size.height < ksize*2 ||
        ksize <= 3 + (img_size_mp < 1 ? 12 : img_size_mp < 4 ? 6 : 2)*(MEDIAN_HAVE_SIMD ? 1 : 3))
    {
        medianBlur_8u_CnR_Om( src.data, src.step, dst.data, dst.step, size, ksize, cn );
    }
    else
    {
        const int r = (ksize - 1) / 2;
        const int CACHE_SIZE = (int)(0.95 * 512 * 1024 / cn); // assume 512 kB cache size
        const int STRIPES = (int) cvCeil( (double)(size.width - 2*r) /
                (CACHE_SIZE / sizeof(Histogram) - 2*r) );
        const int STRIPE_SIZE = (int) cvCeil(
                (double) (size.width + STRIPES*2*r - 2*r ) / STRIPES);

        for( int i = 0; i < size.width; i += STRIPE_SIZE - 2*r )
        {
            int stripe = STRIPE_SIZE;
            // Make sure that the filter kernel fits into one stripe.
            if( i + STRIPE_SIZE - 2*r >= size.width ||
                size.width - (i + STRIPE_SIZE - 2*r) < 2*r+1 )
                stripe = size.width - i;

            medianBlur_8u_CnR_O1( src.data + cn*i, src.step,
                dst.data + cn*i, dst.step, Size(stripe, size.height),
                ksize, cn, i == 0, stripe == size.width - i );

            if( stripe == size.width - i )
                break;
        }
    }
}

/****************************************************************************************\
                                   Bilateral Filtering
\****************************************************************************************/

static void
bilateralFilter_8u( const Mat& src, Mat& dst, int d,
                    double sigma_color, double sigma_space,
                    int borderType )
{
    double gauss_color_coeff = -0.5/(sigma_color*sigma_color);
    double gauss_space_coeff = -0.5/(sigma_space*sigma_space);
    int cn = src.channels();
    int i, j, k, maxk, radius;
    Size size = src.size();

    CV_Assert( (src.type() == CV_8UC1 || src.type() == CV_8UC3) &&
        src.type() == dst.type() && src.size() == dst.size() &&
        src.data != dst.data );

    if( sigma_color <= 0 )
        sigma_color = 1;
    if( sigma_space <= 0 )
        sigma_space = 1;

    if( d <= 0 )
        radius = cvRound(sigma_space*1.5);
    else
        radius = d/2;
    radius = MAX(radius, 1);
    d = radius*2 + 1;

    Mat temp;
    copyMakeBorder( src, temp, radius, radius, radius, radius, borderType );

    Vector<float> _color_weight(cn*256);
    Vector<float> _space_weight(d*d);
    Vector<int> _space_ofs(d*d);
    float* color_weight = &_color_weight[0];
    float* space_weight = &_space_weight[0];
    int* space_ofs = &_space_ofs[0];

    // initialize color-related bilateral filter coefficients
    for( i = 0; i < 256*cn; i++ )
        color_weight[i] = (float)std::exp(i*i*gauss_color_coeff);

    // initialize space-related bilateral filter coefficients
    for( i = -radius, maxk = 0; i <= radius; i++ )
        for( j = -radius; j <= radius; j++ )
        {
            double r = std::sqrt((double)i*i + (double)j*j);
            if( r > radius )
                continue;
            space_weight[maxk] = (float)std::exp(r*r*gauss_space_coeff);
            space_ofs[maxk++] = i*temp.step + j*cn;
        }

    for( i = 0; i < size.height; i++ )
    {
        const uchar* sptr = temp.data + (i+radius)*temp.step + radius*cn;
        uchar* dptr = dst.data + i*dst.step;

        if( cn == 1 )
        {
            for( j = 0; j < size.width; j++ )
            {
                float sum = 0, wsum = 0;
                int val0 = sptr[j];
                for( k = 0; k < maxk; k++ )
                {
                    int val = sptr[j + space_ofs[k]];
                    float w = space_weight[k]*color_weight[std::abs(val - val0)];
                    sum += val*w;
                    wsum += w;
                }
                // overflow is not possible here => there is no need to use CV_CAST_8U
                dptr[j] = (uchar)cvRound(sum/wsum);
            }
        }
        else
        {
            assert( cn == 3 );
            for( j = 0; j < size.width*3; j += 3 )
            {
                float sum_b = 0, sum_g = 0, sum_r = 0, wsum = 0;
                int b0 = sptr[j], g0 = sptr[j+1], r0 = sptr[j+2];
                for( k = 0; k < maxk; k++ )
                {
                    const uchar* sptr_k = sptr + j + space_ofs[k];
                    int b = sptr_k[0], g = sptr_k[1], r = sptr_k[2];
                    float w = space_weight[k]*color_weight[std::abs(b - b0) +
                        std::abs(g - g0) + std::abs(r - r0)];
                    sum_b += b*w; sum_g += g*w; sum_r += r*w;
                    wsum += w;
                }
                wsum = 1.f/wsum;
                b0 = cvRound(sum_b*wsum);
                g0 = cvRound(sum_g*wsum);
                r0 = cvRound(sum_r*wsum);
                dptr[j] = (uchar)b0; dptr[j+1] = (uchar)g0; dptr[j+2] = (uchar)r0;
            }
        }
    }
}


static void
bilateralFilter_32f( const Mat& src, Mat& dst, int d,
                     double sigma_color, double sigma_space,
                     int borderType )
{
    double gauss_color_coeff = -0.5/(sigma_color*sigma_color);
    double gauss_space_coeff = -0.5/(sigma_space*sigma_space);
    int cn = src.channels();
    int i, j, k, maxk, radius;
    double minValSrc=-1, maxValSrc=1;
    const int kExpNumBinsPerChannel = 1 << 12;
    int kExpNumBins = 0;
    float lastExpVal = 1.f;
    float len, scale_index;
    Size size = src.size();

    CV_Assert( (src.type() == CV_32FC1 || src.type() == CV_32FC3) &&
        src.type() == dst.type() && src.size() == dst.size() &&
        src.data != dst.data );

    if( sigma_color <= 0 )
        sigma_color = 1;
    if( sigma_space <= 0 )
        sigma_space = 1;

    if( d <= 0 )
        radius = cvRound(sigma_space*1.5);
    else
        radius = d/2;
    radius = MAX(radius, 1);
    d = radius*2 + 1;
    // compute the min/max range for the input image (even if multichannel)
    
    minMaxLoc( src.reshape(1), &minValSrc, &maxValSrc );
    
    // temporary copy of the image with borders for easy processing
    Mat temp;
    copyMakeBorder( src, temp, radius, radius, radius, radius, borderType );

    // allocate lookup tables
    Vector<float> _space_weight(d*d);
    Vector<int> _space_ofs(d*d);
    float* space_weight = &_space_weight[0];
    int* space_ofs = &_space_ofs[0];

    // assign a length which is slightly more than needed
    len = (float)(maxValSrc - minValSrc) * cn;
    kExpNumBins = kExpNumBinsPerChannel * cn;
    Vector<float> _expLUT(kExpNumBins+2);
    float* expLUT = &_expLUT[0];

    scale_index = kExpNumBins/len;
    
    // initialize the exp LUT
    for( i = 0; i < kExpNumBins+2; i++ )
    {
        if( lastExpVal > 0.f )
        {
            double val =  i / scale_index;
            expLUT[i] = (float)std::exp(val * val * gauss_color_coeff);
            lastExpVal = expLUT[i];
        }
        else
            expLUT[i] = 0.f;
    }
    
    // initialize space-related bilateral filter coefficients
    for( i = -radius, maxk = 0; i <= radius; i++ )
        for( j = -radius; j <= radius; j++ )
        {
            double r = std::sqrt((double)i*i + (double)j*j);
            if( r > radius )
                continue;
            space_weight[maxk] = (float)std::exp(r*r*gauss_space_coeff);
            space_ofs[maxk++] = i*(temp.step/sizeof(float)) + j*cn;
        }

    for( i = 0; i < size.height; i++ )
    {
	    const float* sptr = (const float*)(temp.data + (i+radius)*temp.step) + radius*cn;
        float* dptr = (float*)(dst.data + i*dst.step);

        if( cn == 1 )
        {
            for( j = 0; j < size.width; j++ )
            {
                float sum = 0, wsum = 0;
                float val0 = sptr[j];
                for( k = 0; k < maxk; k++ )
                {
                    float val = sptr[j + space_ofs[k]];
					float alpha = (float)(std::abs(val - val0)*scale_index);
                    int idx = cvFloor(alpha);
                    alpha -= idx;
                    float w = space_weight[k]*(expLUT[idx] + alpha*(expLUT[idx+1] - expLUT[idx]));
	                sum += val*w;
                    wsum += w;
                }
                dptr[j] = (float)(sum/wsum);
            }
        }
        else
        {
            assert( cn == 3 );
            for( j = 0; j < size.width*3; j += 3 )
            {
                float sum_b = 0, sum_g = 0, sum_r = 0, wsum = 0;
                float b0 = sptr[j], g0 = sptr[j+1], r0 = sptr[j+2];
                for( k = 0; k < maxk; k++ )
                {
                    const float* sptr_k = sptr + j + space_ofs[k];
                    float b = sptr_k[0], g = sptr_k[1], r = sptr_k[2];
					float alpha = (float)((std::abs(b - b0) +
                        std::abs(g - g0) + std::abs(r - r0))*scale_index);
                    int idx = cvFloor(alpha);
                    alpha -= idx;
                    float w = space_weight[k]*(expLUT[idx] + alpha*(expLUT[idx+1] - expLUT[idx]));
                    sum_b += b*w; sum_g += g*w; sum_r += r*w;
                    wsum += w;
                }
                wsum = 1.f/wsum;
                b0 = sum_b*wsum;
                g0 = sum_g*wsum;
                r0 = sum_r*wsum;
                dptr[j] = b0; dptr[j+1] = g0; dptr[j+2] = r0;
            }
        }
    }
}


void bilateralFilter( const Mat& src, Mat& dst, int d,
                      double sigmaColor, double sigmaSpace,
                      int borderType )
{
    dst.create( src.size(), src.type() );
    if( src.depth() == CV_8U )
        bilateralFilter_8u( src, dst, d, sigmaColor, sigmaSpace, borderType );
    else if( src.depth() == CV_32F )
        bilateralFilter_32f( src, dst, d, sigmaColor, sigmaSpace, borderType );
    else
        CV_Error( CV_StsUnsupportedFormat,
        "Bilateral filtering is only implemented for 8u and 32f images" );
}

}

//////////////////////////////////////////////////////////////////////////////////////////

CV_IMPL void
cvSmooth( const void* srcarr, void* dstarr, int smooth_type,
          int param1, int param2, double param3, double param4 )
{
    cv::Mat src = cv::cvarrToMat(srcarr), dst0 = cv::cvarrToMat(dstarr), dst = dst0;

    CV_Assert( dst.size() == src.size() &&
        (smooth_type == CV_BLUR_NO_SCALE || dst.type() == src.type()) );

    if( param2 <= 0 )
        param2 = param1;

    if( smooth_type == CV_BLUR || smooth_type == CV_BLUR_NO_SCALE )
        cv::boxFilter( src, dst, dst.depth(), cv::Size(param1, param2), cv::Point(-1,-1),
            smooth_type == CV_BLUR, cv::BORDER_REPLICATE );
    else if( smooth_type == CV_GAUSSIAN )
        cv::GaussianBlur( src, dst, cv::Size(param1, param2), param3, param4, cv::BORDER_REPLICATE );
    else if( smooth_type == CV_MEDIAN )
        cv::medianBlur( src, dst, param1 );
    else
        cv::bilateralFilter( src, dst, param1, param3, param4, cv::BORDER_REPLICATE );

    if( dst.data != dst0.data )
        CV_Error( CV_StsUnmatchedFormats, "The destination image does not have the proper type" );
}

/* End of file. */
