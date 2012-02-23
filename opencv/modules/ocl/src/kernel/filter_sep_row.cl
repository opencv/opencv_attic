//                           License Agreement
//                For Open Source Computer Vision Library
//
// Copyright (C) 2010-2012, Institute Of Software Chinese Academy Of Science, all rights reserved.
// Copyright (C) 2010-2012, Advanced Micro Devices, Inc., all rights reserved.
// Third party copyrights are property of their respective owners.
//
// @Authors
//    Niko Li, Niko.li@amd.com
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
//

#define READ_TIMES_ROW ((2*(RADIUSX+LSIZE0)-1)/LSIZE0) //for c4 only
#define READ_TIMES_COL ((2*(RADIUSY+LSIZE1)-1)/LSIZE1)
//#pragma OPENCL EXTENSION cl_amd_printf : enable
#define RADIUS 1
#if CN ==1
#define ALIGN (((RADIUS)+3)>>2<<2)
#elif CN==2
#define ALIGN (((RADIUS)+1)>>1<<1)
#elif CN==3
#define ALIGN (((RADIUS)+3)>>2<<2)
#elif CN==4
#define ALIGN (RADIUS)
#endif


#ifdef BORDER_CONSTANT
//BORDER_CONSTANT:      iiiiii|abcdefgh|iiiiiii
#define ELEM(i,l_edge,r_edge,elem1,elem2) (i)<(l_edge) | (i) >= (r_edge) ? (elem1) : (elem2)
//                           License Agreement
//                For Open Source Computer Vision Library
//
// Copyright (C) 2010-2012, Institute Of Software Chinese Academy Of Science, all rights reserved.
// Copyright (C) 2010-2012, Advanced Micro Devices, Inc., all rights reserved.
// Third party copyrights are property of their respective owners.
//
// @Authors
//    Niko Li, Niko.li@amd.com
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
//

#endif

#ifdef BORDER_REPLICATE
//BORDER_REPLICATE:     aaaaaa|abcdefgh|hhhhhhh
#define ADDR_L(i,l_edge,r_edge,addr)  (i) < (l_edge) ? (l_edge) : (addr)
#define ADDR_R(i,r_edge,addr)   (i) >= (r_edge) ? (r_edge)-1 : (addr)
#endif

#ifdef BORDER_REFLECT
//BORDER_REFLECT:       fedcba|abcdefgh|hgfedcb
#define ADDR_L(i,l_edge,r_edge,addr)  (i) < (l_edge) ? -(i)-1 : (addr)
#define ADDR_R(i,r_edge,addr) (i) >= (r_edge) ? -(i)-1+((r_edge)<<1) : (addr)
#endif

#ifdef BORDER_REFLECT_101
//BORDER_REFLECT_101:   gfedcb|abcdefgh|gfedcba
#define ADDR_L(i,l_edge,r_edge,addr)  (i) < (l_edge) ? -(i) : (addr)
#define ADDR_R(i,r_edge,addr) (i) >= (r_edge) ? -(i)-2+((r_edge)<<1) : (addr)
#endif

#ifdef BORDER_WRAP
//BORDER_WRAP:          cdefgh|abcdefgh|abcdefg
#define ADDR_L(i,l_edge,r_edge,addr)  (i) < (l_edge) ? (i)+(r_edge) : (addr)
#define ADDR_R(i,r_edge,addr)   (i) >= (r_edge) ?   (i)-(r_edge) : (addr)
#endif

/**********************************************************************************
These kernels are written for separable filters such as Sobel, Scharr, GaussianBlur.
Now(6/29/2011) the kernels only support 8U data type and the anchor of the convovle
kernel must be in the center. ROI is not supported either.
For channels =1,2,4, each kernels read 4 elements(not 4 pixels), and for channels =3,
the kernel read 4 pixels, save them to LDS and read the data needed from LDS to 
calculate the result.
The length of the convovle kernel supported is related to the LSIZE0 and the MAX size
of LDS, which is HW related.
For channels = 1,3 the RADIUS is no more than LSIZE0*2
For channels = 2, the RADIUS is no more than LSIZE0
For channels = 4, arbitary RADIUS is supported unless the LDS is not enough
Niko
6/29/2011
***********************************************************************************/
/*
__kernel __attribute__((reqd_work_group_size(LSIZE0,LSIZE1,1))) void row_filter_C1_D0
						(__global const uchar * restrict src, 
						 __global float * dst,
                         const int cols,
                         const int rows, 
						 const int src_whole_cols,
						 //const int src_whole_rows,
                         const int src_step_in_pixel, 
                         const int src_offset_x, 
                         const int src_offset_y, 
                         const int dst_step_in_pixel,
                         const int dst_offset_in_pixel,
                         __constant float * mat_kernel __attribute__((max_constant_size(4*(2*RADIUS+1)))))
{
    int x = get_global_id(0)<<2;
    int y = get_global_id(1);
    int l_x = get_local_id(0);
    int l_y = get_local_id(1);
	int align = (src_offset_x - RADIUS) & 0xfffffffc;
	int inv_align = (src_offset_x - RADIUS) & 3;
    int i=0;
    float4 sum=0.0f;

	uchar4 temp[2];
	//int baseindex = mad24(y,src_step_in_pixel,x);
	int baserow = mad24(y,src_step_in_pixel,src_offset_y);
	__local uchar4 LDS_DAT[LSIZE1][LSIZE0*2+1];
	#ifdef BORDER_CONSTANT
	temp[0] = *(__global uchar4*)&src[baserow+x+align];
	temp[1] = *(__global uchar4*)&src[baserow+LSIZE0*4+x+align];
	temp[0].x = ELEM(x+align,0,src_whole_cols,0,temp[0].x);
	temp[0].y = ELEM(x+align+1,0,src_whole_cols,0,temp[0].y);
	temp[0].z = ELEM(x+align+2,0,src_whole_cols,0,temp[0].z);
	temp[0].w = ELEM(x+align+3,0,src_whole_cols,0,temp[0].w);
	temp[1].x = ELEM(x+LSIZE0*4+align,0,src_whole_cols,0,temp[1].x);
	temp[1].y = ELEM(x+LSIZE0*4+align+1,0,src_whole_cols,0,temp[1].y);
	temp[1].z = ELEM(x+LSIZE0*4+align+2,0,src_whole_cols,0,temp[1].z);
	temp[1].w = ELEM(x+LSIZE0*4+align+3,0,src_whole_cols,0,temp[1].w);
	#else	
	int not_all_in_range = (x+offset_x-RADIUS<0) | (x+LSIZE0*4+align+4>src_whole_cols);
	int4 index[2];
	if(not_all_in_range)
	{
		index[0].x = ADDR_L(x+align,0,src_whole_cols);
		index[0].y = ADDR_L(x+align+1,0,src_whole_cols);
		index[0].z = ADDR_L(x+align+2,0,src_whole_cols);
		index[0].w = ADDR_L(x+align+3,0,src_whole_cols);
		index[0].x = ADDR_R(x+align,src_whole_cols,index[0].x);
		index[0].y = ADDR_R(x+align+1,src_whole_cols,index[0].y);
		index[0].z = ADDR_R(x+align+2,src_whole_cols,index[0].z);
		index[0].w = ADDR_R(x+align+3,src_whole_cols,index[0].w);
		index[1].x = ADDR_L(x+LSIZE0*4+align,0,src_whole_cols);
		index[1].y = ADDR_L(x+LSIZE0*4+align+1,0,src_whole_cols);
		index[1].z = ADDR_L(x+LSIZE0*4+align+2,0,src_whole_cols);
		index[1].w = ADDR_L(x+LSIZE0*4+align+3,0,src_whole_cols);
		index[1].x = ADDR_R(x+LSIZE0*4+align,src_whole_cols,index[1].x);
		index[1].y = ADDR_R(x+LSIZE0*4+align+1,src_whole_cols,index[1].y);
		index[1].z = ADDR_R(x+LSIZE0*4+align+2,src_whole_cols,index[1].z);
		index[1].w = ADDR_R(x+LSIZE0*4+align+3,src_whole_cols,index[1].w);
		temp[0].x = src[baserow+index[0].x];
		temp[0].y = src[baserow+index[0].y];
		temp[0].z = src[baserow+index[0].z];
		temp[0].w = src[baserow+index[0].w];
		temp[1].x = src[baserow+index[1].x];
		temp[1].y = src[baserow+index[1].y];
		temp[1].z = src[baserow+index[1].z];
		temp[1].w = src[baserow+index[1].w];
	}
	else
	{
		temp[0] = *(__global uchar4*)&src[baserow+x+align];
		temp[1] = *(__global uchar4*)&src[baserow+x+LSIZE0*4+align];
	}
	#endif
	LDS_DAT[l_y][l_x] = temp[0];
	LDS_DAT[l_y][l_x+LSIZE0] = temp[1];
	barrier(CLK_LOCAL_MEM_FENCE);

	int baseindex = mad24(y,dst_step_in_pixel,x+dst_offset_in_pixel);
	sum =convert_float4(vload4(0,(__local uchar*)&LDS_DAT[l_y][l_x]+inv_align+RADIUS))*mat_kernel[RADIUS];
	float4 prefetch_LDS[2];
	for(i=1;i<=RADIUS;i++)
	{
		prefetch_LDS[0]=convert_float4(vload4(0,(__local uchar*)&LDS_DAT[l_y][l_x]+inv_align+RADIUS-i));
		prefetch_LDS[1]=convert_float4(vload4(0,(__local uchar*)&LDS_DAT[l_y][l_x]+inv_align+RADIUS+i));
		sum += prefetch_LDS[0]*mat_kernel[RADIUS-i]+prefetch_LDS[1]*mat_kernel[RADIUS+i];
	}

	if((x+3< cols) & (y < rows))
	{	
		vstore4(sum,0,dst+baseindex);
	}
	else if((x< cols) & (y < rows))
	{
		size_t dst_addr_start = mad24(y,dst_step_in_pixel,dst_offset_in_pixel);
		size_t dst_addr_end = mad24(y,dst_step_in_pixel,cols+dst_offset_in_pixel);
		float4 temp_dst = vload4(0,dst+baseindex);
		temp_dst.x = (baseindex>=dst_addr_start)&(baseindex<dst_addr_end) ? sum.x : temp_dst.x;
		temp_dst.y = (baseindex+1>=dst_addr_start)&(baseindex+1<dst_addr_end) ? sum.y : temp_dst.y;
		temp_dst.z = (baseindex+2>=dst_addr_start)&(baseindex+2<dst_addr_end) ? sum.z : temp_dst.z;
		temp_dst.w = (baseindex+3>=dst_addr_start)&(baseindex+3<dst_addr_end) ? sum.w : temp_dst.w;
		vstore4(temp_dst,0,dst+baseindex);
	}
}


__kernel __attribute__((reqd_work_group_size(LSIZE0,LSIZE1,1))) void row_filter_C2_D0
						(__global const uchar * restrict src, 
						 __global float * dst,
                         const int src_cols,
                         const int src_rows, 
                         const int src_pix_per_row, 
                         const int dst_cols, 
                         const int dst_rows,
                         const int dst_pix_per_row,
                         __constant float * mat_kernel __attribute__((max_constant_size(4*(2*RADIUS+1)))))
{
    int x = get_global_id(0)<<1;
    int y = get_global_id(1);
    int l_x = get_local_id(0);
    int l_y = get_local_id(1);
    int i=0;
    float4 sum=0.0f;
	int2 index[2];
	uchar4 temp[2];
	int baseindex = mad24(y,src_pix_per_row,x<<1);
	int baserow = mul24(y,src_pix_per_row);
	__local uchar4 LDS_DAT[LSIZE1][LSIZE0*2];
	#ifdef BORDER_CONSTANT
	temp[0] = *(__global uchar4*)&src[baseindex-ALIGN*2];
	temp[1] = *(__global uchar4*)&src[baseindex+LSIZE0*4-ALIGN*2];
	temp[0].xy = ELEM(x-ALIGN,0,src_cols,0,temp[0].xy);
	temp[0].zw = ELEM(x-ALIGN+1,0,src_cols,0,temp[0].zw);
	temp[1].xy = ELEM(x+LSIZE0*2-ALIGN,0,src_cols,0,temp[1].xy);
	temp[1].zw = ELEM(x+LSIZE0*2-ALIGN+1,0,src_cols,0,temp[1].zw);
	#else	
	int not_all_in_range = (x-RADIUS<0) | (x+LSIZE0*2-ALIGN+2>src_cols);
	if(not_all_in_range)
	{
		index[0].x = ADDR_L(x-ALIGN,0,src_cols);
		index[0].y = ADDR_L(x-ALIGN+1,0,src_cols);
		index[0].x = ADDR_R(x-ALIGN,src_cols,index[0].x);
		index[0].y = ADDR_R(x-ALIGN+1,src_cols,index[0].y);
		index[1].x = ADDR_L(x+LSIZE0*2-ALIGN,0,src_cols);
		index[1].y = ADDR_L(x+LSIZE0*2-ALIGN+1,0,src_cols);
		index[1].x = ADDR_R(x+LSIZE0*2-ALIGN,src_cols,index[1].x);
		index[1].y = ADDR_R(x+LSIZE0*2-ALIGN+1,src_cols,index[1].y);
		temp[0].xy = *(__global uchar2*)&src[baserow+(index[0].x<<1)];
		temp[0].zw = *(__global uchar2*)&src[baserow+(index[0].y<<1)];
		temp[1].xy = *(__global uchar2*)&src[baserow+(index[1].x<<1)];
		temp[1].zw = *(__global uchar2*)&src[baserow+(index[1].y<<1)];
	}
	else
	{
		temp[0] = *(__global uchar4*)&src[baseindex-(ALIGN<<1)];
		temp[1] = *(__global uchar4*)&src[baseindex+((LSIZE0*2-ALIGN)<<1)];
	}
	#endif
	LDS_DAT[l_y][l_x] = temp[0];
	LDS_DAT[l_y][l_x+LSIZE0] = temp[1];
	barrier(CLK_LOCAL_MEM_FENCE);
	sum = convert_float4(LDS_DAT[l_y][l_x+ALIGN/2]);
	for(i=1;i<=RADIUS;i++)
	{
		float2 prefetch_LDS[4];
		prefetch_LDS[0]=convert_float2(vload2(0,(__local uchar*)&LDS_DAT[l_y][l_x]+ALIGN*2+RADIUS*2-2*i));
		prefetch_LDS[1]=convert_float2(vload2(0,(__local uchar*)&LDS_DAT[l_y][l_x]+ALIGN*2+RADIUS*2+2-2*i));
		prefetch_LDS[2]=convert_float2(vload2(0,(__local uchar*)&LDS_DAT[l_y][l_x]+ALIGN*2+RADIUS*2+2*i));
		prefetch_LDS[3]=convert_float2(vload2(0,(__local uchar*)&LDS_DAT[l_y][l_x]+ALIGN*2+RADIUS*2+2+2*i));		
		sum.xy += prefetch_LDS[0]*mat_kernel[RADIUS-i]+prefetch_LDS[2]*mat_kernel[RADIUS+i];
		sum.zw += prefetch_LDS[1]*mat_kernel[RADIUS-i]+prefetch_LDS[3]*mat_kernel[RADIUS+i];
	}
	baseindex = mad24(y,dst_pix_per_row,x<<1);
	if((x< dst_cols) & (y < dst_rows))
	{
		*(__global float4*)&dst[baseindex] =sum;
	}
}


__kernel __attribute__((reqd_work_group_size(LSIZE0,LSIZE1,1))) void row_filter_C3_D0
						(__global const uchar * restrict src, 
						 __global float * dst,
                         const int src_cols,
                         const int src_rows, 
                         const int src_pix_per_row, 
                         const int dst_cols, 
                         const int dst_rows,
                         const int dst_pix_per_row,
                         __constant float * mat_kernel __attribute__((max_constant_size(4*(2*RADIUS+1)))))
{
    int x = get_global_id(0)<<2;
    int y = get_global_id(1);
    int l_x = get_local_id(0)*3;
    int l_y = get_local_id(1);
    int i=0;
    float4 sum[3];
	int4 index[2];
	uchar4 temp[2*3];
	int baseindex = mad24(y,src_pix_per_row,x*3);
	int baserow = mul24(y,src_pix_per_row);
	__local uchar4 LDS_DAT[LSIZE1][LSIZE0*2*3+1];
	#ifdef BORDER_CONSTANT
	temp[0] = *(__global uchar4*)&src[baseindex-ALIGN*3];
	temp[1] = *(__global uchar4*)&src[baseindex-ALIGN*3+4];
	temp[2] = *(__global uchar4*)&src[baseindex-ALIGN*3+8];
	temp[3] = *(__global uchar4*)&src[baseindex+LSIZE0*12-ALIGN*3];
	temp[4] = *(__global uchar4*)&src[baseindex+LSIZE0*12-ALIGN*3+4];
	temp[5] = *(__global uchar4*)&src[baseindex+LSIZE0*12-ALIGN*3+8];

	temp[0].x = ELEM(x-ALIGN,0,src_cols,0,temp[0].x);
	temp[0].y = ELEM(x-ALIGN,0,src_cols,0,temp[0].y);
	temp[0].z = ELEM(x-ALIGN,0,src_cols,0,temp[0].z);
	temp[0].w = ELEM(x-ALIGN+1,0,src_cols,0,temp[0].w);
	temp[1].x = ELEM(x-ALIGN+1,0,src_cols,0,temp[1].x);
	temp[1].y = ELEM(x-ALIGN+1,0,src_cols,0,temp[1].y);
	temp[1].z = ELEM(x-ALIGN+2,0,src_cols,0,temp[1].z);
	temp[1].w = ELEM(x-ALIGN+2,0,src_cols,0,temp[1].w);
	temp[2].x = ELEM(x-ALIGN+2,0,src_cols,0,temp[2].x);
	temp[2].y = ELEM(x-ALIGN+3,0,src_cols,0,temp[2].y);
	temp[2].z = ELEM(x-ALIGN+3,0,src_cols,0,temp[2].z);
	temp[2].w = ELEM(x-ALIGN+3,0,src_cols,0,temp[2].w);
	temp[3].x = ELEM(x+LSIZE0*4-ALIGN,0,src_cols,0,temp[3].x);
	temp[3].y = ELEM(x+LSIZE0*4-ALIGN,0,src_cols,0,temp[3].y);
	temp[3].z = ELEM(x+LSIZE0*4-ALIGN,0,src_cols,0,temp[3].z);
	temp[3].w = ELEM(x+LSIZE0*4-ALIGN+1,0,src_cols,0,temp[3].w);
	temp[4].x = ELEM(x+LSIZE0*4-ALIGN+1,0,src_cols,0,temp[4].x);
	temp[4].y = ELEM(x+LSIZE0*4-ALIGN+1,0,src_cols,0,temp[4].y);
	temp[4].z = ELEM(x+LSIZE0*4-ALIGN+2,0,src_cols,0,temp[4].z);
	temp[4].w = ELEM(x+LSIZE0*4-ALIGN+2,0,src_cols,0,temp[4].w);
	temp[5].x = ELEM(x+LSIZE0*4-ALIGN+2,0,src_cols,0,temp[5].x);
	temp[5].y = ELEM(x+LSIZE0*4-ALIGN+3,0,src_cols,0,temp[5].y);
	temp[5].z = ELEM(x+LSIZE0*4-ALIGN+3,0,src_cols,0,temp[5].z);
	temp[5].w = ELEM(x+LSIZE0*4-ALIGN+3,0,src_cols,0,temp[5].w);
	#else	
	int not_all_in_range = (x-RADIUS<0) | (x+LSIZE0*4-ALIGN+4>src_cols);
	if(not_all_in_range)
	{
		index[0].x = ADDR_L(x-ALIGN,0,src_cols);
		index[0].x = ADDR_R(x-ALIGN,src_cols,index[0].x);
		index[0].y = ADDR_L(x-ALIGN+1,0,src_cols);
		index[0].y = ADDR_R(x-ALIGN+1,src_cols,index[0].y);
		index[0].z = ADDR_L(x-ALIGN+2,0,src_cols);
		index[0].z = ADDR_R(x-ALIGN+2,src_cols,index[0].z);
		index[0].w = ADDR_L(x-ALIGN+3,0,cols);
		index[0].w = ADDR_R(x-ALIGN+3,src_cols,index[0].w);
		index[1].x = ADDR_L(x+LSIZE0*4-ALIGN,0,src_cols);
		index[1].x = ADDR_R(x+LSIZE0*4-ALIGN,src_cols,index[1].x);
		index[1].y = ADDR_L(x+LSIZE0*4-ALIGN+1,0,src_cols);
		index[1].y = ADDR_R(x+LSIZE0*4-ALIGN+1,src_cols,index[1].y);
		index[1].z = ADDR_L(x+LSIZE0*4-ALIGN+2,0,src_cols);
		index[1].z = ADDR_R(x+LSIZE0*4-ALIGN+2,src_cols,index[1].z);
		index[1].w = ADDR_L(x+LSIZE0*4-ALIGN+3,0,src_cols);
		index[1].w = ADDR_R(x+LSIZE0*4-ALIGN+3,src_cols,index[1].w);
		index[0] = mad24(index[0],3,baserow);
		index[1] = mad24(index[1],3,baserow);
		temp[0].x =src[index[0].x];
		temp[0].y =src[index[0].x+1];
		temp[0].z =src[index[0].x+2];
		temp[0].w =src[index[0].y];
		temp[1].x =src[index[0].y+1];
		temp[1].y =src[index[0].y+2];
		temp[1].z =src[index[0].z];
		temp[1].w =src[index[0].z+1];
		temp[2].x =src[index[0].z+2];
		temp[2].y =src[index[0].w];
		temp[2].z =src[index[0].w+1];
		temp[2].w =src[index[0].w+2];
		temp[3].x =src[index[1].x];
		temp[3].y =src[index[1].x+1];
		temp[3].z =src[index[1].x+2];
		temp[3].w =src[index[1].y];
		temp[4].x =src[index[1].y+1];
		temp[4].y =src[index[1].y+2];
		temp[4].z =src[index[1].z];
		temp[4].w =src[index[1].z+1];
		temp[5].x =src[index[1].z+2];
		temp[5].y =src[index[1].w];
		temp[5].z =src[index[1].w+1];
		temp[5].w =src[index[1].w+2];
	}
	else
	{
		temp[0] = *(__global uchar4*)&src[baseindex-ALIGN*3];
		temp[1] = *(__global uchar4*)&src[baseindex-ALIGN*3+4];
		temp[2] = *(__global uchar4*)&src[baseindex-ALIGN*3+8];
		temp[3] = *(__global uchar4*)&src[baseindex+LSIZE0*12-ALIGN*3];
		temp[4] = *(__global uchar4*)&src[baseindex+LSIZE0*12-ALIGN*3+4];
		temp[5] = *(__global uchar4*)&src[baseindex+LSIZE0*12-ALIGN*3+8];
	}
	#endif
	LDS_DAT[l_y][l_x] = temp[0];
	LDS_DAT[l_y][l_x+1] = temp[1];
	LDS_DAT[l_y][l_x+2] = temp[2];
	LDS_DAT[l_y][l_x+LSIZE0*3] = temp[3];
	LDS_DAT[l_y][l_x+LSIZE0*3+1] = temp[4];
	LDS_DAT[l_y][l_x+LSIZE0*3+2] = temp[5];
	barrier(CLK_LOCAL_MEM_FENCE);
	sum[0] = 0.0f;
	sum[1] = 0.0f;
	sum[2] = 0.0f;
	for(;i<=2*RADIUS;i++)
	{
		float3 prefetch_LDS[4];
		prefetch_LDS[0]=convert_float3(vload3(0,(__local uchar*)&LDS_DAT[l_y][l_x]+ALIGN*3-RADIUS*3+3*i));
		prefetch_LDS[1]=convert_float3(vload3(0,(__local uchar*)&LDS_DAT[l_y][l_x]+ALIGN*3-RADIUS*3+3+3*i));
		prefetch_LDS[2]=convert_float3(vload3(0,(__local uchar*)&LDS_DAT[l_y][l_x]+ALIGN*3-RADIUS*3+6+3*i));
		prefetch_LDS[3]=convert_float3(vload3(0,(__local uchar*)&LDS_DAT[l_y][l_x]+ALIGN*3-RADIUS*3+9+3*i));
	
		sum[0].xyz += prefetch_LDS[0]*mat_kernel[i];
		sum[0].w += prefetch_LDS[1].x*mat_kernel[i];
		sum[1].xy += prefetch_LDS[1].yz*mat_kernel[i];
		sum[1].zw += prefetch_LDS[2].xy*mat_kernel[i];
		sum[2].x += prefetch_LDS[2].z*mat_kernel[i];
		sum[2].yzw += prefetch_LDS[3]*mat_kernel[i];

	}
	baseindex = mad24(y,dst_pix_per_row,x*3);
	if((x< dst_cols) & (y < dst_rows))
	{
		*(__global float4*)&dst[baseindex] = sum[0];
		*(__global float4*)&dst[baseindex+4] = sum[1];
		*(__global float4*)&dst[baseindex+8] = sum[2];
	}
}


__kernel __attribute__((reqd_work_group_size(LSIZE0,LSIZE1,1))) void row_filter_C4_D0
						(__global const uchar * restrict src, 
						 __global float * dst,
                         const int src_cols,
                         const int src_rows, 
                         const int src_pix_per_row, 
                         const int dst_cols, 
                         const int dst_rows,
                         const int dst_pix_per_row,
                         __constant float * mat_kernel __attribute__((max_constant_size(4*(2*RADIUS+1)))))
{
    int x = get_global_id(0);
    int y = get_global_id(1);
    int l_x = get_local_id(0);
    int l_y = get_local_id(1);
    int i=0;
    float4 sum=0.0f;
	int index[READ_TIMES_ROW];
	uchar4 temp[READ_TIMES_ROW];
	int baseindex = mad24(y,src_pix_per_row,x<<2);
	int baserow = mul24(y,src_pix_per_row);
	__local uchar4 LDS_DAT[LSIZE1][LSIZE0*READ_TIMES_ROW];
	#ifdef BORDER_CONSTANT
	for(int j=0;j<READ_TIMES_ROW;j++)
	{
		temp[j] = *(__global uchar4*)&src[baseindex+LSIZE0*4*j-ALIGN*4];
	}
	for(int j=0;j<READ_TIMES_ROW;j++)
	{
		temp[j] = ELEM(x+j*LSIZE0-ALIGN,0,src_cols,0,temp[j]);
	}
	#else	
	int not_all_in_range = (x-RADIUS<0) | (x+READ_TIMES_ROW*LSIZE0-ALIGN>src_cols);
	if(not_all_in_range)
	{
		for(int j=0;j<READ_TIMES_ROW;j++)
		{
			index[j] = ADDR_L(x+j*LSIZE0-ALIGN,0,src_cols);
			index[j] = ADDR_R(x+j*LSIZE0-ALIGN,src_cols,index[j]);
		}
		for(int j=0;j<READ_TIMES_ROW;j++)
		{
			temp[j] = *(__global uchar4*)&src[baserow+(index[j]<<2)];
		}
	}
	else
	{
		for(int j=0;j<READ_TIMES_ROW;j++)
		{
			temp[j] = *(__global uchar4*)&src[baseindex+((j*LSIZE0-ALIGN)<<2)];
		}
	}
	#endif
	for(int j=0;j<READ_TIMES_ROW;j++)
	{
		LDS_DAT[l_y][l_x+j*LSIZE0] = temp[j];
	}	
	barrier(CLK_LOCAL_MEM_FENCE);
	sum = convert_float4(LDS_DAT[l_y][l_x+RADIUS])*mat_kernel[RADIUS];
	for(i=1;i<=RADIUS;i++)
	{
		float4 prefetch_LDS[2];
		prefetch_LDS[0]=convert_float4(LDS_DAT[l_y][l_x+RADIUS-i]);
		prefetch_LDS[1]=convert_float4(LDS_DAT[l_y][l_x+RADIUS+i]);
		sum += prefetch_LDS[0]*mat_kernel[RADIUS-i]+prefetch_LDS[1]*mat_kernel[RADIUS+i];
	}
	baseindex = mad24(y,dst_pix_per_row,x<<2);
	if((x< dst_cols) & (y < dst_rows))
	{
		*(__global float4*)&dst[baseindex] = sum;
	}
}

/*
__kernel __attribute__((reqd_work_group_size(LSIZE0,LSIZE1,1))) void row_filter_C1_D5
						(__global const float * restrict src, 
						 __global float * dst,
                         const int src_cols,
                         const int src_rows, 
                         const int src_pix_per_row, 
                         const int dst_cols, 
                         const int dst_rows,
                         const int dst_pix_per_row,
                         __constant float * mat_kernel __attribute__((max_constant_size(4*(2*RADIUS+1)))))
{
    int x = get_global_id(0);
    int y = get_global_id(1);
    int l_x = get_local_id(0);
    int l_y = get_local_id(1);
    int i;
    float sum;

	float temp[2];
	int baseindex = mad24(y,src_pix_per_row,x);
	int baserow = mul24(y,src_pix_per_row);
	__local float LDS_DAT[LSIZE1][LSIZE0*2+1];
	#ifdef BORDER_CONSTANT
	temp[0] = src[baseindex-RADIUS];
	temp[1] = src[baseindex+LSIZE0-RADIUS];
	temp[0] = ELEM(x-RADIUS,0,src_cols,0.0f,temp[0]);
	temp[1] = ELEM(x+LSIZE0-RADIUS,0,src_cols,0.0f,temp[1]);
	#else	
	int index[2];
	index[0] = ADDR_L(x-RADIUS,0,src_cols);
	index[0] = ADDR_R(x-RADIUS,src_cols,index[0]);
	index[1] = ADDR_L(x+LSIZE0-RADIUS,0,src_cols);
	index[1] = ADDR_R(x+LSIZE0-RADIUS,src_cols,index[1]);
	temp[0] = src[baserow+index[0]];
	temp[1] = src[baserow+index[1]];
	#endif
	LDS_DAT[l_y][l_x] = temp[0];
	LDS_DAT[l_y][l_x+LSIZE0] = temp[1];
	barrier(CLK_LOCAL_MEM_FENCE);

	baseindex = mad24(y,dst_pix_per_row,x);
	sum =LDS_DAT[l_y][l_x+RADIUS]*mat_kernel[RADIUS];

	for(i=1;i<=RADIUS;i++)
	{
		temp[0]=LDS_DAT[l_y][l_x+RADIUS-i];
		temp[1]=LDS_DAT[l_y][l_x+RADIUS+i];
		sum += temp[0]*mat_kernel[RADIUS-i]+temp[1]*mat_kernel[RADIUS+i];
	}
	if((x< dst_cols) & (y < dst_rows))
	{
		dst[baseindex] = sum;
	}
}

__kernel __attribute__((reqd_work_group_size(LSIZE0,LSIZE1,1))) void row_filter_C1_D0
						(__global const uchar * restrict src, 
						 __global float * dst,
                         const int dst_cols,
                         const int dst_rows, 
						 const int src_whole_cols,
						 const int src_whole_rows,
                         const int src_step_in_pixel, 
                         const int src_offset_x, 
                         const int src_offset_y, 
                         const int dst_step_in_pixel,
                         const int radiusy,
                         __constant float * mat_kernel __attribute__((max_constant_size(4*(2*RADIUSX+1)))))
{
	int x = get_global_id(0);
	int y = get_global_id(1);
	int l_x = get_local_id(0);
	int l_y = get_local_id(1);
	int start_x = x+src_offset_x-RADIUSX;
	int start_y = y+src_offset_y-radiusy;
	int start_addr = mad24(start_y,src_step_in_pixel,start_x);
	int i;
	float sum;
	uchar temp[READ_TIMES_ROW];

	__local uchar LDS_DAT[LSIZE1][READ_TIMES_ROW*LSIZE0+1];
	#ifdef BORDER_CONSTANT
	//read pixels from src
	for(i = 0;i<READ_TIMES_ROW;i++)
	{
		temp[i] = src[start_addr+i*LSIZE0];
	}
	//judge if read out of boundary
	for(i = 0;i<READ_TIMES_ROW;i++)
	{
		temp[i]= ELEM(start_x+i*LSIZE0,0,src_whole_cols,0,temp[i]);
		temp[i]= ELEM(start_y,0,src_whole_rows,0,temp[i]);
	}
	#else
	int index[READ_TIMES_ROW];
	//judge if read out of boundary
	for(i = 0;i<READ_TIMES_ROW;i++)
	{
		index[i]= ADDR_L(start_x+i*LSIZE0,0,src_whole_cols,start_x+i*LSIZE0);
		index[i]= ADDR_R(start_x+i*LSIZE0,src_whole_cols,index[i]);
		index[i]= ADDR_L(start_y,0,src_whole_rows,index[i]);
		index[i]= ADDR_R(start_y,src_whole_rows,index[i]);
	}
	//read pixels from src
	for(i = 0;i<READ_TIMES_ROW;i++)
	{
		temp[i] = src[start_addr+i*LSIZE0];
	}	
	#endif

	//save pixels to lds
	for(i = 0;i<READ_TIMES_ROW;i++)
	{
		LDS_DAT[l_y][l_x+i*LSIZE0]=temp[i];
	}
	barrier(CLK_LOCAL_MEM_FENCE);

	//read pixels from lds and calculate the result
	sum =convert_float(LDS_DAT[l_y][l_x+RADIUSX])*mat_kernel[RADIUSX];
	for(i=1;i<=RADIUSX;i++)
	{
		temp[0]=LDS_DAT[l_y][l_x+RADIUSX-i];
		temp[1]=LDS_DAT[l_y][l_x+RADIUSX+i];
		sum += convert_float(temp[0])*mat_kernel[RADIUSX-i]+convert_float(temp[1])*mat_kernel[RADIUSX+i];
	}
	//write the result to dst
	if((x<dst_cols) & y<(dst_rows))
	{
		start_addr = mad24(y,dst_step_in_pixel,x);
		dst[start_addr] = sum;
	}
}*/
__kernel __attribute__((reqd_work_group_size(LSIZE0,LSIZE1,1))) void row_filter_C1_D0
						(__global const uchar * restrict src, 
						 __global float * dst,
                         const int dst_cols,
                         const int dst_rows, 
						 const int src_whole_cols,
						 const int src_whole_rows,
                         const int src_step_in_pixel, 
                         const int src_offset_x, 
                         const int src_offset_y, 
                         const int dst_step_in_pixel,
                         const int radiusy,
                         __constant float * mat_kernel __attribute__((max_constant_size(4*(2*RADIUSX+1)))))
{
	int x = get_global_id(0)<<2;
	int y = get_global_id(1);
	int l_x = get_local_id(0);
	int l_y = get_local_id(1);
	int start_x = x+src_offset_x-RADIUSX & 0xfffffffc;
	int offset = src_offset_x-RADIUSX & 3;
	int start_y = y+src_offset_y-radiusy;
	int start_addr = mad24(start_y,src_step_in_pixel,start_x);
	int i;
	float4 sum;
	uchar4 temp[READ_TIMES_ROW];

	__local uchar4 LDS_DAT[LSIZE1][READ_TIMES_ROW*LSIZE0+1];
	#ifdef BORDER_CONSTANT
	//read pixels from src
	for(i = 0;i<READ_TIMES_ROW;i++)
	{
		temp[i] = *(__global uchar4*)&src[start_addr+i*LSIZE0*4];
	}
	//judge if read out of boundary
	for(i = 0;i<READ_TIMES_ROW;i++)
	{
		temp[i].x= ELEM(start_x+i*LSIZE0*4,0,src_whole_cols,0,temp[i].x);
		temp[i].y= ELEM(start_x+i*LSIZE0*4+1,0,src_whole_cols,0,temp[i].y);
		temp[i].z= ELEM(start_x+i*LSIZE0*4+2,0,src_whole_cols,0,temp[i].z);
		temp[i].w= ELEM(start_x+i*LSIZE0*4+3,0,src_whole_cols,0,temp[i].w);
		temp[i]= ELEM(start_y,0,src_whole_rows,0,temp[i]);
	}
	#else
	int not_all_in_range = (start_x<0) | (start_x + READ_TIMES_ROW*LSIZE0*4+4>src_whole_cols)| (start_y<0) | (start_y >= src_whole_rows);
	int4 index[READ_TIMES_ROW];
	int4 addr;
	int s_y;
	if(not_all_in_range)
	{
		//judge if read out of boundary
		for(i = 0;i<READ_TIMES_ROW;i++)
		{
			index[i].x= ADDR_L(start_x+i*LSIZE0*4,0,src_whole_cols,start_x+i*LSIZE0*4);
			index[i].x= ADDR_R(start_x+i*LSIZE0*4,src_whole_cols,index[i].x);
			index[i].y= ADDR_L(start_x+i*LSIZE0*4+1,0,src_whole_cols,start_x+i*LSIZE0*4+1);
			index[i].y= ADDR_R(start_x+i*LSIZE0*4+1,src_whole_cols,index[i].y);
			index[i].z= ADDR_L(start_x+i*LSIZE0*4+2,0,src_whole_cols,start_x+i*LSIZE0*4+2);
			index[i].z= ADDR_R(start_x+i*LSIZE0*4+2,src_whole_cols,index[i].z);
			index[i].w= ADDR_L(start_x+i*LSIZE0*4+3,0,src_whole_cols,start_x+i*LSIZE0*4+3);
			index[i].w= ADDR_R(start_x+i*LSIZE0*4+3,src_whole_cols,index[i].w);
		}
		s_y= ADDR_L(start_y,0,src_whole_rows,start_y);
		s_y= ADDR_R(start_y,src_whole_rows,s_y);
		//read pixels from src
		for(i = 0;i<READ_TIMES_ROW;i++)
		{
			addr = mad24((int4)s_y,(int4)src_step_in_pixel,index[i]);
			temp[i].x = src[addr.x];
			temp[i].y = src[addr.y];
			temp[i].z = src[addr.z];
			temp[i].w = src[addr.w];
		}
	}
	else
	{
		//read pixels from src
		for(i = 0;i<READ_TIMES_ROW;i++)
		{
			temp[i] = *(__global uchar4*)&src[start_addr+i*LSIZE0*4];
		}	
	}	
	#endif

	//save pixels to lds
	for(i = 0;i<READ_TIMES_ROW;i++)
	{
		LDS_DAT[l_y][l_x+i*LSIZE0]=temp[i];
	}
	barrier(CLK_LOCAL_MEM_FENCE);

	//read pixels from lds and calculate the result
	sum =convert_float4(vload4(0,(__local uchar*)&LDS_DAT[l_y][l_x]+RADIUSX+offset))*mat_kernel[RADIUSX];
	for(i=1;i<=RADIUSX;i++)
	{
		temp[0]=vload4(0,(__local uchar*)&LDS_DAT[l_y][l_x]+RADIUSX+offset-i);
		temp[1]=vload4(0,(__local uchar*)&LDS_DAT[l_y][l_x]+RADIUSX+offset+i);
		sum += convert_float4(temp[0])*mat_kernel[RADIUSX-i]+convert_float4(temp[1])*mat_kernel[RADIUSX+i];
	}
	start_addr = mad24(y,dst_step_in_pixel,x);
	//write the result to dst
	if((x+3<dst_cols) & y<(dst_rows))
	{
		*(__global float4*)&dst[start_addr] = sum;
	}
	else if((x+2<dst_cols) & y<(dst_rows))
	{
		dst[start_addr] = sum.x;
		dst[start_addr+1] = sum.y;
		dst[start_addr+2] = sum.z;
	}
	else if((x+1<dst_cols) & y<(dst_rows))
	{
		dst[start_addr] = sum.x;
		dst[start_addr+1] = sum.y;
	}
	else if((x<dst_cols) & y<(dst_rows))
	{
		dst[start_addr] = sum.x;
	}
}
__kernel __attribute__((reqd_work_group_size(LSIZE0,LSIZE1,1))) void row_filter_C4_D0
						(__global const uchar4 * restrict src, 
						 __global float4 * dst,
                         const int dst_cols,
                         const int dst_rows, 
						 const int src_whole_cols,
						 const int src_whole_rows,
                         const int src_step_in_pixel, 
                         const int src_offset_x, 
                         const int src_offset_y, 
                         const int dst_step_in_pixel,
                         const int radiusy,
                         __constant float * mat_kernel __attribute__((max_constant_size(4*(2*RADIUSX+1)))))
{
	int x = get_global_id(0);
	int y = get_global_id(1);
	int l_x = get_local_id(0);
	int l_y = get_local_id(1);
	int start_x = x+src_offset_x-RADIUSX;
	int start_y = y+src_offset_y-radiusy;
	int start_addr = mad24(start_y,src_step_in_pixel,start_x);
	int i;
	float4 sum;
	uchar4 temp[READ_TIMES_ROW];

	__local uchar4 LDS_DAT[LSIZE1][READ_TIMES_ROW*LSIZE0+1];
	#ifdef BORDER_CONSTANT
	//read pixels from src
	for(i = 0;i<READ_TIMES_ROW;i++)
	{
		temp[i] = src[start_addr+i*LSIZE0];
	}
	//judge if read out of boundary
	for(i = 0;i<READ_TIMES_ROW;i++)
	{
		temp[i]= ELEM(start_x+i*LSIZE0,0,src_whole_cols,0,temp[i]);
		temp[i]= ELEM(start_y,0,src_whole_rows,0,temp[i]);
	}
	#else
	int index[READ_TIMES_ROW];
	int s_x,s_y;
	//judge if read out of boundary
	for(i = 0;i<READ_TIMES_ROW;i++)
	{
		s_x= ADDR_L(start_x+i*LSIZE0,0,src_whole_cols,start_x+i*LSIZE0);
		s_x= ADDR_R(start_x+i*LSIZE0,src_whole_cols,s_x);
		s_y= ADDR_L(start_y,0,src_whole_rows,start_y);
		s_y= ADDR_R(start_y,src_whole_rows,s_y);
		index[i]=mad24(s_y,src_step_in_pixel,s_x);
	}
	//read pixels from src
	for(i = 0;i<READ_TIMES_ROW;i++)
	{
		temp[i] = src[index[i]];
	}	
	#endif

	//save pixels to lds
	for(i = 0;i<READ_TIMES_ROW;i++)
	{
		LDS_DAT[l_y][l_x+i*LSIZE0]=temp[i];
	}
	barrier(CLK_LOCAL_MEM_FENCE);

	//read pixels from lds and calculate the result
	sum =convert_float4(LDS_DAT[l_y][l_x+RADIUSX])*mat_kernel[RADIUSX];
	for(i=1;i<=RADIUSX;i++)
	{
		temp[0]=LDS_DAT[l_y][l_x+RADIUSX-i];
		temp[1]=LDS_DAT[l_y][l_x+RADIUSX+i];
		sum += convert_float4(temp[0])*mat_kernel[RADIUSX-i]+convert_float4(temp[1])*mat_kernel[RADIUSX+i];
	}
	//write the result to dst
	if((x<dst_cols) & y<(dst_rows))
	{
		start_addr = mad24(y,dst_step_in_pixel,x);
		dst[start_addr] = sum;
	}
}

__kernel __attribute__((reqd_work_group_size(LSIZE0,LSIZE1,1))) void row_filter_C1_D5
						(__global const float * restrict src, 
						 __global float * dst,
                         const int dst_cols,
                         const int dst_rows, 
						 const int src_whole_cols,
						 const int src_whole_rows,
                         const int src_step_in_pixel, 
                         const int src_offset_x, 
                         const int src_offset_y, 
                         const int dst_step_in_pixel,
                         const int radiusy,
                         __constant float * mat_kernel __attribute__((max_constant_size(4*(2*RADIUSX+1)))))
{
	int x = get_global_id(0);
	int y = get_global_id(1);
	int l_x = get_local_id(0);
	int l_y = get_local_id(1);
	int start_x = x+src_offset_x-RADIUSX;
	int start_y = y+src_offset_y-radiusy;
	int start_addr = mad24(start_y,src_step_in_pixel,start_x);
	int i;
	float sum;
	float temp[READ_TIMES_ROW];

	__local float LDS_DAT[LSIZE1][READ_TIMES_ROW*LSIZE0+1];
	#ifdef BORDER_CONSTANT
	//read pixels from src
	for(i = 0;i<READ_TIMES_ROW;i++)
	{
		temp[i] = src[start_addr+i*LSIZE0];
	}
	//judge if read out of boundary
	for(i = 0;i<READ_TIMES_ROW;i++)
	{
		temp[i]= ELEM(start_x+i*LSIZE0,0,src_whole_cols,0,temp[i]);
		temp[i]= ELEM(start_y,0,src_whole_rows,0,temp[i]);
	}
	#else
	int index[READ_TIMES_ROW];
	int s_x,s_y;
	//judge if read out of boundary
	for(i = 0;i<READ_TIMES_ROW;i++)
	{
		s_x= ADDR_L(start_x+i*LSIZE0,0,src_whole_cols,start_x+i*LSIZE0);
		s_x= ADDR_R(start_x+i*LSIZE0,src_whole_cols,s_x);
		s_y= ADDR_L(start_y,0,src_whole_rows,start_y);
		s_y= ADDR_R(start_y,src_whole_rows,s_y);
		index[i]=mad24(s_y,src_step_in_pixel,s_x);
	}
	//read pixels from src
	for(i = 0;i<READ_TIMES_ROW;i++)
	{
		temp[i] = src[index[i]];
	}	
	#endif

	//save pixels to lds
	for(i = 0;i<READ_TIMES_ROW;i++)
	{
		LDS_DAT[l_y][l_x+i*LSIZE0]=temp[i];
	}
	barrier(CLK_LOCAL_MEM_FENCE);

	//read pixels from lds and calculate the result
	sum =LDS_DAT[l_y][l_x+RADIUSX]*mat_kernel[RADIUSX];
	for(i=1;i<=RADIUSX;i++)
	{
		temp[0]=LDS_DAT[l_y][l_x+RADIUSX-i];
		temp[1]=LDS_DAT[l_y][l_x+RADIUSX+i];
		sum += temp[0]*mat_kernel[RADIUSX-i]+temp[1]*mat_kernel[RADIUSX+i];
	}
	//write the result to dst
	if((x<dst_cols) & y<(dst_rows))
	{
		start_addr = mad24(y,dst_step_in_pixel,x);
		dst[start_addr] = sum;
	}
}

__kernel __attribute__((reqd_work_group_size(LSIZE0,LSIZE1,1))) void row_filter_C4_D5
						(__global const float4 * restrict src, 
						 __global float4 * dst,
                         const int dst_cols,
                         const int dst_rows, 
						 const int src_whole_cols,
						 const int src_whole_rows,
                         const int src_step_in_pixel, 
                         const int src_offset_x, 
                         const int src_offset_y, 
                         const int dst_step_in_pixel,
                         const int radiusy,
                         __constant float * mat_kernel __attribute__((max_constant_size(4*(2*RADIUSX+1)))))
{
	int x = get_global_id(0);
	int y = get_global_id(1);
	int l_x = get_local_id(0);
	int l_y = get_local_id(1);
	int start_x = x+src_offset_x-RADIUSX;
	int start_y = y+src_offset_y-radiusy;
	int start_addr = mad24(start_y,src_step_in_pixel,start_x);
	int i;
	float4 sum;
	float4 temp[READ_TIMES_ROW];

	__local float4 LDS_DAT[LSIZE1][READ_TIMES_ROW*LSIZE0+1];
	#ifdef BORDER_CONSTANT
	//read pixels from src
	for(i = 0;i<READ_TIMES_ROW;i++)
	{
		temp[i] = src[start_addr+i*LSIZE0];
	}
	//judge if read out of boundary
	for(i = 0;i<READ_TIMES_ROW;i++)
	{
		temp[i]= ELEM(start_x+i*LSIZE0,0,src_whole_cols,0,temp[i]);
		temp[i]= ELEM(start_y,0,src_whole_rows,0,temp[i]);
	}
	#else
	int index[READ_TIMES_ROW];
	int s_x,s_y;
	//judge if read out of boundary
	for(i = 0;i<READ_TIMES_ROW;i++)
	{
		s_x= ADDR_L(start_x+i*LSIZE0,0,src_whole_cols,start_x+i*LSIZE0);
		s_x= ADDR_R(start_x+i*LSIZE0,src_whole_cols,s_x);
		s_y= ADDR_L(start_y,0,src_whole_rows,start_y);
		s_y= ADDR_R(start_y,src_whole_rows,s_y);
		index[i]=mad24(s_y,src_step_in_pixel,s_x);
	}
	//read pixels from src
	for(i = 0;i<READ_TIMES_ROW;i++)
	{
		temp[i] = src[index[i]];
	}	
	#endif

	//save pixels to lds
	for(i = 0;i<READ_TIMES_ROW;i++)
	{
		LDS_DAT[l_y][l_x+i*LSIZE0]=temp[i];
	}
	barrier(CLK_LOCAL_MEM_FENCE);

	//read pixels from lds and calculate the result
	sum =LDS_DAT[l_y][l_x+RADIUSX]*mat_kernel[RADIUSX];
	for(i=1;i<=RADIUSX;i++)
	{
		temp[0]=LDS_DAT[l_y][l_x+RADIUSX-i];
		temp[1]=LDS_DAT[l_y][l_x+RADIUSX+i];
		sum += temp[0]*mat_kernel[RADIUSX-i]+temp[1]*mat_kernel[RADIUSX+i];
	}
	//write the result to dst
	if((x<dst_cols) & y<(dst_rows))
	{
		start_addr = mad24(y,dst_step_in_pixel,x);
		dst[start_addr] = sum;
	}
}


