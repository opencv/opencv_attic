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

//------------------------------------------CvVecBoost-------------------------------------------------//
CvVecBoost::CvVecBoost(const CvMat* _train_data, int _tflag,
                       const CvMat* _responses, const CvMat* _var_idx,
                       const CvMat* _sample_idx, const CvMat* _var_type,
                       const CvMat* _missing_mask, CvBoostParams _params )
{
    set_data();
    default_model_name = "my_vec_boost_tree";

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

    int i, featIdx;
    int feat_count;
    float weak_error, weak_quality;
    
    set_params( _params );

    cvReleaseMat( &active_vars );
    cvReleaseMat( &active_vars_abs );

    if( !_update || !data )
    {
        clear();
        data = new CvDTreeTrainData( _train_data, _tflag, _responses, _var_idx,
            _sample_idx, _var_type, _missing_mask, _params, true, true );

        if( data->get_num_classes() != 2 )
            CV_ERROR( CV_StsNotImplemented,
            "Boosted trees can only be used for 2-class classification." );
        CV_CALL( storage = cvCreateMemStorage() );
        weak = cvCreateSeq( 0, sizeof(CvSeq), sizeof(CvBoostTree*), storage );
        storage = 0;
    }
    else
    {
        data->set_data( _train_data, _tflag, _responses, _var_idx,
            _sample_idx, _var_type, _missing_mask, _params, true, true, true );
    }

    if ( (_params.boost_type == LOGIT) || (_params.boost_type == GENTLE) )
        data->do_responses_copy();
    
    update_weights( 0 );

    //CV_Assert( data->var_all % params.vector_size ) ;
    //feat_count = data->var_all / params.vector_size;
    for( i = 0; i < params.weak_count; i++ )
    {
        CvBoostTree **boostTree;
        boostTree = new CvBoostTree*[feat_count];
        for( featIdx = 0; featIdx < feat_count; i++ )
        {
            boostTree[i] = new CvBoostTree;
            if( !boostTree[i]->train(data, subsample_mask,/* var_idx,*/this) )
            {
                continue;
            }
            //weak_error = boostTree[i]->calc_error( _train_data, CV_TRAIN_ERROR, _responses );
        }

        cvSeqPush( weak, &boostTree[i] );
        update_weights( boostTree[i] );
        trim_weights();
    }

    get_active_vars(); // recompute active_vars* maps and condensed_idx's in the splits.
    data->is_classifier = true;
    ok = true;

    data->free_train_data();

    __END__;

    return ok;
}



using namespace cv;

CvVecBoost::CvVecBoost( const Mat& _train_data, int _tflag,
                       const Mat& _responses, const Mat& _var_idx,
                       const Mat& _sample_idx, const Mat& _var_type,
                       const Mat& _missing_mask,
                       CvBoostParams _params )
{
    set_data();
    default_model_name = "my_vec_boost_tree";
 
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


