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
// Copyright (C) 2010-2012, Institute Of Software Chinese Academy Of Science, all rights reserved.
// Copyright (C) 2010-2012, Advanced Micro Devices, Inc., all rights reserved.
// Third party copyrights are property of their respective owners.
//
// @Authors
//    Niko Li, Niko.li@amd.com
//    Jia Haipeng, jiahaipeng95@gmail.com
//    Shengen Yan, yanshengen@gmail.com
//    Jiang Liyuan, lyuan001.good@163.com
//    Rock Li, Rock.Li@amd.com
//    Zailong Wu, bullet@yeah.net
//
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
//
//   * Redistribution's of source code must retain the above copyright notice,
//     this list of conditions and the following disclaimer.
//
//   * Redistribution's in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other oclMaterials provided with the distribution.
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

#include "test_precomp.hpp"
#if    TS_IMGPROC2
#ifdef HAVE_OPENCL

using namespace cvtest;
using namespace testing;
using namespace std;
using namespace cv;

int nulltype = -1;

#define ONE_TYPE(type)  testing::ValuesIn(typeVector(type))
#define NULL_TYPE  testing::ValuesIn(typeVector(nulltype))

vector<int> typeVector(int type)
{
    vector<int> v;
    v.push_back(type);
    return v;
}

PARAM_TEST_CASE(ImgprocTestBase, int,int,int,int,int, bool)
{
    int type1,type2,type3,type4,type5;
    cv::Scalar val;
    // set up roi
    int roicols;
    int roirows;
    int src1x;
    int src1y;
    int src2x;
    int src2y;
    int dstx;
    int dsty;
    int dst1x;
    int dst1y;
    int maskx;
    int masky;

    //mat
    cv::Mat mat1;
    cv::Mat mat2;
    cv::Mat mask;
    cv::Mat dst;
    cv::Mat dst1; //bak, for two outputs

    //mat with roi
    cv::Mat mat1_roi;
    cv::Mat mat2_roi;
    cv::Mat mask_roi;
    cv::Mat dst_roi;
    cv::Mat dst1_roi; //bak

    //ocl mat
    cv::ocl::oclMat clmat1;
    cv::ocl::oclMat clmat2;
	cv::ocl::oclMat clmask;
    cv::ocl::oclMat cldst;
    cv::ocl::oclMat cldst1; //bak

    //ocl mat with roi
    cv::ocl::oclMat clmat1_roi;
    cv::ocl::oclMat clmat2_roi;
    cv::ocl::oclMat clmask_roi;
    cv::ocl::oclMat cldst_roi;
    cv::ocl::oclMat cldst1_roi;

    virtual void SetUp()
    {
		type1 = GET_PARAM(0);
		type2 = GET_PARAM(1);
		type3 = GET_PARAM(2);
		type4 = GET_PARAM(3);
		type5 = GET_PARAM(4);
        cv::RNG& rng = TS::ptr()->get_rng();
        cv::Size size = cv::Size(256, 256);

		if(type1!=nulltype)
		{
			mat1 = randomMat(rng, size, type1, 5, 16, false);
			clmat1 = mat1;
		}
		if(type2!=nulltype)
		{
			mat2 = randomMat(rng, size, type2, 5, 16, false);
			clmat2 = mat2;
		}
		if(type3!=nulltype)
		{
			dst  = randomMat(rng, size, type3, 5, 16, false);
			cldst = dst;
		}
		if(type4!=nulltype)
		{
			dst1 = randomMat(rng, size, type4, 5, 16, false);
			cldst1 = dst1;
		}
		if(type5!=nulltype)
		{
			mask = randomMat(rng, size, CV_8UC1, 0, 2,  false);
			cv::threshold(mask, mask, 0.5, 255., type5);
			clmask = mask;
		}
        val = cv::Scalar(rng.uniform(-10.0, 10.0), rng.uniform(-10.0, 10.0), rng.uniform(-10.0, 10.0), rng.uniform(-10.0, 10.0));
    }

    void random_roi()
    {
        cv::RNG& rng = TS::ptr()->get_rng();

        //randomize ROI
        roicols = rng.uniform(1, mat1.cols);
        roirows = rng.uniform(1, mat1.rows);
        src1x   = rng.uniform(0, mat1.cols - roicols);
        src1y   = rng.uniform(0, mat1.rows - roirows);
        src2x   = rng.uniform(0, mat2.cols - roicols);
        src2y   = rng.uniform(0, mat2.rows - roirows);
        dstx    = rng.uniform(0, dst.cols  - roicols);
        dsty    = rng.uniform(0, dst.rows  - roirows);
        dst1x    = rng.uniform(0, dst1.cols  - roicols);
        dst1y    = rng.uniform(0, dst1.rows  - roirows);
        maskx   = rng.uniform(0, mask.cols - roicols);
        masky   = rng.uniform(0, mask.rows - roirows);

		if(type1!=nulltype)
		{
			mat1_roi = mat1(Rect(src1x,src1y,roicols,roirows));
			clmat1_roi = clmat1(Rect(src1x,src1y,roicols,roirows));
		}
		if(type2!=nulltype)
		{
			mat2_roi = mat2(Rect(src2x,src2y,roicols,roirows));
			clmat2_roi = clmat2(Rect(src2x,src2y,roicols,roirows));
		}
		if(type3!=nulltype)
		{
			dst_roi  = dst(Rect(dstx,dsty,roicols,roirows));
			cldst_roi = cldst(Rect(dstx,dsty,roicols,roirows));
		}
		if(type4!=nulltype)
		{
			dst1_roi = dst1(Rect(dst1x,dst1y,roicols,roirows));
			cldst1_roi = cldst1(Rect(dst1x,dst1y,roicols,roirows));
		}
		if(type5!=nulltype)
		{
			mask_roi = mask(Rect(maskx,masky,roicols,roirows));
			clmask_roi = clmask(Rect(maskx,masky,roicols,roirows));
		}
    }
};

////////////////////////////////cornerHarris/////////////////////////////////////////////////

struct CornerHarris : ImgprocTestBase {};

TEST_P(CornerHarris, Mat)
{
        random_roi();
		int blockSize =  1 + rand() % 5, apertureSize= 1 + 2 * (rand() % 4);
		double k = 0.1;
		int borderType = 2;// BORDER_REPLICATE;
		cv::cornerHarris(mat1_roi, dst_roi, blockSize, apertureSize, k, borderType);
		cv::ocl::cornerHarris(clmat1_roi, cldst_roi, blockSize, apertureSize, k, borderType);

        cv::Mat cpu_cldst;
        cldst.download(cpu_cldst);

        EXPECT_MAT_NEAR(dst, cpu_cldst, 0.0);
}
INSTANTIATE_TEST_CASE_P(ImgprocTestBase, CornerHarris, Combine(
           Values(CV_8UC1, CV_32FC1),
			NULL_TYPE,
			ONE_TYPE(CV_32FC1),
			NULL_TYPE,
			NULL_TYPE,
            Values(false))); // Values(false) is the reserved parameter

////////////////////////////////integral/////////////////////////////////////////////////

struct Integral : ImgprocTestBase {};

TEST_P(Integral, Mat)
{
        random_roi();

		cv::ocl::integral(clmat1_roi, cldst_roi, cldst1_roi);
		cv::integral(mat1_roi, dst_roi, dst1_roi);

		cv::Mat cpu_cldst, cpu_cldst1;
		cldst.download(cpu_cldst);
        cldst1.download(cpu_cldst1);

        EXPECT_MAT_NEAR(dst, cpu_cldst, 0.0);
		EXPECT_MAT_NEAR(dst1, cpu_cldst1, 0.0);
}
INSTANTIATE_TEST_CASE_P(ImgprocTestBase, Integral, Combine(
            ONE_TYPE(CV_8UC1),
			NULL_TYPE,
			ONE_TYPE(CV_32SC1),
			ONE_TYPE(CV_32FC1),
			NULL_TYPE,
            Values(false))); // Values(false) is the reserved parameter

/////////////////////////////////////////////////////////////////////////////////////////////////
// warpPerspective

PARAM_TEST_CASE(WarpPerspective, int, cv::Size, int)
{
    int type;
	cv::Size dsize;
	int interpolation;

    //src mat
    cv::Mat mat1;
    cv::Mat dst;

    // set up roi
    int src_roicols;
    int src_roirows;
	int dst_roicols;
    int dst_roirows;
    int src1x;
    int src1y;
    int dstx;
    int dsty;

    //src mat with roi
    cv::Mat mat1_roi;
    cv::Mat dst_roi;

    //ocl dst mat for testing
    cv::ocl::oclMat gdst_whole;

    //ocl mat with roi
    cv::ocl::oclMat gmat1;
    cv::ocl::oclMat gdst;

    virtual void SetUp()
    {
        type = GET_PARAM(0);
		dsize = GET_PARAM(1);
		interpolation = GET_PARAM(2);

        cv::RNG& rng = TS::ptr()->get_rng();
        cv::Size size = cv::Size(256, 256);

        mat1 = randomMat(rng, size, type, 5, 16, false);
        dst  = randomMat(rng, dsize, type, 5, 16, false);
    }

    void random_roi()
    {
        cv::RNG& rng = TS::ptr()->get_rng();

        //randomize ROI
        src_roicols = rng.uniform(1, mat1.cols);
        src_roirows = rng.uniform(1, mat1.rows);
		dst_roicols = rng.uniform(1, dst.cols);
        dst_roirows = rng.uniform(1, dst.rows);
        src1x   = rng.uniform(0, mat1.cols - src_roicols);
        src1y   = rng.uniform(0, mat1.rows - src_roirows);
        dstx    = rng.uniform(0, dst.cols  - dst_roicols);
        dsty    = rng.uniform(0, dst.rows  - dst_roirows);

        mat1_roi = mat1(Rect(src1x,src1y,src_roicols,src_roirows));
        dst_roi  = dst(Rect(dstx,dsty,dst_roicols,dst_roirows));

        gdst_whole = dst;
        gdst = gdst_whole(Rect(dstx,dsty,dst_roicols,dst_roirows));

        gmat1 = mat1_roi;
    }
};

TEST_P(WarpPerspective, Mat)
{
	static const double coeffs[3][3] =
	{
		{cos(3.14 / 6), -sin(3.14 / 6), 100.0},
		{sin(3.14 / 6), cos(3.14 / 6), -100.0},
		{0.0, 0.0, 1.0}
	};
	Mat M(3, 3, CV_64F, (void*)coeffs);

        random_roi();

        cv::warpPerspective(mat1_roi, dst_roi, M, dsize, interpolation);
        cv::ocl::warpPerspective(gmat1, gdst, M, dsize, interpolation);

        cv::Mat cpu_dst;
        gdst_whole.download(cpu_dst);

        EXPECT_MAT_NEAR(dst, cpu_dst, 1.0);
}

INSTANTIATE_TEST_CASE_P(Imgproc, WarpPerspective, Combine(Values(CV_8UC1, CV_8UC1/*, CV_32FC1, CV_32FC4*/),  DIFFERENT_SIZES,
                        Values((int)INTER_NEAREST, (int)INTER_LINEAR/*, INTER_CUBIC, INTER_NEAREST | WARP_INVERSE_MAP, INTER_LINEAR | WARP_INVERSE_MAP, INTER_CUBIC | WARP_INVERSE_MAP*/)
                        ));

#endif // HAVE_OPENCL
#endif // TS_IMGPROC2