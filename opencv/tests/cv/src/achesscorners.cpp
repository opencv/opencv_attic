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

#include <stdlib.h>
#include <assert.h>
#include <limits.h>
#include <float.h>

//#include <windows.h>
#include "highgui.h"

//#define WRITE_POINTS

static char* funcs[] =
{
    "cvFindChessBoardCornerGuesses, cvFindCornerSubPix"
};

static char *test_desc[] =
{
    "Regression test"
};

void show_points( IplImage* gray, CvPoint2D32f* u, int u_cnt, CvPoint2D32f* v, int v_cnt )
{
    CvSize size;
    int i;

    cvGetImageRawData( gray, 0, 0, &size );
    
    IplImage* rgb = cvCreateImage( size, 8, 3 );
    cvCvtPlaneToPix( gray, gray, gray, 0, rgb );

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

    named_window( "test", 0 );
    show_iplimage( "test", rgb );

    wait_key(0);
}


/* ///////////////////// chess_corner_test ///////////////////////// */
static int chess_corner_test( void )
{
#ifndef WRITE_POINTS    
    const double rough_success_error_level = 2.5;
    const double precise_success_error_level = 0.2;
    int i = 0, n = 0, j = 0;
    double err = 0, max_rough_error = 0, max_precise_error = 0;
#endif

    const char* all_is_ok = "No errors";
    const char* error_string = all_is_ok;

    /* test parameters */
    char   filepath[100];
    char   filename[100];

    CvPoint2D32f*  u = 0;
    CvPoint2D32f*  v = 0;

    IplImage* img = 0;
    IplImage* gray = 0;
    IplImage* thresh = 0;

    int  idx, max_idx;

    atsGetTestDataPath( filepath, "cameracalibration", 0, 0 );
    sprintf( filename, "%schess_size.dat", filepath );
    
    CvSize2D32f* sz = (CvSize2D32f*)atsReadMatrix( filename, &max_idx, &idx );

    if( idx != 2 )
    {
        error_string = "chess_size.dat can't be readed"; 
        goto test_exit;
    }

    for( idx = 1; idx <= max_idx; idx++ )
    {
        int etalon_count = -1;
        int count = 0;
        CvSize etalon_size = { -1, -1 };
        int result;
        
        /* read the image */
        sprintf( filename, "%schess%d.jpg", filepath, idx );
	
        img = load_iplimage( filename );
        
        if( !img )
        {
            error_string = "one of chess*.jpg images can't be readed"; 
            goto test_exit;
        }

        gray = cvCreateImage( cvSize( img->width, img->height ), IPL_DEPTH_8U, 1 );
        thresh = cvCreateImage( cvSize( img->width, img->height ), IPL_DEPTH_8U, 1 );
        cvCvtColor( img, gray, CV_BGR2GRAY );
 
        sprintf( filename, "%schess_corners%d.dat", filepath, idx );

#ifndef WRITE_POINTS
        u = (CvPoint2D32f*)atsReadMatrix( filename, &n, &i );

        if( u == 0 )
        {
            if( idx == 0 )
                error_string = "chess_corners*.dat files can't be read"; 
            goto test_exit;
        }

        if( n > 0 )
        {
            etalon_size.width = cvRound( u[0].x );
            etalon_size.height = cvRound( u[0].y );

            if( etalon_size.width != cvRound(sz[idx-1].width) ||
                etalon_size.height != cvRound(sz[idx-1].height))
            {
                error_string = "mismatched board sizes";
                goto test_exit;
            }

            etalon_count = etalon_size.width*etalon_size.height;
        }

        if( i != 2 || n != etalon_count + 1 ||
            etalon_size.width <= 0 || etalon_size.height <= 0 )
        {
            error_string = "one of chess_corners*.dat files is corrupted"; 
            goto test_exit;
        }
#else
        etalon_size.width = cvRound( sz[idx-1].width );
        etalon_size.height = cvRound( sz[idx-1].height );
        etalon_count = etalon_size.width*etalon_size.height;

        u = (CvPoint2D32f*)malloc( (etalon_count + 1)*sizeof(u[0]));
#endif

        /* allocate additional buffers */
        v = (CvPoint2D32f*)malloc( (etalon_count + 1)*sizeof(v[0]));

        count = etalon_size.width*etalon_size.height;

        OPENCV_CALL( result = cvFindChessBoardCornerGuesses(
                     gray, thresh, 0, etalon_size, v + 1, &count ));

        //show_points( gray, 0, etalon_count, v + 1, count );

        if( !result || count != etalon_count )
        {
            error_string = "chess board is not found"; 
            goto test_exit;
        }

#ifndef WRITE_POINTS
        err = 0;
        for( j = 0; j < etalon_count; j++ )
        {
            double dx = fabs( v[j+1].x - u[j+1].x );
            double dy = fabs( v[j+1].y - u[j+1].y );

            dx = MAX( dx, dy );
            if( dx > err )
            {
                err = dx;
                if( err > rough_success_error_level )
                {
                    error_string = "bad accuracy of corner guesses"; 
                    goto test_exit;
                }
            }
        }
        max_rough_error = MAX( max_rough_error, err );
#endif

        OPENCV_CALL( cvFindCornerSubPix( gray, v + 1, count, cvSize( 5, 5 ), cvSize(-1,-1),
                            cvTermCriteria(CV_TERMCRIT_EPS|CV_TERMCRIT_ITER,30,0.1)));

        //show_points( gray, u + 1, etalon_count, v + 1, count );

#ifndef WRITE_POINTS
        err = 0;
        for( j = 0; j < etalon_count; j++ )
        {
            double dx = fabs( v[j + 1].x - u[j + 1].x );
            double dy = fabs( v[j + 1].y - u[j + 1].y );

            dx = MAX( dx, dy );
            if( dx > err )
            {
                err = dx;
                if( err > precise_success_error_level )
                {
                    error_string = "bad accuracy of adjusted corners"; 

                    goto test_exit;
                }
            }
        }
        max_precise_error = MAX( max_precise_error, err );
#else
        v[0].x = (float)etalon_size.width;
        v[0].y = (float)etalon_size.height;
        atsWriteMatrix( filename, etalon_count + 1, 2, (float*)v );
#endif

        free( u );
        free( v );
        u = v = 0;
        cvReleaseImage( &img );
        cvReleaseImage( &gray );
        cvReleaseImage( &thresh );
    }

test_exit:

    /* release occupied memory */
    if( u )
        free( u );
    if( v )
        free( v );

    if( sz )
        free( sz );

    if( img )
    {
        cvReleaseImage( &img );
    }
    cvReleaseImage( &gray );
    cvReleaseImage( &thresh );

#ifndef WRITE_POINTS    
    if( error_string == all_is_ok )
    {
        trsWrite( ATS_LST, "Max rough error is %g, max precise error is %g",
                  max_rough_error, max_precise_error );
        return trsResult( TRS_OK, "No errors" );
    }
    else
    {
        trsWrite( ATS_LST, "Fatal error" );
        return trsResult( TRS_FAIL, error_string );
    }
#else
    return TRS_OK;
#endif
}


void  InitAChessCorners(void)
{
    /* Registering test functions */
    trsReg( funcs[0], test_desc[0], atsAlgoClass, chess_corner_test );

} /* InitAChessCorners */

/* End of file. */
