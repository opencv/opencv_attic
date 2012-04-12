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
// Copyright (C) 2010-2012, Multicore Ware, Inc., all rights reserved.
// Copyright (C) 2010-2012, Advanced Micro Devices, Inc., all rights reserved.
// Third party copyrights are property of their respective owners.
//
// @Authors
//    Peng Xiao, pengxiao@multicorewareinc.com
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
#include <ctime>
#include <windows.h>

#if PREF_TEST_OCL

#ifdef HAVE_OPENCL

#define SHOW_CPU false
#define WARM_UP  15   
#define S_REPEAT 5
#define REPEAT   100
#define COUNT_U  1    // count the uploading execution time for ocl mat structures
#define COUNT_D  1


// the following macro section tests the target function (kernel) performance
// upload is the code snippet for converting cv::mat to cv::ocl::oclMat
// downloading is the code snippet for converting cv::ocl::oclMat back to cv::mat
// change COUNT_U and COUNT_D to take downloading and uploading time into account
#define P_TEST_FULL( upload, kernel_call, download ) \
	{ \
	std::cout<< "\n" #kernel_call "\n----------------------"; \
	{upload;} \
	R_TEST( kernel_call, WARM_UP ); \
	clock_t st = clock(); \
	R_T( { \
	if( COUNT_U ) {upload;} \
	kernel_call; \
	if( COUNT_D ) {download;} \
	} ); \
	std::cout<< clock() - st << "ms\n"; \
	}

#define P_TEST_SHORT_FULL( upload, kernel_call, download ) \
	{ \
	std::cout<< "\n" #kernel_call "\n----------------------"; \
	{upload;} \
	R_TEST( kernel_call, 1 ); \
	clock_t st = clock(); \
	R_TEST( { \
	if( COUNT_U ) {upload;} \
	kernel_call; \
	if( COUNT_D ) {download;} \
	},  S_REPEAT); \
	std::cout<< clock() - st << "ms\n"; \
	}


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

#define FILTER_TEST_IMAGE "C:/Windows/Web/Wallpaper/Landscapes/img9.jpg"
#define WARN_NRUN( name ) \
	std::cout << "Warning: " #name " is not runnable!\n";

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
	cv::ocl::oclMat oclRes, oclmat1, oclmat2;
	std::vector<cv::Mat> dstv;
protected:
	ArithmTestP() : type( CV_8UC4 ) {}
	void SetUp()
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
	void Run()
	{
		SetUp();
		P_TEST_FULL(
			oclmat1 = cv::ocl::oclMat(mat1);oclmat2 = cv::ocl::oclMat(mat2),
			cv::ocl::add(oclmat1, oclmat2, oclRes),
			oclRes.download(dst);
		);
	}
};

struct SubtractArrayP : ArithmTestP
{
	void Run()
	{
		SetUp();
		P_TEST_FULL(
			oclmat1 = cv::ocl::oclMat(mat1);oclmat2 = cv::ocl::oclMat(mat2),
			cv::ocl::subtract(oclmat1, oclmat2, oclRes),
			oclRes.download(dst);
		);
	}
};

struct MultiplyArrayP : ArithmTestP
{
	void Run()
	{
		SetUp();
		clock_t start = clock();
		P_TEST_FULL(
			oclmat1 = cv::ocl::oclMat(mat1);oclmat2 = cv::ocl::oclMat(mat2),
			cv::ocl::multiply(oclmat1, oclmat2, oclRes),
			oclRes.download(dst);
		);		
	}
};

struct DivideArrayP : ArithmTestP
{
	void Run()
	{
		SetUp();
		P_TEST_FULL(
			oclmat1 = cv::ocl::oclMat(mat1);oclmat2 = cv::ocl::oclMat(mat2),
			cv::ocl::divide(oclmat1, oclmat2, oclRes),
			oclRes.download(dst);
		);
	}
};

struct ExpP : ArithmTestP
{
	void Run()
	{
		type = CV_32FC1;
		SetUp();
		P_TEST_FULL(
			oclmat1 = cv::ocl::oclMat(mat1),
			cv::ocl::exp(oclmat1, oclRes),
			oclRes.download(dst);
		);
	}
};

struct LogP : ArithmTestP
{
	void Run()
	{
		type = CV_32FC1;
		SetUp();
		P_TEST_FULL(
			oclmat1 = cv::ocl::oclMat(mat1),
			cv::ocl::log(oclmat1, oclRes),
			oclRes.download(dst);
		);
	}
};

struct CompareP : ArithmTestP
{
	void Run()
	{
		type = CV_32FC1;
		SetUp();
		P_TEST_FULL(
			oclmat1 = cv::ocl::oclMat(mat1);oclmat2 = cv::ocl::oclMat(mat2),
			cv::ocl::compare(oclmat1, oclmat2, oclRes, cv::CMP_EQ),
			oclRes.download(dst);
		);
	}
};

struct FlipP : ArithmTestP
{
	void Run()
	{
		SetUp();
		P_TEST_FULL(
			oclmat1 = cv::ocl::oclMat(mat1),
			cv::ocl::flip(oclmat1, oclRes, 0),
			oclRes.download(dst);
		);
	}
protected:
	void SetUp()
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
	void Run()
	{
		type = CV_32F;
		SetUp();
		P_TEST_FULL(
			oclmat1 = cv::ocl::oclMat(mat1);oclmat2 = cv::ocl::oclMat(mat2),
			cv::ocl::magnitude(oclmat1, oclmat1, oclRes),
			oclRes.download(dst);
		);
	}
};

struct LUTP : ArithmTestP
{
	void Run()
	{
		SetUp();
		P_TEST_FULL(
			oclmat1 = cv::ocl::oclMat(mat1);ocllut = cv::ocl::oclMat(lut),
			cv::ocl::LUT(oclmat1, ocllut, oclRes),
			oclRes.download(dst);
		);
	}
protected:
	cv::Mat lut;
	cv::ocl::oclMat ocllut;
	void SetUp()
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

	void Run()
	{
		SetUp();
		P_TEST_FULL(
			oclmat1 = cv::ocl::oclMat(mat1);oclmat2 = cv::ocl::oclMat(mat2),
			cv::ocl::minMax(oclmat1, &minVal, &maxVal, oclmat2),
		{};
		);
	}

protected:
	void SetUp()
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
	void Run()
	{
		SetUp();
		P_TEST_FULL(
			oclmat1 = cv::ocl::oclMat(mat1);oclmat2 = cv::ocl::oclMat(mat2),
			cv::ocl::minMaxLoc(oclmat1, &minVal, &maxVal, &minLoc_gold, &maxLoc_gold, oclmat2),
		{}
		);
	}
};

struct CountNonZeroP : ArithmTestP
{
	int n;
	void Run()
	{
		SetUp();
		P_TEST_FULL(
			oclmat1 = cv::ocl::oclMat(mat1),
			n = cv::ocl::countNonZero(oclmat1),
		{}
		);
	}
protected:
	void SetUp()
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
	void Run()
	{
		SetUp();
		cv::Scalar n;
		P_TEST_FULL(
			oclmat1 = cv::ocl::oclMat(mat1),
			n = cv::ocl::sum(oclmat1),
		{}
		);
	}
};

struct BitwiseP : ArithmTestP
{
protected:
	void SetUp()
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
	void Run()
	{
		SetUp();
		P_TEST_FULL(
			oclmat1 = cv::ocl::oclMat(mat1),
			cv::ocl::bitwise_not(oclmat1, oclRes),
			oclRes.download(dst)
			);
	}
};

struct BitwiseAndP : BitwiseP
{
	void Run()
	{
		SetUp();
		P_TEST_FULL(
			oclmat1 = cv::ocl::oclMat(mat1);oclmat2 = cv::ocl::oclMat(mat2),
			cv::ocl::bitwise_and(oclmat1, oclmat2, oclRes),
			oclRes.download(dst)
			);
	}
};

struct BitwiseXorP : BitwiseP
{
	void Run()
	{
		SetUp();
		P_TEST_FULL(
			oclmat1 = cv::ocl::oclMat(mat1);oclmat2 = cv::ocl::oclMat(mat2),
			cv::ocl::bitwise_xor(oclmat1, oclmat2, oclRes),
			oclRes.download(dst)
			);

	}
};

struct BitwiseOrP : BitwiseP
{
	void Run()
	{
		SetUp();
		P_TEST_FULL(
			oclmat1 = cv::ocl::oclMat(mat1);oclmat2 = cv::ocl::oclMat(mat2),
			cv::ocl::bitwise_or(oclmat1, oclmat2, oclRes),
			oclRes.download(dst)
			);
	}
};

struct TransposeP : ArithmTestP
{
	void Run()
	{
		SetUp();
		P_TEST_FULL(
			oclmat1 = cv::ocl::oclMat(mat1),
			cv::ocl::transpose(oclmat1, oclRes),
			oclRes.download(dst)
			);
	}
};

struct AbsdiffArrayP : ArithmTestP
{
	void Run()
	{
		type = CV_32FC1;
		SetUp();
		P_TEST_FULL(
			oclmat1 = cv::ocl::oclMat(mat1);oclmat2 = cv::ocl::oclMat(mat2),
			cv::ocl::absdiff(oclmat1, oclmat2, oclRes),
			oclRes.download(dst)
			);
	}
};

struct PhaseP : ArithmTestP
{
	void Run()
	{
		type = CV_32F;
		SetUp();
		P_TEST_FULL(
			oclmat1 = cv::ocl::oclMat(mat1);oclmat2 = cv::ocl::oclMat(mat2),
			cv::ocl::phase(oclmat1,oclmat2,oclRes,1),
			oclRes.download(dst)
			);
	}
};

struct CartToPolarP : ArithmTestP
{
	cv::ocl::oclMat oclRes1;
	void Run()
	{
		type = CV_32FC1;
		SetUp();
		clock_t start = clock();
		P_TEST_FULL(
			oclmat1 = cv::ocl::oclMat(mat1);oclmat2 = cv::ocl::oclMat(mat2),
			cv::ocl::cartToPolar(oclmat1,oclmat2,oclRes, oclRes1, 1),
			oclRes.download(dst);oclRes1.download(dst)
			);
	}
};

struct PolarToCartP : ArithmTestP
{
	cv::ocl::oclMat oclRes1;
	void Run()
	{
		type = CV_32FC1;
		SetUp();
		clock_t start = clock();
		P_TEST_FULL(
			oclmat1 = cv::ocl::oclMat(mat1);oclmat2 = cv::ocl::oclMat(mat2),
			cv::ocl::polarToCart(oclmat1,oclmat2,oclRes, oclRes1, 1),
			oclRes.download(dst);oclRes1.download(dst);
		);
	}
};

///////////////////////////////////////
// split & merge
struct SplitP : ArithmTestP
{
	void Run()
	{
		SetUp();
		P_TEST_FULL(
			oclmat1 = cv::ocl::oclMat(mat1),
			cv::ocl::split(oclmat1, dev_dst),
		{			
			dstv.resize(dev_dst.size());
			for (size_t i = 0; i < dev_dst.size(); ++i)
			{
				dev_dst[i].download(dstv[i]);
			}
		}
		);
	}
protected:
	std::vector<cv::ocl::oclMat> dev_dst;
	void SetUp()
	{
		size = cv::Size( 3000, 3000 );

		mat1.create(size, type);
		mat1.setTo(cv::Scalar(1.0, 2.0, 3.0, 4.0));

		oclmat1 = cv::ocl::oclMat(mat1);
	}
};

struct MergeP : SplitP
{
	void Run()
	{
		SetUp();
		cv::ocl::split(oclmat1, dev_dst);
		cv::split(mat1, dstv);
		P_TEST_FULL(
			oclmat1 = cv::ocl::oclMat(mat1),
			cv::ocl::merge(dev_dst, oclmat2),
			oclmat2.download(dst)
			);
	}
};

struct SetToP : ArithmTestP
{
	void Run()
	{
		SetUp();
		static cv::Scalar s = cv::Scalar(1, 2, 3, 4);
		P_TEST_FULL(
			oclmat2 = cv::ocl::oclMat(mat2),
			oclmat1.setTo( s, oclmat2 ),
			oclmat1.download(dst)
			);
	}
protected:
	void SetUp()
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
	void Run()
	{
		SetUp();
		P_TEST_FULL(
			oclmat1 = cv::ocl::oclMat(mat1),
			oclmat1.copyTo( oclRes, oclmat2 ),
			oclRes.download(dst)
			);
	}
};

struct ConvertToP : ArithmTestP
{
	void Run()
	{
		type = CV_32FC1;;
		SetUp();
		cv::RNG& rng = cvtest::TS::ptr()->get_rng();
		const double a = rng.uniform(0.0, 1.0);
		const double b = rng.uniform(-10.0, 10.0);

		int type2 = CV_32FC4;

		P_TEST_FULL(
			oclmat1 = cv::ocl::oclMat(mat1),
			oclmat1.convertTo( oclRes, type2 /*, a, b */ ), // fails when scaling factors a and b are specified
			oclRes.download(dst)
			);
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

	void SetUp()
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
	void Run()
	{
		SetUp();
		P_TEST_FULL(
		{
			ocl_img_rgba = cv::ocl::oclMat(img_rgba);
			ocl_img_gray = cv::ocl::oclMat(img_gray);
		},
		{
			cv::ocl::blur(ocl_img_rgba, dev_dst_rgba, cv::Size(ksize, ksize), cv::Point(-1,-1));
			cv::ocl::blur(ocl_img_gray, dev_dst_gray, cv::Size(ksize, ksize), cv::Point(-1,-1));
		},
		{
			dev_dst_rgba.download(dst_rgba);
			dev_dst_gray.download(dst_gray);
		});
	}
};

struct SobelP : FilterTestP
{
	void Run()
	{
		SetUp();
		P_TEST_FULL(
		{
			ocl_img_rgba = cv::ocl::oclMat(img_rgba);
			ocl_img_gray = cv::ocl::oclMat(img_gray);
		},
		{
			cv::ocl::Sobel(ocl_img_rgba, dev_dst_rgba, -1, dx, dy, ksize, 1, 0);
			cv::ocl::Sobel(ocl_img_gray, dev_dst_gray, -1, dx, dy, ksize, 1, 0);
		},
		{
			dev_dst_rgba.download(dst_rgba);
			dev_dst_gray.download(dst_gray);
		});
	}
};

struct ScharrP : FilterTestP
{
	void Run()
	{
		SetUp();
		dx = 0; dy = 1;
		P_TEST_FULL(
		{
			ocl_img_rgba = cv::ocl::oclMat(img_rgba);
			ocl_img_gray = cv::ocl::oclMat(img_gray);
		},
		{
			cv::ocl::Scharr(ocl_img_rgba, dev_dst_rgba, -1, dx, dy, 1, 0);
			cv::ocl::Scharr(ocl_img_gray, dev_dst_gray, -1, dx, dy, 1, 0);
		},
		{
			dev_dst_rgba.download(dst_rgba);
			dev_dst_gray.download(dst_gray);
		});
	}
};

struct GaussianBlurP : FilterTestP
{
	void Run()
	{
		double sigma1 = 3, sigma2 = 3;
		SetUp();
		P_TEST_FULL(
		{
			ocl_img_rgba = cv::ocl::oclMat(img_rgba);
			ocl_img_gray = cv::ocl::oclMat(img_gray);
		},
		{
			cv::ocl::GaussianBlur(ocl_img_rgba, dev_dst_rgba, cv::Size(ksize, ksize), sigma1, sigma2);
			cv::ocl::GaussianBlur(ocl_img_gray, dev_dst_gray, cv::Size(ksize, ksize), sigma1, sigma2);
		},
		{
			dev_dst_rgba.download(dst_rgba);
			dev_dst_gray.download(dst_gray);
		});
	}
};

struct DilateP : FilterTestP
{
	void Run()
	{
		SetUp();
		P_TEST_FULL(
		{
			ocl_img_rgba = cv::ocl::oclMat(img_rgba);
			ocl_img_gray = cv::ocl::oclMat(img_gray);
		},
		{
			cv::ocl::dilate(ocl_img_rgba, dev_dst_rgba, kernel);
			cv::ocl::dilate(ocl_img_gray, dev_dst_gray, kernel);
		},
		{
			dev_dst_rgba.download(dst_rgba);
			dev_dst_gray.download(dst_gray);
		});
	}
};

struct ErodeP : FilterTestP
{
	void Run()
	{
		SetUp();
		P_TEST_FULL(
		{
			ocl_img_rgba = cv::ocl::oclMat(img_rgba);
			ocl_img_gray = cv::ocl::oclMat(img_gray);
		},
		{
			cv::ocl::erode(ocl_img_rgba, dev_dst_rgba, kernel);
			cv::ocl::erode(ocl_img_gray, dev_dst_gray, kernel);
		},
		{
			dev_dst_rgba.download(dst_rgba);
			dev_dst_gray.download(dst_gray);
		});
	}
};

struct MorphExP : FilterTestP
{
	void Run()
	{
		SetUp();
		cv::ocl::oclMat okernel;
		P_TEST_FULL(
		{
			ocl_img_rgba = cv::ocl::oclMat(img_rgba);
			ocl_img_gray = cv::ocl::oclMat(img_gray);
		},
		{
			cv::ocl::morphologyEx(ocl_img_rgba, dev_dst_rgba, 3, kernel);
			cv::ocl::morphologyEx(ocl_img_gray, dev_dst_gray, 3, kernel);
		},
		{
			dev_dst_rgba.download(dst_rgba);
			dev_dst_gray.download(dst_gray);
		});
	}
};
struct LaplacianP : FilterTestP
{
	void Run()
	{
		SetUp();
		P_TEST_FULL(
		{
			ocl_img_rgba = cv::ocl::oclMat(img_rgba);
			ocl_img_gray = cv::ocl::oclMat(img_gray);
		},
		{
			cv::ocl::Laplacian(ocl_img_rgba, dev_dst_rgba, -1, 3 );
			cv::ocl::Laplacian(ocl_img_gray, dev_dst_gray, -1, 3 );
		},
		{
			dev_dst_rgba.download(dst_rgba);
			dev_dst_gray.download(dst_gray);
		});
	}
};

////////////////////
// histograms
struct CalcHistP : PerfTest
{
	void Run()
	{
		SetUp();
		P_TEST_FULL(
			oclmat = cv::ocl::oclMat( src ),
			cv::ocl::calcHist(oclmat, oclRes),
			oclRes.download(hist)
			);
	}
protected:
	cv::Size size;
	cv::Mat src, hist;

	cv::ocl::oclMat oclmat;
	cv::ocl::oclMat oclRes;

	void SetUp()
	{
		cv::RNG& rng = cvtest::TS::ptr()->get_rng();
		size = cv::Size(3000, 3000);
		src = cvtest::randomMat(rng, size, CV_8UC1, 0, 255, false);
		oclmat = cv::ocl::oclMat( src );
	}
};

struct EqualizeHistP : CalcHistP
{
	void Run()
	{
		SetUp();
		P_TEST_FULL(
			oclmat = cv::ocl::oclMat( src ),
			cv::ocl::equalizeHist(oclmat, oclRes),
			oclRes.download(hist)
			);
	}
};

struct ThresholdP : CalcHistP
{
	void Run()
	{
		SetUp();
		int threshOp = (int)cv::THRESH_TOZERO_INV;;
		double maxVal = 200;
		double thresh = 125;

		clock_t start = clock();

		P_TEST_FULL(
			oclmat = cv::ocl::oclMat( src ),
			cv::ocl::threshold(oclmat, oclRes, thresh, maxVal, threshOp ),
			oclRes.download(hist)
			);
	}
};

struct ResizeP : ArithmTestP
{
	void Run()
	{
		SetUp();
		P_TEST_FULL(
			oclmat1 = cv::ocl::oclMat( mat1 ),
			cv::ocl::resize(oclmat1, oclRes, cv::Size(), 2.0, 2.0),
			oclRes.download(dst)
			);
	}
};

struct CvtColorP : PerfTest
{
	void Run()
	{
		SetUp();
		P_TEST_FULL(
			oclmat = cv::ocl::oclMat( img ),
			cv::ocl::cvtColor(oclmat, ocldst, cvtcode),
			ocldst.download(dst)
			);
	}
protected:
	int type;
	int cvtcode;

	cv::Mat img, dst;
	cv::ocl::oclMat oclmat, ocldst;
	void SetUp()
	{
		type = CV_8U;
		cvtcode = CV_BGR2GRAY;
		cv::Mat imgBase = readImage(FILTER_TEST_IMAGE);
		ASSERT_FALSE(imgBase.empty());

		imgBase.convertTo(img, type, type == CV_32F ? 1.0 / 255.0 : 1.0);
		oclmat = cv::ocl::oclMat( img );
	}
};


struct WarpAffineP : ArithmTestP
{
	void Run()
	{
		SetUp();
		const double aplha = CV_PI / 4;
		double mat[2][3] = { {std::cos(aplha), -std::sin(aplha), mat1.cols / 2},
		{std::sin(aplha),  std::cos(aplha), 0}};
		cv::Mat M(2, 3, CV_64F, (void*) mat);

		P_TEST_FULL(
			oclmat1 = cv::ocl::oclMat( mat1 ),
			cv::ocl::warpAffine( oclmat1, oclRes, M, cv::Size(1500, 1500) ),
			oclRes.download(dst)
			);
	}
};

struct WarpPerspectiveP : ArithmTestP
{
	void Run()
	{
		SetUp();
		const double aplha = CV_PI / 4;
		double mat[3][3] = { {std::cos(aplha), -std::sin(aplha), mat1.cols / 2},
		{std::sin(aplha),  std::cos(aplha), 0},
		{0.0,              0.0,             1.0}};
		cv::Mat M(3, 3, CV_64F, (void*) mat);

		P_TEST_FULL(
			oclmat1 = cv::ocl::oclMat( mat1 ),
			cv::ocl::warpPerspective( oclmat1, oclRes, M, cv::Size(1500, 1500) ),
			oclRes.download(dst)
		);
	}
};


struct CornerHarrisP : FilterTestP
{
	void Run()
	{
		SetUp();
		bordertype = 2;
		P_TEST_FULL(
		{
			ocl_img_gray = cv::ocl::oclMat(img_gray);
		},
		{
			cv::ocl::cornerHarris(ocl_img_gray, dev_dst_gray, 3, ksize, 0.5, bordertype );
		},
		{
			dev_dst_gray.download(dst_gray);
		});
	}
};

void testArithm()
{
	clock_t start = clock();
	std::cout << ">>>>>>>> Performance test started <<<<<<<<\n";
#if 0
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
#endif // 
	{
		LogP log;
		log.Run();
		ExpP exp;
		exp.Run();
	} 
	std::cout.flush();
	{
		PhaseP phase;
		phase.Run();
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

	{
		WarpAffineP warpA;
		warpA.Run();
		WarpPerspectiveP warpP;
		warpP.Run();	
	}

	{
		CornerHarrisP ch;
		ch.Run();
	}
	
	{
		LaplacianP laplacian;
		lplacian.Run();
	}
	std::cout.flush();
	std::cout << ">>>>>>>> Performance test ended <<<<<<<<\ntotal - " << clock() - start << "ms\n";
	std::cout.flush();
}

int main( int, char** )
{
	cvtest::TS::ptr()->init("ocl");
	testArithm();

	return 0;
}

#endif // WITH_OPENCL

#endif // PREF_TEST_ocl
