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

static char* funcs[] =
{
    "cvCalcOpticalFlowPyrLK",
};

static char *test_desc = "Regression Test for Lucas Kanade pyramid-based optical flow";


/* ///////////////////// pyrlk_test ///////////////////////// */

static int pyrlk_test( void )
{
    const double success_error_level = 0.2;

    const char* all_is_ok = "No errors";
    const char* error_string = all_is_ok;

    /* test parameters */
    double  max_err = 0., sum_err = 0;
    int     pt_cmpd = 0;
    int     pt_exceed = 0;
    int     merr_i = 0, merr_j = 0, merr_k = 0;
    char    filepath[100];
    char    filename[100];
    const   char* i_pts_file = "lk_prev.dat";
    const   char* j_pts_file = "lk_next.dat";
    const   char* i_img_file = "rock_1.bmp";
    const   char* j_img_file = "rock_2.bmp";

    CvPoint2D32f*  u = 0;
    CvPoint2D32f*  v = 0;
    CvPoint2D32f*  v2 = 0;
    char* status = 0;

    IplImage* imgI = 0;
    IplImage* imgJ = 0;

    int     n = 0, i = 0, j = 0;

    atsGetTestDataPath( filepath, "optflow", 0, 0 );

    /* read feature points from the first image */
    strcpy( filename, filepath );
    strcat( filename, i_pts_file );

    u = (CvPoint2D32f*)atsReadMatrix( filename, &n, &i );
    
    if( !u )
    {
        error_string = "couldn't read lk_prev.dat file"; 
        goto test_exit;
    }

    if( i != 2 || n <= 0 )
    {
        error_string = "lk_prev.dat file has been corrupted"; 
        goto test_exit;
    }

    /* read feature points from the second image (calculated by MATLAB script) */
    strcpy( filename, filepath );
    strcat( filename, j_pts_file );

    v = (CvPoint2D32f*)atsReadMatrix( filename, &j, &i );
    if( !v )
    {
        error_string = "couldn't read lk_next.dat file"; 
        goto test_exit;
    }

    if( i != 2 || j != n )
    {
        error_string = "lk_next.dat file has been corrupted"; 
        goto test_exit;
    }

    /* allocate adidtional buffers */
    v2 = (CvPoint2D32f*)icvAlloc( n*sizeof(v2[0]));
    status = (char*)icvAlloc(n*sizeof(status[0]));

    /* read first image */
    strcpy( filename, filepath );
    strcat( filename, i_img_file );
    
    imgI = atsCreateImageFromFile( filename );

    if( !imgI )
    {
        error_string = "first image can't be readed"; 
        goto test_exit;
    }

    /* read second image */
    strcpy( filename, filepath );
    strcat( filename, j_img_file );

    imgJ = atsCreateImageFromFile( filename );

    if( !imgJ )
    {
        error_string = "second image can't be readed"; 
        goto test_exit;
    }
    
    /* calculate flow */
    cvCalcOpticalFlowPyrLK( imgI, imgJ, 0, 0, u, v2, n, cvSize( 20, 20 ),
                            4, status, 0, cvTermCriteria( CV_TERMCRIT_ITER|
                            CV_TERMCRIT_EPS, 30, 0.01f ), 0 );

    /* compare results */
    for( i = 0; i < n; i++ )
    {
        if( status[i] != 0 )
        {
            double err;
            if( atsIsNaN( v[i].x ))
            {
                merr_j++;
                continue;
            }

            err = fabs(v2[i].x - v[i].x) + fabs(v2[i].y - v[i].y);
            if( err > max_err )
            {
                max_err = err;
                merr_i = i;
            }

            pt_exceed += err > success_error_level;

            sum_err += err;
            pt_cmpd++;
        }
        else
        {
            if( !atsIsNaN( v[i].x ))
            {
                merr_i = i;
                merr_k++;
            }
        }
    }

test_exit:

    /* release occupied memory */
    icvFree( &status );
    icvFree( &v2 );
    free( u );
    free( v );
    if( imgI ) atsReleaseImage( imgI );
    if( imgJ ) atsReleaseImage( imgJ );

    if( error_string == all_is_ok )
    {
        trsWrite( ATS_LST, "Avg.err is %g, max. err is %g at i = %d, poor pts = %d,"
                           "superflous = %d, deficient = %d",
                           sum_err/MAX(pt_cmpd,1),  max_err, merr_i, pt_exceed,
                           merr_j, merr_k );

        return max_err < 1 && pt_exceed < 3 && merr_k == 0 ?
            trsResult( TRS_OK, "No errors" ) :
            trsResult( TRS_FAIL, "Bad accuracy" );
    }
    else
    {
        trsWrite( ATS_LST, "Fatal error" );
        return trsResult( TRS_FAIL, error_string );
    }
}


void  InitAOptFlowPyrLK(void)
{
    /* Registering test functions */
    trsReg( funcs[0], test_desc, atsAlgoClass, pyrlk_test );

} /* InitAOptFlowPyrLK */

/* End of file. */
