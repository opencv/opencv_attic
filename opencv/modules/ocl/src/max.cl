//Kernel for adding 8bit images
__kernel void max8u(__global const uchar* imgA, __global const uchar* imgB, __global uchar* c, const int imageWidth){

	uint x = get_global_id(0);
	uint y = get_global_id(1);
	uint tid = y*imageWidth+x;

		c[tid] = max(imgA[tid], imgB[tid]);
}

//Kernel for adding 16bit images
__kernel void max16u(__global const ushort* imgA,__global const ushort* imgB, __global ushort* c, const int imageWidth){

	uint x = get_global_id(0);
	uint y = get_global_id(1);
	uint tid = y*imageWidth+x;

	c[tid] = max(imgA[tid], imgB[tid]);
	
}

//Kernel for adding 32bit fp images
__kernel void max32f(__global const float* imgA,__global const float* imgB, __global float* c, const int imageWidth){

	uint x = get_global_id(0);
	uint y = get_global_id(1);
	uint tid = y*imageWidth+x;

	c[tid] = max(imgA[tid], imgB[tid]);
	
}

//Kernel for adding constant to an 8bit image
__kernel void maxConst32f(__global const float* imgA, const float val, __global float* c, const int imageWidth){

	uint x = get_global_id(0);
	uint y = get_global_id(1);
	uint tid = y*imageWidth+x;

	c[tid] = max(imgA[tid], val);

}