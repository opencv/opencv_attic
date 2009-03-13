/*M///////////////////////////////////////////////////////////////////////////////////////
//
//  IMPORTANT: READ BEFORE DOWNLOADING, COPYING, INSTALLING OR USING.
//
//  By downloading, copying, installing or using the software you agree to this license.
//  If you do not agree to this license, do not download, install,
//  copy or use the software.
//
//
//                           License Agreement
//                For Open Source Computer Vision Library
//
// Copyright (C) 2000-2008, Intel Corporation, all rights reserved.
// Copyright (C) 2009, Willow Garage Inc., all rights reserved.
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
//   * The name of the copyright holders may not be used to endorse or promote products
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

#include "_cxcore.h"

/****************************************************************************************\
*                           [scaled] Identity matrix initialization                      *
\****************************************************************************************/

namespace cv {

Mat::Mat(const IplImage* img, bool copyData)
    : flags(0), rows(0), cols(0), step(0), data(0),
      refcount(0), datastart(0), dataend(0)
{
    CvMat m, dst;                                
    int coi=0;
    cvGetMat( img, &m, &coi );
    
    if( copyData )
    {
        if( coi == 0 )
        {
            create( m.rows, m.cols, CV_MAT_TYPE(m.type) );
            dst = *this;
            cvCopy( &m, &dst );
        }
        else
        {
            create( m.rows, m.cols, CV_MAT_DEPTH(m.type) );
            dst = *this;
            CvMat* pdst = &dst;
            const int pairs[] = { coi-1, 0 };
            cvMixChannels( (const CvArr**)&img, 1, (CvArr**)&pdst, 1, pairs, 1 );
        }
    }
    else
    {
        /*if( coi != 0 )
            CV_Error(CV_BadCOI, "When copyData=false, COI must not be set");*/

        *this = Mat(m.rows, m.cols, CV_MAT_TYPE(m.type), m.data.ptr, m.step);
        /*if( img->roi )
        {
            datastart = (uchar*)img->imageData;
            dataend = datastart + img->imageSize;
        }*/
    }
}

Mat cvarrToMat(const CvArr* arr, bool copyData, bool allowND)
{
    Mat m;
    if( CV_IS_MAT(arr) )
        m = Mat((const CvMat*)arr, copyData );
    else if( CV_IS_IMAGE(arr) )
        m = Mat((const IplImage*)arr, copyData );
    else
    {
        CvMat hdr, *cvmat = cvGetMat( arr, &hdr, 0, allowND ? 1 : 0 );
        if( cvmat )
            m = Mat(cvmat, copyData);
    }
    return m;
}

Mat extractImageCOI(const CvArr* arr)
{
    Mat mat = cvarrToMat(arr), ch( mat.size(), mat.depth());
    int coi = 0;
    CV_Assert( CV_IS_IMAGE(arr) && (coi = cvGetImageCOI((const IplImage*)arr)) > 0 );
    Vector<Mat> src(&mat, 1), dst(&ch, 1);
    int _pairs[] = { coi-1, 0 };
    Vector<int> pairs(_pairs, 2);
    mixChannels( src, dst, pairs );

    return ch;
}

Mat Mat::reshape(int new_cn, int new_rows) const
{
    Mat hdr = *this;

    int cn = channels();
    if( new_cn == 0 )
        new_cn = cn;

    int total_width = cols * cn;

    if( (new_cn > total_width || total_width % new_cn != 0) && new_rows == 0 )
        new_rows = rows * total_width / new_cn;

    if( new_rows != 0 && new_rows != rows )
    {
        int total_size = total_width * rows;
        if( !isContinuous() )
            CV_Error( CV_BadStep,
            "The matrix is not continuous, thus its number of rows can not be changed" );

        if( (unsigned)new_rows > (unsigned)total_size )
            CV_Error( CV_StsOutOfRange, "Bad new number of rows" );

        total_width = total_size / new_rows;

        if( total_width * new_rows != total_size )
            CV_Error( CV_StsBadArg, "The total number of matrix elements "
                                    "is not divisible by the new number of rows" );

        hdr.rows = new_rows;
        hdr.step = total_width * elemSize1();
    }

    int new_width = total_width / new_cn;

    if( new_width * new_cn != total_width )
        CV_Error( CV_BadNumChannels,
        "The total width is not divisible by the new number of channels" );

    hdr.cols = new_width;
    hdr.flags = (hdr.flags & ~CV_MAT_CN_MASK) | ((new_cn-1) << CV_CN_SHIFT);
    return hdr;
}


void
setIdentity( Mat& m, const Scalar& s )
{
    int i, j, rows = m.rows, cols = m.cols, type = m.type();
    
    if( type == CV_32FC1 )
    {
        float* data = (float*)m.data;
        float val = (float)s[0];
        int step = m.step/sizeof(data[0]);

        for( i = 0; i < rows; i++, data += step )
        {
            for( j = 0; j < cols; j++ )
                data[j] = 0;
            if( i < cols )
                data[i] = val;
        }
    }
    else if( type == CV_64FC1 )
    {
        double* data = (double*)m.data;
        double val = s[0];
        int step = m.step/sizeof(data[0]);

        for( i = 0; i < rows; i++, data += step )
        {
            for( j = 0; j < cols; j++ )
                data[j] = 0;
            if( i < cols )
                data[i] = val;
        }
    }
    else
    {
        m = Scalar(0);
        m.diag() = s;
    }
}

Scalar trace( const Mat& m )
{
    int i, type = m.type();
    int nm = std::min(m.rows, m.cols);
    
    if( type == CV_32FC1 )
    {
        const float* ptr = (const float*)m.data;
        int step = m.step/sizeof(ptr[0]) + 1;
        double _s = 0;
        for( i = 0; i < nm; i++ )
            _s += ptr[i*step];
        return _s;
    }
    
    if( type == CV_64FC1 )
    {
        const double* ptr = (const double*)m.data;
        int step = m.step/sizeof(ptr[0]) + 1;
        double _s = 0;
        for( i = 0; i < nm; i++ )
            _s += ptr[i*step];
        return _s;
    }
    
    return cv::sum(m.diag());
}


/****************************************************************************************\
*                                       transpose                                        *
\****************************************************************************************/

template<typename T> static void
transposeI_( Mat& mat )
{
    int rows = mat.rows, cols = mat.cols;
    uchar* data = mat.data;
    int step = mat.step;

    for( int i = 0; i < rows; i++ )
    {
        T* row = (T*)(data + step*i);
        uchar* data1 = data + i*sizeof(T);
        for( int j = i+1; j < cols; j++ )
            std::swap( row[j], *(T*)(data1 + step*j) );
    }
}

template<typename T> static void
transpose_( const Mat& src, Mat& dst )
{
    int rows = dst.rows, cols = dst.cols;
    uchar* data = src.data;
    int step = src.step;

    for( int i = 0; i < rows; i++ )
    {
        T* row = (T*)(dst.data + dst.step*i);
        uchar* data1 = data + i*sizeof(T);
        for( int j = 0; j < cols; j++ )
            row[j] = *(T*)(data1 + step*j);
    }
}

typedef void (*TransposeInplaceFunc)( Mat& mat );
typedef void (*TransposeFunc)( const Mat& src, Mat& dst );

void transpose( const Mat& src, Mat& dst )
{
    TransposeInplaceFunc itab[] =
    {
        0,
        transposeI_<uchar>, // 1
        transposeI_<ushort>, // 2
        transposeI_<Vec_<uchar,3> >, // 3
        transposeI_<int>, // 4
        0,
        transposeI_<Vec_<ushort,3> >, // 6
        0,
        transposeI_<int64>, // 8
        0, 0, 0,
        transposeI_<Vec_<int,3> >, // 12
        0, 0, 0,
        transposeI_<Vec_<int64,2> >, // 16
        0, 0, 0, 0, 0, 0, 0,
        transposeI_<Vec_<int64,3> >, // 24
        0, 0, 0, 0, 0, 0, 0,
        transposeI_<Vec_<int64,4> > // 32
    };

    TransposeFunc tab[] =
    {
        0,
        transpose_<uchar>, // 1
        transpose_<ushort>, // 2
        transpose_<Vec_<uchar,3> >, // 3
        transpose_<int>, // 4
        0,
        transpose_<Vec_<ushort,3> >, // 6
        0,
        transpose_<int64>, // 8
        0, 0, 0,
        transpose_<Vec_<int,3> >, // 12
        0, 0, 0,
        transpose_<Vec_<int64,2> >, // 16
        0, 0, 0, 0, 0, 0, 0,
        transpose_<Vec_<int64,3> >, // 24
        0, 0, 0, 0, 0, 0, 0,
        transpose_<Vec_<int64,4> > // 32
    };

    int esz = src.elemSize();
    CV_Assert( esz <= 32 );

    if( dst.data == src.data )
    {
        TransposeInplaceFunc func = itab[esz];
        CV_Assert( src.rows == src.cols && func != 0 );
        func( dst );
    }
    else
    {
        dst.create( src.cols, src.rows, src.type() );
        TransposeFunc func = tab[esz];
        CV_Assert( func != 0 );
        func( src, dst );
    }
}


void completeSymm( Mat& matrix, bool LtoR )
{
    int i, j, nrows = matrix.rows, type = matrix.type();
    int j0 = 0, j1 = nrows;
    CV_Assert( matrix.rows == matrix.cols );

    if( type == CV_32FC1 || type == CV_32SC1 )
    {
        int* data = (int*)matrix.data;
        int step = matrix.step/sizeof(data[0]);
        for( i = 0; i < nrows; i++ )
        {
            if( !LtoR ) j1 = i; else j0 = i+1;
            for( j = j0; j < j1; j++ )
                data[i*step + j] = data[j*step + i];
        }
    }
    else if( type == CV_64FC1 )
    {
        double* data = (double*)matrix.data;
        int step = matrix.step/sizeof(data[0]);
        for( i = 0; i < nrows; i++ )
        {
            if( !LtoR ) j1 = i; else j0 = i+1;
            for( j = j0; j < j1; j++ )
                data[i*step + j] = data[j*step + i];
        }
    }
    else
        CV_Error( CV_StsUnsupportedFormat, "" );
}

Mat Mat::cross(const Mat& m) const
{
    int t = type(), d = CV_MAT_DEPTH(t);
    CV_Assert( size() == m.size() && t == m.type() &&
        ((rows == 3 && cols == 1) || (cols*channels() == 3 && rows == 1)));
    Mat result(rows, cols, t);

    if( d == CV_32F )
    {
        const float *a = (const float*)data, *b = (const float*)m.data;
        float* c = (float*)result.data;
        int lda = rows > 1 ? step/sizeof(a[0]) : 1;
        int ldb = rows > 1 ? m.step/sizeof(b[0]) : 1;

        c[0] = a[lda] * b[ldb*2] - a[lda*2] * b[ldb];
        c[1] = a[lda*2] * b[0] - a[0] * b[ldb*2];
        c[2] = a[0] * b[ldb] - a[lda] * b[0];
    }
    else if( d == CV_64F )
    {
        const double *a = (const double*)data, *b = (const double*)m.data;
        double* c = (double*)result.data;
        int lda = rows > 1 ? step/sizeof(a[0]) : 1;
        int ldb = rows > 1 ? m.step/sizeof(b[0]) : 1;

        c[0] = a[lda] * b[ldb*2] - a[lda*2] * b[ldb];
        c[1] = a[lda*2] * b[0] - a[0] * b[ldb*2];
        c[2] = a[0] * b[ldb] - a[lda] * b[0];
    }

    return result;
}


/****************************************************************************************\
*                                Reduce Mat to Vector                                 *
\****************************************************************************************/

template<typename T, typename ST, class Op> static void
reduceR_( const Mat& srcmat, Mat& dstmat )
{
    typedef typename Op::rtype WT;
    Size size = srcmat.size();
    size.width *= srcmat.channels();
    AutoBuffer<WT> buffer(size.width);
    WT* buf = buffer;
    ST* dst = (ST*)dstmat.data;
    const T* src = (const T*)srcmat.data;
    int i, srcstep = srcmat.step/sizeof(src[0]);
    Op op;

    for( i = 0; i < size.width; i++ )
        buf[i] = src[i];

    for( ; --size.height; )
    {
        src += srcstep;
        for( i = 0; i <= size.width - 4; i += 4 )
        {
            WT s0, s1;
            s0 = op(buf[i], (WT)src[i]);
            s1 = op(buf[i+1], (WT)src[i+1]);
            buf[i] = s0; buf[i+1] = s1;

            s0 = op(buf[i+2], (WT)src[i+2]);
            s1 = op(buf[i+3], (WT)src[i+3]);
            buf[i+2] = s0; buf[i+3] = s1;
        }

        for( ; i < size.width; i++ )
            buf[i] = op(buf[i], (WT)src[i]);
    }

    for( i = 0; i < size.width; i++ )
        dst[i] = (ST)buf[i];
}


template<typename T, typename ST, class Op> static void
reduceC_( const Mat& srcmat, Mat& dstmat )
{
    typedef typename Op::rtype WT;
    Size size = srcmat.size();
    int i, k, cn = srcmat.channels();
    size.width *= cn;
    Op op;

    for( int y = 0; y < size.height; y++ )
    {
        const T* src = (const T*)(srcmat.data + srcmat.step*y);
        ST* dst = (ST*)(dstmat.data + dstmat.step*y);
        if( size.width == cn )
            for( k = 0; k < cn; k++ )
                dst[k] = src[k];
        else
        {
            for( k = 0; k < cn; k++ )
            {
                WT a0 = src[k], a1 = src[k+cn];
                for( i = 2*cn; i <= size.width - 4*cn; i += 4*cn )
                {
                    a0 = op(a0, (WT)src[i+k]);
                    a1 = op(a1, (WT)src[i+k+cn]);
                    a0 = op(a0, (WT)src[i+k+cn*2]);
                    a1 = op(a1, (WT)src[i+k+cn*3]);
                }

                for( ; i < size.width; i += cn )
                {
                    a0 = op(a0, (WT)src[i]);
                }
                a0 = op(a0, a1);
                dst[k] = (ST)a0;
            }
        }
    }
}

typedef void (*ReduceFunc)( const Mat& src, Mat& dst );

void reduce(const Mat& src, Mat& dst, int dim, int op, int dtype)
{
    int op0 = op;
    int stype = src.type(), sdepth = src.depth();
    if( dtype < 0 )
        dtype = stype;
    int ddepth = CV_MAT_DEPTH(dtype);

    dst.create(dim == 0 ? 1 : src.rows, dim == 0 ? src.cols : 1, dtype >= 0 ? dtype : stype);
    Mat temp = dst;
    
    CV_Assert( op == CV_REDUCE_SUM || op == CV_REDUCE_MAX ||
        op == CV_REDUCE_MIN || op == CV_REDUCE_AVG );
    CV_Assert( src.channels() == dst.channels() );

    if( op == CV_REDUCE_AVG )
    {
        op = CV_REDUCE_SUM;
        if( sdepth < CV_32S && ddepth < CV_32S )
            temp.create(dst.rows, dst.cols, CV_32SC(src.channels()));
    }

    ReduceFunc func = 0;
    if( dim == 0 )
    {
        if( op == CV_REDUCE_SUM )
        {
            if(sdepth == CV_8U && ddepth == CV_32S)
                func = reduceR_<uchar,int,OpAdd<int> >;
            if(sdepth == CV_8U && ddepth == CV_32F)
                func = reduceR_<uchar,float,OpAdd<int> >;
            if(sdepth == CV_8U && ddepth == CV_64F)
                func = reduceR_<uchar,double,OpAdd<int> >;
            if(sdepth == CV_16U && ddepth == CV_32F)
                func = reduceR_<ushort,float,OpAdd<float> >;
            if(sdepth == CV_16U && ddepth == CV_64F)
                func = reduceR_<ushort,double,OpAdd<float> >;
            if(sdepth == CV_16S && ddepth == CV_32F)
                func = reduceR_<short,float,OpAdd<float> >;
            if(sdepth == CV_16S && ddepth == CV_64F)
                func = reduceR_<short,double,OpAdd<float> >;
            if(sdepth == CV_32F && ddepth == CV_32F)
                func = reduceR_<float,float,OpAdd<float> >;
            if(sdepth == CV_32F && ddepth == CV_64F)
                func = reduceR_<float,double,OpAdd<double> >;
            if(sdepth == CV_64F && ddepth == CV_64F)
                func = reduceR_<double,double,OpAdd<double> >;
        }
        else if(op == CV_REDUCE_MAX)
        {
            if(sdepth == CV_8U && ddepth == CV_8U)
                func = reduceR_<uchar, uchar, OpMax<uchar> >;
            if(sdepth == CV_32F && ddepth == CV_32F)
                func = reduceR_<float, float, OpMax<float> >;
            if(sdepth == CV_64F && ddepth == CV_64F)
                func = reduceR_<double, double, OpMax<double> >;
        }
        else if(op == CV_REDUCE_MIN)
        {
            if(sdepth == CV_8U && ddepth == CV_8U)
                func = reduceR_<uchar, uchar, OpMin<uchar> >;
            if(sdepth == CV_32F && ddepth == CV_32F)
                func = reduceR_<float, float, OpMin<float> >;
            if(sdepth == CV_64F && ddepth == CV_64F)
                func = reduceR_<double, double, OpMin<double> >;
        }
    }
    else
    {
        if(op == CV_REDUCE_SUM)
        {
            if(sdepth == CV_8U && ddepth == CV_32S)
                func = reduceC_<uchar,int,OpAdd<int> >;
            if(sdepth == CV_8U && ddepth == CV_32F)
                func = reduceC_<uchar,float,OpAdd<int> >;
            if(sdepth == CV_8U && ddepth == CV_64F)
                func = reduceC_<uchar,double,OpAdd<int> >;
            if(sdepth == CV_16U && ddepth == CV_32F)
                func = reduceC_<ushort,float,OpAdd<float> >;
            if(sdepth == CV_16U && ddepth == CV_64F)
                func = reduceC_<ushort,double,OpAdd<float> >;
            if(sdepth == CV_16S && ddepth == CV_32F)
                func = reduceC_<short,float,OpAdd<float> >;
            if(sdepth == CV_16S && ddepth == CV_64F)
                func = reduceC_<short,double,OpAdd<float> >;
            if(sdepth == CV_32F && ddepth == CV_32F)
                func = reduceC_<float,float,OpAdd<float> >;
            if(sdepth == CV_32F && ddepth == CV_64F)
                func = reduceC_<float,double,OpAdd<double> >;
            if(sdepth == CV_64F && ddepth == CV_64F)
                func = reduceC_<double,double,OpAdd<double> >;
        }
        else if(op == CV_REDUCE_MAX)
        {
            if(sdepth == CV_8U && ddepth == CV_8U)
                func = reduceC_<uchar, uchar, OpMax<uchar> >;
            if(sdepth == CV_32F && ddepth == CV_32F)
                func = reduceC_<float, float, OpMax<float> >;
            if(sdepth == CV_64F && ddepth == CV_64F)
                func = reduceC_<double, double, OpMax<double> >;
        }
        else if(op == CV_REDUCE_MIN)
        {
            if(sdepth == CV_8U && ddepth == CV_8U)
                func = reduceC_<uchar, uchar, OpMin<uchar> >;
            if(sdepth == CV_32F && ddepth == CV_32F)
                func = reduceC_<float, float, OpMin<float> >;
            if(sdepth == CV_64F && ddepth == CV_64F)
                func = reduceC_<double, double, OpMin<double> >;
        }
    }

    if( !func )
        CV_Error( CV_StsUnsupportedFormat,
        "Unsupported combination of input and output array formats" );

    func( src, temp );

    if( op0 == CV_REDUCE_AVG )
        temp.convertTo(dst, dst.type(), 1./(dim == 0 ? src.rows : src.cols));
}


template<typename T> static void sort_( const Mat& src, Mat& dst, int flags )
{
    AutoBuffer<T> buf;
    T* bptr;
    Vector<T> v;
    int i, j, n, len;
    bool sortRows = (flags & 1) == CV_SORT_EVERY_ROW;
    bool inplace = src.data == dst.data;
    bool sortDescending = (flags & CV_SORT_DESCENDING) != 0;
    
    if( sortRows )
        n = src.rows, len = src.cols;
    else
    {
        n = src.cols, len = src.rows;
        buf.allocate(len);
    }
    bptr = (T*)buf;

    for( i = 0; i < n; i++ )
    {
        T* ptr = bptr;
        if( sortRows )
        {
            T* dptr = (T*)(dst.data + dst.step*i);
            if( !inplace )
            {
                const T* sptr = (const T*)(src.data + src.step*i);
                for( j = 0; j < len; j++ )
                    dptr[j] = sptr[j];
            }
            ptr = dptr;
        }
        else
        {
            for( j = 0; j < len; j++ )
                ptr[j] = ((const T*)(src.data + src.step*j))[i];
        }
        v.set(ptr, len, false);
        sort( v, LessThan<T>() );
        if( sortDescending )
            for( j = 0; j < len/2; j++ )
                std::swap(ptr[j], ptr[len-1-j]);
        if( !sortRows )
            for( j = 0; j < len; j++ )
                ((T*)(dst.data + dst.step*j))[i] = ptr[j];
    }
}


template<typename T> static void sortIdx_( const Mat& src, Mat& dst, int flags )
{
    AutoBuffer<T> buf;
    AutoBuffer<int> ibuf;
    T* bptr;
    int* _iptr;
    Vector<int> v;
    int i, j, n, len;
    bool sortRows = (flags & 1) == CV_SORT_EVERY_ROW;
    bool sortDescending = (flags & CV_SORT_DESCENDING) != 0;

    CV_Assert( src.data != dst.data );
    
    if( sortRows )
        n = src.rows, len = src.cols;
    else
    {
        n = src.cols, len = src.rows;
        buf.allocate(len);
        ibuf.allocate(len);
    }
    bptr = (T*)buf;
    _iptr = (int*)ibuf;

    for( i = 0; i < n; i++ )
    {
        T* ptr = bptr;
        int* iptr = _iptr;

        if( sortRows )
        {
            ptr = (T*)(src.data + src.step*i);
            iptr = (int*)(dst.data + dst.step*i);
        }
        else
        {
            for( j = 0; j < len; j++ )
                ptr[j] = ((const T*)(src.data + src.step*j))[i];
        }
        for( j = 0; j < len; j++ )
            iptr[j] = j;
        v.set(iptr, len, false);
        sort( v, LessThanIdx<T>(ptr) );
        if( sortDescending )
            for( j = 0; j < len/2; j++ )
                std::swap(iptr[j], iptr[len-1-j]);
        if( !sortRows )
            for( j = 0; j < len; j++ )
                ((int*)(dst.data + dst.step*j))[i] = iptr[j];
    }
}

typedef void (*SortFunc)(const Mat& src, Mat& dst, int flags);

void sort( const Mat& src, Mat& dst, int flags )
{
    static SortFunc tab[] =
    {
        sort_<uchar>, sort_<schar>, sort_<ushort>, sort_<short>,
        sort_<int>, sort_<float>, sort_<double>, 0
    };
    SortFunc func = tab[src.depth()];
    CV_Assert( src.channels() == 1 && func != 0 );
    dst.create( src.size(), src.type() );
    func( src, dst, flags );
}

void sortIdx( const Mat& src, Mat& dst, int flags )
{
    static SortFunc tab[] =
    {
        sortIdx_<uchar>, sortIdx_<schar>, sortIdx_<ushort>, sortIdx_<short>,
        sortIdx_<int>, sortIdx_<float>, sortIdx_<double>, 0
    };
    SortFunc func = tab[src.depth()];
    CV_Assert( src.channels() == 1 && func != 0 );
    if( dst.data == src.data )
        dst.release();
    dst.create( src.size(), CV_32S );
    func( src, dst, flags );
}

}


CV_IMPL void cvSetIdentity( CvArr* arr, CvScalar value )
{
    cv::Mat m = cv::cvarrToMat(arr);
    cv::setIdentity(m, value);
}


CV_IMPL CvScalar cvTrace( const CvArr* arr )
{
    return cv::trace(cv::cvarrToMat(arr));
}


CV_IMPL void cvTranspose( const CvArr* srcarr, CvArr* dstarr )
{
    cv::Mat src = cv::cvarrToMat(srcarr), dst = cv::cvarrToMat(dstarr);

    CV_Assert( src.rows == dst.cols && src.cols == dst.rows && src.type() == dst.type() );
    transpose( src, dst );
}


CV_IMPL void cvCompleteSymm( CvMat* matrix, int LtoR )
{
    cv::Mat m(matrix);
    cv::completeSymm( m, LtoR != 0 );
}


CV_IMPL void cvCrossProduct( const CvArr* srcAarr, const CvArr* srcBarr, CvArr* dstarr )
{
    cv::Mat srcA = cv::cvarrToMat(srcAarr), dst = cv::cvarrToMat(dstarr);

    CV_Assert( srcA.size() == dst.size() && srcA.type() == dst.type() );
    srcA.cross(cv::cvarrToMat(srcBarr)).copyTo(dst);
}


CV_IMPL void
cvReduce( const CvArr* srcarr, CvArr* dstarr, int dim, int op )
{
    cv::Mat src = cv::cvarrToMat(srcarr), dst = cv::cvarrToMat(dstarr);
    
    if( dim < 0 )
        dim = src.rows > dst.rows ? 0 : src.cols > dst.cols ? 1 : dst.cols == 1;

    if( dim > 1 )
        CV_Error( CV_StsOutOfRange, "The reduced dimensionality index is out of range" );

    if( (dim == 0 && (dst.cols != src.cols || dst.rows != 1)) ||
        (dim == 1 && (dst.rows != src.rows || dst.cols != 1)) )
        CV_Error( CV_StsBadSize, "The output array size is incorrect" );
    
    if( src.channels() != dst.channels() )
        CV_Error( CV_StsUnmatchedFormats, "Input and output arrays must have the same number of channels" );

    cv::reduce(src, dst, dim, op, dst.type());
}


CV_IMPL CvArr*
cvRange( CvArr* arr, double start, double end )
{
    int ok = 0;
    
    CvMat stub, *mat = (CvMat*)arr;
    double delta;
    int type, step;
    double val = start;
    int i, j;
    int rows, cols;
    
    if( !CV_IS_MAT(mat) )
        mat = cvGetMat( mat, &stub);

    rows = mat->rows;
    cols = mat->cols;
    type = CV_MAT_TYPE(mat->type);
    delta = (end-start)/(rows*cols);

    if( CV_IS_MAT_CONT(mat->type) )
    {
        cols *= rows;
        rows = 1;
        step = 1;
    }
    else
        step = mat->step / CV_ELEM_SIZE(type);

    if( type == CV_32SC1 )
    {
        int* idata = mat->data.i;
        int ival = cvRound(val), idelta = cvRound(delta);

        if( fabs(val - ival) < DBL_EPSILON &&
            fabs(delta - idelta) < DBL_EPSILON )
        {
            for( i = 0; i < rows; i++, idata += step )
                for( j = 0; j < cols; j++, ival += idelta )
                    idata[j] = ival;
        }
        else
        {
            for( i = 0; i < rows; i++, idata += step )
                for( j = 0; j < cols; j++, val += delta )
                    idata[j] = cvRound(val);
        }
    }
    else if( type == CV_32FC1 )
    {
        float* fdata = mat->data.fl;
        for( i = 0; i < rows; i++, fdata += step )
            for( j = 0; j < cols; j++, val += delta )
                fdata[j] = (float)val;
    }
    else
        CV_Error( CV_StsUnsupportedFormat, "The function only supports 32sC1 and 32fC1 datatypes" );

    ok = 1;
    return ok ? arr : 0;
}


CV_IMPL void
cvSort( const CvArr* _src, CvArr* _dst, CvArr* _idx, int flags )
{
    cv::Mat src = cv::cvarrToMat(_src), dst, idx;
    
    if( _idx )
    {
        cv::Mat idx0 = cv::cvarrToMat(_idx), idx = idx0;
        CV_Assert( src.size() == idx.size() && idx.type() == CV_32S && src.data != idx.data );
        cv::sortIdx( src, idx, flags );
        CV_Assert( idx0.data == idx.data );
    }

    if( _dst )
    {
        cv::Mat dst0 = cv::cvarrToMat(_dst), dst = dst0;
        CV_Assert( src.size() == dst.size() && src.type() == dst.type() );
        cv::sort( src, dst, flags );
        CV_Assert( dst0.data == dst.data );
    }
}


CV_IMPL int
cvKMeans2( const CvArr* samples_arr, int cluster_count, CvArr* labels_arr,
           CvTermCriteria termcrit, int attempts, CvRNG* _rng,
           int flags, CvArr* centers_arr, double* _compactness )
{
    int best_niters = 0;
    CvMat* best_labels = 0;
    CvMat* centers = 0;
    CvMat* old_centers = 0;
    CvMat* counters = 0;

    double best_compactness = DBL_MAX;
    CvTermCriteria termcrit0;
    CvMat samples_stub, *samples = cvGetMat(samples_arr, &samples_stub);
    CvMat labels_stub, *labels = cvGetMat(labels_arr, &labels_stub), *labels0 = labels;
    CvMat centers_stub, *_centers = 0;
    CvRNG default_rng = CvRNG(-1), *rng = _rng ? _rng : &default_rng;
    CvMat* temp = 0;
    int a, i, j, k, sample_count, dims;
    int ids_delta, iter;
    double max_dist;

    if( cluster_count < 1 )
        CV_Error( CV_StsOutOfRange, "Number of clusters should be positive" );

    if( CV_MAT_DEPTH(samples->type) != CV_32F || CV_MAT_TYPE(labels->type) != CV_32SC1 )
        CV_Error( CV_StsUnsupportedFormat,
        "samples should be floating-point matrix, cluster_idx - integer vector" );

    if( (labels->rows != 1 && (labels->cols != 1 || !CV_IS_MAT_CONT(labels->type))) ||
        labels->rows + labels->cols - 1 != samples->rows )
        CV_Error( CV_StsUnmatchedSizes,
        "cluster_idx should be 1D vector of the same number of elements as samples' number of rows" );

    termcrit = cvCheckTermCriteria( termcrit, 1e-6, 100 );

    termcrit.epsilon *= termcrit.epsilon;
    termcrit0 = termcrit;
    sample_count = samples->rows;

    cluster_count = MIN( cluster_count, sample_count );
    dims = samples->cols*CV_MAT_CN(samples->type);
    ids_delta = labels->step ? labels->step/(int)sizeof(int) : 1;

    best_labels = cvCreateMat( sample_count, 1, CV_32SC1 );
    centers = cvCreateMat( cluster_count, dims, CV_64FC1 );
    old_centers = cvCreateMat( cluster_count, dims, CV_64FC1 );
    counters = cvCreateMat( 1, cluster_count, CV_32SC1 );

    if( centers_arr )
    {
        _centers = cvGetMat( centers_arr, &centers_stub );
        if( _centers->rows != cluster_count || _centers->cols != dims ||
            CV_MAT_CN(_centers->type) != 1 )
            CV_Error( CV_StsBadSize, "The output array of centers should be 1-channel, "
            "have as many rows as the number of clusters and "
            "as many columns as the samples' dimensionality" );
    }

    counters->cols = cluster_count; // cut down counters
    max_dist = termcrit.epsilon*2;

    attempts = MAX( attempts, 1 );

    for( a = 0; a < attempts; a++ )
    {
        // init labels
        if( a > 0 || !(flags & CV_KMEANS_USE_INITIAL_LABELS) )
        {
            for( i = 0; i < sample_count; i++ )
                labels->data.i[i] = cvRandInt(rng) % cluster_count;
        }
        else
        {
            for( i = 0; i < sample_count; i++ )
                if( (unsigned)labels->data.i[i] >= (unsigned)cluster_count )
                    CV_Error( CV_StsOutOfRange, "One of provided labels is out of range" );
        }

        for( iter = 0;; iter++ )
        {
            // compute centers
            cvZero( centers );
            cvZero( counters );

            for( i = 0; i < sample_count; i++ )
            {
                float* s = (float*)(samples->data.ptr + i*samples->step);
                k = labels->data.i[i*ids_delta];
                double* c = (double*)(centers->data.ptr + k*centers->step);
                for( j = 0; j <= dims - 4; j += 4 )
                {
                    double t0 = c[j] + s[j];
                    double t1 = c[j+1] + s[j+1];

                    c[j] = t0;
                    c[j+1] = t1;

                    t0 = c[j+2] + s[j+2];
                    t1 = c[j+3] + s[j+3];

                    c[j+2] = t0;
                    c[j+3] = t1;
                }
                for( ; j < dims; j++ )
                    c[j] += s[j];
                counters->data.i[k]++;
            }

            if( iter > 0 )
                max_dist = 0;

            for( k = 0; k < cluster_count; k++ )
            {
                double* c = (double*)(centers->data.ptr + k*centers->step);
                if( counters->data.i[k] != 0 )
                {
                    double scale = 1./counters->data.i[k];
                    for( j = 0; j < dims; j++ )
                        c[j] *= scale;
                }
                else
                {
                    i = cvRandInt( rng ) % sample_count;
                    float* s = (float*)(samples->data.ptr + i*samples->step);
                    for( j = 0; j < dims; j++ )
                        c[j] = s[j];
                }
                
                if( iter > 0 )
                {
                    double dist = 0;
                    double* c_o = (double*)(old_centers->data.ptr + k*old_centers->step);
                    for( j = 0; j < dims; j++ )
                    {
                        double t = c[j] - c_o[j];
                        dist += t*t;
                    }
                    if( max_dist < dist )
                        max_dist = dist;
                }
            }

            if( max_dist < termcrit.epsilon || iter == termcrit.max_iter )
                break;

            // assign labels
            for( i = 0; i < sample_count; i++ )
            {
                const float* s = (const float*)(samples->data.ptr + i*samples->step);
                int k_best = 0;
                double min_dist = DBL_MAX;

                for( k = 0; k < cluster_count; k++ )
                {
                    const double* c = (const double*)(centers->data.ptr + k*centers->step);
                    double dist = 0;
                    
                    j = 0;
                    for( ; j <= dims - 4; j += 4 )
                    {
                        double t0 = c[j] - s[j];
                        double t1 = c[j+1] - s[j+1];
                        dist += t0*t0 + t1*t1;
                        t0 = c[j+2] - s[j+2];
                        t1 = c[j+3] - s[j+3];
                        dist += t0*t0 + t1*t1;
                    }

                    for( ; j < dims; j++ )
                    {
                        double t = c[j] - s[j];
                        dist += t*t;
                    }

                    if( min_dist > dist )
                    {
                        min_dist = dist;
                        k_best = k;
                    }
                }

                labels->data.i[i*ids_delta] = k_best;
            }

            CV_SWAP( centers, old_centers, temp );
        }

        cvZero( counters );
        for( i = 0; i < sample_count; i++ )
            counters->data.i[labels->data.i[i]]++;

        // ensure that we do not have empty clusters
        for( k = 0; k < cluster_count; k++ )
            if( counters->data.i[k] == 0 )
                for(;;)
                {
                    i = cvRandInt(rng) % sample_count;
                    j = labels->data.i[i];
                    if( counters->data.i[j] > 1 )
                    {
                        labels->data.i[i] = k;
                        counters->data.i[j]--;
                        counters->data.i[k]++;
                        break;
                    }
                }

        if( attempts == 1 )
        {
            if( _centers )
                cvConvert( centers, _centers );
            best_niters = iter;
        }

        if( _compactness || attempts > 1 )
        {
            double compactness = 0;
            for( i = 0; i < sample_count; i++ )
            {
                k = labels->data.i[i];
                const float* s = (const float*)(samples->data.ptr + i*samples->step);
                const double* c = (const double*)(centers->data.ptr + k*centers->step);
                double dist = 0;
                for( j = 0; j < dims; j++ )
                {
                    double t = c[j] - s[j];
                    dist += t*t;
                }
                compactness += dist;
            }
            if( compactness < best_compactness )
            {
                best_compactness = compactness;
                best_niters = iter;
                if( _centers )
                    cvConvert( centers, _centers );
                CV_SWAP( labels, best_labels, temp );
            }
        }
    }

    if( best_labels != labels0 )
        cvCopy( best_labels, labels0 );
    else
        best_labels = labels;

    if( _compactness )
        *_compactness = best_compactness;

    cvReleaseMat( &best_labels );
    cvReleaseMat( &centers );
    cvReleaseMat( &old_centers );
    cvReleaseMat( &counters );

    return best_niters;
}


/* End of file. */
