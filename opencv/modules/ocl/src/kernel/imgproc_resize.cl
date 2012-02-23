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
//     and/or other GpuMaterials provided with the distribution.
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


// resize kernel 
// Currently, CV_8UC1  CV_8UC4  CV_32FC1 and CV_32FC4are supported.
// We shall support other types later if necessary.

//this round operation is to approximate CPU's saturate_cast<int>
#define EPS 0.000001f
inline int float2int(float v)
{
    int v1=floor(v);
    int v2= floor(v+(v>=0 ? 0.5f : -0.5f));
    return fabs(v-v1-0.5f) < EPS &&((v1&1)==0) ? v1 : v2;
}


inline int getPoint(__global unsigned char * data, int offset, int x, int y, int step)
{
    return (data[offset+ y * step + x]);
}

inline uint4 getPoint_8uc4(__global uchar4 * data, int offset, int x, int y, int step)
{
    return convert_uint4(data[(offset>>2)+ y * (step>>2) + x]);
}

inline float getPoint_32fc1(__global float * data, int offset, int x, int y, int step)
{
    return data[(offset>>2)+ y * (step>>2) + x];
}

inline float4 getPoint_32fc4(__global float4 * data, int offset, int x, int y, int step)
{
    return data[(offset>>4)+ y * (step>>4) + x];
}

#define INTER_RESIZE_COEF_BITS 11
#define INTER_RESIZE_COEF_SCALE (1 << INTER_RESIZE_COEF_BITS)
#define CAST_BITS (INTER_RESIZE_COEF_BITS << 1)
#define CAST_SCALE (1.0f/(1<<CAST_BITS))
#define INC(x,l) ((x+1) >= (l) ? (x):((x)+1))

__kernel void resizeLN_C1_D0(__global unsigned char * dst, __global unsigned char * src,
                     int dst_offset, int src_offset,int dst_step, int src_step, 
                     int src_cols, int src_rows, int dst_cols, int dst_rows, float ifx, float ify )
{
    int gx = get_global_id(0);
    int dy = get_global_id(1);
   
    gx = (gx<<2) - (dst_offset&3);
    float4 sx;
    sx.s0 = ((gx+0.5f) * ifx - 0.5f);
    sx.s1 = ((gx+1+0.5f) * ifx - 0.5f);
    sx.s2 = ((gx+2+0.5f) * ifx - 0.5f);
    sx.s3 = ((gx+3+0.5f) * ifx - 0.5f);
    float sy = ((dy+0.5) * ify - 0.5f);
    int4 x;
    x.s0 = floor(sx.s0);
    x.s1 = floor(sx.s1);
    x.s2 = floor(sx.s2);
    x.s3 = floor(sx.s3);
    int y = floor(sy);
    float4 u;
    u.s0 = sx.s0 - x.s0;
    u.s1 = sx.s1 - x.s1;
    u.s2 = sx.s2 - x.s2;
    u.s3 = sx.s3 - x.s3;
    float v = sy - y;

    x.s0<0 ? x.s0=0,u.s0=0 : x.s0,u.s0;
    x.s0>=src_cols ? x.s0=src_cols-1,u.s0=0 : x.s0,u.s0;
    x.s1<0 ? x.s1=0,u.s1=0 : x.s1,u.s1;
    x.s1>=src_cols ? x.s1=src_cols-1,u.s1=0 : x.s1,u.s1;
    x.s2<0 ? x.s2=0,u.s2=0 : x.s2,u.s2;
    x.s2>=src_cols ? x.s2=src_cols-1,u.s2=0 : x.s2,u.s2;
    x.s3<0 ? x.s3=0,u.s3=0 : x.s3,u.s3;
    x.s3>=src_cols ? x.s3=src_cols-1,u.s3=0 : x.s3,u.s3;
    y<0 ? y=0,v=0 : y,v;
    y>=src_rows ? y=src_rows-1,v=0 : y,v;
    
    int4 U, U1;
    int V, V1;
    U.s0 = float2int(u.s0 * INTER_RESIZE_COEF_SCALE);
    U.s1 = float2int(u.s1 * INTER_RESIZE_COEF_SCALE);
    U.s2 = float2int(u.s2 * INTER_RESIZE_COEF_SCALE);
    U.s3 = float2int(u.s3 * INTER_RESIZE_COEF_SCALE);
    V = float2int(v * INTER_RESIZE_COEF_SCALE);
    U1.s0= float2int((1-u.s0)*INTER_RESIZE_COEF_SCALE);
    U1.s1= float2int((1-u.s1)*INTER_RESIZE_COEF_SCALE);
    U1.s2= float2int((1-u.s2)*INTER_RESIZE_COEF_SCALE);
    U1.s3= float2int((1-u.s3)*INTER_RESIZE_COEF_SCALE);
    V1= float2int((1-v)*INTER_RESIZE_COEF_SCALE);

    int y_ = INC(y,src_rows);
    int4 x_;
    x_.s0 = INC(x.s0,src_cols);
    x_.s1 = INC(x.s1,src_cols);
    x_.s2 = INC(x.s2,src_cols);
    x_.s3 = INC(x.s3,src_cols);


    int4 val1, val2, val;
    val1.s0 = U1.s0 *  getPoint(src,src_offset,x.s0,y,src_step) +
           U.s0 *  getPoint(src,src_offset,x_.s0,y,src_step) ;
    val1.s1 = U1.s1 *  getPoint(src,src_offset,x.s1,y,src_step) +
           U.s1 *  getPoint(src,src_offset,x_.s1,y,src_step) ;
    val1.s2 = U1.s2 *  getPoint(src,src_offset,x.s2,y,src_step) +
           U.s2 *  getPoint(src,src_offset,x_.s2,y,src_step) ;
    val1.s3 = U1.s3 *  getPoint(src,src_offset,x.s3,y,src_step) +
           U.s3 *  getPoint(src,src_offset,x_.s3,y,src_step) ;
    val2.s0 = U1.s0 *  getPoint(src,src_offset,x.s0,y_,src_step) +
           U.s0 *  getPoint(src,src_offset,x_.s0,y_,src_step);
    val2.s1 = U1.s1 *  getPoint(src,src_offset,x.s1,y_,src_step) +
           U.s1 *  getPoint(src,src_offset,x_.s1,y_,src_step);
    val2.s2 = U1.s2 *  getPoint(src,src_offset,x.s2,y_,src_step) +
           U.s2 *  getPoint(src,src_offset,x_.s2,y_,src_step);
    val2.s3 = U1.s3 *  getPoint(src,src_offset,x.s3,y_,src_step) +
           U.s3 *  getPoint(src,src_offset,x_.s3,y_,src_step);
    val = V1 * val1 + V * val2;
    
    __global uchar4* d = (__global uchar4*)(dst + dst_offset + dy * dst_step + gx);
    uchar4 dVal = *d;
    uchar4 value;
    value.s0 = (gx>=0 && gx<dst_cols && dy>=0 && dy<dst_rows) ? (val.s0 + (1<<(CAST_BITS-1)))*CAST_SCALE: dVal.s0;
    value.s1 = (gx+1>=0 && gx+1<dst_cols && dy>=0 && dy<dst_rows) ? (val.s1 + (1<<(CAST_BITS-1)))*CAST_SCALE: dVal.s1;
    value.s2 = (gx+2>=0 && gx+2<dst_cols && dy>=0 && dy<dst_rows) ? (val.s2 + (1<<(CAST_BITS-1)))*CAST_SCALE : dVal.s2;
    value.s3 = (gx+3>=0 && gx+3<dst_cols && dy>=0 && dy<dst_rows) ? (val.s3 + (1<<(CAST_BITS-1)))*CAST_SCALE : dVal.s3;
    *d = value;
}

__kernel void resizeLN_C4_D0(__global uchar4 * dst, __global uchar4 * src,
                     int dst_offset, int src_offset,int dst_step, int src_step, 
                     int src_cols, int src_rows, int dst_cols, int dst_rows, float ifx, float ify )
{
    int dx = get_global_id(0);
    int dy = get_global_id(1);

    float sx = ((dx+0.5f) * ifx - 0.5f), sy = ((dy+0.5) * ify - 0.5f);
    int x = floor(sx), y = floor(sy);
    float u = sx - x, v = sy - y;

    x<0 ? x=0,u=0 : x,u;
    x>=src_cols ? x=src_cols-1,u=0 : x,u;
    y<0 ? y=0,v=0 : y,v;
    y>=src_rows ? y=src_rows-1,v=0 : y,v;
    
    
    int U = float2int(u * INTER_RESIZE_COEF_SCALE);
    int V = float2int(v * INTER_RESIZE_COEF_SCALE);
    int U1= float2int((1-u)*INTER_RESIZE_COEF_SCALE);
    int V1= float2int((1-v)*INTER_RESIZE_COEF_SCALE);

    int y_ = INC(y,src_rows);
    int x_ = INC(x,src_cols);
      
    uint4 val = U1* V1 *  getPoint_8uc4(src,src_offset,x,y,src_step) +
               U1* V  *  getPoint_8uc4(src,src_offset,x,y_,src_step) +
               U * V1 *  getPoint_8uc4(src,src_offset,x_,y,src_step) +
               U * V  *  getPoint_8uc4(src,src_offset,x_,y_,src_step);
               
    if(dx>=0 && dx<dst_cols && dy>=0 && dy<dst_rows)
         dst[(dst_offset>>2) + dy * (dst_step>>2) + dx] = convert_uchar4((val + (1<<(CAST_BITS-1)))/(1<<CAST_BITS));
}

__kernel void resizeLN_C1_D5(__global float * dst, __global float * src,
                     int dst_offset, int src_offset,int dst_step, int src_step, 
                     int src_cols, int src_rows, int dst_cols, int dst_rows, float ifx, float ify )
{
    int dx = get_global_id(0);
    int dy = get_global_id(1);

    float sx = ((dx+0.5f) * ifx - 0.5f), sy = ((dy+0.5) * ify - 0.5f);
    int x = floor(sx), y = floor(sy);
    float u = sx - x, v = sy - y;

    x<0 ? x=0,u=0 : x,u;
    x>=src_cols ? x=src_cols-1,u=0 : x,u;
    y<0 ? y=0,v=0 : y,v;
    y>=src_rows ? y=src_rows-1,v=0 : y,v;
    
    int y_ = INC(y,src_rows);
    int x_ = INC(x,src_cols);

    float val1 = (1.0f-u) *  getPoint_32fc1(src,src_offset,x,y,src_step) +
                u  *  getPoint_32fc1(src,src_offset,x_,y,src_step) ;
    float val2 = (1.0f-u) *  getPoint_32fc1(src,src_offset,x,y_,src_step) +
                u *  getPoint_32fc1(src,src_offset,x_,y_,src_step);
    float val = (1.0f-v) * val1 + v * val2;

    if(dx>=0 && dx<dst_cols && dy>=0 && dy<dst_rows)
         dst[(dst_offset>>2) + dy * (dst_step>>2) + dx] = val; 
}

__kernel void resizeLN_C4_D5(__global float4 * dst, __global float4 * src,
                     int dst_offset, int src_offset,int dst_step, int src_step, 
                     int src_cols, int src_rows, int dst_cols, int dst_rows, float ifx, float ify )
{
    int dx = get_global_id(0);
    int dy = get_global_id(1);

    float sx = ((dx+0.5f) * ifx - 0.5f), sy = ((dy+0.5) * ify - 0.5f);
    int x = floor(sx), y = floor(sy);
    float u = sx - x, v = sy - y;

    x<0 ? x=0,u=0 : x,u;
    x>=src_cols ? x=src_cols-1,u=0 : x,u;
    y<0 ? y=0,v=0 : y,v;
    y>=src_rows ? y=src_rows-1,v=0 : y,v;
    
    int y_ = INC(y,src_rows);
    int x_ = INC(x,src_cols);

    float4 val1 = (1.0f-u) *  getPoint_32fc4(src,src_offset,x,y,src_step) +
                u  *  getPoint_32fc4(src,src_offset,x_,y,src_step) ;
    float4 val2 = (1.0f-u) *  getPoint_32fc4(src,src_offset,x,y_,src_step) +
                u *  getPoint_32fc4(src,src_offset,x_,y_,src_step);
    float4 val = (1.0f-v) * val1 + v * val2;

    if(dx>=0 && dx<dst_cols && dy>=0 && dy<dst_rows)
         dst[(dst_offset>>4) + dy * (dst_step>>4) + dx] = val; 
}
#if defined DOUBLE_SUPPORT
#if defined (__ATI__)
#pragma OPENCL EXTENSION cl_amd_fp64:enable
#elif defined (__NVIDIA__)
#pragma OPENCL EXTENSION cl_khr_fp64:enable
#endif
inline int double2int(double v)
{
    int v1=floor(v);
    int v2= floor(v+(v>=0 ? 0.5f : -0.5f));
    return (v-v1)==0.5f&&((v1&1)==0) ? v1 : v2;
}

__kernel void resizeNN_C1_D0(__global uchar * dst, __global uchar * src,
                     int dst_offset, int src_offset,int dst_step, int src_step, 
                     int src_cols, int src_rows, int dst_cols, int dst_rows, double ifx, double ify )
{
    int gx = get_global_id(0);
    int dy = get_global_id(1);
    
    gx = (gx<<2) - (dst_offset&3);
    
    int4 sx;
    int sy;
   
    double ss1 = gx*ifx;
    double ss2 = (gx+1)*ifx;
    double ss3 = (gx+2)*ifx;
    double ss4 = (gx+3)*ifx;
    double s5 = dy * ify;
    sx.s0 = min(double2int(ss1), src_cols-1);
    sx.s1 = min(double2int(ss2), src_cols-1);
    sx.s2 = min(double2int(ss3), src_cols-1);
    sx.s3 = min(double2int(ss4), src_cols-1);
    sy = min(double2int(s5), src_rows-1);
    
    uchar4 val;
    val.s0 = src[src_offset + sy * src_step + sx.s0];
    val.s1 = src[src_offset + sy * src_step + sx.s1];
    val.s2 = src[src_offset + sy * src_step + sx.s2];
    val.s3 = src[src_offset + sy * src_step + sx.s3];
    
    __global uchar4* d = (__global uchar4*)(dst + dst_offset + dy * dst_step + gx);
    uchar4 dVal = *d;
    val.s0 = (gx>=0 && gx<dst_cols && dy>=0 && dy<dst_rows) ? val.s0 : dVal.s0;
    val.s1 = (gx+1>=0 && gx+1<dst_cols && dy>=0 && dy<dst_rows) ? val.s1 : dVal.s1;
    val.s2 = (gx+2>=0 && gx+2<dst_cols && dy>=0 && dy<dst_rows) ? val.s2 : dVal.s2;
    val.s3 = (gx+3>=0 && gx+3<dst_cols && dy>=0 && dy<dst_rows) ? val.s3 : dVal.s3;
    *d = val;
}

__kernel void resizeNN_C4_D0(__global uchar4 * dst, __global uchar4 * src,
                     int dst_offset, int src_offset,int dst_step, int src_step, 
                     int src_cols, int src_rows, int dst_cols, int dst_rows, double ifx, double ify )
{
    int dx = get_global_id(0);
    int dy = get_global_id(1);
    
    double s1 = dx*ifx;
    double s2 = dy*ify;
    int sx = min(double2int(s1), src_cols-1);
    int dpos = (dst_offset>>2) + dy * (dst_step>>2) + dx;
    int sy = min(double2int(s2), src_rows-1);
    int spos = (src_offset>>2) + sy * (src_step>>2) + sx;
    
    if(dx>=0 && dx<dst_cols && dy>=0 && dy<dst_rows)
        dst[dpos] = src[spos];
   
}

__kernel void resizeNN_C1_D5(__global float * dst, __global float * src,
                     int dst_offset, int src_offset,int dst_step, int src_step, 
                     int src_cols, int src_rows, int dst_cols, int dst_rows, double ifx, double ify )
{
    int dx = get_global_id(0);
    int dy = get_global_id(1);
    
    double s1 = dx*ifx;
    double s2 = dy*ify;
    int sx = min(double2int(s1), src_cols-1);
    int dpos = (dst_offset>>2) + dy * (dst_step>>2) + dx;
    int sy = min(double2int(s2), src_rows-1);
    int spos = (src_offset>>2) + sy * (src_step>>2) + sx;
    
    if(dx>=0 && dx<dst_cols && dy>=0 && dy<dst_rows)
        dst[dpos] = src[spos];
   
}

__kernel void resizeNN_C4_D5(__global float4 * dst, __global float4 * src,
                     int dst_offset, int src_offset,int dst_step, int src_step, 
                     int src_cols, int src_rows, int dst_cols, int dst_rows, double ifx, double ify )
{
    int dx = get_global_id(0);
    int dy = get_global_id(1);
    double s1 = dx*ifx;
    double s2 = dy*ify;
    int sx = min(double2int(s1), src_cols-1);
    int dpos = (dst_offset>>4) + dy * (dst_step>>4) + dx;
    int sy = min(double2int(s2), src_rows-1);
    int spos = (src_offset>>4) + sy * (src_step>>4) + sx;
    
    if(dx>=0 && dx<dst_cols && dy>=0 && dy<dst_rows)
        dst[dpos] = src[spos];
   
}
#endif

