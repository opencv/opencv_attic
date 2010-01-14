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
#include "cvchessboardgenerator.h"

#include <iostream>

using namespace cv;
using namespace std;

namespace 
{

struct C_Caller
{
    CvMat* objPts;
    CvMat* imgPts;
    CvMat* npoints; 
    Size imageSize;
    CvMat *cameraMatrix;
    CvMat *distCoeffs;
    CvMat *rvecs;
    CvMat *tvecs;    
    int flags;

    void operator()() const  
    {         
        cvCalibrateCamera2(objPts, imgPts, npoints, imageSize, 
            cameraMatrix, distCoeffs, rvecs, tvecs, flags );
    }
};

}

class CV_CameraCalibrationBadArgTest : public CvBadArgTest
{
public:
    CV_CameraCalibrationBadArgTest() : CvBadArgTest("calibrate-camera-c-badarg", "cvCalibrateCamera2"), imgSize(800, 600)        
    { 
        support_testing_modes = CvTS::CORRECTNESS_CHECK_MODE;         
        
        Mat_<float> camMat(3, 3);
        Mat_<float> distCoeffs(1, 5);

        camMat << 300.f, 0.f, imgSize.width/2.f, 0, 300.f, imgSize.height/2.f, 0.f, 0.f, 1.f;    
        distCoeffs << 1.2f, 0.2f, 0.f, 0.f, 0.f;

        ChessBoardGenerator cbg(Size(8,6));
        corSize = cbg.cornersSize();
        vector<Point2f> exp_corn;    
        chessBoard = cbg(Mat(imgSize, CV_8U, Scalar(0)), camMat, distCoeffs, exp_corn);
        Mat_<Point2f>(corSize.height, corSize.width, (Point2f*)&exp_corn[0]).copyTo(corners);        
    };
    ~CV_CameraCalibrationBadArgTest() {} ;
protected:    
    void run(int);  
    void run_func(void) {};

    const static int M = 1;

    Size imgSize;
    Size corSize;
    Mat chessBoard;        
    Mat corners;
};


void CV_CameraCalibrationBadArgTest::run( int /* start_from */ )
{   
    CvMat objPts, imgPts, npoints, cameraMatrix, distCoeffs, rvecs, tvecs;
    Mat zeros(1, sizeof(CvMat), CV_8U, Scalar(0));

    C_Caller caller, bad_caller;
    caller.imageSize = imgSize;    
    caller.objPts = &objPts;
    caller.imgPts = &imgPts;
    caller.npoints = &npoints;    
    caller.cameraMatrix = &cameraMatrix;
    caller.distCoeffs = &distCoeffs;
    caller.rvecs = &rvecs;
    caller.tvecs = &tvecs;  

    /////////////////////////////    
    Mat objPts_cpp;
    Mat imgPts_cpp;
    Mat npoints_cpp;    
    Mat cameraMatrix_cpp;
    Mat distCoeffs_cpp;
    Mat rvecs_cpp;
    Mat tvecs_cpp; 
        
    objPts_cpp.create(corSize, CV_32FC3);
    for(int j = 0; j < corSize.height; ++j)
        for(int i = 0; i < corSize.width; ++i)
            objPts_cpp.at<Point3f>(j, i) = Point3i(i, j, 0);
    objPts_cpp = objPts_cpp.reshape(3, 1);

    imgPts_cpp = corners.clone().reshape(2, 1);
    npoints_cpp = Mat_<int>(M, 1, corSize.width * corSize.height);
    cameraMatrix_cpp.create(3, 3, CV_32F);
    distCoeffs_cpp.create(5, 1, CV_32F);
    rvecs_cpp.create(M, 1, CV_32FC3);
    tvecs_cpp.create(M, 1, CV_32FC3);
    
    caller.flags = 0;
    //CV_CALIB_USE_INTRINSIC_GUESS;    //CV_CALIB_FIX_ASPECT_RATIO
    //CV_CALIB_USE_INTRINSIC_GUESS    //CV_CALIB_FIX_ASPECT_RATIO
    //CV_CALIB_FIX_PRINCIPAL_POINT    //CV_CALIB_ZERO_TANGENT_DIST
    //CV_CALIB_FIX_FOCAL_LENGTH    //CV_CALIB_FIX_K1    //CV_CALIB_FIX_K2    //CV_CALIB_FIX_K3
     
    objPts = objPts_cpp;
    imgPts = imgPts_cpp;
    npoints = npoints_cpp;    
    cameraMatrix = cameraMatrix_cpp;
    distCoeffs = distCoeffs_cpp;
    rvecs = rvecs_cpp;
    tvecs = tvecs_cpp;  

    /* /*//*/ */
    int errors = 0;
             
    bad_caller = caller;
    bad_caller.objPts = 0;
    errors += run_test_case( CV_StsBadArg, "Zero passed in objPts", bad_caller);        

    bad_caller = caller;
    bad_caller.imgPts = 0;
    errors += run_test_case( CV_StsBadArg, "Zero passed in imgPts", bad_caller );        

    bad_caller = caller;
    bad_caller.npoints = 0;
    errors += run_test_case( CV_StsBadArg, "Zero passed in npoints", bad_caller );        

    bad_caller = caller;
    bad_caller.cameraMatrix = 0;
    errors += run_test_case( CV_StsBadArg, "Zero passed in cameraMatrix", bad_caller );

    bad_caller = caller;
    bad_caller.distCoeffs = 0;
    errors += run_test_case( CV_StsBadArg, "Zero passed in distCoeffs", bad_caller );

    bad_caller = caller;
    bad_caller.imageSize.width = -1;
    errors += run_test_case( CV_StsOutOfRange, "Bad image width", bad_caller );

    bad_caller = caller;
    bad_caller.imageSize.height = -1;
    errors += run_test_case( CV_StsOutOfRange, "Bad image height", bad_caller );

    Mat bad_nts_cpp1 = Mat_<float>(M, 1, 1.f);
    Mat bad_nts_cpp2 = Mat_<int>(3, 3, corSize.width * corSize.height); 
    CvMat bad_npts_c1 = bad_nts_cpp1;
    CvMat bad_npts_c2 = bad_nts_cpp2;
    
    bad_caller = caller;    
    bad_caller.npoints = &bad_npts_c1;
    errors += run_test_case( CV_StsUnsupportedFormat, "Bad npoints format", bad_caller );

    bad_caller = caller;    
    bad_caller.npoints = &bad_npts_c2;
    errors += run_test_case( CV_StsUnsupportedFormat, "Bad npoints size", bad_caller );
    
    bad_caller = caller;    
    bad_caller.rvecs = (CvMat*)zeros.ptr();
    errors += run_test_case( CV_StsBadArg, "Bad rvecs header", bad_caller );

    bad_caller = caller;    
    bad_caller.tvecs = (CvMat*)zeros.ptr();
    errors += run_test_case( CV_StsBadArg, "Bad tvecs header", bad_caller );

    Mat bad_rvecs_cpp1(M+1, 1, CV_32FC3); CvMat bad_rvecs_c1 = bad_rvecs_cpp1;
    Mat bad_tvecs_cpp1(M+1, 1, CV_32FC3); CvMat bad_tvecs_c1 = bad_tvecs_cpp1;   

   

    Mat bad_rvecs_cpp2(M, 2, CV_32FC3); CvMat bad_rvecs_c2 = bad_rvecs_cpp2;
    Mat bad_tvecs_cpp2(M, 2, CV_32FC3); CvMat bad_tvecs_c2 = bad_tvecs_cpp2;   

    bad_caller = caller;    
    bad_caller.rvecs = &bad_rvecs_c1;
    errors += run_test_case( CV_StsBadArg, "Bad tvecs header", bad_caller );

    bad_caller = caller;    
    bad_caller.rvecs = &bad_rvecs_c2;
    errors += run_test_case( CV_StsBadArg, "Bad tvecs header", bad_caller );

    bad_caller = caller;    
    bad_caller.tvecs = &bad_tvecs_c1;
    errors += run_test_case( CV_StsBadArg, "Bad tvecs header", bad_caller );

    bad_caller = caller;    
    bad_caller.tvecs = &bad_tvecs_c2;
    errors += run_test_case( CV_StsBadArg, "Bad tvecs header", bad_caller );
    
    Mat bad_cameraMatrix_cpp1(3, 3, CV_32S); CvMat bad_cameraMatrix_c1 = bad_cameraMatrix_cpp1;
    Mat bad_cameraMatrix_cpp2(2, 3, CV_32F); CvMat bad_cameraMatrix_c2 = bad_cameraMatrix_cpp2;
    Mat bad_cameraMatrix_cpp3(3, 2, CV_64F); CvMat bad_cameraMatrix_c3 = bad_cameraMatrix_cpp3;

   

    bad_caller = caller;    
    bad_caller.cameraMatrix = &bad_cameraMatrix_c1;
    errors += run_test_case( CV_StsBadArg, "Bad camearaMatrix header", bad_caller );

    bad_caller = caller;    
    bad_caller.cameraMatrix = &bad_cameraMatrix_c2;
    errors += run_test_case( CV_StsBadArg, "Bad camearaMatrix header", bad_caller );

    bad_caller = caller;    
    bad_caller.cameraMatrix = &bad_cameraMatrix_c3;
    errors += run_test_case( CV_StsBadArg, "Bad camearaMatrix header", bad_caller );

    Mat bad_distCoeffs_cpp1(1, 5, CV_32S); CvMat bad_distCoeffs_c1 = bad_distCoeffs_cpp1;
    Mat bad_distCoeffs_cpp2(2, 2, CV_64F); CvMat bad_distCoeffs_c2 = bad_distCoeffs_cpp2;
    Mat bad_distCoeffs_cpp3(1, 6, CV_64F); CvMat bad_distCoeffs_c3 = bad_distCoeffs_cpp3;

   

    bad_caller = caller;    
    bad_caller.distCoeffs = &bad_distCoeffs_c1;
    errors += run_test_case( CV_StsBadArg, "Bad distCoeffs header", bad_caller );

    bad_caller = caller;    
    bad_caller.distCoeffs = &bad_distCoeffs_c2;
    errors += run_test_case( CV_StsBadArg, "Bad distCoeffs header", bad_caller );

   
    bad_caller = caller;    
    bad_caller.distCoeffs = &bad_distCoeffs_c3;
    errors += run_test_case( CV_StsBadArg, "Bad distCoeffs header", bad_caller );       

    double CM[] = {0, 0, 0, /**/0, 0, 0, /**/0, 0, 0};
    Mat bad_cameraMatrix_cpp4(3, 3, CV_64F, CM); CvMat bad_cameraMatrix_c4 = bad_cameraMatrix_cpp4;      

    bad_caller = caller;    
    bad_caller.flags |= CV_CALIB_USE_INTRINSIC_GUESS;
    bad_caller.cameraMatrix = &bad_cameraMatrix_c4;
    CM[0] = 0; //bad fx
    errors += run_test_case( CV_StsOutOfRange, "Bad camearaMatrix data", bad_caller );       

    CM[0] = 500; CM[4] = 0;  //bad fy
    errors += run_test_case( CV_StsOutOfRange, "Bad camearaMatrix data", bad_caller );       

    CM[0] = 500; CM[4] = 500; CM[2] = -1; //bad cx
    errors += run_test_case( CV_StsOutOfRange, "Bad camearaMatrix data", bad_caller );       

    CM[0] = 500; CM[4] = 500; CM[2] = imgSize.width*2; //bad cx
    errors += run_test_case( CV_StsOutOfRange, "Bad camearaMatrix data", bad_caller );       

    CM[0] = 500; CM[4] = 500; CM[2] = imgSize.width/2;  CM[5] = -1; //bad cy
    errors += run_test_case( CV_StsOutOfRange, "Bad camearaMatrix data", bad_caller );       

    CM[0] = 500; CM[4] = 500; CM[2] = imgSize.width/2;  CM[5] = imgSize.height*2; //bad cy
    errors += run_test_case( CV_StsOutOfRange, "Bad camearaMatrix data", bad_caller );       

    CM[0] = 500; CM[4] = 500; CM[2] = imgSize.width/2; CM[5] = imgSize.height/2;
    CM[1] = 0.1; //Non-zero skew 
    errors += run_test_case( CV_StsOutOfRange, "Bad camearaMatrix data", bad_caller );

    CM[1] = 0;
    CM[3] = 0.1; /* mad matrix shape */ 
    errors += run_test_case( CV_StsOutOfRange, "Bad camearaMatrix data", bad_caller );

    CM[3] = 0; CM[6] = 0.1; /* mad matrix shape */ 
    errors += run_test_case( CV_StsOutOfRange, "Bad camearaMatrix data", bad_caller );

    CM[3] = 0; CM[6] = 0; CM[7] = 0.1; /* mad matrix shape */ 
    errors += run_test_case( CV_StsOutOfRange, "Bad camearaMatrix data", bad_caller );

    CM[3] = 0; CM[6] = 0; CM[7] = 0; CM[8] = 1.1; /* mad matrix shape */ 
    errors += run_test_case( CV_StsOutOfRange, "Bad camearaMatrix data", bad_caller );      
    CM[8] = 1.0;

    /////////////////////////////////////////////////////////////////////////////////////
    bad_caller = caller;        
    Mat bad_objPts_cpp5 = objPts_cpp.clone(); CvMat bad_objPts_c5 = bad_objPts_cpp5;
    bad_caller.objPts = &bad_objPts_c5;

    cv::RNG& rng = theRNG();
    for(int i = 0; i < bad_objPts_cpp5.rows; ++i)
        bad_objPts_cpp5.at<Point3f>(0, i).z += ((float)rng - 0.5f);

    errors += run_test_case( CV_StsBadArg, "Bad objPts data", bad_caller );      



        /*if( flags & CV_CALIB_FIX_ASPECT_RATIO )
        {
            aspectRatio = cvmGet(cameraMatrix,0,0);
            aspectRatio /= cvmGet(cameraMatrix,1,1);
            if( aspectRatio < 0.01 || aspectRatio > 100 )
                CV_Error( CV_StsOutOfRange,
                    "The specified aspect ratio (=A[0][0]/A[1][1]) is incorrect" );
        }
        cvInitIntrinsicParams2D( _M, _m, npoints, imageSize, &_A, aspectRatio );*/            

    if (errors)
        ts->set_failed_test_info(CvTS::FAIL_MISMATCH);
    else
        ts->set_failed_test_info(CvTS::OK);   

    try { caller(); }
    catch (...)
    {  
        ts->set_failed_test_info(CvTS::FAIL_MISMATCH);        
        printf("+!");
    }
}

CV_CameraCalibrationBadArgTest camera_calibration_bad_arg_test;