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

CV_IMPL void
cvKMeans2( const CvArr* samples_arr, int cluster_count,
           CvArr* labels_arr, CvTermCriteria termcrit )
{
    CvMat* centers = 0;
    CvMat* old_centers = 0;
    CvMat* counters = 0;

    CV_FUNCNAME( "cvKMeans2" );

    __BEGIN__;

    CvMat samples_stub, labels_stub;
    CvMat* samples = (CvMat*)samples_arr;
    CvMat* labels = (CvMat*)labels_arr;
    CvMat* temp = 0;
    CvRNG rng = CvRNG(-1);
    int i, j, k, sample_count, dims;
    int ids_delta, iter;
    double max_dist;

    if( !CV_IS_MAT( samples ))
        CV_CALL( samples = cvGetMat( samples, &samples_stub ));

    if( !CV_IS_MAT( labels ))
        CV_CALL( labels = cvGetMat( labels, &labels_stub ));

    if( cluster_count < 1 )
        CV_ERROR( CV_StsOutOfRange, "Number of clusters should be positive" );

    if( CV_MAT_DEPTH(samples->type) != CV_32F || CV_MAT_TYPE(labels->type) != CV_32SC1 )
        CV_ERROR( CV_StsUnsupportedFormat,
        "samples should be floating-point matrix, cluster_idx - integer vector" );

    if( labels->rows != 1 && (labels->cols != 1 || !CV_IS_MAT_CONT(labels->type)) ||
        labels->rows + labels->cols - 1 != samples->rows )
        CV_ERROR( CV_StsUnmatchedSizes,
        "cluster_idx should be 1D vector of the same number of elements as samples' number of rows" );

    CV_CALL( termcrit = cvCheckTermCriteria( termcrit, 1e-6, 100 ));

    termcrit.epsilon *= termcrit.epsilon;
    sample_count = samples->rows;

    if( cluster_count > sample_count )
        cluster_count = sample_count;

    dims = samples->cols*CV_MAT_CN(samples->type);
    ids_delta = labels->step ? labels->step/(int)sizeof(int) : 1;

    CV_CALL( centers = cvCreateMat( cluster_count, dims, CV_64FC1 ));
    CV_CALL( old_centers = cvCreateMat( cluster_count, dims, CV_64FC1 ));
    CV_CALL( counters = cvCreateMat( 1, cluster_count, CV_32SC1 ));

    // init centers
    for( i = 0; i < sample_count; i++ )
        labels->data.i[i] = cvRandInt(&rng) % cluster_count;

    counters->cols = cluster_count; // cut down counters
    max_dist = termcrit.epsilon*2;

    for( iter = 0; iter < termcrit.max_iter; iter++ )
    {
        // computer centers
        cvZero( centers );
        cvZero( counters );

        for( i = 0; i < sample_count; i++ )
        {
            float* s = (float*)(samples->data.ptr + i*samples->step);
            k = labels->data.i[i*ids_delta];
            double* c = (double*)(centers->data.ptr + k*centers->step);
            for( j = 0; j <= dims - 4; j += 4 )
            {
                double t0 = c[j] + s[j];
                double t1 = c[j+1] + s[j+1];

                c[j] = t0;
                c[j+1] = t1;

                t0 = c[j+2] + s[j+2];
                t1 = c[j+3] + s[j+3];

                c[j+2] = t0;
                c[j+3] = t1;
            }
            for( ; j < dims; j++ )
                c[j] += s[j];
            counters->data.i[k]++;
        }

        if( iter > 0 )
            max_dist = 0;

        for( k = 0; k < cluster_count; k++ )
        {
            double* c = (double*)(centers->data.ptr + k*centers->step);
            if( counters->data.i[k] != 0 )
            {
                double scale = 1./counters->data.i[k];
                for( j = 0; j < dims; j++ )
                    c[j] *= scale;
            }
            else
            {
                i = cvRandInt( &rng ) % sample_count;
                float* s = (float*)(samples->data.ptr + i*samples->step);
                for( j = 0; j < dims; j++ )
                    c[j] = s[j];
            }
            
            if( iter > 0 )
            {
                double dist = 0;
                double* c_o = (double*)(old_centers->data.ptr + k*old_centers->step);
                for( j = 0; j < dims; j++ )
                {
                    double t = c[j] - c_o[j];
                    dist += t*t;
                }
                if( max_dist < dist )
                    max_dist = dist;
            }
        }

        // assign labels
        for( i = 0; i < sample_count; i++ )
        {
            float* s = (float*)(samples->data.ptr + i*samples->step);
            int k_best = 0;
            double min_dist = DBL_MAX;

            for( k = 0; k < cluster_count; k++ )
            {
                double* c = (double*)(centers->data.ptr + k*centers->step);
                double dist = 0;
                
                j = 0;
                for( ; j <= dims - 4; j += 4 )
                {
                    double t0 = c[j] - s[j];
                    double t1 = c[j+1] - s[j+1];
                    dist += t0*t0 + t1*t1;
                    t0 = c[j+2] - s[j+2];
                    t1 = c[j+3] - s[j+3];
                    dist += t0*t0 + t1*t1;
                }

                for( ; j < dims; j++ )
                {
                    double t = c[j] - s[j];
                    dist += t*t;
                }

                if( min_dist > dist )
                {
                    min_dist = dist;
                    k_best = k;
                }
            }

            labels->data.i[i*ids_delta] = k_best;
        }

        if( max_dist < termcrit.epsilon )
            break;

        CV_SWAP( centers, old_centers, temp );
    }

    cvZero( counters );
    for( i = 0; i < sample_count; i++ )
        counters->data.i[labels->data.i[i]]++;

    // ensure that we do not have empty clusters
    for( k = 0; k < cluster_count; k++ )
        if( counters->data.i[k] == 0 )
            for(;;)
            {
                i = cvRandInt(&rng) % sample_count;
                j = labels->data.i[i];
                if( counters->data.i[j] > 1 )
                {
                    labels->data.i[i] = k;
                    counters->data.i[j]--;
                    counters->data.i[k]++;
                    break;
                }
            }

    __END__;

    cvReleaseMat( &centers );
    cvReleaseMat( &old_centers );
    cvReleaseMat( &counters );
}


/*
  Finds real roots of cubic, quadratic or linear equation.
  The original code has been taken from Ken Turkowski web page
  (http://www.worldserver.com/turk/opensource/) and adopted for OpenCV.
  Here is the copyright notice.

  -----------------------------------------------------------------------
  Copyright (C) 1978-1999 Ken Turkowski. <turk@computer.org>
 
    All rights reserved.
 
    Warranty Information
      Even though I have reviewed this software, I make no warranty
      or representation, either express or implied, with respect to this
      software, its quality, accuracy, merchantability, or fitness for a
      particular purpose.  As a result, this software is provided "as is,"
      and you, its user, are assuming the entire risk as to its quality
      and accuracy.
 
    This code may be used and freely distributed as long as it includes
    this copyright notice and the above warranty information.
  -----------------------------------------------------------------------
*/
CV_IMPL int
cvSolveCubic( const CvMat* coeffs, CvMat* roots )
{
    int n = 0;
    
    CV_FUNCNAME( "cvSolveCubic" );

    __BEGIN__;

    double a0 = 1., a1, a2, a3;
    double x0 = 0., x1 = 0., x2 = 0.;
    int step = 1, coeff_count;
    
    if( !CV_IS_MAT(coeffs) )
        CV_ERROR( !coeffs ? CV_StsNullPtr : CV_StsBadArg, "Input parameter is not a valid matrix" );

    if( !CV_IS_MAT(roots) )
        CV_ERROR( !roots ? CV_StsNullPtr : CV_StsBadArg, "Output parameter is not a valid matrix" );

    if( CV_MAT_TYPE(coeffs->type) != CV_32FC1 && CV_MAT_TYPE(coeffs->type) != CV_64FC1 ||
        CV_MAT_TYPE(roots->type) != CV_32FC1 && CV_MAT_TYPE(roots->type) != CV_64FC1 )
        CV_ERROR( CV_StsUnsupportedFormat,
        "Both matrices should be floating-point (single or double precision)" );

    coeff_count = coeffs->rows + coeffs->cols - 1;

    if( coeffs->rows != 1 && coeffs->cols != 1 || coeff_count != 3 && coeff_count != 4 )
        CV_ERROR( CV_StsBadSize,
        "The matrix of coefficients must be 1-dimensional vector of 3 or 4 elements" );

    if( roots->rows != 1 && roots->cols != 1 ||
        roots->rows + roots->cols - 1 != 3 )
        CV_ERROR( CV_StsBadSize,
        "The matrix of roots must be 1-dimensional vector of 3 elements" );

    if( CV_MAT_TYPE(coeffs->type) == CV_32FC1 )
    {
        const float* c = coeffs->data.fl;
        if( coeffs->rows > 1 )
            step = coeffs->step/sizeof(c[0]);
        if( coeff_count == 4 )
            a0 = c[0], c += step;
        a1 = c[0];
        a2 = c[step];
        a3 = c[step*2];
    }
    else
    {
        const double* c = coeffs->data.db;
        if( coeffs->rows > 1 )
            step = coeffs->step/sizeof(c[0]);
        if( coeff_count == 4 )
            a0 = c[0], c += step;
        a1 = c[0];
        a2 = c[step];
        a3 = c[step*2];
    }

    if( a0 == 0 )
    {
        if( a1 == 0 )
        {
            if( a2 == 0 )
                n = a3 == 0 ? -1 : 0;
            else
            {
                // linear equation
                x0 = a3/a2;
                n = 1;
            }
        }
        else
        {
            // quadratic equation
            double d = a2*a2 - 4*a1*a3;
            if( d >= 0 )
            {
                d = sqrt(d);
                double q = (-a2 + (a2 < 0 ? -d : d)) * 0.5;
                x0 = q / a1;
                x1 = a3 / q;
                n = d > 0 ? 2 : 1;
            }
        }
    }
    else
    {
        a0 = 1./a0;
        a1 *= a0;
        a2 *= a0;
        a3 *= a0;

        double Q = (a1 * a1 - 3 * a2) * (1./9);
        double R = (2 * a1 * a1 * a1 - 9 * a1 * a2 + 27 * a3) * (1./54);
        double Qcubed = Q * Q * Q;
        double d = Qcubed - R * R;
    
        if( d >= 0 )
        {
            double theta = acos(R / sqrt(Qcubed));
            double sqrtQ = sqrt(Q);
            double t0 = -2 * sqrtQ;
            double t1 = theta * (1./3);
            double t2 = a1 * (1./3);
            x0 = t0 * cos(t1) - t2;
            x1 = t0 * cos(t1 + (2.*CV_PI/3)) - t2;
            x2 = t0 * cos(t1 + (4.*CV_PI/3)) - t2;
            n = 3;
        }
        else
        {
            double e;
            d = sqrt(-d);
            e = pow(d + fabs(R), 0.333333333333);
            if( R > 0 )
                e = -e;
            x0 = (e + Q / e) - a1 * (1./3);
            n = 1;
        }
    }

    step = 1;

    if( CV_MAT_TYPE(roots->type) == CV_32FC1 )
    {
        float* r = roots->data.fl;
        if( roots->rows > 1 )
            step = roots->step/sizeof(r[0]);
        r[0] = (float)x0;
        r[step] = (float)x1;
        r[step*2] = (float)x2;
    }
    else
    {
        double* r = roots->data.db;
        if( roots->rows > 1 )
            step = roots->step/sizeof(r[0]);
        r[0] = x0;
        r[step] = x1;
        r[step*2] = x2;
    }

    __END__;

    return n;
}


CV_IMPL void cvNormalize( const CvArr* src, CvArr* dst,
                          double a, double b, int norm_type, const CvArr* mask )
{
    CvMat* tmp = 0;

    CV_FUNCNAME( "cvNormalize" );

    __BEGIN__;

    double scale, shift;
    
    if( norm_type == CV_MINMAX )
    {
        double smin = 0, smax = 0;
        double dmin = MIN( a, b ), dmax = MAX( a, b );
        cvMinMaxLoc( src, &smin, &smax, 0, 0, mask );
        scale = (dmax - dmin)*(smax - smin > DBL_EPSILON ? 1./(smax - smin) : 0);
        shift = dmin - smin*scale;
    }
    else if( norm_type == CV_L2 || norm_type == CV_L1 || norm_type == CV_C )
    {
        CvMat *s = (CvMat*)src, *d = (CvMat*)dst;
        
        if( CV_IS_MAT(s) && CV_IS_MAT(d) && CV_IS_MAT_CONT(s->type & d->type) &&
            CV_ARE_TYPES_EQ(s,d) && CV_ARE_SIZES_EQ(s,d) && !mask &&
            s->cols*s->rows <= CV_MAX_INLINE_MAT_OP_SIZE*CV_MAX_INLINE_MAT_OP_SIZE )
        {
            int i, len = s->cols*s->rows;
            double norm = 0, v;

            if( CV_MAT_TYPE(s->type) == CV_32FC1 )
            {
                const float* sptr = s->data.fl;
                float* dptr = d->data.fl;
                
                if( norm_type == CV_L2 )
                {
                    for( i = 0; i < len; i++ )
                    {
                        v = sptr[i];
                        norm += v*v;
                    }
                    norm = sqrt(norm);
                }
                else if( norm_type == CV_L1 )
                    for( i = 0; i < len; i++ )
                    {
                        v = fabs((double)sptr[i]);
                        norm += v;
                    }
                else
                    for( i = 0; i < len; i++ )
                    {
                        v = fabs((double)sptr[i]);
                        norm = MAX(norm,v);
                    }

                norm = norm > DBL_EPSILON ? 1./norm : 0.;
                for( i = 0; i < len; i++ )
                    dptr[i] = (float)(sptr[i]*norm);
                EXIT;
            }

            if( CV_MAT_TYPE(s->type) == CV_64FC1 )
            {
                const double* sptr = s->data.db;
                double* dptr = d->data.db;
                
                if( norm_type == CV_L2 )
                {
                    for( i = 0; i < len; i++ )
                    {
                        v = sptr[i];
                        norm += v*v;
                    }
                    norm = sqrt(norm);
                }
                else if( norm_type == CV_L1 )
                    for( i = 0; i < len; i++ )
                    {
                        v = fabs(sptr[i]);
                        norm += v;
                    }
                else
                    for( i = 0; i < len; i++ )
                    {
                        v = fabs(sptr[i]);
                        norm = MAX(norm,v);
                    }

                norm = norm > DBL_EPSILON ? 1./norm : 0.;
                for( i = 0; i < len; i++ )
                    dptr[i] = sptr[i]*norm;
                EXIT;
            }
        }
        
        scale = cvNorm( src, 0, norm_type, mask );
        scale = scale > DBL_EPSILON ? 1./scale : 0.;
        shift = 0;
    }
    else
        CV_ERROR( CV_StsBadArg, "Unknown/unsupported norm type" );
    
    if( !mask )
        cvConvertScale( src, dst, scale, shift );
    else
    {
        CvMat stub, *dmat;
        CV_CALL( dmat = cvGetMat(dst, &stub));
        CV_CALL( tmp = cvCreateMat(dmat->rows, dmat->cols, dmat->type) );
        cvConvertScale( src, tmp, scale, shift );
        cvCopy( tmp, dst, mask );
    }

    __END__;

    if( tmp )
        cvReleaseMat( &tmp );
}


CV_IMPL void cvRandShuffle( CvArr* arr, CvRNG* rng, double iter_factor )
{
    CV_FUNCNAME( "cvRandShuffle" );

    __BEGIN__;

    const int sizeof_int = (int)sizeof(int);
    CvMat stub, *mat = (CvMat*)arr;
    int i, j, k, iters, delta = 0;
    int cont_flag, arr_size, elem_size, cols, step;
    const int pair_buf_sz = 100;
    int* pair_buf = (int*)cvStackAlloc( pair_buf_sz*sizeof(pair_buf[0])*2 );
    CvMat _pair_buf = cvMat( 1, pair_buf_sz*2, CV_32S, pair_buf );
    CvRNG _rng = cvRNG(-1);
    uchar* data = 0;
    int* idata = 0;
    
    if( !CV_IS_MAT(mat) )
        CV_CALL( mat = cvGetMat( mat, &stub ));

    if( !rng )
        rng = &_rng;

    cols = mat->cols;
    step = mat->step;
    arr_size = cols*mat->rows;
    iters = cvRound(iter_factor*arr_size)*2;
    cont_flag = CV_IS_MAT_CONT(mat->type);
    elem_size = CV_ELEM_SIZE(mat->type);
    if( elem_size % sizeof_int == 0 && (cont_flag || step % sizeof_int == 0) )
    {
        idata = mat->data.i;
        step /= sizeof_int;
        elem_size /= sizeof_int;
    }
    else
        data = mat->data.ptr;

    for( i = 0; i < iters; i += delta )
    {
        delta = MIN( iters - i, pair_buf_sz*2 );
        _pair_buf.cols = delta;
        cvRandArr( rng, &_pair_buf, CV_RAND_UNI, cvRealScalar(0), cvRealScalar(arr_size) );
        
        if( cont_flag )
        {
            if( idata )
                for( j = 0; j < delta; j += 2 )
                {
                    int* p = idata + pair_buf[j]*elem_size, *q = idata + pair_buf[j+1]*elem_size, t;
                    for( k = 0; k < elem_size; k++ )
                        CV_SWAP( p[k], q[k], t );
                }
            else
                for( j = 0; j < delta; j += 2 )
                {
                    uchar* p = data + pair_buf[j]*elem_size, *q = data + pair_buf[j+1]*elem_size, t;
                    for( k = 0; k < elem_size; k++ )
                        CV_SWAP( p[k], q[k], t );
                }
        }
        else
        {
            if( idata )
                for( j = 0; j < delta; j += 2 )
                {
                    int idx1 = pair_buf[j], idx2 = pair_buf[j+1], row1, row2;
                    int* p, *q, t;
                    row1 = idx1/step; row2 = idx2/step;
                    p = idata + row1*step + (idx1 - row1*cols)*elem_size;
                    q = idata + row2*step + (idx2 - row2*cols)*elem_size;
                    
                    for( k = 0; k < elem_size; k++ )
                        CV_SWAP( p[k], q[k], t );
                }
            else
                for( j = 0; j < delta; j += 2 )
                {
                    int idx1 = pair_buf[j], idx2 = pair_buf[j+1], row1, row2;
                    uchar* p, *q, t;
                    row1 = idx1/step; row2 = idx2/step;
                    p = data + row1*step + (idx1 - row1*cols)*elem_size;
                    q = data + row2*step + (idx2 - row2*cols)*elem_size;
                    
                    for( k = 0; k < elem_size; k++ )
                        CV_SWAP( p[k], q[k], t );
                }
        }
    }

    __END__;
}


CV_IMPL CvArr*
cvRange( CvArr* arr, double start, double end )
{
    int ok = 0;
    
    CV_FUNCNAME( "cvRange" );

    __BEGIN__;
    
    CvMat stub, *mat = (CvMat*)arr;
    double delta;
    int type, step;
    double val = start;
    int i, j;
    int rows, cols;
    
    if( !CV_IS_MAT(mat) )
        CV_CALL( mat = cvGetMat( mat, &stub) );

    rows = mat->rows;
    cols = mat->cols;
    type = CV_MAT_TYPE(mat->type);
    delta = (end-start)/(rows*cols);

    if( CV_IS_MAT_CONT(mat->type) )
    {
        cols *= rows;
        rows = 1;
        step = 1;
    }
    else
        step = mat->step / CV_ELEM_SIZE(type);

    if( type == CV_32SC1 )
    {
        int* idata = mat->data.i;
        int ival = cvRound(val), idelta = cvRound(delta);

        if( fabs(val - ival) < DBL_EPSILON &&
            fabs(delta - idelta) < DBL_EPSILON )
        {
            for( i = 0; i < rows; i++, idata += step )
                for( j = 0; j < cols; j++, ival += idelta )
                    idata[j] = ival;
        }
        else
        {
            for( i = 0; i < rows; i++, idata += step )
                for( j = 0; j < cols; j++, val += delta )
                    idata[j] = cvRound(val);
        }
    }
    else if( type == CV_32FC1 )
    {
        float* fdata = mat->data.fl;
        for( i = 0; i < rows; i++, fdata += step )
            for( j = 0; j < cols; j++, val += delta )
                fdata[j] = (float)val;
    }
    else
        CV_ERROR( CV_StsUnsupportedFormat, "The function only supports 32sC1 and 32fC1 datatypes" );

    ok = 1;

    __END__;

    return ok ? arr : 0;
}

/* End of file. */
