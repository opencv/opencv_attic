#include <iostream>
#include <iomanip>

#include <opencv2/core/core.hpp>
#include <opencv2/core/opengl_interop.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/gpu/gpu.hpp>

#include "utility_lib/utility_lib.h"

#define PARAM_SCALE     "--scale"
#define PARAM_ALPHA     "--alpha"
#define PARAM_GAMMA     "--gamma"
#define PARAM_INNER     "--inner"
#define PARAM_OUTER     "--outer"
#define PARAM_SOLVER    "--solver"
#define PARAM_OFFSET    "--offset"

using namespace std;
using namespace cv;
using namespace cv::gpu;

class App : public BaseApp
{
public:
    App() : flow(0.197f /*alpha*/, 50.0f /*gamma*/, 0.8f /*scale_factor*/, 
                 10 /*inner_iterations*/, 77 /*outer_iterations*/, 10 /*solver_iterations*/),
            the_same_video_offset(1) {}

protected:
    void process();
    bool parseCmdArgs(int& i, int argc, const char* argv[]);
    void printHelp();

private:
    int the_same_video_offset;
    BroxOpticalFlow flow;
    Ptr<PairFrameSource> source_;
};

bool App::parseCmdArgs(int& i, int argc, const char* argv[])
{
    string arg(argv[i]);

    if (arg == PARAM_SCALE)
    {
        ++i;

        if (i >= argc)
        {
            ostringstream msg;
            msg << "Missing value after " << PARAM_SCALE;
            throw runtime_error(msg.str());
        }

        flow.scale_factor = static_cast<float>(atof(argv[i]));
    }
    else if (arg == PARAM_ALPHA)
    {
        ++i;

        if (i >= argc)
        {
            ostringstream msg;
            msg << "Missing value after " << PARAM_ALPHA;
            throw runtime_error(msg.str());
        }

        flow.alpha = static_cast<float>(atof(argv[i]));
    }
    else if (arg == PARAM_GAMMA)
    {
        ++i;

        if (i >= argc)
        {
            ostringstream msg;
            msg << "Missing value after " << PARAM_GAMMA;
            throw runtime_error(msg.str());
        }

        flow.gamma = static_cast<float>(atof(argv[i]));
    }
    else if (arg == PARAM_INNER)
    {
        ++i;

        if (i >= argc)
        {
            ostringstream msg;
            msg << "Missing value after " << PARAM_INNER;
            throw runtime_error(msg.str());
        }

        flow.inner_iterations = atoi(argv[i]);
    }
    else if (arg == PARAM_OUTER)
    {
        ++i;

        if (i >= argc)
        {
            ostringstream msg;
            msg << "Missing value after " << PARAM_OUTER;
            throw runtime_error(msg.str());
        }

        flow.outer_iterations = atoi(argv[i]);
    }
    else if (arg == PARAM_SOLVER)
    {
        ++i;

        if (i >= argc)
        {
            ostringstream msg;
            msg << "Missing value after " << PARAM_SOLVER;
            throw runtime_error(msg.str());
        }

        flow.solver_iterations = atoi(argv[i]);
    }
    else if (arg == PARAM_OFFSET)
    {
        ++i;

        if (i >= argc)
        {
            ostringstream msg;
            msg << "Missing value after " << PARAM_OFFSET;
            throw runtime_error(msg.str());
        }

        the_same_video_offset = atoi(argv[i]);
    }
    else
        return false;

    return true;
}

void App::printHelp()
{
    cout << "This program demonstrates usage BroxOpticalFlow" << endl;
    cout << "Usage: demo_optical_flow <frames_source1> [<frames_source2>]" << endl;
    cout << setiosflags(ios::left);
    cout << '\t' << setw(15) << PARAM_ALPHA << " - set alpha" << endl;
    cout << '\t' << setw(15) << PARAM_GAMMA << " - set gamma" << endl;
    cout << '\t' << setw(15) << PARAM_INNER << " - set number of inner iterations" << endl;
    cout << '\t' << setw(15) << PARAM_OUTER << " - set number of outer iterations" << endl;
    cout << '\t' << setw(15) << PARAM_SCALE << " - set pyramid scale factor" << endl;
    cout << '\t' << setw(15) << PARAM_SOLVER << " - set number of basic solver iterations" << endl;
    cout << '\t' << setw(15) << PARAM_OFFSET << " - set frames offset for the duplicate video source" << endl;
    cout << "\t\t" << "(in case when ony one video or camera source is given). The default is 1." << endl;
    BaseApp::printHelp();
}

struct DrawData
{
    GlTexture tex;
    GlArrays arr;
    string total_fps;
    string proc_fps;
};

void drawCallback(void* userdata)
{
    DrawData* data = static_cast<DrawData*>(userdata);

    if (data->tex.empty() || data->arr.empty())
        return;

    static GlCamera camera;
    static bool init_camera = true;

    if (init_camera)
    {
        camera.setOrthoProjection(0.0, 1.0, 1.0, 0.0, 0.0, 1.0);
        camera.lookAt(Point3d(0.0, 0.0, 1.0), Point3d(0.0, 0.0, 0.0), Point3d(0.0, 1.0, 0.0));
        init_camera = false;
    }

    camera.setupProjectionMatrix();
    camera.setupModelViewMatrix();

    render(data->tex);
    render(data->arr, RenderMode::TRIANGLES);

    render(data->total_fps, GlFont::get("Courier New", 24, GlFont::WEIGHT_BOLD), Scalar::all(255), Point2d(3.0, 0.0));
    render(data->proc_fps, GlFont::get("Courier New", 24, GlFont::WEIGHT_BOLD), Scalar::all(255), Point2d(3.0, 30.0));
}

void App::process()
{
    if (sources.size() == 1)
        source_ = PairFrameSource::get(sources[0], the_same_video_offset);
    else if (sources.size() == 2)
        source_ = PairFrameSource::get(sources[0], sources[1]);
    else
    {
        cout << "Using default frame sources...\n";
        sources.resize(2);
        sources[0] = new ImageSource("data/optical_flow/rubberwhale1.png");
        sources[1] = new ImageSource("data/optical_flow/rubberwhale2.png");

        source_ = PairFrameSource::get(sources[0], sources[1]);
    }
    
    namedWindow("optical_flow_demo", WINDOW_OPENGL);
    setGlDevice();

    Mat frame0, frame1;

    GpuMat d_frame0Color, d_frame1Color;
    GpuMat d_frame0Color32F, d_frame1Color32F;
    GpuMat d_frame0Gray, d_frame1Gray;   
    GpuMat d_u, d_v;
    GpuMat d_vertex, d_colors;

    DrawData drawData;    
    setOpenGlDrawCallback("optical_flow_demo", drawCallback, &drawData);

    double total_fps = 0;

    while (!exited)
    {
        int64 start = getTickCount();

        source_->next(frame0, frame1);

        d_frame0Color.upload(frame0);
        d_frame1Color.upload(frame1);

        d_frame0Color.convertTo(d_frame0Color32F, CV_32F, 1.0 / 255.0);
        d_frame1Color.convertTo(d_frame1Color32F, CV_32F, 1.0 / 255.0);

        cvtColor(d_frame0Color32F, d_frame0Gray, COLOR_BGR2GRAY);
        cvtColor(d_frame1Color32F, d_frame1Gray, COLOR_BGR2GRAY);

        int64 proc_start = getTickCount();

        flow(d_frame1Gray, d_frame0Gray, d_u, d_v);

        createOpticalFlowNeedleMap(d_u, d_v, d_vertex, d_colors);

        double proc_fps = getTickFrequency()  / (getTickCount() - proc_start);

        stringstream total_fps_str; 
        total_fps_str << "Total FPS : " << setprecision(4) << total_fps;

        stringstream proc_fps_str; 
        proc_fps_str << "Processing FPS : " << setprecision(4) << proc_fps;

        drawData.tex.copyFrom(d_frame0Gray);        
        drawData.arr.setVertexArray(d_vertex);
        drawData.arr.setColorArray(d_colors, false);
        drawData.total_fps = total_fps_str.str();
        drawData.proc_fps = proc_fps_str.str();
        updateWindow("optical_flow_demo");

        processKey(waitKey(10) & 0xff);

        total_fps = getTickFrequency()  / (getTickCount() - proc_start);
    }
}

RUN_APP(App)
