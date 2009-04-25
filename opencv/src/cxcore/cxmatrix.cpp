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

    if( dst.data == src.data && dst.cols == dst.rows )
    {
        TransposeInplaceFunc func = itab[esz];
        CV_Assert( func != 0 );
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

///////////////////////////// n-dimensional matrices ////////////////////////////

namespace cv
{

//////////////////////////////// MatND ///////////////////////////////////

MatND::MatND(const MatND& m, const Vector<Range>& ranges)
 : flags(MAGIC_VAL), dims(0), refcount(0), data(0), datastart(0), dataend(0)
{
    int i, j, d = m.dims;

    for( i = 0; i < d; i++ )
    {
        Range r = ranges[i];
        CV_Assert( r == Range::all() ||
            (0 <= r.start && r.start < r.end && r.end <= m.dim[i].size) );
    }
    *this = m;
    for( i = 0; i < d; i++ )
    {
        Range r = ranges[i];
        if( r != Range::all() )
        {
            dim[i].size = r.end - r.start;
            data += r.start*dim[i].step;
        }
    }
    
    for( i = 0; i < d; i++ )
    {
        if( dim[i].size != 1 )
            break;
    }

    CV_Assert( dim[d-1].step == elemSize() );
    for( j = d-1; j > i; j-- )
    {
        if( dim[j].step*dim[j].size < dim[j-1].step )
            break;
    }
    flags = (flags & ~CONTINUOUS_FLAG) | (j <= i ? CONTINUOUS_FLAG : 0);
}

void MatND::create(const Vector<int>& _sizes, int _type)
{
    int i, d = (int)_sizes.size();
    _type = CV_MAT_TYPE(_type);
    if( data && d == dims && _type == type() )
    {
        for( i = 0; i < d; i++ )
            if( dim[i].size != _sizes[i] )
                break;
        if( i == d )
            return;
    }
    
    release();
    
    flags = (_type & CV_MAT_TYPE_MASK) | MAGIC_VAL | CONTINUOUS_FLAG;
    size_t total = elemSize();
    int64 total1;
    
    CV_Assert( d > 0 );
    for( i = d-1; i >= 0; i-- )
    {
        int sz = _sizes[i];
        dim[i].size = sz;
        dim[i].step = total;
        total1 = (int64)total*sz;
        CV_Assert( sz > 0 );
        if( total1 != (size_t)total1 )
            CV_Error( CV_StsOutOfRange, "The total matrix size does not fit to \"size_t\" type" );
        total = (size_t)total1;
    }
    total = alignSize(total, (int)sizeof(*refcount));
    data = datastart = (uchar*)fastMalloc(total + (int)sizeof(*refcount));
    dataend = datastart + dim[0].step*dim[0].size;
    refcount = (int*)(data + total);
    *refcount = 1;
    dims = d;
}

void MatND::copyTo( MatND& m ) const
{
    m.create( size(), type() );
    NAryMatNDIterator it((Vector<MatND>() << *this, m));

    for( int i = 0; i < it.nplanes; i++, ++it )
        it.planes[0].copyTo(it.planes[1]); 
}

void MatND::copyTo( MatND& m, const MatND& mask ) const
{
    m.create( size(), type() );
    NAryMatNDIterator it((Vector<MatND>() << *this, m, mask));

    for( int i = 0; i < it.nplanes; i++, ++it )
        it.planes[0].copyTo(it.planes[1], it.planes[2]); 
}

void MatND::convertTo( MatND& m, int rtype, double alpha, double beta ) const
{
    rtype = rtype < 0 ? type() : CV_MAKETYPE(CV_MAT_DEPTH(rtype), channels());
    m.create( size(), rtype );
    NAryMatNDIterator it((Vector<MatND>() << *this, m));

    for( int i = 0; i < it.nplanes; i++, ++it )
        it.planes[0].convertTo(it.planes[1], rtype, alpha, beta);
}

MatND& MatND::operator = (const Scalar& s)
{
    NAryMatNDIterator it((Vector<MatND>() << *this));
    for( int i = 0; i < it.nplanes; i++, ++it )
        it.planes[0] = s;

    return *this;
}

MatND& MatND::setTo(const Scalar& s, const MatND& mask)
{
    NAryMatNDIterator it((Vector<MatND>() << *this, mask));
    for( int i = 0; i < it.nplanes; i++, ++it )
        it.planes[0].setTo(s, it.planes[1]);

    return *this;
}

MatND MatND::reshape(int, const Vector<int>&) const
{
    // TBD
    return MatND();
}

MatND::operator Mat() const
{
    int i, d = dims, d1, rows = 1, cols = dim[d-1].size;

    for( d1 = 0; d1 < d; d1++ )
        if( dim[d1].size > 1 )
            break;

    for( i = d-1; i > d1; i-- )
    {
        int64 cols1 = (int64)cols*dim[i-1].size;
        if( cols1 != (int)cols1 || dim[i].size*dim[i].step != dim[i-1].step )
            break;
        cols = (int)cols1;
    }

    size_t step = Mat::AUTO_STEP;
    if( i > d1 )
    {
        --i;
        step = dim[i].step;
        rows = dim[i].size;
        for( ; i > d1; i-- )
        {
            int64 rows1 = (int64)rows*dim[i-1].size;
            if( rows1 != (int)rows1 || dim[i].size*dim[i].step != dim[i-1].step )
                break;
            rows = (int)rows1;
        }

        if( i > d1 )
            CV_Error( CV_StsBadArg,
            "The nD matrix can not be represented as 2D matrix due "
            "to its layout in memory; you may use (Mat)the_matnd.clone() instead" );
    }

    Mat m(rows, cols, type(), data, step);
    m.datastart = datastart;
    m.dataend = dataend;
    m.refcount = refcount;
    m.addref();
    return m;
}

MatND::operator CvMatND() const
{
    CvMatND mat;
    cvInitMatNDHeader( &mat, 1, &dims, type(), data );
    int i, d = dims;
    mat.dims = d;
    for( i = 0; i < d; i++ )
    {
        mat.dim[i].size = dim[i].size;
        mat.dim[i].step = (int)dim[i].step;
    }
    mat.type |= flags & CONTINUOUS_FLAG;
    return mat;
}

NAryMatNDIterator::NAryMatNDIterator(const Vector<MatND>& _arrays)
{
    init(_arrays);
}

void NAryMatNDIterator::init(const Vector<MatND>& _arrays)
{
    CV_Assert( _arrays.size() > 0 );
    arrays = _arrays;
    int i, j, d1=0, i0 = -1, d = -1, n = (int)_arrays.size();
    size_t esz = 0;

    iterdepth = 0;

    for( i = 0; i < n; i++ )
    {
        if( !arrays[i].data )
            continue;

        const MatND& A = arrays[i];
        if( i0 < 0 )
        {
            i0 = i;
            d = A.dims;
            esz = A.elemSize();
            
            // find the first dimensionality which is different from 1;
            // in any of the arrays the first "d1" steps do not affect the continuity
            for( d1 = 0; d1 < d; d1++ )
                if( A.dim[d1].size > 1 )
                    break;
        }
        else
        {
            CV_Assert( A.dims == d );
            for( j = 0; j < d; j++ )
                CV_Assert( A.dim[j].size == arrays[i0].dim[j].size );
        }

        if( !A.isContinuous() )
        {
            CV_Assert( A.dim[d-1].step == esz );
            for( j = d-1; j > d1; j-- )
                if( A.dim[j].step*A.dim[j].size < A.dim[j-1].step )
                    break;
            iterdepth = std::max(iterdepth, j);
        }
    }

    if( i0 < 0 )
        CV_Error( CV_StsBadArg, "All the input arrays are empty" );

    int total = arrays[i0].dim[d-1].size;
    for( j = d-1; j > iterdepth; j-- )
    {
        int64 total1 = (int64)total*arrays[i0].dim[j-1].size;
        if( total1 != (int)total1 )
            break;
        total = (int)total1;
    }

    iterdepth = j;
    if( iterdepth == d1 )
        iterdepth = 0;

    planes.resize(n);
    for( i = 0; i < n; i++ )
    {
        if( !arrays[i].data )
        {
            planes[i] = Mat();
            continue;
        }
        planes[i] = Mat( 1, total, arrays[i].type(), arrays[i].data );
        planes[i].datastart = arrays[i].datastart;
        planes[i].dataend = arrays[i].dataend;
        planes[i].refcount = arrays[i].refcount;
        planes[i].addref();
    }

    idx = 0;
    nplanes = 1;
    for( j = iterdepth-1; j >= 0; j-- )
        nplanes *= arrays[i0].dim[j].size;
}


NAryMatNDIterator& NAryMatNDIterator::operator ++()
{
    if( idx >= nplanes-1 )
        return *this;
    ++idx;

    for( size_t i = 0; i < arrays.size(); i++ )
    {
        const MatND& A = arrays[i];
        Mat& M = planes[i];
        if( !A.data )
            continue;
        int _idx = idx;
        uchar* data = A.data;
        for( int j = iterdepth-1; j >= 0 && _idx > 0; j-- )
        {
            int szi = A.dim[j].size, t = _idx/szi;
            data += (_idx - t * szi)*A.dim[j].step;
            _idx = t;
        }
        M.data = data;
    }
    
    return *this;
}

NAryMatNDIterator NAryMatNDIterator::operator ++(int)
{
    NAryMatNDIterator it = *this;
    ++*this;
    return it;
}

void add(const MatND& a, const MatND& b, MatND& c, const MatND& mask)
{
    c.create(a.size(), a.type());
    NAryMatNDIterator it((Vector<MatND>() << a, b, c, mask));

    for( int i = 0; i < it.nplanes; i++, ++it )
        add( it.planes[0], it.planes[1], it.planes[2], it.planes[3] ); 
}

void subtract(const MatND& a, const MatND& b, MatND& c, const MatND& mask)
{
    c.create(a.size(), a.type());
    NAryMatNDIterator it((Vector<MatND>() << a, b, c, mask));

    for( int i = 0; i < it.nplanes; i++, ++it )
        subtract( it.planes[0], it.planes[1], it.planes[2], it.planes[3] ); 
}

void add(const MatND& a, const MatND& b, MatND& c)
{
    c.create(a.size(), a.type());
    NAryMatNDIterator it((Vector<MatND>() << a, b, c));

    for( int i = 0; i < it.nplanes; i++, ++it )
        add( it.planes[0], it.planes[1], it.planes[2] ); 
}


void subtract(const MatND& a, const MatND& b, MatND& c)
{
    c.create(a.size(), a.type());
    NAryMatNDIterator it((Vector<MatND>() << a, b, c));

    for( int i = 0; i < it.nplanes; i++, ++it )
        subtract( it.planes[0], it.planes[1], it.planes[2] ); 
}

void add(const MatND& a, const Scalar& s, MatND& c, const MatND& mask)
{
    c.create(a.size(), a.type());
    NAryMatNDIterator it((Vector<MatND>() << a, c, mask));

    for( int i = 0; i < it.nplanes; i++, ++it )
        add( it.planes[0], s, it.planes[1], it.planes[2] ); 
}

void subtract(const Scalar& s, const MatND& a, MatND& c, const MatND& mask)
{
    c.create(a.size(), a.type());
    NAryMatNDIterator it((Vector<MatND>() << a, c, mask));

    for( int i = 0; i < it.nplanes; i++, ++it )
        subtract( s, it.planes[0], it.planes[1], it.planes[2] ); 
}

void multiply(const MatND& a, const MatND& b, MatND& c, double scale)
{
    c.create(a.size(), a.type());
    NAryMatNDIterator it((Vector<MatND>() << a, b, c));

    for( int i = 0; i < it.nplanes; i++, ++it )
        multiply( it.planes[0], it.planes[1], it.planes[2], scale ); 
}

void divide(const MatND& a, const MatND& b, MatND& c, double scale)
{
    c.create(a.size(), a.type());
    NAryMatNDIterator it((Vector<MatND>() << a, b, c));

    for( int i = 0; i < it.nplanes; i++, ++it )
        divide( it.planes[0], it.planes[1], it.planes[2], scale ); 
}

void divide(double scale, const MatND& b, MatND& c)
{
    c.create(b.size(), b.type());
    NAryMatNDIterator it((Vector<MatND>() << b, c));

    for( int i = 0; i < it.nplanes; i++, ++it )
        divide( scale, it.planes[0], it.planes[1] ); 
}

void scaleAdd(const MatND& a, double alpha, const MatND& b, MatND& c)
{
    c.create(a.size(), a.type());
    NAryMatNDIterator it((Vector<MatND>() << a, b, c));

    for( int i = 0; i < it.nplanes; i++, ++it )
        scaleAdd( it.planes[0], alpha, it.planes[1], it.planes[2] ); 
}

void addWeighted(const MatND& a, double alpha, const MatND& b,
                 double beta, double gamma, MatND& c)
{
    c.create(a.size(), a.type());
    NAryMatNDIterator it((Vector<MatND>() << a, b, c));

    for( int i = 0; i < it.nplanes; i++, ++it )
        addWeighted( it.planes[0], alpha, it.planes[1], beta, gamma, it.planes[2] );
}

Scalar sum(const MatND& m)
{
    NAryMatNDIterator it((Vector<MatND>() << m));
    Scalar s;

    for( int i = 0; i < it.nplanes; i++, ++it )
        s += sum(it.planes[0]);
    return s;
}

int countNonZero( const MatND& m )
{
    NAryMatNDIterator it((Vector<MatND>() << m));
    int nz = 0;

    for( int i = 0; i < it.nplanes; i++, ++it )
        nz += countNonZero(it.planes[0]);
    return nz;
}

Scalar mean(const MatND& m)
{
    NAryMatNDIterator it((Vector<MatND>() << m));
    double total = 1;
    for( int i = 0; i < m.dims; i++ )
        total *= m.dim[i].size;
    return sum(m)*(1./total);
}

Scalar mean(const MatND& m, const MatND& mask)
{
    if( !mask.data )
        return mean(m);
    NAryMatNDIterator it((Vector<MatND>() << m, mask));
    double total = 0;
    Scalar s;
    for( int i = 0; i < it.nplanes; i++, ++it )
    {
        s += sum(it.planes[0]);
        total += countNonZero(it.planes[1]);
    }
    return s *= std::max(total, 1.);
}

void meanStdDev(const MatND& m, Scalar& mean, Scalar& stddev, const MatND& mask)
{
    NAryMatNDIterator it((Vector<MatND>() << m, mask));
    double total = 0;
    Scalar s, sq;
    int k, cn = m.channels();

    for( int i = 0; i < it.nplanes; i++, ++it )
    {
        Scalar _mean, _stddev;
        meanStdDev(it.planes[0], _mean, _stddev, it.planes[1]);
        double nz = mask.data ? countNonZero(it.planes[1]) :
            (double)it.planes[0].rows*it.planes[0].cols;
        for( k = 0; k < cn; k++ )
        {
            s[k] += _mean[k]*nz;
            sq[k] += (_stddev[k]*_stddev[k] + _mean[k]*_mean[k])*nz;
        }
        total += nz;
    }

    mean = stddev = Scalar();
    total = 1./std::max(total, 1.);
    for( k = 0; k < cn; k++ )
    {
        mean[k] = s[k]*total;
        stddev[k] = std::sqrt(std::max(sq[k]*total - mean[k]*mean[k], 0.));
    }
}

double norm(const MatND& a, int normType, const MatND& mask)
{
    NAryMatNDIterator it((Vector<MatND>() << a, mask));
    double total = 0;

    for( int i = 0; i < it.nplanes; i++, ++it )
    {
        double n = norm(it.planes[0], normType, it.planes[1]);
        if( normType == NORM_INF )
            total = std::max(total, n);
        else if( normType == NORM_L1 )
            total += n;
        else
            total += n*n;
    }

    return normType != NORM_L2 ? total : std::sqrt(total);
}

double norm(const MatND& a, const MatND& b,
            int normType, const MatND& mask)
{
    bool isRelative = (normType & NORM_RELATIVE) != 0;
    normType &= 7;

    NAryMatNDIterator it((Vector<MatND>() << a, b, mask));
    double num = 0, denom = 0;

    for( int i = 0; i < it.nplanes; i++, ++it )
    {
        double n = norm(it.planes[0], it.planes[1], normType, it.planes[2]);
        double d = !isRelative ? 0 : norm(it.planes[1], normType, it.planes[2]);
        if( normType == NORM_INF )
        {
            num = std::max(num, n);
            denom = std::max(denom, d);
        }
        else if( normType == NORM_L1 )
        {
            num += n;
            denom += d;
        }
        else
        {
            num += n*n;
            denom += d*d;
        }
    }

    if( normType == NORM_L2 )
    {
        num = std::sqrt(num);
        denom = std::sqrt(denom);
    }

    return !isRelative ? num : num/std::max(denom,DBL_EPSILON);
}

void normalize( const MatND& src, MatND& dst, double a, double b,
                int norm_type, int rtype, const MatND& mask )
{
    double scale = 1, shift = 0;
    if( norm_type == CV_MINMAX )
    {
        double smin = 0, smax = 0;
        double dmin = std::min( a, b ), dmax = std::max( a, b );
        minMax( src, &smin, &smax, mask );
        scale = (dmax - dmin)*(smax - smin > DBL_EPSILON ? 1./(smax - smin) : 0);
        shift = dmin - smin*scale;
    }
    else if( norm_type == CV_L2 || norm_type == CV_L1 || norm_type == CV_C )
    {
        scale = norm( src, norm_type, mask );
        scale = scale > DBL_EPSILON ? a/scale : 0.;
        shift = 0;
    }
    else
        CV_Error( CV_StsBadArg, "Unknown/unsupported norm type" );
    
    if( !mask.data )
        src.convertTo( dst, rtype, scale, shift );
    else
    {
        MatND temp;
        src.convertTo( temp, rtype, scale, shift );
        temp.copyTo( dst, mask );
    }
}

void minMax(const MatND& a, double* minVal,
            double* maxVal, const MatND& mask)
{
    NAryMatNDIterator it((Vector<MatND>() << a, mask));
    double minval = DBL_MAX, maxval = -DBL_MAX;

    for( int i = 0; i < it.nplanes; i++, ++it )
    {
        double val0 = 0, val1 = 0;
        minMaxLoc( it.planes[0], &val0, &val1, 0, 0, it.planes[1] );
        minval = std::min(minval, val0);
        maxval = std::max(maxval, val1);
    }

    if( minVal )
        *minVal = minval;
    if( maxVal )
        *maxVal = maxval;
}

void merge(const Vector<MatND>& mv, MatND& dst)
{
    size_t k, n = mv.size();
    CV_Assert( n > 0 );
    Vector<MatND> v(n + 1);
    int total_cn = 0;
    for( k = 0; k < n; k++ )
    {
        total_cn += mv[k].channels();
        v[k] = mv[k];
    }
    dst.create( mv[0].size(), CV_MAKETYPE(mv[0].depth(), total_cn) );
    v[n] = dst;
    NAryMatNDIterator it(v);

    for( int i = 0; i < it.nplanes; i++, ++it )
        merge( Vector<Mat>(&it.planes[0], n), it.planes[n] );
}

void split(const MatND& m, Vector<MatND>& mv)
{
    size_t k, n = m.channels();
    CV_Assert( n > 0 );
    mv.resize(n);
    Vector<MatND> v(n + 1);
    for( k = 0; k < n; k++ )
    {
        mv[k].create( m.size(), CV_MAKETYPE(m.depth(), 1) );
        v[k] = mv[k];
    }
    v[n] = m;
    NAryMatNDIterator it(v);

    for( int i = 0; i < it.nplanes; i++, ++it )
    {
        Vector<Mat> temp(&it.planes[0], n);
        split( it.planes[n], temp );
    }
}

void mixChannels(const Vector<MatND>& src, Vector<MatND>& dst,
                 const Vector<int>& fromTo)
{
    size_t k, m = src.size(), n = dst.size();
    CV_Assert( n > 0 && m > 0 );
    Vector<MatND> v(m + n);
    for( k = 0; k < m; k++ )
        v[k] = src[k];
    for( k = 0; k < n; k++ )
        v[m + k] = dst[k];
    NAryMatNDIterator it(v);

    for( int i = 0; i < it.nplanes; i++, ++it )
    {
        Vector<Mat> tsrc(&it.planes[0], m);
        Vector<Mat> tdst(&it.planes[m], n);
        mixChannels( tsrc, tdst, fromTo );
    }
}

void bitwise_and(const MatND& a, const MatND& b, MatND& c, const MatND& mask)
{
    c.create(a.size(), a.type());
    NAryMatNDIterator it((Vector<MatND>() << a, b, c, mask));

    for( int i = 0; i < it.nplanes; i++, ++it )
        bitwise_and( it.planes[0], it.planes[1], it.planes[2], it.planes[3] ); 
}

void bitwise_or(const MatND& a, const MatND& b, MatND& c, const MatND& mask)
{
    c.create(a.size(), a.type());
    NAryMatNDIterator it((Vector<MatND>() << a, b, c, mask));

    for( int i = 0; i < it.nplanes; i++, ++it )
        bitwise_or( it.planes[0], it.planes[1], it.planes[2], it.planes[3] ); 
}

void bitwise_xor(const MatND& a, const MatND& b, MatND& c, const MatND& mask)
{
    c.create(a.size(), a.type());
    NAryMatNDIterator it((Vector<MatND>() << a, b, c, mask));

    for( int i = 0; i < it.nplanes; i++, ++it )
        bitwise_xor( it.planes[0], it.planes[1], it.planes[2], it.planes[3] ); 
}

void bitwise_and(const MatND& a, const Scalar& s, MatND& c, const MatND& mask)
{
    c.create(a.size(), a.type());
    NAryMatNDIterator it((Vector<MatND>() << a, c, mask));

    for( int i = 0; i < it.nplanes; i++, ++it )
        bitwise_and( it.planes[0], s, it.planes[1], it.planes[2] ); 
}

void bitwise_or(const MatND& a, const Scalar& s, MatND& c, const MatND& mask)
{
    c.create(a.size(), a.type());
    NAryMatNDIterator it((Vector<MatND>() << a, c, mask));

    for( int i = 0; i < it.nplanes; i++, ++it )
        bitwise_or( it.planes[0], s, it.planes[1], it.planes[2] ); 
}

void bitwise_xor(const MatND& a, const Scalar& s, MatND& c, const MatND& mask)
{
    c.create(a.size(), a.type());
    NAryMatNDIterator it((Vector<MatND>() << a, c, mask));

    for( int i = 0; i < it.nplanes; i++, ++it )
        bitwise_xor( it.planes[0], s, it.planes[1], it.planes[2] ); 
}

void bitwise_not(const MatND& a, MatND& c)
{
    c.create(a.size(), a.type());
    NAryMatNDIterator it((Vector<MatND>() << a, c));

    for( int i = 0; i < it.nplanes; i++, ++it )
        bitwise_not( it.planes[0], it.planes[1] ); 
}

void absdiff(const MatND& a, const MatND& b, MatND& c)
{
    c.create(a.size(), a.type());
    NAryMatNDIterator it((Vector<MatND>() << a, b, c));

    for( int i = 0; i < it.nplanes; i++, ++it )
        absdiff( it.planes[0], it.planes[1], it.planes[2] ); 
}

void absdiff(const MatND& a, const Scalar& s, MatND& c)
{
    c.create(a.size(), a.type());
    NAryMatNDIterator it((Vector<MatND>() << a, c));

    for( int i = 0; i < it.nplanes; i++, ++it )
        absdiff( it.planes[0], s, it.planes[1] ); 
}

void inRange(const MatND& src, const MatND& lowerb,
             const MatND& upperb, MatND& dst)
{
    dst.create(src.size(), CV_8UC1);
    NAryMatNDIterator it((Vector<MatND>() << src, lowerb, upperb, dst));

    for( int i = 0; i < it.nplanes; i++, ++it )
        inRange( it.planes[0], it.planes[1], it.planes[2], it.planes[3] ); 
}

void inRange(const MatND& src, const Scalar& lowerb,
             const Scalar& upperb, MatND& dst)
{
    dst.create(src.size(), CV_8UC1);
    NAryMatNDIterator it((Vector<MatND>() << src, dst));

    for( int i = 0; i < it.nplanes; i++, ++it )
        inRange( it.planes[0], lowerb, upperb, it.planes[1] ); 
}

void compare(const MatND& a, const MatND& b, MatND& c, int cmpop)
{
    c.create(a.size(), CV_8UC1);
    NAryMatNDIterator it((Vector<MatND>() << a, b, c));

    for( int i = 0; i < it.nplanes; i++, ++it )
        compare( it.planes[0], it.planes[1], it.planes[2], cmpop ); 
}

void compare(const MatND& a, double s, MatND& c, int cmpop)
{
    c.create(a.size(), CV_8UC1);
    NAryMatNDIterator it((Vector<MatND>() << a, c));

    for( int i = 0; i < it.nplanes; i++, ++it )
        compare( it.planes[0], s, it.planes[1], cmpop ); 
}

void min(const MatND& a, const MatND& b, MatND& c)
{
    c.create(a.size(), a.type());
    NAryMatNDIterator it((Vector<MatND>() << a, b, c));

    for( int i = 0; i < it.nplanes; i++, ++it )
        min( it.planes[0], it.planes[1], it.planes[2] );
}

void min(const MatND& a, double alpha, MatND& c)
{
    c.create(a.size(), a.type());
    NAryMatNDIterator it((Vector<MatND>() << a, c));

    for( int i = 0; i < it.nplanes; i++, ++it )
        min( it.planes[0], alpha, it.planes[1] );
}

void max(const MatND& a, const MatND& b, MatND& c)
{
    c.create(a.size(), a.type());
    NAryMatNDIterator it((Vector<MatND>() << a, b, c));

    for( int i = 0; i < it.nplanes; i++, ++it )
        max( it.planes[0], it.planes[1], it.planes[2] ); 
}

void max(const MatND& a, double alpha, MatND& c)
{
    c.create(a.size(), a.type());
    NAryMatNDIterator it((Vector<MatND>() << a, c));

    for( int i = 0; i < it.nplanes; i++, ++it )
        max( it.planes[0], alpha, it.planes[1] );
}

void sqrt(const MatND& a, MatND& b)
{
    b.create(a.size(), a.type());
    NAryMatNDIterator it((Vector<MatND>() << a, b));

    for( int i = 0; i < it.nplanes; i++, ++it )
        sqrt( it.planes[0], it.planes[1] );
}

void pow(const MatND& a, double power, MatND& b)
{
    b.create(a.size(), a.type());
    NAryMatNDIterator it((Vector<MatND>() << a, b));

    for( int i = 0; i < it.nplanes; i++, ++it )
        pow( it.planes[0], power, it.planes[1] );
}

void exp(const MatND& a, MatND& b)
{
    b.create(a.size(), a.type());
    NAryMatNDIterator it((Vector<MatND>() << a, b));

    for( int i = 0; i < it.nplanes; i++, ++it )
        exp( it.planes[0], it.planes[1] );
}

void log(const MatND& a, MatND& b)
{
    b.create(a.size(), a.type());
    NAryMatNDIterator it((Vector<MatND>() << a, b));

    for( int i = 0; i < it.nplanes; i++, ++it )
        log( it.planes[0], it.planes[1] );
}

bool checkRange(const MatND& a, bool quiet, int*,
                double minVal, double maxVal)
{
    NAryMatNDIterator it((Vector<MatND>() << a));

    for( int i = 0; i < it.nplanes; i++, ++it )
    {
        Point pt;
        if( !checkRange( it.planes[0], quiet, &pt, minVal, maxVal ))
        {
            // todo: set index properly
            return false;
        }
    }
    return true;
}


//////////////////////////////// SparseMat ////////////////////////////////

template<typename T1, typename T2> void
convertData_(const void* _from, void* _to, int cn)
{
    const T1* from = (const T1*)_from;
    T2* to = (T2*)_to;
    if( cn == 1 )
        *to = saturate_cast<T2>(*from);
    else
        for( int i = 0; i < cn; i++ )
            to[i] = saturate_cast<T2>(from[i]);
}

template<typename T1, typename T2> void
convertScaleData_(const void* _from, void* _to, int cn, double alpha, double beta)
{
    const T1* from = (const T1*)_from;
    T2* to = (T2*)_to;
    if( cn == 1 )
        *to = saturate_cast<T2>(*from*alpha + beta);
    else
        for( int i = 0; i < cn; i++ )
            to[i] = saturate_cast<T2>(from[i]*alpha + beta);
}

ConvertData getConvertData(int fromType, int toType)
{
    static ConvertData tab[][8] =
    {{ convertData_<uchar, uchar>, convertData_<uchar, schar>,
      convertData_<uchar, ushort>, convertData_<uchar, short>,
      convertData_<uchar, int>, convertData_<uchar, float>,
      convertData_<uchar, double>, 0 },

    { convertData_<schar, uchar>, convertData_<schar, schar>,
      convertData_<schar, ushort>, convertData_<schar, short>,
      convertData_<schar, int>, convertData_<schar, float>,
      convertData_<schar, double>, 0 },

    { convertData_<ushort, uchar>, convertData_<ushort, schar>,
      convertData_<ushort, ushort>, convertData_<ushort, short>,
      convertData_<ushort, int>, convertData_<ushort, float>,
      convertData_<ushort, double>, 0 },

    { convertData_<short, uchar>, convertData_<short, schar>,
      convertData_<short, ushort>, convertData_<short, short>,
      convertData_<short, int>, convertData_<short, float>,
      convertData_<short, double>, 0 },

    { convertData_<int, uchar>, convertData_<int, schar>,
      convertData_<int, ushort>, convertData_<int, short>,
      convertData_<int, int>, convertData_<int, float>,
      convertData_<int, double>, 0 },

    { convertData_<float, uchar>, convertData_<float, schar>,
      convertData_<float, ushort>, convertData_<float, short>,
      convertData_<float, int>, convertData_<float, float>,
      convertData_<float, double>, 0 },

    { convertData_<double, uchar>, convertData_<double, schar>,
      convertData_<double, ushort>, convertData_<double, short>,
      convertData_<double, int>, convertData_<double, float>,
      convertData_<double, double>, 0 },

    { 0, 0, 0, 0, 0, 0, 0, 0 }};

    ConvertData func = tab[CV_MAT_DEPTH(fromType)][CV_MAT_DEPTH(toType)];
    CV_Assert( func != 0 );
    return func;
}

ConvertScaleData getConvertScaleData(int fromType, int toType)
{
    static ConvertScaleData tab[][8] =
    {{ convertScaleData_<uchar, uchar>, convertScaleData_<uchar, schar>,
      convertScaleData_<uchar, ushort>, convertScaleData_<uchar, short>,
      convertScaleData_<uchar, int>, convertScaleData_<uchar, float>,
      convertScaleData_<uchar, double>, 0 },

    { convertScaleData_<schar, uchar>, convertScaleData_<schar, schar>,
      convertScaleData_<schar, ushort>, convertScaleData_<schar, short>,
      convertScaleData_<schar, int>, convertScaleData_<schar, float>,
      convertScaleData_<schar, double>, 0 },

    { convertScaleData_<ushort, uchar>, convertScaleData_<ushort, schar>,
      convertScaleData_<ushort, ushort>, convertScaleData_<ushort, short>,
      convertScaleData_<ushort, int>, convertScaleData_<ushort, float>,
      convertScaleData_<ushort, double>, 0 },

    { convertScaleData_<short, uchar>, convertScaleData_<short, schar>,
      convertScaleData_<short, ushort>, convertScaleData_<short, short>,
      convertScaleData_<short, int>, convertScaleData_<short, float>,
      convertScaleData_<short, double>, 0 },

    { convertScaleData_<int, uchar>, convertScaleData_<int, schar>,
      convertScaleData_<int, ushort>, convertScaleData_<int, short>,
      convertScaleData_<int, int>, convertScaleData_<int, float>,
      convertScaleData_<int, double>, 0 },

    { convertScaleData_<float, uchar>, convertScaleData_<float, schar>,
      convertScaleData_<float, ushort>, convertScaleData_<float, short>,
      convertScaleData_<float, int>, convertScaleData_<float, float>,
      convertScaleData_<float, double>, 0 },

    { convertScaleData_<double, uchar>, convertScaleData_<double, schar>,
      convertScaleData_<double, ushort>, convertScaleData_<double, short>,
      convertScaleData_<double, int>, convertScaleData_<double, float>,
      convertScaleData_<double, double>, 0 },

    { 0, 0, 0, 0, 0, 0, 0, 0 }};

    ConvertScaleData func = tab[CV_MAT_DEPTH(fromType)][CV_MAT_DEPTH(toType)];
    CV_Assert( func != 0 );
    return func;
}

enum { HASH_SIZE0 = 8 };

static inline void copyElem(const uchar* from, uchar* to, size_t elemSize)
{
    size_t i;
    for( i = 0; i <= elemSize - sizeof(int); i += sizeof(int) )
        *(int*)(to + i) = *(const int*)(from + i);
    for( ; i < elemSize; i++ )
        to[i] = from[i];
}

static inline bool isZeroElem(const uchar* data, size_t elemSize)
{
    size_t i;
    for( i = 0; i <= elemSize - sizeof(int); i += sizeof(int) )
        if( *(int*)(data + i) != 0 )
            return false;
    for( ; i < elemSize; i++ )
        if( data[i] != 0 )
            return false;
    return true;
}

SparseMat::Hdr::Hdr( const Vector<int>& _sizes, int _type )
{
    refcount = 1;

    dims = (int)_sizes.size();
    valueOffset = alignSize(sizeof(SparseMat::Node) +
        sizeof(int)*(dims - CV_MAX_DIM), CV_ELEM_SIZE1(_type));
    nodeSize = alignSize(valueOffset +
        CV_ELEM_SIZE(_type), (int)sizeof(size_t));
   
    int i;
    for( i = 0; i < dims; i++ )
        size[i] = _sizes[i];
    for( ; i < CV_MAX_DIM; i++ )
        size[i] = 0;
    clear();
}

void SparseMat::Hdr::clear()
{
    hashtab.clear();
    hashtab.resize(HASH_SIZE0);
    pool.clear();
    nodeCount = freeList = 0;
}


SparseMat::SparseMat(const Mat& m, bool try1d)
: flags(MAGIC_VAL), hdr(0)
{
    bool is1d = try1d && m.cols == 1;
    
    if( is1d )
    {
        int i, M = m.rows;
        const uchar* data = m.data;
        size_t step =  m.step, esz = m.elemSize();
        create( (Vector<int>() << M), m.type() );
        for( i = 0; i < M; i++ )
        {
            const uchar* from = data + step*i;
            if( isZeroElem(from, esz) )
                continue;
            uchar* to = newNode(&i, hash(i));
            copyElem(from, to, esz);
        }
    }
    else
    {
        int i, j, M = m.rows, N = m.cols;
        const uchar* data = m.data;
        size_t step =  m.step, esz = m.elemSize();
        create( (Vector<int>() << M, N), m.type() );
        for( i = 0; i < M; i++ )
        {
            for( j = 0; j < N; j++ )
            {
                const uchar* from = data + step*i + esz*j;
                if( isZeroElem(from, esz) )
                    continue;
                int idx[] = {i, j};
                uchar* to = newNode(idx, hash(i, j));
                copyElem(from, to, esz);
            }
        }
    }
}

SparseMat::SparseMat(const MatND& m)
: flags(MAGIC_VAL), hdr(0)
{
    create( m.size(), m.type() );

    int i, idx[CV_MAX_DIM] = {0}, d = m.dims, lastSize = m.dim[d - 1].size;
    size_t esz = m.elemSize();
    uchar* ptr = m.data;

    for(;;)
    {
        for( i = 0; i < lastSize; i++, ptr += esz )
        {
            if( isZeroElem(ptr, esz) )
                continue;
            idx[d-1] = i;
            uchar* to = newNode(idx, hash(idx));
            copyElem( ptr, to, esz );
        }
        
        for( i = d - 2; i >= 0; i-- )
        {
            ptr += m.dim[i].step - m.dim[i+1].size*m.dim[i+1].step;
            if( ++idx[i] < m.dim[i].size )
                break;
            idx[i] = 0;
        }
        if( i < 0 )
            break;
    }
}
                
SparseMat::SparseMat(const CvSparseMat* m)
: flags(MAGIC_VAL), hdr(0)
{
    CV_Assert(m);
    create( Vector<int>((int*)m->size, m->dims), m->type );

    CvSparseMatIterator it;
    CvSparseNode* n = cvInitSparseMatIterator(m, &it);
    size_t esz = elemSize();

    for( ; n != 0; n = cvGetNextSparseNode(&it) )
    {
        const int* idx = CV_NODE_IDX(m, n);
        uchar* to = newNode(idx, hash(idx));
        copyElem((const uchar*)CV_NODE_VAL(m, n), to, esz);
    }
}

void SparseMat::create(const Vector<int>& _sizes, int _type)
{
    int i, d = (int)_sizes.size();
    CV_Assert( 0 < d && d <= CV_MAX_DIM );
    for( i = 0; i < d; i++ )
        CV_Assert( _sizes[i] > 0 );
    _type = CV_MAT_TYPE(_type);
    if( hdr && _type == type() && hdr->dims == d && hdr->refcount == 1 )
    {
        for( i = 0; i < d; i++ )
            if( _sizes[i] != hdr->size[i] )
                break;
        if( i == d )
        {
            clear();
            return;
        }
    }
    release();
    flags = MAGIC_VAL | _type;
    hdr = new Hdr(_sizes, _type);
}

void SparseMat::copyTo( SparseMat& m ) const
{
    if( this == &m )
        return;
    if( !hdr )
    {
        m.release();
        return;
    }
    m.create( size(), type() );
    SparseMatConstIterator from = begin();
    size_t i, N = hdr->nodeCount, esz = elemSize();

    for( i = 0; i < N; i++, ++from )
    {
        const Node* n = from.node();
        uchar* to = m.newNode(n->idx, n->hashval);
        copyElem( from.ptr, to, esz );
    }
}

void SparseMat::copyTo( Mat& m ) const
{
    CV_Assert( hdr && hdr->dims <= 2 );
    m.create( hdr->size[0], hdr->dims == 2 ? hdr->size[1] : 1, type() );
    m = Scalar(0);

    SparseMatConstIterator from = begin();
    size_t i, N = hdr->nodeCount, esz = elemSize();

    if( hdr->dims == 2 )
    {
        for( i = 0; i < N; i++, ++from )
        {
            const Node* n = from.node();
            uchar* to = m.data + m.step*n->idx[0] + esz*n->idx[1];
            copyElem( from.ptr, to, esz );
        }
    }
    else
    {
        for( i = 0; i < N; i++, ++from )
        {
            const Node* n = from.node();
            uchar* to = m.data + esz*n->idx[0];
            copyElem( from.ptr, to, esz );
        }
    }
}

void SparseMat::copyTo( MatND& m ) const
{
    CV_Assert( hdr );
    m.create( size(), type() );
    m = Scalar(0);

    SparseMatConstIterator from = begin();
    size_t i, N = hdr->nodeCount, esz = elemSize();

    for( i = 0; i < N; i++, ++from )
    {
        const Node* n = from.node();
        copyElem( from.ptr, m.ptr(n->idx), esz);
    }
}


void SparseMat::convertTo( SparseMat& m, int rtype, double alpha ) const
{
    int cn = channels();
    if( rtype < 0 )
        rtype = type();
    rtype = CV_MAKETYPE(rtype, cn);
    if( this == &m && rtype != type()  )
    {
        SparseMat temp;
        convertTo(temp, rtype, alpha);
        m = temp;
        return;
    }
    
    CV_Assert(hdr != 0);
    m.create( size(), rtype );
    
    SparseMatConstIterator from = begin();
    size_t i, N = hdr->nodeCount;

    if( alpha == 1 )
    {
        ConvertData cvtfunc = getConvertData(type(), rtype);
        for( i = 0; i < N; i++, ++from )
        {
            const Node* n = from.node();
            uchar* to = m.newNode(n->idx, n->hashval);
            cvtfunc( from.ptr, to, cn ); 
        }
    }
    else
    {
        ConvertScaleData cvtfunc = getConvertScaleData(type(), rtype);
        for( i = 0; i < N; i++, ++from )
        {
            const Node* n = from.node();
            uchar* to = m.newNode(n->idx, n->hashval);
            cvtfunc( from.ptr, to, cn, alpha, 0 ); 
        }
    }
}


void SparseMat::convertTo( Mat& m, int rtype, double alpha, double beta ) const
{
    int cn = channels();
    if( rtype < 0 )
        rtype = type();
    rtype = CV_MAKETYPE(rtype, cn);
    
    CV_Assert( hdr && hdr->dims <= 2 );
    m.create( hdr->size[0], hdr->dims == 2 ? hdr->size[1] : 1, type() );
    m = Scalar(beta);

    SparseMatConstIterator from = begin();
    size_t i, N = hdr->nodeCount, esz = CV_ELEM_SIZE(rtype);

    if( alpha == 1 && beta == 0 )
    {
        ConvertData cvtfunc = getConvertData(type(), rtype);

        if( hdr->dims == 2 )
        {
            for( i = 0; i < N; i++, ++from )
            {
                const Node* n = from.node();
                uchar* to = m.data + m.step*n->idx[0] + esz*n->idx[1];
                cvtfunc( from.ptr, to, cn );
            }
        }
        else
        {
            for( i = 0; i < N; i++, ++from )
            {
                const Node* n = from.node();
                uchar* to = m.data + esz*n->idx[0];
                cvtfunc( from.ptr, to, cn );
            }
        }
    }
    else
    {
        ConvertScaleData cvtfunc = getConvertScaleData(type(), rtype);

        if( hdr->dims == 2 )
        {
            for( i = 0; i < N; i++, ++from )
            {
                const Node* n = from.node();
                uchar* to = m.data + m.step*n->idx[0] + esz*n->idx[1];
                cvtfunc( from.ptr, to, cn, alpha, beta );
            }
        }
        else
        {
            for( i = 0; i < N; i++, ++from )
            {
                const Node* n = from.node();
                uchar* to = m.data + esz*n->idx[0];
                cvtfunc( from.ptr, to, cn, alpha, beta );
            }
        }
    }
}

void SparseMat::convertTo( MatND& m, int rtype, double alpha, double beta ) const
{
    int cn = channels();
    if( rtype < 0 )
        rtype = type();
    rtype = CV_MAKETYPE(rtype, cn);
    
    CV_Assert( hdr );
    m.create( size(), rtype );
    m = Scalar(beta);

    SparseMatConstIterator from = begin();
    size_t i, N = hdr->nodeCount;

    if( alpha == 1 && beta == 0 )
    {
        ConvertData cvtfunc = getConvertData(type(), rtype);
        for( i = 0; i < N; i++, ++from )
        {
            const Node* n = from.node();
            uchar* to = m.ptr(n->idx);
            cvtfunc( from.ptr, to, cn );
        }
    }
    else
    {
        ConvertScaleData cvtfunc = getConvertScaleData(type(), rtype);
        for( i = 0; i < N; i++, ++from )
        {
            const Node* n = from.node();
            uchar* to = m.ptr(n->idx);
            cvtfunc( from.ptr, to, cn, alpha, beta );
        }
    }
}

void SparseMat::clear()
{
    if( hdr )
        hdr->clear();
}

SparseMat::operator CvSparseMat*() const
{
    if( !hdr )
        return 0;
    CvSparseMat* m = cvCreateSparseMat(hdr->dims, hdr->size, type());

    SparseMatConstIterator from = begin();
    size_t i, N = hdr->nodeCount, esz = elemSize();

    for( i = 0; i < N; i++, ++from )
    {
        const Node* n = from.node();
        uchar* to = cvPtrND(m, n->idx, 0, -1, 0);
        copyElem(from.ptr, to, esz);
    }
    return m;
}

uchar* SparseMat::ptr(int i0, int i1, bool createMissing, size_t* hashval)
{
    CV_Assert( hdr && hdr->dims == 2 );
    size_t h;
    if( hashval )
    {
        if( *hashval )
            h = *hashval;
        else
            *hashval = h = hash(i0, i1);
    }
    else
        h = hash(i0, i1);
    size_t hidx = h & (hdr->hashtab.size() - 1), nidx = hdr->hashtab[hidx];
    uchar* pool = &hdr->pool[0];
    while( nidx != 0 )
    {
        Node* elem = (Node*)(pool + nidx);
        if( elem->hashval == h && elem->idx[0] == i0 && elem->idx[1] == i1 )
            return value(elem);
        nidx = elem->next;
    }

    if( createMissing )
    {
        int idx[] = { i0, i1 };
        return newNode( idx, h );
    }
    return 0;
}

uchar* SparseMat::ptr(int i0, int i1, int i2, bool createMissing, size_t* hashval)
{
    CV_Assert( hdr && hdr->dims == 3 );
    size_t h;
    if( hashval )
    {
        if( *hashval )
            h = *hashval;
        else
            *hashval = h = hash(i0, i1, i2);
    }
    else
        h = hash(i0, i1, i2);
    size_t hidx = h & (hdr->hashtab.size() - 1), nidx = hdr->hashtab[hidx];
    uchar* pool = &hdr->pool[0];
    while( nidx != 0 )
    {
        Node* elem = (Node*)(pool + nidx);
        if( elem->hashval == h && elem->idx[0] == i0 &&
            elem->idx[1] == i1 && elem->idx[2] == i2 )
            return value(elem);
        nidx = elem->next;
    }

    if( createMissing )
    {
        int idx[] = { i0, i1, i2 };
        return newNode( idx, h );
    }
    return 0;
}

uchar* SparseMat::ptr(const int* idx, bool createMissing, size_t* hashval)
{
    CV_Assert( hdr );
    int i, d = hdr->dims;
    size_t h;
    if( hashval )
    {
        if( *hashval )
            h = *hashval;
        else
            *hashval = h = hash(idx);
    }
    else
        h = hash(idx);
    size_t hidx = h & (hdr->hashtab.size() - 1), nidx = hdr->hashtab[hidx];
    uchar* pool = &hdr->pool[0];
    while( nidx != 0 )
    {
        Node* elem = (Node*)(pool + nidx);
        if( elem->hashval == h )
        {
            for( i = 0; i < d; i++ )
                if( elem->idx[i] != idx[i] )
                    break;
            if( i == d )
                return value(elem);
        }
        nidx = elem->next;
    }

    return createMissing ? newNode(idx, h) : 0;
}

void SparseMat::erase(int i0, int i1, size_t* hashval)
{
    CV_Assert( hdr && hdr->dims == 2 );
    size_t h;
    if( hashval )
    {
        if( *hashval )
            h = *hashval;
        else
            *hashval = h = hash(i0, i1);
    }
    else
        h = hash(i0, i1);
    size_t hidx = h & (hdr->hashtab.size() - 1), nidx = hdr->hashtab[hidx], previdx=0;
    uchar* pool = &hdr->pool[0];
    while( nidx != 0 )
    {
        Node* elem = (Node*)(pool + nidx);
        if( elem->hashval == h && elem->idx[0] == i0 && elem->idx[1] == i1 )
            break;
        previdx = nidx;
        nidx = elem->next;
    }

    if( nidx )
        removeNode(hidx, nidx, previdx);
}

void SparseMat::erase(int i0, int i1, int i2, size_t* hashval)
{
    CV_Assert( hdr && hdr->dims == 3 );
    size_t h;
    if( hashval )
    {
        if( *hashval )
            h = *hashval;
        else
            *hashval = h = hash(i0, i1, i2);
    }
    else
        h = hash(i0, i1, i2);
    size_t hidx = h & (hdr->hashtab.size() - 1), nidx = hdr->hashtab[hidx], previdx=0;
    uchar* pool = &hdr->pool[0];
    while( nidx != 0 )
    {
        Node* elem = (Node*)(pool + nidx);
        if( elem->hashval == h && elem->idx[0] == i0 &&
            elem->idx[1] == i1 && elem->idx[2] == i2 )
            break;
        previdx = nidx;
        nidx = elem->next;
    }

    if( nidx )
        removeNode(hidx, nidx, previdx);
}

void SparseMat::erase(const int* idx, size_t* hashval)
{
    CV_Assert( hdr );
    int i, d = hdr->dims;
    size_t h;
    if( hashval )
    {
        if( *hashval )
            h = *hashval;
        else
            *hashval = h = hash(idx);
    }
    else
        h = hash(idx);
    size_t hidx = h & (hdr->hashtab.size() - 1), nidx = hdr->hashtab[hidx], previdx=0;
    uchar* pool = &hdr->pool[0];
    while( nidx != 0 )
    {
        Node* elem = (Node*)(pool + nidx);
        if( elem->hashval == h )
        {
            for( i = 0; i < d; i++ )
                if( elem->idx[i] != idx[i] )
                    break;
            if( i == d )
                break;
        }
        previdx = nidx;
        nidx = elem->next;
    }

    if( nidx )
        removeNode(hidx, nidx, previdx);
}

void SparseMat::resizeHashTab(size_t newsize)
{
    size_t i, hsize = hdr->hashtab.size();
    Vector<size_t> newh(newsize, (size_t)0);
    uchar* pool = &hdr->pool[0];
    for( i = 0; i < hsize; i++ )
    {
        size_t nidx = hdr->hashtab[i];
        while( nidx )
        {
            Node* elem = (Node*)(pool + nidx);
            size_t next = elem->next;
            size_t newhidx = elem->hashval & (newsize - 1);
            elem->next = newh[newhidx];
            newh[newhidx] = nidx;
            nidx = next;
        }
    }
    hdr->hashtab = newh;
}

uchar* SparseMat::newNode(const int* idx, size_t hashval)
{
    const int HASH_MAX_FILL_FACTOR=3;
    assert(hdr);
    size_t hsize = hdr->hashtab.size();
    if( ++hdr->nodeCount > hsize*HASH_MAX_FILL_FACTOR )
    {
        resizeHashTab(std::max(hsize*2, (size_t)8));
        hsize = hdr->hashtab.size();
    }
    
    if( !hdr->freeList )
    {
        size_t i, nsz = hdr->nodeSize, psize = hdr->pool.size(),
            newpsize = std::max(psize*2, 8*nsz);
        hdr->pool.resize(newpsize);
        uchar* pool = &hdr->pool[0];
        hdr->freeList = std::max(psize, nsz);
        for( i = hdr->freeList; i < newpsize - nsz; i += nsz )
            ((Node*)(pool + i))->next = i + nsz;
        ((Node*)(pool + i))->next = 0;
    }
    size_t nidx = hdr->freeList;
    Node* elem = (Node*)&hdr->pool[nidx];
    hdr->freeList = elem->next;
    elem->hashval = hashval;
    size_t hidx = hashval & (hsize - 1);
    elem->next = hdr->hashtab[hidx];
    hdr->hashtab[hidx] = nidx;

    size_t i, d = hdr->dims;
    for( i = 0; i < d; i++ )
        elem->idx[i] = idx[i];
    d = elemSize();
    uchar* p = value(elem);
    for( i = 0; i <= d - sizeof(int); i += sizeof(int) )
        *(int*)(p + i) = 0;
    for( ; i < d; i++ )
        p[i] = 0;
    
    return p;
}


void SparseMat::removeNode(size_t hidx, size_t nidx, size_t previdx)
{
    Node* n = node(nidx);
    if( previdx )
    {
        Node* prev = node(previdx);
        prev->next = n->next;
    }
    else
        hdr->hashtab[hidx] = n->next;
    n->next = hdr->freeList;
    hdr->freeList = nidx;
    --hdr->nodeCount;
}


SparseMatConstIterator::SparseMatConstIterator(const SparseMat* _m)
: m((SparseMat*)_m), hashidx(0), ptr(0)
{
    if(!_m || !_m->hdr)
        return;
    SparseMat::Hdr& hdr = *m->hdr;
    const Vector<size_t>& htab = hdr.hashtab;
    size_t i, hsize = htab.size();
    for( i = 0; i < hsize; i++ )
    {
        size_t nidx = htab[i];
        if( nidx )
        {
            hashidx = i;
            ptr = &hdr.pool[nidx] + hdr.valueOffset;
            return;
        }
    }
}

SparseMatConstIterator& SparseMatConstIterator::operator ++()
{
    if( !ptr || !m || !m->hdr )
        return *this;
    SparseMat::Hdr& hdr = *m->hdr;
    size_t next = ((const SparseMat::Node*)(ptr - hdr.valueOffset))->next;
    if( next )
    {
        ptr = &hdr.pool[next] + hdr.valueOffset;
        return *this;
    }
    size_t i = hashidx + 1, sz = hdr.hashtab.size();
    for( ; i < sz; i++ )
    {
        size_t nidx = hdr.hashtab[i];
        if( nidx )
        {
            hashidx = i;
            ptr = &hdr.pool[nidx] + hdr.valueOffset;
            return *this;
        }
    }
    ptr = 0;
    return *this;
}

}

/* End of file. */
