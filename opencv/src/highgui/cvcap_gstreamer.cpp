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
// this implementation was inspired by gnash's gstreamer interface

//
// use GStreamer to read a video
//

#include <unistd.h>
#include <string.h>
#include <gst/gst.h>
#include "_highgui.h"
#include "gstappsink.h"

#ifdef NDEBUG
#define CV_WARN(message)
#else
#define CV_WARN(message) fprintf(stderr, "warning: %s (%s:%d)\n", message, __FILE__, __LINE__)
#endif

static bool isInited = false;

typedef struct CvCapture_GStreamer
{
	/// method call table
	CvCaptureVTable	       *vtable;

	GstElement	       *pipeline;
	GstElement	       *appsink;

	GstBuffer	       *buffer;

	IplImage	       *frame;
} CvCapture_GStreamer;

static void icvClose_GStreamer(CvCapture *capture)
{
	CvCapture_GStreamer *cap = (CvCapture_GStreamer *)capture;

	if(cap->pipeline) {
		gst_element_set_state(GST_ELEMENT(cap->pipeline), GST_STATE_NULL);
		gst_object_unref(GST_OBJECT(cap->pipeline));
	}

	if(cap->buffer)
		gst_buffer_unref(cap->buffer);

	if(cap->frame)
		cvReleaseImage(&cap->frame);
}

static void icvHandleMessage(CvCapture_GStreamer *cap)
{
	GstBus* bus = gst_element_get_bus(cap->pipeline);

	while(gst_bus_have_pending(bus)) {
		GstMessage* msg = gst_bus_pop(bus);

//		printf("Got %s message\n", GST_MESSAGE_TYPE_NAME(msg));

		switch (GST_MESSAGE_TYPE (msg)) {
		case GST_MESSAGE_STATE_CHANGED:
			GstState oldstate, newstate, pendstate;
			gst_message_parse_state_changed(msg, &oldstate, &newstate, &pendstate);
//			printf("state changed from %d to %d (%d)\n", oldstate, newstate, pendstate);
			break;
		case GST_MESSAGE_ERROR: {
			GError *err;
			gchar *debug;
			gst_message_parse_error(msg, &err, &debug);

			fprintf(stderr, "GStreamer Plugin: Embedded video playback halted; module %s reported: %s\n",
				  gst_element_get_name(GST_MESSAGE_SRC (msg)), err->message);

			g_error_free(err);
			g_free(debug);

			gst_element_set_state(cap->pipeline, GST_STATE_NULL);

			break;
			}
		case GST_MESSAGE_EOS:
//			CV_WARN("NetStream has reached the end of the stream.");

			break;
		default:
//			CV_WARN("unhandled message\n");
			break;
		}

		gst_message_unref(msg);
	}

	gst_object_unref(GST_OBJECT(bus));
}

//
// start the pipeline, grab a buffer, and pause again
//
static int icvGrabFrame_GStreamer(CvCapture *capture)
{
	CvCapture_GStreamer *cap = (CvCapture_GStreamer *)capture;

	if(!cap->pipeline)
		return 0;

	if(gst_app_sink_is_eos(GST_APP_SINK(cap->appsink))) {
		//printf("end of stream\n");
		return 0;
	}

	if(cap->buffer)
		gst_buffer_unref(cap->buffer);

	icvHandleMessage(cap);

	if(!gst_app_sink_get_queue_length(GST_APP_SINK(cap->appsink))) {
		if(gst_element_set_state(GST_ELEMENT(cap->pipeline), GST_STATE_PLAYING) ==
		GST_STATE_CHANGE_FAILURE) {
			icvHandleMessage(cap);
			return 0;
		}

//		printf("pulling buffer\n");

		cap->buffer = gst_app_sink_pull_buffer(GST_APP_SINK(cap->appsink));

		if(gst_element_set_state(GST_ELEMENT(cap->pipeline), GST_STATE_PAUSED) ==
		GST_STATE_CHANGE_FAILURE) {
			icvHandleMessage(cap);
			return 0;
		}
	} else {
//		printf("peeking buffer\n");
		cap->buffer = gst_app_sink_peek_buffer(GST_APP_SINK(cap->appsink));
	}

//	printf("pulled buffer %p\n", cap->buffer);

	if(!cap->buffer)
		return 0;

	return 1;
}

//
// decode buffer
//
static IplImage *icvRetrieveFrame_GStreamer(CvCapture *capture)
{
	CvCapture_GStreamer *cap = (CvCapture_GStreamer *)capture;

	if(!cap->buffer)
		return 0;

	GstCaps* caps = gst_buffer_get_caps(cap->buffer);

	assert(gst_caps_get_size(caps) == 1);

	GstStructure* structure = gst_caps_get_structure(caps, 0);

	gint bpp, endianness, redmask, greenmask, bluemask;

	gst_structure_get_int(structure, "bpp", &bpp);
	gst_structure_get_int(structure, "endianness", &endianness);
	gst_structure_get_int(structure, "red_mask", &redmask);
	gst_structure_get_int(structure, "green_mask", &greenmask);
	gst_structure_get_int(structure, "blue_mask", &bluemask);

	if(!cap->frame) {
		gint height, width;

		gst_structure_get_int(structure, "width", &width);
		gst_structure_get_int(structure, "height", &height);

		cap->frame = cvCreateImage(cvSize(width, height), IPL_DEPTH_8U, 3);
	}

	gst_caps_unref(caps);

	unsigned char *data = GST_BUFFER_DATA(cap->buffer);

	IplImage *frame = cap->frame;
	int nbyte = bpp >> 3;
	int redshift, blueshift, greenshift;
	int mask = redmask;
	for(redshift = 0, mask = redmask; (mask & 1) == 0; mask >>= 1, redshift++)
		;
	for(greenshift = 0, mask = greenmask; (mask & 1) == 0; mask >>= 1, greenshift++)
		;
	for(blueshift = 0, mask = bluemask; (mask & 1) == 0; mask >>= 1, blueshift++)
		;

	for(int r = 0; r < frame->height; r++) {
		for(int c = 0; c < frame->width; c++, data += nbyte) {
			int at = r * frame->widthStep + c * 3;
			frame->imageData[at] = ((*((gint *)data)) & redmask) >> redshift;
			frame->imageData[at+1] = ((*((gint *)data)) & greenmask) >> greenshift;
			frame->imageData[at+2] = ((*((gint *)data)) & bluemask) >> blueshift;
		}
	}

	gst_buffer_unref(cap->buffer);
	cap->buffer = 0;

	return cap->frame;
}

static double icvGetProperty_GStreamer(CvCapture *capture, int id)
{
	CvCapture_GStreamer *cap = (CvCapture_GStreamer *)capture;
	GstFormat format;
	//GstQuery q;
	gint64 value;

	if(!cap->pipeline) {
		CV_WARN("GStreamer: no pipeline");
		return 0;
	}

	switch(id) {
	case CV_CAP_PROP_POS_MSEC:
		format = GST_FORMAT_TIME;
		if(!gst_element_query_position(cap->pipeline, &format, &value)) {
			CV_WARN("GStreamer: unable to query position of stream");
			return 0;
		}
		return value * 1e-6; // nano seconds to milli seconds
	case CV_CAP_PROP_POS_FRAMES:
		format = GST_FORMAT_BUFFERS;
		if(!gst_element_query_position(cap->pipeline, &format, &value)) {
			CV_WARN("GStreamer: unable to query position of stream");
			return 0;
		}
		return value;
	case CV_CAP_PROP_POS_AVI_RATIO:
		format = GST_FORMAT_PERCENT;
		if(!gst_element_query_position(cap->pipeline, &format, &value)) {
			CV_WARN("GStreamer: unable to query position of stream");
			return 0;
		}
//		printf("value %llu %llu %g\n", value, GST_FORMAT_PERCENT_MAX, ((double) value) / GST_FORMAT_PERCENT_MAX);
		return ((double) value) / GST_FORMAT_PERCENT_MAX;
	case CV_CAP_PROP_FRAME_WIDTH:
	case CV_CAP_PROP_FRAME_HEIGHT:
	case CV_CAP_PROP_FPS:
	case CV_CAP_PROP_FOURCC:
		break;
	case CV_CAP_PROP_FRAME_COUNT:
		format = GST_FORMAT_BUFFERS;
		if(!gst_element_query_duration(cap->pipeline, &format, &value)) {
			CV_WARN("GStreamer: unable to query position of stream");
			return 0;
		}
		return value;
	case CV_CAP_PROP_FORMAT:
	case CV_CAP_PROP_MODE:
	case CV_CAP_PROP_BRIGHTNESS:
	case CV_CAP_PROP_CONTRAST:
	case CV_CAP_PROP_SATURATION:
	case CV_CAP_PROP_HUE:
	case CV_CAP_PROP_GAIN:
	case CV_CAP_PROP_CONVERT_RGB:
		break;
	}
	CV_WARN("GStreamer: unhandled property");
	return 0;
}

static int icvSetProperty_GStreamer(CvCapture *capture, int id, double value)
{
	CvCapture_GStreamer *cap = (CvCapture_GStreamer *)capture;
	GstFormat format;
	GstSeekFlags flags;

	if(!cap->pipeline) {
		CV_WARN("GStreamer: no pipeline");
		return 0;
	}

	switch(id) {
	case CV_CAP_PROP_POS_MSEC:
		format = GST_FORMAT_TIME;
		flags = (GstSeekFlags) (GST_SEEK_FLAG_FLUSH|GST_SEEK_FLAG_ACCURATE);
		if(!gst_element_seek_simple(GST_ELEMENT(cap->pipeline), format,
					    flags, (gint64) (value * GST_MSECOND))) {
			CV_WARN("GStreamer: unable to seek");
			return 0;
		}
	case CV_CAP_PROP_POS_FRAMES:
		format = GST_FORMAT_BUFFERS;
		flags = (GstSeekFlags) (GST_SEEK_FLAG_FLUSH|GST_SEEK_FLAG_ACCURATE);
		if(!gst_element_seek_simple(GST_ELEMENT(cap->pipeline), format,
					    flags, (gint64) value)) {
			CV_WARN("GStreamer: unable to seek");
			return 0;
		}
	case CV_CAP_PROP_POS_AVI_RATIO:
		format = GST_FORMAT_PERCENT;
		flags = (GstSeekFlags) (GST_SEEK_FLAG_FLUSH|GST_SEEK_FLAG_ACCURATE);
		if(!gst_element_seek_simple(GST_ELEMENT(cap->pipeline), format,
					    flags, (gint64) (value * GST_FORMAT_PERCENT_MAX))) {
			CV_WARN("GStreamer: unable to seek");
			return 0;
		}
	case CV_CAP_PROP_FRAME_WIDTH:
	case CV_CAP_PROP_FRAME_HEIGHT:
	case CV_CAP_PROP_FPS:
	case CV_CAP_PROP_FOURCC:
	case CV_CAP_PROP_FRAME_COUNT:
	case CV_CAP_PROP_FORMAT:
	case CV_CAP_PROP_MODE:
	case CV_CAP_PROP_BRIGHTNESS:
	case CV_CAP_PROP_CONTRAST:
	case CV_CAP_PROP_SATURATION:
	case CV_CAP_PROP_HUE:
	case CV_CAP_PROP_GAIN:
	case CV_CAP_PROP_CONVERT_RGB:
		break;
	}
	CV_WARN("GStreamer: unhandled property");
	return 0;
}

static CvCaptureVTable capture_vtable =
{
	6,
	( CvCaptureCloseFunc ) icvClose_GStreamer,
	( CvCaptureGrabFrameFunc ) icvGrabFrame_GStreamer,
	( CvCaptureRetrieveFrameFunc ) icvRetrieveFrame_GStreamer,
	( CvCaptureGetPropertyFunc ) icvGetProperty_GStreamer,
	( CvCaptureSetPropertyFunc ) icvSetProperty_GStreamer,
	( CvCaptureGetDescriptionFunc ) 0
};

//
// connect decodebin's dynamically created source pads to colourconverter
//
static void newpad(GstElement *decodebin, GstPad *pad, gboolean last, gpointer data)
{
	GstElement *sink = GST_ELEMENT(data);
	GstStructure *str;
	GstPad *sinkpad;
	GstCaps *caps;

	/* only link once */
	sinkpad = gst_element_get_pad(sink, "sink");

	if(GST_PAD_IS_LINKED(sinkpad)) {
		g_print("sink is already linked\n");
		g_object_unref(sinkpad);
		return;
	}

	/* check media type */
	caps = gst_pad_get_caps(pad);
	str = gst_caps_get_structure(caps, 0);
	const char *structname = gst_structure_get_name(str);
	g_print("new pad %s\n", structname);
	if(!g_strrstr(structname, "video")) {
		gst_caps_unref(caps);
		gst_object_unref(sinkpad);
		return;
	}
	printf("linking pad %s\n", structname);

	gst_caps_unref (caps);

	/* link'n'play */
	gst_pad_link (pad, sinkpad);
}

CvCapture * cvCaptureFromCAM_GStreamer(const char *sourcetype)
{
	CvCapture_GStreamer *capture = 0;
	CV_FUNCNAME("cvCaptureFromCAM_GStreamer");

	__BEGIN__;

//	teststreamer(filename);

//	return 0;
	printf("entered capturecreator\n");

	if(!isInited) {
		gst_init (NULL, NULL);

// according to the documentation this is the way to register a plugin now
// unfortunately, it has not propagated into my distribution yet...
// 		gst_plugin_register_static(GST_VERSION_MAJOR, GST_VERSION_MINOR,
// 			"opencv-appsink", "Element application sink",
// 			"0.1", appsink_plugin_init, "LGPL", "highgui", "opencv",
// 			"http://opencvlibrary.sourceforge.net/");

		isInited = true;
	}

	GstElement *pipeline = gst_pipeline_new (NULL);

	GstElement *source = gst_element_factory_make(sourcetype, NULL);

	GstElement *colour = gst_element_factory_make("ffmpegcolorspace", NULL);

	GstElement *sink = gst_element_factory_make("opencv-appsink", NULL);
	GstCaps *caps = gst_caps_new_simple("video/x-raw-rgb", NULL);
	gst_app_sink_set_caps(GST_APP_SINK(sink), caps);
	gst_base_sink_set_sync(GST_BASE_SINK(sink), false);
//	g_signal_connect(sink, "new-buffer", G_CALLBACK(newbuffer), NULL);

	GstElement *decodebin = gst_element_factory_make("decodebin", NULL);
	g_signal_connect(decodebin, "new-decoded-pad", G_CALLBACK(newpad), colour);

	gst_bin_add_many(GST_BIN(pipeline), source, decodebin, colour, sink, NULL);

	printf("added many\n");

	if(!gst_element_link(source, decodebin)) {
		CV_ERROR(CV_StsError, "GStreamer: cannot link filesrc -> decodebin\n");
		return 0;
	}

	if(!gst_element_link(colour, sink)) {
		CV_ERROR(CV_StsError, "GStreamer: cannot link colour -> sink\n");
		return 0;
	}

	printf("linked\n");

	// construct capture struct
	capture = (CvCapture_GStreamer *)cvAlloc(sizeof(CvCapture_GStreamer));
	memset(capture, 0, sizeof(CvCapture_GStreamer));
	capture->vtable = &capture_vtable;
	capture->pipeline = pipeline;
	capture->appsink = sink;

	printf("pausing\n");

	if(gst_element_set_state(GST_ELEMENT(pipeline), GST_STATE_PAUSED) ==
	   GST_STATE_CHANGE_FAILURE) {
		CV_ERROR(CV_StsError, "GStreamer: unable to start pipeline\n");
		icvHandleMessage(capture);
		cvReleaseCapture((CvCapture **)(void *)&capture);
		return 0;
	}

	printf("state now paused\n");

	icvHandleMessage(capture);

	OPENCV_ASSERT(capture,
                      "cvCaptureFromFile_GStreamer( const char * )", "couldn't create capture");

//	GstClock *clock = gst_pipeline_get_clock(GST_PIPELINE(pipeline));
//	printf("clock %s\n", gst_object_get_name(GST_OBJECT(clock)));

	__END__;

	return (CvCapture *)capture;
}

CvCapture * cvCaptureFromFile_GStreamer (const char * filename)
{
	CvCapture_GStreamer *capture = 0;
	CV_FUNCNAME("cvCaptureFromFile_GStreamer");

	__BEGIN__;

//	teststreamer(filename);

//	return 0;

	if(!isInited) {
		gst_init (NULL, NULL);

// according to the documentation this is the way to register a plugin now
// unfortunately, it has not propagated into my distribution yet...
// 		gst_plugin_register_static(GST_VERSION_MAJOR, GST_VERSION_MINOR,
// 			"opencv-appsink", "Element application sink",
// 			"0.1", appsink_plugin_init, "LGPL", "highgui", "opencv",
// 			"http://opencvlibrary.sourceforge.net/");

		isInited = true;
	}

	GstElement *pipeline = gst_pipeline_new (NULL);

	GstElement *source = gst_element_factory_make("filesrc", NULL);
	g_object_set(G_OBJECT(source), "location", filename, NULL);

	GstElement *colour = gst_element_factory_make("ffmpegcolorspace", NULL);

	GstElement *sink = gst_element_factory_make("opencv-appsink", NULL);
	GstCaps *caps = gst_caps_new_simple("video/x-raw-rgb", NULL);
	gst_app_sink_set_caps(GST_APP_SINK(sink), caps);
	gst_base_sink_set_sync(GST_BASE_SINK(sink), false);
//	g_signal_connect(sink, "new-buffer", G_CALLBACK(newbuffer), NULL);

	GstElement *decodebin = gst_element_factory_make("decodebin", NULL);
	g_signal_connect(decodebin, "new-decoded-pad", G_CALLBACK(newpad), colour);

	gst_bin_add_many(GST_BIN(pipeline), source, decodebin, colour, sink, NULL);

	if(!gst_element_link(source, decodebin)) {
		CV_ERROR(CV_StsError, "GStreamer: cannot link filesrc -> decodebin\n");
		return 0;
	}

	if(!gst_element_link(colour, sink)) {
		CV_ERROR(CV_StsError, "GStreamer: cannot link colour -> sink\n");
		return 0;
	}

	// construct capture struct
	capture = (CvCapture_GStreamer *)cvAlloc(sizeof(CvCapture_GStreamer));
	memset(capture, 0, sizeof(CvCapture_GStreamer));
	capture->vtable = &capture_vtable;
	capture->pipeline = pipeline;
	capture->appsink = sink;

	if(gst_element_set_state(GST_ELEMENT(pipeline), GST_STATE_PAUSED) ==
	   GST_STATE_CHANGE_FAILURE) {
		CV_ERROR(CV_StsError, "GStreamer: unable to start pipeline\n");
		icvHandleMessage(capture);
		cvReleaseCapture((CvCapture **)(void *)&capture);
		return 0;
	}

	icvHandleMessage(capture);

	OPENCV_ASSERT(capture,
                      "cvCaptureFromFile_GStreamer( const char * )", "couldn't create capture");

//	GstClock *clock = gst_pipeline_get_clock(GST_PIPELINE(pipeline));
//	printf("clock %s\n", gst_object_get_name(GST_OBJECT(clock)));

	__END__;

	return (CvCapture *)capture;
}

#if 0
//
//
// image sequence writer
//
//
typedef struct CvVideoWriter_GStreamer {
	CvVideoWriterVTable    *vtable;
	char		       *filename;
	unsigned		currentframe;
};

static int icvWriteFrame_GStreamer( CvVideoWriter* writer, const IplImage* image )
{
	CvVideoWriter_GStreamer *wri = (CvVideoWriter_GStreamer *)writer;

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

static void icvReleaseVideoWriter_GStreamer( CvVideoWriter** writer )
{
	CvVideoWriter_GStreamer **wri = (CvVideoWriter_GStreamer **)writer;

	free((*wri)->filename);
}

static CvVideoWriterVTable writer_vtable =
{
    2,
    (CvVideoWriterCloseFunc)icvReleaseVideoWriter_GStreamer,
    (CvVideoWriterWriteFrameFunc)icvWriteFrame_GStreamer
};

CvVideoWriter* cvCreateVideoWriter_GStreamer( const char* filename )
{
	CvVideoWriter_GStreamer *writer;

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

	writer = (CvVideoWriter_GStreamer *)cvAlloc(sizeof(CvCapture_GStreamer));
	memset(writer, 0, sizeof(CvVideoWriter_GStreamer));
	writer->filename = strdup(name);
	writer->currentframe = offset;
	writer->vtable = &writer_vtable;

	return (CvVideoWriter *)writer;
}
#endif
