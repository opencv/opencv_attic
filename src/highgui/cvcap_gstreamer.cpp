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

#include "_highgui.h"
#include <unistd.h>
#include <string.h>
#include <gst/gst.h>
#include <gst/video/video.h>
#ifdef HAVE_GSTREAMER_APP
#include <gst/app/gstappsink.h>
#include <gst/app/gstappsrc.h>

#else
#include "gstappsink.h"
#endif

#ifdef NDEBUG
#define CV_WARN(message)
#else
#define CV_WARN(message) fprintf(stderr, "warning: %s (%s:%d)\n", message, __FILE__, __LINE__)
#endif

static bool isInited = false;
class CvCapture_GStreamer : public CvCapture
{
public:
    CvCapture_GStreamer() { init(); }
    virtual ~CvCapture_GStreamer() { close(); }

    virtual bool open( int type, const char* filename );
    virtual void close();

    virtual double getProperty(int);
    virtual bool setProperty(int, double);
    virtual bool grabFrame();
    virtual IplImage* retrieveFrame(int);
    GstElement	       *pipeline;
	GstElement	       *source;
	GstElement	       *decodebin;
	GstElement	       *colour;
	GstElement	       *appsink;

	GstBuffer	       *buffer;

	GstCaps		       *caps;	// filter caps inserted right after the source

	IplImage	       *frame;
protected:
    void init();
    bool reopen();
};
void CvCapture_GStreamer::init()
{
	frame=0;
    buffer=0;
}

static void icvClose_GStreamer(CvCapture_GStreamer *cap)
{
	if(cap->pipeline) {
		gst_element_set_state(GST_ELEMENT(cap->pipeline), GST_STATE_NULL);
		gst_object_unref(GST_OBJECT(cap->pipeline));
	}

	if(cap->buffer)
		gst_buffer_unref(cap->buffer);

	if(cap->frame)
		cvReleaseImage(&cap->frame);

	if(cap->caps)
		gst_caps_unref(cap->caps);
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
static int icvGrabFrame_GStreamer(CvCapture_GStreamer *cap)
{
	
	if(!cap->pipeline)
		return 0;

	if(gst_app_sink_is_eos(GST_APP_SINK(cap->appsink))) {
		//printf("end of stream\n");
		return 0;
	}

	if(cap->buffer)
		gst_buffer_unref(cap->buffer);
	icvHandleMessage(cap);

#ifndef HAVE_GSTREAMER_APP
	if(gst_app_sink_get_queue_length(GST_APP_SINK(cap->appsink)))
	{
//		printf("peeking buffer, %d buffers in queue\n",
		cap->buffer = gst_app_sink_peek_buffer(GST_APP_SINK(cap->appsink));
	}
	else
#endif
	{
//		printf("no buffers queued, starting pipeline\n");

		if(gst_element_set_state(GST_ELEMENT(cap->pipeline), GST_STATE_PLAYING) ==
		GST_STATE_CHANGE_FAILURE) {
			icvHandleMessage(cap);
			return 0;
		}

		cap->buffer = gst_app_sink_pull_buffer(GST_APP_SINK(cap->appsink));

//		printf("pausing pipeline\n");

		if(gst_element_set_state(GST_ELEMENT(cap->pipeline), GST_STATE_PAUSED) ==
		GST_STATE_CHANGE_FAILURE) {
			icvHandleMessage(cap);
			return 0;
		}

//		printf("pipeline paused\n");
	} 

	if(!cap->buffer)
		return 0;

//	printf("pulled buffer %p\n", cap->buffer);

	return 1;
}

//
// decode buffer
//
static IplImage *icvRetrieveFrame_GStreamer(CvCapture_GStreamer *cap, int)
{
	if(!cap->buffer)
		return 0;

	GstCaps* caps = gst_buffer_get_caps(cap->buffer);

	assert(gst_caps_get_size(caps) == 1);

	GstStructure* structure = gst_caps_get_structure(caps, 0);

	if(!cap->frame) {
		gint height, width;

		if(!gst_structure_get_int(structure, "width", &width) ||
	   !gst_structure_get_int(structure, "height", &height))
			return 0;

     	//printf("creating frame %dx%d\n", width, height);

		cap->frame = cvCreateImage(cvSize(width, height), IPL_DEPTH_8U, 3);
	}
	gst_caps_unref(caps);

	memcpy (cap->frame->imageData, GST_BUFFER_DATA(cap->buffer), GST_BUFFER_SIZE (cap->buffer));
	gst_buffer_unref(cap->buffer);
	cap->buffer = 0;

	return cap->frame;
}

static double icvGetProperty_GStreamer(CvCapture_GStreamer *cap, int id)
{
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
		format = GST_FORMAT_DEFAULT;
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
		format = GST_FORMAT_DEFAULT;
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
	default:
		CV_WARN("GStreamer: unhandled property");
		break;
	}
	return 0;
}

static void icvRestartPipeline(CvCapture_GStreamer *cap)
{
	CV_FUNCNAME("icvRestartPipeline");

	__BEGIN__;

	printf("restarting pipeline, going to ready\n");

	if(gst_element_set_state(GST_ELEMENT(cap->pipeline), GST_STATE_READY) ==
	   GST_STATE_CHANGE_FAILURE) {
		CV_ERROR(CV_StsError, "GStreamer: unable to start pipeline\n");
		return;
	}

	printf("ready, relinking\n");

	gst_element_unlink(cap->source, cap->decodebin);
	printf("filtering with %s\n", gst_caps_to_string(cap->caps));
	gst_element_link_filtered(cap->source, cap->decodebin, cap->caps);

	printf("relinked, pausing\n");

	if(gst_element_set_state(GST_ELEMENT(cap->pipeline), GST_STATE_PAUSED) ==
	   GST_STATE_CHANGE_FAILURE) {
		CV_ERROR(CV_StsError, "GStreamer: unable to start pipeline\n");
		return;
	}

	printf("state now paused\n");

 	__END__;
}

static void icvSetFilter(CvCapture_GStreamer *cap, const char *property, int type, int v1, int v2)
{
	printf("setting cap %p %s %d %d %d\n", cap->caps, property, type, v1, v2);

	if(!cap->caps) {
		if(type == G_TYPE_INT)
			cap->caps = gst_caps_new_simple("video/x-raw-rgb", property, type, v1, NULL);
		else
			cap->caps = gst_caps_new_simple("video/x-raw-rgb", property, type, v1, v2, NULL);
	} else {
		printf("caps before setting %s\n", gst_caps_to_string(cap->caps));
		if(type == G_TYPE_INT)
			gst_caps_set_simple(cap->caps, "video/x-raw-rgb", property, type, v1, NULL);
		else
			gst_caps_set_simple(cap->caps, "video/x-raw-rgb", property, type, v1, v2, NULL);
	}

	icvRestartPipeline(cap);
}

static void icvRemoveFilter(CvCapture_GStreamer *cap, const char *filter)
{
	if(!cap->caps)
		return;

	GstStructure *s = gst_caps_get_structure(cap->caps, 0);
	gst_structure_remove_field(s, filter);

	icvRestartPipeline(cap);
}

static int icvSetProperty_GStreamer(CvCapture_GStreamer *cap, int id, double value)
{
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
		}
		break;
	case CV_CAP_PROP_POS_FRAMES:
		format = GST_FORMAT_DEFAULT;
		flags = (GstSeekFlags) (GST_SEEK_FLAG_FLUSH|GST_SEEK_FLAG_ACCURATE);
		if(!gst_element_seek_simple(GST_ELEMENT(cap->pipeline), format,
					    flags, (gint64) value)) {
			CV_WARN("GStreamer: unable to seek");
		}
		break;
	case CV_CAP_PROP_POS_AVI_RATIO:
		format = GST_FORMAT_PERCENT;
		flags = (GstSeekFlags) (GST_SEEK_FLAG_FLUSH|GST_SEEK_FLAG_ACCURATE);
		if(!gst_element_seek_simple(GST_ELEMENT(cap->pipeline), format,
					    flags, (gint64) (value * GST_FORMAT_PERCENT_MAX))) {
			CV_WARN("GStreamer: unable to seek");
		}
		break;
	case CV_CAP_PROP_FRAME_WIDTH:
		if(value > 0)
			icvSetFilter(cap, "width", G_TYPE_INT, (int) value, 0);
		else
			icvRemoveFilter(cap, "width");
		break;
	case CV_CAP_PROP_FRAME_HEIGHT:
		if(value > 0)
			icvSetFilter(cap, "height", G_TYPE_INT, (int) value, 0);
		else
			icvRemoveFilter(cap, "height");
		break;
	case CV_CAP_PROP_FPS:
		if(value > 0) {
			int num, denom;
			num = (int) value;
			if(value != num) { // FIXME this supports only fractions x/1 and x/2
				num = (int) (value * 2);
				denom = 2;
			} else
				denom = 1;

			icvSetFilter(cap, "framerate", GST_TYPE_FRACTION, num, denom);
		} else
			icvRemoveFilter(cap, "framerate");
		break;
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
	default:
		CV_WARN("GStreamer: unhandled property");
	}
	return 0;
}

//
// connect decodebin's dynamically created source pads to colourconverter
//
static void icvNewPad(GstElement *decodebin, GstPad *pad, gboolean last, gpointer data)
{
	GstElement *sink = GST_ELEMENT(data);
	GstStructure *str;
	GstPad *sinkpad;
	GstCaps *caps;

	/* link only once */
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
//	g_print("new pad %s\n", structname);
	if(!g_strrstr(structname, "video")) {
		gst_caps_unref(caps);
		gst_object_unref(sinkpad);
		return;
	}
	printf("linking pad %s\n", structname);

	/* link'n'play */
	gst_pad_link (pad, sinkpad);

	gst_caps_unref(caps);
	gst_object_unref(sinkpad);
}

static bool icvCreateCapture_GStreamer(CvCapture_GStreamer *cap, int type, const char *filename)
{
	CV_FUNCNAME("cvCaptureFromCAM_GStreamer");

	__BEGIN__;

//	teststreamer(filename);

//	return 0;

	if(!isInited) {
//		printf("gst_init\n");
		gst_init (NULL, NULL);


		isInited = true;
	}
    bool stream=false;
	const char *sourcetypes[] = {"dv1394src", "v4lsrc", "v4l2src", "filesrc"};
	//printf("entered capturecreator %s\n", sourcetypes[type]);
    GstElement *source;
    if  (type == CV_CAP_GSTREAMER_FILE && gst_uri_is_valid(filename)) {
		printf("Trying to connect to stream \n");
		stream=true;
		source = gst_element_make_from_uri(GST_URI_SRC, filename, NULL);
	}
	else	
		source = gst_element_factory_make(sourcetypes[type], NULL);
	if(!source)
		return 0;

	if(type ==CV_CAP_GSTREAMER_FILE && !gst_uri_is_valid(filename))
		g_object_set(G_OBJECT(source), "location", filename, NULL);

	GstElement *colour = gst_element_factory_make("ffmpegcolorspace", NULL);

#ifdef HAVE_GSTREAMER_APP
	GstElement *sink = gst_element_factory_make("appsink", NULL);
	if (stream) {
		gst_app_sink_set_max_buffers (GST_APP_SINK(sink),1);
		gst_app_sink_set_drop (GST_APP_SINK(sink),true);
	}	
#else
	GstElement *sink = gst_element_factory_make("opencv-appsink", NULL);
#endif
    GstCaps *caps= gst_caps_new_simple("video/x-raw-rgb",
									   "red_mask",   G_TYPE_INT, 255,
 	                                   "green_mask", G_TYPE_INT, 65280,
 	                                   "blue_mask",  G_TYPE_INT, 16711680,
                                       NULL);
    //GstCaps *caps=gst_video_format_new_caps(GST_VIDEO_FORMAT_BGR,,368,30,1,1,1);
    gst_app_sink_set_caps(GST_APP_SINK(sink), caps);
	gst_caps_unref(caps);
	//gst_base_sink_set_sync(GST_BASE_SINK(sink), false);
	//g_signal_connect(sink, "new-buffer", G_CALLBACK(newbuffer), NULL);	
	GstElement *decodebin = gst_element_factory_make("decodebin", NULL);
	g_signal_connect(decodebin, "new-decoded-pad", G_CALLBACK(icvNewPad), colour);

	GstElement *pipeline = gst_pipeline_new (NULL);

	gst_bin_add_many(GST_BIN(pipeline), source, decodebin, colour, sink, NULL);

	switch(type) {
	case CV_CAP_GSTREAMER_V4L2: // default to 640x480, 30 fps
		break;
	case CV_CAP_GSTREAMER_V4L:
	case CV_CAP_GSTREAMER_1394:
        break;
	case CV_CAP_GSTREAMER_FILE:
		if(!gst_element_link(source, decodebin)) {
			CV_ERROR(CV_StsError, "GStreamer: cannot link filesrc -> decodebin\n");
			gst_object_unref(pipeline);
			return 0;
		}
		break;
	}
	if(!gst_element_link(colour, sink)) {
		CV_ERROR(CV_StsError, "GStreamer: cannot link colour -> sink\n");
		gst_object_unref(pipeline);
		return 0;
	}

//	printf("linked, pausing\n");

	if(gst_element_set_state(GST_ELEMENT(pipeline), GST_STATE_READY) ==
	   GST_STATE_CHANGE_FAILURE) {
		CV_WARN("GStreamer: unable to set pipeline to paused\n");
//		icvHandleMessage(capture);
//		cvReleaseCapture((CvCapture **)(void *)&capture);
		gst_object_unref(pipeline);
		return 0;
	}


	if(gst_element_set_state(GST_ELEMENT(pipeline), GST_STATE_PAUSED) ==
	   GST_STATE_CHANGE_FAILURE) {
		CV_WARN("GStreamer: unable to set pipeline to paused\n");
//		icvHandleMessage(capture);
//		cvReleaseCapture((CvCapture **)(void *)&capture);
		gst_object_unref(pipeline);
		return 0;
	}


//	printf("state now paused\n");
	// construct capture struct
	//capture->type = type;
	cap->pipeline = pipeline;
	cap->source = source;
	cap->decodebin = decodebin;
	cap->colour = colour;
	cap->appsink = sink;

	icvHandleMessage(cap);

//	GstClock *clock = gst_pipeline_get_clock(GST_PIPELINE(pipeline));
//	printf("clock %s\n", gst_object_get_name(GST_OBJECT(clock)));

	__END__;

	return true;
}
bool CvCapture_GStreamer::open( int type, const char* filename )
{
    close();
    icvCreateCapture_GStreamer(this, type, filename );
    return true;
}

#ifdef HAVE_GSTREAMER_APP

//
//
// gstreamer image sequence writer
//
//
class CvVideoWriter_GStreamer : public CvVideoWriter
{
public:
    CvVideoWriter_GStreamer() { init(); }
    virtual ~CvVideoWriter_GStreamer() { close(); }

    virtual bool open( const char* filename, int fourcc,
                       double fps, CvSize frameSize, bool isColor );
    virtual void close();
    virtual bool writeFrame( const IplImage* image ); 
protected:
    void init();	
    GstElement* source;
    GstElement* file;
    GstElement* enc;
    GstElement* mux;
    GstElement* colour;
    GstBuffer* buffer;
    GstElement* pipeline;
    int input_pix_fmt;
};
void CvVideoWriter_GStreamer::init()
{
	pipeline=0;
}
void CvVideoWriter_GStreamer::close()
{
  if (pipeline) {	
	gst_app_src_end_of_stream(GST_APP_SRC(source));
	gst_element_set_state (pipeline, GST_STATE_NULL);
	gst_object_unref (GST_OBJECT (pipeline));
  }
}
bool CvVideoWriter_GStreamer::open( const char * filename, int fourcc,
		double fps, CvSize frameSize, bool is_color )
{
    //actually doesn't support fourcc parameter and encode an avi with jpegenc
    //we need to find a common api between backend to support fourcc for avi 
    //but also to choose in a common way codec and container format (ogg,dirac,matroska)
    CV_FUNCNAME("CvVideoWriter_GStreamer::open");
	
    __BEGIN__;
	// check arguments
	assert (filename);
	assert (fps > 0);
	assert (frameSize.width > 0  &&  frameSize.height > 0);
    if(!isInited) {
		gst_init (NULL, NULL);
		isInited = true;
	}
	close();
    source=gst_element_factory_make("appsrc",NULL);
    file=gst_element_factory_make("filesink", NULL);
    enc=gst_element_factory_make("jpegenc", NULL);
    mux=gst_element_factory_make("avimux", NULL);
    colour = gst_element_factory_make("ffmpegcolorspace", NULL);
    g_object_set(G_OBJECT(file), "location", filename, NULL);
    pipeline = gst_pipeline_new (NULL);
    GstCaps* caps;
    if (is_color) {
        input_pix_fmt=1;
		caps= gst_video_format_new_caps(GST_VIDEO_FORMAT_BGR,
												 frameSize.width,
												 frameSize.height,
												 fps,
												 1,
												 1,
												 1); 	
	}								 
	else  {
		input_pix_fmt=0;
		caps= gst_caps_new_simple("video/x-raw-gray",
								  "width", G_TYPE_INT, frameSize.width,
								  "height", G_TYPE_INT, frameSize.height,
								  "framerate", GST_TYPE_FRACTION, int(fps),1,
								  "bpp",G_TYPE_INT,8, 
								  "depth",G_TYPE_INT,8,
                                           NULL);  
    }                                                                          
	gst_app_src_set_caps(GST_APP_SRC(source), caps);	
	gst_bin_add_many(GST_BIN(pipeline), source, colour,mux, file, NULL);
	gst_element_link_many(source,colour,mux,file,NULL);
	if(gst_element_set_state(GST_ELEMENT(pipeline), GST_STATE_PLAYING) ==
		GST_STATE_CHANGE_FAILURE) {
			printf("Cannot put pipeline to play\n");
			return 0;
		}
    __END__;

	return true;
}	
bool CvVideoWriter_GStreamer::writeFrame( const IplImage * image )
{
    
    CV_FUNCNAME("CvVideoWriter_GStreamer::writerFrame");

	__BEGIN__;
	if (input_pix_fmt == 1) {
        if (image->nChannels != 3 || image->depth != IPL_DEPTH_8U) {
            CV_ERROR(CV_StsUnsupportedFormat, "cvWriteFrame() needs images with depth = IPL_DEPTH_8U and nChannels = 3.");
        }
    }
	else if (input_pix_fmt == 0) {
        if (image->nChannels != 1 || image->depth != IPL_DEPTH_8U) {
            CV_ERROR(CV_StsUnsupportedFormat, "cvWriteFrame() needs images with depth = IPL_DEPTH_8U and nChannels = 1.");
        }
    }
	else {
        assert(false);
    }
    int size;
    size = image->imageSize;
    buffer = gst_buffer_new_and_alloc (size); 
    //gst_buffer_set_data (buffer,(guint8*)image->imageData, GST_BUFFER_SIZE(sizeof(image->imageData)));
    memcpy (GST_BUFFER_DATA(buffer),image->imageData, size); 
    GstFlowReturn flow_ret;
    flow_ret=gst_app_src_push_buffer(GST_APP_SRC(source),buffer);
    //gst_buffer_unref(buffer);
	//buffer = 0;
    __END__;
	return true;
}


CvVideoWriter* cvCreateVideoWriter_GStreamer(const char* filename, int fourcc, double fps,
                                           CvSize frameSize, int isColor )
{
    CvVideoWriter_GStreamer* wrt = new CvVideoWriter_GStreamer;
    if( wrt->open(filename, fourcc, fps,
                                           frameSize, isColor))
        return wrt;

    delete wrt;
    return 0;
}

#else

CvVideoWriter* cvCreateVideoWriter_GStreamer(const char*, int, double, CvSize, int )
{
    return 0;
}

#endif

/*static void icvReleaseVideoWriter_GStreamer( CvVideoWriter** writer )
{
	//CvVideoWriter_GStreamer **wri = (CvVideoWriter_GStreamer **)writer;

	//free(this->filename);
}*/


void CvCapture_GStreamer::close()
{
  if (pipeline)	
	icvClose_GStreamer( this );

}

bool CvCapture_GStreamer::grabFrame()
{
    return this ? icvGrabFrame_GStreamer( this ) != 0 : false;
}

IplImage* CvCapture_GStreamer::retrieveFrame(int)
{
    return this ? (IplImage*)icvRetrieveFrame_GStreamer( this, 0 ) : 0;
 
}

double CvCapture_GStreamer::getProperty( int propId )
{
    return this ? icvGetProperty_GStreamer( this, propId ) : 0;
}

bool CvCapture_GStreamer::setProperty( int propId, double value )
{
    return this ? icvSetProperty_GStreamer( this, propId, value ) != 0 : false;
}

CvCapture* cvCreateCapture_GStreamer(int type, const char* filename )
{
    CvCapture_GStreamer* capture = new CvCapture_GStreamer;

    if( capture->open( type, filename ))
        return capture;

    delete capture;
    return 0;
}
