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

#ifndef _CV_IMG_PROC_H_
#define _CV_IMG_PROC_H_

#define ICV_KERNEL_TYPE_MASK        (15<<16)
#define ICV_GENERIC_KERNEL          (0<<16)
#define ICV_SEPARABLE_KERNEL        (1<<16)
#define ICV_BINARY_KERNEL           (2<<16)

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
#define ICV_DEFAULT_GAUSSIAN_KERNEL ICV_SYMMETRIC_KERNEL
#define ICV_CUSTOM_GAUSSIAN_KERNEL  (4+ICV_SYMMETRIC_KERNEL)

#define ICV_MAKE_BINARY_KERNEL( shape ) \
    (ICV_BINARY_KERNEL | (int)(shape))

#define ICV_BINARY_KERNEL_SHAPE(flags) ((flags) & 255)

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

#define  CV_COPY( dst, src, len, idx ) \
    for( (idx) = 0; (idx) < (len); (idx)++) (dst)[idx] = (src)[idx]

#define  CV_SET( dst, val, len, idx )  \
    for( (idx) = 0; (idx) < (len); (idx)++) (dst)[idx] = (val)

/* 
   Finds L1 norm between two blocks.
   !!!! function doesn't perform EMMS !!!!
*/
int  icvCmpBlocksL1_8u_C1( const uchar* vec1, const uchar* vec2, int len );

/* 
   Finds squared L2 norm between two blocks.
   !!!! function don't perform EMMS (except 32f flavor function) !!!!
*/
int64  icvCmpBlocksL2_8u_C1( const uchar* vec1, const uchar* vec2, int len );
double icvCmpBlocksL2_32f_C1( const float* vec1, const float* vec2, int len );


int64  icvSumPixels_8u_C1( const uchar* vec, int len );
double icvSumPixels_32f_C1( const float* vec, int len );

/* 
   Calculates cross correlation for two blocks.
   !!!! function doesn't perform EMMS !!!!
*/
int64  icvCrossCorr_8u_C1( const uchar* vec1, const uchar* vec2, int len );
double icvCrossCorr_32f_C1( const float* vec1, const float* vec2, int len );

/* performs convolution of 2d floating-point array with 3x1, 1x3 or separable 3x3 mask */
void icvSepConvSmall3_32f( float* src, int src_step, float* dst, int dst_step,
            CvSize src_size, const float* kx, const float* ky, float* buffer );

CvStatus CV_STDCALL icvPyrDownBorder_8u_CnR( const uchar *src, int src_step, CvSize src_size,
                                             uchar *dst, int dst_step, CvSize dst_size, int cn );

CvStatus CV_STDCALL icvPyrDownBorder_32f_CnR( const float *src, int src_step, CvSize src_size,
                                              float *dst, int dst_step, CvSize dst_size, int cn );

CvStatus CV_STDCALL icvPyrDownBorder_64f_CnR( const double *src, int src_step, CvSize src_size,
                                              double *dst, int dst_step, CvSize dst_size, int cn );

CvStatus CV_STDCALL icvFilterInitAlloc( int roiWidth, CvDataType dataType, int channels,
                    CvSize elSize, CvPoint elAnchor, const void* elData, int elementFlags,
                    struct CvFilterState ** morphState );

CvStatus CV_STDCALL icvFilterFree( struct CvFilterState ** morphState );

CvStatus CV_STDCALL icvSobelInitAlloc( int roiwidth, int depth, int kerSize,
                                       int origin, int dx, int dy,
                                       struct _CvConvState** state );

CvStatus CV_STDCALL icvSobel_8u16s_C1R( const uchar* pSrc, int srcStep,
                                        short* pDst, int dstStep, CvSize* roiSize,
                                        struct _CvConvState* state, int stage );

CvStatus CV_STDCALL icvSobel_32f_C1R( const float* pSrc, int srcStep,
                                      float* pDst, int dstStep, CvSize* roiSize,
                                      struct _CvConvState* state, int stage );

CvStatus CV_STDCALL icvBlurInitAlloc( int roiWidth, int depth, int kerSize,
                                      struct _CvConvState** state );

CvStatus CV_STDCALL icvBlur_8u16s_C1R( const uchar* pSrc, int srcStep,
                                       short* pDst, int dstStep, CvSize* roiSize,
                                       struct _CvConvState* state, int stage );

CvStatus CV_STDCALL icvBlur_32f_CnR( const float* pSrc, int srcStep,
                                     float* pDst, int dstStep, CvSize* roiSize,
                                     struct _CvConvState* state, int stage );

#define icvBlur_32f_C1R icvBlur_32f_CnR

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

#define CV_WHOLE   0
#define CV_START   1
#define CV_END     2
#define CV_MIDDLE  4

#endif /*_CV_INTERNAL_H_*/
