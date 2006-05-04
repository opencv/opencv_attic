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

bool CvForestTree::train( CvDTreeTrainData* _data, const CvMat* _subsample_idx, CvRTrees* _forest )
{
    bool result = false;
    //CvDTreeTrainData* temp = data;

    CV_FUNCNAME( "CvForestTree::train" );

    __BEGIN__;


    clear();
    forest = _forest;

    data = _data;
    data->shared = true;
    CV_CALL(result = do_train(_subsample_idx));

    __END__;

    //data = temp;
    return result;
}

CvDTreeSplit* CvForestTree::find_best_split( CvDTreeNode* node )
{
    int vi;
    CvDTreeSplit *best_split = 0, *split = 0, *t;

    CV_FUNCNAME("CvForestTree::find_best_split");
    __BEGIN__;

    /*if( !forest )
        CV_ERROR( CV_StsNullPtr, "Invalid <forest> pointer" );*/

    CvMat* active_var_mask = 0;
    if( forest )
    {
        active_var_mask = forest->active_var_mask;
        int var_count = active_var_mask->cols;

        CV_ASSERT( var_count == data->var_count );

        for( vi = 0; vi < var_count; vi++ )
        {
            uchar temp;
            int i1 = cvRandInt(&forest->rng) % var_count;
            int i2 = cvRandInt(&forest->rng) % var_count;
            CV_SWAP( active_var_mask->data.ptr[i1],
                active_var_mask->data.ptr[i2], temp );
        }
    }
    for( vi = 0; vi < data->var_count; vi++ )
    {
        int ci = data->var_type->data.i[vi];
        if( node->num_valid[vi] <= 1 || (active_var_mask && !active_var_mask->data.ptr[vi]) )
            continue;

        if( data->is_classifier )
        {
            if( ci >= 0 )
                split = find_split_cat_gini( node, vi );
            else
                split = find_split_ord_gini( node, vi );
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

//////////////////////////////////////////////////////////////////////////////////////////
//                                  Random trees                                        //
//////////////////////////////////////////////////////////////////////////////////////////

CvRTrees::CvRTrees()
{
    nclasses         = 0;
    oob_error        = 0;
    ntrees           = 0;
    trees            = NULL;
    class_labels     = NULL;
    active_var_mask  = NULL;
    rng = cvRNG(0xffffffff);
}

void CvRTrees::clear()
{
    int k;
    for( k = 0; k < ntrees; k++ )
        delete trees[k];
    cvFree( (void**) &trees );
    cvReleaseMat( &active_var_mask );
    cvReleaseMat( &class_labels );
}

CvRTrees::~CvRTrees()
{
    clear();
}


bool CvRTrees::train( const CvMat* _train_data, int _tflag,
                        const CvMat* _responses, const CvMat* _var_idx,
                        const CvMat* _sample_idx, const CvMat* _var_type,
                        const CvMat* _missing_mask, CvRTParams params )
{
    bool result = false;
    CvMat* sample_idx = 0;
    CvDTreeTrainData* train_data = 0;

    CV_FUNCNAME("CvRTrees::train");
    __BEGIN__;

    int samples_all = 0, var_count = 0;

    CvDTreeParams tree_params( params.max_depth, params.min_sample_count,
        params.regression_accuracy, params.use_surrogates, params.max_categories,
        params.cv_folds, params.use_1se_rule, false, params.priors );
    
    train_data = new CvDTreeTrainData();
    CV_CALL(train_data->set_data( _train_data, _tflag, _responses, _var_idx,
        _sample_idx, _var_type, _missing_mask, tree_params, true,
        &sample_idx, &samples_all ));

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

    if( train_data->is_classifier )
        CV_CALL( class_labels = cvCloneMat( train_data->cat_map ));

    CvMat submask1, submask2;
    cvGetCols( active_var_mask, &submask1, 0, params.nactive_vars );
    cvGetCols( active_var_mask, &submask2, params.nactive_vars, var_count );
    cvSet( &submask1, cvScalar(1) );
    cvZero( &submask2 );

    CV_CALL(result = grow_forest(train_data, sample_idx, samples_all, params.term_crit ));

    result = true;

    __END__;

    cvReleaseMat( &sample_idx );
    //delete train_data;
    return result;
}

bool CvRTrees::grow_forest(
    CvDTreeTrainData* train_data, const CvMat* sample_idx, int nsamples_all,
    const CvTermCriteria term_crit )
{
    bool result = false;

    CvMat* sample_idx_mask_for_tree = 0;
    CvMat* sample_idx_for_tree      = 0;
    CvMat* sample_idx_mask          = 0;

    CvMat* oob_sample_votes	   = 0;
    CvMat* oob_responses       = 0;
    float* oob_samples_ptr     = 0;
    uchar* missing_ptr         = 0;
    float* oob_resp_ptr        = 0;

    CV_FUNCNAME("CvRTrees::grow_forest");
    __BEGIN__;

    const int max_ntrees = term_crit.max_iter;
    const double max_oob_err = term_crit.epsilon;
    
    const int dims     = train_data->var_count;
    const int nsamples = train_data->sample_count;
    const int nclasses = train_data->get_num_classes();

    CvMat oob_predictions_sum = cvMat( 1, nsamples_all, CV_32FC1 );
    CvMat oob_num_of_predictions = cvMat( 1, nsamples_all, CV_32FC1 );

    trees = (CvForestTree**)cvAlloc( sizeof(CvStatModel*)*max_ntrees );
    memset( trees, 0, sizeof(CvStatModel*)*max_ntrees );

    if( train_data->is_classifier )
    {
        CV_CALL(oob_sample_votes = cvCreateMat( nsamples_all, nclasses, CV_32SC1 ));
        cvZero(oob_sample_votes);
    }
    else
    {
        // oob_responses[1,i] = sum of predicted values for the i-th sample
        // oob_responses[2,i] = number of summands (number of predictions for the i-th sample)
        CV_CALL(oob_responses = cvCreateMat( 2, nsamples_all, CV_32FC1 ));
        cvZero(oob_responses);
        cvGetRow( oob_responses, &oob_predictions_sum, 0 );
        cvGetRow( oob_responses, &oob_num_of_predictions, 1 );
    }
    CV_CALL(sample_idx_mask_for_tree = cvCreateMat( 1, nsamples_all, CV_8UC1 ));
    CV_CALL(sample_idx_for_tree      = cvCreateMat( 1, nsamples,     CV_32SC1 ));
    CV_CALL(oob_samples_ptr          = (float*)cvAlloc( sizeof(float)*nsamples*dims ));
    CV_CALL(missing_ptr              = (uchar*)cvAlloc( sizeof(uchar)*nsamples*dims ));
    CV_CALL(oob_resp_ptr             = (float*)cvAlloc( sizeof(float)*nsamples ));

    if( sample_idx )
    {
        int i;

        CV_CALL(sample_idx_mask  = cvCreateMat( 1, nsamples_all, CV_8UC1 ));
        cvZero( sample_idx_mask );
        for( i = 0; i < nsamples; i++ )
            sample_idx_mask->data.ptr[sample_idx->data.i[i]] = 0xFF;
    }

    ntrees = 0;
    
    while( ntrees < max_ntrees )
    {
        int i;
        int oob_samples_count = 0;

        cvZero( sample_idx_mask_for_tree );
        for( i = 0; i < nsamples; i++ ) //form sample for creation one tree
        {
            int idx = cvRandInt( &rng ) % nsamples;
            idx = sample_idx ? sample_idx->data.i[idx] : idx;
            sample_idx_for_tree->data.i[i] = idx;
            sample_idx_mask_for_tree->data.ptr[idx] = 0xFF;
        }

        trees[ntrees] = new CvForestTree();
        CV_CALL(trees[ntrees]->train( train_data, sample_idx_for_tree, this ));

        // form array of OOB samples indices and get these samples
        cvNot( sample_idx_mask_for_tree, sample_idx_mask_for_tree );
        if( sample_idx_mask )
            cvAnd( sample_idx_mask_for_tree, sample_idx_mask, sample_idx_mask_for_tree );
        CV_CALL(train_data->get_vectors( sample_idx_mask_for_tree,
            oob_samples_ptr, missing_ptr, oob_resp_ptr ));

        CvMat oob_sample = cvMat( 1, dims, CV_32FC1, oob_samples_ptr );
        CvMat missing  = cvMat( 1, dims, CV_8UC1,  missing_ptr );

        // predict oob samples
        oob_error = 0;
        for( i = 0; i < nsamples_all; i++ )
        {
            if( !sample_idx_mask_for_tree->data.ptr[i] )
                continue;

            CvDTreeNode* predicted_node;
            CV_CALL(predicted_node = trees[ntrees]->predict(&oob_sample, &missing, true));
            float resp = (float)predicted_node->value;

            if( !train_data->is_classifier )
            {
                oob_predictions_sum.data.fl[i] += resp;
                oob_num_of_predictions.data.fl[i] += 1;

                // compute oob error
                float avg_resp =
                    oob_predictions_sum.data.fl[i]/oob_num_of_predictions.data.fl[i];
                oob_error += powf(avg_resp - oob_resp_ptr[oob_samples_count], 2);
            }
            else
            {
                int class_idx;
                CvPoint max_loc;

                CV_CALL(class_idx = get_class_idx( (int)resp ));
                CV_ASSERT( class_idx >= 0 && class_idx < nclasses );

                CvMat votes;
                cvGetRow(oob_sample_votes, &votes, i);
                votes.data.i[class_idx]++;

                // compute oob error
                cvMinMaxLoc( &votes, 0, 0, 0, &max_loc );

                float prdct_resp = (float)train_data->cat_map->data.i[max_loc.x];
                float true_resp = oob_resp_ptr[oob_samples_count];
                oob_error += (fabs(prdct_resp - true_resp) < FLT_EPSILON) ? 0 : 1;
            }
            oob_sample.data.fl += dims;
            missing.data.ptr  += dims;
            oob_samples_count++;
        }
        if( oob_samples_count > 0 )
            oob_error /= (double)oob_samples_count;

        ntrees++;
        if( term_crit.type != CV_TERMCRIT_ITER && oob_error < max_oob_err )
            break;
    }

    result = true;

    __END__;

    cvReleaseMat( &sample_idx_mask_for_tree );
    cvReleaseMat( &sample_idx_for_tree );
    cvReleaseMat( &sample_idx_mask );
    cvReleaseMat( &oob_sample_votes );
    cvReleaseMat( &oob_responses );
    cvFree( &oob_samples_ptr );
    cvFree( &missing_ptr );
    cvFree( &oob_resp_ptr );

    return result;
}

int CvRTrees::get_class_idx( int label ) const
{
    int idx;
    int* class_labels_ptr = class_labels->data.i;
    for( idx = 0; idx < class_labels->cols; idx++, class_labels_ptr++ )
    {
        if( label == *class_labels_ptr )
            return idx;
    }
    return -1;
}

/* End of file. */
