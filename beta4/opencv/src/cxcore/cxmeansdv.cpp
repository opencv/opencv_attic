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
*                             Mean and StdDev calculation                                *
\****************************************************************************************/


#define CV_IMPL_MEAN_SDV_1D_CASE_COI( temptype, acctype, accsqtype, \
                                      src, len, sum, sqsum, cn )    \
{                                                                   \
    int i;                                                          \
    acctype s1 = 0;                                                 \
    accsqtype sq1 = 0;                                              \
                                                                    \
    for( i = 0; i <= (len) - 4*(cn); i += 4*(cn) )                  \
    {                                                               \
        temptype t0 = (src)[i];                                     \
        temptype t1 = (src)[i + (cn)];                              \
        acctype  s;                                                 \
        accsqtype sq;                                               \
                                                                    \
        s  = (acctype)t0 + (acctype)t1;                             \
        sq = ((accsqtype)t0)*t0 + ((accsqtype)t1)*t1;               \
                                                                    \
        t0 = (src)[i + 2*(cn)];                                     \
        t1 = (src)[i + 3*(cn)];                                     \
                                                                    \
        (sum)[0] += s + (acctype)t0 + (acctype)t1;                  \
        (sqsum)[0] += sq + ((accsqtype)t0)*t0 + ((accsqtype)t1)*t1; \
    }                                                               \
                                                                    \
    for( ; i < (len); i += (cn) )                                   \
    {                                                               \
        temptype t = (src)[i];                                      \
                                                                    \
        s1 += (acctype)t;                                           \
        sq1 += ((accsqtype)t)*t;                                    \
    }                                                               \
                                                                    \
    (sum)[0] += s1;                                                 \
    (sqsum)[0] += sq1;                                              \
}


#define CV_IMPL_MEAN_SDV_1D_CASE_C1( temptype, acctype, accsqtype,  \
                                     src, len, sum, sqsum )         \
CV_IMPL_MEAN_SDV_1D_CASE_COI( temptype, acctype, accsqtype,         \
                              src, len, sum, sqsum, 1 )


#define CV_IMPL_MEAN_SDV_1D_CASE_C2( temptype, acctype, accsqtype,  \
                                     src, len, sum, sqsum )         \
{                                                                   \
    int i;                                                          \
                                                                    \
    for( i = 0; i < (len); i += 2 )                                 \
    {                                                               \
        temptype t0 = (src)[i];                                     \
        temptype t1 = (src)[i + 1];                                 \
                                                                    \
        (sum)[0] += t0;                                             \
        (sum)[1] += t1;                                             \
        (sqsum)[0] += ((accsqtype)t0)*t0;                           \
        (sqsum)[1] += ((accsqtype)t1)*t1;                           \
    }                                                               \
}


#define CV_IMPL_MEAN_SDV_1D_CASE_C3( temptype, acctype, accsqtype,  \
                                     src, len, sum, sqsum )         \
{                                                                   \
    int i;                                                          \
                                                                    \
    for( i = 0; i < (len); i += 3 )                                 \
    {                                                               \
        temptype t0 = (src)[i];                                     \
        temptype t1 = (src)[i + 1];                                 \
        temptype t2 = (src)[i + 2];                                 \
                                                                    \
        (sum)[0] += t0;                                             \
        (sum)[1] += t1;                                             \
        (sum)[2] += t2;                                             \
        (sqsum)[0] += ((accsqtype)t0)*t0;                           \
        (sqsum)[1] += ((accsqtype)t1)*t1;                           \
        (sqsum)[2] += ((accsqtype)t2)*t2;                           \
    }                                                               \
}


#define CV_IMPL_MEAN_SDV_1D_CASE_C4( temptype, acctype, accsqtype,  \
                                     src, len, sum, sqsum )         \
{                                                                   \
    int i;                                                          \
                                                                    \
    for( i = 0; i < (len); i += 4 )                                 \
    {                                                               \
        temptype t0 = (src)[i];                                     \
        temptype t1 = (src)[i + 1];                                 \
                                                                    \
        (sum)[0] += t0;                                             \
        (sum)[1] += t1;                                             \
        (sqsum)[0] += ((accsqtype)t0)*t0;                           \
        (sqsum)[1] += ((accsqtype)t1)*t1;                           \
                                                                    \
        t0 = (src)[i + 2];                                          \
        t1 = (src)[i + 3];                                          \
                                                                    \
        (sum)[2] += t0;                                             \
        (sum)[3] += t1;                                             \
        (sqsum)[2] += ((accsqtype)t0)*t0;                           \
        (sqsum)[3] += ((accsqtype)t1)*t1;                           \
    }                                                               \
}


#define CV_IMPL_MEAN_SDV_MASK_1D_CASE_C1( _mask_op_, temptype, acctype, accsqtype, \
                                          src, mask, len, sum, sqsum, pix )        \
{                                                                   \
    int i;                                                          \
    acctype s1 = 0;                                                 \
    accsqtype sq1 = 0;                                              \
                                                                    \
    for( i = 0; i <= (len) - 4; i += 4 )                            \
    {                                                               \
        int m = ((mask)[i] == 0) - 1;                               \
        temptype t;                                                 \
        acctype s;                                                  \
        accsqtype sq;                                               \
                                                                    \
        t = _mask_op_(m, (src)[i]);                                 \
        (pix) -= m;                                                 \
        s = t;                                                      \
        sq = ((accsqtype)t)*t;                                      \
                                                                    \
        m = ((mask)[i + 1] == 0) - 1;                               \
        t = _mask_op_(m, (src)[i + 1]);                             \
        (pix) -= m;                                                 \
        s += t;                                                     \
        sq += ((accsqtype)t)*t;                                     \
                                                                    \
        m = ((mask)[i + 2] == 0) - 1;                               \
        t = _mask_op_(m, (src)[i + 2]);                             \
        (pix) -= m;                                                 \
        s += t;                                                     \
        sq += ((accsqtype)t)*t;                                     \
                                                                    \
        m = ((mask)[i + 3] == 0) - 1;                               \
        t = _mask_op_(m, (src)[i + 3]);                             \
        (pix) -= m;                                                 \
        sum[0] += s + t;                                            \
        sqsum[0] += sq + ((accsqtype)t)*t;                          \
    }                                                               \
                                                                    \
    for( ; i < (len); i++ )                                         \
    {                                                               \
        int m = ((mask)[i] == 0) - 1;                               \
        temptype t = _mask_op_(m, (src)[i]);                        \
        (pix) -= m;                                                 \
        s1 += t;                                                    \
        sq1 += ((accsqtype)t)*t;                                    \
    }                                                               \
                                                                    \
    sum[0] += s1;                                                   \
    sqsum[0] += sq1;                                                \
}


#define CV_IMPL_MEAN_SDV_MASK_1D_CASE_COI( _mask_op_, temptype, acctype, accsqtype, \
                                           src, mask, len, sum, sqsum, pix, cn )    \
{                                                                   \
    int i;                                                          \
    acctype s1 = 0;                                                 \
    accsqtype sq1 = 0;                                              \
                                                                    \
    for( i = 0; i <= (len) - 4; i += 4 )                            \
    {                                                               \
        int m = ((mask)[i] == 0) - 1;                               \
        temptype t;                                                 \
        acctype s;                                                  \
        accsqtype sq;                                               \
                                                                    \
        t = _mask_op_(m, (src)[i*(cn)]);                            \
        (pix) -= m;                                                 \
        s = t;                                                      \
        sq = ((accsqtype)t)*t;                                      \
                                                                    \
        m = ((mask)[i + 1] == 0) - 1;                               \
        t = _mask_op_(m, (src)[(i + 1)*(cn)]);                      \
        (pix) -= m;                                                 \
        s += t;                                                     \
        sq += ((accsqtype)t)*t;                                     \
                                                                    \
        m = ((mask)[i + 2] == 0) - 1;                               \
        t = _mask_op_(m, (src)[(i + 2)*(cn)]);                      \
        (pix) -= m;                                                 \
        s += t;                                                     \
        sq += ((accsqtype)t)*t;                                     \
                                                                    \
        m = ((mask)[i + 3] == 0) - 1;                               \
        t = _mask_op_(m, (src)[(i + 3)*(cn)]);                      \
        (pix) -= m;                                                 \
        sum[0] += s + t;                                            \
        sqsum[0] += sq + ((accsqtype)t)*t;                          \
    }                                                               \
                                                                    \
    for( ; i < (len); i++ )                                         \
    {                                                               \
        int m = ((mask)[i] == 0) - 1;                               \
        temptype t = _mask_op_(m, (src)[i*(cn)]);                   \
        (pix) -= m;                                                 \
        s1 += t;                                                    \
        sq1 += ((accsqtype)t)*t;                                    \
    }                                                               \
                                                                    \
    sum[0] += s1;                                                   \
    sqsum[0] += sq1;                                                \
}


#define CV_IMPL_MEAN_SDV_MASK_1D_CASE_C2( _mask_op_, temptype, acctype, accsqtype, \
                                          src, mask, len, sum, sqsum, pix )        \
{                                                                   \
    int i;                                                          \
                                                                    \
    for( i = 0; i < (len); i++ )                                    \
    {                                                               \
        int m = ((mask)[i] == 0) - 1;                               \
        temptype t;                                                 \
                                                                    \
        (pix) -= m;                                                 \
        t = _mask_op_(m, (src)[i*2]);                               \
        (sum)[0] += t;                                              \
        (sqsum)[0] += ((accsqtype)t)*t;                             \
                                                                    \
        t = _mask_op_(m, (src)[i*2 + 1]);                           \
        (sum)[1] += t;                                              \
        (sqsum)[1] += ((accsqtype)t)*t;                             \
    }                                                               \
}


#define CV_IMPL_MEAN_SDV_MASK_1D_CASE_C3( _mask_op_, temptype, acctype, accsqtype, \
                                          src, mask, len, sum, sqsum, pix )        \
{                                                                   \
    int i;                                                          \
                                                                    \
    for( i = 0; i < (len); i++ )                                    \
    {                                                               \
        int m = ((mask)[i] == 0) - 1;                               \
        temptype t;                                                 \
                                                                    \
        (pix) -= m;                                                 \
        t = _mask_op_(m, (src)[i*3]);                               \
        (sum)[0] += t;                                              \
        (sqsum)[0] += ((accsqtype)t)*t;                             \
                                                                    \
        t = _mask_op_(m, (src)[i*3 + 1]);                           \
        (sum)[1] += t;                                              \
        (sqsum)[1] += ((accsqtype)t)*t;                             \
                                                                    \
        t = _mask_op_(m, (src)[i*3 + 2]);                           \
        (sum)[2] += t;                                              \
        (sqsum)[2] += ((accsqtype)t)*t;                             \
    }                                                               \
}


#define CV_IMPL_MEAN_SDV_MASK_1D_CASE_C4( _mask_op_, temptype, acctype, accsqtype, \
                                          src, mask, len, sum, sqsum, pix )        \
{                                                                   \
    int i;                                                          \
                                                                    \
    for( i = 0; i < (len); i++ )                                    \
    {                                                               \
        int m = ((mask)[i] == 0) - 1;                               \
        temptype t;                                                 \
                                                                    \
        (pix) -= m;                                                 \
        t = _mask_op_(m, (src)[i*4]);                               \
        (sum)[0] += t;                                              \
        (sqsum)[0] += ((accsqtype)t)*t;                             \
                                                                    \
        t = _mask_op_(m, (src)[i*4 + 1]);                           \
        (sum)[1] += t;                                              \
        (sqsum)[1] += ((accsqtype)t)*t;                             \
                                                                    \
        t = _mask_op_(m, (src)[i*4 + 2]);                           \
        (sum)[2] += t;                                              \
        (sqsum)[2] += ((accsqtype)t)*t;                             \
                                                                    \
        t = _mask_op_(m, (src)[i*4 + 3]);                           \
        (sum)[3] += t;                                              \
        (sqsum)[3] += ((accsqtype)t)*t;                             \
    }                                                               \
}



#define CV_MEAN_SDV_ENTRY( sumtype, sumsqtype ) \
    sumtype sum[4] = {0,0,0,0};                 \
    sumsqtype sqsum[4] = {0,0,0,0}


#define CV_MEAN_SDV_MASK_ENTRY( sumtype, sumsqtype )\
    sumtype sum[4] = {0,0,0,0};                     \
    sumtype sqsum[4] = {0,0,0,0};                   \
    int pix = 0


#define CV_MEAN_SDV_MASK_ENTRY_FLT( sumtype, sumsqtype )\
    float  maskTab[] = { 1.f, 0.f };                    \
    sumtype sum[4] = {0,0,0,0};                         \
    sumtype sqsum[4] = {0,0,0,0};                       \
    int pix = 0


#define CV_MEAN_SDV_EXIT( pix, cn )             \
{                                               \
    double scale = pix ? 1./pix : 0;            \
    for( int k = 0; k < cn; k++ )               \
    {                                           \
        double mn = sum[k]*scale;               \
        mean[k] = mn;                           \
        mn = sqsum[k]*scale - mn*mn;            \
        sdv[k] = sqrt( MAX( mn, 0 ) );          \
    }                                           \
}


#define CV_IMPL_MEAN_SDV_FUNC_2D( _entry_, _exit_,                  \
                            flavor, cn, srctype, sumtype, sumsqtype,\
                            temptype, acctype, accsqtype )          \
IPCVAPI_IMPL( CvStatus, icvMean_StdDev_##flavor##_C##cn##R,         \
                        ( const srctype* src, int step,             \
                          CvSize size, double* mean, double* sdv ), \
                          (src, step, size, mean, sdv) )            \
{                                                                   \
    _entry_( sumtype, sumsqtype );                                  \
    int len = size.width*(cn), height = size.height;                \
                                                                    \
    for( ; size.height--; (char*&)src += step )                     \
    {                                                               \
        CV_IMPL_MEAN_SDV_1D_CASE_C##cn( temptype, acctype, accsqtype,\
                                        src, len, sum, sqsum );     \
    }                                                               \
                                                                    \
    len = size.width*height;                                        \
    _exit_( len, cn );                                              \
                                                                    \
    return CV_OK;                                                   \
}


#define CV_IMPL_MEAN_SDV_FUNC_2D_COI( _entry_, _exit_,              \
                              flavor, srctype, sumtype, sumsqtype,  \
                              temptype, acctype, accsqtype )        \
static CvStatus CV_STDCALL icvMean_StdDev_##flavor##_CnCR           \
                        ( const srctype* src, int step,             \
                          CvSize size, int cn, int coi,             \
                          double* mean, double* sdv )               \
{                                                                   \
    _entry_( sumtype, sumsqtype );                                  \
    int len = size.width*(cn), height = size.height;                \
    (src) += coi - 1;                                               \
                                                                    \
    for( ; size.height--; (char*&)src += step )                     \
    {                                                               \
        CV_IMPL_MEAN_SDV_1D_CASE_COI( temptype, acctype, accsqtype, \
                                      src, len, sum, sqsum, cn );   \
    }                                                               \
                                                                    \
    len = size.width*height;                                        \
    _exit_( len, 1 );                                               \
                                                                    \
    return CV_OK;                                                   \
}


#define CV_IMPL_MEAN_SDV_MASK_FUNC_2D( _mask_op_, _entry_, _exit_,  \
                            flavor, cn, srctype, sumtype, sumsqtype,\
                            temptype, acctype, accsqtype )          \
IPCVAPI_IMPL( CvStatus, icvMean_StdDev_##flavor##_C##cn##MR,        \
                        ( const srctype* src, int step,             \
                          const uchar* mask, int maskStep,          \
                          CvSize size, double* mean, double* sdv ), \
                         (src, step, mask, maskStep, size, mean, sdv))\
{                                                                   \
    _entry_( sumtype, sumsqtype );                                  \
                                                                    \
    for( ; size.height--;                                           \
         (char*&)src += step, (char*&)mask += maskStep )            \
    {                                                               \
        CV_IMPL_MEAN_SDV_MASK_1D_CASE_C##cn( _mask_op_, temptype,   \
            acctype, accsqtype, src, mask, size.width, sum, sqsum, pix);\
    }                                                               \
                                                                    \
    _exit_( pix, cn );                                              \
                                                                    \
    return CV_OK;                                                   \
}


#define CV_IMPL_MEAN_SDV_MASK_FUNC_2D_COI( _mask_op_, _entry_, _exit_,  \
                              flavor, srctype, sumtype, sumsqtype,  \
                              temptype, acctype, accsqtype )        \
static CvStatus CV_STDCALL icvMean_StdDev_##flavor##_CnCMR          \
                        ( const srctype* src, int step,             \
                          const uchar* mask, int maskStep,          \
                          CvSize size, int cn, int coi,             \
                          double* mean, double* sdv )               \
{                                                                   \
    _entry_( sumtype, sumsqtype );                                  \
    (src) += coi - 1;                                               \
                                                                    \
    for( ; size.height--;                                           \
         (char*&)src += step, (char*&)mask += maskStep )            \
    {                                                               \
        CV_IMPL_MEAN_SDV_MASK_1D_CASE_COI( _mask_op_, temptype,     \
            acctype, accsqtype, src, mask, size.width, sum, sqsum, pix, cn);\
    }                                                               \
                                                                    \
    _exit_( pix, 1 );                                               \
                                                                    \
    return CV_OK;                                                   \
}


#define CV_IMPL_MEAN_SDV_ALL( flavor, srctype, sumtype, sumsqtype, temptype,    \
                              acctype, accsqtype )                              \
                                                                                \
    CV_IMPL_MEAN_SDV_FUNC_2D( CV_MEAN_SDV_ENTRY, CV_MEAN_SDV_EXIT,              \
                              flavor, 1, srctype, sumtype,                      \
                              sumsqtype, temptype, acctype, accsqtype )         \
                                                                                \
    CV_IMPL_MEAN_SDV_FUNC_2D( CV_MEAN_SDV_ENTRY, CV_MEAN_SDV_EXIT,              \
                              flavor, 2, srctype, sumtype,                      \
                              sumsqtype, temptype, acctype, accsqtype )         \
                                                                                \
    CV_IMPL_MEAN_SDV_FUNC_2D( CV_MEAN_SDV_ENTRY, CV_MEAN_SDV_EXIT,              \
                              flavor, 3, srctype, sumtype,                      \
                              sumsqtype, temptype, acctype, accsqtype )         \
                                                                                \
    CV_IMPL_MEAN_SDV_FUNC_2D( CV_MEAN_SDV_ENTRY, CV_MEAN_SDV_EXIT,              \
                              flavor, 4, srctype, sumtype,                      \
                              sumsqtype, temptype, acctype, accsqtype )         \
                                                                                \
    CV_IMPL_MEAN_SDV_FUNC_2D_COI( CV_MEAN_SDV_ENTRY, CV_MEAN_SDV_EXIT,          \
                                  flavor, srctype, sumtype,                     \
                                  sumsqtype, temptype, acctype, accsqtype )


#define CV_IMPL_MEAN_SDV_MASK_ALL( flavor, srctype, sumtype, sumsqtype,                 \
                                   temptype, acctype, accsqtype )                       \
                                                                                        \
    CV_IMPL_MEAN_SDV_MASK_FUNC_2D( CV_AND, CV_MEAN_SDV_MASK_ENTRY, CV_MEAN_SDV_EXIT,    \
                                   flavor, 1, srctype, sumtype, sumsqtype,              \
                                   temptype, acctype, accsqtype )                       \
                                                                                        \
    CV_IMPL_MEAN_SDV_MASK_FUNC_2D( CV_AND, CV_MEAN_SDV_MASK_ENTRY, CV_MEAN_SDV_EXIT,    \
                                   flavor, 2, srctype, sumtype, sumsqtype,              \
                                   temptype, acctype, accsqtype )                       \
                                                                                        \
    CV_IMPL_MEAN_SDV_MASK_FUNC_2D( CV_AND, CV_MEAN_SDV_MASK_ENTRY, CV_MEAN_SDV_EXIT,    \
                                   flavor, 3, srctype, sumtype, sumsqtype,              \
                                   temptype, acctype, accsqtype )                       \
                                                                                        \
    CV_IMPL_MEAN_SDV_MASK_FUNC_2D( CV_AND, CV_MEAN_SDV_MASK_ENTRY, CV_MEAN_SDV_EXIT,    \
                                   flavor, 4, srctype, sumtype, sumsqtype,              \
                                   temptype, acctype, accsqtype )                       \
                                                                                        \
    CV_IMPL_MEAN_SDV_MASK_FUNC_2D_COI( CV_AND, CV_MEAN_SDV_MASK_ENTRY, CV_MEAN_SDV_EXIT,\
                                       flavor, srctype, sumtype, sumsqtype,             \
                                       temptype, acctype, accsqtype )


#define CV_IMPL_MEAN_SDV_MASK_ALL_FLT( flavor, srctype, sumtype, sumsqtype,             \
                                       temptype, acctype, accsqtype )                   \
                                                                                        \
    CV_IMPL_MEAN_SDV_MASK_FUNC_2D( CV_MULMASK1, CV_MEAN_SDV_MASK_ENTRY_FLT,             \
                                   CV_MEAN_SDV_EXIT, flavor, 1,                         \
                                   srctype, sumtype, sumsqtype,                         \
                                   temptype, acctype, accsqtype )                       \
                                                                                        \
    CV_IMPL_MEAN_SDV_MASK_FUNC_2D( CV_MULMASK1, CV_MEAN_SDV_MASK_ENTRY_FLT,             \
                                   CV_MEAN_SDV_EXIT, flavor, 2,                         \
                                   srctype, sumtype, sumsqtype,                         \
                                   temptype, acctype, accsqtype )                       \
                                                                                        \
    CV_IMPL_MEAN_SDV_MASK_FUNC_2D( CV_MULMASK1, CV_MEAN_SDV_MASK_ENTRY_FLT,             \
                                   CV_MEAN_SDV_EXIT, flavor, 3,                         \
                                   srctype, sumtype, sumsqtype,                         \
                                   temptype, acctype, accsqtype )                       \
                                                                                        \
    CV_IMPL_MEAN_SDV_MASK_FUNC_2D( CV_MULMASK1, CV_MEAN_SDV_MASK_ENTRY_FLT,             \
                                   CV_MEAN_SDV_EXIT, flavor, 4,                         \
                                   srctype, sumtype, sumsqtype,                         \
                                   temptype, acctype, accsqtype )                       \
                                                                                        \
    CV_IMPL_MEAN_SDV_MASK_FUNC_2D_COI( CV_MULMASK1, CV_MEAN_SDV_MASK_ENTRY_FLT,         \
                                       CV_MEAN_SDV_EXIT, flavor,                        \
                                       srctype, sumtype, sumsqtype,                     \
                                       temptype, acctype, accsqtype )


CV_IMPL_MEAN_SDV_ALL( 8u, uchar, int64, int64, int, int, int )
CV_IMPL_MEAN_SDV_ALL( 16u, ushort, double, double, int, int, double )
CV_IMPL_MEAN_SDV_ALL( 16s, short, double, double, int, int, double )
CV_IMPL_MEAN_SDV_ALL( 32s, int, double, double, double, double, double )
CV_IMPL_MEAN_SDV_ALL( 32f, float, double, double, double, double, double )
CV_IMPL_MEAN_SDV_ALL( 64f, double, double, double, double, double, double )

CV_IMPL_MEAN_SDV_MASK_ALL( 8u, uchar, int64, int64, int, int, int )
CV_IMPL_MEAN_SDV_MASK_ALL( 16u, ushort, double, double, int, int, double )
CV_IMPL_MEAN_SDV_MASK_ALL( 16s, short, double, double, int, int, double )
CV_IMPL_MEAN_SDV_MASK_ALL_FLT( 32s, int, double, double, double, double, double )
CV_IMPL_MEAN_SDV_MASK_ALL_FLT( 32f, float, double, double, double, double, double )
CV_IMPL_MEAN_SDV_MASK_ALL_FLT( 64f, double, double, double, double, double, double )

#define icvMean_StdDev_8s_C1R  0
#define icvMean_StdDev_8s_C2R  0
#define icvMean_StdDev_8s_C3R  0
#define icvMean_StdDev_8s_C4R  0
#define icvMean_StdDev_8s_CnCR 0

#define icvMean_StdDev_8s_C1MR  0
#define icvMean_StdDev_8s_C2MR  0
#define icvMean_StdDev_8s_C3MR  0
#define icvMean_StdDev_8s_C4MR  0
#define icvMean_StdDev_8s_CnCMR 0

CV_DEF_INIT_BIG_FUNC_TAB_2D( Mean_StdDev, R )
CV_DEF_INIT_FUNC_TAB_2D( Mean_StdDev, CnCR )
CV_DEF_INIT_BIG_FUNC_TAB_2D( Mean_StdDev, MR )
CV_DEF_INIT_FUNC_TAB_2D( Mean_StdDev, CnCMR )

CV_IMPL  void
cvAvgSdv( const void* img, CvScalar* _mean, CvScalar* _sdv, const void* mask )
{
    CvScalar mean = {0,0,0,0};
    CvScalar sdv = {0,0,0,0};

    static CvBigFuncTable meansdv_tab;
    static CvFuncTable meansdvcoi_tab;
    static CvBigFuncTable meansdvmask_tab;
    static CvFuncTable meansdvmaskcoi_tab;
    static int inittab = 0;

    CV_FUNCNAME("cvMean_StdDev");

    __BEGIN__;

    int type, coi = 0;
    int mat_step, mask_step = 0;
    CvSize size;
    CvMat stub, maskstub, *mat = (CvMat*)img, *matmask = (CvMat*)mask;

    if( !inittab )
    {
        icvInitMean_StdDevRTable( &meansdv_tab );
        icvInitMean_StdDevCnCRTable( &meansdvcoi_tab );
        icvInitMean_StdDevMRTable( &meansdvmask_tab );
        icvInitMean_StdDevCnCMRTable( &meansdvmaskcoi_tab );
        inittab = 1;
    }

    CV_CALL( mat = cvGetMat( mat, &stub, &coi ));

    type = CV_MAT_TYPE( mat->type );
    size = cvGetMatSize( mat );

    mat_step = mat->step;

    if( !mask )
    {
        if( CV_IS_MAT_CONT( mat->type ))
        {
            size.width *= size.height;
            size.height = 1;
            mat_step = CV_STUB_STEP;
        }

        if( CV_MAT_CN(type) == 1 || coi == 0 )
        {
            CvFunc2D_1A2P func = (CvFunc2D_1A2P)(meansdv_tab.fn_2d[type]);

            if( !func )
                CV_ERROR( CV_StsBadArg, cvUnsupportedFormat );

            IPPI_CALL( func( mat->data.ptr, mat_step, size, mean.val, sdv.val ));
        }
        else
        {
            CvFunc2DnC_1A2P func = (CvFunc2DnC_1A2P)
                (meansdvcoi_tab.fn_2d[CV_MAT_DEPTH(type)]);

            if( !func )
                CV_ERROR( CV_StsBadArg, cvUnsupportedFormat );

            IPPI_CALL( func( mat->data.ptr, mat_step, size,
                             CV_MAT_CN(type), coi, mean.val, sdv.val ));
        }
    }
    else
    {
        CV_CALL( matmask = cvGetMat( matmask, &maskstub ));

        mask_step = matmask->step;

        if( !CV_IS_MASK_ARR( matmask ))
            CV_ERROR( CV_StsBadMask, "" );

        if( !CV_ARE_SIZES_EQ( mat, matmask ))
            CV_ERROR( CV_StsUnmatchedSizes, "" );

        if( CV_IS_MAT_CONT( mat->type & matmask->type ))
        {
            size.width *= size.height;
            size.height = 1;
            mat_step = mask_step = CV_STUB_STEP;
        }

        if( CV_MAT_CN(type) == 1 || coi == 0 )
        {
            CvFunc2D_2A2P func = (CvFunc2D_2A2P)(meansdvmask_tab.fn_2d[type]);

            if( !func )
                CV_ERROR( CV_StsBadArg, cvUnsupportedFormat );

            IPPI_CALL( func( mat->data.ptr, mat_step, matmask->data.ptr,
                             mask_step, size, mean.val, sdv.val ));
        }
        else
        {
            CvFunc2DnC_2A2P func = (CvFunc2DnC_2A2P)
                (meansdvmaskcoi_tab.fn_2d[CV_MAT_DEPTH(type)]);

            if( !func )
                CV_ERROR( CV_StsBadArg, cvUnsupportedFormat );

            IPPI_CALL( func( mat->data.ptr, mat_step,
                             matmask->data.ptr, mask_step,
                             size, CV_MAT_CN(type), coi, mean.val, sdv.val ));
        }
    }

    __END__;

    if( _mean )
        *_mean = mean;

    if( _sdv )
        *_sdv = sdv;
}


/*  End of file  */
