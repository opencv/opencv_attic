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
// Copyright (C) 2008, Nils Hasler, all rights reserved.
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

// Author: Nils Hasler <hasler@mpi-inf.mpg.de>
//
//         Max-Planck-Institut Informatik

//
// capture video from a sequence of images
// the filename when opening can either be a printf pattern such as
// video%04d.png or the first frame of the sequence i.e. video0001.png
//

#include <sys/stat.h>
#include "_highgui.h"

#ifdef NDEBUG
#define CV_WARN(message)
#else
#define CV_WARN(message) fprintf(stderr, "warning: %s (%s:%d)\n", message, __FILE__, __LINE__)
#endif

typedef struct CvCapture_Images
{
	/// method call table
	CvCaptureVTable	       *vtable;

	char		       *filename;	// actually a printf-pattern
	unsigned int		currentframe;
	unsigned int		firstframe;	// number of first frame

	IplImage	       *frame;
	unsigned int		length;		// length of sequence
} CvCapture_Images;

static void icvClose_Images(CvCapture *capture)
{
	CvCapture_Images *cap = (CvCapture_Images *)capture;

	free(cap->filename);
	if(cap->frame)
		cvReleaseImage(&cap->frame);
}

static int icvGrabFrame_Images(CvCapture *capture)
{
	CvCapture_Images *cap = (CvCapture_Images *)capture;
	char str[100];
	char *x = str;
	int size = 100;
	if(snprintf(x, size, cap->filename, cap->firstframe + cap->currentframe) == size - 1) {
		// buffer too small
		size *= 2;

		if(x == str)
			x = (char *)malloc(size);
		else
			x = (char *)realloc(x, size);
	}

	if(cap->frame)
		cvReleaseImage(&cap->frame);

	cap->frame = cvLoadImage(x, CV_LOAD_IMAGE_ANYDEPTH | CV_LOAD_IMAGE_ANYCOLOR);
	int res = (cap->frame != 0);

	if(x != str)
		free(x);

	if(res)
		cap->currentframe++;

	return res;
}

static IplImage *icvRetrieveFrame_Images(CvCapture *capture)
{
	CvCapture_Images *cap = (CvCapture_Images *)capture;

	return cap->frame;
}

static double icvGetProperty_Images(CvCapture *capture, int id)
{
	CvCapture_Images *cap = (CvCapture_Images *)capture;

	switch(id) {
	case CV_CAP_PROP_POS_MSEC:
		CV_WARN("collections of images don't have framerates\n");
		return 0;
	case CV_CAP_PROP_POS_FRAMES:
		return cap->currentframe;
	case CV_CAP_PROP_POS_AVI_RATIO:
		return (double) cap->currentframe / (double) (cap->length - 1);
	case CV_CAP_PROP_FRAME_WIDTH:
		if(cap->frame)
			return cap->frame->width;
		return 0;
	case CV_CAP_PROP_FRAME_HEIGHT:
		if(cap->frame)
			return cap->frame->height;
		return 0;
	case CV_CAP_PROP_FPS:
		CV_WARN("collections of images don't have framerates\n");
		return 1;
	case CV_CAP_PROP_FOURCC:
		CV_WARN("collections of images don't have 4-character codes\n");
		return 0;
	}
	return 0;
}

static int icvSetProperty_Images(CvCapture *capture, int id, double value)
{
	CvCapture_Images *cap = (CvCapture_Images *)capture;

	switch(id) {
	case CV_CAP_PROP_POS_MSEC:
	case CV_CAP_PROP_POS_FRAMES:
		if(value < 0) {
			CV_WARN("seeking to negative positions does not work - clamping\n");
			value = 0;
		}
		if(value >= cap->length) {
			CV_WARN("seeking beyond end of sequence - clamping\n");
			value = cap->length - 1;
		}
		cap->currentframe = (int) value;
		return 0;
	case CV_CAP_PROP_POS_AVI_RATIO:
		if(value > 1) {
			CV_WARN("seeking beyond end of sequence - clamping\n");
			value = 1;
		} else if(value < 0) {
			CV_WARN("seeking to negative positions does not work - clamping\n");
			value = 0;
		}
		cap->currentframe = (unsigned int) ((cap->length - 1) * value);
		return 0;
	}
	CV_WARN("unknown/unhandled property\n");
	return 0;
}

static CvCaptureVTable capture_vtable =
{
	6,
	( CvCaptureCloseFunc ) icvClose_Images,
	( CvCaptureGrabFrameFunc ) icvGrabFrame_Images,
	( CvCaptureRetrieveFrameFunc ) icvRetrieveFrame_Images,
	( CvCaptureGetPropertyFunc ) icvGetProperty_Images,
	( CvCaptureSetPropertyFunc ) icvSetProperty_Images,
	( CvCaptureGetDescriptionFunc ) 0
};

static char *icvExtractPattern(const char *filename, unsigned int *offset)
{
	char *name = (char *)filename;

	// check whether this is a valid image sequence filename
	char *at = strchr(name, '%');
	if(at) {
		int dummy;
		if(sscanf(at + 1, "%ud", &dummy) != 1)
			return 0;
	} else { // no pattern filename was given - extract the pattern
		for(at = name; *at && !isdigit(*at); at++)
			;

		if(!at)
			return 0;

		sscanf(at, "%u", offset);

		int size = strlen(filename) + 20;
		name = (char *)malloc(size);
		strncpy(name, filename, at - filename);
		name[at - filename] = 0;

		strcat(name, "%0");

		int i;
		char *extension;
		for(i = 0, extension = at; isdigit(at[i]); i++, extension++)
			;
		char places[10];
		sprintf(places, "%dd", i);

		strcat(name, places);
		strcat(name, extension);
	}

	return name;
}

CvCapture * cvCaptureFromFile_Images (const char * filename)
{
	unsigned offset = 0;

	char *name = icvExtractPattern(filename, &offset);
	if(!name)
		return 0;

	// determine the length of the sequence
	unsigned length = 0;
	char str[100];
	char *x = str;
	int size = 100;
	while(1) {
		if(snprintf(x, size, name, offset + length) == size - 1) {
			// buffer too small
			size *= 2;

			if(x == str)
				x = (char *)malloc(size);
			else
				x = (char *)realloc(x, size);
		}

		struct stat s;
		if(stat(x, &s)) {
			if(length == 0 && offset == 0) { // allow starting with 0 or 1
				offset++;
				continue;
			}
		}

		if(!cvHaveImageReader(x))
			break;

		length++;
	}

	if(x != str)
		free(x);

	if(length == 0)
		return 0;

	// construct capture struct
	CvCapture_Images *capture = (CvCapture_Images *)cvAlloc(sizeof(CvCapture_Images));
	memset(capture, 0, sizeof(CvCapture_Images));
	capture->vtable = &capture_vtable;
	capture->filename = strdup(name);
	capture->length = length;
	capture->firstframe = offset;

	OPENCV_ASSERT(capture,
                      "cvCaptureFromFile_Images( const char * )", "couldn't create capture");

	if(name != filename)
		free(name);

	return (CvCapture *)capture;
}

//
//
// image sequence writer
//
//
typedef struct CvVideoWriter_Images {
	CvVideoWriterVTable    *vtable;
	char		       *filename;
	unsigned		currentframe;
};

static int icvWriteFrame_Images( CvVideoWriter* writer, const IplImage* image )
{
	CvVideoWriter_Images *wri = (CvVideoWriter_Images *)writer;

	char str[100];
	char *x = str;
	int size = 100;
	while(snprintf(x, size, wri->filename, wri->currentframe) == size - 1) {
		size *= 2;
		if(x == str)
			x = (char *)malloc(size);
		else
			x = (char *)realloc(x, size);
	}

	int ret = cvSaveImage(x, image);

	wri->currentframe++;

	if(x != str)
		free(x);

	return ret;
}

static void icvReleaseVideoWriter_Images( CvVideoWriter** writer )
{
	CvVideoWriter_Images **wri = (CvVideoWriter_Images **)writer;

	free((*wri)->filename);
}

static CvVideoWriterVTable writer_vtable =
{
    2,
    (CvVideoWriterCloseFunc)icvReleaseVideoWriter_Images,
    (CvVideoWriterWriteFrameFunc)icvWriteFrame_Images
};

CvVideoWriter* cvCreateVideoWriter_Images( const char* filename )
{
	CvVideoWriter_Images *writer;

	unsigned offset = 0;
	char *name = icvExtractPattern(filename, &offset);
	if(!name)
		return 0;

	char str[100];
	char *x = str;
	int size = 100;
	while(snprintf(x, size, name, 0) == size - 1) {
		size *= 2;
		if(x == str)
			x = (char *)malloc(size);
		else
			x = (char *)realloc(x, size);
	}
	if(!cvHaveImageWriter(x)) {
		if(x != str)
			free(x);
		return 0;
	}
	if(x != str)
		free(x);

	writer = (CvVideoWriter_Images *)cvAlloc(sizeof(CvCapture_Images));
	memset(writer, 0, sizeof(CvVideoWriter_Images));
	writer->filename = strdup(name);
	writer->currentframe = offset;
	writer->vtable = &writer_vtable;

	return (CvVideoWriter *)writer;
}
