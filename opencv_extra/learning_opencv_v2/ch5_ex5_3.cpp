//
// Example 5-3. Alternative method to combine and threshold image planes
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

#include <stdio.h>
#include <cv.h>
#include <highgui.h>


void sum_rgb( IplImage* src, IplImage* dst ) {
  // Allocate individual image planes.
  IplImage* r = cvCreateImage( cvGetSize(src), IPL_DEPTH_8U, 1 );
  IplImage* g = cvCreateImage( cvGetSize(src), IPL_DEPTH_8U, 1 );
  IplImage* b = cvCreateImage( cvGetSize(src), IPL_DEPTH_8U, 1 );

  // Temporary storage.
  IplImage* s = cvCreateImage(cvGetSize(src), IPL_DEPTH_32F, 1);
      
  // Split image onto the color planes.
  cvSplit( src, r, g, b, NULL );
     
  //Accumulate separate planes, combine and threshold
	cvZero(s);
	cvAcc(b,s);
	cvAcc(g,s);
	cvAcc(r,s);
	
	//Truncate values above 100 and rescale into dst
	cvThreshold( s, s, 100, 100, CV_THRESH_TRUNC );
	cvConvertScale( s, dst, 1, 0 );
 
  cvReleaseImage( &r );
  cvReleaseImage( &g );   
  cvReleaseImage( &b );   
  cvReleaseImage( &s );
}

int main(int argc, char** argv)
{

  // Create a named window with a the name of the file.
  cvNamedWindow( argv[1], 1 );

  // Load the image from the given file name.
  IplImage* src = cvLoadImage( argv[1] );
  IplImage* dst = cvCreateImage( cvGetSize(src), src->depth, 1);
  sum_rgb( src, dst);

  // Show the image in the named window
  cvShowImage( argv[1], dst );

  // Idle until the user hits the "Esc" key.
  while( 1 ) { if( (cvWaitKey( 10 )&0x7f) == 27 ) break; }

  // Clean up and don’t be piggies
  cvDestroyWindow( argv[1] );
  cvReleaseImage( &src );
  cvReleaseImage( &dst );

}
