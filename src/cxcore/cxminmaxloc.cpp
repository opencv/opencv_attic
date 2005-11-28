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

static CvStatus
icvWriteMinMaxResults( double min_val, double max_val,
                       int min_loc, int max_loc,
                       int width, void *minVal, void *maxVal,
                       CvPoint * minLoc, CvPoint * maxLoc, int is_double )
{
    int is_empty = (min_loc|max_loc) < 0;

    if( is_empty )
        min_val = max_val = 0.;

    if( is_double )
    {
        if( minVal )
            *(double*)minVal = min_val;
        if( maxVal )
            *(double*)maxVal = max_val;
    }
    else
    {
        if( minVal )
            *(float*)minVal = (float)min_val;
        if( maxVal )
            *(float*)maxVal = (float)max_val;
    }

    if( minLoc )
    {
        if( !is_empty )
        {
            minLoc->y = min_loc / width;
            minLoc->x = min_loc - minLoc->y * width;
        }
        else
            minLoc->x = minLoc->y = -1;
    }

    if( maxLoc )
    {
        if( !is_empty )
        {
            maxLoc->y = max_loc / width;
            maxLoc->x = max_loc - maxLoc->y * width;
        }
        else
            maxLoc->x = maxLoc->y = -1;
    }
    return CV_NO_ERR;
}


/****************************************************************************************\
*                                     MinMaxLoc                                          *
\****************************************************************************************/
                                                                    
#define CV_MINMAXLOC_ENTRY( _cast_macro_, _toggle_, srctype, temptype, cn )\
    temptype min_val, max_val;                      \
    int min_loc = 0, max_loc = 0;                   \
    int x, loc = 0, width = size.width;             \
                                                    \
    if( (int)(width*(cn)*sizeof(srctype)) == step ) \
    {                                               \
        width *= size.height;                       \
        size.height = 1;                            \
    }                                               \
                                                    \
    min_val = _cast_macro_((src)[0]);               \
    min_val = max_val = _toggle_( min_val )



#define CV_MINMAXLOC_EXIT( _toggle_, _fin_cast_macro_ ) \
    min_val = _toggle_( min_val );                  \
    max_val = _toggle_( max_val );                  \
                                                    \
    return  icvWriteMinMaxResults(                  \
        _fin_cast_macro_(min_val),                  \
        _fin_cast_macro_(max_val),                  \
        min_loc, max_loc, size.width,               \
        minVal, maxVal, minLoc, maxLoc,             \
        sizeof(*minVal) == sizeof(double) )



#define ICV_DEF_MINMAXLOC_1D_CASE_COI( _cast_macro_, _toggle_, temptype, src, len,  \
                                       min_val, max_val, min_loc, max_loc, loc, cn )\
{                                                                                   \
    for( x = 0; x < (len)*(cn); x += (cn), (loc)++ )                                \
    {                                                                               \
        temptype val = _cast_macro_((src)[x]);                                      \
        val = _toggle_(val);                                                        \
                                                                                    \
        if( val < (min_val) )                                                       \
        {                                                                           \
            (min_val) = val;                                                        \
            (min_loc) = (loc);                                                      \
        }                                                                           \
        else if( val > (max_val) )                                                  \
        {                                                                           \
            (max_val) = val;                                                        \
            (max_loc) = (loc);                                                      \
        }                                                                           \
    }                                                                               \
}



#define ICV_DEF_MINMAXLOC_1D_CASE_C1( _cast_macro_, _toggle_, temptype, src, len,   \
                                      min_val, max_val, min_loc, max_loc, loc )     \
    ICV_DEF_MINMAXLOC_1D_CASE_COI( _cast_macro_, _toggle_, temptype, src, len,      \
                                   min_val, max_val, min_loc, max_loc, loc, 1 )



#define ICV_DEF_MINMAXLOC_FUNC_2D( _cast_macro_, _toggle_, _fin_cast_macro_,        \
                                   _entry_, _exit_, flavor, srctype,                \
                                   temptype, extrtype )                             \
IPCVAPI_IMPL( CvStatus,                                                             \
icvMinMaxIndx_##flavor##_C1R,( const srctype* src, int step,                        \
                               CvSize size, extrtype* minVal, extrtype* maxVal,     \
                               CvPoint* minLoc, CvPoint* maxLoc ),                  \
                              (src, step, size, minVal, maxVal, minLoc, maxLoc) )   \
{                                                                                   \
    _entry_( _cast_macro_, _toggle_, srctype, temptype, 1 );                        \
    step /= sizeof(src[0]);                                                         \
                                                                                    \
    for( ; size.height--; src += step )                                             \
    {                                                                               \
        ICV_DEF_MINMAXLOC_1D_CASE_C1( _cast_macro_, _toggle_, temptype, src, width, \
                                      min_val, max_val, min_loc, max_loc, loc );    \
    }                                                                               \
                                                                                    \
    _exit_( _toggle_, _fin_cast_macro_ );                                           \
}


#define ICV_DEF_MINMAXLOC_FUNC_2D_COI( _cast_macro_, _toggle_, _fin_cast_macro_,    \
                                       _entry_, _exit_, flavor,                     \
                                       srctype, temptype, extrtype  )               \
static CvStatus CV_STDCALL                                                          \
icvMinMaxIndx_##flavor##_CnCR( const srctype* src, int step,                        \
                          CvSize size, int cn, int coi,                             \
                          extrtype* minVal, extrtype* maxVal,                       \
                          CvPoint* minLoc, CvPoint* maxLoc )                        \
{                                                                                   \
    (src) += coi - 1;                                                               \
    _entry_( _cast_macro_, _toggle_, srctype, temptype, cn );                       \
    step /= sizeof(src[0]);                                                         \
                                                                                    \
    for( ; size.height--; src += step )                                             \
    {                                                                               \
        ICV_DEF_MINMAXLOC_1D_CASE_COI( _cast_macro_, _toggle_, temptype, src, width,\
                                       min_val, max_val, min_loc, max_loc, loc, cn);\
    }                                                                               \
                                                                                    \
    _exit_( _toggle_, _fin_cast_macro_ );                                           \
}


#define ICV_DEF_MINMAXLOC_ALL( flavor, srctype, temptype, extrtype )                \
    ICV_DEF_MINMAXLOC_FUNC_2D( CV_NOP, CV_NOP, CV_CAST_64F, CV_MINMAXLOC_ENTRY,     \
                               CV_MINMAXLOC_EXIT, flavor,                           \
                               srctype, temptype, extrtype )                        \
    ICV_DEF_MINMAXLOC_FUNC_2D_COI( CV_NOP, CV_NOP, CV_CAST_64F, CV_MINMAXLOC_ENTRY, \
                                   CV_MINMAXLOC_EXIT, flavor, srctype, temptype, extrtype )

#define  _toggle_float_         CV_TOGGLE_FLT
#define  _toggle_double_        CV_TOGGLE_DBL
#define  _as_int_(x)            (*(int*)&(x))
#define  _as_float_(x)          (*(float*)&(x))
#define  _as_int64_(x)          (*(int64*)&(x))
#define  _as_double_(x)         (*(double*)&(x))


#define ICV_DEF_MINMAXLOC_ALL_FLT( flavor, srctype, temptype,                   \
                                   _cast_macro_, _toggle_,                      \
                                   _fin_cast_macro_, extrtype )                 \
                                                                                \
    ICV_DEF_MINMAXLOC_FUNC_2D( _cast_macro_, _toggle_, _fin_cast_macro_,        \
                               CV_MINMAXLOC_ENTRY, CV_MINMAXLOC_EXIT,           \
                               flavor, srctype, temptype, extrtype )            \
    ICV_DEF_MINMAXLOC_FUNC_2D_COI( _cast_macro_, _toggle_, _fin_cast_macro_,    \
                                   CV_MINMAXLOC_ENTRY, CV_MINMAXLOC_EXIT,       \
                                   flavor, srctype, temptype, extrtype )

ICV_DEF_MINMAXLOC_ALL( 8u, uchar, int, float )
ICV_DEF_MINMAXLOC_ALL( 16u, ushort, int, float )
ICV_DEF_MINMAXLOC_ALL( 16s, short, int, float )
ICV_DEF_MINMAXLOC_ALL( 32s, int, int, double )
ICV_DEF_MINMAXLOC_ALL_FLT( 32f, float, int, _as_int_,
                           _toggle_float_, _as_float_, float )
ICV_DEF_MINMAXLOC_ALL_FLT( 64f, double, int64, _as_int64_,
                           _toggle_double_, _as_double_, double )


/****************************************************************************************\
*                              MinMaxLoc with mask                                       *
\****************************************************************************************/


#define CV_MINMAXLOC_MASK_ENTRY( _cast_macro_, _toggle_,            \
                                 srctype, temptype, cn )            \
    temptype min_val = 0, max_val = 0;                              \
    int min_loc = -1, max_loc = -1;                                 \
    int x = 0, y, loc = 0, width = size.width;                      \
    step /= sizeof(src[0]);                                         \
                                                                    \
    if( width*(cn) == step && width == maskStep )                   \
    {                                                               \
        width *= size.height;                                       \
        size.height = 1;                                            \
    }                                                               \
                                                                    \
    for( y = 0; y < size.height; y++, src += step,                  \
                                      mask += maskStep )            \
    {                                                               \
        for( x = 0; x < width; x++, loc++ )                         \
            if( mask[x] != 0 )                                      \
            {                                                       \
                min_loc = max_loc = loc;                            \
                min_val = _cast_macro_((src)[x*(cn)]);              \
                min_val = max_val = _toggle_( min_val );            \
                goto stop_scan;                                     \
            }                                                       \
    }                                                               \
                                                                    \
    stop_scan:;




#define ICV_DEF_MINMAXLOC_MASK_FUNC_2D(_cast_macro_, _toggle_, _fin_cast_macro_,\
                                       _entry_, _exit_, flavor,                 \
                                       srctype, temptype, extrtype )            \
IPCVAPI_IMPL( CvStatus,                                                         \
icvMinMaxIndx_##flavor##_C1MR,( const srctype* src, int step,                   \
                                const uchar* mask, int maskStep,                \
                                CvSize size, extrtype* minVal, extrtype* maxVal,\
                                CvPoint* minLoc, CvPoint* maxLoc ),             \
                               (src, step, mask, maskStep, size,                \
                                minVal, maxVal, minLoc, maxLoc) )               \
{                                                                               \
    _entry_( _cast_macro_, _toggle_, srctype, temptype, 1 );                    \
                                                                                \
    for( ; y < size.height; y++, src += step, mask += maskStep )                \
    {                                                                           \
        for( ; x < width; x++, (loc)++ )                                        \
        {                                                                       \
            temptype val = _cast_macro_((src)[x]);                              \
            int m = (mask)[x] != 0;                                             \
            val = _toggle_(val);                                                \
                                                                                \
            if( val < (min_val) && m )                                          \
            {                                                                   \
                (min_val) = val;                                                \
                (min_loc) = (loc);                                              \
            }                                                                   \
            else if( val > (max_val) && m )                                     \
            {                                                                   \
                (max_val) = val;                                                \
                (max_loc) = (loc);                                              \
            }                                                                   \
        }                                                                       \
        x = 0;                                                                  \
    }                                                                           \
                                                                                \
    _exit_( _toggle_, _fin_cast_macro_ );                                       \
}


#define ICV_DEF_MINMAXLOC_MASK_FUNC_2D_COI( _cast_macro_, _toggle_, _fin_cast_macro_,\
                                     _entry_, _exit_, flavor,                   \
                                     srctype, temptype, extrtype )              \
static CvStatus CV_STDCALL                                                      \
icvMinMaxIndx_##flavor##_CnCMR( const srctype* src, int step,                   \
                           const uchar* mask, int maskStep,                     \
                           CvSize size, int cn, int coi,                        \
                           extrtype* minVal, extrtype* maxVal,                  \
                           CvPoint* minLoc, CvPoint* maxLoc )                   \
{                                                                               \
    (src) += coi - 1;                                                           \
    _entry_( _cast_macro_, _toggle_, srctype, temptype, cn );                   \
                                                                                \
    for( ; y < size.height; y++, src += step, mask += maskStep )                \
    {                                                                           \
        for( ; x < width; x++, (loc)++ )                                        \
        {                                                                       \
            temptype val = _cast_macro_((src)[x*(cn)]);                         \
            int m = (mask)[x] != 0;                                             \
            val = _toggle_(val);                                                \
                                                                                \
            if( val < (min_val) && m )                                          \
            {                                                                   \
                (min_val) = val;                                                \
                (min_loc) = (loc);                                              \
            }                                                                   \
            else if( val > (max_val) && m )                                     \
            {                                                                   \
                (max_val) = val;                                                \
                (max_loc) = (loc);                                              \
            }                                                                   \
        }                                                                       \
        x = 0;                                                                  \
    }                                                                           \
                                                                                \
    _exit_( _toggle_, _fin_cast_macro_ );                                       \
}



#define ICV_DEF_MINMAXLOC_MASK_ALL( flavor, srctype, temptype, extrtype )           \
                                                                                    \
    ICV_DEF_MINMAXLOC_MASK_FUNC_2D( CV_NOP, CV_NOP, CV_CAST_64F,                    \
                               CV_MINMAXLOC_MASK_ENTRY, CV_MINMAXLOC_EXIT,          \
                               flavor, srctype, temptype, extrtype )                \
    ICV_DEF_MINMAXLOC_MASK_FUNC_2D_COI( CV_NOP, CV_NOP, CV_CAST_64F,                \
                                   CV_MINMAXLOC_MASK_ENTRY, CV_MINMAXLOC_EXIT,      \
                                   flavor, srctype, temptype, extrtype )


#define ICV_DEF_MINMAXLOC_MASK_ALL_FLT( flavor, srctype, temptype,                  \
                                        _cast_macro_, _toggle_,                     \
                                        _fin_cast_macro_, extrtype )                \
                                                                                    \
    ICV_DEF_MINMAXLOC_MASK_FUNC_2D( _cast_macro_, _toggle_, _fin_cast_macro_,       \
                                    CV_MINMAXLOC_MASK_ENTRY, CV_MINMAXLOC_EXIT,     \
                                    flavor, srctype, temptype, extrtype )           \
    ICV_DEF_MINMAXLOC_MASK_FUNC_2D_COI( _cast_macro_, _toggle_, _fin_cast_macro_,   \
                                       CV_MINMAXLOC_MASK_ENTRY, CV_MINMAXLOC_EXIT,  \
                                       flavor, srctype, temptype, extrtype )


ICV_DEF_MINMAXLOC_MASK_ALL( 8u, uchar, int, float )
ICV_DEF_MINMAXLOC_MASK_ALL( 16u, ushort, int, float )
ICV_DEF_MINMAXLOC_MASK_ALL( 16s, short, int, float )
ICV_DEF_MINMAXLOC_MASK_ALL( 32s, int, int, double )
ICV_DEF_MINMAXLOC_MASK_ALL_FLT( 32f, float, int, _as_int_,
                                _toggle_float_, _as_float_, float )
ICV_DEF_MINMAXLOC_MASK_ALL_FLT( 64f, double, int64, _as_int64_,
                                _toggle_double_, _as_double_, double )

#define icvMinMaxIndx_8s_C1R    0
#define icvMinMaxIndx_8s_CnCR   0
#define icvMinMaxIndx_8s_C1MR   0
#define icvMinMaxIndx_8s_CnCMR  0

CV_DEF_INIT_FUNC_TAB_2D( MinMaxIndx, C1R )
CV_DEF_INIT_FUNC_TAB_2D( MinMaxIndx, CnCR )
CV_DEF_INIT_FUNC_TAB_2D( MinMaxIndx, C1MR )
CV_DEF_INIT_FUNC_TAB_2D( MinMaxIndx, CnCMR )


CV_IMPL  void
cvMinMaxLoc( const void* img, double* _minVal, double* _maxVal,
             CvPoint* _minLoc, CvPoint* _maxLoc, const void* mask )
{
    static CvFuncTable minmax_tab, minmaxcoi_tab;
    static CvFuncTable minmaxmask_tab, minmaxmaskcoi_tab;
    static int inittab = 0;

    CV_FUNCNAME("cvMinMaxLoc");

    __BEGIN__;

    int type, depth, cn, coi = 0;
    int mat_step, mask_step = 0;
    CvSize size;
    CvMat stub, maskstub, *mat = (CvMat*)img, *matmask = (CvMat*)mask;
    CvPoint minLoc, maxLoc;
    double minVal = 0, maxVal = 0;

    if( !inittab )
    {
        icvInitMinMaxIndxC1RTable( &minmax_tab );
        icvInitMinMaxIndxCnCRTable( &minmaxcoi_tab );
        icvInitMinMaxIndxC1MRTable( &minmaxmask_tab );
        icvInitMinMaxIndxCnCMRTable( &minmaxmaskcoi_tab );
        inittab = 1;
    }
    
    CV_CALL( mat = cvGetMat( mat, &stub, &coi ));

    type = CV_MAT_TYPE( mat->type );
    depth = CV_MAT_DEPTH( type );
    cn = CV_MAT_CN( type );
    size = cvGetMatSize( mat );

    if( cn > 1 && coi == 0 )
        CV_ERROR( CV_StsBadArg, "" );
    
    mat_step = mat->step;

    if( !mask )
    {
        if( size.height == 1 )
            mat_step = CV_STUB_STEP;

        if( CV_MAT_CN(type) == 1 || coi == 0 )
        {
            CvFunc2D_1A4P func = (CvFunc2D_1A4P)(minmax_tab.fn_2d[depth]);

            if( !func )
                CV_ERROR( CV_StsBadArg, cvUnsupportedFormat );

            IPPI_CALL( func( mat->data.ptr, mat_step, size,
                             &minVal, &maxVal, &minLoc, &maxLoc ));
        }
        else
        {
            CvFunc2DnC_1A4P func = (CvFunc2DnC_1A4P)(minmaxcoi_tab.fn_2d[depth]);

            if( !func )
                CV_ERROR( CV_StsBadArg, cvUnsupportedFormat );

            IPPI_CALL( func( mat->data.ptr, mat_step, size, cn, coi,
                             &minVal, &maxVal, &minLoc, &maxLoc ));
        }
    }
    else
    {
        CV_CALL( matmask = cvGetMat( matmask, &maskstub ));

        if( !CV_IS_MASK_ARR( matmask ))
            CV_ERROR( CV_StsBadMask, "" );

        if( !CV_ARE_SIZES_EQ( mat, matmask ))
            CV_ERROR( CV_StsUnmatchedSizes, "" );

        mask_step = matmask->step;

        if( size.height == 1 )
            mat_step = mask_step = CV_STUB_STEP;

        if( CV_MAT_CN(type) == 1 || coi == 0 )
        {
            CvFunc2D_2A4P func = (CvFunc2D_2A4P)(minmaxmask_tab.fn_2d[depth]);

            if( !func )
                CV_ERROR( CV_StsBadArg, cvUnsupportedFormat );

            IPPI_CALL( func( mat->data.ptr, mat_step, matmask->data.ptr,
                             mask_step, size,
                             &minVal, &maxVal, &minLoc, &maxLoc ));
        }
        else
        {
            CvFunc2DnC_2A4P func = (CvFunc2DnC_2A4P)(minmaxmaskcoi_tab.fn_2d[depth]);

            if( !func )
                CV_ERROR( CV_StsBadArg, cvUnsupportedFormat );

            IPPI_CALL( func( mat->data.ptr, mat_step,
                             matmask->data.ptr, mask_step, size, cn, coi,
                             &minVal, &maxVal, &minLoc, &maxLoc ));
        }
    }

    if( depth < CV_32S || depth == CV_32F )
    {
        minVal = *(float*)&minVal;
        maxVal = *(float*)&maxVal;
    }

    if( _minVal )
        *_minVal = minVal;

    if( _maxVal )
        *_maxVal = maxVal;

    if( _minLoc )
        *_minLoc = minLoc;

    if( _maxLoc )
        *_maxLoc = maxLoc;

    __END__;
}


/*  End of file  */
