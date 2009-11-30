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

class KMeansTest : public CvTest {
public:
    KMeansTest();
    ~KMeansTest();
protected:
    virtual void run( int start_from );
    int validate( const Mat& labels, const int* size );
};

KMeansTest::KMeansTest()
: CvTest( "kmeans", "kmeans" )
{
}
KMeansTest::~KMeansTest() 
{
}

void fitDistribution( Mat& data, int bi, int ei, Mat& m, Mat& e )
{
    for( int i = bi; i < ei; i++ )
    {
        Mat r(1, 2, CV_32FC1, data.ptr<float>(i));
        r =  r * e + m;
    }
}

bool checkPointsSet( const Mat& labels, int bi, int ei, bool* busyLbls, int& err )
{
    int count[] = {0, 0, 0};
    for( int i = bi; i < ei; i++ )
        count[labels.at<int>(i, 0)]++;
    int lbl = 0;
    lbl = count[1] > count[lbl] ? 1 : lbl;
    lbl = count[2] > count[lbl] ? 2 : lbl;
    if( busyLbls[lbl] )
        return false;
    else
        busyLbls[lbl] = true;
    err += ei - bi - count[lbl];
    return true;
}

int KMeansTest::validate( const Mat& labels, const int* size )
{
    int pointsCount = size[0]+ size[1] + size[2];
    bool busyLbls[] = {false, false, false};
    int err = 0;
    int code = CvTS::OK;
    if( !checkPointsSet( labels, 0, size[0], busyLbls, err ) )
        code =  CvTS::FAIL_BAD_ACCURACY;
    else 
    { 
        if( !checkPointsSet( labels, size[0], size[0]+size[1], busyLbls, err ) )
            code =  CvTS::FAIL_BAD_ACCURACY;
        else
        {
            if( !checkPointsSet( labels, size[0]+size[1], pointsCount, busyLbls, err ) )
                code =  CvTS::FAIL_BAD_ACCURACY;
            else
            {
                if( (float)err > pointsCount * 0.01f )
                    code =  CvTS::FAIL_BAD_ACCURACY;
            }
        }
    }
    return code;
}

void KMeansTest::run( int /*start_from*/ )
{
    const int iters = 100;
    int size[] = { 5000, 7000, 8000 };
    int pointsCount = size[0]+ size[1] + size[2];
    // mean and covariance
    float mp0[] = {0.0f, 0.0f}, ep0[] = {0.67f, 0.0f, 0.0f, 0.67f};
    float mp1[] = {5.0f, 0.0f}, ep1[] = {1.0f, 0.0f, 0.0f, 1.0f};
    float mp2[] = {1.0f, 5.0f}, ep2[] = {1.0f, 0.0f, 0.0f, 1.0f};
    Mat m0( 1, 2, CV_32FC1, mp0 ), e0( 2, 2, CV_32FC1, ep0 );
    Mat m1( 1, 2, CV_32FC1, mp1 ), e1( 2, 2, CV_32FC1, ep1 );
    Mat m2( 1, 2, CV_32FC1, mp2 ), e2( 2, 2, CV_32FC1, ep2 );

    Mat data( pointsCount, 2, CV_32FC1 );
    Mat bestLabels;

    // generate points by 3 Gaussian distribution 
    randn( data, Scalar::all(0.0), Scalar::all(1.0) );
    fitDistribution( data, 0, size[0], m0, e0 );
    fitDistribution( data, size[0], size[0]+size[1], m1, e1 );
    fitDistribution( data, size[0]+size[1], pointsCount, m2, e2 );
    
    int code = CvTS::OK, tempCode;

    // 1. flag==KMEANS_PP_CENTERS
    kmeans( data, 3, bestLabels, TermCriteria( TermCriteria::COUNT, iters, 0.0), 0, KMEANS_PP_CENTERS, 0 );
    tempCode = validate( bestLabels, size );
    if( tempCode == CvTS::FAIL_BAD_ACCURACY )
        ts->printf( CvTS::LOG, "bad accuracy if flag==KMEANS_PP_CENTERS" );
    code = tempCode != CvTS::OK ? tempCode : code;

    // 2. flag==KMEANS_RANDOM_CENTERS
    kmeans( data, 3, bestLabels, TermCriteria( TermCriteria::COUNT, iters, 0.0), 0, KMEANS_RANDOM_CENTERS, 0 );
    tempCode = validate( bestLabels, size );
    if( tempCode == CvTS::FAIL_BAD_ACCURACY )
        ts->printf( CvTS::LOG, "bad accuracy if flag==KMEANS_RANDOM_CENTERS" );
    code = tempCode != CvTS::OK ? tempCode : code;

    // 3. flag==KMEANS_USE_INITIAL_LABELS
    bestLabels.create( pointsCount, 1, CV_32SC1 );
    for( int i = 0; i < size[0]; i++ )
        bestLabels.at<int>( i, 0 ) = 0;
    for( int i = size[0]; i < size[0]+size[1]; i++ )
        bestLabels.at<int>( i, 0 ) = 1;
    for( int i = size[0]+size[1]; i < pointsCount; i++ )
        bestLabels.at<int>( i, 0 ) = 2;
    RNG rng;
    for( int i = 0; i < 0.5f * pointsCount; i++ )
        bestLabels.at<int>( rng.next() % pointsCount, 0 ) = rng.next() % 3;
    kmeans( data, 3, bestLabels, TermCriteria( TermCriteria::COUNT, iters, 0.0), 0, KMEANS_USE_INITIAL_LABELS, 0 );
    tempCode = validate( bestLabels, size );
    if( tempCode == CvTS::FAIL_BAD_ACCURACY )
        ts->printf( CvTS::LOG, "bad accuracy if flag==KMEANS_USE_INITIAL_LABELS" );
    code = tempCode != CvTS::OK ? tempCode : code;

    ts->set_failed_test_info( code );
}

KMeansTest kmeans_test;
