///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Accumulate average and ~std (really absolute difference) image and use this to detect background and foreground
//
// Typical way of using this is to:
// 	AllocateImages();
//	//loop for N images to accumulate background differences
//	accumulateBackground();
//	//When done, turn this into our avg and std model with high and low bounds
//	createModelsfromStats();
//	//Then use the function to return background in a mask (255 == foreground, 0 == background)
//	backgroundDiff(IplImage *I,IplImage *Imask, int num);
//	//Then tune the high and low difference from average image background acceptance thresholds
//	float scalehigh,scalelow; //Set these, defaults are 7 and 6. Note: scalelow is how many average differences below average
//	scaleHigh(scalehigh);
//	scaleLow(scalelow);
//	//That is, change the scale high and low bounds for what should be background to make it work.
//	//Then continue detecting foreground in the mask image
//	backgroundDiff(IplImage *I,IplImage *Imask, int num);
//
//NOTES: num is camera number which varies from 0 ... NUM_CAMERAS - 1.  Typically you only have one camera, but this routine allows
//     	 you to index many.
//
#ifndef AVGSEG_
#define AVGSEG_


#include "cv.h"				// define all of the opencv classes etc.
#include "highgui.h"
#include "cxcore.h"

//IMPORTANT DEFINES:
#define NUM_CAMERAS   1      		//This function can handle an array of cameras
#define HIGH_SCALE_NUM 7.0        	//How many average differences from average image on the high side == background
#define LOW_SCALE_NUM 6.0		//How many average differences from average image on the low side == background

void AllocateImages(IplImage *I);
void DeallocateImages();
void accumulateBackground(IplImage *I, int number=0);
void scaleHigh(float scale = HIGH_SCALE_NUM, int num = 0);
void scaleLow(float scale = LOW_SCALE_NUM, int num = 0);
void createModelsfromStats();
void backgroundDiff(IplImage *I,IplImage *Imask, int num = 0);

#endif

