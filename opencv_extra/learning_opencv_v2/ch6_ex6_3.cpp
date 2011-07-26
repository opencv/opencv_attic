//
//Example 6-3. Code for perspective transformation
// Usage: warp <image>
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

#include <opencv2/opencv.hpp>
#include <iostream>

using namespace cv;
using namespace std;

int main(int argc, char** argv)
{
    if(argc != 2) { cout << "Usage: ch6_ex6_3 <imagename>\n" << endl; return -1; }
    
    Mat src = imread(argv[1],1);
    if( src.empty() ) { cout << "Can not load " << argv[1] << endl; return -1; } 
    
    Point2f srcQuad[] =
    {
        Point2f(0,0), //src Top left
        Point2f(src.cols-1, 0), // src Top right
        Point2f(src.cols-1, src.rows-1), // src Bottom right
        Point2f(0, src.rows-1)  // src Bottom left
    };
    
    Point2f dstQuad[] =
    {
        Point2f(src.cols*0.05f, src.rows*0.33f),
        Point2f(src.cols*0.9f, src.rows*0.25f),
        Point2f(src.cols*0.8f, src.rows*0.9f),
        Point2f(src.cols*0.2f, src.rows*0.7f)
    };
    
    // COMPUTE PERSPECTIVE MATRIX  
    Mat warp_mat = getPerspectiveTransform(srcQuad, dstQuad);
    Mat dst;
    warpPerspective(src, dst, warp_mat, src.size(), INTER_LINEAR, BORDER_CONSTANT, Scalar());
    for( int i = 0; i < 4; i++ )
        circle(dst, dstQuad[i], 5, Scalar(255, 0, 255), -1, CV_AA);
    
    imshow("Perspective Transform Test", dst);
    waitKey();
    return 0;
}

