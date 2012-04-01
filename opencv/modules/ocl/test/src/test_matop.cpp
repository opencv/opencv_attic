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
//     and/or other GpuMaterials provided with the distribution.
//
//   * The name of the copyright holders may not be used to endorse or promote products
//     derived from this software without specific prior written permission.
//
// This software is provided by the copyright holders and contributors "as is" and
// any express or bpied warranties, including, but not limited to, the bpied
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

#if    TS_MATOP

#ifdef HAVE_OPENCL

#if OK


////////////////////////////////////////////////////////////////////////////////
// split

struct Split : testing::TestWithParam< int >
{
    int type;

    cv::Size size;
    cv::Mat src;

    std::vector<cv::Mat> dst_gold;

    virtual void SetUp() 
    {
        type = GetParam();

        cv::RNG& rng = cvtest::TS::ptr()->get_rng();

        size = cv::Size(rng.uniform(20, 150), rng.uniform(20, 150));

        src.create(size, type);
        src.setTo(cv::Scalar(1.0, 2.0, 3.0, 4.0));
        cv::split(src, dst_gold);
    }
};

TEST_P(Split, Accuracy)
{
    if (CV_MAT_DEPTH(type) == CV_64F )
        return;

    PRINT_TYPE(type);
    PRINT_PARAM(size);

    std::vector<cv::Mat> dst;
    
    ASSERT_NO_THROW(
        std::vector<cv::ocl::oclMat> dev_dst;
        cv::ocl::split(cv::ocl::oclMat(src), dev_dst);

        dst.resize(dev_dst.size());
        for (size_t i = 0; i < dev_dst.size(); ++i)
            dev_dst[i].download(dst[i]);
    );

    ASSERT_EQ(dst_gold.size(), dst.size());

    for (size_t i = 0; i < dst_gold.size(); ++i)
    {
        EXPECT_MAT_NEAR(dst_gold[i], dst[i], 0.0);
    }
}

INSTANTIATE_TEST_CASE_P(MatOp, Split,
                        testing::ValuesIn(all_types()));


////////////////////////////////////////////////////////////////////////////////
// split_merge_consistency

struct SplitMerge : testing::TestWithParam< int >
{
    
    int type;

    cv::Size size;
    cv::Mat orig;

    virtual void SetUp() 
    {
        type = GetParam();
        
        cv::RNG& rng = cvtest::TS::ptr()->get_rng();

        size = cv::Size(rng.uniform(20, 150), rng.uniform(20, 150));

        orig.create(size, type);
        orig.setTo(cv::Scalar(1.0, 2.0, 3.0, 4.0));
    }
};

TEST_P(SplitMerge, Consistency)
{
    if (CV_MAT_DEPTH(type) == CV_64F )
        return;

    
    PRINT_TYPE(type);
    PRINT_PARAM(size);

    cv::Mat final;

    ASSERT_NO_THROW(
        std::vector<cv::ocl::oclMat> dev_vec;
        cv::ocl::oclMat dev_final;

        cv::ocl::split(cv::ocl::oclMat(orig), dev_vec);    
        cv::ocl::merge(dev_vec, dev_final);

        dev_final.download(final);
    );

    EXPECT_MAT_NEAR(orig, final, 0.0);
}

INSTANTIATE_TEST_CASE_P(MatOp, SplitMerge, 
                        testing::ValuesIn(all_types()));


////////////////////////////////////////////////////////////////////////////////
// setTo

struct SetTo : testing::TestWithParam< int >
{
    
    int type;

    cv::Size size;
    cv::Mat mat_gold;

    virtual void SetUp() 
    {
        
        type = GetParam();

        cv::RNG& rng = cvtest::TS::ptr()->get_rng();

        size = cv::Size(rng.uniform(20, 150), rng.uniform(20, 150));

        mat_gold.create(size, type);
    }
};

TEST_P(SetTo, Zero)
{
    if (CV_MAT_DEPTH(type) == CV_64F )
        return;

    
    PRINT_TYPE(type);
    PRINT_PARAM(size);

    static cv::Scalar zero = cv::Scalar::all(0);

    cv::Mat mat;

    ASSERT_NO_THROW(
        cv::ocl::oclMat dev_mat(mat_gold);

        mat_gold.setTo(zero);
        dev_mat.setTo(zero);

        dev_mat.download(mat);
    );

    EXPECT_MAT_NEAR(mat_gold, mat, 0.0);
}

TEST_P(SetTo, SameVal)
{
    if (CV_MAT_DEPTH(type) == CV_64F )
        return;

    
    PRINT_TYPE(type);
    PRINT_PARAM(size);

    static cv::Scalar s = cv::Scalar::all(1);

    cv::Mat mat;

    ASSERT_NO_THROW(
        cv::ocl::oclMat dev_mat(mat_gold);

        mat_gold.setTo(s);
        dev_mat.setTo(s);

        dev_mat.download(mat);
    );

    EXPECT_MAT_NEAR(mat_gold, mat, 0.0);
}

TEST_P(SetTo, DifferentVal)
{
    if (CV_MAT_DEPTH(type) == CV_64F )
        return;

    
    PRINT_TYPE(type);
    PRINT_PARAM(size);

    static cv::Scalar s = cv::Scalar(1, 2, 3, 4);

    cv::Mat mat;

    ASSERT_NO_THROW(
        cv::ocl::oclMat dev_mat(mat_gold);

        mat_gold.setTo(s);
        dev_mat.setTo(s);

        dev_mat.download(mat);
    );

    EXPECT_MAT_NEAR(mat_gold, mat, 0.0);
}

TEST_P(SetTo, Masked)
{
    if (CV_MAT_DEPTH(type) == CV_64F )
        return;

    
    PRINT_TYPE(type);
    PRINT_PARAM(size);

    static cv::Scalar s = cv::Scalar(1, 2, 3, 4);

    
    cv::RNG& rng = cvtest::TS::ptr()->get_rng();
    cv::Mat mask = cvtest::randomMat(rng, mat_gold.size(), CV_8UC1, 0.0, 1.5, false);

    cv::Mat mat;

    ASSERT_NO_THROW(
        cv::ocl::oclMat dev_mat(mat_gold);

        mat_gold.setTo(s, mask);
        dev_mat.setTo(s, cv::ocl::oclMat(mask));

        dev_mat.download(mat);
    );

    EXPECT_MAT_NEAR(mat_gold, mat, 0.0);
}

INSTANTIATE_TEST_CASE_P(MatOp, SetTo,
                        testing::Values( CV_8UC1, CV_8UC4, CV_32SC1, CV_32SC4, CV_32FC1, CV_32FC4 ));


////////////////////////////////////////////////////////////////////////////////
// copyTo

struct CopyTo : testing::TestWithParam< int >
{
    
    int type;

    cv::Size size;
    cv::Mat src;

    virtual void SetUp() 
    {
        
        type = GetParam();
      
        cv::RNG& rng = cvtest::TS::ptr()->get_rng();

        size = cv::Size(rng.uniform(20, 150), rng.uniform(20, 150));

        src = cvtest::randomMat(rng, size, type, 0.0, 127.0, false);
    }
};

TEST_P(CopyTo, WithoutMask)
{
    if (CV_MAT_DEPTH(type) == CV_64F )
        return;

    
    PRINT_TYPE(type);
    PRINT_PARAM(size);

    cv::Mat dst_gold;
    src.copyTo(dst_gold);

    cv::Mat dst;

    ASSERT_NO_THROW(
        cv::ocl::oclMat dev_src(src);

        cv::ocl::oclMat dev_dst;

        dev_src.copyTo(dev_dst);

        dev_dst.download(dst);
    );

    EXPECT_MAT_NEAR(dst_gold, dst, 0.0);
}

TEST_P(CopyTo, Masked)
{
    if (CV_MAT_DEPTH(type) == CV_64F )
        return;
	if( type != CV_8UC1 
		|| type != CV_8UC4
		|| type != CV_32SC1
		|| type != CV_32SC4
		|| type != CV_32FC1
		|| type != CV_32FC4 )
		return;

    PRINT_TYPE(type);
    PRINT_PARAM(size);

    cv::RNG& rng = cvtest::TS::ptr()->get_rng();

    cv::Mat mask = cvtest::randomMat(rng, src.size(), CV_8UC1, 0.0, 2.0, false);

    cv::Mat dst_gold(src.size(), src.type(), cv::Scalar::all(0));
    src.copyTo(dst_gold, mask);

    cv::Mat dst;

    ASSERT_NO_THROW(
        cv::ocl::oclMat dev_src(src);

        cv::ocl::oclMat dev_dst(src.size(), src.type(), cv::Scalar::all(0));

        dev_src.copyTo(dev_dst, cv::ocl::oclMat(mask));

        dev_dst.download(dst);
    );

    EXPECT_MAT_NEAR(dst_gold, dst, 0.0);
}

INSTANTIATE_TEST_CASE_P(MatOp, CopyTo, 
                        testing::ValuesIn(all_types()));


#endif // OK


#if    W_OUT

////////////////////////////////////////////////////////////////////////////////
// merge

struct Merge : testing::TestWithParam< int >
{
    
    int type;

    cv::Size size;
    std::vector<cv::Mat> src;

    cv::Mat dst_gold;

    virtual void SetUp() 
    {
        
        type = GetParam();

        cv::RNG& rng = cvtest::TS::ptr()->get_rng();

        size = cv::Size(rng.uniform(20, 150), rng.uniform(20, 150));

        int depth = CV_MAT_DEPTH(type);
        int num_channels = CV_MAT_CN(type);
        src.reserve(num_channels);
        for (int i = 0; i < num_channels; ++i)
            src.push_back(cv::Mat(size, depth, cv::Scalar::all(i))); 

        cv::merge(src, dst_gold);
    }
};

TEST_P(Merge, Accuracy)
{
    if (CV_MAT_DEPTH(type) == CV_64F )
        return;

    PRINT_TYPE(type);
    PRINT_PARAM(size);

    cv::Mat dst;

    ASSERT_NO_THROW(
        std::vector<cv::ocl::oclMat> dev_src;
        cv::ocl::oclMat dev_dst;

        for (size_t i = 0; i < src.size(); ++i)
            dev_src.push_back(cv::ocl::oclMat(src[i]));

        cv::ocl::merge(dev_src, dev_dst); 

        dev_dst.download(dst);
    );

    EXPECT_MAT_NEAR(dst_gold, dst, 0.0);
}

INSTANTIATE_TEST_CASE_P(MatOp, Merge, 
                        testing::ValuesIn(all_types()));


////////////////////////////////////////////////////////////////////////////////
// convertTo

struct ConvertTo : testing::TestWithParam< std::tr1::tuple<int, int> >
{
    
    int depth1;
    int depth2;

    cv::Size size;
    cv::Mat src;

    virtual void SetUp() 
    {
        
        depth1 = std::tr1::get<0>(GetParam());
        depth2 = std::tr1::get<1>(GetParam());

        cv::RNG& rng = cvtest::TS::ptr()->get_rng();

        size = cv::Size(rng.uniform(20, 150), rng.uniform(20, 150));

        src = cvtest::randomMat(rng, size, depth1, 0.0, 127.0, false);
    }
};

TEST_P(ConvertTo, WithoutScaling)
{
    if ((depth1 == CV_64F || depth2 == CV_64F) )
        return;

    
    PRINT_TYPE(depth1);
    PRINT_TYPE(depth2);
    PRINT_PARAM(size);

    cv::Mat dst_gold;
    src.convertTo(dst_gold, depth2);

    cv::Mat dst;
    
    ASSERT_NO_THROW(
        cv::ocl::oclMat dev_src(src);

        cv::ocl::oclMat dev_dst;

        dev_src.convertTo(dev_dst, depth2);

        dev_dst.download(dst);
    );

    EXPECT_MAT_NEAR(dst_gold, dst, 0.0);
}

TEST_P(ConvertTo, WithScaling)
{
    if ((depth1 == CV_64F || depth2 == CV_64F) )
        return;

    
    PRINT_TYPE(depth1);
    PRINT_TYPE(depth2);
    PRINT_PARAM(size);
    
    cv::RNG& rng = cvtest::TS::ptr()->get_rng();

    const double a = rng.uniform(0.0, 1.0);
    const double b = rng.uniform(-10.0, 10.0);
    
    PRINT_PARAM(a);
    PRINT_PARAM(b);

    cv::Mat dst_gold;
    src.convertTo(dst_gold, depth2, a, b);

    cv::Mat dst;
    
    ASSERT_NO_THROW(
        cv::ocl::oclMat dev_src(src);

        cv::ocl::oclMat dev_dst;

        dev_src.convertTo(dev_dst, depth2, a, b);

        dev_dst.download(dst);
    );

    const double eps = depth2 < CV_32F ? 1 : 1e-4;

    EXPECT_MAT_NEAR(dst_gold, dst, eps);
}

INSTANTIATE_TEST_CASE_P(MatOp, ConvertTo, testing::Combine( 
						testing::Values( CV_8UC1, CV_8UC4, CV_32SC1, CV_32SC4, CV_32FC1, CV_32FC4 ),
						testing::Values( CV_8UC1, CV_8UC4, CV_32SC1, CV_32SC4, CV_32FC1, CV_32FC4 )));


#endif  // W_OUT


#endif // HAVE_OPENCL
#endif // TS_MATOP