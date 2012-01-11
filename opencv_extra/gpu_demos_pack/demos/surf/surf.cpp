#include <utility_lib/utility_lib.h>
#include "opencv2/core/core.hpp"
#include "opencv2/features2d/features2d.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/gpu/gpu.hpp"
#include "opencv2/stitching/detail/matchers.hpp"

using namespace std;
using namespace cv;
using namespace cv::gpu;

class App : public BaseApp
{
public:
    App() : use_gpu(true), find_matches(true) {}

    virtual void run(int argc, char **argv);
    virtual bool processKey(int key);
    virtual void printHelp();

    bool use_gpu;
    bool find_matches;
};

class KeyPointLess
{
public:
    bool operator()(const KeyPoint &p1, const KeyPoint &p2) const
    {
        if (p1.pt.x < p2.pt.x)
            return true;
        if (p1.pt.x > p2.pt.x)
            return false;
        return p1.pt.y < p2.pt.y;
    }
};

void App::run(int argc, char **argv)
{
    parseCmdArgs(argc, argv);
    if (help_showed) 
        return;
    if (sources.size() != 1) 
    {
        cout << "Loading default images...\n";
        sources.resize(1);
        sources[0] = new ImageSource("data/surf/IMG_5252.JPG");
    }

    cout << "\nControls:\n"
         << "  space - chanege CPU/GPU mode\n\n";

    Mat h_img, h_img_gray;
    GpuMat img;

    SURF surf_cpu(500);
    SURF_GPU surf_gpu(500);

    detail::ImageFeatures features;

    vector<KeyPoint> keypoints1_cpu;
    vector<float> descriptors_cpu;

    GpuMat keypoints_gpu;
    GpuMat descriptors_gpu;

    double total_fps = 0;

    while (!exited)
    {
        int64 start = getTickCount();

        sources[0]->next(h_img);
        makeGray(h_img, h_img_gray);

        if (use_gpu)
        {
            img.upload(h_img_gray);
        }

        int64 proc_start = getTickCount();

        if (use_gpu)
        {
            surf_gpu(img, GpuMat(), keypoints_gpu);
            surf_gpu.downloadKeypoints(keypoints_gpu, features.keypoints);
        }
        else
        {
            surf_cpu(h_img_gray, Mat(), features.keypoints);
        }

        double proc_fps = getTickFrequency()  / (getTickCount() - proc_start);

        // As keypoints order can very from iteration to iteration even 
        // on the same image we set order here
        sort(features.keypoints.begin(), features.keypoints.end(), KeyPointLess());

        Mat dst;
        theRNG() = RNG(0);
        drawKeypoints(h_img, features.keypoints, dst);
        stringstream msg; msg << "total FPS = " << setprecision(4) << total_fps;
        putText(dst, msg.str(), Point(0, 30), FONT_HERSHEY_SIMPLEX, 1, Scalar::all(255));
        msg.str(""); msg << "processing FPS = " << setprecision(4) << proc_fps;
        putText(dst, msg.str(), Point(0, 60), FONT_HERSHEY_SIMPLEX, 1, Scalar::all(255));
        putText(dst, use_gpu ? "mode = GPU" : "mode = CPU", Point(0, 90), FONT_HERSHEY_SIMPLEX, 1, Scalar::all(255));
        
        imshow("surf_demo", dst);
        processKey(waitKey(3));

        total_fps = getTickFrequency()  / (getTickCount() - proc_start);
    }
}

bool App::processKey(int key)
{
    if (BaseApp::processKey(key))
        return true;
    switch (key)
    {
    case 32:
        use_gpu = !use_gpu;
        cout << "Use gpu = " << use_gpu << endl;
        break;
    default:
        return false;
    }
    return true;
}

void App::printHelp()
{
    cout << "\nThis program demonstrates using SURF_GPU features detector, descriptor extractor and BruteForceMatcher_GPU" << endl;
    cout << "\nUsage:\n\tdemo_surf <frame_source>\n";
    BaseApp::printHelp();
}

RUN_APP(App)