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

#include "cvtest.h"
//#include "_ipcvMatrix.h"

/* Testing parameters */
static char FuncName[]  = "cvCreatePOSITObject, cvPOSIT, cvReleasePOSITObject";
static char TestName[]  = "POSIT algorithm";
static char TestClass[]   = "Algorithm";

static float flFocalLength;
static float flEpsilon;

typedef float*  Vect32f;
typedef float*  Matr32f;


static int fmaPOSIT(void)
{
    double pi = 3.1415926535;

    /* fixed parameters output */
    /*float rot[3][3]={  0.49010f,  0.85057f, 0.19063f,
                      -0.56948f,  0.14671f, 0.80880f,
                       0.65997f, -0.50495f, 0.55629f };

    float trans[3] = { 0.0f, 0.0f, 40.02637f };
    */

    /* Some variables */
    int        Errors = 0;
    int i,counter;

    CvTermCriteria criteria;
    CvPoint3D32f* obj_points;
    CvPoint2D32f* img_points;
    CvPOSITObject* object;

    float angleX, angleY, angleZ;

    CvMat true_rotationX = cvMat( 3, 3, CV_MAT32F, NULL );
    CvMat true_rotationY = cvMat( 3, 3, CV_MAT32F, NULL );
    CvMat true_rotationZ = cvMat( 3, 3, CV_MAT32F, NULL );
    CvMat tmp_matrix = cvMat( 3, 3, CV_MAT32F, NULL );

    CvMat true_rotation = cvMat( 3, 3, CV_MAT32F, NULL );
    CvMat rotation = cvMat( 3, 3, CV_MAT32F, NULL );
    
    cvmAlloc( &true_rotationX );
    cvmAlloc( &true_rotationY );
    cvmAlloc( &true_rotationZ );
    cvmAlloc( &tmp_matrix );
    cvmAlloc( &true_rotation );
    cvmAlloc( &rotation );

    CvMat translation = cvMat( 3, 1, CV_MAT32F, NULL );
    CvMat true_translation = cvMat( 3, 1, CV_MAT32F, NULL );
    cvmAlloc( &translation );
    cvmAlloc( &true_translation ); 

    static int  read_param = 0;

    /* Initialization global parameters */
    if( !read_param )
    {
        read_param = 1;
        trssRead( &flFocalLength, "760", "focal length of camera" );
        trssRead( &flEpsilon, "0.1", "epsilon" );

    }

    /* Initilization */
    criteria.type = CV_TERMCRIT_EPS|CV_TERMCRIT_ITER;
    criteria.epsilon = flEpsilon;
    criteria.max_iter = 10000;

    /* Allocating source arrays; */
    obj_points = (CvPoint3D32f*)cvAlloc( 8 * sizeof(CvPoint3D32f) );
    img_points = (CvPoint2D32f*)cvAlloc( 8 * sizeof(CvPoint2D32f) );

    /* Fill points arrays with values */

    /* cube model with edge size 10 */
    obj_points[0].x = 0;  obj_points[0].y = 0;  obj_points[0].z = 0;
    obj_points[1].x = 10; obj_points[1].y = 0;  obj_points[1].z = 0;
    obj_points[2].x = 10; obj_points[2].y = 10; obj_points[2].z = 0;
    obj_points[3].x = 0;  obj_points[3].y = 10; obj_points[3].z = 0;
    obj_points[4].x = 0;  obj_points[4].y = 0;  obj_points[4].z = 10;
    obj_points[5].x = 10; obj_points[5].y = 0;  obj_points[5].z = 10;
    obj_points[6].x = 10; obj_points[6].y = 10; obj_points[6].z = 10;
    obj_points[7].x = 0;  obj_points[7].y = 10; obj_points[7].z = 10;

    /* Loop for test some random object positions */
    for ( counter = 0; counter < 20; counter++ )
    {
        /* set all rotation matrix to zero */
        cvmSetZero( &true_rotationX );
        cvmSetZero( &true_rotationY );
        cvmSetZero( &true_rotationZ );
        
        /* fill random rotation matrix */
        angleX = (float)atsInitRandom( 0, 2 * pi );
        angleY = (float)atsInitRandom( 0, 2 * pi );
        angleZ = (float)atsInitRandom( 0, 2 * pi );

        true_rotationX.data.fl[0 *3+ 0] = 1;
        true_rotationX.data.fl[1 *3+ 1] = (float)cos(angleX);
        true_rotationX.data.fl[2 *3+ 2] = true_rotationX.data.fl[1 *3+ 1];
        true_rotationX.data.fl[1 *3+ 2] = -(float)sin(angleX);
        true_rotationX.data.fl[2 *3+ 1] = -true_rotationX.data.fl[1 *3+ 2];

        true_rotationY.data.fl[1 *3+ 1] = 1;
        true_rotationY.data.fl[0 *3+ 0] = (float)cos(angleY);
        true_rotationY.data.fl[2 *3+ 2] = true_rotationY.data.fl[0 *3+ 0];
        true_rotationY.data.fl[0 *3+ 2] = -(float)sin(angleY);
        true_rotationY.data.fl[2 *3+ 0] = -true_rotationY.data.fl[0 *3+ 2];

        true_rotationZ.data.fl[2 *3+ 2] = 1;
        true_rotationZ.data.fl[0 *3+ 0] = (float)cos(angleZ);
        true_rotationZ.data.fl[1 *3+ 1] = true_rotationZ.data.fl[0 *3+ 0];
        true_rotationZ.data.fl[0 *3+ 1] = -(float)sin(angleZ);
        true_rotationZ.data.fl[1 *3+ 0] = -true_rotationZ.data.fl[0 *3+ 1];

        cvmMul( &true_rotationX, &true_rotationY, &tmp_matrix);
        cvmMul( &tmp_matrix, &true_rotationZ, &true_rotation);

        /* fill translation vector */
        true_translation.data.fl[2] = (float)atsInitRandom( 40, 2 * flFocalLength );
        true_translation.data.fl[0] = (float)atsInitRandom( -true_translation.data.fl[2], true_translation.data.fl[2] );
        true_translation.data.fl[1] = (float)atsInitRandom( -true_translation.data.fl[2], true_translation.data.fl[2] );

        /* calculate perspective projection */
        for ( i = 0; i < 8; i++ )
        {
            float vec[3];
            CvMat Vec = cvMat( 3, 1, CV_MAT32F, vec );
            CvMat Obj_point = cvMat( 3, 1, CV_MAT32F, &obj_points[i].x );

            cvmMul( &true_rotation, &Obj_point, &Vec );

            vec[0] += true_translation.data.fl[0];
            vec[1] += true_translation.data.fl[1];
            vec[2] += true_translation.data.fl[2];

            img_points[i].x = flFocalLength * vec[0] / vec[2];
            img_points[i].y = flFocalLength * vec[1] / vec[2];
        }

        /*img_points[0].x = 0 ; img_points[0].y =   0;
        img_points[1].x = 80; img_points[1].y = -93;
        img_points[2].x = 245;img_points[2].y =  -77;
        img_points[3].x = 185;img_points[3].y =  32;
        img_points[4].x = 32; img_points[4].y = 135;
        img_points[5].x = 99; img_points[5].y = 35;
        img_points[6].x = 247; img_points[6].y = 62;
        img_points[7].x = 195; img_points[7].y = 179;
        */

        object = cvCreatePOSITObject( obj_points, 8 );
        cvPOSIT( object, img_points, flFocalLength, criteria,
                              rotation.data.fl, translation.data.fl );
        cvReleasePOSITObject( &object );


        Errors += atsCompSinglePrec( true_rotation.data.fl,
                                     rotation.data.fl, 
                                     9, flEpsilon);
        
        Errors += atsCompSinglePrec( true_translation.data.fl,
                                     translation.data.fl, 3, flEpsilon);
        
        /* output true */
        trsWrite( ATS_LST , " True rotation matrix:\n %f %f %f\n %f %f %f\n %f %f %f\n",
                             true_rotation.data.fl[0],
                             true_rotation.data.fl[1],
                             true_rotation.data.fl[2],
                             true_rotation.data.fl[3],
                             true_rotation.data.fl[4],
                             true_rotation.data.fl[5],
                             true_rotation.data.fl[6],
                             true_rotation.data.fl[7],
                             true_rotation.data.fl[8] );

        trsWrite( ATS_LST , " True translation vector:\n %f %f %f \n ",
                            true_translation.data.fl[0],
                            true_translation.data.fl[1],
                            true_translation.data.fl[2] );  

        /* output computed */
        trsWrite( ATS_LST , " Rotation matrix:\n %f %f %f\n %f %f %f\n %f %f %f\n",
                             rotation.data.fl[0],
                             rotation.data.fl[1],
                             rotation.data.fl[2],
                             rotation.data.fl[3],
                             rotation.data.fl[4],
                             rotation.data.fl[5],
                             rotation.data.fl[6],
                             rotation.data.fl[7],
                             rotation.data.fl[8] );

        trsWrite( ATS_LST , " Translation vector:\n %f %f %f \n ",
                            translation.data.fl[0],
                            translation.data.fl[1],
                            translation.data.fl[2] );  
    }

    cvFree( &obj_points );
    cvFree( &img_points );

    cvmFree( &true_rotationX );
    cvmFree( &true_rotationY );
    cvmFree( &true_rotationZ );
    cvmFree( &tmp_matrix );
    cvmFree( &true_rotation );
    cvmFree( &rotation );
    cvmFree( &translation );
    cvmFree( &true_translation ); 
       
    if (Errors)
        return trsResult( TRS_FAIL, "Total fixed %d errors", Errors );
    else return trsResult( TRS_OK, "No errors fixed for this test" );

}

void InitAPOSIT(void)
{
    trsReg( FuncName, TestName, TestClass, fmaPOSIT);

} /* InitAPOSIT */


/* End of file. */
