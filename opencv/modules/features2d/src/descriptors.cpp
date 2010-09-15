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

#include "precomp.hpp"

#ifdef HAVE_EIGEN2
#include <Eigen/Array>
#endif

//#define _KDTREE

using namespace std;

const int draw_shift_bits = 4;
const int draw_multiplier = 1 << draw_shift_bits;

namespace cv
{

Mat windowedMatchingMask( const vector<KeyPoint>& keypoints1, const vector<KeyPoint>& keypoints2,
                          float maxDeltaX, float maxDeltaY )
{
    if( keypoints1.empty() || keypoints2.empty() )
        return Mat();

    Mat mask( keypoints1.size(), keypoints2.size(), CV_8UC1 );
    for( size_t i = 0; i < keypoints1.size(); i++ )
    {
        for( size_t j = 0; j < keypoints2.size(); j++ )
        {
            Point2f diff = keypoints2[j].pt - keypoints1[i].pt;
            mask.at<uchar>(i, j) = std::abs(diff.x) < maxDeltaX && std::abs(diff.y) < maxDeltaY;
        }
    }
    return mask;
}

/*
 * Drawing functions
 */

static inline void _drawKeypoint( Mat& img, const KeyPoint& p, const Scalar& color, int flags )
{
    Point center( cvRound(p.pt.x * draw_multiplier), cvRound(p.pt.y * draw_multiplier) );

    if( flags & DrawMatchesFlags::DRAW_RICH_KEYPOINTS )
    {
        int radius = cvRound(p.size/2 * draw_multiplier); // KeyPoint::size is a diameter

        // draw the circles around keypoints with the keypoints size
        circle( img, center, radius, color, 1, CV_AA, draw_shift_bits );

        // draw orientation of the keypoint, if it is applicable
        if( p.angle != -1 )
        {
            float srcAngleRad = p.angle*(float)CV_PI/180.f;
            Point orient(cvRound(cos(srcAngleRad)*radius), 
						 cvRound(sin(srcAngleRad)*radius));
            line( img, center, center+orient, color, 1, CV_AA, draw_shift_bits );
        }
#if 0
        else
        {
            // draw center with R=1
            int radius = 1 * draw_multiplier;
            circle( img, center, radius, color, 1, CV_AA, draw_shift_bits );
        }
#endif
    }
    else
    {
        // draw center with R=3
        int radius = 3 * draw_multiplier;
        circle( img, center, radius, color, 1, CV_AA, draw_shift_bits );
    }
}

void drawKeypoints( const Mat& image, const vector<KeyPoint>& keypoints, Mat& outImg,
                    const Scalar& _color, int flags )
{
    if( !(flags & DrawMatchesFlags::DRAW_OVER_OUTIMG) )
        cvtColor( image, outImg, CV_GRAY2BGR );

    RNG& rng=theRNG();
    bool isRandColor = _color == Scalar::all(-1);

    for( vector<KeyPoint>::const_iterator i = keypoints.begin(), ie = keypoints.end(); i != ie; ++i )
    {
        Scalar color = isRandColor ? Scalar(rng(256), rng(256), rng(256)) : _color;
        _drawKeypoint( outImg, *i, color, flags );
    }
}

static void _prepareImgAndDrawKeypoints( const Mat& img1, const vector<KeyPoint>& keypoints1,
                                         const Mat& img2, const vector<KeyPoint>& keypoints2,
                                         Mat& outImg, Mat& outImg1, Mat& outImg2,
                                         const Scalar& singlePointColor, int flags )
{
    Size size( img1.cols + img2.cols, MAX(img1.rows, img2.rows) );
    if( flags & DrawMatchesFlags::DRAW_OVER_OUTIMG )
    {
        if( size.width > outImg.cols || size.height > outImg.rows )
            CV_Error( CV_StsBadSize, "outImg has size less than need to draw img1 and img2 together" );
        outImg1 = outImg( Rect(0, 0, img1.cols, img1.rows) );
        outImg2 = outImg( Rect(img1.cols, 0, img2.cols, img2.rows) );
    }
    else
    {
        outImg.create( size, CV_MAKETYPE(img1.depth(), 3) );
        outImg1 = outImg( Rect(0, 0, img1.cols, img1.rows) );
        outImg2 = outImg( Rect(img1.cols, 0, img2.cols, img2.rows) );
        cvtColor( img1, outImg1, CV_GRAY2RGB );
        cvtColor( img2, outImg2, CV_GRAY2RGB );
    }

    // draw keypoints
    if( !(flags & DrawMatchesFlags::NOT_DRAW_SINGLE_POINTS) )
    {
        Mat outImg1 = outImg( Rect(0, 0, img1.cols, img1.rows) );
        drawKeypoints( outImg1, keypoints1, outImg1, singlePointColor, flags + DrawMatchesFlags::DRAW_OVER_OUTIMG );

        Mat outImg2 = outImg( Rect(img1.cols, 0, img2.cols, img2.rows) );
        drawKeypoints( outImg2, keypoints2, outImg2, singlePointColor, flags + DrawMatchesFlags::DRAW_OVER_OUTIMG );
    }
}

static inline void _drawMatch( Mat& outImg, Mat& outImg1, Mat& outImg2 ,
                          const KeyPoint& kp1, const KeyPoint& kp2, const Scalar& matchColor, int flags )
{
    RNG& rng = theRNG();
    bool isRandMatchColor = matchColor == Scalar::all(-1);
    Scalar color = isRandMatchColor ? Scalar( rng(256), rng(256), rng(256) ) : matchColor;

    _drawKeypoint( outImg1, kp1, color, flags );
    _drawKeypoint( outImg2, kp2, color, flags );

    Point2f pt1 = kp1.pt,
            pt2 = kp2.pt,
            dpt2 = Point2f( std::min(pt2.x+outImg1.cols, float(outImg.cols-1)), pt2.y );

    line( outImg, 
		  Point(cvRound(pt1.x*draw_multiplier), cvRound(pt1.y*draw_multiplier)),
		  Point(cvRound(dpt2.x*draw_multiplier), cvRound(dpt2.y*draw_multiplier)),
          color, 1, CV_AA, draw_shift_bits );
}

void drawMatches( const Mat& img1, const vector<KeyPoint>& keypoints1,
                  const Mat& img2, const vector<KeyPoint>& keypoints2,
                  const vector<DMatch>& matches1to2, Mat& outImg,
                  const Scalar& matchColor, const Scalar& singlePointColor,
                  const vector<char>& matchesMask, int flags )
{
    if( !matchesMask.empty() && matchesMask.size() != matches1to2.size() )
        CV_Error( CV_StsBadSize, "matchesMask must have the same size as matches1to2" );

    Mat outImg1, outImg2;
    _prepareImgAndDrawKeypoints( img1, keypoints1, img2, keypoints2,
                                 outImg, outImg1, outImg2, singlePointColor, flags );

    // draw matches
    for( size_t m = 0; m < matches1to2.size(); m++ )
    {
        int i1 = matches1to2[m].queryDescIdx;
        int i2 = matches1to2[m].trainDescIdx;
        if( matchesMask.empty() || matchesMask[m] )
        {
            const KeyPoint &kp1 = keypoints1[i1], &kp2 = keypoints2[i2];
            _drawMatch( outImg, outImg1, outImg2, kp1, kp2, matchColor, flags );
        }
    }
}

void drawMatches( const Mat& img1, const vector<KeyPoint>& keypoints1,
                  const Mat& img2, const vector<KeyPoint>& keypoints2,
                  const vector<vector<DMatch> >& matches1to2, Mat& outImg,
                  const Scalar& matchColor, const Scalar& singlePointColor,
                  const vector<vector<char> >& matchesMask, int flags )
{
    if( !matchesMask.empty() && matchesMask.size() != matches1to2.size() )
        CV_Error( CV_StsBadSize, "matchesMask must have the same size as matches1to2" );

    Mat outImg1, outImg2;
    _prepareImgAndDrawKeypoints( img1, keypoints1, img2, keypoints2,
                                 outImg, outImg1, outImg2, singlePointColor, flags );

    // draw matches
    for( size_t i = 0; i < matches1to2.size(); i++ )
    {
        for( size_t j = 0; j < matches1to2[i].size(); j++ )
        {
            int i1 = matches1to2[i][j].queryDescIdx;
            int i2 = matches1to2[i][j].trainDescIdx;
            if( matchesMask.empty() || matchesMask[i][j] )
            {
                const KeyPoint &kp1 = keypoints1[i1], &kp2 = keypoints2[i2];
                _drawMatch( outImg, outImg1, outImg2, kp1, kp2, matchColor, flags );
            }
        }
    }
}

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

void DescriptorExtractor::compute( const vector<Mat>& imageCollection, vector<vector<KeyPoint> >& pointCollection, vector<Mat>& descCollection ) const
{
    descCollection.resize( imageCollection.size() );
    for( size_t i = 0; i < imageCollection.size(); i++ )
        computeImpl( imageCollection[i], pointCollection[i], descCollection[i]);
}

void DescriptorExtractor::removeBorderKeypoints( vector<KeyPoint>& keypoints,
                                                 Size imageSize, int borderPixels )
{
    keypoints.erase( remove_if(keypoints.begin(), keypoints.end(),
                               RoiPredicate((float)borderPixels, (float)borderPixels,
                                            (float)(imageSize.width - borderPixels),
                                            (float)(imageSize.height - borderPixels))),
                     keypoints.end());
}

/****************************************************************************************\
*                                SiftDescriptorExtractor                                 *
\****************************************************************************************/
SiftDescriptorExtractor::SiftDescriptorExtractor( double magnification, bool isNormalize, bool recalculateAngles,
                                                  int nOctaves, int nOctaveLayers, int firstOctave, int angleMode )
    : sift( magnification, isNormalize, recalculateAngles, nOctaves, nOctaveLayers, firstOctave, angleMode )
{}

void SiftDescriptorExtractor::computeImpl( const Mat& image, vector<KeyPoint>& keypoints, Mat& descriptors) const
{
    bool useProvidedKeypoints = true;
    sift(image, Mat(), keypoints, descriptors, useProvidedKeypoints);
}

void SiftDescriptorExtractor::read (const FileNode &fn)
{
    double magnification = fn["magnification"];
    bool isNormalize = (int)fn["isNormalize"] != 0;
    bool recalculateAngles = (int)fn["recalculateAngles"] != 0;
    int nOctaves = fn["nOctaves"];
    int nOctaveLayers = fn["nOctaveLayers"];
    int firstOctave = fn["firstOctave"];
    int angleMode = fn["angleMode"];

    sift = SIFT( magnification, isNormalize, recalculateAngles, nOctaves, nOctaveLayers, firstOctave, angleMode );
}

void SiftDescriptorExtractor::write (FileStorage &fs) const
{
//    fs << "algorithm" << getAlgorithmName ();

    SIFT::CommonParams commParams = sift.getCommonParams ();
    SIFT::DescriptorParams descriptorParams = sift.getDescriptorParams ();
    fs << "magnification" << descriptorParams.magnification;
    fs << "isNormalize" << descriptorParams.isNormalize;
    fs << "recalculateAngles" << descriptorParams.recalculateAngles;
    fs << "nOctaves" << commParams.nOctaves;
    fs << "nOctaveLayers" << commParams.nOctaveLayers;
    fs << "firstOctave" << commParams.firstOctave;
    fs << "angleMode" << commParams.angleMode;
}

/****************************************************************************************\
*                                SurfDescriptorExtractor                                 *
\****************************************************************************************/
SurfDescriptorExtractor::SurfDescriptorExtractor( int nOctaves,
                                                  int nOctaveLayers, bool extended )
    : surf( 0.0, nOctaves, nOctaveLayers, extended )
{}

void SurfDescriptorExtractor::computeImpl( const Mat& image, vector<KeyPoint>& keypoints, Mat& descriptors ) const
{
    // Compute descriptors for given keypoints
    vector<float> _descriptors;
    Mat mask;
    bool useProvidedKeypoints = true;
    surf(image, mask, keypoints, _descriptors, useProvidedKeypoints);

    descriptors.create((int)keypoints.size(), (int)surf.descriptorSize(), CV_32FC1);
    assert( (int)_descriptors.size() == descriptors.rows * descriptors.cols );
    std::copy(_descriptors.begin(), _descriptors.end(), descriptors.begin<float>());
}

void SurfDescriptorExtractor::read( const FileNode &fn )
{
    int nOctaves = fn["nOctaves"];
    int nOctaveLayers = fn["nOctaveLayers"];
    bool extended = (int)fn["extended"] != 0;

    surf = SURF( 0.0, nOctaves, nOctaveLayers, extended );
}

void SurfDescriptorExtractor::write( FileStorage &fs ) const
{
//    fs << "algorithm" << getAlgorithmName ();

    fs << "nOctaves" << surf.nOctaves;
    fs << "nOctaveLayers" << surf.nOctaveLayers;
    fs << "extended" << surf.extended;
}

/*
 * Factory function for DescriptorExtractor creating
 */
Ptr<DescriptorExtractor> createDescriptorExtractor( const string& descriptorExtractorType )
{
    DescriptorExtractor* de = 0;
    if( !descriptorExtractorType.compare( "SIFT" ) )
    {
        de = new SiftDescriptorExtractor/*( double magnification=SIFT::DescriptorParams::GET_DEFAULT_MAGNIFICATION(),
                             bool isNormalize=true, bool recalculateAngles=true,
                             int nOctaves=SIFT::CommonParams::DEFAULT_NOCTAVES,
                             int nOctaveLayers=SIFT::CommonParams::DEFAULT_NOCTAVE_LAYERS,
                             int firstOctave=SIFT::CommonParams::DEFAULT_FIRST_OCTAVE,
                             int angleMode=SIFT::CommonParams::FIRST_ANGLE )*/;
    }
    else if( !descriptorExtractorType.compare( "SURF" ) )
    {
        de = new SurfDescriptorExtractor/*( int nOctaves=4, int nOctaveLayers=2, bool extended=false )*/;
    }
    else
    {
        //CV_Error( CV_StsBadArg, "unsupported descriptor extractor type");
    }
    return de;
}

/****************************************************************************************\
*                                      DescriptorMatcher                                 *
\****************************************************************************************/
void DescriptorMatcher::DescriptorCollection::set( const vector<Mat>& descCollection )
{
    clear();

    size_t imageCount = descCollection.size();
    CV_Assert( imageCount > 0 );

    startIdxs.resize( imageCount );

    int dim;
    int type;
    startIdxs[0] = 0;
    for( size_t i = 1; i < imageCount; i++ )
    {
        int s = 0;
        if( !descCollection[i-1].empty() )
        {
            dim = descCollection[i-1].cols;
            type = descCollection[i-1].type();
            s = descCollection[i-1].rows;
        }
        startIdxs[i] = startIdxs[i-1] + s;
    }
    int count = startIdxs[imageCount-1] + descCollection[imageCount-1].rows;

    if( count > 0 )
    {
        dmatrix.create( count, dim, type );
        for( size_t i = 0; i < imageCount; i++ )
        {
            if( !descCollection[i].empty() )
            {
                CV_Assert( descCollection[i].cols == dim && descCollection[i].type() == type );
                Mat m = dmatrix.rowRange( startIdxs[i], startIdxs[i] + descCollection[i].rows );
                descCollection[i].copyTo(m);
            }
        }
    }
}

void DescriptorMatcher::DescriptorCollection::clear()
{
    startIdxs.clear();
    dmatrix.release();
}

const Mat DescriptorMatcher::DescriptorCollection::getDescriptor( int imgIdx, int localDescIdx ) const
{
    CV_Assert( imgIdx < (int)startIdxs.size() );
    int globalIdx = startIdxs[imgIdx] + localDescIdx;
    CV_Assert( globalIdx < (int)size() );

    return getDescriptor( globalIdx );
}

const Mat DescriptorMatcher::DescriptorCollection::getDescriptor( int globalDescIdx ) const
{
    CV_Assert( globalDescIdx < size() );
    return dmatrix.row( globalDescIdx );
}

void DescriptorMatcher::DescriptorCollection::getLocalIdx( int globalDescIdx, int& imgIdx, int& localDescIdx ) const
{
    imgIdx = -1;
    CV_Assert( globalDescIdx < size() );
    for( size_t i = 1; i < startIdxs.size(); i++ )
    {
        if( globalDescIdx < startIdxs[i] )
        {
            imgIdx = i - 1;
            break;
        }
    }
    imgIdx = imgIdx == -1 ? startIdxs.size() -1 : imgIdx;
    localDescIdx = globalDescIdx - startIdxs[imgIdx];
}

/*
 * DescriptorMatcher
 */
void convertMatches( const vector<vector<DMatch> >& knnMatches, vector<DMatch>& matches )
{
    matches.clear();
    matches.reserve( knnMatches.size() );
    for( size_t i = 0; i < knnMatches.size(); i++ )
    {
        CV_Assert( knnMatches[i].size() <= 1 );
        if( !knnMatches[i].empty() )
            matches.push_back( knnMatches[i][0] );
    }
}

void DescriptorMatcher::add( const vector<Mat>& descCollection )
{
    trainDescCollection.insert( trainDescCollection.begin(), descCollection.begin(), descCollection.end() );
}

void DescriptorMatcher::clear()
{
    trainDescCollection.clear();
}

void DescriptorMatcher::match( const Mat& queryDescs, const Mat& trainDescs, vector<DMatch>& matches, const Mat& mask ) const
{
    Ptr<DescriptorMatcher> tempMatcher = createEmptyMatcherCopy();
    tempMatcher->add( vector<Mat>(1, trainDescs) );
    tempMatcher->match( queryDescs, matches, vector<Mat>(1, mask) );
}

void DescriptorMatcher::knnMatch( const Mat& queryDescs, const Mat& trainDescs, vector<vector<DMatch> >& matches, int knn,
                                  const Mat& mask, bool compactResult ) const
{
    Ptr<DescriptorMatcher> tempMatcher = createEmptyMatcherCopy();
    tempMatcher->add( vector<Mat>(1, trainDescs) );
    tempMatcher->knnMatch( queryDescs, matches, knn, vector<Mat>(1, mask), compactResult );
}

void DescriptorMatcher::radiusMatch( const Mat& queryDescs, const Mat& trainDescs, vector<vector<DMatch> >& matches, float maxDistance,
                                     const Mat& mask, bool compactResult ) const
{
    Ptr<DescriptorMatcher> tempMatcher = createEmptyMatcherCopy();
    tempMatcher->add( vector<Mat>(1, trainDescs) );
    tempMatcher->radiusMatch( queryDescs, matches, maxDistance, vector<Mat>(1, mask), compactResult );
}

void DescriptorMatcher::match( const Mat& queryDescs, vector<DMatch>& matches, const vector<Mat>& masks )
{
    vector<vector<DMatch> > knnMatches;
    knnMatch( queryDescs, knnMatches, 1, masks, true /*compactResult*/ );
    convertMatches( knnMatches, matches );
}

void DescriptorMatcher::knnMatch( const Mat& queryDescs, vector<vector<DMatch> >& matches, int knn,
                                  const vector<Mat>& masks, bool compactResult )
{
    train();
    knnMatchImpl( queryDescs, matches, knn, masks, compactResult );
}

void DescriptorMatcher::radiusMatch( const Mat& queryDescs, vector<vector<DMatch> >& matches, float maxDistance,
                                     const vector<Mat>& masks, bool compactResult )
{
    train();
    radiusMatchImpl( queryDescs, matches, maxDistance, masks, compactResult );
}

/*
 * BruteForceMatcher L2 specialization
 */
//template<>
//void BruteForceMatcher<L2<float> >::matchImpl( const Mat& query, const Mat& train, vector<DMatch>& matches, const Mat& mask ) const
//{
//    assert( mask.empty() || (mask.rows == query.rows && mask.cols == train.rows) );
//    assert( query.cols == train.cols ||  query.empty() ||  train.empty() );

//    matches.clear();
//    matches.reserve( query.rows );
//#if (!defined HAVE_EIGEN2)
//    Mat norms;
//    cv::reduce( train.mul( train ), norms, 1, 0);
//    norms = norms.t();
//    Mat desc_2t = train.t();
//    for( int i=0;i<query.rows;i++ )
//    {
//        Mat distances = (-2)*query.row(i)*desc_2t;
//        distances += norms;
//        DMatch match;
//        match.trainDescIdx = -1;
//        double minVal;
//        Point minLoc;
//        if( mask.empty() )
//        {
//            minMaxLoc ( distances, &minVal, 0, &minLoc );
//        }
//        else
//        {
//            minMaxLoc ( distances, &minVal, 0, &minLoc, 0, mask.row( i ) );
//        }
//        match.trainDescIdx = minLoc.x;

//        if( match.trainDescIdx != -1 )
//        {
//            match.queryDescIdx = i;
//            double queryNorm = norm( query.row(i) );
//            match.distance = (float)sqrt( minVal + queryNorm*queryNorm );
//            matches.push_back( match );
//        }
//    }

//#else
//    Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic> desc1t;
//    Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic> desc2;
//    cv2eigen( query.t(), desc1t);
//    cv2eigen( train, desc2 );

//    Eigen::Matrix<float, Eigen::Dynamic, 1> norms = desc2.rowwise().squaredNorm() / 2;

//    if( mask.empty() )
//    {
//        for( int i=0;i<query.rows;i++ )
//        {
//            Eigen::Matrix<float, Eigen::Dynamic, 1> distances = desc2*desc1t.col(i);
//            distances -= norms;
//            DMatch match;
//            match.queryDescIdx = i;
//            match.distance = sqrt( (-2)*distances.maxCoeff( &match.trainDescIdx ) + desc1t.col(i).squaredNorm() );
//            matches.push_back( match );
//        }
//    }
//    else
//    {
//        for( int i=0;i<query.rows;i++ )
//        {
//            Eigen::Matrix<float, Eigen::Dynamic, 1> distances = desc2*desc1t.col(i);
//            distances -= norms;

//            float maxCoeff = -std::numeric_limits<float>::max();
//            DMatch match;
//            match.trainDescIdx = -1;
//            for( int j=0;j<train.rows;j++ )
//            {
//                if( possibleMatch( mask, i, j ) && distances( j, 0 ) > maxCoeff )
//                {
//                    maxCoeff = distances( j, 0 );
//                    match.trainDescIdx = j;
//                }
//            }

//            if( match.trainDescIdx != -1 )
//            {
//                match.queryDescIdx = i;
//                match.distance = sqrt( (-2)*maxCoeff + desc1t.col(i).squaredNorm() );
//                matches.push_back( match );
//            }
//        }
//    }
//#endif
//}

/*
 * Flann based matcher
 */
FlannBasedMatcher::FlannBasedMatcher( const Ptr<flann::IndexParams>& _indexParams, const Ptr<flann::SearchParams>& _searchParams )
    : indexParams(_indexParams), searchParams(_searchParams), addedDescCount(0)
{
    CV_Assert( !_indexParams.empty() );
    CV_Assert( !_searchParams.empty() );
}

void FlannBasedMatcher::add( const vector<Mat>& descCollection )
{
    DescriptorMatcher::add( descCollection );
    addedDescCount += descCollection.size();
}

void FlannBasedMatcher::clear()
{
    DescriptorMatcher::clear();

    mergedDescriptors.clear();
    flannIndex.release();

    addedDescCount = 0;
}

void FlannBasedMatcher::train()
{
    if( flannIndex.empty() || mergedDescriptors.size() < addedDescCount )
    {
        mergedDescriptors.set( trainDescCollection );
        flannIndex = new flann::Index( mergedDescriptors.getDescriptors(), *indexParams );
    }
}

void FlannBasedMatcher::convertToDMatches( const DescriptorCollection* collection, const Mat& indices, const Mat& dists,
                                           vector<vector<DMatch> >& matches )
{
    matches.resize( indices.rows );
    for( int i = 0; i < indices.rows; i++ )
    {
        for( int j = 0; j < indices.cols; j++ )
        {
            int idx = indices.at<int>(i, j);
            assert( idx >= 0 ); // TODO check
            if( idx >= 0 )
            {
                int imgIdx = -1;
                int trainIdx = idx;
                if( collection )
                    collection->getLocalIdx( idx, imgIdx, trainIdx );
                matches[i].push_back( DMatch( i, trainIdx, imgIdx, dists.at<float>(i,j)) );
            }
        }
    }
}

void FlannBasedMatcher::knnMatchImpl( const Mat& queryDescs, vector<vector<DMatch> >& matches, int knn,
                                      const vector<Mat>& /*masks*/, bool /*compactResult*/ )
{
    train();

    Mat indices( queryDescs.rows, knn, CV_32SC1 );
    Mat dists( queryDescs.rows, knn, CV_32FC1);
    flannIndex->knnSearch( queryDescs, indices, dists, knn, *searchParams );

    convertToDMatches( &mergedDescriptors, indices, dists, matches );
}

void FlannBasedMatcher::radiusMatchImpl( const Mat& queryDescs, vector<vector<DMatch> >& matches, float maxDistance,
                                         const vector<Mat>& /*masks*/, bool /*compactResult*/ )
{
    train();

    const int count = trainDescCollection.size(); // TODO do count as param?
    Mat indices( queryDescs.rows, count, CV_32SC1 );
    Mat dists( queryDescs.rows, count, CV_32FC1);
    flannIndex->radiusSearch( queryDescs, indices, dists, maxDistance, *searchParams );

    convertToDMatches( &mergedDescriptors, indices, dists, matches );
}

/*
 * Factory function for DescriptorMatcher creating
 */
Ptr<DescriptorMatcher> createDescriptorMatcher( const string& descriptorMatcherType )
{
    DescriptorMatcher* dm = 0;
    if( !descriptorMatcherType.compare( "BruteForce" ) )
    {
        dm = new BruteForceMatcher<L2<float> >();
    }
    else if ( !descriptorMatcherType.compare( "BruteForce-L1" ) )
    {
        dm = new BruteForceMatcher<L1<float> >();
    }
    else if ( !descriptorMatcherType.compare( "FlannBased" ) )
    {
        dm = new FlannBasedMatcher();
    }
    else
    {
        //CV_Error( CV_StsBadArg, "unsupported descriptor matcher type");
    }

    return dm;
}

/****************************************************************************************\
*                                GenericDescriptorMatch                                  *
\****************************************************************************************/
/*
 * KeyPointCollection
 */
void GenericDescriptorMatcher::KeyPointCollection::add( const vector<Mat>& _images,
                                                        const vector<vector<KeyPoint> >& _points )
{
    CV_Assert( !_images.empty() );
    CV_Assert( _images.size() == _points.size() );

    images.insert( images.end(), _images.begin(), _images.end() );
    points.insert( points.end(), _points.begin(), _points.end() );
    for( size_t i = 0; i < _points.size(); i++ )
        size += _points[i].size();

    size_t prevSize = startIndices.size(), addSize = _images.size();
    startIndices.resize( prevSize + addSize );

    if( prevSize == 0 )
        startIndices[prevSize] = 0; //first
    else
        startIndices[prevSize] = startIndices[prevSize-1] + points[prevSize-1].size();

    for( size_t i = prevSize + 1; i < prevSize + addSize; i++ )
    {
        startIndices[i] = startIndices[i - 1] + points[i - 1].size();
    }
}

void GenericDescriptorMatcher::KeyPointCollection::clear()
{
    points.clear();
}

const KeyPoint& GenericDescriptorMatcher::KeyPointCollection::getKeyPoint( int imgIdx, int localPointIdx ) const
{
    CV_Assert( imgIdx < (int)images.size() );
    CV_Assert( localPointIdx < (int)points[imgIdx].size() );
    return points[imgIdx][localPointIdx];
}

const KeyPoint& GenericDescriptorMatcher::KeyPointCollection::getKeyPoint( int globalPointIdx ) const
{
    int imgIdx, localPointIdx;
    getLocalIdx( globalPointIdx, imgIdx, localPointIdx );
    return points[imgIdx][localPointIdx];
}

void GenericDescriptorMatcher::KeyPointCollection::getLocalIdx( int globalPointIdx, int& imgIdx, int& localPointIdx ) const
{
    imgIdx = -1;
    CV_Assert( globalPointIdx < (int)pointCount() );
    for( size_t i = 1; i < startIndices.size(); i++ )
    {
        if( globalPointIdx < startIndices[i] )
        {
            imgIdx = i - 1;
            break;
        }
    }
    imgIdx = imgIdx == -1 ? startIndices.size() -1 : imgIdx;
    localPointIdx = globalPointIdx - startIndices[imgIdx];
}

/*
 * GenericDescriptorMatcher
 */
void GenericDescriptorMatcher::add( const vector<Mat>& imgCollection,
                                    vector<vector<KeyPoint> >& pointCollection )
{
    trainPointCollection.add( imgCollection, pointCollection );
}

void GenericDescriptorMatcher::clear()
{
    trainPointCollection.clear();
}

void GenericDescriptorMatcher::classify( const Mat& queryImage, vector<KeyPoint>& queryPoints,
                                         const Mat& trainImage, vector<KeyPoint>& trainPoints ) const
{
    vector<DMatch> matches;
    match( queryImage, queryPoints, trainImage, trainPoints, matches );

    // remap keypoint indices to descriptors
    for( size_t i = 0; i < matches.size(); i++ )
        queryPoints[matches[i].queryDescIdx].class_id = trainPoints[matches[i].trainDescIdx].class_id;
}

void GenericDescriptorMatcher::classify( const Mat& queryImage, vector<KeyPoint>& queryPoints )
{
    vector<DMatch> matches;
    match( queryImage, queryPoints, matches );

    // remap keypoint indices to descriptors
    for( size_t i = 0; i < matches.size(); i++ )
        queryPoints[matches[i].queryDescIdx].class_id = trainPointCollection.getKeyPoint( matches[i].trainImgIdx, matches[i].trainDescIdx ).class_id;
}

void GenericDescriptorMatcher::match( const Mat& queryImg, vector<KeyPoint>& queryPoints,
                                      const Mat& trainImg, vector<KeyPoint>& trainPoints,
                                      vector<DMatch>& matches, const Mat& mask ) const
{
    Ptr<GenericDescriptorMatcher> tempMatcher = createEmptyMatcherCopy();
    vector<vector<KeyPoint> > vecTrainPoints(1, trainPoints);
    tempMatcher->add( vector<Mat>(1, trainImg), vecTrainPoints );
    tempMatcher->match( queryImg, queryPoints, matches, vector<Mat>(1, mask) );
    vecTrainPoints[0].swap( trainPoints );
}

void GenericDescriptorMatcher::knnMatch( const Mat& queryImg, vector<KeyPoint>& queryPoints,
                                         const Mat& trainImg, vector<KeyPoint>& trainPoints,
                                         vector<vector<DMatch> >& matches, int knn, const Mat& mask, bool compactResult ) const
{
    Ptr<GenericDescriptorMatcher> tempMatcher = createEmptyMatcherCopy();
    vector<vector<KeyPoint> > vecTrainPoints(1, trainPoints);
    tempMatcher->add( vector<Mat>(1, trainImg), vecTrainPoints );
    tempMatcher->knnMatch( queryImg, queryPoints, matches, knn, vector<Mat>(1, mask), compactResult );
    vecTrainPoints[0].swap( trainPoints );
}

void GenericDescriptorMatcher::radiusMatch( const Mat& queryImg, vector<KeyPoint>& queryPoints,
                                            const Mat& trainImg, vector<KeyPoint>& trainPoints,
                                            vector<vector<DMatch> >& matches, float maxDistance,
                                            const Mat& mask, bool compactResult ) const
{
    Ptr<GenericDescriptorMatcher> tempMatcher = createEmptyMatcherCopy();
    vector<vector<KeyPoint> > vecTrainPoints(1, trainPoints);
    tempMatcher->add( vector<Mat>(1, trainImg), vecTrainPoints );
    tempMatcher->radiusMatch( queryImg, queryPoints, matches, maxDistance, vector<Mat>(1, mask), compactResult );
    vecTrainPoints[0].swap( trainPoints );
}

void GenericDescriptorMatcher::match( const Mat& queryImg, vector<KeyPoint>& queryPoints,
                                      vector<DMatch>& matches, const vector<Mat>& masks )
{
    vector<vector<DMatch> > knnMatches;
    knnMatch( queryImg, queryPoints, knnMatches, 1, masks, false );
    convertMatches( knnMatches, matches );
}

void GenericDescriptorMatcher::knnMatch( const Mat& queryImg, vector<KeyPoint>& queryPoints,
                                         vector<vector<DMatch> >& matches, int knn,
                                         const vector<Mat>& masks, bool compactResult )
{
    train();
    knnMatchImpl( queryImg, queryPoints, matches, knn, masks, compactResult );
}

void GenericDescriptorMatcher::radiusMatch( const Mat& queryImg, vector<KeyPoint>& queryPoints,
                                            vector<vector<DMatch> >& matches, float maxDistance,
                                            const vector<Mat>& masks, bool compactResult )
{
    train();
    radiusMatchImpl( queryImg, queryPoints, matches, maxDistance, masks, compactResult );
}

/*
 * Factory function for GenericDescriptorMatch creating
 */
Ptr<GenericDescriptorMatcher> createGenericDescriptorMatcher( const string& genericDescritptorMatcherType, const string &paramsFilename )
{
    GenericDescriptorMatcher *descriptorMatcher = 0;
    if( ! genericDescritptorMatcherType.compare("ONEWAY") )
    {
        descriptorMatcher = new OneWayDescriptorMatcher();
    }
    else if( ! genericDescritptorMatcherType.compare("FERN") )
    {
        descriptorMatcher = new FernDescriptorMatcher();
    }

    if( !paramsFilename.empty() && descriptorMatcher != 0 )
    {
        FileStorage fs = FileStorage( paramsFilename, FileStorage::READ );
        if( fs.isOpened() )
        {
            descriptorMatcher->read( fs.root() );
            fs.release();
        }
    }

    return descriptorMatcher;
}

/****************************************************************************************\
*                                OneWayDescriptorMatch                                  *
\****************************************************************************************/
OneWayDescriptorMatcher::OneWayDescriptorMatcher( const Params& _params)
{
    initialize(_params);
}

OneWayDescriptorMatcher::~OneWayDescriptorMatcher()
{}

void OneWayDescriptorMatcher::initialize( const Params& _params, const Ptr<OneWayDescriptorBase>& _base )
{
    clear();

    if( _base.empty() )
        base = _base;

    params = _params;
}

void OneWayDescriptorMatcher::clear()
{
    GenericDescriptorMatcher::clear();

    prevTrainCount = 0;
    base->clear();
}

void OneWayDescriptorMatcher::train()
{
    if( base.empty() || prevTrainCount < (int)trainPointCollection.pointCount() )
    {
        base = new OneWayDescriptorObject( params.patchSize, params.poseCount, params.pcaFilename,
                                           params.trainPath, params.trainImagesList, params.minScale, params.maxScale, params.stepScale );

        base->Allocate( trainPointCollection.pointCount() );
        prevTrainCount = trainPointCollection.pointCount();

        const vector<vector<KeyPoint> >& points = trainPointCollection.getKeypoints();
        int count = 0;
        for( size_t i = 0; i < points.size(); i++ )
        {
            IplImage _image = trainPointCollection.getImage(i);
            for( size_t j = 0; j < points[i].size(); j++ )
                base->InitializeDescriptor( count++, &_image, points[i][j], "" );
        }

#if defined(_KDTREE)
        base->ConvertDescriptorsArrayToTree();
#endif
    }
}

void OneWayDescriptorMatcher::knnMatchImpl( const Mat& queryImg, vector<KeyPoint>& queryPoints,
                                            vector<vector<DMatch> >& matches, int knn,
                                            const vector<Mat>& /*masks*/, bool /*compactResult*/ )
{
    train();

    CV_Assert( knn == 1 ); // knn > 1 unsupported because of bug in OneWayDescriptorBase for this case

    matches.resize( queryPoints.size() );
    IplImage _qimage = queryImg;
    for( size_t i = 0; i < queryPoints.size(); i++ )
    {
        int descIdx = -1, poseIdx = -1;
        float distance;
        base->FindDescriptor( &_qimage, queryPoints[i].pt, descIdx, poseIdx, distance );
        matches[i].push_back( DMatch(i, descIdx, distance) );
    }
}

void OneWayDescriptorMatcher::radiusMatchImpl( const Mat& queryImg, vector<KeyPoint>& queryPoints,
                                               vector<vector<DMatch> >& matches, float maxDistance,
                                               const vector<Mat>& /*masks*/, bool /*compactResult*/ )
{
    train();

    matches.resize( queryPoints.size() );
    IplImage _qimage = queryImg;
    for( size_t i = 0; i < queryPoints.size(); i++ )
    {
        int descIdx = -1, poseIdx = -1;
        float distance;
        base->FindDescriptor( &_qimage, queryPoints[i].pt, descIdx, poseIdx, distance );
        if( distance < maxDistance )
            matches[i].push_back( DMatch(i, descIdx, distance) );
    }
}

void OneWayDescriptorMatcher::read( const FileNode &fn )
{
    base = new OneWayDescriptorObject( params.patchSize, params.poseCount, string (), string (), string (),
                                       params.minScale, params.maxScale, params.stepScale );
    base->Read (fn);
}

void OneWayDescriptorMatcher::write( FileStorage& fs ) const
{
    base->Write (fs);
}

/****************************************************************************************\
*                                  FernDescriptorMatch                                   *
\****************************************************************************************/
FernDescriptorMatcher::Params::Params( int _nclasses, int _patchSize, int _signatureSize,
                                     int _nstructs, int _structSize, int _nviews, int _compressionMethod,
                                     const PatchGenerator& _patchGenerator ) :
    nclasses(_nclasses), patchSize(_patchSize), signatureSize(_signatureSize),
    nstructs(_nstructs), structSize(_structSize), nviews(_nviews),
    compressionMethod(_compressionMethod), patchGenerator(_patchGenerator)
{}

FernDescriptorMatcher::Params::Params( const string& _filename )
{
    filename = _filename;
}

FernDescriptorMatcher::FernDescriptorMatcher( const Params& _params )
{
    prevTrainCount = 0;
    params = _params;
    if( !params.filename.empty() )
    {
        classifier = new FernClassifier;
        FileStorage fs(params.filename, FileStorage::READ);
        if( fs.isOpened() )
            classifier->read( fs.getFirstTopLevelNode() );
    }
}

FernDescriptorMatcher::~FernDescriptorMatcher()
{}

void FernDescriptorMatcher::clear()
{
    GenericDescriptorMatcher::clear();

    classifier.release();
    prevTrainCount = 0;
}

void FernDescriptorMatcher::train()
{
    if( classifier.empty() || prevTrainCount < (int)trainPointCollection.pointCount() )
    {
        assert( params.filename.empty() );

        vector<vector<Point2f> > points;
        for( size_t imgIdx = 0; imgIdx < trainPointCollection.imageCount(); imgIdx++ )
            KeyPoint::convert( trainPointCollection.getKeypoints(imgIdx), points[imgIdx] );

        classifier = new FernClassifier( points, trainPointCollection.getImages(), vector<vector<int> >(), 0, // each points is a class
                                         params.patchSize, params.signatureSize, params.nstructs, params.structSize,
                                         params.nviews, params.compressionMethod, params.patchGenerator );
    }
}

void FernDescriptorMatcher::calcBestProbAndMatchIdx( const Mat& image, const Point2f& pt,
                                                     float& bestProb, int& bestMatchIdx, vector<float>& signature )
{
    (*classifier)( image, pt, signature);

    bestProb = -FLT_MAX;
    bestMatchIdx = -1;
    for( int ci = 0; ci < classifier->getClassCount(); ci++ )
    {
        if( signature[ci] > bestProb )
        {
            bestProb = signature[ci];
            bestMatchIdx = ci;
        }
    }
}

void FernDescriptorMatcher::knnMatchImpl( const Mat& queryImg, vector<KeyPoint>& queryPoints,
                                          vector<vector<DMatch> >& matches, int knn,
                                          const vector<Mat>& /*masks*/, bool /*compactResult*/ )
{
    train();

    matches.resize( queryPoints.size() );
    vector<float> signature( (size_t)classifier->getClassCount() );

    for( size_t queryIdx = 0; queryIdx < queryPoints.size(); queryIdx++ )
    {
        (*classifier)( queryImg, queryPoints[queryIdx].pt, signature);

        for( int k = 0; k < knn; k++ )
        {
            DMatch bestMatch;
            size_t ci = 0;
            for( ; ci < signature.size(); ci++ )
            {
                if( -signature[ci] < bestMatch.distance )
                {
                    int imgIdx = -1, trainIdx = -1;
                    trainPointCollection.getLocalIdx( ci , imgIdx, trainIdx );
                    bestMatch = DMatch( queryIdx, trainIdx, imgIdx, -signature[ci] );
                }
            }

            if( bestMatch.trainDescIdx == -1 )
                break;
            signature[ci] = std::numeric_limits<float>::min();
            matches[queryIdx].push_back( bestMatch );
        }
    }
}

void FernDescriptorMatcher::radiusMatchImpl( const Mat& queryImg, vector<KeyPoint>& queryPoints,
                                             vector<vector<DMatch> >& matches, float maxDistance,
                                             const vector<Mat>& /*masks*/, bool /*compactResult*/ )
{
    train();
    matches.resize( queryPoints.size() );
    vector<float> signature( (size_t)classifier->getClassCount() );

    for( size_t i = 0; i < queryPoints.size(); i++ )
    {
        (*classifier)( queryImg, queryPoints[i].pt, signature);

        for( int ci = 0; ci < classifier->getClassCount(); ci++ )
        {
            if( -signature[ci] < maxDistance )
            {
                int imgIdx = -1, trainIdx = -1;
                trainPointCollection.getLocalIdx( ci , imgIdx, trainIdx );
                matches[i].push_back( DMatch( i, trainIdx, imgIdx, -signature[ci] ) );
            }
        }
    }
}

void FernDescriptorMatcher::read( const FileNode &fn )
{
    params.nclasses = fn["nclasses"];
    params.patchSize = fn["patchSize"];
    params.signatureSize = fn["signatureSize"];
    params.nstructs = fn["nstructs"];
    params.structSize = fn["structSize"];
    params.nviews = fn["nviews"];
    params.compressionMethod = fn["compressionMethod"];

    //classifier->read(fn);
}

void FernDescriptorMatcher::write( FileStorage& fs ) const
{
    fs << "nclasses" << params.nclasses;
    fs << "patchSize" << params.patchSize;
    fs << "signatureSize" << params.signatureSize;
    fs << "nstructs" << params.nstructs;
    fs << "structSize" << params.structSize;
    fs << "nviews" << params.nviews;
    fs << "compressionMethod" << params.compressionMethod;

//    classifier->write(fs);
}

/****************************************************************************************\
*                                  VectorDescriptorMatch                                 *
\****************************************************************************************/
void VectorDescriptorMatcher::add( const vector<Mat>& imgCollection,
                                   vector<vector<KeyPoint> >& pointCollection )
{
    clear();
    vector<Mat> descCollection;
    extractor->compute( imgCollection, pointCollection, descCollection );

    matcher->add( descCollection );

    trainPointCollection.add( imgCollection, pointCollection );
}

void VectorDescriptorMatcher::clear()
{
    extractor->clear();
    matcher->clear();
    GenericDescriptorMatcher::clear();
}

void VectorDescriptorMatcher::train()
{
    matcher->train();
}

void VectorDescriptorMatcher::knnMatchImpl( const Mat& queryImg, vector<KeyPoint>& queryPoints,
                                            vector<vector<DMatch> >& matches, int knn,
                                            const vector<Mat>& masks, bool compactResult )
{
    Mat queryDescs;
    extractor->compute( queryImg, queryPoints, queryDescs );
    matcher->knnMatch( queryDescs, matches, knn, masks, compactResult );
}

void VectorDescriptorMatcher::radiusMatchImpl( const Mat& queryImg, vector<KeyPoint>& queryPoints,
                                               vector<vector<DMatch> >& matches, float maxDistance,
                                               const vector<Mat>& masks, bool compactResult )
{
    Mat queryDescs;
    extractor->compute( queryImg, queryPoints, queryDescs );
    matcher->radiusMatch( queryDescs, matches, maxDistance, masks, compactResult );
}

void VectorDescriptorMatcher::read( const FileNode& fn )
{
    GenericDescriptorMatcher::read(fn);
    extractor->read (fn);
}

void VectorDescriptorMatcher::write (FileStorage& fs) const
{
    GenericDescriptorMatcher::write(fs);
    extractor->write (fs);
}

}
