#ifndef UTILITY_LIB_H_
#define UTILITY_LIB_H_

#include <iostream>
#include <string>
#include <sstream>
#include <stdexcept>
#include <iomanip>
#include <algorithm>
#include <utility>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

class FrameSource
{
public:
    virtual ~FrameSource() {}
    virtual void next(cv::Mat &frame) = 0;
    virtual operator std::string() const = 0;
    virtual const std::string& path() const = 0;
};


class ImageSource : public FrameSource
{
public:
    ImageSource(const std::string &path, int flags = 1)
        : img_(cv::imread(path, flags)), path_(path), str_repr_("img: " + path)
    { 
        CV_Assert(!img_.empty()); 
    }

    virtual void next(cv::Mat &frame) { frame = img_; }
    virtual operator std::string() const { return str_repr_; }
    virtual const std::string& path() const { return path_; }

private:
    cv::Mat img_;
    std::string path_;
    std::string str_repr_;
};


class VideoSource : public FrameSource
{
public:
    VideoSource(const std::string &path)
        : vc_(path), path_(path), is_from_file_(true), str_repr_("video: " + path)
    { 
        CV_Assert(vc_.isOpened()); 
    }

    VideoSource(int device, int width = -1, int height = -1)
        : vc_(device), device_(device), is_from_file_(false)
    {
        if (width != -1) 
            vc_.set(CV_CAP_PROP_FRAME_WIDTH, width);
        if (height != -1) 
            vc_.set(CV_CAP_PROP_FRAME_HEIGHT, height);
        if (!vc_.isOpened())
        {
            std::stringstream msg;
            msg << "Can't open camera with dev. ID = " << device_;
            throw std::runtime_error(msg.str());
        }
        std::stringstream text;
        text << "cam: " << device;
        str_repr_ = text.str();
    }
    virtual void next(cv::Mat &frame)
    {
        vc_ >> frame;
        if (frame.empty() && is_from_file_)
        {
            vc_.open(path_);
            vc_ >> frame;
        }
    }
    virtual operator std::string() const { return str_repr_; }
    virtual const std::string& path() const { return path_; }

private:
    cv::VideoCapture vc_;
    std::string path_;
    int device_;
    bool is_from_file_;
    std::string str_repr_;
};


class BaseApp
{
public:
    enum { DEFAULT_WIDTH = -1, DEFAULT_HEIGHT = -1 };

    BaseApp() : exited(false), help_showed(false), 
                frame_width(DEFAULT_WIDTH), frame_height(DEFAULT_HEIGHT) {}

    virtual void run(int argc, char **argv)
    {
        parseCmdArgs(argc, argv);
    }

protected:
    virtual void parseCmdArgs(int argc, char **argv);
    void throwBadArgError(const char *arg);
    virtual bool parseHelpCmdArg(int& i, int argc, char **argv);
    virtual bool parseFrameSourcesCmdArgs(int& i, int argc, char **argv);
    virtual bool parseBaseCmdArgs(int& i, int argc, char **argv);

    virtual void printHelp();
    virtual bool processKey(int key);

    static void printText(const std::string &msg, cv::Mat &img);
    static void makeGray(const cv::Mat &src, cv::Mat &dst);

    bool exited;
    bool help_showed;
    int frame_width, frame_height;
    std::vector<cv::Ptr<FrameSource> > sources;
};

#define RUN_APP(App) \
    int main(int argc, char **argv) \
    { \
        try \
        { \
            App app; \
            app.run(argc, argv); \
        } \
        catch (const std::exception &e) \
        { \
            std::cout << "Error: " << e.what() << endl; \
            return -1; \
        } \
        return 0; \
    }

#endif // #ifndef UTILITY_LIB_H_