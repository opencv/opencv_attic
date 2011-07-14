__kernel void threshBin8u(__global const uchar* img, __global uchar* dst, const float thresh, const float maxval, const int imageWidth){

	uint x = get_global_id(0);
	uint y = get_global_id(1);
	uint tid = y*imageWidth+x;

	if(img[tid] > thresh)
		dst[tid] = (uchar)maxval;
	else
		dst[tid] = 0;

}

__kernel void threshBin32f(__global const float* img, __global float* dst, const float thresh, const float maxval, const int imageWidth){

	uint x = get_global_id(0);
	uint y = get_global_id(1);
	uint tid = y*imageWidth+x;

if(img[tid] > thresh)
		dst[tid] = maxval;
	else
		dst[tid] = 0;
}