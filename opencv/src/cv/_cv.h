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

#ifndef __CV_H_
#define __CV_H_

#if _MSC_VER >= 1200
    /* disable warnings related to inline functions */
    #pragma warning( disable: 4711 4710 4514 )
#endif

typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned long ulong;

#ifdef __BORLANDC__
    #define     WIN32
    #define     CV_DLL
    #undef      _CV_ALWAYS_PROFILE_
    #define     _CV_ALWAYS_NO_PROFILE_
#endif

#define CV_IMPL CV_EXTERN_C

#ifndef IPCVAPI
#define IPCVAPI(type,name,arg)                                \
    CV_EXTERN_C type CV_STDCALL name##_f arg;                 \
    /* function pointer */                                    \
    typedef type (CV_STDCALL* name##_t) arg;                  \
    extern name##_t name;
#endif

#ifndef IPCVAPI_IMPL
#define IPCVAPI_IMPL(type,name,arg)                           \
    /*typedef type (CV_STDCALL* name##_t) arg;*/              \
    CV_EXTERN_C type CV_STDCALL name##_f arg;                 \
    name##_t name = name##_f;                                 \
    CV_EXTERN_C type CV_STDCALL name##_f arg
#endif

#include "cv.h"

#include <math.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <float.h>

/* get alloca declaration */
#ifdef WIN32
    #if defined _MSC_VER || defined __BORLANDC__
        #include <malloc.h>
    #endif
#else
    #include <alloca.h>
#endif

#include "_cvoptions.h"
#include "_cvtables.h"
#include "_cverror.h"

typedef enum {
   cv1u,
   cv8u, cv8s,
   cv16u, cv16s, cv16sc,
   cv32u, cv32s, cv32sc,
   cv32f, cv32fc,
   cv64u, cv64s, cv64sc,
   cv64f, cv64fc
} CvDataType;

CV_EXTERN_C_FUNCPTR( void (CV_CDECL * ICVWriteNodeFunction)(void*,void*) )

#define ICV_KERNEL_TYPE_MASK        (15<<16)
#define ICV_BINARY_KERNEL           (0<<16)
#define ICV_SEPARABLE_KERNEL        (1<<16)

#define ICV_KERNEL_TYPE(flags)      ((flags) & ICV_KERNEL_TYPE_MASK)

#define ICV_MAKE_SEPARABLE_KERNEL( x_type, y_type ) \
    (ICV_SEPARABLE_KERNEL | ((x_type)&255) | (((y_type)&255) << 8))

#define ICV_X_KERNEL_TYPE(flags)    ((flags) & 255)
#define ICV_Y_KERNEL_TYPE(flags)    (((flags) >> 8) & 255)
#define ICV_SYMMETRIC_KERNEL        1
#define ICV_ASYMMETRIC_KERNEL       2

#define ICV_1_2_1_KERNEL            (4*1+ICV_SYMMETRIC_KERNEL)
#define ICV_m1_0_1_KERNEL           (4*2+ICV_ASYMMETRIC_KERNEL)
#define ICV_1_m2_1_KERNEL           (4*3+ICV_SYMMETRIC_KERNEL)
#define ICV_3_10_3_KERNEL           (4*4+ICV_SYMMETRIC_KERNEL)

#define ICV_MAKE_BINARY_KERNEL( shape ) \
    (ICV_BINARY_KERNEL | (int)(shape))

#define ICV_BINARY_KERNEL_SHAPE(flags) ((CvElementShape)((flags) & 255))

typedef struct CvFilterState
{
    /* kernel data */
    int ker_width;
    int ker_height;
    int ker_x;
    int ker_y;
    int kerType;
    uchar *ker0;
    uchar *ker1;
    double divisor;

    /* image data */
    int max_width;
    CvDataType dataType;
    int channels;
    int origin;

    /* cyclic buffer */
    char *buffer;
    int buffer_step;
    int crows;
    char **rows;
    char *tbuf;
}
CvFilterState;

#define _CvConvState CvFilterState
#define CvMorphState CvFilterState

#define  CV_COPY( dst, src, len, idx ) \
    for( (idx) = 0; (idx) < (len); (idx)++) (dst)[idx] = (src)[idx]

#define  CV_SET( dst, val, len, idx )  \
    for( (idx) = 0; (idx) < (len); (idx)++) (dst)[idx] = (val)


typedef struct
{
    uchar  b, g, r;
}
CvRGB8u;


typedef struct
{
    uchar  b, g, r, a;
}
CvRGBA8u;


typedef struct
{
    int  b, g, r;
}
CvRGB32s;


typedef struct
{
    int  b, g, r, a;
}
CvRGBA32s;

typedef struct
{
    float  b, g, r;
}
CvRGB32f;


typedef struct
{
    float  b, g, r, a;
}
CvRGBA32f;


#undef   CV_CALC_MAX
#undef   CV_CALC_MIN

#define  CV_CALC_MIN(a, b) (a) = CV_IMIN((a),(b))
#define  CV_CALC_MAX(a, b) (a) = CV_IMAX((a),(b))

#define CV_MORPH_ALIGN  4

typedef CvStatus( CV_STDCALL* CvFilterFunc )( const void* src, int src_step,
                                              void* dst, int dst_step,
                                              CvSize* size, struct CvFilterState * state,
                                              int stage );

#define CvMorphFunc CvFilterFunc

typedef struct CvFuncTable CvFuncTable;
typedef struct CvBigFuncTable CvBigFuncTable;

#include "_ipcv.h"
#include "_optcv.h"
#include "_cvutils.h"
#include "_cvarr.h"
#include "_cvmatrix.h"

#if _MSC_VER >= 1200
    #define CV_FORCE_INLINE  __forceinline
#else
    #define CV_FORCE_INLINE  CV_INLINE
#endif

#if defined _MSC_VER || defined __BORLANDC__ || defined __ICL
    #define CV_BIG_INT(n)   n##I64
#else
    #define CV_BIG_INT(n)   n##LL
#endif

#define  sizeof_float ((int)sizeof(float))
#define  sizeof_short ((int)sizeof(float))

#define  CV_ORIGIN_TL  0
#define  CV_ORIGIN_BL  1

#define  CV_POS_INF       0x7f800000
#define  CV_NEG_INF       CV_TOGGLE_FLT(0xff800000)
#define  CV_TOGGLE_FLT(x) (((x)&0x7fffffff)^(((int)(x) < 0 ? -1 : 0)))

#define  CV_TOGGLE_DBL(x) \
    (((x)&CV_BIG_INT(0x7fffffffffffffff))^(((int64)(x) < 0 ? -1 : 0)))

#define  CV_PI   3.1415926535897932384626433832795

/* IEEE 754 representation of 1.f */
#define  CV_1F          0x3f800000

#define  CV_NOP(a)      (a)
#define  CV_ADD(a, b)   ((a) + (b))
#define  CV_SUB(a, b)   ((a) - (b))
#define  CV_MUL(a, b)   ((a) * (b))
#define  CV_AND(a, b)   ((a) & (b))
#define  CV_OR(a, b)    ((a) | (b))
#define  CV_XOR(a, b)   ((a) ^ (b))
#define  CV_ANDN(a, b)  (~(a) & (b))
#define  CV_ORN(a, b)   (~(a) | (b))
#define  CV_SQR(a)      ((a) * (a))
#define  CV_MIN(a, b)   ((a) <= (b) ? (a) : (b))
#define  CV_MAX(a, b)   ((a) >= (b) ? (a) : (b))

// (a) < (b) ? -1 : (a) > (b)
#define  CV_CMP(a, b)   ((((a)>=(b))-1)|((a)>(b)))

#define  CV_LT(a, b)    ((a) < (b))
#define  CV_LE(a, b)    ((a) <= (b))
#define  CV_EQ(a, b)    ((a) == (b))
#define  CV_NE(a, b)    ((a) != (b))
#define  CV_GT(a, b)    ((a) > (b))
#define  CV_GE(a, b)    ((a) >= (b))

#define  CV_NONZERO(a)      ((a) != 0)
#define  CV_NONZERO_FLT(a)  (((a)+(a)) != 0)

#define  CV_EMPTY

#define  CV_DEFINE_MASK         \
    float maskTab[2]; maskTab[0] = 0.f; maskTab[1] = 1.f;
#define  CV_ANDMASK( m, x )     ((x) & (((m) == 0) - 1))

// (x) * ((m) == 1 ? 1.f : (m) == 0 ? 0.f : <ERR>
#define  CV_MULMASK( m, x )       (maskTab[(m) != 0]*(x))

// (x) * ((m) == -1 ? 1.f : (m) == 0 ? 0.f : <ERR>
#define  CV_MULMASK1( m, x )      (maskTab[(m)+1]*(x))


#define CV_ZERO_OBJ(x)  memset((x), 0, sizeof(*(x)))


#define  ICV_UN_ENTRY_C1(worktype)           \
    worktype s0 = scalar[0]
    
#define  ICV_UN_ENTRY_C2(worktype)           \
    worktype s0 = scalar[0], s1 = scalar[1]

#define  ICV_UN_ENTRY_C3(worktype)           \
    worktype s0 = scalar[0], s1 = scalar[1], s2 = scalar[2]

#define  ICV_UN_ENTRY_C4(worktype)           \
    worktype s0 = scalar[0], s1 = scalar[1], s2 = scalar[2], s3 = scalar[3]

CV_INLINE void* icvAlignPtr( void* ptr, int align = 32 );
CV_INLINE void* icvAlignPtr( void* ptr, int align )
{
    return (void*)( ((long)ptr + align - 1) & -align );
}

CV_INLINE int icvAlign( int size, int align );
CV_INLINE int icvAlign( int size, int align )
{
    return (size + align - 1) & -align;
}


#define CV_WHOLE   0
#define CV_START   1
#define CV_END     2
#define CV_MIDDLE  4

#ifdef IPCVAPI_DEFINED
    #undef IPCVAPI
    #undef IPCVAPI_DEFINED
#endif

void icvCheckImageHeader( const IplImage* image, const char* img_name );
void icvCheckMaskImageHeader( const IplImage* image, const char* img_name );
#define CV_CHECK_IMAGE( img ) icvCheckImageHeader( img, #img )
#define CV_CHECK_MASK_IMAGE( img ) icvCheckMaskImageHeader( img, #img )

CvTermCriteria icvCheckTermCriteria( CvTermCriteria criteria,
                                     double default_eps, int max_iters );

CV_INLINE bool icvIsRectInRect( CvRect subrect, CvRect mainrect );
CV_INLINE bool icvIsRectInRect( CvRect subrect, CvRect mainrect )
{
    return  subrect.x >= mainrect.x && subrect.y >= mainrect.y &&
            subrect.x + subrect.width <= mainrect.x + mainrect.width &&
            subrect.y + subrect.height <= mainrect.y + mainrect.height;
}

CV_INLINE int icvGetBtPix( const IplImage* image );
CV_INLINE int icvGetBtPix( const IplImage* image )
{
    return  ((image->depth & 255)>>3)*image->nChannels;
}

CV_INLINE bool operator == (CvSize size1, CvSize size2 );
CV_INLINE bool operator == (CvSize size1, CvSize size2 )
{
    return size1.width == size2.width && size1.height == size2.height;
}

CV_INLINE bool operator != (CvSize size1, CvSize size2 );
CV_INLINE bool operator != (CvSize size1, CvSize size2 )
{
    return size1.width != size2.width || size1.height != size2.height;
}

CV_INLINE int icvGetImageCOI( const IplImage* image );
CV_INLINE int icvGetImageCOI( const IplImage* image )
{
    return image->roi ? image->roi->coi : 0;
}

CV_INLINE IplROI icvMakeROI( int x, int y, int width, int height, int coi = 0 );
CV_INLINE IplROI icvMakeROI( int x, int y, int width, int height, int coi )
{
    IplROI roi = { coi, x, y, width, height };
    return roi;
}

#endif /*__CV_H_*/
