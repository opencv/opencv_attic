////////YUV CODEBOOK
// Gary Bradski, July 14, 2005


#include "cv_yuv_codebook.h"

//GLOBALS FOR ALL CAMERA MODELS

//For connected components:
int CVCONTOUR_APPROX_LEVEL = 2;   // Approx.threshold - the bigger it is, the simpler is the boundary
int CVCLOSE_ITR = 1;				// How many iterations of erosion and/or dialation there should be
//#define CVPERIMSCALE 4			// image (width+height)/PERIMSCALE.  If contour lenght < this, delete that contour

//For learning background

//Just some convienience macros
#define CV_CVX_WHITE	CV_RGB(0xff,0xff,0xff)
#define CV_CVX_BLACK	CV_RGB(0x00,0x00,0x00)


///////////////////////////////////////////////////////////////////////////////////
// int updateCodeBook(uchar *p, codeBook &c, unsigned cbBounds)
// Updates the codebook entry with a new data point
//
// p			Pointer to a YUV pixel
// c			Codebook for this pixel
// cbBounds		Learning bounds for codebook (Rule of thumb: 10)
// numChannels	Number of color channels we're learning
//
// NOTES:
//		cvBounds must be of size cvBounds[numChannels]
//
// RETURN
//	codebook index
int cvupdateCodeBook(uchar *p, codeBook &c, unsigned *cbBounds, int numChannels)
{

	if(c.numEntries == 0) c.t = 0;
	c.t += 1;		//Record learning event
	//SET HIGH AND LOW BOUNDS
	int n;
	unsigned int high[3],low[3];
	for(n=0; n<numChannels; n++)
	{
		high[n] = *(p+n)+*(cbBounds+n);
		if(high[n] > 255) high[n] = 255;
		low[n] = *(p+n)-*(cbBounds+n);
		if(low[n] < 0) low[n] = 0;
	}
	int matchChannel;
	//SEE IF THIS FITS AN EXISTING CODEWORD
	int i;
	for(i=0; i<c.numEntries; i++)
	{
		matchChannel = 0;
		for(n=0; n<numChannels; n++)
		{
			if((c.cb[i]->learnLow[n] <= *(p+n)) && (*(p+n) <= c.cb[i]->learnHigh[n])) //Found an entry for this channel
			{
				matchChannel++;
			}
		}
		if(matchChannel == numChannels) //If an entry was found over all channels
		{
			c.cb[i]->t_last_update = c.t;
			//adjust this codeword for the first channel
			for(n=0; n<numChannels; n++)
			{
				if(c.cb[i]->max[n] < *(p+n))
				{
					c.cb[i]->max[n] = *(p+n);
				}
				else if(c.cb[i]->min[n] > *(p+n))
				{
					c.cb[i]->min[n] = *(p+n);
				}
			}
			break;
		}
	}

	//OVERHEAD TO TRACK POTENTIAL STALE ENTRIES
	for(int s=0; s<c.numEntries; s++)
	{
		//This garbage is to track which codebook entries are going stale
		int negRun = c.t - c.cb[s]->t_last_update;
		if(c.cb[s]->stale < negRun) c.cb[s]->stale = negRun;
	}


	//ENTER A NEW CODE WORD IF NEEDED
	if(i == c.numEntries)  //No existing code word found, make a new one
	{
		code_element **foo = new code_element* [c.numEntries+1];
		for(int ii=0; ii<c.numEntries; ii++)
		{
			foo[ii] = c.cb[ii];
		}
		foo[c.numEntries] = new code_element;
		if(c.numEntries) delete [] c.cb;
		c.cb = foo;
		for(n=0; n<numChannels; n++)
		{
			c.cb[c.numEntries]->learnHigh[n] = high[n];
			c.cb[c.numEntries]->learnLow[n] = low[n];
			c.cb[c.numEntries]->max[n] = *(p+n);
			c.cb[c.numEntries]->min[n] = *(p+n);
		}
		c.cb[c.numEntries]->t_last_update = c.t;
		c.cb[c.numEntries]->stale = 0;
		c.numEntries += 1;
	}

	//SLOWLY ADJUST LEARNING BOUNDS
	for(n=0; n<numChannels; n++)
	{
		if(c.cb[i]->learnHigh[n] < high[n]) c.cb[i]->learnHigh[n] += 1;
		if(c.cb[i]->learnLow[n] > low[n]) c.cb[i]->learnLow[n] -= 1;
	}

	return(i);
}

///////////////////////////////////////////////////////////////////////////////////
// uchar cvbackgroundDiff(uchar *p, codeBook &c, int minMod, int maxMod)
// Given a pixel and a code book, determine if the pixel is covered by the codebook
//
// p		pixel pointer (YUV interleaved)
// c		codebook reference
// numChannels  Number of channels we are testing
// maxMod	Add this (possibly negative) number onto max level when code_element determining if new pixel is foreground
// minMod	Subract this (possible negative) number from min level code_element when determining if pixel is foreground
//
// NOTES:
// minMod and maxMod must have length numChannels, e.g. 3 channels => minMod[3], maxMod[3].
//
// Return
// 0 => background, 255 => foreground
uchar cvbackgroundDiff(uchar *p, codeBook &c, int numChannels, int *minMod, int *maxMod)
{
	int matchChannel;
	//SEE IF THIS FITS AN EXISTING CODEWORD
	int i;
	for(i=0; i<c.numEntries; i++)
	{
		matchChannel = 0;
		for(int n=0; n<numChannels; n++)
		{
			if((c.cb[i]->min[n] - minMod[n] <= *(p+n)) && (*(p+n) <= c.cb[i]->max[n] + maxMod[n]))
			{
				matchChannel++; //Found an entry for this channel
			}
			else
			{
				break;
			}
		}
		if(matchChannel == numChannels)
		{
			break; //Found an entry that matched all channels
		}
	}
	if(i >= c.numEntries) return(255);
	return(0);
}


//UTILITES/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
//int clearStaleEntries(codeBook &c)
// After you've learned for some period of time, periodically call this to clear out stale codebook entries
//
//c		Codebook to clean up
//
// Return
// number of entries cleared
int cvclearStaleEntries(codeBook &c)
{
	int staleThresh = c.t>>1;
	int *keep = new int [c.numEntries];
	int keepCnt = 0;
	//SEE WHICH CODEBOOK ENTRIES ARE TOO STALE
	for(int i=0; i<c.numEntries; i++)
	{
		if(c.cb[i]->stale > staleThresh)
			keep[i] = 0; //Mark for destruction
		else
		{
			keep[i] = 1; //Mark to keep
			keepCnt += 1;
		}
	}
	//KEEP ONLY THE GOOD
	c.t = 0;						//Full reset on stale tracking
	code_element **foo = new code_element* [keepCnt];
	int k=0;
	for(int ii=0; ii<c.numEntries; ii++)
	{
		if(keep[ii])
		{
			foo[k] = c.cb[ii];
			foo[k]->stale = 0;		//We have to refresh these entries for next clearStale
			foo[k]->t_last_update = 0;
			k++;
		}
	}
	//CLEAN UP
	delete [] keep;
	delete [] c.cb;
	c.cb = foo;
	int numCleared = c.numEntries - keepCnt;
	c.numEntries = keepCnt;
	return(numCleared);
}

/////////////////////////////////////////////////////////////////////////////////
//int countSegmentation(codeBook *c, IplImage *I)
//
//Count how many pixels are detected as foreground
// c	Codebook
// I	Image (yuv, 24 bits)
// numChannels  Number of channels we are testing
// maxMod	Add this (possibly negative) number onto max level when code_element determining if new pixel is foreground
// minMod	Subract this (possible negative) number from min level code_element when determining if pixel is foreground
//
// NOTES:
// minMod and maxMod must have length numChannels, e.g. 3 channels => minMod[3], maxMod[3].
//
//Return
// Count of fg pixels
//
int cvcountSegmentation(codeBook *c, IplImage *I, int numChannels, int *minMod, int *maxMod)
{
	int count = 0,i;
	uchar *pColor;
	int imageLen = I->width * I->height;

	//GET BASELINE NUMBER OF FG PIXELS FOR Iraw
	pColor = (uchar *)((I)->imageData);
	for(i=0; i<imageLen; i++)
	{
		if(cvbackgroundDiff(pColor, c[i], numChannels, minMod, maxMod))
			count++;
		pColor += 3;
	}
	return(count);
}


///////////////////////////////////////////////////////////////////////////////////////////
//void cvconnectedComponents(IplImage *mask, int poly1_hull0, float perimScale, int *num, CvRect *bbs, CvPoint *centers)
// This cleans up the forground segmentation mask derived from calls to cvbackgroundDiff
//
// mask			Is a grayscale (8 bit depth) "raw" mask image which will be cleaned up
//
// OPTIONAL PARAMETERS:
// poly1_hull0	If set, approximate connected component by (DEFAULT) polygon, or else convex hull (0)
// perimScale 	Len = image (width+height)/perimScale.  If contour len < this, delete that contour (DEFAULT: 4)
// num			Maximum number of rectangles and/or centers to return, on return, will contain number filled (DEFAULT: NULL)
// bbs			Pointer to bounding box rectangle vector of length num.  (DEFAULT SETTING: NULL)
// centers		Pointer to contour centers vectore of length num (DEFULT: NULL)
//
void cvconnectedComponents(IplImage *mask, int poly1_hull0, float perimScale, int *num, CvRect *bbs, CvPoint *centers)
{
static CvMemStorage*	mem_storage	= NULL;
static CvSeq*			contours	= NULL;
//CLEAN UP RAW MASK
	cvMorphologyEx( mask, mask, NULL, NULL, CV_MOP_OPEN, CVCLOSE_ITR );
	cvMorphologyEx( mask, mask, NULL, NULL, CV_MOP_CLOSE, CVCLOSE_ITR );

//FIND CONTOURS AROUND ONLY BIGGER REGIONS
	if( mem_storage==NULL ) mem_storage = cvCreateMemStorage(0);
    else cvClearMemStorage(mem_storage);

	CvContourScanner scanner = cvStartFindContours(mask,mem_storage,sizeof(CvContour),CV_RETR_EXTERNAL,CV_CHAIN_APPROX_SIMPLE);
	CvSeq* c;
	int numCont = 0;
	while( (c = cvFindNextContour( scanner )) != NULL )
	{
		double len = cvContourPerimeter( c );
		double q = (mask->height + mask->width) /perimScale;   //calculate perimeter len threshold
		if( len < q ) //Get rid of blob if it's perimeter is too small
		{
			cvSubstituteContour( scanner, NULL );
		}
		else //Smooth it's edges if it's large enough
		{
			CvSeq* c_new;
			if(poly1_hull0) //Polygonal approximation of the segmentation
	            c_new = cvApproxPoly(c,sizeof(CvContour),mem_storage,CV_POLY_APPROX_DP, CVCONTOUR_APPROX_LEVEL,0);
			else //Convex Hull of the segmentation
				c_new = cvConvexHull2(c,mem_storage,CV_CLOCKWISE,1);
            cvSubstituteContour( scanner, c_new );
			numCont++;
        }
	}
	contours = cvEndFindContours( &scanner );

// PAINT THE FOUND REGIONS BACK INTO THE IMAGE
	cvZero( mask );
	IplImage *maskTemp;
	//CALC CENTER OF MASS AND OR BOUNDING RECTANGLES
	if(num != NULL)
	{
		int N = *num, numFilled = 0, i=0;
		CvMoments moments;
		double M00, M01, M10;
		maskTemp = cvCloneImage(mask);
		for(i=0, c=contours; c != NULL; c = c->h_next,i++ )
		{
			if(i < N) //Only process up to *num of them
			{
				cvDrawContours(maskTemp,c,CV_CVX_WHITE, CV_CVX_WHITE,-1,CV_FILLED,8);
				//Find the center of each contour
				if(centers != NULL)
				{
					cvMoments(maskTemp,&moments,1);
					M00 = cvGetSpatialMoment(&moments,0,0);
					M10 = cvGetSpatialMoment(&moments,1,0);
					M01 = cvGetSpatialMoment(&moments,0,1);
					centers[i].x = (int)(M10/M00);
					centers[i].y = (int)(M01/M00);
				}
				//Bounding rectangles around blobs
				if(bbs != NULL)
				{
					bbs[i] = cvBoundingRect(c);
				}
				cvZero(maskTemp);
				numFilled++;
			}
			//Draw filled contours into mask
			cvDrawContours(mask,c,CV_CVX_WHITE,CV_CVX_WHITE,-1,CV_FILLED,8); //draw to central mask
		} //end looping over contours
		*num = numFilled;
		cvReleaseImage( &maskTemp);
	}
	//ELSE JUST DRAW PROCESSED CONTOURS INTO THE MASK
	else
	{
		for( c=contours; c != NULL; c = c->h_next )
		{
			cvDrawContours(mask,c,CV_CVX_WHITE, CV_CVX_BLACK,-1,CV_FILLED,8);
		}
	}
}

