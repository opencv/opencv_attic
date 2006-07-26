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

const float ord_var_epsilon = FLT_EPSILON*2;
const float ord_nan = FLT_MAX*0.5f;

CvForestTree::CvForestTree()
{
    forest = NULL;
}


CvForestTree::~CvForestTree()
{
    clear();
}


bool CvForestTree::train( CvDTreeTrainData* _data,
                          const CvMat* _subsample_idx,
                          CvRTrees* _forest )
{
    bool result = false;

    CV_FUNCNAME( "CvForestTree::train" );

    __BEGIN__;


    clear();
    forest = _forest;

    data = _data;
    data->shared = true;
    CV_CALL(result = do_train(_subsample_idx));

    __END__;

    return result;
}


CvDTreeSplit* CvForestTree::find_best_split( CvDTreeNode* node )
{
    int vi;
    CvDTreeSplit *best_split = 0, *split = 0, *t;

    CV_FUNCNAME("CvForestTree::find_best_split");
    __BEGIN__;

    CvMat* active_var_mask = 0;
    if( forest )
    {
        int var_count;
        CvRNG* rng = forest->get_rng();

        active_var_mask = forest->get_active_var_mask();
        var_count = active_var_mask->cols;

        CV_ASSERT( var_count == data->var_count );

        for( vi = 0; vi < var_count; vi++ )
        {
            uchar temp;
            int i1 = cvRandInt(rng) % var_count;
            int i2 = cvRandInt(rng) % var_count;
            CV_SWAP( active_var_mask->data.ptr[i1],
                active_var_mask->data.ptr[i2], temp );
        }
    }
    for( vi = 0; vi < data->var_count; vi++ )
    {
        int ci = data->var_type->data.i[vi];
        if( node->num_valid[vi] <= 1
            || (active_var_mask && !active_var_mask->data.ptr[vi]) )
            continue;

        if( data->is_classifier )
        {
            if( ci >= 0 )
                split = find_split_cat_class( node, vi );
            else
                split = find_split_ord_class( node, vi );
        }
        else
        {
            if( ci >= 0 )
                split = find_split_cat_reg( node, vi );
            else
                split = find_split_ord_reg( node, vi );
        }

        if( split )
        {
            if( !best_split || best_split->quality < split->quality )
                CV_SWAP( best_split, split, t );
            if( split )
                cvSetRemoveByPtr( data->split_heap, split );
        }
    }

    __END__;

    return best_split;
}


void CvForestTree::read( CvFileStorage* fs, CvFileNode* fnode, CvRTrees* _forest, CvDTreeTrainData** pdata )
{
    CV_FUNCNAME( "CvForestTree::read" );

    __BEGIN__;

    forest = _forest;

    if( pdata && *pdata )
        data = *pdata;
    else
    {
        CV_CALL(read_train_data_params( fs, fnode ));
        data->shared = true;
        *pdata = data;
    }

    CV_CALL(pruned_tree_idx = cvReadIntByName( fs, fnode, "best_tree_idx", -2 ));
    if( pruned_tree_idx < -1 )
        CV_ERROR( CV_StsParseError, "<best_tree_idx> is absent" );

    CV_CALL(read_tree_nodes( fs, cvGetFileNodeByName( fs, fnode, "nodes" )));

    __END__;
}


void CvForestTree::write( CvFileStorage* fs, const char* name, bool write_td_tp )
{
    CV_FUNCNAME( "CvForestTree::write" );

    __BEGIN__;

    cvStartWriteStruct( fs, name, CV_NODE_MAP );

    if( write_td_tp )
        CV_CALL(write_train_data_params( fs ));

    cvWriteInt( fs, "best_tree_idx", pruned_tree_idx );

    cvStartWriteStruct( fs, "nodes", CV_NODE_SEQ );
    CV_CALL(write_tree_nodes( fs ));
    cvEndWriteStruct( fs ); // nodes

    cvEndWriteStruct( fs ); // name

    __END__;
}

//////////////////////////////////////////////////////////////////////////////////////////
//                                  Random trees                                        //
//////////////////////////////////////////////////////////////////////////////////////////

CvRTrees::CvRTrees()
{
    nclasses         = 0;
    oob_error        = 0;
    ntrees           = 0;
    trees            = NULL;
    active_var_mask  = NULL;
    var_importance   = NULL;
    proximities      = NULL;
    rng = cvRNG(0xffffffff);
    default_model_name = "my_random_trees";
}


void CvRTrees::clear()
{
    int k;
    for( k = 1; k < ntrees; k++ )
        delete trees[k];

    if( trees && *trees )
    {
        trees[0]->share_data( false );
        delete trees[0];
        cvFree( (void**) &trees );
    }

    cvReleaseMat( &active_var_mask );
    cvReleaseMat( &var_importance );
    cvReleaseMat( &proximities );
}


CvRTrees::~CvRTrees()
{
    clear();
}


CvMat* CvRTrees::get_active_var_mask()
{
    return active_var_mask;
}


CvRNG* CvRTrees::get_rng()
{
    return &rng;
}

bool CvRTrees::train( const CvMat* _train_data, int _tflag,
                        const CvMat* _responses, const CvMat* _var_idx,
                        const CvMat* _sample_idx, const CvMat* _var_type,
                        const CvMat* _missing_mask, CvRTParams params )
{
    bool result = false;
    CvDTreeTrainData* train_data = 0;

    CV_FUNCNAME("CvRTrees::train");
    __BEGIN__;

    int var_count = 0;

    clear();

    CvDTreeParams tree_params( params.max_depth, params.min_sample_count,
        params.regression_accuracy, params.use_surrogates, params.max_categories,
        params.cv_folds, params.use_1se_rule, false, params.priors );
    
    train_data = new CvDTreeTrainData();
    CV_CALL(train_data->set_data( _train_data, _tflag, _responses, _var_idx,
        _sample_idx, _var_type, _missing_mask, tree_params, true));

    var_count = train_data->var_count;
    if( params.nactive_vars > var_count )
        params.nactive_vars = var_count;
    else if( params.nactive_vars == 0 )
        params.nactive_vars = (int)sqrt((double)var_count);
    else if( params.nactive_vars < 0 )
        CV_ERROR( CV_StsBadArg, "<nactive_vars> must be non-negative" );
    params.term_crit = cvCheckTermCriteria( params.term_crit, 0.1, 1000 );

    // Create mask of active variables at the tree nodes
    CV_CALL(active_var_mask = cvCreateMat( 1, var_count, CV_8UC1 ));
    if( params.calc_var_importance )
    {
        CV_CALL(var_importance  = cvCreateMat( 1, var_count, CV_32FC1 ));
        cvZero(var_importance);
    }
    if( params.calc_proximities )
    {
        const int n = train_data->sample_count;
        CV_CALL(proximities = cvCreateMat( 1, n*(n-1)/2, CV_32FC1) );
        cvZero( proximities );
    }
    { // initialize active variables mask
        CvMat submask1, submask2;
        cvGetCols( active_var_mask, &submask1, 0, params.nactive_vars );
        cvGetCols( active_var_mask, &submask2, params.nactive_vars, var_count );
        cvSet( &submask1, cvScalar(1) );
        cvZero( &submask2 );
    }

    CV_CALL(result = grow_forest(train_data, params.term_crit ));

    result = true;

    __END__;

    return result;
}


bool CvRTrees::grow_forest( CvDTreeTrainData* train_data, const CvTermCriteria term_crit )
{
    bool result = false;

    CvMat* sample_idx_mask_for_tree = 0;
    CvMat* sample_idx_for_tree      = 0;

    CvMat* oob_sample_votes	   = 0;
    CvMat* oob_responses       = 0;

    float* oob_samples_perm_ptr= 0;

    float* samples_ptr     = 0;
    uchar* missing_ptr     = 0;
    float* true_resp_ptr   = 0;

    CvDTreeNode** predicted_nodes_ptr = 0;

    CV_FUNCNAME("CvRTrees::grow_forest");
    __BEGIN__;

    const int max_ntrees = term_crit.max_iter;
    const double max_oob_err = term_crit.epsilon;
    
    const int dims = train_data->var_count;

    // oob_predictions_sum[i] = sum of predicted values for the i-th sample
    // oob_num_of_predictions[i] = number of summands
    //                            (number of predictions for the i-th sample)
    // initialize these variable to avoid warning C4701
    CvMat oob_predictions_sum = cvMat( 1, 1, CV_32FC1 );
    CvMat oob_num_of_predictions = cvMat( 1, 1, CV_32FC1 );

    nsamples = train_data->sample_count;
    nclasses = train_data->get_num_classes();

    trees = (CvForestTree**)cvAlloc( sizeof(trees[0])*max_ntrees );
    memset( trees, 0, sizeof(trees[0])*max_ntrees );

    if( train_data->is_classifier )
    {
        CV_CALL(oob_sample_votes = cvCreateMat( nsamples, nclasses, CV_32SC1 ));
        cvZero(oob_sample_votes);
    }
    else
    {
        // oob_responses[0,i] = oob_predictions_sum[i]
        //    = sum of predicted values for the i-th sample
        // oob_responses[1,i] = oob_num_of_predictions[i]
        //    = number of summands (number of predictions for the i-th sample)
        CV_CALL(oob_responses = cvCreateMat( 2, nsamples, CV_32FC1 ));
        cvZero(oob_responses);
        cvGetRow( oob_responses, &oob_predictions_sum, 0 );
        cvGetRow( oob_responses, &oob_num_of_predictions, 1 );
    }
    CV_CALL(sample_idx_mask_for_tree = cvCreateMat( 1, nsamples, CV_8UC1 ));
    CV_CALL(sample_idx_for_tree      = cvCreateMat( 1, nsamples, CV_32SC1 ));
    CV_CALL(oob_samples_perm_ptr     = (float*)cvAlloc( sizeof(float)*nsamples*dims ));
    CV_CALL(samples_ptr              = (float*)cvAlloc( sizeof(float)*nsamples*dims ));
    CV_CALL(missing_ptr              = (uchar*)cvAlloc( sizeof(uchar)*nsamples*dims ));
    CV_CALL(true_resp_ptr            = (float*)cvAlloc( sizeof(float)*nsamples ));

    if( proximities )
    {
        CV_CALL(predicted_nodes_ptr  = 
            (CvDTreeNode**)cvAlloc( sizeof(CvDTreeNode*)*nsamples ));
        memset( predicted_nodes_ptr, 0, sizeof(CvDTreeNode*)*nsamples );
    }

    CV_CALL(train_data->get_vectors( 0, samples_ptr, missing_ptr, true_resp_ptr ));

    ntrees = 0;
    while( ntrees < max_ntrees )
    {
        int i, oob_samples_count = 0;
        float ncorrect_responses = 0; // used for estimation of variable importance
        float* true_resp = 0;
        CvMat sample, missing;
        CvDTreeNode** predicted_nodes = 0;
        CvForestTree* tree = 0;

        cvZero( sample_idx_mask_for_tree );
        for( i = 0; i < nsamples; i++ ) //form sample for creation one tree
        {
            int idx = cvRandInt( &rng ) % nsamples;
            sample_idx_for_tree->data.i[i] = idx;
            sample_idx_mask_for_tree->data.ptr[idx] = 0xFF;
        }

        trees[ntrees] = new CvForestTree();
        tree = trees[ntrees];
        CV_CALL(tree->train( train_data, sample_idx_for_tree, this ));

        // form array of OOB samples indices and get these samples
        sample   = cvMat( 1, dims, CV_32FC1, samples_ptr );
        missing  = cvMat( 1, dims, CV_8UC1,  missing_ptr );
        true_resp = true_resp_ptr;
        predicted_nodes = predicted_nodes_ptr;

        oob_error = 0;
        for( i = 0; i < nsamples; i++,
            sample.data.fl += dims, missing.data.ptr += dims, true_resp++ )
        {
            CvDTreeNode* predicted_node = 0;
            if( proximities )
            {
                CV_CALL(predicted_node = tree->predict(&sample, &missing, true));
                *predicted_nodes++ = predicted_node;
            }
            // check if the sample is OOB
            if( sample_idx_mask_for_tree->data.ptr[i] )
                continue;

            // predict oob samples
            if( !predicted_node )
                CV_CALL(predicted_node = tree->predict(&sample, &missing, true));

            if( !train_data->is_classifier )
            {
                float avg_resp, resp = (float)predicted_node->value;
                oob_predictions_sum.data.fl[i] += resp;
                oob_num_of_predictions.data.fl[i] += 1;

                // compute oob error
                avg_resp=oob_predictions_sum.data.fl[i]/oob_num_of_predictions.data.fl[i];
                oob_error += powf(avg_resp - *true_resp, 2);

                ncorrect_responses += powf(resp - *true_resp, 2);
            }
            else //regression
            {
                float prdct_resp;
                CvPoint max_loc;
                CvMat votes;

                cvGetRow(oob_sample_votes, &votes, i);
                votes.data.i[predicted_node->class_idx]++;

                // compute oob error
                cvMinMaxLoc( &votes, 0, 0, 0, &max_loc );

                prdct_resp = (float)train_data->cat_map->data.i[max_loc.x];
                oob_error += (fabs(prdct_resp - *true_resp) < FLT_EPSILON) ? 0 : 1;

                ncorrect_responses += ((int)predicted_node->value == (int)*true_resp);
            }
            oob_samples_count++;
        }
        if( oob_samples_count > 0 )
            oob_error /= (double)oob_samples_count;

        // estimate variable importance
        if( var_importance && oob_samples_count > 0 )
        {
            int m;

            memcpy( oob_samples_perm_ptr, samples_ptr, dims*nsamples*sizeof(float));
            for( m = 0; m < dims; m++ )
            {
                float ncorrect_responses_permuted = 0;
                // randomly permute values of the m-th variable in the oob samples
                float* mth_var_ptr = oob_samples_perm_ptr + m;

                for( i = 0; i < nsamples; i++ )
                {
                    int i1, i2;
                    float temp;

                    if( sample_idx_mask_for_tree->data.ptr[i] ) //the sample is not OOB
                        continue;
                    i1 = cvRandInt( &rng ) % nsamples;
                    i2 = cvRandInt( &rng ) % nsamples;
                    CV_SWAP( mth_var_ptr[i1*dims], mth_var_ptr[i2*dims], temp );

                    // turn values of (m-1)-th variable, that were permuted
                    // at the previous iteration, untouched
                    if( m > 1 )
                        oob_samples_perm_ptr[i*dims+m-1] = samples_ptr[i*dims+m-1];
                }
    
                // predict "permuted" cases and calculate the number of votes for the
                // correct class in the variable-m-permuted oob data
                sample  = cvMat( 1, dims, CV_32FC1, oob_samples_perm_ptr );
                missing = cvMat( 1, dims, CV_8UC1, missing_ptr );
                for( i = 0; i < nsamples; i++,
                    sample.data.fl += dims, missing.data.ptr += dims )
                {
                    float predct_resp, true_resp;

                    if( sample_idx_mask_for_tree->data.ptr[i] ) //the sample is not OOB
                        continue;

                    predct_resp = (float)tree->predict(&sample, &missing, true)->value;
                    true_resp   = true_resp_ptr[i];
                    ncorrect_responses_permuted += train_data->is_classifier ?
                        (int)true_resp == (int)predct_resp 
                        : powf(true_resp - predct_resp, 2);
                }
                var_importance->data.fl[m] += (ncorrect_responses
                    - ncorrect_responses_permuted);
            }
        }
        if( proximities )
        {
            int i, j;
            for( j = 1; j < nsamples; j++ )
                for( i = 0; i < j; i++ )
                    if( predicted_nodes_ptr[i] == predicted_nodes_ptr[j] )
                        proximities->data.fl[(nsamples-1)*i + j-1 - (i*(i+1))/2] += 1.f;
        }
        ntrees++;
        if( term_crit.type != CV_TERMCRIT_ITER && oob_error < max_oob_err )
            break;
    }
    if( var_importance )
        CV_CALL(cvConvertScale( var_importance, var_importance, 1./ntrees/nsamples ));
    if( proximities )
        CV_CALL(cvConvertScale( proximities, proximities, 1./ntrees ));

    result = true;

    __END__;

    cvReleaseMat( &sample_idx_mask_for_tree );
    cvReleaseMat( &sample_idx_for_tree );
    cvReleaseMat( &oob_sample_votes );
    cvReleaseMat( &oob_responses );

    cvFree( &oob_samples_perm_ptr );
    cvFree( &samples_ptr );
    cvFree( &missing_ptr );
    cvFree( &true_resp_ptr );
    cvFree( &predicted_nodes_ptr );

    return result;
}


const CvMat* CvRTrees::get_var_importance()
{
    return var_importance;
}


float CvRTrees::get_proximity( int i, int j ) const
{
    float result = -1;

    CV_FUNCNAME( "CvRTrees::get_proximity" );

    __BEGIN__;

    if( !proximities )
        CV_ERROR( CV_StsBadArg, "The proximities were not processed" );
    if( i < 0 || i >= nsamples || j < 0 || j >= nsamples )
        CV_ERROR( CV_StsBadArg, "Indices are out of range" );

    if( i > j )
    {
        int t;
        CV_SWAP( i, j, t );
    }
    if( i == j )
        result = 1;
    else
        result = proximities->data.fl[(nsamples-1)*i + j-1 - (i*(i+1))/2];

    __END__;

    return result;
}


float CvRTrees::predict( const CvMat* sample, const CvMat* missing ) const
{
    double result = -1;

    CV_FUNCNAME("CvRTrees::predict");
    __BEGIN__;

    int k;

    if( nclasses > 0 ) //classification
    {
        int max_nvotes = 0;
        int* votes = (int*)alloca( sizeof(int)*nclasses );
        memset( votes, 0, sizeof(*votes)*nclasses );
        for( k = 0; k < ntrees; k++ )
        {
            CvDTreeNode* predicted_node = trees[k]->predict( sample, missing );
            int nvotes;
            int class_idx = predicted_node->class_idx;
            CV_ASSERT( 0 <= class_idx && class_idx < nclasses );

            nvotes = ++votes[class_idx];
            if( nvotes > max_nvotes )
            {
                max_nvotes = nvotes;
                result = predicted_node->value;
            }
        }
    }
    else // regression
    {
        result = 0;
        for( k = 0; k < ntrees; k++ )
            result += trees[k]->predict( sample, missing )->value;
        result /= (double)ntrees;
    }

    __END__;

    return (float)result;
}


void CvRTrees::write( CvFileStorage* fs, const char* name )
{
    CV_FUNCNAME( "CvRTrees::write" );

    __BEGIN__;

    int k;

    if( ntrees < 1 || !trees || nsamples < 1 )
        CV_ERROR( CV_StsBadArg, "Invalid CvRTrees object" );

    cvStartWriteStruct( fs, name, CV_NODE_MAP, CV_TYPE_NAME_ML_RTREES );

    cvWriteInt( fs, "nclasses", nclasses );
    cvWriteInt( fs, "nsamples", nsamples );
    cvWriteInt( fs, "nactive_vars", (int)cvSum(active_var_mask).val[0] );
    cvWriteReal( fs, "oob_error", oob_error );

    if( var_importance )
        cvWrite( fs, "var_importance", var_importance );
    if( proximities )
        cvWrite( fs, "proximities",    proximities );

    cvWriteInt( fs, "ntrees", ntrees );

    cvStartWriteStruct( fs, "trees", CV_NODE_SEQ );

    CV_CALL(trees[0]->write( fs, 0, true ));
    for( k = 1; k < ntrees; k++ )
       CV_CALL(trees[k]->write( fs, 0, false ));

    cvEndWriteStruct( fs ); //trees

    cvEndWriteStruct( fs ); //CV_TYPE_NAME_ML_RTREES

    __END__;
}


void CvRTrees::read( CvFileStorage* fs, CvFileNode* fnode )
{
    CV_FUNCNAME( "CvRTrees::read" );

    __BEGIN__;

    int nactive_vars, var_count, k;
    CvSeqReader reader;
    CvFileNode* trees_fnode = 0;
    CvDTreeTrainData* train_data = 0;

    clear();

    nclasses     = cvReadIntByName( fs, fnode, "nclasses", -1 );
    nsamples     = cvReadIntByName( fs, fnode, "nsamples" );
    nactive_vars = cvReadIntByName( fs, fnode, "nactive_vars", -1 );
    oob_error    = cvReadRealByName(fs, fnode, "oob_error", -1 );
    ntrees       = cvReadIntByName( fs, fnode, "ntrees", -1 );

    var_importance = (CvMat*)cvReadByName( fs, fnode, "var_importance" );
    proximities    = (CvMat*)cvReadByName( fs, fnode, "proximities" );

    if( nclasses < 0 || nsamples <= 0 || nactive_vars < 0 || oob_error < 0 || ntrees <= 0)
        CV_ERROR( CV_StsParseError, "Some <nclasses>, <nsamples>, <var_count>, "
        "<nactive_vars>, <oob_error>, <ntrees> of tags are missing" );

    rng = CvRNG( -1 );

    trees = (CvForestTree**)cvAlloc( sizeof(trees[0])*ntrees );
    memset( trees, 0, sizeof(trees[0])*ntrees );

    trees_fnode = cvGetFileNodeByName( fs, fnode, "trees" );
    if( !trees_fnode || !CV_NODE_IS_SEQ(trees_fnode->tag) )
        CV_ERROR( CV_StsParseError, "<trees> tag is missing" );

    cvStartReadSeq( trees_fnode->data.seq, &reader );
    if( reader.seq->total != ntrees )
        CV_ERROR( CV_StsParseError,
        "<ntrees> is not equal to the number of trees saved in file" );
    for( k = 0; k < ntrees; k++ )
    {
        trees[k] = new CvForestTree();
        CV_CALL(trees[k]->read( fs, (CvFileNode*)reader.ptr, this, &train_data ));
        CV_NEXT_SEQ_ELEM( reader.seq->elem_size, reader );
    }

    var_count = trees[0]->get_var_count();
    CV_CALL(active_var_mask = cvCreateMat( 1, var_count, CV_8UC1 ));
    { // initialize active variables mask
        CvMat submask1, submask2;
        cvGetCols( active_var_mask, &submask1, 0, nactive_vars );
        cvGetCols( active_var_mask, &submask2, nactive_vars, var_count );
        cvSet( &submask1, cvScalar(1) );
        cvZero( &submask2 );
    }

    __END__;
}

// End of file.
