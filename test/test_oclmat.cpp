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

#include "ocl.hpp"
#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <iostream>
using namespace std;

using namespace cv;
using namespace cv::ocl;


/*
This test includes the following:
	
	1. Creating OclMat objects of types:
										8bit-single channel
										16bit-single channel
										32bit (floating)-single channel
	2. Uploading and downloading data from raw pointers
	3. Uploading and download data from cv::Mat
*/

int main(){

	//OclMat m1(1024,1024,CV_8UC1);
	//OclMat m2(1024,1024,CV_16UC1);
	//OclMat m3(1024,1024,CV_32FC1);

	Scalar s(20);
	OclMat m4(1024,1024,CV_8UC1); m4 = s;
	OclMat m5(1024,1024,CV_8UC1); m5 = s;
	OclMat m6(1024,1024,CV_8UC1);

	Mat temp2(1024,1024, CV_8UC1);

	cv::ocl::add(m4,m5,m6);

	//temp.release();
	temp2.release();
	m4.release();
	m5.release();
	m6.release();
/*	int elements = m1.cols*m1.rows*m1.channels;
	unsigned char* ptr1 = (unsigned char*)malloc(elements);
	unsigned short* ptr2 = (unsigned short*)malloc(elements*sizeof(unsigned short));
	float* ptr3 = (float*)malloc(elements*sizeof(float));

	for(int i=0;i<elements;i++){
		ptr1[i] = 10;		
		ptr2[i] = 10;
		ptr3[i] = 10.f;
	}

	//Perform blocking upload from CPU to device
	m1._upload(elements, ptr1);
	m2._upload(elements*sizeof(unsigned short), ptr2);
	m3._upload(elements*sizeof(float), ptr3);
	
	for(int i=0;i<elements;i++){
		ptr1[i] = 0;
		ptr2[i] = 0;
		ptr3[i] = 0;
	}

	//Perform blocking download from device to CPU
	m1._download(elements, ptr1);
	m2._download(elements*sizeof(unsigned short), ptr2);
	m3._download(elements*sizeof(float), ptr3);

	//Always release the image objects once used. This performs device memory cleanup etc
	m1.release();
	m2.release();
	m3.release();

	//Cleaning up host-side memory
	free(ptr1);
	free(ptr2);
	free(ptr3);

	//////////////////////////////////////////////////////////////////////////////////////

	//Now create a cv::Mat object and use that to create a OclMat object
	Mat temp(1024,1024, CV_8UC1);
	memset(temp.data, 10, temp.rows*temp.cols);

	//This automatically performs blocking-upload of temp data to the device
	OclMat m4(temp);

	//Download data into a raw pointer to check if it worked in the first place
	unsigned char* ptr4 = (unsigned char*)malloc(1024*1024);
	m4._download(1024*1024, ptr4);

	//Now change the Mat data and use OclMat's upload() to update the data
	memset(temp.data, 50, temp.rows*temp.cols);
	m4.upload(temp);

	m4._download(1024*1024, ptr4);

	//Create another Mat object and download OclMat's data onto this object
	Mat temp2(temp.rows, temp.cols, CV_8UC1);
	m4.download(temp2);

	//Release
	temp.release();
	temp2.release();
	m4.release();
	free(ptr4);*/

}