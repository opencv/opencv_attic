/* This is the contributed code:
Firewire and video4linux camera support for highgui

2003-03-12  Magnus Lundin
lundin@mlu.mine.nu

THIS EXEPERIMENTAL CODE
Tested on 2.4.19 with 1394, video1394, v4l, dc1394 and raw1394 support

This set of files adds support for firevre and usb cameras.
First it tries to install a firewire camera, 
if that fails it tries a v4l/USB camera

It has been tested with the motempl sample program

INSTALLATION
Install OpenCV
Install v4l
Install dc1394 raw1394 - coriander should work with your camera
    Backup highgui folder
    Copy new files
    cd into highgui folder
    make clean  (cvcap.cpp must be rebuilt)
    make
    make install


The build is controlled by the following entries in the highgui Makefile:

libhighgui_la_LIBADD = -L/usr/X11R6/lib -lXm -lMrm -lUil -lpng  -ljpeg -lz -ltiff -lavcodec -lraw1394 -ldc1394_control
DEFS = -DHAVE_CONFIG_H -DHAVE_DC1394 HAVE_CAMV4L


Now it should be possible to use highgui camera functions, works for me.


THINGS TO DO
Better ways to select 1394 or v4l camera
Better support for videosize
Support for yuv411
Integration into OpenCV config and build structure

Comments and changes welcome
/Magnus
*/


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

#if !defined WIN32 && defined HAVE_DC1394

#include <unistd.h>
#include <stdint.h>
#include <libraw1394/raw1394.h>
#include <libdc1394/dc1394_control.h>

#define  DELAY              50000

// bpp for 16-bits cameras... this value works for PtGrey DragonFly...
#define MONO16_BPP 8

/* should be in pixelformat */
static void uyv2bgr(const unsigned char *src, unsigned char *dest, unsigned long long int NumPixels);
static void uyvy2bgr(const unsigned char *src, unsigned char *dest, unsigned long long int NumPixels);
static void uyyvyy2bgr(const unsigned char *src, unsigned char *dest, unsigned long long int NumPixels);
static void y2bgr(const unsigned char *src, unsigned char *dest, unsigned long long int NumPixels);
static void y162bgr(const unsigned char *src, unsigned char *dest, unsigned long long int NumPixels, int bits);
static void rgb482bgr(const unsigned char *src8, unsigned char *dest, unsigned long long int NumPixels, int bits);

static char * videodev[4]={
  "/dev/video1394/0",
  "/dev/video1394/1",
  "/dev/video1394/2",
  "/dev/video1394/3"
};

typedef struct CvCaptureCAM_DC1394
{
    CvCaptureVTable* vtable;
    raw1394handle_t handle;
    nodeid_t  camera_node;
    dc1394_cameracapture* camera;
    int format;
    int mode;
    int color_mode;
    int frame_rate;
    char * device_name;
    IplImage* rgb_frame;
    IplImage frame;
}
CvCaptureCAM_DC1394;

static void icvCloseCAM_DC1394( CvCaptureCAM_DC1394* capture );

static int icvGrabFrameCAM_DC1394( CvCaptureCAM_DC1394* capture );
static IplImage* icvRetrieveFrameCAM_DC1394( CvCaptureCAM_DC1394* capture );

static double icvGetPropertyCAM_DC1394( CvCaptureCAM_DC1394* capture, int property_id );
static int    icvSetPropertyCAM_DC1394( CvCaptureCAM_DC1394* capture, int property_id, double value );

/***********************   Implementations  ***************************************/
#define MAX_PORTS 3 
#define MAX_CAMERAS 8
#define NUM_BUFFERS 8
struct raw1394_portinfo ports[MAX_PORTS];
static raw1394handle_t handles[MAX_PORTS];
static int camCount[MAX_PORTS];
static int numPorts = -1;
static int numCameras = 0;
static nodeid_t *camera_nodes;
struct camnode {dc1394_cameracapture cam;int portnum;} cameras[MAX_CAMERAS];

static const int preferred_modes[]
= {
    // uncomment the following line to test a particular mode:
    //FORMAT_VGA_NONCOMPRESSED, MODE_640x480_MONO16, 0,
    FORMAT_SVGA_NONCOMPRESSED_2,
    MODE_1600x1200_RGB, MODE_1600x1200_YUV422, MODE_1280x960_RGB, MODE_1280x960_YUV422,
    MODE_1600x1200_MONO, MODE_1280x960_MONO, MODE_1600x1200_MONO16, MODE_1280x960_MONO16,
    FORMAT_SVGA_NONCOMPRESSED_1,
    MODE_1024x768_RGB, MODE_1024x768_YUV422, MODE_800x600_RGB, MODE_800x600_YUV422,
    MODE_1024x768_MONO, MODE_800x600_MONO, MODE_1024x768_MONO16, MODE_800x600_MONO16, 
    FORMAT_VGA_NONCOMPRESSED,
   MODE_640x480_RGB, MODE_640x480_YUV422, MODE_640x480_YUV411, MODE_320x240_YUV422,
    MODE_160x120_YUV444, MODE_640x480_MONO, MODE_640x480_MONO16,
    FORMAT_SCALABLE_IMAGE_SIZE,
    MODE_FORMAT7_0, MODE_FORMAT7_1, MODE_FORMAT7_2, MODE_FORMAT7_3,
    MODE_FORMAT7_4, MODE_FORMAT7_5, MODE_FORMAT7_6, MODE_FORMAT7_7,
    0};

static CvCaptureVTable captureCAM_DC1394_vtable = 
{
    6,
    (CvCaptureCloseFunc)icvCloseCAM_DC1394,
    (CvCaptureGrabFrameFunc)icvGrabFrameCAM_DC1394,
    (CvCaptureRetrieveFrameFunc)icvRetrieveFrameCAM_DC1394,
    (CvCaptureGetPropertyFunc)icvGetPropertyCAM_DC1394,
    (CvCaptureSetPropertyFunc)icvSetPropertyCAM_DC1394,
    (CvCaptureGetDescriptionFunc)0
};

void icvInitCapture_DC1394(){
   int p;
   
   raw1394handle_t raw_handle = raw1394_new_handle();
   if( raw_handle == 0 ) {
      numPorts = 0;
      return;
   }
   numPorts = raw1394_get_port_info(raw_handle, ports, MAX_PORTS);
   raw1394_destroy_handle(raw_handle);
   for (p = 0; p < numPorts; p++) {
      handles[p] = dc1394_create_handle(p);
      if (handles[p]==NULL) {  numPorts=-1; return; /*ERROR_CLEANUP_EXIT*/   }

      /* get the camera nodes and describe them as we find them */
      camera_nodes = dc1394_get_camera_nodes(handles[p], &camCount[p], 0);
      for (int i=0;i<camCount[p];i++) {
         cameras[numCameras].cam.node = camera_nodes[i];
         cameras[numCameras].portnum = p;
         dc1394_stop_iso_transmission(handles[p], camera_nodes[i]);
         numCameras++;
      }
   }
};

CvCapture* icvOpenCAM_DC1394( int index ){
    quadlet_t framerates, modes[8], formats;
    int i;

  if (numPorts<0) icvInitCapture_DC1394();
   if (numPorts==0)
     return 0;     /* No i1394 ports found */
   if (numCameras<1)
     return 0;
   if (index>=numCameras)
     return 0;
   if (index<0)
     return 0;

   CvCaptureCAM_DC1394 * pcap = (CvCaptureCAM_DC1394*)cvAlloc(sizeof(CvCaptureCAM_DC1394));
   pcap->vtable = &captureCAM_DC1394_vtable;

   /* Select a port and camera */
   pcap->device_name = videodev[cameras[index].portnum];
   pcap->handle = handles[cameras[index].portnum];
   pcap->camera = &cameras[index].cam;
   
   // get supported formats
   if (dc1394_query_supported_formats(pcap->handle, pcap->camera->node, &formats)<0) {
       fprintf(stderr,"%s:%d: Could not query supported formats\n",__FILE__,__LINE__);
       formats=0x0;
   }
   for (i=0; i < NUM_FORMATS; i++) {
       modes[i]=0;
       if (formats & (0x1<<(31-i))) 
      if (dc1394_query_supported_modes(pcap->handle, pcap->camera->node, i+FORMAT_MIN, &modes[i])<0) {
          fprintf(stderr,"%s:%d: Could not query Format%d modes\n",__FILE__,__LINE__,i);
      }
   }

   pcap->format = 0;
   pcap->mode = 0;
   pcap->color_mode = 0;
   pcap->frame_rate = 0;

   int format = -1;
   int format_min = 0;

   // scan the list of preferred modes, and find a supported one
   for(i=0; (pcap->mode == 0) && (preferred_modes[i] != 0); i++) {
       if((preferred_modes[i] >= FORMAT_MIN) && (preferred_modes[i] <= FORMAT_MAX)) {
      pcap->format = preferred_modes[i];
      format = preferred_modes[i] - FORMAT_MIN;
      format_min = MODE_FORMAT0_MIN + 32*format;
      continue;
       }
       assert(format != -1);
       if ( !(formats & (0x1<<(31-format))) )
      continue;
       if (modes[format] & (0x1<<(31-(preferred_modes[i]-format_min)))) {
      pcap->mode = preferred_modes[i];
       }
   }
   if (pcap->mode == 0) {
       fprintf(stderr,"%s:%d: Could not find a supported mode for this camera\n",__FILE__,__LINE__);
       goto ERROR;
   }

   float bpp;
   bpp = -1;
   switch(pcap->mode) {
   case MODE_160x120_YUV444:
       pcap->color_mode=COLOR_FORMAT7_YUV444;
       bpp=3;
       break;
   case MODE_320x240_YUV422:
   case MODE_640x480_YUV422:
   case MODE_800x600_YUV422:
   case MODE_1024x768_YUV422:
   case MODE_1280x960_YUV422:
   case MODE_1600x1200_YUV422:
       pcap->color_mode=COLOR_FORMAT7_YUV422;
       bpp=2;
       break;
   case MODE_640x480_YUV411:
       pcap->color_mode=COLOR_FORMAT7_YUV411;
       bpp=1.5;
       break;
   case MODE_640x480_RGB:
   case MODE_800x600_RGB:
   case MODE_1024x768_RGB:
   case MODE_1280x960_RGB:
   case MODE_1600x1200_RGB:
       pcap->color_mode=COLOR_FORMAT7_RGB8;
       bpp=3;
       break;
   case MODE_640x480_MONO:
   case MODE_800x600_MONO:
   case MODE_1024x768_MONO:
   case MODE_1280x960_MONO:
   case MODE_1600x1200_MONO:
       pcap->color_mode=COLOR_FORMAT7_MONO8;
       bpp=1;
       break;
   case MODE_640x480_MONO16:
   case MODE_800x600_MONO16:
   case MODE_1024x768_MONO16:
   case MODE_1280x960_MONO16:
   case MODE_1600x1200_MONO16:
       pcap->color_mode=COLOR_FORMAT7_MONO16;
       bpp=2;
       break;
   case MODE_FORMAT7_0:
   case MODE_FORMAT7_1:
   case MODE_FORMAT7_2:
   case MODE_FORMAT7_3:
   case MODE_FORMAT7_4:
   case MODE_FORMAT7_5:
   case MODE_FORMAT7_6:
   case MODE_FORMAT7_7:
       fprintf(stderr,"%s:%d: Format7 not yet supported\n",__FILE__,__LINE__);
       goto ERROR;
       break;
   }
   if (bpp==-1) {
       fprintf(stderr,"%s:%d: ERROR: BPP is -1!!\n",__FILE__,__LINE__);
       goto ERROR;
   }
   
   if (dc1394_query_supported_framerates(pcap->handle, pcap->camera->node, pcap->format, pcap->mode, &framerates)!=DC1394_SUCCESS) {
       fprintf(stderr,"%s:%d: Could not query supported framerates\n",__FILE__,__LINE__);
       framerates = 0;
   }
   for (int f=FRAMERATE_MAX; f>=FRAMERATE_MIN; f--) {
       if (framerates & (0x1<< (31-(f-FRAMERATE_MIN)))) {
           pcap->frame_rate = f;
           f = FRAMERATE_MIN;
       }
   }

   if (pcap->format!=FORMAT_SCALABLE_IMAGE_SIZE) { // everything except Format 7
       if (dc1394_dma_setup_capture(pcap->handle, pcap->camera->node, index+1 /*channel*/,
                                    pcap->format, pcap->mode, SPEED_400, 
                                    pcap->frame_rate, NUM_BUFFERS,
                                 #ifdef HAVE_DC1394_095
                                    0 /*do_extra_buffering*/,
                                 #endif
                                    1 /*DROP_FRAMES*/,
                                    pcap->device_name, pcap->camera) != DC1394_SUCCESS) {
           fprintf(stderr,"%s:%d: Failed to setup DMA capture with VIDEO1394\n",__FILE__,__LINE__);
           goto ERROR;
       }
   }
   else {
       if(dc1394_dma_setup_format7_capture(pcap->handle,pcap->camera->node,index+1 /*channel*/,
                                    pcap->mode, SPEED_400, QUERY_FROM_CAMERA,
                                    (unsigned int)QUERY_FROM_CAMERA, (unsigned int)QUERY_FROM_CAMERA,
                                    (unsigned int)QUERY_FROM_CAMERA, (unsigned int)QUERY_FROM_CAMERA,
                                    NUM_BUFFERS,
                                #ifdef HAVE_DC1394_095
                                    0 /*do_extra_buffering*/,
                                #endif
                                    1 /*DROP_FRAMES*/,
                                    pcap->device_name, pcap->camera) != DC1394_SUCCESS) {
           fprintf(stderr,"%s:%d: Failed to setup DMA capture with VIDEO1394\n",__FILE__,__LINE__);
           goto ERROR;
       }
   }
     
   if (dc1394_start_iso_transmission(pcap->handle, pcap->camera->node)!=DC1394_SUCCESS) {
       fprintf(stderr,"%s:%d: Could not start ISO transmission\n",__FILE__,__LINE__);
       goto ERROR;
   }

   usleep(DELAY);

   dc1394bool_t status;
   if (dc1394_get_iso_status(pcap->handle, pcap->camera->node, &status)!=DC1394_SUCCESS) {
       fprintf(stderr,"%s:%d: Could get ISO status",__FILE__,__LINE__);
       goto ERROR;
   }
   if (status==DC1394_FALSE) {
       fprintf(stderr,"%s:%d: ISO transmission refuses to start",__FILE__,__LINE__);
       goto ERROR;
   }

   cvInitImageHeader( &pcap->frame,cvSize( pcap->camera->frame_width,pcap->camera->frame_height ),
                           IPL_DEPTH_8U, 3, IPL_ORIGIN_TL, 4 );
   /* Allocate space for RGBA data */ 
   pcap->frame.imageData = (char *)cvAlloc(pcap->frame.imageSize);
   return (CvCapture *)pcap;

  ERROR:
   return 0;  
};

static void icvCloseCAM_DC1394( CvCaptureCAM_DC1394* capture ){
   dc1394_stop_iso_transmission(capture->handle, capture->camera->node);
   /* Deallocate space for RGBA data */ 
   cvFree((void**)&capture->frame.imageData);
};

static int icvGrabFrameCAM_DC1394( CvCaptureCAM_DC1394* capture ){
  //  return dc1394_dma_multi_capture(dc1394_cameracapture *cams,int num);
  return dc1394_dma_single_capture(capture->camera);
};

static IplImage* icvRetrieveFrameCAM_DC1394( CvCaptureCAM_DC1394* capture ){
    if(capture->camera->capture_buffer )
    {
        /* Convert to RGBA */
        /*  Convert(capture->mode,(unsigned char *) cameras[i].capture_buffer, 
            capture->frame.imageData ,capture->camera.width, capture->camera.height) */
        unsigned char * src = (unsigned char *)capture->camera->capture_buffer;
        unsigned char * dst = (unsigned char *)capture->frame.imageData;
        switch (capture->color_mode) {
   case COLOR_FORMAT7_RGB8:
     /* Convert RGB to BGR */
     for (int i=0;i<capture->frame.imageSize;i+=6) {
      dst[i]   = src[i+2];
      dst[i+1] = src[i+1];
      dst[i+2] = src[i];
      dst[i+3] = src[i+5];
      dst[i+4] = src[i+4];
      dst[i+5] = src[i+3];
     }
     break;
      case COLOR_FORMAT7_YUV422:
     uyvy2bgr(src,
       dst,
       capture->frame.width * capture->frame.height);
     break;
   case COLOR_FORMAT7_MONO8:
     y2bgr(src,
      dst,
      capture->frame.width * capture->frame.height);
     break;
   case COLOR_FORMAT7_YUV411:
     uyyvyy2bgr(src,
          dst,
          capture->frame.width * capture->frame.height);
     break;
   case COLOR_FORMAT7_YUV444:
     uyv2bgr(src,
       dst,
       capture->frame.width * capture->frame.height);
     break;
   case COLOR_FORMAT7_MONO16:
     y162bgr(src,
       dst,
       capture->frame.width * capture->frame.height, MONO16_BPP);
     break;
   case COLOR_FORMAT7_RGB16:
     rgb482bgr(src,
          dst,
          capture->frame.width * capture->frame.height, MONO16_BPP);
     break;
   default:
     fprintf(stderr,"%s:%d: Unsupported color mode %d\n",__FILE__,__LINE__,capture->color_mode);
     return 0;
      } /* switch (capture->mode) */
   dc1394_dma_done_with_buffer(capture->camera);
   return &capture->frame;
   }
   return 0;
};

static double icvGetPropertyCAM_DC1394( CvCaptureCAM_DC1394* capture, int property_id ){
   switch ( property_id ) {
   case CV_CAP_PROP_FPS:
      dc1394_get_video_framerate(capture->handle, capture->camera->node,
            (unsigned int *) &capture->camera->frame_rate);
      switch(capture->camera->frame_rate) {
      case FRAMERATE_1_875:
     return 1.875;
      case FRAMERATE_3_75:
     return 3.75;
      case FRAMERATE_7_5:
     return 7.5;
      case FRAMERATE_15:
     return 15.;
      case FRAMERATE_30:
     return 30.;
      case FRAMERATE_60:
     return 60;
#if NUM_FRAMERATES > 6
      case FRAMERATE_120:
     return 120;
#endif
#if NUM_FRAMERATES > 7
      case FRAMERATE_240:
     return 240;
#endif
      }
   }
   return 0;
};

static int    
icvSetPropertyCAM_DC1394( CvCaptureCAM_DC1394* capture, int property_id, double value ){
   switch ( property_id ) {
   case CV_CAP_PROP_FPS:
   unsigned int fps=15;
   if(capture->format == FORMAT_SCALABLE_IMAGE_SIZE)
     break; /* format 7 has no fixed framerates */
   if (value==1.875)
     fps=FRAMERATE_1_875;
   else if (value==3.75)
     fps=FRAMERATE_3_75;
   else if (value==7.5)
     fps=FRAMERATE_7_5;
   else if (value==15)
     fps=FRAMERATE_15;
   else if (value==30)
     fps=FRAMERATE_30;
   else if (value==60)
     fps=FRAMERATE_60;
#if NUM_FRAMERATES > 6
   else if (value==120)
     fps=FRAMERATE_120;
#endif
#if NUM_FRAMERATES > 7
   else if (value==240)
     fps=FRAMERATE_240;
#endif
   dc1394_set_video_framerate(capture->handle, capture->camera->node,fps);
   dc1394_get_video_framerate(capture->handle, capture->camera->node,
              (unsigned int *) &capture->camera->frame_rate);
   break;
   }
   return 0;
};

/**********************************************************************
*
*  CONVERSION FUNCTIONS TO RGB 24bpp 
*
**********************************************************************/

/* color conversion functions from Bart Nabbe. *//* corrected by Damien: bad coeficients in YUV2RGB */
#define YUV2RGB(y, u, v, r, g, b)\
r = y + ((v*1436) >> 10);\
g = y - ((u*352 + v*731) >> 10);\
b = y + ((u*1814) >> 10);\
r = r < 0 ? 0 : r;\
g = g < 0 ? 0 : g;\
b = b < 0 ? 0 : b;\
r = r > 255 ? 255 : r;\
g = g > 255 ? 255 : g;\
b = b > 255 ? 255 : b

static void
uyv2bgr(const unsigned char *src, unsigned char *dest,
   unsigned long long int NumPixels)
{
   register int i = NumPixels + (NumPixels << 1) - 1;
   register int j = NumPixels + (NumPixels << 1) - 1;
   register int y, u, v;
   register int r, g, b;

   while (i > 0) {
   v = src[i--] - 128;
   y = src[i--];
   u = src[i--] - 128;
   YUV2RGB(y, u, v, r, g, b);
   dest[j--] = r;
   dest[j--] = g;
   dest[j--] = b;
   }
}

static void
uyvy2bgr(const unsigned char *src, unsigned char *dest,
   unsigned long long int NumPixels)
{
   register int i = (NumPixels << 1) - 1;
   register int j = NumPixels + (NumPixels << 1) - 1;
   register int y0, y1, u, v;
   register int r, g, b;

   while (i > 0) {
   y1 = src[i--];
   v = src[i--] - 128;
   y0 = src[i--];
   u = src[i--] - 128;
   YUV2RGB(y1, u, v, r, g, b);
   dest[j--] = r;
   dest[j--] = g;
   dest[j--] = b;
   YUV2RGB(y0, u, v, r, g, b);
   dest[j--] = r;
   dest[j--] = g;
   dest[j--] = b;
   }
}


static void
uyyvyy2bgr(const unsigned char *src, unsigned char *dest,
     unsigned long long int NumPixels)
{
   register int i = NumPixels + (NumPixels >> 1) - 1;
   register int j = NumPixels + (NumPixels << 1) - 1;
   register int y0, y1, y2, y3, u, v;
   register int r, g, b;

   while (i > 0) {
   y3 = src[i--];
   y2 = src[i--];
   v = src[i--] - 128;
   y1 = src[i--];
   y0 = src[i--];
   u = src[i--] - 128;
   YUV2RGB(y3, u, v, r, g, b);
   dest[j--] = r;
   dest[j--] = g;
   dest[j--] = b;
   YUV2RGB(y2, u, v, r, g, b);
   dest[j--] = r;
   dest[j--] = g;
   dest[j--] = b;
   YUV2RGB(y1, u, v, r, g, b);
   dest[j--] = r;
   dest[j--] = g;
   dest[j--] = b;
   YUV2RGB(y0, u, v, r, g, b);
   dest[j--] = r;
   dest[j--] = g;
   dest[j--] = b;
   }
}

static void
y2bgr(const unsigned char *src, unsigned char *dest,
      unsigned long long int NumPixels)
{
   register int i = NumPixels - 1;
   register int j = NumPixels + (NumPixels << 1) - 1;
   register int y;

   while (i > 0) {
   y = src[i--];
   dest[j--] = y;
   dest[j--] = y;
   dest[j--] = y;
   }
}

static void
y162bgr(const unsigned char *src, unsigned char *dest,
   unsigned long long int NumPixels, int bits)
{
   register int i = (NumPixels << 1) - 1;
   register int j = NumPixels + (NumPixels << 1) - 1;
   register int y;

   while (i > 0) {
   y = src[i--];
   y = (y + (src[i--] << 8)) >> (bits - 8);
   dest[j--] = y;
   dest[j--] = y;
   dest[j--] = y;
   }
}

// this one was in coriander but didn't take bits into account
static void
rgb482bgr(const unsigned char *src, unsigned char *dest,
   unsigned long long int NumPixels, int bits)
{
   register int i = (NumPixels << 1) - 1;
   register int j = NumPixels + (NumPixels << 1) - 1;
   register int y;

   while (i > 0) {
   y = src[i--];
   dest[j-2] = (y + (src[i--] << 8)) >> (bits - 8);
   j--;
   y = src[i--];
   dest[j] = (y + (src[i--] << 8)) >> (bits - 8);
   j--;
   y = src[i--];
   dest[j+2] = (y + (src[i--] << 8)) >> (bits - 8);
   j--;
   }
}

#endif
