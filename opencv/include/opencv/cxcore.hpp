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

#ifndef _CXCORE_HPP_
#define _CXCORE_HPP_

#include "cxmisc.h"

#ifdef __cplusplus

#ifndef SKIP_INCLUDES
#include <algorithm>
#include <complex>
#include <map>
#include <new>
#include <string>
#include <vector>
#endif // SKIP_INCLUDES

namespace cv {

template<typename _Tp> struct CV_EXPORTS Size_;
template<typename _Tp> struct CV_EXPORTS Point_;
template<typename _Tp> struct CV_EXPORTS Rect_;

typedef std::string String;

struct CV_EXPORTS Exception
{
    Exception() { code = 0; line = 0; }
    Exception(int _code, const String& _err, const String& _func, const String& _file, int _line)
        : code(_code), err(_err), func(_func), file(_file), line(_line) {}
    Exception(const Exception& exc)
        : code(exc.code), err(exc.err), func(exc.func), file(exc.file), line(exc.line) {}
    Exception& operator = (const Exception& exc)
    {
        if( this != &exc )
        {
            code = exc.code; err = exc.err; func = exc.func; file = exc.file; line = exc.line;
        }
        return *this;
    }

    int code;
    String err;
    String func;
    String file;
    int line;
};

CV_EXPORTS String format( const char* fmt, ... );
CV_EXPORTS void error( const Exception& exc );

#ifdef __GNUC__
#define CV_Error( code, msg ) cv::error( cv::Exception(code, msg, __func__, __FILE__, __LINE__) )
#define CV_Error_( code, args ) cv::error( cv::Exception(code, cv::format args, __func__, __FILE__, __LINE__) )
#define CV_Assert( expr ) { if(!(expr)) cv::error( cv::Exception(CV_StsAssert, #expr, __func__, __FILE__, __LINE__) ); }
#else
#define CV_Error( code, msg ) cv::error( cv::Exception(code, msg, "", __FILE__, __LINE__) )
#define CV_Error_( code, args ) cv::error( cv::Exception(code, cv::format args, "", __FILE__, __LINE__) )
#define CV_Assert( expr ) { if(!(expr)) cv::error( cv::Exception(CV_StsAssert, #expr, "", __FILE__, __LINE__) ); }
#endif

CV_EXPORTS void setNumThreads(int);
CV_EXPORTS int getNumThreads();
CV_EXPORTS int getThreadNum();

CV_EXPORTS int64 getTickCount();
CV_EXPORTS double getTickFrequency();

CV_EXPORTS void* fastMalloc(size_t);
CV_EXPORTS void fastFree(void* ptr);

template<typename _Tp> static inline _Tp* fastMalloc_(size_t n)
{
    _Tp* ptr = (_Tp*)fastMalloc(n*sizeof(ptr[0]));
    ::new(ptr) _Tp[n];
    return ptr;
}

template<typename _Tp> static inline void fastFree_(_Tp* ptr, size_t n)
{
    for( size_t i = 0; i < n; i++ ) (ptr+i)->~_Tp();
    fastFree(ptr);
}

template<typename _Tp> static inline _Tp* alignPtr(_Tp* ptr, int n=(int)sizeof(_Tp))
{
    return (_Tp*)(((size_t)ptr + n-1) & -n);
}

static inline size_t alignSize(size_t sz, int n)
{
    return (sz + n-1) & -n;
}

CV_EXPORTS void setUseOptimized(bool);
CV_EXPORTS bool useOptimized();

template<typename _Tp> class CV_EXPORTS Allocator
{
public: 
    typedef _Tp value_type;
    typedef value_type* pointer;
    typedef const value_type* const_pointer;
    typedef value_type& reference;
    typedef const value_type& const_reference;
    typedef size_t size_type;
    typedef ptrdiff_t difference_type;
    template<typename U> struct rebind { typedef Allocator<U> other; };

public : 
    explicit Allocator() {}
    ~Allocator() {}
    explicit Allocator(Allocator const&) {}
    template<typename U>
    explicit Allocator(Allocator<U> const&) {}

    // address
    pointer address(reference r) { return &r; }
    const_pointer address(const_reference r) { return &r; }

    pointer allocate(size_type count, const void* =0)
    { return reinterpret_cast<pointer>(fastMalloc(count * sizeof (_Tp))); }

    void deallocate(pointer p, size_type) {fastFree(p); }

    size_type max_size() const
    { return max(static_cast<_Tp>(-1)/sizeof(_Tp), 1); }

    void construct(pointer p, const _Tp& v) { new(static_cast<void*>(p)) _Tp(v); }
    void destroy(pointer p) { p->~_Tp(); }
};

/////////////////////// Vec_ (used as element of multi-channel images ///////////////////// 

template<typename _Tp> struct CV_EXPORTS DataDepth { enum { value = -1, fmt=(int)'\0' }; };

template<> struct DataDepth<uchar> { enum { value = CV_8U, fmt=(int)'u' }; };
template<> struct DataDepth<schar> { enum { value = CV_8S, fmt=(int)'c' }; };
template<> struct DataDepth<ushort> { enum { value = CV_16U, fmt=(int)'w' }; };
template<> struct DataDepth<short> { enum { value = CV_16S, fmt=(int)'s' }; };
template<> struct DataDepth<int> { enum { value = CV_32S, fmt=(int)'i' }; };
template<> struct DataDepth<float> { enum { value = CV_32F, fmt=(int)'f' }; };
template<> struct DataDepth<double> { enum { value = CV_64F, fmt=(int)'d' }; };
template<typename _Tp> struct DataDepth<_Tp*> { enum { value = CV_USRTYPE1, fmt=(int)'r' }; };

template<typename _Tp, int cn> struct CV_EXPORTS Vec_
{
    typedef _Tp value_type;
    enum { depth = DataDepth<_Tp>::value, channels = cn, type = CV_MAKETYPE(depth, channels) };
    
    Vec_();
    Vec_(_Tp v0);
    Vec_(_Tp v0, _Tp v1);
    Vec_(_Tp v0, _Tp v1, _Tp v2);
    Vec_(_Tp v0, _Tp v1, _Tp v2, _Tp v3);
    Vec_(const Vec_<_Tp, cn>& v);
    static Vec_ all(_Tp alpha);
    _Tp dot(const Vec_& v) const;
    double ddot(const Vec_& v) const;
    Vec_ cross(const Vec_& v) const;
    template<typename T2> operator Vec_<T2, cn>() const;
    operator CvScalar() const;
    _Tp operator [](int i) const;
    _Tp& operator[](int i);

    _Tp val[cn];
};

typedef Vec_<uchar, 2> Vec2b;
typedef Vec_<uchar, 3> Vec3b;
typedef Vec_<uchar, 4> Vec4b;

typedef Vec_<short, 2> Vec2s;
typedef Vec_<short, 3> Vec3s;
typedef Vec_<short, 4> Vec4s;

typedef Vec_<int, 2> Vec2i;
typedef Vec_<int, 3> Vec3i;
typedef Vec_<int, 4> Vec4i;

typedef Vec_<float, 2> Vec2f;
typedef Vec_<float, 3> Vec3f;
typedef Vec_<float, 4> Vec4f;

typedef Vec_<double, 2> Vec2d;
typedef Vec_<double, 3> Vec3d;
typedef Vec_<double, 4> Vec4d;

//////////////////////////////// Complex //////////////////////////////

template<typename _Tp> struct CV_EXPORTS Complex
{
    Complex();
    Complex( _Tp _re, _Tp _im=0 );
    template<typename T2> operator Complex<T2>() const;
    Complex conj() const;

    _Tp re, im;
};

typedef Complex<float> Complexf;
typedef Complex<double> Complexd;

//////////////////////////////// Point_ ////////////////////////////////

template<typename _Tp> struct CV_EXPORTS Point_
{
    typedef _Tp value_type;
    
    Point_();
    Point_(_Tp _x, _Tp _y);
    Point_(const Point_& pt);
    Point_(const CvPoint& pt);
    Point_(const CvPoint2D32f& pt);
    Point_(const Size_<_Tp>& sz);
    Point_& operator = (const Point_& pt);
    operator Point_<int>() const;
    operator Point_<float>() const;
    operator Point_<double>() const;
    operator CvPoint() const;
    operator CvPoint2D32f() const;

    _Tp dot(const Point_& pt) const;
    double ddot(const Point_& pt) const;
    bool inside(const Rect_<_Tp>& r) const;
    
    _Tp x, y;
};

template<typename _Tp> struct CV_EXPORTS Point3_
{
    typedef _Tp value_type;
    
    Point3_();
    Point3_(_Tp _x, _Tp _y, _Tp _z);
    Point3_(const Point3_& pt);
    Point3_(const CvPoint3D32f& pt);
    Point3_(const Vec_<_Tp, 3>& t);
    Point3_& operator = (const Point3_& pt);
    Point3_& operator += (const Point3_& pt);
    Point3_& operator -= (const Point3_& pt);
    operator Point3_<int>() const;
    operator Point3_<float>() const;
    operator Point3_<double>() const;
    operator CvPoint3D32f() const;

    _Tp dot(const Point3_& pt) const;
    double ddot(const Point3_& pt) const;
    
    _Tp x, y, z;
};

//////////////////////////////// Size_ ////////////////////////////////

template<typename _Tp> struct CV_EXPORTS Size_
{
    typedef _Tp value_type;
    
    Size_();
    Size_(_Tp _width, _Tp _height);
    Size_(const Size_& sz);
    Size_(const CvSize& sz);
    Size_(const Point_<_Tp>& pt);
    Size_& operator = (const Size_& sz);
    _Tp area() const;

    operator Size_<int>() const;
    operator Size_<float>() const;
    operator Size_<double>() const;
    operator CvSize() const;

    _Tp width, height;
};

//////////////////////////////// Rect_ ////////////////////////////////

template<typename _Tp> struct CV_EXPORTS Rect_
{
    typedef _Tp value_type;
    
    Rect_();
    Rect_(_Tp _x, _Tp _y, _Tp _width, _Tp _height);
    Rect_(const Rect_& r);
    Rect_(const CvRect& r);
    Rect_(const Point_<_Tp>& org, const Size_<_Tp>& sz);
    Rect_(const Point_<_Tp>& pt1, const Point_<_Tp>& pt2);
    Rect_& operator = ( const Rect_& r );
    Point_<_Tp> tl() const;
    Point_<_Tp> br() const;
    
    Size_<_Tp> size() const;
    _Tp area() const;

    operator Rect_<int>() const;
    operator Rect_<float>() const;
    operator Rect_<double>() const;
    operator CvRect() const;

    bool contains(const Point_<_Tp>& pt) const;

    _Tp x, y, width, height;
};

typedef Point_<int> Point2i;
typedef Point2i Point;
typedef Size_<int> Size2i;
typedef Size2i Size;
typedef Rect_<int> Rect;
typedef Point_<float> Point2f;
typedef Point_<double> Point2d;
typedef Size_<float> Size2f;
typedef Point3_<int> Point3i;
typedef Point3_<float> Point3f;
typedef Point3_<double> Point3d;

struct CV_EXPORTS RotatedRect
{
    RotatedRect();
    RotatedRect(const Point2f& _center, const Size2f& _size, float _angle);
    Point2f center;
    Size2f size;
    float angle;
};

//////////////////////////////// Scalar_ ///////////////////////////////

template<typename _Tp> struct CV_EXPORTS Scalar_ : Vec_<_Tp, 4>
{
    Scalar_();
    Scalar_(_Tp v0, _Tp v1, _Tp v2=0, _Tp v3=0);
    Scalar_(const CvScalar& s);
    Scalar_(_Tp v0);
    static Scalar_<_Tp> all(_Tp v0);
    operator CvScalar() const;

    template<typename T2> operator Scalar_<T2>() const;

    Scalar_<_Tp> mul(const Scalar_<_Tp>& t, double scale=1 ) const;
    template<typename T2> void convertTo(T2* buf, int channels, int unroll_to=0) const;
};

typedef Scalar_<double> Scalar;

//////////////////////////////// Range /////////////////////////////////

struct CV_EXPORTS Range
{
    Range();
    Range(int _start, int _end);
    int size() const;
    bool empty() const;
    static Range all();

    int start, end;
};

/////////////////////////////// DataType ////////////////////////////////

template<typename _Tp> struct DataType
{
    typedef _Tp value_type;
    typedef value_type channel_type;
    enum { depth = DataDepth<channel_type>::value, channels = 1,
           fmt=DataDepth<channel_type>::fmt,
           type = CV_MAKETYPE(depth, channels) };
};

template<> struct DataType<uchar>
{
    typedef uchar value_type;
    typedef value_type channel_type;
    enum { depth = DataDepth<channel_type>::value, channels = 1,
           fmt=DataDepth<channel_type>::fmt,
           type = CV_MAKETYPE(depth, channels) };
};

template<> struct DataType<schar>
{
    typedef schar value_type;
    typedef value_type channel_type;
    enum { depth = DataDepth<channel_type>::value, channels = 1,
           fmt=DataDepth<channel_type>::fmt,
           type = CV_MAKETYPE(depth, channels) };
};

template<> struct DataType<ushort>
{
    typedef ushort value_type;
    typedef value_type channel_type;
    enum { depth = DataDepth<channel_type>::value, channels = 1,
           fmt=DataDepth<channel_type>::fmt,
           type = CV_MAKETYPE(depth, channels) };
};

template<> struct DataType<short>
{
    typedef short value_type;
    typedef value_type channel_type;
    enum { depth = DataDepth<channel_type>::value, channels = 1,
           fmt=DataDepth<channel_type>::fmt,
           type = CV_MAKETYPE(depth, channels) };
};

template<> struct DataType<int>
{
    typedef int value_type;
    typedef value_type channel_type;
    enum { depth = DataDepth<channel_type>::value, channels = 1,
           fmt=DataDepth<channel_type>::fmt,
           type = CV_MAKETYPE(depth, channels) };
};

template<> struct DataType<float>
{
    typedef float value_type;
    typedef value_type channel_type;
    enum { depth = DataDepth<channel_type>::value, channels = 1,
           fmt=DataDepth<channel_type>::fmt,
           type = CV_MAKETYPE(depth, channels) };
};

template<> struct DataType<double>
{
    typedef double value_type;
    typedef value_type channel_type;
    enum { depth = DataDepth<channel_type>::value, channels = 1,
           fmt=DataDepth<channel_type>::fmt,
           type = CV_MAKETYPE(depth, channels) };
};

template<typename _Tp, int cn> struct DataType<Vec_<_Tp, cn> >
{
    typedef Vec_<_Tp, cn> value_type;
    typedef _Tp channel_type;
    enum { depth = DataDepth<channel_type>::value, channels = cn,
           fmt = ((channels-1)<<8) + DataDepth<channel_type>::fmt,
           type = CV_MAKETYPE(depth, channels) };
};

template<typename _Tp> struct DataType<std::complex<_Tp> >
{
    typedef std::complex<_Tp> value_type;
    typedef _Tp channel_type;
    enum { depth = DataDepth<channel_type>::value, channels = 2,
           fmt = ((channels-1)<<8) + DataDepth<channel_type>::fmt,
           type = CV_MAKETYPE(depth, channels) };
};

template<typename _Tp> struct DataType<Complex<_Tp> >
{
    typedef Complex<_Tp> value_type;
    typedef _Tp channel_type;
    enum { depth = DataDepth<channel_type>::value, channels = 2,
           fmt = ((channels-1)<<8) + DataDepth<channel_type>::fmt,
           type = CV_MAKETYPE(depth, channels) };
};

template<typename _Tp> struct DataType<Point_<_Tp> >
{
    typedef Point_<_Tp> value_type;
    typedef _Tp channel_type;
    enum { depth = DataDepth<channel_type>::value, channels = 2,
           fmt = ((channels-1)<<8) + DataDepth<channel_type>::fmt,
           type = CV_MAKETYPE(depth, channels) };
};

template<typename _Tp> struct DataType<Point3_<_Tp> >
{
    typedef Point3_<_Tp> value_type;
    typedef _Tp channel_type;
    enum { depth = DataDepth<channel_type>::value, channels = 3,
           fmt = ((channels-1)<<8) + DataDepth<channel_type>::fmt,
           type = CV_MAKETYPE(depth, channels) };
};

template<typename _Tp> struct DataType<Size_<_Tp> >
{
    typedef Size_<_Tp> value_type;
    typedef _Tp channel_type;
    enum { depth = DataDepth<channel_type>::value, channels = 2,
           fmt = ((channels-1)<<8) + DataDepth<channel_type>::fmt,
           type = CV_MAKETYPE(depth, channels) };
};

template<typename _Tp> struct DataType<Rect_<_Tp> >
{
    typedef Rect_<_Tp> value_type;
    typedef _Tp channel_type;
    enum { depth = DataDepth<channel_type>::value, channels = 4,
           fmt = ((channels-1)<<8) + DataDepth<channel_type>::fmt,
           type = CV_MAKETYPE(depth, channels) };
};

template<typename _Tp> struct DataType<Scalar_<_Tp> >
{
    typedef Scalar_<_Tp> value_type;
    typedef _Tp channel_type;
    enum { depth = DataDepth<channel_type>::value, channels = 4,
           fmt = ((channels-1)<<8) + DataDepth<channel_type>::fmt,
           type = CV_MAKETYPE(depth, channels) };
};

template<> struct DataType<Range>
{
    typedef Range value_type;
    typedef int channel_type;
    enum { depth = DataDepth<channel_type>::value, channels = 2,
           fmt = ((channels-1)<<8) + DataDepth<channel_type>::fmt,
           type = CV_MAKETYPE(depth, channels) };
};


//////////////////////////////// Vector ////////////////////////////////

// template vector class. It is similar to STL's vector,
// with a few important differences:
//   1) it can be created on top of user-allocated data w/o copying it
//   2) Vector b = a means copying the header,
//      not the underlying data (use clone() to make a deep copy)
template <typename _Tp> class CV_EXPORTS Vector
{
public:
    typedef _Tp value_type;
    typedef _Tp* iterator;
    typedef const _Tp* const_iterator;
    typedef _Tp& reference;
    typedef const _Tp& const_reference;

    struct CV_EXPORTS Hdr
    {
        Hdr() : data(0), datastart(0), refcount(0), size(0), capacity(0) {};
        _Tp* data;
        _Tp* datastart;
        int* refcount;
        size_t size;
        size_t capacity;
    };

    Vector();
    Vector(size_t _size);
    Vector(size_t _size, const _Tp& val);
    Vector(_Tp* _data, size_t _size, bool _copyData=false);
    Vector(const std::vector<_Tp>& vec, bool _copyData=false);
    Vector(const Vector& d);
    Vector(const Vector& d, const Range& r);

    Vector& operator = (const Vector& d);

    ~Vector();
    Vector clone() const;

    _Tp& operator [] (size_t i);
    const _Tp& operator [] (size_t i) const;
    _Tp& operator [] (int i);
    const _Tp& operator [] (int i) const;
    Vector operator() (const Range& r) const;
    _Tp& back();
    const _Tp& back() const;
    _Tp& front();
    const _Tp& front() const;

    _Tp* begin();
    _Tp* end();
    const _Tp* begin() const;
    const _Tp* end() const;

    void addref();
    void release();
    void set(_Tp* _data, size_t _size, bool _copyData=false);

    void reserve(size_t newCapacity);
    void resize(size_t newSize);
    Vector<_Tp>& push_back(const _Tp& elem);
    Vector<_Tp>& pop_back();
    size_t size() const;
    size_t capacity() const;
    bool empty() const;
    void clear();

protected:
    Hdr hdr;
};

//////////////////// Generic ref-cointing pointer class for C/C++ objects ////////////////////////

template<typename _Tp> struct CV_EXPORTS Ptr
{
    Ptr();
    Ptr(_Tp* _obj);
    ~Ptr();
    Ptr(const Ptr& ptr);
    Ptr& operator = (const Ptr& ptr);
    void addref();
    void release();
    void delete_obj();

    _Tp* operator -> ();
    const _Tp* operator -> () const;

    operator _Tp* ();
    operator const _Tp*() const;

    _Tp* obj;
    int* refcount;
};

//////////////////////////////// Mat ////////////////////////////////

struct Mat;
template<typename M> struct CV_EXPORTS MatExpr_Base_;
typedef MatExpr_Base_<Mat> MatExpr_Base;
template<typename E, typename M> struct MatExpr_;
template<typename A1, typename M, typename Op> struct MatExpr_Op1_;
template<typename A1, typename A2, typename M, typename Op> struct MatExpr_Op2_;
template<typename A1, typename A2, typename A3, typename M, typename Op> struct MatExpr_Op3_;
template<typename A1, typename A2, typename A3, typename A4,
        typename M, typename Op> struct MatExpr_Op4_;
template<typename A1, typename A2, typename A3, typename A4,
        typename A5, typename M, typename Op> struct MatExpr_Op5_;
template<typename M> struct CV_EXPORTS MatOp_DivRS_;
template<typename M> struct CV_EXPORTS MatOp_Inv_;
template<typename M> struct CV_EXPORTS MatOp_MulDiv_;
template<typename M> struct CV_EXPORTS MatOp_Repeat_;
template<typename M> struct CV_EXPORTS MatOp_Set_;
template<typename M> struct CV_EXPORTS MatOp_Scale_;
template<typename M> struct CV_EXPORTS MatOp_T_;

struct Mat;

typedef MatExpr_<MatExpr_Op4_<Size, int, Scalar,
    int, Mat, MatOp_Set_<Mat> >, Mat> MatExpr_Initializer;

template<typename _Tp> struct MatIterator_;
template<typename _Tp> struct MatConstIterator_;

enum { MAGIC_MASK=0xFFFF0000, TYPE_MASK=0x00000FFF, DEPTH_MASK=7 };

static inline size_t getElemSize(int type) { return CV_ELEM_SIZE(type); }

// matrix decomposition types
enum { DECOMP_LU=0, DECOMP_SVD=1, DECOMP_EIG=2, DECOMP_CHOLESKY=3, DECOMP_QR=4, DECOMP_NORMAL=16 };
enum { NORM_INF=1, NORM_L1=2, NORM_L2=4, NORM_RELATIVE=8};
enum { CMP_EQ=0, CMP_GT=1, CMP_GE=2, CMP_LT=3, CMP_LE=4, CMP_NE=5 };
enum { GEMM_1_T=1, GEMM_2_T=2, GEMM_3_T=4 };
enum { DFT_INVERSE=1, DFT_SCALE=2, DFT_ROWS=4, DFT_COMPLEX_OUTPUT=16, DFT_REAL_OUTPUT=32,
    DCT_INVERSE = DFT_INVERSE, DCT_ROWS=DFT_ROWS };

struct CV_EXPORTS Mat
{
    Mat();
    Mat(int _rows, int _cols, int _type);
    Mat(int _rows, int _cols, int _type, const Scalar& _s);
    Mat(Size _size, int _type);
    Mat(const Mat& m);
    Mat(int _rows, int _cols, int _type, void* _data, size_t _step=AUTO_STEP);
    Mat(Size _size, int _type, void* _data, size_t _step=AUTO_STEP);
    Mat(const Mat& m, const Range& rowRange, const Range& colRange);
    Mat(const Mat& m, const Rect& roi);
    Mat(const CvMat* m, bool copyData=false);
    Mat(const IplImage* img, bool copyData=false);
    Mat( const MatExpr_Base& expr );
    ~Mat();
    Mat& operator = (const Mat& m);
    Mat& operator = (const MatExpr_Base& expr);

    operator MatExpr_<Mat, Mat>() const;

    Mat row(int y) const;
    Mat col(int x) const;
    Mat rowRange(int startrow, int endrow) const;
    Mat rowRange(const Range& r) const;
    Mat colRange(int startcol, int endcol) const;
    Mat colRange(const Range& r) const;
    Mat diag(int d=0) const;
    static Mat diag(const Mat& d);

    Mat clone() const;
    void copyTo( Mat& m ) const;
    void copyTo( Mat& m, const Mat& mask ) const;
    void convertTo( Mat& m, int rtype, double alpha=1, double beta=0 ) const;

    void assignTo( Mat& m, int type=-1 ) const;
    Mat& operator = (const Scalar& s);
    Mat& setTo(const Scalar& s, const Mat& mask=Mat());
    Mat reshape(int _cn, int _rows=0) const;

    MatExpr_<MatExpr_Op2_<Mat, double, Mat, MatOp_T_<Mat> >, Mat>
    t() const;
    MatExpr_<MatExpr_Op2_<Mat, int, Mat, MatOp_Inv_<Mat> >, Mat>
        inv(int method=DECOMP_LU) const;
    MatExpr_<MatExpr_Op4_<Mat, Mat, double, char, Mat, MatOp_MulDiv_<Mat> >, Mat>
    mul(const Mat& m, double scale=1) const;
    MatExpr_<MatExpr_Op4_<Mat, Mat, double, char, Mat, MatOp_MulDiv_<Mat> >, Mat>
    mul(const MatExpr_<MatExpr_Op2_<Mat, double, Mat, MatOp_Scale_<Mat> >, Mat>& m, double scale=1) const;
    MatExpr_<MatExpr_Op4_<Mat, Mat, double, char, Mat, MatOp_MulDiv_<Mat> >, Mat>    
    mul(const MatExpr_<MatExpr_Op2_<Mat, double, Mat, MatOp_DivRS_<Mat> >, Mat>& m, double scale=1) const;

    Mat cross(const Mat& m) const;
    double dot(const Mat& m) const;

    static MatExpr_Initializer zeros(int rows, int cols, int type);
    static MatExpr_Initializer zeros(Size size, int type);
    static MatExpr_Initializer ones(int rows, int cols, int type);
    static MatExpr_Initializer ones(Size size, int type);
    static MatExpr_Initializer eye(int rows, int cols, int type);
    static MatExpr_Initializer eye(Size size, int type);

    void create(int _rows, int _cols, int _type);
    void create(Size _size, int _type);
    void addref();
    void release();

    void locateROI( Size& wholeSize, Point& ofs ) const;
    Mat& adjustROI( int dtop, int dbottom, int dleft, int dright );
    Mat operator()( Range rowRange, Range colRange ) const;
    Mat operator()( const Rect& roi ) const;

    operator CvMat() const;
    operator IplImage() const;
    bool isContinuous() const;
    size_t elemSize() const;
    size_t elemSize1() const;
    int type() const;
    int depth() const;
    int channels() const;
    size_t step1() const;
    Size size() const;

    uchar* ptr(int y=0);
    const uchar* ptr(int y=0) const;

    enum { MAGIC_VAL=0x42FF0000, AUTO_STEP=0, CONTINUOUS_FLAG=CV_MAT_CONT_FLAG };

    int flags;
    int rows, cols;
    size_t step;
    uchar* data;
    int* refcount;

    uchar* datastart;
    uchar* dataend;
};


// Multiply-with-Carry RNG
struct CV_EXPORTS RNG
{
    enum { A=4164903690U, UNIFORM=0, NORMAL=1 };

    RNG();
    RNG(unsigned seed);
    RNG(uint64 _state);
    unsigned next();

    operator uchar();
    operator schar();
    operator ushort();
    operator short();
    operator unsigned();
    operator int();
    operator float();
    operator double();
    void fill( Mat& mat, int distType, const Scalar& a, const Scalar& b );

    uint64 state;
};

struct CV_EXPORTS TermCriteria
{
    enum { COUNT=1, EPS=2 };

    TermCriteria();
    TermCriteria(int _type, int _maxCount, double _epsilon);
    
    int type;
    int maxCount;
    double epsilon;
};

CV_EXPORTS Mat cvarrToMat(const CvArr* arr, bool copyData=false, bool allowND=true);
CV_EXPORTS Mat extractImageCOI(const CvArr* arr);

CV_EXPORTS void add(const Mat& a, const Mat& b, Mat& c, const Mat& mask);
CV_EXPORTS void subtract(const Mat& a, const Mat& b, Mat& c, const Mat& mask);
CV_EXPORTS void add(const Mat& a, const Mat& b, Mat& c);
CV_EXPORTS void subtract(const Mat& a, const Mat& b, Mat& c);
CV_EXPORTS void add(const Mat& a, const Scalar& s, Mat& c, const Mat& mask=Mat());
CV_EXPORTS void subtract(const Mat& a, const Scalar& s, Mat& c, const Mat& mask=Mat());

CV_EXPORTS void multiply(const Mat& a, const Mat& b, Mat& c, double scale=1);
CV_EXPORTS void divide(const Mat& a, const Mat& b, Mat& c, double scale=1);
CV_EXPORTS void divide(double scale, const Mat& b, Mat& c);

CV_EXPORTS void subtract(const Scalar& s, const Mat& a, Mat& c, const Mat& mask=Mat());
CV_EXPORTS void scaleAdd(const Mat& a, double alpha, const Mat& b, Mat& c);
CV_EXPORTS void addWeighted(const Mat& a, double alpha, const Mat& b,
                            double beta, double gamma, Mat& c);
CV_EXPORTS void convertScaleAbs(const Mat& a, Mat& c, double alpha=1, double beta=0);
CV_EXPORTS void LUT(const Mat& a, const Mat& lut, Mat& b);

CV_EXPORTS Scalar sum(const Mat& m);
CV_EXPORTS int countNonZero( const Mat& m );

CV_EXPORTS Scalar mean(const Mat& m);
CV_EXPORTS Scalar mean(const Mat& m, const Mat& mask);
CV_EXPORTS void meanStdDev(const Mat& m, Scalar& mean, Scalar& stddev, const Mat& mask=Mat());
CV_EXPORTS double norm(const Mat& a, int normType=NORM_L2);
CV_EXPORTS double norm(const Mat& a, const Mat& b, int normType=NORM_L2);
CV_EXPORTS double norm(const Mat& a, int normType, const Mat& mask);
CV_EXPORTS double norm(const Mat& a, const Mat& b,
                       int normType, const Mat& mask);
CV_EXPORTS void normalize( const Mat& a, Mat& b, double alpha=1, double beta=0,
                          int norm_type=NORM_L2, int rtype=-1, const Mat& mask=Mat());

CV_EXPORTS void minMaxLoc(const Mat& a, double* minVal,
                          double* maxVal=0, Point* minLoc=0,
                          Point* maxLoc=0, const Mat& mask=Mat());
CV_EXPORTS void reduce(const Mat& m, Mat& dst, int dim, int rtype, int dtype=-1);
CV_EXPORTS void merge(const Vector<Mat>& mv, Mat& dst);
CV_EXPORTS void split(const Mat& m, Vector<Mat>& mv);
CV_EXPORTS void mixChannels(const Vector<Mat>& src, Vector<Mat>& dst,
                            const Vector<int>& fromTo);
CV_EXPORTS void flip(const Mat& a, Mat& b, int flipCode);

CV_EXPORTS void repeat(const Mat& a, int ny, int nx, Mat& b);
static inline Mat repeat(const Mat& src, int ny, int nx)
{
    if( nx == 1 && ny == 1 ) return src;
    Mat dst; repeat(src, ny, nx, dst); return dst;
}

CV_EXPORTS void bitwise_and(const Mat& a, const Mat& b, Mat& c, const Mat& mask=Mat());
CV_EXPORTS void bitwise_or(const Mat& a, const Mat& b, Mat& c, const Mat& mask=Mat());
CV_EXPORTS void bitwise_xor(const Mat& a, const Mat& b, Mat& c, const Mat& mask=Mat());
CV_EXPORTS void bitwise_and(const Mat& a, const Scalar& s, Mat& c, const Mat& mask=Mat());
CV_EXPORTS void bitwise_or(const Mat& a, const Scalar& s, Mat& c, const Mat& mask=Mat());
CV_EXPORTS void bitwise_xor(const Mat& a, const Scalar& s, Mat& c, const Mat& mask=Mat());
CV_EXPORTS void bitwise_not(const Mat& a, Mat& c);
CV_EXPORTS void absdiff(const Mat& a, const Mat& b, Mat& c);
CV_EXPORTS void absdiff(const Mat& a, const Scalar& s, Mat& c);
CV_EXPORTS void inRange(const Mat& src, const Mat& lowerb,
                        const Mat& upperb, Mat& dst);
CV_EXPORTS void inRange(const Mat& src, const Scalar& lowerb,
                        const Scalar& upperb, Mat& dst);
CV_EXPORTS void compare(const Mat& a, const Mat& b, Mat& c, int cmpop);
CV_EXPORTS void compare(const Mat& a, double s, Mat& c, int cmpop);
CV_EXPORTS void min(const Mat& a, const Mat& b, Mat& c);
CV_EXPORTS void min(const Mat& a, double alpha, Mat& c);
CV_EXPORTS void max(const Mat& a, const Mat& b, Mat& c);
CV_EXPORTS void max(const Mat& a, double alpha, Mat& c);

CV_EXPORTS void sqrt(const Mat& a, Mat& b);
CV_EXPORTS void pow(const Mat& a, double power, Mat& b);
CV_EXPORTS void exp(const Mat& a, Mat& b);
CV_EXPORTS void log(const Mat& a, Mat& b);
CV_EXPORTS float cubeRoot(float val);
CV_EXPORTS float fastAtan2(float y, float x);
CV_EXPORTS void polarToCart(const Mat& magnitude, const Mat& angle,
                            Mat& x, Mat& y, bool angleInDegrees=false);
CV_EXPORTS void cartToPolar(const Mat& x, const Mat& y,
                            const Mat& magnitude, const Mat& angle,
                            bool angleInDegrees=false);
CV_EXPORTS bool checkRange(const Mat& a, bool quiet=true, Point* pt=0,
                           double minVal=-DBL_MAX, double maxVal=DBL_MAX);

CV_EXPORTS void gemm(const Mat& a, const Mat& b, double alpha,
                     const Mat& c, double gamma, Mat& d, int flags=0);
CV_EXPORTS void mulTransposed( const Mat& a, Mat& c, bool aTa,
                               const Mat& delta=Mat(),
                               double scale=1, int rtype=-1 );
CV_EXPORTS void transpose(const Mat& a, Mat& b);
CV_EXPORTS void transform(const Mat& src, Mat& dst, const Mat& m );
CV_EXPORTS void perspectiveTransform(const Mat& src, Mat& dst, const Mat& m );

CV_EXPORTS void completeSymm(Mat& a, bool lowerToUpper=false);
CV_EXPORTS void setIdentity(Mat& c, const Scalar& s=Scalar(1));
CV_EXPORTS double determinant(const Mat& m);
CV_EXPORTS Scalar trace(const Mat& m);
CV_EXPORTS double invert(const Mat& a, Mat& c, int flags=DECOMP_LU);
CV_EXPORTS bool solve(const Mat& a, const Mat& b, Mat& x, int flags=DECOMP_LU);
CV_EXPORTS void sort(const Mat& a, Mat& b, int flags);
CV_EXPORTS void sortIdx(const Mat& a, Mat& b, int flags);
CV_EXPORTS void solveCubic(const Mat& coeffs, Mat& roots);
CV_EXPORTS void solvePoly(const Mat& coeffs, Mat& roots, int maxIters=20, int fig=100);
CV_EXPORTS bool eigen(const Mat& a, Mat& eigenvalues);
CV_EXPORTS bool eigen(const Mat& a, Mat& eigenvalues, Mat& eigenvectors);

CV_EXPORTS void calcCovariation( const Vector<Mat>& data, Mat& covar, Mat& mean,
                                 int flags, int ctype=CV_64F);
CV_EXPORTS void calcCovariation( const Mat& data, Mat& covar, Mat& mean,
                                 int flags, int ctype=CV_64F);

struct CV_EXPORTS PCA
{
    PCA();
    PCA(const Mat& data, const Mat& mean, int flags, int maxComponents=0);
    PCA& operator()(const Mat& data, const Mat& mean, int flags, int maxComponents=0);
    Mat project(const Mat& vec) const;
    Mat backProject(const Mat& vec) const;

    Mat eigenvectors;
    Mat eigenvalues;
    Mat mean;
};

struct CV_EXPORTS SVD
{
    enum { MODIFY_A=1, NO_UV=2, FULL_UV=4 };
    SVD();
    SVD( const Mat& m, int flags=0 );
    SVD& operator ()( const Mat& m, int flags=0 );

    static void solveZ( const Mat& m, Mat& dst );
    void backSubst( const Mat& rhs, Mat& dst ) const;

    Mat u, w, vt;
};

CV_EXPORTS double mahalanobis(const Mat& v1, const Mat& v2, const Mat& icovar);
static inline double mahalonobis(const Mat& v1, const Mat& v2, const Mat& icovar)
{ return mahalanobis(v1, v2, icovar); }

CV_EXPORTS void dft(const Mat& src, Mat& dst, int flags=0, int nonzeroRows=0);
CV_EXPORTS void idft(const Mat& src, Mat& dst, int flags=0, int nonzeroRows=0);
CV_EXPORTS void dct(const Mat& src, Mat& dst, int flags=0);
CV_EXPORTS void idct(const Mat& src, Mat& dst, int flags=0);
CV_EXPORTS void mulSpectrums(const Mat& a, const Mat& b, Mat& c,
                             int flags, bool conjB=false);
CV_EXPORTS int getOptimalDFTSize(int vecsize);

CV_EXPORTS int kmeans( const Mat& samples, int K,
                       Mat& labels, Mat& centers,
                       TermCriteria crit, int attempts=1,
                       int flags=0, double* compactness=0);

CV_EXPORTS void seqToVector( const CvSeq* ptseq, Vector<Point>& pts );

CV_EXPORTS RNG& theRNG();
static inline int randi() { return (int)theRNG(); }
static inline unsigned randu() { return (unsigned)theRNG(); }
static inline float randf() { return (float)theRNG(); }
static inline double randd() { return (double)theRNG(); }
static inline void randu(Mat& dst, const Scalar& low, const Scalar& high)
{ theRNG().fill(dst, RNG::UNIFORM, low, high); }
static inline void randn(Mat& dst, const Scalar& mean, const Scalar& stddev)
{ theRNG().fill(dst, RNG::NORMAL, mean, stddev); }
CV_EXPORTS void randShuffle(Mat& dst, RNG& rng, double iterFactor=1.);
static inline void randShuffle(Mat& dst, double iterFactor=1.)
{ randShuffle(dst, theRNG(), iterFactor); }

CV_EXPORTS void line(Mat& img, Point pt1, Point pt2, const Scalar& color,
                     int thickness=1, int lineType=8, int shift=0);

CV_EXPORTS void rectangle(Mat& img, Point pt1, Point pt2,
                          const Scalar& color, int thickness=1,
                          int lineType=8, int shift=0);

CV_EXPORTS void circle(Mat& img, Point center, int radius,
                       const Scalar& color, int thickness=1,
                       int lineType=8, int shift=0);

CV_EXPORTS void ellipse(Mat& img, Point center, Size axes,
                        double angle, double startAngle, double endAngle,
                        const Scalar& color, int thickness=1,
                        int lineType=8, int shift=0);

CV_EXPORTS void ellipse(Mat& img, const RotatedRect& box, const Scalar& color,
                        int thickness=1, int lineType=8, int shift=0 );

CV_EXPORTS void fillConvexPoly(Mat& img, const Vector<Point>& pts,
                               const Scalar& color, int lineType=8,
                               int shift=0);

CV_EXPORTS void fillPoly(Mat& img, const Vector<Vector<Point> >& pts,
                         const Scalar& color, int lineType=8, int shift=0,
                         Point offset=Point() );

CV_EXPORTS void polylines(Mat& img, const Vector<Vector<Point> >& pts, bool isClosed,
                          const Scalar& color, int thickness=1, int lineType=8, int shift=0 );

CV_EXPORTS bool clipLine(Size imgSize, Point& pt1, Point& pt2);

struct CV_EXPORTS LineIterator
{
    LineIterator(const Mat& img, Point pt1, Point pt2,
                 int connectivity=8, bool leftToRight=false);
    uchar* operator *();
    LineIterator& operator ++();
    LineIterator operator ++(int);

    uchar* ptr;
    int err, count;
    int minusDelta, plusDelta;
    int minusStep, plusStep;
};

CV_EXPORTS void ellipse2Poly( Point center, Size axes, int angle,
                              int arcStart, int arcEnd, int delta, Vector<Point>& pts );

enum
{
    FONT_HERSHEY_SIMPLEX = 0,
    FONT_HERSHEY_PLAIN = 1,
    FONT_HERSHEY_DUPLEX = 2,
    FONT_HERSHEY_COMPLEX = 3,
    FONT_HERSHEY_TRIPLEX = 4,
    FONT_HERSHEY_COMPLEX_SMALL = 5,
    FONT_HERSHEY_SCRIPT_SIMPLEX = 6,
    FONT_HERSHEY_SCRIPT_COMPLEX = 7,
    FONT_ITALIC = 16
};

CV_EXPORTS void putText( Mat& img, const String& text, Point org,
                         int fontFace, double fontScale, Scalar color,
                         int thickness=1, int linetype=8,
                         bool bottomLeftOrigin=false );

CV_EXPORTS Size getTextSize(const String& text, int fontFace,
                            double fontScale, int thickness,
                            int* baseLine);

///////////////////////////////// Mat_<_Tp> ////////////////////////////////////

template<typename _Tp> struct CV_EXPORTS Mat_ : public Mat
{
    typedef _Tp value_type;
    typedef typename DataType<_Tp>::channel_type channel_type;
    typedef MatIterator_<_Tp> iterator;
    typedef MatConstIterator_<_Tp> const_iterator;
    
    Mat_();
    Mat_(int _rows, int _cols);
    Mat_(int _rows, int _cols, const _Tp& value);
    Mat_(const Mat& m);
    Mat_(const Mat_& m);
    Mat_(int _rows, int _cols, _Tp* _data, size_t _step=AUTO_STEP);
    Mat_(const Mat_& m, const Range& rowRange, const Range& colRange);
    Mat_(const Mat_& m, const Rect& roi);
    Mat_(const MatExpr_Base& expr);
    //~Mat_();

    Mat_& operator = (const Mat& m);
    Mat_& operator = (const Mat_& m);
    Mat_& operator = (const _Tp& s);

    iterator begin();
    iterator end();
    const_iterator begin() const;
    const_iterator end() const;

    void create(int _rows, int _cols);
    void create(Size _size);
    Mat_ cross(const Mat_& m) const;
    Mat_& operator = (const MatExpr_Base& expr);
    template<typename T2> operator Mat_<T2>() const;
    Mat_ row(int y) const;
    Mat_ col(int x) const;
    Mat_ diag(int d=0) const;
    Mat_ clone() const;

    MatExpr_<MatExpr_Op2_<Mat_, double, Mat_, MatOp_T_<Mat> >, Mat_> t() const;
    MatExpr_<MatExpr_Op2_<Mat_, int, Mat_, MatOp_Inv_<Mat> >, Mat_> inv(int method=DECOMP_LU) const;

    MatExpr_<MatExpr_Op4_<Mat_, Mat_, double, char, Mat_, MatOp_MulDiv_<Mat> >, Mat_>
    mul(const Mat_& m, double scale=1) const;
    MatExpr_<MatExpr_Op4_<Mat_, Mat_, double, char, Mat_, MatOp_MulDiv_<Mat> >, Mat_>
    mul(const MatExpr_<MatExpr_Op2_<Mat_, double, Mat_,
        MatOp_Scale_<Mat> >, Mat_>& m, double scale=1) const;
    MatExpr_<MatExpr_Op4_<Mat_, Mat_, double, char, Mat_, MatOp_MulDiv_<Mat> >, Mat_>    
    mul(const MatExpr_<MatExpr_Op2_<Mat_, double, Mat_,
        MatOp_DivRS_<Mat> >, Mat_>& m, double scale=1) const;

    size_t elemSize() const;
    size_t elemSize1() const;
    int type() const;
    int depth() const;
    int channels() const;
    size_t stepT() const;
    size_t step1() const;

    static MatExpr_Initializer zeros(int rows, int cols);
    static MatExpr_Initializer zeros(Size size);
    static MatExpr_Initializer ones(int rows, int cols);
    static MatExpr_Initializer ones(Size size);
    static MatExpr_Initializer eye(int rows, int cols);
    static MatExpr_Initializer eye(Size size);

    Mat_ reshape(int _rows) const;
    Mat_& adjustROI( int dtop, int dbottom, int dleft, int dright );
    Mat_ operator()( const Range& rowRange, const Range& colRange ) const;
    Mat_ operator()( const Rect& roi ) const;

    _Tp* operator [](int y);
    const _Tp* operator [](int y) const;

    _Tp& operator ()(int row, int col);
    _Tp operator ()(int row, int col) const;

    operator MatExpr_<Mat_, Mat_>() const;
};

//////////// Iterators & Comma initializers //////////////////

template<typename _Tp>
struct CV_EXPORTS MatConstIterator_
{
    typedef _Tp value_type;
    typedef int difference_type;

    MatConstIterator_();
    MatConstIterator_(const Mat_<_Tp>* _m);
    MatConstIterator_(const Mat_<_Tp>* _m, int _row, int _col=0);
    MatConstIterator_(const Mat_<_Tp>* _m, Point _pt);
    MatConstIterator_(const MatConstIterator_& it);

    MatConstIterator_& operator = (const MatConstIterator_& it );
    _Tp operator *() const;
    _Tp operator [](int i) const;
    
    MatConstIterator_& operator += (int ofs);
    MatConstIterator_& operator -= (int ofs);
    MatConstIterator_& operator --();
    MatConstIterator_ operator --(int);
    MatConstIterator_& operator ++();
    MatConstIterator_ operator ++(int);
    Point pos() const;

    const Mat_<_Tp>* m;
    _Tp* ptr;
    _Tp* sliceEnd;
};


template<typename _Tp>
struct CV_EXPORTS MatIterator_ : MatConstIterator_<_Tp>
{
    typedef _Tp* pointer;
    typedef _Tp& reference;
    typedef std::random_access_iterator_tag iterator_category;

    MatIterator_();
    MatIterator_(Mat_<_Tp>* _m);
    MatIterator_(Mat_<_Tp>* _m, int _row, int _col=0);
    MatIterator_(const Mat_<_Tp>* _m, Point _pt);
    MatIterator_(const MatIterator_& it);
    MatIterator_& operator = (const MatIterator_<_Tp>& it );

    _Tp& operator *() const;
    _Tp& operator [](int i) const;

    MatIterator_& operator += (int ofs);
    MatIterator_& operator -= (int ofs);
    MatIterator_& operator --();
    MatIterator_ operator --(int);
    MatIterator_& operator ++();
    MatIterator_ operator ++(int);
};

template<typename _Tp> struct CV_EXPORTS MatOp_Iter_;

template<typename _Tp> struct CV_EXPORTS MatCommaInitializer_ :
    MatExpr_<MatExpr_Op1_<MatIterator_<_Tp>, Mat_<_Tp>, MatOp_Iter_<_Tp> >, Mat_<_Tp> >
{
    MatCommaInitializer_(Mat_<_Tp>* _m);
    template<typename T2> MatCommaInitializer_<_Tp>& operator , (T2 v);
    operator Mat_<_Tp>() const;
    Mat_<_Tp> operator *() const;
    void assignTo(Mat& m, int type=-1) const;
};

template<typename _Tp> struct VectorCommaInitializer_
{
    VectorCommaInitializer_(Vector<_Tp>* _vec);
    template<typename T2> VectorCommaInitializer_<_Tp>& operator , (T2 val);
    operator Vector<_Tp>() const;
    Vector<_Tp> operator *() const;

    Vector<_Tp>* vec;
    int idx;
};

template<typename _Tp, size_t fixed_size=CV_MAX_LOCAL_SIZE> struct CV_EXPORTS AutoBuffer
{
    typedef _Tp value_type;

    AutoBuffer();
    AutoBuffer(size_t _size);
    ~AutoBuffer();

    void allocate(size_t _size);
    void deallocate();
    operator _Tp* ();
    operator const _Tp* () const;

    _Tp* ptr;
    size_t size;
    _Tp buf[fixed_size];
};

/////////////////////////// multi-dimensional dense matrix //////////////////////////

struct MatND;
struct SparseMat;

struct CV_EXPORTS MatND
{
    MatND();
    MatND(const Vector<int>& _sizes, int _type);
    MatND(const Vector<int>& _sizes, int _type, const Scalar& _s);
    MatND(const MatND& m);
    MatND(const MatND& m, const Vector<Range>& ranges);
    MatND(const CvMatND* m, bool copyData=false);
    //MatND( const MatExpr_BaseND& expr );
    ~MatND();
    MatND& operator = (const MatND& m);
    //MatND& operator = (const MatExpr_BaseND& expr);

    //operator MatExpr_<MatND, MatND>() const;

    MatND clone() const;
    MatND operator()(const Vector<Range>& ranges) const;

    void copyTo( MatND& m ) const;
    void copyTo( MatND& m, const MatND& mask ) const;
    void convertTo( MatND& m, int rtype, double alpha=1, double beta=0 ) const;

    void assignTo( MatND& m, int type=-1 ) const;
    MatND& operator = (const Scalar& s);
    MatND& setTo(const Scalar& s, const MatND& mask=MatND());
    MatND reshape(int newcn, const Vector<int>& newsz=Vector<int>()) const;

    void create(const Vector<int>& _sizes, int _type);
    void addref();
    void release();

    operator Mat() const;
    operator CvMatND() const;
    bool isContinuous() const;
    size_t elemSize() const;
    size_t elemSize1() const;
    int type() const;
    int depth() const;
    int channels() const;
    size_t step(int i) const;
    size_t step1(int i) const;
    Vector<int> size() const;
    int size(int i) const;

    uchar* ptr(int i0);
    const uchar* ptr(int i0) const;
    uchar* ptr(int i0, int i1);
    const uchar* ptr(int i0, int i1) const;
    uchar* ptr(int i0, int i1, int i2);
    const uchar* ptr(int i0, int i1, int i2) const;
    uchar* ptr(const int* idx);
    const uchar* ptr(const int* idx) const;

    enum { MAGIC_VAL=0x42FE0000, AUTO_STEP=-1,
        CONTINUOUS_FLAG=CV_MAT_CONT_FLAG, MAX_DIM=CV_MAX_DIM };

    int flags;
    int dims;
    
    int* refcount;
    uchar* data;
    uchar* datastart;
    uchar* dataend;
    
    struct
    {
        int size;
        size_t step;
    }
    dim[MAX_DIM];
};

struct CV_EXPORTS NAryMatNDIterator
{
    NAryMatNDIterator();
    NAryMatNDIterator(const Vector<MatND>& arrays);
    void init(const Vector<MatND>& arrays);

    NAryMatNDIterator& operator ++();
    NAryMatNDIterator operator ++(int);
    
    Vector<MatND> arrays;
    Vector<Mat> planes;
    int iterdepth, idx, nplanes;
};

CV_EXPORTS void add(const MatND& a, const MatND& b, MatND& c, const MatND& mask);
CV_EXPORTS void subtract(const MatND& a, const MatND& b, MatND& c, const MatND& mask);
CV_EXPORTS void add(const MatND& a, const MatND& b, MatND& c);
CV_EXPORTS void subtract(const MatND& a, const MatND& b, MatND& c);
CV_EXPORTS void add(const MatND& a, const Scalar& s, MatND& c, const MatND& mask=MatND());

CV_EXPORTS void multiply(const MatND& a, const MatND& b, MatND& c, double scale=1);
CV_EXPORTS void divide(const MatND& a, const MatND& b, MatND& c, double scale=1);
CV_EXPORTS void divide(double scale, const MatND& b, MatND& c);

CV_EXPORTS void subtract(const Scalar& s, const MatND& a, MatND& c, const MatND& mask=MatND());
CV_EXPORTS void scaleAdd(const MatND& a, double alpha, const MatND& b, MatND& c);
CV_EXPORTS void addWeighted(const MatND& a, double alpha, const MatND& b,
                            double beta, double gamma, MatND& c);

CV_EXPORTS Scalar sum(const MatND& m);
CV_EXPORTS int countNonZero( const MatND& m );

CV_EXPORTS Scalar mean(const MatND& m);
CV_EXPORTS Scalar mean(const MatND& m, const MatND& mask);
CV_EXPORTS void meanStdDev(const MatND& m, Scalar& mean, Scalar& stddev, const MatND& mask=MatND());
CV_EXPORTS double norm(const MatND& a, int normType=NORM_L2, const MatND& mask=MatND());
CV_EXPORTS double norm(const MatND& a, const MatND& b,
                       int normType=NORM_L2, const MatND& mask=MatND());
CV_EXPORTS void normalize( const MatND& a, MatND& b, double alpha=1, double beta=0,
                          int norm_type=NORM_L2, int rtype=-1, const MatND& mask=MatND());

CV_EXPORTS void minMax(const MatND& a, double* minVal,
                       double* maxVal, const MatND& mask=MatND());
CV_EXPORTS void merge(const Vector<MatND>& mv, MatND& dst);
CV_EXPORTS void split(const MatND& m, Vector<MatND>& mv);
CV_EXPORTS void mixChannels(const Vector<MatND>& src, Vector<MatND>& dst,
                            const Vector<int>& fromTo);

CV_EXPORTS void bitwise_and(const MatND& a, const MatND& b, MatND& c, const MatND& mask=MatND());
CV_EXPORTS void bitwise_or(const MatND& a, const MatND& b, MatND& c, const MatND& mask=MatND());
CV_EXPORTS void bitwise_xor(const MatND& a, const MatND& b, MatND& c, const MatND& mask=MatND());
CV_EXPORTS void bitwise_and(const MatND& a, const Scalar& s, MatND& c, const MatND& mask=MatND());
CV_EXPORTS void bitwise_or(const MatND& a, const Scalar& s, MatND& c, const MatND& mask=MatND());
CV_EXPORTS void bitwise_xor(const MatND& a, const Scalar& s, MatND& c, const MatND& mask=MatND());
CV_EXPORTS void bitwise_not(const MatND& a, MatND& c);
CV_EXPORTS void absdiff(const MatND& a, const MatND& b, MatND& c);
CV_EXPORTS void absdiff(const MatND& a, const Scalar& s, MatND& c);
CV_EXPORTS void inRange(const MatND& src, const MatND& lowerb,
                        const MatND& upperb, MatND& dst);
CV_EXPORTS void inRange(const MatND& src, const Scalar& lowerb,
                        const Scalar& upperb, MatND& dst);
CV_EXPORTS void compare(const MatND& a, const MatND& b, MatND& c, int cmpop);
CV_EXPORTS void compare(const MatND& a, double s, MatND& c, int cmpop);
CV_EXPORTS void min(const MatND& a, const MatND& b, MatND& c);
CV_EXPORTS void min(const MatND& a, double alpha, MatND& c);
CV_EXPORTS void max(const MatND& a, const MatND& b, MatND& c);
CV_EXPORTS void max(const MatND& a, double alpha, MatND& c);

CV_EXPORTS void sqrt(const MatND& a, MatND& b);
CV_EXPORTS void pow(const MatND& a, double power, MatND& b);
CV_EXPORTS void exp(const MatND& a, MatND& b);
CV_EXPORTS void log(const MatND& a, MatND& b);
CV_EXPORTS bool checkRange(const MatND& a, bool quiet=true, int* idx=0,
                           double minVal=-DBL_MAX, double maxVal=DBL_MAX);

typedef void (*ConvertData)(const void* from, void* to, int cn);
typedef void (*ConvertScaleData)(const void* from, void* to, int cn, double alpha, double beta);

CV_EXPORTS ConvertData getConvertElem(int fromType, int toType);
CV_EXPORTS ConvertScaleData getConvertScaleElem(int fromType, int toType);

template<typename _Tp> struct CV_EXPORTS MatND_ : public MatND
{
    typedef _Tp value_type;
    typedef typename DataType<_Tp>::channel_type channel_type;

    MatND_();
    MatND_(const Vector<int>& _sizes);
    MatND_(const Vector<int>& _sizes, const _Tp& _s);
    MatND_(const MatND& m);
    MatND_(const MatND_& m);
    MatND_(const MatND_& m, const Vector<Range>& ranges);
    MatND_(const CvMatND* m, bool copyData=false);
    MatND_& operator = (const MatND& m);
    MatND_& operator = (const MatND_& m);
    MatND_& operator = (const _Tp& s);

    void create(const Vector<int>& _sizes);
    template<typename T2> operator MatND_<T2>() const;
    MatND_ clone() const;
    MatND_ operator()(const Vector<Range>& ranges) const;

    size_t elemSize() const;
    size_t elemSize1() const;
    int type() const;
    int depth() const;
    int channels() const;
    size_t stepT(int i) const;
    size_t step1(int i) const;

    _Tp& operator ()(const int* idx);
    const _Tp& operator ()(const int* idx) const;

    _Tp& operator ()(int idx0, int idx1, int idx2);
    const _Tp& operator ()(int idx0, int idx1, int idx2) const;
};

/////////////////////////// multi-dimensional sparse matrix //////////////////////////

struct SparseMatIterator;
struct SparseMatConstIterator;

struct CV_EXPORTS SparseMat
{
    typedef SparseMatIterator iterator;
    typedef SparseMatConstIterator const_iterator;

    struct CV_EXPORTS Hdr
    {
        Hdr(const Vector<int>& _sizes, int _type);
        void clear();
        int refcount;
        int dims;
        int valueOffset;
        size_t nodeSize;
        size_t nodeCount;
        size_t freeList;
        Vector<uchar> pool;
        Vector<size_t> hashtab;
        int size[CV_MAX_DIM];
    };

    struct CV_EXPORTS Node
    {
        size_t hashval;
        size_t next;
        int idx[CV_MAX_DIM];
    };

    SparseMat();
    SparseMat(const Vector<int>& _sizes, int _type);
    SparseMat(const SparseMat& m);
    SparseMat(const Mat& m, bool try1d=false);
    SparseMat(const MatND& m);
    SparseMat(const CvSparseMat* m);
    ~SparseMat();
    SparseMat& operator = (const SparseMat& m);
    SparseMat& operator = (const Mat& m);
    SparseMat& operator = (const MatND& m);

    SparseMat clone() const;
    void copyTo( SparseMat& m ) const;
    void copyTo( Mat& m ) const;
    void copyTo( MatND& m ) const;
    void convertTo( SparseMat& m, int rtype, double alpha=1 ) const;
    void convertTo( Mat& m, int rtype, double alpha=1, double beta=0 ) const;
    void convertTo( MatND& m, int rtype, double alpha=1, double beta=0 ) const;

    void assignTo( SparseMat& m, int type=-1 ) const;

    void create(const Vector<int>& _sizes, int _type);
    void clear();
    void addref();
    void release();

    operator CvSparseMat*() const;
    size_t elemSize() const;
    size_t elemSize1() const;
    int type() const;
    int depth() const;
    int channels() const;
    Vector<int> size() const;
    int size(int i) const;

    size_t hash(int i0) const;
    size_t hash(int i0, int i1) const;
    size_t hash(int i0, int i1, int i2) const;
    size_t hash(const int* idx) const;
    
    uchar* ptr(int i0, int i1, bool createMissing, size_t* hashval=0);
    const uchar* get(int i0, int i1, size_t* hashval=0) const;
    uchar* ptr(int i0, int i1, int i2, bool createMissing, size_t* hashval=0);
    const uchar* get(int i0, int i1, int i2, size_t* hashval=0) const;
    uchar* ptr(const int* idx, bool createMissing, size_t* hashval=0);
    const uchar* get(const int* idx, size_t* hashval=0) const;

    void erase(int i0, int i1, size_t* hashval=0);
    void erase(int i0, int i1, int i2, size_t* hashval=0);
    void erase(const int* idx, size_t* hashval=0);

    SparseMatIterator begin();
    SparseMatConstIterator begin() const;
    SparseMatIterator end();
    SparseMatConstIterator end() const;

    uchar* value(Node* n);
    const uchar* value(const Node* n) const;
    Node* node(size_t nidx);
    const Node* node(size_t nidx) const;

    uchar* newNode(const int* idx, size_t hashval);
    void removeNode(size_t hidx, size_t nidx, size_t previdx);
    void resizeHashTab(size_t newsize);

    enum { MAGIC_VAL=0x42FD0000, MAX_DIM=CV_MAX_DIM, HASH_SCALE=0x5bd1e995 };

    int flags;
    Hdr* hdr;
};

struct CV_EXPORTS SparseMatConstIterator
{
    SparseMatConstIterator();
    SparseMatConstIterator(const SparseMat* _m);
    SparseMatConstIterator(const SparseMatConstIterator& it);

    SparseMatConstIterator& operator = (const SparseMatConstIterator& it);
    const uchar* value() const;
    const SparseMat::Node* node() const;
    
    SparseMatConstIterator& operator --();
    SparseMatConstIterator operator --(int);
    SparseMatConstIterator& operator ++();
    SparseMatConstIterator operator ++(int);

    const SparseMat* m;
    size_t hashidx;
    uchar* ptr;
};

struct CV_EXPORTS SparseMatIterator : SparseMatConstIterator
{
    SparseMatIterator();
    SparseMatIterator(SparseMat* _m);
    SparseMatIterator(SparseMat* _m, const int* idx);
    SparseMatIterator(const SparseMatIterator& it);

    SparseMatIterator& operator = (const SparseMatIterator& it);
    uchar* value() const;
    SparseMat::Node* node() const;
    
    SparseMatIterator& operator ++();
    SparseMatIterator operator ++(int);
};


template<typename _Tp> struct SparseMatIterator_;
template<typename _Tp> struct SparseMatConstIterator_;

template<typename _Tp> struct CV_EXPORTS SparseMat_ : public SparseMat
{
    typedef SparseMatIterator_<_Tp> iterator;
    typedef SparseMatConstIterator_<_Tp> const_iterator;

    SparseMat_();
    SparseMat_(const Vector<int>& _sizes);
    SparseMat_(const SparseMat& m);
    SparseMat_(const SparseMat_& m);
    SparseMat_(const Mat& m);
    SparseMat_(const MatND& m);
    SparseMat_(const CvSparseMat* m);
    SparseMat_& operator = (const SparseMat& m);
    SparseMat_& operator = (const SparseMat_& m);
    SparseMat_& operator = (const Mat& m);
    SparseMat_& operator = (const MatND& m);

    SparseMat_ clone() const;
    void create(const Vector<int>& _sizes);
    operator CvSparseMat*() const;

    int type() const;
    int depth() const;
    int channels() const;
    
    _Tp& operator()(int i0, int i1, size_t* hashval=0);
    _Tp operator()(int i0, int i1, size_t* hashval=0) const;
    _Tp& operator()(int i0, int i1, int i2, size_t* hashval=0);
    _Tp operator()(int i0, int i1, int i2, size_t* hashval=0) const;
    _Tp& operator()(const int* idx, size_t* hashval=0);
    _Tp operator()(const int* idx, size_t* hashval=0) const;

    SparseMatIterator_<_Tp> begin();
    SparseMatConstIterator_<_Tp> begin() const;
    SparseMatIterator_<_Tp> end();
    SparseMatConstIterator_<_Tp> end() const;
};

template<typename _Tp> struct CV_EXPORTS SparseMatConstIterator_ : SparseMatConstIterator
{
    typedef std::forward_iterator_tag iterator_category;
    
    SparseMatConstIterator_();
    SparseMatConstIterator_(const SparseMat_<_Tp>* _m);
    SparseMatConstIterator_(const SparseMatConstIterator_& it);

    SparseMatConstIterator_& operator = (const SparseMatConstIterator_& it);
    const _Tp& operator *() const;
    
    SparseMatConstIterator_& operator ++();
    SparseMatConstIterator_ operator ++(int);

    const SparseMat_<_Tp>* m;
    size_t hashidx;
    uchar* ptr;
};

template<typename _Tp> struct CV_EXPORTS SparseMatIterator_ : SparseMatConstIterator_<_Tp>
{
    typedef std::forward_iterator_tag iterator_category;
    
    SparseMatIterator_();
    SparseMatIterator_(SparseMat_<_Tp>* _m);
    SparseMatIterator_(const SparseMatIterator_& it);

    SparseMatIterator_& operator = (const SparseMatIterator_& it);
    _Tp& operator *() const;
    
    SparseMatIterator_& operator ++();
    SparseMatIterator_ operator ++(int);
};

//////////////////////////////////////// XML & YAML I/O ////////////////////////////////////

struct CV_EXPORTS FileNode;

struct CV_EXPORTS FileStorage
{
    enum { READ=0, WRITE=1, APPEND=2 };
    enum { UNDEFINED=0, VALUE_EXPECTED=1, NAME_EXPECTED=2, INSIDE_MAP=4 };
    FileStorage();
    FileStorage(const String& filename, int flags);
    FileStorage(CvFileStorage* fs);
    virtual ~FileStorage();

    virtual bool open(const String& filename, int flags);
    virtual bool isOpened() const;
    virtual void release();

    FileNode root(int streamidx=0) const;
    FileNode operator[](const String& nodename) const;
    FileNode operator[](const char* nodename) const;

    CvFileStorage* operator *() { return fs.obj; }
    const CvFileStorage* operator *() const { return fs.obj; }
    void writeRaw( const String& fmt, const Vector<uchar>& vec );
    void writeObj( const String& name, const void* obj );

    Ptr<CvFileStorage> fs;
    String elname;
    Vector<char> structs;
    int state;
};

struct CV_EXPORTS FileNodeIterator;

struct CV_EXPORTS FileNode
{
    enum { NONE=0, INT=1, REAL=2, FLOAT=REAL, STR=3, STRING=STR, REF=4, SEQ=5, MAP=6, TYPE_MASK=7,
        FLOW=8, USER=16, EMPTY=32, NAMED=64 };
    FileNode();
    FileNode(const CvFileStorage* fs, const CvFileNode* node);
    FileNode(const FileNode& node);
    FileNode operator[](const String& nodename) const;
    FileNode operator[](const char* nodename) const;
    FileNode operator[](int i) const;
    int type() const;
    int rawDataSize(const String& fmt) const;
    bool isNone() const;
    bool isSeq() const;
    bool isMap() const;
    bool isInt() const;
    bool isReal() const;
    bool isString() const;
    bool isNamed() const;
    String name() const;
    size_t count() const;
    operator int() const;
    operator float() const;
    operator double() const;
    operator String() const;

    FileNodeIterator begin() const;
    FileNodeIterator end() const;

    void readRaw( const String& fmt, Vector<uchar>& vec ) const;
    void* readObj() const;

    // do not use wrapper pointer classes for better efficiency
    const CvFileStorage* fs;
    const CvFileNode* node;
};

struct CV_EXPORTS FileNodeIterator
{
    FileNodeIterator();
    FileNodeIterator(const CvFileStorage* fs, const CvFileNode* node, size_t ofs=0);
    FileNodeIterator(const FileNodeIterator& it);
    FileNode operator *() const;
    FileNode operator ->() const;

    FileNodeIterator& operator ++();
    FileNodeIterator operator ++(int);
    FileNodeIterator& operator --();
    FileNodeIterator operator --(int);
    FileNodeIterator& operator += (int);
    FileNodeIterator& operator -= (int);

    FileNodeIterator& readRaw( const String& fmt, Vector<uchar>& vec,
                               size_t maxCount=(size_t)INT_MAX );

    const CvFileStorage* fs;
    const CvFileNode* container;
    CvSeqReader reader;
    size_t remaining;
};

}

#endif // __cplusplus

#include "cxoperations.hpp"

#endif /*_CXCORE_HPP_*/
