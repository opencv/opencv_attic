// Example 10-1. Pyramid Lucas-Kanade optical flow code
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
#include <cxcore.h>
#include <highgui.h>
#include <stdio.h>

const int MAX_CORNERS = 500;
int main(int argc, char** argv) {
   // Initialize, load two images from the file system, and
   // allocate the images and other structures we will need for
   // results.
	//
	IplImage* imgA = cvLoadImage("OpticalFlow0.jpg",CV_LOAD_IMAGE_GRAYSCALE);
	IplImage* imgB = cvLoadImage("OpticalFlow1.jpg",CV_LOAD_IMAGE_GRAYSCALE);
	CvSize      img_sz    = cvGetSize( imgA );
	int         win_size = 10;
	IplImage* imgC = cvLoadImage("OpticalFlow1.jpg",CV_LOAD_IMAGE_UNCHANGED);
	
	// The first thing we need to do is get the features
	// we want to track.
	//
	IplImage* eig_image = cvCreateImage( img_sz, IPL_DEPTH_32F, 1 );
	IplImage* tmp_image = cvCreateImage( img_sz, IPL_DEPTH_32F, 1 );
	int              corner_count = MAX_CORNERS;
	CvPoint2D32f* cornersA        = new CvPoint2D32f[ MAX_CORNERS ];
	cvGoodFeaturesToTrack(
		imgA,
		eig_image,
		tmp_image,
		cornersA,
		&corner_count,
		0.01,
		5.0,
		0,
		3,
		0,
		0.04
	);
	cvFindCornerSubPix(
		imgA,
		cornersA,
		corner_count,
		cvSize(win_size,win_size),
		cvSize(-1,-1),
		cvTermCriteria(CV_TERMCRIT_ITER|CV_TERMCRIT_EPS,20,0.03)
	);
	// Call the Lucas Kanade algorithm
	//
	char features_found[ MAX_CORNERS ];
	float feature_errors[ MAX_CORNERS ];
	CvSize pyr_sz = cvSize( imgA->width+8, imgB->height/3 );
	IplImage* pyrA = cvCreateImage( pyr_sz, IPL_DEPTH_32F, 1 );
  IplImage* pyrB = cvCreateImage( pyr_sz, IPL_DEPTH_32F, 1 );
  CvPoint2D32f* cornersB        = new CvPoint2D32f[ MAX_CORNERS ];
  cvCalcOpticalFlowPyrLK(
     imgA,
     imgB,
     pyrA,
     pyrB,
     cornersA,
     cornersB,
     corner_count,
     cvSize( win_size,win_size ),
     5,
     features_found,
     feature_errors,
     cvTermCriteria( CV_TERMCRIT_ITER | CV_TERMCRIT_EPS, 20, .3 ),
     0
  );
  // Now make some image of what we are looking at:
  //
  for( int i=0; i<corner_count; i++ ) {
     if( features_found[i]==0|| feature_errors[i]>550 ) {
 //       printf("Error is %f/n",feature_errors[i]);
        continue;
     }
 //    printf("Got it/n");
     CvPoint p0 = cvPoint(
        cvRound( cornersA[i].x ),
        cvRound( cornersA[i].y )
     );
     CvPoint p1 = cvPoint(
        cvRound( cornersB[i].x ),
        cvRound( cornersB[i].y )
     );
     cvLine( imgC, p0, p1, CV_RGB(255,0,0),2 );
  }
  cvNamedWindow("ImageA",0);
  cvNamedWindow("ImageB",0);
  cvNamedWindow("LKpyr_OpticalFlow",0);
  cvShowImage("ImageA",imgA);
  cvShowImage("ImageB",imgB);
  cvShowImage("LKpyr_OpticalFlow",imgC);
  cvWaitKey(0);
  return 0;
}

