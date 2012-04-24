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

/*
    This is stright forward port v2 of Matlab calibration engine by Jean-Yves Bouguet
    that is (in a large extent) based on the paper:
    Z. Zhang. "A flexible new technique for camera calibration".
    IEEE Transactions on Pattern Analysis and Machine Intelligence, 22(11):1330-1334, 2000.

    The 1st initial port was done by Valery Mosyagin.
*/

static void
icvGaussNewton( const CvMat* J, const CvMat* err, CvMat* delta,
                CvMat* JtJ=0, CvMat* JtErr=0, CvMat* JtJW=0, CvMat* JtJV=0 )
{
    CvMat* _temp_JtJ = 0;
    CvMat* _temp_JtErr = 0;
    CvMat* _temp_JtJW = 0;
    CvMat* _temp_JtJV = 0;
    
    CV_FUNCNAME( "icvGaussNewton" );

    __BEGIN__;

    if( !CV_IS_MAT(J) || !CV_IS_MAT(err) || !CV_IS_MAT(delta) )
        CV_ERROR( CV_StsBadArg, "Some of required arguments is not a valid matrix" );

    if( !JtJ )
    {
        CV_CALL( _temp_JtJ = cvCreateMat( J->cols, J->cols, J->type ));
        JtJ = _temp_JtJ;
    }
    else if( !CV_IS_MAT(JtJ) )
        CV_ERROR( CV_StsBadArg, "JtJ is not a valid matrix" );

    if( !JtErr )
    {
        CV_CALL( _temp_JtErr = cvCreateMat( J->cols, 1, J->type ));
        JtErr = _temp_JtErr;
    }
    else if( !CV_IS_MAT(JtErr) )
        CV_ERROR( CV_StsBadArg, "JtErr is not a valid matrix" );

    if( !JtJW )
    {
        CV_CALL( _temp_JtJW = cvCreateMat( J->cols, 1, J->type ));
        JtJW = _temp_JtJW;
    }
    else if( !CV_IS_MAT(JtJW) )
        CV_ERROR( CV_StsBadArg, "JtJW is not a valid matrix" );

    if( !JtJV )
    {
        CV_CALL( _temp_JtJV = cvCreateMat( J->cols, J->cols, J->type ));
        JtJV = _temp_JtJV;
    }
    else if( !CV_IS_MAT(JtJV) )
        CV_ERROR( CV_StsBadArg, "JtJV is not a valid matrix" );

    cvMulTransposed( J, JtJ, 1 );
    cvGEMM( J, err, 1, 0, 0, JtErr, CV_GEMM_A_T );
    cvSVD( JtJ, JtJW, 0, JtJV, CV_SVD_MODIFY_A + CV_SVD_V_T );
    cvSVBkSb( JtJW, JtJV, JtJV, JtErr, delta, CV_SVD_U_T + CV_SVD_V_T );

    __END__;

    if( _temp_JtJ || _temp_JtErr || _temp_JtJW || _temp_JtJV )
    {
        cvReleaseMat( &_temp_JtJ );
        cvReleaseMat( &_temp_JtErr );
        cvReleaseMat( &_temp_JtJW );
        cvReleaseMat( &_temp_JtJV );
    }
}


/*static double calc_repr_err( const double* object_points, int o_step,
                             const double* image_points,
                             const double* h, int count )
{
    double err = 0;
    for( int i = 0; i < count; i++ )
    {
        double X = object_points[i*o_step], Y = object_points[i*o_step + 1];
        double x0 = image_points[i*2], y0 = image_points[i*2 + 1];
        double d = 1./(h[6]*X + h[7]*Y + h[8]);
        double x = (h[0]*X + h[1]*Y + h[2])*d;
        double y = (h[3]*X + h[4]*Y + h[5])*d;
        err += fabs(x - x0) + fabs(y - y0);
    }
    return err;
}*/


// finds perspective transformation H between the object plane and image plane,
// so that (sxi,syi,s) ~ H*(Xi,Yi,1)
CV_IMPL void
cvFindHomography( const CvMat* object_points, const CvMat* image_points, CvMat* __H )
{
    CvMat *_m = 0, *_M = 0;
    CvMat *_L = 0;
    
    CV_FUNCNAME( "cvFindHomography" );

    __BEGIN__;

    int h_type;
    int i, k, count, count2;
    CvPoint2D64f *m, *M;
    CvPoint2D64f cm = {0,0}, sm = {0,0};
    double inv_Hnorm[9] = { 0, 0, 0, 0, 0, 0, 0, 0, 1 };
    double H[9];
    CvMat _inv_Hnorm = cvMat( 3, 3, CV_64FC1, inv_Hnorm );
    CvMat _H = cvMat( 3, 3, CV_64FC1, H );
    double LtL[9*9], LW[9], LV[9*9];
    CvMat* _Lp;
    double* L;
    CvMat _LtL = cvMat( 9, 9, CV_64FC1, LtL );
    CvMat _LW = cvMat( 9, 1, CV_64FC1, LW );
    CvMat _LV = cvMat( 9, 9, CV_64FC1, LV );
    CvMat _Hrem = cvMat( 3, 3, CV_64FC1, LV + 8*9 );

    if( !CV_IS_MAT(image_points) || !CV_IS_MAT(object_points) || !CV_IS_MAT(__H) )
        CV_ERROR( CV_StsBadArg, "one of arguments is not a valid matrix" );

    h_type = CV_MAT_TYPE(__H->type);
    if( h_type != CV_32FC1 && h_type != CV_64FC1 )
        CV_ERROR( CV_StsUnsupportedFormat, "Homography matrix must have 32fC1 or 64fC1 type" );
    if( __H->rows != 3 || __H->cols != 3 )
        CV_ERROR( CV_StsBadSize, "Homography matrix must be 3x3" );

    count = MAX(image_points->cols, image_points->rows);
    count2 = MAX(object_points->cols, object_points->rows);
    if( count != count2 )
        CV_ERROR( CV_StsUnmatchedSizes, "Numbers of image and object points do not match" );

    CV_CALL( _m = cvCreateMat( 1, count, CV_64FC2 ));
    CV_CALL( cvConvertPointsHomogenious( image_points, _m ));
    m = (CvPoint2D64f*)_m->data.ptr;
    
    CV_CALL( _M = cvCreateMat( 1, count, CV_64FC2 ));
    CV_CALL( cvConvertPointsHomogenious( object_points, _M ));
    M = (CvPoint2D64f*)_M->data.ptr;

    // calculate the normalization transformation Hnorm.
    for( i = 0; i < count; i++ )
        cm.x += m[i].x, cm.y += m[i].y;
   
    cm.x /= count; cm.y /= count;

    for( i = 0; i < count; i++ )
    {
        double x = m[i].x - cm.x;
        double y = m[i].y - cm.y;
        sm.x += fabs(x); sm.y += fabs(y);
    }

    sm.x /= count; sm.y /= count;
    inv_Hnorm[0] = sm.x;
    inv_Hnorm[4] = sm.y;
    inv_Hnorm[2] = cm.x;
    inv_Hnorm[5] = cm.y;
    sm.x = 1./sm.x;
    sm.y = 1./sm.y;
    
    CV_CALL( _Lp = _L = cvCreateMat( 2*count, 9, CV_64FC1 ) );
    L = _L->data.db;

    for( i = 0; i < count; i++, L += 18 )
    {
        double x = -(m[i].x - cm.x)*sm.x, y = -(m[i].y - cm.y)*sm.y;
        L[0] = L[9 + 3] = M[i].x;
        L[1] = L[9 + 4] = M[i].y;
        L[2] = L[9 + 5] = 1;
        L[9 + 0] = L[9 + 1] = L[9 + 2] = L[3] = L[4] = L[5] = 0;
        L[6] = x*M[i].x;
        L[7] = x*M[i].y;
        L[8] = x;
        L[9 + 6] = y*M[i].x;
        L[9 + 7] = y*M[i].y;
        L[9 + 8] = y;
    }

    if( count > 4 )
    {
        cvMulTransposed( _L, &_LtL, 1 );
        _Lp = &_LtL;
    }

    _LW.rows = MIN(count*2, 9);
    cvSVD( _Lp, &_LW, 0, &_LV, CV_SVD_MODIFY_A + CV_SVD_V_T );
    cvScale( &_Hrem, &_Hrem, 1./_Hrem.data.db[8] );
    cvMatMul( &_inv_Hnorm, &_Hrem, &_H );

    if( count > 4 )
    {
        // reuse the available storage for jacobian and other vars
        CvMat _J = cvMat( 2*count, 8, CV_64FC1, _L->data.db );
        CvMat _err = cvMat( 2*count, 1, CV_64FC1, _L->data.db + 2*count*8 );
        CvMat _JtJ = cvMat( 8, 8, CV_64FC1, LtL );
        CvMat _JtErr = cvMat( 8, 1, CV_64FC1, LtL + 8*8 );
        CvMat _JtJW = cvMat( 8, 1, CV_64FC1, LW );
        CvMat _JtJV = cvMat( 8, 8, CV_64FC1, LV );
        CvMat _Hinnov = cvMat( 8, 1, CV_64FC1, LV + 8*8 );

        for( k = 0; k < 10; k++ )
        {
            double* J = _J.data.db, *err = _err.data.db;
            for( i = 0; i < count; i++, J += 16, err += 2 )
            {
                double di = 1./(H[6]*M[i].x + H[7]*M[i].y + 1.);
                double _xi = (H[0]*M[i].x + H[1]*M[i].y + H[2])*di;
                double _yi = (H[3]*M[i].x + H[4]*M[i].y + H[5])*di;
                err[0] = m[i].x - _xi;
                err[1] = m[i].y - _yi;
                J[0] = M[i].x*di;
                J[1] = M[i].y*di;
                J[2] = di;
                J[8+3] = M[i].x;
                J[8+4] = M[i].y;
                J[8+5] = di;
                J[6] = -J[0]*_xi;
                J[7] = -J[1]*_xi;
                J[8+6] = -J[8+3]*_yi;
                J[8+7] = -J[8+4]*_yi;
                J[3] = J[4] = J[5] = J[8+0] = J[8+1] = J[8+2] = 0.;
            }

            icvGaussNewton( &_J, &_err, &_Hinnov, &_JtJ, &_JtErr, &_JtJW, &_JtJV );

            for( i = 0; i < 8; i++ )
                H[i] += _Hinnov.data.db[i];
        }
    }

    cvConvert( &_H, __H );

    __END__;

    cvReleaseMat( &_m );
    cvReleaseMat( &_M );
    cvReleaseMat( &_L );
}


CV_IMPL int
cvRodrigues2( const CvMat* src, CvMat* dst, CvMat* jacobian )
{
    int result = 0;
    
    CV_FUNCNAME( "cvRogrigues2" );

    __BEGIN__;

    int depth, elem_size;
    int i, k;
    double J[27];
    CvMat _J = cvMat( 3, 9, CV_64F, J );

    if( !CV_IS_MAT(src) )
        CV_ERROR( !src ? CV_StsNullPtr : CV_StsBadArg, "Input argument is not a valid matrix" );

    if( !CV_IS_MAT(dst) )
        CV_ERROR( !dst ? CV_StsNullPtr : CV_StsBadArg,
        "The first output argument is not a valid matrix" );

    depth = CV_MAT_DEPTH(src->type);
    elem_size = CV_ELEM_SIZE(depth);

    if( depth != CV_32F && depth != CV_64F )
        CV_ERROR( CV_StsUnsupportedFormat, "The matrices must have 32f or 64f data type" );

    if( !CV_ARE_DEPTHS_EQ(src, dst) )
        CV_ERROR( CV_StsUnmatchedFormats, "All the matrices must have the same data type" );

    if( jacobian )
    {
        if( !CV_IS_MAT(jacobian) )
            CV_ERROR( CV_StsBadArg, "Jacobian is not a valid matrix" );

        if( !CV_ARE_DEPTHS_EQ(src, jacobian) || CV_MAT_CN(jacobian->type) != 1 )
            CV_ERROR( CV_StsUnmatchedFormats, "Jacobian must have 32fC1 or 64fC1 datatype" );

        if( (jacobian->rows != 9 || jacobian->cols != 3) &&
            (jacobian->rows != 3 || jacobian->cols != 9))
            CV_ERROR( CV_StsBadSize, "Jacobian must be 3x9 or 9x3" );
    }

    if( src->cols == 1 || src->rows == 1 )
    {
        double rx, ry, rz, theta;
        int step = src->rows > 1 ? src->step / elem_size : 1;

        if( src->rows + src->cols*CV_MAT_CN(src->type) - 1 != 3 )
            CV_ERROR( CV_StsBadSize, "Input matrix must be 1x3, 3x1 or 3x3" );

        if( dst->rows != 3 || dst->cols != 3 || CV_MAT_CN(dst->type) != 1 )
            CV_ERROR( CV_StsBadSize, "Output matrix must be 3x3, single-channel floating point matrix" );

        if( depth == CV_32F )
        {
            rx = src->data.fl[0];
            ry = src->data.fl[step];
            rz = src->data.fl[step*2];
        }
        else
        {
            rx = src->data.db[0];
            ry = src->data.db[step];
            rz = src->data.db[step*2];
        }
        theta = sqrt(rx*rx + ry*ry + rz*rz);

        if( theta < DBL_EPSILON )
        {
            cvSetIdentity( dst );

            if( jacobian )
            {
                memset( J, 0, sizeof(J) );
                J[5] = J[15] = J[19] = -1;
                J[7] = J[11] = J[21] = 1;
            }
        }
        else
        {
            const double I[] = { 1, 0, 0, 0, 1, 0, 0, 0, 1 };
            
            double c = cos(theta);
            double s = sin(theta);
            double c1 = 1. - c;
            double itheta = theta ? 1./theta : 0.;

            rx *= itheta; ry *= itheta; rz *= itheta;

            double rrt[] = { rx*rx, rx*ry, rx*rz, rx*ry, ry*ry, ry*rz, rx*rz, ry*rz, rz*rz };
            double _r_x_[] = { 0, -rz, ry, rz, 0, -rx, -ry, rx, 0 };
            double R[9];
            CvMat _R = cvMat( 3, 3, CV_64F, R );

            // R = cos(theta)*I + (1 - cos(theta))*r*rT + sin(theta)*[r_x]
            // where [r_x] is [0 -rz ry; rz 0 -rx; -ry rx 0]
            for( k = 0; k < 9; k++ )
                R[k] = c*I[k] + c1*rrt[k] + s*_r_x_[k];

            cvConvert( &_R, dst );
            
            if( jacobian )
            {
                double drrt[] = { rx+rx, ry, rz, ry, 0, 0, rz, 0, 0,
                                  0, rx, 0, rx, ry+ry, rz, 0, rz, 0,
                                  0, 0, rx, 0, 0, ry, rx, ry, rz+rz };
                double d_r_x_[] = { 0, 0, 0, 0, 0, -1, 0, 1, 0,
                                    0, 0, 1, 0, 0, 0, -1, 0, 0,
                                    0, -1, 0, 1, 0, 0, 0, 0, 0 };
                for( i = 0; i < 3; i++ )
                {
                    double ri = i == 0 ? rx : i == 1 ? ry : rz;
                    double a0 = -s*ri, a1 = (s - 2*c1*itheta)*ri, a2 = c1*itheta;
                    double a3 = (c - s*itheta)*ri, a4 = s*itheta;
                    for( k = 0; k < 9; k++ )
                        J[i*9+k] = a0*I[k] + a1*rrt[k] + a2*drrt[i*9+k] +
                                   a3*_r_x_[k] + a4*d_r_x_[i*9+k];
                }
            }
        }
    }
    else if( src->cols == 3 && src->rows == 3 )
    {
        double R[9], U[9], V[9], W[3], rx, ry, rz;
        CvMat _R = cvMat( 3, 3, CV_64F, R );
        CvMat _U = cvMat( 3, 3, CV_64F, U );
        CvMat _V = cvMat( 3, 3, CV_64F, V );
        CvMat _W = cvMat( 3, 1, CV_64F, W );
        double theta, s, c;
        int step = dst->rows > 1 ? dst->step / elem_size : 1;
        
        if( (dst->rows != 1 || dst->cols*CV_MAT_CN(dst->type) != 3) &&
            (dst->rows != 3 || dst->cols != 1 || CV_MAT_CN(dst->type) != 1))
            CV_ERROR( CV_StsBadSize, "Output matrix must be 1x3 or 3x1" );

        cvConvert( src, &_R );
        if( !cvCheckArr( &_R, CV_CHECK_RANGE+CV_CHECK_QUIET, -100, 100 ) )
        {
            cvZero(dst);
            if( jacobian )
                cvZero(jacobian);
            EXIT;
        }
        
        cvSVD( &_R, &_W, &_U, &_V, CV_SVD_MODIFY_A + CV_SVD_U_T + CV_SVD_V_T );
        cvGEMM( &_U, &_V, 1, 0, 0, &_R, CV_GEMM_A_T );
        
        rx = R[7] - R[5];
        ry = R[2] - R[6];
        rz = R[3] - R[1];

        s = sqrt((rx*rx + ry*ry + rz*rz)*0.25);
        c = (R[0] + R[4] + R[8] - 1)*0.5;
        c = c > 1. ? 1. : c < -1. ? -1. : c;
        theta = acos(c);

        if( s < 1e-5 )
        {
            double t;

            if( c > 0 )
                rx = ry = rz = 0;
            else
            {
                t = (R[0] + 1)*0.5;
                rx = theta*sqrt(MAX(t,0.));
                t = (R[4] + 1)*0.5;
                ry = theta*sqrt(MAX(t,0.))*(R[1] < 0 ? -1. : 1.);
                t = (R[8] + 1)*0.5;
                rz = theta*sqrt(MAX(t,0.))*(R[2] < 0 ? -1. : 1.);
            }

            if( jacobian )
            {
                memset( J, 0, sizeof(J) );
                if( c > 0 )
                {
                    J[5] = J[15] = J[19] = -0.5;
                    J[7] = J[11] = J[21] = 0.5;
                }
            }
        }
        else
        {
            double vth = 1/(2*s);
            
            if( jacobian )
            {
                double t, dtheta_dtr = -1./s;
                // var1 = [vth;theta]
                // var = [om1;var1] = [om1;vth;theta]
                double dvth_dtheta = -vth*c/s;
                double d1 = 0.5*dvth_dtheta*dtheta_dtr;
                double d2 = 0.5*dtheta_dtr;
                // dvar1/dR = dvar1/dtheta*dtheta/dR = [dvth/dtheta; 1] * dtheta/dtr * dtr/dR
                double dvardR[5*9] =
                {
                    0, 0, 0, 0, 0, 1, 0, -1, 0,
                    0, 0, -1, 0, 0, 0, 1, 0, 0,
                    0, 1, 0, -1, 0, 0, 0, 0, 0,
                    d1, 0, 0, 0, d1, 0, 0, 0, d1,
                    d2, 0, 0, 0, d2, 0, 0, 0, d2
                };
                // var2 = [om;theta]
                double dvar2dvar[] =
                {
                    vth, 0, 0, rx, 0,
                    0, vth, 0, ry, 0,
                    0, 0, vth, rz, 0,
                    0, 0, 0, 0, 1
                };
                double domegadvar2[] = 
                {
                    theta, 0, 0, rx*vth,
                    0, theta, 0, ry*vth,
                    0, 0, theta, rz*vth
                };
        
                CvMat _dvardR = cvMat( 5, 9, CV_64FC1, dvardR );
                CvMat _dvar2dvar = cvMat( 4, 5, CV_64FC1, dvar2dvar );
                CvMat _domegadvar2 = cvMat( 3, 4, CV_64FC1, domegadvar2 );
                double t0[3*5];
                CvMat _t0 = cvMat( 3, 5, CV_64FC1, t0 );
        
                cvMatMul( &_domegadvar2, &_dvar2dvar, &_t0 );
                cvMatMul( &_t0, &_dvardR, &_J );

                // transpose every row of _J (treat the rows as 3x3 matrices)
                CV_SWAP(J[1], J[3], t); CV_SWAP(J[2], J[6], t); CV_SWAP(J[5], J[7], t);
                CV_SWAP(J[10], J[12], t); CV_SWAP(J[11], J[15], t); CV_SWAP(J[14], J[16], t);
                CV_SWAP(J[19], J[21], t); CV_SWAP(J[20], J[24], t); CV_SWAP(J[23], J[25], t);
            }

            vth *= theta;
            rx *= vth; ry *= vth; rz *= vth;
        }

        if( depth == CV_32F )
        {
            dst->data.fl[0] = (float)rx;
            dst->data.fl[step] = (float)ry;
            dst->data.fl[step*2] = (float)rz;
        }
        else
        {
            dst->data.db[0] = rx;
            dst->data.db[step] = ry;
            dst->data.db[step*2] = rz;
        }
    }

    if( jacobian )
    {
        if( depth == CV_32F )
        {
            if( jacobian->rows == _J.rows )
                cvConvert( &_J, jacobian );
            else
            {
                float Jf[3*9];
                CvMat _Jf = cvMat( _J.rows, _J.cols, CV_32FC1, Jf );
                cvConvert( &_J, &_Jf );
                cvTranspose( &_Jf, jacobian );
            }
        }
        else if( jacobian->rows == _J.rows )
            cvCopy( &_J, jacobian );
        else
            cvTranspose( &_J, jacobian );
    }

    result = 1;

    __END__;

    return result;
}


CV_IMPL void
cvProjectPoints2( const CvMat* obj_points,
                  const CvMat* r_vec,
                  const CvMat* t_vec,
                  const CvMat* A,
                  const CvMat* dist_coeffs,
                  CvMat* img_points, CvMat* dpdr,
                  CvMat* dpdt, CvMat* dpdf,
                  CvMat* dpdc, CvMat* dpdk )
{
    CvMat *_M = 0, *_m = 0;
    CvMat *_dpdr = 0, *_dpdt = 0, *_dpdc = 0, *_dpdf = 0, *_dpdk = 0;
    
    CV_FUNCNAME( "cvProjectPoints2" );

    __BEGIN__;

    int i, j, count;
    int calc_derivatives;
    const CvPoint3D64f* M;
    CvPoint2D64f* m;
    double r[3], R[9], dRdr[27], t[3], a[9], k[4] = {0,0,0,0}, fx, fy, cx, cy;
    CvMat _r, _t, _a = cvMat( 3, 3, CV_64F, a ), _k;
    CvMat _R = cvMat( 3, 3, CV_64F, R ), _dRdr = cvMat( 3, 9, CV_64F, dRdr );
    double *dpdr_p = 0, *dpdt_p = 0, *dpdk_p = 0, *dpdf_p = 0, *dpdc_p = 0;
    int dpdr_step = 0, dpdt_step = 0, dpdk_step = 0, dpdf_step = 0, dpdc_step = 0;

    if( !CV_IS_MAT(obj_points) || !CV_IS_MAT(r_vec) ||
        !CV_IS_MAT(t_vec) || !CV_IS_MAT(A) ||
        /*!CV_IS_MAT(dist_coeffs) ||*/ !CV_IS_MAT(img_points) )
        CV_ERROR( CV_StsBadArg, "One of required arguments is not a valid matrix" );

    count = MAX(obj_points->rows, obj_points->cols);

    if( CV_IS_CONT_MAT(obj_points->type) && CV_MAT_DEPTH(obj_points->type) == CV_64F &&
        (obj_points->rows == 1 && CV_MAT_CN(obj_points->type) == 3 ||
        obj_points->rows == count && CV_MAT_CN(obj_points->type)*obj_points->cols == 3))
        _M = (CvMat*)obj_points;
    else
    {
        CV_CALL( _M = cvCreateMat( 1, count, CV_64FC3 ));
        CV_CALL( cvConvertPointsHomogenious( obj_points, _M ));
    }

    if( CV_IS_CONT_MAT(img_points->type) && CV_MAT_DEPTH(img_points->type) == CV_64F &&
        (img_points->rows == 1 && CV_MAT_CN(img_points->type) == 2 ||
        img_points->rows == count && CV_MAT_CN(img_points->type)*img_points->cols == 2))
        _m = img_points;
    else
        CV_CALL( _m = cvCreateMat( 1, count, CV_64FC2 ));

    M = (CvPoint3D64f*)_M->data.db;
    m = (CvPoint2D64f*)_m->data.db;

    if( CV_MAT_DEPTH(r_vec->type) != CV_64F && CV_MAT_DEPTH(r_vec->type) != CV_32F ||
        (r_vec->rows != 1 && r_vec->cols != 1 ||
        r_vec->rows*r_vec->cols*CV_MAT_CN(r_vec->type) != 3) &&
        (r_vec->rows != 3 && r_vec->cols != 3 || CV_MAT_CN(r_vec->type) != 1))
        CV_ERROR( CV_StsBadArg, "Rotation must be represented by 1x3 or 3x1 "
                  "floating-point rotation vector, or 3x3 rotation matrix" );

    if( r_vec->rows == 3 && r_vec->cols == 3 )
    {
        _r = cvMat( 3, 1, CV_64FC1, r );
        CV_CALL( cvRodrigues2( r_vec, &_r ));
        CV_CALL( cvRodrigues2( &_r, &_R, &_dRdr ));
        cvCopy( r_vec, &_R );
    }
    else
    {
        _r = cvMat( r_vec->rows, r_vec->cols, CV_MAKETYPE(CV_64F,CV_MAT_CN(r_vec->type)), r );
        CV_CALL( cvConvert( r_vec, &_r ));
        CV_CALL( cvRodrigues2( &_r, &_R, &_dRdr ) );
    }

    if( CV_MAT_DEPTH(t_vec->type) != CV_64F && CV_MAT_DEPTH(t_vec->type) != CV_32F ||
        t_vec->rows != 1 && t_vec->cols != 1 ||
        t_vec->rows*t_vec->cols*CV_MAT_CN(t_vec->type) != 3 )
        CV_ERROR( CV_StsBadArg,
            "Translation vector must be 1x3 or 3x1 floating-point vector" );

    _t = cvMat( t_vec->rows, t_vec->cols, CV_MAKETYPE(CV_64F,CV_MAT_CN(t_vec->type)), t );
    CV_CALL( cvConvert( t_vec, &_t ));

    if( CV_MAT_TYPE(A->type) != CV_64FC1 && CV_MAT_TYPE(A->type) != CV_32FC1 ||
        A->rows != 3 || A->cols != 3 )
        CV_ERROR( CV_StsBadArg, "Instrinsic parameters must be 3x3 floating-point matrix" );

    CV_CALL( cvConvert( A, &_a ));
    fx = a[0]; fy = a[4];
    cx = a[2]; cy = a[5];

    if( dist_coeffs )
    {
        if( !CV_IS_MAT(dist_coeffs) ||
            CV_MAT_DEPTH(dist_coeffs->type) != CV_64F &&
            CV_MAT_DEPTH(dist_coeffs->type) != CV_32F ||
            dist_coeffs->rows != 1 && dist_coeffs->cols != 1 ||
            dist_coeffs->rows*dist_coeffs->cols*CV_MAT_CN(dist_coeffs->type) != 4 )
            CV_ERROR( CV_StsBadArg,
                "Distortion coefficients must be 1x4 or 4x1 floating-point vector" );

        _k = cvMat( dist_coeffs->rows, dist_coeffs->cols,
                    CV_MAKETYPE(CV_64F,CV_MAT_CN(dist_coeffs->type)), k );
        CV_CALL( cvConvert( dist_coeffs, &_k ));
    }
    
    if( dpdr )
    {
        if( !CV_IS_MAT(dpdr) ||
            CV_MAT_TYPE(dpdr->type) != CV_32FC1 &&
            CV_MAT_TYPE(dpdr->type) != CV_64FC1 ||
            dpdr->rows != count*2 || dpdr->cols != 3 )
            CV_ERROR( CV_StsBadArg, "dp/drot must be 2Nx3 floating-point matrix" );

        if( CV_MAT_TYPE(dpdr->type) == CV_64FC1 )
            _dpdr = dpdr;
        else
            CV_CALL( _dpdr = cvCreateMat( 2*count, 3, CV_64FC1 ));
        dpdr_p = _dpdr->data.db;
        dpdr_step = _dpdr->step/sizeof(dpdr_p[0]);
    }

    if( dpdt )
    {
        if( !CV_IS_MAT(dpdt) ||
            CV_MAT_TYPE(dpdt->type) != CV_32FC1 &&
            CV_MAT_TYPE(dpdt->type) != CV_64FC1 ||
            dpdt->rows != count*2 || dpdt->cols != 3 )
            CV_ERROR( CV_StsBadArg, "dp/dT must be 2Nx3 floating-point matrix" );

        if( CV_MAT_TYPE(dpdt->type) == CV_64FC1 )
            _dpdt = dpdt;
        else
            CV_CALL( _dpdt = cvCreateMat( 2*count, 3, CV_64FC1 ));
        dpdt_p = _dpdt->data.db;
        dpdt_step = _dpdt->step/sizeof(dpdt_p[0]);
    }

    if( dpdf )
    {
        if( !CV_IS_MAT(dpdf) ||
            CV_MAT_TYPE(dpdf->type) != CV_32FC1 && CV_MAT_TYPE(dpdf->type) != CV_64FC1 ||
            dpdf->rows != count*2 || dpdf->cols != 2 )
            CV_ERROR( CV_StsBadArg, "dp/df must be 2Nx2 floating-point matrix" );

        if( CV_MAT_TYPE(dpdf->type) == CV_64FC1 )
            _dpdf = dpdf;
        else
            CV_CALL( _dpdf = cvCreateMat( 2*count, 2, CV_64FC1 ));
        dpdf_p = _dpdf->data.db;
        dpdf_step = _dpdf->step/sizeof(dpdf_p[0]);
    }

    if( dpdc )
    {
        if( !CV_IS_MAT(dpdc) ||
            CV_MAT_TYPE(dpdc->type) != CV_32FC1 && CV_MAT_TYPE(dpdc->type) != CV_64FC1 ||
            dpdc->rows != count*2 || dpdc->cols != 2 )
            CV_ERROR( CV_StsBadArg, "dp/dc must be 2Nx2 floating-point matrix" );

        if( CV_MAT_TYPE(dpdc->type) == CV_64FC1 )
            _dpdc = dpdc;
        else
            CV_CALL( _dpdc = cvCreateMat( 2*count, 2, CV_64FC1 ));
        dpdc_p = _dpdc->data.db;
        dpdc_step = _dpdc->step/sizeof(dpdc_p[0]);
    }

    if( dpdk )
    {
        if( !CV_IS_MAT(dpdk) ||
            CV_MAT_TYPE(dpdk->type) != CV_32FC1 && CV_MAT_TYPE(dpdk->type) != CV_64FC1 ||
            dpdk->rows != count*2 || (dpdk->cols != 4 && dpdk->cols != 2) )
            CV_ERROR( CV_StsBadArg, "dp/df must be 2Nx4 or 2Nx2 floating-point matrix" );

        if( !dist_coeffs )
            CV_ERROR( CV_StsNullPtr, "dist_coeffs is NULL while dpdk is not" );

        if( CV_MAT_TYPE(dpdk->type) == CV_64FC1 )
            _dpdk = dpdk;
        else
            CV_CALL( _dpdk = cvCreateMat( dpdk->rows, dpdk->cols, CV_64FC1 ));
        dpdk_p = _dpdk->data.db;
        dpdk_step = _dpdk->step/sizeof(dpdk_p[0]);
    }

    calc_derivatives = dpdr || dpdt || dpdf || dpdc || dpdk;

    for( i = 0; i < count; i++ )
    {
        double X = M[i].x, Y = M[i].y, Z = M[i].z;
        double x = R[0]*X + R[1]*Y + R[2]*Z + t[0];
        double y = R[3]*X + R[4]*Y + R[5]*Z + t[1];
        double z = R[6]*X + R[7]*Y + R[8]*Z + t[2];
        double r2, r4, a1, a2, a3, cdist;
        double xd, yd;

        z = z ? 1./z : 1;
        x *= z; y *= z;

        r2 = x*x + y*y;
        r4 = r2*r2;
        a1 = 2*x*y;
        a2 = r2 + 2*x*x;
        a3 = r2 + 2*y*y;
        cdist = 1 + k[0]*r2 + k[1]*r4;
        xd = x*cdist + k[2]*a1 + k[3]*a2;
        yd = y*cdist + k[2]*a3 + k[3]*a1;

        m[i].x = xd*fx + cx;
        m[i].y = yd*fy + cy;

        if( calc_derivatives )
        {
            if( dpdc_p )
            {
                dpdc_p[0] = 1; dpdc_p[1] = 0;
                dpdc_p[dpdc_step] = 0;
                dpdc_p[dpdc_step+1] = 1;
                dpdc_p += dpdc_step*2;
            }

            if( dpdf_p )
            {
                dpdf_p[0] = xd; dpdf_p[1] = 0;
                dpdf_p[dpdf_step] = 0;
                dpdf_p[dpdf_step+1] = yd;
                dpdf_p += dpdf_step*2;
            }

            if( dpdk_p )
            {
                dpdk_p[0] = fx*x*r2;
                dpdk_p[1] = fx*x*r4;
                dpdk_p[dpdk_step] = fy*y*r2;
                dpdk_p[dpdk_step+1] = fy*y*r4;
                if( _dpdk->cols > 2 )
                {
                    dpdk_p[2] = fx*a1;
                    dpdk_p[3] = fx*a2;
                    dpdk_p[dpdk_step+2] = fy*a3;
                    dpdk_p[dpdk_step+3] = fy*a1;
                }
                dpdk_p += dpdk_step*2;
            }

            if( dpdt_p )
            {
                double dxdt[] = { z, 0, -x*z }, dydt[] = { 0, z, -y*z };
                for( j = 0; j < 3; j++ )
                {
                    double dr2dt = 2*x*dxdt[j] + 2*y*dydt[j];
                    double dcdist_dt = k[0]*dr2dt + 2*k[1]*r2*dr2dt;
                    double da1dt = 2*(x*dydt[j] + y*dxdt[j]);
                    double dmxdt = fx*(dxdt[j]*cdist + x*dcdist_dt +
                                k[2]*da1dt + k[3]*(dr2dt + 2*x*dxdt[j]));
                    double dmydt = fy*(dydt[j]*cdist + y*dcdist_dt +
                                k[2]*(dr2dt + 2*y*dydt[j]) + k[3]*da1dt);
                    dpdt_p[j] = dmxdt;
                    dpdt_p[dpdt_step+j] = dmydt;
                }
                dpdt_p += dpdt_step*2;
            }

            if( dpdr_p )
            {
                double dx0dr[] =
                {
                    X*dRdr[0] + Y*dRdr[1] + Z*dRdr[2],
                    X*dRdr[9] + Y*dRdr[10] + Z*dRdr[11],
                    X*dRdr[18] + Y*dRdr[19] + Z*dRdr[20]
                };
                double dy0dr[] =
                {
                    X*dRdr[3] + Y*dRdr[4] + Z*dRdr[5],
                    X*dRdr[12] + Y*dRdr[13] + Z*dRdr[14],
                    X*dRdr[21] + Y*dRdr[22] + Z*dRdr[23]
                };
                double dz0dr[] =
                {
                    X*dRdr[6] + Y*dRdr[7] + Z*dRdr[8],
                    X*dRdr[15] + Y*dRdr[16] + Z*dRdr[17],
                    X*dRdr[24] + Y*dRdr[25] + Z*dRdr[26]
                };
                for( j = 0; j < 3; j++ )
                {
                    double dxdr = z*(dx0dr[j] - x*dz0dr[j]);
                    double dydr = z*(dy0dr[j] - y*dz0dr[j]);
                    double dr2dr = 2*x*dxdr + 2*y*dydr;
                    double dcdist_dr = k[0]*dr2dr + 2*k[1]*r2*dr2dr;
                    double da1dr = 2*(x*dydr + y*dxdr);
                    double dmxdr = fx*(dxdr*cdist + x*dcdist_dr +
                                k[2]*da1dr + k[3]*(dr2dr + 2*x*dxdr));
                    double dmydr = fy*(dydr*cdist + y*dcdist_dr +
                                k[2]*(dr2dr + 2*y*dydr) + k[3]*da1dr);
                    dpdr_p[j] = dmxdr;
                    dpdr_p[dpdr_step+j] = dmydr;
                }
                dpdr_p += dpdr_step*2;
            }
        }
    }

    if( _m != img_points )
        cvConvertPointsHomogenious( _m, img_points );
    if( _dpdr != dpdr )
        cvConvert( _dpdr, dpdr );
    if( _dpdt != dpdt )
        cvConvert( _dpdt, dpdt );
    if( _dpdf != dpdf )
        cvConvert( _dpdf, dpdf );
    if( _dpdc != dpdc )
        cvConvert( _dpdc, dpdc );
    if( _dpdk != dpdk )
        cvConvert( _dpdk, dpdk );

    __END__;

    if( _M != obj_points )
        cvReleaseMat( &_M );
    if( _m != img_points )
        cvReleaseMat( &_m );
    if( _dpdr != dpdr )
        cvReleaseMat( &_dpdr );
    if( _dpdt != dpdt )
        cvReleaseMat( &_dpdt );
    if( _dpdf != dpdf )
        cvReleaseMat( &_dpdf );
    if( _dpdc != dpdc )
        cvReleaseMat( &_dpdc );
    if( _dpdk != dpdk )
        cvReleaseMat( &_dpdk );
}


CV_IMPL void
cvFindExtrinsicCameraParams2( const CvMat* obj_points,
                  const CvMat* img_points, const CvMat* A,
                  const CvMat* dist_coeffs,
                  CvMat* r_vec, CvMat* t_vec )
{
    const int max_iter = 20;
    CvMat *_M = 0, *_Mxy = 0, *_m = 0, *_mn = 0, *_L = 0, *_J = 0;
    
    CV_FUNCNAME( "cvFindExtrinsicCameraParams2" );

    __BEGIN__;

    int i, j, count;
    double a[9], k[4] = { 0, 0, 0, 0 }, R[9], ifx, ify, cx, cy;
    double Mc[3] = {0, 0, 0}, MM[9], U[9], V[9], W[3];
    double JtJ[6*6], JtErr[6], JtJW[6], JtJV[6*6], delta[6], param[6];
    CvPoint3D64f* M = 0;
    CvPoint2D64f *m = 0, *mn = 0;
    CvMat _a = cvMat( 3, 3, CV_64F, a );
    CvMat _R = cvMat( 3, 3, CV_64F, R );
    CvMat _r = cvMat( 3, 1, CV_64F, param );
    CvMat _t = cvMat( 3, 1, CV_64F, param + 3 );
    CvMat _Mc = cvMat( 1, 3, CV_64F, Mc );
    CvMat _MM = cvMat( 3, 3, CV_64F, MM );
    CvMat _U = cvMat( 3, 3, CV_64F, U );
    CvMat _V = cvMat( 3, 3, CV_64F, V );
    CvMat _W = cvMat( 3, 1, CV_64F, W );
    CvMat _JtJ = cvMat( 6, 6, CV_64F, JtJ );
    CvMat _JtErr = cvMat( 6, 1, CV_64F, JtErr );
    CvMat _JtJW = cvMat( 6, 1, CV_64F, JtJW );
    CvMat _JtJV = cvMat( 6, 6, CV_64F, JtJV );
    CvMat _delta = cvMat( 6, 1, CV_64F, delta );
    CvMat _param = cvMat( 6, 1, CV_64F, param );
    CvMat _dpdr, _dpdt;

    if( !CV_IS_MAT(obj_points) || !CV_IS_MAT(img_points) ||
        !CV_IS_MAT(A) || !CV_IS_MAT(r_vec) || !CV_IS_MAT(t_vec) )
        CV_ERROR( CV_StsBadArg, "One of required arguments is not a valid matrix" );

    count = MAX(obj_points->cols, obj_points->rows);
    CV_CALL( _M = cvCreateMat( 1, count, CV_64FC3 ));
    CV_CALL( _Mxy = cvCreateMat( 1, count, CV_64FC2 ));
    CV_CALL( _m = cvCreateMat( 1, count, CV_64FC2 ));
    CV_CALL( _mn = cvCreateMat( 1, count, CV_64FC2 ));
    M = (CvPoint3D64f*)_M->data.db;
    m = (CvPoint2D64f*)_m->data.db;
    mn = (CvPoint2D64f*)_mn->data.db;

    CV_CALL( cvConvertPointsHomogenious( obj_points, _M ));
    CV_CALL( cvConvertPointsHomogenious( img_points, _m ));
    CV_CALL( cvConvert( A, &_a ));

    if( dist_coeffs )
    {
        CvMat _k;
        if( !CV_IS_MAT(dist_coeffs) ||
            CV_MAT_DEPTH(dist_coeffs->type) != CV_64F &&
            CV_MAT_DEPTH(dist_coeffs->type) != CV_32F ||
            dist_coeffs->rows != 1 && dist_coeffs->cols != 1 ||
            dist_coeffs->rows*dist_coeffs->cols*CV_MAT_CN(dist_coeffs->type) != 4 )
            CV_ERROR( CV_StsBadArg,
                "Distortion coefficients must be 1x4 or 4x1 floating-point vector" );

        _k = cvMat( dist_coeffs->rows, dist_coeffs->cols,
                    CV_MAKETYPE(CV_64F,CV_MAT_CN(dist_coeffs->type)), k );
        CV_CALL( cvConvert( dist_coeffs, &_k ));
    }

    if( CV_MAT_DEPTH(r_vec->type) != CV_64F && CV_MAT_DEPTH(r_vec->type) != CV_32F ||
        r_vec->rows != 1 && r_vec->cols != 1 ||
        r_vec->rows*r_vec->cols*CV_MAT_CN(r_vec->type) != 3 )
        CV_ERROR( CV_StsBadArg, "Rotation vector must be 1x3 or 3x1 floating-point vector" );

    if( CV_MAT_DEPTH(t_vec->type) != CV_64F && CV_MAT_DEPTH(t_vec->type) != CV_32F ||
        t_vec->rows != 1 && t_vec->cols != 1 ||
        t_vec->rows*t_vec->cols*CV_MAT_CN(t_vec->type) != 3 )
        CV_ERROR( CV_StsBadArg,
            "Translation vector must be 1x3 or 3x1 floating-point vector" );

    ifx = 1./a[0]; ify = 1./a[4];
    cx = a[2]; cy = a[5];

    // normalize image points
    // (unapply the intrinsic matrix transformation and distortion)
    for( i = 0; i < count; i++ )
    {
        double x = (m[i].x - cx)*ifx, y = (m[i].y - cy)*ify, x0 = x, y0 = y;

        // compensate distortion iteratively
        if( dist_coeffs )
            for( j = 0; j < 5; j++ )
            {
                double r2 = x*x + y*y;
                double icdist = 1./(1 + k[0]*r2 + k[1]*r2*r2);
                double delta_x = 2*k[2]*x*y + k[3]*(r2 + 2*x*x);
                double delta_y = k[2]*(r2 + 2*y*y) + 2*k[3]*x*y;
                x = (x0 - delta_x)*icdist;
                y = (y0 - delta_y)*icdist;
            }
        mn[i].x = x; mn[i].y = y;

        // calc mean(M)
        Mc[0] += M[i].x;
        Mc[1] += M[i].y;
        Mc[2] += M[i].z;
    }

    Mc[0] /= count;
    Mc[1] /= count;
    Mc[2] /= count;

    cvReshape( _M, _M, 1, count );
    cvMulTransposed( _M, &_MM, 1, &_Mc );
    cvSVD( &_MM, &_W, 0, &_V, CV_SVD_MODIFY_A + CV_SVD_V_T );

    // initialize extrinsic parameters
    if( W[2]/W[1] < 1e-3 || count < 4 )
    {
        // a planar structure case (all M's lie in the same plane)
        double tt[3], h[9], h1_norm, h2_norm;
        CvMat* R_transform = &_V;
        CvMat T_transform = cvMat( 3, 1, CV_64F, tt );
        CvMat _H = cvMat( 3, 3, CV_64F, h );
        CvMat _h1, _h2, _h3;

        if( V[2]*V[2] + V[5]*V[5] < 1e-10 )
            cvSetIdentity( R_transform );

        if( cvDet(R_transform) < 0 )
            cvScale( R_transform, R_transform, -1 );

        cvGEMM( R_transform, &_Mc, -1, 0, 0, &T_transform, CV_GEMM_B_T );

        for( i = 0; i < count; i++ )
        {
            const double* Rp = R_transform->data.db;
            const double* Tp = T_transform.data.db;
            const double* src = _M->data.db + i*3;
            double* dst = _Mxy->data.db + i*2;

            dst[0] = Rp[0]*src[0] + Rp[1]*src[1] + Rp[2]*src[2] + Tp[0];
            dst[1] = Rp[3]*src[0] + Rp[4]*src[1] + Rp[5]*src[2] + Tp[1];
        }

        cvFindHomography( _Mxy, _mn, &_H );

        cvGetCol( &_H, &_h1, 0 );
        _h2 = _h1; _h2.data.db++;
        _h3 = _h2; _h3.data.db++;
        h1_norm = sqrt(h[0]*h[0] + h[3]*h[3] + h[6]*h[6]);
        h2_norm = sqrt(h[1]*h[1] + h[4]*h[4] + h[7]*h[7]);

        cvScale( &_h1, &_h1, 1./h1_norm );
        cvScale( &_h2, &_h2, 1./h2_norm );
        cvScale( &_h3, &_t, 2./(h1_norm + h2_norm));
        cvCrossProduct( &_h1, &_h2, &_h3 );

        cvRodrigues2( &_H, &_r );
        cvRodrigues2( &_r, &_H );
        cvMatMulAdd( &_H, &T_transform, &_t, &_t );
        cvMatMul( &_H, R_transform, &_R );
        cvRodrigues2( &_R, &_r );
    }
    else
    {
        // non-planar structure. Use DLT method
        double* L;
        double LL[12*12], LW[12], LV[12*12], sc;
        CvMat _LL = cvMat( 12, 12, CV_64F, LL );
        CvMat _LW = cvMat( 12, 1, CV_64F, LW );
        CvMat _LV = cvMat( 12, 12, CV_64F, LV );
        CvMat _RRt, _RR, _tt;

        CV_CALL( _L = cvCreateMat( 2*count, 12, CV_64F ));
        L = _L->data.db;

        for( i = 0; i < count; i++, L += 24 )
        {
            double x = -mn[i].x, y = -mn[i].y;
            L[0] = L[16] = M[i].x;
            L[1] = L[17] = M[i].y;
            L[2] = L[18] = M[i].z;
            L[3] = L[19] = 1.;
            L[4] = L[5] = L[6] = L[7] = 0.;
            L[12] = L[13] = L[14] = L[15] = 0.;
            L[8] = x*M[i].x;
            L[9] = x*M[i].y;
            L[10] = x*M[i].z;
            L[11] = x;
            L[20] = y*M[i].x;
            L[21] = y*M[i].y;
            L[22] = y*M[i].z;
            L[23] = y;
        }

        cvMulTransposed( _L, &_LL, 1 );
        cvSVD( &_LL, &_LW, 0, &_LV, CV_SVD_MODIFY_A + CV_SVD_V_T );
        _RRt = cvMat( 3, 4, CV_64F, LV + 11*12 );
        cvGetCols( &_RRt, &_RR, 0, 3 );
        cvGetCol( &_RRt, &_tt, 3 );
        if( cvDet(&_RR) < 0 )
            cvScale( &_RRt, &_RRt, -1 );
        sc = cvNorm(&_RR);
        cvSVD( &_RR, &_W, &_U, &_V, CV_SVD_MODIFY_A + CV_SVD_U_T + CV_SVD_V_T );
        cvGEMM( &_U, &_V, 1, 0, 0, &_R, CV_GEMM_A_T );
        cvScale( &_tt, &_t, cvNorm(&_R)/sc );
        cvRodrigues2( &_R, &_r );
        cvReleaseMat( &_L );
    }

    CV_CALL( _J = cvCreateMat( 2*count, 6, CV_64FC1 ));
    cvGetCols( _J, &_dpdr, 0, 3 );
    cvGetCols( _J, &_dpdt, 3, 6 );

    // refine extrinsic parameters using iterative algorithm
    for( i = 0; i < max_iter; i++ )
    {
        double n1, n2;
        cvReshape( _mn, _mn, 2, 1 );
        cvProjectPoints2( _M, &_r, &_t, &_a, dist_coeffs,
                          _mn, &_dpdr, &_dpdt, 0, 0, 0 );
        cvSub( _m, _mn, _mn );
        cvReshape( _mn, _mn, 1, 2*count );

        cvMulTransposed( _J, &_JtJ, 1 );
        cvGEMM( _J, _mn, 1, 0, 0, &_JtErr, CV_GEMM_A_T );
        cvSVD( &_JtJ, &_JtJW, 0, &_JtJV, CV_SVD_MODIFY_A + CV_SVD_V_T );
        if( JtJW[5]/JtJW[0] < 1e-12 )
            break;
        cvSVBkSb( &_JtJW, &_JtJV, &_JtJV, &_JtErr,
                  &_delta, CV_SVD_U_T + CV_SVD_V_T );
        cvAdd( &_delta, &_param, &_param );
        n1 = cvNorm( &_delta );
        n2 = cvNorm( &_param );
        if( n1/n2 < 1e-10 )
            break;
    }

    _r = cvMat( r_vec->rows, r_vec->cols,
        CV_MAKETYPE(CV_64F,CV_MAT_CN(r_vec->type)), param );
    _t = cvMat( t_vec->rows, t_vec->cols,
        CV_MAKETYPE(CV_64F,CV_MAT_CN(t_vec->type)), param + 3 );

    cvConvert( &_r, r_vec );
    cvConvert( &_t, t_vec );

    __END__;

    cvReleaseMat( &_M );
    cvReleaseMat( &_Mxy );
    cvReleaseMat( &_m );
    cvReleaseMat( &_mn );
    cvReleaseMat( &_L );
    cvReleaseMat( &_J );
}


static void
icvInitIntrinsicParams2D( const CvMat* obj_points,
                          const CvMat* img_points,
                          const CvMat* point_counts,
                          CvSize image_size,
                          CvMat* intrinsic_matrix,
                          double aspect_ratio )
{
    CvMat *_A = 0, *_b = 0;
    
    CV_FUNCNAME( "icvInitIntrinsicParams2D" );

    __BEGIN__;

    int i, j, pos, img_count;
    double a[9] = { 0, 0, 0, 0, 0, 0, 0, 0, 1 };
    double H[9], AtA[4], AtAW[2], AtAV[4], Atb[2], f[2];
    CvMat _a = cvMat( 3, 3, CV_64F, a );
    CvMat _H = cvMat( 3, 3, CV_64F, H );
    CvMat _AtA = cvMat( 2, 2, CV_64F, AtA );
    CvMat _AtAW = cvMat( 2, 1, CV_64F, AtAW );
    CvMat _AtAV = cvMat( 2, 2, CV_64F, AtAV );
    CvMat _Atb = cvMat( 2, 1, CV_64F, Atb );
    CvMat _f = cvMat( 2, 1, CV_64F, f );

    assert( CV_MAT_TYPE(point_counts->type) == CV_32SC1 &&
            CV_IS_MAT_CONT(point_counts->type) );
    img_count = point_counts->rows + point_counts->cols - 1;

    if( CV_MAT_TYPE(obj_points->type) != CV_32FC3 &&
        CV_MAT_TYPE(obj_points->type) != CV_64FC3 ||
        CV_MAT_TYPE(img_points->type) != CV_32FC2 &&
        CV_MAT_TYPE(img_points->type) != CV_64FC2 )
        CV_ERROR( CV_StsUnsupportedFormat, "Both object points and image points must be 2D" );

    if( obj_points->rows != 1 || img_points->rows != 1 )
        CV_ERROR( CV_StsBadSize, "object points and image points must be a single-row matrices" );

    CV_CALL( _A = cvCreateMat( 2*img_count, 2, CV_64F ));
    CV_CALL( _b = cvCreateMat( 2*img_count, 1, CV_64F ));
    a[2] = (image_size.width - 1)*0.5;
    a[5] = (image_size.height - 1)*0.5;

    // extract vanishing points in order to obtain initial value for the focal length
    for( i = 0, pos = 0; i < img_count; i++ )
    {
        double* Ap = _A->data.db + i*4;
        double* bp = _b->data.db + i*2;
        int count = point_counts->data.i[i];
        double h[3], v[3], d1[3], d2[3];
        double n[4] = {0,0,0,0};
        CvMat _m, _M;
        cvGetCols( obj_points, &_M, pos, pos + count );
        cvGetCols( img_points, &_m, pos, pos + count );
        pos += count;

        CV_CALL( cvFindHomography( &_M, &_m, &_H ));
        
        H[0] -= H[6]*a[2]; H[1] -= H[7]*a[2]; H[2] -= H[8]*a[2];
        H[3] -= H[6]*a[5]; H[4] -= H[7]*a[5]; H[5] -= H[8]*a[5];

        for( j = 0; j < 3; j++ )
        {
            double t0 = H[j*3], t1 = H[j*3+1];
            h[j] = t0; v[j] = t1;
            d1[j] = (t0 + t1)*0.5;
            d2[j] = (t0 - t1)*0.5;
            n[0] += t0*t0; n[1] += t1*t1;
            n[2] += d1[j]*d1[j]; n[3] += d2[j]*d2[j];
        }

        for( j = 0; j < 4; j++ )
            n[j] = 1./sqrt(n[j]);

        for( j = 0; j < 3; j++ )
        {
            h[j] *= n[0]; v[j] *= n[1];
            d1[j] *= n[2]; d2[j] *= n[3];
        }

        Ap[0] = h[0]*v[0]; Ap[1] = h[1]*v[1];
        Ap[2] = d1[0]*d2[0]; Ap[3] = d1[1]*d2[1];
        bp[0] = -h[2]*v[2]; bp[1] = -d1[2]*d2[2];
    }

    // while it is not about gradient descent search,
    // the formula is the same: f = inv(At*A)*At*b
    icvGaussNewton( _A, _b, &_f, &_AtA, &_Atb, &_AtAW, &_AtAV );
    a[0] = sqrt(fabs(1./f[0]));
    a[4] = sqrt(fabs(1./f[1]));
    if( aspect_ratio != 0 )
    {
        double tf = (a[0] + a[4])/(aspect_ratio + 1.);
        a[0] = aspect_ratio*tf;
        a[4] = tf;
    }

    cvConvert( &_a, intrinsic_matrix );

    __END__;

    cvReleaseMat( &_A );
    cvReleaseMat( &_b );
}


/* finds intrinsic and extrinsic camera parameters
   from a few views of known calibration pattern */
CV_IMPL void
cvCalibrateCamera2( const CvMat* obj_points,
                    const CvMat* img_points,
                    const CvMat* point_counts,
                    CvSize image_size,
                    CvMat* A, CvMat* dist_coeffs,
                    CvMat* r_vecs, CvMat* t_vecs,
                    int flags )
{
    double alpha_smooth = 0.4;
    
    CvMat *counts = 0, *_M = 0, *_m = 0;
    CvMat *_Ji = 0, *_Je = 0, *_JtJ = 0, *_JtErr = 0, *_JtJW = 0, *_JtJV = 0;
    CvMat *_param = 0, *_param_innov = 0, *_err = 0;
    
    CV_FUNCNAME( "cvCalibrateCamera2" );

    __BEGIN__;

    double a[9];
    CvMat _a = cvMat( 3, 3, CV_64F, a ), _k;
    CvMat _Mi, _mi, _ri, _ti, _part;
    CvMat _dpdr, _dpdt, _dpdf, _dpdc, _dpdk;
    CvMat _sr_part = cvMat( 1, 3, CV_64F ), _st_part = cvMat( 1, 3, CV_64F ), _r_part, _t_part;
    int i, j, pos, iter, img_count, count = 0, max_count = 0, total = 0, param_count;
    int r_depth = 0, t_depth = 0, r_step = 0, t_step = 0, cn, dims;
    int output_r_matrices = 0;
    double aspect_ratio = 0.;

    if( !CV_IS_MAT(obj_points) || !CV_IS_MAT(img_points) ||
        !CV_IS_MAT(point_counts) || !CV_IS_MAT(A) || !CV_IS_MAT(dist_coeffs) )
        CV_ERROR( CV_StsBadArg, "One of required vector arguments is not a valid matrix" );

    if( image_size.width <= 0 || image_size.height <= 0 )
        CV_ERROR( CV_StsOutOfRange, "image width and height must be positive" );

    if( CV_MAT_TYPE(point_counts->type) != CV_32SC1 ||
        point_counts->rows != 1 && point_counts->cols != 1 )
        CV_ERROR( CV_StsUnsupportedFormat,
            "the array of point counters must be 1-dimensional integer vector" );

    CV_CALL( counts = cvCreateMat( point_counts->rows, point_counts->width, CV_32SC1 ));
    cvCopy( point_counts, counts );

    img_count = counts->rows + counts->cols - 1;

    if( r_vecs )
    {
        r_depth = CV_MAT_DEPTH(r_vecs->type);
        r_step = r_vecs->rows == 1 ? 3*CV_ELEM_SIZE(r_depth) : r_vecs->step;
        cn = CV_MAT_CN(r_vecs->type);
        if( !CV_IS_MAT(r_vecs) || r_depth != CV_32F && r_depth != CV_64F ||
            (r_vecs->rows != img_count || r_vecs->cols*cn != 3 && r_vecs->cols*cn != 9) &&
            (r_vecs->rows != 1 || r_vecs->cols != img_count || cn != 3) )
            CV_ERROR( CV_StsBadArg, "the output array of rotation vectors must be 3-channel "
                "1xn or nx1 array or 1-channel nx3 or nx9 array, where n is the number of views" );
        output_r_matrices = r_vecs->rows == img_count && r_vecs->cols*cn == 9;
    }

    if( t_vecs )
    {
        t_depth = CV_MAT_DEPTH(t_vecs->type);
        t_step = t_vecs->rows == 1 ? 3*CV_ELEM_SIZE(t_depth) : t_vecs->step;
        cn = CV_MAT_CN(t_vecs->type);
        if( !CV_IS_MAT(t_vecs) || t_depth != CV_32F && t_depth != CV_64F ||
            (t_vecs->rows != img_count || t_vecs->cols*cn != 3) &&
            (t_vecs->rows != 1 || t_vecs->cols != img_count || cn != 3) )
            CV_ERROR( CV_StsBadArg, "the output array of translation vectors must be 3-channel "
                "1xn or nx1 array or 1-channel nx3 array, where n is the number of views" );
    }

    if( CV_MAT_TYPE(A->type) != CV_32FC1 && CV_MAT_TYPE(A->type) != CV_64FC1 ||
        A->rows != 3 || A->cols != 3 )
        CV_ERROR( CV_StsBadArg,
            "Intrinsic parameters must be 3x3 floating-point matrix" );

    if( CV_MAT_TYPE(dist_coeffs->type) != CV_32FC1 &&
        CV_MAT_TYPE(dist_coeffs->type) != CV_64FC1 ||
        (dist_coeffs->rows != 4 || dist_coeffs->cols != 1) &&
        (dist_coeffs->cols != 4 || dist_coeffs->rows != 1))
        CV_ERROR( CV_StsBadArg,
            "Distortion coefficients must be 4x1 or 1x4 floating-point matrix" );

    for( i = 0; i < img_count; i++ )
    {
        int temp_count = counts->data.i[i];
        if( temp_count < 4 )
        {
            char buf[100];
            sprintf( buf, "The number of points in the view #%d is <4", i );
            CV_ERROR( CV_StsOutOfRange, buf );
        }
        max_count = MAX( max_count, temp_count );
        total += temp_count;
    }

    dims = CV_MAT_CN(obj_points->type)*(obj_points->rows == 1 ? 1 : obj_points->cols);

    if( CV_MAT_DEPTH(obj_points->type) != CV_32F &&
        CV_MAT_DEPTH(obj_points->type) != CV_64F ||
        (obj_points->rows != total || dims != 3 && dims != 2) &&
        (obj_points->rows != 1 || obj_points->cols != total || (dims != 3 && dims != 2)) )
        CV_ERROR( CV_StsBadArg, "Object points must be 1xn or nx1, 2- or 3-channel matrix, "
                                "or nx3 or nx2 single-channel matrix" );

    cn = CV_MAT_CN(img_points->type);
    if( CV_MAT_DEPTH(img_points->type) != CV_32F &&
        CV_MAT_DEPTH(img_points->type) != CV_64F ||
        (img_points->rows != total || img_points->cols*cn != 2) &&
        (img_points->rows != 1 || img_points->cols != total || cn != 2) )
        CV_ERROR( CV_StsBadArg, "Image points must be 1xn or nx1, 2-channel matrix, "
                                "or nx2 single-channel matrix" );

    CV_CALL( _M = cvCreateMat( 1, total, CV_64FC3 ));
    CV_CALL( _m = cvCreateMat( 1, total, CV_64FC2 ));

    CV_CALL( cvConvertPointsHomogenious( obj_points, _M ));
    CV_CALL( cvConvertPointsHomogenious( img_points, _m ));

    param_count = 8 + img_count*6;
    CV_CALL( _param = cvCreateMat( param_count, 1, CV_64FC1 ));
    CV_CALL( _param_innov = cvCreateMat( param_count, 1, CV_64FC1 ));
    CV_CALL( _JtJ = cvCreateMat( param_count, param_count, CV_64FC1 ));
    CV_CALL( _JtErr = cvCreateMat( param_count, 1, CV_64FC1 ));
    CV_CALL( _JtJW = cvCreateMat( param_count, 1, CV_64FC1 ));
    CV_CALL( _JtJV = cvCreateMat( param_count, param_count, CV_64FC1 ));
    CV_CALL( _Ji = cvCreateMat( max_count*2, 8, CV_64FC1 ));
    CV_CALL( _Je = cvCreateMat( max_count*2, 6, CV_64FC1 ));
    CV_CALL( _err = cvCreateMat( max_count*2, 1, CV_64FC1 ));
    
    cvGetCols( _Je, &_dpdr, 0, 3 );
    cvGetCols( _Je, &_dpdt, 3, 6 );
    cvGetCols( _Ji, &_dpdf, 0, 2 );
    cvGetCols( _Ji, &_dpdc, 2, 4 );
    cvGetCols( _Ji, &_dpdk, 4, flags & CV_CALIB_ZERO_TANGENT_DIST ? 6 : 8 );
    cvZero( _Ji );

    // 1. initialize intrinsic parameters
    if( flags & CV_CALIB_USE_INTRINSIC_GUESS )
    {
        cvConvert( A, &_a );
        if( a[0] <= 0 || a[4] <= 0 )
            CV_ERROR( CV_StsOutOfRange, "Focal length (fx and fy) must be positive" );
        if( a[2] < 0 || a[2] >= image_size.width ||
            a[5] < 0 || a[5] >= image_size.height )
            CV_ERROR( CV_StsOutOfRange, "Principal point must be within the image" );
        if( fabs(a[1]) > 1e-5 )
            CV_ERROR( CV_StsOutOfRange, "Non-zero skew is not supported by the function" );
        if( fabs(a[3]) > 1e-5 || fabs(a[6]) > 1e-5 ||
            fabs(a[7]) > 1e-5 || fabs(a[8]-1) > 1e-5 )
            CV_ERROR( CV_StsOutOfRange,
                "The intrinsic matrix must have [fx 0 cx; 0 fy cy; 0 0 1] shape" );
        a[1] = a[3] = a[6] = a[7] = 0.;
        a[8] = 1.;

        if( flags & CV_CALIB_FIX_ASPECT_RATIO )
            aspect_ratio = a[0]/a[4];
    }
    else
    {
        if( dims == 3 )
        {
            CvScalar mean, sdv;
            cvAvgSdv( _M, &mean, &sdv );
            if( fabs(mean.val[2]) > 1e-5 && fabs(mean.val[2] - 1) > 1e-5 ||
                fabs(sdv.val[2]) > 1e-5 )
                CV_ERROR( CV_StsBadArg,
                "For non-planar calibration rigs the initial intrinsic matrix must be specified" );
        }
        for( i = 0; i < total; i++ )
            ((CvPoint3D64f*)(_M->data.db + i*3))->z = 0.;

        if( flags & CV_CALIB_FIX_ASPECT_RATIO )
        {
            aspect_ratio = cvmGet(A,0,0);
            aspect_ratio /= cvmGet(A,1,1);
            if( aspect_ratio < 0.01 || aspect_ratio > 100 )
                CV_ERROR( CV_StsOutOfRange,
                    "The specified aspect ratio (=a(0,0)/a(1,1)) is incorrect" );
        }
        icvInitIntrinsicParams2D( _M, _m, counts, image_size, &_a, aspect_ratio );
    }

    _k = cvMat( dist_coeffs->rows, dist_coeffs->cols, CV_64FC1, _param->data.db + 4 );
    cvZero( &_k );

    // 2. initialize extrinsic parameters
    for( i = 0, pos = 0; i < img_count; i++, pos += count )
    {
        count = counts->data.i[i];
        _ri = cvMat( 1, 3, CV_64FC1, _param->data.db + 8 + i*6 );
        _ti = cvMat( 1, 3, CV_64FC1, _param->data.db + 8 + i*6 + 3 );

        cvGetCols( _M, &_Mi, pos, pos + count );
        cvGetCols( _m, &_mi, pos, pos + count );
        cvFindExtrinsicCameraParams2( &_Mi, &_mi, &_a, &_k, &_ri, &_ti );
    }

    _param->data.db[0] = a[0];
    _param->data.db[1] = a[4];
    _param->data.db[2] = a[2];
    _param->data.db[3] = a[5];

    // 3. run the optimization
    for( iter = 0; iter < 30; iter++ )
    {
        double* jj = _JtJ->data.db;
        double change;

        for( i = 0, pos = 0; i < img_count; i++, pos += count )
        {
            count = counts->data.i[i];
            _ri = cvMat( 1, 3, CV_64FC1, _param->data.db + 8 + i*6);
            _ti = cvMat( 1, 3, CV_64FC1, _param->data.db + 8 + i*6 + 3);

            cvGetCols( _M, &_Mi, pos, pos + count );
            _mi = cvMat( count*2, 1, CV_64FC1, _m->data.db + pos*2 );

            _dpdr.rows = _dpdt.rows = _dpdf.rows = _dpdc.rows = _dpdk.rows = count*2;

            _err->cols = 1;
            _err->rows = count*2;
            cvReshape( _err, _err, 2, count );
            cvProjectPoints2( &_Mi, &_ri, &_ti, &_a, &_k, _err, &_dpdr, &_dpdt, &_dpdf,
                              flags & CV_CALIB_FIX_PRINCIPAL_POINT ? 0 : &_dpdc, &_dpdk );

            // alter dpdf in case if only one of the focal
            // parameters (fy) is independent variable
            if( flags & CV_CALIB_FIX_ASPECT_RATIO )
                for( j = 0; j < count; j++ )
                {
                    double* dpdf_j = (double*)(_dpdf.data.ptr + j*_dpdf.step*2);
                    dpdf_j[1] = dpdf_j[0]*aspect_ratio;
                    dpdf_j[0] = 0.;
                }

            cvReshape( _err, _err, 1, count*2 );
            cvSub( &_mi, _err, _err );
            
            _Je->rows = _Ji->rows = count*2;
            
            cvGetSubRect( _JtJ, &_part, cvRect(0,0,8,8) );
            cvGEMM( _Ji, _Ji, 1, &_part, i > 0, &_part, CV_GEMM_A_T );

            cvGetSubRect( _JtJ, &_part, cvRect(8+i*6,8+i*6,6,6) );
            cvMulTransposed( _Je, &_part, 1 );
            
            cvGetSubRect( _JtJ, &_part, cvRect(8+i*6,0,6,8) );
            cvGEMM( _Ji, _Je, 1, 0, 0, &_part, CV_GEMM_A_T );

            cvGetRows( _JtErr, &_part, 0, 8 );
            cvGEMM( _Ji, _err, 1, &_part, i > 0, &_part, CV_GEMM_A_T );

            cvGetRows( _JtErr, &_part, 8 + i*6, 8 + (i+1)*6 );
            cvGEMM( _Je, _err, 1, 0, 0, &_part, CV_GEMM_A_T );
        }

        // make the matrix JtJ exactly symmetrical and add missing zeros
        for( i = 0; i < param_count; i++ )
        {
            int mj = i < 8 ? param_count : ((i - 8)/6)*6 + 14;
            for( j = i+1; j < mj; j++ )
                jj[j*param_count + i] = jj[i*param_count + j];
            for( ; j < param_count; j++ )
                jj[j*param_count + i] = jj[i*param_count + j] = 0;
        }

        cvSVD( _JtJ, _JtJW, 0, _JtJV, CV_SVD_MODIFY_A + CV_SVD_V_T );
        cvSVBkSb( _JtJW, _JtJV, _JtJV, _JtErr, _param_innov, CV_SVD_U_T + CV_SVD_V_T );

        cvScale( _param_innov, _param_innov, 1. - pow(1. - alpha_smooth, iter + 1.) );
        cvGetRows( _param_innov, &_part, 0, 4 );
        change = cvNorm( &_part );
        cvGetRows( _param, &_part, 0, 4 );
        change /= cvNorm( &_part );

        if( flags & CV_CALIB_FIX_PRINCIPAL_POINT )
            _param_innov->data.db[2] = _param_innov->data.db[3] = 0.;

        if( flags & CV_CALIB_ZERO_TANGENT_DIST )
            _param_innov->data.db[6] = _param_innov->data.db[7] = 0.;

        cvAdd( _param, _param_innov, _param );

        if( flags & CV_CALIB_FIX_ASPECT_RATIO )
            _param->data.db[0] = _param->data.db[1]*aspect_ratio;
        
        a[0] = _param->data.db[0];
        a[4] = _param->data.db[1];
        a[2] = _param->data.db[2];
        a[5] = _param->data.db[3];

        if( change < FLT_EPSILON )
            break;
    }

    cvConvert( &_a, A );
    cvConvert( &_k, dist_coeffs );

    _r_part = cvMat( output_r_matrices ? 3 : 1, 3, r_depth );
    _t_part = cvMat( 1, 3, t_depth );
    for( i = 0; i < img_count; i++ )
    {
        if( r_vecs )
        {
            _sr_part.data.db = _param->data.db + 8 + i*6;
            _r_part.data.ptr = r_vecs->data.ptr + i*r_step;
            if( !output_r_matrices )
                cvConvert( &_sr_part, &_r_part );
            else
            {
                cvRodrigues2( &_sr_part, &_a );
                cvConvert( &_a, &_r_part );
            }
        }
        if( t_vecs )
        {
            _st_part.data.db = _param->data.db + 8 + i*6 + 3;
            _t_part.data.ptr = t_vecs->data.ptr + i*t_step;
            cvConvert( &_st_part, &_t_part );
        }
    }

    __END__;

    cvReleaseMat( &counts );
    cvReleaseMat( &_M );
    cvReleaseMat( &_m );
    cvReleaseMat( &_param );
    cvReleaseMat( &_param_innov );
    cvReleaseMat( &_JtJ );
    cvReleaseMat( &_JtErr );
    cvReleaseMat( &_JtJW );
    cvReleaseMat( &_JtJV );
    cvReleaseMat( &_Ji );
    cvReleaseMat( &_Je );
    cvReleaseMat( &_err );
}

/* End of file. */
