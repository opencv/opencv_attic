//
// example 6-1 Hough circles
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
//
/*
You'll have to tune to detect the circles you expect
vSeq* cvHoughCircles( 
	CvArr* image,			//Da image
	void* storage,			//Storage for sequences
	int method, 			//Always CV_HOUGH_GRADIENT until you, reader, invent a better method
	double dp, 				//Hough space shrinkage. a bit larger is faster, might detect better,
	double min_dist,		//Damps out multiple detection in the same area
   double param1=100,   //High Canny threshold (edge thresh), low is half (link finding). See Canny
   double param2=100,   //Threshold where we declare detection in Canny space
   int min_radius=0,    //Smallest circle to find
   int max_radius=0     //Largest circle to find
  );
*/



#include <cv.h>
#include <highgui.h>
#include <math.h>

int main(int argc, char** argv) {
  IplImage* image = cvLoadImage( 
    argv[1],
    CV_LOAD_IMAGE_GRAYSCALE
  );
 IplImage* src = cvLoadImage( argv[1] ); //Changed for prettier show in color
  CvMemStorage* storage = cvCreateMemStorage(0);
  cvSmooth(image, image, CV_GAUSSIAN, 5, 5 );
   CvSeq* results = cvHoughCircles( 
    image, 
    storage, 
    CV_HOUGH_GRADIENT, 
    4, 
    image->width/10 
  ); 
  for( int i = 0; i < results->total; i++ ) {
    float* p = (float*) cvGetSeqElem( results, i );
    CvPoint pt = cvPoint( cvRound( p[0] ), cvRound( p[1] ) );
    cvCircle( 
      src,
      pt, 
      cvRound( p[2] ),
      CV_RGB(0xff,0,0) 
    );
  }
  cvNamedWindow( "cvHoughCircles", 1 );
  cvShowImage( "cvHoughCircles", src);
  cvWaitKey(0);
}

