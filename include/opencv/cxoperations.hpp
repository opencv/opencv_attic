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

#ifndef _OPENCV_CORE_OPERATIONS_H_
#define _OPENCV_CORE_OPERATIONS_H_

#ifndef SKIP_INCLUDES
#include <string.h>
#include <limits.h>
#endif // SKIP_INCLUDES

#ifdef __cplusplus

namespace cv
{

/////////////// saturate_cast (used in image & signal processing) ///////////////////

template<typename _Tp> static inline _Tp saturate_cast(uchar v) { return _Tp(v); }
template<typename _Tp> static inline _Tp saturate_cast(schar v) { return _Tp(v); }
template<typename _Tp> static inline _Tp saturate_cast(ushort v) { return _Tp(v); }
template<typename _Tp> static inline _Tp saturate_cast(short v) { return _Tp(v); }
template<typename _Tp> static inline _Tp saturate_cast(unsigned v) { return _Tp(v); }
template<typename _Tp> static inline _Tp saturate_cast(int v) { return _Tp(v); }
template<typename _Tp> static inline _Tp saturate_cast(float v) { return _Tp(v); }
template<typename _Tp> static inline _Tp saturate_cast(double v) { return _Tp(v); }

template<> inline uchar saturate_cast<uchar>(schar v)
{ return (uchar)std::max((int)v, 0); }
template<> inline uchar saturate_cast<uchar>(ushort v)
{ return (uchar)std::min((unsigned)v, (unsigned)UCHAR_MAX); }
template<> inline uchar saturate_cast<uchar>(int v)
{ return (uchar)((unsigned)v <= UCHAR_MAX ? v : v > 0 ? UCHAR_MAX : 0); }
template<> inline uchar saturate_cast<uchar>(short v)
{ return saturate_cast<uchar>((int)v); }
template<> inline uchar saturate_cast<uchar>(unsigned v)
{ return (uchar)std::min(v, (unsigned)UCHAR_MAX); }
template<> inline uchar saturate_cast<uchar>(float v)
{ int iv = cvRound(v); return saturate_cast<uchar>(iv); }
template<> inline uchar saturate_cast<uchar>(double v)
{ int iv = cvRound(v); return saturate_cast<uchar>(iv); }

template<> inline schar saturate_cast<schar>(uchar v)
{ return (schar)std::min((int)v, SCHAR_MAX); }
template<> inline schar saturate_cast<schar>(ushort v)
{ return (schar)std::min((unsigned)v, (unsigned)SCHAR_MAX); }
template<> inline schar saturate_cast<schar>(int v)
{
    return (schar)((unsigned)(v-SCHAR_MIN) <= (unsigned)UCHAR_MAX ?
                v : v > 0 ? SCHAR_MAX : SCHAR_MIN);
}
template<> inline schar saturate_cast<schar>(short v)
{ return saturate_cast<schar>((int)v); }
template<> inline schar saturate_cast<schar>(unsigned v)
{ return (schar)std::min(v, (unsigned)SCHAR_MAX); }

template<> inline schar saturate_cast<schar>(float v)
{ int iv = cvRound(v); return saturate_cast<schar>(iv); }
template<> inline schar saturate_cast<schar>(double v)
{ int iv = cvRound(v); return saturate_cast<schar>(iv); }

template<> inline ushort saturate_cast<ushort>(schar v)
{ return (ushort)std::max((int)v, 0); }
template<> inline ushort saturate_cast<ushort>(short v)
{ return (ushort)std::max((int)v, 0); }
template<> inline ushort saturate_cast<ushort>(int v)
{ return (ushort)((unsigned)v <= (unsigned)USHRT_MAX ? v : v > 0 ? USHRT_MAX : 0); }
template<> inline ushort saturate_cast<ushort>(unsigned v)
{ return (ushort)std::min(v, (unsigned)USHRT_MAX); }
template<> inline ushort saturate_cast<ushort>(float v)
{ int iv = cvRound(v); return saturate_cast<ushort>(iv); }
template<> inline ushort saturate_cast<ushort>(double v)
{ int iv = cvRound(v); return saturate_cast<ushort>(iv); }

template<> inline short saturate_cast<short>(ushort v)
{ return (short)std::min((int)v, SHRT_MAX); }
template<> inline short saturate_cast<short>(int v)
{
    return (short)((unsigned)(v - SHRT_MIN) <= (unsigned)USHRT_MAX ?
            v : v > 0 ? SHRT_MAX : SHRT_MIN);
}
template<> inline short saturate_cast<short>(unsigned v)
{ return (short)std::min(v, (unsigned)SHRT_MAX); }
template<> inline short saturate_cast<short>(float v)
{ int iv = cvRound(v); return saturate_cast<short>(iv); }
template<> inline short saturate_cast<short>(double v)
{ int iv = cvRound(v); return saturate_cast<short>(iv); }

template<> inline int saturate_cast<int>(float v) { return cvRound(v); }
template<> inline int saturate_cast<int>(double v) { return cvRound(v); }

// we intentionally do not clip negative numbers, to make -1 become 0xffffffff etc.
template<> inline unsigned saturate_cast<unsigned>(float v){ return cvRound(v); }
template<> inline unsigned saturate_cast<unsigned>(double v) { return cvRound(v); }


/////////////////////////// short vector (Vec_) /////////////////////////////

template<typename _Tp, int cn> inline Vec_<_Tp, cn>::Vec_() {}
template<typename _Tp, int cn> inline Vec_<_Tp, cn>::Vec_(_Tp v0)
{
    val[0] = v0;
    for(int i = 1; i < cn; i++) val[i] = _Tp();
}

template<typename _Tp, int cn> inline Vec_<_Tp, cn>::Vec_(_Tp v0, _Tp v1)
{
    val[0] = v0; val[1] = v1;
    for(int i = 2; i < cn; i++) val[i] = _Tp(0);
}

template<typename _Tp, int cn> inline Vec_<_Tp, cn>::Vec_(_Tp v0, _Tp v1, _Tp v2)
{
    val[0] = v0; val[1] = v1; val[2] = v2;
    for(int i = 3; i < cn; i++) val[i] = _Tp(0);
}

template<typename _Tp, int cn> inline Vec_<_Tp, cn>::Vec_(_Tp v0, _Tp v1, _Tp v2, _Tp v3)
{
    val[0] = v0; val[1] = v1; val[2] = v2; val[3] = v3;
    for(int i = 4; i < cn; i++) val[i] = _Tp(0);
}

template<typename _Tp, int cn> inline Vec_<_Tp, cn>::Vec_(const Vec_<_Tp, cn>& v)
{
    for( int i = 0; i < cn; i++ ) val[i] = v.val[i];
}

template<typename _Tp, int cn> inline Vec_<_Tp, cn> Vec_<_Tp, cn>::all(_Tp alpha)
{
    Vec_ v;
    for( int i = 0; i < cn; i++ ) v.val[i] = alpha;
    return v;
}

template<typename _Tp, int cn> inline _Tp Vec_<_Tp, cn>::dot(const Vec_<_Tp, cn>& v) const
{
    _Tp s = 0;
    for( int i = 0; i < cn; i++ ) s += val[i]*v.val[i];
    return s;
}

template<typename _Tp, int cn> inline double Vec_<_Tp, cn>::ddot(const Vec_<_Tp, cn>& v) const
{
    double s = 0;
    for( int i = 0; i < cn; i++ ) s += (double)val[i]*v.val[i];
    return s;
}

template<typename _Tp, int cn> inline Vec_<_Tp, cn> Vec_<_Tp, cn>::cross(const Vec_<_Tp, cn>& v) const
{
    return Vec_<_Tp, cn>(); // for arbitrary-size vector there is no cross-product defined
}

template<typename _Tp, int cn> template<typename T2>
inline Vec_<_Tp, cn>::operator Vec_<T2, cn>() const
{
    Vec_<T2, cn> v;
    for( int i = 0; i < cn; i++ ) v.val[i] = saturate_cast<T2>(val[i]);
    return v;
}

template<typename _Tp, int cn> inline Vec_<_Tp, cn>::operator CvScalar() const
{
    CvScalar s = {{0,0,0,0}};
    int i;
    for( i = 0; i < std::min(cn, 4); i++ ) s.val[i] = val[i];
    for( ; i < 4; i++ ) s.val[i] = 0;
    return s;
}

template<typename _Tp, int cn> inline _Tp Vec_<_Tp, cn>::operator [](int i) const { return val[i]; }
template<typename _Tp, int cn> inline _Tp& Vec_<_Tp, cn>::operator[](int i) { return val[i]; }

template<typename _Tp, int cn> static inline Vec_<_Tp, cn>
operator + (const Vec_<_Tp, cn>& a, const Vec_<_Tp, cn>& b)
{
    Vec_<_Tp, cn> c = a;
    return c += b;
}

template<typename _Tp, int cn> static inline Vec_<_Tp, cn>
operator - (const Vec_<_Tp, cn>& a, const Vec_<_Tp, cn>& b)
{
    Vec_<_Tp, cn> c = a;
    return c -= b;
}

template<typename _Tp> static inline
Vec_<_Tp, 2>& operator *= (Vec_<_Tp, 2>& a, _Tp alpha)
{
    a[0] *= alpha; a[1] *= alpha;
    return a;
}

template<typename _Tp> static inline
Vec_<_Tp, 3>& operator *= (Vec_<_Tp, 3>& a, _Tp alpha)
{
    a[0] *= alpha; a[1] *= alpha; a[2] *= alpha;
    return a;
}

template<typename _Tp> static inline
Vec_<_Tp, 4>& operator *= (Vec_<_Tp, 4>& a, _Tp alpha)
{
    a[0] *= alpha; a[1] *= alpha; a[2] *= alpha; a[3] *= alpha;
    return a;
}

template<typename _Tp, int cn> static inline Vec_<_Tp, cn>
operator * (const Vec_<_Tp, cn>& a, _Tp alpha)
{
    Vec_<_Tp, cn> c = a;
    return c *= alpha;
}

template<typename _Tp, int cn> static inline Vec_<_Tp, cn>
operator * (_Tp alpha, const Vec_<_Tp, cn>& a)
{
    return a * alpha;
}

template<typename _Tp, int cn> static inline Vec_<_Tp, cn>
operator - (const Vec_<_Tp, cn>& a)
{
    Vec_<_Tp,cn> t;
    for( int i = 0; i < cn; i++ ) t.val[i] = saturate_cast<_Tp>(-a.val[i]);
    return t;
}

template<> inline Vec_<float, 3> Vec_<float, 3>::cross(const Vec_<float, 3>& v) const
{
    return Vec_<float,3>(val[1]*v.val[2] - val[2]*v.val[1],
                     val[2]*v.val[0] - val[0]*v.val[2],
                     val[0]*v.val[1] - val[1]*v.val[0]);
}

template<> inline Vec_<double, 3> Vec_<double, 3>::cross(const Vec_<double, 3>& v) const
{
    return Vec_<double,3>(val[1]*v.val[2] - val[2]*v.val[1],
                     val[2]*v.val[0] - val[0]*v.val[2],
                     val[0]*v.val[1] - val[1]*v.val[0]);
}

template<typename T1, typename T2> static inline
Vec_<T1, 2>& operator += (Vec_<T1, 2>& a, const Vec_<T2, 2>& b)
{
    a[0] = saturate_cast<T1>(a[0] + b[0]);
    a[1] = saturate_cast<T1>(a[1] + b[1]);
    return a;
}

template<typename T1, typename T2> static inline
Vec_<T1, 3>& operator += (Vec_<T1, 3>& a, const Vec_<T2, 3>& b)
{
    a[0] = saturate_cast<T1>(a[0] + b[0]);
    a[1] = saturate_cast<T1>(a[1] + b[1]);
    a[2] = saturate_cast<T1>(a[2] + b[2]);
    return a;
}

template<typename T1, typename T2> static inline
Vec_<T1, 4>& operator += (Vec_<T1, 4>& a, const Vec_<T2, 4>& b)
{
    a[0] = saturate_cast<T1>(a[0] + b[0]);
    a[1] = saturate_cast<T1>(a[1] + b[1]);
    a[2] = saturate_cast<T1>(a[2] + b[2]);
    a[3] = saturate_cast<T1>(a[3] + b[3]);
    return a;
}

//////////////////////////////// Complex //////////////////////////////

template<typename _Tp> inline Complex<_Tp>::Complex() : re(0), im(0) {}
template<typename _Tp> inline Complex<_Tp>::Complex( _Tp _re, _Tp _im ) : re(_re), im(_im) {}
template<typename _Tp> template<typename T2> inline Complex<_Tp>::operator Complex<T2>() const
{ return Complex<T2>(saturate_cast<T2>(re), saturate_cast<T2>(im)); }
template<typename _Tp> inline Complex<_Tp> Complex<_Tp>::conj() const
{ return Complex<_Tp>(re, -im); }

template<typename _Tp> static inline
Complex<_Tp> operator + (const Complex<_Tp>& a, const Complex<_Tp>& b)
{ return Complex<_Tp>( a.re + b.re, a.im + b.im ); }

template<typename _Tp> static inline
Complex<_Tp>& operator += (Complex<_Tp>& a, const Complex<_Tp>& b)
{ a.re += b.re; a.im += b.im; return a; }

template<typename _Tp> static inline
Complex<_Tp> operator - (const Complex<_Tp>& a, const Complex<_Tp>& b)
{ return Complex<_Tp>( a.re - b.re, a.im - b.im ); }

template<typename _Tp> static inline
Complex<_Tp>& operator -= (Complex<_Tp>& a, const Complex<_Tp>& b)
{ a.re -= b.re; a.im -= b.im; return a; }

template<typename _Tp> static inline
Complex<_Tp> operator - (const Complex<_Tp>& a)
{ return Complex<_Tp>(-a.re, -a.im); }

template<typename _Tp> static inline
Complex<_Tp> operator * (const Complex<_Tp>& a, const Complex<_Tp>& b)
{ return Complex<_Tp>( a.re*b.re - a.im*b.im, a.re*b.im + a.im*b.re ); }

template<typename _Tp> static inline
Complex<_Tp> operator * (const Complex<_Tp>& a, _Tp b)
{ return Complex<_Tp>( a.re*b, a.im*b ); }

template<typename _Tp> static inline
Complex<_Tp> operator * (_Tp b, const Complex<_Tp>& a)
{ return Complex<_Tp>( a.re*b, a.im*b ); }

template<typename _Tp> static inline
Complex<_Tp> operator + (const Complex<_Tp>& a, _Tp b)
{ return Complex<_Tp>( a.re + b, a.im ); }

template<typename _Tp> static inline
Complex<_Tp> operator - (const Complex<_Tp>& a, _Tp b)
{ return Complex<_Tp>( a.re - b, a.im ); }

template<typename _Tp> static inline
Complex<_Tp> operator + (_Tp b, const Complex<_Tp>& a)
{ return Complex<_Tp>( a.re + b, a.im ); }

template<typename _Tp> static inline
Complex<_Tp> operator - (_Tp b, const Complex<_Tp>& a)
{ return Complex<_Tp>( b - a.re, -a.im ); }

template<typename _Tp> static inline
Complex<_Tp>& operator += (Complex<_Tp>& a, _Tp b)
{ a.re += b; return a; }

template<typename _Tp> static inline
Complex<_Tp>& operator -= (Complex<_Tp>& a, _Tp b)
{ a.re -= b; return a; }

template<typename _Tp> static inline
Complex<_Tp>& operator *= (Complex<_Tp>& a, _Tp b)
{ a.re *= b; a.im *= b; return a; }

template<typename _Tp> static inline
double abs(const Complex<_Tp>& a)
{ return std::sqrt( (double)a.re*a.re + (double)a.im*a.im); }

template<typename _Tp> static inline
Complex<_Tp> operator / (const Complex<_Tp>& a, const Complex<_Tp>& b)
{
    double t = 1./((double)b.re*b.re + (double)b.im*b.im);
    return Complex<_Tp>( (_Tp)((a.re*b.re + a.im*b.im)*t),
                        (_Tp)((-a.re*b.im + a.im*b.re)*t) );
}

template<typename _Tp> static inline
Complex<_Tp>& operator /= (Complex<_Tp>& a, const Complex<_Tp>& b)
{
    return (a = a / b);
}

template<typename _Tp> static inline
Complex<_Tp> operator / (const Complex<_Tp>& a, _Tp b)
{
    _Tp t = (_Tp)1/b;
    return Complex<_Tp>( a.re*t, a.im*t );
}

template<typename _Tp> static inline
Complex<_Tp> operator / (_Tp b, const Complex<_Tp>& a)
{
    return Complex<_Tp>(b)/a;
}

template<typename _Tp> static inline
Complex<_Tp> operator /= (const Complex<_Tp>& a, _Tp b)
{
    _Tp t = (_Tp)1/b;
    a.re *= t; a.im *= t; return a;
}

//////////////////////////////// 2D Point ////////////////////////////////

template<typename _Tp> inline Point_<_Tp>::Point_() : x(0), y(0) {}
template<typename _Tp> inline Point_<_Tp>::Point_(_Tp _x, _Tp _y) : x(_x), y(_y) {}
template<typename _Tp> inline Point_<_Tp>::Point_(const Point_& pt) : x(pt.x), y(pt.y) {}
template<typename _Tp> inline Point_<_Tp>::Point_(const CvPoint& pt) : x((_Tp)pt.x), y((_Tp)pt.y) {}
template<typename _Tp> inline Point_<_Tp>::Point_(const CvPoint2D32f& pt)
    : x(saturate_cast<_Tp>(pt.x)), y(saturate_cast<_Tp>(pt.y)) {}
template<typename _Tp> inline Point_<_Tp>::Point_(const Size_<_Tp>& sz) : x(sz.width), y(sz.height) {}
template<typename _Tp> inline Point_<_Tp>& Point_<_Tp>::operator = (const Point_& pt)
{ x = pt.x; y = pt.y; return *this; }

template<typename _Tp> inline Point_<_Tp>::operator Point_<int>() const
{ return Point_<int>(saturate_cast<int>(x), saturate_cast<int>(y)); }
template<typename _Tp> inline Point_<_Tp>::operator Point_<float>() const
{ return Point_<float>(float(x), float(y)); }
template<typename _Tp> inline Point_<_Tp>::operator Point_<double>() const
{ return Point_<double>(x, y); }
template<typename _Tp> inline Point_<_Tp>::operator CvPoint() const
{ return cvPoint(saturate_cast<int>(x), saturate_cast<int>(y)); }
template<typename _Tp> inline Point_<_Tp>::operator CvPoint2D32f() const
{ return cvPoint2D32f((float)x, (float)y); }

template<typename _Tp> inline _Tp Point_<_Tp>::dot(const Point_& pt) const
{ return x*pt.x + y*pt.y; }
template<typename _Tp> inline double Point_<_Tp>::ddot(const Point_& pt) const
{ return (double)x*pt.x + (double)y*pt.y; }

template<typename _Tp> static inline Point_<_Tp>&
operator += (Point_<_Tp>& a, const Point_<_Tp>& b) { a.x += b.x; a.y += b.y; return a; }

template<typename _Tp> static inline Point_<_Tp>&
operator -= (Point_<_Tp>& a, const Point_<_Tp>& b) { a.x -= b.x; a.y -= b.y; return a; }

template<typename _Tp> static inline double norm(const Point_<_Tp>& pt)
{ return std::sqrt((double)pt.x*pt.x + (double)pt.y*pt.y); }

template<typename _Tp> static inline bool operator == (const Point_<_Tp>& a, const Point_<_Tp>& b)
{ return a.x == b.x && a.y == b.y; }

template<typename _Tp> static inline bool operator != (const Point_<_Tp>& a, const Point_<_Tp>& b)
{ return !(a == b); }

template<typename _Tp> static inline Point_<_Tp> operator + (const Point_<_Tp>& a, const Point_<_Tp>& b)
{ return Point_<_Tp>( a.x + b.x, a.y + b.y ); }

template<typename _Tp> static inline Point_<_Tp> operator - (const Point_<_Tp>& a, const Point_<_Tp>& b)
{ return Point_<_Tp>( a.x - b.x, a.y - b.y ); }

template<typename _Tp> static inline Point_<_Tp> operator - (const Point_<_Tp>& a)
{ return Point_<_Tp>( -a.x, -a.y ); }

template<typename _Tp> static inline Point_<_Tp> operator * (const Point_<_Tp>& a, _Tp b)
{ return Point_<_Tp>( a.x*b, a.y*b ); }

template<typename _Tp> static inline Point_<_Tp> operator * (_Tp a, const Point_<_Tp>& b)
{ return Point_<_Tp>( a*b.x, a*b.y ); }

//////////////////////////////// 3D Point ////////////////////////////////

template<typename _Tp> inline Point3_<_Tp>::Point3_() : x(0), y(0), z(0) {}
template<typename _Tp> inline Point3_<_Tp>::Point3_(_Tp _x, _Tp _y, _Tp _z) : x(_x), y(_y), z(_z) {}
template<typename _Tp> inline Point3_<_Tp>::Point3_(const Point3_& pt) : x(pt.x), y(pt.y), z(pt.z) {}
template<typename _Tp> inline Point3_<_Tp>::Point3_(const CvPoint3D32f& pt) :
    x(saturate_cast<_Tp>(pt.x)), y(saturate_cast<_Tp>(pt.y)), z(saturate_cast<_Tp>(pt.z)) {}
template<typename _Tp> inline Point3_<_Tp>::Point3_(const Vec_<_Tp, 3>& t) : x(t[0]), y(t[1]), z(t[2]) {}

template<typename _Tp> inline Point3_<_Tp>::operator Point3_<int>() const
{ return Point3_<int>(saturate_cast<int>(x), saturate_cast<int>(y), saturate_cast<int>(z)); }
template<typename _Tp> inline Point3_<_Tp>::operator Point3_<float>() const
{ return Point3_<float>(float(x), float(y), float(z)); }
template<typename _Tp> inline Point3_<_Tp>::operator Point3_<double>() const
{ return Point3_<double>(x, y, z); }
template<typename _Tp> inline Point3_<_Tp>::operator CvPoint3D32f() const
{ return cvPoint3D32f((float)x, (float)y, (float)z); }

template<typename _Tp> inline Point3_<_Tp>& Point3_<_Tp>::operator = (const Point3_& pt)
{ x = pt.x; y = pt.y; z = pt.z; return *this; }

template<typename _Tp> inline _Tp Point3_<_Tp>::dot(const Point3_& pt) const
{ return x*pt.x + y*pt.y + z*pt.z; }
template<typename _Tp> inline double Point3_<_Tp>::ddot(const Point3_& pt) const
{ return (double)x*pt.x + (double)y*pt.y + (double)z*pt.z; }

template<typename _Tp> static inline Point3_<_Tp>&
operator += (Point3_<_Tp>& a, const Point3_<_Tp>& b) { a += b.x; a += b.y; a += b.z; return a; }
template<typename _Tp> static inline Point3_<_Tp>&
operator -= (Point3_<_Tp>& a, const Point3_<_Tp>& b) { a -= b.x; a -= b.y; a -= b.z; return a; }

template<typename _Tp> static inline double norm(const Point3_<_Tp>& pt)
{ return std::sqrt((double)pt.x*pt.x + (double)pt.y*pt.y + (double)pt.z*pt.z); }

template<typename _Tp> static inline bool operator == (const Point3_<_Tp>& a, const Point3_<_Tp>& b)
{ return a.x == b.x && a.y == b.y && a.z == b.z; }

template<typename _Tp> static inline Point3_<_Tp> operator + (const Point3_<_Tp>& a, const Point3_<_Tp>& b)
{ return Point3_<_Tp>( a.x + b.x, a.y + b.y, a.z + b.z ); }

template<typename _Tp> static inline Point3_<_Tp> operator - (const Point3_<_Tp>& a, const Point3_<_Tp>& b)
{ return Point3_<_Tp>( a.x - b.x, a.y - b.y, a.z - b.z ); }

template<typename _Tp> static inline Point3_<_Tp> operator - (const Point3_<_Tp>& a)
{ return Point3_<_Tp>( -a.x, -a.y, -a.z ); }

template<typename _Tp> static inline Point3_<_Tp> operator * (const Point3_<_Tp>& a, _Tp b)
{ return Point3_<_Tp>( a.x*b, a.y*b, a.z*b ); }

template<typename _Tp> static inline Point3_<_Tp> operator * (_Tp a, const Point3_<_Tp>& b)
{ return Point3_<_Tp>( a*b.x, a*b.y, a*b.z ); }

//////////////////////////////// Size ////////////////////////////////

template<typename _Tp> inline Size_<_Tp>::Size_()
    : width(0), height(0) {}
template<typename _Tp> inline Size_<_Tp>::Size_(_Tp _width, _Tp _height)
    : width(_width), height(_height) {}
template<typename _Tp> inline Size_<_Tp>::Size_(const Size_& sz)
    : width(sz.width), height(sz.height) {}
template<typename _Tp> inline Size_<_Tp>::Size_(const CvSize& sz)
    : width(saturate_cast<_Tp>(sz.width)), height(saturate_cast<_Tp>(sz.height)) {}
template<typename _Tp> inline Size_<_Tp>::Size_(const CvSize2D32f& sz)
    : width(saturate_cast<_Tp>(sz.width)), height(saturate_cast<_Tp>(sz.height)) {}
template<typename _Tp> inline Size_<_Tp>::Size_(const Point_<_Tp>& pt) : width(pt.x), height(pt.y) {}

template<typename _Tp> inline Size_<_Tp>::operator Size_<int>() const
{ return Size_<int>(saturate_cast<int>(width), saturate_cast<int>(height)); }
template<typename _Tp> inline Size_<_Tp>::operator Size_<float>() const
{ return Size_<float>(float(width), float(height)); }
template<typename _Tp> inline Size_<_Tp>::operator Size_<double>() const
{ return Size_<double>(width, height); }
template<typename _Tp> inline Size_<_Tp>::operator CvSize() const
{ return cvSize(saturate_cast<int>(width), saturate_cast<int>(height)); }
template<typename _Tp> inline Size_<_Tp>::operator CvSize2D32f() const
{ return cvSize2D32f((float)width, (float)height); }

template<typename _Tp> inline Size_<_Tp>& Size_<_Tp>::operator = (const Size_<_Tp>& sz)
{ width = sz.width; height = sz.height; return *this; }
template<typename _Tp> inline _Tp Size_<_Tp>::area() const { return width*height; }

template<typename _Tp> static inline Size_<_Tp>& operator += (Size_<_Tp>& a, const Size_<_Tp>& b)
{ a.width += b.width; a.height += b.height; return a; }
template<typename _Tp> static inline Size_<_Tp>& operator -= (Size_<_Tp>& a, const Size_<_Tp>& b)
{ a.width -= b.width; a.height -= b.height; return a; }

template<typename _Tp> static inline bool operator == (const Size_<_Tp>& a, const Size_<_Tp>& b)
{ return a.width == b.width && a.height == b.height; }
template<typename _Tp> static inline bool operator != (const Size_<_Tp>& a, const Size_<_Tp>& b)
{ return a.width != b.width || a.height != b.height; }

//////////////////////////////// Rect ////////////////////////////////


template<typename _Tp> inline Rect_<_Tp>::Rect_() : x(0), y(0), width(0), height(0) {}
template<typename _Tp> inline Rect_<_Tp>::Rect_(_Tp _x, _Tp _y, _Tp _width, _Tp _height) : x(_x), y(_y), width(_width), height(_height) {}
template<typename _Tp> inline Rect_<_Tp>::Rect_(const Rect_<_Tp>& r) : x(r.x), y(r.y), width(r.width), height(r.height) {}
template<typename _Tp> inline Rect_<_Tp>::Rect_(const CvRect& r) : x((_Tp)r.x), y((_Tp)r.y), width((_Tp)r.width), height((_Tp)r.height) {}
template<typename _Tp> inline Rect_<_Tp>::Rect_(const Point_<_Tp>& org, const Size_<_Tp>& sz) :
    x(org.x), y(org.y), width(sz.width), height(sz.height) {}
template<typename _Tp> inline Rect_<_Tp>::Rect_(const Point_<_Tp>& pt1, const Point_<_Tp>& pt2)
{
    x = min(pt1.x, pt2.x); y = min(pt1.y, pt2.y);
    width = max(pt1.x, pt2.x) - x; height = max(pt1.y, pt2.y) - y;
}
template<typename _Tp> inline Rect_<_Tp>& Rect_<_Tp>::operator = ( const Rect_<_Tp>& r )
{ x = r.x; y = r.y; width = r.width; height = r.height; return *this; }

template<typename _Tp> inline Point_<_Tp> Rect_<_Tp>::tl() const { return Point_<_Tp>(x,y); }
template<typename _Tp> inline Point_<_Tp> Rect_<_Tp>::br() const { return Point_<_Tp>(x+width, y+height); }

template<typename _Tp> static inline Rect_<_Tp>& operator += ( Rect_<_Tp>& a, const Point_<_Tp>& b )
{ a.x += b.x; a.y += b.y; return a; }
template<typename _Tp> static inline Rect_<_Tp>& operator -= ( Rect_<_Tp>& a, const Point_<_Tp>& b )
{ a.x -= b.x; a.y -= b.y; return a; }

template<typename _Tp> static inline Rect_<_Tp>& operator += ( Rect_<_Tp>& a, const Size_<_Tp>& b )
{ a.width += b.width; a.height += b.height; return a; }

template<typename _Tp> static inline Rect_<_Tp>& operator -= ( Rect_<_Tp>& a, const Size_<_Tp>& b )
{ a.width -= b.width; a.height -= b.height; return a; }

template<typename _Tp> static inline Rect_<_Tp>& operator &= ( Rect_<_Tp>& a, const Rect_<_Tp>& b )
{
    _Tp x1 = max(a.x, b.x), y1 = max(a.y, b.y);
    a.width = min(a.x + a.width, b.x + b.width) - x1;
    a.height = min(a.y + a.height, b.y + b.height) - y1;
    a.x = x1; a.y = y1;
    if( a.width <= 0 || a.height <= 0 )
        a = Rect();
    return a;
}

template<typename _Tp> static inline Rect_<_Tp>& operator |= ( Rect_<_Tp>& a, const Rect_<_Tp>& b )
{
    _Tp x1 = min(a.x, b.x), y1 = min(a.y, b.y);
    a.width = max(a.x + a.width, b.x + b.width) - x1;
    a.height = max(a.y + a.height, b.y + b.height) - y1;
    a.x = x1; a.y = y1;
    return a;
}

template<typename _Tp> inline Size_<_Tp> Rect_<_Tp>::size() const { return Size_<_Tp>(width, height); }
template<typename _Tp> inline _Tp Rect_<_Tp>::area() const { return width*height; }

template<typename _Tp> inline Rect_<_Tp>::operator Rect_<int>() const
{ return Rect_<int>(saturate_cast<int>(x), saturate_cast<int>(y),
                    saturate_cast<int>(width), saturate_cast<int>(height)); }
template<typename _Tp> inline Rect_<_Tp>::operator Rect_<float>() const { return Rect_<float>(float(x), float(y), float(width), float(height)); }
template<typename _Tp> inline Rect_<_Tp>::operator Rect_<double>() const { return Rect_<double>(x, y, width, height); }
template<typename _Tp> inline Rect_<_Tp>::operator CvRect() const
{ return cvRect(saturate_cast<int>(x), saturate_cast<int>(y),
                saturate_cast<int>(width), saturate_cast<int>(height)); }

template<typename _Tp> inline bool Rect_<_Tp>::contains(const Point_<_Tp>& pt) const
{ return x <= pt.x && pt.x < x + width && y <= pt.y && pt.y < y + height; }

template<typename _Tp> static inline bool operator == (const Rect_<_Tp>& a, const Rect_<_Tp>& b)
{
    return a.x == b.x && a.y == b.y && a.width == b.width && a.height == b.height;
}

template<typename _Tp> static inline Rect_<_Tp> operator + (const Rect_<_Tp>& a, const Point_<_Tp>& b)
{
    return Rect_<_Tp>( a.x + b.x, a.y + b.y, a.width, a.height );
}

template<typename _Tp> static inline Rect_<_Tp> operator - (const Rect_<_Tp>& a, const Point_<_Tp>& b)
{
    return Rect_<_Tp>( a.x - b.x, a.y - b.y, a.width, a.height );
}

template<typename _Tp> static inline Rect_<_Tp> operator + (const Rect_<_Tp>& a, const Size_<_Tp>& b)
{
    return Rect_<_Tp>( a.x, a.y, a.width + b.width, a.height + b.height );
}

template<typename _Tp> static inline Rect_<_Tp> operator & (const Rect_<_Tp>& a, const Rect_<_Tp>& b)
{
    Rect_<_Tp> c = a;
    return c &= b;
}

template<typename _Tp> static inline Rect_<_Tp> operator | (const Rect_<_Tp>& a, const Rect_<_Tp>& b)
{
    Rect_<_Tp> c = a;
    return c |= b;
}

template<typename _Tp> inline bool Point_<_Tp>::inside( const Rect_<_Tp>& r ) const
{
    return r.contains(*this);
}

inline RotatedRect::RotatedRect() { angle = 0; }
inline RotatedRect::RotatedRect(const Point2f& _center, const Size2f& _size, float _angle)
    : center(_center), size(_size), angle(_angle) {}
inline RotatedRect::RotatedRect(const CvBox2D& box)
    : center(box.center), size(box.size), angle(box.angle) {}
inline RotatedRect::operator CvBox2D() const
{
    CvBox2D box; box.center = center; box.size = size; box.angle = angle;
    return box;
}

//////////////////////////////// Scalar_ ///////////////////////////////

template<typename _Tp> inline Scalar_<_Tp>::Scalar_()
{ this->val[0] = this->val[1] = this->val[2] = this->val[3] = 0; }

template<typename _Tp> inline Scalar_<_Tp>::Scalar_(_Tp v0, _Tp v1, _Tp v2, _Tp v3)
{ this->val[0] = v0; this->val[1] = v1; this->val[2] = v2; this->val[3] = v3; }

template<typename _Tp> inline Scalar_<_Tp>::Scalar_(const CvScalar& s)
{
    this->val[0] = saturate_cast<_Tp>(s.val[0]);
    this->val[1] = saturate_cast<_Tp>(s.val[1]);
    this->val[2] = saturate_cast<_Tp>(s.val[2]);
    this->val[3] = saturate_cast<_Tp>(s.val[3]);
}

template<typename _Tp> inline Scalar_<_Tp>::Scalar_(_Tp v0)
{ this->val[0] = v0; this->val[1] = this->val[2] = this->val[3] = 0; }

template<typename _Tp> inline Scalar_<_Tp> Scalar_<_Tp>::all(_Tp v0)
{ return Scalar_<_Tp>(v0, v0, v0, v0); }
template<typename _Tp> inline Scalar_<_Tp>::operator CvScalar() const
{ return cvScalar(this->val[0], this->val[1], this->val[2], this->val[3]); }

template<typename _Tp> template<typename T2> inline Scalar_<_Tp>::operator Scalar_<T2>() const
{
    return Scalar_<T2>(saturate_cast<T2>(this->val[0]),
                  saturate_cast<T2>(this->val[1]),
                  saturate_cast<T2>(this->val[2]),
                  saturate_cast<T2>(this->val[3]));
}

template<typename _Tp> static inline Scalar_<_Tp>& operator += (Scalar_<_Tp>& a, const Scalar_<_Tp>& b)
{
    a.val[0] = saturate_cast<_Tp>(a.val[0] + b.val[0]);
    a.val[1] = saturate_cast<_Tp>(a.val[1] + b.val[1]);
    a.val[2] = saturate_cast<_Tp>(a.val[2] + b.val[2]);
    a.val[3] = saturate_cast<_Tp>(a.val[3] + b.val[3]);
    return a;
}

template<typename _Tp> static inline Scalar_<_Tp>& operator -= (Scalar_<_Tp>& a, const Scalar_<_Tp>& b)
{
    a.val[0] = saturate_cast<_Tp>(a.val[0] - b.val[0]);
    a.val[1] = saturate_cast<_Tp>(a.val[1] - b.val[1]);
    a.val[2] = saturate_cast<_Tp>(a.val[2] - b.val[2]);
    a.val[3] = saturate_cast<_Tp>(a.val[3] - b.val[3]);
    return a;
}

template<typename _Tp> static inline Scalar_<_Tp>& operator *= ( Scalar_<_Tp>& a, _Tp v )
{
    a.val[0] = saturate_cast<_Tp>(a.val[0] * v);
    a.val[1] = saturate_cast<_Tp>(a.val[1] * v);
    a.val[2] = saturate_cast<_Tp>(a.val[2] * v);
    a.val[3] = saturate_cast<_Tp>(a.val[3] * v);
    return a;
}

template<typename _Tp> inline Scalar_<_Tp> Scalar_<_Tp>::mul(const Scalar_<_Tp>& t, double scale ) const
{
    return Scalar_<_Tp>( saturate_cast<_Tp>(this->val[0]*t.val[0]*scale),
                       saturate_cast<_Tp>(this->val[1]*t.val[1]*scale),
                       saturate_cast<_Tp>(this->val[2]*t.val[2]*scale),
                       saturate_cast<_Tp>(this->val[3]*t.val[3]*scale));
}

template<typename _Tp> static inline bool operator == ( const Scalar_<_Tp>& a, const Scalar_<_Tp>& b )
{
    return a.val[0] == b.val[0] && a.val[1] == b.val[1] &&
        a.val[2] == b.val[2] && a.val[3] == b.val[3];
}

template<typename _Tp> static inline bool operator != ( const Scalar_<_Tp>& a, const Scalar_<_Tp>& b )
{
    return a.val[0] != b.val[0] || a.val[1] != b.val[1] ||
        a.val[2] != b.val[2] || a.val[3] != b.val[3];
}

template<typename _Tp> template<typename T2> inline void Scalar_<_Tp>::convertTo(T2* buf, int cn, int unroll_to) const
{
    int i;
    CV_Assert(cn <= 4);
    for( i = 0; i < cn; i++ )
        buf[i] = saturate_cast<T2>(this->val[i]);
    for( ; i < unroll_to; i++ )
        buf[i] = buf[i-cn];
}

static inline void scalarToRawData(const Scalar& s, void* buf, int type, int unroll_to=0)
{
    int depth = CV_MAT_DEPTH(type), cn = CV_MAT_CN(type);
    switch(depth)
    {
    case CV_8U:
        s.convertTo((uchar*)buf, cn, unroll_to);
        break;
    case CV_8S:
        s.convertTo((schar*)buf, cn, unroll_to);
        break;
    case CV_16U:
        s.convertTo((ushort*)buf, cn, unroll_to);
        break;
    case CV_16S:
        s.convertTo((short*)buf, cn, unroll_to);
        break;
    case CV_32S:
        s.convertTo((int*)buf, cn, unroll_to);
        break;
    case CV_32F:
        s.convertTo((float*)buf, cn, unroll_to);
        break;
    case CV_64F:
        s.convertTo((double*)buf, cn, unroll_to);
        break;
    default:
        CV_Error(CV_StsUnsupportedFormat,"");
    }
}


template<typename _Tp> static inline Scalar_<_Tp> operator + (const Scalar_<_Tp>& a, const Scalar_<_Tp>& b)
{
    return Scalar_<_Tp>(saturate_cast<_Tp>(a.val[0] + b.val[0]),
                      saturate_cast<_Tp>(a.val[1] + b.val[1]),
                      saturate_cast<_Tp>(a.val[2] + b.val[2]),
                      saturate_cast<_Tp>(a.val[3] + b.val[3]));
}

template<typename _Tp> static inline Scalar_<_Tp> operator - (const Scalar_<_Tp>& a, const Scalar_<_Tp>& b)
{
    return Scalar_<_Tp>(saturate_cast<_Tp>(a.val[0] - b.val[0]),
                      saturate_cast<_Tp>(a.val[1] - b.val[1]),
                      saturate_cast<_Tp>(a.val[2] - b.val[2]),
                      saturate_cast<_Tp>(a.val[3] - b.val[3]));
}

template<typename _Tp> static inline Scalar_<_Tp> operator * (const Scalar_<_Tp>& a, _Tp alpha)
{
    return Scalar_<_Tp>(saturate_cast<_Tp>(a.val[0] * alpha),
                      saturate_cast<_Tp>(a.val[1] * alpha),
                      saturate_cast<_Tp>(a.val[2] * alpha),
                      saturate_cast<_Tp>(a.val[3] * alpha));
}

template<typename _Tp> static inline Scalar_<_Tp> operator * (_Tp alpha, const Scalar_<_Tp>& a)
{
    return a*alpha;
}

template<typename _Tp> static inline Scalar_<_Tp> operator - (const Scalar_<_Tp>& a)
{
    return Scalar_<_Tp>(saturate_cast<_Tp>(-a.val[0]), saturate_cast<_Tp>(-a.val[1]),
                      saturate_cast<_Tp>(-a.val[2]), saturate_cast<_Tp>(-a.val[3]));
}

//////////////////////////////// Range /////////////////////////////////

inline Range::Range() : start(0), end(0) {}
inline Range::Range(int _start, int _end) : start(_start), end(_end) {}
inline int Range::size() const { return end - start; }
inline bool Range::empty() const { return start == end; }
inline Range Range::all() { return Range(INT_MIN, INT_MAX); }

static inline bool operator == (const Range& r1, const Range& r2)
{ return r1.start == r2.start && r1.end == r2.end; }

static inline bool operator != (const Range& r1, const Range& r2)
{ return !(r1 == r2); }

static inline bool operator !(const Range& r)
{ return r.start == r.end; }

static inline Range operator & (const Range& r1, const Range& r2)
{
    Range r(std::max(r1.start, r2.start), std::min(r2.start, r2.end));
    r.end = std::max(r.end, r.start);
    return r;
}

static inline Range& operator &= (Range& r1, const Range& r2)
{
    r1 = r1 & r2;
    return r1;
}

static inline Range operator + (const Range& r1, int delta)
{
    return Range(r1.start + delta, r1.end + delta);
}

static inline Range operator + (int delta, const Range& r1)
{
    return Range(r1.start + delta, r1.end + delta);
}

static inline Range operator - (const Range& r1, int delta)
{
    return r1 + (-delta);
}


template <typename _Tp> inline Vector<_Tp>::Vector() {}
template <typename _Tp> inline Vector<_Tp>::Vector(size_t _size) { resize(_size); }
template <typename _Tp> inline Vector<_Tp>::Vector(size_t _size, const _Tp& val)
{
    resize(_size);
    for(size_t i = 0; i < _size; i++)
        hdr.data[i] = val;
}
template <typename _Tp> inline Vector<_Tp>::Vector(_Tp* _data, size_t _size, bool _copyData)
{ set(_data, _size, _copyData); }
template <typename _Tp> inline Vector<_Tp>::Vector(const std::vector<_Tp>& vec, bool _copyData)
{ set(&vec[0], vec.size(), _copyData); }
template <typename _Tp> inline Vector<_Tp>::Vector(const Vector& d)
{ *this = d; }
template <typename _Tp> inline Vector<_Tp>::Vector(const Vector& d, const Range& r)
{
    if( r == Range::all() )
        r = Range(0, d.size());
    if( r.size() > 0 && r.start >= 0 && r.end <= d.size() )
    {
        if( d.hdr.refcount )
            ++*d.hdr.refcount;
        hdr.refcount = d.hdr.refcount;
        hdr.datastart = d.hdr.datastart;
        hdr.data = d.hdr.data + r.start;
        hdr.capacity = hdr.size = r.size();
    }
}

template <typename _Tp> inline Vector<_Tp>& Vector<_Tp>::operator = (const Vector<_Tp>& d)
{
    if( this != &d )
    {
        if( d.hdr.refcount )
            ++*d.hdr.refcount;
        release();
        hdr = d.hdr;
    }
    return *this;
}

template <typename _Tp> inline Vector<_Tp>::~Vector() { release(); }
template <typename _Tp> inline Vector<_Tp> Vector<_Tp>::clone() const
{ return Vector(hdr.data, hdr.size, true); }

template <typename _Tp> inline void Vector<_Tp>::copyTo(Vector<_Tp>& vec) const
{
    size_t i, sz = size();
    vec.resize(sz);
    const _Tp* src = hdr.data;
    _Tp* dst = vec.hdr.data;
    for( i = 0; i < sz; i++ )
        dst[i] = src[i];
}

template <typename _Tp> inline void Vector<_Tp>::copyTo(std::vector<_Tp>& vec) const
{
    size_t i, sz = size();
    vec.resize(sz);
    const _Tp* src = hdr.data;
    _Tp* dst = sz ? &vec[0] : 0;
    for( i = 0; i < sz; i++ )
        dst[i] = src[i];
}

template <typename _Tp> inline Vector<_Tp>::operator CvMat() const
{ return cvMat(1, size(), type(), (void*)hdr.data); }

template <typename _Tp> inline _Tp& Vector<_Tp>::operator [] (size_t i) { CV_DbgAssert( i < size() ); return hdr.data[i]; }
template <typename _Tp> inline const _Tp& Vector<_Tp>::operator [] (size_t i) const { CV_DbgAssert( i < size() ); return hdr.data[i]; }
template <typename _Tp> inline _Tp& Vector<_Tp>::operator [] (int i) { CV_DbgAssert( (size_t)i < size() ); return hdr.data[i]; }
template <typename _Tp> inline const _Tp& Vector<_Tp>::operator [] (int i) const { CV_DbgAssert( (size_t)i < size() ); return hdr.data[i]; }
template <typename _Tp> inline Vector<_Tp> Vector<_Tp>::operator() (const Range& r) const { return Vector(*this, r); }
template <typename _Tp> inline _Tp& Vector<_Tp>::back() { CV_DbgAssert(!empty()); return hdr.data[hdr.size-1]; }
template <typename _Tp> inline const _Tp& Vector<_Tp>::back() const { CV_DbgAssert(!empty()); return hdr.data[hdr.size-1]; }
template <typename _Tp> inline _Tp& Vector<_Tp>::front() { CV_DbgAssert(!empty()); return hdr.data[0]; }
template <typename _Tp> inline const _Tp& Vector<_Tp>::front() const { CV_DbgAssert(!empty()); return hdr.data[0]; }

template <typename _Tp> inline _Tp* Vector<_Tp>::begin() { return hdr.data; }
template <typename _Tp> inline _Tp* Vector<_Tp>::end() { return hdr.data + hdr.size; }
template <typename _Tp> inline const _Tp* Vector<_Tp>::begin() const { return hdr.data; }
template <typename _Tp> inline const _Tp* Vector<_Tp>::end() const { return hdr.data + hdr.size; }

template <typename _Tp> inline void Vector<_Tp>::addref()
{ if( hdr.refcount ) ++*hdr.refcount; }

template <typename _Tp> inline void Vector<_Tp>::release()
{
    if( hdr.refcount && --*hdr.refcount == 0 )
        fastFree_<_Tp>(hdr.datastart, hdr.capacity);
    hdr = Hdr();
}

template <typename _Tp> inline void Vector<_Tp>::set(_Tp* _data, size_t _size, bool _copyData)
{
    if( !_copyData )
    {
        release();
        hdr.data = hdr.datastart = _data;
        hdr.size = hdr.capacity = _size;
        hdr.refcount = 0;
    }
    else
    {
        reserve(_size);
        for( size_t i = 0; i < _size; i++ )
            hdr.data[i] = _data[i];
        hdr.size = _size;
    }
}

template <typename _Tp> inline void Vector<_Tp>::reserve(size_t newCapacity)
{
    _Tp* newData;
    int* newRefcount;
    size_t i, oldSize = hdr.size;
    if( (!hdr.refcount || *hdr.refcount == 1) && hdr.capacity >= newCapacity )
        return;
    newCapacity = std::max(newCapacity, oldSize);
    size_t datasize = alignSize(newCapacity*sizeof(_Tp), (size_t)sizeof(*newRefcount));
    newData = (_Tp*)fastMalloc(datasize + sizeof(*newRefcount));
    for( i = 0; i < oldSize; i++ )
        ::new(newData + i) _Tp(hdr.data[i]);
    _Tp dummy = _Tp();
    for( ; i < newCapacity; i++ )
        ::new(newData + i) _Tp(dummy);
    newRefcount = (int*)((uchar*)newData + datasize);
    *newRefcount = 1;
    release();
    hdr.data = hdr.datastart = newData;
    hdr.capacity = newCapacity;
    hdr.size = oldSize;
    hdr.refcount = newRefcount;
}

template <typename _Tp> inline void Vector<_Tp>::resize(size_t newSize)
{
    size_t i;
    newSize = std::max(newSize, (size_t)0);
    if( (!hdr.refcount || *hdr.refcount == 1) && hdr.size == newSize )
        return;
    if( newSize > hdr.capacity )
        reserve(std::max(newSize, std::max((size_t)4, hdr.capacity*2)));
    for( i = hdr.size; i < newSize; i++ )
        hdr.data[i] = _Tp();
    hdr.size = newSize;
}

template <typename _Tp> inline Vector<_Tp>& Vector<_Tp>::push_back(const _Tp& elem)
{
    if( hdr.size == hdr.capacity )
        reserve( std::max((size_t)4, hdr.capacity*2) );
    hdr.data[hdr.size++] = elem;
    return *this;
}
template <typename _Tp> inline Vector<_Tp>& Vector<_Tp>::pop_back()
{
    if( hdr.size > 0 )
        --hdr.size;
    return *this;
}

template <typename _Tp> inline size_t Vector<_Tp>::size() const { return hdr.size; }
template <typename _Tp> inline size_t Vector<_Tp>::capacity() const { return hdr.capacity; }
template <typename _Tp> inline bool Vector<_Tp>::empty() const { return hdr.size == 0; }
template <typename _Tp> inline void Vector<_Tp>::clear() { resize(0); }
template <typename _Tp> inline int Vector<_Tp>::type() const
{ return DataType<_Tp>::type; }

template<typename _Tp> inline typename DataType<_Tp>::work_type
dot(const Vector<_Tp>& v1, const Vector<_Tp>& v2)
{
    typedef typename DataType<_Tp>::work_type _Tw;
    size_t i, n = v1.size();
    assert(v1.size() == v2.size());

    _Tw s = 0;
    const _Tp *ptr1 = &v1[0], *ptr2 = &v2[0];
    for( i = 0; i <= n - 4; i += 4 )
        s += (_Tw)ptr1[i]*ptr2[i] + (_Tw)ptr1[i+1]*ptr2[i+1] +
            (_Tw)ptr1[i+2]*ptr2[i+2] + (_Tw)ptr1[i+3]*ptr2[i+3];
    for( ; i < n; i++ )
        s += (_Tw)ptr1[i]*ptr2[i];
    return s;
}

// Multiply-with-Carry RNG
inline RNG::RNG() { state = 0xffffffff; }
inline RNG::RNG(unsigned seed) { state = seed ? seed : 0xffffffff; }
inline RNG::RNG(uint64 _state) { state = _state ? _state : 0xffffffff; }
inline unsigned RNG::next()
{
    state = (uint64)(unsigned)state*A + (unsigned)(state >> 32);
    return (unsigned)state;
}

inline RNG::operator uchar() { return (uchar)next(); }
inline RNG::operator schar() { return (schar)next(); }
inline RNG::operator ushort() { return (ushort)next(); }
inline RNG::operator short() { return (short)next(); }
inline RNG::operator unsigned() { return next(); }
inline RNG::operator int() { return (int)next(); }
// * (2^32-1)^-1
inline RNG::operator float() { return next()*2.3283064365386962890625e-10f; }
inline RNG::operator double()
{
    unsigned t = next();
    return (((uint64)t << 32) | next())*5.4210108624275221700372640043497e-20;
}

inline TermCriteria::TermCriteria() : type(0), maxCount(0), epsilon(0) {}
inline TermCriteria::TermCriteria(int _type, int _maxCount, double _epsilon)
    : type(_type), maxCount(_maxCount), epsilon(_epsilon) {}
inline TermCriteria::TermCriteria(const CvTermCriteria& criteria)
    : type(criteria.type), maxCount(criteria.max_iter), epsilon(criteria.epsilon) {}
inline TermCriteria::operator CvTermCriteria() const
{ return cvTermCriteria(type, maxCount, epsilon); }

inline uchar* LineIterator::operator *() { return ptr; }
inline LineIterator& LineIterator::operator ++()
{
    int mask = err < 0 ? -1 : 0;
    err += minusDelta + (plusDelta & mask);
    ptr += minusStep + (plusStep & mask);
    return *this;
}
inline LineIterator LineIterator::operator ++(int)
{
    LineIterator it = *this;
    ++(*this);
    return it;
}

template<typename _Tp> inline VectorCommaInitializer_<_Tp>::
VectorCommaInitializer_(Vector<_Tp>* _vec) : vec(_vec), idx(0) {}

template<typename _Tp> template<typename T2> inline VectorCommaInitializer_<_Tp>&
VectorCommaInitializer_<_Tp>::operator , (T2 val)
{
    if( (size_t)idx < vec->size() )
        (*vec)[idx] = _Tp(val);
    else
        vec->push_back(_Tp(val));
    idx++;
    return *this;
}

template<typename _Tp> inline VectorCommaInitializer_<_Tp>::operator Vector<_Tp>() const
{ return *vec; }

template<typename _Tp> inline Vector<_Tp> VectorCommaInitializer_<_Tp>::operator *() const
{ return *vec; }

template<typename _Tp, typename T2> static inline VectorCommaInitializer_<_Tp>
operator << (const Vector<_Tp>& vec, T2 val)
{
    VectorCommaInitializer_<_Tp> commaInitializer((Vector<_Tp>*)&vec);
    return (commaInitializer, val);
}

/////////////////////////////// AutoBuffer ////////////////////////////////////////

template<typename _Tp, size_t fixed_size> inline AutoBuffer<_Tp, fixed_size>::AutoBuffer()
: ptr(buf), size(fixed_size) {}

template<typename _Tp, size_t fixed_size> inline AutoBuffer<_Tp, fixed_size>::AutoBuffer(size_t _size)
: ptr(buf), size(fixed_size) { allocate(_size); }

template<typename _Tp, size_t fixed_size> inline AutoBuffer<_Tp, fixed_size>::~AutoBuffer()
{ deallocate(); }

template<typename _Tp, size_t fixed_size> inline void AutoBuffer<_Tp, fixed_size>::allocate(size_t _size)
{
    if(_size <= size)
        return;
    deallocate();
    if(_size > fixed_size)
    {
        ptr = (_Tp*)fastMalloc(_size*sizeof(_Tp));
        size = _size;
    }
}

template<typename _Tp, size_t fixed_size> inline void AutoBuffer<_Tp, fixed_size>::deallocate()
{
    if( ptr != buf )
    {
        fastFree(ptr);
        ptr = buf;
        size = fixed_size;
    }
}

template<typename _Tp, size_t fixed_size> inline AutoBuffer<_Tp, fixed_size>::operator _Tp* ()
{ return ptr; }

template<typename _Tp, size_t fixed_size> inline AutoBuffer<_Tp, fixed_size>::operator const _Tp* () const
{ return ptr; }


/////////////////////////////////// Ptr ////////////////////////////////////////

template<typename _Tp> inline Ptr<_Tp>::Ptr() : obj(0), refcount(0) {}
template<typename _Tp> inline Ptr<_Tp>::Ptr(_Tp* _obj) : obj(_obj)
{
    if(obj)
    {
        refcount = (int*)fastMalloc(sizeof(*refcount));
        *refcount = 1;
    }
    else
        refcount = 0;
}

template<typename _Tp> inline void Ptr<_Tp>::addref()
{ if( refcount ) ++*refcount; }

template<typename _Tp> inline void Ptr<_Tp>::release()
{
    if( refcount && --*refcount == 0 )
    {
        delete_obj();
        fastFree(refcount);
    }
    refcount = 0;
    obj = 0;
}

template<typename _Tp> inline void Ptr<_Tp>::delete_obj()
{
    if( obj ) delete obj;
}

template<typename _Tp> inline Ptr<_Tp>::~Ptr() { release(); }

template<typename _Tp> inline Ptr<_Tp>::Ptr(const Ptr<_Tp>& ptr)
{
    obj = ptr.obj;
    refcount = ptr.refcount;
    addref();
}

template<typename _Tp> inline Ptr<_Tp>& Ptr<_Tp>::operator = (const Ptr<_Tp>& ptr)
{
    int* _refcount = ptr.refcount;
    if( _refcount )
        ++*_refcount;
    release();
    obj = ptr.obj;
    refcount = _refcount;
    return *this;
}

template<typename _Tp> inline _Tp* Ptr<_Tp>::operator -> () { return obj; }
template<typename _Tp> inline const _Tp* Ptr<_Tp>::operator -> () const { return obj; }

template<typename _Tp> inline Ptr<_Tp>::operator _Tp* () { return obj; }
template<typename _Tp> inline Ptr<_Tp>::operator const _Tp*() const { return obj; }

//////////////////////////////////////// XML & YAML I/O ////////////////////////////////////

template<> inline void Ptr<CvFileStorage>::delete_obj()
{ cvReleaseFileStorage(&obj); }

static inline void write( FileStorage& fs, const String& name, int value )
{ cvWriteInt( *fs, name.size() ? name.c_str() : 0, value ); }

static inline void write( FileStorage& fs, const String& name, float value )
{ cvWriteReal( *fs, name.size() ? name.c_str() : 0, value ); }

static inline void write( FileStorage& fs, const String& name, double value )
{ cvWriteReal( *fs, name.size() ? name.c_str() : 0, value ); }

static inline void write( FileStorage& fs, const String& name, const String& value )
{ cvWriteString( *fs, name.size() ? name.c_str() : 0, value.c_str() ); }

template<typename _Tp> static inline void write(FileStorage& fs, const _Tp& value)
{ write(fs, String(), value); }

template<> inline void write(FileStorage& fs, const int& value )
{ cvWriteInt( *fs, 0, value ); }

template<> inline void write(FileStorage& fs, const float& value )
{ cvWriteReal( *fs, 0, value ); }

template<> inline void write(FileStorage& fs, const double& value )
{ cvWriteReal( *fs, 0, value ); }

template<> inline void write(FileStorage& fs, const String& value )
{ cvWriteString( *fs, 0, value.c_str() ); }

template<typename _Tp> inline void write(FileStorage& fs, const Point_<_Tp>& pt )
{
    write(fs, pt.x);
    write(fs, pt.y);
}

template<typename _Tp> inline void write(FileStorage& fs, const Point3_<_Tp>& pt )
{
    write(fs, pt.x);
    write(fs, pt.y);
    write(fs, pt.z);
}

template<typename _Tp> inline void write(FileStorage& fs, const Size_<_Tp>& sz )
{
    write(fs, sz.width);
    write(fs, sz.height);
}

template<typename _Tp> inline void write(FileStorage& fs, const Complex<_Tp>& c )
{
    write(fs, c.re);
    write(fs, c.im);
}

template<typename _Tp> inline void write(FileStorage& fs, const Rect_<_Tp>& r )
{
    write(fs, r.x);
    write(fs, r.y);
    write(fs, r.width);
    write(fs, r.height);
}

template<typename _Tp, int cn> inline void write(FileStorage& fs, const Vec_<_Tp, cn>& v )
{
    for(int i = 0; i < cn; i++)
        write(fs, v.val[i]);
}

template<typename _Tp> inline void write(FileStorage& fs, const Scalar_<_Tp>& s )
{
    write(fs, s.val[0]);
    write(fs, s.val[1]);
    write(fs, s.val[2]);
    write(fs, s.val[3]);
}

inline void write(FileStorage& fs, const Range& r )
{
    write(fs, r.start);
    write(fs, r.end);
}

struct CV_EXPORTS WriteStructContext
{
    WriteStructContext(FileStorage& _fs, const String& name,
        int flags, const String& typeName=String()) : fs(&_fs)
    {
        cvStartWriteStruct(**fs, !name.empty() ? name.c_str() : 0, flags,
            !typeName.empty() ? typeName.c_str() : 0);
    }
    ~WriteStructContext() { cvEndWriteStruct(**fs); }
    FileStorage* fs;
};

template<typename _Tp> inline void write(FileStorage& fs, const String& name, const Point_<_Tp>& pt )
{
    WriteStructContext ws(fs, name, CV_NODE_SEQ+CV_NODE_FLOW);
    write(fs, pt.x);
    write(fs, pt.y);
}

template<typename _Tp> inline void write(FileStorage& fs, const String& name, const Point3_<_Tp>& pt )
{
    WriteStructContext ws(fs, name, CV_NODE_SEQ+CV_NODE_FLOW);
    write(fs, pt.x);
    write(fs, pt.y);
    write(fs, pt.z);
}

template<typename _Tp> inline void write(FileStorage& fs, const String& name, const Size_<_Tp>& sz )
{
    WriteStructContext ws(fs, name, CV_NODE_SEQ+CV_NODE_FLOW);
    write(fs, sz.width);
    write(fs, sz.height);
}

template<typename _Tp> inline void write(FileStorage& fs, const String& name, const Complex<_Tp>& c )
{
    WriteStructContext ws(fs, name, CV_NODE_SEQ+CV_NODE_FLOW);
    write(fs, c.re);
    write(fs, c.im);
}

template<typename _Tp> inline void write(FileStorage& fs, const String& name, const Rect_<_Tp>& r )
{
    WriteStructContext ws(fs, name, CV_NODE_SEQ+CV_NODE_FLOW);
    write(fs, r.x);
    write(fs, r.y);
    write(fs, r.width);
    write(fs, r.height);
}

template<typename _Tp, int cn> inline void write(FileStorage& fs, const String& name, const Vec_<_Tp, cn>& v )
{
    WriteStructContext ws(fs, name, CV_NODE_SEQ+CV_NODE_FLOW);
    for(int i = 0; i < cn; i++)
        write(fs, v.val[i]);
}

template<typename _Tp> inline void write(FileStorage& fs, const String& name, const Scalar_<_Tp>& s )
{
    WriteStructContext ws(fs, name, CV_NODE_SEQ+CV_NODE_FLOW);
    write(fs, s.val[0]);
    write(fs, s.val[1]);
    write(fs, s.val[2]);
    write(fs, s.val[3]);
}

inline void write(FileStorage& fs, const String& name, const Range& r )
{
    WriteStructContext ws(fs, name, CV_NODE_SEQ+CV_NODE_FLOW);
    write(fs, r.start);
    write(fs, r.end);
}

template<typename _Tp, int numflag> struct CV_EXPORTS VecWriterProxy
{
    VecWriterProxy( FileStorage* _fs ) : fs(_fs) {}
    void operator()(const Vector<_Tp>& vec) const
    {
        size_t i, count = vec.size();
        for( i = 0; i < count; i++ )
            write( *fs, vec[i] );
    }
    FileStorage* fs;
};

template<typename _Tp> struct CV_EXPORTS VecWriterProxy<_Tp,1>
{
    VecWriterProxy( FileStorage* _fs ) : fs(_fs) {}
    void operator()(const Vector<_Tp>& vec) const
    {
        int _fmt = DataType<_Tp>::fmt;
        char fmt[] = { (char)((_fmt>>8)+'1'), (char)_fmt, '\0' };
        fs->writeRaw( String(fmt), Vector<uchar>((uchar*)&vec[0], vec.size()*sizeof(_Tp)) );
    }
    FileStorage* fs;
};


template<typename _Tp> static inline void write( FileStorage& fs, const Vector<_Tp>& vec )
{
    VecWriterProxy<_Tp, DataType<_Tp>::fmt != 0> w(&fs);
    w(vec);
}

template<typename _Tp> static inline FileStorage&
operator << ( FileStorage& fs, const Vector<_Tp>& vec )
{
    VecWriterProxy<_Tp, DataType<_Tp>::fmt != 0> w(&fs);
    w(vec);
    return fs;
}

CV_EXPORTS void write( FileStorage& fs, const String& name, const Mat& value );

template<typename _Tp> static inline FileStorage& operator << (FileStorage& fs, const _Tp& value)
{
    if( !fs.isOpened() )
        return fs;
    if( fs.state == FileStorage::NAME_EXPECTED + FileStorage::INSIDE_MAP )
        CV_Error( CV_StsError, "No element name has been given" );
    write( fs, fs.elname, value );
    if( fs.state & FileStorage::INSIDE_MAP )
        fs.state = FileStorage::NAME_EXPECTED + FileStorage::INSIDE_MAP;
    return fs;
}

CV_EXPORTS FileStorage& operator << (FileStorage& fs, const String& str);

static inline FileStorage& operator << (FileStorage& fs, const char* str)
{ return (fs << String(str)); }

inline FileNode FileStorage::operator[](const String& nodename) const
{
    return FileNode(fs.obj, cvGetFileNodeByName(fs.obj, 0, nodename.c_str()));
}
inline FileNode FileStorage::operator[](const char* nodename) const
{
    return FileNode(fs.obj, cvGetFileNodeByName(fs.obj, 0, nodename));
}

inline FileNode::FileNode() : fs(0), node(0) {}
inline FileNode::FileNode(const CvFileStorage* _fs, const CvFileNode* _node)
    : fs(_fs), node(_node) {}

inline FileNode::FileNode(const FileNode& _node) : fs(_node.fs), node(_node.node) {}
inline FileNode FileNode::operator[](const String& nodename) const
{
    return FileNode(fs, cvGetFileNodeByName(fs, node, nodename.c_str()));
}
inline FileNode FileNode::operator[](const char* nodename) const
{
    return FileNode(fs, cvGetFileNodeByName(fs, node, nodename));
}

inline FileNode FileNode::operator[](int i) const
{
    return isSeq() ? FileNode(fs, (CvFileNode*)cvGetSeqElem(node->data.seq, i)) :
        i == 0 ? *this : FileNode();
}

inline int FileNode::type() const { return !node ? NONE : (node->tag & TYPE_MASK); }
inline bool FileNode::isNone() const { return type() == NONE; }
inline bool FileNode::isSeq() const { return type() == SEQ; }
inline bool FileNode::isMap() const { return type() == MAP; }
inline bool FileNode::isInt() const { return type() == INT; }
inline bool FileNode::isReal() const { return type() == REAL; }
inline bool FileNode::isString() const { return type() == STR; }
inline bool FileNode::isNamed() const { return !node ? false : (node->tag & NAMED) != 0; }
inline String FileNode::name() const
{
    const char* str;
    return !node || (str = cvGetFileNodeName(node)) == 0 ? String() : String(str);
}
inline size_t FileNode::count() const
{
    int t = type();
    return t == MAP ? ((CvSet*)node->data.map)->active_count :
        t == SEQ ? node->data.seq->total : node != 0;
}

static inline void read(const FileNode& node, bool& value, bool default_value)
{ value = cvReadInt(node.node, default_value) != 0; }

static inline void read(const FileNode& node, uchar& value, uchar default_value)
{ value = saturate_cast<uchar>(cvReadInt(node.node, default_value)); }

static inline void read(const FileNode& node, schar& value, schar default_value)
{ value = saturate_cast<schar>(cvReadInt(node.node, default_value)); }

static inline void read(const FileNode& node, ushort& value, ushort default_value)
{ value = saturate_cast<ushort>(cvReadInt(node.node, default_value)); }

static inline void read(const FileNode& node, short& value, short default_value)
{ value = saturate_cast<short>(cvReadInt(node.node, default_value)); }

static inline void read(const FileNode& node, int& value, int default_value)
{ value = cvReadInt(node.node, default_value); }

static inline void read(const FileNode& node, float& value, float default_value)
{ value = (float)cvReadReal(node.node, default_value); }

static inline void read(const FileNode& node, double& value, double default_value)
{ value = cvReadReal(node.node, default_value); }

static inline void read(const FileNode& node, String& value, const String& default_value)
{ value = String(cvReadString(node.node, default_value.c_str())); }

inline FileNode::operator int() const
{
    return cvReadInt(node, 0);
}
inline FileNode::operator float() const
{
    return (float)cvReadReal(node, 0);
}
inline FileNode::operator double() const
{
    return cvReadReal(node, 0);
}
inline FileNode::operator String() const
{
    return String(cvReadString(node, ""));
}

inline void FileNode::readRaw( const String& fmt, Vector<uchar>& vec ) const
{
    begin().readRaw( fmt, vec );
}

template<typename _Tp, int numflag> struct CV_EXPORTS VecReaderProxy
{
    VecReaderProxy( FileNodeIterator* _it ) : it(_it) {}
    void operator()(Vector<_Tp>& vec, size_t count) const
    {
        count = std::min(count, it->remaining);
        vec.resize(count);
        for( size_t i = 0; i < count; i++, ++(*it) )
            read(**it, vec[i], _Tp());
    }
    FileNodeIterator* it;
};

template<typename _Tp> struct CV_EXPORTS VecReaderProxy<_Tp,1>
{
    VecReaderProxy( FileNodeIterator* _it ) : it(_it) {}
    void operator()(Vector<_Tp>& vec, size_t count) const
    {
        size_t remaining = it->remaining, cn = DataType<_Tp>::channels;
        int _fmt = DataType<_Tp>::fmt;
        char fmt[] = { (char)((_fmt>>8)+'1'), (char)_fmt, '\0' };
        count = std::min(count, remaining/cn);
        vec.resize(count);
        Vector<uchar> _vec((uchar*)&vec[0], count*sizeof(_Tp), false);
        it->readRaw( String(fmt), _vec, count );
    }
    FileNodeIterator* it;
};

template<typename _Tp> static inline void
read( FileNodeIterator& it, Vector<_Tp>& vec, size_t maxCount=(size_t)INT_MAX )
{
    VecReaderProxy<_Tp, DataType<_Tp>::fmt != 0> r(&it);
    r(vec, maxCount);
}

template<typename _Tp> static inline void
read( FileNode& node, Vector<_Tp>& vec, const Vector<_Tp>& /*default_value*/ )
{
    read( node.begin(), vec );
}

inline FileNodeIterator FileNode::begin() const
{
    return FileNodeIterator(fs, node);
}

inline FileNodeIterator FileNode::end() const
{
    return FileNodeIterator(fs, node, count());
}

inline FileNode FileNodeIterator::operator *() const
{ return FileNode(fs, (const CvFileNode*)reader.ptr); }

inline FileNode FileNodeIterator::operator ->() const
{ return FileNode(fs, (const CvFileNode*)reader.ptr); }

template<typename _Tp> static inline FileNodeIterator& operator >> (FileNodeIterator& it, _Tp& value)
{ read( *it, value, _Tp()); return ++it; }

template<typename _Tp> static inline
FileNodeIterator& operator >> (FileNodeIterator& it, Vector<_Tp>& vec)
{
    VecReaderProxy<_Tp, DataType<_Tp>::fmt != 0> r(&it);
    r(vec, (size_t)INT_MAX);
    return it;
}

template<typename _Tp> static inline void operator >> (const FileNode& n, _Tp& value)
{ FileNodeIterator it = n.begin(); it >> value; }

static inline bool operator == (const FileNodeIterator& it1, const FileNodeIterator& it2)
{
    return it1.fs == it2.fs && it1.container == it2.container &&
        it1.reader.ptr == it2.reader.ptr && it1.remaining == it2.remaining;
}

static inline bool operator != (const FileNodeIterator& it1, const FileNodeIterator& it2)
{
    return !(it1 == it2);
}

static inline int operator - (const FileNodeIterator& it1, const FileNodeIterator& it2)
{
    return it2.remaining - it1.remaining;
}

static inline bool operator < (const FileNodeIterator& it1, const FileNodeIterator& it2)
{
    return it1.remaining > it2.remaining;
}

    
//////////////////////////////////////// Various algorithms ////////////////////////////////////
    
template<typename _Tp> static inline _Tp gcd(_Tp a, _Tp b)
{
    if( a < b )
        std::swap(a, b);
    while( b > 0 )
    {
        _Tp r = a % b;
        a = b;
        b = r;
    }
    return a;
}

/****************************************************************************************\

  Generic implementation of QuickSort algorithm
  Use it as: Vector<_Tp> a; ... sort(a,<less_than_predictor>);

  The current implementation was derived from *BSD system qsort():

    * Copyright (c) 1992, 1993
    *  The Regents of the University of California.  All rights reserved.
    *
    * Redistribution and use in source and binary forms, with or without
    * modification, are permitted provided that the following conditions
    * are met:
    * 1. Redistributions of source code must retain the above copyright
    *    notice, this list of conditions and the following disclaimer.
    * 2. Redistributions in binary form must reproduce the above copyright
    *    notice, this list of conditions and the following disclaimer in the
    *    documentation and/or other materials provided with the distribution.
    * 3. All advertising materials mentioning features or use of this software
    *    must display the following acknowledgement:
    *  This product includes software developed by the University of
    *  California, Berkeley and its contributors.
    * 4. Neither the name of the University nor the names of its contributors
    *    may be used to endorse or promote products derived from this software
    *    without specific prior written permission.
    *
    * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
    * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
    * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
    * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
    * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
    * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
    * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
    * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
    * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
    * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
    * SUCH DAMAGE.

\****************************************************************************************/

template<typename _Tp, class _LT> void sort( Vector<_Tp>& vec, _LT LT=_LT() )
{
    int isort_thresh = 7;
    int sp = 0;

    struct
    {
        _Tp *lb;
        _Tp *ub;
    }
    stack[48];

    _Tp* array = &vec[0];
    int total = vec.size();

    if( total <= 1 )
        return;

    stack[0].lb = array;
    stack[0].ub = array + (total - 1);

    while( sp >= 0 )
    {
        _Tp* left = stack[sp].lb;
        _Tp* right = stack[sp--].ub;

        for(;;)
        {
            int i, n = (int)(right - left) + 1, m;
            _Tp* ptr;
            _Tp* ptr2;

            if( n <= isort_thresh )
            {
            insert_sort:
                for( ptr = left + 1; ptr <= right; ptr++ )
                {
                    for( ptr2 = ptr; ptr2 > left && LT(ptr2[0],ptr2[-1]); ptr2--)
                        std::swap( ptr2[0], ptr2[-1] );
                }
                break;
            }
            else
            {
                _Tp* left0;
                _Tp* left1;
                _Tp* right0;
                _Tp* right1;
                _Tp* pivot;
                _Tp* a;
                _Tp* b;
                _Tp* c;
                int swap_cnt = 0;

                left0 = left;
                right0 = right;
                pivot = left + (n/2);

                if( n > 40 )
                {
                    int d = n / 8;
                    a = left, b = left + d, c = left + 2*d;
                    left = LT(*a, *b) ? (LT(*b, *c) ? b : (LT(*a, *c) ? c : a))
                                      : (LT(*c, *b) ? b : (LT(*a, *c) ? a : c));

                    a = pivot - d, b = pivot, c = pivot + d;
                    pivot = LT(*a, *b) ? (LT(*b, *c) ? b : (LT(*a, *c) ? c : a))
                                      : (LT(*c, *b) ? b : (LT(*a, *c) ? a : c));

                    a = right - 2*d, b = right - d, c = right;
                    right = LT(*a, *b) ? (LT(*b, *c) ? b : (LT(*a, *c) ? c : a))
                                      : (LT(*c, *b) ? b : (LT(*a, *c) ? a : c));
                }

                a = left, b = pivot, c = right;
                pivot = LT(*a, *b) ? (LT(*b, *c) ? b : (LT(*a, *c) ? c : a))
                                   : (LT(*c, *b) ? b : (LT(*a, *c) ? a : c));
                if( pivot != left0 )
                {
                    std::swap( *pivot, *left0 );
                    pivot = left0;
                }
                left = left1 = left0 + 1;
                right = right1 = right0;

                for(;;)
                {
                    while( left <= right && !LT(*pivot, *left) )
                    {
                        if( !LT(*left, *pivot) )
                        {
                            if( left > left1 )
                                std::swap( *left1, *left );
                            swap_cnt = 1;
                            left1++;
                        }
                        left++;
                    }

                    while( left <= right && !LT(*right, *pivot) )
                    {
                        if( !LT(*pivot, *right) )
                        {
                            if( right < right1 )
                                std::swap( *right1, *right );
                            swap_cnt = 1;
                            right1--;
                        }
                        right--;
                    }

                    if( left > right )
                        break;
                    std::swap( *left, *right );
                    swap_cnt = 1;
                    left++;
                    right--;
                }

                if( swap_cnt == 0 )
                {
                    left = left0, right = right0;
                    goto insert_sort;
                }

                n = std::min( (int)(left1 - left0), (int)(left - left1) );
                for( i = 0; i < n; i++ )
                    std::swap( left0[i], left[i-n] );

                n = std::min( (int)(right0 - right1), (int)(right1 - right) );
                for( i = 0; i < n; i++ )
                    std::swap( left[i], right0[i-n+1] );
                n = (int)(left - left1);
                m = (int)(right1 - right);
                if( n > 1 )
                {
                    if( m > 1 )
                    {
                        if( n > m )
                        {
                            stack[++sp].lb = left0;
                            stack[sp].ub = left0 + n - 1;
                            left = right0 - m + 1, right = right0;
                        }
                        else
                        {
                            stack[++sp].lb = right0 - m + 1;
                            stack[sp].ub = right0;
                            left = left0, right = left0 + n - 1;
                        }
                    }
                    else
                        left = left0, right = left0 + n - 1;
                }
                else if( m > 1 )
                    left = right0 - m + 1, right = right0;
                else
                    break;
            }
        }
    }
}

template<typename _Tp> struct CV_EXPORTS LessThan
{
    bool operator()(const _Tp& a, const _Tp& b) const { return a < b; }
};

template<typename _Tp> struct CV_EXPORTS GreaterEq
{
    bool operator()(const _Tp& a, const _Tp& b) const { return a >= b; }
};

template<typename _Tp> struct CV_EXPORTS LessThanIdx
{
    LessThanIdx( const _Tp* _arr ) : arr(_arr) {}
    bool operator()(int a, int b) const { return arr[a] < arr[b]; }
    const _Tp* arr;
};

template<typename _Tp> struct CV_EXPORTS GreaterEqIdx
{
    GreaterEqIdx( const _Tp* _arr ) : arr(_arr) {}
    bool operator()(int a, int b) const { return arr[a] >= arr[b]; }
    const _Tp* arr;
};

    
// This function splits the input sequence or set into one or more equivalence classes and
// returns the vector of labels - 0-based class indexes for each element.
// predicate(a,b) returns true if the two sequence elements certainly belong to the same class.
//
// The algorithm is described in "Introduction to Algorithms"
// by Cormen, Leiserson and Rivest, the chapter "Data structures for disjoint sets"
template<typename _Tp, class _EqPredicate> int
partition( const Vector<_Tp>& _vec, Vector<int>& labels,
           _EqPredicate predicate=_EqPredicate())
{
    int i, j, N = (int)_vec.size();
    const _Tp* vec = &_vec[0];
    
    const int PARENT=0;
    const int RANK=1;
    
    Vector<int> _nodes(N*2);
    int (*nodes)[2] = (int(*)[2])&_nodes[0];
    
    // The first O(N) pass: create N single-vertex trees
    for(i = 0; i < N; i++)
    {
        nodes[i][PARENT]=-1;
        nodes[i][RANK] = 0;
    }
    
    // The main O(N^2) pass: merge connected components
    for( i = 0; i < N; i++ )
    {
        int root = i;
        
        // find root
        while( nodes[root][PARENT] >= 0 )
            root = nodes[root][PARENT];
        
        for( j = 0; j < N; j++ )
        {
            if( i == j || !predicate(vec[i], vec[j]))
                continue;
            int root2 = j;
            
            while( nodes[root2][PARENT] >= 0 )
                root2 = nodes[root2][PARENT];
            
            if( root2 != root )
            {
                // unite both trees
                int rank = nodes[root][RANK], rank2 = nodes[root2][RANK];
                if( rank > rank2 )
                    nodes[root2][PARENT] = root;
                else
                {
                    nodes[root][PARENT] = root2;
                    nodes[root2][RANK] += rank == rank2;
                    root = root2;
                }
                assert( nodes[root][PARENT] < 0 );
                
                int k = j, parent;
                
                // compress the path from node2 to root
                while( (parent = nodes[k][PARENT]) >= 0 )
                {
                    nodes[k][PARENT] = root;
                    k = parent;
                }
                
                // compress the path from node to root
                k = i;
                while( (parent = nodes[k][PARENT]) >= 0 )
                {
                    nodes[k][PARENT] = root;
                    k = parent;
                }
            }
        }
    }
    
    // Final O(N) pass: enumerate classes
    labels.resize(N);
    int nclasses = 0;
    
    for( i = 0; i < N; i++ )
    {
        int root = i;
        while( nodes[root][PARENT] >= 0 )
            root = nodes[root][PARENT];
        // re-use the rank as the class label
        if( nodes[root][RANK] >= 0 )
            nodes[root][RANK] = ~nclasses++;
        labels[i] = ~nodes[root][RANK];
    }
    
    return nclasses;
}

//////////////////////////////////////////////////////////////////////////////

template<typename _Tp> inline Seq<_Tp>::Seq() : seq(0) {}
template<typename _Tp> inline Seq<_Tp>::Seq( const CvSeq* _seq ) : seq((CvSeq*)_seq)
{
    CV_Assert(!_seq || _seq->elem_size == sizeof(_Tp));
}

template<typename _Tp> inline Seq<_Tp>::Seq( const MemStorage& storage,
                                             int headerSize )
{
    CV_Assert(headerSize >= sizeof(CvSeq));
    seq = cvCreateSeq(DataType<_Tp>::type, headerSize, sizeof(_Tp), storage.obj);
}

template<typename _Tp> inline _Tp& Seq<_Tp>::operator [](int idx)
{ return *(_Tp*)cvGetSeqElem(seq, idx); }

template<typename _Tp> inline const _Tp& Seq<_Tp>::operator [](int idx) const
{ return *(_Tp*)cvGetSeqElem(seq, idx); }

template<typename _Tp> inline SeqIterator<_Tp> Seq<_Tp>::begin() const
{ return SeqIterator<_Tp>(*this); }

template<typename _Tp> inline SeqIterator<_Tp> Seq<_Tp>::end() const
{ return SeqIterator<_Tp>(*this, true); }

template<typename _Tp> inline size_t Seq<_Tp>::size() const
{ return seq ? seq->total : 0; }

template<typename _Tp> inline int Seq<_Tp>::type() const
{ return seq ? CV_MAT_TYPE(seq->flags) : 0; }

template<typename _Tp> inline int Seq<_Tp>::depth() const
{ return seq ? CV_MAT_DEPTH(seq->flags) : 0; }

template<typename _Tp> inline int Seq<_Tp>::channels() const
{ return seq ? CV_MAT_CN(seq->flags) : 0; }

template<typename _Tp> inline size_t Seq<_Tp>::elemSize() const
{ return seq ? seq->elem_size : 0; }

template<typename _Tp> inline size_t Seq<_Tp>::index(const _Tp& elem) const
{ return cvSeqElemIdx(seq, &elem); }

template<typename _Tp> inline void Seq<_Tp>::push_back(const _Tp& elem)
{ cvSeqPush(seq, &elem); }

template<typename _Tp> inline void Seq<_Tp>::push_front(const _Tp& elem)
{ cvSeqPushFront(seq, &elem); }

template<typename _Tp> inline _Tp& Seq<_Tp>::back()
{ return *(_Tp*)cvGetSeqElem(seq, -1); }

template<typename _Tp> inline const _Tp& Seq<_Tp>::back() const
{ return *(const _Tp*)cvGetSeqElem(seq, -1); }

template<typename _Tp> inline _Tp& Seq<_Tp>::front()
{ return *(_Tp*)cvGetSeqElem(seq, 0); }

template<typename _Tp> inline const _Tp& Seq<_Tp>::front() const
{ return *(const _Tp*)cvGetSeqElem(seq, 0); }

template<typename _Tp> inline bool Seq<_Tp>::empty() const
{ return !seq || seq->total == 0; }

template<typename _Tp> inline void Seq<_Tp>::clear()
{ if(seq) cvClearSeq(seq); }

template<typename _Tp> inline void Seq<_Tp>::pop_back()
{ cvSeqPop(seq); }

template<typename _Tp> inline void Seq<_Tp>::pop_front()
{ cvSeqPopFront(seq); }

template<typename _Tp> inline void Seq<_Tp>::copyTo(Vector<_Tp>& vec, const Range& range) const
{
    size_t len = !seq ? 0 : range == Range::all() ? seq->total : range.end - range.start;
    vec.resize(len);
    if( seq )
        cvCvtSeqToArray(seq, &vec[0], cvSlice(range.start, range.end));
}

template<typename _Tp> inline Seq<_Tp>::operator Vector<_Tp>() const
{
    Vector<_Tp> vec;
    copyTo(vec);
    return vec;
}

template<typename _Tp> inline SeqIterator<_Tp>::SeqIterator()
{ memset(this, 0, sizeof(*this)); }

template<typename _Tp> inline SeqIterator<_Tp>::SeqIterator(const Seq<_Tp>& seq, bool seekEnd)
{
    cvStartReadSeq(seq.seq, this);
    if( seekEnd )
        index = seq.seq->total;
}

template<typename _Tp> inline void SeqIterator<_Tp>::seek(size_t pos)
{
    cvSetSeqReaderPos(this, (int)pos, false);
    index = pos;
}

template<typename _Tp> inline size_t SeqIterator<_Tp>::tell() const
{ return index; }

template<typename _Tp> inline _Tp& SeqIterator<_Tp>::operator *()
{ return *(_Tp*)ptr; }

template<typename _Tp> inline const _Tp& SeqIterator<_Tp>::operator *() const
{ return *(const _Tp*)ptr; }

template<typename _Tp> inline SeqIterator<_Tp>& SeqIterator<_Tp>::operator ++()
{
    CV_NEXT_SEQ_ELEM(sizeof(_Tp), *this);
    if( ++index >= seq->total*2 )
        index = 0;
    return *this;
}

template<typename _Tp> inline SeqIterator<_Tp> SeqIterator<_Tp>::operator ++(int) const
{
    SeqIterator<_Tp> it = *this;
    ++*this;
    return it;
}

template<typename _Tp> inline SeqIterator<_Tp>& SeqIterator<_Tp>::operator --()
{
    CV_PREV_SEQ_ELEM(sizeof(_Tp), *this);
    if( --index < 0 )
        index = seq->total*2-1;
    return *this;
}

template<typename _Tp> inline SeqIterator<_Tp> SeqIterator<_Tp>::operator --(int) const
{
    SeqIterator<_Tp> it = *this;
    --*this;
    return it;
}

template<typename _Tp> inline SeqIterator<_Tp>& SeqIterator<_Tp>::operator +=(int delta)
{
    cvSetSeqReaderPos(this, delta, 1);
    index += delta;
    int n = seq->total*2;
    if( index < 0 )
        index += n;
    if( index >= n )
        index -= n;
    return *this;
}

template<typename _Tp> inline SeqIterator<_Tp>& SeqIterator<_Tp>::operator -=(int delta)
{
    return (*this += -delta);
}

template<typename _Tp> inline ptrdiff_t operator - (const SeqIterator<_Tp>& a,
                                                    const SeqIterator<_Tp>& b)
{
    ptrdiff_t delta = a.index - b.index, n = a.seq->total;
    if( std::abs(delta) > n )
        delta += delta < 0 ? n : -n;
    return delta;
}

template<typename _Tp> inline bool operator == (const SeqIterator<_Tp>& a,
                                                const SeqIterator<_Tp>& b)
{
    return a.seq == b.seq && a.index == b.index;
}

template<typename _Tp> inline bool operator != (const SeqIterator<_Tp>& a,
                                                const SeqIterator<_Tp>& b)
{
    return !(a == b);
}

}

#endif
#endif
