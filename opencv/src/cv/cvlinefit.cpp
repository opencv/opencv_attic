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
#include "_cvutils.h"

#include <math.h>

#define _FP double

#if _MSC_VER >= 1200
#pragma warning (disable:4701)
#endif

IPCVAPI( CvStatus, icvFitLine2D, (CvPoint2D32f* points, int count, CvDisType dist, 
                                  void *param, float reps, float aeps, float* line ))

IPCVAPI( CvStatus, icvFitLine3D, (CvPoint3D32f* points, int count, CvDisType dist, 
                                  void *param, float reps, float aeps, float* line ))

CvStatus
icvFitLine2D_wods( CvPoint2D32f * points, int _count, float *weights, float *line )
{
    _FP x = 0, y = 0, x2 = 0, y2 = 0, xy = 0, w = 0;
    _FP dx2, dy2, dxy;
    int i;
    int count = _count;
    float t;

    /* Calculating the average of x and y... */

    if( weights == 0 )
    {
        for( i = 0; i < count; i += 1 )
        {
            x += points[i].x;
            y += points[i].y;
            x2 += points[i].x * points[i].x;
            y2 += points[i].y * points[i].y;
            xy += points[i].x * points[i].y;
        }
        w = (float) count;
    }
    else
    {
        for( i = 0; i < count; i += 1 )
        {
            x += weights[i] * points[i].x;
            y += weights[i] * points[i].y;
            x2 += weights[i] * points[i].x * points[i].x;
            y2 += weights[i] * points[i].y * points[i].y;
            xy += weights[i] * points[i].x * points[i].y;
            w += weights[i];
        }
    }

    x /= w;
    y /= w;
    x2 /= w;
    y2 /= w;
    xy /= w;

    dx2 = x2 - x * x;
    dy2 = y2 - y * y;
    dxy = xy - x * y;

    t = (float) atan2( 2 * dxy, dx2 - dy2 ) / 2;
    line[0] = (float) cos( t );
    line[1] = (float) sin( t );

    line[2] = (float) x;
    line[3] = (float) y;

    return CV_NO_ERR;
}

CvStatus
icvFitLine3D_wods( CvPoint3D32f * points, int count, float *weights, float *line )
{
    int i;
    CvStatus ret;
    float w0 = 0;
    float x0 = 0, y0 = 0, z0 = 0;
    float x2 = 0, y2 = 0, z2 = 0, xy = 0, yz = 0, xz = 0;
    float dx2, dy2, dz2, dxy, dxz, dyz;
    float *v;
    float n;
    float det[9], evc[9], evl[3];

    memset( evl, 0, 3*sizeof(evl[0]));
    memset( evc, 0, 9*sizeof(evl[0]));

    if( weights )
    {
        for( i = 0; i < count; i++ )
        {
            float x = points[i].x;
            float y = points[i].y;
            float z = points[i].z;
            float w = weights[i];


            x2 += x * x * w;
            xy += x * y * w;
            xz += x * z * w;
            y2 += y * y * w;
            yz += y * z * w;
            z2 += z * z * w;
            x0 += x * w;
            y0 += y * w;
            z0 += z * w;
            w0 += w;
        }
    }
    else
    {
        for( i = 0; i < count; i++ )
        {
            float x = points[i].x;
            float y = points[i].y;
            float z = points[i].z;

            x2 += x * x;
            xy += x * y;
            xz += x * z;
            y2 += y * y;
            yz += y * z;
            z2 += z * z;
            x0 += x;
            y0 += y;
            z0 += z;
        }
        w0 = (float) count;
    }

    x2 /= w0;
    xy /= w0;
    xz /= w0;
    y2 /= w0;
    yz /= w0;
    z2 /= w0;

    x0 /= w0;
    y0 /= w0;
    z0 /= w0;

    dx2 = x2 - x0 * x0;
    dxy = xy - x0 * y0;
    dxz = xz - x0 * z0;
    dy2 = y2 - y0 * y0;
    dyz = yz - y0 * z0;
    dz2 = z2 - z0 * z0;

    det[0] = dz2 + dy2;
    det[1] = -dxy;
    det[2] = -dxz;
    det[3] = det[1];
    det[4] = dx2 + dz2;
    det[5] = -dyz;
    det[6] = det[2];
    det[7] = det[5];
    det[8] = dy2 + dx2;

    /* Searching for a eigenvector of det corresponding to the minimal eigenvalue */
    ret = icvJacobiEigens_32f( det, evc, evl, 3, 0 );
    if( ret != CV_NO_ERR )
    {
        return ret;
    }

    i = evl[0] < evl[1] ? (evl[0] < evl[2] ? 0 : 2) : (evl[1] < evl[2] ? 1 : 2);
    v = &evc[i * 3];
    n = (float) sqrt( v[0] * v[0] + v[1] * v[1] + v[2] * v[2] );
    line[0] = v[0] / n;
    line[1] = v[1] / n;
    line[2] = v[2] / n;
    line[3] = x0;
    line[4] = y0;
    line[5] = z0;

    return CV_NO_ERR;
}

void
icvCalcDist2D( CvPoint2D32f * points, int count, float *_line, float *dist )
{
    int j;
    float px = _line[2], py = _line[3];
    float nx = _line[1], ny = -_line[0];

    for( j = 0; j < count; j++ )
    {
        float x, y;

        x = points[j].x - px;
        y = points[j].y - py;

        dist[j] = (float) fabs( nx * x + ny * y );
    }
}

void
icvCalcDist3D( CvPoint3D32f * points, int count, float *_line, float *dist )
{
    int j;
    float px = _line[3], py = _line[4], pz = _line[5];
    float vx = _line[0], vy = _line[1], vz = _line[2];

    for( j = 0; j < count; j++ )
    {
        float x, y, z;

        x = points[j].x - px;
        y = points[j].y - py;
        z = points[j].z - pz;

        dist[j] =
            (float) sqrt( (vy * z - vz * y) * (vy * z - vz * y) +
                          (vz * x - vx * z) * (vz * x - vx * z) + (vx * y - vy * x) * (vx * y -
                                                                                       vy *
                                                                                       x) );
    }
}

void
icvWeightL1( float *d, int count, float *w )
{
    int i;

    for( i = 0; i < count; i++ )
    {
        w[i] = 1 / (float) fabs( (double) d[i] );
    }
}

void
icvWeightL12( float *d, int count, float *w )
{
    int i;

    for( i = 0; i < count; i++ )
    {
        w[i] = 1.0f / (float) sqrt( 1 + (double) (d[i] * d[i] / 2) );
    }
}

void
icvWeightFair( float *d, int count, float *w, float _c )
{
    int i;
    const float c = _c == 0 ? 1 / 1.3998f : 1 / _c;

    for( i = 0; i < count; i++ )
    {
        w[i] = 1 / (1 + d[i] * c);
    }
}

void
icvWeightWelsch( float *d, int count, float *w, float _c )
{
    int i;
    const float c = _c == 0 ? 1 / 2.9846f : 1 / _c;

    for( i = 0; i < count; i++ )
    {
        w[i] = (float) exp( -d[i] * d[i] * c * c );
    }
}


/* Takes an array of 2D points, type of distance (including user-defined 
distance specified by callbacks, fills the array of four floats with line
parameters A, B, C, D, where (A, B) is the normalized direction vector,
(C, D) is the point that belongs to the line. */

IPCVAPI_IMPL( CvStatus, icvFitLine2D, (CvPoint2D32f * points, int count, CvDisType dist,
                                       void *param, float reps, float aeps, float *line) )
{
    void (*calc_weights) (float *, int, float *) = 0;
    void (*calc_weights_param) (float *, int, float *, float) = 0;
    float *w;                   /* weights */
    float *r;                   /* square distances */
    int i;
    float _line[6], _lineprev[6];
    int first = 1;
    float rdelta = reps != 0 ? reps : 1.0f;
    float adelta = aeps != 0 ? aeps : 0.01f;
    CvStatus ret;
    union
    {
        void *p;
        void (*fp) (float *, int, float *);
    }
    _PFP;
    float _param = 0;

    if( points == 0 || (dist == CV_DIST_USER && param == 0) || line == 0 )
    {
        return CV_NULLPTR_ERR;
    }

    if( count < 0 || reps < 0 || aeps < 0 )
    {
        return CV_BADFACTOR_ERR;
    }

    switch (dist)
    {
    case CV_DIST_L2:
        return icvFitLine2D_wods( points, count, 0, line );

    case CV_DIST_L1:
        calc_weights = icvWeightL1;
        break;

    case CV_DIST_L12:
        calc_weights = icvWeightL12;
        break;

    case CV_DIST_FAIR:
        calc_weights_param = icvWeightFair;
        _param = param ? *(float *) param : 0;
        break;

    case CV_DIST_WELSCH:
        calc_weights_param = icvWeightWelsch;
        _param = param ? *(float *) param : 0;
        break;

    case CV_DIST_USER:
        _PFP.p = param;
        calc_weights = (void ( * )(float *, int, float *)) _PFP.fp;
        break;

    default:
        return CV_BADFACTOR_ERR;
    }

    w = (float *) icvAlloc( count * sizeof( float ));
    r = (float *) icvAlloc( count * sizeof( float ));

    for( i = 0; i < count; i++ )
        w[i] = 1.0f;

    ret = icvFitLine2D_wods( points, count, 0, _line );
    for( i = 0; i < 100; i++ )
    {
        if( ret != CV_NO_ERR )
        {
            icvFree( &w );
            icvFree( &r );
            return ret;
        }

        if( first )
        {
            first = 0;
        }
        else
        {
            if( fabs( acos( (float) (_line[0] * _lineprev[0] + _line[1] * _lineprev[1]) ))
                < adelta )
            {
                float x, y, d;

                x = (float) fabs( _line[2] - _lineprev[2] );
                y = (float) fabs( _line[3] - _lineprev[3] );

                d = x > y ? x : y;
                if( d < rdelta )
                {
                    // Return...
                    memcpy( line, _line, 4 * sizeof( float ));

                    icvFree( &w );
                    icvFree( &r );
                    return CV_NO_ERR;
                }

            }
        }
        /* calculate distances */
        icvCalcDist2D( points, count, _line, r );

        /* calculate weights */
        if( calc_weights )
        {
            calc_weights( r, count, w );
        }
        else if( calc_weights_param )
        {
            calc_weights_param( r, count, w, _param );
        }
        else
        {
            assert( 0 );
            return CV_BADFACTOR_ERR;
        }


        /* save the line parameters */
        memcpy( _lineprev, _line, 4 * sizeof( float ));

        /* Run again... */
        ret = icvFitLine2D_wods( points, count, w, _line );
    }

    icvFree( &w );
    icvFree( &r );
    return CV_BADCONVERGENCE_ERR;
}


/* Takes an array of 3D points, type of distance (including user-defined 
distance specified by callbacks, fills the array of four floats with line 
parameters A, B, C, D, E, F, where (A, B, C) is the normalized direction vector, 
(D, E, F) is the point that belongs to the line. */

IPCVAPI_IMPL( CvStatus, icvFitLine3D, (CvPoint3D32f * points, int count, CvDisType dist,
                                       void *param, float reps, float aeps, float *line) )
{
    void (*calc_weights) (float *, int, float *) = 0;
    void (*calc_weights_param) (float *, int, float *, float) = 0;
    float *w;                   /* weights */
    float *r;                   /* square distances */
    int i;
    float _line[6], _lineprev[6];
    int first = 1;
    float rdelta = reps != 0 ? reps : 1.0f;
    float adelta = aeps != 0 ? aeps : 0.01f;
    CvStatus ret;
    union
    {
        void *p;
        void (*fp) (float *, int, float *);
    }
    _PFP;
    float _param = 0;

    if( points == 0 || (dist == CV_DIST_USER && param == 0) || line == 0 )
    {
        return CV_NULLPTR_ERR;
    }

    if( count < 0 || reps < 0 || aeps < 0 )
    {
        return CV_BADFACTOR_ERR;
    }

    switch (dist)
    {
    case CV_DIST_L2:
        return icvFitLine3D_wods( points, count, 0, line );

    case CV_DIST_L1:
        calc_weights = icvWeightL1;
        break;

    case CV_DIST_L12:
        calc_weights = icvWeightL12;
        break;

    case CV_DIST_FAIR:
        calc_weights_param = icvWeightFair;
        _param = param ? *(float *) param : 0;
        break;

    case CV_DIST_WELSCH:
        calc_weights_param = icvWeightWelsch;
        _param = param ? *(float *) param : 0;
        break;

    case CV_DIST_USER:
        _PFP.p = param;
        calc_weights = (void ( * )(float *, int, float *)) _PFP.fp;
        break;

    default:
        return CV_BADFACTOR_ERR;
    }

    w = (float *) icvAlloc( count * sizeof( float ));
    r = (float *) icvAlloc( count * sizeof( float ));

    for( i = 0; i < count; i++ )
        w[i] = 1.0f;

    ret = icvFitLine3D_wods( points, count, 0, _line );
    for( i = 0; i < 100; i++ )
    {
        if( ret != CV_NO_ERR )
        {
            icvFree( &w );
            icvFree( &r );
            return ret;
        }

        if( first )
        {
            first = 0;
        }
        else
        {
            if( fabs
                ( acos
                  ( (float)
                    (_line[0] * _lineprev[0] + _line[1] * _lineprev[1] +
                     _line[2] * _lineprev[2]) )) < adelta )
            {
                float x, y, z, ax, ay, az, dx, dy, dz, d;

                x = _line[3] - _lineprev[3];
                y = _line[4] - _lineprev[4];
                z = _line[5] - _lineprev[5];
                ax = _line[0] - _lineprev[0];
                ay = _line[1] - _lineprev[1];
                az = _line[2] - _lineprev[2];
                dx = (float) fabs( y * az - z * ay );
                dy = (float) fabs( z * ax - x * az );
                dz = (float) fabs( x * ay - y * ax );

                d = dx > dy ? (dx > dz ? dx : dz) : (dy > dz ? dy : dz);
                if( d < rdelta )
                {
                    // Return...
                    memcpy( line, _line, 6 * sizeof( float ));

                    icvFree( &w );
                    icvFree( &r );
                    return CV_NO_ERR;
                }

            }
        }
        /* calculate distances */
        icvCalcDist3D( points, count, _line, r );

        /* calculate weights */
        if( calc_weights )
        {
            calc_weights( r, count, w );
        }
        else if( calc_weights_param )
        {
            calc_weights_param( r, count, w, _param );
        }
        else
        {
            assert( 0 );
            return CV_BADFACTOR_ERR;
        }

        /* save the line parameters */
        memcpy( _lineprev, _line, 6 * sizeof( float ));

        /* Run again... */
        ret = icvFitLine3D_wods( points, count, w, _line );
    }

    icvFree( &w );
    icvFree( &r );
    return CV_BADCONVERGENCE_ERR;
}


CV_IMPL void
cvFitLine3D( CvPoint3D32f * points, int count, CvDisType dist,
             void *param, float reps, float aeps, float *line )
{
    CV_FUNCNAME( "cvFitLine3D" );

    __BEGIN__;

    if( points == 0 || line == 0 )
        CV_ERROR( IPL_BadDataPtr, "Null pointer." );

    if( count < 0 )
        CV_ERROR( IPL_StsBadArg, "Count must be positive." );

    IPPI_CALL( icvFitLine3D( points, count, dist, param, reps, aeps, line ));

    __CLEANUP__;
    __END__;
}


CV_IMPL void
cvFitLine2D( CvPoint2D32f * points, int count, CvDisType dist,
             void *param, float reps, float aeps, float *line )
{
    CV_FUNCNAME( "cvFitLine2D" );

    __BEGIN__;

    if( points == 0 || line == 0 )
        CV_ERROR( IPL_BadDataPtr, "Null pointer." );

    if( count < 0 )
        CV_ERROR( IPL_StsBadArg, "Count must be positive." );

    IPPI_CALL( icvFitLine2D( points, count, dist, param, reps, aeps, line ));

    __CLEANUP__;
    __END__;
}
