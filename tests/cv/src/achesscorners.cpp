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
#include "cvchessboardgenerator.h"

#include <limits>

using namespace std;
using namespace cv;

void show_points( const Mat& gray, const Mat& u, const vector<Point2f>& v, Size pattern_size, bool was_found )
{
    Mat rgb( gray.size(), CV_8U);
    merge(vector<Mat>(3, gray), rgb);
        
    for(size_t i = 0; i < v.size(); i++ )
        circle( rgb, v[i], 3, CV_RGB(255, 0, 0), CV_FILLED);            

    if( !u.empty() )
    {
        const Point2f* u_data = u.ptr<Point2f>();
        size_t count = u.cols * u.rows;
        for(size_t i = 0; i < count; i++ )
            circle( rgb, u_data[i], 3, CV_RGB(0, 255, 0), CV_FILLED);
    }
    if (!v.empty())
    {
        Mat corners(v.size(), 1, CV_32FC2, (void*)&v[0]);     
        drawChessboardCorners( rgb, pattern_size, corners, was_found );
    }
    //namedWindow( "test", 0 ); imshow( "test", rgb ); waitKey(0);
}


class CV_ChessboardDetectorTest : public CvTest
{
public:
    CV_ChessboardDetectorTest();
protected:
    void run(int);
};

CV_ChessboardDetectorTest::CV_ChessboardDetectorTest():
    CvTest( "chessboard-detector", "cvFindChessboardCorners" )
{
    support_testing_modes = CvTS::CORRECTNESS_CHECK_MODE;
}

double calcError(const vector<Point2f>& v, const Mat& u)
{
    size_t count_exp = static_cast<size_t>(u.cols * u.rows);
    const Point2f* u_data = u.ptr<Point2f>();

    double err = numeric_limits<double>::max();
    for( size_t k = 0; k < 2; ++k )
    {
        double err1 = 0;
        for(size_t j = 0; j < count_exp; ++j )
        {
            int j1 = k == 0 ? j : count_exp - j - 1;
            double dx = fabs( v[j].x - u_data[j1].x );
            double dy = fabs( v[j].y - u_data[j1].y );

            dx = MAX( dx, dy );
            if( dx > err1 )
                err1 = dx;
        }
        err = min(err, err1);
    }
    return err;
}


/* ///////////////////// chess_corner_test ///////////////////////// */
void CV_ChessboardDetectorTest::run( int /*start_from */)
{
    CvTS& ts = *this->ts;

//#define WRITE_POINTS 1
#ifndef WRITE_POINTS    
    const double rough_success_error_level = 2.5;
    const double precise_success_error_level = 2;
    double max_rough_error = 0, max_precise_error = 0;
#endif
    string folder = string(ts.get_data_path()) + "cameracalibration/";

    FileStorage fs( folder + "chessboard_list.dat", FileStorage::READ );
    FileNode board_list = fs["boards"];
        
    if( !fs.isOpened() || board_list.empty() || !board_list.isSeq() || board_list.size() % 2 != 0 )
    {
        ts.printf( CvTS::LOG, "chessboard_list.dat can not be readed or is not valid" );
        ts.set_failed_test_info( CvTS::FAIL_MISSING_TEST_DATA );
        return;
    }

    int progress = 0;
    int max_idx = board_list.node->data.seq->total/2;

    for(int idx = 9; idx < max_idx; ++idx )
    {
        ts.update_context( this, idx, true );
        
        /* read the image */
        string img_file = board_list[idx * 2];                    
        Mat gray = imread( folder + img_file, 0);
                
        if( gray.empty() )
        {
            ts.printf( CvTS::LOG, "one of chessboard images can't be read: %s", img_file.c_str() );
            ts.set_failed_test_info( CvTS::FAIL_MISSING_TEST_DATA );
            return;
        }

        string filename = folder + (string)board_list[idx * 2 + 1];
        Mat expected;
        {
            CvMat *u = (CvMat*)cvLoad( filename.c_str() );
            if(!u )
            {                
                ts.printf( CvTS::LOG, "one of chessboard corner files can't be read: %s", filename.c_str() ); 
                ts.set_failed_test_info( CvTS::FAIL_MISSING_TEST_DATA );
                return;                
            }
            expected = Mat(u, true);
            cvReleaseMat( &u );
        }                
        size_t count_exp = static_cast<size_t>(expected.cols * expected.rows);                
        Size pattern_size = expected.size();

        vector<Point2f> v;        
        bool result = findChessboardCorners(gray, pattern_size, v, CV_CALIB_CB_ADAPTIVE_THRESH | CV_CALIB_CB_NORMALIZE_IMAGE);        
        show_points( gray, Mat(), v, pattern_size, result );
        if( !result || v.size() != count_exp )
        {
            ts.printf( CvTS::LOG, "chess board is not found\n" );
            ts.set_failed_test_info( CvTS::FAIL_INVALID_OUTPUT );
            return;
        }

#ifndef WRITE_POINTS
        double err = calcError(v, expected);
        if( err > rough_success_error_level )
        {
            ts.printf( CvTS::LOG, "bad accuracy of corner guesses" );
            ts.set_failed_test_info( CvTS::FAIL_BAD_ACCURACY );
            return;
        }
        max_rough_error = MAX( max_rough_error, err );
#endif
        cornerSubPix( gray, v, Size(5, 5), Size(-1,-1), TermCriteria(TermCriteria::EPS|TermCriteria::MAX_ITER, 30, 0.1));        
        show_points( gray, expected, v, pattern_size, result  );

#ifndef WRITE_POINTS
        err = calcError(v, expected);
        if( err > precise_success_error_level )
        {
            ts.printf( CvTS::LOG, "bad accuracy of adjusted corners" ); 
            ts.set_failed_test_info( CvTS::FAIL_BAD_ACCURACY );
            return;
        }
        max_precise_error = MAX( max_precise_error, err );
#else
        Mat mat_v(pattern_size, CV_32FC2, (void*)&v[0]);
        CvMat cvmat_v = mat_v;
        cvSave( filename.c_str(), &cvmat_v );
#endif
        progress = update_progress( progress, idx, max_idx, 0 );
    }

    ts.set_failed_test_info( CvTS::OK);
}

CV_ChessboardDetectorTest chessboard_detector_test;

/* End of file. */