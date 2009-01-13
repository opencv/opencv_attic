#include "cvtest.h"

// * add test for cvFindFeaturesBoxed

#include <algorithm>
#include <vector>
#include <iostream>

class CV_SpilltreeTest : public CvTest {
public:
  CV_SpilltreeTest();
  ~CV_SpilltreeTest();
protected:
  virtual void run( int start_from );
};

CV_SpilltreeTest::CV_SpilltreeTest()
: CvTest( "spill-tree", "cvFindFeatures-spill" ) {
}

CV_SpilltreeTest::~CV_SpilltreeTest() {
}


void CV_SpilltreeTest::run( int /*start_from*/ )
{
  /*
  int dims = 64;
  int features = 2000;
  int k = 1; // * should also test 2nd nn etc.?
  int emax = 20;
  double noise = .2;
  int points = 1000;

  //  CvRNG rng = cvRNG();
  //  CvMat* desc = cvCreateMat(features, dims, CV_64FC1);
  //  cvRandArr( &rng, desc, CV_RAND_UNI, cvRealScalar(0), cvRealScalar(1));
  CvRNG rng = cvRNG(0xffffffff);
  CvMat* desc = cvCreateMat( features, dims, CV_64FC1 );
  cvRandArr( &rng, desc, CV_RAND_UNI, cvRealScalar(-100), cvRealScalar(100) );

  CvFeatureTree* tr = cvCreateSpillTree( desc, 50, .7, .2 );
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
  std::cout << "correct_perc = " << correct_perc << std::endl;
  if (correct_perc < .8)
    ts->set_failed_test_info(CvTS::FAIL_INVALID_OUTPUT);

  cvReleaseFeatureTree(tr);
  */

  //  /*
  int n = 10000;
  int d = 128;
  int k = 10;
  CvMat* toy = cvCreateMat( n, d, CV_64FC1 );
  CvRNG rng_state = cvRNG(0xffffffff);
  cvRandArr( &rng_state, toy, CV_RAND_UNI, cvRealScalar(-100), cvRealScalar(100) );
  ts->printf( CvTS::LOG, "data generated\n" );
  long long t;
  t = cvGetTickCount();
  CvFeatureTree* tr = cvCreateSpillTree( toy, 50, .7, .2 );
  t = cvGetTickCount() - t;
  ts->printf( CvTS::LOG, "spill tree created in %lld ms.\n", t/1000000 );
  CvMat* vector = cvCreateMat( 2, d, CV_64FC1 );
  cvRandArr( &rng_state, vector, CV_RAND_UNI, cvRealScalar(-100), cvRealScalar(100) );
  double* vv = vector->data.db;
  ts->printf( CvTS::LOG, "test drive: " );
  for ( int i = 0; i < d-1; i++ )
    ts->printf( CvTS::LOG, "%.2f ", vv[i] );
  ts->printf( CvTS::LOG, "%.2f\n", vv[d-1] );
  CvMat* res = cvCreateMat( 2, k, CV_32SC1 );
  CvMat* dist = cvCreateMat( 2, k, CV_64FC1 );
  t = cvGetTickCount();
  cvFindFeatures( tr, vector, res, dist, k );
  //  */
}


//CV_SpilltreeTest spilltree_test;
