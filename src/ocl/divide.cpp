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


#include "ocl.hpp"

namespace cv{
	namespace ocl{

		extern bool initialized;

		CV_EXPORTS void divide(const OclMat& a, const OclMat& b, OclMat& c){
		
			if(!initialized)
				return;

			//if(a.empty() || b.empty() || c.empty())
				//return;

			cl_program program;
			cl_kernel  DIV;
			cl_int status;

			int size = a.cols*a.rows*a.channels();

			status = cv::ocl::util::buildOCLProgram("divide.cl", &ocl_context, &ocl_cmd_queue, &program);

			switch(a.elemSize()){
			
				case 1:
					DIV = clCreateKernel(program, "div8u", &status);
					break;

				case 2:
					DIV = clCreateKernel(program, "div16u", &status);
					break;

				case 4:
					DIV = clCreateKernel(program, "div32f", &status);
					break;
				}

			//Set Kernel Arguments, Enque and Run Kernels
			size_t     globalThreads[1];
			size_t     localThreads[2]; 
			cl_event events[1];

			//Set Kernel Arguments
			status = clSetKernelArg(DIV, 0, sizeof(cl_mem), (void *)&(a.data));
			status = clSetKernelArg(DIV, 1, sizeof(cl_mem), (void *)&(b.data));
			status = clSetKernelArg(DIV, 2, sizeof(cl_mem), (void *)&(c.data));
			status = clSetKernelArg(DIV, 3, sizeof(cl_int), (void *)&a.cols);
	
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
											DIV,
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
			status = clReleaseKernel(DIV);

			}

		CV_EXPORTS void divide(const OclMat& a, const Scalar& sc, OclMat& c){
		
			if(!initialized)
				return;

			float val = (float)sc[0];
			
			if(val == 0.0f)
				return;

			cl_program program;
			cl_kernel  DIV;
			cl_int status;

			int size = a.cols*a.rows*a.channels();

			status = cv::ocl::util::buildOCLProgram("divide.cl", &ocl_context, &ocl_cmd_queue, &program);

			switch(a.elemSize()){

				case 4:
					DIV = clCreateKernel(program, "divConst32f", &status);
					break;

				default:
					return;
				}

			//Set Kernel Arguments, Enque and Run Kernels
			size_t     globalThreads[1];
			size_t     localThreads[2]; 
			cl_event events[1];

			//Set Kernel Arguments
			status = clSetKernelArg(DIV, 0, sizeof(cl_mem), (void *)&(a.data));
			status = clSetKernelArg(DIV, 1, sizeof(cl_float), (void *)&val);
			status = clSetKernelArg(DIV, 2, sizeof(cl_mem), (void *)&(c.data));
			status = clSetKernelArg(DIV, 3, sizeof(cl_int), (void *)&a.cols);
	
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
											DIV,
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
			status = clReleaseKernel(DIV);

		}

	}
}