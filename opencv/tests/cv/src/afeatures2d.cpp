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
#include "cvtest.h"
#include "opencv2/core/core.hpp"

using namespace std;
using namespace cv;

/*
 * Regression tests for feature detectors comparing keypoints.
 */

class CV_FeatureDetectorTest : public CvTest
{
public:
    CV_FeatureDetectorTest( const char* test_name, const Ptr<FeatureDetector>& _fdetector ) :
        CvTest( test_name, "cv::FeatureDetector::detect"), fdetector(_fdetector) {}

protected:
    virtual void run( int start_from )
    {
        const float ptDif = 1.f;
        const float sizeDif = 1.f;
        const float angleDif = 2.f;
        const float responseDif = 0.1f;

        int code = CvTS::OK;
        string imgFilename = string(ts->get_data_path()) + "/features2d/img.png";
        string resFilename = string(ts->get_data_path()) + "/features2d/" + string(name) + ".xml";

        Mat image = imread( imgFilename );
        if( image.empty() )
        {
            stringstream ss; ss << "image " << imgFilename << "can not be read" << endl;
            ts->printf( CvTS::LOG, ss.str().c_str() );
            ts->set_failed_test_info( CvTS::FAIL_INVALID_TEST_DATA );
            return;
        }

        FileStorage fs( resFilename, FileStorage::READ );

        vector<KeyPoint> calcKeypoints;
        fdetector->detect( image, calcKeypoints );

        if( fs.isOpened() ) // compare computed and valid keypoints
        {
            // TODO compare saved feature detector params with current ones
            vector<KeyPoint> validKeypoints;
            read( fs.getFirstTopLevelNode(), validKeypoints );
            if( validKeypoints.empty() )
            {
                stringstream ss; ss <<"image " << imgFilename << "can not be read" << endl;
                ts->printf( CvTS::LOG, ss.str().c_str() );
                ts->set_failed_test_info( CvTS::FAIL_INVALID_TEST_DATA );
                return;
            }

            int badPointCount = 0, commonPointCount = max(validKeypoints.size(), calcKeypoints.size());
            for( size_t v = 0; v < validKeypoints.size(); v++ )
            {
                for( size_t c = 0; c < calcKeypoints.size(); c++ )
                {
                    if( norm( calcKeypoints[c].pt - validKeypoints[v].pt ) < ptDif )
                    {
                        if( abs(calcKeypoints[c].size - validKeypoints[v].size) > sizeDif ||
                            abs(calcKeypoints[c].angle - validKeypoints[v].angle) > angleDif ||
                            abs(calcKeypoints[c].response - validKeypoints[v].response) > responseDif ||
                            calcKeypoints[c].octave != validKeypoints[v].octave ||
                            calcKeypoints[c].class_id != validKeypoints[v].class_id )
                        {
                            badPointCount++;
                        }
                    }
                    else
                        badPointCount++;
                }
            }
            if( badPointCount > 0.9 * commonPointCount )
            {
                stringstream ss; ss <<"Bad accuracy: badPointCount = " << badPointCount << endl;
                ts->printf( CvTS::LOG, ss.str().c_str() );
                ts->set_failed_test_info( CvTS::FAIL_BAD_ACCURACY );
                return;
            }
        }
        else // write
        {
            fs.open( resFilename, FileStorage::WRITE );
            if( !fs.isOpened() )
            {
                stringstream ss; ss <<"file " << resFilename << "can not be opened to write" << endl;
                ts->printf( CvTS::LOG, ss.str().c_str() );
                ts->set_failed_test_info( CvTS::FAIL_INVALID_TEST_DATA );
                return;
            }
            else
            {
                fdetector->write( fs );
                write( fs, "keypoints", calcKeypoints );
            }
        }
        ts->set_failed_test_info( CvTS::OK );
    }

    Ptr<FeatureDetector> fdetector;
};

CV_FeatureDetectorTest fastTest( "fast", createFeatureDetector("FAST") );
CV_FeatureDetectorTest gfttTest( "gftt", createFeatureDetector("GFTT") );
CV_FeatureDetectorTest harrisTest( "harris", createFeatureDetector("HARRIS") );
CV_FeatureDetectorTest mserTest( "mser", createFeatureDetector("MSER") );
CV_FeatureDetectorTest siftTest( "sift", createFeatureDetector("SIFT") );
CV_FeatureDetectorTest starTest( "star", createFeatureDetector("STAR") );
CV_FeatureDetectorTest surfTest( "surf", createFeatureDetector("SURF") );

/*
 * Regression tests for descriptor extractor
 */
