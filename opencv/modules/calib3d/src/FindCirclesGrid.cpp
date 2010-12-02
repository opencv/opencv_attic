#include <camera_calibration_with_circles/FindCirclesGrid.hpp>

using namespace cv;

bool findCirclesGrid(const cv::Mat& image, cv::Size patternSize, std::vector<cv::Point2f>& corners, int )
{
  corners.clear();
  Mat imageGray;
  if (image.channels() == 3)
    cvtColor(image, imageGray, CV_BGR2GRAY);
  else
    imageGray = image;

  Ptr<BlobDetector> detector = new BlobDetector();
  //Ptr<FeatureDetector> detector = new MserFeatureDetector(params);

  vector<KeyPoint> keypoints;
  detector->detectImpl(imageGray, keypoints);

  GraphBoxFinderParameters parameters;
  parameters.maxBasisDistance = 7;
  parameters.minDensity = 6;
  parameters.minBestPathLength = 6;
  parameters.minGraphConfidence = patternSize.width * parameters.vertexGain + (patternSize.width - 1)
      * parameters.edgeGain;

  try
  {
    GraphBoxFinder boxFinder(patternSize.width, patternSize.height, imageGray, keypoints, parameters);
    vector<Point2f> vectors, filteredVectors, basis;
    boxFinder.computeEdgeVectorsOfRNG(vectors);

    boxFinder.filterOutliersByDensity(vectors, filteredVectors);
    boxFinder.findBasis(filteredVectors, basis);
    vector<Graph> basisGraphs;
    boxFinder.computeBasisGraphs(basis, basisGraphs);
    boxFinder.findMCS(basis, basisGraphs);
    boxFinder.getHoles(corners);

    if (corners.size() == patternSize.area())
    {
      return true;
    }
  }
  catch (cv::Exception &e)
  {
  }

  return false;
}

