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

#include "_highgui.h"
#include "cvcap_tyzx.h"

#ifdef WIN32
#if defined(HAVE_TYZX)
#if _MSC_VER >= 1200
#pragma comment(lib,"DeepSeaIF.lib")
#endif

DeepSeaIF * g_tyzx_camera = 0;
int g_tyzx_refcount = 0;

int icvOpenCAM_TYZX(CvCaptureCAM_TYZX * capture, int index )
{
	if(!g_tyzx_camera){
		g_tyzx_camera = new DeepSeaIF;
		if(!g_tyzx_camera) return 0;
	
		if(!g_tyzx_camera->initializeSettings(NULL)){
			delete g_tyzx_camera;
			return 0;
		}
	
		// set initial sensor mode
		// TODO is g_tyzx_camera redundant?
		g_tyzx_camera->setSensorMode(g_tyzx_camera->getSensorMode());

		// mm's
		g_tyzx_camera->setZUnits((int) 1000);
		
	    g_tyzx_camera->enableLeftColor(true);
		g_tyzx_camera->setColorMode(DeepSeaIF::BGRcolor);
		g_tyzx_camera->setDoIntensityCrop(true);
		g_tyzx_camera->enable8bitImages(true);
		if(!g_tyzx_camera->startCapture()){
			return 0;
		}
		g_tyzx_refcount++;
	}
	capture->index=index;
	return 1;
}

void icvCloseCAM_TYZX( CvCaptureCAM_TYZX* capture )
{
	if( capture && capture->image )
	{
		cvReleaseImage( &capture->image );
		capture->image = 0;
		g_tyzx_refcount--;
		if(g_tyzx_refcount==0){
			delete g_tyzx_camera;
		}
	}
}


int icvGrabFrameCAM_TYZX( CvCaptureCAM_TYZX*  )
{
	return g_tyzx_camera && g_tyzx_camera->grab();
}

void icvAllocateImageCAM_TYZX( CvCaptureCAM_TYZX* capture ){
	int depth, nch;
	CvSize size;

	// assume we want to resize
	if(capture->image){
		cvReleaseImage(&(capture->image));
	}

	// figure out size depending on index provided
	switch(capture->index){
		case CV_TYZX_RIGHT:
			size = cvSize(g_tyzx_camera->intensityWidth(), g_tyzx_camera->intensityHeight());
			depth = 8;
			nch = 1;
			break;
		case CV_TYZX_Z:
			size = cvSize(g_tyzx_camera->zWidth(), g_tyzx_camera->zHeight());
			depth = IPL_DEPTH_16S;
			nch = 1;
			break;
		case CV_TYZX_LEFT:
		default:
			size = cvSize(g_tyzx_camera->intensityWidth(), g_tyzx_camera->intensityHeight());
			depth = 8;
			nch = 1;
			break;
	}
	capture->image = cvCreateImage(size, depth, nch);
}

/// Copy 'grabbed' image into capture buffer and return it.
IplImage* icvRetrieveFrameCAM_TYZX( CvCaptureCAM_TYZX* capture )
{
	if(!capture || !g_tyzx_camera) return 0;

	if(!capture->image){
		icvAllocateImageCAM_TYZX(capture);
		if(!capture->image) return 0;
	}

	// copy camera image into buffer.
	// tempting to reference TYZX memory directly to avoid copying.
	switch(capture->index){

		case CV_TYZX_RIGHT:
			memcpy(capture->image->imageData, g_tyzx_camera->getRImage(), capture->image->imageSize);
			break;
		case CV_TYZX_Z:
			memcpy(capture->image->imageData, g_tyzx_camera->getZImage(), capture->image->imageSize);
			break;
		case CV_TYZX_LEFT:
		default:
			memcpy(capture->image->imageData, g_tyzx_camera->getLImage(), capture->image->imageSize);
			break;
	}

	return capture->image;
}


double icvGetPropertyCAM_TYZX( CvCaptureCAM_TYZX* capture, int property_id )
{
	CvSize size;
	switch(capture->index){
		case CV_TYZX_LEFT:
			size = cvSize(g_tyzx_camera->intensityWidth(), g_tyzx_camera->intensityHeight());
			break;
		case CV_TYZX_RIGHT:
			size = cvSize(g_tyzx_camera->intensityWidth(), g_tyzx_camera->intensityHeight());
			break;
		case CV_TYZX_Z:
			size = cvSize(g_tyzx_camera->zWidth(), g_tyzx_camera->zHeight());
			break;
		default:
			size=cvSize(0,0);
	}
	switch( property_id )
	{
		case CV_CAP_PROP_FRAME_WIDTH:
			return size.width;
		case CV_CAP_PROP_FRAME_HEIGHT:
			return size.height;
	}
	return 0;
}

int icvSetPropertyCAM_TYZX( CvCaptureCAM_TYZX* , int , double  ){
	int retval = -1;
	return retval;
}


#endif // TYZX
#endif // WIN32

