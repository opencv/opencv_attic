__kernel void derivatives_compute(__global uchar* imgA,__global uchar* imgB, __global float *u,__global float *v, __global float *fx, __global float *fy, __global float *ft, const int imageWidth, const int xRadius, const int yRadius)
{
	uint x = get_global_id(0);
	uint y = get_global_id(1);
	uint tid = y*imageWidth + x;

	//float Ix, Iy, It;

	fx[tid] = -imgA[tid - 1 - imageWidth] + 
		 imgA[tid + 1 - imageWidth] - 
		 2*imgA[tid - 1] +
		 2*imgA[tid + 1] - 
		 imgA[tid - 1 + imageWidth] +
		 imgA[tid + 1 + imageWidth];

	fy[tid] =  imgA[tid - 1 - imageWidth] +
		  2*imgA[tid - imageWidth] +
		  imgA[tid + 1 - imageWidth] -
		  imgA[tid - 1 + imageWidth] - 
		  2*imgA[tid + imageWidth] - 
		  imgA[tid + 1 + imageWidth];

	ft[tid] = imgB[tid] - imgA[tid];

	barrier(CLK_GLOBAL_MEM_FENCE);

	float s_data[3];

	float A = 0.f, B = 0.f, C = 0.f, D = 0.f, E = 0.f;

	for (short i = -xRadius; i <= yRadius; i++)
	{
		for (short j = -yRadius; j <= yRadius; j++)
		{
			
			s_data[0] = fx[tid + i + j * imageWidth];
			s_data[1] = fy[tid + i + j * imageWidth];
			s_data[2] = ft[tid + i + j * imageWidth];

			A += s_data[0] * s_data[0]; 
			B += s_data[1] * s_data[1]; 
			C += s_data[0] * s_data[1];
			D += s_data[0] * s_data[2];
			E += s_data[1] * s_data[2];
		}
	}

	/* 
	Solving the following system of equation 

	    [ A  C] * [ u ] = [ D ]
        [ C  B]   [ v ]   [ E ]

		u  =  1/ (A*B - C*C) * (D*B - E*C);
	    v  =  1/ (A*B - C*C) * (E*A - D*C);     */
    
	// Output values //

	u[tid] =  (D*B - E*C)/(A*B - C*C);
    v[tid] =  (E*A - D*C)/(A*B - C*C);
}