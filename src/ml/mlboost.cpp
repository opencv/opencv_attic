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

#include "_ml.h"
#include <stddef.h>

#ifdef _OPENMP
#include <omp.h>
#endif /* _OPENMP */

/* from file cvclassifier.h */

/* Convert matrix to vector */
#define CV_MAT2VEC( mat, vdata, vstep, num )       \
    assert( (mat).rows == 1 || (mat).cols == 1 );  \
    (vdata) = ((mat).data.ptr);                    \
    if( (mat).rows == 1 )                          \
    {                                              \
        (vstep) = CV_ELEM_SIZE( (mat).type );      \
        (num) = (mat).cols;                        \
    }                                              \
    else                                           \
    {                                              \
        (vstep) = (mat).step;                      \
        (num) = (mat).rows;                        \
    }

/* Set up <sample> matrix header to be <num> sample of <trainData> samples matrix */
#define CV_GET_SAMPLE( trainData, tdflags, num, sample )                                 \
if( CV_IS_ROW_SAMPLE( tdflags ) )                                                        \
{                                                                                        \
    cvInitMatHeader( &(sample), 1, (trainData).cols,                                     \
                     CV_MAT_TYPE( (trainData).type ),                                    \
                     ((trainData).data.ptr + (num) * (trainData).step),                  \
                     (trainData).step );                                                 \
}                                                                                        \
else                                                                                     \
{                                                                                        \
    cvInitMatHeader( &(sample), (trainData).rows, 1,                                     \
                     CV_MAT_TYPE( (trainData).type ),                                    \
                     ((trainData).data.ptr + (num) * CV_ELEM_SIZE( (trainData).type )),  \
                     (trainData).step );                                                 \
}

#define CV_GET_SAMPLE_STEP( trainData, tdflags, sstep )                                  \
(sstep) = ( ( CV_IS_ROW_SAMPLE( tdflags ) )                                              \
           ? (trainData).step : CV_ELEM_SIZE( (trainData).type ) );


#define CV_LOGRATIO_THRESHOLD 0.00001F

/* log( val / (1 - val ) ) */
CV_INLINE float cvLogRatio( float val );

CV_INLINE float cvLogRatio( float val )
{
    float tval;

    tval = MAX(CV_LOGRATIO_THRESHOLD, MIN( 1.0F - CV_LOGRATIO_THRESHOLD, (val) ));
    return logf( tval / (1.0F - tval) );
}

typedef CvClassifier* (*CvClassifierConstructor)( CvMat*, int, CvMat*, CvMat*, CvMat*,
                                                  CvMat*, CvMat*, CvMat*,
                                                  CvClassifierTrainParams* );

typedef enum CvStumpType
{
    CV_CLASSIFICATION       = 0,
    CV_CLASSIFICATION_CLASS = 1,
    CV_REGRESSION           = 2
} CvStumpType;

typedef enum CvStumpError
{
    CV_MISCLASSIFICATION = 0,
    CV_GINI              = 1,
    CV_ENTROPY           = 2,
    CV_SQUARE            = 3
} CvStumpError;

typedef struct CvStumpTrainParams
{
    CV_STAT_MODEL_PARAM_FIELDS();
    CvStumpType  type;
    CvStumpError error;
} CvStumpTrainParams;

typedef struct CvMTStumpTrainParams
{
    CV_STAT_MODEL_PARAM_FIELDS();
    CvStumpType  type;
    CvStumpError error;
    int portion; /* number of components calculated in each thread */
    int numcomp; /* total number of components */
    
    /* callback which fills <mat> with components [first, first+num[ */
    void (*getTrainData)( CvMat* mat, CvMat* sampleIdx, CvMat* compIdx,
                          int first, int num, void* userdata );
    CvMat* sortedIdx; /* presorted samples indices */
    void* userdata; /* passed to callback */
} CvMTStumpTrainParams;

typedef struct CvStumpClassifier
{
    CV_STAT_MODEL_FIELDS();
    int compidx;
    
    float lerror; /* impurity of the right node */
    float rerror; /* impurity of the left  node */
    
    float threshold;
    float left;
    float right;
} CvStumpClassifier;

typedef struct CvTREEBOOSTTrainParams
{
    CV_STAT_MODEL_PARAM_FIELDS();
    /* desired number of internal nodes */
    int count;
    CvClassifierTrainParams* stumpTrainParams;
    CvClassifierConstructor  stumpConstructor;
    
    /*
     * Split sample indices <idx>
     * on the "left" indices <left> and "right" indices <right>
     * according to samples components <compidx> values and <threshold>.
     *
     * NOTE: Matrices <left> and <right> must be allocated using cvCreateMat function
     *   since they are freed using cvReleaseMat function
     *
     * If it is NULL then the default implementation which evaluates training
     * samples from <trainData> passed to classifier constructor is used
     */
    void (*splitIdx)( int compidx, float threshold,
                      CvMat* idx, CvMat** left, CvMat** right,
                      void* userdata );
    void* userdata;
} CvTREEBOOSTTrainParams;

CV_IMPL
void cvGetSortedIndices( CvMat* val, CvMat* idx, int sortcols CV_DEFAULT( 0 ) );

CV_IMPL
void cvReleaseStumpClassifier( CvClassifier** classifier );

CV_IMPL
float cvEvalStumpClassifier( const CvClassifier* classifier,
                             const CvMat* sample, CvMat* CV_DEFAULT(0) );

CV_IMPL
CvClassifier* cvCreateStumpClassifier( CvMat* trainData,
                     int flags,
                     CvMat* trainClasses,
                     CvMat* typeMask,
                     CvMat* missedMeasurementsMask,
                     CvMat* compIdx,
                     CvMat* sampleIdx,
                     CvMat* weights,
                     CvClassifierTrainParams* trainParams );

/*
 * cvCreateMTStumpClassifier
 *
 * Multithreaded stump classifier constructor
 * Includes huge train data support through callback function
 */
CV_IMPL
CvClassifier* cvCreateMTStumpClassifier( CvMat* trainData,
                                         int flags,
                                         CvMat* trainClasses,
                                         CvMat* typeMask,
                                         CvMat* missedMeasurementsMask,
                                         CvMat* compIdx,
                                         CvMat* sampleIdx,
                                         CvMat* weights,
                                         CvClassifierTrainParams* trainParams );

/*
 * cvCreateTREEBOOSTClassifier
 *
 * TREEBOOST classifier constructor
 */
CV_IMPL
CvClassifier* cvCreateTREEBOOSTClassifier( CvMat* trainData,
                     int flags,
                     CvMat* trainClasses,
                     CvMat* typeMask,
                      CvMat* missedMeasurementsMask,
                      CvMat* compIdx,
                      CvMat* sampleIdx,
                      CvMat* weights,
                      CvClassifierTrainParams* trainParams );

CV_IMPL
void cvReleaseTREEBOOSTClassifier( CvClassifier** classifier );

CV_IMPL
float cvEvalTREEBOOSTClassifier( const CvClassifier* classifier,
                                 const CvMat* sample, CvMat* CV_DEFAULT(0) );

/****************************************************************************************\
*                                        Boosting                                        *
\****************************************************************************************/

/****************************************************************************************\
*                             Iterative training functions                               *
\****************************************************************************************/

CV_INLINE void
icvBoostStart( CvMat* responses, CvMat* weak_responses, CvMat* weights, int type,
               CvMat* sum_resp );

double
icvBoostIterate( CvMat* weak_eval, CvMat* responses, CvMat* weak_train_resp,
                 CvMat* weights, int type, CvMat* sum_resp );
                
CV_INLINE void
icvBoostStart( CvMat* responses, CvMat* weak_responses, CvMat* weights, int type,
               CvMat* sum_resp )
{
    icvBoostIterate( NULL, responses, weak_responses, weights, type, sum_resp);
}

/*
 * CvBoostTrainer
 *
 * The CvBoostTrainer structure represents internal boosting trainer.
 */
typedef struct CvBoostTrainer CvBoostTrainer;

/*
 * cvBoostStartTraining
 *
 * The cvBoostStartTraining function starts training process and calculates
 * response values and weights for the first weak classifier training.
 *
 * Parameters
 *   trainClasses
 *     Vector of classes of training samples classes. Each element must be 0 or 1 and
 *     of type CV_32FC1.
 *   weakTrainVals
 *     Vector of response values for the first trained weak classifier.
 *     Must be of type CV_32FC1.
 *   weights
 *     Weight vector of training samples for the first trained weak classifier.
 *     Must be of type CV_32FC1.
 *   type
 *     Boosting type. CV_BT_DISCRETE, CV_BT_REAL, CV_BT_LOGIT, CV_BT_GENTLE
 *     types are supported.
 *
 * Return Values
 *   The return value is a pointer to internal trainer structure which is used
 *   to perform next training iterations.
 *
 * Remarks
 *   weakTrainVals and weights must be allocated before calling the function
 *   and of the same size as trainingClasses. Usually weights should be initialized
 *   with 1.0 value.
 *   The function calculates response values and weights for the first weak
 *   classifier training and stores them into weakTrainVals and weights
 *   respectively.
 *   Note, the training of the weak classifier using weakTrainVals, weight,
 *   trainingData is outside of this function.
 */
CV_IMPL
CvBoostTrainer* cvBoostStartTraining( CvMat* trainClasses,
                                      CvMat* weakTrainVals,
                                      CvMat* weights,
                                      int type );
/*
 * cvBoostNextWeakClassifier
 *
 * The cvBoostNextWeakClassifier function performs next training
 * iteration and caluclates response values and weights for the next weak
 * classifier training.
 *
 * Parameters
 *   weakEvalVals
 *     Vector of values obtained by evaluation of each sample with
 *     the last trained weak classifier (iteration i). Must be of CV_32FC1 type.
 *   trainClasses
 *     Vector of classes of training samples. Each element must be 0 or 1,
 *     and of type CV_32FC1.
 *   weakTrainVals
 *     Vector of response values for the next weak classifier training
 *     (iteration i+1). Must be of type CV_32FC1.
 *   weights
 *     Weight vector of training samples for the next weak classifier training
 *     (iteration i+1). Must be of type CV_32FC1.
 *   trainer
 *     A pointer to internal trainer returned by the cvBoostStartTraining
 *     function call.
 *
 * Return Values
 *   The return value is the coefficient for the last trained weak classifier.
 *
 * Remarks
 *   weakTrainVals and weights must be exactly the same vectors as used in
 *   the cvBoostStartTraining function call and should not be modified.
 *   The function calculates response values and weights for the next weak
 *   classifier training and stores them into weakTrainVals and weights
 *   respectively.
 *   Note, the training of the weak classifier of iteration i+1 using
 *   weakTrainVals, weight, trainingData is outside of this function.
 */
CV_IMPL
float cvBoostNextWeakClassifier( CvMat* weakEvalVals,
                                 CvMat* trainClasses,
                                 CvMat* weakTrainVals,
                                 CvMat* weights,
                                 CvBoostTrainer* trainer );

/*
 * cvBoostEndTraining
 *
 * The cvBoostEndTraining function finishes training process and releases
 * internally allocated memory.
 *
 * Parameters
 *   trainer
 *     A pointer to a pointer to internal trainer returned by the cvBoostStartTraining
 *     function call.
 */
CV_IMPL
void cvBoostEndTraining( CvBoostTrainer** trainer );

CV_IMPL
CvMat* cvTrimWeights( CvMat* weights, CvMat* idx, float factor );

/* end from cvclassifier.h */

#define CV_DECLARE_QSORT( func_name, T, less_than )                     \
void func_name( T* array, size_t length, int aux );

#define less_than( a, b ) ((a) < (b))

CV_DECLARE_QSORT( icvSort_32f, float, less_than )

CV_IMPLEMENT_QSORT( icvSort_32f, float, less_than )

typedef struct CvValArray
{
    uchar* data;
    int    step;
} CvValArray;

#define CMP_VALUES( idx1, idx2 )                                 \
    ( *( (float*) (aux->data + ((int) (idx1)) * aux->step ) ) <  \
      *( (float*) (aux->data + ((int) (idx2)) * aux->step ) ) )

CV_IMPLEMENT_QSORT_EX( icvSortIndexedValArray_16s, short, CMP_VALUES, CvValArray* )

CV_IMPLEMENT_QSORT_EX( icvSortIndexedValArray_32s, int,   CMP_VALUES, CvValArray* )

CV_IMPLEMENT_QSORT_EX( icvSortIndexedValArray_32f, float, CMP_VALUES, CvValArray* )

CV_IMPL
void cvGetSortedIndices( CvMat* val, CvMat* idx, int sortcols )
{
    int idxtype = 0;
    //uchar* data = NULL;
    int istep = 0;
    int jstep = 0;

    int i = 0;
    int j = 0;

    CvValArray va;

    assert( idx != NULL );
    assert( val != NULL );

    idxtype = CV_MAT_TYPE( idx->type );
    assert( idxtype == CV_16SC1 || idxtype == CV_32SC1 || idxtype == CV_32FC1 );
    assert( CV_MAT_TYPE( val->type ) == CV_32FC1 );
    if( sortcols )
    {
        assert( idx->rows == val->cols );
        assert( idx->cols == val->rows );
        istep = CV_ELEM_SIZE( val->type );
        jstep = val->step;
    }
    else
    {
        assert( idx->rows == val->rows );
        assert( idx->cols == val->cols );
        istep = val->step;
        jstep = CV_ELEM_SIZE( val->type );
    }

    va.data = val->data.ptr;
    va.step = jstep;
    switch( idxtype )
    {
        case CV_16SC1:
            for( i = 0; i < idx->rows; i++ )
            {
                for( j = 0; j < idx->cols; j++ )
                {
                    CV_MAT_ELEM( *idx, short, i, j ) = (short) j;
                }
                icvSortIndexedValArray_16s( (short*) (idx->data.ptr + i * idx->step),
                                            idx->cols, &va );
                va.data += istep;
            }
            break;

        case CV_32SC1:
            for( i = 0; i < idx->rows; i++ )
            {
                for( j = 0; j < idx->cols; j++ )
                {
                    CV_MAT_ELEM( *idx, int, i, j ) = j;
                }
                icvSortIndexedValArray_32s( (int*) (idx->data.ptr + i * idx->step),
                                            idx->cols, &va );
                va.data += istep;
            }
            break;

        case CV_32FC1:
            for( i = 0; i < idx->rows; i++ )
            {
                for( j = 0; j < idx->cols; j++ )
                {
                    CV_MAT_ELEM( *idx, float, i, j ) = (float) j;
                }
                icvSortIndexedValArray_32f( (float*) (idx->data.ptr + i * idx->step),
                                            idx->cols, &va );
                va.data += istep;
            }
            break;

        default:
            assert( 0 );
            break;
    }
}

/*
 * get index at specified position from index matrix of any type
 * if matrix is NULL, then specified position is returned
 */
int icvGetIdxAt( CvMat* idx, int pos )
{
    if( idx == NULL )
    {
        return pos;
    }
    else
    {
        CvScalar sc;
        int type;

        type = CV_MAT_TYPE( idx->type );
        cvRawDataToScalar( idx->data.ptr + pos *
            ( (idx->rows == 1) ? CV_ELEM_SIZE( type ) : idx->step ), type, &sc );

        return (int) sc.val[0];
    }
}

CV_IMPL
void cvReleaseStumpClassifier( CvClassifier** classifier )
{
    cvFree( (void**) classifier );
    *classifier = 0;
}

CV_IMPL
float cvEvalStumpClassifier( const CvClassifier* classifier,
                             const CvMat* sample, CvMat* )
{
    assert( classifier != NULL );
    assert( sample != NULL );
    assert( CV_MAT_TYPE( sample->type ) == CV_32FC1 );
    
    if( (CV_MAT_ELEM( (*sample), float, 0,
            ((CvStumpClassifier*) classifier)->compidx )) <
        ((CvStumpClassifier*) classifier)->threshold ) 
    {
        return ((CvStumpClassifier*) classifier)->left;
    }
    else
    {
        return ((CvStumpClassifier*) classifier)->right;
    }
}

#define ICV_DEF_FIND_STUMP_THRESHOLD( suffix, type, error )                              \
CV_IMPL int icvFindStumpThreshold_##suffix(                                              \
        uchar* data, int datastep,                                                       \
        uchar* wdata, int wstep,                                                         \
        uchar* ydata, int ystep,                                                         \
        uchar* idxdata, int idxstep, int num,                                            \
        float* lerror,                                                                   \
        float* rerror,                                                                   \
        float* threshold, float* left, float* right,                                     \
        float* sumw, float* sumwy, float* sumwyy )                                       \
{                                                                                        \
    int found = 0;                                                                       \
    float wyl  = 0.0F;                                                                   \
    float wl   = 0.0F;                                                                   \
    float wyyl = 0.0F;                                                                   \
    float wyr  = 0.0F;                                                                   \
    float wr   = 0.0F;                                                                   \
                                                                                         \
    float curleft  = 0.0F;                                                               \
    float curright = 0.0F;                                                               \
    float* prevval = NULL;                                                               \
    float* curval  = NULL;                                                               \
    float curlerror = 0.0F;                                                              \
    float currerror = 0.0F;                                                              \
    float wposl;                                                                         \
    float wposr;                                                                         \
                                                                                         \
    int i = 0;                                                                           \
    int idx = 0;                                                                         \
                                                                                         \
    wposl = wposr = 0.0F;                                                                \
    if( *sumw == FLT_MAX )                                                               \
    {                                                                                    \
        /* calculate sums */                                                             \
        float *y = NULL;                                                                 \
        float *w = NULL;                                                                 \
        float wy = 0.0F;                                                                 \
                                                                                         \
        *sumw   = 0.0F;                                                                  \
        *sumwy  = 0.0F;                                                                  \
        *sumwyy = 0.0F;                                                                  \
        for( i = 0; i < num; i++ )                                                       \
        {                                                                                \
            idx = (int) ( *((type*) (idxdata + i*idxstep)) );                            \
            w = (float*) (wdata + idx * wstep);                                          \
            *sumw += *w;                                                                 \
            y = (float*) (ydata + idx * ystep);                                          \
            wy = (*w) * (*y);                                                            \
            *sumwy += wy;                                                                \
            *sumwyy += wy * (*y);                                                        \
        }                                                                                \
    }                                                                                    \
                                                                                         \
    for( i = 0; i < num; i++ )                                                           \
    {                                                                                    \
        idx = (int) ( *((type*) (idxdata + i*idxstep)) );                                \
        curval = (float*) (data + idx * datastep);                                       \
         /* for debug purpose */                                                         \
        if( i > 0 ) assert( (*prevval) <= (*curval) );                                   \
                                                                                         \
        wyr  = *sumwy - wyl;                                                             \
        wr   = *sumw  - wl;                                                              \
                                                                                         \
        if( wl > 0.0 ) curleft = wyl / wl;                                               \
        else curleft = 0.0F;                                                             \
                                                                                         \
        if( wr > 0.0 ) curright = wyr / wr;                                              \
        else curright = 0.0F;                                                            \
                                                                                         \
        error                                                                            \
                                                                                         \
        if( curlerror + currerror < (*lerror) + (*rerror) )                              \
        {                                                                                \
            (*lerror) = curlerror;                                                       \
            (*rerror) = currerror;                                                       \
            *threshold = *curval;                                                        \
            if( i > 0 ) {                                                                \
                *threshold = 0.5F * (*threshold + *prevval);                             \
            }                                                                            \
            *left  = curleft;                                                            \
            *right = curright;                                                           \
            found = 1;                                                                   \
        }                                                                                \
                                                                                         \
        do                                                                               \
        {                                                                                \
            wl  += *((float*) (wdata + idx * wstep));                                    \
            wyl += (*((float*) (wdata + idx * wstep)))                                   \
                * (*((float*) (ydata + idx * ystep)));                                   \
            wyyl += *((float*) (wdata + idx * wstep))                                    \
                * (*((float*) (ydata + idx * ystep)))                                    \
                * (*((float*) (ydata + idx * ystep)));                                   \
        }                                                                                \
        while( (++i) < num &&                                                            \
            ( *((float*) (data + (idx =                                                  \
                (int) ( *((type*) (idxdata + i*idxstep))) ) * datastep))                 \
                == *curval ) );                                                          \
        --i;                                                                             \
        prevval = curval;                                                                \
    } /* for each value */                                                               \
                                                                                         \
    return found;                                                                        \
}

/* misclassification error
 * err = MIN( wpos, wneg );
 */
#define ICV_DEF_FIND_STUMP_THRESHOLD_MISC( suffix, type )                                \
    ICV_DEF_FIND_STUMP_THRESHOLD( misc_##suffix, type,                                   \
        wposl = 0.5F * ( wl + wyl );                                                     \
        wposr = 0.5F * ( wr + wyr );                                                     \
        curleft = 0.5F * ( 1.0F + curleft );                                             \
        curright = 0.5F * ( 1.0F + curright );                                           \
        curlerror = MIN( wposl, wl - wposl );                                            \
        currerror = MIN( wposr, wr - wposr );                                            \
    )

/* gini error
 * err = 2 * wpos * wneg /(wpos + wneg)
 */
#define ICV_DEF_FIND_STUMP_THRESHOLD_GINI( suffix, type )                                \
    ICV_DEF_FIND_STUMP_THRESHOLD( gini_##suffix, type,                                   \
        wposl = 0.5F * ( wl + wyl );                                                     \
        wposr = 0.5F * ( wr + wyr );                                                     \
        curleft = 0.5F * ( 1.0F + curleft );                                             \
        curright = 0.5F * ( 1.0F + curright );                                           \
        curlerror = 2.0F * wposl * ( 1.0F - curleft );                                   \
        currerror = 2.0F * wposr * ( 1.0F - curright );                                  \
    )

#define CV_ENTROPY_THRESHOLD FLT_MIN

/* entropy error
 * err = - wpos * log(wpos / (wpos + wneg)) - wneg * log(wneg / (wpos + wneg))
 */
#define ICV_DEF_FIND_STUMP_THRESHOLD_ENTROPY( suffix, type )                             \
    ICV_DEF_FIND_STUMP_THRESHOLD( entropy_##suffix, type,                                \
        wposl = 0.5F * ( wl + wyl );                                                     \
        wposr = 0.5F * ( wr + wyr );                                                     \
        curleft = 0.5F * ( 1.0F + curleft );                                             \
        curright = 0.5F * ( 1.0F + curright );                                           \
        curlerror = currerror = 0.0F;                                                    \
        if( curleft > CV_ENTROPY_THRESHOLD )                                             \
            curlerror -= wposl * logf( curleft );                                        \
        if( curleft < 1.0F - CV_ENTROPY_THRESHOLD )                                      \
            curlerror -= (wl - wposl) * logf( 1.0F - curleft );                          \
                                                                                         \
        if( curright > CV_ENTROPY_THRESHOLD )                                            \
            currerror -= wposr * logf( curright );                                       \
        if( curright < 1.0F - CV_ENTROPY_THRESHOLD )                                     \
            currerror -= (wr - wposr) * logf( 1.0F - curright );                         \
    )

/* least sum of squares error */
#define ICV_DEF_FIND_STUMP_THRESHOLD_SQ( suffix, type )                                  \
    ICV_DEF_FIND_STUMP_THRESHOLD( sq_##suffix, type,                                     \
        /* calculate error (sum of squares)          */                                  \
        /* err = sum( w * (y - left(rigt)Val)^2 )    */                                  \
        curlerror = wyyl + curleft * curleft * wl - 2.0F * curleft * wyl;                \
        currerror = (*sumwyy) - wyyl + curright * curright * wr - 2.0F * curright * wyr; \
    )

ICV_DEF_FIND_STUMP_THRESHOLD_MISC( 16s, short )

ICV_DEF_FIND_STUMP_THRESHOLD_MISC( 32s, int )

ICV_DEF_FIND_STUMP_THRESHOLD_MISC( 32f, float )


ICV_DEF_FIND_STUMP_THRESHOLD_GINI( 16s, short )

ICV_DEF_FIND_STUMP_THRESHOLD_GINI( 32s, int )

ICV_DEF_FIND_STUMP_THRESHOLD_GINI( 32f, float )


ICV_DEF_FIND_STUMP_THRESHOLD_ENTROPY( 16s, short )

ICV_DEF_FIND_STUMP_THRESHOLD_ENTROPY( 32s, int )

ICV_DEF_FIND_STUMP_THRESHOLD_ENTROPY( 32f, float )


ICV_DEF_FIND_STUMP_THRESHOLD_SQ( 16s, short )

ICV_DEF_FIND_STUMP_THRESHOLD_SQ( 32s, int )

ICV_DEF_FIND_STUMP_THRESHOLD_SQ( 32f, float )

typedef int (*CvFindThresholdFunc)( uchar* data, int datastep,
                                    uchar* wdata, int wstep,
                                    uchar* ydata, int ystep,
                                    uchar* idxdata, int idxstep, int num,
                                    float* lerror,
                                    float* rerror,
                                    float* threshold, float* left, float* right,
                                    float* sumw, float* sumwy, float* sumwyy );

CvFindThresholdFunc findStumpThreshold_16s[4] = {
        icvFindStumpThreshold_misc_16s,
        icvFindStumpThreshold_gini_16s,
        icvFindStumpThreshold_entropy_16s,
        icvFindStumpThreshold_sq_16s
    };

CvFindThresholdFunc findStumpThreshold_32s[4] = {
        icvFindStumpThreshold_misc_32s,
        icvFindStumpThreshold_gini_32s,
        icvFindStumpThreshold_entropy_32s,
        icvFindStumpThreshold_sq_32s
    };

CvFindThresholdFunc findStumpThreshold_32f[4] = {
        icvFindStumpThreshold_misc_32f,
        icvFindStumpThreshold_gini_32f,
        icvFindStumpThreshold_entropy_32f,
        icvFindStumpThreshold_sq_32f
    };

CV_IMPL
CvClassifier* cvCreateStumpClassifier( CvMat* trainData,
                     int flags,
                     CvMat* trainClasses,
                     CvMat* /*typeMask*/,
                     CvMat* /*missedMeasurementsMask*/,
                     CvMat* /*compIdx*/,
                     CvMat* sampleIdx,
                     CvMat* weights,
                     CvClassifierTrainParams* trainParams
                   )
{
    CvStumpClassifier* stump = NULL;
    int m = 0; /* number of samples */
    int n = 0; /* number of components */
    uchar* data = NULL;
    int cstep   = 0;
    int sstep   = 0;
    uchar* ydata = NULL;
    int ystep    = 0;
    uchar* idxdata = NULL;
    int idxstep    = 0;
    int l = 0; /* number of indices */     
    uchar* wdata = NULL;
    int wstep    = 0;

    int* idx = NULL;
    int i = 0;
    
    float sumw   = FLT_MAX;
    float sumwy  = FLT_MAX;
    float sumwyy = FLT_MAX;

    assert( trainData != NULL );
    assert( CV_MAT_TYPE( trainData->type ) == CV_32FC1 );
    assert( trainClasses != NULL );
    assert( CV_MAT_TYPE( trainClasses->type ) == CV_32FC1 );
    //assert( missedMeasurementsMask == NULL );
    //assert( compIdx == NULL );
    assert( weights != NULL );
    assert( CV_MAT_TYPE( weights->type ) == CV_32FC1 );
    assert( trainParams != NULL );
    //typeMask; compIdx; missedMeasurementsMask;

    data = trainData->data.ptr;
    if( CV_IS_ROW_SAMPLE( flags ) )
    {
        cstep = CV_ELEM_SIZE( trainData->type );
        sstep = trainData->step;
        m = trainData->rows;
        n = trainData->cols;
    }
    else
    {
        sstep = CV_ELEM_SIZE( trainData->type );
        cstep = trainData->step;
        m = trainData->cols;
        n = trainData->rows;
    }

    ydata = trainClasses->data.ptr;
    if( trainClasses->rows == 1 )
    {
        assert( trainClasses->cols == m );
        ystep = CV_ELEM_SIZE( trainClasses->type );
    }
    else
    {
        assert( trainClasses->rows == m );
        ystep = trainClasses->step;
    }

    wdata = weights->data.ptr;
    if( weights->rows == 1 )
    {
        assert( weights->cols == m );
        wstep = CV_ELEM_SIZE( weights->type );
    }
    else
    {
        assert( weights->rows == m );
        wstep = weights->step;
    }

    l = m;
    if( sampleIdx != NULL )
    {
        assert( CV_MAT_TYPE( sampleIdx->type ) == CV_32FC1 );

        idxdata = sampleIdx->data.ptr;
        if( sampleIdx->rows == 1 )
        {
            l = sampleIdx->cols;
            idxstep = CV_ELEM_SIZE( sampleIdx->type );
        }
        else
        {
            l = sampleIdx->rows;
            idxstep = sampleIdx->step;
        }
        assert( l <= m );
    }

    idx = (int*) cvAlloc( l * sizeof( int ) );
    stump = (CvStumpClassifier*) cvAlloc( sizeof( CvStumpClassifier) );

    /* START */
    memset( (void*) stump, 0, sizeof( CvStumpClassifier ) );

    stump->predict = cvEvalStumpClassifier;
    stump->update = NULL;
    //stump->save = NULL;
    stump->release = cvReleaseStumpClassifier;

    stump->lerror = FLT_MAX;
    stump->rerror = FLT_MAX;
    stump->left  = 0.0F;
    stump->right = 0.0F;

    /* copy indices */
    if( sampleIdx != NULL )
    {
        for( i = 0; i < l; i++ )
        {
            idx[i] = (int) *((float*) (idxdata + i*idxstep));
        }
    }
    else
    {
        for( i = 0; i < l; i++ )
        {
            idx[i] = i;
        }
    }

    for( i = 0; i < n; i++ )
    {
        CvValArray va;

        va.data = data + i * cstep;
        va.step = sstep;
        icvSortIndexedValArray_32s( idx, l, &va );
        if( findStumpThreshold_32s[(int) ((CvStumpTrainParams*) trainParams)->error]
              ( data + i * cstep, sstep,
                wdata, wstep, ydata, ystep, (uchar*) idx, sizeof( int ), l,
                &(stump->lerror), &(stump->rerror),
                &(stump->threshold), &(stump->left), &(stump->right), 
                &sumw, &sumwy, &sumwyy ) )
        {
            stump->compidx = i;
        }
    } /* for each component */

    /* END */

    cvFree( (void**) &idx );

    if( ((CvStumpTrainParams*) trainParams)->type == CV_CLASSIFICATION_CLASS )
    {
        stump->left = 2.0F * (stump->left >= 0.5F) - 1.0F;
        stump->right = 2.0F * (stump->right >= 0.5F) - 1.0F;
    }

    return (CvClassifier*) stump;
}

/*
 * cvCreateMTStumpClassifier
 *
 * Multithreaded stump classifier constructor
 * Includes huge train data support through callback function
 */
CV_IMPL
CvClassifier* cvCreateMTStumpClassifier( CvMat* trainData,
                      int flags,
                      CvMat* trainClasses,
                      CvMat* /*typeMask*/,
                      CvMat* /*missedMeasurementsMask*/,
                      CvMat* compIdx,
                      CvMat* sampleIdx,
                      CvMat* weights,
                      CvClassifierTrainParams* trainParams )
{
    CvStumpClassifier* stump = NULL;
    int m = 0; /* number of samples */
    int n = 0; /* number of components */
    uchar* data = NULL;
    int cstep   = 0;
    int sstep   = 0;
    int datan   = 0; /* num components */
    uchar* ydata = NULL;
    int ystep    = 0;
    uchar* idxdata = NULL;
    int idxstep    = 0;
    int l = 0; /* number of indices */     
    uchar* wdata = NULL;
    int wstep    = 0;

    uchar* sorteddata = NULL;
    int sortedtype    = 0;
    int sortedcstep   = 0; /* component step */
    int sortedsstep   = 0; /* sample step */
    int sortedn       = 0; /* num components */
    int sortedm       = 0; /* num samples */

    char* filter = NULL;
    int i = 0;
    
    int compidx = 0;
    int stumperror;
    int portion;

    /* private variables */
    CvMat mat;
    CvValArray va;
    float lerror;
    float rerror;
    float left;
    float right;
    float threshold;
    int optcompidx;

    float sumw;
    float sumwy;
    float sumwyy;

    int t_compidx;
    int t_n;
    
    int ti;
    int tj;
    int tk;

    uchar* t_data;
    int t_cstep;
    int t_sstep;

    int matcstep;
    int matsstep;

    int* t_idx;
    /* end private variables */

    assert( trainParams != NULL );
    assert( trainClasses != NULL );
    assert( CV_MAT_TYPE( trainClasses->type ) == CV_32FC1 );
    //assert( missedMeasurementsMask == NULL );
    //assert( compIdx == NULL );
    //typeMask; missedMeasurementsMask;

    stumperror = (int) ((CvMTStumpTrainParams*) trainParams)->error;

    ydata = trainClasses->data.ptr;
    if( trainClasses->rows == 1 )
    {
        m = trainClasses->cols;
        ystep = CV_ELEM_SIZE( trainClasses->type );
    }
    else
    {
        m = trainClasses->rows;
        ystep = trainClasses->step;
    }

    wdata = weights->data.ptr;
    if( weights->rows == 1 )
    {
        assert( weights->cols == m );
        wstep = CV_ELEM_SIZE( weights->type );
    }
    else
    {
        assert( weights->rows == m );
        wstep = weights->step;
    }

    if( ((CvMTStumpTrainParams*) trainParams)->sortedIdx != NULL )
    {
        sortedtype =
            CV_MAT_TYPE( ((CvMTStumpTrainParams*) trainParams)->sortedIdx->type );
        assert( sortedtype == CV_16SC1 || sortedtype == CV_32SC1
                || sortedtype == CV_32FC1 );
        sorteddata = ((CvMTStumpTrainParams*) trainParams)->sortedIdx->data.ptr;
        sortedsstep = CV_ELEM_SIZE( sortedtype );
        sortedcstep = ((CvMTStumpTrainParams*) trainParams)->sortedIdx->step;
        sortedn = ((CvMTStumpTrainParams*) trainParams)->sortedIdx->rows;
        sortedm = ((CvMTStumpTrainParams*) trainParams)->sortedIdx->cols;
    }

    if( trainData == NULL )
    {
        assert( ((CvMTStumpTrainParams*) trainParams)->getTrainData != NULL );
        n = ((CvMTStumpTrainParams*) trainParams)->numcomp;
        assert( n > 0 );
    }
    else
    {
        assert( CV_MAT_TYPE( trainData->type ) == CV_32FC1 );
        data = trainData->data.ptr;
        if( CV_IS_ROW_SAMPLE( flags ) )
        {
            cstep = CV_ELEM_SIZE( trainData->type );
            sstep = trainData->step;
            assert( m == trainData->rows );
            datan = n = trainData->cols;
        }
        else
        {
            sstep = CV_ELEM_SIZE( trainData->type );
            cstep = trainData->step;
            assert( m == trainData->cols );
            datan = n = trainData->rows;
        }
        if( ((CvMTStumpTrainParams*) trainParams)->getTrainData != NULL )
        {
            n = ((CvMTStumpTrainParams*) trainParams)->numcomp;
        }        
    }
    assert( datan <= n );

    if( sampleIdx != NULL )
    {
        assert( CV_MAT_TYPE( sampleIdx->type ) == CV_32FC1 );
        idxdata = sampleIdx->data.ptr;
        idxstep = ( sampleIdx->rows == 1 )
            ? CV_ELEM_SIZE( sampleIdx->type ) : sampleIdx->step;
        l = ( sampleIdx->rows == 1 ) ? sampleIdx->cols : sampleIdx->rows;

        if( sorteddata != NULL )
        {
            filter = (char*) cvAlloc( sizeof( char ) * m );
            memset( (void*) filter, 0, sizeof( char ) * m );
            for( i = 0; i < l; i++ )
            {
                filter[(int) *((float*) (idxdata + i * idxstep))] = (char) 1;
            }
        }
    }
    else
    {
        l = m;
    }

    stump = (CvStumpClassifier*) cvAlloc( sizeof( CvStumpClassifier) );

    /* START */
    memset( (void*) stump, 0, sizeof( CvStumpClassifier ) );

    portion = ((CvMTStumpTrainParams*)trainParams)->portion;
    
    if( portion < 1 )
    {
        /* auto portion */
        portion = n;
        #ifdef _OPENMP
        portion /= omp_get_max_threads();        
        #endif /* _OPENMP */        
    }

    stump->predict = cvEvalStumpClassifier;
    stump->update = NULL;
    //stump->save = NULL;
    stump->release = cvReleaseStumpClassifier;

    stump->lerror = FLT_MAX;
    stump->rerror = FLT_MAX;
    stump->left  = 0.0F;
    stump->right = 0.0F;

    compidx = 0;
    #ifdef _OPENMP
    #pragma omp parallel private(mat, va, lerror, rerror, left, right, threshold, \
                                 optcompidx, sumw, sumwy, sumwyy, t_compidx, t_n, \
                                 ti, tj, tk, t_data, t_cstep, t_sstep, matcstep, \
                                 matsstep, t_idx)
    #endif /* _OPENMP */
    {
        lerror = FLT_MAX;
        rerror = FLT_MAX;
        left  = 0.0F;
        right = 0.0F;
        threshold = 0.0F;
        optcompidx = 0;

        sumw   = FLT_MAX;
        sumwy  = FLT_MAX;
        sumwyy = FLT_MAX;

        t_compidx = 0;
        t_n = 0;
        
        ti = 0;
        tj = 0;
        tk = 0;

        t_data = NULL;
        t_cstep = 0;
        t_sstep = 0;

        matcstep = 0;
        matsstep = 0;

        t_idx = NULL;

        mat.data.ptr = NULL;
        
        if( datan < n )
        {
            /* prepare matrix for callback */
            if( CV_IS_ROW_SAMPLE( flags ) )
            {
                mat = cvMat( m, portion, CV_32FC1, 0 );
                matcstep = CV_ELEM_SIZE( mat.type );
                matsstep = mat.step;
            }
            else
            {
                mat = cvMat( portion, m, CV_32FC1, 0 );
                matcstep = mat.step;
                matsstep = CV_ELEM_SIZE( mat.type );
            }
            mat.data.ptr = (uchar*) cvAlloc( sizeof( float ) * mat.rows * mat.cols );
        }

        if( filter != NULL || sortedn < n )
        {
            t_idx = (int*) cvAlloc( sizeof( int ) * m );
            if( sortedn == 0 || filter == NULL )
            {
                if( idxdata != NULL )
                {
                    for( ti = 0; ti < l; ti++ )
                    {
                        t_idx[ti] = (int) *((float*) (idxdata + ti * idxstep));
                    }
                }
                else
                {
                    for( ti = 0; ti < l; ti++ )
                    {
                        t_idx[ti] = ti;
                    }
                }                
            }
        }

        #ifdef _OPENMP
        #pragma omp critical(c_compidx)
        #endif /* _OPENMP */
        {
            t_compidx = compidx;
            compidx += portion;
        }
        while( t_compidx < n )
        {
            t_n = portion;
            if( t_compidx < datan )
            {
                t_n = ( t_n < (datan - t_compidx) ) ? t_n : (datan - t_compidx);
                t_data = data;
                t_cstep = cstep;
                t_sstep = sstep;
            }
            else
            {
                t_n = ( t_n < (n - t_compidx) ) ? t_n : (n - t_compidx);
                t_cstep = matcstep;
                t_sstep = matsstep;
                t_data = mat.data.ptr - t_compidx * t_cstep;

                /* calculate components */
                ((CvMTStumpTrainParams*)trainParams)->getTrainData( &mat,
                        sampleIdx, compIdx, t_compidx, t_n,
                        ((CvMTStumpTrainParams*)trainParams)->userdata );
            }

            if( sorteddata != NULL )
            {
                if( filter != NULL )
                {
                    /* have sorted indices and filter */
                    switch( sortedtype )
                    {
                        case CV_16SC1:
                            for( ti = t_compidx; ti < MIN( sortedn, t_compidx + t_n ); ti++ )
                            {
                                tk = 0;
                                for( tj = 0; tj < sortedm; tj++ )
                                {
                                    int curidx = (int) ( *((short*) (sorteddata
                                            + ti * sortedcstep + tj * sortedsstep)) );
                                    if( filter[curidx] != 0 )
                                    {
                                        t_idx[tk++] = curidx;
                                    }
                                }
                                if( findStumpThreshold_32s[stumperror]( 
                                        t_data + ti * t_cstep, t_sstep,
                                        wdata, wstep, ydata, ystep,
                                        (uchar*) t_idx, sizeof( int ), tk,
                                        &lerror, &rerror,
                                        &threshold, &left, &right, 
                                        &sumw, &sumwy, &sumwyy ) )
                                {
                                    optcompidx = ti;
                                }
                            }
                            break;
                        case CV_32SC1:
                            for( ti = t_compidx; ti < MIN( sortedn, t_compidx + t_n ); ti++ )
                            {
                                tk = 0;
                                for( tj = 0; tj < sortedm; tj++ )
                                {
                                    int curidx = (int) ( *((int*) (sorteddata
                                            + ti * sortedcstep + tj * sortedsstep)) );
                                    if( filter[curidx] != 0 )
                                    {
                                        t_idx[tk++] = curidx;
                                    }
                                }
                                if( findStumpThreshold_32s[stumperror]( 
                                        t_data + ti * t_cstep, t_sstep,
                                        wdata, wstep, ydata, ystep,
                                        (uchar*) t_idx, sizeof( int ), tk,
                                        &lerror, &rerror,
                                        &threshold, &left, &right, 
                                        &sumw, &sumwy, &sumwyy ) )
                                {
                                    optcompidx = ti;
                                }
                            }
                            break;
                        case CV_32FC1:
                            for( ti = t_compidx; ti < MIN( sortedn, t_compidx + t_n ); ti++ )
                            {
                                tk = 0;
                                for( tj = 0; tj < sortedm; tj++ )
                                {
                                    int curidx = (int) ( *((float*) (sorteddata
                                            + ti * sortedcstep + tj * sortedsstep)) );
                                    if( filter[curidx] != 0 )
                                    {
                                        t_idx[tk++] = curidx;
                                    }
                                }
                                if( findStumpThreshold_32s[stumperror]( 
                                        t_data + ti * t_cstep, t_sstep,
                                        wdata, wstep, ydata, ystep,
                                        (uchar*) t_idx, sizeof( int ), tk,
                                        &lerror, &rerror,
                                        &threshold, &left, &right, 
                                        &sumw, &sumwy, &sumwyy ) )
                                {
                                    optcompidx = ti;
                                }
                            }
                            break;
                        default:
                            assert( 0 );
                            break;
                    }
                }
                else
                {
                    /* have sorted indices */
                    switch( sortedtype )
                    {
                        case CV_16SC1:
                            for( ti = t_compidx; ti < MIN( sortedn, t_compidx + t_n ); ti++ )
                            {
                                if( findStumpThreshold_16s[stumperror]( 
                                        t_data + ti * t_cstep, t_sstep,
                                        wdata, wstep, ydata, ystep,
                                        sorteddata + ti * sortedcstep, sortedsstep, sortedm,
                                        &lerror, &rerror,
                                        &threshold, &left, &right, 
                                        &sumw, &sumwy, &sumwyy ) )
                                {
                                    optcompidx = ti;
                                }
                            }
                            break;
                        case CV_32SC1:
                            for( ti = t_compidx; ti < MIN( sortedn, t_compidx + t_n ); ti++ )
                            {
                                if( findStumpThreshold_32s[stumperror]( 
                                        t_data + ti * t_cstep, t_sstep,
                                        wdata, wstep, ydata, ystep,
                                        sorteddata + ti * sortedcstep, sortedsstep, sortedm,
                                        &lerror, &rerror,
                                        &threshold, &left, &right, 
                                        &sumw, &sumwy, &sumwyy ) )
                                {
                                    optcompidx = ti;
                                }
                            }
                            break;
                        case CV_32FC1:
                            for( ti = t_compidx; ti < MIN( sortedn, t_compidx + t_n ); ti++ )
                            {
                                if( findStumpThreshold_32f[stumperror]( 
                                        t_data + ti * t_cstep, t_sstep,
                                        wdata, wstep, ydata, ystep,
                                        sorteddata + ti * sortedcstep, sortedsstep, sortedm,
                                        &lerror, &rerror,
                                        &threshold, &left, &right, 
                                        &sumw, &sumwy, &sumwyy ) )
                                {
                                    optcompidx = ti;
                                }
                            }
                            break;
                        default:
                            assert( 0 );
                            break;
                    }
                }
            }

            ti = MAX( t_compidx, MIN( sortedn, t_compidx + t_n ) );
            for( ; ti < t_compidx + t_n; ti++ )
            {
                va.data = t_data + ti * t_cstep;
                va.step = t_sstep;
                icvSortIndexedValArray_32s( t_idx, l, &va );
                if( findStumpThreshold_32s[stumperror]( 
                        t_data + ti * t_cstep, t_sstep,
                        wdata, wstep, ydata, ystep,
                        (uchar*)t_idx, sizeof( int ), l,
                        &lerror, &rerror,
                        &threshold, &left, &right, 
                        &sumw, &sumwy, &sumwyy ) )
                {
                    optcompidx = ti;
                }
            }
            #ifdef _OPENMP
            #pragma omp critical(c_compidx)
            #endif /* _OPENMP */
            {
                t_compidx = compidx;
                compidx += portion;
            }
        } /* while have training data */

        /* get the best classifier */
        #ifdef _OPENMP
        #pragma omp critical(c_beststump)
        #endif /* _OPENMP */
        {
            if( lerror + rerror < stump->lerror + stump->rerror )
            {
                stump->lerror    = lerror;
                stump->rerror    = rerror;
                stump->compidx   = optcompidx;
                stump->threshold = threshold;
                stump->left      = left;
                stump->right     = right;
            }
        }

        /* free allocated memory */
        if( mat.data.ptr != NULL )
        {
            cvFree( (void**) &(mat.data.ptr) );
        }
        if( t_idx != NULL )
        {
            cvFree( (void**) &t_idx );
        }
    } /* end of parallel region */

    /* END */

    /* free allocated memory */
    if( filter != NULL )
    {
        cvFree( (void**) &filter );
    }

    if( ((CvMTStumpTrainParams*) trainParams)->type == CV_CLASSIFICATION_CLASS )
    {
        stump->left = 2.0F * (stump->left >= 0.5F) - 1.0F;
        stump->right = 2.0F * (stump->right >= 0.5F) - 1.0F;
    }

    return (CvClassifier*) stump;
}

CV_IMPL
float cvEvalTREEBOOSTClassifier( const CvClassifier* classifier,
                                 const CvMat* sample, CvMat* )
{
    CV_FUNCNAME( "cvEvalTREEBOOSTClassifier" );

    int idx;

    idx = 0;
    __BEGIN__;


    CV_ASSERT( classifier != NULL );
    CV_ASSERT( sample != NULL );
    CV_ASSERT( CV_MAT_TYPE( sample->type ) == CV_32FC1 );
    CV_ASSERT( sample->rows == 1 || sample->cols == 1 );

    if( sample->rows == 1 )
    {
        do
        {
            if( (CV_MAT_ELEM( (*sample), float, 0,
                    ((CvTreeBoostClassifier*) classifier)->comp_idx[idx] )) <
                ((CvTreeBoostClassifier*) classifier)->threshold[idx] ) 
            {
                idx = ((CvTreeBoostClassifier*) classifier)->left[idx];
            }
            else
            {
                idx = ((CvTreeBoostClassifier*) classifier)->right[idx];
            }
        } while( idx > 0 );
    }
    else
    {
        do
        {
            if( (CV_MAT_ELEM( (*sample), float,
                    ((CvTreeBoostClassifier*) classifier)->comp_idx[idx], 0 )) <
                ((CvTreeBoostClassifier*) classifier)->threshold[idx] ) 
            {
                idx = ((CvTreeBoostClassifier*) classifier)->left[idx];
            }
            else
            {
                idx = ((CvTreeBoostClassifier*) classifier)->right[idx];
            }
        } while( idx > 0 );
    } 

    __END__;

    return ((CvTreeBoostClassifier*) classifier)->val[-idx];
}

CV_IMPL
float cvEvalTREEBOOSTClassifierIdx( const CvClassifier* classifier, const CvMat* sample )
{
    CV_FUNCNAME( "cvEvalTREEBOOSTClassifierIdx" );

    int idx;

    idx = 0;
    __BEGIN__;


    CV_ASSERT( classifier != NULL );
    CV_ASSERT( sample != NULL );
    CV_ASSERT( CV_MAT_TYPE( sample->type ) == CV_32FC1 );
    CV_ASSERT( sample->rows == 1 || sample->cols == 1 );

    if( sample->rows == 1 )
    {
        do
        {
            if( (CV_MAT_ELEM( (*sample), float, 0,
                    ((CvTreeBoostClassifier*) classifier)->comp_idx[idx] )) <
                ((CvTreeBoostClassifier*) classifier)->threshold[idx] ) 
            {
                idx = ((CvTreeBoostClassifier*) classifier)->left[idx];
            }
            else
            {
                idx = ((CvTreeBoostClassifier*) classifier)->right[idx];
            }
        } while( idx > 0 );
    }
    else
    {
        do
        {
            if( (CV_MAT_ELEM( (*sample), float,
                    ((CvTreeBoostClassifier*) classifier)->comp_idx[idx], 0 )) <
                ((CvTreeBoostClassifier*) classifier)->threshold[idx] ) 
            {
                idx = ((CvTreeBoostClassifier*) classifier)->left[idx];
            }
            else
            {
                idx = ((CvTreeBoostClassifier*) classifier)->right[idx];
            }
        } while( idx > 0 );
    } 

    __END__;

    return (float) (-idx);
}

CV_IMPL
void cvReleaseTREEBOOSTClassifier( CvStatModel** classifier )
{
    CV_FUNCNAME( "cvReleaseTREEBOOSTClassifier" );

    __BEGIN__;

    /* TODO: add type check */
    
    CV_CALL( cvFree( (void**) classifier ) );
    if( classifier )
        *classifier = NULL;

    __END__;
}

CvTreeBoostClassifier* icvAllocTREEBOOSTClassifier( int num_internal_nodes )
{
    CvTreeBoostClassifier* ptr = NULL;

    CV_FUNCNAME( "icvAllocTREEBOOSTClassifier" );

    __BEGIN__;
    size_t header_size = 0;

    if( num_internal_nodes < 1 )
        CV_ERROR( CV_StsBadArg, "Number of internal nodes must be positive" );

    header_size = sizeof( *ptr ) + num_internal_nodes *
        (sizeof( *ptr->comp_idx ) + sizeof( *ptr->threshold ) +
         sizeof( *ptr->left ) + sizeof( *ptr->right )) +
        (num_internal_nodes + 1) * sizeof( *ptr->val );

    CV_CALL( ptr = (CvTreeBoostClassifier*) cvCreateStatModel( CV_STAT_MODEL_MAGIC_VAL,
        header_size, cvReleaseTREEBOOSTClassifier, cvEvalTREEBOOSTClassifier, NULL ));

    ptr->count = num_internal_nodes;
    
    ptr->comp_idx = (int*) (ptr + 1);
    ptr->threshold = (float*) (ptr->comp_idx + num_internal_nodes);
    ptr->left = (int*) (ptr->threshold + num_internal_nodes);
    ptr->right = (int*) (ptr->left + num_internal_nodes);
    ptr->val = (float*) (ptr->right + num_internal_nodes);
    
    __END__;

    if( cvGetErrStatus() < 0 )
        cvReleaseTREEBOOSTClassifier( (CvStatModel**) &ptr );

    return ptr;
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

/* internal structure used in TREEBOOST creation */
typedef struct CvTREEBOOSTNode
{
    CvMat* sampleIdx;
    CvStumpClassifier* stump;
    int parent;
    int leftflag;
    float errdrop;
} CvTREEBOOSTNode;

CV_IMPL
CvClassifier* cvCreateTREEBOOSTClassifier( CvMat* trainData,
                     int flags,
                     CvMat* trainClasses,
                     CvMat* typeMask,
                      CvMat* missedMeasurementsMask,
                      CvMat* compIdx,
                      CvMat* sampleIdx,
                      CvMat* weights,
                      CvClassifierTrainParams* trainParams )
{
    CvTreeBoostClassifier* TREEBOOST = NULL;

    CV_FUNCNAME( "cvCreateTREEBOOSTClassifier" );

    __BEGIN__;

    size_t datasize = 0;
    int count = 0;
    int i = 0;
    int j = 0;
    
    CvTREEBOOSTNode* intnode = NULL;
    CvTREEBOOSTNode* list = NULL;
    int listcount = 0;
    CvMat* lidx = NULL;
    CvMat* ridx = NULL;
    
    float maxerrdrop = 0.0F;
    int idx = 0;

    void (*splitIdxCallback)( int compidx, float threshold,
                              CvMat* idx, CvMat** left, CvMat** right,
                              void* userdata );
    void* userdata;

    count = ((CvTREEBOOSTTrainParams*) trainParams)->count;
    
    assert( count > 0 );

    CV_CALL( TREEBOOST = icvAllocTREEBOOSTClassifier( count ) );

    datasize = sizeof( CvTREEBOOSTNode ) * (count + count);
    intnode = (CvTREEBOOSTNode*) cvAlloc( datasize );
    memset( intnode, 0, datasize );
    list = (CvTREEBOOSTNode*) (intnode + count);

    splitIdxCallback = ((CvTREEBOOSTTrainParams*) trainParams)->splitIdx;
    userdata = ((CvTREEBOOSTTrainParams*) trainParams)->userdata;
    if( splitIdxCallback == NULL )
    {
        splitIdxCallback = ( CV_IS_ROW_SAMPLE( flags ) )
            ? icvDefaultSplitIdx_R : icvDefaultSplitIdx_C;
        userdata = trainData;
    }

    /* create root of the tree */
    intnode[0].sampleIdx = sampleIdx;
    intnode[0].stump = (CvStumpClassifier*)
        ((CvTREEBOOSTTrainParams*) trainParams)->stumpConstructor( trainData, flags,
            trainClasses, typeMask, missedMeasurementsMask, compIdx, sampleIdx, weights,
            ((CvTREEBOOSTTrainParams*) trainParams)->stumpTrainParams );
    TREEBOOST->left[0] = TREEBOOST->right[0] = 0;

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
                ((CvTREEBOOSTTrainParams*) trainParams)->stumpConstructor( trainData, flags,
                    trainClasses, typeMask, missedMeasurementsMask, compIdx,
                    list[listcount].sampleIdx,
                    weights, ((CvTREEBOOSTTrainParams*) trainParams)->stumpTrainParams );
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
                ((CvTREEBOOSTTrainParams*) trainParams)->stumpConstructor( trainData, flags,
                    trainClasses, typeMask, missedMeasurementsMask, compIdx,
                    list[listcount].sampleIdx,
                    weights, ((CvTREEBOOSTTrainParams*) trainParams)->stumpTrainParams );
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
            TREEBOOST->left[list[idx].parent] = i;
        }
        else
        {
            TREEBOOST->right[list[idx].parent] = i;
        }
        if( idx != (listcount - 1) )
        {
            list[idx] = list[listcount - 1];
        }
        listcount--;
    }

    /* fill <TREEBOOST> fields */
    j = 0;
    TREEBOOST->count = 0;
    for( i = 0; i < count && (intnode[i].stump != NULL); i++ )
    {
        TREEBOOST->count++;
        TREEBOOST->comp_idx[i] = intnode[i].stump->compidx;
        TREEBOOST->threshold[i] = intnode[i].stump->threshold;
        
        /* leaves */
        if( TREEBOOST->left[i] <= 0 )
        {
            TREEBOOST->left[i] = -j;
            TREEBOOST->val[j] = intnode[i].stump->left;
            j++;
        }
        if( TREEBOOST->right[i] <= 0 )
        {
            TREEBOOST->right[i] = -j;
            TREEBOOST->val[j] = intnode[i].stump->right;
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
    
    cvFree( (void**) &intnode );

    __END__;

    return (CvClassifier*) TREEBOOST;
}

/****************************************************************************************\
*                                        Boosting                                        *
\****************************************************************************************/

/* discrete, real, gentle */
void
icvBoostStartTraining( CvMat* responses, CvMat* weak_train_resp, CvMat* /* weights */,
                       CvMat* /* sum_resp */ )
{
    CV_FUNCNAME( "icvBoostStartTraining" );
    __BEGIN__;
    CV_CALL( cvConvertScale( responses, weak_train_resp, 2, -1 ) );
    __END__;
}

double
icvBoostNextWeakClassifierDAB( CvMat* weak_eval, CvMat* responses,
                               CvMat* /* weak_train_resp */, CvMat* weights,
                               CvMat* /* sum_resp */ )
{
    double c = 0;
    //CV_FUNCNAME( "icvBoostNextWeakClassifierDAB" );

    __BEGIN__;
    float sumw;
    float err;
    int i;

    sumw = 0.0F;
    err = 0.0F;
    for( i = 0; i < responses->cols; ++i )
    {
        sumw += CV_MAT_ELEM( *weights, float, 0, i );
        err += CV_MAT_ELEM( *weights, float, 0, i ) *
            ( CV_MAT_ELEM( *weak_eval, float, 0, i ) !=
              (2.0F * CV_MAT_ELEM( *responses, int, 0, i ) - 1.0F) );
    }
    if( sumw != 0 ) err /= sumw;
    err = -cvLogRatio( err );
    
    sumw = 0;
    for( i = 0; i < responses->cols; ++i )
    {
        CV_MAT_ELEM( *weights, float, 0, i ) *= expf( err *
            ( CV_MAT_ELEM( *weak_eval, float, 0, i ) !=
              (2.0F * CV_MAT_ELEM( *responses, int, 0, i ) - 1.0F) ) );

        sumw += CV_MAT_ELEM( *weights, float, 0, i );
    }
    if( sumw != 0 )
        for( i = 0; i < weights->cols; ++i )
            CV_MAT_ELEM( *weights, float, 0, i ) /= sumw;

    c = err;
    __END__;
    
    return c;
}

double
icvBoostNextWeakClassifierRAB( CvMat* weak_eval, CvMat* responses,
                               CvMat* /* weak_train_resp */, CvMat* weights,
                               CvMat* /* sum_resp */ )
{
    double c = 0.5;
    //CV_FUNCNAME( "icvBoostNextWeakClassifierRAB" );

    __BEGIN__;
    float sumw;
    int i;

    sumw = 0;
    for( i = 0; i < responses->cols; ++i )
    {
        CV_MAT_ELEM( *weights, float, 0, i ) *= expf( 
            -(CV_MAT_ELEM( *responses, int, 0, i ) - 0.5F) *
             CV_MAT_ELEM( *weak_eval, float, 0, i ) );

        sumw += CV_MAT_ELEM( *weights, float, 0, i );
    }
    if( sumw != 0 )
        for( i = 0; i < weights->cols; ++i )
            CV_MAT_ELEM( *weights, float, 0, i ) /= sumw;
    __END__;
    
    return c;
}

#define CV_LB_PROB_THRESH      0.01F
#define CV_LB_WEIGHT_THRESHOLD 0.0001F

void
icvResponsesAndWeightsLB( CvMat* responses, CvMat* weak_train_resp, CvMat* weights,
                          CvMat* sum_resp )
{
    //CV_FUNCNAME( "icvResponsesAndWeightsLB" );

    __BEGIN__;

    int i;

    for( i = 0; i < responses->cols; ++i )
    {
        float p = 1.0F / (1.0F + expf( -2.0F * CV_MAT_ELEM( *sum_resp, float, 0, i ) ));
        CV_MAT_ELEM( *weights, float, 0, i ) = 
            MAX( p * (1.0F - p), CV_LB_WEIGHT_THRESHOLD );
        if( CV_MAT_ELEM( *responses, int, 0, i ) == 1 )
        {
            CV_MAT_ELEM( *weak_train_resp, float, 0, i ) = 
                1.0F / (MAX( p, CV_LB_PROB_THRESH ));
        }
        else
        {
            CV_MAT_ELEM( *weak_train_resp, float, 0, i ) = 
                -1.0F / (MAX( 1.0F - p, CV_LB_PROB_THRESH ));
        }
    }
    __END__;
}

void
icvBoostStartTrainingLB( CvMat* responses, CvMat* weak_train_resp, CvMat* weights,
                         CvMat* sum_resp )
{
    CV_FUNCNAME( "icvBoostStartTraining" );
    __BEGIN__;
    /* CV_CALL( cvSet( sum_resp, cvScalar( 0 ) ) ); */
    CV_CALL( icvResponsesAndWeightsLB( responses, weak_train_resp, weights, sum_resp ));
    __END__;
}

double
icvBoostNextWeakClassifierLB( CvMat* weak_eval, CvMat* responses, CvMat* weak_train_resp,
                              CvMat* weights, CvMat* sum_resp )
{
    double c = 0.5;
    CV_FUNCNAME( "icvBoostNextWeakClassifierLB" );
    __BEGIN__;
    int i;
    for( i = 0; i < sum_resp->cols; ++i )
        CV_MAT_ELEM( *sum_resp, float, 0, i ) +=
            0.5F * CV_MAT_ELEM( *weak_eval, float, 0, i );
    CV_CALL( icvResponsesAndWeightsLB( responses, weak_train_resp, weights, sum_resp ));
    __END__;
    return c;
}

double
icvBoostNextWeakClassifierGAB( CvMat* weak_eval, CvMat* responses,
                               CvMat* /* weak_train_resp */, CvMat* weights,
                               CvMat* /* sum_resp */ )
{
    double c = 1;
    //CV_FUNCNAME( "icvBoostNextWeakClassifierGAB" );
    __BEGIN__;
    float sumw;
    int i;

    sumw = 0;
    for( i = 0; i < responses->cols; ++i )
    {
        CV_MAT_ELEM( *weights, float, 0, i ) *= expf( 
            -(2.0F * CV_MAT_ELEM( *responses, int, 0, i ) - 1.0F) *
             CV_MAT_ELEM( *weak_eval, float, 0, i ) );

        sumw += CV_MAT_ELEM( *weights, float, 0, i );
    }
    if( sumw != 0 )
        for( i = 0; i < weights->cols; ++i )
            CV_MAT_ELEM( *weights, float, 0, i ) /= sumw;
    __END__;
    return c;
}

typedef void (*CvBoostStartTraining)( CvMat* responses,
                                      CvMat* weak_train_resp,
                                      CvMat* weights,
                                      CvMat* sum_resp );

typedef double (*CvBoostNextWeakClassifier)( CvMat* weak_eval,
                                             CvMat* responses,
                                             CvMat* weak_train_resp,
                                             CvMat* weights,
                                             CvMat* sum_resp );

CvBoostStartTraining startTraining[] = {
        icvBoostStartTraining,
        icvBoostStartTraining,
        icvBoostStartTrainingLB,
        icvBoostStartTraining
    };

CvBoostNextWeakClassifier nextWeakClassifier[] = {
        icvBoostNextWeakClassifierDAB,
        icvBoostNextWeakClassifierRAB,
        icvBoostNextWeakClassifierLB,
        icvBoostNextWeakClassifierGAB
    };

/* the function is internal so asserts are used for input arguments checks */
double
icvBoostIterate( CvMat* weak_eval, CvMat* responses, CvMat* weak_train_resp,
                 CvMat* weights, int type, CvMat* sum_resp )
{
    double c = 1;

    //CV_FUNCNAME( "icvBoostIterate" );
    
    __BEGIN__;

    /* the function is internal so asserts are used for input arguments checks */
   
    assert( CV_IS_MAT( responses ) );
    assert( CV_MAT_TYPE( responses->type ) == CV_32SC1 );
    assert( responses->rows == 1 );

    assert( !weak_eval || CV_IS_MAT( weak_eval ) );
    assert( !weak_eval || CV_MAT_TYPE( weak_eval->type ) == CV_32FC1 );
    assert( !weak_eval || weak_eval->rows == 1 );
    assert( !weak_eval || weak_eval->cols == responses->cols );
    
    assert( CV_IS_MAT( weak_train_resp ) );
    assert( CV_MAT_TYPE( weak_train_resp->type ) == CV_32FC1 );
    assert( weak_train_resp->rows == 1 );
    assert( weak_train_resp->cols == responses->cols );
    
    assert( CV_IS_MAT( weights ) );
    assert( CV_MAT_TYPE( weights->type ) == CV_32FC1 );
    assert( weights->rows == 1 );
    assert( weights->cols == responses->cols );

    assert( type >= CV_BT_DISCRETE );
    assert( type <= CV_BT_GENTLE );

    assert( CV_IS_MAT( weights ) );
    assert( CV_MAT_TYPE( weights->type ) == CV_32FC1 );
    assert( weights->rows == 1 );
    assert( weights->cols == responses->cols );

    assert( type != CV_BT_LOGIT || sum_resp );
    assert( !sum_resp || CV_IS_MAT( sum_resp ) );
    assert( !sum_resp || CV_MAT_TYPE( sum_resp->type ) == CV_32FC1 );
    assert( !sum_resp || sum_resp->rows == 1 );
    assert( !sum_resp || sum_resp->cols == responses->cols );

    if( !weak_eval )
    {
        /* start training */
        startTraining[type]( responses, weak_train_resp, weights, sum_resp );
    }
    else
    {
        c = nextWeakClassifier[type]( weak_eval, responses, weak_train_resp, weights, 
            sum_resp );
    }

    __END__;

    return c;
}

/****************************************************************************************\
*                                    Utility functions                                   *
\****************************************************************************************/

CV_IMPL
CvMat* cvTrimWeights( CvMat* weights, CvMat* idx, float factor )
{
    CvMat* ptr;

    CV_FUNCNAME( "cvTrimWeights" );
    
    ptr = NULL;
    __BEGIN__;
    int i, index, num;
    float sum_weights;
    uchar* wdata;
    int wstep;
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

        if( i > 0 )
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
        cvFree( (void**) &sorted_weights );
    }

    __END__;

    return ptr;
}

/****************************************************************************************\
*                                   Boosted trees models                                 *
\****************************************************************************************/

typedef struct CvBoostTrainState
{
    /* input defragmented training set */
    const float** raw_train_data; /* train_data is here */
    CvMat  train_data;            /* mat header for raw_train_data */
    int flags;
    CvMat* responses;             /* CV_32SC1. 0 or 1 values */

    CvStumpTrainParams stump_params;
    CvTREEBOOSTTrainParams weak_params;
    
    int valid;              /* are weak_train_resp and sum_resp valid? */
    CvMat* weights;
    CvMat* weak_train_resp; /* responses for weak classifier training */
    CvMat* weak_eval_resp;  /* last weak evaluation responses */
    CvMat* sum_resp;        /* only for CV_BT_LOGIT */
}
CvBoostTrainState;

void
icvReleaseBoostTrainStateMembers( CvBoostTrainState* ts )
{
    if( ts )
    {
        cvFree( (void**) &ts->raw_train_data );
        cvReleaseMat( &ts->responses );
        cvReleaseMat( &ts->weights );
        cvReleaseMat( &ts->weak_train_resp );
        cvReleaseMat( &ts->weak_eval_resp );
        cvReleaseMat( &ts->sum_resp );
        ts->valid = 0;
    }
}

/****************************************************************************************\
*                                        Get/Set                                         *
\****************************************************************************************/
CV_IMPL double
cvBtGetParamValue( CvStatModel* model,
                   const void* object,
                   int param_type,
                   int /* index */ )
{
    double val = 0;

    CV_FUNCNAME( "cvBtGetParamValue" );

    __BEGIN__;
    CvBtClassifier* bt = (CvBtClassifier*) model;

    if( !CV_IS_BOOST_TREE(bt) )
        CV_ERROR( bt ? CV_StsBadArg : CV_StsNullPtr, "Invalid boosted trees model");
    if( object )
        CV_ERROR( CV_StsBadArg, "Object pointer must be NULL" );

    switch( param_type )
    {
        case CV_MODEL_CLASS_NUM:
            val = (double) ((bt->class_labels) ? bt->class_labels->cols : 0);
            break;
        case CV_MODEL_FEATURE_NUM:
            val = (double) (bt->total_features);
            break;

        case CV_BT_ITER_STEP:
            val = (double) (bt->params.num_iter);
            break;
        case CV_BT_NUM_ITER:
            val = (double) ((bt->weak) ? bt->weak->total : 0);
            break;
        default:
            CV_ERROR( CV_StsBadArg, "Unsupported parameter" );
    }

    __END__;
    
    return val;
}

CV_IMPL void
cvBtGetParamMat( CvStatModel* model,
                 const void* object,
                 int param_type,
                 int /* index */,
                 CvMat* mat )
{
    CV_FUNCNAME( "cvBtGetParamMat" );

    __BEGIN__;
    CvBtClassifier* bt = (CvBtClassifier*) model;

    if( !CV_IS_BOOST_TREE( bt ) )
        CV_ERROR( bt ? CV_StsBadArg : CV_StsNullPtr, "Invalid boosted trees model");
    if( object )
        CV_ERROR( CV_StsBadArg, "Object pointer must be NULL" );

    mat->data.ptr = NULL;

    switch( param_type )
    {
        case CV_BT_WEIGHTS:
        {
            CvMat* weights = ((CvBoostTrainState*) bt->ts)->weights;

            if( weights )
            {
                CV_CALL( cvInitMatHeader( mat, weights->rows, weights->cols,
                    CV_MAT_TYPE( weights->type ),
                    weights->data.ptr, weights->step ));
            }
            break;
        }
        default:
            CV_ERROR( CV_StsBadArg, "Unsupported parameter" );
    }

    __END__;
}

CV_IMPL void
cvBtSetParamValue( CvStatModel* model,
                   const void* object,
                   int param_type,
                   int /* index */,
                   double value )
{
    CV_FUNCNAME( "cvBtSetParamValue" );

    __BEGIN__;
    CvBtClassifier* bt = (CvBtClassifier*) model;
    int i;

    if( !CV_IS_BOOST_TREE( bt ) )
        CV_ERROR( bt ? CV_StsBadArg : CV_StsNullPtr, "Invalid boosted trees model");
    if( object )
        CV_ERROR( CV_StsBadArg, "Object pointer must be NULL" );

    switch( param_type )
    {
        case CV_BT_ITER_STEP:
            i = cvRound( value );
            if( i < 0 )
                CV_ERROR( CV_StsBadArg, "Iteration step value must be non-negative" );
            bt->params.num_iter = i;
            if( bt->params.num_iter == 0 )
                icvReleaseBoostTrainStateMembers( (CvBoostTrainState*) bt->ts );
            break;
        default:
            CV_ERROR( CV_StsBadArg, "Unsupported parameter" );
    }

    __END__;
}

CV_IMPL void
cvBtSetParamMat( CvStatModel* model,
                 const void* object,
                 int param_type,
                 int /* index */,
                 CvMat* mat )
{
    CV_FUNCNAME( "cvBtSetParamMat" );

    __BEGIN__;
    CvBtClassifier* bt = (CvBtClassifier*) model;

    if( !CV_IS_BOOST_TREE( bt ) )
        CV_ERROR( bt ? CV_StsBadArg : CV_StsNullPtr, "Invalid boosted trees model");
    if( object )
        CV_ERROR( CV_StsBadArg, "Object pointer must be NULL" );

    switch( param_type )
    {
        case CV_BT_WEIGHTS:
        {
            CvMat** weights = &(((CvBoostTrainState*) bt->ts)->weights);

            if( !CV_IS_MAT( mat ) )
                CV_ERROR( mat ? CV_StsBadArg : CV_StsNullPtr, "Invalid matrix");
            if( CV_MAT_TYPE( mat->type ) != CV_32FC1 )
                CV_ERROR( CV_StsBadArg, "Matrix must be of 32fC1 type");
            if( mat->rows != 1 && mat->cols != 1 )
                CV_ERROR( CV_StsBadArg, "Matrix must have 1 row or 1 column");

            if( !(*weights) || (*weights)->cols != mat->rows + mat->cols - 1 )
            {
                cvReleaseMat( weights );
                CV_CALL( *weights = cvCreateMat( 1, mat->rows + mat->cols - 1,
                    CV_32FC1 ));
                ((CvBoostTrainState*) bt->ts)->valid = 0;
            }
            if( mat->rows == 1 )
            {
                CV_CALL( cvCopy( mat, *weights ) );
            }
            else
            {
                CV_CALL( cvTranspose( mat, *weights ) );
            }
            break;
        }
        break;
        default:
            CV_ERROR( CV_StsBadArg, "Unsupported parameter" );
    }

    __END__;
}

/****************************************************************************************\
*                                       Evaluation                                       *
\****************************************************************************************/
int
icvEvalTREEBOOSTRow( CvTreeBoostClassifier* tree, const float* sample )
{
    assert( tree != NULL );
    assert( sample != NULL );

    int idx = 0;

    do
    {
        if( sample[tree->comp_idx[idx]] < tree->threshold[idx] )
            idx = tree->left[idx];
        else
            idx = tree->right[idx];
    } while( idx > 0 );

    return -idx;
}

int
icvEvalTREEBOOSTCol( CvTreeBoostClassifier* tree, const float* sample, ptrdiff_t step )
{
    assert( tree != NULL );
    assert( sample != NULL );

    int idx = 0;

    do
    {
        if( *((float*) ((char*) sample + step * tree->comp_idx[idx])) <
                tree->threshold[idx] )
            idx = tree->left[idx];
        else
            idx = tree->right[idx];
    } while( idx > 0 );

    return -idx;
}

CV_IMPL float
cvEvalBtClassifier( const CvStatModel* model, const CvMat* sample, CvMat* prob )
{
    float* row_sample = NULL;
    int label = 0;

    CV_FUNCNAME( "cvEvalBtClassifier" );

    __BEGIN__;

    float val = 0;
    CvBtClassifier* bt = (CvBtClassifier*) model;
    int i;
    CvSeqReader reader;
    CvTreeBoostClassifier* tree;
    int cls = 0;

    if( !CV_IS_BOOST_TREE( bt ) )
        CV_ERROR( bt ? CV_StsBadArg : CV_StsNullPtr, "Invalid boosted trees model");

    CV_CALL( cvPreparePredictData( sample, bt->total_features, bt->comp_idx,
        bt->class_labels->cols, prob, &row_sample ));
    
    CV_CALL( cvStartReadSeq( bt->weak, &reader ) );
    for( i = 0; i < bt->weak->total; ++i )
    {
        CV_READ_SEQ_ELEM( tree, reader );
        val += tree->val[icvEvalTREEBOOSTRow( tree, row_sample )];
    }
    cls = (val >= 0.0F);
    if( prob )
    {
        if( prob->rows == 1 )
        {
            CV_MAT_ELEM( *prob, float, 0, 0 ) = (float) (cls == 0);
            CV_MAT_ELEM( *prob, float, 0, 1 ) = (float) (cls == 1);
        }
        else
        {
            CV_MAT_ELEM( *prob, float, 0, 0 ) = (float) (cls == 0);
            CV_MAT_ELEM( *prob, float, 1, 0 ) = (float) (cls == 1);
        }
    }

    label = CV_MAT_ELEM( *bt->class_labels, int, 0, cls );

    __END__;

    if( row_sample && (void*) row_sample != (void*) sample->data.ptr )
        cvFree( (void**) &row_sample );

    return (float) label;
}

CV_IMPL float
cvEvalWeakClassifiers( const CvBtClassifier* bt, const CvMat* sample,
                       CvMat* weak_vals, CvSlice slice, int eval_type )
{
    float val = 0;
    float* row_sample = NULL;

    CV_FUNCNAME( "cvEvalWeakClassifiers" );

    __BEGIN__;

    int total = 0;
    CvSeqReader reader;
    CvTreeBoostClassifier* tree;
    int i;

    if( !CV_IS_BOOST_TREE( bt ) )
        CV_ERROR( bt ? CV_StsBadArg : CV_StsNullPtr, "Invalid boosted trees model");
    if( slice.start_index < 0 || slice.start_index > bt->weak->total )
        CV_ERROR( CV_StsBadArg, "start index is out of range" );
    slice.end_index = MIN( slice.end_index, bt->weak->total );
    if( slice.end_index < slice.start_index )
        CV_ERROR( CV_StsBadArg, "end index is out of range" );
    total = slice.end_index - slice.start_index;
    if( total <= 0 )
        EXIT;
    if( eval_type != CV_BT_VALUE && eval_type != CV_BT_INDEX )
        CV_ERROR( CV_StsBadArg, "Invalid eval_type" );
    
    if( weak_vals && (!CV_IS_MAT( weak_vals ) ||
            ( eval_type == CV_BT_VALUE && CV_MAT_TYPE( weak_vals->type ) != CV_32FC1 ) ||
            ( eval_type == CV_BT_INDEX && CV_MAT_TYPE( weak_vals->type ) != CV_32SC1 )) )
        CV_ERROR( CV_StsBadArg, "weak_vals must be NULL or matrix of type 32fC1"
            " for values or 32fC1 for indices" );
    if( weak_vals && eval_type == CV_BT_VALUE && 
            (weak_vals->rows != 1 || (weak_vals->cols != 1 && weak_vals->cols != total)))
        CV_ERROR( CV_StsBadArg, "weak_vals matrix must be NULL, 1 by 1 or "
            "1 by <number_of_evaluated_classifiers>" );
    if( weak_vals && eval_type == CV_BT_INDEX && 
            (weak_vals->rows != 1 || weak_vals->cols != total ))
        CV_ERROR( CV_StsBadArg, "weak_vals matrix must be NULL or "
            "1 by <number_of_evaluated_classifiers>" );
    if( !weak_vals && eval_type == CV_BT_INDEX && total != 1 )
        CV_ERROR( CV_StsNullPtr, "weak_vals matrix must be specified" );

    CV_CALL( cvPreparePredictData( sample, bt->total_features, bt->comp_idx,
        bt->class_labels->cols, NULL, &row_sample ));
    
    CV_CALL( cvStartReadSeq( bt->weak, &reader ) );
    CV_CALL( cvSetSeqReaderPos( &reader, slice.start_index ) );

    val = 0;    
    if( eval_type == CV_BT_VALUE )
    {
        if( weak_vals && weak_vals->cols > 1 )
        {
            float weak_val;
            for( i = 0; i < total; ++i )
            {
                CV_READ_SEQ_ELEM( tree, reader );
                weak_val = tree->val[icvEvalTREEBOOSTRow( tree, row_sample )];
                CV_MAT_ELEM( *weak_vals, float, 0, i ) = weak_val;
                val += weak_val;
            }
        }
        else
        {
            for( i = 0; i < total; ++i )
            {
                CV_READ_SEQ_ELEM( tree, reader );
                val += tree->val[icvEvalTREEBOOSTRow( tree, row_sample )];
            }
            if( weak_vals )
            {
                CV_MAT_ELEM( *weak_vals, float, 0, 0 ) = val;
            }
        }
    }
    else
    {
        /* CV_BT_INDEX */
        if( weak_vals )
        {
            for( i = 0; i < total; ++i )
            {
                CV_READ_SEQ_ELEM( tree, reader );
                CV_MAT_ELEM( *weak_vals, int, 0, i ) = 
                    icvEvalTREEBOOSTRow( tree, row_sample );
            }
            val = (float) CV_MAT_ELEM( *weak_vals, int, 0, 0 );
        }
        else
        {
            CV_READ_SEQ_ELEM( tree, reader );
            val = (float) icvEvalTREEBOOSTRow( tree, row_sample );
        }
    }

    __END__;

    if( row_sample && (void*) row_sample != (void*) sample->data.ptr )
        cvFree( (void**) &row_sample );

    return val;
}

CV_IMPL void
cvPruneBtClassifier( CvBtClassifier* bt, CvSlice slice )
{
    CV_FUNCNAME( "cvPruneBtClassifier" );

    __BEGIN__;

    if( !CV_IS_BOOST_TREE( bt ) )
        CV_ERROR( bt ? CV_StsBadArg : CV_StsNullPtr, "Invalid boosted trees model");
    if( slice.start_index < 0 || slice.start_index > bt->weak->total )
        CV_ERROR( CV_StsBadArg, "start index is out of range" );
    slice.end_index = MIN( slice.end_index, bt->weak->total );
    if( slice.end_index < slice.start_index )
        CV_ERROR( CV_StsBadArg, "end index is out of range" );

    CV_CALL( cvSeqRemoveSlice( bt->weak, slice ) );
    ((CvBoostTrainState*) bt->ts)->valid = 0;

    __END__;
}

CV_IMPL void
cvReleaseBtClassifier( CvStatModel** model )
{
    CV_FUNCNAME( "cvReleaseBtClassifier" );

    __BEGIN__;

    int i;
    CvBtClassifier* bt;
    CvSeqReader reader;
    CvTreeBoostClassifier* tree;
    
    if( !model )
        CV_ERROR( CV_StsNullPtr, "NULL double pointer" );
    
    bt = (CvBtClassifier*) *model;
    
    if( !bt )
        EXIT;

    if( !CV_IS_BOOST_TREE(bt) )
        CV_ERROR( CV_StsBadArg, "Invalid Boosted trees model" );

    CV_CALL( cvStartReadSeq( bt->weak, &reader ) );
    for( i = 0; i < bt->weak->total; ++i )
    {
        CV_READ_SEQ_ELEM( tree, reader );
        tree->release( (CvStatModel**) &(tree) );
    }
    CV_CALL( cvReleaseMemStorage( &(bt->weak->storage) ));

    cvReleaseMat( &bt->class_labels );
    cvReleaseMat( &bt->comp_idx );

    icvReleaseBoostTrainStateMembers( (CvBoostTrainState*) bt->ts );

    cvFree( (void**) model );
    *model = NULL;

    __END__;
}

CvBtClassifierTrainParams*
icvBtDefaultTrainParams( CvBtClassifierTrainParams* tp )
{
    assert( tp );
    
    tp->boost_type = CV_BT_GENTLE;
    tp->num_iter = 100;
    tp->infl_trim_rate = 0.95f;
    tp->num_splits = 2;
    
    return tp;
}

CvBtClassifier*
icvAllocBtClassifier( CvBtClassifierTrainParams* train_params )
{
    CvBtClassifier* bt = NULL;

    CV_FUNCNAME( "icvAllocBtClassifier" );

    __BEGIN__;

    size_t data_size;
    CvStumpTrainParams* stump_params;
    CvTREEBOOSTTrainParams* weak_params;

    CvStumpType types[] = { CV_CLASSIFICATION_CLASS, CV_CLASSIFICATION,
                            CV_REGRESSION, CV_REGRESSION };
    CvStumpError errors[] = { CV_MISCLASSIFICATION, CV_GINI, CV_SQUARE, CV_SQUARE };

    if( train_params )
    {
        if( train_params->boost_type < CV_BT_DISCRETE ||
            train_params->boost_type > CV_BT_GENTLE )
            CV_ERROR( CV_StsBadArg, "Invalid boosting type" );
        if( train_params->num_iter < 0 )
            CV_ERROR( CV_StsBadArg, "Number of iterations must be non-negative" );
        if( train_params->infl_trim_rate <= 0 || train_params->infl_trim_rate > 1 )
            CV_ERROR( CV_StsBadArg, "Influence trimming rate must be in (0, 1]" );
        if( train_params->num_splits < 1 )
            CV_ERROR( CV_StsBadArg, "Number of splits must be positive" );
    }

    data_size = sizeof( *bt ) + sizeof( CvBoostTrainState );
    CV_CALL( bt = (CvBtClassifier*) cvCreateStatModel(
        CV_STAT_MODEL_MAGIC_VAL | CV_BOOST_TREE_MAGIC_VAL, data_size,
        cvReleaseBtClassifier, cvEvalBtClassifier ));

    CV_CALL( bt->weak = cvCreateSeq( 0, sizeof( *(bt->weak) ),
        sizeof( CvTreeBoostClassifier* ),  cvCreateMemStorage() ));

    bt->ts = bt + 1;
    
    if( !train_params )
    {
        icvBtDefaultTrainParams( &bt->params );
    }
    else
    {
        bt->params = *train_params;
    }

    stump_params = &((CvBoostTrainState*) bt->ts)->stump_params;
    weak_params = &((CvBoostTrainState*) bt->ts)->weak_params;

    stump_params->type = types[bt->params.boost_type];
    stump_params->error = errors[bt->params.boost_type];

    weak_params->count = bt->params.num_splits;
    weak_params->stumpConstructor = &cvCreateStumpClassifier;
    weak_params->stumpTrainParams = (CvClassifierTrainParams*) stump_params;

    bt->get_value = &cvBtGetParamValue;
    bt->set_value = &cvBtSetParamValue;
    bt->get_mat   = &cvBtGetParamMat;
    bt->set_mat   = &cvBtSetParamMat;

    __END__;

    if( cvGetErrStatus() < 0 )
        cvReleaseBtClassifier( (CvStatModel**) &bt );

    return bt;
}

void
icvBtNewTrainData( CvBtClassifier* bt,
                   CvMat* train_data,
                   int flags,
                   CvMat* responses,
                   CvMat* comp_idx,
                   CvMat* sample_idx,
                   CvMat* /*type_mask*/,
                   CvMat* /*missval_mask*/ )
{
    const float** new_raw_train_data = NULL;
    CvMat* new_responses = NULL;
    CvMat* new_resp_map = NULL;
    CvMat* new_comp_idx = NULL;

    CV_FUNCNAME( "icvBtNewTrainData" );
    __BEGIN__;

    int i;
    int total_samples = 0;
    int selected_features = 0;
    int total_features = 0;
    
    int new_bt = (bt->weak->total == 0);

    CvBoostTrainState* ts = (CvBoostTrainState*) bt->ts;

    CV_CALL( cvPrepareTrainData( "cvUpdateBtClassifier", train_data, flags,
            responses, CV_VAR_CATEGORICAL, comp_idx, sample_idx, true,
            &new_raw_train_data, &total_samples, &selected_features,
            &total_features, &new_responses, &new_resp_map, &new_comp_idx, 0 ));

    if( !new_bt && total_features < bt->total_features )
        CV_ERROR( CV_StsBadArg, "Subsequent training set must have at least the same"
            " number of features" );
    if( new_resp_map->cols != 2 )
        CV_ERROR( CV_StsBadArg, "Only two-class problems are supported" );
    if( !new_bt && 
        ((CV_MAT_ELEM( *bt->class_labels, int, 0, 0 ) !=
          CV_MAT_ELEM( *new_resp_map, int, 0, 0 )) ||
         (CV_MAT_ELEM( *bt->class_labels, int, 0, 1 ) !=
          CV_MAT_ELEM( *new_resp_map, int, 0, 1 ))) )
    {
        CV_ERROR( CV_StsBadArg, "Subsequent class labels must be the same" );
    }
    if( !new_bt )
    {
        int invalid_comp_idx = 0;
        if( (!bt->comp_idx && new_comp_idx) || (bt->comp_idx && !new_comp_idx) )
            invalid_comp_idx = 1;
        else if( bt->comp_idx && new_comp_idx )
        {
            if( bt->comp_idx->cols > new_comp_idx->cols )
                invalid_comp_idx = 1;
            else
            {
                for( i = 0; i < bt->comp_idx->cols; ++i )
                {
                    if( CV_MAT_ELEM( *bt->comp_idx, int, 0, i ) !=
                        CV_MAT_ELEM( *new_comp_idx, int, 0, i ) )
                    {
                        invalid_comp_idx = 1;
                        break;
                    }
                }
            }
        }
        if( invalid_comp_idx )
            CV_ERROR( CV_StsBadArg, "Subsequent comp_idx must be the superset of the "
            "previous one" );
    }
    
    /*if( new_type_mask )
    {
        for( i = 0; i < new_type_mask->cols - 1; ++i )
            if( CV_MAT_ELEM( *new_type_mask, uchar, 0, i ) != 0 )
                CV_ERROR( CV_StsBadArg, "Only numerical features are supported" );
            if( CV_MAT_ELEM( *new_type_mask, uchar, 0, i ) != 1 )
                CV_ERROR( CV_StsBadArg, "Only categorical responses are supported" );
    }*/

    /* "commit" */
    bt->total_features = total_features;
    cvReleaseMat( &bt->class_labels );
    bt->class_labels = new_resp_map;
    cvReleaseMat( &bt->comp_idx );
    bt->comp_idx = new_comp_idx;

    cvFree( (void**) &ts->raw_train_data );
    ts->raw_train_data = new_raw_train_data;
    CV_CALL( cvInitMatHeader( &ts->train_data,
            CV_IS_ROW_SAMPLE( flags ) ? total_samples : selected_features,
            CV_IS_ROW_SAMPLE( flags ) ? selected_features : total_samples, CV_32FC1,
            (void*) new_raw_train_data[0],
            ((char*)new_raw_train_data[1] - (char*)new_raw_train_data[0]) ));
    /* the cvPrepareTrainData function always copies train data and rearranges it in 
       a such way that samples are in rows */
    /* ts->flags = flags; */
    ts->flags = CV_ROW_SAMPLE;
    cvReleaseMat( &ts->responses );
    ts->responses = new_responses;

    ts->valid = 0;

    __END__;

    if( cvGetErrStatus() < 0 )
    {
        cvFree( (void**) &new_raw_train_data );
        cvReleaseMat( &new_responses );
        cvReleaseMat( &new_resp_map );
        cvReleaseMat( &new_comp_idx );
    }
}

CV_IMPL void
cvUpdateBtClassifier( CvBtClassifier* model,
                      CvMat* train_data,
                      int flags,
                      CvMat* responses,
                      CvStatModelParams* train_params,
                      CvMat* comp_idx,
                      CvMat* sample_idx,
                      CvMat* type_mask,
                      CvMat* missval_mask)
{
    CV_FUNCNAME( "cvUpdateBtClassifier" );
    __BEGIN__;

    CvBtClassifier* bt = (CvBtClassifier*) model;
    CvBoostTrainState* ts = (CvBoostTrainState*) bt->ts;

    int i, j;
    CvSeqWriter writer;
    CvTreeBoostClassifier* weak = NULL;

    if( !CV_IS_BOOST_TREE( bt ) )
        CV_ERROR( bt ? CV_StsBadArg : CV_StsNullPtr, "Invalid boosted trees model");
    if( train_params )
        CV_ERROR( CV_StsError, "Train parameters are not expected and must be NULL" );
    if( missval_mask )
        CV_ERROR( CV_StsError, "Missed values are not supported and must be NULL" );

    if( train_data == NULL )
    {
        /* update the model using old training data */

        if( responses || comp_idx || sample_idx || type_mask )
            CV_ERROR( CV_StsBadArg, "train_data, responses, comp_idx, sample_idx and "
                "type_mask must be NULL simultaneously" );

        if( bt->params.num_iter <= 0 )
        {
            /* release extra */
            icvReleaseBoostTrainStateMembers( ts );
        }
    }
    else
    {
        /* new training data */
        CV_CALL( icvBtNewTrainData( bt, train_data, flags, responses,
            comp_idx, sample_idx, type_mask, missval_mask ) );
    }
    if( bt->params.num_iter <= 0 )
        EXIT;
    if( !ts->raw_train_data )
        CV_ERROR( CV_StsBadArg, "Training data set must be specified" );

    if( !ts->valid )
    {
        if( !ts->weights || ts->weights->cols != ts->responses->cols )
        {
            cvReleaseMat( &ts->weights );
            CV_CALL( ts->weights = cvCreateMat( 1, ts->responses->cols, CV_32FC1 ));
            CV_CALL( cvSet( ts->weights, cvScalar( 1.0 / ts->responses->cols ) ));
        }
        if( !ts->weak_train_resp || ts->weak_train_resp->cols != ts->responses->cols )
        {
            cvReleaseMat( &ts->weak_train_resp );
            CV_CALL( ts->weak_train_resp=cvCreateMat( 1, ts->responses->cols, CV_32FC1 ));
            CV_CALL( cvSet( ts->weak_train_resp, cvScalar(0) ));
        }
        if( !ts->weak_eval_resp || ts->weak_eval_resp->cols != ts->responses->cols )
        {
            cvReleaseMat( &ts->weak_eval_resp );
            CV_CALL( ts->weak_eval_resp = cvCreateMat( 1, ts->responses->cols, CV_32FC1));
            CV_CALL( cvSet( ts->weak_eval_resp, cvScalar(0) ));
        }

        if( bt->params.boost_type == CV_BT_LOGIT )
        {
            CvSeqReader reader;

            if( !ts->sum_resp || ts->sum_resp->cols != ts->responses->cols )
            {
                cvReleaseMat( &ts->sum_resp );
                CV_CALL( ts->sum_resp = cvCreateMat( 1, ts->responses->cols, CV_32FC1 ));
                CV_CALL( cvSet( ts->sum_resp, cvScalar(0) ));
            }
            CV_CALL( cvSet( ts->sum_resp, cvScalar(0) ) );
            CV_CALL( cvStartReadSeq( bt->weak, &reader ) );
            if( CV_IS_ROW_SAMPLE( ts->flags ) )
            {
                for( i = 0; i < bt->weak->total; ++i )
                {
                    CV_READ_SEQ_ELEM( weak, reader );
                    for( j = 0; j < ts->responses->cols; ++j )
                    {
                        CV_MAT_ELEM( *ts->sum_resp, float, 0, j ) += 
                            weak->val[icvEvalTREEBOOSTRow( weak, ts->raw_train_data[j] )];
                    }
                }
            }
            else
            {
                ptrdiff_t step = (char*) ts->raw_train_data[1] -
                    (char*) ts->raw_train_data[0];

                for( i = 0; i < bt->weak->total; ++i )
                {
                    CV_READ_SEQ_ELEM( weak, reader );
                    for( j = 0; j < ts->responses->cols; ++j )
                    {
                        CV_MAT_ELEM( *ts->sum_resp, float, 0, j ) += 
                            weak->val[icvEvalTREEBOOSTCol( weak, ts->raw_train_data[j],
                            step )];
                    }
                }
            }
        }
        else
        {
            cvReleaseMat( &ts->sum_resp );
        }

        CV_CALL( icvBoostIterate( NULL, ts->responses, ts->weak_train_resp, ts->weights,
            bt->params.boost_type, ts->sum_resp ) );
        ts->valid = 1;
    }
    /* perform iteration(s) */
    CV_CALL( cvStartAppendToSeq( bt->weak, &writer ) );
    for( i = 0; i < bt->params.num_iter; ++i )
    {
        CvMat* sample_idx = NULL;        
        double c = 0;

        /* create sample subset by influence trimming */
        if( bt->params.infl_trim_rate < 1.0 )
        {
            CV_CALL( sample_idx = cvTrimWeights( ts->weights, NULL,
                (float) bt->params.infl_trim_rate ) );
        }

        /* train weak classifier */
        CV_CALL( weak = (CvTreeBoostClassifier*) cvCreateTREEBOOSTClassifier(
            &ts->train_data, ts->flags, ts->weak_train_resp, NULL /* type_mask */,
            NULL /* missval_mask */, NULL /* comp_idx */, sample_idx, ts->weights,
            (CvClassifierTrainParams*) &ts->weak_params ) );

        if( bt->params.boost_type == CV_BT_REAL )
        {
            for( j = 0; j <= weak->count; ++j )
                weak->val[j] = cvLogRatio( weak->val[j] );
        }
        
        if( CV_IS_ROW_SAMPLE( ts->flags ) )
        {
            for( j = 0; j < ts->responses->cols; ++j )
            {
                CV_MAT_ELEM( *ts->weak_eval_resp, float, 0, j ) = 
                    weak->val[icvEvalTREEBOOSTRow( weak, ts->raw_train_data[j] )];
            }
        }
        else
        {
            ptrdiff_t step = (char*)ts->raw_train_data[1] - (char*)ts->raw_train_data[0];

            for( j = 0; j < ts->responses->cols; ++j )
            {
                CV_MAT_ELEM( *ts->weak_eval_resp, float, 0, j ) = 
                    weak->val[icvEvalTREEBOOSTCol( weak, ts->raw_train_data[0]+j, step)];
            }
        }
        
        /* update boosting training state */
        CV_CALL( c = icvBoostIterate( ts->weak_eval_resp, ts->responses,
            ts->weak_train_resp, ts->weights, bt->params.boost_type, ts->sum_resp ) );
        
        for( j = 0; j <= weak->count; ++j )
            weak->val[j] *= (float) c;

        CV_WRITE_SEQ_ELEM( weak, writer );

        /* clean up */
        cvReleaseMat( &sample_idx );
    }
    CV_CALL( cvEndWriteSeq( &writer ) );

    __END__;
}

CV_IMPL CvStatModel*
cvCreateBtClassifier( CvMat* train_data,
                      int flags,
                      CvMat* responses,
                      CvStatModelParams* train_params,
                      CvMat* comp_idx,
                      CvMat* sample_idx,
                      CvMat* type_mask,
                      CvMat* missval_mask)
{
    CvBtClassifier* bt = NULL;

    CV_FUNCNAME( "cvCreateBtClassifier" );

    __BEGIN__;

    CV_CALL( bt = icvAllocBtClassifier( (CvBtClassifierTrainParams*) train_params ));

    CV_CALL( cvUpdateBtClassifier( bt, train_data, flags, responses,
        NULL /* train_params */, comp_idx, sample_idx, type_mask, missval_mask ));

    __END__;

    if( cvGetErrStatus() < 0 )
        cvReleaseBtClassifier( (CvStatModel**) &bt );
    
    return (CvStatModel*) bt;
}

/****************************************************************************************\
*                                  Persistence functions                                 *
\****************************************************************************************/

/* field names */

#define ICV_BT_PARAMS_NAME         "params"

#define ICV_BT_BOOST_TYPE_NAME     "boost_type"
#define ICV_BT_NUM_ITER_NAME       "num_iter"
#define ICV_BT_INFL_TRIM_NAME      "infl_trim_rate"
#define ICV_BT_NUM_SPLITS_NAME     "num_splits"

#define ICV_BT_CLASS_LABELS_NAME   "class_labels"
#define ICV_BT_TOTAL_FEATURES_NAME "total_features"

#define ICV_BT_TREES_NAME          "trees"

#define ICV_BT_COMP_IDX_NAME       "comp_idx"
#define ICV_BT_WEIGHTS_NAME        "weights"

#define ICV_BT_COMP_IDX_NAME       "comp_idx"
#define ICV_BT_THRESHOLD_NAME      "threshold"
#define ICV_BT_LEFT_NODE_NAME      "left_node"
#define ICV_BT_LEFT_VAL_NAME       "left_val"
#define ICV_BT_RIGHT_NODE_NAME     "right_node"
#define ICV_BT_RIGHT_VAL_NAME      "right_val"

static struct { int type; char* name; } icvBtBoostTypeMap[] =
{
    {CV_BT_DISCRETE, "Discrete AdaBoost"},
    {CV_BT_REAL,     "Real AdaBoost"},
    {CV_BT_LOGIT,    "LogitBoost"},
    {CV_BT_GENTLE,   "Gentle AdaBoost"},
    /* dummy last line */
    {-1,             NULL}
};

char* icvBoostTypeToName( int boost_type )
{
    int i;
    for( i = 0; icvBtBoostTypeMap[i].name != NULL; ++i )
    {
        if( icvBtBoostTypeMap[i].type == boost_type ) break;
    }
    return icvBtBoostTypeMap[i].name;
}

/* returns -1 if fails */
int icvNameToBoostType( const char* boost_name )
{
    int i;
    for( i = 0; icvBtBoostTypeMap[i].name != NULL; ++i )
    {
        if( !strcmp( icvBtBoostTypeMap[i].name, boost_name ) ) break;
    }

    return icvBtBoostTypeMap[i].type;
}

CV_IMPL int
cvIsBtClassifier( const void* struct_ptr )
{
    //CV_FUNCNAME( "cvIsBtClassifier" );

    __BEGIN__;

    return CV_IS_BOOST_TREE( struct_ptr );

    __END__;
}

CV_IMPL void*
cvReadBtClassifier( CvFileStorage* storage, CvFileNode* node )
{
    CvBtClassifier* bt = 0;
    
    CV_FUNCNAME( "cvReadBtClassifier" );

    __BEGIN__;
    const char* boost_name = NULL;
    CvBtClassifierTrainParams params;
    CvFileNode* params_fn = NULL;
    CvFileNode* trees_fn = NULL;
    void* ptr = NULL;
    CvSeqReader trees_fn_reader;
    CvSeqWriter weak_writer;
    char buf[256];
    int i, j;

    assert( storage );
    assert( node );

    CV_CALL( params_fn = cvGetFileNodeByName( storage, node, ICV_BT_PARAMS_NAME ) );
    if( !params_fn || !CV_NODE_IS_MAP( params_fn->tag ) )
        CV_ERROR( CV_StsError, "Invalid params" );

    CV_CALL( boost_name = cvReadStringByName( storage, params_fn,
        ICV_BT_BOOST_TYPE_NAME, NULL ));
    params.boost_type = icvNameToBoostType( boost_name );
    if( params.boost_type == -1 )
        CV_ERROR( CV_StsError, "Invalid params.boost_type" );
    CV_CALL( params.num_iter = cvReadIntByName( storage, params_fn,
        ICV_BT_NUM_ITER_NAME, -1 ));
    if( params.num_iter < 0 )
        CV_ERROR( CV_StsError, "Invalid params.num_iter" );
    CV_CALL( params.infl_trim_rate = cvReadRealByName( storage, params_fn,
        ICV_BT_INFL_TRIM_NAME, -1 ));
    if( params.infl_trim_rate <= 0 || params.infl_trim_rate > 1 )
        CV_ERROR( CV_StsError, "Invalid params.infl_trim_rate" );
    CV_CALL( params.num_splits = cvReadIntByName( storage, params_fn,
        ICV_BT_NUM_SPLITS_NAME, -1 ));
    if( params.num_splits <= 0 )
        CV_ERROR( CV_StsError, "Invalid params.num_splits" );
    
    CV_CALL( bt = icvAllocBtClassifier( &params ) );

    CV_CALL( ptr = cvReadByName( storage, node, ICV_BT_CLASS_LABELS_NAME ));
    if( !CV_IS_MAT( ptr ) )
    {
        cvRelease( &ptr );
        CV_ERROR( CV_StsError, "Invalid class_labels" );
    }
    bt->class_labels = (CvMat*) ptr;

    CV_CALL( bt->total_features = cvReadIntByName( storage, node,
        ICV_BT_TOTAL_FEATURES_NAME, -1 ));
    if( bt->total_features <= 0 )
        CV_ERROR( CV_StsError, "Invalid total_features" );

    CV_CALL( ptr = cvReadByName( storage, node, ICV_BT_COMP_IDX_NAME ));
    if( ptr != NULL && !CV_IS_MAT( ptr ) )
    {
        cvRelease( &ptr );
        CV_ERROR( CV_StsError, "Invalid comp_idx" );
    }
    bt->comp_idx = (CvMat*) ptr;

    CV_CALL( ptr = cvReadByName( storage, node, ICV_BT_WEIGHTS_NAME ));
    if( ptr != NULL && !CV_IS_MAT( ptr ) )
    {
        cvRelease( &ptr );
        CV_ERROR( CV_StsError, "Invalid comp_idx" );
    }
    ((CvBoostTrainState*) bt->ts)->weights = (CvMat*) ptr;

    CV_CALL( trees_fn = cvGetFileNodeByName( storage, node, ICV_BT_TREES_NAME ) );
    if( !trees_fn || !CV_NODE_IS_SEQ( trees_fn->tag ) )
        CV_ERROR( CV_StsError, "Invalid trees" );
    CV_CALL( cvStartReadSeq( trees_fn->data.seq, &trees_fn_reader ));
    CV_CALL( cvStartAppendToSeq( bt->weak , &weak_writer ));
    for( i = 0; i < trees_fn->data.seq->total; ++i )
    {        
        CvFileNode* tree_fn = NULL;
        CvSeqReader tree_fn_reader;
        CvTreeBoostClassifier* tree = NULL;
        int next_leaf_idx = 0;

        tree_fn = (CvFileNode*) trees_fn_reader.ptr;

        if( !tree_fn || !CV_NODE_IS_SEQ( tree_fn->tag ) ||
                tree_fn->data.seq->total < 1 ||
                tree_fn->data.seq->total > bt->params.num_splits )
        {
            sprintf( buf, "Invalid tree %d", i );
            CV_ERROR( CV_StsError, buf );
        }

        CV_CALL( tree = icvAllocTREEBOOSTClassifier( bt->params.num_splits ) );
        CV_WRITE_SEQ_ELEM( tree, weak_writer );
        CV_CALL( cvStartReadSeq( tree_fn->data.seq, &tree_fn_reader ));
        for( j = 0; j < tree_fn->data.seq->total; ++j )
        {
            CvFileNode* node_fn = NULL;

            int comp_idx = -1;
            double threshold = DBL_MAX;
            int left = -1;
            int right = -1;
            double left_val = DBL_MAX;
            double right_val = DBL_MAX;

            node_fn = (CvFileNode*) tree_fn_reader.ptr;
            if( !node_fn || !CV_NODE_IS_MAP( node_fn->tag ) )
            {
                sprintf( buf, "Invalid node %d of tree %d", j, i );
                CV_ERROR( CV_StsError, buf );
            }

            CV_CALL( comp_idx = cvReadIntByName( storage, node_fn,
                ICV_BT_COMP_IDX_NAME, -1 ));
            CV_CALL( threshold = cvReadRealByName( storage, node_fn,
                ICV_BT_THRESHOLD_NAME, DBL_MAX ));

            CV_CALL( left = cvReadIntByName( storage, node_fn,
                ICV_BT_LEFT_NODE_NAME, -1 ));
            CV_CALL( left_val = cvReadRealByName( storage, node_fn,
                ICV_BT_LEFT_VAL_NAME, DBL_MAX ));

            CV_CALL( right = cvReadIntByName( storage, node_fn,
                ICV_BT_RIGHT_NODE_NAME, -1 ));
            CV_CALL( right_val = cvReadRealByName( storage, node_fn,
                ICV_BT_RIGHT_VAL_NAME, DBL_MAX ));
            
            if( comp_idx < 0 || comp_idx >= bt->total_features )
            {
                sprintf( buf, "Invalid node %d of tree %d: bad comp_idx", j, i );
                CV_ERROR( CV_StsError, buf );
            }
            if( threshold == DBL_MAX )
            {
                sprintf( buf, "Invalid node %d of tree %d: bad threshold", j, i );
                CV_ERROR( CV_StsError, buf );
            }
            if( (left == -1 && left_val == DBL_MAX) ||
                (left != -1 && (left < 1 || left >= bt->params.num_splits)) )
            {
                sprintf( buf, "Invalid node %d of tree %d: bad left branch", j, i );
                CV_ERROR( CV_StsError, buf );
            }
            if( (right == -1 && right_val == DBL_MAX) ||
                (right != -1 && (right < 1 || right >= bt->params.num_splits)) )
            {
                sprintf( buf, "Invalid node %d of tree %d: bad right branch", j, i );
                CV_ERROR( CV_StsError, buf );
            }

            tree->comp_idx[j] = comp_idx;
            tree->threshold[j] = (float)threshold;
            if( left == -1 )
            {
                if( next_leaf_idx > bt->params.num_splits )
                {
                    sprintf( buf, "Invalid tree %d: too many leaves", i );
                    CV_ERROR( CV_StsError, buf );
                }
                tree->left[j] = -next_leaf_idx;
                tree->val[next_leaf_idx++] = (float) left_val;
            }
            else
            {
                tree->left[j] = left;
            }
            if( right == -1 )
            {
                if( next_leaf_idx > bt->params.num_splits )
                {
                    sprintf( buf, "Invalid tree %d: too many leaves", i );
                    CV_ERROR( CV_StsError, buf );
                }
                tree->right[j] = -next_leaf_idx;
                tree->val[next_leaf_idx++] = (float) right_val;
            }
            else
            {
                tree->right[j] = right;
            }
            CV_NEXT_SEQ_ELEM( sizeof( *node_fn ), tree_fn_reader );
        }
        CV_NEXT_SEQ_ELEM( sizeof( *tree_fn ), trees_fn_reader );
    } /* for each tree */
    CV_CALL( cvEndWriteSeq( &weak_writer ) );

    __END__;

    if( cvGetErrStatus() < 0 )
        cvReleaseBtClassifier( (CvStatModel**) &bt );

    return bt;
}


CV_IMPL void
cvWriteBtClassifier( CvFileStorage* storage, const char* name, const void* struct_ptr,
                     CvAttrList attributes )
{
    CV_FUNCNAME( "cvWriteBtClassifier" );

    __BEGIN__;

    int i, j;
    const CvBtClassifier* bt = (const CvBtClassifier*) struct_ptr;
    CvSeqReader reader;

    if( !CV_IS_BOOST_TREE( bt ) )
        CV_ERROR( bt ? CV_StsBadArg : CV_StsNullPtr, "Invalid boosted trees model");

    /* TODO: add integrity checks */

    CV_CALL( cvStartWriteStruct( storage, name, CV_NODE_MAP,
                                 cvTypeOf( struct_ptr )->type_name, attributes ));

    /* params */
    CV_CALL( cvStartWriteStruct( storage, ICV_BT_PARAMS_NAME, CV_NODE_MAP ));
    CV_CALL( cvWriteString( storage, ICV_BT_BOOST_TYPE_NAME,
                            icvBoostTypeToName( bt->params.boost_type ), 0 ));
    CV_CALL( cvWriteInt( storage, ICV_BT_NUM_ITER_NAME, bt->params.num_iter ));
    CV_CALL( cvWriteReal( storage, ICV_BT_INFL_TRIM_NAME, bt->params.infl_trim_rate ));
    CV_CALL( cvWriteInt( storage, ICV_BT_NUM_SPLITS_NAME, bt->params.num_splits ));
    CV_CALL( cvEndWriteStruct( storage )); /* params */
    CV_CALL( cvWrite( storage, ICV_BT_CLASS_LABELS_NAME, bt->class_labels ));
    CV_CALL( cvWriteInt( storage, ICV_BT_TOTAL_FEATURES_NAME, bt->total_features ));
    if( bt->comp_idx )
        CV_CALL( cvWrite( storage, ICV_BT_COMP_IDX_NAME, bt->comp_idx ));
    if( ((CvBoostTrainState*) bt->ts)->weights && ((CvBoostTrainState*) bt->ts)->valid )
        CV_CALL( cvWrite( storage, ICV_BT_WEIGHTS_NAME,
            ((CvBoostTrainState*) bt->ts)->weights ));
    
    /* write sequence of trees */
    CV_CALL( cvStartWriteStruct( storage, ICV_BT_TREES_NAME, CV_NODE_SEQ ));
    CV_CALL( cvStartReadSeq( bt->weak, &reader ));
    for( i = 0; i < bt->weak->total; ++i )
    {
        const CvTreeBoostClassifier* tree = NULL;
        char comment[256];

        CV_CALL( cvStartWriteStruct( storage, NULL, CV_NODE_SEQ ));
        sprintf( comment, "tree %d", i );
        CV_CALL( cvWriteComment( storage, comment, 1 ));

        CV_READ_SEQ_ELEM( tree, reader );

        for( j = 0; j < tree->count; ++j )
        {
            CV_CALL( cvStartWriteStruct( storage, NULL, CV_NODE_MAP ));
            if( j ) sprintf( comment, "node %d", j );
            else sprintf( comment, "root node %d", j );
            CV_CALL( cvWriteComment( storage, comment, 1 ));
            CV_CALL( cvWriteInt( storage, ICV_BT_COMP_IDX_NAME, tree->comp_idx[j] ));
            CV_CALL( cvWriteReal( storage, ICV_BT_THRESHOLD_NAME, tree->threshold[j] ));
            if( tree->left[j] > 0 )
            {
                CV_CALL( cvWriteInt( storage, ICV_BT_LEFT_NODE_NAME, tree->left[j] ));
            }
            else
            {
                CV_CALL( cvWriteReal( storage, ICV_BT_LEFT_VAL_NAME,
                                      tree->val[-tree->left[j]] ));
            }
            if( tree->right[j] > 0 )
            {
                CV_CALL( cvWriteInt( storage, ICV_BT_RIGHT_NODE_NAME, tree->right[j] ));
            }
            else
            {
                CV_CALL( cvWriteReal( storage, ICV_BT_RIGHT_VAL_NAME,
                                      tree->val[-tree->right[j]] ));
            }
            CV_CALL( cvEndWriteStruct( storage )); /* node */
        }

        CV_CALL( cvEndWriteStruct( storage )); /* tree */
    }

    CV_CALL( cvEndWriteStruct( storage )); /* trees */
        
    CV_CALL( cvEndWriteStruct( storage )); /* bt classifier */

    __END__;
}

CV_IMPL void*
cvCloneBtClassifier( const void* struct_ptr )
{
    CvBtClassifier* out = NULL;
    
    CV_FUNCNAME( "cvCloneBtClassifier" );

    __BEGIN__;

    CvBtClassifier* bt = (CvBtClassifier*) struct_ptr;
    CvSeqReader reader;
    CvSeqWriter writer;
    int i, j;

    if( !CV_IS_BOOST_TREE( bt ) )
        CV_ERROR( bt ? CV_StsBadArg : CV_StsNullPtr, "Invalid boosted trees model");

    CV_CALL( out = icvAllocBtClassifier( &bt->params ) );

    CV_CALL( out->class_labels = cvCloneMat( bt->class_labels ) );
    out->total_features = bt->total_features;
    if( bt->comp_idx )
        CV_CALL( bt->comp_idx = cvCloneMat( bt->comp_idx ) );
    CV_CALL( cvStartReadSeq( bt->weak, &reader ) );
    CV_CALL( cvStartAppendToSeq( out->weak, &writer ) );
    for( i = 0; i < bt->weak->total; ++i )
    {
        CvTreeBoostClassifier* weak = NULL;
        CvTreeBoostClassifier* out_weak = NULL;

        CV_READ_SEQ_ELEM( weak, reader );
        CV_CALL( out_weak = icvAllocTREEBOOSTClassifier( bt->params.num_splits ) );
        out_weak->count = weak->count;
        for( j = 0; j < weak->count; ++j )
        {
            out_weak->comp_idx[j] = weak->comp_idx[j];
            out_weak->threshold[j] = weak->threshold[j];
            out_weak->left[j] = weak->left[j];
            out_weak->right[j] = weak->right[j];
            out_weak->val[j] = weak->val[j];
        }
        out_weak->val[j] = weak->val[j];
        CV_WRITE_SEQ_ELEM( out_weak, writer );
    }
    CV_CALL( cvEndWriteSeq( &writer ) );

    /* TODO write train state clone code */

    __END__;
    if( cvGetErrStatus() < 0 && out )
        out->release( (CvStatModel**) &out );

    return out;
}

int
icvRegisterBtClassifierType()
{
    //CV_FUNCNAME( "icvRegisterBtClassifierType" );

    __BEGIN__;

    CvTypeInfo info;

    info.header_size = sizeof( info );
    info.is_instance = cvIsBtClassifier;
    info.release = (CvReleaseFunc) cvReleaseBtClassifier;
    info.read = cvReadBtClassifier;
    info.write = cvWriteBtClassifier;
    info.clone = cvCloneBtClassifier;
    info.type_name = CV_TYPE_NAME_ML_BOOSTING;
    cvRegisterType( &info );

    __END__;

    return 1;
}

static int icv_register_bt_classifier_type = icvRegisterBtClassifierType();

/* End of file. */

