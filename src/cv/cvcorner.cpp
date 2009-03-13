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
#include <stdio.h>


namespace cv
{

static void
calcMinEigenVal( const Mat& _cov, Mat& _dst )
{
    int i, j;
    Size size = _cov.size();
    if( _cov.isContinuous() && _dst.isContinuous() )
    {
        size.width *= size.height;
        size.height = 1;
    }

    for( i = 0; i < size.height; i++ )
    {
        const float* cov = (const float*)(_cov.data + _cov.step*i);
        float* dst = (float*)(_dst.data + _dst.step*i);
        
        for( j = 0; j < size.width; j++ )
        {
            double a = cov[j*3]*0.5;
            double b = cov[j*3+1];
            double c = cov[j*3+2]*0.5;
            dst[j] = (float)((a + c) - std::sqrt((a - c)*(a - c) + b*b));
        }
    }
}


static void
calcHarris( const Mat& _cov, Mat& _dst, double k )
{
    int i, j;
    Size size = _cov.size();
    if( _cov.isContinuous() && _dst.isContinuous() )
    {
        size.width *= size.height;
        size.height = 1;
    }

    for( i = 0; i < size.height; i++ )
    {
        const float* cov = (const float*)(_cov.data + _cov.step*i);
        float* dst = (float*)(_dst.data + _dst.step*i);

        for( j = 0; j < size.width; j++ )
        {
            double a = cov[j*3];
            double b = cov[j*3+1];
            double c = cov[j*3+2];
            dst[j] = (float)(a*c - b*b - k*(a + c)*(a + c));
        }
    }
}


static void
calcEigenValsVecs( const Mat& _cov, Mat& _dst )
{
    int i, j;
    Size size = _cov.size();
    if( _cov.isContinuous() && _dst.isContinuous() )
    {
        size.width *= size.height;
        size.height = 1;
    }

    for( i = 0; i < size.height; i++ )
    {
        const float* cov = (const float*)(_cov.data + _cov.step*i);
        float* dst = (float*)(_dst.data + _dst.step*i);

        for( j = 0; j < size.width; j++ )
        {
            double a = cov[j*3];
            double b = cov[j*3+1];
            double c = cov[j*3+2];

            double u = (a + c)*0.5;
            double v = std::sqrt((a - c)*(a - c)*0.25 + b*b);
            double l1 = u + v;
            double l2 = u - v;

            double x = b;
            double y = l1 - a;
            double e = fabs(x);

            if( e + fabs(y) < 1e-4 )
            {
                y = b;
                x = l1 - c;
                e = fabs(x);
                if( e + fabs(y) < 1e-4 )
                {
                    e = 1./(e + fabs(y) + FLT_EPSILON);
                    x *= e, y *= e;
                }
            }

            double d = 1./std::sqrt(x*x + y*y + DBL_EPSILON);
            dst[6*j] = (float)l1;
            dst[6*j + 2] = (float)(x*d);
            dst[6*j + 3] = (float)(y*d);

            x = b;
            y = l2 - a;
            e = fabs(x);

            if( e + fabs(y) < 1e-4 )
            {
                y = b;
                x = l2 - c;
                e = fabs(x);
                if( e + fabs(y) < 1e-4 )
                {
                    e = 1./(e + fabs(y) + FLT_EPSILON);
                    x *= e, y *= e;
                }
            }

            d = 1./std::sqrt(x*x + y*y + DBL_EPSILON);
            dst[6*j + 1] = (float)l2;
            dst[6*j + 4] = (float)(x*d);
            dst[6*j + 5] = (float)(y*d);
        }
    }
}


enum { MINEIGENVAL=0, HARRIS=1, EIGENVALSVECS=2 };


static void
cornerEigenValsVecs( const Mat& src, Mat& eigenv, int block_size,
                     int aperture_size, int op_type, double k=0.,
                     int borderType=BORDER_DEFAULT )
{
    int depth = src.depth();
    double scale = (double)(1 << ((aperture_size > 0 ? aperture_size : 3) - 1)) * block_size;
    if( aperture_size < 0 )
        scale *= 2.;
    if( depth == CV_8U )
        scale *= 255.;
    scale = 1./scale;

    CV_Assert( src.type() == CV_8UC1 || src.type() == CV_32FC1 );

    Mat Dx, Dy;
    if( aperture_size > 0 )
    {
        Sobel( src, Dx, CV_32F, 1, 0, aperture_size, scale, 0, borderType );
        Sobel( src, Dy, CV_32F, 0, 1, aperture_size, scale, 0, borderType );
    }
    else
    {
        Scharr( src, Dx, CV_32F, 1, 0, scale, 0, borderType );
        Scharr( src, Dy, CV_32F, 0, 1, scale, 0, borderType );
    }

    Size size = src.size();
    Mat cov( size, CV_32FC3 );
    int i, j;

    for( i = 0; i < size.height; i++ )
    {
        float* cov_data = (float*)(cov.data + i*cov.step);
        const float* dxdata = (const float*)(Dx.data + i*Dx.step);
        const float* dydata = (const float*)(Dy.data + i*Dy.step);

        for( j = 0; j < size.width; j++ )
        {
            float dx = dxdata[j];
            float dy = dydata[j];

            cov_data[j*3] = dx*dx;
            cov_data[j*3+1] = dx*dy;
            cov_data[j*3+2] = dy*dy;
        }
    }

    boxFilter(cov, cov, cov.depth(), Size(block_size, block_size),
        Point(-1,-1), false, borderType );

    if( op_type == MINEIGENVAL )
        calcMinEigenVal( cov, eigenv );
    else if( op_type == HARRIS )
        calcHarris( cov, eigenv, k );
    else if( op_type == EIGENVALSVECS )
        calcEigenValsVecs( cov, eigenv );
}


void cornerMinEigenVal( const Mat& src, Mat& dst, int blockSize, int ksize, int borderType )
{
    dst.create( src.size(), CV_32F );
    cornerEigenValsVecs( src, dst, blockSize, ksize, MINEIGENVAL, 0, borderType );
}


void cornerHarris( const Mat& src, Mat& dst, int blockSize, int ksize, double k, int borderType )
{
    dst.create( src.size(), CV_32F );
    cornerEigenValsVecs( src, dst, blockSize, ksize, HARRIS, k, borderType );
}


void cornerEigenValsAndVecs( const Mat& src, Mat& dst, int blockSize, int ksize, int borderType )
{
    if( dst.rows != src.rows || dst.cols*dst.channels() != src.cols*6 || dst.depth() != CV_32F )
        dst.create( src.size(), CV_32FC(6) );
    cornerEigenValsVecs( src, dst, blockSize, ksize, EIGENVALSVECS, 0, borderType );
}


void preCornerDetect( const Mat& src, Mat& dst, int ksize, int borderType )
{
    Mat Dx, Dy, D2x, D2y, Dxy;

    CV_Assert( src.type() == CV_8UC1 || src.type() == CV_32FC1 );
    dst.create( src.size(), CV_32F );

    Sobel( src, Dx, CV_32F, 1, 0, ksize, 1, 0, borderType );
    Sobel( src, Dy, CV_32F, 0, 1, ksize, 1, 0, borderType );
    Sobel( src, D2x, CV_32F, 2, 0, ksize, 1, 0, borderType );
    Sobel( src, D2y, CV_32F, 0, 2, ksize, 1, 0, borderType );
    Sobel( src, Dxy, CV_32F, 1, 1, ksize, 1, 0, borderType );

    double factor = 1 << (ksize - 1);
    if( src.depth() == CV_8U )
        factor *= 255;
    factor = 1./(factor * factor * factor);

    Size size = src.size();
    int i, j;
    for( i = 0; i < size.height; i++ )
    {
        float* dstdata = (float*)(dst.data + i*dst.step);
        const float* dxdata = (const float*)(Dx.data + i*Dx.step);
        const float* dydata = (const float*)(Dy.data + i*Dy.step);
        const float* d2xdata = (const float*)(D2x.data + i*D2x.step);
        const float* d2ydata = (const float*)(D2y.data + i*D2y.step);
        const float* dxydata = (const float*)(Dxy.data + i*Dxy.step);
        
        for( j = 0; j < size.width; j++ )
        {
            double dx = dxdata[j];
            double dy = dydata[j];
            dstdata[j] = (float)(factor*(dx*dx*d2ydata[j] + dy*dy*d2xdata[j] - 2*dx*dy*dxydata[j]));
        }
    }
}


}

CV_IMPL void
cvCornerMinEigenVal( const CvArr* srcarr, CvArr* dstarr,
                     int block_size, int aperture_size )
{
    cv::Mat src = cv::cvarrToMat(srcarr), dst = cv::cvarrToMat(dstarr);

    CV_Assert( src.size() == dst.size() && dst.type() == CV_32FC1 );
    cv::cornerMinEigenVal( src, dst, block_size, aperture_size, cv::BORDER_REPLICATE );
}

CV_IMPL void
cvCornerHarris( const CvArr* srcarr, CvArr* dstarr,
                int block_size, int aperture_size, double k )
{
    cv::Mat src = cv::cvarrToMat(srcarr), dst = cv::cvarrToMat(dstarr);

    CV_Assert( src.size() == dst.size() && dst.type() == CV_32FC1 );
    cv::cornerHarris( src, dst, block_size, aperture_size, k, cv::BORDER_REPLICATE );
}


CV_IMPL void
cvCornerEigenValsAndVecs( const void* srcarr, void* dstarr,
                          int block_size, int aperture_size )
{
    cv::Mat src = cv::cvarrToMat(srcarr), dst = cv::cvarrToMat(dstarr);

    CV_Assert( src.rows == dst.rows && src.cols*6 == dst.cols*dst.channels() && dst.depth() == CV_32F );
    cv::cornerEigenValsAndVecs( src, dst, block_size, aperture_size, cv::BORDER_REPLICATE );
}


CV_IMPL void
cvPreCornerDetect( const void* srcarr, void* dstarr, int aperture_size )
{
    cv::Mat src = cv::cvarrToMat(srcarr), dst = cv::cvarrToMat(dstarr);

    CV_Assert( src.size() == dst.size() && dst.type() == CV_32FC1 );
    cv::preCornerDetect( src, dst, aperture_size, cv::BORDER_REPLICATE );
}

/* End of file */
