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

#include "_cxts.h"

static const int default_test_case_count = 500;
static const int default_max_log_array_size = 9;

CvArrTest::CvArrTest( const char* _test_name, const char* _test_funcs, const char* _test_descr ) :
    CvTest( _test_name, _test_funcs, _test_descr )
{
    test_case_count = default_test_case_count;

    iplimage_allowed = true;
    cvmat_allowed = true;
    optional_mask = true;
    min_log_array_size = 0;
    max_log_array_size = default_max_log_array_size;
    element_wise_relative_error = true;

    max_arr = MAX_ARR;
    test_array = new CvTestPtrVec[max_arr];
    max_hdr = 0;
    hdr = 0;
}


CvArrTest::~CvArrTest()
{
    clear();
    delete[] test_array;
    test_array = 0;
}


int CvArrTest::write_default_params( CvFileStorage* fs )
{
    write_param( fs, "test_case_count", test_case_count );
    write_param( fs, "min_log_array_size", min_log_array_size );
    write_param( fs, "max_log_array_size", max_log_array_size );
    return 0;
}


void CvArrTest::clear()
{
    if( test_array )
    {
        int i, j, n;

        for( i = 0; i < max_arr; i++ )
        {
            n = test_array[i].size();
            for( j = 0; j < n; j++ )
                cvRelease( &test_array[i][j] );
        }
    }
    delete[] hdr;
    hdr = 0;
    max_hdr = 0;
}


int CvArrTest::read_params( CvFileStorage* fs )
{
    min_log_array_size = cvReadInt( find_param( fs, "min_log_array_size" ), min_log_array_size );
    max_log_array_size = cvReadInt( find_param( fs, "max_log_array_size" ), max_log_array_size );
    test_case_count = cvReadInt( find_param( fs, "test_case_count" ), test_case_count );

    min_log_array_size = cvTsClipInt( min_log_array_size, 0, 20 );
    max_log_array_size = cvTsClipInt( max_log_array_size, min_log_array_size, 20 );
    test_case_count = cvTsClipInt( test_case_count, 0, 100000 );

    return 0;
}


int CvArrTest::get_test_case_count()
{
    return test_case_count;
}

void CvArrTest::get_test_array_types_and_sizes( int /*test_case_idx*/, CvSize** sizes, int** types )
{
    CvRNG* rng = ts->get_rng();
    CvSize size;
    double val;
    int i, j;

    val = cvRandReal(rng) * (max_log_array_size - min_log_array_size) + min_log_array_size;
    size.width = cvRound( exp(val*CV_LOG2) );
    val = cvRandReal(rng) * (max_log_array_size - min_log_array_size) + min_log_array_size;
    size.height = cvRound( exp(val*CV_LOG2) );

    for( i = 0; i < max_arr; i++ )
    {
        int count = test_array[i].size();
        for( j = 0; j < count; j++ )
        {
            sizes[i][j] = size;
            if( i == MASK )
                types[i][j] = CV_8UC1;
        }
    }
}


static const int icvTsTypeToDepth[] =
{
    IPL_DEPTH_8U, IPL_DEPTH_8S, IPL_DEPTH_16U, IPL_DEPTH_16S,
    IPL_DEPTH_32S, IPL_DEPTH_32F, IPL_DEPTH_64F
};


int CvArrTest::prepare_test_case( int test_case_idx )
{
    CvSize** sizes = (CvSize**)malloc( max_arr*sizeof(sizes[0]) );
    int** types = (int**)malloc( max_arr*sizeof(types[0]) );
    int i, j, total = 0;
    CvRNG* rng = ts->get_rng();

    CV_FUNCNAME( "CvArrTest::prepare_test_case" );

    __BEGIN__;

    for( i = 0; i < max_arr; i++ )
    {
        int count = test_array[i].size();
        count = MAX(count, 1);
        sizes[i] = (CvSize*)malloc( count*sizeof(sizes[i][0]) );
        types[i] = (int*)malloc( count*sizeof(types[i][0]) );
    }

    get_test_array_types_and_sizes( test_case_idx, sizes, types );

    for( i = 0; i < max_arr; i++ )
    {
        int count = test_array[i].size();
        total += count;
        for( j = 0; j < count; j++ )
        {
            unsigned t = cvRandInt(rng);
            int is_image = !cvmat_allowed ? 1 : iplimage_allowed ? (t & 1) : 0;
            int create_mask = (t & 6) == 0; // ~ each of 3 tests will use mask
            int use_roi = t & 8;
            CvSize size = sizes[i][j], whole_size = size;
            CvRect roi = {0,0,0,0};

            cvRelease( &test_array[i][j] );
            if( size.width > 0 && size.height > 0 &&
                types[i][j] >= 0 && (i != MASK || create_mask) )
            {
                if( use_roi )
                {
                    roi.width = size.width;
                    roi.height = size.height;
                    whole_size.width += cvRandInt(rng) % 10;
                    whole_size.height += cvRandInt(rng) % 10;
                    if( whole_size.width > size.width )
                        roi.x = cvRandInt(rng) % (whole_size.width - size.width);
                    if( whole_size.height > size.height )
                        roi.y = cvRandInt(rng) % (whole_size.height - size.height);
                }

                if( is_image )
                {
                    CV_CALL( test_array[i][j] = cvCreateImage( whole_size,
                        icvTsTypeToDepth[CV_MAT_DEPTH(types[i][j])],
                        CV_MAT_CN(types[i][j]) ));
                    if( use_roi )
                        cvSetImageROI( (IplImage*)test_array[i][j], roi );
                }
                else
                {
                    CV_CALL( test_array[i][j] = cvCreateMat( whole_size.height,
                                                whole_size.width, types[i][j] ));
                    if( use_roi )
                    {
                        CvMat submat, *mat = (CvMat*)test_array[i][j];
                        cvGetSubRect( test_array[i][j], &submat, roi );
                        submat.refcount = mat->refcount;
                        *mat = submat;
                    }
                }
            }
        }
    }

    if( total > max_hdr )
    {
        delete hdr;
        max_hdr = total;
        hdr = new CvMat[max_hdr];
    }

    total = 0;
    for( i = 0; i < max_arr; i++ )
    {
        int count = test_array[i].size();
        test_mat[i] = count > 0 ? hdr + total : 0;
        for( j = 0; j < count; j++ )
        {
            CvArr* arr = test_array[i][j];
            CvMat* mat = &test_mat[i][j];
            if( !arr )
                memset( mat, 0, sizeof(*mat) );
            else if( CV_IS_MAT( arr ))
            {
                *mat = *(CvMat*)arr;
                mat->refcount = 0;
            }
            else
                cvGetMat( arr, mat, 0, 0 );
            if( mat->data.ptr )
                fill_array( test_case_idx, i, j, mat );
        }
        total += count;
    }

    for( i = 0; i < max_arr; i++ )
    {
        free( sizes[i] );
        free( types[i] );
    }

    __END__;

    free( sizes );
    free( types );
    return 0;
}


void CvArrTest::get_minmax_bounds( int i, int /*j*/, int type, CvScalar* low, CvScalar* high )
{
    double l, u;

    if( i == MASK )
    {
        l = -2;
        u = 2;
    }
    else
    {
        l = cvTsMinVal(type);
        u = cvTsMaxVal(type);
    }

    *low = cvScalarAll(l);
    *high = cvScalarAll(u);
}


void CvArrTest::fill_array( int /*test_case_idx*/, int i, int j, CvMat* arr )
{
    if( i == REF_INPUT_OUTPUT )
        cvTsCopy( &test_mat[INPUT_OUTPUT][j], arr, 0 );
    else if( i == INPUT || i == INPUT_OUTPUT || i == MASK )
    {
        int type = cvGetElemType( arr );
        CvScalar low, high;

        get_minmax_bounds( i, j, type, &low, &high );
        cvTsRandUni( ts->get_rng(), arr, low, high );
    }
}


double CvArrTest::get_success_error_level( int /*test_case_idx*/, int i, int j )
{
    int elem_depth = CV_MAT_DEPTH(cvGetElemType(test_array[i][j]));
    assert( i == OUTPUT || i == INPUT_OUTPUT );
    return elem_depth < CV_32F ? 0 : elem_depth == CV_32F ? FLT_EPSILON*100: DBL_EPSILON*5000;
}


void CvArrTest::prepare_to_validation( int /*test_case_idx*/ )
{
    assert(0);
}


int CvArrTest::validate_test_results( int test_case_idx )
{
    static const char* arr_names[] = { "input", "input/output", "output",
                                       "ref input/output", "ref output",
                                       "temporary", "mask" };
    static const char* type_names[] = { "8u", "8s", "16u", "16s", "32s", "32f", "64f" };
    int i, j;
    prepare_to_validation( test_case_idx );

    for( i = 0; i < 2; i++ )
    {
        int i0 = i == 0 ? OUTPUT : INPUT_OUTPUT;
        int i1 = i == 0 ? REF_OUTPUT : REF_INPUT_OUTPUT;
        int count = test_array[i0].size();

        assert( count == test_array[i1].size() );
        for( j = 0; j < count; j++ )
        {
            double err_level;
            CvPoint idx = {0,0};
            double max_diff = 0;
            int code;
            char msg[100];

            if( !test_array[i1][j] )
                continue;

            err_level = get_success_error_level( test_case_idx, i0, j );
            code = cvTsCmpEps( &test_mat[i0][j], &test_mat[i1][j], &max_diff, err_level, &idx,
                               element_wise_relative_error );

            switch( code )
            {
            case -1:
                sprintf( msg, "Too big difference (=%g)", max_diff );
                code = CvTS::FAIL_BAD_ACCURACY;
                break;
            case -2:
                strcpy( msg, "Invalid output" );
                code = CvTS::FAIL_INVALID_OUTPUT;
                break;
            case -3:
                strcpy( msg, "Invalid output in the reference array" );
                code = CvTS::FAIL_INVALID_OUTPUT;
                break;
            default:
                continue;
            }
            ts->printf( CvTS::LOG, "%s in %s array %d at (%d,%d)\n", msg,
                        arr_names[i0], j, idx.x, idx.y );
            for( i0 = 0; i0 < max_arr; i0++ )
            {
                int count = test_array[i0].size();
                if( i0 == REF_INPUT_OUTPUT || i0 == OUTPUT || i0 == TEMP )
                    continue;
                for( i1 = 0; i1 < count; i1++ )
                {
                    CvArr* arr = test_array[i0][i1];
                    if( arr )
                    {
                        CvSize size = cvGetSize(arr);
                        int type = cvGetElemType(arr);
                        ts->printf( CvTS::LOG, "%s array %d type=%sC%d, size=(%d,%d)\n",
                                    arr_names[i0], i1, type_names[CV_MAT_DEPTH(type)],
                                    CV_MAT_CN(type), size.width, size.height );
                    }
                }
            }
            ts->set_failed_test_info( code );
            return code;
        }
    }

    return 0;
}

/* End of file. */
