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
#include <limits.h>
#include <float.h>
#include "_cvgeom.h"

int
icvIsPtInCircle3( CvPoint2D32f pt, CvPoint2D32f a, CvPoint2D32f b, CvPoint2D32f c )
{
    double val = (a.x * a.x + a.y * a.y) * cvTriangleArea( b, c, pt ) -
        (b.x * b.x + b.y * b.y) * cvTriangleArea( a, c, pt ) +
        (c.x * c.x + c.y * c.y) * cvTriangleArea( a, b, pt ) -
        (pt.x * pt.x + pt.y * pt.y) * cvTriangleArea( a, b, c );

    return val > FLT_EPSILON ? 1 : val < -FLT_EPSILON ? -1 : 0;
}


int
icvIntersectLines( double x1, double dx1, double y1, double dy1,
                   double x2, double dx2, double y2, double dy2, double *t2 )
{
    double d = dx1 * dy2 - dx2 * dy1;
    int result = -1;

    if( d != 0 )
    {
        *t2 = ((x2 - x1) * dy1 - (y2 - y1) * dx1) / d;
        result = 0;
    }
    return result;
}


void
icvCreateCenterNormalLine( CvSubdiv2DEdge edge, double *_a, double *_b, double *_c )
{
    CvPoint2D32f org = cvSubdiv2DEdgeOrg( edge )->pt;
    CvPoint2D32f dst = cvSubdiv2DEdgeDst( edge )->pt;

    double a = dst.x - org.x;
    double b = dst.y - org.y;
    double c = -(a * (dst.x + org.x) + b * (dst.y + org.y));

    *_a = a + a;
    *_b = b + b;
    *_c = c;
}


void
icvIntersectLines3( double *a0, double *b0, double *c0,
                    double *a1, double *b1, double *c1, CvPoint2D32f * point )
{
    double det = a0[0] * b1[0] - a1[0] * b0[0];

    if( det != 0 )
    {
        det = 1. / det;
        point->x = (float) ((b0[0] * c1[0] - b1[0] * c0[0]) * det);
        point->y = (float) ((a1[0] * c0[0] - a0[0] * c1[0]) * det);
    }
    else
    {
        point->x = point->y = FLT_MAX;
    }
}
