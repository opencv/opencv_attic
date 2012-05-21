#if defined (__ATI__)
#pragma OPENCL EXTENSION cl_amd_fp64:enable
#elif defined (__NVIDIA__)
#pragma OPENCL EXTENSION cl_khr_fp64:enable
#endif
#define CV_PI 3.1415926535898
/**************************************phase inradians**************************************/
__kernel void arithm_phase_inradians_D5(__global float *src1, int src1_step, int src1_offset,
                                        __global float *src2, int src2_step, int src2_offset,
                                        __global float *dst,  int dst_step,  int dst_offset,
                                        int rows, int cols, int dst_step1)
{

	int x = get_global_id(0);
	int y = get_global_id(1);
	
	if (x < cols && y < rows)
	{
		int src1_index = mad24(y, src1_step, (x << 2) + src1_offset);
		int src2_index = mad24(y, src2_step, (x << 2) + src2_offset);
		int dst_index  = mad24(y, dst_step, (x << 2) + dst_offset);
		
		float data1 = *((__global float *)((__global char *)src1 + src1_index));
		float data2 = *((__global float *)((__global char *)src2 + src2_index));
		float tmp = atan2(data2, data1);
		
		*((__global float *)((__global char *)dst + dst_index)) = tmp;
	}
	
}


#if defined (DOUBLE_SUPPORT)
__kernel void arithm_phase_inradians_D6(__global double *src1, int src1_step, int src1_offset,
                                        __global double *src2, int src2_step, int src2_offset,
                                        __global double *dst,  int dst_step,  int dst_offset,
                                        int rows, int cols, int dst_step1)
{

	int x = get_global_id(0);
	int y = get_global_id(1);
	
	if (x < cols && y < rows)
	{
		int src1_index = mad24(y, src1_step, (x << 3) + src1_offset);
		int src2_index = mad24(y, src2_step, (x << 3) + src2_offset);
		int dst_index  = mad24(y, dst_step, (x << 3) + dst_offset);
		
		double data1 = *((__global double *)((__global char *)src1 + src1_index));
		double data2 = *((__global double *)((__global char *)src2 + src2_index));
		
		*((__global double *)((__global char *)dst + dst_index)) = atan2(data2, data1);
	}
	
}
#endif

/**************************************phase indegrees**************************************/
__kernel void arithm_phase_indegrees_D5(__global float *src1, int src1_step, int src1_offset,
                                        __global float *src2, int src2_step, int src2_offset,
                                        __global float *dst,  int dst_step,  int dst_offset,
                                        int rows, int cols, int dst_step1)
{

	int x = get_global_id(0);
	int y = get_global_id(1);
	
	if (x < cols && y < rows)
	{
		int src1_index = mad24(y, src1_step, (x << 2) + src1_offset);
		int src2_index = mad24(y, src2_step, (x << 2) + src2_offset);
		int dst_index  = mad24(y, dst_step, (x << 2) + dst_offset);
		
		float data1 = *((__global float *)((__global char *)src1 + src1_index));
		float data2 = *((__global float *)((__global char *)src2 + src2_index));
		float tmp = atan2(data2, data1);
		float tmp_data = 180 * tmp / CV_PI;
		
		*((__global float *)((__global char *)dst + dst_index)) = tmp_data;
	}
	
}


#if defined (DOUBLE_SUPPORT)
__kernel void arithm_phase_indegrees_D6(__global double *src1, int src1_step, int src1_offset,
                                        __global double *src2, int src2_step, int src2_offset,
                                        __global double *dst,  int dst_step,  int dst_offset,
                                        int rows, int cols, int dst_step1)
{

	int x = get_global_id(0);
	int y = get_global_id(1);
	
	if (x < cols && y < rows)
	{
		int src1_index = mad24(y, src1_step, (x << 3) + src1_offset);
		int src2_index = mad24(y, src2_step, (x << 3) + src2_offset);
		int dst_index  = mad24(y, dst_step, (x << 3) + dst_offset);
		
		double data1 = *((__global double *)((__global char *)src1 + src1_index));
		double data2 = *((__global double *)((__global char *)src2 + src2_index));
		double tmp = atan2(data2, data1);
		double tmp_data = 180 * tmp / CV_PI;
		
		*((__global double *)((__global char *)dst + dst_index)) = tmp_data;
	}
	
}
#endif
