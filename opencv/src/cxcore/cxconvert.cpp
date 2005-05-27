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
*                             Conversion pixel -> plane                                  *
\****************************************************************************************/

#define  ICV_DEF_PX2PL2PX_ENTRY_C2( arrtype_ptr, ptr )  \
    arrtype_ptr plane0 = ptr[0];                        \
    arrtype_ptr plane1 = ptr[1];

#define  ICV_DEF_PX2PL2PX_ENTRY_C3( arrtype_ptr, ptr )  \
    arrtype_ptr plane0 = ptr[0];                        \
    arrtype_ptr plane1 = ptr[1];                        \
    arrtype_ptr plane2 = ptr[2];

#define  ICV_DEF_PX2PL2PX_ENTRY_C4( arrtype_ptr, ptr )  \
    arrtype_ptr plane0 = ptr[0];                        \
    arrtype_ptr plane1 = ptr[1];                        \
    arrtype_ptr plane2 = ptr[2];                        \
    arrtype_ptr plane3 = ptr[3];


#define  ICV_DEF_PX2PL_C2( arrtype, len )           \
{                                                   \
    int j;                                          \
                                                    \
    for( j = 0; j < (len); j++, (src) += 2 )        \
    {                                               \
        arrtype t0 = (src)[0];                      \
        arrtype t1 = (src)[1];                      \
                                                    \
        plane0[j] = t0;                             \
        plane1[j] = t1;                             \
    }                                               \
    plane0 += dststep;                              \
    plane1 += dststep;                              \
}


#define  ICV_DEF_PX2PL_C3( arrtype, len )           \
{                                                   \
    int j;                                          \
                                                    \
    for( j = 0; j < (len); j++, (src) += 3 )        \
    {                                               \
        arrtype t0 = (src)[0];                      \
        arrtype t1 = (src)[1];                      \
        arrtype t2 = (src)[2];                      \
                                                    \
        plane0[j] = t0;                             \
        plane1[j] = t1;                             \
        plane2[j] = t2;                             \
    }                                               \
    plane0 += dststep;                              \
    plane1 += dststep;                              \
    plane2 += dststep;                              \
}


#define  ICV_DEF_PX2PL_C4( arrtype, len )           \
{                                                   \
    int j;                                          \
                                                    \
    for( j = 0; j < (len); j++, (src) += 4 )        \
    {                                               \
        arrtype t0 = (src)[0];                      \
        arrtype t1 = (src)[1];                      \
                                                    \
        plane0[j] = t0;                             \
        plane1[j] = t1;                             \
                                                    \
        t0 = (src)[2];                              \
        t1 = (src)[3];                              \
                                                    \
        plane2[j] = t0;                             \
        plane3[j] = t1;                             \
    }                                               \
    plane0 += dststep;                              \
    plane1 += dststep;                              \
    plane2 += dststep;                              \
    plane3 += dststep;                              \
}


#define  ICV_DEF_PX2PL_COI( arrtype, len, cn )      \
{                                                   \
    int j;                                          \
                                                    \
    for( j = 0; j <= (len) - 4; j += 4, (src) += 4*(cn))\
    {                                               \
        arrtype t0 = (src)[0];                      \
        arrtype t1 = (src)[(cn)];                   \
                                                    \
        (dst)[j] = t0;                              \
        (dst)[j+1] = t1;                            \
                                                    \
        t0 = (src)[(cn)*2];                         \
        t1 = (src)[(cn)*3];                         \
                                                    \
        (dst)[j+2] = t0;                            \
        (dst)[j+3] = t1;                            \
    }                                               \
                                                    \
    for( ; j < (len); j++, (src) += (cn))           \
    {                                               \
        (dst)[j] = (src)[0];                        \
    }                                               \
}


#define  ICV_DEF_COPY_PX2PL_FUNC_2D( arrtype, flavor,   \
                                     cn, entry_macro )  \
IPCVAPI_IMPL( CvStatus, icvCopy_##flavor##_C##cn##P##cn##R,\
( const arrtype* src, int srcstep,                      \
  arrtype** dst, int dststep, CvSize size ),            \
  (src, srcstep, dst, dststep, size))                   \
{                                                       \
    entry_macro(arrtype*, dst);                         \
    srcstep /= sizeof(arrtype);                         \
    dststep /= sizeof(arrtype);                         \
                                                        \
    for( ; size.height--; src += srcstep )              \
    {                                                   \
        ICV_DEF_PX2PL_C##cn( arrtype, size.width );     \
        src -= size.width*(cn);                         \
    }                                                   \
                                                        \
    return CV_OK;                                       \
}


#define  ICV_DEF_COPY_PX2PL_FUNC_2D_COI( arrtype, flavor )\
IPCVAPI_IMPL( CvStatus, icvCopy_##flavor##_CnC1CR,      \
( const arrtype* src, int srcstep, arrtype* dst, int dststep,\
  CvSize size, int cn, int coi ),                       \
  (src, srcstep, dst, dststep, size, cn, coi))          \
{                                                       \
    src += coi - 1;                                     \
    srcstep /= sizeof(arrtype);                         \
    dststep /= sizeof(arrtype);                         \
                                                        \
    for( ; size.height--; src += srcstep, dst += dststep )\
    {                                                   \
        ICV_DEF_PX2PL_COI( arrtype, size.width, cn );   \
        src -= size.width*(cn);                         \
    }                                                   \
                                                        \
    return CV_OK;                                       \
}


ICV_DEF_COPY_PX2PL_FUNC_2D( uchar, 8u, 2, ICV_DEF_PX2PL2PX_ENTRY_C2 )
ICV_DEF_COPY_PX2PL_FUNC_2D( uchar, 8u, 3, ICV_DEF_PX2PL2PX_ENTRY_C3 )
ICV_DEF_COPY_PX2PL_FUNC_2D( uchar, 8u, 4, ICV_DEF_PX2PL2PX_ENTRY_C4 )
ICV_DEF_COPY_PX2PL_FUNC_2D( ushort, 16s, 2, ICV_DEF_PX2PL2PX_ENTRY_C2 )
ICV_DEF_COPY_PX2PL_FUNC_2D( ushort, 16s, 3, ICV_DEF_PX2PL2PX_ENTRY_C3 )
ICV_DEF_COPY_PX2PL_FUNC_2D( ushort, 16s, 4, ICV_DEF_PX2PL2PX_ENTRY_C4 )
ICV_DEF_COPY_PX2PL_FUNC_2D( int, 32f, 2, ICV_DEF_PX2PL2PX_ENTRY_C2 )
ICV_DEF_COPY_PX2PL_FUNC_2D( int, 32f, 3, ICV_DEF_PX2PL2PX_ENTRY_C3 )
ICV_DEF_COPY_PX2PL_FUNC_2D( int, 32f, 4, ICV_DEF_PX2PL2PX_ENTRY_C4 )
ICV_DEF_COPY_PX2PL_FUNC_2D( int64, 64f, 2, ICV_DEF_PX2PL2PX_ENTRY_C2 )
ICV_DEF_COPY_PX2PL_FUNC_2D( int64, 64f, 3, ICV_DEF_PX2PL2PX_ENTRY_C3 )
ICV_DEF_COPY_PX2PL_FUNC_2D( int64, 64f, 4, ICV_DEF_PX2PL2PX_ENTRY_C4 )


ICV_DEF_COPY_PX2PL_FUNC_2D_COI( uchar, 8u )
ICV_DEF_COPY_PX2PL_FUNC_2D_COI( ushort, 16s )
ICV_DEF_COPY_PX2PL_FUNC_2D_COI( int, 32f )
ICV_DEF_COPY_PX2PL_FUNC_2D_COI( int64, 64f )


/****************************************************************************************\
*                             Conversion plane -> pixel                                  *
\****************************************************************************************/


#define  ICV_DEF_PL2PX_C2( arrtype, len )   \
{                                           \
    int j;                                  \
                                            \
    for( j = 0; j < (len); j++, (dst) += 2 )\
    {                                       \
        arrtype t0 = plane0[j];             \
        arrtype t1 = plane1[j];             \
                                            \
        dst[0] = t0;                        \
        dst[1] = t1;                        \
    }                                       \
    plane0 += srcstep;                      \
    plane1 += srcstep;                      \
}


#define  ICV_DEF_PL2PX_C3( arrtype, len )   \
{                                           \
    int j;                                  \
                                            \
    for( j = 0; j < (len); j++, (dst) += 3 )\
    {                                       \
        arrtype t0 = plane0[j];             \
        arrtype t1 = plane1[j];             \
        arrtype t2 = plane2[j];             \
                                            \
        dst[0] = t0;                        \
        dst[1] = t1;                        \
        dst[2] = t2;                        \
    }                                       \
    plane0 += srcstep;                      \
    plane1 += srcstep;                      \
    plane2 += srcstep;                      \
}


#define  ICV_DEF_PL2PX_C4( arrtype, len )   \
{                                           \
    int j;                                  \
                                            \
    for( j = 0; j < (len); j++, (dst) += 4 )\
    {                                       \
        arrtype t0 = plane0[j];             \
        arrtype t1 = plane1[j];             \
                                            \
        dst[0] = t0;                        \
        dst[1] = t1;                        \
                                            \
        t0 = plane2[j];                     \
        t1 = plane3[j];                     \
                                            \
        dst[2] = t0;                        \
        dst[3] = t1;                        \
    }                                       \
    plane0 += srcstep;                      \
    plane1 += srcstep;                      \
    plane2 += srcstep;                      \
    plane3 += srcstep;                      \
}


#define  ICV_DEF_PL2PX_COI( arrtype, len, cn )          \
{                                                       \
    int j;                                              \
                                                        \
    for( j = 0; j <= (len) - 4; j += 4, (dst) += 4*(cn))\
    {                                                   \
        arrtype t0 = (src)[j];                          \
        arrtype t1 = (src)[j+1];                        \
                                                        \
        (dst)[0] = t0;                                  \
        (dst)[(cn)] = t1;                               \
                                                        \
        t0 = (src)[j+2];                                \
        t1 = (src)[j+3];                                \
                                                        \
        (dst)[(cn)*2] = t0;                             \
        (dst)[(cn)*3] = t1;                             \
    }                                                   \
                                                        \
    for( ; j < (len); j++, (dst) += (cn))               \
    {                                                   \
        (dst)[0] = (src)[j];                            \
    }                                                   \
}


#define  ICV_DEF_COPY_PL2PX_FUNC_2D( arrtype, flavor, cn, entry_macro )         \
IPCVAPI_IMPL( CvStatus,                                                         \
icvCopy_##flavor##_P##cn##C##cn##R,( const arrtype** src, int srcstep,          \
                                    arrtype* dst, int dststep, CvSize size ),   \
                                    (src, srcstep, dst, dststep, size))         \
{                                                                               \
    entry_macro(const arrtype*, src);                                           \
    srcstep /= sizeof(arrtype);                                                 \
    dststep /= sizeof(arrtype);                                                 \
                                                                                \
    for( ; size.height--; dst += dststep )                                      \
    {                                                                           \
        ICV_DEF_PL2PX_C##cn( arrtype, size.width );                             \
        dst -= size.width*(cn);                                                 \
    }                                                                           \
                                                                                \
    return CV_OK;                                                               \
}


#define  ICV_DEF_COPY_PL2PX_FUNC_2D_COI( arrtype, flavor )              \
IPCVAPI_IMPL( CvStatus, icvCopy_##flavor##_C1CnCR,                      \
( const arrtype* src, int srcstep, arrtype* dst, int dststep,           \
  CvSize size, int cn, int coi ), (src, srcstep, dst, dststep, size, cn, coi))\
{                                                                       \
    dst += coi - 1;                                                     \
    for( ; size.height--; (char*&)src += srcstep,                       \
                          (char*&)dst += dststep )                      \
    {                                                                   \
        ICV_DEF_PL2PX_COI( arrtype, size.width, cn );                   \
        dst -= size.width*(cn);                                         \
    }                                                                   \
                                                                        \
    return CV_OK;                                                       \
}


ICV_DEF_COPY_PL2PX_FUNC_2D( uchar, 8u, 2, ICV_DEF_PX2PL2PX_ENTRY_C2 )
ICV_DEF_COPY_PL2PX_FUNC_2D( uchar, 8u, 3, ICV_DEF_PX2PL2PX_ENTRY_C3 )
ICV_DEF_COPY_PL2PX_FUNC_2D( uchar, 8u, 4, ICV_DEF_PX2PL2PX_ENTRY_C4 )
ICV_DEF_COPY_PL2PX_FUNC_2D( ushort, 16s, 2, ICV_DEF_PX2PL2PX_ENTRY_C2 )
ICV_DEF_COPY_PL2PX_FUNC_2D( ushort, 16s, 3, ICV_DEF_PX2PL2PX_ENTRY_C3 )
ICV_DEF_COPY_PL2PX_FUNC_2D( ushort, 16s, 4, ICV_DEF_PX2PL2PX_ENTRY_C4 )
ICV_DEF_COPY_PL2PX_FUNC_2D( int, 32f, 2, ICV_DEF_PX2PL2PX_ENTRY_C2 )
ICV_DEF_COPY_PL2PX_FUNC_2D( int, 32f, 3, ICV_DEF_PX2PL2PX_ENTRY_C3 )
ICV_DEF_COPY_PL2PX_FUNC_2D( int, 32f, 4, ICV_DEF_PX2PL2PX_ENTRY_C4 )
ICV_DEF_COPY_PL2PX_FUNC_2D( int64, 64f, 2, ICV_DEF_PX2PL2PX_ENTRY_C2 )
ICV_DEF_COPY_PL2PX_FUNC_2D( int64, 64f, 3, ICV_DEF_PX2PL2PX_ENTRY_C3 )
ICV_DEF_COPY_PL2PX_FUNC_2D( int64, 64f, 4, ICV_DEF_PX2PL2PX_ENTRY_C4 )

ICV_DEF_COPY_PL2PX_FUNC_2D_COI( uchar, 8u )
ICV_DEF_COPY_PL2PX_FUNC_2D_COI( ushort, 16s )
ICV_DEF_COPY_PL2PX_FUNC_2D_COI( int, 32f )
ICV_DEF_COPY_PL2PX_FUNC_2D_COI( int64, 64f )


#define  ICV_DEF_PXPLPX_TAB( name, FROM, TO )                           \
static void                                                             \
name( CvBigFuncTable* tab )                                             \
{                                                                       \
    tab->fn_2d[CV_8UC2] = (void*)icvCopy##_8u_##FROM##2##TO##2R;        \
    tab->fn_2d[CV_8UC3] = (void*)icvCopy##_8u_##FROM##3##TO##3R;        \
    tab->fn_2d[CV_8UC4] = (void*)icvCopy##_8u_##FROM##4##TO##4R;        \
                                                                        \
    tab->fn_2d[CV_8SC2] = (void*)icvCopy##_8u_##FROM##2##TO##2R;        \
    tab->fn_2d[CV_8SC3] = (void*)icvCopy##_8u_##FROM##3##TO##3R;        \
    tab->fn_2d[CV_8SC4] = (void*)icvCopy##_8u_##FROM##4##TO##4R;        \
                                                                        \
    tab->fn_2d[CV_16UC2] = (void*)icvCopy##_16s_##FROM##2##TO##2R;      \
    tab->fn_2d[CV_16UC3] = (void*)icvCopy##_16s_##FROM##3##TO##3R;      \
    tab->fn_2d[CV_16UC4] = (void*)icvCopy##_16s_##FROM##4##TO##4R;      \
                                                                        \
    tab->fn_2d[CV_16SC2] = (void*)icvCopy##_16s_##FROM##2##TO##2R;      \
    tab->fn_2d[CV_16SC3] = (void*)icvCopy##_16s_##FROM##3##TO##3R;      \
    tab->fn_2d[CV_16SC4] = (void*)icvCopy##_16s_##FROM##4##TO##4R;      \
                                                                        \
    tab->fn_2d[CV_32SC2] = (void*)icvCopy##_32f_##FROM##2##TO##2R;      \
    tab->fn_2d[CV_32SC3] = (void*)icvCopy##_32f_##FROM##3##TO##3R;      \
    tab->fn_2d[CV_32SC4] = (void*)icvCopy##_32f_##FROM##4##TO##4R;      \
                                                                        \
    tab->fn_2d[CV_32FC2] = (void*)icvCopy##_32f_##FROM##2##TO##2R;      \
    tab->fn_2d[CV_32FC3] = (void*)icvCopy##_32f_##FROM##3##TO##3R;      \
    tab->fn_2d[CV_32FC4] = (void*)icvCopy##_32f_##FROM##4##TO##4R;      \
                                                                        \
    tab->fn_2d[CV_64FC2] = (void*)icvCopy##_64f_##FROM##2##TO##2R;      \
    tab->fn_2d[CV_64FC3] = (void*)icvCopy##_64f_##FROM##3##TO##3R;      \
    tab->fn_2d[CV_64FC4] = (void*)icvCopy##_64f_##FROM##4##TO##4R;      \
}



#define  ICV_DEF_PXPLCOI_TAB( name, FROM, TO )                          \
static void                                                             \
name( CvFuncTable* tab )                                                \
{                                                                       \
    tab->fn_2d[CV_8U] = (void*)icvCopy##_8u_##FROM##TO##CR;             \
    tab->fn_2d[CV_8S] = (void*)icvCopy##_8u_##FROM##TO##CR;             \
    tab->fn_2d[CV_16U] = (void*)icvCopy##_16s_##FROM##TO##CR;           \
    tab->fn_2d[CV_16S] = (void*)icvCopy##_16s_##FROM##TO##CR;           \
    tab->fn_2d[CV_32S] = (void*)icvCopy##_32f_##FROM##TO##CR;           \
    tab->fn_2d[CV_32F] = (void*)icvCopy##_32f_##FROM##TO##CR;           \
    tab->fn_2d[CV_64F] = (void*)icvCopy##_64f_##FROM##TO##CR;           \
}


ICV_DEF_PXPLPX_TAB( icvInitSplitRTable, C, P )
ICV_DEF_PXPLCOI_TAB( icvInitSplitRCoiTable, Cn, C1 )
ICV_DEF_PXPLPX_TAB( icvInitCvtPlaneToPixRTable, P, C )
ICV_DEF_PXPLCOI_TAB( icvInitCvtPlaneToPixRCoiTable, C1, Cn )

typedef CvStatus (CV_STDCALL *CvSplitFunc)( const void* src, int srcstep,
                                                    void** dst, int dststep, CvSize size);

typedef CvStatus (CV_STDCALL *CvExtractPlaneFunc)( const void* src, int srcstep,
                                                   void* dst, int dststep,
                                                   CvSize size, int cn, int coi );

typedef CvStatus (CV_STDCALL *CvMergeFunc)( const void** src, int srcstep,
                                                    void* dst, int dststep, CvSize size);

typedef CvStatus (CV_STDCALL *CvInsertPlaneFunc)( const void* src, int srcstep,
                                                  void* dst, int dststep,
                                                  CvSize size, int cn, int coi );

CV_IMPL void
cvSplit( const void* srcarr, void* dstarr0, void* dstarr1, void* dstarr2, void* dstarr3 )
{
    static CvBigFuncTable  pxpl_tab;
    static CvFuncTable  pxplcoi_tab;
    static int inittab = 0;

    CV_FUNCNAME( "cvSplit" );

    __BEGIN__;

    CvMat stub[5], *dst[4], *src = (CvMat*)srcarr;
    CvSize size;
    void* dstptr[4] = { 0, 0, 0, 0 };
    int type, cn, coi = 0;
    int i, nzplanes = 0, nzidx = -1;
    int cont_flag;
    int src_step, dst_step = 0;

    if( !inittab )
    {
        icvInitSplitRTable( &pxpl_tab );
        icvInitSplitRCoiTable( &pxplcoi_tab );
        inittab = 1;
    }

    dst[0] = (CvMat*)dstarr0;
    dst[1] = (CvMat*)dstarr1;
    dst[2] = (CvMat*)dstarr2;
    dst[3] = (CvMat*)dstarr3;

    CV_CALL( src = cvGetMat( src, stub + 4, &coi ));

    //if( coi != 0 )
    //    CV_ERROR( CV_BadCOI, "" );

    type = CV_MAT_TYPE( src->type );
    cn = CV_MAT_CN( type );

    cont_flag = src->type;

    if( cn == 1 )
        CV_ERROR( CV_BadNumChannels, "" );

    for( i = 0; i < 4; i++ )
    {
        if( dst[i] )
        {
            nzplanes++;
            nzidx = i;
            CV_CALL( dst[i] = cvGetMat( dst[i], stub + i ));
            if( CV_MAT_CN( dst[i]->type ) != 1 )
                CV_ERROR( CV_BadNumChannels, "" );
            if( !CV_ARE_DEPTHS_EQ( dst[i], src ))
                CV_ERROR( CV_StsUnmatchedFormats, "" );
            if( !CV_ARE_SIZES_EQ( dst[i], src ))
                CV_ERROR( CV_StsUnmatchedSizes, "" );
            if( nzplanes > i && i > 0 && dst[i]->step != dst[i-1]->step )
                CV_ERROR( CV_BadStep, "" );
            dst_step = dst[i]->step;
            dstptr[nzplanes-1] = dst[i]->data.ptr;

            cont_flag &= dst[i]->type;
        }
    }

    src_step = src->step;
    size = cvGetMatSize( src );

    if( CV_IS_MAT_CONT( cont_flag ))
    {
        size.width *= size.height;
        src_step = dst_step = CV_STUB_STEP;

        size.height = 1;
    }

    if( nzplanes == cn )
    {
        CvSplitFunc func = (CvSplitFunc)pxpl_tab.fn_2d[type];

        if( !func )
            CV_ERROR( CV_StsUnsupportedFormat, "" );

        IPPI_CALL( func( src->data.ptr, src_step, dstptr, dst_step, size ));
    }
    else if( nzplanes == 1 )
    {
        CvExtractPlaneFunc func = (CvExtractPlaneFunc)pxplcoi_tab.fn_2d[CV_MAT_DEPTH(type)];

        if( !func )
            CV_ERROR( CV_StsUnsupportedFormat, "" );

        IPPI_CALL( func( src->data.ptr, src_step,
                         dst[nzidx]->data.ptr, dst_step,
                         size, cn, nzidx + 1 ));
    }
    else
    {
        CV_ERROR( CV_StsBadArg,
            "Either all output planes or only one output plane should be non zero" );
    }

    __END__;
}



CV_IMPL void
cvMerge( const void* srcarr0, const void* srcarr1, const void* srcarr2,
         const void* srcarr3, void* dstarr )
{
    static CvBigFuncTable plpx_tab;
    static CvFuncTable plpxcoi_tab;
    static int inittab = 0;

    CV_FUNCNAME( "cvMerge" );

    __BEGIN__;

    int src_step = 0, dst_step;
    CvMat stub[5], *src[4], *dst = (CvMat*)dstarr;
    CvSize size;
    const void* srcptr[4] = { 0, 0, 0, 0 };
    int type, cn, coi = 0;
    int i, nzplanes = 0, nzidx = -1;
    int cont_flag;

    if( !inittab )
    {
        icvInitCvtPlaneToPixRTable( &plpx_tab );
        icvInitCvtPlaneToPixRCoiTable( &plpxcoi_tab );
        inittab = 1;
    }

    src[0] = (CvMat*)srcarr0;
    src[1] = (CvMat*)srcarr1;
    src[2] = (CvMat*)srcarr2;
    src[3] = (CvMat*)srcarr3;

    CV_CALL( dst = cvGetMat( dst, stub + 4, &coi ));

    type = CV_MAT_TYPE( dst->type );
    cn = CV_MAT_CN( type );

    cont_flag = dst->type;

    if( cn == 1 )
        CV_ERROR( CV_BadNumChannels, "" );

    for( i = 0; i < 4; i++ )
    {
        if( src[i] )
        {
            nzplanes++;
            nzidx = i;
            CV_CALL( src[i] = cvGetMat( src[i], stub + i ));
            if( CV_MAT_CN( src[i]->type ) != 1 )
                CV_ERROR( CV_BadNumChannels, "" );
            if( !CV_ARE_DEPTHS_EQ( src[i], dst ))
                CV_ERROR( CV_StsUnmatchedFormats, "" );
            if( !CV_ARE_SIZES_EQ( src[i], dst ))
                CV_ERROR( CV_StsUnmatchedSizes, "" );
            if( nzplanes > i && i > 0 && src[i]->step != src[i-1]->step )
                CV_ERROR( CV_BadStep, "" );
            src_step = src[i]->step;
            srcptr[nzplanes-1] = (const void*)(src[i]->data.ptr);

            cont_flag &= src[i]->type;
        }
    }

    size = cvGetMatSize( dst );
    dst_step = dst->step;

    if( CV_IS_MAT_CONT( cont_flag ))
    {
        size.width *= size.height;
        src_step = dst_step = CV_STUB_STEP;
        size.height = 1;
    }

    if( nzplanes == cn )
    {
        CvMergeFunc func = (CvMergeFunc)plpx_tab.fn_2d[type];

        if( !func )
            CV_ERROR( CV_StsUnsupportedFormat, "" );

        IPPI_CALL( func( srcptr, src_step, dst->data.ptr, dst_step, size ));
    }
    else if( nzplanes == 1 )
    {
        CvInsertPlaneFunc func = (CvInsertPlaneFunc)plpxcoi_tab.fn_2d[CV_MAT_DEPTH(type)];

        if( !func )
            CV_ERROR( CV_StsUnsupportedFormat, "" );

        IPPI_CALL( func( src[nzidx]->data.ptr, src_step,
                         dst->data.ptr, dst_step,
                         size, cn, nzidx + 1 ));
    }
    else
    {
        CV_ERROR( CV_StsBadArg,
            "Either all input planes or only one input plane should be non zero" );
    }

    __END__;
}



/****************************************************************************************\
*                                   cvConvertScaleAbs                                    *
\****************************************************************************************/

#define ICV_DEF_CVT_SCALE_ABS_CASE( worktype, cast_macro1,              \
                                    scale_macro, abs_macro,             \
                                    cast_macro2, src, a, b )            \
                                                                        \
{                                                                       \
    for( ; size.height--; (char*&)(src) += srcstep,                     \
                          (char*&)(dst) += dststep )                    \
    {                                                                   \
        int i;                                                          \
                                                                        \
        for( i = 0; i <= size.width - 4; i += 4 )                       \
        {                                                               \
            worktype t0 = scale_macro((a)*cast_macro1((src)[i]) + (b)); \
            worktype t1 = scale_macro((a)*cast_macro1((src)[i+1])+(b)); \
                                                                        \
            t0 = (worktype)abs_macro(t0);                               \
            t1 = (worktype)abs_macro(t1);                               \
                                                                        \
            dst[i] = cast_macro2(t0);                                   \
            dst[i+1] = cast_macro2(t1);                                 \
                                                                        \
            t0 = scale_macro((a)*cast_macro1((src)[i+2]) + (b));        \
            t1 = scale_macro((a)*cast_macro1((src)[i+3]) + (b));        \
                                                                        \
            t0 = (worktype)abs_macro(t0);                               \
            t1 = (worktype)abs_macro(t1);                               \
                                                                        \
            dst[i+2] = cast_macro2(t0);                                 \
            dst[i+3] = cast_macro2(t1);                                 \
        }                                                               \
                                                                        \
        for( ; i < size.width; i++ )                                    \
        {                                                               \
            worktype t0 = scale_macro((a)*cast_macro1((src)[i]) + (b)); \
            t0 = (worktype)abs_macro(t0);                               \
            dst[i] = cast_macro2(t0);                                   \
        }                                                               \
    }                                                                   \
}


#define ICV_FIX_SHIFT  15
#define ICV_SCALE(x)   (((x) + (1 << (ICV_FIX_SHIFT-1))) >> ICV_FIX_SHIFT)

static CvStatus CV_STDCALL
icvCvtScaleAbsTo_8u_C1R( const uchar* src, int srcstep,
                         uchar* dst, int dststep,
                         CvSize size, double scale, double shift,
                         int param )
{
    int srctype = param;

    size.width *= CV_MAT_CN(srctype);

    switch( CV_MAT_DEPTH(srctype) )
    {
    case  CV_8S:
        if( fabs( scale ) <= 128. &&
            fabs( shift ) <= (INT_MAX*0.5)/(1 << ICV_FIX_SHIFT))
        {
            int iscale = cvRound(scale*(1 << ICV_FIX_SHIFT));
            int ishift = cvRound(shift*(1 << ICV_FIX_SHIFT));

            if( iscale == ICV_FIX_SHIFT && ishift == 0 )
            {
                ICV_DEF_CVT_SCALE_ABS_CASE( int, CV_NOP, CV_NOP, CV_IABS,
                                            CV_CAST_8U, (char*&)src, 1, 0 )
            }
            else
            {
                ICV_DEF_CVT_SCALE_ABS_CASE( int, CV_NOP, ICV_SCALE, CV_IABS,
                                       CV_CAST_8U, (char*&)src, iscale, ishift )
            }
        }
        else
        {
            ICV_DEF_CVT_SCALE_ABS_CASE( int, CV_NOP, cvRound, CV_IABS,
                                   CV_CAST_8U, (char*&)src, scale, shift )
        }
        break;
    case  CV_8U:
        if( fabs( scale ) <= 128. &&
            fabs( shift ) <= (INT_MAX*0.5)/(1 << ICV_FIX_SHIFT))
        {
            int iscale = cvRound(scale*(1 << ICV_FIX_SHIFT));
            int ishift = cvRound(shift*(1 << ICV_FIX_SHIFT));

            if( iscale == ICV_FIX_SHIFT && ishift == 0 )
            {
                icvCopy_8u_C1R( src, srcstep, dst, dststep, size );
            }
            else
            {
                ICV_DEF_CVT_SCALE_ABS_CASE( int, CV_NOP, ICV_SCALE, CV_IABS,
                                   CV_CAST_8U, (uchar*&)src, iscale, ishift );
            }
        }
        else
        {
            ICV_DEF_CVT_SCALE_ABS_CASE( int, CV_NOP, cvRound, CV_IABS,
                                   CV_CAST_8U, (uchar*&)src, scale, shift );
        }
        break;
    case  CV_16U:
        if( fabs( scale ) <= 1. && fabs(shift) < DBL_EPSILON )
        {
            int iscale = cvRound(scale*(1 << ICV_FIX_SHIFT));

            if( iscale == ICV_FIX_SHIFT )
            {
                ICV_DEF_CVT_SCALE_ABS_CASE( int, CV_NOP, CV_NOP, CV_IABS,
                                       CV_CAST_8U, (ushort*&)src, 1, 0 );
            }
            else
            {
                ICV_DEF_CVT_SCALE_ABS_CASE( int, CV_NOP, ICV_SCALE, CV_IABS,
                                       CV_CAST_8U, (ushort*&)src, iscale, 0 );
            }
        }
        else
        {
            ICV_DEF_CVT_SCALE_ABS_CASE( int, CV_NOP, cvRound, CV_IABS,
                                   CV_CAST_8U, (ushort*&)src, scale, shift );
        }
        break;
    case  CV_16S:
        if( fabs( scale ) <= 1. &&
            fabs( shift ) <= (INT_MAX*0.5)/(1 << ICV_FIX_SHIFT))
        {
            int iscale = cvRound(scale*(1 << ICV_FIX_SHIFT));
            int ishift = cvRound(shift*(1 << ICV_FIX_SHIFT));

            if( iscale == ICV_FIX_SHIFT && ishift == 0 )
            {
                ICV_DEF_CVT_SCALE_ABS_CASE( int, CV_NOP, CV_NOP, CV_IABS,
                                       CV_CAST_8U, (short*&)src, 1, 0 );
            }
            else
            {
                ICV_DEF_CVT_SCALE_ABS_CASE( int, CV_NOP, ICV_SCALE, CV_IABS,
                                    CV_CAST_8U, (short*&)src, iscale, ishift );
            }
        }
        else
        {
            ICV_DEF_CVT_SCALE_ABS_CASE( int, CV_NOP, cvRound, CV_IABS,
                                   CV_CAST_8U, (short*&)src, scale, shift );
        }
        break;
    case  CV_32S:
        ICV_DEF_CVT_SCALE_ABS_CASE( int, CV_NOP, cvRound, CV_IABS,
                                    CV_CAST_8U, (int*&)src, scale, shift );
        break;
    case  CV_32F:
        ICV_DEF_CVT_SCALE_ABS_CASE( int, CV_NOP, cvRound, CV_IABS,
                                    CV_CAST_8U, (float*&)src, scale, shift );
        break;
    case  CV_64F:
        ICV_DEF_CVT_SCALE_ABS_CASE( int, CV_NOP, cvRound, CV_IABS,
                                    CV_CAST_8U, (double*&)src, scale, shift );
        break;
    default:
        assert(0);
        return CV_BADFLAG_ERR;
    }

    return  CV_OK;
}


CV_IMPL void
cvConvertScaleAbs( const void* srcarr, void* dstarr,
                   double scale, double shift )
{
    CV_FUNCNAME( "cvConvertScaleAbs" );

    __BEGIN__;

    int coi1 = 0, coi2 = 0;
    CvMat  srcstub, *src = (CvMat*)srcarr;
    CvMat  dststub, *dst = (CvMat*)dstarr;
    CvSize size;
    int src_step, dst_step;

    CV_CALL( src = cvGetMat( src, &srcstub, &coi1 ));
    CV_CALL( dst = cvGetMat( dst, &dststub, &coi2 ));

    if( coi1 != 0 || coi2 != 0 )
        CV_ERROR( CV_BadCOI, "" );

    if( !CV_ARE_SIZES_EQ( src, dst ))
        CV_ERROR( CV_StsUnmatchedSizes, "" );

    if( !CV_ARE_CNS_EQ( src, dst ))
        CV_ERROR( CV_StsUnmatchedFormats, "" );

    if( CV_MAT_DEPTH( dst->type ) != CV_8U )
        CV_ERROR( CV_StsUnsupportedFormat, "" );

    size = cvGetMatSize( src );
    src_step = src->step;
    dst_step = dst->step;

    if( CV_IS_MAT_CONT( src->type & dst->type ))
    {
        size.width *= size.height;
        src_step = dst_step = CV_STUB_STEP;
        size.height = 1;
    }

    IPPI_CALL( icvCvtScaleAbsTo_8u_C1R( src->data.ptr, src_step,
                             (uchar*)(dst->data.ptr), dst_step,
                             size, scale, shift, CV_MAT_TYPE(src->type)));
    __END__;
}

/****************************************************************************************\
*                                      cvConvertScale                                    *
\****************************************************************************************/

#define ICV_DEF_CVT_SCALE_CASE( worktype, cast_macro1,                  \
                                scale_macro, cast_macro2, src, a, b )   \
                                                                        \
{                                                                       \
    for( ; size.height--; (char*&)(src) += srcstep,                     \
                          (char*&)(dst) += dststep )                    \
    {                                                                   \
        for( i = 0; i <= size.width - 4; i += 4 )                       \
        {                                                               \
            worktype t0 = scale_macro((a)*cast_macro1((src)[i])+(b));   \
            worktype t1 = scale_macro((a)*cast_macro1((src)[i+1])+(b)); \
                                                                        \
            dst[i] = cast_macro2(t0);                                   \
            dst[i+1] = cast_macro2(t1);                                 \
                                                                        \
            t0 = scale_macro((a)*cast_macro1((src)[i+2]) + (b));        \
            t1 = scale_macro((a)*cast_macro1((src)[i+3]) + (b));        \
                                                                        \
            dst[i+2] = cast_macro2(t0);                                 \
            dst[i+3] = cast_macro2(t1);                                 \
        }                                                               \
                                                                        \
        for( ; i < size.width; i++ )                                    \
        {                                                               \
            worktype t0 = scale_macro((a)*cast_macro1((src)[i]) + (b)); \
            dst[i] = cast_macro2(t0);                                   \
        }                                                               \
    }                                                                   \
}


#define  ICV_DEF_CVT_SCALE_FUNC_INT( flavor, dsttype, cast_macro )      \
static  CvStatus  CV_STDCALL                                            \
icvCvtScaleTo_##flavor##_C1R( const char* src, int srcstep,             \
                              dsttype* dst, int dststep, CvSize size,   \
                              double scale, double shift, int param )   \
{                                                                       \
    int i, srctype = param;                                             \
    dsttype lut[256];                                                   \
                                                                        \
    switch( CV_MAT_DEPTH(srctype) )                                     \
    {                                                                   \
    case  CV_8U:                                                        \
        if( size.width*size.height >= 256 )                             \
        {                                                               \
            for( i = 0; i < 256; i++ )                                  \
            {                                                           \
                int t = cvRound( i*scale + shift );                     \
                lut[i] = cast_macro(t);                                 \
            }                                                           \
                                                                        \
            icvLUT_Transform8u_##flavor##_C1R( src, srcstep, dst,       \
                                               dststep, size, lut );    \
        }                                                               \
        else if( fabs( scale ) <= 128. &&                               \
                 fabs( shift ) <= (INT_MAX*0.5)/(1 << ICV_FIX_SHIFT))   \
        {                                                               \
            int iscale = cvRound(scale*(1 << ICV_FIX_SHIFT));           \
            int ishift = cvRound(shift*(1 << ICV_FIX_SHIFT));           \
                                                                        \
            ICV_DEF_CVT_SCALE_CASE( int, CV_NOP, ICV_SCALE, cast_macro, \
                                    (uchar*&)src, iscale, ishift );     \
        }                                                               \
        else                                                            \
        {                                                               \
            ICV_DEF_CVT_SCALE_CASE( int, CV_NOP, cvRound, cast_macro,   \
                                    (uchar*&)src, scale, shift );       \
        }                                                               \
        break;                                                          \
    case  CV_8S:                                                        \
        if( size.width*size.height >= 256 )                             \
        {                                                               \
            for( i = 0; i < 256; i++ )                                  \
            {                                                           \
                int t = cvRound( (char)i*scale + shift );               \
                lut[i] = cast_macro(t);                                 \
            }                                                           \
                                                                        \
            icvLUT_Transform8u_##flavor##_C1R( src, srcstep, dst,       \
                                               dststep, size, lut );    \
        }                                                               \
        else if( fabs( scale ) <= 128. &&                               \
                 fabs( shift ) <= (INT_MAX*0.5)/(1 << ICV_FIX_SHIFT))   \
        {                                                               \
            int iscale = cvRound(scale*(1 << ICV_FIX_SHIFT));           \
            int ishift = cvRound(shift*(1 << ICV_FIX_SHIFT));           \
                                                                        \
            ICV_DEF_CVT_SCALE_CASE( int, CV_NOP, ICV_SCALE, cast_macro, \
                                    (char*&)src, iscale, ishift );      \
        }                                                               \
        else                                                            \
        {                                                               \
            ICV_DEF_CVT_SCALE_CASE( int, CV_NOP, cvRound, cast_macro,   \
                                    (char*&)src, scale, shift );        \
        }                                                               \
        break;                                                          \
    case  CV_16U:                                                       \
        if( fabs( scale ) <= 1. && shift == 0 )                         \
        {                                                               \
            int iscale = cvRound(scale*(1 << ICV_FIX_SHIFT));           \
                                                                        \
            ICV_DEF_CVT_SCALE_CASE( int, CV_NOP, ICV_SCALE, cast_macro, \
                                    (ushort*&)src, iscale, 0 );         \
        }                                                               \
        else                                                            \
        {                                                               \
            ICV_DEF_CVT_SCALE_CASE( int, CV_NOP, cvRound, cast_macro,   \
                                    (ushort*&)src, scale, shift );      \
        }                                                               \
        break;                                                          \
    case  CV_16S:                                                       \
        if( fabs( scale ) <= 1. &&                                      \
            fabs( shift ) <= (INT_MAX*0.5)/(1 << ICV_FIX_SHIFT))        \
        {                                                               \
            int iscale = cvRound(scale*(1 << ICV_FIX_SHIFT));           \
            int ishift = cvRound(shift*(1 << ICV_FIX_SHIFT));           \
                                                                        \
            ICV_DEF_CVT_SCALE_CASE( int, CV_NOP, ICV_SCALE, cast_macro, \
                                    (short*&)src, iscale, ishift );     \
        }                                                               \
        else                                                            \
        {                                                               \
            ICV_DEF_CVT_SCALE_CASE( int, CV_NOP, cvRound, cast_macro,   \
                                    (short*&)src, scale, shift );       \
        }                                                               \
        break;                                                          \
    case  CV_32S:                                                       \
        ICV_DEF_CVT_SCALE_CASE( int, CV_NOP, cvRound, cast_macro,       \
                                (int*&)src, scale, shift )              \
        break;                                                          \
    case  CV_32F:                                                       \
        ICV_DEF_CVT_SCALE_CASE( int, CV_NOP, cvRound, cast_macro,       \
                                (float*&)src, scale, shift )            \
        break;                                                          \
    case  CV_64F:                                                       \
        ICV_DEF_CVT_SCALE_CASE( int, CV_NOP, cvRound, cast_macro,       \
                                (double*&)src, scale, shift )           \
        break;                                                          \
    default:                                                            \
        assert(0);                                                      \
        return CV_BADFLAG_ERR;                                          \
    }                                                                   \
                                                                        \
    return  CV_OK;                                                      \
}


#define  ICV_DEF_CVT_SCALE_FUNC_FLT( flavor, dsttype, cast_macro )      \
static  CvStatus  CV_STDCALL                                            \
icvCvtScaleTo_##flavor##_C1R( const char* src, int srcstep,             \
                              dsttype* dst, int dststep, CvSize size,   \
                              double scale, double shift, int param )   \
{                                                                       \
    int i, srctype = param;                                             \
    dsttype lut[256];                                                   \
                                                                        \
    switch( CV_MAT_DEPTH(srctype) )                                     \
    {                                                                   \
    case  CV_8U:                                                        \
        if( size.width*size.height >= 256 )                             \
        {                                                               \
            for( i = 0; i < 256; i++ )                                  \
            {                                                           \
                lut[i] = (dsttype)(i*scale + shift);                    \
            }                                                           \
                                                                        \
            icvLUT_Transform8u_##flavor##_C1R( src, srcstep, dst,       \
                                               dststep, size, lut );    \
        }                                                               \
        else                                                            \
        {                                                               \
            ICV_DEF_CVT_SCALE_CASE( dsttype, CV_8TO32F, cast_macro,     \
                                CV_NOP, (uchar*&)src, scale, shift );   \
        }                                                               \
        break;                                                          \
    case  CV_8S:                                                        \
        if( size.width*size.height >= 256 )                             \
        {                                                               \
            for( i = 0; i < 256; i++ )                                  \
            {                                                           \
                lut[i] = (dsttype)((char)i*scale + shift);              \
            }                                                           \
                                                                        \
            icvLUT_Transform8u_##flavor##_C1R( src, srcstep, dst,       \
                                               dststep, size, lut );    \
        }                                                               \
        else                                                            \
        {                                                               \
            ICV_DEF_CVT_SCALE_CASE( dsttype, CV_8TO32F, cast_macro,     \
                                CV_NOP, (char*&)src, scale, shift );    \
        }                                                               \
        break;                                                          \
    case  CV_16U:                                                       \
        ICV_DEF_CVT_SCALE_CASE( dsttype, CV_NOP, cast_macro, CV_NOP,    \
                                (ushort*&)src, scale, shift );          \
        break;                                                          \
    case  CV_16S:                                                       \
        ICV_DEF_CVT_SCALE_CASE( dsttype, CV_NOP, cast_macro, CV_NOP,    \
                                (short*&)src, scale, shift );           \
        break;                                                          \
    case  CV_32S:                                                       \
        ICV_DEF_CVT_SCALE_CASE( dsttype, CV_NOP, cast_macro, CV_NOP,    \
                                (int*&)src, scale, shift )              \
        break;                                                          \
    case  CV_32F:                                                       \
        ICV_DEF_CVT_SCALE_CASE( dsttype, CV_NOP, cast_macro, CV_NOP,    \
                                (float*&)src, scale, shift )            \
        break;                                                          \
    case  CV_64F:                                                       \
        ICV_DEF_CVT_SCALE_CASE( dsttype, CV_NOP, cast_macro, CV_NOP,    \
                                (double*&)src, scale, shift )           \
        break;                                                          \
    default:                                                            \
        assert(0);                                                      \
        return CV_BADFLAG_ERR;                                          \
    }                                                                   \
                                                                        \
    return  CV_OK;                                                      \
}


ICV_DEF_CVT_SCALE_FUNC_INT( 8u, uchar, CV_CAST_8U )
ICV_DEF_CVT_SCALE_FUNC_INT( 8s, char, CV_CAST_8S )
ICV_DEF_CVT_SCALE_FUNC_INT( 16s, short, CV_CAST_16S )
ICV_DEF_CVT_SCALE_FUNC_INT( 16u, ushort, CV_CAST_16U )
ICV_DEF_CVT_SCALE_FUNC_INT( 32s, int, CV_CAST_32S )

ICV_DEF_CVT_SCALE_FUNC_FLT( 32f, float, CV_CAST_32F )
ICV_DEF_CVT_SCALE_FUNC_FLT( 64f, double, CV_CAST_64F )

CV_DEF_INIT_FUNC_TAB_2D( CvtScaleTo, C1R )


/****************************************************************************************\
*                             Conversion w/o scaling macros                              *
\****************************************************************************************/

#define ICV_DEF_CVT_CASE_2D( src_type, dst_type, work_type, cast_macro1, cast_macro2, \
                             src_ptr, src_step, dst_ptr, dst_step, size )           \
{                                                                                   \
    for( ; (size).height--; (src_ptr) += (src_step), (dst_ptr) += (dst_step))       \
    {                                                                               \
        int i;                                                                      \
                                                                                    \
        for( i = 0; i <= (size).width - 4; i += 4 )                                 \
        {                                                                           \
            work_type t0 = cast_macro1(((src_type*)(src_ptr))[i]);                  \
            work_type t1 = cast_macro1(((src_type*)(src_ptr))[i+1]);                \
                                                                                    \
            ((dst_type*)(dst_ptr))[i] = cast_macro2(t0);                            \
            ((dst_type*)(dst_ptr))[i+1] = cast_macro2(t1);                          \
                                                                                    \
            t0 = cast_macro1(((src_type*)(src_ptr))[i+2]);                          \
            t1 = cast_macro1(((src_type*)(src_ptr))[i+3]);                          \
                                                                                    \
            ((dst_type*)(dst_ptr))[i+2] = cast_macro2(t0);                          \
            ((dst_type*)(dst_ptr))[i+3] = cast_macro2(t1);                          \
        }                                                                           \
                                                                                    \
        for( ; i < (size).width; i++ )                                              \
        {                                                                           \
            work_type t0 = cast_macro1(((src_type*)(src_ptr))[i]);                  \
            ((dst_type*)(dst_ptr))[i] = cast_macro2(t0);                            \
        }                                                                           \
    }                                                                               \
}


#define ICV_DEF_CVT_FUNC_2D( flavor, dst_type, work_type, cast_macro2,  \
                             src_depth1, src_type1, cast_macro11,       \
                             src_depth2, src_type2, cast_macro12,       \
                             src_depth3, src_type3, cast_macro13,       \
                             src_depth4, src_type4, cast_macro14,       \
                             src_depth5, src_type5, cast_macro15,       \
                             src_depth6, src_type6, cast_macro16 )      \
static CvStatus CV_STDCALL                                              \
icvCvtTo_##flavor##_C1R( const void* pSrc, int step1,                   \
                         void* pDst, int step,                          \
                         CvSize size, int param )                       \
{                                                                       \
    int srctype = param;                                                \
    const char* src = (const char*)pSrc;                                \
    char* dst = (char*)pDst;                                            \
                                                                        \
    switch( CV_MAT_DEPTH(srctype) )                                     \
    {                                                                   \
    case src_depth1:                                                    \
        ICV_DEF_CVT_CASE_2D( src_type1, dst_type, work_type,            \
                             cast_macro11, cast_macro2,                 \
                             src, step1, dst, step, size );             \
        break;                                                          \
    case src_depth2:                                                    \
        ICV_DEF_CVT_CASE_2D( src_type2, dst_type, work_type,            \
                             cast_macro12, cast_macro2,                 \
                             src, step1, dst, step, size );             \
        break;                                                          \
    case src_depth3:                                                    \
        ICV_DEF_CVT_CASE_2D( src_type3, dst_type, work_type,            \
                             cast_macro13, cast_macro2,                 \
                             src, step1, dst, step, size );             \
        break;                                                          \
    case src_depth4:                                                    \
        ICV_DEF_CVT_CASE_2D( src_type4, dst_type, work_type,            \
                             cast_macro14, cast_macro2,                 \
                             src, step1, dst, step, size );             \
        break;                                                          \
    case src_depth5:                                                    \
        ICV_DEF_CVT_CASE_2D( src_type5, dst_type, work_type,            \
                             cast_macro15, cast_macro2,                 \
                             src, step1, dst, step, size );             \
        break;                                                          \
    case src_depth6:                                                    \
        ICV_DEF_CVT_CASE_2D( src_type6, dst_type, work_type,            \
                             cast_macro16, cast_macro2,                 \
                             src, step1, dst, step, size );             \
        break;                                                          \
    }                                                                   \
                                                                        \
    return  CV_OK;                                                      \
}


ICV_DEF_CVT_FUNC_2D( 8u, uchar, int, CV_CAST_8U,
                     CV_8S,  char,   CV_NOP,
                     CV_16U, ushort, CV_NOP,
                     CV_16S, short,  CV_NOP,
                     CV_32S, int,    CV_NOP,
                     CV_32F, float,  cvRound,
                     CV_64F, double, cvRound )

ICV_DEF_CVT_FUNC_2D( 8s, char, int, CV_CAST_8S,
                     CV_8U,  uchar,  CV_NOP,
                     CV_16U, ushort, CV_NOP,
                     CV_16S, short,  CV_NOP,
                     CV_32S, int,    CV_NOP,
                     CV_32F, float,  cvRound,
                     CV_64F, double, cvRound )

ICV_DEF_CVT_FUNC_2D( 16u, ushort, int, CV_CAST_16U,
                     CV_8U,  uchar,  CV_NOP,
                     CV_8S,  char,   CV_NOP,
                     CV_16S, short,  CV_NOP,
                     CV_32S, int,    CV_NOP,
                     CV_32F, float,  cvRound,
                     CV_64F, double, cvRound )

ICV_DEF_CVT_FUNC_2D( 16s, short, int, CV_CAST_16S,
                     CV_8U,  uchar,  CV_NOP,
                     CV_8S,  char,   CV_NOP,
                     CV_16U, ushort, CV_NOP,
                     CV_32S, int,    CV_NOP,
                     CV_32F, float,  cvRound,
                     CV_64F, double, cvRound )

ICV_DEF_CVT_FUNC_2D( 32s, int, int, CV_NOP,
                     CV_8U,  uchar,  CV_NOP,
                     CV_8S,  char,   CV_NOP,
                     CV_16U, ushort, CV_NOP,
                     CV_16S, short,  CV_NOP,
                     CV_32F, float,  cvRound,
                     CV_64F, double, cvRound )

ICV_DEF_CVT_FUNC_2D( 32f, float, float, CV_NOP,
                     CV_8U,  uchar,  CV_8TO32F,
                     CV_8S,  char,   CV_8TO32F,
                     CV_16U, ushort, CV_NOP,
                     CV_16S, short,  CV_NOP,
                     CV_32S, int,    CV_CAST_32F,
                     CV_64F, double, CV_CAST_32F )

ICV_DEF_CVT_FUNC_2D( 64f, double, double, CV_NOP,
                     CV_8U,  uchar,  CV_8TO32F,
                     CV_8S,  char,   CV_8TO32F,
                     CV_16U, ushort, CV_NOP,
                     CV_16S, short,  CV_NOP,
                     CV_32S, int,    CV_NOP,
                     CV_32F, float,  CV_NOP )

CV_DEF_INIT_FUNC_TAB_2D( CvtTo, C1R )


typedef  CvStatus (CV_STDCALL *CvCvtFunc)( const void* src, int srcstep,
                                           void* dst, int dststep, CvSize size,
                                           int param );

typedef  CvStatus (CV_STDCALL *CvCvtScaleFunc)( const void* src, int srcstep,
                                             void* dst, int dststep, CvSize size,
                                             double scale, double shift,
                                             int param );

CV_IMPL void
cvConvertScale( const void* srcarr, void* dstarr,
                double scale, double shift )
{
    static CvFuncTable cvt_tab, cvtscale_tab;
    static int inittab = 0;

    CV_FUNCNAME( "cvConvertScale" );

    __BEGIN__;

    int type;
    int is_nd = 0;
    CvMat  srcstub, *src = (CvMat*)srcarr;
    CvMat  dststub, *dst = (CvMat*)dstarr;
    CvSize size;
    int src_step, dst_step;

    if( !CV_IS_MAT(src) )
    {
        if( CV_IS_MATND(src) )
            is_nd = 1;
        else
        {
            int coi = 0;
            CV_CALL( src = cvGetMat( src, &srcstub, &coi ));

            if( coi != 0 )
                CV_ERROR( CV_BadCOI, "" );
        }
    }

    if( !CV_IS_MAT(dst) )
    {
        if( CV_IS_MATND(dst) )
            is_nd = 1;
        else
        {
            int coi = 0;
            CV_CALL( dst = cvGetMat( dst, &dststub, &coi ));

            if( coi != 0 )
                CV_ERROR( CV_BadCOI, "" );
        }
    }

    if( is_nd )
    {
        CvArr* arrs[] = { src, dst };
        CvMatND stubs[2];
        CvNArrayIterator iterator;
        int dsttype;

        CV_CALL( cvInitNArrayIterator( 2, arrs, 0, stubs, &iterator, CV_NO_DEPTH_CHECK ));

        type = iterator.hdr[0]->type;
        dsttype = iterator.hdr[1]->type;
        iterator.size.width *= CV_MAT_CN(type);

        if( !inittab )
        {
            icvInitCvtToC1RTable( &cvt_tab );
            icvInitCvtScaleToC1RTable( &cvtscale_tab );
            inittab = 1;
        }

        if( scale == 1 && shift == 0 )
        {
            CvCvtFunc func = (CvCvtFunc)(cvt_tab.fn_2d[CV_MAT_DEPTH(dsttype)]);
            if( !func )
                CV_ERROR( CV_StsUnsupportedFormat, "" );

            do
            {
                IPPI_CALL( func( iterator.ptr[0], CV_STUB_STEP,
                                 iterator.ptr[1], CV_STUB_STEP,
                                 iterator.size, type ));
            }
            while( cvNextNArraySlice( &iterator ));
        }
        else
        {
            CvCvtScaleFunc func =
                (CvCvtScaleFunc)(cvtscale_tab.fn_2d[CV_MAT_DEPTH(dsttype)]);
            if( !func )
                CV_ERROR( CV_StsUnsupportedFormat, "" );

            do
            {
                IPPI_CALL( func( iterator.ptr[0], CV_STUB_STEP,
                                 iterator.ptr[1], CV_STUB_STEP,
                                 iterator.size, scale, shift, type ));
            }
            while( cvNextNArraySlice( &iterator ));
        }
        EXIT;
    }

    if( !CV_ARE_SIZES_EQ( src, dst ))
        CV_ERROR( CV_StsUnmatchedSizes, "" );

    size = cvGetMatSize( src );
    type = CV_MAT_TYPE(src->type);
    src_step = src->step;
    dst_step = dst->step;

    if( CV_IS_MAT_CONT( src->type & dst->type ))
    {
        size.width *= size.height;
        src_step = dst_step = CV_STUB_STEP;
        size.height = 1;
    }

    size.width *= CV_MAT_CN( type );

    if( CV_ARE_TYPES_EQ( src, dst ) && size.height == 1 &&
        size.width <= CV_MAX_INLINE_MAT_OP_SIZE )
    {
        if( CV_MAT_DEPTH(type) == CV_32F )
        {
            const float* srcdata = (const float*)(src->data.ptr);
            float* dstdata = (float*)(dst->data.ptr);

            do
            {
                dstdata[size.width - 1] = (float)(srcdata[size.width-1]*scale + shift);
            }
            while( --size.width );

            EXIT;
        }

        if( CV_MAT_DEPTH(type) == CV_64F )
        {
            const double* srcdata = (const double*)(src->data.ptr);
            double* dstdata = (double*)(dst->data.ptr);

            do
            {
                dstdata[size.width - 1] = srcdata[size.width-1]*scale + shift;
            }
            while( --size.width );

            EXIT;
        }
    }

    if( !inittab )
    {
        icvInitCvtToC1RTable( &cvt_tab );
        icvInitCvtScaleToC1RTable( &cvtscale_tab );
        inittab = 1;
    }

    if( !CV_ARE_CNS_EQ( src, dst ))
        CV_ERROR( CV_StsUnmatchedFormats, "" );

    if( scale == 1 && shift == 0 )
    {
        if( !CV_ARE_DEPTHS_EQ( src, dst ))
        {
            CvCvtFunc func = (CvCvtFunc)(cvt_tab.fn_2d[CV_MAT_DEPTH(dst->type)]);

            if( !func )
                CV_ERROR( CV_StsUnsupportedFormat, "" );

            IPPI_CALL( func( src->data.ptr, src_step,
                       dst->data.ptr, dst_step, size, type ));
        }
        else
            cvCopy( src, dst );
    }
    else
    {
        CvCvtScaleFunc func = (CvCvtScaleFunc)
            (cvtscale_tab.fn_2d[CV_MAT_DEPTH(dst->type)]);

        if( !func )
            CV_ERROR( CV_StsUnsupportedFormat, "" );

        IPPI_CALL( func( src->data.ptr, src_step,
                   dst->data.ptr, dst_step, size,
                   scale, shift, type ));
    }

    __END__;
}

/********************* helper functions for converting 32f<->64f ************************/

IPCVAPI_IMPL( CvStatus, icvCvt_32f64f,
    ( const float* src, double* dst, int len ), (src, dst, len) )
{
    int i;
    for( i = 0; i <= len - 4; i += 4 )
    {
        double t0 = src[i];
        double t1 = src[i+1];

        dst[i] = t0;
        dst[i+1] = t1;

        t0 = src[i+2];
        t1 = src[i+3];

        dst[i+2] = t0;
        dst[i+3] = t1;
    }

    for( ; i < len; i++ )
        dst[i] = src[i];

    return CV_OK;
}


IPCVAPI_IMPL( CvStatus, icvCvt_64f32f,
    ( const double* src, float* dst, int len ), (src, dst, len) )
{
    int i = 0;
    for( ; i <= len - 4; i += 4 )
    {
        double t0 = src[i];
        double t1 = src[i+1];

        dst[i] = (float)t0;
        dst[i+1] = (float)t1;

        t0 = src[i+2];
        t1 = src[i+3];

        dst[i+2] = (float)t0;
        dst[i+3] = (float)t1;
    }

    for( ; i < len; i++ )
        dst[i] = (float)src[i];

    return CV_OK;
}


CvStatus CV_STDCALL icvScale_32f( const float* src, float* dst, int len, float a, float b )
{
    int i;
    for( i = 0; i <= len - 4; i += 4 )
    {
        double t0 = src[i]*a + b;
        double t1 = src[i+1]*a + b;

        dst[i] = (float)t0;
        dst[i+1] = (float)t1;

        t0 = src[i+2]*a + b;
        t1 = src[i+3]*a + b;

        dst[i+2] = (float)t0;
        dst[i+3] = (float)t1;
    }

    for( ; i < len; i++ )
        dst[i] = (float)(src[i]*a + b);

    return CV_OK;
}


CvStatus CV_STDCALL icvScale_64f( const double* src, double* dst, int len, double a, double b )
{
    int i;
    for( i = 0; i <= len - 4; i += 4 )
    {
        double t0 = src[i]*a + b;
        double t1 = src[i+1]*a + b;

        dst[i] = t0;
        dst[i+1] = t1;

        t0 = src[i+2]*a + b;
        t1 = src[i+3]*a + b;

        dst[i+2] = t0;
        dst[i+3] = t1;
    }

    for( ; i < len; i++ )
        dst[i] = src[i]*a + b;

    return CV_OK;
}

/* End of file. */
