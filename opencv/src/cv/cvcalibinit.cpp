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

/************************************************************************************\
    This is improved variant of chessboard corner detection algorithm that
    uses a graph of connected quads. It is based on the code contributed
    by Vladimir Vezhnevets and Philip Gruebele.
    Here is the copyright notice from the original Vladimir's code:
    ===============================================================

    The algorithms developed and implemented by Vezhnevets Vldimir
    aka Dead Moroz (vvp@graphics.cs.msu.ru)
    See http://graphics.cs.msu.su/en/research/calibration/opencv.html
    for detailed information.

    Reliability additions and modifications made by Philip Gruebele.
    <a href="mailto:pgruebele@cox.net">pgruebele@cox.net</a>

\************************************************************************************/

#include "_cv.h"

//=====================================================================================
// Implementation for the enhanced calibration object detection
//=====================================================================================

#define MAX_CONTOUR_APPROX  7

typedef struct CvContourEx
{
    CV_CONTOUR_FIELDS()
    int counter;
}
CvContourEx;

//=====================================================================================

/// Corner info structure
/** This structure stores information about the chessboard corner.*/
typedef struct CvCBCorner
{
    CvPoint2D32f pt; // Coordinates of the corner
    int row;         // Board row index
    int count;       // Number of neighbor corners
    struct CvCBCorner* neighbors[4]; // Neighbor corners
}
CvCBCorner;

//=====================================================================================
/// Quadrangle contour info structure
/** This structure stores information about the chessboard quadrange.*/
typedef struct CvCBQuad
{
    int count;      // Number of quad neibors
    int group_idx;  // quad group ID
    float edge_len; // quad size characteristic
    CvCBCorner *corners[4]; // Coordinates of quad corners
    struct CvCBQuad *neighbors[4]; // Pointers of quad neighbors
}
CvCBQuad;

//=====================================================================================

//static CvMat* debug_img = 0;

static int icvGenerateQuads( CvCBQuad **quads, CvCBCorner **corners,
                             CvMemStorage *storage, CvMat *image, int flags );

static void icvFindQuadNeighbors( CvCBQuad *quads, int quad_count );

static int icvFindConnectedQuads( CvCBQuad *quads, int quad_count,
                                  CvCBQuad **quad_group, int group_idx,
                                  CvMemStorage* storage );

static int icvCheckQuadGroup( CvCBQuad **quad_group, int count,
                              CvCBCorner **out_corners, CvSize pattern_size );

static int icvCleanFoundConnectedQuads( int quad_count,
                CvCBQuad **quads, CvSize pattern_size );

CV_IMPL
int cvFindChessboardCorners( const void* arr, CvSize pattern_size,
                             CvPoint2D32f* out_corners, int* out_corner_count,
                             int flags )
{
    const int min_dilations = 1;
    const int max_dilations = 3;
    int found = 0;
    CvMat* norm_img = 0;
    CvMat* thresh_img = 0;
    CvMemStorage* storage = 0;

    CvCBQuad *quads = 0, **quad_group = 0;
    CvCBCorner *corners = 0, **corner_group = 0;

    if( out_corner_count )
        *out_corner_count = 0;

    CV_FUNCNAME( "cvFindChessBoardCornerGuesses2" );

    __BEGIN__;

    int quad_count, group_idx, i, dilations;
    CvMat stub, *img = (CvMat*)arr;

    CV_CALL( img = cvGetMat( img, &stub ));
    //debug_img = img;

    if( CV_MAT_DEPTH( img->type ) != CV_8U || CV_MAT_CN( img->type ) == 2 )
        CV_ERROR( CV_StsUnsupportedFormat, "Only 8-bit grayscale or color images are supported" );

    if( pattern_size.width <= 2 || pattern_size.height <= 2 )
        CV_ERROR( CV_StsOutOfRange, "pattern should have at least 2x2 size" );

    if( !out_corners )
        CV_ERROR( CV_StsNullPtr, "Null pointer to corners" );

    CV_CALL( storage = cvCreateMemStorage(0) );
    CV_CALL( thresh_img = cvCreateMat( img->rows, img->cols, CV_8UC1 ));

    if( CV_MAT_CN(img->type) != 1 || (flags & CV_CALIB_CB_NORMALIZE_IMAGE) )
    {
        // equalize the input image histogram -
        // that should make the contrast between "black" and "white" areas big enough
        CV_CALL( norm_img = cvCreateMat( img->rows, img->cols, CV_8UC1 ));

        if( CV_MAT_CN(img->type) != 1 )
        {
            CV_CALL( cvCvtColor( img, norm_img, CV_BGR2GRAY ));
            img = norm_img;
        }

        if( flags & CV_CALIB_CB_NORMALIZE_IMAGE )
        {
            cvEqualizeHist( img, norm_img );
            img = norm_img;
        }
    }

    // Try our standard "1" dilation, but if the pattern is not found, iterate the whole procedure with higher dilations.
    // This is necessary because some squares simply do not separate properly with a single dilation.  However,
    // we want to use the minimum number of dilations possible since dilations cause the squares to become smaller,
    // making it difficult to detect smaller squares.
    for( dilations = min_dilations; dilations <= max_dilations; dilations++ )
    {
        // convert the input grayscale image to binary (black-n-white)
        if( flags & CV_CALIB_CB_ADAPTIVE_THRESH )
        {
            int block_size = cvRound(MIN(img->cols,img->rows)*0.2)|1;
            cvDilate( img, thresh_img, 0, dilations );

            // convert to binary
            cvAdaptiveThreshold( img, thresh_img, 255,
                CV_ADAPTIVE_THRESH_MEAN_C, CV_THRESH_BINARY, block_size, 0 );
            cvDilate( thresh_img, thresh_img, 0, dilations-1 );
        }
        else
        {
            // Make dilation before the thresholding.
            // It splits chessboard corners
            //cvDilate( img, thresh_img, 0, 1 );

            // empiric threshold level
            double mean = cvMean( img );
            int thresh_level = cvRound( mean - 10 );
            thresh_level = MAX( thresh_level, 10 );

            cvThreshold( img, thresh_img, thresh_level, 255, CV_THRESH_BINARY );
            cvDilate( thresh_img, thresh_img, 0, dilations );
        }

        // So we can find rectangles that go to the edge, we draw a white line around the image edge.
        // Otherwise FindContours will miss those clipped rectangle contours.
        // The border color will be the image mean, because otherwise we risk screwing up filters like cvSmooth()...
        cvRectangle( thresh_img, cvPoint(0,0), cvPoint(thresh_img->cols-1,
                     thresh_img->rows-1), CV_RGB(255,255,255), 3, 8);

        CV_CALL( quad_count = icvGenerateQuads( &quads, &corners, storage, thresh_img, flags ));
        if( quad_count <= 0 )
            continue;

        // Find quad's neighbors
        CV_CALL( icvFindQuadNeighbors( quads, quad_count ));

        CV_CALL( quad_group = (CvCBQuad**)cvAlloc( sizeof(quad_group[0]) * quad_count));
        CV_CALL( corner_group = (CvCBCorner**)cvAlloc( sizeof(corner_group[0]) * quad_count*4 ));

        for( group_idx = 0; ; group_idx++ )
        {
            int count;
            CV_CALL( count = icvFindConnectedQuads( quads, quad_count, quad_group, group_idx, storage ));

            if( count == 0 )
                break;

            // If count is more than it should be, this will remove those quads
            // which cause maximum deviation from a nice square pattern.
            CV_CALL( count = icvCleanFoundConnectedQuads( count, quad_group, pattern_size ));
            CV_CALL( count = icvCheckQuadGroup( quad_group, count, corner_group, pattern_size ));

            if( count > 0 || (out_corner_count && -count > *out_corner_count) )
            {
                int n = count > 0 ? pattern_size.width * pattern_size.height : -count;
                n = MIN( n, pattern_size.width * pattern_size.height );

                // copy corners to output array
                for( i = 0; i < n; i++ )
                    out_corners[i] = corner_group[i]->pt;

                if( out_corner_count )
                    *out_corner_count = n;

                if( count > 0 )
                {
                    found = 1;
                    EXIT;
                }
            }
        }

        cvFree( &quads );
        cvFree( &corners );
    }

    __END__;

    cvReleaseMemStorage( &storage );
    cvReleaseMat( &norm_img );
    cvReleaseMat( &thresh_img );
    cvFree( &quads );
    cvFree( &corners );
    cvFree( &quad_group );
    cvFree( &corner_group );

    return found;
}


// if we found too many connect quads, remove those which probably do not belong.
static int
icvCleanFoundConnectedQuads( int quad_count, CvCBQuad **quad_group, CvSize pattern_size )
{
    CvMemStorage *temp_storage = 0;
    CvPoint2D32f *centers = 0;

    CV_FUNCNAME( "icvCleanFoundConnectedQuads" );

    __BEGIN__;

    CvPoint2D32f center = {0,0};
    int i, j, k;
    // number of quads this pattern should contain
    int count = ((pattern_size.width + 1)*(pattern_size.height + 1) + 1)/2;

    // if we have more quadrangles than we should,
    // try to eliminate duplicates or ones which don't belong to the pattern rectangle...
    if( quad_count <= count )
        EXIT;

    // create an array of quadrangle centers
    CV_CALL( centers = (CvPoint2D32f *)cvAlloc( sizeof(centers[0])*quad_count ));
    CV_CALL( temp_storage = cvCreateMemStorage(0));

    for( i = 0; i < quad_count; i++ )
    {
        CvPoint2D32f ci = {0,0};
        CvCBQuad* q = quad_group[i];

        for( j = 0; j < 4; j++ )
        {
            CvPoint2D32f pt = q->corners[j]->pt;
            ci.x += pt.x;
            ci.y += pt.y;
        }

        ci.x *= 0.25f;
        ci.y *= 0.25f;

        centers[i] = ci;
        center.x += ci.x;
        center.y += ci.y;
    }
    center.x /= quad_count;
    center.y /= quad_count;

    // If we still have more quadrangles than we should,
    // we try to eliminate bad ones based on minimizing the bounding box.
    // We iteratively remove the point which reduces the size of
    // the bounding box of the blobs the most
    // (since we want the rectangle to be as small as possible)
    // remove the quadrange that causes the biggest reduction
    // in pattern size until we have the correct number
    for( ; quad_count > count; quad_count-- )
    {
        double min_box_area = DBL_MAX;
        int skip, min_box_area_index = -1;
        CvCBQuad *q0, *q;

        // For each point, calculate box area without that point
        for( skip = 0; skip < quad_count; skip++ )
        {
            // get bounding rectangle
            CvPoint2D32f temp = centers[skip]; // temporarily make index 'skip' the same as
            centers[skip] = center;            // pattern center (so it is not counted for convex hull)
            CvMat pointMat = cvMat(1, quad_count, CV_32FC2, centers);
            CvSeq *hull = cvConvexHull2( &pointMat, temp_storage, CV_CLOCKWISE, 1 );
            centers[skip] = temp;
            double hull_area = fabs(cvContourArea(hull, CV_WHOLE_SEQ));

            // remember smallest box area
            if( hull_area < min_box_area )
            {
                min_box_area = hull_area;
                min_box_area_index = skip;
            }
            cvClearMemStorage( temp_storage );
        }

        q0 = quad_group[min_box_area_index];

        // remove any references to this quad as a neighbor
        for( i = 0; i < quad_count; i++ )
        {
            q = quad_group[i];
            for( j = 0; j < 4; j++ )
            {
                if( q->neighbors[j] == q0 )
                {
                    q->neighbors[j] = 0;
                    q->count--;
                    for( k = 0; k < 4; k++ )
                        if( q0->neighbors[k] == q )
                        {
                            q0->neighbors[k] = 0;
                            q0->count--;
                            break;
                        }
                    break;
                }
            }
        }

        // remove the quad
        quad_count--;
        quad_group[min_box_area_index] = quad_group[quad_count];
        centers[min_box_area_index] = centers[quad_count];
    }

    __END__;

    cvReleaseMemStorage( &temp_storage );
    cvFree( &centers );

    return quad_count;
}

//=====================================================================================

static int
icvFindConnectedQuads( CvCBQuad *quad, int quad_count, CvCBQuad **out_group,
                       int group_idx, CvMemStorage* storage )
{
    CvMemStorage* temp_storage = cvCreateChildMemStorage( storage );
    CvSeq* stack = cvCreateSeq( 0, sizeof(*stack), sizeof(void*), temp_storage );
    int i, count = 0;

    // Scan the array for a first unlabeled quad
    for( i = 0; i < quad_count; i++ )
    {
        if( quad[i].count > 0 && quad[i].group_idx < 0)
            break;
    }

    // Recursively find a group of connected quads starting from the seed quad[i]
    if( i < quad_count )
    {
        CvCBQuad* q = &quad[i];
        cvSeqPush( stack, &q );
        out_group[count++] = q;
        q->group_idx = group_idx;

        while( stack->total )
        {
            cvSeqPop( stack, &q );
            for( i = 0; i < 4; i++ )
            {
                CvCBQuad *neighbor = q->neighbors[i];
                if( neighbor && neighbor->count > 0 && neighbor->group_idx < 0 )
                {
                    cvSeqPush( stack, &neighbor );
                    out_group[count++] = neighbor;
                    neighbor->group_idx = group_idx;
                }
            }
        }
    }

    cvReleaseMemStorage( &temp_storage );
    return count;
}


//=====================================================================================

static int
icvCheckQuadGroup( CvCBQuad **quad_group, int quad_count,
                   CvCBCorner **out_corners, CvSize pattern_size )
{
    const int ROW1 = 1000000;
    const int ROW2 = 2000000;
    const int ROW_ = 3000000;
    int result = 0;
    int i, out_corner_count = 0, corner_count = 0;
    CvCBCorner** corners = 0;

    CV_FUNCNAME( "icvCheckQuadGroup" );

    __BEGIN__;

    int j, k, kk;
    int width = 0, height = 0;
    int hist[5] = {0,0,0,0,0};
    CvCBCorner* first = 0, *first2 = 0, *right, *cur, *below, *c;
    CV_CALL( corners = (CvCBCorner**)cvAlloc( quad_count*4*sizeof(corners[0]) ));

    // build dual graph, which vertices are internal quad corners
    // and two vertices are connected iff they lie on the same quad edge
    for( i = 0; i < quad_count; i++ )
    {
        CvCBQuad* q = quad_group[i];
        /*CvScalar color = q->count == 0 ? cvScalar(0,255,255) :
                         q->count == 1 ? cvScalar(0,0,255) :
                         q->count == 2 ? cvScalar(0,255,0) :
                         q->count == 3 ? cvScalar(255,255,0) :
                                         cvScalar(255,0,0);*/

        for( j = 0; j < 4; j++ )
        {
            //cvLine( debug_img, cvPointFrom32f(q->corners[j]->pt), cvPointFrom32f(q->corners[(j+1)&3]->pt), color, 1, CV_AA, 0 );
            if( q->neighbors[j] )
            {
                CvCBCorner *a = q->corners[j], *b = q->corners[(j+1)&3];
                // mark internal corners that belong to:
                //   - a quad with a single neighbor - with ROW1,
                //   - a quad with two neighbors     - with ROW2
                // make the rest of internal corners with ROW_
                int row_flag = q->count == 1 ? ROW1 : q->count == 2 ? ROW2 : ROW_;

                if( a->row == 0 )
                {
                    corners[corner_count++] = a;
                    a->row = row_flag;
                }
                else if( a->row > row_flag )
                    a->row = row_flag;

                if( q->neighbors[(j+1)&3] )
                {
                    if( a->count >= 4 || b->count >= 4 )
                        EXIT;
                    for( k = 0; k < 4; k++ )
                    {
                        if( a->neighbors[k] == b )
                            EXIT;
                        if( b->neighbors[k] == a )
                            EXIT;
                    }
                    a->neighbors[a->count++] = b;
                    b->neighbors[b->count++] = a;
                }
            }
        }
    }

    if( corner_count != pattern_size.width*pattern_size.height )
        EXIT;

    for( i = 0; i < corner_count; i++ )
    {
        int n = corners[i]->count;
        assert( 0 <= n && n <= 4 );
        hist[n]++;
        if( !first && n == 2 )
        {
            if( corners[i]->row == ROW1 )
                first = corners[i];
            else if( !first2 && corners[i]->row == ROW2 )
                first2 = corners[i];
        }
    }

    // start with a corner that belongs to a quad with a signle neighbor.
    // if we do not have such, start with a corner of a quad with two neighbors.
    if( !first )
        first = first2;

    if( !first || hist[0] != 0 || hist[1] != 0 || hist[2] != 4 ||
        hist[3] != (pattern_size.width + pattern_size.height)*2 - 8 )
        EXIT;

    cur = first;
    right = below = 0;
    out_corners[out_corner_count++] = cur;

    for( k = 0; k < 4; k++ )
    {
        c = cur->neighbors[k];
        if( c )
        {
            if( !right )
                right = c;
            else if( !below )
                below = c;
        }
    }

    if( !right || right->count != 2 && right->count != 3 ||
        !below || below->count != 2 && below->count != 3 )
        EXIT;

    cur->row = 0;
    //cvCircle( debug_img, cvPointFrom32f(cur->pt), 3, cvScalar(0,255,0), -1, 8, 0 );

    first = below; // remember the first corner in the next row
    // find and store the first row (or column)
    for(j=1;;j++)
    {
        right->row = 0;
        out_corners[out_corner_count++] = right;
        //cvCircle( debug_img, cvPointFrom32f(right->pt), 3, cvScalar(0,255-j*10,0), -1, 8, 0 );
        if( right->count == 2 )
            break;
        if( right->count != 3 || out_corner_count >= MAX(pattern_size.width,pattern_size.height) )
            EXIT;
        cur = right;
        for( k = 0; k < 4; k++ )
        {
            c = cur->neighbors[k];
            if( c && c->row > 0 )
            {
                for( kk = 0; kk < 4; kk++ )
                {
                    if( c->neighbors[kk] == below )
                        break;
                }
                if( kk < 4 )
                    below = c;
                else
                    right = c;
            }
        }
    }

    width = out_corner_count;
    if( width == pattern_size.width )
        height = pattern_size.height;
    else if( width == pattern_size.height )
        height = pattern_size.width;
    else
        EXIT;

    // find and store all the other rows
    for( i = 1; ; i++ )
    {
        if( !first )
            break;
        cur = first;
        first = 0;
        for( j = 0;; j++ )
        {
            cur->row = i;
            out_corners[out_corner_count++] = cur;
            //cvCircle( debug_img, cvPointFrom32f(cur->pt), 3, cvScalar(0,0,255-j*10), -1, 8, 0 );
            if( cur->count == 2 + (i < height-1) && j > 0 )
                break;

            right = 0;

            // find a neighbor that has not been processed yet
            // and that has a neighbor from the previous row
            for( k = 0; k < 4; k++ )
            {
                c = cur->neighbors[k];
                if( c && c->row > i )
                {
                    for( kk = 0; kk < 4; kk++ )
                    {
                        if( c->neighbors[kk] && c->neighbors[kk]->row == i-1 )
                            break;
                    }
                    if( kk < 4 )
                    {
                        right = c;
                        if( j > 0 )
                            break;
                    }
                    else if( j == 0 )
                        first = c;
                }
            }
            if( !right )
                EXIT;
            cur = right;
        }

        if( j != width - 1 )
            EXIT;
    }

    if( out_corner_count != corner_count )
        EXIT;

    // check if we need to transpose the board
    if( width != pattern_size.width )
    {
        CV_SWAP( width, height, k );

        memcpy( corners, out_corners, corner_count*sizeof(corners[0]) );
        for( i = 0; i < height; i++ )
            for( j = 0; j < width; j++ )
                out_corners[i*width + j] = corners[j*height + i];
    }

    // check if we need to revert the order in each row
    {
        CvPoint2D32f p0 = out_corners[0]->pt, p1 = out_corners[pattern_size.width-1]->pt,
                     p2 = out_corners[pattern_size.width]->pt;
        if( (p1.x - p0.x)*(p2.y - p1.y) - (p1.y - p0.y)*(p2.x - p1.x) < 0 )
        {
            if( width % 2 == 0 )
            {
                for( i = 0; i < height; i++ )
                    for( j = 0; j < width/2; j++ )
                        CV_SWAP( out_corners[i*width+j], out_corners[i*width+width-j-1], c );
            }
            else
            {
                for( j = 0; j < width; j++ )
                    for( i = 0; i < height/2; i++ )
                        CV_SWAP( out_corners[i*width+j], out_corners[(height - i - 1)*width+j], c );
            }
        }
    }

    result = corner_count;

    __END__;

    if( result <= 0 && corners )
    {
        corner_count = MIN( corner_count, pattern_size.width*pattern_size.height );
        for( i = 0; i < corner_count; i++ )
            out_corners[i] = corners[i];
        result = -corner_count;
    }

    cvFree( &corners );

    return result;
}

//=====================================================================================

static void icvFindQuadNeighbors( CvCBQuad *quads, int quad_count )
{
    const float thresh_scale = 1.f;
    int idx, i, k, j;
    float dx, dy, dist;

    // find quad neighbors
    for( idx = 0; idx < quad_count; idx++ )
    {
        CvCBQuad* cur_quad = &quads[idx];

        // choose the points of the current quadrangle that are close to
        // some points of the other quadrangles
        // (it can happen for split corners (due to dilation) of the
        // checker board). Search only in other quadrangles!

        // for each corner of this quadrangle
        for( i = 0; i < 4; i++ )
        {
            CvPoint2D32f pt;
            float min_dist = FLT_MAX;
            int closest_corner_idx = -1;
            CvCBQuad *closest_quad = 0;
            CvCBCorner *closest_corner = 0;

            if( cur_quad->neighbors[i] )
                continue;

            pt = cur_quad->corners[i]->pt;

            // find the closest corner in all other quadrangles
            for( k = 0; k < quad_count; k++ )
            {
                if( k == idx )
                    continue;

                for( j = 0; j < 4; j++ )
                {
                    if( quads[k].neighbors[j] )
                        continue;

                    dx = pt.x - quads[k].corners[j]->pt.x;
                    dy = pt.y - quads[k].corners[j]->pt.y;
                    dist = dx * dx + dy * dy;

                    if( dist < min_dist &&
                        dist <= cur_quad->edge_len*thresh_scale &&
                        dist <= quads[k].edge_len*thresh_scale )
                    {
                        closest_corner_idx = j;
                        closest_quad = &quads[k];
                        min_dist = dist;
                    }
                }
            }

            // we found a matching corner point?
            if( closest_corner_idx >= 0 && min_dist < FLT_MAX )
            {
                // If another point from our current quad is closer to the found corner
                // than the current one, then we don't count this one after all.
                // This is necessary to support small squares where otherwise the wrong
                // corner will get matched to closest_quad;
                closest_corner = closest_quad->corners[closest_corner_idx];

                for( j = 0; j < 4; j++ )
                {
                    if( cur_quad->neighbors[j] == closest_quad )
                        break;

                    dx = closest_corner->pt.x - cur_quad->corners[j]->pt.x;
                    dy = closest_corner->pt.y - cur_quad->corners[j]->pt.y;

                    if( dx * dx + dy * dy < min_dist )
                        break;
                }

                if( j < 4 || cur_quad->count >= 4 || closest_quad->count >= 4 )
                    continue;

                // Check that each corner is a neighbor of different quads
                for( j = 0; j < closest_quad->count; j++ )
                {
                    if( closest_quad->neighbors[j] == cur_quad )
                        break;
                }
                if( j < closest_quad->count )
                    continue;

                // check whether the closest corner to closest_corner
                // is different from cur_quad->corners[i]->pt
                for( k = 0; k < quad_count; k++ )
                {
                    CvCBQuad* q = &quads[k];
                    if( k == idx || q == closest_quad )
                        continue;

                    for( j = 0; j < 4; j++ )
                        if( !q->neighbors[j] )
                        {
                            dx = closest_corner->pt.x - q->corners[j]->pt.x;
                            dy = closest_corner->pt.y - q->corners[j]->pt.y;
                            dist = dx*dx + dy*dy;
                            if( dist < min_dist )
                                break;
                        }
                    if( j < 4 )
                        break;
                }

                if( k < quad_count )
                    continue;

                closest_corner->pt.x = (pt.x + closest_corner->pt.x) * 0.5f;
                closest_corner->pt.y = (pt.y + closest_corner->pt.y) * 0.5f;

                // We've found one more corner - remember it
                cur_quad->count++;
                cur_quad->neighbors[i] = closest_quad;
                cur_quad->corners[i] = closest_corner;

                closest_quad->count++;
                closest_quad->neighbors[closest_corner_idx] = cur_quad;
            }
        }
    }
}

//=====================================================================================

static int
icvGenerateQuads( CvCBQuad **out_quads, CvCBCorner **out_corners,
                  CvMemStorage *storage, CvMat *image, int flags )
{
    int quad_count = 0;
    CvMemStorage *temp_storage = 0;

    if( out_quads )
        *out_quads = 0;

    if( out_corners )
        *out_corners = 0;

    CV_FUNCNAME( "icvGenerateQuads" );

    __BEGIN__;

    CvSeq *src_contour = 0;
    CvSeq *root;
    CvContourEx* board = 0;
    CvContourScanner scanner;
    int i, idx, min_size;

    CV_ASSERT( out_corners && out_quads );

    // empiric bound for minimal allowed perimeter for squares
    min_size = cvRound( image->cols * image->rows * .03 * 0.01 * 0.92 );

    // create temporary storage for contours and the sequence of pointers to found quadrangles
    CV_CALL( temp_storage = cvCreateChildMemStorage( storage ));
    CV_CALL( root = cvCreateSeq( 0, sizeof(CvSeq), sizeof(CvSeq*), temp_storage ));

    // initialize contour retrieving routine
    CV_CALL( scanner = cvStartFindContours( image, temp_storage, sizeof(CvContourEx),
                                            CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE ));

    // get all the contours one by one
    while( (src_contour = cvFindNextContour( scanner )) != 0 )
    {
        CvSeq *dst_contour = 0;
        CvRect rect = ((CvContour*)src_contour)->rect;

        // reject contours with too small perimeter
        if( CV_IS_SEQ_HOLE(src_contour) && rect.width*rect.height >= min_size )
        {
            const int min_approx_level = 2, max_approx_level = MAX_CONTOUR_APPROX;
            int approx_level;
            for( approx_level = min_approx_level; approx_level <= max_approx_level; approx_level++ )
            {
                dst_contour = cvApproxPoly( src_contour, sizeof(CvContour), temp_storage,
                                            CV_POLY_APPROX_DP, (float)approx_level );
                // we call this again on its own output, because sometimes
                // cvApproxPoly() does not simplify as much as it should.
                dst_contour = cvApproxPoly( dst_contour, sizeof(CvContour), temp_storage,
                                            CV_POLY_APPROX_DP, (float)approx_level );

                if( dst_contour->total == 4 )
                    break;
            }

            // reject non-quadrangles
            if( dst_contour->total == 4 && cvCheckContourConvexity(dst_contour) )
            {
                CvPoint pt[4];
                double d1, d2, p = cvContourPerimeter(dst_contour);
                double area = fabs(cvContourArea(dst_contour, CV_WHOLE_SEQ));
                double dx, dy;

                for( i = 0; i < 4; i++ )
                    pt[i] = *(CvPoint*)cvGetSeqElem(dst_contour, i);

                dx = pt[0].x - pt[2].x;
                dy = pt[0].y - pt[2].y;
                d1 = sqrt(dx*dx + dy*dy);

                dx = pt[1].x - pt[3].x;
                dy = pt[1].y - pt[3].y;
                d2 = sqrt(dx*dx + dy*dy);

                // philipg.  Only accept those quadrangles which are more square
                // than rectangular and which are big enough
                double d3, d4;
                dx = pt[0].x - pt[1].x;
                dy = pt[0].y - pt[1].y;
                d3 = sqrt(dx*dx + dy*dy);
                dx = pt[1].x - pt[2].x;
                dy = pt[1].y - pt[2].y;
                d4 = sqrt(dx*dx + dy*dy);
                if( !(flags & CV_CALIB_CB_FILTER_QUADS) ||
                    d3*4 > d4 && d4*4 > d3 && d3*d4 < area*1.5 && area > min_size &&
                    d1 >= 0.15 * p && d2 >= 0.15 * p )
                {
                    CvContourEx* parent = (CvContourEx*)(src_contour->v_prev);
                    parent->counter++;
                    if( !board || board->counter < parent->counter )
                        board = parent;
                    dst_contour->v_prev = (CvSeq*)parent;
                    //for( i = 0; i < 4; i++ ) cvLine( debug_img, pt[i], pt[(i+1)&3], cvScalar(200,255,255), 1, CV_AA, 0 );
                    cvSeqPush( root, &dst_contour );
                }
            }
        }
    }

    // finish contour retrieving
    cvEndFindContours( &scanner );

    // allocate quad & corner buffers
    CV_CALL( *out_quads = (CvCBQuad*)cvAlloc(root->total * sizeof((*out_quads)[0])));
    CV_CALL( *out_corners = (CvCBCorner*)cvAlloc(root->total * 4 * sizeof((*out_corners)[0])));

    // Create array of quads structures
    for( idx = 0; idx < root->total; idx++ )
    {
        CvCBQuad* q = &(*out_quads)[quad_count];
        src_contour = *(CvSeq**)cvGetSeqElem( root, idx );
        if( (flags & CV_CALIB_CB_FILTER_QUADS) && src_contour->v_prev != (CvSeq*)board )
            continue;

        // reset group ID
        memset( q, 0, sizeof(*q) );
        q->group_idx = -1;
        assert( src_contour->total == 4 );
        for( i = 0; i < 4; i++ )
        {
            CvPoint2D32f pt = cvPointTo32f(*(CvPoint*)cvGetSeqElem(src_contour, i));
            CvCBCorner* corner = &(*out_corners)[quad_count*4 + i];

            memset( corner, 0, sizeof(*corner) );
            corner->pt = pt;
            q->corners[i] = corner;
        }
        q->edge_len = FLT_MAX;
        for( i = 0; i < 4; i++ )
        {
            float dx = q->corners[i]->pt.x - q->corners[(i+1)&3]->pt.x;
            float dy = q->corners[i]->pt.y - q->corners[(i+1)&3]->pt.y;
            float d = dx*dx + dy*dy;
            if( q->edge_len > d )
                q->edge_len = d;
        }
        quad_count++;
    }

    __END__;

    if( cvGetErrStatus() < 0 )
    {
        if( out_quads )
            cvFree( out_quads );
        if( out_corners )
            cvFree( out_corners );
        quad_count = 0;
    }

    cvReleaseMemStorage( &temp_storage );
    return quad_count;
}


CV_IMPL void
cvDrawChessboardCorners( CvArr* _image, CvSize pattern_size,
                         CvPoint2D32f* corners, int count, int found )
{
    CV_FUNCNAME( "cvDrawChessboardCorners" );

    __BEGIN__;

    const int shift = 0;
    const int radius = 4;
    const int r = radius*(1 << shift);
    int i;
    CvMat stub, *image;
    double scale = 1;
    int type, cn, line_type;

    CV_CALL( image = cvGetMat( _image, &stub ));

    type = CV_MAT_TYPE(image->type);
    cn = CV_MAT_CN(type);
    if( cn != 1 && cn != 3 && cn != 4 )
        CV_ERROR( CV_StsUnsupportedFormat, "Number of channels must be 1, 3 or 4" );

    switch( CV_MAT_DEPTH(image->type) )
    {
    case CV_8U:
        scale = 1;
        break;
    case CV_16U:
        scale = 256;
        break;
    case CV_32F:
        scale = 1./255;
        break;
    default:
        CV_ERROR( CV_StsUnsupportedFormat,
            "Only 8-bit, 16-bit or floating-point 32-bit images are supported" );
    }

    line_type = type == CV_8UC1 || type == CV_8UC3 ? CV_AA : 8;

    if( !found )
    {
        CvScalar color = {{0,0,255}};
        if( cn == 1 )
            color = cvScalarAll(200);
        color.val[0] *= scale;
        color.val[1] *= scale;
        color.val[2] *= scale;
        color.val[3] *= scale;

        for( i = 0; i < count; i++ )
        {
            CvPoint pt;
            pt.x = cvRound(corners[i].x*(1 << shift));
            pt.y = cvRound(corners[i].y*(1 << shift));
            cvLine( image, cvPoint( pt.x - r, pt.y - r ),
                    cvPoint( pt.x + r, pt.y + r ), color, 1, line_type, shift );
            cvLine( image, cvPoint( pt.x - r, pt.y + r),
                    cvPoint( pt.x + r, pt.y - r), color, 1, line_type, shift );
            cvCircle( image, pt, r+(1<<shift), color, 1, line_type, shift );
        }
    }
    else
    {
        int x, y;
        CvPoint prev_pt = {0, 0};
        const int line_max = 7;
        static const CvScalar line_colors[line_max] =
        {
            {{0,0,255}},
            {{0,128,255}},
            {{0,200,200}},
            {{0,255,0}},
            {{200,200,0}},
            {{255,0,0}},
            {{255,0,255}}
        };

        for( y = 0, i = 0; y < pattern_size.height; y++ )
        {
            CvScalar color = line_colors[y % line_max];
            if( cn == 1 )
                color = cvScalarAll(200);
            color.val[0] *= scale;
            color.val[1] *= scale;
            color.val[2] *= scale;
            color.val[3] *= scale;

            for( x = 0; x < pattern_size.width; x++, i++ )
            {
                CvPoint pt;
                pt.x = cvRound(corners[i].x*(1 << shift));
                pt.y = cvRound(corners[i].y*(1 << shift));

                if( i != 0 )
                    cvLine( image, prev_pt, pt, color, 1, line_type, shift );

                cvLine( image, cvPoint(pt.x - r, pt.y - r),
                        cvPoint(pt.x + r, pt.y + r), color, 1, line_type, shift );
                cvLine( image, cvPoint(pt.x - r, pt.y + r),
                        cvPoint(pt.x + r, pt.y - r), color, 1, line_type, shift );
                cvCircle( image, pt, r+(1<<shift), color, 1, line_type, shift );
                prev_pt = pt;
            }
        }
    }

    __END__;
}


/* End of file. */
