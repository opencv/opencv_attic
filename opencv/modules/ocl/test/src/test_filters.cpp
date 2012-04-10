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

#if TS_FILTERS
#ifdef HAVE_OPENCL

namespace
{
    double checkNorm(const cv::Mat& m1, const cv::Mat& m2, const cv::Size& ksize)
    {
        cv::Rect roi(ksize.width, ksize.height, m1.cols - 2 * ksize.width, m1.rows - 2 * ksize.height);
        cv::Mat m1ROI = m1(roi);
        cv::Mat m2ROI = m2(roi);
        return ::checkNorm(m1ROI, m2ROI);
    }

    double checkNorm(const cv::Mat& m1, const cv::Mat& m2, int ksize)
    {
        return checkNorm(m1, m2, cv::Size(ksize, ksize));
    }
}

#define EXPECT_MAT_NEAR_KSIZE(mat1, mat2, ksize, eps) \
    { \
        ASSERT_EQ(mat1.type(), mat2.type()); \
        ASSERT_EQ(mat1.size(), mat2.size()); \
        EXPECT_LE(checkNorm(mat1, mat2, ksize), eps); \
    }

#if OK

/////////////////////////////////////////////////////////////////////////////////////////////////
// blur

struct Blur : testing::TestWithParam< std::tr1::tuple<int, int>  >
{
    
    cv::Size ksize;
    
    cv::Mat img_rgba;
    cv::Mat img_gray;

    cv::Mat dst_gold_rgba;
    cv::Mat dst_gold_gray;

	int bordertype;
    
    virtual void SetUp()
    {
		bordertype = (int)cv::BORDER_DEFAULT;
        ksize = cv::Size(std::tr1::get<0>(GetParam()), std::tr1::get<1>(GetParam()));    
        cv::Mat img = readImage("stereobp/aloe-L.png");
        ASSERT_FALSE(img.empty());
        
        cv::cvtColor(img, img_rgba, CV_BGR2BGRA);
        cv::cvtColor(img, img_gray, CV_BGR2GRAY);

        cv::blur(img_rgba, dst_gold_rgba, ksize, cv::Point(-1,-1), bordertype);
        cv::blur(img_gray, dst_gold_gray, ksize, cv::Point(-1,-1), bordertype);
    }
};

TEST_P(Blur, Accuracy)
{
    
    PRINT_PARAM(ksize);

    cv::Mat dst_rgba;
    cv::Mat dst_gray;

    ASSERT_NO_THROW(
        cv::ocl::oclMat dev_dst_rgba;
        cv::ocl::oclMat dev_dst_gray;

		cv::ocl::blur(cv::ocl::oclMat(img_rgba), dev_dst_rgba, ksize, cv::Point(-1,-1), bordertype);
        cv::ocl::blur(cv::ocl::oclMat(img_gray), dev_dst_gray, ksize, cv::Point(-1,-1), bordertype);

        dev_dst_rgba.download(dst_rgba);
        dev_dst_gray.download(dst_gray);
    );

    EXPECT_MAT_NEAR_KSIZE(dst_gold_rgba, dst_rgba, ksize, 1.0);
    EXPECT_MAT_NEAR_KSIZE(dst_gold_gray, dst_gray, ksize, 1.0);
}

INSTANTIATE_TEST_CASE_P(Filter, Blur, testing::Combine(
                        testing::Values(3, 5, 7), 
                        testing::Values(3, 5, 7)));

/////////////////////////////////////////////////////////////////////////////////////////////////
// sobel

struct Sobel : testing::TestWithParam< std::tr1::tuple<int, std::pair<int, int>> >
{
    
    int ksize;
    int dx, dy;
    
    cv::Mat img_rgba;
    cv::Mat img_gray;

    cv::Mat dst_gold_rgba;
    cv::Mat dst_gold_gray;
    
	int bordertype;
    virtual void SetUp()
    {
		bordertype = (int)cv::BORDER_DEFAULT;
        ksize = std::tr1::get<0>(GetParam());
        std::pair<int, int> d = std::tr1::get<1>(GetParam());
        dx = d.first; dy = d.second;

        cv::Mat img = readImage("stereobp/aloe-L.png");
        ASSERT_FALSE(img.empty());
        
        cv::cvtColor(img, img_rgba, CV_BGR2BGRA);
        cv::cvtColor(img, img_gray, CV_BGR2GRAY);
        
        cv::Sobel(img_rgba, dst_gold_rgba, -1, dx, dy, ksize, 1, 0, bordertype);
        cv::Sobel(img_gray, dst_gold_gray, -1, dx, dy, ksize, 1, 0, bordertype);
    }
};

TEST_P(Sobel, Accuracy)
{
    
    PRINT_PARAM(ksize);
    PRINT_PARAM(dx);
    PRINT_PARAM(dy);

    cv::Mat dst_rgba;
    cv::Mat dst_gray;

    ASSERT_NO_THROW(
        cv::ocl::oclMat dev_dst_rgba;
        cv::ocl::oclMat dev_dst_gray;

        cv::ocl::Sobel(cv::ocl::oclMat(img_rgba), dev_dst_rgba, -1, dx, dy, ksize, 1, 0, bordertype);
        cv::ocl::Sobel(cv::ocl::oclMat(img_gray), dev_dst_gray, -1, dx, dy, ksize, 1, 0, bordertype);

        dev_dst_rgba.download(dst_rgba);
        dev_dst_gray.download(dst_gray);
    );

    EXPECT_MAT_NEAR_KSIZE(dst_gold_rgba, dst_rgba, ksize, 0.0);
    EXPECT_MAT_NEAR_KSIZE(dst_gold_gray, dst_gray, ksize, 0.0);
}

INSTANTIATE_TEST_CASE_P(Filter, Sobel, testing::Combine( 
                        testing::Values(3, 5, 7), 
                        testing::Values(std::make_pair(1, 0), std::make_pair(0, 1), std::make_pair(1, 1), std::make_pair(2, 0), std::make_pair(2, 1), std::make_pair(0, 2), std::make_pair(1, 2), std::make_pair(2, 2))));


/////////////////////////////////////////////////////////////////////////////////////////////////
// scharr

struct Scharr : testing::TestWithParam< std::pair<int, int> >
{
    
    int dx, dy;
    
    cv::Mat img_rgba;
    cv::Mat img_gray;

    cv::Mat dst_gold_rgba;
    cv::Mat dst_gold_gray;
    
	int bordertype;
    virtual void SetUp()
    {
		bordertype = (int)cv::BORDER_DEFAULT;
        std::pair<int, int> d = GetParam();
        dx = d.first; dy = d.second;

        cv::Mat img = readImage("stereobp/aloe-L.png");
        ASSERT_FALSE(img.empty());
        
        cv::cvtColor(img, img_rgba, CV_BGR2BGRA);
        cv::cvtColor(img, img_gray, CV_BGR2GRAY);

        cv::Scharr(img_rgba, dst_gold_rgba, -1, dx, dy, 1, 0, bordertype);
        cv::Scharr(img_gray, dst_gold_gray, -1, dx, dy, 1, 0, bordertype);
    }
};

TEST_P(Scharr, Accuracy)
{
    
    PRINT_PARAM(dx);
    PRINT_PARAM(dy);

    cv::Mat dst_rgba;
    cv::Mat dst_gray;

    ASSERT_NO_THROW(
        cv::ocl::oclMat dev_dst_rgba;
        cv::ocl::oclMat dev_dst_gray;

        cv::ocl::Scharr(cv::ocl::oclMat(img_rgba), dev_dst_rgba, -1, dx, dy, 1, 0, bordertype);
        cv::ocl::Scharr(cv::ocl::oclMat(img_gray), dev_dst_gray, -1, dx, dy, 1, 0, bordertype);

        dev_dst_rgba.download(dst_rgba);
        dev_dst_gray.download(dst_gray);
    );

    EXPECT_MAT_NEAR_KSIZE(dst_gold_rgba, dst_rgba, 3, 0.0);
    EXPECT_MAT_NEAR_KSIZE(dst_gold_gray, dst_gray, 3, 0.0);
}

INSTANTIATE_TEST_CASE_P(Filter, Scharr, 
                        testing::Values(std::make_pair(1, 0), std::make_pair(0, 1)),
						);


/////////////////////////////////////////////////////////////////////////////////////////////////
// gaussianBlur

struct GaussianBlur : testing::TestWithParam< std::tr1::tuple<int, int> >
{
    
    cv::Size ksize;
    
    cv::Mat img_rgba;
    cv::Mat img_gray;

    double sigma1, sigma2;

    cv::Mat dst_gold_rgba;
    cv::Mat dst_gold_gray;
    
    virtual void SetUp()
    {
        ksize = cv::Size(std::tr1::get<0>(GetParam()), std::tr1::get<1>(GetParam()));
        
        cv::Mat img = readImage("stereobp/aloe-L.png");
        ASSERT_FALSE(img.empty());
        
        cv::cvtColor(img, img_rgba, CV_BGR2BGRA);
        cv::cvtColor(img, img_gray, CV_BGR2GRAY);
        
        cv::RNG& rng = cvtest::TS::ptr()->get_rng();

        sigma1 = rng.uniform(0.1, 1.0); 
        sigma2 = rng.uniform(0.1, 1.0);
        
        cv::GaussianBlur(img_rgba, dst_gold_rgba, ksize, sigma1, sigma2);
        cv::GaussianBlur(img_gray, dst_gold_gray, ksize, sigma1, sigma2);
    }
};

TEST_P(GaussianBlur, Accuracy)
{
    
    PRINT_PARAM(ksize);
    PRINT_PARAM(sigma1);
    PRINT_PARAM(sigma2);

    cv::Mat dst_rgba;
    cv::Mat dst_gray;

    ASSERT_NO_THROW(
        cv::ocl::oclMat dev_dst_rgba;
        cv::ocl::oclMat dev_dst_gray;

        cv::ocl::GaussianBlur(cv::ocl::oclMat(img_rgba), dev_dst_rgba, ksize, sigma1, sigma2);
        cv::ocl::GaussianBlur(cv::ocl::oclMat(img_gray), dev_dst_gray, ksize, sigma1, sigma2);

        dev_dst_rgba.download(dst_rgba);
        dev_dst_gray.download(dst_gray);
    );

    EXPECT_MAT_NEAR_KSIZE(dst_gold_rgba, dst_rgba, ksize, 3.0);
    EXPECT_MAT_NEAR_KSIZE(dst_gold_gray, dst_gray, ksize, 3.0);
}

INSTANTIATE_TEST_CASE_P(Filter, GaussianBlur, testing::Combine(
                        testing::Values(3, 5, 7), 
                        testing::Values(3, 5, 7)));

/////////////////////////////////////////////////////////////////////////////////////////////////
// erode

struct Erode : testing::Test
{
    
    
    cv::Mat img_rgba;
    cv::Mat img_gray;

    cv::Mat kernel;

    cv::Mat dst_gold_rgba;
    cv::Mat dst_gold_gray;
    
    virtual void SetUp()
    {
        kernel = cv::Mat::ones(3, 3, CV_8U);
        
        cv::Mat img = readImage("stereobp/aloe-L.png");
        ASSERT_FALSE(img.empty());
        
        cv::cvtColor(img, img_rgba, CV_BGR2BGRA);
        cv::cvtColor(img, img_gray, CV_BGR2GRAY);

        cv::erode(img_rgba, dst_gold_rgba, kernel);
        cv::erode(img_gray, dst_gold_gray, kernel);
    }
};

TEST_F(Erode, Accuracy)
{
    cv::Mat dst_rgba;
    cv::Mat dst_gray;

    ASSERT_NO_THROW(
        cv::ocl::oclMat dev_dst_rgba;
        cv::ocl::oclMat dev_dst_gray;

        cv::ocl::erode(cv::ocl::oclMat(img_rgba), dev_dst_rgba, kernel);
        cv::ocl::erode(cv::ocl::oclMat(img_gray), dev_dst_gray, kernel);

        dev_dst_rgba.download(dst_rgba);
        dev_dst_gray.download(dst_gray);
    );

    EXPECT_MAT_NEAR_KSIZE(dst_gold_rgba, dst_rgba, 3, 0.0);
    EXPECT_MAT_NEAR_KSIZE(dst_gold_gray, dst_gray, 3, 0.0);
}


/////////////////////////////////////////////////////////////////////////////////////////////////
// Laplacian

struct Laplacian : testing::TestWithParam< int >
{
    
    int ksize;
    
    cv::Mat img_rgba;
    cv::Mat img_gray;

    cv::Mat dst_gold_rgba;
    cv::Mat dst_gold_gray;

	int bordertype;
    
    virtual void SetUp()
    {
		bordertype = (int)cv::BORDER_DEFAULT;
        ksize = GetParam();    
        cv::Mat img = readImage("stereobp/aloe-L.png");
        ASSERT_FALSE(img.empty());
        
        cv::cvtColor(img, img_rgba, CV_BGR2BGRA);
        cv::cvtColor(img, img_gray, CV_BGR2GRAY);

        cv::Laplacian(img_rgba, dst_gold_rgba, -1, ksize );
        cv::Laplacian(img_gray, dst_gold_gray, -1, ksize );
    }
};

TEST_P(Laplacian, Accuracy)
{
    
    PRINT_PARAM(ksize);

    cv::Mat dst_rgba;
    cv::Mat dst_gray;

    ASSERT_NO_THROW(
        cv::ocl::oclMat dev_dst_rgba;
        cv::ocl::oclMat dev_dst_gray;

		cv::ocl::Laplacian(cv::ocl::oclMat(img_rgba), dev_dst_rgba, -1, ksize);
        cv::ocl::Laplacian(cv::ocl::oclMat(img_gray), dev_dst_gray, -1, ksize);

        dev_dst_rgba.download(dst_rgba);
        dev_dst_gray.download(dst_gray);
    );

    EXPECT_MAT_NEAR_KSIZE(dst_gold_rgba, dst_rgba, ksize, 1.0);
    EXPECT_MAT_NEAR_KSIZE(dst_gold_gray, dst_gray, ksize, 1.0);
}

INSTANTIATE_TEST_CASE_P(Filter, Laplacian, 
                        testing::Values(1, 3));

#endif // OK


#if CLERR
/////////////////////////////////////////////////////////////////////////////////////////////////
// laplacian

struct Laplacian : testing::TestWithParam< int >
{
    
    int ksize;

    cv::Mat img_rgba;
    cv::Mat img_gray;

    cv::Mat dst_gold_rgba;
    cv::Mat dst_gold_gray;
    
    virtual void SetUp()
    {
		ksize = GetParam();

        cv::Mat img = readImage("stereobp/aloe-L.png");
        ASSERT_FALSE(img.empty());
        
        cv::cvtColor(img, img_rgba, CV_BGR2BGRA);
        cv::cvtColor(img, img_gray, CV_BGR2GRAY);

        cv::Laplacian(img_rgba, dst_gold_rgba, -1, ksize);
        cv::Laplacian(img_gray, dst_gold_gray, -1, ksize);
    }
};

TEST_P(Laplacian, Accuracy)
{
    
    PRINT_PARAM(ksize);

    cv::Mat dst_rgba;
    cv::Mat dst_gray;

    ASSERT_NO_THROW(
        cv::ocl::oclMat dev_dst_rgba;
        cv::ocl::oclMat dev_dst_gray;

        cv::ocl::Laplacian(cv::ocl::oclMat(img_rgba), dev_dst_rgba, -1, ksize);
        cv::ocl::Laplacian(cv::ocl::oclMat(img_gray), dev_dst_gray, -1, ksize);

        dev_dst_rgba.download(dst_rgba);
        dev_dst_gray.download(dst_gray);
    );

    EXPECT_MAT_NEAR_KSIZE(dst_gold_rgba, dst_rgba, 3, 0.0);
    EXPECT_MAT_NEAR_KSIZE(dst_gold_gray, dst_gray, 3, 0.0);
}

INSTANTIATE_TEST_CASE_P(Filter, Laplacian, 
                        testing::Values(1, 3));

/////////////////////////////////////////////////////////////////////////////////////////////////
// bilateralFilter

struct BilateralFilter : testing::TestWithParam< int >
{
    
    int dsize;

    cv::Mat img;

    cv::Mat dst_gold;
    
    virtual void SetUp()
    {
		dsize = GetParam();

        img = readImage("stereobp/aloe-L.png");
        ASSERT_FALSE(img.empty());
       
		cv::bilateralFilter( img, dst_gold, dsize, 20, 20 );
    }
};

TEST_P(BilateralFilter, Accuracy)
{
    
    PRINT_PARAM(dsize);

    cv::Mat dst;
    cv::ocl::oclMat input( img );
	cv::ocl::oclMat output;
    ASSERT_NO_THROW(
		cv::ocl::bilateralFilter( input, output, dsize, 20, 100, 0 );
        output.download(dst);
    );

    EXPECT_MAT_NEAR_KSIZE(dst_gold, dst, 3, 0.0);
}

INSTANTIATE_TEST_CASE_P(Filter, BilateralFilter, 
                        testing::Values(3, 4, 5, 6, 7));



#endif // CLERR


#if W_OUT



/////////////////////////////////////////////////////////////////////////////////////////////////
// dilate

struct Dilate : testing::Test
{

    cv::Mat img_rgba;
    cv::Mat img_gray;

    cv::Mat kernel;

    cv::Mat dst_gold_rgba;
    cv::Mat dst_gold_gray;
    
    virtual void SetUp()
    {
        kernel = cv::Mat::ones(3, 3, CV_8U);
        
        cv::Mat img = readImage("stereobp/aloe-L.png");
        ASSERT_FALSE(img.empty());
        
        cv::cvtColor(img, img_rgba, CV_BGR2BGRA);
        cv::cvtColor(img, img_gray, CV_BGR2GRAY);

        cv::dilate(img_rgba, dst_gold_rgba, kernel);
        cv::dilate(img_gray, dst_gold_gray, kernel);
    }
};

TEST_F(Dilate, Accuracy)
{
    cv::Mat dst_rgba;
    cv::Mat dst_gray;

    ASSERT_NO_THROW(
        cv::ocl::oclMat dev_dst_rgba;
        cv::ocl::oclMat dev_dst_gray;

        cv::ocl::dilate(cv::ocl::oclMat(img_rgba), dev_dst_rgba, kernel);
        cv::ocl::dilate(cv::ocl::oclMat(img_gray), dev_dst_gray, kernel);

        dev_dst_rgba.download(dst_rgba);
        dev_dst_gray.download(dst_gray);
    );

    EXPECT_MAT_NEAR_KSIZE(dst_gold_rgba, dst_rgba, 3, 0.0);
    EXPECT_MAT_NEAR_KSIZE(dst_gold_gray, dst_gray, 3, 0.0);
}

/////////////////////////////////////////////////////////////////////////////////////////////////
// morphEx

static const int morphOps[] = {cv::MORPH_OPEN, CV_MOP_CLOSE, CV_MOP_GRADIENT, CV_MOP_TOPHAT, CV_MOP_BLACKHAT};
static const char* morphOps_str[] = {"MORPH_OPEN", "MOP_CLOSE", "MOP_GRADIENT", "MOP_TOPHAT", "MOP_BLACKHAT"};

struct MorphEx : testing::TestWithParam< int >
{
    
    int morphOpsIdx;
    
    cv::Mat img_rgba;
    cv::Mat img_gray;

    cv::Mat kernel;

    cv::Mat dst_gold_rgba;
    cv::Mat dst_gold_gray;
    
    virtual void SetUp()
    {
        morphOpsIdx = GetParam();

        cv::Mat img = readImage("stereobp/aloe-L.png");
        ASSERT_FALSE(img.empty());
        
        cv::cvtColor(img, img_rgba, CV_BGR2BGRA);
        cv::cvtColor(img, img_gray, CV_BGR2GRAY);

        kernel = cv::Mat::ones(3, 3, CV_8U);
        
        cv::morphologyEx(img_rgba, dst_gold_rgba, morphOps[morphOpsIdx], kernel);
        cv::morphologyEx(img_gray, dst_gold_gray, morphOps[morphOpsIdx], kernel);
    }
};

TEST_P(MorphEx, Accuracy)
{
    const char* morphOpStr = morphOps_str[morphOpsIdx];

    PRINT_PARAM(morphOpStr);

    cv::Mat dst_rgba;
    cv::Mat dst_gray;

    ASSERT_NO_THROW(
        cv::ocl::oclMat dev_dst_rgba;
        cv::ocl::oclMat dev_dst_gray;

        cv::ocl::morphologyEx(cv::ocl::oclMat(img_rgba), dev_dst_rgba, morphOps[morphOpsIdx], cv::ocl::oclMat(kernel));
        cv::ocl::morphologyEx(cv::ocl::oclMat(img_gray), dev_dst_gray, morphOps[morphOpsIdx], cv::ocl::oclMat(kernel));

        dev_dst_rgba.download(dst_rgba);
        dev_dst_gray.download(dst_gray);
    );

    EXPECT_MAT_NEAR_KSIZE(dst_gold_rgba, dst_rgba, 4, 0.0);
    EXPECT_MAT_NEAR_KSIZE(dst_gold_gray, dst_gray, 4, 0.0);
}

INSTANTIATE_TEST_CASE_P(Filter, MorphEx,
                        testing::Range(0, 5));


#endif // W_OUT




#endif // HAVE_OPENCL
#endif // TS_FILTERS

