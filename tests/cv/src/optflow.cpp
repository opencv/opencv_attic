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

#include "cvtest.h"
#include <string>
#include <iostream>
#include <fstream>
#include <iterator>
#include "cvaux.h"

using namespace cv;
using namespace std;

class CV_OptFlowTest : public CvTest
{
public:
    CV_OptFlowTest();
    ~CV_OptFlowTest();    
protected:    
    void run(int);
	
	bool ImagesTest(const string& dir, const string& tmp);
	bool VideoTest(const string& dir, const string& tmp, int fourcc);
	
	bool GuiTest(const string& dir, const string& tmp);
};

CV_OptFlowTest::CV_OptFlowTest(): CvTest( "algorithm-opticalflow", "?" )
{
    support_testing_modes = CvTS::CORRECTNESS_CHECK_MODE;
}
CV_OptFlowTest::~CV_OptFlowTest() {}


Mat copnvert2flow(const Mat& velx, const Mat& vely)
{
    Mat flow(velx.size(), CV_32FC2);
    for(int y = 0 ; y < flow.rows; ++y)
        for(int x = 0 ; x < flow.cols; ++x)                        
            flow.at<Point2f>(y, x) = Point2f(velx.at<float>(y, x), vely.at<float>(y, x));            
    return flow;
}

void calcOpticalFlowLK( const Mat& prev, const Mat& curr, Size winSize, Mat& flow )
{
    Mat velx(prev.size(), CV_32F), vely(prev.size(), CV_32F); 
    CvMat cvvelx = velx;    CvMat cvvely = vely;
    CvMat cvprev = prev;    CvMat cvcurr = curr;
    cvCalcOpticalFlowLK( &cvprev, &cvcurr, winSize, &cvvelx, &cvvely );
    flow = copnvert2flow(velx, vely);
}

void calcOpticalFlowBM( const Mat& prev, const Mat& curr, Size bSize, Size shiftSize, Size maxRange, int usePrevious, Mat& flow )
{
    Size sz((curr.cols - bSize.width)/shiftSize.width, (curr.rows - bSize.height)/shiftSize.height);
    Mat velx(sz, CV_32F), vely(sz, CV_32F);    

    CvMat cvvelx = velx;    CvMat cvvely = vely;
    CvMat cvprev = prev;    CvMat cvcurr = curr;
    cvCalcOpticalFlowBM( &cvprev, &cvcurr, bSize, shiftSize, maxRange, usePrevious, &cvvelx, &cvvely);                     
    flow = copnvert2flow(velx, vely);
}

void calcOpticalFlowHS( const Mat& prev, const Mat& curr, int usePrevious, double lambda, TermCriteria criteria, Mat& flow)
{        
    Mat velx(prev.size(), CV_32F), vely(prev.size(), CV_32F);
    CvMat cvvelx = velx;    CvMat cvvely = vely;
    CvMat cvprev = prev;    CvMat cvcurr = curr;
    cvCalcOpticalFlowHS( &cvprev, &cvcurr, usePrevious, &cvvelx, &cvvely, lambda, criteria );
    flow = copnvert2flow(velx, vely);
}

double showFlow(const string& name, const Mat& gray, const Mat& flow, const Rect& where, const Point& d)
{       
    const int mult = 10;
     
    Mat tmp, cflow;    
    resize(gray, tmp, gray.size() * mult);    
    cvtColor(tmp, cflow, CV_GRAY2BGR);        
    
    for(int y = 0; y < flow.rows; ++y)
        for(int x = 0; x < flow.cols; ++x)
        {
            Point2f f = flow.at<Point2f>(y, x);

            if (f.x == 0 && f.y == 0)
                continue;

            const float m2 = 0.5;

            //float n = (float)norm(f); f.x /= n; f.y /= n;
            
            if (1 || f.x * f.x + f.y * f.y > 0.01)
            {
                Point p1 = Point(x, y) * mult;
                Point p2 = Point(cvRound(x + f.x*m2), cvRound(y + f.y*m2)) * mult;

                line(cflow, p1, p2, CV_RGB(0, 255, 0));            
            }            
            circle(cflow, Point(x, y) * mult, 2, CV_RGB(255, 0, 0));
        }

    rectangle(cflow, (where.tl() + d) * mult, (where.br() + d) * mult, CV_RGB(0, 0, 255));    
    namedWindow(name, 1); imshow(name, cflow);
    
    double angle = atan2((float)d.y, (float)d.x);
    double error = 0;

    bool all = true;
    Mat inner = flow(where);
    for(int y = 0; y < inner.rows; ++y)
        for(int x = 0; x < inner.cols; ++x)
        {
            const Point2f f = flow.at<Point2f>(y, x);

            if (f.x == 0 && f.y == 0)
                continue;

            all = false;

            double a = atan2(f.y, f.x);
            error += fabs(angle - a);            
        }
    return all ? -1 : error / (inner.cols * inner.rows);
}


Mat generateImage(const Size& sz, bool doBlur = true)
{
    RNG rng;
    Mat mat(sz, CV_8U);
    mat = Scalar(0);
    for(int y = 0; y < mat.rows; ++y)
        for(int x = 0; x < mat.cols; ++x)
            mat.at<uchar>(y, x) = (uchar)rng;    
    if (doBlur)
        blur(mat, mat, Size(3, 3));
    return mat;
}

void CV_OptFlowTest::run( int /* start_from */)
{	
    Size matSize(50, 50);
    Size movSize(8, 8);

    Point d(3, 3);

    Mat smpl = generateImage(movSize, false);
    cv::add(smpl, Scalar(50), smpl);
    smpl = Scalar(0);
    
    Mat prev = generateImage(matSize);    
    Mat curr = prev.clone();
    
    Rect rect(Point(prev.cols/2, prev.rows/2) - Point(movSize.width/2, movSize.height/2), movSize);
       
    Mat m1 = prev(rect);                            smpl.copyTo(m1);
    m1 = curr(Rect(rect.tl() + d, rect.br() + d));  smpl.copyTo(m1);   
                   
    Mat flowLK, flowBM, flowHS, flowFB, flowBM_received;
    calcOpticalFlowLK( prev, curr, Size(15, 15), flowLK);        
    calcOpticalFlowBM( prev, curr, Size(15, 15), Size(1, 1), Size(15, 15), 0, flowBM_received);       
    calcOpticalFlowHS( prev, curr, 0, 5, TermCriteria(TermCriteria::MAX_ITER, 400, 0), flowHS);                 
    calcOpticalFlowFarneback( prev, curr, flowFB, 0.5, 3, std::max(d.x, d.y) + 20, 10, 5, 1.2, 0);
    
    flowBM.create(prev.size(), CV_32FC2);
    flowBM = Scalar(0);    
    Point origin((flowBM.cols - flowBM_received.cols)/2, (flowBM.rows - flowBM_received.rows)/2);
    Mat wcp = flowBM(Rect(origin, flowBM_received.size()));
    flowBM_received.copyTo(wcp);
      
    /*cout << "Error LK = " << showFlow("LK", prev, flowLK, rect, d) << endl;
    cout << "Error BM = " << showFlow("BM", prev, flowBM, rect, d) << endl;
    cout << "Error HS = " << showFlow("HS", prev, flowHS, rect, d) << endl;
    cout << "Error FB = " << showFlow("FB", prev, flowFB, rect, d) << endl;*/
                
    //waitKey();
        
    if (1)
    {        
        ts->set_failed_test_info(CvTS::FAIL_MISMATCH);
        return;
    }    
    ts->set_failed_test_info(CvTS::OK);
}

CV_OptFlowTest optFlow_test;

