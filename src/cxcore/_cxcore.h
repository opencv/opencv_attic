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

#ifndef _CXCORE_INTERNAL_H_
#define _CXCORE_INTERNAL_H_

#if _MSC_VER >= 1200
    /* disable warnings related to inline functions */
    #pragma warning( disable: 4711 4710 4514 )
#endif

typedef unsigned long ulong;

#ifdef __BORLANDC__
    #define     WIN32
    #define     CV_DLL
    #undef      _CV_ALWAYS_PROFILE_
    #define     _CV_ALWAYS_NO_PROFILE_
#endif

#include "cxcore.h"
#include "cxmisc.h"
#include "_cxipp.h"
#include <math.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <float.h>

// -128.f ... 255.f
extern const float icv8x32fTab[];
#define CV_8TO32F(x)  icv8x32fTab[(x)+128]

extern const ushort icv8x16uSqrTab[];
#define CV_SQR_8U(x)  icv8x16uSqrTab[(x)+255]

extern const char* icvHersheyGlyphs[];

extern const int icvPixSize[];
extern const signed char icvDepthToType[];

#define icvIplToCvDepth( depth ) \
    icvDepthToType[(((depth) & 255) >> 2) + ((depth) < 0)]

typedef CvFunc2D_3A1I CvArithmBinMaskFunc2D;
typedef CvFunc2D_2A1P1I CvArithmUniMaskFunc2D;


/****************************************************************************************\
*                                   Complex arithmetics                                  *
\****************************************************************************************/

struct CvComplex32f;
struct CvComplex64f;

struct CvComplex32f
{
    float re, im;

    CvComplex32f() {}
    CvComplex32f( float _re, float _im=0 ) : re(_re), im(_im) {}
    explicit CvComplex32f( const CvComplex64f& v );
    //CvComplex32f( const CvComplex32f& v ) : re(v.re), im(v.im) {}
    //CvComplex32f& operator = (const CvComplex32f& v ) { re = v.re; im = v.im; return *this; }
    operator CvComplex64f() const;
};

struct CvComplex64f
{
    double re, im;

    CvComplex64f() {}
    CvComplex64f( double _re, double _im=0 ) : re(_re), im(_im) {}
    explicit CvComplex64f( const CvComplex32f& v );
    //CvComplex64f( const CvComplex64f& v ) : re(v.re), im(v.im) {}
    //CvComplex64f& operator = (const CvComplex64f& v ) { re = v.re; im = v.im; return *this; }
    operator CvComplex32f() const;
};

inline CvComplex32f::CvComplex32f( const CvComplex64f& v ) : re((float)v.re), im((float)v.im) {}
inline CvComplex64f::CvComplex64f( const CvComplex32f& v ) : re(v.re), im(v.im) {}

inline CvComplex32f operator + (CvComplex32f a, CvComplex32f b)
{
    return CvComplex32f( a.re + b.re, a.im + b.im );
}

inline CvComplex32f& operator += (CvComplex32f& a, CvComplex32f b)
{
    a.re += b.re;
    a.im += b.im;
    return a;
}

inline CvComplex32f operator - (CvComplex32f a, CvComplex32f b)
{
    return CvComplex32f( a.re - b.re, a.im - b.im );
}

inline CvComplex32f& operator -= (CvComplex32f& a, CvComplex32f b)
{
    a.re -= b.re;
    a.im -= b.im;
    return a;
}

inline CvComplex32f operator - (CvComplex32f a)
{
    return CvComplex32f( -a.re, -a.im );
}

inline CvComplex32f operator * (CvComplex32f a, CvComplex32f b)
{
    return CvComplex32f( a.re*b.re - a.im*b.im, a.re*b.im + a.im*b.re );
}

inline double abs(CvComplex32f a)
{
    return sqrt( (double)a.re*a.re + (double)a.im*a.im );
}

inline CvComplex32f conj(CvComplex32f a)
{
    return CvComplex32f( a.re, -a.im );
}


inline CvComplex32f operator / (CvComplex32f a, CvComplex32f b)
{
    double t = 1./((double)b.re*b.re + (double)b.im*b.im);
    return CvComplex32f( (float)((a.re*b.re + a.im*b.im)*t),
                         (float)((-a.re*b.im + a.im*b.re)*t) );
}

inline CvComplex32f operator * (double a, CvComplex32f b)
{
    return CvComplex32f( (float)(a*b.re), (float)(a*b.im) );
}

inline CvComplex32f operator * (CvComplex32f a, double b)
{
    return CvComplex32f( (float)(a.re*b), (float)(a.im*b) );
}

inline CvComplex32f::operator CvComplex64f() const
{
    return CvComplex64f(re,im);
}


inline CvComplex64f operator + (CvComplex64f a, CvComplex64f b)
{
    return CvComplex64f( a.re + b.re, a.im + b.im );
}

inline CvComplex64f& operator += (CvComplex64f& a, CvComplex64f b)
{
    a.re += b.re;
    a.im += b.im;
    return a;
}

inline CvComplex64f operator - (CvComplex64f a, CvComplex64f b)
{
    return CvComplex64f( a.re - b.re, a.im - b.im );
}

inline CvComplex64f& operator -= (CvComplex64f& a, CvComplex64f b)
{
    a.re -= b.re;
    a.im -= b.im;
    return a;
}

inline CvComplex64f operator - (CvComplex64f a)
{
    return CvComplex64f( -a.re, -a.im );
}

inline CvComplex64f operator * (CvComplex64f a, CvComplex64f b)
{
    return CvComplex64f( a.re*b.re - a.im*b.im, a.re*b.im + a.im*b.re );
}

inline double abs(CvComplex64f a)
{
    return sqrt( (double)a.re*a.re + (double)a.im*a.im );
}

inline CvComplex64f operator / (CvComplex64f a, CvComplex64f b)
{
    double t = 1./((double)b.re*b.re + (double)b.im*b.im);
    return CvComplex64f( (a.re*b.re + a.im*b.im)*t,
                         (-a.re*b.im + a.im*b.re)*t );
}

inline CvComplex64f operator * (double a, CvComplex64f b)
{
    return CvComplex64f( a*b.re, a*b.im );
}

inline CvComplex64f operator * (CvComplex64f a, double b)
{
    return CvComplex64f( a.re*b, a.im*b );
}

inline CvComplex64f::operator CvComplex32f() const
{
    return CvComplex32f((float)re,(float)im);
}

inline CvComplex64f conj(CvComplex64f a)
{
    return CvComplex64f( a.re, -a.im );
}

inline CvComplex64f operator + (CvComplex64f a, CvComplex32f b)
{
    return CvComplex64f( a.re + b.re, a.im + b.im );
}

inline CvComplex64f operator + (CvComplex32f a, CvComplex64f b)
{
    return CvComplex64f( a.re + b.re, a.im + b.im );
}

inline CvComplex64f operator - (CvComplex64f a, CvComplex32f b)
{
    return CvComplex64f( a.re - b.re, a.im - b.im );
}

inline CvComplex64f operator - (CvComplex32f a, CvComplex64f b)
{
    return CvComplex64f( a.re - b.re, a.im - b.im );
}

inline CvComplex64f operator * (CvComplex64f a, CvComplex32f b)
{
    return CvComplex64f( a.re*b.re - a.im*b.im, a.re*b.im + a.im*b.re );
}

inline CvComplex64f operator * (CvComplex32f a, CvComplex64f b)
{
    return CvComplex64f( a.re*b.re - a.im*b.im, a.re*b.im + a.im*b.re );
}


typedef CvStatus (CV_STDCALL * CvCopyMaskFunc)(const void* src, int src_step,
                                               void* dst, int dst_step, CvSize size,
                                               const void* mask, int mask_step);

CvCopyMaskFunc icvGetCopyMaskFunc( int elem_size );

CvStatus CV_STDCALL icvSetZero_8u_C1R( uchar* dst, int dststep, CvSize size );

#endif /*_CXCORE_INTERNAL_H_*/
