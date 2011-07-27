//
// ch10_ex10_1b_Horn_Schunck   Optical flow by the Farneback method
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

#include "opencv2/opencv.hpp"
#include <math.h>
#include <iostream>

using namespace cv;
using namespace std;

int main(int argc, char** argv)
{
    // Initialize, load two images from the file system, and
    // allocate the images and other structures we will need for
    // results.

    Mat imgA = imread("OpticalFlow0.jpg",0);
    Mat imgB = imread("OpticalFlow1.jpg",0);
    // exit if no input images
    if(imgA.empty() || imgB.empty()) { printf("One of OpticalFlow0.jpg and/or OpticalFlow1.jpg didn't load\n"); return -1;}

    namedWindow( "OpticalFlow0" );
    namedWindow( "OpticalFlow1" );
    namedWindow( "Flow Results" );

    imshow( "OpticalFlow0",imgA );
    imshow( "OpticalFlow1",imgB );

    // Call the Farneback algorithm
    Mat flow;
    calcOpticalFlowFarneback(imgA, imgB, flow, 0.5, 4, 21, 5, 5, 1.2, 0);

    // Now make some image of what we are looking at:
    //
    Mat imgC;
    cvtColor(imgA, imgC, CV_GRAY2BGR);
    int step = 12;
    for( int y=0; y<imgC.rows; y += step ) {
        //const float* px = velx.ptr<float>(y);
        //const float* py = vely.ptr<float>(y);
        for( int x=0; x<imgC.cols; x += step ) {
            //float vx = px[x], vy = py[x];
            float vx = flow.at<Vec2f>(y,x)[0], vy = flow.at<Vec2f>(y,x)[1];
            //if( vx>1 && vy>1 )
            {
                circle(
                    imgC,
                    Point( x, y ),
                    2,
                    Scalar(0,255,0),
                    -1
                );
                line(
                    imgC,
                    Point( x, y ),
                    Point( cvRound(x+vx*2), cvRound(y+vy*2) ),
                    Scalar(0,255,0),
                    1,
                    0
                );
            }
        }
    }
    // show tracking
    imshow( "Flow Results",imgC );
    waitKey(0);
    return 0;
}
