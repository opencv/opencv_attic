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


#define ICV_DEF_SUM_NOHINT_FUNC_2D( __op__, _entry_, _exit_, name, flavor,  \
                             cn, srctype, sumtype, sumtype_final )          \
IPCVAPI_IMPL(CvStatus, icv##name##_##flavor##_C##cn##R,( const srctype* src,\
                                int step, CvSize size, sumtype_final* sum ),\
                                (src, step, size, sum) )                    \
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


#define ICV_DEF_SUM_HINT_FUNC_2D( __op__, _entry_, _exit_, name, flavor,    \
                             cn, srctype, sumtype, sumtype_final )          \
IPCVAPI_IMPL(CvStatus, icv##name##_##flavor##_C##cn##R,( const srctype* src,\
                                    int step, CvSize size, sumtype_final* sum,\
                                    CvHintAlgorithm /*hint*/ ),             \
                                    (src, step, size, sum, cvAlgHintAccurate) )\
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


#define ICV_DEF_SUM_FUNC_2D_COI( __op__, name, flavor, srctype,             \
                                 sumtype, sumtype_final )                   \
static CvStatus CV_STDCALL icv##name##_##flavor##_CnCR( const srctype* src, \
                int step, CvSize size, int cn, int coi, sumtype_final* sum )\
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


#define ICV_DEF_SUM_ALL( __op__, name, flavor, srctype, sumtype, sumtype_final, hint )\
    ICV_DEF_SUM_##hint##_FUNC_2D( __op__, CV_SUM_ENTRY_C1, CV_SUM_EXIT_C1,      \
                         name, flavor, 1, srctype, sumtype, sumtype_final )     \
    ICV_DEF_SUM_##hint##_FUNC_2D( __op__, CV_SUM_ENTRY_C2, CV_SUM_EXIT_C2,      \
                         name, flavor, 2, srctype, sumtype, sumtype_final )     \
    ICV_DEF_SUM_##hint##_FUNC_2D( __op__, CV_SUM_ENTRY_C3, CV_SUM_EXIT_C3,      \
                         name, flavor, 3, srctype, sumtype, sumtype_final )     \
    ICV_DEF_SUM_##hint##_FUNC_2D( __op__, CV_SUM_ENTRY_C4, CV_SUM_EXIT_C4,      \
                         name, flavor, 4, srctype, sumtype, sumtype_final )     \
    ICV_DEF_SUM_FUNC_2D_COI( __op__, name, flavor, srctype, sumtype, sumtype_final )


ICV_DEF_SUM_ALL( CV_NOP, Sum, 8u, uchar, int64, double, NOHINT )
ICV_DEF_SUM_ALL( CV_NOP, Sum, 16u, ushort, int64, double, NOHINT )
ICV_DEF_SUM_ALL( CV_NOP, Sum, 16s, short, int64, double, NOHINT )
ICV_DEF_SUM_ALL( CV_CAST_64S, Sum, 32s, int, int64, double, NOHINT )
ICV_DEF_SUM_ALL( CV_NOP, Sum, 32f, float, double, double, HINT )
ICV_DEF_SUM_ALL( CV_NOP, Sum, 64f, double, double, double, NOHINT )

#define icvSum_8s_C1R   0
#define icvSum_8s_C2R   0
#define icvSum_8s_C3R   0
#define icvSum_8s_C4R   0
#define icvSum_8s_CnCR  0

CV_DEF_INIT_BIG_FUNC_TAB_2D( Sum, R )
CV_DEF_INIT_FUNC_TAB_2D( Sum, CnCR )

CV_IMPL CvScalar
cvSum( const CvArr* arr )
{
    static CvBigFuncTable sum_tab;
    static CvFuncTable sumcoi_tab;
    static int inittab = 0;

    CvScalar sum = {0,0,0,0};

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
            CvNArrayIterator iterator;
            int pass_hint;

            CV_CALL( cvInitNArrayIterator( 1, (void**)&mat, 0, &stub, &iterator ));

            type = CV_MAT_TYPE(iterator.hdr[0]->type);

            pass_hint = CV_MAT_DEPTH(type) == CV_32F;

            if( !pass_hint )
            {
                CvFunc2D_1A1P func = (CvFunc2D_1A1P)(sum_tab.fn_2d[type]);
                if( !func )
                    CV_ERROR( CV_StsUnsupportedFormat, "" );
       
                do
                {
                    CvScalar temp = {0,0,0,0};
                    IPPI_CALL( func( iterator.ptr[0], CV_STUB_STEP,
                                     iterator.size, temp.val ));
                    sum.val[0] += temp.val[0];
                    sum.val[1] += temp.val[1];
                    sum.val[2] += temp.val[2];
                    sum.val[3] += temp.val[3];
                }
                while( cvNextNArraySlice( &iterator ));
            }
            else
            {
                CvFunc2D_1A1P1I func = (CvFunc2D_1A1P1I)(sum_tab.fn_2d[type]);
                if( !func )
                    CV_ERROR( CV_StsUnsupportedFormat, "" );
       
                do
                {
                    CvScalar temp = {0,0,0,0};
                    IPPI_CALL( func( iterator.ptr[0], CV_STUB_STEP,
                                     iterator.size, temp.val, cvAlgHintAccurate ));
                    sum.val[0] += temp.val[0];
                    sum.val[1] += temp.val[1];
                    sum.val[2] += temp.val[2];
                    sum.val[3] += temp.val[3];
                }
                while( cvNextNArraySlice( &iterator ));
            }
            EXIT;
        }
        else
            CV_CALL( mat = cvGetMat( mat, &stub, &coi ));
    }

    type = CV_MAT_TYPE(mat->type);
    size = cvGetMatSize( mat );

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
        int pass_hint = CV_MAT_DEPTH(type) == CV_32F;
        if( !pass_hint )
        {
            CvFunc2D_1A1P func = (CvFunc2D_1A1P)(sum_tab.fn_2d[type]);

            if( !func )
                CV_ERROR( CV_StsBadArg, cvUnsupportedFormat );

            IPPI_CALL( func( mat->data.ptr, mat_step, size, sum.val ));
        }
        else
        {
            CvFunc2D_1A1P1I func = (CvFunc2D_1A1P1I)(sum_tab.fn_2d[type]);

            if( !func )
                CV_ERROR( CV_StsBadArg, cvUnsupportedFormat );

            IPPI_CALL( func( mat->data.ptr, mat_step, size, sum.val, cvAlgHintAccurate ));
        }
    }
    else
    {
        CvFunc2DnC_1A1P func = (CvFunc2DnC_1A1P)(sumcoi_tab.fn_2d[CV_MAT_DEPTH(type)]);

        if( !func )
            CV_ERROR( CV_StsBadArg, cvUnsupportedFormat );

        IPPI_CALL( func( mat->data.ptr, mat_step, size,
                         CV_MAT_CN(type), coi, sum.val ));
    }

    __END__;

    return  sum;
}


#define ICV_DEF_SUM_C1( __op__, name, flavor, srctype, sumtype )        \
    ICV_DEF_SUM_NOHINT_FUNC_2D( __op__, CV_SUM_ENTRY_C1, CV_SUM_EXIT_C1,\
                         name, flavor, 1, srctype, sumtype, int )       \
    ICV_DEF_SUM_FUNC_2D_COI( __op__, name, flavor, srctype, sumtype, int )

ICV_DEF_SUM_C1( CV_NONZERO, CountNonZero, 8u, uchar, int )
ICV_DEF_SUM_C1( CV_NONZERO, CountNonZero, 16s, ushort, int )
ICV_DEF_SUM_C1( CV_NONZERO, CountNonZero, 32s, int, int )
ICV_DEF_SUM_C1( CV_NONZERO_FLT, CountNonZero, 32f, int, int )
ICV_DEF_SUM_C1( CV_NONZERO_FLT, CountNonZero, 64f, int64, int )

#define icvCountNonZero_8s_C1R icvCountNonZero_8u_C1R
#define icvCountNonZero_8s_CnCR icvCountNonZero_8u_CnCR
#define icvCountNonZero_16u_C1R icvCountNonZero_16s_C1R
#define icvCountNonZero_16u_CnCR icvCountNonZero_16s_CnCR

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
            CvNArrayIterator iterator;
            CvFunc2D_1A1P func;

            CV_CALL( cvInitNArrayIterator( 1, (void**)&mat, 0, &stub, &iterator ));

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
            while( cvNextNArraySlice( &iterator ));
            EXIT;
        }
        else
            CV_CALL( mat = cvGetMat( mat, &stub, &coi ));
    }

    type = CV_MAT_TYPE(mat->type);
    size = cvGetMatSize( mat );

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
            CV_ERROR( CV_StsBadArg, cvUnsupportedFormat );

        IPPI_CALL( func( mat->data.ptr, mat_step, size, &count ));
    }
    else
    {
        CvFunc2DnC_1A1P func = (CvFunc2DnC_1A1P)(nzcoi_tab.fn_2d[CV_MAT_DEPTH(type)]);

        if( !func )
            CV_ERROR( CV_StsBadArg, cvUnsupportedFormat );

        IPPI_CALL( func( mat->data.ptr, mat_step, size, CV_MAT_CN(type), coi, &count ));
    }

    __END__;

    return  count;
}

/* End of file. */
