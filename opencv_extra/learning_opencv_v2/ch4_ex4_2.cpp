// An example program in which the
// user can draw boxes on the screen.
//
#include <cv.h>
#include <highgui.h>
#include <stdio.h>
 /* License:
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
*/
//
// Using a trackbar to create a "switch" that the user can turn on and off.
// We make this value global so everyone can see it.
//
int g_switch_value = 1;
void switch_off_function() {printf("Pause\n");}; //YOU COULD DO SOMETHING WITH THESE FUNCTIONS TOO
void switch_on_function() {printf("Run\n");}; 

//
// This will be the callback that we give to the
// trackbar.
//
void switch_callback( int position ) {
  if( position == 0 ) {
    switch_off_function();
  } else {
    switch_on_function();
  }
}
 
 //OK, OK, I ADDED READING A MOVIE AND USING THE "BUTTON" TO STOP AND GO
int main( int argc, char* argv[] ) {
  	IplImage *frame; //To hold movie images
   CvCapture* g_capture         = NULL;
   if((argc < 2 )|| !(g_capture = cvCreateFileCapture( argv[1] ))){
   	printf("Failed to open %s\n",argv[1]);
   	return -1;
   }

  // Name the main window
  //
  cvNamedWindow( "Example4_2", 1 );
 
  // Create the trackbar.  We give it a name,
  // and tell it the name of the parent window.
  //
  cvCreateTrackbar(
    "Switch",
    "Example4_2",
    &g_switch_value,
    1,
    switch_callback
  );
 
  // This will just cause OpenCV to idle until 
  // someone hits the "Escape" key.
  //
  while( 1 ) {
    if(g_switch_value){
		 frame = cvQueryFrame( g_capture );
   	 if( !frame ) break;
    }
    cvShowImage( "Example4_2", frame);
    if(cvWaitKey(10)==27 ) break;
  }
  
  //CLEAN UP
  cvReleaseCapture( &g_capture );
  cvDestroyWindow( "Example4_2" );
  return(0);

}
