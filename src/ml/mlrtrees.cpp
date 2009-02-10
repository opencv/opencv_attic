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


bool
CvForestTree::train( const CvMat*, int, const CvMat*, const CvMat*,
                    const CvMat*, const CvMat*, const CvMat*, CvDTreeParams )
{
    assert(0);
    return false;
}


bool
CvForestTree::train( CvDTreeTrainData*, const CvMat* )
{
    assert(0);
    return false;
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


void CvForestTree::read( CvFileStorage* fs, CvFileNode* fnode, CvRTrees* _forest, CvDTreeTrainData* _data )
{
    CvDTree::read( fs, fnode, _data );
    forest = _forest;
}


void CvForestTree::read( CvFileStorage*, CvFileNode* )
{
    assert(0);
}

void CvForestTree::read( CvFileStorage* _fs, CvFileNode* _node,
                         CvDTreeTrainData* _data )
{
    CvDTree::read( _fs, _node, _data );
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
    data             = NULL;
    active_var_mask  = NULL;
    var_importance   = NULL;
    rng = cvRNG(0xffffffff);
    default_model_name = "my_random_trees";
}


void CvRTrees::clear()
{
    int k;
    for( k = 0; k < ntrees; k++ )
        delete trees[k];
    cvFree( &trees );

    delete data;
    data = 0;

    cvReleaseMat( &active_var_mask );
    cvReleaseMat( &var_importance );
    ntrees = 0;
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

    CV_FUNCNAME("CvRTrees::train");
    __BEGIN__
    int var_count = 0;

    clear();

    CvDTreeParams tree_params( params.max_depth, params.min_sample_count,
        params.regression_accuracy, params.use_surrogates, params.max_categories,
        params.cv_folds, params.use_1se_rule, false, params.priors );

    data = new CvDTreeTrainData();
    CV_CALL(data->set_data( _train_data, _tflag, _responses, _var_idx,
        _sample_idx, _var_type, _missing_mask, tree_params, true));

    var_count = data->var_count;
    if( params.nactive_vars > var_count )
        params.nactive_vars = var_count;
    else if( params.nactive_vars == 0 )
        params.nactive_vars = (int)sqrt((double)var_count);
    else if( params.nactive_vars < 0 )
        CV_ERROR( CV_StsBadArg, "<nactive_vars> must be non-negative" );

    // Create mask of active variables at the tree nodes
    CV_CALL(active_var_mask = cvCreateMat( 1, var_count, CV_8UC1 ));
    if( params.calc_var_importance )
    {
        CV_CALL(var_importance  = cvCreateMat( 1, var_count, CV_32FC1 ));
        cvZero(var_importance);
    }
    { // initialize active variables mask
        CvMat submask1, submask2;
        cvGetCols( active_var_mask, &submask1, 0, params.nactive_vars );
        cvGetCols( active_var_mask, &submask2, params.nactive_vars, var_count );
        cvSet( &submask1, cvScalar(1) );
        cvZero( &submask2 );
    }

    CV_CALL(result = grow_forest( params.term_crit ));

    result = true;

    __END__
    return result;
    
}


bool CvRTrees::grow_forest( const CvTermCriteria term_crit )
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

    CV_FUNCNAME("CvRTrees::grow_forest");
    __BEGIN__;

    const int max_ntrees = term_crit.max_iter;
    const double max_oob_err = term_crit.epsilon;

    const int dims = data->var_count;
    float maximal_response = 0;

#define RF_OOB
#ifdef RF_OOB
    // oob_predictions_sum[i] = sum of predicted values for the i-th sample
    // oob_num_of_predictions[i] = number of summands
    //                            (number of predictions for the i-th sample)
    // initialize these variable to avoid warning C4701
    CvMat oob_predictions_sum = cvMat( 1, 1, CV_32FC1 );
    CvMat oob_num_of_predictions = cvMat( 1, 1, CV_32FC1 );
#endif
    nsamples = data->sample_count;
    nclasses = data->get_num_classes();

    trees = (CvForestTree**)cvAlloc( sizeof(trees[0])*max_ntrees );
    memset( trees, 0, sizeof(trees[0])*max_ntrees );

#ifdef RF_OOB
    if( data->is_classifier )
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
#endif
    CV_CALL(sample_idx_mask_for_tree = cvCreateMat( 1, nsamples, CV_8UC1 ));
    CV_CALL(sample_idx_for_tree      = cvCreateMat( 1, nsamples, CV_32SC1 ));
#ifdef RF_OOB
    CV_CALL(oob_samples_perm_ptr     = (float*)cvAlloc( sizeof(float)*nsamples*dims ));
    CV_CALL(samples_ptr              = (float*)cvAlloc( sizeof(float)*nsamples*dims ));
    CV_CALL(missing_ptr              = (uchar*)cvAlloc( sizeof(uchar)*nsamples*dims ));
    CV_CALL(true_resp_ptr            = (float*)cvAlloc( sizeof(float)*nsamples ));

    CV_CALL(data->get_vectors( 0, samples_ptr, missing_ptr, true_resp_ptr ));
    {
        double minval, maxval;
        CvMat responses = cvMat(1, nsamples, CV_32FC1, true_resp_ptr);
        cvMinMaxLoc( &responses, &minval, &maxval );
        maximal_response = (float)MAX( MAX( fabs(minval), fabs(maxval) ), 0 );
    }
#endif

    ntrees = 0;
    while( ntrees < max_ntrees )
    {
        int i, oob_samples_count = 0;
        double ncorrect_responses = 0; // used for estimation of variable importance
        CvForestTree* tree = 0;

        cvZero( sample_idx_mask_for_tree );
        for(i = 0; i < nsamples; i++ ) //form sample for creation one tree
        {
            int idx = cvRandInt( &rng ) % nsamples;
            sample_idx_for_tree->data.i[i] = idx;
            sample_idx_mask_for_tree->data.ptr[idx] = 0xFF;
        }

        trees[ntrees] = new CvForestTree();
        tree = trees[ntrees];
        CV_CALL(tree->train( data, sample_idx_for_tree, this ));

#ifdef RF_OOB
        CvMat sample, missing;
        // form array of OOB samples indices and get these samples
        sample   = cvMat( 1, dims, CV_32FC1, samples_ptr );
        missing  = cvMat( 1, dims, CV_8UC1,  missing_ptr );

        oob_error = 0;
        for( i = 0; i < nsamples; i++,
            sample.data.fl += dims, missing.data.ptr += dims )
        {
            CvDTreeNode* predicted_node = 0;
            // check if the sample is OOB
            if( sample_idx_mask_for_tree->data.ptr[i] )
                continue;

            // predict oob samples
            if( !predicted_node )
                CV_CALL(predicted_node = tree->predict(&sample, &missing, true));

            if( !data->is_classifier ) //regression
            {
                double avg_resp, resp = predicted_node->value;
                oob_predictions_sum.data.fl[i] += (float)resp;
                oob_num_of_predictions.data.fl[i] += 1;

                // compute oob error
                avg_resp = oob_predictions_sum.data.fl[i]/oob_num_of_predictions.data.fl[i];
                avg_resp -= true_resp_ptr[i];
                oob_error += avg_resp*avg_resp;
                resp = (resp - true_resp_ptr[i])/maximal_response;
                ncorrect_responses += exp( -resp*resp );
            }
            else //classification
            {
                double prdct_resp;
                CvPoint max_loc;
                CvMat votes;

                cvGetRow(oob_sample_votes, &votes, i);
                votes.data.i[predicted_node->class_idx]++;

                // compute oob error
                cvMinMaxLoc( &votes, 0, 0, 0, &max_loc );

                prdct_resp = data->cat_map->data.i[max_loc.x];
                oob_error += (fabs(prdct_resp - true_resp_ptr[i]) < FLT_EPSILON) ? 0 : 1;

                ncorrect_responses += cvRound(predicted_node->value - true_resp_ptr[i]) == 0;
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
                double ncorrect_responses_permuted = 0;
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
                    double predct_resp, true_resp;

                    if( sample_idx_mask_for_tree->data.ptr[i] ) //the sample is not OOB
                        continue;

                    predct_resp = tree->predict(&sample, &missing, true)->value;
                    true_resp   = true_resp_ptr[i];
                    if( data->is_classifier )
                        ncorrect_responses_permuted += cvRound(true_resp - predct_resp) == 0;
                    else
                    {
                        true_resp = (true_resp - predct_resp)/maximal_response;
                        ncorrect_responses_permuted += exp( -true_resp*true_resp );
                    }
                }
                var_importance->data.fl[m] += (float)(ncorrect_responses
                    - ncorrect_responses_permuted);
            }
        }
#endif //RF_OOB
        ntrees++;
        if( term_crit.type != CV_TERMCRIT_ITER && oob_error < max_oob_err )
            break;
    }
    if( var_importance )
        CV_CALL(cvConvertScale( var_importance, var_importance, 1./ntrees/nsamples ));

    result = true;

    __END__;

    cvReleaseMat( &sample_idx_mask_for_tree );
    cvReleaseMat( &sample_idx_for_tree );

#ifdef RF_OOB
    cvReleaseMat( &oob_sample_votes );
    cvReleaseMat( &oob_responses );

    cvFree( &oob_samples_perm_ptr );
    cvFree( &samples_ptr );
    cvFree( &missing_ptr );
    cvFree( &true_resp_ptr );
#endif

    return result;
}


const CvMat* CvRTrees::get_var_importance()
{
    return var_importance;
}


float CvRTrees::get_proximity( const CvMat* sample1, const CvMat* sample2,
                              const CvMat* missing1, const CvMat* missing2 ) const
{
    float result = 0;

    //CV_FUNCNAME( "CvRTrees::get_proximity" );

    __BEGIN__;

    int i;
    for( i = 0; i < ntrees; i++ )
        result += trees[i]->predict( sample1, missing1 ) ==
        trees[i]->predict( sample2, missing2 ) ?  1 : 0;
    result = result/(float)ntrees;

    __END__;

    return result;
}


float CvRTrees::get_train_error()
{
    float err = -1;

    CV_FUNCNAME("CvRTrees::get_train_error");
    __BEGIN__;

    int sample_count = data->sample_count;
    int var_count = data->var_count;

    float *values_ptr = (float*)cvAlloc( sizeof(float)*sample_count*var_count );
    uchar *missing_ptr = (uchar*)cvAlloc( sizeof(uchar)*sample_count*var_count );
    float *responses_ptr = (float*)cvAlloc( sizeof(float)*sample_count );

    data->get_vectors( 0, values_ptr, missing_ptr, responses_ptr);
    
    if (data->is_classifier)
    {
        int err_count = 0;
        float *vp = values_ptr;
        uchar *mp = missing_ptr;    
        for (int si = 0; si < sample_count; si++, vp += var_count, mp += var_count)
        {
            CvMat sample = cvMat( 1, var_count, CV_32FC1, vp );
            CvMat missing = cvMat( 1, var_count, CV_8UC1,  mp );
            float r = predict( &sample, &missing );
            if (fabs(r - responses_ptr[si]) >= FLT_EPSILON)
                err_count++;
        }
        err = (float)err_count / (float)sample_count;
    }
    else
        CV_ERROR( CV_StsBadArg, "This method is not supported for regression problems" );
    
    cvFree( &values_ptr );
    cvFree( &missing_ptr );
    cvFree( &responses_ptr ); 

     __END__;

    return err;
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

float CvRTrees::predict_prob( const CvMat* sample, const CvMat* missing) const
{
    double result = -1;
	
    CV_FUNCNAME("CvRTrees::predict_prob");
    __BEGIN__;
	
    int k;
	
	if( nclasses == 2 ) //classification
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
		
		return float(votes[1])/ntrees;
    }
    else // regression
    {
		CV_ERROR(CV_StsBadArg, "This function works for binary classification problems only...");
    }
	
    __END__;
	
    return -1;
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

    cvWriteInt( fs, "ntrees", ntrees );

    CV_CALL(data->write_params( fs ));

    cvStartWriteStruct( fs, "trees", CV_NODE_SEQ );

    for( k = 0; k < ntrees; k++ )
    {
        cvStartWriteStruct( fs, 0, CV_NODE_MAP );
        CV_CALL( trees[k]->write( fs ));
        cvEndWriteStruct( fs );
    }

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

    clear();

    nclasses     = cvReadIntByName( fs, fnode, "nclasses", -1 );
    nsamples     = cvReadIntByName( fs, fnode, "nsamples" );
    nactive_vars = cvReadIntByName( fs, fnode, "nactive_vars", -1 );
    oob_error    = cvReadRealByName(fs, fnode, "oob_error", -1 );
    ntrees       = cvReadIntByName( fs, fnode, "ntrees", -1 );

    var_importance = (CvMat*)cvReadByName( fs, fnode, "var_importance" );

    if( nclasses < 0 || nsamples <= 0 || nactive_vars < 0 || oob_error < 0 || ntrees <= 0)
        CV_ERROR( CV_StsParseError, "Some <nclasses>, <nsamples>, <var_count>, "
        "<nactive_vars>, <oob_error>, <ntrees> of tags are missing" );

    rng = CvRNG( -1 );

    trees = (CvForestTree**)cvAlloc( sizeof(trees[0])*ntrees );
    memset( trees, 0, sizeof(trees[0])*ntrees );

    data = new CvDTreeTrainData();
    data->read_params( fs, fnode );
    data->shared = true;

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
        CV_CALL(trees[k]->read( fs, (CvFileNode*)reader.ptr, this, data ));
        CV_NEXT_SEQ_ELEM( reader.seq->elem_size, reader );
    }

    var_count = data->var_count;
    CV_CALL(active_var_mask = cvCreateMat( 1, var_count, CV_8UC1 ));
    {
        // initialize active variables mask
        CvMat submask1, submask2;
        cvGetCols( active_var_mask, &submask1, 0, nactive_vars );
        cvGetCols( active_var_mask, &submask2, nactive_vars, var_count );
        cvSet( &submask1, cvScalar(1) );
        cvZero( &submask2 );
    }

    __END__;
}


int CvRTrees::get_tree_count() const
{
    return ntrees;
}

CvForestTree* CvRTrees::get_tree(int i) const
{
    return (unsigned)i < (unsigned)ntrees ? trees[i] : 0;
}


///

CvDTreeSplit* CvForestERTree::find_best_split( CvDTreeNode* node )
{
    int vi;
    CvDTreeSplit *best_split = 0, *split = 0, *t;

    CV_FUNCNAME("CvForestERTree::find_best_split");
    __BEGIN__;

    CvMat* active_var_mask = 0;
    int n = node->sample_count;
    int* best_l_sample_idx = (int*)cvAlloc(n*sizeof(best_l_sample_idx[0]));
    int best_ln = 0;
    int* best_r_sample_idx = (int*)cvAlloc(n*sizeof(best_r_sample_idx[0]));
    int best_rn = 0;
    int* tsidx = 0;

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
        if( /*node->get_num_valid(vi) <= 1
            || */(active_var_mask && !active_var_mask->data.ptr[vi]) )
            continue;

        if( data->is_classifier )
        {
            if( ci >= 0)
            {
                split = find_split_cat_class( node, vi); 
            }
            else
            {               
                split = find_split_ord_class( node, vi); 
            }
        }
        else
        {
            CV_ERROR( CV_StsBadArg, "ERTrees do not support regression problems" );
        }

        if( split )
        {
            if( !best_split || best_split->quality < split->quality )
            {
                CV_SWAP( best_split, split, t );
                best_ln = ((CvERTreeNode*)node)->ln;
                int* tsidx = ((CvERTreeNode*)node)->l_sample_idx;
                for (int li = 0; li < best_ln; li++)
                    best_l_sample_idx[li] = tsidx[li];
                best_rn = ((CvERTreeNode*)node)->rn;
                tsidx = ((CvERTreeNode*)node)->r_sample_idx;
                for (int ri = 0; ri < best_rn; ri++)
                    best_r_sample_idx[ri] = tsidx[ri];
            }
            if( split )
                cvSetRemoveByPtr( data->split_heap, split );
        }
    }
    cvFree( &((CvERTreeNode*)node)->l_sample_idx );
    cvFree( &((CvERTreeNode*)node)->r_sample_idx );

    ((CvERTreeNode*)node)->l_sample_idx = (int*)cvAlloc(best_ln*sizeof(((CvERTreeNode*)node)->l_sample_idx[0]));
    ((CvERTreeNode*)node)->ln = best_ln;
    ((CvERTreeNode*)node)->r_sample_idx = (int*)cvAlloc(best_rn*sizeof(((CvERTreeNode*)node)->r_sample_idx[0]));
    ((CvERTreeNode*)node)->rn = best_rn;
    tsidx = ((CvERTreeNode*)node)->l_sample_idx;
    for (int li = 0; li < best_ln; li++)
        tsidx[li] = best_l_sample_idx[li];
    tsidx = ((CvERTreeNode*)node)->r_sample_idx;
    for (int ri = 0; ri < best_rn; ri++)
        tsidx[ri] = best_r_sample_idx[ri];

    cvFree( &best_l_sample_idx );
    cvFree( &best_r_sample_idx );
    
    __END__;

    return best_split;
}


void CvForestERTree::try_split_node( CvDTreeNode* node )
{
    CvDTreeSplit* best_split = 0;
    int i, n = node->sample_count;
    bool can_split = true;

    calc_node_value( node );

    if( node->sample_count <= data->params.min_sample_count ||
        node->depth >= data->params.max_depth )
        can_split = false;

    if( can_split && data->is_classifier )
    {
        int* cls_count = data->counts->data.i;
        int nz = 0, m = ((CvERTreeTrainData*)data)->get_num_classes();
        for( i = 0; i < m; i++ )
            nz += cls_count[i] != 0;
        if( nz == 1 ) // there is only one class
            can_split = false;
    }
    else if( can_split )
    {
        if( sqrt(node->node_risk)/n < data->params.regression_accuracy )
            can_split = false;
    }

    if( can_split )
    {
        best_split = find_best_split(node);
        node->split = best_split;
    }

    if( !can_split || !best_split )
    {
        data->free_node_data(node);
        return;
    }

    split_node_data( node );

    try_split_node( (CvERTreeNode*)node->left );
    try_split_node( (CvERTreeNode*)node->right );
}


CvDTreeSplit* CvForestERTree::find_split_ord_class( CvDTreeNode* node, int vi )
{
    const float epsilon = FLT_EPSILON*2;
    int n = node->sample_count;
    int m = ((CvERTreeTrainData*)data)->get_num_classes();
    int* lc = (int*)cvStackAlloc(m*sizeof(lc[0]));
    int* rc = (int*)cvStackAlloc(m*sizeof(rc[0]));
    int i;
    const double* priors = data->have_priors ? data->priors_mult->data.db : 0;
    double best_val = 0;

    const float* ord_pred = ((CvERTreeTrainData*)data)->ord_pred->data.fl;
    int ci = data->get_var_type(vi);
    const int idx = ((CvERTreeTrainData*)data)->get_ord_var_idx(ci);
    const int* sidx = ((CvERTreeNode*)node)->sample_idx;
    const int* resp = ((CvERTreeTrainData*)data)->resp->data.i;
    int pstep = ((CvERTreeTrainData*)data)->ord_pred->step / CV_ELEM_SIZE(((CvERTreeTrainData*)data)->ord_pred->type);
    int rstep = ((CvERTreeTrainData*)data)->resp->step / CV_ELEM_SIZE(((CvERTreeTrainData*)data)->resp->type);
    bool is_find_split = false;

    const float split_delta = (1 + FLT_EPSILON) * FLT_EPSILON;
    double split_val = 0;

    if( !priors )
    {
        float pmin, pmax;
        pmin = ord_pred[sidx[0]*pstep + idx];
        pmax = pmin;
        for (int si = 1; si < n; si++)
        {
            float ptemp = ord_pred[sidx[si]*pstep + idx];
            if ( ptemp < pmin)
                pmin = ptemp;
            if ( ptemp > pmax)
                pmax = ptemp;
        }
        float fdiff = pmax-pmin;
        if (fdiff > epsilon)
        {
            is_find_split = true;
            CvRNG* rng = &data->rng;
            split_val = pmin + cvRandReal(rng) * fdiff ;
            if (split_val - pmin <= FLT_EPSILON)
                split_val = pmin + split_delta;
            if (pmax - split_val <= FLT_EPSILON)
                split_val = pmax - split_delta;       

            // init arrays of class instance counters on both sides of the split
            for( i = 0; i < m; i++ )
            {
                lc[i] = 0;
                rc[i] = 0;
            }

            if (!((CvERTreeNode*)node)->l_sample_idx)
            {
                ((CvERTreeNode*)node)->l_sample_idx = (int*)cvAlloc( sizeof(((CvERTreeNode*)node)->l_sample_idx[0])*n );
                ((CvERTreeNode*)node)->ln = 0;
            }
            if (!((CvERTreeNode*)node)->r_sample_idx)
            {
                ((CvERTreeNode*)node)->r_sample_idx = (int*)cvAlloc( sizeof(((CvERTreeNode*)node)->r_sample_idx[0])*n );
                ((CvERTreeNode*)node)->rn = 0;
            }

            int* l_sind = ((CvERTreeNode*)node)->l_sample_idx;
            int* r_sind = ((CvERTreeNode*)node)->r_sample_idx;

            // calculate Gini index
            double lbest_val = 0, rbest_val = 0;
            int L = 0, R = 0;
            for( int si = 0; si < n; si++ )
            {
                int r = resp[sidx[si]*rstep];
                if ((ord_pred[sidx[si]*pstep + idx]) < split_val)
                {
                    lc[r]++;
                    l_sind[L] = sidx[si];
                    L++;
                }
                else
                {
                    rc[r]++;
                    r_sind[R] = sidx[si];
                    R++;
                }
            }
            for (int i = 0; i < m; i++)
            {
                lbest_val += lc[i]*lc[i];
                rbest_val += rc[i]*rc[i];
            }
            lbest_val = lbest_val/L;
            rbest_val = rbest_val/R;
            best_val = lbest_val + rbest_val;

            ((CvERTreeNode*)node)->ln = L;
            ((CvERTreeNode*)node)->rn = R;
        }
    }        
    return is_find_split ? data->new_split_ord( vi, (float)split_val, -1, 0, (float)best_val ) : 0;
}

CvDTreeSplit* CvForestERTree::find_split_cat_class( CvDTreeNode* node, int vi )
{
    int ci = data->get_var_type(vi);
    int n = node->sample_count;
    int cm = ((CvERTreeTrainData*)data)->get_num_classes(); 
    int vm = ((CvERTreeTrainData*)data)->cat_count->data.i[ci];
    double best_val = 0;
    CvDTreeSplit *split = 0;

    if ( vm > 1 )
    {
        const int* cat_pred = ((CvERTreeTrainData*)data)->cat_pred->data.i;
        const int* resp = ((CvERTreeTrainData*)data)->resp->data.i;
        const int* sidx = ((CvERTreeNode*)node)->sample_idx;
        int pstep = ((CvERTreeTrainData*)data)->cat_pred->step / CV_ELEM_SIZE(((CvERTreeTrainData*)data)->cat_pred->type);
        int rstep = ((CvERTreeTrainData*)data)->resp->step / CV_ELEM_SIZE(((CvERTreeTrainData*)data)->resp->type);

        int* lc = (int*)cvStackAlloc(cm*sizeof(lc[0]));
        int* rc = (int*)cvStackAlloc(cm*sizeof(rc[0]));
        
        const double* priors = data->have_priors ? data->priors_mult->data.db : 0;       

        if( !priors )
        {
            int *valid_cidx = (int*)cvStackAlloc(vm*sizeof(valid_cidx[0]));
            for (int i = 0; i < vm; i++)
            {
                valid_cidx[i] = -1;
            }
            for (int si = 0; si < n; si++)
            {
                int c = cat_pred[sidx[si]*pstep + ci];
                valid_cidx[c]++;
            }

            int valid_ccount = 0;
            for (int i = 0; i < vm; i++)
                if (valid_cidx[i] >= 0)
                {
                    valid_cidx[i] = valid_ccount;
                    valid_ccount++;
                }
            if (valid_ccount > 1)
            {
                CvRNG* rng = forest->get_rng();
                int lcv_count = 1 + cvRandInt(rng) % (valid_ccount-1);

                CvMat* var_class_mask = cvCreateMat( 1, valid_ccount, CV_8UC1 );
                CvMat submask;
                memset(var_class_mask->data.ptr, 0, valid_ccount*CV_ELEM_SIZE(var_class_mask->type));
                cvGetCols( var_class_mask, &submask, 0, lcv_count );
                cvSet( &submask, cvScalar(1) );
                for (int i = 0; i < valid_ccount; i++)
                {
                    uchar temp;
                    int i1 = cvRandInt( rng ) % valid_ccount;
                    int i2 = cvRandInt( rng ) % valid_ccount;
                    CV_SWAP( var_class_mask->data.ptr[i1], var_class_mask->data.ptr[i2], temp );
                }

                // init arrays of class instance counters on both sides of the split
                for(int i = 0; i < cm; i++ )
                {
                    lc[i] = 0;
                    rc[i] = 0;
                }

                if (!((CvERTreeNode*)node)->l_sample_idx)
                {
                    ((CvERTreeNode*)node)->l_sample_idx = (int*)cvAlloc( sizeof(((CvERTreeNode*)node)->l_sample_idx[0])*n );
                    ((CvERTreeNode*)node)->ln = 0;
                }
                if (!((CvERTreeNode*)node)->r_sample_idx)
                {
                    ((CvERTreeNode*)node)->r_sample_idx = (int*)cvAlloc( sizeof(((CvERTreeNode*)node)->r_sample_idx[0])*n );
                    ((CvERTreeNode*)node)->rn = 0;
                }

                int* l_sind = ((CvERTreeNode*)node)->l_sample_idx;
                int* r_sind = ((CvERTreeNode*)node)->r_sample_idx;

                split = data->new_split_cat( vi, -1. );

                // calculate Gini index
                double lbest_val = 0, rbest_val = 0;
                int L = 0, R = 0;
                                
                for( int si = 0; si < n; si++ )
                {
                    int r = resp[sidx[si]*rstep];
                    int var_class_idx = cat_pred[sidx[si]*pstep + ci];
                    int mask_class_idx = valid_cidx[var_class_idx];
                    if (var_class_mask->data.ptr[mask_class_idx])
                    {
                        lc[r]++;
                        l_sind[L] = sidx[si];
                        L++;                 
                        split->subset[var_class_idx >> 5] |= 1 << (var_class_idx & 31);
                    }
                    else
                    {
                        rc[r]++;
                        r_sind[R] = sidx[si];
                        R++;
                    }
                }
                for (int i = 0; i < cm; i++)
                {
                    lbest_val += lc[i]*lc[i];
                    rbest_val += rc[i]*rc[i];
                }
                lbest_val = lbest_val/L;
                rbest_val = rbest_val/R;
                best_val = lbest_val + rbest_val;

                split->quality = (float)best_val;

                ((CvERTreeNode*)node)->ln = L;
                ((CvERTreeNode*)node)->rn = R;

                cvReleaseMat(&var_class_mask);
            }             
        }
    }        
  
    return split;
}

void CvForestERTree::calc_node_value( CvDTreeNode* node )
{
    CV_FUNCNAME("CvForestERTree::calc_node_value");
    __BEGIN__;

    int i, j, k, n = node->sample_count, cv_n = data->params.cv_folds;

    if( data->is_classifier )
    {
        int* cls_count = data->counts->data.i;

        int m = ((CvERTreeTrainData*)data)->get_num_classes();
        int* cv_cls_count = (int*)cvStackAlloc(m*cv_n*sizeof(cv_cls_count[0]));
        double max_val = -1, total_weight = 0;
        int max_k = -1;

        for( k = 0; k < m; k++ )
            cls_count[k] = 0;
        static int inc = 0;
        inc ++;
       
        if (cv_n)
        {
             CV_ERROR( CV_StsBadArg, "pruning do not supported ERTrees" );
        }
        else
        {
            const int* rdata = ((CvERTreeTrainData*)data)->resp->data.i;
            int step = ((CvERTreeTrainData*)data)->resp->step/CV_ELEM_SIZE(((CvERTreeTrainData*)data)->resp->type);
            int *sidx = ((CvERTreeNode*)node)->sample_idx;
            for( i = 0; i < n; i++ )
                cls_count[rdata[sidx[i]*step]]++;
        }

   
        if( data->have_priors )
        {
            CV_ERROR( CV_StsBadArg, "priors do not supported ERTrees" );
        }

        for( k = 0; k < m; k++ )
        {
            double val = cls_count[k];
            total_weight += val;
            if( max_val < val )
            {
                max_val = val;
                max_k = k;
            }
        }

        node->class_idx = max_k;
        const int* ldata = ((CvERTreeTrainData*)data)->class_lables[data->cat_var_count]->data.i;
        node->value = ldata[max_k]; 

        node->node_risk = total_weight - max_val;

        for( j = 0; j < cv_n; j++ )
        {
            double sum_k = 0, sum = 0, max_val_k = 0;
            max_val = -1; max_k = -1;

            for( k = 0; k < m; k++ )
            {
                double val_k = cv_cls_count[j*m + k];
                double val = cls_count[k] - val_k;
                sum_k += val_k;
                sum += val;
                if( max_val < val )
                {
                    max_val = val;
                    max_val_k = val_k;
                    max_k = k;
                }
            }

            node->cv_Tn[j] = INT_MAX;
            node->cv_node_risk[j] = sum - max_val;
            node->cv_node_error[j] = sum_k - max_val_k;
        }
    }
    else
    {
        CV_ERROR( CV_StsBadArg, "ERTrees do not support regression problems" );
    }
    __END__
}

void CvForestERTree::split_node_data( CvDTreeNode* node )
{
    int ln = ((CvERTreeNode*)node)->ln;
    int rn = ((CvERTreeNode*)node)->rn;
    node->left = ((CvERTreeTrainData*)data)->new_node( (CvERTreeNode*)node, ln, &((CvERTreeNode*)node)->l_sample_idx );
    node->right = ((CvERTreeTrainData*)data)->new_node( (CvERTreeNode*)node, rn, &((CvERTreeNode*)node)->r_sample_idx );
  
    data->free_node_data(node);
}

CvDTreeNode* CvForestERTree::predict( const CvMat* _sample,
                                     const CvMat* _missing, bool preprocessed_input ) const
{
    CvDTreeNode* result = 0;

    CV_FUNCNAME( "CvForestERTree::predict" );

    __BEGIN__;

    int i, step, mstep = 0;
    const float* sample;
    const uchar* m = 0;
    CvDTreeNode* node = root;
    const int* vtype;
    const int* vidx;

    if( !node )
        CV_ERROR( CV_StsError, "The tree has not been trained yet" );

    if( !CV_IS_MAT(_sample) || CV_MAT_TYPE(_sample->type) != CV_32FC1 ||
        (_sample->cols != 1 && _sample->rows != 1) ||
        (_sample->cols + _sample->rows - 1 != data->var_all && !preprocessed_input) ||
        (_sample->cols + _sample->rows - 1 != data->var_count && preprocessed_input) )
        CV_ERROR( CV_StsBadArg,
        "the input sample must be 1d floating-point vector with the same "
        "number of elements as the total number of variables used for training" );

    sample = _sample->data.fl;
    step = CV_IS_MAT_CONT(_sample->type) ? 1 : _sample->step/sizeof(sample[0]);

    if( _missing )
    {
        if( !CV_IS_MAT(_missing) || !CV_IS_MASK_ARR(_missing) ||
            !CV_ARE_SIZES_EQ(_missing, _sample) )
            CV_ERROR( CV_StsBadArg,
            "the missing data mask must be 8-bit vector of the same size as input sample" );
        m = _missing->data.ptr;
        mstep = CV_IS_MAT_CONT(_missing->type) ? 1 : _missing->step/sizeof(m[0]);
    }

    vtype = data->var_type->data.i;
    vidx = data->var_idx && !preprocessed_input ? data->var_idx->data.i : 0;

    while( node->Tn > pruned_tree_idx && node->left )
    {
        CvDTreeSplit* split = node->split;
        int dir = 0;
        for( ; !dir && split != 0; split = split->next )
        {
            int vi = split->var_idx;
            int ci = vtype[vi];
            i = vidx ? vidx[vi] : vi;
            float val = sample[i*step];
            if( m && m[i*mstep] )
                continue;
            if( ci < 0 ) // ordered
                dir = val <= split->ord.c ? -1 : 1;
            else // categorical
            {
                int c;
                int c_count = data->cat_count->data.i[ci];
                if( preprocessed_input )
                    c = cvRound(val);
                else
                {
                    int ival = cvRound(val);
                    int i = 0;
                    int* labls = ((CvERTreeTrainData*)data)->class_lables[ci]->data.i;
                    if( ival != val )
                        CV_ERROR( CV_StsBadArg,
                            "one of input categorical variable is not an integer" );
                    for (i = 0; i < c_count; i++)
                        if (ival == labls[i]) break;
                    c = i;
                }
                if (c == c_count)
                {
                    CvRNG* rng = &data->rng;
                    dir = 2*(cvRandInt(rng)%2)-1;
                }
                else
                    dir = CV_DTREE_CAT_DIR(c, split->subset);
            }

            if( split->inversed )
                dir = -dir;
        }

        if( !dir )
        {
            double diff = node->right->sample_count - node->left->sample_count;
            dir = diff < 0 ? -1 : 1;
        }
        node = dir < 0 ? node->left : node->right;
    }

    result = node;

    __END__;

    return result;
}


bool CvERTrees::train( const CvMat* _train_data, int _tflag,
                      const CvMat* _responses, const CvMat* _var_idx,
                      const CvMat* _sample_idx, const CvMat* _var_type,
                      const CvMat* _missing_mask, CvRTParams params )
{
    bool result = false;

    CvDTreeParams tree_params( params.max_depth, params.min_sample_count,
        params.regression_accuracy, params.use_surrogates, params.max_categories,
        params.cv_folds, params.use_1se_rule, false, params.priors );

    CV_FUNCNAME("CvERTrees::train");
    __BEGIN__;

    int var_count = 0;

    if (_var_idx || _sample_idx || _missing_mask)
        CV_ERROR( CV_StsBadArg, "ERTrees do not support _var_idx, _sample_idx, _missing_mask" );

    clear();

    data = new CvERTreeTrainData();

    CV_CALL(((CvERTreeTrainData*)data)->set_data( _train_data, _tflag, _responses, _var_idx,
        _sample_idx, _var_type, _missing_mask, tree_params));

    var_count = data->var_count;
    if( params.nactive_vars > var_count )
        params.nactive_vars = var_count;
    else if( params.nactive_vars == 0 )
        params.nactive_vars = (int)sqrt((double)var_count);
    else if( params.nactive_vars < 0 )
        CV_ERROR( CV_StsBadArg, "<nactive_vars> must be non-negative" );
   
    // Create mask of active variables at the tree nodes
    CV_CALL(active_var_mask = cvCreateMat( 1, var_count, CV_8UC1 ));
    if( params.calc_var_importance )
    {
        CV_CALL(var_importance  = cvCreateMat( 1, var_count, CV_32FC1 ));
        cvZero(var_importance);
    }
    { // initialize active variables mask
        CvMat submask1, submask2;
        cvGetCols( active_var_mask, &submask1, 0, params.nactive_vars );
        cvSet( &submask1, cvScalar(1) );
        if (params.nactive_vars < var_count)
        {
            cvGetCols( active_var_mask, &submask2, params.nactive_vars, var_count );
            cvZero( &submask2 );
        }
    }

    if( params.calc_var_importance )
    {
        CV_CALL(var_importance  = cvCreateMat( 1, var_count, CV_32FC1 ));
        cvZero(var_importance);
    }


    CV_CALL(result = grow_forest( params.term_crit ));

    result = true;

    __END__;
    return result;
    
}


bool CvERTrees::grow_forest( const CvTermCriteria term_crit )
{
    bool result = false;

    CV_FUNCNAME("CvERTrees::grow_forest");
    __BEGIN__;

    const int max_ntrees = term_crit.max_iter;
    const double max_oob_err = term_crit.epsilon;

    nsamples = data->sample_count;
    nclasses = ((CvERTreeTrainData*)data)->get_num_classes();

    trees = (CvForestTree**)cvAlloc( sizeof(trees[0])*max_ntrees );

    memset( trees, 0, sizeof(trees[0])*max_ntrees );

//#define ET_OOB
#ifdef ET_OOB
    CvMat* oob_sample_votes	   = 0;
    CvMat* oob_responses       = 0;

    float* oob_samples_perm_ptr= 0;

    float* samples_ptr     = 0;
    uchar* missing_ptr     = 0;
    float* true_resp_ptr   = 0;

    const int dims = data->var_count;
    float maximal_response = 0;   

    CvMat oob_predictions_sum = cvMat( 1, 1, CV_32FC1 );
    CvMat oob_num_of_predictions = cvMat( 1, 1, CV_32FC1 );

    if( data->is_classifier )
    {
        CV_CALL(oob_sample_votes = cvCreateMat( nsamples, nclasses, CV_32SC1 ));
        cvZero(oob_sample_votes);
    }
    else
    {
        CV_CALL(oob_responses = cvCreateMat( 2, nsamples, CV_32FC1 ));
        cvZero(oob_responses);
        cvGetRow( oob_responses, &oob_predictions_sum, 0 );
        cvGetRow( oob_responses, &oob_num_of_predictions, 1 );
    }
    CV_CALL(oob_samples_perm_ptr     = (float*)cvAlloc( sizeof(float)*nsamples*dims ));
    CV_CALL(samples_ptr              = (float*)cvAlloc( sizeof(float)*nsamples*dims ));
    CV_CALL(missing_ptr              = (uchar*)cvAlloc( sizeof(uchar)*nsamples*dims ));
    CV_CALL(true_resp_ptr            = (float*)cvAlloc( sizeof(float)*nsamples ));

    CV_CALL(data->get_vectors( 0, samples_ptr, 0, true_resp_ptr ));
    {
        double minval, maxval;
        CvMat responses = cvMat(1, nsamples, CV_32FC1, true_resp_ptr);
        cvMinMaxLoc( &responses, &minval, &maxval );
        maximal_response = (float)MAX( MAX( fabs(minval), fabs(maxval) ), 0 );
    }
#endif
    ntrees = 0;
    while( ntrees < max_ntrees )
    {
        CvForestERTree* tree = 0;

        trees[ntrees] = new CvForestERTree();
        tree = (CvForestERTree*)trees[ntrees];
        CV_CALL(tree->train( data, 0, this ));

#ifdef ET_OOB
        int i, oob_samples_count = 0;
        double ncorrect_responses = 0;
        CvMat sample, missing;
        sample   = cvMat( 1, dims, CV_32FC1, samples_ptr );
        missing  = cvMat( 1, dims, CV_8UC1,  missing_ptr );

        oob_error = 0;
        for( i = 0; i < nsamples; i++,
            sample.data.fl += dims, missing.data.ptr += dims )
        {
            CvDTreeNode* predicted_node = 0;
            // predict oob samples
            if( !predicted_node )
                CV_CALL(predicted_node = tree->predict(&sample, &missing, true));

            if( !data->is_classifier ) //regression
            {
                CV_ERROR( CV_StsBadArg, "ERTrees do not support regression problems" );
            }
            else //classification
            {
                double prdct_resp;
                CvPoint max_loc;
                CvMat votes;

                cvGetRow(oob_sample_votes, &votes, i);
                votes.data.i[predicted_node->class_idx]++;

                // compute oob error
                cvMinMaxLoc( &votes, 0, 0, 0, &max_loc );

                prdct_resp = ((CvERTreeTrainData*)data)->class_lables[data->cat_var_count]->data.i[max_loc.x];
                oob_error += (fabs(prdct_resp - true_resp_ptr[i]) < FLT_EPSILON) ? 0 : 1;

                ncorrect_responses += cvRound(predicted_node->value - true_resp_ptr[i]) == 0;
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
                double ncorrect_responses_permuted = 0;
                // randomly permute values of the m-th variable in the oob samples
                float* mth_var_ptr = oob_samples_perm_ptr + m;

                for( i = 0; i < nsamples; i++ )
                {
                    int i1, i2;
                    float temp;

                    //if( sample_idx_mask_for_tree->data.ptr[i] ) //the sample is not OOB
                    //    continue;
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
                    double predct_resp, true_resp;

                    //if( sample_idx_mask_for_tree->data.ptr[i] ) //the sample is not OOB
                    //    continue;

                    predct_resp = tree->predict(&sample, &missing, true)->value;
                    true_resp   = true_resp_ptr[i];
                    if( data->is_classifier )
                        ncorrect_responses_permuted += cvRound(true_resp - predct_resp) == 0;
                    else
                    {
                        true_resp = (true_resp - predct_resp)/maximal_response;
                        ncorrect_responses_permuted += exp( -true_resp*true_resp );
                    }
                }
                var_importance->data.fl[m] += (float)(ncorrect_responses
                    - ncorrect_responses_permuted);
            }
        }
#endif //ET_OOB
        ntrees++;

        if( term_crit.type != CV_TERMCRIT_ITER && oob_error < max_oob_err )
            break;
    }
    if( var_importance )
        CV_CALL(cvConvertScale( var_importance, var_importance, 1./ntrees/nsamples ));

    result = true;

    __END__;

#ifdef ET_OOB
    cvReleaseMat( &oob_sample_votes );
    cvReleaseMat( &oob_responses );

    cvFree( &oob_samples_perm_ptr );
    cvFree( &samples_ptr );
    cvFree( &missing_ptr );
    cvFree( &true_resp_ptr );
#endif
    return result;
}


void CvERTrees::clear()
{
    int k;
    for( k = 0; k < ntrees; k++ )
        delete (CvForestERTree*)trees[k];
    cvFree( &trees );

    delete (CvERTreeTrainData*)data;
    data = 0;

    cvReleaseMat( &active_var_mask );
    cvReleaseMat( &var_importance );
    ntrees = 0;  
}

void CvERTrees::read( CvFileStorage* fs, CvFileNode* node )
{
    CV_FUNCNAME("CvERTrees::read");
    __BEGIN__;
    fs = 0; node = 0;
    CV_ERROR( CV_StsBadArg, "ERTrees do not support this method" );
    __END__;
};

void CvERTrees::write( CvFileStorage* fs, const char* name )
{
    CV_FUNCNAME("CvERTrees::write");
    __BEGIN__;
    fs = 0; name = 0;
    CV_ERROR( CV_StsBadArg, "ERTrees do not support this method" );
    __END__;
};
// End of file.
