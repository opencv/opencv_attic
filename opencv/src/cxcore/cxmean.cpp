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
#include <float.h>

/****************************************************************************************\
*                              Mean value over the region                                *
\****************************************************************************************/

#define ICV_IMPL_MEAN_1D_CASE_C1( _mask_op_, acctype, src, mask, len, sum, pix ) \
{                                                                   \
    int i;                                                          \
    acctype s1 = 0;                                                 \
                                                                    \
    for( i = 0; i <= (len) - 4; i += 4 )                            \
    {                                                               \
        int m = ((mask)[i] == 0) - 1;                               \
        acctype s;                                                  \
                                                                    \
        s = _mask_op_(m,(src)[i]);                                  \
        (pix) -= m;                                                 \
                                                                    \
        m = ((mask)[i + 1] == 0) - 1;                               \
        s += _mask_op_(m,(src)[i + 1]);                             \
        (pix) -= m;                                                 \
                                                                    \
        m = ((mask)[i + 2] == 0) - 1;                               \
        s += _mask_op_(m,(src)[i + 2]);                             \
        (pix) -= m;                                                 \
                                                                    \
        m = ((mask)[i + 3] == 0) - 1;                               \
        (sum)[0] += s + _mask_op_(m,(src)[i + 3]);                  \
        (pix) -= m;                                                 \
    }                                                               \
                                                                    \
    for( ; i < (len); i++ )                                         \
    {                                                               \
        int m = ((mask)[i] == 0) - 1;                               \
                                                                    \
        s1 += _mask_op_(m,(src)[i]);                                \
        (pix) -= m;                                                 \
    }                                                               \
                                                                    \
    (sum)[0] += s1;                                                 \
}


#define ICV_IMPL_MEAN_1D_CASE_COI( _mask_op_, acctype, src, mask, len, sum, pix, cn ) \
{                                                                   \
    int i;                                                          \
    acctype s1 = 0;                                                 \
                                                                    \
    for( i = 0; i <= (len) - 4; i += 4 )                            \
    {                                                               \
        int m = ((mask)[i] == 0) - 1;                               \
        acctype s;                                                  \
                                                                    \
        s = _mask_op_( m, (src)[i*(cn)]);                           \
        (pix) -= m;                                                 \
                                                                    \
        m = ((mask)[i + 1] == 0) - 1;                               \
        s += _mask_op_( m, (src)[(i + 1)*(cn)]);                    \
        (pix) -= m;                                                 \
                                                                    \
        m = ((mask)[i + 2] == 0) - 1;                               \
        s += _mask_op_( m, (src)[(i + 2)*(cn)]);                    \
        (pix) -= m;                                                 \
                                                                    \
        m = ((mask)[i + 3] == 0) - 1;                               \
        (sum)[0] += s + _mask_op_( m, (src)[(i + 3)*(cn)]);         \
        (pix) -= m;                                                 \
    }                                                               \
                                                                    \
    for( ; i < (len); i++ )                                         \
    {                                                               \
        int m = ((mask)[i] == 0) - 1;                               \
                                                                    \
        s1 += _mask_op_( m, (src)[i*(cn)]);                         \
        (pix) -= m;                                                 \
    }                                                               \
                                                                    \
    (sum)[0] += s1;                                                 \
}


#define ICV_IMPL_MEAN_1D_CASE_C2( _mask_op_, acctype, src, mask, len, sum, pix ) \
{                                                                   \
    int i;                                                          \
                                                                    \
    for( i = 0; i <= (len) - 2; i += 2 )                            \
    {                                                               \
        int m = ((mask)[i] == 0) - 1;                               \
                                                                    \
        (sum)[0] += _mask_op_( m, (src)[i*2]);                      \
        (sum)[1] += _mask_op_( m, (src)[i*2 + 1]);                  \
        (pix) -= m;                                                 \
                                                                    \
        m = ((mask)[i + 1] == 0) - 1;                               \
        (sum)[0] += _mask_op_( m, (src)[i*2 + 2]);                  \
        (sum)[1] += _mask_op_( m, (src)[i*2 + 3]);                  \
        (pix) -= m;                                                 \
    }                                                               \
                                                                    \
    for( ; i < (len); i++ )                                         \
    {                                                               \
        int m = ((mask)[i] == 0) - 1;                               \
                                                                    \
        (sum)[0] += _mask_op_( m, (src)[i*2]);                      \
        (sum)[1] += _mask_op_( m, (src)[i*2 + 1]);                  \
        (pix) -= m;                                                 \
    }                                                               \
}


#define ICV_IMPL_MEAN_1D_CASE_C3( _mask_op_, acctype, src, mask, len, sum, pix ) \
{                                                                   \
    int i;                                                          \
                                                                    \
    for( i = 0; i < (len); i++ )                                    \
    {                                                               \
        int m = ((mask)[i] == 0) - 1;                               \
                                                                    \
        (sum)[0] += _mask_op_( m, (src)[i*3]);                      \
        (sum)[1] += _mask_op_( m, (src)[i*3 + 1]);                  \
        (sum)[2] += _mask_op_( m, (src)[i*3 + 2]);                  \
        (pix) -= m;                                                 \
    }                                                               \
}


#define ICV_IMPL_MEAN_1D_CASE_C4( _mask_op_, acctype, src, mask, len, sum, pix ) \
{                                                                   \
    int i;                                                          \
                                                                    \
    for( i = 0; i < (len); i++ )                                    \
    {                                                               \
        int m = ((mask)[i] == 0) - 1;                               \
                                                                    \
        (sum)[0] += _mask_op_( m, (src)[i*4]);                      \
        (sum)[1] += _mask_op_( m, (src)[i*4 + 1]);                  \
        (sum)[2] += _mask_op_( m, (src)[i*4 + 2]);                  \
        (sum)[3] += _mask_op_( m, (src)[i*4 + 3]);                  \
        (pix) -= m;                                                 \
    }                                                               \
}



#define ICV_MEAN_ENTRY( sumtype ) \
    sumtype sum[4] = {0,0,0,0};  \
    int pix = 0


#define ICV_MEAN_ENTRY_FLT( sumtype ) \
    float  maskTab[] = { 1.f, 0.f }; \
    sumtype sum[4] = {0,0,0,0};      \
    int pix = 0


#define ICV_MEAN_EXIT(cn)                  \
{                                         \
    double scale = pix ? 1./pix : 0;      \
    for( int k = 0; k < cn; k++ )         \
        mean[k] = sum[k]*scale;           \
}                                         \
return CV_OK;


#define ICV_IMPL_MEAN_FUNC_2D( _mask_op_, _entry_, _exit_,          \
                              flavor, cn, srctype, sumtype, acctype)\
IPCVAPI_IMPL( CvStatus, icvMean_##flavor##_C##cn##MR,               \
                          ( const srctype* src, int step,           \
                            const uchar* mask, int maskStep,        \
                            CvSize size, double* mean ),            \
                           (src, step, mask, maskStep, size, mean) )\
{                                                                   \
    _entry_( sumtype );                                             \
                                                                    \
    for( ; size.height--;                                           \
         (char*&)src += step, (char*&)mask += maskStep )            \
    {                                                               \
        ICV_IMPL_MEAN_1D_CASE_C##cn( _mask_op_, acctype, src, mask, \
                                    size.width, sum, pix );         \
    }                                                               \
                                                                    \
    _exit_(cn);                                                     \
}


#define ICV_IMPL_MEAN_FUNC_2D_COI( _mask_op_, _entry_, _exit_,      \
                                  flavor, srctype, sumtype, acctype)\
static CvStatus CV_STDCALL                                          \
icvMean_##flavor##_CnCMR( const srctype* src, int step,             \
                        const uchar* mask, int maskStep,            \
                        CvSize size, int cn, int coi, double* mean )\
{                                                                   \
    _entry_( sumtype );                                             \
    (src) += coi - 1;                                               \
                                                                    \
    for( ; size.height--;                                           \
         (char*&)src += step, (char*&)mask += maskStep )            \
    {                                                               \
        ICV_IMPL_MEAN_1D_CASE_COI( _mask_op_, acctype, src, mask,   \
                                  size.width, sum, pix, cn );       \
    }                                                               \
                                                                    \
    mean[0] = sum[0]*(pix ? 1./pix : 0);                            \
                                                                    \
    return CV_OK;                                                   \
}


#define ICV_IMPL_MEAN_ALL( flavor, srctype, sumtype, acctype )        \
    ICV_IMPL_MEAN_FUNC_2D( CV_AND, ICV_MEAN_ENTRY, ICV_MEAN_EXIT,     \
                           flavor, 1, srctype, sumtype, acctype )     \
    ICV_IMPL_MEAN_FUNC_2D( CV_AND, ICV_MEAN_ENTRY, ICV_MEAN_EXIT,     \
                           flavor, 2, srctype, sumtype, acctype )     \
    ICV_IMPL_MEAN_FUNC_2D( CV_AND, ICV_MEAN_ENTRY, ICV_MEAN_EXIT,     \
                           flavor, 3, srctype, sumtype, acctype )     \
    ICV_IMPL_MEAN_FUNC_2D( CV_AND, ICV_MEAN_ENTRY, ICV_MEAN_EXIT,     \
                           flavor, 4, srctype, sumtype, acctype )     \
    ICV_IMPL_MEAN_FUNC_2D_COI( CV_AND, ICV_MEAN_ENTRY, ICV_MEAN_EXIT, \
                               flavor, srctype, sumtype, acctype )


#define ICV_IMPL_MEAN_ALL_FLT( flavor, srctype, sumtype, acctype )              \
    ICV_IMPL_MEAN_FUNC_2D( CV_MULMASK1, ICV_MEAN_ENTRY_FLT, ICV_MEAN_EXIT,      \
                           flavor, 1, srctype, sumtype, acctype )               \
    ICV_IMPL_MEAN_FUNC_2D( CV_MULMASK1, ICV_MEAN_ENTRY_FLT, ICV_MEAN_EXIT,      \
                           flavor, 2, srctype, sumtype, acctype )               \
    ICV_IMPL_MEAN_FUNC_2D( CV_MULMASK1, ICV_MEAN_ENTRY_FLT, ICV_MEAN_EXIT,      \
                           flavor, 3, srctype, sumtype, acctype )               \
    ICV_IMPL_MEAN_FUNC_2D( CV_MULMASK1, ICV_MEAN_ENTRY_FLT, ICV_MEAN_EXIT,      \
                           flavor, 4, srctype, sumtype, acctype )               \
    ICV_IMPL_MEAN_FUNC_2D_COI( CV_MULMASK1, ICV_MEAN_ENTRY_FLT, ICV_MEAN_EXIT,  \
                               flavor, srctype, sumtype, acctype )

ICV_IMPL_MEAN_ALL( 8u, uchar, int64, int )
ICV_IMPL_MEAN_ALL( 16u, ushort, int64, int )
ICV_IMPL_MEAN_ALL( 16s, short, int64, int )
ICV_IMPL_MEAN_ALL( 32s, int, int64, int64 )
ICV_IMPL_MEAN_ALL_FLT( 32f, float, double, double )
ICV_IMPL_MEAN_ALL_FLT( 64f, double, double, double )

#define icvMean_8s_C1MR 0
#define icvMean_8s_C2MR 0
#define icvMean_8s_C3MR 0
#define icvMean_8s_C4MR 0
#define icvMean_8s_CnCMR 0

CV_DEF_INIT_BIG_FUNC_TAB_2D( Mean, MR )
CV_DEF_INIT_FUNC_TAB_2D( Mean, CnCMR )

CV_IMPL  CvScalar
cvAvg( const void* img, const void* maskarr )
{
    CvScalar mean = {0,0,0,0};

    static CvBigFuncTable mean_tab;
    static CvFuncTable meancoi_tab;
    static int inittab = 0;

    CV_FUNCNAME("cvAvg");

    __BEGIN__;

    CvSize size;
    double scale;

    if( !maskarr )
    {
        CV_CALL( mean = CvScalar(cvSum(img)));
        size = cvGetSize( img );
        size.width *= size.height;
        scale = size.width ? 1./size.width : 0;

        mean.val[0] *= scale;
        mean.val[1] *= scale;
        mean.val[2] *= scale;
        mean.val[3] *= scale;
    }
    else
    {
        int type, coi = 0;
        int mat_step, mask_step;

        CvMat stub, maskstub, *mat = (CvMat*)img, *mask = (CvMat*)maskarr;

        if( !inittab )
        {
            icvInitMeanMRTable( &mean_tab );
            icvInitMeanCnCMRTable( &meancoi_tab );
            inittab = 1;
        }

        if( !CV_IS_MAT(mat) )
            CV_CALL( mat = cvGetMat( mat, &stub, &coi ));

        if( !CV_IS_MAT(mask) )
            CV_CALL( mask = cvGetMat( mask, &maskstub ));

        if( !CV_IS_MASK_ARR(mask) )
            CV_ERROR( CV_StsBadMask, "" );

        if( !CV_ARE_SIZES_EQ( mat, mask ) )
            CV_ERROR( CV_StsUnmatchedSizes, "" );

        type = CV_MAT_TYPE( mat->type );
        size = cvGetMatSize( mat );

        mat_step = mat->step;
        mask_step = mask->step;

        if( CV_IS_MAT_CONT( mat->type & mask->type ))
        {
            size.width *= size.height;
            size.height = 1;
            mat_step = mask_step = CV_STUB_STEP;
        }

        if( CV_MAT_CN(type) == 1 || coi == 0 )
        {
            CvFunc2D_2A1P func = (CvFunc2D_2A1P)(mean_tab.fn_2d[type]);

            if( !func )
                CV_ERROR( CV_StsBadArg, cvUnsupportedFormat );

            IPPI_CALL( func( mat->data.ptr, mat_step, mask->data.ptr,
                             mask_step, size, mean.val ));
        }
        else
        {
            CvFunc2DnC_2A1P func = (CvFunc2DnC_2A1P)(
                meancoi_tab.fn_2d[CV_MAT_DEPTH(type)]);

            if( !func )
                CV_ERROR( CV_StsBadArg, cvUnsupportedFormat );

            IPPI_CALL( func( mat->data.ptr, mat_step, mask->data.ptr,
                             mask_step, size, CV_MAT_CN(type), coi, mean.val ));
        }
    }

    __END__;

    return  mean;
}

/*  End of file  */
