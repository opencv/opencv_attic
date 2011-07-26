//
//Example 6-2. An affine transformation
// Usage: warp_affine <image>
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

// warp_affine <image>
#include <opencv2/opencv.hpp>
#include <iostream>

using namespace cv;
using namespace std;

int main(int argc, char** argv)
{
   if(argc != 2) { cout << "Usage: ch6_ex6_2 <imagename>\n" << endl; return -1; }
   
   Mat src = imread(argv[1],1);
   if( src.empty() ) { cout << "Can not load " << argv[1] << endl; return -1; } 
   
   Point2f srcTri[] =
   {
      Point2f(0,0), //src Top left
      Point2f(src.cols-1, 0), // src Top right
      Point2f(0, src.rows-1)  // src Bottom left
   };
   
   Point2f dstTri[] =
   {
      Point2f(src.cols*0.f,src.rows*0.33f), // dst Top left
      Point2f(src.cols*0.85f, src.rows*0.25f), // dst Top right
      Point2f(src.cols*0.15f, src.rows*0.7f)  // dst Bottom left
   }; 
   
   // COMPUTE AFFINE MATRIX  
   Mat warp_mat = getAffineTransform(srcTri, dstTri);
   Mat dst, dst2;
   warpAffine(src, dst, warp_mat, src.size(), INTER_LINEAR, BORDER_CONSTANT, Scalar());
   for( int i = 0; i < 3; i++ )
       circle(dst, dstTri[i], 5, Scalar(255, 0, 255), -1, CV_AA);
   
   imshow("Affine Transform Test", dst);
   waitKey();
   
   for(int frame=0;;frame++)
   {
       // COMPUTE ROTATION MATRIX
       Point2f center(src.cols*0.5f, src.rows*0.5f); 
       double angle = frame*3 % 360, scale = (cos((angle - 60)*CV_PI/180) + 1.05)*0.8;
       
       Mat rot_mat = getRotationMatrix2D(center, angle, scale);
       //float r = src.cols*0.3f;
       //rot_mat.at<double>(0,2) += r*cos(frame*5*CV_PI/180);
       //rot_mat.at<double>(1,2) += r*sin(frame*5*CV_PI/180);
       
       warpAffine(src, dst, rot_mat, src.size(), INTER_LINEAR, BORDER_CONSTANT, Scalar());
       imshow("Rotated Image", dst);
       if( waitKey(30) >= 0 )
           break;
   }
   return 0;
}
