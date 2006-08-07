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

static const float ord_nan = FLT_MAX*0.5f;
static const int min_block_size = 1 << 16;
static const int block_size_delta = 1 << 10;

CvDTreeTrainData::CvDTreeTrainData()
{
    var_idx = var_type = cat_count = cat_ofs = cat_map =
        priors = counts = buf = direction = split_buf = 0;
    tree_storage = temp_storage = 0;

    clear();
}


CvDTreeTrainData::CvDTreeTrainData( const CvMat* _train_data, int _tflag,
                      const CvMat* _responses, const CvMat* _var_idx,
                      const CvMat* _sample_idx, const CvMat* _var_type,
                      const CvMat* _missing_mask,
                      CvDTreeParams _params, bool _shared, bool _add_weights )
{
    var_idx = var_type = cat_count = cat_ofs = cat_map =
        priors = counts = buf = direction = split_buf = 0;
    tree_storage = temp_storage = 0;
    
    set_data( _train_data, _tflag, _responses, _var_idx, _sample_idx,
              _var_type, _missing_mask, _params, _shared, _add_weights );
}


CvDTreeTrainData::~CvDTreeTrainData()
{
    clear();
}


bool CvDTreeTrainData::set_params( const CvDTreeParams& _params )
{
    bool ok = false;
    
    CV_FUNCNAME( "CvDTreeTrainData::set_params" );

    __BEGIN__;

    // set parameters
    params = _params;

    if( params.max_categories < 2 )
        CV_ERROR( CV_StsOutOfRange, "params.max_categories should be >= 2" );
    params.max_categories = MIN( params.max_categories, 15 );

    if( params.max_depth < 0 )
        CV_ERROR( CV_StsOutOfRange, "params.max_depth should be >= 0" );
    params.max_depth = MIN( params.max_depth, 25 );

    params.min_sample_count = MAX(params.min_sample_count,1);

    if( params.cv_folds < 0 )
        CV_ERROR( CV_StsOutOfRange,
        "params.cv_folds should be =0 (the tree is not pruned) "
        "or n>0 (tree is pruned using n-fold cross-validation)" );

    if( params.cv_folds == 1 )
        params.cv_folds = 0;

    if( params.regression_accuracy < 0 )
        CV_ERROR( CV_StsOutOfRange, "params.regression_accuracy should be >= 0" );

    ok = true;

    __END__;

    return ok;
}


#define CV_CMP_NUM_PTR(a,b) (*(a) < *(b))
static CV_IMPLEMENT_QSORT_EX( icvSortIntPtr, int*, CV_CMP_NUM_PTR, int )
static CV_IMPLEMENT_QSORT_EX( icvSortDblPtr, double*, CV_CMP_NUM_PTR, int )

#define CV_CMP_PAIRS(a,b) ((a).val < (b).val)
static CV_IMPLEMENT_QSORT_EX( icvSortPairs, CvPair32s32f, CV_CMP_PAIRS, int )

void CvDTreeTrainData::set_data( const CvMat* _train_data, int _tflag,
    const CvMat* _responses, const CvMat* _var_idx, const CvMat* _sample_idx,
    const CvMat* _var_type, const CvMat* _missing_mask, CvDTreeParams _params,
    bool _shared, bool _add_weights )
{
    CvMat* sample_idx = 0;
    CvMat* var_type0 = 0;
    CvMat* tmp_map = 0;
    int** int_ptr = 0;

    CV_FUNCNAME( "CvDTreeTrainData::set_data" );

    __BEGIN__;

    int sample_all = 0, r_type = 0, cv_n;
    int total_c_count = 0;
    int tree_block_size, temp_block_size, max_split_size, nv_size, cv_size = 0;
    int ds_step, dv_step, ms_step = 0, mv_step = 0; // {data|mask}{sample|var}_step
    int vi, i;
    char err[100];
    const int *sidx = 0, *vidx = 0;

    clear();

    var_all = 0;
    rng = cvRNG(-1);

    CV_CALL( set_params( _params ));

    // check parameter types and sizes
    CV_CALL( cvCheckTrainData( _train_data, _tflag, _missing_mask, &var_all, &sample_all ));
    if( _tflag == CV_ROW_SAMPLE )
    {
        ds_step = _train_data->step/CV_ELEM_SIZE(_train_data->type);
        dv_step = 1;
        if( _missing_mask )
            ms_step = _missing_mask->step, mv_step = 1;
    }
    else
    {
        dv_step = _train_data->step/CV_ELEM_SIZE(_train_data->type);
        ds_step = 1;
        if( _missing_mask )
            mv_step = _missing_mask->step, ms_step = 1;
    }

    sample_count = sample_all;
    var_count = var_all;

    if( _sample_idx )
    {
        CV_CALL( sample_idx = cvPreprocessIndexArray( _sample_idx, sample_all ));
        sidx = sample_idx->data.i;
        sample_count = sample_idx->rows + sample_idx->cols - 1;
    }

    if( _var_idx )
    {
        CV_CALL( var_idx = cvPreprocessIndexArray( _var_idx, var_all ));
        vidx = var_idx->data.i;
        var_count = var_idx->rows + var_idx->cols - 1;
    }

    if( !CV_IS_MAT(_responses) ||
        (CV_MAT_TYPE(_responses->type) != CV_32SC1 &&
         CV_MAT_TYPE(_responses->type) != CV_32FC1) ||
        _responses->rows != 1 && _responses->cols != 1 ||
        _responses->rows + _responses->cols - 1 != sample_all )
        CV_ERROR( CV_StsBadArg, "The array of _responses must be an integer or "
                  "floating-point vector containing as many elements as "
                  "the total number of samples in the training data matrix" );

    CV_CALL( var_type0 = cvPreprocessVarType( _var_type, var_idx, var_all, &r_type ));
    CV_CALL( var_type = cvCreateMat( 1, var_count+2, CV_32SC1 ));

    cat_var_count = 0;
    ord_var_count = -1;

    is_classifier = r_type == CV_VAR_CATEGORICAL;

    // step 0. calc the number of categorical vars
    for( vi = 0; vi < var_count; vi++ )
    {
        var_type->data.i[vi] = var_type0->data.ptr[vi] == CV_VAR_CATEGORICAL ?
            cat_var_count++ : ord_var_count--;
    }

    ord_var_count = ~ord_var_count;
    cv_n = params.cv_folds;
    // set the two last elements of var_type array to be able
    // to locate responses and cross-validation labels using
    // the corresponding get_* functions.
    var_type->data.i[var_count] = cat_var_count;
    var_type->data.i[var_count+1] = cat_var_count+1;

    // in case of single ordered predictor we need dummy cv_labels
    // for safe split_node_data() operation
    have_cv_labels = cv_n > 0 || ord_var_count == 1 && cat_var_count == 0;
    have_weights = _add_weights;

    buf_size = (ord_var_count*2 + cat_var_count + 1 +
        (have_cv_labels ? 1 : 0) + (have_weights ? 1 : 0))*sample_count + 2;
    shared = _shared;
    buf_count = shared ? 3 : 2;
    CV_CALL( buf = cvCreateMat( buf_count, buf_size, CV_32SC1 ));
    CV_CALL( cat_count = cvCreateMat( 1, cat_var_count+1, CV_32SC1 ));
    CV_CALL( cat_ofs = cvCreateMat( 1, cat_count->cols+1, CV_32SC1 ));
    CV_CALL( cat_map = cvCreateMat( 1, cat_count->cols*10 + 128, CV_32SC1 ));

    // now calculate the maximum size of split,
    // create memory storage that will keep nodes and splits of the decision tree
    // allocate root node and the buffer for the whole training data
    max_split_size = cvAlign(sizeof(CvDTreeSplit) +
        (MAX(0,sample_count - 33)/32)*sizeof(int),sizeof(void*));
    tree_block_size = MAX((int)sizeof(CvDTreeNode)*8, max_split_size);
    tree_block_size = MAX(tree_block_size + block_size_delta, min_block_size);
    CV_CALL( tree_storage = cvCreateMemStorage( tree_block_size ));
    CV_CALL( node_heap = cvCreateSet( 0, sizeof(*node_heap), sizeof(CvDTreeNode), tree_storage ));

    temp_block_size = nv_size = var_count*sizeof(int);
    if( cv_n )
    {
        if( sample_count < cv_n*MAX(params.min_sample_count,10) )
            CV_ERROR( CV_StsOutOfRange,
                "The many folds in cross-validation for such a small dataset" );

        cv_size = cvAlign( cv_n*(sizeof(int) + sizeof(double)*2), sizeof(double) );
        temp_block_size = MAX(temp_block_size, cv_size);
    }

    temp_block_size = MAX( temp_block_size + block_size_delta, min_block_size );
    CV_CALL( temp_storage = cvCreateMemStorage( temp_block_size ));
    CV_CALL( nv_heap = cvCreateSet( 0, sizeof(*nv_heap), nv_size, temp_storage ));
    if( cv_size )
        CV_CALL( cv_heap = cvCreateSet( 0, sizeof(*cv_heap), cv_size, temp_storage ));

    CV_CALL( data_root = new_node( 0, sample_count, 0, 0 ));
    CV_CALL( int_ptr = (int**)cvAlloc( sample_count*sizeof(int_ptr[0]) ));

    max_c_count = 1;

    // transform the training data to convenient representation
    for( vi = 0; vi <= var_count; vi++ )
    {
        int ci;
        const uchar* mask = 0;
        int m_step = 0, step;
        const int* idata = 0;
        const float* fdata = 0;
        int num_valid = 0;

        if( vi < var_count ) // analyze i-th input variable
        {
            int vi0 = vidx ? vidx[vi] : vi;
            ci = get_var_type(vi);
            step = ds_step; m_step = ms_step;
            if( CV_MAT_TYPE(_train_data->type) == CV_32SC1 )
                idata = _train_data->data.i + vi0*dv_step;
            else
                fdata = _train_data->data.fl + vi0*dv_step;
            if( _missing_mask )
                mask = _missing_mask->data.ptr + vi0*mv_step;
        }
        else // analyze _responses
        {
            ci = cat_var_count;
            step = CV_IS_MAT_CONT(_responses->type) ?
                1 : _responses->step / CV_ELEM_SIZE(_responses->type);
            if( CV_MAT_TYPE(_responses->type) == CV_32SC1 )
                idata = _responses->data.i;
            else
                fdata = _responses->data.fl;
        }

        if( vi < var_count && ci >= 0 ||
            vi == var_count && is_classifier ) // process categorical variable or response
        {
            int c_count, prev_label;
            int* c_map, *dst = get_cat_var_data( data_root, vi );

            // copy data
            for( i = 0; i < sample_count; i++ )
            {
                int val = INT_MAX, si = sidx ? sidx[i] : i;
                if( !mask || !mask[si*m_step] )
                {
                    if( idata )
                        val = idata[si*step];
                    else
                    {
                        float t = fdata[si*step];
                        val = cvRound(t);
                        if( val != t )
                        {
                            sprintf( err, "%d-th value of %d-th (categorical) "
                                "variable is not an integer", i, vi );
                            CV_ERROR( CV_StsBadArg, err );
                        }
                    }

                    if( val == INT_MAX )
                    {
                        sprintf( err, "%d-th value of %d-th (categorical) "
                            "variable is too large", i, vi );
                        CV_ERROR( CV_StsBadArg, err );
                    }
                    num_valid++;
                }
                dst[i] = val;
                int_ptr[i] = dst + i;
            }

            // sort all the values, including the missing measurements
            // that should all move to the end
            icvSortIntPtr( int_ptr, sample_count, 0 );
            //qsort( int_ptr, sample_count, sizeof(int_ptr[0]), icvCmpIntPtr );

            c_count = num_valid > 0;

            // count the categories
            for( i = 1; i < num_valid; i++ )
                c_count += *int_ptr[i] != *int_ptr[i-1];

            if( vi > 0 )
                max_c_count = MAX( max_c_count, c_count );
            cat_count->data.i[ci] = c_count;
            cat_ofs->data.i[ci] = total_c_count;

            // resize cat_map, if need
            if( cat_map->cols < total_c_count + c_count )
            {
                tmp_map = cat_map;
                CV_CALL( cat_map = cvCreateMat( 1,
                    MAX(cat_map->cols*3/2,total_c_count+c_count), CV_32SC1 ));
                for( i = 0; i < total_c_count; i++ )
                    cat_map->data.i[i] = tmp_map->data.i[i];
                cvReleaseMat( &tmp_map );
            }

            c_map = cat_map->data.i + total_c_count;
            total_c_count += c_count;

            // compact the class indices and build the map
            prev_label = ~*int_ptr[0];
            c_count = -1;

            for( i = 0; i < num_valid; i++ )
            {
                int cur_label = *int_ptr[i];
                if( cur_label != prev_label )
                    c_map[++c_count] = prev_label = cur_label;
                *int_ptr[i] = c_count;
            }

            // replace labels for missing values with -1
            for( ; i < sample_count; i++ )
                *int_ptr[i] = -1;
        }
        else if( ci < 0 ) // process ordered variable
        {
            CvPair32s32f* dst = get_ord_var_data( data_root, vi );

            for( i = 0; i < sample_count; i++ )
            {
                float val = ord_nan;
                int si = sidx ? sidx[i] : i;
                if( !mask || !mask[si*m_step] )
                {
                    if( idata )
                        val = (float)idata[si*step];
                    else
                        val = fdata[si*step];

                    if( fabs(val) >= ord_nan )
                    {
                        sprintf( err, "%d-th value of %d-th (ordered) "
                            "variable (=%g) is too large", i, vi, val );
                        CV_ERROR( CV_StsBadArg, err );
                    }
                    num_valid++;
                }
                dst[i].i = i;
                dst[i].val = val;
            }

            icvSortPairs( dst, sample_count, 0 );
        }
        else // special case: process ordered response,
             // it will be stored similarly to categorical vars (i.e. no pairs)
        {
            float* dst = get_ord_responses( data_root );

            for( i = 0; i < sample_count; i++ )
            {
                float val = ord_nan;
                int si = sidx ? sidx[i] : i;
                if( idata )
                    val = (float)idata[si*step];
                else
                    val = fdata[si*step];

                if( fabs(val) >= ord_nan )
                {
                    sprintf( err, "%d-th value of %d-th (ordered) "
                        "variable (=%g) is out of range", i, vi, val );
                    CV_ERROR( CV_StsBadArg, err );
                }
                dst[i] = val;
            }

            cat_count->data.i[cat_var_count] = 0;
            cat_ofs->data.i[cat_var_count] = total_c_count;
            num_valid = sample_count;
        }

        if( vi < var_count )
            data_root->set_num_valid(vi, num_valid);
    }

    if( cv_n )
    {
        int* dst = get_cv_labels(data_root);
        CvRNG* r = &rng;

        for( i = vi = 0; i < sample_count; i++ )
        {
            dst[i] = vi++;
            vi &= vi < cv_n ? -1 : 0;
        }

        for( i = 0; i < sample_count; i++ )
        {
            int a = cvRandInt(r) % sample_count;
            int b = cvRandInt(r) % sample_count;
            CV_SWAP( dst[a], dst[b], vi );
        }
    }

    cat_map->cols = MAX( total_c_count, 1 );

    max_split_size = cvAlign(sizeof(CvDTreeSplit) +
        (MAX(0,max_c_count - 33)/32)*sizeof(int),sizeof(void*));
    CV_CALL( split_heap = cvCreateSet( 0, sizeof(*split_heap), max_split_size, tree_storage ));

    have_priors = is_classifier && params.priors;
    if( is_classifier )
    {
        int m = get_num_classes(), rows = 4;
        double sum = 0;
        CV_CALL( priors = cvCreateMat( 1, m, CV_64F ));
        for( i = 0; i < m; i++ )
        {
            double val = have_priors ? params.priors[i] : 1.;
            if( val <= 0 )
                CV_ERROR( CV_StsOutOfRange, "Every class weight should be positive" );
            priors->data.db[i] = val;
            sum += val;
        }
        // normalize weights
        if( have_priors )
            cvScale( priors, priors, 1./sum );

        if( cat_var_count > 0 || params.cv_folds > 0 )
        {
            // need storage for cjk (see find_split_cat_gini) and risks/errors
            rows += MAX( max_c_count, params.cv_folds ) + 1;
            // add buffer for k-means clustering
            if( m > 2 && max_c_count > params.max_categories )
                rows += params.max_categories + (max_c_count+m-1)/m;
        }

        CV_CALL( counts = cvCreateMat( rows, m, CV_32SC2 ));
    }

    CV_CALL( direction = cvCreateMat( 1, sample_count, CV_8UC1 ));
    CV_CALL( split_buf = cvCreateMat( 1, sample_count, CV_32SC1 ));

    __END__;

    cvFree( &int_ptr );
    cvReleaseMat( &sample_idx );
    cvReleaseMat( &var_type0 );
    cvReleaseMat( &tmp_map );
}


CvDTreeNode* CvDTreeTrainData::subsample_data( const CvMat* _subsample_idx )
{
    CvDTreeNode* root = 0;
    CvMat* isubsample_idx = 0;
    CvMat* subsample_co = 0;
    
    CV_FUNCNAME( "CvDTreeTrainData::subsample_data" );

    __BEGIN__;

    if( !data_root )
        CV_ERROR( CV_StsError, "No training data has been set" );
    
    if( _subsample_idx )
        CV_CALL( isubsample_idx = cvPreprocessIndexArray( _subsample_idx, sample_count ));

    if( !isubsample_idx )
    {
        // make a copy of the root node
        CvDTreeNode temp;
        int i;
        root = new_node( 0, 1, 0, 0 );
        temp = *root;
        *root = *data_root;
        root->num_valid = temp.num_valid;
        if( root->num_valid )
        {
            for( i = 0; i < var_count; i++ )
                root->num_valid[i] = data_root->num_valid[i];
        }
        root->cv_Tn = temp.cv_Tn;
        root->cv_node_risk = temp.cv_node_risk;
        root->cv_node_error = temp.cv_node_error;
    }
    else
    {
        int* sidx = isubsample_idx->data.i;
        // co - array of count/offset pairs (to handle duplicated values in _subsample_idx)
        int* co, cur_ofs = 0;
        int vi, i, total = data_root->sample_count;
        int count = isubsample_idx->rows + isubsample_idx->cols - 1;
        root = new_node( 0, count, 1, 0 );

        CV_CALL( subsample_co = cvCreateMat( 1, total*2, CV_32SC1 ));
        cvZero( subsample_co );
        co = subsample_co->data.i;
        for( i = 0; i < count; i++ )
            co[sidx[i]*2]++;
        for( i = 0; i < total; i++ )
        {
            if( co[i*2] )
            {
                co[i*2+1] = cur_ofs;
                cur_ofs += co[i*2];
            }
            else
                co[i*2+1] = -1;
        }

        for( vi = 0; vi <= var_count + (have_cv_labels ? 1 : 0); vi++ )
        {
            int ci = get_var_type(vi);

            if( ci >= 0 || vi >= var_count )
            {
                const int* src = get_cat_var_data( data_root, vi );
                int* dst = get_cat_var_data( root, vi );
                int num_valid = 0;

                for( i = 0; i < count; i++ )
                {
                    int val = src[sidx[i]];
                    dst[i] = val;
                    num_valid += val >= 0;
                }

                if( vi < var_count )
                    root->set_num_valid(vi, num_valid);
            }
            else
            {
                const CvPair32s32f* src = get_ord_var_data( data_root, vi );
                CvPair32s32f* dst = get_ord_var_data( root, vi );
                int j = 0, idx, count_i;
                int num_valid = data_root->get_num_valid(vi);

                for( i = 0; i < num_valid; i++ )
                {
                    idx = src[i].i;
                    count_i = co[idx*2];
                    if( count_i )
                    {
                        float val = src[i].val;
                        for( cur_ofs = co[idx*2+1]; count_i > 0; count_i--, j++, cur_ofs++ )
                        {
                            dst[j].val = val;
                            dst[j].i = cur_ofs;
                        }
                    }
                }

                root->set_num_valid(vi, j);

                for( ; i < total; i++ )
                {
                    idx = src[i].i;
                    count_i = co[idx*2];
                    if( count_i )
                    {
                        float val = src[i].val;
                        for( cur_ofs = co[idx*2+1]; count_i > 0; count_i--, j++, cur_ofs++ )
                        {
                            dst[j].val = val;
                            dst[j].i = cur_ofs;
                        }
                    }
                }
            }
        }
    }

    __END__;

    cvReleaseMat( &isubsample_idx );
    cvReleaseMat( &subsample_co );

    return root;
}


void CvDTreeTrainData::get_vectors( const CvMat* _subsample_idx,
                                    float* values, uchar* missing,
                                    float* responses, bool get_class_idx )
{
    CvMat* subsample_idx = 0;
    CvMat* subsample_co = 0;
    
    CV_FUNCNAME( "CvDTreeTrainData::get_vectors" );

    __BEGIN__;

    int i, vi, total = sample_count, count = total, cur_ofs = 0;
    int* sidx = 0;
    int* co = 0;

    if( _subsample_idx )
    {
        CV_CALL( subsample_idx = cvPreprocessIndexArray( _subsample_idx, sample_count ));
        sidx = subsample_idx->data.i;
        CV_CALL( subsample_co = cvCreateMat( 1, sample_count*2, CV_32SC1 ));
        co = subsample_co->data.i;
        cvZero( subsample_co );
        count = subsample_idx->cols + subsample_idx->rows - 1;
        for( i = 0; i < count; i++ )
            co[sidx[i]*2]++;
        for( i = 0; i < total; i++ )
        {
            int count_i = co[i*2];
            if( count_i )
            {
                co[i*2+1] = cur_ofs*var_count;
                cur_ofs += count_i;
            }
        }
    }

    memset( missing, 1, count*var_count );

    for( vi = 0; vi < var_count; vi++ )
    {
        int ci = get_var_type(vi);
        if( ci >= 0 ) // categorical
        {
            float* dst = values + vi;
            uchar* m = missing + vi;
            const int* src = get_cat_var_data(data_root, vi);

            for( i = 0; i < count; i++, dst += var_count, m += var_count )
            {
                int idx = sidx ? sidx[i] : i;
                int val = src[idx];
                *dst = (float)val;
                *m = val < 0;
            }
        }
        else // ordered
        {
            float* dst = values + vi;
            uchar* m = missing + vi;
            const CvPair32s32f* src = get_ord_var_data(data_root, vi);
            int count1 = data_root->get_num_valid(vi);

            for( i = 0; i < count1; i++ )
            {
                int idx = src[i].i;
                int count_i = 1;
                if( co )
                {
                    count_i = co[idx*2];
                    cur_ofs = co[idx*2+1];
                }
                else
                    cur_ofs = idx*var_count;
                if( count_i )
                {
                    float val = src[i].val;
                    for( ; count_i > 0; count_i--, cur_ofs += var_count )
                    {
                        dst[cur_ofs] = val;
                        m[cur_ofs] = 0;
                    }
                }
            }
        }
    }

    // copy responses
    if( is_classifier )
    {
        const int* src = get_class_labels(data_root);
        for( i = 0; i < count; i++ )
        {
            int idx = sidx ? sidx[i] : i;
            int val = get_class_idx ? src[idx] :
                cat_map->data.i[cat_ofs->data.i[cat_var_count]+src[idx]];
            responses[i] = (float)val;
        }
    }
    else
    {
        const float* src = get_ord_responses(data_root);
        for( i = 0; i < count; i++ )
        {
            int idx = sidx ? sidx[i] : i;
            responses[i] = src[idx];
        }
    }

    __END__;

    cvReleaseMat( &subsample_idx );
    cvReleaseMat( &subsample_co );
}


CvDTreeNode* CvDTreeTrainData::new_node( CvDTreeNode* parent, int count,
                                         int storage_idx, int offset )
{
    CvDTreeNode* node = (CvDTreeNode*)cvSetNew( node_heap );

    node->sample_count = count;
    node->depth = parent ? parent->depth + 1 : 0;
    node->parent = parent;
    node->left = node->right = 0;
    node->split = 0;
    node->value = 0;
    node->class_idx = 0;
    node->maxlr = 0.;

    node->buf_idx = storage_idx;
    node->offset = offset;
    if( nv_heap )
        node->num_valid = (int*)cvSetNew( nv_heap );
    else
        node->num_valid = 0;
    node->alpha = node->node_risk = node->tree_risk = node->tree_error = 0.;
    node->complexity = 0;

    if( params.cv_folds > 0 && cv_heap )
    {
        int cv_n = params.cv_folds;
        node->Tn = INT_MAX;
        node->cv_Tn = (int*)cvSetNew( cv_heap );
        node->cv_node_risk = (double*)cvAlignPtr(node->cv_Tn + cv_n, sizeof(double));
        node->cv_node_error = node->cv_node_risk + cv_n;
    }
    else
    {
        node->Tn = 0;
        node->cv_Tn = 0;
        node->cv_node_risk = 0;
        node->cv_node_error = 0;
    }

    return node;
}


CvDTreeSplit* CvDTreeTrainData::new_split_ord( int vi, float cmp_val,
                int split_point, int inversed, float quality )
{
    CvDTreeSplit* split = (CvDTreeSplit*)cvSetNew( split_heap );
    split->var_idx = vi;
    split->ord.c = cmp_val;
    split->ord.split_point = split_point;
    split->inversed = inversed;
    split->quality = quality;
    split->next = 0;

    return split;
}


CvDTreeSplit* CvDTreeTrainData::new_split_cat( int vi, float quality )
{
    CvDTreeSplit* split = (CvDTreeSplit*)cvSetNew( split_heap );
    int i, n = (max_c_count + 31)/32;

    split->var_idx = vi;
    split->inversed = 0;
    split->quality = quality;
    for( i = 0; i < n; i++ )
        split->subset[i] = 0;
    split->next = 0;

    return split;
}


void CvDTreeTrainData::free_node( CvDTreeNode* node )
{
    CvDTreeSplit* split = node->split;
    free_node_data( node );
    while( split )
    {
        CvDTreeSplit* next = split->next;
        cvSetRemoveByPtr( split_heap, split );
        split = next;
    }
    node->split = 0;
    cvSetRemoveByPtr( node_heap, node );
}


void CvDTreeTrainData::free_node_data( CvDTreeNode* node )
{
    if( node->num_valid )
    {
        cvSetRemoveByPtr( nv_heap, node->num_valid );
        node->num_valid = 0;
    }
    // do not free cv_* fields, as all the cross-validation related data is released at once.
}


void CvDTreeTrainData::free_train_data()
{
    cvReleaseMat( &counts );
    cvReleaseMat( &buf );
    cvReleaseMat( &direction );
    cvReleaseMat( &split_buf );
    cvReleaseMemStorage( &temp_storage );
    cv_heap = nv_heap = 0;
}


void CvDTreeTrainData::clear()
{
    free_train_data();

    cvReleaseMemStorage( &tree_storage );

    cvReleaseMat( &var_idx );
    cvReleaseMat( &var_type );
    cvReleaseMat( &cat_count );
    cvReleaseMat( &cat_ofs );
    cvReleaseMat( &cat_map );
    cvReleaseMat( &priors );

    node_heap = split_heap = 0;

    sample_count = var_all = var_count = max_c_count = ord_var_count = cat_var_count = 0;
    have_cv_labels = have_priors = is_classifier = false;

    buf_count = buf_size = 0;
    shared = false;

    data_root = 0;

    rng = cvRNG(-1);
}


int CvDTreeTrainData::get_num_classes() const
{
    return is_classifier ? cat_count->data.i[cat_var_count] : 0;
}


int CvDTreeTrainData::get_var_type(int vi) const
{
    return var_type->data.i[vi];
}


CvPair32s32f* CvDTreeTrainData::get_ord_var_data( CvDTreeNode* n, int vi )
{
    int oi = ~get_var_type(vi);
    assert( 0 <= oi && oi < ord_var_count );
    return (CvPair32s32f*)(buf->data.i + n->buf_idx*buf->cols +
                           n->offset + oi*n->sample_count*2);
}


int* CvDTreeTrainData::get_class_labels( CvDTreeNode* n )
{
    return get_cat_var_data( n, var_count );
}


float* CvDTreeTrainData::get_ord_responses( CvDTreeNode* n )
{
    return (float*)get_cat_var_data( n, var_count );
}


int* CvDTreeTrainData::get_cv_labels( CvDTreeNode* n )
{
    return params.cv_folds > 0 ? get_cat_var_data( n, var_count + 1 ) : 0;
}


int* CvDTreeTrainData::get_cat_var_data( CvDTreeNode* n, int vi )
{
    int ci = get_var_type(vi);
    assert( 0 <= ci && ci <= cat_var_count + 1 );
    return buf->data.i + n->buf_idx*buf->cols + n->offset +
           (ord_var_count*2 + ci)*n->sample_count;
}


float* CvDTreeTrainData::get_weights( CvDTreeNode* n )
{
    return have_weights ?
        (float*)get_cat_var_data( n, var_count + 1 + (params.cv_folds > 0) ) : 0;
}


int CvDTreeTrainData::get_child_buf_idx( CvDTreeNode* n )
{
    int idx = n->buf_idx + 1;
    if( idx >= buf_count )
        idx = shared ? 1 : 0;
    return idx;
}


/////////////////////// Decision Tree /////////////////////////

CvDTree::CvDTree()
{
    data = 0;
    var_importance = 0;
    default_model_name = "my_tree";

    clear();
}


void CvDTree::clear()
{
    cvReleaseMat( &var_importance );
    if( data )
    {
        if( !data->shared )
            delete data;
        else
            free_tree();
        data = 0;
    }
    root = 0;
    pruned_tree_idx = -1;
}


CvDTree::~CvDTree()
{
    clear();
}


const CvDTreeNode* CvDTree::get_root() const
{
    return root;
}


int CvDTree::get_pruned_tree_idx() const
{
    return pruned_tree_idx;
}


CvDTreeTrainData* CvDTree::get_data()
{
    return data;
}


bool CvDTree::train( const CvMat* _train_data, int _tflag,
                     const CvMat* _responses, const CvMat* _var_idx,
                     const CvMat* _sample_idx, const CvMat* _var_type,
                     const CvMat* _missing_mask, CvDTreeParams _params )
{
    bool result = false;

    CV_FUNCNAME( "CvDTree::train" );

    __BEGIN__;

    clear();
    data = new CvDTreeTrainData( _train_data, _tflag, _responses,
                                 _var_idx, _sample_idx, _var_type,
                                 _missing_mask, _params, false );
    CV_CALL( result = do_train(0));

    __END__;

    return result;
}


bool CvDTree::train( CvDTreeTrainData* _data, const CvMat* _subsample_idx )
{
    bool result = false;

    CV_FUNCNAME( "CvDTree::train" );

    __BEGIN__;

    clear();
    data = _data;
    data->shared = true;
    CV_CALL( result = do_train(_subsample_idx));

    __END__;

    return result;
}


bool CvDTree::do_train( const CvMat* _subsample_idx )
{
    bool result = false;

    CV_FUNCNAME( "CvDTree::do_train" );

    __BEGIN__;

    root = data->subsample_data( _subsample_idx );

    CV_CALL( try_split_node(root));
    
    if( data->params.cv_folds > 0 )
        CV_CALL( prune_cv());

    if( !data->shared )
        data->free_train_data();

    result = true;

    __END__;

    return result;
}


#define DTREE_CAT_DIR(idx,subset) \
    (2*((subset[(idx)>>5]&(1 << ((idx) & 31)))==0)-1)

void CvDTree::try_split_node( CvDTreeNode* node )
{
    CvDTreeSplit* best_split = 0;
    int i, n = node->sample_count, vi;
    bool can_split = true;
    double quality_scale;

    calc_node_value( node );

    if( node->sample_count <= data->params.min_sample_count ||
        node->depth >= data->params.max_depth )
        can_split = false;

    if( can_split && data->is_classifier )
    {
        // check if we have a "pure" node,
        // we assume that cls_count is filled by calc_node_value()
        int* cls_count = data->counts->data.i;
        int nz = 0, m = data->get_num_classes();
        for( i = 0; i < m; i++ )
            nz += cls_count[i] != 0;
        if( nz == 1 ) // there is only one class
            can_split = false;
    }
    else if( can_split )
    {
        const float* responses = data->get_ord_responses( node );
        float diff = responses[n-1] - responses[0];
        if( diff < data->params.regression_accuracy )
            can_split = false;
    }

    if( can_split )
    {
        best_split = find_best_split(node);
        // TODO: check the split quality ...
        node->split = best_split;
    }

    if( !can_split || !best_split )
    {
        data->free_node_data(node);
        return;
    }

    quality_scale = calc_node_dir( node );

    if( data->params.use_surrogates )
    {
        // find all the surrogate splits
        // and sort them by their similarity to the primary one
        for( vi = 0; vi < data->var_count; vi++ )
        {
            CvDTreeSplit* split;
            int ci = data->get_var_type(vi);

            if( vi == best_split->var_idx )
                continue;

            if( ci >= 0 )
                split = find_surrogate_split_cat( node, vi );
            else
                split = find_surrogate_split_ord( node, vi );

            if( split )
            {
                // insert the split
                CvDTreeSplit* prev_split = node->split;
                split->quality = (float)(split->quality*quality_scale);

                while( prev_split->next &&
                       prev_split->next->quality > split->quality )
                    prev_split = prev_split->next;
                split->next = prev_split->next;
                prev_split->next = split;
            }
        }
    }

    split_node_data( node );
    try_split_node( node->left );
    try_split_node( node->right );
}


// calculate direction (left(-1),right(1),missing(0))
// for each sample using the best split
// the function returns scale coefficients for surrogate split quality factors.
// the scale is applied to normalize surrogate split quality relatively to the
// best (primary) split quality. That is, if a surrogate split is absolutely
// identical to the primary split, its quality will be set to the maximum value = 
// quality of the primary split; otherwise, it will be lower.
// besides, the function compute node->maxlr,
// minimum possible quality (w/o considering the above mentioned scale)
// for a surrogate split. Surrogate splits with quality less than node->maxlr
// are not discarded.
double CvDTree::calc_node_dir( CvDTreeNode* node )
{
    char* dir = (char*)data->direction->data.ptr;
    int i, n = node->sample_count, vi = node->split->var_idx;
    double L, R;

    assert( !node->split->inversed );

    if( data->get_var_type(vi) >= 0 ) // split on categorical var
    {
        const int* labels = data->get_cat_var_data(node,vi);
        const int* subset = node->split->subset;

        if( !data->have_priors )
        {
            int sum = 0, sum_abs = 0;

            for( i = 0; i < n; i++ )
            {
                int idx = labels[i];
                int d = idx >= 0 ? DTREE_CAT_DIR(idx,subset) : 0;
                sum += d; sum_abs += d & 1;
                dir[i] = (char)d;
            }

            R = (sum_abs + sum) >> 1;
            L = (sum_abs - sum) >> 1;
        }
        else
        {
            const int* responses = data->get_class_labels(node);
            const double* priors = data->priors->data.db;
            double sum = 0, sum_abs = 0;

            for( i = 0; i < n; i++ )
            {
                int idx = labels[i];
                double w = priors[responses[i]];
                int d = idx >= 0 ? DTREE_CAT_DIR(idx,subset) : 0;
                sum += d*w; sum_abs += (d & 1)*w;
                dir[i] = (char)d;
            }

            R = (sum_abs + sum) * 0.5;
            L = (sum_abs - sum) * 0.5;
        }
    }
    else // split on ordered var
    {
        const CvPair32s32f* sorted = data->get_ord_var_data(node,vi);
        int split_point = node->split->ord.split_point;
        int n1 = node->get_num_valid(vi);

        assert( 0 <= split_point && split_point < n1-1 );

        if( !data->have_priors )
        {
            for( i = 0; i <= split_point; i++ )
                dir[sorted[i].i] = (char)-1;
            for( ; i < n1; i++ )
                dir[sorted[i].i] = (char)1;
            for( ; i < n; i++ )
                dir[sorted[i].i] = (char)0;

            L = split_point-1;
            R = n1 - split_point + 1;
        }
        else
        {
            const int* responses = data->get_class_labels(node);
            const double* priors = data->priors->data.db;
            L = R = 0;

            for( i = 0; i <= split_point; i++ )
            {
                int idx = sorted[i].i;
                double w = priors[responses[idx]];
                dir[idx] = (char)-1;
                L += w;
            }

            for( ; i < n1; i++ )
            {
                int idx = sorted[i].i;
                double w = priors[responses[idx]];
                dir[idx] = (char)1;
                R += w;
            }

            for( ; i < n; i++ )
                dir[sorted[i].i] = (char)0;
        }
    }

    node->maxlr = MAX( L, R );
    return node->split->quality/(L + R);
}


CvDTreeSplit* CvDTree::find_best_split( CvDTreeNode* node )
{
    int vi;
    CvDTreeSplit *best_split = 0, *split = 0, *t;

    for( vi = 0; vi < data->var_count; vi++ )
    {
        int ci = data->get_var_type(vi);
        if( node->get_num_valid(vi) <= 1 )
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

    return best_split;
}


CvDTreeSplit* CvDTree::find_split_ord_class( CvDTreeNode* node, int vi )
{
    const float epsilon = FLT_EPSILON*2;
    const CvPair32s32f* sorted = data->get_ord_var_data(node, vi);
    const int* responses = data->get_class_labels(node);
    int n = node->sample_count;
    int n1 = node->get_num_valid(vi);
    int m = data->get_num_classes();
    const int* rc0 = data->counts->data.i;
    int* lc = (int*)(rc0 + m);
    int* rc = lc + m;
    int i, best_i = -1;
    double lsum2 = 0, rsum2 = 0, best_val = 0;
    const double* priors = data->have_priors ? data->priors->data.db : 0;

    // init arrays of class instance counters on both sides of the split
    for( i = 0; i < m; i++ )
    {
        lc[i] = 0;
        rc[i] = rc0[i];
    }

    // compensate for missing values
    for( i = n1; i < n; i++ )
        rc[responses[sorted[i].i]]--;

    if( !priors )
    {
        int L = 0, R = n1;

        for( i = 0; i < m; i++ )
            rsum2 += (double)rc[i]*rc[i];

        for( i = 0; i < n1 - 1; i++ )
        {
            int idx = responses[sorted[i].i];
            int lv, rv;
            L++; R--;
            lv = lc[idx]; rv = rc[idx];
            lsum2 += lv*2 + 1;
            rsum2 -= rv*2 - 1;
            lc[idx] = lv + 1; rc[idx] = rv - 1;

            if( sorted[i].val + epsilon < sorted[i+1].val )
            {
                double val = lsum2/L + rsum2/R;
                if( best_val < val )
                {
                    best_val = val;
                    best_i = i;
                }
            }
        }
    }
    else
    {
        double L = 0, R = 0;
        for( i = 0; i < m; i++ )
        {
            double wv = rc[i]*priors[i];
            R += wv;
            rsum2 += wv*wv;
        }

        for( i = 0; i < n1 - 1; i++ )
        {
            int idx = responses[sorted[i].i];
            int lv, rv;
            double p = priors[idx], p2 = p*p;
            L += p; R -= p;
            lv = lc[idx]; rv = rc[idx];
            lsum2 += p2*(lv*2 + 1);
            rsum2 -= p2*(rv*2 - 1);
            lc[idx] = lv + 1; rc[idx] = rv - 1;

            if( sorted[i].val + epsilon < sorted[i+1].val )
            {
                double val = lsum2/L + rsum2/R;
                if( best_val < val )
                {
                    best_val = val;
                    best_i = i;
                }
            }
        }
    }

    return best_i >= 0 ? data->new_split_ord( vi,
        (sorted[best_i].val + sorted[best_i+1].val)*0.5f, best_i,
        0, (float)best_val ) : 0;
}


void CvDTree::cluster_categories( const int* vectors, int n, int m,
                                int* csums, int k, int* labels )
{
    // TODO: consider adding priors (class weights) and sample weights to the clustering algorithm
    int iters = 0, max_iters = 100;
    int i, j, idx;
    double* buf = (double*)cvStackAlloc( (n + k)*sizeof(buf[0]) );
    double *v_weights = buf, *c_weights = buf + k;
    bool modified = true;
    CvRNG* r = &data->rng;

    // assign labels randomly
    for( i = idx = 0; i < n; i++ )
    {
        int sum = 0;
        const int* v = vectors + i*m;
        labels[i] = idx++;
        idx &= idx < k ? -1 : 0;

        // compute weight of each vector
        for( j = 0; j < m; j++ )
            sum += v[j];
        v_weights[i] = sum ? 1./sum : 0.;
    }

    for( i = 0; i < n; i++ )
    {
        int i1 = cvRandInt(r) % n;
        int i2 = cvRandInt(r) % n;
        CV_SWAP( labels[i1], labels[i2], j );
    }

    for( iters = 0; iters <= max_iters; iters++ )
    {
        // calculate csums
        for( i = 0; i < k; i++ )
        {
            for( j = 0; j < m; j++ )
                csums[i*m + j] = 0;
        }

        for( i = 0; i < n; i++ )
        {
            const int* v = vectors + i*m;
            int* s = csums + labels[i]*m;
            for( j = 0; j < m; j++ )
                s[j] += v[j];
        }

        // exit the loop here, when we have up-to-date csums
        if( iters == max_iters || !modified )
            break;

        modified = false;

        // calculate weight of each cluster
        for( i = 0; i < k; i++ )
        {
            const int* s = csums + i*m;
            int sum = 0;
            for( j = 0; j < m; j++ )
                sum += s[j];
            c_weights[i] = sum ? 1./sum : 0;
        }

        // now for each vector determine the closest cluster
        for( i = 0; i < n; i++ )
        {
            const int* v = vectors + i*m;
            double alpha = v_weights[i];
            double min_dist2 = DBL_MAX;
            int min_idx = -1;

            for( idx = 0; idx < k; idx++ )
            {
                const int* s = csums + idx*m;
                double dist2 = 0., beta = c_weights[idx];
                for( j = 0; j < m; j++ )
                {
                    double t = v[j]*alpha - s[j]*beta;
                    dist2 += t*t;
                }
                if( min_dist2 > dist2 )
                {
                    min_dist2 = dist2;
                    min_idx = idx;
                }
            }

            if( min_idx != labels[i] )
                modified = true;
            labels[i] = min_idx;
        }
    }
}


CvDTreeSplit* CvDTree::find_split_cat_class( CvDTreeNode* node, int vi )
{
    CvDTreeSplit* split;
    const int* labels = data->get_cat_var_data(node, vi);
    const int* responses = data->get_class_labels(node);
    int ci = data->get_var_type(vi);
    int n = node->sample_count;
    int m = data->get_num_classes();
    int _mi = data->cat_count->data.i[ci], mi = _mi;
    const int* rc0 = data->counts->data.i;
    int* lc = (int*)(rc0 + m);
    int* rc = lc + m;
    int* _cjk = rc + m*2, *cjk = _cjk;
    double* c_weights = (double*)cvStackAlloc( mi*sizeof(c_weights[0]) );
    int* cluster_labels = 0;
    int** int_ptr = 0;
    int i, j, k, idx;
    double L = 0, R = 0;
    double best_val = 0;
    int prevcode = 0, best_subset = -1, subset_i, subset_n, subtract = 0;
    const double* priors = data->priors->data.db;

    // init array of counters:
    // c_{jk} - number of samples that have vi-th input variable = j and response = k.
    for( j = -1; j < mi; j++ )
        for( k = 0; k < m; k++ )
            cjk[j*m + k] = 0;

    for( i = 0; i < n; i++ )
    {
        j = labels[i];
        k = responses[i];
        cjk[j*m + k]++;
    }

    if( m > 2 )
    {
        if( mi > data->params.max_categories )
        {
            mi = MIN(data->params.max_categories, n);
            cjk += _mi*m;
            cluster_labels = cjk + mi*m;
            cluster_categories( _cjk, _mi, m, cjk, mi, cluster_labels );
        }
        subset_i = 1;
        subset_n = 1 << mi;
    }
    else
    {
        assert( m == 2 );
        int_ptr = (int**)cvStackAlloc( mi*sizeof(int_ptr[0]) );
        for( j = 0; j < mi; j++ )
            int_ptr[j] = cjk + j*2 + 1;
        icvSortIntPtr( int_ptr, mi, 0 );
        subset_i = 0;
        subset_n = mi;
    }

    for( k = 0; k < m; k++ )
    {
        int sum = 0;
        for( j = 0; j < mi; j++ )
            sum += cjk[j*m + k];
        rc[k] = sum;
        lc[k] = 0;
    }

    for( j = 0; j < mi; j++ )
    {
        double sum = 0;
        for( k = 0; k < m; k++ )
            sum += cjk[j*m + k]*priors[k];
        c_weights[j] = sum;
        R += c_weights[j];
    }

    for( ; subset_i < subset_n; subset_i++ )
    {
        double weight;
        int* crow;
        double lsum2 = 0, rsum2 = 0;

        if( m == 2 )
            idx = (int)(int_ptr[subset_i] - cjk)/2;
        else
        {
            int graycode = (subset_i>>1)^subset_i;
            int diff = graycode ^ prevcode;

            // determine index of the changed bit.
            Cv32suf u;
            idx = diff >= (1 << 16) ? 16 : 0;
            u.f = (float)(((diff >> 16) | diff) & 65535);
            idx += (u.i >> 23) - 127;
            subtract = graycode < prevcode;
            prevcode = graycode;
        }

        crow = cjk + idx*m;
        weight = c_weights[idx];
        if( weight < FLT_EPSILON )
            continue;

        if( !subtract )
        {
            for( k = 0; k < m; k++ )
            {
                int t = crow[k];
                int lval = lc[k] + t;
                int rval = rc[k] - t;
                double p = priors[k], p2 = p*p;
                lsum2 += p2*lval*lval;
                rsum2 += p2*rval*rval;
                lc[k] = lval; rc[k] = rval;
            }
            L += weight;
            R -= weight;
        }
        else
        {
            for( k = 0; k < m; k++ )
            {
                int t = crow[k];
                int lval = lc[k] - t;
                int rval = rc[k] + t;
                double p = priors[k], p2 = p*p;
                lsum2 += p2*lval*lval;
                rsum2 += p2*rval*rval;
                lc[k] = lval; rc[k] = rval;
            }
            L -= weight;
            R += weight;
        }

        if( L > FLT_EPSILON && R > FLT_EPSILON )
        {
            double val = lsum2/L + rsum2/R;
            if( best_val < val )
            {
                best_val = val;
                best_subset = subset_i;
            }
        }
    }

    if( best_subset < 0 )
        return 0;

    split = data->new_split_cat( vi, (float)best_val );

    if( m == 2 )
    {
        for( i = 0; i <= best_subset; i++ )
        {
            idx = (int)(int_ptr[i] - cjk) >> 1;
            split->subset[idx >> 5] |= 1 << (idx & 31);
        }
    }
    else
    {
        for( i = 0; i < _mi; i++ )
        {
            idx = cluster_labels ? cluster_labels[i] : i;
            if( best_subset & (1 << idx) )
                split->subset[i >> 5] |= 1 << (i & 31);
        }
    }

    return split;
}


CvDTreeSplit* CvDTree::find_split_ord_reg( CvDTreeNode* node, int vi )
{
    const float epsilon = FLT_EPSILON*2;
    const CvPair32s32f* sorted = data->get_ord_var_data(node, vi);
    const float* responses = data->get_ord_responses(node);
    int n = node->sample_count;
    int n1 = node->get_num_valid(vi);
    int i, best_i = -1;
    double best_val = 0, lsum = 0, rsum = node->value*n;
    int L = 0, R = n1;

    // compensate for missing values
    for( i = n1; i < n; i++ )
        rsum -= responses[sorted[i].i];

    // find the optimal split
    for( i = 0; i < n1 - 1; i++ )
    {
        float t = responses[sorted[i].i];
        L++; R--;
        lsum += t;
        rsum -= t;

        if( sorted[i].val + epsilon < sorted[i+1].val )
        {
            double val = lsum*lsum/L + rsum*rsum/R;
            if( best_val < val )
            {
                best_val = val;
                best_i = i;
            }
        }
    }

    return best_i >= 0 ? data->new_split_ord( vi,
        (sorted[best_i].val + sorted[best_i+1].val)*0.5f, best_i,
        0, (float)best_val ) : 0;
}


CvDTreeSplit* CvDTree::find_split_cat_reg( CvDTreeNode* node, int vi )
{
    CvDTreeSplit* split;
    const int* labels = data->get_cat_var_data(node, vi);
    const float* responses = data->get_ord_responses(node);
    int ci = data->get_var_type(vi);
    int n = node->sample_count;
    int mi = data->cat_count->data.i[ci];
    double* sum = (double*)cvStackAlloc( (mi+1)*sizeof(sum[0]) ) + 1;
    int* counts = (int*)cvStackAlloc( (mi+1)*sizeof(counts[0]) ) + 1;
    double** sum_ptr = 0;
    int i, L = 0, R = 0;
    double best_val = 0, lsum = 0, rsum = 0;
    int best_subset = -1, subset_i;

    for( i = -1; i < mi; i++ )
        sum[i] = counts[i] = 0;

    // calculate sum response and weight of each category of the input var
    for( i = 0; i < n; i++ )
    {
        int idx = labels[i];
        double s = sum[idx] + responses[i];
        int nc = counts[idx] + 1;
        sum[idx] = s;
        counts[idx] = nc;
    }

    // calculate average response in each category
    for( i = 0; i < mi; i++ )
    {
        R += counts[i];
        rsum += sum[i];
        sum[i] /= MAX(counts[i],1);
        sum_ptr[i] = sum + i;
    }

    icvSortDblPtr( sum_ptr, mi, 0 );

    // revert back to unnormalized sum
    // (there should be a very little loss of accuracy)
    for( i = 0; i < mi; i++ )
        sum[i] *= counts[i];

    for( subset_i = 0; subset_i < mi-1; subset_i++ )
    {
        int idx = (int)(sum_ptr[subset_i] - sum);
        int ni = counts[idx];

        if( ni )
        {
            double s = sum[idx];
            lsum += s; L += ni;
            rsum -= s; R -= ni;
            
            if( L && R )
            {
                double val = lsum*lsum/L + rsum*rsum/R;
                if( best_val < val )
                {
                    best_val = val;
                    best_subset = subset_i;
                }
            }
        }
    }

    if( best_subset < 0 )
        return 0;

    split = data->new_split_cat( vi, (float)best_val );
    for( i = 0; i <= best_subset; i++ )
    {
        int idx = (int)(sum_ptr[i] - sum);
        split->subset[idx >> 5] |= 1 << (idx & 31);
    }

    return split;
}


CvDTreeSplit* CvDTree::find_surrogate_split_ord( CvDTreeNode* node, int vi )
{
    const float epsilon = FLT_EPSILON*2;
    const CvPair32s32f* sorted = data->get_ord_var_data(node, vi);
    const char* dir = (char*)data->direction->data.ptr;
    int n1 = node->get_num_valid(vi);
    // LL - number of samples that both the primary and the surrogate splits send to the left
    // LR - ... primary split sends to the left and the surrogate split sends to the right
    // RL - ... primary split sends to the right and the surrogate split sends to the left
    // RR - ... both send to the right
    int i, best_i = -1, best_inversed = 0;
    double best_val; 

    if( !data->have_priors )
    {
        int LL = 0, RL = 0, LR, RR;
        int worst_val = cvFloor(node->maxlr), _best_val = worst_val;
        int sum = 0, sum_abs = 0;
        
        for( i = 0; i < n1; i++ )
        {
            int d = dir[sorted[i].i];
            sum += d; sum_abs += d & 1;
        }

        // sum_abs = R + L; sum = R - L
        RR = (sum_abs + sum) >> 1;
        LR = (sum_abs - sum) >> 1;

        // initially all the samples are sent to the right by the surrogate split,
        // LR of them are sent to the left by primary split, and RR - to the right.
        // now iteratively compute LL, LR, RL and RR for every possible surrogate split value.
        for( i = 0; i < n1 - 1; i++ )
        {
            int d = dir[sorted[i].i];

            if( d < 0 )
            {
                LL++; LR--;
                if( LL + RR > _best_val && sorted[i].val + epsilon < sorted[i+1].val )
                {
                    best_val = LL + RR;
                    best_i = i; best_inversed = 0;
                }
            }
            else if( d > 0 )
            {
                RL++; RR--;
                if( RL + LR > _best_val && sorted[i].val + epsilon < sorted[i+1].val )
                {
                    best_val = RL + LR;
                    best_i = i; best_inversed = 1;
                }
            }
        }
        best_val = _best_val;
    }
    else
    {
        double LL = 0, RL = 0, LR, RR;
        double worst_val = node->maxlr;
        double sum = 0, sum_abs = 0;
        const double* priors = data->priors->data.db;
        const int* responses = data->get_class_labels(node);
        best_val = worst_val;
        
        for( i = 0; i < n1; i++ )
        {
            int idx = sorted[i].i;
            double w = priors[responses[idx]];
            int d = dir[idx];
            sum += d*w; sum_abs += (d & 1)*w;
        }

        // sum_abs = R + L; sum = R - L
        RR = (sum_abs + sum)*0.5;
        LR = (sum_abs - sum)*0.5;

        // initially all the samples are sent to the right by the surrogate split,
        // LR of them are sent to the left by primary split, and RR - to the right.
        // now iteratively compute LL, LR, RL and RR for every possible surrogate split value.
        for( i = 0; i < n1 - 1; i++ )
        {
            int idx = sorted[i].i;
            double w = priors[responses[idx]];
            int d = dir[idx];

            if( d < 0 )
            {
                LL += w; LR -= w;
                if( LL + RR > best_val && sorted[i].val + epsilon < sorted[i+1].val )
                {
                    best_val = LL + RR;
                    best_i = i; best_inversed = 0;
                }
            }
            else if( d > 0 )
            {
                RL += w; RR -= w;
                if( RL + LR > best_val && sorted[i].val + epsilon < sorted[i+1].val )
                {
                    best_val = RL + LR;
                    best_i = i; best_inversed = 1;
                }
            }
        }
    }

    return best_i >= 0 ? data->new_split_ord( vi,
        (sorted[best_i].val + sorted[best_i+1].val)*0.5f, best_i,
        best_inversed, (float)best_val ) : 0;
}


CvDTreeSplit* CvDTree::find_surrogate_split_cat( CvDTreeNode* node, int vi )
{
    const int* labels = data->get_cat_var_data(node, vi);
    const char* dir = (char*)data->direction->data.ptr;
    int n = node->sample_count;
    // LL - number of samples that both the primary and the surrogate splits send to the left
    // LR - ... primary split sends to the left and the surrogate split sends to the right
    // RL - ... primary split sends to the right and the surrogate split sends to the left
    // RR - ... both send to the right
    CvDTreeSplit* split = data->new_split_cat( vi, 0 );
    int i, mi = data->cat_count->data.i[data->get_var_type(vi)];
    double best_val = 0;
    double* lc = (double*)cvStackAlloc( (mi+1)*2*sizeof(lc[0]) ) + 1;
    double* rc = lc + mi + 1;
    
    for( i = -1; i < mi; i++ )
        lc[i] = rc[i] = 0;

    // for each category calculate the weight of samples
    // sent to the left (lc) and to the right (rc) by the primary split
    if( !data->have_priors )
    {
        int* _lc = data->counts->data.i + 1;
        int* _rc = _lc + mi + 1;

        for( i = -1; i < mi; i++ )
            _lc[i] = _rc[i] = 0;

        for( i = 0; i < n; i++ )
        {
            int idx = labels[i];
            int d = dir[i];
            int sum = _lc[idx] + d;
            int sum_abs = _rc[idx] + (d & 1);
            _lc[idx] = sum; _rc[idx] = sum_abs;
        }

        for( i = 0; i < mi; i++ )
        {
            int sum = _lc[i];
            int sum_abs = _rc[i];
            lc[i] = (sum_abs - sum) >> 1;
            rc[i] = (sum_abs + sum) >> 1;
        }
    }
    else
    {
        const double* priors = data->priors->data.db;
        const int* responses = data->get_class_labels(node);

        for( i = 0; i < n; i++ )
        {
            int idx = labels[i];
            double w = priors[responses[i]];
            int d = dir[i];
            double sum = lc[idx] + d*w;
            double sum_abs = rc[idx] + (d & 1)*w;
            lc[idx] = sum; rc[idx] = sum_abs;
        }

        for( i = 0; i < mi; i++ )
        {
            double sum = lc[i];
            double sum_abs = rc[i];
            lc[i] = (sum_abs - sum) * 0.5;
            rc[i] = (sum_abs + sum) * 0.5;
        }
    }

    // 2. now form the split.
    // in each category send all the samples to the same direction as majority
    for( i = 0; i < mi; i++ )
    {
        double lval = lc[i], rval = rc[i];
        if( lval > rval )
        {
            split->subset[i >> 5] |= 1 << (i & 31);
            best_val += lval;
        }
        else
            best_val += rval;
    }

    split->quality = (float)best_val;
    if( split->quality <= node->maxlr )
        cvSetRemoveByPtr( data->split_heap, split ), split = 0;

    return split;
}


void CvDTree::calc_node_value( CvDTreeNode* node )
{
    int i, j, k, n = node->sample_count, cv_n = data->params.cv_folds;
    const int* cv_labels = data->get_cv_labels(node);

    if( data->is_classifier )
    {
        // in case of classification tree:
        //  * node value is the label of the class that has the largest weight in the node.
        //  * node risk is the weighted number of misclassified samples,
        //  * j-th cross-validation fold value and risk are calculated as above,
        //    but using the samples with cv_labels(*)!=j.
        //  * j-th cross-validation fold error is calculated as the weighted number of
        //    misclassified samples with cv_labels(*)==j.

        // compute the number of instances of each class
        int* cls_count = data->counts->data.i;
        const int* responses = data->get_class_labels(node);
        int m = data->get_num_classes();
        int* cv_cls_count = cls_count + m;
        double max_val = -1, total_weight = 0;
        int max_k = -1;
        double* priors = data->priors->data.db;

        for( k = 0; k < m; k++ )
            cls_count[k] = 0;

        if( cv_n == 0 )
        {
            for( i = 0; i < n; i++ )
                cls_count[responses[i]]++;
        }
        else
        {
            for( j = 0; j < cv_n; j++ )
                for( k = 0; k < m; k++ )
                    cv_cls_count[j*m + k] = 0;

            for( i = 0; i < n; i++ )
            {
                j = cv_labels[i]; k = responses[i];
                cv_cls_count[j*m + k]++;
            }

            for( j = 0; j < cv_n; j++ )
                for( k = 0; k < m; k++ )
                    cls_count[k] += cv_cls_count[j*m + k];
        }

        for( k = 0; k < m; k++ )
        {
            double val = cls_count[k]*priors[k];
            total_weight += val;
            if( max_val < val )
            {
                max_val = val;
                max_k = k;
            }
        }

        node->class_idx = max_k;
        node->value = data->cat_map->data.i[
            data->cat_ofs->data.i[data->cat_var_count] + max_k];
        node->node_risk = total_weight - max_val;

        for( j = 0; j < cv_n; j++ )
        {
            double sum_k = 0, sum = 0, max_val_k = 0;
            max_val = -1; max_k = -1;

            for( k = 0; k < m; k++ )
            {
                double w = priors[k];
                double val_k = cv_cls_count[j*m + k]*w;
                double val = cls_count[k]*w - val_k;
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
        // in case of regression tree:
        //  * node value is 1/n*sum_i(Y_i), where Y_i is i-th response,
        //    n is the number of samples in the node.
        //  * node risk is the sum of squared errors: sum_i((Y_i - <node_value>)^2)
        //  * j-th cross-validation fold value and risk are calculated as above,
        //    but using the samples with cv_labels(*)!=j.
        //  * j-th cross-validation fold error is calculated
        //    using samples with cv_labels(*)==j as the test subset:
        //    error_j = sum_(i,cv_labels(i)==j)((Y_i - <node_value_j>)^2),
        //    where node_value_j is the node value calculated
        //    as described in the previous bullet, and summation is done
        //    over the samples with cv_labels(*)==j.

        double sum = 0, sum2 = 0;
        const float* values = data->get_ord_responses(node);
        double *cv_sum = 0, *cv_sum2 = 0;
        int* cv_count = 0;
        
        if( cv_n == 0 )
        {
            // if cross-validation is not used, we even do not compute node_risk
            // (so the tree sequence T1>...>{root} may not be built).
            for( i = 0; i < n; i++ )
                sum += values[i];
        }
        else
        {
            cv_sum = (double*)cvStackAlloc( cv_n*sizeof(cv_sum[0]) );
            cv_sum2 = (double*)cvStackAlloc( cv_n*sizeof(cv_sum2[0]) );
            cv_count = (int*)cvStackAlloc( cv_n*sizeof(cv_count[0]) );

            for( j = 0; j < cv_n; j++ )
            {
                cv_sum[j] = cv_sum2[j] = 0.;
                cv_count[j] = 0;
            }

            for( i = 0; i < n; i++ )
            {
                j = cv_labels[i];
                double t = values[i];
                double s = cv_sum[j] + t;
                double s2 = cv_sum2[j] + t*t;
                int nc = cv_count[j] + 1;
                cv_sum[j] = s;
                cv_sum2[j] = s2;
                cv_count[j] = nc;
            }

            for( j = 0; j < cv_n; j++ )
            {
                sum += cv_sum[j];
                sum2 += cv_sum2[j];
            }

            node->node_risk = sum2 - (sum/n)*sum;
        }

        node->value = sum/n;

        for( j = 0; j < cv_n; j++ )
        {
            double s = cv_sum[j], si = sum - s;
            double s2 = cv_sum2[j], s2i = sum2 - s2;
            int c = cv_count[j], ci = n - c;
            double r = si/MAX(ci,1);
            node->cv_node_risk[j] = s2i - r*r*ci;
            node->cv_node_error[j] = s2 - 2*r*s + c*r*r;
            node->cv_Tn[j] = INT_MAX;
        }
    }
}


void CvDTree::complete_node_dir( CvDTreeNode* node )
{
    int vi, i, n = node->sample_count, nl, nr, d0 = 0, d1 = -1;
    int nz = n - node->get_num_valid(node->split->var_idx);
    char* dir = (char*)data->direction->data.ptr;

    // try to complete direction using surrogate splits
    if( nz && data->params.use_surrogates )
    {
        CvDTreeSplit* split = node->split->next;
        for( ; split != 0 && nz; split = split->next )
        {
            int inversed_mask = split->inversed ? -1 : 0;
            vi = split->var_idx;

            if( data->get_var_type(vi) >= 0 ) // split on categorical var
            {
                const int* labels = data->get_cat_var_data(node, vi);
                const int* subset = split->subset;

                for( i = 0; i < n; i++ )
                {
                    int idx;
                    if( !dir[i] && (idx = labels[i]) >= 0 )
                    {
                        int d = DTREE_CAT_DIR(idx,subset);
                        dir[i] = (char)((d ^ inversed_mask) - inversed_mask);
                        if( --nz )
                            break;
                    }
                }
            }
            else // split on ordered var
            {
                const CvPair32s32f* sorted = data->get_ord_var_data(node, vi);
                int split_point = split->ord.split_point;
                int n1 = node->get_num_valid(vi);

                assert( 0 <= split_point && split_point < n-1 );

                for( i = 0; i < n1; i++ )
                {
                    int idx = sorted[i].i;
                    if( !dir[idx] )
                    {
                        int d = i <= split_point ? -1 : 1;
                        dir[idx] = (char)((d ^ inversed_mask) - inversed_mask);
                        if( --nz )
                            break;
                    }
                }
            }
        }
    }

    // find the default direction for the rest
    if( nz )
    {
        for( i = nr = 0; i < n; i++ )
            nr += dir[i] > 0;
        nl = n - nr - nz;
        d0 = nl > nr ? -1 : nr > nl;
    }

    // make sure that every sample is directed either to the left or to the right
    for( i = 0; i < n; i++ )
    {
        int d = dir[i];
        if( !d )
        {
            d = d0;
            if( !d )
                d = d1, d1 = -d1;
        }
        d = d > 0;
        dir[i] = (char)d; // remap (-1,1) to (0,1)
    }
}


void CvDTree::split_node_data( CvDTreeNode* node )
{
    int vi, i, n = node->sample_count, nl, nr;
    char* dir = (char*)data->direction->data.ptr;
    CvDTreeNode *left = 0, *right = 0;
    int* new_idx = data->split_buf->data.i;
    int new_buf_idx = data->get_child_buf_idx( node );

    complete_node_dir(node);

    for( i = nl = nr = 0; i < n; i++ )
    {
        int d = dir[i];
        // initialize new indices for splitting ordered variables
        new_idx[i] = (nl & (d-1)) | (nr & -d); // d ? ri : li
        nr += d;
        nl += d^1;
    }

    node->left = left = data->new_node( node, nl, new_buf_idx, node->offset );
    node->right = right = data->new_node( node, nr, new_buf_idx, node->offset +
        (data->ord_var_count*2 + data->cat_var_count+1+data->have_cv_labels)*nl );

    // split ordered variables, keep both halves sorted.
    for( vi = 0; vi < data->var_count; vi++ )
    {
        int ci = data->get_var_type(vi);
        int n1 = node->get_num_valid(vi);
        CvPair32s32f *src, *ldst0, *rdst0, *ldst, *rdst;
        CvPair32s32f tl, tr;

        if( ci >= 0 )
            continue;

        src = data->get_ord_var_data(node, vi);
        ldst0 = ldst = data->get_ord_var_data(left, vi);
        rdst0 = rdst = data->get_ord_var_data(right, vi);
        tl = ldst0[nl]; tr = rdst0[nr];

        // split sorted
        for( i = 0; i < n1; i++ )
        {
            int idx = src[i].i;
            float val = src[i].val;
            int d = dir[idx];
            idx = new_idx[idx];
            ldst->i = rdst->i = idx;
            ldst->val = rdst->val = val;
            ldst += d^1;
            rdst += d;
        }

        left->set_num_valid(vi, (int)(ldst - ldst0));
        right->set_num_valid(vi, (int)(rdst - rdst0));

        // split missing
        for( ; i < n; i++ )
        {
            int idx = src[i].i;
            int d = dir[idx];
            idx = new_idx[idx];
            ldst->i = rdst->i = idx;
            ldst->val = rdst->val = ord_nan;
            ldst += d^1;
            rdst += d;
        }

        ldst0[nl] = tl; rdst0[nr] = tr;
    }

    // split categorical vars, responses and cv_labels using new_idx relocation table
    for( vi = 0; vi <= data->var_count + data->have_cv_labels + data->have_weights; vi++ )
    {
        int ci = data->get_var_type(vi);
        int n1 = node->get_num_valid(vi), nr1 = 0;
        int *src, *ldst0, *rdst0, *ldst, *rdst;
        int tl, tr;

        if( ci < 0 )
            continue;

        src = data->get_cat_var_data(node, vi);
        ldst0 = ldst = data->get_cat_var_data(left, vi);
        rdst0 = rdst = data->get_cat_var_data(right, vi);
        tl = ldst0[nl]; tr = rdst0[nr];

        for( i = 0; i < n; i++ )
        {
            int d = dir[i];
            int val = src[i];
            *ldst = *rdst = val;
            ldst += d^1;
            rdst += d;
            nr1 += (val >= 0)&d;
        }

        if( vi < data->var_count )
        {
            left->set_num_valid(vi, n1 - nr1);
            right->set_num_valid(vi, nr1);
        }

        ldst0[nl] = tl; rdst0[nr] = tr;
    }

    // deallocate the parent node data that is not needed anymore
    data->free_node_data(node);
}


void CvDTree::prune_cv()
{
    CvMat* ab = 0;
    CvMat* temp = 0;
    CvMat* err_jk = 0;
    
    // 1. build tree sequence for each cv fold, calculate error_{Tj,beta_k}.
    // 2. choose the best tree index (if need, apply 1SE rule).
    // 3. store the best index and cut the branches.

    CV_FUNCNAME( "CvDTree::prune_cv" );

    __BEGIN__;

    int ti, j, tree_count = 0, cv_n = data->params.cv_folds, n = root->sample_count;
    // currently, 1SE for regression is not implemented
    bool use_1se = data->params.use_1se_rule != 0 && data->is_classifier;
    double* err;
    double min_err = 0, min_err_se = 0;
    int min_idx = -1;
    
    CV_CALL( ab = cvCreateMat( 1, 256, CV_64F ));

    // build the main tree sequence, calculate alpha's
    for(;;tree_count++)
    {
        double min_alpha = update_tree_rnc(tree_count, -1);
        if( cut_tree(tree_count, -1, min_alpha) )
            break;

        if( ab->cols <= tree_count )
        {
            CV_CALL( temp = cvCreateMat( 1, ab->cols*3/2, CV_64F ));
            for( ti = 0; ti < ab->cols; ti++ )
                temp->data.db[ti] = ab->data.db[ti];
            cvReleaseMat( &ab );
            ab = temp;
            temp = 0;
        }

        ab->data.db[tree_count] = min_alpha;
    }

    ab->data.db[0] = 0.;
    for( ti = 1; ti < tree_count-1; ti++ )
        ab->data.db[ti] = sqrt(ab->data.db[ti]*ab->data.db[ti+1]);
    ab->data.db[tree_count-1] = DBL_MAX*0.5;

    CV_CALL( err_jk = cvCreateMat( cv_n, tree_count, CV_64F ));
    err = err_jk->data.db;

    for( j = 0; j < cv_n; j++ )
    {
        int tj = 0, tk = 0;
        for( ; tk < tree_count; tj++ )
        {
            double min_alpha = update_tree_rnc(tj, j);
            if( cut_tree(tj, j, min_alpha) )
                min_alpha = DBL_MAX;

            for( ; tk < tree_count; tk++ )
            {
                if( ab->data.db[tk] > min_alpha )
                    break;
                err[j*tree_count + tk] = root->tree_error;
            }
        }
    }

    for( ti = 0; ti < tree_count; ti++ )
    {
        double sum_err = 0;
        for( j = 0; j < cv_n; j++ )
            sum_err += err[j*tree_count + ti];
        if( ti == 0 || sum_err < min_err )
        {
            min_err = sum_err;
            min_idx = ti;
            if( use_1se )
                min_err_se = sqrt( sum_err*(n - sum_err) );
        }
        else if( sum_err < min_err + min_err_se )
            min_idx = ti;
    }

    pruned_tree_idx = min_idx;
    free_prune_data(data->params.truncate_pruned_tree != 0);

    __END__;

    cvReleaseMat( &err_jk );
    cvReleaseMat( &ab );
    cvReleaseMat( &temp );
}


double CvDTree::update_tree_rnc( int T, int fold )
{
    CvDTreeNode* node = root;
    double min_alpha = DBL_MAX;
    
    for(;;)
    {
        CvDTreeNode* parent;
        for(;;)
        {
            int t = fold >= 0 ? node->cv_Tn[fold] : node->Tn;
            if( t <= T || !node->left )
            {
                node->complexity = 1;
                node->tree_risk = node->node_risk;
                node->tree_error = 0.;
                if( fold >= 0 )
                {
                    node->tree_risk = node->cv_node_risk[fold];
                    node->tree_error = node->cv_node_error[fold];
                }
                break;
            }
            node = node->left;
        }
        
        for( parent = node->parent; parent && parent->right == node;
            node = parent, parent = parent->parent )
        {
            parent->complexity += node->complexity;
            parent->tree_risk += node->tree_risk;
            parent->tree_error += node->tree_error;

            parent->alpha = ((fold >= 0 ? parent->cv_node_risk[fold] : parent->node_risk)
                - parent->tree_risk)/(parent->complexity - 1);
            min_alpha = MIN( min_alpha, parent->alpha );
        }

        if( !parent )
            break;

        parent->complexity = node->complexity;
        parent->tree_risk = node->tree_risk;
        parent->tree_error = node->tree_error;
        node = parent->right;
    }

    return min_alpha;
}


int CvDTree::cut_tree( int T, int fold, double min_alpha )
{
    CvDTreeNode* node = root;
    if( !node->left )
        return 1;

    for(;;)
    {
        CvDTreeNode* parent;
        for(;;)
        {
            int t = fold >= 0 ? node->cv_Tn[fold] : node->Tn;
            if( t <= T || !node->left )
                break;
            if( node->alpha <= min_alpha + FLT_EPSILON )
            {
                if( fold >= 0 )
                    node->cv_Tn[fold] = T;
                else
                    node->Tn = T;
                if( node == root )
                    return 1;
                break;
            }
            node = node->left;
        }
        
        for( parent = node->parent; parent && parent->right == node;
            node = parent, parent = parent->parent )
            ;

        if( !parent )
            break;

        node = parent->right;
    }

    return 0;
}


void CvDTree::free_prune_data(bool cut_tree)
{
    CvDTreeNode* node = root;
    
    for(;;)
    {
        CvDTreeNode* parent;
        for(;;)
        {
            // do not call cvSetRemoveByPtr( cv_heap, node->cv_Tn )
            // as we will clear the whole cross-validation heap at the end
            node->cv_Tn = 0;
            node->cv_node_error = node->cv_node_risk = 0;
            if( !node->left )
                break;
            node = node->left;
        }
        
        for( parent = node->parent; parent && parent->right == node;
            node = parent, parent = parent->parent )
        {
            if( cut_tree && parent->Tn <= pruned_tree_idx )
            {
                data->free_node( parent->left );
                data->free_node( parent->right );
                parent->left = parent->right = 0;
            }
        }

        if( !parent )
            break;

        node = parent->right;
    }

    if( data->cv_heap )
        cvClearSet( data->cv_heap );
}


void CvDTree::free_tree()
{
    if( root && data && data->shared )
    {
        pruned_tree_idx = INT_MIN;
        free_prune_data(true);
        data->free_node(root);
        root = 0;
    }
}


CvDTreeNode* CvDTree::predict( const CvMat* _sample,
    const CvMat* _missing, bool preprocessed_input ) const
{
    CvDTreeNode* result = 0;
    int* catbuf = 0;

    CV_FUNCNAME( "CvDTree::predict" );

    __BEGIN__;

    int i, step, mstep = 0;
    const float* sample;
    const uchar* m = 0;
    CvDTreeNode* node = root;
    const int* vtype;
    const int* vidx;
    const int* cmap;
    const int* cofs;

    if( !node )
        CV_ERROR( CV_StsError, "The tree has not been trained yet" );

    if( !CV_IS_MAT(_sample) || CV_MAT_TYPE(_sample->type) != CV_32FC1 ||
        _sample->cols != 1 && _sample->rows != 1 ||
        _sample->cols + _sample->rows - 1 != data->var_all && !preprocessed_input ||
        _sample->cols + _sample->rows - 1 != data->var_count && preprocessed_input )
            CV_ERROR( CV_StsBadArg,
        "the input sample must be 1d floating-point vector with the same "
        "number of elements as the total number of variables used for training" );

    sample = _sample->data.fl;
    step = CV_IS_MAT_CONT(_sample->type) ? 1 : _sample->step/sizeof(data[0]);

    if( data->cat_count && !preprocessed_input ) // cache for categorical variables
    {
        int n = data->cat_count->cols;
        catbuf = (int*)cvStackAlloc(n*sizeof(catbuf[0]));
        for( i = 0; i < n; i++ )
            catbuf[i] = -1;
    }

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
    cmap = data->cat_map->data.i;
    cofs = data->cat_ofs->data.i;

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
                if( preprocessed_input )
                    c = cvRound(val);
                else
                {
                    c = catbuf[ci];
                    if( c < 0 )
                    {
                        int a = c = cofs[ci];
                        int b = cofs[ci+1];
                        int ival = cvRound(val);
                        if( ival != val )
                            CV_ERROR( CV_StsBadArg,
                            "one of input categorical variable is not an integer" );

                        while( a < b )
                        {
                            c = (a + b) >> 1;
                            if( ival < cmap[c] )
                                b = c;
                            else if( ival > cmap[c] )
                                a = c+1;
                            else
                                break;
                        }

                        if( c < 0 || ival != cmap[c] )
                            continue;

                        catbuf[ci] = c -= cofs[ci];
                    }
                }
                dir = DTREE_CAT_DIR(c, split->subset);
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


const CvMat* CvDTree::get_var_importance()
{
    if( !var_importance )
    {
        CvDTreeNode* node = root;
        double* importance;
        if( !node )
            return 0;
        var_importance = cvCreateMat( 1, data->var_count, CV_64F );
        cvZero( var_importance );
        importance = var_importance->data.db;

        for(;;)
        {
            CvDTreeNode* parent;
            for( ;; node = node->left )
            {
                CvDTreeSplit* split = node->split;
                
                if( !node->left || node->Tn <= pruned_tree_idx )
                    break;
                
                for( ; split != 0; split = split->next )
                    importance[split->var_idx] += split->quality;
            }

            for( parent = node->parent; parent && parent->right == node;
                node = parent, parent = parent->parent )
                ;

            if( !parent )
                break;

            node = parent->right;
        }

        cvNormalize( var_importance, var_importance, 1., 0, CV_L1 );
    }

    return var_importance;
}


void CvDTree::write_train_data_params( CvFileStorage* fs )
{
    CV_FUNCNAME( "CvDTree::write_train_data_params" );

    __BEGIN__;

    int vi, vcount = data->var_count;

    cvWriteInt( fs, "is_classifier", data->is_classifier ? 1 : 0 );
    cvWriteInt( fs, "var_all", data->var_all );
    cvWriteInt( fs, "var_count", data->var_count );
    cvWriteInt( fs, "ord_var_count", data->ord_var_count );
    cvWriteInt( fs, "cat_var_count", data->cat_var_count );

    cvStartWriteStruct( fs, "training_params", CV_NODE_MAP );
    cvWriteInt( fs, "use_surrogates", data->params.use_surrogates ? 1 : 0 );

    if( data->is_classifier )
    {
        cvWriteInt( fs, "max_categories", data->params.max_categories );
    }
    else
    {
        cvWriteReal( fs, "regression_accuracy", data->params.regression_accuracy );
    }

    cvWriteInt( fs, "max_depth", data->params.max_depth );
    cvWriteInt( fs, "min_sample_count", data->params.min_sample_count );
    cvWriteInt( fs, "cross_validation_folds", data->params.cv_folds );
    
    if( data->params.cv_folds > 1 )
    {
        cvWriteInt( fs, "use_1se_rule", data->params.use_1se_rule ? 1 : 0 );
        cvWriteInt( fs, "truncate_pruned_tree", data->params.truncate_pruned_tree ? 1 : 0 );
    }

    if( data->priors )
        cvWrite( fs, "priors", data->priors );

    cvEndWriteStruct( fs );

    if( data->var_idx )
        cvWrite( fs, "var_idx", data->var_idx );
    
    cvStartWriteStruct( fs, "var_type", CV_NODE_SEQ+CV_NODE_FLOW );

    for( vi = 0; vi < vcount; vi++ )
        cvWriteInt( fs, 0, data->var_type->data.i[vi] >= 0 );

    cvEndWriteStruct( fs );

    if( data->cat_count && (data->cat_var_count > 0 || data->is_classifier) )
    {
        CV_ASSERT( data->cat_count != 0 );
        cvWrite( fs, "cat_count", data->cat_count );
        cvWrite( fs, "cat_map", data->cat_map );
    }

    __END__;
}


void CvDTree::write_split( CvFileStorage* fs, CvDTreeSplit* split )
{
    int ci;
    
    cvStartWriteStruct( fs, 0, CV_NODE_MAP + CV_NODE_FLOW );
    cvWriteInt( fs, "var", split->var_idx );
    cvWriteReal( fs, "quality", split->quality );

    ci = data->get_var_type(split->var_idx);
    if( ci >= 0 ) // split on a categorical var
    {
        int i, n = data->cat_count->data.i[ci], to_right = 0, default_dir;
        for( i = 0; i < n; i++ )
            to_right += DTREE_CAT_DIR(i,split->subset) > 0;

        // ad-hoc rule when to use inverse categorical split notation
        // to achieve more compact and clear representation
        default_dir = to_right <= 1 || to_right <= MIN(3, n/2) || to_right <= n/3 ? -1 : 1;
        
        cvStartWriteStruct( fs, default_dir*(split->inversed ? -1 : 1) > 0 ?
                            "in" : "not_in", CV_NODE_SEQ+CV_NODE_FLOW );
        for( i = 0; i < n; i++ )
        {
            int dir = DTREE_CAT_DIR(i,split->subset);
            if( dir*default_dir < 0 )
                cvWriteInt( fs, 0, i );
        }
        cvEndWriteStruct( fs );
    }
    else
        cvWriteReal( fs, !split->inversed ? "le" : "gt", split->ord.c );

    cvEndWriteStruct( fs );
}


void CvDTree::write_node( CvFileStorage* fs, CvDTreeNode* node )
{
    CvDTreeSplit* split;
    
    cvStartWriteStruct( fs, 0, CV_NODE_MAP );

    cvWriteInt( fs, "depth", node->depth );
    cvWriteInt( fs, "sample_count", node->sample_count );
    cvWriteReal( fs, "value", node->value );
    
    if( data->is_classifier )
        cvWriteInt( fs, "norm_class_idx", node->class_idx );

    cvWriteInt( fs, "Tn", node->Tn );
    cvWriteInt( fs, "complexity", node->complexity );
    cvWriteReal( fs, "alpha", node->alpha );
    cvWriteReal( fs, "node_risk", node->node_risk );
    cvWriteReal( fs, "tree_risk", node->tree_risk );
    cvWriteReal( fs, "tree_error", node->tree_error );

    if( node->left )
    {
        cvStartWriteStruct( fs, "splits", CV_NODE_SEQ );

        for( split = node->split; split != 0; split = split->next )
            write_split( fs, split );

        cvEndWriteStruct( fs );
    }

    cvEndWriteStruct( fs );
}


void CvDTree::write_tree_nodes( CvFileStorage* fs )
{
    //CV_FUNCNAME( "CvDTree::write_tree_nodes" );

    __BEGIN__;

    CvDTreeNode* node = root;

    // traverse the tree and save all the nodes in depth-first order
    for(;;)
    {
        CvDTreeNode* parent;
        for(;;)
        {
            write_node( fs, node );
            if( !node->left )
                break;
            node = node->left;
        }
        
        for( parent = node->parent; parent && parent->right == node;
            node = parent, parent = parent->parent )
            ;

        if( !parent )
            break;

        node = parent->right;
    }

    __END__;
}


void CvDTree::write( CvFileStorage* fs, const char* name )
{
    //CV_FUNCNAME( "CvDTree::write" );

    __BEGIN__;

    cvStartWriteStruct( fs, name, CV_NODE_MAP, CV_TYPE_NAME_ML_TREE );

    write_train_data_params( fs );

    cvWriteInt( fs, "best_tree_idx", pruned_tree_idx );
    get_var_importance();
    cvWrite( fs, "var_importance", var_importance );

    cvStartWriteStruct( fs, "nodes", CV_NODE_SEQ );
    write_tree_nodes( fs );
    cvEndWriteStruct( fs );

    cvEndWriteStruct( fs );

    __END__;
}


void CvDTree::read_train_data_params( CvFileStorage* fs, CvFileNode* node )
{
    CV_FUNCNAME( "CvDTree::read_train_data_params" );

    __BEGIN__;
    
    CvDTreeParams params;
    CvFileNode *tparams_node, *vartype_node;
    CvSeqReader reader;
    int is_classifier, vi, cat_var_count, ord_var_count;
    int max_split_size, tree_block_size;

    data = new CvDTreeTrainData;

    is_classifier = (cvReadIntByName( fs, node, "is_classifier" ) != 0);
    data->is_classifier = is_classifier;
    data->var_all = cvReadIntByName( fs, node, "var_all" );
    data->var_count = cvReadIntByName( fs, node, "var_count", data->var_all );
    data->cat_var_count = cvReadIntByName( fs, node, "cat_var_count" );
    data->ord_var_count = cvReadIntByName( fs, node, "ord_var_count" );

    tparams_node = cvGetFileNodeByName( fs, node, "training_params" );

    if( tparams_node ) // training parameters are not necessary
    {
        data->params.use_surrogates = cvReadIntByName( fs, tparams_node, "use_surrogates", 1 ) != 0;

        if( is_classifier )
        {
            data->params.max_categories = cvReadIntByName( fs, tparams_node, "max_categories" );
        }
        else
        {
            data->params.regression_accuracy =
                (float)cvReadRealByName( fs, tparams_node, "regression_accuracy" );
        }

        data->params.max_depth = cvReadIntByName( fs, tparams_node, "max_depth" );
        data->params.min_sample_count = cvReadIntByName( fs, tparams_node, "min_sample_count" );
        data->params.cv_folds = cvReadIntByName( fs, tparams_node, "cross_validation_folds" );
    
        if( data->params.cv_folds > 1 )
        {
            data->params.use_1se_rule = cvReadIntByName( fs, tparams_node, "use_1se_rule" ) != 0;
            data->params.truncate_pruned_tree =
                cvReadIntByName( fs, tparams_node, "truncate_pruned_tree" ) != 0;
        }

        data->priors = (CvMat*)cvReadByName( fs, tparams_node, "priors" );
        if( data->priors && !CV_IS_MAT(data->priors) )
            CV_ERROR( CV_StsParseError, "priors must stored as a matrix" );
    }

    CV_CALL( data->var_idx = (CvMat*)cvReadByName( fs, node, "var_idx" ));
    if( data->var_idx )
    {
        if( !CV_IS_MAT(data->var_idx) ||
            data->var_idx->cols != 1 && data->var_idx->rows != 1 ||
            data->var_idx->cols + data->var_idx->rows - 1 != data->var_count ||
            CV_MAT_TYPE(data->var_idx->type) != CV_32SC1 )
            CV_ERROR( CV_StsParseError,
                "var_idx (if exist) must be valid 1d integer vector containing <var_count> elements" );

        for( vi = 0; vi < data->var_count; vi++ )
            if( (unsigned)data->var_idx->data.i[vi] >= (unsigned)data->var_all )
                CV_ERROR( CV_StsOutOfRange, "some of var_idx elements are out of range" );
    }
    
    ////// read var type
    CV_CALL( data->var_type = cvCreateMat( 1, data->var_count + 2, CV_32SC1 ));

    vartype_node = cvGetFileNodeByName( fs, node, "var_type" );
    if( !vartype_node || CV_NODE_TYPE(vartype_node->tag) != CV_NODE_SEQ ||
        vartype_node->data.seq->total != data->var_count )
        CV_ERROR( CV_StsParseError, "var_type must exist and be a sequence of 0's and 1's" );

    cvStartReadSeq( vartype_node->data.seq, &reader );

    cat_var_count = 0;
    ord_var_count = -1;
    for( vi = 0; vi < data->var_count; vi++ )
    {
        CvFileNode* n = (CvFileNode*)reader.ptr;
        if( CV_NODE_TYPE(n->tag) != CV_NODE_INT || (n->data.i & ~1) )
            CV_ERROR( CV_StsParseError, "var_type must exist and be a sequence of 0's and 1's" );
        data->var_type->data.i[vi] = n->data.i ? cat_var_count++ : ord_var_count--;
        CV_NEXT_SEQ_ELEM( reader.seq->elem_size, reader );
    }
    
    ord_var_count = ~ord_var_count;
    if( cat_var_count != data->cat_var_count || ord_var_count != data->ord_var_count )
        CV_ERROR( CV_StsParseError, "var_type is inconsistent with cat_var_count and ord_var_count" );
    //////

    if( data->cat_var_count > 0 || is_classifier )
    {
        int ccount, max_c_count = 0, total_c_count = 0;
        CV_CALL( data->cat_count = (CvMat*)cvReadByName( fs, node, "cat_count" ));
        CV_CALL( data->cat_map = (CvMat*)cvReadByName( fs, node, "cat_map" ));

        if( !CV_IS_MAT(data->cat_count) || !CV_IS_MAT(data->cat_map) ||
            data->cat_count->cols != 1 && data->cat_count->rows != 1 ||
            CV_MAT_TYPE(data->cat_count->type) != CV_32SC1 ||
            data->cat_count->cols + data->cat_count->rows - 1 != cat_var_count + is_classifier ||
            data->cat_map->cols != 1 && data->cat_map->rows != 1 ||
            CV_MAT_TYPE(data->cat_map->type) != CV_32SC1 )
            CV_ERROR( CV_StsParseError,
            "Both cat_count and cat_map must exist and be valid 1d integer vectors of an appropriate size" );

        ccount = cat_var_count + is_classifier;

        CV_CALL( data->cat_ofs = cvCreateMat( 1, ccount + 1, CV_32SC1 ));
        data->cat_ofs->data.i[0] = 0;

        for( vi = 0; vi < ccount; vi++ )
        {
            int val = data->cat_count->data.i[vi];
            if( val <= 0 )
                CV_ERROR( CV_StsOutOfRange, "some of cat_count elements are out of range" );
            max_c_count = MAX( max_c_count, val );
            data->cat_ofs->data.i[vi+1] = total_c_count += val;
        }

        if( data->cat_map->cols + data->cat_map->rows - 1 != total_c_count )
            CV_ERROR( CV_StsBadSize,
            "cat_map vector length is not equal to the total number of categories in all categorical vars" );
        
        data->max_c_count = max_c_count;
    }

    max_split_size = cvAlign(sizeof(CvDTreeSplit) +
        (MAX(0,data->max_c_count - 33)/32)*sizeof(int),sizeof(void*));

    tree_block_size = MAX((int)sizeof(CvDTreeNode)*8, max_split_size);
    tree_block_size = MAX(tree_block_size + block_size_delta, min_block_size);
    CV_CALL( data->tree_storage = cvCreateMemStorage( tree_block_size ));
    CV_CALL( data->node_heap = cvCreateSet( 0, sizeof(data->node_heap[0]),
            sizeof(CvDTreeNode), data->tree_storage ));
    CV_CALL( data->split_heap = cvCreateSet( 0, sizeof(data->split_heap[0]),
            max_split_size, data->tree_storage ));

    __END__;
}


CvDTreeSplit* CvDTree::read_split( CvFileStorage* fs, CvFileNode* fnode )
{
    CvDTreeSplit* split = 0;
    
    CV_FUNCNAME( "CvDTree::read_split" );

    __BEGIN__;

    int vi, ci;
    
    if( !fnode || CV_NODE_TYPE(fnode->tag) != CV_NODE_MAP )
        CV_ERROR( CV_StsParseError, "some of the splits are not stored properly" );

    vi = cvReadIntByName( fs, fnode, "var", -1 );
    if( (unsigned)vi >= (unsigned)data->var_count )
        CV_ERROR( CV_StsOutOfRange, "Split variable index is out of range" );

    ci = data->get_var_type(vi);
    if( ci >= 0 ) // split on categorical var
    {
        int i, n = data->cat_count->data.i[ci], inversed = 0;
        CvSeqReader reader;
        CvFileNode* inseq;
        split = data->new_split_cat( vi, 0 );
        inseq = cvGetFileNodeByName( fs, fnode, "in" );
        if( !inseq )
        {
            inseq = cvGetFileNodeByName( fs, fnode, "not_in" );
            inversed = 1;
        }
        if( !inseq || CV_NODE_TYPE(inseq->tag) != CV_NODE_SEQ )
            CV_ERROR( CV_StsParseError,
            "Either 'in' or 'not_in' tags should be inside a categorical split data" );

        cvStartReadSeq( inseq->data.seq, &reader );

        for( i = 0; i < reader.seq->total; i++ )
        {
            CvFileNode* inode = (CvFileNode*)reader.ptr;
            int val = inode->data.i;
            if( CV_NODE_TYPE(inode->tag) != CV_NODE_INT || (unsigned)val >= (unsigned)n )
                CV_ERROR( CV_StsOutOfRange, "some of in/not_in elements are out of range" );

            split->subset[val >> 5] |= 1 << (val & 31);
            CV_NEXT_SEQ_ELEM( reader.seq->elem_size, reader );
        }

        // for categorical splits we do not use inversed splits,
        // instead we inverse the variable set in the split
        if( inversed )
            for( i = 0; i < (n + 31) >> 5; i++ )
                split->subset[i] ^= -1;
    }
    else
    {
        CvFileNode* cmp_node;
        split = data->new_split_ord( vi, 0, 0, 0, 0 );

        cmp_node = cvGetFileNodeByName( fs, fnode, "le" );
        if( !cmp_node )
        {
            cmp_node = cvGetFileNodeByName( fs, fnode, "gt" );
            split->inversed = 1;
        }

        split->ord.c = (float)cvReadReal( cmp_node );
    }
        
    split->quality = (float)cvReadRealByName( fs, fnode, "quality" );

    __END__;
    
    return split;
}


CvDTreeNode* CvDTree::read_node( CvFileStorage* fs, CvFileNode* fnode, CvDTreeNode* parent )
{
    CvDTreeNode* node = 0;
    
    CV_FUNCNAME( "CvDTree::read_node" );

    __BEGIN__;

    CvFileNode* splits;
    int i, depth;

    if( !fnode || CV_NODE_TYPE(fnode->tag) != CV_NODE_MAP )
        CV_ERROR( CV_StsParseError, "some of the tree elements are not stored properly" );

    CV_CALL( node = data->new_node( parent, 0, 0, 0 ));
    depth = cvReadIntByName( fs, fnode, "depth", -1 );
    if( depth != node->depth )
        CV_ERROR( CV_StsParseError, "incorrect node depth" );

    node->sample_count = cvReadIntByName( fs, fnode, "sample_count" );
    node->value = cvReadRealByName( fs, fnode, "value" );
    if( data->is_classifier )
        node->class_idx = cvReadIntByName( fs, fnode, "norm_class_idx" );

    node->Tn = cvReadIntByName( fs, fnode, "Tn" );
    node->complexity = cvReadIntByName( fs, fnode, "complexity" );
    node->alpha = cvReadRealByName( fs, fnode, "alpha" );
    node->node_risk = cvReadRealByName( fs, fnode, "node_risk" );
    node->tree_risk = cvReadRealByName( fs, fnode, "tree_risk" );
    node->tree_error = cvReadRealByName( fs, fnode, "tree_error" );

    splits = cvGetFileNodeByName( fs, fnode, "splits" );
    if( splits )
    {
        CvSeqReader reader;
        CvDTreeSplit* last_split = 0;

        if( CV_NODE_TYPE(splits->tag) != CV_NODE_SEQ )
            CV_ERROR( CV_StsParseError, "splits tag must stored as a sequence" );

        cvStartReadSeq( splits->data.seq, &reader );
        for( i = 0; i < reader.seq->total; i++ )
        {
            CvDTreeSplit* split;
            CV_CALL( split = read_split( fs, (CvFileNode*)reader.ptr ));
            if( !last_split )
                node->split = last_split = split;
            else
                last_split = last_split->next = split;

            CV_NEXT_SEQ_ELEM( reader.seq->elem_size, reader );
        }
    }

    __END__;
    
    return node;
}


void CvDTree::read_tree_nodes( CvFileStorage* fs, CvFileNode* fnode )
{
    CV_FUNCNAME( "CvDTree::read_tree_nodes" );

    __BEGIN__;

    CvSeqReader reader;
    CvDTreeNode _root;
    CvDTreeNode* parent = &_root;
    int i;
    parent->left = parent->right = parent->parent = 0;

    cvStartReadSeq( fnode->data.seq, &reader );

    for( i = 0; i < reader.seq->total; i++ )
    {
        CvDTreeNode* node;
        
        CV_CALL( node = read_node( fs, (CvFileNode*)reader.ptr, parent != &_root ? parent : 0 ));
        if( !parent->left )
            parent->left = node;
        else
            parent->right = node;
        if( node->split )
            parent = node;
        else
        {
            while( parent && parent->right )
                parent = parent->parent;
        }

        CV_NEXT_SEQ_ELEM( reader.seq->elem_size, reader );
    }

    root = _root.left;

    __END__;
}


void CvDTree::read( CvFileStorage* fs, CvFileNode* fnode )
{
    CV_FUNCNAME( "CvDTree::read" );

    __BEGIN__;

    CvFileNode* tree_nodes;

    clear();
    read_train_data_params( fs, fnode );

    tree_nodes = cvGetFileNodeByName( fs, fnode, "nodes" );
    if( !tree_nodes || CV_NODE_TYPE(tree_nodes->tag) != CV_NODE_SEQ )
        CV_ERROR( CV_StsParseError, "nodes tag is missing" );

    pruned_tree_idx = cvReadIntByName( fs, fnode, "best_tree_idx", -1 );

    read_tree_nodes( fs, tree_nodes );
    get_var_importance(); // recompute variable importance

    __END__;
}

/* End of file. */
