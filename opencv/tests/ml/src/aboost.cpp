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

#include "mltest.h"

class CV_BoostTest : public CV_TreesBaseTest
{
public:
    CV_BoostTest();
    ~CV_BoostTest();
protected:
    int str_to_boost_type(const char* str);
    virtual bool train( int test_case_idx );
    virtual float predict(const CvMat* sample, const CvMat* missing_data_mask);
    CvBoost* boost;
};

CV_BoostTest :: CV_BoostTest() : CV_TreesBaseTest("boost", "train and predict")
{
    boost = new CvBoost();
}

CV_BoostTest :: ~CV_BoostTest()
{
    delete boost;
}

int CV_BoostTest :: str_to_boost_type(const char* str)
{
    if ( strcmp( str, "DISCRETE" ) == 0 )
        return CvBoost::DISCRETE;
    if ( strcmp( str, "REAL" ) == 0 )
        return CvBoost::REAL;    
    if ( strcmp( str, "LOGIT" ) == 0 )
        return CvBoost::LOGIT;
    if ( strcmp( str, "GENTLE" ) == 0 )
        return CvBoost::GENTLE;
    return CvBoost::REAL;
}

bool CV_BoostTest :: train( int test_case_idx )
{
    int BOOST_TYPE, WEAK_COUNT, MAX_DEPTH;
    float WEIGHT_TRIM_RATE;
    bool USE_SURROGATE;

    const char* data_name = ((CvFileNode*)cvGetSeqElem( data_sets, test_case_idx ))->data.str.ptr;     

    // read validation params
    CvFileStorage* fs = ts->get_file_storage();
    CvFileNode* prms = cvGetFileNodeByName( fs, 0, "validation" );
    prms = cvGetFileNodeByName( fs, prms, name );
    prms = cvGetFileNodeByName( fs, prms, data_name );
    prms = cvGetFileNodeByName( fs, prms, "model_params" );
    BOOST_TYPE = str_to_boost_type(cvGetFileNodeByName( fs, prms, "type" )->data.str.ptr);
    WEAK_COUNT = cvGetFileNodeByName( fs, prms, "weak_count" )->data.i;
    WEIGHT_TRIM_RATE = (float)cvGetFileNodeByName( fs, prms, "weight_trim_rate" )->data.f;
    MAX_DEPTH = cvGetFileNodeByName( fs, prms, "max_depth" )->data.i;
    USE_SURROGATE = (cvGetFileNodeByName( fs, prms, "use_surrogate" )->data.i != 0);

    return   boost->train( data, CV_ROW_SAMPLE, responses, 0, train_sample_idx, var_type, missing,
        CvBoostParams(BOOST_TYPE, WEAK_COUNT, WEIGHT_TRIM_RATE, MAX_DEPTH, USE_SURROGATE, 0) );
}

float CV_BoostTest :: predict(const CvMat* sample, const CvMat* missing_data_mask)
{
    assert(boost);
    return (float)boost->predict(sample, missing_data_mask);
}

CV_BoostTest boost_test;


/* End of file. */
