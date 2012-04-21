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

#include "precomp.hpp"

#ifdef HAVE_OPENCL

using namespace cvtest;
using namespace testing;
using namespace std;


MatType nulltype = -1;

#define ONE_TYPE(type)  testing::ValuesIn(typeVector(type))
#define NULL_TYPE  testing::ValuesIn(typeVector(nulltype))


vector<MatType> typeVector(MatType type)
{
    vector<MatType> v;
    v.push_back(type);
    return v;
}


PARAM_TEST_CASE(ImgprocTestBase, MatType,MatType,MatType,MatType,MatType, bool)
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
		cv::Size size = cv::Size(2560, 2560);
		double min = 1,max = 20; 
		if(type1!=nulltype)
		{
			mat1 = randomMat(rng, size, type1, min, max, false);
			clmat1 = mat1;
		}
		if(type2!=nulltype)
		{
			mat2 = randomMat(rng, size, type2, min, max, false);
			clmat2 = mat2;
		}
		if(type3!=nulltype)
		{
			dst  = randomMat(rng, size, type3, min, max, false);
			cldst = dst;
		}
		if(type4!=nulltype)
		{
			dst1 = randomMat(rng, size, type4, min, max, false);
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
////////////////////////////////equalizeHist//////////////////////////////////////////

struct equalizeHist : ImgprocTestBase {};

TEST_P(equalizeHist, Mat) 
{    
    if (mat1.type() != CV_8UC1 || mat1.type() != dst.type())
    {
        cout<<"Unsupported type"<<endl;
	EXPECT_DOUBLE_EQ(0.0, 0.0);
    }
    else
    {
        for(int j=0; j<LOOP_TIMES; j++)
        {
            random_roi();
            cv::equalizeHist(mat1_roi, dst_roi);
            cv::ocl::equalizeHist(clmat1_roi, cldst_roi);
            cv::Mat cpu_cldst;
            cldst.download(cpu_cldst);

            EXPECT_MAT_NEAR(dst, cpu_cldst, 0.0);
        }
    }
}
/*
INSTANTIATE_TEST_CASE_P(ImgprocTestBase, equalizeHist, Combine(
            ONE_TYPE(CV_8UC1),
            NULL_TYPE,
            ONE_TYPE(CV_8UC1),
            NULL_TYPE,
            NULL_TYPE,
            Values(false))); // Values(false) is the reserved parameter
            */


////////////////////////////////bilateralFilter////////////////////////////////////////////

struct bilateralFilter : ImgprocTestBase {};

TEST_P(bilateralFilter, Mat) 
{    
    double sigmacolor = 50.0;
    int radius = 9;
    int d = 2*radius+1;
    double sigmaspace = 20.0;
    int bordertype[] = {cv::BORDER_CONSTANT,cv::BORDER_REPLICATE/*,BORDER_REFLECT,BORDER_WRAP,BORDER_REFLECT_101*/};
    const char* borderstr[]={"BORDER_CONSTANT", "BORDER_REPLICATE"/*, "BORDER_REFLECT","BORDER_WRAP","BORDER_REFLECT_101"*/};
    if (mat1.type() != CV_8UC1 || mat1.type() != dst.type())
    {
        cout<<"Unsupported type"<<endl;
	EXPECT_DOUBLE_EQ(0.0, 0.0);
    }
    else
    {
        for(int i=0;i<sizeof(bordertype)/sizeof(int);i++)
        for(int j=0; j<LOOP_TIMES; j++)
        {
            random_roi();
            cv::bilateralFilter(mat1_roi, dst_roi, d,sigmacolor,sigmaspace, bordertype[i]);
            cv::ocl::bilateralFilter(clmat1_roi, cldst_roi, d,sigmacolor,sigmaspace, bordertype[i]);

            cv::Mat cpu_cldst;
            cldst.download(cpu_cldst);
            EXPECT_MAT_NEAR(dst, cpu_cldst, 0.0);
        }
    }
}
/*
INSTANTIATE_TEST_CASE_P(ImgprocTestBase, bilateralFilter, Combine(
            ONE_TYPE(CV_8UC1),
            NULL_TYPE,
            ONE_TYPE(CV_8UC1),
            NULL_TYPE,
            NULL_TYPE,
            Values(false))); // Values(false) is the reserved parameter
            */

////////////////////////////////copyMakeBorder////////////////////////////////////////////

struct CopyMakeBorder : ImgprocTestBase {};

TEST_P(CopyMakeBorder, Mat) 
{    
    int bordertype[] = {cv::BORDER_CONSTANT,cv::BORDER_REPLICATE/*,BORDER_REFLECT,BORDER_WRAP,BORDER_REFLECT_101*/};
    const char* borderstr[]={"BORDER_CONSTANT", "BORDER_REPLICATE"/*, "BORDER_REFLECT","BORDER_WRAP","BORDER_REFLECT_101"*/};
    if ((mat1.type() != CV_8UC1 && mat1.type() != CV_8UC4 && mat1.type() != CV_32SC1) || mat1.type() != dst.type())
    {
        cout<<"Unsupported type"<<endl;
	EXPECT_DOUBLE_EQ(0.0, 0.0);
    }
    else
    {
        for(int i=0;i<sizeof(bordertype)/sizeof(int);i++)
        for(int j=0; j<LOOP_TIMES; j++)
        {
            random_roi();
	    cv::copyMakeBorder(mat1_roi, dst_roi, 7,5,5,7, bordertype[i],cv::Scalar(1.0));
	    cv::ocl::copyMakeBorder(clmat1_roi, cldst_roi,7,5,5,7,  bordertype[i],cv::Scalar(1.0));

            cv::Mat cpu_cldst;
            cldst.download(cpu_cldst);

            EXPECT_MAT_NEAR(dst, cpu_cldst, 0.0);
        }
    }
}

/*
INSTANTIATE_TEST_CASE_P(ImgprocTestBase, CopyMakeBorder, Combine(
            Values(CV_8UC1, CV_8UC4, CV_32SC1),
	    NULL_TYPE,
	    Values(CV_8UC1,CV_8UC4,CV_32SC1),
	    NULL_TYPE,
	    NULL_TYPE,
            Values(false))); // Values(false) is the reserved parameter
            */

////////////////////////////////cornerMinEigenVal//////////////////////////////////////////

struct cornerMinEigenVal : ImgprocTestBase {};

TEST_P(cornerMinEigenVal, Mat) 
{    
    for(int j=0; j<LOOP_TIMES; j++)
    {

        random_roi();
	       int blockSize = 7, apertureSize= 1 + 2 * (rand() % 4);
        //int borderType = cv::BORDER_CONSTANT;
        //int borderType = cv::BORDER_REPLICATE;
       	int borderType = cv::BORDER_REFLECT;
        cv::cornerMinEigenVal(mat1_roi, dst_roi, blockSize, apertureSize, borderType); 
        cv::ocl::cornerMinEigenVal(clmat1_roi, cldst_roi, blockSize, apertureSize, borderType);
    
        
        cv::Mat cpu_cldst;
        cldst.download(cpu_cldst);
        //cout<<dst<<endl;
        //cout<<cpu_cldst<<endl;
        EXPECT_MAT_NEAR(dst, cpu_cldst, 1);
    }
}
//INSTANTIATE_TEST_CASE_P(ImgprocTestBase, cornerMinEigenVal, Combine(
//			Values(CV_8UC1,CV_32FC1),
//			NULL_TYPE,
//			ONE_TYPE(CV_32FC1),
//			NULL_TYPE,
//			NULL_TYPE,
//		 Values(false))); // Values(false) is the reserved parameter


////////////////////////////////cornerHarris//////////////////////////////////////////

struct cornerHarris : ImgprocTestBase {};

TEST_P(cornerHarris, Mat) 
{    
    for(int j=0; j<LOOP_TIMES; j++)
    {

        random_roi();
       	int blockSize = 7, apertureSize= 1 + 2 * (rand() % 4);
       	double k = 2;
        //int borderType = cv::BORDER_CONSTANT;
        //int borderType = cv::BORDER_REPLICATE;
       	int borderType = cv::BORDER_REFLECT;
        cv::cornerHarris(mat1_roi, dst_roi, blockSize, apertureSize, k, borderType); 
       	cv::ocl::cornerHarris(clmat1_roi, cldst_roi, blockSize, apertureSize, k, borderType);
        cv::Mat cpu_cldst;
        cldst.download(cpu_cldst);
        //cout<<dst<<endl;
        //cout<<cpu_cldst<<endl;
        EXPECT_MAT_NEAR(dst, cpu_cldst, 1);
    }
}
//INSTANTIATE_TEST_CASE_P(ImgprocTestBase, cornerHarris, Combine(
//			Values(CV_8UC1,CV_32FC1),
//			NULL_TYPE,
//			ONE_TYPE(CV_32FC1),
//			NULL_TYPE,
//			NULL_TYPE,
//   Values(false))); // Values(false) is the reserved parameter

////////////////////////////////integral/////////////////////////////////////////////////

struct integral : ImgprocTestBase {};

TEST_P(integral, Mat) 
{    
    for(int j=0; j<LOOP_TIMES; j++)
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
}
/*
INSTANTIATE_TEST_CASE_P(ImgprocTestBase, integral, Combine(
            ONE_TYPE(CV_8UC1),
			NULL_TYPE,
			ONE_TYPE(CV_32SC1),
			ONE_TYPE(CV_32FC1),
			NULL_TYPE,
            Values(false))); // Values(false) is the reserved parameter
            */

/////////////////////////////////////////////////////////////////////////////////////////////////
// warpAffine  & warpPerspective

PARAM_TEST_CASE(WarpTestBase, MatType, cv::Size, int)
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

/////warpAffine

struct WarpAffine : WarpTestBase{};

TEST_P(WarpAffine, Mat)
{
	static const double coeffs[2][3] =
	{
		{cos(3.14 / 6), -sin(3.14 / 6), 100.0},
		{sin(3.14 / 6), cos(3.14 / 6), -100.0}
	};
	Mat M(2, 3, CV_64F, (void*)coeffs);

	for(int j=0; j<LOOP_TIMES; j++)
    {
        random_roi();

        cv::warpAffine(mat1_roi, dst_roi, M, dsize, interpolation);
        cv::ocl::warpAffine(gmat1, gdst, M, dsize, interpolation);

        cv::Mat cpu_dst;
        gdst_whole.download(cpu_dst);

        EXPECT_MAT_NEAR(dst, cpu_dst, 1.0);
    }

}

//INSTANTIATE_TEST_CASE_P(Imgproc, WarpAffine, Combine(Values(CV_8UC1, CV_8UC4, CV_32FC1, CV_32FC4),  DIFFERENT_SIZES, Values((MatType)cv::INTER_NEAREST, (MatType)cv::INTER_LINEAR, (MatType)cv::INTER_CUBIC, (MatType)(cv::INTER_NEAREST | cv::WARP_INVERSE_MAP), (MatType)(cv::INTER_LINEAR | cv::WARP_INVERSE_MAP), (MatType)(cv::INTER_CUBIC | cv::WARP_INVERSE_MAP))));

// warpPerspective

struct WarpPerspective : WarpTestBase{};

TEST_P(WarpPerspective, Mat)
{
	static const double coeffs[3][3] =
	{
		{cos(3.14 / 6), -sin(3.14 / 6), 100.0},
		{sin(3.14 / 6), cos(3.14 / 6), -100.0},
		{0.0, 0.0, 1.0}
	};
	Mat M(3, 3, CV_64F, (void*)coeffs);

	for(int j=0; j<LOOP_TIMES; j++)
    {
        random_roi();

        cv::warpPerspective(mat1_roi, dst_roi, M, dsize, interpolation);
        cv::ocl::warpPerspective(gmat1, gdst, M, dsize, interpolation);

        cv::Mat cpu_dst;
        gdst_whole.download(cpu_dst);

        EXPECT_MAT_NEAR(dst, cpu_dst, 1.0);
    }

}

//INSTANTIATE_TEST_CASE_P(Imgproc, WarpPerspective, Combine(Values(CV_8UC1, CV_8UC4, CV_32FC1, CV_32FC4),  DIFFERENT_SIZES, Values((MatType)cv::INTER_NEAREST, (MatType)cv::INTER_LINEAR, (MatType)cv::INTER_CUBIC, (MatType)(cv::INTER_NEAREST | cv::WARP_INVERSE_MAP), (MatType)(cv::INTER_LINEAR | cv::WARP_INVERSE_MAP), (MatType)(cv::INTER_CUBIC | cv::WARP_INVERSE_MAP))));

/////////////////////////////////////////////////////////////////////////////////////////////////
// resize

PARAM_TEST_CASE(Resize, MatType, cv::Size, double, double, int)
{
    int type;
	cv::Size dsize;
	double fx, fy;
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
		fx = GET_PARAM(2);
		fy = GET_PARAM(3);
		interpolation = GET_PARAM(4);

        cv::RNG& rng = TS::ptr()->get_rng();
        //cv::Size size = cv::Size(256, 256);
        cv::Size size = cv::Size(3000, 3000);

		if(dsize == cv::Size() && !(fx > 0 && fy > 0))
		{
			cout << "invalid dsize and fx fy" << endl;
			return;
		}

		if(dsize == cv::Size()) 
		{
			dsize.width = (int)(size.width * fx);
			dsize.height = (int)(size.height * fy);
		}

        mat1 = randomMat(rng, size, type, 5, 16, false);
        dst  = randomMat(rng, dsize, type, 5, 16, false);
  
    }

    void random_roi()
    {
        cv::RNG& rng = TS::ptr()->get_rng();

        //randomize ROI
		/*	
        src_roicols = rng.uniform(1, mat1.cols);
        src_roirows = rng.uniform(1, mat1.rows);
		dst_roicols = rng.uniform(1, dst.cols);
        dst_roirows = rng.uniform(1, dst.rows);
        src1x   = rng.uniform(0, mat1.cols - src_roicols);
        src1y   = rng.uniform(0, mat1.rows - src_roirows);
        dstx    = rng.uniform(0, dst.cols  - dst_roicols);
        dsty    = rng.uniform(0, dst.rows  - dst_roirows);
*/
        src_roicols = mat1.cols;
        src_roirows = mat1.rows;
		dst_roicols = dst.cols;
        dst_roirows = dst.rows;
        src1x   = 0;
        src1y   = 0;
        dstx    = 0;
        dsty    = 0;

        mat1_roi = mat1(Rect(src1x,src1y,src_roicols,src_roirows));
        dst_roi  = dst(Rect(dstx,dsty,dst_roicols,dst_roirows));

        gdst_whole = dst;
        gdst = gdst_whole(Rect(dstx,dsty,dst_roicols,dst_roirows));


        gmat1 = mat1_roi;
    }

};

TEST_P(Resize, Mat)
{
	for(int j=0; j<10/*LOOP_TIMES*/; j++)
    {
        random_roi();

       // cv::resize(mat1_roi, dst_roi, dsize, fx, fy, interpolation);
       // cv::ocl::resize(gmat1, gdst, dsize, fx, fy, interpolation);

		double t1 = (double)cv::getTickCount();
        cv::resize(mat1_roi, dst_roi, dsize, fx, fy, interpolation);
		t1 = ((double)cv::getTickCount()-t1)*1000/cv::getTickFrequency();

		double t2 = (double)cv::getTickCount();
        cv::ocl::resize(gmat1, gdst, dsize, fx, fy, interpolation);
		t2 = ((double)cv::getTickCount()-t2)*1000/cv::getTickFrequency();

		cout << "cpu time :" << t1 << "ms" << endl;
		cout << "gpu time :" << t2 << "ms" << endl;

        cv::Mat cpu_dst;
        gdst_whole.download(cpu_dst);

        EXPECT_MAT_NEAR(dst, cpu_dst, 1.0);
    }

}

//INSTANTIATE_TEST_CASE_P(Imgproc, Resize, Combine(Values(CV_8UC1, CV_8UC4, CV_32FC1, CV_32FC4),  Values(cv::Size()), Values(0.5, 1.5, 2), Values(0.5, 1.5, 2), Values((MatType)cv::INTER_NEAREST, (MatType)cv::INTER_LINEAR)));
//INSTANTIATE_TEST_CASE_P(Imgproc, Resize, Combine(ONE_TYPE(CV_8UC1),  Values(cv::Size(3300, 3300)), Values(0.), Values(0.), Values((MatType)cv::INTER_NEAREST, (MatType)cv::INTER_LINEAR)));


/////////////////////////////////////////////////////////////////////////////////////////////////
//threshold 

PARAM_TEST_CASE(Threshold, MatType, ThreshOp)
{
    int type;
	int threshOp;

    //src mat
    cv::Mat mat1; 
    cv::Mat dst;

    // set up roi
    int roicols;
    int roirows;
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
		threshOp = GET_PARAM(1);

        cv::RNG& rng = TS::ptr()->get_rng();
        cv::Size size = cv::Size(2560, 2560);

        mat1 = randomMat(rng, size, type, 5, 16, false);
        dst  = randomMat(rng, size, type, 5, 16, false);

    }

    void random_roi()
    {
        cv::RNG& rng = TS::ptr()->get_rng();

        //randomize ROI
		roicols = rng.uniform(1, mat1.cols);
        roirows = rng.uniform(1, mat1.rows);
        src1x   = rng.uniform(0, mat1.cols - roicols);
        src1y   = rng.uniform(0, mat1.rows - roirows);
        dstx    = rng.uniform(0, dst.cols  - roicols);
        dsty    = rng.uniform(0, dst.rows  - roirows);
        /*
        roicols = mat1.cols;
        roirows = mat1.rows;
        src1x   = 0;
        src1y   = 0;
        dstx    = 0;
        dsty    = 0; 
*/

        mat1_roi = mat1(Rect(src1x,src1y,roicols,roirows));
        dst_roi  = dst(Rect(dstx,dsty,roicols,roirows));

        gdst_whole = dst;
        gdst = gdst_whole(Rect(dstx,dsty,roicols,roirows));


        gmat1 = mat1_roi;
    }

};

TEST_P(Threshold, Mat)
{
	for(int j=0; j<LOOP_TIMES; j++)
    {
        random_roi();
    	double maxVal = randomDouble(20.0, 127.0);
    	double thresh = randomDouble(0.0, maxVal);

        cv::threshold(mat1_roi, dst_roi, thresh, maxVal, threshOp);
        cv::ocl::threshold(gmat1, gdst, thresh, maxVal, threshOp);

        cv::Mat cpu_dst;
        gdst_whole.download(cpu_dst);

        //EXPECT_MAT_NEAR(dst, cpu_dst, 1e-5);
        EXPECT_MAT_NEAR(dst, cpu_dst, 1);
    }

}
//INSTANTIATE_TEST_CASE_P(Imgproc, Threshold, Combine(Values(CV_8UC1, CV_32FC1), Values(ThreshOp(cv::THRESH_BINARY), ThreshOp(cv::THRESH_BINARY_INV), ThreshOp(cv::THRESH_TRUNC), ThreshOp(cv::THRESH_TOZERO), ThreshOp(cv::THRESH_TOZERO_INV))));

#endif // HAVE_OPENCL
