/*M///////////////////////////////////////////////////////////////////////////////////////
//
//  IMPORTANT: READ BEFORE DOWNLOADING, COPYING, INSTALLING OR USING.
//
//  By downloading, copying, installing or using the software you agree to this license.
//  ifyou do not agree to this license, do not download, install,
//  copy or use the software.
//
//
//                        Intel License Agreement
//                For Open Source Computer Vision Library
//
// Copyright( C) 2000, Intel Corporation, all rights reserved.
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
//(including, but not limited to, procurement of substitute goods or services;
// loss of use, data, or profits; or business interruption) however caused
// and on any theory of liability, whether in contract, strict liability,
// or tort(including negligence or otherwise) arising in any way out of
// the use of this software, even ifadvised of the possibility of such damage.
//
//M*/

#include "_ml.h"


/*
   CvEMStatModel
 * dims         - samples' dimension.
 * nclusters    - number of clusters to cluster samples to.
 * cov_mat_type - type of covariation matrice (look CvEMStatModelParams).
 * comp_idx     - vector that contains features' indices to process.
 * means        - calculated by the EM algorithm set of gaussians' means.
 * log_weight_div_det - auxilary vector that k-th component is equal to
                        (-2)*ln(weights_k/det(Sigma_k)^0.5),
                        where <weights_k> is the weight,
                        <Sigma_k> is the covariation matrice of k-th cluster.
 * inv_eigen_values   - set of 1*dims matrices, <inv_eigen_values>[k] contains
                        inversed eigen values of covariation matrice of the k-th cluster.
                        In the case of <cov_mat_type> == CV_EM_COV_MAT_DIAGONAL,
                        inv_eigen_values[k] = Sigma_k^(-1).
 * covs_rotate_mats   - used only if cov_mat_type == CV_EM_COV_MAT_GENERAL, in all the
                        other cases it is NULL. <covs_rotate_mats>[k] is the orthogonal
                        matrice, obtained by the SVD-decomposition of Sigma_k.
   Both <inv_eigen_values> and <covs_rotate_mats> fields are used for representation of
   covariation matrices and simplifying EM calculations.
   For fixed k denote
   u = covs_rotate_mats[k],
   v = inv_eigen_values[k],
   w = v^(-1);
   if <cov_mat_type> == CV_EM_COV_MAT_GENERAL, then Sigma_k = u w u',
   else                                             Sigma_k = w.
   Symbol ' means transposition.
 */
typedef struct CvEMStatModel
{
    int dims;
    int nclusters;
    int cov_mat_type;
    CvMat* comp_idx;
    CvMat* log_weight_div_det;
    CvMat* means;
    CvMat** inv_eigen_values;
    CvMat** cov_rotate_mats;
} CvEMStatModel;


/****************************************************************************************\
*                             Functions' declarations                                    *
\****************************************************************************************/

/*  Evaluates given vector <sample> using one m-step of EM iteration algorithm. */
//float icvEMPredict( const CvStatModel* model, const CvMat* sample, CvMat* probs );

/* Releases all storages used by cvEMStatModel. */
//void icvEMRelease( CvStatModel** p_model );

/* Calculates initial parameters of Gauss Mixture (means, cov. matrices and weights) and
   cluster labels by executing cvKMeans2. <samples64> is an auxilary variable. */
void icvFindInitialApproximation( const float** out_train_data, CvMat* samples64,
    int nclusters, const CvMat* imeans,
    CvMat* labels, CvMat* means, CvMat** covs, CvMat* weights );

/* Differs from cvKMeans2 only by the possibility of determining the initial centers. */
void icvKMeans( const CvArr* samples_arr, int cluster_count,
           CvArr* labels_arr, CvTermCriteria termcrit, const CvMat* icenters );

/* Inverses cov. matrices <covs> and stores them in <inv_covs>. By inversing it also
   calculates the determinants of <covs>, if <determinants> != NULL.
   if <correct_singular_cov> == 1, then if some singular value (dispersy) is equal to 0,
   the inversed singular value will be 0, etc. 1 / 0 = 0;
   if <correct_singular_cov> == 0, then the singular values are inversed naturally,
   etc. 1 / 0 = infty. */
//void icvInverseCovs( CvMat** covs, int nclusters, CvMat** inv_covs, CvMat* determinants,
//                    int cov_mat_type );

/* Executes EM iterative algorithm, returns logarithm of likelihood.
   covs[k] = u[k]*w[k]*(u[k])' */
double icvEM( const CvMat* samples, int nclusters,
           CvMat* expects, CvMat* weights, CvMat* log_weight_div_det, CvMat* means,
           CvMat** covs, CvMat** u, CvMat** w, int cov_mat_type, CvTermCriteria term_crit,
           int start_step );
/****************************************************************************************/
#define ICV_CHECK_COVS( covs, nclusters )                                               \
{                                                                                       \
    int i;                                                                              \
    for( i = 0; i < nclusters; i++ )                                                    \
    {                                                                                   \
        int type;                                                                       \
        if( !CV_IS_MAT( (covs)[i] ))                                                    \
            CV_ERROR( CV_StsBadArg, #covs": Invalids matrices" );                       \
        type = CV_MAT_TYPE( (covs)[i]->type );                                          \
        if( type != CV_32FC1 && type != CV_64FC1 )                                      \
            CV_ERROR( CV_StsBadArg, #covs": Type of covariation "                       \
            "matrices should be CV_32FC1 or CV_64FC1" );                                \
        if( (covs)[i]->cols != (covs)[i]->rows )                                        \
            CV_ERROR( CV_StsBadArg, #covs": matrices should be square" );               \
        if( (covs)[i]->cols != dims )                                                   \
            CV_ERROR( CV_StsBadArg, #covs": should accord with comp_idx" );             \
    }                                                                                   \
}
/****************************************************************************************/
#define ICV_EM_MODEL_PARAMS_REQUIRED( p )                                               \
{                                                                                       \
    int k;                                                                              \
    switch( (p).term_crit.type )                                                        \
    {                                                                                   \
    case CV_TERMCRIT_EPS:                                                               \
        if( (p).term_crit.epsilon < 0 )                                                 \
            (p).term_crit.epsilon = 0;                                                  \
        (p).term_crit.max_iter = 10000;                                                 \
        break;                                                                          \
    case CV_TERMCRIT_ITER:                                                              \
        if( (p).term_crit.max_iter < 1 )                                                \
            (p).term_crit.max_iter = 1;                                                 \
        (p).term_crit.epsilon = 1e-6;                                                   \
        break;                                                                          \
    case CV_TERMCRIT_EPS|CV_TERMCRIT_ITER:                                              \
        if( (p).term_crit.epsilon < 0 )                                                 \
            (p).term_crit.epsilon = 0;                                                  \
        if( (p).term_crit.max_iter < 1 )                                                \
            (p).term_crit.max_iter = 1;                                                 \
        break;                                                                          \
    default:                                                                            \
        CV_ERROR( CV_StsBadArg, #p": Invalid termination criteria" );                   \
    }                                                                                   \
    switch( (p).cov_mat_type )                                                          \
    {                                                                                   \
    case CV_EM_COV_MAT_SPHERICAL:                                                       \
        cov_mat_is_spherical = 1;                                                       \
        break;                                                                          \
    case CV_EM_COV_MAT_DIAGONAL:                                                        \
        cov_mat_is_diagonal = 1;                                                        \
        break;                                                                          \
    case CV_EM_COV_MAT_GENERAL:                                                         \
        cov_mat_is_general = 1;                                                         \
        break;                                                                          \
    default:                                                                            \
        CV_ERROR( CV_StsBadArg, #p": Invalid <cov_mat_type>" );                         \
    }                                                                                   \
    switch( (p).start_step )                                                            \
    {                                                                                   \
    case CV_EM_START_M_STEP:                                                            \
        if( (p).weights || (p).means || (p).covs )                                      \
            CV_ERROR( CV_StsBadArg, #p": in the case of start m-step "                  \
                "<weights>, <means> and <covs> should be NULL's" );                     \
        break;                                                                          \
    case CV_EM_START_E_STEP:                                                            \
        if( (p).probs )                                                                 \
            CV_ERROR( CV_StsBadArg, #p": in the case of start e-step "                  \
                "<probs> should be NULL's" );                                           \
        if( !(p).means )                                                                \
            CV_ERROR( CV_StsBadArg, #p": in the case of start e-step "                  \
                "<means> should be not NULL" );                                         \
        break;                                                                          \
    case CV_EM_START_AUTO_STEP:                                                         \
        if( (p).probs || (p).weights || (p).means || (p).covs )                         \
            CV_ERROR( CV_StsBadArg, #p": in the case of start auto-step "               \
                "<probs>, <weights>, <means> and <covs> should be NULL's" );            \
        break;                                                                          \
    default:                                                                            \
        CV_ERROR( CV_StsBadArg, #p": Invalid <start_step>" );                           \
    }                                                                                   \
    if( (p).nclusters < 1 )                                                             \
        CV_ERROR( CV_StsBadArg, #p": Number of clusters should be > 0" );               \
    if( (p).covs )                                                                      \
    {                                                                                   \
        for( k = 0; k < (p).nclusters; k++ )                                            \
        {                                                                               \
            int type, i;                                                                \
            CvMat* cov = (p).covs[k];                                                   \
            if( !CV_IS_MAT( cov ))                                                      \
                CV_ERROR( CV_StsBadArg, "Invalids matrices" );                          \
            type = CV_MAT_TYPE( cov->type );                                            \
            if( type != CV_32FC1 && type != CV_64FC1 )                                  \
                CV_ERROR( CV_StsBadArg, "Type of covariation matrices should be "       \
                "CV_32FC1 or CV_64FC1" );                                               \
            if( cov->rows != cov->cols )                                                \
                CV_ERROR(CV_StsBadArg, "Covariation matrices should be square" );       \
            if( cov->cols != dims )                                                     \
                CV_ERROR( CV_StsBadArg, "Invalid dimensionality of covariance matrices");\
            if( cov_mat_is_diagonal || cov_mat_is_spherical )                           \
            {                                                                           \
                CvMat diag1, diag2;                                                     \
                for( i = 1; i < dims; i++ )                                             \
                {                                                                       \
                    CV_CALL(cvGetDiag( cov, &diag1, i ));                               \
                    CV_CALL(cvGetDiag( cov, &diag2, -i ));                              \
                    CV_CALL(cvAbs(&diag1, &diag1));                                     \
                    CV_CALL(cvAbs(&diag2, &diag2));                                     \
                    if( cvSum(&diag1).val[0] + cvSum(&diag2).val[0] > 10.f*FLT_EPSILON )\
                        CV_ERROR( CV_StsBadArg, "According to <cov_mat_type>, "         \
                        "covariation matrices should be diagonal" );                    \
                }                                                                       \
            }                                                                           \
            if( cov_mat_is_spherical )                                                  \
            {                                                                           \
                CvMat diag;                                                             \
                float disp;                                                             \
                CV_CALL(cvGetDiag( cov, &diag ));                                       \
                disp = (float)(type == CV_32FC1 ? *diag.data.fl :*diag.data.db );       \
                diag.data.ptr += (size_t)diag.step;                                     \
                for( i = 1; i < dims; i++, diag.data.ptr += (size_t)diag.step )         \
                {                                                                       \
                    float x = (float)(type == CV_32FC1 ? *diag.data.fl : *diag.data.db);\
                    if( fabs( disp - x ) > 10.f*FLT_EPSILON )                           \
                        CV_ERROR( CV_StsBadArg, "According to <cov_mat_type>, "         \
                        "covariation matrices should be spherical" );                   \
                }                                                                       \
            }                                                                           \
        }                                                                               \
    }                                                                                   \
    if( (p).weights )                                                                   \
    {                                                                                   \
        int type = CV_MAT_TYPE((p).weights->type);                                      \
        double minval, maxval, sum;                                                     \
        if( type != CV_32FC1 && type != CV_64FC1 )                                      \
            CV_ERROR( CV_StsBadArg, #p": Invalid <weights> type" );                     \
        CV_CALL(cvMinMaxLoc( (p).weights, &minval, &maxval ));                          \
        CV_CALL(sum = cvSum( (p).weights ).val[0]);                                     \
        if( minval < 0 || maxval > 1 )                                                  \
        {                                                                               \
            CV_ERROR( CV_StsBadArg, #p": <weights>:"                                    \
            "one of the elements is not in [0, 1]" );                                   \
        }                                                                               \
        else if( fabs(sum - 1) > FLT_EPSILON )                                          \
            CV_ERROR( CV_StsBadArg, #p": <weights>:"                                    \
            "the sum of the elements is not equal to 1");                               \
    }                                                                                   \
}
/****************************************************************************************/
ML_IMPL void
cvEM(const CvMat*  _samples,
     int tflag,
     CvMat* _labels,
     const CvEMStatModelParams* _params,
     const CvMat*  _comp_idx,
     const CvMat*  _sample_idx,
     CvMat*  _probs,
     CvMat*  _means,
     CvMat*  _weights,
     CvMat** _covs/*,
     CvEMStatModel** em_model*/ )
{
    const float** out_train_data = 0;
    CvMat* samples64             = 0;
    CvMat* sample_idx, *comp_idx = 0;
    CvMat* means = 0, *labels = 0, *probs = 0, *transposed_probs = 0;
    CvMat** covs                 = 0;
    CvMat** cov_rotate_mats      = 0;
    CvMat** covs_eigen_values    = 0;
    CvMat* weights               = 0;
    CvMat* log_weight_div_det    = 0;
    int nclusters                = 0;
    int i;

    if( !_labels && !_probs && !_means && !_covs && !_weights/* && !em_model*/ )
        return;
    /*if( em_model )
        *em_model = 0;*/

    CV_FUNCNAME("cvEM");
    __BEGIN__;

    int nsamples_all, nsamples, dims_all, dims;
    int cov_mat_is_general = 0, cov_mat_is_diagonal = 0, cov_mat_is_spherical = 0;
    CvEMStatModelParams params = *_params;

    CV_CALL(cvPrepareTrainData( "cvEM",
        _samples, tflag, 0, CV_VAR_CATEGORICAL,
        _comp_idx, _sample_idx, false, &out_train_data,
        &nsamples, &dims, &dims_all, 0, 0, &comp_idx, &sample_idx ));

    ICV_EM_MODEL_PARAMS_REQUIRED( params );

    nclusters = params.nclusters;
    if( nsamples <= nclusters )
        CV_ERROR( CV_StsBadArg,"Number of samples should be >"
        "then number of clusters" );
    nsamples_all = (tflag == CV_ROW_SAMPLE) ? _samples->rows : _samples->cols;

    CV_CALL(log_weight_div_det = cvCreateMat( 1, nclusters, CV_64FC1 ));
    CV_CALL(transposed_probs   = cvCreateMat( nsamples, nclusters, CV_64FC1 ));
    CV_CALL(samples64        = cvCreateMat( nsamples,  dims,      CV_64FC1 ));
    CV_CALL(probs            = cvCreateMat( nclusters, nsamples,  CV_64FC1 ));
    CV_CALL(means            = cvCreateMat( nclusters, dims,      CV_64FC1 ));
    CV_CALL(weights          = cvCreateMat( 1,         nclusters, CV_64FC1 ));
    CV_CALL(covs             = (CvMat**)cvAlloc( nclusters * sizeof(*covs) ));
    CV_CALL(cov_rotate_mats  = (CvMat**)cvAlloc( nclusters * sizeof(*cov_rotate_mats) ));
    CV_CALL(covs_eigen_values= (CvMat**)cvAlloc( nclusters * sizeof(*covs_eigen_values) ));
    for( i = 0; i < nclusters; i++ )
    {
        CV_CALL(covs[i]             = cvCreateMat( dims, dims, CV_64FC1 ));
        CV_CALL(cov_rotate_mats[i]  = cvCreateMat( dims, dims, CV_64FC1 ));
        CV_CALL(covs_eigen_values[i] = cvCreateMat( 1,    dims, CV_64FC1 ));
        CV_CALL(cvSetZero( cov_rotate_mats[i] ));
    }
    if( !labels )
        labels = cvCreateMat( 1, nsamples, CV_32SC1 );

    ICV_CONVERT_FLOAT_ARRAY_TO_MATRICE( out_train_data, samples64 );
    // Prepare initial parameters
    if( nclusters == 1 )
    {
        CV_CALL( icvFindInitialApproximation( out_train_data, samples64, nclusters,
        0, labels, means, covs, weights ));
    }
    else if( params.start_step == CV_EM_START_M_STEP )
    {
        CV_CALL(icvCutCols( params.probs, probs, sample_idx, nsamples_all ));
        // Check probs;
        {
            CvMat prob;
            double minval, maxval;
            
            CV_CALL(cvMinMaxLoc( probs, &minval, &maxval ));
            if( minval < 0 || maxval > 1 )
                CV_ERROR( CV_StsBadArg, "Initial <probs>: "
                "all the elements should be in [0, 1]" );
            cvGetCol( probs, &prob, 0 );
            for( i = 0; i < nsamples; i++, prob.data.db ++ )
                if( fabs(cvSum( &prob ).val[0] - 1) > FLT_EPSILON )
                    CV_ERROR( CV_StsBadArg, "Initial <probs>: "
                    "the sum of the each column should be equal to 1" );
        }
    }
    else if( params.start_step == CV_EM_START_E_STEP )
    {
        CV_ASSERT( params.means );
        CV_CALL(icvCutCols( params.means, means, comp_idx, dims_all ));
        if( params.weights && params.covs )
        {
            CV_CALL(cvConvertScale( params.weights, weights ));
            for( i = 0; i < nclusters; i++ )
                CV_CALL( cvConvert( params.covs[i], covs[i] ));
        }
        else
            CV_CALL( icvFindInitialApproximation( out_train_data, samples64, nclusters,
            means, labels, means, covs, weights ));
    }
    else if( params.start_step == CV_EM_START_AUTO_STEP )
    {
        CV_CALL( icvFindInitialApproximation( out_train_data, samples64, nclusters,
            0, labels, means, covs, weights ));
    }
    if( cov_mat_is_diagonal || cov_mat_is_spherical )
    {
        CvMat diag, *cov;
        int j;
        for( i = 0; i < nclusters; i++ )
        {
            cov = covs[i];
            cvGetDiag( cov, &diag, 0 );
            if( cov_mat_is_spherical )
                CV_CALL(cvSet( &diag, cvScalar(cvSum(&diag).val[0] / (double)dims )));
            for( j = 1; j < dims; j++ )
            {
                CV_CALL(cvSetZero( cvGetDiag( cov, &diag, -j )));
                CV_CALL(cvSetZero( cvGetDiag( cov, &diag, j )));
            }
        }
    }
    // Start calculations
    if( nclusters == 1 )
    {
        CvMat diag;
        CV_CALL(cvSet( probs, cvScalar( 1 )));
        CV_CALL(cvSet( transposed_probs, cvScalar( 1 )));

        if( cov_mat_is_general )
        {
            CV_CALL(cvSVD( *covs, *covs_eigen_values, *cov_rotate_mats, 0, CV_SVD_U_T ));
        }
        else if( cov_mat_is_diagonal )
        {
            CV_CALL(cvTranspose( cvGetDiag( *covs, &diag ), *covs_eigen_values ));
        }
        else
        {
            double disp = cvSum( cvGetDiag( *covs, &diag ) ).val[0] / (double)dims;
            disp = MAX( disp, FLT_EPSILON );
            covs_eigen_values[0]->data.db[0] = disp;
            log_weight_div_det->data.db[0] = pow( disp, dims );
        }
        if( cov_mat_is_general || cov_mat_is_diagonal )
        {
            double* w = covs_eigen_values[0]->data.db;
            double det = 1;
            int j;
            CV_CALL(cvMaxS( *covs_eigen_values, FLT_EPSILON, *covs_eigen_values ));
            for( j = 0; j < dims; j++, w++ )
                det *= *w;
            log_weight_div_det->data.db[0] = det;
        }
        cvPow( log_weight_div_det, log_weight_div_det, 0.5 );
        CV_CALL(cvDiv( weights, log_weight_div_det, log_weight_div_det ));
        CV_CALL(cvLog( log_weight_div_det, log_weight_div_det ));
        CV_CALL(cvConvertScale( log_weight_div_det, log_weight_div_det, -2, 0 ));
    }
    else
    {
        CV_CALL( icvEM( samples64, nclusters, probs, weights,
            log_weight_div_det, means, covs, cov_rotate_mats, covs_eigen_values,
            params.cov_mat_type, params.term_crit, params.start_step ));

        CV_CALL(cvTranspose( probs, transposed_probs ));
        if( _labels )
            CV_CALL(icvFindClusterLabels( transposed_probs, 0, 1, labels ));
    }
    // Saving obtained results
    /*if( em_model )
    {
        CvEMStatModel* em = 0;
        size_t size;
    
        CV_CALL( *em_model = (CvEMStatModel*)cvCreateStatModel(
            CV_STAT_MODEL_MAGIC_VAL|CV_EM_MAGIC_VAL,
            sizeof(CvEMStatModel), icvEMRelease, icvEMPredict, 0 ));
        em = *em_model;
        em->dims              = dims_all;
        em->nclusters         = nclusters;
        em->cov_mat_type      = params.cov_mat_type;
        em->comp_idx          = comp_idx;
        CV_CALL( em->means    = cvCreateMat( nclusters, dims, CV_32FC1 ));
        CV_CALL( em->log_weight_div_det  = cvCreateMat( 1, nclusters, CV_32FC1 ));

        CV_CALL(cvConvert( means, em->means ));
        CV_CALL(cvConvert( log_weight_div_det, em->log_weight_div_det ));

        size = nclusters * sizeof(*em->inv_eigen_values);
        CV_CALL( em->inv_eigen_values = (CvMat**)cvAlloc( size ));
        memset( em->inv_eigen_values, 0, size );
        for( i = 0; i < nclusters; i++ )
        {
            CV_CALL(em->inv_eigen_values[i] = cvCreateMat( 1, dims, CV_32FC1 ));
            CV_CALL(cvDiv( NULL, covs_eigen_values[i], covs_eigen_values[i] ));
            CV_CALL(cvConvert( covs_eigen_values[i], em->inv_eigen_values[i] ));
        }
        if( params.cov_mat_type == CV_EM_COV_MAT_GENERAL )
        {
            size = nclusters * sizeof(*em->cov_rotate_mats);
            CV_CALL(em->cov_rotate_mats = (CvMat**)cvAlloc( size ));
            memset( em->cov_rotate_mats, 0, size );
            for( i = 0; i < nclusters; i++ )
            {
                CV_CALL(em->cov_rotate_mats[i] = cvCreateMat( dims, dims, CV_32FC1 ));
                CV_CALL(cvConvert( cov_rotate_mats[i], em->cov_rotate_mats[i] ));
            }
        }
    }*/

    CV_CALL(cvWritebackLabels( labels, _labels, means, _means, transposed_probs, _probs,
        sample_idx, nsamples_all, comp_idx, dims_all ));
    if( _weights )
    {
        CV_ASSERT(ICV_IS_MAT_OF_TYPE(_weights, CV_32FC1)
               || ICV_IS_MAT_OF_TYPE(_weights, CV_64FC1));
        CV_CALL(cvConvert( weights, _weights ));
    }
    if( _covs )
    {
        ICV_CHECK_COVS( _covs, nclusters );
        for( i = 0; i < nclusters; i++ )
            CV_CALL( cvConvert( covs[i], _covs[i] ));
    }

    __END__;

    cvFree( (void**)&out_train_data );
    cvReleaseMat(&samples64);
    cvReleaseMat(&sample_idx);
    cvReleaseMat(&means);
    cvReleaseMat(&labels);
    cvReleaseMat(&probs);
    cvReleaseMat(&transposed_probs);
    cvReleaseMat(&weights);
    cvReleaseMat(&log_weight_div_det);
    //if( !em_model || (cvGetErrStatus() < 0) )
        cvReleaseMat( &comp_idx );
    if( covs )
    {
        for( i = 0; i < nclusters; i++ )
            cvReleaseMat( &covs[i] );
        cvFree( (void**) &covs );
    }
    if( cov_rotate_mats )
    {
        for( i = 0; i < nclusters; i++ )
            cvReleaseMat( &cov_rotate_mats[i] );
        cvFree( (void**) &cov_rotate_mats );
    }
    if( covs_eigen_values )
    {
        for( i = 0; i < nclusters; i++ )
            cvReleaseMat( &covs_eigen_values[i] );
        cvFree( (void**) &covs_eigen_values );
    }
    /*if( cvGetErrStatus() < 0 )
        cvReleaseStatModel( (CvStatModel**)&em_model );*/
} // End of cvEM

/****************************************************************************************/
#if 0
float
icvEMPredict( const CvStatModel* model, const CvMat* _sample, CvMat* _probs )
{
    float* sample_data   = 0;
    void* buffer         = 0;
    int allocated_buffer = 0;
    int cls              = 0;

    CV_FUNCNAME( "icvEMPredict" );
    __BEGIN__;

    int k, dims;
    int nclusters;
    int is_general = 0, is_diagonal = 0, is_spherical = 0;
    double opt = FLT_MAX;
    double* expo_data;
    float* lwd = 0;
    size_t size;
    CvMat sample, mean, diff, t_diff, expo;
    CvEMStatModel* em = (CvEMStatModel*)model;

// check input parameters
    if( !CV_IS_EM(em) )
        CV_ERROR( CV_StsBadArg,"Invalid EM statmodel pointer" );
    if( em->cov_mat_type == CV_EM_COV_MAT_GENERAL )
        is_general  = 1;
    else if( em->cov_mat_type == CV_EM_COV_MAT_DIAGONAL )
        is_diagonal = 1;
    else if( em->cov_mat_type == CV_EM_COV_MAT_SPHERICAL )
        is_spherical  = 1;
    else
        CV_ERROR( CV_StsBadArg,"Invalid value of <cov_mat_type>" );

    nclusters = em->nclusters;
    dims = em->means->cols;
    if( nclusters <= 0 || dims <= 0)
        CV_ERROR( CV_StsBadArg,"Invalid <nclusters> or <dims> fields" );

    for( k = 0; k < nclusters; k++ )
    {
        if( !ICV_IS_MAT_OF_TYPE(em->inv_eigen_values[k], CV_32FC1) ||
            em->inv_eigen_values[k]->rows != 1 ||
            em->inv_eigen_values[k]->cols != dims )
            CV_ERROR( CV_StsBadArg,"Invalid <inv_eigen_values> matrice" );
        if( is_general )
            if( !ICV_IS_MAT_OF_TYPE(em->cov_rotate_mats[k], CV_32FC1) ||
                em->cov_rotate_mats[k]->rows != dims ||
                em->cov_rotate_mats[k]->cols != dims )
                CV_ERROR( CV_StsBadArg,"Invalid <cov_rotate_mats> matrice" );
    }
    if( !ICV_IS_MAT_OF_TYPE(em->means, CV_32FC1) ||
        em->means->rows != nclusters ||
        em->means->cols != dims )
        CV_ERROR( CV_StsBadArg,"Invalid <means> matrice" );
    if( !ICV_IS_MAT_OF_TYPE(em->log_weight_div_det, CV_32FC1) ||
        em->log_weight_div_det->rows != 1 ||
        em->log_weight_div_det->cols != nclusters )
        CV_ERROR( CV_StsBadArg,"Invalid <log_weight_div_det> matrice" );

    CV_CALL( cvPreparePredictData( _sample, em->dims, em->comp_idx, nclusters, _probs, &sample_data ));

// allocate memory and initializing headers for calculating
    size = sizeof(double) * nclusters + sizeof(float) * dims;
    if( size <= CV_MAX_LOCAL_SIZE )
        buffer = alloca( size );
    else
    {
        CV_CALL( buffer = cvAlloc( size ));
        allocated_buffer = 1;
    }
    CV_CALL( expo   = cvMat( 1, nclusters, CV_64FC1, buffer ));
    CV_CALL( diff   = cvMat( 1, dims, CV_32FC1, (double*)buffer + nclusters ));
    CV_CALL( t_diff = cvMat( dims, 1, CV_32FC1, diff.data.fl ));
    CV_CALL( sample = cvMat( 1, dims, CV_32FC1, sample_data ));

// calculate the probabilities
    expo_data = expo.data.db;
    lwd = em->log_weight_div_det->data.fl;
    CV_CALL(cvGetRow( em->means, &mean, 0 ));
    for( k = 0; k < nclusters; k++, expo_data++, mean.data.fl += dims, lwd++ )
    {
        double cur;
        CvMat* u = em->cov_rotate_mats ? em->cov_rotate_mats[k] : 0;
        CvMat* w = em->inv_eigen_values[k];
        // cov = u w u'  -->  cov^(-1) = u w^(-1) u'
        CV_CALL(cvSub( &mean, &sample, &diff ));
        if( is_general )
            cvMatMul(u, &t_diff, &t_diff);
        CV_CALL(cvPow( &t_diff, &t_diff, 2 ));
        if( is_spherical )
            cur = cvSum( &diff ).val[0] * w->data.fl[0];
        else
        {
            CV_CALL(cvMul( &diff, w, &diff ));
            cur = cvSum( &diff ).val[0];
        }

        cur += (double)*lwd;
        *expo_data = cur;
        if( cur < opt )
        {
            cls = k;
            opt = cur;
        }
        /* probability = (2*pi)^(-dims/2)*exp( -0.5 * cur ) */
    }
    if( _probs )
    {
        CV_CALL( cvConvertScale( &expo, &expo, -0.5 ));
        CV_CALL( cvExp( &expo, &expo ));
        if( _probs->cols == 1 )
            CV_CALL( cvReshape( &expo, &expo, 0, nclusters ));
        CV_CALL( cvConvertScale( &expo, _probs, 1./cvSum( &expo ).val[0] ));
    }

    __END__;

    if( sample_data != _sample->data.fl )
        cvFree( (void**)&sample_data );
    if( allocated_buffer )
        cvFree( &buffer );

    return (float)cls;
} // End of icvEMPredict

/****************************************************************************************/
void
icvEMRelease( CvStatModel** p_model )
{
    CV_FUNCNAME("icvEMRelease");
    __BEGIN__;

    int k, nclusters;
    CvMat** w = 0;
    CvEMStatModel* em = 0;

    if( !p_model )
        CV_ERROR( CV_StsBadArg, "NULL double pointer" );

    em = (CvEMStatModel*)*p_model;
    if( !em )
        return;
    if( !CV_IS_EM(em) )
        CV_ERROR( CV_StsBadArg,"Invalid EM statmodel pointer" );

    cvReleaseMat( &em->comp_idx );
    cvReleaseMat( &em->means );
    cvReleaseMat( &em->log_weight_div_det );

    nclusters = em->nclusters;

    w = em->inv_eigen_values;
    for( k = 0; k < nclusters; k++ )
        cvReleaseMat( &w[k] );
    cvFree( (void**) em->inv_eigen_values );

    if( em->cov_rotate_mats )
    {
        CvMat** u = em->cov_rotate_mats;
        for( k = 0; k < nclusters; k++ )
            cvReleaseMat( &u[k] );
        cvFree( (void**) em->cov_rotate_mats );
    }

    cvFree( (void**)p_model );
    
    __END__;
} // End of icvEMRelease
#endif

/****************************************************************************************/
/* log_weight_div_det[k] = -2*log(weights_k) + log(det(Sigma_k)))

   covs[k] = cov_rotate_mats[k] * eigen_values[k] * (cov_rotate_mats[k])'
   cov_rotate_mats[k] are orthogonal matrices of eigen vectors and
   eigen_values[k] are diagonal matrices (represented by 1D vectors) of eigen values.

   The probability <alpha_ki> of belonging vector x_i to the k-th cluster is proportional to:
   weights_k * exp{ -0.5[ln(det(Sigma_k)) + (x_i - mu_k)' Sigma_k^(-1) (x_i - mu_k)] }
   We calculate these probabilities here by the equivalent formulae:
   Denote
   S_ki = -0.5(ln(det(Sigma_k)) + (x_i - mu_k)' Sigma_k^(-1) (x_i - mu_k)) + ln(weights_k),
   M_i = max_k S_ki = S_qi, so that the q-th class is the one where maximum reaches. Then
   alpha_ki = exp{ S_ki - M_i } / ( 1 + sum_j!=q exp{ S_ji - M_i })
*/
double icvEM(const CvMat* samples, int nclusters,
           CvMat* expects, CvMat* weights, CvMat* log_weight_div_det, CvMat* means,
           CvMat** covs, CvMat** cov_rotate_mats, CvMat** eigen_values, int cov_mat_type,
           CvTermCriteria term_crit, int start_step )
{
    CvMat* centered_sample = 0;
    CvMat* covs_item         = 0;
    void* buffer             = 0;
    int allocated_buffer     = 0;
    int k;
    double log_likely_hood = -DBL_MAX;

    CV_FUNCNAME("icvEM");
    __BEGIN__;

    const int nsamples          = samples->rows;
    const int dims              = samples->cols;
    const double min_dispersy   = FLT_EPSILON;
    const double min_det_value  = MAX( DBL_MIN, pow( min_dispersy, dims ));
    const double likelyhood_addition = -CV_LOG2PI * (double)nsamples * (double)dims / 2.;
    
    int n = 0, j;
    int is_general = 0, is_diagonal = 0, is_spherical = 0;
    double pred_log_likely_hood = -DBL_MAX / 1000.;
    size_t size;
    CvMat log_det, log_weights, t_centered_sample;
    CvMat sample, mean, expect;

    if( log_weight_div_det )
        CV_ASSERT( ICV_IS_MAT_OF_TYPE(log_weight_div_det, CV_64FC1) );
    CV_ASSERT( ICV_IS_MAT_OF_TYPE(weights,  CV_64FC1) );
    CV_ASSERT( ICV_IS_MAT_OF_TYPE(samples,  CV_64FC1) );
    CV_ASSERT( ICV_IS_MAT_OF_TYPE(expects,  CV_64FC1) );
    CV_ASSERT( ICV_IS_MAT_OF_TYPE(means,    CV_64FC1) );
    CV_ASSERT( ICV_IS_MAT_OF_TYPE(covs[0],  CV_64FC1) );
    CV_ASSERT( ICV_IS_MAT_OF_TYPE(cov_rotate_mats[0], CV_64FC1) );
    CV_ASSERT( ICV_IS_MAT_OF_TYPE(eigen_values[0], CV_64FC1) );

    if( cov_mat_type == CV_EM_COV_MAT_GENERAL )
        is_general  = 1;
    else if( cov_mat_type == CV_EM_COV_MAT_DIAGONAL )
        is_diagonal = 1;
    else if( cov_mat_type == CV_EM_COV_MAT_SPHERICAL )
        is_spherical  = 1;
    /* In the case of <cov_mat_type> == CV_EM_COV_MAT_DIAGONAL, the vector eigen_values[k]
    contains the diagonal elements (dispersies). In the case of
    <cov_mat_type> == CV_EM_COV_MAT_SPHERICAL - the 0-ths elements of the vectors eigen_values[k]
    are to be equal to the average of the dimensions dispersies. */

    size = 2 * sizeof(double) * nclusters;
    if( size <= CV_MAX_LOCAL_SIZE )
        buffer = alloca( size );
    else
    {
        CV_CALL( buffer = cvAlloc( size ));
        allocated_buffer = 1;
    }
    CV_CALL(log_det     = cvMat( 1, nclusters, CV_64FC1, (double*)buffer ));
    CV_CALL(log_weights = cvMat( 1, nclusters, CV_64FC1, (double*)buffer + nclusters));

    CV_CALL(covs_item         = cvCreateMat( dims, dims, CV_64FC1 ));
    CV_CALL(centered_sample   = cvCreateMat( 1, dims, CV_64FC1 ));
    CV_CALL(t_centered_sample = cvMat( dims, 1, CV_64FC1, centered_sample->data.ptr ));

    CV_CALL(cvGetRow( samples, &sample, 0 ));
    CV_CALL(cvGetRow( means, &mean, 0 ));
    CV_CALL(cvGetRow( expects, &expect, 0 ));

    if( start_step == CV_EM_START_M_STEP )
        goto m_step;

    for( k = 0; k < nclusters; k++ )
    {
        CvMat* w = eigen_values[k];
        CvMat diag;

        if( is_general || is_diagonal )
        {
            double det = 1;
            double* w_data = w->data.db;

            if( is_general )
            {
                CV_CALL(cvSVD( covs[k], w, cov_rotate_mats[k], 0, CV_SVD_U_T ));
            }
            else
                CV_CALL(cvTranspose( cvGetDiag( covs[k], &diag ), w ));
            for( j = 0; j < dims; j++, w_data++ )
                det *= *w_data;
            if( det < min_det_value )
            {
                if( start_step == CV_EM_START_AUTO_STEP )
                    det = min_det_value;
                else
                    CV_ERROR( CV_StsBadArg,"Covariation matrice is singular" );
            }
            log_det.data.db[k] = det;
        }
        else
        {
            double disp = cvSum( cvGetDiag( covs[k], &diag ) ).val[0] / (double)dims;
            if( disp < min_dispersy )
            {
                if( start_step == CV_EM_START_AUTO_STEP )
                    disp = min_dispersy;
                else
                    CV_ERROR( CV_StsBadArg,"Covariation matrice is singular" );
            }
            CV_CALL(cvSet( w, cvRealScalar(disp)));
            log_det.data.db[k] = disp;
        }
    }
    CV_CALL(cvLog( &log_det, &log_det ));
    if( is_spherical )
        CV_CALL(cvScale( &log_det, &log_det, dims ));

    while( n++ < term_crit.max_iter )
    {
        int i;
        double *expects_data, *weights_data, *log_weights_data, *log_det_data;

        sample.data.ptr = samples->data.ptr;
        mean.data.ptr   = means->data.ptr;
        expect.data.ptr = expects->data.ptr;

        // e-step
        // calculate (x_i - mu_k)' Sigma_k^(-1) (x_i - mu_k)
        expects_data = expects->data.db;
        mean.data.db = means->data.db;
        for( k = 0; k < nclusters; k++, mean.data.db += dims, expect.data.db += nsamples )
        {
            CvMat* w = eigen_values[k];
            CvMat* u = cov_rotate_mats[k];

            sample.data.db = samples->data.db;
            for( i = 0; i < nsamples; i++, expects_data++, sample.data.db += dims )
            {
                CV_CALL(cvSub( &sample, &mean, centered_sample ));
                if( is_general )
                    cvMatMul(u, &t_centered_sample, &t_centered_sample);
                CV_CALL(cvPow( centered_sample, centered_sample, 2 ));
                if( !is_spherical )
                    CV_CALL(cvDiv( centered_sample, w, centered_sample ));
                *expects_data = cvSum( centered_sample ).val[0];
            }
            if( is_spherical )
                CV_CALL(cvScale( &expect, &expect, 1. / w->data.db[0] ));
        }
        // log(det(Sigma_k)) + (x_i - mu_k)' Sigma_k^(-1) (x_i - mu_k)
        log_det_data = log_det.data.db;
        cvGetRow( expects, &expect, 0 );
        for( k = 0; k < nclusters; k++, expect.data.db += nsamples, log_det_data++ )
            CV_CALL(cvAddS( &expect, cvRealScalar( *log_det_data), &expect ));

        // S_ki = -0.5[log(det(Sigma_k)) + (x_i - mu_k)' Sigma_k^(-1) (x_i - mu_k)] + log(weights_k)
        CV_CALL(cvScale( expects, expects, -0.5, 0 ));
        CV_CALL(cvLog( weights, &log_weights ));
        log_weights_data = log_weights.data.db;
        cvGetRow( expects, &expect, 0 );
        for( k = 0; k < nclusters; k++, expect.data.db += nsamples, log_weights_data++ )
            CV_CALL(cvAddS( &expect, cvRealScalar( *log_weights_data), &expect ));

        CV_CALL(cvGetCol( expects, &expect, 0 ));
        for( i = 0; i < nsamples; i++, expect.data.db++ )
        {
            double min, max;
            // S_ki = S_ki - max_j S_ji
            CV_CALL(cvMinMaxLoc( &expect, &min, &max ));
            CV_CALL(cvSubS( &expect, cvRealScalar( max ), &expect ));
        }
        CV_CALL(cvExp( expects, expects )); // exp( S_ki )
        // alpha_ki = exp( S_ki ) / sum_j exp( S_ji )
        CV_CALL(cvGetCol( expects, &expect, 0 ));
        log_likely_hood = 0;
        for( i = 0; i < nsamples; i++, expect.data.db++ )
        {
            const double sum = cvSum(&expect).val[0];

            CV_ASSERT( sum >= 1. );
            CV_CALL(cvScale( &expect, &expect, 1. / sum ));
            log_likely_hood += log( sum );
        }
        log_likely_hood += likelyhood_addition;

        // check termination criteria
        if( fabs( (log_likely_hood - pred_log_likely_hood) / pred_log_likely_hood )
            < term_crit.epsilon )
            break;
        pred_log_likely_hood = log_likely_hood;
m_step:
        // m-step
        CV_CALL(cvMatMul( expects, samples, means ));
        CV_CALL(cvGetRow( expects, &expect, 0 ));
        expects_data = expects->data.db;
        mean.data.db = means->data.db;
        weights_data = weights->data.db;
        log_det_data = log_det.data.db;
        for( k = 0; k < nclusters; k++, mean.data.db += dims, expect.data.db += nsamples, log_det_data++ )
        {
            double sum, inv_sum;
            CvMat* cov = covs[k];
            CvMat* w = eigen_values[k];

            CV_CALL(sum = MAX(cvSum( &expect ).val[0], DBL_MIN));
            inv_sum = 1 / sum;

            //process weights
            *weights_data++ = sum;
            //process means
            CV_CALL(cvScale( &mean, &mean, inv_sum ));
            // normalize expects to obtain more accurate result
            CV_CALL(cvScale(&expect, &expect, inv_sum));
            //process covs
            CV_CALL(cvSetZero( cov ));
            CV_CALL(cvSetZero( w ));
            sample.data.db = samples->data.db;
            for( i = 0; i < nsamples; i++, sample.data.db += dims, expects_data++ )
            {
                if( is_general )
                {
                    CV_CALL(cvMulTransposed( &sample, covs_item, 1, &mean ));
                    CV_CALL(cvScaleAdd( covs_item, cvRealScalar( *expects_data ), cov, cov ));
                    CV_ASSERT(*expects_data >= 0);
                }
                else
                {
                    CV_CALL(cvSub( &sample, &mean, centered_sample ));
                    CV_CALL(cvPow( centered_sample, centered_sample, 2 ));
                    CV_CALL(cvScaleAdd( centered_sample, cvRealScalar( *expects_data ), w, w ));
                }
            }
            if( is_spherical )
            {
                const double disp = MAX(cvSum( w ).val[0] / (double)dims, min_dispersy);
                w->data.db[0] = disp;
                *log_det_data = disp;
            }
            else
            {
                double *diag = w->data.db;
                double det = 1;

                if( is_general )
                    CV_CALL(cvSVD( cov, w, cov_rotate_mats[k], 0, CV_SVD_U_T ));
                CV_CALL(cvMaxS( w, min_dispersy, w ));
                for( j = 0; j < dims; j++, diag++ )
                    det *= *diag;
                *log_det_data = det;
            }
        }
        CV_CALL(cvConvertScale( weights, weights, 1. / (double)nsamples, 0 ));
        CV_CALL(cvMaxS( weights, DBL_MIN, weights ));

        CV_CALL(cvLog( &log_det, &log_det ));
        if( is_spherical )
            CV_CALL(cvScale( &log_det, &log_det, dims ));
    } // end of iteration process
    //log_weight_div_det[k] = -2*log(weights_k/det(Sigma_k))^0.5) = -2*log(weights_k) + log(det(Sigma_k)))
    if( log_weight_div_det )
    {
        CV_CALL(cvScale( &log_weights, log_weight_div_det, -2 ));
        CV_CALL(cvAdd( log_weight_div_det, &log_det, log_weight_div_det ));
    }
/*  Fill all the cov. mats with its values:
    1) if <cov_mat_type> == CV_EM_COV_MAT_DIAGONAL we used array of <w> as diagonals.
       Now w[k] should be copied back to the diagonals of covs[k];
    2) if <cov_mat_type> == CV_EM_COV_MAT_SPHERICAL we used the 0-th element of w[k]
       as an average dispersy in each cluster. The value of the 0-th element of w[k]
       should be copied to the all of the diagonal elements of covs[k]. */
    if( is_spherical )
    {
        CvMat diag;
        CV_CALL(cvGetDiag( *covs, &diag, 0 ));
        for( k = 0; k < nclusters; k++ )
        {
            CvMat* cov = covs[k];
            CvMat* w = eigen_values[k];
            const double disp = w->data.db[0];
            diag.data.db = cov->data.db;
            CV_CALL(cvSet( &diag, cvRealScalar(disp)));
            CV_CALL(cvSet( w, cvRealScalar( disp )));
        }
    }
    else if( is_diagonal )
    {
        CvMat diag;
        CV_CALL(cvGetDiag( *covs, &diag ));
        for( k = 0; k < nclusters; k++ )
        {
            diag.data.db = covs[k]->data.db;
            CV_CALL(cvTranspose( eigen_values[k], &diag));
        }
    }

    __END__;

    cvReleaseMat( &centered_sample );
    cvReleaseMat( &covs_item );
    if( allocated_buffer )
        cvFree( &buffer );

    return log_likely_hood;
} // End of icvEM

/****************************************************************************************/
void icvFindInitialApproximation( const float** out_train_data, CvMat* samples64,
    int nclusters, const CvMat* imeans,
    CvMat* labels, CvMat* means, CvMat** covs, CvMat* weights )
{
    int allocated_class_ranges = 0;
    int* class_ranges = 0;
    CvMat* samples32  = 0;
    CvMat** samples_arr = 0;
    int nsamples = 0;
    int i;

    CV_FUNCNAME("icvFindInitialApproximation");
    __BEGIN__;

    const size_t class_ranges_size = (nclusters + 1) * sizeof(*class_ranges);
    double* weights_data = weights->data.db;
    int dims = samples64->cols;
    int j;

    CV_ASSERT(ICV_IS_MAT_OF_TYPE(labels, CV_32SC1));
    CV_ASSERT(ICV_IS_MAT_OF_TYPE(samples64, CV_64FC1));
    CV_ASSERT(ICV_IS_MAT_OF_TYPE(weights, CV_64FC1));
    CV_ASSERT(ICV_IS_MAT_OF_TYPE(means, CV_64FC1));
    CV_ASSERT(means->rows == nclusters);

    nsamples = samples64->rows;

    CV_CALL( samples_arr = (CvMat**)cvAlloc( nsamples * sizeof(*samples_arr) ));
    for( i = 0; i < nsamples; i++ )
        CV_CALL( samples_arr[i] = cvCreateMatHeader( 1, dims, CV_64FC1 ));

    if( nclusters == 1 )
    {
        for( i = 0; i < nsamples; i++ )
            samples_arr[i]->data.db = samples64->data.db + i*dims;
        CV_CALL( cvCalcCovarMatrix( (const CvArr**)samples_arr, nsamples, covs[0],
            means, CV_COVAR_NORMAL | CV_COVAR_SCALE ));
        *weights_data = 1;
        CV_CALL(cvZero( labels ));
    }
    else if( nclusters == nsamples )
    {
        CV_CALL(cvCopy( samples64, means ));
        CV_CALL(cvSet( weights, cvRealScalar( 1. / (double)nclusters ) ));
        for( i = 0; i < nclusters; i++ )
        {
            CV_CALL(cvZero( covs[i] ));
            labels->data.i[i] = i;
        }
    }
    else
    {
        if( class_ranges_size <= CV_MAX_LOCAL_SIZE )
            class_ranges = (int*)alloca( class_ranges_size );
        else
        {
            allocated_class_ranges = 1;
            CV_CALL( class_ranges = (int*)cvAlloc( class_ranges_size ));
        }
        CV_CALL( samples32 = cvCreateMat( nsamples, dims, CV_32FC1 ));
        ICV_CONVERT_FLOAT_ARRAY_TO_MATRICE( out_train_data, samples32 );
        
        if( imeans )
        {
            CV_CALL( icvKMeans( samples32, nclusters, labels,
                cvTermCriteria( CV_TERMCRIT_ITER, 1, 0.5 ), imeans ));
        }
        else
            CV_CALL( cvKMeans2( samples32, nclusters, labels,
            cvTermCriteria( CV_TERMCRIT_ITER|CV_TERMCRIT_EPS, 5, 0.5 ) ));
        CV_CALL( cvSortSamplesByClasses( out_train_data, labels, class_ranges ));
        
        ICV_CONVERT_FLOAT_ARRAY_TO_MATRICE( out_train_data, samples64 );
        for( i = 0; i < nclusters; i++, weights_data++ )
        {
            int left = class_ranges[i], right = class_ranges[i+1];
            int cluster_size = right - left;
            CvMat avg;
            
            CV_ASSERT( cluster_size > 0 );
            for( j = left; j < right; j++ )
                samples_arr[j - left]->data.db = samples64->data.db + j*dims;
            
            CV_CALL( cvGetRow( means, &avg, i ));
            CV_CALL( cvCalcCovarMatrix( (const CvArr**)samples_arr, cluster_size, covs[i],
                &avg, CV_COVAR_NORMAL | CV_COVAR_SCALE ));
            *weights_data = (double)cluster_size / (double)nsamples;
        }
        CV_CALL( cvConvert( samples32, samples64 ) );
    }

    __END__;

    if( allocated_class_ranges )
        cvFree( (void**)&class_ranges );
    cvReleaseMat( &samples32 );
    for( i = 0; i < nsamples; i++ )
    {
        samples_arr[i]->data.ptr = 0;
        cvReleaseMat( samples_arr + i );
    }
    cvFree( (void**) samples_arr );
} // End of icvFindInitialApproximation

/****************************************************************************************/
void icvKMeans( const CvArr* samples_arr, int cluster_count,
           CvArr* labels_arr, CvTermCriteria termcrit, const CvMat* icenters )
{
    CvMat* centers = 0;
    CvMat* old_centers = 0;
    CvMat* counters = 0;
    
    CV_FUNCNAME( "icvKMeans" );

    __BEGIN__;

    CvMat samples_stub, *samples = (CvMat*)samples_arr;
    CvMat cluster_idx_stub, *labels = (CvMat*)labels_arr;
    CvMat* temp = 0;
    CvRNG rng = cvRNG(-1);
    int i, j, k, sample_count, dims;
    int ids_delta, iter;
    double max_dist;
    int pix_size;

    if( !CV_IS_MAT( samples ))
        CV_CALL( samples = cvGetMat( samples, &samples_stub ));
    
    if( !CV_IS_MAT( labels ))
        CV_CALL( labels = cvGetMat( labels, &cluster_idx_stub ));

    if( cluster_count < 1 )
        CV_ERROR( CV_StsOutOfRange, "Number of clusters should be positive" );

    if( CV_MAT_DEPTH(samples->type) != CV_32F || CV_MAT_TYPE(labels->type) != CV_32SC1 )
        CV_ERROR( CV_StsUnsupportedFormat,
        "samples should be floating-point matrix, cluster_idx - integer vector" );

    pix_size = CV_ELEM_SIZE(samples->type);

    if( labels->rows != 1 && (labels->cols != 1 || !CV_IS_MAT_CONT(labels->type)) ||
        labels->rows + labels->cols - 1 != samples->rows )
        CV_ERROR( CV_StsUnmatchedSizes,
        "cluster_idx should be 1D vector of the same number of elements as samples' number of rows" ); 

    switch( termcrit.type )
    {
    case CV_TERMCRIT_EPS:
        if( termcrit.epsilon < 0 )
            termcrit.epsilon = 0;
        termcrit.max_iter = 100;
        break;
    case CV_TERMCRIT_ITER:
        if( termcrit.max_iter < 1 )
            termcrit.max_iter = 1;
        termcrit.epsilon = 1e-6;
        break;
    case CV_TERMCRIT_EPS|CV_TERMCRIT_ITER:
        if( termcrit.epsilon < 0 )
            termcrit.epsilon = 0;
        if( termcrit.max_iter < 1 )
            termcrit.max_iter = 1;
        break;
    default:
        CV_ERROR( CV_StsBadArg, "Invalid termination criteria" );
    }

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
    CV_ASSERT(ICV_IS_MAT_OF_TYPE(icenters, CV_32FC1) || ICV_IS_MAT_OF_TYPE(icenters, CV_64FC1));
    CV_CALL(cvConvert( icenters, centers ));

    counters->cols = cluster_count; // cut down counters
    max_dist = termcrit.epsilon*2;

    for( iter = 0; iter < termcrit.max_iter; iter++ )
    {
        int i, j, k;

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

        // computer centers
        CV_SWAP( centers, old_centers, temp );
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

        if( max_dist < termcrit.epsilon )
            break;
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
} // End of icvKMeans

#if 0
/****************************************************************************************/
static int icvIsEMModel( const void* ptr )
{
    return CV_IS_EM(ptr);
} // End of icvIsEMModel

/****************************************************************************************/
static void icvReleaseEMModel( void** ptr )
{
    CV_FUNCNAME("icvReleaseEMModel");

    __BEGIN__;

    CvEMStatModel* em = 0;

    if( !ptr )
        CV_ERROR( CV_StsNullPtr, "NULL double pointer" );
    
    CV_ASSERT( CV_IS_EM(*ptr) );

    em = (CvEMStatModel*) *ptr;
    em->release( (CvStatModel**)ptr );

    __END__;
} // End of icvReleaseEMModel

/****************************************************************************************/
static void* icvReadEMModel( CvFileStorage* fs, CvFileNode* root_node )
{
    CvEMStatModel* em = 0;

    CV_FUNCNAME("icvReadEMModel");
    __BEGIN__;

    CvFileNode* node = 0;
    CvSeq* seq = 0;
    CvSeqReader reader;
    int k;
    size_t data_size;

    CV_CALL(em = (CvEMStatModel*)cvCreateStatModel(
        CV_STAT_MODEL_MAGIC_VAL|CV_EM_MAGIC_VAL,
        sizeof(CvEMStatModel), icvEMRelease, icvEMPredict, 0 ));

    CV_CALL(em->dims         = cvReadIntByName( fs, root_node, "dims", -1 ));
    CV_CALL(em->nclusters    = cvReadIntByName( fs, root_node, "nclusters", -1 ));
    CV_CALL(em->cov_mat_type = cvReadIntByName( fs, root_node, "cov_mat_type", -1 ));

    if( em->dims <= 0 || em->nclusters <= 0 || em->cov_mat_type < 0 )
        CV_ERROR( CV_StsParseError,
        "Some of \"nclusters\", \"cov_mat_type\" or \"dims\" fields"
        "of em model are missing" );

    CV_CALL(em->comp_idx           = (CvMat*)cvReadByName( fs, root_node, "comp_idx" ));
    CV_CALL(em->log_weight_div_det = (CvMat*)cvReadByName( fs, root_node, "log_weight_div_det" ));
    CV_CALL(em->means              = (CvMat*)cvReadByName( fs, root_node, "means" ));
    if( !em->means || !em->log_weight_div_det )
        CV_ERROR(CV_StsParseError, "No \"means\" or \"log_weight_div_det\" in EM model");

    data_size = em->nclusters * sizeof(CvMat*);
    CV_CALL( em->inv_eigen_values = (CvMat**)cvAlloc( data_size ));
    memset( em->inv_eigen_values, 0, data_size );

    CV_CALL( node = cvGetFileNodeByName( fs, root_node, "inv_eigen_values" ));
    seq = node->data.seq;
    if( !CV_NODE_IS_SEQ(node->tag) || seq->total != em->nclusters)
        CV_ERROR( CV_StsBadArg, "" );
    CV_CALL( cvStartReadSeq( seq, &reader, 0 ));
    for( k = 0; k < em->nclusters; k++ )
    {
        CV_CALL( em->inv_eigen_values[k] = (CvMat*)cvRead( fs, (CvFileNode*)reader.ptr ));
        CV_NEXT_SEQ_ELEM( seq->elem_size, reader );
    }

    if( em->cov_mat_type == CV_EM_COV_MAT_GENERAL )
    {
        data_size = em->nclusters * sizeof(CvMat*);
        CV_CALL( em->cov_rotate_mats = (CvMat**)cvAlloc( data_size ));
        memset( em->cov_rotate_mats, 0, data_size );

        CV_CALL( node = cvGetFileNodeByName( fs, root_node, "cov_rotate_mats" ));
        seq = node->data.seq;
        if( !CV_NODE_IS_SEQ(node->tag) || seq->total != em->nclusters)
            CV_ERROR( CV_StsBadArg, "" );
        CV_CALL( cvStartReadSeq( seq, &reader, 0 ));
        for( k = 0; k < em->nclusters; k++ )
        {
            CV_CALL( em->cov_rotate_mats[k] = (CvMat*)cvRead( fs, (CvFileNode*)reader.ptr ));
            CV_NEXT_SEQ_ELEM( seq->elem_size, reader );
        }
    }

    __END__;

    if( cvGetErrStatus() < 0 && em )
        em->release( (CvStatModel**)&em );
    
    return (void*)em;
} // End of icvReadEMModel

/****************************************************************************************/
static void
icvWriteEMModel( CvFileStorage* fs, const char* name,
                  const void* struct_ptr, CvAttrList /*attr*/ )
                                   
{
    CV_FUNCNAME ("icvWriteEMModel");
    __BEGIN__;

    CvEMStatModel* em = (CvEMStatModel*)struct_ptr;
    int k;
    
    if( !CV_IS_EM(em) )
        CV_ERROR( CV_StsBadArg, "Invalid pointer" );

    CV_CALL( cvStartWriteStruct( fs, name, CV_NODE_MAP, CV_TYPE_NAME_ML_EM ));

    cvWriteInt( fs, "dims", em->dims );
    cvWriteInt( fs, "nclusters", em->nclusters );
    cvWriteInt( fs, "cov_mat_type", em->cov_mat_type );

    if( em->comp_idx )
        CV_CALL( cvWrite( fs, "comp_idx", em->comp_idx ));
    CV_CALL( cvWrite( fs, "log_weight_div_det", em->log_weight_div_det ));
    CV_CALL( cvWrite( fs, "means", em->means ));

    CV_CALL( cvStartWriteStruct( fs, "inv_eigen_values", CV_NODE_SEQ ));
    for( k = 0; k < em->nclusters; k++ )
        CV_CALL( cvWrite( fs, NULL, em->inv_eigen_values[k] ));
    CV_CALL( cvEndWriteStruct( fs ));
    if( em->cov_rotate_mats )
    {
        CV_CALL( cvStartWriteStruct( fs, "cov_rotate_mats", CV_NODE_SEQ ));
        for( k = 0; k < em->nclusters; k++ )
            CV_CALL( cvWrite( fs, NULL, em->cov_rotate_mats[k] ));
        CV_CALL( cvEndWriteStruct( fs ));
    }

    cvEndWriteStruct( fs );

    __END__;
} // End of icvWriteEMModel

static int icvRegisterEMStatModelType()
{
    CvTypeInfo info;

    info.header_size = sizeof( info );
    info.is_instance = icvIsEMModel;
    info.release = icvReleaseEMModel;
    info.read = icvReadEMModel;
    info.write = icvWriteEMModel;
    info.clone = NULL;
    info.type_name = CV_TYPE_NAME_ML_EM;
    cvRegisterType( &info );

    return 1;
} // End of icvRegisterEMStatModelType

static int em = icvRegisterEMStatModelType();

#endif

// End of file
