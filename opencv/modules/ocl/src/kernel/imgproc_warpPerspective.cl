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


//wrapPerspective kernel
//support data types: CV_8UC1, CV_8UC4, CV_32FC1, CV_32FC4, and three interpolation methods: NN, Linear, Cubic.

#if defined (__ATI__)
#pragma OPENCL EXTENSION cl_amd_fp64:enable
#elif defined (__NVIDIA__)
#pragma OPENCL EXTENSION cl_khr_fp64:enable
#endif

typedef double F;
#define INTER_BITS 5
#define INTER_TAB_SIZE (1 << INTER_BITS)
#define AB_BITS max(10, (int)INTER_BITS) 
#define AB_SCALE (1 << AB_BITS) 
#define INTER_REMAP_COEF_BITS 15
#define INTER_REMAP_COEF_SCALE (1 << INTER_REMAP_COEF_BITS)

inline void interpolateCubic( float x, float* coeffs )
{
    const float A = -0.75f;

    coeffs[0] = ((A*(x + 1.f) - 5.0f*A)*(x + 1.f) + 8.0f*A)*(x + 1.f) - 4.0f*A;
    coeffs[1] = ((A + 2.f)*x - (A + 3.f))*x*x + 1.f;
    coeffs[2] = ((A + 2.f)*(1.f - x) - (A + 3.f))*(1.f - x)*(1.f - x) + 1.f;
    coeffs[3] = 1.f - coeffs[0] - coeffs[1] - coeffs[2];
}


/**********************************************8UC1*********************************************
***********************************************************************************************/
__kernel void warpPerspectiveNN_C1_D0(__global uchar const * restrict src, __global uchar * dst, int src_cols, int src_rows,
                            int dst_cols, int dst_rows, int srcStep, int dstStep, 
                            int src_offset, int dst_offset,  __constant F * M )
{
    int dx = get_global_id(0);
    int dy = get_global_id(1);
    dx = (dx<<2) - (dst_offset&3);
    
    double4 DX = (double4)(dx, dx+1, dx+2, dx+3);
    double4 X0 = M[0]*DX + M[1]*dy + M[2];
    double4 Y0 = M[3]*DX + M[4]*dy + M[5];
    double4 W = M[6]*DX + M[7]*dy + M[8];
    W = (W!=0) ? 1./W : 0;
    short4 X = convert_short4(rint(X0*W));
    short4 Y = convert_short4(rint(Y0*W));
    int4 sx = convert_int4(X);
    int4 sy = convert_int4(Y);

    int4 DXD = (int4)(dx, dx+1, dx+2, dx+3);
    __global uchar4 * d = (__global uchar4 *)(dst+dst_offset+dy*dstStep+dx);
    uchar4 dval = *d;
    int4 dcon = DXD >= 0 && DXD < dst_cols && dy >= 0 && dy < dst_rows;
    int4 scon = sx >= 0 && sx < src_cols && sy >= 0 && sy < src_rows;
    int4 spos = src_offset + sy * srcStep + sx;
    uchar4 sval;
    sval.s0 = scon.s0 ? src[spos.s0] : 0;
    sval.s1 = scon.s1 ? src[spos.s1] : 0;
    sval.s2 = scon.s2 ? src[spos.s2] : 0;
    sval.s3 = scon.s3 ? src[spos.s3] : 0;
    dval = convert_uchar4(dcon != 0) ? sval : dval;
    *d = dval;

}

__kernel void warpPerspectiveLinear_C1_D0(__global const uchar * restrict src, __global uchar * dst,
                            int src_cols, int src_rows, int dst_cols, int dst_rows, int srcStep, 
                            int dstStep, int src_offset, int dst_offset,  __constant F * M )
{
    int dx = get_global_id(0);
    int dy = get_global_id(1);
 
    F X0 = M[0]*dx + M[1]*dy + M[2];
    F Y0 = M[3]*dx + M[4]*dy + M[5];
    F W = M[6]*dx + M[7]*dy + M[8];
    W = W ? INTER_TAB_SIZE/W : 0;
    int X = rint(X0*W);
    int Y = rint(Y0*W);
    
    int sx = (short)(X >> INTER_BITS);
    int sy = (short)(Y >> INTER_BITS);
    int ay = (short)(Y & (INTER_TAB_SIZE-1));
    int ax = (short)(X & (INTER_TAB_SIZE-1));
   
    uchar v[4];
    int i, j;
#pragma unroll 4
    for(i=0; i<4;  i++)
       v[i] = (sx+(i&1) >= 0 && sx+(i&1) < src_cols && sy+(i>>1) >= 0 && sy+(i>>1) < src_rows) ? src[src_offset + (sy+(i>>1)) * srcStep + (sx+(i&1))] : 0;

    short itab[4];
    float tab1y[2], tab1x[2];
    tab1y[0] = 1.0 - 1.f/INTER_TAB_SIZE*ay;
    tab1y[1] = 1.f/INTER_TAB_SIZE*ay;
    tab1x[0] = 1.0 - 1.f/INTER_TAB_SIZE*ax;
    tab1x[1] = 1.f/INTER_TAB_SIZE*ax;
    
#pragma unroll 4
    for(i=0; i<4;  i++)
    {
        float v = tab1y[(i>>1)] * tab1x[(i&1)];
        itab[i] = convert_short_sat(rint( v * INTER_REMAP_COEF_SCALE ));
    }
    if(dx >=0 && dx < dst_cols && dy >= 0 && dy < dst_rows)
    {
        int sum = 0;
        for ( i =0; i<4; i++ )
        {
            sum += v[i] * itab[i] ;
        }
        dst[dst_offset+dy*dstStep+dx] = convert_uchar_sat ( (sum + (1 << (INTER_REMAP_COEF_BITS-1))) >> INTER_REMAP_COEF_BITS ) ;
    }
}

/*
__kernel void warpPerspectiveLinear_C1_D0(__global const uchar * restrict src, __global uchar * dst,
                            int src_cols, int src_rows, int dst_cols, int dst_rows, int srcStep, 
                            int dstStep, int src_offset, int dst_offset,  __constant F * M )
{
    int dx = get_global_id(0);
    int dy = get_global_id(1);
    dx = (dx<<2) - (dst_offset&3);
    double4 DX = (double4)(dx, dx+1, dx+2, dx+3);
    float inter_tab_size = INTER_TAB_SIZE;

    double M12 = M[1] * dy + M[2];
    double M45 = M[4]*dy + M[5];
    double M78 = M[7]*dy + M[8];
    
    double4 X0 = M[0] *DX + M12;
    double4 Y0 = M[3] *DX + M45;
    double4 W = M[6] *DX + M78;
    W = (W!=0) ? inter_tab_size/W : 0;
    int4 X = convert_int4(rint(X0 * W));
    int4 Y = convert_int4(rint(Y0 * W));
    
    int4 sx = convert_int4(convert_short4(X >> INTER_BITS));
    int4 sy = convert_int4(convert_short4(Y >> INTER_BITS));
    short4 ay = convert_short4(Y & (INTER_TAB_SIZE-1));
    short4 ax = convert_short4(X & (INTER_TAB_SIZE-1));
    
    
    uchar4 v0, v1, v2,v3;
    int4 scon0, scon1, scon2, scon3;
    int4 spos0, spos1, spos2, spos3;

    scon0 = (sx >= 0 && sx < src_cols && sy >= 0 && sy < src_rows);
    scon1 = (sx+1 >= 0 && sx+1 < src_cols && sy >= 0 && sy < src_rows);
    scon2 = (sx >= 0 && sx < src_cols && sy+1 >= 0 && sy+1 < src_rows);
    scon3 = (sx+1 >= 0 && sx+1 < src_cols && sy+1 >= 0 && sy+1 < src_rows);
    spos0 = src_offset + sy * srcStep + sx;
    spos1 = src_offset + sy * srcStep + sx + 1;
    spos2 = src_offset + (sy+1) * srcStep + sx;
    spos3 = src_offset + (sy+1) * srcStep + sx + 1;

    v0.s0 = scon0.s0 ? src[spos0.s0] : 0;
    v1.s0 = scon1.s0 ? src[spos1.s0] : 0;
    v2.s0 = scon2.s0 ? src[spos2.s0] : 0;
    v3.s0 = scon3.s0 ? src[spos3.s0] : 0;

    v0.s1 = scon0.s1 ? src[spos0.s1] : 0;
    v1.s1 = scon1.s1 ? src[spos1.s1] : 0;
    v2.s1 = scon2.s1 ? src[spos2.s1] : 0;
    v3.s1 = scon3.s1 ? src[spos3.s1] : 0;

    v0.s2 = scon0.s2 ? src[spos0.s2] : 0;
    v1.s2 = scon1.s2 ? src[spos1.s2] : 0;
    v2.s2 = scon2.s2 ? src[spos2.s2] : 0;
    v3.s2 = scon3.s2 ? src[spos3.s2] : 0;

    v0.s3 = scon0.s3 ? src[spos0.s3] : 0;
    v1.s3 = scon1.s3 ? src[spos1.s3] : 0;
    v2.s3 = scon2.s3 ? src[spos2.s3] : 0;
    v3.s3 = scon3.s3 ? src[spos3.s3] : 0;
 
    short4 itab0, itab1, itab2, itab3;
    float4 taby, tabx;
    taby = 1.f/inter_tab_size * convert_float4(ay);
    tabx = 1.f/inter_tab_size * convert_float4(ax);
 
    itab0 = convert_short4_sat(( (1.0f-taby)*(1.0f-tabx) * INTER_REMAP_COEF_SCALE ));
    itab1 = convert_short4_sat(( (1.0f-taby)*tabx * INTER_REMAP_COEF_SCALE ));
    itab2 = convert_short4_sat(( taby*(1.0f-tabx) * INTER_REMAP_COEF_SCALE ));
    itab3 = convert_short4_sat(( taby*tabx * INTER_REMAP_COEF_SCALE ));


    int4 val;
    uchar4 tval;
    val = convert_int4(v0) * convert_int4(itab0) + convert_int4(v1) * convert_int4(itab1) 
        + convert_int4(v2) * convert_int4(itab2) + convert_int4(v3) * convert_int4(itab3);
    tval = convert_uchar4_sat ( (val + (1 << (INTER_REMAP_COEF_BITS-1))) >> INTER_REMAP_COEF_BITS ) ;
    
    __global uchar4 * d =(__global uchar4 *)(dst+dst_offset+dy*dstStep+dx);
    uchar4 dval = *d;
    int4 DXD = (int4)(dx, dx+1, dx+2, dx+3);
    int4 dcon = DXD >= 0 && DXD < dst_cols && dy >= 0 && dy < dst_rows;
    dval = convert_uchar4(dcon != 0) ? tval : dval;
    *d = dval;
}
*/
/*
__kernel void warpPerspectiveNN_C1_D0(__global uchar const * restrict src, __global uchar * dst, int src_cols, int src_rows,
                            int dst_cols, int dst_rows, int srcStep, int dstStep, 
                            int src_offset, int dst_offset,  __constant F * M )
{
    int dx = get_global_id(0);
    int dy = get_global_id(1);
    
    double X0 = M[0]*dx + M[1]*dy + M[2];
    double Y0 = M[3]*dx + M[4]*dy + M[5];
    double W = M[6]*dx + M[7]*dy + M[8];
    W = W ? 1./W : 0;
    int X = rint(X0*W);
    int Y = rint(Y0*W);
    short sx = (short)X;
    short sy = (short)Y;
    if(dx >= 0 && dx < dst_cols && dy >= 0 && dy <dst_rows)
        dst[dst_offset+dy*dstStep+dx] = (sx >= 0 && sx < src_cols && sy >= 0 && sy < src_rows) ? src[src_offset+sy*srcStep+sx] : 0; 
}
*/
/*
__kernel void warpPerspective_8u_Linear(__global uchar * src, __global uchar * dst, int cols, int rows,  int cn,
                            int srcStep, int dstStep, __global double * M, int interpolation)
{
    int dx = get_global_id(0);
    int dy = get_global_id(1);
    
    double X0 = M[0]*dx + M[1]*dy + M[2];
    double Y0 = M[3]*dx + M[4]*dy + M[5];
    double W = M[6]*dx + M[7]*dy + M[8];
    W = W ? INTER_TAB_SIZE/W : 0;
    int X = rint(X0*W);
    int Y = rint(Y0*W);
    
    short sx = (short)(X >> INTER_BITS);
    short sy = (short)(Y >> INTER_BITS);
    short ay = (short)(Y & (INTER_TAB_SIZE-1));
    short ax = (short)(X & (INTER_TAB_SIZE-1));
    
    uchar v[16];
    int i, j, c;

    for(i=0; i<2;  i++)
        for(j=0; j<2; j++)
            for(c=0; c<cn; c++)
                v[i*2*cn + j*cn + c] = (sx+j >= 0 && sx+j < cols && sy+i >= 0 && sy+i < rows) ? src[(sy+i) * srcStep + (sx+j)*cn + c] : 0;
   
    short itab[4];
    float tab1y[2], tab1x[2];
    tab1y[0] = 1.0 - 1.f/INTER_TAB_SIZE*ay;
    tab1y[1] = 1.f/INTER_TAB_SIZE*ay;
    tab1x[0] = 1.0 - 1.f/INTER_TAB_SIZE*ax;
    tab1x[1] = 1.f/INTER_TAB_SIZE*ax;
    
    for( i=0; i<2; i++ )
    {
        for( j=0; j<2; j++)
        {
            float v = tab1y[i] * tab1x[j];
            itab[i*2+j] = convert_short_sat(rint( v * INTER_REMAP_COEF_SCALE ));
        }
    }
    if( sx+1 < 0 || sx >= cols || sy+1 < 0 || sy >= rows)
    {
        for(c = 0; c < cn; c++)
            dst[dy*dstStep+dx*cn+c] = 0;
    }
    else
    {
        int sum;
        for(c = 0; c < cn; c++)
        {
            sum = 0;
            for ( i =0; i<4; i++ )
            {
                sum += v[i*cn+c] * itab[i] ;
            }
            dst[dy*dstStep+dx*cn+c] = convert_uchar_sat ( rint(sum + (1 << (INTER_REMAP_COEF_BITS-1))) >> INTER_REMAP_COEF_BITS ) ;
        }
    }
}

__kernel void warpPerspective_8u_Cubic(__global uchar * src, __global uchar * dst, int cols, int rows,  int cn,
                            int srcStep, int dstStep, __global double * M, int interpolation)
{
    int dx = get_global_id(0);
    int dy = get_global_id(1);
    
    double X0 = M[0]*dx + M[1]*dy + M[2];
    double Y0 = M[3]*dx + M[4]*dy + M[5];
    double W = M[6]*dx + M[7]*dy + M[8];
    W = W ? INTER_TAB_SIZE/W : 0;
    int X = rint(X0*W);
    int Y = rint(Y0*W);
    
    short sx = (short)(X >> INTER_BITS) - 1;
    short sy = (short)(Y >> INTER_BITS) - 1;
    short ay = (short)(Y & (INTER_TAB_SIZE-1));
    short ax = (short)(X & (INTER_TAB_SIZE-1));
    
    uchar v[64];
    int i, j, c;

    for(i=0; i<4;  i++)
        for(j=0; j<4; j++)
            for(c=0; c<cn; c++)
                v[i*4*cn + j*cn + c] = (sx+j >= 0 && sx+j < cols && sy+i >= 0 && sy+i < rows) ? src[(sy+i) * srcStep + (sx+j)*cn + c] : 0;
   
    short itab[16];
    float tab1y[4], tab1x[4];
    float axx, ayy;

    ayy = 1.f/INTER_TAB_SIZE * ay;
    axx = 1.f/INTER_TAB_SIZE * ax;
    interpolateCubic(ayy, tab1y);
    interpolateCubic(axx, tab1x);
    int isum = 0;
    for( i=0; i<4; i++ )
    {
        for( j=0; j<4; j++)
        {
            double v = tab1y[i] * tab1x[j];
            isum += itab[i*4+j] = convert_short_sat( rint( v * INTER_REMAP_COEF_SCALE ) );
        }
    }
    if( isum != INTER_REMAP_COEF_SCALE )
    {
        int k1, k2, ksize = 4;
        int diff = isum - INTER_REMAP_COEF_SCALE;
        int ksize2 = ksize/2, Mk1=ksize2, Mk2=ksize2, mk1=ksize2, mk2=ksize2;
        for( k1 = ksize2; k1 < ksize2+2; k1++ )
            for( k2 = ksize2; k2 < ksize2+2; k2++ )
            {
                if( itab[k1*ksize+k2] < itab[mk1*ksize+mk2] )
                    mk1 = k1, mk2 = k2;
                else if( itab[k1*ksize+k2] > itab[Mk1*ksize+Mk2] )
                     Mk1 = k1, Mk2 = k2;
            }
            if( diff < 0 )
                itab[Mk1*ksize + Mk2] = (short)(itab[Mk1*ksize + Mk2] - diff);
            else
                itab[mk1*ksize + mk2] = (short)(itab[mk1*ksize + mk2] - diff);
    }

    if( sx+4 < 0 || sx >= cols || sy+4 < 0 || sy >= rows)
    {
        for(c = 0; c < cn; c++)
            dst[dy*dstStep+dx*cn+c] = 0;
    }
    else
    {
        int sum;
        for(c = 0; c < cn; c++)
        {
            sum = 0;
            for ( i =0; i<16; i++ )
            {
                sum += v[i*cn+c] * itab[i] ;
            }
            dst[dy*dstStep+dx*cn+c] = convert_uchar_sat( rint(sum + (1 << (INTER_REMAP_COEF_BITS-1))) >> INTER_REMAP_COEF_BITS ) ;
        }
    }
}

__kernel void warpPerspective_16u_NN(__global ushort * src, __global ushort * dst, int cols, int rows,  int cn,
                            int srcStep, int dstStep, __global double * M, int interpolation)
{
    int dx = get_global_id(0);
    int dy = get_global_id(1);
    
    double X0 = M[0]*dx + M[1]*dy + M[2];
    double Y0 = M[3]*dx + M[4]*dy + M[5];
    double W = M[6]*dx + M[7]*dy + M[8];
    W = W ? 1./W : 0;
    int X = rint(X0*W);
    int Y = rint(Y0*W);
    short sx = (short)X;
    short sy = (short)Y;

    for(int c = 0; c < cn; c++)
        dst[dy*dstStep+dx*cn+c] = (sx >= 0 && sx < cols && sy >= 0 && sy < rows) ? src[sy*srcStep+sx*cn+c] : 0; 
}

__kernel void warpPerspective_16u_Linear(__global ushort * src, __global ushort * dst, int cols, int rows,  int cn,
                            int srcStep, int dstStep, __global double * M, int interpolation)
{
    int dx = get_global_id(0);
    int dy = get_global_id(1);
    
    double X0 = M[0]*dx + M[1]*dy + M[2];
    double Y0 = M[3]*dx + M[4]*dy + M[5];
    double W = M[6]*dx + M[7]*dy + M[8];
    W = W ? INTER_TAB_SIZE/W : 0;
    int X = rint(X0*W);
    int Y = rint(Y0*W);
    
    short sx = (short)(X >> INTER_BITS);
    short sy = (short)(Y >> INTER_BITS);
    short ay = (short)(Y & (INTER_TAB_SIZE-1));
    short ax = (short)(X & (INTER_TAB_SIZE-1));
    
    ushort v[16];
    int i, j, c;

    for(i=0; i<2;  i++)
        for(j=0; j<2; j++)
            for(c=0; c<cn; c++)
                v[i*2*cn + j*cn + c] = (sx+j >= 0 && sx+j < cols && sy+i >= 0 && sy+i < rows) ? src[(sy+i) * srcStep + (sx+j)*cn + c] : 0;
   
    float tab[4];
    float tab1y[2], tab1x[2];
    tab1y[0] = 1.0 - 1.f/INTER_TAB_SIZE*ay;
    tab1y[1] = 1.f/INTER_TAB_SIZE*ay;
    tab1x[0] = 1.0 - 1.f/INTER_TAB_SIZE*ax;
    tab1x[1] = 1.f/INTER_TAB_SIZE*ax;
    
    for( i=0; i<2; i++ )
    {
        for( j=0; j<2; j++)
        {
            tab[i*2+j] = tab1y[i] * tab1x[j];
        }
    }
    if( sx+1 < 0 || sx >= cols || sy+1 < 0 || sy >= rows)
    {
        for(c = 0; c < cn; c++)
            dst[dy*dstStep+dx*cn+c] = 0;
    }
    else
    {
        float sum;
        for(c = 0; c < cn; c++)
        {
            sum = 0;
            for ( i =0; i<4; i++ )
            {
                sum += v[i*cn+c] * tab[i] ;
            }
            dst[dy*dstStep+dx*cn+c] = convert_ushort_sat( rint(sum) ) ;
        }
    }
}

__kernel void warpPerspective_16u_Cubic(__global ushort * src, __global ushort * dst, int cols, int rows,  int cn,
                            int srcStep, int dstStep, __global double * M, int interpolation)
{
    int dx = get_global_id(0);
    int dy = get_global_id(1);
    
    double X0 = M[0]*dx + M[1]*dy + M[2];
    double Y0 = M[3]*dx + M[4]*dy + M[5];
    double W = M[6]*dx + M[7]*dy + M[8];
    W = W ? INTER_TAB_SIZE/W : 0;
    int X = rint(X0*W);
    int Y = rint(Y0*W);
    
    short sx = (short)(X >> INTER_BITS) - 1;
    short sy = (short)(Y >> INTER_BITS) - 1;
    short ay = (short)(Y & (INTER_TAB_SIZE-1));
    short ax = (short)(X & (INTER_TAB_SIZE-1));
    
    ushort v[64];
    int i, j, c;

    for(i=0; i<4;  i++)
        for(j=0; j<4; j++)
            for(c=0; c<cn; c++)
                v[i*4*cn + j*cn + c] = (sx+j >= 0 && sx+j < cols && sy+i >= 0 && sy+i < rows) ? src[(sy+i) * srcStep + (sx+j)*cn + c] : 0;
   
    float tab[16];
    float tab1y[4], tab1x[4];
    float axx, ayy;

    ayy = 1.f/INTER_TAB_SIZE * ay;
    axx = 1.f/INTER_TAB_SIZE * ax;
    interpolateCubic(ayy, tab1y);
    interpolateCubic(axx, tab1x);
    for( i=0; i<4; i++ )
    {
        for( j=0; j<4; j++)
        {
            tab[i*4+j] = tab1y[i] * tab1x[j];
        }
    }

    int width = cols-3>0 ? cols-3 : 0;
    int height = rows-3>0 ? rows-3 : 0;
    if((unsigned)sx < width && (unsigned)sy < height )
    {
        float sum;
        for(c = 0; c < cn; c++)
        {
            sum = 0;
            for ( i =0; i<4; i++ )
            {
                    sum += v[i*4*cn+c] * tab[i*4] + v[i*4*cn+c+1]*tab[i*4+1]
                          +v[i*4*cn+c+2] * tab[i*4+2] + v[i*4*cn+c+3]*tab[i*4+3];
            }
            dst[dy*dstStep+dx*cn+c] = convert_ushort_sat( rint(sum ));
        }
    }
    else if( sx+4 < 0 || sx >= cols || sy+4 < 0 || sy >= rows)
    {
        for(c = 0; c < cn; c++)
            dst[dy*dstStep+dx*cn+c] = 0;
    }
    else
    {
        float sum;
        for(c = 0; c < cn; c++)
        {
            sum = 0;
            for ( i =0; i<16; i++ )
            {
                    sum += v[i*cn+c] * tab[i];
            }
            dst[dy*dstStep+dx*cn+c] = convert_ushort_sat( rint(sum ));
        }
    }
}


__kernel void warpPerspective_32s_NN(__global int * src, __global int * dst, int cols, int rows,  int cn,
                            int srcStep, int dstStep, __global double * M, int interpolation)
{
    int dx = get_global_id(0);
    int dy = get_global_id(1);
    
    double X0 = M[0]*dx + M[1]*dy + M[2];
    double Y0 = M[3]*dx + M[4]*dy + M[5];
    double W = M[6]*dx + M[7]*dy + M[8];
    W = W ? 1./W : 0;
    int X = rint(X0*W);
    int Y = rint(Y0*W);
    short sx = (short)X;
    short sy = (short)Y;

    for(int c = 0; c < cn; c++)
        dst[dy*dstStep+dx*cn+c] = (sx >= 0 && sx < cols && sy >= 0 && sy < rows) ? src[sy*srcStep+sx*cn+c] : 0; 
}

__kernel void warpPerspective_32s_Linear(__global int * src, __global int * dst, int cols, int rows,  int cn,
                            int srcStep, int dstStep, __global double * M, int interpolation)
{
    int dx = get_global_id(0);
    int dy = get_global_id(1);
    
    double X0 = M[0]*dx + M[1]*dy + M[2];
    double Y0 = M[3]*dx + M[4]*dy + M[5];
    double W = M[6]*dx + M[7]*dy + M[8];
    W = W ? INTER_TAB_SIZE/W : 0;
    int X = rint(X0*W);
    int Y = rint(Y0*W);
    
    short sx = (short)(X >> INTER_BITS);
    short sy = (short)(Y >> INTER_BITS);
    short ay = (short)(Y & (INTER_TAB_SIZE-1));
    short ax = (short)(X & (INTER_TAB_SIZE-1));
    
    int v[16];
    int i, j, c;

    for(i=0; i<2;  i++)
        for(j=0; j<2; j++)
            for(c=0; c<cn; c++)
                v[i*2*cn + j*cn + c] = (sx+j >= 0 && sx+j < cols && sy+i >= 0 && sy+i < rows) ? src[(sy+i) * srcStep + (sx+j)*cn + c] : 0;
   
    float tab[4];
    float tab1y[2], tab1x[2];
    tab1y[0] = 1.0 - 1.f/INTER_TAB_SIZE*ay;
    tab1y[1] = 1.f/INTER_TAB_SIZE*ay;
    tab1x[0] = 1.0 - 1.f/INTER_TAB_SIZE*ax;
    tab1x[1] = 1.f/INTER_TAB_SIZE*ax;
    
    for( i=0; i<2; i++ )
    {
        for( j=0; j<2; j++)
        {
            tab[i*2+j] = tab1y[i] * tab1x[j];
        }
    }
    if( sx+1 < 0 || sx >= cols || sy+1 < 0 || sy >= rows)
    {
        for(c = 0; c < cn; c++)
            dst[dy*dstStep+dx*cn+c] = 0;
    }
    else
    {
        float sum;
        for(c = 0; c < cn; c++)
        {
            sum = 0;
            for ( i =0; i<4; i++ )
            {
                sum += v[i*cn+c] * tab[i] ;
            }
            dst[dy*dstStep+dx*cn+c] = convert_int_sat( rint(sum) ) ;
        }
    }
}

__kernel void warpPerspective_32s_Cubic(__global int * src, __global int * dst, int cols, int rows,  int cn,
                            int srcStep, int dstStep, __global double * M, int interpolation)
{
    int dx = get_global_id(0);
    int dy = get_global_id(1);
    
    double X0 = M[0]*dx + M[1]*dy + M[2];
    double Y0 = M[3]*dx + M[4]*dy + M[5];
    double W = M[6]*dx + M[7]*dy + M[8];
    W = W ? INTER_TAB_SIZE/W : 0;
    int X = rint(X0*W);
    int Y = rint(Y0*W);
    
    short sx = (short)(X >> INTER_BITS) - 1;
    short sy = (short)(Y >> INTER_BITS) - 1;
    short ay = (short)(Y & (INTER_TAB_SIZE-1));
    short ax = (short)(X & (INTER_TAB_SIZE-1));

    int v[64];
    int i, j, c;

    for(i=0; i<4;  i++)
        for(j=0; j<4; j++)
            for(c=0; c<cn; c++)
                v[i*4*cn + j*cn + c] = (sx+j >= 0 && sx+j < cols && sy+i >= 0 && sy+i < rows) ? src[(sy+i) * srcStep + (sx+j)*cn + c] : 0;
   
    float tab[16];
    float tab1y[4], tab1x[4];
    float axx, ayy;

    ayy = 1.f/INTER_TAB_SIZE * ay;
    axx = 1.f/INTER_TAB_SIZE * ax;
    interpolateCubic(ayy, tab1y);
    interpolateCubic(axx, tab1x);
    for( i=0; i<4; i++ )
    {
        for( j=0; j<4; j++)
        {
            tab[i*4+j] = tab1y[i] * tab1x[j];
        }
    }

    if( sx+4 < 0 || sx >= cols || sy+4 < 0 || sy >= rows)
    {
        for(c = 0; c < cn; c++)
            dst[dy*dstStep+dx*cn+c] = 0;
    }
    else
    {
        float sum;
        for(c = 0; c < cn; c++)
        {
            sum = 0;
            for ( i =0; i<16; i++ )
            {
                sum += v[i*cn+c] * tab[i] ;
            }
            dst[dy*dstStep+dx*cn+c] = convert_int_sat( rint(sum ));
        }
    }
}


__kernel void warpPerspective_32f_NN(__global float * src, __global float * dst, int cols, int rows,  int cn,
                            int srcStep, int dstStep, __global double * M, int interpolation)
{
    int dx = get_global_id(0);
    int dy = get_global_id(1);
    
    double X0 = M[0]*dx + M[1]*dy + M[2];
    double Y0 = M[3]*dx + M[4]*dy + M[5];
    double W = M[6]*dx + M[7]*dy + M[8];
    W = W ? 1./W : 0;
    int X = rint(X0*W);
    int Y = rint(Y0*W);
    short sx = (short)X;
    short sy = (short)Y;

    for(int c = 0; c < cn; c++)
        dst[dy*dstStep+dx*cn+c] = (sx >= 0 && sx < cols && sy >= 0 && sy < rows) ? src[sy*srcStep+sx*cn+c] : 0; 
}

__kernel void warpPerspective_32f_Linear(__global float * src, __global float * dst, int cols, int rows,  int cn,
                            int srcStep, int dstStep, __global double * M, int interpolation)
{
    int dx = get_global_id(0);
    int dy = get_global_id(1);
    
    double X0 = M[0]*dx + M[1]*dy + M[2];
    double Y0 = M[3]*dx + M[4]*dy + M[5];
    double W = M[6]*dx + M[7]*dy + M[8];
    W = W ? INTER_TAB_SIZE/W : 0;
    int X = rint(X0*W);
    int Y = rint(Y0*W);
    
    short sx = (short)(X >> INTER_BITS);
    short sy = (short)(Y >> INTER_BITS);
    short ay = (short)(Y & (INTER_TAB_SIZE-1));
    short ax = (short)(X & (INTER_TAB_SIZE-1));
    
    float v[16];
    int i, j, c;

    for(i=0; i<2;  i++)
        for(j=0; j<2; j++)
            for(c=0; c<cn; c++)
                v[i*2*cn + j*cn + c] = (sx+j >= 0 && sx+j < cols && sy+i >= 0 && sy+i < rows) ? src[(sy+i) * srcStep + (sx+j)*cn + c] : 0;
   
    float tab[4];
    float tab1y[2], tab1x[2];
    tab1y[0] = 1.0 - 1.f/INTER_TAB_SIZE*ay;
    tab1y[1] = 1.f/INTER_TAB_SIZE*ay;
    tab1x[0] = 1.0 - 1.f/INTER_TAB_SIZE*ax;
    tab1x[1] = 1.f/INTER_TAB_SIZE*ax;
    
    for( i=0; i<2; i++ )
    {
        for( j=0; j<2; j++)
        {
            tab[i*2+j] = tab1y[i] * tab1x[j];
        }
    }
    if( sx+1 < 0 || sx >= cols || sy+1 < 0 || sy >= rows)
    {
        for(c = 0; c < cn; c++)
            dst[dy*dstStep+dx*cn+c] = 0;
    }
    else
    {
        float sum;
        for(c = 0; c < cn; c++)
        {
            sum = 0;
            for ( i =0; i<4; i++ )
            {
                sum += v[i*cn+c] * tab[i] ;
            }
            dst[dy*dstStep+dx*cn+c] = sum ;
        }
    }
}

__kernel void warpPerspective_32f_Cubic(__global float * src, __global float * dst, int cols, int rows,  int cn,
                            int srcStep, int dstStep, __global double * M, int interpolation)
{
    int dx = get_global_id(0);
    int dy = get_global_id(1);
    
    double X0 = M[0]*dx + M[1]*dy + M[2];
    double Y0 = M[3]*dx + M[4]*dy + M[5];
    double W = M[6]*dx + M[7]*dy + M[8];
    W = W ? INTER_TAB_SIZE/W : 0;
    int X = rint(X0*W);
    int Y = rint(Y0*W);
    
    short sx = (short)(X >> INTER_BITS) - 1;
    short sy = (short)(Y >> INTER_BITS) - 1;
    short ay = (short)(Y & (INTER_TAB_SIZE-1));
    short ax = (short)(X & (INTER_TAB_SIZE-1));

    float v[64];
    int i, j, c;

    for(i=0; i<4;  i++)
        for(j=0; j<4; j++)
            for(c=0; c<cn; c++)
                v[i*4*cn + j*cn + c] = (sx+j >= 0 && sx+j < cols && sy+i >= 0 && sy+i < rows) ? src[(sy+i) * srcStep + (sx+j)*cn + c] : 0;
   
    float tab[16];
    float tab1y[4], tab1x[4];
    float axx, ayy;

    ayy = 1.f/INTER_TAB_SIZE * ay;
    axx = 1.f/INTER_TAB_SIZE * ax;
    interpolateCubic(ayy, tab1y);
    interpolateCubic(axx, tab1x);
    for( i=0; i<4; i++ )
    {
        for( j=0; j<4; j++)
        {
            tab[i*4+j] = tab1y[i] * tab1x[j];
        }
    }

    int width = cols-3>0 ? cols-3 : 0;
    int height = rows-3>0 ? rows-3 : 0;
    if((unsigned)sx < width && (unsigned)sy < height )
    {
        float sum;
        for(c = 0; c < cn; c++)
        {
            sum = 0;
            for ( i =0; i<4; i++ )
            {
                    sum += v[i*4*cn+c] * tab[i*4] + v[i*4*cn+c+1]*tab[i*4+1]
                          +v[i*4*cn+c+2] * tab[i*4+2] + v[i*4*cn+c+3]*tab[i*4+3];
            }
            dst[dy*dstStep+dx*cn+c] = sum;
        }
    }
    else if( sx+4 < 0 || sx >= cols || sy+4 < 0 || sy >= rows)
    {
        for(c = 0; c < cn; c++)
            dst[dy*dstStep+dx*cn+c] = 0;
    }
    else
    {
        float sum;
        for(c = 0; c < cn; c++)
        {
            sum = 0;
            for ( i =0; i<16; i++ )
            {
                    sum += v[i*cn+c] * tab[i];
            }
            dst[dy*dstStep+dx*cn+c] = sum;
        }
    }
}
*/
