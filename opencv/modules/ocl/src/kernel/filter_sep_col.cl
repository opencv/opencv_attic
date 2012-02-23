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
//

#define READ_TIMES_COL ((2*(RADIUSY+LSIZE1)-1)/LSIZE1)
#define RADIUS 1
#if CN ==1
#define ALIGN (((RADIUS)+3)>>2<<2)
#elif CN==2
#define ALIGN (((RADIUS)+1)>>1<<1)
#elif CN==3
#define ALIGN (((RADIUS)+3)>>2<<2)
#elif CN==4
#define ALIGN (RADIUS)
#define READ_TIMES_ROW ((2*(RADIUS+LSIZE0)-1)/LSIZE0)
#endif

#ifdef BORDER_CONSTANT
//BORDER_CONSTANT:      iiiiii|abcdefgh|iiiiiii
#define ELEM(i,l_edge,r_edge,elem1,elem2) (i)<(l_edge) | (i) >= (r_edge) ? (elem1) : (elem2)
#endif

#ifdef BORDER_REPLICATE
//BORDER_REPLICATE:     aaaaaa|abcdefgh|hhhhhhh
#define ADDR_L(i,l_edge,r_edge)  (i) < (l_edge) ? (l_edge) : (i)
#define ADDR_R(i,r_edge,addr)   (i) >= (r_edge) ? (r_edge)-1 : (addr)
#endif

#ifdef BORDER_REFLECT
//BORDER_REFLECT:       fedcba|abcdefgh|hgfedcb
#define ADDR_L(i,l_edge,r_edge)  (i) < (l_edge) ? -(i)-1 : (i)
#define ADDR_R(i,r_edge,addr) (i) >= (r_edge) ? -(i)-1+((r_edge)<<1) : (addr)
#endif

#ifdef BORDER_REFLECT_101
//BORDER_REFLECT_101:   gfedcb|abcdefgh|gfedcba
#define ADDR_L(i,l_edge,r_edge)  (i) < (l_edge) ? -(i) : (i)
#define ADDR_R(i,r_edge,addr) (i) >= (r_edge) ? -(i)-2+((r_edge)<<1) : (addr)
#endif

#ifdef BORDER_WRAP
//BORDER_WRAP:          cdefgh|abcdefgh|abcdefg
#define ADDR_L(i,l_edge,r_edge)  (i) < (l_edge) ? (i)+(r_edge) : (i)
#define ADDR_R(i,r_edge,addr)   (i) >= (r_edge) ?   (i)-(r_edge) : (addr)
#endif


/**********************************************************************************
These kernels are written for separable filters such as Sobel, Scharr, GaussianBlur.
Now(6/29/2011) the kernels only support 8U data type and the anchor of the convovle
kernel must be in the center. ROI is not supported either.
Each kernels read 4 elements(not 4 pixels), save them to LDS and read the data needed
from LDS to calculate the result.
The length of the convovle kernel supported is only related to the MAX size of LDS, 
which is HW related.
Niko
6/29/2011
***********************************************************************************/
/*
__kernel __attribute__((reqd_work_group_size(LSIZE0,LSIZE1,1)))void col_filter_C1_D0
						(__global const float * restrict src,
						 __global uchar * dst,
                         const int cols,
                         const int rows, 
						 //const int src_whole_cols,
						 const int src_whole_rows,
                         const int src_step_in_pixel, 
                         const int src_offset_x, 
                         const int src_offset_y, 
                         const int dst_step_in_pixel,
                         const int dst_offset_in_pixel,
                         __constant float * mat_kernel __attribute__((max_constant_size(4*(2*RADIUS+1)))))
{
    int x = get_global_id(0)<<2;
    int y = get_global_id(1);
    int l_x = get_local_id(0)<<2;
    int l_y = get_local_id(1);   
	int i=0;

	float4 temp[READ_TIMES_COL];
	int baseindex[READ_TIMES_COL];
	for(int j=0;j<READ_TIMES_COL;j++)
	{
		baseindex[j]=mad24(y+src_offset_y+j*LSIZE1-RADIUS,src_step_in_pixel,x+src_offset_x);
	}
    float4 sum = 0.0f;
	__local float LDS_DAT[READ_TIMES_COL*LSIZE1][LSIZE0*4+1];
	#ifdef BORDER_CONSTANT
	for(int j=0;j<READ_TIMES_COL;j++)
	{
		temp[j] = vload4(0,src+baseindex[j]);
	}
	for(int j=0;j<READ_TIMES_COL;j++)
	{
		temp[j] = ELEM(y+src_offset_y+j*LSIZE1-RADIUS,0,src_whole_rows,0.0f,temp[j]);
	}
	#else
	int not_all_in_range = (y+src_offset_y-RADIUS<0) | (y+src_offset_y+(READ_TIMES_COL-1)*LSIZE1-RADIUS>=src_whole_rows);
	int index[READ_TIMES_COL];
	if(not_all_in_range)
	{
		for(int j=0;j<READ_TIMES_COL;j++)
		{
			index[j] = ADDR_L(y+src_offset_y+j*LSIZE1-RADIUS,0,src_whole_rows);
			index[j] = ADDR_R(y+src_offset_y+j*LSIZE1-RADIUS,src_whole_rows,index[j]);
		}
		for(int j=0;j<READ_TIMES_COL;j++)
		{
			baseindex[j]=mad24(index[j],src_step_in_pixel,x+src_offset_x);
		}
	}
	for(int j=0;j<READ_TIMES_COL;j++)
	{
		temp[j] = *(__global float4*)(src+baseindex[j]);
	}
	#endif
	for(int j=0;j<READ_TIMES_COL;j++)
	{
		vstore4(temp[j],0,&LDS_DAT[l_y+j*LSIZE1][l_x]);
	}	

    barrier(CLK_LOCAL_MEM_FENCE);    
	sum = vload4(0,&LDS_DAT[l_y+RADIUS][l_x])*mat_kernel[RADIUS];
	float4 prefetch_LDS[2];
	for(i=1;i<=RADIUS;i++)
	{
		prefetch_LDS[0]=vload4(0,&LDS_DAT[l_y+RADIUS-i][l_x]);
		prefetch_LDS[1]=vload4(0,&LDS_DAT[l_y+RADIUS+i][l_x]);
		sum += prefetch_LDS[0] * mat_kernel[RADIUS-i]+prefetch_LDS[1] * mat_kernel[RADIUS+i];
	}
    barrier(CLK_LOCAL_MEM_FENCE);   
	vstore4(sum,0,&LDS_DAT[l_y][l_x]);
    barrier(CLK_LOCAL_MEM_FENCE); 
	i = mad24(y,dst_step_in_pixel,x+dst_offset_in_pixel & 0xfffffffc);
	int off = dst_offset_in_pixel & 3;
	//uchar4 out = convert_uchar4_sat(sum);
	if((x< cols) & (y < rows))
	{
		uchar4 out = *(__global uchar4*)&dst[i];
		uchar4 temp = convert_uchar4_sat(vload4(0,&LDS_DAT[l_y][l_x-off]));
		size_t dst_addr_start = mad24(y,dst_step_in_pixel,dst_offset_in_pixel);
		size_t dst_addr_end = mad24(y,dst_step_in_pixel,cols+dst_offset_in_pixel);
		out.x = (i>=dst_addr_start)&(i<dst_addr_end) ? temp.x : out.x;
		out.y = (i+1>=dst_addr_start)&(i+1<dst_addr_end) ? temp.y : out.y;
		out.z = (i+2>=dst_addr_start)&(i+2<dst_addr_end) ? temp.z : out.z;
		out.w = (i+3>=dst_addr_start)&(i+3<dst_addr_end) ? temp.w : out.w;
		*(__global uchar4*)&dst[i] = out;
	}
}

__kernel __attribute__((reqd_work_group_size(LSIZE0,LSIZE1,1)))void col_filter_C2_D0
						(__global const float * restrict src,
						 __global uchar * dst,
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
    int l_x = get_local_id(0)<<2;
    int l_y = get_local_id(1);   
	int i=0;
	int index[READ_TIMES_COL];
	float4 temp[READ_TIMES_COL];
	int baseindex[READ_TIMES_COL];
	for(int j=0;j<READ_TIMES_COL;j++)
	{
		baseindex[j]=mad24(y+j*LSIZE1-RADIUS,src_pix_per_row,x);
	}
    float4 sum = 0.0f;
	__local float LDS_DAT[READ_TIMES_COL*LSIZE1][LSIZE0*4+1];
	#ifdef BORDER_CONSTANT
	for(int j=0;j<READ_TIMES_COL;j++)
	{
		temp[j] = *(__global float4*)(src+baseindex[j]);
	}
	for(int j=0;j<READ_TIMES_COL;j++)
	{
		temp[j] = ELEM(y+j*LSIZE1-RADIUS,0,src_rows,0.0f,temp[j]);
	}
	#else
	int not_all_in_range = (y-RADIUS<0) | (y+(READ_TIMES_COL-1)*LSIZE1-RADIUS>=src_rows);
	if(not_all_in_range)
	{
		for(int j=0;j<READ_TIMES_COL;j++)
		{
			index[j] = ADDR_L(y+j*LSIZE1-RADIUS,0,src_rows);
			index[j] = ADDR_R(y+j*LSIZE1-RADIUS,src_rows,index[j]);
		}
		for(int j=0;j<READ_TIMES_COL;j++)
		{
			baseindex[j]=mad24(index[j],src_pix_per_row,x);
		}
	}
	for(int j=0;j<READ_TIMES_COL;j++)
	{
		temp[j] = *(__global float4*)(src+baseindex[j]);
	}
	#endif
	for(int j=0;j<READ_TIMES_COL;j++)
	{
		vstore4(temp[j],0,&LDS_DAT[l_y+j*LSIZE1][l_x]);
	}	

    barrier(CLK_LOCAL_MEM_FENCE);    

	sum = vload4(0,&LDS_DAT[l_y+RADIUS][l_x])*mat_kernel[RADIUS];
	float4 prefetch_LDS[2];
	for(i=1;i<=RADIUS;i++)
	{
		prefetch_LDS[0]=vload4(0,&LDS_DAT[l_y+RADIUS-i][l_x]);
		prefetch_LDS[1]=vload4(0,&LDS_DAT[l_y+RADIUS+i][l_x]);
		sum += prefetch_LDS[0] * mat_kernel[RADIUS-i]+prefetch_LDS[1] * mat_kernel[RADIUS+i];
	}
	i = mad24(y,dst_pix_per_row,x);
	uchar4 out = convert_uchar4_sat(sum);
	if((x< (dst_cols<<1)) & (y < dst_rows))
	{
		*(__global uchar4*)&dst[i] = out;
	}
}


__kernel __attribute__((reqd_work_group_size(LSIZE0,LSIZE1,1)))void col_filter_C3_D0
						(__global const float * restrict src,
						 __global uchar * dst,
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
    int l_x = get_local_id(0)<<2;
    int l_y = get_local_id(1);   
	int i=0;
	int index[READ_TIMES_COL];
	float4 temp[READ_TIMES_COL];
	int baseindex[READ_TIMES_COL];
	for(int j=0;j<READ_TIMES_COL;j++)
	{
		baseindex[j]=mad24(y+j*LSIZE1-RADIUS,src_pix_per_row,x);
	}
    float4 sum = 0.0f;
	__local float LDS_DAT[READ_TIMES_COL*LSIZE1][LSIZE0*4+1];
	#ifdef BORDER_CONSTANT
	for(int j=0;j<READ_TIMES_COL;j++)
	{
		temp[j] = *(__global float4*)(src+baseindex[j]);
	}
	for(int j=0;j<READ_TIMES_COL;j++)
	{
		temp[j] = ELEM(y+j*LSIZE1-RADIUS,0,src_rows,0.0f,temp[j]);
	}
	#else
	int not_all_in_range = (y-RADIUS<0) | (y+(READ_TIMES_COL-1)*LSIZE1-RADIUS>=src_rows);
	if(not_all_in_range)
	{
		for(int j=0;j<READ_TIMES_COL;j++)
		{
			index[j] = ADDR_L(y+j*LSIZE1-RADIUS,0,src_rows);
			index[j] = ADDR_R(y+j*LSIZE1-RADIUS,src_rows,index[j]);
		}
		for(int j=0;j<READ_TIMES_COL;j++)
		{
			baseindex[j]=mad24(index[j],src_pix_per_row,x);
		}
	}
	for(int j=0;j<READ_TIMES_COL;j++)
	{
		temp[j] = *(__global float4*)(src+baseindex[j]);
	}
	#endif
	for(int j=0;j<READ_TIMES_COL;j++)
	{
		vstore4(temp[j],0,&LDS_DAT[l_y+j*LSIZE1][l_x]);
	}	

    barrier(CLK_LOCAL_MEM_FENCE);    

	for(;i<=2*RADIUS;i++)
	{
		float4 prefetch_LDS=vload4(0,&LDS_DAT[l_y+i][l_x]);
		sum += prefetch_LDS * mat_kernel[i];
	}
	i = mad24(y,dst_pix_per_row,x);
	uchar4 out = convert_uchar4_sat(sum);
	if((x< dst_cols*3) & (y < dst_rows))
	{
		*(__global uchar4*)&dst[i] = out;
	}
}


__kernel __attribute__((reqd_work_group_size(LSIZE0,LSIZE1,1)))void col_filter_C4_D0
						(__global const float * restrict src,
						 __global uchar * dst,
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
    int l_x = get_local_id(0)<<2;
    int l_y = get_local_id(1);   
	int i=0;
	int index[READ_TIMES_COL];
	float4 temp[READ_TIMES_COL];
	int baseindex[READ_TIMES_COL];
	for(int j=0;j<READ_TIMES_COL;j++)
	{
		baseindex[j]=mad24(y+j*LSIZE1-RADIUS,src_pix_per_row,x);
	}
    float4 sum = 0.0f;
	__local float LDS_DAT[READ_TIMES_COL*LSIZE1][LSIZE0*4+1];
	#ifdef BORDER_CONSTANT
	for(int j=0;j<READ_TIMES_COL;j++)
	{
		temp[j] = *(__global float4*)(src+baseindex[j]);
	}
	for(int j=0;j<READ_TIMES_COL;j++)
	{
		temp[j] = ELEM(y+j*LSIZE1-RADIUS,0,src_rows,0.0f,temp[j]);
	}
	#else
	int not_all_in_range = (y-RADIUS<0) | (y+(READ_TIMES_COL-1)*LSIZE1-RADIUS>=src_rows);
	if(not_all_in_range)
	{
		for(int j=0;j<READ_TIMES_COL;j++)
		{
			index[j] = ADDR_L(y+j*LSIZE1-RADIUS,0,src_rows);
			index[j] = ADDR_R(y+j*LSIZE1-RADIUS,src_rows,index[j]);
		}
		for(int j=0;j<READ_TIMES_COL;j++)
		{
			baseindex[j]=mad24(index[j],src_pix_per_row,x);
		}
	}
	for(int j=0;j<READ_TIMES_COL;j++)
	{
		temp[j] = *(__global float4*)(src+baseindex[j]);
	}
	#endif
	for(int j=0;j<READ_TIMES_COL;j++)
	{
		vstore4(temp[j],0,&LDS_DAT[l_y+j*LSIZE1][l_x]);
	}	

    barrier(CLK_LOCAL_MEM_FENCE);    

	sum = vload4(0,&LDS_DAT[l_y+RADIUS][l_x])*mat_kernel[RADIUS];
	float4 prefetch_LDS[2];
	for(i=1;i<=RADIUS;i++)
	{
		prefetch_LDS[0]=vload4(0,&LDS_DAT[l_y+RADIUS-i][l_x]);
		prefetch_LDS[1]=vload4(0,&LDS_DAT[l_y+RADIUS+i][l_x]);
		sum += prefetch_LDS[0] * mat_kernel[RADIUS-i]+prefetch_LDS[1] * mat_kernel[RADIUS+i];
	}
	i = mad24(y,dst_pix_per_row,x);
	uchar4 out = convert_uchar4_sat(sum);
	if((x< (dst_cols<<2)) & (y < dst_rows))
	{
		*(__global uchar4*)&dst[i] = out;
	}
}


/*
__kernel __attribute__((reqd_work_group_size(LSIZE0,LSIZE1,1)))void col_filter_C1_D5
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
	float temp[READ_TIMES_COL];
	int baseindex[READ_TIMES_COL];
	for(int j=0;j<READ_TIMES_COL;j++)
	{
		baseindex[j]=mad24(y+j*LSIZE1-RADIUS,src_pix_per_row,x);
	}

	__local float LDS_DAT[READ_TIMES_COL*LSIZE1][LSIZE0+1];
	#ifdef BORDER_CONSTANT
	for(int j=0;j<READ_TIMES_COL;j++)
	{
		temp[j] = src[baseindex[j]];
	}
	for(int j=0;j<READ_TIMES_COL;j++)
	{
		temp[j] = ELEM(y+j*LSIZE1-RADIUS,0,src_rows,0.0f,temp[j]);
	}
	#else
	int index[READ_TIMES_COL];
	int not_all_in_range = (y-RADIUS<0) | (y+(READ_TIMES_COL-1)*LSIZE1-RADIUS>=src_rows);
	if(not_all_in_range)
	{
		for(int j=0;j<READ_TIMES_COL;j++)
		{
			index[j] = ADDR_L(y+j*LSIZE1-RADIUS,0,src_rows);
			index[j] = ADDR_R(y+j*LSIZE1-RADIUS,src_rows,index[j]);
		}
		for(int j=0;j<READ_TIMES_COL;j++)
		{
			baseindex[j]=mad24(index[j],src_pix_per_row,x);
		}
	}
	for(int j=0;j<READ_TIMES_COL;j++)
	{
		temp[j] = src[baseindex[j]];
	}
	#endif
	for(int j=0;j<READ_TIMES_COL;j++)
	{
		LDS_DAT[l_y+j*LSIZE1][l_x]=temp[j];
	}	

    barrier(CLK_LOCAL_MEM_FENCE);    
	sum = LDS_DAT[l_y+RADIUS][l_x]*mat_kernel[RADIUS];

	for(i=1;i<=RADIUS;i++)
	{
		temp[0]=LDS_DAT[l_y+RADIUS-i][l_x];
		temp[1]=LDS_DAT[l_y+RADIUS+i][l_x];
		sum += temp[0] * mat_kernel[RADIUS-i]+temp[1] * mat_kernel[RADIUS+i];
	}
	i = mad24(y,dst_pix_per_row,x);
	if((x< dst_cols) & (y < dst_rows))
	{
		dst[i] = sum;
	}
}
*/

__kernel __attribute__((reqd_work_group_size(LSIZE0,LSIZE1,1))) void col_filter_C1_D0
						(__global const float * restrict src, 
						 __global uchar * dst,
                         const int dst_cols,
                         const int dst_rows, 
						 //const int src_whole_cols,
						 //const int src_whole_rows,
                         const int src_step_in_pixel, 
                         //const int src_offset_x, 
                         //const int src_offset_y, 
                         const int dst_step_in_pixel,
                         const int dst_offset_in_pixel,
                         __constant float * mat_kernel __attribute__((max_constant_size(4*(2*RADIUSY+1)))))
{
	int x = get_global_id(0);
	int y = get_global_id(1);
	int l_x = get_local_id(0);
	int l_y = get_local_id(1);
	int start_addr = mad24(y,src_step_in_pixel,x);
	int i;
	float sum;
	float temp[READ_TIMES_COL];

	__local float LDS_DAT[LSIZE1*READ_TIMES_COL][LSIZE0+1];

	//read pixels from src
	for(i = 0;i<READ_TIMES_COL;i++)
	{
		temp[i] = src[start_addr+i*LSIZE1*src_step_in_pixel];
	}
	//save pixels to lds
	for(i = 0;i<READ_TIMES_COL;i++)
	{
		LDS_DAT[l_y+i*LSIZE1][l_x] = temp[i];
	}
	barrier(CLK_LOCAL_MEM_FENCE);
	//read pixels from lds and calculate the result
	sum = LDS_DAT[l_y+RADIUSY][l_x]*mat_kernel[RADIUSY];
	for(i=1;i<=RADIUSY;i++)
	{
		temp[0]=LDS_DAT[l_y+RADIUSY-i][l_x];
		temp[1]=LDS_DAT[l_y+RADIUSY+i][l_x];
		sum += temp[0] * mat_kernel[RADIUSY-i]+temp[1] * mat_kernel[RADIUSY+i];
	}
	//write the result to dst
	if((x<dst_cols) & y<(dst_rows))
	{
		start_addr = mad24(y,dst_step_in_pixel,x+dst_offset_in_pixel);
		dst[start_addr] = convert_uchar_sat(sum);
	}
}

__kernel __attribute__((reqd_work_group_size(LSIZE0,LSIZE1,1))) void col_filter_C4_D0
						(__global const float4 * restrict src, 
						 __global uchar4 * dst,
                         const int dst_cols,
                         const int dst_rows, 
						 //const int src_whole_cols,
						 //const int src_whole_rows,
                         const int src_step_in_pixel, 
                         //const int src_offset_x, 
                         //const int src_offset_y, 
                         const int dst_step_in_pixel,
                         const int dst_offset_in_pixel,
                         __constant float * mat_kernel __attribute__((max_constant_size(4*(2*RADIUSY+1)))))
{
	int x = get_global_id(0);
	int y = get_global_id(1);
	int l_x = get_local_id(0);
	int l_y = get_local_id(1);
	int start_addr = mad24(y,src_step_in_pixel,x);
	int i;
	float4 sum;
	float4 temp[READ_TIMES_COL];

	__local float4 LDS_DAT[LSIZE1*READ_TIMES_COL][LSIZE0+1];

	//read pixels from src
	for(i = 0;i<READ_TIMES_COL;i++)
	{
		temp[i] = src[start_addr+i*LSIZE1*src_step_in_pixel];
	}
	//save pixels to lds
	for(i = 0;i<READ_TIMES_COL;i++)
	{
		LDS_DAT[l_y+i*LSIZE1][l_x] = temp[i];
	}
	barrier(CLK_LOCAL_MEM_FENCE);
	//read pixels from lds and calculate the result
	sum = LDS_DAT[l_y+RADIUSY][l_x]*mat_kernel[RADIUSY];
	for(i=1;i<=RADIUSY;i++)
	{
		temp[0]=LDS_DAT[l_y+RADIUSY-i][l_x];
		temp[1]=LDS_DAT[l_y+RADIUSY+i][l_x];
		sum += temp[0] * mat_kernel[RADIUSY-i]+temp[1] * mat_kernel[RADIUSY+i];
	}
	//write the result to dst
	if((x<dst_cols) & y<(dst_rows))
	{
		start_addr = mad24(y,dst_step_in_pixel,x+dst_offset_in_pixel);
		dst[start_addr] = convert_uchar4_sat(sum);
	}
}

__kernel __attribute__((reqd_work_group_size(LSIZE0,LSIZE1,1))) void col_filter_C1_D5
						(__global const float * restrict src, 
						 __global float * dst,
                         const int dst_cols,
                         const int dst_rows, 
						 //const int src_whole_cols,
						 //const int src_whole_rows,
                         const int src_step_in_pixel, 
                         //const int src_offset_x, 
                         //const int src_offset_y, 
                         const int dst_step_in_pixel,
                         const int dst_offset_in_pixel,
                         __constant float * mat_kernel __attribute__((max_constant_size(4*(2*RADIUSY+1)))))
{
	int x = get_global_id(0);
	int y = get_global_id(1);
	int l_x = get_local_id(0);
	int l_y = get_local_id(1);
	int start_addr = mad24(y,src_step_in_pixel,x);
	int i;
	float sum;
	float temp[READ_TIMES_COL];

	__local float LDS_DAT[LSIZE1*READ_TIMES_COL][LSIZE0+1];

	//read pixels from src
	for(i = 0;i<READ_TIMES_COL;i++)
	{
		temp[i] = src[start_addr+i*LSIZE1*src_step_in_pixel];
	}
	//save pixels to lds
	for(i = 0;i<READ_TIMES_COL;i++)
	{
		LDS_DAT[l_y+i*LSIZE1][l_x] = temp[i];
	}
	barrier(CLK_LOCAL_MEM_FENCE);
	//read pixels from lds and calculate the result
	sum = LDS_DAT[l_y+RADIUSY][l_x]*mat_kernel[RADIUSY];
	for(i=1;i<=RADIUSY;i++)
	{
		temp[0]=LDS_DAT[l_y+RADIUSY-i][l_x];
		temp[1]=LDS_DAT[l_y+RADIUSY+i][l_x];
		sum += temp[0] * mat_kernel[RADIUSY-i]+temp[1] * mat_kernel[RADIUSY+i];
	}
	//write the result to dst
	if((x<dst_cols) & y<(dst_rows))
	{
		start_addr = mad24(y,dst_step_in_pixel,x+dst_offset_in_pixel);
		dst[start_addr] = sum;
	}
}
__kernel __attribute__((reqd_work_group_size(LSIZE0,LSIZE1,1))) void col_filter_C4_D5
						(__global const float4 * restrict src, 
						 __global float4 * dst,
                         const int dst_cols,
                         const int dst_rows, 
						 //const int src_whole_cols,
						 //const int src_whole_rows,
                         const int src_step_in_pixel, 
                         //const int src_offset_x, 
                         //const int src_offset_y, 
                         const int dst_step_in_pixel,
                         const int dst_offset_in_pixel,
                         __constant float * mat_kernel __attribute__((max_constant_size(4*(2*RADIUSY+1)))))
{
	int x = get_global_id(0);
	int y = get_global_id(1);
	int l_x = get_local_id(0);
	int l_y = get_local_id(1);
	int start_addr = mad24(y,src_step_in_pixel,x);
	int i;
	float4 sum;
	float4 temp[READ_TIMES_COL];

	__local float4 LDS_DAT[LSIZE1*READ_TIMES_COL][LSIZE0+1];

	//read pixels from src
	for(i = 0;i<READ_TIMES_COL;i++)
	{
		temp[i] = src[start_addr+i*LSIZE1*src_step_in_pixel];
	}
	//save pixels to lds
	for(i = 0;i<READ_TIMES_COL;i++)
	{
		LDS_DAT[l_y+i*LSIZE1][l_x] = temp[i];
	}
	barrier(CLK_LOCAL_MEM_FENCE);
	//read pixels from lds and calculate the result
	sum = LDS_DAT[l_y+RADIUSY][l_x]*mat_kernel[RADIUSY];
	for(i=1;i<=RADIUSY;i++)
	{
		temp[0]=LDS_DAT[l_y+RADIUSY-i][l_x];
		temp[1]=LDS_DAT[l_y+RADIUSY+i][l_x];
		sum += temp[0] * mat_kernel[RADIUSY-i]+temp[1] * mat_kernel[RADIUSY+i];
	}
	//write the result to dst
	if((x<dst_cols) & y<(dst_rows))
	{
		start_addr = mad24(y,dst_step_in_pixel,x+dst_offset_in_pixel);
		dst[start_addr] = sum;
	}
}
