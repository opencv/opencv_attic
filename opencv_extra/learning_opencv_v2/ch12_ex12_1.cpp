// Given a view of a checkerboard on a plane, view that image and a 
// list of others frontal parallel to that plane
//
// This presumes that you have previously callibrated your camera and stored
// an instrinics and distortion model for your camera.
//
// console application.
// Gary Bradski Oct 3, 2008
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
#include <stdio.h>
#include <stdlib.h>

void help(){
	printf("Birds eye view\n\n"
			"  birdseye board_w board_h intrinsics_mat.xml distortion_mat.xml checker_image \n\n"
			"Where: board_{w,h}    are the # of internal corners in the checkerboard\n"
			"       intrinsic      intrinsic path/name of matrix from prior calibration\n"
			"       distortion     distortion path/name of matrix from pror calibration\n"
			"       checker_image  is the path/name of image of checkerboard on the plane \n"
			"                      Frontal view of this will be shown.\n\n"
			" ADJUST VIEW HEIGHT using keys 'u' up, 'd' down. ESC to quit.\n\n");
}

int main(int argc, char* argv[]) {
	if(argc != 6){
		printf("\nERROR: too few parameters\n");
		help();
		return -1;
	}
	help();
	//INPUT PARAMETERS:
	int board_w = atoi(argv[1]);
	int board_h = atoi(argv[2]);
	int board_n  = board_w * board_h;
	CvSize board_sz = cvSize( board_w, board_h );
	CvMat *intrinsic = (CvMat*)cvLoad(argv[3]);
    CvMat *distortion = (CvMat*)cvLoad(argv[4]);
	IplImage *image = 0, *gray_image = 0;
	if((image = cvLoadImage(argv[5]))== 0){
		printf("Error: Couldn't load %s\n",argv[5]);
		return -1;
	}
	gray_image = cvCreateImage(cvGetSize(image),8,1);
    cvCvtColor(image, gray_image, CV_BGR2GRAY);

	//UNDISTORT OUR IMAGE
    IplImage* mapx = cvCreateImage( cvGetSize(image), IPL_DEPTH_32F, 1 );
    IplImage* mapy = cvCreateImage( cvGetSize(image), IPL_DEPTH_32F, 1 );
    cvInitUndistortMap(
      intrinsic,
      distortion,
      mapx,
      mapy
    );
	IplImage *t = cvCloneImage(image);
    cvRemap( t, image, mapx, mapy );

	//GET THE CHECKERBOARD ON THE PLANE
	cvNamedWindow("Checkers");
    CvPoint2D32f* corners = new CvPoint2D32f[ board_n ];
    int corner_count = 0;
    int found = cvFindChessboardCorners(
        image,
        board_sz,
        corners,
        &corner_count, 
        CV_CALIB_CB_ADAPTIVE_THRESH | CV_CALIB_CB_FILTER_QUADS
      );
	if(!found){
		printf("Couldn't aquire checkerboard on %s, only found %d of %d corners\n",
				argv[5],corner_count,board_n);
		return -1;
	}
	//Get Subpixel accuracy on those corners
	cvFindCornerSubPix(gray_image, corners, corner_count, 
			  cvSize(11,11),cvSize(-1,-1), 
			  cvTermCriteria( CV_TERMCRIT_EPS+CV_TERMCRIT_ITER, 30, 0.1 ));

	//GET THE IMAGE AND OBJECT POINTS:
	//Object points are at (r,c): (0,0), (board_w-1,0), (0,board_h-1), (board_w-1,board_h-1)
	//That means corners are at: corners[r*board_w + c]
	CvPoint2D32f objPts[4], imgPts[4];
	objPts[0].x = 0;         objPts[0].y = 0; 
	objPts[1].x = board_w-1; objPts[1].y = 0; 
	objPts[2].x = 0;         objPts[2].y = board_h-1;
	objPts[3].x = board_w-1; objPts[3].y = board_h-1; 
	imgPts[0] = corners[0];
	imgPts[1] = corners[board_w-1];
	imgPts[2] = corners[(board_h-1)*board_w];
	imgPts[3] = corners[(board_h-1)*board_w + board_w-1];

	//DRAW THE POINTS in order: B,G,R,YELLOW
	cvCircle(image,cvPointFrom32f(imgPts[0]),9,CV_RGB(0,0,255),3);
	cvCircle(image,cvPointFrom32f(imgPts[1]),9,CV_RGB(0,255,0),3);
	cvCircle(image,cvPointFrom32f(imgPts[2]),9,CV_RGB(255,0,0),3);
	cvCircle(image,cvPointFrom32f(imgPts[3]),9,CV_RGB(255,255,0),3);

	//DRAW THE FOUND CHECKERBOARD
	cvDrawChessboardCorners(image, board_sz, corners, corner_count, found);
    cvShowImage( "Checkers", image );

	//FIND THE HOMOGRAPHY
	CvMat *H = cvCreateMat( 3, 3, CV_32F);
	CvMat *H_invt = cvCreateMat(3,3,CV_32F);
	cvGetPerspectiveTransform(objPts,imgPts,H);

	//LET THE USER ADJUST THE Z HEIGHT OF THE VIEW
	float Z = 25;
	int key = 0;
	IplImage *birds_image = cvCloneImage(image);
	cvNamedWindow("Birds_Eye");
    while(key != 27) {//escape key stops
	   CV_MAT_ELEM(*H,float,2,2) = Z;
//	   cvInvert(H,H_invt); //If you want to invert the homography directly
//	   cvWarpPerspective(image,birds_image,H_invt,CV_INTER_LINEAR+CV_WARP_FILL_OUTLIERS );
	   //USE HOMOGRAPHY TO REMAP THE VIEW
	   cvWarpPerspective(image,birds_image,H,
			CV_INTER_LINEAR+CV_WARP_INVERSE_MAP+CV_WARP_FILL_OUTLIERS );
	   cvShowImage("Birds_Eye", birds_image);
	   key = cvWaitKey();
	   if(key == 'u') Z += 0.5;
	   if(key == 'd') Z -= 0.5;
	}

	//SHOW ROTATION AND TRANSLATION VECTORS
	CvMat* image_points  = cvCreateMat(4,1,CV_32FC2);
	CvMat* object_points = cvCreateMat(4,1,CV_32FC3);
	for(int i=0;i<4;++i){
		CV_MAT_ELEM(*image_points,CvPoint2D32f,i,0) = imgPts[i];
		CV_MAT_ELEM(*object_points,CvPoint3D32f,i,0) = cvPoint3D32f(objPts[i].x,objPts[i].y,0);
	}

	CvMat *RotRodrigues   = cvCreateMat(3,1,CV_32F);
	CvMat *Rot   = cvCreateMat(3,3,CV_32F);
	CvMat *Trans = cvCreateMat(3,1,CV_32F);
	cvFindExtrinsicCameraParams2(object_points,image_points,
			intrinsic,distortion,
			RotRodrigues,Trans);
	cvRodrigues2(RotRodrigues,Rot);

	//SAVE AND EXIT
	cvSave("Rot.xml",Rot);
	cvSave("Trans.xml",Trans);
	cvSave("H.xml",H);
	cvInvert(H,H_invt);
    cvSave("H_invt.xml",H_invt); //Bottom row of H invert is horizon line
	return 0;
}
