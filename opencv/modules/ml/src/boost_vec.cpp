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

#include "precomp.hpp"

//CvBoostParams::CvBoostParams() : CvBoostParams()
//{
//    vector_size = 1; //scalar features
//}
//
//CvBoostParams::CvBoostParams(int boost_type, int weak_count, 
//                                   double weight_trim_rate, int max_depth, 
//                                   bool use_surrogates, const float *priors, int vec_size) 
//                                   : CvBoostParams(boost_type, weak_count, 
//                                   weight_trim_rate, max_depth, use_surrogates, priors)
//{
//    vector_size = vec_size;
//}

static inline double
log_ratio( double val )
{
    const double eps = 1e-5;

    val = MAX( val, eps );
    val = MIN( val, 1. - eps );
    return log( val/(1. - val) );
}

//------------------------------------------CvVecBoost-------------------------------------------------//
void CvVecBoost::set_data()
{
    CvBoost::set_data();
    data_all = 0;
}

void CvVecBoost::clear()
{
    CvBoost::clear();
    if (data_all)
    {
        int feat_count = data_all[0] ? data_all[0]->var_all / params.feat_size : 0;
        for( int i = 0; i < feat_count; i++ )
        {
            if (data_all[i])
                delete data_all[i];
            data_all[i] = 0;
        }
        delete data_all;
    }
    data_all = 0;
}

CvVecBoost::~CvVecBoost()
{
    clear();
}

CvVecBoost::CvVecBoost(const CvMat* _train_data, int _tflag,
                       const CvMat* _responses, const CvMat* _var_idx,
                       const CvMat* _sample_idx, const CvMat* _var_type,
                       const CvMat* _missing_mask, CvBoostParams _params )
{
    default_model_name = "my_vec_boost_tree";
    data_all = 0;

    train(_train_data, _tflag, _responses, _var_idx,
                       _sample_idx, _var_type, _missing_mask, _params);
}


bool CvVecBoost::train(const CvMat* _train_data, int _tflag, 
                       const CvMat* _responses, const CvMat* _var_idx,
                       const CvMat* _sample_idx, const CvMat* _var_type,
                       const CvMat* _missing_mask, CvBoostParams _params, bool _update )
{
    bool ok = false;
    CvMemStorage* storage = 0;

    CV_FUNCNAME( "CvVecBoost::train" );

    __BEGIN__;

    int i, j, featIdx;
    int feat_count, bestTreeIdx = -1;
    float weak_quality, best_weak_quality = FLT_MIN;
    CvMat* varIdx;
    
    set_params( _params );

    varIdx = cvCreateMat(1, params.feat_size, CV_32S);

    ////////////////////////////////////////////////////////////////////
#if 0 
    data = new CvDTreeTrainData( _train_data, _tflag, _responses, _var_idx,
        _sample_idx, _var_type, _missing_mask, _params, true, true );
    CvBoostTree *tstTree = new CvBoostTree;
    update_weights( 0 );
    tstTree->train(data, subsample_mask, this);
#endif

#if 0
    for( j = 0; j < params.feat_size; j++)
    {
        varIdx->data.i[j] = 0 * params.feat_size + j;
    }
    data = new CvDTreeTrainData( _train_data, _tflag, _responses, varIdx,
        _sample_idx, _var_type, _missing_mask, _params, true, true );
    CvBoostTree *testTree = new CvBoostTree;
    update_weights( 0 );
    testTree->train(data, subsample_mask, this);
#endif
    /////////////////////////////////////////////////////////////////////

    data = new CvDTreeTrainData( _train_data, _tflag, _responses, _var_idx,
        _sample_idx, _var_type, _missing_mask, _params, true, true );


    if( data->get_num_classes() != 2 )
        CV_ERROR( CV_StsNotImplemented, "Boosted trees can only be used for 2-class classification." );

    CV_Assert( !(data->var_all % params.feat_size) );
    feat_count = data->var_all / params.feat_size;

    CV_CALL( storage = cvCreateMemStorage() );
    weak = cvCreateSeq( 0, sizeof(CvSeq), sizeof(CvBoostTree*), storage );
    storage = 0;     

    if ( (_params.boost_type == LOGIT) || (_params.boost_type == GENTLE) )
        data->do_responses_copy();

    data_all = new CvDTreeTrainData*[feat_count];      
    for( featIdx = 0; featIdx < feat_count; featIdx++ )
    {
        for( j = 0; j < params.feat_size; j++)
        {
            varIdx->data.i[j] = featIdx * params.feat_size + j;
        }
        data_all[featIdx] = new CvDTreeTrainData( _train_data, _tflag, _responses, 
            varIdx, _sample_idx, _var_type, _missing_mask,_params, true, true );
    }

    if ( (_params.boost_type == LOGIT) || (_params.boost_type == GENTLE) )
    {
        for( featIdx = 0; featIdx < feat_count; featIdx++ )
        {
            data_all[featIdx]->do_responses_copy();
        }
    }

    for( featIdx = 0; featIdx < feat_count; featIdx++ )
    {
        init_data(featIdx);
    }

    for( i = 0; i < params.weak_count; i++ )
    {
        CvBoostTree **boostTree;
        boostTree = new CvBoostTree*[feat_count];
        for( featIdx = 0; featIdx < feat_count; featIdx++ )
        {
            boostTree[featIdx] = new CvBoostTree;
            if( !boostTree[featIdx]->train(data_all[featIdx], subsample_mask, this) )
            {
                continue;
            }
            weak_quality = boostTree[featIdx]->get_tree_quality();
            if( weak_quality > best_weak_quality )
            {
                best_weak_quality = weak_quality;
                bestTreeIdx = featIdx;
            }
        }
        cvSeqPush( weak, &boostTree[bestTreeIdx] );
        CvBoost::update_weights( boostTree[bestTreeIdx] );
        trim_weights();
    }
    cvReleaseMat( &varIdx );

    get_active_vars(); // recompute active_vars* maps and condensed_idx's in the splits.
    data->is_classifier = true;
    ok = true;

    data->free_train_data();
    for( featIdx = 0; featIdx < feat_count; featIdx++)
        data_all[featIdx]->free_train_data();

    __END__;

    return ok;
}

void CvVecBoost::update_weights(CvBoostTree *tree, int treeIdx)
{
    CV_FUNCNAME( "CvVecBoost::update_weights" );

    __BEGIN__;

    int i, n = data_all[treeIdx]->sample_count;
    double sumw = 0.;
    int step = 0;
    float* fdata = 0;
    int *sample_idx_buf;
    const int* sample_idx = 0;
    cv::AutoBuffer<uchar> inn_buf;
    int _buf_size = (params.boost_type == LOGIT) || (params.boost_type == GENTLE) ? data_all[treeIdx]->sample_count*sizeof(int) : 0;
    if( !tree )
        _buf_size += n*sizeof(int);
    else
    {
        if( have_subsample )
            _buf_size += data_all[treeIdx]->buf->step*(sizeof(float)+sizeof(uchar));
    }
    inn_buf.allocate(_buf_size);
    uchar* cur_buf_pos = (uchar*)inn_buf;

    if ( (params.boost_type == LOGIT) || (params.boost_type == GENTLE) )
    {
        step = CV_IS_MAT_CONT(data_all[treeIdx]->responses_copy->type) ?
            1 : data_all[treeIdx]->responses_copy->step / CV_ELEM_SIZE(data_all[treeIdx]->responses_copy->type);
        fdata = data_all[treeIdx]->responses_copy->data.fl;
        sample_idx_buf = (int*)cur_buf_pos;
        cur_buf_pos = (uchar*)(sample_idx_buf + data_all[treeIdx]->sample_count);
        sample_idx = data_all[treeIdx]->get_sample_indices( data_all[treeIdx]->data_root, sample_idx_buf );
    }
    CvMat* dtree_data_buf = data_all[treeIdx]->buf;
    if( !tree ) // before training the first tree, initialize weights and other parameters
    {
        int* class_labels_buf = (int*)cur_buf_pos;
        cur_buf_pos = (uchar*)(class_labels_buf + n);
        const int* class_labels = data_all[treeIdx]->get_class_labels(data_all[treeIdx]->data_root, class_labels_buf);
        // in case of logitboost and gentle adaboost each weak tree is a regression tree,
        // so we need to convert class labels to floating-point values

        double w0 = 1./n;
        double p[2] = { 1, 1 };

        cvReleaseMat( &orig_response );
        cvReleaseMat( &sum_response );
        cvReleaseMat( &weak_eval );
        cvReleaseMat( &subsample_mask );
        cvReleaseMat( &weights );
        cvReleaseMat( &subtree_weights );

        CV_CALL( orig_response = cvCreateMat( 1, n, CV_32S ));
        CV_CALL( weak_eval = cvCreateMat( 1, n, CV_64F ));
        CV_CALL( subsample_mask = cvCreateMat( 1, n, CV_8U ));
        CV_CALL( weights = cvCreateMat( 1, n, CV_64F ));
        CV_CALL( subtree_weights = cvCreateMat( 1, n + 2, CV_64F ));

        if( data_all[treeIdx]->have_priors )
        {
            // compute weight scale for each class from their prior probabilities
            int c1 = 0;
            for( i = 0; i < n; i++ )
                c1 += class_labels[i];
            p[0] = data_all[treeIdx]->priors->data.db[0]*(c1 < n ? 1./(n - c1) : 0.);
            p[1] = data_all[treeIdx]->priors->data.db[1]*(c1 > 0 ? 1./c1 : 0.);
            p[0] /= p[0] + p[1];
            p[1] = 1. - p[0];
        }

        if (data_all[treeIdx]->is_buf_16u)
        {
            unsigned short* labels = (unsigned short*)(dtree_data_buf->data.s + data_all[treeIdx]->data_root->buf_idx*dtree_data_buf->cols +
                data_all[treeIdx]->data_root->offset + (data_all[treeIdx]->work_var_count-1)*data_all[treeIdx]->sample_count);
            for( i = 0; i < n; i++ )
            {
                // save original categorical responses {0,1}, convert them to {-1,1}
                orig_response->data.i[i] = class_labels[i]*2 - 1;
                // make all the samples active at start.
                // later, in trim_weights() deactivate/reactive again some, if need
                subsample_mask->data.ptr[i] = (uchar)1;
                // make all the initial weights the same.
                weights->data.db[i] = w0*p[class_labels[i]];
                // set the labels to find (from within weak tree learning proc)
                // the particular sample weight, and where to store the response.
                labels[i] = (unsigned short)i;
            }
        }
        else
        {
            int* labels = dtree_data_buf->data.i + data_all[treeIdx]->data_root->buf_idx*dtree_data_buf->cols +
                data_all[treeIdx]->data_root->offset + (data_all[treeIdx]->work_var_count-1)*data_all[treeIdx]->sample_count;

            for( i = 0; i < n; i++ )
            {
                // save original categorical responses {0,1}, convert them to {-1,1}
                orig_response->data.i[i] = class_labels[i]*2 - 1;
                // make all the samples active at start.
                // later, in trim_weights() deactivate/reactive again some, if need
                subsample_mask->data.ptr[i] = (uchar)1;
                // make all the initial weights the same.
                weights->data.db[i] = w0*p[class_labels[i]];
                // set the labels to find (from within weak tree learning proc)
                // the particular sample weight, and where to store the response.
                labels[i] = i;
            }
        }

        if( params.boost_type == LOGIT )
        {
            CV_CALL( sum_response = cvCreateMat( 1, n, CV_64F ));

            for( i = 0; i < n; i++ )
            {
                sum_response->data.db[i] = 0;
                fdata[sample_idx[i]*step] = orig_response->data.i[i] > 0 ? 2.f : -2.f;
            }

            // in case of logitboost each weak tree is a regression tree.
            // the target function values are recalculated for each of the trees
            data_all[treeIdx]->is_classifier = false;
        }
        else if( params.boost_type == GENTLE )
        {
            for( i = 0; i < n; i++ )
                fdata[sample_idx[i]*step] = (float)orig_response->data.i[i];

            data_all[treeIdx]->is_classifier = false;
        }
    }
    else
    {
        // at this moment, for all the samples that participated in the training of the most
        // recent weak classifier we know the responses. For other samples we need to compute them
        if( have_subsample )
        {
            float* values0, *values = (float*)cur_buf_pos;
            cur_buf_pos = (uchar*)(values + data_all[treeIdx]->buf->step);
            uchar* missing0, *missing = cur_buf_pos;
            cur_buf_pos = missing + data_all[treeIdx]->buf->step;
            CvMat _sample, _mask;
            values0 = values;
            missing0 = missing;

            // invert the subsample mask
            cvXorS( subsample_mask, cvScalar(1.), subsample_mask );
            data_all[treeIdx]->get_vectors( subsample_mask, values, missing, 0 );
           
            _sample = cvMat( 1, data_all[treeIdx]->var_count, CV_32F );
            _mask = cvMat( 1, data_all[treeIdx]->var_count, CV_8U );

            // run tree through all the non-processed samples
            for( i = 0; i < n; i++ )
                if( subsample_mask->data.ptr[i] )
                {
                    _sample.data.fl = values;
                    _mask.data.ptr = missing;
                    values += _sample.cols;
                    missing += _mask.cols;
                    weak_eval->data.db[i] = tree->predict( &_sample, &_mask, true )->value;
                }
        }

        // now update weights and other parameters for each type of boosting
        if( params.boost_type == DISCRETE )
        {
            // Discrete AdaBoost:
            //   weak_eval[i] (=f(x_i)) is in {-1,1}
            //   err = sum(w_i*(f(x_i) != y_i))/sum(w_i)
            //   C = log((1-err)/err)
            //   w_i *= exp(C*(f(x_i) != y_i))

            double C, err = 0.;
            double scale[] = { 1., 0. };

            for( i = 0; i < n; i++ )
            {
                double w = weights->data.db[i];
                sumw += w;
                err += w*(weak_eval->data.db[i] != orig_response->data.i[i]);
            }

            if( sumw != 0 )
                err /= sumw;
            C = err = -log_ratio( err );
            scale[1] = exp(err);

            sumw = 0;
            for( i = 0; i < n; i++ )
            {
                double w = weights->data.db[i]*
                    scale[weak_eval->data.db[i] != orig_response->data.i[i]];
                sumw += w;
                weights->data.db[i] = w;
            }

            tree->scale( C );
        }
        else if( params.boost_type == REAL )
        {
            // Real AdaBoost:
            //   weak_eval[i] = f(x_i) = 0.5*log(p(x_i)/(1-p(x_i))), p(x_i)=P(y=1|x_i)
            //   w_i *= exp(-y_i*f(x_i))

            for( i = 0; i < n; i++ )
                weak_eval->data.db[i] *= -orig_response->data.i[i];

            cvExp( weak_eval, weak_eval );

            for( i = 0; i < n; i++ )
            {
                double w = weights->data.db[i]*weak_eval->data.db[i];
                sumw += w;
                weights->data.db[i] = w;
            }
        }
        else if( params.boost_type == LOGIT )
        {
            // LogitBoost:
            //   weak_eval[i] = f(x_i) in [-z_max,z_max]
            //   sum_response = F(x_i).
            //   F(x_i) += 0.5*f(x_i)
            //   p(x_i) = exp(F(x_i))/(exp(F(x_i)) + exp(-F(x_i))=1/(1+exp(-2*F(x_i)))
            //   reuse weak_eval: weak_eval[i] <- p(x_i)
            //   w_i = p(x_i)*1(1 - p(x_i))
            //   z_i = ((y_i+1)/2 - p(x_i))/(p(x_i)*(1 - p(x_i)))
            //   store z_i to the data_all[treeIdx]->data_root as the new target responses

            const double lb_weight_thresh = FLT_EPSILON;
            const double lb_z_max = 10.;
            /*float* responses_buf = data_all[treeIdx]->get_resp_float_buf();
            const float* responses = 0;
            data_all[treeIdx]->get_ord_responses(data_all[treeIdx]->data_root, responses_buf, &responses);*/

            /*if( weak->total == 7 )
                putchar('*');*/

            for( i = 0; i < n; i++ )
            {
                double s = sum_response->data.db[i] + 0.5*weak_eval->data.db[i];
                sum_response->data.db[i] = s;
                weak_eval->data.db[i] = -2*s;
            }

            cvExp( weak_eval, weak_eval );

            for( i = 0; i < n; i++ )
            {
                double p = 1./(1. + weak_eval->data.db[i]);
                double w = p*(1 - p), z;
                w = MAX( w, lb_weight_thresh );
                weights->data.db[i] = w;
                sumw += w;
                if( orig_response->data.i[i] > 0 )
                {
                    z = 1./p;
                    fdata[sample_idx[i]*step] = (float)MIN(z, lb_z_max);
                }
                else
                {
                    z = 1./(1-p);
                    fdata[sample_idx[i]*step] = (float)-MIN(z, lb_z_max);
                }
            }
        }
        else
        {
            // Gentle AdaBoost:
            //   weak_eval[i] = f(x_i) in [-1,1]
            //   w_i *= exp(-y_i*f(x_i))
            assert( params.boost_type == GENTLE );

            for( i = 0; i < n; i++ )
                weak_eval->data.db[i] *= -orig_response->data.i[i];

            cvExp( weak_eval, weak_eval );

            for( i = 0; i < n; i++ )
            {
                double w = weights->data.db[i] * weak_eval->data.db[i];
                weights->data.db[i] = w;
                sumw += w;
            }
        }
    }

    // renormalize weights
    if( sumw > FLT_EPSILON )
    {
        sumw = 1./sumw;
        for( i = 0; i < n; ++i )
            weights->data.db[i] *= sumw;
    }

    __END__;
}

void CvVecBoost::init_data(int treeIdx)
{
    CV_FUNCNAME( "CvVecBoost::init_data" );

    __BEGIN__;

    int i, n = data_all[treeIdx]->sample_count;
    int step = 0;
    float* fdata = 0;
    int *sample_idx_buf;
    const int* sample_idx = 0;
    cv::AutoBuffer<uchar> inn_buf;
    int _buf_size = (params.boost_type == LOGIT) || (params.boost_type == GENTLE) ? data_all[treeIdx]->sample_count*sizeof(int) : 0;
    _buf_size += n*sizeof(int);

    inn_buf.allocate(_buf_size);
    uchar* cur_buf_pos = (uchar*)inn_buf;

    if ( (params.boost_type == LOGIT) || (params.boost_type == GENTLE) )
    {
        step = CV_IS_MAT_CONT(data_all[treeIdx]->responses_copy->type) ?
            1 : data_all[treeIdx]->responses_copy->step / CV_ELEM_SIZE(data_all[treeIdx]->responses_copy->type);
        fdata = data_all[treeIdx]->responses_copy->data.fl;
        sample_idx_buf = (int*)cur_buf_pos;
        cur_buf_pos = (uchar*)(sample_idx_buf + data_all[treeIdx]->sample_count);
        sample_idx = data_all[treeIdx]->get_sample_indices( data_all[treeIdx]->data_root, sample_idx_buf );
    }
    CvMat* dtree_data_buf = data_all[treeIdx]->buf;

    // before training the first tree, initialize weights and other parameters
    int* class_labels_buf = (int*)cur_buf_pos;
    cur_buf_pos = (uchar*)(class_labels_buf + n);
    const int* class_labels = data_all[treeIdx]->get_class_labels(data_all[treeIdx]->data_root, class_labels_buf);
    // in case of logitboost and gentle adaboost each weak tree is a regression tree,
    // so we need to convert class labels to floating-point values

    double w0 = 1./n;
    double p[2] = { 1, 1 };

    cvReleaseMat( &orig_response );
    cvReleaseMat( &sum_response );
    cvReleaseMat( &weak_eval );
    cvReleaseMat( &subsample_mask );
    cvReleaseMat( &weights );
    cvReleaseMat( &subtree_weights );

    CV_CALL( orig_response = cvCreateMat( 1, n, CV_32S ));
    CV_CALL( weak_eval = cvCreateMat( 1, n, CV_64F ));
    CV_CALL( subsample_mask = cvCreateMat( 1, n, CV_8U ));
    CV_CALL( weights = cvCreateMat( 1, n, CV_64F ));
    CV_CALL( subtree_weights = cvCreateMat( 1, n + 2, CV_64F ));

    if( data_all[treeIdx]->have_priors )
    {
        // compute weight scale for each class from their prior probabilities
        int c1 = 0;
        for( i = 0; i < n; i++ )
            c1 += class_labels[i];
        p[0] = data_all[treeIdx]->priors->data.db[0]*(c1 < n ? 1./(n - c1) : 0.);
        p[1] = data_all[treeIdx]->priors->data.db[1]*(c1 > 0 ? 1./c1 : 0.);
        p[0] /= p[0] + p[1];
        p[1] = 1. - p[0];
    }

    if (data_all[treeIdx]->is_buf_16u)
    {
        unsigned short* labels = (unsigned short*)(dtree_data_buf->data.s + data_all[treeIdx]->data_root->buf_idx*dtree_data_buf->cols +
            data_all[treeIdx]->data_root->offset + (data_all[treeIdx]->work_var_count-1)*data_all[treeIdx]->sample_count);
        for( i = 0; i < n; i++ )
        {
            // save original categorical responses {0,1}, convert them to {-1,1}
            orig_response->data.i[i] = class_labels[i]*2 - 1;
            // make all the samples active at start.
            // later, in trim_weights() deactivate/reactive again some, if need
            subsample_mask->data.ptr[i] = (uchar)1;
            // make all the initial weights the same.
            weights->data.db[i] = w0*p[class_labels[i]];
            // set the labels to find (from within weak tree learning proc)
            // the particular sample weight, and where to store the response.
            labels[i] = (unsigned short)i;
        }
    }
    else
    {
        int* labels = dtree_data_buf->data.i + data_all[treeIdx]->data_root->buf_idx*dtree_data_buf->cols +
            data_all[treeIdx]->data_root->offset + (data_all[treeIdx]->work_var_count-1)*data_all[treeIdx]->sample_count;

        for( i = 0; i < n; i++ )
        {
            // save original categorical responses {0,1}, convert them to {-1,1}
            orig_response->data.i[i] = class_labels[i]*2 - 1;
            // make all the samples active at start.
            // later, in trim_weights() deactivate/reactive again some, if need
            subsample_mask->data.ptr[i] = (uchar)1;
            // make all the initial weights the same.
            weights->data.db[i] = w0*p[class_labels[i]];
            // set the labels to find (from within weak tree learning proc)
            // the particular sample weight, and where to store the response.
            labels[i] = i;
        }
    }

    if( params.boost_type == LOGIT )
    {
        CV_CALL( sum_response = cvCreateMat( 1, n, CV_64F ));

        for( i = 0; i < n; i++ )
        {
            sum_response->data.db[i] = 0;
            fdata[sample_idx[i]*step] = orig_response->data.i[i] > 0 ? 2.f : -2.f;
        }

        // in case of logitboost each weak tree is a regression tree.
        // the target function values are recalculated for each of the trees
        data_all[treeIdx]->is_classifier = false;
    }
    else if( params.boost_type == GENTLE )
    {
        for( i = 0; i < n; i++ )
            fdata[sample_idx[i]*step] = (float)orig_response->data.i[i];

        data_all[treeIdx]->is_classifier = false;
    }

    __END__;
}



using namespace cv;

CvVecBoost::CvVecBoost( const Mat& _train_data, int _tflag,
                       const Mat& _responses, const Mat& _var_idx,
                       const Mat& _sample_idx, const Mat& _var_type,
                       const Mat& _missing_mask,
                       CvBoostParams _params )
{
    default_model_name = "my_vec_boost_tree";
    data_all = 0;
 
    train( _train_data, _tflag, _responses, _var_idx, _sample_idx,
          _var_type, _missing_mask, _params );
}    

bool CvVecBoost::train( const Mat& _train_data, int _tflag,
                       const Mat& _responses, const Mat& _var_idx,
                       const Mat& _sample_idx, const Mat& _var_type,
                       const Mat& _missing_mask,
                       CvBoostParams _params, bool _update )
{
    CvMat tdata = _train_data, responses = _responses, vidx = _var_idx,
        sidx = _sample_idx, vtype = _var_type, mmask = _missing_mask;
    return train(&tdata, _tflag, &responses, vidx.data.ptr ? &vidx : 0,
          sidx.data.ptr ? &sidx : 0, vtype.data.ptr ? &vtype : 0,
          mmask.data.ptr ? &mmask : 0, _params, _update);
}


