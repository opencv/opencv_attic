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

using namespace cv;

class CV_WatershedTest : public CvTest
{
public:
    CV_WatershedTest();
    ~CV_WatershedTest();    
protected:    
    void run(int);
};

CV_WatershedTest::CV_WatershedTest(): CvTest( "segmentation-watershed", "cvWatershed" )
{
    support_testing_modes = CvTS::CORRECTNESS_CHECK_MODE;
}
CV_WatershedTest::~CV_WatershedTest() {}

void CV_WatershedTest::run( int start_from )
{      
    Mat orig = imread(string(ts->get_data_path()) + "inpaint/orig.jpg");    
    Mat exp = imread(string(ts->get_data_path()) + "watershed/wshed_exp.png", 0); 
    FileStorage fs(string(ts->get_data_path()) + "watershed/comp.xml", FileStorage::READ);
            
    if (orig.empty() || exp.empty() || orig.size() != exp.size() || !fs.isOpened())
    {
        ts->set_failed_test_info( CvTS::FAIL_INVALID_TEST_DATA );  
        return;
    }
               
    CvSeq* cnts = (CvSeq*)fs["contours"].readObj();

    Mat markers(orig.size(), CV_32SC1);
    IplImage iplmrks = markers;    
    int compNum = 0;
    for( ; cnts != 0; cnts = cnts->h_next, compNum++ )
        cvDrawContours( &iplmrks, cnts, Scalar::all(compNum + 1), Scalar::all(compNum + 1), -1, CV_FILLED);        
    fs.release();

    watershed(orig, markers);
 
    Mat markers8U;
    markers.convertTo(markers8U, CV_8U, 1, 1);
    
    if (0 != norm(markers8U, exp, NORM_INF))
    {    
        ts->set_failed_test_info( CvTS::FAIL_MISMATCH );  
        return;
    }          

    ts->set_failed_test_info(CvTS::OK);
}

CV_WatershedTest watershed_test;
