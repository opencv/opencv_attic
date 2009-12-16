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

#include "cxcoretest.h"
#include <string>
#include <iostream>
#include <fstream>
#include <iterator>
#include <limits>
#include <numeric>
#include "cvaux.h"

using namespace cv;
using namespace std;



class CV_OperationsTest : public CvTest
{
public:
    CV_OperationsTest();
    ~CV_OperationsTest();    
protected:
    void run(int);    

    struct test_excep {};

    bool TestMat();
    bool TestTemplateMat();
    bool TestMatND();
    bool TestSparseMat();
    bool operations1();

    void checkDiff(const Mat& m1, const Mat& m2) { if (norm(m1, m2, NORM_INF) != 0) throw test_excep(); }

};

CV_OperationsTest::CV_OperationsTest(): CvTest( "operations", "?" )
{
    support_testing_modes = CvTS::CORRECTNESS_CHECK_MODE;
}
CV_OperationsTest::~CV_OperationsTest() {}


bool CV_OperationsTest::TestMat()
{
    try
    {
        Mat one_3x1(3, 1, CV_32F, Scalar(1.0));
        Mat shi_3x1(3, 1, CV_32F, Scalar(1.2));
        Mat shi_2x1(2, 1, CV_32F, Scalar(-1));
        Scalar shift = Scalar::all(15);

        float data[] = { sqrt(2.f)/2, -sqrt(2.f)/2, 1.f, sqrt(2.f)/2, sqrt(2.f)/2, 10.f };
        Mat rot_2x3(2, 3, CV_32F, data);
                   
        Mat res = 2 * rot_2x3 * (one_3x1 + shi_3x1 + shi_3x1 + shi_3x1) - shi_2x1 + shift;

        Mat tmp, res2;
        add(one_3x1, shi_3x1, tmp);
        add(tmp, shi_3x1, tmp);
        add(tmp, shi_3x1, tmp);
        gemm(rot_2x3, tmp, 2, shi_2x1, -1, res2, 0);
        add(res2, Mat(2, 1, CV_32F, shift), res2);
        
        checkDiff(res, res2);
            
        Mat mat4x4(4, 4, CV_32F);
        randu(mat4x4, Scalar(0), Scalar(10));

        Mat roi1 = mat4x4(Rect(Point(1, 1), Size(2, 2)));
        Mat roi2 = mat4x4(Range(1, 3), Range(1, 3));
        
        checkDiff(roi1, roi2);
        checkDiff(mat4x4, mat4x4(Rect(Point(0,0), mat4x4.size())));        

        Mat intMat10(3, 3, CV_32S, Scalar(10));
        Mat intMat11(3, 3, CV_32S, Scalar(11));
        Mat resMat(3, 3, CV_8U, Scalar(255));
                
        checkDiff(resMat, intMat10 == intMat10);
        checkDiff(resMat, intMat10 <  intMat11);
        checkDiff(resMat, intMat11 >  intMat10);
        checkDiff(resMat, intMat10 <= intMat11);
        checkDiff(resMat, intMat11 >= intMat10);

        checkDiff(resMat, intMat10 == 10.0);
        checkDiff(resMat, intMat10 <  11.0);
        checkDiff(resMat, intMat11 >  10.0);
        checkDiff(resMat, intMat10 <= 11.0);
        checkDiff(resMat, intMat11 >= 10.0);

        Mat maskMat4(3, 3, CV_8U, Scalar(4));
        Mat maskMat1(3, 3, CV_8U, Scalar(1));
        Mat maskMat5(3, 3, CV_8U, Scalar(5));
        Mat maskMat0(3, 3, CV_8U, Scalar(0));


        checkDiff(maskMat0, maskMat4 & maskMat1);
        checkDiff(maskMat0, Scalar(1) & maskMat4);
        checkDiff(maskMat0, maskMat4 & Scalar(1));

        Mat m;
        m = maskMat4.clone(); m&=maskMat1; checkDiff(maskMat0, m);
        m = maskMat4.clone(); m&=Scalar(1); checkDiff(maskMat0, m);

        checkDiff(maskMat0, (maskMat4 | maskMat4) & (maskMat1 | maskMat1));
        checkDiff(maskMat0, (maskMat4 | maskMat4) & maskMat1);
        checkDiff(maskMat0, maskMat4 & (maskMat1 | maskMat1));
        
        checkDiff(maskMat0, maskMat5 ^ (maskMat4 | maskMat1));
        checkDiff(maskMat0, Scalar(5) ^ (maskMat4 | Scalar(1)));

        checkDiff(maskMat5, maskMat5 | (maskMat4 ^ maskMat1));
        checkDiff(maskMat5, maskMat5 | (maskMat4 ^ Scalar(1)));

        checkDiff(~maskMat1, maskMat1 ^ 0xFF);
        checkDiff(~(maskMat1 | maskMat1), maskMat1 ^ 0xFF); 

        checkDiff(maskMat1, maskMat4/4.0);      
        

    }
    catch (const test_excep&)
    {
        ts->set_failed_test_info(CvTS::FAIL_MISMATCH);
        return false;
    }
    return true;
}


bool CV_OperationsTest::TestTemplateMat()
{  
    try
    {
        Mat_<float> one_3x1(3, 1, 1.0);
        Mat_<float> shi_3x1(3, 1, 1.2);
        Mat_<float> shi_2x1(2, 1, -2);
        Scalar shift = Scalar::all(15);

        float data[] = { sqrt(2.f)/2, -sqrt(2.f)/2, 1.f, sqrt(2.f)/2, sqrt(2.f)/2, 10.f };
        Mat_<float> rot_2x3(2, 3, data);
               
        Mat_<float> res = 2 * rot_2x3 * (one_3x1 + shi_3x1 + shi_3x1 + shi_3x1) - shi_2x1 + shift;

        Mat_<float> tmp, res2;
        add(one_3x1, shi_3x1, tmp);
        add(tmp, shi_3x1, tmp);
        add(tmp, shi_3x1, tmp);
        gemm(rot_2x3, tmp, 2, shi_2x1, -1, res2, 0);
        add(res2, Mat(2, 1, CV_32F, shift), res2);
        
        checkDiff(res, res2);
            
        Mat_<float> mat4x4(4, 4);
        randu(mat4x4, Scalar(0), Scalar(10));

        Mat_<float> roi1 = mat4x4(Rect(Point(1, 1), Size(2, 2)));
        Mat_<float> roi2 = mat4x4(Range(1, 3), Range(1, 3));
        
        checkDiff(roi1, roi2);
        checkDiff(mat4x4, mat4x4(Rect(Point(0,0), mat4x4.size())));        

        Mat_<int> intMat10(3, 3, 10);
        Mat_<int> intMat11(3, 3, 11);
        Mat_<uchar> resMat(3, 3, 255);
                
        checkDiff(resMat, intMat10 == intMat10);
        checkDiff(resMat, intMat10 <  intMat11);
        checkDiff(resMat, intMat11 >  intMat10);
        checkDiff(resMat, intMat10 <= intMat11);
        checkDiff(resMat, intMat11 >= intMat10);

        checkDiff(resMat, intMat10 == 10.0);
        checkDiff(resMat, intMat10 <  11.0);
        checkDiff(resMat, intMat11 >  10.0);
        checkDiff(resMat, intMat10 <= 11.0);
        checkDiff(resMat, intMat11 >= 10.0);

        Mat_<uchar> maskMat4(3, 3, 4);
        Mat_<uchar> maskMat1(3, 3, 1);
        Mat_<uchar> maskMat5(3, 3, 5);
        Mat_<uchar> maskMat0(3, 3, (uchar)0);

        checkDiff(maskMat0, maskMat4 & maskMat1);        
        checkDiff(maskMat0, Scalar(1) & maskMat4);
        checkDiff(maskMat0, maskMat4 & Scalar(1));
                        
        Mat m;
        m = maskMat4.clone(); m&=maskMat1; checkDiff(maskMat0, m);
        m = maskMat4.clone(); m&=Scalar(1); checkDiff(maskMat0, m);
        
        checkDiff(maskMat0, (maskMat4 | maskMat4) & (maskMat1 | maskMat1));
        checkDiff(maskMat0, (maskMat4 | maskMat4) & maskMat1);
        checkDiff(maskMat0, maskMat4 & (maskMat1 | maskMat1));

        checkDiff(maskMat0, maskMat5 ^ (maskMat4 | maskMat1));
        checkDiff(maskMat0, Scalar(5) ^ (maskMat4 | Scalar(1)));

        checkDiff(maskMat5, maskMat5 | (maskMat4 ^ maskMat1));
        checkDiff(maskMat5, maskMat5 | (maskMat4 ^ Scalar(1)));

        checkDiff(~maskMat1, maskMat1 ^ 0xFF);
        checkDiff(~(maskMat1 | maskMat1), maskMat1 ^ 0xFF); 

        checkDiff(maskMat1 + maskMat4, maskMat5);
        checkDiff(maskMat1 + Scalar(4), maskMat5);
        checkDiff(Scalar(4) + maskMat1, maskMat5);
        checkDiff(Scalar(4) + (maskMat1 & maskMat1), maskMat5);

        checkDiff(maskMat1 + 4.0, maskMat5);
        checkDiff((maskMat1 & 0xFF) + 4.0, maskMat5);
        checkDiff(4.0 + maskMat1, maskMat5);

        m = maskMat4.clone(); m+=Scalar(1); checkDiff(m, maskMat5);
        m = maskMat4.clone(); m+=maskMat1; checkDiff(m, maskMat5);
        m = maskMat4.clone(); m+=(maskMat1 | maskMat1); checkDiff(m, maskMat5);

        checkDiff(maskMat5 - maskMat1, maskMat4);
        checkDiff(maskMat5 - Scalar(1), maskMat4);
        checkDiff((maskMat5 | maskMat5) - Scalar(1), maskMat4);
        checkDiff(maskMat5 - 1, maskMat4);
        checkDiff((maskMat5 | maskMat5) - 1, maskMat4);
        checkDiff((maskMat5 | maskMat5) - (maskMat1 | maskMat1), maskMat4);
        
        m = maskMat5.clone(); m-=Scalar(1); checkDiff(m, maskMat4);
        m = maskMat5.clone(); m-=maskMat1; checkDiff(m, maskMat4);
        m = maskMat5.clone(); m-=(maskMat1 | maskMat1); checkDiff(m, maskMat4);

        checkDiff(maskMat1, maskMat4/4.0);       

        Mat_<float> negf(3, 3, -3.0);        
        Mat_<int> negi(3, 3, -3);        

        checkDiff(abs(negf), -negf);         
        //checkDiff(abs(negi), -(negi & negi));
    }
    catch (const test_excep&)
    {
        ts->set_failed_test_info(CvTS::FAIL_MISMATCH);
        return false;
    }
    return true;
}

bool CV_OperationsTest::TestMatND()
{  
    int sizes[] = { 3, 3, 3};
    cv::MatND nd(3, sizes, CV_32F);

   /* MatND res = nd * nd + nd;    
    MatND res2;
    cv::gemm(nd, nd, 1, nd, 1, res2);
    
    if (!checkMatSetError(res1, res2))
        return false;*/

    return true;
}

bool CV_OperationsTest::TestSparseMat()
{  
    try
    {
        int sizes[] = { 10, 10, 10};
        int dims = sizeof(sizes)/sizeof(sizes[0]);
        SparseMat mat(dims, sizes, CV_32FC2);

        if (mat.dims() != dims) throw test_excep();
        if (mat.channels() != 2) throw test_excep();
        if (mat.depth() != CV_32F) throw test_excep();

        SparseMat mat2 = mat.clone();

        

    }
    catch (const test_excep&)
    {
        ts->set_failed_test_info(CvTS::FAIL_MISMATCH);
        return false;
    }
    return true;
}

bool CV_OperationsTest::operations1()
{    
    try 
    {
        Point3d p1(1, 1, 1), p2(2, 2, 2), p4(4, 4, 4);    
        p1*=2;    
        if (!(p1     == p2)) throw test_excep();
        if (!(p2 * 2 == p4)) throw test_excep();
        if (!(p2 * 2.f == p4)) throw test_excep();
        if (!(p2 * 2.f == p4)) throw test_excep();

        Point2d pi1(1, 1), pi2(2, 2), pi4(4, 4);    
        pi1*=2;
        if (!(pi1     == pi2)) throw test_excep();
        if (!(pi2 * 2 == pi4)) throw test_excep();
        if (!(pi2 * 2.f == pi4)) throw test_excep();
        if (!(pi2 * 2.f == pi4)) throw test_excep();
        
        Vec2d v12(1, 1), v22(2, 2);
        v12*=2.0;
        if (!(v12 == v22)) throw test_excep();
        
        Vec3d v13(1, 1, 1), v23(2, 2, 2);
        v13*=2.0;
        if (!(v13 == v23)) throw test_excep();

        Vec4d v14(1, 1, 1, 1), v24(2, 2, 2, 2);
        v14*=2.0;
        if (!(v14 == v24)) throw test_excep();
        
        Size sz(10, 20);
        if (sz.area() != 200) throw test_excep();
        if (sz.width != 10 || sz.height != 20) throw test_excep();
        if (((CvSize)sz).width != 10 || ((CvSize)sz).height != 20) throw test_excep();
        
        Vec<double, 5> v5d(1, 1, 1, 1, 1);
        Vec<double, 6> v6d(1, 1, 1, 1, 1, 1);
        Vec<double, 7> v7d(1, 1, 1, 1, 1, 1, 1);
        Vec<double, 8> v8d(1, 1, 1, 1, 1, 1, 1, 1);
        Vec<double, 9> v9d(1, 1, 1, 1, 1, 1, 1, 1, 1);
        Vec<double,10> v10d(1, 1, 1, 1, 1, 1, 1, 1, 1, 1);


    }
    catch(const test_excep&)
    {
        ts->set_failed_test_info(CvTS::FAIL_MISMATCH);
        return false;
    }
    return true;
}

void CV_OperationsTest::run( int /* start_from */)
{
    if (!TestMat())
        return;

 /*   if (!TestMatND())
        return;*/

    if (!TestSparseMat())
        return;

    if (!operations1())
        return;

    ts->set_failed_test_info(CvTS::OK);
}

CV_OperationsTest cv_Operations_test;


