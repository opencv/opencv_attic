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


// Global state (did we call EnterMovies?)
static int did_enter_movies = 0;

// ----------------------------------------------------------------------------------------
#pragma mark Reading Video Files

/// Movie state structure for QuickTime movies
typedef struct CvCapture_QT_Movie
{
	CvCaptureVTable * vtable; 

	Movie      myMovie;            // movie handle
	GWorldPtr  myGWorld;           // we render into an offscreen GWorld
	
	CvSize     size;               // dimensions of the movie
	TimeValue  movie_start_time;   // movies can start at arbitrary times
	long       number_of_frames;   // duration in frames
	long       next_frame_time;
	long       next_frame_number; 
	
	IplImage * image_rgb;          // will point to the PixMap of myGWorld
	IplImage * image_bgr;          // will be returned by icvRetrieveFrame_QT()

} CvCapture_QT_Movie;



static       int         icvOpenFile_QT_Movie      (CvCapture_QT_Movie * capture, const char  * filename);
static       int         icvClose_QT_Movie         (CvCapture_QT_Movie * capture);
static       double      icvGetProperty_QT_Movie   (CvCapture_QT_Movie * capture, int property_id);
static       int         icvSetProperty_QT_Movie   (CvCapture_QT_Movie * capture, int property_id, double value);
static       int         icvGrabFrame_QT_Movie     (CvCapture_QT_Movie * capture);
static const void      * icvRetrieveFrame_QT_Movie (CvCapture_QT_Movie * capture);


static CvCaptureVTable capture_QT_Movie_vtable = 
{
    6,
    (CvCaptureCloseFunc)           icvClose_QT_Movie,
    (CvCaptureGrabFrameFunc)       icvGrabFrame_QT_Movie,
    (CvCaptureRetrieveFrameFunc)   icvRetrieveFrame_QT_Movie,
    (CvCaptureGetPropertyFunc)     icvGetProperty_QT_Movie,
    (CvCaptureSetPropertyFunc)     icvSetProperty_QT_Movie,
    (CvCaptureGetDescriptionFunc)  0
};


CvCapture * cvCaptureFromFile_QT (const char * filename)
{
	static int did_enter_movies = 0;
	if (! did_enter_movies)
	{
		EnterMovies();
		did_enter_movies = 1;
	}
	
    CvCapture_QT_Movie * capture = 0;
	
    if (filename)
    {
        capture = (CvCapture_QT_Movie *) cvAlloc (sizeof (*capture));
        memset (capture, 0, sizeof(*capture));
		
        capture->vtable = &capture_QT_Movie_vtable;
		
        if (!icvOpenFile_QT_Movie (capture, filename))
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
static int icvOpenFile_QT_Movie (CvCapture_QT_Movie * capture, const char * filename)
{
	Rect          myRect;
	short         myResID        = 0;
	Handle        myDataRef      = nil;
	OSType        myDataRefType  = 0;
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
	capture->size              = cvSize (-1,-1);
	
	
	// we would use CFStringCreateWithFileSystemRepresentation (kCFAllocatorDefault, filename) on Mac OS X 10.4
	CFStringRef   inPath = CFStringCreateWithCString (kCFAllocatorDefault, filename, kCFStringEncodingISOLatin1);
	OPENCV_ASSERT ((inPath != nil), "icvOpenFile_QT_Movie", "couldnt create CFString from a string");
	
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
		GetMovieNextInterestingTime (capture->myMovie, short (nextTimeMediaSample + nextTimeEdgeOK), 
		                             1, & whichMediaType, TimeValue (0), 0, & theTime, NULL);
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
			GetMovieNextInterestingTime (capture->myMovie, short (nextTimeMediaSample), 
			                             1, & whichMediaType, theTime, 0, & theTime, NULL);
			capture->number_of_frames++;
		}
	}
	
	// get the bounding rectangle of the movie
	GetMoviesError ();
	GetMovieBox (capture->myMovie, & myRect);
	capture->size = cvSize (myRect.right - myRect.left, myRect.bottom - myRect.top);
	
	// create gworld for decompressed image
	myErr = QTNewGWorld (& capture->myGWorld, k32ARGBPixelFormat /* k24BGRPixelFormat geht leider nicht */, 
	                     & myRect, nil, nil, 0);
	OPENCV_ASSERT (myErr == noErr, "icvOpenFile_QT_Movie", "couldnt create QTNewGWorld() for output image");
	SetMovieGWorld (capture->myMovie, capture->myGWorld, nil);
	
	// build IplImage header that will point to the PixMap of the Movie's GWorld later on
	capture->image_rgb = cvCreateImageHeader (capture->size, IPL_DEPTH_8U, 4);
	
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
static int icvClose_QT_Movie (CvCapture_QT_Movie * capture)
{
	OPENCV_ASSERT (capture,          "icvClose_QT_Movie", "'capture' is a NULL-pointer");
	OPENCV_ASSERT (capture->myMovie, "icvClose_QT_Movie", "invalid Movie handle");
	
	// deallocate and free resources
	cvReleaseImage       (& capture->image_bgr);
	cvReleaseImageHeader (& capture->image_rgb);
	DisposeGWorld        (capture->myGWorld);
	DisposeMovie         (capture->myMovie);
	
	// okay, that's it
	return 1;
}

/**
 * get a capture property
 * 
 * @author Mark Asbach <asbach@ient.rwth-aachen.de>
 * @date   2005-11-05
 */
static double icvGetProperty_QT_Movie (CvCapture_QT_Movie * capture, int property_id)
{
	OPENCV_ASSERT (capture,                        "icvGetProperty_QT_Movie", "'capture' is a NULL-pointer");
	OPENCV_ASSERT (capture->myMovie,               "icvGetProperty_QT_Movie", "invalid Movie handle");
	OPENCV_ASSERT (capture->number_of_frames >  0, "icvGetProperty_QT_Movie", "movie has invalid number of frames");
	OPENCV_ASSERT (capture->movie_start_time >= 0, "icvGetProperty_QT_Movie", "movie has invalid start time");
	
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
			OPENCV_ERROR (CV_StsBadArg, "icvSetProperty_QT_Movie", "unknown or unhandled property_id");
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
static int icvSetProperty_QT_Movie (CvCapture_QT_Movie * capture, int property_id, double value)
{
	OPENCV_ASSERT (capture,                        "icvSetProperty_QT_Movie", "'capture' is a NULL-pointer");
	OPENCV_ASSERT (capture->myMovie,               "icvSetProperty_QT_Movie", "invalid Movie handle");
	OPENCV_ASSERT (capture->number_of_frames >  0, "icvSetProperty_QT_Movie", "movie has invalid number of frames");
	OPENCV_ASSERT (capture->movie_start_time >= 0, "icvSetProperty_QT_Movie", "movie has invalid start time");
    
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
			OPENCV_ERROR (CV_StsBadArg, "icvSetProperty_QT_Movie", "unknown or unhandled property_id");
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
static int icvGrabFrame_QT_Movie (CvCapture_QT_Movie * capture)
{
	OPENCV_ASSERT (capture,          "icvGrabFrame_QT_Movie", "'capture' is a NULL-pointer");
	OPENCV_ASSERT (capture->myMovie, "icvGrabFrame_QT_Movie", "invalid Movie handle");
	
	TimeValue    myCurrTime;
	OSType       myType     = VisualMediaCharacteristic;
	OSErr        myErr      = noErr;
	
	
	// jump to current video sample
	SetMovieTimeValue (capture->myMovie, capture->next_frame_time);
	myErr = GetMoviesError();
	if (myErr != noErr)
	{
		fprintf (stderr, "Couldn't SetMovieTimeValue() in icvGrabFrame_QT_Movie.\n");
		return  0;
	}
	
	// where are we now?
	myCurrTime = GetMovieTime (capture->myMovie, NULL);
	
	// increment counters
	capture->next_frame_number++;
	GetMovieNextInterestingTime (capture->myMovie, nextTimeStep, 1, & myType, myCurrTime, 1, & capture->next_frame_time, NULL);
	myErr = GetMoviesError();
	if (myErr != noErr)
	{
		fprintf (stderr, "Couldn't GetMovieNextInterestingTime() in icvGrabFrame_QT_Movie.\n");
		return 0;
	}
	
	// that's it
    return 1;
}

/**
 * render the current frame into an image buffer and convert to OpenCV IplImage
 * buffer layout (BGR sampling)
 * 
 * @author Mark Asbach <asbach@ient.rwth-aachen.de>
 * @date   2005-11-06
 */
static const void * icvRetrieveFrame_QT_Movie (CvCapture_QT_Movie * capture)
{
	OPENCV_ASSERT (capture,            "icvRetrieveFrame_QT_Movie", "'capture' is a NULL-pointer");
	OPENCV_ASSERT (capture->myMovie,   "icvRetrieveFrame_QT_Movie", "invalid Movie handle");
	OPENCV_ASSERT (capture->image_rgb, "icvRetrieveFrame_QT_Movie", "invalid source image");
	OPENCV_ASSERT (capture->image_bgr, "icvRetrieveFrame_QT_Movie", "invalid destination image");
	
	PixMapHandle  myPixMapHandle = nil;
	OSErr         myErr          = noErr;
	
	
	// invalidates the movie's display state so that the Movie Toolbox 
	// redraws the movie the next time we call MoviesTask
	UpdateMovie (capture->myMovie);
	myErr = GetMoviesError ();
	if (myErr != noErr)
	{
		fprintf (stderr, "Couldn't UpdateMovie() in icvRetrieveFrame_QT_Movie().\n");
		return 0;
	}
	
	// service active movie (= redraw immediately)
	MoviesTask (capture->myMovie, 0L);
	myErr = GetMoviesError ();
	if (myErr != noErr)
	{
		fprintf (stderr, "MoviesTask() didn't succeed in icvRetrieveFrame_QT_Movie().\n");
		return 0;
	}
	
	// update IplImage header that points to PixMap of the Movie's GWorld.
	// unfortunately, cvCvtColor doesn't know ARGB, the QuickTime pixel format,
	// so we pass a modfied address.
	// ATTENTION: don't access the last pixel's alpha entry, it's inexistant
	myPixMapHandle = GetGWorldPixMap (capture->myGWorld);
	LockPixels (myPixMapHandle);
	cvSetData (capture->image_rgb, GetPixBaseAddr (myPixMapHandle) + 1, GetPixRowBytes (myPixMapHandle));
	
	// covert RGB of GWorld to BGR
	cvCvtColor (capture->image_rgb, capture->image_bgr, CV_RGBA2BGR);
	
	// allow QuickTime to access the buffer again
	UnlockPixels (myPixMapHandle);
	
    // always return the same image pointer
	return capture->image_bgr;
}


// ----------------------------------------------------------------------------------------
#pragma mark -
#pragma mark Capturing from Video Cameras


#ifdef USE_VDIG_VERSION

	/// SequenceGrabber state structure for QuickTime
	typedef struct CvCapture_QT_Cam_vdig
	{
		CvCaptureVTable  * vtable; 

		ComponentInstance  grabber;
		short              channel;
		GWorldPtr          myGWorld;
		PixMapHandle       pixmap;
		
		CvSize             size;
		long               number_of_frames;
		
		IplImage         * image_rgb; // will point to the PixMap of myGWorld
		IplImage         * image_bgr; // will be returned by icvRetrieveFrame_QT()

	} CvCapture_QT_Cam;

#else

	typedef struct CvCapture_QT_Cam_barg
	{
		CvCaptureVTable  * vtable; 

		SeqGrabComponent   grabber;
		SGChannel          channel;
		GWorldPtr          gworld;
		Rect               bounds;
		ImageSequence      sequence;

		volatile bool      got_frame;

		CvSize             size;
		IplImage         * image_rgb; // will point to the PixMap of myGWorld
		IplImage         * image_bgr; // will be returned by icvRetrieveFrame_QT()

	} CvCapture_QT_Cam;

#endif

static       int         icvOpenCamera_QT        (CvCapture_QT_Cam * capture, const int index);
static       int         icvClose_QT_Cam         (CvCapture_QT_Cam * capture);
static       double      icvGetProperty_QT_Cam   (CvCapture_QT_Cam * capture, int property_id);
static       int         icvSetProperty_QT_Cam   (CvCapture_QT_Cam * capture, int property_id, double value);
static       int         icvGrabFrame_QT_Cam     (CvCapture_QT_Cam * capture);
static const void      * icvRetrieveFrame_QT_Cam (CvCapture_QT_Cam * capture);


static CvCaptureVTable capture_QT_Cam_vtable = 
{
    6,
    (CvCaptureCloseFunc)           icvClose_QT_Cam,
    (CvCaptureGrabFrameFunc)       icvGrabFrame_QT_Cam,
    (CvCaptureRetrieveFrameFunc)   icvRetrieveFrame_QT_Cam,
    (CvCaptureGetPropertyFunc)     icvGetProperty_QT_Cam,
    (CvCaptureSetPropertyFunc)     icvSetProperty_QT_Cam,
    (CvCaptureGetDescriptionFunc)  0
};


/**
 * Initialize memory structure and call method to open camera
 *
 * @author Mark Asbach <asbach@ient.rwth-aachen.de>
 * @date 2006-01-29
 */
CvCapture * cvCaptureFromCAM_QT (const int index)
{
	if (! did_enter_movies)
	{
		EnterMovies();
		did_enter_movies = 1;
	}
	
    CvCapture_QT_Cam * capture = 0;
	
    if (index >= 0)
    {
        capture = (CvCapture_QT_Cam *) cvAlloc (sizeof (*capture));
        memset (capture, 0, sizeof(*capture));
		
        capture->vtable = &capture_QT_Cam_vtable;
	
        if (!icvOpenCamera_QT (capture, index))
            cvReleaseCapture ((CvCapture**) & capture);
    }
	
    return (CvCapture *) capture;
}

/// capture properties currently unimplemented for QuickTime camera interface
static double icvGetProperty_QT_Cam (CvCapture_QT_Cam * capture, int property_id)
{
	assert (0);
	return 0;
}

/// capture properties currently unimplemented for QuickTime camera interface
static int icvSetProperty_QT_Cam (CvCapture_QT_Cam * capture, int property_id, double value)
{
	assert (0);
	return 0;
}

#ifdef USE_VDIG_VERSION
#pragma mark Capturing using VDIG

/**
 * Open a quicktime video grabber component. This could be an attached
 * IEEE1394 camera, a web cam, an iSight or digitizer card / video converter.
 * 
 * @author Mark Asbach <asbach@ient.rwth-aachen.de>
 * @date 2006-01-29
 */
static int icvOpenCamera_QT (CvCapture_QT_Cam * capture, const int index)
{
	OPENCV_ASSERT (capture,            "icvOpenCamera_QT", "'capture' is a NULL-pointer");
	OPENCV_ASSERT (capture->index >=0, "icvOpenCamera_QT", "camera index is negative");

	ComponentDescription	component_description;
	Component				component = 0;
	int                     number_of_inputs = 0;
	Rect                    myRect;
	ComponentResult			result = noErr;
	

	// travers all components and count video digitizer channels
	component_description.componentType         = videoDigitizerComponentType;
	component_description.componentSubType      = 0L;
	component_description.componentManufacturer = 0L;
	component_description.componentFlags        = 0L;
	component_description.componentFlagsMask    = 0L;
	do
	{
		// traverse component list
		component = FindNextComponent (component, & component_description);		
		
		// found a component?
		if (component)
		{
			// dump component name
			#ifndef NDEBUG
				ComponentDescription  desc;
				Handle                nameHandle = NewHandleClear (200);  
				char                  nameBuffer [255];

				result = GetComponentInfo (component, & desc, nameHandle, nil, nil);
				OPENCV_ASSERT (result == noErr, "icvOpenCamera_QT", "couldnt GetComponentInfo()");
				OPENCV_ASSERT (*nameHandle, "icvOpenCamera_QT", "No name returned by GetComponentInfo()");
				snprintf (nameBuffer, (**nameHandle) + 1, "%s", (char *) (* nameHandle + 1));
				printf ("- Videodevice: %s\n", nameBuffer);
				DisposeHandle (nameHandle);
			#endif
			
			// open component to count number of inputs
			capture->grabber = OpenComponent (component);
			if (capture->grabber)
			{
				result = VDGetNumberOfInputs (capture->grabber, & capture->channel);
				if (result != noErr)
					fprintf (stderr, "Couldnt GetNumberOfInputs: %d\n", (int) result);
				else
				{
					#ifndef NDEBUG
						printf ("  Number of inputs: %d\n", (int) capture->channel + 1);
					#endif
					
					// add to overall number of inputs
					number_of_inputs += capture->channel + 1;
					
					// did the user select an input that falls into this device's 
					// range of inputs? Then leave the loop
					if (number_of_inputs > index)
					{
						// calculate relative channel index
						capture->channel = index - number_of_inputs + capture->channel + 1;
						OPENCV_ASSERT (capture->channel >= 0, "icvOpenCamera_QT", "negative channel number");
						
						// dump channel name
						#ifndef NDEBUG
							char  name[256];
							Str255  nameBuffer;
						
							result = VDGetInputName (capture->grabber, capture->channel, nameBuffer);
							OPENCV_ASSERT (result == noErr, "ictOpenCamera_QT", "couldnt GetInputName()");
							snprintf (name, *nameBuffer, "%s", (char *) (nameBuffer + 1));
							printf ("  Choosing input %d - %s\n", (int) capture->channel, name);
						#endif
						
						// leave the loop
						break;
					}
				}
				
				// obviously no inputs of this device/component were needed
				CloseComponent (capture->grabber);
			}
		}
	}
	while (component);
	
	// did we find the desired input?
	if (! component)
	{
		fprintf(stderr, "Not enough inputs available - can't choose input %d\n", index);
		return 0;
	}
	
	// -- Okay now, we selected the digitizer input, lets set up digitizer destination --
	
	ClearMoviesStickyError();
	
	// Select the desired input
	result = VDSetInput (capture->grabber, capture->channel);
	OPENCV_ASSERT (result == noErr, "icvOpenCamera_QT", "couldnt select video digitizer input");
										
	// get the bounding rectangle of the video digitizer
	result = VDGetActiveSrcRect (capture->grabber, capture->channel, & myRect);
	OPENCV_ASSERT (result == noErr, "icvOpenCamera_QT", "couldnt create VDGetActiveSrcRect from digitizer");
	myRect.right = 640; myRect.bottom = 480;
	capture->size = cvSize (myRect.right - myRect.left, myRect.bottom - myRect.top);
	printf ("Source rect is %d, %d -- %d, %d\n", (int) myRect.left, (int) myRect.top, (int) myRect.right, (int) myRect.bottom);
	
	// create offscreen GWorld
	result = QTNewGWorld (& capture->myGWorld, k32ARGBPixelFormat, & myRect, nil, nil, 0);
	OPENCV_ASSERT (result == noErr, "icvOpenCamera_QT", "couldnt create QTNewGWorld() for output image");
	
	// get pixmap
	capture->pixmap = GetGWorldPixMap (capture->myGWorld);
	result = GetMoviesError ();
	OPENCV_ASSERT (result == noErr, "icvOpenCamera_QT", "couldnt get pixmap");

	// set digitizer rect
	result = VDSetDigitizerRect (capture->grabber, & myRect);
	OPENCV_ASSERT (result == noErr, "icvOpenCamera_QT", "couldnt create VDGetActiveSrcRect from digitizer");
	
	// set destination of digitized input
	result = VDSetPlayThruDestination (capture->grabber, capture->pixmap, & myRect, nil, nil);
	printf ("QuickTime error: %d\n", (int) result);
	OPENCV_ASSERT (result == noErr, "icvOpenCamera_QT", "couldnt set video destination");
	
	// get destination of digitized images
	result = VDGetPlayThruDestination (capture->grabber, & capture->pixmap, nil, nil, nil);
	printf ("QuickTime error: %d\n", (int) result);
	OPENCV_ASSERT (result == noErr, "icvOpenCamera_QT", "couldnt get video destination");
	OPENCV_ASSERT (capture->pixmap != nil, "icvOpenCamera_QT", "empty set video destination");
	
	// get the bounding rectangle of the video digitizer
	GetPixBounds (capture->pixmap, & myRect);
	capture->size = cvSize (myRect.right - myRect.left, myRect.bottom - myRect.top);

	// build IplImage header that will point to the PixMap of the Movie's GWorld later on
	capture->image_rgb = cvCreateImageHeader (capture->size, IPL_DEPTH_8U, 4);
	OPENCV_ASSERT (capture->image_rgb, "icvOpenCamera_QT", "couldnt create image header");
	
	// create IplImage that hold correctly formatted result
	capture->image_bgr = cvCreateImage (capture->size, IPL_DEPTH_8U, 3);
	OPENCV_ASSERT (capture->image_bgr, "icvOpenCamera_QT", "couldnt create image");
	
	// notify digitizer component, that we well be starting grabbing soon
	result = VDCaptureStateChanging (capture->grabber, vdFlagCaptureIsForRecord | vdFlagCaptureStarting | vdFlagCaptureLowLatency);
	OPENCV_ASSERT (result == noErr, "icvOpenCamera_QT", "couldnt set capture state");
	
	
	// yeah, we did it
	return 1;
}

static int icvClose_QT_Cam (CvCapture_QT_Cam * capture)
{
	OPENCV_ASSERT (capture, "icvClose_QT_Cam", "'capture' is a NULL-pointer");
	
	ComponentResult	result = noErr;

	// notify digitizer component, that we well be stopping grabbing soon
	result = VDCaptureStateChanging (capture->grabber, vdFlagCaptureStopping);
	OPENCV_ASSERT (result == noErr, "icvOpenCamera_QT", "couldnt set capture state");
	
	// release memory
	cvReleaseImage       (& capture->image_bgr);
	cvReleaseImageHeader (& capture->image_rgb);
	DisposeGWorld        (capture->myGWorld);
	CloseComponent       (capture->grabber);
	
	// sucessful
	return 1;
}

static int icvGrabFrame_QT_Cam (CvCapture_QT_Cam * capture)
{
	OPENCV_ASSERT (capture,          "icvGrabFrame_QT_Cam", "'capture' is a NULL-pointer");
	OPENCV_ASSERT (capture->grabber, "icvGrabFrame_QT_Cam", "'grabber' is a NULL-pointer");
	
	ComponentResult	result = noErr;
	
	// grab one frame
	result = VDGrabOneFrame (capture->grabber);
	if (result != noErr)
	{
		fprintf (stderr, "VDGrabOneFrame failed\n");
		return 0;
	}
	
	// successful
	return 1;
}

static const void * icvRetrieveFrame_QT_Cam (CvCapture_QT_Cam * capture)
{
	OPENCV_ASSERT (capture, "icvRetrieveFrame_QT_Cam", "'capture' is a NULL-pointer");
	
	PixMapHandle  myPixMapHandle = nil;

	// update IplImage header that points to PixMap of the Movie's GWorld.
	// unfortunately, cvCvtColor doesn't know ARGB, the QuickTime pixel format,
	// so we pass a modfied address.
	// ATTENTION: don't access the last pixel's alpha entry, it's inexistant
	//myPixMapHandle = GetGWorldPixMap (capture->myGWorld);
	myPixMapHandle = capture->pixmap;
	LockPixels (myPixMapHandle);
	cvSetData (capture->image_rgb, GetPixBaseAddr (myPixMapHandle) + 1, GetPixRowBytes (myPixMapHandle));
	
	// covert RGB of GWorld to BGR
	cvCvtColor (capture->image_rgb, capture->image_bgr, CV_RGBA2BGR);
	
	// allow QuickTime to access the buffer again
	UnlockPixels (myPixMapHandle);
	
    // always return the same image pointer
	return capture->image_bgr;
}

#else 
#pragma mark Capturing using Sequence Grabber

static OSErr icvDataProc_QT_Cam (SGChannel channel, Ptr raw_data, long len, long *, long, TimeValue, short, long refCon)
{
	CvCapture_QT_Cam  * capture = (CvCapture_QT_Cam *) refCon;
	CodecFlags          ignore;
	ComponentResult     err     = noErr;
	
	
	// we need valid pointers
	OPENCV_ASSERT (capture,          "icvDataProc_QT_Cam", "'capture' is a NULL-pointer");
	OPENCV_ASSERT (capture->gworld,  "icvDataProc_QT_Cam", "'gworld' is a NULL-pointer");
	OPENCV_ASSERT (raw_data,         "icvDataProc_QT_Cam", "'raw_data' is a NULL-pointer");
	
	// create a decompression sequence the first time
	if (capture->sequence == 0)
	{
		ImageDescriptionHandle   description = (ImageDescriptionHandle) NewHandle(0);
		
		// we need a decompression sequence that fits the raw data coming from the camera
		err = SGGetChannelSampleDescription (channel, (Handle) description);
		OPENCV_ASSERT (err == noErr, "icvDataProc_QT_Cam", "couldnt get channel sample description");
		err = DecompressSequenceBegin (&capture->sequence, description, capture->gworld, 0, &capture->bounds, 
			                           nil, srcCopy, nil, 0, codecNormalQuality, bestSpeedCodec);
		OPENCV_ASSERT (err == noErr, "icvDataProc_QT_Cam", "couldnt begin decompression sequence");

		DisposeHandle ((Handle) description);
	}
	
	// okay, we have a decompression sequence -> decompress!
	err = DecompressSequenceFrameS (capture->sequence, raw_data, len, 0, &ignore, nil);
	if (err != noErr)
	{
		fprintf (stderr, "icvDataProc_QT_Cam: couldn't decompress frame - %d\n", (int) err);
		return err;
	}
	
	// check if we dropped a frame
	#ifndef NDEBUG
		if (capture->got_frame)
			fprintf (stderr, "icvDataProc_QT_Cam: frame was dropped\n");
	#endif
	
	// everything worked as expected
	capture->got_frame = true;
	return noErr;
}


static int icvOpenCamera_QT (CvCapture_QT_Cam * capture, const int index)
{
	OPENCV_ASSERT (capture,    "icvOpenCamera_QT", "'capture' is a NULL-pointer");
	OPENCV_ASSERT (index >= 0, "icvOpenCamera_QT", "camera index is negative");
	
	PixMapHandle  pixmap       = nil;
	OSErr         result       = noErr;
	Rect          defaultRect  = {0, 0, 240, 320};
	
	// create offscreen GWorld
	capture->bounds = defaultRect;
	result = QTNewGWorld (& (capture->gworld), k32ARGBPixelFormat, & (capture->bounds), 0, 0, 0);
	capture->size = cvSize (capture->bounds.right - capture->bounds.left, capture->bounds.bottom - capture->bounds.top);
	OPENCV_ASSERT (result == noErr, "icvOpenCamera_QT", "couldnt create offscreen GWorld");
	
	// build IplImage header that points to the PixMap of the Movie's GWorld.
	// unfortunately, cvCvtColor doesn't know ARGB, the QuickTime pixel format,
	// so we shift the base address by one byte.
	// ATTENTION: don't access the last pixel's alpha entry, it's inexistant
	capture->image_rgb = cvCreateImageHeader (capture->size, IPL_DEPTH_8U, 4);
	OPENCV_ASSERT (capture->image_rgb, "icvOpenCamera_QT", "couldnt create image header");
	pixmap = GetGWorldPixMap (capture->gworld);
	OPENCV_ASSERT (pixmap, "icvOpenCamera_QT", "didn't get GWorld PixMap handle");
	LockPixels (pixmap);
	cvSetData (capture->image_rgb, GetPixBaseAddr (pixmap) + 1, GetPixRowBytes (pixmap));
	
	// create IplImage that hold correctly formatted result
	capture->image_bgr = cvCreateImage (capture->size, IPL_DEPTH_8U, 3);
	OPENCV_ASSERT (capture->image_bgr, "icvOpenCamera_QT", "couldnt create image");
	
	// open sequence grabber component
	capture->grabber = OpenDefaultComponent (SeqGrabComponentType, 0);
	OPENCV_ASSERT (capture->grabber, "icvOpenCamera_QT", "couldnt create image");
	
	// initialize sequence grabber component
	result = SGInitialize (capture->grabber);
	OPENCV_ASSERT (result == noErr, "icvOpenCamera_QT", "couldnt initialize sequence grabber");
	result = SGSetDataRef (capture->grabber, 0, 0, seqGrabDontMakeMovie);
	OPENCV_ASSERT (result == noErr, "icvOpenCamera_QT", "couldnt set data reference of sequence grabber");
	result = SGSetGWorld (capture->grabber, capture->gworld, 0);
	OPENCV_ASSERT (result == noErr, "icvOpenCamera_QT", "couldnt set GWorld for sequence grabber");
	
	// set up video channel
	result = SGNewChannel (capture->grabber, VideoMediaType, & (capture->channel));
	OPENCV_ASSERT (result == noErr, "icvOpenCamera_QT", "couldnt create new video channel");
	result = SGSetChannelBounds (capture->channel, & (capture->bounds));
	OPENCV_ASSERT (result == noErr, "icvOpenCamera_QT", "couldnt set video channel bounds");
	result = SGSetChannelUsage (capture->channel, seqGrabRecord);
	OPENCV_ASSERT (result == noErr, "icvOpenCamera_QT", "couldnt set channel usage");
	
	// tell the sequence grabber to invoke our data proc
	result = SGSetDataProc (capture->grabber, NewSGDataUPP (icvDataProc_QT_Cam), (long) capture);
	OPENCV_ASSERT (result == noErr, "icvOpenCamera_QT", "couldnt set data proc");
	
	// start recording
	result = SGStartRecord (capture->grabber);
	OPENCV_ASSERT (result == noErr, "icvOpenCamera_QT", "couldnt start recording");
	
	return 1;
}


static int icvClose_QT_Cam (CvCapture_QT_Cam * capture)
{
	OPENCV_ASSERT (capture, "icvClose_QT_Cam", "'capture' is a NULL-pointer");
	
	OSErr  result = noErr;
	
	
	// stop recording
	result = SGStop (capture->grabber);
	OPENCV_ASSERT (result == noErr, "icveClose_QT_Cam", "couldnt stop recording");

	// close sequence grabber component
	result = CloseComponent (capture->grabber);
	OPENCV_ASSERT (result == noErr, "icveClose_QT_Cam", "couldnt close sequence grabber component");
	
	// end decompression sequence
	CDSequenceEnd (capture->sequence);
	
	// free memory
	cvReleaseImage (& capture->image_bgr);
	cvReleaseImageHeader (& capture->image_rgb);
	DisposeGWorld (capture->gworld); 
	
	// sucessful
	return 1;
}

static int icvGrabFrame_QT_Cam (CvCapture_QT_Cam * capture)
{
	OPENCV_ASSERT (capture,          "icvGrabFrame_QT_Cam", "'capture' is a NULL-pointer");
	OPENCV_ASSERT (capture->grabber, "icvGrabFrame_QT_Cam", "'grabber' is a NULL-pointer");

	ComponentResult	result = noErr;


	// grab one frame
	result = SGIdle (capture->grabber);
	if (result != noErr)
	{
		fprintf (stderr, "SGIdle failed in icvGrabFrame_QT_Cam with error %d\n", (int) result);
		return 0;
	}
	
	// successful
	return 1;
}

static const void * icvRetrieveFrame_QT_Cam (CvCapture_QT_Cam * capture)
{
	OPENCV_ASSERT (capture,            "icvRetrieveFrame_QT_Cam", "'capture' is a NULL-pointer");
	OPENCV_ASSERT (capture->image_rgb, "icvRetrieveFrame_QT_Cam", "invalid source image");
	OPENCV_ASSERT (capture->image_bgr, "icvRetrieveFrame_QT_Cam", "invalid destination image");
	
	OSErr         myErr          = noErr;
	

	// service active sequence grabbers (= redraw immediately)
	while (! capture->got_frame)
	{
		myErr = SGIdle (capture->grabber);
		if (myErr != noErr)
		{
			fprintf (stderr, "SGIdle() didn't succeed in icvRetrieveFrame_QT_Cam().\n");
			return 0;
		}
	}
	
	// covert RGB of GWorld to BGR
	cvCvtColor (capture->image_rgb, capture->image_bgr, CV_RGBA2BGR);
	
	// reset grabbing status
	capture->got_frame = false;
	
    // always return the same image pointer
	return capture->image_bgr;
}

#endif

