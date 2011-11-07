#include <iostream>
#include "utility_lib.h"

using namespace std;
using namespace cv;

void BaseApp::parseCmdArgs(int argc, char **argv)
{
    for (int i = 1; i < argc && !help_showed; ++i)
    {
        if (parseBaseCmdArgs(i, argc, argv))
            continue;
        throwBadArgError(argv[i]);
    }
}


void BaseApp::throwBadArgError(const char *arg)
{
    throw runtime_error("Unknown command line argument: " + string(arg));
}


bool BaseApp::parseHelpCmdArg(int &i, int argc, char **argv)
{
    if (string(argv[i]) == "--help")
    {
        printHelp();
        help_showed = true;
        return true;
    }
    return false;
}


bool BaseApp::parseFrameSourcesCmdArgs(int &i, int argc, char **argv)
{
    if (string(argv[i]) == "-i") sources.push_back(new ImageSource(argv[++i]));
    else if (string(argv[i]) == "-v") sources.push_back(new VideoSource(argv[++i]));
    else if (string(argv[i]) == "-w") frame_width = atoi(argv[++i]);
    else if (string(argv[i]) == "-h") frame_height = atoi(argv[++i]);
    else if (string(argv[i]) == "-c") sources.push_back(new VideoSource(atoi(argv[++i]), frame_width, frame_height));
    else return false;
    return true;
}


bool BaseApp::parseBaseCmdArgs(int &i, int argc, char **argv)
{
    if (parseHelpCmdArg(i, argc, argv))
        return true;
    if (parseFrameSourcesCmdArgs(i, argc, argv))
        return true;
    return false;
}


void BaseApp::printHelp()
{
    cout << "Frames Source Command Args:\n"
         << "  -i <img_path>\n"
         << "       Image source path.\n"
         << "  -v <video_path>\n"
         << "       Video source path.\n"
         << "  -c <device_ID>\n"
         << "       Camera device ID\n"
         << "  -w <camera_frame_width>\n"
         << "  -h <camera_frame_height>\n";
}


bool BaseApp::processKey(int key)
{
    if (key == 27/*escape*/)
    {
        exited = true;
        return true;
    }
    return false;
}


void BaseApp::makeGray(const Mat &src, Mat &dst)
{
    if (src.channels() == 1)
        dst = src;
    else if (src.channels() == 3)
        cvtColor(src, dst, CV_BGR2GRAY);
    else
    {
        stringstream msg;
        msg << "Can't convert image to gray: unsupported #channels = " << src.channels();
        throw runtime_error(msg.str());
    }
}