#ifdef _CH_
#pragma package <opencv>
#endif

#ifndef _EiC
#include "cv.h"
#include "highgui.h"
#include <stdlib.h>
#include <stdio.h>
#endif

#define NUMBER 50
#define DELAY 10
char wndname[] = "Drawing Demo";

int main( int argc, char** argv )
{
    int i;
    CvPoint pt1,pt2;
    IplImage* image;
    double angle;
    CvSize sz;
    CvPoint  ptt[6];
    CvPoint* pt[2];
    int  arr[2];
    CvFont font;
    CvRandState rng;
    int width = 500, height = 500;
    int width3 = width*3, height3 = height*3;
    CvSize text_size;
    int ymin = 0;

    // Load the source image
    image = cvCreateImage( cvSize(width,height), 8, 3 );

    // Create a window
    cvNamedWindow(wndname, 1 );
    cvShowImage(wndname,image);
    cvRandInit( &rng, 0, 1, -1, CV_RAND_UNI );

    for (i = 0; i< NUMBER; i++)
    {
        pt1.x=cvRandNext(&rng) % width3 - width;
        pt1.y=cvRandNext(&rng) % height3 - height;
        pt2.x=cvRandNext(&rng) % width3 - width;
        pt2.y=cvRandNext(&rng) % height3 - height;

        cvLine( image,pt1, pt2, cvRandNext(&rng), cvRandNext(&rng)%20, 8 );
        cvShowImage(wndname,image);
        cvWaitKey(DELAY);
    }

    for (i = 0; i< NUMBER; i++)
    {
        pt1.x=cvRandNext(&rng) % width3 - width;
        pt1.y=cvRandNext(&rng) % height3 - height;
        pt2.x=cvRandNext(&rng) % width3 - width;
        pt2.y=cvRandNext(&rng) % height3 - height;

        cvRectangle( image,pt1, pt2, cvRandNext(&rng), cvRandNext(&rng)%20 );
        cvShowImage(wndname,image);
        cvWaitKey(DELAY);
    }

    for (i = 0; i< NUMBER; i++)
    {
        pt1.x=cvRandNext(&rng) % width3 - width;
        pt1.y=cvRandNext(&rng) % height3 - height;
        sz.width =cvRandNext(&rng)%200;
        sz.height=cvRandNext(&rng)%200;
        angle = (cvRandNext(&rng)%1000)*0.180;

        cvEllipse( image, pt1, sz, angle, angle - 100, angle + 200, cvRandNext(&rng), 6 );
        cvShowImage(wndname,image);
        cvWaitKey(DELAY);
    }

    pt[0] = &(ptt[0]);
    pt[1] = &(ptt[3]);

    arr[0] = 3;
    arr[1] = 3;

    for (i = 0; i< NUMBER; i++)
    {
        pt[0][0].x=cvRandNext(&rng) % width3 - width;
        pt[0][0].y=cvRandNext(&rng) % height3 - height;
        pt[0][1].x=cvRandNext(&rng) % width3 - width;
        pt[0][1].y=cvRandNext(&rng) % height3 - height;
        pt[0][2].x=cvRandNext(&rng) % width3 - width;
        pt[0][2].y=cvRandNext(&rng) % height3 - height;
        pt[1][0].x=cvRandNext(&rng) % width3 - width;
        pt[1][0].y=cvRandNext(&rng) % height3 - height;
        pt[1][1].x=cvRandNext(&rng) % width3 - width;
        pt[1][1].y=cvRandNext(&rng) % height3 - height;
        pt[1][2].x=cvRandNext(&rng) % width3 - width;
        pt[1][2].y=cvRandNext(&rng) % height3 - height;

        cvPolyLine( image, pt, arr, 2, 1, cvRandNext(&rng), 2, 8 );
        cvShowImage(wndname,image);
        cvWaitKey(DELAY);
    }

    for (i = 0; i< NUMBER; i++)
    {
        pt[0][0].x=cvRandNext(&rng) % width3 - width;
        pt[0][0].y=cvRandNext(&rng) % height3 - height;
        pt[0][1].x=cvRandNext(&rng) % width3 - width;
        pt[0][1].y=cvRandNext(&rng) % height3 - height;
        pt[0][2].x=cvRandNext(&rng) % width3 - width;
        pt[0][2].y=cvRandNext(&rng) % height3 - height;
        pt[1][0].x=cvRandNext(&rng) % width3 - width;
        pt[1][0].y=cvRandNext(&rng) % height3 - height;
        pt[1][1].x=cvRandNext(&rng) % width3 - width;
        pt[1][1].y=cvRandNext(&rng) % height3 - height;
        pt[1][2].x=cvRandNext(&rng) % width3 - width;
        pt[1][2].y=cvRandNext(&rng) % height3 - height;

        cvFillPoly( image, pt, arr, 2, cvRandNext(&rng) );
        cvShowImage(wndname,image);
        cvWaitKey(DELAY);
    }

    for (i = 0; i< NUMBER; i++)
    {
        pt1.x=cvRandNext(&rng) % width3 - width;
        pt1.y=cvRandNext(&rng) % height3 - height;

        cvCircle( image, pt1, cvRandNext(&rng)%300, cvRandNext(&rng), 1 );
        cvShowImage(wndname,image);
        cvWaitKey(DELAY);
    }

    for (i = 1; i< NUMBER; i++)
    {
        pt1.x=cvRandNext(&rng) % width3 - width;
        pt1.y=cvRandNext(&rng) % height3 - height;

        cvInitFont( &font, CV_FONT_VECTOR0,
                    (cvRandNext(&rng)%100)*0.05+0.1, (cvRandNext(&rng)%100)*0.05+0.1, 
                    (cvRandNext(&rng)%5)*0.1, cvRound(cvRandNext(&rng)%10) );

        cvPutText( image, "Testing text rendering!", pt1, &font, cvRandNext(&rng));
        cvShowImage(wndname,image);
        cvWaitKey(DELAY);
    }

    cvInitFont( &font, CV_FONT_VECTOR0, 1, 2, 0.0, 7 );

    cvGetTextSize( "OpenCV forever!", &font, &text_size, &ymin );

    pt1.x = (width - text_size.width)/2;
    pt1.y = (height + text_size.height)/2;

    for( i = 0; i < 255; i++ )
    {
        cvPutText( image, "OpenCV forever!", pt1, &font, CV_RGB(255,i,i));
        cvShowImage(wndname,image);
        cvWaitKey(DELAY);
    }

    // Wait for a key stroke; the same function arranges events processing
    cvWaitKey(0);
    cvReleaseImage(&image);
    cvDestroyWindow(wndname);

    return 0;
}

#ifdef _EiC
main(1,"drawing.c");
#endif
