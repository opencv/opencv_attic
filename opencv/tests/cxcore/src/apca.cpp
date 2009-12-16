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

#include "cxcoretest.h"

using namespace cv;


class CV_PCATest : public CvTest
{
public:
    CV_PCATest() : CvTest( "pca", "PCA funcs" ) {}
protected:
    void run( int);
};

void CV_PCATest::run( int )
{
    int code = CvTS::OK, err;
    int maxComponents = 1;
    Mat points( 1000, 3, CV_32FC1);

	RNG rng = *ts->get_rng(); // get ts->rng seed
	rng.fill( points, RNG::NORMAL, Scalar::all(0.0), Scalar::all(1.0) );

    float mp[] = { 3.0f, 3.0f, 3.0f }, cp[] = { 0.5f, 0.0f, 0.0f,
                                                0.0f, 1.0f, 0.0f,
                                                0.0f, 0.0f, 0.3f };
    Mat mean( 1, 3, CV_32FC1, mp ),
        cov( 3, 3, CV_32FC1, cp );
    for( int i = 0; i < points.rows; i++ )
    {
        Mat r(1, points.cols, CV_32FC1, points.ptr<float>(i));
        r =  r * cov + mean; 
    }

    PCA pca( points, Mat(), CV_PCA_DATA_AS_ROW, maxComponents );

    // check project
    Mat prjPoints = pca.project( points );
    err = 0;
    for( int i = 0; i < prjPoints.rows; i++ )
    {
        float val = prjPoints.at<float>(i,0);
        if( val > 3.0f || val < -3.0f )
            err++;
    }
	float projectErr = 0.02f;
	if( (float)err > prjPoints.rows * projectErr )
    {
        ts->printf( CvTS::LOG, "bad accuracy of project() (real = %f, permissible = %f)",
			(float)err/(float)prjPoints.rows, projectErr );
        code = CvTS::FAIL_BAD_ACCURACY;
    }

    // check backProject
    Mat points1 = pca.backProject( prjPoints );
    err = 0;
	for( int i = 0; i < points.rows; i++ ) 
	{
		if( fabs(points1.at<float>(i,0) - mean.at<float>(0,0)) > 0.15 ||
            fabs(points1.at<float>(i,1) - points.at<float>(i,1)) > 0.05 ||
            fabs(points1.at<float>(i,2) - mean.at<float>(0,2)) > 0.15 )
            err++;
	}
	float backProjectErr = 0.05f;
	if( (float)err > prjPoints.rows*backProjectErr )
    {
        ts->printf( CvTS::LOG, "bad accuracy of backProject() (real = %f, permissible = %f)",
			(float)err/(float)prjPoints.rows, backProjectErr );
        code = CvTS::FAIL_BAD_ACCURACY;
    }

	CvRNG *oldRng = ts->get_rng(); // set ts->rng seed
	*oldRng = rng.state;

    ts->set_failed_test_info( code );
}

CV_PCATest pca_test;