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

/****************************************************************************************/
/****************************************************************************************/
/****************************************************************************************/
/* TYPES AND DEFINITIONS */

#include <stdio.h>

#define icvCvt_32f_64d  icvCvt_32f64f
#define icvCvt_64d_32f  icvCvt_64f32f

CvStatus icvFindHomography( int numPoints,
                            CvSize imageSize,
                            CvPoint2D64d * imagePoints,

                            CvPoint2D64d * objectPoints, CvMatr64d Homography );

CvStatus icvInitIntrinsicParams( int numImages,
                                 int *numPoints,
                                 CvSize imageSize,
                                 CvPoint2D64d * imagePoints,
                                 CvPoint2D64d * objectPoints,
                                 CvVect64d focalLength,
                                 CvPoint2D64d * principalPoint,

                                 CvVect64d distortion, CvMatr64d cameraMatrix );

CvStatus icvNormalizeImagePoints( int numPoints,
                                  CvPoint2D64d * ImagePoints,
                                  CvVect64d focalLength,
                                  CvPoint2D64d principalPoint,
                                  CvVect64d distortion, CvPoint3D64d * resImagePoints );

CvStatus icvRigidMotionTransform( int numPoints,
                                  CvPoint3D64d * objectPoints,
                                  CvVect64d rotVect, CvVect64d transVect,
                                  CvPoint3D64d * rigidMotionTrans,/* Output points */
                                  CvMatr64d derivMotionRot, CvMatr64d derivMotionTrans );

CvStatus icvSVDSym_64d( CvMatr64d LtL, CvVect64d resV, int num );

/****************************************************************************************/
/*F//////////////////////////////////////////////////////////////////////////////////////
//    Name: icvFindHomography
//    Purpose: Finds homography for image points and it's projection on image
//    Context:
//    Parameters:  numPoints    - number of points
//                 imageSize    - size of image
//                 imagePoints  - vector of image points coordinates
//                 objectPoints - vector of object points coordinates on 2D etalon
//                 Homography   - found homography matrix 3x3
//
//F*/
CvStatus
icvFindHomography( int numPoints,
                   CvSize imageSize,
                   CvPoint2D64d * imagePoints,
                   CvPoint2D64d * objectPoints, CvMatr64d Homography )
{                               /* Find homography for one image */

    CvPoint2D64d *normPoints;
    CvPoint3D64d *mPoints;
    CvPoint3D64d *mNormPoints;
    CvMatr64d L;
    double mxx, myy;
    double scxx, scyy;
    int t;
    int base1, base2;
    int iter;

    /* Optimization */
    CvMatr64d mrep;
    CvMatr64d J;
    CvMatr64d Jtran;

    CvMatr64d MMM;
    CvMatr64d M;
    CvVect64d m_err;
    double m1, m2, m3;
    double mm2_1, mm2_2;
    double mm3_1, mm3_2;
    double coef;
    double Hnorm[9];
    double Hrem[9];
    double H[9];
    double invHnorm[9];
    double LtL[9 * 9];
    double JtJ[8 * 8];
    double invJtJ[8 * 8];
    double tmp8[8];
    double hh_innov[8];
    double hhv[8];
    double hhv_up[8];

    imageSize = imageSize;

    /* Test right parameters */
    if( numPoints <= 0 ||       /* !!! Need to know minimal number of points */
        imagePoints == 0 || objectPoints == 0 )
    {
        return CV_BADPOINT_ERR;
    }

    /* Create matrix for homography */
    normPoints = (CvPoint2D64d *) icvCreateVector_64d( numPoints * 2 );

    mrep = icvCreateMatrix_64d( numPoints, 3 );
    J = icvCreateMatrix_64d( 8, 2 * numPoints );
    Jtran = icvCreateMatrix_64d( 2 * numPoints, 8 );
    MMM = icvCreateMatrix_64d( numPoints, 3 );
    M = icvCreateMatrix_64d( numPoints, 3 );
    m_err = icvCreateVector_64d( 2 * numPoints );
    mPoints = (CvPoint3D64d *) icvCreateVector_64d( numPoints * 3 );
    mNormPoints = (CvPoint3D64d *) icvCreateVector_64d( numPoints * 3 );
    L = icvCreateMatrix_64d( 9, 2 * numPoints );

    /* Prenormalixation of point coordinates */
    mxx = 0;
    myy = 0;
    for( t = 0; t < numPoints; t++ )
    {
        mxx += imagePoints[t].x;
        myy += imagePoints[t].y;
    }

    mxx /= numPoints;
    myy /= numPoints;

    for( t = 0; t < numPoints; t++ )
    {
        normPoints[t].x = imagePoints[t].x - mxx;
        normPoints[t].y = imagePoints[t].y - myy;
    }

    scxx = 0;
    scyy = 0;

    for( t = 0; t < numPoints; t++ )
    {
        scxx += fabs( normPoints[t].x );
        scyy += fabs( normPoints[t].y );
    }
    scxx /= numPoints;
    scyy /= numPoints;

    /* Create matrix M from points */

    for( t = 0; t < numPoints; t++ )
    {
        M[numPoints * 0 + t] = objectPoints[t].x;
        M[numPoints * 1 + t] = objectPoints[t].y;
        M[numPoints * 2 + t] = 1.0f;
    }

    /* Create initial homography matrix */

    Hnorm[0] = 1.0f / scxx;
    Hnorm[1] = 0.0f;
    Hnorm[2] = -mxx / scxx;

    Hnorm[3] = 0.0;
    Hnorm[4] = 1.0f / scyy;
    Hnorm[5] = -myy / scyy;

    Hnorm[6] = 0.0f;
    Hnorm[7] = 0.0f;
    Hnorm[8] = 1.0f;

    invHnorm[0] = scxx;
    invHnorm[1] = 0.0f;
    invHnorm[2] = mxx;

    invHnorm[3] = 0.0f;
    invHnorm[4] = scyy;
    invHnorm[5] = myy;

    invHnorm[6] = 0.0f;
    invHnorm[7] = 0.0f;
    invHnorm[8] = 1.0f;

    /* Create matrix with 1 */
    /* Test memory allocation */

    for( t = 0; t < numPoints; t++ )
    {
        mPoints[t].x = imagePoints[t].x;
        mPoints[t].y = imagePoints[t].y;
        mPoints[t].z = 1.0f;
    }

    /* Apply this initial homography */
    /* mn = H * m */

    for( t = 0; t < numPoints; t++ )
    {
        icvTransformVector_64d( Hnorm,
                                 (double *) &(mPoints[t]),
                                 (double *) &(mNormPoints[t]), 3, 3 );
    }

    /* Compute homography between m and mn */
    /* Build the matrix L */

    /* Fill matrix by values */

    for( t = 0; t < numPoints; t++ )
    {
        base1 = t * 18;
        base2 = t * 18 + 9;

        L[base1] = objectPoints[t].x;
        L[base1 + 1] = objectPoints[t].y;
        L[base1 + 2] = 1.0f;
        L[base1 + 3] = 0.0f;
        L[base1 + 4] = 0.0f;
        L[base1 + 5] = 0.0f;
        L[base1 + 6] = -mNormPoints[t].x * objectPoints[t].x;
        L[base1 + 7] = -mNormPoints[t].x * objectPoints[t].y;
        L[base1 + 8] = -mNormPoints[t].x;

        L[base2] = 0.0f;
        L[base2 + 1] = 0.0f;
        L[base2 + 2] = 0.0f;
        L[base2 + 3] = objectPoints[t].x;
        L[base2 + 4] = objectPoints[t].y;
        L[base2 + 5] = 1.0f;
        L[base2 + 6] = -mNormPoints[t].y * objectPoints[t].x;
        L[base2 + 7] = -mNormPoints[t].y * objectPoints[t].y;
        L[base2 + 8] = -mNormPoints[t].y;

    }

    if( numPoints > 4 )
    {
        icvMulTransMatrixR_64d( L, 9, 2 * numPoints, LtL );
    }

    /*   SVD   */
    icvSVDSym_64d( LtL, Hrem, 9 );

    icvScaleVector_64d( Hrem, Hrem, 9, 1.0f / Hrem[8] );

    icvMulMatrix_64d( invHnorm, 3, 3, Hrem, 3, 3, H );

    icvCopyVector_64d( H, 9, Homography );

    icvCopyVector_64d( Homography, 8, hhv );

    if( numPoints > 4 )
    {
        /* Optimization */
        for( iter = 0; iter < 10; iter++ )
        {
            icvMulMatrix_64d( H, 3, 3, M, numPoints, 3, mrep );

            /* Fill matrix J using temp MMM */
            for( t = 0; t < numPoints; t++ )
            {
                coef = mrep[numPoints * 2 + t];

                m1 = -M[numPoints * 0 + t] / coef;
                m2 = -M[numPoints * 1 + t] / coef;
                m3 = -M[numPoints * 2 + t] / coef;

                mm2_1 = (mrep[numPoints * 0 + t] / coef) * (-m1);
                mm2_2 = (mrep[numPoints * 0 + t] / coef) * (-m2);

                mm3_1 = (mrep[numPoints * 1 + t] / coef) * (-m1);
                mm3_2 = (mrep[numPoints * 1 + t] / coef) * (-m2);

                base1 = 16 * t;
                base2 = 16 * t + 8;

                J[base1 + 0] = m1;
                J[base1 + 1] = m2;
                J[base1 + 2] = m3;
                J[base1 + 3] = 0.0f;
                J[base1 + 4] = 0.0f;
                J[base1 + 5] = 0.0f;
                J[base1 + 6] = mm2_1;
                J[base1 + 7] = mm2_2;

                J[base2 + 0] = 0.0f;
                J[base2 + 1] = 0.0f;
                J[base2 + 2] = 0.0f;
                J[base2 + 3] = m1;
                J[base2 + 4] = m2;
                J[base2 + 5] = m3;
                J[base2 + 6] = mm3_1;
                J[base2 + 7] = mm3_2;

                /* Compute error */
                m_err[t * 2] = imagePoints[t].x - mrep[numPoints * 0 + t] / coef;
                m_err[t * 2 + 1] = imagePoints[t].y - mrep[numPoints * 1 + t] / coef;

            }

            /* Compute JtJ */
            icvMulTransMatrixR_64d( J, 8, 2 * numPoints, JtJ );
            /* Compute trans J */
            icvTransposeMatrix_64d( J, 8, 2 * numPoints, Jtran );

            /* Compute inv(Jtj) matrix */
            icvInvertMatrix_64d( JtJ, 8, invJtJ );

            /* Compute hh_innov */
            icvTransformVector_64d( Jtran, m_err, tmp8, 2 * numPoints, 8 );

            icvTransformVector_64d( invJtJ, tmp8, hh_innov, 8, 8 );

            icvSubVector_64d( hhv, hh_innov, hhv_up, 8 );

            /* Create new H matrix */
            icvCopyVector_64d( hhv_up, 8, hhv );

            icvCopyVector_64d( hhv_up, 8, H );

            H[8] = 1.0f;
        }
    }

    icvCopyVector_64d( H, 9, Homography );

    /* Free allocated memory */
    icvDeleteMatrix( mrep );
    icvDeleteMatrix( J );
    icvDeleteMatrix( Jtran );
    icvDeleteMatrix( MMM );
    icvDeleteMatrix( M );
    icvDeleteVector( normPoints );
    icvDeleteVector( m_err );
    icvDeleteVector( mPoints );
    icvDeleteVector( mNormPoints );
    icvDeleteMatrix( L );

    return CV_NO_ERR;
}

/*======================================================================================*/
/*F//////////////////////////////////////////////////////////////////////////////////////
//    Name: icvSVDSym_64d
//    Purpose: Singular Value Decomposition for symmetric matrix
//    Context:
//    Parameters:  LtL - sourece simmetric matrix
//                 reV - resulting vector of eigen values
//                 num - size of matrix
//
//F*/
CvStatus
icvSVDSym_64d( CvMatr64d LtL, CvVect64d resV, int num )
{
    CvMatr64d V;
    CvVect64d E;
    int haveFound;
    int t;

    V = icvCreateMatrix_64d( num, num );
    E = icvCreateVector_64d( num );

    icvJacobiEigens_64d( LtL, V, E, num, 0.0 );

    /* Find first minimul eigen value != 0 */
    haveFound = 0;
    for( t = num - 1; t >= 0; t++ )
    {
        if( E[t] != 0.0 )
        {
            haveFound = 1;
            /* Copy vector */
            icvCopyVector_64d( V + num * t, num, resV );
            break;
        }
    }

    if( haveFound == 0 )
    {
        icvDeleteMatrix( V );
        icvDeleteVector( E );
        return CV_NO_ERR;       /* Error have no found */
    }

    icvDeleteMatrix( V );
    icvDeleteVector( E );
    return CV_NO_ERR;

}

/*======================================================================================*/

CvStatus
icvInitIntrinsicParams( int numImages,
                        int *numPoints,
                        CvSize imageSize,
                        CvPoint2D64d * imagePoints,
                        CvPoint2D64d * objectPoints,
                        CvVect64d focalLength,
                        CvPoint2D64d * principalPoint,
                        CvVect64d distortion, CvMatr64d cameraMatrix )
{
    CvMatr64d allHomographies;
    CvMatr64d A;
    CvMatr64d Atran;
    CvVect64d b;
    int base;
    int t;
    int currImage;
    double norm;
    double a1, b1, c1;
    double a2, b2, c2;
    double a3, b3, c3;
    double a4, b4, c4;
    double tmpH[3 * 3];
    double Htran[3 * 3];
    double SubMatr[3 * 3];
    double AtA[2 * 2];
    double invAtA[2 * 2];
    double tmpb[2];
    double V_hori_pix[3];
    double V_vert_pix[3];
    double V_diag1_pix[3];
    double V_diag2_pix[3];

    /* Compute all homographies */
    allHomographies = icvCreateMatrix_64d( 9, numImages );
    A = icvCreateMatrix_64d( 2, numImages * 2 );
    Atran = icvCreateMatrix_64d( numImages * 2, 2 );
    b = icvCreateVector_64d( numImages * 2 );

    base = 0;
    for( t = 0; t < numImages; t++ )
    {
        icvFindHomography( numPoints[t],
                           imageSize,
                           imagePoints + base, objectPoints + base,
                           allHomographies + t * 9 );
        base += numPoints[t];
    }

    /* Init principal point */
    principalPoint->x = imageSize.width / 2.0f - 0.5f;
    principalPoint->y = imageSize.height / 2.0f - 0.5f;

    /* Init distortion coeficients */
    distortion[0] = 0.0f;
    distortion[1] = 0.0f;
    distortion[2] = 0.0f;
    distortion[3] = 0.0f;

    /* compute subtract matrix */
    SubMatr[0] = 1.0;
    SubMatr[1] = 0.0;
    SubMatr[2] = -principalPoint->x;

    SubMatr[3] = 0.0;
    SubMatr[4] = 1.0;
    SubMatr[5] = -principalPoint->y;

    SubMatr[6] = 0.0;
    SubMatr[7] = 0.0;
    SubMatr[8] = 1.0;

    /* Subtract principal point shift from all homographies */
    for( t = 0; t < numImages; t++ )
    {
        icvMulMatrix_64d( SubMatr, 3, 3, allHomographies + t * 9, 3, 3, tmpH );

        icvCopyMatrix_64d( tmpH, 3, 3, allHomographies + t * 9 );

    }

    /* Create  */
    for( currImage = 0; currImage < numImages; currImage++ )
    {

        icvTransposeMatrix_64d( allHomographies + currImage * 9, 3, 3, Htran );

        /* Create vanishing points */

        icvCopyVector_64d( Htran + 0, 3, V_hori_pix );
        icvCopyVector_64d( Htran + 3, 3, V_vert_pix );

        icvAddVector_64d( Htran + 0, Htran + 3, V_diag1_pix, 3 );
        icvScaleVector_64d( V_diag1_pix, V_diag1_pix, 3, 0.5f );

        icvSubVector_64d( Htran + 0, Htran + 3, V_diag2_pix, 3 );
        icvScaleVector_64d( V_diag2_pix, V_diag2_pix, 3, 0.5f );

        norm = icvNormVector_64d( V_hori_pix, 3 );
        icvScaleVector_64d( V_hori_pix, V_hori_pix, 3, 1.0f / norm );

        norm = icvNormVector_64d( V_vert_pix, 3 );
        icvScaleVector_64d( V_vert_pix, V_vert_pix, 3, 1.0f / norm );

        norm = icvNormVector_64d( V_diag1_pix, 3 );
        icvScaleVector_64d( V_diag1_pix, V_diag1_pix, 3, 1.0f / norm );

        norm = icvNormVector_64d( V_diag2_pix, 3 );
        icvScaleVector_64d( V_diag2_pix, V_diag2_pix, 3, 1.0f / norm );

        a1 = V_hori_pix[0];
        b1 = V_hori_pix[1];
        c1 = V_hori_pix[2];

        a2 = V_vert_pix[0];
        b2 = V_vert_pix[1];
        c2 = V_vert_pix[2];

        a3 = V_diag1_pix[0];
        b3 = V_diag1_pix[1];
        c3 = V_diag1_pix[2];

        a4 = V_diag2_pix[0];
        b4 = V_diag2_pix[1];
        c4 = V_diag2_pix[2];

        /* Combine matrix A */
        A[currImage * 4 + 0] = a1 * a2;
        A[currImage * 4 + 1] = b1 * b2;
        A[currImage * 4 + 2] = a3 * a4;
        A[currImage * 4 + 3] = b3 * b4;

        /* Combine vector b */
        b[currImage * 2 + 0] = -c1 * c2;
        b[currImage * 2 + 1] = -c3 * c4;

    }

    /* Compute AtA */
    icvMulTransMatrixR_64d( A, 2, 2 * numImages, AtA );

    /* Compute inverse of matr AtA */
    icvInvertMatrix_64d( AtA, 2, invAtA );

    /* Compute trans A */
    icvTransposeMatrix_64d( A, 2, 2 * numImages, Atran );

    /* multiplicate matrix At and vector b */
    icvTransformVector_64d( Atran, b, tmpb, 2 * numImages, 2 );

    icvTransformVector_64d( invAtA, tmpb, focalLength, 2, 2 );

    focalLength[0] = sqrt( fabs( 1.0 / focalLength[0] ));
    focalLength[1] = sqrt( fabs( 1.0 / focalLength[1] ));

    cameraMatrix[0] = focalLength[0];
    cameraMatrix[1] = 0.0f;
    cameraMatrix[2] = principalPoint->x;

    cameraMatrix[3] = 0.0f;
    cameraMatrix[4] = focalLength[1];
    cameraMatrix[5] = principalPoint->y;

    cameraMatrix[6] = 0.0f;
    cameraMatrix[7] = 0.0f;
    cameraMatrix[8] = 1.0f;

    icvDeleteMatrix( allHomographies );
    icvDeleteMatrix( A );
    icvDeleteMatrix( Atran );
    icvDeleteVector( b );

    return CV_NO_ERR;
}

/*======================================================================================*/

IPCVAPI_IMPL( CvStatus, icvRodrigues_64d, (CvMatr64d rotMatr,
                                           CvVect64d rotVect,
                                           CvMatr64d Jacobian, CvRodriguesType convType) )
{


    double      eps = DBL_EPSILON;
    double      bigeps = 10e+20*eps;
    int         t;
    double      dm3din[3*4];
    double      dm2dm3[4*4];

    if( convType == CV_RODRIGUES_V2M )
    {/* Convert Vector to Matrix Jac is w=3 x h=9 */
        
        double       theta;
        double       invTheta;
        double       alpha,betta,gamma;
        double       omega[3];
        double       omegav[3*3];
        double       A[3*3];
        double       tmp33[3*3];
        double       tmp49[4*9];
        double       tmp34[3*4];
        double       dRdm1[21*9];
        double       dm1dm2[4*21];
        double       w1,w2,w3;

        theta = icvNormVector_64d( rotVect, 3 );

        if( theta < eps )
        {
            /* Set Rotate matrix to ones */
            icvSetIdentity_64d(rotMatr, 3, 3);

            if( Jacobian != 0 )
            {
                //icvSetIdentity_64d(Jacobian,9,3);
                Jacobian[0]  =  0;
                Jacobian[1]  =  0;
                Jacobian[2]  =  0;
                Jacobian[3]  =  0;
                Jacobian[4]  =  0;
                Jacobian[5]  =  1;
                Jacobian[6]  =  0;
                Jacobian[7]  = -1;
                Jacobian[8]  =  0;
                Jacobian[9]  =  0;
                Jacobian[10] =  0;
                Jacobian[11] = -1;
                Jacobian[12] =  0;
                Jacobian[13] =  0;
                Jacobian[14] =  0;
                Jacobian[15] =  1;
                Jacobian[16] =  0;
                Jacobian[17] =  0;
                Jacobian[18] =  0;
                Jacobian[19] =  1;
                Jacobian[20] =  0;
                Jacobian[21] = -1;
                Jacobian[22] =  0;
                Jacobian[23] =  0;
                Jacobian[24] =  0;
                Jacobian[25] =  0;
                Jacobian[26] =  0;
            }
        }
        else
        {
            invTheta = 1.0 / theta;
            
            icvScaleVector_64d(rotVect, omega, 3, 1.0f / theta);

            icvSetIdentity_64d(dm3din,3,4);
            
            icvCopyVector_64d(omega,3,dm3din+9);
            
            icvSetIdentity_64d(dm2dm3,4,4);
            icvScaleMatrix_64d(dm2dm3,dm2dm3,4,3,invTheta);

            for( t = 0; t < 3; t++ )
            {
                dm2dm3[t * 4 + 3] = - omega[t]/theta;
            }

            alpha = (double)cos(theta);
            betta = (double)sin(theta);
            gamma = 1 - alpha;

            omegav[0] =   0;
            omegav[1] = - omega[2];
            omegav[2] =   omega[1];

            omegav[3] =   omega[2];
            omegav[4] =   0;
            omegav[5] = - omega[0];

            omegav[6] = - omega[1];
            omegav[7] =   omega[0];
            omegav[8] =   0;

            /* Compute OMEGA*OMEGAt */
            icvMulMatrix_64d(omega,1,3,omega,3,1,A);

            icvSetZero_64d(dm1dm2,4,21);
            dm1dm2[0 * 4 + 3] = - betta;
            dm1dm2[1 * 4 + 3] =   alpha;
            dm1dm2[2 * 4 + 3] =   betta;

            dm1dm2[4 * 4 + 2] =   1;
            dm1dm2[5 * 4 + 1] = - 1;
            dm1dm2[6 * 4 + 2] = - 1;
            dm1dm2[8 * 4 + 0] =   1;
            dm1dm2[9 * 4 + 1] =   1;
            dm1dm2[10* 4 + 0] = - 1;

            w1 = omega[0];
            w2 = omega[1];
            w3 = omega[2];

            dm1dm2[12 * 4 + 0] = 2 * w1;
            dm1dm2[13 * 4 + 0] = w2;
            dm1dm2[14 * 4 + 0] = w3;
            dm1dm2[15 * 4 + 0] = w2;
            dm1dm2[16 * 4 + 0] = 0;
            dm1dm2[17 * 4 + 0] = 0;
            dm1dm2[18 * 4 + 0] = w3;
            dm1dm2[19 * 4 + 0] = 0;
            dm1dm2[20 * 4 + 0] = 0;

            dm1dm2[12 * 4 + 1] = 0;
            dm1dm2[13 * 4 + 1] = w1;
            dm1dm2[14 * 4 + 1] = 0;
            dm1dm2[15 * 4 + 1] = w1;
            dm1dm2[16 * 4 + 1] = 2 * w2;
            dm1dm2[17 * 4 + 1] = w3;
            dm1dm2[18 * 4 + 1] = 0;
            dm1dm2[19 * 4 + 1] = w3;
            dm1dm2[20 * 4 + 1] = 0;

            dm1dm2[12 * 4 + 2] = 0;
            dm1dm2[13 * 4 + 2] = 0;
            dm1dm2[14 * 4 + 2] = w1;
            dm1dm2[15 * 4 + 2] = 0;
            dm1dm2[16 * 4 + 2] = 0;
            dm1dm2[17 * 4 + 2] = w2;
            dm1dm2[18 * 4 + 2] = w1;
            dm1dm2[19 * 4 + 2] = w2;
            dm1dm2[20 * 4 + 2] = 2 * w3;

            icvSetIdentity_64d(rotMatr, 3, 3);
            icvScaleMatrix_64d(rotMatr, rotMatr, 3, 3, alpha);

            icvScaleMatrix_64d(omegav, tmp33, 3, 3, betta);
            icvAddMatrix_64d(rotMatr, tmp33, rotMatr, 3, 3);

            icvScaleMatrix_64d(A, tmp33, 3, 3, gamma);
            icvAddMatrix_64d(rotMatr, tmp33, rotMatr, 3, 3);

            if( Jacobian != 0 )
            {
                icvSetZero_64d(dRdm1,21,9);
                dRdm1[0 * 21 + 0] = 1;
                dRdm1[4 * 21 + 0] = 1;
                dRdm1[8 * 21 + 0] = 1;

                for( t = 0; t < 9; t++ )
                {
                    dRdm1[t * 21 + 1]     = omegav[(t%3)*3 + t/3];
                    dRdm1[t * 21 + t + 3] = betta;
                    dRdm1[t * 21 + 2    ] = A[(t%3)*3 + t/3];
                    dRdm1[t * 21 + t +12] = gamma;
                }

                /* Calculate  Jacobian = dRdm1 * dm1dm2 * dm2dm3 * dm3din */

                icvMulMatrix_64d(dRdm1,21,9,dm1dm2,4,21,tmp49);
                icvMulMatrix_64d(dm2dm3,4,4,dm3din,3,4,tmp34);
                icvMulMatrix_64d(tmp49,4,9,tmp34,3,4,Jacobian);
            }    
        }
    }
    else
    {/* convType==CV_M2V Convert Matrix to Vector  Jac is w=9 x h=3 */
        
        double       trace;
        double       theta;
        double       dthetadtr;
        double       vth;

        double       dtrdR[9*1];
        double       dvar1dtheta[2];
        double       om1[3];
        double       dthetadR[9];
        double       dvardR[9*5];
        double       dvar2dvar[5*4];
        double       tmp53[5*3];
        double       om[3];
        double       domegadvar2[4*3];
        double       tmp3[3];

        double       invRotMatr[9];
        double       tmpEye3[9];
        double       tmpmat3[9];
        double       detRotMatr;
        double       matrS[3];
        CvMat        tmpMat, matS;

        /* Need to test corrected input data */

        icvDetMatrix3x3_64d(rotMatr, &detRotMatr);

        if( fabs(detRotMatr - 1.0) < bigeps )
        {
            /* norm(in' * in - eye(3)) < bigeps */
            icvInvertMatrix_64d(rotMatr,3,invRotMatr);
            icvMulMatrix_64d(rotMatr,3,3,invRotMatr,3,3,tmpmat3);
            icvSetIdentity_64d(tmpEye3, 3, 3);
            icvSubMatrix_64d(tmpmat3,tmpEye3,tmpmat3,3,3);

          /*  icvSVDecomposition_64d(    tmpmat3,
                                        3,3,
                                        matrS,
                                        matrV); */

            tmpMat = cvMat( 3, 3, CV_64FC1, tmpmat3 );
            matS = cvMat( 3, 1, CV_64FC1, matrS );
            cvSVD( &tmpMat, &matS );
            
            if( matrS[0] < bigeps )/* norm(matr) is max of singular value of matr  */
            {
                icvMulMatrix_64d(dvar1dtheta,1,2,dthetadR,9,1,dvardR + 9 * 3);
                        
                icvTrace_64d(rotMatr, 3, 3, &trace);

                trace       = (trace - 1.0) / 2.0;

                if( trace > 1 )
                {
                    theta = 0;
                }
                else
                {
                    if( trace < -1 )
                    {
                        theta = -CV_PI;
                    }
                    else
                    {
                        theta = acos(trace);
                    }
                }


                icvSetZero_64d(dtrdR,9,1);
                
                dtrdR[0]= 0.5f;
                dtrdR[4]= 0.5f;
                dtrdR[8]= 0.5f;

                if( sin(theta) >= 1e-5 )
                {
                    /* Test for error sqrt(x) x >= 0 */

                    vth = (double)(1/(2*sin(theta)));
                    
                    om1[0] = rotMatr[2 * 3 + 1] - rotMatr[1 * 3 + 2];
                    om1[1] = rotMatr[0 * 3 + 2] - rotMatr[2 * 3 + 0];
                    om1[2] = rotMatr[1 * 3 + 0] - rotMatr[0 * 3 + 1];

                    icvScaleVector_64d(om1,om,3,vth);

                    icvScaleVector_64d(om,rotVect,3,theta);

                    if( Jacobian != 0 )
                    {
                        dthetadtr = (double)(-1 / sqrt(1 - trace * trace));

                        icvScaleVector_64d(dtrdR, dthetadR, 9, dthetadtr);
                        

                        dvar1dtheta[0] = (double)(-vth*cos(theta)/sin(theta));
                        dvar1dtheta[1] = 1;

                        icvSetZero_64d(dvardR,9,5);
                        icvMulMatrix_64d(dvar1dtheta,1,2,dthetadR,9,1,dvardR + 9 * 3);

                        dvardR[0 * 9 + 5] =  1;
                        dvardR[0 * 9 + 7] = -1;
                        dvardR[1 * 9 + 2] = -1;
                        dvardR[1 * 9 + 6] =  1;
                        dvardR[2 * 9 + 1] =  1;
                        dvardR[2 * 9 + 3] = -1;
                        
                        icvSetZero_64d(dvar2dvar,5,4);

                        dvar2dvar[0 * 5 + 0] = vth;
                        dvar2dvar[1 * 5 + 1] = vth;
                        dvar2dvar[2 * 5 + 2] = vth;

                        dvar2dvar[0 * 5 + 3] = om1[0];
                        dvar2dvar[1 * 5 + 3] = om1[1];
                        dvar2dvar[2 * 5 + 3] = om1[2];
                        dvar2dvar[3 * 5 + 4] = 1;

                        /* Calculate dout = domegadvar2 * dvar2dvar * dvardR */

                        icvSetZero_64d(domegadvar2,4,3);

                        domegadvar2[0 * 4 + 0] = theta;
                        domegadvar2[1 * 4 + 1] = theta;
                        domegadvar2[2 * 4 + 2] = theta;

                        domegadvar2[0 * 4 + 3] = om[0];
                        domegadvar2[1 * 4 + 3] = om[1];
                        domegadvar2[2 * 4 + 3] = om[2];

                        icvMulMatrix_64d(domegadvar2,4,3,dvar2dvar,5,4,tmp53);

                        icvMulMatrix_64d(tmp53,5,3,dvardR,9,5,Jacobian);
                    }
                }
                else
                {
                    if( trace > 0 )
                    {
                        icvSetZero_64d(rotVect, 3, 1);

                        if( Jacobian != 0 )
                        {
                            icvSetZero_64d(Jacobian, 9, 3);

                            Jacobian[0 * 9 + 5] =  0.5f;
                            Jacobian[0 * 9 + 7] = -0.5f;
                            Jacobian[1 * 9 + 2] = -0.5f;
                            Jacobian[1 * 9 + 6] =  0.5f;
                            Jacobian[2 * 9 + 1] =  0.5f;
                            Jacobian[2 * 9 + 3] = -0.5f;
                        }
                    }
                    else
                    {
                        tmp3[0] = (double)(sqrt((rotMatr[0 * 3 + 0] + 1.0f) * 0.5f));
                        tmp3[1] = (double)(sqrt((rotMatr[1 * 3 + 1] + 1.0f) * 0.5f));
                        tmp3[2] = (double)(sqrt((rotMatr[2 * 3 + 2] + 1.0f) * 0.5f));

                        if( rotMatr[0 * 3 + 1] < 0 )
                        {
                            tmp3[1] = - tmp3[1];
                        }

                        if( rotMatr[0 * 3 + 2] < 0 )
                        {
                            tmp3[2] = - tmp3[2];
                        }

                        icvCopyVector_64d(tmp3,3,rotVect);


                        if( Jacobian != 0 )
                        {
                            /* !!! Jacobian is not defined in this section */
                            /* If it's need it is an error. */
                            /* We must to set Jacobian to NaN */
                            /* NaN - not a number (1.#QNAN) */
                            icvSetZero_64d(Jacobian,9,3);
                        }
                    }
                }
            }
            else
            {/* max sing > bigeps */
                return CV_BADFACTOR_ERR;
            }

        }/* test for bigeps */
        else
        {/* det > bigeps */
            return CV_BADFACTOR_ERR;
        }
        
    }/* Convertion type */

    return CV_NO_ERR;
}

/*======================================================================================*/

CvStatus
icvNormalizeImagePoints( int numPoints,
                         CvPoint2D64d * ImagePoints,
                         CvVect64d focalLength,
                         CvPoint2D64d principalPoint,
                         CvVect64d distortion, CvPoint3D64d * resImagePoints )
{
    int t;
    int iter;
    double k1, k2;
    double p1, p2;
    double tmpr;
    double k_radial;
    double x, y;
    double delta_x, delta_y;
    CvMatr64d r_2;
    CvPoint2D64d *desPnt;

    r_2 = icvCreateVector_64d( numPoints );

    desPnt = (CvPoint2D64d *) icvCreateVector_64d( numPoints * 2 );

    k1 = distortion[0];
    k2 = distortion[1];

    p1 = distortion[2];
    p2 = distortion[3];

    /* Shift points to principal point and use focal length */
    for( t = 0; t < numPoints; t++ )
    {
        resImagePoints[t].x = (ImagePoints[t].x - principalPoint.x) / focalLength[0];
        resImagePoints[t].y = (ImagePoints[t].y - principalPoint.y) / focalLength[1];
        resImagePoints[t].z = 0;
        desPnt[t].x = (ImagePoints[t].x - principalPoint.x) / focalLength[0];
        desPnt[t].y = (ImagePoints[t].y - principalPoint.y) / focalLength[1];
    }

    /* Compensate lens distortion */

    for( iter = 0; iter < 5; iter++ )
    {
        for( t = 0; t < numPoints; t++ )
        {
            x = resImagePoints[t].x;
            y = resImagePoints[t].y;

            tmpr = x * x + y * y;

            k_radial = 1 + k1 * tmpr + k2 * tmpr * tmpr;
            r_2[t] = tmpr;

            delta_x = 2 * p1 * x * y + p2 * (tmpr + 2 * x * x);
            delta_y = p1 * (tmpr + 2 * y * y) + 2 * p2 * x * y;

            x = (desPnt[t].x - delta_x) / k_radial;
            y = (desPnt[t].y - delta_y) / k_radial;

            resImagePoints[t].x = x;
            resImagePoints[t].y = y;
        }
    }

    icvDeleteVector( r_2 );
    icvDeleteVector( desPnt );

    return CV_NO_ERR;
}

/*======================================================================================*/
/* Computes the rigid motion transformation Y = R*X+T, where R = rodrigues(om) */
CvStatus
icvRigidMotionTransform( int numPoints,
                         CvPoint3D64d * objectPoints,
                         CvVect64d rotVect, CvVect64d transVect,
                         CvPoint3D64d * rigidMotionTrans, /* Output points */
                         CvMatr64d derivMotionRot, CvMatr64d derivMotionTrans )
{
    CvMatr64d tmpMatr;
    CvMatr64d tmpMatr2;
    CvMatr64d dYdR;
    int t;
    double x, y, z;

    double rotMatr[3 * 3];
    double dRdom[3 * 9];

    tmpMatr = icvCreateMatrix_64d( numPoints, 3 );
    tmpMatr2 = icvCreateMatrix_64d( numPoints, 3 );
    dYdR = icvCreateMatrix_64d( 9, 3 * numPoints );

    icvRodrigues_64d( rotMatr, rotVect, dRdom, CV_RODRIGUES_V2M );

    icvTransposeMatrix_64d( (double *) objectPoints, 3, numPoints, tmpMatr );

    icvMulMatrix_64d( rotMatr, 3, 3, tmpMatr, numPoints, 3, tmpMatr2 );

    icvTransposeMatrix_64d( tmpMatr2, numPoints, 3, (double *) rigidMotionTrans );

    for( t = 0; t < numPoints; t++ )
    {
        icvAddVector_64d( (double *) (rigidMotionTrans + t),
                           transVect, (double *) (rigidMotionTrans + t), 3 );
    }

    icvSetZero_64d( dYdR, 9, 3 * numPoints );

    for( t = 0; t < numPoints; t++ )
    {
        icvSetIdentity_64d( derivMotionTrans + t * 9, 3, 3 );
    }

    for( t = 0; t < numPoints; t++ )
    {
        x = objectPoints[t].x;
        y = objectPoints[t].y;
        z = objectPoints[t].z;

        dYdR[t * 27 + 0] = x;
        dYdR[t * 27 + 3] = y;
        dYdR[t * 27 + 6] = z;

        dYdR[t * 27 + 10] = x;
        dYdR[t * 27 + 13] = y;
        dYdR[t * 27 + 16] = z;

        dYdR[t * 27 + 20] = x;
        dYdR[t * 27 + 23] = y;
        dYdR[t * 27 + 26] = z;
    }

    icvMulMatrix_64d( dYdR, 9, numPoints * 3, dRdom, 3, 9, derivMotionRot );

    icvDeleteMatrix( dYdR );
    icvDeleteMatrix( tmpMatr );
    icvDeleteMatrix( tmpMatr2 );

    return CV_NO_ERR;
}

/*======================================================================================*/

CvStatus
icvProjectPoints( int numPoints,
                  CvPoint3D64d * objectPoints,
                  CvVect64d rotVect,
                  CvVect64d transVect,
                  CvVect64d focalLength,
                  CvPoint2D64d principalPoint,
                  CvVect64d distortion,
                  CvPoint2D64d * imagePoints,
                  CvVect64d derivPointsRot,
                  CvVect64d derivPointsTrans,
                  CvVect64d derivPointsFocal,
                  CvVect64d derivPointsPrincipal, CvVect64d derivPointsDistort )
{
    CvPoint3D64d *Y;
    CvPoint2D64d *x;

    CvMatr64d trans_x;
    CvMatr64d dYdom;
    CvMatr64d dYdT;

    CvVect64d inv_Z;
    CvVect64d bb;
    CvVect64d cc;

    CvMatr64d dxdom;
    CvMatr64d dxdT;

    CvVect64d r2;
    CvVect64d r4;
    CvVect64d cdist;
    double tmp;
    double tmpx, tmpy;

    CvMatr64d tmpn3;
    CvMatr64d dr2dom;
    CvMatr64d dr2dT;
    CvMatr64d dr4dom;
    CvMatr64d dr4dT;
    CvMatr64d dcdistdom;
    CvMatr64d dcdistdT;
    CvMatr64d dcdistdk;
    CvMatr64d xd1;
    CvMatr64d xd2;
    CvMatr64d dxd1dom;
    CvVect64d tmpn;
    CvMatr64d dxd1dT;
    CvMatr64d dxd1dk;
    CvVect64d a1;
    CvVect64d a2;
    CvVect64d a3;
    CvMatr64d delta_x;
    CvVect64d aa;
    CvMatr64d ddelta_xdom;
    CvMatr64d ddelta_xdT;
    CvMatr64d ddelta_xdk;
    CvMatr64d dxd2dom;
    CvMatr64d dxd2dT;
    CvMatr64d dxd2dk;

    double tmp3[3];
    double tmp6[6];
    int t;

    Y = (CvPoint3D64d *) icvCreateVector_64d( numPoints * 3 );
    x = (CvPoint2D64d *) icvCreateVector_64d( numPoints * 2 );
    trans_x = icvCreateMatrix_64d( numPoints, 2 );
    dYdom = icvCreateMatrix_64d( 3, 3 * numPoints );
    dYdT = icvCreateMatrix_64d( 3, 3 * numPoints );
    inv_Z = icvCreateVector_64d( numPoints );
    dxdom = icvCreateMatrix_64d( 3, 2 * numPoints );
    dxdT = icvCreateMatrix_64d( 3, 2 * numPoints );
    tmpn3 = icvCreateMatrix_64d( 3, numPoints );
    dr2dom = icvCreateMatrix_64d( 3, numPoints );
    dr2dT = icvCreateMatrix_64d( 3, numPoints );
    r2 = icvCreateVector_64d( numPoints );
    r4 = icvCreateVector_64d( numPoints );
    cdist = icvCreateVector_64d( numPoints );
    dr4dom = icvCreateMatrix_64d( 3, numPoints );
    dr4dT = icvCreateMatrix_64d( 3, numPoints );
    dcdistdom = icvCreateMatrix_64d( 3, numPoints );
    dcdistdT = icvCreateMatrix_64d( 3, numPoints );
    dcdistdk = icvCreateMatrix_64d( 4, numPoints );
    xd1 = icvCreateMatrix_64d( numPoints, 2 );
    xd2 = icvCreateMatrix_64d( numPoints, 2 );
    dxd1dom = icvCreateMatrix_64d( 3, 2 * numPoints );
    dxd1dT = icvCreateMatrix_64d( 3, 2 * numPoints );
    dxd1dk = icvCreateMatrix_64d( 4, 2 * numPoints );
    a1 = icvCreateVector_64d( numPoints );
    a2 = icvCreateVector_64d( numPoints );
    a3 = icvCreateVector_64d( numPoints );
    delta_x = icvCreateMatrix_64d( numPoints, 2 );
    tmpn = icvCreateVector_64d( numPoints );
    aa = icvCreateVector_64d( numPoints );
    bb = icvCreateVector_64d( numPoints );
    cc = icvCreateVector_64d( numPoints );
    ddelta_xdom = icvCreateMatrix_64d( 3, 2 * numPoints );
    ddelta_xdT = icvCreateMatrix_64d( 3, 2 * numPoints );
    ddelta_xdk = icvCreateMatrix_64d( 4, 2 * numPoints );
    dxd2dom = icvCreateMatrix_64d( 3, 2 * numPoints );
    dxd2dT = icvCreateMatrix_64d( 3, 2 * numPoints );
    dxd2dk = icvCreateMatrix_64d( 4, 2 * numPoints );

    icvRigidMotionTransform( numPoints, objectPoints, rotVect, transVect,
                             Y, /* Output points */
                             dYdom, dYdT );

    for( t = 0; t < numPoints; t++ )
    {
        inv_Z[t] = 1.0f / Y[t].z;
    }

    for( t = 0; t < numPoints; t++ )
    {
        x[t].x = Y[t].x * inv_Z[t];
        x[t].y = Y[t].y * inv_Z[t];
    }

    icvTransposeMatrix_64d( (double *) x, 2, numPoints, trans_x );

    for( t = 0; t < numPoints; t++ )
    {
        bb[t] = -x[t].x * inv_Z[t];
        cc[t] = -x[t].y * inv_Z[t];
    }

    for( t = 0; t < numPoints; t++ )
    {
        icvScaleVector_64d( dYdom + t * 9, dxdom + t * 6, 3, inv_Z[t] );
        icvScaleVector_64d( dYdom + t * 9 + 6, tmp3, 3, bb[t] );
        icvAddVector_64d( dxdom + t * 6, tmp3, dxdom + t * 6, 3 );
        icvScaleVector_64d( dYdom + t * 9 + 3, dxdom + t * 6 + 3, 3, inv_Z[t] );
        icvScaleVector_64d( dYdom + t * 9 + 6, tmp3, 3, cc[t] );
        icvAddVector_64d( dxdom + t * 6 + 3, tmp3, dxdom + t * 6 + 3, 3 );
    }

    for( t = 0; t < numPoints; t++ )
    {
        icvScaleVector_64d( dYdT + t * 9, dxdT + t * 6, 3, inv_Z[t] );
        icvScaleVector_64d( dYdT + t * 9 + 6, tmp3, 3, bb[t] );
        icvAddVector_64d( dxdT + t * 6, tmp3, dxdT + t * 6, 3 );
        icvScaleVector_64d( dYdT + t * 9 + 3, dxdT + t * 6 + 3, 3, inv_Z[t] );
        icvScaleVector_64d( dYdT + t * 9 + 6, tmp3, 3, cc[t] );
        icvAddVector_64d( dxdT + t * 6 + 3, tmp3, dxdT + t * 6 + 3, 3 );
    }

    /* Add Distortion */

    for( t = 0; t < numPoints; t++ )
    {
        tmp = x[t].x * x[t].x + x[t].y * x[t].y;
        r2[t] = tmp;
        r4[t] = tmp * tmp;
    }

    for( t = 0; t < numPoints; t++ )
    {
        icvScaleVector_64d( dxdom + t * 6, dr2dom + t * 3, 3, 2 * x[t].x );
        icvScaleVector_64d( dxdom + t * 6 + 3, tmp3, 3, 2 * x[t].y );
        icvAddVector_64d( dr2dom + t * 3, tmp3, dr2dom + t * 3, 3 );
    }


    for( t = 0; t < numPoints; t++ )
    {
        icvScaleVector_64d( dxdT + t * 6, dr2dT + t * 3, 3, 2 * x[t].x );
        icvScaleVector_64d( dxdT + t * 6 + 3, tmp3, 3, 2 * x[t].y );
        icvAddVector_64d( dr2dT + t * 3, tmp3, dr2dT + t * 3, 3 );
    }

    for( t = 0; t < numPoints; t++ )
    {
        icvScaleVector_64d( dr2dom + t * 3, dr4dom + t * 3, 3, r2[t] * 2 );
    }

    for( t = 0; t < numPoints; t++ )
    {
        icvScaleVector_64d( dr2dT + t * 3, dr4dT + t * 3, 3, r2[t] * 2 );
    }

    /* Radial distortion  */

    for( t = 0; t < numPoints; t++ )
    {
        cdist[t] = 1.0f + distortion[0] * r2[t] + distortion[1] * r4[t];
    }

    icvScaleVector_64d( dr2dom, dcdistdom, 3 * numPoints, distortion[0] );
    icvScaleVector_64d( dr4dom, tmpn3, 3 * numPoints, distortion[1] );
    icvAddVector_64d( dcdistdom, tmpn3, dcdistdom, 3 * numPoints );
    icvScaleVector_64d( dr2dT, dcdistdT, 3 * numPoints, distortion[0] );
    icvScaleVector_64d( dr4dT, tmpn3, 3 * numPoints, distortion[1] );
    icvAddVector_64d( dcdistdT, tmpn3, dcdistdT, 3 * numPoints );

    icvSetZero_64d( dcdistdk, 4, numPoints );
    for( t = 0; t < numPoints; t++ )
    {
        dcdistdk[t * 4 + 0] = r2[t];
        dcdistdk[t * 4 + 1] = r4[t];
    }

    icvMulVectors_64d( trans_x, cdist, xd1, numPoints );
    icvMulVectors_64d( trans_x + numPoints, cdist, xd1 + numPoints, numPoints );

    for( t = 0; t < numPoints; t++ )
    {
        icvScaleVector_64d( dcdistdom + t * 3, dxd1dom + t * 6 + 0, 3, x[t].x );
        icvScaleVector_64d( dcdistdom + t * 3, dxd1dom + t * 6 + 3, 3, x[t].y );
    }

    for( t = 0; t < numPoints; t++ )
    {
        icvScaleVector_64d( dxdom + t * 6, tmp6, 6, cdist[t] );
        icvAddVector_64d( dxd1dom + t * 6, tmp6, dxd1dom + t * 6, 6 );
    }

    for( t = 0; t < numPoints; t++ )
    {
        icvScaleVector_64d( dcdistdT + t * 3, dxd1dT + t * 6 + 0, 3, x[t].x );
        icvScaleVector_64d( dcdistdT + t * 3, dxd1dT + t * 6 + 3, 3, x[t].y );
    }

    for( t = 0; t < numPoints; t++ )
    {
        icvScaleVector_64d( dxdT + t * 6, tmp6, 6, cdist[t] );
        icvAddVector_64d( dxd1dT + t * 6, tmp6, dxd1dT + t * 6, 6 );
    }

    for( t = 0; t < numPoints; t++ )
    {
        icvScaleVector_64d( dcdistdk + t * 4, dxd1dk + t * 8 + 0, 4, x[t].x );
        icvScaleVector_64d( dcdistdk + t * 4, dxd1dk + t * 8 + 4, 4, x[t].y );
    }

    /* Tangential distortion */
    for( t = 0; t < numPoints; t++ )
    {
        tmpx = x[t].x;
        tmpy = x[t].y;
        a1[t] = 2 * tmpx * tmpy;
        a2[t] = r2[t] + 2 * tmpx * tmpx;
        a3[t] = r2[t] + 2 * tmpy * tmpy;
    }

    icvScaleVector_64d( a1, delta_x, numPoints, distortion[2] );
    icvScaleVector_64d( a2, tmpn, numPoints, distortion[3] );
    icvAddVector_64d( delta_x, tmpn, delta_x, numPoints );

    icvScaleVector_64d( a3, delta_x + numPoints, numPoints, distortion[2] );
    icvScaleVector_64d( a1, tmpn, numPoints, distortion[3] );
    icvAddVector_64d( delta_x + numPoints, tmpn, delta_x + numPoints, numPoints );

    /* aa,bb,cc */

    icvScaleVector_64d( trans_x + numPoints, tmpn, numPoints, 2 * distortion[2] );
    icvScaleVector_64d( trans_x, aa, numPoints, 6 * distortion[3] );
    icvAddVector_64d( aa, tmpn, aa, numPoints );

    icvScaleVector_64d( trans_x, tmpn, numPoints, 2 * distortion[2] );
    icvScaleVector_64d( trans_x + numPoints, bb, numPoints, 2 * distortion[3] );
    icvAddVector_64d( bb, tmpn, bb, numPoints );

    icvScaleVector_64d( trans_x + numPoints, tmpn, numPoints, 6 * distortion[2] );
    icvScaleVector_64d( trans_x, cc, numPoints, 2 * distortion[3] );
    icvAddVector_64d( cc, tmpn, cc, numPoints );


    for( t = 0; t < numPoints; t++ )
    {
        icvScaleVector_64d( dxdom + t * 6, ddelta_xdom + t * 6, 3, aa[t] );
        icvScaleVector_64d( dxdom + t * 6 + 3, tmp3, 3, bb[t] );
        icvAddVector_64d( ddelta_xdom + t * 6, tmp3, ddelta_xdom + t * 6, 3 );

        icvScaleVector_64d( dxdom + t * 6, ddelta_xdom + t * 6 + 3, 3, bb[t] );
        icvScaleVector_64d( dxdom + t * 6 + 3, tmp3, 3, cc[t] );
        icvAddVector_64d( ddelta_xdom + t * 6 + 3, tmp3, ddelta_xdom + t * 6 + 3, 3 );
    }

    for( t = 0; t < numPoints; t++ )
    {
        icvScaleVector_64d( dxdT + t * 6, ddelta_xdT + t * 6, 3, aa[t] );
        icvScaleVector_64d( dxdT + t * 6 + 3, tmp3, 3, bb[t] );
        icvAddVector_64d( ddelta_xdT + t * 6, tmp3, ddelta_xdT + t * 6, 3 );

        icvScaleVector_64d( dxdT + t * 6, ddelta_xdT + t * 6 + 3, 3, bb[t] );
        icvScaleVector_64d( dxdT + t * 6 + 3, tmp3, 3, cc[t] );
        icvAddVector_64d( ddelta_xdT + t * 6 + 3, tmp3, ddelta_xdT + t * 6 + 3, 3 );
    }

    icvSetZero_64d( ddelta_xdk, 4, 2 * numPoints );

    for( t = 0; t < numPoints; t++ )
    {
        ddelta_xdk[t * 8 + 2] = a1[t];
        ddelta_xdk[t * 8 + 3] = a2[t];
        ddelta_xdk[t * 8 + 6] = a3[t];
        ddelta_xdk[t * 8 + 7] = a1[t];
    }

    icvAddVector_64d( xd1, delta_x, xd2, 2 * numPoints );
    icvAddVector_64d( dxd1dom, ddelta_xdom, dxd2dom, 2 * numPoints * 3 );
    icvAddVector_64d( dxd1dT, ddelta_xdT, dxd2dT, 2 * numPoints * 3 );
    icvAddVector_64d( dxd1dk, ddelta_xdk, dxd2dk, 2 * numPoints * 4 );

    /* Pixel coordinates */

    for( t = 0; t < numPoints; t++ )
    {
        imagePoints[t].x = xd2[0 * numPoints + t] * focalLength[0] + principalPoint.x;
        imagePoints[t].y = xd2[1 * numPoints + t] * focalLength[1] + principalPoint.y;
    }

    for( t = 0; t < numPoints; t++ )
    {
        icvScaleVector_64d( dxd2dom + t * 6, derivPointsRot + t * 6, 3,
                             focalLength[0] );
        icvScaleVector_64d( dxd2dom + t * 6 + 3, derivPointsRot + t * 6 + 3, 3,
                             focalLength[1] );
    }

    for( t = 0; t < numPoints; t++ )
    {
        icvScaleVector_64d( dxd2dT + t * 6, derivPointsTrans + t * 6, 3,
                             focalLength[0] );
        icvScaleVector_64d( dxd2dT + t * 6 + 3, derivPointsTrans + t * 6 + 3, 3,
                             focalLength[1] );
    }

    for( t = 0; t < numPoints; t++ )
    {
        icvScaleVector_64d( dxd2dk + t * 8, derivPointsDistort + t * 8, 4,
                             focalLength[0] );
        icvScaleVector_64d( dxd2dk + t * 8 + 4, derivPointsDistort + t * 8 + 4, 4,
                             focalLength[1] );
    }

    icvSetZero_64d( derivPointsFocal, 2, 2 * numPoints );

    for( t = 0; t < numPoints; t++ )
    {
        derivPointsFocal[t * 4 + 0] = xd2[numPoints * 0 + t];
        derivPointsFocal[t * 4 + 3] = xd2[numPoints * 1 + t];
    }

    for( t = 0; t < numPoints; t++ )
    {
        icvSetIdentity_64d( derivPointsPrincipal + t * 4, 2, 2 );
    }

    icvDeleteVector( Y );
    icvDeleteVector( x );
    icvDeleteMatrix( trans_x );
    icvDeleteMatrix( dYdom );
    icvDeleteMatrix( dYdT );
    icvDeleteVector( inv_Z );
    icvDeleteMatrix( dxdom );
    icvDeleteMatrix( dxdT );
    icvDeleteMatrix( tmpn3 );
    icvDeleteMatrix( dr2dom );
    icvDeleteMatrix( dr2dT );
    icvDeleteVector( r2 );
    icvDeleteVector( r4 );
    icvDeleteVector( cdist );
    icvDeleteMatrix( dr4dom );
    icvDeleteMatrix( dr4dT );
    icvDeleteMatrix( dcdistdom );
    icvDeleteMatrix( dcdistdT );
    icvDeleteMatrix( dcdistdk );
    icvDeleteMatrix( xd1 );
    icvDeleteMatrix( xd2 );
    icvDeleteMatrix( dxd1dom );
    icvDeleteMatrix( dxd1dT );
    icvDeleteMatrix( dxd1dk );
    icvDeleteVector( a1 );
    icvDeleteVector( a2 );
    icvDeleteVector( a3 );
    icvDeleteMatrix( delta_x );
    icvDeleteVector( tmpn );
    icvDeleteVector( aa );
    icvDeleteVector( bb );
    icvDeleteVector( cc );
    icvDeleteMatrix( ddelta_xdom );
    icvDeleteMatrix( ddelta_xdT );
    icvDeleteMatrix( ddelta_xdk );
    icvDeleteMatrix( dxd2dom );
    icvDeleteMatrix( dxd2dT );
    icvDeleteMatrix( dxd2dk );

    return CV_NO_ERR;
}

#define REAL_ZERO(x) (x<0.00001 && x> -0.00001)
#define Sgn(x)  (x<0?-1:(x>0?1:0))

/*======================================================================================*/
CvStatus
icvFindExtrinsicCameraParams_64d( int numPoints,
                                  CvSize imageSize,
                                  CvPoint2D64d * imagePoints,
                                  CvPoint3D64d * objectPoints,
                                  CvVect64d focalLength,
                                  CvPoint2D64d principalPoint,
                                  CvVect64d distortion,
                                  CvVect64d rotVect, CvVect64d transVect )
{
    /* Normalizate points  */

    CvPoint3D64d *resImagePoints;
    CvMatr64d matrY;
    CvMatr64d X_kk_trans;
    CvPoint2D64d *im2Pnt;
    CvPoint2D64d *ob2Pnt;
    CvMatr64d X_new;
    int t;
    double det;
    double r;
    double tmpf1, tmpf2;
    CvPoint2D64d *projectImagePoints;
    CvVect64d derivPointsRot;
    CvVect64d derivPointsTrans;
    CvVect64d derivPointsFocal;
    CvVect64d derivPointsPrincipal;
    CvVect64d derivPointsDistort;
    CvMatr64d matrJJ;
    CvMatr64d matrJJt;

    CvPoint2D64d *ex_points;
    int iter;
    double change;
    double norm1;
    double norm2;
    double sc;

    double matrYY[3 * 3];
    double matrU[3 * 3];
    double matrV[3 * 3];
    double matrS[3];
    double X_mean[3];
    double R_trans[3 * 3];
    double Homography[3 * 3];
    double rotMatr[3 * 3];
    double T_trans[3];
    double tmp3[3];
    double tmp3f1[3];
    double tmp3f2[3];
    double Jacobian[3 * 9];
    double omckk[3];
    double tmpRotMatr[3 * 3];
    double matrJJtJJ[6 * 6];
    double invmatrJJtJJ[6 * 6];
    double tmp6[6];
    double vectParam_innov[6];
    double vect_Param_up[6];
    double vect_Param[6];

    CvMat  matU, matS, matV;

    matrY = icvCreateMatrix_64d( numPoints, 3 );
    X_new = icvCreateMatrix_64d( numPoints, 3 );
    X_kk_trans = icvCreateMatrix_64d( numPoints, 3 );
    matrJJ = icvCreateMatrix_64d( 6, 2 * numPoints );
    matrJJt = icvCreateMatrix_64d( 2 * numPoints, 6 );
    derivPointsRot = icvCreateVector_64d( 3 * 2 * numPoints );
    derivPointsTrans = icvCreateVector_64d( 3 * 2 * numPoints );
    derivPointsFocal = icvCreateVector_64d( 2 * 2 * numPoints );
    derivPointsPrincipal = icvCreateVector_64d( 2 * 2 * numPoints );
    derivPointsDistort = icvCreateVector_64d( 4 * 2 * numPoints );
    im2Pnt = (CvPoint2D64d *) icvCreateVector_64d( numPoints * 2 );
    ob2Pnt = (CvPoint2D64d *) icvCreateVector_64d( numPoints * 2 );
    projectImagePoints = (CvPoint2D64d *) icvCreateVector_64d( numPoints * 2 );
    ex_points = (CvPoint2D64d *) icvCreateVector_64d( numPoints * 2 );
    resImagePoints = (CvPoint3D64d *) icvCreateVector_64d( numPoints * 3 );

    /*  compute trans matrix of points */
    icvTransposeMatrix_64d( (double *) objectPoints, 3, numPoints, X_kk_trans );

    icvNormalizeImagePoints( numPoints,
                             imagePoints,
                             focalLength, principalPoint, distortion, resImagePoints );


    /* check for planarity of the structure */
    /*  compute mean value */

    X_mean[0] = 0;
    X_mean[1] = 0;
    X_mean[2] = 0;

    for( t = 0; t < numPoints; t++ )
    {
        X_mean[0] += objectPoints[t].x;
        X_mean[1] += objectPoints[t].y;
        X_mean[2] += objectPoints[t].z;
    }

    X_mean[0] = X_mean[0] / numPoints;
    X_mean[1] = X_mean[1] / numPoints;
    X_mean[2] = X_mean[2] / numPoints;


    /*  fill matrix Y */
    for( t = 0; t < numPoints; t++ )
    {
        matrY[numPoints * 0 + t] = X_kk_trans[numPoints * 0 + t] - X_mean[0];
        matrY[numPoints * 1 + t] = X_kk_trans[numPoints * 1 + t] - X_mean[1];
        matrY[numPoints * 2 + t] = X_kk_trans[numPoints * 2 + t] - X_mean[2];
    }

    /* Compute matrix YY */

    icvMulTransMatrixL_64d( matrY, numPoints, 3, matrYY );

    /*  Compute SVD of YY only */

    icvCopyMatrix_64d( matrYY, 3, 3, matrU );

    /* icvSVDecomposition_64d( matrU, 3,
                             3,
                             matrS, 
                             matrV ); */

    matU = cvMat( 3, 3, CV_MAT64D, matrU );
    matV = cvMat( 3, 3, CV_MAT64D, matrV );
    matS = cvMat( 3, 1, CV_MAT64D, matrS );
    cvSVD( &matU, &matS, 0, &matV );

    r = matrS[2] / matrS[1];

    if( r < 0.0001 )
    {                           /* Test for planarity */
        /* Transorm the plane to bring it in the Z=0 plane */

        icvTransposeMatrix_64d( matrV, 3, 3, R_trans );

       double r_norm = sqrt(R_trans[2]*R_trans[2] + R_trans[5]*R_trans[5]);

        if( r_norm < 1e-6 )
        {
            R_trans[0] = 1; R_trans[1] = 0; R_trans[2] = 0;
            R_trans[3] = 0; R_trans[4] = 1; R_trans[5] = 0;
            R_trans[6] = 0; R_trans[7] = 0; R_trans[8] = 1;
        }

        icvDetMatrix3x3_64d( R_trans, &det );

        if( det < 0 )
        {
            icvScaleMatrix_64d( R_trans, R_trans, 3, 3, -1 );

        }

        icvMulMatrix_64d( R_trans, 3, 3, X_mean, 1, 3, T_trans );
        icvScaleMatrix_64d( T_trans, T_trans, 1, 3, -1 );

        icvMulMatrix_64d( R_trans, 3, 3, X_kk_trans, numPoints, 3, X_new );

        for( t = 0; t < numPoints; t++ )
        {
            X_new[numPoints * 0 + t] += T_trans[0];
        }

        for( t = 0; t < numPoints; t++ )
        {
            X_new[numPoints * 1 + t] += T_trans[1];
        }

        for( t = 0; t < numPoints; t++ )
        {
            X_new[numPoints * 2 + t] += T_trans[2];
        }

        /* Compute the planar homography */
        for( t = 0; t < numPoints; t++ )
        {
            im2Pnt[t].x = resImagePoints[t].x;
            im2Pnt[t].y = resImagePoints[t].y;
            ob2Pnt[t].x = X_new[0 * numPoints + t];
            ob2Pnt[t].y = X_new[1 * numPoints + t];
        }

        icvFindHomography( numPoints, imageSize, im2Pnt, ob2Pnt, Homography );


        /* De-embed the motion parameters from the homography */
        for( t = 0; t < 3; t++ )
        {
            tmp3[t] = Homography[t * 3 + 0];
        }
        tmpf1 = icvNormVector_64d( tmp3, 3 );

        for( t = 0; t < 3; t++ )
        {
            tmp3[t] = Homography[t * 3 + 1];
        }
        tmpf2 = icvNormVector_64d( tmp3, 3 );
        
        sc = (tmpf1 + tmpf2) * 0.5f;

        icvScaleVector_64d( Homography, Homography, 9, 1.0f / sc );

        
        /* Scale part of homography */
        for( t = 0; t < 3; t++ )
        {
            tmp3[t] = Homography[t * 3 + 0];
        }
        tmpf1 = icvNormVector_64d( tmp3, 3 );

        for( t = 0; t < 3; t++ )
        {
            tmp3[t] = Homography[t * 3 + 1];
        }
        tmpf2 = icvNormVector_64d( tmp3, 3 );

        for( t = 0; t < 3; t++ )
        {
            Homography[t*3+0] = Homography[t*3+0]/tmpf1;
        }

        for( t = 0; t < 3; t++ )
        {
            Homography[t*3+1] = Homography[t*3+1]/tmpf2;
        }

        /* fill rotate matrix */
        icvCopyVector_64d( Homography, 9, rotMatr );

        tmp3f1[0] = Homography[0];
        tmp3f1[1] = Homography[3]; 
        tmp3f1[2] = Homography[6];

        tmp3f2[0] = Homography[1];
        tmp3f2[1] = Homography[4];
        tmp3f2[2] = Homography[7];

        icvCrossProduct2L_64d( tmp3f1, tmp3f2, tmp3 );

        rotMatr[0] = Homography[0];
        rotMatr[3] = Homography[3];
        rotMatr[6] = Homography[6];

        rotMatr[1] = Homography[1];
        rotMatr[4] = Homography[4];
        rotMatr[7] = Homography[7];

        rotMatr[2] = tmp3[0];
        rotMatr[5] = tmp3[1];
        rotMatr[8] = tmp3[2];

        icvRodrigues_64d( rotMatr, omckk, Jacobian, CV_RODRIGUES_M2V );

        icvRodrigues_64d( rotMatr, omckk, Jacobian, CV_RODRIGUES_V2M );

        transVect[0] = Homography[2];
        transVect[1] = Homography[5];
        transVect[2] = Homography[8];

        icvMulMatrix_64d( rotMatr, 3, 3, T_trans, 1, 3, tmp3 );

        icvAddVector_64d( transVect, tmp3, transVect, 3 );

        /* Compute rotation matrix */
        icvMulMatrix_64d( rotMatr, 3, 3, R_trans, 3, 3, tmpRotMatr );

        icvRodrigues_64d( tmpRotMatr, rotVect, Jacobian, CV_RODRIGUES_M2V );

    }
    else
    {
        /* Non planar structure detected */
        
        int i;
//        J = zeros(2*Np,12);
        CvMatr64d matrJ;
        CvMatr64d matrJtrans;
        CvMatr64d matrJJ;
        matrJ = icvCreateMatrix_64d( 12, 2 * numPoints );
        matrJtrans = icvCreateMatrix_64d( 2 * numPoints,12 );
        matrJJ = icvCreateMatrix_64d(12,12);
        icvSetZero_64d( matrJ,      12, 2 * numPoints );
        icvSetZero_64d( matrJtrans, 2 * numPoints, 12 );

//        xX = (ones(3,1)*xn(1,:)).*X_kk;
        CvMatr64d matrxX;
        matrxX = icvCreateMatrix_64d(numPoints,3);
        /* Fill matrix */
        for( i = 0; i < numPoints; i++ )
        {
            matrxX[i]             = resImagePoints[i].x * X_kk_trans[i];
            matrxX[numPoints+i]   = resImagePoints[i].x * X_kk_trans[numPoints+i];
            matrxX[numPoints*2+i] = resImagePoints[i].x * X_kk_trans[numPoints*2+i];
        }
        
//        yX = (ones(3,1)*xn(2,:)).*X_kk;
        CvMatr64d matryX;
        matryX = icvCreateMatrix_64d(numPoints,3);
        /* Fill matrix */
        for( i = 0; i < numPoints; i++ )
        {
            matryX[i]             = X_kk_trans[i]             * resImagePoints[i].y;
            matryX[numPoints+i]   = X_kk_trans[numPoints+i]   * resImagePoints[i].y;
            matryX[numPoints*2+i] = X_kk_trans[numPoints*2+i] * resImagePoints[i].y;
        }

        for( i = 0; i < numPoints; i++ )
        {
//        J(1:2:end,[1 4 7]) = -X_kk';
            matrJ[i*12*2+0] = -objectPoints[i].x;
            matrJ[i*12*2+3] = -objectPoints[i].y;
            matrJ[i*12*2+6] = -objectPoints[i].z;

//        J(2:2:end,[2 5 8]) = X_kk';
            matrJ[i*12*2+12+1] = objectPoints[i].x;
            matrJ[i*12*2+12+4] = objectPoints[i].y;
            matrJ[i*12*2+12+7] = objectPoints[i].z;

//        J(1:2:end,[3 6 9]) = xX';
            matrJ[i*12*2+2] = matrxX[i];
            matrJ[i*12*2+5] = matrxX[numPoints+i];
            matrJ[i*12*2+8] = matrxX[numPoints*2+i];

//        J(2:2:end,[3 6 9]) = -yX';
            matrJ[i*12*2+12+2] = -matryX[i];
            matrJ[i*12*2+12+5] = -matryX[numPoints+i];
            matrJ[i*12*2+12+8] = -matryX[numPoints*2+i];

//        J(1:2:end,12) = xn(1,:)';
            matrJ[i*12*2+11] = resImagePoints[i].x;

//        J(2:2:end,12) = -xn(2,:)';
            matrJ[i*12*2+12+11] = -resImagePoints[i].y;

//        J(1:2:end,10) = -ones(Np,1);
            matrJ[i*12*2+9] = -1;

//        J(2:2:end,11) = ones(Np,1);
            matrJ[i*12*2+12+10] = 1;

        }

        /* compute trans matrix J */
        icvTransposeMatrix_64d(matrJ,12,numPoints*2,matrJtrans);


//        JJ = J'*J;
        icvMulMatrix_64d(matrJtrans,numPoints*2,12,matrJ,12,numPoints*2,matrJJ);
        
//        [U,S,V] = svd(JJ);
        CvMat* matrVm;
        matrVm = cvCreateMat(12,12,CV_64FC1);

        CvMat* matrJJm;
        matrJJm = cvCreateMat(12,12,CV_64FC1);

        /* Copy matrix JJ */
        for( i = 0; i < 12*12; i++ )
        {
            CvScalar value;
            value.val[0] = matrJJ[i];
            cvSetAt(matrJJm,value,i/12,i%12);
        }

        CvMat* matrWm;
        matrWm = cvCreateMat(12,12,CV_64FC1);

        cvSVD(matrJJm,matrWm,0,matrVm);
//        cvSVD( CvArr* aarr, CvArr* warr, CvArr* uarr, CvArr* varr, int flags )


//        RR = reshape(V(1:9,12),3,3);
        CvMat* matrRR;
        matrRR = cvCreateMat(3,3,CV_64FC1);
        /* Copy matrix elements (reshape) */
        for( i = 0; i < 9; i++ )
        {
            CvScalar value;
            value = cvGetAt( matrVm, i, 11 );
            cvSetAt(matrRR,value,i%3,i/3);
        }
        double det;
        det = cvDet(matrRR);

        

//        if det(RR) < 0,
//          V(:,12) = -V(:,12);
//          RR = -RR;
//        end;
        if( det < 0 )
        {
            /* Change sign in column of matrix matrVm */
            CvScalar value;
            for( i = 0; i < 12; i++ )
            {
                value = cvGetAt(matrVm,i,11);
                value.val[0] = - value.val[0];
                cvSetAt(matrVm,value,i,11);
            }
            /* Change sign of RR */
            cvConvertScale(matrRR,matrRR,-1);
        }

//        [Ur,Sr,Vr] = svd(RR);
        /* Make SVD of RR*/  
        CvMat* matrUr;
        CvMat* matrVr;
        CvMat* matrWr;
        matrUr = cvCreateMat(3,3,CV_64FC1);
        matrVr = cvCreateMat(3,3,CV_64FC1);
        matrWr = cvCreateMat(3,3,CV_64FC1);
        cvSVD(matrRR,matrWr,matrUr,matrVr);


//        Rckk = Ur*Vr';
        /* Make matrix Rckk */
        CvMat* matrRckk;
        CvMat* matrVrt;
        matrRckk = cvCreateMat(3,3,CV_64FC1);

        matrVrt = cvCreateMat(3,3,CV_64FC1);
        cvTranspose(matrVr,matrVrt);

        cvMatMul(matrUr,matrVrt,matrRckk);



//        sc = norm(V(1:9,12)) / norm(Rckk(:));
        double sum;
        double norm1,norm2;
        
        sum = 0;
        for( i = 0; i < 9; i++ )
        {
            CvScalar value;
            value = cvGetAt(matrRckk,i/3,i%3);
            sum += (value.val[0] * value.val[0]);
        }
        norm1 = sqrt(sum);

        sum = 0;
        for( i = 0; i < 9; i++ )
        {
            CvScalar value;
            value = cvGetAt(matrVm,i,11);
            sum += (value.val[0] * value.val[0]);
        }
        norm2 = sqrt(sum);

        double sc;
        sc = norm2 / norm1;

//        Tckk = V(10:12,12)/sc;
        CvMat* matrTckk;
        matrTckk = cvCreateMat(3,1,CV_64FC1);
        for( i = 0; i < 3; i++ )
        {
            CvScalar value;
            value = cvGetAt(matrVm,i+9,11);
            cvSetAt(matrTckk,value,i,0);
        }
        cvConvertScale(matrTckk,matrTckk,1.0/sc);


//        omckk = rodrigues(Rckk);
        CvMat* vectomckk;
        vectomckk = cvCreateMat(3,1,CV_64FC1);

        CvMat* jacobian;

        jacobian = cvCreateMat(3,9,CV_64FC1);
        
        cvRodrigues( matrRckk, vectomckk,
                     jacobian, CV_RODRIGUES_M2V);
        
//        Rckk = rodrigues(omckk);
        cvRodrigues( matrRckk, vectomckk,
                     jacobian, CV_RODRIGUES_V2M);

        /* Copy rot vector */
        for( i = 0; i < 3; i++ )
        {
            CvScalar value;
            value = cvGetAt(vectomckk,i,0);
            rotVect[i] = value.val[0];
        }

        /* Copy  trans vect*/
        for( i = 0; i < 3; i++ )
        {
            CvScalar value;
            value = cvGetAt(matrTckk,i,0);
            transVect[i] = value.val[0];
        }


        /* Free matrices */
        icvDeleteMatrix(matrJ);
        icvDeleteMatrix(matrJtrans);
        icvDeleteMatrix(matrJJ);
        icvDeleteMatrix(matrxX);
        icvDeleteMatrix(matryX);
        
        cvReleaseMat(&matrVm);
        cvReleaseMat(&matrJJm);
        cvReleaseMat(&matrWm);
        cvReleaseMat(&matrRR);
        cvReleaseMat(&matrUr);
        cvReleaseMat(&matrVr);
        cvReleaseMat(&matrWr);
        cvReleaseMat(&matrRckk);
        cvReleaseMat(&matrVrt);
        cvReleaseMat(&matrTckk);
        cvReleaseMat(&vectomckk);
        cvReleaseMat(&jacobian);
            
    }

    /***** Finish INIT *****/
    /***** Begin REFINE ****/
    
    /* Final optimazation (minimize the reprojection error in pixel) */
    /* through Gradient Descent */

    iter = 0;
    change = 1.0f;

    icvCopyVector_64d( rotVect, 3, vect_Param );
    icvCopyVector_64d( transVect, 3, vect_Param + 3 );

    while( (change > 1e-10) && (iter < 20) )
    {

        
        icvProjectPoints( numPoints, objectPoints, rotVect, transVect,
                          focalLength, principalPoint, distortion,
                          projectImagePoints,
                          derivPointsRot,       /* need     */
                          derivPointsTrans,     /* need     */
                          derivPointsFocal,     /* not need */
                          derivPointsPrincipal, /* not need */
                          derivPointsDistort ); /* not need */
        /* compute ex points */


        icvSubVector_64d( (double *) imagePoints, (double *) projectImagePoints,
                           (double *) ex_points, numPoints * 2 );
        /* create vector matrJJ */


        for( t = 0; t < numPoints * 2; t++ )
        {
            icvCopyVector_64d( derivPointsRot + t * 3, 3, matrJJ + t * 6 );
            icvCopyVector_64d( derivPointsTrans + t * 3, 3, matrJJ + t * 6 + 3 );
        }

        icvMulTransMatrixR_64d( matrJJ, 6, 2 * numPoints, matrJJtJJ );
        
        icvInvertMatrix_64d( matrJJtJJ, 6, invmatrJJtJJ );

        icvTransposeMatrix_64d( matrJJ, 6, 2 * numPoints, matrJJt );

        icvMulMatrix_64d( matrJJt, 2 * numPoints, 6,
                          (double *) ex_points, 1, 2 * numPoints,
                           tmp6 );
        icvMulMatrix_64d( invmatrJJtJJ, 6, 6, tmp6, 1, 6, vectParam_innov );

        icvAddVector_64d( vect_Param, vectParam_innov, vect_Param_up, 6 );
        norm1 = icvNormVector_64d( vectParam_innov, 6 );
        norm2 = icvNormVector_64d( vect_Param_up, 6 );

        change = norm1 / norm2;

        icvCopyVector_64d( vect_Param_up, 6, vect_Param );

        icvCopyVector_64d( vect_Param, 3, rotVect );
        icvCopyVector_64d( vect_Param + 3, 3, transVect );
        
        iter++;

    }
    
    /* Free allocated memory */
    icvDeleteMatrix( matrY );
    icvDeleteMatrix( X_new );
    icvDeleteMatrix( X_kk_trans );
    icvDeleteMatrix( matrJJ );
    icvDeleteMatrix( matrJJt );
    icvDeleteVector( derivPointsRot );
    icvDeleteVector( derivPointsTrans );
    icvDeleteVector( derivPointsFocal );
    icvDeleteVector( derivPointsPrincipal );
    icvDeleteVector( derivPointsDistort );
    icvDeleteVector( im2Pnt );
    icvDeleteVector( ob2Pnt );
    icvDeleteVector( projectImagePoints );
    icvDeleteVector( ex_points );
    icvDeleteVector( resImagePoints );

    return CV_NO_ERR;
}

/*======================================================================================*/
CvStatus
icvCalibrateCamera_64d( int numImages,
                        int *numPoints,
                        CvSize imageSize,
                        CvPoint2D64d * imagePoints,
                        CvPoint3D64d * objectPoints,
                        CvVect64d distortion,
                        CvMatr64d cameraMatrix,
                        CvVect64d transVects, CvMatr64d rotMatrs, int useIntrinsicGuess )
{

    CvPoint2D64d principalPoint;
    CvPoint2D64d *objectPoints2D;
    CvVect64d vectParam;
    CvVect64d vectParam_innov_64d;
    CvVect64d vectParam_up_64d;
    CvMatr64d matrJJ;
    CvMatr64d matrJJt;
    CvMatr64d matrJJtJJ;
    CvMatr64d invmatrJJtJJ;
    CvVect64d tmp_vectwidthJJ;
    CvMatr64d matrJJt_64d;
    CvMatr64d matrJJtJJ_64d;
    CvMatr64d invmatrJJtJJ_64d;
    CvMatr64d tmp_vectwidthJJ_64d;
    CvPoint2D64d optim_PrincipalPoint;
    CvPoint2D64d *vect_ex;
    CvVect64d derivPointsRot;
    CvVect64d derivPointsTrans;
    CvVect64d derivPointsFocal;
    CvVect64d derivPointsPrincipal;
    CvVect64d derivPointsDistort;
    int base;
    int baseJJ;
    int allNumPoints;
    int t;
    double change;
    double norm1;
    double norm2;
    int iter;
    int currImage;
    int currLine;
    int maxNumber;
    int numCurrPoints;
    int widthJJ;
    int lineShift;
    double Jacobian[3 * 9];
    double focalLength[2];
    double optim_focalLength[2];
    double optim_distortion[4];
    double optim_rotVect[3];
    double optim_transVect[3];

    /* For non warnings */
    useIntrinsicGuess = useIntrinsicGuess;

    widthJJ = 8 + 6 * numImages;

    /* Create */

    vectParam = icvCreateVector_64d( widthJJ );
    vectParam_innov_64d = icvCreateVector_64d( widthJJ );
    vectParam_up_64d = icvCreateVector_64d( widthJJ );
    matrJJtJJ = icvCreateMatrix_64d( widthJJ, widthJJ );
    invmatrJJtJJ = icvCreateMatrix_64d( widthJJ, widthJJ );
    matrJJtJJ_64d = icvCreateMatrix_64d( widthJJ, widthJJ );
    invmatrJJtJJ_64d = icvCreateMatrix_64d( widthJJ, widthJJ );

    /* Count number of all points */

    allNumPoints = 0;
    maxNumber = numPoints[0];

    for( t = 0; t < numImages; t++ )
    {
        allNumPoints += numPoints[t];
        if( maxNumber < numPoints[t] )
        {
            maxNumber = numPoints[t];
        }
    }

    vect_ex = (CvPoint2D64d *) icvCreateVector_64d( allNumPoints * 2 );
    objectPoints2D = (CvPoint2D64d *) icvCreateVector_64d( allNumPoints * 2 );
    tmp_vectwidthJJ = icvCreateVector_64d( widthJJ );
    tmp_vectwidthJJ_64d = icvCreateVector_64d( widthJJ );
    matrJJ = icvCreateMatrix_64d( widthJJ, allNumPoints * 2 );
    matrJJt = icvCreateMatrix_64d( allNumPoints * 2, widthJJ );
    matrJJt_64d = icvCreateMatrix_64d( widthJJ, allNumPoints * 2 );
    derivPointsRot = icvCreateVector_64d( 3 * 2 * maxNumber );
    derivPointsTrans = icvCreateVector_64d( 3 * 2 * maxNumber );
    derivPointsFocal = icvCreateVector_64d( 2 * 2 * maxNumber );
    derivPointsPrincipal = icvCreateVector_64d( 2 * 2 * maxNumber );
    derivPointsDistort = icvCreateVector_64d( 4 * 2 * maxNumber );

    /* convert all object points from 2D to 3D */
    for( t = 0; t < allNumPoints; t++ )
    {
        objectPoints2D[t].x = objectPoints[t].x;
        objectPoints2D[t].y = objectPoints[t].y;
    }

    icvInitIntrinsicParams( numImages,
                            numPoints,
                            imageSize,
                            imagePoints,
                            objectPoints2D,
                            focalLength, &principalPoint, distortion, cameraMatrix );

    base = 0;

    for( t = 0; t < numImages; t++ )
    {                           /* for each image find extrinsic params */
        icvFindExtrinsicCameraParams_64d( numPoints[t],
                                          imageSize,
                                          imagePoints + base,
                                          objectPoints + base,
                                          focalLength,
                                          principalPoint,
                                          distortion,
                                          vectParam + 6 + t * 6, vectParam + 9 + t * 6 );
        base += numPoints[t];
    }
    /* Combine init params for optimization */

    vectParam[0] = focalLength[0];
    vectParam[1] = focalLength[1];
    icvCopyVector_64d( distortion, 4, vectParam + 2 );
    vectParam[6 + 6 * numImages + 0] = principalPoint.x;
    vectParam[6 + 6 * numImages + 1] = principalPoint.y;

    iter = 0;
    change = 1;

    /* Main optimization */

    while( (change > 0.000000001f) && (iter < 30) )
    {

        optim_PrincipalPoint.x = vectParam[6 + 6 * numImages + 0];
        optim_PrincipalPoint.y = vectParam[6 + 6 * numImages + 1];

        icvCopyVector_64d( vectParam + 0, 2, optim_focalLength );
        icvCopyVector_64d( vectParam + 2, 4, optim_distortion );

        base = 0;
        baseJJ = 0;

        icvSetZero_64d( matrJJ, numImages * 6 + 8, allNumPoints * 2 );

        for( currImage = 0; currImage < numImages; currImage++ )
        {

            /* Extract rot and tran vectors for current image */
            icvCopyVector_64d( vectParam + 6 + currImage * 6, 3, optim_rotVect );
            icvCopyVector_64d( vectParam + 6 + 3 + currImage * 6, 3, optim_transVect );

            numCurrPoints = numPoints[currImage];

            /* fill JJ matrix */
            icvProjectPoints( numCurrPoints,
                              objectPoints + base,
                              optim_rotVect,
                              optim_transVect,
                              optim_focalLength,
                              optim_PrincipalPoint,
                              optim_distortion,
                              vect_ex + base,
                              derivPointsRot,
                              derivPointsTrans,
                              derivPointsFocal,
                              derivPointsPrincipal, derivPointsDistort );

            /* compute subtruct  */

            icvSubVector_64d( (double *) (imagePoints + base),
                               (double *) (vect_ex + base),
                               (double *) (vect_ex + base), numCurrPoints * 2 );

            /* Fill part of matrix JJ (matrix JJkk) */

            for( currLine = 0; currLine < numCurrPoints * 2; currLine++ )
            {
                /* Fill matrix JJ */

                lineShift = widthJJ * currLine;

                matrJJ[baseJJ + lineShift + 0] = derivPointsFocal[currLine * 2 + 0];
                matrJJ[baseJJ + lineShift + 1] = derivPointsFocal[currLine * 2 + 1];

                icvCopyVector_64d( derivPointsDistort + currLine * 4, 4,
                                    matrJJ + baseJJ + lineShift + 2 );

                icvCopyVector_64d( derivPointsRot + currLine * 3,
                                    3, matrJJ + baseJJ + lineShift + 6 + 6 * currImage );


                icvCopyVector_64d( derivPointsTrans + currLine * 3,
                                    3,
                                    matrJJ + baseJJ + lineShift + 6 + 6 * currImage + 3 );

                matrJJ[baseJJ + lineShift + widthJJ - 2] =
                    derivPointsPrincipal[currLine * 2 + 0];
                matrJJ[baseJJ + lineShift + widthJJ - 1] =
                    derivPointsPrincipal[currLine * 2 + 1];

            }                   /* for fill matrix JJ */

            baseJJ += (numCurrPoints * 2) * (widthJJ);
            base += numCurrPoints;

        }                       /* for each image */

        icvComplexMult_64d( matrJJ, matrJJt_64d, widthJJ, allNumPoints * 2 );

        icvMulMatrix_64d( matrJJt_64d,
                           allNumPoints * 2,
                           widthJJ,
                           (double *) vect_ex, 1, allNumPoints * 2, vectParam_innov_64d );

        icvAddVector_64d( vectParam, vectParam_innov_64d, vectParam_up_64d, widthJJ );

        norm1 = icvNormVector_64d( vectParam_innov_64d, widthJJ );
        norm2 = icvNormVector_64d( vectParam_up_64d, widthJJ );
        change = norm1 / norm2;

        icvCopyVector_64d( vectParam_up_64d, widthJJ, vectParam );

        iter++;

    }


    /* Fill by found parameters */
    /*  Fill camera matrix */
    cameraMatrix[0] = vectParam[0];
    cameraMatrix[1] = 0.0f;
    cameraMatrix[2] = vectParam[6 + numImages * 6];

    cameraMatrix[3] = 0.0f;
    cameraMatrix[4] = vectParam[1];
    cameraMatrix[5] = vectParam[6 + numImages * 6 + 1];

    cameraMatrix[6] = 0.0f;
    cameraMatrix[7] = 0.0f;
    cameraMatrix[8] = 1.0f;

    /* Dill Distortion */
    distortion[0] = vectParam[2];
    distortion[1] = vectParam[3];
    distortion[2] = vectParam[4];
    distortion[3] = vectParam[5];

    /* Copy rotate matrices (before convert it from vectors) */
    for( t = 0; t < numImages; t++ )
    {

        /* Copy rotate matrices (before convert it from vectors) */
        icvRodrigues_64d( rotMatrs + t * 9,
                          vectParam + 6 + t * 6, Jacobian, CV_RODRIGUES_V2M );

        /* Copy trans vectors */
        icvCopyVector_64d( vectParam + 6 + 3 + t * 6, 3, transVects + t * 3 );
    }

    /* Free allocated memory */
    icvDeleteVector( vectParam );
    icvDeleteVector( vectParam_innov_64d );
    icvDeleteVector( vectParam_up_64d );
    icvDeleteMatrix( matrJJtJJ );
    icvDeleteMatrix( invmatrJJtJJ );
    icvDeleteMatrix( matrJJtJJ_64d );
    icvDeleteMatrix( invmatrJJtJJ_64d );
    icvDeleteVector( vect_ex );
    icvDeleteVector( objectPoints2D );
    icvDeleteVector( tmp_vectwidthJJ );
    icvDeleteVector( tmp_vectwidthJJ_64d );
    icvDeleteMatrix( matrJJ );
    icvDeleteMatrix( matrJJt );
    icvDeleteMatrix( matrJJt_64d );
    icvDeleteVector( derivPointsRot );
    icvDeleteVector( derivPointsTrans );
    icvDeleteVector( derivPointsFocal );
    icvDeleteVector( derivPointsPrincipal );
    icvDeleteVector( derivPointsDistort );

    return CV_NO_ERR;
}

/*======================================================================================*/

/* Convertors From float input data to double data */
static CvStatus icvCalibrateCamera( int numImages,
                             int *numPoints,
                             CvSize imageSize,
                             CvPoint2D32f * imagePoints32f,
                             CvPoint3D32f * objectPoints32f,
                             CvVect32f distortion32f,
                             CvMatr32f cameraMatrix32f,
                             CvVect32f transVects32f,
                             CvMatr32f rotMatrs32f,
                             int useIntrinsicGuess )
{
    CvStatus status;
    CvPoint2D64d *imagePoints64d;
    CvPoint3D64d *objectPoints64d;
    double distortion64d[4];
    double cameraMatrix64d[3 * 3];
    CvVect64d transVects64d;
    CvMatr64d rotMatrs64d;
    int totalNumber;
    int curr;

    /* Count total numbers of points */
    totalNumber = 0;
    for( curr = 0; curr < numImages; curr++ )
    {
        totalNumber += numPoints[curr];
    }

    /* Allocate memory */
    imagePoints64d = (CvPoint2D64d *) icvCreateVector_64d( totalNumber * 2 );
    objectPoints64d = (CvPoint3D64d *) icvCreateVector_64d( totalNumber * 3 );
    transVects64d = icvCreateVector_64d( numImages * 3 );
    rotMatrs64d = icvCreateVector_64d( numImages * 3 * 3 );

    /* Convert this data to double */
    icvCvt_32f_64d( (float*) imagePoints32f,
                    (double*) imagePoints64d, totalNumber * 2 );
    
    icvCvt_32f_64d( (float*) objectPoints32f,
                    (double*) objectPoints64d, totalNumber * 3 );

    /* Call double data function */
    status = icvCalibrateCamera_64d( numImages,
                                     numPoints,
                                     imageSize,
                                     imagePoints64d,
                                     objectPoints64d,
                                     distortion64d,
                                     cameraMatrix64d,
                                     transVects64d, rotMatrs64d, useIntrinsicGuess );

    /* Convert to float */
    icvCvt_64d_32f( distortion64d, distortion32f, 4 );
    icvCvt_64d_32f( cameraMatrix64d, cameraMatrix32f, 3 * 3 );
    icvCvt_64d_32f( transVects64d, transVects32f, numImages * 3 );
    icvCvt_64d_32f( rotMatrs64d, rotMatrs32f, numImages * 3 * 3 );

    /* Free allocated memory */
    icvDeleteVector( imagePoints64d );
    icvDeleteVector( objectPoints64d );
    icvDeleteVector( transVects64d );
    icvDeleteVector( rotMatrs64d );

    return status;
}

/*======================================================================================*/

static CvStatus icvFindExtrinsicCameraParams( int numPoints,
                                              CvSize imageSize,
                                              CvPoint2D32f * imagePoints32f,
                                              CvPoint3D32f * objectPoints32f,
                                              CvVect32f focalLength32f,
                                              CvPoint2D32f principalPoint32f,
                                              CvVect32f distortion32f,
                                              CvVect32f rotVect32f,
                                              CvVect32f transVect32f )
{
    CvStatus status;
    CvPoint2D64d *imagePoints64d;
    CvPoint3D64d *objectPoints64d;
    double focalLength64d[2] = { 0, 0 };
    CvPoint2D64d principalPoint64d = { 0, 0 };
    double distortion64d[4] = { 0, 0, 0, 0 };
    double rotVect64d[3] = { 0, 0, 0 };
    double transVect64d[3] = { 0, 0, 0 };

    /* Allocate memory */
    imagePoints64d = (CvPoint2D64d *) icvCreateVector_64d( numPoints * 2 );
    objectPoints64d = (CvPoint3D64d *) icvCreateVector_64d( numPoints * 3 );

    /* Convert this data to double */
    icvCvt_32f_64d( (float *) imagePoints32f,
                     (double *) imagePoints64d, numPoints * 2 );
    
    icvCvt_32f_64d( (float *) objectPoints32f,
                     (double *) objectPoints64d, numPoints * 3 );

    icvCvt_32f_64d( focalLength32f, focalLength64d, 2 );
    icvCvt_32f_64d( (float *) &principalPoint32f, (double *) &principalPoint64d, 2 );
    icvCvt_32f_64d( distortion32f, distortion64d, 4 );

    status = icvFindExtrinsicCameraParams_64d( numPoints,
                                               imageSize,
                                               imagePoints64d,
                                               objectPoints64d,
                                               focalLength64d,
                                               principalPoint64d,
                                               distortion64d,
                                               rotVect64d, transVect64d );

    /* Convert to float */
    icvCvt_64d_32f( rotVect64d, rotVect32f, 3 );
    icvCvt_64d_32f( transVect64d, transVect32f, 3 );

    /* Free allocated memory */
    icvDeleteVector( imagePoints64d );
    icvDeleteVector( objectPoints64d );

    return status;
}

/*======================================================================================*/

IPCVAPI_IMPL( CvStatus, icvRodrigues, (CvMatr32f rotMatr32f,
                                       CvVect32f rotVect32f,
                                       CvMatr32f Jacobian32f, CvRodriguesType convType) )
{

    double rotMatr64d[3 * 3];
    double rotVect64d[3];
    double Jacobian64d[3 * 9];
    CvStatus status;

    if( convType == CV_RODRIGUES_V2M )
    {
        icvCvt_32f_64d( rotVect32f, rotVect64d, 3 );
    }
    else
    {
        icvCvt_32f_64d( rotMatr32f, rotMatr64d, 3 * 3 );
    }

    status = icvRodrigues_64d( rotMatr64d, rotVect64d, Jacobian32f ? Jacobian64d : 0, convType );

    if( convType == CV_RODRIGUES_V2M )
    {
        icvCvt_64d_32f( rotMatr64d, rotMatr32f, 3 * 3 );
    }
    else
    {
        icvCvt_64d_32f( rotVect64d, rotVect32f, 3 );
    }

    if( Jacobian32f )
        icvCvt_64d_32f( Jacobian64d, Jacobian32f, 3 * 9 );

    return status;

}


CV_IMPL void
cvCalibrateCamera( int numImages,
                   int *numPoints,
                   CvSize imageSize,
                   CvPoint2D32f * imagePoints32f,
                   CvPoint3D32f * objectPoints32f,
                   CvVect32f distortion32f,
                   CvMatr32f cameraMatrix32f,
                   CvVect32f transVects32f,
                   CvMatr32f rotMatrs32f,
                   int useIntrinsicGuess )
{
    CV_FUNCNAME( "cvCalibrateCamera" );
    __BEGIN__;

    IPPI_CALL( icvCalibrateCamera( numImages,
                                   numPoints,
                                   imageSize,
                                   imagePoints32f,
                                   objectPoints32f,
                                   distortion32f,
                                   cameraMatrix32f,
                                   transVects32f,
                                   rotMatrs32f,
                                   useIntrinsicGuess ));
    __CLEANUP__;
    __END__;
}

/*======================================================================================*/

CV_IMPL void
cvCalibrateCamera_64d( int numImages,
                       int *numPoints,
                       CvSize imageSize,
                       CvPoint2D64d * imagePoints,
                       CvPoint3D64d * objectPoints,
                       CvVect64d distortion,
                       CvMatr64d cameraMatrix,
                       CvVect64d transVects, CvMatr64d rotMatrs, int useIntrinsicGuess )
{
    CV_FUNCNAME( "cvCalibrateCamera_64d" );
    __BEGIN__;

    IPPI_CALL( icvCalibrateCamera_64d( numImages,
                                       numPoints,
                                       imageSize,
                                       imagePoints,
                                       objectPoints,
                                       distortion,
                                       cameraMatrix,
                                       transVects, rotMatrs, useIntrinsicGuess ));
    __CLEANUP__;
    __END__;
}

/*======================================================================================*/
CV_IMPL void
cvFindExtrinsicCameraParams( int numPoints,
                             CvSize imageSize,
                             CvPoint2D32f * imagePoints32f,
                             CvPoint3D32f * objectPoints32f,
                             CvVect32f focalLength32f,
                             CvPoint2D32f principalPoint32f,
                             CvVect32f distortion32f,
                             CvVect32f rotVect32f, CvVect32f transVect32f )
{
    CV_FUNCNAME( "cvFindExtrinsicCameraParams" );
    __BEGIN__;

    IPPI_CALL( icvFindExtrinsicCameraParams( numPoints,
                                             imageSize,
                                             imagePoints32f,
                                             objectPoints32f,
                                             focalLength32f,
                                             principalPoint32f,
                                             distortion32f, rotVect32f, transVect32f ));
    __CLEANUP__;
    __END__;
}

/*======================================================================================*/

CV_IMPL void
cvFindExtrinsicCameraParams_64d( int numPoints,
                                 CvSize imageSize,
                                 CvPoint2D64d * imagePoints,
                                 CvPoint3D64d * objectPoints,
                                 CvVect64d focalLength,
                                 CvPoint2D64d principalPoint,
                                 CvVect64d distortion,
                                 CvVect64d rotVect,
                                 CvVect64d transVect )
{
    CV_FUNCNAME( "cvFindExtrinsicCameraParams_64d" );
    __BEGIN__;

    IPPI_CALL( icvFindExtrinsicCameraParams_64d( numPoints,
                                                 imageSize,
                                                 imagePoints,
                                                 objectPoints,
                                                 focalLength,
                                                 principalPoint,
                                                 distortion, rotVect, transVect ));
    __CLEANUP__;
    __END__;
}
/*======================================================================================*/

CvStatus icvProjectPointsSimple(  int numPoints,
                CvPoint3D64d * objectPoints,
                CvVect64d rotMatr,
                CvVect64d transVect,
                CvMatr64d cameraMatrix,
                CvVect64d distortion,
                CvPoint2D64d* imagePoints)
{
    CvPoint2D64d *x;
    CvMatr64d trans_x;
    CvVect64d r2;
    CvVect64d r4;
    CvVect64d cdist;
    double tmp;
    double tmpx, tmpy;
    CvMatr64d xd1;
    CvMatr64d xd2;
    CvVect64d tmpn;
    CvVect64d a1;
    CvVect64d a2;
    CvVect64d a3;
    CvMatr64d delta_x;
    double focalLength[2];
    CvPoint2D64d principalPoint;

    focalLength[0] = cameraMatrix[0];
    focalLength[1] = cameraMatrix[4];

    principalPoint.x = cameraMatrix[2];
    principalPoint.y = cameraMatrix[5];

    int t;

    x = (CvPoint2D64d *) icvCreateVector_64d( numPoints * 2 );
    
    trans_x     = icvCreateMatrix_64d( numPoints, 2 );
    r2          = icvCreateVector_64d( numPoints );
    r4          = icvCreateVector_64d( numPoints );
    cdist       = icvCreateVector_64d( numPoints );
    xd1         = icvCreateMatrix_64d( numPoints, 2 );
    xd2         = icvCreateMatrix_64d( numPoints, 2 );
    a1          = icvCreateVector_64d( numPoints );
    a2          = icvCreateVector_64d( numPoints );
    a3          = icvCreateVector_64d( numPoints );
    delta_x     = icvCreateMatrix_64d( numPoints, 2 );
    tmpn        = icvCreateVector_64d( numPoints );

    /* Apply rotate matrix */

    double pointM[3];
    CvMat cRotMatr = cvMat(3,3,CV_MAT64D,rotMatr);
    CvMat cPoint   = cvMat(3,1,CV_MAT64D,pointM);
    CvMat cTransVect = cvMat(3,1,CV_MAT64D,transVect);
    
    int currPoint;

    for( currPoint = 0; currPoint < numPoints; currPoint++ )
    {
        double invZ;
        CvMat point = cvMat(3,1,CV_MAT64D,objectPoints+currPoint);
        cvMatMulAdd(&cRotMatr,&point,&cTransVect, &cPoint);

        invZ  = 1.0 / cPoint.data.db[2];
        x[currPoint].x = cPoint.data.db[0] * invZ;
        x[currPoint].y = cPoint.data.db[1] * invZ;
    }

    icvTransposeMatrix_64d( (double *) x, 2, numPoints, trans_x );

    /* Add Distortion */
    for( t = 0; t < numPoints; t++ )
    {
        tmp = x[t].x * x[t].x + x[t].y * x[t].y;
        r2[t] = tmp;
        r4[t] = tmp * tmp;
    }

    /* Radial distortion  */
    for( t = 0; t < numPoints; t++ )
    {
        cdist[t] = 1.0f + distortion[0] * r2[t] + distortion[1] * r4[t];
    }

    icvMulVectors_64d( trans_x, cdist, xd1, numPoints );
    icvMulVectors_64d( trans_x + numPoints, cdist, xd1 + numPoints, numPoints );

    /* Tangential distortion */
    for( t = 0; t < numPoints; t++ )
    {
        tmpx = x[t].x;
        tmpy = x[t].y;
        a1[t] = 2 * tmpx * tmpy;
        a2[t] = r2[t] + 2 * tmpx * tmpx;
        a3[t] = r2[t] + 2 * tmpy * tmpy;
    }

    icvScaleVector_64d( a1, delta_x, numPoints, distortion[2] );
    icvScaleVector_64d( a2, tmpn, numPoints, distortion[3] );
    icvAddVector_64d( delta_x, tmpn, delta_x, numPoints );

    icvScaleVector_64d( a3, delta_x + numPoints, numPoints, distortion[2] );
    icvScaleVector_64d( a1, tmpn, numPoints, distortion[3] );
    icvAddVector_64d( delta_x + numPoints, tmpn, delta_x + numPoints, numPoints );

    icvAddVector_64d( xd1, delta_x, xd2, 2 * numPoints );

    /* Pixel coordinates */

    for( t = 0; t < numPoints; t++ )
    {
        imagePoints[t].x = xd2[0 * numPoints + t] * focalLength[0] + principalPoint.x;
        imagePoints[t].y = xd2[1 * numPoints + t] * focalLength[1] + principalPoint.y;
    }

    icvDeleteVector( x );
    icvDeleteMatrix( trans_x );
    icvDeleteVector( r2 );
    icvDeleteVector( r4 );
    icvDeleteVector( cdist );
    icvDeleteMatrix( xd1 );
    icvDeleteMatrix( xd2 );
    icvDeleteVector( a1 );
    icvDeleteVector( a2 );
    icvDeleteVector( a3 );
    icvDeleteMatrix( delta_x );
    icvDeleteVector( tmpn );
    return CV_NO_ERR;
}

/*======================================================================================*/
CV_IMPL void
cvProjectPointsSimple(  int numPoints,
                CvPoint3D64d * objectPoints,
                CvVect64d rotMatr,
                CvVect64d transVect,
                CvMatr64d cameraMatrix,
                CvVect64d distortion,
                CvPoint2D64d* imagePoints)
{
    CV_FUNCNAME( "cvProjectPointsSimple" );
    __BEGIN__;

    IPPI_CALL(icvProjectPointsSimple(   numPoints,
                                        objectPoints,
                                        rotMatr,
                                        transVect,
                                        cameraMatrix,
                                        distortion,
                                        imagePoints));
    __CLEANUP__;
    __END__;
}


/*======================================================================================*/

void
cvProjectPoints( int numPoints,
                 CvPoint3D64d * objectPoints,
                 CvVect64d rotVect,
                 CvVect64d transVect,
                 CvVect64d focalLength,
                 CvPoint2D64d principalPoint,
                 CvVect64d distortion,
                 CvPoint2D64d * imagePoints,
                 CvVect64d derivPointsRot,
                 CvVect64d derivPointsTrans,
                 CvVect64d derivPointsFocal,
                 CvVect64d derivPointsPrincipal, CvVect64d derivPointsDistort )
{
    CV_FUNCNAME( "cvProjectPoints" );

    __BEGIN__;

    IPPI_CALL( icvProjectPoints( numPoints, objectPoints, rotVect,
                                 transVect, focalLength, principalPoint,
                                 distortion, imagePoints, derivPointsRot,
                                 derivPointsTrans, derivPointsFocal,
                                 derivPointsPrincipal, derivPointsDistort ));

    __CLEANUP__;
    __END__;
}
