//      ./adaptThresh 15 1 1 71 15 adrian.jpg
// Example 5-4. Threshold versus adaptive threshold
// Compare thresholding with adaptive thresholding
// CALL:
// ./adaptThreshold Threshold 1binary 1adaptivemean \
//                    blocksize offset filename
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
#include <math.h>
IplImage *Igray=0, *It = 0, *Iat;
int main( int argc, char** argv )
{
     if(argc != 7){return -1;          }
     //Command line
     double threshold = (double)atof(argv[1]);
     int threshold_type = atoi(argv[2]) ?
              CV_THRESH_BINARY : CV_THRESH_BINARY_INV;
     int adaptive_method = atoi(argv[3]) ?
              CV_ADAPTIVE_THRESH_MEAN_C : CV_ADAPTIVE_THRESH_GAUSSIAN_C;
     int block_size = atoi(argv[4]);
     double offset = (double)atof(argv[5]);
     //Read in gray image
     if((Igray = cvLoadImage( argv[6], CV_LOAD_IMAGE_GRAYSCALE)) == 0){
          return     -1;}
     // Create the grayscale output images
     It = cvCreateImage(cvSize(Igray->width,Igray->height),
                          IPL_DEPTH_8U, 1);
     Iat = cvCreateImage(cvSize(Igray->width,Igray->height),
                          IPL_DEPTH_8U, 1);
     //Threshold
     cvThreshold(Igray,It,threshold,255,threshold_type);
     cvAdaptiveThreshold(Igray, Iat, 255, adaptive_method,
                          threshold_type, block_size, offset);
     //PUT UP 2 WINDOWS
     cvNamedWindow("Raw",1);
     cvNamedWindow("Threshold",1);
     cvNamedWindow("Adaptive Threshold",1);
     //Show the results
     cvShowImage("Raw",Igray);
     cvShowImage("Threshold",It);
     cvShowImage("Adaptive Threshold",Iat);
     cvWaitKey(0);
     //Clean up
     cvReleaseImage(&Igray);
     cvReleaseImage(&It);
     cvReleaseImage(&Iat);
	  cvDestroyWindow("Raw");
	  cvDestroyWindow("Threshold");
	  cvDestroyWindow("Adaptive Threshold");
	  return(0);
}

