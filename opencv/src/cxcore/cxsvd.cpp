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
#include <float.h>

/* y[0:m,0:n] += diag(a[0:1,0:m]) * x[0:m,0:n] */
static  void
icvMatrAXPY_64f( int m, int n, const double* x, int dx,
                 const double* a, double* y, int dy )
{
    int i, j;

    for( i = 0; i < m; i++, x += dx, y += dy )
    {
        double s = a[i];

        for( j = 0; j <= n - 4; j += 4 )
        {
            double t0 = y[j]   + s*x[j];
            double t1 = y[j+1] + s*x[j+1];
            y[j]   = t0;
            y[j+1] = t1;
            t0 = y[j+2] + s*x[j+2];
            t1 = y[j+3] + s*x[j+3];
            y[j+2] = t0;
            y[j+3] = t1;
        }

        for( ; j < n; j++ ) y[j] += s*x[j];
    }
}

static  void
icvMatrAXPY_32f( int m, int n, const float* x, int dx,
                 const float* a, float* y, int dy )
{
    int i, j;

    for( i = 0; i < m; i++, x += dx, y += dy )
    {
        double s = a[i];

        for( j = 0; j <= n - 4; j += 4 )
        {
            double t0 = y[j]   + s*x[j];
            double t1 = y[j+1] + s*x[j+1];
            y[j]   = (float)t0;
            y[j+1] = (float)t1;
            t0 = y[j+2] + s*x[j+2];
            t1 = y[j+3] + s*x[j+3];
            y[j+2] = (float)t0;
            y[j+3] = (float)t1;
        }

        for( ; j < n; j++ )
            y[j] = (float)(y[j] + s*x[j]);
    }
}


static void
icvSVBkSb_64f( int m, int n, const double* w,
               const double* uT, int lduT,
               const double* vT, int ldvT,
               const double* b, int ldb, int nb,
               double* x, int ldx, double* buffer )
{
    double threshold = w[0]*DBL_EPSILON;
    int i, j, nm = MIN( m, n );

    if( !b )
        nb = m;

    for( i = 0; i < n; i++ )
        memset( x + i*ldx, 0, nb*sizeof(x[0]));

    /*for( i = 0; i < nm; i++ )
        threshold += w[i];
    threshold *= 2*DBL_EPSILON;*/

    /* vT * inv(w) * uT * b */
    for( i = 0; i < nm; i++, uT += lduT, vT += ldvT )
    {
        double wi = w[i];

        if( wi > threshold )
        {
            wi = 1./wi;

            if( nb == 1 )
            {
                double s = 0;
                if( b )
                {
                    if( ldb == 1 )
                    {
                        for( j = 0; j <= m - 4; j += 4 )
                            s += uT[j]*b[j] + uT[j+1]*b[j+1] + uT[j+2]*b[j+2] + uT[j+3]*b[j+3];
                        for( ; j < m; j++ )
                            s += uT[j]*b[j];
                    }
                    else
                    {
                        for( j = 0; j < m; j++ )
                            s += uT[j]*b[j*ldb];
                    }
                }
                else
                    s = uT[0];
                s *= wi;
                if( ldx == 1 )
                {
                    for( j = 0; j <= n - 4; j += 4 )
                    {
                        double t0 = x[j] + s*vT[j];
                        double t1 = x[j+1] + s*vT[j+1];
                        x[j] = t0;
                        x[j+1] = t1;
                        t0 = x[j+2] + s*vT[j+2];
                        t1 = x[j+3] + s*vT[j+3];
                        x[j+2] = t0;
                        x[j+3] = t1;
                    }

                    for( ; j < n; j++ )
                        x[j] += s*vT[j];
                }
                else
                {
                    for( j = 0; j < n; j++ )
                        x[j*ldx] += s*vT[j];
                }
            }
            else
            {
                if( b )
                {
                    memset( buffer, 0, nb*sizeof(buffer[0]));
                    icvMatrAXPY_64f( m, nb, b, ldb, uT, buffer, 0 );
                    for( j = 0; j < nb; j++ )
                        buffer[j] *= wi;
                }
                else
                {
                    for( j = 0; j < nb; j++ )
                        buffer[j] = uT[j]*wi;
                }
                icvMatrAXPY_64f( n, nb, buffer, 0, vT, x, ldx );
            }
        }
    }
}


static void
icvSVBkSb_32f( int m, int n, const float* w,
               const float* uT, int lduT,
               const float* vT, int ldvT,
               const float* b, int ldb, int nb,
               float* x, int ldx, float* buffer )
{
    float threshold = w[0]*FLT_EPSILON;
    int i, j, nm = MIN( m, n );

    if( !b )
        nb = m;

    for( i = 0; i < n; i++ )
        memset( x + i*ldx, 0, nb*sizeof(x[0]));

    /*for( i = 0; i < nm; i++ )
        threshold += w[i];
    threshold *= 2*FLT_EPSILON;*/

    /* vT * inv(w) * uT * b */
    for( i = 0; i < nm; i++, uT += lduT, vT += ldvT )
    {
        double wi = w[i];
        
        if( wi > threshold )
        {
            wi = 1./wi;

            if( nb == 1 )
            {
                double s = 0;
                if( b )
                {
                    if( ldb == 1 )
                    {
                        for( j = 0; j <= m - 4; j += 4 )
                            s += uT[j]*b[j] + uT[j+1]*b[j+1] + uT[j+2]*b[j+2] + uT[j+3]*b[j+3];
                        for( ; j < m; j++ )
                            s += uT[j]*b[j];
                    }
                    else
                    {
                        for( j = 0; j < m; j++ )
                            s += uT[j]*b[j*ldb];
                    }
                }
                else
                    s = uT[0];
                s *= wi;

                if( ldx == 1 )
                {
                    for( j = 0; j <= n - 4; j += 4 )
                    {
                        double t0 = x[j] + s*vT[j];
                        double t1 = x[j+1] + s*vT[j+1];
                        x[j] = (float)t0;
                        x[j+1] = (float)t1;
                        t0 = x[j+2] + s*vT[j+2];
                        t1 = x[j+3] + s*vT[j+3];
                        x[j+2] = (float)t0;
                        x[j+3] = (float)t1;
                    }

                    for( ; j < n; j++ )
                        x[j] = (float)(x[j] + s*vT[j]);
                }
                else
                {
                    for( j = 0; j < n; j++ )
                        x[j*ldx] = (float)(x[j*ldx] + s*vT[j]);
                }
            }
            else
            {
                if( b )
                {
                    memset( buffer, 0, nb*sizeof(buffer[0]));
                    icvMatrAXPY_32f( m, nb, b, ldb, uT, buffer, 0 );
                    for( j = 0; j < nb; j++ )
                        buffer[j] = (float)(buffer[j]*wi);
                }
                else
                {
                    for( j = 0; j < nb; j++ )
                        buffer[j] = (float)(uT[j]*wi);
                }
                icvMatrAXPY_32f( n, nb, buffer, 0, vT, x, ldx );
            }
        }
    }
}


CV_IMPL void
cvSVD( CvArr* aarr, CvArr* warr, CvArr* uarr, CvArr* varr, int flags )
{
    uchar* buffer = 0;
    int local_alloc = 0;

    CV_FUNCNAME( "cvSVD" );

    __BEGIN__;

    CvMat astub0, astub, *a0 = (CvMat*)aarr, *a = a0;
    CvMat wstub0, wstub, *w0 = (CvMat*)warr, *w = w0;
    CvMat ustub0, ustub, *u0 = (CvMat*)uarr, *u = u0;
    CvMat vstub0, vstub, *v0 = (CvMat*)varr, *v = v0;

    integer m, n;
    int nm, type, elem_size, w_rows, w_cols, w_is_mat = 0, u_rows = 0, u_cols = 0, v_rows = 0, v_cols = 0;
    int a_ofs = 0, u_ofs = 0, v_ofs = 0, work_ofs=0, iwork_ofs=0, buf_size = 0;
    int temp_a = 0, temp_u = 0, temp_v = 0, temp_w = 1;
    double u1=0, v1=0, work1=0;
    float uf1=0, vf1=0, workf1=0;
    integer lda, ldu, ldv, lwork=-1, iwork1=0, info=0, *iwork=0;
    char mode[] = {u || v ? 'S' : 'N', '\0'};

    if( !CV_IS_MAT( a ))
        CV_CALL( a0 = a = cvGetMat( a, &astub0 ));

    if( !CV_IS_MAT( w ))
        CV_CALL( w0 = w = cvGetMat( w, &wstub0 ));

    if( !CV_ARE_TYPES_EQ( a, w ))
        CV_ERROR( CV_StsUnmatchedFormats, "" );

    m = a->rows;
    n = a->cols;
    nm = MIN(m, n);
    type = CV_MAT_TYPE(a->type);
    elem_size = CV_ELEM_SIZE(type);
    w_rows = w->rows;
    w_cols = w->cols;
    w_is_mat = w_cols > 1 && w_rows > 1;

    if( u )
    {
        if( !CV_IS_MAT( u ))
            CV_CALL( u0 = u = cvGetMat( u, &ustub0 ));

        if( !CV_ARE_TYPES_EQ( a, u ))
            CV_ERROR( CV_StsUnmatchedFormats, "" );
    }

    if( v )
    {
        if( !CV_IS_MAT( v ))
            CV_CALL( v0 = v = cvGetMat( v, &vstub0 ));

        if( !CV_ARE_TYPES_EQ( a, v ))
            CV_ERROR( CV_StsUnmatchedFormats, "" );
    }

    if( m != n &&
        ((u && u->rows == u->cols && u->rows == MAX(m,n)) ||
         (v && v->rows == v->cols && v->rows == MAX(m,n))))
        mode[0] = 'A';

    u_rows = m;
    u_cols = mode[0] == 'A' ? u_rows : nm;
    v_cols = n;
    v_rows = mode[0] == 'A' ? v_cols : nm;

    if( !w_is_mat && CV_IS_MAT_CONT(w->type) && w_cols + w_rows - 1 == nm )
        temp_w = 0;

    if( u || v )
    {
        temp_v = temp_u = 1;
        if( v && v->rows == v_rows && v->cols == v_cols )
            temp_v = 0;
        else if( (flags & CV_SVD_MODIFY_A) && mode[0] != 'A' &&
            a->rows == v_rows && a->cols == v_cols )
        {
            mode[0] = 'O';
            temp_v = 0;
        }

        if( u && u->rows == u_rows && u->cols == u_cols )
            temp_u = 0;
        else if( (flags & CV_SVD_MODIFY_A) && mode[0] != 'A' && mode[0] != 'O' &&
            a->rows == u_rows && a->cols == u_cols )
        {
            mode[0] = 'O';
            temp_u = 0;
        }
    }

    if( !(flags & CV_SVD_MODIFY_A) )
    {
        if( mode[0] == 'N' || mode[0] == 'A' )
            temp_a = 1;
        else if( ((v && CV_ARE_SIZES_EQ(a, v)) || (temp_v && v_rows == a->rows && v_cols == a->cols) ||
                  (u && CV_ARE_SIZES_EQ(a, u)) || (temp_u && u_rows == a->rows && u_cols == a->cols)) &&
                  mode[0] == 'S' )
            mode[0] = 'O';
    }

    lda = a->step/elem_size;
    ldv = n;
    ldu = m;

    if( type == CV_32F )
    {
        sgesdd_(mode, &n, &m, a->data.fl, &lda, w->data.fl,
            &vf1, &ldv, &uf1, &ldu, &workf1, &lwork, &iwork1, &info );
        lwork = cvRound(workf1);
    }
    else
    {
        dgesdd_(mode, &n, &m, a->data.db, &lda, w->data.db,
            &v1, &ldv, &u1, &ldu, &work1, &lwork, &iwork1, &info );
        lwork = cvRound(work1);
    }

    assert(info == 0);
    if( temp_w )
        buf_size += nm*elem_size;
    if( temp_a )
    {
        a_ofs = buf_size;
        buf_size += n*m*elem_size;
    }
    if( temp_v )
    {
        v_ofs = buf_size;
        buf_size += v_rows*v_cols*elem_size;
    }
    if( temp_u )
    {
        u_ofs = buf_size;
        buf_size += u_rows*u_cols*elem_size;
    }
    work_ofs = buf_size;
    buf_size += lwork*elem_size;
    buf_size = cvAlign(buf_size, sizeof(iwork[0]));
    iwork_ofs = buf_size;
    buf_size += 8*nm*sizeof(integer);

    if( buf_size <= CV_MAX_LOCAL_SIZE )
    {
        buffer = (uchar*)cvStackAlloc( buf_size );
        local_alloc = 1;
    }
    else
    {
        CV_CALL( buffer = (uchar*)cvAlloc( buf_size ));
    }

    if( temp_w )
        w = &(wstub = cvMat( 1, nm, type, buffer ));

    if( temp_a )
    {
        a = &(astub = cvMat( a->rows, a->cols, type, buffer + a_ofs ));
        cvCopy(a0, a);
    }

    if( temp_v )
        v = &(vstub = cvMat( v_rows, v_cols, type, buffer + v_ofs ));

    if( temp_u )
        u = &(ustub = cvMat( u_rows, u_cols, type, buffer + u_ofs ));

    if( !(flags & CV_SVD_MODIFY_A) && !temp_a )
    {
        if( v && CV_ARE_SIZES_EQ(a, v) )
        {
            cvCopy(a, v);
            a = v;
        }
        else if( u && CV_ARE_SIZES_EQ(a, u) )
        {
            cvCopy(a, u);
            a = u;
        }
    }

    if( mode[0] != 'N' )
    {
        if( !v )
            v = a;
        else if( !u )
            u = a;
        assert( u && v );
        ldv = v->step/elem_size;
        ldu = u->step/elem_size;
    }

    lda = a->step/elem_size;
    if( type == CV_32F )
    {
        sgesdd_(mode, &n, &m, a->data.fl, &lda, w->data.fl,
            v ? v->data.fl : &vf1, &ldv, u ? u->data.fl : &uf1, &ldu,
            (float*)(buffer + work_ofs), &lwork, (integer*)(buffer + iwork_ofs), &info );
    }
    else
    {
        dgesdd_(mode, &n, &m, a->data.db, &lda, w->data.db,
            v ? v->data.db : &v1, &ldv, u ? u->data.db : &u1, &ldu,
            (double*)(buffer + work_ofs), &lwork, (integer*)(buffer + iwork_ofs), &info );
    }
    assert(info == 0);

    if( w != w0 )
    {
        int shift = w0->cols != 1;
        cvSetZero( w0 );
        if( type == CV_32FC1 )
            for( int i = 0; i < n; i++ )
                ((float*)(w->data.ptr + i*w->step))[i*shift] = w->data.fl[i];
        else
            for( int i = 0; i < n; i++ )
                ((double*)(w->data.ptr + i*w->step))[i*shift] = w->data.db[i];
    }

    if( u0 )
    {
        if( flags & CV_SVD_U_T )
            cvTranspose( u, u0 );
        else if( u != u0 )
            cvCopy( u, u0 );
    }

    if( v0 )
    {
        if( !(flags & CV_SVD_V_T) )
            cvTranspose( v, v0 );
        else if( v != v0 )
            cvCopy( v, v0 );
    }

    CV_CHECK_NANS( w );

    __END__;

    if( buffer && !local_alloc )
        cvFree( &buffer );
}

CV_IMPL void
cvSVBkSb( const CvArr* warr, const CvArr* uarr,
          const CvArr* varr, const CvArr* barr,
          CvArr* xarr, int flags )
{
    uchar* buffer = 0;
    int local_alloc = 0;

    CV_FUNCNAME( "cvSVBkSb" );

    __BEGIN__;

    CvMat wstub, *w = (CvMat*)warr;
    CvMat bstub, *b = (CvMat*)barr;
    CvMat xstub, *x = (CvMat*)xarr;
    CvMat ustub, ustub2, *u = (CvMat*)uarr;
    CvMat vstub, vstub2, *v = (CvMat*)varr;
    uchar* tw = 0;
    int type;
    int temp_u = 0, temp_v = 0;
    int u_buf_offset = 0, v_buf_offset = 0, w_buf_offset = 0, t_buf_offset = 0;
    int buf_size = 0, pix_size;
    int m, n, nm;
    int u_rows, u_cols;
    int v_rows, v_cols;

    if( !CV_IS_MAT( w ))
        CV_CALL( w = cvGetMat( w, &wstub ));

    if( !CV_IS_MAT( u ))
        CV_CALL( u = cvGetMat( u, &ustub ));

    if( !CV_IS_MAT( v ))
        CV_CALL( v = cvGetMat( v, &vstub ));

    if( !CV_IS_MAT( x ))
        CV_CALL( x = cvGetMat( x, &xstub ));

    if( !CV_ARE_TYPES_EQ( w, u ) || !CV_ARE_TYPES_EQ( w, v ) || !CV_ARE_TYPES_EQ( w, x ))
        CV_ERROR( CV_StsUnmatchedFormats, "All matrices must have the same type" );

    type = CV_MAT_TYPE( w->type );
    pix_size = CV_ELEM_SIZE(type);

    if( !(flags & CV_SVD_U_T) )
    {
        temp_u = 1;
        u_buf_offset = buf_size;
        buf_size += u->cols*u->rows*pix_size;
        u_rows = u->rows;
        u_cols = u->cols;
    }
    else
    {
        u_rows = u->cols;
        u_cols = u->rows;
    }

    if( !(flags & CV_SVD_V_T) )
    {
        temp_v = 1;
        v_buf_offset = buf_size;
        buf_size += v->cols*v->rows*pix_size;
        v_rows = v->rows;
        v_cols = v->cols;
    }
    else
    {
        v_rows = v->cols;
        v_cols = v->rows;
    }

    m = u_rows;
    n = v_rows;
    nm = MIN(n,m);

    if( (u_rows != u_cols && v_rows != v_cols) || x->rows != v_rows )
        CV_ERROR( CV_StsBadSize, "V or U matrix must be square" );

    if( (w->rows == 1 || w->cols == 1) && w->rows + w->cols - 1 == nm )
    {
        if( CV_IS_MAT_CONT(w->type) )
            tw = w->data.ptr;
        else
        {
            w_buf_offset = buf_size;
            buf_size += nm*pix_size;
        }
    }
    else
    {
        if( w->cols != v_cols || w->rows != u_cols )
            CV_ERROR( CV_StsBadSize, "W must be 1d array of MIN(m,n) elements or "
                                    "matrix which size matches to U and V" );
        w_buf_offset = buf_size;
        buf_size += nm*pix_size;
    }

    if( b )
    {
        if( !CV_IS_MAT( b ))
            CV_CALL( b = cvGetMat( b, &bstub ));
        if( !CV_ARE_TYPES_EQ( w, b ))
            CV_ERROR( CV_StsUnmatchedFormats, "All matrices must have the same type" );
        if( b->cols != x->cols || b->rows != m )
            CV_ERROR( CV_StsUnmatchedSizes, "b matrix must have (m x x->cols) size" );
    }
    else
    {
        b = &bstub;
        memset( b, 0, sizeof(*b));
    }

    t_buf_offset = buf_size;
    buf_size += (MAX(m,n) + b->cols)*pix_size;

    if( buf_size <= CV_MAX_LOCAL_SIZE )
    {
        buffer = (uchar*)cvStackAlloc( buf_size );
        local_alloc = 1;
    }
    else
        CV_CALL( buffer = (uchar*)cvAlloc( buf_size ));

    if( temp_u )
    {
        cvInitMatHeader( &ustub2, u_cols, u_rows, type, buffer + u_buf_offset );
        cvT( u, &ustub2 );
        u = &ustub2;
    }

    if( temp_v )
    {
        cvInitMatHeader( &vstub2, v_cols, v_rows, type, buffer + v_buf_offset );
        cvT( v, &vstub2 );
        v = &vstub2;
    }

    if( !tw )
    {
        int i, shift = w->cols > 1 ? pix_size : 0;
        tw = buffer + w_buf_offset;
        for( i = 0; i < nm; i++ )
            memcpy( tw + i*pix_size, w->data.ptr + i*(w->step + shift), pix_size );
    }

    if( type == CV_32FC1 )
    {
        icvSVBkSb_32f( m, n, (float*)tw, u->data.fl, u->step/sizeof(float),
                       v->data.fl, v->step/sizeof(float),
                       b->data.fl, b->step/sizeof(float), b->cols,
                       x->data.fl, x->step/sizeof(float),
                       (float*)(buffer + t_buf_offset) );
    }
    else if( type == CV_64FC1 )
    {
        icvSVBkSb_64f( m, n, (double*)tw, u->data.db, u->step/sizeof(double),
                       v->data.db, v->step/sizeof(double),
                       b->data.db, b->step/sizeof(double), b->cols,
                       x->data.db, x->step/sizeof(double),
                       (double*)(buffer + t_buf_offset) );
    }
    else
    {
        CV_ERROR( CV_StsUnsupportedFormat, "" );
    }

    __END__;

    if( buffer && !local_alloc )
        cvFree( &buffer );
}

/* End of file. */
