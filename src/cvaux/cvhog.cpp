/*M///////////////////////////////////////////////////////////////////////////////////////
//
//  IMPORTANT: READ BEFORE DOWNLOADING, COPYING, INSTALLING OR USING.
//
//  By downloading, copying, installing or using the software you agree to this license.
//  If you do not agree to this license, do not download, install,
//  copy or use the software.
//
//
//                           License Agreement
//                For Open Source Computer Vision Library
//
// Copyright (C) 2000-2008, Intel Corporation, all rights reserved.
// Copyright (C) 2009, Willow Garage Inc., all rights reserved.
// Third party copyrights are property of their respective owners.
//
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
//
//   * Redistribution's of source code must retain the above copyright notice,
//     this list of conditions and the following disclaimer.
//
//   * Redistribution's in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//
//   * The name of the copyright holders may not be used to endorse or promote products
//     derived from this software without specific prior written permission.
//
// This software is provided by the copyright holders and contributors "as is" and
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
//M*/

#include "_cvaux.h"

namespace cv
{

HOGParams::HOGParams() 
    : cellN(3), cellX(6), cellY(6), nbins(9), strideX(6), strideY(6)
{   
}; 

cv::HOGParams::HOGParams(int _cellN, int _cellX, int _cellY,
                         int _nbins, int _strideX, int _strideY)
    : cellN(_cellN), cellX(_cellX), cellY(_cellY), 
      nbins(_nbins), strideX(_strideX), strideY(_strideY)
{
}

void calc_gradients_data(const Mat& image, Mat& theta, Mat& rho)
{
    CV_Assert ( image.channels() == 1 );
            
    theta.create( image.size(), CV_32F ); 
    rho.create( image.size(), CV_32F );

    Mat dx, dy;
    Sobel( image, dx, CV_16S, 1, 0, 1); //, 1, 0, BORDER_REPLICATE );
    Sobel( image, dy, CV_16S, 0, 1, 1); //, 1, 0, BORDER_REPLICATE );                                   
    
    for (int y = 0; y < image.rows; ++y)
    {
        const short *line_dx = (const short*)dx.ptr(y);
        const short *line_dy = (const short*)dy.ptr(y);

        float *line_theta = (float*)theta.ptr(y);
        float *line_rho   = (float*)rho.ptr(y);

        for (int x = 0; x < image.cols; ++x)            
        {
            float val_dx = line_dx[x];
            float val_dy = line_dy[x];
                            
            line_theta[x] = fastAtan2( val_dy, val_dx );
            line_rho[x] = std::sqrt( val_dx * val_dx + val_dy * val_dy );
        }
    }                    
}

void normrow_L2(Mat& mat, float e)
{
    float *data = (float*)mat.ptr();
    float sum  = 0;
    for(int i = 0; i < mat.cols; ++i)
        sum += data[i] * data[i];
    
    sum = std::sqrt(sum + e * e);
    for(int i = 0; i < mat.cols; ++i)
        data[i] /=sum;                
}

void init_ind_hash(Mat& ind_hash, const HOGParams& params)
{
    int block_width  = params.cellN * params.cellX;
    int block_height = params.cellN * params.cellY;
    int cols = block_width < block_height ? block_height :  block_width;

    ind_hash.create(2, cols, CV_32S);

    int* hash_data_x = (int*)ind_hash.ptr(0);
    int* hash_data_y = (int*)ind_hash.ptr(1);

    for(int i = 0; i < cols; ++i)
    {
        hash_data_x[i] = i / params.cellX;
        hash_data_y[i] = i / params.cellY;
    }
}

void build_block_histogram(int x0, int y0, const Mat& theta, const Mat& rho, 
        const Mat& ind_hash, const HOGParams& params, Mat& hog)
{           
    int block_width  = params.cellN * params.cellX;
    int block_height = params.cellN * params.cellY;

    CV_Assert( ind_hash.rows == 2 );
    CV_Assert( ind_hash.cols == block_width < block_height ? block_height :  block_width);

    float* hog_data = (float*)hog.ptr();

    const int* hash_data_x = (const int*)ind_hash.ptr(0);
    const int* hash_data_y = (const int*)ind_hash.ptr(1);        
    float d_bin_ori_inv = params.nbins / 360.f;        

    for(int y = 0; y < block_height; ++y )
    {
        const float* line_theta = (const float*)theta.ptr(y + y0);
        const float* line_rho = (const float*)rho.ptr(y + y0);
        
        for(int x = 0; x < block_width; ++x )
        {
            float th = line_theta[x + x0];
            float rh = line_rho[x + x0];
            
            int binx = hash_data_x[x];
            int biny = hash_data_y[y];

            int bino = cvFloor(th * d_bin_ori_inv);

            int idx = binx * params.cellN * params.nbins + biny * params.nbins + bino;
            hog_data[idx] += rh;
        }
    }      
    normrow_L2(hog, 0.001f);
}

void extractHOG( const Mat& image, Mat& hogs, const HOGParams& params)
{               
    int block_width = params.cellN * params.cellX;
    int block_height = params.cellN * params.cellY;

    CV_Assert( image.channels() == 1 );
    CV_Assert( image.cols >= block_width && image.rows >= block_height );
        
    int block_num_x = (image.cols - block_width ) / params.strideX + 1;
    int block_num_y = (image.rows - block_height) / params.strideY + 1;

    hogs.create(block_num_x * block_num_y, params.histSize(), CV_32F);
    hogs = Scalar(0);
        
    Mat theta, rho;                 
    calc_gradients_data(image, theta, rho);

    Mat ind_hash;
    init_ind_hash(ind_hash, params);

    for(int bnum = 0; bnum < block_num_x * block_num_y; ++bnum)    
    {
        Mat hog_i = hogs.row(bnum);

        //bnum = by * block_num_x + bx
        int bx = (bnum % block_num_x) * params.strideX;
        int by = (bnum / block_num_x) * params.strideY;        

        build_block_histogram(bx, by, theta, rho, ind_hash, params, hog_i);                                
    }  
}

}
