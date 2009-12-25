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

using namespace cv;

// --------------------------------- CV_CameraCalibrationTest --------------------------------------------

class CV_CameraCalibrationTest : public CvTest
{
public:
    CV_CameraCalibrationTest( const char* testName, const char* testFuncs );
    ~CV_CameraCalibrationTest();
    void clear();
protected:
    int compare(double* val, double* refVal, int len,
                double eps, const char* paramName);
	virtual void calibrate( int imageCount, int* pointCounts,
		CvSize imageSize, CvPoint2D64f* imagePoints, CvPoint3D64f* objectPoints,
		double* distortionCoeffs, double* cameraMatrix, double* translationVectors,
		double* rotationMatrices, int flags ) = 0;
	virtual void project( int pointCount, CvPoint3D64f* objectPoints,
		double* rotationMatrix, double*  translationVector,
		double* cameraMatrix, double* distortion, CvPoint2D64f* imagePoints ) = 0;

    void run(int);
};

CV_CameraCalibrationTest::CV_CameraCalibrationTest( const char* testName, const char* testFuncs ):
    CvTest( testName, testFuncs )
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
    char            filepath[200];
    char            filename[200];
    
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
        progress = update_progress( progress, currTest, numTests, 0 );
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
        calibrate(  numImages,
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
            project(  numPoints,
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

// --------------------------------- CV_CameraCalibrationTest_C --------------------------------------------

class CV_CameraCalibrationTest_C : public CV_CameraCalibrationTest
{
public:
	CV_CameraCalibrationTest_C() : CV_CameraCalibrationTest( "calibrate-camera-c", "cvCalibrateCamera2" ) {}
protected:
	virtual void calibrate( int imageCount, int* pointCounts,
		CvSize imageSize, CvPoint2D64f* imagePoints, CvPoint3D64f* objectPoints,
		double* distortionCoeffs, double* cameraMatrix, double* translationVectors,
		double* rotationMatrices, int flags );
	virtual void project( int pointCount, CvPoint3D64f* objectPoints,
		double* rotationMatrix, double*  translationVector,
		double* cameraMatrix, double* distortion, CvPoint2D64f* imagePoints );
};

void CV_CameraCalibrationTest_C::calibrate( int imageCount, int* pointCounts,
		CvSize imageSize, CvPoint2D64f* imagePoints, CvPoint3D64f* objectPoints,
		double* distortionCoeffs, double* cameraMatrix, double* translationVectors,
		double* rotationMatrices, int flags )
{
	cvCalibrateCamera_64d(  imageCount,
							pointCounts,
                            imageSize,
                            imagePoints,
                            objectPoints,
                            distortionCoeffs,
                            cameraMatrix,
                            translationVectors,
                            rotationMatrices,
                            flags );
}

void CV_CameraCalibrationTest_C::project( int pointCount, CvPoint3D64f* objectPoints,
		double* rotationMatrix, double*  translationVector,
		double* cameraMatrix, double* distortion, CvPoint2D64f* imagePoints )
{
	cvProjectPointsSimple(  pointCount,
                            objectPoints,
                            rotationMatrix,
                            translationVector,
                            cameraMatrix,
                            distortion,
                            imagePoints );
}

CV_CameraCalibrationTest_C calibrate_test_c;

// --------------------------------- CV_CameraCalibrationTest_CPP --------------------------------------------

class CV_CameraCalibrationTest_CPP : public CV_CameraCalibrationTest
{
public:
	CV_CameraCalibrationTest_CPP() : CV_CameraCalibrationTest( "calibrate-camera-cpp", "cv::calibrateCamera" ) {}
protected:
	virtual void calibrate( int imageCount, int* pointCounts,
		CvSize imageSize, CvPoint2D64f* imagePoints, CvPoint3D64f* objectPoints,
		double* distortionCoeffs, double* cameraMatrix, double* translationVectors,
		double* rotationMatrices, int flags );
	virtual void project( int pointCount, CvPoint3D64f* objectPoints,
		double* rotationMatrix, double*  translationVector,
		double* cameraMatrix, double* distortion, CvPoint2D64f* imagePoints );
};

void CV_CameraCalibrationTest_CPP::calibrate( int imageCount, int* pointCounts,
		CvSize _imageSize, CvPoint2D64f* _imagePoints, CvPoint3D64f* _objectPoints,
		double* _distortionCoeffs, double* _cameraMatrix, double* translationVectors,
		double* rotationMatrices, int flags )
{
	vector<vector<Point3f> > objectPoints( imageCount );
	vector<vector<Point2f> > imagePoints( imageCount );
	Size imageSize = _imageSize;
	Mat cameraMatrix, distCoeffs;
	vector<Mat> rvecs, tvecs;

	CvPoint3D64f* op = _objectPoints;
	CvPoint2D64f* ip = _imagePoints;
	vector<vector<Point3f> >::iterator objectPointsIt = objectPoints.begin();
	vector<vector<Point2f> >::iterator imagePointsIt = imagePoints.begin();
	for( int i = 0; i < imageCount; ++objectPointsIt, ++imagePointsIt, i++ )
	{
		int num = pointCounts[i];
		objectPointsIt->resize( num );
		imagePointsIt->resize( num );
		vector<Point3f>::iterator oIt = objectPointsIt->begin();
		vector<Point2f>::iterator iIt = imagePointsIt->begin();
		for( int j = 0; j < num; ++oIt, ++iIt, j++, op++, ip++)
		{
			oIt->x = (float)op->x, oIt->y = (float)op->y, oIt->z = (float)op->z;
			iIt->x = (float)ip->x, iIt->y = (float)ip->y;
		}
	}

	calibrateCamera( objectPoints,
	                 imagePoints,
					 imageSize,
					 cameraMatrix,
					 distCoeffs,
					 rvecs,
					 tvecs,
					 flags );

	assert( cameraMatrix.type() == CV_64FC1 );
	memcpy( _cameraMatrix, cameraMatrix.data, 9*sizeof(double) );

	assert( cameraMatrix.type() == CV_64FC1 );
	memcpy( _distortionCoeffs, distCoeffs.data, 4*sizeof(double) );

	vector<Mat>::iterator rvecsIt = rvecs.begin();
	vector<Mat>::iterator tvecsIt = tvecs.begin();
	double *rm = rotationMatrices,
		   *tm = translationVectors;
	assert( rvecsIt->type() == CV_64FC1 );
	assert( tvecsIt->type() == CV_64FC1 );
	for( int i = 0; i < imageCount; ++rvecsIt, ++tvecsIt, i++, rm+=9, tm+=3 )
	{
		Mat r9( 3, 3, CV_64FC1 );
		Rodrigues( *rvecsIt, r9 );
		memcpy( rm, r9.data, 9*sizeof(double) );
		memcpy( tm, tvecsIt->data, 3*sizeof(double) );
	}
}

void CV_CameraCalibrationTest_CPP::project( int pointCount, CvPoint3D64f* _objectPoints,
		double* rotationMatrix, double*  translationVector,
		double* _cameraMatrix, double* distortion, CvPoint2D64f* _imagePoints )
{
	Mat objectPoints( pointCount, 3, CV_64FC1, _objectPoints );
	Mat rmat( 3, 3, CV_64FC1, rotationMatrix ),
		rvec( 1, 3, CV_64FC1 ),
		tvec( 1, 3, CV_64FC1, translationVector );
	Mat cameraMatrix( 3, 3, CV_64FC1, _cameraMatrix );
	Mat distCoeffs( 1, 4, CV_64FC1, distortion );
	vector<Point2f> imagePoints;
	Rodrigues( rmat, rvec );
	
	objectPoints.convertTo( objectPoints, CV_32FC1 );
	projectPoints( objectPoints, rvec, tvec,
				   cameraMatrix, distCoeffs, imagePoints );
	vector<Point2f>::const_iterator it = imagePoints.begin();
	for( int i = 0; it != imagePoints.end(); ++it, i++ )
	{
		_imagePoints[i] = cvPoint2D64f( it->x, it->y );
	}
}

//CV_CameraCalibrationTest_CPP calibrate_test_cpp;


///////////////////////////////// Stereo Calibration /////////////////////////////////////

using namespace cv;
using namespace std;

class CV_StereoCalibrationTest : public CvTest
{
public:
    CV_StereoCalibrationTest();
    ~CV_StereoCalibrationTest();
    void clear();
    //int write_default_params(CvFileStorage* fs);
    
protected:
    bool checkPandROI(int test_case_idx,
                      const Mat& M, const Mat& D, const Mat& R,
                      const Mat& P, Size imgsize, Rect roi);
    
    void run(int);
};


CV_StereoCalibrationTest::CV_StereoCalibrationTest():
CvTest( "calibrate-stereo", "cvStereoCalibrate" )
{
    support_testing_modes = CvTS::CORRECTNESS_CHECK_MODE;
}


CV_StereoCalibrationTest::~CV_StereoCalibrationTest()
{
    clear();
}


void CV_StereoCalibrationTest::clear()
{
    CvTest::clear();
}


bool CV_StereoCalibrationTest::checkPandROI(int test_case_idx, const Mat& M, const Mat& D, const Mat& R,
                                            const Mat& P, Size imgsize, Rect roi)
{
    const double eps = 0.05;
    const int N = 21;
    int x, y, k;
    vector<Point2f> pts, upts;
    
    // step 1. check that all the original points belong to the destination image 
    for( y = 0; y < N; y++ )
        for( x = 0; x < N; x++ )
            pts.push_back(Point2f((float)x*imgsize.width/(N-1), (float)y*imgsize.height/(N-1)));
    
    undistortPoints( pts, upts, M, D, R, P );
    for( k = 0; k < N*N; k++ )
        if( upts[k].x < -imgsize.width*eps || upts[k].x > imgsize.width*(1+eps) ||
           upts[k].y < -imgsize.height*eps || upts[k].y > imgsize.height*(1+eps) )
    {
        ts->printf(CvTS::LOG, "Test #%d. The point (%g, %g) was mapped to (%g, %g) which is out of image\n",
                   test_case_idx, pts[k].x, pts[k].y, upts[k].x, upts[k].y);
        return false;
    }
    
    // step 2. check that all the points inside ROI belong to the original source image
    Mat temp(imgsize, CV_8U), utemp, map1, map2;
    temp = Scalar::all(1);
    initUndistortRectifyMap(M, D, R, P, imgsize, CV_16SC2, map1, map2);
    remap(temp, utemp, map1, map2, INTER_LINEAR);
    
    if(roi.x < 0 || roi.y < 0 || roi.x + roi.width > imgsize.width || roi.y + roi.height > imgsize.height)
    {
        ts->printf(CvTS::LOG, "Test #%d. The ROI=(%d, %d, %d, %d) is outside of the imge rectangle\n",
                   test_case_idx, roi.x, roi.y, roi.width, roi.height);
        return false;
    }
    double s = sum(utemp(roi))[0];
    if( s > roi.area() || roi.area() - s > roi.area()*(1-eps) )
    {
        ts->printf(CvTS::LOG, "Test #%d. The ratio of black pixels inside the valid ROI (~%g%%) is too large\n",
                   test_case_idx, s*100./roi.area());
        return false;
    }
    
    return true;
}

void CV_StereoCalibrationTest::run( int )
{
    const int ntests = 1;
    const double maxReprojErr = 2;
    const double maxScanlineDistErr = 3;
    FILE* f = 0;
    
    for(int testcase = 1; testcase <= ntests; testcase++)
    {
        char filepath[1000];
        char buf[1000];
        sprintf( filepath, "%sstereo/case%d/stereo_calib.txt", ts->get_data_path(), testcase );
        f = fopen(filepath, "rt");
        Size patternSize;
        vector<string> imglist;
        
        if( !f || !fgets(buf, sizeof(buf)-3, f) || sscanf(buf, "%d%d", &patternSize.width, &patternSize.height) != 2 )
        {
            ts->printf( CvTS::LOG, "The file %s can not be opened or has invalid content\n", filepath );
            ts->set_failed_test_info( f ? CvTS::FAIL_INVALID_TEST_DATA : CvTS::FAIL_MISSING_TEST_DATA );
            return;
        }
        
        for(;;)
        {
            if( !fgets( buf, sizeof(buf)-3, f ))
                break;
            size_t len = strlen(buf);
            while( len > 0 && isspace(buf[len-1]))
                buf[--len] = '\0';
            if( buf[0] == '#')
                continue;
            sprintf(filepath, "%sstereo/case%d/%s", ts->get_data_path(), testcase, buf );
            imglist.push_back(string(filepath));
        }
        fclose(f);
        
        if( imglist.size() == 0 || imglist.size() % 2 != 0 )
        {
            ts->printf( CvTS::LOG, "The number of images is 0 or an odd number in the case #%d\n", testcase );
            ts->set_failed_test_info( CvTS::FAIL_INVALID_TEST_DATA );
            return;
        }
        
        size_t i, nframes = imglist.size()/2;
        int j, npoints = patternSize.width*patternSize.height;
        vector<vector<Point3f> > objpt(nframes);
        vector<vector<Point2f> > imgpt1(nframes);
        vector<vector<Point2f> > imgpt2(nframes);
        Size imgsize;
        
        for( i = 0; i < nframes; i++ )
        {
            Mat left = imread(imglist[i*2]);
            Mat right = imread(imglist[i*2+1]);
            if(!left.data || !right.data)
            {
                ts->printf( CvTS::LOG, "Can not load images %s and %s, testcase %d\n",
                           imglist[i*2].c_str(), imglist[i*2+1].c_str(), testcase );
                ts->set_failed_test_info( CvTS::FAIL_MISSING_TEST_DATA );
                return;
            }
            imgsize = left.size();
            bool found1 = findChessboardCorners(left, patternSize, imgpt1[i]); 
            bool found2 = findChessboardCorners(right, patternSize, imgpt2[i]);
            if(!found1 || !found2)
            {
                ts->printf( CvTS::LOG, "The function could not detect boards on the images %s and %s, testcase %d\n",
                           imglist[i*2].c_str(), imglist[i*2+1].c_str(), testcase );
                ts->set_failed_test_info( CvTS::FAIL_INVALID_OUTPUT );
                return;
            }
            
            for( j = 0; j < npoints; j++ )
                objpt[i].push_back(Point3f((float)(j%patternSize.width), (float)(j/patternSize.width), 0.f));
        }
        
        Mat M1 = Mat::eye(3,3,CV_64F), M2 = Mat::eye(3,3,CV_64F), D1(5,1,CV_64F), D2(5,1,CV_64F), R, T, E, F;
        M1.at<double>(0,2) = M2.at<double>(0,2)=(imgsize.width-1)*0.5;
        M1.at<double>(1,2) = M2.at<double>(1,2)=(imgsize.height-1)*0.5;
        D1 = Scalar::all(0);
        D2 = Scalar::all(0);
        double err = stereoCalibrate(objpt, imgpt1, imgpt2, M1, D1, M2, D2, imgsize, R, T, E, F,
                                     TermCriteria(TermCriteria::MAX_ITER+TermCriteria::EPS, 30, 1e-6),
                                     CV_CALIB_SAME_FOCAL_LENGTH
                                     //+ CV_CALIB_FIX_ASPECT_RATIO
                                     + CV_CALIB_FIX_PRINCIPAL_POINT
                                     + CV_CALIB_ZERO_TANGENT_DIST
                                     );
        err /= nframes*npoints;
        if( err > maxReprojErr )
        {
            ts->printf( CvTS::LOG, "The average reprojection error is too big (=%g), testcase %d\n", err, testcase);
            ts->set_failed_test_info( CvTS::FAIL_INVALID_OUTPUT );
            return;
        }
        
        Mat R1, R2, P1, P2, Q;
        Rect roi1, roi2;
        stereoRectify(M1, D1, M2, D2, imgsize, R, T, R1, R2, P1, P2, Q, 1, imgsize, &roi1, &roi2, 0);
        
        if(norm(R1.t()*R1 - Mat::eye(3,3,CV_64F)) > 0.01 ||
           norm(R2.t()*R2 - Mat::eye(3,3,CV_64F)) > 0.01 ||
           abs(determinant(F)) > 0.01)
        {
            ts->printf( CvTS::LOG, "The computed R1 and R2 are not orthogonal, or the computed F is not singular, testcase %d\n", testcase);
            ts->set_failed_test_info( CvTS::FAIL_INVALID_OUTPUT );
            return;
        }
        
        if(!checkPandROI(testcase, M1, D1, R1, P1, imgsize, roi1))
        {
            ts->set_failed_test_info( CvTS::FAIL_BAD_ACCURACY );
            return;
        }
        
        if(!checkPandROI(testcase, M2, D2, R2, P2, imgsize, roi2))
        {
            ts->set_failed_test_info( CvTS::FAIL_BAD_ACCURACY );
            return;
        }
        
        bool verticalStereo = abs(P2.at<double>(0,3)) < abs(P2.at<double>(1,3));
        double maxDiff = 0;
        
        for( i = 0; i < nframes; i++ )
        {
            vector<Point2f> temp[2];
            undistortPoints(imgpt1[i], temp[0], M1, D1, R1, P1);
            undistortPoints(imgpt2[i], temp[1], M2, D2, R2, P2);
            
            for( j = 0; j < npoints; j++ )
            {
                double diff = verticalStereo ? abs(temp[0][j].x - temp[1][j].x) : abs(temp[0][j].y - temp[1][j].y);
                maxDiff = max(maxDiff, diff);
                if( maxDiff > maxScanlineDistErr )
                {
                    ts->printf( CvTS::LOG, "The distance between %s coordinates is too big(=%g), testcase %d\n",
                               verticalStereo ? "x" : "y", diff, testcase);
                    ts->set_failed_test_info( CvTS::FAIL_BAD_ACCURACY );
                    return;
                }
            }                
        }
        ts->printf( CvTS::LOG, "Testcase %d. Max distance =%g\n", testcase, maxDiff );
    }
}

CV_StereoCalibrationTest stereocalib_test;
