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
#include <limits.h>
#include <float.h>

#define DRAW_CONTOURS   0
#define DRAW_CONTOURS2   0

static char* funcName[] = 
{
    "cvStartFindContours, cvFindNextContour, cvEndFindContours, cvDrawContours, cvApproxChains, "
    "cvStartReadChainPoints, cvReadChainPoint",
    "cvCopySeqToArray, cvContourPerimeter",
    "cvApproxPoly, cvDrawContours, icvSeqTreeToSeq",
};

static char* testName[] = 
{
    "Retrieving contours",
    "Comparing perimeter calculation with etalon algorithm",
    "Visual test for approx contours"
};

static int img_size;
static int min_blob_size, max_blob_size;
static int max_contour_size;
static int base_iters;
static int blob_count;
static int init_contour_params = 0;
static const int max_storage_size = 100000;

void read_contour_params( void )
{
    if( !init_contour_params )
    {
        trsiRead( &img_size, "307", "linear image size" );
        trsiRead( &min_blob_size, "1", "Minimal size of blob" );
        trsiRead( &max_blob_size, "50", "Maximal size of blob" );
        trsiRead( &max_contour_size, "10000", "Maximal contour size" );
        trsiRead( &blob_count, "100", "Number of blobs" );
        trsiRead( &base_iters, "300", "Number of iterations" );

        init_contour_params = 1;
    }
}


static void mark_contours_etalon( IplImage* img, int val )
{
    void* _data = 0;
    uchar* data;
    int    i, step = 0;
    CvSize size;
    
    assert( img->depth == IPL_DEPTH_8U && img->nChannels == 1 && (val&1) != 0);

    atsGetImageInfo( img, &_data, &step, &size, 0, 0, 0 );

    data = (uchar*)_data;
    data += step;
    for( i = 1; i < size.height - 1; i++, data += step )
    {
        int j;
        for( j = 1; j < size.width - 1; j++ )
        {
            uchar* t = data + j;
            if( *t == 1 && (t[-step] == 0 || t[-1] == 0 || t[1] == 0 || t[step] == 0))
                *t = (uchar)val;
        }
    }

    cvThreshold( img, img, val - 2, val, CV_THRESH_BINARY );
}


static int contour_retrieving_test( void )
{
    const double  success_error_level = 0;
    const int  depth = IPL_DEPTH_8U;
    const int  min_brightness = 0, max_brightness = 2;
    const int  mark_val = 255, mark_val2 = mark_val >> DRAW_CONTOURS;

    int  seed  = atsGetSeed();

    /* position where the maximum error occured */
    int     merr_iter = 0;

    /* test parameters */
    int     i = 0;
    double  max_err = 0.;
    int     code = TRS_OK;

    IplImage    *src_img, *dst1_img, *dst2_img, *dst3_img;
    /*IplImage    *srcfl_img;*/
    AtsRandState rng_state;
    CvMemStorage* storage;
    CvSize size;

    atsRandInit( &rng_state, 0, 1, seed );

    read_contour_params();

    size = cvSize( img_size, img_size*3/4 );

    src_img  = cvCreateImage( size, depth, 1 );
    dst1_img = cvCreateImage( size, depth, 1 );
    dst2_img = cvCreateImage( size, depth, 1 );
    dst3_img = cvCreateImage( size, depth, 1 );

    storage = cvCreateMemStorage(atsRandPlain32s(&rng_state) % max_storage_size );

    for( i = 0; i < base_iters; i++ )
    {
        double err = 0, err1 = 0;
        int count = 0, count2 = 0, count3 = 0;
        CvChainApproxMethod approxMethod;
        CvContourRetrievalMode retrMode;

        (int&)approxMethod = atsRandPlain32s( &rng_state ) % 4 + 1;
        (int&)retrMode = atsRandPlain32s( &rng_state ) % 4;
        
        CvSeq* contours = 0;
        CvSeq* contours2 = 0;
        CvSeq* contours3 = 0;
        atsGenerateBlobImage( src_img, min_blob_size, max_blob_size, blob_count,
                              min_brightness, max_brightness, &rng_state );
        cvCopy( src_img, dst2_img );
        cvCopy( src_img, dst3_img );
        atsClearBorder( src_img );
        cvCopy( src_img, dst1_img );

        mark_contours_etalon( dst1_img, mark_val );

        count = cvFindContours( dst2_img, storage, &contours, sizeof(CvContour),
                                retrMode, approxMethod );

        cvZero( dst2_img );
        if( contours )
        {
            cvDrawContours( dst2_img, contours, cvScalar(mark_val2), cvScalar(mark_val2), INT_MAX );
        }

#if DRAW_CONTOURS
        {
            named_window( "Test", 0 );
            cvMulS( src_img, src_img, 64 );
            show_iplimage( "Test", src_img );
            wait_key(0);
            iplXor( src_img, dst1_img, dst1_img );
            show_iplimage( "Test", dst1_img );
            wait_key(0);
            destroy_window( "Test" );
        }
#endif

        if( retrMode != CV_RETR_EXTERNAL && approxMethod < CV_CHAIN_APPROX_TC89_L1 ) 
            err = cvNorm( dst1_img, dst2_img, CV_L1 );
        
        count2 = cvFindContours( dst3_img, storage, &contours2, sizeof(CvChain),
                                 retrMode, CV_CHAIN_CODE );

        {
            CvTreeNodeIterator iterator;
            cvInitTreeNodeIterator( &iterator, contours2, INT_MAX );
            CvSeq* seq = contours2;
            
            while( cvNextTreeNode( &iterator ) )
            {
                count3++;
                assert( iterator.node != seq );
                seq = (CvSeq*)iterator.node;
                assert( count3 <= count );
            }

            assert( count3 == count );

            count3 = 0;
        }

        if( count2 != count )
        {
            code = TRS_FAIL;
            goto test_exit;
        }

        if( approxMethod < CV_CHAIN_APPROX_TC89_L1 )
        {
            cvZero( dst3_img );

            if( contours2 )
            {
                cvDrawContours( dst3_img, contours2, cvScalar(mark_val2), cvScalar(mark_val2), INT_MAX );
            }

            err1 = cvNorm( dst2_img, dst3_img, CV_L1 );
            err = MAX( err, err1 );

            if( approxMethod == CV_CHAIN_APPROX_NONE )
            {
                CvTreeNodeIterator iterator1;
                CvTreeNodeIterator iterator2;

                cvInitTreeNodeIterator( &iterator1, contours, INT_MAX );
                cvInitTreeNodeIterator( &iterator2, contours2, INT_MAX );

                for(;;)
                {
                    CvSeq* seq1 = (CvSeq*)cvNextTreeNode( &iterator1 );
                    CvChain* seq2 = (CvChain*)cvNextTreeNode( &iterator2 );
                    CvSeqReader reader;
                    CvChainPtReader reader2;
                    CvChainPtReader reader3;

                    if( !seq1 )
                        break;

                    assert( seq2 );

                    cvStartReadSeq( seq1, &reader );
                    cvStartReadChainPoints( seq2, &reader2 );
                    cvStartReadChainPoints( seq2, &reader3 );

                    for( int j = 0; j < seq1->total; j++ )
                    {
                        CvPoint pt;
                        CvPoint pt2;
                        CvPoint pt3;

                        CV_READ_SEQ_ELEM( pt, reader );
                        CV_READ_CHAIN_POINT( pt2, reader2 );
                        pt3 = cvReadChainPoint( &reader3 );

                        if( pt.x != pt2.x || pt.x != pt3.x ||
                            pt.y != pt2.y || pt.y != pt3.y )
                        {
                            code = TRS_FAIL;
                            goto test_exit;
                        }
                    }
                }
            }
        }

        contours3 = cvApproxChains( contours2, storage, approxMethod, 0, 0, 1 );

        {
            CvTreeNodeIterator iterator1;
            CvTreeNodeIterator iterator2;

            cvInitTreeNodeIterator( &iterator1, contours, INT_MAX );
            cvInitTreeNodeIterator( &iterator2, contours3, INT_MAX );

            for( ;; count3++ )
            {
                CvSeq* seq1 = (CvSeq*)cvNextTreeNode( &iterator1 );
                CvSeq* seq2 = (CvSeq*)cvNextTreeNode( &iterator2 );
                CvSeqReader reader;
                CvSeqReader reader2;

                if( !seq2 )
                    break;

                assert( seq1 );

                cvStartReadSeq( seq1, &reader );
                cvStartReadSeq( seq2, &reader2 );

                for( int j = 0; j < seq1->total; j++ )
                {
                    CvPoint pt;
                    CvPoint pt2;

                    CV_READ_SEQ_ELEM( pt, reader );
                    CV_READ_SEQ_ELEM( pt2, reader2 );

                    if( pt.x != pt2.x || pt.y != pt2.y )
                    {
                        code = TRS_FAIL;
                        goto test_exit;
                    }
                }
            }

            if( count3 != count )
            {
                code = TRS_FAIL;
                goto test_exit;
            }
        }

        if( err > max_err )
        {
            merr_iter = i;
            max_err   = err;
            if( max_err > success_error_level )
                goto test_exit;
        }

        cvClearMemStorage( storage );
    }

test_exit:

    cvReleaseMemStorage( &storage );

    cvReleaseImage( &src_img );
    cvReleaseImage( &dst1_img );
    cvReleaseImage( &dst2_img );
    cvReleaseImage( &dst3_img );

    if( code == TRS_OK )
    {
        trsWrite( ATS_LST, "Max err is %g at iter = %d, seed = %08x",
                           max_err, merr_iter, seed );

        return max_err <= success_error_level ?
            trsResult( TRS_OK, "No errors" ) :
            trsResult( TRS_FAIL, "Bad accuracy" );
    }
    else
    {
        trsWrite( ATS_LST, "Fatal error at iter = %d, seed = %08x", i, seed );
        return trsResult( TRS_FAIL, "Test failed" );
    }
}


int get_slice_length( CvSeq* seq, CvSlice slice )
{
    int total = seq->total;
    int length;

    /*if( slice.start_index == slice.end_index )
        return 0;*/
    if( slice.start_index < 0 )
        slice.start_index += total;
    if( slice.end_index <= 0 )
        slice.end_index += total;
    length = slice.end_index - slice.start_index;
    if( length < 0 )
        length += total;
    else if( length > total )
        length = total;

    return length;
}


double calc_contour_perimeter( CvPoint* array, int count )
{
    int i;
    double s = 0;

    for( i = 0; i < count - 1; i++ )
    {
        int dx = array[i].x - array[i+1].x;
        int dy = array[i].y - array[i+1].y;
        
        s += sqrt((double)dx*dx + dy*dy);
    }

    return s;
}


static int contour_perimeter_test( void )
{
    const double  success_error_level = 1e-4;
    const int max_delta = 100;

    int  seed = atsGetSeed();

    /* position where the maximum error occured */
    int     merr_iter = 0;

    /* test parameters */
    int     i = 0, j = 0;
    int     curr_size = 0;
    CvPoint*  array = 0;
    double  max_err = 0.;
    int     code = TRS_OK;

    AtsRandState rng_state;
    CvMemStorage* storage;
    CvSize size;

    atsRandInit( &rng_state, 0, 1, seed );

    read_contour_params();

    size = cvSize( img_size, img_size*3/4 );

    storage = cvCreateMemStorage(atsRandPlain32s(&rng_state) % max_storage_size );

    for( i = 0; i < base_iters; i++ )
    {
        double perimeter, etalon_perimeter;
        double err = 0;
        int count = atsRandPlain32s( &rng_state ) % (max_contour_size) + 1;
        int len, len0;
        CvSlice slice; 
        CvSeqReader reader;
        CvPoint pt0 = { 0, 0 };

        CvSeq* contour = cvCreateSeq( CV_SEQ_CONTOUR, sizeof(CvContour),
                                      sizeof(CvPoint), storage );

        for( j = 0; j < count; j++ )
        {
            CvPoint pt;

            if( j % 10 == 0 )
            {
                pt0 = atsRandPoint(&rng_state, size);
            }

            pt.x = atsRandPlain32s(&rng_state) % max_delta - max_delta/2;
            pt.y = atsRandPlain32s(&rng_state) % max_delta - max_delta/2;

            cvSeqPush( contour, &pt0 );

            pt0.x += pt.x;
            pt0.y += pt.y;
        }

        if( i % 10 == 0 )
        {
            slice = CV_WHOLE_SEQ;
        }
        else
        {
            slice.start_index = atsRandPlain32s( &rng_state ) % (count*3) - count;
            slice.end_index = atsRandPlain32s( &rng_state ) % (count*3) - count;
        }

        perimeter = cvArcLength( contour, slice );
 
        if( perimeter < 0 )
        {
            code = TRS_FAIL;
            goto test_exit;
        }

        len0 = cvSliceLength( slice, contour );
        slice.end_index++;

        len = cvSliceLength( slice, contour );
        if( len <= 0 )
        {
            assert( len0 != 0 );
            len = MIN( len0 + 1, count );
            if( slice.start_index < 0 )
                slice.start_index += count;
            else if( slice.start_index >= count )
                slice.start_index -= count;
            slice.end_index = slice.start_index + len;
        }

        if( len <= 0 )
            continue;

        if( len + 1 > curr_size )
        {
            curr_size = len + 1;
            array = (CvPoint*)realloc( array, curr_size*sizeof(array[0]) );
        }
        
        cvCvtSeqToArray( contour, array, slice );

        cvStartReadSeq( contour, &reader, 0 );
        cvSetSeqReaderPos( &reader, slice.start_index, 0 );

        for( j = 0; j < len; j++ )
        {
            CvPoint pt;
            CV_READ_SEQ_ELEM( pt, reader );
            if( array[j].x != pt.x || array[j].y != pt.y )
            {
                code = TRS_FAIL;
                goto test_exit;
            }
        }

        if( len0 == contour->total )
        {
            array[len++] = array[0];
        }

        etalon_perimeter = calc_contour_perimeter( array, len );

        assert( etalon_perimeter >= 0 );

        err = fabs(perimeter - etalon_perimeter)/(etalon_perimeter + DBL_EPSILON);

        if( err > max_err )
        {
            merr_iter = i;
            max_err   = err;
            if( max_err > success_error_level )
                goto test_exit;
        }

        cvClearMemStorage( storage );
    }

test_exit:

    cvReleaseMemStorage( &storage );
    if(array)
        free(array);

    if( code == TRS_OK )
    {
        trsWrite( ATS_LST, "Max err is %g at iter = %d, seed = %08x",
                           max_err, merr_iter, seed );

        return max_err <= success_error_level ?
            trsResult( TRS_OK, "No errors" ) :
            trsResult( TRS_FAIL, "Bad accuracy" );
    }
    else
    {
        trsWrite( ATS_LST, "Fatal error at iter = %d, seed = %08x", i, seed );
        return trsResult( TRS_FAIL, "Test failed" );
    }
}

#if 0
static int contour_approx_drawing_test( void )
{
    const int  depth = IPL_DEPTH_8U;
    const int  min_brightness = 0, max_brightness = 2;

    const char* err_desc = "No errors";
    int     seed  = atsGetSeed();

    /* test parameters */
    int     i = 0;
    int     code = TRS_OK;

    IplImage    *src_img, *dst2_img;
    AtsRandState rng_state;
    CvMemStorage* storage;
    CvSize size;

    atsRandInit( &rng_state, 0, 1, seed );

    read_contour_params();

    size = cvSize( img_size, img_size*3/4 );

    src_img  = cvCreateImage( size, depth, 1 );
    dst2_img = cvCreateImage( size, depth, 3 );

    storage = cvCreateMemStorage(atsRandPlain32s(&rng_state) % max_storage_size );

    for( i = 0; i < base_iters; i++ )
    {
        int count;
        CvChainApproxMethod approxMethod;
        CvContourRetrievalMode retrMode;

        (int&)approxMethod = atsRandPlain32s( &rng_state ) % 4 + 1;
        (int&)retrMode = atsRandPlain32s( &rng_state ) % 4;
        
        CvSeq* contours = 0;
        atsGenerateBlobImage( src_img, min_blob_size, max_blob_size, blob_count,
                              min_brightness, max_brightness, &rng_state );

        count = cvFindContours( src_img, storage, &contours, sizeof(CvContour),
                                retrMode, approxMethod );

        cvZero( dst2_img );
        CvSeq* seq = icvSeqTreeToSeq( contours );
        int j;

        if( count != seq->total )
        {
            code = TRS_FAIL;
            err_desc = "icvSeqTreeToSeq doesn't gather right quantity of contours";
        }

        for( j = 0; j < seq->total; j++ )
        {
            CvSeq* cur = *(CvSeq**)cvGetSeqElem( seq, j );
            CvSeq* new_seq = 0;
            
            if( (atsRandPlain32s( &rng_state ) & 0x8000) == 0 )
            {
                cur->total = MAX( cur->total/2, 1 );
                cur->flags &= ~CV_SEQ_FLAG_CLOSED;
            }

            OPENCV_CALL( new_seq = cvApproxPoly( cur, cur->header_size, storage, CV_POLY_APPROX_DP,
                                   atsRandPlain32s(&rng_state)%10, 0 ));
            OPENCV_CALL( cvDrawContours( dst2_img, new_seq, CV_RGB(255,0,0), CV_RGB(0,255,0), 0, 1 ));
        }

#if DRAW_CONTOURS2
        named_window( "test", 0 );
        cvDrawContours( dst2_img, contours, CV_RGB(255,255,255), CV_RGB(255,255,255), 255, 1 );
        show_iplimage( "test", dst2_img );
        wait_key(0);
#endif

        cvClearMemStorage( storage );
    }

    cvReleaseMemStorage( &storage );

    cvReleaseImage( &src_img );
    cvReleaseImage( &dst2_img );

    trsWrite( ATS_LST, "Fatal error at iter = %d, seed = %08x", i, seed );
    return trsResult( code, err_desc );
}
#endif

void InitAContours()
{
    trsReg( funcName[0], testName[0], atsAlgoClass, contour_retrieving_test );
    trsReg( funcName[1], testName[1], atsAlgoClass, contour_perimeter_test );
    //trsReg( funcName[2], testName[2], atsAlgoClass, contour_approx_drawing_test );
}

/* End of file. */
