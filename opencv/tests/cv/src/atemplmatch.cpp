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
    "cvMatchTemplate"
};

static char *test_desc = "Test for template matching functions";

/* actual parameters */
static int min_img_size, max_img_size;
static int img_size_delta_type, img_size_delta;
static int min_templ_size;
static int templ_size_delta_type, templ_size_delta;
static int base_iters;

/* which tests have to run */
static int dt_l = 0, dt_h = 2,
           meth_l = 0, meth_h = 5;

static int init_templ_match_params = 0;

static const int img8u_range = 255;
static const int img8s_range = 128;
static const float img32f_range = 100.f;
static const int img32f_bits = 23;

static void read_templ_match_params( void )
{
    if( !init_templ_match_params )
    {
        int  data_types, method;

        /* Determine which tests are needed to run */
        trsCaseRead( &data_types,"/a/8u/8s/32f", "a",
            "a - all, 8u - unsigned char, 8s - signed char, 32f - float" );
        if( data_types != 0 ) dt_l = dt_h = data_types - 1;

        trsCaseRead( &method,"/a/sd/sdn/cr/crn/cf/cfn", "a",
            "\n\ta   - all"
            "\n\tsd  - squared diff"
            "\n\tsdn - normed squared diff"
            "\n\tcr  - cross correlation"
            "\n\tcrn - normed cross correlation"
            "\n\tcf  - correlation coefficient"
            "\n\tcrn - normed correlation coefficient");
        if( method != 0 ) meth_l = meth_h = method - 1;

        /* read tests params */
        trsiRead( &min_img_size, "1", "Minimal width or height of image" );
        trsiRead( &max_img_size, "32", "Maximal width or height of image" );
        trsCaseRead( &img_size_delta_type,"/a/m", "m", "a - add, m - multiply" );
        trsiRead( &img_size_delta, "3", "Image size step(factor)" );
        trsiRead( &min_templ_size, "1", "Minimal width or height of image" );
        trsCaseRead( &templ_size_delta_type,"/a/m", "a", "a - add, m - multiply" );
        trsiRead( &templ_size_delta, "3", "Image size step(factor)" );
        trsiRead( &base_iters, "1000", "Base number of iterations" );

        init_templ_match_params = 1;
    }
}


static double calc_mean( IplImage* src )
{
    double mean;
    atsCalcImageStatistics( src, 0, 0, 0, 0, 0,
                            0, 0, &mean, 0, 0, 0, 0, 0 );
    return mean;
}


static void match_templ_etalon( IplImage* src, IplImage* templ, IplImage* dst,
                                CvTemplMatchMethod method )
{
    float*   dst_data = 0;
    int      dst_step = 0;
    int      depth = 0;
    int      i, j;
    CvSize  src_sz, templ_sz, dst_sz;
    IplROI*  src_roi = src->roi;
    IplROI   tsrc_roi;

    int      is_normed, is_centered;
    double   nrmT = 1, deltaT = 0;

    atsGetImageInfo( src,   0, 0, &src_sz, &depth, 0, 0 );
    atsGetImageInfo( templ, 0, 0, &templ_sz, 0, 0, 0 );
    atsGetImageInfo( dst, (void**)&dst_data, &dst_step, &dst_sz, 0, 0, 0 );

    assert( depth == IPL_DEPTH_32F );
    assert( templ_sz.width <= src_sz.width &&
            templ_sz.height <= src_sz.height &&
            dst_sz.width == src_sz.width - templ_sz.width + 1 &&
            dst_sz.height == src_sz.height - templ_sz.height + 1 );

    src->roi = &tsrc_roi;
    tsrc_roi.coi = 0;
    tsrc_roi.width = templ_sz.width;
    tsrc_roi.height = templ_sz.height;

    is_normed = method == CV_TM_SQDIFF_NORMED ||
                method == CV_TM_CCORR_NORMED ||
                method == CV_TM_CCOEFF_NORMED;

    is_centered = method == CV_TM_CCOEFF || method == CV_TM_CCOEFF_NORMED;

    if( is_centered ) deltaT = calc_mean( templ );
    if( is_normed ) nrmT = atsCrossCorr( templ, templ, deltaT, deltaT );
    dst_step /= 4;

    for( i = 0; i < dst_sz.height; i++ )
    {
        for( j = 0; j < dst_sz.width; j++ )
        {
            double val = 0, nrmS = 1, deltaS = 0;

            tsrc_roi.xOffset = j;
            tsrc_roi.yOffset = i;

            if( is_centered ) deltaS = calc_mean( src );
            if( is_normed ) nrmS = atsCrossCorr( src, src, deltaS, deltaS );
            
            switch( method )
            {
            case CV_TM_SQDIFF_NORMED:
            case CV_TM_SQDIFF:
                val = cvNorm( src, templ, CV_L2 );
                val *= val;
                break;
            default:
                val = atsCrossCorr( src, templ, deltaS, deltaT );
            }

            if( is_normed )
            {
                val /= (sqrt(nrmS + FLT_EPSILON)*sqrt(nrmT + FLT_EPSILON));
            }
            dst_data[i*dst_step + j] = (float)val;
        }
    }

    src->roi = src_roi;
}


/* ///////////////////// match_templ_test ///////////////////////// */

static int match_templ_test( void* arg )
{
    const double success_error_level = 1e-6;
    const double error_level_scale = 1e-3;

    int   param     = (int)arg;
    int   depth     = param/6;
    int   method    = param%6;

    int   seed = atsGetSeed();

    /* position where the maximum error occured */
    int   merr_w = 0, merr_h = 0, merr_tw = 0, merr_th = 0, merr_iter = 0;

    /* test parameters */
    int     w = 0, h = 0, tw = 0, th = 0, i = 0;
    double  max_err = 0.;
    //int     code = TRS_OK;

    IplROI       src_roi, templ_roi, dst_roi;
    IplImage    *src_img, *templ_img, *dst_img, *dst2_img;
    IplImage    *srcfl_img = 0, *templfl_img = 0;
    AtsRandState rng_state;

    atsRandInit( &rng_state, 0, 1, seed );

    read_templ_match_params();

    if( !(ATS_RANGE( depth, dt_l, dt_h+1 ) &&
          ATS_RANGE( method, meth_l, meth_h+1 ))) return TRS_UNDEF;

    depth = depth == 2 ? IPL_DEPTH_32F : depth == 1 ? IPL_DEPTH_8S : IPL_DEPTH_8U;

    src_img = atsCreateImage( max_img_size, max_img_size, depth, 1, 0 );
    templ_img = atsCreateImage( max_img_size, max_img_size, depth, 1, 0 );
    dst_img = atsCreateImage( max_img_size, max_img_size, IPL_DEPTH_32F, 1, 0 );
    dst2_img = atsCreateImage( max_img_size, max_img_size, IPL_DEPTH_32F, 1, 0 );

    if( depth != IPL_DEPTH_32F )
    {
        srcfl_img = atsCreateImage( max_img_size, max_img_size, IPL_DEPTH_32F, 1, 0 );
        templfl_img = atsCreateImage( max_img_size, max_img_size, IPL_DEPTH_32F, 1, 0 );
    }
    else
    {
        srcfl_img = src_img;
        templfl_img = templ_img;
    }
    
    src_img->roi = srcfl_img->roi = &src_roi;
    templ_img->roi = templfl_img->roi = &templ_roi;
    dst_img->roi = dst2_img->roi = &dst_roi;

    src_roi.coi = dst_roi.coi = templ_roi.coi = 0;

    src_roi.xOffset = src_roi.yOffset =
    templ_roi.xOffset = templ_roi.yOffset =     
    dst_roi.xOffset = dst_roi.yOffset = 0;

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
        atsRandSetFloatBits( &rng_state, img32f_bits );
        break;
    }

    for( h = min_img_size; h <= max_img_size; )
    {
        for( w = min_img_size; w <= max_img_size; )
        {
            int th_limit = h < 50 ? h : h/2;
            int tw_limit = w < 50 ? w : w/2;

            src_roi.width = w;
            src_roi.height = h;

            atsFillRandomImageEx( src_img, &rng_state );
            
            for( th = min_templ_size; th <= th_limit; )
            {
                for( tw = min_templ_size; tw <= tw_limit; )
                {
                    int  denom = (w - min_img_size + 1)*(h - min_img_size + 1)*
                                 (tw - min_templ_size+1)*(th - min_templ_size+1);
                    int  iters = (base_iters*2 + denom)/(2*denom);

                    templ_roi.width = tw;
                    templ_roi.height = th;

                    dst_roi.width = w - tw + 1;
                    dst_roi.height = h - th + 1;

                    if( iters < 1 ) iters = 1;

                    for( i = 0; i < iters; i++ )
                    {
                        double err;
                        atsFillRandomImageEx( templ_img, &rng_state );

                        if( depth != IPL_DEPTH_32F )
                        {
                            atsConvert( src_img, srcfl_img );
                            atsConvert( templ_img, templfl_img );
                        }

                        match_templ_etalon( srcfl_img, templfl_img, dst2_img,
                                            (CvTemplMatchMethod)method );

                        /* call the library function */
                        cvMatchTemplate( src_img, templ_img, dst_img,
                                         (CvTemplMatchMethod)method );

                        err = cvNorm( dst_img, dst2_img, CV_C );

                        if( method == 5 && tw == 1 && th == 1 )
                        {
                            err *= error_level_scale;
                        }

                        if( err > max_err )
                        {
                            merr_w    = w;
                            merr_h    = h;
                            merr_tw   = tw;
                            merr_th   = th;
                            merr_iter = i;
                            max_err   = err;
                            if( max_err > success_error_level ) goto test_exit;
                        }
                    }
                    ATS_INCREASE( tw, templ_size_delta_type, templ_size_delta );
                }
                ATS_INCREASE( th, templ_size_delta_type, templ_size_delta );
            }
            ATS_INCREASE( w, img_size_delta_type, img_size_delta );
        } /* end of the loop by w */
        ATS_INCREASE( h, img_size_delta_type, img_size_delta );
    }  /* end of the loop by h */

test_exit:

    dst_img->roi = dst2_img->roi = 0;
    atsReleaseImage( dst_img );
    
    src_img->roi = srcfl_img->roi = 0;
    atsReleaseImage( src_img );

    templ_img->roi = templfl_img->roi = 0;
    atsReleaseImage( templ_img );

    if( depth != IPL_DEPTH_32F )
    {
        atsReleaseImage( srcfl_img );
        atsReleaseImage( templfl_img );
    }

    //if( code == TRS_OK )
    {
        trsWrite( ATS_LST, "Max err is %g at w = %d, h = %d, "
                           "tw = %d, th = %d, iter = %d, seed = %08x",
                           max_err, merr_w, merr_h, merr_tw, merr_th,
                           merr_iter, seed );

        return max_err <= success_error_level ?
            trsResult( TRS_OK, "No errors" ) :
            trsResult( TRS_FAIL, "Bad accuracy" );
    }
    /*else
    {
        trsWrite( ATS_LST, "Fatal error at w = %d, h = %d, "
                           "tw = %d, th = %d, iter = %d, seed = %08x",
                           w, h, tw, th, i, seed );
        return trsResult( TRS_FAIL, "Function returns error code" );
    }*/
}


#define MTEMPL_SQDIFF_8U          0
#define MTEMPL_SQDIFF_NORMED_8U   1
#define MTEMPL_CCORR_8U           2
#define MTEMPL_CCORR_NORMED_8U    3
#define MTEMPL_CCOEFF_8U          4
#define MTEMPL_CCOEFF_NORMED_8U   5

#define MTEMPL_SQDIFF_8S          6
#define MTEMPL_SQDIFF_NORMED_8S   7
#define MTEMPL_CCORR_8S           8
#define MTEMPL_CCORR_NORMED_8S    9
#define MTEMPL_CCOEFF_8S         10
#define MTEMPL_CCOEFF_NORMED_8S  11

#define MTEMPL_SQDIFF_32F        12
#define MTEMPL_SQDIFF_NORMED_32F 13
#define MTEMPL_CCORR_32F         14
#define MTEMPL_CCORR_NORMED_32F  15
#define MTEMPL_CCOEFF_32F        16
#define MTEMPL_CCOEFF_NORMED_32F 17


void InitAMatchTemplate( void )
{
    /* Registering test functions */
    trsRegArg( funcs[0], test_desc, atsAlgoClass, match_templ_test, MTEMPL_SQDIFF_8U );
    trsRegArg( funcs[0], test_desc, atsAlgoClass, match_templ_test, MTEMPL_SQDIFF_NORMED_8U );
    trsRegArg( funcs[0], test_desc, atsAlgoClass, match_templ_test, MTEMPL_CCORR_8U );
    trsRegArg( funcs[0], test_desc, atsAlgoClass, match_templ_test, MTEMPL_CCORR_NORMED_8U );
    trsRegArg( funcs[0], test_desc, atsAlgoClass, match_templ_test, MTEMPL_CCOEFF_8U );
    trsRegArg( funcs[0], test_desc, atsAlgoClass, match_templ_test, MTEMPL_CCOEFF_NORMED_8U );

    trsRegArg( funcs[0], test_desc, atsAlgoClass, match_templ_test, MTEMPL_SQDIFF_8S );
    trsRegArg( funcs[0], test_desc, atsAlgoClass, match_templ_test, MTEMPL_SQDIFF_NORMED_8S );
    trsRegArg( funcs[0], test_desc, atsAlgoClass, match_templ_test, MTEMPL_CCORR_8S );
    trsRegArg( funcs[0], test_desc, atsAlgoClass, match_templ_test, MTEMPL_CCORR_NORMED_8S );
    trsRegArg( funcs[0], test_desc, atsAlgoClass, match_templ_test, MTEMPL_CCOEFF_8S );
    trsRegArg( funcs[0], test_desc, atsAlgoClass, match_templ_test, MTEMPL_CCOEFF_NORMED_8S );

    trsRegArg( funcs[0], test_desc, atsAlgoClass, match_templ_test, MTEMPL_SQDIFF_32F );
    trsRegArg( funcs[0], test_desc, atsAlgoClass, match_templ_test, MTEMPL_SQDIFF_NORMED_32F );
    trsRegArg( funcs[0], test_desc, atsAlgoClass, match_templ_test, MTEMPL_CCORR_32F );
    trsRegArg( funcs[0], test_desc, atsAlgoClass, match_templ_test, MTEMPL_CCORR_NORMED_32F );
    trsRegArg( funcs[0], test_desc, atsAlgoClass, match_templ_test, MTEMPL_CCOEFF_32F );
    trsRegArg( funcs[0], test_desc, atsAlgoClass, match_templ_test, MTEMPL_CCOEFF_NORMED_32F );
} /* InitAMatchTemplate */


/* End of file. */

