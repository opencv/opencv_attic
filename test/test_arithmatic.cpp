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

using namespace cv;
using namespace cv::ocl;
using namespace cvtest;


int testAdd(const Mat& a, const Mat& b);
int testAdd(const Mat& a, const Scalar& val);

int testSub(const Mat& a, const Mat& b);
int testSub(const Mat& a, const Scalar& val);

int testMul(const Mat& a, const Mat& b);
int testMul(const Mat& a, const Scalar& val);

int testDiv(const Mat& a, const Mat& b);
int testDiv(const Mat& a, const Scalar& val);

int testMin(const Mat& a, const Mat& b);
int testMin(const Mat& a, const Scalar& val);

int testMax(const Mat& a, const Mat& b);
int testMax(const Mat& a, const Scalar& val);

int verifyResult(const Mat& a, const Mat& b);

int main( int argc, char** argv ){

    Size size(1024,1024);
    Mat mat1_8u, mat2_8u, mat1_16u, mat2_16u, mat1_32f, mat2_32f;
        
	RNG& rng = TS::ptr()->get_rng();
        
    mat1_8u = randomMat(rng, size, CV_8UC1, 1, 255, false);
    mat2_8u = randomMat(rng, size, CV_8UC1, 1, 255, false);

	mat1_16u = randomMat(rng, size, CV_16UC1, 1, 65535, false);
	mat2_16u = randomMat(rng, size, CV_16UC1, 1, 65535, false);

	mat1_32f = randomMat(rng, size, CV_32FC1, 0, 1, false);
	mat2_32f = randomMat(rng, size, CV_32FC1, 0, 1, false);

	Scalar val(mat1_32f.data[0]);

	int status;

	////////////////////////////////////ADD TEST////////////////////////////////////
	//! Test 8bit image addition
	status = testAdd(mat1_8u, mat2_8u);
	if(status != 0){ printf("Add test (8u) failed \n"); }
	if(status == 0){ printf("Add test (8u) passed\n"); }

	//! Test 16bit image addition
	status = testAdd(mat1_16u, mat2_16u);
	if(status != 0){ printf("Add test (16u) failed \n"); }
	if(status == 0){ printf("Add test (16u) passed\n"); }
	
	//! Test 32bit floating image addition
	status = testAdd(mat1_32f, mat2_32f);
	if(status != 0){ printf("Add test (32f) failed \n"); }
	if(status == 0){ printf("Add test (32f) passed\n"); }

	//! Test 32bit floating image addition (with scalar value)
	status = testAdd(mat1_32f, val);
	if(status != 0){ printf("Add test (32f) failed \n\n"); }
	if(status == 0){ printf("Add test (32f) passed\n\n"); }


	////////////////////////////////////SUBTRACT TEST////////////////////////////////////
	//! Test 8bit image subtraction
	status = testSub(mat1_8u, mat2_8u);
	if(status != 0){ printf("Sub test (8u) failed \n"); }
	if(status == 0){ printf("Sub test (8u) passed\n"); }

	//! Test 16bit image subtraction
	status = testSub(mat1_16u, mat2_16u);
	if(status != 0){ printf("Sub test (16u) failed \n"); }
	if(status == 0){ printf("Sub test (16u) passed\n"); }
	
	//! Test 32bit floating image subtraction
	status = testSub(mat1_32f, mat2_32f);
	if(status != 0){ printf("Sub test (32f) failed \n"); }
	if(status == 0){ printf("Sub test (32f) passed\n"); }

	//! Test 32bit floating image subtraction (with Scalar value)
	status = testSub(mat1_32f, val);
	if(status != 0){ printf("Sub test (32f) failed \n\n"); }
	if(status == 0){ printf("Sub test (32f) passed\n\n"); }


	////////////////////////////////////MULTIPLY TEST////////////////////////////////////
	//! Test 8bit image multiplication
	status = testMul(mat1_8u, mat2_8u);
	if(status != 0){ printf("Mul test (8u) failed \n"); }
	if(status == 0){ printf("Mul test (8u) passed\n"); }

	//! Test 16bit image multiplication
	status = testMul(mat1_16u, mat2_16u);
	if(status != 0){ printf("Mul test (16u) failed \n"); }
	if(status == 0){ printf("Mul test (16u) passed\n"); }
	
	//! Test 32bit floating image multiplication
	status = testMul(mat1_32f, mat2_32f);
	if(status != 0){ printf("Mul test (32f) failed \n"); }
	if(status == 0){ printf("Mul test (32f) passed\n"); }

	//! Test 32bit floating image multiplication (with Scalar value)
	status = testMul(mat1_32f, val);
	if(status != 0){ printf("Mul test (32f) failed \n\n"); }
	if(status == 0){ printf("Mul test (32f) passed\n\n"); }


	////////////////////////////////////DIVIDE TEST////////////////////////////////////
	//! Test 8bit image division
	status = testDiv(mat1_8u, mat2_8u);
	if(status != 0){ printf("Div test (8u) failed \n"); }
	if(status == 0){ printf("Div test (8u) passed\n"); }

	//! Test 16bit image division
	status = testDiv(mat1_16u, mat2_16u);
	if(status != 0){ printf("Div test (16u) failed \n"); }
	if(status == 0){ printf("Div test (16u) passed\n"); }
	
	//! Test 32bit floating image division
	status = testDiv(mat1_32f, mat2_32f);
	if(status != 0){ printf("Div test (32f) failed \n"); }
	if(status == 0){ printf("Div test (32f) passed\n"); }

	//! Test 32bit floating image division (with Scalar value)
	status = testDiv(mat1_32f, val);
	if(status != 0){ printf("Div test (32f) failed \n\n"); }
	if(status == 0){ printf("Div test (32f) passed\n\n"); }

	////////////////////////////////////MIN TEST////////////////////////////////////
	//! Test 8bit image division
	status = testMin(mat1_8u, mat2_8u);
	if(status != 0){ printf("Min test (8u) failed \n"); }
	if(status == 0){ printf("Min test (8u) passed\n"); }

	//! Test 16bit image division
	status = testMin(mat1_16u, mat2_16u);
	if(status != 0){ printf("Min test (16u) failed \n"); }
	if(status == 0){ printf("Min test (16u) passed\n"); }
	
	//! Test 32bit floating image division
	status = testMin(mat1_32f, mat2_32f);
	if(status != 0){ printf("Min test (32f) failed \n"); }
	if(status == 0){ printf("Min test (32f) passed\n"); }

	//! Test 32bit floating image division (with Scalar value)
	status = testMin(mat1_32f, val);
	if(status != 0){ printf("Min test (32f) failed \n\n"); }
	if(status == 0){ printf("Min test (32f) passed\n\n"); }

}

int testAdd(const Mat& a, const Mat& b){

	Mat dst_gold;
	add(a, b, dst_gold);
	
	Mat dst;
	OclMat oclRes(a.rows, a.cols, a.type());

	add(OclMat(a), OclMat(b), oclRes);
	oclRes.download(dst);

	int s = verifyResult(dst_gold, dst);

	oclRes.release();
	dst_gold.release();
	dst.release();
	
	return s;
}

int testAdd(const Mat& a, const Scalar& val){

	Mat dst_gold;
	add(a, val, dst_gold);

	Mat dst;
	OclMat oclRes(a.rows, a.cols, a.type());

	add(OclMat(a), val, oclRes);
	oclRes.download(dst);

	int s = verifyResult(dst_gold, dst);

	oclRes.release();
	dst_gold.release();
	dst.release();
	
	return s;
}

int testSub(const Mat& a, const Mat& b){

	Mat dst_gold;
	subtract(a, b, dst_gold);

	Mat dst;
	OclMat oclRes(a.rows, a.cols, a.type());

	subtract(OclMat(a), OclMat(b), oclRes);
	oclRes.download(dst);

	int s = verifyResult(dst_gold, dst);

	oclRes.release();
	dst_gold.release();
	dst.release();

	return s;
}

int testSub(const Mat& a, const Scalar& val){

	Mat dst_gold;
	subtract(a, val, dst_gold);

	Mat dst;
	OclMat oclRes(a.rows, a.cols, a.type());

	subtract(OclMat(a), val, oclRes);
	oclRes.download(dst);

	int s = verifyResult(dst_gold, dst);

	oclRes.release();
	dst_gold.release();
	dst.release();

	return s;
}

int testMul(const Mat& a, const Mat& b){

	Mat dst_gold;
	multiply(a, b, dst_gold);
	
	Mat dst;
	OclMat oclRes(a.rows, a.cols, a.type());

	multiply(OclMat(a), OclMat(b), oclRes);
	oclRes.download(dst);

	int s = verifyResult(dst_gold, dst);

	oclRes.release();
	dst_gold.release();
	dst.release();
	
	return s;
}

int testMul(const Mat& a, const Scalar& val){

	Mat dst_gold;
	multiply(a, val, dst_gold);

	Mat dst;
	OclMat oclRes(a.rows, a.cols, a.type());

	multiply(OclMat(a), val, oclRes);
	oclRes.download(dst);

	int s = verifyResult(dst_gold, dst);

	oclRes.release();
	dst_gold.release();
	dst.release();
	
	return s;
}

int testDiv(const Mat& a, const Mat& b){

	Mat dst_gold;
	divide(a, b, dst_gold);
	
	Mat dst;
	OclMat oclRes(a.rows, a.cols, a.type());

	divide(OclMat(a), OclMat(b), oclRes);
	oclRes.download(dst);

	int s = verifyResult(dst_gold, dst);

	oclRes.release();
	dst_gold.release();
	dst.release();
	
	return s;
}

int testDiv(const Mat& a, const Scalar& val){

	Mat dst_gold;
	divide(a, val, dst_gold);

	Mat dst;
	OclMat oclRes(a.rows, a.cols, a.type());

	divide(OclMat(a), val, oclRes);
	oclRes.download(dst);

	int s = verifyResult(dst_gold, dst);

	oclRes.release();
	dst_gold.release();
	dst.release();
	
	return s;
}

int testMin(const Mat& a, const Mat& b){

	Mat dst_gold(a.rows, a.cols, a.type());
	
	int count = a.cols*a.rows;

	for(int i=0; i<count; i++)
		dst_gold.data[i] = (a.data[i] < b.data[i])?a.data[i]:b.data[i];
	
	Mat dst;
	OclMat oclRes(a.rows, a.cols, a.type());

	min(OclMat(a), OclMat(b), oclRes);
	oclRes.download(dst);

	int s = verifyResult(dst_gold, dst);

	oclRes.release();
	dst_gold.release();
	dst.release();
	
	return s;
}

int testMin(const Mat& a, const Scalar& val){

	Mat dst_gold(a.rows, a.cols, a.type());
	int count = a.cols*a.rows*a.channels();
	for(int i=0; i<count; i++)
		dst_gold.data[i] = (a.data[i] < val[0])?a.data[i]:val[0];

	Mat dst;
	OclMat oclRes(a.rows, a.cols, a.type());

	min(OclMat(a), val, oclRes);
	oclRes.download(dst);

	int s = verifyResult(dst_gold, dst);

	oclRes.release();
	dst_gold.release();
	dst.release();
	
	return s;
}

int testMax(const Mat& a, const Mat& b){

	Mat dst_gold(a.rows, a.cols, a.type());
	int count = a.cols*a.rows*a.channels();
	for(int i=0; i<count; i++)
		dst_gold.data[i] = (a.data[i] > b.data[i])?a.data[i]:b.data[i];
	
	Mat dst;
	OclMat oclRes(a.rows, a.cols, a.type());

	max(OclMat(a), OclMat(b), oclRes);
	oclRes.download(dst);

	int s = verifyResult(dst_gold, dst);

	oclRes.release();
	dst_gold.release();
	dst.release();
	
	return s;
}

int testMax(const Mat& a, const Scalar& val){

	Mat dst_gold(a.rows, a.cols, a.type());
	int count = a.cols*a.rows*a.channels();
	for(int i=0; i<count; i++)
		dst_gold.data[i] = (a.data[i] > val[0])?a.data[i]:val[0];

	Mat dst;
	OclMat oclRes(a.rows, a.cols, a.type());

	max(OclMat(a), val, oclRes);
	oclRes.download(dst);

	int s = verifyResult(dst_gold, dst);

	oclRes.release();
	dst_gold.release();
	dst.release();
	
	return s;
}

int verifyResult(const Mat& a, const Mat& b){

	int count = a.rows*a.cols*a.channels();

	for(int i=0;i<count;i++)
		if(a.data[i] != b.data[i])
			return -1;

	return 0;
}