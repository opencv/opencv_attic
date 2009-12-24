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
 //                For Open Source Computer Vision Library
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

#include <vector>
#include <algorithm>

//#define DEBUG_WINDOWS

#if defined(DEBUG_WINDOWS)
#include "highgui.h"
#endif

void icvGetQuadrangleHypotheses(CvSeq* contours, std::vector<float>& quads)
{
    const float min_aspect_ratio = 0.7f;
    const float max_aspect_ratio = 1.3f;
    const float min_box_size = 10.0f;
    
    for(CvSeq* seq = contours; seq != NULL; seq = seq->h_next)
    {
        CvBox2D box = cvMinAreaRect2(seq);
        float aspect_ratio = box.size.width/MAX(box.size.height, 1);
        if(aspect_ratio < min_aspect_ratio || aspect_ratio > max_aspect_ratio)
        {
            continue;
        }
        
        float box_size = MAX(box.size.width, box.size.height);
        if(box_size < min_box_size)
        {
            continue;
        }
        
        quads.push_back(box_size);
    }
}

// does a fast check if a chessboard is in the input image. This is a workaround to 
// a problem of cvFindChessboardCorners being slow on images with no chessboard
// - src: input image
// - size: chessboard size
// Returns 1 if a chessboard can be in this image and findChessboardCorners should be called, 
// 0 if there is no chessboard, -1 in case of error
int cvCheckChessboard(IplImage* src, CvSize size)
{
    if(src->nChannels > 1 || src->depth != 8)
    {
        return -1;
    }
    
    const int erosion_count = 1;
    const double black_level = 60;
    const double white_level = 160;
    
#if defined(DEBUG_WINDOWS)
    cvNamedWindow("1", 1);
    cvShowImage("1", src);
    cvWaitKey(0);
#endif //DEBUG_WINDOWS

    CvMemStorage* storage = cvCreateMemStorage();
    
    IplImage* white = cvCloneImage(src);
    IplImage* black = cvCloneImage(src);
        
    cvErode(white, white, NULL, erosion_count);
    cvDilate(black, black, NULL, erosion_count);
    IplImage* thresh = cvCreateImage(cvGetSize(src), IPL_DEPTH_8U, 1);
    
    int result = 0;
    for(float thresh_level = black_level; thresh_level < white_level && !result; thresh_level += 20.0f)
    {
        cvThreshold(white, thresh, thresh_level, 255, CV_THRESH_BINARY);
        
#if defined(DEBUG_WINDOWS)
        cvShowImage("1", thresh);
        cvWaitKey(0);
#endif //DEBUG_WINDOWS
        
        CvSeq* first = 0;
        std::vector<float> quads;
        cvFindContours(thresh, storage, &first, sizeof(CvContour), CV_RETR_CCOMP);        
        icvGetQuadrangleHypotheses(first, quads);
        
        cvThreshold(black, thresh, thresh_level, 255, CV_THRESH_BINARY_INV);
        
#if defined(DEBUG_WINDOWS)
        cvShowImage("1", thresh);
        cvWaitKey(0);
#endif //DEBUG_WINDOWS
        
        cvFindContours(thresh, storage, &first, sizeof(CvContour), CV_RETR_CCOMP);
        icvGetQuadrangleHypotheses(first, quads);
        
        std::sort(quads.begin(), quads.end());
        
        // now check if there are many hypotheses with similar sizes
        // do this by floodfill-style algorithm
        const size_t min_quads_count = size.width*size.height/2;
        const float size_rel_dev = 0.3f;
        
        for(size_t i = 0; i < quads.size(); i++)
        {
            size_t j = i + 1;
            for(; j < quads.size(); j++)
            {
                if(quads[j]/quads[i] > 1.0f + size_rel_dev)
                {
                    break;
                }
            }
            
            if(j + 1 > min_quads_count + i)
            {
                result = 1;
                break;
            }
        }
    }
    
    
    cvReleaseImage(&thresh);
    cvReleaseImage(&white);
    cvReleaseImage(&black);
    cvReleaseMemStorage(&storage);
    
    return result;
}
