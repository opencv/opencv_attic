// ch7_ex7_5_HistBackProj  OK, OK, this isn't in the book, its' "extra"
//     We cut the source code for actually doing back project in the book
//     but here it is no extra charge.
//   Gary Bradski Oct 3, 2008
//
#include <cv.h>
#include <cxcore.h>
#include <highgui.h>
#include <stdio.h>
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

void help(){
printf("\nCall is:\n" 
  "  ch7_ex7_5_HistBackProj modelImage testImage patch_type\n"
  "     patch_type takes on the following methods of matching:\n"
  "        0 Correlation, 1 ChiSqr, 2 Intersect, 3 Bhattacharyya\n"
  "     Projection is done using cvCalcBackProject()\n\n");
 }

//Learns histogram from derived from the first image, and backprojects it onto the second 
//  If patch_type is present, it does cvCalcBackProjectPatch()
//     patch_type takes on the following methods of matching:
//     0 Correlation, 1 ChiSqr, 2 Intersect, 3 Bhattacharyya
//  it does cvCalcBackProject().
// Call is: 
//    ch7BackProj modelImage testImage patch_type
// 
int main( int argc, char** argv ) {

    IplImage* src[2],*dst=0,*ftmp=0; //dst is what to display on
	int i,type = 0;
	int patch = 0; //default to cvCalcBackProject()
    if( argc >= 3){ 
		if(argc > 3) {
			patch = 1;
			type = atoi(argv[3]); //turn on cvCalcBackProjecPatch() using type
		}
		printf("Patch = %d, type = %d\n",patch,type);
		//Load 2 images, first on is to build histogram of, 2nd is to run on
		src[0] = 0; src[1] = 0;
		for(i = 1; i<3; ++i){
			if((src[i-1]=cvLoadImage(argv[i], 1))== 0) {
				printf("Error on reading image %d: %s\n",i,argv[i]);
				return(-1);
			}
		}
        // Compute the HSV image, and decompose it into separate planes.
        //
        IplImage *hsv[2], *h_plane[2],*s_plane[2],*v_plane[2],*planes[2][2]; 
       	IplImage* hist_img[2];
		CvHistogram* hist[2];
       // int h_bins = 30, s_bins = 32; 
	    int h_bins = 16, s_bins = 16;
        int    hist_size[] = { h_bins, s_bins };
        float  h_ranges[]  = { 0, 180 };          // hue is [0,180]
        float  s_ranges[]  = { 0, 255 }; 
        float* ranges[]    = { h_ranges, s_ranges };
		int scale = 10;
#define patchx 61
#define patchy 61
		if(patch){
			int iwidth = src[1]->width - patchx + 1;
			int iheight = src[1]->height - patchy + 1;
			ftmp = cvCreateImage( cvSize(iwidth,iheight),32,1);
			cvZero(ftmp);
		}
		dst = cvCreateImage( cvGetSize(src[1]),8,1);

		cvZero(dst);
 		for(i = 0; i<2; ++i){ 
			hsv[i] = cvCreateImage( cvGetSize(src[i]), 8, 3 ); 
        	cvCvtColor( src[i], hsv[i], CV_BGR2HSV );

			h_plane[i]  = cvCreateImage( cvGetSize(src[i]), 8, 1 );
        	s_plane[i]  = cvCreateImage( cvGetSize(src[i]), 8, 1 );
        	v_plane[i]  = cvCreateImage( cvGetSize(src[i]), 8, 1 );
        	planes[i][0] = h_plane[i];
			planes[i][1] = s_plane[i];
        	cvCvtPixToPlane( hsv[i], h_plane[i], s_plane[i], v_plane[i], 0 );
	        // Build the histogram and compute its contents.
    	    //
         	hist[i] = cvCreateHist( 
            	2, 
            	hist_size, 
            	CV_HIST_ARRAY, 
            	ranges, 
            	1 
          		); 
        	cvCalcHist( planes[i], hist[i], 0, 0 );
			if(patch)
				cvNormalizeHist( hist[i], 1.0 ); //Don't normalize for cvCalcBackProject(), 
			                                 
			// Create an image to use to visualize our histogram.
  	        //
         	hist_img[i] = cvCreateImage(  
          		cvSize( h_bins * scale, s_bins * scale ), 
          		8, 
          		3
        	); 
        	cvZero( hist_img[i] );

        	// populate our visualization with little gray squares.
        	//
        	float max_value = 0;
			float *fp,fval;
        	cvGetMinMaxHistValue( hist[i], 0, &max_value, 0, 0 );

	        for( int h = 0; h < h_bins; h++ ) {
    	        for( int s = 0; s < s_bins; s++ ) {
        	        float bin_val = cvQueryHistValue_2D( hist[i], h, s );
	               	int intensity = cvRound( bin_val * 255 / max_value );
                	cvRectangle( 
                  		hist_img[i], 
                  		cvPoint( h*scale, s*scale ),
                  		cvPoint( (h+1)*scale - 1, (s+1)*scale - 1),
                  		CV_RGB(intensity,intensity,intensity), 
                  		CV_FILLED
                	);
            	}
        	}
		}//For the 2 images

		//DO THE BACK PROJECTION
		if((patch)) {
			printf("Doing cvCalcBackProjectPatch() with type =%d\n",type);
			cvCalcBackProjectPatch(planes[1],ftmp,cvSize(61,61),hist[0],type,1.0);
			printf("ftmp count = %d\n",cvCountNonZero(ftmp));

		}else {
			printf("Doing cvCalcBackProject()\n");
			cvCalcBackProject(planes[1],dst,hist[0]);
 		}

        //DISPLAY
		cvNamedWindow( "Model Image", 0 );
        cvShowImage(   "Model Image", src[0] );
        cvNamedWindow( "Model H-S Histogram", 0 );
        cvShowImage(   "Model H-S Histogram", hist_img[0] );

		cvNamedWindow( "Test Image", 0 );
        cvShowImage(   "Test Image", src[1] );
        cvNamedWindow( "Test H-S Histogram", 0 );
        cvShowImage(   "Test H-S Histogram", hist_img[1] );

		cvNamedWindow( "Back Projection",0);
		cvShowImage(   "Back Projection", ftmp );
        cvWaitKey(0);
    }
	else { printf("Error: Wrong number of arguments\n"); help(); return -1;}
}
