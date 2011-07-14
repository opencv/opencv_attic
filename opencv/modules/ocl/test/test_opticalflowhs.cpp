//*M///////////////////////////////////////////////////////////////////////////////////////
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

using namespace cv;
using namespace cv::ocl;
using namespace std;

//Draws the sample motion vectors on the 1st frame
static void drawOptFlowMap(const Mat& velX, const Mat& velY, Mat& cflowmap, int step, double, const Scalar& color)
{
    for(int y = 0; y < cflowmap.rows; y += step)
        for(int x = 0; x < cflowmap.cols; x += step)
        {
			int linearAddr = y*cflowmap.cols + x;
            line(cflowmap, Point(x,y), Point(cvRound(x+velX.data[linearAddr]), cvRound(y+velY.data[linearAddr])),color);
            circle(cflowmap, Point(x,y), 2, color, -1);
        }
}

int test_opticalflowhs(){

	VideoCapture capture("d:/dman.avi");
	if(!capture.isOpened()){
		cout<<"Failed to capture video, check the video path again, exiting...\n";
		exit(0);
	}

	Mat frame0color, frame1color;

	namedWindow("Dancing man", 1);

	for(;;){
		//Capture 1st frame
		capture >> frame0color;
		if(frame0color.empty()){ cout<<"exiting..."; break; }

		//Create intermediate frames for holding grayscale images of the source frames
		Mat frame0gray(frame0color.rows, frame0color.cols, CV_8UC1);
		Mat frame1gray(frame0color.rows, frame0color.cols, CV_8UC1);

		//Capture 2nd frame
		capture >> frame1color;
		if(frame1color.empty()){ cout<<"exiting..."; break; }

		//Convert source frames into grayscale
		cvtColor(frame0color, frame0gray, CV_RGB2GRAY);
		cvtColor(frame0color, frame0gray, CV_RGB2GRAY);

		//Create horizontal and vertical velocity vectors on CPU and also on GPU
		Mat velX(frame0gray.rows, frame0gray.cols, CV_32FC1);
		Mat velY(frame0gray.rows, frame0gray.cols, CV_32FC1);
		OclMat velXgpu(velX.rows, velX.rows, velX.type());
		OclMat velYgpu(velX.rows, velX.rows, velX.type());

		CvTermCriteria IterCriteria;
		IterCriteria.type = CV_TERMCRIT_ITER;
		IterCriteria.max_iter = 100;
		float lambda = 0.1f;

		//Compute optical flow on the GPU
		cv::ocl::calcOpticalFlowHS(OclMat(frame0gray), OclMat(frame1gray), velXgpu, velYgpu, IterCriteria, lambda);
		
		//Download velocity vectors to the CPU
		velXgpu.download(velX);
		velYgpu.download(velY);

		//Draw the motion vectors on the source image
		drawOptFlowMap(velX, velY, frame0color, 16, 1.5, CV_RGB(0, 255, 0));

		imshow("Dancing man",frame0color);
		if(waitKey(30)>=0)
            break;

		//Cleanup
		frame0gray.release();
		frame1gray.release();
		velX.release();
		velY.release();
		velXgpu.release();
		velYgpu.release();
	}

		//Some more cleanup
		frame0color.release();
		frame1color.release();
}