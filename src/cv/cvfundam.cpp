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
#include "_cvwrap.h"
#include "_cvvm.h"


/*=====================================================================================*/

int
icvGaussMxN( double *A, double *B, int M, int N, double **solutions )
{
    int *variables;
    int row, swapi, i, i_best = 0, j, j_best = 0, t;
    double swapd, ratio, bigest;

    if( !A || !B || !M || !N )
        return -1;

    variables = (int *) icvAlloc( (long) N * sizeof( int ));

    if( variables == 0 )
        return -1;

    for( i = 0; i < N; i++ )
    {
        variables[i] = i;
    }                           /* for */

    /* -----  Direct way  ----- */

    for( row = 0; row < M; row++ )
    {

        bigest = 0;

        for( j = row; j < M; j++ )
        {                       /* search non null element */
            for( i = row; i < N; i++ )
            {

                if( fabs( A[j * N + i] ) > fabs( bigest ))
                {
                    bigest = A[j * N + i];
                    i_best = i;
                    j_best = j;
                }               /* if */
            }                   /* for */
        }                       /* for */

        if( REAL_ZERO( bigest ))
            break;              /* if all shank elements are null */

        if( j_best - row )
        {

            for( t = 0; t < N; t++ )
            {                   /* swap a rows */

                swapd = A[row * N + t];
                A[row * N + t] = A[j_best * N + t];
                A[j_best * N + t] = swapd;
            }                   /* for */

            swapd = B[row];
            B[row] = B[j_best];
            B[j_best] = swapd;
        }                       /* if */

        if( i_best - row )
        {

            for( t = 0; t < M; t++ )
            {                   /* swap a columns  */

                swapd = A[t * N + i_best];
                A[t * N + i_best] = A[t * N + row];
                A[t * N + row] = swapd;
            }                   /* for */

            swapi = variables[row];
            variables[row] = variables[i_best];
            variables[i_best] = swapi;
        }                       /* if */

        for( i = row + 1; i < M; i++ )
        {                       /* recounting A and B */

            ratio = -A[i * N + row] / A[row * N + row];
            B[i] += B[row] * ratio;

            for( j = N - 1; j >= row; j-- )
            {

                A[i * N + j] += A[row * N + j] * ratio;
            }                   /* for */
        }                       /* for */
    }                           /* for */

    if( row < M )
    {                           /* if rank(A)<M */

        for( j = row; j < M; j++ )
        {
            if( !REAL_ZERO( B[j] ))
            {

                icvFree( &variables );
                return -1;      /* if system is antithetic */
            }                   /* if */
        }                       /* for */

        M = row;                /* decreasing size of the task */
    }                           /* if */

    /* ----- Reverse way ----- */

    if( M < N )
    {                           /* if solution are not exclusive */

        *solutions = (double *) icvAlloc( ((N - M + 1) * N) * sizeof( double ));

        if( *solutions == 0 )
        {
            icvFree( &variables );
            return -1;
        }


        for( t = M; t <= N; t++ )
        {
            for( j = M; j < N; j++ )
            {

                (*solutions)[(t - M) * N + variables[j]] = (double) (t == j);
            }                   /* for */

            for( i = M - 1; i >= 0; i-- )
            {                   /* finding component of solution */

                if( t < N )
                {
                    (*solutions)[(t - M) * N + variables[i]] = 0;
                }
                else
                {
                    (*solutions)[(t - M) * N + variables[i]] = B[i] / A[i * N + i];
                }               /* if */

                for( j = i + 1; j < N; j++ )
                {

                    (*solutions)[(t - M) * N + variables[i]] -=
                        (*solutions)[(t - M) * N + variables[j]] * A[i * N + j] / A[i * N + i];
                }               /* for */
            }                   /* for */
        }                       /* for */

        icvFree( &variables );
        return N - M;
    }                           /* if */

    *solutions = (double *) icvAlloc( (N) * sizeof( double ));

    if( solutions == 0 )
        return -1;

    for( i = N - 1; i >= 0; i-- )
    {                           /* finding exclusive solution */

        (*solutions)[variables[i]] = B[i] / A[i * N + i];

        for( j = i + 1; j < N; j++ )
        {

            (*solutions)[variables[i]] -=
                (*solutions)[variables[j]] * A[i * N + j] / A[i * N + i];
        }                       /* for */
    }                           /* for */

    icvFree( &variables );
    return 0;

}                               /* icvGaussMxN */

/*=====================================================================================*/

CvStatus
icvGetCoof( double *f1, double *f2, double *a2, double *a1, double *a0 )
{
    double G[9], a3;
    int i;

    if( !f1 || !f2 || !a0 || !a1 || !a2 )
        return CV_BADFACTOR_ERR;

    for( i = 0; i < 9; i++ )
    {

        G[i] = f1[i] - f2[i];
    }                           /* for */

    a3 = icvDet( G );

    if( REAL_ZERO( a3 ))
        return CV_BADFACTOR_ERR;

    *a2 = 0;
    *a1 = 0;
    *a0 = icvDet( f2 );

    for( i = 0; i < 9; i++ )
    {

        *a2 += f2[i] * icvMinor( G, (int) (i % 3), (int) (i / 3) );
        *a1 += G[i] * icvMinor( f2, (int) (i % 3), (int) (i / 3) );
    }                           /* for */

    *a0 /= a3;
    *a1 /= a3;
    *a2 /= a3;

    return CV_NO_ERR;

}                               /* icvGetCoof */



/*======================================================================================*/

/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:    icvLMedS7
//    Purpose:
//      
//      
//    Context:
//    Parameters:
//     
//      
//      
//     
//      
//    
//     
//    Returns:
//      CV_NO_ERR if all Ok or error code
//    Notes:
//F*/

CvStatus
icvLMedS7( int *points1, int *points2, CvMatrix3 * matrix )
{                               /* Incorrect realization */
    CvStatus error = CV_NO_ERR;

/*    int         amount; */
    matrix = matrix;
    points1 = points1;
    points2 = points2;

/*    error = cs_Point7( points1, points2, matrix ); */
/*    error = icvPoint7    ( points1, points2, matrix,&amount ); */
    return error;

}                               /* icvLMedS7 */


/*======================================================================================*/

/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:    icvPoint7
//    Purpose:
//      
//      
//    Context:
//    Parameters:
//     
//      
//      
//     
//      
//    
//     
//    Returns:
//      CV_NO_ERR if all Ok or error code
//    Notes:
//F*/

CvStatus
icvPoint7( int *ml, int *mr, double *F, int *amount )
{
    double A[63], B[7];
    double *solutions;
    double a2, a1, a0;
    double squares[6];
    int i, j;

/*    int         amount; */
/*    float*     F; */

    CvStatus error = CV_BADFACTOR_ERR;

/*    F = (float*)matrix->m; */

    if( !ml || !mr || !F )
        return CV_BADFACTOR_ERR;

    for( i = 0; i < 7; i++ )
    {
        for( j = 0; j < 9; j++ )
        {

            A[i * 9 + j] = (double) ml[i * 3 + j / 3] * (double) mr[i * 3 + j % 3];
        }                       /* for */
        B[i] = 0;
    }                           /* for */

    *amount = 0;

    if( icvGaussMxN( A, B, 7, 9, &solutions ) == 2 )
    {
        if( icvGetCoef( solutions, solutions + 9, &a2, &a1, &a0 ) == CV_NO_ERR )
        {
            icvCubic( a2, a1, a0, squares );

            for( i = 0; i < 1; i++ )
            {

                if( REAL_ZERO( squares[i * 2 + 1] ))
                {

                    for( j = 0; j < 9; j++ )
                    {

                        F[*amount + j] = (float) (squares[i] * solutions[j] +
                                                  (1 - squares[i]) * solutions[j + 9]);
                    }           /* for */

                    *amount += 9;

                    error = CV_NO_ERR;
                }               /* if */
            }                   /* for */

            icvFree( &solutions );
            return error;
        }
        else
        {
            icvFree( &solutions );
        }                       /* if */

    }
    else
    {
        icvFree( &solutions );
    }                           /* if */

    return error;
}                               /* icvPoint7 */

CV_IMPL void
cvFindFundamentalMatrix( int *points1, int *points2, int numpoints, int /*method */ ,
                         float* matrix )
{
    CV_FUNCNAME( "cvFindFundamentalMatrix" );
    __BEGIN__;

    IPPI_CALL( icvLMedS( points1, points2, numpoints, (CvMatrix3*)matrix ));

    __CLEANUP__;
    __END__;
}
