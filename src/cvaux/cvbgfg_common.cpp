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

#include "_cvaux.h"

//  Function cvCreateFGDStatModel initializes foreground detection process
// parameters:
//      first_frame - frame from video sequence
//      parameters  - (optional) if NULL default parameters of the algorithm will be used
//      p_model     - pointer to CvFGDStatModel structure
int  cvCreateFGDStatModel( IplImage*              first_frame,
                           CvFGDStatModelParams*  parameters,
                           CvFGDStatModel*        p_model );

//  Function cvReleaseFGDModel releazes memory needed for foreground detection process
// parameters:
//      p_model     - pointer to CvFGDStatModel structure
void cvReleaseFGDStatModel( CvFGDStatModel* stats );


//  Function cvUpdateCvFGDStatModel updates statistical model and returns number of foreground regions
// parameters:
//      curr_frame  - current frame from video sequence
//      p_model     - pointer to CvFGDStatModel structure
int  cvUpdateFGDStatModel( IplImage*        curr_frame,
                           CvFGDStatModel*  model );


//Function cvCreateBGStatModel creates and returns initialized BG model
// parameters:
//      first_frame   - frame from video sequence
//      model_type – type of BG model (CV_BG_MODEL_FGD,…)
//      parameters  - (optional) if NULL the default parameters of the algorithm will be used
CV_IMPL CvBGStatModel* cvCreateBGStatModel( IplImage* first_frame, int model_type, void* params )
{
    CvBGStatModel*  bg_model = NULL;

    if( model_type == CV_BG_MODEL_FGD )
    {
        CvFGDStatModel* fgd_model = (CvFGDStatModel*)cvAlloc(sizeof(CvFGDStatModel));
        if(!fgd_model) return NULL;
        if(!cvCreateFGDStatModel( first_frame, (CvFGDStatModelParams*)params, fgd_model ) ) return 0;
        bg_model = (CvBGStatModel*)fgd_model;
        bg_model->type = model_type;
    }
    else
        return 0;

    return bg_model;
}


//  Function cvReleaseBGStatModel releases memory used by BGStatModel
// parameters:
//      bg_model   - pointer to CvBGStatModel structure
CV_IMPL void  cvReleaseBGStatModel(CvBGStatModel** bg_model )
{
    CvBGStatModel* model = *bg_model;
    if( model->type == CV_BG_MODEL_FGD )
        cvReleaseFGDStatModel( (CvFGDStatModel*)model );

    cvFree((void**)&model);
    *bg_model = NULL;
}


//  Function cvUpdateBGStatModel updates statistical model and returns number of found foreground regions
// parameters:
//      curr_frame  - current frame from video sequence
//      bg_model   - pointer to CvBGStatModel structure
CV_IMPL int  cvUpdateBGStatModel( IplImage* curr_frame, CvBGStatModel*  bg_model )
{
    if( bg_model->type == CV_BG_MODEL_FGD )
        return cvUpdateFGDStatModel( curr_frame, (CvFGDStatModel*)bg_model );
    else
        return 0;

}


//  Function cvRefineForegroundMaskBySegm preforms FG post-processing based on segmentation
//    (all pixels of the segment will be classified as FG if majority of pixels of the region are FG).
// parameters:
//      segments - pointer to result of segmentation (for example MeanShiftSegmentation)
//      bg_model - pointer to CvBGStatModel structure
CV_IMPL void cvRefineForegroundMaskBySegm( CvSeq* segments, CvBGStatModel*  bg_model )
{
	IplImage* tmp_image = cvCreateImage(cvSize(bg_model->foreground->width,bg_model->foreground->height),
							IPL_DEPTH_8U, 1);
	for( ; segments; segments = ((CvSeq*)segments)->h_next )
	{
		CvSeq seq = *segments;
		seq.v_next = seq.h_next = NULL;
		cvZero(tmp_image);
		cvDrawContours( tmp_image, &seq, CV_RGB(0, 0, 255), CV_RGB(0, 0, 255), 10, -1);
		int num1 = cvCountNonZero(tmp_image);
        cvAnd(tmp_image, bg_model->foreground, tmp_image);
		int num2 = cvCountNonZero(tmp_image);
		if( num2 > num1*0.5 )
			cvDrawContours( bg_model->foreground, &seq, CV_RGB(0, 0, 255), CV_RGB(0, 0, 255), 10, -1);
		else
			cvDrawContours( bg_model->foreground, &seq, CV_RGB(0, 0, 0), CV_RGB(0, 0, 0), 10, -1);
	}
	cvReleaseImage(&tmp_image);
}

