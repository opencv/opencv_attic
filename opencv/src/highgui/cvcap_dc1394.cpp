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

#include <libraw1394/raw1394.h>
#include <libdc1394/dc1394_control.h>

/* should be in pixelformat */
static void yuv422_to_bgr(unsigned char * src,unsigned char * dest,int w,int h);

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
   pcap->format = FORMAT_VGA_NONCOMPRESSED;
   pcap->mode = MODE_640x480_RGB, //MODE_320x240_YUV422;
     //pcap->mode = MODE_640x480_YUV422, //MODE_320x240_YUV422;
     //pcap->mode = MODE_320x240_YUV422, //MODE_320x240_YUV422;
   pcap->frame_rate = FRAMERATE_15;
   /* Select a port and camera */
   pcap->device_name = videodev[cameras[index].portnum];
   pcap->handle = handles[cameras[index].portnum];
   pcap->camera = &cameras[index].cam;

   dc1394_dma_setup_capture(pcap->handle,pcap->camera->node,index+1 /*channel*/,
                pcap->format, pcap->mode,SPEED_200, pcap->frame_rate, NUM_BUFFERS,
                            1 /*DROP_FRAMES*/,pcap->device_name, pcap->camera);

   dc1394_start_iso_transmission(pcap->handle, pcap->camera->node);

   cvInitImageHeader( &pcap->frame,cvSize( pcap->camera->frame_width,pcap->camera->frame_height ),
                           IPL_DEPTH_8U, 3, IPL_ORIGIN_TL, 4 );
   /* Allocate space for RGBA data */ 
   pcap->frame.imageData = (char *)cvAlloc(pcap->frame.imageSize);
   return (CvCapture *)pcap;
};

static void icvCloseCAM_DC1394( CvCaptureCAM_DC1394* capture ){
   dc1394_stop_iso_transmission(capture->handle, capture->camera->node);
   /* Deallocate space for RGBA data */ 
   cvFree(&(void *)capture->frame.imageData);
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
        char * src = (char *)capture->camera->capture_buffer;
        char * dst = (char *)capture->frame.imageData;
        switch (capture->mode) {
    case MODE_640x480_RGB: 
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
    case MODE_640x480_YUV422:
        case MODE_320x240_YUV422: 
       yuv422_to_bgr((unsigned char *)capture->camera->capture_buffer,(unsigned char *)capture->frame.imageData,capture->frame.width,capture->frame.height); 
       break;
        } /* switch (capture->mode) */
    dc1394_dma_done_with_buffer(capture->camera);
    return &capture->frame;
    }
    return 0;
};

static double icvGetPropertyCAM_DC1394( CvCaptureCAM_DC1394* capture, int property_id ){
   return 0;
};

static int    
icvSetPropertyCAM_DC1394( CvCaptureCAM_DC1394* capture, int property_id, double value ){
   switch ( property_id ) {
      case CV_CAP_PROP_FPS:
     unsigned int fps=15;
         if (value==7.5) fps=FRAMERATE_7_5;
         if (value==15) fps=FRAMERATE_15;
         if (value==30) fps=FRAMERATE_30;
         dc1394_set_video_framerate(capture->handle, capture->camera->node,fps);
         dc1394_get_video_framerate(capture->handle, capture->camera->node,(unsigned int *) &capture->camera->frame_rate);
         break;
   }
   return 0;
};

/*********************************************************************************************
PIXELFORMAT CONVERSIONS - Unoptimized
*********************************************************************************************/
void yuv422_to_bgr(unsigned char * src,unsigned char * dest,int w,int h) {
/* UYVY unsigned char to BGR unsigned char */ 
int R,G,B;
unsigned char * pSrc, * pDest;
double YY1,YY2,U,V;
/*
if (useMMX) {
   MMX_ConvUYVYTo32(src,dest,w,h);
   return;
   } // else
*/
pSrc=src;
for (int r=0;r<h;r++) {
   //   pDest=dest+w*(h-r-1)*3;
   pDest=dest+w*r*3;
   for (int c=w/2;c>0;c--)
      {
         U=(*pSrc++)-128.0;
         YY1=1.164*((*pSrc++)-16.0);
         V=(*pSrc++)-128.0;
         YY2=1.164*((*pSrc++)-16.0);
         B=YY1          + 2.018*U;
         G=YY1 - 0.813*V- 0.391*U;
         R=YY1 + 1.596*V;
         if (R<0) R=0;if (G<0) G=0;if (B<0) B=0;
         if (R>255) R=255;if (G>255) G=255;if (B>255) B=255;
         pDest[0] =(unsigned char)B ;
         pDest[1] =(unsigned char)G ;
         pDest[2] =(unsigned char)R ;
         pDest+=3;
         B=YY2          + 2.018*U;
         G=YY2 - 0.813*V- 0.391*U;
         R=YY2 + 1.596*V;
         if (R<0) R=0;if (G<0) G=0;if (B<0) B=0;
         if (R>255) R=255;if (G>255) G=255;if (B>255) B=255;
         pDest[0] =(unsigned char)B ;
         pDest[1] =(unsigned char)G ;
         pDest[2] =(unsigned char)R ;
         pDest+=3;
         }
   }
}

#endif
