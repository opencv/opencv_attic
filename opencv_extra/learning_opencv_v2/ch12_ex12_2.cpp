// Uses Calibration example from ch 11 as input to Example 12-2 at the bottom
//   Example 12-2.  Computing the fundamental matrix using RANSAC.

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
printf("\n\n"
" Calling convention:\n"
" ch12_ex12_2  board_w  board_h  number_of_boards  skip_frames\n"
"\n"
"   WHERE:\n"
"     board_w, board_h   -- are the number of corners along the row and columns respectively\n"
"     number_of_boards   -- are the number of chessboard views to collect before calibration\n"
"     skip_frames        -- are the number of frames to skip before trying to collect another\n"
"                           good chessboard.  This allows you time to move the chessboard.  \n"
"                           Move it to many different locations and angles so that calibration \n"
"                           space will be well covered. \n"
"\n"
"This is really the same as ch11_ex12_1 calibration with video except it uses the reults to also compute the\n"
"fundamental matrix\n\n"
" Hit ‘p’ to pause/unpause, ESC to quit\n"
"\n");
}
//
//

int n_boards = 0; //Will be set by input list
int board_dt = 90; //Wait 90 frames per chessboard view
int board_w;
int board_h;
int main(int argc, char* argv[]) {
  
  CvCapture* capture;// = cvCreateCameraCapture( 0 );
 // assert( capture );

  if(argc != 5){
    printf("\nERROR: Wrong number of input parameters");
    help();
    return -1;
  }
  help();
  board_w  = atoi(argv[1]);
  board_h  = atoi(argv[2]);
  n_boards = atoi(argv[3]);
  board_dt = atoi(argv[4]);
  
  int board_n  = board_w * board_h;
  CvSize board_sz = cvSize( board_w, board_h );
  capture = cvCreateCameraCapture( 0 );
  if(!capture) { printf("\nCouldn't open the camera\n"); help(); return -1;}

  cvNamedWindow( "Calibration" );
  cvNamedWindow( "Raw Video");
  //ALLOCATE STORAGE
  CvMat* image_points      = cvCreateMat(n_boards*board_n,2,CV_32FC1);
  CvMat* object_points     = cvCreateMat(n_boards*board_n,3,CV_32FC1);
  CvMat* point_counts      = cvCreateMat(n_boards,1,CV_32SC1);
  CvMat* intrinsic_matrix  = cvCreateMat(3,3,CV_32FC1);
  CvMat* distortion_coeffs = cvCreateMat(4,1,CV_32FC1);

  CvPoint2D32f* corners = new CvPoint2D32f[ board_n ];
  int corner_count;
  int successes = 0;
  int step, frame = 0;

  IplImage *image = cvQueryFrame( capture );
  IplImage *gray_image = cvCreateImage(cvGetSize(image),8,1);//subpixel
 
  // CAPTURE CORNER VIEWS LOOP UNTIL WE’VE GOT n_boards 
  // SUCCESSFUL CAPTURES (ALL CORNERS ON THE BOARD ARE FOUND)
  //
  help();
  while(successes < n_boards) {
    //Skip every board_dt frames to allow user to move chessboard
    if((frame++ % board_dt) == 0) {
       //Find chessboard corners:
       int found = cvFindChessboardCorners(
                image, board_sz, corners, &corner_count, 
                CV_CALIB_CB_ADAPTIVE_THRESH | CV_CALIB_CB_FILTER_QUADS
       );

       //Get Subpixel accuracy on those corners
       cvCvtColor(image, gray_image, CV_BGR2GRAY);
       cvFindCornerSubPix(gray_image, corners, corner_count, 
                  cvSize(11,11),cvSize(-1,-1), cvTermCriteria(    
                  CV_TERMCRIT_EPS+CV_TERMCRIT_ITER, 30, 0.1 ));

       //Draw it
       cvDrawChessboardCorners(image, board_sz, corners, 
                  corner_count, found);
 //      cvShowImage( "Calibration", image );
   
       // If we got a good board, add it to our data
       if( corner_count == board_n ) {
          cvShowImage( "Calibration", image ); //show in color if we did collect the image
          step = successes*board_n;
          for( int i=step, j=0; j<board_n; ++i,++j ) {
             CV_MAT_ELEM(*image_points, float,i,0) = corners[j].x;
             CV_MAT_ELEM(*image_points, float,i,1) = corners[j].y;
             CV_MAT_ELEM(*object_points,float,i,0) = j/board_w;
             CV_MAT_ELEM(*object_points,float,i,1) = j%board_w;
             CV_MAT_ELEM(*object_points,float,i,2) = 0.0f;
          }
          CV_MAT_ELEM(*point_counts, int,successes,0) = board_n;    
          successes++;
          printf("Collected our %d of %d needed chessboard images\n",successes,n_boards);
       }
       else
         cvShowImage( "Calibration", gray_image ); //Show Gray if we didn't collect the image
    } //end skip board_dt between chessboard capture

    //Handle pause/unpause and ESC
    int c = cvWaitKey(15);
    if(c == 'p'){  
       c = 0;
       while(c != 'p' && c != 27){
            c = cvWaitKey(250);
       }
     }
     if(c == 27)
        return 0;
    image = cvQueryFrame( capture ); //Get next image
    cvShowImage("Raw Video", image);
  } //END COLLECTION WHILE LOOP.
  cvDestroyWindow("Calibration");
  printf("\n\n*** CALLIBRATING THE CAMERA...");
  //ALLOCATE MATRICES ACCORDING TO HOW MANY CHESSBOARDS FOUND
  CvMat* object_points2  = cvCreateMat(successes*board_n,3,CV_32FC1);
  CvMat* image_points2   = cvCreateMat(successes*board_n,2,CV_32FC1);
  CvMat* point_counts2   = cvCreateMat(successes,1,CV_32SC1);
  //TRANSFER THE POINTS INTO THE CORRECT SIZE MATRICES
  for(int i = 0; i<successes*board_n; ++i){
      CV_MAT_ELEM( *image_points2, float, i, 0) = 
             CV_MAT_ELEM( *image_points, float, i, 0);
      CV_MAT_ELEM( *image_points2, float,i,1) =   
             CV_MAT_ELEM( *image_points, float, i, 1);
      CV_MAT_ELEM(*object_points2, float, i, 0) =  
             CV_MAT_ELEM( *object_points, float, i, 0) ;
      CV_MAT_ELEM( *object_points2, float, i, 1) = 
             CV_MAT_ELEM( *object_points, float, i, 1) ;
      CV_MAT_ELEM( *object_points2, float, i, 2) = 
             CV_MAT_ELEM( *object_points, float, i, 2) ;
  } 
  for(int i=0; i<successes; ++i){ //These are all the same number
    CV_MAT_ELEM( *point_counts2, int, i, 0) = 
             CV_MAT_ELEM( *point_counts, int, i, 0);
  }
  cvReleaseMat(&object_points);
  cvReleaseMat(&image_points);
  cvReleaseMat(&point_counts);

  // At this point we have all of the chessboard corners we need.
  // Initialize the intrinsic matrix such that the two focal
  // lengths have a ratio of 1.0
  //
  CV_MAT_ELEM( *intrinsic_matrix, float, 0, 0 ) = 1.0f;
  CV_MAT_ELEM( *intrinsic_matrix, float, 1, 1 ) = 1.0f;

  //CALIBRATE THE CAMERA!
  cvCalibrateCamera2(
      object_points2, image_points2,
      point_counts2,  cvGetSize( image ),
      intrinsic_matrix, distortion_coeffs,
      NULL, NULL,0  //CV_CALIB_FIX_ASPECT_RATIO
  );

  // SAVE THE INTRINSICS AND DISTORTIONS
  printf(" *** DONE!\n\nStoring Intrinsics.xml and Distortions.xml files\n\n");
  cvSave("Intrinsics.xml",intrinsic_matrix);
  cvSave("Distortion.xml",distortion_coeffs);

  // EXAMPLE OF LOADING THESE MATRICES BACK IN:
  CvMat *intrinsic = (CvMat*)cvLoad("Intrinsics.xml");
  CvMat *distortion = (CvMat*)cvLoad("Distortion.xml");



// ------------------------- chapter 12 example 2 below -------------------------
//
    int point_count = board_n; //CV_MAT_ELEM( *point_counts2, int, 1, 0);
    printf("point_count=%d\n",point_count);
    CvMat* points1;
    CvMat* points2;
    CvMat* status;
    CvMat* fundamental_matrix;

    points1 = cvCreateMat(1,point_count,CV_32FC2);
    points2 = cvCreateMat(1,point_count,CV_32FC2);
    status = cvCreateMat(1,point_count,CV_8UC1);
//CV_MAT_ELEM( *mat, float, 3, 2 );


// create pointers to 2D points in points1, points2, initialize it etc...

CvPoint2D32f* pt1 = (CvPoint2D32f*)points1->data.fl;
CvPoint2D32f* pt2 = (CvPoint2D32f*)points2->data.fl;

CvPoint2D32f* imgpt1 = (CvPoint2D32f*)image_points2->data.fl;
CvPoint2D32f* imgpt2 = (CvPoint2D32f*)(image_points2->data.fl + point_count);


for(int i =  0; i < point_count; i++ )
{
    pt1[i] = imgpt1[i];
    pt2[i] = imgpt2[i];
}

    fundamental_matrix = cvCreateMat(3,3,CV_32FC1);
   int fm_count = cvFindFundamentalMat( points1,points2,fundamental_matrix,
                                        CV_FM_RANSAC,1.0,0.99,status );
                                        
printf("DONE computing fundamental matrix:\n\n");
for(int r = 0; r<3; r++) 
{
	for(int c = 0; c < 3; c++)
	{
		printf("%+10f  ",CV_MAT_ELEM( *fundamental_matrix, float,  r, c));
	}
	printf("\n");
}                 
printf("\n");
                                        
// ------------------------- END chapter 12 example 2 above -------------------------

  // Build the undistort map which we will use for all 
  // subsequent frames.
  //
  IplImage* mapx = cvCreateImage( cvGetSize(image), IPL_DEPTH_32F, 1 );
  IplImage* mapy = cvCreateImage( cvGetSize(image), IPL_DEPTH_32F, 1 );
  cvInitUndistortMap(
    intrinsic,
    distortion,
    mapx,
    mapy
  );
  // Just run the camera to the screen, now showing the raw and
  // the undistorted image.
  //
  cvNamedWindow( "Undistort" );
  while(image) {
    IplImage *t = cvCloneImage(image);
    cvShowImage( "Raw Video", image ); // Show raw image
    cvRemap( t, image, mapx, mapy );     // Undistort image
    cvReleaseImage(&t);
    cvShowImage("Undistort", image);     // Show corrected image

    //Handle pause/unpause and ESC
    int c = cvWaitKey(15);
    if(c == 'p'){ 
       c = 0;
       while(c != 'p' && c != 27){
            c = cvWaitKey(250);
       }
    }
    if(c == 27)
        break;
    image = cvQueryFrame( capture );
  } 

  return 0;
}






