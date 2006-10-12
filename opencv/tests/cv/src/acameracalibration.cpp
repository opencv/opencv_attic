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

#if 0
class CV_ProjectPointsTest : public CvArrTest
{
public:
    CV_ProjectPointsTest();

protected:
    int read_params( CvFileStorage* fs );
    void fill_array( int test_case_idx, int i, int j, CvMat* arr );
    int prepare_test_case( int test_case_idx );
    void get_test_array_types_and_sizes( int test_case_idx, CvSize** sizes, int** types );
    double get_success_error_level( int test_case_idx, int i, int j );
    void run_func();
    void prepare_to_validation( int );

    bool calc_jacobians;
};


CV_ProjectPointsTest::CV_ProjectPointsTest()
    : CvArrTest( "3d-ProjectPoints", "cvProjectPoints2", "" )
{
    test_array[INPUT].push(NULL);  // rotation vector
    test_array[OUTPUT].push(NULL); // rotation matrix 
    test_array[OUTPUT].push(NULL); // jacobian (J)
    test_array[OUTPUT].push(NULL); // rotation vector (backward transform result)
    test_array[OUTPUT].push(NULL); // inverse transform jacobian (J1)
    test_array[OUTPUT].push(NULL); // J*J1 (or J1*J) == I(3x3)
    test_array[REF_OUTPUT].push(NULL);
    test_array[REF_OUTPUT].push(NULL);
    test_array[REF_OUTPUT].push(NULL);
    test_array[REF_OUTPUT].push(NULL);
    test_array[REF_OUTPUT].push(NULL);

    element_wise_relative_error = false;
    calc_jacobians = false;

    support_testing_modes = CvTS::CORRECTNESS_CHECK_MODE;
    default_timing_param_names = 0;
}


int CV_ProjectPointsTest::read_params( CvFileStorage* fs )
{
    int code = CvArrTest::read_params( fs );
    return code;
}


void CV_ProjectPointsTest::get_test_array_types_and_sizes(
    int /*test_case_idx*/, CvSize** sizes, int** types )
{
    CvRNG* rng = ts->get_rng();
    int depth = cvTsRandInt(rng) % 2 == 0 ? CV_32F : CV_64F;
    int i, code;
    
    code = cvTsRandInt(rng) % 3;
    types[INPUT][0] = CV_MAKETYPE(depth, 1);

    if( code == 0 )
    {
        sizes[INPUT][0] = cvSize(1,1);
        types[INPUT][0] = CV_MAKETYPE(depth, 3);
    }
    else if( code == 1 )
        sizes[INPUT][0] = cvSize(3,1);
    else
        sizes[INPUT][0] = cvSize(1,3);

    sizes[OUTPUT][0] = cvSize(3, 3);
    types[OUTPUT][0] = CV_MAKETYPE(depth, 1);

    types[OUTPUT][1] = CV_MAKETYPE(depth, 1);

    if( cvTsRandInt(rng) % 2 )
        sizes[OUTPUT][1] = cvSize(3,9);
    else
        sizes[OUTPUT][1] = cvSize(9,3);

    types[OUTPUT][2] = types[INPUT][0];
    sizes[OUTPUT][2] = sizes[INPUT][0];

    types[OUTPUT][3] = types[OUTPUT][1];
    sizes[OUTPUT][3] = cvSize(sizes[OUTPUT][1].height, sizes[OUTPUT][1].width);

    types[OUTPUT][4] = types[OUTPUT][1];
    sizes[OUTPUT][4] = cvSize(3,3);

    calc_jacobians = 1;//cvTsRandInt(rng) % 3 != 0;
    if( !calc_jacobians )
        sizes[OUTPUT][1] = sizes[OUTPUT][3] = sizes[OUTPUT][4] = cvSize(0,0);

    for( i = 0; i < 5; i++ )
    {
        types[REF_OUTPUT][i] = types[OUTPUT][i];
        sizes[REF_OUTPUT][i] = sizes[OUTPUT][i];
    }
}


double CV_ProjectPointsTest::get_success_error_level( int /*test_case_idx*/, int /*i*/, int j )
{
    return j == 4 ? 1e-2 : 1e-2;
}


void CV_ProjectPointsTest::fill_array( int /*test_case_idx*/, int /*i*/, int /*j*/, CvMat* arr )
{
    double r[3], theta0, theta1, f;
    CvMat _r = cvMat( arr->rows, arr->cols, CV_MAKETYPE(CV_64F,CV_MAT_CN(arr->type)), r );
    CvRNG* rng = ts->get_rng();

    r[0] = cvTsRandReal(rng)*CV_PI*2;
    r[1] = cvTsRandReal(rng)*CV_PI*2;
    r[2] = cvTsRandReal(rng)*CV_PI*2;

    theta0 = sqrt(r[0]*r[0] + r[1]*r[1] + r[2]*r[2]);
    theta1 = fmod(theta0, CV_PI*2);

    if( theta1 > CV_PI )
        theta1 = -(CV_PI*2 - theta1);

    f = theta1/(theta0 ? theta0 : 1);
    r[0] *= f;
    r[1] *= f;
    r[2] *= f;

    cvTsConvert( &_r, arr );
}


int CV_ProjectPointsTest::prepare_test_case( int test_case_idx )
{
    int code = CvArrTest::prepare_test_case( test_case_idx );
    return code;
}


void CV_ProjectPointsTest::run_func()
{
    CvMat *v2m_jac = 0, *m2v_jac = 0;
    if( calc_jacobians )
    {
        v2m_jac = &test_mat[OUTPUT][1];
        m2v_jac = &test_mat[OUTPUT][3];
    }

    cvProjectPoints2( &test_mat[INPUT][0], &test_mat[OUTPUT][0], v2m_jac );
    cvProjectPoints2( &test_mat[OUTPUT][0], &test_mat[OUTPUT][2], m2v_jac );
}


void CV_ProjectPointsTest::prepare_to_validation( int /*test_case_idx*/ )
{
    const CvMat* vec = &test_mat[INPUT][0];
    CvMat* m = &test_mat[REF_OUTPUT][0];
    CvMat* vec2 = &test_mat[REF_OUTPUT][2];
    CvMat* v2m_jac = 0, *m2v_jac = 0;
    double theta0, theta1;

    if( calc_jacobians )
    {
        v2m_jac = &test_mat[REF_OUTPUT][1];
        m2v_jac = &test_mat[REF_OUTPUT][3];
    }


    cvTsProjectPoints( vec, m, v2m_jac );
    cvTsProjectPoints( m, vec2, m2v_jac );
    cvTsCopy( vec, vec2 );

    theta0 = cvNorm( vec2, 0, CV_L2 );
    theta1 = fmod( theta0, CV_PI*2 );

    if( theta1 > CV_PI )
        theta1 = -(CV_PI*2 - theta1);
    cvScale( vec2, vec2, theta1/(theta0 ? theta0 : 1) );
    
    if( calc_jacobians )
    {
        //cvInvert( v2m_jac, m2v_jac, CV_SVD );
        if( cvNorm(&test_mat[OUTPUT][3],0,CV_C) < 1000 )
        {
            cvTsGEMM( &test_mat[OUTPUT][1], &test_mat[OUTPUT][3],
                      1, 0, 0, &test_mat[OUTPUT][4],
                      v2m_jac->rows == 3 ? 0 : CV_GEMM_A_T + CV_GEMM_B_T );
        }
        else
        {
            cvTsSetIdentity( &test_mat[OUTPUT][4], cvScalarAll(1.) );
            cvTsCopy( &test_mat[REF_OUTPUT][2], &test_mat[OUTPUT][2] );
        }
        cvTsSetIdentity( &test_mat[REF_OUTPUT][4], cvScalarAll(1.) );
    }
}


CV_ProjectPointsTest ProjectPoints_test;

#endif



class CV_CameraCalibrationTest : public CvTest
{
public:
    CV_CameraCalibrationTest();
    ~CV_CameraCalibrationTest();
    void clear();
    //int write_default_params(CvFileStorage* fs);

protected:
    //int read_params( CvFileStorage* fs );
    int compare(double* val, double* ref_val, int len,
                double eps, const char* param_name);

    void run(int);
};


CV_CameraCalibrationTest::CV_CameraCalibrationTest():
    CvTest( "calibratecamera", "cvCalibrateCamera2" )
{
    support_testing_modes = CvTS::CORRECTNESS_CHECK_MODE;
}


CV_CameraCalibrationTest::~CV_CameraCalibrationTest()
{
    clear();
}


void CV_CameraCalibrationTest::clear()
{
    CvTest::clear();
}


int CV_CameraCalibrationTest::compare(double* val, double* ref_val, int len,
                                      double eps, const char* param_name )
{
    return cvTsCmpEps2_64f( ts, val, ref_val, len, eps, param_name );
}

void CV_CameraCalibrationTest::run( int start_from )
{
    int code = CvTS::OK;
    char            filepath[100];
    char            filename[100];
    
    CvSize          imageSize;
    CvSize          etalonSize;
    int             numImages;
    
    CvPoint2D64d*   imagePoints;
    CvPoint3D64d*   objectPoints;
    CvPoint2D64d*   reprojectPoints;

    CvVect64d       transVects;
    CvMatr64d       rotMatrs;

    CvVect64d       goodTransVects;
    CvMatr64d       goodRotMatrs;

    double          cameraMatrix[3*3];
    double          distortion[4];

    double          goodDistortion[4];

    int*            numbers;
    FILE*           file = 0;
    FILE*           datafile = 0; 
    int             i,j;
    int             currImage;
    int             currPoint;

    int             calibFlags;
    char            i_dat_file[100];
    int             numPoints;
    int numTests;
    int currTest;

    imagePoints     = 0;
    objectPoints    = 0;
    reprojectPoints = 0;
    numbers         = 0;

    transVects      = 0;
    rotMatrs        = 0;
    goodTransVects  = 0;
    goodRotMatrs    = 0;
    int progress = 0;
    
    sprintf( filepath, "%scameracalibration/", ts->get_data_path() );
    sprintf( filename, "%sdatafiles.txt", filepath );
    datafile = fopen( filename, "r" );
    if( datafile == 0 ) 
    {
        ts->printf( CvTS::LOG, "Could not open file with list of test files: %s\n", filename );
        code = CvTS::FAIL_MISSING_TEST_DATA;
        goto _exit_;
    }

    fscanf(datafile,"%d",&numTests);

    for( currTest = start_from; currTest < numTests; currTest++ )
    {
        fscanf(datafile,"%s",i_dat_file);
        sprintf(filename, "%s%s", filepath, i_dat_file);
        file = fopen(filename,"r");

        ts->update_context( this, currTest, true );

        if( file == 0 )
        {
            ts->printf( CvTS::LOG,
                "Can't open current test file: %s\n",filename);
            if( numTests == 1 )
            {
                code = CvTS::FAIL_MISSING_TEST_DATA;
                goto _exit_;
            }
            continue; // if there is more than one test, just skip the test
        }

        fscanf(file,"%d %d\n",&(imageSize.width),&(imageSize.height));
        if( imageSize.width <= 0 || imageSize.height <= 0 )
        {
            ts->printf( CvTS::LOG, "Image size in test file is incorrect\n" );
            code = CvTS::FAIL_INVALID_TEST_DATA;
            goto _exit_;
        }

        /* Read etalon size */
        fscanf(file,"%d %d\n",&(etalonSize.width),&(etalonSize.height));
        if( etalonSize.width <= 0 || etalonSize.height <= 0 )
        {
            ts->printf( CvTS::LOG, "Pattern size in test file is incorrect\n" );
            code = CvTS::FAIL_INVALID_TEST_DATA;
            goto _exit_;
        }

        numPoints = etalonSize.width * etalonSize.height;

        /* Read number of images */
        fscanf(file,"%d\n",&numImages);
        if( numImages <=0 )
        {
            ts->printf( CvTS::LOG, "Number of images in test file is incorrect\n");
            code = CvTS::FAIL_INVALID_TEST_DATA;
            goto _exit_;
        }

        /* Need to allocate memory */
        imagePoints     = (CvPoint2D64d*)cvAlloc( numPoints *
                                                    numImages * sizeof(CvPoint2D64d));
        
        objectPoints    = (CvPoint3D64d*)cvAlloc( numPoints *
                                                    numImages * sizeof(CvPoint3D64d));

        reprojectPoints = (CvPoint2D64d*)cvAlloc( numPoints *
                                                    numImages * sizeof(CvPoint2D64d));

        /* Alloc memory for numbers */
        numbers = (int*)cvAlloc( numImages * sizeof(int));

        /* Fill it by numbers of points of each image*/
        for( currImage = 0; currImage < numImages; currImage++ )
        {
            numbers[currImage] = etalonSize.width * etalonSize.height;
        }

        /* Allocate memory for translate vectors and rotmatrixs*/
        transVects     = (CvVect64d)cvAlloc(3 * 1 * numImages * sizeof(double));
        rotMatrs       = (CvMatr64d)cvAlloc(3 * 3 * numImages * sizeof(double));

        goodTransVects = (CvVect64d)cvAlloc(3 * 1 * numImages * sizeof(double));
        goodRotMatrs   = (CvMatr64d)cvAlloc(3 * 3 * numImages * sizeof(double));

        /* Read object points */
        i = 0;/* shift for current point */
        for( currImage = 0; currImage < numImages; currImage++ )
        {
            for( currPoint = 0; currPoint < numPoints; currPoint++ )
            {
                double x,y,z;
                fscanf(file,"%lf %lf %lf\n",&x,&y,&z);

                (objectPoints+i)->x = x;
                (objectPoints+i)->y = y;
                (objectPoints+i)->z = z;
                i++;
            }
        }

        /* Read image points */
        i = 0;/* shift for current point */
        for( currImage = 0; currImage < numImages; currImage++ )
        {
            for( currPoint = 0; currPoint < numPoints; currPoint++ )
            {
                double x,y;
                fscanf(file,"%lf %lf\n",&x,&y);

                (imagePoints+i)->x = x;
                (imagePoints+i)->y = y;
                i++;
            }
        }

        /* Read good data computed before */

        /* Focal lengths */
        double goodFcx,goodFcy;
        fscanf(file,"%lf %lf",&goodFcx,&goodFcy);

        /* Principal points */
        double goodCx,goodCy;
        fscanf(file,"%lf %lf",&goodCx,&goodCy);

        /* Read distortion */

        fscanf(file,"%lf",goodDistortion+0);
        fscanf(file,"%lf",goodDistortion+1);
        fscanf(file,"%lf",goodDistortion+2);
        fscanf(file,"%lf",goodDistortion+3);

        /* Read good Rot matrixes */
        for( currImage = 0; currImage < numImages; currImage++ )
        {
            for( i = 0; i < 3; i++ )
                for( j = 0; j < 3; j++ )
                    fscanf(file, "%lf", goodRotMatrs + currImage * 9 + j * 3 + i);
        }

        /* Read good Trans vectors */
        for( currImage = 0; currImage < numImages; currImage++ )
        {
            for( i = 0; i < 3; i++ )
                fscanf(file, "%lf", goodTransVects + currImage * 3 + i);
        }
        
        calibFlags = 
                     //CV_CALIB_FIX_PRINCIPAL_POINT +
                     //CV_CALIB_ZERO_TANGENT_DIST +
                     //CV_CALIB_FIX_ASPECT_RATIO +
                     //CV_CALIB_USE_INTRINSIC_GUESS + 
                     0;
        memset( cameraMatrix, 0, 9*sizeof(cameraMatrix[0]) );
        cameraMatrix[0] = cameraMatrix[4] = 807.;
        cameraMatrix[2] = (imageSize.width - 1)*0.5;
        cameraMatrix[5] = (imageSize.height - 1)*0.5;
        cameraMatrix[8] = 1.;

        /* Now we can calibrate camera */
        cvCalibrateCamera_64d(  numImages,
                                numbers,
                                imageSize,
                                imagePoints,
                                objectPoints,
                                distortion,
                                cameraMatrix,
                                transVects,
                                rotMatrs,
                                calibFlags );

        /* ---- Reproject points to the image ---- */
        for( currImage = 0; currImage < numImages; currImage++ )
        {
            int numPoints = etalonSize.width * etalonSize.height;
            cvProjectPointsSimple(  numPoints,
                                    objectPoints + currImage * numPoints,
                                    rotMatrs + currImage * 9,
                                    transVects + currImage * 3,
                                    cameraMatrix,
                                    distortion,
                                    reprojectPoints + currImage * numPoints);
        }


        /* ----- Compute reprojection error ----- */
        i = 0;
        double dx,dy;
        double rx,ry;
        double meanDx,meanDy;
        double maxDx = 0.0;
        double maxDy = 0.0;

        meanDx = 0;
        meanDy = 0;
        for( currImage = 0; currImage < numImages; currImage++ )
        {
            for( currPoint = 0; currPoint < etalonSize.width * etalonSize.height; currPoint++ )
            {
                rx = reprojectPoints[i].x;
                ry = reprojectPoints[i].y;
                dx = rx - imagePoints[i].x;
                dy = ry - imagePoints[i].y;

                meanDx += dx;
                meanDy += dy;

                dx = fabs(dx);
                dy = fabs(dy);

                if( dx > maxDx )
                    maxDx = dx;
                
                if( dy > maxDy )
                    maxDy = dy;
                i++;
            }
        }

        meanDx /= numImages * etalonSize.width * etalonSize.height;
        meanDy /= numImages * etalonSize.width * etalonSize.height;

        /* ========= Compare parameters ========= */

        /* ----- Compare focal lengths ----- */
        code = compare(cameraMatrix+0,&goodFcx,1,0.01,"fx");
        if( code < 0 )
            goto _exit_;

        code = compare(cameraMatrix+4,&goodFcy,1,0.01,"fy");
        if( code < 0 )
            goto _exit_;

        /* ----- Compare principal points ----- */
        code = compare(cameraMatrix+2,&goodCx,1,0.01,"cx");
        if( code < 0 )
            goto _exit_;

        code = compare(cameraMatrix+5,&goodCy,1,0.01,"cy");
        if( code < 0 )
            goto _exit_;

        /* ----- Compare distortion ----- */
        code = compare(distortion,goodDistortion,4,0.01,"[k1,k2,p1,p2]");
        if( code < 0 )
            goto _exit_;

        /* ----- Compare rot matrixs ----- */
        code = compare(rotMatrs,goodRotMatrs, 9*numImages,0.05,"rotation matrices");
        if( code < 0 )
            goto _exit_;

        /* ----- Compare rot matrixs ----- */
        code = compare(transVects,goodTransVects, 3*numImages,0.05,"translation vectors");
        if( code < 0 )
            goto _exit_;

        if( maxDx > 1.0 )
        {
            ts->printf( CvTS::LOG,
                      "Error in reprojection maxDx=%f > 1.0\n",maxDx);
            code = CvTS::FAIL_BAD_ACCURACY; goto _exit_;
        }

        if( maxDy > 1.0 )
        {
            ts->printf( CvTS::LOG,
                      "Error in reprojection maxDy=%f > 1.0\n",maxDy);
            code = CvTS::FAIL_BAD_ACCURACY; goto _exit_;
        }

        progress = update_progress( progress, currTest, numTests, 0 );

        cvFree(&imagePoints);
        cvFree(&objectPoints);
        cvFree(&reprojectPoints);
        cvFree(&numbers);

        cvFree(&transVects);
        cvFree(&rotMatrs);
        cvFree(&goodTransVects);
        cvFree(&goodRotMatrs);

        fclose(file);
        file = 0;
    }

_exit_:

    if( file )
        fclose(file);

    if( datafile )
        fclose(datafile);

    /* Free all allocated memory */
    cvFree(&imagePoints);
    cvFree(&objectPoints);
    cvFree(&reprojectPoints);
    cvFree(&numbers);

    cvFree(&transVects);
    cvFree(&rotMatrs);
    cvFree(&goodTransVects);
    cvFree(&goodRotMatrs);

    if( code < 0 )
        ts->set_failed_test_info( code );
}

CV_CameraCalibrationTest calibrate_test;
