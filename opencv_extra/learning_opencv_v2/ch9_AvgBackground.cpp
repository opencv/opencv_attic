#include "ch9_AvgBackground.h"


//GLOBALS

IplImage *IavgF[NUM_CAMERAS],*IdiffF[NUM_CAMERAS], *IprevF[NUM_CAMERAS], *IhiF[NUM_CAMERAS], *IlowF[NUM_CAMERAS];
IplImage *Iscratch,*Iscratch2,*Igray1,*Igray2,*Igray3,*Imaskt;
IplImage *Ilow1[NUM_CAMERAS],*Ilow2[NUM_CAMERAS],*Ilow3[NUM_CAMERAS],*Ihi1[NUM_CAMERAS],*Ihi2[NUM_CAMERAS],*Ihi3[NUM_CAMERAS];

float Icount[NUM_CAMERAS];

void AllocateImages(IplImage *I)  //I is just a sample for allocation purposes
{
	for(int i = 0; i<NUM_CAMERAS; i++){
		IavgF[i] = cvCreateImage( cvGetSize(I), IPL_DEPTH_32F, 3 );
		IdiffF[i] = cvCreateImage( cvGetSize(I), IPL_DEPTH_32F, 3 );
		IprevF[i] = cvCreateImage( cvGetSize(I), IPL_DEPTH_32F, 3 );
		IhiF[i] = cvCreateImage( cvGetSize(I), IPL_DEPTH_32F, 3 );
		IlowF[i] = cvCreateImage(cvGetSize(I), IPL_DEPTH_32F, 3 );
		Ilow1[i] = cvCreateImage( cvGetSize(I), IPL_DEPTH_32F, 1 );
		Ilow2[i] = cvCreateImage( cvGetSize(I), IPL_DEPTH_32F, 1 );
		Ilow3[i] = cvCreateImage( cvGetSize(I), IPL_DEPTH_32F, 1 );
		Ihi1[i] = cvCreateImage( cvGetSize(I), IPL_DEPTH_32F, 1 );
		Ihi2[i] = cvCreateImage( cvGetSize(I), IPL_DEPTH_32F, 1 );
		Ihi3[i] = cvCreateImage( cvGetSize(I), IPL_DEPTH_32F, 1 );
		cvZero(IavgF[i]  );
		cvZero(IdiffF[i]  );
		cvZero(IprevF[i]  );
		cvZero(IhiF[i] );
		cvZero(IlowF[i]  );		
		Icount[i] = 0.00001; //Protect against divide by zero
	}
	Iscratch = cvCreateImage( cvGetSize(I), IPL_DEPTH_32F, 3 );
	Iscratch2 = cvCreateImage( cvGetSize(I), IPL_DEPTH_32F, 3 );
	Igray1 = cvCreateImage( cvGetSize(I), IPL_DEPTH_32F, 1 );
	Igray2 = cvCreateImage( cvGetSize(I), IPL_DEPTH_32F, 1 );
	Igray3 = cvCreateImage( cvGetSize(I), IPL_DEPTH_32F, 1 );
	Imaskt = cvCreateImage( cvGetSize(I), IPL_DEPTH_8U, 1 );

	cvZero(Iscratch);
	cvZero(Iscratch2 );
}

void DeallocateImages()
{
	for(int i=0; i<NUM_CAMERAS; i++){
		cvReleaseImage(&IavgF[i]);
		cvReleaseImage(&IdiffF[i] );
		cvReleaseImage(&IprevF[i] );
		cvReleaseImage(&IhiF[i] );
		cvReleaseImage(&IlowF[i] );
		cvReleaseImage(&Ilow1[i]  );
		cvReleaseImage(&Ilow2[i]  );
		cvReleaseImage(&Ilow3[i]  );
		cvReleaseImage(&Ihi1[i]   );
		cvReleaseImage(&Ihi2[i]   );
		cvReleaseImage(&Ihi3[i]  );
	}
	cvReleaseImage(&Iscratch);
	cvReleaseImage(&Iscratch2);

	cvReleaseImage(&Igray1  );
	cvReleaseImage(&Igray2 );
	cvReleaseImage(&Igray3 );

	cvReleaseImage(&Imaskt);
}

// Accumulate the background statistics for one more frame
// We accumulate the images, the image differences and the count of images for the 
//    the routine createModelsfromStats() to work on after we're done accumulating N frames.
// I		Background image, 3 channel, 8u
// number	Camera number
void accumulateBackground(IplImage *I, int number)
{
	static int first = 1;
	cvCvtScale(I,Iscratch,1,0); //To float;
	if (!first){
		cvAcc(Iscratch,IavgF[number]);
		cvAbsDiff(Iscratch,IprevF[number],Iscratch2);
		cvAcc(Iscratch2,IdiffF[number]);
		Icount[number] += 1.0;
	}
	first = 0;
	cvCopy(Iscratch,IprevF[number]);
}

// Scale the average difference from the average image high acceptance threshold
void scaleHigh(float scale, int num)
{
	cvConvertScale(IdiffF[num],Iscratch,scale); //Converts with rounding and saturation
	cvAdd(Iscratch,IavgF[num],IhiF[num]);
	cvCvtPixToPlane( IhiF[num], Ihi1[num],Ihi2[num],Ihi3[num], 0 );
}

// Scale the average difference from the average image low acceptance threshold
void scaleLow(float scale, int num)
{
	cvConvertScale(IdiffF[num],Iscratch,scale); //Converts with rounding and saturation
	cvSub(IavgF[num],Iscratch,IlowF[num]);
	cvCvtPixToPlane( IlowF[num], Ilow1[num],Ilow2[num],Ilow3[num], 0 );
}

//Once you've learned the background long enough, turn it into a background model
void createModelsfromStats()
{
	for(int i=0; i<NUM_CAMERAS; i++)
	{
		cvConvertScale(IavgF[i],IavgF[i],(double)(1.0/Icount[i]));
		cvConvertScale(IdiffF[i],IdiffF[i],(double)(1.0/Icount[i]));
		cvAddS(IdiffF[i],cvScalar(1.0,1.0,1.0),IdiffF[i]);  //Make sure diff is always something
		scaleHigh(HIGH_SCALE_NUM,i);
		scaleLow(LOW_SCALE_NUM,i);
	}
}

// Create a binary: 0,255 mask where 255 means forground pixel
// I		Input image, 3 channel, 8u
// Imask	mask image to be created, 1 channel 8u
// num		camera number.
//
void backgroundDiff(IplImage *I,IplImage *Imask, int num)  //Mask should be grayscale
{
	cvCvtScale(I,Iscratch,1,0); //To float;
	//Channel 1
	cvCvtPixToPlane( Iscratch, Igray1,Igray2,Igray3, 0 );
	cvInRange(Igray1,Ilow1[num],Ihi1[num],Imask);
	//Channel 2
	cvInRange(Igray2,Ilow2[num],Ihi2[num],Imaskt);
	cvOr(Imask,Imaskt,Imask);
	//Channel 3
	cvInRange(Igray3,Ilow3[num],Ihi3[num],Imaskt);
	cvOr(Imask,Imaskt,Imask);
	//Finally, invert the results
	cvSubRS( Imask, cvScalar(255), Imask);
}

//////////////////////////////////////////////////////////////////////////
/*
//Utility comparision function
gbCmp(IplImage *I1, IplImage *I2, IplImage *Imask, int op)
{
	int len = I1->width*I1->height;
	int x;
	float *fp1 = (float *)I1->imageData;
	float *fp2 = (float *)I2->imageData;
	char *cp = Imask->imageData;
	if(op == CV_CMP_GT)
	{
		for(x=0;x<len;x++)
		{
			if(*fp1++ > *fp2++)
				*cp++ = 255;
			else
				*cp++ = 0;
		}
	}
	else
	{
		for(x=0;x<len;x++)
		{
			if(*fp1++ < *fp2++)
				*cp++ = 255;
			else
				*cp++ = 0;
		}
	}
}


void backgroundDiff(IplImage *I,IplImage *Imask, int num)  //Mask should be grayscale
{
	cvCvtScale(I,Iscratch,1,0); //To float;
	cvCvtPixToPlane( Iscratch, Igray1,Igray2,Igray3, 0 );

	gbCmp(Igray1,Ihi1[num],Imask,CV_CMP_GT);
	gbCmp(Igray2,Ihi2[num],Imaskt,CV_CMP_GT);
	cvOr(Imask,Imaskt,Imask);
	gbCmp(Igray3,Ihi3[num],Imaskt,CV_CMP_GT);
	cvOr(Imask,Imaskt,Imask);

	gbCmp(Igray1,Ilow1[num],Imaskt,CV_CMP_LT);
	cvOr(Imask,Imaskt,Imask);
	gbCmp(Igray2,Ilow2[num],Imaskt,CV_CMP_LT);
	cvOr(Imask,Imaskt,Imask);
	gbCmp(Igray3,Ilow3[num],Imaskt,CV_CMP_LT);
	cvOr(Imask,Imaskt,Imask);
	//Some morphology
//	cvErode( Imask, Imask, NULL, 1);
//	cvMorphologyEx(Imask, Imask, NULL, NULL, CV_MOP_CLOSE, 1);
}
*/
