__kernel void derivatives(__global const uchar* imgAsrc,__global const uchar* imgBsrc, __global uchar* imgA, __global uchar* imgB, __global float *u,__global float *v, const int imageWidth, const int imageHeight, const int criteria, const float lambda)
{
	uint x = get_global_id(0);
	uint y = get_global_id(1);
	uint tid = y*imageWidth+x;

/*	int count = 0;
	float temp0 = 0.00001f;
	float temp1 = 0.00001f;

	float filterCoeffs[25] = {1/273,4/273,7/273,4/273,1/273,
							  4/273,16/273,26/273,16/273,4/273,
							  7/273,26/273,41/273,26/273,7/273,
							  4/273,16/273,26/273,16/273,4/273,
							  1/273,4/273,7/273,4/273,1/273};


	float filterCoeffs[9] = {0.11f,0.11f,0.11f,0.11f,0.11f,0.11f,0.11f,0.11f,0.11f};
	uint imageSize = imageWidth*imageHeight;

	int addr = 0;
	for (int i=-1; i<=1; i++)
		  for (int j=-1; j<=1; j++){
			  addr = (y+j)*imageWidth + x+i;
			  if(addr >= 0 && addr < imageSize){
			  temp0 += filterCoeffs[count]*imgAsrc[(y+j)*imageWidth + x+i];
			  temp1 += filterCoeffs[count]*imgBsrc[(y+j)*imageWidth + x+i];
			  ++count;
			  }
		  } 

	imgA[tid] = (uchar)temp0;
	imgB[tid] = (uchar)temp1;
*/
	imgA[tid] = imgAsrc[tid];
	imgB[tid] = imgBsrc[tid];
	barrier(CLK_GLOBAL_MEM_FENCE);

	uchar s_data[8];

	float Ix, Iy, It;

	s_data[0] = imgA[tid];
	s_data[1] = imgB[tid];
	s_data[2] = imgA[tid + imageWidth];
	s_data[3] = imgB[tid + imageWidth];
	s_data[4] = imgA[tid + 1];
	s_data[5] = imgB[tid + 1];
	s_data[6] = imgA[tid + imageWidth + 1];
	s_data[7] = imgB[tid + imageWidth + 1];


	 Ix = (s_data[2] - s_data[0] + s_data[6] - s_data[4] +
		   s_data[3] - s_data[1] + s_data[7] - s_data[5] )/4;
	 Iy = (s_data[4] - s_data[0] + s_data[6] - s_data[2] +
		   s_data[5] - s_data[1] + s_data[7] - s_data[3] )/4;
	 It = (s_data[1] - s_data[0] + s_data[5] - s_data[4] +
		   s_data[3] - s_data[2] + s_data[7] - s_data[6] )/4;
	

	float u_bar = 0.f;
	float v_bar = 0.f;

	for (short i = 0; i < criteria; i++)
	{

	u_bar = u_bar - (Ix*(Ix*u_bar + Iy*v_bar + It) / (lambda*lambda + Ix*Ix + Iy*Iy ));
	v_bar = v_bar - (Iy*(Ix*u_bar + Iy*v_bar + It) / (lambda*lambda + Ix*Ix + Iy*Iy ));

	}

	u[tid] = u_bar;
	v[tid] = v_bar;
}