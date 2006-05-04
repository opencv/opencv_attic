/*M///////////////////////////////////////////////////////////////////////////////////////
//
//  IMPORTANT: READ BEFORE DOWNLOADING, COPYING, INSTALLING OR USING.
//
//  By downloading, copying, installing or using the software you agree to this license.
//  If you do not agree to this license, do not download, install,
//  copy or use the software.
//
//
//            Intel License Agreement
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

/****************************************************************************************\
*                 Bayes classifier with assumption of normal features                    *
\****************************************************************************************/

#define ICV_IS_NBAYES( nb )                                          \
{                                                                           \
    if( !CV_IS_NBAYES( nb ) )                                               \
        CV_ERROR( CV_StsBadArg, "" );                                       \
    if( (nb)->dims < 1 )                                                    \
        CV_ERROR( CV_StsBadArg, "" );                                       \
    if( !ICV_IS_MAT_OF_TYPE( (nb)->cls_labels, CV_32SC1 ) )                 \
        CV_ERROR( CV_StsBadArg, "" );                                       \
    if( (nb)->cls_labels->rows != 1 || (nb)->cls_labels->cols < 2 )         \
        CV_ERROR( CV_StsBadArg, "" );                                       \
    for( i = 0; i < (nb)->cls_labels->cols; i++ )                           \
    {                                                                       \
        int k;                                                              \
        if( !ICV_IS_MAT_OF_TYPE( (nb)->count[i], CV_32SC1 ) )               \
            CV_ERROR( CV_StsBadArg, "" );                                   \
        if( !ICV_IS_MAT_OF_TYPE( (nb)->sum[i], CV_32FC1 ) )                 \
            CV_ERROR( CV_StsBadArg, "" );                                   \
        if( !ICV_IS_MAT_OF_TYPE( (nb)->productsum[i], CV_32FC1 ) )          \
            CV_ERROR( CV_StsBadArg, "" );                                   \
        if( !ICV_IS_MAT_OF_TYPE( (nb)->avg[i], CV_32FC1 ) )                 \
            CV_ERROR( CV_StsBadArg, "" );                                   \
        if( !ICV_IS_MAT_OF_TYPE( (nb)->inv_eigen_values[i], CV_32FC1 ) )    \
            CV_ERROR( CV_StsBadArg, "" );                                   \
        if( !ICV_IS_MAT_OF_TYPE( (nb)->cov_rotate_mats[i], CV_32FC1 ) )     \
            CV_ERROR( CV_StsBadArg, "" );                                   \
        if( (k = (nb)->count[i]->cols) > (nb)->dims )                       \
            CV_ERROR( CV_StsBadArg, "" );                                   \
        if( (nb)->count[i]->rows      != 1 ||                               \
            (nb)->sum[i]->rows        != 1 || (nb)->sum[i]->cols != k ||    \
            (nb)->avg[i]->rows        != 1 || (nb)->avg[i]->cols != k ||    \
            (nb)->productsum[i]->rows != k ||                               \
            (nb)->productsum[i]->cols != k ||                               \
            (nb)->inv_eigen_values[i]->rows != 1 ||                         \
            (nb)->inv_eigen_values[i]->cols != k ||                         \
            (nb)->cov_rotate_mats[i]->rows  != k  ||                        \
            (nb)->cov_rotate_mats[i]->cols  != k )                          \
            CV_ERROR( CV_StsBadArg, "" );                                   \
    }                                                                       \
}

/****************************************************************************************\
*                            Main functions declarations                                 *
\****************************************************************************************/
/* Classification of given vector featureVector using NB classifier.
   problt - vector of calculated "probabilities" of the assignement 
   featureVector to the each of the clusters */
float icvPredictNBClassifier (const CvClassifier* classifier,
                       const CvMat* featureVector,
                       CvMat* problt CV_DEFAULT(0));

/* Releases all storages used by NB classifier */
void icvReleaseNBClassifier (CvClassifier** classifier);

/****************************************************************************************\
*                           Inner functions declarations                                 *
\****************************************************************************************/
static void icvCreateNBData( CvNBClassifier* nb, int k );

static void icvUpdateNBData( CvNBClassifier* nb,
                             const float** train_data,
                             const CvMat* responses );

/****************************************************************************************\
*                            Main functions definitions                                  *
\****************************************************************************************/
CV_IMPL
CvStatModel* cvCreateNBClassifier( const CvMat* _train_data,
                int    tflag,
                const CvMat* train_classes,
                const CvClassifierTrainParams* train_params,
                const CvMat* _comp_idx,
                const CvMat* _sample_idx,
                const CvMat* type_mask,
                const CvMat* missed_measurements_mask )
{
    CvNBClassifier* nb = 0;
    CvMat* responses   = 0;
    const float** train_data = 0;

    CV_FUNCNAME( "cvCreateNBClassifier" );
    __BEGIN__;

    int nsamples, dims;

    ICV_ARG_NULL( type_mask );
    ICV_ARG_NULL( missed_measurements_mask );
    ICV_ARG_NULL( train_params );

    CV_CALL( nb = (CvNBClassifier*)cvCreateStatModel (
        CV_CLASSIFIER_MAGIC_VAL | CV_NBAYES_MAGIC_VAL, 
        sizeof(CvNBClassifier), icvReleaseNBClassifier,
        icvPredictNBClassifier, 0 ));

    CV_CALL( cvPrepareTrainData( "cvCreateNBClassifier",
        _train_data, tflag, train_classes, CV_VAR_CATEGORICAL,
        _comp_idx, _sample_idx, false, &train_data,
        &nsamples, &dims, &nb->dims, &responses,
        &nb->cls_labels, &nb->comp_idx ));

    CV_CALL( icvCreateNBData( nb, dims )); //k==cidx_size, n=dims
    CV_CALL( icvUpdateNBData( nb, train_data, responses ));

    __END__;

    cvReleaseMat( &responses );
    cvFree( (void**)&train_data ); 

    return (CvStatModel*)nb;
} // End of cvCreateNBClassifier

/****************************************************************************************/
float icvPredictNBClassifier( const CvStatModel* cl, const CvMat* _sample, CvMat* _probs )
{
    int cls  = 0;
    float* x = 0;
    void* buffer = 0;
    int allocated_buffer = 0;
    CvNBClassifier* nb = (CvNBClassifier*)cl;

    CV_FUNCNAME( "icvPredictNBClassifier" );
    __BEGIN__;

    int i, dims;
    int nclasses;
    double opt = FLT_MAX;
    double* expo_data;
    CvMat expo, diff, t_diff, sample;
    size_t size;

    ICV_IS_NBAYES( nb );

    nclasses = nb->cls_labels->cols;
    dims = nb->avg[0]->cols;

    CV_CALL( cvPreparePredictData( _sample, nb->dims, nb->comp_idx, nclasses, _probs, &x ));
// allocate memory and initializing headers for calculating
    size = sizeof(double) * nclasses + sizeof(float) * dims;
    if( size <= CV_MAX_LOCAL_SIZE )
        buffer = alloca( size );
    else
    {
        CV_CALL( buffer = cvAlloc( size ));
        allocated_buffer = 1;
    }
    CV_CALL( expo   = cvMat( 1, nclasses, CV_64FC1, buffer ));
    CV_CALL( diff   = cvMat( 1, dims, CV_32FC1, (double*)buffer + nclasses ));
    CV_CALL( t_diff = cvMat( dims, 1, CV_32FC1, diff.data.fl ));
    CV_CALL( sample = cvMat( 1, dims, CV_32FC1, x ));

    expo_data = expo.data.db;
    for( i = 0; i < nclasses; i++, expo_data++ )
    {
        double cur;
        CvMat* u = nb->cov_rotate_mats[i];
        CvMat* w = nb->inv_eigen_values[i];
        // cov = u w u'  -->  cov^(-1) = u w^(-1) u'
        CV_CALL(cvSub( nb->avg[i], &sample, &diff ));
        CV_CALL(cvMatMul( u, &t_diff, &t_diff ));
        CV_CALL(cvPow( &t_diff, &t_diff, 2 ));
        CV_CALL(cvMul( &diff, w, &diff ));

        CV_CALL(cur = cvSum( &diff ).val[0] + (double)nb->c[i]);

        *expo_data = cur;
        if( cur < opt )
        {
            cls = i;
            opt = cur;
        }
        /* probability = exp( -0.5 * cur ) */
    }
    if( _probs )
    {
        CV_CALL( cvConvertScale( &expo, &expo, -0.5 ));
        CV_CALL( cvExp( &expo, &expo ));
        if( _probs->cols == 1 )
            CV_CALL( cvReshape( &expo, &expo, 1, nclasses ));
        CV_CALL( cvConvertScale( &expo, _probs, 1./cvSum( &expo ).val[0] ));
    }

    __END__;

    if( x != _sample->data.fl )
        cvFree( (void**)&x );
    if( allocated_buffer )
        cvFree( &buffer );

    return (float)nb->cls_labels->data.i[cls];
} // End of icvPredictNBClassifier

/****************************************************************************************/
void icvReleaseNBClassifier( CvClassifier** cl )
{
    CV_FUNCNAME( "icvReleaseNBClassifier" );
    __BEGIN__;

    int cls;
    CvNBClassifier* nb;

    if( cl == NULL )
        CV_ERROR( CV_StsBadArg, "" );
    if( *cl == NULL ) 
        return;
    if( !CV_IS_NBAYES(*cl) )
        CV_ERROR( CV_StsBadArg, "" );

    nb = (CvNBClassifier*) *cl;
    if( nb->cls_labels )
    {
        for( cls = 0; cls < nb->cls_labels->cols; cls++ )
        {
            CV_CALL( cvReleaseMat( &nb->count[cls] ) );
            CV_CALL( cvReleaseMat( &nb->sum[cls] ) );
            CV_CALL( cvReleaseMat( &nb->productsum[cls] ) );
            CV_CALL( cvReleaseMat( &nb->avg[cls] ) );
            CV_CALL( cvReleaseMat( &nb->inv_eigen_values[cls] ) );
            CV_CALL( cvReleaseMat( &nb->cov_rotate_mats[cls] ) );
        }
    }
    CV_CALL( cvReleaseMat( &nb->cls_labels ) );
    CV_CALL( cvReleaseMat( &nb->comp_idx ) );
    CV_CALL( cvFree( (void**) &nb->count ) );
    CV_CALL( cvFree( (void**) cl ) );

    __END__;

} // End of icvReleaseNBClassifier

/****************************************************************************************\
*                            Inner functions definitions                                 *
\****************************************************************************************/
/* The function creates arrays of <count>, <sum>, <productsum>, <avg>, <inv>, <c> */
void icvCreateNBData( CvNBClassifier* nb, int k )
{
    int  nclasses = nb->cls_labels->cols;

    CV_FUNCNAME( "icvCreateNBData" );
    __BEGIN__;

    int cls;
    const size_t mat_size = sizeof( CvMat* );
    const size_t data_size = nclasses * ( 6 *  mat_size + sizeof( float ) );

    CV_CALL( nb->count = (CvMat**)cvAlloc( data_size ));
    memset( nb->count, 0, data_size );

    nb->sum             = nb->count      + nclasses;
    nb->productsum      = nb->sum        + nclasses;
    nb->avg             = nb->productsum + nclasses;
    nb->inv_eigen_values= nb->avg        + nclasses;
    nb->cov_rotate_mats = nb->inv_eigen_values         + nclasses;
    nb->c               = (float*)(nb->cov_rotate_mats + nclasses);
    for( cls = 0; cls < nclasses; cls++ )
    {
        CV_CALL(nb->count[cls]            = cvCreateMat( 1, k, CV_32SC1 ));
        CV_CALL(nb->sum[cls]              = cvCreateMat( 1, k, CV_32FC1 ));
        CV_CALL(nb->productsum[cls]       = cvCreateMat( k, k, CV_32FC1 ));
        CV_CALL(nb->avg[cls]              = cvCreateMat( 1, k, CV_32FC1 ));
        CV_CALL(nb->inv_eigen_values[cls] = cvCreateMat( 1, k, CV_32FC1 ));
        CV_CALL(nb->cov_rotate_mats[cls]  = cvCreateMat( k, k, CV_32FC1 ));
        CV_CALL(cvZero( nb->count[cls] ));
        CV_CALL(cvZero( nb->sum[cls] ));
        CV_CALL(cvZero( nb->productsum[cls] ));
        CV_CALL(cvZero( nb->avg[cls] ));
        CV_CALL(cvZero( nb->inv_eigen_values[cls] ));
        CV_CALL(cvZero( nb->cov_rotate_mats[cls] ));
    }
    return;

    __END__;
// if error happened, all of the nb's fields must be freed
    if( nb )
    {
        int cls;

        if( nb->count )
            for( cls = 0; cls < nclasses; cls++ )
                cvReleaseMat( &nb->count[cls] );
        if( nb->sum )
            for( cls = 0; cls < nclasses; cls++ )
                cvReleaseMat( &nb->sum[cls] );
        if( nb->productsum )
            for( cls = 0; cls < nclasses; cls++ )
                cvReleaseMat( &nb->productsum[cls] );
        if( nb->avg )
            for( cls = 0; cls < nclasses; cls++ )
                cvReleaseMat( &nb->avg[cls] );
        if( nb->inv_eigen_values )
            for( cls = 0; cls < nclasses; cls++ )
                cvReleaseMat( &nb->inv_eigen_values[cls] );
        if( nb->cov_rotate_mats )
            for( cls = 0; cls < nclasses; cls++ )
                cvReleaseMat( &nb->cov_rotate_mats[cls] );
        cvFree( (void**)&nb->count );
    }
} // End of icvCreateNBData

/****************************************************************************************/
static void icvUpdateNBData( CvNBClassifier* nb, const float** _train_data,
    const CvMat* responses )
{
    CvMat* cov = 0;

    CV_FUNCNAME("icvUpdateNBData");
    __BEGIN__;

    int s, c1, c2;
    int cls;
    int k;
    int l = responses->cols;
    int nclasses = nb->cls_labels->cols;
    int* responses_data = responses->data.i;
    const float min_dispersy = FLT_EPSILON;

    k = nb->avg[0]->cols;
    CV_CALL( cov = cvCreateMat( k, k, CV_32FC1 ));

    /* process train data (count, sum , productsum) */
    for( s = 0; s < l; s++, responses_data++ )
    {
        int* count_data;
        float* sum_data;
        float* prod_data;
        const float* train_data = _train_data[s];
        
        cls = *responses_data;
        count_data = nb->count[cls]->data.i;
        sum_data = nb->sum[cls]->data.fl;
        prod_data = nb->productsum[cls]->data.fl;
        for( c1 = 0; c1 < k; prod_data += (++c1), sum_data++, count_data++ )
        {
            float val1 = train_data[c1];
            (*sum_data) += val1;
            (*count_data)++;
            for( c2 = c1; c2 < k; c2++, prod_data++ )
            {
                double val1val2;
                float val2 = train_data[c2];
                val1val2 = (double)val1 * (double)val2;
                *prod_data += (float)val1val2;
            }
        }
        CV_CALL( cvCompleteSymm( nb->productsum[cls], 0 ));
    }

    /* calculate avg, covariance matrix, c */
    for( cls = 0; cls < nclasses; cls++ )
    {
        double det = 1;
        int i, j;
        CvMat* w = nb->inv_eigen_values[cls];
        float* diag = w->data.fl;

        int* count_data = nb->count[cls]->data.i;
        float* avg_data = nb->avg[cls]->data.fl;
        float* sum1 = nb->sum[cls]->data.fl;
        for( j = 0; j < k; j++, count_data++, avg_data++, sum1++ )
        {
            int count = *count_data;
            *avg_data = count ? *sum1 / count : 0.f;
        }
        count_data = nb->count[cls]->data.i;
        avg_data = nb->avg[cls]->data.fl;
        sum1 = nb->sum[cls]->data.fl;
        for( i = 0; i < k; i++, count_data++, avg_data++, sum1++ )
        {
            float avg1;
            int count;
            float* avg2_data = nb->avg[cls]->data.fl;
            float* sum2 = nb->sum[cls]->data.fl;
            float* prod_data = (float*)(nb->productsum[cls]->data.ptr +
                (size_t)i*nb->productsum[cls]->step);
            float* cov_data = (float*)(cov->data.ptr +
                (size_t)cov->step * i);

            avg1 = *avg_data;
            count = *count_data;
            for( j = 0; j <= i; j++, sum2++, avg2_data++, prod_data++, cov_data++ )
            {
                float avg2, sum12;
                float cov_val;

                avg2  = *avg2_data;
                sum12 = *prod_data;

                cov_val = sum12 - avg1 * (*sum2) - avg2 * (*sum1) + avg1 * avg2 * count;
                cov_val = (count > 1) ? cov_val / (count - 1) : cov_val;
                *cov_data = cov_val;
            }
        }
        CV_CALL( cvCompleteSymm( cov, 1 ));
        CV_CALL(cvSVD( cov, w, nb->cov_rotate_mats[cls], 0, CV_SVD_U_T ));
        CV_CALL(cvMaxS( w, min_dispersy, w ));
        for( j = 0; j < k; j++, diag++ )
            det *= (double)*diag;
        CV_CALL( cvDiv( NULL, w, w ));
        nb->c[cls] = (float) log( det );
    }
    __END__;

    cvReleaseMat( &cov );
} // End of icvUpdateNBData
/*
 * (2*PI)^(-dims/2) * det^(-1/2) * exp( -(1/2)*val ) =
 *   exp[ -(dims/2)*log(2*PI) - (1/2)*log(det) - (1/2)*val ]
 */

/****************************************************************************************/
static void icvWriteNBClassifier( CvFileStorage* fs,
                           const char* name,
                           const void* struct_ptr,
                           CvAttrList /*attributes*/ )
{
    CV_FUNCNAME( "icvWriteNBClassifier" );
    __BEGIN__;

    CvNBClassifier* nb = (CvNBClassifier*)struct_ptr;
    int nclasses, i;

    ICV_IS_NBAYES( nb );
    nclasses = nb->cls_labels->cols;
    
    CV_CALL( cvStartWriteStruct( fs, name, CV_NODE_MAP, CV_TYPE_NAME_ML_NBAYES ));

    CV_CALL( cvWriteInt( fs, "dims", nb->dims ));
    if( nb->comp_idx )
        CV_CALL( cvWrite( fs, "comp_idx", nb->comp_idx ));
    CV_CALL( cvWrite( fs, "cls_labels", nb->cls_labels ));

    CV_CALL( cvStartWriteStruct( fs, "count", CV_NODE_SEQ ));
    for( i = 0; i < nclasses; i++ )
        CV_CALL( cvWrite( fs, NULL, nb->count[i] ));
    CV_CALL( cvEndWriteStruct( fs ));

    CV_CALL( cvStartWriteStruct( fs, "sum", CV_NODE_SEQ ));
    for( i = 0; i < nclasses; i++ )
        CV_CALL( cvWrite( fs, NULL, nb->sum[i] ));
    CV_CALL( cvEndWriteStruct( fs ));
 
    CV_CALL( cvStartWriteStruct( fs, "productsum", CV_NODE_SEQ ));
    for( i = 0; i < nclasses; i++ )
        CV_CALL( cvWrite( fs, NULL, nb->productsum[i] ));
    CV_CALL( cvEndWriteStruct( fs ));

    CV_CALL( cvStartWriteStruct( fs, "avg", CV_NODE_SEQ ));
    for( i = 0; i < nclasses; i++ )
        CV_CALL( cvWrite( fs, NULL, nb->avg[i] ));
    CV_CALL( cvEndWriteStruct( fs ));

    CV_CALL( cvStartWriteStruct( fs, "inv_eigen_values", CV_NODE_SEQ ));
    for( i = 0; i < nclasses; i++ )
        CV_CALL( cvWrite( fs, NULL, nb->inv_eigen_values[i] ));
    CV_CALL( cvEndWriteStruct( fs ));

    CV_CALL( cvStartWriteStruct( fs, "cov_rotate_mats", CV_NODE_SEQ ));
    for( i = 0; i < nclasses; i++ )
        CV_CALL( cvWrite( fs, NULL, nb->cov_rotate_mats[i] ));
    CV_CALL( cvEndWriteStruct( fs ));

    CV_CALL( cvStartWriteStruct( fs, "c", CV_NODE_SEQ ));
    cvWriteRawData( fs, nb->c, nclasses, "f" );
    CV_CALL( cvEndWriteStruct( fs ));

    CV_CALL( cvEndWriteStruct( fs ));

    __END__;
} // End of icvWriteNBClassifier

/****************************************************************************************/
static void* icvReadNBClassifier( CvFileStorage* fs, CvFileNode* root_node )
{
    CvNBClassifier* nb = 0;

    CV_FUNCNAME( "icvReadNBClassifier" );
    __BEGIN__;

    int nclasses, i;
    size_t data_size;
    CvFileNode* node;
    CvSeq* seq;
    CvSeqReader reader;
    
    CV_CALL( nb = (CvNBClassifier*)cvCreateStatModel(
        CV_STAT_MODEL_MAGIC_VAL|CV_NBAYES_MAGIC_VAL,
        sizeof(CvNBClassifier), icvReleaseNBClassifier, icvPredictNBClassifier, 0 ));

    CV_CALL( nb->dims = cvReadIntByName( fs, root_node, "dims", -1 ));
    CV_CALL( nb->comp_idx = (CvMat*)cvReadByName( fs, root_node, "comp_idx" ));
    CV_CALL( nb->cls_labels = (CvMat*)cvReadByName( fs, root_node, "cls_labels" ));
    if( !nb->cls_labels )
        CV_ERROR( CV_StsParseError, "No \"cls_labels\" in NBayes classifier" );
    if( nb->cls_labels->cols < 1 )
        CV_ERROR( CV_StsBadArg, "Number of classes is less 1" );
    if( nb->dims <= 0 )
        CV_ERROR( CV_StsParseError,
        "The field \"dims\" of NBayes classifier is missing" );
    nclasses = nb->cls_labels->cols;

    data_size = nclasses * ( 6 *  sizeof(CvMat*) + sizeof( float ) );
    CV_CALL( nb->count = (CvMat**)cvAlloc( data_size ));
    memset( nb->count, 0, data_size );

    nb->sum = nb->count + nclasses;
    nb->productsum  = nb->sum  + nclasses;
    nb->avg = nb->productsum + nclasses;
    nb->inv_eigen_values = nb->avg + nclasses;
    nb->cov_rotate_mats = nb->inv_eigen_values + nclasses;
    nb->c   = (float*)(nb->cov_rotate_mats + nclasses);


    CV_CALL( node = cvGetFileNodeByName( fs, root_node, "count" ));
    seq = node->data.seq;
    if( !CV_NODE_IS_SEQ(node->tag) || seq->total != nclasses)
        CV_ERROR( CV_StsBadArg, "" );
    CV_CALL( cvStartReadSeq( seq, &reader, 0 ));
    for( i = 0; i < nclasses; i++ )
    {
        CV_CALL( nb->count[i] = (CvMat*)cvRead( fs, (CvFileNode*)reader.ptr ));
        CV_NEXT_SEQ_ELEM( seq->elem_size, reader );
    }

    CV_CALL( node = cvGetFileNodeByName( fs, root_node, "sum" ));
    seq = node->data.seq;
    if( !CV_NODE_IS_SEQ(node->tag) || seq->total != nclasses)
        CV_ERROR( CV_StsBadArg, "" );
    CV_CALL( cvStartReadSeq( seq, &reader, 0 ));
    for( i = 0; i < nclasses; i++ )
    {
        CV_CALL( nb->sum[i] = (CvMat*)cvRead( fs, (CvFileNode*)reader.ptr ));
        CV_NEXT_SEQ_ELEM( seq->elem_size, reader );
    }

    CV_CALL( node = cvGetFileNodeByName( fs, root_node, "productsum" ));
    seq = node->data.seq;
    if( !CV_NODE_IS_SEQ(node->tag) || seq->total != nclasses)
        CV_ERROR( CV_StsBadArg, "" );
    CV_CALL( cvStartReadSeq( seq, &reader, 0 ));
    for( i = 0; i < nclasses; i++ )
    {
        CV_CALL( nb->productsum[i] = (CvMat*)cvRead( fs, (CvFileNode*)reader.ptr ));
        CV_NEXT_SEQ_ELEM( seq->elem_size, reader );
    }

    CV_CALL( node = cvGetFileNodeByName( fs, root_node, "avg" ));
    seq = node->data.seq;
    if( !CV_NODE_IS_SEQ(node->tag) || seq->total != nclasses)
        CV_ERROR( CV_StsBadArg, "" );
    CV_CALL( cvStartReadSeq( seq, &reader, 0 ));
    for( i = 0; i < nclasses; i++ )
    {
        CV_CALL( nb->avg[i] = (CvMat*)cvRead( fs, (CvFileNode*)reader.ptr ));
        CV_NEXT_SEQ_ELEM( seq->elem_size, reader );
    }

    CV_CALL( node = cvGetFileNodeByName( fs, root_node, "inv_eigen_values" ));
    seq = node->data.seq;
    if( !CV_NODE_IS_SEQ(node->tag) || seq->total != nclasses)
        CV_ERROR( CV_StsBadArg, "" );
    CV_CALL( cvStartReadSeq( seq, &reader, 0 ));
    for( i = 0; i < nclasses; i++ )
    {
        CV_CALL( nb->inv_eigen_values[i] = (CvMat*)cvRead( fs, (CvFileNode*)reader.ptr ));
        CV_NEXT_SEQ_ELEM( seq->elem_size, reader );
    }

    CV_CALL( node = cvGetFileNodeByName( fs, root_node, "cov_rotate_mats" ));
    seq = node->data.seq;
    if( !CV_NODE_IS_SEQ(node->tag) || seq->total != nclasses)
        CV_ERROR( CV_StsBadArg, "" );
    CV_CALL( cvStartReadSeq( seq, &reader, 0 ));
    for( i = 0; i < nclasses; i++ )
    {
        CV_CALL( nb->cov_rotate_mats[i] = (CvMat*)cvRead( fs, (CvFileNode*)reader.ptr ));
        CV_NEXT_SEQ_ELEM( seq->elem_size, reader );
    }

    CV_CALL( node = cvGetFileNodeByName( fs, root_node, "c" ));
    if( !CV_NODE_IS_SEQ(node->tag) || node->data.seq->total != nclasses)
        CV_ERROR( CV_StsBadArg, "" );
    CV_CALL( cvReadRawData( fs, node, nb->c, "f" ));

    __END__;

    if( cvGetErrStatus() < 0 && nb )
        nb->release( (CvStatModel**)&nb );
    
    return (void*)nb;
} // End of icvReadNBClassifier

/****************************************************************************************/
static int icvIsNBClassifier( const void* ptr )
{
    return CV_IS_NBAYES( ptr );
}

/****************************************************************************************/
static void icvReleaseNBStatModel( void** ptr )
{
    CV_FUNCNAME( "icvReleaseNBClassifier" );

    __BEGIN__;

    if( !ptr )
        CV_ERROR( CV_StsNullPtr, "NULL double pointer" );
    
    CV_ASSERT( CV_IS_NBAYES( *ptr ) );

    icvReleaseNBClassifier( (CvClassifier**) ptr );    

    *ptr = 0;

    __END__;
} // End of icvReleaseNBClassifier

/****************************************************************************************/
static int icvRegisterNBClassifierType()
{
    CvTypeInfo info;

    info.header_size = sizeof( info );
    info.is_instance = icvIsNBClassifier;
    info.release = icvReleaseNBStatModel;
    info.read = icvReadNBClassifier;
    info.write = icvWriteNBClassifier;
    info.clone = NULL;
    info.type_name = CV_TYPE_NAME_ML_NBAYES;
    cvRegisterType( &info );

    return 1;
} // End of icvRegisterNBClassifierType

static int nb = icvRegisterNBClassifierType();

/* End of file. */

