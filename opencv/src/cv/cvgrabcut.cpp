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

#include "_cv.h"
#include "cvgcgraph.hpp"

using namespace cv;

class GMM
{
public:
    static const uchar K = 5;

    GMM( Mat& model );
    float operator()( Vec3b color ) const;
    float operator()( uchar ci, Vec3b color ) const;
    uchar whatComponent( Vec3b color ) const;

    void startLearning();
    void addSample( uchar ci, Vec3b color );
    void endLearning();
private:
    void calcAuxData( uchar ci );
    float* coefs;
    float* mean;
    float* cov;

    // auxData
    float inverseCov[K][3][3];
    float covDeterm[K];

    // for learning
    float sum[K][3];
    float prod[K][3][3];
    int count[K];
    int totalCount;
};

GMM::GMM( Mat& model )
{
    if( model.empty() )
    {
        model.create( 1, 13*K, CV_32FC1 );
        model.setTo(Scalar(0));
    }
    else if( (model.type() != CV_32FC1) || (model.rows != 1) || (model.cols != 13*K) )
        CV_Error( CV_StsBadArg, "model must have CV_32FC1 type, rows == 1 and cols == 13*K" );

    coefs = model.ptr<float>(0);
    mean = coefs + K;
    cov = mean + 3*K;

    for( uchar ci = 0; ci < K; ci++ )
        if( coefs[ci] > 0 )
             calcAuxData( ci );
}

float GMM::operator()( Vec3b color ) const
{
    float res = 0;
    for( uchar ki = 0; ki < GMM::K; ki++ )
        res += coefs[ki] * this->operator()(ki, color );
    return res;
}

float GMM::operator()( uchar ci, Vec3b color ) const
{
    float res = 0;
    if( coefs[ci] > 0 )
    {
        if( covDeterm[ci] > FLT_EPSILON )
        {
            Vec3f diff = color;
            float* m = mean + 3*ci;
            diff[0] -= m[0]; diff[1] -= m[1]; diff[2] -= m[2];
            float mult = diff[0]*(diff[0]*inverseCov[ci][0][0] + diff[1]*inverseCov[ci][1][0] + diff[2]*inverseCov[ci][2][0])
                + diff[1]*(diff[0]*inverseCov[ci][0][1] + diff[1]*inverseCov[ci][1][1] + diff[2]*inverseCov[ci][2][1])
                + diff[2]*(diff[0]*inverseCov[ci][0][2] + diff[1]*inverseCov[ci][1][2] + diff[2]*inverseCov[ci][2][2]);
            res = 1.0f/sqrt(covDeterm[ci]) * exp(-0.5f*mult);
        }
    }
    return res;
}

uchar GMM::whatComponent( Vec3b color ) const
{
    uchar k = 0;
    float max = 0;

    for( uchar i = 0; i < K; i++ )
    {
        float p = this->operator()( i, color );
        if( p > max )
        {
            k = i;
            max = p;
        }
    }
    return k;
}

void GMM::startLearning()
{
    for( uchar ci = 0; ci < K; ci++)
    {
        sum[ci][0] = sum[ci][1] = sum[ci][2] = 0;
        prod[ci][0][0] = prod[ci][0][1] = prod[ci][0][2] = 0;
        prod[ci][1][0] = prod[ci][1][1] = prod[ci][1][2] = 0;
        prod[ci][2][0] = prod[ci][2][1] = prod[ci][2][2] = 0;
        count[ci] = 0;
    }
    totalCount = 0;
}

void GMM::addSample( uchar ci, Vec3b color )
{
    sum[ci][0] += color[0], sum[ci][1] += color[1], sum[ci][2] += color[2];
    prod[ci][0][0] += color[0]*color[0], prod[ci][0][1] += color[0]*color[1], prod[ci][0][2] += color[0]*color[2];
    prod[ci][1][0] += color[1]*color[0], prod[ci][1][1] += color[1]*color[1], prod[ci][1][2] += color[1]*color[2];
    prod[ci][2][0] += color[2]*color[0], prod[ci][2][1] += color[2]*color[1], prod[ci][2][2] += color[2]*color[2];
    count[ci]++;
    totalCount++;
}

void GMM::endLearning()
{
    for( uchar ci = 0; ci < K; ci++ )
    {
        if( count[ci] == 0 )
        {
            coefs[ci] = 0;
        }
        else
        {
            int n = count[ci];
            coefs[ci] = (float)count[ci]/totalCount;
            float* m = mean + 3*ci;
            m[0] = sum[ci][0]/n; m[1] = sum[ci][1]/n; m[2] = sum[ci][2]/n;
            
            float* c = cov + 9*ci;
            c[0] = prod[ci][0][0]/n - m[0]*m[0], c[1] = prod[ci][0][1]/n - m[0]*m[1], c[2] = prod[ci][0][2]/n - m[0]*m[2];
            c[3] = prod[ci][1][0]/n - m[1]*m[0], c[4] = prod[ci][1][1]/n - m[1]*m[1], c[5] = prod[ci][1][2]/n - m[1]*m[2];
            c[6] = prod[ci][2][0]/n - m[2]*m[0], c[7] = prod[ci][2][1]/n - m[2]*m[1], c[8] = prod[ci][2][2]/n - m[2]*m[2];

            calcAuxData( ci );
        }
    }
}

void GMM::calcAuxData( uchar ci )
{
    if( coefs[ci] > 0 )
    {
        float *c = cov + 9*ci;
        float dtrm = covDeterm[ci] = c[0]*(c[4]*c[8] - c[5]*c[7])
            - c[1]*(c[3]*c[8] - c[5]*c[6]) + c[2]*(c[3]*c[7] - c[4]*c[6]);

        if( dtrm > FLT_EPSILON )
        {
            inverseCov[ci][0][0] =  (c[4]*c[8] - c[5]*c[7]) / dtrm;
            inverseCov[ci][1][0] = -(c[3]*c[8] - c[5]*c[6]) / dtrm;
            inverseCov[ci][2][0] =  (c[3]*c[7] - c[4]*c[6]) / dtrm;
            inverseCov[ci][0][1] = -(c[1]*c[8] - c[2]*c[7]) / dtrm;
            inverseCov[ci][1][1] =  (c[0]*c[8] - c[2]*c[6]) / dtrm;
            inverseCov[ci][2][1] = -(c[0]*c[7] - c[1]*c[6]) / dtrm;
            inverseCov[ci][0][2] =  (c[1]*c[5] - c[2]*c[4]) / dtrm;
            inverseCov[ci][1][2] = -(c[0]*c[5] - c[2]*c[3]) / dtrm;
            inverseCov[ci][2][2] =  (c[0]*c[4] - c[1]*c[3]) / dtrm;
        }
    }
}

float calcBeta( const Mat& img )
{
    float beta = 0;
    Point p;
    for( p.y = 0; p.y < img.rows; p.y++ )
    {
        for( p.x = 0; p.x < img.cols; p.x++ )
        {
            Vec3f color = img.at<Vec3b>(p);
            if( p.x>0 ) // left
            {
                Vec3f diff = color - (Vec3f)img.at<Vec3b>(p.y, p.x-1);
                beta += diff.dot(diff);
            }
            if( p.y>0 && p.x>0 ) // upleft
            {
                Vec3f diff = color - (Vec3f)img.at<Vec3b>(p.y-1, p.x-1);
                beta += diff.dot(diff);
            }
            if( p.y>0 ) // up
            {
                Vec3f diff = color - (Vec3f)img.at<Vec3b>(p.y-1, p.x);
                beta += diff.dot(diff);
            }
            if( p.y>0 && p.x<img.cols-1) // upright
            {
                Vec3f diff = color - (Vec3f)img.at<Vec3b>(p.y-1, p.x+1);
                beta += diff.dot(diff);
            }
        }
    }
    beta = 0.5f*(4*img.cols*img.rows - 3*img.cols - 3*img.rows + 2)/beta;
    return beta;
}

void calcNWeights( const Mat& img, Mat& left, Mat& upleft, Mat& up, Mat& upright, float beta, float gamma )
{
    const float sqrt2 = sqrt(2.0f);
    left.create( img.rows, img.cols, CV_32FC1 );
    upleft.create( img.rows, img.cols, CV_32FC1 );
    up.create( img.rows, img.cols, CV_32FC1 );
    upright.create( img.rows, img.cols, CV_32FC1 );
    Point p, p2;
    Vec3f c, diff;
    for( p.y = 0; p.y < img.rows; p.y++ )
    {
        for( p.x = 0; p.x < img.cols; p.x++ )
        {
            c = img.at<Vec3b>(p);

            p2.y = p.y; // left            
            p2.x = p.x-1;
            if( p2.x>=0 )
            {
                diff = c - (Vec3f)img.at<Vec3b>(p2);
                left.at<float>(p) = gamma * exp(-beta*diff.dot(diff));
            }
            else
                left.at<float>(p) = 0;

            p2.y = p.y-1; // upleft
            p2.x = p.x-1;
            if( p2.x>=0 && p2.y>=0 )
            {
                diff = c - (Vec3f)img.at<Vec3b>(p2);
                upleft.at<float>(p) = gamma * exp(-beta*diff.dot(diff)) / sqrt2;
            }
            else
                upleft.at<float>(p) = 0;

            p2.y = p.y-1; // up
            p2.x = p.x;
            if( p2.y>=0 )
            {
                diff = c - (Vec3f)img.at<Vec3b>(p2);
                up.at<float>(p) = gamma * exp(-beta*diff.dot(diff));
            }
            else
                up.at<float>(p) = 0;

            p2.y = p.y-1; // upright
            p2.x = p.x+1;
            if( p2.x<img.cols-1 && p2.y>=0 )
            {
                diff = c - (Vec3f)img.at<Vec3b>(p2);
                upright.at<float>(p) = gamma * exp(-beta*diff.dot(diff)) / sqrt2;
            }
            else
                upright.at<float>(p) = 0;
        }
    }
}

void checkMask( const Mat& img, const Mat& mask )
{
    if( mask.empty() )
        CV_Error( CV_StsBadArg, "mask is empty" );
    if( mask.type() != CV_8UC1 )
        CV_Error( CV_StsBadArg, "mask mush have CV_8UC1 type" );
    if( mask.cols != img.cols || mask.rows != img.rows )
        CV_Error( CV_StsBadArg, "mask mush have rows and cols as img" );
    Point p;
    for( p.y = 0; p.y < mask.rows; p.y++ )
    {
        for( p.x = 0; p.x < mask.cols; p.x++ )
        {
            uchar val = mask.at<uchar>(p);
            if( val!=GC_BGD && val!=GC_FGD && val!=GC_PR_BGD && val!=GC_PR_FGD )
                CV_Error( CV_StsBadArg, "mask element value must be equel"
                    "GC_BGD or GC_FGD or GC_PR_BGD or GC_PR_FGD" );
        }
    }
}

void cv::grabCut( const Mat& img, Mat& mask, Rect rect, 
             Mat& bgdModel, Mat& fgdModel,
             int iterCount, int flag )
{
    if( img.empty() )
        CV_Error( CV_StsBadArg, "image is empty" );
    if( img.type() != CV_8UC3 )
        CV_Error( CV_StsBadArg, "image mush have CV_8UC3 type" );

    const int KMI = 10;
    const int KMT = KMEANS_PP_CENTERS;

    GMM bgdGMM( bgdModel ), fgdGMM( fgdModel );
    Mat cidx( img.size(), CV_8UC1 );

    Point p;
    if( flag == GC_INIT_WITH_RECT || flag == GC_INIT_WITH_MASK )
    {
        if( flag == GC_INIT_WITH_RECT )
        {
            mask.create( img.size(), CV_8UC1 );
            mask.setTo( GC_BGD );

            rect.x = max(0, rect.x);
            rect.y = max(0, rect.y);
            rect.width = min(rect.width, img.cols-rect.x);
            rect.height = min(rect.height, img.rows-rect.y);

            Mat maskRect = mask( Range(rect.y, rect.y + rect.height), Range(rect.x, rect.x + rect.width) );
            maskRect.setTo( Scalar(GC_PR_FGD) );
        }
        else // flag == GC_INIT_WITH_MASK 
            checkMask( img, mask );

        // init GMMs
        Mat bgdLabels, fgdLabels;
        vector<Vec3f> bgdSamples, fgdSamples;
        for( p.y = 0; p.y < img.rows; p.y++ )
        {
            for( p.x = 0; p.x < img.cols; p.x++ )
            {
                if( mask.at<uchar>(p) == GC_BGD )
                    bgdSamples.push_back( (Vec3f)img.at<Vec3b>(p) );
                else // GC_PR_BGD | GC_FGD | GC_PR_FGD
                    fgdSamples.push_back( (Vec3f)img.at<Vec3b>(p) );
            }
        }
        CV_Assert( !bgdSamples.empty() && !fgdSamples.empty() );
        Mat _bgdSamples( (int)bgdSamples.size(), 3, CV_32FC1, &bgdSamples[0][0] );
        kmeans( _bgdSamples, GMM::K, bgdLabels, TermCriteria( CV_TERMCRIT_ITER, KMI, 0.0), 0, KMT, 0 );
        Mat _fgdSamples( (int)fgdSamples.size(), 3, CV_32FC1, &fgdSamples[0][0] );
        kmeans( _fgdSamples, GMM::K, fgdLabels, TermCriteria( CV_TERMCRIT_ITER, KMI, 0.0), 0, KMT, 0 );

        bgdGMM.startLearning();
        for( int i = 0; i < (int)bgdSamples.size(); i++ )
            bgdGMM.addSample( (uchar)bgdLabels.at<int>(i,0), bgdSamples[i] );
        bgdGMM.endLearning();

        fgdGMM.startLearning();
        for( int i = 0; i < (int)fgdSamples.size(); i++ )
            fgdGMM.addSample( (uchar)fgdLabels.at<int>(i,0), fgdSamples[i] );
        fgdGMM.endLearning();
    }

    if( iterCount <= 0)
        return;

    if( flag != GC_INIT_WITH_MASK )
        checkMask( img, mask );

    const float gamma = 50;
    const float lambda = 9*gamma;
    const float beta = calcBeta( img );
    Mat left, upleft, up, upright;
    calcNWeights( img, left, upleft, up, upright, beta, gamma );

    for( int i = 0; i < iterCount; i++ )
    {
        // assign GMMs components
        for( p.y = 0; p.y < img.rows; p.y++ )
        {
            for( p.x = 0; p.x < img.cols; p.x++ )
            {
                Vec3b color = img.at<Vec3b>(p);
                cidx.at<uchar>(p) = mask.at<uchar>(p) == GC_BGD || mask.at<uchar>(p) == GC_PR_BGD ?
                    bgdGMM.whatComponent(color) : fgdGMM.whatComponent(color);
            }
        }

        // learn GMMs parameters
        bgdGMM.startLearning();
        fgdGMM.startLearning();
        for( uchar ci = 0; ci < GMM::K; ci++ )
        {
            for( p.y = 0; p.y < img.rows; p.y++ )
            {
                for( p.x = 0; p.x < img.cols; p.x++ )
                {
                    if( cidx.at<uchar>(p) == ci )
                    {
                        if( mask.at<uchar>(p) == GC_BGD || mask.at<uchar>(p) == GC_PR_BGD )    
                            bgdGMM.addSample( ci, img.at<Vec3b>(p) );
                        else
                            fgdGMM.addSample( ci, img.at<Vec3b>(p) );
                    }
                }
            }
        }
        bgdGMM.endLearning();
        fgdGMM.endLearning();

        // estimate segmentation
        GCGraph<float> graph( img.cols*img.rows, 8*img.cols*img.rows - 6*(img.cols + img.rows) + 4 );
        for( p.y = 0; p.y < img.rows; p.y++ )
        {
            for( p.x = 0; p.x < img.cols; p.x++)
            {
                // add node
                graph.addVtx();
                int idx = p.y*img.cols+p.x;
                Vec3b color = img.at<Vec3b>(p);

                // set t-weights
                float fromSource, toSink;
                if( mask.at<uchar>(p) == GC_PR_BGD || mask.at<uchar>(p) == GC_PR_FGD )
                {
                    fromSource = -log( bgdGMM(color) );
                    toSink = -log( fgdGMM(color) );
                }
                else if( mask.at<uchar>(p) == GC_BGD )
                {
                    fromSource = 0;
                    toSink = lambda;
                }
                else // GC_BGD
                {
                    fromSource = lambda;
                    toSink = 0;
                }
                graph.addTermWeights( idx, fromSource, toSink );

                // set n-weights
                if( p.x>0 )
                {
                    float w = left.at<float>(p);
                    graph.addEdges( idx, idx-1, w, w );
                }
                if( p.x>0 && p.y>0 )
                {
                    float w = upleft.at<float>(p);
                    graph.addEdges( idx, idx-img.cols-1, w, w );
                }
                if( p.y>0 )
                {
                    float w = up.at<float>(p);
                    graph.addEdges( idx, idx-img.cols, w, w );
                }
                if( p.x<img.cols-1 && p.y>0 )
                {
                    float w = upright.at<float>(p);
                    graph.addEdges( idx, idx-img.cols+1, w, w );
                }
            }
        }

        graph.maxFlow();

        for( p.y = 0; p.y < img.rows; p.y++ )
        {
            for( p.x = 0; p.x < img.cols; p.x++ )
            {
                if( mask.at<uchar>(p) == GC_PR_BGD || mask.at<uchar>(p) == GC_PR_FGD )
                {
                    if( graph.inSourceSegment( p.y*img.cols+p.x) )
                        mask.at<uchar>(p) = GC_PR_FGD;
                    else
                        mask.at<uchar>(p) = GC_PR_BGD;
                }
            }
        }
    }
}