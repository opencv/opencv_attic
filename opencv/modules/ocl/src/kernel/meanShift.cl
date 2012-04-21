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
//    Shengen Yan,yanshengen@gmail.com
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


short2 do_mean_shift(int x0, int y0, __global unsigned char* out,int out_step, 
               __global unsigned char * in, int in_step, int cols, int rows, 
                                int sp, int sr, int maxIter, float eps)
{
    int isr2 = sr*sr;
    int idx = y0 * in_step + x0 * 4;
    uchar4 c = (uchar4)(in[idx],in[idx+1],in[idx+2],in[idx+3]);

    // iterate meanshift procedure
    for( int iter = 0; iter < maxIter; iter++ )
    {
        int count = 0;
        int s0 = 0, s1 = 0, s2 = 0, sx = 0, sy = 0;
        float icount;

        //mean shift: process pixels in window (p-sigmaSp)x(p+sigmaSp)
        int minx = x0-sp;
        int miny = y0-sp;
        int maxx = x0+sp;
        int maxy = y0+sp;

        //deal with the image boundary
        if(minx<0) minx=0;
        if(miny<0) miny=0;
        if(maxx>=cols) maxx = cols-1;
        if(maxy>=rows) maxy = rows-1;

        for( int y = miny; y <= maxy; y++)
        {
            int rowCount = 0;
            for( int x = minx; x <= maxx; x++ )
            {                    
                //uchar4 t = tex2D( tex_meanshift, x, y );
                int idx = y * in_step + x * 4;
                uchar4 t = (uchar4)(in[idx],in[idx+1],in[idx+2],in[idx+3]);

                int norm2 = (t.x - c.x) * (t.x - c.x) + (t.y - c.y) * (t.y - c.y) + (t.z - c.z) * (t.z - c.z);
                if( norm2 <= isr2 )
                {
                    s0 += t.x; s1 += t.y; s2 += t.z;
                    sx += x; rowCount++;
                }
            }
            count += rowCount;
            sy += y*rowCount;
        }

        if( count == 0 )
            break;

        icount = 1.f/count;
        int x1 = convert_int_rtz(sx*icount);
        int y1 = convert_int_rtz(sy*icount);
        s0 = convert_int_rtz(s0*icount);
        s1 = convert_int_rtz(s1*icount);
        s2 = convert_int_rtz(s2*icount);

        int norm2 = (s0 - c.x) * (s0 - c.x) + (s1 - c.y) * (s1 - c.y) + (s2 - c.z) * (s2 - c.z);

        bool stopFlag = (x0 == x1 && y0 == y1) || (abs(x1-x0) + abs(y1-y0) + norm2 <= eps);

        x0 = x1; y0 = y1;
        c.x = s0; c.y = s1; c.z = s2;

        if( stopFlag )
            break;
    }

    int base = get_global_id(1)*out_step + get_global_id(0) * 4;
    //FIXME!!! It seems OpenCL does not support complex pointer arithmetic.
    //FIXME!!! As a result, we have to do it in a tedious way as follows:
    //*(uchar4*)(out + base) = c;
    out[base] = c.x;
    out[base+1] = c.y;
    out[base+2] = c.z;
    out[base+3] = c.w;

    return (short2)((short)x0, (short)y0);
}

__kernel void meanshift_kernel(__global unsigned char* out, int out_step, 
                               __global unsigned char * in, int in_step, 
                        int cols, int rows,int sp, int sr, int maxIter, float eps )
{
    int x0 = get_global_id(0); 
    int y0 = get_global_id(1); 

    if( x0 < cols && y0 < rows )
        do_mean_shift(x0, y0, out, out_step, in, in_step, cols, rows, sp, sr, maxIter, eps);
}

__kernel void meanshiftproc_kernel( __global unsigned char * in, __global unsigned char* outr, 
                                    __global short2 *outsp, int instep, int outrstep, 
                                    int outspstep, int cols, int rows, 
                                    int sp, int sr, int maxIter, float eps )
{
    int x0 = get_global_id(0); 
    int y0 = get_global_id(1); 

    if( x0 < cols && y0 < rows )
    {
        //int basesp = (blockIdx.y * blockDim.y + threadIdx.y) * outspstep + (blockIdx.x * blockDim.x + threadIdx.x) * 2 * sizeof(short);
        //*(short2*)(outsp + basesp) = do_mean_shift(x0, y0, outr, outrstep, cols, rows, sp, sr, maxIter, eps);
        // we have ensured before that ((outspstep & 0x11)==0).
        int basesp = y0 * (outspstep >> 2) + x0;
        outsp[basesp] = do_mean_shift(x0, y0, outr, outrstep, in, instep, cols, rows, sp, sr, maxIter, eps);
    }
}

