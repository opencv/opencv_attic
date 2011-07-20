/* License:
   Oct. 3, 2008
   Right to use this code in any way you want without warrenty, support or any guarentee of it working.

   BOOK: It would be nice if you cited it:
   Learning OpenCV: Computer Vision with the OpenCV Library
     by Gary Bradski and Adrian Kaehler
     Published by O'Reilly Media, October 3, 2008
 
   AVAILABLE AT: 
     http://www.amazon.com/Learning-OpenCV-Computer-Vision-Library/dp/0596516134
     Or: http://oreilly.com/catalog/9780596516130/
     ISBN-10: 0596516134 or: ISBN-13: 978-0596516130    

   OTHER OPENCV SITES:
   * The source code is on sourceforge at:
     http://sourceforge.net/projects/opencvlibrary/
   * The OpenCV wiki page (As of Oct 1, 2008 this is down for changing over servers, but should come back):
     http://opencvlibrary.sourceforge.net/
   * An active user group is at:
     http://tech.groups.yahoo.com/group/OpenCV/
   * The minutes of weekly OpenCV development meetings are at:
     http://pr.willowgarage.com/wiki/OpenCV
*/
#include <stdio.h>
#include <iostream>
#include <fstream>
#include "cv.h"
#include "highgui.h"
/*
OK, you caught us.  Video playback under linux is still just bad.  Part of this is due to FFMPEG, part of this
is due to lack of standards in video files.  But the position slider here will often not work. We tried to at least
find number of frames using the "getAVIFrames" hack below.  Terrible.  But, this file shows something of how to
put a slider up and play with it.  Sorry.
*/


using namespace std;

int        g_slider_position = 0;
CvCapture* g_capture         = NULL;

void onTrackbarSlide(int pos) {
    cvSetCaptureProperty(
        g_capture,
        CV_CAP_PROP_POS_FRAMES,
        pos
    );
}

//Hack because sometimes the number of frames in a video is not accessible. 
//Probably delete this on Widows
int getAVIFrames(char * fname) { 
    char tempSize[4];
    // Trying to open the video file
    ifstream  videoFile( fname , ios::in | ios::binary );
    // Checking the availablity of the file
    if ( !videoFile ) {
      cout << "Couldn’t open the input file " << fname << endl;
      exit( 1 );
    }
    // get the number of frames
    videoFile.seekg( 0x30 , ios::beg );
    videoFile.read( tempSize , 4 );
    int frames = (unsigned char ) tempSize[0] + 0x100*(unsigned char ) tempSize[1] + 0x10000*(unsigned char ) tempSize[2] +    0x1000000*(unsigned char ) tempSize[3];
    videoFile.close(  );
    return frames;
}


int main( int argc, char** argv ) {
    cvNamedWindow( "Example2_3", CV_WINDOW_AUTOSIZE );
    g_capture = cvCreateFileCapture( argv[1] );
    IplImage *foo = cvQueryFrame( g_capture);


    int frames = (int) cvGetCaptureProperty(
        g_capture,
        CV_CAP_PROP_FRAME_COUNT
    );
    
    int tmpw = (int) cvGetCaptureProperty(
        g_capture,
        CV_CAP_PROP_FRAME_WIDTH
    );

    int tmph = (int) cvGetCaptureProperty(
        g_capture,
        CV_CAP_PROP_FRAME_HEIGHT
    );

    printf("opencv frames %d w %d h %d\n",frames,tmpw,tmph);

    frames = getAVIFrames(argv[1]); //This is a hack because on linux, getting number of frames often doesn't work

    printf("hacked frames %d w %d h %d\n",frames,tmpw,tmph);

    cvCreateTrackbar(
        "Position",
        "Example2_3",
        &g_slider_position,
        frames,
        onTrackbarSlide
    );
    IplImage* frame;
    frames = 0;
    while(1) {
        frame = cvQueryFrame( g_capture );
        if( !frame ) break;
//      int frames = cvGetCaptureProperty( g_capture, CV_CAP_PROP_POS_FRAMES);//This should work, sometimes it does not on linux
	frames++; //My cheat
	printf("\nFrame number=%d\n",frames);
        cvSetTrackbarPos("Position","Example2_3",frames);
        cvShowImage( "Example2_3", frame );
        char c = (char)cvWaitKey(10);
        if( c == 27 ) break;
    }
    cvReleaseCapture( &g_capture );
    cvDestroyWindow( "Example2_3" );
    return(0);
}
