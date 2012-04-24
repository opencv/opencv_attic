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

CV_TreesBaseTest :: CV_TreesBaseTest( const char* test_name, const char* test_funcs ) : 
    CvTest( test_name, test_funcs )
{   
}

CV_TreesBaseTest :: ~CV_TreesBaseTest()
{
}

int CV_TreesBaseTest :: read_params( CvFileStorage* fs )
{
    int code = CvTS::OK;
    CvFileNode* run_params = fs ? cvGetFileNodeByName( fs, 0, "run_params" ) : 0;
    data_sets_names = (fs && run_params ) ? cvGetFileNodeByName( fs, run_params, name )->data.seq : 0;
    test_case_count = data_sets_names ? data_sets_names->total : -1;
    return code;
}

void CV_TreesBaseTest :: run( int start_from )
{
    int code = CvTS::OK;
    start_from = 0;
    
    for (int i = 0; i < test_case_count; i++)
    {
        int temp_code = run_test_case( i );
        if (temp_code == CvTS::OK)
            temp_code = validate_test_results( i );
        if (temp_code != CvTS::OK)
            code = temp_code;
    }
    if ( test_case_count <= 0)
    {
        ts->printf( CvTS::LOG, "config file is not determined or not correct" );
        code = CvTS::FAIL_INVALID_TEST_DATA;
    }
    ts->set_failed_test_info( code );
}

int CV_TreesBaseTest :: validate_test_results( int test_case_idx )
{
    float mean, sigma6;
    int iters;
    const char* data_name = ((CvFileNode*)cvGetSeqElem( data_sets_names, test_case_idx ))->data.str.ptr;     

    // read validation params
    CvFileStorage* fs = ts->get_file_storage();
    CvFileNode* fnode = cvGetFileNodeByName( fs, 0, "validation" ), *fnode1 = 0;
    fnode = cvGetFileNodeByName( fs, fnode, name );
    fnode = cvGetFileNodeByName( fs, fnode, data_name );
    fnode = cvGetFileNodeByName( fs, fnode, "result" );
    fnode1 = cvGetFileNodeByName( fs, fnode, "iter_count" );
    if ( !fnode1 )
    {
        ts->printf( CvTS::LOG, "iter_count can not be read from config file" );
        return CvTS::FAIL_INVALID_TEST_DATA;
    }
    iters = fnode1->data.i; 
    if ( iters > 0)
    {
        fnode1 = cvGetFileNodeByName( fs, fnode, "mean" );
        if ( !fnode1 )
        {
            ts->printf( CvTS::LOG, "mean can not be read from config file" );
            return CvTS::FAIL_INVALID_TEST_DATA;
        }
        mean = (float)fnode1->data.f;
        fnode1 = cvGetFileNodeByName( fs, fnode, "sigma" );
        if ( !fnode1 )
        {
            ts->printf( CvTS::LOG, "sigma can not be read from config file" );
            return CvTS::FAIL_INVALID_TEST_DATA;
        }
        sigma6 = 6*(float)fnode1->data.f;
        if ( abs(get_error() - mean) > sigma6 )
        {
            ts->printf( CvTS::LOG, "in test case %d test error is out of range", test_case_idx);
            return CvTS::FAIL_BAD_ACCURACY;
        }
    }
    else
    {
        ts->printf( CvTS::LOG, "validation info is not suitable" );
        return CvTS::FAIL_INVALID_TEST_DATA;
    }
    return CvTS::OK;
}

int CV_TreesBaseTest :: prepare_test_case( int test_case_idx )
{
    int train_sample_count, resp_idx;
    char filepath[1000];
    char filename[1000];
    CvFileNode* fnode = data_sets_names ? (CvFileNode*)cvGetSeqElem( data_sets_names, test_case_idx ) : 0, *fnode1 = 0;    
    const char* data_path = ts->get_data_path(), * data_name;
    const char* var_types = 0;

    assert( fnode ); 
    
    if ( !data_path )
    {
        ts->printf( CvTS::LOG, "data_path is empty" );
        return CvTS::FAIL_INVALID_TEST_DATA;
    }
    
    clear();

    data_name = fnode->data.str.ptr;         

    sprintf( filepath, "%s", data_path );
    sprintf( filename, "%s%s.data", filepath, data_name );

    if ( data.read_csv( filename ) != 0)
    {
        char msg[100];
        sprintf( msg, "file %s can not be read", filename );
        ts->printf( CvTS::LOG, msg );
        return CvTS::FAIL_INVALID_TEST_DATA;
    }

    // read model params
    CvFileStorage* fs = ts->get_file_storage();
    fnode = cvGetFileNodeByName( fs, 0, "validation" );
    fnode = cvGetFileNodeByName( fs, fnode, name );
    fnode = cvGetFileNodeByName( fs, fnode, data_name );
    fnode = cvGetFileNodeByName( fs, fnode, "data_params" );
    fnode1 = cvGetFileNodeByName( fs, fnode, "LS" );
    if ( !fnode1 )
    {
        ts->printf( CvTS::LOG, "LS can not be read from config file" );
        return CvTS::FAIL_INVALID_TEST_DATA;
    }
    train_sample_count =  fnode1->data.i;
    CvTrainTestSplit spl(train_sample_count);
    data.set_train_test_split( &spl );
    fnode1 = cvGetFileNodeByName( fs, fnode, "resp_idx" );
    if ( !fnode1 )
    {
        ts->printf( CvTS::LOG, "resp_idx can not be read from config file" );
        return CvTS::FAIL_INVALID_TEST_DATA;
    }
    resp_idx = fnode1->data.i;
    data.set_response_idx( resp_idx );
    fnode1 = cvGetFileNodeByName( fs, fnode, "types" );
    if ( !fnode1 )
    {
        ts->printf( CvTS::LOG, "types can not be read from config file" );
        return CvTS::FAIL_INVALID_TEST_DATA;
    }
    var_types = fnode1->data.str.ptr;
    data.set_var_types( var_types );

    return CvTS::OK;
}


int CV_TreesBaseTest :: run_test_case( int test_case_idx )
{
    int code = CvTS::OK;
    code = prepare_test_case(test_case_idx);
    
    if (code == CvTS::OK)
    {
//#define GET_STAT
#ifdef GET_STAT
        const char* data_name = ((CvFileNode*)cvGetSeqElem( data_sets_names, test_case_idx ))->data.str.ptr;     
        printf("%s, %s      ", name, data_name);
        const int icount = 100;
        float res[icount];
        for (int k = 0; k < icount; k++)
        {
#endif
            data.mix_train_and_test_idx();
            code = train( test_case_idx );            
#ifdef GET_STAT
            float case_result = get_error();

            res[k] = case_result;
        }
        float mean = 0, sigma = 0;
        for (int k = 0; k < icount; k++)
        {
            mean += res[k];
        }
        mean = mean /icount;
        for (int k = 0; k < icount; k++)
        {
            sigma += (res[k] - mean)*(res[k] - mean);
        }
        sigma = sqrt(sigma/icount);
        printf("%f, %f\n", mean, sigma);
#endif
    }
    return code;
}

/* End of file. */
