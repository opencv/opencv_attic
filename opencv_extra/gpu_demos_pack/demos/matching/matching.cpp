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
    bool operator()(const pair<int, KeyPoint> &p1, const pair<int, KeyPoint> &p2) const
    {
        if (p1.second.pt.x < p2.second.pt.x)
            return true;
        if (p1.second.pt.x > p2.second.pt.x)
            return false;
        return p1.second.pt.y < p2.second.pt.y;
    }
};

class MatchLess 
{
public:
    bool operator()(const DMatch &m1, const DMatch &m2) const
    {
        if (m1.queryIdx < m2.queryIdx)
            return true;
        return m1.queryIdx == m2.queryIdx && m1.trainIdx < m2.trainIdx;
    }
};

void App::run(int argc, char **argv)
{
    parseCmdArgs(argc, argv);
    if (help_showed) 
        return;
    if (sources.size() != 2) 
    {
        cout << "Loading default images...\n";
        sources.resize(2);
        sources[0] = new ImageSource("data/matching/t34mA.jpg");
        sources[1] = new ImageSource("data/matching/t34mB.jpg");
    }

    cout << "\nControls:\n"
         << "  space - chanege CPU/GPU mode\n\n";

    Mat h_img1, h_img2, h_img1_gray, h_img2_gray;

    SURF surf_cpu(1000);

    vector<KeyPoint> keypoints1_cpu, keypoints2_cpu;
    vector<float> descriptors1_cpu, descriptors2_cpu;
    GpuMat keypoints1_gpu, keypoints2_gpu;
    GpuMat descriptors1_gpu, descriptors2_gpu;

    BruteForceMatcher< L2<float> > matcher_cpu;
    BruteForceMatcher_GPU< L2<float> > matcher_gpu;
    vector<vector<DMatch> > matches;
    vector<DMatch> good_matches;

    sources[0]->next(h_img1);
    sources[1]->next(h_img2);
    makeGray(h_img1, h_img1_gray);
    makeGray(h_img2, h_img2_gray);

    surf_cpu(h_img1_gray, Mat(), keypoints1_cpu, descriptors1_cpu);
    surf_cpu(h_img2_gray, Mat(), keypoints2_cpu, descriptors2_cpu);

    double total_fps = 0;

    while (!exited)
    {
        int64 start = getTickCount();

        int64 proc_start = getTickCount();

        if (use_gpu)
        {
            descriptors1_gpu.upload(Mat(descriptors1_cpu).reshape(0, keypoints1_cpu.size()));
            descriptors2_gpu.upload(Mat(descriptors2_cpu).reshape(0, keypoints2_cpu.size()));
            matcher_gpu.knnMatch(descriptors1_gpu, descriptors2_gpu, matches, 2);
        }
        else
            matcher_cpu.knnMatch(Mat(descriptors1_cpu).reshape(0, keypoints1_cpu.size()),
                                 Mat(descriptors2_cpu).reshape(0, keypoints2_cpu.size()),
                                 matches, 2);

        good_matches.clear();
        good_matches.reserve(matches.size());

        for (size_t i = 0; i < matches.size(); ++i)
        {
            if (matches[i].size() < 2)
                continue;
            const DMatch &m1 = matches[i][0];
            const DMatch &m2 = matches[i][1];
            if (m1.distance < m2.distance * 0.5)
                good_matches.push_back(m1);
        }

        double proc_fps = getTickFrequency()  / (getTickCount() - proc_start);

        Mat dst;
        theRNG() = RNG(0);
        drawMatches(h_img1, keypoints1_cpu, h_img2, keypoints2_cpu, good_matches, dst);
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
    cout << "\nThis program demonstrates using SURF_GPU features detector, descriptor extractor and BruteForceMatcher_GPU"
         << "\nUsage:\n\tdemo_surf <frame_source>\n\n"
         << "Only image sources are fully supported, in case of video or camera the first frame will be used.\n\n";
    BaseApp::printHelp();
}

RUN_APP(App)