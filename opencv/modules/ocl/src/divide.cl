//Kernel for adding 8bit images
__kernel void div8u(__global const uchar* imgA, __global const uchar* imgB, __global uchar* c, const int imageWidth){

	uint x = get_global_id(0);
	uint y = get_global_id(1);
	uint tid = y*imageWidth+x;

	if(imgB[tid] == 0)
		c[tid] = 255;

	if(imgB[tid] > imgA[tid])
		c[tid] = 1;

	else
		c[tid] = ceil((float)((imgA[tid] / imgB [tid])));
	
}

//Kernel for adding 16bit images
__kernel void div16u(__global const ushort* imgA,__global const ushort* imgB, __global ushort* c, const int imageWidth){

	uint x = get_global_id(0);
	uint y = get_global_id(1);
	uint tid = y*imageWidth+x;

	if(imgB[tid] == 0)
		c[tid] = 65535;
	else
		c[tid] = (ushort)(imgA[tid] / imgB [tid]);
}

//Kernel for adding 32bit fp images
__kernel void div32f(__global const float* imgA,__global const float* imgB, __global float* c, const int imageWidth){

	uint x = get_global_id(0);
	uint y = get_global_id(1);
	uint tid = y*imageWidth+x;

	if(imgB[tid] == 0)
		c[tid] = 1e30;
	else
		c[tid] = imgA[tid] / imgB [tid];
	
}

//Kernel for adding constant to an 8bit image
__kernel void divConst32f(__global const float* imgA, const float val, __global float* c, const int imageWidth){

	uint x = get_global_id(0);
	uint y = get_global_id(1);
	uint tid = y*imageWidth+x;

	c[tid] = imgA[tid] / val;

}