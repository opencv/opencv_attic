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

typedef struct CvUnDistortData
{
    int ind;
    ushort a0;
    ushort a1;
    ushort a2;
    ushort a3;
}
CvUnDistortData;

/*F//////////////////////////////////////////////////////////////////////////////////////
//    Names: icvUnDistortInit_8uC1R, icvUnDistortInit_8uC3R,
//    Purpose: The functions calculate arrays of distorted points indices and
//             interpolation coefficients for cvUnDistort function
//    Context:
//    Parameters:  size        - size of each image or of its ROI
//                 step        - full width of each image in bytes
//                 intrMatrix  - matrix of the camera intrinsic parameters
//                 distCoeffs  - vector of the distortion coefficients (k1, k2, p1 and p2)
//                 interToggle - interpolation toggle
//                 data        - distortion data array
//
//    Returns: CV_NO_ERR or error code
//
//    Notes:  1. If interToggle=0, interpolation disabled;
//               else bilinear interpolation is used.
//            2. Array data must be allocated before. If interToggle = 0, its length
//               must be size.width*size.height elements; else 3*size.width*size.height.
//F*/
/*______________________________________________________________________________________*/

IPCVAPI_IMPL( CvStatus, icvUnDistortInit,
    ( int srcStep, int* data, int mapStep, CvSize size, const float *intrMatrix,
      const float *distCoeffs, int interToggle, int pixSize ),
      (srcStep, data, mapStep, size, intrMatrix, distCoeffs, interToggle, pixSize) )
{
    const float a1 = 1.f / intrMatrix[0], b1 = 1.f / intrMatrix[4],
        u0 = intrMatrix[2], v0 = intrMatrix[5],
        k1 = distCoeffs[0], k2 = distCoeffs[1], p1 = distCoeffs[2], p2 = distCoeffs[3];
    int u, v;
    float p22 = 2.f * p2;

    if( size.width <= 0 || size.height <= 0 )
        return CV_BADSIZE_ERR;

    if( !intrMatrix || !distCoeffs || !data )
        return CV_NULLPTR_ERR;

    if( !interToggle )
    {
        for( v = 0; v < size.height; v++, (char*&)data += mapStep )
        {
            float dv = v - v0;
            float y = b1 * dv;
            float y1 = p1 / (y?y:0.1f);
            float y2 = y * y;
            float y3 = 2.f * p1 * y;

            for( u = 0; u < size.width; u++ )
            {
                float du = u - u0;
                float x = a1 * du;
                float x1 = p2 / (x?x:0.1f);
                float x2 = x * x;
                float x3 = p22 * x;
                float r2 = x2 + y2;
                float bx = r2 * (k1 + r2 * k2) + x3 + y3;
                float by = bx + r2 * y1;
                int ud = u, vd = v;

                bx += r2 * x1;
                ud += cvRound( bx * du );
                vd += cvRound( by * dv );
                data[u] = ud < 0 || ud >= size.width || vd < 0 || vd >= size.height ?
                    0 : vd * srcStep + ud*pixSize;
            }
        }
    }
    else                        /* interpolation */
    {
        const int sizex = size.width - 2, sizey = size.height - 2;
        CvUnDistortData *uData = (CvUnDistortData *) data;

        const float s15 = 32768.f;
        const int bmax = 32767;

        for( v = 0; v < size.height; v++, (char*&)uData += mapStep )
        {
            float dv = v - v0;
            float y = b1 * dv;
            float y1 = p1 / (y?y:0.1f);
            float y2 = y * y;
            float y3 = 2.f * p1 * y;

            for( u = 0; u < size.width; u++ )
            {
                float du = u - u0;
                float x = a1 * du;
                float x1 = p2 / (x?x:0.1f);
                float x2 = x * x;
                float x3 = p22 * x;
                float r2 = x2 + y2;
                float bx = r2 * (k1 + r2 * k2) + x3 + y3;
                float by = bx + r2 * y1;
                float uf = (float) u, vf = (float) v;
                int ud, vd;

                bx += r2 * x1;
                uf += bx * du;
                vf += by * dv;
                ud = cvFloor( uf );
                vd = cvFloor( vf );
                if( ud < 0 || ud > sizex || vd < 0 || vd > sizey )
                {
                    (uData + u)->ind = 0;
                    (uData + u)->a0 = (uData + u)->a1 = (uData + u)->a2 = (uData + u)->a3 =
                    (ushort) 0;
                }
                else
                {
                    float uf1, vf1;
                    int b0, b1, b2, b3;

                    (uData + u)->ind = vd * srcStep + ud*pixSize;
                    uf -= (float) ud;
                    vf -= (float) vd;
                    uf1 = 1.f - uf;
                    vf1 = 1.f - vf;
                    b0 = (int) (s15 * uf1 * vf1);
                    b1 = (int) (s15 * uf * vf1);
                    b2 = (int) (s15 * uf1 * vf);
                    b3 = (int) (s15 * uf * vf);
                    if( b0 < 0 )
                        b0 = 0;
                    if( b1 < 0 )
                        b1 = 0;
                    if( b2 < 0 )
                        b2 = 0;
                    if( b3 < 0 )
                        b3 = 0;
                    if( b0 > bmax )
                        b0 = bmax;
                    if( b1 > bmax )
                        b1 = bmax;
                    if( b2 > bmax )
                        b2 = bmax;
                    if( b3 > bmax )
                        b3 = bmax;
                    (uData + u)->a0 = (ushort) b0;
                    (uData + u)->a1 = (ushort) b1;
                    (uData + u)->a2 = (ushort) b2;
                    (uData + u)->a3 = (ushort) b3;
                }
            }                   /* u */
        }                       /* v */
    }                           /* else */

    return CV_NO_ERR;
}

/*======================================================================================*/

/*F//////////////////////////////////////////////////////////////////////////////////////
//    Names: icvUnDistort_8u_C1R, icvUnDistort_8u_C3R
//    Purpose: The functions correct radial and tangential distortion in the frame
//             using previousely calculated arrays of distorted points indices and
//             undistortion coefficients
//    Context:
//    Parameters:  src    - source (distorted) image
//                 dst    - output (undistorted) image
//                 step        - full width of each image in bytes
//                 size        - size of each image or of its ROI
//                 interToggle - interpolation toggle
//                 data        - distortion data array
//
//    Returns: CV_NO_ERR or error code
//
//    Notes:  1. Either icvUnDistortInit_8u_C1R or icvUnDistortInit_8u_C3R function
//               must be used previously
//            2. See Notes to the icvUnDistortInit_8u_C1R, icvUnDistortInit_8u_C3R
//               functions
//F*/
/*______________________________________________________________________________________*/

IPCVAPI_IMPL( CvStatus, icvUnDistort_8u_C1R,
    ( const uchar* src, int srcStep, const int* data, int mapStep,
      uchar* dst, int dstStep, CvSize size, int interToggle ),
      (src, srcStep, data, mapStep, dst, dstStep, size, interToggle) )
{
    int u, v;
    uchar buf;

    if( size.width <= 0 || size.height <= 0 )
        return CV_BADSIZE_ERR;

    if( !src || !dst || !data )
        return CV_NULLPTR_ERR;

    buf = *src;
    *(uchar*&)src = (uchar)0;

    if( !interToggle )          /* Interpolation disabled */
    {
        for( v = 0; v < size.height; v++, dst += dstStep, (char*&)data += mapStep )
        {
            for( u = 0; u <= size.width - 4; u += 4 )
            {
                uchar t0 = src[data[u]];
                uchar t1 = src[data[u+1]];

                dst[u] = t0;
                dst[u + 1] = t1;

                t0 = src[data[u+2]];
                t1 = src[data[u+3]];

                dst[u + 2] = t0;
                dst[u + 3] = t1;
            }

            for( ; u < size.width; u++ )
            {
                dst[u] = src[data[u]];
            }
        }
    }
    else /* Interpolation enabled */
    {
        CvUnDistortData *uData = (CvUnDistortData *) data;

        for( v = 0; v < size.height; v++, dst += dstStep, (char*&)uData += mapStep )
        {
            for( u = 0; u < size.width; u++ )
            {
                CvUnDistortData d = uData[u];
                const uchar* s = src + d.ind;

                dst[u] = (uchar)((s[0]*d.a0 + s[1]*d.a1 +
                        s[srcStep]*d.a2 + s[srcStep+1]*d.a3) >> 15);
            }
        }
    }

    *(uchar*&)src = buf;
    return CV_NO_ERR;
}


#define ICV_COPY_C3( src_ptr, dst_ptr ) \
    (dst_ptr)[0] = (src_ptr)[0];        \
    t0 = (src_ptr)[1];                  \
    t1 = (src_ptr)[2];                  \
    (dst_ptr)[1] = t0;                  \
    (dst_ptr)[2] = t1


/*_____________________________ 3-CHANNEL IMAGES _______________________________*/

IPCVAPI_IMPL( CvStatus, icvUnDistort_8u_C3R,
    ( const uchar* src, int srcStep, const int* data, int mapStep,
      uchar* dst, int dstStep, CvSize size, int interToggle ),
      (src, srcStep, data, mapStep, dst, dstStep, size, interToggle) )
{
    int u, v;
    uchar buf[3];

    if( size.width <= 0 || size.height <= 0 )
        return CV_BADSIZE_ERR;

    if( !src || !dst || !data )
        return CV_NULLPTR_ERR;

    memcpy( buf, src, 3 );
    memset( (void*)src, 0, 3 );

    if( !interToggle )          /* Interpolation disabled */
    {
        for( v = 0; v < size.height; v++, dst += dstStep, (char*&)data += mapStep )
        {
            for( u = 0; u <= size.width - 4; u += 4, dst += 12 )
            {
                uchar t0, t1;

                int v3 = data[u];
                ICV_COPY_C3( src + v3, dst );

                v3 = data[u + 1];
                ICV_COPY_C3( src + v3, dst + 3 );

                v3 = data[u + 2];
                ICV_COPY_C3( src + v3, dst + 6 );

                v3 = data[u + 3];
                ICV_COPY_C3( src + v3, dst + 9 );
            }

            for( ; u < size.width; u++, dst += 3 )
            {
                int v3 = data[u];
                uchar t0, t1;

                ICV_COPY_C3( src + v3, dst );
            }

            dst -= size.width * 3;
        }
    }
    else  /* Interpolation enabled */
    {
        CvUnDistortData *uData = (CvUnDistortData *) data;

        for( v = 0; v < size.height; v++, dst += dstStep, (char*&)uData += mapStep )
        {
            for( u = 0; u < size.width; u++, dst += 3 )
            {
                CvUnDistortData d = uData[u];
                const uchar* s = src + d.ind;

                dst[0] = (uchar) ((s[0] * d.a0 + s[3] * d.a1 +
                                   s[srcStep] * d.a2 + s[srcStep + 3] * d.a3) >> 15);

                dst[1] = (uchar) ((s[1] * d.a0 + s[4] * d.a1 +
                                   s[srcStep + 1] * d.a2 + s[srcStep + 4] * d.a3) >> 15);

                dst[2] = (uchar) ((s[2] * d.a0 + s[5] * d.a1 +
                                   s[srcStep + 2] * d.a2 + s[srcStep + 5] * d.a3) >> 15);
            }

            dst -= size.width * 3;
        }
    }

    memcpy( (void*)src, buf, 3 );
    return CV_NO_ERR;
}

/*======================================================================================*/

/*F//////////////////////////////////////////////////////////////////////////////////////
//    Names: icvUnDistort1_8uC1R, icvUnDistort1_8uC3R
//    Purpose: The functions correct radial image distortion using known matrix of the
//             camera intrinsic parameters and distortion coefficients
//    Context:
//    Parameters:  src    - source (distorted) image
//                 dst    - output (undistorted) image
//                 step        - full width of each image in bytes
//                 size        - ROI size of each image
//                 intrMatrix  - matrix of the camera intrinsic parameters
//                 distCoeffs  - vector of two distortion coefficients (k1 and k2)
//                 interToggle - interpolation toggle
//
//    Returns: CV_NO_ERR or error code
//
//    Notes:   If interToggle=0, interpolation disabled;
//             else bilinear interpolation is used.
//F*/
/*______________________________________________________________________________________*/

#define S  22
#define S2 11
#define FM (float)0x400000

IPCVAPI_IMPL( CvStatus, icvUnDistort1_8u_C1R,
    ( const uchar* src, int srcStep, uchar* dst, int dstStep, CvSize size,
      const float *intrMatrix, const float *distCoeffs, int interToggle ),
      (src, srcStep, dst, dstStep, size, intrMatrix, distCoeffs, interToggle) )
{
    const float fm = FM;
    const float a1 = 1.f / intrMatrix[0], b1 = 1.f / intrMatrix[4],
        u0 = intrMatrix[2], v0 = intrMatrix[5], k1 = distCoeffs[0], k2 = distCoeffs[1];
    float *x2;
    float *du;
    int u, v;
    uchar buf[1];

    if( size.width <= 0 || size.height <= 0 )
        return CV_BADSIZE_ERR;
    if( !src || !dst || !intrMatrix || !distCoeffs )
        return CV_NULLPTR_ERR;

    x2 = (float *) cvAlloc( sizeof( float ) * size.width );

    if( x2 == NULL )
        return CV_OUTOFMEM_ERR;
    du = (float *) cvAlloc( sizeof( float ) * size.width );

    if( du == NULL )
    {
        cvFree( (void **) &x2 );
        return CV_OUTOFMEM_ERR;
    }

    memcpy( buf, src, 1 );
    memset( (void*)src, 0, 1 );

    if( !interToggle )
    {
        for( u = 0; u < size.width; u++ )
        {
            float w = u - u0;
            float x = a1 * w;

            x2[u] = x * x;
            du[u] = w;
        }
        for( v = 0; v < size.height; v++, dst += dstStep )
        {
            float y = b1 * (v - v0);
            float y2 = y * y;
            float dv = v - v0;

            for( u = 0; u < size.width; u++ )
            {
                float r2 = x2[u] + y2;
                float dist = r2 * (k1 + r2 * k2);
                int ud = u + cvRound( du[u] * dist );
                int vd = v + cvRound( dv * dist );

                dst[u] = (uchar) (ud < 0 || ud >= size.width || vd < 0 ||
                                       vd >= size.height ? 0 : src[vd * srcStep + ud]);
            }
        }
    }
    else
    {
        int sizex = size.width - 2, sizey = size.height - 2;

        for( u = 0; u < size.width; u++ )
        {
            float w = u - u0;
            float x = a1 * w;

            x2[u] = x * x;
            du[u] = fm * w;
        }
        for( v = 0; v < size.height; v++, dst += dstStep )
        {
            float y = b1 * (v - v0);
            float y2 = y * y;
            float dv = fm * (v - v0);

            for( u = 0; u < size.width; u++ )
            {
                float r2 = x2[u] + y2;
                float dist = r2 * (k1 + r2 * k2);
                int iu = cvRound( du[u] * dist );
                int iv = cvRound( dv * dist );
                int ud = iu >> S;
                int vd = iv >> S;

                iu -= ud << S;
                iv -= vd << S;
                ud += u;
                vd += v;

                if( ud < 0 || ud > sizex || vd < 0 || vd > sizey )
                    dst[u] = 0;
                else
                {
                    int iuv = (iu >> S2) * (iv >> S2);
                    int uv = vd * srcStep + ud;
                    int a0 = src[uv];
                    int a = src[uv + 1] - a0;
                    int b = src[uv + srcStep] - a0;
                    int c = src[uv + srcStep + 1] - a0 - a - b;
                    int d = ((a * iu + b * iv + c * iuv) >> S) + a0;

                    d = !(d & ~255) ? d : d < 0 ? 0 : 255;
                    dst[u] = (uchar) d;
                }
            }
        }
    }

    memcpy( (void*)src, buf, 1 );

    cvFree( (void **) &x2 );
    cvFree( (void **) &du );
    return CV_NO_ERR;
}
/*______________________________________________________ 3-CHANNEL IMAGES ______________*/

IPCVAPI_IMPL( CvStatus, icvUnDistort1_8u_C3R,
    ( const uchar* src, int srcStep, uchar* dst, int dstStep, CvSize size,
      const float *intrMatrix, const float *distCoeffs, int interToggle ),
      (src, srcStep, dst, dstStep, size, intrMatrix, distCoeffs, interToggle) )
{
    const float fm = FM;
    const float a1 = 1.f / intrMatrix[0], b1 = 1.f / intrMatrix[4],
        u0 = intrMatrix[2], v0 = intrMatrix[5], k1 = distCoeffs[0], k2 = distCoeffs[1];
    const int p = 0xFFFFFF;
    float *x2;
    float *du;
    int u, v;
    uchar buf[3];

    if( size.width <= 0 || size.height <= 0 )
        return CV_BADSIZE_ERR;
    if( !src || !dst || !intrMatrix || !distCoeffs )
        return CV_NULLPTR_ERR;

    x2 = (float *) cvAlloc( sizeof( float ) * size.width );

    if( x2 == NULL )
        return CV_OUTOFMEM_ERR;
    du = (float *) cvAlloc( sizeof( float ) * size.width );

    if( du == NULL )
    {
        cvFree( (void **) &x2 );
        return CV_OUTOFMEM_ERR;
    }

    memcpy( buf, src, 3 );
    memset( (void*)src, 0, 3 );

    if( !interToggle )
    {
        for( u = 0; u < size.width; u++ )
        {
            float w = u - u0;
            float x = a1 * w;

            x2[u] = x * x;
            du[u] = w;
        }

        for( v = 0; v < size.height; v++, dst += dstStep )
        {
            float y = b1 * (v - v0);
            float y2 = y * y;
            float dv = v - v0;

            if( v == size.height - 1 )
                size.width--;
            for( u = 0; u < size.width; u++ )
            {
                float r2 = x2[u] + y2;
                float dist = r2 * (k1 + r2 * k2);
                int ud = u + cvRound( du[u] * dist );
                int vd = v + cvRound( dv * dist );
                int u3 = u + u + u;

                if( ud < 0 || ud >= size.width || vd < 0 || vd >= size.height )
                    *(int *) (dst + u3) = 0;
                else
                {
                    int uv = vd * srcStep + ud + ud + ud;

                    *(int *) (dst + u3) = *(int *) (src + uv) & p;
                }
            }
            if( v == size.height - 1 )
            {
                float r2 = x2[u] + y2;
                float dist = r2 * (k1 + r2 * k2);
                int ud = u + cvRound( du[u] * dist );
                int vd = v + cvRound( dv * dist );
                int u3 = u + u + u;

                size.width++;
                if( ud < 0 || ud >= size.width || vd < 0 || vd >= size.height )
                    dst[u3] = dst[u3 + 1] = dst[u3 + 2] = 0;
                else
                {
                    int uv = vd * srcStep + ud + ud + ud;

                    dst[u3] = src[uv];
                    dst[u3 + 1] = src[uv + 1];
                    dst[u3 + 2] = src[uv + 2];
                }
            }
        }
    }
    else                        /* interpolation */
    {
        int sizex = size.width - 2, sizey = size.height - 2;

        for( u = 0; u < size.width; u++ )
        {
            float w = u - u0;
            float x = a1 * w;

            x2[u] = x * x;
            du[u] = fm * w;
        }

        for( v = 0; v < size.height; v++, dst += dstStep )
        {
            float y = b1 * (v - v0);
            float y2 = y * y;
            float dv = fm * (v - v0);

            if( v == size.height - 1 )
                size.width--;
            for( u = 0; u < size.width; u++ )
            {
                float r2 = x2[u] + y2;
                float dist = r2 * (k1 + r2 * k2);
                int iu = cvRound( du[u] * dist );
                int iv = cvRound( dv * dist );
                int ud = iu >> S;
                int vd = iv >> S;
                int u3 = u + u + u;

                iu -= ud << S;
                iv -= vd << S;
                ud += u;
                vd += v;

                if( ud < 0 || ud > sizex || vd < 0 || vd > sizey )
                    *(int *) (dst + u3) = 0;
                else
                {
                    int iuv = (iu >> S2) * (iv >> S2);
                    int uv = vd * srcStep + ud + ud + ud;
                    int uvs = uv + srcStep;
                    int a01 = src[uv];
                    int a02 = src[uv + 1];
                    int a03 = src[uv + 2];
                    int a1 = src[uv + 3];
                    int a2 = src[uv + 4];
                    int a3 = src[uv + 5];
                    int b1 = src[uvs];
                    int b2 = src[uvs + 1];
                    int b3 = src[uvs + 2];
                    int c1 = src[uvs + 3];
                    int c2 = src[uvs + 4];
                    int c3 = src[uvs + 5];
                    int d1, d2, d3;

                    a1 -= a01;
                    a2 -= a02;
                    a3 -= a03;
                    b1 -= a01;
                    b2 -= a02;
                    b3 -= a03;
                    c1 -= a01 + a1 + b1;
                    c2 -= a02 + a2 + b2;
                    c3 -= a03 + a3 + b3;
                    d1 = ((a1 * iu + b1 * iv + c1 * iuv) >> S) + a01;
                    d2 = ((a2 * iu + b2 * iv + c2 * iuv) >> S) + a02;
                    d3 = ((a3 * iu + b3 * iv + c3 * iuv) >> S) + a03;
                    d1 = !(d1 & ~255) ? d1 : d1 < 0 ? 0 : 255;
                    d2 = !(d2 & ~255) ? d2 : d2 < 0 ? 0 : 255;
                    d3 = !(d3 & ~255) ? d3 : d3 < 0 ? 0 : 255;
                    d1 |= (d2 << 8) | (d3 << 16);
                    *(int *) (dst + u3) = d1;
                }
            }
            if( v == size.height - 1 )
            {
                float r2 = x2[u] + y2;
                float dist = r2 * (k1 + r2 * k2);
                int iu = cvRound( du[u] * dist );
                int iv = cvRound( dv * dist );
                int ud = iu >> S;
                int vd = iv >> S;
                int u3 = u + u + u;

                size.width++;
                iu -= ud << S;
                iv -= vd << S;
                ud += u;
                vd += v;

                if( ud < 0 || ud > sizex || vd < 0 || vd > sizey )
                    dst[u3] = dst[u3 + 1] = dst[u3 + 2] = 0;
                else
                {
                    int iuv = (iu >> S2) * (iv >> S2);
                    int uv = vd * srcStep + ud + ud + ud;
                    int uvs = uv + srcStep;
                    int a01 = src[uv];
                    int a02 = src[uv + 1];
                    int a03 = src[uv + 2];
                    int a1 = src[uv + 3];
                    int a2 = src[uv + 4];
                    int a3 = src[uv + 5];
                    int b1 = src[uvs];
                    int b2 = src[uvs + 1];
                    int b3 = src[uvs + 2];
                    int c1 = src[uvs + 3];
                    int c2 = src[uvs + 4];
                    int c3 = src[uvs + 5];
                    int d1, d2, d3;

                    a1 -= a01;
                    a2 -= a02;
                    a3 -= a03;
                    b1 -= a01;
                    b2 -= a02;
                    b3 -= a03;
                    c1 -= a01 + a1 + b1;
                    c2 -= a02 + a2 + b2;
                    c3 -= a03 + a3 + b3;
                    d1 = ((a1 * iu + b1 * iv + c1 * iuv) >> S) + a01;
                    d2 = ((a2 * iu + b2 * iv + c2 * iuv) >> S) + a02;
                    d3 = ((a3 * iu + b3 * iv + c3 * iuv) >> S) + a03;
                    dst[u3] = (uchar) d1;
                    dst[u3 + 1] = (uchar) d2;
                    dst[u3 + 2] = (uchar) d3;
                }
            }
        }
    }

    memcpy( (void*)src, buf, 1 );

    cvFree( (void **) &x2 );
    cvFree( (void **) &du );
    return CV_NO_ERR;
}

/*======================================================================================*/

/*F//////////////////////////////////////////////////////////////////////////////////////
//    Names: icvUnDistortEx_8uC1R, icvUnDistortEx_8uC3R
//    Purpose: The functions correct radial and tangential image distortions using known
//             matrix of the camera intrinsic parameters and distortion coefficients
//    Context:
//    Parameters:  src    - source (distorted) image
//                 dst    - output (undistorted) image
//                 step        - full width of each image in bytes
//                 size        - ROI size of each image
//                 intrMatrix  - matrix of the camera intrinsic parameters
//                 distCoeffs  - vector of the distortion coefficients
//                               (k1, k2, p1, p2)
//                 interToggle - interpolation toggle
//
//    Returns: CV_NO_ERR or error code
//
//    Notes:   If interToggle=0, interpolation disabled;
//             else bilinear interpolation is used.
//F*/
/*______________________________________________________________________________________*/

IPCVAPI_IMPL( CvStatus, icvUnDistortEx_8u_C1R,
    ( const uchar* src, int srcStep, uchar* dst, int dstStep, CvSize size,
      const float *intrMatrix, const float *distCoeffs, int interToggle ),
      (src, srcStep, dst, dstStep, size, intrMatrix, distCoeffs, interToggle) )
{
    const float fm = FM;
    const float a1 = 1.f / intrMatrix[0], b1 = 1.f / intrMatrix[4],
        u0 = intrMatrix[2], v0 = intrMatrix[5],
        k1 = distCoeffs[0], k2 = distCoeffs[1], p1 = distCoeffs[2], p2 = distCoeffs[3];
    float *x1;
    float *x2;
    float *x3;
    float *du;
    int u, v;
    uchar buf[1];

    if( size.width <= 0 || size.height <= 0 )
        return CV_BADSIZE_ERR;
    if( !src || !dst || !intrMatrix || !distCoeffs )
        return CV_NULLPTR_ERR;

    /*if ( !p1 && !p2 ) return icvUnDistort1_8uC1R ( src, dst, step, size,
       intrMatrix, distCoeffs, interToggle ); */

    x1 = (float *) cvAlloc( sizeof( float ) * size.width );
    x2 = (float *) cvAlloc( sizeof( float ) * size.width );
    x3 = (float *) cvAlloc( sizeof( float ) * size.width );
    du = (float *) cvAlloc( sizeof( float ) * size.width );

    if( x1 == NULL || x2 == NULL || x3 == NULL || du == NULL )
    {
        if( x1 )
            cvFree( (void **) &x1 );
        if( x2 )
            cvFree( (void **) &x2 );
        if( x3 )
            cvFree( (void **) &x3 );
        if( du )
            cvFree( (void **) &du );
        return CV_OUTOFMEM_ERR;
    }

    memcpy( buf, src, 1 );
    memset( (void*)src, 0, 1 );

    if( !interToggle )
    {
        for( u = 0; u < size.width; u++ )
        {
            float w = u - u0;
            float x = a1 * w;

            x1[u] = p2 / (x?x:0.1f);
            x2[u] = x * x;
            x3[u] = 2.f * p2 * x;
            du[u] = w;
        }

        for( v = 0; v < size.height; v++, dst += dstStep )
        {
            float dv = v - v0;
            float y = b1 * dv;
            float y1 = p1 / (y?y:0.1f);
            float y2 = y * y;
            float y3 = 2.f * p1 * y;

            for( u = 0; u < size.width; u++ )
            {
                float r2 = x2[u] + y2;
                float bx = r2 * (k1 + r2 * k2) + x3[u] + y3;
                float by = bx + r2 * y1;
                int ud = u, vd = v;

                bx += r2 * x1[u];
                ud += cvRound( bx * du[u] );
                vd += cvRound( by * dv );
                dst[u] = (uchar) (ud < 0 || ud >= size.width || vd < 0 ||
                                       vd >= size.height ? 0 : src[vd * srcStep + ud]);
            }
        }
    }
    else                        /* interpolation */
    {
        int sizex = size.width - 2, sizey = size.height - 2;

        for( u = 0; u < size.width; u++ )
        {
            float w = u - u0;
            float x = a1 * w;

            x1[u] = p2 / (x?x:0.1f);
            x2[u] = x * x;
            x3[u] = 2.f * p2 * x;
            du[u] = fm * w;
        }

        for( v = 0; v < size.height; v++, dst += dstStep )
        {
            float dv = v - v0;
            float y = b1 * dv;
            float y1 = p1 / (y?y:0.1f);
            float y2 = y * y;
            float y3 = 2.f * p1 * y;

            dv *= fm;

            for( u = 0; u < size.width; u++ )
            {
                float r2 = x2[u] + y2;
                float bx = r2 * (k1 + r2 * k2) + x3[u] + y3;
                float by = bx + r2 * y1;
                int iu, iv, ud, vd;

                bx += r2 * x1[u];
                iu = cvRound( bx * du[u] );
                iv = cvRound( by * dv );
                ud = iu >> S;
                vd = iv >> S;
                iu -= ud << S;
                iv -= vd << S;
                ud += u;
                vd += v;

                if( ud < 0 || ud > sizex || vd < 0 || vd > sizey )
                    dst[u] = 0;
                else
                {
                    int iuv = (iu >> S2) * (iv >> S2);
                    int uv = vd * srcStep + ud;
                    int a0 = src[uv];
                    int a = src[uv + 1] - a0;
                    int b = src[uv + srcStep] - a0;
                    int c = src[uv + srcStep + 1] - a0 - a - b;
                    int d = ((a * iu + b * iv + c * iuv) >> S) + a0;

                    d = !(d & ~255) ? d : d < 0 ? 0 : 255;
                    dst[u] = (uchar) d;
                }
            }
        }
    }

    memcpy( (void*)src, buf, 1 );

    cvFree( (void **) &x1 );
    cvFree( (void **) &x2 );
    cvFree( (void **) &x3 );
    cvFree( (void **) &du );
    return CV_NO_ERR;
}
/*______________________________________________________ 3-CHANNEL IMAGES ______________*/

IPCVAPI_IMPL( CvStatus, icvUnDistortEx_8u_C3R,
    ( const uchar* src, int srcStep, uchar* dst, int dstStep, CvSize size,
      const float *intrMatrix, const float *distCoeffs, int interToggle ),
     (src, srcStep, dst, dstStep, size, intrMatrix, distCoeffs, interToggle) )
{
    const float fm = FM;
    const float a1 = 1.f / intrMatrix[0], b1 = 1.f / intrMatrix[4],
        u0 = intrMatrix[2], v0 = intrMatrix[5],
        k1 = distCoeffs[0], k2 = distCoeffs[1], p1 = distCoeffs[2], p2 = distCoeffs[3];
    const int p = 0xFFFFFF;
    float *x1;
    float *x2;
    float *x3;
    float *du;
    int u, v;
    uchar buf[3];

    if( size.width <= 0 || size.height <= 0 )
        return CV_BADSIZE_ERR;
    if( !src || !dst || !intrMatrix || !distCoeffs )
        return CV_NULLPTR_ERR;

    /*if ( !p1 && !p2 ) return icvUnDistort1_8uC3R ( src, dst, srcStep, size,
       intrMatrix, distCoeffs, interToggle ); */

    x1 = (float *) cvAlloc( sizeof( float ) * size.width );
    x2 = (float *) cvAlloc( sizeof( float ) * size.width );
    x3 = (float *) cvAlloc( sizeof( float ) * size.width );
    du = (float *) cvAlloc( sizeof( float ) * size.width );

    if( x1 == NULL || x2 == NULL || x3 == NULL || du == NULL )
    {
        if( x1 )
            cvFree( (void **) &x1 );
        if( x2 )
            cvFree( (void **) &x2 );
        if( x3 )
            cvFree( (void **) &x3 );
        if( du )
            cvFree( (void **) &du );
        return CV_OUTOFMEM_ERR;
    }

    memcpy( buf, src, 3 );
    memset( (void*)src, 0, 3 );

    if( !interToggle )
    {
        for( u = 0; u < size.width; u++ )
        {
            float w = u - u0;
            float x = a1 * w;

            x1[u] = p2 / (x?x:0.1f);
            x2[u] = x * x;
            x3[u] = 2.f * p2 * x;
            du[u] = w;
        }

        for( v = 0; v < size.height; v++, dst += dstStep )
        {
            float dv = v - v0;
            float y = b1 * dv;
            float y1 = p1 / (y?y:0.1f);
            float y2 = y * y;
            float y3 = 2.f * p1 * y;

            if( v == size.height - 1 )
                size.width--;

            for( u = 0; u < size.width; u++ )
            {
                float r2 = x2[u] + y2;
                float bx = r2 * (k1 + r2 * k2) + x3[u] + y3;
                float by = bx + r2 * y1;
                int ud = u, vd = v, u3 = u + u + u;

                bx += r2 * x1[u];
                ud += cvRound( bx * du[u] );
                vd += cvRound( by * dv );
                if( ud < 0 || ud >= size.width || vd < 0 || vd >= size.height )
                    *(int *) (dst + u3) = 0;
                else
                {
                    int uv = vd * srcStep + ud + ud + ud;

                    *(int *) (dst + u3) = *(int *) (src + uv) & p;
                }
            }

            if( v == size.height - 1 )
            {
                float r2 = x2[u] + y2;
                float bx = r2 * (k1 + r2 * k2) + x3[u] + y3;
                float by = bx + r2 * y1;
                int ud = u, vd = v, u3 = u + u + u;

                bx += r2 * x1[u];
                ud += cvRound( bx * du[u] );
                vd += cvRound( by * dv );
                size.width++;
                if( ud < 0 || ud >= size.width || vd < 0 || vd >= size.height )
                    dst[u3] = dst[u3 + 1] = dst[u3 + 2] = 0;
                else
                {
                    int uv = vd * srcStep + ud + ud + ud;

                    dst[u3] = src[uv];
                    dst[u3 + 1] = src[uv + 1];
                    dst[u3 + 2] = src[uv + 2];
                }
            }
        }
    }
    else                        /* interpolation */
    {
        int sizex = size.width - 2, sizey = size.height - 2;

        for( u = 0; u < size.width; u++ )
        {
            float w = u - u0;
            float x = a1 * w;

            x1[u] = p2 / (x?x:0.1f);
            x2[u] = x * x;
            x3[u] = 2.f * p2 * x;
            du[u] = fm * w;
        }

        for( v = 0; v < size.height; v++, dst += dstStep )
        {
            float dv = v - v0;
            float y = b1 * dv;
            float y1 = p1 / (y?y:0.1f);
            float y2 = y * y;
            float y3 = 2.f * p1 * y;

            dv *= fm;

            if( v == size.height - 1 )
                size.width--;

            for( u = 0; u < size.width; u++ )
            {
                float r2 = x2[u] + y2;
                float bx = r2 * (k1 + r2 * k2) + x3[u] + y3;
                float by = bx + r2 * y1;
                int iu, iv, ud, vd, u3 = u + u + u;

                bx += r2 * x1[u];
                iu = cvRound( bx * du[u] );
                iv = cvRound( by * dv );
                ud = iu >> S;
                vd = iv >> S;
                iu -= ud << S;
                iv -= vd << S;
                ud += u;
                vd += v;

                if( ud < 0 || ud > sizex || vd < 0 || vd > sizey )
                    *(int *) (dst + u3) = 0;
                else
                {
                    int iuv = (iu >> S2) * (iv >> S2);
                    int uv = vd * srcStep + ud + ud + ud;
                    int uvs = uv + srcStep;
                    int a01 = src[uv];
                    int a02 = src[uv + 1];
                    int a03 = src[uv + 2];
                    int a1 = src[uv + 3];
                    int a2 = src[uv + 4];
                    int a3 = src[uv + 5];
                    int b1 = src[uvs];
                    int b2 = src[uvs + 1];
                    int b3 = src[uvs + 2];
                    int c1 = src[uvs + 3];
                    int c2 = src[uvs + 4];
                    int c3 = src[uvs + 5];
                    int d1, d2, d3;

                    a1 -= a01;
                    a2 -= a02;
                    a3 -= a03;
                    b1 -= a01;
                    b2 -= a02;
                    b3 -= a03;
                    c1 -= a01 + a1 + b1;
                    c2 -= a02 + a2 + b2;
                    c3 -= a03 + a3 + b3;
                    d1 = ((a1 * iu + b1 * iv + c1 * iuv) >> S) + a01;
                    d2 = ((a2 * iu + b2 * iv + c2 * iuv) >> S) + a02;
                    d3 = ((a3 * iu + b3 * iv + c3 * iuv) >> S) + a03;
                    d1 = !(d1 & ~255) ? d1 : d1 < 0 ? 0 : 255;
                    d2 = !(d2 & ~255) ? d2 : d2 < 0 ? 0 : 255;
                    d3 = !(d3 & ~255) ? d3 : d3 < 0 ? 0 : 255;
                    d1 |= (d2 << 8) | (d3 << 16);
                    *(int *) (dst + u3) = d1;
                }
            }

            if( v == size.height - 1 )
            {
                float r2 = x2[u] + y2;
                float bx = r2 * (k1 + r2 * k2) + x3[u] + y3;
                float by = bx + r2 * y1;
                int iu, iv, ud, vd, u3 = u + u + u;

                bx += r2 * x1[u];
                iu = cvRound( bx * du[u] );
                iv = cvRound( by * dv );
                ud = iu >> S;
                vd = iv >> S;
                iu -= ud << S;
                iv -= vd << S;
                ud += u;
                vd += v;
                size.width++;

                if( ud < 0 || ud > sizex || vd < 0 || vd > sizey )
                    dst[u3] = dst[u3 + 1] = dst[u3 + 2] = 0;
                else
                {
                    int iuv = (iu >> S2) * (iv >> S2);
                    int uv = vd * srcStep + ud + ud + ud;
                    int uvs = uv + srcStep;
                    int a01 = src[uv];
                    int a02 = src[uv + 1];
                    int a03 = src[uv + 2];
                    int a1 = src[uv + 3];
                    int a2 = src[uv + 4];
                    int a3 = src[uv + 5];
                    int b1 = src[uvs];
                    int b2 = src[uvs + 1];
                    int b3 = src[uvs + 2];
                    int c1 = src[uvs + 3];
                    int c2 = src[uvs + 4];
                    int c3 = src[uvs + 5];
                    int d1, d2, d3;

                    a1 -= a01;
                    a2 -= a02;
                    a3 -= a03;
                    b1 -= a01;
                    b2 -= a02;
                    b3 -= a03;
                    c1 -= a01 + a1 + b1;
                    c2 -= a02 + a2 + b2;
                    c3 -= a03 + a3 + b3;
                    d1 = ((a1 * iu + b1 * iv + c1 * iuv) >> S) + a01;
                    d2 = ((a2 * iu + b2 * iv + c2 * iuv) >> S) + a02;
                    d3 = ((a3 * iu + b3 * iv + c3 * iuv) >> S) + a03;
                    dst[u3] = (uchar) d1;
                    dst[u3 + 1] = (uchar) d2;
                    dst[u3 + 2] = (uchar) d3;
                }
            }
        }
    }

    memcpy( (void*)src, buf, 3 );

    cvFree( (void **) &x1 );
    cvFree( (void **) &x2 );
    cvFree( (void **) &x3 );
    cvFree( (void **) &du );
    return CV_NO_ERR;
}
/*______________________________________________________________________________________*/


/*F//////////////////////////////////////////////////////////////////////////////////////
//    Name: cvUnDistortOnce
//    Purpose: The function corrects radial and tangential image distortion using known
//             matrix of the camera intrinsic parameters and distortion coefficients
//    Context:
//    Parameters:  srcImage    - source (distorted) image
//                 dstImage    - output (undistorted) image
//                 intrMatrix  - matrix of the camera intrinsic parameters
//                 distCoeffs  - vector of the distortion coefficients (k1, k2, p1 and p2)
//                 interpolate - interpolation toggle (optional parameter)
//
//    Notes:   1. If interpolate = 0, interpolation disabled;
//                else (default) bilinear interpolation is used.
//             2. If p1 = p2 = 0, function acts faster.
//F*/

typedef CvStatus (CV_STDCALL * CvUnDistortOnceFunc)( const void* src, int srcstep,
                                                     void* dst, int dststep, CvSize size,
                                                     const float* intrMatrix,
                                                     const float* distCoeffs,
                                                     int interToggle );


CV_IMPL void
cvUnDistortOnce( const void* srcImage, void* dstImage,
                 const float* intrMatrix, const float* distCoeffs,
                 int interpolate )
{
    static void* undist_tab[4];
    static int inittab = 0;

    CV_FUNCNAME( "cvUnDistortOnce" );

    __BEGIN__;

    int coi1 = 0, coi2 = 0;
    CvMat srcstub, *src = (CvMat*)srcImage;
    CvMat dststub, *dst = (CvMat*)dstImage;
    CvSize size;
    CvUnDistortOnceFunc  func = 0;

    if( !inittab )
    {
        undist_tab[0] = (void*)icvUnDistort1_8u_C1R;
        undist_tab[1] = (void*)icvUnDistortEx_8u_C1R;
        undist_tab[2] = (void*)icvUnDistort1_8u_C3R;
        undist_tab[3] = (void*)icvUnDistortEx_8u_C3R;
        inittab = 1;
    }

    CV_CALL( src = cvGetMat( src, &srcstub, &coi1 ));
    CV_CALL( dst = cvGetMat( dst, &dststub, &coi2 ));

    if( coi1 != 0 || coi2 != 0 )
        CV_ERROR( CV_BadCOI, "" );

    if( CV_MAT_TYPE(src->type) != CV_8UC1 &&
        CV_MAT_TYPE(src->type) != CV_8UC3 )
        CV_ERROR( CV_StsUnsupportedFormat, "" );

    if( !CV_ARE_TYPES_EQ( src, dst ))
        CV_ERROR( CV_StsUnmatchedFormats, "" );

    if( !CV_ARE_SIZES_EQ( src, dst ))
        CV_ERROR( CV_StsUnmatchedSizes, "" );

    if( !intrMatrix || !distCoeffs )
        CV_ERROR( CV_StsNullPtr, "" );

    size = cvGetMatSize( src );

    func = (CvUnDistortOnceFunc)
        (undist_tab[(CV_MAT_CN(src->type)-1)+(distCoeffs[2] != 0 || distCoeffs[3] != 0)]);

    if( !func )
        CV_ERROR( CV_StsUnsupportedFormat, "" );

    IPPI_CALL( func( src->data.ptr, src->step, dst->data.ptr, dst->step, size,
                     intrMatrix, distCoeffs, interpolate ));

    __END__;
}


/*F//////////////////////////////////////////////////////////////////////////////////////
//    Name: cvUnDistortInit
//    Purpose: The function calculates arrays of distorted points indices and
//             interpolation coefficients for cvUnDistort function using known
//             matrix of the camera intrinsic parameters and distortion coefficients
//    Context:
//    Parameters:  srcImage    - source (distorted) image
//                 intrMatrix  - matrix of the camera intrinsic parameters
//                 distCoeffs  - vector of the distortion coefficients (k1, k2, p1 and p2)
//                 data        - distortion data array
//                 interpolate - interpolation toggle (optional parameter)
//
//    Notes:  1. If interpolate=0, interpolation disabled;
//               else (default) bilinear interpolation is used.
//            2. Array data must be allocated before. If interpolate = 0, its length
//               must be size.width*size.height elements; else 3*size.width*size.height.
//F*/
CV_IMPL void
cvUnDistortInit( const void* srcImage, void* undistMap,
                 const float* intrMatrix, const float* distCoeffs,
                 int  interpolate )
{
    CV_FUNCNAME( "cvUnDistortInit" );

    __BEGIN__;

    int coi1 = 0, coi2 = 0;
    CvMat srcstub, *src = (CvMat*)srcImage;
    CvMat mapstub, *map = (CvMat*)undistMap;
    CvSize size;

    CV_CALL( src = cvGetMat( src, &srcstub, &coi1 ));
    CV_CALL( map = cvGetMat( map, &mapstub, &coi2 ));

    if( coi1 != 0 || coi2 != 0 )
        CV_ERROR( CV_BadCOI, "" );

    if( CV_MAT_TYPE( map->type ) != CV_32SC1 && CV_MAT_TYPE( map->type ) != CV_32SC3 )
        CV_ERROR( CV_StsUnsupportedFormat, "" );

    if( !intrMatrix || !distCoeffs )
        CV_ERROR( CV_StsNullPtr, "" );

    if( src->height > map->height ||
        interpolate && src->width*3 > map->width*CV_MAT_CN( map->type ) ||
        !interpolate && src->width > map->width )
        CV_ERROR( CV_StsUnmatchedSizes, "" );

    size = cvGetMatSize( src );

    IPPI_CALL( icvUnDistortInit( src->step, (int*)map->data.ptr, map->step,
                                 size, intrMatrix, distCoeffs,
                                 interpolate, CV_ELEM_SIZE(src->type)));

    __END__;
}


/*F//////////////////////////////////////////////////////////////////////////////////////
//    Name: cvUnDistort
//    Purpose: The function corrects radial and tangential distortion in the frame
//             using previousely calculated arrays of distorted points indices and
//             undistortion coefficients
//    Context:
//    Parameters:  srcImage    - source (distorted) image
//                 dstImage    - output (undistorted) image
//                 data        - distortion data array
//                 interpolate - interpolation toggle (optional parameter)
//
//    Notes:  1. If interpolate=0, interpolation disabled;
//               else (default) bilinear interpolation is used.
//            2. Array data must be allocated and calculated by the cvUnDistortInit
//               function before. If interpolate = 0, its length must be
//               size.width*size.height elements; else 3*size.width*size.height.
//F*/
CV_IMPL void
cvUnDistort( const void* srcImage, void* dstImage,
             const void* undistMap, int interpolate )
{
    CV_FUNCNAME( "cvUnDistort" );

    __BEGIN__;

    int coi1 = 0, coi2 = 0, coi3 = 0;
    CvMat srcstub, *src = (CvMat*)srcImage;
    CvMat mapstub, *map = (CvMat*)undistMap;
    CvMat dststub, *dst = (CvMat*)dstImage;
    CvSize size;

    CV_CALL( src = cvGetMat( src, &srcstub, &coi1 ));
    CV_CALL( map = cvGetMat( map, &mapstub, &coi2 ));
    CV_CALL( dst = cvGetMat( dst, &dststub, &coi3 ));

    if( coi1 != 0 || coi2 != 0 || coi3 != 0 )
        CV_ERROR( CV_BadCOI, "" );

    if( CV_MAT_TYPE(src->type) != CV_8UC1 && CV_MAT_TYPE(src->type) != CV_8UC3 )
        CV_ERROR( CV_StsUnsupportedFormat, "" );

    if( !CV_ARE_TYPES_EQ( src, dst ))
        CV_ERROR( CV_StsUnmatchedFormats, "" );

    if( CV_MAT_TYPE( map->type ) != CV_32SC1 && CV_MAT_TYPE( map->type ) != CV_32SC3 )
        CV_ERROR( CV_StsUnsupportedFormat, "" );

    if( src->height > map->height ||
        interpolate && src->width*3 > map->width*CV_MAT_CN( map->type ) ||
        !interpolate && src->width > map->width )
        CV_ERROR( CV_StsUnmatchedSizes, "" );

    if( !CV_ARE_SIZES_EQ( src, dst ))
        CV_ERROR( CV_StsUnmatchedSizes, "" );

    size = cvGetMatSize( src );

    if( CV_MAT_TYPE( src->type ) == CV_8UC1 )
    {
        IPPI_CALL( icvUnDistort_8u_C1R( src->data.ptr, src->step,
                                        (int*)map->data.ptr, map->step,
                                        dst->data.ptr, dst->step,
                                        size, interpolate ));
    }
    else if( CV_MAT_TYPE( src->type ) == CV_8UC3 )
    {
        IPPI_CALL( icvUnDistort_8u_C3R( src->data.ptr, src->step,
                                        (int*)map->data.ptr, map->step,
                                        dst->data.ptr, dst->step,
                                        size, interpolate ));
    }
    else
    {
        CV_ERROR( CV_StsUnsupportedFormat, "Undistorted image must have 8uC1 or 8uC3 format" );
    }

    __END__;
}


/*F//////////////////////////////////////////////////////////////////////////////////////
//    Name: cvConvertMap
//    Purpose: The function converts floating-point pixel coordinate map to
//             faster fixed-point map, used within cvUnDistort (cvRemap)
//    Context:
//    Parameters:  flUndistMap - source map: (width x height x 32fC2) or
//                                           (width*2 x height x 32fC1).
//                               each pair are pixel coordinates (x,y) in a source image.
//                 undistMap - resultant map: (width x height x 32sC3) or
//                                 (width*3 x height x 32sC1) if interpolation is enabled;
//                                 (width x height x 32sC1) if interpolation is disabled;
//                 pixelSize - bytes per pixel in images that will be transformed
//                             using the map. For byte-depth images it is just
//                             a number of channels.
//                 interpolate - interpolation flag (turned on by default)
//F*/
CV_IMPL void
cvConvertMap( const CvArr* srcImage, const CvArr* flUndistMap,
              CvArr* undistMap, int interpolate )
{
    CV_FUNCNAME( "cvConvertMap" );

    __BEGIN__;

    int coi1 = 0, coi2 = 0, coi3 = 0;
    int i, j;
    CvMat srcstub, *src = (CvMat*)srcImage;
    CvMat mapstub, *map = (CvMat*)undistMap;
    CvMat flmapstub, *flmap = (CvMat*)flUndistMap;
    CvSize size;
    CvPoint2D32f* flmapdata = 0;
    int srcStep, pixSize;

    CV_CALL( src = cvGetMat( src, &srcstub, &coi1 ));
    CV_CALL( map = cvGetMat( map, &mapstub, &coi2 ));
    CV_CALL( flmap = cvGetMat( flmap, &flmapstub, &coi3 ));

    if( coi1 != 0 || coi2 != 0 || coi3 != 0 )
        CV_ERROR( CV_BadCOI, "" );

    size = cvGetSize( src );

    if( CV_MAT_DEPTH( flmap->type ) != CV_32F )
        CV_ERROR( CV_StsUnsupportedFormat, "Source map should have 32f depth" );

    if( CV_MAT_CN( flmap->type ) > 2 ||
        size.width*2 != flmap->width * CV_MAT_CN( flmap->type ) ||
        size.height != flmap->height )
        CV_ERROR( CV_StsUnmatchedSizes, "Source map and source image have unmatched sizes");

    if( CV_MAT_TYPE( map->type ) != CV_32SC1 &&
        (CV_MAT_TYPE( map->type ) != CV_32SC3 || !interpolate))
        CV_ERROR( CV_StsUnsupportedFormat, "" );

    if( map->height != size.height ||
        interpolate && size.width*3 != map->width*CV_MAT_CN( map->type ) ||
        !interpolate && size.width != map->width )
        CV_ERROR( CV_StsUnmatchedSizes, "" );

    flmapdata = (CvPoint2D32f*)flmap->data.ptr;
    pixSize = CV_ELEM_SIZE(src->type);
    srcStep = src->step;

    if( !interpolate )
    {
        int* mapdata = (int*)map->data.ptr;

        for( i = 0; i < size.height; i++, (char*&)flmapdata += flmap->step,
                                          (char*&)mapdata += map->step )
        {
            for( j = 0; j < size.width; j++ )
            {
                int ix = cvRound(flmapdata[j].x);
                int iy = cvRound(flmapdata[j].y);

                if( (unsigned)ix < (unsigned)size.width &&
                    (unsigned)iy < (unsigned)size.height )
                    mapdata[j] = iy*srcStep + ix*pixSize;
                else
                    mapdata[j] = 0;
            }
        }
    }
    else
    {
        CvUnDistortData* mapdata = (CvUnDistortData*)map->data.ptr;

        for( i = 0; i < size.height; i++, (char*&)flmapdata += flmap->step,
                                          (char*&)mapdata += map->step )
        {
            for( j = 0; j < size.width; j++ )
            {
                float x = flmapdata[j].x;
                float y = flmapdata[j].y;
                int ix = cvFloor(x);
                int iy = cvFloor(y);

                x -= ix;
                y -= iy;

                if( (unsigned)ix < (unsigned)(size.width-1) &&
                    (unsigned)iy < (unsigned)(size.height-1) )
                {
                    mapdata[j].ind = iy*srcStep + ix*pixSize;
                    mapdata[j].a0 = (ushort)cvRound((1.f - x)*(1.f - y)*(1 << 15));
                    mapdata[j].a1 = (ushort)cvRound(x*(1.f - y)*(1 << 15));
                    mapdata[j].a2 = (ushort)cvRound((1.f - x)*y*(1 << 15));
                    mapdata[j].a3 = (ushort)cvRound(x*y*(1 << 15));
                }
                else
                {
                    mapdata[j].ind = 0;
                    mapdata[j].a0 = 0;
                    mapdata[j].a1 = 0;
                    mapdata[j].a2 = 0;
                    mapdata[j].a3 = 0;
                }
            }
        }
    }

    __END__;
}

/*  End of file  */
