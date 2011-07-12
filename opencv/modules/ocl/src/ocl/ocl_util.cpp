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

string cv::ocl::util::string_replace( string src, string const& target, string const& repl)
{
    // handle error situations/trivial cases

    if (target.length() == 0) {
        // searching for a match to the empty string will result in 
        //  an infinite loop
        //  it might make sense to throw an exception for this case
        return src;
    }

    if (src.length() == 0) {
        return src;  // nothing to match against
    }

    size_t idx = 0;

    for (;;) {
        idx = src.find( target, idx);
        if (idx == string::npos)  break;

        src.replace( idx, target.length(), repl);
        idx += repl.length();
    }

    return src;
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
	status = clGetContextInfo(  *context, 
             					CL_CONTEXT_DEVICES, 
             					4, 
             					devices, 
             					NULL);
	size_t deviceListSize;

	//sourceStr = string_replace( string_replace(string_replace(sourceStr, "${funcname}", "add32fc1"), "${T}", "float"), "${sat}", "_sat");
	//sourceStr = string_replace(sourceStr, "${op}","+");

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

//Write the PTX binary
void writeBinaries(cl_context* context, cl_command_queue* commandQueue, cl_program* program)
{
		ofstream myfile("d:/branches/ocl/opencv/modules/ocl/src/ocl/kernel.ptx");

        cl_uint program_num_devices;
        clGetProgramInfo(*program, CL_PROGRAM_NUM_DEVICES, sizeof(cl_uint), &program_num_devices, NULL);

		if (program_num_devices == 0)
        {
                        std::cerr << "no valid binary was found" << std::endl;
                        return;
        }

        size_t* binaries_sizes = new size_t[program_num_devices];

        clGetProgramInfo(*program, CL_PROGRAM_BINARY_SIZES, program_num_devices*sizeof(size_t), binaries_sizes, NULL);

        char **binaries = new char*[program_num_devices];

        for (size_t i = 0; i < 1; i++)
                        binaries[i] = new char[binaries_sizes[i]+1];

        clGetProgramInfo(*program, CL_PROGRAM_BINARIES, program_num_devices*sizeof(size_t), binaries, NULL);
        
        if(myfile.is_open())
        {
                for (size_t i = 0; i < program_num_devices; i++)
                {
                                myfile << binaries[i];
                }
        }
        myfile.close();

        for (size_t i = 0; i < program_num_devices; i++)
                        delete [] binaries[i];

        delete [] binaries;
}


int cv::ocl::util::buildOCLProgramBinary(const char *filename, cl_context* context, cl_command_queue* commandQueue, cl_program* program1){

	cl_program program;
	size_t cb;

	string  sourceStr = convertToString(filename);
	cl_int status;
	cl_device_id  *devices = (cl_device_id *)malloc(4);
	status = clGetContextInfo(  *context, 
             					CL_CONTEXT_DEVICES, 
             					4, 
             					devices, 
             					NULL);
	size_t deviceListSize;

	//const char *source    = sourceStr.c_str();

	const char* source = "__kernel void threshBin8u(__global const uchar* img, __global uchar* dst, const float thresh, const float maxval, const int imageWidth){"\
						"uint x = get_global_id(0);"\
						"uint y = get_global_id(1);"\
						"uint tid = y*imageWidth+x;"\
						"if(img[tid] > thresh)"\
						"dst[tid] = (uchar)maxval;"\
						"else"\
						"dst[tid] = 0;}";

	size_t sourceSize[]    = { strlen(source) };

	/*program = clCreateProgramWithSource(  *context, 
              								1, 
              								&source,
 											sourceSize,
             								&status);



	//writeBinaries(context, commandQueue, &program);
 	
*/
 	FILE* fp = fopen("d:/branches/ocl/opencv/modules/ocl/src/ocl/kernel.ptx", "r");
        fseek (fp , 0 , SEEK_END);
        const size_t lSize = ftell(fp);
        rewind(fp);
        unsigned char* buffer;
        buffer = (unsigned char*) malloc (lSize);
        fread(buffer, 1, lSize, fp);
        fclose(fp);

		status= clGetContextInfo(*context, CL_CONTEXT_DEVICES, 0, NULL, &cb);
		cl_device_id *cdDevices = (cl_device_id*)malloc(cb);
		clGetContextInfo(*context, CL_CONTEXT_DEVICES, cb, cdDevices, NULL);

		cl_int statusErr;
        *program1 = clCreateProgramWithBinary(*context, 1, (const cl_device_id *)cdDevices, 
                                &lSize, (const unsigned char**)&buffer, &status, &statusErr);
        
		if (status != CL_SUCCESS){ cout<<"Error in clCreateProgramWithBinary, Line "<<__LINE__<<" in file "<<__FILE__<<" "<<endl; } 
 
	
    status = clBuildProgram(*program1, 0, NULL, NULL, NULL, NULL);

			cl_int logStatus;
    		char * buildLog = NULL;
    		size_t buildLogSize = 0;
    		logStatus = clGetProgramBuildInfo(	*program1,
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

        	logStatus = clGetProgramBuildInfo(	*program1, 
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


	return status;
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


