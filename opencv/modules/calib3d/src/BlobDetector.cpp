#include <camera_calibration_with_circles/BlobDetector.hpp>

using namespace cv;

BlobDetector::BlobDetector()
{
  threshStep = 20;
  minThresh = 70;
  maxThresh = 200;
  maxCentersDist = 7;

  defaultKeypointSize = 1;

  minArea = 16;
  maxArea = 1000;
  //minInertiaRatio = 0.6;
  minInertiaRatio = 0.4;
  minCircularity = 0.8;

  isGrayscaleCentroid = false;
  centroidROIMargin = 2;

  filterByArea = true;
  filterByInertia = true;

  filterByColor = true;
  filterByCircularity = false;
}

Point2d BlobDetector::computeGrayscaleCentroid(const Mat &image, const vector<Point> &contour) const
{
  Rect rect = boundingRect(Mat(contour));
  rect.x -= centroidROIMargin;
  rect.y -= centroidROIMargin;
  rect.width += 2 * centroidROIMargin;
  rect.height += 2 * centroidROIMargin;

  rect.x = rect.x < 0 ? 0 : rect.x;
  rect.y = rect.y < 0 ? 0 : rect.y;
  rect.width = rect.x + rect.width < image.cols ? rect.width : image.cols - rect.x;
  rect.height = rect.y + rect.height < image.rows ? rect.height : image.rows - rect.y;

  Mat roi = image(rect);
  assert( roi.type() == CV_8UC1 );

  Mat invRoi = 255 - roi;
  invRoi.convertTo(invRoi, CV_32FC1);
  invRoi = invRoi.mul(invRoi);

  Moments moms = moments(invRoi);

  Point2d tl = rect.tl();
  Point2d roiCentroid(moms.m10 / moms.m00, moms.m01 / moms.m00);

  Point2d centroid = tl + roiCentroid;
  return centroid;
}

void BlobDetector::findBlobs(const cv::Mat &image, const cv::Mat &binaryImage, vector<Center> &centers) const
{
  centers.clear();

  vector<vector<Point> > contours;
  Mat tmpBinaryImage = binaryImage.clone();
  findContours(tmpBinaryImage, contours, CV_RETR_LIST, CV_CHAIN_APPROX_NONE);

  for (size_t contourIdx = 0; contourIdx < contours.size(); contourIdx++)
  {
    Center center;
    center.confidence = 1;
    Moments moms = moments(Mat(contours[contourIdx]));
    if (filterByArea)
    {
      double area = moms.m00;
      if (area < minArea || area > maxArea)
        continue;
    }

    if (filterByCircularity)
    {
      double area = moms.m00;
      double perimeter = arcLength(Mat(contours[contourIdx]), true);
      double ratio = 4 * M_PI * area / (perimeter * perimeter);
      if (ratio < minCircularity)
        continue;
    }

    if (filterByInertia)
    {
      double denominator = sqrt(pow(2 * moms.mu11, 2) + pow(moms.mu20 - moms.mu02, 2));
      const double eps = 1e-2;
      double ratio;
      if (denominator > eps)
      {
        double cosmin = (moms.mu20 - moms.mu02) / denominator;
        double sinmin = 2 * moms.mu11 / denominator;
        double cosmax = -cosmin;
        double sinmax = -sinmin;

        double imin = 0.5 * (moms.mu20 + moms.mu02) - 0.5 * (moms.mu20 - moms.mu02) * cosmin - moms.mu11 * sinmin;
        double imax = 0.5 * (moms.mu20 + moms.mu02) - 0.5 * (moms.mu20 - moms.mu02) * cosmax - moms.mu11 * sinmax;
        ratio = imin / imax;
      }
      else
      {
        ratio = 1;
      }

      if (ratio < minInertiaRatio)
        continue;

      center.confidence = ratio * ratio;
    }

    if( isGrayscaleCentroid )
      center.location = computeGrayscaleCentroid( image, contours[contourIdx] );
    else
      center.location = Point2d(moms.m10 / moms.m00, moms.m01 / moms.m00);


    if (filterByColor)
    {
      if (binaryImage.at<uchar> (center.location.y, center.location.x) == 255)
        continue;

    }
    centers.push_back(center);
  }

}

void BlobDetector::detectImpl(const cv::Mat& image, std::vector<cv::KeyPoint>& keypoints, const cv::Mat& mask) const
{
  keypoints.clear();
  const int defaultKeypointSize = 1;

  vector<vector<Center> > centers;
  for (double thresh = minThresh; thresh < maxThresh; thresh += threshStep)
  {

    Mat binarizedImage;
    threshold(image, binarizedImage, thresh, 255, THRESH_BINARY);

    vector<Center> curCenters;
    findBlobs(image, binarizedImage, curCenters);
    for (size_t i = 0; i < curCenters.size(); i++)
    {
      bool isNew = true;
      for (size_t j = 0; j < centers.size(); j++)
      {
        if (norm(centers[j][0].location - curCenters[i].location) < maxCentersDist)
        {
          centers[j].push_back(curCenters[i]);
          isNew = false;
          break;
        }
      }
      if (isNew)
      {
        centers.push_back(vector<Center> (1, curCenters[i]));
      }
    }
  }

  for (size_t i = 0; i < centers.size(); i++)
  {
    Point2d sumPoint(0, 0);
    double normalizer = 0;
    for (size_t j = 0; j < centers[i].size(); j++)
    {
      sumPoint += centers[i][j].confidence * centers[i][j].location;
      normalizer += centers[i][j].confidence;
    }
    sumPoint *= (1. / normalizer);
    KeyPoint kpt(sumPoint, defaultKeypointSize);
    keypoints.push_back(kpt);
  }
}
