/*M///////////////////////////////////////////////////////////////////////////////////////
//
//  IMPORTANT: READ BEFORE DOWNLOADING, COPYING, INSTALLING OR USING.
//
//  By downloading, copying, installing or using the software you agree to this license.
//  If you do not agree to this license, do not download, install,
//  copy or use the software.
//
//
//                           License Agreement
//                For Open Source Computer Vision Library
//
// Copyright (C) 2000-2008, Intel Corporation, all rights reserved.
// Copyright (C) 2009, Willow Garage Inc., all rights reserved.
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
//   * The name of the copyright holders may not be used to endorse or promote products
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
#include <string>
#include <iostream>
#include <fstream>
#include <iterator>
#include <limits>
#include <numeric>
#include "cvaux.h"

using namespace cv;
using namespace std;

class CV_RigidTransform_Test : public CvTest
{
public:
    CV_RigidTransform_Test();
    ~CV_RigidTransform_Test();    
protected:
    void run(int);    

    bool testNPoints();
    bool testImage();    
};

CV_RigidTransform_Test::CV_RigidTransform_Test(): CvTest( "estimateRigidTransform", "?" )
{
    support_testing_modes = CvTS::CORRECTNESS_CHECK_MODE;
}
CV_RigidTransform_Test::~CV_RigidTransform_Test() {}

struct WrapAff2D
{
    const double *F;
    WrapAff2D(const Mat& aff) : F(aff.ptr<double>()) {}
    Point2f operator()(const Point2f& p)
    {
        return Point2d( p.x * F[0] + p.y * F[1] +  F[2],
                        p.x * F[3] + p.y * F[4] +  F[5]);      
    }
};

bool CV_RigidTransform_Test::testNPoints()
{   
    Mat aff(2, 3, CV_64F);
    cv::randu(aff, Scalar(-2), Scalar(2));          

    const int n = 3;

    Mat fpts(1, n, CV_32FC2);
    Mat tpts(1, n, CV_32FC2);

    cv::randu(fpts, Scalar(0,0), Scalar(10,10));                                     
    transform(fpts.ptr<Point2f>(), fpts.ptr<Point2f>() + n, tpts.ptr<Point2f>(), WrapAff2D(aff));

    Mat aff_est = estimateRigidTransform(fpts, tpts, true);
            
    const double thres = 1e-3;
    if (norm(aff_est, aff, NORM_INF) > thres)
    {        
        cout << norm(aff_est, aff, NORM_INF) << endl;
        ts->set_failed_test_info(CvTS::FAIL_MISMATCH);
        return false;
    }    
    return true;
}

bool CV_RigidTransform_Test::testImage()
{
    Mat img;
    pyrDown(imread( string(ts->get_data_path()) + "shared/graffiti.png", 1), img);
         
    Mat aff = cv::getRotationMatrix2D(Point(img.cols/2, img.rows/2), 1, 0.99);
    aff.ptr<double>()[2]+=3;
    aff.ptr<double>()[5]+=3;
        
    Mat rotated;
    warpAffine(img, rotated, aff, img.size());
     
    Mat aff_est = estimateRigidTransform(img, rotated, true);
    
    const double thres = 0.03;
    if (norm(aff_est, aff, NORM_INF) > thres)
    {                
        ts->set_failed_test_info(CvTS::FAIL_MISMATCH);
        return false;
    }    

    return true;
}

void CV_RigidTransform_Test::run( int /* start_from */)
{	
    DefaultRngAuto dra; (void)dra;

    if (!testNPoints())
        return;

    if (!testImage())
        return;

    ts->set_failed_test_info(CvTS::OK);
}

CV_RigidTransform_Test CV_RigidTransform_test;

