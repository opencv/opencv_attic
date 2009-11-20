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

using namespace std;
using namespace cv;

class CV_GrabcutTest : public CvTest
{
public:
    CV_GrabcutTest();
    ~CV_GrabcutTest();    
protected:    
    void run(int);    
};

CV_GrabcutTest::CV_GrabcutTest(): CvTest( "segmentation-grabcut", "cv::grabCut" )
{
    support_testing_modes = CvTS::CORRECTNESS_CHECK_MODE;
}
CV_GrabcutTest::~CV_GrabcutTest() {}

void CV_GrabcutTest::run( int /* start_from */)
{       
    Mat img = imread(string(ts->get_data_path()) + "shared/airplane.jpg");    
    Mat exp_mask1 = imread(string(ts->get_data_path()) + "grabcut/exp_mask1.png", 0);
    Mat exp_mask2 = imread(string(ts->get_data_path()) + "grabcut/exp_mask2.png", 0);
    string xml = string(ts->get_data_path()) + "grabcut/results.xml";
    
    Rect rect(Point(24, 126), Point(483, 294));
    Mat exp_bgdModel, exp_fgdModel;

    Mat mask;
    Mat bgdModel, fgdModel;
    grabCut( img, mask, rect, bgdModel, fgdModel, 0, GC_INIT_WITH_RECT );    
    grabCut( img, mask, rect, bgdModel, fgdModel, 2, GC_EVAL );

    if ( 0 != norm(mask, exp_mask1, NORM_L2))
    {
        ts->set_failed_test_info(CvTS::FAIL_MISMATCH);
        return;
    }
        
    bgdModel.release();
    fgdModel.release();
    grabCut( img, mask, rect, bgdModel, fgdModel, 0, GC_INIT_WITH_MASK );
    grabCut( img, mask, rect, bgdModel, fgdModel, 1, GC_EVAL );

    if ( 0 != norm(mask, exp_mask2, NORM_L2))
    {
        ts->set_failed_test_info(CvTS::FAIL_MISMATCH);
        return;
    }                    
    ts->set_failed_test_info(CvTS::OK);
}

CV_GrabcutTest grabcut_test;
