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
// Copyright (C) 2008, Xavier Delacour, all rights reserved.
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

// 2008-04-27 Xavier Delacour <xavier.delacour@gmail.com>

#include <_highgui.h>
#include <unistd.h>
#include <unicap.h>
extern "C" {
#include <ucil.h>
}

#ifdef NDEBUG
#define CV_WARN(message)
#else
#define CV_WARN(message) fprintf(stderr, "warning: %s (%s:%d)\n", message, __FILE__, __LINE__)
#endif

typedef struct CvCapture_Unicap
{
  CvCaptureVTable *vtable;

  bool device_initialized;

  int desired_device;
  int desired_format;
  CvSize desired_size;
  bool convert_rgb;

  unicap_handle_t handle;
  unicap_device_t device;
  unicap_format_t format_spec;
  unicap_format_t format;
  unicap_data_buffer_t raw_buffer;
  unicap_data_buffer_t buffer;

  IplImage *raw_frame;
  IplImage *frame;
} CvCapture_Unicap;

static bool icvShutdownDevice(CvCapture_Unicap *cap) {
  CV_FUNCNAME("icvShutdownDevice");
  __BEGIN__;

  if (!SUCCESS(unicap_stop_capture(cap->handle)))
    CV_ERROR(CV_StsError, "unicap: failed to stop capture on device\n");
  
  if (!SUCCESS(unicap_close(cap->handle)))
    CV_ERROR(CV_StsError, "unicap: failed to close the device\n");

  cvReleaseImage(&cap->raw_frame);
  cvReleaseImage(&cap->frame);

  cap->device_initialized = false;

  return true;
  __END__;
  return false;
}

static bool icvInitDevice(CvCapture_Unicap *cap) {
  CV_FUNCNAME("icvInitDevice");
  __BEGIN__;

  if (cap->device_initialized && !icvShutdownDevice(cap))
    return false;

  if(!SUCCESS(unicap_enumerate_devices(NULL, &cap->device, cap->desired_device)))
    CV_ERROR(CV_StsError, "unicap: failed to get info for device\n");

  if(!SUCCESS(unicap_open( &cap->handle, &cap->device)))
    CV_ERROR(CV_StsError, "unicap: failed to open device\n");

  unicap_void_format(&cap->format_spec);

  if (!SUCCESS(unicap_enumerate_formats(cap->handle, &cap->format_spec, &cap->format, cap->desired_format))) {
    icvShutdownDevice(cap);
    CV_ERROR(CV_StsError, "unicap: failed to get video format\n");
  }

  int i;
  for (i = cap->format.size_count - 1; i > 0; i--)
    if (cap->format.sizes[i].width == cap->desired_size.width && 
	cap->format.sizes[i].height == cap->desired_size.height)
      break;
  cap->format.size.width = cap->format.sizes[i].width;
  cap->format.size.height = cap->format.sizes[i].height;

  if (!SUCCESS(unicap_set_format(cap->handle, &cap->format))) {
    icvShutdownDevice(cap);
    CV_ERROR(CV_StsError, "unicap: failed to set video format\n");
  }

  memset(&cap->raw_buffer, 0x0, sizeof(unicap_data_buffer_t));
  cap->raw_frame = cvCreateImage(cvSize(cap->format.size.width, 
					cap->format.size.height), 
				  8, cap->format.bpp / 8);
  memcpy(&cap->raw_buffer.format, &cap->format, sizeof(cap->raw_buffer.format));
  cap->raw_buffer.data = (unsigned char*)cap->raw_frame->imageData;
  cap->raw_buffer.buffer_size = cap->format.size.width * 
    cap->format.size.height * cap->format.bpp / 8;

  memset(&cap->buffer, 0x0, sizeof(unicap_data_buffer_t));
  memcpy(&cap->buffer.format, &cap->format, sizeof(cap->buffer.format));

  cap->buffer.format.fourcc = UCIL_FOURCC('B','G','R','3');
  cap->buffer.format.bpp = 24;
  // * todo support greyscale output
  //    cap->buffer.format.fourcc = UCIL_FOURCC('G','R','E','Y');
  //    cap->buffer.format.bpp = 8;

  cap->frame = cvCreateImage(cvSize(cap->buffer.format.size.width, 
				    cap->buffer.format.size.height), 
			      8, cap->buffer.format.bpp / 8);
  cap->buffer.data = (unsigned char*)cap->frame->imageData;
  cap->buffer.buffer_size = cap->buffer.format.size.width * 
    cap->buffer.format.size.height * cap->buffer.format.bpp / 8;

  if(!SUCCESS(unicap_start_capture(cap->handle))) {
    icvShutdownDevice(cap);
    CV_ERROR(CV_StsError, "unicap: failed to start capture on device\n");
  }

  cap->device_initialized = true;
  return true;
  __END__;
  return false;
}

static void icvClose_Unicap(CvCapture_Unicap *cap) {
  icvShutdownDevice(cap);
  cvFree(&cap);
}

static int icvGrabFrame_Unicap(CvCapture_Unicap *cap) {
  CV_FUNCNAME("icvGrabFrame_Unicap");
  __BEGIN__;

  unicap_data_buffer_t *returned_buffer;

  int retry_count = 100;

  while (retry_count--) {
    if(!SUCCESS(unicap_queue_buffer(cap->handle, &cap->raw_buffer)))
      CV_ERROR(CV_StsError, "unicap: failed to queue a buffer on device\n");

    if(SUCCESS(unicap_wait_buffer(cap->handle, &returned_buffer)))
      return 1;
    
    CV_WARN("unicap: failed to wait for buffer on device\n");
    usleep(100 * 1000);
  }

  __END__;
  return 0;
}

static IplImage *icvRetrieveFrame_Unicap(CvCapture_Unicap *cap) {
  if (cap->convert_rgb) {
    ucil_convert_buffer(&cap->buffer, &cap->raw_buffer);
    return cap->frame;
  }
  return cap->raw_frame;
}

static double icvGetProperty_Unicap(CvCapture_Unicap *cap, int id) {
  switch (id) {
  case CV_CAP_PROP_POS_MSEC: break;
  case CV_CAP_PROP_POS_FRAMES: break;
  case CV_CAP_PROP_POS_AVI_RATIO: break;
  case CV_CAP_PROP_FRAME_WIDTH:
    return cap->desired_size.width;
  case CV_CAP_PROP_FRAME_HEIGHT:
    return cap->desired_size.height;
  case CV_CAP_PROP_FPS: break;
  case CV_CAP_PROP_FOURCC: break;
  case CV_CAP_PROP_FRAME_COUNT: break;
  case CV_CAP_PROP_FORMAT:
    return cap->desired_format;
  case CV_CAP_PROP_MODE: break;
  case CV_CAP_PROP_BRIGHTNESS: break;
  case CV_CAP_PROP_CONTRAST: break;
  case CV_CAP_PROP_SATURATION: break;
  case CV_CAP_PROP_HUE: break;
  case CV_CAP_PROP_GAIN: break;
  case CV_CAP_PROP_CONVERT_RGB:
    return cap->convert_rgb;
  }

  return 0;
}

static int icvSetProperty_Unicap(CvCapture_Unicap *cap, int id, double value) {
  bool reinit = false;

  switch (id) {
  case CV_CAP_PROP_POS_MSEC: break;
  case CV_CAP_PROP_POS_FRAMES: break;
  case CV_CAP_PROP_POS_AVI_RATIO: break;
  case CV_CAP_PROP_FRAME_WIDTH:
    cap->desired_size.width = (int)value;
    reinit = true;
    break;
  case CV_CAP_PROP_FRAME_HEIGHT:
    cap->desired_size.height = (int)value;
    reinit = true;
    break;
  case CV_CAP_PROP_FPS: break;
  case CV_CAP_PROP_FOURCC: break;
  case CV_CAP_PROP_FRAME_COUNT: break;
  case CV_CAP_PROP_FORMAT:
    cap->desired_format = id;
    reinit = true;
    break;
  case CV_CAP_PROP_MODE: break;
  case CV_CAP_PROP_BRIGHTNESS: break;
  case CV_CAP_PROP_CONTRAST: break;
  case CV_CAP_PROP_SATURATION: break;
  case CV_CAP_PROP_HUE: break;
  case CV_CAP_PROP_GAIN: break;
  case CV_CAP_PROP_CONVERT_RGB:
    cap->convert_rgb = value != 0;
    break;
  }

  if (reinit && !icvInitDevice(cap))
    return 0;

  return 1;
}

static CvCaptureVTable capture_vtable =
  {
    6,
    ( CvCaptureCloseFunc ) icvClose_Unicap,
    ( CvCaptureGrabFrameFunc ) icvGrabFrame_Unicap,
    ( CvCaptureRetrieveFrameFunc ) icvRetrieveFrame_Unicap,
    ( CvCaptureGetPropertyFunc ) icvGetProperty_Unicap,
    ( CvCaptureSetPropertyFunc ) icvSetProperty_Unicap,
    ( CvCaptureGetDescriptionFunc ) 0
  };


CvCapture * cvCaptureFromCAM_Unicap (const int index) {
  CvCapture_Unicap *cap = 0;

  cap = (CvCapture_Unicap *)cvAlloc(sizeof(CvCapture_Unicap));
  memset(cap, 0, sizeof(CvCapture_Unicap));

  cap->device_initialized = false;

  cap->desired_device = index < 0 ? 0 : index;
  cap->desired_format = 0;
  cap->desired_size = cvSize(320, 240);
  cap->convert_rgb = true;
  
  if (!icvInitDevice(cap)) {
    cvFree(&cap);
    return 0;
  }

  cap->vtable = &capture_vtable;

  return (CvCapture *)cap;
}
