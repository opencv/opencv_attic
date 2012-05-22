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
// Copyright (C) 2010-2012, Institute Of Software Chinese Academy Of Science, all rights reserved.
// Copyright (C) 2010-2012, Advanced Micro Devices, Inc., all rights reserved.
// Third party copyrights are property of their respective owners.
//
// @Authors
//    Guoping Long, longguoping@gmail.com
//	  Niko Li, newlife20080214@gmail.com
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
//
//   * Redistribution's of source code must retain the above copyright notice,
//     this list of conditions and the following disclaimer.
//
//   * Redistribution's in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other oclMaterials provided with the distribution.
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

#include "opencv2/ocl/Threadsafe.h"
#include <iomanip>
#include "precomp.hpp"
#include "binaryCaching.hpp"

using namespace cv;
using namespace cv::ocl;
using namespace std;
using std::cout;
using std::endl;
#define USE_CL_BIN

//#define PRINT_KERNEL_RUN_TIME
#define RUN_TIMES 1000

//#define AMD_DOUBLE_DIFFER

#if !defined (HAVE_OPENCL)

namespace cv
{
    namespace ocl
    {

        cl_device_id getDevice()
        {
            throw_nogpu();
            return 0;
        }

        void getComputeCapability(cl_device_id, int &major, int &minor)
        {
            throw_nogpu();
        }

        void openCLMallocPitch(ClContext * /*clCxt*/, void **/*dev_ptr*/, size_t * /*pitch*/,
                               size_t /*widthInBytes*/, size_t /*height*/)
        {
            throw_nogpu();
        }

        void openCLMemcpy2D(ClContext * /*clCxt*/, void * /*dst*/, size_t /*dpitch*/,
                            const void * /*src*/, size_t /*spitch*/,
                            size_t /*width*/, size_t /*height*/, enum openCLMemcpyKind /*kind*/)
        {
            throw_nogpu();
        }

        void openCLCopyBuffer2D(ClContext * /*clCxt*/, void * /*dst*/, size_t /*dpitch*/,
                                const void * /*src*/, size_t /*spitch*/,
                                size_t /*width*/, size_t /*height*/, enum openCLMemcpyKind /*kind*/)
        {
            throw_nogpu();
        }

        void openCLFree(void * /*devPtr*/)
        {
            throw_nogpu();
        }

        cl_kernel openCLGetKernelFromSource(const ClContext * /*clCxt*/,
                                            const char **/*fileName*/, string /*kernelName*/)
        {
            throw_nogpu();
        }

        void openCLVerifyKernel(const ClContext * /*clCxt*/, cl_kernel /*kernel*/, size_t * /*blockSize*/,
                                size_t * /*globalThreads*/, size_t * /*localThreads*/)
        {
            throw_nogpu();
        }

        cl_mem load_constant(cl_context context, cl_command_queue command_queue, const void *value,
                             const size_t size)
        {
            throw_nogpu();
        }

    }//namespace ocl
}//namespace cv

#else /* !defined (HAVE_OPENCL) */

namespace cv
{
    namespace ocl
    {
        //CriticalSection cs;
        /*
        * The binary caching system to eliminate redundant program source compilation.
        * Strictly, this is not a cache because we do not implement evictions right now.
        * We shall add such features to trade-off memory consumption and performance when necessary.
        */
        auto_ptr<ProgramCache> ProgramCache::programCache;
        ProgramCache *programCache = NULL;
        ProgramCache::ProgramCache()
        {
            codeCache.clear();
            cacheSize = 0;
        }

        ProgramCache::~ProgramCache()
        {
            releaseProgram();
        }

        cl_program ProgramCache::progLookup(string srcsign)
        {
            map<string, cl_program>::iterator iter;
            iter = codeCache.find(srcsign);
            if(iter != codeCache.end())
                return iter->second;
            else
                return NULL;
        }

        void ProgramCache::addProgram(string srcsign , cl_program program)
        {
            if(!progLookup(srcsign))
            {
                codeCache.insert(map<string, cl_program>::value_type(srcsign, program));
            }
        }

        void ProgramCache::releaseProgram()
        {
            map<string, cl_program>::iterator iter;
            for(iter = codeCache.begin(); iter != codeCache.end(); iter++)
            {
                openCLSafeCall(clReleaseProgram(iter->second));
            }
            codeCache.clear();
            cacheSize = 0;
            //cout << "release Program Cache succed!" << endl;
        }

        ////////////////////////Common OpenCL specific calls///////////////
        //OCLInfo::OCLInfo()
        //{
        //	oclplatform = 0;
        //	oclcontext = 0;
        //	devnum = 0;
        //}
        //OCLInfo::~OCLInfo()
        //{
        //	release();
        //}
        //void OCLInfo::release()
        //{
        //	if(oclplatform)
        //	{
        //		oclplatform = 0;
        //	}
        //	if(oclcontext)
        //	{
        //		openCLSafeCall(clReleaseContext(oclcontext));
        //	}
        //	devices.empty();
        //	devName.empty();
        //}
        inline int divUp(int total, int grain)
        {
            return (total + grain - 1) / grain;
        }
        cl_device_id getDevice()
        {
            ClContext *clCxt = ClContext::getContext();
            return clCxt->devices[0];
        }
        int getDevice(std::vector<OCLInfo> &oclinfo, cl_device_type devicetype)
        {
            int devcienums = 0;
            //int oclinfo_i = oclinfo.size();
            // Platform info
            cl_int status = 0;
            cl_uint numPlatforms;
            //cl_platform_id platform = NULL;
            OCLInfo ocltmpinfo;
            openCLSafeCall(clGetPlatformIDs(0, NULL, &numPlatforms));
            CV_Assert(numPlatforms > 0);
            cl_platform_id *platforms = new cl_platform_id[numPlatforms];

            openCLSafeCall(clGetPlatformIDs(numPlatforms, platforms, NULL));
            char deviceName[256];
            for (unsigned i = 0; i < numPlatforms; ++i)
            {
                cl_uint numsdev;
                status = clGetDeviceIDs(platforms[i], devicetype, 0, NULL, &numsdev);
                if(status != CL_DEVICE_NOT_FOUND)
                {
                    openCLVerifyCall(status);
                }
                if(numsdev > 0)
                {
                    devcienums += numsdev;
                    cl_device_id *devices = new cl_device_id[numsdev];
                    openCLSafeCall(clGetDeviceIDs(platforms[i], devicetype, numsdev, devices, NULL));
                    ocltmpinfo.oclplatform = platforms[i];
                    for(unsigned j = 0; j < numsdev; j++)
                    {
                        ocltmpinfo.devices.push_back(devices[j]);
                        openCLSafeCall(clGetDeviceInfo(devices[j], CL_DEVICE_NAME, 256, deviceName, NULL));
                        ocltmpinfo.devName.push_back(std::string(deviceName));
                    }
                    delete[] devices;
                    oclinfo.push_back(ocltmpinfo);
                    ocltmpinfo.release();
                }
            }
            delete[] platforms;
            if(devcienums > 0)
            {
                setDevice(oclinfo[0]);
            }
            return devcienums;
        }
        void setDevice(OCLInfo &oclinfo, int devnum)
        {
            CV_Assert(devnum >= 0);
            cl_int status = 0;
            cl_context_properties cps[3] =
            {
                CL_CONTEXT_PLATFORM, (cl_context_properties)(oclinfo.oclplatform), 0
            };
            oclinfo.devnum = devnum;
            oclinfo.oclcontext = clCreateContext(cps, 1, &oclinfo.devices[devnum], NULL, NULL, &status);
            openCLVerifyCall(status);
            //create the command queue using the first device of the list
            oclinfo.clCmdQueue = clCreateCommandQueue(oclinfo.oclcontext, oclinfo.devices[devnum],
                                 CL_QUEUE_PROFILING_ENABLE, &status);
            openCLVerifyCall(status);

            //get device information
            openCLSafeCall(clGetDeviceInfo(oclinfo.devices[devnum], CL_DEVICE_MAX_WORK_GROUP_SIZE,
                                           sizeof(size_t), (void *)&oclinfo.maxWorkGroupSize, NULL));
            openCLSafeCall(clGetDeviceInfo(oclinfo.devices[devnum], CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS,
                                           sizeof(cl_uint), (void *)&oclinfo.maxDimensions, NULL));
            oclinfo.maxWorkItemSizes = new size_t[oclinfo.maxDimensions];
            openCLSafeCall(clGetDeviceInfo(oclinfo.devices[devnum], CL_DEVICE_MAX_WORK_ITEM_SIZES,
                                           sizeof(size_t)*oclinfo.maxDimensions, (void *)oclinfo.maxWorkItemSizes, NULL));
            openCLSafeCall(clGetDeviceInfo(oclinfo.devices[devnum], CL_DEVICE_MAX_COMPUTE_UNITS,
                                           sizeof(cl_uint), (void *)&oclinfo.maxComputeUnits, NULL));
            //initialize extra options for compilation. Currently only fp64 is included.
            //Assume 4KB is enough to store all possible extensions.

            const int EXT_LEN = 4096 + 1 ;
            char extends_set[EXT_LEN];
            size_t extends_size;
            openCLSafeCall(clGetDeviceInfo(oclinfo.devices[devnum], CL_DEVICE_EXTENSIONS,
                                           EXT_LEN, (void *)extends_set, &extends_size));
            CV_Assert(extends_size < EXT_LEN);
            extends_set[EXT_LEN-1] = 0;
            //oclinfo.extra_options = NULL;
            int fp64_khr = string(extends_set).find("cl_khr_fp64");
            int fp64_amd = string(extends_set).find("cl_amd_fp64");
            //CV_Assert( !(fp64_nvi>=0 && fp64_nvi<EXT_LEN) || !(fp64_amd>=0 && fp64_amd<EXT_LEN) );
            if(fp64_khr >= 0 && fp64_khr < EXT_LEN)
            {
                //CV_Assert(!oclinfo.extra_options);
                sprintf(oclinfo.extra_options , "-D __NVIDIA__ -D DOUBLE_SUPPORT");
            }
            else if(fp64_amd >= 0 && fp64_amd < EXT_LEN)
            {
                //CV_Assert(!oclinfo.extra_options);
                sprintf(oclinfo.extra_options , "-D __ATI__ -D DOUBLE_SUPPORT");
            }
            ClContext::setContext(oclinfo);
        }
        void getComputeCapability(cl_device_id, int &major, int &minor)
        {
            major = 1;
            minor = 1;
        }

        void openCLMallocPitch(ClContext *clCxt, void **dev_ptr, size_t *pitch,
                               size_t widthInBytes, size_t height)
        {
            cl_int status;

            *dev_ptr = clCreateBuffer(clCxt->clContext, CL_MEM_READ_WRITE,
                                      widthInBytes * height, 0, &status);
            openCLVerifyCall(status);
            *pitch = widthInBytes;
        }

        void openCLMemcpy2D(ClContext *clCxt, void *dst, size_t dpitch,
                            const void *src, size_t spitch,
                            size_t width, size_t height, enum openCLMemcpyKind kind)
        {
            //CV_DbgAssert( (dpitch == width) && (spitch == width) );

            size_t buffer_origin[3] = {0, 0, 0};
            size_t host_origin[3] = {0, 0, 0};
            size_t region[3] = {width, height, 1};
            if(kind == clMemcpyHostToDevice)
            {
                openCLSafeCall(clEnqueueWriteBufferRect(clCxt->clCmdQueue, (cl_mem)dst, CL_TRUE,
                                                        buffer_origin, host_origin, region, dpitch, 0, spitch, 0, src, 0, 0, 0));

                //openCLSafeCall(clEnqueueWriteBuffer(clCxt->clCmdQueue,
                //(cl_mem)dst,1,0,width*height,src,0,0,0));

            }
            else if(kind == clMemcpyDeviceToHost)
            {
                openCLSafeCall(clEnqueueReadBufferRect(clCxt->clCmdQueue, (cl_mem)src, CL_TRUE,
                                                       buffer_origin, host_origin, region, spitch, 0, dpitch, 0, dst, 0, 0, 0));

                //openCLSafeCall(clEnqueueReadBuffer(clCxt->clCmdQueue,
                //(cl_mem)src,CL_TRUE,0,width*height,dst,0,0,0));
            }
        }

        void openCLCopyBuffer2D(ClContext *clCxt, void *dst, size_t dpitch, int dst_offset,
                                const void *src, size_t spitch,
                                size_t width, size_t height, int src_offset, enum openCLMemcpyKind kind)
        {
            //openCLSafeCall(clEnqueueCopyBuffer(clCxt->clCmdQueue,
            //(cl_mem)src,(cl_mem)dst,0,0,width*height,0,0,0));

            size_t src_origin[3] = {src_offset % spitch, src_offset / spitch, 0};
            size_t dst_origin[3] = {dst_offset % dpitch, dst_offset / dpitch, 0};
            size_t region[3] = {width, height, 1};

            openCLSafeCall(clEnqueueCopyBufferRect(clCxt->clCmdQueue, (cl_mem)src, (cl_mem)dst, src_origin, dst_origin,
                                                   region, spitch, 0, dpitch, 0, 0, 0, 0));
        }

        void openCLFree(void *devPtr)
        {
            openCLSafeCall(clReleaseMemObject((cl_mem)devPtr));
        }
        cl_kernel openCLGetKernelFromSource(const ClContext *clCxt, const char **source, string kernelName)
        {
            return openCLGetKernelFromSource(clCxt, source, kernelName, NULL);
        }
#ifdef USE_CL_BIN
        string Binpath;
        void setBinpath(const char *path)
        {
            Binpath = path;
        }
        int savetofile(const ClContext *clcxt,  cl_program &program, const char *kernelname)
        {
            cl_int status;
            size_t numDevices = 1;    //save the compile info of the frist device only
            cl_device_id *devices = clcxt->devices;
            //figure out the sizes of each of the binaries.
            size_t *binarySizes = (size_t *)malloc( sizeof(size_t) * numDevices );
            if(devices == NULL)
            {
                printf("Failed to allocate host memory.(binarySizes)\r\n");
                return 0;
            }

            status = clGetProgramInfo(program,
                                      CL_PROGRAM_BINARY_SIZES,
                                      sizeof(size_t) * numDevices,
                                      binarySizes, NULL);
            if(status != CL_SUCCESS)
            {
                printf("clGetProgramInfo(CL_PROGRAM_BINARY_SIZES) failed.\r\n");
                return 0;
            }

            size_t i = 0;
            //copy over all of the generated binaries.
            char **binaries = (char **)malloc( sizeof(char *) * numDevices );
            if(binaries == NULL)
            {
                printf("Failed to allocate host memory.(binaries)\r\n");
                return 0;
            }

            for(i = 0; i < numDevices; i++)
            {
                if(binarySizes[i] != 0)
                {
                    binaries[i] = (char *)malloc( sizeof(char) * binarySizes[i]);
                    if(binaries[i] == NULL)
                    {
                        printf("Failed to allocate host memory.(binaries[i])\r\n");
                        return 0;
                    }
                }
                else
                {
                    binaries[i] = NULL;
                }
            }
            status = clGetProgramInfo(program,
                                      CL_PROGRAM_BINARIES,
                                      sizeof(char *) * numDevices,
                                      binaries,
                                      NULL);
            if(status != CL_SUCCESS)
            {
                printf("clGetProgramInfo(CL_PROGRAM_BINARIES) failed.\r\n");
                return 0;
            }

            //dump out each binary into its own separate file.
            for(i = 0; i < numDevices; i++)
            {
                char fileName[100];
                sprintf(fileName, "%s\\%s.%d", Binpath.c_str(), kernelname, (int)i);
                if(binarySizes[i] != 0)
                {
                    char deviceName[1024];
                    status = clGetDeviceInfo(devices[i],
                                             CL_DEVICE_NAME,
                                             sizeof(deviceName),
                                             deviceName,
                                             NULL);
                    if(status != CL_SUCCESS)
                    {
                        printf( "clGetDeviceInfo(CL_DEVICE_NAME) failed.\r\n");
                        return 0;
                    }

                    printf( "%s binary kernel: %s\n", deviceName, fileName);
                    FILE *fp = fopen(fileName, "wb+");
                    if(fp == NULL)
                    {
                        printf("Failed to load kernel file : %s\r\n", fileName);
                        return 0;
                    }
                    else
                    {
                        fwrite(binaries[i], binarySizes[i], 1, fp);
                        free(binaries[i]);
                        fclose(fp);
                    }
                }
                else
                {
                    printf("Skipping %s since there is no binary data to write!\n",
                           fileName);
                }
            }
            free(binarySizes);
            free(binaries);
            return 1;
        }
#endif

        cl_kernel openCLGetKernelFromSource(const ClContext *clCxt, const char **source, string kernelName,
                                            const char *build_options)
        {
            cl_kernel kernel;
            cl_program program ;
            cl_int status = 0;
            stringstream src_sign;
            string srcsign;
            CV_Assert(programCache != NULL);

            if(NULL != build_options)
                src_sign << (int64)source << clCxt->clContext << "_" << build_options;
            else
                src_sign << (int64)source << clCxt->clContext;
            srcsign = src_sign.str();

            program = NULL;
            program = programCache->progLookup(srcsign);

            if(!program)
            {
                //config build programs
                char all_build_options[1024];
                memset(all_build_options, 0, 1024);
                if(clCxt -> extra_options)
                    strcat(all_build_options, clCxt -> extra_options);
                strcat(all_build_options, " ");
                if(build_options != NULL)
                    strcat(all_build_options, build_options);
#ifdef USE_CL_BIN
                FILE *fp;
                string filename = Binpath + "\\" + kernelName + ".0";
                fp = fopen(filename.c_str(), "rb");
                if(fp == NULL || Binpath.size() == 0)                              //we should genetate a binary file for the first time.
                {
                    program = clCreateProgramWithSource(
                                  clCxt->clContext, 1, source, NULL, &status);
                    openCLVerifyCall(status);
                    status = clBuildProgram(program, 1, &(clCxt->devices[0]), all_build_options, NULL, NULL);
                    if(status == CL_SUCCESS && Binpath.size()) savetofile(clCxt, program, kernelName.c_str());
                }
                else
                {
                    fseek(fp, 0, SEEK_END);
                    size_t binarySize = ftell(fp);
                    fseek(fp, 0, SEEK_SET);
                    char *binary = new char[binarySize];
                    fread(binary, binarySize, 1, fp);
                    fclose(fp);
                    cl_int status = 0;
                    program = clCreateProgramWithBinary(clCxt->clContext,
                                                        1,
                                                        &(clCxt->devices[0]),
                                                        (const size_t *)&binarySize,
                                                        (const unsigned char **)&binary,
                                                        NULL,
                                                        &status);
                    openCLVerifyCall(status);
                    status = clBuildProgram(program, 1, &(clCxt->devices[0]), all_build_options, NULL, NULL);


                }
#else
program = clCreateProgramWithSource(
              clCxt->clContext, 1, source, NULL, &status);
openCLVerifyCall(status);
status = clBuildProgram(program, 1, &(clCxt->devices[0]), all_build_options, NULL, NULL);
#endif

                //status = clBuildProgram(program,1,&(clCxt->devices[0]), all_build_options, NULL,NULL);
                if(status != CL_SUCCESS)
                {
                    if(status == CL_BUILD_PROGRAM_FAILURE)
                    {
                        cl_int logStatus;
                        char *buildLog = NULL;
                        size_t buildLogSize = 0;
                        logStatus = clGetProgramBuildInfo(program,
                                                          clCxt->devices[0], CL_PROGRAM_BUILD_LOG, buildLogSize,
                                                          buildLog, &buildLogSize);
                        if(logStatus != CL_SUCCESS)
                            cout << "Failed to build the program and get the build info." << endl;
                        buildLog = new char[buildLogSize];
                        CV_DbgAssert(!!buildLog);
                        memset(buildLog, 0, buildLogSize);
                        openCLSafeCall(clGetProgramBuildInfo(program, clCxt->devices[0],
                                                             CL_PROGRAM_BUILD_LOG, buildLogSize, buildLog, NULL));
                        cout << "\n\t\t\tBUILD LOG\n";
                        cout << buildLog << endl;
                        delete buildLog;
                    }
                    openCLVerifyCall(status);
                }
                //Cache the binary for future use if build_options is null
                if( (programCache->cacheSize += 1) < programCache->MAX_PROG_CACHE_SIZE)
                    programCache->addProgram(srcsign, program);
                else cout << "Warning: code cache has been full.\n";
            }
            kernel = clCreateKernel(program, kernelName.c_str(), &status);
            openCLVerifyCall(status);
            return kernel;
        }

        void openCLVerifyKernel(const ClContext *clCxt, cl_kernel kernel, size_t *blockSize,
                                size_t *globalThreads, size_t *localThreads)
        {
            size_t kernelWorkGroupSize;
            openCLSafeCall(clGetKernelWorkGroupInfo(kernel, clCxt->devices[0],
                                                    CL_KERNEL_WORK_GROUP_SIZE, sizeof(size_t), &kernelWorkGroupSize, 0));
            //cout << "workgroup size: " << kernelWorkGroupSize << endl;

            // This verify action implementation later
            //if(localThreads[0] * localThreads[1] > kernelWorkGroupSize) {
            //CV_DbgAssert( (kernelWorkGroupSize >= 64 ||
            //kernelWorkGroupSize >= 32) );
            //if(kernelWorkGroupSize >= 64)
            //{
            //*blockSize = 8;
            //localThreads[0] = *blockSize;
            //localThreads[1] = *blockSize;
            //}else//(kernelWorkGroupSize >= 32)
            //{
            //*blockSize = 4;
            //localThreads[0] = *blockSize;
            //localThreads[1] = *blockSize;
            //}
            //}
            CV_DbgAssert( (localThreads[0] <= clCxt->maxWorkItemSizes[0]) &&
                          (localThreads[1] <= clCxt->maxWorkItemSizes[1]) &&
                          (localThreads[2] <= clCxt->maxWorkItemSizes[2]) &&
                          ((localThreads[0] * localThreads[1] * localThreads[2]) <= kernelWorkGroupSize) &&
                          (localThreads[0] * localThreads[1] * localThreads[2]) <= clCxt->maxWorkGroupSize);
        }
        void openCLExecuteKernel(ClContext *clCxt , const char **source, string kernelName, vector< pair<size_t, const void *> > &args,
                                 int globalcols , int globalrows, size_t blockSize , int kernel_expand_depth , int kernel_expand_channel )
        {
            stringstream idxStr;
            if(kernel_expand_depth != -1)
            {
                idxStr << "_";
                idxStr << kernel_expand_depth;
            }
            if(kernel_expand_channel != -1)
            {
                idxStr << "_";
                idxStr << kernel_expand_channel;
            }
            kernelName += idxStr.str();

            cout << kernelName << endl;

            cl_kernel kernel;
            kernel = openCLGetKernelFromSource(clCxt, source, kernelName);
            int t1 = globalcols % blockSize, t2 = globalrows % blockSize;
            if(t1 != 0)
                t1 = blockSize - t1;
            if(t2 != 0)
                t2 = blockSize - t2;
            size_t globalThreads[3] = { globalcols + t1 , globalrows + t2, 1 };
            size_t localThreads[3] = { blockSize , blockSize, 1 };
            cv::ocl::openCLVerifyKernel(clCxt, kernel, &blockSize, globalThreads, localThreads);

            for(int i = 0; i < args.size(); i ++)
            {
                openCLSafeCall(clSetKernelArg(kernel, i, args[i].first, args[i].second));
            }
            cl_event time_event;
            cl_ulong queued, start, end;
            openCLSafeCall(clEnqueueNDRangeKernel(clCxt->clCmdQueue, kernel, 3, NULL, globalThreads,
                                                  localThreads, 0, NULL, &time_event));
            clWaitForEvents(1, &time_event);
            clFinish(clCxt->clCmdQueue);
            openCLSafeCall(clReleaseKernel(kernel));
            clGetEventProfilingInfo(time_event, CL_PROFILING_COMMAND_QUEUED, sizeof(cl_ulong) , &queued, NULL);
            clGetEventProfilingInfo(time_event, CL_PROFILING_COMMAND_START, sizeof(cl_ulong) , &start, NULL);
            clGetEventProfilingInfo(time_event, CL_PROFILING_COMMAND_END, sizeof(cl_ulong) , &end, NULL);
            //cl_ulong kernelRealExecTimeNs = end - start;
            //cl_ulong kernelTotalExecTimeNs = end - queued;
            //printf("kernel real run time=%lf\n",(double)kernelRealExecTimeNs/1000);
            //printf("kernel total run time=%lf\n",(double)kernelTotalExecTimeNs/1000);
        }

#ifdef PRINT_KERNEL_RUN_TIME
        static double total_execute_time = 0;
        static double total_kernel_time = 0;
#endif
        void openCLExecuteKernel_(ClContext *clCxt , const char **source, string kernelName, size_t globalThreads[3],
                                  size_t localThreads[3],  vector< pair<size_t, const void *> > &args, int channels,
                                  int depth, char *build_options)
        {
            //construct kernel name
            //The rule is functionName_Cn_Dn, C represent Channels, D Represent DataType Depth, n represent an integer number
            //for exmaple split_C2_D2, represent the split kernel with channels =2 and dataType Depth = 2(Data type is char)
            stringstream idxStr;
            if(channels != -1)
                idxStr << "_C" << channels;
            if(depth != -1)
                idxStr << "_D" << depth;
            kernelName += idxStr.str();

            cl_kernel kernel;
            kernel = openCLGetKernelFromSource(clCxt, source, kernelName, build_options);

            globalThreads[0] = divUp(globalThreads[0], localThreads[0]) * localThreads[0];
            globalThreads[1] = divUp(globalThreads[1], localThreads[1]) * localThreads[1];
            globalThreads[2] = divUp(globalThreads[2], localThreads[2]) * localThreads[2];

            size_t blockSize = localThreads[0] * localThreads[1] * localThreads[2];
            cv::ocl::openCLVerifyKernel(clCxt, kernel, &blockSize, globalThreads, localThreads);

            for(int i = 0; i < args.size(); i ++)
                openCLSafeCall(clSetKernelArg(kernel, i, args[i].first, args[i].second));

#ifndef PRINT_KERNEL_RUN_TIME
            openCLSafeCall(clEnqueueNDRangeKernel(clCxt->clCmdQueue, kernel, 3, NULL, globalThreads,
                                                  localThreads, 0, NULL, NULL));

#else
cl_event event = NULL;
openCLSafeCall(clEnqueueNDRangeKernel(clCxt->clCmdQueue, kernel, 3, NULL, globalThreads,
                                      localThreads, 0, NULL, &event));

cl_ulong start_time, end_time, queue_time;
double execute_time = 0;
double total_time   = 0;

openCLSafeCall(clWaitForEvents(1, &event));
openCLSafeCall(clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_START,
                                       sizeof(cl_ulong), &start_time, 0));

openCLSafeCall(clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_END,
                                       sizeof(cl_ulong), &end_time, 0));

openCLSafeCall(clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_QUEUED,
                                       sizeof(cl_ulong), &queue_time, 0));

execute_time = (double)(end_time - start_time) / (1000 * 1000);
total_time = (double)(end_time - queue_time) / (1000 * 1000);

//	cout << setiosflags(ios::left) << setw(15) << execute_time;
//	cout << setiosflags(ios::left) << setw(15) << total_time - execute_time;
//	cout << setiosflags(ios::left) << setw(15) << total_time << endl;

total_execute_time += execute_time;
total_kernel_time += total_time;
clReleaseEvent(event);
#endif

            clFinish(clCxt->clCmdQueue);
            openCLSafeCall(clReleaseKernel(kernel));
        }

        void openCLExecuteKernel(ClContext *clCxt , const char **source, string kernelName,
                                 size_t globalThreads[3], size_t localThreads[3],
                                 vector< pair<size_t, const void *> > &args, int channels, int depth)
        {
            openCLExecuteKernel(clCxt, source, kernelName, globalThreads, localThreads, args,
                                channels, depth, NULL);
        }
        void openCLExecuteKernel(ClContext *clCxt , const char **source, string kernelName,
                                 size_t globalThreads[3], size_t localThreads[3],
                                 vector< pair<size_t, const void *> > &args, int channels, int depth, char *build_options)

        {
#ifndef PRINT_KERNEL_RUN_TIME
            openCLExecuteKernel_(clCxt, source, kernelName, globalThreads, localThreads, args, channels, depth,
                                 build_options);
#else
string data_type[] = { "uchar", "char", "ushort", "short", "int", "float", "double"};
cout << endl;
cout << "Function Name: " << kernelName;
if(depth >= 0)
    cout << " |data type: " << data_type[depth];
cout << " |channels: " << channels;
cout << " |Time Unit: " << "ms" << endl;

total_execute_time = 0;
total_kernel_time = 0;
cout << "-------------------------------------" << endl;

cout << setiosflags(ios::left) << setw(15) << "excute time";
cout << setiosflags(ios::left) << setw(15) << "lauch time";
cout << setiosflags(ios::left) << setw(15) << "kernel time" << endl;
int i = 0;
for(i = 0; i < RUN_TIMES; i++)
    openCLExecuteKernel_(clCxt, source, kernelName, globalThreads, localThreads, args, channels, depth,
                         build_options);

cout << "average kernel excute time: " << total_execute_time / RUN_TIMES << endl; // "ms" << endl;
cout << "average kernel total time:  " << total_kernel_time / RUN_TIMES << endl; // "ms" << endl;
#endif
        }

        cl_mem load_constant(cl_context context, cl_command_queue command_queue, const void *value,
                             const size_t size)
        {
            int status;
            cl_mem con_struct;

            con_struct = clCreateBuffer(context, CL_MEM_READ_ONLY, size, NULL, &status);
            openCLSafeCall(status);

            openCLSafeCall(clEnqueueWriteBuffer(command_queue, con_struct, 1, 0, size,
                                                value, 0, 0, 0));

            return con_struct;

        }

        /////////////////////////////OpenCL initialization/////////////////
        auto_ptr<ClContext> ClContext::clCxt;
        int ClContext::val = 0;
        CriticalSection cs;
        ClContext *ClContext::getContext()
        {
            if(val == 0)
            {
                AutoLock al(&cs);
                if( NULL == clCxt.get())
                    clCxt.reset(new ClContext);

                val = 1;
                return clCxt.get();
            }
            else
            {
                return clCxt.get();
            }
        }
        void ClContext::setContext(OCLInfo &oclinfo)
        {
            ClContext *clcxt = getContext();
            clcxt->clContext = oclinfo.oclcontext;
            clcxt->clCmdQueue = oclinfo.clCmdQueue;
            clcxt->devices = &oclinfo.devices[oclinfo.devnum];
            clcxt->maxDimensions = oclinfo.maxDimensions;
            clcxt->maxWorkGroupSize = oclinfo.maxWorkGroupSize;
            clcxt->maxWorkItemSizes = oclinfo.maxWorkItemSizes;
            clcxt->maxComputeUnits = oclinfo.maxComputeUnits;
            //extra options to recognize compiler options
            clcxt->extra_options = oclinfo.extra_options;
        }
        ClContext::ClContext()
        {
            //Information of the OpenCL context
            clContext = NULL;
            clCmdQueue = NULL;
            devices = NULL;
            maxDimensions = 0;
            maxWorkGroupSize = 0;
            maxWorkItemSizes = NULL;
            maxComputeUnits = 0;
            //extra options to recognize vendor specific fp64 extensions
            extra_options = NULL;
            ////debug information
            //cout << "Constructing a ClContext instance." << endl;

            //cl_int status;
            //cl_uint numPlatforms;
            //cl_platform_id platform = NULL;

            //openCLSafeCall(clGetPlatformIDs(0,NULL,&numPlatforms));
            //CV_DbgAssert(numPlatforms > 0);
            //cl_platform_id *platforms = new cl_platform_id[numPlatforms];
            //CV_DbgAssert(!!platforms);
            //openCLSafeCall(clGetPlatformIDs(numPlatforms,platforms,NULL));
            //char platformName[100];
            //for (unsigned i = 0; i < numPlatforms; ++i)
            //{
            //	openCLSafeCall(clGetPlatformInfo(platforms[i],
            //                                    CL_PLATFORM_VENDOR,
            //                                    sizeof(platformName),
            //                                    platformName,
            //                                    NULL));
            //	platform = platforms[i];
            //	if (!strcmp(platformName, "Advanced Micro Devices, Inc."))
            //	{
            //		break;
            //	}
            //}
            //delete [] platforms;
            //cl_context_properties cps[3] = {
            //	CL_CONTEXT_PLATFORM,(cl_context_properties)platform,0
            //};
            //clContext = clCreateContextFromType(
            //	cps,CL_DEVICE_TYPE_GPU,NULL,NULL,&status);
            //openCLVerifyCall(status);

            //// get the size of device list
            //size_t deviceListSize;
            //openCLSafeCall(clGetContextInfo(clContext,CL_CONTEXT_DEVICES,0,NULL,&deviceListSize));
            //cl_uint deviceCount = deviceListSize/sizeof(cl_device_id);
            //devices = new cl_device_id[deviceCount];
            //int deviceid = 0;
            ////get the device data
            //openCLSafeCall(clGetContextInfo(clContext,
            //	CL_CONTEXT_DEVICES,deviceListSize,devices,NULL));
            //for(unsigned i = 0; i < deviceCount; ++i)
            //{
            //	openCLSafeCall(clGetDeviceInfo(devices[i],
            //	CL_DEVICE_VENDOR,sizeof(platformName),platformName,NULL));
            //	deviceid = i;
            //	if (!strcmp(platformName, "Advanced Micro Devices, Inc."))
            //	{
            //		break;
            //	}
            //}
            ////create the command queue using the first device of the list
            //clCmdQueue = clCreateCommandQueue(clContext,devices[deviceid],
            //	CL_QUEUE_PROFILING_ENABLE,&status);
            //openCLVerifyCall(status);

            ////get device information
            //openCLSafeCall(clGetDeviceInfo(devices[deviceid],CL_DEVICE_MAX_WORK_GROUP_SIZE,
            //	sizeof(size_t),(void*)&maxWorkGroupSize,NULL));
            //openCLSafeCall(clGetDeviceInfo(devices[deviceid],CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS,
            //	sizeof(cl_uint),(void*)&maxDimensions,NULL));
            //maxWorkItemSizes = new size_t[maxDimensions];
            //openCLSafeCall(clGetDeviceInfo(devices[deviceid],CL_DEVICE_MAX_WORK_ITEM_SIZES,
            //	sizeof(size_t)*maxDimensions,(void*)maxWorkItemSizes,NULL));
            //openCLSafeCall(clGetDeviceInfo(devices[deviceid],CL_DEVICE_MAX_COMPUTE_UNITS,
            //	sizeof(cl_uint),(void*)&maxComputeUnits,NULL));
            ////initialize extra options for compilation. Currently only fp64 is included.
            ////Assume 4KB is enough to store all possible extensions.

            //const int EXT_LEN =4096+1 ;
            //char extends_set[EXT_LEN];
            //size_t extends_size;
            //openCLSafeCall(clGetDeviceInfo(devices[deviceid],CL_DEVICE_EXTENSIONS,
            //	EXT_LEN,(void*)extends_set,&extends_size));
            //CV_Assert(extends_size < EXT_LEN);
            //extends_set[EXT_LEN-1]=0;
            //extra_options = NULL;
            //int fp64_khr = string(extends_set).find("cl_khr_fp64");
            //int fp64_amd = string(extends_set).find("cl_amd_fp64");
            ////CV_Assert( !(fp64_nvi>=0 && fp64_nvi<EXT_LEN) || !(fp64_amd>=0 && fp64_amd<EXT_LEN) );
            //if(fp64_khr>=0 && fp64_khr<EXT_LEN)
            //{
            //	CV_Assert(!extra_options);
            //	extra_options = (char*)("-D __NVIDIA__ -D DOUBLE_SUPPORT");
            //}
            //else if(fp64_amd>=0 && fp64_amd<EXT_LEN)
            //{
            //	CV_Assert(!extra_options);
            //	extra_options = (char*)("-D __ATI__ -D DOUBLE_SUPPORT");
            //}

            //Initialize the program cache
            programCache = ProgramCache::getProgramCache();
        }

        ClContext::~ClContext()
        {
            //bookkeeping
            programCache->releaseProgram();
            //openCLSafeCall(clReleaseCommandQueue(clCmdQueue));
            //openCLSafeCall(clReleaseContext(clContext));
            //if(devices) delete [] devices;
            //if(maxWorkItemSizes) delete [] maxWorkItemSizes;
        }
        OCLInfo::OCLInfo()
        {
            oclplatform = 0;
            oclcontext = 0;
            clCmdQueue = 0;
            devnum = 0;
            maxDimensions = 0;
            maxWorkGroupSize = 0;
            maxWorkItemSizes = 0;
            maxComputeUnits = 0;
            //extra_options = 0;
        }
        void OCLInfo::release()
        {
            if(oclplatform)
            {
                oclplatform = 0;
            }
            if(clCmdQueue)
            {
                openCLSafeCall(clReleaseCommandQueue(clCmdQueue));
            }
            ProgramCache::getProgramCache()->releaseProgram();
            if(oclcontext)
            {
                openCLSafeCall(clReleaseContext(oclcontext));
            }
            if(maxWorkItemSizes)
            {
                delete[] maxWorkItemSizes;
                maxWorkItemSizes = 0;
            }
            //if(extra_options)
            //{
            //	delete[] extra_options;
            //	extra_options = 0;
            //}
            devices.empty();
            devName.empty();
        }
        OCLInfo::~OCLInfo()
        {
            release();
        }
        OCLInfo &OCLInfo::operator = (const OCLInfo &m)
        {
            oclplatform = m.oclplatform;
            oclcontext = m.oclcontext;
            clCmdQueue = m.clCmdQueue;
            devnum = m.devnum;
            maxDimensions = m.maxDimensions;
            maxWorkGroupSize = m.maxWorkGroupSize;
            maxWorkItemSizes = m.maxWorkItemSizes;
            maxComputeUnits = m.maxComputeUnits;
            memcpy(extra_options, m.extra_options, 512);
            for(int i = 0; i < m.devices.size(); i++)
            {
                devices.push_back(m.devices[i]);
                devName.push_back(m.devName[i]);
            }
            return *this;
        }
        OCLInfo::OCLInfo(const OCLInfo &m)
        {
            *this = m;
        }
    }//namespace ocl

}//namespace cv
#endif
