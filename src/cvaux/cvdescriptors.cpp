/*M///////////////////////////////////////////////////////////////////////////////////////
//
//  IMPORTANT: READ BEFORE DOWNLOADING, COPYING, INSTALLING OR USING.
//
//  By downloading, copying, installing or using the software you agree to this license.
//  If you do not agree to this license, do not download, install,
//  copy or use the software.
//
//
//                        Intel License Agreement
//                For Open Source Computer Vision Library
//
// Copyright (C) 2000, Intel Corporation, all rights reserved.
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
//   * The name of Intel Corporation may not be used to endorse or promote products
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

#include "_cvaux.h"

using namespace std;
using namespace cv;

/*
    DescriptorExtractor
*/
struct RoiPredicate
{
    RoiPredicate(float _minX, float _minY, float _maxX, float _maxY)
        : minX(_minX), minY(_minY), maxX(_maxX), maxY(_maxY)
    {}

    bool operator()( const KeyPoint& keyPt) const
    {
        Point2f pt = keyPt.pt;
        return (pt.x < minX) || (pt.x >= maxX) || (pt.y < minY) || (pt.y >= maxY);
    }

    float minX, minY, maxX, maxY;
};

void DescriptorExtractor::removeBorderKeypoints( vector<KeyPoint>& keypoints,
                                                 Size imageSize, int borderPixels )
{
    keypoints.erase( remove_if(keypoints.begin(), keypoints.end(),
                               RoiPredicate(borderPixels, borderPixels,
                                            imageSize.width - borderPixels,
                                            imageSize.height - borderPixels)),
                     keypoints.end());
}

/*
  SurfDescriptorExtractor
*/
SurfDescriptorExtractor::SurfDescriptorExtractor( int nOctaves,
                                                  int nOctaveLayers, bool extended )
    : surf( 0.0, nOctaves, nOctaveLayers, extended )
{}

void SurfDescriptorExtractor::compute(const cv::Mat& image,
                                      std::vector<cv::KeyPoint>& keypoints,
                                      cv::Mat& descriptors) const
{
    // Compute descriptors for given keypoints
    vector<float> _descriptors;
    Mat mask;
    bool useProvidedKeypoints = true;
    surf(image, mask, keypoints, _descriptors, useProvidedKeypoints);

    descriptors.create(keypoints.size(), surf.descriptorSize(), CV_32FC1);
    assert( (int)_descriptors.size() == descriptors.rows * descriptors.cols );
    std::copy(_descriptors.begin(), _descriptors.end(), descriptors.begin<float>());
}
