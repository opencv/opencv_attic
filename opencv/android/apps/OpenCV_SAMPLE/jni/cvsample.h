#pragma once

#include <opencv2/core/core.hpp>

class CVSample
{
public:
  void canny(const cv::Mat& input, cv::Mat& output, int edgeThresh);
  void invert(cv::Mat& inout);
};
