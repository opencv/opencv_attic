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
//   * Redistribution's in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other GpuMaterials provided with the distribution.
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

#ifndef __OPENCV_GPU_DEVMEM2D_HPP__
#define __OPENCV_GPU_DEVMEM2D_HPP__

namespace cv
{    
    namespace gpu
    {
        // Simple lightweight structures that encapsulates information about an image on device.
        // It is intended to pass to nvcc-compiled code. GpuMat depends on headers that nvcc can't compile

        template<typename T> struct PtrStep_
        {
            T* ptr;
            size_t step;

            typedef T elem_type;
            enum { elem_size = sizeof(elem_type) };

#if defined(__CUDACC__) 
            __host__ __device__ 
#endif
            size_t elemSize() const { return elem_size; }            
        };
        
        template <typename T> struct DevMem2D_
        {            
            int cols;
            int rows;
            T* ptr;
            size_t step;
            size_t elem_step;

            /*__host__*/
            DevMem2D_() : cols(0), rows(0), ptr(0), step(0), elem_step(0) {}

            /*__host__*/
            DevMem2D_(int rows_, int cols_, T *ptr_, size_t step_)
                : cols(cols_), rows(rows_), ptr(ptr_), step(step_), elem_step(step_ / sizeof(T)) {}
            
            template <typename U>
            /*__host__*/
            explicit DevMem2D_(const DevMem2D_<U>& d)
                : cols(d.cols), rows(d.rows), ptr((T*)d.ptr), step(d.step), elem_step(d.step / sizeof(T)) {}

            template <typename U>
            /*__host__*/
            operator PtrStep_<U>() const { PtrStep_<U> dt; dt.ptr = ptr; dt.step = step; return dt; }

            typedef typename PtrStep_<T>::elem_type elem_type;
            enum { elem_size = PtrStep_<T>::elem_size };
#if defined(__CUDACC__) 
            __host__ __device__ 
#endif
            size_t elemSize() const { return elem_size; }
        };

        typedef DevMem2D_<unsigned char> DevMem2D;
        typedef DevMem2D_<float> DevMem2Df;
        typedef DevMem2D_<int> DevMem2Di;
    }
}

#endif /* __OPENCV_GPU_DEVMEM2D_HPP__ */
