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
#include <cv.h>
#include <highgui.h>
#define affine 
#ifdef affine
int main(int argc, char** argv)
{
   CvPoint2D32f srcTri[3], dstTri[3];
   CvMat* rot_mat = cvCreateMat(2,3,CV_32FC1);
   CvMat* warp_mat = cvCreateMat(2,3,CV_32FC1);
   IplImage *src, *dst;
    if( argc == 2 && ((src=cvLoadImage(argv[1],1)) != 0 ))
    {
   dst = cvCloneImage(src);
   dst->origin = src->origin;
   cvZero(dst);

   //COMPUTE WARP MATRIX
   srcTri[0].x = 0;          //src Top left
   srcTri[0].y = 0;
   srcTri[1].x = src->width - 1;    //src Top right
   srcTri[1].y = 0;
   srcTri[2].x = 0;          //src Bottom left
   srcTri[2].y = src->height - 1;
   //- - - - - - - - - - - - - - -//
   dstTri[0].x = src->width*0.0;    //dst Top left
   dstTri[0].y = src->height*0.33;
   dstTri[1].x = src->width*0.85; //dst Top right
   dstTri[1].y = src->height*0.25;
   dstTri[2].x = src->width*0.15; //dst Bottom left
   dstTri[2].y = src->height*0.7;
   cvGetAffineTransform(srcTri,dstTri,warp_mat);
   cvWarpAffine(src,dst,warp_mat);
   cvCopy(dst,src);

   //COMPUTE ROTATION MATRIX
   CvPoint2D32f center = cvPoint2D32f(src->width/2,
                                         src->height/2);
   double angle = -50.0;
   double scale = 0.6;
   cv2DRotationMatrix(center,angle,scale,rot_mat);
   cvWarpAffine(src,dst,rot_mat);

   //DO THE TRANSFORM:
   cvNamedWindow( "Affine_Transform", 1 );
      cvShowImage( "Affine_Transform", dst );
      cvWaitKey();
    }
   cvReleaseImage(&dst);
   cvReleaseMat(&rot_mat);
   cvReleaseMat(&warp_mat);
    return 0;
}
#endif
