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

/****************************************************************************************\
*                                      Copy/Set                                          *
\****************************************************************************************/

IPPAPI( IppStatus, ippiCopy_8u_C1R, ( const uchar* src, int src_step,
                             uchar* dst, int dst_step, IppiSize size ))

IPPAPI( IppStatus, ippiSet_8u_C1R, ( uchar value, uchar* dst,
                                     int dst_step, IppiSize size ))

#define IPCV_COPYSET( flavor, arrtype, scalartype )                                 \
IPPAPI( IppStatus, ippiCopy##flavor, ( const arrtype* src, int srcstep,             \
                                       arrtype* dst, int dststep, IppiSize size,    \
                                       const uchar* mask, int maskstep ))           \
IPPAPI( IppStatus, ippiSet##flavor, ( arrtype* dst, int dststep,                    \
                                      const uchar* mask, int maskstep,              \
                                      IppiSize size, const arrtype* scalar ))

IPCV_COPYSET( _8u_C1MR, uchar, int )
IPCV_COPYSET( _16s_C1MR, ushort, int )
IPCV_COPYSET( _8u_C3MR, uchar, int )
IPCV_COPYSET( _8u_C4MR, int, int )
IPCV_COPYSET( _16s_C3MR, ushort, int )
IPCV_COPYSET( _16s_C4MR, int64, int64 )
IPCV_COPYSET( _32f_C3MR, int, int )
IPCV_COPYSET( _32f_C4MR, int, int )

#undef IPCV_COPYSET

/****************************************************************************************\
*                                       Arithmetics                                      *
\****************************************************************************************/

#define IPCV_BIN_ARITHM_INT( name, flavor, arrtype )                    \
IPPAPI( IppStatus, ippi##name##_##flavor##_C1RSfs,                      \
( const arrtype* src1, int srcstep1, const arrtype* src2, int srcstep2, \
  arrtype* dst, int dststep, IppiSize size, int scalefactor ))

IPCV_BIN_ARITHM_INT( Add, 8u, uchar )
IPCV_BIN_ARITHM_INT( Add, 16s, short )
IPCV_BIN_ARITHM_INT( Sub, 8u, uchar )
IPCV_BIN_ARITHM_INT( Sub, 16s, short )

IPCV_BIN_ARITHM_INT( Add, 16u, ushort )
IPCV_BIN_ARITHM_INT( Sub, 16u, ushort )

#define IPCV_BIN_ARITHM_FLT( name, flavor, arrtype )                    \
IPPAPI( IppStatus, ippi##name##_##flavor##_C1R,                         \
( const arrtype* src1, int srcstep1, const arrtype* src2, int srcstep2, \
  arrtype* dst, int dststep, IppiSize size ))

IPCV_BIN_ARITHM_FLT( Add, 32f, float )
IPCV_BIN_ARITHM_FLT( Sub, 32f, float )

#undef IPCV_BIN_ARITHM_INT
#undef IPCV_BIN_ARITHM_FLT

/****************************************************************************************\
*                                     Logical operations                                 *
\****************************************************************************************/

#define IPCV_LOGIC( name )                                          \
IPPAPI( IppStatus, ippi##name##_8u_C1R,                             \
( const uchar* src1, int srcstep1, const uchar* src2, int srcstep2, \
  uchar* dst, int dststep, IppiSize size ))

IPCV_LOGIC( And )
IPCV_LOGIC( Or )
IPCV_LOGIC( Xor )

#undef IPCV_LOGIC

IPPAPI( IppStatus, ippiNot_8u_C1R,
( const uchar* src, int step1, uchar* dst, int step, IppiSize size ))

/****************************************************************************************\
*                                Image Statistics                                        *
\****************************************************************************************/

///////////////////////////////////////// Mean //////////////////////////////////////////

#define IPCV_DEF_MEAN_MASK( flavor, srctype )           \
IPPAPI( IppStatus, ippiMean_##flavor##_C1MR,            \
( const srctype* img, int imgstep, const uchar* mask,   \
  int maskStep, IppiSize size, double* mean ))

IPCV_DEF_MEAN_MASK( 8u, uchar )
IPCV_DEF_MEAN_MASK( 16u, ushort )
IPCV_DEF_MEAN_MASK( 32f, float )

#undef IPCV_DEF_MEAN_MASK

//////////////////////////////////// Mean_StdDev ////////////////////////////////////////

#define IPCV_DEF_MEAN_SDV( flavor, srctype )                                \
IPPAPI( IppStatus, ippiMean_StdDev_##flavor##_C1R,                          \
( const srctype* img, int imgstep, IppiSize size,                           \
  double* mean, double* sdv ))                                              \
IPPAPI( IppStatus, ippiMean_StdDev_##flavor##_C1MR,                         \
( const srctype* img, int imgstep,                                          \
  const uchar* mask, int maskStep,                                          \
  IppiSize size, double* mean, double* sdv ))

IPCV_DEF_MEAN_SDV( 8u, uchar )
IPCV_DEF_MEAN_SDV( 16u, ushort )
IPCV_DEF_MEAN_SDV( 32f, float )

#undef IPCV_DEF_MEAN_SDV

//////////////////////////////////// MinMaxIndx /////////////////////////////////////////

#define IPCV_DEF_MIN_MAX_LOC( flavor, srctype, extrtype )       \
IPPAPI( IppStatus, ippiMinMaxIndx_##flavor##_C1R,                \
( const srctype* img, int imgstep,                              \
  IppiSize size, extrtype* minVal, extrtype* maxVal,            \
  IppiPoint* minLoc, IppiPoint* maxLoc ))                       \
                                                                \
IPPAPI( IppStatus, ippiMinMaxIndx_##flavor##_C1MR,               \
( const srctype* img, int imgstep,                              \
  const uchar* mask, int maskStep,                              \
  IppiSize size, extrtype* minVal, extrtype* maxVal,            \
  IppiPoint* minLoc, IppiPoint* maxLoc ))

IPCV_DEF_MIN_MAX_LOC( 8u, uchar, float )
IPCV_DEF_MIN_MAX_LOC( 16u, ushort, float )
IPCV_DEF_MIN_MAX_LOC( 32f, float, float )

#undef IPCV_MIN_MAX_LOC

////////////////////////////////////////// Sum //////////////////////////////////////////

#define IPCV_DEF_SUM_NOHINT( flavor, srctype )                              \
IPPAPI( IppStatus, ippiSum_##flavor##_C1R,( const srctype* img, int imgstep,\
                                           IppiSize size, double* sum ))    \
IPPAPI( IppStatus, ippiSum_##flavor##_C3R,( const srctype* img, int imgstep,\
                                           IppiSize size, double* sum ))    \
IPPAPI( IppStatus, ippiSum_##flavor##_C4R,( const srctype* img, int imgstep,\
                                           IppiSize size, double* sum ))    \

#define IPCV_DEF_SUM_HINT( flavor, srctype )                                \
IPPAPI( IppStatus, ippiSum_##flavor##_C1R,                                  \
                        ( const srctype* img, int imgstep,                  \
                          IppiSize size, double* sum, IppHintAlgorithm hint ))   \
IPPAPI( IppStatus, ippiSum_##flavor##_C3R,                                  \
                        ( const srctype* img, int imgstep,                  \
                          IppiSize size, double* sum, IppHintAlgorithm hint ))   \
IPPAPI( IppStatus, ippiSum_##flavor##_C4R,                                  \
                        ( const srctype* img, int imgstep,                  \
                          IppiSize size, double* sum, IppHintAlgorithm hint ))

IPCV_DEF_SUM_NOHINT( 8u, uchar )
IPCV_DEF_SUM_NOHINT( 16s, short )
IPCV_DEF_SUM_HINT( 32f, float )

#undef IPCV_DEF_SUM_NOHINT
#undef IPCV_DEF_SUM_HINT

////////////////////////////////////////// Norms /////////////////////////////////

#define IPCV_DEF_NORM_NOHINT_C1( flavor, srctype )                                      \
IPPAPI( IppStatus, ippiNorm_Inf_##flavor##_C1R, ( const srctype* img, int imgstep,      \
                                                  IppiSize size, double* norm ))        \
IPPAPI( IppStatus, ippiNorm_L1_##flavor##_C1R,                                          \
                                             ( const srctype* img, int imgstep,         \
                                               IppiSize size, double* norm ))           \
IPPAPI( IppStatus, ippiNormDiff_Inf_##flavor##_C1R,                                     \
                                             ( const srctype* img1, int imgstep1,       \
                                               const srctype* img2, int imgstep2,       \
                                               IppiSize size, double* norm ))           \
IPPAPI( IppStatus, ippiNormDiff_L1_##flavor##_C1R,                                      \
                                             ( const srctype* img1, int imgstep1,       \
                                               const srctype* img2, int imgstep2,       \
                                               IppiSize size, double* norm ))           \
IPPAPI( IppStatus, ippiNormDiff_L2_##flavor##_C1R,                                      \
                                             ( const srctype* img1, int imgstep1,       \
                                               const srctype* img2, int imgstep2,       \
                                               IppiSize size, double* norm ))

#define IPCV_DEF_NORM_HINT_C1( flavor, srctype )                                        \
IPPAPI( IppStatus, ippiNorm_Inf_##flavor##_C1R, ( const srctype* img, int imgstep,      \
                                                  IppiSize size, double* norm ))        \
IPPAPI( IppStatus, ippiNorm_L1_##flavor##_C1R, ( const srctype* img, int imgstep,       \
                                IppiSize size, double* norm, IppHintAlgorithm hint ))        \
IPPAPI( IppStatus, ippiNorm_L2_##flavor##_C1R, ( const srctype* img, int imgstep,       \
                                IppiSize size, double* norm, IppHintAlgorithm hint ))        \
IPPAPI( IppStatus, ippiNormDiff_Inf_##flavor##_C1R, ( const srctype* img1, int imgstep1,\
                                                      const srctype* img2, int imgstep2,\
                                                      IppiSize size, double* norm ))    \
IPPAPI( IppStatus, ippiNormDiff_L1_##flavor##_C1R, ( const srctype* img1, int imgstep1, \
                                                     const srctype* img2, int imgstep2, \
                                IppiSize size, double* norm, IppHintAlgorithm hint ))         \
IPPAPI( IppStatus, ippiNormDiff_L2_##flavor##_C1R, ( const srctype* img1, int imgstep1, \
                                                     const srctype* img2, int imgstep2, \
                                IppiSize size, double* norm, IppHintAlgorithm hint ))

#define IPCV_DEF_NORM_MASK_C1( flavor, srctype )                                        \
IPPAPI( IppStatus, ippiNorm_Inf_##flavor##_C1MR, ( const srctype* img, int imgstep,     \
                                                   const uchar* mask, int maskstep,     \
                                                   IppiSize size, double* norm ))       \
IPPAPI( IppStatus, ippiNorm_L1_##flavor##_C1MR, ( const srctype* img, int imgstep,      \
                                                  const uchar* mask, int maskstep,      \
                                                  IppiSize size, double* norm ))        \
IPPAPI( IppStatus, ippiNorm_L2_##flavor##_C1MR, ( const srctype* img, int imgstep,      \
                                                  const uchar* mask, int maskstep,      \
                                                  IppiSize size, double* norm ))        \
IPPAPI( IppStatus, ippiNormDiff_Inf_##flavor##_C1MR,( const srctype* img1, int imgstep1,\
                                                      const srctype* img2, int imgstep2,\
                                                      const uchar* mask, int maskstep,  \
                                                      IppiSize size, double* norm ))    \
IPPAPI( IppStatus, ippiNormDiff_L1_##flavor##_C1MR,( const srctype* img1, int imgstep1, \
                                                     const srctype* img2, int imgstep2, \
                                                     const uchar* mask, int maskstep,   \
                                                     IppiSize size, double* norm ))     \
IPPAPI( IppStatus, ippiNormDiff_L2_##flavor##_C1MR,( const srctype* img1, int imgstep1, \
                                                     const srctype* img2, int imgstep2, \
                                                     const uchar* mask, int maskstep,   \
                                                     IppiSize size, double* norm ))

IPCV_DEF_NORM_NOHINT_C1( 8u, uchar )
IPCV_DEF_NORM_MASK_C1( 8u, uchar )

IPCV_DEF_NORM_MASK_C1( 16u, ushort )

/*IPCV_DEF_NORM_NOHINT_C1( 16s, short ) // these functions contain are buggy in IPP 4.1 */

IPCV_DEF_NORM_HINT_C1( 32f, float )
IPCV_DEF_NORM_MASK_C1( 32f, float )

#undef IPCV_DEF_NORM_HONINT_C1
#undef IPCV_DEF_NORM_HINT_C1
#undef IPCV_DEF_NORM_MASK_C1

////////////////////////////////////// AbsDiff ///////////////////////////////////////////

#define IPCV_ABS_DIFF( flavor, arrtype )                    \
IPPAPI( IppStatus, ippiAbsDiff_##flavor##_C1R,              \
( const arrtype* src1, int srcstep1,                        \
  const arrtype* src2, int srcstep2,                        \
  arrtype* dst, int dststep, IppiSize size ))

IPCV_ABS_DIFF( 8u, uchar )
IPCV_ABS_DIFF( 16u, ushort )
IPCV_ABS_DIFF( 32f, float )

#undef IPCV_ABS_DIFF

////////////////////////////// Comparisons //////////////////////////////////////////

#define IPCV_CMP( arrtype, flavor )                                                 \
IPPAPI( IppStatus, ippiCompare_##flavor##_C1R,                                      \
            ( const arrtype* src1, int srcstep1, const arrtype* src2, int srcstep2, \
              arrtype* dst, int dststep, IppiSize size, int cmp_op ))               \
IPPAPI( IppStatus, ippiCompareC_##flavor##_C1R,                                     \
            ( const arrtype* src1, int srcstep1, arrtype scalar,                    \
              arrtype* dst, int dststep, IppiSize size, int cmp_op ))               \
IPPAPI( IppStatus, ippiThreshold_GT_##flavor##_C1R,                                 \
            ( const arrtype* pSrc, int srcstep, arrtype* pDst, int dststep,         \
              IppiSize size, arrtype threshold ))                                   \
IPPAPI( IppStatus, ippiThreshold_LT_##flavor##_C1R,                                 \
            ( const arrtype* pSrc, int srcstep, arrtype* pDst, int dststep,         \
              IppiSize size, arrtype threshold ))
IPCV_CMP( uchar, 8u )
IPCV_CMP( short, 16s )
IPCV_CMP( float, 32f )
#undef IPCV_CMP

/****************************************************************************************\
*                                       Utilities                                        *
\****************************************************************************************/

////////////////////////////// Copy Pixel <-> Plane /////////////////////////////////

#define IPCV_PIX_PLANE( flavor, arrtype )                                           \
IPPAPI( IppStatus, ippiCopy_##flavor##_C3P3R,                                        \
    ( const arrtype* src, int srcstep, arrtype** dst, int dststep, IppiSize size )) \
IPPAPI( IppStatus, ippiCopy_##flavor##_C4P4R,                                        \
    ( const arrtype* src, int srcstep, arrtype** dst, int dststep, IppiSize size )) \
IPPAPI( IppStatus, ippiCopy_##flavor##_P3C3R,                                        \
    ( const arrtype** src, int srcstep, arrtype* dst, int dststep, IppiSize size )) \
IPPAPI( IppStatus, ippiCopy_##flavor##_P4C4R,                                        \
    ( const arrtype** src, int srcstep, arrtype* dst, int dststep, IppiSize size ))

IPCV_PIX_PLANE( 8u, uchar )
IPCV_PIX_PLANE( 16s, ushort )
IPCV_PIX_PLANE( 32f, int )

#undef IPCV_PIX_PLANE

/****************************************************************************************/
/*                            Math routines and RNGs                                    */
/****************************************************************************************/

IPPAPI( IppStatus, ippsInvSqrt_32f_A21, ( const float* src, float* dst, int len ))
IPPAPI( IppStatus, ippsSqrt_32f_A21, ( const float* src, float* dst, int len ))
IPPAPI( IppStatus, ippsInvSqrt_64f_A50, ( const double* src, double* dst, int len ))
IPPAPI( IppStatus, ippsSqrt_64f_A50, ( const double* src, double* dst, int len ))

IPPAPI( IppStatus, ippsLn_32f_A21, ( const float *x, float *y, int n ) )
IPPAPI( IppStatus, ippsLn_64f_A50, ( const double *x, double *y, int n ) )
IPPAPI( IppStatus, ippsExp_32f_A21, ( const float *x, float *y, int n ) )
IPPAPI( IppStatus, ippsExp_64f_A50, ( const double *x, double *y, int n ) )

IPPAPI( IppStatus, ippibFastArctan_32f, ( const float* y, const float* x, float* angle, int len ))

/****************************************************************************************/
/*                    Affine transformations of matrix/image elements                   */
/****************************************************************************************/

#define IPCV_TRANSFORM( suffix, cn )                            \
IPPAPI( IppStatus, ippiColorTwist##suffix##_C##cn##R,           \
        ( const void* src, int srcstep, void* dst, int dststep, \
          IppiSize roisize, const float* twist_matrix ))

IPCV_TRANSFORM( 32f_8u, 3 )
IPCV_TRANSFORM( 32f_16u, 3 )
IPCV_TRANSFORM( 32f_16s, 3 )
IPCV_TRANSFORM( _32f, 3 )
IPCV_TRANSFORM( _32f, 4 )

#undef IPCV_TRANSFORM

#define IPCV_TRANSFORM_N1( suffix )                             \
IPPAPI( IppStatus, ippiColorToGray##suffix,                     \
        ( const void* src, int srcstep, void* dst, int dststep, \
          IppiSize roisize, const float* coeffs ))

IPCV_TRANSFORM_N1( _8u_C3C1R )
IPCV_TRANSFORM_N1( _16u_C3C1R )
IPCV_TRANSFORM_N1( _16s_C3C1R )
IPCV_TRANSFORM_N1( _32f_C3C1R )
IPCV_TRANSFORM_N1( _8u_AC4C1R )
IPCV_TRANSFORM_N1( _16u_AC4C1R )
IPCV_TRANSFORM_N1( _16s_AC4C1R )
IPCV_TRANSFORM_N1( _32f_AC4C1R )

#undef IPCV_TRANSFORM_N1

/****************************************************************************************/
/*                  Matrix routines from BLAS/LAPACK compatible libraries               */
/****************************************************************************************/

#define IPCV_DFT( init_flavor, fwd_flavor, inv_flavor )                                 \
IPPAPI( IppStatus, ippsDFTInitAlloc_##init_flavor, ( void** state, int len, int flag,   \
                                                     IppHintAlgorithm hint ))           \
IPPAPI( IppStatus, ippsDFTFree_##init_flavor, ( void* state ))                          \
IPPAPI( IppStatus, ippsDFTGetBufSize_##init_flavor, ( const void* spec, int* buf_size ))\
IPPAPI( IppStatus, ippsDFTFwd_##fwd_flavor, ( const void* src, void* dst,               \
                                              const void* spec, void* buffer ))         \
IPPAPI( IppStatus, ippsDFTInv_##inv_flavor, ( const void* src, void* dst,               \
                                              const void* spec, void* buffer ))

IPCV_DFT( C_32fc, CToC_32fc, CToC_32fc )
IPCV_DFT( R_32f, RToPack_32f, PackToR_32f )
IPCV_DFT( C_64fc, CToC_64fc, CToC_64fc )
IPCV_DFT( R_64f, RToPack_64f, PackToR_64f )
#undef IPCV_DFT

/****************************************************************************************\
*                                  Creating Borders                                      *
\****************************************************************************************/

#define IPCV_COPY_BORDER( bordertype, flavor, arrtype )                         \
IPPAPI( IppStatus, ippiCopy##bordertype##Border_##flavor,                       \
            ( const arrtype* pSrc,  int srcStep, IppiSize srcRoiSize,           \
                    arrtype* pDst,  int dstStep, IppiSize dstRoiSize,           \
                    int topBorderWidth, int leftBorderWidth ))

IPCV_COPY_BORDER( Replicate, 8u_C1R, uchar )
IPCV_COPY_BORDER( Replicate, 16s_C1R, ushort )
IPCV_COPY_BORDER( Replicate, 8u_C3R, uchar )
IPCV_COPY_BORDER( Replicate, 32s_C1R, int )
IPCV_COPY_BORDER( Replicate, 16s_C3R, ushort )
IPCV_COPY_BORDER( Replicate, 16s_C4R, int )
IPCV_COPY_BORDER( Replicate, 32s_C3R, int )
IPCV_COPY_BORDER( Replicate, 32s_C4R, int )

#undef IPCV_COPY_BORDER

#define IPCV_COPY_CONST_BORDER_C1( flavor, arrtype )                            \
IPPAPI( IppStatus, ippiCopyConstBorder_##flavor,                                \
            ( const arrtype* pSrc,  int srcStep, IppiSize srcRoiSize,           \
                    arrtype* pDst,  int dstStep, IppiSize dstRoiSize,           \
                    int topBorderWidth, int leftBorderWidth, arrtype value ))

IPCV_COPY_CONST_BORDER_C1( 8u_C1R, uchar )
IPCV_COPY_CONST_BORDER_C1( 16s_C1R, ushort )
IPCV_COPY_CONST_BORDER_C1( 32s_C1R, int )

#undef IPCV_COPY_CONST_BORDER_C1

#define IPCV_COPY_CONST_BORDER_CN( flavor, arrtype )                            \
IPPAPI( IppStatus, ippiCopyConstBorder_##flavor,                                \
            ( const arrtype* pSrc,  int srcStep, IppiSize srcRoiSize,           \
                    arrtype* pDst,  int dstStep, IppiSize dstRoiSize,           \
                    int topBorderWidth, int leftBorderWidth, const arrtype* value ))

IPCV_COPY_CONST_BORDER_CN( 8u_C3R, uchar )
IPCV_COPY_CONST_BORDER_CN( 16s_C3R, ushort )
IPCV_COPY_CONST_BORDER_CN( 16s_C4R, int )
IPCV_COPY_CONST_BORDER_CN( 32s_C3R, int )
IPCV_COPY_CONST_BORDER_CN( 32s_C4R, int )

#undef IPCV_COPY_CONST_BORDER_CN

/****************************************************************************************\
*                                        Moments                                         *
\****************************************************************************************/

#define IPCV_MOMENTS( suffix, cn )                      \
IPPAPI( IppStatus, ippiMoments##suffix##_C##cn##R,      \
( const void* img, int step, IppiSize size, void* momentstate ))

IPCV_MOMENTS( 64f_8u, 1 )
IPCV_MOMENTS( 64f_32f, 1 )

#undef IPCV_MOMENTS

IPPAPI( IppStatus, ippiMomentInitAlloc_64f, (void** momentstate, IppHintAlgorithm hint ))
IPPAPI( IppStatus, ippiMomentFree_64f, (void* momentstate ))
IPPAPI( IppStatus, ippiGetSpatialMoment_64f, (const void* momentstate, int mOrd, int nOrd,
                                              int nChannel, IppiPoint roiOffset, double* value ))

/****************************************************************************************\
*                                  Background differencing                               *
\****************************************************************************************/

/////////////////////////////////// Accumulation /////////////////////////////////////////

#define IPCV_ACCUM( flavor, arrtype, acctype )                                      \
IPPAPI( IppStatus, ippiAddSquare_##flavor##_C1IR,                                   \
    ( const arrtype* src, int srcstep, acctype* dst, int dststep, IppiSize size ))  \
IPPAPI( IppStatus, ippiAddProduct_##flavor##_C1IR,                                  \
    ( const arrtype* src1, int srcstep1, const arrtype* src2, int srcstep2,         \
      acctype* dst, int dststep, IppiSize size ))                                   \
IPPAPI( IppStatus, ippiAddWeighted_##flavor##_C1IR,                                 \
    ( const arrtype* src, int srcstep, acctype* dst, int dststep,                   \
      IppiSize size, acctype alpha ))                                               \
                                                                                    \
IPPAPI( IppStatus, ippiAdd_##flavor##_C1IMR,                                        \
    ( const arrtype* src, int srcstep, const uchar* mask, int maskstep,             \
      acctype* dst, int dststep, IppiSize size ))                                   \
IPPAPI( IppStatus, ippiAddSquare_##flavor##_C1IMR,                                  \
    ( const arrtype* src, int srcstep, const uchar* mask, int maskstep,             \
      acctype* dst, int dststep, IppiSize size ))                                   \
IPPAPI( IppStatus, ippiAddProduct_##flavor##_C1IMR,                                 \
    ( const arrtype* src1, int srcstep1, const arrtype* src2, int srcstep2,         \
      const uchar* mask, int maskstep, acctype* dst, int dststep, IppiSize size ))  \
IPPAPI( IppStatus, ippiAddWeighted_##flavor##_C1IMR,                                \
    ( const arrtype* src, int srcstep, const uchar* mask, int maskstep,             \
      acctype* dst, int dststep, IppiSize size, acctype alpha ))

IPPAPI( IppStatus, ippiAdd_8u32f_C1IR,
    ( const uchar* src, int srcstep, float* dst, int dststep, IppiSize size ))

IPCV_ACCUM( 8u32f, uchar, float )
IPCV_ACCUM( 32f, float, float )

#undef IPCV_ACCUM

/****************************************************************************************\
*                                       Pyramids                                         *
\****************************************************************************************/

IPPAPI( IppStatus, ippiPyrDownGetBufSize_Gauss5x5,
            ( int roiWidth, IppDataType dataType, int channels, int* bufSize ))

IPPAPI( IppStatus, ippiPyrUpGetBufSize_Gauss5x5,
            ( int roiWidth, IppDataType dataType, int channels, int* bufSize ))

#define ICV_PYRDOWN( flavor, cn )                                           \
IPPAPI( IppStatus, ippiPyrDown_Gauss5x5_##flavor##_C##cn##R,                \
( const void* pSrc, int srcStep, void* pDst, int dstStep,                   \
  IppiSize roiSize, void* pBuffer ))

#define ICV_PYRUP( flavor, cn )                                             \
IPPAPI( IppStatus, ippiPyrUp_Gauss5x5_##flavor##_C##cn##R,                  \
( const void* pSrc, int srcStep, void* pDst, int dstStep,                   \
  IppiSize roiSize, void* pBuffer ))

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

#define IPCV_RESIZE( flavor, cn )                                           \
IPPAPI( IppStatus, ippiResize_##flavor##_C##cn##R,                          \
           (const void* src, IppiSize srcsize, int srcstep, IppiRect srcroi,  \
            void* dst, int dststep, IppiSize dstroi,                        \
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
IPPAPI( IppStatus, ippiWarpAffineBack_##flavor##_C##cn##R,                  \
    (const void* src, IppiSize srcsize, int srcstep, IppiRect srcroi,       \
    void* dst, int dststep, IppiRect dstroi,                                \
    const double* coeffs, int interpolate ))

IPCV_WARPAFFINE_BACK( 8u, 1 )
IPCV_WARPAFFINE_BACK( 8u, 3 )
IPCV_WARPAFFINE_BACK( 8u, 4 )

IPCV_WARPAFFINE_BACK( 32f, 1 )
IPCV_WARPAFFINE_BACK( 32f, 3 )
IPCV_WARPAFFINE_BACK( 32f, 4 )

#undef IPCV_WARPAFFINE_BACK

#define IPCV_WARPPERSPECTIVE_BACK( flavor, cn )                             \
IPPAPI( IppStatus, ippiWarpPerspectiveBack_##flavor##_C##cn##R,             \
    (const void* src, IppiSize srcsize, int srcstep, IppiRect srcroi,       \
    void* dst, int dststep, IppiRect dstroi,                                \
    const double* coeffs, int interpolate ))

IPCV_WARPPERSPECTIVE_BACK( 8u, 1 )
IPCV_WARPPERSPECTIVE_BACK( 8u, 3 )
IPCV_WARPPERSPECTIVE_BACK( 8u, 4 )

IPCV_WARPPERSPECTIVE_BACK( 32f, 1 )
IPCV_WARPPERSPECTIVE_BACK( 32f, 3 )
IPCV_WARPPERSPECTIVE_BACK( 32f, 4 )

#undef IPCV_WARPPERSPECTIVE_BACK

#define IPCV_WARPPERSPECTIVE( flavor, cn )                              \
IPPAPI( IppStatus, ippiWarpPerspective_##flavor##_C##cn##R,             \
    (const void* src, IppiSize srcsize, int srcstep, IppiRect srcroi,   \
    void* dst, int dststep, IppiRect dstroi,                            \
    const double* coeffs, int interpolate ))

IPCV_WARPPERSPECTIVE( 8u, 1 )
IPCV_WARPPERSPECTIVE( 8u, 3 )
IPCV_WARPPERSPECTIVE( 8u, 4 )

IPCV_WARPPERSPECTIVE( 32f, 1 )
IPCV_WARPPERSPECTIVE( 32f, 3 )
IPCV_WARPPERSPECTIVE( 32f, 4 )

#undef IPCV_WARPPERSPECTIVE

#define IPCV_REMAP( flavor, cn )                                        \
IPPAPI( IppStatus, ippiRemap_##flavor##_C##cn##R,                       \
    ( const void* src, IppiSize srcsize, int srcstep, IppiRect srcroi,  \
      const float* xmap, int xmapstep, const float* ymap, int ymapstep, \
      void* dst, int dststep, IppiSize dstsize, int interpolation )) 

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

#define IPCV_MORPHOLOGY( minmaxtype, morphtype, flavor, cn )                    \
IPPAPI( IppStatus, ippiFilter##minmaxtype##BorderReplicate_##flavor##_C##cn##R, \
    ( const void* src, int srcstep, void* dst, int dststep, IppiSize roi,       \
    IppiSize esize, IppiPoint anchor, void* buffer ))                           \
IPPAPI( IppStatus, ippiFilter##minmaxtype##GetBufferSize_##flavor##_C##cn##R,   \
    ( int width, IppiSize esize, int* bufsize ))                                \
IPPAPI( IppStatus, ippi##morphtype##BorderReplicate_##flavor##_C##cn##R,        \
    ( const void* src, int srcstep, void* dst, int dststep,                     \
    IppiSize roi, int bordertype, void* morphstate ))

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

#define IPCV_MORPHOLOGY_INIT_ALLOC( flavor, cn )                        \
IPPAPI( IppStatus, ippiMorphologyInitAlloc_##flavor##_C##cn##R,         \
    ( int width, const uchar* element, IppiSize esize,                  \
    IppiPoint anchor, void** morphstate ))

IPCV_MORPHOLOGY_INIT_ALLOC( 8u, 1 )
IPCV_MORPHOLOGY_INIT_ALLOC( 8u, 3 )
IPCV_MORPHOLOGY_INIT_ALLOC( 8u, 4 )
IPCV_MORPHOLOGY_INIT_ALLOC( 32f, 1 )
IPCV_MORPHOLOGY_INIT_ALLOC( 32f, 3 )
IPCV_MORPHOLOGY_INIT_ALLOC( 32f, 4 )

#undef IPCV_MORPHOLOGY_INIT_ALLOC

IPPAPI( IppStatus, ippiMorphologyFree, ( void* morphstate ))

/****************************************************************************************\
*                                 Smoothing Filters                                      *
\****************************************************************************************/

#define IPCV_FILTER_MEDIAN( flavor, cn )                            \
IPPAPI( IppStatus, ippiFilterMedian_##flavor##_C##cn##R,            \
            ( const void* src, int srcstep, void* dst, int dststep, \
              IppiSize roi, IppiSize ksize, IppiPoint anchor ))

IPCV_FILTER_MEDIAN( 8u, 1 )
IPCV_FILTER_MEDIAN( 8u, 3 )
IPCV_FILTER_MEDIAN( 8u, 4 )

#define IPCV_FILTER_BOX( flavor, cn )                               \
IPPAPI( IppStatus, ippiFilterBox_##flavor##_C##cn##R,               \
            ( const void* src, int srcstep, void* dst, int dststep, \
              IppiSize roi, IppiSize ksize, IppiPoint anchor ))

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

#define IPCV_FILTER_SOBEL_BORDER( suffix, flavor, srctype )             \
IPPAPI( IppStatus, ippiFilterSobel##suffix##GetBufferSize_##flavor##_C1R,\
    ( IppiSize roi, int masksize, int* buffersize ))                    \
IPPAPI( IppStatus, ippiFilterSobel##suffix##Border_##flavor##_C1R,      \
    ( const void* src, int srcstep, void* dst, int dststep,             \
      IppiSize roi, int masksize, int bordertype,                       \
      srctype bordervalue, void* buffer ))

IPCV_FILTER_SOBEL_BORDER( NegVert, 8u16s, uchar )
IPCV_FILTER_SOBEL_BORDER( Horiz, 8u16s, uchar )
IPCV_FILTER_SOBEL_BORDER( VertSecond, 8u16s, uchar )
IPCV_FILTER_SOBEL_BORDER( HorizSecond, 8u16s, uchar )
IPCV_FILTER_SOBEL_BORDER( Cross, 8u16s, uchar )

IPCV_FILTER_SOBEL_BORDER( NegVert, 32f, float )
IPCV_FILTER_SOBEL_BORDER( Horiz, 32f, float )
IPCV_FILTER_SOBEL_BORDER( VertSecond, 32f, float )
IPCV_FILTER_SOBEL_BORDER( HorizSecond, 32f, float )
IPCV_FILTER_SOBEL_BORDER( Cross, 32f, float )

#undef IPCV_FILTER_SOBEL_BORDER

#define IPCV_FILTER_SCHARR_BORDER( suffix, flavor, srctype )             \
IPPAPI( IppStatus, ippiFilterScharr##suffix##GetBufferSize_##flavor##_C1R,\
    ( IppiSize roi, int* buffersize ))                                   \
IPPAPI( IppStatus, ippiFilterScharr##suffix##Border_##flavor##_C1R,      \
    ( const void* src, int srcstep, void* dst, int dststep, IppiSize roi,\
      int bordertype, srctype bordervalue, void* buffer ))

IPCV_FILTER_SCHARR_BORDER( Vert, 8u16s, uchar )
IPCV_FILTER_SCHARR_BORDER( Horiz, 8u16s, uchar )

IPCV_FILTER_SCHARR_BORDER( Vert, 32f, float )
IPCV_FILTER_SCHARR_BORDER( Horiz, 32f, float )

#undef IPCV_FILTER_SCHARR_BORDER


#define IPCV_FILTER_LAPLACIAN_BORDER( flavor, srctype )         \
IPPAPI( IppStatus, ippiFilterLaplacianGetBufferSize_##flavor##_C1R,\
    ( IppiSize roi, int masksize, int* buffersize ))            \
IPPAPI( IppStatus, ippiFilterLaplacianBorder_##flavor##_C1R,    \
    ( const void* src, int srcstep, void* dst, int dststep,     \
      IppiSize roi, int masksize, int bordertype,               \
      srctype bordervalue, void* buffer ))

IPCV_FILTER_LAPLACIAN_BORDER( 8u16s, uchar )
IPCV_FILTER_LAPLACIAN_BORDER( 32f, float )

#undef IPCV_FILTER_LAPLACIAN_BORDER


#define IPCV_FILTER_SOBEL( suffix, flavor )                 \
IPPAPI( IppStatus, ippiFilterSobel##suffix##_##flavor##_C1R,\
    ( const void* src, int srcstep, void* dst, int dststep, \
      IppiSize roi, int aperture ))

IPCV_FILTER_SOBEL( Vert, 8u16s )
IPCV_FILTER_SOBEL( Horiz, 8u16s )
IPCV_FILTER_SOBEL( VertSecond, 8u16s )
IPCV_FILTER_SOBEL( HorizSecond, 8u16s )
IPCV_FILTER_SOBEL( Cross, 8u16s )

IPCV_FILTER_SOBEL( VertMask, 32f )
IPCV_FILTER_SOBEL( HorizMask, 32f )
IPCV_FILTER_SOBEL( VertSecond, 32f )
IPCV_FILTER_SOBEL( HorizSecond, 32f )
IPCV_FILTER_SOBEL( Cross, 32f )

#undef IPCV_FILTER_SOBEL

#define IPCV_FILTER_SCHARR( suffix, flavor )                    \
IPPAPI( IppStatus, ippiFilterScharr##suffix##_##flavor##_C1R,   \
    ( const void* src, int srcstep, void* dst, int dststep, IppiSize roi ))

IPCV_FILTER_SCHARR( Vert, 8u16s )
IPCV_FILTER_SCHARR( Horiz, 8u16s )
IPCV_FILTER_SCHARR( Vert, 32f )
IPCV_FILTER_SCHARR( Horiz, 32f )

#undef IPCV_FILTER_SCHARR

/****************************************************************************************\
*                                   Generic Filters                                      *
\****************************************************************************************/

#define IPCV_FILTER( suffix, cn, ksizetype, anchortype )                                \
IPPAPI( IppStatus, ippiFilter##suffix##_C##cn##R,                                       \
            ( const void* src, int srcstep, void* dst, int dststep, IppiSize size,      \
              const float* kernel, ksizetype ksize, anchortype anchor ))

IPCV_FILTER( 32f_8u, 1, IppiSize, IppiPoint )
IPCV_FILTER( 32f_8u, 3, IppiSize, IppiPoint )
IPCV_FILTER( 32f_8u, 4, IppiSize, IppiPoint )

IPCV_FILTER( 32f_16s, 1, IppiSize, IppiPoint )
IPCV_FILTER( 32f_16s, 3, IppiSize, IppiPoint )
IPCV_FILTER( 32f_16s, 4, IppiSize, IppiPoint )

IPCV_FILTER( _32f, 1, IppiSize, IppiPoint )
IPCV_FILTER( _32f, 3, IppiSize, IppiPoint )
IPCV_FILTER( _32f, 4, IppiSize, IppiPoint )

IPCV_FILTER( Column32f_8u, 1, int, int )
IPCV_FILTER( Column32f_8u, 3, int, int )
IPCV_FILTER( Column32f_8u, 4, int, int )

IPCV_FILTER( Column32f_16s, 1, int, int )
IPCV_FILTER( Column32f_16s, 3, int, int )
IPCV_FILTER( Column32f_16s, 4, int, int )

IPCV_FILTER( Column_32f, 1, int, int )
IPCV_FILTER( Column_32f, 3, int, int )
IPCV_FILTER( Column_32f, 4, int, int )

IPCV_FILTER( Row32f_8u, 1, int, int )
IPCV_FILTER( Row32f_8u, 3, int, int )
IPCV_FILTER( Row32f_8u, 4, int, int )

IPCV_FILTER( Row32f_16s, 1, int, int )
IPCV_FILTER( Row32f_16s, 3, int, int )
IPCV_FILTER( Row32f_16s, 4, int, int )

IPCV_FILTER( Row_32f, 1, int, int )
IPCV_FILTER( Row_32f, 3, int, int )
IPCV_FILTER( Row_32f, 4, int, int )

#undef IPCV_FILTER


/****************************************************************************************\
*                                  Color Transformations                                 *
\****************************************************************************************/

#define IPCV_COLOR( funcname, flavor )                          \
IPPAPI( IppStatus, ippi##funcname##_##flavor##_C3R,             \
        ( const void* src, int srcstep, void* dst, int dststep, IppiSize size ))

IPCV_COLOR( RGBToXYZ, 8u )
IPCV_COLOR( RGBToXYZ, 16u )
IPCV_COLOR( RGBToXYZ, 32f )
IPCV_COLOR( XYZToRGB, 8u )
IPCV_COLOR( XYZToRGB, 16u )
IPCV_COLOR( XYZToRGB, 32f )

IPCV_COLOR( RGBToHSV, 8u )
IPCV_COLOR( HSVToRGB, 8u )

IPCV_COLOR( RGBToHLS, 8u )
IPCV_COLOR( RGBToHLS, 32f )
IPCV_COLOR( HLSToRGB, 8u )
IPCV_COLOR( HLSToRGB, 32f )

IPCV_COLOR( BGRToLab, 8u )
IPCV_COLOR( LabToBGR, 8u )

IPCV_COLOR( RGBToLUV, 8u )
IPCV_COLOR( RGBToLUV, 32f )
IPCV_COLOR( LUVToRGB, 8u )
IPCV_COLOR( LUVToRGB, 32f )

/****************************************************************************************\
*                                  Motion Templates                                      *
\****************************************************************************************/

IPPAPI( IppStatus, ippiUpdateMotionHistory_8u32f_C1IR,
    ( const uchar* silIm, int silStep, float* mhiIm, int mhiStep,
      IppiSize size,float  timestamp, float  mhi_duration ))

/****************************************************************************************\
*                                 Template Matching                                      *
\****************************************************************************************/

#define ICV_MATCHTEMPLATE( flavor, arrtype )                        \
IPPAPI( IppStatus, ippiCrossCorrValid_Norm_##flavor##_C1R,          \
        ( const arrtype* pSrc, int srcStep, IppiSize srcRoiSize,    \
        const arrtype* pTpl, int tplStep, IppiSize tplRoiSize,      \
        float* pDst, int dstStep ))                                 \
IPPAPI( IppStatus, ippiCrossCorrValid_NormLevel_##flavor##_C1R,     \
        ( const arrtype* pSrc, int srcStep, IppiSize srcRoiSize,    \
        const arrtype* pTpl, int tplStep, IppiSize tplRoiSize,      \
        float* pDst, int dstStep ))                                 \
IPPAPI( IppStatus, ippiSqrDistanceValid_Norm_##flavor##_C1R,        \
        ( const arrtype* pSrc, int srcStep, IppiSize srcRoiSize,    \
        const arrtype* pTpl, int tplStep, IppiSize tplRoiSize,      \
        float* pDst, int dstStep ))

ICV_MATCHTEMPLATE( 8u32f, Ipp8u )
ICV_MATCHTEMPLATE( 32f, float )

#undef ICV_MATCHTEMPLATE

/****************************************************************************************/
/*                                Distance Transform                                    */
/****************************************************************************************/

IPPAPI(IppStatus, ippiDistanceTransform_3x3_8u32f_C1R,
    ( const uchar* pSrc, int srcStep, float* pDst,
      int dstStep, IppiSize roiSize, float* pMetrics ))

IPPAPI(IppStatus, ippiDistanceTransform_5x5_8u32f_C1R,
    ( const uchar* pSrc, int srcStep, float* pDst,
      int dstStep, IppiSize roiSize, float* pMetrics ))

IPPAPI( IppStatus, ippiGetDistanceTransformMask, ( int maskType, float* pMetrics ))


IPPAPI(IppStatus, ippiDistanceTransform_3x3_8u_C1IR,
    ( uchar* pSrc, int srcStep, IppiSize roiSize, int* pMetrics ))

IPPAPI(IppStatus, ippiDistanceTransform_3x3_8u_C1R,
    ( const uchar* pSrc, int srcStep, uchar* pDst,
      int dstStep, IppiSize roiSize, int* pMetrics ))


/****************************************************************************************\
*                                 Radial Distortion Removal                              *
\****************************************************************************************/

IPPAPI(IppStatus, ippiUndistortGetSize, (IppiSize roiSize, int *pBufsize))

IPPAPI(IppStatus, ippiCreateMapCameraUndistort_32f_C1R, (Ipp32f *pxMap, int xStep,
                  Ipp32f *pyMap, int yStep, IppiSize roiSize,
                  Ipp32f fx, Ipp32f fy, Ipp32f cx, Ipp32f cy,
                  Ipp32f k1, Ipp32f k2, Ipp32f p1, Ipp32f p2, Ipp8u *pBuffer))

IPPAPI(IppStatus, ippiUndistortRadial_8u_C1R, (const Ipp8u* pSrc, int srcStep,
                         Ipp8u* pDst, int dstStep, IppiSize roiSize, Ipp32f fx, Ipp32f fy,
                         Ipp32f cx, Ipp32f cy, Ipp32f k1, Ipp32f k2, Ipp8u *pBuffer))

IPPAPI(IppStatus, ippiUndistortRadial_8u_C3R, (const Ipp8u* pSrc, int srcStep,
                         Ipp8u* pDst, int dstStep, IppiSize roiSize, Ipp32f fx, Ipp32f fy,
                         Ipp32f cx, Ipp32f cy, Ipp32f k1, Ipp32f k2, Ipp8u *pBuffer))

/****************************************************************************************\
*                            Subpixel-accurate rectangle extraction                      *
\****************************************************************************************/

IPPAPI(IppStatus, ippiCopySubpix_8u_C1R,  (const Ipp8u* pSrc, int srcStep,
                  Ipp8u* pDst, int dstStep, IppiSize roiSize, Ipp32f dx, Ipp32f dy))

IPPAPI(IppStatus, ippiCopySubpix_8u32f_C1R,  (const Ipp8u* pSrc, int srcStep,
                  Ipp32f* pDst, int dstStep, IppiSize roiSize, Ipp32f dx, Ipp32f dy))

IPPAPI(IppStatus, ippiCopySubpix_32f_C1R,  (const Ipp32f* pSrc, int srcStep,
                  Ipp32f* pDst, int dstStep, IppiSize roiSize, Ipp32f dx, Ipp32f dy))

/****************************************************************************************\
*                                Lucas-Kanade Optical Flow                               *
\****************************************************************************************/

IPPAPI(IppStatus, ippiOpticalFlowPyrLKInitAlloc_8u_C1R, (void** ppState,
       IppiSize roiSize, int winSize, IppHintAlgorithm hint))

IPPAPI(IppStatus, ippiOpticalFlowPyrLKFree_8u_C1R, (void* pState))

IPPAPI(IppStatus, ippiOpticalFlowPyrLK_8u_C1R, (void *pPyr1, void *pPyr2,
                         const Ipp32f *pPrev, Ipp32f *pNext, Ipp8s *pStatus, Ipp32f *pError,
                         int numFeat, int winSize, int maxLev, int maxIter, Ipp32f threshold,
                         void *pState))

/****************************************************************************************\
*                                 Haar Object Detector                                   *
\****************************************************************************************/

IPPAPI(IppStatus, ippiIntegral_8u32s_C1R,  (const Ipp8u* pSrc, int srcStep,
                  Ipp32s* pDst, int dstStep, IppiSize roiSize, Ipp32s val))

IPPAPI(IppStatus, ippiSqrIntegral_8u32s64f_C1R,  (const Ipp8u* pSrc, int srcStep,
                  Ipp32s* pDst, int dstStep, Ipp64f* pSqr, int sqrStep, IppiSize roiSize,
                  Ipp32s val, Ipp64f valSqr))

IPPAPI(IppStatus, ippiRectStdDev_32f_C1R,  (const Ipp32f* pSrc, int srcStep,
                         const Ipp64f* pSqr, int sqrStep, Ipp32f* pDst, int dstStep,
                         IppiSize roiSize, IppiRect rect))

IPPAPI(IppStatus, ippiHaarClassifierInitAlloc_32f, (void **pState,
                 const IppiRect* pFeature, const Ipp32f* pWeight, const Ipp32f* pThreshold,
                 const Ipp32f* pVal1, const Ipp32f* pVal2, const int* pNum, int length))

IPPAPI(IppStatus, ippiHaarClassifierFree_32f,(void *pState))

IPPAPI(IppStatus, ippiApplyHaarClassifier_32f_C1R, (const Ipp32f* pSrc, int srcStep,
                         const Ipp32f* pNorm, int normStep, Ipp8u* pMask, int maskStep,
                         IppiSize roi, int *pPositive, Ipp32f threshold,
                         void *pState))
