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

/****************************************************************************************\
*           Factory functions for descriptor extractor and matcher creating              *
\****************************************************************************************/

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
*                                      DescriptorMatcher                                 *
\****************************************************************************************/
void DescriptorMatcher::DescriptorCollection::set( const vector<Mat>& descCollection )
{
    clear();

    int imageCount = descCollection.size();
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
    CV_Assert( imgIdx < startIdxs.size() );
    int globalIdx = startIdxs[imgIdx] + localDescIdx;
    CV_Assert( globalIdx < size() );

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
        if( !knnMatches.empty() )
            matches.push_back( knnMatches[i][0] );
    }
}

void DescriptorMatcher::match( const Mat& queryDescs, const Mat& trainDescs, vector<DMatch>& matches, const Mat& mask ) const
{
    vector<vector<DMatch> > knnMatches;
    knnMatchImpl( queryDescs, trainDescs, knnMatches, 1, mask, false );
    convertMatches( knnMatches, matches );
}

void DescriptorMatcher::knnMatch( const Mat& queryDescs, const Mat& trainDescs, vector<vector<DMatch> >& matches, int knn,
                                  const Mat& mask, bool equalSizes ) const
{
    knnMatchImpl( queryDescs, trainDescs, matches, knn, mask, equalSizes );
}

void DescriptorMatcher::radiusMatch( const Mat& queryDescs, const Mat& trainDescs, vector<vector<DMatch> >& matches, float maxDistance,
                                     const Mat& mask, bool equalSizes ) const
{
    radiusMatchImpl( queryDescs, trainDescs, matches, maxDistance, mask, equalSizes );
}

void DescriptorMatcher::match( const Mat& queryDescs, vector<DMatch>& matches, const vector<Mat>& masks )
{
    vector<vector<DMatch> > knnMatches;
    knnMatchImpl( queryDescs, knnMatches, 1, masks, false );
    convertMatches( knnMatches, matches );
}

void DescriptorMatcher::knnMatch( const Mat& queryDescs, vector<vector<DMatch> >& matches, int knn,
                                  const vector<Mat>& masks, bool equalSizes )
{
    knnMatchImpl( queryDescs, matches, knn, masks, equalSizes );
}

void DescriptorMatcher::radiusMatch( const Mat& queryDescs, vector<vector<DMatch> >& matches, float maxDistance,
                                     const vector<Mat>& masks, bool equalSizes )
{
    radiusMatchImpl( queryDescs, matches, maxDistance, masks, equalSizes );
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
    : indexParams(_indexParams), searchParams(_searchParams)
{
    CV_Assert( !_indexParams.empty() );
    CV_Assert( !_searchParams.empty() );
}

void FlannBasedMatcher::setTrainCollection( const vector<Mat>& descCollection )
{
    trainDescCollection.set( descCollection );
    flannIndex = new flann::Index( trainDescCollection.getDescriptors(), *indexParams );
}

void FlannBasedMatcher::clear()
{
    trainDescCollection.clear();
    flannIndex.release();
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

void FlannBasedMatcher::knnMatchImpl( const Mat& queryDescs, const Mat& trainDescs, vector<vector<DMatch> >& matches, int knn,
                                      const Mat& mask, bool /*equalSizes*/ ) const
{
    CV_Assert( mask.empty() ); // unsupported

    Ptr<flann::Index> tempFlannIndex = new flann::Index( trainDescs, *indexParams );

    Mat indices( queryDescs.rows, knn, CV_32SC1 );
    Mat dists( queryDescs.rows, knn, CV_32FC1);
    tempFlannIndex->knnSearch( queryDescs, indices, dists, knn, *searchParams );

    convertToDMatches( 0, indices, dists, matches );
}

void FlannBasedMatcher::radiusMatchImpl( const Mat& queryDescs, const Mat& trainDescs, vector<vector<DMatch> >& matches, float maxDistance,
                                         const Mat& mask, bool /*equalSizes*/ ) const
{
    CV_Assert( mask.empty() ); // unsupported

    Ptr<flann::Index> tempFlannIndex = new flann::Index( trainDescs, *indexParams );

    const int count = trainDescs.rows; // TODO do count as param?
    Mat indices( queryDescs.rows, count, CV_32SC1 );
    Mat dists( queryDescs.rows, count, CV_32FC1);
    tempFlannIndex->radiusSearch( queryDescs, indices, dists, maxDistance, *searchParams );

    convertToDMatches( 0, indices, dists, matches );
}

void FlannBasedMatcher::knnMatchImpl( const Mat& queryDescs, vector<vector<DMatch> >& matches, int knn,
                                      const vector<Mat>& masks, bool /*equalSizes*/ )
{
    CV_Assert( masks.empty() );
    Mat indices( queryDescs.rows, knn, CV_32SC1 );
    Mat dists( queryDescs.rows, knn, CV_32FC1);
    flannIndex->knnSearch( queryDescs, indices, dists, knn, *searchParams );

    convertToDMatches( &trainDescCollection, indices, dists, matches );
}

void FlannBasedMatcher::radiusMatchImpl( const Mat& queryDescs, vector<vector<DMatch> >& matches, float maxDistance,
                                         const vector<Mat>& masks, bool /*equalSizes*/ )
{
    CV_Assert( masks.empty() );
    const int count = trainDescCollection.size(); // TODO do count as param?
    Mat indices( queryDescs.rows, count, CV_32SC1 );
    Mat dists( queryDescs.rows, count, CV_32FC1);
    flannIndex->radiusSearch( queryDescs, indices, dists, maxDistance, *searchParams );

    convertToDMatches( &trainDescCollection, indices, dists, matches );
}

/****************************************************************************************\
*                                GenericDescriptorMatch                                  *
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
        startIndices.push_back((int)(*startIndices.rbegin() + points.rbegin()->size()));

    // add image and keypoints
    images.push_back(_image);
    points.push_back(_points);
}

KeyPoint KeyPointCollection::getKeyPoint( int index ) const
{
    size_t i = 0;
    for(; i < startIndices.size() && startIndices[i] <= index; i++);
    i--;
    assert(i < startIndices.size() && (size_t)index - startIndices[i] < points[i].size());

    return points[i][index - startIndices[i]];
}

size_t KeyPointCollection::calcKeypointCount() const
{
    if( startIndices.empty() )
        return 0;
    return *startIndices.rbegin() + points.rbegin()->size();
}

void KeyPointCollection::clear()
{
    images.clear();
    points.clear();
    startIndices.clear();
}

/*
 * GenericDescriptorMatch
 */

void GenericDescriptorMatcher::match( const Mat&, vector<KeyPoint>&, vector<DMatch>& )
{
}

void GenericDescriptorMatcher::match( const Mat&, vector<KeyPoint>&, vector<vector<DMatch> >&, float )
{
}

void GenericDescriptorMatcher::add( KeyPointCollection& collection )
{
    for( size_t i = 0; i < collection.images.size(); i++ )
        add( collection.images[i], collection.points[i] );
}

void GenericDescriptorMatcher::classify( const Mat& image, vector<cv::KeyPoint>& points )
{
    vector<int> keypointIndices;
    match( image, points, keypointIndices );

    // remap keypoint indices to descriptors
    for( size_t i = 0; i < keypointIndices.size(); i++ )
        points[i].class_id = collection.getKeyPoint(keypointIndices[i]).class_id;
};

void GenericDescriptorMatcher::clear()
{
    collection.clear();
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
        FernDescriptorMatcher::Params params;
        params.signatureSize = numeric_limits<int>::max();
        descriptorMatcher = new FernDescriptorMatcher(params);
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
OneWayDescriptorMatcher::OneWayDescriptorMatcher()
{}

OneWayDescriptorMatcher::OneWayDescriptorMatcher( const Params& _params)
{
    initialize(_params);
}

OneWayDescriptorMatcher::~OneWayDescriptorMatcher()
{}

void OneWayDescriptorMatcher::initialize( const Params& _params, OneWayDescriptorBase *_base)
{
    base.release();
    if (_base != 0)
    {
        base = _base;
    }
    params = _params;
}

void OneWayDescriptorMatcher::add( const Mat& image, vector<KeyPoint>& keypoints )
{
    if( base.empty() )
        base = new OneWayDescriptorObject( params.patchSize, params.poseCount, params.pcaFilename,
                                           params.trainPath, params.trainImagesList, params.minScale, params.maxScale, params.stepScale);

    size_t trainFeatureCount = keypoints.size();

    base->Allocate( (int)trainFeatureCount );

    IplImage _image = image;
    for( size_t i = 0; i < keypoints.size(); i++ )
        base->InitializeDescriptor( (int)i, &_image, keypoints[i], "" );

    collection.add( Mat(), keypoints );

#if defined(_KDTREE)
    base->ConvertDescriptorsArrayToTree();
#endif
}

void OneWayDescriptorMatcher::add( KeyPointCollection& keypoints )
{
    if( base.empty() )
        base = new OneWayDescriptorObject( params.patchSize, params.poseCount, params.pcaFilename,
                                           params.trainPath, params.trainImagesList, params.minScale, params.maxScale, params.stepScale);

    size_t trainFeatureCount = keypoints.calcKeypointCount();

    base->Allocate( (int)trainFeatureCount );

    int count = 0;
    for( size_t i = 0; i < keypoints.points.size(); i++ )
    {
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

void OneWayDescriptorMatcher::match( const Mat& image, vector<KeyPoint>& points, vector<int>& indices)
{
    vector<DMatch> matchings( points.size() );
    indices.resize(points.size());

    match( image, points, matchings );

    for( size_t i = 0; i < points.size(); i++ )
        indices[i] = matchings[i].trainDescIdx;
}

void OneWayDescriptorMatcher::match( const Mat& image, vector<KeyPoint>& points, vector<DMatch>& matches )
{
    matches.resize( points.size() );
    IplImage _image = image;
    for( size_t i = 0; i < points.size(); i++ )
    {
        int poseIdx = -1;

        DMatch match;
        match.queryDescIdx = (int)i;
        match.trainDescIdx = -1;
        base->FindDescriptor( &_image, points[i].pt, match.trainDescIdx, poseIdx, match.distance );
        matches[i] = match;
    }
}

void OneWayDescriptorMatcher::match( const Mat& image, vector<KeyPoint>& points, vector<vector<DMatch> >& matches, float /*threshold*/ )
{
    matches.clear();
    matches.resize( points.size() );

    vector<DMatch> dmatches;
    match( image, points, dmatches );
    for( size_t i=0;i<matches.size();i++ )
    {
        matches[i].push_back( dmatches[i] );
    }

    /*
    printf("Start matching %d points\n", points.size());
    //std::cout << "Start matching " << points.size() << "points\n";
    assert(collection.images.size() == 1);
    int n = collection.points[0].size();

    printf("n = %d\n", n);
    for( size_t i = 0; i < points.size(); i++ )
    {
        //printf("Matching %d\n", i);
        //int poseIdx = -1;

        DMatch match;
        match.indexQuery = i;
        match.indexTrain = -1;


        CvPoint pt = points[i].pt;
        CvRect roi = cvRect(cvRound(pt.x - 24/4),
                            cvRound(pt.y - 24/4),
                            24/2, 24/2);
        cvSetImageROI(&_image, roi);

        std::vector<int> desc_idxs;
        std::vector<int> pose_idxs;
        std::vector<float> distances;
        std::vector<float> _scales;


        base->FindDescriptor(&_image, n, desc_idxs, pose_idxs, distances, _scales);
        cvResetImageROI(&_image);

        for( int j=0;j<n;j++ )
        {
            match.indexTrain = desc_idxs[j];
            match.distance = distances[j];
            matches[i].push_back( match );
        }

        //sort( matches[i].begin(), matches[i].end(), compareIndexTrain );
        //for( int j=0;j<n;j++ )
        //{
            //printf( "%d %f;  ",matches[i][j].indexTrain, matches[i][j].distance);
        //}
        //printf("\n\n\n");



        //base->FindDescriptor( &_image, 100, points[i].pt, match.indexTrain, poseIdx, match.distance );
        //matches[i].push_back( match );
    }
    */
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

void OneWayDescriptorMatcher::classify( const Mat& image, vector<KeyPoint>& points )
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

void OneWayDescriptorMatcher::clear ()
{
    GenericDescriptorMatcher::clear();
    base->clear ();
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

FernDescriptorMatcher::FernDescriptorMatcher()
{}

FernDescriptorMatcher::FernDescriptorMatcher( const Params& _params )
{
    params = _params;
}

FernDescriptorMatcher::~FernDescriptorMatcher()
{}

void FernDescriptorMatcher::initialize( const Params& _params )
{
    classifier.release();
    params = _params;
    if( !params.filename.empty() )
    {
        classifier = new FernClassifier;
        FileStorage fs(params.filename, FileStorage::READ);
        if( fs.isOpened() )
            classifier->read( fs.getFirstTopLevelNode() );
    }
}

void FernDescriptorMatcher::add( const Mat& image, vector<KeyPoint>& keypoints )
{
    if( params.filename.empty() )
        collection.add( image, keypoints );
}

void FernDescriptorMatcher::trainFernClassifier()
{
    if( classifier.empty() )
    {
        assert( params.filename.empty() );

        vector<vector<Point2f> > points;
        for( size_t imgIdx = 0; imgIdx < collection.images.size(); imgIdx++ )
            KeyPoint::convert( collection.points[imgIdx], points[imgIdx] );

        classifier = new FernClassifier( points, collection.images, vector<vector<int> >(), 0, // each points is a class
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

void FernDescriptorMatcher::match( const Mat& image, vector<KeyPoint>& keypoints, vector<int>& indices )
{
    trainFernClassifier();

    indices.resize( keypoints.size() );
    vector<float> signature( (size_t)classifier->getClassCount() );

    for( size_t pi = 0; pi < keypoints.size(); pi++ )
    {
        //calcBestProbAndMatchIdx( image, keypoints[pi].pt, bestProb, indices[pi], signature );
        //TODO: use octave and image pyramid
        indices[pi] = (*classifier)(image, keypoints[pi].pt, signature);
    }
}

void FernDescriptorMatcher::match( const Mat& image, vector<KeyPoint>& keypoints, vector<DMatch>& matches )
{
    trainFernClassifier();

    matches.resize( keypoints.size() );
    vector<float> signature( (size_t)classifier->getClassCount() );

    for( int pi = 0; pi < (int)keypoints.size(); pi++ )
    {
        matches[pi].queryDescIdx = pi;
        calcBestProbAndMatchIdx( image, keypoints[pi].pt, matches[pi].distance, matches[pi].trainDescIdx, signature );
        //matching[pi].distance is log of probability so we need to transform it
        matches[pi].distance = -matches[pi].distance;
    }
}

void FernDescriptorMatcher::match( const Mat& image, vector<KeyPoint>& keypoints, vector<vector<DMatch> >& matches, float threshold )
{
    trainFernClassifier();

    matches.resize( keypoints.size() );
    vector<float> signature( (size_t)classifier->getClassCount() );

    for( int pi = 0; pi < (int)keypoints.size(); pi++ )
    {
        (*classifier)( image, keypoints[pi].pt, signature);

        DMatch match;
        match.queryDescIdx = pi;

        for( int ci = 0; ci < classifier->getClassCount(); ci++ )
        {
            if( -signature[ci] < threshold )
            {
                match.distance = -signature[ci];
                match.trainDescIdx = ci;
                matches[pi].push_back( match );
            }
        }
    }
}

void FernDescriptorMatcher::classify( const Mat& image, vector<KeyPoint>& keypoints )
{
    trainFernClassifier();

    vector<float> signature( (size_t)classifier->getClassCount() );
    for( size_t pi = 0; pi < keypoints.size(); pi++ )
    {
        float bestProb = 0;
        int bestMatchIdx = -1;
        calcBestProbAndMatchIdx( image, keypoints[pi].pt, bestProb, bestMatchIdx, signature );
        keypoints[pi].class_id = collection.getKeyPoint(bestMatchIdx).class_id;
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

void FernDescriptorMatcher::clear ()
{
    GenericDescriptorMatcher::clear();
    classifier.release();
}

/****************************************************************************************\
*                                  VectorDescriptorMatch                                 *
\****************************************************************************************/
void VectorDescriptorMatcher::add( const Mat& image, vector<KeyPoint>& keypoints )
{
//    Mat descriptors;
//    extractor->compute( image, keypoints, descriptors );
//    matcher->add( descriptors );

//    collection.add( Mat(), keypoints );
};

void VectorDescriptorMatcher::match( const Mat& image, vector<KeyPoint>& points, vector<int>& keypointIndices )
{
//    Mat descriptors;
//    extractor->compute( image, points, descriptors );

//    matcher->match( descriptors, keypointIndices );
};

void VectorDescriptorMatcher::match( const Mat& image, vector<KeyPoint>& points, vector<DMatch>& matches )
{
//    Mat descriptors;
//    extractor->compute( image, points, descriptors );

//    matcher->match( descriptors, matches );
}

void VectorDescriptorMatcher::match( const Mat& image, vector<KeyPoint>& points,
                                   vector<vector<DMatch> >& matches, float threshold )
{
//    Mat descriptors;
//    extractor->compute( image, points, descriptors );

//    matcher->match( descriptors, matches, threshold );
}

void VectorDescriptorMatcher::clear()
{
//    GenericDescriptorMatch::clear();
//    matcher->clear();
}

void VectorDescriptorMatcher::read( const FileNode& fn )
{
//    GenericDescriptorMatch::read(fn);
//    extractor->read (fn);
}

void VectorDescriptorMatcher::write (FileStorage& fs) const
{
//    GenericDescriptorMatch::write(fs);
//    extractor->write (fs);
}

}
