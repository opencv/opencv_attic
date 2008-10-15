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

#ifdef HAVE_CONFIG_H
    #include <cvconfig.h>
#endif

#ifdef HAVE_MALLOC_H
    #include <malloc.h>
#endif

#include <stdio.h>
#include <memory.h>
#include <float.h>
#include <math.h>

#include <time.h>
#include <limits.h>

#include <_cvcommon.h>
#include <_cvhaartraining.h>
#include <cvclassifier.h>

#ifdef _OPENMP
#include <omp.h>
#endif /* _OPENMP */

#define CV_BOOST_IMPL

const int rsort_bits1 = (sizeof(float)*8+2)/3, rsort_bits2 = rsort_bits1;
const int rsort_bits3 = sizeof(float)*8 - rsort_bits1 - rsort_bits2;
const int rsort_max_bits = rsort_bits1, rsort_max_size = 1 << rsort_max_bits;
const int rsort_shift2 = rsort_bits1, rsort_shift3 = rsort_shift2 + rsort_bits2;
const int rsort_mask1 = (1 << rsort_bits1) - 1;
const int rsort_mask2 = (1 << rsort_bits2) - 1;
const int rsort_mask3 = (1 << rsort_bits3) - 1;
const int rsort_h_size = rsort_mask1 + rsort_mask2 + rsort_mask3 + 6;

#define toggle_32f32u(x) ((x) ^ (((x) >> 31) | 0x80000000))
#define toggle_32u32f(x) ((x) ^ ((~(x) >> 31) | 0x80000000))

static void
icvRadixSortIndexedValArray( const idx_type* src, int size,
                             float* fdata, idx_type* dst, idx_type* buf )
{
    idx_type hist[rsort_h_size];
    const int h2_ofs = 2 + rsort_mask1, h3_ofs = h2_ofs + 2 + rsort_mask2;
    int i, j, k, pos;
    int v, s;
    int* idata = (int*)fdata;

    /* pass 1. compute all histograms */
    for( k = 0; k < rsort_h_size; k++ )
        hist[k] = 0;

    for( j = 0; j < size; j++ )
    {
        i = src[j];
        v = idata[i];
        v = toggle_32f32u(v);
        int k1 = v & rsort_mask1;
        int pos1 = hist[k1+1] + 1;
        int k2 = (v >> rsort_shift2) & rsort_mask2;
        int pos2 = hist[k2+h2_ofs+1] + 1;
        int k3 = (unsigned)v >> rsort_shift3;
        int pos3 = hist[k3+h3_ofs+1] + 1;
        idata[i] = v;
        hist[k1 + 1] = (idx_type)pos1;
        hist[k2 + h2_ofs+1] = (idx_type)pos2;
        hist[k3 + h3_ofs+1] = (idx_type)pos3;
    }

    for( k = 0, s = -1; k < h2_ofs; k++ )
        hist[k] = (idx_type)(s += hist[k]);

    for( k = h2_ofs, s = -1; k < h3_ofs; k++ )
        hist[k] = (idx_type)(s += hist[k]);

    for( k = h3_ofs, s = -1; k < rsort_h_size; k++ )
        hist[k] = (idx_type)(s += hist[k]);

    /* pass 2. sort by the lower rsort_bits1 bits */
    for( j = 0; j < size; j++ )
    {
        int i = src[j];
        int v = idata[i];
        k = v & rsort_mask1;
        pos = hist[k]+1;
        dst[pos] = (idx_type)i;
        hist[k] = (idx_type)pos;
    }

    /* pass 3. sort by the next rsort_bits2 bits */
    for( j = 0; j < size; j++ )
    {
        int i = dst[j];
        int v = idata[i];
        k = (v >> rsort_shift2) & rsort_mask2;
        pos = hist[k + h2_ofs]+1;
        buf[pos] = (idx_type)i;
        hist[k + h2_ofs] = (idx_type)pos;
    }

    /* pass 4. sort by the highest rsort_bits3 bits */
    for( j = 0; j < size; j++ )
    {
        int i = buf[j];
        int v = idata[i];
        k = (unsigned)v >> rsort_shift3;
        pos = hist[k + h3_ofs]+1;
        idata[i] = toggle_32u32f(v);
        dst[pos] = (idx_type)i;
        hist[k + h3_ofs] = (idx_type)pos;
    }
}


CV_BOOST_IMPL
void cvGetSortedIndices( CvMat* val, CvMat* idx, int sortcols )
{
    int i, rows = idx->rows, cols = idx->cols;
    idx_type *src, *buf;

    OPENCV_ASSERT( CV_MAT_TYPE(idx->type) == CV_16SC1 &&
            CV_MAT_TYPE(val->type) == CV_32FC1 &&
            CV_ARE_SIZES_EQ(val, idx) && sortcols == 0, "", "" );

    src = (idx_type*)cvAlloc( cols*sizeof(src[0]) );
    buf = (idx_type*)cvAlloc( cols*sizeof(buf[0]) ); 

    for( i = 0; i < cols; i++ )
        src[i] = (idx_type)i;

    for( i = 0; i < rows; i++ )
    {
        idx_type* dst = (idx_type*)(idx->data.ptr + idx->step*i); 
        icvRadixSortIndexedValArray( src, cols,
            (float*)(val->data.ptr + val->step*i), dst, buf );
    }

    cvFree( &src );
    cvFree( &buf );
}


CV_BOOST_IMPL
void cvReleaseStumpClassifier( CvClassifier** classifier )
{
    cvFree( classifier );
    *classifier = 0;
}


CV_BOOST_IMPL
float cvEvalStumpClassifier( CvClassifier* _classifier, CvMat* sample )
{
    CvStumpClassifier* classifier = (CvStumpClassifier*)_classifier;
    assert( !classifier && !sample && CV_MAT_TYPE( sample->type ) == CV_32FC1 );
    
    return sample->data.fl[classifier->compidx] < classifier->threshold ?
        classifier->left : classifier->right; 
}


static int icvFindStumpThreshold(
        const float* data, const float* wdata,
        const float* ydata, const idx_type* idxdata, int num,
        CvStumpClassifier* stump, CvStumpError errtype,
        double sumw, double sumwy, double sumwyy )
{
    int found = 0;
    double wyl = 0, wl = 0, wyyl = 0;
    double prevval = -FLT_MAX;
    double lerror = stump->lerror, rerror = stump->rerror;
    double min_error = lerror + rerror;
    double left = 0, right = 0, threshold = 0;
    int i;

    for( i = 0; i < num; i++ )
    {
        int idx = idxdata[i];
        double curval = data[idx];
        double w, wr = sumw  - wl;

        if( wl > 0 && wr > 0 && curval > prevval )
        {
            double wyr = sumwy - wyl;
            w = wl*wr;

            if( errtype == CV_SQUARE )
            {
                // min least squares approximation:
                // err = sum( w * (y - val_{left|right})^2 )
                // note: a special algorithm is used to reduce the number of operations
                double err1 = wyl*wyl*wr + wyr*wyr*wl;

                if( (sumwyy - min_error)*w < err1 )
                {
                    w = 1./w;
                    left = wyl*wr*w;
                    right = wyr*wl*w;
                    min_error = sumwyy - err1*w;
                    lerror = wyyl - left*wyl;
                    rerror = min_error - lerror;
                    threshold = i > 0 ? 0.5 * (curval + prevval) : curval;
                    found = 1;
                }
            }
            else
            {
                w = 1./w;
                double wposl = 0.5*(wl + wyl);
                double wposr = 0.5*(wr + wyr);
                double curleft = 0.5*(1 + wyl*wr*w);
                double curright = 0.5*(1 + wyr*wl*w);
                double curlerror, currerror;

                if( errtype == CV_GINI )
                {
                    // gini error: err = 2 * wpos * wneg /(wpos + wneg)
                    curlerror = 2 * wposl * (1 - curleft);
                    currerror = 2 * wposr * (1 - curright);
                }
                else if( errtype == CV_MISCLASSIFICATION )
                {
                    // misclassification error: err = MIN( wpos, wneg );
                    curlerror = MIN( wposl, wl - wposl );
                    currerror = MIN( wposr, wr - wposr );
                }
                else
                {
                    const double eps = FLT_MIN;
                    // entropy:
                    // err = - wpos * log(wpos /(wpos + wneg)) - wneg * log(wneg/(wpos + wneg))
                    curlerror = currerror = 0;

                    if( curleft > eps )
                        curlerror -= wposl * log( curleft );
                    if( curleft < 1 - eps )
                        curlerror -= (wl - wposl) * log( 1 - curleft );

                    if( curright > eps )
                        currerror -= wposr * log( curright );
                    if( curright < 1 - eps )
                        currerror -= (wr - wposr) * log( 1 - curright );
                }

                if( curlerror + currerror < min_error )
                {
                    left = curleft;
                    right = curright;
                    min_error = curlerror + currerror;
                    lerror = curlerror;
                    rerror = currerror;
                    threshold = i > 0 ? 0.5 * (curval + prevval) : curval;
                    found = 1;
                }
            }
        }

        //do
        {
            w = wdata[idx];
            double wy = wdata[idx]*ydata[idx];
            wl += w; wyl += wy;
            wyyl += wy*ydata[idx];
        }
        //while( ++i < num && data[idx = idxdata[i]] == curval );
        //--i;
        prevval = curval;
    }

    if( found )
    {
        stump->left = (float)left;
        stump->right = (float)right;
        stump->lerror = (float)lerror;
        stump->rerror = (float)rerror;
        stump->threshold = (float)threshold;
    }

    return found;
}

static void
icvCalcWSums( const float* wdata,
    const float* ydata, const idx_type* idxdata, int num,
    double* _sumw, double* _sumwy, double* _sumwyy )
{
    double sumw = 0, sumwy = 0, sumwyy = 0;
    int i;

    for( i = 0; i < num; i++ )
    {
        int idx = idxdata[i];
        double w = wdata[idx];
        double y = ydata[idx];
        double wy = w*y;
        sumw += w;
        sumwy += wy;
        sumwyy += wy*y;
    }

    *_sumw = sumw;
    *_sumwy = sumwy;
    *_sumwyy = sumwyy;
}


/*
 * cvCreateMTStumpClassifier
 *
 * Multithreaded stump classifier constructor
 * Includes huge train data support through callback function
 */
CV_BOOST_IMPL
CvClassifier* cvCreateMTStumpClassifier( CvMat* trainData,
                      int flags,
                      CvMat* trainClasses,
                      CvMat* ,
                      CvMat* ,
                      CvMat* ,
                      CvMat* sampleIdx,
                      CvMat* weights,
                      CvClassifierTrainParams* _trainParams )
{
    CvStumpClassifier* best_stump = 0;
    CvMTStumpTrainParams* trainParams = (CvMTStumpTrainParams*)_trainParams;
    int m = trainClasses->cols + trainClasses->rows - 1; /* number of samples */
    int n = 0; /* number of components */
    int l = 0; /* number of active samples */
    int datan = 0; /* num components */
    float* ydata = trainClasses->data.fl;
    idx_type* idxdata = 0;
    double sumw = 0, sumwy = 0, sumwyy = 0;
    
    float* wdata = weights->data.fl;

    idx_type* sorteddata = 0;
    size_t sortedstep = 0; /* component step */
    /* size of the cache of sorted indexes */
    int sortedn = 0; /* number of rows */
    int sortedm = 0; /* number of columns */

    char* mask = 0;
    const int min_portion0 = 10, max_portion1 = 50;
    int i, t_compidx, portion0;
    
    CvStumpError stumperror = trainParams->error;
    int max_threads = cvGetNumThreads();

    idx_type* sorted_idx[CV_MAX_THREADS] = {0};
    CvStumpClassifier stumps[CV_MAX_THREADS];
    CvMat* callback_data[CV_MAX_THREADS] = {0};

    assert( CV_MAT_TYPE(trainClasses->type) == CV_32FC1 &&
            CV_IS_MAT_CONT(trainClasses->type & weights->type) );

    if( trainParams->sortedIdx )
    {
        assert( CV_MAT_TYPE(trainParams->sortedIdx->type) == CV_16SC1 );
        sorteddata = (idx_type*)trainParams->sortedIdx->data.ptr;
        sortedstep = trainParams->sortedIdx->step/sizeof(sorteddata[0]);
        sortedn = trainParams->sortedIdx->rows;
        sortedm = trainParams->sortedIdx->cols;
    }

    if( !trainData )
    {
        n = trainParams->numcomp;
        assert( trainParams->getTrainData != 0 && n > 0 );
    }
    else
    {
        assert( CV_MAT_TYPE(trainData->type) == CV_32FC1 && trainData->cols == m );
        datan = n = trainData->rows;

        if( trainParams->getTrainData != 0 )
            n = trainParams->numcomp;
    }
    assert( datan <= n );

    l = !sampleIdx ? m : sampleIdx->rows + sampleIdx->cols - 1;
    idxdata = (idx_type*)cvAlloc( l*sizeof(idxdata[0]) );
    if( sampleIdx != 0 )
    {
        assert( CV_MAT_TYPE( sampleIdx->type ) == CV_32FC1 );
        const float* _idxdata = sampleIdx->data.fl;
        int idxstep = MAX( sampleIdx->step/sizeof(_idxdata[0]), 1 );

        for( i = 0; i < l; i++ )
            idxdata[i] = (idx_type)cvRound(_idxdata[i*idxstep]);

        if( sorteddata != 0 )
        {
            mask = (char*)cvAlloc( m );
            memset( mask, 0, m );
            for( i = 0; i < l; i++ )
                mask[idxdata[i]] = (char)1;
        }
    }
    else
        for( i = 0; i < l; i++ )
            idxdata[i] = (idx_type)i;

    best_stump = (CvStumpClassifier*)cvAlloc( sizeof(*best_stump) );
    memset( best_stump, 0, sizeof(*best_stump) );

    best_stump->eval = cvEvalStumpClassifier;
    best_stump->tune = 0;
    best_stump->save = 0;
    best_stump->release = cvReleaseStumpClassifier;

    best_stump->lerror = best_stump->rerror = FLT_MAX;
    best_stump->left = best_stump->right = 0.f;
    
    portion0 = trainParams->portion;
    if( portion0 < 1 )
        portion0 = MAX( (1 << 20)/m, min_portion0 );

    for( i = 0; i < max_threads; i++ )
    {
        sorted_idx[i] = (idx_type*)cvAlloc( l*2*sizeof(sorted_idx[i][0]) );
        stumps[i] = *best_stump;

        if( datan < n )
            callback_data[i] = cvCreateMat( MIN(portion0, max_portion1), m, CV_32F );
    }

    icvCalcWSums( wdata, ydata, idxdata, l, &sumw, &sumwy, &sumwyy );

    #ifdef _OPENMP
    #pragma omp parallel for num_threads(max_threads) schedule(dynamic)
    #endif /* _OPENMP */
    for( t_compidx = 0; t_compidx < n; t_compidx += portion0 )
    {
        /*
        This function, unlike cvCreateStumpClassifier,
        can compute the features and sort their indexes on-fly.
        Then, it can use "cache", i.e. some precomputed features
        ("datan" rows of trainData) and/or
        some previously sorted indexes
        ("sortedn" rows of sorteddata).
        In general sortedn != datan, so for every feature we
        need to be able to handle all 4 cases:
        1) no feature values and no sorted indexes in cache,
        2) only sorted indices are in cache
        3) only feature values in cache
        4) both feature values and sorted indexes are in cache.
        that makes the loop body more complex.
        */
        int thread_idx = cvGetThreadNum();
        float lerror = FLT_MAX, rerror = FLT_MAX;
        float left  = 0.f, right = 0.f, threshold = 0.f;
        int optcompidx = 0;
        int ti, t_n = MIN( portion0, n - t_compidx );

        idx_type *t_idx = 0;
        CvStumpClassifier* stump = &stumps[thread_idx];

        for( ti = t_compidx; ti < t_compidx + t_n; )
        {
            CvMat mat;
            int t_n1 = t_n; // when all the feature values are in cache,
                            // this nested loop body is run just once
            if( ti + t_n1 > datan )
            {
                // we are completely or partially outside of
                // "precomputed feature values" zone.
                
                if( ti < datan )
                    // if we are partially outside, let's proceed up to the boundary.
                    t_n1 = datan - ti;
                else
                    // otherwise, we probably need to decrease
                    // the step to fit the temporary buffer
                    t_n1 = MIN( t_n1, max_portion1 );
            }
            
            // now, regarding feature vals availability in cache, we have just 2 options:
            // 1) the whole stripe is in cache
            // 2) the whole stripe is not in cache
            if( ti < datan )
            {
                cvInitMatHeader( &mat, t_n1, m, CV_32F,
                    trainData->data.ptr + trainData->step*ti, trainData->step );
            }
            else
            {
                cvInitMatHeader( &mat, t_n1, m, CV_32F,
                    callback_data[thread_idx]->data.ptr,
                    callback_data[thread_idx]->step );

                trainParams->getTrainData( &mat, sampleIdx, 0,
                    ti, t_n1, trainParams->userdata );
            }

            for( t_n1 += ti; ti < t_n1; ti++, mat.data.ptr += mat.step )
            {
                int tj, tk = l;
                if( ti < sortedn )
                {
                    t_idx = sorteddata + ti*sortedstep;
                    tk = sortedm;
                    if( mask )
                    {
                        t_idx = sorted_idx[thread_idx];
                        for( tj = tk = 0; tj < sortedm; tj++ )
                        {
                            int curidx = sorteddata[ti*sortedstep + tj];
                            if( mask[curidx] != 0 )
                                t_idx[tk++] = (idx_type)curidx;
                        }
                    }
                }
                else
                {
                    t_idx = sorted_idx[thread_idx];
                    icvRadixSortIndexedValArray( idxdata, tk, mat.data.fl, t_idx, t_idx + tk );
                }

                if( icvFindStumpThreshold( mat.data.fl, wdata, ydata,
                        t_idx, tk, stump, stumperror, sumw, sumwy, sumwyy ) )
                    stump->compidx = ti;
            }
        }
    }

    cvFree( &mask );
    cvFree( &idxdata );

    for( i = 0; i < max_threads; i++ )
    {
        cvFree( &sorted_idx[i] );
        cvReleaseMat( &callback_data[i] );

        if( (double)stumps[i].lerror + stumps[i].rerror <
            (double)best_stump->lerror + best_stump->rerror )
            *best_stump = stumps[i];
    }

    if( trainParams->type == CV_CLASSIFICATION_CLASS )
    {
        best_stump->left = best_stump->left >= 0.5f ? 1.f : -1.f;
        best_stump->right = best_stump->right >= 0.5f ? 1.f : -1.f;
    }

    return (CvClassifier*)best_stump;
}


CV_BOOST_IMPL
float cvEvalCARTClassifier( CvClassifier* classifier, CvMat* sample )
{
    CV_FUNCNAME( "cvEvalCARTClassifier" );

    int idx;

    __BEGIN__;

    CV_ASSERT( classifier != NULL );
    CV_ASSERT( sample != NULL );
    CV_ASSERT( CV_MAT_TYPE( sample->type ) == CV_32FC1 );
    CV_ASSERT( sample->rows == 1 || sample->cols == 1 );

    idx = 0;
    if( sample->rows == 1 )
    {
        do
        {
            if( (CV_MAT_ELEM( (*sample), float, 0,
                    ((CvCARTClassifier*) classifier)->compidx[idx] )) <
                ((CvCARTClassifier*) classifier)->threshold[idx] ) 
            {
                idx = ((CvCARTClassifier*) classifier)->left[idx];
            }
            else
            {
                idx = ((CvCARTClassifier*) classifier)->right[idx];
            }
        } while( idx > 0 );
    }
    else
    {
        do
        {
            if( (CV_MAT_ELEM( (*sample), float,
                    ((CvCARTClassifier*) classifier)->compidx[idx], 0 )) <
                ((CvCARTClassifier*) classifier)->threshold[idx] ) 
            {
                idx = ((CvCARTClassifier*) classifier)->left[idx];
            }
            else
            {
                idx = ((CvCARTClassifier*) classifier)->right[idx];
            }
        } while( idx > 0 );
    } 

    __END__;

    return ((CvCARTClassifier*) classifier)->val[-idx];
}

CV_BOOST_IMPL
float cvEvalCARTClassifierIdx( CvClassifier* classifier, CvMat* sample )
{
    CV_FUNCNAME( "cvEvalCARTClassifierIdx" );

    int idx;

    __BEGIN__;


    CV_ASSERT( classifier != NULL );
    CV_ASSERT( sample != NULL );
    CV_ASSERT( CV_MAT_TYPE( sample->type ) == CV_32FC1 );
    CV_ASSERT( sample->rows == 1 || sample->cols == 1 );

    idx = 0;
    if( sample->rows == 1 )
    {
        do
        {
            if( (CV_MAT_ELEM( (*sample), float, 0,
                    ((CvCARTClassifier*) classifier)->compidx[idx] )) <
                ((CvCARTClassifier*) classifier)->threshold[idx] ) 
            {
                idx = ((CvCARTClassifier*) classifier)->left[idx];
            }
            else
            {
                idx = ((CvCARTClassifier*) classifier)->right[idx];
            }
        } while( idx > 0 );
    }
    else
    {
        do
        {
            if( (CV_MAT_ELEM( (*sample), float,
                    ((CvCARTClassifier*) classifier)->compidx[idx], 0 )) <
                ((CvCARTClassifier*) classifier)->threshold[idx] ) 
            {
                idx = ((CvCARTClassifier*) classifier)->left[idx];
            }
            else
            {
                idx = ((CvCARTClassifier*) classifier)->right[idx];
            }
        } while( idx > 0 );
    } 

    __END__;

    return (float) (-idx);
}

CV_BOOST_IMPL
void cvReleaseCARTClassifier( CvClassifier** classifier )
{
    cvFree( classifier );
    *classifier = NULL;
}

void CV_CDECL icvDefaultSplitIdx_R( int compidx, float threshold,
                                    CvMat* idx, CvMat** left, CvMat** right,
                                    void* userdata )
{
    CvMat* trainData = (CvMat*) userdata;
    int i = 0;

    *left = cvCreateMat( 1, trainData->rows, CV_32FC1 );
    *right = cvCreateMat( 1, trainData->rows, CV_32FC1 );
    (*left)->cols = (*right)->cols = 0;
    if( idx == NULL )
    {
        for( i = 0; i < trainData->rows; i++ )
        {
            if( CV_MAT_ELEM( *trainData, float, i, compidx ) < threshold )
            {
                (*left)->data.fl[(*left)->cols++] = (float) i;
            }
            else
            {
                (*right)->data.fl[(*right)->cols++] = (float) i;
            }
        }
    }
    else
    {
        uchar* idxdata;
        int idxnum;
        int idxstep;
        int index;

        idxdata = idx->data.ptr;
        idxnum = (idx->rows == 1) ? idx->cols : idx->rows;
        idxstep = (idx->rows == 1) ? CV_ELEM_SIZE( idx->type ) : idx->step;
        for( i = 0; i < idxnum; i++ )
        {
            index = (int) *((float*) (idxdata + i * idxstep));
            if( CV_MAT_ELEM( *trainData, float, index, compidx ) < threshold )
            {
                (*left)->data.fl[(*left)->cols++] = (float) index;
            }
            else
            {
                (*right)->data.fl[(*right)->cols++] = (float) index;
            }
        }
    }
}

void CV_CDECL icvDefaultSplitIdx_C( int compidx, float threshold,
                                    CvMat* idx, CvMat** left, CvMat** right,
                                    void* userdata )
{
    CvMat* trainData = (CvMat*) userdata;
    int i = 0;

    *left = cvCreateMat( 1, trainData->cols, CV_32FC1 );
    *right = cvCreateMat( 1, trainData->cols, CV_32FC1 );
    (*left)->cols = (*right)->cols = 0;
    if( idx == NULL )
    {
        for( i = 0; i < trainData->cols; i++ )
        {
            if( CV_MAT_ELEM( *trainData, float, compidx, i ) < threshold )
            {
                (*left)->data.fl[(*left)->cols++] = (float) i;
            }
            else
            {
                (*right)->data.fl[(*right)->cols++] = (float) i;
            }
        }
    }
    else
    {
        uchar* idxdata;
        int idxnum;
        int idxstep;
        int index;

        idxdata = idx->data.ptr;
        idxnum = (idx->rows == 1) ? idx->cols : idx->rows;
        idxstep = (idx->rows == 1) ? CV_ELEM_SIZE( idx->type ) : idx->step;
        for( i = 0; i < idxnum; i++ )
        {
            index = (int) *((float*) (idxdata + i * idxstep));
            if( CV_MAT_ELEM( *trainData, float, compidx, index ) < threshold )
            {
                (*left)->data.fl[(*left)->cols++] = (float) index;
            }
            else
            {
                (*right)->data.fl[(*right)->cols++] = (float) index;
            }
        }
    }
}

/* internal structure used in CART creation */
typedef struct CvCARTNode
{
    CvMat* sampleIdx;
    CvStumpClassifier* stump;
    int parent;
    int leftflag;
    float errdrop;
} CvCARTNode;

CV_BOOST_IMPL
CvClassifier* cvCreateCARTClassifier( CvMat* trainData,
                     int flags,
                     CvMat* trainClasses,
                     CvMat* typeMask,
                     CvMat* missedMeasurementsMask,
                     CvMat* compIdx,
                     CvMat* sampleIdx,
                     CvMat* weights,
                     CvClassifierTrainParams* trainParams )
{
    CvCARTClassifier* cart = NULL;
    size_t datasize = 0;
    int count = 0;
    int i = 0;
    int j = 0;
    
    CvCARTNode* intnode = NULL;
    CvCARTNode* list = NULL;
    int listcount = 0;
    CvMat* lidx = NULL;
    CvMat* ridx = NULL;
    
    float maxerrdrop = 0.0F;
    int idx = 0;

    void (*splitIdxCallback)( int compidx, float threshold,
                              CvMat* idx, CvMat** left, CvMat** right,
                              void* userdata );
    void* userdata;

    count = ((CvCARTTrainParams*) trainParams)->count;
    
    assert( count > 0 );

    datasize = sizeof( *cart ) + (sizeof( float ) + 3 * sizeof( int )) * count + 
        sizeof( float ) * (count + 1);
    
    cart = (CvCARTClassifier*) cvAlloc( datasize );
    memset( cart, 0, datasize );
    
    cart->count = count;
    
    cart->eval = cvEvalCARTClassifier;
    cart->save = NULL;
    cart->release = cvReleaseCARTClassifier;

    cart->compidx = (int*) (cart + 1);
    cart->threshold = (float*) (cart->compidx + count);
    cart->left  = (int*) (cart->threshold + count);
    cart->right = (int*) (cart->left + count);
    cart->val = (float*) (cart->right + count);

    datasize = sizeof( CvCARTNode ) * (count + count);
    intnode = (CvCARTNode*) cvAlloc( datasize );
    memset( intnode, 0, datasize );
    list = (CvCARTNode*) (intnode + count);

    splitIdxCallback = ((CvCARTTrainParams*) trainParams)->splitIdx;
    userdata = ((CvCARTTrainParams*) trainParams)->userdata;
    if( splitIdxCallback == NULL )
    {
        splitIdxCallback = ( CV_IS_ROW_SAMPLE( flags ) )
            ? icvDefaultSplitIdx_R : icvDefaultSplitIdx_C;
        userdata = trainData;
    }

    /* create root of the tree */
    intnode[0].sampleIdx = sampleIdx;
    intnode[0].stump = (CvStumpClassifier*)
        ((CvCARTTrainParams*) trainParams)->stumpConstructor( trainData, flags,
            trainClasses, typeMask, missedMeasurementsMask, compIdx, sampleIdx, weights,
            ((CvCARTTrainParams*) trainParams)->stumpTrainParams );
    cart->left[0] = cart->right[0] = 0;

    /* build tree */
    listcount = 0;
    for( i = 1; i < count; i++ )
    {
        /* split last added node */
        splitIdxCallback( intnode[i-1].stump->compidx, intnode[i-1].stump->threshold,
            intnode[i-1].sampleIdx, &lidx, &ridx, userdata );
        
        if( intnode[i-1].stump->lerror != 0.0F )
        {
            list[listcount].sampleIdx = lidx;
            list[listcount].stump = (CvStumpClassifier*)
                ((CvCARTTrainParams*) trainParams)->stumpConstructor( trainData, flags,
                    trainClasses, typeMask, missedMeasurementsMask, compIdx,
                    list[listcount].sampleIdx,
                    weights, ((CvCARTTrainParams*) trainParams)->stumpTrainParams );
            list[listcount].errdrop = intnode[i-1].stump->lerror
                - (list[listcount].stump->lerror + list[listcount].stump->rerror);
            list[listcount].leftflag = 1;
            list[listcount].parent = i-1;
            listcount++;
        }
        else
        {
            cvReleaseMat( &lidx );
        }
        if( intnode[i-1].stump->rerror != 0.0F )
        {
            list[listcount].sampleIdx = ridx;
            list[listcount].stump = (CvStumpClassifier*)
                ((CvCARTTrainParams*) trainParams)->stumpConstructor( trainData, flags,
                    trainClasses, typeMask, missedMeasurementsMask, compIdx,
                    list[listcount].sampleIdx,
                    weights, ((CvCARTTrainParams*) trainParams)->stumpTrainParams );
            list[listcount].errdrop = intnode[i-1].stump->rerror
                - (list[listcount].stump->lerror + list[listcount].stump->rerror);
            list[listcount].leftflag = 0;
            list[listcount].parent = i-1;
            listcount++;
        }
        else
        {
            cvReleaseMat( &ridx );
        }
        
        if( listcount == 0 ) break;

        /* find the best node to be added to the tree */
        idx = 0;
        maxerrdrop = list[idx].errdrop;
        for( j = 1; j < listcount; j++ )
        {
            if( list[j].errdrop > maxerrdrop )
            {
                idx = j;
                maxerrdrop = list[j].errdrop;
            }
        }
        intnode[i] = list[idx];
        if( list[idx].leftflag )
        {
            cart->left[list[idx].parent] = i;
        }
        else
        {
            cart->right[list[idx].parent] = i;
        }
        if( idx != (listcount - 1) )
        {
            list[idx] = list[listcount - 1];
        }
        listcount--;
    }

    /* fill <cart> fields */
    j = 0;
    cart->count = 0;
    for( i = 0; i < count && (intnode[i].stump != NULL); i++ )
    {
        cart->count++;
        cart->compidx[i] = intnode[i].stump->compidx;
        cart->threshold[i] = intnode[i].stump->threshold;
        
        /* leaves */
        if( cart->left[i] <= 0 )
        {
            cart->left[i] = -j;
            cart->val[j] = intnode[i].stump->left;
            j++;
        }
        if( cart->right[i] <= 0 )
        {
            cart->right[i] = -j;
            cart->val[j] = intnode[i].stump->right;
            j++;
        }
    }
    
    /* CLEAN UP */
    for( i = 0; i < count && (intnode[i].stump != NULL); i++ )
    {
        intnode[i].stump->release( (CvClassifier**) &(intnode[i].stump) );
        if( i != 0 )
        {
            cvReleaseMat( &(intnode[i].sampleIdx) );
        }
    }
    for( i = 0; i < listcount; i++ )
    {
        list[i].stump->release( (CvClassifier**) &(list[i].stump) );
        cvReleaseMat( &(list[i].sampleIdx) );
    }
    
    cvFree( &intnode );

    return (CvClassifier*) cart;
}

/****************************************************************************************\
*                                        Boosting                                        *
\****************************************************************************************/

typedef struct CvBoostTrainer
{
    CvBoostType type;
    int count;             /* (idx) ? number_of_indices : number_of_samples */
    int* idx;
    float* F;
} CvBoostTrainer;

/*
 * cvBoostStartTraining, cvBoostNextWeakClassifier, cvBoostEndTraining
 *
 * These functions perform training of 2-class boosting classifier
 * using ANY appropriate weak classifier
 */

CV_BOOST_IMPL
CvBoostTrainer* icvBoostStartTraining( CvMat* trainClasses,
                                       CvMat* weakTrainVals,
                                       CvMat* weights,
                                       CvMat* sampleIdx,
                                       CvBoostType type )
{
    uchar* ydata;
    int ystep;
    int m;
    uchar* traindata;
    int trainstep;
    int trainnum;
    int i;
    int idx;

    size_t datasize;
    CvBoostTrainer* ptr;

    int idxnum;
    int idxstep;
    uchar* idxdata;

    assert( trainClasses != NULL );
    assert( CV_MAT_TYPE( trainClasses->type ) == CV_32FC1 );
    assert( weakTrainVals != NULL );
    assert( CV_MAT_TYPE( weakTrainVals->type ) == CV_32FC1 );

    CV_MAT2VEC( *trainClasses, ydata, ystep, m );
    CV_MAT2VEC( *weakTrainVals, traindata, trainstep, trainnum );

    assert( m == trainnum );

    idxnum = 0;
    idxstep = 0;
    idxdata = NULL;
    if( sampleIdx )
    {
        CV_MAT2VEC( *sampleIdx, idxdata, idxstep, idxnum );
    }
        
    datasize = sizeof( *ptr ) + sizeof( *ptr->idx ) * idxnum;
    ptr = (CvBoostTrainer*) cvAlloc( datasize );
    memset( ptr, 0, datasize );
    ptr->F = NULL;
    ptr->idx = NULL;

    ptr->count = m;
    ptr->type = type;
    
    if( idxnum > 0 )
    {
        CvScalar s;

        ptr->idx = (int*) (ptr + 1);
        ptr->count = idxnum;
        for( i = 0; i < ptr->count; i++ )
        {
            cvRawDataToScalar( idxdata + i*idxstep, CV_MAT_TYPE( sampleIdx->type ), &s );
            ptr->idx[i] = (int) s.val[0];
        }
    }
    for( i = 0; i < ptr->count; i++ )
    {
        idx = (ptr->idx) ? ptr->idx[i] : i;

        *((float*) (traindata + idx * trainstep)) = 
            2.0F * (*((float*) (ydata + idx * ystep))) - 1.0F;
    }

    return ptr;
}

/*
 *
 * Discrete AdaBoost functions
 *
 */
CV_BOOST_IMPL
float icvBoostNextWeakClassifierDAB( CvMat* weakEvalVals,
                                     CvMat* trainClasses,
                                     CvMat* weakTrainVals,
                                     CvMat* weights,
                                     CvBoostTrainer* trainer )
{
    uchar* evaldata;
    int evalstep;
    int m;
    uchar* ydata;
    int ystep;
    int ynum;
    uchar* wdata;
    int wstep;
    int wnum;

    float sumw;
    float err;
    int i;
    int idx;

    assert( weakEvalVals != NULL );
    assert( CV_MAT_TYPE( weakEvalVals->type ) == CV_32FC1 );
    assert( trainClasses != NULL );
    assert( CV_MAT_TYPE( trainClasses->type ) == CV_32FC1 );
    assert( weights != NULL );
    assert( CV_MAT_TYPE( weights ->type ) == CV_32FC1 );

    CV_MAT2VEC( *weakEvalVals, evaldata, evalstep, m );
    CV_MAT2VEC( *trainClasses, ydata, ystep, ynum );
    CV_MAT2VEC( *weights, wdata, wstep, wnum );

    assert( m == ynum );
    assert( m == wnum );

    sumw = 0.0F;
    err = 0.0F;
    for( i = 0; i < trainer->count; i++ )
    {
        idx = (trainer->idx) ? trainer->idx[i] : i;

        sumw += *((float*) (wdata + idx*wstep));
        err += (*((float*) (wdata + idx*wstep))) *
            ( (*((float*) (evaldata + idx*evalstep))) != 
                2.0F * (*((float*) (ydata + idx*ystep))) - 1.0F );
    }
    err /= sumw;
    err = -cvLogRatio( err );
    
    for( i = 0; i < trainer->count; i++ )
    {
        idx = (trainer->idx) ? trainer->idx[i] : i;

        *((float*) (wdata + idx*wstep)) *= expf( err * 
            ((*((float*) (evaldata + idx*evalstep))) != 
                2.0F * (*((float*) (ydata + idx*ystep))) - 1.0F) );
        sumw += *((float*) (wdata + idx*wstep));
    }
    for( i = 0; i < trainer->count; i++ )
    {
        idx = (trainer->idx) ? trainer->idx[i] : i;

        *((float*) (wdata + idx * wstep)) /= sumw;
    }
    
    return err;
}

/*
 *
 * Real AdaBoost functions
 *
 */
CV_BOOST_IMPL
float icvBoostNextWeakClassifierRAB( CvMat* weakEvalVals,
                                     CvMat* trainClasses,
                                     CvMat* weakTrainVals,
                                     CvMat* weights,
                                     CvBoostTrainer* trainer )
{
    uchar* evaldata;
    int evalstep;
    int m;
    uchar* ydata;
    int ystep;
    int ynum;
    uchar* wdata;
    int wstep;
    int wnum;

    float sumw;
    int i, idx;

    assert( weakEvalVals != NULL );
    assert( CV_MAT_TYPE( weakEvalVals->type ) == CV_32FC1 );
    assert( trainClasses != NULL );
    assert( CV_MAT_TYPE( trainClasses->type ) == CV_32FC1 );
    assert( weights != NULL );
    assert( CV_MAT_TYPE( weights ->type ) == CV_32FC1 );

    CV_MAT2VEC( *weakEvalVals, evaldata, evalstep, m );
    CV_MAT2VEC( *trainClasses, ydata, ystep, ynum );
    CV_MAT2VEC( *weights, wdata, wstep, wnum );

    assert( m == ynum );
    assert( m == wnum );


    sumw = 0.0F;
    for( i = 0; i < trainer->count; i++ )
    {
        idx = (trainer->idx) ? trainer->idx[i] : i;

        *((float*) (wdata + idx*wstep)) *= expf( (-(*((float*) (ydata + idx*ystep))) + 0.5F)
            * cvLogRatio( *((float*) (evaldata + idx*evalstep)) ) );
        sumw += *((float*) (wdata + idx*wstep));
    }
    for( i = 0; i < trainer->count; i++ )
    {
        idx = (trainer->idx) ? trainer->idx[i] : i;

        *((float*) (wdata + idx*wstep)) /= sumw;
    }
    
    return 1.0F;
}

/*
 *
 * LogitBoost functions
 *
 */
#define CV_LB_PROB_THRESH      0.01F
#define CV_LB_WEIGHT_THRESHOLD 0.0001F

CV_BOOST_IMPL
void icvResponsesAndWeightsLB( int num, uchar* wdata, int wstep,
                               uchar* ydata, int ystep,
                               uchar* fdata, int fstep,
                               uchar* traindata, int trainstep,
                               int* indices )
{
    int i, idx;
    float p;

    for( i = 0; i < num; i++ )
    {
        idx = (indices) ? indices[i] : i;

        p = 1.0F / (1.0F + expf( -(*((float*) (fdata + idx*fstep)))) );
        *((float*) (wdata + idx*wstep)) = MAX( p * (1.0F - p), CV_LB_WEIGHT_THRESHOLD );
        if( *((float*) (ydata + idx*ystep)) == 1.0F )
        {
            *((float*) (traindata + idx*trainstep)) = 
                1.0F / (MAX( p, CV_LB_PROB_THRESH ));
        }
        else
        {
            *((float*) (traindata + idx*trainstep)) = 
                -1.0F / (MAX( 1.0F - p, CV_LB_PROB_THRESH ));
        }
    }
}

CV_BOOST_IMPL
CvBoostTrainer* icvBoostStartTrainingLB( CvMat* trainClasses,
                                         CvMat* weakTrainVals,
                                         CvMat* weights,
                                         CvMat* sampleIdx,
                                         CvBoostType type )
{
    size_t datasize;
    CvBoostTrainer* ptr;

    uchar* ydata;
    int ystep;
    int m;
    uchar* traindata;
    int trainstep;
    int trainnum;
    uchar* wdata;
    int wstep;
    int wnum;
    int i;

    int idxnum;
    int idxstep;
    uchar* idxdata;

    assert( trainClasses != NULL );
    assert( CV_MAT_TYPE( trainClasses->type ) == CV_32FC1 );
    assert( weakTrainVals != NULL );
    assert( CV_MAT_TYPE( weakTrainVals->type ) == CV_32FC1 );
    assert( weights != NULL );
    assert( CV_MAT_TYPE( weights->type ) == CV_32FC1 );

    CV_MAT2VEC( *trainClasses, ydata, ystep, m );
    CV_MAT2VEC( *weakTrainVals, traindata, trainstep, trainnum );
    CV_MAT2VEC( *weights, wdata, wstep, wnum );

    assert( m == trainnum );
    assert( m == wnum );


    idxnum = 0;
    idxstep = 0;
    idxdata = NULL;
    if( sampleIdx )
    {
        CV_MAT2VEC( *sampleIdx, idxdata, idxstep, idxnum );
    }
        
    datasize = sizeof( *ptr ) + sizeof( *ptr->F ) * m + sizeof( *ptr->idx ) * idxnum;
    ptr = (CvBoostTrainer*) cvAlloc( datasize );
    memset( ptr, 0, datasize );
    ptr->F = (float*) (ptr + 1);
    ptr->idx = NULL;

    ptr->count = m;
    ptr->type = type;
    
    if( idxnum > 0 )
    {
        CvScalar s;

        ptr->idx = (int*) (ptr->F + m);
        ptr->count = idxnum;
        for( i = 0; i < ptr->count; i++ )
        {
            cvRawDataToScalar( idxdata + i*idxstep, CV_MAT_TYPE( sampleIdx->type ), &s );
            ptr->idx[i] = (int) s.val[0];
        }
    }

    for( i = 0; i < m; i++ )
    {
        ptr->F[i] = 0.0F;
    }

    icvResponsesAndWeightsLB( ptr->count, wdata, wstep, ydata, ystep,
                              (uchar*) ptr->F, sizeof( *ptr->F ),
                              traindata, trainstep, ptr->idx );

    return ptr;
}

CV_BOOST_IMPL
float icvBoostNextWeakClassifierLB( CvMat* weakEvalVals,
                                    CvMat* trainClasses,
                                    CvMat* weakTrainVals,
                                    CvMat* weights,
                                    CvBoostTrainer* trainer )
{
    uchar* evaldata;
    int evalstep;
    int m;
    uchar* ydata;
    int ystep;
    int ynum;
    uchar* traindata;
    int trainstep;
    int trainnum;
    uchar* wdata;
    int wstep;
    int wnum;
    int i, idx;

    assert( weakEvalVals != NULL );
    assert( CV_MAT_TYPE( weakEvalVals->type ) == CV_32FC1 );
    assert( trainClasses != NULL );
    assert( CV_MAT_TYPE( trainClasses->type ) == CV_32FC1 );
    assert( weakTrainVals != NULL );
    assert( CV_MAT_TYPE( weakTrainVals->type ) == CV_32FC1 );
    assert( weights != NULL );
    assert( CV_MAT_TYPE( weights ->type ) == CV_32FC1 );

    CV_MAT2VEC( *weakEvalVals, evaldata, evalstep, m );
    CV_MAT2VEC( *trainClasses, ydata, ystep, ynum );
    CV_MAT2VEC( *weakTrainVals, traindata, trainstep, trainnum );
    CV_MAT2VEC( *weights, wdata, wstep, wnum );

    assert( m == ynum );
    assert( m == wnum );
    assert( m == trainnum );
    //assert( m == trainer->count );

    for( i = 0; i < trainer->count; i++ )
    {
        idx = (trainer->idx) ? trainer->idx[i] : i;

        trainer->F[idx] += *((float*) (evaldata + idx * evalstep));
    }
    
    icvResponsesAndWeightsLB( trainer->count, wdata, wstep, ydata, ystep,
                              (uchar*) trainer->F, sizeof( *trainer->F ),
                              traindata, trainstep, trainer->idx );

    return 1.0F;
}

/*
 *
 * Gentle AdaBoost
 *
 */
CV_BOOST_IMPL
float icvBoostNextWeakClassifierGAB( CvMat* weakEvalVals,
                                     CvMat* trainClasses,
                                     CvMat* weakTrainVals,
                                     CvMat* weights,
                                     CvBoostTrainer* trainer )
{
    uchar* evaldata;
    int evalstep;
    int m;
    uchar* ydata;
    int ystep;
    int ynum;
    uchar* wdata;
    int wstep;
    int wnum;

    int i, idx;
    float sumw;

    assert( weakEvalVals != NULL );
    assert( CV_MAT_TYPE( weakEvalVals->type ) == CV_32FC1 );
    assert( trainClasses != NULL );
    assert( CV_MAT_TYPE( trainClasses->type ) == CV_32FC1 );
    assert( weights != NULL );
    assert( CV_MAT_TYPE( weights->type ) == CV_32FC1 );

    CV_MAT2VEC( *weakEvalVals, evaldata, evalstep, m );
    CV_MAT2VEC( *trainClasses, ydata, ystep, ynum );
    CV_MAT2VEC( *weights, wdata, wstep, wnum );

    assert( m == ynum );
    assert( m == wnum );

    sumw = 0.0F;
    for( i = 0; i < trainer->count; i++ )
    {
        idx = (trainer->idx) ? trainer->idx[i] : i;

        *((float*) (wdata + idx*wstep)) *= 
            expf( -(*((float*) (evaldata + idx*evalstep)))
                  * ( 2.0F * (*((float*) (ydata + idx*ystep))) - 1.0F ) );
        sumw += *((float*) (wdata + idx*wstep));
    }
    
    for( i = 0; i < trainer->count; i++ )
    {
        idx = (trainer->idx) ? trainer->idx[i] : i;

        *((float*) (wdata + idx*wstep)) /= sumw;
    }

    return 1.0F;
}

typedef CvBoostTrainer* (*CvBoostStartTraining)( CvMat* trainClasses,
                                                 CvMat* weakTrainVals,
                                                 CvMat* weights,
                                                 CvMat* sampleIdx,
                                                 CvBoostType type );

typedef float (*CvBoostNextWeakClassifier)( CvMat* weakEvalVals,
                                            CvMat* trainClasses,
                                            CvMat* weakTrainVals,
                                            CvMat* weights,
                                            CvBoostTrainer* data );

CvBoostStartTraining startTraining[4] = {
        icvBoostStartTraining,
        icvBoostStartTraining,
        icvBoostStartTrainingLB,
        icvBoostStartTraining
    };

CvBoostNextWeakClassifier nextWeakClassifier[4] = {
        icvBoostNextWeakClassifierDAB,
        icvBoostNextWeakClassifierRAB,
        icvBoostNextWeakClassifierLB,
        icvBoostNextWeakClassifierGAB
    };

/*
 *
 * Dispatchers
 *
 */
CV_BOOST_IMPL
CvBoostTrainer* cvBoostStartTraining( CvMat* trainClasses,
                                      CvMat* weakTrainVals,
                                      CvMat* weights,
                                      CvMat* sampleIdx,
                                      CvBoostType type )
{
    return startTraining[type]( trainClasses, weakTrainVals, weights, sampleIdx, type );
}

CV_BOOST_IMPL
void cvBoostEndTraining( CvBoostTrainer** trainer )
{
    cvFree( trainer );
    *trainer = NULL;
}

CV_BOOST_IMPL
float cvBoostNextWeakClassifier( CvMat* weakEvalVals,
                                 CvMat* trainClasses,
                                 CvMat* weakTrainVals,
                                 CvMat* weights,
                                 CvBoostTrainer* trainer )
{
    return nextWeakClassifier[trainer->type]( weakEvalVals, trainClasses,
        weakTrainVals, weights, trainer    );
}

/****************************************************************************************\
*                                    Boosted tree models                                 *
\****************************************************************************************/

typedef struct CvBtTrainer
{
    /* {{ external */    
    CvMat* trainData;
    int flags;
    
    CvMat* trainClasses;
    int m;
    uchar* ydata;
    int ystep;

    CvMat* sampleIdx;
    int numsamples;
    
    float param[2];
    CvBoostType type;
    int numclasses;
    /* }} external */

    CvMTStumpTrainParams stumpParams;
    CvCARTTrainParams  cartParams;

    float* f;          /* F_(m-1) */
    CvMat* y;          /* yhat    */
    CvMat* weights;
    CvBoostTrainer* boosttrainer;
} CvBtTrainer;

/*
 * cvBtStart, cvBtNext, cvBtEnd
 *
 * These functions perform iterative training of
 * 2-class (CV_DABCLASS - CV_GABCLASS, CV_L2CLASS), K-class (CV_LKCLASS) classifier
 * or fit regression model (CV_LSREG, CV_LADREG, CV_MREG)
 * using decision tree as a weak classifier.
 */

typedef void (*CvZeroApproxFunc)( float* approx, CvBtTrainer* trainer );

/* Mean zero approximation */
void icvZeroApproxMean( float* approx, CvBtTrainer* trainer )
{
    int i;
    int idx;

    approx[0] = 0.0F;
    for( i = 0; i < trainer->numsamples; i++ )
    {
        idx = icvGetIdxAt( trainer->sampleIdx, i );
        approx[0] += *((float*) (trainer->ydata + idx * trainer->ystep));
    }
    approx[0] /= (float) trainer->numsamples;
}

/*
 * Median zero approximation
 */
void icvZeroApproxMed( float* approx, CvBtTrainer* trainer )
{
    int i;
    int idx;

    for( i = 0; i < trainer->numsamples; i++ )
    {
        idx = icvGetIdxAt( trainer->sampleIdx, i );
        trainer->f[i] = *((float*) (trainer->ydata + idx * trainer->ystep));
    }
    
    icvSort_32f( trainer->f, trainer->numsamples, 0 );
    approx[0] = trainer->f[trainer->numsamples / 2];
}

/*
 * 0.5 * log( mean(y) / (1 - mean(y)) ) where y in {0, 1}
 */
void icvZeroApproxLog( float* approx, CvBtTrainer* trainer )
{
    float y_mean;

    icvZeroApproxMean( &y_mean, trainer );
    approx[0] = 0.5F * cvLogRatio( y_mean );
}

/*
 * 0 zero approximation
 */
void icvZeroApprox0( float* approx, CvBtTrainer* trainer )
{
    int i;

    for( i = 0; i < trainer->numclasses; i++ )
    {
        approx[i] = 0.0F;
    }
}

static CvZeroApproxFunc icvZeroApproxFunc[] =
{
    icvZeroApprox0,    /* CV_DABCLASS */
    icvZeroApprox0,    /* CV_RABCLASS */
    icvZeroApprox0,    /* CV_LBCLASS  */
    icvZeroApprox0,    /* CV_GABCLASS */
    icvZeroApproxLog,  /* CV_L2CLASS  */
    icvZeroApprox0,    /* CV_LKCLASS  */
    icvZeroApproxMean, /* CV_LSREG    */
    icvZeroApproxMed,  /* CV_LADREG   */
    icvZeroApproxMed,  /* CV_MREG     */
};

CV_BOOST_IMPL
void cvBtNext( CvCARTClassifier** trees, CvBtTrainer* trainer );

CV_BOOST_IMPL
CvBtTrainer* cvBtStart( CvCARTClassifier** trees,
                        CvMat* trainData,
                        int flags,
                        CvMat* trainClasses,
                        CvMat* sampleIdx,
                        int numsplits,
                        CvBoostType type,
                        int numclasses,
                        float* param )
{
    CvBtTrainer* ptr;

    CV_FUNCNAME( "cvBtStart" );

    __BEGIN__;

    size_t data_size;
    float* zero_approx;
    int m;
    int i, j;
    
    if( trees == NULL )
    {
        CV_ERROR( CV_StsNullPtr, "Invalid trees parameter" );
    }
    
    if( type < CV_DABCLASS || type > CV_MREG ) 
    {
        CV_ERROR( CV_StsUnsupportedFormat, "Unsupported type parameter" );
    }
    if( type == CV_LKCLASS )
    {
        CV_ASSERT( numclasses >= 2 );
    }
    else
    {
        numclasses = 1;
    }

    m = MAX( trainClasses->rows, trainClasses->cols );
    ptr = NULL;
    data_size = sizeof( *ptr );
    if( type > CV_GABCLASS )
    {
        data_size += m * numclasses * sizeof( *(ptr->f) );
    }
    CV_CALL( ptr = (CvBtTrainer*) cvAlloc( data_size ) );
    memset( ptr, 0, data_size );
    ptr->f = (float*) (ptr + 1);

    ptr->trainData = trainData;
    ptr->flags = flags;
    ptr->trainClasses = trainClasses;
    CV_MAT2VEC( *trainClasses, ptr->ydata, ptr->ystep, ptr->m );
    
    memset( &(ptr->cartParams), 0, sizeof( ptr->cartParams ) );
    memset( &(ptr->stumpParams), 0, sizeof( ptr->stumpParams ) );

    switch( type )
    {
        case CV_DABCLASS:
            ptr->stumpParams.error = CV_MISCLASSIFICATION;
            ptr->stumpParams.type  = CV_CLASSIFICATION_CLASS;
            break;
        case CV_RABCLASS:
            ptr->stumpParams.error = CV_GINI;
            ptr->stumpParams.type  = CV_CLASSIFICATION;
            break;
        default:
            ptr->stumpParams.error = CV_SQUARE;
            ptr->stumpParams.type  = CV_REGRESSION;
    }
    ptr->cartParams.count = numsplits;
    ptr->cartParams.stumpTrainParams = (CvClassifierTrainParams*) &(ptr->stumpParams);
    ptr->cartParams.stumpConstructor = cvCreateMTStumpClassifier;

    ptr->param[0] = param[0];
    ptr->param[1] = param[1];
    ptr->type = type;
    ptr->numclasses = numclasses;

    CV_CALL( ptr->y = cvCreateMat( 1, m, CV_32FC1 ) );
    ptr->sampleIdx = sampleIdx;
    ptr->numsamples = ( sampleIdx == NULL ) ? ptr->m
                             : MAX( sampleIdx->rows, sampleIdx->cols );
    
    ptr->weights = cvCreateMat( 1, m, CV_32FC1 );
    cvSet( ptr->weights, cvScalar( 1.0 ) );    
    
    if( type <= CV_GABCLASS )
    {
        ptr->boosttrainer = cvBoostStartTraining( ptr->trainClasses, ptr->y,
            ptr->weights, NULL, type );

        CV_CALL( cvBtNext( trees, ptr ) );
    }
    else
    {
        data_size = sizeof( *zero_approx ) * numclasses;
        CV_CALL( zero_approx = (float*) cvAlloc( data_size ) );
        icvZeroApproxFunc[type]( zero_approx, ptr );
        for( i = 0; i < m; i++ )
        {
            for( j = 0; j < numclasses; j++ )
            {
                ptr->f[i * numclasses + j] = zero_approx[j];
            }
        }

        CV_CALL( cvBtNext( trees, ptr ) );

        for( i = 0; i < numclasses; i++ )
        {
            for( j = 0; j <= trees[i]->count; j++ )
            {
                trees[i]->val[j] += zero_approx[i];
            }
        }    
        CV_CALL( cvFree( &zero_approx ) );
    }

    __END__;

    return ptr;
}

void icvBtNext_LSREG( CvCARTClassifier** trees, CvBtTrainer* trainer )
{
    int i;

    /* yhat_i = y_i - F_(m-1)(x_i) */
    for( i = 0; i < trainer->m; i++ )
    {
        trainer->y->data.fl[i] = 
            *((float*) (trainer->ydata + i * trainer->ystep)) - trainer->f[i];
    }

    trees[0] = (CvCARTClassifier*) cvCreateCARTClassifier( trainer->trainData,
        trainer->flags,
        trainer->y, NULL, NULL, NULL, trainer->sampleIdx, trainer->weights,
        (CvClassifierTrainParams*) &trainer->cartParams );
}


void icvBtNext_LADREG( CvCARTClassifier** trees, CvBtTrainer* trainer )
{
    CvCARTClassifier* ptr;
    int i, j;
    CvMat sample;
    int sample_step;
    uchar* sample_data;
    int index;
    
    int data_size;
    int* idx;
    float* resp;
    int respnum;
    float val;

    data_size = trainer->m * sizeof( *idx );
    idx = (int*) cvAlloc( data_size );
    data_size = trainer->m * sizeof( *resp );
    resp = (float*) cvAlloc( data_size );

    /* yhat_i = sign(y_i - F_(m-1)(x_i)) */
    for( i = 0; i < trainer->numsamples; i++ )
    {
        index = icvGetIdxAt( trainer->sampleIdx, i );
        trainer->y->data.fl[index] = (float)
             CV_SIGN( *((float*) (trainer->ydata + index * trainer->ystep))
                     - trainer->f[index] );
    }

    ptr = (CvCARTClassifier*) cvCreateCARTClassifier( trainer->trainData, trainer->flags,
        trainer->y, NULL, NULL, NULL, trainer->sampleIdx, trainer->weights,
        (CvClassifierTrainParams*) &trainer->cartParams );

    CV_GET_SAMPLE( *trainer->trainData, trainer->flags, 0, sample );
    CV_GET_SAMPLE_STEP( *trainer->trainData, trainer->flags, sample_step );
    sample_data = sample.data.ptr;
    for( i = 0; i < trainer->numsamples; i++ )
    {
        index = icvGetIdxAt( trainer->sampleIdx, i );
        sample.data.ptr = sample_data + index * sample_step;
        idx[index] = (int) cvEvalCARTClassifierIdx( (CvClassifier*) ptr, &sample );
    }
    for( j = 0; j <= ptr->count; j++ )
    {
        respnum = 0;
        for( i = 0; i < trainer->numsamples; i++ )
        {
            index = icvGetIdxAt( trainer->sampleIdx, i );
            if( idx[index] == j )
            {
                resp[respnum++] = *((float*) (trainer->ydata + index * trainer->ystep))
                                  - trainer->f[index];
            }
        }
        if( respnum > 0 )
        {
            icvSort_32f( resp, respnum, 0 );
            val = resp[respnum / 2];
        }
        else
        {
            val = 0.0F;
        }
        ptr->val[j] = val;
    }

    cvFree( &idx );
    cvFree( &resp );
    
    trees[0] = ptr;
}


void icvBtNext_MREG( CvCARTClassifier** trees, CvBtTrainer* trainer )
{
    CvCARTClassifier* ptr;
    int i, j;
    CvMat sample;
    int sample_step;
    uchar* sample_data;
    
    int data_size;
    int* idx;
    float* resid;
    float* resp;
    int respnum;
    float rhat;
    float val;
    float delta;
    int index;

    data_size = trainer->m * sizeof( *idx );
    idx = (int*) cvAlloc( data_size );
    data_size = trainer->m * sizeof( *resp );
    resp = (float*) cvAlloc( data_size );
    data_size = trainer->m * sizeof( *resid );
    resid = (float*) cvAlloc( data_size );

    /* resid_i = (y_i - F_(m-1)(x_i)) */
    for( i = 0; i < trainer->numsamples; i++ )
    {
        index = icvGetIdxAt( trainer->sampleIdx, i );
        resid[index] = *((float*) (trainer->ydata + index * trainer->ystep))
                       - trainer->f[index];
        /* for delta */
        resp[i] = (float) fabs( resid[index] );
    }
    
    /* delta = quantile_alpha{abs(resid_i)} */
    icvSort_32f( resp, trainer->numsamples, 0 );
    delta = resp[(int)(trainer->param[1] * (trainer->numsamples - 1))];

    /* yhat_i */
    for( i = 0; i < trainer->numsamples; i++ )
    {
        index = icvGetIdxAt( trainer->sampleIdx, i );
        trainer->y->data.fl[index] = MIN( delta, ((float) fabs( resid[index] )) ) *
                                 CV_SIGN( resid[index] );
    }
    
    ptr = (CvCARTClassifier*) cvCreateCARTClassifier( trainer->trainData, trainer->flags,
        trainer->y, NULL, NULL, NULL, trainer->sampleIdx, trainer->weights,
        (CvClassifierTrainParams*) &trainer->cartParams );

    CV_GET_SAMPLE( *trainer->trainData, trainer->flags, 0, sample );
    CV_GET_SAMPLE_STEP( *trainer->trainData, trainer->flags, sample_step );
    sample_data = sample.data.ptr;
    for( i = 0; i < trainer->numsamples; i++ )
    {
        index = icvGetIdxAt( trainer->sampleIdx, i );
        sample.data.ptr = sample_data + index * sample_step;
        idx[index] = (int) cvEvalCARTClassifierIdx( (CvClassifier*) ptr, &sample );
    }
    for( j = 0; j <= ptr->count; j++ )
    {
        respnum = 0;

        for( i = 0; i < trainer->numsamples; i++ )
        {
            index = icvGetIdxAt( trainer->sampleIdx, i );
            if( idx[index] == j )
            {
                resp[respnum++] = *((float*) (trainer->ydata + index * trainer->ystep))
                                  - trainer->f[index];
            }
        }
        if( respnum > 0 )
        {
            /* rhat = median(y_i - F_(m-1)(x_i)) */
            icvSort_32f( resp, respnum, 0 );
            rhat = resp[respnum / 2];
            
            /* val = sum{sign(r_i - rhat_i) * min(delta, abs(r_i - rhat_i)}
             * r_i = y_i - F_(m-1)(x_i)
             */
            val = 0.0F;
            for( i = 0; i < respnum; i++ )
            {
                val += CV_SIGN( resp[i] - rhat )
                       * MIN( delta, (float) fabs( resp[i] - rhat ) );
            }

            val = rhat + val / (float) respnum;
        }
        else
        {
            val = 0.0F;
        }

        ptr->val[j] = val;

    }

    cvFree( &resid );
    cvFree( &resp );
    cvFree( &idx );
    
    trees[0] = ptr;
}

//#define CV_VAL_MAX 1e304

//#define CV_LOG_VAL_MAX 700.0

#define CV_VAL_MAX 1e+8

#define CV_LOG_VAL_MAX 18.0

void icvBtNext_L2CLASS( CvCARTClassifier** trees, CvBtTrainer* trainer )
{
    CvCARTClassifier* ptr;
    int i, j;
    CvMat sample;
    int sample_step;
    uchar* sample_data;
    
    int data_size;
    int* idx;
    int respnum;
    float val;
    double val_f;

    float sum_weights;
    float* weights;
    float* sorted_weights;
    CvMat* trimmed_idx;
    CvMat* sample_idx;
    int index;
    int trimmed_num;

    data_size = trainer->m * sizeof( *idx );
    idx = (int*) cvAlloc( data_size );

    data_size = trainer->m * sizeof( *weights );
    weights = (float*) cvAlloc( data_size );
    data_size = trainer->m * sizeof( *sorted_weights );
    sorted_weights = (float*) cvAlloc( data_size );
    
    /* yhat_i = (4 * y_i - 2) / ( 1 + exp( (4 * y_i - 2) * F_(m-1)(x_i) ) ).
     *   y_i in {0, 1}
     */
    sum_weights = 0.0F;
    for( i = 0; i < trainer->numsamples; i++ )
    {
        index = icvGetIdxAt( trainer->sampleIdx, i );
        val = 4.0F * (*((float*) (trainer->ydata + index * trainer->ystep))) - 2.0F;
        val_f = val * trainer->f[index];
        val_f = ( val_f < CV_LOG_VAL_MAX ) ? exp( val_f ) : CV_LOG_VAL_MAX;
        val = (float) ( (double) val / ( 1.0 + val_f ) );
        trainer->y->data.fl[index] = val;
        val = (float) fabs( val );
        weights[index] = val * (2.0F - val);
        sorted_weights[i] = weights[index];
        sum_weights += sorted_weights[i];
    }
    
    trimmed_idx = NULL;
    sample_idx = trainer->sampleIdx;
    trimmed_num = trainer->numsamples;
    if( trainer->param[1] < 1.0F )
    {
        /* perform weight trimming */
        
        float threshold;
        int count;
        
        icvSort_32f( sorted_weights, trainer->numsamples, 0 );

        sum_weights *= (1.0F - trainer->param[1]);
        
        i = -1;
        do { sum_weights -= sorted_weights[++i]; }
        while( sum_weights > 0.0F && i < (trainer->numsamples - 1) );
        
        threshold = sorted_weights[i];

        while( i > 0 && sorted_weights[i-1] == threshold ) i--;

        if( i > 0 )
        {
            trimmed_num = trainer->numsamples - i;            
            trimmed_idx = cvCreateMat( 1, trimmed_num, CV_32FC1 );
            count = 0;
            for( i = 0; i < trainer->numsamples; i++ )
            {
                index = icvGetIdxAt( trainer->sampleIdx, i );
                if( weights[index] >= threshold )
                {
                    CV_MAT_ELEM( *trimmed_idx, float, 0, count ) = (float) index;
                    count++;
                }
            }
            
            assert( count == trimmed_num );

            sample_idx = trimmed_idx;

            printf( "Used samples %%: %g\n", 
                (float) trimmed_num / (float) trainer->numsamples * 100.0F );
        }
    }

    ptr = (CvCARTClassifier*) cvCreateCARTClassifier( trainer->trainData, trainer->flags,
        trainer->y, NULL, NULL, NULL, sample_idx, trainer->weights,
        (CvClassifierTrainParams*) &trainer->cartParams );

    CV_GET_SAMPLE( *trainer->trainData, trainer->flags, 0, sample );
    CV_GET_SAMPLE_STEP( *trainer->trainData, trainer->flags, sample_step );
    sample_data = sample.data.ptr;
    for( i = 0; i < trimmed_num; i++ )
    {
        index = icvGetIdxAt( sample_idx, i );
        sample.data.ptr = sample_data + index * sample_step;
        idx[index] = (int) cvEvalCARTClassifierIdx( (CvClassifier*) ptr, &sample );
    }
    for( j = 0; j <= ptr->count; j++ )
    {
        respnum = 0;
        val = 0.0F;
        sum_weights = 0.0F;
        for( i = 0; i < trimmed_num; i++ )
        {
            index = icvGetIdxAt( sample_idx, i );
            if( idx[index] == j )
            {
                val += trainer->y->data.fl[index];
                sum_weights += weights[index];
                respnum++;
            }
        }
        if( sum_weights > 0.0F )
        {
            val /= sum_weights;
        }
        else
        {
            val = 0.0F;
        }
        ptr->val[j] = val;
    }
    
    if( trimmed_idx != NULL ) cvReleaseMat( &trimmed_idx );
    cvFree( &sorted_weights );
    cvFree( &weights );
    cvFree( &idx );
    
    trees[0] = ptr;
}

void icvBtNext_LKCLASS( CvCARTClassifier** trees, CvBtTrainer* trainer )
{
    int i, j, k, kk, num;
    CvMat sample;
    int sample_step;
    uchar* sample_data;
    
    int data_size;
    int* idx;
    int respnum;
    float val;

    float sum_weights;
    float* weights;
    float* sorted_weights;
    CvMat* trimmed_idx;
    CvMat* sample_idx;
    int index;
    int trimmed_num;
    double sum_exp_f;
    double exp_f;
    double f_k;

    data_size = trainer->m * sizeof( *idx );
    idx = (int*) cvAlloc( data_size );
    data_size = trainer->m * sizeof( *weights );
    weights = (float*) cvAlloc( data_size );
    data_size = trainer->m * sizeof( *sorted_weights );
    sorted_weights = (float*) cvAlloc( data_size );
    trimmed_idx = cvCreateMat( 1, trainer->numsamples, CV_32FC1 );

    for( k = 0; k < trainer->numclasses; k++ )
    {
        /* yhat_i = y_i - p_k(x_i), y_i in {0, 1}      */
        /* p_k(x_i) = exp(f_k(x_i)) / (sum_exp_f(x_i)) */
        sum_weights = 0.0F;
        for( i = 0; i < trainer->numsamples; i++ )
        {
            index = icvGetIdxAt( trainer->sampleIdx, i );
            /* p_k(x_i) = 1 / (1 + sum(exp(f_kk(x_i) - f_k(x_i)))), kk != k */
            num = index * trainer->numclasses;
            f_k = (double) trainer->f[num + k];
            sum_exp_f = 1.0;
            for( kk = 0; kk < trainer->numclasses; kk++ )
            {
                if( kk == k ) continue;
                exp_f = (double) trainer->f[num + kk] - f_k;
                exp_f = (exp_f < CV_LOG_VAL_MAX) ? exp( exp_f ) : CV_VAL_MAX;
                if( exp_f == CV_VAL_MAX || exp_f >= (CV_VAL_MAX - sum_exp_f) )
                {
                    sum_exp_f = CV_VAL_MAX;
                    break;
                }
                sum_exp_f += exp_f;
            }

            val = (float) ( (*((float*) (trainer->ydata + index * trainer->ystep))) 
                            == (float) k );
            val -= (float) ( (sum_exp_f == CV_VAL_MAX) ? 0.0 : ( 1.0 / sum_exp_f ) );

            assert( val >= -1.0F );
            assert( val <= 1.0F );

            trainer->y->data.fl[index] = val;
            val = (float) fabs( val );
            weights[index] = val * (1.0F - val);
            sorted_weights[i] = weights[index];
            sum_weights += sorted_weights[i];
        }

        sample_idx = trainer->sampleIdx;
        trimmed_num = trainer->numsamples;
        if( trainer->param[1] < 1.0F )
        {
            /* perform weight trimming */
        
            float threshold;
            int count;
        
            icvSort_32f( sorted_weights, trainer->numsamples, 0 );

            sum_weights *= (1.0F - trainer->param[1]);
        
            i = -1;
            do { sum_weights -= sorted_weights[++i]; }
            while( sum_weights > 0.0F && i < (trainer->numsamples - 1) );
        
            threshold = sorted_weights[i];

            while( i > 0 && sorted_weights[i-1] == threshold ) i--;

            if( i > 0 )
            {
                trimmed_num = trainer->numsamples - i;            
                trimmed_idx->cols = trimmed_num;
                count = 0;
                for( i = 0; i < trainer->numsamples; i++ )
                {
                    index = icvGetIdxAt( trainer->sampleIdx, i );
                    if( weights[index] >= threshold )
                    {
                        CV_MAT_ELEM( *trimmed_idx, float, 0, count ) = (float) index;
                        count++;
                    }
                }
            
                assert( count == trimmed_num );

                sample_idx = trimmed_idx;

                printf( "k: %d Used samples %%: %g\n", k, 
                    (float) trimmed_num / (float) trainer->numsamples * 100.0F );
            }
        } /* weight trimming */

        trees[k] = (CvCARTClassifier*) cvCreateCARTClassifier( trainer->trainData,
            trainer->flags, trainer->y, NULL, NULL, NULL, sample_idx, trainer->weights,
            (CvClassifierTrainParams*) &trainer->cartParams );

        CV_GET_SAMPLE( *trainer->trainData, trainer->flags, 0, sample );
        CV_GET_SAMPLE_STEP( *trainer->trainData, trainer->flags, sample_step );
        sample_data = sample.data.ptr;
        for( i = 0; i < trimmed_num; i++ )
        {
            index = icvGetIdxAt( sample_idx, i );
            sample.data.ptr = sample_data + index * sample_step;
            idx[index] = (int) cvEvalCARTClassifierIdx( (CvClassifier*) trees[k],
                                                        &sample );
        }
        for( j = 0; j <= trees[k]->count; j++ )
        {
            respnum = 0;
            val = 0.0F;
            sum_weights = 0.0F;
            for( i = 0; i < trimmed_num; i++ )
            {
                index = icvGetIdxAt( sample_idx, i );
                if( idx[index] == j )
                {
                    val += trainer->y->data.fl[index];
                    sum_weights += weights[index];
                    respnum++;
                }
            }
            if( sum_weights > 0.0F )
            {
                val = ((float) (trainer->numclasses - 1)) * val /
                      ((float) (trainer->numclasses)) / sum_weights;
            }
            else
            {
                val = 0.0F;
            }
            trees[k]->val[j] = val;
        }
    } /* for each class */
    
    cvReleaseMat( &trimmed_idx );
    cvFree( &sorted_weights );
    cvFree( &weights );
    cvFree( &idx );
}


void icvBtNext_XXBCLASS( CvCARTClassifier** trees, CvBtTrainer* trainer )
{
    float alpha;
    int i;
    CvMat* weak_eval_vals;
    CvMat* sample_idx;
    int num_samples;
    CvMat sample;
    uchar* sample_data;
    int sample_step;

    weak_eval_vals = cvCreateMat( 1, trainer->m, CV_32FC1 );

    sample_idx = cvTrimWeights( trainer->weights, trainer->sampleIdx,
                                trainer->param[1] );
    num_samples = ( sample_idx == NULL )
        ? trainer->m : MAX( sample_idx->rows, sample_idx->cols );

    printf( "Used samples %%: %g\n", 
        (float) num_samples / (float) trainer->numsamples * 100.0F );

    trees[0] = (CvCARTClassifier*) cvCreateCARTClassifier( trainer->trainData,
        trainer->flags, trainer->y, NULL, NULL, NULL,
        sample_idx, trainer->weights,
        (CvClassifierTrainParams*) &trainer->cartParams );
    
    /* evaluate samples */
    CV_GET_SAMPLE( *trainer->trainData, trainer->flags, 0, sample );
    CV_GET_SAMPLE_STEP( *trainer->trainData, trainer->flags, sample_step );
    sample_data = sample.data.ptr;
    
    for( i = 0; i < trainer->m; i++ )
    {
        sample.data.ptr = sample_data + i * sample_step;
        weak_eval_vals->data.fl[i] = trees[0]->eval( (CvClassifier*) trees[0], &sample );
    }

    alpha = cvBoostNextWeakClassifier( weak_eval_vals, trainer->trainClasses,
        trainer->y, trainer->weights, trainer->boosttrainer );
    
    /* multiply tree by alpha */
    for( i = 0; i <= trees[0]->count; i++ )
    {
        trees[0]->val[i] *= alpha;
    }
    if( trainer->type == CV_RABCLASS )
    {
        for( i = 0; i <= trees[0]->count; i++ )
        {
            trees[0]->val[i] = cvLogRatio( trees[0]->val[i] );
        }
    }
    
    if( sample_idx != NULL && sample_idx != trainer->sampleIdx )
    {
        cvReleaseMat( &sample_idx );
    }
    cvReleaseMat( &weak_eval_vals );
}

typedef void (*CvBtNextFunc)( CvCARTClassifier** trees, CvBtTrainer* trainer );

static CvBtNextFunc icvBtNextFunc[] =
{
    icvBtNext_XXBCLASS,
    icvBtNext_XXBCLASS,
    icvBtNext_XXBCLASS,
    icvBtNext_XXBCLASS,
    icvBtNext_L2CLASS,
    icvBtNext_LKCLASS,
    icvBtNext_LSREG,
    icvBtNext_LADREG,
    icvBtNext_MREG
};

CV_BOOST_IMPL
void cvBtNext( CvCARTClassifier** trees, CvBtTrainer* trainer )
{

    CV_FUNCNAME( "cvBtNext" );

    __BEGIN__;

    int i, j;
    int index;
    CvMat sample;
    int sample_step;
    uchar* sample_data;

    icvBtNextFunc[trainer->type]( trees, trainer );        

    /* shrinkage */
    if( trainer->param[0] != 1.0F )
    {
        for( j = 0; j < trainer->numclasses; j++ )
        {
            for( i = 0; i <= trees[j]->count; i++ )
            {
                trees[j]->val[i] *= trainer->param[0];
            }
        }
    }

    if( trainer->type > CV_GABCLASS )
    {
        /* update F_(m-1) */
        CV_GET_SAMPLE( *(trainer->trainData), trainer->flags, 0, sample );
        CV_GET_SAMPLE_STEP( *(trainer->trainData), trainer->flags, sample_step );
        sample_data = sample.data.ptr;
        for( i = 0; i < trainer->numsamples; i++ )
        {
            index = icvGetIdxAt( trainer->sampleIdx, i );
            sample.data.ptr = sample_data + index * sample_step;
            for( j = 0; j < trainer->numclasses; j++ )
            {            
                trainer->f[index * trainer->numclasses + j] += 
                    trees[j]->eval( (CvClassifier*) (trees[j]), &sample );
            }
        }
    }
    
    __END__;
}

CV_BOOST_IMPL
void cvBtEnd( CvBtTrainer** trainer )
{
    CV_FUNCNAME( "cvBtEnd" );
    
    __BEGIN__;
    
    if( trainer == NULL || (*trainer) == NULL )
    {
        CV_ERROR( CV_StsNullPtr, "Invalid trainer parameter" );
    }
    
    if( (*trainer)->y != NULL )
    {
        CV_CALL( cvReleaseMat( &((*trainer)->y) ) );
    }
    if( (*trainer)->weights != NULL )
    {
        CV_CALL( cvReleaseMat( &((*trainer)->weights) ) );
    }
    if( (*trainer)->boosttrainer != NULL )
    {
        CV_CALL( cvBoostEndTraining( &((*trainer)->boosttrainer) ) );
    }
    CV_CALL( cvFree( trainer ) );

    __END__;
}

/****************************************************************************************\
*                         Boosted tree model as a classifier                             *
\****************************************************************************************/

CV_BOOST_IMPL
float cvEvalBtClassifier( CvClassifier* classifier, CvMat* sample )
{
    float val;

    CV_FUNCNAME( "cvEvalBtClassifier" );

    __BEGIN__;
    
    int i;

    val = 0.0F;
    if( CV_IS_TUNABLE( classifier->flags ) )
    {
        CvSeqReader reader;
        CvCARTClassifier* tree;

        CV_CALL( cvStartReadSeq( ((CvBtClassifier*) classifier)->seq, &reader ) );
        for( i = 0; i < ((CvBtClassifier*) classifier)->numiter; i++ )
        {
            CV_READ_SEQ_ELEM( tree, reader );
            val += tree->eval( (CvClassifier*) tree, sample );
        }
    }
    else
    {
        CvCARTClassifier** ptree;

        ptree = ((CvBtClassifier*) classifier)->trees;
        for( i = 0; i < ((CvBtClassifier*) classifier)->numiter; i++ )
        {
            val += (*ptree)->eval( (CvClassifier*) (*ptree), sample );
            ptree++;
        }
    }

    __END__;

    return val;
}

CV_BOOST_IMPL
float cvEvalBtClassifier2( CvClassifier* classifier, CvMat* sample )
{
    float val;

    CV_FUNCNAME( "cvEvalBtClassifier2" );

    __BEGIN__;
    
    CV_CALL( val = cvEvalBtClassifier( classifier, sample ) );

    __END__;

    return (float) (val >= 0.0F);
}

CV_BOOST_IMPL
float cvEvalBtClassifierK( CvClassifier* classifier, CvMat* sample )
{
    int cls;

    CV_FUNCNAME( "cvEvalBtClassifierK" );

    __BEGIN__;
    
    int i, k;
    float max_val;
    int numclasses;

    float* vals;
    size_t data_size;

    numclasses = ((CvBtClassifier*) classifier)->numclasses;
    data_size = sizeof( *vals ) * numclasses;
    CV_CALL( vals = (float*) cvAlloc( data_size ) );
    memset( vals, 0, data_size );

    if( CV_IS_TUNABLE( classifier->flags ) )
    {
        CvSeqReader reader;
        CvCARTClassifier* tree;

        CV_CALL( cvStartReadSeq( ((CvBtClassifier*) classifier)->seq, &reader ) );
        for( i = 0; i < ((CvBtClassifier*) classifier)->numiter; i++ )
        {
            for( k = 0; k < numclasses; k++ )
            {
                CV_READ_SEQ_ELEM( tree, reader );
                vals[k] += tree->eval( (CvClassifier*) tree, sample );
            }
        }

    }
    else
    {
        CvCARTClassifier** ptree;

        ptree = ((CvBtClassifier*) classifier)->trees;
        for( i = 0; i < ((CvBtClassifier*) classifier)->numiter; i++ )
        {
            for( k = 0; k < numclasses; k++ )
            {
                vals[k] += (*ptree)->eval( (CvClassifier*) (*ptree), sample );
                ptree++;
            }
        }
    }

    cls = 0;
    max_val = vals[cls];
    for( k = 1; k < numclasses; k++ )
    {
        if( vals[k] > max_val )
        {
            max_val = vals[k];
            cls = k;
        }
    }

    CV_CALL( cvFree( &vals ) );

    __END__;

    return (float) cls;
}

typedef float (*CvEvalBtClassifier)( CvClassifier* classifier, CvMat* sample );

static CvEvalBtClassifier icvEvalBtClassifier[] =
{
    cvEvalBtClassifier2,
    cvEvalBtClassifier2,
    cvEvalBtClassifier2,
    cvEvalBtClassifier2,
    cvEvalBtClassifier2,
    cvEvalBtClassifierK,
    cvEvalBtClassifier,
    cvEvalBtClassifier,
    cvEvalBtClassifier
};

CV_BOOST_IMPL
int cvSaveBtClassifier( CvClassifier* classifier, const char* filename )
{
    CV_FUNCNAME( "cvSaveBtClassifier" );

    __BEGIN__;

    FILE* file;
    int i, j;
    CvSeqReader reader;
    CvCARTClassifier* tree;

    CV_ASSERT( classifier );
    CV_ASSERT( filename );
    
    if( !icvMkDir( filename ) || !(file = fopen( filename, "w" )) )
    {
        CV_ERROR( CV_StsError, "Unable to create file" );
    }

    if( CV_IS_TUNABLE( classifier->flags ) )
    {
        CV_CALL( cvStartReadSeq( ((CvBtClassifier*) classifier)->seq, &reader ) );
    }
    fprintf( file, "%d %d\n%d\n%d\n", (int) ((CvBtClassifier*) classifier)->type,
                                      ((CvBtClassifier*) classifier)->numclasses,
                                      ((CvBtClassifier*) classifier)->numfeatures,
                                      ((CvBtClassifier*) classifier)->numiter );
    
    for( i = 0; i < ((CvBtClassifier*) classifier)->numclasses *
                    ((CvBtClassifier*) classifier)->numiter; i++ )
    {
        if( CV_IS_TUNABLE( classifier->flags ) )
        {
            CV_READ_SEQ_ELEM( tree, reader );
        }
        else
        {
            tree = ((CvBtClassifier*) classifier)->trees[i];
        }

        fprintf( file, "%d\n", tree->count );
        for( j = 0; j < tree->count; j++ )
        {
            fprintf( file, "%d %g %d %d\n", tree->compidx[j],
                                            tree->threshold[j],
                                            tree->left[j],
                                            tree->right[j] );
        }
        for( j = 0; j <= tree->count; j++ )
        {
            fprintf( file, "%g ", tree->val[j] );
        }
        fprintf( file, "\n" );
    }

    fclose( file );

    __END__;

    return 1;
}


CV_BOOST_IMPL
void cvReleaseBtClassifier( CvClassifier** ptr )
{
    CV_FUNCNAME( "cvReleaseBtClassifier" );

    __BEGIN__;

    int i;

    if( ptr == NULL || *ptr == NULL )
    {
        CV_ERROR( CV_StsNullPtr, "" );
    }
    if( CV_IS_TUNABLE( (*ptr)->flags ) )
    {
        CvSeqReader reader;
        CvCARTClassifier* tree;

        CV_CALL( cvStartReadSeq( ((CvBtClassifier*) *ptr)->seq, &reader ) );
        for( i = 0; i < ((CvBtClassifier*) *ptr)->numclasses *
                        ((CvBtClassifier*) *ptr)->numiter; i++ )
        {
            CV_READ_SEQ_ELEM( tree, reader );
            tree->release( (CvClassifier**) (&tree) );
        }
        CV_CALL( cvReleaseMemStorage( &(((CvBtClassifier*) *ptr)->seq->storage) ) );
    }
    else
    {
        CvCARTClassifier** ptree;

        ptree = ((CvBtClassifier*) *ptr)->trees;
        for( i = 0; i < ((CvBtClassifier*) *ptr)->numclasses *
                        ((CvBtClassifier*) *ptr)->numiter; i++ )
        {
            (*ptree)->release( (CvClassifier**) ptree );
            ptree++;
        }
    }

    CV_CALL( cvFree( ptr ) );
    *ptr = NULL;

    __END__;
}

void cvTuneBtClassifier( CvClassifier* classifier, CvMat*, int flags,
                         CvMat*, CvMat* , CvMat*, CvMat*, CvMat* )
{
    CV_FUNCNAME( "cvTuneBtClassifier" );

    __BEGIN__;

    size_t data_size;

    if( CV_IS_TUNABLE( flags ) )
    {
        if( !CV_IS_TUNABLE( classifier->flags ) )
        {
            CV_ERROR( CV_StsUnsupportedFormat,
                      "Classifier does not support tune function" );
        }
        else
        {
            /* tune classifier */
            CvCARTClassifier** trees;

            printf( "Iteration %d\n", ((CvBtClassifier*) classifier)->numiter + 1 );

            data_size = sizeof( *trees ) * ((CvBtClassifier*) classifier)->numclasses;
            CV_CALL( trees = (CvCARTClassifier**) cvAlloc( data_size ) );
            CV_CALL( cvBtNext( trees,
                (CvBtTrainer*) ((CvBtClassifier*) classifier)->trainer ) );
            CV_CALL( cvSeqPushMulti( ((CvBtClassifier*) classifier)->seq,
                trees, ((CvBtClassifier*) classifier)->numclasses ) );
            CV_CALL( cvFree( &trees ) );
            ((CvBtClassifier*) classifier)->numiter++;
        }
    }
    else
    {
        if( CV_IS_TUNABLE( classifier->flags ) )
        {
            /* convert */
            void* ptr;

            assert( ((CvBtClassifier*) classifier)->seq->total ==
                        ((CvBtClassifier*) classifier)->numiter *
                        ((CvBtClassifier*) classifier)->numclasses );

            data_size = sizeof( ((CvBtClassifier*) classifier)->trees[0] ) *
                ((CvBtClassifier*) classifier)->seq->total;
            CV_CALL( ptr = cvAlloc( data_size ) );
            CV_CALL( cvCvtSeqToArray( ((CvBtClassifier*) classifier)->seq, ptr ) );
            CV_CALL( cvReleaseMemStorage( 
                    &(((CvBtClassifier*) classifier)->seq->storage) ) );
            ((CvBtClassifier*) classifier)->trees = (CvCARTClassifier**) ptr;
            classifier->flags &= ~CV_TUNABLE;
            CV_CALL( cvBtEnd( (CvBtTrainer**)
                &(((CvBtClassifier*) classifier)->trainer )) );
            ((CvBtClassifier*) classifier)->trainer = NULL;
        }
    }

    __END__;
}

CvBtClassifier* icvAllocBtClassifier( CvBoostType type, int flags, int numclasses,
                                      int numiter )
{
    CvBtClassifier* ptr;
    size_t data_size;

    assert( numclasses >= 1 );
    assert( numiter >= 0 );
    assert( ( numclasses == 1 ) || (type == CV_LKCLASS) );

    data_size = sizeof( *ptr );
    ptr = (CvBtClassifier*) cvAlloc( data_size );
    memset( ptr, 0, data_size );

    if( CV_IS_TUNABLE( flags ) )
    {
        ptr->seq = cvCreateSeq( 0, sizeof( *(ptr->seq) ), sizeof( *(ptr->trees) ),
                                cvCreateMemStorage() );
        ptr->numiter = 0;
    }
    else
    {
        data_size = numclasses * numiter * sizeof( *(ptr->trees) );
        ptr->trees = (CvCARTClassifier**) cvAlloc( data_size );
        memset( ptr->trees, 0, data_size );

        ptr->numiter = numiter;
    }

    ptr->flags = flags;
    ptr->numclasses = numclasses;
    ptr->type = type;

    ptr->eval = icvEvalBtClassifier[(int) type];
    ptr->tune = cvTuneBtClassifier;
    ptr->save = cvSaveBtClassifier;
    ptr->release = cvReleaseBtClassifier;

    return ptr;
}

CV_BOOST_IMPL
CvClassifier* cvCreateBtClassifier( CvMat* trainData,
                                    int flags,
                                    CvMat* trainClasses,
                                    CvMat* typeMask,
                                    CvMat* missedMeasurementsMask,
                                    CvMat* compIdx,
                                    CvMat* sampleIdx,
                                    CvMat* weights,
                                    CvClassifierTrainParams* trainParams )
{
    CvBtClassifier* ptr;

    CV_FUNCNAME( "cvCreateBtClassifier" );

    __BEGIN__;
    CvBoostType type;
    int num_classes;
    int num_iter;
    int i;
    CvCARTClassifier** trees;
    size_t data_size;

    CV_ASSERT( trainData != NULL );
    CV_ASSERT( trainClasses != NULL );
    CV_ASSERT( typeMask == NULL );
    CV_ASSERT( missedMeasurementsMask == NULL );
    CV_ASSERT( compIdx == NULL );
    CV_ASSERT( weights == NULL );
    CV_ASSERT( trainParams != NULL );

    type = ((CvBtClassifierTrainParams*) trainParams)->type;
    
    if( type >= CV_DABCLASS && type <= CV_GABCLASS && sampleIdx )
    {
        CV_ERROR( CV_StsBadArg, "Sample indices are not supported for this type" );
    }

    if( type == CV_LKCLASS )
    {
        double min_val;
        double max_val;

        cvMinMaxLoc( trainClasses, &min_val, &max_val );
        num_classes = (int) (max_val + 1.0);
        
        CV_ASSERT( num_classes >= 2 );
    }
    else
    {
        num_classes = 1;
    }
    num_iter = ((CvBtClassifierTrainParams*) trainParams)->numiter;
    
    CV_ASSERT( num_iter > 0 );

    ptr = icvAllocBtClassifier( type, CV_TUNABLE | flags, num_classes, num_iter );
    ptr->numfeatures = (CV_IS_ROW_SAMPLE( flags )) ? trainData->cols : trainData->rows;
    
    i = 0;

    printf( "Iteration %d\n", 1 );

    data_size = sizeof( *trees ) * ptr->numclasses;
    CV_CALL( trees = (CvCARTClassifier**) cvAlloc( data_size ) );

    CV_CALL( ptr->trainer = cvBtStart( trees, trainData, flags, trainClasses, sampleIdx,
        ((CvBtClassifierTrainParams*) trainParams)->numsplits, type, num_classes,
        &(((CvBtClassifierTrainParams*) trainParams)->param[0]) ) );

    CV_CALL( cvSeqPushMulti( ptr->seq, trees, ptr->numclasses ) );
    CV_CALL( cvFree( &trees ) );
    ptr->numiter++;
    
    for( i = 1; i < num_iter; i++ )
    {
        ptr->tune( (CvClassifier*) ptr, NULL, CV_TUNABLE, NULL, NULL, NULL, NULL, NULL );
    }
    if( !CV_IS_TUNABLE( flags ) )
    {
        /* convert */
        ptr->tune( (CvClassifier*) ptr, NULL, 0, NULL, NULL, NULL, NULL, NULL );
    }

    __END__;

    return (CvClassifier*) ptr;
}

CV_BOOST_IMPL
CvClassifier* cvCreateBtClassifierFromFile( const char* filename )
{
    CvBtClassifier* ptr;

    CV_FUNCNAME( "cvCreateBtClassifierFromFile" );
    
    __BEGIN__;

    FILE* file;
    int i, j;
    int data_size;
    int num_classifiers;
    int num_features;
    int num_classes;
    int type;

    CV_ASSERT( filename != NULL );

    ptr = NULL;
    file = fopen( filename, "r" );
    if( !file )
    {
        CV_ERROR( CV_StsError, "Unable to open file" );
    }
    
    fscanf( file, "%d %d %d %d", &type, &num_classes, &num_features, &num_classifiers );

    CV_ASSERT( type >= (int) CV_DABCLASS && type <= (int) CV_MREG );
    CV_ASSERT( num_features > 0 );
    CV_ASSERT( num_classifiers > 0 );

    if( (CvBoostType) type != CV_LKCLASS )
    {
        num_classes = 1;
    }
    ptr = icvAllocBtClassifier( (CvBoostType) type, 0, num_classes, num_classifiers );
    ptr->numfeatures = num_features;
    
    for( i = 0; i < num_classes * num_classifiers; i++ )
    {
        int count;
        CvCARTClassifier* tree;

        fscanf( file, "%d", &count );

        data_size = sizeof( *tree )
            + count * ( sizeof( *(tree->compidx) ) + sizeof( *(tree->threshold) ) +
                        sizeof( *(tree->right) ) + sizeof( *(tree->left) ) )
            + (count + 1) * ( sizeof( *(tree->val) ) );
        CV_CALL( tree = (CvCARTClassifier*) cvAlloc( data_size ) );
        memset( tree, 0, data_size );
        tree->eval = cvEvalCARTClassifier;
        tree->tune = NULL;
        tree->save = NULL;
        tree->release = cvReleaseCARTClassifier;
        tree->compidx = (int*) ( tree + 1 );
        tree->threshold = (float*) ( tree->compidx + count );
        tree->left = (int*) ( tree->threshold + count );
        tree->right = (int*) ( tree->left + count );
        tree->val = (float*) ( tree->right + count );

        tree->count = count;
        for( j = 0; j < tree->count; j++ )
        {
            fscanf( file, "%d %g %d %d", &(tree->compidx[j]),
                                         &(tree->threshold[j]),
                                         &(tree->left[j]),
                                         &(tree->right[j]) );
        }
        for( j = 0; j <= tree->count; j++ )
        {
            fscanf( file, "%g", &(tree->val[j]) );
        }
        ptr->trees[i] = tree;
    }

    fclose( file );

    __END__;

    return (CvClassifier*) ptr;
}

/****************************************************************************************\
*                                    Utility functions                                   *
\****************************************************************************************/

CV_BOOST_IMPL
CvMat* cvTrimWeights( CvMat* weights, CvMat* idx, float factor )
{
    CvMat* ptr;

    CV_FUNCNAME( "cvTrimWeights" );
    __BEGIN__;
    int i, index, num;
    float sum_weights;
    uchar* wdata;
    size_t wstep;
    int wnum;
    float threshold;
    int count;
    float* sorted_weights;

    CV_ASSERT( CV_MAT_TYPE( weights->type ) == CV_32FC1 );

    ptr = idx;
    sorted_weights = NULL;

    if( factor > 0.0F && factor < 1.0F )
    {
        size_t data_size;

        CV_MAT2VEC( *weights, wdata, wstep, wnum );
        num = ( idx == NULL ) ? wnum : MAX( idx->rows, idx->cols );

        data_size = num * sizeof( *sorted_weights );
        sorted_weights = (float*) cvAlloc( data_size );
        memset( sorted_weights, 0, data_size );

        sum_weights = 0.0F;
        for( i = 0; i < num; i++ )
        {
            index = icvGetIdxAt( idx, i );
            sorted_weights[i] = *((float*) (wdata + index * wstep));
            sum_weights += sorted_weights[i];
        }

        icvSort_32f( sorted_weights, num, 0 );

        sum_weights *= (1.0F - factor);

        i = -1;
        do { sum_weights -= sorted_weights[++i]; }
        while( sum_weights > 0.0F && i < (num - 1) );

        threshold = sorted_weights[i];

        while( i > 0 && sorted_weights[i-1] == threshold ) i--;

        if( i > 0 || ( idx != NULL && CV_MAT_TYPE( idx->type ) != CV_32FC1 ) )
        {
            CV_CALL( ptr = cvCreateMat( 1, num - i, CV_32FC1 ) );
            count = 0;
            for( i = 0; i < num; i++ )
            {
                index = icvGetIdxAt( idx, i );
                if( *((float*) (wdata + index * wstep)) >= threshold )
                {
                    CV_MAT_ELEM( *ptr, float, 0, count ) = (float) index;
                    count++;
                }
            }
        
            assert( count == ptr->cols );
        }
        cvFree( &sorted_weights );
    }

    __END__;

    return ptr;
}


CV_BOOST_IMPL
void cvReadTrainData( const char* filename, int flags,
                      CvMat** trainData,
                      CvMat** trainClasses )
{

    CV_FUNCNAME( "cvReadTrainData" );

    __BEGIN__;

    FILE* file;
    int m, n;
    int i, j;
    float val;

    if( filename == NULL )
    {
        CV_ERROR( CV_StsNullPtr, "filename must be specified" );
    }
    if( trainData == NULL )
    {
        CV_ERROR( CV_StsNullPtr, "trainData must be not NULL" );
    }
    if( trainClasses == NULL )
    {
        CV_ERROR( CV_StsNullPtr, "trainClasses must be not NULL" );
    }
    
    *trainData = NULL;
    *trainClasses = NULL;
    file = fopen( filename, "r" );
    if( !file )
    {
        CV_ERROR( CV_StsError, "Unable to open file" );
    }

    fscanf( file, "%d %d", &m, &n );

    if( CV_IS_ROW_SAMPLE( flags ) )
    {
        CV_CALL( *trainData = cvCreateMat( m, n, CV_32FC1 ) );
    }
    else
    {
        CV_CALL( *trainData = cvCreateMat( n, m, CV_32FC1 ) );
    }
    
    CV_CALL( *trainClasses = cvCreateMat( 1, m, CV_32FC1 ) );

    for( i = 0; i < m; i++ )
    {
        for( j = 0; j < n; j++ )
        {
            fscanf( file, "%f", &val );
            if( CV_IS_ROW_SAMPLE( flags ) )
            {
                CV_MAT_ELEM( **trainData, float, i, j ) = val;
            }
            else
            {
                CV_MAT_ELEM( **trainData, float, j, i ) = val;
            }
        }
        fscanf( file, "%f", &val );
        CV_MAT_ELEM( **trainClasses, float, 0, i ) = val;
    }

    fclose( file );

    __END__;
    
}

CV_BOOST_IMPL
void cvWriteTrainData( const char* filename, int flags,
                       CvMat* trainData, CvMat* trainClasses, CvMat* sampleIdx )
{
    CV_FUNCNAME( "cvWriteTrainData" );

    __BEGIN__;

    FILE* file;
    int m, n;
    int i, j;
    int clsrow;
    int count;
    int idx;
    CvScalar sc;

    if( filename == NULL )
    {
        CV_ERROR( CV_StsNullPtr, "filename must be specified" );
    }
    if( trainData == NULL || CV_MAT_TYPE( trainData->type ) != CV_32FC1 )
    {
        CV_ERROR( CV_StsUnsupportedFormat, "Invalid trainData" );
    }
    if( CV_IS_ROW_SAMPLE( flags ) )
    {
        m = trainData->rows;
        n = trainData->cols;
    }
    else
    {
        n = trainData->rows;
        m = trainData->cols;
    }
    if( trainClasses == NULL || CV_MAT_TYPE( trainClasses->type ) != CV_32FC1 ||
        MIN( trainClasses->rows, trainClasses->cols ) != 1 )
    {
        CV_ERROR( CV_StsUnsupportedFormat, "Invalid trainClasses" );
    }
    clsrow = (trainClasses->rows == 1);
    if( m != ( (clsrow) ? trainClasses->cols : trainClasses->rows ) )
    {
        CV_ERROR( CV_StsUnmatchedSizes, "Incorrect trainData and trainClasses sizes" );
    }
    
    if( sampleIdx != NULL )
    {
        count = (sampleIdx->rows == 1) ? sampleIdx->cols : sampleIdx->rows;
    }
    else
    {
        count = m;
    }
    

    file = fopen( filename, "w" );
    if( !file )
    {
        CV_ERROR( CV_StsError, "Unable to create file" );
    }

    fprintf( file, "%d %d\n", count, n );

    for( i = 0; i < count; i++ )
    {
        if( sampleIdx )
        {
            if( sampleIdx->rows == 1 )
            {
                sc = cvGet2D( sampleIdx, 0, i );
            }
            else
            {
                sc = cvGet2D( sampleIdx, i, 0 );
            }
            idx = (int) sc.val[0];
        }
        else
        {
            idx = i;
        }
        for( j = 0; j < n; j++ )
        {
            fprintf( file, "%g ", ( (CV_IS_ROW_SAMPLE( flags ))
                                    ? CV_MAT_ELEM( *trainData, float, idx, j ) 
                                    : CV_MAT_ELEM( *trainData, float, j, idx ) ) );
        }
        fprintf( file, "%g\n", ( (clsrow)
                                ? CV_MAT_ELEM( *trainClasses, float, 0, idx )
                                : CV_MAT_ELEM( *trainClasses, float, idx, 0 ) ) );
    }

    fclose( file );
    
    __END__;
}


#define ICV_RAND_SHUFFLE( suffix, type )                                                 \
void icvRandShuffle_##suffix( uchar* data, size_t step, int num )                        \
{                                                                                        \
    CvRandState state;                                                                   \
    time_t seed;                                                                         \
    type tmp;                                                                            \
    int i;                                                                               \
    float rn;                                                                            \
                                                                                         \
    time( &seed );                                                                       \
                                                                                         \
    cvRandInit( &state, (double) 0, (double) 0, (int)seed );                             \
    for( i = 0; i < (num-1); i++ )                                                       \
    {                                                                                    \
        rn = ((float) cvRandNext( &state )) / (1.0F + UINT_MAX);                         \
        CV_SWAP( *((type*)(data + i * step)),                                            \
                 *((type*)(data + ( i + (int)( rn * (num - i ) ) )* step)),              \
                 tmp );                                                                  \
    }                                                                                    \
}

ICV_RAND_SHUFFLE( 8U, uchar )

ICV_RAND_SHUFFLE( 16S, short )

ICV_RAND_SHUFFLE( 32S, int )

ICV_RAND_SHUFFLE( 32F, float )

CV_BOOST_IMPL
void cvRandShuffleVec( CvMat* mat )
{
    CV_FUNCNAME( "cvRandShuffle" );

    __BEGIN__;

    uchar* data;
    size_t step;
    int num;

    if( (mat == NULL) || !CV_IS_MAT( mat ) || MIN( mat->rows, mat->cols ) != 1 )
    {
        CV_ERROR( CV_StsUnsupportedFormat, "" );
    }

    CV_MAT2VEC( *mat, data, step, num );
    switch( CV_MAT_TYPE( mat->type ) )
    {
        case CV_8UC1:
            icvRandShuffle_8U( data, step, num);
            break;
        case CV_16SC1:
            icvRandShuffle_16S( data, step, num);
            break;
        case CV_32SC1:
            icvRandShuffle_32S( data, step, num);
            break;
        case CV_32FC1:
            icvRandShuffle_32F( data, step, num);
            break;
        default:
            CV_ERROR( CV_StsUnsupportedFormat, "" );
    }

    __END__;
}

/* End of file. */
