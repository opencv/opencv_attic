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

#define  ICV_DEF_ACC_FUNC_BODY( srctype, dsttype, cvtmacro )                        \
{                                                                                   \
    for( ; roiSize.height--; (char*&)pSrc += srcStep,                               \
                             (char*&)pSrcDst += srcDstStep )                        \
    {                                                                               \
        int x;                                                                      \
                                                                                    \
        for( x = 0; x <= roiSize.width - 4; x += 4 )                                \
        {                                                                           \
            dsttype t0 = pSrcDst[x] + cvtmacro(pSrc[x]);                            \
            dsttype t1 = pSrcDst[x + 1] + cvtmacro(pSrc[x + 1]);                    \
                                                                                    \
            pSrcDst[x] = (dsttype)t0;                                               \
            pSrcDst[x + 1] = (dsttype)t1;                                           \
                                                                                    \
            t0 = pSrcDst[x + 2] + cvtmacro(pSrc[x + 2]);                            \
            t1 = pSrcDst[x + 3] + cvtmacro(pSrc[x + 3]);                            \
                                                                                    \
            pSrcDst[x + 2] = (dsttype)t0;                                           \
            pSrcDst[x + 3] = (dsttype)t1;                                           \
        }                                                                           \
                                                                                    \
        for( ; x < roiSize.width; x++ )                                             \
        {                                                                           \
            dsttype t0 = pSrcDst[x] + cvtmacro(pSrc[x]);                            \
            pSrcDst[x] = (dsttype)t0;                                               \
        }                                                                           \
    }                                                                               \
                                                                                    \
    return CV_OK;                                                                   \
}

#define  ICV_DEF_ACC_FUNC( name, srctype, dsttype, cvtmacro )                       \
IPCVAPI_IMPL( CvStatus,                                                             \
name,( const srctype *pSrc, int srcStep,                                            \
      dsttype *pSrcDst, int srcDstStep,                                             \
      CvSize roiSize ), (pSrc, srcStep, pSrcDst, srcDstStep, roiSize) )             \
ICV_DEF_ACC_FUNC_BODY( srctype, dsttype, cvtmacro )

#define  ICV_DEF_ACC_FUNC_STATIC( name, srctype, dsttype, cvtmacro )                \
static CvStatus CV_STDCALL                                                          \
name( const srctype *pSrc, int srcStep,                                             \
      dsttype *pSrcDst, int srcDstStep,                                             \
      CvSize roiSize )                                                              \
ICV_DEF_ACC_FUNC_BODY( srctype, dsttype, cvtmacro )

#define  ICV_DEF_ACCPROD_FUNC( name, srctype, dsttype, cvtmacro )                   \
IPCVAPI_IMPL( CvStatus,                                                             \
name,( const srctype *pSrc1, int src1Step, const srctype *pSrc2, int src2Step,      \
      dsttype *pSrcDst, int srcDstStep, CvSize roiSize ),                           \
      (pSrc1, src1Step, pSrc2, src2Step, pSrcDst, srcDstStep, roiSize) )            \
{                                                                                   \
    for( ; roiSize.height--; (char*&)pSrc1 += src1Step,                             \
                             (char*&)pSrc2 += src2Step,                             \
                             (char*&)pSrcDst += srcDstStep )                        \
    {                                                                               \
        int x;                                                                      \
                                                                                    \
        for( x = 0; x <= roiSize.width - 4; x += 4 )                                \
        {                                                                           \
            dsttype t0 = pSrcDst[x] + cvtmacro(pSrc1[x])*cvtmacro(pSrc2[x]);        \
            dsttype t1 = pSrcDst[x+1] + cvtmacro(pSrc1[x+1])*cvtmacro(pSrc2[x+1]);  \
                                                                                    \
            pSrcDst[x] = (dsttype)t0;                                               \
            pSrcDst[x + 1] = (dsttype)t1;                                           \
                                                                                    \
            t0 = pSrcDst[x + 2] + cvtmacro(pSrc1[x + 2])*cvtmacro(pSrc2[x + 2]);    \
            t1 = pSrcDst[x + 3] + cvtmacro(pSrc1[x + 3])*cvtmacro(pSrc2[x + 3]);    \
                                                                                    \
            pSrcDst[x + 2] = (dsttype)t0;                                           \
            pSrcDst[x + 3] = (dsttype)t1;                                           \
        }                                                                           \
                                                                                    \
        for( ; x < roiSize.width; x++ )                                             \
        {                                                                           \
            dsttype t0 = pSrcDst[x] + cvtmacro(pSrc1[x])*cvtmacro(pSrc2[x]);        \
            pSrcDst[x] = (dsttype)t0;                                               \
        }                                                                           \
    }                                                                               \
                                                                                    \
    return CV_OK;                                                                   \
}


#define  ICV_DEF_ACCWEIGHT_FUNC( name, srctype, dsttype, cvtmacro )                 \
IPCVAPI_IMPL( CvStatus,                                                             \
name,( const srctype *pSrc, int srcStep, dsttype *pSrcDst, int srcDstStep,          \
      CvSize roiSize, dsttype alpha ),                                              \
      (pSrc, srcStep, pSrcDst, srcDstStep, roiSize, alpha) )                        \
{                                                                                   \
    for( ; roiSize.height--; (char*&)pSrc += srcStep,                               \
                             (char*&)pSrcDst += srcDstStep )                        \
    {                                                                               \
        int x;                                                                      \
                                                                                    \
        for( x = 0; x <= roiSize.width - 4; x += 4 )                                \
        {                                                                           \
            dsttype t0 = pSrcDst[x] + alpha*(cvtmacro(pSrc[x]) - pSrcDst[x]);       \
            dsttype t1 = pSrcDst[x+1] + alpha*(cvtmacro(pSrc[x+1]) - pSrcDst[x+1]); \
                                                                                    \
            pSrcDst[x] = (dsttype)t0;                                               \
            pSrcDst[x + 1] = (dsttype)t1;                                           \
                                                                                    \
            t0 = pSrcDst[x + 2] + alpha*(cvtmacro(pSrc[x + 2]) - pSrcDst[x + 2]);   \
            t1 = pSrcDst[x + 3] + alpha*(cvtmacro(pSrc[x + 3]) - pSrcDst[x + 3]);   \
                                                                                    \
            pSrcDst[x + 2] = (dsttype)t0;                                           \
            pSrcDst[x + 3] = (dsttype)t1;                                           \
        }                                                                           \
                                                                                    \
        for( ; x < roiSize.width; x++ )                                             \
        {                                                                           \
            dsttype t0 = pSrcDst[x] + alpha*(cvtmacro(pSrc[x]) - pSrcDst[x]);       \
            pSrcDst[x] = (dsttype)t0;                                               \
        }                                                                           \
    }                                                                               \
                                                                                    \
    return CV_OK;                                                                   \
}


#define  ICV_DEF_ACCMASK_CASE_C1( dsttype, cvtmacro, maskmacro, prepare_mask )  \
{                                                                               \
    for( x = 0; x <= roiSize.width - 4; x += 4 )                                \
    {                                                                           \
        dsttype t0 = pSrcDst[x] + cvtmacro(maskmacro(pMask[x], pSrc[x]));       \
        dsttype t1 = pSrcDst[x+1] + cvtmacro(maskmacro(pMask[x+1], pSrc[x+1])); \
                                                                                \
        pSrcDst[x] = (dsttype)t0;                                               \
        pSrcDst[x + 1] = (dsttype)t1;                                           \
                                                                                \
        t0 = pSrcDst[x + 2] + cvtmacro(maskmacro(pMask[x + 2], pSrc[x + 2]));   \
        t1 = pSrcDst[x + 3] + cvtmacro(maskmacro(pMask[x + 3], pSrc[x + 3]));   \
                                                                                \
        pSrcDst[x + 2] = (dsttype)t0;                                           \
        pSrcDst[x + 3] = (dsttype)t1;                                           \
    }                                                                           \
                                                                                \
    for( ; x < roiSize.width; x++ )                                             \
    {                                                                           \
        dsttype t0 = pSrcDst[x] + cvtmacro(maskmacro(pMask[x], pSrc[x]));       \
        pSrcDst[x] = (dsttype)t0;                                               \
    }                                                                           \
}


#define  ICV_DEF_ACCMASK_CASE_C2( dsttype, cvtmacro, maskmacro, prepare_mask )  \
{                                                                               \
    for( x = 0; x < roiSize.width; x++ )                                        \
    {                                                                           \
        prepare_mask(m,pMask[x]);                                               \
        dsttype t0 = pSrcDst[x*2] + cvtmacro(maskmacro(pSrc[x*2], m ));         \
        dsttype t1 = pSrcDst[x*2+1] + cvtmacro(maskmacro(pSrc[x*2+1], m ));     \
                                                                                \
        pSrcDst[x*2] = (dsttype)t0;                                             \
        pSrcDst[x*2 + 1] = (dsttype)t1;                                         \
    }                                                                           \
}


#define  ICV_DEF_ACCMASK_CASE_C3( dsttype, cvtmacro, maskmacro, prepare_mask )  \
{                                                                               \
    for( x = 0; x < roiSize.width; x++ )                                        \
    {                                                                           \
        prepare_mask(m,pMask[x]);                                               \
        dsttype t0 = pSrcDst[x*3] + cvtmacro(maskmacro(pSrc[x*3], m ));         \
        dsttype t1 = pSrcDst[x*3+1] + cvtmacro(maskmacro(pSrc[x*3+1], m ));     \
        dsttype t2 = pSrcDst[x*3+2] + cvtmacro(maskmacro(pSrc[x*3+2], m ));     \
                                                                                \
        pSrcDst[x*3] = (dsttype)t0;                                             \
        pSrcDst[x*3 + 1] = (dsttype)t1;                                         \
        pSrcDst[x*3 + 2] = (dsttype)t2;                                         \
    }                                                                           \
}


#define  ICV_DEF_ACCMASK_CASE_C4( dsttype, cvtmacro, maskmacro, prepare_mask )  \
{                                                                               \
    for( x = 0; x < roiSize.width; x++ )                                        \
    {                                                                           \
        prepare_mask(m,pMask[x]);                                               \
        dsttype t0 = pSrcDst[x*4] + cvtmacro(maskmacro(pSrc[x*4], m ));         \
        dsttype t1 = pSrcDst[x*4+1] + cvtmacro(maskmacro(pSrc[x*4+1], m ));     \
        dsttype t2 = pSrcDst[x*4+2] + cvtmacro(maskmacro(pSrc[x*4+2], m ));     \
        dsttype t3 = pSrcDst[x*4+3] + cvtmacro(maskmacro(pSrc[x*4+3], m ));     \
                                                                                \
        pSrcDst[x*4] = (dsttype)t0;                                             \
        pSrcDst[x*4 + 1] = (dsttype)t1;                                         \
        pSrcDst[x*4 + 2] = (dsttype)t2;                                         \
        pSrcDst[x*4 + 3] = (dsttype)t3;                                         \
    }                                                                           \
}


#define  ICV_DEF_ACCMASK_FUNC( name, cn, srctype, dsttype, cvtmacro,            \
                               maskmacro, define_mask, prepare_mask )           \
IPCVAPI_IMPL( CvStatus,                                                         \
name,( const srctype *pSrc, int srcStep,                                        \
      const uchar *pMask, int maskStep,                                         \
      dsttype *pSrcDst, int srcDstStep,                                         \
      CvSize roiSize ),                                                         \
      (pSrc, srcStep, pMask, maskStep, pSrcDst, srcDstStep, roiSize) )          \
{                                                                               \
    int x;                                                                      \
    define_mask;                                                                \
                                                                                \
    for( ; roiSize.height--; (char*&)pSrc += srcStep,                           \
                             (char*&)pMask += maskStep,                         \
                             (char*&)pSrcDst += srcDstStep )                    \
    {                                                                           \
        ICV_DEF_ACCMASK_CASE_C##cn( dsttype, cvtmacro,                          \
                                    maskmacro, prepare_mask);                   \
    }                                                                           \
                                                                                \
    return CV_OK;                                                               \
}


#define  ICV_DEF_ACCPRODMASK_CASE_C1( dsttype, cvtmacro, maskmacro, prepare_mask)\
{                                                                               \
    for( x = 0; x <= roiSize.width - 4; x += 4 )                                \
    {                                                                           \
        dsttype t0 = pSrcDst[x] + cvtmacro(pSrc1[x])*                           \
                     cvtmacro(maskmacro(pMask[x], pSrc2[x]));                   \
        dsttype t1 = pSrcDst[x + 1] + cvtmacro(pSrc1[x + 1])*                   \
                     cvtmacro(maskmacro(pMask[x + 1], pSrc2[x + 1]));           \
                                                                                \
        pSrcDst[x] = (dsttype)t0;                                               \
        pSrcDst[x + 1] = (dsttype)t1;                                           \
                                                                                \
        t0 = pSrcDst[x + 2] + cvtmacro(pSrc1[x + 2])*                           \
                              cvtmacro(maskmacro(pMask[x + 2], pSrc2[x + 2]));  \
        t1 = pSrcDst[x + 3] + cvtmacro(pSrc1[x + 3])*                           \
                              cvtmacro(maskmacro(pMask[x + 3], pSrc2[x + 3]));  \
                                                                                \
        pSrcDst[x + 2] = (dsttype)t0;                                           \
        pSrcDst[x + 3] = (dsttype)t1;                                           \
    }                                                                           \
                                                                                \
    for( ; x < roiSize.width; x++ )                                             \
    {                                                                           \
        dsttype t0 = pSrcDst[x] + cvtmacro(pSrc1[x])*                           \
                                  cvtmacro(maskmacro(pMask[x], pSrc2[x]));      \
        pSrcDst[x] = (dsttype)t0;                                               \
    }                                                                           \
}


#define  ICV_DEF_ACCPRODMASK_CASE_C2( dsttype, cvtmacro, maskmacro, prepare_mask)\
{                                                                               \
    for( x = 0; x < roiSize.width; x++ )                                        \
    {                                                                           \
        prepare_mask(m,pMask[x]);                                               \
        dsttype t0 = pSrcDst[x*2] + cvtmacro(pSrc1[x*2])*                       \
                                    cvtmacro(maskmacro(pSrc2[x*2], m ));        \
        dsttype t1 = pSrcDst[x*2+1] + cvtmacro(pSrc1[x*2+1])*                   \
                                      cvtmacro(maskmacro(pSrc2[x*2+1], m ));    \
                                                                                \
        pSrcDst[x*2] = (dsttype)t0;                                             \
        pSrcDst[x*2 + 1] = (dsttype)t1;                                         \
    }                                                                           \
}


#define  ICV_DEF_ACCPRODMASK_CASE_C3( dsttype, cvtmacro, maskmacro, prepare_mask)\
{                                                                               \
    for( x = 0; x < roiSize.width; x++ )                                        \
    {                                                                           \
        prepare_mask(m,pMask[x]);                                               \
        dsttype t0 = pSrcDst[x*3] + cvtmacro(pSrc1[x*3])*                       \
                                    cvtmacro(maskmacro(pSrc2[x*3], m ));        \
        dsttype t1 = pSrcDst[x*3+1] + cvtmacro(pSrc1[x*3+1])*                   \
                                      cvtmacro(maskmacro(pSrc2[x*3+1], m ));    \
        dsttype t2 = pSrcDst[x*3+2] + cvtmacro(pSrc1[x*3+2])*                   \
                                      cvtmacro(maskmacro(pSrc2[x*3+2], m ));    \
                                                                                \
        pSrcDst[x*3] = (dsttype)t0;                                             \
        pSrcDst[x*3 + 1] = (dsttype)t1;                                         \
        pSrcDst[x*3 + 2] = (dsttype)t2;                                         \
    }                                                                           \
}


#define  ICV_DEF_ACCPRODMASK_CASE_C4( dsttype, cvtmacro, maskmacro, prepare_mask ) \
{                                                                               \
    for( x = 0; x < roiSize.width; x++ )                                        \
    {                                                                           \
        prepare_mask(m,pMask[x]);                                               \
        dsttype t0 = pSrcDst[x*4] + cvtmacro(pSrc1[x*4])*                       \
                                    cvtmacro(maskmacro(pSrc2[x*4], m ));        \
        dsttype t1 = pSrcDst[x*4+1] + cvtmacro(pSrc1[x*4+1])*                   \
                                      cvtmacro(maskmacro(pSrc2[x*4+1], m ));    \
        dsttype t2 = pSrcDst[x*4+2] + cvtmacro(pSrc1[x*4+2])*                   \
                                      cvtmacro(maskmacro(pSrc2[x*4+2], m ));    \
        dsttype t3 = pSrcDst[x*4+3] + cvtmacro(pSrc1[x*4+3])*                   \
                                      cvtmacro(maskmacro(pSrc2[x*4+3], m ));    \
                                                                                \
        pSrcDst[x*4] = (dsttype)t0;                                             \
        pSrcDst[x*4 + 1] = (dsttype)t1;                                         \
        pSrcDst[x*4 + 2] = (dsttype)t2;                                         \
        pSrcDst[x*4 + 3] = (dsttype)t3;                                         \
    }                                                                           \
}


#define  ICV_DEF_ACCPRODMASK_FUNC( name, cn, srctype, dsttype, cvtmacro,        \
                                   maskmacro, define_mask, prepare_mask )       \
IPCVAPI_IMPL( CvStatus,                                                         \
name,( const srctype *pSrc1, int src1Step,                                      \
      const srctype *pSrc2, int src2Step,                                       \
      const uchar *pMask, int maskStep,                                         \
      dsttype *pSrcDst, int srcDstStep,                                         \
      CvSize roiSize ),                                                         \
      (pSrc1, src1Step, pSrc2, src2Step, pMask, maskStep,                       \
       pSrcDst, srcDstStep, roiSize) )                                          \
{                                                                               \
    int x;                                                                      \
    define_mask;                                                                \
                                                                                \
    for( ; roiSize.height--; (char*&)pSrc1 += src1Step,                         \
                             (char*&)pSrc2 += src2Step,                         \
                             (char*&)pMask += maskStep,                         \
                             (char*&)pSrcDst += srcDstStep )                    \
    {                                                                           \
        ICV_DEF_ACCPRODMASK_CASE_C##cn( dsttype, cvtmacro,                      \
                                        maskmacro, prepare_mask );              \
    }                                                                           \
                                                                                \
    return CV_OK;                                                               \
}


#define  ICV_DEF_ACCWEIGHTEDMASK_CASE_C1( dsttype, cvtmacro,                    \
                                          maskmacro, prepare_mask )             \
{                                                                               \
    for( x = 0; x <= roiSize.width - 4; x += 4 )                                \
    {                                                                           \
        dsttype t0 = pSrcDst[x] +                                               \
                     maskmacro(pMask[x], cvtmacro(pSrc[x]) - pSrcDst[x]);       \
        dsttype t1 = pSrcDst[x+1] +                                             \
                     maskmacro(pMask[x+1], cvtmacro(pSrc[x+1]) - pSrcDst[x+1]); \
                                                                                \
        pSrcDst[x] = (dsttype)t0;                                               \
        pSrcDst[x + 1] = (dsttype)t1;                                           \
                                                                                \
        t0 = pSrcDst[x+2] +                                                     \
             maskmacro(pMask[x+2], cvtmacro(pSrc[x+2]) - pSrcDst[x+2]);         \
        t1 = pSrcDst[x+3] +                                                     \
             maskmacro(pMask[x+3], cvtmacro(pSrc[x+3]) - pSrcDst[x+3]);         \
                                                                                \
        pSrcDst[x + 2] = (dsttype)t0;                                           \
        pSrcDst[x + 3] = (dsttype)t1;                                           \
    }                                                                           \
                                                                                \
    for( ; x < roiSize.width; x++ )                                             \
    {                                                                           \
        dsttype t0 = pSrcDst[x] +                                               \
                     maskmacro(pMask[x], cvtmacro(pSrc[x]) - pSrcDst[x]);       \
        pSrcDst[x] = (dsttype)t0;                                               \
    }                                                                           \
}


#define  ICV_DEF_ACCWEIGHTEDMASK_CASE_C2( dsttype, cvtmacro,                    \
                                          maskmacro, prepare_mask )             \
{                                                                               \
    for( x = 0; x < roiSize.width; x++ )                                        \
    {                                                                           \
        prepare_mask( m, pMask[x] );                                            \
                                                                                \
        dsttype t0 = pSrcDst[x*2] +                                             \
                     maskmacro(cvtmacro(pSrc[x*3]) - pSrcDst[x*2], m );         \
        dsttype t1 = pSrcDst[x*2+1] +                                           \
                     maskmacro(cvtmacro(pSrc[x*2+1]) - pSrcDst[x*2+1], m );     \
                                                                                \
        pSrcDst[x*2] = (dsttype)t0;                                             \
        pSrcDst[x*2 + 1] = (dsttype)t1;                                         \
    }                                                                           \
}


#define  ICV_DEF_ACCWEIGHTEDMASK_CASE_C3( dsttype, cvtmacro,                    \
                                          maskmacro, prepare_mask )             \
{                                                                               \
    for( x = 0; x < roiSize.width; x++ )                                        \
    {                                                                           \
        prepare_mask( m, pMask[x] );                                            \
                                                                                \
        dsttype t0 = pSrcDst[x*3] +                                             \
                     maskmacro(cvtmacro(pSrc[x*3]) - pSrcDst[x*3], m );         \
        dsttype t1 = pSrcDst[x*3+1] +                                           \
                     maskmacro(cvtmacro(pSrc[x*3+1]) - pSrcDst[x*3+1], m );     \
        dsttype t2 = pSrcDst[x*3+2] +                                           \
                     maskmacro(cvtmacro(pSrc[x*3+2]) - pSrcDst[x*3+2], m );     \
                                                                                \
        pSrcDst[x*3] = (dsttype)t0;                                             \
        pSrcDst[x*3 + 1] = (dsttype)t1;                                         \
        pSrcDst[x*3 + 2] = (dsttype)t2;                                         \
    }                                                                           \
}


#define  ICV_DEF_ACCWEIGHTEDMASK_CASE_C4( dsttype, cvtmacro,                   \
                                          maskmacro, prepare_mask )            \
{                                                                              \
    for( x = 0; x < roiSize.width; x++ )                                       \
    {                                                                          \
        prepare_mask( m, pMask[x] );                                           \
                                                                               \
        dsttype t0 = pSrcDst[x*4] +                                            \
                     maskmacro(cvtmacro(pSrc[x*4]) - pSrcDst[x*4], m );        \
        dsttype t1 = pSrcDst[x*4+1] +                                          \
                     maskmacro(cvtmacro(pSrc[x*4+1]) - pSrcDst[x*4+1], m );    \
        dsttype t2 = pSrcDst[x*4+2] +                                          \
                     maskmacro(cvtmacro(pSrc[x*4+2]) - pSrcDst[x*4+2], m );    \
        dsttype t3 = pSrcDst[x*4+3] +                                          \
                     maskmacro(cvtmacro(pSrc[x*4+3]) - pSrcDst[x*4+3], m );    \
                                                                               \
        pSrcDst[x*4] = (dsttype)t0;                                            \
        pSrcDst[x*4 + 1] = (dsttype)t1;                                        \
        pSrcDst[x*4 + 2] = (dsttype)t2;                                        \
        pSrcDst[x*4 + 3] = (dsttype)t3;                                        \
    }                                                                          \
}


#define  ICV_DEF_ACCWEIGHTMASK_FUNC( name, cn, srctype, dsttype, cvtmacro,     \
                                      maskmacro, define_mask, prepare_mask )   \
IPCVAPI_IMPL( CvStatus,                                                        \
name,( const srctype *pSrc, int srcStep,                                       \
      const uchar *pMask, int maskStep,                                        \
      dsttype *pSrcDst, int srcDstStep,                                        \
      CvSize roiSize, dsttype alpha ),                                         \
      (pSrc, srcStep, pMask, maskStep, pSrcDst, srcDstStep, roiSize, alpha) )  \
{                                                                              \
    int x;                                                                     \
    define_mask;                                                               \
                                                                               \
    for( ; roiSize.height--; (char*&)pSrc += srcStep,                          \
                             (char*&)pMask += maskStep,                        \
                             (char*&)pSrcDst += srcDstStep )                   \
    {                                                                          \
        ICV_DEF_ACCWEIGHTEDMASK_CASE_C##cn( dsttype, cvtmacro,                 \
                                            maskmacro, prepare_mask);          \
    }                                                                          \
                                                                               \
    return CV_OK;                                                              \
}


#define CV_DEFINE_ALPHA_MASK   float maskTab[] = { 0.f, alpha }
#define CV_PREPARE_INT_MASK( m, srcmask )  int m = ((srcmask) == 0) - 1
#define CV_PREPARE_FLT_MASK( m, srcmask )  float m = maskTab[(srcmask) != 0]
#define ICV_DUMMY(x) ((x)=(x))

#define  ICV_DEF_ACC_ALL( flavor, srctype, dsttype )                                   \
                                                                                       \
ICV_DEF_ACC_FUNC( icvAddSquare_##flavor##_C1IR, srctype, dsttype, CV_8TO32F_SQR )      \
ICV_DEF_ACCPROD_FUNC( icvAddProduct_##flavor##_C1IR, srctype, dsttype, CV_8TO32F )     \
ICV_DEF_ACCWEIGHT_FUNC( icvAddWeighted_##flavor##_C1IR, srctype, dsttype, CV_8TO32F )  \
                                                                                       \
ICV_DEF_ACCMASK_FUNC( icvAdd_##flavor##_C1IMR, 1, srctype, dsttype,                    \
                      CV_8TO32F, CV_ANDMASK, ICV_DUMMY(pSrcDst), CV_PREPARE_INT_MASK ) \
ICV_DEF_ACCMASK_FUNC( icvAdd_##flavor##_C3IMR, 3, srctype, dsttype,                    \
                       CV_8TO32F, CV_AND, ICV_DUMMY(pSrcDst), CV_PREPARE_INT_MASK )    \
                                                                                       \
ICV_DEF_ACCMASK_FUNC( icvAddSquare_##flavor##_C1IMR, 1, srctype, dsttype,              \
                      CV_8TO32F_SQR, CV_ANDMASK, ICV_DUMMY(pSrcDst), CV_PREPARE_INT_MASK )\
ICV_DEF_ACCMASK_FUNC( icvAddSquare_##flavor##_C3IMR, 3, srctype, dsttype,              \
                      CV_8TO32F_SQR, CV_AND, ICV_DUMMY(pSrcDst), CV_PREPARE_INT_MASK ) \
                                                                                       \
ICV_DEF_ACCPRODMASK_FUNC( icvAddProduct_##flavor##_C1IMR, 1, srctype, dsttype,         \
                          CV_8TO32F, CV_ANDMASK, ICV_DUMMY(pSrcDst), CV_PREPARE_INT_MASK )\
ICV_DEF_ACCPRODMASK_FUNC( icvAddProduct_##flavor##_C3IMR, 3, srctype, dsttype,         \
                          CV_8TO32F, CV_AND, ICV_DUMMY(pSrcDst), CV_PREPARE_INT_MASK ) \
                                                                                       \
ICV_DEF_ACCWEIGHTMASK_FUNC( icvAddWeighted_##flavor##_C1IMR, 1, srctype, dsttype,      \
                            CV_8TO32F, CV_MULMASK, CV_DEFINE_ALPHA_MASK,               \
                            CV_PREPARE_FLT_MASK )                                      \
ICV_DEF_ACCWEIGHTMASK_FUNC( icvAddWeighted_##flavor##_C3IMR, 3, srctype, dsttype,      \
                            CV_8TO32F, CV_MUL, CV_DEFINE_ALPHA_MASK,                   \
                            CV_PREPARE_FLT_MASK )


#define  ICV_DEF_ACC_ALL_FLT( flavor, srctype, dsttype )                               \
                                                                                       \
ICV_DEF_ACC_FUNC( icvAddSquare_##flavor##_C1IR, srctype, dsttype, CV_SQR )             \
ICV_DEF_ACCPROD_FUNC( icvAddProduct_##flavor##_C1IR, srctype, dsttype, CV_NOP )        \
ICV_DEF_ACCWEIGHT_FUNC( icvAddWeighted_##flavor##_C1IR, srctype, dsttype, CV_NOP )     \
                                                                                       \
ICV_DEF_ACCMASK_FUNC( icvAdd_##flavor##_C1IMR, 1, srctype, dsttype,                    \
                       CV_NOP, CV_MULMASK, CV_DEFINE_MASK, CV_PREPARE_FLT_MASK )       \
ICV_DEF_ACCMASK_FUNC( icvAdd_##flavor##_C3IMR, 3, srctype, dsttype,                    \
                       CV_NOP, CV_MUL, CV_DEFINE_MASK, CV_PREPARE_FLT_MASK )           \
                                                                                       \
ICV_DEF_ACCMASK_FUNC( icvAddSquare_##flavor##_C1IMR, 1, srctype, dsttype,              \
                       CV_SQR, CV_MULMASK, CV_DEFINE_MASK, CV_PREPARE_FLT_MASK )       \
ICV_DEF_ACCMASK_FUNC( icvAddSquare_##flavor##_C3IMR, 3, srctype, dsttype,              \
                       CV_SQR, CV_MUL, CV_DEFINE_MASK, CV_PREPARE_FLT_MASK )           \
                                                                                       \
ICV_DEF_ACCPRODMASK_FUNC( icvAddProduct_##flavor##_C1IMR, 1, srctype, dsttype,         \
                           CV_NOP, CV_MULMASK, CV_DEFINE_MASK, CV_PREPARE_FLT_MASK )   \
ICV_DEF_ACCPRODMASK_FUNC( icvAddProduct_##flavor##_C3IMR, 3, srctype, dsttype,         \
                           CV_NOP, CV_MUL, CV_DEFINE_MASK, CV_PREPARE_FLT_MASK )       \
                                                                                       \
ICV_DEF_ACCWEIGHTMASK_FUNC( icvAddWeighted_##flavor##_C1IMR, 1, srctype, dsttype,      \
                             CV_NOP, CV_MULMASK, CV_DEFINE_ALPHA_MASK,                 \
                             CV_PREPARE_FLT_MASK )                                     \
ICV_DEF_ACCWEIGHTMASK_FUNC( icvAddWeighted_##flavor##_C3IMR, 3, srctype, dsttype,      \
                             CV_NOP, CV_MUL, CV_DEFINE_ALPHA_MASK,                     \
                             CV_PREPARE_FLT_MASK )

ICV_DEF_ACC_FUNC( icvAdd_8u32f_C1IR, uchar, float, CV_8TO32F )
ICV_DEF_ACC_FUNC_STATIC( icvAdd_8u64f_C1IR, uchar, double, CV_8TO32F )
ICV_DEF_ACC_FUNC_STATIC( icvAdd_16s32f_C1IR, short, float, CV_8TO32F )
ICV_DEF_ACC_FUNC_STATIC( icvAdd_16s64f_C1IR, short, double, CV_8TO32F )
ICV_DEF_ACC_FUNC_STATIC( icvAdd_32f64f_C1IR, float, double, CV_NOP )

ICV_DEF_ACC_ALL( 8u32f, uchar, float )
ICV_DEF_ACC_ALL_FLT( 32f, float, float )

static void
icvInitAccTable( CvFuncTable* tabfl, CvFuncTable* tabdb,
                 CvBigFuncTable* masktab )
{
    tabfl->fn_2d[CV_8U] = (void*)icvAdd_8u32f_C1IR;
    tabfl->fn_2d[CV_16S] = (void*)icvAdd_16s32f_C1IR;
    tabfl->fn_2d[CV_32F] = 0;

    tabdb->fn_2d[CV_8U] = (void*)icvAdd_8u64f_C1IR;
    tabdb->fn_2d[CV_16S] = (void*)icvAdd_16s64f_C1IR;
    tabdb->fn_2d[CV_32F] = (void*)icvAdd_32f64f_C1IR;
    tabdb->fn_2d[CV_64F] = 0;

    if( masktab )
    {
        masktab->fn_2d[CV_8UC1] = (void*)icvAdd_8u32f_C1IMR;
        masktab->fn_2d[CV_8SC1] = 0;
        masktab->fn_2d[CV_32FC1] = (void*)icvAdd_32f_C1IMR;

        masktab->fn_2d[CV_8UC3] = (void*)icvAdd_8u32f_C3IMR;
        masktab->fn_2d[CV_8SC3] = 0;
        masktab->fn_2d[CV_32FC3] = (void*)icvAdd_32f_C3IMR;
    }
}


#define  ICV_DEF_INIT_ACC_TAB( FUNCNAME )                                           \
static  void  icvInit##FUNCNAME##Table( CvFuncTable* tab, CvBigFuncTable* masktab ) \
{                                                                                   \
    tab->fn_2d[CV_8U] = (void*)icv##FUNCNAME##_8u32f_C1IR;                          \
    tab->fn_2d[CV_8S] = 0;                                                          \
    tab->fn_2d[CV_32F] = (void*)icv##FUNCNAME##_32f_C1IR;                           \
                                                                                    \
    masktab->fn_2d[CV_8UC1] = (void*)icv##FUNCNAME##_8u32f_C1IMR;                   \
    masktab->fn_2d[CV_8SC1] = 0;                                                    \
    masktab->fn_2d[CV_32FC1] = (void*)icv##FUNCNAME##_32f_C1IMR;                    \
                                                                                    \
    masktab->fn_2d[CV_8UC3] = (void*)icv##FUNCNAME##_8u32f_C3IMR;                   \
    masktab->fn_2d[CV_8SC3] = 0;                                                    \
    masktab->fn_2d[CV_32FC3] = (void*)icv##FUNCNAME##_32f_C3IMR;                    \
}


ICV_DEF_INIT_ACC_TAB( AddSquare )
ICV_DEF_INIT_ACC_TAB( AddProduct )
ICV_DEF_INIT_ACC_TAB( AddWeighted )


CV_IMPL void
cvAcc( const void* arr, void* sumarr, const void* maskarr )
{
    static CvFuncTable acc_tab[2];
    static CvBigFuncTable accmask_tab;
    static int inittab = 0;
    
    CV_FUNCNAME( "cvAcc" );

    __BEGIN__;

    int type, sumdepth;
    int mat_step, sum_step, mask_step = 0;
    CvSize size;
    CvMat stub, *mat = (CvMat*)arr;
    CvMat sumstub, *sum = (CvMat*)sumarr;
    CvMat maskstub, *mask = (CvMat*)maskarr;

    if( !inittab )
    {
        icvInitAccTable( &acc_tab[0], &acc_tab[1], &accmask_tab );
        inittab = 1;
    }

    if( !CV_IS_MAT( mat ) || !CV_IS_MAT( sum ))
    {
        int coi1 = 0, coi2 = 0;
        CV_CALL( mat = cvGetMat( mat, &stub, &coi1 ));
        CV_CALL( sum = cvGetMat( sum, &sumstub, &coi2 ));
        if( coi1 + coi2 != 0 )
            CV_ERROR( CV_BadCOI, "" );
    }

    if( CV_ARE_TYPES_EQ( mat, sum ) && !mask )
    {
        CV_CALL( cvAdd( mat, sum, sum, 0 ));
        EXIT;
    }

    if( !CV_ARE_CNS_EQ( mat, sum ))
        CV_ERROR( CV_StsUnmatchedFormats, "" );

    sumdepth = CV_MAT_DEPTH( sum->type );
    if( sumdepth != CV_32F && (maskarr != 0 || sumdepth != CV_64F))
        CV_ERROR( CV_BadDepth, "Bad accumulator type" );

    if( !CV_ARE_SIZES_EQ( mat, sum ))
        CV_ERROR( CV_StsUnmatchedSizes, "" );

    size = cvGetMatSize( mat );
    type = CV_MAT_TYPE( mat->type );

    mat_step = mat->step;
    sum_step = sum->step;

    if( !mask )
    {
        CvFunc2D_2A func=(CvFunc2D_2A)acc_tab[sumdepth==CV_64F].fn_2d[CV_MAT_DEPTH(type)];

        if( !func )
            CV_ERROR( CV_StsUnsupportedFormat, "Unsupported type combination" );

        size.width *= CV_MAT_CN(type);
        if( CV_IS_MAT_CONT( mat->type & sum->type ))
        {
            size.width *= size.height;
            mat_step = sum_step = CV_STUB_STEP;
            size.height = 1;
        }

        IPPI_CALL( func( mat->data.ptr, mat_step, sum->data.ptr, sum_step, size ));
    }
    else
    {
        CvFunc2D_3A func = (CvFunc2D_3A)accmask_tab.fn_2d[type];

        if( !func )
            CV_ERROR( CV_StsUnsupportedFormat, "" );

        CV_CALL( mask = cvGetMat( mask, &maskstub ));

        if( !CV_IS_MASK_ARR( mask ))
            CV_ERROR( CV_StsBadMask, "" );

        if( !CV_ARE_SIZES_EQ( mat, mask ))
            CV_ERROR( CV_StsUnmatchedSizes, "" );            

        mask_step = mask->step;

        if( CV_IS_MAT_CONT( mat->type & sum->type & mask->type ))
        {
            size.width *= size.height;
            mat_step = sum_step = mask_step = CV_STUB_STEP;
            size.height = 1;
        }

        IPPI_CALL( func( mat->data.ptr, mat_step, mask->data.ptr, mask_step,
                         sum->data.ptr, sum_step, size ));
    }

    __END__;
}


CV_IMPL void
cvSquareAcc( const void* arr, void* sq_sum, const void* maskarr )
{
    static CvFuncTable acc_tab;
    static CvBigFuncTable accmask_tab;
    static int inittab = 0;
    
    CV_FUNCNAME( "cvSquareAcc" );

    __BEGIN__;

    int coi1, coi2;
    int type;
    int mat_step, sum_step, mask_step = 0;
    CvSize size;
    CvMat stub, *mat = (CvMat*)arr;
    CvMat sumstub, *sum = (CvMat*)sq_sum;
    CvMat maskstub, *mask = (CvMat*)maskarr;

    if( !inittab )
    {
        icvInitAddSquareTable( &acc_tab, &accmask_tab );
        inittab = 1;
    }

    CV_CALL( mat = cvGetMat( mat, &stub, &coi1 ));
    CV_CALL( sum = cvGetMat( sum, &sumstub, &coi2 ));

    if( coi1 != 0 || coi2 != 0 )
        CV_ERROR( CV_BadCOI, "" );

    if( !CV_ARE_CNS_EQ( mat, sum ))
        CV_ERROR( CV_StsUnmatchedFormats, "" );

    if( CV_MAT_DEPTH( sum->type ) != CV_32F )
        CV_ERROR( CV_BadDepth, "" );

    if( !CV_ARE_SIZES_EQ( mat, sum ))
        CV_ERROR( CV_StsUnmatchedSizes, "" );

    size = cvGetMatSize( mat );
    type = CV_MAT_TYPE( mat->type );

    mat_step = mat->step;
    sum_step = sum->step;

    if( !mask )
    {
        CvFunc2D_2A func = (CvFunc2D_2A)acc_tab.fn_2d[CV_MAT_DEPTH(type)];

        if( !func )
            CV_ERROR( CV_StsUnsupportedFormat, "" );

        size.width *= CV_MAT_CN(type);

        if( CV_IS_MAT_CONT( mat->type & sum->type ))
        {
            size.width *= size.height;
            mat_step = sum_step = CV_STUB_STEP;;
            size.height = 1;
        }

        IPPI_CALL( func( mat->data.ptr, mat_step, sum->data.ptr, sum_step, size ));
    }
    else
    {
        CvFunc2D_3A func = (CvFunc2D_3A)accmask_tab.fn_2d[type];

        if( !func )
            CV_ERROR( CV_StsUnsupportedFormat, "" );

        CV_CALL( mask = cvGetMat( mask, &maskstub ));

        if( !CV_IS_MASK_ARR( mask ))
            CV_ERROR( CV_StsBadMask, "" );

        if( !CV_ARE_SIZES_EQ( mat, mask ))
            CV_ERROR( CV_StsUnmatchedSizes, "" );            

        mask_step = mask->step;

        if( CV_IS_MAT_CONT( mat->type & sum->type & mask->type ))
        {
            size.width *= size.height;
            mat_step = sum_step = mask_step = CV_STUB_STEP;
            size.height = 1;
        }

        IPPI_CALL( func( mat->data.ptr, mat_step, mask->data.ptr, mask_step,
                         sum->data.ptr, sum_step, size ));
    }

    __END__;
}


CV_IMPL void
cvMultiplyAcc( const void* arrA, const void* arrB,
               void* acc, const void* maskarr )
{
    static CvFuncTable acc_tab;
    static CvBigFuncTable accmask_tab;
    static int inittab = 0;
    
    CV_FUNCNAME( "cvMultiplyAcc" );

    __BEGIN__;

    int coi1, coi2, coi3;
    int type;
    int mat1_step, mat2_step, sum_step, mask_step = 0;
    CvSize size;
    CvMat stub1, *mat1 = (CvMat*)arrA;
    CvMat stub2, *mat2 = (CvMat*)arrB;
    CvMat sumstub, *sum = (CvMat*)acc;
    CvMat maskstub, *mask = (CvMat*)maskarr;

    if( !inittab )
    {
        icvInitAddProductTable( &acc_tab, &accmask_tab );
        inittab = 1;
    }

    CV_CALL( mat1 = cvGetMat( mat1, &stub1, &coi1 ));
    CV_CALL( mat2 = cvGetMat( mat2, &stub2, &coi2 ));
    CV_CALL( sum = cvGetMat( sum, &sumstub, &coi3 ));

    if( coi1 != 0 || coi2 != 0 || coi3 != 0 )
        CV_ERROR( CV_BadCOI, "" );

    if( !CV_ARE_CNS_EQ( mat1, mat2 ) || !CV_ARE_CNS_EQ( mat1, sum ))
        CV_ERROR( CV_StsUnmatchedFormats, "" );

    if( CV_MAT_DEPTH( sum->type ) != CV_32F )
        CV_ERROR( CV_BadDepth, "" );

    if( !CV_ARE_SIZES_EQ( mat1, sum ) || !CV_ARE_SIZES_EQ( mat2, sum ))
        CV_ERROR( CV_StsUnmatchedSizes, "" );

    size = cvGetMatSize( mat1 );
    type = CV_MAT_TYPE( mat1->type );

    mat1_step = mat1->step;
    mat2_step = mat2->step;
    sum_step = sum->step;

    if( !mask )
    {
        CvFunc2D_3A func = (CvFunc2D_3A)acc_tab.fn_2d[CV_MAT_DEPTH(type)];

        if( !func )
            CV_ERROR( CV_StsUnsupportedFormat, "" );

        size.width *= CV_MAT_CN(type);

        if( CV_IS_MAT_CONT( mat1->type & mat2->type & sum->type ))
        {
            size.width *= size.height;
            mat1_step = mat2_step = sum_step = CV_STUB_STEP;
            size.height = 1;
        }

        IPPI_CALL( func( mat1->data.ptr, mat1_step, mat2->data.ptr, mat2_step,
                         sum->data.ptr, sum_step, size ));
    }
    else
    {
        CvFunc2D_4A func = (CvFunc2D_4A)accmask_tab.fn_2d[type];

        if( !func )
            CV_ERROR( CV_StsUnsupportedFormat, "" );

        CV_CALL( mask = cvGetMat( mask, &maskstub ));

        if( !CV_IS_MASK_ARR( mask ))
            CV_ERROR( CV_StsBadMask, "" );

        if( !CV_ARE_SIZES_EQ( mat1, mask ))
            CV_ERROR( CV_StsUnmatchedSizes, "" );

        mask_step = mask->step;

        if( CV_IS_MAT_CONT( mat1->type & mat2->type & sum->type & mask->type ))
        {
            size.width *= size.height;
            mat1_step = mat2_step = sum_step = mask_step = CV_STUB_STEP;
            size.height = 1;
        }

        IPPI_CALL( func( mat1->data.ptr, mat1_step, mat2->data.ptr, mat2_step,
                         mask->data.ptr, mask_step,
                         sum->data.ptr, sum_step, size ));
    }

    __END__;
}


typedef CvStatus (CV_STDCALL *CvAddWeightedFunc)( const void* src, int srcstep,
                                                  void* dst, int dststep,
                                                  CvSize size, float alpha );

typedef CvStatus (CV_STDCALL *CvAddWeightedMaskFunc)( const void* src, int srcstep,
                                                      void* dst, int dststep,
                                                      const void* mask, int maskstep,
                                                      CvSize size, float alpha );

CV_IMPL void
cvRunningAvg( const void* arrY, void* arrU,
              double alpha, const void* maskarr )
{
    static CvFuncTable acc_tab;
    static CvBigFuncTable accmask_tab;
    static int inittab = 0;
    
    CV_FUNCNAME( "cvRunningAvg" );

    __BEGIN__;

    int coi1, coi2;
    int type;
    int mat_step, sum_step, mask_step = 0;
    CvSize size;
    CvMat stub, *mat = (CvMat*)arrY;
    CvMat sumstub, *sum = (CvMat*)arrU;
    CvMat maskstub, *mask = (CvMat*)maskarr;

    if( !inittab )
    {
        icvInitAddWeightedTable( &acc_tab, &accmask_tab );
        inittab = 1;
    }

    CV_CALL( mat = cvGetMat( mat, &stub, &coi1 ));
    CV_CALL( sum = cvGetMat( sum, &sumstub, &coi2 ));

    if( coi1 != 0 || coi2 != 0 )
        CV_ERROR( CV_BadCOI, "" );

    if( !CV_ARE_CNS_EQ( mat, sum ))
        CV_ERROR( CV_StsUnmatchedFormats, "" );

    if( CV_MAT_DEPTH( sum->type ) != CV_32F )
        CV_ERROR( CV_BadDepth, "" );

    if( !CV_ARE_SIZES_EQ( mat, sum ))
        CV_ERROR( CV_StsUnmatchedSizes, "" );

    size = cvGetMatSize( mat );
    type = CV_MAT_TYPE( mat->type );

    mat_step = mat->step;
    sum_step = sum->step;

    if( !mask )
    {
        CvAddWeightedFunc func = (CvAddWeightedFunc)acc_tab.fn_2d[CV_MAT_DEPTH(type)];

        if( !func )
            CV_ERROR( CV_StsUnsupportedFormat, "" );

        size.width *= CV_MAT_CN(type);
        if( CV_IS_MAT_CONT( mat->type & sum->type ))
        {
            size.width *= size.height;
            mat_step = sum_step = CV_STUB_STEP;
            size.height = 1;
        }

        IPPI_CALL( func( mat->data.ptr, mat_step,
                         sum->data.ptr, sum_step, size, (float)alpha ));
    }
    else
    {
        CvAddWeightedMaskFunc func = (CvAddWeightedMaskFunc)accmask_tab.fn_2d[type];

        if( !func )
            CV_ERROR( CV_StsUnsupportedFormat, "" );

        CV_CALL( mask = cvGetMat( mask, &maskstub ));

        if( !CV_IS_MASK_ARR( mask ))
            CV_ERROR( CV_StsBadMask, "" );

        if( !CV_ARE_SIZES_EQ( mat, mask ))
            CV_ERROR( CV_StsUnmatchedSizes, "" );

        mask_step = mask->step;

        if( CV_IS_MAT_CONT( mat->type & sum->type & mask->type ))
        {
            size.width *= size.height;
            mat_step = sum_step = mask_step = CV_STUB_STEP;
            size.height = 1;
        }

        IPPI_CALL( func( mat->data.ptr, mat_step, mask->data.ptr, mask_step,
                         sum->data.ptr, sum_step, size, (float)alpha ));
    }

    __END__;
}


/* End of file. */
