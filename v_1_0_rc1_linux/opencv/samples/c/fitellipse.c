/********************************************************************************
*
*
*  This program is demonstration for ellipse fitting. Program finds 
*  contours and approximate it by ellipses.
*
*  Trackbar specify threshold parametr.
*
*  White lines is contours. Red lines is fitting ellipses.
*
*
*  Autor:  Denis Burenkov.
*
*
*
********************************************************************************/
#ifdef _CH_
#pragma package <opencv>
#endif

#ifndef _EiC
#include "cv.h"
#include "highgui.h"
#endif

int slider_pos = 70;

// Load the source image. HighGUI use.
IplImage *image02 = 0, *image03 = 0, *image04 = 0;

void process_image(int h);

int main( int argc, char** argv )
{
    const char* filename = argc == 2 ? argv[1] : (char*)"stuff.jpg";
    
    // load image and force it to be grayscale
    if( (image03 = cvLoadImage(filename, 0)) == 0 )
        return -1;

    // Create the destination images
    image02 = cvCloneImage( image03 );
    image04 = cvCloneImage( image03 );

    // Create windows.
    cvNamedWindow("Source", 1);
    cvNamedWindow("Result", 1);

    // Show the image.
    cvShowImage("Source", image03);

    // Create toolbars. HighGUI use.
    cvCreateTrackbar( "Threshold", "Result", &slider_pos, 255, process_image );

    process_image(0);

    // Wait for a key stroke; the same function arranges events processing                
    cvWaitKey(0);
    cvReleaseImage(&image02);
    cvReleaseImage(&image03);

    cvDestroyWindow("Source");
    cvDestroyWindow("Result");

    return 0;
}

// Define trackbar callback functon. This function find contours,
// draw it and approximate it by ellipses.
void process_image(int h)
{
    CvMemStorage* stor;
    CvSeq* cont;
    CvBox2D32f* box;
    CvPoint* PointArray;
    CvPoint2D32f* PointArray2D32f;
    
    // Create dynamic structure and sequence.
    stor = cvCreateMemStorage(0);
    cont = cvCreateSeq(CV_SEQ_ELTYPE_POINT, sizeof(CvSeq), sizeof(CvPoint) , stor);
    
    // Threshold the source image. This needful for cvFindContours().
    cvThreshold( image03, image02, slider_pos, 255, CV_THRESH_BINARY );
    
    // Find all contours.
    cvFindContours( image02, stor, &cont, sizeof(CvContour), 
                    CV_RETR_LIST, CV_CHAIN_APPROX_NONE, cvPoint(0,0));
    
    // Clear images. IPL use.
    cvZero(image02);
    cvZero(image04);
    
    // This cycle draw all contours and approximate it by ellipses.
    for(;cont;cont = cont->h_next)
    {   
        int i; // Indicator of cycle.
        int count = cont->total; // This is number point in contour
        CvPoint center;
        CvSize size;
        
        // Number point must be more than or equal to 6 (for cvFitEllipse_32f).        
        if( count < 6 )
            continue;
        
        // Alloc memory for contour point set.    
        PointArray = (CvPoint*)malloc( count*sizeof(CvPoint) );
        PointArray2D32f= (CvPoint2D32f*)malloc( count*sizeof(CvPoint2D32f) );
        
        // Alloc memory for ellipse data.
        box = (CvBox2D32f*)malloc(sizeof(CvBox2D32f));
        
        // Get contour point set.
        cvCvtSeqToArray(cont, PointArray, CV_WHOLE_SEQ);
        
        // Convert CvPoint set to CvBox2D32f set.
        for(i=0; i<count; i++)
        {
            PointArray2D32f[i].x = (float)PointArray[i].x;
            PointArray2D32f[i].y = (float)PointArray[i].y;
        }
        
        // Fits ellipse to current contour.
        cvFitEllipse(PointArray2D32f, count, box);
        
        // Draw current contour.
        cvDrawContours(image04,cont,CV_RGB(255,255,255),CV_RGB(255,255,255),0,1,8,cvPoint(0,0));
        
        // Convert ellipse data from float to integer representation.
        center.x = cvRound(box->center.x);
        center.y = cvRound(box->center.y);
        size.width = cvRound(box->size.width*0.5);
        size.height = cvRound(box->size.height*0.5);
        box->angle = -box->angle;
        
        // Draw ellipse.
        cvEllipse(image04, center, size,
                  box->angle, 0, 360,
                  CV_RGB(0,0,255), 1, CV_AA, 0);
        
        // Free memory.          
        free(PointArray);
        free(PointArray2D32f);
        free(box);
    }
    
    // Show image. HighGUI use.
    cvShowImage( "Result", image04 );
}

#ifdef _EiC
main(1,"fitellipse.c");
#endif
