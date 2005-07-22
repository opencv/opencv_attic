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

CV_IMPL CvSeq* cvPointSeqFromMat( int seq_kind, const CvArr* arr,
                                  CvContour* contour_header, CvSeqBlock* block )
{
    CvSeq* contour = 0;

    CV_FUNCNAME( "cvPointSeqFromMat" );

    assert( arr != 0 && contour_header != 0 && block != 0 );

    __BEGIN__;
    
    int eltype;
    CvMat* mat = (CvMat*)arr;
    
    if( !CV_IS_MAT( mat ))
        CV_ERROR( CV_StsBadArg, "Input array is not a valid matrix" ); 

    eltype = CV_MAT_TYPE( mat->type );
    if( eltype != CV_32SC2 && eltype != CV_32FC2 )
        CV_ERROR( CV_StsUnsupportedFormat,
        "The matrix can not be converted to point sequence because of "
        "inappropriate element type" );

    if( mat->width != 1 && mat->height != 1 || !CV_IS_MAT_CONT(mat->type))
        CV_ERROR( CV_StsBadArg,
        "The matrix converted to point sequence must be "
        "1-dimensional and continuous" );

    CV_CALL( cvMakeSeqHeaderForArray(
            (seq_kind & (CV_SEQ_KIND_MASK|CV_SEQ_FLAG_CLOSED)) | eltype,
            sizeof(CvContour), CV_ELEM_SIZE(eltype), mat->data.ptr,
            mat->width*mat->height, (CvSeq*)contour_header, block ));

    contour = (CvSeq*)contour_header;

    __END__;

    return contour;
}


/*F///////////////////////////////////////////////////////////////////////////////////////
//    Names:      icvJacobiEigens_32f, icvJacobiEigens_64d
//    Purpose:    Eigenvalues & eigenvectors calculation of a symmetric matrix:
//                A Vi  =  Ei Vi
//    Context:   
//    Parameters: A(n, n) - source symmetric matrix (n - rows & columns number),
//                V(n, n) - matrix of its eigenvectors 
//                          (i-th row is an eigenvector Vi),
//                E(n)    - vector of its eigenvalues
//                          (i-th element is an eigenvalue Ei),
//                eps     - accuracy of diagonalization.
//               
//    Returns:
//    CV_NO_ERROR or error code     
//    Notes:
//        1. The functions destroy source matrix A, so if you need it further, you
//           have to copy it before the processing.
//        2. Eigenvalies and eigenvectors are sorted in Ei absolute value descending.
//        3. Calculation time depends on eps value. If the time isn't very important,
//           we recommend to set eps = 0.
//F*/

/*=========================== Single precision function ================================*/

CvStatus CV_STDCALL icvJacobiEigens_32f(float *A, float *V, float *E, int n, float eps)
{
    int i, j, k, ind, iters = 0;
    float *AA = A, *VV = V;
    double Amax, anorm = 0, ax;

    if( A == NULL || V == NULL || E == NULL )
        return CV_NULLPTR_ERR;
    if( n <= 0 )
        return CV_BADSIZE_ERR;
    if( eps < DBL_EPSILON )
        eps = DBL_EPSILON;

    /*-------- Prepare --------*/
    for( i = 0; i < n; i++, VV += n, AA += n )
    {
        for( j = 0; j < i; j++ )
        {
            double Am = AA[j];

            anorm += Am * Am;
        }
        for( j = 0; j < n; j++ )
            VV[j] = 0.f;
        VV[i] = 1.f;
    }

    anorm = sqrt( anorm + anorm );
    ax = anorm * eps / n;
    Amax = anorm;

    while( Amax > ax && iters++ < 100 )
    {
        Amax /= n;
        do                      /* while (ind) */
        {
            int p, q;
            float *V1 = V, *A1 = A;

            ind = 0;
            for( p = 0; p < n - 1; p++, A1 += n, V1 += n )
            {
                float *A2 = A + n * (p + 1), *V2 = V + n * (p + 1);

                for( q = p + 1; q < n; q++, A2 += n, V2 += n )
                {
                    double x, y, c, s, c2, s2, a;
                    float *A3, Apq = A1[q], App, Aqq, Aip, Aiq, Vpi, Vqi;

                    if( fabs( Apq ) < Amax )
                        continue;

                    ind = 1;

                    /*---- Calculation of rotation angle's sine & cosine ----*/
                    App = A1[p];
                    Aqq = A2[q];
                    y = 5.0e-1 * (App - Aqq);
                    x = -Apq / sqrt( (double)Apq * Apq + (double)y * y );
                    if( y < 0.0 )
                        x = -x;
                    s = x / sqrt( 2.0 * (1.0 + sqrt( 1.0 - (double)x * x )));
                    s2 = s * s;
                    c = sqrt( 1.0 - s2 );
                    c2 = c * c;
                    a = 2.0 * Apq * c * s;

                    /*---- Apq annulation ----*/
                    A3 = A;
                    for( i = 0; i < p; i++, A3 += n )
                    {
                        Aip = A3[p];
                        Aiq = A3[q];
                        Vpi = V1[i];
                        Vqi = V2[i];
                        A3[p] = (float) (Aip * c - Aiq * s);
                        A3[q] = (float) (Aiq * c + Aip * s);
                        V1[i] = (float) (Vpi * c - Vqi * s);
                        V2[i] = (float) (Vqi * c + Vpi * s);
                    }
                    for( ; i < q; i++, A3 += n )
                    {
                        Aip = A1[i];
                        Aiq = A3[q];
                        Vpi = V1[i];
                        Vqi = V2[i];
                        A1[i] = (float) (Aip * c - Aiq * s);
                        A3[q] = (float) (Aiq * c + Aip * s);
                        V1[i] = (float) (Vpi * c - Vqi * s);
                        V2[i] = (float) (Vqi * c + Vpi * s);
                    }
                    for( ; i < n; i++ )
                    {
                        Aip = A1[i];
                        Aiq = A2[i];
                        Vpi = V1[i];
                        Vqi = V2[i];
                        A1[i] = (float) (Aip * c - Aiq * s);
                        A2[i] = (float) (Aiq * c + Aip * s);
                        V1[i] = (float) (Vpi * c - Vqi * s);
                        V2[i] = (float) (Vqi * c + Vpi * s);
                    }
                    A1[p] = (float) (App * c2 + Aqq * s2 - a);
                    A2[q] = (float) (App * s2 + Aqq * c2 + a);
                    A1[q] = A2[p] = 0.0f;
                }               /*q */
            }                   /*p */
        }
        while( ind );
        Amax /= n;
    }                           /* while ( Amax > ax ) */

    for( i = 0, k = 0; i < n; i++, k += n + 1 )
        E[i] = A[k];
    /*printf(" M = %d\n", M); */

    /* -------- ordering -------- */
    for( i = 0; i < n; i++ )
    {
        int m = i;
        float Em = (float) fabs( E[i] );

        for( j = i + 1; j < n; j++ )
        {
            float Ej = (float) fabs( E[j] );

            m = (Em < Ej) ? j : m;
            Em = (Em < Ej) ? Ej : Em;
        }
        if( m != i )
        {
            int l;
            float b = E[i];

            E[i] = E[m];
            E[m] = b;
            for( j = 0, k = i * n, l = m * n; j < n; j++, k++, l++ )
            {
                b = V[k];
                V[k] = V[l];
                V[l] = b;
            }
        }
    }

    return CV_NO_ERR;
}

/*=========================== Double precision function ================================*/

CvStatus CV_STDCALL icvJacobiEigens_64d(double *A, double *V, double *E, int n, double eps)
{
    int i, j, k, p, q, ind, iters = 0;
    double *A1 = A, *V1 = V, *A2 = A, *V2 = V;
    double Amax = 0.0, anorm = 0.0, ax;

    if( A == NULL || V == NULL || E == NULL )
        return CV_NULLPTR_ERR;
    if( n <= 0 )
        return CV_BADSIZE_ERR;
    if( eps < DBL_EPSILON )
        eps = DBL_EPSILON;

    /*-------- Prepare --------*/
    for( i = 0; i < n; i++, V1 += n, A1 += n )
    {
        for( j = 0; j < i; j++ )
        {
            double Am = A1[j];

            anorm += Am * Am;
        }
        for( j = 0; j < n; j++ )
            V1[j] = 0.0;
        V1[i] = 1.0;
    }

    anorm = sqrt( anorm + anorm );
    ax = anorm * eps / n;
    Amax = anorm;

    while( Amax > ax && iters++ < 100 )
    {
        Amax /= n;
        do                      /* while (ind) */
        {
            ind = 0;
            A1 = A;
            V1 = V;
            for( p = 0; p < n - 1; p++, A1 += n, V1 += n )
            {
                A2 = A + n * (p + 1);
                V2 = V + n * (p + 1);
                for( q = p + 1; q < n; q++, A2 += n, V2 += n )
                {
                    double x, y, c, s, c2, s2, a;
                    double *A3, Apq, App, Aqq, App2, Aqq2, Aip, Aiq, Vpi, Vqi;

                    if( fabs( A1[q] ) < Amax )
                        continue;
                    Apq = A1[q];

                    ind = 1;

                    /*---- Calculation of rotation angle's sine & cosine ----*/
                    App = A1[p];
                    Aqq = A2[q];
                    y = 5.0e-1 * (App - Aqq);
                    x = -Apq / sqrt( Apq * Apq + (double)y * y );
                    if( y < 0.0 )
                        x = -x;
                    s = x / sqrt( 2.0 * (1.0 + sqrt( 1.0 - (double)x * x )));
                    s2 = s * s;
                    c = sqrt( 1.0 - s2 );
                    c2 = c * c;
                    a = 2.0 * Apq * c * s;

                    /*---- Apq annulation ----*/
                    A3 = A;
                    for( i = 0; i < p; i++, A3 += n )
                    {
                        Aip = A3[p];
                        Aiq = A3[q];
                        Vpi = V1[i];
                        Vqi = V2[i];
                        A3[p] = Aip * c - Aiq * s;
                        A3[q] = Aiq * c + Aip * s;
                        V1[i] = Vpi * c - Vqi * s;
                        V2[i] = Vqi * c + Vpi * s;
                    }
                    for( ; i < q; i++, A3 += n )
                    {
                        Aip = A1[i];
                        Aiq = A3[q];
                        Vpi = V1[i];
                        Vqi = V2[i];
                        A1[i] = Aip * c - Aiq * s;
                        A3[q] = Aiq * c + Aip * s;
                        V1[i] = Vpi * c - Vqi * s;
                        V2[i] = Vqi * c + Vpi * s;
                    }
                    for( ; i < n; i++ )
                    {
                        Aip = A1[i];
                        Aiq = A2[i];
                        Vpi = V1[i];
                        Vqi = V2[i];
                        A1[i] = Aip * c - Aiq * s;
                        A2[i] = Aiq * c + Aip * s;
                        V1[i] = Vpi * c - Vqi * s;
                        V2[i] = Vqi * c + Vpi * s;
                    }
                    App2 = App * c2 + Aqq * s2 - a;
                    Aqq2 = App * s2 + Aqq * c2 + a;
                    A1[p] = App2;
                    A2[q] = Aqq2;
                    A1[q] = A2[p] = 0.0;
                }               /*q */
            }                   /*p */
        }
        while( ind );
    }                           /* while ( Amax > ax ) */

    for( i = 0, k = 0; i < n; i++, k += n + 1 )
        E[i] = A[k];

    /* -------- ordering -------- */
    for( i = 0; i < n; i++ )
    {
        int m = i;
        double Em = fabs( E[i] );

        for( j = i + 1; j < n; j++ )
        {
            double Ej = fabs( E[j] );

            m = (Em < Ej) ? j : m;
            Em = (Em < Ej) ? Ej : Em;
        }
        if( m != i )
        {
            int l;
            double b = E[i];

            E[i] = E[m];
            E[m] = b;
            for( j = 0, k = i * n, l = m * n; j < n; j++, k++, l++ )
            {
                b = V[k];
                V[k] = V[l];
                V[l] = b;
            }
        }
    }

    return CV_NO_ERR;
}


#define ICV_COPY_REPLICATE_BORDER_FUNC( flavor, arrtype, cn )   \
IPCVAPI_IMPL( CvStatus,                                         \
icvCopyReplicateBorder_##flavor, (                              \
    const arrtype *src, int srcstep, CvSize srcroi,             \
    arrtype* dst, int dststep, CvSize dstroi, int top, int left ),\
    (src, srcstep, srcroi, dst, dststep, dstroi, top, left))    \
{                                                               \
    int i, j;                                                   \
    srcstep /= sizeof(src[0]);                                  \
    dststep /= sizeof(dst[0]);                                  \
    srcroi.width *= cn;                                         \
    dstroi.width *= cn;                                         \
    left *= cn;                                                 \
                                                                \
    for( i = 0; i < dstroi.height; i++, dst += dststep )        \
    {                                                           \
        memcpy( dst + left, src, srcroi.width*sizeof(src[0]) ); \
        for( j = left - 1; j >= 0; j-- )                        \
            dst[j] = dst[j + cn];                               \
        for( j = left + srcroi.width; j < dstroi.width; j++ )   \
            dst[j] = dst[j - cn];                               \
        if( i >= top && i < top + srcroi.height - 1 )           \
            src += srcstep;                                     \
    }                                                           \
                                                                \
    return CV_OK;                                               \
}


#define ICV_COPY_CONST_BORDER_FUNC_C1( flavor, arrtype )        \
IPCVAPI_IMPL( CvStatus,                                         \
icvCopyConstBorder_##flavor, (                                  \
    const arrtype *src, int srcstep, CvSize srcroi,             \
    arrtype* dst, int dststep, CvSize dstroi,                   \
    int top, int left, arrtype value ),                         \
    (src, srcstep, srcroi, dst, dststep, dstroi, top, left, value ))\
{                                                               \
    int i, j;                                                   \
    srcstep /= sizeof(src[0]);                                  \
    dststep /= sizeof(dst[0]);                                  \
                                                                \
    for( i = 0; i < dstroi.height; i++, dst += dststep )        \
    {                                                           \
        if( i == 0 || i == srcroi.height + top )                \
        {                                                       \
            int limit = i < top || i == srcroi.height + top ?   \
                        dstroi.width : left;                    \
            for( j = 0; j < limit; j++ )                        \
                dst[j] = value;                                 \
                                                                \
            if( limit == dstroi.width )                         \
                continue;                                       \
                                                                \
            for( j=srcroi.width+left; j < dstroi.width; j++ )   \
                dst[j] = value;                                 \
        }                                                       \
                                                                \
        if( i < top || i > srcroi.height + top )                \
            memcpy( dst, dst-dststep, dstroi.width*sizeof(dst[0]));\
        else                                                    \
        {                                                       \
            if( i > 0 )                                         \
            {                                                   \
                for( j = 0; j < left; j++ )                     \
                    dst[j] = dst[j - dststep];                  \
                for( j = srcroi.width + left; j < dstroi.width; j++ ) \
                    dst[j] = dst[j - dststep];                  \
            }                                                   \
            memcpy( dst, src, srcroi.width*sizeof(dst[0]));     \
            src += srcstep;                                     \
        }                                                       \
    }                                                           \
                                                                \
    return CV_OK;                                               \
}


#define ICV_COPY_CONST_BORDER_FUNC_CN( flavor, arrtype, cn )    \
IPCVAPI_IMPL( CvStatus,                                         \
icvCopyConstBorder_##flavor, (                                  \
    const arrtype *src, int srcstep, CvSize srcroi,             \
    arrtype* dst, int dststep, CvSize dstroi,                   \
    int top, int left, const arrtype* value ),                  \
    (src, srcstep, srcroi, dst, dststep, dstroi, top, left, value ))\
{                                                               \
    int i, j, k;                                                \
    srcstep /= sizeof(src[0]);                                  \
    dststep /= sizeof(dst[0]);                                  \
    srcroi.width *= cn;                                         \
    dstroi.width *= cn;                                         \
    left *= cn;                                                 \
                                                                \
    for( i = 0; i < dstroi.height; i++, dst += dststep )        \
    {                                                           \
        if( i == 0 || i == srcroi.height + top )                \
        {                                                       \
            int limit = i < top || i == srcroi.height + top ?   \
                        dstroi.width : left;                    \
            for( j = 0; j < limit; j += cn )                    \
                for( k = 0; k < cn; k++ )                       \
                    dst[j+k] = value[k];                        \
                                                                \
            if( limit == dstroi.width )                         \
                continue;                                       \
                                                                \
            for( j=srcroi.width+left; j < dstroi.width; j+=cn ) \
                for( k = 0; k < cn; k++ )                       \
                    dst[j+k] = value[k];                        \
        }                                                       \
                                                                \
        if( i < top || i > srcroi.height + top )                \
            memcpy( dst, dst-dststep, dstroi.width*sizeof(dst[0]));\
        else                                                    \
        {                                                       \
            if( i > 0 )                                         \
            {                                                   \
                for( j = 0; j < left; j++ )                     \
                    dst[j] = dst[j - dststep];                  \
                for( j = srcroi.width + left; j < dstroi.width; j++ ) \
                    dst[j] = dst[j - dststep];                  \
            }                                                   \
            memcpy( dst, src, srcroi.width*sizeof(dst[0]));     \
            src += srcstep;                                     \
        }                                                       \
    }                                                           \
                                                                \
    return CV_OK;                                               \
}


ICV_COPY_REPLICATE_BORDER_FUNC( 8u_C1R, uchar, 1 )  // 1
ICV_COPY_REPLICATE_BORDER_FUNC( 16s_C1R, ushort, 1 )// 2
ICV_COPY_REPLICATE_BORDER_FUNC( 8u_C3R, uchar, 3 )  // 3
ICV_COPY_REPLICATE_BORDER_FUNC( 32s_C1R, int, 1 )   // 4
ICV_COPY_REPLICATE_BORDER_FUNC( 16s_C3R, ushort, 3 )// 6
ICV_COPY_REPLICATE_BORDER_FUNC( 16s_C4R, int, 2 )   // 8
ICV_COPY_REPLICATE_BORDER_FUNC( 32s_C3R, int, 3 )   // 12
ICV_COPY_REPLICATE_BORDER_FUNC( 32s_C4R, int, 4 )   // 16
ICV_COPY_REPLICATE_BORDER_FUNC( 64f_C3R, int, 6 )   // 24
ICV_COPY_REPLICATE_BORDER_FUNC( 64f_C4R, int, 8 )   // 32

ICV_COPY_CONST_BORDER_FUNC_C1( 8u_C1R, uchar )      // 1
ICV_COPY_CONST_BORDER_FUNC_C1( 16s_C1R, ushort )    // 2
ICV_COPY_CONST_BORDER_FUNC_C1( 32s_C1R, int )       // 4

ICV_COPY_CONST_BORDER_FUNC_CN( 8u_C3R, uchar, 3 )   // 3
ICV_COPY_CONST_BORDER_FUNC_CN( 16s_C3R, ushort, 3 ) // 6
ICV_COPY_CONST_BORDER_FUNC_CN( 16s_C4R, int, 2 )    // 8
ICV_COPY_CONST_BORDER_FUNC_CN( 32s_C3R, int, 3 )    // 12
ICV_COPY_CONST_BORDER_FUNC_CN( 32s_C4R, int, 4 )    // 16
ICV_COPY_CONST_BORDER_FUNC_CN( 64f_C3R, int, 6 )    // 24
ICV_COPY_CONST_BORDER_FUNC_CN( 64f_C4R, int, 8 )    // 32


CvCopyNonConstBorderFunc icvGetCopyNonConstBorderFunc( int pix_size, int /*bordertype*/ )
{
    CvBtFuncTable borderrepl_tab;
    int initflag = 0;

    assert( (unsigned)pix_size <= 4*sizeof(double) );

    if( !initflag )
    {
        borderrepl_tab.fn_2d[1] = (void*)icvCopyReplicateBorder_8u_C1R;
        borderrepl_tab.fn_2d[2] = (void*)icvCopyReplicateBorder_16s_C1R;
        borderrepl_tab.fn_2d[3] = (void*)icvCopyReplicateBorder_8u_C3R;
        borderrepl_tab.fn_2d[4] = (void*)icvCopyReplicateBorder_32s_C1R;
        borderrepl_tab.fn_2d[6] = (void*)icvCopyReplicateBorder_16s_C3R;
        borderrepl_tab.fn_2d[8] = (void*)icvCopyReplicateBorder_16s_C4R;
        borderrepl_tab.fn_2d[12] = (void*)icvCopyReplicateBorder_32s_C3R;
        borderrepl_tab.fn_2d[16] = (void*)icvCopyReplicateBorder_32s_C4R;
        borderrepl_tab.fn_2d[24] = (void*)icvCopyReplicateBorder_64f_C3R;
        borderrepl_tab.fn_2d[32] = (void*)icvCopyReplicateBorder_64f_C4R;
        initflag = 1;
    }

    return (CvCopyNonConstBorderFunc)borderrepl_tab.fn_2d[pix_size];
}


CvCopyConstBorderFunc_Cn icvGetCopyConstBorderFunc_Cn( int pix_size )
{
    CvBtFuncTable borderconst_tab;
    int initflag = 0;

    assert( (unsigned)pix_size <= 4*sizeof(double) );

    if( !initflag )
    {
        borderconst_tab.fn_2d[1] = 0; // manual handling
        borderconst_tab.fn_2d[2] = 0; // manual handling
        borderconst_tab.fn_2d[3] = (void*)icvCopyConstBorder_8u_C3R;
        borderconst_tab.fn_2d[4] = 0; // manual handling
        borderconst_tab.fn_2d[6] = (void*)icvCopyConstBorder_16s_C3R;
        borderconst_tab.fn_2d[8] = (void*)icvCopyConstBorder_16s_C4R;
        borderconst_tab.fn_2d[12] = (void*)icvCopyConstBorder_32s_C3R;
        borderconst_tab.fn_2d[16] = (void*)icvCopyConstBorder_32s_C4R;
        borderconst_tab.fn_2d[24] = (void*)icvCopyConstBorder_64f_C3R;
        borderconst_tab.fn_2d[32] = (void*)icvCopyConstBorder_64f_C4R;
        initflag = 1;
    }

    return (CvCopyConstBorderFunc_Cn)borderconst_tab.fn_2d[pix_size];
}


CV_IMPL void
cvCopyMakeBorder( const CvArr* srcarr, CvArr* dstarr, CvPoint offset,
                  int bordertype, CvScalar value )
{
    CV_FUNCNAME( "cvCopyMakeBorder" );

    __BEGIN__;

    CvMat srcstub, *src = (CvMat*)srcarr;
    CvMat dststub, *dst = (CvMat*)dstarr;
    CvSize srcsize, dstsize;
    int srcstep, dststep;
    int pix_size, type;

    if( !CV_IS_MAT(src) )
        CV_CALL( src = cvGetMat( src, &srcstub ));
    
    if( !CV_IS_MAT(dst) )    
        CV_CALL( dst = cvGetMat( dst, &dststub ));

    if( offset.x < 0 || offset.y < 0 )
        CV_ERROR( CV_StsOutOfRange, "Offset (left/top border width) is negative" );

    if( src->rows + offset.y > dst->rows || src->cols + offset.x > dst->cols )
        CV_ERROR( CV_StsBadSize, "Source array is too big or destination array is too small" );

    if( !CV_ARE_TYPES_EQ( src, dst ))
        CV_ERROR( CV_StsUnmatchedFormats, "" );

    type = CV_MAT_TYPE(src->type);
    pix_size = CV_ELEM_SIZE(type);
    srcsize = cvGetMatSize(src);
    dstsize = cvGetMatSize(dst);
    srcstep = src->step;
    dststep = dst->step;
    if( srcstep == 0 )
        srcstep = CV_STUB_STEP;
    if( dststep == 0 )
        dststep = CV_STUB_STEP;

    if( bordertype == IPL_BORDER_REPLICATE )
    {
        CvCopyNonConstBorderFunc func =
            icvGetCopyNonConstBorderFunc( pix_size, IPL_BORDER_REPLICATE );
        if( !func )
            CV_ERROR( CV_StsUnsupportedFormat, "" );

        IPPI_CALL( func( src->data.ptr, srcstep, srcsize,
                         dst->data.ptr, dststep, dstsize,
                         offset.y, offset.x ));
    }
    else if( bordertype == IPL_BORDER_CONSTANT )
    {
        double buf[4];
        cvScalarToRawData( &value, buf, src->type, 0 );

        if( pix_size == 1 )
        {
            IPPI_CALL( icvCopyConstBorder_8u_C1R( src->data.ptr, srcstep, srcsize,
                                                  dst->data.ptr, dststep, dstsize,
                                                  offset.y, offset.x, ((uchar*)buf)[0] ));
        }
        else if( pix_size == 2 )
        {
            IPPI_CALL( icvCopyConstBorder_16s_C1R( (ushort*)src->data.ptr, srcstep, srcsize,
                                                   (ushort*)dst->data.ptr, dststep, dstsize,
                                                   offset.y, offset.x, ((ushort*)buf)[0] ));
        }
        else if( pix_size == 4 )
        {
            IPPI_CALL( icvCopyConstBorder_32s_C1R( src->data.i, srcstep, srcsize,
                                                   dst->data.i, dststep, dstsize,
                                                   offset.y, offset.x, ((int*)buf)[0] ));
        }
        else
        {
            CvCopyConstBorderFunc_Cn func =
                icvGetCopyConstBorderFunc_Cn( pix_size );
            if( !func )
                CV_ERROR( CV_StsUnsupportedFormat, "" );

            IPPI_CALL( func( src->data.ptr, srcstep, srcsize,
                             dst->data.ptr, dststep, dstsize,
                             offset.y, offset.x, buf ));
        }
    }
    else
        CV_ERROR( CV_StsBadFlag, "Unknown/unsupported border type" );
    
    __END__;
}

/* End of file. */
