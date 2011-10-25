#include <iostream>

#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/gpu/gpu.hpp"
#include "opencv2/gpu/highgui_gpu.hpp"

using namespace std;
using namespace cv;
using namespace cv::gpu;

int main(int argc, char* argv[])
{
    if (argc != 3)
        return -1;

    Mat img0 = imread(argv[1]);
    Mat img1 = imread(argv[2]);

    // set a CUDA device to use OpenGL interoperability
    setGLDevice();

    namedWindow("WINDOW_NORMAL", WINDOW_NORMAL);
    namedWindow("WINDOW_AUTOSIZE", WINDOW_AUTOSIZE);
    
    // we must create at least one OpenGL Window to use CUDA code
    namedWindow("WINDOW_OPENGL", WINDOW_OPENGL);
    namedWindow("WINDOW_OPENGL | WINDOW_AUTOSIZE", WINDOW_OPENGL | WINDOW_AUTOSIZE);

    GpuMat d_img0(img0);
    GpuMat d_img1(img1);

    imshow("WINDOW_NORMAL", img0);
    imshow("WINDOW_AUTOSIZE", img1);

    imshow("WINDOW_OPENGL", d_img0, "Hello, world!\nLA LA LA");
    imshow("WINDOW_OPENGL | WINDOW_AUTOSIZE", d_img1);

    waitKey();

    return 0;
}
