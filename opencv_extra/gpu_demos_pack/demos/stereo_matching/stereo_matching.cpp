#include <utility_lib/utility_lib.h>
#include "opencv2/core/core.hpp"
#include "opencv2/calib3d/calib3d.hpp"
#include "opencv2/features2d/features2d.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/gpu/gpu.hpp"

using namespace std;
using namespace cv;
using namespace cv::gpu;

class App : public BaseApp
{
public:
    enum {BM_CPU, BM, BP, CSBP} method;
    App() : method(BM), ndisp(64), use_gpu(true), fixed_method(false) {}

    virtual void run(int argc, char **argv);
    virtual void parseCmdArgs(int argc, char **argv);
    virtual void printHelp();
    virtual bool processKey(int key);
    void printCommands();
    void printParams();

    string method_str() const
    {
        switch (method)
        {
        case BM_CPU: return "BM_CPU";
        case BM: return "BM_GPU";
        case BP: return "BP_GPU";
        case CSBP: return "CSBP_GPU";
        }
        return "";
    }

    string left_name, right_name;
    int ndisp;

    Mat left_src, right_src;
    Mat left, right; 
    GpuMat d_left, d_right;

    StereoBM bm_cpu;
    StereoBM_GPU bm;
    StereoBeliefPropagation bp;
    StereoConstantSpaceBP csbp;
    bool fixed_method;

    bool use_gpu;
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
        sources[0] = new ImageSource("data/stereo_matching/tsukuba1.png");
        sources[1] = new ImageSource("data/stereo_matching/tsukuba2.png");
    }

    printCommands();
    printParams();

    Mat disp, disp_16s;
    gpu::GpuMat d_disp;

    double total_fps = 0;

    while (!exited)
    {
        int64 start = getTickCount();

        sources[0]->next(left_src);
        sources[1]->next(right_src);
        makeGray(left_src, left);
        makeGray(right_src, right);
        d_left.upload(left);
        d_right.upload(right);

        imshow("left", left);
        imshow("right", right);

	    // Set common parameters
        bm.ndisp = ndisp;
        bm_cpu.init(StereoBM::BASIC_PRESET, bm.ndisp, bm.winSize);
        bp.ndisp = ndisp;
        csbp.ndisp = ndisp;

        // Prepare disparity map of specified type
        disp.create(left.size(), CV_8U);
        d_disp.create(left.size(), CV_8U);

        int64 proc_start = getTickCount();

        switch (method)
        {
        case BM_CPU: bm_cpu(left, right, disp_16s); disp_16s.convertTo(disp, CV_8U, 255. / (16. * ndisp)); break;
        case BM: bm(d_left, d_right, d_disp); break;
        case BP: bp(d_left, d_right, d_disp); break;
        case CSBP: csbp(d_left, d_right, d_disp); break;
        }

        double proc_fps = getTickFrequency()  / (getTickCount() - proc_start);        

        if (method != BM_CPU)
        {
            d_disp.download(disp);
            disp *= 255. / ndisp;
        }

        stringstream msg; 
        msg << "method = " << method_str();
        putText(disp, msg.str(), Point(0, 30), FONT_HERSHEY_SIMPLEX, 1, Scalar::all(255));
        msg.str(""); msg << "total FPS = " << setprecision(3) << total_fps;
        putText(disp, msg.str(), Point(0, 60), FONT_HERSHEY_SIMPLEX, 1, Scalar::all(255));
        msg.str(""); msg << "processing FPS = " << setprecision(3) << proc_fps;
        putText(disp, msg.str(), Point(0, 90), FONT_HERSHEY_SIMPLEX, 1, Scalar::all(255));
        imshow("stereo_matching_demo - disp map", disp);

        processKey(waitKey(3));

        total_fps = getTickFrequency()  / (getTickCount() - proc_start);
    }
}

void App::printHelp()
{
    cout << "Usage: demo_stereo_matching\n"
         << "    <left_frame_source> <right_frame_source> # rectified\n"
         << "    -m <stereo_match_method> # BM | BP | CSBP\n"
         << "    --ndisp <num_disparity_levels>\n\n";
    BaseApp::printHelp();
}

void App::parseCmdArgs(int argc, char **argv)
{
    for (int i = 1; i < argc && !help_showed; ++i)
    {
        if (parseBaseCmdArgs(i, argc, argv))
            continue;
        if (string(argv[i]) == "-m")
        {
            if (string(argv[i + 1]) == "BM_CPU") method = BM_CPU;
            else if (string(argv[i + 1]) == "BM") method = BM;
            else if (string(argv[i + 1]) == "BP") method = BP;
            else if (string(argv[i + 1]) == "CSBP") method = CSBP;
            else throw runtime_error("Unknown stereo matching method: " + string(argv[i + 1]));
            i++;
        }
        else if (string(argv[i]) == "--ndisp") 
            ndisp = atoi(argv[++i]);
        else if (string(argv[i]) == "--fixedmethod")
            fixed_method = true;
        else
            throwBadArgError(argv[i]);
    }
}

bool App::processKey(int key)
{
    if (key >= 0)
        key = key & 0xff;

    if (BaseApp::processKey(key))
        return true;

    switch (toupper(key))
    {
    case 32:
        if (method == BM_CPU)
            method = BM;
        else if (method == BM)
            method = BM_CPU;
        else
            cout << "Only BM supports CPU/GPU modes\n";
        break;
    case 'P':
        printParams();
        break;  
    case 'M':
        if (fixed_method)
            cout << "Method is fixed\n";
        else
        {
            switch (method)
            {
            case BM_CPU:
                method = BP;
                break;
            case BM:
                method = BP;
                break;
            case BP:
                method = CSBP;
                break;
            case CSBP:
                method = BM_CPU;
                break;
            }
            cout << "method: " << method_str() << endl;
        }
        break;
    case '1':
        ndisp = ndisp < 16 ? 16 : ndisp + 16;
        cout << "ndisp: " << ndisp << endl;
        bm_cpu.init(StereoBM::BASIC_PRESET, ndisp);
        bm.ndisp = ndisp;
        bp.ndisp = ndisp;
        csbp.ndisp = ndisp;
        break;
    case 'Q':
        ndisp = max(ndisp - 16, 16);
        cout << "ndisp: " << ndisp << endl;
        bm_cpu.init(StereoBM::BASIC_PRESET, ndisp);
        bm.ndisp = ndisp;
        bp.ndisp = ndisp;
        csbp.ndisp = ndisp;
        break;
    case '2':
        if (method == BM || method == BM_CPU)
        {
            bm.winSize = min(bm.winSize + 2, 51);
            bm_cpu.init(StereoBM::BASIC_PRESET, ndisp, bm.winSize);
            cout << "win_size: " << bm.winSize << endl;
        }
        break;
    case 'W':
        if (method == BM || method == BM_CPU)
        {
            bm.winSize = max(bm.winSize - 2, 1);
            bm_cpu.init(StereoBM::BASIC_PRESET, ndisp, bm.winSize);
            cout << "win_size: " << bm.winSize << endl;
        }
        break;
    case '3':
        if (method == BP)
        {
            bp.iters += 1;
            cout << "iter_count: " << bp.iters << endl;
        }
        else if (method == CSBP)
        {
            csbp.iters += 1;
            cout << "iter_count: " << csbp.iters << endl;
        }
        break;
    case 'E':
        if (method == BP)
        {
            bp.iters = max(bp.iters - 1, 1);
            cout << "iter_count: " << bp.iters << endl;
        }
        else if (method == CSBP)
        {
            csbp.iters = max(csbp.iters - 1, 1);
            cout << "iter_count: " << csbp.iters << endl;
        }
        break;
    case '4':
        if (method == BP)
        {
            bp.levels += 1;
            cout << "level_count: " << bp.levels << endl;
        }
        else if (method == CSBP)
        {
            csbp.levels += 1;
            cout << "level_count: " << csbp.levels << endl;
        }
        break;
    case 'R':
        if (method == BP)
        {
            bp.levels = max(bp.levels - 1, 1);
            cout << "level_count: " << bp.levels << endl;
        }
        else if (method == CSBP)
        {
            csbp.levels = max(csbp.levels - 1, 1);
            cout << "level_count: " << csbp.levels << endl;
        }
        break;
    default:
        return false;
    }
    return true;
}

void App::printCommands()
{
    cout << "\nControls:\n"
        << "\tesc - exit\n"
        << "\tspace - toggle CPU/GPU mode (for BM only)\n"
        << "\tp - print current parameters\n"
        << "\tm - change stereo match method\n"
        << "\t1/q - increase/decrease maximum disparity\n"
        << "\t2/w - increase/decrease window size (for BM only)\n"
        << "\t3/e - increase/decrease iteration count (for BP and CSBP only)\n"
        << "\t4/r - increase/decrease level count (for BP and CSBP only)\n";
}

void App::printParams()
{
    cout << "--- Parameters ---\n";
    cout << "image_size: (" << left.cols << ", " << left.rows << ")\n";
    cout << "image_channels: " << left.channels() << endl;
    cout << "method: " << method_str() << endl
        << "ndisp: " << ndisp << endl;
    switch (method)
    {
    case BM_CPU:
        cout << "win_size: " << bm.winSize << endl;
        break;
    case BM:
        cout << "win_size: " << bm.winSize << endl;
        break;
    case BP:
        cout << "iter_count: " << bp.iters << endl;
        cout << "level_count: " << bp.levels << endl;
        break;
    case CSBP:
        cout << "iter_count: " << csbp.iters << endl;
        cout << "level_count: " << csbp.levels << endl;
        break;
    }
    cout << endl;
}

RUN_APP(App)
