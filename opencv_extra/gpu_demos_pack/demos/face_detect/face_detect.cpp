// WARNING: this sample is under construction! Use it on your own risk.
#if defined _MSC_VER && _MSC_VER >= 1400
#pragma warning(disable : 4100)
#endif

#include <utility_lib/utility_lib.h>
#include "opencv2/contrib/contrib.hpp"
#include "opencv2/objdetect/objdetect.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/gpu/gpu.hpp"

using namespace std;
using namespace cv;
using namespace cv::gpu;

class App : public BaseApp
{
public:
    App() : useGPU(true), scaleFactor(1.), findLargestObject(false), 
            filterRects(true), helpScreen(false) {}

    virtual void run(int argc, char **argv);
    virtual void parseCmdArgs(int argc, char **argv);
    virtual bool processKey(int key);
    virtual void printHelp();

    string cascade_name;
    CascadeClassifier_GPU cascade_gpu;
    CascadeClassifier cascade_cpu;

    /* parameters */
    bool useGPU;
    double scaleFactor;
    bool findLargestObject;
    bool filterRects;
    bool helpScreen;
};


template<class T>
void convertAndResize(const T& src, T& gray, T& resized, double scale)
{
    if (src.channels() == 3)
    {
        cvtColor( src, gray, CV_BGR2GRAY );
    }
    else
    {
        gray = src;
    }

    Size sz(cvRound(gray.cols * scale), cvRound(gray.rows * scale));

    if (scale != 1)
    {
        resize(gray, resized, sz);
    }
    else
    {
        resized = gray;
    }
}


void matPrint(Mat &img, int lineOffsY, Scalar fontColor, const string &ss)
{
    int fontFace = FONT_HERSHEY_DUPLEX;
    double fontScale = 0.8;
    int fontThickness = 2;
    Size fontSize = cv::getTextSize("T[]", fontFace, fontScale, fontThickness, 0);

    Point org;
    org.x = 1;
    org.y = 3 * fontSize.height * (lineOffsY + 1) / 2;
    putText(img, ss, org, fontFace, fontScale, CV_RGB(0,0,0), 5*fontThickness/2, 16);
    putText(img, ss, org, fontFace, fontScale, fontColor, fontThickness, 16);
}


void displayState(Mat &canvas, bool bHelp, bool bGpu, bool bLargestFace, bool bFilter, double fps)
{
    Scalar fontColorRed = CV_RGB(255,0,0);
    Scalar fontColorNV  = CV_RGB(118,185,0);

    ostringstream ss;
    ss << "FPS = " << setprecision(1) << fixed << fps;
    matPrint(canvas, 0, fontColorRed, ss.str());
    ss.str("");
    ss << "[" << canvas.cols << "x" << canvas.rows << "], " <<
        (bGpu ? "GPU, " : "CPU, ") <<
        (bLargestFace ? "OneFace, " : "MultiFace, ") <<
        (bFilter ? "Filter:ON" : "Filter:OFF");
    matPrint(canvas, 1, fontColorRed, ss.str());

    // by Anatoly. MacOS fix. ostringstream(const string&) is a private
    // matPrint(canvas, 2, fontColorNV, ostringstream("Space - switch GPU / CPU"));
    if (bHelp)
    {
        matPrint(canvas, 2, fontColorNV, "Space - switch GPU / CPU");
        matPrint(canvas, 3, fontColorNV, "M - switch OneFace / MultiFace");
        matPrint(canvas, 4, fontColorNV, "F - toggle rectangles Filter");
        matPrint(canvas, 5, fontColorNV, "H - toggle hotkeys help");
        matPrint(canvas, 6, fontColorNV, "1/Q - increase/decrease scale");
    }
    else
    {
        matPrint(canvas, 2, fontColorNV, "H - toggle hotkeys help");
    }
}


void App::run(int argc, char **argv)
{
    parseCmdArgs(argc, argv);
    if (help_showed) 
        return;

    if (getCudaEnabledDeviceCount() == 0)
        throw runtime_error("No GPU found or the library is compiled without GPU support");    

    if (cascade_name.empty())
    {
        cout << "Using default cascade file...\n";
        cascade_name = "data/face_detect/haarcascade_frontalface_alt.xml";
    }      

    if (!cascade_gpu.load(cascade_name) || !cascade_cpu.load(cascade_name))
    {
        stringstream msg;
        msg << "Could not load cascade classifier \"" << cascade_name << "\"";
        throw runtime_error(msg.str());
    }

    if (sources.size() != 1)
    {
        cout << "Loading default frames source...\n";
        sources.resize(1);
        sources[0] = new VideoSource("data/face_detect/browser.flv");
    }

    Mat frame, frame_cpu, gray_cpu, resized_cpu, faces_downloaded, frameDisp;
    vector<Rect> facesBuf_cpu;

    GpuMat frame_gpu, gray_gpu, resized_gpu, facesBuf_gpu;

    int detections_num;
    while (!exited)
    {
        sources[0]->next(frame_cpu);
        frame_gpu.upload(frame_cpu);

        convertAndResize(frame_gpu, gray_gpu, resized_gpu, scaleFactor);
        convertAndResize(frame_cpu, gray_cpu, resized_cpu, scaleFactor);

        TickMeter tm;
        tm.start();

        if (useGPU)
        {
            cascade_gpu.visualizeInPlace = true;
            cascade_gpu.findLargestObject = findLargestObject;

            detections_num = cascade_gpu.detectMultiScale(resized_gpu, facesBuf_gpu, 1.2,
                                                          (filterRects || findLargestObject) ? 4 : 0);
            facesBuf_gpu.colRange(0, detections_num).download(faces_downloaded);
        }
        else
        {
            Size minSize = cascade_gpu.getClassifierSize();
            cascade_cpu.detectMultiScale(resized_cpu, facesBuf_cpu, 1.2,
                                         (filterRects || findLargestObject) ? 4 : 0,
                                         (findLargestObject ? CV_HAAR_FIND_BIGGEST_OBJECT : 0)
                                            | CV_HAAR_SCALE_IMAGE,
                                         minSize);
            detections_num = (int)facesBuf_cpu.size();
        }

        if (!useGPU && detections_num)
        {
            for (int i = 0; i < detections_num; ++i)
            {
                rectangle(resized_cpu, facesBuf_cpu[i], Scalar(255));
            }
        }

        if (useGPU)
        {
            resized_gpu.download(resized_cpu);
        }

        tm.stop();
        double detectionTime = tm.getTimeMilli();
        double fps = 1000 / detectionTime;

        /*//print detections to console
        cout << setfill(' ') << setprecision(2);
        cout << setw(6) << fixed << fps << " FPS, " << detections_num << " det";
        if ((filterRects || findLargestObject) && detections_num > 0)
        {
            Rect *faceRects = useGPU ? faces_downloaded.ptr<Rect>() : &facesBuf_cpu[0];
            for (int i = 0; i < min(detections_num, 2); ++i)
            {
                cout << ", [" << setw(4) << faceRects[i].x
                     << ", " << setw(4) << faceRects[i].y
                     << ", " << setw(4) << faceRects[i].width
                     << ", " << setw(4) << faceRects[i].height << "]";
            }
        }
        cout << endl;*/

        cvtColor(resized_cpu, frameDisp, CV_GRAY2BGR);
        displayState(frameDisp, helpScreen, useGPU, findLargestObject, filterRects, fps);
        imshow("face_detect_demo", frameDisp);

        processKey(waitKey(3));
    }   
}


void App::parseCmdArgs(int argc, char **argv)
{
    for (int i = 1; i < argc && !help_showed; ++i)
    {
        if (parseBaseCmdArgs(i, argc, argv))
            continue;
        if (string(argv[i]) == "--cascade")
            cascade_name = argv[++i];
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
        useGPU = !useGPU;
        break;
    case 'M':
        findLargestObject = !findLargestObject;
        break;
    case 'F':
        filterRects = !filterRects;
        break;
    case '1':
        scaleFactor *= 1.05;
        break;
    case 'Q':
        scaleFactor /= 1.05;
        break;
    case 'H':
        helpScreen = !helpScreen;
        break;
    default:
        return false;
    }
    return true;
}


void App::printHelp()
{
    cout << "Usage: demo_face_detect --cascade <cascade_file> <frames_source>\n\n";
    BaseApp::printHelp();
}

RUN_APP(App)
