#include "ocl.hpp"

#include <opencv/cv.h>
#include <opencv/highgui.h>

using namespace cv;
using namespace cv::ocl;

int main(){

	IplImage* f0 = cvLoadImage("d:/frame_0.png",0);
	IplImage* f1 = cvLoadImage("d:/frame_1.png",0);

	IplImage* img0 = cvCreateImage(cvGetSize(f0), 8, 1);
	IplImage* img1 = cvCreateImage(cvGetSize(f0), 8, 1);

	cvSmooth(f0, img0,CV_GAUSSIAN,5,5);
	cvSmooth(f1, img1,CV_GAUSSIAN,5,5);
	
	Mat im0 = img0;
	Mat im1 = img1;

	OclMat prev(im0);
	OclMat curr(im1);

	OclMat u(im0.rows, im0.cols, CV_32FC1);
	OclMat v(im0.rows, im0.cols, CV_32FC1);

	CvTermCriteria IterCriteria;
	IterCriteria.type = CV_TERMCRIT_ITER;
	IterCriteria.max_iter = 100;
	float lambda = 0.1f;

	cv::ocl::calcOpticalFlowHS(prev, curr, u, v, IterCriteria, lambda);

	Mat um(u.rows, u.cols, CV_32FC1);
	Mat vm(u.rows, u.cols, CV_32FC1);
	u.download(um);
	v.download(vm);

	IplImage velX = um;
	IplImage velY = vm;

	cvShowImage("velX", &velX);
	cvShowImage("velY", &velY);
	cvWaitKey(0);

}