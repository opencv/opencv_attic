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
#include  <limits.h>
#include  <float.h>
#include  "_cvwrap.h"
#include  "_cvdatastructs.h"
#include  "_cvgeom.h"

CV_IMPL  double  cvContourPerimeter( CvSeq *contour, CvSlice slice )
{
    double perimeter = 0;
    int i, j = 0, count;
    const int N = 16;
    float buffer[N];

    CvSeqReader reader;

    CV_FUNCNAME("cvCalcContourPerimeter");

    __BEGIN__;

    if( !contour )
        CV_ERROR_FROM_STATUS( CV_NULLPTR_ERR );

    if( !CV_IS_SEQ_POLYLINE( contour ))
        CV_ERROR_FROM_STATUS( CV_BADFLAG_ERR );

    if( contour->total > 1 )
    {
        CvPoint pt1, pt2;
        
        cvStartReadSeq( contour, &reader, 0 );
        cvSetSeqReaderPos( &reader, slice.startIndex );
        count = icvSliceLength( slice, contour );

        CV_ADJUST_EDGE_COUNT( count, contour );

        /* scroll the reader by 1 point */
        CV_READ_EDGE( pt1, pt2, reader );

        for( i = 0; i < count; i++ )
        {
            int dx, dy;
            int edge_length;

            CV_READ_EDGE( pt1, pt2, reader );

            dx = pt2.x - pt1.x;
            dy = pt2.y - pt1.y;

            edge_length = dx * dx + dy * dy;

            buffer[j] = (float)edge_length;
            if( ++j == N || i == count - 1 )
            {
                cvbSqrt( buffer, buffer, j );
                for( ; j > 0; j-- )
                    perimeter += buffer[j-1];
            }
        }
    }

    __CLEANUP__
    __END__

    return perimeter;
}


CvStatus
icvFindCircle( CvPoint2D32f pt0, CvPoint2D32f pt1,
               CvPoint2D32f pt2, CvPoint2D32f * center, float *radius )
{
    double x1 = (pt0.x + pt1.x) * 0.5;
    double dy1 = pt0.x - pt1.x;
    double x2 = (pt1.x + pt2.x) * 0.5;
    double dy2 = pt1.x - pt2.x;
    double y1 = (pt0.y + pt1.y) * 0.5;
    double dx1 = pt1.y - pt0.y;
    double y2 = (pt1.y + pt2.y) * 0.5;
    double dx2 = pt2.y - pt1.y;
    double t = 0;

    CvStatus result = CV_OK;

    if( icvIntersectLines( x1, dx1, y1, dy1, x2, dx2, y2, dy2, &t ) >= 0 )
    {
        center->x = (float) (x2 + dx2 * t);
        center->y = (float) (y2 + dy2 * t);
        *radius = (float) icvDistanceL2_32f( *center, pt0 );
    }
    else
    {
        center->x = center->y = 0.f;
        radius = 0;
        result = CV_NOTDEFINED_ERR;
    }

    return result;
}



int
icvFindEnslosingCicle4pts_32f( CvPoint2D32f * pts, CvPoint2D32f * _center, float *_radius )
{
    int shuffles[4][4] = { {0, 1, 2, 3}, {0, 1, 3, 2}, {2, 3, 0, 1}, {2, 3, 1, 0} };

    int idxs[4] = { 0, 1, 2, 3 };
    int i, j, k = 1, mi = 0;
    float max_dist = 0;
    CvPoint2D32f center;
    CvPoint2D32f min_center;
    float radius, min_radius = FLT_MAX;
    CvPoint2D32f res_pts[4];

    center = min_center = pts[0];
    radius = 1.f;

    for( i = 0; i < 4; i++ )
        for( j = i + 1; j < 4; j++ )
        {
            float dist = icvDistanceL2_32f( pts[i], pts[j] );

            if( max_dist < dist )
            {
                max_dist = dist;
                idxs[0] = i;
                idxs[1] = j;
            }
        }

    if( max_dist == 0 )
        goto function_exit;

    k = 2;
    for( i = 0; i < 4; i++ )
    {
        for( j = 0; j < k; j++ )
            if( i == idxs[j] )
                break;
        if( j == k )
            idxs[k++] = i;
    }

    center = icvMidPoint( pts[idxs[0]], pts[idxs[1]] );
    radius = (float) (icvDistanceL2_32f( pts[idxs[0]], center ) * (1 + 0.03));
    if( radius < 1.f )
        radius = 1.f;

    if( icvIsPtInCircle( pts[idxs[2]], center, radius ) &&
        icvIsPtInCircle( pts[idxs[3]], center, radius ))
    {
        k = 2;
    }
    else
    {
        mi = -1;
        for( i = 0; i < 4; i++ )
        {
            if( icvFindCircle( pts[shuffles[i][0]], pts[shuffles[i][1]],
                               pts[shuffles[i][2]], &center, &radius ) >= 0 )
            {
                radius *= 1.03f;
                if( radius < 2.f )
                    radius = 2.f;

                if( icvIsPtInCircle( pts[shuffles[i][3]], center, radius ) &&
                    min_radius > radius )
                {
                    min_radius = radius;
                    min_center = center;
                    mi = i;
                }
            }
        }
        assert( mi >= 0 );
        if( mi < 0 )
            mi = 0;
        k = 3;
        center = min_center;
        radius = min_radius;
        for( i = 0; i < 4; i++ )
            idxs[i] = shuffles[mi][i];
    }

  function_exit:

    *_center = center;
    *_radius = radius;

    /* reorder output points */
    for( i = 0; i < 4; i++ )
    {
        res_pts[i] = pts[idxs[i]];
    }

    for( i = 0; i < 4; i++ )
    {
        pts[i] = res_pts[i];
    }

    return k;
}


CV_IMPL void
cvMinEnclosingCircle( CvSeq * sequence, CvPoint2D32f * _center, float *_radius )
{
    const int max_iters = 20;
    CvSeqReader reader;
    int i, k, count;
    CvPoint *pt_left, *pt_right, *pt_top, *pt_bottom;
    CvPoint pt;
    CvPoint2D32f center = { 0, 0 };
    CvPoint2D32f pts[8];
    float radius = 0;

    if( _center )
        _center->x = _center->y = 0.f;
    if( _radius )
        *_radius = 0;

    CV_FUNCNAME( "cvMinEnclosingCircle" );

    __BEGIN__;

    if( !sequence || !_center || !_radius )
        CV_ERROR_FROM_STATUS( CV_NULLPTR_ERR );
    if( sequence->total <= 0 )
        CV_ERROR_FROM_STATUS( CV_BADSIZE_ERR );
    if( !CV_IS_SEQ_POINT_SET( sequence ))
        CV_ERROR_FROM_STATUS( CV_BADFLAG_ERR );

    CV_CALL( cvStartReadSeq( sequence, &reader, 0 ));

    pt_left = pt_right = pt_top = pt_bottom = (CvPoint *) (reader.ptr);
    CV_READ_SEQ_ELEM( pt, reader );

    count = sequence->total;
    for( i = 1; i < count; i++ )
    {
        CvPoint *pt_ptr = (CvPoint *) (reader.ptr);
        CvPoint pt;

        CV_READ_SEQ_ELEM( pt, reader );

        if( pt.x < pt_left->x )
            pt_left = pt_ptr;
        if( pt.x > pt_right->x )
            pt_right = pt_ptr;
        if( pt.y < pt_top->y )
            pt_top = pt_ptr;
        if( pt.y > pt_bottom->y )
            pt_bottom = pt_ptr;
    }

    pts[0] = icvCvtPoint32s_32f( *pt_left );
    pts[1] = icvCvtPoint32s_32f( *pt_right );
    pts[2] = icvCvtPoint32s_32f( *pt_top );
    pts[3] = icvCvtPoint32s_32f( *pt_bottom );

    for( k = 0; k < max_iters; k++ )
    {
        icvFindEnslosingCicle4pts_32f( pts, &center, &radius );
        cvStartReadSeq( sequence, &reader, 0 );

        for( i = 0; i < count; i++ )
        {
            CvPoint pt;
            CvPoint2D32f ptfl;

            CV_READ_SEQ_ELEM( pt, reader );

            ptfl = icvCvtPoint32s_32f( pt );
            if( !icvIsPtInCircle( ptfl, center, radius ))
            {
                pts[3] = ptfl;
                break;
            }
        }
        if( i == count )
            break;
    }

    __CLEANUP__;
    __END__;

    *_center = center;
    *_radius = radius;
}


/* End of file. */
