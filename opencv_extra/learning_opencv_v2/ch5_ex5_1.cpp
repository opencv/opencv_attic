//
//Example 5-1. Doing something with each element in the sequence of connected components returned
//             by cvPyrSegmentation(
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
#include <stdio.h>

void f( 
  IplImage* src, 
  IplImage* dst 
) {
  CvMemStorage* storage = cvCreateMemStorage(0);
  CvSeq* comp = NULL;

  cvPyrSegmentation( src, dst, storage, &comp, 4, 200, 50 );
  int n_comp = comp->total;

  for( int i=0; i<n_comp; i++ ) {
    CvConnectedComp* cc = (CvConnectedComp*) cvGetSeqElem( comp, i );
    // do_something_with( cc );
  }
  cvReleaseMemStorage( &storage );
}

int main(int argc, char** argv)
{

  // Create a named window with a the name of the file.
  cvNamedWindow( argv[1], 1 );
  // Load the image from the given file name.
  IplImage* src = cvLoadImage( argv[1] );
  if(!src) { printf("Couldn't seem to Open %s, sorry\n",argv[1]); return -1;}
  IplImage* dst = cvCreateImage( cvGetSize(src), src->depth, src->nChannels);
  f( src, dst);

  // Show the image in the named window
  cvShowImage( argv[1], dst );

  // Idle until the user hits the "Esc" key.
  while( 1 ) { if( cvWaitKey( 100 ) == 27 ) break; }

  // Clean up and don’t be piggies
  cvDestroyWindow( argv[1] );
  cvReleaseImage( &src );
  cvReleaseImage( &dst );

}
