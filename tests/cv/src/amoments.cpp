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

// image moments
class CV_MomentsTest : public CvArrTest
{
public:
    CV_MomentsTest();

protected:
    
    enum { MOMENT_COUNT = 25 };
    
    int support_testing_modes();
    int prepare_test_case( int test_case_idx );
    void prepare_to_validation( int /*test_case_idx*/ );
    void get_test_array_types_and_sizes( int test_case_idx, CvSize** sizes, int** types );
    void get_minmax_bounds( int i, int j, int type, CvScalar* low, CvScalar* high );
    double get_success_error_level( int test_case_idx, int i, int j );
    void run_func();
    int coi;
    bool is_binary;
};


CV_MomentsTest::CV_MomentsTest()
    : CvArrTest( "moments-raster", "cvMoments, cvGetNormalizedCentralMoment", "" )
{
    test_array[INPUT].push(NULL);
    test_array[OUTPUT].push(NULL);
    test_array[REF_OUTPUT].push(NULL);
    coi = -1;
    is_binary = false;
    //element_wise_relative_error = false;
}


void CV_MomentsTest::get_minmax_bounds( int i, int j, int type, CvScalar* low, CvScalar* high )
{
    CvArrTest::get_minmax_bounds( i, j, type, low, high );
    int depth = CV_MAT_DEPTH(type);
    
    if( depth == CV_16U )
    {
        *low = cvScalarAll(0);
        *high = cvScalarAll(1000);
    }
    else if( depth == CV_16S )
    {
        *low = cvScalarAll(-1000);
        *high = cvScalarAll(1000);
    }
    else if( depth == CV_32F )
    {
        *low = cvScalarAll(-1);
        *high = cvScalarAll(1);
    }
}


void CV_MomentsTest::get_test_array_types_and_sizes( int test_case_idx,
                                                CvSize** sizes, int** types )
{
    CvRNG* rng = ts->get_rng();
    CvArrTest::get_test_array_types_and_sizes( test_case_idx, sizes, types );
    int cn = CV_MAT_CN(types[INPUT][0]);
    int depth = test_case_idx*4/test_case_count;
    depth = depth == 0 ? CV_8U : depth == 1 ? CV_16U : depth == 2 ? CV_16S : CV_32F;
    if( cn == 2 )
        cn = 1;

    types[INPUT][0] = CV_MAKETYPE(depth, cn);
    types[OUTPUT][0] = types[REF_OUTPUT][0] = CV_64FC1;
    sizes[OUTPUT][0] = sizes[REF_OUTPUT][0] = cvSize(MOMENT_COUNT,1);

    is_binary = cvTsRandInt(rng) % 2 != 0;
    coi = 0;
    cvmat_allowed = true;
    if( cn > 1 )
    {
        coi = cvTsRandInt(rng) % cn;
        cvmat_allowed = false;
    }
}


double CV_MomentsTest::get_success_error_level( int /*test_case_idx*/, int /*i*/, int /*j*/ )
{
    int depth = CV_MAT_DEPTH(test_mat[INPUT][0].type);
    return depth != CV_32F ? FLT_EPSILON : FLT_EPSILON*100;
}



int CV_MomentsTest::prepare_test_case( int test_case_idx )
{
    int code = CvArrTest::prepare_test_case( test_case_idx );
    if( code > 0 )
    {
        int cn = CV_MAT_CN(test_mat[INPUT][0].type);
        if( cn > 1 )
            cvSetImageCOI( (IplImage*)test_array[INPUT][0], coi + 1 );
    }

    return code;
}


void CV_MomentsTest::run_func()
{
    CvMoments* m = (CvMoments*)test_mat[OUTPUT][0].data.db;
    double* others = (double*)(m + 1);
    cvMoments( test_array[INPUT][0], m, is_binary );
    others[0] = cvGetNormalizedCentralMoment( m, 2, 0 );
    others[1] = cvGetNormalizedCentralMoment( m, 1, 1 );
    others[2] = cvGetNormalizedCentralMoment( m, 0, 2 );
    others[3] = cvGetNormalizedCentralMoment( m, 3, 0 );
    others[4] = cvGetNormalizedCentralMoment( m, 2, 1 );
    others[5] = cvGetNormalizedCentralMoment( m, 1, 2 );
    others[6] = cvGetNormalizedCentralMoment( m, 0, 3 );
}


void CV_MomentsTest::prepare_to_validation( int /*test_case_idx*/ )
{
    CvMat* src = &test_mat[INPUT][0];
    CvMoments m;
    double* mdata = test_mat[REF_OUTPUT][0].data.db;
    int depth = CV_MAT_DEPTH(src->type);
    int cn = CV_MAT_CN(src->type);
    int i, y, x, cols = src->cols;
    double xc = 0., yc = 0.;
    
    memset( &m, 0, sizeof(m));

    for( y = 0; y < src->rows; y++ )
    {
        double s0 = 0, s1 = 0, s2 = 0, s3 = 0;
        uchar* ptr = src->data.ptr + y*src->step;
        for( x = 0; x < cols; x++ )
        {
            double val;
            if( depth == CV_8U )
                val = ptr[x*cn + coi];
            else if( depth == CV_16U )
                val = ((ushort*)ptr)[x*cn + coi];
            else if( depth == CV_16S )
                val = ((short*)ptr)[x*cn + coi];
            else
                val = ((float*)ptr)[x*cn + coi];

            if( is_binary )
                val = val != 0;

            s0 += val;
            s1 += val*x;
            s2 += val*x*x;
            s3 += ((val*x)*x)*x;
        }

        m.m00 += s0;
        m.m01 += s0*y;
        m.m02 += (s0*y)*y;
        m.m03 += ((s0*y)*y)*y;
        
        m.m10 += s1;
        m.m11 += s1*y;
        m.m12 += (s1*y)*y;

        m.m20 += s2;
        m.m21 += s2*y;

        m.m30 += s3;
    }

    if( m.m00 != 0 )
    {
        xc = m.m10/m.m00, yc = m.m01/m.m00;
        m.inv_sqrt_m00 = 1./sqrt(fabs(m.m00));
    }

    for( y = 0; y < src->rows; y++ )
    {
        double s0 = 0, s1 = 0, s2 = 0, s3 = 0, y1 = y - yc;
        uchar* ptr = src->data.ptr + y*src->step;
        for( x = 0; x < cols; x++ )
        {
            double val, x1 = x - xc;
            if( depth == CV_8U )
                val = ptr[x*cn + coi];
            else if( depth == CV_16U )
                val = ((ushort*)ptr)[x*cn + coi];
            else if( depth == CV_16S )
                val = ((short*)ptr)[x*cn + coi];
            else
                val = ((float*)ptr)[x*cn + coi];

            if( is_binary )
                val = val != 0;

            s0 += val;
            s1 += val*x1;
            s2 += val*x1*x1;
            s3 += ((val*x1)*x1)*x1;
        }

        m.mu02 += s0*y1*y1;
        m.mu03 += ((s0*y1)*y1)*y1;
        
        m.mu11 += s1*y1;
        m.mu12 += (s1*y1)*y1;

        m.mu20 += s2;
        m.mu21 += s2*y1;

        m.mu30 += s3;
    }

    memcpy( mdata, &m, sizeof(m));
    mdata += sizeof(m)/sizeof(m.m00);

    /* calc normalized moments */
    {
        double inv_m00 = m.inv_sqrt_m00*m.inv_sqrt_m00;
        double s2 = inv_m00*inv_m00; /* 1./(m00 ^ (2/2 + 1)) */
        double s3 = s2*m.inv_sqrt_m00; /* 1./(m00 ^ (3/2 + 1)) */

        mdata[0] = m.mu20 * s2;
        mdata[1] = m.mu11 * s2;
        mdata[2] = m.mu02 * s2;

        mdata[3] = m.mu30 * s3;
        mdata[4] = m.mu21 * s3;
        mdata[5] = m.mu12 * s3;
        mdata[6] = m.mu03 * s3;
    }

    {
    double* a = test_mat[REF_OUTPUT][0].data.db;
    double* b = test_mat[OUTPUT][0].data.db;
    for( i = 0; i < MOMENT_COUNT; i++ )
    {
        if( fabs(a[i]) < 1e-3 )
            a[i] = 0;
        if( fabs(b[i]) < 1e-3 )
            b[i] = 0;
    }
    }
}


int CV_MomentsTest::support_testing_modes()
{
    return CvTS::CORRECTNESS_CHECK_MODE; // for now disable the timing test
}


CV_MomentsTest img_moments_test;


// Hu invariants
class CV_HuMomentsTest : public CvArrTest
{
public:
    CV_HuMomentsTest();

protected:
    
    enum { MOMENT_COUNT = 18, HU_MOMENT_COUNT = 7 };
    
    int support_testing_modes();
    int prepare_test_case( int test_case_idx );
    void prepare_to_validation( int /*test_case_idx*/ );
    void get_test_array_types_and_sizes( int test_case_idx, CvSize** sizes, int** types );
    void get_minmax_bounds( int i, int j, int type, CvScalar* low, CvScalar* high );
    double get_success_error_level( int test_case_idx, int i, int j );
    void run_func();
};


CV_HuMomentsTest::CV_HuMomentsTest()
    : CvArrTest( "moments-hu", "cvHuMoments", "" )
{
    test_array[INPUT].push(NULL);
    test_array[OUTPUT].push(NULL);
    test_array[REF_OUTPUT].push(NULL);
}


void CV_HuMomentsTest::get_minmax_bounds( int i, int j, int type, CvScalar* low, CvScalar* high )
{
    CvArrTest::get_minmax_bounds( i, j, type, low, high );
    *low = cvScalarAll(-10000);
    *high = cvScalarAll(10000);
}


void CV_HuMomentsTest::get_test_array_types_and_sizes( int test_case_idx,
                                                CvSize** sizes, int** types )
{
    CvArrTest::get_test_array_types_and_sizes( test_case_idx, sizes, types );
    types[INPUT][0] = types[OUTPUT][0] = types[REF_OUTPUT][0] = CV_64FC1;
    sizes[INPUT][0] = cvSize(MOMENT_COUNT,1);
    sizes[OUTPUT][0] = sizes[REF_OUTPUT][0] = cvSize(HU_MOMENT_COUNT,1);
}


double CV_HuMomentsTest::get_success_error_level( int /*test_case_idx*/, int /*i*/, int /*j*/ )
{
    return FLT_EPSILON;
}



int CV_HuMomentsTest::prepare_test_case( int test_case_idx )
{
    int code = CvArrTest::prepare_test_case( test_case_idx );
    if( code > 0 )
    {
        // ...
    }

    return code;
}


void CV_HuMomentsTest::run_func()
{
    cvGetHuMoments( (CvMoments*)test_mat[INPUT][0].data.db,
                    (CvHuMoments*)test_mat[OUTPUT][0].data.db );
}


void CV_HuMomentsTest::prepare_to_validation( int /*test_case_idx*/ )
{
    CvMoments* m = (CvMoments*)test_mat[INPUT][0].data.db;
    CvHuMoments* hu = (CvHuMoments*)test_mat[REF_OUTPUT][0].data.db;

    double inv_m00 = m->inv_sqrt_m00*m->inv_sqrt_m00;
    double s2 = inv_m00*inv_m00; /* 1./(m00 ^ (2/2 + 1)) */
    double s3 = s2*m->inv_sqrt_m00; /* 1./(m00 ^ (3/2 + 1)) */

    double nu20 = m->mu20 * s2;
    double nu11 = m->mu11 * s2;
    double nu02 = m->mu02 * s2;
                  
    double nu30 = m->mu30 * s3;
    double nu21 = m->mu21 * s3;
    double nu12 = m->mu12 * s3;
    double nu03 = m->mu03 * s3;

    #undef sqr
    #define sqr(a) ((a)*(a))

    hu->hu1 = nu20 + nu02;
    hu->hu2 = sqr(nu20 - nu02) + 4*sqr(nu11);
    hu->hu3 = sqr(nu30 - 3*nu12) + sqr(3*nu21 - nu03);
    hu->hu4 = sqr(nu30 + nu12) + sqr(nu21 + nu03);
    hu->hu5 = (nu30 - 3*nu12)*(nu30 + nu12)*(sqr(nu30 + nu12) - 3*sqr(nu21 + nu03)) +
            (3*nu21 - nu03)*(nu21 + nu03)*(3*sqr(nu30 + nu12) - sqr(nu21 + nu03));
    hu->hu6 = (nu20 - nu02)*(sqr(nu30 + nu12) - sqr(nu21 + nu03)) +
            4*nu11*(nu30 + nu12)*(nu21 + nu03);
    hu->hu7 = (3*nu21 - nu03)*(nu30 + nu12)*(sqr(nu30 + nu12) - 3*sqr(nu21 + nu03)) +
            (3*nu12 - nu30)*(nu21 + nu03)*(3*sqr(nu30 + nu12) - sqr(nu21 + nu03));
}


int CV_HuMomentsTest::support_testing_modes()
{
    return CvTS::CORRECTNESS_CHECK_MODE; // for now disable the timing test
}


CV_HuMomentsTest hu_moments_test;
