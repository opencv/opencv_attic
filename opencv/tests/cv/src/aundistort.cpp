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
//     and/or other materials provided with the distribution.
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

#include "cvtest.h"

class CV_DefaultNewCameraMatrixTest : public CvArrTest
{
public:
	CV_DefaultNewCameraMatrixTest();
protected:
	//int validate_test_results( int test_case_idx );
	int prepare_test_case (int test_case_idx);
	void prepare_to_validation( int test_case_idx );
	void get_test_array_types_and_sizes( int test_case_idx, CvSize** sizes, int** types );
	void run_func();
	
	cv::Size img_size;
	cv::Mat camera_mat;
	cv::Mat new_camera_mat;

	int matrix_type;

	bool center_principal_point;

	static const int MAX_X = 2048;
	static const int MAX_Y = 2048;
	static const int MAX_VAL = 10000;
};

CV_DefaultNewCameraMatrixTest::CV_DefaultNewCameraMatrixTest() : CvArrTest("undistort-getDefaultNewCameraMatrix","getDefaultNewCameraMatrix")
{
	 test_array[INPUT].push(NULL);
	 test_array[OUTPUT].push(NULL);
	 test_array[REF_OUTPUT].push(NULL);
}

void CV_DefaultNewCameraMatrixTest::get_test_array_types_and_sizes( int test_case_idx, CvSize** sizes, int** types )
{
	CvRNG* rng = ts->get_rng();
	matrix_type = types[INPUT][0] = types[OUTPUT][0]= types[REF_OUTPUT][0] = cvTsRandInt(rng)%2 ? CV_64F : CV_32F;
	sizes[INPUT][0] = sizes[OUTPUT][0] = sizes[REF_OUTPUT][0] = cvSize(3,3);
}

int CV_DefaultNewCameraMatrixTest::prepare_test_case(int test_case_idx)
{
	int code = CvArrTest::prepare_test_case( test_case_idx );

	if (code <= 0)
		return code;

	CvRNG* rng = ts->get_rng();

	img_size.width = cvTsRandInt(rng) % MAX_X + 1;
	img_size.height = cvTsRandInt(rng) % MAX_Y + 1;

	center_principal_point = ((cvTsRandInt(rng) % 2)!=0);

	// Generating camera_mat matrix
	double sz = MAX(img_size.width, img_size.height);
	double aspect_ratio = cvTsRandReal(rng)*0.6 + 0.7;
	double a[9] = {0,0,0,0,0,0,0,0,1};
	CvMat _a = cvMat(3,3,CV_64F,a);
	a[2] = (img_size.width - 1)*0.5 + cvTsRandReal(rng)*10 - 5;
    a[5] = (img_size.height - 1)*0.5 + cvTsRandReal(rng)*10 - 5;
    a[0] = sz/(0.9 - cvTsRandReal(rng)*0.6);
    a[4] = aspect_ratio*a[0];

	//Copying into input array
	CvMat* _a0 = &test_mat[INPUT][0];
	cvTsConvert( &_a, _a0 );
	camera_mat = _a0;
	//new_camera_mat = camera_mat;

	return code;
	
}

void CV_DefaultNewCameraMatrixTest::run_func()
{
	new_camera_mat = cv::getDefaultNewCameraMatrix(camera_mat,img_size,center_principal_point);
}

void CV_DefaultNewCameraMatrixTest::prepare_to_validation( int test_case_idx )
{
	const CvMat* src = &test_mat[INPUT][0];
	CvMat* dst = &test_mat[REF_OUTPUT][0];
	CvMat* test_output = &test_mat[OUTPUT][0];
	CvMat output = new_camera_mat;
	cvTsConvert( &output, test_output );
	if (!center_principal_point)
	{
		cvCopy(src,dst);
	}
	else
	{
		double a[9] = {0,0,0,0,0,0,0,0,1};
		CvMat _a = cvMat(3,3,CV_64F,a);
		if (matrix_type == CV_64F)
		{
			a[0] = ((double*)(src->data.ptr + src->step*0))[0];
			a[4] = ((double*)(src->data.ptr + src->step*1))[1];
		}
		else
		{
			a[0] = (double)((float*)(src->data.ptr + src->step*0))[0];
			a[4] = (double)((float*)(src->data.ptr + src->step*1))[1];
		}
		a[2] = (img_size.width - 1)*0.5;
		a[5] = (img_size.height - 1)*0.5;
		cvTsConvert( &_a, dst );
	}
}

CV_DefaultNewCameraMatrixTest default_new_camera_matrix_test; 