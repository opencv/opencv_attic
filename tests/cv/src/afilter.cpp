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

#if 0

class CxCore_FilterBaseTest : public CvFilterBaseTest
{
public:
    CxCore_FilterBaseTest( const char* test_name, const char* test_funcs );
    int write_default_params(CvFileStorage* fs);

protected:
    void prepare_to_validation( int test_case_idx );
    int prepare_test_case( int test_case_idx );
    int read_params( CvFileStorage* fs );
    void get_test_array_types_and_sizes( int test_case_idx, CvSize** sizes, int** types );
    CvSize aperture_size;
    CvPoint anchor;
    int max_aperture_size;
};


CxCore_FilterBaseTest::CxCore_FilterBaseTest( const char* test_name, const char* test_funcs )
    : CvArrTest( test_name, test_funcs, "" ), max_aperture_size(0)
{
    test_array[INPUT].push(NULL);
    test_array[INPUT].push(NULL);
    test_array[OUTPUT].push(NULL);
    test_array[REF_OUTPUT].push(NULL);
    aperture_size = cvSize(0,0);
    anchor = cvPoint(0,0);
}


void CxCore_FilterBaseTest::get_test_array_types_and_sizes( int test_case_idx,
                                               CvSize** sizes, int** types )
{
    CvRNG* rng = ts->get_rng();
    int depth = test_case_idx*CV_64F/test_case_count;
    int cn = cvTsRandInt(rng) % 4 + 1;
    int i, j;
    CvArrTest::get_test_array_types_and_sizes( test_case_idx, sizes, types );
    depth += depth == CV_8S;

    for( i = 0; i < max_arr; i++ )
    {
        int count = test_array[i].size();
        int type = i != MASK ? CV_MAKETYPE(depth, cn) : CV_8UC1;
        for( j = 0; j < count; j++ )
        {
            types[i][j] = type;
        }
    }

    if( generate_scalars )
    {
        double max_val = 10.;
        for( i = 0; i < 4; i++ )
        {
            if( generate_scalars & 1 )
            {
                alpha.val[i] = exp((cvTsRandReal(rng)-0.5)*max_val*2*CV_LOG2);
                alpha.val[i] *= (cvTsRandInt(rng) & 1) ? 1 : -1;
            }
            if( generate_scalars & 2 )
            {
                beta.val[i] = exp((cvTsRandReal(rng)-0.5)*max_val*2*CV_LOG2);
                beta.val[i] *= (cvTsRandInt(rng) & 1) ? 1 : -1;
            }
            if( generate_scalars & 4 )
            {
                gamma.val[i] = exp((cvTsRandReal(rng)-0.5)*max_val*2*CV_LOG2);
                gamma.val[i] *= (cvTsRandInt(rng) & 1) ? 1 : -1;
            }
        }
    }

    if( depth == CV_32F )
    {
        CvMat fl = cvMat( 1, 4, CV_32F, buf );
        CvMat db = cvMat( 1, 4, CV_64F, 0 );

        db.data.db = alpha.val;
        cvTsConvert( &db, &fl );
        cvTsConvert( &fl, &db );

        db.data.db = beta.val;
        cvTsConvert( &db, &fl );
        cvTsConvert( &fl, &db );

        db.data.db = gamma.val;
        cvTsConvert( &db, &fl );
        cvTsConvert( &fl, &db );
    }
}


void CxCore_ArithmTest::prepare_to_validation( int /*test_case_idx*/ )
{
    const CvMat* mask = test_array[MASK].size() > 0 && test_array[MASK][0] ? &test_mat[MASK][0] : 0;
    CvMat* output = test_array[REF_INPUT_OUTPUT].size() > 0 ?
        &test_mat[REF_INPUT_OUTPUT][0] : &test_mat[REF_OUTPUT][0];
    CvMat* temp_dst = mask ? &test_mat[TEMP][0] : output;
    cvTsAdd( &test_mat[INPUT][0], alpha,
             test_array[INPUT].size() > 1 ? &test_mat[INPUT][1] : 0, beta,
             gamma, temp_dst, calc_abs );
    if( mask )
        cvTsCopy( temp_dst, output, mask );
}


CxCore_ArithmTest arithm( "arithm", "" );

#endif
