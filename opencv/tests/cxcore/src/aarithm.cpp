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

//////////////////////////////////////////////////////////////////////////////////////////
////////////////// tests for arithmetic, logic and statistical functions /////////////////
//////////////////////////////////////////////////////////////////////////////////////////

#include "cxcoretest.h"
#include <float.h>

class CxCore_ArithmTest : public CvArrTest
{
public:
    CxCore_ArithmTest( const char* test_name, const char* test_funcs,
                       int _generate_scalars=0, bool _allow_mask=true, bool _calc_abs=false );
protected:
    void prepare_to_validation( int test_case_idx );
    void get_test_array_types_and_sizes( int test_case_idx, CvSize** sizes, int** types );
    CvScalar alpha, beta, gamma;
    int generate_scalars;
    bool allow_mask, calc_abs;
};


CxCore_ArithmTest::CxCore_ArithmTest( const char* test_name, const char* test_funcs,
                                      int _generate_scalars, bool _allow_mask, bool _calc_abs )
    : CvArrTest( test_name, test_funcs, "" ),
    generate_scalars(_generate_scalars), allow_mask(_allow_mask), calc_abs(_calc_abs)
{
    test_array[INPUT].push(NULL);
    test_array[INPUT].push(NULL);
    if( allow_mask )
    {
        test_array[INPUT_OUTPUT].push(NULL);
        test_array[REF_INPUT_OUTPUT].push(NULL);
        test_array[TEMP].push(NULL);
        test_array[MASK].push(NULL);
    }
    else
    {
        test_array[OUTPUT].push(NULL);
        test_array[REF_OUTPUT].push(NULL);
    }
    alpha = beta = gamma = cvScalarAll(0);
}

void CxCore_ArithmTest::get_test_array_types_and_sizes( int test_case_idx,
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

////////////////////////////// add /////////////////////////////

class CxCore_AddTest : public CxCore_ArithmTest
{
public:
    CxCore_AddTest();
protected:
    void run_func();
};

CxCore_AddTest::CxCore_AddTest()
    : CxCore_ArithmTest( "arithm-add", "cvAdd", 0, true )
{
    alpha = beta = cvScalarAll(1.);
}

void CxCore_AddTest::run_func()
{
    cvAdd( test_array[INPUT][0], test_array[INPUT][1],
        test_array[INPUT_OUTPUT][0], test_array[MASK][0] );
}

CxCore_AddTest add_test;

////////////////////////////// sub /////////////////////////////

class CxCore_SubTest : public CxCore_ArithmTest
{
public:
    CxCore_SubTest();
protected:
    void run_func();
};

CxCore_SubTest::CxCore_SubTest()
    : CxCore_ArithmTest( "arithm-sub", "cvSub", 0, true )
{
    alpha = cvScalarAll(1.);
    beta = cvScalarAll(-1.);
}

void CxCore_SubTest::run_func()
{
    cvSub( test_array[INPUT][0], test_array[INPUT][1],
           test_array[INPUT_OUTPUT][0], test_array[MASK][0] );
}

CxCore_SubTest sub_test;


////////////////////////////// adds /////////////////////////////

class CxCore_AddSTest : public CxCore_ArithmTest
{
public:
    CxCore_AddSTest();
protected:
    void run_func();
};

CxCore_AddSTest::CxCore_AddSTest()
    : CxCore_ArithmTest( "arithm-adds", "cvAddS", 4, true )
{
    test_array[INPUT].pop();
    alpha = cvScalarAll(1.);
}

void CxCore_AddSTest::run_func()
{
    cvAddS( test_array[INPUT][0], gamma,
            test_array[INPUT_OUTPUT][0], test_array[MASK][0] );
}

CxCore_AddSTest adds_test;

////////////////////////////// subrs /////////////////////////////

class CxCore_SubRSTest : public CxCore_ArithmTest
{
public:
    CxCore_SubRSTest();
protected:
    void run_func();
};

CxCore_SubRSTest::CxCore_SubRSTest()
    : CxCore_ArithmTest( "arithm-subrs", "cvSubRS", 4, true )
{
    test_array[INPUT].pop();
    alpha = cvScalarAll(-1.);
}

void CxCore_SubRSTest::run_func()
{
    cvSubRS( test_array[INPUT][0], gamma,
             test_array[INPUT_OUTPUT][0], test_array[MASK][0] );
}

CxCore_SubRSTest subrs_test;

////////////////////////////// addweighted /////////////////////////////

class CxCore_AddWeightedTest : public CxCore_ArithmTest
{
public:
    CxCore_AddWeightedTest();
protected:
    void get_test_array_types_and_sizes( int test_case_idx,
                                          CvSize** sizes, int** types );
    double get_success_error_level( int test_case_idx, int i, int j );
    void run_func();
};

CxCore_AddWeightedTest::CxCore_AddWeightedTest()
    : CxCore_ArithmTest( "arithm-addweighted", "cvAddWeighted", 7, false )
{
}

void CxCore_AddWeightedTest::get_test_array_types_and_sizes( int test_case_idx,
                                                    CvSize** sizes, int** types )
{
    CxCore_ArithmTest::get_test_array_types_and_sizes( test_case_idx, sizes, types );
    alpha = cvScalarAll(alpha.val[0]);
    beta = cvScalarAll(beta.val[0]);
    gamma = cvScalarAll(gamma.val[0]);
}


double CxCore_AddWeightedTest::get_success_error_level( int test_case_idx, int i, int j )
{
    if( CV_MAT_DEPTH(cvGetElemType(test_array[i][j])) <= CV_32S )
    {
        return alpha.val[0] != cvRound(alpha.val[0]) ||
               beta.val[0] != cvRound(beta.val[0]) ||
               gamma.val[0] != cvRound(gamma.val[0]);
    }
    else
        return CvArrTest::get_success_error_level( test_case_idx, i, j );
}


void CxCore_AddWeightedTest::run_func()
{
    cvAddWeighted( test_array[INPUT][0], alpha.val[0],
                   test_array[INPUT][1], beta.val[0],
                   gamma.val[0], test_array[OUTPUT][0] );
}

CxCore_AddWeightedTest addweighted_test;


////////////////////////////// absdiff /////////////////////////////

class CxCore_AbsDiffTest : public CxCore_ArithmTest
{
public:
    CxCore_AbsDiffTest();
protected:
    void run_func();
};

CxCore_AbsDiffTest::CxCore_AbsDiffTest()
    : CxCore_ArithmTest( "arithm-absdiff", "cvAbsDiff", 0, false, true )
{
    alpha = cvScalarAll(1.);
    beta = cvScalarAll(-1.);
}

void CxCore_AbsDiffTest::run_func()
{
    cvAbsDiff( test_array[INPUT][0], test_array[INPUT][1], test_array[OUTPUT][0] );
}

CxCore_AbsDiffTest absdiff_test;

////////////////////////////// absdiffs /////////////////////////////

class CxCore_AbsDiffSTest : public CxCore_ArithmTest
{
public:
    CxCore_AbsDiffSTest();
protected:
    void run_func();
};

CxCore_AbsDiffSTest::CxCore_AbsDiffSTest()
    : CxCore_ArithmTest( "arithm-absdiffs", "cvAbsDiffS", 4, false, true )
{
    alpha = cvScalarAll(-1.);
    test_array[INPUT].pop();
}

void CxCore_AbsDiffSTest::run_func()
{
    cvAbsDiffS( test_array[INPUT][0], test_array[OUTPUT][0], gamma );
}

CxCore_AbsDiffSTest absdiffs_test;


////////////////////////////// mul /////////////////////////////

class CxCore_MulTest : public CxCore_ArithmTest
{
public:
    CxCore_MulTest();
protected:
    void run_func();
    void prepare_to_validation( int /*test_case_idx*/ );
    double get_success_error_level( int test_case_idx, int i, int j );
};

CxCore_MulTest::CxCore_MulTest()
    : CxCore_ArithmTest( "arithm-mul", "cvMul", 4, false, false )
{
}


double CxCore_MulTest::get_success_error_level( int test_case_idx, int i, int j )
{
    if( CV_MAT_DEPTH(cvGetElemType(test_array[i][j])) <= CV_32S )
    {
        return gamma.val[0] != cvRound(gamma.val[0]);
    }
    else
        return CvArrTest::get_success_error_level( test_case_idx, i, j );
}


void CxCore_MulTest::run_func()
{
    cvMul( test_array[INPUT][0], test_array[INPUT][1],
           test_array[OUTPUT][0], gamma.val[0] );
}

void CxCore_MulTest::prepare_to_validation( int /*test_case_idx*/ )
{
    cvTsMul( &test_mat[INPUT][0], &test_mat[INPUT][1],
             cvScalarAll(gamma.val[0]),
             &test_mat[REF_OUTPUT][0] );
}

CxCore_MulTest mul_test;

////////////////////////////// div /////////////////////////////

class CxCore_DivTest : public CxCore_ArithmTest
{
public:
    CxCore_DivTest();
protected:
    void run_func();
    void prepare_to_validation( int /*test_case_idx*/ );
};

CxCore_DivTest::CxCore_DivTest()
    : CxCore_ArithmTest( "arithm-div", "cvDiv", 4, false, false )
{
}

void CxCore_DivTest::run_func()
{
    cvDiv( test_array[INPUT][0], test_array[INPUT][1],
           test_array[OUTPUT][0], gamma.val[0] );
}

void CxCore_DivTest::prepare_to_validation( int /*test_case_idx*/ )
{
    cvTsDiv( &test_mat[INPUT][0], &test_mat[INPUT][1],
             cvScalarAll(gamma.val[0]),
             &test_mat[REF_OUTPUT][0] );
}

CxCore_DivTest div_test;

////////////////////////////// recip /////////////////////////////

class CxCore_RecipTest : public CxCore_ArithmTest
{
public:
    CxCore_RecipTest();
protected:
    void run_func();
    void prepare_to_validation( int /*test_case_idx*/ );
};

CxCore_RecipTest::CxCore_RecipTest()
    : CxCore_ArithmTest( "arithm-recip", "cvDiv", 4, false, false )
{
    test_array[INPUT].pop();
}

void CxCore_RecipTest::run_func()
{
    cvDiv( 0, test_array[INPUT][0],
           test_array[OUTPUT][0], gamma.val[0] );
}

void CxCore_RecipTest::prepare_to_validation( int /*test_case_idx*/ )
{
    cvTsDiv( 0, &test_mat[INPUT][0],
             cvScalarAll(gamma.val[0]),
             &test_mat[REF_OUTPUT][0] );
}

CxCore_RecipTest recip_test;


///////////////// matrix copy/initializing/permutations /////////////////////
                                                   
class CxCore_MemTest : public CxCore_ArithmTest
{
public:
    CxCore_MemTest( const char* test_name, const char* test_funcs,
                    int _generate_scalars=0, bool _allow_mask=true );
protected:
    double get_success_error_level( int /*test_case_idx*/, int /*i*/, int /*j*/ );
};

CxCore_MemTest::CxCore_MemTest( const char* test_name, const char* test_funcs,
                                int _generate_scalars, bool _allow_mask ) :
    CxCore_ArithmTest( test_name, test_funcs, _generate_scalars, _allow_mask, false )
{
}

double CxCore_MemTest::get_success_error_level( int /*test_case_idx*/, int /*i*/, int /*j*/ )
{
    return 0;
}

CxCore_MemTest mem_test( "mem", "" );

///////////////// setidentity /////////////////////

class CxCore_SetIdentityTest : public CxCore_MemTest
{
public:
    CxCore_SetIdentityTest();
protected:
    void run_func();
    void prepare_to_validation( int test_case_idx );
};


CxCore_SetIdentityTest::CxCore_SetIdentityTest() :
    CxCore_MemTest( "mem-setidentity", "cvSetIdentity", 4, false )
{
    test_array[INPUT].clear();
}


void CxCore_SetIdentityTest::run_func()
{
    cvSetIdentity(test_array[OUTPUT][0], gamma);
}


void CxCore_SetIdentityTest::prepare_to_validation( int )
{
    cvTsSetIdentity( &test_mat[REF_OUTPUT][0], gamma );
}

CxCore_SetIdentityTest setidentity_test;


///////////////// SetZero /////////////////////

class CxCore_SetZeroTest : public CxCore_MemTest
{
public:
    CxCore_SetZeroTest();
protected:
    void run_func();
    void prepare_to_validation( int test_case_idx );
};


CxCore_SetZeroTest::CxCore_SetZeroTest() :
    CxCore_MemTest( "mem-setzero", "cvSetZero", 0, false )
{
    test_array[INPUT].clear();
}


void CxCore_SetZeroTest::run_func()
{
    cvSetZero(test_array[OUTPUT][0]);
}


void CxCore_SetZeroTest::prepare_to_validation( int )
{
    cvTsZero( &test_mat[REF_OUTPUT][0] );
}

CxCore_SetZeroTest setzero_test;


///////////////// Set /////////////////////

class CxCore_FillTest : public CxCore_MemTest
{
public:
    CxCore_FillTest();
protected:
    void run_func();
    void prepare_to_validation( int test_case_idx );
};


CxCore_FillTest::CxCore_FillTest() :
    CxCore_MemTest( "mem-fill", "cvSet", 4, true )
{
    test_array[INPUT].clear();
}


void CxCore_FillTest::run_func()
{
    cvSet(test_array[INPUT_OUTPUT][0], gamma, test_array[MASK][0]);
}


void CxCore_FillTest::prepare_to_validation( int )
{
    if( test_array[MASK][0] )
    {
        cvTsAdd( 0, cvScalarAll(0.), 0, cvScalarAll(0.), gamma, &test_mat[TEMP][0], 0 );
        cvTsCopy( &test_mat[TEMP][0], &test_mat[REF_INPUT_OUTPUT][0], &test_mat[MASK][0] );
    }
    else
    {
        cvTsAdd( 0, cvScalarAll(0.), 0, cvScalarAll(0.), gamma, &test_mat[REF_INPUT_OUTPUT][0], 0 );
    }
}

CxCore_FillTest fill_test;


///////////////// Copy /////////////////////

class CxCore_CopyTest : public CxCore_MemTest
{
public:
    CxCore_CopyTest();
protected:
    double get_success_error_level( int test_case_idx, int i, int j );
    void run_func();
    void prepare_to_validation( int test_case_idx );
};


CxCore_CopyTest::CxCore_CopyTest() :
    CxCore_MemTest( "mem-copy", "cvCopy", 0, true )
{
    test_array[INPUT].pop();
}


double CxCore_CopyTest::get_success_error_level( int /*test_case_idx*/, int /*i*/, int /*j*/ )
{
    return 0;
}


void CxCore_CopyTest::run_func()
{
    cvCopy(test_array[INPUT][0], test_array[INPUT_OUTPUT][0], test_array[MASK][0]);
}


void CxCore_CopyTest::prepare_to_validation( int )
{
    cvTsCopy( &test_mat[INPUT][0], &test_mat[REF_INPUT_OUTPUT][0],
              test_array[MASK].size() > 0 && test_array[MASK][0] ? &test_mat[MASK][0] : 0 );
}

CxCore_CopyTest copy_test;

///////////////// Transpose /////////////////////

class CxCore_TransposeTest : public CxCore_MemTest
{
public:
    CxCore_TransposeTest();
protected:
    void get_test_array_types_and_sizes( int test_case_idx, CvSize** sizes, int** types );
    int prepare_test_case( int test_case_idx );
    double get_success_error_level( int test_case_idx, int i, int j );
    void run_func();
    void prepare_to_validation( int test_case_idx );
    bool inplace;
};


CxCore_TransposeTest::CxCore_TransposeTest() :
    CxCore_MemTest( "mem-transpose", "cvTranspose", 0, false ), inplace(false)
{
    test_array[INPUT].pop();
}


double CxCore_TransposeTest::get_success_error_level( int /*test_case_idx*/, int /*i*/, int /*j*/ )
{
    return 0;
}

void CxCore_TransposeTest::get_test_array_types_and_sizes( int test_case_idx, CvSize** sizes, int** types )
{
    int bits = cvTsRandInt(ts->get_rng());
    CxCore_ArithmTest::get_test_array_types_and_sizes( test_case_idx, sizes, types );

    inplace = false;
    if( bits & 1 )
    {
        sizes[INPUT][0].height = sizes[INPUT][0].width;
        inplace = (bits & 2) != 0;
    }

    sizes[OUTPUT][0] = sizes[REF_OUTPUT][0] = cvSize(sizes[INPUT][0].height, sizes[INPUT][0].width );
}


int CxCore_TransposeTest::prepare_test_case( int test_case_idx )
{
    int ok = CxCore_ArithmTest::prepare_test_case( test_case_idx );
    if( inplace )
        cvTsCopy( &test_mat[INPUT][0], &test_mat[OUTPUT][0] );
    return ok;
}

void CxCore_TransposeTest::run_func()
{
    cvTranspose( inplace ? test_array[OUTPUT][0] : test_array[INPUT][0], test_array[OUTPUT][0]);
}


void CxCore_TransposeTest::prepare_to_validation( int )
{
    cvTsTranspose( &test_mat[INPUT][0], &test_mat[REF_OUTPUT][0] );
}

CxCore_TransposeTest transpose_test;


///////////////// Flip /////////////////////

class CxCore_FlipTest : public CxCore_MemTest
{
public:
    CxCore_FlipTest();
protected:
    void get_test_array_types_and_sizes( int test_case_idx, CvSize** sizes, int** types );
    int prepare_test_case( int test_case_idx );
    double get_success_error_level( int test_case_idx, int i, int j );
    void run_func();
    void prepare_to_validation( int test_case_idx );
    int flip_type;
    bool inplace;
};


CxCore_FlipTest::CxCore_FlipTest() :
    CxCore_MemTest( "mem-flip", "cvFlip", 0, false ), flip_type(0), inplace(false)
{
    test_array[INPUT].pop();
}


double CxCore_FlipTest::get_success_error_level( int /*test_case_idx*/, int /*i*/, int /*j*/ )
{
    return 0;
}

void CxCore_FlipTest::get_test_array_types_and_sizes( int test_case_idx, CvSize** sizes, int** types )
{
    int bits = cvTsRandInt(ts->get_rng());
    CxCore_ArithmTest::get_test_array_types_and_sizes( test_case_idx, sizes, types );

    flip_type = (bits & 3) - 2;
    flip_type += flip_type == -2;
    inplace = (bits & 4) != 0;
}


int CxCore_FlipTest::prepare_test_case( int test_case_idx )
{
    int ok = CxCore_ArithmTest::prepare_test_case( test_case_idx );
    if( inplace )
        cvTsCopy( &test_mat[INPUT][0], &test_mat[OUTPUT][0] );
    return ok;
}

void CxCore_FlipTest::run_func()
{
    cvFlip(inplace ? test_array[OUTPUT][0] : test_array[INPUT][0], test_array[OUTPUT][0], flip_type);
}


void CxCore_FlipTest::prepare_to_validation( int )
{
    cvTsFlip( &test_mat[INPUT][0], &test_mat[REF_OUTPUT][0], flip_type );
}

CxCore_FlipTest flip_test;


///////////////// Split /////////////////////

class CxCore_SplitTest : public CxCore_MemTest
{
public:
    CxCore_SplitTest();
protected:
    void get_test_array_types_and_sizes( int test_case_idx, CvSize** sizes, int** types );
    int prepare_test_case( int test_case_idx );
    double get_success_error_level( int test_case_idx, int i, int j );
    void run_func();
    void prepare_to_validation( int test_case_idx );
    bool are_images;
    int coi;
    void* hdrs[4];
};


CxCore_SplitTest::CxCore_SplitTest() :
    CxCore_MemTest( "mem-split", "cvSplit", 0, false ), are_images(false), coi(0)
{
    test_array[INPUT].pop();
    memset( hdrs, 0, sizeof(hdrs) );
}


double CxCore_SplitTest::get_success_error_level( int /*test_case_idx*/, int /*i*/, int /*j*/ )
{
    return 0;
}

void CxCore_SplitTest::get_test_array_types_and_sizes( int test_case_idx, CvSize** sizes, int** types )
{
    int cn, depth;
    CvRNG* rng = ts->get_rng();
    CxCore_ArithmTest::get_test_array_types_and_sizes( test_case_idx, sizes, types );
    cn = cvTsRandInt(rng)%3 + 2;
    depth = CV_MAT_DEPTH(types[INPUT][0]);
    types[INPUT][0] = CV_MAKETYPE(depth, cn);
    types[OUTPUT][0] = types[REF_OUTPUT][0] = depth;

    if( (cvTsRandInt(rng) & 3) != 0 )
    {
        coi = cvTsRandInt(rng) % cn;
    }
    else
    {
        sizes[OUTPUT][0] = sizes[REF_OUTPUT][0] =
            cvSize(sizes[INPUT][0].width,sizes[INPUT][0].height*cn);
        coi = -1;
    }

    are_images = cvTsRandInt(rng)%2 != 0;
}

int CxCore_SplitTest::prepare_test_case( int test_case_idx )
{
    int ok = CxCore_ArithmTest::prepare_test_case( test_case_idx );
    CvMat* input = &test_mat[INPUT][0];
    CvMat* output = &test_mat[OUTPUT][0];
    int depth = CV_MAT_DEPTH(input->type);
    int i, cn = CV_MAT_CN(input->type), y = 0;
    CvSize sz = cvGetSize(input);
    for( i = 0; i < cn; i++ )
    {
        if( coi < 0 || coi == i )
        {
            if( are_images )
                hdrs[i] = cvCreateImageHeader( sz, cvCvToIplDepth(depth), 1 );
            else
                hdrs[i] = cvCreateMatHeader( sz.height, sz.width, depth );
            cvSetData( hdrs[i], output->data.ptr + output->step*y, output->step );
            y += sz.height;
        }
    }

    return ok;
}


void CxCore_SplitTest::run_func()
{
    cvSplit( test_array[INPUT][0], hdrs[0], hdrs[1], hdrs[2], hdrs[3] );
}


void CxCore_SplitTest::prepare_to_validation( int )
{
    CvMat* input = &test_mat[INPUT][0];
    CvMat* output = &test_mat[REF_OUTPUT][0];
    int i, cn = CV_MAT_CN(input->type), y = 0;
    CvSize sz = cvGetSize(input);

    for( i = 0; i < cn; i++ )
    {
        if( coi < 0 || coi == i )
        {
            CvMat stub, *h;
            cvSetData( hdrs[i], output->data.ptr + output->step*y, output->step );
            h = cvGetMat( hdrs[i], &stub );
            cvTsExtract( input, h, i );
            if( are_images )
                cvReleaseImageHeader( (IplImage**)&hdrs[i] );
            else
                cvReleaseMat( (CvMat**)&hdrs[i] );
            y += sz.height;

        }
    }
}

CxCore_SplitTest split_test;


///////////////// Merge /////////////////////

class CxCore_MergeTest : public CxCore_MemTest
{
public:
    CxCore_MergeTest();
protected:
    void get_test_array_types_and_sizes( int test_case_idx, CvSize** sizes, int** types );
    int prepare_test_case( int test_case_idx );
    double get_success_error_level( int test_case_idx, int i, int j );
    void run_func();
    void prepare_to_validation( int test_case_idx );
    bool are_images;
    int coi;
    void* hdrs[4];
};


CxCore_MergeTest::CxCore_MergeTest() :
    CxCore_MemTest( "mem-merge", "cvMerge", 0, false ), are_images(false), coi(0)
{
    test_array[INPUT].pop();
    test_array[OUTPUT].clear();
    test_array[REF_OUTPUT].clear();
    test_array[INPUT_OUTPUT].push(NULL);
    test_array[REF_INPUT_OUTPUT].push(NULL);
    memset( hdrs, 0, sizeof(hdrs) );
}


double CxCore_MergeTest::get_success_error_level( int /*test_case_idx*/, int /*i*/, int /*j*/ )
{
    return 0;
}


void CxCore_MergeTest::get_test_array_types_and_sizes( int test_case_idx, CvSize** sizes, int** types )
{
    int cn, depth;
    CvRNG* rng = ts->get_rng();
    CxCore_ArithmTest::get_test_array_types_and_sizes( test_case_idx, sizes, types );
    cn = cvTsRandInt(rng)%3 + 2;
    depth = CV_MAT_DEPTH(types[INPUT][0]);
    types[INPUT][0] = depth;
    types[INPUT_OUTPUT][0] = types[REF_INPUT_OUTPUT][0] = CV_MAKETYPE(depth, cn);

    if( (cvTsRandInt(rng) & 3) != 0 )
        coi = cvTsRandInt(rng) % cn;
    else
    {
        sizes[INPUT][0] = cvSize(sizes[INPUT_OUTPUT][0].width,sizes[INPUT_OUTPUT][0].height*cn);
        coi = -1;
    }

    are_images = cvTsRandInt(rng)%2 != 0;
}


int CxCore_MergeTest::prepare_test_case( int test_case_idx )
{
    int ok = CxCore_ArithmTest::prepare_test_case( test_case_idx );
    CvMat* input = &test_mat[INPUT][0];
    CvMat* output = &test_mat[INPUT_OUTPUT][0];
    int depth = CV_MAT_DEPTH(input->type);
    int i, cn = CV_MAT_CN(output->type), y = 0;
    CvSize sz = cvGetSize(output);
    for( i = 0; i < cn; i++ )
    {
        assert( hdrs[i] == 0 );
        
        if( coi < 0 || coi == i )
        {
            if( are_images )
                hdrs[i] = cvCreateImageHeader( sz, cvCvToIplDepth(depth), 1 );
            else
                hdrs[i] = cvCreateMatHeader( sz.height, sz.width, depth );
            cvSetData( hdrs[i], input->data.ptr + input->step*y, input->step );
            y += sz.height;
        }
    }

    return ok;
}


void CxCore_MergeTest::run_func()
{
    cvMerge( hdrs[0], hdrs[1], hdrs[2], hdrs[3], test_array[INPUT_OUTPUT][0] );
}


void CxCore_MergeTest::prepare_to_validation( int )
{
    CvMat* input = &test_mat[INPUT][0];
    CvMat* output = &test_mat[REF_INPUT_OUTPUT][0];
    int i, cn = CV_MAT_CN(output->type), y = 0;
    CvSize sz = cvGetSize(output);

    for( i = 0; i < cn; i++ )
    {
        if( coi < 0 || coi == i )
        {
            CvMat stub, *h;
            cvSetData( hdrs[i], input->data.ptr + input->step*y, input->step );
            h = cvGetMat( hdrs[i], &stub, 0 );
            cvTsInsert( h, output, i );
            if( are_images )
                cvReleaseImageHeader( (IplImage**)&hdrs[i] );
            else
                cvReleaseMat( (CvMat**)&hdrs[i] );
            y += sz.height;
        }
    }
}

CxCore_MergeTest merge_test;


////////////////////////////// min/max  /////////////////////////////

class CxCore_MinMaxBaseTest : public CxCore_ArithmTest
{
public:
    CxCore_MinMaxBaseTest( const char* test_name, const char* test_funcs,
                           int _op_type, int _generate_scalars=0 );
protected:
    void get_test_array_types_and_sizes( int test_case_idx, CvSize** sizes, int** types );
    double get_success_error_level( int /*test_case_idx*/, int /*i*/, int /*j*/ );
    void prepare_to_validation( int /*test_case_idx*/ );
    int op_type;
};

CxCore_MinMaxBaseTest::CxCore_MinMaxBaseTest( const char* test_name, const char* test_funcs,
                                              int _op_type, int _generate_scalars )
    : CxCore_ArithmTest( test_name, test_funcs, _generate_scalars, false, false ), op_type(_op_type)
{
    if( _generate_scalars )
        test_array[INPUT].pop();
}

double CxCore_MinMaxBaseTest::get_success_error_level( int /*test_case_idx*/, int /*i*/, int /*j*/ )
{
    return 0;
}

void CxCore_MinMaxBaseTest::get_test_array_types_and_sizes( int test_case_idx, CvSize** sizes, int** types )
{
    int i, j;
    CxCore_ArithmTest::get_test_array_types_and_sizes( test_case_idx, sizes, types );
    for( i = 0; i < max_arr; i++ )
    {
        int count = test_array[i].size();
        for( j = 0; j < count; j++ )
        {
            types[i][j] &= ~CV_MAT_CN_MASK;            
        }
    }
}

void CxCore_MinMaxBaseTest::prepare_to_validation( int /*test_case_idx*/ )
{
    if( !generate_scalars )
        cvTsMinMax( &test_mat[INPUT][0], &test_mat[INPUT][1],
                    &test_mat[REF_OUTPUT][0], op_type );
    else
        cvTsMinMaxS( &test_mat[INPUT][0], gamma.val[0],
                     &test_mat[REF_OUTPUT][0], op_type );
}


class CxCore_MinTest : public CxCore_MinMaxBaseTest
{
public:
    CxCore_MinTest();
protected:
    void run_func();
};


CxCore_MinTest::CxCore_MinTest()
    : CxCore_MinMaxBaseTest( "arithm-min", "cvMin", CV_TS_MIN, 0 )
{
}

void CxCore_MinTest::run_func()
{
    cvMin( test_array[INPUT][0], test_array[INPUT][1], test_array[OUTPUT][0] );
}

CxCore_MinTest min_test;


////////////////////////////// max /////////////////////////////

class CxCore_MaxTest : public CxCore_MinMaxBaseTest
{
public:
    CxCore_MaxTest();
protected:
    void run_func();
};

CxCore_MaxTest::CxCore_MaxTest()
    : CxCore_MinMaxBaseTest( "arithm-max", "cvMax", CV_TS_MAX, 0 )
{
}

void CxCore_MaxTest::run_func()
{
    cvMax( test_array[INPUT][0], test_array[INPUT][1], test_array[OUTPUT][0] );
}

CxCore_MaxTest max_test;


////////////////////////////// mins /////////////////////////////

class CxCore_MinSTest : public CxCore_MinMaxBaseTest
{
public:
    CxCore_MinSTest();
protected:
    void run_func();
};

CxCore_MinSTest::CxCore_MinSTest()
    : CxCore_MinMaxBaseTest( "arithm-mins", "cvMinS", CV_TS_MIN, 4 )
{
}

void CxCore_MinSTest::run_func()
{
    cvMinS( test_array[INPUT][0], gamma.val[0], test_array[OUTPUT][0] );
}

CxCore_MinSTest mins_test;

////////////////////////////// maxs /////////////////////////////

class CxCore_MaxSTest : public CxCore_MinMaxBaseTest
{
public:
    CxCore_MaxSTest();
protected:
    void run_func();
};

CxCore_MaxSTest::CxCore_MaxSTest()
    : CxCore_MinMaxBaseTest( "arithm-maxs", "cvMaxS", CV_TS_MAX, 4 )
{
}

void CxCore_MaxSTest::run_func()
{
    cvMaxS( test_array[INPUT][0], gamma.val[0], test_array[OUTPUT][0] );
}

CxCore_MaxSTest maxs_test;


//////////////////////////////// logic ///////////////////////////////////////

class CxCore_LogicTest : public CxCore_ArithmTest
{
public:
    CxCore_LogicTest( const char* test_name, const char* test_funcs, int _logic_op,
                      int _generate_scalars=0, bool _allow_mask=true );
protected:
    void prepare_to_validation( int test_case_idx );
    int logic_op;
};

CxCore_LogicTest::CxCore_LogicTest( const char* test_name, const char* test_funcs,
                            int _logic_op, int _generate_scalars, bool _allow_mask )
    : CxCore_ArithmTest( test_name, test_funcs, _generate_scalars, _allow_mask, false ),
    logic_op(_logic_op)
{
    if( _generate_scalars )
        test_array[INPUT].pop();
}

void CxCore_LogicTest::prepare_to_validation( int /*test_case_idx*/ )
{
    int ref_output_idx = allow_mask ? REF_INPUT_OUTPUT : REF_OUTPUT;
    int output_idx = allow_mask ? INPUT_OUTPUT : OUTPUT;
    const CvMat* mask = test_array[MASK].size() > 0 && test_array[MASK][0] ? &test_mat[MASK][0] : 0;
    CvMat* dst = mask ? &test_mat[TEMP][0] : &test_mat[ref_output_idx][0];
    int i;
    if( test_array[INPUT].size() > 1 )
    {
        cvTsLogic( &test_mat[INPUT][0], &test_mat[INPUT][1], dst, logic_op );
    }
    else
    {
        cvTsLogicS( &test_mat[INPUT][0], gamma, dst, logic_op );
    }
    if( mask )
        cvTsCopy( dst, &test_mat[ref_output_idx][0], mask );
    
    for( i = 0; i < 2; i++ )
    {
        dst = i == 0 ? &test_mat[ref_output_idx][0] : &test_mat[output_idx][0];

        if( CV_IS_MAT(dst) )
        {
            CvMat* mat = (CvMat*)dst;
            mat->cols *= CV_ELEM_SIZE(mat->type);
            mat->type = (mat->type & ~CV_MAT_TYPE_MASK) | CV_8UC1;
        }
        else
        {
            IplImage* img = (IplImage*)dst;
            int elem_size;
        
            assert( CV_IS_IMAGE(dst) );
            elem_size = ((img->depth & 255)>>3)*img->nChannels;
            img->width *= elem_size;
        
            if( img->roi )
            {
                img->roi->xOffset *= elem_size;
                img->roi->width *= elem_size;
            }
            img->depth = IPL_DEPTH_8U;
            img->nChannels = 1;
        }
    }
}

CxCore_LogicTest logic_test("logic", "", -1);

///////////////////////// and //////////////////////////

class CxCore_AndTest : public CxCore_LogicTest
{
public:
    CxCore_AndTest();
protected:
    void run_func();
};

CxCore_AndTest::CxCore_AndTest()
    : CxCore_LogicTest( "logic-and", "cvAnd", CV_TS_LOGIC_AND )
{
}

void CxCore_AndTest::run_func()
{
    cvAnd( test_array[INPUT][0], test_array[INPUT][1],
           test_array[INPUT_OUTPUT][0], test_array[MASK][0] );
}

CxCore_AndTest and_test;


class CxCore_AndSTest : public CxCore_LogicTest
{
public:
    CxCore_AndSTest();
protected:
    void run_func();
};

CxCore_AndSTest::CxCore_AndSTest()
    : CxCore_LogicTest( "logic-ands", "cvAndS", CV_TS_LOGIC_AND, 4 )
{
}

void CxCore_AndSTest::run_func()
{
    cvAndS( test_array[INPUT][0], gamma,
            test_array[INPUT_OUTPUT][0], test_array[MASK][0] );
}

CxCore_AndSTest ands_test;


///////////////////////// or /////////////////////////

class CxCore_OrTest : public CxCore_LogicTest
{
public:
    CxCore_OrTest();
protected:
    void run_func();
};

CxCore_OrTest::CxCore_OrTest()
    : CxCore_LogicTest( "logic-or", "cvOr", CV_TS_LOGIC_OR )
{
}

void CxCore_OrTest::run_func()
{
    cvOr( test_array[INPUT][0], test_array[INPUT][1],
          test_array[INPUT_OUTPUT][0], test_array[MASK][0] );
}

CxCore_OrTest or_test;


class CxCore_OrSTest : public CxCore_LogicTest
{
public:
    CxCore_OrSTest();
protected:
    void run_func();
};

CxCore_OrSTest::CxCore_OrSTest()
    : CxCore_LogicTest( "logic-ors", "cvOrS", CV_TS_LOGIC_OR, 4 )
{
}

void CxCore_OrSTest::run_func()
{
    cvOrS( test_array[INPUT][0], gamma,
           test_array[INPUT_OUTPUT][0], test_array[MASK][0] );
}

CxCore_OrSTest ors_test;


////////////////////////// xor ////////////////////////////

class CxCore_XorTest : public CxCore_LogicTest
{
public:
    CxCore_XorTest();
protected:
    void run_func();
};

CxCore_XorTest::CxCore_XorTest()
    : CxCore_LogicTest( "logic-xor", "cvXor", CV_TS_LOGIC_XOR )
{
}

void CxCore_XorTest::run_func()
{
    cvXor( test_array[INPUT][0], test_array[INPUT][1],
           test_array[INPUT_OUTPUT][0], test_array[MASK][0] );
}

CxCore_XorTest xor_test;


class CxCore_XorSTest : public CxCore_LogicTest
{
public:
    CxCore_XorSTest();
protected:
    void run_func();
};

CxCore_XorSTest::CxCore_XorSTest()
    : CxCore_LogicTest( "logic-xors", "cvXorS", CV_TS_LOGIC_XOR, 4 )
{
}

void CxCore_XorSTest::run_func()
{
    cvXorS( test_array[INPUT][0], gamma,
            test_array[INPUT_OUTPUT][0], test_array[MASK][0] );
}

CxCore_XorSTest xors_test;


////////////////////////// not ////////////////////////////

class CxCore_NotTest : public CxCore_LogicTest
{
public:
    CxCore_NotTest();
protected:
    void run_func();
};

CxCore_NotTest::CxCore_NotTest()
    : CxCore_LogicTest( "logic-not", "cvNot", CV_TS_LOGIC_NOT, 4, false )
{
}

void CxCore_NotTest::run_func()
{
    cvNot( test_array[INPUT][0],
           test_array[OUTPUT][0] );
}

CxCore_NotTest nots_test;

///////////////////////// cmp //////////////////////////////

class CxCore_CmpBaseTest : public CxCore_ArithmTest
{
public:
    CxCore_CmpBaseTest( const char* test_name, const char* test_funcs,
                        int in_range, int _generate_scalars=0 );
protected:
    double get_success_error_level( int test_case_idx, int i, int j );
    void get_test_array_types_and_sizes( int test_case_idx,
                                         CvSize** sizes, int** types );
    void prepare_to_validation( int test_case_idx );
    int in_range;
    int cmp_op;
};

CxCore_CmpBaseTest::CxCore_CmpBaseTest( const char* test_name, const char* test_funcs,
                                        int _in_range, int _generate_scalars )
    : CxCore_ArithmTest( test_name, test_funcs, _generate_scalars, 0, 0 ), in_range(_in_range)
{
    if( in_range )
    {
        test_array[INPUT].push(NULL);
        test_array[TEMP].push(NULL);
        test_array[TEMP].push(NULL);
        if( !generate_scalars )
            test_array[TEMP].push(NULL);
    }
    if( generate_scalars )
        test_array[INPUT].pop();
    cmp_op = -1;
}

double CxCore_CmpBaseTest::get_success_error_level( int /*test_case_idx*/, int /*i*/, int /*j*/ )
{
    return 0;
}


void CxCore_CmpBaseTest::get_test_array_types_and_sizes( int test_case_idx,
                                               CvSize** sizes, int** types )
{
    int j, count;
    CxCore_ArithmTest::get_test_array_types_and_sizes( test_case_idx, sizes, types );
    types[OUTPUT][0] = types[REF_OUTPUT][0] = CV_8UC1;
    if( !in_range )
    {
        // for cmp tests make all the input arrays single-channel
        count = test_array[INPUT].size();
        for( j = 0; j < count; j++ )
            types[INPUT][j] &= ~CV_MAT_CN_MASK;

        cmp_op = cvTsRandInt(ts->get_rng()) % 6; // == > >= < <= !=
    }
    else
    {
        types[TEMP][0] = CV_8UC1;
        types[TEMP][1] &= ~CV_MAT_CN_MASK;
        if( !generate_scalars )
            types[TEMP][2] &= ~CV_MAT_CN_MASK;
    }
}


void CxCore_CmpBaseTest::prepare_to_validation( int /*test_case_idx*/ )
{
    CvMat* dst = &test_mat[REF_OUTPUT][0];
    if( !in_range )
    {
        if( test_array[INPUT].size() > 1 )
        {
            cvTsCmp( &test_mat[INPUT][0], &test_mat[INPUT][1], dst, cmp_op );
        }
        else
        {
            cvTsCmpS( &test_mat[INPUT][0], gamma.val[0], dst, cmp_op );
        }
    }
    else
    {
        int el_type = CV_MAT_TYPE( test_mat[INPUT][0].type );
        int i, cn = CV_MAT_CN(el_type);
        CvMat* tdst = dst;

        for( i = 0; i < cn*2; i++ )
        {
            int coi = i / 2, is_lower = (i % 2) == 0;
            int cmp_op = is_lower ? CV_CMP_GE : CV_CMP_LT;
            const CvMat* src = &test_mat[INPUT][0];
            const CvMat* lu = generate_scalars ? 0 : &test_mat[INPUT][is_lower?1:2];
            double luS = is_lower ? alpha.val[coi] : gamma.val[coi];
            
            if( cn > 1 )
            {
                cvTsExtract( src, &test_mat[TEMP][1], coi );
                src = &test_mat[TEMP][1];

                if( !generate_scalars )
                {
                    cvTsExtract( lu, &test_mat[TEMP][2], coi );
                    lu = &test_mat[TEMP][2];
                }
            }

            if( !generate_scalars )
                cvTsCmp( src, lu, tdst, cmp_op );
            else
                cvTsCmpS( src, luS, tdst, cmp_op );
            if( i > 0 )
                cvTsLogic( tdst, dst, dst, CV_TS_LOGIC_AND );
            tdst = &test_mat[TEMP][0];
        }
    }
}

CxCore_CmpBaseTest cmpbase_test( "cmp", "", 0 );

class CxCore_CmpTest : public CxCore_CmpBaseTest
{
public:
    CxCore_CmpTest();
protected:
    void run_func();
};

CxCore_CmpTest::CxCore_CmpTest()
    : CxCore_CmpBaseTest( "cmp-cmp", "cvCmp", 0, 0 )
{
}

void CxCore_CmpTest::run_func()
{
    cvCmp( test_array[INPUT][0], test_array[INPUT][1],
           test_array[OUTPUT][0], cmp_op );
}

CxCore_CmpTest cmp_test;


class CxCore_CmpSTest : public CxCore_CmpBaseTest
{
public:
    CxCore_CmpSTest();
protected:
    void run_func();
};

CxCore_CmpSTest::CxCore_CmpSTest()
    : CxCore_CmpBaseTest( "cmp-cmps", "cvCmpS", 0, 4 )
{
}

void CxCore_CmpSTest::run_func()
{
    cvCmpS( test_array[INPUT][0], gamma.val[0],
            test_array[OUTPUT][0], cmp_op );
}

CxCore_CmpSTest cmps_test;


class CxCore_InRangeTest : public CxCore_CmpBaseTest
{
public:
    CxCore_InRangeTest();
protected:
    void run_func();
};

CxCore_InRangeTest::CxCore_InRangeTest()
    : CxCore_CmpBaseTest( "cmp-inrange", "cvInRange", 1, 0 )
{
}

void CxCore_InRangeTest::run_func()
{
    cvInRange( test_array[INPUT][0], test_array[INPUT][1],
               test_array[INPUT][2], test_array[OUTPUT][0] );
}

CxCore_InRangeTest inrange_test;


class CxCore_InRangeSTest : public CxCore_CmpBaseTest
{
public:
    CxCore_InRangeSTest();
protected:
    void run_func();
};

CxCore_InRangeSTest::CxCore_InRangeSTest()
    : CxCore_CmpBaseTest( "cmp-inranges", "cvInRangeS", 1, 5 )
{
}

void CxCore_InRangeSTest::run_func()
{
    cvInRangeS( test_array[INPUT][0], alpha, gamma, test_array[OUTPUT][0] );
}

CxCore_InRangeSTest inranges_test;


/////////////////////////// convertscale[abs] ////////////////////////////////////////

class CxCore_CvtBaseTest : public CxCore_ArithmTest
{
public:
    CxCore_CvtBaseTest( const char* test_name, const char* test_funcs,
                        bool calc_abs );
protected:
    void get_test_array_types_and_sizes( int test_case_idx,
                                         CvSize** sizes, int** types );
    double get_success_error_level( int test_case_idx, int i, int j );
    void prepare_to_validation( int /*test_case_idx*/ );
};


CxCore_CvtBaseTest::CxCore_CvtBaseTest( const char* test_name,
                                        const char* test_funcs,
                                        bool _calc_abs )
    : CxCore_ArithmTest( test_name, test_funcs, 5, false, _calc_abs )
{
    test_array[INPUT].pop();
}

// unlike many other arithmetic functions, conversion operations support 8s type,
// also, for cvCvtScale output array depth may be arbitrary and
// for cvCvtScaleAbs output depth = CV_8U
void CxCore_CvtBaseTest::get_test_array_types_and_sizes( int test_case_idx,
                                                CvSize** sizes, int** types )
{
    CxCore_ArithmTest::get_test_array_types_and_sizes( test_case_idx, sizes, types );
    CvRNG* rng = ts->get_rng();
    int depth = CV_8U, rbits;
    types[INPUT][0] = (types[INPUT][0] & ~CV_MAT_DEPTH_MASK)|
                    (test_case_idx*6/test_case_count);
    if( !calc_abs )
        depth = cvTsRandInt(rng) % 6;
    types[OUTPUT][0] = types[REF_OUTPUT][0] = (types[INPUT][0] & ~CV_MAT_DEPTH_MASK)|depth;

    rbits = cvTsRandInt(rng);
    // check special cases: shift=0 and/or scale=1.
    if( (rbits & 3) == 0 )
        gamma.val[0] = 0;
    if( (rbits & 12) == 0 )
        alpha.val[0] = 0;
}


double CxCore_CvtBaseTest::get_success_error_level( int test_case_idx, int i, int j )
{
    if( CV_MAT_DEPTH(cvGetElemType(test_array[i][j])) <= CV_32S )
    {
        return alpha.val[0] != cvRound(alpha.val[0]) ||
               beta.val[0] != cvRound(beta.val[0]) ||
               gamma.val[0] != cvRound(gamma.val[0]);
    }
    else
        return CvArrTest::get_success_error_level( test_case_idx, i, j );
}


void CxCore_CvtBaseTest::prepare_to_validation( int /*test_case_idx*/ )
{
    cvTsAdd( &test_mat[INPUT][0], cvScalarAll(alpha.val[0]), 0, beta,
             cvScalarAll(gamma.val[0]), &test_mat[REF_OUTPUT][0], calc_abs );
}

CxCore_CvtBaseTest cvt_test( "cvt", "", false );


class CxCore_CvtScaleTest : public CxCore_CvtBaseTest
{
public:
    CxCore_CvtScaleTest();
protected:
    void run_func();
};

CxCore_CvtScaleTest::CxCore_CvtScaleTest()
    : CxCore_CvtBaseTest( "cvt-scale", "cvCvtScale", false )
{
}

void CxCore_CvtScaleTest::run_func()
{
    cvConvertScale( test_array[INPUT][0], test_array[OUTPUT][0],
                    alpha.val[0], gamma.val[0] );
}

CxCore_CvtScaleTest cvtscale_test;


class CxCore_CvtScaleAbsTest : public CxCore_CvtBaseTest
{
public:
    CxCore_CvtScaleAbsTest();
protected:
    void run_func();
};

CxCore_CvtScaleAbsTest::CxCore_CvtScaleAbsTest()
    : CxCore_CvtBaseTest( "cvt-scaleabs", "cvCvtScaleAbs", true )
{
}

void CxCore_CvtScaleAbsTest::run_func()
{
    cvConvertScaleAbs( test_array[INPUT][0], test_array[OUTPUT][0],
                       alpha.val[0], gamma.val[0] );
}

CxCore_CvtScaleAbsTest cvtscaleabs_test;


/////////////////////////////// statistics //////////////////////////////////

class CxCore_StatTest : public CvArrTest
{
public:
    CxCore_StatTest( const char* test_name, const char* test_funcs,
                     int _output_count, bool _single_channel,
                     bool _allow_mask=1, bool _is_binary=0 );
protected:
    void get_test_array_types_and_sizes( int test_case_idx, CvSize** sizes, int** types );
    int  prepare_test_case( int test_case_idx );
    double get_success_error_level( int test_case_idx, int i, int j );

    int coi;
    int output_count;
    bool single_channel;
    bool allow_mask;
    bool is_binary;
};

CxCore_StatTest::CxCore_StatTest( const char* test_name,
                        const char* test_funcs, int _output_count,
                        bool _single_channel, bool _allow_mask, bool _is_binary )
    : CvArrTest( test_name, test_funcs, "" ), output_count(_output_count),
    single_channel(_single_channel), allow_mask(_allow_mask), is_binary(_is_binary)
{
    test_array[INPUT].push(NULL);
    if( is_binary )
        test_array[INPUT].push(NULL);
    if( allow_mask )
        test_array[MASK].push(NULL);
    test_array[OUTPUT].push(NULL);
    test_array[REF_OUTPUT].push(NULL);
    coi = 0;
}

void CxCore_StatTest::get_test_array_types_and_sizes( int test_case_idx,
                                            CvSize** sizes, int** types )
{
    CvRNG* rng = ts->get_rng();
    int depth = test_case_idx*CV_64F/test_case_count;
    int cn = cvTsRandInt(rng) % 4 + 1;
    int j, count = test_array[INPUT].size();
    
    CvArrTest::get_test_array_types_and_sizes( test_case_idx, sizes, types );
    depth += depth == CV_8S;

    for( j = 0; j < count; j++ )
        types[INPUT][j] = CV_MAKETYPE(depth, cn);

    // regardless of the test case, the output is always a fixed-size tuple of numbers
    sizes[OUTPUT][0] = sizes[REF_OUTPUT][0] = cvSize( output_count, 1 );
    types[OUTPUT][0] = types[REF_OUTPUT][0] = CV_64FC1;

    coi = 0;
    cvmat_allowed = true;
    if( cn > 1 && (single_channel || (cvTsRandInt(rng) & 3) == 0) )
    {
        coi = cvTsRandInt(rng) % cn + 1;
        cvmat_allowed = false;
    }
}

int CxCore_StatTest::prepare_test_case( int test_case_idx )
{
    int code = CvArrTest::prepare_test_case( test_case_idx );
    
    if( coi )
    {
        int j, count = test_array[INPUT].size();
        for( j = 0; j < count; j++ )
        {
            IplImage* img = (IplImage*)test_array[INPUT][j];
            if( img )
                cvSetImageCOI( img, coi );
        }
    }

    return code;
}

double CxCore_StatTest::get_success_error_level( int test_case_idx, int i, int j )
{
    int depth = CV_MAT_DEPTH(cvGetElemType(test_array[INPUT][0]));
    if( depth == CV_32F )
        return FLT_EPSILON*1000;
    if( depth == CV_64F )
        return DBL_EPSILON*1000;
    else
        return CvArrTest::get_success_error_level( test_case_idx, i, j );
}

CxCore_StatTest stat_test( "stat", "", 0, 1 );

////////////////// sum /////////////////
class CxCore_SumTest : public CxCore_StatTest
{
public:
    CxCore_SumTest();
protected:
    void run_func();
    void prepare_to_validation( int test_case_idx );
};


CxCore_SumTest::CxCore_SumTest()
    : CxCore_StatTest( "stat-sum", "cvSum", 4 /* CvScalar */, false, false, false )
{
}

void CxCore_SumTest::run_func()
{
    *(CvScalar*)(test_mat[OUTPUT][0].data.db) = cvSum(test_array[INPUT][0]);
}

void CxCore_SumTest::prepare_to_validation( int /*test_case_idx*/ )
{
    CvScalar mean;
    int nonzero = cvTsMeanStdDevNonZero( &test_mat[INPUT][0], 0, &mean, 0, coi );
    mean.val[0] *= nonzero;
    mean.val[1] *= nonzero;
    mean.val[2] *= nonzero;
    mean.val[3] *= nonzero;

    *(CvScalar*)(test_mat[REF_OUTPUT][0].data.db) = mean;
}

CxCore_SumTest sum_test;


////////////////// nonzero /////////////////
class CxCore_NonZeroTest : public CxCore_StatTest
{
public:
    CxCore_NonZeroTest();
protected:
    void run_func();
    void prepare_to_validation( int test_case_idx );
    void get_test_array_types_and_sizes( int test_case_idx,
                                         CvSize** sizes, int** types );
};


CxCore_NonZeroTest::CxCore_NonZeroTest()
    : CxCore_StatTest( "stat-nonzero", "cvCountNonZero", 1 /* int */, true, false, false )
{
    test_array[TEMP].push(NULL);
    test_array[TEMP].push(NULL);
}

void CxCore_NonZeroTest::run_func()
{
    test_mat[OUTPUT][0].data.db[0] = cvCountNonZero(test_array[INPUT][0]);
}

void CxCore_NonZeroTest::get_test_array_types_and_sizes( int test_case_idx,
                                              CvSize** sizes, int** types )
{
    CxCore_StatTest::get_test_array_types_and_sizes( test_case_idx, sizes, types );
    types[TEMP][0] = CV_8UC1;
    if( CV_MAT_CN(types[INPUT][0]) > 1 )
        types[TEMP][1] = types[INPUT][0] & ~CV_MAT_CN_MASK;
    else
        sizes[TEMP][1] = cvSize(0,0);
}


void CxCore_NonZeroTest::prepare_to_validation( int /*test_case_idx*/ )
{
    CvMat* plane = &test_mat[INPUT][0];
    if( CV_MAT_CN(plane->type) > 1 )
    {
        plane = &test_mat[TEMP][1];
        assert( coi > 0 );
        cvTsExtract( &test_mat[INPUT][0], plane, coi-1 );
    }
    cvTsCmpS( plane, 0, &test_mat[TEMP][0], CV_CMP_NE );
    int nonzero = cvTsMeanStdDevNonZero( &test_mat[INPUT][0], &test_mat[TEMP][0], 0, 0, coi );
    test_mat[REF_OUTPUT][0].data.db[0] = nonzero;
}


CxCore_NonZeroTest nonzero_test;


/////////////////// mean //////////////////////
class CxCore_MeanTest : public CxCore_StatTest
{
public:
    CxCore_MeanTest();
protected:
    void run_func();
    void prepare_to_validation( int test_case_idx );
};


CxCore_MeanTest::CxCore_MeanTest()
    : CxCore_StatTest( "stat-mean", "cvAvg", 4 /* CvScalar */, false, true, false )
{
}

void CxCore_MeanTest::run_func()
{
    *(CvScalar*)(test_mat[OUTPUT][0].data.db) =
        cvAvg(test_array[INPUT][0], test_array[MASK][0]);
}

void CxCore_MeanTest::prepare_to_validation( int /*test_case_idx*/ )
{
    CvScalar mean;
    cvTsMeanStdDevNonZero( &test_mat[INPUT][0],
        test_array[MASK][0] ? &test_mat[MASK][0] : 0,
        &mean, 0, coi );
    *(CvScalar*)(test_mat[REF_OUTPUT][0].data.db) = mean;
}

CxCore_MeanTest mean_test;


/////////////////// mean_stddev //////////////////////
class CxCore_MeanStdDevTest : public CxCore_StatTest
{
public:
    CxCore_MeanStdDevTest();
protected:
    void run_func();
    void prepare_to_validation( int test_case_idx );
};


CxCore_MeanStdDevTest::CxCore_MeanStdDevTest()
    : CxCore_StatTest( "stat-mean_stddev", "cvAvgSdv", 8 /* CvScalar x 2 */, false, true, false )
{
}

void CxCore_MeanStdDevTest::run_func()
{
    cvAvgSdv( test_array[INPUT][0],
              &((CvScalar*)(test_mat[OUTPUT][0].data.db))[0],
              &((CvScalar*)(test_mat[OUTPUT][0].data.db))[1],
              test_array[MASK][0] );
}

void CxCore_MeanStdDevTest::prepare_to_validation( int /*test_case_idx*/ )
{
    CvScalar mean, stddev;
    cvTsMeanStdDevNonZero( &test_mat[INPUT][0],
        test_array[MASK][0] ? &test_mat[MASK][0] : 0,
        &mean, &stddev, coi );
    ((CvScalar*)(test_mat[REF_OUTPUT][0].data.db))[0] = mean;
    ((CvScalar*)(test_mat[REF_OUTPUT][0].data.db))[1] = stddev;
}

CxCore_MeanStdDevTest mean_stddev_test;


/////////////////// minmaxloc //////////////////////
class CxCore_MinMaxLocTest : public CxCore_StatTest
{
public:
    CxCore_MinMaxLocTest();
protected:
    void run_func();
    void prepare_to_validation( int test_case_idx );
};


CxCore_MinMaxLocTest::CxCore_MinMaxLocTest()
    : CxCore_StatTest( "stat-minmaxloc", "cvMinMaxLoc", 6 /* double x 2 + CvPoint x 2 */, true, true, false )
{
}

void CxCore_MinMaxLocTest::run_func()
{
    CvPoint minloc = {0,0}, maxloc = {0,0};
    double* output = test_mat[OUTPUT][0].data.db;

    cvMinMaxLoc( test_array[INPUT][0],
        output, output+1, &minloc, &maxloc,
        test_array[MASK][0] );
    output[2] = minloc.x;
    output[3] = minloc.y;
    output[4] = maxloc.x;
    output[5] = maxloc.y;
}

void CxCore_MinMaxLocTest::prepare_to_validation( int /*test_case_idx*/ )
{
    double minval = 0, maxval = 0;
    CvPoint minloc = {0,0}, maxloc = {0,0};
    double* ref_output = test_mat[REF_OUTPUT][0].data.db;
    cvTsMinMaxLoc( &test_mat[INPUT][0], test_array[MASK][0] ?
        &test_mat[MASK][0] : 0, &minval, &maxval, &minloc, &maxloc, coi );
    ref_output[0] = minval;
    ref_output[1] = maxval;
    ref_output[2] = minloc.x;
    ref_output[3] = minloc.y;
    ref_output[4] = maxloc.x;
    ref_output[5] = maxloc.y;
}

CxCore_MinMaxLocTest minmaxloc_test;


/////////////////// norm //////////////////////
class CxCore_NormTest : public CxCore_StatTest
{
public:
    CxCore_NormTest();
protected:
    void run_func();
    void prepare_to_validation( int test_case_idx );
    void get_test_array_types_and_sizes( int test_case_idx,
                                         CvSize** sizes, int** types );
    double get_success_error_level( int test_case_idx, int i, int j );
    int norm_type;
};


CxCore_NormTest::CxCore_NormTest()
    : CxCore_StatTest( "stat-norm", "cvNorm", 1 /* double */, false, true, true )
{
    test_array[TEMP].push(NULL);
}


double CxCore_NormTest::get_success_error_level( int test_case_idx, int i, int j )
{
    int depth = CV_MAT_DEPTH(cvGetElemType(test_array[INPUT][0]));
    if( (depth == CV_16U || depth == CV_16S) && (norm_type&3) != CV_C  )
        return FLT_EPSILON*100;
    else
        return CxCore_StatTest::get_success_error_level( test_case_idx, i, j );
}

void CxCore_NormTest::get_test_array_types_and_sizes( int test_case_idx,
                                               CvSize** sizes, int** types )
{
    int intype;
    int norm_kind;
    CxCore_StatTest::get_test_array_types_and_sizes( test_case_idx, sizes, types );
    norm_type = cvTsRandInt(ts->get_rng()) % 3; // CV_C, CV_L1 or CV_L2
    norm_kind = cvTsRandInt(ts->get_rng()) % 3; // simple, difference or relative difference
    if( norm_kind == 0 )
        sizes[INPUT][1] = cvSize(0,0);
    norm_type = (1 << norm_type) | (norm_kind*8);
    intype = types[INPUT][0];
    if( CV_MAT_CN(intype) > 1 && coi == 0 )
        sizes[MASK][0] = cvSize(0,0);
    sizes[TEMP][0] = cvSize(0,0);
    if( (norm_type & (CV_DIFF|CV_RELATIVE)) && CV_MAT_DEPTH(intype) <= CV_32F )
    {
        sizes[TEMP][0] = sizes[INPUT][0];
        types[TEMP][0] = (intype & ~CV_MAT_DEPTH_MASK)|
            (CV_MAT_DEPTH(intype) < CV_32F ? CV_32S : CV_64F);
    }
}


void CxCore_NormTest::run_func()
{
    /*if( ts->get_current_test_info()->test_case_idx == 268 )
    {
        cvSave( "_a.xml", &test_mat[INPUT][0] );
        cvSave( "_b.xml", &test_mat[INPUT][1] );
        putchar('.');
    }*/
    
    test_mat[OUTPUT][0].data.db[0] = cvNorm( test_array[INPUT][0],
            test_array[INPUT][1], norm_type, test_array[MASK][0] );
}

void CxCore_NormTest::prepare_to_validation( int /*test_case_idx*/ )
{
    double a_norm = 0, b_norm = 0;
    CvMat* a = &test_mat[INPUT][0];
    CvMat* b = &test_mat[INPUT][1];
    CvMat* mask = test_array[MASK][0] ? &test_mat[MASK][0] : 0;
    CvMat* diff = a;

    if( norm_type & (CV_DIFF|CV_RELATIVE) )
    {
        diff = &test_mat[TEMP][0] ? &test_mat[TEMP][0] : a;
        cvTsAdd( a, cvScalarAll(1.), b, cvScalarAll(-1.),
                 cvScalarAll(0.), diff, 0 );
    }
    a_norm = cvTsNorm( diff, mask, norm_type & CV_NORM_MASK, coi );
    if( norm_type & CV_RELATIVE )
    {
        b_norm = cvTsNorm( b, mask, norm_type & CV_NORM_MASK, coi );
        a_norm /= (b_norm + DBL_EPSILON );
    }
    test_mat[REF_OUTPUT][0].data.db[0] = a_norm;
}

CxCore_NormTest norm_test;


///////////////// Trace /////////////////////

class CxCore_TraceTest : public CxCore_StatTest
{
public:
    CxCore_TraceTest();
protected:
    void run_func();
    void prepare_to_validation( int test_case_idx );
};


CxCore_TraceTest::CxCore_TraceTest() :
    CxCore_StatTest( "matrix-trace", "cvTrace", 4, false, false, false )
{
}


void CxCore_TraceTest::run_func()
{
    *((CvScalar*)(test_mat[OUTPUT][0].data.db)) = cvTrace(test_array[INPUT][0]);
}

void CxCore_TraceTest::prepare_to_validation( int )
{
    CvMat* mat = &test_mat[INPUT][0];
    int i, j, count = MIN( mat->rows, mat->cols );
    CvScalar trace = {0,0,0,0};

    for( i = 0; i < count; i++ )
    {
        CvScalar el = cvGet2D( mat, i, i );
        for( j = 0; j < 4; j++ )
            trace.val[j] += el.val[j];
    }

    *((CvScalar*)(test_mat[REF_OUTPUT][0].data.db)) = trace;
}

CxCore_TraceTest trace_test;

// TODO: repeat(?), reshape(?), lut

/* End of file. */
