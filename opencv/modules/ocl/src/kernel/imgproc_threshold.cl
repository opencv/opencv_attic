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
//    Zhang Ying, zhangying913@gmail.com
//
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
// This software is provided by the copyright holders and contributors as is and
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

#if defined (__ATI__)
#pragma OPENCL EXTENSION cl_amd_fp64:enable
#elif defined (__NVIDIA__)
#pragma OPENCL EXTENSION cl_khr_fp64:enable
#endif

// threshold type:
// enum { THRESH_BINARY=0, THRESH_BINARY_INV=1, THRESH_TRUNC=2, THRESH_TOZERO=3,
//       THRESH_TOZERO_INV=4, THRESH_MASK=7, THRESH_OTSU=8 };

__kernel void threshold_C1_D0(__global const uchar * restrict src, __global uchar *dst, 
                              int src_offset, int src_step,
                              int dst_offset, int dst_rows, int dst_cols, int dst_step,
                              double thresh, double max_val, int thresh_type
                              )
{
    const int gx = get_global_id(0);
    const int gy = get_global_id(1);
    
    if(gx < dst_cols && gy < dst_rows)
    {
        uchar sdata = src[src_offset + gy * src_step + gx];
        uchar ddata;
        switch (thresh_type)
        {
            case 0:
                ddata = sdata > thresh ? max_val : 0;
                break;
            case 1:
                ddata = sdata > thresh ? 0 : max_val;
                break;
            case 2:
                ddata = sdata > thresh ? thresh : sdata;
                break;
            case 3:
                ddata = sdata > thresh ? sdata : 0;
                break;
            case 4:
                ddata = sdata > thresh ? 0 : sdata;
                break;
            default:
                ddata = sdata;
        }
        dst[dst_offset + gy * dst_step + gx] = ddata;
    }
}


__kernel void threshold_C1_D5(__global const float * restrict src, __global float *dst, 
                              int src_offset, int src_step,
                              int dst_offset, int dst_rows, int dst_cols, int dst_step,
                              double thresh, double max_val, int thresh_type
                              )
{
    const int gx = get_global_id(0);
    const int gy = get_global_id(1);
    
    if(gx < dst_cols && gy < dst_rows)
    {
        float sdata = src[src_offset/4 + gy * src_step/4 + gx];
        float ddata;
        switch (thresh_type)
        {
            case 0:
                ddata = sdata > thresh ? max_val : 0;
                break;
            case 1:
                ddata = sdata > thresh ? 0 : max_val;
                break;
            case 2:
                ddata = sdata > thresh ? thresh : sdata;
                break;
            case 3:
                ddata = sdata > thresh ? sdata : 0;
                break;
            case 4:
                ddata = sdata > thresh ? 0 : sdata;
                break;
            default:
                ddata = sdata;
        }
        dst[dst_offset/4 + gy * dst_step/4 + gx] = ddata;
    }
}

