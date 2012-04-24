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
#include "_cvgeom.h"

typedef struct
{
    float x, y;
}
icvVector;

typedef struct
{
    int bottom;
    int left;
    float height;
    float width;
    float base_a;
    float base_b;

}
icvMinAreaState;

/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:    icvRotatingCalipers
//    Purpose:
//      Rotating calipers algorithm with some applications
//
//    Context:
//    Parameters:
//      points      - convex hull vertices ( any orientation )
//      n           - number of vertices
//      mode        - concrete application of algorithm 
//                    can be  _CV_CALIPERS_MAXHEIGHT   or   
//                            _CV_CALIPERS_MINAREARECT  
//      left, bottom, right, top - indexes of extremal points
//      out         - output info
//    Returns:
//    Notes:
//F*/

/* we will use usual cartesian coordinates */
CvStatus
icvRotatingCalipers( CvPoint * points, int n, int mode,
                     int left, int bottom, int right, int top, char *out )
{
    float minarea = FLT_MAX;
    /*float minarea = (float)  ((points[top].y - points[bottom].y) *
                             (points[right].x - points[left].x));
    */
    float max_dist = 0;
    char buffer[32];
    int i, k;

    int seq[4] = { -1, -1, -1, -1 };

    /* rotating calipers sides will always have coordinates    
       (a,b) (-b,a) (-a,-b) (b, -a)     
     */
    /* this is a first base bector (a,b) initialized by (1,0) */
    float orientation;
    float base_a;
    float base_b = 0;

    CvPoint2D32f *vect;
    float *inv_vect_length;

    vect = (CvPoint2D32f *) icvAlloc( n * sizeof( CvPoint2D32f ));
    if( vect == NULL )
        return CV_OUTOFMEM_ERR;

    inv_vect_length = (float *) icvAlloc( n * sizeof( float ));

    if( inv_vect_length == NULL )
    {
        icvFree( &vect );
        return CV_OUTOFMEM_ERR;
    }

    /* translate point sequense to vector sequense */
    for( i = 0; i < n - 1; i++ )
    {
        vect[i].x = (float) (points[i + 1].x - points[i].x);
        vect[i].y = (float) (points[i + 1].y - points[i].y);
        inv_vect_length[i] = cvInvSqrt( vect[i].x * vect[i].x + vect[i].y * vect[i].y );
    }
    vect[n - 1].x = (float) (points[0].x - points[n - 1].x);
    vect[n - 1].y = (float) (points[0].y - points[n - 1].y);
    inv_vect_length[n - 1] = cvInvSqrt( vect[n - 1].x * vect[n - 1].x +
                                        vect[n - 1].y * vect[n - 1].y );

    /* find convex hull orientation */
    {
        float ax = vect[0].x;
        float ay = vect[0].y;
        float bx = vect[1].x;
        float by = vect[1].y;

        float convexity = ax * by - ay * bx;

        assert( convexity != 0 );
        orientation = (convexity > 0) ? 1.f : (-1.f);
    }
    base_a = orientation;

/*****************************************************************************************/
/*                         init calipers position                                        */
    seq[0] = bottom;
    seq[1] = right;
    seq[2] = top;
    seq[3] = left;
/*****************************************************************************************/
/*                         Main loop - evaluate angles and rotate calipers               */

    /* all of edges will be checked while rotating calipers by 90 degrees */
    for( k = 0; k < n; k++ )
    {
        /* sinus of minimal angle */
        float sinus;

        /* compute cosinus of angle between calipers side and poligon edge */
        /* dp - dot product */
        float dp0 = base_a * vect[seq[0]].x + base_b * vect[seq[0]].y;
        float dp1 = -base_b * vect[seq[1]].x + base_a * vect[seq[1]].y;
        float dp2 = -base_a * vect[seq[2]].x - base_b * vect[seq[2]].y;
        float dp3 = base_b * vect[seq[3]].x - base_a * vect[seq[3]].y;

        float cosalpha = dp0 * inv_vect_length[seq[0]];
        float maxcos = cosalpha;

        /* number of calipers edges, that has minimal angle with edge */
        int main_element = 0;

        /* choose minimal angle */
        cosalpha = dp1 * inv_vect_length[seq[1]];
        maxcos = (cosalpha > maxcos) ? (main_element = 1, cosalpha) : maxcos;
        cosalpha = dp2 * inv_vect_length[seq[2]];
        maxcos = (cosalpha > maxcos) ? (main_element = 2, cosalpha) : maxcos;
        cosalpha = dp3 * inv_vect_length[seq[3]];
        maxcos = (cosalpha > maxcos) ? (main_element = 3, cosalpha) : maxcos;

        sinus = orientation * cvSqrt( 1 - maxcos * maxcos );

        /* rotate calipers */
        {
            float x = base_a;
            float y = base_b;

            base_a = maxcos * x - sinus * y;
            base_b = sinus * x + maxcos * y;
        }

        /* change base point of main edge */
        seq[main_element] += 1;
        seq[main_element] = (seq[main_element] == n) ? 0 : seq[main_element];

        switch (mode)
        {
        case CV_CALIPERS_MAXHEIGHT:
            {
                /* now main element lies on edge alligned to calipers side */

                /* find opposite element i.e. transform  */
                /* 0->2, 1->3, 2->0, 3->1                */
                int opposite_el = main_element ^ 2;

                int dx = points[seq[opposite_el]].x - points[seq[main_element]].x;
                int dy = points[seq[opposite_el]].y - points[seq[main_element]].y;

                float dist;

                if( main_element & 1 )
                {
                    dist = dx * base_a + dy * base_b;
                }
                else
                {
                    dist = dx * (-base_b) + dy * base_a;
                }

                dist = (float) fabs( dist );

                if( dist > max_dist )
                {

                    max_dist = dist;

                    /*temporary */
/*
                    ((CvPoint*)out)[0] = points[(seq[main_element] + n - 1)%n];
                    ((CvPoint*)out)[1] = points[seq[main_element]];
                    ((CvPoint*)out)[2] = points[seq[opposite_el]];
*/
                }

                break;
            }
        case CV_CALIPERS_MINAREARECT:
            /* find area of rectangle */
            {
                float height;
                float area;

                /* find vector left-right */
                int dx = points[seq[1]].x - points[seq[3]].x;
                int dy = points[seq[1]].y - points[seq[3]].y;

                /* dotproduct */
                float width = dx * base_a + dy * base_b;

                /* find vector left-right */
                dx = points[seq[2]].x - points[seq[0]].x;
                dy = points[seq[2]].y - points[seq[0]].y;

                /* dotproduct */
                height = -dx * base_b + dy * base_a;

                area = width * height;
                if( area <= minarea )
                {
                    float *buf = (float *) buffer;

                    minarea = area;
                    /* leftist point */
                    ((int *) buf)[0] = seq[3];
                    buf[1] = base_a;
                    buf[2] = width;
                    buf[3] = base_b;
                    buf[4] = height;
                    /* bottom point */
                    ((int *) buf)[5] = seq[0];
                    buf[6] = area;
                }
                break;
            }
        }                       /*switch */

    }                           /* for */
    switch (mode)
    {
    case CV_CALIPERS_MINAREARECT:
        {
            float *buf = (float *) buffer;

            float A1 = buf[1];
            float B1 = buf[3];

            float A2 = -buf[3];
            float B2 = buf[1];

            float C1 = A1 * points[((int *) buf)[0]].x + points[((int *) buf)[0]].y * B1;
            float C2 = A2 * points[((int *) buf)[5]].x + points[((int *) buf)[5]].y * B2;

            float idet = 1.f / (A1 * B2 - A2 * B1);

            float px = (C1 * B2 - C2 * B1) * idet;
            float py = (A1 * C2 - A2 * C1) * idet;

            ((float *) out)[0] = px;
            ((float *) out)[1] = py;

            ((float *) out)[2] = A1 * buf[2];
            ((float *) out)[3] = B1 * buf[2];

            ((float *) out)[4] = A2 * buf[4];
            ((float *) out)[5] = B2 * buf[4];

//            ((float *) out)[6] = buf[6];

        }
        break;
    case CV_CALIPERS_MAXHEIGHT:
        {
            *((float *) out) = max_dist;
        }
        break;
    }

    icvFree( &vect );
    icvFree( &inv_vect_length );

    return CV_OK;
}

IPCVAPI_IMPL( CvStatus, icvMinAreaRect, (CvPoint * points, int n,
                                         int left, int bottom, int right, int top,
                                         CvPoint2D32f * anchor,
                                         CvPoint2D32f * vect1, CvPoint2D32f * vect2) )

/*CvStatus icvMinAreaRect(CvPoint * points, int n,
                           int left, int bottom, int right, int top,
                           CvPoint2D32f * anchor,
                           CvPoint2D32f * vect1, 
                           CvPoint2D32f * vect2)*/
{
    /* check left, bottom, right, top
       if they all == -1 - compute */
    if( left == -1 && bottom == -1 && right == -1 && top == -1 )
    {   
        left = bottom = right = top = 0;
        for( int i = 1; i < n; i++ )
        {
            left  = (points[i].x < points[left].x) ? i : left;
            right = (points[i].x > points[right].x) ? i : right;
            top = (points[i].y > points[top].y) ? i : top;
            bottom = (points[i].y < points[bottom].y) ? i : bottom;
        }
    }

    CvPoint2D32f out[3];
    CvStatus st = icvRotatingCalipers( points, n, CV_CALIPERS_MINAREARECT, left, bottom, right,
                                       top, (char *) out );

    *anchor = out[0];
    *vect1 = out[1];
    *vect2 = out[2];
    return st;
}

