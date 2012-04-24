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

CvSeq* icvPointSeqFromMat( int seq_kind, const CvArr* arr,
                           CvContour* contour_header, CvSeqBlock* block )
{
    CvSeq* contour = 0;

    CV_FUNCNAME( "icvPointSeqFromMat" );

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
    int i, j, k, ind;
    float *AA = A, *VV = V;
    double Amax, anorm = 0, ax;

    if( A == NULL || V == NULL || E == NULL )
        return CV_NULLPTR_ERR;
    if( n <= 0 )
        return CV_BADSIZE_ERR;
    if( eps < 1.0e-7f )
        eps = 1.0e-7f;

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

    while( Amax > ax )
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
    int i, j, k, p, q, ind;
    double *A1 = A, *V1 = V, *A2 = A, *V2 = V;
    double Amax = 0.0, anorm = 0.0, ax;

    if( A == NULL || V == NULL || E == NULL )
        return CV_NULLPTR_ERR;
    if( n <= 0 )
        return CV_BADSIZE_ERR;
    if( eps < 1.0e-15 )
        eps = 1.0e-15;

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

    while( Amax > ax )
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


/* 
   Finds L1 norm between two blocks.
*/
int
icvCmpBlocksL1_8u_C1( const uchar * vec1, const uchar * vec2, int len )
{
    int i, sum = 0;

    for( i = 0; i <= len - 4; i += 4 )
    {
        int t0 = abs(vec1[i] - vec2[i]);
        int t1 = abs(vec1[i + 1] - vec2[i + 1]);
        int t2 = abs(vec1[i + 2] - vec2[i + 2]);
        int t3 = abs(vec1[i + 3] - vec2[i + 3]);

        sum += t0 + t1 + t2 + t3;
    }

    for( ; i < len; i++ )
    {
        int t0 = abs(vec1[i] - vec2[i]);
        sum += t0;
    }

    return sum;
}


int64
icvCmpBlocksL2_8u_C1( const uchar * vec1, const uchar * vec2, int len )
{
    int i, s = 0;
    int64 sum = 0;

    for( i = 0; i <= len - 4; i += 4 )
    {
        int v = vec1[i] - vec2[i];
        int e = v * v;

        v = vec1[i + 1] - vec2[i + 1];
        e += v * v;
        v = vec1[i + 2] - vec2[i + 2];
        e += v * v;
        v = vec1[i + 3] - vec2[i + 3];
        e += v * v;
        sum += e;
    }

    for( ; i < len; i++ )
    {
        int v = vec1[i] - vec2[i];

        s += v * v;
    }

    return sum + s;
}


double
icvCmpBlocksL2_32f_C1( const float *vec1, const float *vec2, int len )
{
    double sum = 0;
    int i;

    for( i = 0; i <= len - 4; i += 4 )
    {
        double v0 = vec1[i] - vec2[i];
        double v1 = vec1[i + 1] - vec2[i + 1];
        double v2 = vec1[i + 2] - vec2[i + 2];
        double v3 = vec1[i + 3] - vec2[i + 3];

        sum += v0 * v0 + v1 * v1 + v2 * v2 + v3 * v3;
    }
    for( ; i < len; i++ )
    {
        double v = vec1[i] - vec2[i];

        sum += v * v;
    }
    return sum;
}


/* 
   Calculates cross correlation for two blocks.
*/
int64
icvCrossCorr_8u_C1( const uchar * vec1, const uchar * vec2, int len )
{
    int i, s = 0;
    int64 sum = 0;

    for( i = 0; i <= len - 4; i += 4 )
    {
        int e = vec1[i] * vec2[i];
        int v = vec1[i + 1] * vec2[i + 1];

        e += v;
        v = vec1[i + 2] * vec2[i + 2];
        e += v;
        v = vec1[i + 3] * vec2[i + 3];
        e += v;
        sum += e;
    }

    for( ; i < len; i++ )
    {
        s += vec1[i] * vec2[i];
    }

    return sum + s;
}

double
icvCrossCorr_32f_C1( const float *vec1, const float *vec2, int len )
{
    double sum = 0;
    int i;

    for( i = 0; i <= len - 4; i += 4 )
    {
        double v0 = vec1[i] * vec2[i];
        double v1 = vec1[i + 1] * vec2[i + 1];
        double v2 = vec1[i + 2] * vec2[i + 2];
        double v3 = vec1[i + 3] * vec2[i + 3];

        sum += v0 + v1 + v2 + v3;
    }
    for( ; i < len; i++ )
    {
        double v = vec1[i] * vec2[i];

        sum += v;
    }
    return sum;
}


/* 
   Calculates cross correlation for two blocks.
*/
int64
icvSumPixels_8u_C1( const uchar * vec, int len )
{
    int i, s = 0;
    int64 sum = 0;

    for( i = 0; i <= len - 4; i += 4 )
    {
        sum += vec[i] + vec[i + 1] + vec[i + 2] + vec[i + 3];
    }

    for( ; i < len; i++ )
    {
        s += vec[i];
    }

    return sum + s;
}

double
icvSumPixels_32f_C1( const float *vec, int len )
{
    double sum = 0;
    int i;

    for( i = 0; i <= len - 4; i += 4 )
    {
        sum += vec[i] + vec[i + 1] + vec[i + 2] + vec[i + 3];
    }

    for( ; i < len; i++ )
    {
        sum += vec[i];
    }
    return sum;
}

/* End of file. */
