/*M///////////////////////////////////////////////////////////////////////////////////////
//
//  IMPORTANT: READ BEFORE DOWNLOADING, COPYING, INSTALLING OR USING.
//
//  By downloading, copying, installing or using the software you agree to this license.
//  If you do not agree to this license, do not download, install,
//  copy or use the software.
//
//
//                           License Agreement
//                For Open Source Computer Vision Library
//
// Copyright (C) 2000-2008, Intel Corporation, all rights reserved.
// Copyright (C) 2009, Willow Garage Inc., all rights reserved.
// Third party copyrights are property of their respective owners.
//
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
//
//   * Redistribution's of source code must retain the above copyright notice,
//     this list of conditions and the following disclaimer.
//
//   * The name of the copyright holders may not be used to endorse or promote products
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

namespace cv{
	namespace ocl{

		extern bool initialized;
		bool initOFlowLK = false;

		cl_program program;

		CV_EXPORTS void calcOpticalFlowLK(const OclMat& prev, const OclMat& img, OclMat& velX, OclMat& velY, CvSize winSize){
		

			if(!initialized)
				return;

			//if(prev.empty() || img.empty() || velX.empty() || velY.empty())
				//return;

			if(winSize.width != winSize.height || winSize.width <= 0 || winSize.height <= 0)
				return;

			int xRadius = winSize.width/2;
			int yRadius = winSize.height/2;

			//cl_program program;
			//cl_kernel  OFLOWLK;
			cl_int status;

			cl_mem fx;
			cl_mem fy;
			cl_mem ft;

			cl_kernel OFLOWLK;

			int size = prev.cols*prev.rows;

			//Build only once, cache it afterwards.
			if(!initOFlowLK){
			status = cv::ocl::util::buildOCLProgram("opticalFlowLK.cl", &ocl_context, &ocl_cmd_queue, &program);
			initOFlowLK = true;}

			OFLOWLK = clCreateKernel(program, "derivatives_compute", &status);

			fx = clCreateBuffer(ocl_context, CL_MEM_READ_WRITE, sizeof(cl_float)*size, NULL, &status);	
			fy = clCreateBuffer(ocl_context, CL_MEM_READ_WRITE, sizeof(cl_float)*size, NULL, &status);	
			ft = clCreateBuffer(ocl_context, CL_MEM_READ_WRITE, sizeof(cl_float)*size, NULL, &status);	

			//Set Kernel Arguments, Enque and Run Kernels
			size_t     globalThreads[1];
			size_t     localThreads[2]; 
			cl_event events[1];

			//Set Kernel Arguments
			status = clSetKernelArg(OFLOWLK, 0, sizeof(cl_mem), (void *)&(prev.data));
			status = clSetKernelArg(OFLOWLK, 1, sizeof(cl_mem), (void *)&(img.data));
			status = clSetKernelArg(OFLOWLK, 2, sizeof(cl_mem), (void *)&(velX.data));
			status = clSetKernelArg(OFLOWLK, 3, sizeof(cl_mem), (void *)&(velY.data));
			status = clSetKernelArg(OFLOWLK, 4, sizeof(cl_mem), (void *)&fx);
			status = clSetKernelArg(OFLOWLK, 5, sizeof(cl_mem), (void *)&fy);
			status = clSetKernelArg(OFLOWLK, 6, sizeof(cl_mem), (void *)&ft);
			status = clSetKernelArg(OFLOWLK, 7, sizeof(cl_int), (void *)&(prev.cols));
			status = clSetKernelArg(OFLOWLK, 8, sizeof(cl_int), (void *)&xRadius);
			status = clSetKernelArg(OFLOWLK, 9, sizeof(cl_int), (void *)&yRadius);

			globalThreads[0] = size;
			localThreads[0] = 16;
			localThreads[1] = 16;

			int totalLocal = localThreads[0]*localThreads[1];

			if (totalLocal > globalThreads[0]){
				localThreads[0] = globalThreads[0];
				localThreads[1] = 1;
			}

			if ((globalThreads[0])%(totalLocal) != 0)
				globalThreads[0] += totalLocal -(globalThreads[0])%(totalLocal);


			//Execute the kernel
			status = clEnqueueNDRangeKernel(ocl_cmd_queue, 
											OFLOWLK,
											1,
											NULL,
											globalThreads,
											localThreads,
											0,
											NULL,
											&events[0]);

			//Blocking kernel execution. Wait for completion
			status = clWaitForEvents(1, &events[0]);
			
			//Cleanup
			clReleaseEvent(events[0]);
			status = clReleaseMemObject(fx);
			status = clReleaseMemObject(fy);
			status = clReleaseMemObject(ft);
			status = clReleaseKernel(OFLOWLK);
		}

	}
}