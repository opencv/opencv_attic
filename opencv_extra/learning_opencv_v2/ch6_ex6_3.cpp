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

#include <cv.h>
#include <highgui.h>

int main(int argc, char** argv)
{
   CvPoint2D32f srcQuad[4], dstQuad[4];
   CvMat* warp_matrix = cvCreateMat(3,3,CV_32FC1);
   IplImage *src, *dst;
    if( argc == 2 && ((src=cvLoadImage(argv[1],1)) != 0 ))
    {
   dst = cvCloneImage(src);
   dst->origin = src->origin;
   cvZero(dst);

   srcQuad[0].x = 0;           //src Top left
   srcQuad[0].y = 0;
   srcQuad[1].x = src->width - 1;  //src Top right
   srcQuad[1].y = 0;
   srcQuad[2].x = 0;           //src Bottom left
   srcQuad[2].y = src->height - 1;
   srcQuad[3].x = src->width - 1;  //src Bot right
   srcQuad[3].y = src->height - 1;
      //- - - - - - - - - - - - - -//
   dstQuad[0].x = src->width*0.05;  //dst Top left
   dstQuad[0].y = src->height*0.33;
   dstQuad[1].x = src->width*0.9;  //dst Top right
   dstQuad[1].y = src->height*0.25;
   dstQuad[2].x = src->width*0.2;  //dst Bottom left
   dstQuad[2].y = src->height*0.7;      
   dstQuad[3].x = src->width*0.8;  //dst Bot right
   dstQuad[3].y = src->height*0.9;

   cvGetPerspectiveTransform(srcQuad,dstQuad,
                                     warp_matrix);
   cvWarpPerspective(src,dst,warp_matrix);
   cvNamedWindow( "Perspective_Warp", 1 );
      cvShowImage( "Perspective_Warp", dst );
      cvWaitKey();
    }
   cvReleaseImage(&dst);
   cvReleaseMat(&warp_matrix);
    return 0;
}
