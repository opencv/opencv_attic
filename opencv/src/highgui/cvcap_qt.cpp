/*M///////////////////////////////////////////////////////////////////////////////////////
//
//  IMPORTANT: READ BEFORE DOWNLOADING, COPYING, INSTALLING OR USING.
//
//  By downloading, copying, installing or using the software you agree to this license.
//  If you do not agree to this license, do not download, install,
//  copy or use the software.
//
//
//                        Intel License Agreement
//                For Open Source Computer Vision Library
//
// Copyright (C) 2000, Intel Corporation, all rights reserved.
// Third party copyrights are property of their respective owners.
//
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
//
//   * Redistribution's of source code must retain the above copyright notice,
//     this list of conditions and the following disclaimer.
//
//   * Redistribution's in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//
//   * The name of Intel Corporation may not be used to endorse or promote products
//     derived from this software without specific prior written permission.
//
// This software is provided by the copyright holders and contributors "as is" and
// any express or implied warranties, including, but not limited to, the implied
// warranties of merchantability and fitness for a particular purpose are disclaimed.
// In no event shall the Intel Corporation or contributors be liable for any direct,
// indirect, incidental, special, exemplary, or consequential damages
// (including, but not limited to, procurement of substitute goods or services;
// loss of use, data, or profits; or business interruption) however caused
// and on any theory of liability, whether in contract, strict liability,
// or tort (including negligence or otherwise) arising in any way out of
// the use of this software, even if advised of the possibility of such damage.
//
//M*/


#include "_highgui.h"
#include "cv.h"

// Original implementation by   Mark Asbach
//                              Institute of Communications Engineering
//                              RWTH Aachen University
//
// For implementation details and background see:
// http://developer.apple.com/samplecode/qtframestepper.win/listing1.html
//
// Please note that timing will only be correct for videos that contain a visual track
// that has full length (compared to other tracks)


// standard includes
#include <cstdio>
#include <cassert>

// Mac OS includes
#include <Carbon/Carbon.h>
#include <QuickTime/QuickTime.h>


typedef struct CvCapture_QT
{
	CvCaptureVTable * vtable; 

	Movie      myMovie;
	GWorldPtr  myGWorld;
	
	CvSize     size;
	TimeValue  movie_start_time;
	long       number_of_frames;
	long       next_frame_time;
	long       next_frame_number;
	
	IplImage * image_rgb; // will point to the PixMap of myGWorld
	IplImage * image_bgr; // will be returned by icvRetrieveFrame_QT()

} CvCapture_QT;


static       int         icvOpenFile_QT      (CvCapture_QT * capture, const char  * filename);
static       int         icvClose_QT         (CvCapture_QT * capture);
static       double      icvGetProperty_QT   (CvCapture_QT * capture, int property_id);
static       int         icvSetProperty_QT   (CvCapture_QT * capture, int property_id, double value);
static       int         icvGrabFrame_QT     (CvCapture_QT * capture);
static const void      * icvRetrieveFrame_QT (CvCapture_QT * capture);


static CvCaptureVTable capture_QT_vtable = 
{
    6,
    (CvCaptureCloseFunc)           icvClose_QT,
    (CvCaptureGrabFrameFunc)       icvGrabFrame_QT,
    (CvCaptureRetrieveFrameFunc)   icvRetrieveFrame_QT,
    (CvCaptureGetPropertyFunc)     icvGetProperty_QT,
    (CvCaptureSetPropertyFunc)     icvSetProperty_QT,
    (CvCaptureGetDescriptionFunc)  0
};


CvCapture* cvCaptureFromFile_QT( const char* filename )
{
	static int did_enter_movies = 0;
	if (! did_enter_movies)
	{
		EnterMovies();
		did_enter_movies = 1;
	}
	
    CvCapture_QT * capture = 0;
	
    if (filename)
    {
        capture = (CvCapture_QT *) cvAlloc (sizeof (*capture));
        memset (capture, 0, sizeof(*capture));
		
        capture->vtable = &capture_QT_vtable;
		
        if (!icvOpenFile_QT (capture, filename))
            cvReleaseCapture ((CvCapture**) & capture);
    }
	
    return (CvCapture*)capture;
}



/**
 * convert full path to CFStringRef and open corresponding Movie. Then
 * step over 'interesting frame times' to count total number of frames
 * for video material with varying frame durations and create offscreen
 * GWorld for rendering the movie frames.
 * 
 * @author Mark Asbach <asbach@ient.rwth-aachen.de>
 * @date   2005-11-04
 */
static int icvOpenFile_QT (CvCapture_QT * capture, const char * filename)
{
	Rect          myRect;
	short         myResID        = 0;
	Handle        myDataRef      = nil;
	OSType        myDataRefType  = 0;
	PixMapHandle  myPixMapHandle = nil;
	OSErr         myErr          = noErr;
	
	
	// no old errors please
	ClearMoviesStickyError ();
	
	// initialize pointers to zero
	capture->myMovie  = 0;
	capture->myGWorld = nil;
	
	// initialize numbers with invalid values
	capture->next_frame_time   = -1;
	capture->next_frame_number = -1;
	capture->number_of_frames  = -1;
	capture->movie_start_time  = -1;
	
	
	// we would use CFStringCreateWithFileSystemRepresentation (kCFAllocatorDefault, filename) on Mac OS X 10.4
	CFStringRef   inPath = CFStringCreateWithCString (kCFAllocatorDefault, filename, kCFStringEncodingISOLatin1);
//	OPENCV_ASSERT ((inPath == nil), "icvOpenFile_QT", "couldnt create CFString from a string");
	
	// create the data reference
	myErr = QTNewDataReferenceFromFullPathCFString (inPath, kQTPOSIXPathStyle, 0, & myDataRef, & myDataRefType);
	if (myErr != noErr)
	{
		fprintf (stderr, "Couldn't create QTNewDataReferenceFromFullPathCFString().\n");
		return 0;
	}
	
	// get the Movie
	myErr = NewMovieFromDataRef(& capture->myMovie, newMovieActive | newMovieAsyncOK /* | newMovieIdleImportOK */,
								& myResID, myDataRef, myDataRefType);
	
	// dispose of the data reference handle - we no longer need it
	DisposeHandle (myDataRef);
	
	// if NewMovieFromDataRef failed, we already disposed the DataRef, so just return with an error
	if (myErr != noErr)
	{
		fprintf (stderr, "Couldn't create a NewMovieFromDataRef() - error is %d.\n",  myErr);
		return 0;
	}
	
	// count the number of video 'frames' in the movie by stepping through all of the
	// video 'interesting times', or in other words, the places where the movie displays
	// a new video sample. The time between these interesting times is not necessarily constant.
	{
		OSType      whichMediaType = VisualMediaCharacteristic;
		TimeValue   theTime        = -1;
		
		// find out movie start time
		GetMovieNextInterestingTime (capture->myMovie, short (nextTimeMediaSample + nextTimeEdgeOK), 1, & whichMediaType, TimeValue (0), 0, & theTime, NULL);
		if (theTime == -1)
		{
			fprintf (stderr, "Couldn't inquire first frame time\n");
			return 0;
		}
		capture->movie_start_time  = theTime;
		capture->next_frame_time   = theTime;
		capture->next_frame_number = 0;
		
		// count all 'interesting times' of the movie
		capture->number_of_frames  = 0;
		while (theTime >= 0) 
		{
			GetMovieNextInterestingTime (capture->myMovie, short (nextTimeMediaSample), 1, & whichMediaType, theTime, 0, & theTime, NULL);
			capture->number_of_frames++;
		}
	}
	
	// get the bounding rectangle of the movie
	GetMoviesError ();
	GetMovieBox (capture->myMovie, & myRect);
	capture->size = cvSize (myRect.right - myRect.left, myRect.bottom - myRect.top);
	
	// create gworld for decompressed image
	myErr = QTNewGWorld (& capture->myGWorld, k24RGBPixelFormat /* k24BGRPixelFormat geht leider nicht */, 
	                     & myRect, nil, nil, 0);
	if (myErr != noErr)
	{
		fprintf (stderr, "Couldn't create QTNewGWorld() for output image.\n");
		return 0;
	}
	SetMovieGWorld (capture->myMovie, capture->myGWorld, nil);
	
	// build IplImage header that points to PixMap of the Movie's GWorld
	capture->image_rgb = cvCreateImageHeader (capture->size, IPL_DEPTH_8U, 3);
	myPixMapHandle = GetGWorldPixMap (capture->myGWorld);
	LockPixels (myPixMapHandle);
	cvSetData (capture->image_rgb, GetPixBaseAddr (myPixMapHandle), GetPixRowBytes (myPixMapHandle));
	
	// create IplImage that hold correctly formatted result
	capture->image_bgr = cvCreateImage (capture->size, IPL_DEPTH_8U, 3);
	
	// okay, that's it - should we wait until the Movie is playable?
	return 1;
}

/**
 * dispose of QuickTime Movie and free memory buffers
 * 
 * @author Mark Asbach <asbach@ient.rwth-aachen.de>
 * @date   2005-11-04
 */
static int icvClose_QT(CvCapture_QT * capture)
{
//	OPENCV_ASSERT (capture,          "icvClose_QT", "'capture' is a NULL-pointer");
//	OPENCV_ASSERT (capture->myMovie, "icvClose_QT", "invalid Movie handle");
	
	// deallocate and free resources
	cvReleaseImage       (& capture->image_bgr);
	cvReleaseImageHeader (& capture->image_rgb);
	DisposeGWorld        (capture->myGWorld);
	DisposeMovie         (capture->myMovie);
	
	// okay, that's it - should we wait until the Movie is playable?
	return 1;
}

/**
 * get a capture property
 * 
 * @author Mark Asbach <asbach@ient.rwth-aachen.de>
 * @date   2005-11-05
 */
static double icvGetProperty_QT (CvCapture_QT * capture, int property_id)
{
//	OPENCV_ASSERT (capture,                        "icvGetProperty_QT", "'capture' is a NULL-pointer");
//	OPENCV_ASSERT (capture->myMovie,               "icvGetProperty_QT", "invalid Movie handle");
//	OPENCV_ASSERT (capture->number_of_frames >  0, "icvGetProperty_QT", "movie has invalid number of frames");
//	OPENCV_ASSERT (capture->movie_start_time >= 0, "icvGetProperty_QT", "movie has invalid start time");
	
    // inquire desired property
    switch (property_id)
    {
		case CV_CAP_PROP_POS_FRAMES:
			return (capture->next_frame_number);
		
		case CV_CAP_PROP_POS_MSEC:
		case CV_CAP_PROP_POS_AVI_RATIO:
			{
				TimeValue   position  = capture->next_frame_time - capture->movie_start_time;
				
				if (property_id == CV_CAP_PROP_POS_MSEC)
				{
					TimeScale   timescale = GetMovieTimeScale (capture->myMovie);
					return (static_cast<double> (position) * 1000.0 / timescale);
				}
				else
				{
					TimeValue   duration  = GetMovieDuration  (capture->myMovie);
					return (static_cast<double> (position) / duration);
				}
			}
			break; // never reached
		
		case CV_CAP_PROP_FRAME_WIDTH:
			return static_cast<double> (capture->size.width);
		
		case CV_CAP_PROP_FRAME_HEIGHT:
			return static_cast<double> (capture->size.height);
		
		case CV_CAP_PROP_FPS:
			{
				TimeValue   duration  = GetMovieDuration  (capture->myMovie);
				TimeScale   timescale = GetMovieTimeScale (capture->myMovie);
				
				return (capture->number_of_frames / (static_cast<double> (duration) / timescale));
			}
		
		case CV_CAP_PROP_FRAME_COUNT:
			return static_cast<double> (capture->number_of_frames);
		
		case CV_CAP_PROP_FOURCC:  // not implemented
		case CV_CAP_PROP_FORMAT:  // not implemented
		case CV_CAP_PROP_MODE:    // not implemented
		default:
			// unhandled or unknown capture property
			OPENCV_ERROR (CV_StsBadArg, "icvSetProperty_QT", "unknown or unhandled property_id");
			return CV_StsBadArg;
    }
    
    return 0;
}

/**
 * set a capture property. With movie files, it is only possible to set the
 * position (i.e. jump to a given time or frame number)
 * 
 * @author Mark Asbach <asbach@ient.rwth-aachen.de>
 * @date   2005-11-05
 */
static int icvSetProperty_QT (CvCapture_QT * capture, int property_id, double value)
{
//	OPENCV_ASSERT (capture,                        "icvSetProperty_QT", "'capture' is a NULL-pointer");
//	OPENCV_ASSERT (capture->myMovie,               "icvSetProperty_QT", "invalid Movie handle");
//	OPENCV_ASSERT (capture->number_of_frames >  0, "icvSetProperty_QT", "movie has invalid number of frames");
//	OPENCV_ASSERT (capture->movie_start_time >= 0, "icvSetProperty_QT", "movie has invalid start time");
    
    // inquire desired property
	// 
	// rework these three points to really work through 'interesting times'.
	// with the current implementation, they result in wrong times or wrong frame numbers with content that 
	// features varying frame durations
    switch (property_id)
    {
		case CV_CAP_PROP_POS_MSEC:
		case CV_CAP_PROP_POS_AVI_RATIO:
			{
				TimeValue    destination;
				OSType       myType     = VisualMediaCharacteristic;
				OSErr        myErr      = noErr;

				if (property_id == CV_CAP_PROP_POS_MSEC)
				{
					TimeScale  timescale   = GetMovieTimeScale      (capture->myMovie);
					           destination = static_cast<TimeValue> (value / 1000.0 * timescale + capture->movie_start_time);
				}
				else
				{
					TimeValue  duration    = GetMovieDuration       (capture->myMovie);
					           destination = static_cast<TimeValue> (value * duration + capture->movie_start_time);
				}
				
				// really seek?
				if (capture->next_frame_time == destination)
					break;
				
				// seek into which direction?
				if (capture->next_frame_time < destination)
				{
					while (capture->next_frame_time < destination)
					{
						capture->next_frame_number++;
						GetMovieNextInterestingTime (capture->myMovie, nextTimeStep, 1, & myType, capture->next_frame_time,  
						                             1, & capture->next_frame_time, NULL);
						myErr = GetMoviesError();
						if (myErr != noErr)
						{
							fprintf (stderr, "Couldn't go on to GetMovieNextInterestingTime() in icvGrabFrame_QT.\n");
							return 0;
						}
					}
				}
				else
				{
					while (capture->next_frame_time > destination)
					{
						capture->next_frame_number--;
						GetMovieNextInterestingTime (capture->myMovie, nextTimeStep, 1, & myType, capture->next_frame_time, 
						                             -1, & capture->next_frame_time, NULL);
						myErr = GetMoviesError();
						if (myErr != noErr)
						{
							fprintf (stderr, "Couldn't go back to GetMovieNextInterestingTime() in icvGrabFrame_QT.\n");
							return 0;
						}
					}
				}
			}
			break;
		
		case CV_CAP_PROP_POS_FRAMES:
			{
				TimeValue    destination = static_cast<TimeValue> (value);
				short        direction   = (destination > capture->next_frame_number) ? 1 : -1;
				OSType       myType      = VisualMediaCharacteristic;
				OSErr        myErr       = noErr;
				
				while (destination != capture->next_frame_number)
				{
					capture->next_frame_number += direction;
					GetMovieNextInterestingTime (capture->myMovie, nextTimeStep, 1, & myType, capture->next_frame_time, 
												 direction, & capture->next_frame_time, NULL);
					myErr = GetMoviesError();
					if (myErr != noErr)
					{
						fprintf (stderr, "Couldn't step to desired frame number in icvGrabFrame_QT.\n");
						return 0;
					}
				}
			}
			break;
		
		default:
			// unhandled or unknown capture property
			OPENCV_ERROR (CV_StsBadArg, "icvSetProperty_QT", "unknown or unhandled property_id");
			return 0;
	}
	
	// positive result means success
	return 1;
}

/**
 * the original meaning of this method is to acquire raw frame data for the next video
 * frame but not decompress it. With the QuickTime video reader, this is reduced to
 * advance to the current frame time.
 * 
 * @author Mark Asbach <asbach@ient.rwth-aachen.de>
 * @date   2005-11-06
 */
static int icvGrabFrame_QT (CvCapture_QT * capture)
{
//	OPENCV_ASSERT (capture,          "icvGrabFrame_QT", "'capture' is a NULL-pointer");
//	OPENCV_ASSERT (capture->myMovie, "icvGrabFrame_QT", "invalid Movie handle");
	
	TimeValue    myCurrTime;
	OSType       myType     = VisualMediaCharacteristic;
	OSErr        myErr      = noErr;
	
	
	// jump to current video sample
	SetMovieTimeValue (capture->myMovie, capture->next_frame_time);
	myErr = GetMoviesError();
	if (myErr != noErr)
	{
		fprintf (stderr, "Couldn't SetMovieTimeValue() in icvGrabFrame_QT.\n");
		return  (myErr);
	}
	
	// where are we now?
	myCurrTime = GetMovieTime (capture->myMovie, NULL);
	
	// increment counters
	capture->next_frame_number++;
	GetMovieNextInterestingTime (capture->myMovie, nextTimeStep, 1, & myType, myCurrTime, 1, & capture->next_frame_time, NULL);
	myErr = GetMoviesError();
	if (myErr != noErr)
	{
		fprintf (stderr, "Couldn't GetMovieNextInterestingTime() in icvGrabFrame_QT.\n");
		return (myErr);
	}
	
	// that's it
    return 0;
}

/**
 * render the current frame into an image buffer and convert to OpenCV IplImage
 * buffer layout (BGR sampling)
 * 
 * @author Mark Asbach <asbach@ient.rwth-aachen.de>
 * @date   2005-11-06
 */
static const void * icvRetrieveFrame_QT (CvCapture_QT * capture)
{
//	OPENCV_ASSERT (capture,            "icvRetrieveFrame_QT", "'capture' is a NULL-pointer");
//	OPENCV_ASSERT (capture->myMovie,   "icvRetrieveFrame_QT", "invalid Movie handle");
//	OPENCV_ASSERT (capture->image_rgb, "icvRetrieveFrame_QT", "invalid source image");
//	OPENCV_ASSERT (capture->image_bgr, "icvRetrieveFrame_QT", "invalid destination image");
	
	OSErr        myErr      = noErr;
	
	
	// invalidates the movie's display state so that the Movie Toolbox 
	// redraws the movie the next time we call MoviesTask
	UpdateMovie (capture->myMovie);
	myErr = GetMoviesError ();
	if (myErr != noErr)
	{
		fprintf (stderr, "Couldn't UpdateMovie() in icvRetrieveFrame_QT().\n");
		return 0;
	}
	
	// service active movie (= redraw immediately)
	MoviesTask (capture->myMovie, 0L);
	myErr = GetMoviesError ();
	if (myErr != noErr)
	{
		fprintf (stderr, "MoviesTask() didn't succeed in icvRetrieveFrame_QT().\n");
		return 0;
	}
	
	// covert RGB of GWorld to BGR
	cvCvtColor (capture->image_rgb, capture->image_bgr, CV_RGB2BGR);
	
    // always return the same image pointer
	return capture->image_bgr;

}


