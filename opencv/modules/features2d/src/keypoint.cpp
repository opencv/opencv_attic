/*M///////////////////////////////////////////////////////////////////////////////////////
//
//  IMPORTANT: READ BEFORE DOWNLOADING, COPYING, INSTALLING OR USING.
//
//  By downloading, copying, installing or using the software you agree to this license.
//  If you do not agree to this license, do not download, install,
//  copy or use the software.
//
//
//                           License Agreement
//                For Open Source Computer Vision Library
//
// Copyright (C) 2008, Willow Garage Inc., all rights reserved.
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
#include <limits>

namespace cv
{
	

void write(FileStorage& fs, const string& objname, const vector<KeyPoint>& keypoints)
{
    WriteStructContext ws(fs, objname, CV_NODE_SEQ + CV_NODE_FLOW);
    
    int i, npoints = (int)keypoints.size();
    for( i = 0; i < npoints; i++ )
    {
        const KeyPoint& kpt = keypoints[i];
        write(fs, kpt.pt.x);
        write(fs, kpt.pt.y);
        write(fs, kpt.size);
        write(fs, kpt.angle);
        write(fs, kpt.response);
        write(fs, kpt.octave);
    }
}


void read(const FileNode& node, vector<KeyPoint>& keypoints)
{
    keypoints.resize(0);
    FileNodeIterator it = node.begin(), it_end = node.end();
    for( ; it != it_end; )
    {
        KeyPoint kpt;
        it >> kpt.pt.x >> kpt.pt.y >> kpt.size >> kpt.angle >> kpt.response >> kpt.octave;
        keypoints.push_back(kpt);
    }
}
    

void KeyPoint::convert(const std::vector<KeyPoint>& keypoints, std::vector<Point2f>& points2f,
                       const vector<int>& keypointIndexes)
{
    if( keypointIndexes.empty() )
    {
        points2f.resize( keypoints.size() );
        for( size_t i = 0; i < keypoints.size(); i++ )
            points2f[i] = keypoints[i].pt;
    }
    else
    {
        points2f.resize( keypointIndexes.size() );
        for( size_t i = 0; i < keypointIndexes.size(); i++ )
        {
            int idx = keypointIndexes[i];
            if( idx >= 0 )
                points2f[i] = keypoints[idx].pt;
            else
            {
                CV_Error( CV_StsBadArg, "keypointIndexes has element < 0. TODO: process this case" );
                //points2f[i] = Point2f(-1, -1);
            }
        }
    }
}
    
void KeyPoint::convert( const std::vector<Point2f>& points2f, std::vector<KeyPoint>& keypoints,
                        float size, float response, int octave, int class_id )
{
    for( size_t i = 0; i < points2f.size(); i++ )
        keypoints[i] = KeyPoint(points2f[i], size, -1, response, octave, class_id);
}

float KeyPoint::overlap( const KeyPoint& kp1, const KeyPoint& kp2 )
{
    float a = kp1.size * 0.5f;
    float b = kp2.size * 0.5f;
    float a_2 = a * a;
    float b_2 = b * b;

    Point2f p1 = kp1.pt;
    Point2f p2 = kp2.pt;
    float c = (float)norm( p1 - p2 );

    float ovrl = 0.f;

    // one circle is completely encovered by the other => no intersection points!
    if( min( a, b ) + c <= max( a, b ) )
        return min( a_2, b_2 ) / max( a_2, b_2 );

    if( c < a + b ) // circles intersect
    {
        float c_2 = c * c;
        float cosAlpha = ( b_2 + c_2 - a_2 ) / ( kp2.size * c );
        float cosBeta  = ( a_2 + c_2 - b_2 ) / ( kp1.size * c );
        float alpha = acos( cosAlpha );
        float beta = acos( cosBeta );
        float sinAlpha = sin(alpha);
        float sinBeta  = sin(beta);

        float segmentAreaA = a_2 * beta;
        float segmentAreaB = b_2 * alpha;

        float triangleAreaA = a_2 * sinBeta * cosBeta;
        float triangleAreaB = b_2 * sinAlpha * cosAlpha;

        float intersectionArea = segmentAreaA + segmentAreaB - triangleAreaA - triangleAreaB;
        float unionArea = (a_2 + b_2) * (float)CV_PI - intersectionArea;

        ovrl = intersectionArea / unionArea;
    }

    return ovrl;
}

static inline void linearizeHomographyAt( const Mat_<double>& H, const Point2f& pt, Mat_<double>& A )
{
    A.create(2,2);
    double p1 = H(0,0)*pt.x + H(0,1)*pt.y + H(0,2),
           p2 = H(1,0)*pt.x + H(1,1)*pt.y + H(1,2),
           p3 = H(2,0)*pt.x + H(2,1)*pt.y + H(2,2),
           p3_2 = p3*p3;
    if( p3 )
    {
        A(0,0) = H(0,0)/p3 - p1*H(2,0)/p3_2; // fxdx
        A(0,1) = H(0,1)/p3 - p1*H(2,1)/p3_2; // fxdy

        A(1,0) = H(1,0)/p3 - p2*H(2,0)/p3_2; // fydx
        A(1,1) = H(1,1)/p3 - p2*H(2,1)/p3_2; // fydx
    }
    else
        A.setTo(Scalar::all(std::numeric_limits<double>::max()));
}

void KeyPoint::project( const vector<KeyPoint>& src, vector<KeyPoint>& dst, const Mat& _H )
{
    if(  !src.empty() )
    {
        assert( !_H.empty() && _H.cols == 3 && _H.rows == 3 && _H.type() == CV_64FC1 );
        Mat_<double> H( 3, 3, (double*)_H.data );

        vector<Point2f> points; KeyPoint::convert( src, points );
        Mat prjPoints; perspectiveTransform( Mat(points), prjPoints, H );

        dst.resize(src.size());

        for( size_t i = 0; i < src.size(); i++ )
        {
            Point2f dstPt = prjPoints.at<Point2f>(i,0);

            // computes size of projected keypoint
            float srcSize2 = src[i].size * src[i].size;
            Mat_<double> M(2, 2);
            M(0,0) = M(1,1) = 1./srcSize2;
            M(1,0) = M(0,1) = 0;
            Mat_<double> invM; invert(M, invM);
            Mat_<double> Aff; linearizeHomographyAt(H, src[i].pt, Aff);
            Mat_<double> dstM; invert(Aff*invM*Aff.t(), dstM);
            Mat_<double> eval; eigen( dstM, eval );
            assert( eval(0,0) && eval(1,0) );
            float dstSize = pow(1./(eval(0,0)*eval(1,0)), 0.25);

            // computes angle of projected keypoint
            // TODO: check angle projection
            float srcAngleRad = src[i].angle*CV_PI/180;
            Point2f vec1(cos(srcAngleRad), sin(srcAngleRad)), vec2;
            vec2.x = Aff(0,0)*vec1.x + Aff(0,1)*vec1.y;
            vec2.y = Aff(1,0)*vec1.x + Aff(0,1)*vec1.y;
            float dstAngleGrad = fastAtan2(vec2.y, vec2.x);

            dst[i] = KeyPoint( dstPt, dstSize, dstAngleGrad, src[i].response, src[i].octave, src[i].class_id );
        }
    }
}

void KeyPoint::filterByRect( vector<KeyPoint>& keypoints, const Rect& rect, vector<int>* origIndices )
{
    if( !keypoints.empty() )
    {
        vector<KeyPoint> tmp;
        tmp.reserve( keypoints.size() );
        if( origIndices )
        {
            origIndices->clear();
            origIndices->reserve( keypoints.size() );
        }

        for( size_t i = 0; i < keypoints.size(); i++ )
        {
            if( rect.contains(keypoints[i].pt) )
            {
                tmp.push_back(keypoints[i]);
                if( origIndices )
                origIndices->push_back(i);
            }
        }

        tmp.swap( keypoints );
    }
}

}
