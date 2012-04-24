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

#include "_cxcore.h"

/****************************************************************************************\
*                                    LUT Transform                                       *
\****************************************************************************************/

#define  ICV_DEF_LUT_FUNC_8U_C1( flavor, dsttype )                  \
CvStatus CV_STDCALL                                                 \
icvLUT_Transform8u_##flavor##_C1R( const void* srcptr, int srcstep, \
                           void* dstptr, int dststep, CvSize size,  \
                           const void* lutptr )                     \
{                                                                   \
    const uchar* src = (const uchar*)srcptr;                        \
    dsttype* dst = (dsttype*)dstptr;                                \
    const dsttype* lut = (const dsttype*)lutptr;                    \
                                                                    \
    for( ; size.height--; src += srcstep, (char*&)dst += dststep )  \
    {                                                               \
        int i;                                                      \
        for( i = 0; i <= size.width - 4; i += 4 )                   \
        {                                                           \
            dsttype t0 = lut[src[i]];                               \
            dsttype t1 = lut[src[i+1]];                             \
            dst[i] = t0;                                            \
            dst[i+1] = t1;                                          \
                                                                    \
            t0 = lut[src[i+2]];                                     \
            t1 = lut[src[i+3]];                                     \
            dst[i+2] = t0;                                          \
            dst[i+3] = t1;                                          \
        }                                                           \
                                                                    \
        for( ; i < size.width; i++ )                                \
        {                                                           \
            dsttype t0 = lut[src[i]];                               \
            dst[i] = t0;                                            \
        }                                                           \
    }                                                               \
                                                                    \
    return CV_OK;                                                   \
}


#define  ICV_DEF_LUT_FUNC_8U_C2( flavor, dsttype )                  \
CvStatus CV_STDCALL                                                 \
icvLUT_Transform8u_##flavor##_C2R( const void* srcptr, int srcstep, \
                           void* dstptr, int dststep, CvSize size,  \
                           const void* lutptr )                     \
{                                                                   \
    const uchar* src = (const uchar*)srcptr;                        \
    dsttype* dst = (dsttype*)dstptr;                                \
    const dsttype* lut = (const dsttype*)lutptr;                    \
    size.width *= 2;                                                \
                                                                    \
    for( ; size.height--; src += srcstep, (char*&)dst += dststep )  \
    {                                                               \
        int i;                                                      \
        for( i = 0; i < size.width; i += 2 )                        \
        {                                                           \
            dsttype t0 = lut[src[i]*2];                             \
            dsttype t1 = lut[src[i+1]*2 + 1];                       \
            dst[i] = t0;                                            \
            dst[i+1] = t1;                                          \
        }                                                           \
    }                                                               \
                                                                    \
    return CV_OK;                                                   \
}


#define  ICV_DEF_LUT_FUNC_8U_C3( flavor, dsttype )                  \
CvStatus CV_STDCALL                                                 \
icvLUT_Transform8u_##flavor##_C3R( const void* srcptr, int srcstep, \
                           void* dstptr, int dststep, CvSize size,  \
                           const void* lutptr )                     \
{                                                                   \
    const uchar* src = (const uchar*)srcptr;                        \
    dsttype* dst = (dsttype*)dstptr;                                \
    const dsttype* lut = (const dsttype*)lutptr;                    \
    size.width *= 3;                                                \
                                                                    \
    for( ; size.height--; src += srcstep, (char*&)dst += dststep )  \
    {                                                               \
        int i;                                                      \
        for( i = 0; i < size.width; i += 3 )                        \
        {                                                           \
            dsttype t0 = lut[src[i]*3];                             \
            dsttype t1 = lut[src[i+1]*3 + 1];                       \
            dsttype t2 = lut[src[i+2]*3 + 2];                       \
            dst[i] = t0;                                            \
            dst[i+1] = t1;                                          \
            dst[i+2] = t2;                                          \
        }                                                           \
    }                                                               \
                                                                    \
    return CV_OK;                                                   \
}


#define  ICV_DEF_LUT_FUNC_8U_C4( flavor, dsttype )                  \
CvStatus CV_STDCALL                                                 \
icvLUT_Transform8u_##flavor##_C4R( const void* srcptr, int srcstep, \
                           void* dstptr, int dststep, CvSize size,  \
                           const void* lutptr )                     \
{                                                                   \
    const uchar* src = (const uchar*)srcptr;                        \
    dsttype* dst = (dsttype*)dstptr;                                \
    const dsttype* lut = (const dsttype*)lutptr;                    \
    size.width *= 4;                                                \
                                                                    \
    for( ; size.height--; src += srcstep, (char*&)dst += dststep )  \
    {                                                               \
        int i;                                                      \
        for( i = 0; i < size.width; i += 4 )                        \
        {                                                           \
            dsttype t0 = lut[src[i]*4];                             \
            dsttype t1 = lut[src[i+1]*4 + 1];                       \
            dst[i] = t0;                                            \
            dst[i+1] = t1;                                          \
            t0 = lut[src[i+2]*4 + 2];                               \
            t1 = lut[src[i+3]*4 + 3];                               \
            dst[i+2] = t0;                                          \
            dst[i+3] = t1;                                          \
        }                                                           \
    }                                                               \
                                                                    \
    return CV_OK;                                                   \
}


ICV_DEF_LUT_FUNC_8U_C1( 8u, uchar )
ICV_DEF_LUT_FUNC_8U_C1( 16s, ushort )
ICV_DEF_LUT_FUNC_8U_C1( 32s, int )
ICV_DEF_LUT_FUNC_8U_C1( 64f, int64 )

ICV_DEF_LUT_FUNC_8U_C2( 8u, uchar )
static ICV_DEF_LUT_FUNC_8U_C2( 16s, ushort )
static ICV_DEF_LUT_FUNC_8U_C2( 32s, int )
static ICV_DEF_LUT_FUNC_8U_C2( 64f, int64 )
                    
ICV_DEF_LUT_FUNC_8U_C3( 8u, uchar )
static ICV_DEF_LUT_FUNC_8U_C3( 16s, ushort )
static ICV_DEF_LUT_FUNC_8U_C3( 32s, int )
static ICV_DEF_LUT_FUNC_8U_C3( 64f, int64 )
                    
ICV_DEF_LUT_FUNC_8U_C4( 8u, uchar )
static ICV_DEF_LUT_FUNC_8U_C4( 16s, ushort )
static ICV_DEF_LUT_FUNC_8U_C4( 32s, int )
static ICV_DEF_LUT_FUNC_8U_C4( 64f, int64 )

#define  icvLUT_Transform8u_8s_C2R    icvLUT_Transform8u_8u_C2R
#define  icvLUT_Transform8u_16u_C2R   icvLUT_Transform8u_16s_C2R
#define  icvLUT_Transform8u_32f_C2R   icvLUT_Transform8u_32s_C2R

#define  icvLUT_Transform8u_8s_C3R    icvLUT_Transform8u_8u_C3R
#define  icvLUT_Transform8u_16u_C3R   icvLUT_Transform8u_16s_C3R
#define  icvLUT_Transform8u_32f_C3R   icvLUT_Transform8u_32s_C3R

#define  icvLUT_Transform8u_8s_C4R    icvLUT_Transform8u_8u_C4R
#define  icvLUT_Transform8u_16u_C4R   icvLUT_Transform8u_16s_C4R
#define  icvLUT_Transform8u_32f_C4R   icvLUT_Transform8u_32s_C4R

CV_DEF_INIT_BIG_FUNC_TAB_2D( LUT_Transform8u, R )

CV_IMPL  void
cvLUT( const void* srcarr, void* dstarr, const void* lutarr )
{
    static CvBigFuncTable lut_tab;
    static int inittab = 0;

    CV_FUNCNAME( "cvLUT" );

    __BEGIN__;

    int  coi1 = 0, coi2 = 0;
    int  type, cn, lut_cn;
    CvMat  srcstub, *src = (CvMat*)srcarr;
    CvMat  dststub, *dst = (CvMat*)dstarr;
    CvMat  lutstub, *lut = (CvMat*)lutarr;
    uchar* lut_data;
    uchar* shuffled_lut = (uchar*)cvStackAlloc(256*4*sizeof(double));
    CvLUT_TransformFunc func;
    CvSize size;
    int src_step, dst_step;

    if( !inittab )
    {
        icvInitLUT_Transform8uRTable( &lut_tab );
        inittab = 1;
    }

    CV_CALL( src = cvGetMat( src, &srcstub, &coi1 ));
    CV_CALL( dst = cvGetMat( dst, &dststub, &coi2 ));
    CV_CALL( lut = cvGetMat( lut, &lutstub ));

    if( coi1 != 0 || coi2 != 0 )
        CV_ERROR( CV_BadCOI, "" );

    if( !CV_ARE_SIZES_EQ( src, dst ))
        CV_ERROR( CV_StsUnmatchedSizes, "" );

    if( !CV_ARE_CNS_EQ( src, dst ))
        CV_ERROR( CV_StsUnmatchedFormats, "" );

    if( CV_MAT_DEPTH( src->type ) > CV_8S )
        CV_ERROR( CV_StsUnsupportedFormat, "" );

    type = CV_MAT_TYPE( dst->type );
    cn = CV_MAT_CN( dst->type );
    lut_cn = CV_MAT_CN( lut->type );

    if( !CV_IS_MAT_CONT(lut->type) || (lut_cn != 1 && lut_cn != cn) ||
        !CV_ARE_DEPTHS_EQ( dst, lut ) || lut->width*lut->height != 256 )
        CV_ERROR( CV_StsBadArg, "The LUT must be continuous, single-channel array \n"
                                "with 256 elements of the same type as destination" );

    size = cvGetMatSize( src );
    if( lut_cn == 1 )
        size.width *= cn;
    src_step = src->step;
    dst_step = dst->step;

    if( CV_IS_MAT_CONT( src->type & dst->type ))
    {
        size.width *= size.height;
        src_step = dst_step = CV_STUB_STEP;
        size.height = 1;
    }

    lut_data = lut->data.ptr;

    if( CV_MAT_DEPTH( src->type ) == CV_8S )
    {
        int half_size = CV_ELEM_SIZE(type)*128;

        // shuffle lut
        memcpy( shuffled_lut, lut_data + half_size, half_size );
        memcpy( shuffled_lut + half_size, lut_data, half_size );

        lut_data = shuffled_lut;
    }

    func = (CvLUT_TransformFunc)(lut_tab.fn_2d[type]);
    if( !func )
        CV_ERROR( CV_StsUnsupportedFormat, "" );

    IPPI_CALL( func( src->data.ptr, src_step, dst->data.ptr,
                     dst_step, size, lut_data));

    __END__;
}

/* End of file. */
