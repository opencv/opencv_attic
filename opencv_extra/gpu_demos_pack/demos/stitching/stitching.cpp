#include <utility_lib/utility_lib.h>
#include "opencv2/core/core.hpp"
#include "opencv2/features2d/features2d.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/stitching/stitcher.hpp"
#include "opencv2/gpu/gpu.hpp"

using namespace std;
using namespace cv;
using namespace cv::gpu;

class App : public BaseApp
{
public:
    App() : use_gpu(true), 
            stitcher_cpu(Stitcher::createDefault(false)),
            stitcher_gpu(Stitcher::createDefault(true)) {}

    virtual void run(int argc, char **argv);
    virtual bool processKey(int key);
    virtual void printHelp();

    bool use_gpu;
    Stitcher stitcher_cpu;
    Stitcher stitcher_gpu;
};

void App::run(int argc, char **argv)
{
    parseCmdArgs(argc, argv);
    if (help_showed) 
        return;
    if (sources.empty()) 
    {
        cout << "Loading default images...\n";
        sources.resize(2);
        sources[0] = new ImageSource("data/stitching/t100mA.JPG");
        sources[1] = new ImageSource("data/stitching/t100mB.JPG");
    }

    stitcher_cpu.setFeaturesMatcher(new detail::BestOf2NearestMatcher(false, 0.5));
    stitcher_gpu.setFeaturesMatcher(new detail::BestOf2NearestMatcher(true, 0.5));

    cout << "\nControls:\n"
         << "  space - chanege CPU/GPU mode\n\n";

    vector<Mat> imgs(sources.size());
    Mat pano;

    double total_fps = 0;
    bool first_iter_cpu = true;
    bool first_iter_gpu = true;

    while (!exited)
    {
        int64 start = getTickCount();

        for (size_t i = 0; i < sources.size(); ++i)
            sources[i]->next(imgs[i]);

        int64 proc_start = getTickCount();

        if (use_gpu)
        {
            if (first_iter_gpu)
            {
                stitcher_gpu.estimateTransform(imgs);
                first_iter_gpu = false;
            }
            stitcher_gpu.composePanorama(imgs, pano);
        }
        else
        {
            if (first_iter_cpu)
            {
                stitcher_cpu.estimateTransform(imgs);
                first_iter_cpu = false;
            }
            stitcher_cpu.composePanorama(imgs, pano);
        }

        double proc_fps = getTickFrequency()  / (getTickCount() - proc_start);

        stringstream msg; msg << "total FPS = " << setprecision(4) << total_fps;
        putText(pano, msg.str(), Point(0, 30), FONT_HERSHEY_SIMPLEX, 1, Scalar::all(255));
        msg.str(""); msg << "processing FPS = " << setprecision(4) << proc_fps;
        putText(pano, msg.str(), Point(0, 60), FONT_HERSHEY_SIMPLEX, 1, Scalar::all(255));
        putText(pano, use_gpu ? "mode = GPU" : "mode = CPU", Point(0, 90), FONT_HERSHEY_SIMPLEX, 1, Scalar::all(255));

        imshow("stitching_demo", pano);
        cout << endl;
        processKey(waitKey(3));

        total_fps = getTickFrequency()  / (getTickCount() - proc_start);
    }
}

bool App::processKey(int key)
{
    if (key >= 0)
        key = key & 0xff;

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
    cout << "Rotation model images stitcher.\n\n"
         << "Usage: demo_stitching img1 img2 [...imgN]\n\n";
    BaseApp::printHelp();
}

RUN_APP(App)
