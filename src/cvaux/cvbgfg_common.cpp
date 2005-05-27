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

/* End of file. */

