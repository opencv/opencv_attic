//
// ch10_ex10_1b_Horn_Schunck   Optical flow by the Horn-Schunck method
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

#include "cv.h"
#include "highgui.h"
#include <math.h>
#include <stdio.h>
#define CVX_GRAY50 cvScalar(100)
#define CVX_WHITE  cvScalar(255)

int main(int argc, char** argv)
{
    // Initialize, load two images from the file system, and
    // allocate the images and other structures we will need for
    // results.

    // exit if no input images
	 IplImage *imgA = 0, *imgB = 0;
    imgA = cvLoadImage("OpticalFlow0.jpg",0);
    imgB = cvLoadImage("OpticalFlow1.jpg",0);
    if(!(imgA)||!(imgB)){ printf("One of OpticalFlow0.jpg and/or OpticalFlow1.jpg didn't load\n"); return -1;}

    IplImage* velx = cvCreateImage(cvGetSize(imgA),IPL_DEPTH_32F,1);
    IplImage* vely = cvCreateImage(cvGetSize(imgA),IPL_DEPTH_32F,1);

    IplImage* imgC = cvCreateImage(cvGetSize(imgA),IPL_DEPTH_8U,3);

    cvNamedWindow( "OpticalFlow0" );
    cvNamedWindow( "OpticalFlow1" );
    cvNamedWindow( "Flow Results" );

    cvShowImage( "OpticalFlow0",imgA );
    cvShowImage( "OpticalFlow1",imgB );

    // Call the actual Horn and Schunck algorithm
    //
    cvCalcOpticalFlowHS( 
        imgA, 
        imgB, 
        0,
        velx,
        vely,
        .10,
        cvTermCriteria( 
            CV_TERMCRIT_ITER | CV_TERMCRIT_EPS,
            imgA->width,
            1e-6
        )
    );

    // Now make some image of what we are looking at:
    //
    cvZero( imgC );
    int step = 4;
    for( int y=0; y<imgC->height; y += step ) {
        float* px = (float*) ( velx->imageData + y * velx->widthStep );
        float* py = (float*) ( vely->imageData + y * vely->widthStep );
        for( int x=0; x<imgC->width; x += step ) {
            if( px[x]>1 && py[x]>1 ) {
                cvCircle(
                    imgC,
                    cvPoint( x, y ),
                    2,
                    CVX_GRAY50,
                    -1
                );
                cvLine(
                    imgC,
                    cvPoint( x, y ),
                    cvPoint( x+px[x]/2, y+py[x]/2 ),
                    CV_RGB(255,0,0),
                    1,
                    0
                );
            }
        }
    }
    // show tracking
    cvShowImage( "Flow Results",imgC );
    
    cvWaitKey(0);

    // destroy windows
    cvDestroyWindow( "OpticalFlow0" );
    cvDestroyWindow( "OpticalFlow1" );
    cvDestroyWindow( "Flow Results" );
    // release memory
    cvReleaseImage( &imgA );
    cvReleaseImage( &imgB );
    cvReleaseImage( &imgC );

    return 0;
}
