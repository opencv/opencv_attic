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
#include "_cv.h"
#include "_cvwrap.h"
#include "_cvgeom.h"
#include "string.h"

#include <limits.h>

typedef struct CvContourEx
{
    CV_CONTOUR_FIELDS()
    int counter;
}
CvContourEx;


/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:    cvFindChessBoardCornerGuesses8uC1R
//    Purpose:
//      Function finds first approximation of internal corners on the chess board.
//    Context:
//    Parameters:
//      img      - source haltone image
//      step     - its full width in bytes
//      thresh   - temporary image where will the thresholded source image be stored.
//      th_step  - full width of temporary image row in bytes
//      size     - width and height of the images in pixels
//      etalon_size - number of corners in checkerboard per row and per column.
//      corners  - pointer to array, containing found points
//      corner_count - number of corners found
//    Returns:
//      CV_NO_ERR if all Ok or error code
//    Notes:
//F*/
CV_IMPL int
cvFindChessBoardCornerGuesses( const void* arr, void* thresharr,
                               CvMemStorage * storage,
                               CvSize etalon_size, CvPoint2D32f * corners,
                               int *corner_count )
{
#define BUFFER_SIZE   8192
    const int min_approx_level = 2;
    const int max_approx_level = 4;
    int min_size;
    CvContourScanner scanner;
    CvStatus result = CV_NO_ERR;
    double mean;
    int thresh_level;
    int temp_count = 0, found = 0;
    CvPoint pt_buffer[BUFFER_SIZE];
    CvSeqReader reader;
    CvSeq *src_contour = 0;
    CvMemStorage *storage1 = 0;
    CvSeq *root;
    CvContourEx* board = 0;
    int max_count = 0;
    CvPoint *iPoints = 0;
    CvPoint *ordered = 0;
    CvPoint *hullpoints = 0;
    int *indices = 0;
    int idx;
    int quandrangles = 0;

    CV_FUNCNAME( "cvFindChessBoardCornerGuesses" );

    __BEGIN__;

    CvMat  stub, *img = (CvMat*)arr;
    CvMat  thstub, *thresh = (CvMat*)thresharr;
    CvSize size;

    if( corner_count )
    {
        max_count = *corner_count;
        *corner_count = 0;
    }

    CV_CALL( img = cvGetMat( img, &stub ));
    CV_CALL( thresh = cvGetMat( thresh, &thstub ));

    if( CV_ARR_TYPE( img->type ) != CV_8UC1 ||
        CV_ARR_TYPE( thresh->type ) != CV_8UC1 )
        CV_ERROR( CV_BadDepth, icvUnsupportedFormat );

    if( !CV_ARE_SIZES_EQ( img, thresh ))
        CV_ERROR( CV_StsUnmatchedSizes, "" );

    size = icvGetMatSize( img );

    // 
    //   Create temporary storages.
    //   First one will store retrived contours and the
    //   second one - approximated contours.
    storage1 = storage ? cvCreateChildMemStorage( storage ) : cvCreateMemStorage(0);
    root = cvCreateSeq( 0, sizeof(CvSeq), sizeof(CvSeq*), storage1 );

    // empiric bound for minimal allowed perimeter for squares 
    min_size = cvRound( size.width*size.height * .03 * 0.03 );

    // empiric threshold level 
    mean = cvMean( img );
    thresh_level = cvRound( mean - 10 );
    thresh_level = MAX( thresh_level, 10 );

    // Make dilation before the thresholding.
    // It splits chessboard corners 
    cvDilate( img, thresh, 0, 1 );

    // convert to binary 
    cvThreshold( thresh, thresh, thresh_level, 255, CV_THRESH_BINARY );

    // initialize contour retrieving routine 
    scanner = cvStartFindContours( thresh, storage1, sizeof( CvContourEx ),
                                   CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE );

    // get all the contours one by one 
    while( (src_contour = cvFindNextContour( scanner )) != 0 )
    {
        CvSeq *dst_contour = 0;
        CvRect rect = ((CvContour*)src_contour)->rect;

        // reject contours with too small perimeter 
        if( CV_IS_SEQ_HOLE( src_contour ) &&
            rect.width*rect.height >= min_size )
        {
            int approx_level;

            for( approx_level = min_approx_level;
                 approx_level <= max_approx_level;
                 approx_level++ )
            {
                dst_contour = cvApproxPoly( src_contour, sizeof( CvContour ),
                                            storage1, CV_POLY_APPROX_DP,
                                            (float)approx_level );

                if( dst_contour->total == 4 )
                    break;
            }

            quandrangles += dst_contour->total == 4;

            // reject non-quadrangles 
            if( dst_contour->total == 4 && cvCheckContourConvexity( dst_contour ))
            {
                CvPoint pt[4];
                int i;
                double d1, d2, p = cvContourPerimeter(dst_contour);

                for( i = 0; i < 4; i++ )
                    pt[i] = *(CvPoint*)cvGetSeqElem( dst_contour, i );

                d1 = sqrt((pt[0].x - pt[2].x)*(pt[0].x - pt[2].x) +
                          (pt[0].y - pt[2].y)*(pt[0].y - pt[2].y));

                d2 = sqrt((pt[1].x - pt[3].x)*(pt[1].x - pt[3].x) +
                          (pt[1].y - pt[3].y)*(pt[1].y - pt[3].y));

                if( d1 >= 0.25*p && d2 >= 0.25*p )
                {
                    CvContourEx* parent = (CvContourEx*)(src_contour->v_prev);
                    parent->counter++;
                    if( !board || board->counter < parent->counter )
                        board = parent;
                    dst_contour->v_prev = (CvSeq*)parent;
                    cvSeqPush( root, &dst_contour );
                }
            }
        }
    }

    // finish contour retrieving 
    cvEndFindContours( &scanner );

    // iterate through all the "good" contours and store the corners. 
    //for( src_contour = root; src_contour != 0; src_contour = src_contour->h_next )
    for( idx = 0; idx < root->total; idx++ )
    {
        src_contour = *(CvSeq**)cvGetSeqElem( root, idx );
        int i, j, total = src_contour->total;
        int keep_temp_count = temp_count;

        if( src_contour->v_prev != (CvSeq*)board )
            continue;

        cvStartReadSeq( src_contour, &reader, 0 );

        assert( total == 4 );

        //   choose the points of the current quadrangle that are close to
        //   some points of the other quadrangles
        //   (it can happen for splitted corners (due to dilation) of the
        //   checker board). Search only in other quadrangles! That's why
        //   keep_temp_count is used instead of temp_count 
        for( i = 0; i < total; i++ )
        {
            CvPoint pt;

            CV_READ_SEQ_ELEM( pt, reader );

            for( j = 0; j < keep_temp_count; j++ )
            {
                int dx = pt.x - pt_buffer[j].x;
                int dy = pt.y - pt_buffer[j].y;

                dx = abs( dx );
                dy = abs( dy );

                if( MAX( dx, dy ) < 8 )
                    break;
            }

            // if the close point has been found, store the middle point
            // to the destination buffer 
            if( j < keep_temp_count )
            {
                if( found >= max_count )
                {
                    result = CV_OUTOFMEM_ERR;
                    EXIT;
                }
                corners[found].x = (pt.x + pt_buffer[j].x) * 0.5f;
                corners[found].y = (pt.y + pt_buffer[j].y) * 0.5f;
                found++;
                // remove found point from the temporary buffer 
                memmove( pt_buffer + j, pt_buffer + j + 1,
                         (temp_count - j - 1) * sizeof( CvPoint ));
                temp_count--;
                keep_temp_count--;
            }
            else
            {
                // else store point in the buffer 
                if( temp_count >= BUFFER_SIZE )
                {
                    result = CV_OUTOFMEM_ERR;
                    EXIT;
                }
                pt_buffer[temp_count++] = pt;
            }
        }
    }

    result = CV_NOTDEFINED_ERR;

    /*************** reorder found corners in the right order **************/
    // ------ New processing of points -------- 
    if( found == etalon_size.width * etalon_size.height )
    {
        // Copy all points to iPoints array for inner computation 
        // This is a prepare part. It calls ones 
        int etalon_points = found;
        CvPoint cornerPoints[4];
        CvPoint firstPoint = { 0, 0 };

        CvSeq hullcontour;
        CvSeqBlock hullcontour_blk;
        CvSeq *hullcontour2 = 0;

        CvPoint start_pt;
        CvPoint end_pt;
        CvPoint botStart_pt = { 0, 0 };
        CvPoint botEnd_pt = { 0, 0 };

        int min_dist;
        int test_dist;

        int numRestPoints = etalon_points;
        int ind_size;
        int level, max_level = 16;
        int numNearest;
        int numNearPoints;

        int currLine = 0;
        int numPointsInLine = 0;        // Now we don't know how many points in line 
        int numLines = 0;       // Now we don't know how many lines 
        int dx, dy, denom;
        int i, j;

        indices = (int *) icvAlloc( etalon_points * sizeof( int ));

        iPoints = (CvPoint *) icvAlloc( etalon_points * sizeof( CvPoint ));
        ordered = (CvPoint *) icvAlloc( etalon_points * sizeof( CvPoint ));
        hullpoints = (CvPoint *) icvAlloc( etalon_points * sizeof( CvPoint ));

        for( i = 0; i < etalon_points; i++ )
        {
            iPoints[i] = icvCvtPoint32f_32s( corners[i] );
        }

        numRestPoints = etalon_points;

#define IPCV_L2_DIST( pt1, pt2 )  \
        (dx = (pt1).x - (pt2).x, dy = (pt1).y - (pt2).y, dx*dx + dy*dy)

        // Find minimal distance between image etalon points 
        min_dist = INT_MAX;

        for( i = 0; i < etalon_points - 1; i++ )
        {
            for( j = i + 1; j < etalon_points; j++ )
            {
                int dist = IPCV_L2_DIST( iPoints[i], iPoints[j] );

                if( dist < min_dist )
                {
                    min_dist = dist;
                }
            }
        }

        test_dist = min_dist / 2;

        // minimal distance was found 
        for( currLine = 0; numRestPoints > 0; currLine++ )
        {
            // Find convex hull. This is part call many times. 
            // For each points line 

            // Find convex hull for all rest points 

            if( numRestPoints == numPointsInLine )
            {
                // This is a last line and we know start and end point 
                start_pt = botStart_pt;
                end_pt = botEnd_pt;
            }
            else
            {
                cvConvexHull( iPoints, numRestPoints, 0,
                              CV_CLOCKWISE, indices, &ind_size );

                // My Make Sequance from convex hull 
                // Make array of points according convex hull indexes 
                for( i = 0; i < ind_size; i++ )
                {
                    hullpoints[i] = iPoints[indices[i]];
                }

                cvMakeSeqHeaderForArray( CV_SEQ_POLYGON, sizeof( CvSeq ),
                                         sizeof( CvPoint ),
                                         hullpoints, ind_size, &hullcontour,
                                         &hullcontour_blk );

                max_level = cvRound( sqrt( min_dist ));

                // approximate convex hull. we should get quadrangle 
                for( level = min_approx_level; level <= max_level; level++ )
                {
                    hullcontour2 = cvApproxPoly( &hullcontour, sizeof( CvContour ),
                                                 storage1, CV_POLY_APPROX_DP,
                                                 (float) level );

                    if( hullcontour2->total == 4 )
                        break;
                }

                if( level > max_level )
                {
                    result = CV_NOTDEFINED_ERR;
                    EXIT;
                }

                cvCvtSeqToArray( hullcontour2, cornerPoints );

                // Find two nearest corner points to last found points 
                // but if its first points just set it 

                if( currLine == 0 )
                {
                    firstPoint = cornerPoints[0];
                    botStart_pt = cornerPoints[3];
                    botEnd_pt = cornerPoints[2];
                    numNearest = 0;
                }
                else
                {
                    // Find nearest points to first point 
                    min_dist = INT_MAX;
                    numNearest = -1;

                    for( i = 0; i < 4; i++ )
                    {
                        int dist = IPCV_L2_DIST( firstPoint, cornerPoints[i] );

                        if( dist < min_dist )
                        {
                            numNearest = i;
                            min_dist = dist;
                        }
                    }
                    assert( numNearest >= 0 );
                    firstPoint = cornerPoints[numNearest];
                }

                // set end point for first line 
                start_pt = firstPoint;
                end_pt = cornerPoints[(numNearest + 1) % 4];
                // Now we know position of quad 
            }
            // Now we know position of current line 
            // Find nearest points to line 
            numNearPoints = 0;
            dx = start_pt.x - end_pt.x;
            dy = start_pt.y - end_pt.y;
            denom = dx * dx + dy * dy;

            for( i = 0; i < numRestPoints; i++ )
            {
                int num = (iPoints[i].x - start_pt.x) * dy -
                            (iPoints[i].y - start_pt.y) * dx;

                if( ((int64) num) * num < ((int64) denom) * test_dist )
                {
                    indices[numNearPoints++] = i;
                }
            }

            // Test number of points in the line 
            // Number must be equal etalon 
            if( currLine == 0 )
            {
                // This is a first step and need to set number points 
                if( numNearPoints == etalon_size.width )
                {
                    numPointsInLine = etalon_size.width;
                    numLines = etalon_size.height;
                }
                else
                {
                    if( numNearPoints == etalon_size.height )
                    {
                        numPointsInLine = etalon_size.height;
                        numLines = etalon_size.width;
                    }
                    else
                    {
                        // Number of found points in line not correct 
                        result = CV_NOTDEFINED_ERR;
                        EXIT;
                    }
                }
            }
            else if( numNearPoints != numPointsInLine )
            {
                // Number of found points in line not correct 
                result = CV_NOTDEFINED_ERR;
                EXIT;
            }

            // bubble sort of newly collected points by the distance to start_pt 
            for( i = numNearPoints - 1; i > 0; i-- )
            {
                for( j = 0; j < i; j++ )
                {
                    CvPoint pt = iPoints[indices[j]];
                    int dist = IPCV_L2_DIST( start_pt, pt );

                    pt = iPoints[indices[j + 1]];
                    if( dist > IPCV_L2_DIST( start_pt, pt ))
                    {
                        // Change points 
                        int t = indices[j + 1];

                        indices[j + 1] = indices[j];
                        indices[j] = t;
                    }
                }
            }

            // Collect found points to array 
            for( i = 0; i < numNearPoints; i++ )
            {
                j = indices[i];
                ordered[currLine * numPointsInLine + i] = iPoints[j];
                iPoints[j].x = INT_MIN; // mark the point as a deleted one 
            }

            // Exclude found points. Test all rest points 
            for( i = 0, j = 0; i < numRestPoints; i++ )
            {
                if( iPoints[i].x != INT_MIN )
                {
                    if( i != j )
                        iPoints[j] = iPoints[i];
                    j++;
                }
            }

            assert( numRestPoints == j + numNearPoints );
            numRestPoints = j;
        }

        // Switch horizontal and vertical order of points if it need 
        if( etalon_size.width == numPointsInLine )
        {
            if( etalon_size.height == numLines )
            {
                for( i = 0; i < etalon_points; i++ )
                {
                    corners[i] = icvCvtPoint32s_32f( ordered[i] );
                }
            }
            else
            {
                result = CV_NOTDEFINED_ERR;
                EXIT;
            }
        }
        else
        {
            if( etalon_size.width == numLines )
            {
                for( i = 0; i < etalon_size.height; i++ )
                    for( j = 0; j < etalon_size.width; j++ )
                    {
                        corners[i * etalon_size.width + j] =
                            icvCvtPoint32s_32f( ordered[j * etalon_size.height + i] );
                    }
            }
            else
            {
                result = CV_NOTDEFINED_ERR;
                EXIT;
            }
        }

        // calculate vector product, if it's positive,
        // reverse all the rows 
        if( (corners[etalon_size.width - 1].x - corners[0].x) *
            (corners[etalon_size.width].y - corners[etalon_size.width - 1].y) -
            (corners[etalon_size.width - 1].y - corners[0].y) *
            (corners[etalon_size.width].x - corners[etalon_size.width - 1].x) > 0 )
        {
            for( i = 0; i < etalon_size.height; i++ )
                for( j = 0; j < etalon_size.width / 2; j++ )
                {
                    CvPoint2D32f temp = corners[i * etalon_size.width + j];

                    corners[i * etalon_size.width + j] =
                        corners[(i + 1) * etalon_size.width - 1 - j];
                    corners[(i + 1) * etalon_size.width - 1 - j] = temp;
                }
        }

        // All right points are collected and we can add them 
        result = CV_NO_ERR;
    }
    //////////////////////////////////////////////
    //////////////////////////////////////////////
    //////////////////////////////////////////////
    //////////////////////////////////////////////
    __CLEANUP__;
    __END__;

    icvFree( &iPoints );
    icvFree( &indices );
    icvFree( &ordered );
    icvFree( &hullpoints );

    // release storages 
    cvReleaseMemStorage( &storage1 );

    assert( found >= 0 );

    // store number of found corners 
    if( corner_count )
        *corner_count = found;

    return result == CV_OK;
}

/* End of file. */
