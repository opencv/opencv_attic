#include "cvaux.h"
#include "highgui.h"
#include <stdio.h>
#include <string>

using namespace cv;

int main(int argc, char** argv)
{
    VideoCapture cap;
    bool update_bg_model = true;
    
    if( argc < 2 )
        cap.open(0);
    else
        cap.open(std::string(argv[1]));
    
    if( !cap.isOpened() )
    {
        printf("can not open camera or video file\n");
        return -1;
    }
    
    Mat tmp_frame, bgmask;
    
    cap >> tmp_frame;
    if(!tmp_frame.data)
    {
        printf("can not read data from the video source\n");
        return -1;
    }
    
    namedWindow("video", 1);
    namedWindow("segmented", 1);
    
    BackgroundSubtractorMOG bgsubtractor;
    
    for(;;)
    {
        //double t = (double)cvGetTickCount();
        cap >> tmp_frame;
        if( !tmp_frame.data )
            break;
        bgsubtractor(tmp_frame, bgmask, update_bg_model ? -1 : 0);
        //t = (double)cvGetTickCount() - t;
        //printf( "%d. %.1f\n", fr, t/(cvGetTickFrequency()*1000.) );
        imshow("video", tmp_frame);
        imshow("segmented", bgmask);
        char keycode = waitKey(30);
        if( keycode == 27 ) break;
        if( keycode == ' ' )
            update_bg_model = !update_bg_model;
    }
    
    return 0;
}
