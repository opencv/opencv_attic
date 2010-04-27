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

/****************************************************************************************\
*                                 DescriptorExtractor                                    *
\****************************************************************************************/

/*
 *   DescriptorExtractor
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
 * SurfDescriptorExtractor
 */
SurfDescriptorExtractor::SurfDescriptorExtractor( int nOctaves,
                                                  int nOctaveLayers, bool extended )
    : surf( 0.0, nOctaves, nOctaveLayers, extended )
{}

void SurfDescriptorExtractor::compute( const Mat& image,
                                       vector<KeyPoint>& keypoints,
                                       Mat& descriptors) const
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

/****************************************************************************************\
*                                DescriptorMatchGeneric                                  *
\****************************************************************************************/

/*
 * KeyPointCollection
 */
void KeyPointCollection::add( const Mat& _image, const vector<KeyPoint>& _points )
{
    // update m_start_indices
    if( startIndices.empty() )
        startIndices.push_back(0);
    else
        startIndices.push_back(*startIndices.rbegin() + points.rbegin()->size());

    // add image and keypoints
    images.push_back(_image);
    points.push_back(_points);
}

KeyPoint KeyPointCollection::getKeyPoint( int index ) const
{
    size_t i = 0;
    for(; i < startIndices.size() && startIndices[i] <= index; i++);
    i--;
    assert(i < startIndices.size() && index - startIndices[i] < points[i].size());

    return points[i][index - startIndices[i]];
}

size_t KeyPointCollection::calcKeypointCount() const
{
    if( startIndices.empty() )
        return 0;
    return *startIndices.rbegin() + points.rbegin()->size();
}

/*
 * DescriptorMatchGeneric
 */
void DescriptorMatchGeneric::add( KeyPointCollection& collection )
{
    for( size_t i = 0; i < collection.images.size(); i++ )
        add( collection.images[i], collection.points[i] );
}

void DescriptorMatchGeneric::classify( const Mat& image, vector<cv::KeyPoint>& points )
{
    vector<int> keypointIndices;
    match( image, points, keypointIndices );

    // remap keypoint indices to descriptors
    for( size_t i = 0; i < keypointIndices.size(); i++ )
        points[i].class_id = collection.getKeyPoint(keypointIndices[i]).class_id;
};

/*
 * DescriptorMatchOneWay
 */
const float DescriptorMatchOneWay::DescriptorMatchOneWayParams::minScaleDefault = 1.0;
const float DescriptorMatchOneWay::DescriptorMatchOneWayParams::maxScaleDefault = 3.0;
const float DescriptorMatchOneWay::DescriptorMatchOneWayParams::stepScaleDefault = 1.15;

DescriptorMatchOneWay::DescriptorMatchOneWay()
{}

DescriptorMatchOneWay::DescriptorMatchOneWay( const DescriptorMatchOneWay::DescriptorMatchOneWayParams& _params)
{
    initialize(_params);
}

DescriptorMatchOneWay::~DescriptorMatchOneWay()
{}

void DescriptorMatchOneWay::initialize( const DescriptorMatchOneWay::DescriptorMatchOneWayParams& _params)
{
    base.release();
    params = _params;
}

void DescriptorMatchOneWay::add( const Mat& image, vector<KeyPoint>& keypoints )
{
    if( base.empty() )
    {
        base = new OneWayDescriptorObject( params.patchSize, params.poseCount, params.trainPath.c_str(),
                                           params.pcaConfig.c_str(), params.pcaHrConfig.c_str(),
                                           params.pcaDescConfig.c_str());
    }

    size_t trainFeatureCount = keypoints.size();

    base->Allocate( trainFeatureCount );
    // classIds.insert( classIds.end(), trainFeatureCount, -1);

    IplImage _image = image;
    for( size_t i = 0; i < keypoints.size(); i++ )
        base->InitializeDescriptor( i, &_image, keypoints[i], "" );

    collection.add( Mat(), keypoints );

#if defined(_KDTREE)
    base->ConvertDescriptorsArrayToTree();
#endif
}

void DescriptorMatchOneWay::add( const KeyPointCollection& keypoints )
{
    if( base.empty() )
    {
        base = new OneWayDescriptorObject( params.patchSize, params.poseCount, params.trainPath.c_str(),
                                           params.pcaConfig.c_str(), params.pcaHrConfig.c_str(),
                                           params.pcaDescConfig.c_str());
    }

    size_t trainFeatureCount = keypoints.calcKeypointCount();

    base->Allocate( trainFeatureCount ); //TBD

    int count = 0;
    for( size_t i = 0; i < keypoints.points.size(); i++ )
    {
        // classIds.insert(classIds.end(), keypoints.classIds[i].begin(), keypoints.classIds[i].end());

        for( size_t j = 0; j < keypoints.points[i].size(); j++ )
        {
            IplImage img = keypoints.images[i];
            base->InitializeDescriptor( count++, &img, keypoints.points[i][j], "" );
        }

        collection.add( Mat(), keypoints.points[i] );
    }

#if defined(_KDTREE)
    base->ConvertDescriptorsArrayToTree();
#endif
}

void DescriptorMatchOneWay::match( const Mat& image, vector<KeyPoint>& points, vector<int>& indices)
{
    indices.resize(points.size());
    IplImage _image = image;
    for( size_t i = 0; i < points.size(); i++ )
    {
        int descIdx = -1;
        int poseIdx = -1;
        float distance;
        base->FindDescriptor( &_image, points[i].pt, descIdx, poseIdx, distance );
        indices[i] = descIdx;
    }
}

void DescriptorMatchOneWay::classify( const Mat& image, vector<KeyPoint>& points )
{
    IplImage _image = image;
    for( size_t i = 0; i < points.size(); i++ )
    {
        int descIdx = -1;
        int poseIdx = -1;
        float distance;
        base->FindDescriptor(&_image, points[i].pt, descIdx, poseIdx, distance);
        points[i].class_id = collection.getKeyPoint(descIdx).class_id;
    }
}
