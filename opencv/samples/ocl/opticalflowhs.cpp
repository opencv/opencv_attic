#include <opencv2/ocl.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv/cv.h>


using namespace cv;
using namespace cv::ocl;


//Draws the sample motion vectors on the 1st frame
void drawOptFlowMap(const Mat& vel, Mat& cflowmap, int step, double, const Scalar& color)
{
    for(int y = 0; y < cflowmap.rows; y += step)
        for(int x = 0; x < cflowmap.cols; x += step)
        {
			const Point2f& fxy = vel.at<Point2f>(y, x);
            line(cflowmap, Point(x,y), Point(cvRound(x+fxy.x), cvRound(y+fxy.y)),color);
            circle(cflowmap, Point(x,y), 2, color, -1);
        }
}

int main(){

	VideoCapture capture("d:/dancingman.avi");
	if(!capture.isOpened()){
		cout<<"Failed to capture video, check the video path again, exiting...\n";
		exit(0);
	}

	Mat frame0color, frame1color;

	namedWindow("Dancing man", 1);

	for(;;){
		//Capture 1st frame
		capture >> frame0color;
		if(frame0color.empty()){ cout<<"exiting..."; break; }

		//Create intermediate frames for holding grayscale images of the source frames
		Mat frame0gray(frame0color.rows, frame0color.cols, CV_8UC1);
		Mat frame1gray(frame0color.rows, frame0color.cols, CV_8UC1);

		//Capture 2nd frame
		capture >> frame1color;
		if(frame1color.empty()){ cout<<"exiting..."; break; }

		//Convert source frames into grayscale
		cvtColor(frame0color, frame0gray, CV_RGB2GRAY);
		cvtColor(frame1color, frame1gray, CV_RGB2GRAY);

		
		//Create horizontal and vertical velocity vectors on CPU and also on GPU
		Mat velX(frame0gray.rows, frame0gray.cols, CV_32FC1);
		Mat velY(frame0gray.rows, frame0gray.cols, CV_32FC1);
		OclMat velXgpu(velX.rows, velX.cols, CV_32FC1);
		OclMat velYgpu(velX.rows, velX.cols, CV_32FC1);

		CvTermCriteria IterCriteria;
		IterCriteria.type = CV_TERMCRIT_ITER;
		IterCriteria.max_iter = 100;
		float lambda = 0.1f;

		//Compute optical flow on the GPU
		cv::ocl::calcOpticalFlowHS(OclMat(frame0gray), OclMat(frame1gray), velXgpu, velYgpu, IterCriteria, lambda);

		//Download velocity vectors to the CPU
		velXgpu.download(velX);
		velYgpu.download(velY);

		Mat vel(velX.rows, velX.cols, CV_32FC2);
		Mat _vel[2];
		_vel[0] = velX;
		_vel[1] = velY;
		merge(_vel, 2, vel);

		for(int i=0;i<1024;i++)
			cout<<(float)velX.data[i]<<endl;

		//Draw the motion vectors on the source image
		drawOptFlowMap(vel, frame0color, 8, 1.5, CV_RGB(0, 255, 0));

		imshow("Dancing man",frame0color);
		if(waitKey(30)>=0)
            break;

		//Cleanup
		frame0gray.release();
		frame1gray.release();
		velX.release();
		velY.release();
		velXgpu.release();
		velYgpu.release();
		
	}

		//Some more cleanup
		frame0color.release();
		frame1color.release();
}