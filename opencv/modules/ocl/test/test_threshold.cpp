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


int testThresholdBinary(const Mat& src, float thresh, float maxval);

static int verifyResult(const Mat& a, const Mat& b);

int test_threshold() {

    Size size(1024,1024);
    Mat src_8u, src_32f;
        
	RNG& rng = TS::ptr()->get_rng();
        
    src_8u = randomMat(rng, size, CV_8UC1, 1, 255, false);
	src_32f = randomMat(rng, size, CV_32FC1, 0, 1, false);

	int status;

	float thresh = 100;
	float  maxval = 1;

	status = testThresholdBinary(src_8u, thresh, maxval);
	if(status != 0){ printf("Threshold test (8u) failed \n"); }
	if(status == 0){ printf("Threshold test (8u) passed\n"); }

	status = testThresholdBinary(src_32f, thresh, maxval);
	if(status != 0){ printf("Threshold test (32f) failed \n"); }
	if(status == 0){ printf("Threshold test (32f) passed\n"); }
}

int testThresholdBinary(const Mat& src, float thresh, float maxval){

	Mat dst_gold;

	threshold(src, dst_gold, thresh, maxval, CV_THRESH_BINARY);
	
	Mat dst(src.rows, src.cols, src.type());
	OclMat oclDst(src.rows, src.cols, src.type());

	threshold(OclMat(src), oclDst, thresh, maxval, CV_THRESH_BINARY);

	oclDst.download(dst);

	int s = verifyResult(dst_gold, dst);

	oclDst.release();
	dst_gold.release();
	dst.release();
	
	return s;
}

static int verifyResult(const Mat& a, const Mat& b){

	int count = a.rows*a.cols*a.channels();

	for(int i=0;i<count;i++)
		if(a.data[i] != b.data[i])
			return -1;

	return 0;
}