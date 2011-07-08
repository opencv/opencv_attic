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

//forward declaration
string convertToString(const char *filename);

cl_platform_id cv::ocl::util::GetOCLPlatform()
{
    cl_platform_id pPlatforms[10] = { 0 };
    char pPlatformName[128] = { 0 };

    cl_uint uiPlatformsCount = 0;
    cl_int err = clGetPlatformIDs(10, pPlatforms, &uiPlatformsCount);
    for (cl_uint ui = 0; ui < uiPlatformsCount; ++ui)
    {
        err = clGetPlatformInfo(pPlatforms[ui], CL_PLATFORM_NAME, 128 * sizeof(char), pPlatformName, NULL);
        if ( err != CL_SUCCESS )
        {
            return NULL;
        }

            return pPlatforms[ui];
    }
}

int cv::ocl::util::createContext(cl_context* context, cl_command_queue* cmd_queue, bool hasGPU){

    size_t cb;
	cl_int err;
	cl_platform_id platform_id = cv::ocl::util::GetOCLPlatform();

	cl_context_properties context_properties[3] = {CL_CONTEXT_PLATFORM, (cl_context_properties)platform_id, NULL };

	//If the GPU is present, create the context on GPU
	if(hasGPU)
		*context = clCreateContextFromType(context_properties, CL_DEVICE_TYPE_GPU, NULL, NULL, NULL);
	else
		*context = clCreateContextFromType(context_properties, CL_DEVICE_TYPE_GPU, NULL, NULL, NULL);

    // get the list of devices associated with context
    err = clGetContextInfo(*context, CL_CONTEXT_DEVICES, 0, NULL, &cb);

	cl_device_id *devices = (cl_device_id*)malloc(cb);

	clGetContextInfo(*context, CL_CONTEXT_DEVICES, cb, devices, NULL);

    // create a command-queue
    *cmd_queue = clCreateCommandQueue(*context, devices[0], 0, NULL);
    free(devices);

	return CL_SUCCESS;
}

int cv::ocl::util::buildOCLProgram(const char *filename, cl_context* context, cl_command_queue* commandQueue, cl_program* program){

	string  sourceStr = convertToString(filename);
	cl_int status;
	cl_device_id  *devices = (cl_device_id *)malloc(4);
	size_t deviceListSize;
	const char *source    = sourceStr.c_str();
	size_t sourceSize[]    = { strlen(source) };

	*program = clCreateProgramWithSource(  *context, 
              								1, 
              								&source,
 											sourceSize,
             								&status);

	
 	

 	status = clBuildProgram(*program, 1, devices, NULL, NULL, NULL);


	if(status != CL_SUCCESS)
	{
    	if(status == CL_BUILD_PROGRAM_FAILURE)
    	{
    		cl_int logStatus;
    		char * buildLog = NULL;
    		size_t buildLogSize = 0;
    		logStatus = clGetProgramBuildInfo(	*program,
                                      			devices[0],
                                      			CL_PROGRAM_BUILD_LOG,
                                      			buildLogSize,
                                      			buildLog,
												&buildLogSize);

           	if(logStatus != CL_SUCCESS)
			{
					 cout<<"Error: Gettin Program build info\n";
			}


            
        	buildLog = (char*)malloc(buildLogSize);

        	if(buildLog == NULL)
        	{
            	 cout<<"Error: Gettin build log\n";
        	}

        	memset(buildLog, 0, buildLogSize);

        	logStatus = clGetProgramBuildInfo(	*program, 
												devices[0], 
												CL_PROGRAM_BUILD_LOG, 
												buildLogSize, 
												buildLog, 
												NULL);
        	if(logStatus != CL_SUCCESS)
			{
			 cout<<"Error: cl_get Program build info failled\n";
			free(buildLog);
			}

        	 cout << " \n\t\t\tBUILD LOG\n";
        	 cout << " ************************************************\n";
        	 cout << buildLog <<  endl;
        	 cout << " ************************************************\n";
        	free(buildLog);
        }

        if(status != CL_SUCCESS)
		{
			 cout<<"Error: cl_buildprogram failed\n";
		}
    	
	}
	return 1;
}

string convertToString(const char *filename)
{
	size_t size;
	char*  str;
	 string s;

	 fstream f(filename, ( fstream::in |  fstream::binary));

	if(f.is_open())
	{
		size_t fileSize;
		f.seekg(0,  fstream::end);
		size = fileSize = f.tellg();
		f.seekg(0,  fstream::beg);

		str = new char[size+1];
		if(!str)
		{
			f.close();
			return NULL;
		}

		f.read(str, fileSize);
		f.close();
		str[size] = '\0';
	
		s = str;
		
		return s;
	}

return NULL;
}
