__kernel void add_kernel (__global const uchar *a, __global const uchar* b, __global uchar* c){
	int tid = get_global_id(0);
	c[tid] = a[tid] + b[tid];
}
