//
// Example 6-4. Log-polar transform example
// logPolar.cpp : Defines the entry point for the console applicatio
// 
//
//   input to second cvLogPolar does not need "CV_WARP_FILL_OUTLIERS": "M, CV_INTER_LINEAR+CV_WARP_FILL_OUTLIERS+CV_WARP_INVERSE_MAP );" --> "M, CV_INTER_LINEAR+CV_WARP_INVERSE_MAP );"
//
//
// ./ch6_ex6_4 image m
//     Where m is the scale factor, which should be set so that the 
//     features of interest dominate the available image area.
//
/* *************** License:**************************
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
   ************************************************** */
#include <cv.h>
#include <highgui.h>

int main(int argc, char** argv)
{
    IplImage* src;
   double M; 
    if( argc == 3 && ((src=cvLoadImage(argv[1],1)) != 0 ))
    {
      M = atof(argv[2]);
        IplImage* dst = cvCreateImage( cvGetSize(src), 8, 3 );
        IplImage* src2 = cvCreateImage( cvGetSize(src), 8, 3 );
        cvLogPolar( src,  dst, cvPoint2D32f(src->width/2,src->height/2),  
                    M, CV_INTER_LINEAR+CV_WARP_FILL_OUTLIERS );
        cvLogPolar( dst, src2, cvPoint2D32f(src->width/2,src->height/2),
                    M, CV_INTER_LINEAR+CV_WARP_INVERSE_MAP );
        cvNamedWindow( "log-polar", 1 );
        cvShowImage( "log-polar", dst );
        cvNamedWindow( "inverse log-polar", 1 );
        cvShowImage( "inverse log-polar", src2 );
        cvWaitKey();
    }
    return 0;
}
