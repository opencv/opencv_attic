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

class CV_DTreeTest : public CV_TreesBaseTest
{
public:
    CV_DTreeTest();
    virtual ~CV_DTreeTest();
protected:
    virtual bool train( int test_case_idx );
    virtual float predict(const CvMat* sample, const CvMat* missing_data_mask);
    CvDTree* tree;
};

CV_DTreeTest :: CV_DTreeTest() : CV_TreesBaseTest("dtree", "train and predict")
{
    tree = new CvDTree();
}

CV_DTreeTest :: ~CV_DTreeTest()
{
    delete tree;
}

bool CV_DTreeTest :: train( int test_case_idx )
{
    int MAX_DEPTH, MIN_SAMPLE_COUNT, MAX_CATEGORIES, CV_FOLDS;
    float REG_ACCURACY = 0;
    bool USE_SURROGATE, IS_PRUNED;
    
    const char* data_name = ((CvFileNode*)cvGetSeqElem( data_sets, test_case_idx ))->data.str.ptr;      

    // read validation params
    CvFileStorage* fs = ts->get_file_storage();
    CvFileNode* prms = cvGetFileNodeByName( fs, 0, "validation" );
    prms = cvGetFileNodeByName( fs, prms, name );
    prms = cvGetFileNodeByName( fs, prms, data_name );
    prms = cvGetFileNodeByName( fs, prms, "model_params" );
    MAX_DEPTH = cvGetFileNodeByName( fs, prms, "max_depth" )->data.i;
    MIN_SAMPLE_COUNT = cvGetFileNodeByName( fs, prms, "min_sample_count" )->data.i;
    USE_SURROGATE = (cvGetFileNodeByName( fs, prms, "use_surrogate" )->data.i != 0);
    MAX_CATEGORIES = cvGetFileNodeByName( fs, prms, "max_categories" )->data.i;
    CV_FOLDS = cvGetFileNodeByName( fs, prms, "cv_folds" )->data.i;
    IS_PRUNED = (cvGetFileNodeByName( fs, prms, "is_pruned" )->data.i != 0);

    
    return tree->train(data, CV_ROW_SAMPLE, responses, 0, train_sample_idx, var_type, missing, 
       CvDTreeParams(MAX_DEPTH,
       MIN_SAMPLE_COUNT, 
       REG_ACCURACY,
       USE_SURROGATE,
       MAX_CATEGORIES,
       CV_FOLDS,
       false,
       IS_PRUNED,
       0 )); 
}

float CV_DTreeTest :: predict(const CvMat* sample, const CvMat* missing_data_mask)
{
    assert(tree);
    return (float)tree->predict(sample, missing_data_mask)->value;
}

CV_DTreeTest dtree_test;
/* End of file. */
