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

/* 
   Initializes line iterator.
   Returns number of points on the line or negative number if error.
*/
CV_IMPL int
cvInitLineIterator( const void* img, CvPoint pt1, CvPoint pt2,
                    CvLineIterator* iterator, int connectivity )
{
    int count = -1;
    
    CV_FUNCNAME( "cvInitLineIterator" );

    __BEGIN__;

    CvMat stub, *mat = (CvMat*)img;
    int dx, dy, s;
    int bt_pix, bt_pix0, step;

    CV_CALL( mat = cvGetMat( mat, &stub ));

    if( !iterator )
        CV_ERROR( CV_StsNullPtr, "" );

    if( connectivity != 8 && connectivity != 4 )
        CV_ERROR( CV_StsBadArg, "" );

    if( (unsigned)pt1.x >= (unsigned)(mat->width) ||
        (unsigned)pt2.x >= (unsigned)(mat->width) ||
        (unsigned)pt1.y >= (unsigned)(mat->height) ||
        (unsigned)pt2.y >= (unsigned)(mat->height) )
        CV_ERROR( CV_StsBadPoint, "" );

    assert( connectivity == 4 || connectivity == 8 );

    bt_pix0 = bt_pix = CV_ELEM_SIZE(mat->type);
    step = mat->step;

    dx = pt2.x - pt1.x;
    dy = pt2.y - pt1.y;
    s = dx < 0 ? -1 : 0;

    dx = (dx ^ s) - s;
    bt_pix = (bt_pix ^ s) - s;

    iterator->ptr = (uchar*)(mat->data.ptr + pt1.y * step + pt1.x * bt_pix0);

    s = dy < 0 ? -1 : 0;
    dy = (dy ^ s) - s;
    step = (step ^ s) - s;

    s = dy > dx ? -1 : 0;
    
    /* conditional swaps */
    dx ^= dy & s;
    dy ^= dx & s;
    dx ^= dy & s;

    bt_pix ^= step & s;
    step ^= bt_pix & s;
    bt_pix ^= step & s;

    if( connectivity == 8 )
    {
        assert( dx >= 0 && dy >= 0 );
        
        iterator->err = dx - (dy + dy);
        iterator->plus_delta = dx + dx;
        iterator->minus_delta = -(dy + dy);
        iterator->plus_step = step;
        iterator->minus_step = bt_pix;
        count = dx + 1;
    }
    else /* connectivity == 4 */
    {
        assert( dx >= 0 && dy >= 0 );
        
        iterator->err = 0;
        iterator->plus_delta = (dx + dx) + (dy + dy);
        iterator->minus_delta = -(dy + dy);
        iterator->plus_step = step - bt_pix;
        iterator->minus_step = bt_pix;
        count = dx + dy + 1;
    }

    __END__;

    return count;
}


/**************************************************************************************\
*                                   line samplers                                      *
\**************************************************************************************/

CV_IMPL int
cvSampleLine( const void* img, CvPoint pt1, CvPoint pt2,
              void* buffer, int connectivity )
{
    int count = -1;
    
    CV_FUNCNAME( "cvSampleLine" );

    __BEGIN__;
    
    int i, coi = 0, pix_size;
    CvMat stub, *mat = (CvMat*)img;
    CvLineIterator iterator;

    CV_CALL( mat = cvGetMat( mat, &stub, &coi ));

    if( coi != 0 )
        CV_ERROR( CV_BadCOI, "" );

    if( !buffer )
        CV_ERROR( CV_StsNullPtr, "" );

    CV_CALL( count = cvInitLineIterator( mat, pt1, pt2, &iterator, connectivity ));

    pix_size = CV_ELEM_SIZE(mat->type);
    for( i = 0; i < count; i++ )
    {
        CV_MEMCPY_AUTO( buffer, iterator.ptr, pix_size );
        (char*&)buffer += pix_size;
        CV_NEXT_LINE_POINT( iterator );
    }

    __END__;

    return count;
}


static const void*
icvAdjustRect( const void* srcptr, int src_step, int pix_size,
               CvSize src_size, CvSize win_size,
               CvPoint ip, CvRect* pRect )
{
    CvRect rect;
    const char* src = (const char*)srcptr;

    if( ip.x >= 0 )
    {
        src += ip.x*pix_size;
        rect.x = 0;
    }
    else
    {
        rect.x = -ip.x;
        if( rect.x > win_size.width )
            rect.x = win_size.width;
    }

    if( ip.x + win_size.width < src_size.width )
        rect.width = win_size.width;
    else
    {
        rect.width = src_size.width - ip.x - 1;
        if( rect.width < 0 )
        {
            src += rect.width*pix_size;
            rect.width = 0;
        }
        assert( rect.width <= win_size.width );
    }

    if( ip.y >= 0 )
    {
        src += ip.y * src_step;
        rect.y = 0;
    }
    else
        rect.y = -ip.y;

    if( ip.y + win_size.height < src_size.height )
        rect.height = win_size.height;
    else
    {
        rect.height = src_size.height - ip.y - 1;
        if( rect.height < 0 )
        {
            src += rect.height*src_step;
            rect.height = 0;
        }
    }

    *pRect = rect;
    return src - rect.x*pix_size;
}


#define  ICV_DEF_GET_RECT_SUB_PIX_FUNC( flavor, srctype, dsttype, worktype, \
                                        cast_macro, scale_macro, cast_macro2 )\
IPCVAPI_IMPL( CvStatus, icvGetRectSubPix_##flavor##_C1R,                    \
( const srctype* src, int src_step, CvSize src_size,                        \
  dsttype* dst, int dst_step, CvSize win_size, CvPoint2D32f center ),       \
  (src, src_step, src_size, dst, dst_step, win_size, center) )              \
{                                                                           \
    CvPoint ip;                                                             \
    worktype  a11, a12, a21, a22, b1, b2;                                   \
    float a, b;                                                             \
    int i, j;                                                               \
                                                                            \
    center.x -= (win_size.width-1)*0.5f;                                    \
    center.y -= (win_size.height-1)*0.5f;                                   \
                                                                            \
    ip.x = cvFloor( center.x );                                             \
    ip.y = cvFloor( center.y );                                             \
                                                                            \
    a = center.x - ip.x;                                                    \
    b = center.y - ip.y;                                                    \
    a11 = scale_macro((1.f-a)*(1.f-b));                                     \
    a12 = scale_macro(a*(1.f-b));                                           \
    a21 = scale_macro((1.f-a)*b);                                           \
    a22 = scale_macro(a*b);                                                 \
    b1 = scale_macro(1.f - b);                                              \
    b2 = scale_macro(b);                                                    \
                                                                            \
    src_step /= sizeof( src[0] );                                           \
                                                                            \
    if( 0 <= ip.x && ip.x + win_size.width < src_size.width &&              \
        0 <= ip.y && ip.y + win_size.height < src_size.height )             \
    {                                                                       \
        /* extracted rectangle is totally inside the image */               \
        src += ip.y * src_step + ip.x;                                      \
                                                                            \
        for( i = 0; i < win_size.height; i++, src += src_step,              \
                                              (char*&)dst += dst_step )     \
        {                                                                   \
            for( j = 0; j <= win_size.width - 2; j += 2 )                   \
            {                                                               \
                worktype s0 = cast_macro(src[j])*a11 +                      \
                              cast_macro(src[j+1])*a12 +                    \
                              cast_macro(src[j+src_step])*a21 +             \
                              cast_macro(src[j+src_step+1])*a22;            \
                worktype s1 = cast_macro(src[j+1])*a11 +                    \
                              cast_macro(src[j+2])*a12 +                    \
                              cast_macro(src[j+src_step+1])*a21 +           \
                              cast_macro(src[j+src_step+2])*a22;            \
                                                                            \
                dst[j] = (dsttype)cast_macro2(s0);                          \
                dst[j+1] = (dsttype)cast_macro2(s1);                        \
            }                                                               \
                                                                            \
            for( ; j < win_size.width; j++ )                                \
            {                                                               \
                worktype s0 = cast_macro(src[j])*a11 +                      \
                              cast_macro(src[j+1])*a12 +                    \
                              cast_macro(src[j+src_step])*a21 +             \
                              cast_macro(src[j+src_step+1])*a22;            \
                                                                            \
                dst[j] = (dsttype)cast_macro2(s0);                          \
            }                                                               \
        }                                                                   \
    }                                                                       \
    else                                                                    \
    {                                                                       \
        CvRect r;                                                           \
                                                                            \
        src = (const srctype*)icvAdjustRect( src, src_step*sizeof(*src),    \
                               sizeof(*src), src_size, win_size,ip, &r);    \
                                                                            \
        for( i = 0; i < win_size.height; i++, (char*&)dst += dst_step )     \
        {                                                                   \
            const srctype *src2 = src + src_step;                           \
                                                                            \
            if( i < r.y || i >= r.height )                                  \
                src2 -= src_step;                                           \
                                                                            \
            for( j = 0; j < r.x; j++ )                                      \
            {                                                               \
                worktype s0 = cast_macro(src[r.x])*b1 +                     \
                              cast_macro(src2[r.x])*b2;                     \
                                                                            \
                dst[j] = (dsttype)cast_macro2(s0);                          \
            }                                                               \
                                                                            \
            for( ; j < r.width; j++ )                                       \
            {                                                               \
                worktype s0 = cast_macro(src[j])*a11 +                      \
                              cast_macro(src[j+1])*a12 +                    \
                              cast_macro(src2[j])*a21 +                     \
                              cast_macro(src2[j+1])*a22;                    \
                                                                            \
                dst[j] = (dsttype)cast_macro2(s0);                          \
            }                                                               \
                                                                            \
            for( ; j < win_size.width; j++ )                                \
            {                                                               \
                worktype s0 = cast_macro(src[r.width])*b1 +                 \
                              cast_macro(src2[r.width])*b2;                 \
                                                                            \
                dst[j] = (dsttype)cast_macro2(s0);                          \
            }                                                               \
                                                                            \
            if( i < r.height )                                              \
                src = src2;                                                 \
        }                                                                   \
    }                                                                       \
                                                                            \
    return CV_OK;                                                           \
}


#define  ICV_DEF_GET_RECT_SUB_PIX_FUNC_C3( flavor, srctype, dsttype, worktype, \
                                        cast_macro, scale_macro, mul_macro )\
IPCVAPI_IMPL( CvStatus, icvGetRectSubPix_##flavor##_C3R,                    \
( const srctype* src, int src_step, CvSize src_size,                        \
  dsttype* dst, int dst_step, CvSize win_size, CvPoint2D32f center ),       \
  (src, src_step, src_size, dst, dst_step, win_size, center) )              \
{                                                                           \
    CvPoint ip;                                                             \
    worktype a, b;                                                          \
    int i, j;                                                               \
                                                                            \
    center.x -= (win_size.width-1)*0.5f;                                    \
    center.y -= (win_size.height-1)*0.5f;                                   \
                                                                            \
    ip.x = cvFloor( center.x );                                             \
    ip.y = cvFloor( center.y );                                             \
                                                                            \
    a = scale_macro( center.x - ip.x );                                     \
    b = scale_macro( center.y - ip.y );                                     \
                                                                            \
    src_step /= sizeof( src[0] );                                           \
                                                                            \
    if( 0 <= ip.x && ip.x + win_size.width < src_size.width &&              \
        0 <= ip.y && ip.y + win_size.height < src_size.height )             \
    {                                                                       \
        /* extracted rectangle is totally inside the image */               \
        src += ip.y * src_step + ip.x*3;                                    \
                                                                            \
        for( i = 0; i < win_size.height; i++, src += src_step,              \
                                              (char*&)dst += dst_step )     \
        {                                                                   \
            for( j = 0; j < win_size.width; j++ )                           \
            {                                                               \
                worktype s0 = cast_macro(src[j*3]);                         \
                worktype s1 = cast_macro(src[j*3 + src_step]);              \
                s0 += mul_macro( a, (cast_macro(src[j*3+3]) - s0));         \
                s1 += mul_macro( a, (cast_macro(src[j*3+3+src_step]) - s1));\
                dst[j*3] = (dsttype)(s0 + mul_macro( b, (s1 - s0)));        \
                                                                            \
                s0 = cast_macro(src[j*3+1]);                                \
                s1 = cast_macro(src[j*3+1 + src_step]);                     \
                s0 += mul_macro( a, (cast_macro(src[j*3+4]) - s0));         \
                s1 += mul_macro( a, (cast_macro(src[j*3+4+src_step]) - s1));\
                dst[j*3+1] = (dsttype)(s0 + mul_macro( b, (s1 - s0)));      \
                                                                            \
                s0 = cast_macro(src[j*3+2]);                                \
                s1 = cast_macro(src[j*3+2 + src_step]);                     \
                s0 += mul_macro( a, (cast_macro(src[j*3+5]) - s0));         \
                s1 += mul_macro( a, (cast_macro(src[j*3+5+src_step]) - s1));\
                dst[j*3+2] = (dsttype)(s0 + mul_macro( b, (s1 - s0)));      \
            }                                                               \
        }                                                                   \
    }                                                                       \
    else                                                                    \
    {                                                                       \
        CvRect r;                                                           \
                                                                            \
        src = (const srctype*)icvAdjustRect( src, src_step*sizeof(*src),    \
                             sizeof(*src)*3, src_size, win_size, ip, &r );  \
                                                                            \
        for( i = 0; i < win_size.height; i++, (char*&)dst += dst_step )     \
        {                                                                   \
            const srctype *src2 = src + src_step;                           \
                                                                            \
            if( i < r.y || i >= r.height )                                  \
                src2 -= src_step;                                           \
                                                                            \
            for( j = 0; j < r.x; j++ )                                      \
            {                                                               \
                worktype s0 = cast_macro(src[r.x*3]);                       \
                worktype s1 = cast_macro(src2[r.x*3]);                      \
                dst[j*3] = (dsttype)(s0 + mul_macro( b, (s1 - s0)));        \
                                                                            \
                s0 = cast_macro(src[r.x*3+1]);                              \
                s1 = cast_macro(src2[r.x*3+1]);                             \
                dst[j*3+1] = (dsttype)(s0 + mul_macro( b, (s1 - s0)));      \
                                                                            \
                s0 = cast_macro(src[r.x*3+2]);                              \
                s1 = cast_macro(src2[r.x*3+2]);                             \
                dst[j*3+2] = (dsttype)(s0 + mul_macro( b, (s1 - s0)));      \
            }                                                               \
                                                                            \
            for( ; j < r.width; j++ )                                       \
            {                                                               \
                worktype s0 = cast_macro(src[j*3]);                         \
                worktype s1 = cast_macro(src2[j*3]);                        \
                s0 += mul_macro( a, (cast_macro(src[j*3 + 3]) - s0));       \
                s1 += mul_macro( a, (cast_macro(src2[j*3 + 3]) - s1));      \
                dst[j*3] = (dsttype)(s0 + mul_macro( b, (s1 - s0)));        \
                                                                            \
                s0 = cast_macro(src[j*3+1]);                                \
                s1 = cast_macro(src2[j*3+1]);                               \
                s0 += mul_macro( a, (cast_macro(src[j*3 + 4]) - s0));       \
                s1 += mul_macro( a, (cast_macro(src2[j*3 + 4]) - s1));      \
                dst[j*3+1] = (dsttype)(s0 + mul_macro( b, (s1 - s0)));      \
                                                                            \
                s0 = cast_macro(src[j*3+2]);                                \
                s1 = cast_macro(src2[j*3+2]);                               \
                s0 += mul_macro( a, (cast_macro(src[j*3 + 5]) - s0));       \
                s1 += mul_macro( a, (cast_macro(src2[j*3 + 5]) - s1));      \
                dst[j*3+2] = (dsttype)(s0 + mul_macro( b, (s1 - s0)));      \
            }                                                               \
                                                                            \
            for( ; j < win_size.width; j++ )                                \
            {                                                               \
                worktype s0 = cast_macro(src[r.width*3]);                   \
                worktype s1 = cast_macro(src2[r.width*3]);                  \
                dst[j*3] = (dsttype)(s0 + mul_macro( b, (s1 - s0)));        \
                                                                            \
                s0 = cast_macro(src[r.width*3+1]);                          \
                s1 = cast_macro(src2[r.width*3+1]);                         \
                dst[j*3+1] = (dsttype)(s0 + mul_macro( b, (s1 - s0)));      \
                                                                            \
                s0 = cast_macro(src[r.width*3+2]);                          \
                s1 = cast_macro(src2[r.width*3+2]);                         \
                dst[j*3+2] = (dsttype)(s0 + mul_macro( b, (s1 - s0)));      \
            }                                                               \
                                                                            \
            if( i < r.height )                                              \
                src = src2;                                                 \
        }                                                                   \
    }                                                                       \
                                                                            \
    return CV_OK;                                                           \
}


#define ICV_SHIFT             16
#define ICV_SCALE(x)          cvRound((x)*(1 << ICV_SHIFT))
#define ICV_MUL_SCALE(x,y)    (((x)*(y) + (1 << (ICV_SHIFT-1))) >> ICV_SHIFT)
#define ICV_DESCALE(x)        (((x)+(1 << (ICV_SHIFT-1))) >> ICV_SHIFT)

ICV_DEF_GET_RECT_SUB_PIX_FUNC( 8u, uchar, uchar, int, CV_NOP, ICV_SCALE, ICV_DESCALE )
ICV_DEF_GET_RECT_SUB_PIX_FUNC( 32f, float, float, float, CV_NOP, CV_NOP, CV_NOP )
ICV_DEF_GET_RECT_SUB_PIX_FUNC( 8u32f, uchar, float, float, CV_8TO32F, CV_NOP, CV_NOP )

ICV_DEF_GET_RECT_SUB_PIX_FUNC_C3( 8u, uchar, uchar, int, CV_NOP, ICV_SCALE, ICV_MUL_SCALE )
ICV_DEF_GET_RECT_SUB_PIX_FUNC_C3( 32f, float, float, float, CV_NOP, CV_NOP, CV_MUL )
ICV_DEF_GET_RECT_SUB_PIX_FUNC_C3( 8u32f, uchar, float, float, CV_8TO32F, CV_NOP, CV_MUL )


#define  ICV_DEF_INIT_SUBPIX_TAB( FUNCNAME, FLAG )                  \
static void icvInit##FUNCNAME##FLAG##Table( CvFuncTable* tab )      \
{                                                                   \
    tab->fn_2d[CV_8U] = (void*)icv##FUNCNAME##_8u_##FLAG;           \
    tab->fn_2d[CV_32F] = (void*)icv##FUNCNAME##_32f_##FLAG;         \
                                                                    \
    tab->fn_2d[1] = (void*)icv##FUNCNAME##_8u32f_##FLAG;            \
}


ICV_DEF_INIT_SUBPIX_TAB( GetRectSubPix, C1R )
ICV_DEF_INIT_SUBPIX_TAB( GetRectSubPix, C3R )

typedef CvStatus (CV_STDCALL *CvGetRectSubPixFunc)( const void* src, int src_step,
                                                    CvSize src_size, void* dst,
                                                    int dst_step, CvSize win_size,
                                                    CvPoint2D32f center );

CV_IMPL void
cvGetRectSubPix( const void* srcarr, void* dstarr, CvPoint2D32f center )
{
    static CvFuncTable gr_tab[2];
    static int inittab = 0;
    CV_FUNCNAME( "cvGetRectSubPix" );

    __BEGIN__;

    CvMat srcstub, *src = (CvMat*)srcarr;
    CvMat dststub, *dst = (CvMat*)dstarr;
    CvSize src_size, dst_size;
    CvGetRectSubPixFunc func;
    int cn;

    if( !inittab )
    {
        icvInitGetRectSubPixC1RTable( gr_tab + 0 );
        icvInitGetRectSubPixC3RTable( gr_tab + 1 );
        inittab = 1;
    }

    if( !CV_IS_MAT(src))
        CV_CALL( src = cvGetMat( src, &srcstub ));

    if( !CV_IS_MAT(dst))
        CV_CALL( dst = cvGetMat( dst, &dststub ));

    cn = CV_MAT_CN( src->type );

    if( (cn != 1 && cn != 3) || !CV_ARE_CNS_EQ( src, dst ))
        CV_ERROR( CV_StsUnsupportedFormat, "" );

    src_size = cvGetMatSize( src );
    dst_size = cvGetMatSize( dst );

    if( dst_size.width > src_size.width || dst_size.height > src_size.height )
        CV_ERROR( CV_StsBadSize, "destination ROI must be smaller than source ROI" );

    if( CV_ARE_DEPTHS_EQ( src, dst ))
    {
        func = (CvGetRectSubPixFunc)(gr_tab[cn != 1].fn_2d[CV_MAT_DEPTH(src->type)]);
    }
    else
    {
        if( CV_MAT_DEPTH( src->type ) != CV_8U || CV_MAT_DEPTH( dst->type ) != CV_32F )
            CV_ERROR( CV_StsUnsupportedFormat, "" );

        func = (CvGetRectSubPixFunc)(gr_tab[cn != 1].fn_2d[1]);
    }

    if( !func )
        CV_ERROR( CV_StsUnsupportedFormat, "" );

    IPPI_CALL( func( src->data.ptr, src->step, src_size,
                     dst->data.ptr, dst->step, dst_size, center ));

    __END__;
}


#define GET_X(X,Y)  ((X)*A11 + (Y)*A12 + b1)
#define GET_Y(X,Y)  ((X)*A21 + (Y)*A22 + b2)
#define PTR_AT(X,Y) (src + (Y)*src_step + (X))

#define CLIP_X(x) (unsigned)(x) < (unsigned)src_size.width ?  \
                  (x) : (x) < 0 ? 0 : src_size.width - 1

#define CLIP_Y(y) (unsigned)(y) < (unsigned)src_size.height ? \
                  (y) : (y) < 0 ? 0 : src_size.height - 1


#define ICV_DEF_GET_QUADRANGLE_SUB_PIX_FUNC( flavor, srctype, dsttype,      \
                                             worktype, cast_macro, cvt )    \
IPCVAPI_IMPL( CvStatus, icvGetQuadrangleSubPix_##flavor##_C1R,              \
              ( const srctype * src, int src_step, CvSize src_size,         \
                dsttype *dst, int dst_step, CvSize win_size,                \
                const float *matrix, int fill_outliers,                     \
                dsttype* fillval ),                                         \
                (src, src_step, src_size, dst, dst_step, win_size,          \
                matrix, fill_outliers, fillval) )                           \
{                                                                           \
    int x, y, x0, y0, x1, y1;                                               \
    float  A11 = matrix[0], A12 = matrix[1], b1 = matrix[2] - 0.5f;         \
    float  A21 = matrix[3], A22 = matrix[4], b2 = matrix[5] - 0.5f;         \
    dsttype fv = *fillval;                                                  \
    int left = -(win_size.width >> 1), right = win_size.width + left - 1;   \
    int top = -(win_size.height >> 1), bottom = win_size.height + top - 1;  \
                                                                            \
    src_step /= sizeof(srctype);                                            \
    dst_step /= sizeof(dsttype);                                            \
                                                                            \
    dst -= left;                                                            \
                                                                            \
    for( y = top; y <= bottom; y++, dst += dst_step )                       \
    {                                                                       \
        float xs = GET_X( left, y );                                        \
        float ys = GET_Y( left, y );                                        \
        float xe = GET_X( right, y );                                       \
        float ye = GET_Y( right, y );                                       \
                                                                            \
        if( (unsigned)cvFloor(xs) < (unsigned)(src_size.width - 1) &&       \
            (unsigned)cvFloor(ys) < (unsigned)(src_size.height - 1) &&      \
            (unsigned)cvFloor(xe) < (unsigned)(src_size.width - 1) &&       \
            (unsigned)cvFloor(ye) < (unsigned)(src_size.height - 1))        \
        {                                                                   \
            for( x = left; x <= right; x++ )                                \
            {                                                               \
                worktype p0, p1;                                            \
                float a, b;                                                 \
                                                                            \
                int ixs = cvFloor( xs );                                    \
                int iys = cvFloor( ys );                                    \
                const srctype *ptr = PTR_AT( ixs, iys );                    \
                                                                            \
                a = xs - ixs;                                               \
                b = ys - iys;                                               \
                                                                            \
                xs += A11;                                                  \
                ys += A21;                                                  \
                                                                            \
                p0 = cvt(ptr[0]) + a * (cvt(ptr[1]) - cvt(ptr[0]));         \
                p1 = cvt(ptr[src_step]) + a * (cvt(ptr[src_step + 1]) -     \
                                              cvt(ptr[src_step]));          \
                dst[x] = cast_macro(p0 + b * (p1 - p0));                    \
            }                                                               \
        }                                                                   \
        else                                                                \
        {                                                                   \
            for( x = left; x <= right; x++ )                                \
            {                                                               \
                worktype p0, p1;                                            \
                float a, b;                                                 \
                                                                            \
                int ixs = cvFloor( xs );                                    \
                int iys = cvFloor( ys );                                    \
                                                                            \
                a = xs - ixs;                                               \
                b = ys - iys;                                               \
                                                                            \
                xs += A11;                                                  \
                ys += A21;                                                  \
                                                                            \
                if( (unsigned)ixs < (unsigned)(src_size.width - 1) &&       \
                    (unsigned)iys < (unsigned)(src_size.height - 1) )       \
                {                                                           \
                    const srctype *ptr = PTR_AT( ixs, iys );                \
                                                                            \
                    p0 = cvt(ptr[0]) + a * (cvt(ptr[1]) - cvt(ptr[0]));     \
                    p1 = cvt(ptr[src_step]) + a * (cvt(ptr[src_step + 1]) - \
                                                  cvt(ptr[src_step]));      \
                                                                            \
                    dst[x] = cast_macro(p0 + b * (p1 - p0));                \
                }                                                           \
                else if( !fill_outliers )                                   \
                {                                                           \
                    /* the slowest branch */                                \
                    worktype t0, t1;                                        \
                                                                            \
                    x0 = CLIP_X( ixs );                                     \
                    y0 = CLIP_Y( iys );                                     \
                    x1 = CLIP_X( ixs + 1 );                                 \
                    y1 = CLIP_Y( iys + 1 );                                 \
                                                                            \
                    t0 = cvt(*PTR_AT( x0, y0 ));                            \
                    t1 = cvt(*PTR_AT( x1, y0 ));                            \
                    p0 = t0 + a * (t1 - t0);                                \
                                                                            \
                    t0 = cvt(*PTR_AT( x0, y1 ));                            \
                    t1 = cvt(*PTR_AT( x1, y1 ));                            \
                    p1 = t0 + a * (t1 - t0);                                \
                    p0 = p0 + b * (p1 - p0);                                \
                    dst[x] = cast_macro(p0);                                \
                }                                                           \
                else                                                        \
                {                                                           \
                    dst[x] = fv;                                            \
                }                                                           \
            }                                                               \
        }                                                                   \
    }                                                                       \
                                                                            \
    return CV_OK;                                                           \
}


#define ICV_DEF_GET_QUADRANGLE_SUB_PIX_FUNC_C3( flavor, srctype, dsttype,   \
                                                worktype, cast_macro, cvt ) \
IPCVAPI_IMPL( CvStatus, icvGetQuadrangleSubPix_##flavor##_C3R,              \
              ( const srctype * src, int src_step, CvSize src_size,         \
                dsttype *dst, int dst_step, CvSize win_size,                \
                const float *matrix, int fill_outliers,                     \
                dsttype* fillval ),                                         \
                (src, src_step, src_size, dst, dst_step, win_size,          \
                matrix, fill_outliers, fillval) )                           \
{                                                                           \
    int x, y, x0, y0, x1, y1;                                               \
    float  A11 = matrix[0], A12 = matrix[1], b1 = matrix[2] - 0.5f;         \
    float  A21 = matrix[3], A22 = matrix[4], b2 = matrix[5] - 0.5f;         \
    int left = -(win_size.width >> 1), right = win_size.width + left - 1;   \
    int top = -(win_size.height >> 1), bottom = win_size.height + top - 1;  \
                                                                            \
    src_step /= sizeof(srctype);                                            \
    dst_step /= sizeof(dsttype);                                            \
                                                                            \
    dst -= left*3;                                                          \
                                                                            \
    for( y = top; y <= bottom; y++, dst += dst_step )                       \
    {                                                                       \
        float xs = GET_X( left, y );                                        \
        float ys = GET_Y( left, y );                                        \
        float xe = GET_X( right, y );                                       \
        float ye = GET_Y( right, y );                                       \
                                                                            \
        if( (unsigned)cvFloor(xs) < (unsigned)(src_size.width - 1) &&       \
            (unsigned)cvFloor(ys) < (unsigned)(src_size.height - 1) &&      \
            (unsigned)cvFloor(xe) < (unsigned)(src_size.width - 1) &&       \
            (unsigned)cvFloor(ye) < (unsigned)(src_size.height - 1))        \
        {                                                                   \
            for( x = left; x <= right; x++ )                                \
            {                                                               \
                worktype p0, p1;                                            \
                float a, b;                                                 \
                                                                            \
                int ixs = cvFloor( xs );                                    \
                int iys = cvFloor( ys );                                    \
                const srctype *ptr = PTR_AT( ixs*3, iys );                  \
                                                                            \
                a = xs - ixs;                                               \
                b = ys - iys;                                               \
                                                                            \
                xs += A11;                                                  \
                ys += A21;                                                  \
                                                                            \
                p0 = cvt(ptr[0]) + a * (cvt(ptr[3]) - cvt(ptr[0]));         \
                p1 = cvt(ptr[src_step]) + a * (cvt(ptr[src_step + 3]) -     \
                                              cvt(ptr[src_step]));          \
                dst[x*3] = cast_macro(p0 + b * (p1 - p0));                  \
                                                                            \
                p0 = cvt(ptr[1]) + a * (cvt(ptr[4]) - cvt(ptr[1]));         \
                p1 = cvt(ptr[src_step+1]) + a * (cvt(ptr[src_step + 4]) -   \
                                                cvt(ptr[src_step + 1]));    \
                dst[x*3+1] = cast_macro(p0 + b * (p1 - p0));                \
                                                                            \
                p0 = cvt(ptr[2]) + a * (cvt(ptr[5]) - cvt(ptr[2]));         \
                p1 = cvt(ptr[src_step+2]) + a * (cvt(ptr[src_step + 5]) -   \
                                                cvt(ptr[src_step + 2]));    \
                dst[x*3+2] = cast_macro(p0 + b * (p1 - p0));                \
            }                                                               \
        }                                                                   \
        else                                                                \
        {                                                                   \
            for( x = left; x <= right; x++ )                                \
            {                                                               \
                worktype p0, p1;                                            \
                float a, b;                                                 \
                                                                            \
                int ixs = cvFloor( xs );                                    \
                int iys = cvFloor( ys );                                    \
                                                                            \
                a = xs - ixs;                                               \
                b = ys - iys;                                               \
                                                                            \
                xs += A11;                                                  \
                ys += A21;                                                  \
                                                                            \
                if( (unsigned)ixs < (unsigned)(src_size.width - 1) &&       \
                    (unsigned)iys < (unsigned)(src_size.height - 1) )       \
                {                                                           \
                    const srctype *ptr = PTR_AT( ixs*3, iys );              \
                                                                            \
                    p0 = cvt(ptr[0]) + a * (cvt(ptr[3]) - cvt(ptr[0]));     \
                    p1 = cvt(ptr[src_step]) + a * (cvt(ptr[src_step + 3]) - \
                                                   cvt(ptr[src_step]));     \
                    dst[x*3] = cast_macro(p0 + b * (p1 - p0));              \
                                                                            \
                    p0 = cvt(ptr[1]) + a * (cvt(ptr[4]) - cvt(ptr[1]));     \
                    p1 = cvt(ptr[src_step+1]) + a * (cvt(ptr[src_step + 4])-\
                                                    cvt(ptr[src_step + 1]));\
                    dst[x*3+1] = cast_macro(p0 + b * (p1 - p0));            \
                                                                            \
                    p0 = cvt(ptr[2]) + a * (cvt(ptr[5]) - cvt(ptr[2]));     \
                    p1 = cvt(ptr[src_step+2]) + a * (cvt(ptr[src_step + 5])-\
                                                    cvt(ptr[src_step + 2]));\
                    dst[x*3+2] = cast_macro(p0 + b * (p1 - p0));            \
                }                                                           \
                else if( !fill_outliers )                                   \
                {                                                           \
                    /* the slowest branch */                                \
                    worktype t0, t1;                                        \
                                                                            \
                    x0 = CLIP_X( ixs );                                     \
                    y0 = CLIP_Y( iys );                                     \
                    x1 = CLIP_X( ixs + 1 );                                 \
                    y1 = CLIP_Y( iys + 1 );                                 \
                                                                            \
                    t0 = cvt(*PTR_AT( x0*3, y0 ));                          \
                    t1 = cvt(*PTR_AT( x1*3, y0 ));                          \
                    p0 = t0 + a * (t1 - t0);                                \
                                                                            \
                    t0 = cvt(*PTR_AT( x0*3, y1 ));                          \
                    t1 = cvt(*PTR_AT( x1*3, y1 ));                          \
                    p1 = t0 + a * (t1 - t0);                                \
                    p0 = p0 + b * (p1 - p0);                                \
                    dst[x*3] = cast_macro(p0);                              \
                                                                            \
                    t0 = cvt(*PTR_AT( x0*3+1, y0 ));                        \
                    t1 = cvt(*PTR_AT( x1*3+1, y0 ));                        \
                    p0 = t0 + a * (t1 - t0);                                \
                                                                            \
                    t0 = cvt(*PTR_AT( x0*3+1, y1 ));                        \
                    t1 = cvt(*PTR_AT( x1*3+1, y1 ));                        \
                    p1 = t0 + a * (t1 - t0);                                \
                    p0 = p0 + b * (p1 - p0);                                \
                    dst[x*3+1] = cast_macro(p0);                            \
                                                                            \
                    t0 = cvt(*PTR_AT( x0*3+2, y0 ));                        \
                    t1 = cvt(*PTR_AT( x1*3+2, y0 ));                        \
                    p0 = t0 + a * (t1 - t0);                                \
                                                                            \
                    t0 = cvt(*PTR_AT( x0*3+2, y1 ));                        \
                    t1 = cvt(*PTR_AT( x1*3+2, y1 ));                        \
                    p1 = t0 + a * (t1 - t0);                                \
                    p0 = p0 + b * (p1 - p0);                                \
                    dst[x*3+2] = cast_macro(p0);                            \
                }                                                           \
                else                                                        \
                {                                                           \
                    dst[x*3] = fillval[0];                                  \
                    dst[x*3+1] = fillval[1];                                \
                    dst[x*3+2] = fillval[2];                                \
                }                                                           \
            }                                                               \
        }                                                                   \
    }                                                                       \
                                                                            \
    return CV_OK;                                                           \
}


#define ICV_32F8U(x)  ((uchar)cvRound(x))

ICV_DEF_GET_QUADRANGLE_SUB_PIX_FUNC( 8u, uchar, uchar, float, ICV_32F8U, CV_8TO32F )
ICV_DEF_GET_QUADRANGLE_SUB_PIX_FUNC( 32f, float, float, float, CV_NOP, CV_NOP )
ICV_DEF_GET_QUADRANGLE_SUB_PIX_FUNC( 8u32f, uchar, float, float, CV_NOP, CV_8TO32F )

ICV_DEF_GET_QUADRANGLE_SUB_PIX_FUNC_C3( 8u, uchar, uchar, float, ICV_32F8U, CV_8TO32F )
ICV_DEF_GET_QUADRANGLE_SUB_PIX_FUNC_C3( 32f, float, float, float, CV_NOP, CV_NOP )
ICV_DEF_GET_QUADRANGLE_SUB_PIX_FUNC_C3( 8u32f, uchar, float, float, CV_NOP, CV_8TO32F )

ICV_DEF_INIT_SUBPIX_TAB( GetQuadrangleSubPix, C1R )
ICV_DEF_INIT_SUBPIX_TAB( GetQuadrangleSubPix, C3R )

typedef CvStatus (CV_STDCALL *CvGetQuadrangleSubPixFunc)(
                                         const void* src, int src_step,
                                         CvSize src_size, void* dst,
                                         int dst_step, CvSize win_size,
                                         const float* matrix, int fill_outliers,
                                         const void* fillvalue );

CV_IMPL void
cvGetQuadrangleSubPix( const void* srcarr, void* dstarr,
                       const CvMat* mat, int fillOutliers,
                       CvScalar fillValue )
{
    static  CvFuncTable  gq_tab[2];
    static  int inittab = 0;
    CV_FUNCNAME( "cvGetQuadrangleSubPix" );

    __BEGIN__;

    CvMat srcstub, *src = (CvMat*)srcarr;
    CvMat dststub, *dst = (CvMat*)dstarr;
    CvSize src_size, dst_size;
    CvGetQuadrangleSubPixFunc func;
    double buf[12];
    int cn;

    if( !inittab )
    {
        icvInitGetQuadrangleSubPixC1RTable( gq_tab + 0 );
        icvInitGetQuadrangleSubPixC3RTable( gq_tab + 1 );
        inittab = 1;
    }

    if( !CV_IS_MAT(src))
        CV_CALL( src = cvGetMat( src, &srcstub ));

    if( !CV_IS_MAT(dst))
        CV_CALL( dst = cvGetMat( dst, &dststub ));

    if( !CV_IS_MAT(mat))
        CV_ERROR( CV_StsBadArg, "map matrix is not valid" );

    cn = CV_MAT_CN( src->type );

    if( (cn != 1 && cn != 3) || !CV_ARE_CNS_EQ( src, dst ))
        CV_ERROR( CV_StsUnsupportedFormat, "" );

    src_size = cvGetMatSize( src );
    dst_size = cvGetMatSize( dst );

    /*if( dst_size.width > src_size.width || dst_size.height > src_size.height )
        CV_ERROR( CV_StsBadSize, "destination ROI must not be larger than source ROI" );*/

    CV_CALL( cvScalarToRawData( &fillValue, buf, dst->type, 1 ));
    
    if( !CV_IS_MAT_CONT( mat->type ) ||
        CV_MAT_DEPTH( mat->type ) != CV_32F ||
        mat->width*mat->height != 6 )
        CV_ERROR( CV_StsBadArg, "Matrix argument must be continuous array of 6 floats" );

    if( CV_ARE_DEPTHS_EQ( src, dst ))
    {
        func = (CvGetQuadrangleSubPixFunc)(gq_tab[cn != 1].fn_2d[CV_MAT_DEPTH(src->type)]);
    }
    else
    {
        if( CV_MAT_DEPTH( src->type ) != CV_8U || CV_MAT_DEPTH( dst->type ) != CV_32F )
            CV_ERROR( CV_StsUnsupportedFormat, "" );

        func = (CvGetQuadrangleSubPixFunc)(gq_tab[cn != 1].fn_2d[1]);
    }

    if( !func )
        CV_ERROR( CV_StsUnsupportedFormat, "" );

    IPPI_CALL( func( src->data.ptr, src->step, src_size,
                     dst->data.ptr, dst->step, dst_size,
                     mat->data.fl, fillOutliers, buf ));

    __END__;
}


/* End of file. */
