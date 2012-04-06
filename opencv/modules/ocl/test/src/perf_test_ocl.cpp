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
#include <ctime>
#include <windows.h>

#if PREF_TEST_OCL

#ifdef HAVE_OPENCL

#define SHOW_CPU false
#define REPEAT   100
#define R_T2( test ) \
	{ \
		std::cout<< "\n" #test "\n----------------------"; \
		R_TEST( test, 15 ) \
		clock_t st = clock(); \
		R_T( test ) \
		std::cout<< clock() - st << "ms\n"; \
	}
#define R_T( test ) \
	R_TEST( test, REPEAT )
#define R_TEST( test, repeat ) \
	try{ \
		for( int i = 0; i < repeat; i ++ ) { test; } \
	} catch( ... ) { std::cout << "||||| Exception catched! |||||\n"; return; }
#define COUNT_D 0
#define FILTER_TEST_IMAGE "C:/Windows/Web/Wallpaper/Landscapes/img9.jpg"
#define WARN_NRUN( name ) \
	std::cout << "Warning: " #name " is not runnable!\n";


void print_info();

// performance base class
struct PerfTest
{
	virtual void Run()   = 0;
protected:
	virtual void SetUp() = 0;
};
///////////////////////////////////////
// Arithm
struct ArithmTestP : PerfTest
{
    int type;

    cv::Size size;
    cv::Mat mat1, mat2;
	cv::Mat dst;
	cv::ocl::oclMat gpuRes, oclmat1, oclmat2;
	std::vector<cv::Mat> dstv;
protected:
	ArithmTestP() : type( CV_8UC4 ) {}
    virtual void SetUp()
    {
        cv::RNG& rng = cvtest::TS::ptr()->get_rng();
        size = cv::Size( 3000, 3000 ); // big input image
        mat1 = cvtest::randomMat(rng, size, type, 1, 255, false);
        mat2 = cvtest::randomMat(rng, size, type, 1, 255, false);
		oclmat1 = cv::ocl::oclMat(mat1);
		oclmat2 = cv::ocl::oclMat(mat2);
    }
};

struct AddArrayP : ArithmTestP
{
	virtual void Run()
	{
		SetUp();
		clock_t start = clock();
		R_T2(
				cv::ocl::add(oclmat1, oclmat2, gpuRes);
				if( COUNT_D ) gpuRes.download(dst);
			);
		if( SHOW_CPU )
		{
			start = clock();
			cv::add(mat1, mat2, dst);
			std::cout<< "cv::add -- " << clock() - start << "ms\n";
		}
	}
};

struct SubtractArrayP : ArithmTestP
{
	virtual void Run()
	{
		SetUp();
		clock_t start = clock();
		R_T2(
			cv::ocl::subtract(oclmat1, oclmat2, gpuRes);
			if( COUNT_D ) gpuRes.download(dst);
		);
		if( SHOW_CPU )
		{
			start = clock();
			cv::subtract(mat1, mat2, dst);
			std::cout<< "cv::subtract -- " << clock() - start << "ms\n";
		}
	}
};

struct MultiplyArrayP : ArithmTestP
{
	virtual void Run()
	{
		SetUp();
		clock_t start = clock();
		R_T2(
			cv::ocl::multiply(oclmat1, oclmat2, gpuRes);
			if( COUNT_D ) gpuRes.download(dst);
		);
		if( SHOW_CPU )
		{
			start = clock();
			cv::multiply(mat1, mat2, dst);
			std::cout<< "cv::multiply -- " << clock() - start << "ms\n";
		}
	}
};

struct DivideArrayP : ArithmTestP
{
	virtual void Run()
	{
		SetUp();
		clock_t start = clock();
		R_T2(
			cv::ocl::divide(oclmat1, oclmat2, gpuRes);
			if( COUNT_D ) gpuRes.download(dst);
		);
		if( SHOW_CPU )
		{
			start = clock();
			cv::divide(mat1, mat2, dst);
			std::cout<< "cv::divide -- " << clock() - start << "ms\n";
		}
	}
};

struct CompareP : ArithmTestP
{
	virtual void Run()
	{
		type = CV_32FC1;
		SetUp();
		clock_t start = clock();
		R_T2(
			cv::ocl::compare(oclmat1, oclmat2, gpuRes, cv::CMP_EQ);
			if( COUNT_D ) gpuRes.download(dst);
		);
		if( SHOW_CPU )
		{
			start = clock();
			cv::compare(mat1, mat2, dst, cv::CMP_EQ);
			std::cout<< "cv::CompareP -- " << clock() - start << "ms\n";
		}
	}
};

struct FlipP : ArithmTestP
{
	virtual void Run()
	{
		SetUp();
		clock_t start = clock();
		R_T2(
			cv::ocl::flip(oclmat1, gpuRes, 0);
			if( COUNT_D ) gpuRes.download(dst);
		);
		if( SHOW_CPU )
		{
			start = clock();
			cv::flip(mat1, dst, 0);
			std::cout<< "cv::FlipP -- " << clock() - start << "ms\n";
		}
	}
protected:
	virtual void SetUp()
    {
		type = CV_8UC4;
        cv::RNG& rng = cvtest::TS::ptr()->get_rng();
        size = cv::Size(3000, 3000);
        mat1 = cvtest::randomMat(rng, size, type, 1, 255, false);
		oclmat1 = cv::ocl::oclMat(mat1);
    }
};

struct MagnitudeP : ArithmTestP
{
	virtual void Run()
	{
		type = CV_32F;
		SetUp();
		clock_t start = clock();
		R_T2(
			cv::ocl::magnitude(oclmat1, oclmat1, gpuRes);
			if( COUNT_D ) gpuRes.download(dst);
		);
		if( SHOW_CPU )
		{
			start = clock();
			cv::magnitude(mat1, mat2, dst);
			std::cout<< "cv::Magnitude -- " << clock() - start << "ms\n";
		}
	}
};

struct LUTP : ArithmTestP
{
	virtual void Run()
	{
		SetUp();
		clock_t start = clock();
		R_T2(
			cv::ocl::LUT(oclmat1, ocllut, gpuRes);
			if( COUNT_D ) gpuRes.download(dst);
		);
		if( SHOW_CPU )
		{
			start = clock();
			cv::LUT(mat1, lut, dst);
			std::cout<< "cv::LUT -- " << clock() - start << "ms\n";
		}
	}
protected:
	cv::Mat lut;
	cv::ocl::oclMat ocllut;
	virtual void SetUp()
    {
		type = CV_8UC4;
        cv::RNG& rng = cvtest::TS::ptr()->get_rng();
        size = cv::Size(3000, 3000);
        mat1 = cvtest::randomMat(rng, size, type, 1, 255, false);
		lut = cvtest::randomMat(rng, cv::Size(256, 1), CV_8UC1, 100, 200, false);
		oclmat1 = cv::ocl::oclMat(mat1);
		ocllut  = cv::ocl::oclMat(lut);
    }
};

struct MinMaxP : ArithmTestP
{
    double minVal_gold, minVal;
    double maxVal_gold, maxVal;

	virtual void Run()
	{
		SetUp();
		clock_t start = clock();
		R_T2(
			cv::ocl::minMax(oclmat1, &minVal, &maxVal, oclmat2);
		);
	
		if( SHOW_CPU )
		{
			start = clock();
			cv::minMaxLoc(mat1, &minVal_gold, &maxVal_gold, 0, 0, mat2);
			std::cout<< "cv::minMax -- " << clock() - start << "ms\n";
		}
	}

protected:
	virtual void SetUp()
    {
        type = CV_64F;

        cv::RNG& rng = cvtest::TS::ptr()->get_rng();

        size = cv::Size(3000, 3000);

        mat1 = cvtest::randomMat(rng, size, type, 0.0, 127.0, false);
        mat2 = cvtest::randomMat(rng, size, CV_8UC1, 0, 2, false);

		oclmat1 = cv::ocl::oclMat(mat1);
		oclmat2 = cv::ocl::oclMat(mat2);
    }
};

struct MinMaxLocP : MinMaxP
{
    cv::Point minLoc_gold;
    cv::Point maxLoc_gold;
	virtual void Run()
	{
		SetUp();
		clock_t start = clock();
		R_T2(
			cv::ocl::minMaxLoc(oclmat1, &minVal, &maxVal, &minLoc_gold, &maxLoc_gold, oclmat2);
		);
		
		if( SHOW_CPU )
		{
			start = clock();
			cv::minMaxLoc(mat1, &minVal_gold, &maxVal_gold, &minLoc_gold, &maxLoc_gold, mat2);
			std::cout<< "cv::minMaxLoc -- " << clock() - start << "ms\n";
		}
	}
};

struct CountNonZeroP : ArithmTestP
{
	int n;
	virtual void Run()
	{
		SetUp();
		clock_t start = clock();
		R_T2(
			n = cv::ocl::countNonZero(oclmat1);
		);
		if( SHOW_CPU )
		{
			start = clock();
			n = cv::countNonZero(mat1);
			std::cout<< "cv::countNonZero -- " << clock() - start << "ms\n";
		}
	}
protected:
    virtual void SetUp()
    {
        type = 6;

        cv::RNG& rng = cvtest::TS::ptr()->get_rng();

        size = cv::Size( 3000, 3000 );

        cv::Mat matBase = cvtest::randomMat(rng, size, CV_8U, 0.0, 1.0, false);
        matBase.convertTo(mat1, type);

		oclmat1 = cv::ocl::oclMat(mat1);
    }
};

struct SumP : ArithmTestP
{
	virtual void Run()
	{
		SetUp();

		cv::Scalar n;
		clock_t start = clock();
		R_T2(
			n = cv::ocl::sum(oclmat1);
		);
		
		if( SHOW_CPU )
		{
			start = clock();
			n = cv::sum(mat1);
			std::cout<< "cv::sum -- " << clock() - start << "ms\n";
		}
	}
};

struct BitwiseP : ArithmTestP
{
protected:
    virtual void SetUp()
    {
        type = CV_8UC4;

        cv::RNG& rng = cvtest::TS::ptr()->get_rng();

        size = cv::Size( 3000, 3000 );

        mat1.create(size, type);
        mat2.create(size, type);

        for (int i = 0; i < mat1.rows; ++i)
        {
            cv::Mat row1(1, static_cast<int>(mat1.cols * mat1.elemSize()), CV_8U, (void*)mat1.ptr(i));
            rng.fill(row1, cv::RNG::UNIFORM, cv::Scalar(0), cv::Scalar(255));

            cv::Mat row2(1, static_cast<int>(mat2.cols * mat2.elemSize()), CV_8U, (void*)mat2.ptr(i));
            rng.fill(row2, cv::RNG::UNIFORM, cv::Scalar(0), cv::Scalar(255));
        }
		oclmat1 = cv::ocl::oclMat(mat1);
		oclmat2 = cv::ocl::oclMat(mat2);
    }
};

struct BitwiseNotP : BitwiseP
{
	virtual void Run()
	{
		SetUp();
		clock_t start = clock();
		R_T2(
			cv::ocl::bitwise_not(oclmat1, gpuRes);
			if( COUNT_D ) gpuRes.download(dst);
		);
		
		if( SHOW_CPU )
		{
			start = clock();
			dst = ~mat1;
			std::cout<< "cv::BitWiseNot -- " << clock() - start << "ms\n";
		}
	}
};

struct BitwiseAndP : BitwiseP
{
	virtual void Run()
	{
		SetUp();
		clock_t start = clock();
		R_T2(
			cv::ocl::bitwise_and(oclmat1, oclmat2, gpuRes);
			if( COUNT_D ) gpuRes.download(dst);
		);
		
		if( SHOW_CPU )
		{
			start = clock();
			dst = mat1 & mat2;
			std::cout<< "cv::BitWiseAnd -- " << clock() - start << "ms\n";
		}
	}
};

struct BitwiseXorP : BitwiseP
{
	virtual void Run()
	{
		SetUp();
		clock_t start = clock();
		R_T2(
			cv::ocl::bitwise_xor(oclmat1, oclmat2, gpuRes);
			if( COUNT_D ) gpuRes.download(dst);
		);
		
		if( SHOW_CPU )
		{
			start = clock();
			dst = mat1 ^ mat2;
			std::cout<< "cv::BitWiseXor -- " << clock() - start << "ms\n";
		}
	}
};

struct BitwiseOrP : BitwiseP
{
	virtual void Run()
	{
		SetUp();
		clock_t start = clock();
		R_T2(
			cv::ocl::bitwise_or(oclmat1, oclmat2, gpuRes);
			if( COUNT_D ) gpuRes.download(dst);
		);
		
		if( SHOW_CPU )
		{
			start = clock();
			dst = mat1 | mat2;
			std::cout<< "cv::BitWiseOr -- " << clock() - start << "ms\n";
		}
	}
};

struct TransposeP : ArithmTestP
{
	virtual void Run()
	{
		SetUp();
		clock_t start = clock();
		R_T2(
			cv::ocl::transpose(oclmat1, gpuRes);
			if( COUNT_D ) gpuRes.download(dst);
		);
		
		if( SHOW_CPU )
		{
			start = clock();
			cv::transpose(mat1, dst);
			std::cout<< "cv::transpose -- " << clock() - start << "ms\n";
		}
	}
};

struct AbsdiffArrayP : ArithmTestP
{
	virtual void Run()
	{
		type = CV_32FC1;
		SetUp();
		clock_t start = clock();
		R_T2(
			cv::ocl::absdiff(oclmat1, oclmat2, gpuRes);
			if( COUNT_D ) gpuRes.download(dst);
		);
		
		if( SHOW_CPU )
		{
			start = clock();
			cv::absdiff(mat1, mat2, dst);
			std::cout<< "cv::absdiff -- " << clock() - start << "ms\n";
		}
	}
};

struct PhaseP : ArithmTestP
{
	virtual void Run()
	{
		type = CV_32F;
		SetUp();
		clock_t start = clock();
		R_T2(
			cv::ocl::phase(oclmat1,oclmat2,gpuRes,1);
			if( COUNT_D ) gpuRes.download(dst);
		);
	}
};

struct CartToPolarP : ArithmTestP
{
	cv::ocl::oclMat gpuRes1;
	virtual void Run()
	{
		type = CV_64FC4;
		SetUp();
		clock_t start = clock();
		R_TEST(
			cv::ocl::cartToPolar(oclmat1,oclmat2,gpuRes, gpuRes1, 1);
			if( COUNT_D ) {gpuRes.download(dst);gpuRes1.download(dst);}
		, 5);
		std::cout<< "ocl::CartToPolar -- " << clock() - start << "ms\n";
	}
};

struct PolarToCartP : ArithmTestP
{
	cv::ocl::oclMat gpuRes1;
	virtual void Run()
	{
		type = CV_64FC4;
		SetUp();
		clock_t start = clock();
		R_TEST(
			cv::ocl::polarToCart(oclmat1,oclmat2,gpuRes, gpuRes1, 1);
			if( COUNT_D ) {gpuRes.download(dst);gpuRes1.download(dst);}
		, 2);
		std::cout<< "ocl::polarToCart -- " << clock() - start << "ms\n";
	}
};

///////////////////////////////////////
// split & merge
struct SplitP : ArithmTestP
{
	virtual void Run()
	{
		SetUp();
		clock_t start = clock();
		R_T2(
			cv::ocl::split(oclmat1, dev_dst);
			if( COUNT_D )
			{
				dstv.resize(dev_dst.size());
				for (size_t i = 0; i < dev_dst.size(); ++i)
				{
					dev_dst[i].download(dstv[i]);
				}
			}
		);
		if( SHOW_CPU )
		{
			start = clock();
			cv::split(mat1, dstv);
			std::cout<< "cv::split -- " << clock() - start << "ms\n";
		}
	}
protected:
	std::vector<cv::ocl::oclMat> dev_dst;
	virtual void SetUp()
	{
        size = cv::Size( 3000, 3000 );

        mat1.create(size, type);
        mat1.setTo(cv::Scalar(1.0, 2.0, 3.0, 4.0));

		oclmat1 = cv::ocl::oclMat(mat1);
	}
};

struct MergeP : SplitP
{
	virtual void Run()
	{
		SetUp();
		cv::ocl::split(oclmat1, dev_dst);
		cv::split(mat1, dstv);

		//-------->
		clock_t start = clock();
		R_T2(
			cv::ocl::merge(dev_dst, oclmat2);
			if( COUNT_D ) oclmat2.download(dst);
		);

		if( SHOW_CPU )
		{
			start = clock();
			cv::merge(dstv, dst);
			std::cout<< "cv::merge -- " << clock() - start << "ms\n";
		}
	}
};

struct SetToP : ArithmTestP
{
	virtual void Run()
	{
		SetUp();
		static cv::Scalar s = cv::Scalar(1, 2, 3, 4);

		clock_t start = clock();
		R_T2(
			oclmat1.setTo( s, oclmat2 );
			if( COUNT_D ) oclmat1.download(dst);
		);

		if( SHOW_CPU )
		{
			start = clock();
			mat1.setTo( s, mat2 );
			std::cout<< "cv::setTo -- " << clock() - start << "ms\n";
		}
	}
protected:
	virtual void SetUp()
    {
		type = CV_32FC4;
        size = cv::Size(3000, 3000);

        mat1.create(size, type);
		oclmat1.create(size, type);

		cv::RNG& rng = cvtest::TS::ptr()->get_rng();
		mat2 = cvtest::randomMat(rng, size, CV_8UC1, 0.0, 1.5, false);
		oclmat2 = cv::ocl::oclMat(mat2);
    }
};

struct CopyToP : SetToP
{
	virtual void Run()
	{
		SetUp();
		clock_t start = clock();
		R_T2(
			oclmat1.copyTo( gpuRes, oclmat2 );
			if( COUNT_D ) gpuRes.download(dst);
		);
		
		if( SHOW_CPU )
		{
			start = clock();
			mat1.copyTo( dst, mat2 );
			std::cout<< "cv::copyTo -- " << clock() - start << "ms\n";
		}
	}
};

struct ConvertToP : ArithmTestP
{
	virtual void Run()
	{
		type = CV_32FC1;;
		SetUp();
		cv::RNG& rng = cvtest::TS::ptr()->get_rng();
		const double a = rng.uniform(0.0, 1.0);
		const double b = rng.uniform(-10.0, 10.0);

		int type2 = CV_32FC4;

		clock_t start = clock();
		R_T2(
			oclmat1.convertTo( gpuRes, type2 /*, a, b */ ); // fails when scaling factors a and b are specified
			if( COUNT_D ) gpuRes.download(dst);
		);
		
		if( SHOW_CPU )
		{
			start = clock();
			mat1.convertTo( dst, type2, a, b  );
			std::cout<< "cv::ConvertToP -- " << clock() - start << "ms\n";
		}
	}
};

////////////////////////////////////////////
// Filters

struct FilterTestP : PerfTest
{
protected:
	int ksize;
    int dx, dy;

    cv::Mat img_rgba;
    cv::Mat img_gray;

	cv::ocl::oclMat ocl_img_rgba;
    cv::ocl::oclMat ocl_img_gray;

	cv::ocl::oclMat dev_dst_rgba;
    cv::ocl::oclMat dev_dst_gray;

    cv::Mat dst_rgba;
    cv::Mat dst_gray;

	cv::Mat kernel;

	int bordertype;

    virtual void SetUp()
    {
		bordertype = (int)cv::BORDER_DEFAULT;
        ksize = 7;
        dx = ksize/2; dy = ksize/2;

		kernel = cv::Mat::ones(ksize, ksize, CV_8U);

        cv::Mat img = readImage(FILTER_TEST_IMAGE);
		ASSERT_FALSE(img.empty());

        cv::cvtColor(img, img_rgba, CV_BGR2BGRA);
        cv::cvtColor(img, img_gray, CV_BGR2GRAY);

		ocl_img_rgba = cv::ocl::oclMat(img_rgba);
		ocl_img_gray = cv::ocl::oclMat(img_gray);
    }
};

struct BlurP : FilterTestP
{
	virtual void Run()
	{
		SetUp();
		clock_t start = clock();
		R_T2(
			cv::ocl::blur(ocl_img_rgba, dev_dst_rgba, cv::Size(ksize, ksize), cv::Point(-1,-1), bordertype);
			cv::ocl::blur(ocl_img_gray, dev_dst_gray, cv::Size(ksize, ksize), cv::Point(-1,-1), bordertype);
			if( COUNT_D )
			{
				dev_dst_rgba.download(dst_rgba);
				dev_dst_gray.download(dst_gray);
			}
		);

		if( SHOW_CPU )
		{
			start = clock();
			cv::blur(img_rgba, dst_rgba, cv::Size(ksize, ksize), cv::Point(-1,-1), bordertype);
			cv::blur(img_gray, dst_gray, cv::Size(ksize, ksize), cv::Point(-1,-1), bordertype);
			std::cout<< "cv::blur -- " << clock() - start << "ms\n";
		}
	}
};

struct SobelP : FilterTestP
{
	virtual void Run()
	{
		SetUp();
		clock_t start = clock();
		R_T2(
			cv::ocl::Sobel(ocl_img_rgba, dev_dst_rgba, -1, dx, dy, ksize, 1, 0, bordertype);
			cv::ocl::Sobel(ocl_img_gray, dev_dst_gray, -1, dx, dy, ksize, 1, 0, bordertype);
			if( COUNT_D )
			{
				dev_dst_rgba.download(dst_rgba);
				dev_dst_gray.download(dst_gray);
			}
		);
		if( SHOW_CPU )
		{
			start = clock();
			cv::Sobel(img_rgba, dst_rgba, -1, dx, dy, ksize, 1, 0, bordertype);
			cv::Sobel(img_gray, dst_gray, -1, dx, dy, ksize, 1, 0, bordertype);
			std::cout<< "cv::SobelP -- " << clock() - start << "ms\n";
		}
	}
};

struct ScharrP : FilterTestP
{
	virtual void Run()
	{
		SetUp();
		dx = 0; dy = 1;
		clock_t start = clock();
		R_T2(
			cv::ocl::Scharr(ocl_img_rgba, dev_dst_rgba, -1, dx, dy, 1, 0, bordertype);
			cv::ocl::Scharr(ocl_img_gray, dev_dst_gray, -1, dx, dy, 1, 0, bordertype);
			if( COUNT_D )
			{
				dev_dst_rgba.download(dst_rgba);
				dev_dst_gray.download(dst_gray);
			}
		);

		if( SHOW_CPU )
		{
			start = clock();
			cv::Scharr(img_rgba, dst_rgba, -1, dx, dy, 1, 0, bordertype);
			cv::Scharr(img_gray, dst_gray, -1, dx, dy, 1, 0, bordertype);
			std::cout<< "cv::ScharrP -- " << clock() - start << "ms\n";
		}
	}
};

struct GaussianBlurP : FilterTestP
{
	virtual void Run()
	{
		double sigma1 = 3, sigma2 = 3;
		SetUp();
		clock_t start = clock();
		R_T2(
			cv::ocl::GaussianBlur(ocl_img_rgba, dev_dst_rgba, cv::Size(ksize, ksize), sigma1, sigma2);
			cv::ocl::GaussianBlur(ocl_img_gray, dev_dst_gray, cv::Size(ksize, ksize), sigma1, sigma2);
			if( COUNT_D )
			{
				dev_dst_rgba.download(dst_rgba);
				dev_dst_gray.download(dst_gray);
			}
		);
		if( SHOW_CPU )
		{
			start = clock();
			cv::GaussianBlur(img_rgba, dst_rgba, cv::Size(ksize, ksize), sigma1, sigma2);
			cv::GaussianBlur(img_gray, dst_gray, cv::Size(ksize, ksize), sigma1, sigma2);
			std::cout<< "cv::GaussianBlur -- " << clock() - start << "ms\n";
		}
	}
};

struct DilateP : FilterTestP
{
	virtual void Run()
	{
		SetUp();
		clock_t start = clock();
		R_T2(
			cv::ocl::dilate(ocl_img_rgba, dev_dst_rgba, kernel);
			cv::ocl::dilate(ocl_img_gray, dev_dst_gray, kernel);
			if( COUNT_D )
			{
				dev_dst_rgba.download(dst_rgba);
				dev_dst_gray.download(dst_gray);
			}
		);

		if( SHOW_CPU )
		{
			start = clock();
			cv::dilate(img_rgba, dst_rgba, kernel);
			cv::dilate(img_gray, dst_gray, kernel);
			std::cout<< "cv::DilateP -- " << clock() - start << "ms\n";
		}
	}
};

struct ErodeP : FilterTestP
{
	virtual void Run()
	{
		SetUp();
		clock_t start = clock();
		R_T2(
			cv::ocl::erode(ocl_img_rgba, dev_dst_rgba, kernel);
			cv::ocl::erode(ocl_img_gray, dev_dst_gray, kernel);
			if( COUNT_D )
			{
				dev_dst_rgba.download(dst_rgba);
				dev_dst_gray.download(dst_gray);
			}
		);

		if( SHOW_CPU )
		{
			start = clock();
			cv::erode(img_rgba, dst_rgba, kernel);
			cv::erode(img_gray, dst_gray, kernel);
			std::cout<< "cv::erode -- " << clock() - start << "ms\n";
		}
	}
};

struct MorphExP : FilterTestP
{
	virtual void Run()
	{
		SetUp();
		cv::ocl::oclMat okernel( kernel );
		clock_t start = clock();

		R_T2(
			cv::ocl::morphologyEx(ocl_img_rgba, dev_dst_rgba, 3, okernel);
			cv::ocl::morphologyEx(ocl_img_gray, dev_dst_gray, 3, okernel);

			if( COUNT_D )
			{
				dev_dst_rgba.download(dst_rgba);
				dev_dst_gray.download(dst_gray);
			}
		);
		
		if( SHOW_CPU )
		{
			start = clock();
			cv::morphologyEx(img_rgba, dst_rgba, 3, kernel);
			cv::morphologyEx(img_gray, dst_gray, 3, kernel);
			std::cout<< "cv::morphologyEx -- " << clock() - start << "ms\n";
		}
	}
};

////////////////////
// histograms
struct CalcHistP : PerfTest
{
	virtual void Run()
	{
		SetUp();
		clock_t start = clock();
		R_T2(
			cv::ocl::calcHist(oclmat, gpuRes);
			if( COUNT_D ) gpuRes.download(hist);
		);
		
		if( SHOW_CPU )
		{
			start = clock();
			//cv::calcHist(src, hist);
			std::cout<< "cv::CalcHistP -- " << clock() - start << "ms (not tested)\n";
		}
	}
protected:
	cv::Size size;
    cv::Mat src, hist;

	cv::ocl::oclMat oclmat;
	cv::ocl::oclMat gpuRes;

	virtual void SetUp()
	{
		cv::RNG& rng = cvtest::TS::ptr()->get_rng();
        size = cv::Size(3000, 3000);
		src = cvtest::randomMat(rng, size, CV_8UC1, 0, 255, false);
		oclmat = cv::ocl::oclMat( src );
	}
};

struct EqualizeHistP : CalcHistP
{
	virtual void Run()
	{
		SetUp();
		clock_t start = clock();
		R_T2(
			cv::ocl::equalizeHist(oclmat, gpuRes);
			if( COUNT_D ) gpuRes.download(hist);
		);
		
		R_T2(
			cv::equalizeHist(src, hist);
		);
		if(SHOW_CPU)
		{
			start = clock();
			cv::equalizeHist(src, hist);
			std::cout<< "cv::equalizeHist -- " << clock() - start << "ms\n";
		}
	}
};

struct ThresholdP : CalcHistP
{
	virtual void Run()
	{
		SetUp();
		int threshOp = (int)cv::THRESH_TOZERO_INV;;
		double maxVal = 200;
		double thresh = 125;

		clock_t start = clock();
		R_T2(
			cv::ocl::threshold(oclmat, gpuRes, thresh, maxVal, threshOp );
			if( COUNT_D ) gpuRes.download(hist);
		);
		
		if( SHOW_CPU )
		{
			start = clock();
			cv::threshold(src, hist, thresh, maxVal, threshOp );
			std::cout<< "cv::ThresholdP -- " << clock() - start << "ms\n";
		}
	}
};

struct ResizeP : ArithmTestP
{
	virtual void Run()
	{
		SetUp();

		clock_t start = clock();
		R_T2(
			cv::ocl::resize(oclmat1, gpuRes, cv::Size(), 2.0, 2.0);
			if( COUNT_D ) gpuRes.download(dst);
		);
		


		if( SHOW_CPU )
		{
			R_T2(
				cv::resize(mat1, dst, cv::Size(), 2.0, 2.0);
			);
		}
	}
};

struct CvtColorP : PerfTest
{
	virtual void Run()
	{

		SetUp();

		clock_t start = clock();
		R_T2(
			cv::ocl::cvtColor(oclmat, ocldst, cvtcode);
			if( COUNT_D ) ocldst.download(dst);
		);
	
		if( SHOW_CPU )
		{
			start = clock();
			cv::cvtColor(img, dst, cvtcode);
			std::cout<< "cv::cvtColor -- " << clock() - start << "ms\n";
		}
	}
protected:
	int type;
    int cvtcode;

    cv::Mat img, dst;
	cv::ocl::oclMat oclmat, ocldst;
	virtual void SetUp()
	{
		type = CV_8U;
		cvtcode = CV_BGR2GRAY;
		cv::Mat imgBase = readImage(FILTER_TEST_IMAGE);
		ASSERT_FALSE(imgBase.empty());

		imgBase.convertTo(img, type, type == CV_32F ? 1.0 / 255.0 : 1.0);
		oclmat = cv::ocl::oclMat( img );
	};
};

void testArithm()
{
	clock_t start = clock();
	std::cout << ">>>>>>>> Performance test started <<<<<<<<\n";
	{
		AddArrayP AddArrayP;
		AddArrayP.Run();
		SubtractArrayP subarray;
		subarray.Run();
		MultiplyArrayP MultiplyArrayP;
		MultiplyArrayP.Run();
		DivideArrayP DivideArrayP;
		DivideArrayP.Run();
	}
	std::cout.flush();
	{
		CompareP comp;
		comp.Run();
		MagnitudeP magnitude;
		magnitude.Run();
		LUTP lut;
		lut.Run();
		FlipP FlipP;
		FlipP.Run();
		MinMaxP minmax;
		minmax.Run();
		MinMaxLocP minmaxloc;
		minmaxloc.Run();
		CountNonZeroP cnz;
		cnz.Run();
		SumP sum;
		sum.Run();
	}
	std::cout.flush();
	{
		BitwiseNotP bn;
		bn.Run();
		BitwiseOrP bo;
		bo.Run();
		BitwiseAndP ba;
		ba.Run();
		BitwiseXorP bx;
		bx.Run();
	}
	std::cout.flush();
	{
		TransposeP transpose;
		transpose.Run();
		AbsdiffArrayP absdiff;
		absdiff.Run();
		SplitP split;
		split.Run();
		MergeP merge;
		merge.Run();
		SetToP setto;
		setto.Run();
		CopyToP copyto;
		copyto.Run();
		ConvertToP convertto;
		convertto.Run();
	}
	std::cout.flush();
	{
		BlurP blur;
		blur.Run();
		SobelP sobel;
		sobel.Run();
		ScharrP scharr;
		scharr.Run();
		GaussianBlurP gblur;
		gblur.Run();
		DilateP dilate;
		dilate.Run();
		ErodeP erode;
		erode.Run();
	}
	std::cout.flush();
	{
		MorphExP morphex;
		morphex.Run();
		CalcHistP calchist;
		calchist.Run();
		EqualizeHistP eqhist;
		eqhist.Run();
		ThresholdP threshold;
		threshold.Run();
		ResizeP resize;
		resize.Run();
		CvtColorP cvtcolor;
		cvtcolor.Run();
	}

	std::cout.flush();
	{
		//PhaseP phase;
		//phase.Run();
	}
	std::cout.flush();
	{
		CartToPolarP ctop;
		ctop.Run();
	}
	std::cout.flush();
	{
		PolarToCartP ptoc;
		ptoc.Run();
	}
	std::cout << ">>>>>>>> Performance test ended <<<<<<<<\ntotal - " << clock() - start << "ms\n";
	std::cout.flush();
}

int main( int, char** )
{
	print_info();
	cvtest::TS::ptr()->init("ocl");
	testArithm();

	return 0;
}

#endif // WITH_OPENCL

#endif // PREF_TEST_OCL
