#include "test_precomp.hpp"

using namespace cv;
using namespace cv::ocl;

void test_add(){

	IplImage* img0 = cvLoadImage("d:/frame_0.png",0);
	IplImage* img1 = cvLoadImage("d:/frame_1.png",0);

	Mat im0 = img0;
	Mat im1 = img1;

	OclMat a(im0);
	OclMat b(im1);
	OclMat c(a.rows, a.cols, a.type());

	cv::ocl::add(a, b, c);

	c.download(im0);

	IplImage sum = im0;

	cvShowImage("sum", &sum);
//	cvShowImage("velY", &velY);
	cvWaitKey(0);

}