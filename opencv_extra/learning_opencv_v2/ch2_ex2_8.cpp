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

IplImage* doCanny(
    IplImage* in,
    double    lowThresh,
    double    highThresh,
    double    aperture)
{
    IplImage* out = cvCreateImage( 
        cvGetSize( in ),
        in->depth, //IPL_DEPTH_8U,    
        1);
    cvCanny( in, out, lowThresh, highThresh, aperture );
    return( out );
};

IplImage* doPyrDown(
  IplImage* in,
  int       filter = IPL_GAUSSIAN_5x5)
{

    // Best to make sure input image is divisible by two.
    //
    assert( in->width%2 == 0 && in->height%2 == 0 );

    IplImage* out = cvCreateImage( 
        cvSize( in->width/2, in->height/2 ),
        in->depth,
        in->nChannels
    );
    cvPyrDown( in, out );
    return( out );
};

int main( int argc, char** argv )
{
  cvNamedWindow("Example Gray", CV_WINDOW_AUTOSIZE );
  cvNamedWindow("Example Pyr", CV_WINDOW_AUTOSIZE );
  cvNamedWindow("Example Canny", CV_WINDOW_AUTOSIZE );
  IplImage* img_rgb = cvLoadImage( argv[1] );
  IplImage* out;

  out = cvCreateImage( cvSize( img_rgb->width,img_rgb->height ), img_rgb->depth, 1);
  cvCvtColor(img_rgb, out ,CV_BGR2GRAY);
  cvShowImage("Example Gray", out );
  out = doPyrDown( out );
  out = doPyrDown( out );
  cvShowImage("Example Pyr", out );
  out = doCanny( out, 10, 100, 3 );
  cvShowImage("Example Canny", out );

  cvWaitKey(0);
  cvReleaseImage( &out);
  cvDestroyWindow("Example Gray");
  cvDestroyWindow("Example Pyr");
  cvDestroyWindow("Example Canny");
}
