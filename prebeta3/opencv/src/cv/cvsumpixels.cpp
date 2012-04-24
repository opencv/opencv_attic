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

/****************************************************************************************\
*                             Find sum of pixels in the ROI                              *
\****************************************************************************************/


#define ICV_DEF_SUM_1D_CASE_COI( __op__, src, len, sum, cn )                \
{                                                                           \
    int i;                                                                  \
                                                                            \
    for( i = 0; i <= (len) - 4*(cn); i += 4*(cn) )                          \
        (sum)[0] += __op__((src)[i]) + __op__((src)[i+(cn)]) +              \
                    __op__((src)[i+(cn)*2]) + __op__((src)[i+(cn)*3]);      \
                                                                            \
    for( ; i < (len); i += (cn) )                                           \
        (sum)[0] += __op__((src)[i]);                                       \
}                                                                           \


#define ICV_DEF_SUM_1D_CASE_C1( __op__, src, len, sum ) \
    ICV_DEF_SUM_1D_CASE_COI( __op__, src, len, sum, 1 )


#define ICV_DEF_SUM_1D_CASE_C2( __op__, src, len, sum )                     \
{                                                                           \
    int i;                                                                  \
                                                                            \
    for( i = 0; i <= (len) - 8; i += 8 )                                    \
    {                                                                       \
        (sum)[0] += __op__((src)[i]) + __op__((src)[i+2]) +                 \
                    __op__((src)[i+4]) + __op__((src)[i+6]);                \
        (sum)[1] += __op__((src)[i+1]) + __op__((src)[i+3]) +               \
                    __op__((src)[i+5]) + __op__((src)[i+7]);                \
    }                                                                       \
                                                                            \
    for( ; i < (len); i += 2 )                                              \
    {                                                                       \
        (sum)[0] += __op__((src)[i]);                                       \
        (sum)[1] += __op__((src)[i+1]);                                     \
    }                                                                       \
}                                                                           \


#define ICV_DEF_SUM_1D_CASE_C3( __op__, src, len, sum )                     \
{                                                                           \
    int i;                                                                  \
                                                                            \
    for( i = 0; i <= (len) - 12; i += 12 )                                  \
    {                                                                       \
        (sum)[0] += __op__((src)[i]) + __op__((src)[i+3]) +                 \
                    __op__((src)[i+6]) + __op__((src)[i+9]);                \
        (sum)[1] += __op__((src)[i+1]) + __op__((src)[i+4]) +               \
                    __op__((src)[i+7]) + __op__((src)[i+10]);               \
        (sum)[2] += __op__((src)[i+2]) + __op__((src)[i+5]) +               \
                    __op__((src)[i+8]) + __op__((src)[i+11]);               \
    }                                                                       \
                                                                            \
    for( ; i < (len); i += 3 )                                              \
    {                                                                       \
        (sum)[0] += __op__((src)[i]);                                       \
        (sum)[1] += __op__((src)[i+1]);                                     \
        (sum)[2] += __op__((src)[i+2]);                                     \
    }                                                                       \
}


#define ICV_DEF_SUM_1D_CASE_C4( __op__, src, len, sum )                     \
{                                                                           \
    int i;                                                                  \
                                                                            \
    for( i = 0; i <= (len) - 16; i += 16 )                                  \
    {                                                                       \
        (sum)[0] += __op__((src)[i]) + __op__((src)[i+4]) +                 \
                    __op__((src)[i+8]) + __op__((src)[i+12]);               \
        (sum)[1] += __op__((src)[i+1]) + __op__((src)[i+5]) +               \
                    __op__((src)[i+9]) + __op__((src)[i+13]);               \
        (sum)[2] += __op__((src)[i+2]) + __op__((src)[i+6]) +               \
                    __op__((src)[i+10]) + __op__((src)[i+14]);              \
        (sum)[3] += __op__((src)[i+3]) + __op__((src)[i+7]) +               \
                    __op__((src)[i+11]) + __op__((src)[i+15]);              \
    }                                                                       \
                                                                            \
    for( ; i < (len); i += 4 )                                              \
    {                                                                       \
        (sum)[0] += __op__((src)[i]);                                       \
        (sum)[1] += __op__((src)[i+1]);                                     \
        (sum)[2] += __op__((src)[i+2]);                                     \
        (sum)[3] += __op__((src)[i+3]);                                     \
    }                                                                       \
}


#define CV_SUM_ENTRY_C1( sumtype ) \
    sumtype temp[1] = { 0 };

#define CV_SUM_ENTRY_C2( sumtype ) \
    sumtype temp[2] = { 0, 0 };

#define CV_SUM_ENTRY_C3( sumtype ) \
    sumtype temp[3] = { 0, 0, 0 };

#define CV_SUM_ENTRY_C4( sumtype ) \
    sumtype temp[4] = { 0, 0, 0, 0 };

#define CV_SUM_EXIT_C1(sumtype_final) \
    sum[0] = (sumtype_final)temp[0]

#define CV_SUM_EXIT_C2(sumtype_final) \
    CV_SUM_EXIT_C1(sumtype_final), sum[1] = (sumtype_final)temp[1]

#define CV_SUM_EXIT_C3(sumtype_final) \
    CV_SUM_EXIT_C2(sumtype_final), sum[2] = (sumtype_final)temp[2]

#define CV_SUM_EXIT_C4(sumtype_final) \
    CV_SUM_EXIT_C3(sumtype_final), sum[3] = (sumtype_final)temp[3]


#define ICV_DEF_SUM_FUNC_2D( __op__, _entry_, _exit_, name, flavor,         \
                             cn, srctype, sumtype, sumtype_final )          \
IPCVAPI_IMPL(CvStatus, icv##name##_##flavor##_C##cn##R,( const srctype* src,\
                                    int step, CvSize size, sumtype_final* sum ))\
{                                                                           \
    _entry_( sumtype );                                                     \
    size.width *= cn;                                                       \
                                                                            \
    for( int y = 0; y < size.height; y++, (char*&)src += step )             \
    {                                                                       \
        ICV_DEF_SUM_1D_CASE_C##cn( __op__, src, size.width, temp );         \
    }                                                                       \
                                                                            \
    _exit_(sumtype_final);                                                  \
                                                                            \
    return CV_OK;                                                           \
}


#define ICV_DEF_SUM_0D_CASE_C1( __op__, src, sum )                          \
{                                                                           \
    (sum)[0] += __op__((src)[0]);                                           \
}                                                                           \


#define ICV_DEF_SUM_0D_CASE_C2( __op__, src, sum )                          \
{                                                                           \
    (sum)[0] += __op__((src)[0]);                                           \
    (sum)[1] += __op__((src)[1]);                                           \
}                                                                           \


#define ICV_DEF_SUM_FUNC_2D_EX( __op__, _entry_, _exit_, name, flavor,      \
                                cn, srctype, sumtype, sumtype_final )       \
IPCVAPI_IMPL(CvStatus, icv##name##_##flavor##_C##cn##R,( const srctype* src,\
                                    int step, CvSize size, sumtype_final* sum )) \
{                                                                           \
    _entry_( sumtype );                                                     \
                                                                            \
    if( size.width == 1 )                                                   \
    {                                                                       \
        for( int y = 0; y < size.height; y++, (char*&)src += step )         \
        {                                                                   \
            ICV_DEF_SUM_0D_CASE_C##cn( __op__, src, temp );                 \
        }                                                                   \
    }                                                                       \
    else                                                                    \
    {                                                                       \
        size.width *= cn;                                                   \
                                                                            \
        for( int y = 0; y < size.height; y++, (char*&)src += step )         \
        {                                                                   \
            ICV_DEF_SUM_1D_CASE_C##cn( __op__, src, size.width, temp );     \
        }                                                                   \
    }                                                                       \
                                                                            \
    _exit_(sumtype_final);                                                  \
                                                                            \
    return CV_OK;                                                           \
}



#define ICV_DEF_SUM_FUNC_2D_COI( __op__, name, flavor, srctype,             \
                                 sumtype, sumtype_final )                   \
IPCVAPI_IMPL(CvStatus, icv##name##_##flavor##_CnCR, ( const srctype* src,   \
                    int step, CvSize size, int cn, int coi, sumtype_final* sum )) \
{                                                                           \
    CV_SUM_ENTRY_C1( sumtype );                                             \
                                                                            \
    size.width *= cn;                                                       \
    src += coi - 1;                                                         \
                                                                            \
    for( int y = 0; y < size.height; y++, (char*&)src += step )             \
    {                                                                       \
        ICV_DEF_SUM_1D_CASE_COI( __op__, src, size.width, temp, cn );       \
    }                                                                       \
                                                                            \
    CV_SUM_EXIT_C1(sumtype_final);                                          \
                                                                            \
    return CV_OK;                                                           \
}


#define ICV_DEF_SUM_ALL( __op__, name, flavor, srctype, sumtype, sumtype_final )\
    ICV_DEF_SUM_FUNC_2D( __op__, CV_SUM_ENTRY_C1, CV_SUM_EXIT_C1,               \
                         name, flavor, 1, srctype, sumtype, sumtype_final )     \
    ICV_DEF_SUM_FUNC_2D( __op__, CV_SUM_ENTRY_C2, CV_SUM_EXIT_C2,               \
                         name, flavor, 2, srctype, sumtype, sumtype_final )     \
    ICV_DEF_SUM_FUNC_2D( __op__, CV_SUM_ENTRY_C3, CV_SUM_EXIT_C3,               \
                         name, flavor, 3, srctype, sumtype, sumtype_final )     \
    ICV_DEF_SUM_FUNC_2D( __op__, CV_SUM_ENTRY_C4, CV_SUM_EXIT_C4,               \
                         name, flavor, 4, srctype, sumtype, sumtype_final )     \
    ICV_DEF_SUM_FUNC_2D_COI( __op__, name, flavor, srctype, sumtype, sumtype_final )


ICV_DEF_SUM_ALL( CV_NOP, Sum, 8u, uchar, int64, double )
ICV_DEF_SUM_ALL( CV_NOP, Sum, 8s, char, int64, double )
ICV_DEF_SUM_ALL( CV_NOP, Sum, 16s, short, int64, double )
ICV_DEF_SUM_ALL( CV_CAST_64S, Sum, 32s, int, int64, double )
ICV_DEF_SUM_ALL( CV_NOP, Sum, 32f, float, double, double )
ICV_DEF_SUM_ALL( CV_NOP, Sum, 64f, double, double, double )

CV_DEF_INIT_BIG_FUNC_TAB( Sum, R )
CV_DEF_INIT_FUNC_TAB_2D( Sum, CnCR )

CV_IMPL CvScalar
cvSum( const CvArr* arr )
{
    static CvBigFuncTable sum_tab;
    static CvFuncTable sumcoi_tab;
    static int inittab = 0;

    CvScalar sum = {{ 0, 0, 0, 0 }};

    CV_FUNCNAME("cvSum");

    __BEGIN__;

    int type, coi = 0;
    int mat_step;
    CvSize size;
    CvMat stub, *mat = (CvMat*)arr;

    if( !inittab )
    {
        icvInitSumRTable( &sum_tab );
        icvInitSumCnCRTable( &sumcoi_tab );
        inittab = 1;
    }

    if( !CV_IS_MAT(mat) )
    {
        if( CV_IS_MATND(mat) )
        {
            CvMatND stub;
            CvMatNDIterator iterator;
            CvFunc2D_1A1P func;

            CV_CALL( icvPrepareArrayOp( 1, (void**)&mat, 0, &stub, &iterator ));

            type = CV_MAT_TYPE(iterator.hdr[0]->type);
            func = (CvFunc2D_1A1P)(sum_tab.fn_2d[type]);
            if( !func )
                CV_ERROR( CV_StsUnsupportedFormat, "" );
       
            do
            {
                CvScalar temp = {{ 0, 0, 0, 0 }};
                IPPI_CALL( func( iterator.ptr[0], CV_STUB_STEP,
                                 iterator.size, temp.val ));
                sum.val[0] += temp.val[0];
                sum.val[1] += temp.val[1];
                sum.val[2] += temp.val[2];
                sum.val[3] += temp.val[3];
            }
            while( icvNextMatNDSlice( &iterator ));
            EXIT;
        }
        else
            CV_CALL( mat = cvGetMat( mat, &stub, &coi ));
    }

    type = CV_MAT_TYPE(mat->type);
    size = icvGetMatSize( mat );

    mat_step = mat->step;

    if( CV_IS_MAT_CONT( mat->type ))
    {
        size.width *= size.height;
        
        if( size.width <= CV_MAX_INLINE_MAT_OP_SIZE )
        {
            if( type == CV_32FC1 )
            {
                float* data = mat->data.fl;

                do
                {
                    sum.val[0] += data[size.width - 1];
                }
                while( --size.width );

                EXIT;
            }

            if( type == CV_64FC1 )
            {
                double* data = mat->data.db;

                do
                {
                    sum.val[0] += data[size.width - 1];
                }
                while( --size.width );

                EXIT;
            }
        }
        size.height = 1;
        mat_step = CV_STUB_STEP;
    }

    if( CV_MAT_CN(type) == 1 || coi == 0 )
    {
        CvFunc2D_1A1P func = (CvFunc2D_1A1P)(sum_tab.fn_2d[type]);

        if( !func )
            CV_ERROR( CV_StsBadArg, icvUnsupportedFormat );

        IPPI_CALL( func( mat->data.ptr, mat_step, size, sum.val ));
    }
    else
    {
        CvFunc2DnC_1A1P func = (CvFunc2DnC_1A1P)(sumcoi_tab.fn_2d[CV_MAT_DEPTH(type)]);

        if( !func )
            CV_ERROR( CV_StsBadArg, icvUnsupportedFormat );

        IPPI_CALL( func( mat->data.ptr, mat_step, size,
                         CV_MAT_CN(type), coi, sum.val ));
    }

    __END__;

    return  sum;
}


#define ICV_DEF_SUM_C1( __op__, name, flavor, srctype, sumtype )        \
    ICV_DEF_SUM_FUNC_2D( __op__, CV_SUM_ENTRY_C1, CV_SUM_EXIT_C1,       \
                         name, flavor, 1, srctype, sumtype, int )       \
    ICV_DEF_SUM_FUNC_2D_COI( __op__, name, flavor, srctype, sumtype, int )

ICV_DEF_SUM_C1( CV_NONZERO, CountNonZero, 8u, uchar, int )
ICV_DEF_SUM_C1( CV_NONZERO, CountNonZero, 16s, ushort, int )
ICV_DEF_SUM_C1( CV_NONZERO, CountNonZero, 32s, int, int )
ICV_DEF_SUM_C1( CV_NONZERO_FLT, CountNonZero, 32f, int, int )
ICV_DEF_SUM_C1( CV_NONZERO_FLT, CountNonZero, 64f, int64, int )

#define icvCountNonZero_8s_C1R icvCountNonZero_8u_C1R
#define icvCountNonZero_8s_CnCR icvCountNonZero_8u_CnCR

CV_DEF_INIT_FUNC_TAB_2D( CountNonZero, C1R )
CV_DEF_INIT_FUNC_TAB_2D( CountNonZero, CnCR )


CV_IMPL int
cvCountNonZero( const CvArr* img )
{
    static CvFuncTable nz_tab;
    static CvFuncTable nzcoi_tab;
    static int inittab = 0;

    int count = 0;

    CV_FUNCNAME("cvCountNonZero");

    __BEGIN__;

    int type, coi = 0;
    int mat_step;
    CvSize size;
    CvMat stub, *mat = (CvMat*)img;

    if( !inittab )
    {
        icvInitCountNonZeroC1RTable( &nz_tab );
        icvInitCountNonZeroCnCRTable( &nzcoi_tab );
        inittab = 1;
    }

    if( !CV_IS_MAT(mat) )
    {
        if( CV_IS_MATND(mat) )
        {
            CvMatND stub;
            CvMatNDIterator iterator;
            CvFunc2D_1A1P func;

            CV_CALL( icvPrepareArrayOp( 1, (void**)&mat, 0, &stub, &iterator ));

            type = CV_MAT_TYPE(iterator.hdr[0]->type);

            if( CV_MAT_CN(type) != 1 )
                CV_ERROR( CV_BadNumChannels,
                    "Only single-channel array are supported here" );

            func = (CvFunc2D_1A1P)(nz_tab.fn_2d[CV_MAT_DEPTH(type)]);
            if( !func )
                CV_ERROR( CV_StsUnsupportedFormat, "" );
       
            do
            {
                int temp;
                IPPI_CALL( func( iterator.ptr[0], CV_STUB_STEP,
                                 iterator.size, &temp ));
                count += temp;
            }
            while( icvNextMatNDSlice( &iterator ));
            EXIT;
        }
        else
            CV_CALL( mat = cvGetMat( mat, &stub, &coi ));
    }

    type = CV_MAT_TYPE(mat->type);
    size = icvGetMatSize( mat );

    mat_step = mat->step;

    if( CV_IS_MAT_CONT( mat->type ))
    {
        size.width *= size.height;
        size.height = 1;
        mat_step = CV_STUB_STEP;
    }

    if( CV_MAT_CN(type) == 1 || coi == 0 )
    {
        CvFunc2D_1A1P func = (CvFunc2D_1A1P)(nz_tab.fn_2d[CV_MAT_DEPTH(type)]);

        if( CV_MAT_CN(type) != 1 )
            CV_ERROR( CV_BadNumChannels,
            "The function can handle only a single channel at a time (use COI)");

        if( !func )
            CV_ERROR( CV_StsBadArg, icvUnsupportedFormat );

        IPPI_CALL( func( mat->data.ptr, mat_step, size, &count ));
    }
    else
    {
        CvFunc2DnC_1A1P func = (CvFunc2DnC_1A1P)(nzcoi_tab.fn_2d[CV_MAT_DEPTH(type)]);

        if( !func )
            CV_ERROR( CV_StsBadArg, icvUnsupportedFormat );

        IPPI_CALL( func( mat->data.ptr, mat_step, size, CV_MAT_CN(type), coi, &count ));
    }

    __END__;

    return  count;
}


#define ICV_DEF_INTEGRAL_OP( flavor, arrtype, sumtype, sqsumtype, worktype,     \
                             cast_macro, cast_sqr_macro )                       \
IPCVAPI( CvStatus,                                                              \
    icvIntegralImage_##flavor##_C1R,( const arrtype* src, int srcstep,          \
                                    sumtype* sum, int sumstep,                  \
                                    sqsumtype* sqsum, int sqsumstep,            \
                                    sumtype* tilted, int tiltedstep,            \
                                    CvSize size ))                              \
                                                                                \
IPCVAPI_IMPL( CvStatus,                                                         \
    icvIntegralImage_##flavor##_C1R,( const arrtype* src, int srcstep,          \
                                    sumtype* sum, int sumstep,                  \
                                    sqsumtype* sqsum, int sqsumstep,            \
                                    sumtype* tilted, int tiltedstep,            \
                                    CvSize size ))                              \
{                                                                               \
    int x, y;                                                                   \
    sumtype s;                                                                  \
    sqsumtype sq;                                                               \
    sumtype* buf = 0;                                                           \
                                                                                \
    srcstep /= sizeof(src[0]);                                                  \
                                                                                \
    memset( sum, 0, (size.width+1)*sizeof(sum[0]));                             \
    sumstep /= sizeof(sum[0]);                                                  \
    sum += sumstep + 1;                                                         \
                                                                                \
    if( sqsum )                                                                 \
    {                                                                           \
        memset( sqsum, 0, (size.width+1)*sizeof(sqsum[0]));                     \
        sqsumstep /= sizeof(sqsum[0]);                                          \
        sqsum += sqsumstep + 1;                                                 \
    }                                                                           \
                                                                                \
    if( tilted )                                                                \
    {                                                                           \
        memset( tilted, 0, (size.width+1)*sizeof(tilted[0]));                   \
        tiltedstep /= sizeof(tilted[0]);                                        \
        tilted += tiltedstep + 1;                                               \
    }                                                                           \
                                                                                \
    if( sqsum == 0 && tilted == 0 )                                             \
    {                                                                           \
        sum[-1] = 0;                                                            \
        for( x = 0, s = 0; x < size.width; x++ )                                \
        {                                                                       \
            sumtype t = cast_macro(src[x]);                                     \
            sum[x] = (s += t);                                                  \
        }                                                                       \
                                                                                \
        for( y = 1; y < size.height; y++ )                                      \
        {                                                                       \
            src += srcstep;                                                     \
            sum += sumstep;                                                     \
            sum[-1] = 0;                                                        \
                                                                                \
            for( x = 0, s = 0; x < size.width; x++ )                            \
            {                                                                   \
                sumtype t = cast_macro(src[x]);                                 \
                s += t;                                                         \
                sum[x] = sum[x - sumstep] + s;                                  \
            }                                                                   \
        }                                                                       \
    }                                                                           \
    else if( tilted == 0 )                                                      \
    {                                                                           \
        sum[-1] = 0;                                                            \
        sqsum[-1] = 0;                                                          \
                                                                                \
        for( x = 0, s = 0, sq = 0; x < size.width; x++ )                        \
        {                                                                       \
            worktype it = src[x];                                               \
            sumtype t = cast_macro(it);                                         \
            sqsumtype tq = cast_sqr_macro(it);                                  \
            s += t;                                                             \
            sq += tq;                                                           \
            sum[x] = s;                                                         \
            sqsum[x] = sq;                                                      \
        }                                                                       \
                                                                                \
        for( y = 1; y < size.height; y++ )                                      \
        {                                                                       \
            src += srcstep;                                                     \
            sum += sumstep;                                                     \
            sqsum += sqsumstep;                                                 \
                                                                                \
            sum[-1] = 0;                                                        \
            sqsum[-1] = 0;                                                      \
                                                                                \
            for( x = 0, s = 0, sq = 0; x < size.width; x++ )                    \
            {                                                                   \
                worktype it = src[x];                                           \
                sumtype t = cast_macro(it);                                     \
                sqsumtype tq = cast_sqr_macro(it);                              \
                s += t;                                                         \
                sq += tq;                                                       \
                t = sum[x - sumstep] + s;                                       \
                tq = sqsum[x - sqsumstep] + sq;                                 \
                sum[x] = t;                                                     \
                sqsum[x] = tq;                                                  \
            }                                                                   \
        }                                                                       \
    }                                                                           \
    else                                                                        \
    {                                                                           \
        if( sqsum == 0 )                                                        \
        {                                                                       \
            assert(0);                                                          \
            return CV_NULLPTR_ERR;                                              \
        }                                                                       \
                                                                                \
        buf = (sumtype*)alloca( (size.width + 1 )* sizeof(buf[0]));             \
        sum[-1] = tilted[-1] = 0;                                               \
        sqsum[-1] = 0;                                                          \
                                                                                \
        for( x = 0, s = 0, sq = 0; x < size.width; x++ )                        \
        {                                                                       \
            worktype it = src[x];                                               \
            sumtype t = cast_macro(it);                                         \
            sqsumtype tq = cast_sqr_macro(it);                                  \
            buf[x] = tilted[x] = t;                                             \
            s += t;                                                             \
            sq += tq;                                                           \
            sum[x] = s;                                                         \
            sqsum[x] = sq;                                                      \
        }                                                                       \
                                                                                \
        if( size.width == 1 )                                                   \
            buf[1] = 0;                                                         \
                                                                                \
        for( y = 1; y < size.height; y++ )                                      \
        {                                                                       \
            worktype it;                                                        \
            sumtype t0;                                                         \
            sqsumtype tq0;                                                      \
                                                                                \
            src += srcstep;                                                     \
            sum += sumstep;                                                     \
            sqsum += sqsumstep;                                                 \
            tilted += tiltedstep;                                               \
                                                                                \
            it = src[0/*x*/];                                                   \
            s = t0 = cast_macro(it);                                            \
            sq = tq0 = cast_sqr_macro(it);                                      \
                                                                                \
            sum[-1] = 0;                                                        \
            sqsum[-1] = 0;                                                      \
            /*tilted[-1] = buf[0];*/                                            \
            tilted[-1] = tilted[-tiltedstep];                                   \
                                                                                \
            sum[0] = sum[-sumstep] + t0;                                        \
            sqsum[0] = sqsum[-sqsumstep] + tq0;                                 \
            tilted[0] = tilted[-tiltedstep] + t0 + buf[1];                      \
                                                                                \
            for( x = 1; x < size.width - 1; x++ )                               \
            {                                                                   \
                sumtype t1 = buf[x];                                            \
                buf[x-1] = t1 + t0;                                             \
                it = src[x];                                                    \
                t0 = cast_macro(it);                                            \
                tq0 = cast_sqr_macro(it);                                       \
                s += t0;                                                        \
                sq += tq0;                                                      \
                sum[x] = sum[x - sumstep] + s;                                  \
                sqsum[x] = sqsum[x - sqsumstep] + sq;                           \
                t1 += buf[x+1] + t0 + tilted[x - tiltedstep - 1];               \
                tilted[x] = t1;                                                 \
            }                                                                   \
                                                                                \
            if( size.width > 1 )                                                \
            {                                                                   \
                sumtype t1 = buf[x];                                            \
                buf[x-1] = t1 + t0;                                             \
                it = src[x];    /*+*/                                           \
                t0 = cast_macro(it);                                            \
                tq0 = cast_sqr_macro(it);                                       \
                s += t0;                                                        \
                sq += tq0;                                                      \
                sum[x] = sum[x - sumstep] + s;                                  \
                sqsum[x] = sqsum[x - sqsumstep] + sq;                           \
                tilted[x] = t0 + t1 + tilted[x - tiltedstep - 1];               \
                buf[x] = t0;                                                    \
            }                                                                   \
        }                                                                       \
    }                                                                           \
                                                                                \
    return CV_OK;                                                               \
}


ICV_DEF_INTEGRAL_OP( 8u32s, uchar, int, double, int, CV_NOP, CV_8TO32F_SQR )
ICV_DEF_INTEGRAL_OP( 8u64f, uchar, double, double, int, CV_8TO32F, CV_8TO32F_SQR )
ICV_DEF_INTEGRAL_OP( 32f64f, float, double, double, double, CV_NOP, CV_SQR )
ICV_DEF_INTEGRAL_OP( 64f, double, double, double, double, CV_NOP, CV_SQR )


static void icvInitIntegralImageTable( CvFuncTable* table )
{
    table->fn_2d[CV_8U] = (void*)icvIntegralImage_8u64f_C1R;
    table->fn_2d[CV_32F] = (void*)icvIntegralImage_32f64f_C1R;
    table->fn_2d[CV_64F] = (void*)icvIntegralImage_64f_C1R;
}

typedef CvStatus (CV_STDCALL * CvIntegralImageFunc)( const void* src, int srcstep,
                                                     void* sum, int sumstep,
                                                     void* sqsum, int sqsumstep,
                                                     void* tilted, int tiltedstep,
                                                     CvSize size );


CV_IMPL void
cvIntegral( const CvArr* image, CvArr* sumImage,
            CvArr* sumSqImage, CvArr* tiltedSumImage )
{
    static CvFuncTable tab;
    static int inittab = 0;
    
    CV_FUNCNAME( "cvIntegralImage" );

    __BEGIN__;

    CvMat src_stub, *src = (CvMat*)image;
    CvMat sum_stub, *sum = (CvMat*)sumImage;
    CvMat sqsum_stub, *sqsum = (CvMat*)sumSqImage;
    CvMat tilted_stub, *tilted = (CvMat*)tiltedSumImage;
    int coi0 = 0, coi1 = 0, coi2 = 0, coi3 = 0;
    CvIntegralImageFunc func = 0;

    if( !inittab )
    {
        icvInitIntegralImageTable( &tab );
        inittab = 1;
    }

    CV_CALL( src = cvGetMat( src, &src_stub, &coi0 ));
    CV_CALL( sum = cvGetMat( sum, &sum_stub, &coi1 ));
    
    if( sum->width != src->width + 1 ||
        sum->height != src->height + 1 )
        CV_ERROR( CV_StsUnmatchedSizes, "" );

    if( CV_MAT_TYPE( sum->type ) != CV_64FC1 &&
        (CV_MAT_TYPE( src->type ) != CV_8UC1 ||
         CV_MAT_TYPE( sum->type ) != CV_32SC1))
        CV_ERROR( CV_StsUnsupportedFormat,
        "Sum array must be single-channel and have 64f "
        "(or 32s in case of 8u source array) type" );

    if( sqsum )
    {
        CV_CALL( sqsum = cvGetMat( sqsum, &sqsum_stub, &coi2 ));
        if( !CV_ARE_SIZES_EQ( sum, sqsum ) )
            CV_ERROR( CV_StsUnmatchedSizes, "" );
        if( CV_MAT_TYPE( sqsum->type ) != CV_64FC1 )
            CV_ERROR( CV_StsUnsupportedFormat,
                      "Squares sum array must be 64f,single-channel" );
    }

    if( tilted )
    {
        if( !sqsum )
            CV_ERROR( CV_StsNullPtr,
            "Squared sum array must be passed if tilted sum array is passed" );

        CV_CALL( tilted = cvGetMat( tilted, &tilted_stub, &coi3 ));
        if( !CV_ARE_SIZES_EQ( sum, tilted ) )
            CV_ERROR( CV_StsUnmatchedSizes, "" );
        if( !CV_ARE_TYPES_EQ( sum, tilted ) )
            CV_ERROR( CV_StsUnmatchedFormats,
                      "Sum and tilted sum must have the same types" );
    }

    if( coi0 || coi1 || coi2 || coi3 )
        CV_ERROR( CV_BadCOI, "COI is not supported by the function" );

    if( CV_MAT_TYPE( sum->type ) == CV_32SC1 )
        func = (CvIntegralImageFunc)icvIntegralImage_8u32s_C1R;
    else
    {
        func = (CvIntegralImageFunc)tab.fn_2d[CV_MAT_DEPTH(src->type)];
        if( !func )
            CV_ERROR( CV_StsUnsupportedFormat, "This source image format is unsupported" );
    }

    IPPI_CALL( func( src->data.ptr, src->step, sum->data.ptr, sum->step,
                     sqsum ? sqsum->data.ptr : 0, sqsum ? sqsum->step : 0,
                     tilted ? tilted->data.ptr : 0, tilted ? tilted->step : 0,
                     icvGetMatSize( src )));


    __END__;
}


/* End of file. */
