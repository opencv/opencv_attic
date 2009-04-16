/*M///////////////////////////////////////////////////////////////////////////////////////
//
//  IMPORTANT: READ BEFORE DOWNLOADING, COPYING, INSTALLING OR USING.
//
//  By downloading, copying, installing or using the software you agree to this license.
//  If you do not agree to this license, do not download, install,
//  copy or use the software.
//
//
//                           License Agreement
//                For Open Source Computer Vision Library
//
// Copyright (C) 2000-2008, Intel Corporation, all rights reserved.
// Copyright (C) 2009, Willow Garage Inc., all rights reserved.
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
//   * The name of the copyright holders may not be used to endorse or promote products
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

#include "_cxcore.h"
#include "clapack.h"
#undef max
#undef min

namespace cv
{

/****************************************************************************************\
*                                 Determinant of the matrix                              *
\****************************************************************************************/

#define det2(m)   (m(0,0)*m(1,1) - m(0,1)*m(1,0))
#define det3(m)   (m(0,0)*(m(1,1)*m(2,2) - m(1,2)*m(2,1)) -  \
                   m(0,1)*(m(1,0)*m(2,2) - m(1,2)*m(2,0)) +  \
                   m(0,2)*(m(1,0)*m(2,1) - m(1,1)*m(2,0)))

double determinant( const Mat& mat )
{
    double result = 0;
    int type = mat.type(), rows = mat.rows;
    int step = mat.step;
    const uchar* m = mat.data;

    CV_Assert( mat.rows == mat.cols );

    #define Mf(y, x) ((float*)(m + y*step))[x]
    #define Md(y, x) ((double*)(m + y*step))[x]

    if( type == CV_32F )
    {
        if( rows == 2 )
            result = det2(Mf);
        else if( rows == 3 )
            result = det3(Mf);
        else if( rows == 1 )
            result = Mf(0,0);
        else
        {
            integer i, n = rows, *ipiv, info=0;
            int bufSize = n*n*sizeof(float) + (n+1)*sizeof(ipiv[0]), sign=0;
            AutoBuffer<uchar> buffer(bufSize);

            Mat a(n, n, CV_32F, (uchar*)buffer);
            mat.copyTo(a);

            ipiv = (integer*)cvAlignPtr(a.data + a.step*a.rows, sizeof(integer));
            sgetrf_(&n, &n, (float*)a.data, &n, ipiv, &info);
            assert(info >= 0);

            if( info == 0 )
            {
                result = 1;
                for( i = 0; i < n; i++ )
                {
                    result *= ((float*)a.data)[i*(n+1)];
                    sign ^= ipiv[i] != i+1;
                }
                result *= sign ? -1 : 1;
            }
        }
    }
    else if( type == CV_64F )
    {
        if( rows == 2 )
            result = det2(Md);
        else if( rows == 3 )
            result = det3(Md);
        else if( rows == 1 )
            result = Md(0,0);
        else
        {
            integer i, n = rows, *ipiv, info=0;
            int bufSize = n*n*sizeof(double) + (n+1)*sizeof(ipiv[0]), sign=0;
            AutoBuffer<uchar> buffer(bufSize);

            Mat a(n, n, CV_64F, (uchar*)buffer);
            mat.copyTo(a);
            ipiv = (integer*)cvAlignPtr(a.data + a.step*a.rows, sizeof(integer));

            dgetrf_(&n, &n, (double*)a.data, &n, ipiv, &info);
            assert(info >= 0);

            if( info == 0 )
            {
                result = 1;
                for( i = 0; i < n; i++ )
                {
                    result *= ((double*)a.data)[i*(n+1)];
                    sign ^= ipiv[i] != i+1;
                }
                result *= sign ? -1 : 1;
            }
        }
    }
    else
        CV_Error( CV_StsUnsupportedFormat, "" );

    #undef Mf
    #undef Md

    return result;
}

/****************************************************************************************\
*                          Inverse (or pseudo-inverse) of a matrix                       *
\****************************************************************************************/

#define Sf( y, x ) ((float*)(srcdata + y*srcstep))[x]
#define Sd( y, x ) ((double*)(srcdata + y*srcstep))[x]
#define Df( y, x ) ((float*)(dstdata + y*dststep))[x]
#define Dd( y, x ) ((double*)(dstdata + y*dststep))[x]

double invert( const Mat& src, Mat& dst, int method )
{
    double result = 0;
    int type = src.type();

    CV_Assert( method == DECOMP_LU || method == DECOMP_CHOLESKY || method == DECOMP_SVD );

    if( method == DECOMP_SVD )
    {
        int n = std::min(src.rows, src.cols);
        SVD svd(src);
        svd.backSubst(Mat(), dst);

        return type == CV_32F ?
            (((float*)svd.w.data)[0] >= FLT_EPSILON ?
            ((float*)svd.w.data)[n-1]/((float*)svd.w.data)[0] : 0) :
            (((double*)svd.w.data)[0] >= DBL_EPSILON ?
            ((double*)svd.w.data)[n-1]/((double*)svd.w.data)[0] : 0);
    }

    CV_Assert( src.rows == src.cols && (type == CV_32F || type == CV_64F));
    dst.create( src.rows, src.cols, type );

    if( method == DECOMP_LU || method == DECOMP_CHOLESKY )
    {
        if( src.rows <= 3 )
        {
            uchar* srcdata = src.data;
            uchar* dstdata = dst.data;
            int srcstep = src.step;
            int dststep = dst.step;

            if( src.rows == 2 )
            {
                if( type == CV_32FC1 )
                {
                    double d = det2(Sf);
                    if( d != 0. )
                    {
                        double t0, t1;
                        result = d;
                        d = 1./d;
                        t0 = Sf(0,0)*d;
                        t1 = Sf(1,1)*d;
                        Df(1,1) = (float)t0;
                        Df(0,0) = (float)t1;
                        t0 = -Sf(0,1)*d;
                        t1 = -Sf(1,0)*d;
                        Df(0,1) = (float)t0;
                        Df(1,0) = (float)t1;
                    }
                }
                else
                {
                    double d = det2(Sd);
                    if( d != 0. )
                    {
                        double t0, t1;
                        result = d;
                        d = 1./d;
                        t0 = Sd(0,0)*d;
                        t1 = Sd(1,1)*d;
                        Dd(1,1) = t0;
                        Dd(0,0) = t1;
                        t0 = -Sd(0,1)*d;
                        t1 = -Sd(1,0)*d;
                        Dd(0,1) = t0;
                        Dd(1,0) = t1;
                    }
                }
            }
            else if( src.rows == 3 )
            {
                if( type == CV_32FC1 )
                {
                    double d = det3(Sf);
                    if( d != 0. )
                    {
                        float t[9];
                        result = d;
                        d = 1./d;

                        t[0] = (float)((Sf(1,1) * Sf(2,2) - Sf(1,2) * Sf(2,1)) * d);
                        t[1] = (float)((Sf(0,2) * Sf(2,1) - Sf(0,1) * Sf(2,2)) * d);
                        t[2] = (float)((Sf(0,1) * Sf(1,2) - Sf(0,2) * Sf(1,1)) * d);
                                      
                        t[3] = (float)((Sf(1,2) * Sf(2,0) - Sf(1,0) * Sf(2,2)) * d);
                        t[4] = (float)((Sf(0,0) * Sf(2,2) - Sf(0,2) * Sf(2,0)) * d);
                        t[5] = (float)((Sf(0,2) * Sf(1,0) - Sf(0,0) * Sf(1,2)) * d);
                                      
                        t[6] = (float)((Sf(1,0) * Sf(2,1) - Sf(1,1) * Sf(2,0)) * d);
                        t[7] = (float)((Sf(0,1) * Sf(2,0) - Sf(0,0) * Sf(2,1)) * d);
                        t[8] = (float)((Sf(0,0) * Sf(1,1) - Sf(0,1) * Sf(1,0)) * d);

                        Df(0,0) = t[0]; Df(0,1) = t[1]; Df(0,2) = t[2];
                        Df(1,0) = t[3]; Df(1,1) = t[4]; Df(1,2) = t[5];
                        Df(2,0) = t[6]; Df(2,1) = t[7]; Df(2,2) = t[8];
                    }
                }
                else
                {
                    double d = det3(Sd);
                    if( d != 0. )
                    {
                        double t[9];
                        result = d;
                        d = 1./d;

                        t[0] = (Sd(1,1) * Sd(2,2) - Sd(1,2) * Sd(2,1)) * d;
                        t[1] = (Sd(0,2) * Sd(2,1) - Sd(0,1) * Sd(2,2)) * d;
                        t[2] = (Sd(0,1) * Sd(1,2) - Sd(0,2) * Sd(1,1)) * d;
                               
                        t[3] = (Sd(1,2) * Sd(2,0) - Sd(1,0) * Sd(2,2)) * d;
                        t[4] = (Sd(0,0) * Sd(2,2) - Sd(0,2) * Sd(2,0)) * d;
                        t[5] = (Sd(0,2) * Sd(1,0) - Sd(0,0) * Sd(1,2)) * d;
                               
                        t[6] = (Sd(1,0) * Sd(2,1) - Sd(1,1) * Sd(2,0)) * d;
                        t[7] = (Sd(0,1) * Sd(2,0) - Sd(0,0) * Sd(2,1)) * d;
                        t[8] = (Sd(0,0) * Sd(1,1) - Sd(0,1) * Sd(1,0)) * d;

                        Dd(0,0) = t[0]; Dd(0,1) = t[1]; Dd(0,2) = t[2];
                        Dd(1,0) = t[3]; Dd(1,1) = t[4]; Dd(1,2) = t[5];
                        Dd(2,0) = t[6]; Dd(2,1) = t[7]; Dd(2,2) = t[8];
                    }
                }
            }
            else
            {
                assert( src.rows == 1 );

                if( type == CV_32FC1 )
                {
                    double d = Sf(0,0);
                    if( d != 0. )
                    {
                        result = d;
                        Df(0,0) = (float)(1./d);
                    }
                }
                else
                {
                    double d = Sd(0,0);
                    if( d != 0. )
                    {
                        result = d;
                        Dd(0,0) = 1./d;
                    }
                }
            }
            return result;
        }

        {
        integer n = dst.cols, lwork=-1, elem_size = CV_ELEM_SIZE(type),
            lda = dst.step/elem_size, piv1=0, info=0;

        if( dst.data == src.data )
        {
            dst.release();
            dst.create( src.rows, src.cols, type );
        }
        src.copyTo(dst);
        if( method == DECOMP_LU )
        {
            int buf_size = (int)(n*sizeof(integer));
            AutoBuffer<uchar> buf;
            uchar* buffer;

            if( type == CV_32F )
            {
                real work1 = 0;
                sgetri_(&n, (float*)dst.data, &lda, &piv1, &work1, &lwork, &info);
                lwork = cvRound(work1);
            }
            else
            {
                double work1 = 0;
                dgetri_(&n, (double*)dst.data, &lda, &piv1, &work1, &lwork, &info);
                lwork = cvRound(work1);
            }

            buf_size += (int)((lwork + 1)*elem_size);
            buf.allocate(buf_size);
            buffer = (uchar*)buf;

            if( type == CV_32F )
            {
                sgetrf_(&n, &n, (float*)dst.data, &lda, (integer*)buffer, &info);
                sgetri_(&n, (float*)dst.data, &lda, (integer*)buffer,
                    (float*)(buffer + n*sizeof(integer)), &lwork, &info);
            }
            else
            {
                dgetrf_(&n, &n, (double*)dst.data, &lda, (integer*)buffer, &info);
                dgetri_(&n, (double*)dst.data, &lda, (integer*)buffer,
                    (double*)cvAlignPtr(buffer + n*sizeof(integer), elem_size), &lwork, &info);
            }
        }
        else if( method == CV_CHOLESKY )
        {
            char L[] = {'L', '\0'};
            if( type == CV_32F )
            {
                spotrf_(L, &n, (float*)dst.data, &lda, &info);
                spotri_(L, &n, (float*)dst.data, &lda, &info);
            }
            else
            {
                dpotrf_(L, &n, (double*)dst.data, &lda, &info);
                dpotri_(L, &n, (double*)dst.data, &lda, &info);
            }
            completeSymm(dst);
        }
        result = info == 0;
        }
    }

    if( !result )
        dst = Scalar(0);

    return result;
}

/****************************************************************************************\
*                              Solving a linear system                                   *
\****************************************************************************************/

bool solve( const Mat& src, const Mat& src2, Mat& dst, int method )
{
    bool result = true;
    int type = src.type();
    bool is_normal = (method & DECOMP_NORMAL) != 0;

    CV_Assert( type == src2.type() && (type == CV_32F || type == CV_64F) );

    method &= ~DECOMP_NORMAL;
    CV_Assert( (method != DECOMP_LU && method != DECOMP_CHOLESKY) ||
        is_normal || src.rows == src.cols );

    dst.create( src.cols, src2.cols, src.type() );

    // check case of a single equation and small matrix
    if( (method == DECOMP_LU || method == DECOMP_CHOLESKY) &&
        src.rows <= 3 && src.rows == src.cols && src2.cols == 1 )
    {
        #define bf(y) ((float*)(bdata + y*src2step))[0]
        #define bd(y) ((double*)(bdata + y*src2step))[0]

        uchar* srcdata = src.data;
        uchar* bdata = src2.data;
        uchar* dstdata = dst.data;
        int srcstep = src.step;
        int src2step = src2.step;
        int dststep = dst.step;

        if( src.rows == 2 )
        {
            if( type == CV_32FC1 )
            {
                double d = det2(Sf);
                if( d != 0. )
                {
                    float t;
                    d = 1./d;
                    t = (float)((bf(0)*Sf(1,1) - bf(1)*Sf(0,1))*d);
                    Df(1,0) = (float)((bf(1)*Sf(0,0) - bf(0)*Sf(1,0))*d);
                    Df(0,0) = t;
                }
                else
                    result = false;
            }
            else
            {
                double d = det2(Sd);
                if( d != 0. )
                {
                    double t;
                    d = 1./d;
                    t = (bd(0)*Sd(1,1) - bd(1)*Sd(0,1))*d;
                    Dd(1,0) = (bd(1)*Sd(0,0) - bd(0)*Sd(1,0))*d;
                    Dd(0,0) = t;
                }
                else
                    result = false;
            }
        }
        else if( src.rows == 3 )
        {
            if( type == CV_32FC1 )
            {
                double d = det3(Sf);
                if( d != 0. )
                {
                    float t[3];
                    d = 1./d;

                    t[0] = (float)(d*
                           (bf(0)*(Sf(1,1)*Sf(2,2) - Sf(1,2)*Sf(2,1)) -
                            Sf(0,1)*(bf(1)*Sf(2,2) - Sf(1,2)*bf(2)) +
                            Sf(0,2)*(bf(1)*Sf(2,1) - Sf(1,1)*bf(2))));

                    t[1] = (float)(d*
                           (Sf(0,0)*(bf(1)*Sf(2,2) - Sf(1,2)*bf(2)) -
                            bf(0)*(Sf(1,0)*Sf(2,2) - Sf(1,2)*Sf(2,0)) +
                            Sf(0,2)*(Sf(1,0)*bf(2) - bf(1)*Sf(2,0))));

                    t[2] = (float)(d*
                           (Sf(0,0)*(Sf(1,1)*bf(2) - bf(1)*Sf(2,1)) -
                            Sf(0,1)*(Sf(1,0)*bf(2) - bf(1)*Sf(2,0)) +
                            bf(0)*(Sf(1,0)*Sf(2,1) - Sf(1,1)*Sf(2,0))));

                    Df(0,0) = t[0];
                    Df(1,0) = t[1];
                    Df(2,0) = t[2];
                }
                else
                    result = false;
            }
            else
            {
                double d = det3(Sd);
                if( d != 0. )
                {
                    double t[9];

                    d = 1./d;
                    
                    t[0] = ((Sd(1,1) * Sd(2,2) - Sd(1,2) * Sd(2,1))*bd(0) +
                            (Sd(0,2) * Sd(2,1) - Sd(0,1) * Sd(2,2))*bd(1) +
                            (Sd(0,1) * Sd(1,2) - Sd(0,2) * Sd(1,1))*bd(2))*d;

                    t[1] = ((Sd(1,2) * Sd(2,0) - Sd(1,0) * Sd(2,2))*bd(0) +
                            (Sd(0,0) * Sd(2,2) - Sd(0,2) * Sd(2,0))*bd(1) +
                            (Sd(0,2) * Sd(1,0) - Sd(0,0) * Sd(1,2))*bd(2))*d;

                    t[2] = ((Sd(1,0) * Sd(2,1) - Sd(1,1) * Sd(2,0))*bd(0) +
                            (Sd(0,1) * Sd(2,0) - Sd(0,0) * Sd(2,1))*bd(1) +
                            (Sd(0,0) * Sd(1,1) - Sd(0,1) * Sd(1,0))*bd(2))*d;

                    Dd(0,0) = t[0];
                    Dd(1,0) = t[1];
                    Dd(2,0) = t[2];
                }
                else
                    result = false;
            }
        }
        else
        {
            assert( src.rows == 1 );

            if( type == CV_32FC1 )
            {
                double d = Sf(0,0);
                if( d != 0. )
                    Df(0,0) = (float)(bf(0)/d);
                else
                    result = false;
            }
            else
            {
                double d = Sd(0,0);
                if( d != 0. )
                    Dd(0,0) = (bd(0)/d);
                else
                    result = false;
            }
        }
    }

    {
    double rcond=-1, s1=0, work1=0, *work=0, *s=0;
    float frcond=-1, fs1=0, fwork1=0, *fwork=0, *fs=0;
    integer m = src.rows, m_ = m, n = src.cols, mn = std::max(m,n),
        nm = std::min(m, n), nb = src2.cols, lwork=-1, liwork=0, iwork1=0,
        lda = m, ldx = mn, info=0, rank=0, *iwork=0;
    int elem_size = CV_ELEM_SIZE(type);
    bool copy_rhs=false;
    int buf_size=0;
    AutoBuffer<uchar> buffer;
    uchar* ptr;
    char N[] = {'N', '\0'}, L[] = {'L', '\0'};

    if( m <= n )
        is_normal = false;
    else if( is_normal )
        m_ = n;

    buf_size += (is_normal ? n*n : m*n)*elem_size;

    if( m_ != n || nb > 1 || !dst.isContinuous() )
    {
        copy_rhs = true;
        if( is_normal )
            buf_size += n*nb*elem_size;
        else
            buf_size += mn*nb*elem_size;
    }

    if( method == DECOMP_SVD || method == DECOMP_EIG )
    {
        integer nlvl = cvRound(std::log(std::max(std::min(m_,n)/25., 1.))/CV_LOG2) + 1;
        liwork = std::min(m_,n)*(3*std::max(nlvl,(integer)0) + 11);

        if( type == CV_32F )
            sgelsd_(&m_, &n, &nb, (float*)src.data, &lda, (float*)dst.data, &ldx,
                &fs1, &frcond, &rank, &fwork1, &lwork, &iwork1, &info);
        else
            dgelsd_(&m_, &n, &nb, (double*)src.data, &lda, (double*)dst.data, &ldx,
                &s1, &rcond, &rank, &work1, &lwork, &iwork1, &info );
        buf_size += nm*elem_size + (liwork + 1)*sizeof(integer);
    }
    else if( method == DECOMP_QR )
    {
        if( type == CV_32F )
            sgels_(N, &m_, &n, &nb, (float*)src.data, &lda,
                (float*)dst.data, &ldx, &fwork1, &lwork, &info );
        else
            dgels_(N, &m_, &n, &nb, (double*)src.data, &lda,
                (double*)dst.data, &ldx, &work1, &lwork, &info );
    }
    else if( method == DECOMP_LU )
    {
        buf_size += (n+1)*sizeof(integer);
    }
    else if( method == DECOMP_CHOLESKY )
        ;
    else
        CV_Error( CV_StsBadArg, "Unknown method" );
    assert(info == 0);

    lwork = cvRound(type == CV_32F ? (double)fwork1 : work1);
    buf_size += lwork*elem_size;
    buffer.allocate(buf_size);
    ptr = (uchar*)buffer;

    Mat at(n, m_, type, ptr);
    ptr += n*m_*elem_size;

    if( method == DECOMP_CHOLESKY || method == DECOMP_EIG )
        src.copyTo(at);
    else if( !is_normal )
        transpose(src, at);
    else
        mulTransposed(src, at, true);

    Mat xt;
    if( !is_normal )
    {
        if( copy_rhs )
        {
            Mat temp(nb, mn, type, ptr);
            ptr += nb*mn*elem_size;
            Mat bt = temp.colRange(0, m);
            xt = temp.colRange(0, n);
            transpose(src2, bt);
        }
        else
        {
            src2.copyTo(dst);
            xt = Mat(1, n, type, dst.data);        
        }
    }
    else
    {
        if( copy_rhs )
        {
            xt = Mat(nb, n, type, ptr);
            ptr += nb*n*elem_size;
        }
        else
            xt = Mat(1, n, type, dst.data);
        // (a'*b)' = b'*a
        gemm( src2, src, 1, Mat(), 0, xt, GEMM_1_T );
    }
    
    lda = at.step ? at.step/elem_size : at.cols;
    ldx = xt.step ? xt.step/elem_size : (!is_normal && copy_rhs ? mn : n);

    if( method == DECOMP_SVD || method == DECOMP_EIG )
    {
        if( type == CV_32F )
        {
            fs = (float*)ptr;
            ptr += nm*elem_size;
            fwork = (float*)ptr;
            ptr += lwork*elem_size;
            iwork = (integer*)cvAlignPtr(ptr, sizeof(integer));

            sgelsd_(&m_, &n, &nb, (float*)at.data, &lda, (float*)xt.data, &ldx,
                fs, &frcond, &rank, fwork, &lwork, iwork, &info);
        }
        else
        {
            s = (double*)ptr;
            ptr += nm*elem_size;
            work = (double*)ptr;
            ptr += lwork*elem_size;
            iwork = (integer*)cvAlignPtr(ptr, sizeof(integer));

            dgelsd_(&m_, &n, &nb, (double*)at.data, &lda, (double*)xt.data, &ldx,
                s, &rcond, &rank, work, &lwork, iwork, &info);
        }
    }
    else if( method == CV_QR )
    {
        if( type == CV_32F )
        {
            fwork = (float*)ptr;
            sgels_(N, &m_, &n, &nb, (float*)at.data, &lda,
                (float*)xt.data, &ldx, fwork, &lwork, &info);
        }
        else
        {
            work = (double*)ptr;
            dgels_(N, &m_, &n, &nb, (double*)at.data, &lda,
                (double*)xt.data, &ldx, work, &lwork, &info);
        }
    }
    else if( method == CV_CHOLESKY || (method == CV_LU && is_normal) )
    {
        if( type == CV_32F )
        {
            spotrf_(L, &n, (float*)at.data, &lda, &info);
            spotrs_(L, &n, &nb, (float*)at.data, &lda, (float*)xt.data, &ldx, &info);
        }
        else
        {
            dpotrf_(L, &n, (double*)at.data, &lda, &info);
            dpotrs_(L, &n, &nb, (double*)at.data, &lda, (double*)xt.data, &ldx, &info);
        }
    }
    else if( method == CV_LU )
    {
        iwork = (integer*)cvAlignPtr(ptr, sizeof(integer));
        if( type == CV_32F )
            sgesv_(&n, &nb, (float*)at.data, &lda, iwork, (float*)xt.data, &ldx, &info );
        else
            dgesv_(&n, &nb, (double*)at.data, &lda, iwork, (double*)xt.data, &ldx, &info );
    }
    else
        assert(0);
    assert(info == 0);
    result = info == 0;

    if( !result )
        dst = Scalar(0);
    else if( xt.data != dst.data )
        transpose( xt, dst );

    return result;
    }
}


/////////////////// finding eigenvalues and eigenvectors of a symmetric matrix ///////////////

static bool eigen( const Mat& src, Mat& evals, Mat& evects, bool computeEvects )
{
    bool result;
    integer i, n, m=0, lda, ldv, lwork=-1, iwork1=0, liwork=-1, idummy=0, info=0;
    integer *isupport, *iwork;
    char job[] = { computeEvects ? 'V' : 'N', '\0' };
    char A[] = {'A', '\0'}, L[] = {'L', '\0'};
    uchar* work;

    AutoBuffer<uchar> buf;

    int type = src.type();
    int elem_size = src.elemSize();
    lda = src.step/elem_size;
    n = ldv = src.rows;

    CV_Assert( src.rows == src.cols && (type == CV_32F || type == CV_64F));
    // allow for 1xn eigenvalue matrix too
    if( !(evals.rows == 1 && evals.cols == n && evals.type() == type) )
        evals.create(n, 1, type);
    
    if( computeEvects )
    {
        evects.create(n, n, type);
        ldv = evects.step/elem_size;
    }

    bool copy_evals = !evals.isContinuous();

    if( type == CV_32FC1 )
    {
        float work1 = 0, dummy = 0, abstol = 0, *s;

        ssyevr_(job, A, L, &n, (float*)src.data, &lda, &dummy, &dummy, &idummy, &idummy,
            &abstol, &m, (float*)evals.data, (float*)evects.data, &ldv,
            &idummy, &work1, &lwork, &iwork1, &liwork, &info );
        assert( info == 0 );

        lwork = cvRound(work1);
        liwork = iwork1;
        buf.allocate((lwork + n*n + (copy_evals ? n : 0))*elem_size +
                     (liwork+2*n+1)*sizeof(integer));
        Mat a(n, n, type, (uchar*)buf);
        work = a.data + n*n*elem_size;
        if( copy_evals )
            s = (float*)(work + lwork*elem_size);
        else
            s = (float*)evals.data;

        iwork = (integer*)cvAlignPtr(work + (lwork + (copy_evals ? n : 0))*elem_size, sizeof(integer));
        isupport = iwork + liwork;

        ssyevr_(job, A, L, &n, (float*)src.data, &lda, &dummy, &dummy,
            &idummy, &idummy, &abstol, &m, s, (float*)evects.data,
            &ldv, isupport, (float*)work, &lwork, iwork, &liwork, &info );
        assert( info == 0 );
        result = info == 0;

        for( i = 0; i < n/2; i++ )
            CV_SWAP(s[i], s[n-i-1], work1);
    }
    else
    {
        double work1 = 0, dummy = 0, abstol = 0, *s;

        dsyevr_(job, A, L, &n, (double*)src.data, &lda, &dummy, &dummy, &idummy, &idummy,
            &abstol, &m, (double*)evals.data, (double*)evects.data, &ldv,
            &idummy, &work1, &lwork, &iwork1, &liwork, &info );
        assert( info == 0 );

        lwork = cvRound(work1);
        liwork = iwork1;
        buf.allocate((lwork + n*n + (copy_evals ? n : 0))*elem_size +
                     (liwork+2*n+1)*sizeof(integer));
        Mat a(n, n, type, (uchar*)buf);
        work = a.data + n*n*elem_size;

        if( copy_evals )
            s = (double*)(work + lwork*elem_size);
        else
            s = (double*)evals.data;

        iwork = (integer*)cvAlignPtr(work + (lwork + (copy_evals ? n : 0))*elem_size, sizeof(integer));
        isupport = iwork + liwork;

        dsyevr_(job, A, L, &n, (double*)src.data, &lda, &dummy, &dummy,
            &idummy, &idummy, &abstol, &m, s, (double*)evects.data,
            &ldv, isupport, (double*)work, &lwork, iwork, &liwork, &info );
        assert( info == 0 );
        result = info == 0;

        for( i = 0; i < n/2; i++ )
            CV_SWAP(s[i], s[n-i-1], work1);
    }

    if( copy_evals )
        Mat(evals.rows, evals.cols, type, work + lwork*elem_size).copyTo(evals);

    if( computeEvects )
        flip(evects, evects, 0);

    return result;
}

bool eigen( const Mat& src, Mat& evals )
{
    Mat evects;
    return eigen(src, evals, evects, false);
}

bool eigen( const Mat& src, Mat& evals, Mat& evects )
{
    return eigen(src, evals, evects, true);
}



/* y[0:m,0:n] += diag(a[0:1,0:m]) * x[0:m,0:n] */
template<typename T1, typename T2, typename T3> static void
MatrAXPY( int m, int n, const T1* x, int dx,
          const T2* a, int inca, T3* y, int dy )
{
    int i, j;
    for( i = 0; i < m; i++, x += dx, y += dy )
    {
        T2 s = a[i*inca];
        for( j = 0; j <= n - 4; j += 4 )
        {
            T3 t0 = (T3)(y[j]   + s*x[j]);
            T3 t1 = (T3)(y[j+1] + s*x[j+1]);
            y[j]   = t0;
            y[j+1] = t1;
            t0 = (T3)(y[j+2] + s*x[j+2]);
            t1 = (T3)(y[j+3] + s*x[j+3]);
            y[j+2] = t0;
            y[j+3] = t1;
        }

        for( ; j < n; j++ )
            y[j] = (T3)(y[j] + s*x[j]);
    }
}

template<typename T> static void
SVBkSb( int m, int n, const T* w, int incw,
        const T* u, int ldu, int uT,
        const T* v, int ldv, int vT,
        const T* b, int ldb, int nb,
        T* x, int ldx, double* buffer, T eps )
{
    double threshold = 0;
    int udelta0 = uT ? ldu : 1, udelta1 = uT ? 1 : ldu;
    int vdelta0 = vT ? ldv : 1, vdelta1 = vT ? 1 : ldv;
    int i, j, nm = std::min(m, n);

    if( !b )
        nb = m;

    for( i = 0; i < n; i++ )
        for( j = 0; j < nb; j++ )
            x[i*ldx + j] = 0;

    for( i = 0; i < nm; i++ )
        threshold += w[i*incw];
    threshold *= eps;

    // v * inv(w) * uT * b
    for( i = 0; i < nm; i++, u += udelta0, v += vdelta0 )
    {
        double wi = w[i*incw];
        if( wi <= threshold )
            continue;
        wi = 1/wi;

        if( nb == 1 )
        {
            double s = 0;
            if( b )
                for( j = 0; j < m; j++ )
                    s += u[j*udelta1]*b[j*ldb];
            else
                s = u[0];
            s *= wi;

            for( j = 0; j < n; j++ )
                x[j*ldx] = (T)(x[j*ldx] + s*v[j*vdelta1]);
        }
        else
        {
            if( b )
            {
                for( j = 0; j < nb; j++ )
                    buffer[j] = 0;
                MatrAXPY( m, nb, b, ldb, u, udelta1, buffer, 0 );
                for( j = 0; j < nb; j++ )
                    buffer[j] *= wi;
            }
            else
            {
                for( j = 0; j < nb; j++ )
                    buffer[j] = u[j*udelta1]*wi;
            }
            MatrAXPY( n, nb, buffer, 0, v, vdelta1, x, ldx );
        }
    }
}


SVD& SVD::operator ()(const Mat& a, int flags)
{
    integer m = a.rows, n = a.cols, mn = std::max(m, n), nm = std::min(m, n);
    int type = a.type(), elem_size = a.elemSize();
    
    if( flags & NO_UV )
    {
        u.release();
        vt.release();
    }
    else
    {
        u.create( (int)m, (int)((flags & FULL_UV) ? m : nm), type );
        vt.create( (int)((flags & FULL_UV) ? n : nm), n, type );
    }

    w.create(nm, 1, type);

    Mat _a = a;
    int a_ofs = 0, work_ofs=0, iwork_ofs=0, buf_size = 0;
    bool temp_a = false;
    double u1=0, v1=0, work1=0;
    float uf1=0, vf1=0, workf1=0;
    integer lda, ldu, ldv, lwork=-1, iwork1=0, info=0, *iwork=0;
    char mode[] = {u.data || vt.data ? 'S' : 'N', '\0'};

    if( m != n && !(flags & NO_UV) && (flags & FULL_UV) )
        mode[0] = 'A';

    if( !(flags & MODIFY_A) )
    {
        if( mode[0] == 'N' || mode[0] == 'A' )
            temp_a = true;
        else if( ((vt.data && a.size() == vt.size()) || (u.data && a.size() == u.size())) &&
                  mode[0] == 'S' )
            mode[0] = 'O';
    }

    lda = a.cols;
    ldv = ldu = mn;

    if( type == CV_32F )
    {
        sgesdd_(mode, &n, &m, (float*)a.data, &lda, (float*)w.data,
            &vf1, &ldv, &uf1, &ldu, &workf1, &lwork, &iwork1, &info );
        lwork = cvRound(workf1);
    }
    else
    {
        dgesdd_(mode, &n, &m, (double*)a.data, &lda, (double*)w.data,
            &v1, &ldv, &u1, &ldu, &work1, &lwork, &iwork1, &info );
        lwork = cvRound(work1);
    }

    assert(info == 0);
    if( temp_a )
    {
        a_ofs = buf_size;
        buf_size += n*m*elem_size;
    }
    work_ofs = buf_size;
    buf_size += lwork*elem_size;
    buf_size = cvAlign(buf_size, sizeof(iwork[0]));
    iwork_ofs = buf_size;
    buf_size += 8*nm*sizeof(integer);
    
    AutoBuffer<uchar> buf(buf_size);
    uchar* buffer = (uchar*)buf;

    if( temp_a )
    {
        _a = Mat(a.rows, a.cols, type, buffer );
        a.copyTo(_a);
    }

    if( !(flags & MODIFY_A) && !temp_a )
    {
        if( vt.data && a.size() == vt.size() )
        {
            a.copyTo(vt);
            _a = vt;
        }
        else if( u.data && a.size() == u.size() )
        {
            a.copyTo(u);
            _a = u;
        }
    }

    if( mode[0] != 'N' )
    {
        ldv = vt.step ? vt.step/elem_size : vt.cols;
        ldu = u.step ? u.step/elem_size : u.cols;
    }

    lda = _a.step ? _a.step/elem_size : _a.cols;
    if( type == CV_32F )
    {
        sgesdd_(mode, &n, &m, (float*)_a.data, &lda, (float*)w.data,
            (float*)vt.data, &ldv, (float*)u.data, &ldu,
            (float*)(buffer + work_ofs), &lwork, (integer*)(buffer + iwork_ofs), &info );
    }
    else
    {
        dgesdd_(mode, &n, &m, (double*)_a.data, &lda, (double*)w.data,
            (double*)vt.data, &ldv, (double*)u.data, &ldu,
            (double*)(buffer + work_ofs), &lwork, (integer*)(buffer + iwork_ofs), &info );
    }
    assert(info == 0);
    return *this;
}


void SVD::backSubst( const Mat& rhs, Mat& dst ) const
{
    int type = w.type(), esz = w.elemSize();
    int m = u.rows, n = vt.cols, nb = rhs.data ? rhs.cols : m;
    AutoBuffer<double> buffer(nb);
    CV_Assert( u.data && vt.data && w.data );

    if( rhs.data )
        CV_Assert( rhs.type() == type && rhs.rows == m );

    dst.create( n, nb, type );
    if( type == CV_32F )
        SVBkSb(m, n, (float*)w.data, 1, (float*)u.data, u.step/esz, false,
            (float*)vt.data, vt.step/esz, true, (float*)rhs.data, rhs.step/esz,
            nb, (float*)dst.data, dst.step/esz, buffer, 10*FLT_EPSILON );
    else if( type == CV_64F )
        SVBkSb(m, n, (double*)w.data, 1, (double*)u.data, u.step/esz, false,
            (double*)vt.data, vt.step/esz, true, (double*)rhs.data, rhs.step/esz,
            nb, (double*)dst.data, dst.step/esz, buffer, 2*DBL_EPSILON );
    else
        CV_Error( CV_StsUnsupportedFormat, "" );
}

}


CV_IMPL double
cvDet( const CvArr* arr )
{
    return determinant(cv::cvarrToMat(arr));
}


CV_IMPL double
cvInvert( const CvArr* srcarr, CvArr* dstarr, int method )
{
    cv::Mat src = cv::cvarrToMat(srcarr), dst = cv::cvarrToMat(dstarr);

    CV_Assert( src.type() == dst.type() && src.rows == dst.cols && src.cols == dst.rows );
    return cv::invert( src, dst, method == CV_CHOLESKY ? cv::DECOMP_CHOLESKY :
        method == CV_SVD || method == CV_SVD_SYM ? cv::DECOMP_SVD : cv::DECOMP_LU );
}


CV_IMPL int
cvSolve( const CvArr* Aarr, const CvArr* barr, CvArr* xarr, int method )
{
    cv::Mat A = cv::cvarrToMat(Aarr), b = cv::cvarrToMat(barr), x = cv::cvarrToMat(xarr);

    CV_Assert( A.type() == x.type() && A.cols == x.rows && x.cols == b.cols );
    return cv::solve( A, b, x, method == CV_CHOLESKY ? cv::DECOMP_CHOLESKY :
        method == CV_SVD || method == CV_SVD_SYM ? cv::DECOMP_SVD :
        A.rows > A.cols ? cv::DECOMP_QR : cv::DECOMP_LU );
}


CV_IMPL void
cvEigenVV( CvArr* srcarr, CvArr* evectsarr, CvArr* evalsarr, double )
{
    cv::Mat src = cv::cvarrToMat(srcarr), evals = cv::cvarrToMat(evalsarr);
    if( evectsarr )
    {
        cv::Mat evects = cv::cvarrToMat(evectsarr);
        eigen(src, evals, evects);
    }
    else
        eigen(src, evals);
}


CV_IMPL void
cvSVD( CvArr* aarr, CvArr* warr, CvArr* uarr, CvArr* varr, int flags )
{
    cv::Mat a = cv::cvarrToMat(aarr), w = cv::cvarrToMat(warr), u, v;
    int m = a.rows, n = a.cols, type = a.type(), mn = std::max(m, n), nm = std::min(m, n);

    CV_Assert( w.type() == type &&
        (w.size() == cv::Size(nm,1) || w.size() == cv::Size(1, nm) ||
        w.size() == cv::Size(nm, nm) || w.size() == cv::Size(n, m)) );

    cv::SVD svd;

    if( w.size() == cv::Size(nm, 1) )
        svd.w = cv::Mat(nm, 1, type, w.data );
    else if( w.isContinuous() )
        svd.w = w;

    if( uarr )
    {
        u = cv::cvarrToMat(uarr);
        CV_Assert( u.type() == type );
        svd.u = u;
    }

    if( varr )
    {
        v = cv::cvarrToMat(varr);
        CV_Assert( v.type() == type );
        svd.vt = v;
    }

    svd(a, ((flags & CV_SVD_MODIFY_A) ? cv::SVD::MODIFY_A : 0) |
        ((!svd.u.data && !svd.vt.data) ? cv::SVD::NO_UV : 0) |
        ((m != n && (svd.u.size() == cv::Size(mn, mn) ||
        svd.vt.size() == cv::Size(mn, mn))) ? cv::SVD::FULL_UV : 0));

    if( u.data )
    {
        if( flags & CV_SVD_U_T )
            cv::transpose( svd.u, u );
        else if( u.data != svd.u.data )
        {
            CV_Assert( u.size() == svd.u.size() );
            svd.u.copyTo(u);
        }
    }

    if( v.data )
    {
        if( !(flags & CV_SVD_V_T) )
            cv::transpose( svd.vt, v );
        else if( v.data != svd.vt.data )
        {
            CV_Assert( v.size() == svd.vt.size() );
            svd.vt.copyTo(v);
        }
    }

    if( w.data != svd.w.data )
    {
        if( w.size() == svd.w.size() )
            svd.w.copyTo(w);
        else
        {
            w = cv::Scalar(0);
            cv::Mat wd = w.diag();
            svd.w.copyTo(wd);
        }
    }
}


CV_IMPL void
cvSVBkSb( const CvArr* warr, const CvArr* uarr,
          const CvArr* varr, const CvArr* rhsarr,
          CvArr* dstarr, int flags )
{
    cv::Mat w = cv::cvarrToMat(warr), u = cv::cvarrToMat(uarr),
        v = cv::cvarrToMat(varr), rhs, dst = cv::cvarrToMat(dstarr);
    int type = w.type();
    bool uT = (flags & CV_SVD_U_T) != 0, vT = (flags & CV_SVD_V_T) != 0;
    int m = !uT ? u.rows : u.cols;
    int n = vT ? v.cols : v.rows;
    int nm = std::min(n, m), nb;
    int esz = w.elemSize();
    int incw = w.size() == cv::Size(nm, 1) ? 1 : w.step/esz + (w.cols > 1 && w.rows > 1);

    CV_Assert( type == u.type() && type == v.type() &&
        type == dst.type() && dst.rows == n &&
        (!uT ? u.cols : u.rows) >= nm && (vT ? v.rows : v.cols) >= nm &&
        (w.size() == cv::Size(nm, 1) || w.size() == cv::Size(1, nm) ||
        w.size() == cv::Size(nm, nm) || w.size() == cv::Size(n, m)));

    if( rhsarr )
    {
        rhs = cv::cvarrToMat(rhsarr);
        nb = rhs.cols;
        CV_Assert( type == rhs.type() );
    }
    else
        nb = m;
    
    CV_Assert( dst.cols == nb );
    cv::AutoBuffer<double> buffer(nb);

    if( type == CV_32F )
        cv::SVBkSb(m, n, (float*)w.data, incw, (float*)u.data, u.step/esz, uT,
            (float*)v.data, v.step/esz, vT, (float*)rhs.data, rhs.step/esz,
            nb, (float*)dst.data, dst.step/esz, buffer, 2*FLT_EPSILON );
    else
        cv::SVBkSb(m, n, (double*)w.data, incw, (double*)u.data, u.step/esz, uT,
            (double*)v.data, v.step/esz, vT, (double*)rhs.data, rhs.step/esz,
            nb, (double*)dst.data, dst.step/esz, buffer, 2*DBL_EPSILON );
}
