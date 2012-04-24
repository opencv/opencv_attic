#ifdef _CH_
#error "The file needs cvaux, which is not wrapped yet. Sorry"
#endif

#ifndef _EiC
#include "cv.h"
#include "cvaux.h"
#include "highgui.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include <float.h>
#include <limits.h>
#include <time.h>
#include <ctype.h>
#endif

#define ORIG_WIN_SIZE  24
static CvMemStorage* storage = 0;
static CvHidHaarClassifierCascade* hid_cascade = 0;

void detect_and_draw( IplImage* image );

int main( int argc, char** argv )
{
    CvCapture* capture = 0;

    CvHaarClassifierCascade* cascade =
    cvLoadHaarClassifierCascade( "<default_face_cascade>",
                         cvSize( ORIG_WIN_SIZE, ORIG_WIN_SIZE ));
    hid_cascade = cvCreateHidHaarClassifierCascade( cascade, 0, 0, 0, 1 );
    cvReleaseHaarClassifierCascade( &cascade );

    cvNamedWindow( "result", 1 );
    storage = cvCreateMemStorage(0);
    
    if( argc == 1 || (argc == 2 && strlen(argv[1]) == 1 && isdigit(argv[1][0])))
        capture = cvCaptureFromCAM( argc == 2 ? argv[1][0] - '0' : 0 );
    else if( argc == 2 )
        capture = cvCaptureFromAVI( argv[1] ); 

    if( capture )
    {
        for(;;)
        {
            IplImage *frame, *frame_copy;
            if( !cvGrabFrame( capture ))
                break;
            frame = cvRetrieveFrame( capture );
            if( !frame )
                break;

            frame_copy = cvCloneImage( frame );
            detect_and_draw( frame_copy );

            if( cvWaitKey( 10 ) >= 0 )
                break;
        }

        cvReleaseCapture( &capture );
    }
    else
    {
        char* filename = argc == 2 ? argv[1] : (char*)"lena.jpg";
        IplImage* image = cvLoadImage( filename, 1 );

        if( image )
        {
            cvFlip( image, image, 0 );
            image->origin = IPL_ORIGIN_BL;
            detect_and_draw( image );
            cvWaitKey(0);
            cvReleaseImage( &image );
        }
    }
    
    cvDestroyWindow("result");

    return 0;
}

void detect_and_draw( IplImage* img )
{
    int scale = 2;
    IplImage* temp = cvCreateImage( cvSize(img->width/2,img->height/2), 8, 3 );
    IplImage* canvas = cvCreateImage( cvSize(temp->width,temp->height*3/2), 8, 3 );
    CvPoint offset = cvPoint( 0, temp->height/3 );
    CvPoint pt1, pt2;
    int i;

    cvZero( canvas );
    cvPyrDown( img, temp, CV_GAUSSIAN_5x5 );
    cvFlip( temp, temp, 0 );
    cvClearMemStorage( storage );

    cvSetImageROI( canvas, cvRect( offset.x, offset.y, temp->width, temp->height ));
    cvCopy( temp, canvas, 0 );
    cvResetImageROI( canvas );
    
    if( hid_cascade )
    {
        CvSeq* faces = cvHaarDetectObjects( canvas, hid_cascade, storage,
                                            1.2, 2, CV_HAAR_DO_CANNY_PRUNING );
        for( i = 0; i < (faces ? faces->total : 0); i++ )
        {
            CvRect* r = (CvRect*)cvGetSeqElem( faces, i, 0 );
            r->x -= offset.x;
            r->y -= offset.y;
            pt1.x = r->x*scale;
            pt1.y = img->height - r->y*scale;
            pt2.x = (r->x+r->width)*scale;
            pt2.y = img->height - (r->y+r->height)*scale;
            cvRectangle( img, pt1, pt2, CV_RGB(255,0,0), 3 );
        }
    }

    cvShowImage( "result", img );
    cvReleaseImage( &temp );
    cvReleaseImage( &canvas );
}

#ifdef _EiC
main(1,"facedetect.c");
#endif
