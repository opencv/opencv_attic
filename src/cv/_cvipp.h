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

/****************************************************************************************\
*                                  Creating Borders                                      *
\****************************************************************************************/

#define IPCV_COPY_BORDER( bordertype, flavor, arrtype )                             \
IPCVAPI_EX( CvStatus, icvCopy##bordertype##Border_##flavor,                         \
            "ippiCopy" #bordertype "Border_" #flavor, CV_PLUGINS1(CV_PLUGIN_IPPI),  \
            ( const arrtype* pSrc,  int srcStep, CvSize srcRoiSize,                 \
                    arrtype* pDst,  int dstStep, CvSize dstRoiSize,                 \
                    int topBorderWidth, int leftBorderWidth ))

IPCV_COPY_BORDER( Replicate, 8u_C1R, uchar )
IPCV_COPY_BORDER( Replicate, 16s_C1R, ushort )
IPCV_COPY_BORDER( Replicate, 8u_C3R, uchar )
IPCV_COPY_BORDER( Replicate, 32s_C1R, int )
IPCV_COPY_BORDER( Replicate, 16s_C3R, ushort )
IPCV_COPY_BORDER( Replicate, 16s_C4R, int )
IPCV_COPY_BORDER( Replicate, 32s_C3R, int )
IPCV_COPY_BORDER( Replicate, 32s_C4R, int )
IPCV_COPY_BORDER( Replicate, 64f_C3R, int )
IPCV_COPY_BORDER( Replicate, 64f_C4R, int )

#undef IPCV_COPY_BORDER

#define IPCV_COPY_CONST_BORDER_C1( flavor, arrtype )                                \
IPCVAPI_EX( CvStatus, icvCopyConstBorder_##flavor,                                  \
            "ippiCopyConstBorder_" #flavor, CV_PLUGINS1(CV_PLUGIN_IPPI),            \
            ( const arrtype* pSrc,  int srcStep, CvSize srcRoiSize,                 \
                    arrtype* pDst,  int dstStep, CvSize dstRoiSize,                 \
                    int topBorderWidth, int leftBorderWidth, arrtype value ))

IPCV_COPY_CONST_BORDER_C1( 8u_C1R, uchar )
IPCV_COPY_CONST_BORDER_C1( 16s_C1R, ushort )
IPCV_COPY_CONST_BORDER_C1( 32s_C1R, int )

#undef IPCV_COPY_CONST_BORDER_C1

#define IPCV_COPY_CONST_BORDER_CN( flavor, arrtype )                                \
IPCVAPI_EX( CvStatus, icvCopyConstBorder_##flavor,                                  \
            "ippiCopyConstBorder_" #flavor, CV_PLUGINS1(CV_PLUGIN_IPPI),            \
            ( const arrtype* pSrc,  int srcStep, CvSize srcRoiSize,                 \
                    arrtype* pDst,  int dstStep, CvSize dstRoiSize,                 \
                    int topBorderWidth, int leftBorderWidth, const arrtype* value ))

IPCV_COPY_CONST_BORDER_CN( 8u_C3R, uchar )
IPCV_COPY_CONST_BORDER_CN( 16s_C3R, ushort )
IPCV_COPY_CONST_BORDER_CN( 16s_C4R, int )
IPCV_COPY_CONST_BORDER_CN( 32s_C3R, int )
IPCV_COPY_CONST_BORDER_CN( 32s_C4R, int )
IPCV_COPY_CONST_BORDER_CN( 64f_C3R, int )
IPCV_COPY_CONST_BORDER_CN( 64f_C4R, int )

#undef IPCV_COPY_CONST_BORDER_CN

/****************************************************************************************\
*                                        Moments                                         *
\****************************************************************************************/

#define IPCV_MOMENTS( suffix, ipp_suffix, cn )                      \
IPCVAPI_EX( CvStatus, icvMoments##suffix##_C##cn##R,                \
"ippiMoments" #ipp_suffix "_C" #cn "R", CV_PLUGINS1(CV_PLUGIN_IPPI),\
( const void* img, int step, CvSize size, void* momentstate ))

IPCV_MOMENTS( _8u, 64f_8u, 1 )
IPCV_MOMENTS( _32f, 64f_32f, 1 )

#undef IPCV_MOMENTS

IPCVAPI_EX( CvStatus, icvMomentInitAlloc_64f,
            "ippiMomentInitAlloc_64f", CV_PLUGINS1(CV_PLUGIN_IPPI),
            (void** momentstate, CvHintAlgorithm hint ))

IPCVAPI_EX( CvStatus, icvMomentFree_64f,
            "ippiMomentFree_64f", CV_PLUGINS1(CV_PLUGIN_IPPI),
            (void* momentstate ))

IPCVAPI_EX( CvStatus, icvGetSpatialMoment_64f,
            "ippiGetSpatialMoment_64f", CV_PLUGINS1(CV_PLUGIN_IPPI),
            (const void* momentstate, int mOrd, int nOrd,
             int nChannel, CvPoint roiOffset, double* value ))

/****************************************************************************************\
*                                  Background differencing                               *
\****************************************************************************************/

/////////////////////////////////// Accumulation /////////////////////////////////////////

#define IPCV_ACCUM( flavor, arrtype, acctype )                                      \
IPCVAPI_EX( CvStatus, icvAdd_##flavor##_C1IR,                                       \
    "ippiAdd_" #flavor "_C1IR", CV_PLUGINS1(CV_PLUGIN_IPPCV),                       \
    ( const arrtype* src, int srcstep, acctype* dst, int dststep, CvSize size ))    \
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
    "ippiAdd_" #flavor "_C3IMR", CV_PLUGINS1(CV_PLUGIN_IPPCV),                      \
    ( const arrtype* src, int srcstep, const uchar* mask, int maskstep,             \
      acctype* dst, int dststep, CvSize size ))                                     \
IPCVAPI_EX( CvStatus, icvAddSquare_##flavor##_C3IMR,                                \
    "ippiAddSquare_" #flavor "_C3IMR", CV_PLUGINS1(CV_PLUGIN_IPPCV),                \
    ( const arrtype* src, int srcstep, const uchar* mask, int maskstep,             \
      acctype* dst, int dststep, CvSize size ))                                     \
IPCVAPI_EX( CvStatus, icvAddProduct_##flavor##_C3IMR,                               \
    "ippiAddProduct_" #flavor "_C3IMR", CV_PLUGINS1(CV_PLUGIN_IPPCV),               \
    ( const arrtype* src1, int srcstep1, const arrtype* src2, int srcstep2,         \
      const uchar* mask, int maskstep, acctype* dst, int dststep, CvSize size ))    \
IPCVAPI_EX( CvStatus, icvAddWeighted_##flavor##_C3IMR,                              \
    "ippiAddWeighted_" #flavor "_C3IMR", CV_PLUGINS1(CV_PLUGIN_IPPCV),              \
    ( const arrtype* src, int srcstep, const uchar* mask, int maskstep,             \
      acctype* dst, int dststep, CvSize size, acctype alpha ))

IPCV_ACCUM( 8u32f, uchar, float )
IPCV_ACCUM( 32f, float, float )

#undef IPCV_ACCUM

/****************************************************************************************\
*                                        Samplers                                        *
\****************************************************************************************/

////////////////////////////////////// GetRectSubPix ////////////////////////////////////////

#define IPCV_GET_RECT_SUB_PIX( flavor, cn, srctype, dsttype )       \
IPCVAPI_EX( CvStatus, icvGetRectSubPix_##flavor##_C##cn##R,         \
"ippiGetRectSubPix_" #flavor "_C" #cn "R",                          \
CV_PLUGINS1(CV_PLUGIN_IPPCV),                                       \
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
"ippiGetQuadrangeRectSubPix_" #flavor "_C" #cn "R",                 \
CV_PLUGINS1(CV_PLUGIN_IPPCV),                                       \
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
*                                       Pyramids                                         *
\****************************************************************************************/

IPCVAPI_EX( CvStatus, icvPyrDownGetBufSize_Gauss5x5,
           "ippiPyrDownGetBufSize_Gauss5x5", CV_PLUGINS1(CV_PLUGIN_IPPCV),
            ( int roiWidth, CvDataType dataType, int channels, int* bufSize ))

IPCVAPI_EX( CvStatus, icvPyrUpGetBufSize_Gauss5x5,
           "ippiPyrUpGetBufSize_Gauss5x5", CV_PLUGINS1(CV_PLUGIN_IPPCV),
            ( int roiWidth, CvDataType dataType, int channels, int* bufSize ))

#define ICV_PYRDOWN( flavor, cn )                                           \
IPCVAPI_EX( CvStatus, icvPyrDown_Gauss5x5_##flavor##_C##cn##R,              \
"ippiPyrDown_Gauss5x5_" #flavor "_C" #cn "R", CV_PLUGINS1(CV_PLUGIN_IPPCV), \
( const void* pSrc, int srcStep, void* pDst, int dstStep,                   \
  CvSize roiSize, void* pBuffer ))

#define ICV_PYRUP( flavor, cn )                                             \
IPCVAPI_EX( CvStatus, icvPyrUp_Gauss5x5_##flavor##_C##cn##R,                \
"ippiPyrUp_Gauss5x5_" #flavor "_C" #cn "R", CV_PLUGINS1(CV_PLUGIN_IPPCV),   \
( const void* pSrc, int srcStep, void* pDst, int dstStep,                   \
  CvSize roiSize, void* pBuffer ))

ICV_PYRDOWN( 8u, 1 )
ICV_PYRDOWN( 8u, 3 )
ICV_PYRDOWN( 32f, 1 )
ICV_PYRDOWN( 32f, 3 )

ICV_PYRUP( 8u, 1 )
ICV_PYRUP( 8u, 3 )
ICV_PYRUP( 32f, 1 )
ICV_PYRUP( 32f, 3 )

#undef ICV_PYRDOWN
#undef ICV_PYRUP

/****************************************************************************************\
*                                Geometric Transformations                               *
\****************************************************************************************/

/*#define IPCV_REMAP( flavor, cn, arrtype )                                   \
IPCVAPI_EX( CvStatus, icvRemap_##flavor##_C##cn##R,                         \
    "ippiRemap_" #flavor "_C" #cn "R", CV_PLUGINS1(CV_PLUGIN_IPPI),         \
    ( const arrtype* src, CvSize srcsize, int srcstep, CvSize srcroi,       \
      const float* xmap, int xmapstep, const float* ymap, int ymapstep,     \
      arrtype* dst, int dststep, CvSize dstroi, int interpolation ))

IPCV_REMAP( 8u, 1, uchar )
IPCV_REMAP( 8u, 3, uchar )
IPCV_REMAP( 8u, 4, uchar )

IPCV_REMAP( 32f, 1, float )
IPCV_REMAP( 32f, 3, float )
IPCV_REMAP( 32f, 4, float )

#undef IPCV_REMAP*/

#define IPCV_RESIZE( flavor, cn )                                           \
IPCVAPI_EX( CvStatus, icvResize_##flavor##_C##cn##R,                        \
            "ippiResize_" #flavor "_C" #cn "R", CV_PLUGINS1(CV_PLUGIN_IPPI),\
           (const void* src, CvSize srcsize, int srcstep, CvRect srcroi,    \
            void* dst, int dststep, CvSize dstroi,                          \
            double xfactor, double yfactor, int interpolation ))

IPCV_RESIZE( 8u, 1 )
IPCV_RESIZE( 8u, 3 )
IPCV_RESIZE( 8u, 4 )

IPCV_RESIZE( 16u, 1 )
IPCV_RESIZE( 16u, 3 )
IPCV_RESIZE( 16u, 4 )

IPCV_RESIZE( 32f, 1 )
IPCV_RESIZE( 32f, 3 )
IPCV_RESIZE( 32f, 4 )

#undef IPCV_RESIZE

#define IPCV_WARPAFFINE_BACK( flavor, cn )                                  \
IPCVAPI_EX( CvStatus, icvWarpAffineBack_##flavor##_C##cn##R,                \
    "ippiWarpAffineBack_" #flavor "_C" #cn "R", CV_PLUGINS1(CV_PLUGIN_IPPI),\
    (const void* src, CvSize srcsize, int srcstep, CvRect srcroi,           \
    void* dst, int dststep, CvRect dstroi,                                  \
    const double* coeffs, int interpolate ))

IPCV_WARPAFFINE_BACK( 8u, 1 )
IPCV_WARPAFFINE_BACK( 8u, 3 )
IPCV_WARPAFFINE_BACK( 8u, 4 )

IPCV_WARPAFFINE_BACK( 32f, 1 )
IPCV_WARPAFFINE_BACK( 32f, 3 )
IPCV_WARPAFFINE_BACK( 32f, 4 )

#undef IPCV_WARPAFFINE_BACK

#define IPCV_WARPPERSPECTIVE_BACK( flavor, cn )                             \
IPCVAPI_EX( CvStatus, icvWarpPerspectiveBack_##flavor##_C##cn##R,           \
    "ippiWarpPerspectiveBack_" #flavor "_C" #cn "R", CV_PLUGINS1(CV_PLUGIN_IPPI),\
    (const void* src, CvSize srcsize, int srcstep, CvRect srcroi,           \
    void* dst, int dststep, CvRect dstroi,                                  \
    const double* coeffs, int interpolate ))

IPCV_WARPPERSPECTIVE_BACK( 8u, 1 )
IPCV_WARPPERSPECTIVE_BACK( 8u, 3 )
IPCV_WARPPERSPECTIVE_BACK( 8u, 4 )

IPCV_WARPPERSPECTIVE_BACK( 32f, 1 )
IPCV_WARPPERSPECTIVE_BACK( 32f, 3 )
IPCV_WARPPERSPECTIVE_BACK( 32f, 4 )

#undef IPCV_WARPPERSPECTIVE_BACK


#define IPCV_WARPPERSPECTIVE( flavor, cn )                                  \
IPCVAPI_EX( CvStatus, icvWarpPerspective_##flavor##_C##cn##R,               \
    "ippiWarpPerspective_" #flavor "_C" #cn "R", CV_PLUGINS1(CV_PLUGIN_IPPI),\
    (const void* src, CvSize srcsize, int srcstep, CvRect srcroi,           \
    void* dst, int dststep, CvRect dstroi,                                  \
    const double* coeffs, int interpolate ))

IPCV_WARPPERSPECTIVE( 8u, 1 )
IPCV_WARPPERSPECTIVE( 8u, 3 )
IPCV_WARPPERSPECTIVE( 8u, 4 )

IPCV_WARPPERSPECTIVE( 32f, 1 )
IPCV_WARPPERSPECTIVE( 32f, 3 )
IPCV_WARPPERSPECTIVE( 32f, 4 )

#undef IPCV_WARPPERSPECTIVE

#define IPCV_REMAP( flavor, cn )                                        \
IPCVAPI_EX( CvStatus, icvRemap_##flavor##_C##cn##R,                     \
    "ippiRemap_" #flavor "_C" #cn "R", CV_PLUGINS1(CV_PLUGIN_IPPI),     \
    ( const void* src, CvSize srcsize, int srcstep, CvRect srcroi,      \
      const float* xmap, int xmapstep, const float* ymap, int ymapstep, \
      void* dst, int dststep, CvSize dstsize, int interpolation )) 

IPCV_REMAP( 8u, 1 )
IPCV_REMAP( 8u, 3 )
IPCV_REMAP( 8u, 4 )

IPCV_REMAP( 32f, 1 )
IPCV_REMAP( 32f, 3 )
IPCV_REMAP( 32f, 4 )

#undef IPCV_REMAP

/****************************************************************************************\
*                                      Morphology                                        *
\****************************************************************************************/

#define IPCV_MORPHOLOGY( minmaxtype, morphtype, flavor, cn )            \
IPCVAPI_EX( CvStatus, icv##morphtype##Rect_##flavor##_C##cn##R,         \
            "ippiFilter" #minmaxtype "_" #flavor "_C" #cn "R",          \
            CV_PLUGINS1(CV_PLUGIN_IPPI),                                \
            ( const void* src, int srcstep, void* dst, int dststep,     \
              CvSize roi, CvSize esize, CvPoint anchor ))               \
IPCVAPI_EX( CvStatus, icv##morphtype##Any_##flavor##_C##cn##R,          \
            "ippi" #morphtype "_" #flavor "_C" #cn "R",                 \
            CV_PLUGINS1(CV_PLUGIN_IPPI),                                \
            ( const void* src, int srcstep, void* dst, int dststep,     \
              CvSize roi, const uchar* element,                         \
              CvSize esize, CvPoint anchor ))

IPCV_MORPHOLOGY( Min, Erode, 8u, 1 )
IPCV_MORPHOLOGY( Min, Erode, 8u, 3 )
IPCV_MORPHOLOGY( Min, Erode, 8u, 4 )
IPCV_MORPHOLOGY( Min, Erode, 32f, 1 )
IPCV_MORPHOLOGY( Min, Erode, 32f, 3 )
IPCV_MORPHOLOGY( Min, Erode, 32f, 4 )
IPCV_MORPHOLOGY( Max, Dilate, 8u, 1 )
IPCV_MORPHOLOGY( Max, Dilate, 8u, 3 )
IPCV_MORPHOLOGY( Max, Dilate, 8u, 4 )
IPCV_MORPHOLOGY( Max, Dilate, 32f, 1 )
IPCV_MORPHOLOGY( Max, Dilate, 32f, 3 )
IPCV_MORPHOLOGY( Max, Dilate, 32f, 4 )

#undef IPCV_MORPHOLOGY

/****************************************************************************************\
*                                 Smoothing Filters                                      *
\****************************************************************************************/

#define IPCV_FILTER_MEDIAN( flavor, cn )                                            \
IPCVAPI_EX( CvStatus, icvFilterMedian_##flavor##_C##cn##R,                          \
            "ippiFilterMedian_" #flavor "_C" #cn "R", CV_PLUGINS1(CV_PLUGIN_IPPI),  \
            ( const void* src, int srcstep, void* dst, int dststep,                 \
              CvSize roi, CvSize ksize, CvPoint anchor ))

IPCV_FILTER_MEDIAN( 8u, 1 )
IPCV_FILTER_MEDIAN( 8u, 3 )
IPCV_FILTER_MEDIAN( 8u, 4 )

#define IPCV_FILTER_BOX( flavor, cn )                                               \
IPCVAPI_EX( CvStatus, icvFilterBox_##flavor##_C##cn##R,                             \
            "ippiFilterBox_" #flavor "_C" #cn "R", CV_PLUGINS1(CV_PLUGIN_IPPI),     \
            ( const void* src, int srcstep, void* dst, int dststep,                 \
              CvSize roi, CvSize ksize, CvPoint anchor ))

IPCV_FILTER_BOX( 8u, 1 )
IPCV_FILTER_BOX( 8u, 3 )
IPCV_FILTER_BOX( 8u, 4 )
IPCV_FILTER_BOX( 32f, 1 )
IPCV_FILTER_BOX( 32f, 3 )
IPCV_FILTER_BOX( 32f, 4 )

#undef IPCV_FILTER_BOX

/****************************************************************************************\
*                                 Derivative Filters                                     *
\****************************************************************************************/

#define IPCV_FILTER_SOBEL( suffix, ipp_suffix, flavor )                             \
IPCVAPI_EX( CvStatus, icvFilterSobel##suffix##_##flavor##_C1R,                      \
    "ippiFilterSobel" #ipp_suffix "_" #flavor "_C1R", CV_PLUGINS1(CV_PLUGIN_IPPI),  \
    ( const void* src, int srcstep, void* dst, int dststep, CvSize roi, int aperture ))

IPCV_FILTER_SOBEL( Vert, Vert, 8u16s )
IPCV_FILTER_SOBEL( Horiz, Horiz, 8u16s )
IPCV_FILTER_SOBEL( VertSecond, VertSecond, 8u16s )
IPCV_FILTER_SOBEL( HorizSecond, HorizSecond, 8u16s )
IPCV_FILTER_SOBEL( Cross, Cross, 8u16s )

IPCV_FILTER_SOBEL( Vert, VertMask, 32f )
IPCV_FILTER_SOBEL( Horiz, HorizMask, 32f )
IPCV_FILTER_SOBEL( VertSecond, VertSecond, 32f )
IPCV_FILTER_SOBEL( HorizSecond, HorizSecond, 32f )
IPCV_FILTER_SOBEL( Cross, Cross, 32f )

#undef IPCV_FILTER_SOBEL

#define IPCV_FILTER_SCHARR( suffix, ipp_suffix, flavor )                            \
IPCVAPI_EX( CvStatus, icvFilterScharr##suffix##_##flavor##_C1R,                     \
    "ippiFilterScharr" #ipp_suffix "_" #flavor "_C1R", CV_PLUGINS1(CV_PLUGIN_IPPI), \
    ( const void* src, int srcstep, void* dst, int dststep, CvSize roi ))

IPCV_FILTER_SCHARR( Vert, Vert, 8u16s )
IPCV_FILTER_SCHARR( Horiz, Horiz, 8u16s )
IPCV_FILTER_SCHARR( Vert, Vert, 32f )
IPCV_FILTER_SCHARR( Horiz, Horiz, 32f )

#undef IPCV_FILTER_SCHARR

/****************************************************************************************\
*                                   Generic Filters                                      *
\****************************************************************************************/

#define IPCV_FILTER( suffix, ipp_suffix, cn, ksizetype, anchortype )                    \
IPCVAPI_EX( CvStatus, icvFilter##suffix##_C##cn##R,                                     \
            "ippiFilter" #ipp_suffix "_C" #cn "R", CV_PLUGINS1(CV_PLUGIN_IPPI),         \
            ( const void* src, int srcstep, void* dst, int dststep, CvSize size,        \
              const float* kernel, ksizetype ksize, anchortype anchor ))

IPCV_FILTER( _8u, 32f_8u, 1, CvSize, CvPoint )
IPCV_FILTER( _8u, 32f_8u, 3, CvSize, CvPoint )
IPCV_FILTER( _8u, 32f_8u, 4, CvSize, CvPoint )

IPCV_FILTER( _16s, 32f_16s, 1, CvSize, CvPoint )
IPCV_FILTER( _16s, 32f_16s, 3, CvSize, CvPoint )
IPCV_FILTER( _16s, 32f_16s, 4, CvSize, CvPoint )

IPCV_FILTER( _32f, _32f, 1, CvSize, CvPoint )
IPCV_FILTER( _32f, _32f, 3, CvSize, CvPoint )
IPCV_FILTER( _32f, _32f, 4, CvSize, CvPoint )

IPCV_FILTER( Column_8u, Column32f_8u, 1, int, int )
IPCV_FILTER( Column_8u, Column32f_8u, 3, int, int )
IPCV_FILTER( Column_8u, Column32f_8u, 4, int, int )

IPCV_FILTER( Column_16s, Column32f_16s, 1, int, int )
IPCV_FILTER( Column_16s, Column32f_16s, 3, int, int )
IPCV_FILTER( Column_16s, Column32f_16s, 4, int, int )

IPCV_FILTER( Column_32f, Column_32f, 1, int, int )
IPCV_FILTER( Column_32f, Column_32f, 3, int, int )
IPCV_FILTER( Column_32f, Column_32f, 4, int, int )

IPCV_FILTER( Row_8u, Row32f_8u, 1, int, int )
IPCV_FILTER( Row_8u, Row32f_8u, 3, int, int )
IPCV_FILTER( Row_8u, Row32f_8u, 4, int, int )

IPCV_FILTER( Row_16s, Row32f_16s, 1, int, int )
IPCV_FILTER( Row_16s, Row32f_16s, 3, int, int )
IPCV_FILTER( Row_16s, Row32f_16s, 4, int, int )

IPCV_FILTER( Row_32f, Row_32f, 1, int, int )
IPCV_FILTER( Row_32f, Row_32f, 3, int, int )
IPCV_FILTER( Row_32f, Row_32f, 4, int, int )

#undef IPCV_FILTER


/****************************************************************************************\
*                                  Color Transformations                                 *
\****************************************************************************************/

#define IPCV_COLOR( funcname, ipp_funcname, flavor )                          \
IPCVAPI_EX( CvStatus, icv##funcname##_##flavor##_C3R,                         \
        "ippi" #ipp_funcname "_" #flavor "_C3R", CV_PLUGINS1(CV_PLUGIN_IPPI), \
        ( const void* src, int srcstep, void* dst, int dststep, CvSize size ))

IPCV_COLOR( RGB2XYZ, RGBToXYZ, 8u )
IPCV_COLOR( RGB2XYZ, RGBToXYZ, 16u )
IPCV_COLOR( RGB2XYZ, RGBToXYZ, 32f )
IPCV_COLOR( XYZ2RGB, XYZToRGB, 8u )
IPCV_COLOR( XYZ2RGB, XYZToRGB, 16u )
IPCV_COLOR( XYZ2RGB, XYZToRGB, 32f )

IPCV_COLOR( RGB2HSV, RGBToHSV, 8u )
IPCV_COLOR( HSV2RGB, HSVToRGB, 8u )

IPCV_COLOR( RGB2HLS, RGBToHLS, 8u )
IPCV_COLOR( RGB2HLS, RGBToHLS, 32f )
IPCV_COLOR( HLS2RGB, HLSToRGB, 8u )
IPCV_COLOR( HLS2RGB, HLSToRGB, 32f )

IPCV_COLOR( BGR2Lab, BGRToLab, 8u )
IPCV_COLOR( Lab2BGR, LabToBGR, 8u )

IPCV_COLOR( RGB2Luv, RGBToLUV, 8u )
/*IPCV_COLOR( RGB2Luv, RGBToLUV, 32f )*/
IPCV_COLOR( Luv2RGB, LUVToRGB, 8u )
/*IPCV_COLOR( Luv2RGB, LUVToRGB, 32f )*/

/****************************************************************************************\
*                                  Motion Templates                                      *
\****************************************************************************************/

IPCVAPI_EX( CvStatus, icvUpdateMotionHistory_8u32f_C1IR,
    "ippiUpdateMotionHistory_8u32f_C1IR", CV_PLUGINS1(CV_PLUGIN_IPPCV),
    ( const uchar* silIm, int silStep, float* mhiIm, int mhiStep,
      CvSize size,float  timestamp, float  mhi_duration ))

/****************************************************************************************\
*                                 Cross Correlation                                      *
\****************************************************************************************/

/*#define ICV_CROSSCORR_DIRECT( flavor, arrtype, corrtype )       \
IPCVAPI_EX( CvStatus, icvCrossCorrDirect_##flavor##_CnR,        \
  "ippiCrossCorrDirect_" #flavor "_CnR", CV_PLUGINS1(CV_PLUGIN_IPPCV),\
( const arrtype* img0, int imgstep, CvSize imgsize,             \
  const arrtype* templ0, int templstep, CvSize templsize,       \
  corrtype* corr, int corrstep, CvSize corrsize, int cn ))

ICV_CROSSCORR_DIRECT( 8u32f, uchar, float )
ICV_CROSSCORR_DIRECT( 32f, float, float )
ICV_CROSSCORR_DIRECT( 64f, double, double )

#undef ICV_CROSSCORR_DIRECT*/

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
*                                      Lens undistortion                                 *
\****************************************************************************************/

IPCVAPI_EX( CvStatus, icvUnDistort1_8u_C1R,
    "ippiUnDistort1_8u_C1R", CV_PLUGINS1(CV_PLUGIN_IPPCV),
    ( const uchar* srcImage, int srcStep,
    uchar* dstImage, int dstStep,
    CvSize size, const float* intrMatrix,
    const float* distCoeffs, int interToggle ))

IPCVAPI_EX( CvStatus, icvUnDistort1_8u_C3R,
    "ippiUnDistort1_8u_C3R", CV_PLUGINS1(CV_PLUGIN_IPPCV),
    ( const uchar* srcImage, int srcStep,
    uchar* dstImage, int dstStep,
    CvSize size, const float* intrMatrix,
    const float* distCoeffs, int interToggle ))

IPCVAPI_EX( CvStatus, icvUnDistortEx_8u_C1R,
    "ippiUnDistortEx_8u_C1R", CV_PLUGINS1(CV_PLUGIN_IPPCV),
    ( const uchar* srcImage, int srcStep,
    uchar* dstImage, int dstStep,
    CvSize size, const float* intrMatrix,
    const float* distCoeffs, int interToggle ))

IPCVAPI_EX( CvStatus, icvUnDistortEx_8u_C3R,
    "ippiUnDistortEx_8u_C3R", CV_PLUGINS1(CV_PLUGIN_IPPCV),    
    ( const uchar* srcImage, int srcStep,
    uchar* dstImage, int dstStep,
    CvSize size, const float* intrMatrix,
    const float* distCoeffs, int interToggle ))

IPCVAPI_EX( CvStatus, icvUnDistortInit,
    "ippiUnDistortInit", CV_PLUGINS1(CV_PLUGIN_IPPCV),    
    ( int srcStep, int* map, int mapStep, CvSize size,
      const float* intrMatrix, const float* distCoeffs,
      int interToggle, int pixSize ))

IPCVAPI_EX( CvStatus, icvUnDistort_8u_C1R,
    "ippiUnDistort_8u_C1R", CV_PLUGINS1(CV_PLUGIN_IPPCV),        
    ( const uchar* srcImage, int srcStep, const int* map, int mapstep,
      uchar* dstImage, int dstStep, CvSize size, int interToggle ))

IPCVAPI_EX( CvStatus, icvUnDistort_8u_C3R,
    "ippiUnDistort_8u_C3R", CV_PLUGINS1(CV_PLUGIN_IPPCV),        
    ( const uchar* srcImage, int srcStep, const int* map, int mapstep,
      uchar* dstImage, int dstStep, CvSize size, int interToggle ))

/****************************************************************************************\
*                               Thresholding functions                                   *
\****************************************************************************************/

IPCVAPI_EX( CvStatus, icvThresh_8u_C1R, "ippiThresh_8u_C1R", CV_PLUGINS1(CV_PLUGIN_IPPCV),
           ( const uchar*  src, int  src_step, uchar*  dst, int  dst_step,
             CvSize  roi, int  thresh, uchar max_val, int type ))

IPCVAPI_EX( CvStatus, icvThresh_32f_C1R, "ippiThresh_32f_C1R", CV_PLUGINS1(CV_PLUGIN_IPPCV),
           ( const float*  src, int  src_step, float*  dst, int  dst_step,
             CvSize  roi, float  thresh, float max_val, int type))

/****************************************************************************************\
*                                 Canny Edge Detector                                    *
\****************************************************************************************/

IPCVAPI_EX( CvStatus, icvCannyGetSize, "ippiCannyGetSize", 0/*CV_PLUGINS1(CV_PLUGIN_IPPCV)*/,
           ( CvSize roiSize, int* bufferSize ))

IPCVAPI_EX( CvStatus, icvCanny_16s8u_C1R, "ippiCanny_16s8u_C1R", 0/*CV_PLUGINS1(CV_PLUGIN_IPPCV)*/,
    ( const short* pSrcDx, int srcDxStep, const short* pSrcDy, int srcDyStep,
      uchar*  pDstEdges, int dstEdgeStep, CvSize roiSize, float lowThresh,
      float  highThresh, void* pBuffer ))

#endif /*_CV_IPP_H_*/

