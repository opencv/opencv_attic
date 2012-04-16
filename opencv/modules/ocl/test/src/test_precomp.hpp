/*M///////////////////////////////////////////////////////////////////////////////////////
//
//  IMPORTANT: READ BEFORE DOWNLOADING, COPYING, INSTALLING OR USING.
//
//  By downloading, copying, installing or using the software you agree to this license.
//  If you do not agree to this license, do not download, install,
//  copy or use the software.
//
//
//                        Intel License Agreement
//                For Open Source Computer Vision Library
//
// Copyright (C) 2000, Intel Corporation, all rights reserved.
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
//   * The name of Intel Corporation may not be used to endorse or promote products
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

#ifndef __OPENCV_TEST_PRECOMP_HPP__
#define __OPENCV_TEST_PRECOMP_HPP__

#include <cmath>
#include <cstdio>
#include <fstream>
#include <sstream>
#include <limits>
#include <string>
#include <algorithm>
#include <iterator>
#include "cvconfig.h"
#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/calib3d/calib3d.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/ts/ts.hpp"
#include "opencv2/ocl/ocl.hpp"
#include "test_ocl_base.hpp"

// for filtering out console error output 


#define PERF_TEST_OCL 0 // set to 1 to enable performance tests
#define TEST_ALL      0 // ignore the filtering flags (eg, OK, CLERR, etc.)

#if PERF_TEST_OCL
	void  run_perf_test();
#else  
	#if TEST_ALL
		#define OK    1 // no err
		#define CLERR 0 // cl compiling err
		#define W_OUT 0 // wrong output
		#define K_ERR 0 // kernel execution failed
		#define K_MIS 0 // kernel function missing

		#define TS_ARITHM  1  // test_arithm.cpp
		#define TS_IMGPROC 1  // test_imgproc.cpp
		#define TS_CALIB3D 1  // test_calib3d.cpp
		#define TS_FEATU2D 1  // test_features2d.cpp
		#define TS_MATOP   1  // test_matop.cpp
		#define TS_FILTERS 1  // test_filters.cpp
		#define TS_HAAR    1  // test_haar.cpp
		#define TS_IMGPROC2 1 // test_imgproc_2.cpp
	#else
		#define OK    1 // no err
		#define CLERR 0 // cl compiling err
		#define W_OUT 0 // wrong output
		#define K_ERR 0 // kernel execution failed
		#define K_MIS 0 // kernel function missing

		#define TS_ARITHM  1  // test_arithm.cpp
		#define TS_IMGPROC 0  // test_imgproc.cpp
		#define TS_CALIB3D 0  // test_calib3d.cpp
		#define TS_FEATU2D 0  // test_features2d.cpp
		#define TS_MATOP   0  // test_matop.cpp
		#define TS_FILTERS 0  // test_filters.cpp
		#define TS_HAAR    0  // test_haar.cpp
		#define TS_IMGPROC2 0 // test_imgproc_2.cpp
	#endif // TEST_ALL
#endif // PERF_TEST_OCL

#endif // __OPENCV_TEST_PRECOMP_HPP__
