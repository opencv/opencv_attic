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

class CV_ImgWarpBaseTest : public CvArrTest
{
public:
    CV_ImgWarpBaseTest( const char* test_name, const char* test_funcs, bool warp_matrix );
    int write_default_params(CvFileStorage* fs);

protected:
    int support_testing_modes();
    int read_params( CvFileStorage* fs );
    int prepare_test_case( int test_case_idx );
    void get_test_array_types_and_sizes( int test_case_idx, CvSize** sizes, int** types );
    void get_minmax_bounds( int i, int j, int type, CvScalar* low, CvScalar* high );
    int interpolation;
    int max_interpolation;
    double spatial_scale_zoom, spatial_scale_decimate;
};


CV_ImgWarpBaseTest::CV_ImgWarpBaseTest( const char* test_name, const char* test_funcs, bool warp_matrix )
    : CvArrTest( test_name, test_funcs, "" )
{
    test_array[INPUT].push(NULL);
    if( warp_matrix )
        test_array[INPUT].push(NULL);
    test_array[INPUT_OUTPUT].push(NULL);
    test_array[REF_INPUT_OUTPUT].push(NULL);
    max_interpolation = 4;
    interpolation = 0;
    element_wise_relative_error = false;
    spatial_scale_zoom = 0.01;
    spatial_scale_decimate = 0.005;
}


int CV_ImgWarpBaseTest::read_params( CvFileStorage* fs )
{
    int code = CvArrTest::read_params( fs );
    return code;
}


int CV_ImgWarpBaseTest::write_default_params( CvFileStorage* fs )
{
    int code = CvArrTest::write_default_params( fs );
    return code;
}


void CV_ImgWarpBaseTest::get_minmax_bounds( int i, int j, int type, CvScalar* low, CvScalar* high )
{
    CvArrTest::get_minmax_bounds( i, j, type, low, high );
    if( CV_MAT_DEPTH(type) == CV_32F )
    {
        *low = cvScalarAll(-10.);
        *high = cvScalarAll(10);
    }
}


void CV_ImgWarpBaseTest::get_test_array_types_and_sizes( int test_case_idx,
                                                CvSize** sizes, int** types )
{
    CvRNG* rng = ts->get_rng();
    int depth = test_case_idx*3/test_case_count;
    int cn = cvTsRandInt(rng) % 3 + 1;
    CvArrTest::get_test_array_types_and_sizes( test_case_idx, sizes, types );
    depth = depth == 0 ? CV_8U : depth == 1 ? CV_16U : CV_32F;
    cn += cn == 2;

    types[INPUT][0] = types[INPUT_OUTPUT][0] = types[REF_INPUT_OUTPUT][0] = CV_MAKETYPE(depth, cn);
    if( test_array[INPUT].size() > 1 )
        types[INPUT][1] = cvTsRandInt(rng) & 1 ? CV_32FC1 : CV_64FC1;

    interpolation = cvTsRandInt(rng) % max_interpolation;
}


int CV_ImgWarpBaseTest::support_testing_modes()
{
    return CvTS::CORRECTNESS_CHECK_MODE; // for now disable the timing test
}


int CV_ImgWarpBaseTest::prepare_test_case( int test_case_idx )
{
    int code = CvArrTest::prepare_test_case( test_case_idx );
    CvMat* img = &test_mat[INPUT][0];
    int i, j, cols = img->cols;
    int type = CV_MAT_TYPE(img->type), depth = CV_MAT_DEPTH(type), cn = CV_MAT_CN(type);
    double scale = depth == CV_16U ? 1000. : 255.*0.5;
    double space_scale = spatial_scale_decimate;
    float* buffer;

    if( code <= 0 )
        return code;

    if( test_mat[INPUT_OUTPUT][0].cols >= img->cols &&
        test_mat[INPUT_OUTPUT][0].rows >= img->rows )
        space_scale = spatial_scale_zoom;

    buffer = (float*)malloc( img->cols*cn*sizeof(buffer[0]) );
    
    for( i = 0; i < img->rows; i++ )
    {
        uchar* ptr = img->data.ptr + i*img->step;
        switch( cn )
        {
        case 1:
            for( j = 0; j < cols; j++ )
                buffer[j] = (float)((sin((i+1)*space_scale)*sin((j+1)*space_scale)+1.)*scale);
            break;
        case 2:
            for( j = 0; j < cols; j++ )
            {
                buffer[j*2] = (float)((sin((i+1)*space_scale)+1.)*scale);
                buffer[j*2+1] = (float)((sin((i+j)*space_scale)+1.)*scale);
            }
            break;
        case 3:
            for( j = 0; j < cols; j++ )
            {
                buffer[j*3] = (float)((sin((i+1)*space_scale)+1.)*scale);
                buffer[j*3+1] = (float)((sin(j*space_scale)+1.)*scale);
                buffer[j*3+2] = (float)((sin((i+j)*space_scale)+1.)*scale);
            }
            break;
        case 4:
            for( j = 0; j < cols; j++ )
            {
                buffer[j*4] = (float)((sin((i+1)*space_scale)+1.)*scale);
                buffer[j*4+1] = (float)((sin(j*space_scale)+1.)*scale);
                buffer[j*4+2] = (float)((sin((i+j)*space_scale)+1.)*scale);
                buffer[j*4+3] = (float)((sin((i-j)*space_scale)+1.)*scale);
            }
            break;
        default:
            assert(0);
        }

        switch( depth )
        {
        case CV_8U:
            for( j = 0; j < cols*cn; j++ )
                ptr[j] = (uchar)cvRound(buffer[j]);
            break;
        case CV_16U:
            for( j = 0; j < cols*cn; j++ )
                ((ushort*)ptr)[j] = (ushort)cvRound(buffer[j]);
            break;
        case CV_32F:
            for( j = 0; j < cols*cn; j++ )
                ((float*)ptr)[j] = (float)buffer[j];
            break;
        default:
            assert(0);
        }
    }

    free( buffer );

    return code;
}

CV_ImgWarpBaseTest imgwarp_base( "warp", "", false );


/////////////////////////

class CV_ResizeTest : public CV_ImgWarpBaseTest
{
public:
    CV_ResizeTest();

protected:
    void get_test_array_types_and_sizes( int test_case_idx, CvSize** sizes, int** types );
    void run_func();
    void prepare_to_validation( int /*test_case_idx*/ );
    double get_success_error_level( int test_case_idx, int i, int j );
};


CV_ResizeTest::CV_ResizeTest()
    : CV_ImgWarpBaseTest( "warp-resize", "cvResize", false )
{
}


void CV_ResizeTest::get_test_array_types_and_sizes( int test_case_idx, CvSize** sizes, int** types )
{
    CvRNG* rng = ts->get_rng();
    CV_ImgWarpBaseTest::get_test_array_types_and_sizes( test_case_idx, sizes, types );
    CvSize sz;

    sz.width = (cvTsRandInt(rng) % sizes[INPUT][0].width) + 1;
    sz.height = (cvTsRandInt(rng) % sizes[INPUT][0].height) + 1;

    if( cvTsRandInt(rng) & 1 )
    {
        int xfactor = cvTsRandInt(rng) % 10 + 1;
        int yfactor = cvTsRandInt(rng) % 10 + 1;

        if( cvTsRandInt(rng) & 1 )
            yfactor = xfactor;

        sz.width = sizes[INPUT][0].width / xfactor;
        sz.width = MAX(sz.width,1);
        sz.height = sizes[INPUT][0].height / yfactor;
        sz.height = MAX(sz.height,1);
        sizes[INPUT][0].width = sz.width * xfactor;
        sizes[INPUT][0].height = sz.height * yfactor;
    }

    if( cvTsRandInt(rng) & 1 )
        sizes[INPUT_OUTPUT][0] = sizes[REF_INPUT_OUTPUT][0] = sz;
    else
    {
        sizes[INPUT_OUTPUT][0] = sizes[REF_INPUT_OUTPUT][0] = sizes[INPUT][0];
        sizes[INPUT][0] = sz;
    }
}


void CV_ResizeTest::run_func()
{
    cvResize( test_array[INPUT][0], test_array[INPUT_OUTPUT][0], interpolation );
}


double CV_ResizeTest::get_success_error_level( int /*test_case_idx*/, int /*i*/, int /*j*/ )
{
    int depth = CV_MAT_DEPTH(test_mat[INPUT][0].type);
    return depth == CV_8U ? 16 : depth == CV_16U ? 1024 : 5e-2;
}


void CV_ResizeTest::prepare_to_validation( int /*test_case_idx*/ )
{
    CvMat* src = &test_mat[INPUT][0];
    CvMat* dst = &test_mat[REF_INPUT_OUTPUT][0];
    int i, j, k;
    CvMat* x_idx = cvCreateMat( 1, dst->cols, CV_32SC1 );
    CvMat* y_idx = cvCreateMat( 1, dst->rows, CV_32SC1 );
    int* x_tab = x_idx->data.i;
    int elem_size = CV_ELEM_SIZE(src->type); 
    int drows = dst->rows, dcols = dst->cols;

    if( interpolation == CV_INTER_NN )
    {
        for( j = 0; j < dcols; j++ )
        {
            int t = (j*src->cols*2 + MIN(src->cols,dcols) - 1)/(dcols*2);
            t -= t >= src->cols;
            x_idx->data.i[j] = t*elem_size;
        }

        for( j = 0; j < drows; j++ )
        {
            int t = (j*src->rows*2 + MIN(src->rows,drows) - 1)/(drows*2);
            t -= t >= src->rows;
            y_idx->data.i[j] = t;
        }
    }
    else
    {
        double scale_x = (double)src->cols/dcols;
        double scale_y = (double)src->rows/drows;
        
        for( j = 0; j < dcols; j++ )
        {
            double f = ((j+0.5)*scale_x - 0.5);
            i = cvRound(f);
            x_idx->data.i[j] = (i < 0 ? 0 : i >= src->cols ? src->cols - 1 : i)*elem_size;
        }

        for( j = 0; j < drows; j++ )
        {
            double f = ((j+0.5)*scale_y - 0.5);
            i = cvRound(f);
            y_idx->data.i[j] = i < 0 ? 0 : i >= src->rows ? src->rows - 1 : i;
        }
    }

    for( i = 0; i < drows; i++ )
    {
        uchar* dptr = dst->data.ptr + dst->step*i;
        const uchar* sptr0 = src->data.ptr + src->step*y_idx->data.i[i];
        
        for( j = 0; j < dcols; j++, dptr += elem_size )
        {
            const uchar* sptr = sptr0 + x_tab[j];
            for( k = 0; k < elem_size; k++ )
                dptr[k] = sptr[k];
        }
    }

    cvReleaseMat( &x_idx );
    cvReleaseMat( &y_idx );
}

CV_ResizeTest warp_resize_test;


/////////////////////////

class CV_WarpAffineTest : public CV_ImgWarpBaseTest
{
public:
    CV_WarpAffineTest();

protected:
    void get_test_array_types_and_sizes( int test_case_idx, CvSize** sizes, int** types );
    void run_func();
    int prepare_test_case( int test_case_idx );
    void prepare_to_validation( int /*test_case_idx*/ );
    double get_success_error_level( int test_case_idx, int i, int j );
};


CV_WarpAffineTest::CV_WarpAffineTest()
    : CV_ImgWarpBaseTest( "warp-affine", "cvWarpAffine", true )
{
    //spatial_scale_zoom = spatial_scale_decimate;
    spatial_scale_decimate = spatial_scale_zoom;
}


void CV_WarpAffineTest::get_test_array_types_and_sizes( int test_case_idx, CvSize** sizes, int** types )
{
    CV_ImgWarpBaseTest::get_test_array_types_and_sizes( test_case_idx, sizes, types );
    CvSize sz = sizes[INPUT][0];
    // run for the second time to get output of a different size
    CV_ImgWarpBaseTest::get_test_array_types_and_sizes( test_case_idx, sizes, types );
    sizes[INPUT][0] = sz;
    sizes[INPUT][1] = cvSize( 3, 2 );
}


void CV_WarpAffineTest::run_func()
{
    cvWarpAffine( test_array[INPUT][0], test_array[INPUT_OUTPUT][0], &test_mat[INPUT][1] );
}


double CV_WarpAffineTest::get_success_error_level( int /*test_case_idx*/, int /*i*/, int /*j*/ )
{
    int depth = CV_MAT_DEPTH(test_mat[INPUT][0].type);
    return depth == CV_8U ? 16 : depth == CV_16U ? 1024 : 5e-2;
}


int CV_WarpAffineTest::prepare_test_case( int test_case_idx )
{
    CvRNG* rng = ts->get_rng();
    int code = CV_ImgWarpBaseTest::prepare_test_case( test_case_idx );
    const CvMat* src = &test_mat[INPUT][0];
    const CvMat* dst = &test_mat[INPUT_OUTPUT][0]; 
    CvMat* mat = &test_mat[INPUT][1];
    CvPoint2D32f center;
    double scale, angle;
    double buf[6];
    CvMat tmp = cvMat( 2, 3, mat->type, buf );

    if( code <= 0 )
        return code;

    center.x = (float)((cvTsRandReal(rng)*1.2 - 0.1)*src->cols);
    center.y = (float)((cvTsRandReal(rng)*1.2 - 0.1)*src->rows);
    angle = cvTsRandReal(rng)*360;
    scale = ((double)dst->rows/src->rows + (double)dst->cols/src->cols)*0.5;
    cv2DRotationMatrix( center, angle, scale, mat );
    cvRandArr( rng, &tmp, CV_RAND_NORMAL, cvScalarAll(1.), cvScalarAll(0.01) );
    cvMaxS( &tmp, 0.9, &tmp );
    cvMinS( &tmp, 1.1, &tmp );
    cvMul( &tmp, mat, mat, 1. );

    return code;
}


void CV_WarpAffineTest::prepare_to_validation( int /*test_case_idx*/ )
{
    CvMat* src = &test_mat[INPUT][0];
    CvMat* dst = &test_mat[REF_INPUT_OUTPUT][0];
    CvMat* dst1 = &test_mat[INPUT_OUTPUT][0];
    int x, y, k;
    int elem_size = CV_ELEM_SIZE(src->type);
    int srows = src->rows, scols = src->cols;
    int drows = dst->rows, dcols = dst->cols;
    int depth = CV_MAT_DEPTH(src->type), cn = CV_MAT_CN(src->type);
    int step = src->step / CV_ELEM_SIZE(depth);
    const uchar* sptr0 = src->data.ptr;
    double m[6], tm[6];
    CvMat srcAb = cvMat(2, 3, CV_64FC1, tm ), A, b, invA, invAb, dstAb = cvMat( 2, 3, CV_64FC1, m );

    //cvInvert( &tM, &M, CV_LU );
    // [R|t] -> [R^-1 | -(R^-1)*t]
    cvTsConvert( &test_mat[INPUT][1], &srcAb );
    cvGetCols( &srcAb, &A, 0, 2 );
    cvGetCol( &srcAb, &b, 2 );
    cvGetCols( &dstAb, &invA, 0, 2 );
    cvGetCol( &dstAb, &invAb, 2 );
    cvInvert( &A, &invA, CV_SVD );
    cvGEMM( &invA, &b, -1, 0, 0, &invAb );

    cvTsZero( dst );

    for( y = 0; y < drows; y++ )
    {
        uchar* dptr = dst->data.ptr + dst->step*y;
        
        for( x = 0; x < dcols; x++, dptr += elem_size )
        {
            double xs = x*m[0] + y*m[1] + m[2];
            double ys = x*m[3] + y*m[4] + m[5];
            int ixs = cvFloor(xs);
            int iys = cvFloor(ys);

            if( (unsigned)ixs >= (unsigned)(scols - 1) ||
                (unsigned)iys >= (unsigned)(srows - 1))
            {
                memset( dst1->data.ptr + dst1->step*y + x*elem_size, 0, elem_size );
                continue;
            }

            xs -= ixs;
            ys -= iys;
            
            switch( depth )
            {
            case CV_8U:
                {
                const uchar* sptr = sptr0 + iys*step + ixs*cn;
                for( k = 0; k < cn; k++ )
                {
                    double v00 = sptr[k];
                    double v01 = sptr[cn + k];
                    double v10 = sptr[step + k];
                    double v11 = sptr[step + cn + k];

                    v00 = v00 + xs*(v01 - v00);
                    v10 = v10 + xs*(v11 - v10);
                    v00 = v00 + ys*(v10 - v00);
                    dptr[k] = (uchar)cvRound(v00);
                }
                }
                break;
            case CV_16U:
                {
                const ushort* sptr = (const ushort*)sptr0 + iys*step + ixs*cn;
                for( k = 0; k < cn; k++ )
                {
                    double v00 = sptr[k];
                    double v01 = sptr[cn + k];
                    double v10 = sptr[step + k];
                    double v11 = sptr[step + cn + k];

                    v00 = v00 + xs*(v01 - v00);
                    v10 = v10 + xs*(v11 - v10);
                    v00 = v00 + ys*(v10 - v00);
                    ((ushort*)dptr)[k] = (ushort)cvRound(v00);
                }
                }
                break;
            case CV_32F:
                {
                const float* sptr = (const float*)sptr0 + iys*step + ixs*cn;
                for( k = 0; k < cn; k++ )
                {
                    double v00 = sptr[k];
                    double v01 = sptr[cn + k];
                    double v10 = sptr[step + k];
                    double v11 = sptr[step + cn + k];

                    v00 = v00 + xs*(v01 - v00);
                    v10 = v10 + xs*(v11 - v10);
                    v00 = v00 + ys*(v10 - v00);
                    ((float*)dptr)[k] = (float)v00;
                }
                }
                break;
            default:
                assert(0);
            }
        }
    }

    /*cvNamedWindow( "cv", 0 );
    cvNamedWindow( "ref", 0 );
    cvShowImage( "cv", test_array[INPUT_OUTPUT][0] );
    cvShowImage( "ref", test_array[REF_INPUT_OUTPUT][0] );
    cvWaitKey();*/
}

CV_WarpAffineTest warp_affine_test;



/////////////////////////

class CV_WarpPerspectiveTest : public CV_ImgWarpBaseTest
{
public:
    CV_WarpPerspectiveTest();

protected:
    void get_test_array_types_and_sizes( int test_case_idx, CvSize** sizes, int** types );
    void run_func();
    int prepare_test_case( int test_case_idx );
    void prepare_to_validation( int /*test_case_idx*/ );
    double get_success_error_level( int test_case_idx, int i, int j );
};


CV_WarpPerspectiveTest::CV_WarpPerspectiveTest()
    : CV_ImgWarpBaseTest( "warp-perspective", "cvWarpPerspective", true )
{
    //spatial_scale_zoom = spatial_scale_decimate;
    spatial_scale_decimate = spatial_scale_zoom;
}


void CV_WarpPerspectiveTest::get_test_array_types_and_sizes( int test_case_idx, CvSize** sizes, int** types )
{
    CV_ImgWarpBaseTest::get_test_array_types_and_sizes( test_case_idx, sizes, types );
    CvSize sz = sizes[INPUT][0];
    // run for the second time to get output of a different size
    CV_ImgWarpBaseTest::get_test_array_types_and_sizes( test_case_idx, sizes, types );
    sizes[INPUT][0] = sz;
    sizes[INPUT][1] = cvSize( 3, 3 );
}


void CV_WarpPerspectiveTest::run_func()
{
    cvWarpPerspective( test_array[INPUT][0], test_array[INPUT_OUTPUT][0], &test_mat[INPUT][1] );
}


double CV_WarpPerspectiveTest::get_success_error_level( int /*test_case_idx*/, int /*i*/, int /*j*/ )
{
    int depth = CV_MAT_DEPTH(test_mat[INPUT][0].type);
    return depth == CV_8U ? 16 : depth == CV_16U ? 1024 : 5e-2;
}


int CV_WarpPerspectiveTest::prepare_test_case( int test_case_idx )
{
    CvRNG* rng = ts->get_rng();
    int code = CV_ImgWarpBaseTest::prepare_test_case( test_case_idx );
    const CvMat* src = &test_mat[INPUT][0];
    const CvMat* dst = &test_mat[INPUT_OUTPUT][0]; 
    CvMat* mat = &test_mat[INPUT][1];
    CvPoint2D32f s[4], d[4]; 
    float buf[16];
    CvMat tmp = cvMat( 1, 16, CV_32FC1, buf );
    int i;

    if( code <= 0 )
        return code;

    s[0] = cvPoint2D32f(0,0);
    d[0] = cvPoint2D32f(0,0);
    s[1] = cvPoint2D32f(src->cols-1,0);
    d[1] = cvPoint2D32f(dst->cols-1,0);
    s[2] = cvPoint2D32f(src->cols-1,src->rows-1);
    d[2] = cvPoint2D32f(dst->cols-1,dst->rows-1);
    s[3] = cvPoint2D32f(0,src->rows-1);
    d[3] = cvPoint2D32f(0,dst->rows-1);

    cvRandArr( rng, &tmp, CV_RAND_NORMAL, cvScalarAll(0.), cvScalarAll(0.1) );

    for( i = 0; i < 4; i++ )
    {
        s[i].x += buf[i*4]*src->cols/2;
        s[i].y += buf[i*4+1]*src->rows/2;
        d[i].x += buf[i*4+2]*dst->cols/2;
        d[i].y += buf[i*4+3]*dst->rows/2;
    }

    cvWarpPerspectiveQMatrix( s, d, mat );
    return code;
}


void CV_WarpPerspectiveTest::prepare_to_validation( int /*test_case_idx*/ )
{
    CvMat* src = &test_mat[INPUT][0];
    CvMat* dst = &test_mat[REF_INPUT_OUTPUT][0];
    CvMat* dst1 = &test_mat[INPUT_OUTPUT][0];
    int x, y, k;
    int elem_size = CV_ELEM_SIZE(src->type);
    int srows = src->rows, scols = src->cols;
    int drows = dst->rows, dcols = dst->cols;
    int depth = CV_MAT_DEPTH(src->type), cn = CV_MAT_CN(src->type);
    int step = src->step / CV_ELEM_SIZE(depth);
    const uchar* sptr0 = src->data.ptr;
    double m[9], tm[9];
    CvMat srcM = cvMat(3, 3, CV_64FC1, tm ), dstM = cvMat( 3, 3, CV_64FC1, m );

    //cvInvert( &tM, &M, CV_LU );
    // [R|t] -> [R^-1 | -(R^-1)*t]
    cvTsConvert( &test_mat[INPUT][1], &srcM );
    cvInvert( &srcM, &dstM, CV_SVD );

    cvTsZero( dst );

    for( y = 0; y < drows; y++ )
    {
        uchar* dptr = dst->data.ptr + dst->step*y;
        
        for( x = 0; x < dcols; x++, dptr += elem_size )
        {
            double xs = x*m[0] + y*m[1] + m[2];
            double ys = x*m[3] + y*m[4] + m[5];
            double ds = x*m[6] + y*m[7] + m[8];
            int ixs, iys;
            
            ds = ds ? 1./ds : 0;
            xs *= ds;
            ys *= ds;
            ixs = cvFloor(xs);
            iys = cvFloor(ys);

            if( (unsigned)ixs >= (unsigned)(scols - 1) ||
                (unsigned)iys >= (unsigned)(srows - 1))
            {
                memset( dst1->data.ptr + dst1->step*y + x*elem_size, 0, elem_size );
                continue;
            }

            xs -= ixs;
            ys -= iys;
            
            switch( depth )
            {
            case CV_8U:
                {
                const uchar* sptr = sptr0 + iys*step + ixs*cn;
                for( k = 0; k < cn; k++ )
                {
                    double v00 = sptr[k];
                    double v01 = sptr[cn + k];
                    double v10 = sptr[step + k];
                    double v11 = sptr[step + cn + k];

                    v00 = v00 + xs*(v01 - v00);
                    v10 = v10 + xs*(v11 - v10);
                    v00 = v00 + ys*(v10 - v00);
                    dptr[k] = (uchar)cvRound(v00);
                }
                }
                break;
            case CV_16U:
                {
                const ushort* sptr = (const ushort*)sptr0 + iys*step + ixs*cn;
                for( k = 0; k < cn; k++ )
                {
                    double v00 = sptr[k];
                    double v01 = sptr[cn + k];
                    double v10 = sptr[step + k];
                    double v11 = sptr[step + cn + k];

                    v00 = v00 + xs*(v01 - v00);
                    v10 = v10 + xs*(v11 - v10);
                    v00 = v00 + ys*(v10 - v00);
                    ((ushort*)dptr)[k] = (ushort)cvRound(v00);
                }
                }
                break;
            case CV_32F:
                {
                const float* sptr = (const float*)sptr0 + iys*step + ixs*cn;
                for( k = 0; k < cn; k++ )
                {
                    double v00 = sptr[k];
                    double v01 = sptr[cn + k];
                    double v10 = sptr[step + k];
                    double v11 = sptr[step + cn + k];

                    v00 = v00 + xs*(v01 - v00);
                    v10 = v10 + xs*(v11 - v10);
                    v00 = v00 + ys*(v10 - v00);
                    ((float*)dptr)[k] = (float)v00;
                }
                }
                break;
            default:
                assert(0);
            }
        }
    }

    /*cvNamedWindow( "cv", 0 );
    cvNamedWindow( "ref", 0 );
    cvShowImage( "cv", test_array[INPUT_OUTPUT][0] );
    cvShowImage( "ref", test_array[REF_INPUT_OUTPUT][0] );
    cvWaitKey();*/
}

CV_WarpPerspectiveTest warp_perspective_test;

