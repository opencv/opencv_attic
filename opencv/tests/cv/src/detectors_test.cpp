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
#include "cvaux.h"

using namespace cv;
using namespace std;

class CV_DetectorsTest : public CvTest
{
public:
    CV_DetectorsTest();
    ~CV_DetectorsTest();    
protected:    
    void run(int);    
};

CV_DetectorsTest::CV_DetectorsTest(): CvTest( "feature-detectors", "?" )
{
    support_testing_modes = CvTS::CORRECTNESS_CHECK_MODE;
}
CV_DetectorsTest::~CV_DetectorsTest() {}

struct OutOfMask
{
    const Mat& msk;
    OutOfMask(const Mat& mask) : msk(mask) {};
    OutOfMask& operator=(const OutOfMask&);
    bool operator()(const KeyPoint& kp) const { return msk.at<uchar>(kp.pt) == 0; }
};

struct WrapPoint
{
    const double* R;
    WrapPoint(const Mat& rmat) : R(rmat.ptr<double>()) { };
    
    KeyPoint operator()(const KeyPoint& kp) const 
    {                
        KeyPoint res = kp;
        res.pt.x = static_cast<float>(kp.pt.x * R[0] + kp.pt.y * R[1] + R[2]);
        res.pt.y = static_cast<float>(kp.pt.x * R[3] + kp.pt.y * R[4] + R[5]);                
        return res;                             
    }
};

void CV_DetectorsTest::run( int /*start_from*/ )
{	       
    Mat oimg = imread(string(ts->get_data_path()) + "shared/baboon.jpg", 0);
    //Mat oimg = imread(string(ts->get_data_path()) + "shared/fruits.jpg", 0);

    if (oimg.empty())
    {
        ts->set_failed_test_info( CvTS::FAIL_INVALID_TEST_DATA );
        return;
    }    

    Point center(oimg.cols/2, oimg.rows/2);
              
    Mat aff = getRotationMatrix2D(center, 45, 1);

    Mat rimg;  
    warpAffine( oimg, rimg, aff, oimg.size());

    Mat mask(oimg.size(), CV_8U, Scalar(0));    
    circle(mask, center, std::min(center.x, center.y) - 10, Scalar(255), CV_FILLED);
    Mat inv_mask;
    mask.convertTo(inv_mask, CV_8U, -1, 255);

    Mat oimg_color, rimg_color;
    cvtColor(oimg, oimg_color, CV_GRAY2BGR);   oimg_color.setTo(Scalar(0), inv_mask);
    cvtColor(rimg, rimg_color, CV_GRAY2BGR);   rimg_color.setTo(Scalar(0), inv_mask);   

    SURF surf(1536, 2);
    StarDetector star(45, 30, 10, 8, 5);

    vector<KeyPoint> opoints_surf, rpoints_surf, opoints_star, rpoints_star;
    surf(oimg, mask, opoints_surf);
    surf(rimg, mask, rpoints_surf);

    star(oimg, opoints_star);
    star(rimg, rpoints_star);
    
    opoints_star.erase(
        remove_if(opoints_star.begin(), opoints_star.end(), OutOfMask(mask)),   
        opoints_star.end());

    rpoints_star.erase(
        remove_if(rpoints_star.begin(), rpoints_star.end(), OutOfMask(mask)), 
        rpoints_star.end());

    vector<KeyPoint> exp_rpoints_surf(opoints_surf.size()), exp_rpoints_star(opoints_star.size());
    transform(opoints_surf.begin(), opoints_surf.end(), exp_rpoints_surf.begin(), WrapPoint(aff));
    transform(opoints_star.begin(), opoints_star.end(), exp_rpoints_star.begin(), WrapPoint(aff));
    
    for(size_t i = 0; i < opoints_surf.size(); ++i)
    {
        circle(oimg_color, opoints_surf[i].pt, (int)opoints_surf[i].size/2, CV_RGB(0, 255, 0));
        circle(rimg_color, exp_rpoints_surf[i].pt, (int)exp_rpoints_surf[i].size/2, CV_RGB(255, 0, 0));
    }

    for(size_t i = 0; i < rpoints_surf.size(); ++i)
        circle(rimg_color, rpoints_surf[i].pt, (int)rpoints_surf[i].size/2, CV_RGB(0, 255, 0));

    for(size_t i = 0; i < opoints_star.size(); ++i)
    {
        circle(oimg_color, opoints_star[i].pt, (int)opoints_star[i].size/2, CV_RGB(0, 0, 255));
        circle(rimg_color, exp_rpoints_surf[i].pt, (int)exp_rpoints_surf[i].size/2, CV_RGB(255, 0, 0));
    }

    for(size_t i = 0; i < rpoints_star.size(); ++i)
        circle(rimg_color, rpoints_star[i].pt, (int)rpoints_star[i].size/2, CV_RGB(0, 0, 255));
    
  /*  namedWindow("R"); imshow("R", rimg_color); 
    namedWindow("O"); imshow("O", oimg_color); 
    waitKey();*/

    ts->set_failed_test_info( CvTS::FAIL_GENERIC );		
}


CV_DetectorsTest Detectors_test;



