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
#include "_cvdatastructs.h"

/****************************************************************************************\

   calculate image homography 
   
\****************************************************************************************/

static CvStatus icvCalcImageHomography( float* line, CvPoint3D32f * center,
                                        float* intrinsic,
                                        float* homography )
{
    float norm_xy, norm_xz, xy_sina, xy_cosa, xz_sina, xz_cosa;
    double plane_dist;
    float rx[3], ry[3], rz[3];

/*    IppmMatr32f r_trans;       */
    float *r_trans;
    float *r_p;
    float ed[3][3] = { {1.f, 0.f, 0.f}, {0.f, 1.f, 0.f}, {0.f, 0.f, 1.f} };
    float *sub, *inv_int;
    float t_trans[3];
    float nx1, nx2, ny2, nz1;

    assert( line != NULL );
    if( line == NULL )
        return CV_NULLPTR_ERR;

    norm_xy = cvSqrt( line[0] * line[0] + line[1] * line[1] );
    xy_cosa = line[0] / norm_xy;
    xy_sina = line[1] / norm_xy;

    norm_xz = cvSqrt( line[0] * line[0] + line[2] * line[2] );
    xz_cosa = line[0] / norm_xz;
    xz_sina = line[2] / norm_xz;

/*  rotation around y axis  */
    nx1 = -xz_sina;
    nz1 = xz_cosa;

/*  rotation around z axis  */
    nx2 = xy_cosa * nx1;
    ny2 = xy_sina * nx1;

/*  normal of the arm plane   */
    norm_xz = cvSqrt( nx2 * nx2 + ny2 * ny2 + nz1 * nz1 );
    rz[0] = nx2 / norm_xz;
    rz[1] = ny2 / norm_xz;
    rz[2] = nz1 / norm_xz;

/*  new axe  x  */
    rx[0] = line[0];
    rx[1] = line[1];
    rx[2] = line[2];
/*  new axe  y  */
    icvCrossProduct2L_32f( &rz[0], &rx[0], &ry[0] );
    norm_xz = cvSqrt( ry[0] * ry[0] + ry[1] * ry[1] + ry[2] * ry[2] );
    ry[0] /= norm_xz;
    ry[1] /= norm_xz;
    ry[2] /= norm_xz;

/*  transpone rotation matrix    */
    r_trans = icvCreateMatrix_32f( 3, 3 );
    r_p = (float *) r_trans;
    r_p[0] = rx[0];
    r_p[1] = rx[1];
    r_p[2] = rx[2];
    r_p[3] = ry[0];
    r_p[4] = ry[1];
    r_p[5] = ry[2];
    r_p[6] = rz[0];
    r_p[7] = rz[1];
    r_p[8] = rz[2];

/*  calculate center distanse from arm plane  */
    plane_dist = icvDotProduct_32f( (float *) center, &rz[0], 3 );

    sub = icvCreateMatrix_32f( 3, 3 );

/*   calculate ed - r_trans   */
    icvSubMatrix_32f( &ed[0][0], r_trans, sub, 3, 3 );

/*   calculate (ed - r_trans)*center   */
//    t_trans =ippmCreateVector_32f (3);
    icvTransformVector_32f( sub, (float *) center, &t_trans[0], 3, 3 );

/*  calculate (t_trans*rz)/plane_dist   matrix  */

    sub[0] = t_trans[0] * rz[0];
    sub[1] = t_trans[0] * rz[1];
    sub[2] = t_trans[0] * rz[2];
    sub[3] = t_trans[1] * rz[0];
    sub[4] = t_trans[1] * rz[1];
    sub[5] = t_trans[1] * rz[2];
    sub[6] = t_trans[2] * rz[0];
    sub[7] = t_trans[2] * rz[1];
    sub[8] = t_trans[2] * rz[2];

    icvScaleVector_32f( sub, sub, 3, (float) (1. / plane_dist) );

/*  calculate  r_trans + (t_trans*rz)/plane_dist   matrix   */
    icvAddMatrix_32f( r_trans, sub, sub, 3, 3 );

/*   calculate  intrinsic * (r_trans + (t_trans*rz)/plane_dist)   matrix   */
    icvMulMatrix_32f( (float *) intrinsic, 3, 3, sub, 3, 3, sub );

/*   calculate  Homography matrix   */
    inv_int = icvCreateMatrix_32f( 3, 3 );
    icvInvertMatrix_32f( (float *) intrinsic, 3, inv_int );
    icvMulMatrix_32f( sub, 3, 3, inv_int, 3, 3, (float *) homography );

    icvDeleteMatrix( r_trans );
    icvDeleteMatrix( inv_int );
    icvDeleteMatrix( sub );

    return CV_OK;

}

/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:     cvCalcImageHomography
//    Purpose:  calculates the cooficients of the homography matrix
//    Context:   
//    Parameters: 
//      line   - pointer to the input 3D-line
//      center - pointer to the input hand center
//      intrinsic - intrinsic camera parameters matrix
//      homography - result homography matrix
//      
//    Notes:
//F*/
CV_IMPL void
cvCalcImageHomography( float *line, CvPoint3D32f * center,
                       float* intrinsic, float* homography )
{
    CV_FUNCNAME( "cvCalcImageHomography" );
    __BEGIN__;

    IPPI_CALL( icvCalcImageHomography( line, center, intrinsic, homography ));

    __END__;
}
