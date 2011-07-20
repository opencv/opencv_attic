//
// Example 9-1. Reading out the RGB values of all pixels in one row of a video and accumulating those
//              values into three separate files
//
// STORE TO DISK A LINE SEGMENT OF BGR PIXELS FROM pt1 to pt2.  
//…
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

#include <stdio.h>
#include <cv.h>
#include <highgui.h>

void help() {
printf("\nRead out RGB pixel values and store them to disk\nCall:\n"
"./ch9_ex9_1 avi_file\n"
"\n This will store to files blines.csv, glines.csv and rlines.csv\n\n");
}

int main( int argc, char** argv  )
{
    if(argc != 2) {help(); return -1;}
    cvNamedWindow( "Example9_1", CV_WINDOW_AUTOSIZE );
    CvCapture* capture = cvCreateFileCapture( argv[1] );
	if(!capture) {printf("\nCouldn't open %s\n",argv[1]); help(); return -1;}
    CvPoint pt1 = cvPoint(10,10);
    CvPoint pt2 = cvPoint(20,20);

    int max_buffer;
    IplImage *rawImage;
    int r[10000],g[10000],b[10000];
    FILE *fptrb = fopen("blines.csv","w"); //Store the data here
    FILE *fptrg = fopen("glines.csv","w"); // for each color channel
    FILE *fptrr = fopen("rlines.csv","w");
    CvLineIterator iterator;
    //MAIN PROCESSING LOOP:
    for(;;){
        if( !cvGrabFrame( capture ))
              break;
        rawImage = cvRetrieveFrame( capture );
        max_buffer = cvInitLineIterator(rawImage,pt1,pt2,&iterator,8,0);
        cvShowImage( "Example9_1", rawImage );
        int c = cvWaitKey(10);
        for(int j=0; j<max_buffer; j++){
            fprintf(fptrb,"%d,", iterator.ptr[0]); //Write blue value
            fprintf(fptrg,"%d,", iterator.ptr[1]); //green
            fprintf(fptrr,"%d,", iterator.ptr[2]); //red
            iterator.ptr[2] = 255;  //Mark this sample in red
            CV_NEXT_LINE_POINT(iterator); //Step to the next pixel
        }
        //OUTPUT THE DATA IN ROWS:
        fprintf(fptrb,"\n");fprintf(fptrg,"\n");fprintf(fptrr,"\n");
    }
    //CLEAN UP:
    printf("\nData stored to files: blines.csv, glines.csv and rlines.csv\n\n");
    fclose(fptrb); fclose(fptrg); fclose(fptrr);
    cvReleaseCapture( &capture );
    cvDestroyWindow( "Example9_1" );
}
