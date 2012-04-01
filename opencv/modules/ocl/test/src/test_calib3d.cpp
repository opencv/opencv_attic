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

#include "test_precomp.hpp"

#if    TS_CALIB3D
#ifdef HAVE_OPENCL

#if    OK
#endif //OK


#if    K_ERR

//////////////////////////////////////////////////////////////////////////
// BlockMatching

struct StereoBlockMatching : testing::Test
{
    cv::Mat img_l;
    cv::Mat img_r;
    cv::Mat img_template;
     
    virtual void SetUp() 
    {     
        img_l = readImage("stereobm/aloe-L.png", CV_LOAD_IMAGE_GRAYSCALE);
        img_r = readImage("stereobm/aloe-R.png", CV_LOAD_IMAGE_GRAYSCALE);
        img_template = readImage("stereobm/aloe-disp.png", CV_LOAD_IMAGE_GRAYSCALE);
        
        ASSERT_FALSE(img_l.empty());
        ASSERT_FALSE(img_r.empty());
        ASSERT_FALSE(img_template.empty());
    }
};

TEST_F(StereoBlockMatching, Regression) 
{
    cv::Mat disp;

    ASSERT_NO_THROW(
        cv::ocl::oclMat dev_disp;
        cv::ocl::StereoBM_GPU bm(0, 128, 19);

        bm(cv::ocl::oclMat(img_l), cv::ocl::oclMat(img_r), dev_disp);
        
        dev_disp.download(disp);
    );

    disp.convertTo(disp, img_template.type());
    
    EXPECT_MAT_NEAR(img_template, disp, 0.0);
}

//////////////////////////////////////////////////////////////////////////
// BeliefPropagation

struct StereoBeliefPropagation : testing::Test
{
    cv::Mat img_l;
    cv::Mat img_r;
    cv::Mat img_template;
    

    virtual void SetUp() 
    {       
           
        img_l = readImage("stereobp/aloe-L.png");
        img_r = readImage("stereobp/aloe-R.png");
        img_template = readImage("stereobp/aloe-disp.png", CV_LOAD_IMAGE_GRAYSCALE);
        
        ASSERT_FALSE(img_l.empty());
        ASSERT_FALSE(img_r.empty());
        ASSERT_FALSE(img_template.empty());
    }
};

TEST_F(StereoBeliefPropagation, Regression) 
{
    cv::Mat disp;

    ASSERT_NO_THROW(
        cv::ocl::oclMat dev_disp;
        cv::ocl::StereoBeliefPropagation bpm(64, 8, 2, 25, 0.1f, 15, 1, CV_16S);

        bpm(cv::ocl::oclMat(img_l), cv::ocl::oclMat(img_r), dev_disp);
        
        dev_disp.download(disp);
    );

    disp.convertTo(disp, img_template.type());
    
    EXPECT_MAT_NEAR(img_template, disp, 0.0);
}




#endif // K_ERR

#if    W_OUT

//////////////////////////////////////////////////////////////////////////
// ConstantSpaceBP

struct StereoConstantSpaceBP : testing::Test
{
    cv::Mat img_l;
    cv::Mat img_r;
    cv::Mat img_template;
	
    virtual void SetUp() 
    {
        img_l = readImage("csstereobp/aloe-L.png"); 
        img_r = readImage("csstereobp/aloe-R.png");

        img_template = readImage("csstereobp/aloe-disp.png", CV_LOAD_IMAGE_GRAYSCALE);
            
        ASSERT_FALSE(img_l.empty());
        ASSERT_FALSE(img_r.empty());
        ASSERT_FALSE(img_template.empty());        
    }
};

TEST_F(StereoConstantSpaceBP, Regression) 
{
    cv::Mat disp;

    ASSERT_NO_THROW(
        cv::ocl::oclMat dev_disp;
        cv::ocl::StereoConstantSpaceBP bpm(128, 16, 4, 4);

        bpm(cv::ocl::oclMat(img_l), cv::ocl::oclMat(img_r), dev_disp);
        
        dev_disp.download(disp);
    );

    disp.convertTo(disp, img_template.type());
    
    EXPECT_MAT_NEAR(img_template, disp, 1.0);
}

#endif // W_OUT


#endif // HAVE_OPENCL
#endif // TS_CALIB3D
