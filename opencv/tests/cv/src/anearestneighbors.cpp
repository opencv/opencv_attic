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

#include <algorithm>
#include <vector>
#include <iostream>

using namespace cv;
using namespace cv::flann;

//--------------------------------------------------------------------------------
class NearestNeighborTest : public CvTest
{
public:
    NearestNeighborTest( const char* test_name, const char* test_funcs ) 
        : CvTest( test_name, test_funcs ) {}
protected:
    virtual void run( int start_from );
    virtual void createModel( const Mat& data ) = 0;
    virtual void searchNeighbors( Mat& points, Mat& neighbors ) = 0;
    virtual void releaseModel() = 0;
};

void NearestNeighborTest::run( int /*start_from*/ ) {
    int dims = 30;
    int featuresCount = 2000;
    int K = 1; // * should also test 2nd nn etc.?
    float noise = 0.2f;
    int pointsCount = 1000;

    RNG rng;
    Mat desc( featuresCount, dims, CV_32FC1 );
    rng.fill( desc, RNG::UNIFORM, Scalar(0.0f), Scalar(1.0f) );

    createModel( desc );
    
    Mat points( pointsCount, dims, CV_32FC1 );
    Mat results( pointsCount, K, CV_32SC1 );

    std::vector<int> fmap( pointsCount );
    for( int pi = 0; pi < pointsCount; pi++ )
    {
        int fi = rng.next() % featuresCount;
        fmap[pi] = fi;
        for( int d = 0; d < dims; d++ )
            points.at<float>(pi, d) = desc.at<float>(fi, d) + rng.uniform(0.0f, 1.0f) * noise;
    }
    searchNeighbors( points, results );

    releaseModel();

    int correctMatches = 0;
    for( int pi = 0; pi < pointsCount; pi++ )
    {
        if( fmap[pi] == results.at<int>(pi, 0) )
            correctMatches++;
    }

    double correctPerc = correctMatches / (double)pointsCount;
    ts->printf( CvTS::LOG, "correct_perc = %d\n", correctPerc );
    if (correctPerc < .8)
        ts->set_failed_test_info(CvTS::FAIL_INVALID_OUTPUT);
}

//--------------------------------------------------------------------------------
class CV_LSHTest : public NearestNeighborTest
{
public:
    CV_LSHTest() : NearestNeighborTest( "lsh", "cvLSHQuery" ) {}
protected:
    virtual void createModel( const Mat& data );
    virtual void searchNeighbors( Mat& points, Mat& neighbors );
    virtual void releaseModel();
    struct CvLSH* lsh;
    CvMat desc;
};

void CV_LSHTest::createModel( const Mat& data )
{
    desc = data;
    lsh = cvCreateMemoryLSH( data.cols, data.rows, 70, 20, CV_32FC1 );
    cvLSHAdd( lsh, &desc );
}

void CV_LSHTest::searchNeighbors( Mat& points, Mat& neighbors )
{
    const int emax = 20;
    Mat dist( points.rows, neighbors.cols, CV_64FC1);
    CvMat _dist = dist, _points = points, _neighbors = neighbors;
    cvLSHQuery( lsh, &_points, &_neighbors, &_dist, neighbors.cols, emax );
}

void CV_LSHTest::releaseModel()
{
    cvReleaseLSH( &lsh );
}

//--------------------------------------------------------------------------------
class CV_FeatureTreeTest_C : public NearestNeighborTest
{
public:
    CV_FeatureTreeTest_C( const char* test_name, const char* test_funcs ) 
        : NearestNeighborTest( test_name, test_funcs ) {}
protected:
    virtual void searchNeighbors( Mat& points, Mat& neighbors );
    virtual void releaseModel();
    CvFeatureTree* tr;
    CvMat desc;
};

void CV_FeatureTreeTest_C::searchNeighbors( Mat& points, Mat& neighbors )
{
    const int emax = 20;
    Mat dist( points.rows, neighbors.cols, CV_64FC1);
    CvMat _dist = dist, _points = points, _neighbors = neighbors;
    cvFindFeatures( tr, &_points, &_neighbors, &_dist, neighbors.cols, emax );
}

void CV_FeatureTreeTest_C::releaseModel()
{
    cvReleaseFeatureTree( tr );
}

//--------------------------------------
class CV_SpillTreeTest_C : public CV_FeatureTreeTest_C
{
public:
    CV_SpillTreeTest_C(): CV_FeatureTreeTest_C( "spilltree_c", "cvFindFeatures-spill" ) {}
protected:
    virtual void createModel( const Mat& data );
};

void CV_SpillTreeTest_C::createModel( const Mat& data )
{
    desc = data;
    tr = cvCreateSpillTree( &desc );
}

//--------------------------------------
class CV_KDTreeTest_C : public CV_FeatureTreeTest_C
{
public:
    CV_KDTreeTest_C(): CV_FeatureTreeTest_C( "kdtree_c", "cvFindFeatures-kd" ) {}
protected:
    virtual void createModel( const Mat& data );
};

void CV_KDTreeTest_C::createModel( const Mat& data )
{
    desc = data;
    tr = cvCreateKDTree( &desc );
}

//--------------------------------------------------------------------------------
class CV_KDTreeTest_CPP : public NearestNeighborTest
{
public:
    CV_KDTreeTest_CPP() : NearestNeighborTest( "kdtree_cpp", "cv::KDTree funcs" ) {}
protected:
    virtual void createModel( const Mat& data );
    virtual void searchNeighbors( Mat& points, Mat& neighbors );
    virtual void releaseModel();
    KDTree* tr;
};

void CV_KDTreeTest_CPP::createModel( const Mat& data )
{
    tr = new KDTree( data, false );
}

void CV_KDTreeTest_CPP::searchNeighbors( Mat& points, Mat& neighbors )
{
    const int emax = 20;
    for( int pi = 0; pi < points.rows; pi++ )
        tr->findNearest( points.ptr<float>(pi), neighbors.cols, emax, neighbors.ptr<int>(pi) );
}

void CV_KDTreeTest_CPP::releaseModel()
{
    delete tr;
}

//--------------------------------------------------------------------------------
class CV_FlannTest : public NearestNeighborTest
{
public:
    CV_FlannTest( const char* test_name, const char* test_funcs ) 
        : NearestNeighborTest( test_name, test_funcs ) {}
protected:
    void createIndex( const Mat& data, const IndexParams& params );
    void knnSearch( Mat& points, Mat& neighbors );
    void radiusSearch( Mat& points, Mat& neighbors );
    virtual void releaseModel();
    Index* index;
};

void CV_FlannTest::createIndex( const Mat& data, const IndexParams& params )
{
    index = new Index( data, params );
}

void CV_FlannTest::knnSearch( Mat& points, Mat& neighbors )
{
    Mat dist( points.rows, neighbors.cols, CV_32FC1);
    index->knnSearch( points, neighbors, dist, 1, SearchParams() );
}

void CV_FlannTest::radiusSearch( Mat& points, Mat& neighbors )
{
    Mat dist( 1, neighbors.cols, CV_32FC1);
    // radiusSearch can only search one feature at a time for range search
    for( int i = 0; i < points.rows; i++ )
    {
        Mat p( 1, points.cols, CV_32FC1, points.ptr<float>(i) ),
            n( 1, neighbors.cols, CV_32SC1, neighbors.ptr<int>(i) );
        index->radiusSearch( p, n, dist, 10.0f, SearchParams() );
    }
}

void CV_FlannTest::releaseModel()
{
    delete index;
}

//---------------------------------------
class CV_FlannLinearIndexTest : public CV_FlannTest
{
public:
    CV_FlannLinearIndexTest() : CV_FlannTest( "flann_linear", "LinearIndex" ) {}
protected:
    virtual void createModel( const Mat& data ) { createIndex( data, LinearIndexParams() ); }
    virtual void searchNeighbors( Mat& points, Mat& neighbors ) { knnSearch( points, neighbors ); }
};

//---------------------------------------
class CV_FlannKMeansIndexTest : public CV_FlannTest
{
public:
    CV_FlannKMeansIndexTest() : CV_FlannTest( "flann_kmeans", "KMeansIndex" ) {}
protected:
    virtual void createModel( const Mat& data ) { createIndex( data, KMeansIndexParams() ); }
    virtual void searchNeighbors( Mat& points, Mat& neighbors ) { radiusSearch( points, neighbors ); }
};

//---------------------------------------
class CV_FlannKDTreeIndexTest : public CV_FlannTest
{
public:
    CV_FlannKDTreeIndexTest() : CV_FlannTest( "flann_kdtree", "KDTreeIndex" ) {}
protected:
    virtual void createModel( const Mat& data ) { createIndex( data, KDTreeIndexParams() ); }
    virtual void searchNeighbors( Mat& points, Mat& neighbors ) { radiusSearch( points, neighbors ); }
};

//----------------------------------------
class CV_FlannAutotunedIndexTest : public CV_FlannTest
{
public:
    CV_FlannAutotunedIndexTest() : CV_FlannTest( "flann_autotuned", "AutotunedIndex" ) {}
protected:
    virtual void createModel( const Mat& data ) { createIndex( data, AutotunedIndexParams() ); }
    virtual void searchNeighbors( Mat& points, Mat& neighbors ) { knnSearch( points, neighbors ); }
};

CV_LSHTest lsh_test;
CV_SpillTreeTest_C spilltree_test_c;
CV_KDTreeTest_C kdtree_test_c;
CV_KDTreeTest_CPP kdtree_test_cpp;
CV_FlannLinearIndexTest flann_linear_index;
CV_FlannKMeansIndexTest flann_kmeans_index;
CV_FlannKDTreeIndexTest flann_kdtree_index;
CV_FlannAutotunedIndexTest flann_autotuned_index;
