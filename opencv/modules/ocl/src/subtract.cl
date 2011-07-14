//Kernel for adding 8bit images
__kernel void sub8u(__global const uchar* imgA, __global const uchar* imgB, __global uchar* diff, const int imageWidth){

	uint x = get_global_id(0);
	uint y = get_global_id(1);
	uint tid = y*imageWidth+x;

	int s = imgA[tid] - imgB [tid];
	
	//Exceeds the limit, set to min(uchar)
	if(s < 0)
		diff[tid] = 0;
	else
		diff[tid] = s;
}

//Kernel for adding 16bit images
__kernel void sub16u(__global const ushort* imgA,__global const ushort* imgB, __global ushort* diff, const int imageWidth){

	uint x = get_global_id(0);
	uint y = get_global_id(1);
	uint tid = y*imageWidth+x;

	int s = imgA[tid] - imgB [tid];
	
	//Exceeds the limit, set to min(ushort)
	if(s < 0)
		diff[tid] = 0;
	else
		diff[tid] = s;
}

//Kernel for adding 32bit fp images
__kernel void sub32f(__global const float* imgA, __global const float* imgB, __global float* diff, const int imageWidth){

	uint x = get_global_id(0);
	uint y = get_global_id(1);
	uint tid = y*imageWidth+x;

	diff[tid] = imgA[tid] - imgB[tid];
	
}

//Kernel for adding constant to an 8bit image
__kernel void subConst32f(__global const float* imgA, const float val, __global float* diff, const int imageWidth){

	uint x = get_global_id(0);
	uint y = get_global_id(1);
	uint tid = y*imageWidth+x;

	diff[tid] = imgA[tid] - val;

}