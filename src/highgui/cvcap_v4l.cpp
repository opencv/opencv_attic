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

#if !defined WIN32 && defined HAVE_CAMV4L

#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/mman.h>

#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/videodev.h>

#define STREAMBUFS  4 /* Number of streaming buffer */

/* Device Capture Objects */

typedef struct CvCaptureCAM_V4L
{
    CvCaptureVTable* vtable;
    int camera_fd;
    struct video_capability vcap;
    struct video_window     vwin;
    struct video_picture    vpic; 
    struct video_mbuf       vmbuf;
    struct video_mmap       vmmap;
    char * capture_buffer;
    char * mmap_buffer;
    int format;
    int mode;
    int frame_rate;
    char * device_name;
    IplImage* rgb_frame;
    IplImage frame;
}
CvCaptureCAM_V4L;

static void icvCloseCAM_V4L( CvCaptureCAM_V4L* capture );

static int icvGrabFrameCAM_V4L( CvCaptureCAM_V4L* capture );
static IplImage* icvRetrieveFrameCAM_V4L( CvCaptureCAM_V4L* capture );

static double icvGetPropertyCAM_V4L( CvCaptureCAM_V4L* capture, int property_id );
static int    icvSetPropertyCAM_V4L( CvCaptureCAM_V4L* capture, int property_id, double value );

static void icvSetVideoSize( CvCaptureCAM_V4L* capture, int w, int h);
/***********************   Implementations  ***************************************/
#define MAX_PORTS 3 
#define MAX_CAMERAS 8
#define NUM_BUFFERS 8

static int numCameras = -1;

CvCaptureVTable captureCAM_V4L_vtable = 
{
    6,
    (CvCaptureCloseFunc)icvCloseCAM_V4L,
    (CvCaptureGrabFrameFunc)icvGrabFrameCAM_V4L,
    (CvCaptureRetrieveFrameFunc)icvRetrieveFrameCAM_V4L,
    (CvCaptureGetPropertyFunc)icvGetPropertyCAM_V4L,
    (CvCaptureSetPropertyFunc)icvSetPropertyCAM_V4L,
    (CvCaptureGetDescriptionFunc)0
};

void icvInitCapture_V4L(){
   int fd;
   struct video_capability vcap;
   numCameras = 0;
   fd = open("/dev/video0",O_RDONLY);
   if (fd == 0) {
      return;
   }
   if (ioctl(fd, VIDIOCGCAP, &vcap) < 0) {
      close(fd);
      return;
   }
   numCameras ++;
   close(fd);
   fd = open("/dev/video1",O_RDONLY);
   if (fd == 0) {
      return;
   }
   if (ioctl(fd, VIDIOCGCAP, &vcap) < 0) {
      close(fd);
      return;
   }
   numCameras ++;
   close(fd);
};

CvCapture* icvOpenCAM_V4L( int index ){
   if (numCameras<1)
      icvInitCapture_V4L();
   if (numCameras<1)
     return 0;
   if (index>=numCameras)
     return 0;
   if (index<0)
     return 0;
   CvCaptureCAM_V4L * pcap = (CvCaptureCAM_V4L*)cvAlloc(sizeof(CvCaptureCAM_V4L));
   /* w/o memset some parts  arent initialized */
   memset(pcap,0,sizeof(CvCaptureCAM_V4L));
   pcap->vtable = &captureCAM_V4L_vtable;
   pcap->frame.imageData = NULL;
   pcap->capture_buffer = NULL;
   /* Select camera */
   pcap->camera_fd = open("/dev/video0",O_RDONLY);
   if (pcap->camera_fd == 0) {
      cvFree((void **)&pcap);
      return NULL;
   }
   if (ioctl(pcap->camera_fd, VIDIOCGCAP, &pcap->vcap) < 0) {
      close(pcap->camera_fd);
      cvFree((void **)&pcap);
      return NULL;
   }

   /* -----
      this is for bttv like boards
      set the aprop. channel if available 
      and set the video mode.
      dont care if the ioctls fail
      hardcoded by hand for now 
    ---- */

   if (pcap->vcap.channels>0) {
     video_channel vc;
     /* enter the channel you want here */
     vc.channel=1;
     ioctl(pcap->camera_fd, VIDIOCGCHAN , &vc);
     /* set the video mode*/
     /* ( VIDEO_MODE_PAL, VIDEO_MODE_NTSC, VIDEO_MODE_SECAM) */
     vc.norm = VIDEO_MODE_PAL;
     ioctl(pcap->camera_fd, VIDIOCSCHAN , &vc); 
   }

   icvSetVideoSize( pcap, 640, 480);

   /* Set picture depth and format */
   if(ioctl(pcap->camera_fd, VIDIOCGPICT, &pcap->vpic) < 0) {
      close(pcap->camera_fd);
      cvFree((void **)&pcap);
      return NULL;
   }
   pcap->vpic.brightness=32640;
   pcap->vpic.contrast=32640;
   pcap->vpic.depth=24;
   pcap->vpic.palette=VIDEO_PALETTE_RGB24;
   if(ioctl(pcap->camera_fd, VIDIOCSPICT, &pcap->vpic) < 0) {
      close(pcap->camera_fd);
      cvFree((void **)&pcap);
      return NULL;
   }

   /* Setup mapped memory io */
   ioctl(pcap->camera_fd, VIDIOCGMBUF, &pcap->vmbuf);
   pcap->mmap_buffer  = (char *)mmap(0, pcap->vmbuf.size, PROT_READ, MAP_SHARED, pcap->camera_fd, 0);
   pcap->vmmap.frame  =  0;
   pcap->vmmap.format =  pcap->vpic.palette;

   return (CvCapture *)pcap;
};

static void icvCloseCAM_V4L( CvCaptureCAM_V4L* capture ){
   close(capture->camera_fd);
   /* Deallocate space for RGBA data */ 
   cvFree((void **)&capture->capture_buffer);
   cvFree((void **)&capture->frame.imageData);
   if (capture->mmap_buffer>(char *)1L) {
        munmap(capture->mmap_buffer, capture->vmbuf.size);
   }
};

int syncFrame=-1, grabFrame=-1;

static int icvGrabFrameCAM_V4L( CvCaptureCAM_V4L* capture ){  
  if (capture->mmap_buffer>(char *)1L) {
     if (grabFrame<0) {
        capture->vmmap.frame=0;
        ioctl(capture->camera_fd, VIDIOCMCAPTURE, &capture->vmmap);
        capture->vmmap.frame=1;
        ioctl(capture->camera_fd, VIDIOCMCAPTURE, &capture->vmmap);
        grabFrame=1;
        syncFrame=0;
        //return (ioctl(capture->camera_fd, VIDIOCMCAPTURE, &capture->vmmap)>=0);
        return 1;
     }
     else
       {
        ioctl(capture->camera_fd, VIDIOCMCAPTURE, &capture->vmmap);
        syncFrame = (syncFrame+1)%2;
        return 1;
       }
  }
  else {
      int grabbedbytes = read(capture->camera_fd, capture->capture_buffer, capture->frame.imageSize);
      return (grabbedbytes == capture->frame.imageSize) ;
  }
};

static IplImage* icvRetrieveFrameCAM_V4L( CvCaptureCAM_V4L* capture ){
   char* src;
   char* dst;
   if (capture->mmap_buffer>(char *)1L) {
      capture->vmmap.frame=syncFrame;
      ioctl(capture->camera_fd, VIDIOCSYNC, &capture->vmmap.frame);
      src = (char *)capture->mmap_buffer;
      src += capture->vmbuf.offsets[capture->vmmap.frame];
      //capture->vmmap.frame = (capture->vmmap.frame+1)%capture->vmbuf.frames;
   }
   else if(capture->capture_buffer )
      src = (char *)capture->capture_buffer;
   else
      return 0;   

   if (capture->vpic.palette == VIDEO_PALETTE_RGB24) { 
      dst = capture->frame.imageData;
      memcpy(dst,src,capture->frame.imageSize);
      return &capture->frame;
   }
   return 0;
};

static double icvGetPropertyCAM_V4L( CvCaptureCAM_V4L* capture, int property_id ){
   return 0;
};

static void icvSetVideoSize( CvCaptureCAM_V4L* capture, int w, int h) {
  if (capture==0) return;
   if (w==0) {
      switch (h) {
         case 480: w=640;break; 
         case 288: w=352;break; 
         case 240: w=320;break; 
         case 144: w=176;break; 
         case 120: w=160;break; 
      }
   }
   if (h==0) {
      switch (w) {
         case 640: h=480;break; 
         case 352: h=288;break; 
         case 320: h=240;break; 
         case 176: h=144;break; 
         case 160: h=120;break; 
      }
   };
   if (w>capture->vcap.maxwidth) {
       w=capture->vcap.maxwidth;
   }
   if (h>capture->vcap.maxheight) {
       h=capture->vcap.maxheight;
   }
   capture->vwin.width=w;    
   capture->vwin.height=h;    
   if (ioctl(capture->camera_fd, VIDIOCSWIN, &capture->vwin) < 0) {
      close(capture->camera_fd);
      cvFree((void **)&capture);
      return; /* be sure to quit if capture is freed SEGV*/ 
   }
   if (ioctl(capture->camera_fd, VIDIOCGWIN, &capture->vwin) < 0) {
      close(capture->camera_fd);
      cvFree((void **)&capture);
      return; /* be sure to quit if capture is freed SEGV*/ 
   }
   capture->vmmap.width  =  capture->vwin.width;
   capture->vmmap.height =  capture->vwin.height;
   cvInitImageHeader( &capture->frame,cvSize( capture->vwin.width,capture->vwin.height ),
                           IPL_DEPTH_8U, 3, IPL_ORIGIN_TL, 4 );
   /* Deallocate space for RGBA data */ 
   if (capture->capture_buffer)  cvFree((void **)&capture->capture_buffer);
   if (capture->frame.imageData) cvFree((void **)&capture->frame.imageData);
   /* Allocate space for RGBA data */ 
   capture->frame.imageData = (char *)cvAlloc(capture->frame.imageSize);
   capture->capture_buffer = (char *)cvAlloc(capture->frame.imageSize);
}

static int    icvSetPropertyCAM_V4L( CvCaptureCAM_V4L* capture, int property_id, double value ){
   switch (property_id) {
       case CV_CAP_PROP_FRAME_WIDTH:
         icvSetVideoSize( capture, cvRound(value), 0);
         break;
       case CV_CAP_PROP_FRAME_HEIGHT:
         icvSetVideoSize( capture, 0, cvRound(value));
         break;
   }
   return 0;
};

#endif
