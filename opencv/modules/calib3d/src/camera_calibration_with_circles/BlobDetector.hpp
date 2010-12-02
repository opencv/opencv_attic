#ifndef BLOBDETECTOR_HPP_
#define BLOBDETECTOR_HPP_

#include "precomp.hpp"
#include "../../../features2d/include/opencv2/features2d/features2d.hpp"


class BlobDetector 
{
public:
  BlobDetector();

  virtual void detectImpl(const cv::Mat& image, vector<cv::KeyPoint>& keypoints, const cv::Mat& mask = cv::Mat()) const;
private:
  struct Center
  {
    cv::Point2d location;
    double confidence;
  };

  void findBlobs( const cv::Mat &image, const cv::Mat &binaryImage, vector<Center> &centers) const;
  cv::Point2d computeGrayscaleCentroid( const cv::Mat &image, const vector<cv::Point> &contour ) const;

  float threshStep;
  float minThresh;
  float maxThresh;
  float maxCentersDist;

  int defaultKeypointSize;

  float minArea;
  float maxArea;
  float minCircularity;
  float minInertiaRatio;
  bool filterByArea, filterByInertia, filterByCircularity, filterByColor;

  bool isGrayscaleCentroid;
  int centroidROIMargin;
};

#endif /* BLOBDETECTOR_HPP_ */
