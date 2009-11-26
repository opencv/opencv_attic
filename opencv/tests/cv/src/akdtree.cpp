// 2009-01-14, Xavier Delacour <xavier.delacour@gmail.com>

#include "cvtest.h"

// * add test for cvFindFeaturesBoxed

#include <algorithm>
#include <vector>
#include <iostream>

using namespace cv;

class CV_KdtreeTest : public CvTest {
public:
  CV_KdtreeTest();
  ~CV_KdtreeTest();
protected:
  virtual void run( int start_from );
};

CV_KdtreeTest::CV_KdtreeTest()
: CvTest( "kd-tree", "cvFindFeatures-kd" ) {
}

CV_KdtreeTest::~CV_KdtreeTest() {
}


void CV_KdtreeTest::run( int /*start_from*/ ) {
  int dims = 64;
  int features = 2000;
  int k = 1; // * should also test 2nd nn etc.?
  int emax = 20;
  double noise = .2;
  int points = 1000;

  CvRNG rng = cvRNG();
  CvMat* desc = cvCreateMat(features, dims, CV_64FC1);
  cvRandArr( &rng, desc, CV_RAND_UNI, cvRealScalar(0), cvRealScalar(1));

  CvFeatureTree* tr = cvCreateKDTree(desc);
  CvMat* results = cvCreateMat(points, k, CV_32SC1);
  CvMat* dist = cvCreateMat(points, k, CV_64FC1);

  CvMat* pts = cvCreateMat(points, dims, CV_64FC1);
  std::vector<int> fmap(points);
  for (int j = 0; j < points; ++j) {
    int fi = cvRandInt(&rng) % features;
    fmap[j] = fi;
    double* f = (double*)cvPtr2D(desc, fi, 0);
    double* p = (double*)cvPtr2D(pts, j, 0);
    for (int k = 0; k < dims; ++ k)
      p[k] = f[k] + cvRandReal(&rng) * noise;
  }

  cvFindFeatures(tr, pts, results, dist, k, emax);

  int correct_matches = 0;
  { // Aisle "j" to avoid error on MSVC6
    for (int j = 0; j < points; ++j) {
      int fi = (int)cvGetReal2D(results, j, 0);
      if (fmap[j] == fi)
        ++correct_matches;
    }
  }

  double correct_perc = correct_matches / (double)points;
  ts->printf( CvTS::LOG, "correct_perc = %d\n", correct_perc );
  if (correct_perc < .8)
    ts->set_failed_test_info(CvTS::FAIL_INVALID_OUTPUT);

  cvReleaseFeatureTree(tr);
}

CV_KdtreeTest kdtree_test;



// ----------------------------- test for cv::KDTree -----------------------
class KDTreeTest : public CvTest {
public:
    KDTreeTest();
    ~KDTreeTest();
protected:
    virtual void run( int start_from );
};

KDTreeTest::KDTreeTest()
: CvTest( "cpp_kdtree", "cv::KDTree funcs" )
{
}
KDTreeTest::~KDTreeTest() 
{
}

void KDTreeTest::run( int /*start_from*/ ) {
    int dims = 64;
    int featuresCount = 2000;
    int K = 1; // * should also test 2nd nn etc.?
    int emax = 2000;
    float noise = 0.2f;
    int pointsCount = 1000;

    RNG rng;
    Mat desc( featuresCount, dims, CV_32FC1 );
    rng.fill( desc, RNG::UNIFORM, Scalar(0.0f), Scalar(1.0f) );

    KDTree tr( desc );
    Mat pts( pointsCount, dims, CV_32FC1 );
    Mat results( pointsCount, K, CV_32SC1 );

    std::vector<int> fmap( pointsCount );
    for( int pi = 0; pi < pointsCount; pi++ )
    {
        int fi = rng.next() % featuresCount;
        fmap[pi] = fi;
        for( int d = 0; d < dims; d++ )
            pts.at<float>(pi, d) = desc.at<float>(fi, d) + rng.uniform(0.0f, 1.0f) * noise;
        tr.findNearest( pts.ptr<float>(pi), K, emax, results.ptr<int>(pi) );
    }

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

KDTreeTest cpp_kdtree_test;

