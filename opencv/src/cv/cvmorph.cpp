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

/****************************************************************************************\
                     Basic Morphological Operations: Erosion & Dilation
\****************************************************************************************/

namespace cv
{

template<typename T> struct MinOp
{
    typedef T type1;
    typedef T type2;
    typedef T rtype;
    T operator ()(T a, T b) const { return std::min(a, b); }
};

template<typename T> struct MaxOp
{
    typedef T type1;
    typedef T type2;
    typedef T rtype;
    T operator ()(T a, T b) const { return std::max(a, b); }
};

#undef CV_MIN_8U
#undef CV_MAX_8U
#define CV_MIN_8U(a,b)       ((a) - CV_FAST_CAST_8U((a) - (b)))
#define CV_MAX_8U(a,b)       ((a) + CV_FAST_CAST_8U((b) - (a)))

template<> inline uchar MinOp<uchar>::operator ()(uchar a, uchar b) const { return CV_MIN_8U(a, b); }
template<> inline uchar MaxOp<uchar>::operator ()(uchar a, uchar b) const { return CV_MAX_8U(a, b); }

template<class Op> struct MorphRowFilter : public BaseRowFilter
{
    typedef typename Op::rtype T;
    
    MorphRowFilter( int _ksize, int _anchor )
    {
        ksize = _ksize;
        anchor = _anchor;
    }
    
    void operator()(const uchar* src, uchar* dst, int width, int cn)
    {
        int i, j, k, _ksize = ksize*cn;
        const T* S = (const T*)src;
        Op op;
        T* D = (T*)dst;
        width *= cn;

        if( _ksize == cn )
        {
            for( i = 0; i < width; i++ )
                D[i] = S[i];
            return;
        }

        for( k = 0; k < cn; k++, S++, D++ )
        {
            for( i = 0; i <= width - cn*2; i += cn*2 )
            {
                const T* s = S + i;
                T m = s[cn];
                for( j = cn*2; j < _ksize; j += cn )
                    m = op(m, s[j]);
                D[i] = op(m, s[0]);
                D[i+cn] = op(m, s[j]);
            }

            for( ; i < width; i += cn )
            {
                const T* s = S + i;
                T m = s[0];
                for( j = cn; j < _ksize; j += cn )
                    m = op(m, s[j]);
                D[i] = m;
            }
        }
    }
};


template<class Op> struct MorphColumnFilter : public BaseColumnFilter
{
    typedef typename Op::rtype T;
    
    MorphColumnFilter( int _ksize, int _anchor )
    {
        ksize = _ksize;
        anchor = _anchor;
    }
    
    void operator()(const uchar** _src, uchar* dst, int dststep, int count, int width)
    {
        int i, k, _ksize = ksize;
        const T** src = (const T**)_src;
        T* D = (T*)dst;
        Op op;
        dststep /= sizeof(D[0]);

        for( ; _ksize > 1 && count > 1; count -= 2, D += dststep*2, src += 2 )
        {
            for( i = 0; i <= width - 4; i += 4 )
            {
                const T* sptr = src[1] + i;
                T s0 = sptr[0], s1 = sptr[1], s2 = sptr[2], s3 = sptr[3];

                for( k = 2; k < _ksize; k++ )
                {
                    sptr = src[k] + i;
                    s0 = op(s0, sptr[0]); s1 = op(s1, sptr[1]);
                    s2 = op(s2, sptr[2]); s3 = op(s3, sptr[3]);
                }

                sptr = src[0] + i;
                D[i] = op(s0, sptr[0]);
                D[i+1] = op(s1, sptr[1]);
                D[i+2] = op(s2, sptr[2]);
                D[i+3] = op(s3, sptr[3]);

                sptr = src[k] + i;
                D[i+dststep] = op(s0, sptr[0]);
                D[i+dststep+1] = op(s1, sptr[1]);
                D[i+dststep+2] = op(s2, sptr[2]);
                D[i+dststep+3] = op(s3, sptr[3]);
            }

            for( ; i < width; i++ )
            {
                T s0 = src[1][i];

                for( k = 2; k < _ksize; k++ )
                    s0 = op(s0, src[k][i]);

                D[i] = op(s0, src[0][i]);
                D[i+dststep] = op(s0, src[k][i]);
            }
        }

        for( ; count > 0; count--, D += dststep, src++ )
        {
            for( i = 0; i <= width - 4; i += 4 )
            {
                const T* sptr = src[0] + i;
                T s0 = sptr[0], s1 = sptr[1], s2 = sptr[2], s3 = sptr[3];

                for( k = 1; k < _ksize; k++ )
                {
                    sptr = src[k] + i;
                    s0 = op(s0, sptr[0]); s1 = op(s1, sptr[1]);
                    s2 = op(s2, sptr[2]); s3 = op(s3, sptr[3]);
                }

                D[i] = s0; D[i+1] = s1;
                D[i+2] = s2; D[i+3] = s3;
            }

            for( ; i < width; i++ )
            {
                T s0 = src[0][i];
                for( k = 1; k < _ksize; k++ )
                    s0 = op(s0, src[k][i]);
                D[i] = s0;
            }
        }
    }
};


template<class Op> struct MorphFilter : BaseFilter
{
    typedef typename Op::rtype T;
    
    MorphFilter( const Mat& _kernel, Point _anchor )
    {
        anchor = _anchor;
        ksize = _kernel.size();
        CV_Assert( _kernel.type() == CV_8U );
        
        Vector<uchar> coeffs; // we do not really the values of non-zero
                              // kernel elements, just their locations 
        preprocess2DKernel( _kernel, coords, coeffs );
        ptrs.resize( coords.size() );
    }
    
    void operator()(const uchar** src, uchar* dst, int dststep, int count, int width, int cn)
    {
        const Point* pt = &coords[0];
        const T** kp = (const T**)&ptrs[0];
        int i, k, nz = (int)coords.size();
        Op op;

        width *= cn;
        for( ; count > 0; count--, dst += dststep, src++ )
        {
            T* D = (T*)dst;

            for( k = 0; k < nz; k++ )
                kp[k] = (const T*)src[pt[k].y] + pt[k].x*cn;

            for( i = 0; i <= width - 4; i += 4 )
            {
                const T* sptr = kp[0] + i;
                T s0 = sptr[0], s1 = sptr[1], s2 = sptr[2], s3 = sptr[3];

                for( k = 1; k < nz; k++ )
                {
                    sptr = kp[k] + i;
                    s0 = op(s0, sptr[0]); s1 = op(s1, sptr[1]);
                    s2 = op(s2, sptr[2]); s3 = op(s3, sptr[3]);
                }

                D[i] = s0; D[i+1] = s1;
                D[i+2] = s2; D[i+3] = s3;
            }

            for( ; i < width; i++ )
            {
                T s0 = kp[0][i];
                for( k = 1; k < nz; k++ )
                    s0 = op(s0, kp[k][i]);
                D[i] = s0;
            }
        }
    }

    Vector<Point> coords;
    Vector<uchar*> ptrs;
};

/////////////////////////////////// External Interface /////////////////////////////////////

Ptr<BaseRowFilter> getMorphologyRowFilter(int op, int type, int ksize, int anchor)
{
    int depth = CV_MAT_DEPTH(type);
    if( anchor < 0 )
        anchor = ksize/2;
    CV_Assert( op == MORPH_ERODE || op == MORPH_DILATE );
    if( op == MORPH_ERODE )
    {
        if( depth == CV_8U )
            return Ptr<BaseRowFilter>(new MorphRowFilter<MinOp<uchar> >(ksize, anchor));
        if( depth == CV_16U )
            return Ptr<BaseRowFilter>(new MorphRowFilter<MinOp<ushort> >(ksize, anchor));
        if( depth == CV_32F )
            return Ptr<BaseRowFilter>(new MorphRowFilter<MinOp<float> >(ksize, anchor));
    }
    else
    {
        if( depth == CV_8U )
            return Ptr<BaseRowFilter>(new MorphRowFilter<MaxOp<uchar> >(ksize, anchor));
        if( depth == CV_16U )
            return Ptr<BaseRowFilter>(new MorphRowFilter<MaxOp<ushort> >(ksize, anchor));
        if( depth == CV_32F )
            return Ptr<BaseRowFilter>(new MorphRowFilter<MaxOp<float> >(ksize, anchor));
    }

    CV_Error_( CV_StsNotImplemented, ("Unsupported data type (=%d)", type));
    return Ptr<BaseRowFilter>(0);
}

Ptr<BaseColumnFilter> getMorphologyColumnFilter(int op, int type, int ksize, int anchor)
{
    int depth = CV_MAT_DEPTH(type);
    if( anchor < 0 )
        anchor = ksize/2;
    CV_Assert( op == MORPH_ERODE || op == MORPH_DILATE );
    if( op == MORPH_ERODE )
    {
        if( depth == CV_8U )
            return Ptr<BaseColumnFilter>(new MorphColumnFilter<MinOp<uchar> >(ksize, anchor));
        if( depth == CV_16U )
            return Ptr<BaseColumnFilter>(new MorphColumnFilter<MinOp<ushort> >(ksize, anchor));
        if( depth == CV_32F )
            return Ptr<BaseColumnFilter>(new MorphColumnFilter<MinOp<float> >(ksize, anchor));
    }
    else
    {
        if( depth == CV_8U )
            return Ptr<BaseColumnFilter>(new MorphColumnFilter<MaxOp<uchar> >(ksize, anchor));
        if( depth == CV_16U )
            return Ptr<BaseColumnFilter>(new MorphColumnFilter<MaxOp<ushort> >(ksize, anchor));
        if( depth == CV_32F )
            return Ptr<BaseColumnFilter>(new MorphColumnFilter<MaxOp<float> >(ksize, anchor));
    }

    CV_Error_( CV_StsNotImplemented, ("Unsupported data type (=%d)", type));
    return Ptr<BaseColumnFilter>(0);
}


Ptr<BaseFilter> getMorphologyFilter(int op, int type, const Mat& kernel, Point anchor)
{
    int depth = CV_MAT_DEPTH(type);
    anchor = normalizeAnchor(anchor, kernel.size());
    CV_Assert( op == MORPH_ERODE || op == MORPH_DILATE );
    if( op == MORPH_ERODE )
    {
        if( depth == CV_8U )
            return Ptr<BaseFilter>(new MorphFilter<MinOp<uchar> >(kernel, anchor));
        if( depth == CV_16U )
            return Ptr<BaseFilter>(new MorphFilter<MinOp<ushort> >(kernel, anchor));
        if( depth == CV_32F )
            return Ptr<BaseFilter>(new MorphFilter<MinOp<float> >(kernel, anchor));
    }
    else
    {
        if( depth == CV_8U )
            return Ptr<BaseFilter>(new MorphFilter<MaxOp<uchar> >(kernel, anchor));
        if( depth == CV_16U )
            return Ptr<BaseFilter>(new MorphFilter<MaxOp<ushort> >(kernel, anchor));
        if( depth == CV_32F )
            return Ptr<BaseFilter>(new MorphFilter<MaxOp<float> >(kernel, anchor));
    }

    CV_Error_( CV_StsNotImplemented, ("Unsupported data type (=%d)", type));
    return Ptr<BaseFilter>(0);
}


Ptr<FilterEngine> createMorphologyFilter( int op, int type, const Mat& kernel,
         Point anchor, int _rowBorderType, int _columnBorderType,
         const Scalar& _borderValue, int maxBufRows )
{
    anchor = normalizeAnchor(anchor, kernel.size());
    
    Ptr<BaseRowFilter> rowFilter;
    Ptr<BaseColumnFilter> columnFilter;
    Ptr<BaseFilter> filter2D;

    if( countNonZero(kernel) == kernel.rows*kernel.cols )
    {
        // rectangular structuring element 
        rowFilter = getMorphologyRowFilter(op, type, kernel.cols, anchor.x);
        columnFilter = getMorphologyColumnFilter(op, type, kernel.rows, anchor.y);
    }
    else
        filter2D = getMorphologyFilter(op, type, kernel, anchor);

    Scalar borderValue = _borderValue;
    if( (_rowBorderType == BORDER_CONSTANT || _columnBorderType == BORDER_CONSTANT) &&
        borderValue == morphologyDefaultBorderValue() )
    {
        int depth = CV_MAT_TYPE(type);
        CV_Assert( depth == CV_8U || depth == CV_16U || depth == CV_32F );
        if( op == MORPH_ERODE )
            borderValue = Scalar::all( depth == CV_8U ? (double)UCHAR_MAX :
                depth == CV_16U ? (double)USHRT_MAX : (double)FLT_MAX );
        else
            borderValue = Scalar::all( depth == CV_8U || depth == CV_16U ?
                0. : (double)-FLT_MAX );
    }

    return Ptr<FilterEngine>(new FilterEngine(filter2D, rowFilter, columnFilter,
        type, type, type, _rowBorderType, _columnBorderType, borderValue, maxBufRows ));
}


Mat getStructuringElement(int shape, Size ksize, Point anchor)
{
    int i, j;
    int r = 0, c = 0;
    double inv_r2 = 0;

    CV_Assert( shape == MORPH_RECT || shape == MORPH_CROSS || shape == MORPH_ELLIPSE );

    anchor = normalizeAnchor(anchor, ksize);

    if( ksize == Size(1,1) )
        shape = MORPH_RECT;

    if( shape == MORPH_ELLIPSE )
    {
        r = ksize.height/2;
        c = ksize.width/2;
        inv_r2 = r ? 1./((double)r*r) : 0;
    }

    Mat elem(ksize, CV_8U);

    for( i = 0; i < ksize.height; i++ )
    {
        uchar* ptr = elem.data + i*elem.step;
        int j1 = 0, j2 = 0;

        if( shape == MORPH_RECT || (shape == MORPH_CROSS && i == anchor.y) )
            j2 = ksize.width;
        else if( shape == MORPH_CROSS )
            j1 = anchor.x, j2 = j1 + 1;
        else
        {
            int dy = i - r;
            if( std::abs(dy) <= r )
            {
                int dx = saturate_cast<int>(c*std::sqrt((r*r - dy*dy)*inv_r2));
                j1 = std::max( c - dx, 0 );
                j2 = std::min( c + dx + 1, ksize.width );
            }
        }

        for( j = 0; j < j1; j++ )
            ptr[j] = 0;
        for( ; j < j2; j++ )
            ptr[j] = 1;
        for( ; j < ksize.width; j++ )
            ptr[j] = 0;
    }

    return elem;
}

static void morphOp( int op, const Mat& src, Mat& dst, const Mat& _kernel,
                     Point anchor, int iterations,
                     int borderType, const Scalar& borderValue )
{
    Mat kernel;
    Size ksize = _kernel.data ? _kernel.size() : Size(3,3);
    anchor = normalizeAnchor(anchor, ksize);

    CV_Assert( anchor.inside(Rect(0, 0, ksize.width, ksize.height)) );

    if( iterations == 0 || _kernel.rows*_kernel.cols == 1 )
    {
        src.copyTo(dst);
        return;
    }

    dst.create( src.size(), src.type() );

    if( !_kernel.data )
    {
       kernel = getStructuringElement(MORPH_RECT, Size(1+iterations*2,1+iterations*2));
       iterations = 1;
    }
    else if( iterations > 1 && countNonZero(_kernel) == _kernel.rows*_kernel.cols )
    {
        kernel = getStructuringElement(MORPH_RECT,
                Size(ksize.width + iterations*(ksize.width-1),
                     ksize.height + iterations*(ksize.height-1)),
                Point(anchor.x*iterations, anchor.y*iterations));
        iterations = 1;
    }
    else
        kernel = _kernel;

    Ptr<FilterEngine> f = createMorphologyFilter(op, src.type(),
        kernel, anchor, borderType, borderType, borderValue );

    f->apply( src, dst );
    for( int i = 1; i < iterations; i++ )
        f->apply( dst, dst );
}


void erode( const Mat& src, Mat& dst, const Mat& kernel,
            Point anchor, int iterations,
            int borderType, const Scalar& borderValue )
{
    morphOp( MORPH_ERODE, src, dst, kernel, anchor, iterations, borderType, borderValue );
}


void dilate( const Mat& src, Mat& dst, const Mat& kernel,
             Point anchor, int iterations,
             int borderType, const Scalar& borderValue )
{
    morphOp( MORPH_DILATE, src, dst, kernel, anchor, iterations, borderType, borderValue );
}


void morphologyEx( const Mat& src, Mat& dst, int op, const Mat& kernel,
                   Point anchor, int iterations, int borderType,
                   const Scalar& borderValue )
{
    Mat temp;
    switch( op )
    {
    case MORPH_OPEN:
        erode( src, dst, kernel, anchor, iterations, borderType, borderValue );
        dilate( dst, dst, kernel, anchor, iterations, borderType, borderValue );
        break;
    case CV_MOP_CLOSE:
        dilate( src, dst, kernel, anchor, iterations, borderType, borderValue );
        erode( dst, dst, kernel, anchor, iterations, borderType, borderValue );
        break;
    case CV_MOP_GRADIENT:
        erode( src, temp, kernel, anchor, iterations, borderType, borderValue );
        dilate( src, dst, kernel, anchor, iterations, borderType, borderValue );
        dst -= temp;
        break;
    case CV_MOP_TOPHAT:
        if( src.data != dst.data )
            temp = dst;
        erode( src, temp, kernel, anchor, iterations, borderType, borderValue );
        dilate( temp, temp, kernel, anchor, iterations, borderType, borderValue );
        dst = src - temp;
        break;
    case CV_MOP_BLACKHAT:
        if( src.data != dst.data )
            temp = dst;
        dilate( src, temp, kernel, anchor, iterations, borderType, borderValue );
        erode( temp, temp, kernel, anchor, iterations, borderType, borderValue );        
        dst = temp - src;
        break;
    default:
        CV_Error( CV_StsBadArg, "unknown morphological operation" );
    }
}

}

CV_IMPL IplConvKernel *
cvCreateStructuringElementEx( int cols, int rows,
                              int anchorX, int anchorY,
                              int shape, int *values )
{
    cv::Size ksize = cv::Size(cols, rows);
    cv::Point anchor = cv::Point(anchorX, anchorY);
    CV_Assert( cols > 0 && rows > 0 && anchor.inside(cv::Rect(0,0,cols,rows)) &&
        (shape != CV_SHAPE_CUSTOM || values != 0));

    int i, size = rows * cols;
    int element_size = sizeof(IplConvKernel) + size*sizeof(int);
    IplConvKernel *element = (IplConvKernel*)cvAlloc(element_size + 32);

    element->nCols = cols;
    element->nRows = rows;
    element->anchorX = anchorX;
    element->anchorY = anchorY;
    element->nShiftR = shape < CV_SHAPE_ELLIPSE ? shape : CV_SHAPE_CUSTOM;
    element->values = (int*)(element + 1);

    if( shape == CV_SHAPE_CUSTOM )
    {
        for( i = 0; i < size; i++ )
            element->values[i] = values[i];
    }
    else
    {
        cv::Mat elem = cv::getStructuringElement(shape, ksize, anchor);
        for( i = 0; i < size; i++ )
            element->values[i] = elem.data[i];
    }

    return element;
}


CV_IMPL void
cvReleaseStructuringElement( IplConvKernel ** element )
{
    if( !element )
        CV_Error( CV_StsNullPtr, "" );
    cvFree( element );
}


static void convertConvKernel( const IplConvKernel* src, cv::Mat& dst, cv::Point& anchor )
{
    if(!src)
    {
        anchor = cv::Point(1,1);
        dst.release();
        return;
    }
    anchor = cv::Point(src->anchorX, src->anchorY);
    dst.create(src->nRows, src->nCols, CV_8U);

    int i, size = src->nRows*src->nCols;
    for( i = 0; i < size; i++ )
        dst.data[i] = (uchar)src->values[i];
}


CV_IMPL void
cvErode( const CvArr* srcarr, CvArr* dstarr, IplConvKernel* element, int iterations )
{
    cv::Mat src = cv::cvarrToMat(srcarr), dst = cv::cvarrToMat(dstarr), kernel;
    CV_Assert( element && src.size() == dst.size() && src.type() == dst.type() );
    cv::Point anchor;
    convertConvKernel( element, kernel, anchor );
    cv::erode( src, dst, kernel, anchor, iterations, cv::BORDER_REPLICATE );
}


CV_IMPL void
cvDilate( const CvArr* srcarr, CvArr* dstarr, IplConvKernel* element, int iterations )
{
    cv::Mat src = cv::cvarrToMat(srcarr), dst = cv::cvarrToMat(dstarr), kernel;
    CV_Assert( src.size() == dst.size() && src.type() == dst.type() );
    cv::Point anchor;
    convertConvKernel( element, kernel, anchor );
    cv::dilate( src, dst, kernel, anchor, iterations, cv::BORDER_REPLICATE );
}


CV_IMPL void
cvMorphologyEx( const void* srcarr, void* dstarr, void*,
                IplConvKernel* element, int op, int iterations )
{
    cv::Mat src = cv::cvarrToMat(srcarr), dst = cv::cvarrToMat(dstarr), kernel;
    CV_Assert( element && src.size() == dst.size() && src.type() == dst.type() );
    cv::Point anchor;
    convertConvKernel( element, kernel, anchor );
    cv::morphologyEx( src, dst, op, kernel, anchor, iterations, cv::BORDER_REPLICATE );
}

/* End of file. */
