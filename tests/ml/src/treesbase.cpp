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
    data = responses = missing = var_type = train_sample_idx = test_sample_idx = 0;
    total_class_count = 0;
    class_map = new map<string, int>();
    time_t t;
    time( &t );
    rng = cvRNG( -t );
}

CV_TreesBaseTest :: ~CV_TreesBaseTest()
{
    clear();
    delete class_map;
}

int CV_TreesBaseTest :: read_params( CvFileStorage* fs )
{
    CvFileNode* run_params = cvGetFileNodeByName( fs, 0, "run_params" );
    data_sets = cvGetFileNodeByName( fs, run_params, name )->data.seq;
    test_case_count = data_sets->total;
    return 0;
}

float CV_TreesBaseTest :: str_to_flt_elem(char* token)
{
    
    char* stopstring = NULL;
    float val = (float)strtod( token, &stopstring );
    if ( !strcmp( stopstring, "?" ) ) // missed value
        val = FLT_MAX;
    else
    {
        if ( (*stopstring != 0) && (*stopstring != '\n')) // not number
        {
            int idx = (*class_map)[token];
            if ( idx == 0)
            {
                total_class_count++;
                idx = total_class_count;
                (*class_map)[token] = idx;
            }
            val = (float)idx;
        }
    }
    return val;
}

int CV_TreesBaseTest :: load_data( const char* filename)
{
    const int M = 10000;
    FILE* f = fopen( filename, "rt" );
    CvMemStorage* storage;
    CvSeq* seq;
    char *buf;
    char *ptr;
    float* el_ptr;
    CvSeqReader reader;
    int var_count = 0;

    if( !f ) 
        return -1;

    // read the first line and determine the number of variables
    buf = new char[M];
    if( !fgets( buf, M, f ))
    {
        fclose(f);
        return -1;
    }
    for( ptr = buf; *ptr != '\0'; ptr++ )
        var_count += (*ptr == ',');

    // create temporary memory storage to store the whole database
    el_ptr = new float[var_count+1];
    storage = cvCreateMemStorage();
    seq = cvCreateSeq( 0, sizeof(*seq), (var_count+1)*sizeof(float), storage );

    for(;;)
    {
        char *token = NULL;
        token = strtok(buf, " ,");
        if (!token) return -1;
        for (int i = 0; i < var_count; i++)
        {
            el_ptr[i] = str_to_flt_elem(token);
            token = strtok(NULL, " ,\0");
            if (!token)
                return -1;
        }
        el_ptr[var_count] = str_to_flt_elem(token);
        cvSeqPush( seq, el_ptr );
        if( !fgets( buf, M, f ) || !strchr( buf, ',' ) )
            break;
    }
    fclose(f);

    // allocate the output matrices and copy the base there
    data = cvCreateMat( seq->total, var_count, CV_32F );
    missing = cvCreateMat( seq->total, var_count, CV_8U );
    responses = cvCreateMat( seq->total, 1, CV_32F );

    cvStartReadSeq( seq, &reader );
    for(int i = 0; i < seq->total; i++ )
    {
        const float* sdata = (float*)reader.ptr;
        float* ddata = data->data.fl + var_count*i;
        float* dr = responses->data.fl + i;
        uchar* dm = missing->data.ptr + var_count*i;

        for( int j = 0; j < var_count; j++ )
        {
            ddata[j] = sdata[j];
            dm[j] = ( FLT_MAX - sdata[j] < FLT_EPSILON );
        }
        *dr = sdata[var_count];
        CV_NEXT_SEQ_ELEM( seq->elem_size, reader );
    }
    cvReleaseMemStorage( &storage );
    delete []el_ptr;
    delete []buf;
    return var_count;
}

void CV_TreesBaseTest :: mix_train_and_test_idx()
{
    int n = train_sample_idx->cols + test_sample_idx->cols;
    assert( idx );
    for (int i = 0; i < n; i++)
    {
        int a = cvRandInt( &rng ) % n;
        int b = cvRandInt( &rng ) % n;
        int t;
        CV_SWAP( idx[a], idx[b], t );
    }
}

void CV_TreesBaseTest :: run( int start_from )
{
    int code = CvTS::OK;
    /*char filepath[1000];
    char filename[1000];
    sprintf( filepath, "%s", ts->get_data_path() );
    sprintf( filename, "%s%s_res.txt", filepath, name );
    
    FILE *f = fopen( filename, "wt" ); 
    */start_from = 0;
    
    for (int i = 0; i < test_case_count; i++)
    {
        
        int temp_code = run_test_case( i );
        if (temp_code == CvTS::OK)
            temp_code = validate_test_results( i );
       /* if (f)
        {
            const char* data_name = ((CvFileNode*)cvGetSeqElem( data_sets, i ))->data.str.ptr; 
            fprintf( f, "%s,%f,%d\n", data_name, case_result, temp_code);
        }*/
        if (temp_code != CvTS::OK)
            code = temp_code;
    }
   /* if (f) fclose( f );*/
    ts->set_failed_test_info( code );
}


int CV_TreesBaseTest :: get_var_type(const char* str, int var_count)
{
    // suppose that string str has correct format!
    int step;
    var_type = cvCreateMat( var_count+1, 1, CV_8U);
    step = var_type->step/CV_ELEM_SIZE(var_type->type);
    cvSet( var_type, cvScalarAll(CV_VAR_CATEGORICAL) );
    const char* op = strstr( str, "ord" );    
    if (op)
    {
        if ( strlen(op) == 3)
            cvSet( var_type, cvScalarAll(CV_VAR_ORDERED) );
        else
        {
            char* stopstring = NULL;            
            op += 4; // pass "ord["

            do
            {
                int b1 = (int)strtod( op, &stopstring );
                op = stopstring + 1;
                if ( (stopstring[0] == ',') || (stopstring[0] == ']'))
                    var_type->data.ptr[b1*step] = CV_VAR_ORDERED;
                else 
                {
                    if ( stopstring[0] == '-') 
                    {
                        int b2 = (int)strtod( op, &stopstring);
                        for (int i = b1; i <= b2; i++)
                            var_type->data.ptr[i*step] = CV_VAR_ORDERED;
                    }
                }
            }
            while (stopstring[0] != ']');
        }
    }    

    return 1;    
}

int CV_TreesBaseTest :: validate_test_results( int test_case_idx )
{
    int code = CvTS::OK;
    float mean, sigma6;
    int iters;
    const char* data_name = ((CvFileNode*)cvGetSeqElem( data_sets, test_case_idx ))->data.str.ptr;     

    // read validation params
    CvFileStorage* fs = ts->get_file_storage();
    CvFileNode* prms = cvGetFileNodeByName( fs, 0, "validation" );
    prms = cvGetFileNodeByName( fs, prms, name );
    prms = cvGetFileNodeByName( fs, prms, data_name );
    prms = cvGetFileNodeByName( fs, prms, "result" );
    iters = cvGetFileNodeByName( fs, prms, "iter_count" )->data.i; 
    if (iters)
    {
        mean = (float)cvGetFileNodeByName( fs, prms, "mean" )->data.f;
        sigma6 = 6*(float)cvGetFileNodeByName( fs, prms, "sigma" )->data.f;
        
        if ( abs(case_result - mean) > sigma6 )
            code = CvTS::FAIL_BAD_ARG_CHECK;
    }
    return code;
}

int CV_TreesBaseTest :: prepare_test_case( int test_case_idx )
{
    int var_count, lsize, tsize, n;
    char filepath[1000];
    char filename[1000];
    const char* data_name = ((CvFileNode*)cvGetSeqElem( data_sets, test_case_idx ))->data.str.ptr;     

    clear();

    sprintf( filepath, "%s", ts->get_data_path() );
    sprintf( filename, "%s%s.data", filepath, data_name );

    var_count = load_data(filename);
    if (var_count < 0)
    {
        ts->printf( CvTS::LOG, "%s can not be readed or is not valid", filename );
        return CvTS::FAIL_INVALID_TEST_DATA;
    }

    // read model params
    CvFileStorage* fs = ts->get_file_storage();
    CvFileNode* prms = cvGetFileNodeByName( fs, 0, "validation" );
    prms = cvGetFileNodeByName( fs, prms, name );
    prms = cvGetFileNodeByName( fs, prms, data_name );
    prms = cvGetFileNodeByName( fs, prms, "data_params" );
    lsize = cvGetFileNodeByName( fs, prms, "LS" )->data.i;
    tsize = cvGetFileNodeByName( fs, prms, "TS" )->data.i;
    if ( !get_var_type(cvGetFileNodeByName( fs, prms, "types" )->data.str.ptr, var_count) )
        return CvTS::FAIL_INVALID_TEST_DATA;
    is_classifier = var_type->data.ptr[var_count*var_type->step/CV_ELEM_SIZE(var_type->type)] == CV_VAR_CATEGORICAL;
    
    n = lsize + tsize;
    idx = (int*)cvAlloc( n*sizeof(idx[0]) );
    for (int i = 0; i < n; i++ )
        idx[i] = i;
    train_sample_idx = cvCreateMatHeader( 1, lsize, CV_32SC1 );
    test_sample_idx = cvCreateMatHeader( 1, tsize, CV_32SC1 );
    *train_sample_idx = cvMat( 1, lsize, CV_32SC1, &idx[0] );
    *test_sample_idx = cvMat( 1, tsize, CV_32SC1, &idx[lsize] );
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
        const char* data_name = ((CvFileNode*)cvGetSeqElem( data_sets, test_case_idx ))->data.str.ptr;     
        printf("%s, %s      ", name, data_name);
        const int icount = 100;
        float res[icount];
        for (int k = 0; k < icount; k++)
        {
#endif
            mix_train_and_test_idx();
            train( test_case_idx );
            case_result = get_error( test_sample_idx );
#ifdef GET_STAT
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

float CV_TreesBaseTest :: get_error(CvMat* sample_idx)
{
    float err = 0;
    int* sidx = sample_idx->data.i;
    if ( is_classifier )
    {
        for( int i = 0; i < sample_idx->cols; i++ )
        {
            CvMat sample, miss;
            cvGetRow( data, &sample, sidx[i] ); 
            cvGetRow( missing, &miss, sidx[i] ); 
            float r = predict( &sample, &miss );
            int d = fabs((double)r - responses->data.fl[sidx[i]]) <= FLT_EPSILON ? 0 : 1;
            err += d;
        }
        err = err / (float)sample_idx->cols * 100;
    }
    else
    {
        for( int i = 0; i < sample_idx->cols; i++ )
        {
            CvMat sample, miss;
            cvGetRow( data, &sample, sidx[i] ); 
            cvGetRow( missing, &miss, sidx[i] ); 
            float r = predict( &sample, &miss );
            float d = r - responses->data.fl[sidx[i]];
            err += d*d;
        }
        err = err / (float)sample_idx->cols;    
    }
    return err;
}

void CV_TreesBaseTest :: clear()
{
    if ( !class_map->empty() )
        class_map->clear();

    cvReleaseMat( &data );
    cvReleaseMat( &responses );
    cvReleaseMat( &missing );
    cvReleaseMat( &var_type );

    cvReleaseMat( &train_sample_idx );
    cvReleaseMat( &test_sample_idx );
    idx = 0;
    total_class_count = 0;
}

/* End of file. */
