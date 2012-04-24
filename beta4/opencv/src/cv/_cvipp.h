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

#ifndef _CV_IPP_H_
#define _CV_IPP_H_

//////////////////////////////////////// Moments ////////////////////////////////////////

#define IPCV_DEF_MOMENTS( flavor, srctype )                         \
IPCVAPI_EX( CvStatus, icvMomentsInTile_##flavor##_CnCR,             \
"ippiMomentsGray_" #flavor "_CnCR", CV_PLUGINS1(CV_PLUGIN_OPTCV),   \
( const srctype* img, int step, CvSize size, int cn, int coi, double *moments ))

IPCV_DEF_MOMENTS( 8u, uchar )
IPCV_DEF_MOMENTS( 16u, ushort )
IPCV_DEF_MOMENTS( 16s, short )
IPCV_DEF_MOMENTS( 32f, float )
IPCV_DEF_MOMENTS( 64f, double )

#define IPCV_DEF_BINARY_MOMENTS( flavor, srctype )                  \
IPCVAPI_EX( CvStatus, icvMomentsInTileBin_##flavor##_CnCR,          \
"ippiMomentsBinary_" #flavor "_CnCR", CV_PLUGINS1(CV_PLUGIN_OPTCV), \
( const srctype* img, int step, CvSize size, int cn, int coi, double *moments ))

IPCV_DEF_BINARY_MOMENTS( 8u, uchar )
IPCV_DEF_BINARY_MOMENTS( 16s, ushort )
IPCV_DEF_BINARY_MOMENTS( 32f, int )
IPCV_DEF_BINARY_MOMENTS( 64f, int64 )

#undef IPCV_DEF_MOMENTS
#undef IPCV_DEF_BINARY_MOMENTS

/****************************************************************************************\
*                                  Background differencing                               *
\****************************************************************************************/

/////////////////////////////////// Accumulation /////////////////////////////////////////

#define IPCV_ACCUM( flavor, arrtype, acctype )                                      \
IPCVAPI_EX( CvStatus, icvAddSquare_##flavor##_C1IR,                                 \
    "ippiAddSquare_" #flavor "_C1IR", CV_PLUGINS1(CV_PLUGIN_IPPCV),                 \
    ( const arrtype* src, int srcstep, acctype* dst, int dststep, CvSize size ))    \
IPCVAPI_EX( CvStatus, icvAddProduct_##flavor##_C1IR,                                \
    "ippiAddProduct_" #flavor "_C1IR", CV_PLUGINS1(CV_PLUGIN_IPPCV),                \
    ( const arrtype* src1, int srcstep1, const arrtype* src2, int srcstep2,         \
      acctype* dst, int dststep, CvSize size ))                                     \
IPCVAPI_EX( CvStatus, icvAddWeighted_##flavor##_C1IR,                               \
    "ippiAddWeighted_" #flavor "_C1IR", CV_PLUGINS1(CV_PLUGIN_IPPCV),               \
    ( const arrtype* src, int srcstep, acctype* dst, int dststep,                   \
      CvSize size, acctype alpha ))                                                 \
                                                                                    \
IPCVAPI_EX( CvStatus, icvAdd_##flavor##_C1IMR,                                      \
    "ippiAdd_" #flavor "_C1IMR", CV_PLUGINS1(CV_PLUGIN_IPPCV),                      \
    ( const arrtype* src, int srcstep, const uchar* mask, int maskstep,             \
      acctype* dst, int dststep, CvSize size ))                                     \
IPCVAPI_EX( CvStatus, icvAddSquare_##flavor##_C1IMR,                                \
    "ippiAddSquare_" #flavor "_C1IMR", CV_PLUGINS1(CV_PLUGIN_IPPCV),                \
    ( const arrtype* src, int srcstep, const uchar* mask, int maskstep,             \
      acctype* dst, int dststep, CvSize size ))                                     \
IPCVAPI_EX( CvStatus, icvAddProduct_##flavor##_C1IMR,                               \
    "ippiAddProduct_" #flavor "_C1IMR", CV_PLUGINS1(CV_PLUGIN_IPPCV),               \
    ( const arrtype* src1, int srcstep1, const arrtype* src2, int srcstep2,         \
      const uchar* mask, int maskstep, acctype* dst, int dststep, CvSize size ))    \
IPCVAPI_EX( CvStatus, icvAddWeighted_##flavor##_C1IMR,                              \
    "ippiAddWeighted_" #flavor "_C1IMR", CV_PLUGINS1(CV_PLUGIN_IPPCV),              \
    ( const arrtype* src, int srcstep, const uchar* mask, int maskstep,             \
      acctype* dst, int dststep, CvSize size, acctype alpha ))                      \
                                                                                    \
IPCVAPI_EX( CvStatus, icvAdd_##flavor##_C3IMR,                                      \
    "ippiAdd_" #flavor "_C3IMR", CV_PLUGINS2(CV_PLUGIN_IPPCV,CV_PLUGIN_OPTCV),      \
    ( const arrtype* src, int srcstep, const uchar* mask, int maskstep,             \
      acctype* dst, int dststep, CvSize size ))                                     \
IPCVAPI_EX( CvStatus, icvAddSquare_##flavor##_C3IMR,                                \
    "ippiAddSquare_" #flavor "_C3IMR", CV_PLUGINS2(CV_PLUGIN_IPPCV,CV_PLUGIN_OPTCV),\
    ( const arrtype* src, int srcstep, const uchar* mask, int maskstep,             \
      acctype* dst, int dststep, CvSize size ))                                     \
IPCVAPI_EX( CvStatus, icvAddProduct_##flavor##_C3IMR,                               \
    "ippiAddProduct_" #flavor "_C3IMR", CV_PLUGINS2(CV_PLUGIN_IPPCV,CV_PLUGIN_OPTCV),\
    ( const arrtype* src1, int srcstep1, const arrtype* src2, int srcstep2,         \
      const uchar* mask, int maskstep, acctype* dst, int dststep, CvSize size ))    \
IPCVAPI_EX( CvStatus, icvAddWeighted_##flavor##_C3IMR,                              \
    "ippiAddWeighted_" #flavor "_C3IMR", CV_PLUGINS2(CV_PLUGIN_IPPCV,CV_PLUGIN_OPTCV),\
    ( const arrtype* src, int srcstep, const uchar* mask, int maskstep,             \
      acctype* dst, int dststep, CvSize size, acctype alpha ))

IPCVAPI_EX( CvStatus, icvAdd_8u32f_C1IR,
    "ippiAdd_8u32f_C1IR", CV_PLUGINS1(CV_PLUGIN_IPPCV),
    ( const uchar* src, int srcstep, float* dst, int dststep, CvSize size ))

IPCV_ACCUM( 8u32f, uchar, float )
IPCV_ACCUM( 32f, float, float )

#undef IPCV_ACCUM

/****************************************************************************************\
*                                         Samplers                                       *
\****************************************************************************************/

////////////////////////////////////// GetRectSubPix ////////////////////////////////////////

#define IPCV_GET_RECT_SUB_PIX( flavor, cn, srctype, dsttype )       \
IPCVAPI_EX( CvStatus, icvGetRectSubPix_##flavor##_C##cn##R,         \
"ippiGetRectSubPix_" #flavor "_C" #cn "R", CV_PLUGINS2(CV_PLUGIN_OPTCV,CV_PLUGIN_IPPCV),\
( const srctype* src, int src_step, CvSize src_size,                \
  dsttype* dst, int dst_step, CvSize win_size, CvPoint2D32f center ))

IPCV_GET_RECT_SUB_PIX( 8u, 1, uchar, uchar )
IPCV_GET_RECT_SUB_PIX( 8u32f, 1, uchar, float )
IPCV_GET_RECT_SUB_PIX( 32f, 1, float, float )

IPCV_GET_RECT_SUB_PIX( 8u, 3, uchar, uchar )
IPCV_GET_RECT_SUB_PIX( 8u32f, 3, uchar, float )
IPCV_GET_RECT_SUB_PIX( 32f, 3, float, float )

#define IPCV_GET_QUADRANGLE_SUB_PIX( flavor, cn, srctype, dsttype ) \
IPCVAPI_EX( CvStatus, icvGetQuadrangleSubPix_##flavor##_C##cn##R,   \
"ippiGetQuadrangeRectSubPix_" #flavor "_C" #cn "R", CV_PLUGINS2(CV_PLUGIN_OPTCV,CV_PLUGIN_IPPCV),\
( const srctype* src, int src_step, CvSize src_size,                \
  dsttype* dst, int dst_step, CvSize win_size,                      \
  const float *matrix, int fillOutliers, dsttype* fillValue ))

IPCV_GET_QUADRANGLE_SUB_PIX( 8u, 1, uchar, uchar )
IPCV_GET_QUADRANGLE_SUB_PIX( 8u32f, 1, uchar, float )
IPCV_GET_QUADRANGLE_SUB_PIX( 32f, 1, float, float )

IPCV_GET_QUADRANGLE_SUB_PIX( 8u, 3, uchar, uchar )
IPCV_GET_QUADRANGLE_SUB_PIX( 8u32f, 3, uchar, float )
IPCV_GET_QUADRANGLE_SUB_PIX( 32f, 3, float, float )

#undef IPCV_GET_RECT_SUB_PIX
#undef IPCV_GET_QUADRANGLE_SUB_PIX


/****************************************************************************************\
*                                        Pyramids                                        *
\****************************************************************************************/

IPCVAPI_EX( CvStatus, icvPyrUpGetBufSize_Gauss5x5, "ippiPyrUpGetBufSize_Gauss5x5", CV_PLUGINS1(CV_PLUGIN_IPPCV),
        ( int roiWidth, CvDataType dataType, int channels, int* bufSize))

IPCVAPI_EX( CvStatus, icvPyrDownGetBufSize_Gauss5x5, "ippiPyrDownGetBufSize_Gauss5x5", CV_PLUGINS1(CV_PLUGIN_IPPCV),
        ( int roiWidth, CvDataType dataType, int channels, int* bufSize))

#define ICV_PYRAMID( name, flavor, arrtype )                    \
IPCVAPI_EX( CvStatus, icv##name##_##flavor##_C1R,               \
"ippi" #name "_" #flavor "_C1R", CV_PLUGINS1(CV_PLUGIN_IPPCV),  \
( const arrtype* pSrc, int srcStep, arrtype* pDst, int dstStep, \
  CvSize roiSize, void* pBuffer ))                              \
IPCVAPI_EX( CvStatus, icv##name##_##flavor##_C3R,               \
"ippi" #name "_" #flavor "_C3R", CV_PLUGINS1(CV_PLUGIN_IPPCV),  \
( const arrtype* pSrc, int srcStep, arrtype* pDst, int dstStep, \
  CvSize roiSize, void* pBuffer ))

ICV_PYRAMID( PyrUp_Gauss5x5, 8u, uchar )
ICV_PYRAMID( PyrUp_Gauss5x5, 32f, float )
ICV_PYRAMID( PyrUp_Gauss5x5, 64f, double )

ICV_PYRAMID( PyrDown_Gauss5x5, 8u, uchar )
ICV_PYRAMID( PyrDown_Gauss5x5, 32f, float )
ICV_PYRAMID( PyrDown_Gauss5x5, 64f, double )

#undef ICV_PYRAMID

/****************************************************************************************\
*                                      Morphology                                        *
\****************************************************************************************/

#define IPCV_MORPH_PLUGINS 0
//#define IPCV_MORPH_PLUGINS CV_PLUGINS1(CV_PLUGIN_IPPCV)

#define IPCV_MORPHOLOGY( name )                                             \
IPCVAPI_EX(CvStatus, icv##name##_8u_C1R,                                    \
           "ippi" #name "_8u_C1R", IPCV_MORPH_PLUGINS,                      \
           ( const uchar* pSrc, int srcStep, uchar* pDst, int dstStep,      \
             CvSize* roiSize, struct CvMorphState* state, int stage ))      \
                                                                            \
IPCVAPI_EX(CvStatus, icv##name##_8u_C3R,                                    \
           "ippi" #name "_8u_C3R", IPCV_MORPH_PLUGINS,                      \
           ( const uchar* pSrc, int srcStep, uchar* pDst, int dstStep,      \
             CvSize* roiSize, struct CvMorphState* state, int stage ))      \
                                                                            \
IPCVAPI_EX(CvStatus, icv##name##_8u_C4R,                                    \
           "ippi" #name "_8u_C4R", IPCV_MORPH_PLUGINS,                      \
           ( const uchar* pSrc, int srcStep, uchar* pDst, int dstStep,      \
             CvSize* roiSize, struct CvMorphState* state, int stage ))      \
                                                                            \
IPCVAPI_EX(CvStatus, icv##name##_32f_C1R,                                   \
           "ippi" #name "_32f_C1R", IPCV_MORPH_PLUGINS,                     \
           ( const int* pSrc, int srcStep, int* pDst, int dstStep,          \
             CvSize* roiSize, struct CvMorphState* state, int stage ))      \
                                                                            \
IPCVAPI_EX(CvStatus, icv##name##_32f_C3R,                                   \
           "ippi" #name "_32f_C3R", IPCV_MORPH_PLUGINS,                     \
           ( const int* pSrc, int srcStep, int* pDst, int dstStep,          \
             CvSize* roiSize, struct CvMorphState* state, int stage ))      \
                                                                            \
IPCVAPI_EX(CvStatus, icv##name##_32f_C4R,                                   \
           "ippi" #name "_32f_C4R", IPCV_MORPH_PLUGINS,                     \
           ( const int* pSrc, int srcStep, int* pDst, int dstStep,          \
             CvSize* roiSize, struct CvMorphState* state, int stage ))


IPCV_MORPHOLOGY( ErodeStrip_Rect )
IPCV_MORPHOLOGY( ErodeStrip_Cross )
IPCV_MORPHOLOGY( ErodeStrip )

IPCV_MORPHOLOGY( DilateStrip_Rect )
IPCV_MORPHOLOGY( DilateStrip_Cross )
IPCV_MORPHOLOGY( DilateStrip )

#undef IPCV_MORPHOLOGY

IPCVAPI_EX(CvStatus, icvMorphologyInitAlloc, "ippiMorphologyInitAlloc", IPCV_MORPH_PLUGINS,
    ( int roiWidth, CvDataType dataType, int channels, CvSize elSize, CvPoint elAnchor,
      int elShape, int* elData, struct CvMorphState** morphState ))

IPCVAPI_EX( CvStatus, icvMorphologyFree, "ippiMorphologyFree", IPCV_MORPH_PLUGINS,
    ( struct CvMorphState** morphState ))

#undef IPCV_MORPH_PLUGINS

/****************************************************************************************\
*                                  Motion Template                                       *
\****************************************************************************************/

IPCVAPI_EX( CvStatus, icvUpdateMotionHistory_8u32f_C1IR,
    "ippiUpdateMotionHistory_8u32f_C1IR", CV_PLUGINS1(CV_PLUGIN_IPPCV),
    ( const uchar* silIm, int silStep, float* mhiIm, int mhiStep,
      CvSize size,float  timestamp, float  mhi_duration ))

/****************************************************************************************\
*                                      Template matching                                 *
\****************************************************************************************/

#define ICV_MATCHTEMPLATE_BUFSIZE( comp_type )                  \
IPCVAPI_EX( CvStatus, icvMatchTemplateGetBufSize_##comp_type,   \
    "ippiMatchTemplateGetBufSize_" #comp_type, 0/*CV_PLUGINS1(CV_PLUGIN_IPPCV)*/,         \
    ( CvSize roiSize, CvSize templSize, CvDataType dataType, int* bufferSize ))

ICV_MATCHTEMPLATE_BUFSIZE( SqDiff )
ICV_MATCHTEMPLATE_BUFSIZE( SqDiffNormed )
ICV_MATCHTEMPLATE_BUFSIZE( Corr )
ICV_MATCHTEMPLATE_BUFSIZE( CorrNormed )
ICV_MATCHTEMPLATE_BUFSIZE( Coeff )
ICV_MATCHTEMPLATE_BUFSIZE( CoeffNormed )

#undef ICV_MATCHTEMPLATE_BUFSIZE

#define ICV_MATCHTEMPLATE( comp_type )                          \
IPCVAPI_EX( CvStatus, icvMatchTemplate_##comp_type##_8u32f_C1R, \
    "ippiMatchTemplate_" #comp_type "_8u32f_C1R", 0/*CV_PLUGINS1(CV_PLUGIN_IPPCV)*/,      \
    (const uchar* pImage, int imageStep, CvSize roiSize,        \
    const uchar* pTemplate, int templStep, CvSize templSize,    \
    float* pResult, int resultStep, void* pBuffer ))            \
                                                                \
IPCVAPI_EX( CvStatus, icvMatchTemplate_##comp_type##_32f_C1R,   \
    "ippiMatchTemplate_" #comp_type "_32f_C1R", 0/*CV_PLUGINS1(CV_PLUGIN_IPPCV)*/,        \
    (const float* pImage, int imageStep, CvSize roiSize,        \
    const float* pTemplate, int templStep, CvSize templSize,    \
    float* pResult, int resultStep, void* pBuffer ))


ICV_MATCHTEMPLATE( SqDiff )
ICV_MATCHTEMPLATE( SqDiffNormed )
ICV_MATCHTEMPLATE( Corr )
ICV_MATCHTEMPLATE( CorrNormed )
ICV_MATCHTEMPLATE( Coeff )
ICV_MATCHTEMPLATE( CoeffNormed )

#undef ICV_MATCHTEMPLATE

/****************************************************************************************/
/*                                Distance Transform                                    */
/****************************************************************************************/

IPCVAPI_EX(CvStatus, icvDistanceTransform_3x3_8u32f_C1R,
    "ippiDistanceTransform_3x3_8u32f_C1R", CV_PLUGINS1(CV_PLUGIN_IPPCV),
    ( const uchar* pSrc, int srcStep, float* pDst,
      int dstStep, CvSize roiSize, float* pMetrics ))

IPCVAPI_EX(CvStatus, icvDistanceTransform_5x5_8u32f_C1R,
    "ippiDistanceTransform_5x5_8u32f_C1R", CV_PLUGINS1(CV_PLUGIN_IPPCV),
    ( const uchar* pSrc, int srcStep, float* pDst,
      int dstStep, CvSize roiSize, float* pMetrics ))

IPCVAPI_EX( CvStatus, icvGetDistanceTransformMask,
    "ippiGetDistanceTransformMask", CV_PLUGINS1(CV_PLUGIN_IPPCV),
    ( int maskType, float* pMetrics ))

/****************************************************************************************\
*                                  Pyramid segmentation                                  *
\****************************************************************************************/

IPCVAPI_EX( CvStatus,  icvUpdatePyrLinks_8u_C1, "ippiUpdatePyrLinks_8u_C1", CV_PLUGINS1(CV_PLUGIN_OPTCV),
           ( int layer, void* layer_data, CvSize size, void* parent_layer,
             void* writer, float threshold, int is_last_iter, void* stub,
             CvWriteNodeFunction ))

IPCVAPI_EX( CvStatus,  icvUpdatePyrLinks_8u_C3, "ippiUpdatePyrLinks_8u_C3", CV_PLUGINS1(CV_PLUGIN_OPTCV),
           ( int layer, void* layer_data, CvSize size, void* parent_layer,
             void* writer, float threshold, int is_last_iter, void* stub,
             CvWriteNodeFunction ))

/****************************************************************************************\
*                                      Lens undistortion                                 *
\****************************************************************************************/

IPCVAPI_EX( CvStatus, icvUnDistort1_8u_C1R,
    "ippiUnDistort1_8u_C1R", CV_PLUGINS1(CV_PLUGIN_OPTCV),
    ( const uchar* srcImage, int srcStep,
    uchar* dstImage, int dstStep,
    CvSize size, const float* intrMatrix,
    const float* distCoeffs, int interToggle ))

IPCVAPI_EX( CvStatus, icvUnDistort1_8u_C3R,
    "ippiUnDistort1_8u_C3R", CV_PLUGINS1(CV_PLUGIN_OPTCV),
    ( const uchar* srcImage, int srcStep,
    uchar* dstImage, int dstStep,
    CvSize size, const float* intrMatrix,
    const float* distCoeffs, int interToggle ))

IPCVAPI_EX( CvStatus, icvUnDistortEx_8u_C1R,
    "ippiUnDistortEx_8u_C1R", CV_PLUGINS1(CV_PLUGIN_OPTCV),
    ( const uchar* srcImage, int srcStep,
    uchar* dstImage, int dstStep,
    CvSize size, const float* intrMatrix,
    const float* distCoeffs, int interToggle ))

IPCVAPI_EX( CvStatus, icvUnDistortEx_8u_C3R,
    "ippiUnDistortEx_8u_C3R", CV_PLUGINS1(CV_PLUGIN_OPTCV),    
    ( const uchar* srcImage, int srcStep,
    uchar* dstImage, int dstStep,
    CvSize size, const float* intrMatrix,
    const float* distCoeffs, int interToggle ))

IPCVAPI_EX( CvStatus, icvUnDistortInit,
    "ippiUnDistortInit", CV_PLUGINS1(CV_PLUGIN_OPTCV),    
    ( int srcStep, int* map, int mapStep, CvSize size,
      const float* intrMatrix, const float* distCoeffs,
      int interToggle, int pixSize ))

IPCVAPI_EX( CvStatus, icvUnDistort_8u_C1R,
    "ippiUnDistort_8u_C1R", CV_PLUGINS1(CV_PLUGIN_OPTCV),        
    ( const uchar* srcImage, int srcStep, const int* map, int mapstep,
      uchar* dstImage, int dstStep, CvSize size, int interToggle ))

IPCVAPI_EX( CvStatus, icvUnDistort_8u_C3R,
    "ippiUnDistort_8u_C3R", CV_PLUGINS1(CV_PLUGIN_OPTCV),        
    ( const uchar* srcImage, int srcStep, const int* map, int mapstep,
      uchar* dstImage, int dstStep, CvSize size, int interToggle ))

/****************************************************************************************\
*                               Thresholding functions                                   *
\****************************************************************************************/

IPCVAPI_EX(CvStatus, icvThresh_8u_C1R, "ippiThresh_8u_C1R", CV_PLUGINS1(CV_PLUGIN_OPTCV),
           ( const uchar*  src, int  src_step, uchar*  dst, int  dst_step,
             CvSize  roi, int  thresh, uchar max_val, int type ))

IPCVAPI_EX(CvStatus, icvThresh_32f_C1R, "ippiThresh_32f_C1R", CV_PLUGINS1(CV_PLUGIN_OPTCV),
           ( const float*  src, int  src_step, float*  dst, int  dst_step,
             CvSize  roi, float  thresh, float max_val, int type))

#if 0

/****************************************************************************************\
*                                 Geometrical transforms                                 *
\****************************************************************************************/

IPCVAPI_EX( CvStatus, icvResize_NN_8u_C1R,
    "ippiResize_NN_8u_C1R", CV_PLUGINS1(CV_PLUGIN_OPTCV),
    ( const uchar* src, int srcstep, CvSize srcsize,
      uchar* dst, int dststep, CvSize dstsize, int pix_size ))

#define IPCV_RESIZE_BILINEAR( flavor, cn, arrtype )                     \
IPCVAPI_EX( CvStatus, icvResize_Bilinear_##flavor##_C##cn##R,           \
    "ippiResize_Bilinear_" #flavor "_C" #cn "R", CV_PLUGINS1(CV_PLUGIN_OPTCV),   \
    ( const arrtype* src, int srcstep, CvSize srcsize,                  \
      arrtype* dst, int dststep, CvSize dstsize ))

IPCV_RESIZE_BILINEAR( 8u, 1, uchar )
IPCV_RESIZE_BILINEAR( 8u, 2, uchar )
IPCV_RESIZE_BILINEAR( 8u, 3, uchar )
IPCV_RESIZE_BILINEAR( 8u, 4, uchar )

IPCV_RESIZE_BILINEAR( 32f, 1, float )
IPCV_RESIZE_BILINEAR( 32f, 2, float )
IPCV_RESIZE_BILINEAR( 32f, 3, float )
IPCV_RESIZE_BILINEAR( 32f, 4, float )

/****************************************************************************************\
*                                 Canny Edge Detector                                    *
\****************************************************************************************/

IPCVAPI( CvStatus, icvCannyGetSize, ( CvSize roiSize, int* bufferSize ))

IPCVAPI( CvStatus, icvCanny_16s8u_C1R, ( const short* pSrcDx, int srcDxStep,
                                         const short* pSrcDy, int srcDyStep,
                                         uchar*  pDstEdges, int dstEdgeStep, 
                                         CvSize roiSize, float  lowThresh,
                                         float  highThresh, void* pBuffer ))
/****************************************************************************************\
*                                 Blur (w/o scaling)                                     *
\****************************************************************************************/

/********************* cxcore functions ***********************/

IPCVAPI_EX( CvStatus, icvCopy_8u_C1R, "ippiCopy_8u_C1R", "ippi",
                  ( const uchar* src, int src_step,
                    uchar* dst, int dst_step, CvSize size ))

IPCVAPI_EX( CvStatus, icvbFastArctan_32f, "ippibFastArctan_32f", CV_PLUGINS1(CV_PLUGIN_IPPCV),
                                    ( const float* y, const float* x, float* angle, int len ))

#define IPCV_MULTRANS( letter, flavor, arrtype )                \
IPCVAPI_EX( CvStatus, icvMulTransposed##letter##_##flavor,      \
            "ippiMulTransposed_" #letter "_" #flavor, CV_PLUGINS1(CV_PLUGIN_OPTCV),  \
            ( const arrtype* src, int srcstep,                  \
              arrtype* dst, int dststep, CvSize size ))

IPCV_MULTRANS( R, 32f, float )
IPCV_MULTRANS( R, 64f, double )
IPCV_MULTRANS( L, 32f, float )
IPCV_MULTRANS( L, 64f, double )

#define IPCV_CVT_TO( flavor )                                                       \
IPCVAPI_EX( CvStatus, icvCvtTo_##flavor##_C1R, "", "", ( const void* src, int step1,\
                      void* dst, int step, CvSize size, int param ))
IPCV_CVT_TO( 32f )

#endif

#endif /*_CV_PLUGIN_H_*/

