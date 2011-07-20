//Example 12-2. Example of 2D line fitting.

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

#include "cv.h"
#include "highgui.h"
#include <math.h>


int main( int argc, char** argv )
{
    IplImage* img = cvCreateImage( cvSize( 500, 500 ), 8, 3 );
    CvRNG rng = cvRNG(-1);

    cvNamedWindow( "fitline", 1 );
        
    for(;;)
    {
        char key;
        int i, count = cvRandInt(&rng)%100 + 1, outliers = count/5;
        float a = cvRandReal(&rng)*200;
        float b = cvRandReal(&rng)*40;
        float angle = cvRandReal(&rng)*CV_PI;
        float cos_a = cos(angle), sin_a = sin(angle);
        CvPoint pt1, pt2;
        CvPoint* points = (CvPoint*)malloc( count * sizeof(points[0]));
        CvMat pointMat = cvMat( 1, count, CV_32SC2, points );
        float line[4];
        float d, t;

        b = MIN(a*0.3, b);

        // generate some points that are close to the line
        for( i = 0; i < count - outliers; i++ )
        {
            float x = (cvRandReal(&rng)*2-1)*a;
            float y = (cvRandReal(&rng)*2-1)*b;
            points[i].x = cvRound(x*cos_a - y*sin_a + img->width/2);
            points[i].y = cvRound(x*sin_a + y*cos_a + img->height/2);
        }

        // generate "completely off" points
        for( ; i < count; i++ )
        {
            points[i].x = cvRandInt(&rng) % img->width;
            points[i].y = cvRandInt(&rng) % img->height;
        }

        // find the optimal line
        cvFitLine( &pointMat, CV_DIST_L1, 1, 0.001, 0.001, line );
        cvZero( img );

        // draw the points
        for( i = 0; i < count; i++ )
            cvCircle( img, points[i], 2, i < count - outliers ? CV_RGB(255, 0, 0) :
                CV_RGB(255,255,0), CV_FILLED, CV_AA, 0 );

        // ... and the long enough line to cross the whole image
        d = sqrt((double)line[0]*line[0] + (double)line[1]*line[1]);
        line[0] /= d;
        line[1] /= d;
        t = (float)(img->width + img->height);
        pt1.x = cvRound(line[2] - line[0]*t);
        pt1.y = cvRound(line[3] - line[1]*t);
        pt2.x = cvRound(line[2] + line[0]*t);
        pt2.y = cvRound(line[3] + line[1]*t);
        cvLine( img, pt1, pt2, CV_RGB(0,255,0), 3, CV_AA, 0 );

        cvShowImage( "fitline", img );

        key = (char) cvWaitKey(0);
        if( key == 27 || key == 'q' || key == 'Q' ) // 'ESC'
            break;
        free( points );
    }
    
    cvDestroyWindow( "fitline" );
    return 0;
}
