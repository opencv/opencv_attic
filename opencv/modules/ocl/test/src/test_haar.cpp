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
// Copyright (C) 2010-2012, Institute Of Software Chinese Academy Of Science, all rights reserved.
// Copyright (C) 2010-2012, Advanced Micro Devices, Inc., all rights reserved.
// Third party copyrights are property of their respective owners.
//
// @Authors
//    Jia Haipeng, jiahaipeng95@gmail.com
//
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
//
//   * Redistribution's of source code must retain the above copyright notice,
//     this list of conditions and the following disclaimer.
//
//   * Redistribution's in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other oclMaterials provided with the distribution.
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

#include "opencv2/objdetect/objdetect.hpp"
#include "test_precomp.hpp"

#if    TS_HAAR
#ifdef HAVE_OPENCL

using namespace cvtest;
using namespace testing;
using namespace std;
using namespace cv;

struct getRect { Rect operator ()(const CvAvgComp& e) const { return e.rect; } };

PARAM_TEST_CASE(HaarTestBase, int, int)
{
    cv::ocl::OclCascadeClassifier cascade, nestedCascade;
    Mat img;

    double scale;
    int index;

    virtual void SetUp()
    {
        scale = 1.1;

        string cascadeName="../resource/haarcascade_frontalface_alt.xml";

        if( !cascade.load( cascadeName ) )
        {
            cout << "ERROR: Could not load classifier cascade" << endl;
            cout << "Usage: facedetect [--cascade=<cascade_path>]\n"
                "   [--nested-cascade[=nested_cascade_path]]\n"
                "   [--scale[=<image scale>\n"
                "   [filename|camera_index]\n" << endl ;

            return;
        }

        for(int i = 1;i < 2;i++)
        {
            index = i;
#if WIN32
			const char * buff = "../resource/group4.jpg";
            img = imread( buff, 1 );
#else
            img = imread("test.jpg", 1 );
#endif
            if(img.empty()){ std::cout << "Couldn't read test.jpg" << std::endl;continue;}
        }
    }
};

////////////////////////////////faceDetect/////////////////////////////////////////////////

struct Haar : HaarTestBase {};

TEST_P(Haar, FaceDetect)
{
    int i = 0;
    double t = 0;
    vector<Rect> faces;
    const static Scalar colors[] =  { CV_RGB(0,0,255),
        CV_RGB(0,128,255),
        CV_RGB(0,255,255),
        CV_RGB(0,255,0),
        CV_RGB(255,128,0),
        CV_RGB(255,255,0),
        CV_RGB(255,0,0),
        CV_RGB(255,0,255)} ;

    Mat gray, smallImg(cvRound (img.rows/scale), cvRound(img.cols/scale), CV_8UC1 );
    MemStorage storage(cvCreateMemStorage(0));
	if(img.channels()==3 ||img.channels()==4)
	{
    cvtColor( img, gray, CV_BGR2GRAY );
	}
	else
	{
		copy(img,gray);
	}
    resize( gray, smallImg, smallImg.size(), 0, 0, INTER_LINEAR );
    equalizeHist( smallImg, smallImg );
    CvMat _image = smallImg;

    Mat tempimg(&_image, false);

    cv::ocl::oclMat image(tempimg);
    CvSeq* _objects;

    for(int i= 0; i<1; i++)
    {
        t = (double)cvGetTickCount();
        _objects = cascade.oclHaarDetectObjects( image, storage, 1.1,
                2, 0
                |CV_HAAR_SCALE_IMAGE
                , Size(30,30), Size(0, 0) );

        t = (double)cvGetTickCount() - t ;
        printf( "detection time = %g ms\n", t/((double)cvGetTickFrequency()*1000.) );
    }

    vector<CvAvgComp> vecAvgComp;
    Seq<CvAvgComp>(_objects).copyTo(vecAvgComp);
    faces.resize(vecAvgComp.size());
    std::transform(vecAvgComp.begin(), vecAvgComp.end(), faces.begin(), getRect());
    for( vector<Rect>::const_iterator r = faces.begin(); r != faces.end(); r++, i++ )
    {
        Mat smallImgROI;
        vector<Rect> nestedObjects;
        Point center;
        Scalar color = colors[i%8];
        int radius;
        center.x = cvRound((r->x + r->width*0.5)*scale);
        center.y = cvRound((r->y + r->height*0.5)*scale);
        radius = cvRound((r->width + r->height)*0.25*scale);
        circle( img, center, radius, color, 3, 8, 0 );
    }

#if WIN32
    char buf[256];
    sprintf(buf,"D:/result/%d.jpg",index);
    imwrite(buf,img);
#else
    imwrite( "testdet.jpg", img );
#endif
}

INSTANTIATE_TEST_CASE_P(HaarTestBase, Haar, Combine(Values(1),
            Values(1)));

#endif // HAVE_OPENCL
#endif // TS_HAAR