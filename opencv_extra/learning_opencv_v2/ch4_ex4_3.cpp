/* ************ License: *******************************
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
* *****************************************************/
#include <cv.h>
#include <highgui.h>
#include <stdio.h>
int main(int argc, char** argv)
{
   //Adding something to open a video so that we can read its properties ...
  	IplImage *frame; //To hold movie images
   CvCapture* capture         = NULL;
   if((argc < 2 )|| !(capture = cvCreateFileCapture( argv[1] ))){
   	printf("Failed to open %s\n",argv[1]);
   	return -1;
   }
   //Read the properties
	double f = cvGetCaptureProperty(
		capture,
		CV_CAP_PROP_FOURCC
	);
	char* fourcc = (char*) (&f);
	printf("Properties of %s are:\n",argv[1]);
	printf("FORCC = %d | %d | %d | %d |\n",fourcc[0],fourcc[1],fourcc[2],fourcc[3]);
   cvReleaseCapture( &capture );
   return 0;
}
