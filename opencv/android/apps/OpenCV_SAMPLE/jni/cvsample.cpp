#include "cvsample.h"
#include <opencv2/imgproc/imgproc.hpp>

void CVSample::canny(const cv::Mat& input, cv::Mat& output, int edgeThresh)
{
  if (input.empty())
    return;
  cv::Mat gray;
  if (input.channels() == 3)
  {
    cv::cvtColor(input, gray, CV_RGB2GRAY);
  }
  else
    gray = input;
  cv::Canny(gray, output, edgeThresh, edgeThresh * 3, 3);
}
