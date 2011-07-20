//
// Example 8-2. Finding contours based on a trackbar’s location; the contours are updated
//              whenever the trackbar is moved
//
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
   ************************************************** *///
//


#include <cv.h>
#include <highgui.h>
#include <stdio.h>

IplImage*    g_image    = NULL;
IplImage*    g_gray    = NULL;
int        g_thresh  = 100;
CvMemStorage*  g_storage  = NULL;

void on_trackbar(int) {
  if( g_storage==NULL ) {
    g_gray = cvCreateImage( cvGetSize(g_image), 8, 1 );
    g_storage = cvCreateMemStorage(0);
  } else {
    cvClearMemStorage( g_storage );
  }
  CvSeq* contours = 0;
  cvCvtColor( g_image, g_gray, CV_BGR2GRAY );
  cvThreshold( g_gray, g_gray, g_thresh, 255, CV_THRESH_BINARY );
  cvFindContours( g_gray, g_storage, &contours );
  cvZero( g_gray );
  if( contours )
    cvDrawContours( 
      g_gray, 
      contours, 
      cvScalarAll(255),
      cvScalarAll(255), 
      100 
    );
  cvShowImage( "Contours", g_gray );
}

int main( int argc, char** argv )
{
  if( argc != 2 || !(g_image = cvLoadImage(argv[1])) ){
  printf("\nExample 8_2 Contour retreival using trackbar\nCall is:\n./ch8_ex8_2 image\n");
  return -1;}
  cvNamedWindow( "Contours", 1 );
  cvCreateTrackbar( 
    "Threshold", 
    "Contours", 
    &g_thresh, 
    255, 
    on_trackbar
  );
  on_trackbar(0);
  cvWaitKey();
  return 0; 
}
