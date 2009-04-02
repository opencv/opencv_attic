/*M///////////////////////////////////////////////////////////////////////////////////////
//
//  IMPORTANT: READ BEFORE DOWNLOADING, COPYING, INSTALLING OR USING.
//
//  By downloading, copying, installing or using the software you agree to this license.
//  If you do not agree to this license, do not download, install,
//  copy or use the software.
//
//
//                           License Agreement
//                For Open Source Computer Vision Library
//
// Copyright (C) 2000-2008, Intel Corporation, all rights reserved.
// Copyright (C) 2009, Willow Garage Inc., all rights reserved.
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
//   * The name of the copyright holders may not be used to endorse or promote products
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

#ifndef _ML_TEST_H_
#define _ML_TEST_H_

#if defined _MSC_VER && _MSC_VER >= 1200
#pragma warning( disable: 4710 4711 4514 4996 )
#endif

#include "cxcore.h"
#include "cxmisc.h"
#include "cxts.h"
#include "ml.h"
#include <map>
#include <string>
#include <iostream>


using namespace std;

class CV_TreesBaseTest : public CvTest
{
public:
    CV_TreesBaseTest( const char* test_name, const char* test_funcs );
    virtual ~CV_TreesBaseTest();
protected:
    virtual int read_params( CvFileStorage* fs );
    float str_to_flt_elem( char* token );
    int load_data( const char* filename);
    void mix_train_and_test_idx();
    virtual void run( int start_from );
    int get_var_type(const char* str, int var_count);
    virtual int prepare_test_case( int test_case_idx );
    virtual int run_test_case( int test_case_idx );
    virtual bool train( int test_case_idx ) = 0;
    virtual int validate_test_results( int test_case_idx );
    virtual float predict(const CvMat* sample, const CvMat* missing_data_mask) = 0;
    float get_error(CvMat* sample_idx);    
    virtual void clear();

    CvMat* data;
    CvMat* responses;
    CvMat* missing;
    CvMat* var_type;
    bool is_classifier;

    CvMat* train_sample_idx;
    CvMat* test_sample_idx;
    int* idx; // data of train_sample_idx and test_sample_idx

    CvSeq* data_sets;

    float case_result;

    int total_class_count;
    map<string, int> *class_map;
    FILE* res_file;

    CvRNG rng;
};

#endif /* _ML_TEST_H_ */

/* End of file. */
