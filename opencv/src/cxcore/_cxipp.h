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

#ifndef _CXCORE_IPP_H_
#define _CXCORE_IPP_H_

/****************************************************************************************\
*                                      Copy/Set                                          *
\****************************************************************************************/

IPCVAPI_EX( CvStatus, icvCopy_8u_C1R, "ippiCopy_8u_C1R", CV_PLUGINS1(CV_PLUGIN_IPPI),
                  ( const uchar* src, int src_step,
                    uchar* dst, int dst_step, CvSize size ))

IPCVAPI_EX( CvStatus, icvSetByte_8u_C1R, "ippiSet_8u_C1R", CV_PLUGINS1(CV_PLUGIN_IPPI),
                  ( uchar value, uchar* dst, int dst_step, CvSize size ))

#define IPCV_CVT_TO( flavor )                                                   \
IPCVAPI_EX( CvStatus, icvCvtTo_##flavor##_C1R, "ippiCvtTo_" #flavor "_C1R", 0,  \
    ( const void* src, int step1, void* dst, int step, CvSize size, int param ))

IPCV_CVT_TO( 8u )
IPCV_CVT_TO( 8s )
IPCV_CVT_TO( 16u )
IPCV_CVT_TO( 16s )
IPCV_CVT_TO( 32s )
IPCV_CVT_TO( 32f )
IPCV_CVT_TO( 64f )

#undef IPCV_CVT_TO

IPCVAPI_EX( CvStatus, icvCvt_32f64f, "ippsConvert_32f64f",
            CV_PLUGINS1(CV_PLUGIN_IPPS), ( const float* src, double* dst, int len ))
IPCVAPI_EX( CvStatus, icvCvt_64f32f, "ippsConvert_64f32f",
            CV_PLUGINS1(CV_PLUGIN_IPPS), ( const double* src, float* dst, int len ))

/* dst(idx) = src(idx)*a + b */
IPCVAPI_EX( CvStatus, icvScale_32f, "ippsScale_32f", 0, ( const float* src, float* dst,
                                                          int len, float a, float b ))
IPCVAPI_EX( CvStatus, icvScale_64f, "ippsScale_64f", 0, ( const double* src, double* dst,
                                                          int len, double a, double b ))

#define IPCV_COPYSET( flavor, arrtype, scalartype )                                 \
IPCVAPI_EX( CvStatus, icvCopy##flavor, "ippiCopy" #flavor,                          \
                                    CV_PLUGINS1(CV_PLUGIN_IPPI),                    \
                                   ( const arrtype* src, int srcstep,               \
                                     arrtype* dst, int dststep, CvSize size,        \
                                     const uchar* mask, int maskstep ))             \
IPCVAPI_EX( CvStatus, icvSet##flavor, "ippiSet" #flavor,                            \
                                    CV_PLUGINS1(CV_PLUGIN_OPTCV),                   \
                                  ( arrtype* dst, int dststep,                      \
                                    const uchar* mask, int maskstep,                \
                                    CvSize size, const arrtype* scalar ))

IPCV_COPYSET( _8u_C1MR, uchar, int )
IPCV_COPYSET( _16s_C1MR, ushort, int )
IPCV_COPYSET( _8u_C3MR, uchar, int )
IPCV_COPYSET( _8u_C4MR, int, int )
IPCV_COPYSET( _16s_C3MR, ushort, int )
IPCV_COPYSET( _16s_C4MR, int64, int64 )
IPCV_COPYSET( _32f_C3MR, int, int )
IPCV_COPYSET( _32f_C4MR, int, int )
IPCV_COPYSET( _64s_C3MR, int64, int64 )
IPCV_COPYSET( _64s_C4MR, int64, int64 )


/****************************************************************************************\
*                                       Arithmetics                                      *
\****************************************************************************************/

#define IPCV_BIN_ARITHM( name )                                     \
IPCVAPI_EX( CvStatus, icv##name##_8u_C1R,                           \
    "ippi" #name "_8u_C1RSfs", CV_PLUGINS1(CV_PLUGIN_IPPI),         \
( const uchar* src1, int srcstep1, const uchar* src2, int srcstep2, \
  uchar* dst, int dststep, CvSize size, int scalefactor ))          \
IPCVAPI_EX( CvStatus, icv##name##_16u_C1R,                          \
    "ippi" #name "_16u_C1RSfs", CV_PLUGINS1(CV_PLUGIN_IPPI),        \
( const ushort* src1, int srcstep1, const ushort* src2, int srcstep2,\
  ushort* dst, int dststep, CvSize size, int scalefactor ))         \
IPCVAPI_EX( CvStatus, icv##name##_16s_C1R,                          \
    "ippi" #name "_16s_C1RSfs", CV_PLUGINS1(CV_PLUGIN_IPPI),        \
( const short* src1, int srcstep1, const short* src2, int srcstep2, \
  short* dst, int dststep, CvSize size, int scalefactor ))          \
IPCVAPI_EX( CvStatus, icv##name##_32s_C1R,                          \
    "ippi" #name "_32s_C1R", CV_PLUGINS1(CV_PLUGIN_IPPI),           \
( const int* src1, int srcstep1, const int* src2, int srcstep2,     \
  int* dst, int dststep, CvSize size ))                             \
IPCVAPI_EX( CvStatus, icv##name##_32f_C1R,                          \
    "ippi" #name "_32f_C1R", CV_PLUGINS1(CV_PLUGIN_IPPI),           \
( const float* src1, int srcstep1, const float* src2, int srcstep2, \
  float* dst, int dststep, CvSize size ))                           \
IPCVAPI_EX( CvStatus, icv##name##_64f_C1R,                          \
    "ippi" #name "_64f_C1R", CV_PLUGINS1(CV_PLUGIN_IPPI),           \
( const double* src1, int srcstep1, const double* src2, int srcstep2,\
  double* dst, int dststep, CvSize size ))


#define IPCV_UN_ARITHM( name )                                      \
IPCVAPI_EX( CvStatus, icv##name##_8u_C1R,                           \
    "ippcv" #name "_8u_C1R", CV_PLUGINS1(CV_PLUGIN_OPTCV),          \
( const uchar* src, int srcstep, uchar* dst, int dststep,           \
  CvSize size, const int* scalar ))                                 \
IPCVAPI_EX( CvStatus, icv##name##_16u_C1R,                          \
    "ippcv" #name "_16u_C1R", CV_PLUGINS1(CV_PLUGIN_OPTCV),         \
( const ushort* src, int srcstep, ushort* dst, int dststep,         \
  CvSize size, const int* scalar ))                                 \
IPCVAPI_EX( CvStatus, icv##name##_16s_C1R,                          \
    "ippcv" #name "_16s_C1R", CV_PLUGINS1(CV_PLUGIN_OPTCV),         \
( const short* src, int srcstep, short* dst, int dststep,           \
  CvSize size, const int* scalar ))                                 \
IPCVAPI_EX( CvStatus, icv##name##_32s_C1R,                          \
    "ippcv" #name "_32s_C1R", CV_PLUGINS1(CV_PLUGIN_OPTCV),         \
( const int* src, int srcstep, int* dst, int dststep,               \
  CvSize size, const int* scalar ))                                 \
IPCVAPI_EX( CvStatus, icv##name##_32f_C1R,                          \
    "ippcv" #name "_32f_C1R", CV_PLUGINS1(CV_PLUGIN_OPTCV),         \
( const float* src, int srcstep, float* dst, int dststep,           \
  CvSize size, const float* scalar ))                               \
IPCVAPI_EX( CvStatus, icv##name##_64f_C1R,                          \
    "ippcv" #name "_64f_C1R", CV_PLUGINS1(CV_PLUGIN_OPTCV),         \
( const double* src, int srcstep, double* dst, int dststep,         \
  CvSize size, const double* scalar ))


IPCV_BIN_ARITHM( Add )
IPCV_BIN_ARITHM( Sub )
IPCV_UN_ARITHM( AddC )
IPCV_UN_ARITHM( SubRC )

#undef IPCV_BIN_ARITHM
#undef IPCV_UN_ARITHM

#define IPCV_MUL( flavor, arrtype )                                 \
IPCVAPI_EX( CvStatus, icvMul_##flavor##_C1R,                        \
    "ippcvMul_" #flavor "_C1R", CV_PLUGINS1(CV_PLUGIN_OPTCV),       \
( const arrtype* src1, int step1, const arrtype* src2, int step2,   \
  arrtype* dst, int step, CvSize size, double scale ))

IPCV_MUL( 8u, uchar )
IPCV_MUL( 16u, ushort )
IPCV_MUL( 16s, short )
IPCV_MUL( 32s, int )
IPCV_MUL( 32f, float )
IPCV_MUL( 64f, double )

#undef IPCV_MUL

/****************************************************************************************\
*                                     Logical operations                                 *
\****************************************************************************************/

#define IPCV_LOGIC( name )                                              \
IPCVAPI_EX( CvStatus, icv##name##_8u_C1R,                               \
    "ippi" #name "_8u_C1R", 0/*CV_PLUGINS1(CV_PLUGIN_IPPI)*/,           \
( const uchar* src1, int srcstep1, const uchar* src2, int srcstep2,     \
  uchar* dst, int dststep, CvSize size ))                               \
IPCVAPI_EX( CvStatus, icv##name##C_8u_CnR,                              \
    "ippi" #name "C_8u_CnR", CV_PLUGINS1(CV_PLUGIN_OPTCV),              \
( const uchar* src1, int srcstep1, uchar* dst, int dststep,             \
  CvSize, const uchar* scalar, int pix_size ))

IPCV_LOGIC( And )
IPCV_LOGIC( Or )
IPCV_LOGIC( Xor )

#undef IPCV_LOGIC

IPCVAPI_EX( CvStatus, icvNot_8u_C1R, "ippiNot_8u_C1R", 0/*CV_PLUGINS1(CV_PLUGIN_IPPI)*/,
( const uchar* src, int step1, uchar* dst, int step, CvSize size ))

/****************************************************************************************\
*                                Image Statistics                                        *
\****************************************************************************************/

///////////////////////////////////////// Mean //////////////////////////////////////////

#define IPCV_DEF_MEAN_MASK( flavor, srctype )           \
IPCVAPI_EX( CvStatus, icvMean_##flavor##_C1MR,          \
"ippiMean_" #flavor "_C1MR", CV_PLUGINS1(CV_PLUGIN_IPPCV), \
( const srctype* img, int imgstep, const uchar* mask,   \
  int maskStep, CvSize size, double* mean ))            \
IPCVAPI_EX( CvStatus, icvMean_##flavor##_C2MR,          \
"ippiMean_" #flavor "_C2MR", CV_PLUGINS1(CV_PLUGIN_OPTCV), \
( const srctype* img, int imgstep, const uchar* mask,   \
  int maskStep, CvSize size, double* mean ))            \
IPCVAPI_EX( CvStatus, icvMean_##flavor##_C3MR,          \
"ippiMean_" #flavor "_C3MR", CV_PLUGINS1(CV_PLUGIN_OPTCV), \
( const srctype* img, int imgstep, const uchar* mask,   \
  int maskStep, CvSize size, double* mean ))            \
IPCVAPI_EX( CvStatus, icvMean_##flavor##_C4MR,          \
"ippiMean_" #flavor "_C4MR", CV_PLUGINS1(CV_PLUGIN_OPTCV), \
( const srctype* img, int imgstep, const uchar* mask,   \
  int maskStep, CvSize size, double* mean ))

IPCV_DEF_MEAN_MASK( 8u, uchar )
IPCV_DEF_MEAN_MASK( 16u, ushort )
IPCV_DEF_MEAN_MASK( 16s, short )
IPCV_DEF_MEAN_MASK( 32s, int )
IPCV_DEF_MEAN_MASK( 32f, float )
IPCV_DEF_MEAN_MASK( 64f, double )

#undef IPCV_DEF_MEAN_MASK

//////////////////////////////////// Mean_StdDev ////////////////////////////////////////

#define IPCV_DEF_MEAN_SDV( flavor, srctype )                                \
IPCVAPI_EX( CvStatus, icvMean_StdDev_##flavor##_C1R,                        \
"ippiMean_StdDev_" #flavor "_C1R", CV_PLUGINS1(CV_PLUGIN_IPPCV),            \
( const srctype* img, int imgstep, CvSize size, double* mean, double* sdv ))\
IPCVAPI_EX( CvStatus, icvMean_StdDev_##flavor##_C2R,                        \
"ippiMean_StdDev_" #flavor "_C2R", CV_PLUGINS1(CV_PLUGIN_OPTCV),            \
( const srctype* img, int imgstep, CvSize size, double* mean, double* sdv ))\
IPCVAPI_EX( CvStatus, icvMean_StdDev_##flavor##_C3R,                        \
"ippiMean_StdDev_" #flavor "_C3R", CV_PLUGINS1(CV_PLUGIN_OPTCV),            \
( const srctype* img, int imgstep, CvSize size, double* mean, double* sdv ))\
IPCVAPI_EX( CvStatus, icvMean_StdDev_##flavor##_C4R,                        \
"ippiMean_StdDev_" #flavor "_C4R", CV_PLUGINS1(CV_PLUGIN_OPTCV),            \
( const srctype* img, int imgstep, CvSize size, double* mean, double* sdv ))\
                                                                            \
IPCVAPI_EX( CvStatus, icvMean_StdDev_##flavor##_C1MR,                       \
"ippiMean_StdDev_" #flavor "_C1MR", CV_PLUGINS1(CV_PLUGIN_IPPCV),           \
( const srctype* img, int imgstep,                                          \
  const uchar* mask, int maskStep,                                          \
  CvSize size, double* mean, double* sdv ))                                 \
IPCVAPI_EX( CvStatus, icvMean_StdDev_##flavor##_C2MR,                       \
"ippiMean_StdDev_" #flavor "_C2MR", CV_PLUGINS1(CV_PLUGIN_OPTCV),           \
( const srctype* img, int imgstep,  const uchar* mask, int maskStep,        \
  CvSize size, double* mean, double* sdv ))                                 \
IPCVAPI_EX( CvStatus, icvMean_StdDev_##flavor##_C3MR,                       \
"ippiMean_StdDev_" #flavor "_C3MR", CV_PLUGINS1(CV_PLUGIN_OPTCV),           \
( const srctype* img, int imgstep,                                          \
  const uchar* mask, int maskStep,                                          \
  CvSize size, double* mean, double* sdv ))                                 \
IPCVAPI_EX( CvStatus, icvMean_StdDev_##flavor##_C4MR,                       \
"ippiMean_StdDev_" #flavor "_C4MR", CV_PLUGINS1(CV_PLUGIN_OPTCV),           \
( const srctype* img, int imgstep,                                          \
  const uchar* mask, int maskStep,                                          \
  CvSize size, double* mean, double* sdv ))

IPCV_DEF_MEAN_SDV( 8u, uchar )
IPCV_DEF_MEAN_SDV( 16u, ushort )
IPCV_DEF_MEAN_SDV( 16s, short )
IPCV_DEF_MEAN_SDV( 32s, int )
IPCV_DEF_MEAN_SDV( 32f, float )
IPCV_DEF_MEAN_SDV( 64f, double )

#undef IPCV_DEF_MEAN_SDV

//////////////////////////////////// MinMaxIndx /////////////////////////////////////////


#define IPCV_DEF_MIN_MAX_LOC( flavor, srctype, extrtype )       \
IPCVAPI_EX( CvStatus, icvMinMaxIndx_##flavor##_C1R,             \
"ippiMinMaxIndx_" #flavor "_C1R", CV_PLUGINS1(CV_PLUGIN_IPPCV), \
( const srctype* img, int imgstep,                              \
  CvSize size, extrtype* minVal, extrtype* maxVal,              \
  CvPoint* minLoc, CvPoint* maxLoc ))                           \
                                                                \
IPCVAPI_EX( CvStatus, icvMinMaxIndx_##flavor##_C1MR,            \
"ippiMinMaxIndx_" #flavor "_C1MR", CV_PLUGINS1(CV_PLUGIN_IPPCV),\
( const srctype* img, int imgstep,                              \
  const uchar* mask, int maskStep,                              \
  CvSize size, extrtype* minVal, extrtype* maxVal,              \
  CvPoint* minLoc, CvPoint* maxLoc ))

IPCV_DEF_MIN_MAX_LOC( 8u, uchar, float )
IPCV_DEF_MIN_MAX_LOC( 16u, ushort, float )
IPCV_DEF_MIN_MAX_LOC( 16s, short, float )
IPCV_DEF_MIN_MAX_LOC( 32s, int, double )
IPCV_DEF_MIN_MAX_LOC( 32f, float, float )
IPCV_DEF_MIN_MAX_LOC( 64f, double, double )

#undef IPCV_MIN_MAX_LOC

////////////////////////////////////////// Sum //////////////////////////////////////////

#define IPCV_DEF_SUM_NOHINT( flavor, srctype )                              \
IPCVAPI_EX( CvStatus, icvSum_##flavor##_C1R,                                \
            "ippiSum_" #flavor "_C1R", CV_PLUGINS1(CV_PLUGIN_IPPI),         \
                                         ( const srctype* img, int imgstep, \
                                           CvSize size, double* sum ))      \
IPCVAPI_EX( CvStatus, icvSum_##flavor##_C2R,                                \
           "ippiSum_" #flavor "_C2R", CV_PLUGINS1(CV_PLUGIN_IPPI),          \
                                         ( const srctype* img, int imgstep, \
                                           CvSize size, double* sum ))      \
IPCVAPI_EX( CvStatus, icvSum_##flavor##_C3R,                                \
           "ippiSum_" #flavor "_C3R", CV_PLUGINS1(CV_PLUGIN_IPPI),          \
                                         ( const srctype* img, int imgstep, \
                                           CvSize size, double* sum ))      \
IPCVAPI_EX( CvStatus, icvSum_##flavor##_C4R,                                \
           "ippiSum_" #flavor "_C4R", CV_PLUGINS1(CV_PLUGIN_IPPI),          \
                                         ( const srctype* img, int imgstep, \
                                           CvSize size, double* sum ))

#define IPCV_DEF_SUM_HINT( flavor, srctype )                                \
IPCVAPI_EX( CvStatus, icvSum_##flavor##_C1R,                                \
            "ippiSum_" #flavor "_C1R", CV_PLUGINS1(CV_PLUGIN_IPPI),         \
                        ( const srctype* img, int imgstep,                  \
                          CvSize size, double* sum, CvHintAlgorithm ))      \
IPCVAPI_EX( CvStatus, icvSum_##flavor##_C2R,                                \
           "ippiSum_" #flavor "_C2R", CV_PLUGINS1(CV_PLUGIN_IPPI),          \
                        ( const srctype* img, int imgstep,                  \
                          CvSize size, double* sum, CvHintAlgorithm ))      \
IPCVAPI_EX( CvStatus, icvSum_##flavor##_C3R,                                \
           "ippiSum_" #flavor "_C3R", CV_PLUGINS1(CV_PLUGIN_IPPI),          \
                        ( const srctype* img, int imgstep,                  \
                          CvSize size, double* sum, CvHintAlgorithm ))      \
IPCVAPI_EX( CvStatus, icvSum_##flavor##_C4R,                                \
           "ippiSum_" #flavor "_C4R", CV_PLUGINS1(CV_PLUGIN_IPPI),          \
                        ( const srctype* img, int imgstep,                  \
                          CvSize size, double* sum, CvHintAlgorithm ))

IPCV_DEF_SUM_NOHINT( 8u, uchar )
IPCV_DEF_SUM_NOHINT( 16u, ushort )
IPCV_DEF_SUM_NOHINT( 16s, short )
IPCV_DEF_SUM_NOHINT( 32s, int )
IPCV_DEF_SUM_HINT( 32f, float )
IPCV_DEF_SUM_NOHINT( 64f, double )

#undef IPCV_DEF_SUM_NOHINT
#undef IPCV_DEF_SUM_HINT

////////////////////////////////////////// CountNonZero /////////////////////////////////

#define IPCV_DEF_NON_ZERO( flavor, srctype )                            \
IPCVAPI_EX( CvStatus, icvCountNonZero_##flavor##_C1R,                   \
    "ippiCountNonZero_" #flavor "_C1R", CV_PLUGINS1(CV_PLUGIN_OPTCV),   \
    ( const srctype* img, int imgstep, CvSize size, int* nonzero ))

IPCV_DEF_NON_ZERO( 8u, uchar )
IPCV_DEF_NON_ZERO( 16s, ushort )
IPCV_DEF_NON_ZERO( 32s, int )
IPCV_DEF_NON_ZERO( 32f, int )
IPCV_DEF_NON_ZERO( 64f, int64 )

#undef IPCV_DEF_NON_ZERO

////////////////////////////////////////// Norms /////////////////////////////////

#define IPCV_DEF_NORM_NOHINT_C1( flavor, srctype )                                      \
IPCVAPI_EX( CvStatus, icvNorm_Inf_##flavor##_C1R,                                       \
            "ippiNorm_Inf_" #flavor "_C1R", CV_PLUGINS1(CV_PLUGIN_IPPI),                \
                                             ( const srctype* img, int imgstep,         \
                                               CvSize size, double* norm ))             \
IPCVAPI_EX( CvStatus, icvNorm_L1_##flavor##_C1R,                                        \
           "ippiNorm_L1_" #flavor "_C1R", CV_PLUGINS1(CV_PLUGIN_IPPI),                  \
                                             ( const srctype* img, int imgstep,         \
                                               CvSize size, double* norm ))             \
IPCVAPI_EX( CvStatus, icvNorm_L2_##flavor##_C1R,                                        \
           "ippiNorm_L2_" #flavor "_C1R", CV_PLUGINS1(CV_PLUGIN_IPPI),                  \
                                             ( const srctype* img, int imgstep,         \
                                               CvSize size, double* norm ))             \
IPCVAPI_EX( CvStatus, icvNormDiff_Inf_##flavor##_C1R,                                   \
           "ippiNormDiff_Inf_" #flavor "_C1R", CV_PLUGINS1(CV_PLUGIN_IPPI),             \
                                             ( const srctype* img1, int imgstep1,       \
                                               const srctype* img2, int imgstep2,       \
                                               CvSize size, double* norm ))             \
IPCVAPI_EX( CvStatus, icvNormDiff_L1_##flavor##_C1R,                                    \
           "ippiNormDiff_L1_" #flavor "_C1R", CV_PLUGINS1(CV_PLUGIN_IPPI),              \
                                             ( const srctype* img1, int imgstep1,       \
                                               const srctype* img2, int imgstep2,       \
                                               CvSize size, double* norm ))             \
IPCVAPI_EX( CvStatus, icvNormDiff_L2_##flavor##_C1R,                                    \
           "ippiNormDiff_L2_" #flavor "_C1R", CV_PLUGINS1(CV_PLUGIN_IPPI),              \
                                             ( const srctype* img1, int imgstep1,       \
                                               const srctype* img2, int imgstep2,       \
                                               CvSize size, double* norm ))

#define IPCV_DEF_NORM_HINT_C1( flavor, srctype )                                        \
IPCVAPI_EX( CvStatus, icvNorm_Inf_##flavor##_C1R,                                       \
            "ippiNorm_Inf_" #flavor "_C1R", CV_PLUGINS1(CV_PLUGIN_IPPI),                \
                                        ( const srctype* img, int imgstep,              \
                                          CvSize size, double* norm ))                  \
IPCVAPI_EX( CvStatus, icvNorm_L1_##flavor##_C1R,                                        \
           "ippiNorm_L1_" #flavor "_C1R", CV_PLUGINS1(CV_PLUGIN_IPPI),                  \
                                        ( const srctype* img, int imgstep,              \
                                          CvSize size, double* norm, CvHintAlgorithm )) \
IPCVAPI_EX( CvStatus, icvNorm_L2_##flavor##_C1R,                                        \
           "ippiNorm_L2_" #flavor "_C1R", CV_PLUGINS1(CV_PLUGIN_IPPI),                  \
                                        ( const srctype* img, int imgstep,              \
                                          CvSize size, double* norm, CvHintAlgorithm )) \
IPCVAPI_EX( CvStatus, icvNormDiff_Inf_##flavor##_C1R,                                   \
           "ippiNormDiff_Inf_" #flavor "_C1R", CV_PLUGINS1(CV_PLUGIN_IPPI),             \
                                        ( const srctype* img1, int imgstep1,            \
                                          const srctype* img2, int imgstep2,            \
                                          CvSize size, double* norm ))                  \
IPCVAPI_EX( CvStatus, icvNormDiff_L1_##flavor##_C1R,                                    \
           "ippiNormDiff_L1_" #flavor "_C1R", CV_PLUGINS1(CV_PLUGIN_IPPI),              \
                                        ( const srctype* img1, int imgstep1,            \
                                          const srctype* img2, int imgstep2,            \
                                          CvSize size, double* norm, CvHintAlgorithm )) \
IPCVAPI_EX( CvStatus, icvNormDiff_L2_##flavor##_C1R,                                    \
           "ippiNormDiff_L2_" #flavor "_C1R", CV_PLUGINS1(CV_PLUGIN_IPPI),              \
                                        ( const srctype* img1, int imgstep1,            \
                                          const srctype* img2, int imgstep2,            \
                                          CvSize size, double* norm, CvHintAlgorithm ))

#define IPCV_DEF_NORM_MASK_C1( flavor, srctype )                                        \
IPCVAPI_EX( CvStatus, icvNorm_Inf_##flavor##_C1MR,                                      \
           "ippiNorm_Inf_" #flavor "_C1MR", CV_PLUGINS1(CV_PLUGIN_IPPCV),               \
                                             ( const srctype* img, int imgstep,         \
                                               const uchar* mask, int maskstep,         \
                                               CvSize size, double* norm ))             \
IPCVAPI_EX( CvStatus, icvNorm_L1_##flavor##_C1MR,                                       \
            "ippiNorm_L1_" #flavor "_C1MR", CV_PLUGINS1(CV_PLUGIN_IPPCV),               \
                                             ( const srctype* img, int imgstep,         \
                                                const uchar* mask, int maskstep,        \
                                                CvSize size, double* norm ))            \
IPCVAPI_EX( CvStatus, icvNorm_L2_##flavor##_C1MR,                                       \
           "ippiNorm_L2_" #flavor "_C1MR", CV_PLUGINS1(CV_PLUGIN_IPPCV),                \
                                             ( const srctype* img, int imgstep,         \
                                               const uchar* mask, int maskstep,         \
                                               CvSize size, double* norm ))             \
IPCVAPI_EX( CvStatus, icvNormDiff_Inf_##flavor##_C1MR,                                  \
           "ippiNormDiff_Inf_" #flavor "_C1MR", CV_PLUGINS1(CV_PLUGIN_IPPCV),           \
                                             ( const srctype* img1, int imgstep1,       \
                                               const srctype* img2, int imgstep2,       \
                                               const uchar* mask, int maskstep,         \
                                               CvSize size, double* norm ))             \
IPCVAPI_EX( CvStatus, icvNormDiff_L1_##flavor##_C1MR,                                   \
           "ippiNormDiff_L1_" #flavor "_C1MR", CV_PLUGINS1(CV_PLUGIN_IPPCV),            \
                                             ( const srctype* img1, int imgstep1,       \
                                               const srctype* img2, int imgstep2,       \
                                               const uchar* mask, int maskstep,         \
                                               CvSize size, double* norm ))             \
IPCVAPI_EX( CvStatus, icvNormDiff_L2_##flavor##_C1MR,                                   \
           "ippiNormDiff_L2_" #flavor "_C1MR", CV_PLUGINS1(CV_PLUGIN_IPPCV),            \
                                             ( const srctype* img1, int imgstep1,       \
                                               const srctype* img2, int imgstep2,       \
                                               const uchar* mask, int maskstep,         \
                                               CvSize size, double* norm ))

IPCV_DEF_NORM_NOHINT_C1( 8u, uchar )
IPCV_DEF_NORM_MASK_C1( 8u, uchar )

IPCV_DEF_NORM_NOHINT_C1( 16u, ushort )
IPCV_DEF_NORM_MASK_C1( 16u, ushort )

IPCV_DEF_NORM_NOHINT_C1( 16s, short )
IPCV_DEF_NORM_MASK_C1( 16s, short )

IPCV_DEF_NORM_NOHINT_C1( 32s, int )
IPCV_DEF_NORM_MASK_C1( 32s, int )

IPCV_DEF_NORM_HINT_C1( 32f, float )
IPCV_DEF_NORM_MASK_C1( 32f, float )

IPCV_DEF_NORM_NOHINT_C1( 64f, double )
IPCV_DEF_NORM_MASK_C1( 64f, double )

#undef IPCV_DEF_NORM_HONINT_C1
#undef IPCV_DEF_NORM_HINT_C1
#undef IPCV_DEF_NORM_MASK_C1


////////////////////////////////////// AbsDiff ///////////////////////////////////////////

#define IPCV_ABS_DIFF( flavor, arrtype )                    \
IPCVAPI_EX( CvStatus, icvAbsDiff_##flavor##_C1R,            \
"ippiAbsDiff_" #flavor "_C1R", CV_PLUGINS1(CV_PLUGIN_IPPCV),   \
( const arrtype* src1, int srcstep1,                        \
  const arrtype* src2, int srcstep2,                        \
  arrtype* dst, int dststep, CvSize size ))

IPCV_ABS_DIFF( 8u, uchar )
IPCV_ABS_DIFF( 16u, ushort )
IPCV_ABS_DIFF( 16s, short )
IPCV_ABS_DIFF( 32s, int )
IPCV_ABS_DIFF( 32f, float )
IPCV_ABS_DIFF( 64f, double )

#undef IPCV_ABS_DIFF

#define IPCV_ABS_DIFF_C( flavor, arrtype, scalartype )      \
IPCVAPI_EX( CvStatus, icvAbsDiffC_##flavor##_CnR,           \
"ippiAbsDiffC_" #flavor "_CnR", CV_PLUGINS1(CV_PLUGIN_OPTCV),\
( const arrtype* src1, int srcstep1,                        \
  arrtype* dst, int dststep,                                \
  CvSize size, const scalartype* scalar ))

IPCV_ABS_DIFF_C( 8u, uchar, int )
IPCV_ABS_DIFF_C( 16u, ushort, int )
IPCV_ABS_DIFF_C( 16s, short, int )
IPCV_ABS_DIFF_C( 32s, int, int )
IPCV_ABS_DIFF_C( 32f, float, float )
IPCV_ABS_DIFF_C( 64f, double, double )

#undef IPCV_ABS_DIFF_C

////////////////////////////// Comparisons //////////////////////////////////////////

#define IPCV_CMP( arrtype, flavor )                                                 \
IPCVAPI_EX( CvStatus, icvCompare_##flavor##_C1R,                                    \
            "ippiCompare_" #flavor "_C1R", CV_PLUGINS1(CV_PLUGIN_IPPI),             \
            ( const arrtype* src1, int srcstep1, const arrtype* src2, int srcstep2, \
              arrtype* dst, int dststep, CvSize size, int cmp_op ))                 \
IPCVAPI_EX( CvStatus, icvCompareC_##flavor##_C1R,                                   \
            "ippiCompareC_" #flavor "_C1R", CV_PLUGINS1(CV_PLUGIN_IPPI),            \
            ( const arrtype* src1, int srcstep1, arrtype scalar,                    \
              arrtype* dst, int dststep, CvSize size, int cmp_op ))                 \
IPCVAPI_EX( CvStatus, icvThreshold_GT_##flavor##_C1R,                               \
            "ippiThreshold_GT_" #flavor "_C1R", CV_PLUGINS1(CV_PLUGIN_IPPI),        \
            ( const arrtype* pSrc, int srcstep, arrtype* pDst, int dststep,         \
              CvSize size, arrtype threshold ))                                     \
IPCVAPI_EX( CvStatus, icvThreshold_LT_##flavor##_C1R,                               \
            "ippiThreshold_LT_" #flavor "_C1R", CV_PLUGINS1(CV_PLUGIN_IPPI),        \
            ( const arrtype* pSrc, int srcstep, arrtype* pDst, int dststep,         \
              CvSize size, arrtype threshold ))
IPCV_CMP( uchar, 8u )
IPCV_CMP( short, 16s )
IPCV_CMP( float, 32f )
#undef IPCV_CMP

/****************************************************************************************\
*                                       Utilities                                        *
\****************************************************************************************/

////////////////////////////// Copy Pixel <-> Plane /////////////////////////////////

#define IPCV_PIX_PLANE( flavor, arrtype )                                           \
IPCVAPI_EX( CvStatus, icvCopy_##flavor##_C2P2R,                                     \
    "ippiCopy_" #flavor "_C2P2R", CV_PLUGINS2(CV_PLUGIN_IPPI,CV_PLUGIN_OPTCV),      \
    ( const arrtype* src, int srcstep, arrtype** dst, int dststep, CvSize size ))   \
IPCVAPI_EX( CvStatus, icvCopy_##flavor##_C3P3R,                                     \
    "ippiCopy_" #flavor "_C3P3R", CV_PLUGINS2(CV_PLUGIN_IPPI,CV_PLUGIN_OPTCV),      \
    ( const arrtype* src, int srcstep, arrtype** dst, int dststep, CvSize size ))   \
IPCVAPI_EX( CvStatus, icvCopy_##flavor##_C4P4R,                                     \
    "ippiCopy_" #flavor "_C4P4R", CV_PLUGINS2(CV_PLUGIN_IPPI,CV_PLUGIN_OPTCV),      \
    ( const arrtype* src, int srcstep, arrtype** dst, int dststep, CvSize size ))   \
IPCVAPI_EX( CvStatus, icvCopy_##flavor##_CnC1CR,                                    \
    "ippiCopy_" #flavor "_CnC1CR", CV_PLUGINS1(CV_PLUGIN_OPTCV),                    \
    ( const arrtype* src, int srcstep, arrtype* dst, int dststep,                   \
      CvSize size, int cn, int coi ))                                               \
IPCVAPI_EX( CvStatus, icvCopy_##flavor##_C1CnCR,                                    \
    "ippiCopy_" #flavor "_CnC1CR", CV_PLUGINS1(CV_PLUGIN_OPTCV),                    \
    ( const arrtype* src, int srcstep, arrtype* dst, int dststep,                   \
      CvSize size, int cn, int coi ))                                               \
IPCVAPI_EX( CvStatus, icvCopy_##flavor##_P2C2R,                                     \
    "ippiCopy_" #flavor "_P2C2R", CV_PLUGINS2(CV_PLUGIN_IPPI,CV_PLUGIN_OPTCV),      \
    ( const arrtype** src, int srcstep, arrtype* dst, int dststep, CvSize size ))   \
IPCVAPI_EX( CvStatus, icvCopy_##flavor##_P3C3R,                                     \
    "ippiCopy_" #flavor "_P3C3R", CV_PLUGINS2(CV_PLUGIN_IPPI,CV_PLUGIN_OPTCV),      \
    ( const arrtype** src, int srcstep, arrtype* dst, int dststep, CvSize size ))   \
IPCVAPI_EX( CvStatus, icvCopy_##flavor##_P4C4R,                                     \
    "ippiCopy_" #flavor "_P4C4R", CV_PLUGINS2(CV_PLUGIN_IPPI,CV_PLUGIN_OPTCV),      \
    ( const arrtype** src, int srcstep, arrtype* dst, int dststep, CvSize size ))

IPCV_PIX_PLANE( 8u, uchar )
IPCV_PIX_PLANE( 16s, ushort )
IPCV_PIX_PLANE( 32f, int )
IPCV_PIX_PLANE( 64f, int64 )

#undef IPCV_PIX_PLANE

/****************************************************************************************/
/*                            Math routines and RNGs                                    */
/****************************************************************************************/

IPCVAPI_EX( CvStatus, icvInvSqrt_32f, "ippsInvSqrt_32f_A21, ippibInvSqrt_32f",
           CV_PLUGINS2(CV_PLUGIN_IPPVM,CV_PLUGIN_OPTCV),
           ( const float* src, float* dst, int len ))
IPCVAPI_EX( CvStatus, icvSqrt_32f, "ippsSqrt_32f_A21, ippsSqrt_32f, ippibSqrt_32f",
           CV_PLUGINS3(CV_PLUGIN_IPPVM,CV_PLUGIN_IPPS,CV_PLUGIN_OPTCV),
           ( const float* src, float* dst, int len ))
IPCVAPI_EX( CvStatus, icvInvSqrt_64f, "ippsInvSqrt_64f_A50, ippibInvSqrt_64f",
           CV_PLUGINS2(CV_PLUGIN_IPPVM,CV_PLUGIN_OPTCV),
           ( const double* src, double* dst, int len ))
IPCVAPI_EX( CvStatus, icvSqrt_64f, "ippsSqrt_64f_A50, ippsSqrt_64f, ippibSqrt_64f",
           CV_PLUGINS3(CV_PLUGIN_IPPVM,CV_PLUGIN_IPPS,CV_PLUGIN_OPTCV),
           ( const double* src, double* dst, int len ))

IPCVAPI_EX( CvStatus, icvLog_32f, "ippsLn_32f_A21, ippsLn_32f, ippibLog_32f",
           CV_PLUGINS3(CV_PLUGIN_IPPVM,CV_PLUGIN_IPPS,CV_PLUGIN_OPTCV),
           ( const float *x, float *y, int n ) )
IPCVAPI_EX( CvStatus, icvLog_64f, "ippsLn_64f_A50, ippsLn_64f, ippibLog_64f",
           CV_PLUGINS3(CV_PLUGIN_IPPVM,CV_PLUGIN_IPPS,CV_PLUGIN_OPTCV),
           ( const double *x, double *y, int n ) )
IPCVAPI_EX( CvStatus, icvExp_32f, "ippsExp_32f_A21, ippsExp_32f, ippibExp_32f",
           CV_PLUGINS3(CV_PLUGIN_IPPVM,CV_PLUGIN_IPPS,CV_PLUGIN_OPTCV),
           ( const float *x, float *y, int n ) )
IPCVAPI_EX( CvStatus, icvExp_64f, "ippsExp_64f_A50, ippsExp_64f, ippibExp_64f",
           CV_PLUGINS3(CV_PLUGIN_IPPVM,CV_PLUGIN_OPTCV,CV_PLUGIN_IPPS),
           ( const double *x, double *y, int n ) )
IPCVAPI_EX( CvStatus, icvFastArctan_32f, "ippibFastArctan_32f",
           CV_PLUGINS1(CV_PLUGIN_IPPCV),
           ( const float* y, const float* x, float* angle, int len ))

/****************************************************************************************/
/*                                  Error handling functions                            */
/****************************************************************************************/

IPCVAPI_EX( CvStatus, icvCheckArray_32f_C1R,
           "ippiCheckArray_32f_C1R", CV_PLUGINS1(CV_PLUGIN_OPTCV),
           ( const float* src, int srcstep,
             CvSize size, int flags,
             double min_val, double max_val ))

IPCVAPI_EX( CvStatus, icvCheckArray_64f_C1R,
           "ippiCheckArray_64f_C1R", CV_PLUGINS1(CV_PLUGIN_OPTCV),
           ( const double* src, int srcstep,
             CvSize size, int flags,
             double min_val, double max_val ))

/****************************************************************************************/
/*                                Vector operations                                     */
/****************************************************************************************/

#define IPCV_DOTPRODUCT_2D( flavor, arrtype, sumtype )                  \
IPCVAPI_EX( CvStatus, icvDotProduct_##flavor##_C1R,                     \
           "ippiDotProduct_" #flavor "_C1R", CV_PLUGINS1(CV_PLUGIN_OPTCV), \
                              ( const arrtype* src1, int step1,         \
                                const arrtype* src2, int step2,         \
                                CvSize size, sumtype* _sum ))

IPCV_DOTPRODUCT_2D( 8u, uchar, int64 )
IPCV_DOTPRODUCT_2D( 16u, ushort, int64 )
IPCV_DOTPRODUCT_2D( 16s, short, int64 )
IPCV_DOTPRODUCT_2D( 32s, int, double )
IPCV_DOTPRODUCT_2D( 32f, float, double )
IPCV_DOTPRODUCT_2D( 64f, double, double )

#undef IPCV_DOTPRODUCT_2D

/****************************************************************************************/
/*                                    Linear Algebra                                    */
/****************************************************************************************/

IPCVAPI_EX( CvStatus, icvLUDecomp_32f, "ippiLUDecomp_32f_C1R", CV_PLUGINS1(CV_PLUGIN_OPTCV),
                                    ( double* A, int stepA, CvSize sizeA,
                                      float* B, int stepB, CvSize sizeB,
                                      double* _det ))

IPCVAPI_EX( CvStatus, icvLUDecomp_64f, "ippiLUDecomp_64f_C1R", CV_PLUGINS1(CV_PLUGIN_OPTCV),
                                    ( double* A, int stepA, CvSize sizeA,
                                      double* B, int stepB, CvSize sizeB,
                                      double* _det ))

IPCVAPI_EX( CvStatus, icvLUBack_32f, "ippiLUBack_32f_C1R", CV_PLUGINS1(CV_PLUGIN_OPTCV),
                                  ( double* A, int stepA, CvSize sizeA,
                                    float* B, int stepB, CvSize sizeB ))

IPCVAPI_EX( CvStatus, icvLUBack_64f, "ippiLUBack_64f_C1R", CV_PLUGINS1(CV_PLUGIN_OPTCV),
                                  ( double* A, int stepA, CvSize sizeA,
                                    double* B, int stepB, CvSize sizeB ))

/****************************************************************************************/
/*                  Matrix routines from BLAS/LAPACK compatible libraries               */
/****************************************************************************************/

IPCVAPI_C_EX( void, icvBLAS_GEMM_32f, "sgemm, mkl_sgemm", CV_PLUGINS2(CV_PLUGIN_MKL,CV_PLUGIN_MKL),
                        (const char *transa, const char *transb, int *n, int *m, int *k,
                         const void *alpha, const void *a, int *lda, const void *b, int *ldb,
                         const void *beta, void *c, int *ldc ))

IPCVAPI_C_EX( void, icvBLAS_GEMM_64f, "dgemm, mkl_dgemm", CV_PLUGINS2(CV_PLUGIN_MKL,CV_PLUGIN_MKL),
                        (const char *transa, const char *transb, int *n, int *m, int *k,
                         const void *alpha, const void *a, int *lda, const void *b, int *ldb,
                         const void *beta, void *c, int *ldc ))

IPCVAPI_C_EX( void, icvBLAS_GEMM_32fc, "cgemm, mkl_cgemm", CV_PLUGINS2(CV_PLUGIN_MKL,CV_PLUGIN_MKL),
                        (const char *transa, const char *transb, int *n, int *m, int *k,
                         const void *alpha, const void *a, int *lda, const void *b, int *ldb,
                         const void *beta, void *c, int *ldc ))

IPCVAPI_C_EX( void, icvBLAS_GEMM_64fc, "zgemm, mkl_zgemm", CV_PLUGINS2(CV_PLUGIN_MKL,CV_PLUGIN_MKL),
                        (const char *transa, const char *transb, int *n, int *m, int *k,
                         const void *alpha, const void *a, int *lda, const void *b, int *ldb,
                         const void *beta, void *c, int *ldc ))


#define IPCV_DFT( init_flavor, fwd_flavor, inv_flavor )                                 \
IPCVAPI_EX( CvStatus, icvDFTInitAlloc_##init_flavor, "ippsDFTInitAlloc_" #init_flavor,  \
            CV_PLUGINS1(CV_PLUGIN_IPPS), ( void**, int, int, CvHintAlgorithm ))         \
                                                                                        \
IPCVAPI_EX( CvStatus, icvDFTFree_##init_flavor, "ippsDFTFree_" #init_flavor,            \
            CV_PLUGINS1(CV_PLUGIN_IPPS), ( void* ))                                     \
                                                                                        \
IPCVAPI_EX( CvStatus, icvDFTGetBufSize_##init_flavor, "ippsDFTGetBufSize_" #init_flavor,\
            CV_PLUGINS1(CV_PLUGIN_IPPS), ( const void* spec, int* buf_size ))           \
                                                                                        \
IPCVAPI_EX( CvStatus, icvDFTFwd_##fwd_flavor, "ippsDFTFwd_" #fwd_flavor,                \
            CV_PLUGINS1(CV_PLUGIN_IPPS), ( const void* src, void* dst,                  \
            const void* spec, void* buffer ))                                           \
                                                                                        \
IPCVAPI_EX( CvStatus, icvDFTInv_##inv_flavor, "ippsDFTInv_" #inv_flavor,                \
            CV_PLUGINS1(CV_PLUGIN_IPPS), ( const void* src, void* dst,                  \
            const void* spec, void* buffer ))

IPCV_DFT( C_32fc, CToC_32fc, CToC_32fc )
IPCV_DFT( R_32f, RToPack_32f, PackToR_32f )
IPCV_DFT( C_64fc, CToC_64fc, CToC_64fc )
IPCV_DFT( R_64f, RToPack_64f, PackToR_64f )
#undef IPCV_DFT

/*#define IPCV_DCT( dir, flavor )                                                         \
IPCVAPI_EX( CvStatus, icvDCT##dir##InitAlloc_##flavor,                                  \
            "ippsDCT" #dir "InitAlloc_" #flavor, CV_PLUGINS1(CV_PLUGIN_IPPS),           \
            ( void** spec, int, CvHintAlgorithm ))                                      \
IPCVAPI_EX( CvStatus, icvDCT##dir##Free_##flavor,                                       \
            "ippsDCT" #dir "Free_" #flavor, CV_PLUGINS1(CV_PLUGIN_IPPS), ( void* spec ))\
IPCVAPI_EX( CvStatus, icvDCT##dir##GetBufSize_##flavor,                                 \
            "ippsDCT" #dir "GetBufSize_" #flavor, CV_PLUGINS1(CV_PLUGIN_IPPS),          \
            ( const void* spec, int* buf_size ))                                        \
IPCVAPI_EX( CvStatus, icvDCT##dir##_##flavor,                                           \
            "ippsDCT" #dir "_" #flavor, CV_PLUGINS1(CV_PLUGIN_IPPS),                    \
            ( const void* src, void* dst, const void* spec, void* buf ))

IPCV_DCT( Fwd, 32f )
IPCV_DCT( Inv, 32f )
IPCV_DCT( Fwd, 64f )
IPCV_DCT( Inv, 64f )
#undef IPCV_DCT*/

#endif /*_CXCORE_IPP_H_*/

