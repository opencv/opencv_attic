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
    "cvMoments",
    "cvGetHuMoments"
};

static char *test_desc = "Comparing with the simple algorithm";

#define IMGSTAT_MOMENTS          0
#define IMGSTAT_MOMENTS_BINARY   1

/* actual parameters */
static int min_img_size, max_img_size;
static int img_size_delta_type, img_size_delta;
static int base_iters;

/* which tests have to run */
static int fn_l = 0, fn_h = ATS_DIM(funcs)-1,
           dt_l = 0, dt_h = 2,
           ch_l = 0, ch_h = 1;

static int init_moments_params = 0;

static const int img8u_range = 255;
static const int img8s_range = 128;
static const float img32f_range = 100.f;
static const int img32f_bits = 3;

static double rel_err( double a, double b )
{
    return fabs(a - b)/(fabs(a) + 1);
}

#if 0
static void convert_ipl_to_ats( IplMomentState lstate, AtsMomentState* astate )
{
    astate->m00 = iplGetSpatialMoment( lstate, 0, 0 );
    astate->m10 = iplGetSpatialMoment( lstate, 1, 0 );
    astate->m01 = iplGetSpatialMoment( lstate, 0, 1 );
    astate->m20 = iplGetSpatialMoment( lstate, 2, 0 );
    astate->m11 = iplGetSpatialMoment( lstate, 1, 1 );
    astate->m02 = iplGetSpatialMoment( lstate, 0, 2 );
    astate->m30 = iplGetSpatialMoment( lstate, 3, 0 );
    astate->m21 = iplGetSpatialMoment( lstate, 2, 1 );
    astate->m12 = iplGetSpatialMoment( lstate, 1, 2 );
    astate->m03 = iplGetSpatialMoment( lstate, 0, 3 );

    astate->mu20 = iplGetCentralMoment( lstate, 2, 0 );
    astate->mu11 = iplGetCentralMoment( lstate, 1, 1 );
    astate->mu02 = iplGetCentralMoment( lstate, 0, 2 );
    astate->mu30 = iplGetCentralMoment( lstate, 3, 0 );
    astate->mu21 = iplGetCentralMoment( lstate, 2, 1 );
    astate->mu12 = iplGetCentralMoment( lstate, 1, 2 );
    astate->mu03 = iplGetCentralMoment( lstate, 0, 3 );

    /* calc normalized moments */
    {
        double inv_m00 = astate->m00 == 0 ? 0 : 1./astate->m00;
        double s2 = inv_m00*inv_m00; /* 1./(m00 ^ (2/2 + 1)) */
        double s3 = s2*sqrt(inv_m00); /* 1./(m00 ^ (3/2 + 1)) */

        astate->nu20 = astate->mu20 * s2;
        astate->nu11 = astate->mu11 * s2;
        astate->nu02 = astate->mu02 * s2;

        astate->nu30 = astate->mu30 * s3;
        astate->nu21 = astate->mu21 * s3;
        astate->nu12 = astate->mu12 * s3;
        astate->nu03 = astate->mu03 * s3;
    }
}
#endif

static void read_moments_params( void )
{
    if( !init_moments_params )
    {
        int func, data_types, channels;

        /* Determine which tests are needed to run */
        trsCaseRead( &func, "/a/m/mb", "a",
                     "Function type: \n"
                     "a - all\n"
                     "m - moments\n"
                     "mb - moments binary\n");
        if( func != 0 ) fn_l = fn_h = func - 1;

        trsCaseRead( &data_types,"/a/8u/8s/32f", "a",
            "a - all, 8u - unsigned char, 8s - signed char, 32f - float" );
        if( data_types != 0 ) dt_l = dt_h = data_types - 1;

        trsCaseRead( &channels, "/a/1/3", "a", "a - all, 1 - single channel, 3 - three channels" );
        if( channels != 0 ) ch_l = ch_h = channels - 1;

        /* read tests params */
        trsiRead( &min_img_size, "1", "Minimal width or height of image" );
        trsiRead( &max_img_size, "1000", "Maximal width or height of image" );
        trsCaseRead( &img_size_delta_type,"/a/m", "m", "a - add, m - multiply" );
        trsiRead( &img_size_delta, "3", "Image size step(factor)" );
        trsiRead( &base_iters, "100", "Base number of iterations" );

        init_moments_params = 1;
    }
}

/* ///////////////////// moments_test ///////////////////////// */
static int moments_test( void* arg )
{
    static double weight[] = { 
        1e6, 1e6, 1e6,  /* m00, m10, m01 */
        1e6, 1e6, 1e6, 1, 1, 1, 1, /* m20 - m03 */
        1, 1e-4, 1, 1e-5, 1e-5, 1e-5, 1e-5, /* mu20 - mu03 */
        1, 1, 1, 1, 1, 1, 1 }; /* nu20 - nu03 */

    const double success_error_level = 1e-4;
    int   param     = (int)arg;
    int   binary    = param >= 6;

    int   depth     = (param % 6)/2;
    int   channels  = (param & 1);

    int   seed      = atsGetSeed();

    /* position where the maximum error occured */
    int   merr_w = 0, merr_h = 0, merr_iter = 0, merr_c = 0;

    /* test parameters */
    int     w = 0, h = 0, i = 0, c = 0;
    double  max_err = 0.;
    //int     code = TRS_OK;

    IplROI       roi;
    IplImage    *img;
    AtsRandState rng_state;

    atsRandInit( &rng_state, 0, 1, seed );

    read_moments_params();

    if( !(ATS_RANGE( binary, fn_l, fn_h+1 ) &&
          ATS_RANGE( depth, dt_l, dt_h+1 ) &&
          ATS_RANGE( channels, ch_l, ch_h+1 ))) return TRS_UNDEF;

    depth = depth == 2 ? IPL_DEPTH_32F : depth == 1 ? IPL_DEPTH_8S : IPL_DEPTH_8U;
    channels = channels*2 + 1;

    img  = atsCreateImage( max_img_size, max_img_size, depth, channels, 0 );

    roi.coi = 0;
    roi.xOffset = roi.yOffset = 0;

    img->roi = &roi;

    for( h = min_img_size; h <= max_img_size; )
    {
        for( w = min_img_size; w <= max_img_size; )
        {
            int  denom = (w - min_img_size + 1)*(h - min_img_size + 1)*channels;
            int  iters = (base_iters*2 + denom)/(2*denom);

            roi.width = w;
            roi.height = h;

            if( iters < 1 ) iters = 1;

            for( i = 0; i < iters; i++ )
            {
                switch( depth )
                {
                case IPL_DEPTH_8U:
                    atsRandSetBounds( &rng_state, 0, img8u_range );
                    break;
                case IPL_DEPTH_8S:
                    atsRandSetBounds( &rng_state, -img8s_range, img8s_range );
                    break;
                case IPL_DEPTH_32F:
                    atsRandSetBounds( &rng_state, -img32f_range, img32f_range );
                    if( binary ) atsRandSetFloatBits( &rng_state, img32f_bits );
                    break;
                }

                roi.coi = 0;
                atsFillRandomImageEx( img, &rng_state );
                /*iplSet( img, depth == IPL_DEPTH_8S ? 125 : 251 );*/

                for( c = 1; c <= channels; c++ )
                {
                    double err0 = 0;
                    AtsMomentState astate0, astate1;
                    CvMoments istate;
                    double* a0 = (double*)&astate0;
                    double* a1 = (double*)&astate1;
                    int j;

                    roi.coi = c;


                    /* etalon function */
                    atsCalcMoments( img, &astate0, binary );

                    /* cv function */
                    cvMoments( img, &istate, binary );

                    atsGetMoments( &istate, &astate1 );

                    /*iplMoments( img, lstate ); */
                    /*convert_ipl_to_ats( lstate, &astate1 ); */

                    for( j = 0; j < sizeof(astate0)/sizeof(double); j++ )
                    {
                        double err = rel_err( a0[j], a1[j] )*weight[j];
                        err0 = MAX( err0, err );
                    }

                    if( err0 > max_err )
                    {
                        merr_w    = w;
                        merr_h    = h;
                        merr_iter = i;
                        merr_c    = c;
                        max_err   = err0;
                        if( max_err > success_error_level )
                            goto test_exit;
                    }
                }
            }
            ATS_INCREASE( w, img_size_delta_type, img_size_delta );
        } /* end of the loop by w */

        ATS_INCREASE( h, img_size_delta_type, img_size_delta );
    }  /* end of the loop by h */

test_exit:

    img->roi = 0;

    cvReleaseImage(& img);

    //if( code == TRS_OK )
    {
        trsWrite( ATS_LST, "Max err is %g at w = %d, h = %d, "
                           "iter = %d, c = %d, seed = %08x",
                           max_err, merr_w, merr_h, merr_iter, merr_c, seed );

        return max_err <= success_error_level ?
            trsResult( TRS_OK, "No errors" ) :
            trsResult( TRS_FAIL, "Bad accuracy" );
    }
    /*else
    {
        trsWrite( ATS_LST, "Fatal error at w = %d, h = %d, "
                           "iter = %d, c = %d, seed = %08x",
                           w, h, i, c, seed );
        return trsResult( TRS_FAIL, "Function returns error code" );
    }*/
}


static double sqr( double x ) { return x*x; }

static int hu_moments_test( void )
{
    const double success_error_level = 1e-7;
    CvSize size = { 512, 512 };
    int i;
    IplImage* img = atsCreateImage( size.width, size.height, 8, 1, 0 );
    CvMoments moments;
    CvHuMoments a, b;
    AtsRandState rng_state;

    int  seed = atsGetSeed();

    double nu20, nu02, nu11, nu30, nu21, nu12, nu03;
    double err = 0;
    char   buffer[100];

    atsRandInit( &rng_state, 0, 255, seed );
    atsbRand8u( &rng_state, (uchar*)(img->imageData), size.width * size.height );

    cvMoments( img, &moments, 0 );
    atsReleaseImage( img );

    nu20 = cvGetNormalizedCentralMoment( &moments, 2, 0 );
    nu11 = cvGetNormalizedCentralMoment( &moments, 1, 1 );
    nu02 = cvGetNormalizedCentralMoment( &moments, 0, 2 );

    nu30 = cvGetNormalizedCentralMoment( &moments, 3, 0 );
    nu21 = cvGetNormalizedCentralMoment( &moments, 2, 1 );
    nu12 = cvGetNormalizedCentralMoment( &moments, 1, 2 );
    nu03 = cvGetNormalizedCentralMoment( &moments, 0, 3 );

    cvGetHuMoments( &moments, &a );

    b.hu1 = nu20 + nu02;
    b.hu2 = sqr(nu20 - nu02) + 4*sqr(nu11);
    b.hu3 = sqr(nu30 - 3*nu12) + sqr(3*nu21 - nu03);
    b.hu4 = sqr(nu30 + nu12) + sqr(nu21 + nu03);
    b.hu5 = (nu30 - 3*nu12)*(nu30 + nu12)*(sqr(nu30 + nu12) - 3*sqr(nu21 + nu03)) +
            (3*nu21 - nu03)*(nu21 + nu03)*(3*sqr(nu30 + nu12) - sqr(nu21 + nu03));
    b.hu6 = (nu20 - nu02)*(sqr(nu30 + nu12) - sqr(nu21 + nu03)) +
            4*nu11*(nu30 + nu12)*(nu21 + nu03);
    b.hu7 = (3*nu21 - nu03)*(nu30 + nu12)*(sqr(nu30 + nu12) - 3*sqr(nu21 + nu03)) +
            (3*nu12 - nu30)*(nu21 + nu03)*(3*sqr(nu30 + nu12) - sqr(nu21 + nu03));

    for( i = 0; i < 7; i++ )
    {
        double t = rel_err( ((double*)&b)[i], ((double*)&a)[i] );
        if( t > err ) err = t;
    }

    sprintf( buffer, "Accuracy: %.4e", err );
    return trsResult( err > success_error_level ? TRS_FAIL : TRS_OK, buffer );
}


#define _8U_C1     0
#define _8U_C3     1
#define _8S_C1     2
#define _8S_C3     3
#define _32F_C1    4
#define _32F_C3    5

#define BIN_8U_C1     6
#define BIN_8U_C3     7
#define BIN_8S_C1     8
#define BIN_8S_C3     9
#define BIN_32F_C1   10
#define BIN_32F_C3   11

void InitAMoments( void )
{
    /* Register test functions */

    trsRegArg( funcs[0], test_desc, atsAlgoClass, moments_test, _8U_C1 );
    trsRegArg( funcs[0], test_desc, atsAlgoClass, moments_test, _8U_C3 );
    trsRegArg( funcs[0], test_desc, atsAlgoClass, moments_test, _8S_C1 );
    trsRegArg( funcs[0], test_desc, atsAlgoClass, moments_test, _8S_C3 );
    trsRegArg( funcs[0], test_desc, atsAlgoClass, moments_test, _32F_C1 );
    trsRegArg( funcs[0], test_desc, atsAlgoClass, moments_test, _32F_C3 );

    trsRegArg( funcs[0], test_desc, atsAlgoClass, moments_test, BIN_8U_C1 );
    trsRegArg( funcs[0], test_desc, atsAlgoClass, moments_test, BIN_8U_C3 );
    trsRegArg( funcs[0], test_desc, atsAlgoClass, moments_test, BIN_8S_C1 );
    trsRegArg( funcs[0], test_desc, atsAlgoClass, moments_test, BIN_8S_C3 );
    trsRegArg( funcs[0], test_desc, atsAlgoClass, moments_test, BIN_32F_C1 );
    trsRegArg( funcs[0], test_desc, atsAlgoClass, moments_test, BIN_32F_C3 );

    trsReg( funcs[1], test_desc, atsAlgoClass, hu_moments_test );

} /* InitAMoments */

/* End of file. */

/* End of file. */
