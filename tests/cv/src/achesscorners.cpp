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

void show_points( IplImage* gray, CvPoint2D32f* u, int u_cnt, CvPoint2D32f* v, int v_cnt,
                  CvSize etalon_size, int was_found )
{
    CvSize size;
    int i;

    cvGetImageRawData( gray, 0, 0, &size );
    
    IplImage* rgb = cvCreateImage( size, 8, 3 );
    cvMerge( gray, gray, gray, 0, rgb );

    if( v )
    {
        for( i = 0; i < v_cnt; i++ )
        {
            cvCircle( rgb, cvPoint(cvRound(v[i].x), cvRound(v[i].y)), 3, CV_RGB(255,0,0), CV_FILLED);
        }
    }

    if( u )
    {
        for( i = 0; i < u_cnt; i++ )
        {
            cvCircle( rgb, cvPoint(cvRound(u[i].x), cvRound(u[i].y)), 3, CV_RGB(0,255,0), CV_FILLED);
        }
    }

    cvDrawChessboardCorners( rgb, etalon_size, v, v_cnt, was_found );

    cvvNamedWindow( "test", 0 );
    cvvShowImage( "test", rgb );

    cvvWaitKey(0);
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

/* ///////////////////// chess_corner_test ///////////////////////// */
void CV_ChessboardDetectorTest::run( int start_from )
{
    int code = CvTS::OK;

#ifndef WRITE_POINTS    
    const double rough_success_error_level = 2.5;
    const double precise_success_error_level = 0.2;
    double err = 0, max_rough_error = 0, max_precise_error = 0;
#endif

    /* test parameters */
    char   filepath[1000];
    char   filename[1000];

    CvMat*  _u = 0;
    CvMat*  _v = 0;
    CvPoint2D32f* u;
    CvPoint2D32f* v;

    IplImage* img = 0;
    IplImage* gray = 0;
    IplImage* thresh = 0;

    int  idx, max_idx;
    int  progress = 0;

    sprintf( filepath, "%scameracalibration/", ts->get_data_path() );
    sprintf( filename, "%schessboard_list.dat", filepath );
    CvFileStorage* fs = cvOpenFileStorage( filename, 0, CV_STORAGE_READ );
    CvFileNode* board_list = fs ? cvGetFileNodeByName( fs, 0, "boards" ) : 0;

    if( !fs || !board_list || !CV_NODE_IS_SEQ(board_list->tag) ||
        board_list->data.seq->total % 2 != 0 )
    {
        ts->printf( CvTS::LOG, "chessboard_list.dat can not be readed or is not valid" );
        code = CvTS::FAIL_MISSING_TEST_DATA;
        goto _exit_;
    }

    max_idx = board_list->data.seq->total/2;

    for( idx = start_from; idx < max_idx; idx++ )
    {
        int etalon_count = -1;
        int count = 0;
        CvSize etalon_size = { -1, -1 };
        int j, result;
        
        ts->update_context( this, idx-1, true );

        /* read the image */
        sprintf( filename, "%s%s", filepath,
            cvReadString((CvFileNode*)cvGetSeqElem(board_list->data.seq,idx*2),"dummy.txt"));
    
        img = cvLoadImage( filename );
        
        if( !img )
        {
            ts->printf( CvTS::LOG, "one of chessboard images can't be read: %s", filename );
            if( max_idx == 1 )
            {
                code = CvTS::FAIL_MISSING_TEST_DATA;
                goto _exit_;
            }
            continue;
        }

        gray = cvCreateImage( cvSize( img->width, img->height ), IPL_DEPTH_8U, 1 );
        thresh = cvCreateImage( cvSize( img->width, img->height ), IPL_DEPTH_8U, 1 );
        cvCvtColor( img, gray, CV_BGR2GRAY );
 
        sprintf( filename, "%s%s", filepath,
            cvReadString((CvFileNode*)cvGetSeqElem(board_list->data.seq,idx*2+1),"dummy.txt"));

        _u = (CvMat*)cvLoad( filename );

        if( _u == 0 )
        {
            if( idx == 0 )
                ts->printf( CvTS::LOG, "one of chessboard corner files can't be read: %s", filename ); 
            if( max_idx == 1 )
            {
                code = CvTS::FAIL_MISSING_TEST_DATA;
                goto _exit_;
            }
            continue;
        }

        etalon_size.width = _u->cols;
        etalon_size.height = _u->rows;
        etalon_count = etalon_size.width*etalon_size.height;

        /* allocate additional buffers */
        _v = cvCloneMat( _u );
        count = etalon_count;

        u = (CvPoint2D32f*)_u->data.fl;
        v = (CvPoint2D32f*)_v->data.fl;

        OPENCV_CALL( result = cvFindChessBoardCornerGuesses(
                     gray, thresh, 0, etalon_size, v, &count ));

        //show_points( gray, 0, etalon_count, v, count, etalon_size, result );
        if( !result || count != etalon_count )
        {
            ts->printf( CvTS::LOG, "chess board is not found" );
            code = CvTS::FAIL_INVALID_OUTPUT;
            goto _exit_;
        }

#ifndef WRITE_POINTS
        err = 0;
        for( j = 0; j < etalon_count; j++ )
        {
            double dx = fabs( v[j].x - u[j].x );
            double dy = fabs( v[j].y - u[j].y );

            dx = MAX( dx, dy );
            if( dx > err )
            {
                err = dx;
                if( err > rough_success_error_level )
                {
                    ts->printf( CvTS::LOG, "bad accuracy of corner guesses" );
                    code = CvTS::FAIL_BAD_ACCURACY;
                    goto _exit_;
                }
            }
        }
        max_rough_error = MAX( max_rough_error, err );
#endif
        OPENCV_CALL( cvFindCornerSubPix( gray, v, count, cvSize( 5, 5 ), cvSize(-1,-1),
                            cvTermCriteria(CV_TERMCRIT_EPS|CV_TERMCRIT_ITER,30,0.1)));
        //show_points( gray, u + 1, etalon_count, v, count );

#ifndef WRITE_POINTS
        err = 0;
        for( j = 0; j < etalon_count; j++ )
        {
            double dx = fabs( v[j].x - u[j].x );
            double dy = fabs( v[j].y - u[j].y );

            dx = MAX( dx, dy );
            if( dx > err )
            {
                err = dx;
                if( err > precise_success_error_level )
                {
                    ts->printf( CvTS::LOG, "bad accuracy of adjusted corners" ); 
                    code = CvTS::FAIL_BAD_ACCURACY;
                    goto _exit_;
                }
            }
        }
        max_precise_error = MAX( max_precise_error, err );
#else
        cvSave( filename, _v );
#endif
        cvReleaseMat( &_u );
        cvReleaseMat( &_v );
        cvReleaseImage( &img );
        cvReleaseImage( &gray );
        cvReleaseImage( &thresh );
        progress = update_progress( progress, idx-1, max_idx, 0 );
    }

_exit_:

    /* release occupied memory */
    cvReleaseMat( &_u );
    cvReleaseMat( &_v );
    cvReleaseFileStorage( &fs );
    cvReleaseImage( &img );
    cvReleaseImage( &gray );
    cvReleaseImage( &thresh );

    if( code < 0 )
        ts->set_failed_test_info( code );
}

CV_ChessboardDetectorTest chessboard_detector_test;

/* End of file. */
