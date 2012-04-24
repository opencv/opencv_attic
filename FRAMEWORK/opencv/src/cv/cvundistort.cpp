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

static CvStatus
icvUnDistort_8u_CnR( const uchar* src, int srcstep,
                     uchar* dst, int dststep, CvSize size,
                     const float* intrinsic_matrix,
                     const float* dist_coeffs, int cn )
{
    int u, v, i;
    float u0 = intrinsic_matrix[2], v0 = intrinsic_matrix[5];
    float fx = intrinsic_matrix[0], fy = intrinsic_matrix[4];
    float _fx = 1.f/fx, _fy = 1.f/fy;
    float k1 = dist_coeffs[0], k2 = dist_coeffs[1];
    float p1 = dist_coeffs[2], p2 = dist_coeffs[3];

    srcstep /= sizeof(src[0]);
    dststep /= sizeof(dst[0]);

    for( v = 0; v < size.height; v++, dst += dststep )
    {
        float y = (v - v0)*_fy;
        float y2 = y*y;
        float ky = 1 + (k1 + k2*y2)*y2;
        float k2y = 2*k2*y2;
        float _2p1y = 2*p1*y;
        float _3p1y2 = 3*p1*y2;
        float p2y2 = p2*y2;

        for( u = 0; u < size.width; u++ )
        {
            float x = (u - u0)*_fx;
            float x2 = x*x;
            float kx = (k1 + k2*x2)*x2;
            float d = kx + ky + k2y*x2;
            float _u = fx*(x*(d + _2p1y) + p2y2 + (3*p2)*x2) + u0;
            float _v = fy*(y*(d + (2*p2)*x) + _3p1y2 + p1*x2) + v0;
            int iu = cvRound(_u*(1 << ICV_WARP_SHIFT));
            int iv = cvRound(_v*(1 << ICV_WARP_SHIFT));
            int ifx = iu & ICV_WARP_MASK;
            int ify = iv & ICV_WARP_MASK;
            iu >>= ICV_WARP_SHIFT;
            iv >>= ICV_WARP_SHIFT;

            float a0 = icvLinearCoeffs[ifx*2];
            float a1 = icvLinearCoeffs[ifx*2 + 1];
            float b0 = icvLinearCoeffs[ify*2];
            float b1 = icvLinearCoeffs[ify*2 + 1];

            if( (unsigned)iv < (unsigned)(size.height - 1) &&
                (unsigned)iu < (unsigned)(size.width - 1) )
            {
                const uchar* ptr = src + iv*srcstep + iu*cn;
                for( i = 0; i < cn; i++ )
                {
                    float t0 = a1*CV_8TO32F(ptr[i]) + a0*CV_8TO32F(ptr[i+cn]);
                    float t1 = a1*CV_8TO32F(ptr[i+srcstep]) + a0*CV_8TO32F(ptr[i + srcstep + cn]);
                    dst[u*cn + i] = (uchar)cvRound(b1*t0 + b0*t1);
                }
            }
            else
            {
                for( i = 0; i < cn; i++ )
                    dst[u*cn + i] = 0;
            }
        }
    }

    return CV_OK;
}


icvUndistortGetSize_t icvUndistortGetSize_p = 0;
icvCreateMapCameraUndistort_32f_C1R_t icvCreateMapCameraUndistort_32f_C1R_p = 0;
icvUndistortRadial_8u_C1R_t icvUndistortRadial_8u_C1R_p = 0;
icvUndistortRadial_8u_C3R_t icvUndistortRadial_8u_C3R_p = 0;

typedef CvStatus (CV_STDCALL * CvUndistortRadialIPPFunc)
    ( const void* pSrc, int srcStep, void* pDst, int dstStep, CvSize roiSize,
      float fx, float fy, float cx, float cy, float k1, float k2, uchar *pBuffer );

CV_IMPL void
cvUndistort2( const CvArr* _src, CvArr* _dst, const CvMat* A, const CvMat* dist_coeffs )
{
    static int inittab = 0;
    uchar* buffer = 0;

    CV_FUNCNAME( "cvUndistort2" );

    __BEGIN__;

    float a[9], k[4];
    int coi1 = 0, coi2 = 0;
    CvMat srcstub, *src = (CvMat*)_src;
    CvMat dststub, *dst = (CvMat*)_dst;
    CvMat _a = cvMat( 3, 3, CV_32F, a ), _k;
    int cn, src_step, dst_step;
    CvSize size;

    if( !inittab )
    {
        icvInitLinearCoeffTab();
        icvInitCubicCoeffTab();
        inittab = 1;
    }

    CV_CALL( src = cvGetMat( src, &srcstub, &coi1 ));
    CV_CALL( dst = cvGetMat( dst, &dststub, &coi2 ));

    if( coi1 != 0 || coi2 != 0 )
        CV_ERROR( CV_BadCOI, "The function does not support COI" );

    if( CV_MAT_DEPTH(src->type) != CV_8U )
        CV_ERROR( CV_StsUnsupportedFormat, "Only 8-bit images are supported" );

    if( src->data.ptr == dst->data.ptr )
        CV_ERROR( CV_StsNotImplemented, "In-place undistortion is not implemented" );

    if( !CV_ARE_TYPES_EQ( src, dst ))
        CV_ERROR( CV_StsUnmatchedFormats, "" );

    if( !CV_ARE_SIZES_EQ( src, dst ))
        CV_ERROR( CV_StsUnmatchedSizes, "" );

    if( !CV_IS_MAT(A) || A->rows != 3 || A->cols != 3  ||
        CV_MAT_TYPE(A->type) != CV_32FC1 && CV_MAT_TYPE(A->type) != CV_64FC1 )
        CV_ERROR( CV_StsBadArg, "Intrinsic matrix must be a valid 3x3 floating-point matrix" );

    if( !CV_IS_MAT(dist_coeffs) || dist_coeffs->rows != 1 && dist_coeffs->cols != 1 ||
        dist_coeffs->rows*dist_coeffs->cols*CV_MAT_CN(dist_coeffs->type) != 4 ||
        CV_MAT_DEPTH(dist_coeffs->type) != CV_64F &&
        CV_MAT_DEPTH(dist_coeffs->type) != CV_32F )
        CV_ERROR( CV_StsBadArg,
            "Distortion coefficients must be 1x4 or 4x1 floating-point vector" );

    cvConvert( A, &_a );
    _k = cvMat( dist_coeffs->rows, dist_coeffs->cols,
                CV_MAKETYPE(CV_32F, CV_MAT_CN(dist_coeffs->type)), k );
    cvConvert( dist_coeffs, &_k );

    cn = CV_MAT_CN(src->type);
    size = cvGetMatSize(src);
    src_step = src->step ? src->step : CV_STUB_STEP;
    dst_step = dst->step ? dst->step : CV_STUB_STEP;

    if( fabs((double)k[2]) < 1e-5 && fabs((double)k[3]) < 1e-5 && icvUndistortGetSize_p )
    {
        int buf_size = 0;
        CvUndistortRadialIPPFunc func =
            cn == 1 ? (CvUndistortRadialIPPFunc)icvUndistortRadial_8u_C1R_p :
                      (CvUndistortRadialIPPFunc)icvUndistortRadial_8u_C3R_p;

        if( func && icvUndistortGetSize_p( size, &buf_size ) >= 0 && buf_size > 0 )
        {
            CV_CALL( buffer = (uchar*)cvAlloc( buf_size ));
            if( func( src->data.ptr, src_step, dst->data.ptr,
                      dst_step, size, a[0], a[4],
                      a[2], a[5], k[0], k[1], buffer ) >= 0 )
                EXIT;
        }
    }

    icvUnDistort_8u_CnR( src->data.ptr, src_step,
        dst->data.ptr, dst_step, size, a, k, cn );

    __END__;

    cvFree( &buffer );
}


CV_IMPL void
cvInitUndistortMap( const CvMat* A, const CvMat* dist_coeffs,
                    CvArr* mapxarr, CvArr* mapyarr )
{
    uchar* buffer = 0;

    CV_FUNCNAME( "cvInitUndistortMap" );

    __BEGIN__;
    
    float a[9], k[4];
    int coi1 = 0, coi2 = 0;
    CvMat mapxstub, *_mapx = (CvMat*)mapxarr;
    CvMat mapystub, *_mapy = (CvMat*)mapyarr;
    float *mapx, *mapy;
    CvMat _a = cvMat( 3, 3, CV_32F, a ), _k;
    int mapxstep, mapystep;
    int u, v;
    float u0, v0, fx, fy, _fx, _fy, k1, k2, p1, p2;
    CvSize size;

    CV_CALL( _mapx = cvGetMat( _mapx, &mapxstub, &coi1 ));
    CV_CALL( _mapy = cvGetMat( _mapy, &mapystub, &coi2 ));

    if( coi1 != 0 || coi2 != 0 )
        CV_ERROR( CV_BadCOI, "The function does not support COI" );

    if( CV_MAT_TYPE(_mapx->type) != CV_32FC1 )
        CV_ERROR( CV_StsUnsupportedFormat, "Both maps must have 32fC1 type" );

    if( !CV_ARE_TYPES_EQ( _mapx, _mapy ))
        CV_ERROR( CV_StsUnmatchedFormats, "" );

    if( !CV_ARE_SIZES_EQ( _mapx, _mapy ))
        CV_ERROR( CV_StsUnmatchedSizes, "" );

    if( !CV_IS_MAT(A) || A->rows != 3 || A->cols != 3  ||
        CV_MAT_TYPE(A->type) != CV_32FC1 && CV_MAT_TYPE(A->type) != CV_64FC1 )
        CV_ERROR( CV_StsBadArg, "Intrinsic matrix must be a valid 3x3 floating-point matrix" );

    if( !CV_IS_MAT(dist_coeffs) || dist_coeffs->rows != 1 && dist_coeffs->cols != 1 ||
        dist_coeffs->rows*dist_coeffs->cols*CV_MAT_CN(dist_coeffs->type) != 4 ||
        CV_MAT_DEPTH(dist_coeffs->type) != CV_64F &&
        CV_MAT_DEPTH(dist_coeffs->type) != CV_32F )
        CV_ERROR( CV_StsBadArg,
            "Distortion coefficients must be 1x4 or 4x1 floating-point vector" );

    cvConvert( A, &_a );
    _k = cvMat( dist_coeffs->rows, dist_coeffs->cols,
                CV_MAKETYPE(CV_32F, CV_MAT_CN(dist_coeffs->type)), k );
    cvConvert( dist_coeffs, &_k );

    u0 = a[2]; v0 = a[5];
    fx = a[0]; fy = a[4];
    _fx = 1.f/fx; _fy = 1.f/fy;
    k1 = k[0]; k2 = k[1];
    p1 = k[2]; p2 = k[3];

    mapxstep = _mapx->step ? _mapx->step : CV_STUB_STEP;
    mapystep = _mapy->step ? _mapy->step : CV_STUB_STEP;
    mapx = _mapx->data.fl;
    mapy = _mapy->data.fl;

    size = cvGetMatSize(_mapx);
    
    /*if( icvUndistortGetSize_p && icvCreateMapCameraUndistort_32f_C1R_p )
    {
        int buf_size = 0;
        if( icvUndistortGetSize_p( size, &buf_size ) && buf_size > 0 )
        {
            CV_CALL( buffer = (uchar*)cvAlloc( buf_size ));
            if( icvCreateMapCameraUndistort_32f_C1R_p(
                mapx, mapxstep, mapy, mapystep, size,
                a[0], a[4], a[2], a[5], k[0], k[1], k[2], k[3], buffer ) >= 0 )
                EXIT;
        }
    }*/
    
    mapxstep /= sizeof(mapx[0]);
    mapystep /= sizeof(mapy[0]);

    for( v = 0; v < size.height; v++, mapx += mapxstep, mapy += mapystep )
    {
        float y = (v - v0)*_fy;
        float y2 = y*y;
        float _2p1y = 2*p1*y;
        float _3p1y2 = 3*p1*y2;
        float p2y2 = p2*y2;

        for( u = 0; u < size.width; u++ )
        {
            float x = (u - u0)*_fx;
            float x2 = x*x;
            float r2 = x2 + y2;
            float d = 1 + (k1 + k2*r2)*r2;
            float _u = fx*(x*(d + _2p1y) + p2y2 + (3*p2)*x2) + u0;
            float _v = fy*(y*(d + (2*p2)*x) + _3p1y2 + p1*x2) + v0;
            mapx[u] = _u;
            mapy[u] = _v;
        }
    }

    __END__;

    cvFree( &buffer );
}

/*  End of file  */
