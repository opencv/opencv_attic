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

#include "ocl.h"
#include <iostream>
using namespace std;

using namespace cv::ocl;

int main(){


	cv::ocl::init();
	
	//Cretae 2 input frames for Optical Flow
	OclMat m1(1024,1024,CV_8UC1);
	OclMat m2(1024,1024,CV_8UC1);

	//Create 2 images for storing Horizontal and Vertical components
	OclMat u(1024,1024,CV_32FC1);
	OclMat v(1024,1024,CV_32FC1);

	//Window Size for OF computation
	CvSize winSize = {3,3};

	//Element count
	int elements = m1.cols*m1.rows*m1.channels;
	unsigned char* ptr1 = (unsigned char*)malloc(elements);
	unsigned char* ptr2 = (unsigned char*)malloc(elements);
	float* ptr3 = (float*)malloc(elements*4);
	float* ptr4 = (float*)malloc(elements*4);
	memset(ptr1, 10, elements);
	memset(ptr2, 5, elements);
	for(int i=0;i<elements;i++){
		ptr3[i] = 0;
		ptr4[i] = 0;
	}

	m1._upload(elements, ptr1);
	m2._upload(elements, ptr2);
	u._upload(elements*4, ptr3);
	v._upload(elements*4, ptr3);

	cv::ocl::opticalFlowLK(m1, m2, winSize, u, v);

	u._download(elements*4, ptr4);

	m1.release();
	m2.release();
	u.release();
	v.release();

	cout<<"OpticalFlow test performed...\n";
}