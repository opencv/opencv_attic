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

#include "_cxcore.h"
#include "clapack.h"

/****************************************************************************************\
*                                 Determinant of the matrix                              *
\****************************************************************************************/

#define det2(m)   (m(0,0)*m(1,1) - m(0,1)*m(1,0))
#define det3(m)   (m(0,0)*(m(1,1)*m(2,2) - m(1,2)*m(2,1)) -  \
                   m(0,1)*(m(1,0)*m(2,2) - m(1,2)*m(2,0)) +  \
                   m(0,2)*(m(1,0)*m(2,1) - m(1,1)*m(2,0)))

CV_IMPL double
cvDet( const CvArr* arr )
{
    double result = 0;
    uchar* buffer = 0;
    int local_alloc = 0;
    
    CV_FUNCNAME( "cvDet" );

    __BEGIN__;

    CvMat stub, *mat = (CvMat*)arr;
    uchar* m;
    int type, step, rows;

    if( !CV_IS_MAT( mat ))
    {
        CV_CALL( mat = cvGetMat( mat, &stub ));
    }

    type = CV_MAT_TYPE( mat->type );

    if( mat->rows != mat->cols )
        CV_ERROR( CV_StsBadSize, "The matrix must be square" );

    m = mat->data.ptr;
    rows = mat->rows;
    step = mat->step;

    #define Mf( y, x ) ((float*)(m + y*step))[x]
    #define Md( y, x ) ((double*)(m + y*step))[x]

    if( type == CV_32F )
    {
        if( rows == 2 )
            result = det2(Mf);
        else if( rows == 3 )
            result = det3(Mf);
        else if( rows == 1 )
            result = mat->data.fl[0];
        else
        {
            integer i, n = rows, *ipiv, info=0;
            int bufSize = n*n*sizeof(float) + (n+1)*sizeof(ipiv[0]), sign=0;
            CvMat a;
            
            if( bufSize <= CV_MAX_LOCAL_SIZE )
            {
                buffer = (uchar*)cvStackAlloc( bufSize );
                local_alloc = 1;
            }
            else
            {
                CV_CALL( buffer = (uchar*)cvAlloc( bufSize ));
            }

            a = cvMat(n, n, CV_32F, buffer);
            cvCopy(mat, &a);
            ipiv = (integer*)cvAlignPtr(a.data.ptr + a.step*a.rows, sizeof(integer));

            sgetrf_(&n, &n, a.data.fl, &n, ipiv, &info);
            assert(info >= 0);

            if( info == 0 )
            {
                result = 1;
                for( i = 0; i < n; i++ )
                {
                    result *= a.data.fl[i*(n+1)];
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
            result = mat->data.db[0];
        else
        {
            integer i, n = rows, *ipiv, info=0;
            int bufSize = n*n*sizeof(double) + (n+1)*sizeof(ipiv[0]), sign=0;
            CvMat a;
            
            if( bufSize <= CV_MAX_LOCAL_SIZE )
            {
                buffer = (uchar*)cvStackAlloc( bufSize );
                local_alloc = 1;
            }
            else
            {
                CV_CALL( buffer = (uchar*)cvAlloc( bufSize ));
            }

            a = cvMat(n, n, CV_64F, buffer);
            cvCopy(mat, &a);
            ipiv = (integer*)cvAlignPtr(a.data.ptr + a.step*a.rows, sizeof(integer));

            dgetrf_(&n, &n, a.data.db, &n, ipiv, &info);
            assert(info >= 0);

            if( info == 0 )
            {
                result = 1;
                for( i = 0; i < n; i++ )
                {
                    result *= a.data.db[i*(n+1)];
                    sign ^= ipiv[i] != i+1;
                }
                result *= sign ? -1 : 1;
            }
        }
    }

    #undef Mf
    #undef Md

    __END__;

    if( buffer && !local_alloc )
        cvFree( &buffer );

    return result;
}



/****************************************************************************************\
*                          Inverse (or pseudo-inverse) of the matrix                     *
\****************************************************************************************/

#define Sf( y, x ) ((float*)(srcdata + y*srcstep))[x]
#define Sd( y, x ) ((double*)(srcdata + y*srcstep))[x]
#define Df( y, x ) ((float*)(dstdata + y*dststep))[x]
#define Dd( y, x ) ((double*)(dstdata + y*dststep))[x]

CV_IMPL double
cvInvert( const CvArr* srcarr, CvArr* dstarr, int method )
{
    CvMat* u = 0;
    CvMat* v = 0;
    CvMat* w = 0;

    uchar* buffer = 0;
    int local_alloc = 0;
    double result = 0;
    
    CV_FUNCNAME( "cvInvert" );

    __BEGIN__;

    CvMat sstub, *src = (CvMat*)srcarr;
    CvMat dstub, *dst = (CvMat*)dstarr;
    int type;

    if( !CV_IS_MAT( src ))
        CV_CALL( src = cvGetMat( src, &sstub ));

    if( !CV_IS_MAT( dst ))
        CV_CALL( dst = cvGetMat( dst, &dstub ));

    type = CV_MAT_TYPE( src->type );

    if( method == CV_SVD || method == CV_SVD_SYM )
    {
        int n = MIN(src->rows,src->cols);
        if( method == CV_SVD_SYM && src->rows != src->cols )
            CV_ERROR( CV_StsBadSize, "CV_SVD_SYM method is used for non-square matrix" );

        CV_CALL( u = cvCreateMat( n, src->rows, src->type ));
        if( method != CV_SVD_SYM )
            CV_CALL( v = cvCreateMat( n, src->cols, src->type ));
        CV_CALL( w = cvCreateMat( n, 1, src->type ));
        CV_CALL( cvSVD( src, w, u, v, CV_SVD_U_T + CV_SVD_V_T ));

        if( type == CV_32FC1 )
            result = w->data.fl[0] >= FLT_EPSILON ?
                     w->data.fl[w->rows-1]/w->data.fl[0] : 0;
        else
            result = w->data.db[0] >= FLT_EPSILON ?
                     w->data.db[w->rows-1]/w->data.db[0] : 0;

        CV_CALL( cvSVBkSb( w, u, v ? v : u, 0, dst, CV_SVD_U_T + CV_SVD_V_T ));
        EXIT;
    }
    else if( method != CV_LU && method != CV_CHOLESKY )
        CV_ERROR( CV_StsBadArg, "Unknown inversion method" );

    if( !CV_ARE_TYPES_EQ( src, dst ))
        CV_ERROR( CV_StsUnmatchedFormats, "" );

    if( src->width != src->height )
        CV_ERROR( CV_StsBadSize, "The matrix must be square" );

    if( !CV_ARE_SIZES_EQ( src, dst ))
        CV_ERROR( CV_StsUnmatchedSizes, "" );

    if( type != CV_32FC1 && type != CV_64FC1 )
        CV_ERROR( CV_StsUnsupportedFormat, "" );

    if( method == CV_LU || method == CV_CHOLESKY )
    {
        if( src->width <= 3 )
        {
            uchar* srcdata = src->data.ptr;
            uchar* dstdata = dst->data.ptr;
            int srcstep = src->step;
            int dststep = dst->step;

            if( src->width == 2 )
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
            else if( src->width == 3 )
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
                assert( src->width == 1 );

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
            EXIT;
        }

        {
        integer n = dst->cols, type = CV_MAT_TYPE(dst->type), lwork=-1,
            elem_size = CV_ELEM_SIZE(type), lda = dst->step/elem_size, piv1=0, info=0;
        int buf_size = (int)(n*sizeof(integer));

        cvCopy( src, dst );
        if( method == CV_LU )
        {
            if( type == CV_32F )
            {
                real work1 = 0;
                sgetri_(&n, dst->data.fl, &lda, &piv1, &work1, &lwork, &info);
                lwork = cvRound(work1);
            }
            else
            {
                double work1 = 0;
                dgetri_(&n, dst->data.db, &lda, &piv1, &work1, &lwork, &info);
                lwork = cvRound(work1);
            }

            buf_size += (int)((lwork + 1)*elem_size);
            if( buf_size <= CV_MAX_LOCAL_SIZE )
            {
                buffer = (uchar*)cvStackAlloc( buf_size );
                local_alloc = 1;
            }
            else
            {
                CV_CALL( buffer = (uchar*)cvAlloc( buf_size ));
            }

            if( type == CV_32F )
            {
                sgetrf_(&n, &n, dst->data.fl, &lda, (integer*)buffer, &info);
                sgetri_(&n, dst->data.fl, &lda, (integer*)buffer,
                    (float*)(buffer + n*sizeof(integer)), &lwork, &info);
            }
            else
            {
                dgetrf_(&n, &n, dst->data.db, &lda, (integer*)buffer, &info);
                dgetri_(&n, dst->data.db, &lda, (integer*)buffer,
                    (double*)cvAlignPtr(buffer + n*sizeof(integer), elem_size), &lwork, &info);
            }
        }
        else if( method == CV_CHOLESKY )
        {
            if( type == CV_32F )
            {
                spotrf_("L", &n, dst->data.fl, &lda, &info);
                spotri_("L", &n, dst->data.fl, &lda, &info);
            }
            else
            {
                dpotrf_("L", &n, dst->data.db, &lda, &info);
                dpotri_("L", &n, dst->data.db, &lda, &info);
            }
            cvCompleteSymm(dst);
        }
        result = info == 0;
        }
    }

    if( !result )
        CV_CALL( cvSetZero( dst ));

    __END__;

    if( buffer && !local_alloc )
        cvFree( &buffer );

    if( u || v || w )
    {
        cvReleaseMat( &u );
        cvReleaseMat( &v );
        cvReleaseMat( &w );
    }

    return result;
}


/****************************************************************************************\
*                               Linear system [least-squares] solution                   *
\****************************************************************************************/

CV_IMPL int
cvSolve( const CvArr* A, const CvArr* b, CvArr* x, int method )
{
    uchar *buffer = 0, *ptr;
    int local_alloc = 0;
    int result = 1;

    CV_FUNCNAME( "cvSolve" );

    __BEGIN__;

    CvMat sstub, *src = (CvMat*)A, at;
    CvMat dstub, *dst = (CvMat*)x, xt;
    CvMat bstub, *src2 = (CvMat*)b;
    double rcond=-1, s1=0, work1=0, *work=0, *s=0;
    float frcond=-1, fs1=0, fwork1=0, *fwork=0, *fs=0;
    integer m, m_, n, mn, nm, nb, lwork=-1, liwork=0, iwork1=0,
        lda, ldx, info=0, rank=0, *iwork=0;
    int type, elem_size, buf_size=0;
    bool copy_rhs=false, is_normal=(method & CV_NORMAL)!=0;

    if( !CV_IS_MAT( src ))
        CV_CALL( src = cvGetMat( src, &sstub ));

    if( !CV_IS_MAT( src2 ))
        CV_CALL( src2 = cvGetMat( src2, &bstub ));

    if( !CV_IS_MAT( dst ))
        CV_CALL( dst = cvGetMat( dst, &dstub ));

    type = CV_MAT_TYPE(src->type);

    if( !CV_ARE_TYPES_EQ(src, src2) || !CV_ARE_TYPES_EQ(src, dst) )
        CV_ERROR( CV_StsUnmatchedFormats,
        "All the input and output matrices must have the same type" );

    if( type != CV_32FC1 && type != CV_64FC1 )
        CV_ERROR( CV_StsUnsupportedFormat,
        "All the input and output matrices must be 32fC1 or 64fC1" );

    method &= ~CV_NORMAL;
    if( (method == CV_LU || method == CV_CHOLESKY) && !is_normal && src->rows != src->cols )
        CV_ERROR( CV_StsBadSize, "LU and Cholesky methods work only with square matrices" );

    // check case of a single equation and small matrix
    if( (method == CV_LU || method == CV_CHOLESKY) &&
        src->rows <= 3 && src->rows == src->cols && src2->cols == 1 )
    {
        #define bf(y) ((float*)(bdata + y*src2step))[0]
        #define bd(y) ((double*)(bdata + y*src2step))[0]

        uchar* srcdata = src->data.ptr;
        uchar* bdata = src2->data.ptr;
        uchar* dstdata = dst->data.ptr;
        int srcstep = src->step;
        int src2step = src2->step;
        int dststep = dst->step;

        if( src->width == 2 )
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
                    result = 0;
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
                    result = 0;
            }
        }
        else if( src->width == 3 )
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
                    result = 0;
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
                    result = 0;
            }
        }
        else
        {
            assert( src->width == 1 );

            if( type == CV_32FC1 )
            {
                double d = Sf(0,0);
                if( d != 0. )
                    Df(0,0) = (float)(bf(0)/d);
                else
                    result = 0;
            }
            else
            {
                double d = Sd(0,0);
                if( d != 0. )
                    Dd(0,0) = (bd(0)/d);
                else
                    result = 0;
            }
        }
    }

    elem_size = CV_ELEM_SIZE(src->type);
    lda = m = m_ = src->rows;
    n = src->cols;
    nb = src2->cols;
    ldx = mn = MAX(m, n);
    nm = MIN(m, n);

    if( m <= n )
        is_normal = false;
    else if( is_normal )
        m_ = n;

    buf_size += (is_normal ? n*n : m*n)*elem_size;

    if( m_ != n || nb > 1 || !CV_IS_MAT_CONT(dst->type) )
    {
        copy_rhs = true;
        if( is_normal )
            buf_size += n*nb*elem_size;
        else
            buf_size += mn*nb*elem_size;
    }

    if( method == CV_SVD || method == CV_SVD_SYM )
    {
        int nlvl = cvRound(log(MAX(MIN(m_,n)/25., 1.))/CV_LOG2) + 1;
        liwork = MIN(m_,n)*(3*MAX(nlvl,0) + 11);

        if( type == CV_32F )
            sgelsd_(&m_, &n, &nb, src->data.fl, &lda, dst->data.fl, &ldx,
                &fs1, &frcond, &rank, &fwork1, &lwork, &iwork1, &info);
        else
            dgelsd_(&m_, &n, &nb, src->data.db, &lda, dst->data.db, &ldx,
                &s1, &rcond, &rank, &work1, &lwork, &iwork1, &info );
        buf_size += nm*elem_size + (liwork + 1)*sizeof(integer);
    }
    else if( method == CV_QR )
    {
        if( type == CV_32F )
            sgels_("N", &m_, &n, &nb, src->data.fl, &lda, dst->data.fl, &ldx, &fwork1, &lwork, &info );
        else
            dgels_("N", &m_, &n, &nb, src->data.db, &lda, dst->data.db, &ldx, &work1, &lwork, &info );
    }
    else if( method == CV_LU )
    {
        buf_size += (n+1)*sizeof(integer);
    }
    else if( method == CV_CHOLESKY )
        ;
    else
        CV_ERROR( CV_StsBadArg, "Unknown method" );
    assert(info == 0);

    lwork = cvRound(type == CV_32F ? (double)fwork1 : work1);
    buf_size += lwork*elem_size;

    if( buf_size <= CV_MAX_LOCAL_SIZE )
    {
        buffer = (uchar*)cvStackAlloc( buf_size );
        local_alloc = 1;
    }
    else
        CV_CALL( buffer = (uchar*)cvAlloc( buf_size ));

    ptr = buffer;

    at = cvMat(n, m_, type, ptr);
    ptr += n*m_*elem_size;

    if( method == CV_SVD_SYM || method == CV_CHOLESKY )
        cvCopy( src, &at );
    else if( !is_normal )
        cvTranspose( src, &at );
    else
        cvMulTransposed( src, &at, 1 );

    if( !is_normal )
    {
        if( copy_rhs )
        {
            CvMat temp = cvMat(nb, mn, type, ptr), bt;
            ptr += nb*mn*elem_size;
            cvGetCols(&temp, &bt, 0, m);
            cvGetCols(&temp, &xt, 0, n);
            cvTranspose( src2, &bt );
        }
        else
        {
            cvCopy( src2, dst );
            xt = cvMat(1, n, type, dst->data.ptr);        
        }
    }
    else
    {
        if( copy_rhs )
        {
            xt = cvMat(nb, n, type, ptr);
            ptr += nb*n*elem_size;
        }
        else
            xt = cvMat(1, n, type, dst->data.ptr);
        // (a'*b)' = b'*a
        cvGEMM( src2, src, 1, 0, 0, &xt, CV_GEMM_A_T );
    }
    
    lda = at.step ? at.step/elem_size : at.cols;
    ldx = xt.step ? xt.step/elem_size : (!is_normal && copy_rhs ? mn : n);

    if( method == CV_SVD || method == CV_SVD_SYM )
    {
        if( type == CV_32F )
        {
            fs = (float*)ptr;
            ptr += nm*elem_size;
            fwork = (float*)ptr;
            ptr += lwork*elem_size;
            iwork = (integer*)cvAlignPtr(ptr, sizeof(integer));

            sgelsd_(&m_, &n, &nb, at.data.fl, &lda, xt.data.fl, &ldx,
                fs, &frcond, &rank, fwork, &lwork, iwork, &info);
        }
        else
        {
            s = (double*)ptr;
            ptr += nm*elem_size;
            work = (double*)ptr;
            ptr += lwork*elem_size;
            iwork = (integer*)cvAlignPtr(ptr, sizeof(integer));

            dgelsd_(&m_, &n, &nb, at.data.db, &lda, xt.data.db, &ldx,
                s, &rcond, &rank, work, &lwork, iwork, &info);
        }
    }
    else if( method == CV_QR )
    {
        if( type == CV_32F )
        {
            fwork = (float*)ptr;
            sgels_("N", &m_, &n, &nb, at.data.fl, &lda, xt.data.fl, &ldx, fwork, &lwork, &info);
        }
        else
        {
            work = (double*)ptr;
            dgels_("N", &m_, &n, &nb, at.data.db, &lda, xt.data.db, &ldx, work, &lwork, &info);
        }
    }
    else if( method == CV_CHOLESKY || (method == CV_LU && is_normal) )
    {
        if( type == CV_32F )
        {
            spotrf_("L", &n, at.data.fl, &lda, &info);
            spotrs_("L", &n, &nb, at.data.fl, &lda, xt.data.fl, &ldx, &info);
        }
        else
        {
            dpotrf_("L", &n, at.data.db, &lda, &info);
            dpotrs_("L", &n, &nb, at.data.db, &lda, xt.data.db, &ldx, &info);
        }
    }
    else if( method == CV_LU )
    {
        iwork = (integer*)cvAlignPtr(ptr, sizeof(integer));
        if( type == CV_32F )
            sgesv_(&n, &nb, at.data.fl, &lda, iwork, xt.data.fl, &ldx, &info );
        else
            dgesv_(&n, &nb, at.data.db, &lda, iwork, xt.data.db, &ldx, &info );
    }
    else
        assert(0);
    assert(info == 0);
    result = info == 0;

    if( !result )
        cvSetZero( dst );
    else if( xt.data.ptr != dst->data.ptr )
        cvTranspose( &xt, dst );

    __END__;

    if( buffer && !local_alloc )
        cvFree( &buffer );

    return result;
}


/////////////////// finding eigenvalues and eigenvectors of a symmetric matrix ///////////////

CV_IMPL void
cvEigenVV( CvArr* srcarr, CvArr* evectsarr, CvArr* evalsarr, double )
{
    uchar* work = 0;

    CV_FUNCNAME( "cvEigenVV" );

    __BEGIN__;

    CvMat sstub, *src = (CvMat*)srcarr;
    CvMat estub1, *evects = (CvMat*)evectsarr;
    CvMat estub2, *evals = (CvMat*)evalsarr;
    integer i, n, m=0, lda, ldv, lwork=-1, iwork1=0, liwork=-1, idummy=0, info=0, type, elem_size;
    integer *isupport, *iwork;
    bool copy_evals;
    char job[] = { evects ? 'V' : 'N', '\0' };

    if( !CV_IS_MAT( src ))
        CV_CALL( src = cvGetMat( src, &sstub ));

    if( !CV_IS_MAT( evals ))
        CV_CALL( evals = cvGetMat( evals, &estub2 ));

    type = CV_MAT_TYPE(src->type);
    elem_size = CV_ELEM_SIZE(src->type);
    lda = src->step/elem_size;
    n = ldv = src->cols;

    if( src->cols != src->rows )
        CV_ERROR( CV_StsUnmatchedSizes, "source is not quadratic matrix" );

    if( (evals->rows != src->rows || evals->cols != 1) &&
        (evals->cols != src->rows || evals->rows != 1))
        CV_ERROR( CV_StsBadSize, "eigenvalues vector has inappropriate size" );

    if( !CV_ARE_TYPES_EQ( src, evals ))
        CV_ERROR( CV_StsUnmatchedFormats,
        "input matrix, eigenvalues and eigenvectors must have the same type" );

    if( evects )
    {
        if( !CV_IS_MAT( evects ))
            CV_CALL( evects = cvGetMat( evects, &estub1 ));
        if( !CV_ARE_SIZES_EQ( src, evects) )
            CV_ERROR( CV_StsUnmatchedSizes,
                "eigenvectors matrix has inappropriate size" );
        if( !CV_ARE_TYPES_EQ( src, evects ))
            CV_ERROR( CV_StsUnmatchedFormats,
                "input matrix, eigenvalues and eigenvectors must have the same type" );
        ldv = evects->step/elem_size;
    }

    copy_evals = !CV_IS_MAT_CONT(evals->type);

    if( type == CV_32FC1 )
    {
        float work1 = 0, dummy = 0, abstol = 0, *s;

        ssyevr_(job, "A", "L", &n, src->data.fl, &lda, &dummy, &dummy, &idummy, &idummy,
            &abstol, &m, evals->data.fl, evects ? evects->data.fl : 0, &ldv,
            &idummy, &work1, &lwork, &iwork1, &liwork, &info );
        assert( info == 0 );

        lwork = cvRound(work1);
        liwork = iwork1;
        work = (uchar*)cvAlloc((lwork + (copy_evals ? n : 0))*elem_size +
                               (liwork+2*n+1)*sizeof(integer));
        if( copy_evals )
            s = (float*)(work + lwork*elem_size);
        else
            s = evals->data.fl;

        iwork = (integer*)cvAlignPtr(work + (lwork + (copy_evals ? n : 0))*elem_size, sizeof(integer));
        isupport = iwork + liwork;

        ssyevr_(job, "A", "L", &n, src->data.fl, &lda, &dummy, &dummy,
            &idummy, &idummy, &abstol, &m, s, evects ? evects->data.fl : 0,
            &ldv, isupport, (float*)work, &lwork, iwork, &liwork, &info );
        assert( info == 0 );

        for( i = 0; i < n/2; i++ )
            CV_SWAP(s[i], s[n-i-1], work1);
    }
    else if( type == CV_64FC1 )
    {
        double work1 = 0, dummy = 0, abstol = 0, *s;

        dsyevr_(job, "A", "L", &n, src->data.db, &lda, &dummy, &dummy, &idummy, &idummy,
            &abstol, &m, evals->data.db, evects ? evects->data.db : 0, &ldv,
            &idummy, &work1, &lwork, &iwork1, &liwork, &info );
        assert( info == 0 );

        lwork = cvRound(work1);
        liwork = iwork1;
        work = (uchar*)cvAlloc((lwork + (copy_evals ? n : 0))*elem_size +
                               (liwork+2*n+1)*sizeof(integer));
        if( copy_evals )
            s = (double*)(work + lwork*elem_size);
        else
            s = evals->data.db;

        iwork = (integer*)cvAlignPtr(work + (lwork + (copy_evals ? n : 0))*elem_size, sizeof(integer));
        isupport = iwork + liwork;

        dsyevr_(job, "A", "L", &n, src->data.db, &lda, &dummy, &dummy,
            &idummy, &idummy, &abstol, &m, s, evects ? evects->data.db : 0,
            &ldv, isupport, (double*)work, &lwork, iwork, &liwork, &info );
        assert( info == 0 );

        for( i = 0; i < n/2; i++ )
            CV_SWAP(s[i], s[n-i-1], work1);
    }
    else
    {
        CV_ERROR( CV_StsUnsupportedFormat, "Only 32fC1 and 64fC1 types are supported" );
    }

    if( copy_evals )
    {
        CvMat s = cvMat( evals->rows, evals->cols, type, work + lwork*elem_size );
        cvCopy( &s, evals );
    }

    if( evects )
        cvFlip(evects, evects, 0);

    __END__;

    cvFree( &work );
}
