#ifndef FINDCIRCLESGRID_HPP_
#define FINDCIRCLESGRID_HPP_

#include "precomp.hpp"
#include "BlobDetector.hpp"
#include "GraphBoxFinder.hpp"

bool findCirclesGrid(const cv::Mat& image, cv::Size patternSize, std::vector<cv::Point2f>& corners, int flags =
    cv::CALIB_CB_ADAPTIVE_THRESH + cv::CALIB_CB_NORMALIZE_IMAGE);

#endif /* FINDCIRCLESGRID_HPP_ */
