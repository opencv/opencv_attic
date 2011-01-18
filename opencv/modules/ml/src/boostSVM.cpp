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

CvBoostSVMParams::CvBoostSVMParams()
{
    //boost parameters
    boost_type = CvBoost::REAL;
    weak_count = 100;
    weight_trim_rate = 0.95;

    //SVM parameters almost all commented because they initialized such in the CvSVMParams class
    //svm_type = CvSVM::C_SVC; //classification
    kernel_type = CvSVM::LINEAR; //linear kernel
    //degree = 0.0; //not used for linear SVM
    //gamma = 0.0; //not used for linear SVM
    //coef0 = 0.0; //not used for linear SVM
    //C = 1.0; //cost coefficient
    //nu = 0.0;
    //p = 0.0;
    //class_weights = NULL;
    //term_crit = cvTermCriteria( CV_TERMCRIT_ITER+CV_TERMCRIT_EPS, 1000, FLT_EPSILON );
}

CvBoostSVMParams::CvBoostSVMParams( int boostType, int weakCount, double weightTrimRate,
                                   double cost, CvMat* classWeights, CvTermCriteria termCrit )
                                   : CvSVMParams(CvSVM::C_SVC, CvSVM::LINEAR, 0, 0, 0, cost, 0, 0, classWeights, termCrit)
{
    //boost parameters
    boost_type = boostType;
    weak_count = weakCount;
    weight_trim_rate = weightTrimRate;

    //SVM parameters
    //svm_type = CvSVM::C_SVC; //classification
    //kernel_type = CvSVM::LINEAR; //linear kernel
    //C = cost; //cost coefficient
    //class_weights = classWeights;
    //term_crit = termCrit;
    //degree = 0.0; //not used for linear SVM
    //gamma = 0.0; //not used for linear SVM
    //coef0 = 0.0; //not used for linear SVM
    //nu = 0.0; // not used for C-classification (C_SVC)
    //p = 0.0;
}

//------------------------------------------CvBoostSVM-------------------------------------------------//
CvBoostSVM::CvBoostSVM()
{
    weak = 0;
    default_model_name = "my_boost_SVM";
    orig_response = sum_response = weak_eval =
        subsample_mask = weights = subtree_weights = 0;
    have_active_cat_vars = have_subsample = false;

    clear();
}


void CvBoostSVM::prune( CvSlice slice )
{
    if( weak )
    {
        CvSeqReader reader;
        int i, count = cvSliceLength( slice, weak );

        cvStartReadSeq( weak, &reader );
        cvSetSeqReaderPos( &reader, slice.start_index );

        for( i = 0; i < count; i++ )
        {
            CvSVM* w;
            CV_READ_SEQ_ELEM( w, reader );
            delete w;
        }

        cvSeqRemoveSlice( weak, slice );
    }
}


void CvBoostSVM::clear()
{
    if( weak )
    {
        prune( CV_WHOLE_SEQ );
        cvReleaseMemStorage( &weak->storage );
    }    
    weak = 0;
    cvReleaseMat( &orig_response );
    cvReleaseMat( &sum_response );
    cvReleaseMat( &weak_eval );
    cvReleaseMat( &subsample_mask );
    cvReleaseMat( &weights );
    cvReleaseMat( &subtree_weights );

    have_subsample = false;
}


CvBoostSVM::~CvBoostSVM()
{
    clear();
}


CvBoostSVM::CvBoostSVM( const CvMat* _train_data, int _tflag, const CvMat* _responses, const CvMat* _var_idx, const CvMat* _sample_idx, const CvMat* _var_type, const CvMat* _missing_mask, CvBoostSVMParams _params )
{
    weak = 0;
    default_model_name = "my_boost_SVM";
    orig_response = sum_response = weak_eval =
        subsample_mask = weights = subtree_weights = 0;

    train( _train_data, _tflag, _responses, _var_idx, _sample_idx,
           _var_type, _missing_mask, _params );
}


bool CvBoostSVM::set_params( const CvBoostSVMParams& _params )
{
    bool ok = false;

    CV_FUNCNAME( "CvBoostSVM::set_params" );

    __BEGIN__;

    params = _params;
    if( params.boost_type != CvBoost::DISCRETE && params.boost_type != CvBoost::REAL &&
        params.boost_type != CvBoost::LOGIT && params.boost_type != CvBoost::GENTLE )
        CV_ERROR( CV_StsBadArg, "Unknown/unsupported boosting type" );

    params.weak_count = MAX( params.weak_count, 1 );
    params.weight_trim_rate = MAX( params.weight_trim_rate, 0. );
    params.weight_trim_rate = MIN( params.weight_trim_rate, 1. );
    if( params.weight_trim_rate < FLT_EPSILON )
        params.weight_trim_rate = 1.f;

    ok = true;

    __END__;

    return ok;
}


bool CvBoostSVM::train( const CvMat* _train_data, int _tflag,
              const CvMat* _responses, const CvMat* _var_idx,
              const CvMat* _sample_idx, const CvMat* _var_type,
              const CvMat* _missing_mask,
              CvBoostSVMParams _params, bool _update )
{
    bool ok = false;
    CvMemStorage* storage = 0;

    CV_FUNCNAME( "CvBoostSVM::train" );

    __BEGIN__;

    int i;
    
    set_params( _params );

    if( !_update )
    {
        clear();

        CV_CALL( storage = cvCreateMemStorage() );
        weak = cvCreateSeq( 0, sizeof(CvSeq), sizeof(CvSVM*), storage );
        storage = 0;
    }
    else
    {
        ;//??????????????????
    }

    if ( (_params.boost_type == CvBoost::LOGIT) || (_params.boost_type == CvBoost::GENTLE) )
    {
        ;
    }
    
    update_weights( 0 );

    for( i = 0; i < params.weak_count; i++ )
    {
        CvSVM* svm = new CvSVM;
        if( !svm->train( _train_data, _responses, _var_idx, _sample_idx, _params ) ) 
        {
            delete svm;
            continue;
        }
        //cvCheckArr( get_weak_response());
        cvSeqPush( weak, &svm );
        update_weights( svm );
        trim_weights();
    }

    ok = true;

    __END__;

    return ok;
}

bool CvBoostSVM::train( CvMLData* _data,
             CvBoostSVMParams params,
             bool update )
{
    bool result = false;

    CV_FUNCNAME( "CvBoostSVM::train" );

    __BEGIN__;

    const CvMat* values = _data->get_values();
    const CvMat* response = _data->get_responses();
    const CvMat* missing = _data->get_missing();
    const CvMat* var_types = _data->get_var_types();
    const CvMat* train_sidx = _data->get_train_sample_idx();
    const CvMat* var_idx = _data->get_var_idx();


    CV_CALL( result = train( values, CV_ROW_SAMPLE, response, var_idx,
        train_sidx, var_types, missing, params, update ) );

    __END__;

    return result;
}


//void CvBoostSVM::update_weights( CvSVM* svm )
//{
//    CV_FUNCNAME( "CvBoostSVM::update_weights" );
//
//    __BEGIN__;
//
//    int i, n = svm->get_sample_count();
//    double sumw = 0.;
//    //int step = 0;
//    //float* fdata = 0;
//    //int *sample_idx_buf;
//    //const int* sample_idx = 0;
//    //cv::AutoBuffer<uchar> inn_buf;
//    //int _buf_size = (params.boost_type == LOGIT) || (params.boost_type == GENTLE) ? sample_count*sizeof(int) : 0;
//    //if( !svm )
//    //    _buf_size += n*sizeof(int);
//    //else
//    //{
//    //    if( have_subsample )
//    //        _buf_size += /*data->buf->step**/(sizeof(float)+sizeof(uchar));
//    //}
//    //inn_buf.allocate(_buf_size);
//    //uchar* cur_buf_pos = (uchar*)inn_buf;
//
//    //if ( (params.boost_type == LOGIT) || (params.boost_type == GENTLE) )
//    //{
//    //    //step = CV_IS_MAT_CONT(data->responses_copy->type) ?
//    //    //    1 : data->responses_copy->step / CV_ELEM_SIZE(data->responses_copy->type);
//    //    //fdata = data->responses_copy->data.fl;
//    //    //sample_idx_buf = (int*)cur_buf_pos;
//    //    //cur_buf_pos = (uchar*)(sample_idx_buf + data->sample_count);
//    //    //sample_idx = data->get_sample_indices( data->data_root, sample_idx_buf );
//    //}
//    ////CvMat* dtree_data_buf = data->buf;
//    if( !svm ) // before training the first tree, initialize weights and other parameters
//    {
//        //int* class_labels_buf = (int*)cur_buf_pos;
//        //cur_buf_pos = (uchar*)(class_labels_buf + n);
//        //const int* class_labels = data->get_class_labels(data->data_root, class_labels_buf);
//        // in case of logitboost and gentle adaboost each weak tree is a regression tree,
//        // so we need to convert class labels to floating-point values
//
//        double w0 = 1./n;
//        double p[2] = { 1, 1 };
//
//        cvReleaseMat( &orig_response );
//        cvReleaseMat( &sum_response );
//        cvReleaseMat( &weak_eval );
//        cvReleaseMat( &subsample_mask );
//        cvReleaseMat( &weights );
//        cvReleaseMat( &subtree_weights );
//
//        CV_CALL( orig_response = cvCreateMat( 1, n, CV_32S ));
//        CV_CALL( weak_eval = cvCreateMat( 1, n, CV_64F ));
//        CV_CALL( subsample_mask = cvCreateMat( 1, n, CV_8U ));
//        CV_CALL( weights = cvCreateMat( 1, n, CV_64F ));
//        CV_CALL( subtree_weights = cvCreateMat( 1, n + 2, CV_64F ));
//
//        //if( data->have_priors )
//        //{
//        //    // compute weight scale for each class from their prior probabilities
//        //    int c1 = 0;
//        //    for( i = 0; i < n; i++ )
//        //        c1 += class_labels[i];
//        //    p[0] = data->priors->data.db[0]*(c1 < n ? 1./(n - c1) : 0.);
//        //    p[1] = data->priors->data.db[1]*(c1 > 0 ? 1./c1 : 0.);
//        //    p[0] /= p[0] + p[1];
//        //    p[1] = 1. - p[0];
//        //}
//
//        //if (data->is_buf_16u)
//        {
//            unsigned short* labels = (unsigned short*)(dtree_data_buf->data.s + data->data_root->buf_idx*dtree_data_buf->cols +
//                data->data_root->offset + (data->work_var_count-1)*data->sample_count);
//            for( i = 0; i < n; i++ )
//            {
//                // save original categorical responses {0,1}, convert them to {-1,1}
//                orig_response->data.i[i] = class_labels[i]*2 - 1;
//                // make all the samples active at start.
//                // later, in trim_weights() deactivate/reactive again some, if need
//                subsample_mask->data.ptr[i] = (uchar)1;
//                // make all the initial weights the same.
//                weights->data.db[i] = w0*p[class_labels[i]];
//                // set the labels to find (from within weak tree learning proc)
//                // the particular sample weight, and where to store the response.
//                labels[i] = (unsigned short)i;
//            }
//        }
//        else
//        {
//            int* labels = dtree_data_buf->data.i + data->data_root->buf_idx*dtree_data_buf->cols +
//                data->data_root->offset + (data->work_var_count-1)*data->sample_count;
//
//            for( i = 0; i < n; i++ )
//            {
//                // save original categorical responses {0,1}, convert them to {-1,1}
//                orig_response->data.i[i] = class_labels[i]*2 - 1;
//                // make all the samples active at start.
//                // later, in trim_weights() deactivate/reactive again some, if need
//                subsample_mask->data.ptr[i] = (uchar)1;
//                // make all the initial weights the same.
//                weights->data.db[i] = w0*p[class_labels[i]];
//                // set the labels to find (from within weak tree learning proc)
//                // the particular sample weight, and where to store the response.
//                labels[i] = i;
//            }
//        }
//
//        if( params.boost_type == LOGIT )
//        {
//            CV_CALL( sum_response = cvCreateMat( 1, n, CV_64F ));
//
//            for( i = 0; i < n; i++ )
//            {
//                sum_response->data.db[i] = 0;
//                fdata[sample_idx[i]*step] = orig_response->data.i[i] > 0 ? 2.f : -2.f;
//            }
//
//            // in case of logitboost each weak tree is a regression tree.
//            // the target function values are recalculated for each of the trees
//            data->is_classifier = false;
//        }
//        else if( params.boost_type == GENTLE )
//        {
//            for( i = 0; i < n; i++ )
//                fdata[sample_idx[i]*step] = (float)orig_response->data.i[i];
//
//            data->is_classifier = false;
//        }
//    }
//    else
//    {
//        // at this moment, for all the samples that participated in the training of the most
//        // recent weak classifier we know the responses. For other samples we need to compute them
//        if( have_subsample )
//        {
//            float* values0, *values = (float*)cur_buf_pos;
//            cur_buf_pos = (uchar*)(values + data->buf->step);
//            uchar* missing0, *missing = cur_buf_pos;
//            cur_buf_pos = missing + data->buf->step;
//            CvMat _sample, _mask;
//            values0 = values;
//            missing0 = missing;
//
//            // invert the subsample mask
//            cvXorS( subsample_mask, cvScalar(1.), subsample_mask );
//            data->get_vectors( subsample_mask, values, missing, 0 );
//           
//            _sample = cvMat( 1, data->var_count, CV_32F );
//            _mask = cvMat( 1, data->var_count, CV_8U );
//
//            // run tree through all the non-processed samples
//            for( i = 0; i < n; i++ )
//                if( subsample_mask->data.ptr[i] )
//                {
//                    _sample.data.fl = values;
//                    _mask.data.ptr = missing;
//                    values += _sample.cols;
//                    missing += _mask.cols;
//                    weak_eval->data.db[i] = tree->predict( &_sample, &_mask, true )->value;
//                }
//        }
//
//        // now update weights and other parameters for each type of boosting
//        if( params.boost_type == DISCRETE )
//        {
//            // Discrete AdaBoost:
//            //   weak_eval[i] (=f(x_i)) is in {-1,1}
//            //   err = sum(w_i*(f(x_i) != y_i))/sum(w_i)
//            //   C = log((1-err)/err)
//            //   w_i *= exp(C*(f(x_i) != y_i))
//
//            double C, err = 0.;
//            double scale[] = { 1., 0. };
//
//            for( i = 0; i < n; i++ )
//            {
//                double w = weights->data.db[i];
//                sumw += w;
//                err += w*(weak_eval->data.db[i] != orig_response->data.i[i]);
//            }
//
//            if( sumw != 0 )
//                err /= sumw;
//            C = err = -log_ratio( err );
//            scale[1] = exp(err);
//
//            sumw = 0;
//            for( i = 0; i < n; i++ )
//            {
//                double w = weights->data.db[i]*
//                    scale[weak_eval->data.db[i] != orig_response->data.i[i]];
//                sumw += w;
//                weights->data.db[i] = w;
//            }
//
//            tree->scale( C );
//        }
//        else if( params.boost_type == REAL )
//        {
//            // Real AdaBoost:
//            //   weak_eval[i] = f(x_i) = 0.5*log(p(x_i)/(1-p(x_i))), p(x_i)=P(y=1|x_i)
//            //   w_i *= exp(-y_i*f(x_i))
//
//            for( i = 0; i < n; i++ )
//                weak_eval->data.db[i] *= -orig_response->data.i[i];
//
//            cvExp( weak_eval, weak_eval );
//
//            for( i = 0; i < n; i++ )
//            {
//                double w = weights->data.db[i]*weak_eval->data.db[i];
//                sumw += w;
//                weights->data.db[i] = w;
//            }
//        }
//        else if( params.boost_type == LOGIT )
//        {
//            // LogitBoost:
//            //   weak_eval[i] = f(x_i) in [-z_max,z_max]
//            //   sum_response = F(x_i).
//            //   F(x_i) += 0.5*f(x_i)
//            //   p(x_i) = exp(F(x_i))/(exp(F(x_i)) + exp(-F(x_i))=1/(1+exp(-2*F(x_i)))
//            //   reuse weak_eval: weak_eval[i] <- p(x_i)
//            //   w_i = p(x_i)*1(1 - p(x_i))
//            //   z_i = ((y_i+1)/2 - p(x_i))/(p(x_i)*(1 - p(x_i)))
//            //   store z_i to the data->data_root as the new target responses
//
//            const double lb_weight_thresh = FLT_EPSILON;
//            const double lb_z_max = 10.;
//            /*float* responses_buf = data->get_resp_float_buf();
//            const float* responses = 0;
//            data->get_ord_responses(data->data_root, responses_buf, &responses);*/
//
//            /*if( weak->total == 7 )
//                putchar('*');*/
//
//            for( i = 0; i < n; i++ )
//            {
//                double s = sum_response->data.db[i] + 0.5*weak_eval->data.db[i];
//                sum_response->data.db[i] = s;
//                weak_eval->data.db[i] = -2*s;
//            }
//
//            cvExp( weak_eval, weak_eval );
//
//            for( i = 0; i < n; i++ )
//            {
//                double p = 1./(1. + weak_eval->data.db[i]);
//                double w = p*(1 - p), z;
//                w = MAX( w, lb_weight_thresh );
//                weights->data.db[i] = w;
//                sumw += w;
//                if( orig_response->data.i[i] > 0 )
//                {
//                    z = 1./p;
//                    fdata[sample_idx[i]*step] = (float)MIN(z, lb_z_max);
//                }
//                else
//                {
//                    z = 1./(1-p);
//                    fdata[sample_idx[i]*step] = (float)-MIN(z, lb_z_max);
//                }
//            }
//        }
//        else
//        {
//            // Gentle AdaBoost:
//            //   weak_eval[i] = f(x_i) in [-1,1]
//            //   w_i *= exp(-y_i*f(x_i))
//            assert( params.boost_type == GENTLE );
//
//            for( i = 0; i < n; i++ )
//                weak_eval->data.db[i] *= -orig_response->data.i[i];
//
//            cvExp( weak_eval, weak_eval );
//
//            for( i = 0; i < n; i++ )
//            {
//                double w = weights->data.db[i] * weak_eval->data.db[i];
//                weights->data.db[i] = w;
//                sumw += w;
//            }
//        }
//    }
//
//    // renormalize weights
//    if( sumw > FLT_EPSILON )
//    {
//        sumw = 1./sumw;
//        for( i = 0; i < n; ++i )
//            weights->data.db[i] *= sumw;
//    }
//
//    __END__;
//}