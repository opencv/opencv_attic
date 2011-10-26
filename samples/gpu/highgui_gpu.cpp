#include <iostream>
#include <sstream>

#include "opencv2/core/core.hpp"
#include "opencv2/calib3d/calib3d.hpp"
#include "opencv2/contrib/contrib.hpp"
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

    Mat img0Gray, img1Gray;
    cvtColor(img0, img0Gray, COLOR_BGR2GRAY);
    cvtColor(img1, img1Gray, COLOR_BGR2GRAY);

    Mat disp;

    static float Q_vals[4 * 4] = 
    {
        1,0,0,0,
        0,1,0,0,
        0,0,1,0,
        0,0,0,1
    };
    Mat Q(4, 4, CV_32F, Q_vals);

    namedWindow("CPU Disparity", WINDOW_NORMAL);
    
    // we must create at least one OpenGL Window to use CUDA code
    namedWindow("GPU Disparity", WINDOW_OPENGL | WINDOW_NORMAL);
    namedWindow("GPU Point Cloud", WINDOW_OPENGL | WINDOW_AUTOSIZE);

    // set a CUDA device to use OpenGL interoperability
    setGlDevice();

    GpuMat d_img0(img0);
    GpuMat d_img0Gray(img0Gray);
    GpuMat d_img1Gray(img1Gray);
    GpuMat d_disp;
    GpuMat d_xyzw;

    StereoBM bm(0, 256);
    StereoBM_GPU d_bm(0, 256);

    while (true)
    {
        TickMeter cpuTm, gpuTm, gpuReprojectTm;

        cpuTm.start();
        bm(img0Gray, img1Gray, disp);
        cpuTm.stop();

        disp.convertTo(disp, CV_8U, 1.0 / 16.0);

        gpuTm.start();
        d_bm(d_img0Gray, d_img1Gray, d_disp);
        gpuTm.stop();

        gpuReprojectTm.start();
        reprojectImageTo3D(d_disp, d_xyzw, Q);
        gpuReprojectTm.stop();

        ostringstream cpuFps;
        cpuFps << "FPS: " << 1.0 / cpuTm.getTimeSec();
        ostringstream gpuFps;
        gpuFps << "FPS: " << 1.0 / gpuTm.getTimeSec();
        ostringstream gpuReprojectFps;
        gpuReprojectFps << "FPS: " << 1.0 / gpuReprojectTm.getTimeSec();

        putText(disp, cpuFps.str(), Point(0, 35), FONT_HERSHEY_PLAIN, 2.0, Scalar::all(255));

        imshow("CPU Disparity", disp);
        imshow("GPU Disparity", d_disp, gpuFps.str());

        Camera camera;

        camera.lookAt(Point3d(0.0, 0.0, 170.0), Point3d(60.0, 60.0, 0.0), Point3d(0.0, -1.0, 0.0));
        camera.setScale(Point3d(0.1, 0.1, 0.1));

        pointCloudShow("GPU Point Cloud", d_xyzw, camera, d_img0);

        if (waitKey(1) >= 0)
            break;
    }

    return 0;
}
