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

static char* func_name[3] =
{
    "cvUpdateMHIByTime",
    "cvCalcMotionGradient",
    "cvCalcGlobalOrientation"
};

static int max_img_size, min_img_size;
static int img_size_delta_type, img_size_delta;
static int min_aperture_size = 3, max_aperture_size = 5;
static int base_iters;
static int init_mot_templ2_params = 0;

static char* test_desc = "comparing with the simple algorithm";


static void UpdateMHIByTimeEtalon( IplImage *silh, IplImage *mhi,
                                   float time_stamp, float mhi_duration )
{
    uchar*   silh_data;
    float*   mhi_data;
    float    low_time = time_stamp - mhi_duration;

    int      silh_step, mhi_step;
    int      i, j;
    CvSize  sz;

    atsGetImageInfo( silh, (void**)&silh_data, &silh_step, &sz, 0, 0, 0 );
    atsGetImageInfo( mhi, (void**)&mhi_data, &mhi_step, 0, 0, 0, 0 );

    mhi_step /= 4;

    for( i = 0; i < sz.height; i++, silh_data += silh_step, mhi_data += mhi_step )
        for( j = 0; j < sz.width; j++ )
        {
            if( silh_data[j] != 0 )
            {
                mhi_data[j] = time_stamp;
            }
            else if( mhi_data[j] < low_time )
            {
                mhi_data[j] = 0.f;
            }
        }
}


static void CalcMotionGradientEtalon(
                                   IplImage*  mhi,
                                   IplImage*  mask,
                                   IplImage*  orient,
                                   IplImage*  dervX_min,
                                   IplImage*  dervY_max,
                                   int   apertureSize,
                                   float max_delta, float min_delta,
                                   int   origin )
{
    float    limit;
    int      element_values[256];

    uchar*   mask_data;
    float*   x_data;
    float*   y_data;
    float*   orient_data;
    int      x_step, y_step, orient_step, mask_step;
    int      i, j;
    int      max_ker_width, max_ker_height;
    CvSize  sz;
    IplConvKernelFP* dX, *dY;
    IplConvKernel element;

    assert( 1 <= apertureSize && apertureSize <= 7 && (apertureSize&1) != 0 );

    /* build derivative kernels */
    dX = atsCalcDervConvKernel( 1, 0, apertureSize, origin );
    dY = atsCalcDervConvKernel( 0, 1, apertureSize, origin );

    max_ker_width = MAX( dX->nCols, dY->nCols );
    max_ker_height = MAX( dX->nRows, dY->nRows );

    /* calc derivatives "underflow" limit - then to clear orient */
    limit = 1e-4f*max_ker_width*max_ker_height;

#if 1
    /* prepare images */
    atsReplicateBorders( mhi, apertureSize, apertureSize );
    atsConvolve( mhi, dervX_min, dX );
    atsConvolve( mhi, dervY_max, dY );
#else
    /* calc derivatives */
    atsConvolve( mhi, dervX_min, dX );
    atsConvolve( mhi, dervY_max, dY );
#endif

    /* calc orientation */
    atsGetImageInfo( dervY_max, (void**)&y_data, &y_step, &sz, 0, 0, 0 );
    atsGetImageInfo( dervX_min, (void**)&x_data, &x_step, 0, 0, 0, 0 );
    atsGetImageInfo( orient, (void**)&orient_data, &orient_step, 0, 0, 0, 0 );

    x_step /= 4;
    y_step /= 4;
    orient_step /= 4;

    for( i = 0; i < sz.height; i++, x_data += x_step, y_data += y_step,
                                    orient_data += orient_step  )
        for( j = 0; j < sz.width; j++ )
        {
            float angle = (float)(atan2( y_data[j], x_data[j] )*57.29578f);
            if( angle < 0 ) angle += 360;
            orient_data[j] = fabs(y_data[j]) > limit || fabs(x_data[j]) > limit ? angle : 0;
        }

    /* build rectangular structuring element */
    element.nCols = max_ker_width;
    element.nRows = max_ker_height;
    element.anchorX = max_ker_width/2;
    element.anchorY = max_ker_height/2;
    element.nShiftR = 0;
    memset( element_values, -1, sizeof( element_values ));
    element.values = element_values;

    /* apply min and max filters */
    atsMinFilterEx( mhi, dervX_min, &element );
    atsMaxFilterEx( mhi, dervY_max, &element );

    /* calc mask */
    x_data -= sz.height * x_step;
    y_data -= sz.height * y_step;
    orient_data -= sz.height * orient_step;

    atsGetImageInfo( mask, (void**)&mask_data, &mask_step, 0, 0, 0, 0 );

    cvSet( mask, cvScalarAll(255) );

    for( i = 0; i < sz.height; i++, x_data += x_step, y_data += y_step,
                                    mask_data += mask_step,
                                    orient_data += orient_step )
        for( j = 0; j < sz.width; j++ )
        {
            float delta = y_data[j] - x_data[j];
            assert( delta >= 0 );
            mask_data[j] = (uchar)(min_delta <= delta && delta <= max_delta);
            /*if( !mask_data[j] ) orient_data[j] = 0;*/
        }

    atsDeleteConvKernelFP( dX );
    atsDeleteConvKernelFP( dY );
}


static void CalcGlobalOrientationEtalon( IplImage* orient, IplImage* mask,
                                         IplImage* mhi, float time_stamp,
                                         float mhi_duration, float* angle, float* delta )
{
#define HIST_SIZE 12
    float*   mhi_data;
    uchar*   mask_data;
    float*   orient_data;

    int      mhi_step, mask_step, orient_step;
    CvSize  sz;
    int      y, x;
    int      histogram[HIST_SIZE];
    int      max_bin = 0;

    double   base_orientation = 0, delta_orientation = 0, weight = 0;
    double   low_time = time_stamp - mhi_duration;
    double   global_orientation;

    atsGetImageInfo( mhi, (void**)&mhi_data, &mhi_step, &sz, 0, 0, 0 );
    atsGetImageInfo( mask, (void**)&mask_data, &mask_step, 0, 0, 0, 0 );
    atsGetImageInfo( orient, (void**)&orient_data, &orient_step, 0, 0, 0, 0 );

    orient_step /= sizeof(float);
    mhi_step /= sizeof(float);

    memset( histogram, 0, sizeof( histogram ));

    /* build historgam */
    for( y = 0; y < sz.height; y++ )
    {
        for( x = 0; x < sz.width; x++ )
            if( mask_data[x] )
            {
                int bin = cvFloor( (orient_data[x]*HIST_SIZE)/360 );
                histogram[bin < 0 ? 0 : bin >= HIST_SIZE ? HIST_SIZE-1 : bin]++;
            }
        orient_data += orient_step;
        mask_data += mask_step;
    }

    for( x = 1; x < HIST_SIZE; x++ )
    {
        if( histogram[x] > histogram[max_bin] ) max_bin = x;
    }

    base_orientation = ((double)max_bin*360)/HIST_SIZE;

    mask_data -= sz.height*mask_step;
    orient_data -= sz.height*orient_step;

    for( y = 0; y < sz.height; y++ )
    {
        for( x = 0; x < sz.width; x++ )
            if( mask_data[x] )
            {
                double diff = orient_data[x] - base_orientation;
                double delta_weight = mhi_data[x] >= low_time ?
                    (((mhi_data[x] - low_time)/mhi_duration)*254 + 1)/255 : 0;

                if( diff < -180 ) diff += 360;
                if( diff > 180 ) diff -= 360;

                if( delta_weight > 0 )
                {
                    delta_orientation += diff*delta_weight;
                    weight += delta_weight;
                }
            }
        mhi_data += mhi_step;
        orient_data += orient_step;
        mask_data += mask_step;
    }

    if( weight == 0 )
        global_orientation = base_orientation;
    else
    {
        global_orientation = base_orientation + delta_orientation/weight;
        if( global_orientation < 0 ) global_orientation += 360;
        if( global_orientation > 360 ) global_orientation -= 360;
    }
    *angle = (float)global_orientation;
    *delta = (float)delta_orientation;
}

static double compare_img_angles( IplImage* img0, IplImage* img1, IplImage* mask )
{
    float*   data0;
    float*   data1;
    uchar*   mask_data;
    int      step0, step1, mask_step;
    int      x, y;
    CvSize  sz;
    double   merr = 0;

    atsGetImageInfo( img0, (void**)&data0, &step0, &sz, 0, 0, 0 );
    atsGetImageInfo( img1, (void**)&data1, &step1, 0, 0, 0, 0 );
    atsGetImageInfo( mask, (void**)&mask_data, &mask_step, 0, 0, 0, 0 );

    step0 /= 4;
    step1 /= 4;

    for( y = 0; y < sz.height; y++, data0 += step0, data1 += step1, mask_data += mask_step )
        for( x = 0; x < sz.width; x++ )
        {
            if( mask_data[x] )
            {
                double err = atsCompareAngles( data0[x], data1[x] );
                merr = MAX( merr, err );
            }
        }
    return merr;
}

static void read_mot_templ2_params( void )
{
    if( !init_mot_templ2_params )
    {
        /* Read test params */
        trsiRead( &min_img_size, "6", "Minimal linear size of the image" );
        trsiRead( &max_img_size, "31", "Maximal linear size of the image" );
        trsCaseRead( &img_size_delta_type,"/a/m", "a", "a - add, m - multiply" );
        trsiRead( &img_size_delta, "3", "Image size step(factor)" );
        trsiRead( &base_iters, "1000", "Base number of iterations" );

        init_mot_templ2_params = 1;
    }
}


static int update_mhi_by_time_test( void )
{
    const float max_time = 100.f;
    const int success_error_level = 0;
    const int mhi_depth = IPL_DEPTH_32F;
    const int silh_depth = IPL_DEPTH_8U;
    const int channels = 1;

    int   seed = atsGetSeed();

    /* position where the maximum error occured */
    int   merr_w = 0, merr_h = 0, merr_iter = 0;

    /* test parameters */
    int     w = 0, h = 0, i = 0;
    float   time_stamp = 0, mhi_duration = 0;
    double  max_err = 0.;
    //int     code = TRS_OK;

    IplROI       roi;
    IplImage    *silh, *mhi, *mhi_copy;
    AtsRandState rng_state;
    atsRandInit( &rng_state, 0, 1, seed );

    read_mot_templ2_params();

    silh = atsCreateImage( max_img_size, max_img_size, silh_depth, channels, 0 );
    mhi = atsCreateImage( max_img_size, max_img_size, mhi_depth, channels, 0 );
    mhi_copy = atsCreateImage( max_img_size, max_img_size, mhi_depth, channels, 0 );

    roi.coi = 0;
    roi.xOffset = roi.yOffset = 0;

    silh->roi = mhi->roi = mhi_copy->roi = &roi;

    for( h = min_img_size; h <= max_img_size; )
    {
        for( w = min_img_size; w <= max_img_size; )
        {
            int  denom = (w - min_img_size + 1)*(h - min_img_size + 1)*channels;
            int  iters = (base_iters*2 + denom)/(2*denom);
            CvSize size;

            size.width = roi.width = w;
            size.height = roi.height = h;

            if( iters < 1 ) iters = 1;

            for( i = 0; i < iters; i++ )
            {
                double err;

                atsRandSetBounds( &rng_state, 0, max_time );

                time_stamp = atsRand32f( &rng_state );
                mhi_duration = atsRand32f( &rng_state );

                /*if( time_stamp < mhi_duration )
                {
                    float temp;
                    ATS_SWAP( time_stamp, mhi_duration, temp );
                }*/

                atsFillRandomImageEx( mhi, &rng_state );

                atsRandSetBounds( &rng_state, 0, 1 );
                atsFillRandomImageEx( silh, &rng_state );

                cvCopy( mhi, mhi_copy );

                UpdateMHIByTimeEtalon( silh, mhi_copy, time_stamp, mhi_duration );

                cvUpdateMHIByTime( silh, mhi, time_stamp, mhi_duration );

                err = cvNorm( mhi, mhi_copy, CV_C );

                if( err > max_err )
                {
                    merr_w    = w;
                    merr_h    = h;
                    merr_iter = i;
                    max_err   = err;
                    if( max_err > success_error_level ) goto test_exit;
                }
            }
            ATS_INCREASE( w, img_size_delta_type, img_size_delta );
        } /* end of the loop by w */

        ATS_INCREASE( h, img_size_delta_type, img_size_delta );
    }  /* end of the loop by h */

test_exit:

    silh->roi = mhi->roi = mhi_copy->roi = 0;

    atsReleaseImage( silh );
    atsReleaseImage( mhi );
    atsReleaseImage( mhi_copy );

    //if( code == TRS_OK )
    {
        trsWrite( ATS_LST, "Max err is %g at w = %d, h = %d, "
                           "iter = %d, seed = %08x",
                           max_err, merr_w, merr_h, merr_iter, seed );

        return max_err <= success_error_level ?
            trsResult( TRS_OK, "No errors" ) :
            trsResult( TRS_FAIL, "Bad accuracy" );
    }
    /*else
    {
        trsWrite( ATS_LST, "Fatal error at w = %d, h = %d, "
                           "iter = %d, seed = %08x",
                           w, h, i, seed );
        return trsResult( TRS_FAIL, "Function returns error code" );
    }*/
}



static int calc_motion_gradient_test( void )
{
    const float max_time = 100.f;
    const float delta_range = 5.f;
    const float success_orient_error_level = 1;
    const float success_mask_error_level = 1;
    const int mhi_depth = IPL_DEPTH_32F;
    const int orient_depth = IPL_DEPTH_32F;
    const int mask_depth = IPL_DEPTH_8U;
    const int silh_depth = IPL_DEPTH_8U;
    const int channels = 1;
    const int origin = 0;

    int   seed = atsGetSeed();

    /* position where the maximum error occured */
    int   merr_w = 0, merr_h = 0, merr_iter = 0, merr_aperture_size = 0;

    /* test parameters */
    int     w = 0, h = 0, i = 0;
    int     aperture_size = 0;
    float   max_delta = 0.f, min_delta = 0.f;
    double  max_orient_err = 0., max_mask_err = 0.;
    //int     code = TRS_OK;

    IplROI       roi;
    IplImage    *orient, *silh, *orient2, *mask, *mask2, *mhi;
    IplImage    *dervX_min, *dervY_max;
    AtsRandState rng_state;

    atsRandInit( &rng_state, 0, 1, seed );

    read_mot_templ2_params();

    orient = atsCreateImage( max_img_size, max_img_size, orient_depth, channels, 0 );
    orient2 = atsCreateImage( max_img_size, max_img_size, orient_depth, channels, 0 );
    mhi = atsCreateImage( max_img_size, max_img_size, mhi_depth, channels, 0 );
    mask = atsCreateImage( max_img_size, max_img_size, mask_depth, channels, 0 );
    mask2 = atsCreateImage( max_img_size, max_img_size, mask_depth, channels, 0 );
    silh  = atsCreateImage( max_img_size, max_img_size, silh_depth, channels, 0 );
    dervX_min = atsCreateImage( max_img_size, max_img_size, mhi_depth, channels, 0 );
    dervY_max = atsCreateImage( max_img_size, max_img_size, mhi_depth, channels, 0 );

    cvZero( silh );

    roi.coi = 0;
    roi.xOffset = roi.yOffset = 0;

    orient->roi = orient2->roi = mhi->roi = mask->roi = mask2->roi = silh->roi = &roi;
    dervX_min->roi = dervY_max->roi = &roi;

    for( h = min_img_size; h <= max_img_size; )
    {
        for( w = min_img_size; w <= max_img_size; )
        {
            int  denom = (w - min_img_size + 1)*(h - min_img_size + 1)*channels;
            int  iters = (base_iters*2 + denom)/(2*denom);
            CvSize size;

            size.width = roi.width  = w;
            size.height = roi.height = h;

            if( iters < 1 ) iters = 1;

            for( i = 0; i < iters; i++ )
            {
                double err;

                atsRandSetBounds( &rng_state, 0, delta_range );

                max_delta = atsRand32f( &rng_state ) + delta_range;
                min_delta = atsRand32f( &rng_state );

                atsRandSetBounds( &rng_state, 0, max_time );
                atsFillRandomImageEx( mhi, &rng_state );

                /* cut off some motion */
                UpdateMHIByTimeEtalon( silh, mhi, max_time, max_time*2/3 );

                for( aperture_size = min_aperture_size; aperture_size <= max_aperture_size;
                     aperture_size += 2 )
                {
                    CalcMotionGradientEtalon( mhi, mask2, orient2, dervX_min, dervY_max,
                                              aperture_size, max_delta, min_delta, origin );

                    cvCalcMotionGradient( mhi, mask, orient, max_delta, min_delta, aperture_size );

                    /* compare angles */
                    err = compare_img_angles( orient, orient2, mask2 );

                    if( err > max_orient_err )
                    {
                        merr_w    = w;
                        merr_h    = h;
                        merr_iter = i;
                        merr_aperture_size = aperture_size;
                        max_orient_err   = err;
                        if( max_orient_err > success_orient_error_level )
                            goto test_exit;
                    }

                    err = cvNorm( mask, mask2, CV_L1 );
                    cvXor( mask, mask2, mask );

                    if( err > max_mask_err )
                    {
                        merr_w    = w;
                        merr_h    = h;
                        merr_iter = i;
                        merr_aperture_size = aperture_size;
                        max_mask_err   = err;
                        if( max_mask_err > success_mask_error_level )
                            goto test_exit;
                    }

                    roi.xOffset = roi.yOffset = 0;
                    roi.width = w;
                    roi.height = h;
                }
            }
            ATS_INCREASE( w, img_size_delta_type, img_size_delta );
        } /* end of the loop by w */

        ATS_INCREASE( h, img_size_delta_type, img_size_delta );
    }  /* end of the loop by h */

test_exit:

    silh->roi = mhi->roi = mask->roi = mask2->roi = orient->roi = orient2->roi = 0;
    dervX_min->roi = dervY_max->roi = 0;

    atsReleaseImage( silh );
    atsReleaseImage( mhi );
    atsReleaseImage( mask );
    atsReleaseImage( mask2);
    atsReleaseImage( orient );
    atsReleaseImage( orient2 );
    atsReleaseImage( dervX_min );
    atsReleaseImage( dervY_max );

    //if( code == TRS_OK )
    {
        trsWrite( ATS_LST, "Max orient err is %g, Max mask err is %g at w = %d, h = %d, "
                           "iter = %d, aperture_size = %d, seed = %08x",
                           max_orient_err, max_mask_err, merr_w, merr_h, merr_iter,
                           merr_aperture_size, seed );

        return max_orient_err <= success_orient_error_level &&
               max_mask_err <= success_mask_error_level ?
            trsResult( TRS_OK, "No errors" ) :
               max_mask_err > success_mask_error_level ?
            trsResult( TRS_FAIL, "Bad mask accuracy" ) :
            trsResult( TRS_FAIL, "Bad orient accuracy" );
    }
    /*else
    {
        trsWrite( ATS_LST, "Fatal error at w = %d, h = %d, "
                           "iter = %d, aperture_size = %d, seed = %08x",
                           w, h, i, aperture_size, seed );
        return trsResult( TRS_FAIL, "Function returns error code" );
    }*/
}



static int calc_global_orientation_test( void )
{
    const float max_time = 2000.f;
    const float mask_range = 3.f;
    const float success_orient_error_level = 30;
    const int mhi_depth = IPL_DEPTH_32F;
    const int orient_depth = IPL_DEPTH_32F;
    const int mask_depth = IPL_DEPTH_8U;
    const int silh_depth = IPL_DEPTH_8U;
    const int channels = 1;

    int   seed = atsGetSeed();

    /* position where the maximum error occured */
    int   merr_w = 0, merr_h = 0, merr_iter = 0;

    /* test parameters */
    int     w = 0, h = 0, i = 0;
    float   time_stamp = 0.f, mhi_duration = 0.f;
    float   min_angle = 0.f, max_angle = 0.f;

    float   res_angle = 0.f, std_angle = 0.f;
    double  max_err = 0.;
    int     code = TRS_OK;

    IplROI       roi;
    IplImage    *orient, *silh, *mask, *mhi;
    AtsRandState rng_state;

    atsRandInit( &rng_state, 0, 1, seed );

    read_mot_templ2_params();

    orient = atsCreateImage( max_img_size, max_img_size, orient_depth, channels, 0 );
    mhi = atsCreateImage( max_img_size, max_img_size, mhi_depth, channels, 0 );
    mask = atsCreateImage( max_img_size, max_img_size, mask_depth, channels, 0 );
    silh  = atsCreateImage( max_img_size, max_img_size, silh_depth, channels, 0 );

    cvZero( silh );

    roi.coi = 0;
    roi.xOffset = roi.yOffset = 0;

    orient->roi = mhi->roi = mask->roi = silh->roi = &roi;

    for( h = MAX( 4, min_img_size); h <= max_img_size; )
    {
        for( w = MAX( 4, min_img_size); w <= max_img_size; )
        {
            int  denom = (w - min_img_size + 1)*(h - min_img_size + 1)*channels;
            int  iters = (base_iters*2 + denom)/(2*denom);
            CvSize size;

            if( iters < 1 ) iters = 1;

            size.width = roi.width  = w;
            size.height = roi.height = h;

            for( i = 0; i < iters; i++ )
            {
                double err;
                float  temp, delta = 0;

                atsRandSetBounds( &rng_state, 0, max_time );

                time_stamp = atsRand32f( &rng_state ) + 0.01f;
                mhi_duration = atsRand32f( &rng_state ) + 0.01f;

                if( time_stamp < mhi_duration )
                    ATS_SWAP( time_stamp, mhi_duration, temp );

                if( time_stamp < mhi_duration + 1 ) time_stamp += 1.f;

                atsRandSetBounds( &rng_state, mhi_duration, time_stamp );
                atsFillRandomImageEx( mhi, &rng_state );

                /* cut off some motion */
                UpdateMHIByTimeEtalon( silh, mhi, max_time, max_time*2/3 );

                /* generate random angles range */
                atsRandSetBounds( &rng_state, 0, 360 );
                min_angle = atsRand32f( &rng_state );
                max_angle = atsRand32f( &rng_state );

                if( min_angle > max_angle )
                    ATS_SWAP( min_angle, max_angle, temp );

                /* make orientation image */
                min_angle += (max_angle - min_angle)*0.3f;
                max_angle -= (max_angle - min_angle)*0.3f;

                atsRandSetBounds( &rng_state, min_angle, max_angle );
                atsFillRandomImageEx( orient, &rng_state );

                /* make mask image */
                atsRandSetBounds( &rng_state, 1, mask_range+1 );
                atsFillRandomImageEx( mask, &rng_state );
                cvSubS( mask, cvScalarAll((int)(mask_range - 1)), mask );

                CalcGlobalOrientationEtalon( orient, mask, mhi, time_stamp,
                                             mhi_duration, &std_angle, &delta );

                res_angle = (float)cvCalcGlobalOrientation( orient, mask, mhi,
                                                            time_stamp, mhi_duration );

                if( !(min_angle - success_orient_error_level <= res_angle &&
                      max_angle + success_orient_error_level >= res_angle) &&
                    !(min_angle - success_orient_error_level <= res_angle+360 &&
                      max_angle + success_orient_error_level >= res_angle+360)
                      && w > 4 && h > 4 )
                {
                    code = -2;
                    goto test_exit;
                }

                /* compare angles */
                err = atsCompareAngles( res_angle, std_angle );

                if( err > max_err )
                {
                    merr_w    = w;
                    merr_h    = h;
                    merr_iter = i;
                    max_err = err;
                    if( max_err > success_orient_error_level )
                        goto test_exit;
                }
            }
            ATS_INCREASE( w, img_size_delta_type, img_size_delta );
        } /* end of the loop by w */

        ATS_INCREASE( h, img_size_delta_type, img_size_delta );
    }  /* end of the loop by h */

test_exit:

    silh->roi = mhi->roi = mask->roi = orient->roi = 0;

    atsReleaseImage( silh );
    atsReleaseImage( mhi );
    atsReleaseImage( mask );
    atsReleaseImage( orient );

    if( code == TRS_OK )
    {
        trsWrite( ATS_LST, "Max orient err is %g at w = %d, h = %d, "
                           "iter = %d, seed = %08x",
                           max_err, merr_w, merr_h, merr_iter, seed );

        return max_err <= success_orient_error_level ?
            trsResult( TRS_OK, "No errors" ) :
            trsResult( TRS_FAIL, "Bad orient accuracy" );
    }
    else
    {
        trsWrite( ATS_LST, "Fatal error at w = %d, h = %d, iter = %d, seed = %08x",
                           w, h, i, seed );
        return trsResult( TRS_FAIL, "Function returns error code" );
    }
}


void InitAMotionTemplates( void )
{
    trsReg( func_name[0], test_desc, atsAlgoClass, update_mhi_by_time_test );
    trsReg( func_name[1], test_desc, atsAlgoClass, calc_motion_gradient_test );
    trsReg( func_name[2], test_desc, atsAlgoClass, calc_global_orientation_test );

} /* InitAMotionTemplates */

/* End of file. */

/* End of file. */
