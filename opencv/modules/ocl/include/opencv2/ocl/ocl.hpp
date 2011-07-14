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

#ifndef __OPENCV_OCL_HPP__
#define __OPENCV_OCL_HPP__

#include "opencv2/core/core.hpp"

//Root level namespace
namespace cv
{

//This namespace will contain all the source code for the OpenCL module
namespace ocl
{

class CV_EXPORTS OclMat{

public:
    //! default constructor
    OclMat();
    
    //! constructs OclMat of the specified size and type (supported types are CV_8UC1, CV_8UC3, CV_16UC1, CV_16UC3, CV_64FC3 etc)
    OclMat(int rows, int cols, int type);
    OclMat(Size size, int type);
    //! constucts OclMat and fills it with the specified value _s.
    OclMat(int rows, int cols, int type, const Scalar& s);
    OclMat(Size size, int type, const Scalar& s);
    //! copy constructor
    OclMat(const OclMat& m);

    //! builds OclMat from Mat. Perfom blocking upload to device.
    explicit OclMat (const Mat& m);

    //! destructor - calls release()
    ~OclMat();

    //Releases the OpenCL context, command queue and the data buffer
    void release();

    //! assignment operators
    OclMat& operator = (const OclMat& m);
    //! assignment operator. Perfom blocking upload to device.
    OclMat& operator = (const Mat& m);

    //! sets every OclMatelement to s
    OclMat& operator = (const Scalar& s);

    //! sets some of the OclMat elements to s, according to the mask
    OclMat& setTo(const Scalar& s);

    //! pefroms blocking upload data to OclMat.
    void upload(const cv::Mat& m);

    //! downloads data from device to host memory. Blocking calls.
    operator Mat();
    void download(cv::Mat& m);

    //! returns the size of element in bytes.
    size_t elemSize() const;
    //! returns the size of element channel in bytes.
    size_t elemSize1() const;
    //! returns element type, similar to CV_MAT_TYPE(cvMat->type)
    int type() const;
    //! returns element type, similar to CV_MAT_DEPTH(cvMat->type)
    int depth() const;
    //! returns element type, similar to CV_MAT_CN(cvMat->type)
    int channels() const;
    //! returns step/elemSize1()
    size_t step1() const;
    //! returns OclMatrix size:
    // width == number of columns, height == number of rows
    Size size() const;
    //! returns true if OclMat data is NULL
    bool empty() const;

    int flags;

     //! the number of rows and columns
    int rows, cols;
    //! a distance between successive rows in bytes; includes the gap if any
    size_t step;
    //! pointer to the data of type cl_mem
    void* data;

    int* refcount;

    //! allocates new OclMat data unless the OclMat already has specified size and type.
    // previous data is unreferenced if needed.
    void create(int rows, int cols, int type);
    void create(Size size, int type);

    void _upload(size_t size, void* src);
    void _download(size_t size, void* dst);

};

//! Creates the OpenCL context and command queue
//! Should be called at the beginning of every session for a single platform
CV_EXPORTS void init();

//! ///////////////////////////////////IMAGE PROCESSING////////////////////////////////////////////

//! Optical Flow Horn & Schunck
CV_EXPORTS void calcOpticalFlowHS(const OclMat& a, const OclMat& b, OclMat& velX, OclMat& velY, CvTermCriteria IterCriteria, float lambda);

//! Optical Flow Lucas & Kanade
CV_EXPORTS void calcOpticalFlowLK(const OclMat& prev, const OclMat& img, OclMat& velX, OclMat& velY, CvSize winSize);

//! applies fixed threshold to the image
CV_EXPORTS void threshold(const OclMat& src, OclMat& dst, float thresh, float maxval, int type);


//! ///////////////////////////ARITHMETIC//////////////////////////////////////////////////////////////////////////

//! adds one matrix to another (c = a + b)
CV_EXPORTS void add(const OclMat& a, const OclMat& b, OclMat& c );

//! adds scalar to a matrix (c = a + s)
CV_EXPORTS void add(const OclMat& a, const Scalar& sc, OclMat& c );

//! subtracts one matrix from another (c = a - b)
CV_EXPORTS void subtract(const OclMat& a, const OclMat& b, OclMat& c );

//! subtracts scalar from a matrix (c = a - s)
CV_EXPORTS void subtract(const OclMat& a, const Scalar& sc, OclMat& c );

//! computes element-wise product of the two arrays (c = a * b)
CV_EXPORTS void multiply(const OclMat& a, const OclMat& b, OclMat& c );

//! multiplies matrix to a scalar (c = a * s)
CV_EXPORTS void multiply(const OclMat& a, const Scalar& sc, OclMat& c );

//! computes element-wise quotient of the two arrays (c = a / b)
CV_EXPORTS void divide(const OclMat& a, const OclMat& b, OclMat& c );

//! computes element-wise quotient of matrix and scalar (c = a / s)
CV_EXPORTS void divide(const OclMat& a, const Scalar& sc, OclMat& c );

//! computes exponent of each matrix element (b = e**a)
//! supports only CV_32FC1 type
CV_EXPORTS void exp(const OclMat& a, OclMat& b );

//! computes natural logarithm of absolute value of each matrix element: b = log(abs(a))
CV_EXPORTS void log(const OclMat& a, OclMat& b );

//! computes element-wise absolute difference of two arrays (c = abs(a - b))
CV_EXPORTS void absdiff(const OclMat& a, const OclMat& b, OclMat& c );

//! computes element-wise absolute difference of array and scalar (c = abs(a - s))
CV_EXPORTS void absdiff(const OclMat& a, const Scalar& s, OclMat& c );

//! compares elements of two arrays (c = a <cmpop> b)
CV_EXPORTS void compare(const OclMat& a, const OclMat& b, OclMat& c, int cmpop );

//! performs per-elements bit-wise inversion
CV_EXPORTS void bitwise_not(const OclMat& src, OclMat& dst );

//! calculates per-element bit-wise disjunction of two arrays
CV_EXPORTS void bitwise_or(const OclMat& src1, const OclMat& src2, OclMat& dst );

//! calculates per-element bit-wise conjunction of two arrays
CV_EXPORTS void bitwise_and(const OclMat& src1, const OclMat& src2, OclMat& dst );

//! calculates per-element bit-wise "exclusive or" operation
CV_EXPORTS void bitwise_xor(const OclMat& src1, const OclMat& src2, OclMat& dst );

//! computes per-element minimum of two arrays (dst = min(src1, src2))
CV_EXPORTS void min(const OclMat& src1, const OclMat& src2, OclMat& dst );

//! computes per-element minimum of array and scalar (dst = min(src1, src2))
CV_EXPORTS void min(const OclMat& src1, const Scalar& src2, OclMat& dst );

//! computes per-element maximum of two arrays (dst = max(src1, src2))
CV_EXPORTS void max(const OclMat& src1, const OclMat& src2, OclMat& dst );

//! computes per-element maximum of array and scalar (dst = max(src1, src2))
CV_EXPORTS void max(const OclMat& src1, const Scalar& src2, OclMat& dst );

}
    
}

#endif
