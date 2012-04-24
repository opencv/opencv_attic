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
            int k = labels->data.i[i*ids_delta];
            double* c = (double*)(centers->data.ptr + k*centers->step);
            j = 0;
            for( ; j <= dims - 4; j += 4 )
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
                int i = cvRandInt( &rng ) % sample_count;
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

/* End of file. */

