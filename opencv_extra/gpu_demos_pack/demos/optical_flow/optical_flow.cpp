#include <utility_lib/utility_lib.h>
#include "opencv2/core/core.hpp"
#include "opencv2/features2d/features2d.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/gpu/gpu.hpp"

#define PARAM_SCALE     "--scale"
#define PARAM_ALPHA     "--alpha"
#define PARAM_GAMMA     "--gamma"
#define PARAM_INNER     "--inner"
#define PARAM_OUTER     "--outer"
#define PARAM_SOLVER    "--solver"
#define PARAM_TIME_STEP "--time-step"
#define PARAM_HELP      "--help"

using namespace std;
using namespace cv;
using namespace cv::gpu;

class App : public BaseApp
{
public:
    App() : flow(0.197f /*alpha*/, 50.0f /*gamma*/, 0.8f /*scale_factor*/, 
                 10 /*inner_iterations*/, 77 /*outer_iterations*/, 10 /*solver_iterations*/),
            the_same_video_offset(1) {}

    virtual void run(int argc, char **argv);
    virtual void parseCmdArgs(int argc, char **argv);
    virtual void printHelp();
    virtual bool processKey(int key);

    int the_same_video_offset;
    BroxOpticalFlow flow;
};

template <typename T> inline T clamp (T x, T a, T b)
{
    return ((x) > (a) ? ((x) < (b) ? (x) : (b)) : (a));
}

template <typename T> inline T mapValue(T x, T a, T b, T c, T d)
{
    x = clamp(x, a, b);
    return c + (d - c) * (x - a) / (b - a);
}

void getFlowField(const Mat& u, const Mat& v, Mat& flowField)
{
    float maxDisplacement = 1.0f;

    for (int i = 0; i < u.rows; ++i)
    {
        const float* ptr_u = u.ptr<float>(i);
        const float* ptr_v = v.ptr<float>(i);

        for (int j = 0; j < u.cols; ++j)
        {
            float d = max(fabsf(ptr_u[j]), fabsf(ptr_v[j]));

            if (d > maxDisplacement) 
                maxDisplacement = d;
        }
    }

    flowField.create(u.size(), CV_8UC4);

    for (int i = 0; i < flowField.rows; ++i)
    {
        const float* ptr_u = u.ptr<float>(i);
        const float* ptr_v = v.ptr<float>(i);

        Vec4b* row = flowField.ptr<Vec4b>(i);

        for (int j = 0; j < flowField.cols; ++j)
        {
            row[j][0] = 0;
            row[j][1] = static_cast<unsigned char> (mapValue (-ptr_v[j], -maxDisplacement, maxDisplacement, 0.0f, 255.0f));
            row[j][2] = static_cast<unsigned char> (mapValue ( ptr_u[j], -maxDisplacement, maxDisplacement, 0.0f, 255.0f));
            row[j][3] = 255;
        }
    }
}

void App::run(int argc, char **argv)
{
    parseCmdArgs(argc, argv);
    if (help_showed) 
        return;

    if (sources.size() == 1 && dynamic_cast<VideoSource*>(static_cast<FrameSource*>(sources[0])) != 0)
    {
        sources.push_back(new VideoSource(sources[0]->path()));
        Mat tmp; 
        for (int i = 0; i < the_same_video_offset; ++i)
            sources.back()->next(tmp);
    }
    else if (sources.size() != 2)
    {
        cout << "Using default frame sources...\n";
        sources.resize(2);
        sources[0] = new ImageSource("data/optical_flow/rubberwhale1.png");
        sources[1] = new ImageSource("data/optical_flow/rubberwhale2.png");
    }

    Mat frame0Color, frame1Color;
    Mat frame0Gray, frame1Gray;

    GpuMat d_frame0, d_frame1;
    
    Mat fu, fv;
    Mat bu, bv;

    GpuMat d_fu, d_fv;
    GpuMat d_bu, d_bv;

    Mat flowFieldForward;
    Mat flowFieldBackward;

    double total_fps = 0;

    while (!exited)
    {
        int64 start = getTickCount();

        sources[0]->next(frame0Color);
        sources[1]->next(frame1Color);

        imshow("optical_flow_demo - frame 0", frame0Color);
        imshow("optical_flow_demo - frame 1", frame1Color);        

        frame0Color.convertTo(frame0Color, CV_32F, 1.0 / 255.0);
        frame1Color.convertTo(frame1Color, CV_32F, 1.0 / 255.0);

        cvtColor(frame0Color, frame0Gray, COLOR_BGR2GRAY);
        cvtColor(frame1Color, frame1Gray, COLOR_BGR2GRAY);

        d_frame0.upload(frame0Gray);
        d_frame1.upload(frame1Gray);

        int64 proc_start = getTickCount();

        flow(d_frame0, d_frame1, d_fu, d_fv);
        //flow(d_frame1, d_frame0, d_bu, d_bv);

        double proc_fps = getTickFrequency()  / (getTickCount() - proc_start);        

        d_fu.download(fu);
        d_fv.download(fv);
        
        //d_bu.download(bu);
        //d_bv.download(bv);

        getFlowField(fu, fv, flowFieldForward);
        //getFlowField(bu, bv, flowFieldBackward);

        stringstream msg; msg << "total FPS = " << setprecision(4) << total_fps;
        putText(flowFieldForward, msg.str(), Point(0, 30), FONT_HERSHEY_SIMPLEX, 1, Scalar::all(255));
        //putText(flowFieldBackward, msg.str(), Point(0, 30), FONT_HERSHEY_SIMPLEX, 1, Scalar::all(255));
        msg.str(""); msg << "processing FPS = " << setprecision(4) << proc_fps;
        putText(flowFieldForward, msg.str(), Point(0, 60), FONT_HERSHEY_SIMPLEX, 1, Scalar::all(255));
        //putText(flowFieldBackward, msg.str(), Point(0, 60), FONT_HERSHEY_SIMPLEX, 1, Scalar::all(255));

        imshow("optical_flow_demo - forward flow", flowFieldForward);
        //imshow("optical_flow_demo - backward flow", flowFieldBackward);

        processKey(waitKey(3));

        total_fps = getTickFrequency()  / (getTickCount() - proc_start);
    }
}

void App::printHelp()
{
    cout << "Usage: demo_optical_flow <frames_source1> <frames_source2>\n";
    cout << setiosflags(ios::left);
    cout << "\t" << setw(15) << PARAM_ALPHA << " - set alpha\n";
    cout << "\t" << setw(15) << PARAM_GAMMA << " - set gamma\n";
    cout << "\t" << setw(15) << PARAM_INNER << " - set number of inner iterations\n";
    cout << "\t" << setw(15) << PARAM_OUTER << " - set number of outer iterations\n";
    cout << "\t" << setw(15) << PARAM_SCALE << " - set pyramid scale factor\n";
    cout << "\t" << setw(15) << PARAM_SOLVER << " - set number of basic solver iterations\n";
    cout << "\t" << setw(15) << PARAM_TIME_STEP << " - set frame interpolation time step\n";
    cout << "\t" << setw(15) << PARAM_HELP << " - display this help message\n";
    cout << "\t" << setw(15) << "--offset" << " - set frames offset for the duplicate video source\n"
         << "(in case when ony one video or camera source is given). The dafault is 1.\n\n";
    BaseApp::printHelp();
}

void App::parseCmdArgs(int argc, char **argv)
{
    for (int i = 1; i < argc && !help_showed; ++i)
    {
        if (parseBaseCmdArgs(i, argc, argv))
            continue;
        else if (string(argv[i]) == PARAM_SCALE)
            flow.scale_factor = static_cast<float>(atof(argv[++i]));
        else if (string(argv[i]) == PARAM_ALPHA)
            flow.alpha = static_cast<float>(atof(argv[++i]));
        else if (string(argv[i]) == PARAM_GAMMA)
            flow.gamma = static_cast<float>(atof(argv[++i]));
        else if (string(argv[i]) == PARAM_INNER)
            flow.inner_iterations = atoi(argv[++i]);
        else if (string(argv[i]) == PARAM_OUTER)
            flow.outer_iterations = atoi(argv[++i]);
        else if (string(argv[i]) == PARAM_SOLVER)
            flow.solver_iterations = atoi(argv[++i]);
        else if (string(argv[i]) == "--offset")
            the_same_video_offset = atoi(argv[++i]);
        else
            throwBadArgError(argv[i]);
    }
}

bool App::processKey(int key)
{
    if (BaseApp::processKey(key))
        return true;
    return false;
}


RUN_APP(App)