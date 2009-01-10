#include "_cvfeaturetree.h"

void cvReleaseFeatureTree(CvFeatureTree* tr) {
  delete tr;
}

// desc is m x d set of candidate points.
// results is m x k set of row indices of matching points.
// dist is m x k distance to matching points.
void cvFindFeatures(CvFeatureTree* tr, CvMat* desc,
		    CvMat* results, CvMat* dist, int k, int emax) {

  tr->FindFeatures(desc, k, emax, results, dist);
}

int cvFindFeaturesBoxed(CvFeatureTree* tr,
			CvMat* bounds_min, CvMat* bounds_max,
			CvMat* results) {
  return tr->FindOrthoRange(bounds_min, bounds_max, results);
}
