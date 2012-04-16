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


#if TS_IMGPROC

#ifdef HAVE_OPENCL

#if OK

///////////////////////////////////////////////////////////////////////////////////////////////////////
// histograms

struct CalcHist : testing::Test
{
    cv::Size size;
    cv::Mat src;
    cv::Mat hist_gold;
    
    virtual void SetUp()
    {
        cv::RNG& rng = cvtest::TS::ptr()->get_rng();

        size = cv::Size(2000, 2000);
        
        src = cvtest::randomMat(rng, size, CV_8UC1, 0, 255, false);

        hist_gold.create(1, 256, CV_32SC1);
        hist_gold.setTo(cv::Scalar::all(0));

        int* hist = hist_gold.ptr<int>();
        for (int y = 0; y < src.rows; ++y)
        {
            const uchar* src_row = src.ptr(y);

            for (int x = 0; x < src.cols; ++x)
                ++hist[src_row[x]];
        }
    }
};

TEST_F(CalcHist, Accuracy)
{
    PRINT_PARAM(size);

    cv::Mat hist;
    
    ASSERT_NO_THROW(
        cv::ocl::oclMat gpuHist;

        cv::ocl::calcHist(cv::ocl::oclMat(src), gpuHist);

        gpuHist.download(hist);
    );

    EXPECT_MAT_NEAR(hist_gold, hist, 0.0);
}


struct EqualizeHist : testing::Test
{
    cv::Size size;
    cv::Mat src;
    cv::Mat dst_gold;
    
    virtual void SetUp()
    {
        cv::RNG& rng = cvtest::TS::ptr()->get_rng();

        size = cv::Size(2000, 2000);
        
        src = cvtest::randomMat(rng, size, CV_8UC1, 0, 255, false);

        cv::equalizeHist(src, dst_gold);
    }
};

TEST_F(EqualizeHist, Accuracy)
{
    
    PRINT_PARAM(size);

    cv::Mat dst;
    
    ASSERT_NO_THROW(
        cv::ocl::oclMat gpuDst;

        cv::ocl::equalizeHist(cv::ocl::oclMat(src), gpuDst);

        gpuDst.download(dst);
    );

    EXPECT_MAT_NEAR(dst_gold, dst, 3.0);
}

#endif // OK

#if CLERR


///////////////////////////////////////////////////////////////////////////////////////////////////////
// resize

struct Resize : testing::TestWithParam< std::tr1::tuple<int, int> >
{
    
    int type;
    int interpolation;

    cv::Size size;
    cv::Mat src;

    cv::Mat dst_gold1;
    cv::Mat dst_gold2;
    
    virtual void SetUp()
    {
        
        type = std::tr1::get<0>(GetParam());
        interpolation = std::tr1::get<1>(GetParam());

        cv::RNG& rng = cvtest::TS::ptr()->get_rng();

        size = cv::Size(rng.uniform(20, 150), rng.uniform(20, 150));

        src = cvtest::randomMat(rng, size, type, 0.0, 127.0, false);

        cv::resize(src, dst_gold1, cv::Size(), 2.0, 2.0, interpolation);
        cv::resize(src, dst_gold2, cv::Size(), 0.5, 0.5, interpolation);
    }
};

TEST_P(Resize, Accuracy)
{
    static const char* interpolations[] = {"INTER_NEAREST", "INTER_LINEAR"};
    const char* interpolationStr = interpolations[interpolation];


    PRINT_TYPE(type);
    PRINT_PARAM(size);
    PRINT_PARAM(interpolationStr);

    cv::Mat dst1;
    cv::Mat dst2;

    ASSERT_NO_THROW(
        cv::ocl::oclMat dev_src(src);
        cv::ocl::oclMat gpuRes1;
        cv::ocl::oclMat gpuRes2;

        cv::ocl::resize(dev_src, gpuRes1, cv::Size(), 2.0, 2.0, interpolation);
        cv::ocl::resize(dev_src, gpuRes2, cv::Size(), 0.5, 0.5, interpolation);

        gpuRes1.download(dst1);
        gpuRes2.download(dst2);
    );

    EXPECT_MAT_SIMILAR(dst_gold1, dst1, 0.5);
    EXPECT_MAT_SIMILAR(dst_gold2, dst2, 0.5);
}

INSTANTIATE_TEST_CASE_P(ImgProc, Resize, testing::Combine(
                        testing::Values(CV_8UC1, CV_8UC4, CV_32FC1, CV_32FC4), 
                        testing::Values((int)cv::INTER_NEAREST, (int)cv::INTER_LINEAR)));



#endif // CLERR

#if W_OUT

///////////////////////////////////////////////////////////////////////////////////////////////////////
// cvtColor

struct CvtColor : testing::TestWithParam< std::tr1::tuple<int, int> >
{
    
    int type;
    int cvtcode;

    cv::Mat img;
    
    virtual void SetUp()
    {
        type    = std::tr1::get<0>(GetParam());
        cvtcode = std::tr1::get<1>(GetParam());

        cv::Mat imgBase = readImage("stereobm/aloe-L.png");
        ASSERT_FALSE(imgBase.empty());

        imgBase.convertTo(img, type, type == CV_32F ? 1.0 / 255.0 : 1.0);
    }
};

TEST_P(CvtColor, Accuracy)
{
    
    PRINT_TYPE(type);

    cv::Mat src = img;
    cv::Mat dst_gold;
    cv::cvtColor(src, dst_gold, cvtcode);

    cv::Mat dst;

    ASSERT_NO_THROW(
        cv::ocl::oclMat gpuRes;

        cv::ocl::cvtColor(cv::ocl::oclMat(src), gpuRes, cvtcode);

        gpuRes.download(dst);
    );

    EXPECT_MAT_NEAR(dst_gold, dst, 1e-5);
}


INSTANTIATE_TEST_CASE_P(ImgProc, CvtColor, 
						testing::Combine(
                        testing::Values(CV_8U, CV_16U),
                        testing::Values( CV_BGR2GRAY, CV_BGRA2GRAY, CV_RGB2GRAY, CV_RGBA2GRAY )));


///////////////////////////////////////////////////////////////////////////////////////////////////////
// copyMakeBorder

struct CopyMakeBorder : testing::TestWithParam< std::tr1::tuple<int, int> >
{
    
    int type;

    cv::Size size;
    cv::Mat src;
    int top;
    int botton;
    int left;
    int right;
	int bordertype;
    cv::Scalar val;

    cv::Mat dst_gold;
    
    virtual void SetUp()
    {
        type       = std::tr1::get<0>(GetParam());
        bordertype = std::tr1::get<1>(GetParam());

        cv::RNG& rng = cvtest::TS::ptr()->get_rng();

        size = cv::Size(rng.uniform(20, 150), rng.uniform(20, 150));

        src = cvtest::randomMat(rng, size, type, 0.0, 127.0, false);
        
        top = rng.uniform(1, 10);
        botton = rng.uniform(1, 10);
        left = rng.uniform(1, 10);
        right = rng.uniform(1, 10);
        val = cv::Scalar(rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255));

        cv::copyMakeBorder(src, dst_gold, top, botton, left, right, bordertype, val);
    }
};

TEST_P(CopyMakeBorder, Accuracy)
{

    PRINT_TYPE(type);
    PRINT_PARAM(size);
    PRINT_PARAM(top);
    PRINT_PARAM(botton);
    PRINT_PARAM(left);
    PRINT_PARAM(right);
    PRINT_PARAM(val);

    cv::Mat dst;

    ASSERT_NO_THROW(
        cv::ocl::oclMat gpuRes;

        cv::ocl::copyMakeBorder(cv::ocl::oclMat(src), gpuRes, top, botton, left, right, bordertype, val);
		
        gpuRes.download(dst);
    );
    EXPECT_MAT_NEAR(dst_gold, dst, 0.0);
}

INSTANTIATE_TEST_CASE_P(ImgProc, CopyMakeBorder, testing::Combine(
                        testing::Values(CV_8UC1, CV_8UC4, CV_32SC1),
						testing::Values(cv::BORDER_CONSTANT, cv::BORDER_REPLICATE, cv::BORDER_REFLECT_101)));


///////////////////////////////////////////////////////////////////////////////////////////////////////
// warpAffine & warpPerspective

static const int warpFlags[] = {cv::INTER_NEAREST, cv::INTER_LINEAR, cv::INTER_CUBIC, cv::INTER_NEAREST | cv::WARP_INVERSE_MAP, cv::INTER_LINEAR | cv::WARP_INVERSE_MAP, cv::INTER_CUBIC | cv::WARP_INVERSE_MAP};
static const char* warpFlags_str[] = {"INTER_NEAREST", "INTER_LINEAR", "INTER_CUBIC", "INTER_NEAREST | WARP_INVERSE_MAP", "INTER_LINEAR | WARP_INVERSE_MAP", "INTER_CUBIC | WARP_INVERSE_MAP"};

struct WarpAffine : testing::TestWithParam< std::tr1::tuple<int, int> >
{
    
    int type;
    int flagIdx;

    cv::Size size;
    cv::Mat src;
    cv::Mat M;

    cv::Mat dst_gold;
    
    virtual void SetUp()
    {
        
        type = std::tr1::get<0>(GetParam());
        flagIdx = std::tr1::get<1>(GetParam());

        cv::RNG& rng = cvtest::TS::ptr()->get_rng();

        size = cv::Size(rng.uniform(20, 150), rng.uniform(20, 150));

        src = cvtest::randomMat(rng, size, type, 0.0, 127.0, false);

        static double reflect[2][3] = { {-1,  0, 0},
                                        { 0, -1, 0}};
        reflect[0][2] = size.width;
        reflect[1][2] = size.height;
        M = cv::Mat(2, 3, CV_64F, (void*)reflect); 

        cv::warpAffine(src, dst_gold, M, src.size(), flagIdx);       
    }
};

TEST_P(WarpAffine, Accuracy)
{
    const char* warpFlagStr = warpFlags_str[flagIdx];

    PRINT_TYPE(type);
    PRINT_PARAM(size);
    PRINT_PARAM(warpFlagStr);

    cv::Mat dst;

    ASSERT_NO_THROW(
        cv::ocl::oclMat gpuRes;

        cv::ocl::warpAffine(cv::ocl::oclMat(src), gpuRes, M, src.size(), flagIdx);

        gpuRes.download(dst);
    );

    // Check inner parts (ignoring 1 pixel width border)
    cv::Mat dst_gold_roi = dst_gold.rowRange(1, dst_gold.rows - 1).colRange(1, dst_gold.cols - 1);
    cv::Mat dst_roi = dst.rowRange(1, dst.rows - 1).colRange(1, dst.cols - 1);

    EXPECT_MAT_NEAR(dst_gold_roi, dst_roi, 1e-3);
}

struct WarpPerspective : testing::TestWithParam< std::tr1::tuple<int, int> >
{
    
    int type;
    int flagIdx;

    cv::Size size;
    cv::Mat src;
    cv::Mat M;

    cv::Mat dst_gold;
    
    virtual void SetUp()
    {
        
        type = std::tr1::get<0>(GetParam());
        flagIdx = std::tr1::get<1>(GetParam());


        cv::RNG& rng = cvtest::TS::ptr()->get_rng();

        size = cv::Size(rng.uniform(20, 150), rng.uniform(20, 150));

        src = cvtest::randomMat(rng, size, type, 0.0, 127.0, false);

        static double reflect[3][3] = { { -1, 0, 0},
                                        { 0, -1, 0},
                                        { 0,  0, 1}};
        reflect[0][2] = size.width;
        reflect[1][2] = size.height;
        M = cv::Mat(3, 3, CV_64F, (void*)reflect);

        cv::warpPerspective(src, dst_gold, M, src.size(), flagIdx);       
    }
};

TEST_P(WarpPerspective, Accuracy)
{
    const char* warpFlagStr = warpFlags_str[flagIdx];


    PRINT_TYPE(type);
    PRINT_PARAM(size);
    PRINT_PARAM(warpFlagStr);

    cv::Mat dst;

    ASSERT_NO_THROW(
        cv::ocl::oclMat gpuRes;

        cv::ocl::warpPerspective(cv::ocl::oclMat(src), gpuRes, M, src.size(), flagIdx);

        gpuRes.download(dst);
    );

    // Check inner parts (ignoring 1 pixel width border)
    cv::Mat dst_gold_roi = dst_gold.rowRange(1, dst_gold.rows - 1).colRange(1, dst_gold.cols - 1);
    cv::Mat dst_roi = dst.rowRange(1, dst.rows - 1).colRange(1, dst.cols - 1);

    EXPECT_MAT_NEAR(dst_gold_roi, dst_roi, 1e-3);
}

INSTANTIATE_TEST_CASE_P(ImgProc, WarpAffine, testing::Combine(
                        testing::Values(CV_8UC1, CV_8UC3, CV_8UC4, CV_32FC1, CV_32FC3, CV_32FC4),
						testing::Values((int)cv::INTER_NEAREST, (int)cv::INTER_LINEAR, (int)cv::INTER_CUBIC)));

INSTANTIATE_TEST_CASE_P(ImgProc, WarpPerspective, testing::Combine(
                        testing::Values(CV_8UC1, CV_8UC3, CV_8UC4, CV_32FC1, CV_32FC3, CV_32FC4),
                        testing::Values((int)cv::INTER_NEAREST, (int)cv::INTER_LINEAR, (int)cv::INTER_CUBIC)));

///////////////////////////////////////////////////////////////////////////////////////////////////////
// integral

struct Integral : testing::Test
{
    cv::Size size;
    cv::Mat src;

    cv::Mat dst_gold_sum, dst_gold_sqsum;
    
    virtual void SetUp()
    {
        cv::RNG& rng = cvtest::TS::ptr()->get_rng();

        size = cv::Size(rng.uniform(20, 150), rng.uniform(20, 150));

        src = cvtest::randomMat(rng, size, CV_8UC1, 0.0, 255.0, false); 

        cv::integral(src, dst_gold_sum, dst_gold_sqsum, 0);     
    }
};

TEST_F(Integral, Accuracy)
{
    
    PRINT_PARAM(size);

    cv::Mat dst, dstSq;

	//ASSERT_NO_THROW(
        cv::ocl::oclMat gpuRes, gpuResSq;
        cv::ocl::integral(cv::ocl::oclMat(src), gpuRes, gpuResSq);
        
		gpuRes.download(dst);
		gpuResSq.download(dstSq);
	//);

    EXPECT_MAT_NEAR(dst_gold_sum, dst, 0.0);
	EXPECT_MAT_NEAR(dst_gold_sqsum, dstSq, 0.0);
}



////////////////////////////////////////////////////////////////////////
// Norm

static const int normTypes[] = {cv::NORM_INF, cv::NORM_L1, cv::NORM_L2};
static const char* normTypes_str[] = {"NORM_INF", "NORM_L1", "NORM_L2"};

struct Norm : testing::TestWithParam< std::tr1::tuple<int, int> >
{
    
    int type;
    int normTypeIdx;

    cv::Size size;
    cv::Mat src;

    double gold;

    virtual void SetUp()
    {
        
        type = std::tr1::get<0>(GetParam());
        normTypeIdx = std::tr1::get<1>(GetParam());

        cv::RNG& rng = cvtest::TS::ptr()->get_rng();

        size = cv::Size(rng.uniform(100, 400), rng.uniform(100, 400));

        src = cvtest::randomMat(rng, size, type, 0.0, 10.0, false);

        gold = cv::norm(src, normTypeIdx);
    }
};

TEST_P(Norm, Accuracy)
{
    const char* normTypeStr = normTypes_str[normTypeIdx];

    
    PRINT_TYPE(type);
    PRINT_PARAM(size);
    PRINT_PARAM(normTypeStr);

    double res;

    ASSERT_NO_THROW(
        res = cv::ocl::norm(cv::ocl::oclMat(src), normTypeIdx);
    );

    ASSERT_NEAR(res, gold, 0.5);
}

INSTANTIATE_TEST_CASE_P(ImgProc, Norm, testing::Combine(
                        testing::Values( CV_8UC1 ),
						testing::Values( (int)cv::NORM_INF, (int)cv::NORM_L1, (int)cv::NORM_L2 )));


#endif // W_OUT


#if    K_VFF 
///////////////////////////////////////////////////////////////////////////////////////////////////////
// threshold

struct Threshold : testing::TestWithParam< std::tr1::tuple<int, int> >
{
    int type;
    int threshOp;

    cv::Size size;
    cv::Mat src;
    double maxVal;
    double thresh;

    cv::Mat dst_gold;
    
    virtual void SetUp()
    {
        type = std::tr1::get<0>(GetParam());
        threshOp = std::tr1::get<1>(GetParam());

        cv::RNG& rng = cvtest::TS::ptr()->get_rng();

        size = cv::Size(rng.uniform(20, 150), rng.uniform(20, 150));

        src = cvtest::randomMat(rng, size, type, 0.0, 127.0, false);

        maxVal = rng.uniform(20.0, 127.0);
        thresh = rng.uniform(0.0, maxVal);

        cv::threshold(src, dst_gold, thresh, maxVal, threshOp);
    }
};

TEST_P(Threshold, Accuracy)
{
	if( type == CV_8U && threshOp == (int)cv::THRESH_BINARY )
		return;

    static const char* ops[] = {"THRESH_BINARY", "THRESH_BINARY_INV", "THRESH_TRUNC", "THRESH_TOZERO", "THRESH_TOZERO_INV"};
    const char* threshOpStr = ops[threshOp];

    PRINT_TYPE(type);
    PRINT_PARAM(size);
    PRINT_PARAM(threshOpStr);
    PRINT_PARAM(maxVal);
    PRINT_PARAM(thresh);

    cv::Mat dst;

    ASSERT_NO_THROW(
        cv::ocl::oclMat gpuRes;

        cv::ocl::threshold(cv::ocl::oclMat(src), gpuRes, thresh, maxVal, threshOp);

        gpuRes.download(dst);
    );

    EXPECT_MAT_NEAR(dst_gold, dst, 0.0);
}

INSTANTIATE_TEST_CASE_P(ImgProc, Threshold, testing::Combine(
                        testing::Values(CV_8U, CV_32F), 
                        testing::Values((int)cv::THRESH_BINARY, (int)cv::THRESH_BINARY_INV, (int)cv::THRESH_TRUNC, (int)cv::THRESH_TOZERO, (int)cv::THRESH_TOZERO_INV)));

//////////////////////////////////////////////////////////////////////////////
// meanShift

struct MeanShift : testing::Test
{
    cv::Mat rgba;

    int spatialRad;
    int colorRad;

    virtual void SetUp()
    {

        cv::Mat img = readImage("meanshift/cones.png");
        ASSERT_FALSE(img.empty());
        
        cv::cvtColor(img, rgba, CV_BGR2BGRA);

        spatialRad = 30;
        colorRad = 30;
    }
};

TEST_F(MeanShift, Filtering)
{
    cv::Mat img_template;
    
    img_template = readImage("meanshift/con_result.png");

    ASSERT_FALSE(img_template.empty());

    cv::Mat dst;

    ASSERT_NO_THROW(
        cv::ocl::oclMat dev_dst;
        cv::ocl::meanShiftFiltering(cv::ocl::oclMat(rgba), dev_dst, spatialRad, colorRad);
        dev_dst.download(dst);
    );

    ASSERT_EQ(CV_8UC4, dst.type());

    cv::Mat result;
    cv::cvtColor(dst, result, CV_BGRA2BGR);

    EXPECT_MAT_NEAR(img_template, result, 0.0);
}

TEST_F(MeanShift, Proc)
{
    cv::Mat spmap_template;
    cv::FileStorage fs;

    fs.open(std::string(cvtest::TS::ptr()->get_data_path()) + "meanshift/spmap.yaml", cv::FileStorage::READ);

    ASSERT_TRUE(fs.isOpened());

    fs["spmap"] >> spmap_template;

    ASSERT_TRUE(!rgba.empty() && !spmap_template.empty());

    cv::Mat rmap_filtered;
    cv::Mat rmap;
    cv::Mat spmap;

    ASSERT_NO_THROW(
        cv::ocl::oclMat d_rmap_filtered;
        cv::ocl::meanShiftFiltering(cv::ocl::oclMat(rgba), d_rmap_filtered, spatialRad, colorRad);

        cv::ocl::oclMat d_rmap;
        cv::ocl::oclMat d_spmap;
        cv::ocl::meanShiftProc(cv::ocl::oclMat(rgba), d_rmap, d_spmap, spatialRad, colorRad);

        d_rmap_filtered.download(rmap_filtered);
        d_rmap.download(rmap);
        d_spmap.download(spmap);
    );

    ASSERT_EQ(CV_8UC4, rmap.type());
    
    EXPECT_MAT_NEAR(rmap_filtered, rmap, 0.0);    
    EXPECT_MAT_NEAR(spmap_template, spmap, 0.0);
}


struct MeanShiftSegmentation : testing::TestWithParam< int >
{
    
    int minsize;
    
    cv::Mat rgba;

    cv::Mat dst_gold;

    virtual void SetUp()
    {
        
        minsize = GetParam();

        cv::Mat img = readImage("meanshift/cones.png");
        ASSERT_FALSE(img.empty());
        
        cv::cvtColor(img, rgba, CV_BGR2BGRA);

        std::ostringstream path;
        path << "meanshift/cones_segmented_sp10_sr10_minsize" << minsize;
        path << ".png";

        dst_gold = readImage(path.str());
        ASSERT_FALSE(dst_gold.empty());
    }
};

TEST_P(MeanShiftSegmentation, Regression)
{
    
    PRINT_PARAM(minsize);

    cv::Mat dst;

    ASSERT_NO_THROW(
        cv::ocl::meanShiftSegmentation(cv::ocl::oclMat(rgba), dst, 10, 10, minsize);
    );

    cv::Mat dst_rgb;
    cv::cvtColor(dst, dst_rgb, CV_BGRA2BGR);

    EXPECT_MAT_SIMILAR(dst_gold, dst_rgb, 1e-3);
}

INSTANTIATE_TEST_CASE_P(ImgProc, MeanShiftSegmentation, 
                        testing::Values(0, 4, 20, 84, 340, 1364));

#endif // K_VFF


#endif // HAVE_OPENCL
#endif // TS_IMGPROC
