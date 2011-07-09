//Kernel for adding 8bit images
__kernel void add8u(__global const uchar* imgA, __global const uchar* imgB, __global uchar* sum, const int imageWidth){

	uint x = get_global_id(0);
	uint y = get_global_id(1);
	uint tid = y*imageWidth+x;

	int s = imgA[tid] + imgB [tid];
	
	//Exceeds the limit, set to max(uchar)
	if(s > 255)
		sum[tid] = 255;
	else
		sum[tid] = s;
}

//Kernel for adding 16bit images
__kernel void add16u(__global const ushort* imgA,__global const ushort* imgB, __global ushort* sum, const int imageWidth){

	uint x = get_global_id(0);
	uint y = get_global_id(1);
	uint tid = y*imageWidth+x;

	int s = imgA[tid] + imgB [tid];
	
	//Exceeds the limit, set to max(uchar)
	if(s > 65535)
		sum[tid] = 65535;
	else
		sum[tid] = s;
}

//Kernel for adding 32bit fp images
__kernel void add32f(__global const float* imgA,__global const float* imgB, __global float* sum, const int imageWidth){

	uint x = get_global_id(0);
	uint y = get_global_id(1);
	uint tid = y*imageWidth+x;

	sum[tid] = imgA[tid] + imgB[tid];
	
}

//Kernel for adding constant to an 8bit image
__kernel void addConst32f(__global const float* imgA, const float val, __global float* sum, const int imageWidth){

	uint x = get_global_id(0);
	uint y = get_global_id(1);
	uint tid = y*imageWidth+x;

	sum[tid] = imgA[tid] + val;

}