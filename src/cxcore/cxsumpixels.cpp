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

#define ICV_SUM_COI_CASE( __op__, len, cn )                 \
    for( ; x <= (len) - 4*(cn); x += 4*(cn) )               \
        s0 += __op__(src[x]) + __op__(src[x+(cn)]) +        \
              __op__(src[x+(cn)*2]) + __op__(src[x+(cn)*3]);\
                                                            \
    for( ; x < (len); x += (cn) )                           \
        s0 += __op__(src[x]);


#define ICV_SUM_CASE_C1( __op__, len )                      \
    ICV_SUM_COI_CASE( __op__, len, 1 )


#define ICV_SUM_CASE_C2( __op__, len )                      \
    for( ; x <= (len) - 8; x += 8 )                         \
    {                                                       \
        s0 += __op__(src[x]) + __op__(src[x+2]) +           \
              __op__(src[x+4]) + __op__(src[x+6]);          \
        s1 += __op__(src[x+1]) + __op__(src[x+3]) +         \
              __op__(src[x+5]) + __op__(src[x+7]);          \
    }                                                       \
                                                            \
    for( ; x < (len); x += 2 )                              \
    {                                                       \
        s0 += __op__(src[x]);                               \
        s1 += __op__(src[x+1]);                             \
    }



#define ICV_SUM_CASE_C3( __op__, len )                      \
    for( ; x <= (len) - 12; x += 12 )                       \
    {                                                       \
        s0 += __op__(src[x]) + __op__(src[x+3]) +           \
              __op__(src[x+6]) + __op__(src[x+9]);          \
        s1 += __op__(src[x+1]) + __op__(src[x+4]) +         \
              __op__(src[x+7]) + __op__(src[x+10]);         \
        s2 += __op__(src[x+2]) + __op__(src[x+5]) +         \
              __op__(src[x+8]) + __op__(src[x+11]);         \
    }                                                       \
                                                            \
    for( ; x < (len); x += 3 )                              \
    {                                                       \
        s0 += __op__(src[x]);                               \
        s1 += __op__(src[x+1]);                             \
        s2 += __op__(src[x+2]);                             \
    }


#define ICV_SUM_CASE_C4( __op__, len )                      \
    for( ; x <= (len) - 16; x += 16 )                       \
    {                                                       \
        s0 += __op__(src[x]) + __op__(src[x+4]) +           \
              __op__(src[x+8]) + __op__(src[x+12]);         \
        s1 += __op__(src[x+1]) + __op__(src[x+5]) +         \
              __op__(src[x+9]) + __op__(src[x+13]);         \
        s2 += __op__(src[x+2]) + __op__(src[x+6]) +         \
              __op__(src[x+10]) + __op__(src[x+14]);        \
        s3 += __op__(src[x+3]) + __op__(src[x+7]) +         \
              __op__(src[x+11]) + __op__(src[x+15]);        \
    }                                                       \
                                                            \
    for( ; x < (len); x += 4 )                              \
    {                                                       \
        s0 += __op__(src[x]);                               \
        s1 += __op__(src[x+1]);                             \
        s2 += __op__(src[x+2]);                             \
        s3 += __op__(src[x+3]);                             \
    }


////////////////////////////////////// entry macros //////////////////////////////////////

#define ICV_SUM_ENTRY_COMMON()          \
    step /= sizeof(src[0])

#define ICV_SUM_ENTRY_C1( sumtype )     \
    sumtype s0 = 0;                     \
    ICV_SUM_ENTRY_COMMON()

#define ICV_SUM_ENTRY_C2( sumtype )     \
    sumtype s0 = 0, s1 = 0;             \
    ICV_SUM_ENTRY_COMMON()

#define ICV_SUM_ENTRY_C3( sumtype )     \
    sumtype s0 = 0, s1 = 0, s2 = 0;     \
    ICV_SUM_ENTRY_COMMON()

#define ICV_SUM_ENTRY_C4( sumtype )         \
    sumtype s0 = 0, s1 = 0, s2 = 0, s3 = 0; \
    ICV_SUM_ENTRY_COMMON()


#define ICV_SUM_ENTRY_BLOCK_COMMON( block_size )    \
    int remaining = block_size;                     \
    ICV_SUM_ENTRY_COMMON()

#define ICV_SUM_ENTRY_BLOCK_C1( sumtype, worktype, block_size ) \
    sumtype sum0 = 0;                                           \
    worktype s0 = 0;                                            \
    ICV_SUM_ENTRY_BLOCK_COMMON( block_size )

#define ICV_SUM_ENTRY_BLOCK_C2( sumtype, worktype, block_size ) \
    sumtype sum0 = 0, sum1 = 0;                                 \
    worktype s0 = 0, s1 = 0;                                    \
    ICV_SUM_ENTRY_BLOCK_COMMON( block_size )

#define ICV_SUM_ENTRY_BLOCK_C3( sumtype, worktype, block_size ) \
    sumtype sum0 = 0, sum1 = 0, sum2 = 0;                       \
    worktype s0 = 0, s1 = 0, s2 = 0;                            \
    ICV_SUM_ENTRY_BLOCK_COMMON( block_size )

#define ICV_SUM_ENTRY_BLOCK_C4( sumtype, worktype, block_size ) \
    sumtype sum0 = 0, sum1 = 0, sum2 = 0, sum3 = 0;             \
    worktype s0 = 0, s1 = 0, s2 = 0, s3 = 0;                    \
    ICV_SUM_ENTRY_BLOCK_COMMON( block_size )


/////////////////////////////////////// exit macros //////////////////////////////////////

#define ICV_SUM_EXIT_C1( tmp, sumtype )     \
    sum[0] = (sumtype)tmp##0

#define ICV_SUM_EXIT_C2( tmp, sumtype )     \
    sum[0] = (sumtype)tmp##0;               \
    sum[1] = (sumtype)tmp##1;

#define ICV_SUM_EXIT_C3( tmp, sumtype )     \
    sum[0] = (sumtype)tmp##0;               \
    sum[1] = (sumtype)tmp##1;               \
    sum[2] = (sumtype)tmp##2;

#define ICV_SUM_EXIT_C4( tmp, sumtype )     \
    sum[0] = (sumtype)tmp##0;               \
    sum[1] = (sumtype)tmp##1;               \
    sum[2] = (sumtype)tmp##2;               \
    sum[3] = (sumtype)tmp##3;

#define ICV_SUM_EXIT_BLOCK_C1( sumtype )    \
    sum0 += s0;                             \
    ICV_SUM_EXIT_C1( sum, sumtype )

#define ICV_SUM_EXIT_BLOCK_C2( sumtype )    \
    sum0 += s0; sum1 += s1;                 \
    ICV_SUM_EXIT_C2( sum, sumtype )

#define ICV_SUM_EXIT_BLOCK_C3( sumtype )    \
    sum0 += s0; sum1 += s1;                 \
    sum2 += s2;                             \
    ICV_SUM_EXIT_C3( sum, sumtype )

#define ICV_SUM_EXIT_BLOCK_C4( sumtype )    \
    sum0 += s0; sum1 += s1;                 \
    sum2 += s2; sum3 += s3;                 \
    ICV_SUM_EXIT_C4( sum, sumtype )

////////////////////////////////////// update macros /////////////////////////////////////

#define ICV_SUM_UPDATE_COMMON( block_size ) \
    remaining = block_size

#define ICV_SUM_UPDATE_C1( block_size )     \
    ICV_SUM_UPDATE_COMMON( block_size );    \
    sum0 += s0;                             \
    s0 = 0

#define ICV_SUM_UPDATE_C2( block_size )     \
    ICV_SUM_UPDATE_COMMON( block_size );    \
    sum0 += s0; sum1 += s1;                 \
    s0 = s1 = 0

#define ICV_SUM_UPDATE_C3( block_size )     \
    ICV_SUM_UPDATE_COMMON( block_size );    \
    sum0 += s0; sum1 += s1; sum2 += s2;     \
    s0 = s1 = s2 = 0

#define ICV_SUM_UPDATE_C4( block_size )     \
    ICV_SUM_UPDATE_COMMON( block_size );    \
    sum0 += s0; sum1 += s1;                 \
    sum2 += s2; sum3 += s3;                 \
    s0 = s1 = s2 = s3 = 0


#define ICV_DEF_SUM_NOHINT_BLOCK_FUNC_2D( name, flavor, cn,     \
    __op__, arrtype, sumtype_final, sumtype, worktype, block_size )\
IPCVAPI_IMPL(CvStatus, icv##name##_##flavor##_C##cn##R,(        \
    const arrtype* src, int step, CvSize size,                  \
    sumtype_final* sum ), (src, step, size, sum) )              \
{                                                               \
    ICV_SUM_ENTRY_BLOCK_C##cn(sumtype,worktype,(block_size)*(cn)); \
    size.width *= cn;                                           \
                                                                \
    for( ; size.height--; src += step )                         \
    {                                                           \
        int x = 0;                                              \
        while( x < size.width )                                 \
        {                                                       \
            int limit = MIN( remaining, size.width - x );       \
            remaining -= limit;                                 \
            limit += x;                                         \
            ICV_SUM_CASE_C##cn( __op__, limit );                \
            if( remaining == 0 )                                \
            {                                                   \
                ICV_SUM_UPDATE_C##cn( (block_size)*(cn) );      \
            }                                                   \
        }                                                       \
    }                                                           \
                                                                \
    ICV_SUM_EXIT_BLOCK_C##cn( sumtype_final );                  \
    return CV_OK;                                               \
}


#define ICV_DEF_SUM_NOHINT_FUNC_2D( name, flavor, cn,           \
    __op__, arrtype, sumtype_final, sumtype, worktype, block_size )\
IPCVAPI_IMPL(CvStatus, icv##name##_##flavor##_C##cn##R,(        \
    const arrtype* src, int step, CvSize size,                  \
    sumtype_final* sum ), (src, step, size, sum) )              \
{                                                               \
    ICV_SUM_ENTRY_C##cn( sumtype );                             \
    size.width *= cn;                                           \
                                                                \
    for( ; size.height--; src += step )                         \
    {                                                           \
        int x = 0;                                              \
        ICV_SUM_CASE_C##cn( __op__, size.width );               \
    }                                                           \
                                                                \
    ICV_SUM_EXIT_C##cn( s, sumtype_final );                     \
    return CV_OK;                                               \
}


#define ICV_DEF_SUM_HINT_FUNC_2D( name, flavor, cn,             \
    __op__, arrtype, sumtype_final, sumtype, worktype, block_size )\
IPCVAPI_IMPL(CvStatus, icv##name##_##flavor##_C##cn##R,(        \
    const arrtype* src, int step, CvSize size,                  \
    sumtype_final* sum, CvHintAlgorithm /*hint*/ ),             \
    (src, step, size, sum, cvAlgHintAccurate) )                 \
{                                                               \
    ICV_SUM_ENTRY_C##cn( sumtype );                             \
    size.width *= cn;                                           \
                                                                \
    for( ; size.height--; src += step )                         \
    {                                                           \
        int x = 0;                                              \
        ICV_SUM_CASE_C##cn( __op__, size.width );               \
    }                                                           \
                                                                \
    ICV_SUM_EXIT_C##cn( s, sumtype_final );                     \
    return CV_OK;                                               \
}


#define ICV_DEF_SUM_NOHINT_BLOCK_FUNC_2D_COI( name, flavor,     \
    __op__, arrtype, sumtype_final, sumtype, worktype, block_size )\
static CvStatus CV_STDCALL icv##name##_##flavor##_CnCR(         \
    const arrtype* src, int step, CvSize size, int cn,          \
    int coi, sumtype_final* sum )                               \
{                                                               \
    ICV_SUM_ENTRY_BLOCK_C1(sumtype,worktype,(block_size)*(cn)); \
    size.width *= cn;                                           \
    src += coi - 1;                                             \
                                                                \
    for( ; size.height--; src += step )                         \
    {                                                           \
        int x = 0;                                              \
        while( x < size.width )                                 \
        {                                                       \
            int limit = MIN( remaining, size.width - x );       \
            remaining -= limit;                                 \
            limit += x;                                         \
            ICV_SUM_COI_CASE( __op__, limit, cn );              \
            if( remaining == 0 )                                \
            {                                                   \
                ICV_SUM_UPDATE_C1( (block_size)*(cn) );         \
            }                                                   \
        }                                                       \
    }                                                           \
                                                                \
    ICV_SUM_EXIT_BLOCK_C1( sumtype_final );                     \
    return CV_OK;                                               \
}


#define ICV_DEF_SUM_NOHINT_FUNC_2D_COI( name, flavor,           \
    __op__, arrtype, sumtype_final, sumtype, worktype, block_size )\
static CvStatus CV_STDCALL icv##name##_##flavor##_CnCR(         \
    const arrtype* src, int step, CvSize size, int cn,          \
    int coi, sumtype_final* sum )                               \
{                                                               \
    ICV_SUM_ENTRY_C1( sumtype );                                \
    size.width *= cn;                                           \
    src += coi - 1;                                             \
                                                                \
    for( ; size.height--; src += step )                         \
    {                                                           \
        int x = 0;                                              \
        ICV_SUM_COI_CASE( __op__, size.width, cn );             \
    }                                                           \
                                                                \
    ICV_SUM_EXIT_C1( s, sumtype_final );                        \
    return CV_OK;                                               \
}


#define ICV_DEF_SUM_ALL( name, flavor, __op__, arrtype, sumtype_final, sumtype, \
                         worktype, hintp_type, nohint_type, block_size )        \
    ICV_DEF_SUM_##hintp_type##_FUNC_2D( name, flavor, 1, __op__, arrtype,       \
                         sumtype_final, sumtype, worktype, block_size )         \
    ICV_DEF_SUM_##hintp_type##_FUNC_2D( name, flavor, 2, __op__, arrtype,       \
                         sumtype_final, sumtype, worktype, block_size )         \
    ICV_DEF_SUM_##hintp_type##_FUNC_2D( name, flavor, 3, __op__, arrtype,       \
                         sumtype_final, sumtype, worktype, block_size )         \
    ICV_DEF_SUM_##hintp_type##_FUNC_2D( name, flavor, 4, __op__, arrtype,       \
                         sumtype_final, sumtype, worktype, block_size )         \
    ICV_DEF_SUM_##nohint_type##_FUNC_2D_COI( name, flavor, __op__, arrtype,     \
                         sumtype_final, sumtype, worktype, block_size )

ICV_DEF_SUM_ALL( Sum, 8u, CV_NOP, uchar, double, int64, unsigned,
                 NOHINT_BLOCK, NOHINT_BLOCK, 1 << 24 )
ICV_DEF_SUM_ALL( Sum, 16u, CV_NOP, ushort, double, int64, unsigned,
                 NOHINT_BLOCK, NOHINT_BLOCK, 1 << 16 )
ICV_DEF_SUM_ALL( Sum, 16s, CV_NOP, short, double, int64, int,
                 NOHINT_BLOCK, NOHINT_BLOCK, 1 << 16 )
ICV_DEF_SUM_ALL( Sum, 32s, CV_NOP, int, double, double, double, NOHINT, NOHINT, 0 )
ICV_DEF_SUM_ALL( Sum, 32f, CV_NOP, float, double, double, double, HINT, NOHINT, 0 )
ICV_DEF_SUM_ALL( Sum, 64f, CV_NOP, double, double, double, double, NOHINT, NOHINT, 0 )

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

    CvScalar sum = {{0,0,0,0}};

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
            CvMatND nstub;
            CvNArrayIterator iterator;
            int pass_hint;

            CV_CALL( cvInitNArrayIterator( 1, (void**)&mat, 0, &nstub, &iterator ));

            type = CV_MAT_TYPE(iterator.hdr[0]->type);

            pass_hint = CV_MAT_DEPTH(type) == CV_32F;

            if( !pass_hint )
            {
                CvFunc2D_1A1P func = (CvFunc2D_1A1P)(sum_tab.fn_2d[type]);
                if( !func )
                    CV_ERROR( CV_StsUnsupportedFormat, "" );
       
                do
                {
                    CvScalar temp = {{0,0,0,0}};
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
                    CvScalar temp = {{0,0,0,0}};
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


#define ICV_DEF_NONZERO_ALL( flavor, __op__, arrtype )              \
    ICV_DEF_SUM_NOHINT_FUNC_2D( CountNonZero, flavor, 1, __op__,    \
                                arrtype, int, int, int, 0 )         \
    ICV_DEF_SUM_NOHINT_FUNC_2D_COI( CountNonZero, flavor, __op__,   \
                                    arrtype, int, int, int, 0 )

#undef  CV_NONZERO_DBL
#define CV_NONZERO_DBL(x) (((x) & CV_BIG_INT(0x7fffffffffffffff)) != 0)

ICV_DEF_NONZERO_ALL( 8u, CV_NONZERO, uchar )
ICV_DEF_NONZERO_ALL( 16s, CV_NONZERO, ushort )
ICV_DEF_NONZERO_ALL( 32s, CV_NONZERO, int )
ICV_DEF_NONZERO_ALL( 32f, CV_NONZERO_FLT, int )
ICV_DEF_NONZERO_ALL( 64f, CV_NONZERO_DBL, int64 )

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
            CvMatND nstub;
            CvNArrayIterator iterator;
            CvFunc2D_1A1P func;

            CV_CALL( cvInitNArrayIterator( 1, (void**)&mat, 0, &nstub, &iterator ));

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
