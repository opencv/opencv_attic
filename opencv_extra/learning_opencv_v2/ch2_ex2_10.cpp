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
#include "cv.h"
#include "highgui.h"
#include <stdio.h>

// Convert a video to grayscale
// argv[1]: input video file
// argv[2]: name of new output file
//

//#define NOWRITE 1;   //Turn this on (removed the first comment out "//" if you can't write on linux

main( int argc, char* argv[] ) {
    cvNamedWindow( "Example2_10", CV_WINDOW_AUTOSIZE );
    cvNamedWindow( "Log_Polar", CV_WINDOW_AUTOSIZE );
    CvCapture* capture = cvCreateFileCapture( argv[1] );
    if (!capture){
        return -1;
    }
    IplImage* bgr_frame;
    double fps = cvGetCaptureProperty (
        capture,
        CV_CAP_PROP_FPS
    );
	printf("fps=%d\n",(int)fps);

    CvSize size = cvSize(
        (int)cvGetCaptureProperty( capture, CV_CAP_PROP_FRAME_WIDTH),
        (int)cvGetCaptureProperty( capture, CV_CAP_PROP_FRAME_HEIGHT)
    );
    
    printf("frame (w, h) = (%d, %d)\n",size.width,size.height);
#ifndef NOWRITE
 CvVideoWriter* writer = cvCreateVideoWriter(  // On linux Will only work if you've installed ffmpeg development files correctly, 
        argv[2],                               // otherwise segmentation fault.  Windows probably better.
        CV_FOURCC('D','X','5','0'),    
        fps,
        size
    );
#endif
    IplImage* logpolar_frame = cvCreateImage(
        size,
        IPL_DEPTH_8U,
        3
    );

    IplImage* gray_frame = cvCreateImage(
        size,
        IPL_DEPTH_8U,
        1
    );
 
    while( (bgr_frame=cvQueryFrame(capture)) != NULL ) {
        cvShowImage( "Example2_10", bgr_frame );
        cvConvertImage(   //We never make use of this gray image
            bgr_frame,
            gray_frame,
            CV_RGB2GRAY
        );
        cvLogPolar( bgr_frame, logpolar_frame,  //This is just a fun conversion the mimic's the human visual system
                    cvPoint2D32f(bgr_frame->width/2,
                    bgr_frame->height/2), 
                    40, 
                    CV_INTER_LINEAR+CV_WARP_FILL_OUTLIERS );
        cvShowImage( "Log_Polar", logpolar_frame );
        //Sigh, on linux, depending on your ffmpeg, this often won't work ...
#ifndef NOWRITE
       cvWriteToAVI( writer, logpolar_frame );
#endif
        char c = cvWaitKey(10);
        if( c == 27 ) break;
    }
#ifndef NOWRITE
    cvReleaseVideoWriter( &writer );
#endif
    cvReleaseImage( &gray_frame );
    cvReleaseImage( &logpolar_frame );
    cvReleaseCapture( &capture );
}
