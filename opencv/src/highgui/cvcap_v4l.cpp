/* This is the contributed code:

File:             cvcap_v4l.cpp
Current Location: ../opencv-0.9.6/otherlibs/highgui

Original Version: 2003-03-12  Magnus Lundin lundin@mlu.mine.nu
Original Comments:

ML:This set of files adds support for firevre and usb cameras.
First it tries to install a firewire camera,
if that fails it tries a v4l/USB camera
It has been tested with the motempl sample program
 
This Patch:   August 24, 2004 Travis Wood   TravisOCV@tkwood.com
For Release:  OpenCV-Linux Beta4  opencv-0.9.6
Tested On:    LMLBT44 with 8 video inputs
Problems?     Post problems/fixes to OpenCV group on groups.yahoo.com
Patched Comments:

TW: The cv cam utils that came with the initial release of OpenCV for LINUX Beta4
were not working.  I have rewritten them so they work for me. At the same time, trying
to keep the original code as ML wrote it as unchanged as possible.  No one likes to debug
someone elses code, so I resisted changes as much as possible.  I have tried to keep the
same "ideas" where applicable, that is, where I could figure out what the previous author
intended. Some areas I just could not help myself and had to "spiffy-it-up" my way.

These drivers should work with other V4L frame capture cards other then my bttv
driven frame capture card.  

Re Written driver for standard V4L mode. Tested using LMLBT44 video capture card.
Standard bttv drivers are on the LMLBT44 with up to 8 Inputs.

This utility was written with the help of the document:
http://pages.cpcs.ucalgary.ca/~sayles/VFL_HowTo
as a general guide for interfacing into the V4l standard.

Made the index value passed for icvOpenCAM_V4L(index) be the number of the
video device source in the /dev tree. The -1 uses original /dev/video.

Index  Device
  0    /dev/video0
  1    /dev/video1
  2    /dev/video2
  3    /dev/video3
  ...
  7    /dev/video7
with
  -1   /dev/video

TW: You can select any video source, but this package was limited from the start to only
ONE camera opened at any ONE time.  
This is an original program limitation.
If you are interested, I will make my version available to other OpenCV users.  The big
difference in mine is you may pass the camera number as part of the cv argument, but this
convention is non standard for current OpenCV calls and the camera number is not currently
passed into the called routine.

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

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/mman.h>

#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/videodev.h>

/* I highly recomend taking out the next line and installing it this way.  That is, if your
   video card can handle it! */

//#define V4L_SCALE_OPTION

/* Defaults - If your board can do better, set it here.  Set for the most common type inputs. */
/* These 2 DEFAULTS are only valis if V4L_SCALE_OPTION is defined */
#define DEFAULT_V4L_WIDTH  640
#define DEFAULT_V4L_HEIGHT 480

#define CHANNEL_NUMBER 1
#define MAX_CAMERAS 8

#define MAX_DEVICE_DRIVER_NAME 14

/* Device Capture Objects */

typedef struct CvCaptureCAM_V4L
{
    CvCaptureVTable* vtable;
    int deviceHandle;
    struct video_capability capability;
    struct video_window     captureWindow;
    struct video_picture    imageProperties; 
    struct video_mbuf       memoryBuffer;
    struct video_mmap       *mmaps;
    char *memoryMap;
    IplImage frame;
}
CvCaptureCAM_V4L;

static void icvCloseCAM_V4L( CvCaptureCAM_V4L* capture );

static int icvGrabFrameCAM_V4L( CvCaptureCAM_V4L* capture );
static IplImage* icvRetrieveFrameCAM_V4L( CvCaptureCAM_V4L* capture );

static double icvGetPropertyCAM_V4L( CvCaptureCAM_V4L* capture, int property_id );
static int    icvSetPropertyCAM_V4L( CvCaptureCAM_V4L* capture, int property_id, double value );

static void icvSetVideoSize( CvCaptureCAM_V4L* capture, int w, int h);

//static void icvSetVideoSize( CvCaptureCAM_V4L* capture, int w, int h);
/***********************   Implementations  ***************************************/

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
 

/* Simple test program: Find number of Video Sources available.
   Start from 0 and go to MAX_CAMERAS while checking for the device with that name.
   If it fails on the first attempt of /dev/video0, then check if /dev/video is valid.
   Returns the global numCameras with the correct value (we hope) */

void icvInitCapture_V4L() {
   int deviceHandle;
   int CameraNumber;
   char deviceName[MAX_DEVICE_DRIVER_NAME];

   CameraNumber = 0;
   while(CameraNumber < MAX_CAMERAS) {
      /* Print the CameraNumber at the end of the string with a width of one character */
      sprintf(deviceName, "/dev/video%1d", CameraNumber);
      /* Test using an open to see if this new device name really does exists. */
      deviceHandle = open(deviceName, O_RDONLY);
      if (deviceHandle != -1) {
         /* This device does indeed exist - add it to the total so far */
         numCameras = CameraNumber+1;
         close(deviceHandle);
      }
      else {
         /* Check if we failed on the fist device attempt of /dev/video0. 
         If we did, then check for a simple /dev/video with no trailing number */
         if (numCameras<1) {
            strcpy(deviceName, "/dev/video");
            deviceHandle = open(deviceName, O_RDWR);
            if (deviceHandle == -1) {
               /* Could not find ANY /dev/video type inputs! 
               return with -1 still in numCameras. */
               return;
            }
            else {
               /* It would seem that the device is called /dev/video at this point.
               return with the total number of cameras equal to just one. */
               numCameras = 1;
               return;
            } /* End if-else */
         } /* End numCameras<1 */
         /* So we ran into a device name that does not exist.  That would mean we ran
         into the last device name after sucessfully openening previous devices. We
         now know the true number of /dev/videoX sources at this point. This is now 
         reflected in numCameras correctly.  All we need to do now is return. */
         return;
      } /* End if-else */

      /* Set up to test the next /dev/video source in line */
      CameraNumber++;
   } /* End while */
      
}; /* End icvInitCapture_V4L */

CvCapture* icvOpenCAM_V4L( int index ) {
   int i;
   struct video_channel selectedChannel;
   char deviceName[MAX_DEVICE_DRIVER_NAME];

   if (numCameras<1)
      icvInitCapture_V4L(); /* Havent called icvInitCapture yet - do it now! */
   if (numCameras<1)
     return 0; /* Are there any /dev/video input sources? */
   if (index>=numCameras)
     return 0; /* Did someone ask for too high a video source number? */
   /* Allocate memory for this humongus CvCaptureCAM_V4L structure that contains ALL
      the handles for V4L processing */
   CvCaptureCAM_V4L * capture = (CvCaptureCAM_V4L*)cvAlloc(sizeof(CvCaptureCAM_V4L));
   if (!capture) {
      printf("\nCould not allocate memory for capture process.\n");
      return NULL;
   }
   /* w/o memset some parts  arent initialized - AKA: Fill it with zeros so it is clean */
   memset(capture,0,sizeof(CvCaptureCAM_V4L));
   /* Present the routines needed for V4L funtionality.  They are inserted as part of
      the standard set of cv calls promoting transparency.  "Vector Table" insertion. */
   capture->vtable = &captureCAM_V4L_vtable;

   /* Select camera, or rather, V4L video source */
   if (index<0) { /* Asking for the plane old /dev/video device of lor' */
     strcpy(deviceName, "/dev/video");
   }
   else {
      /* Print the CameraNumber at the end of the string with a width of one character */
      sprintf(deviceName, "/dev/video%1d", index);
      /* Test using an open to see if this new device name really does exists. */
   }

   /* No matter what the name - it still must be opened! */
   capture->deviceHandle = open(deviceName, O_RDWR);
   if (capture->deviceHandle == 0) {
      printf("\n\nV4L device %s: Unable to open for READ ONLY\n",deviceName);
      icvCloseCAM_V4L(capture);
      return NULL;
   }

   /* Query the newly opened device for its capabilities */
   if (ioctl(capture->deviceHandle, VIDIOCGCAP, &capture->capability) < 0) {
      printf("\n\nV4L device %s: Unable to query its capability.\n",deviceName);
      icvCloseCAM_V4L(capture);
      return NULL;
   }

   /* Can this device capture video to memory? */
   if ((capture->capability.type & VID_TYPE_CAPTURE) == 0) {
      /* Nope. */
      printf("\n\nV4L device %s is unable to capture video to memory.\n",deviceName);
      icvCloseCAM_V4L(capture);
      return NULL;
   }

   /* The following code sets the CHANNEL_NUMBER of the video input.  Some video sources
   have sub "Channel Numbers".  For a typical V4L TV capture card, this is usually 1.
   I myself am using a simple NTSC video input capture card that uses the value of 1.
   If you are not in North America or have a different video standard, you WILL have to change
   the following settings and recompile/reinstall.  This set of settings is based on
   the most commonly encountered input video source types (like my bttv card) */
   if(capture->capability.channels>0) {
     selectedChannel.channel=CHANNEL_NUMBER;
     if (ioctl(capture->deviceHandle, VIDIOCGCHAN , &selectedChannel) != -1) {
        /* set the video mode to ( VIDEO_MODE_PAL, VIDEO_MODE_NTSC, VIDEO_MODE_SECAM) */
        selectedChannel.norm = VIDEO_MODE_NTSC;
        if (ioctl(capture->deviceHandle, VIDIOCSCHAN , &selectedChannel) == -1) {
           /* Could not set selected channel - Oh well */
           printf("\n%d, %s not NTSC capable.\n",selectedChannel.channel, selectedChannel.name);
        } /* End if */
     } /* End if */ 
   } /* End if */

#if defined V4L_SCALE_OPTION
   /* If you need a bigger picture, set V4L_SCALE_OPTION up top.  Otherwise, it is safer
      to leave it out and allow for its default scale to come up. */

   /* If your card can open a bigger window, it will be attempted here.  I have a version
      of this program that returns the "Property" set ability, but it is buggy.  It also
      handles multiple camers for stero work.  Contact TW if you would like to use it */
   if ((capture->capability.type & VID_TYPE_SCALES) != 0) {
      /* Supports the ability to scale captured images */
      capture->captureWindow.x = 0;
      capture->captureWindow.y = 0;
      capture->captureWindow.width  = DEFAULT_V4L_WIDTH;
      capture->captureWindow.height = DEFAULT_V4L_HEIGHT;
      capture->captureWindow.chromakey = 0;
      capture->captureWindow.flags = 0;
      capture->captureWindow.clips = 0;
      capture->captureWindow.clipcount = 0;
      if (ioctl(capture->deviceHandle, VIDIOCSWIN, &capture->captureWindow) == -1) {
         printf("V4l scaleing error: Device is lying to me saying it can be scaled.\n");
         icvCloseCAM_V4L(capture);
         return NULL;
      }
      /* Now, we be set! */
   }

#endif

   /* Find Window info */
   if(ioctl(capture->deviceHandle, VIDIOCGWIN, &capture->captureWindow) == -1) {
      printf("\n\nV4L: Could not obtain specifics of capture window.\n\n");
      icvCloseCAM_V4L(capture);
      return NULL;
   }

   /* Find Picture info */
   if(ioctl(capture->deviceHandle, VIDIOCGPICT, &capture->imageProperties) < 0) {
      printf("\n\nV4L: Unable to determine size of incoming image\n");
      icvCloseCAM_V4L(capture);
      return NULL;
   }

   /* Check to see if this HARD WIRED part is supported */
   if ( (capture->imageProperties.depth != 24) 
     || (capture->imageProperties.palette != VIDEO_PALETTE_RGB24) ) {
      printf("\n\nV4L Warn: This V4L device might need to be hardwired a different way. \n");
      printf("Look in cvcap_v4l.cpp and modifiy this code located at this comment.\n");
      /* I will now exit as there is half a chance the following SET command might pull
         this capture card into a suitable capture mode... */
   } 
   /* Yet MORE things that might have to be changes with your frame capture card */
   capture->imageProperties.depth=      24;
   capture->imageProperties.palette=    VIDEO_PALETTE_RGB24;
   /* This sets the scale to the center of a 2^16 number */
   capture->imageProperties.brightness= 65535/2;
   capture->imageProperties.contrast=   65535/2;
   capture->imageProperties.colour=     65535/2;
   capture->imageProperties.hue=        65535/2;
   if(ioctl(capture->deviceHandle, VIDIOCSPICT, &capture->imageProperties) < 0) {
      printf("\n\nUnable to set Brightness, Contrast, Color, Hue, Depth or Palette.\n");
      icvCloseCAM_V4L(capture);
      return NULL;
   }

   /* Setup mapped memory io */
   ioctl(capture->deviceHandle, VIDIOCGMBUF, &capture->memoryBuffer);
   capture->memoryMap  = (char *)mmap(0, 
                                   capture->memoryBuffer.size,
                                   PROT_READ | PROT_WRITE,
                                   MAP_SHARED,
                                   capture->deviceHandle,
                                   0);
   if (capture->memoryMap == MAP_FAILED) {
      printf("\nMapping Memmory from video source error: %s\n", strerror(errno));
      icvCloseCAM_V4L(capture);
   }

   /* Set up video_mmap structure pointing to this memory mapped area so each image may be
      retrieved from an index value */
   capture->mmaps = (struct video_mmap *)
                 (malloc(capture->memoryBuffer.frames * sizeof(struct video_mmap)));
   if (!capture->mmaps) {
      printf("V4L: Could not memory map video frames.\n");
      icvCloseCAM_V4L(capture);
      return NULL;
   }
   /* Fill out each of the video_mmap structures with frame information */
   i = 0;
   while (i < capture->memoryBuffer.frames) {
      capture->mmaps[i].frame  = i;
      capture->mmaps[i].width  = capture->captureWindow.width;
      capture->mmaps[i].height = capture->captureWindow.height;
      capture->mmaps[i].format = capture->imageProperties.palette;
      ++i;
   }

   /* Set up Image data */
   capture->frame.imageSize = capture->captureWindow.width * capture->captureWindow.height
                            * ((capture->imageProperties.depth+7)>>3);
   cvInitImageHeader( &capture->frame,
                      cvSize( capture->captureWindow.width,
                              capture->captureWindow.height ),
                      IPL_DEPTH_8U, 3, IPL_ORIGIN_TL, 4 );
   /* Allocate space for RGBA data */
   capture->frame.imageData = (char *)cvAlloc(capture->frame.imageSize);

   return (CvCapture *)capture;
}; /* End icvOpenCAM_V4L */

static int bufferIndex;
static int FirstCapture=1;

/* I got a little tricky here, in that: I initiate the camera DMA first, then SYNC with the data
   after processing. This sped things up! */
static int icvGrabFrameCAM_V4L( CvCaptureCAM_V4L* capture) {
   int i;

   if (FirstCapture) { 
      /* Some general initialization must take place the first time through */
      FirstCapture = 0;
      /* This is just a technicality, but all buffers must be filled up before any
         staggered SYNC is applied.  SO, filler up. */
      i = 0;
      while (i < (capture->memoryBuffer.frames)) {
         if (ioctl(capture->deviceHandle, VIDIOCMCAPTURE, &capture->mmaps[i]) == -1) {
            printf("\n\nV4L Initial Capture Error: Unable to load initial memory buffers.\n\n");
            return 0;
         }
         ++i;
      }

      bufferIndex = capture->memoryBuffer.frames - 1;
   }

   if (ioctl(capture->deviceHandle, VIDIOCSYNC, &capture->mmaps[bufferIndex]) == -1) {
      printf("\n\nCould not SYNC to video stream. %s\n\n", strerror(errno));
      return(0);
   }

   return(1);
}

static IplImage* icvRetrieveFrameCAM_V4L( CvCaptureCAM_V4L* capture ) {
   /* Now get what has already been captured as a IplImage return */
   memcpy((char *)capture->frame.imageData, 
          (char *)(capture->memoryMap + capture->memoryBuffer.offsets[bufferIndex]),
          capture->frame.imageSize);

   ++bufferIndex;
   if (bufferIndex == capture->memoryBuffer.frames) {
      /* Do not let buffer index past the last buffer - reset it */
      bufferIndex = 0;
   }

   /* Start capture of next frame even before getting last buffer */
   if (ioctl(capture->deviceHandle, VIDIOCMCAPTURE, &capture->mmaps[bufferIndex]) == -1) {
      printf("\n\nUnable to start next buffer cycle. %s\n\n", strerror(errno));
      return(0);
   }

   return(&capture->frame);
}

static double icvGetPropertyCAM_V4L( CvCaptureCAM_V4L* capture, int property_id ){

   if (ioctl(capture->deviceHandle, VIDIOCGWIN, &capture->captureWindow) < 0) {
      icvCloseCAM_V4L(capture);
      return NULL;
   }

   switch (property_id) {
       case CV_CAP_PROP_FRAME_WIDTH:
         return((double)capture->captureWindow.width);
       case CV_CAP_PROP_FRAME_HEIGHT:
         return((double)capture->captureWindow.height);
   }
   return 0;
};

#if defined V4L_SCALE_OPTION

static void icvSetVideoSize( CvCaptureCAM_V4L* capture, int w, int h) {
  int i;

  if (capture==0) return;
  if ((capture->capability.type & VID_TYPE_SCALES) == 0) return;
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
   if (w>capture->capability.maxwidth) {
       w=capture->capability.maxwidth;
   }
   if (h>capture->capability.maxheight) {
       h=capture->capability.maxheight;
   }
   capture->captureWindow.width=w;
   capture->captureWindow.height=h;

   if (ioctl(capture->deviceHandle, VIDIOCSWIN, &capture->captureWindow) < 0) {
      icvCloseCAM_V4L(capture);
      return;
   }

   if (ioctl(capture->deviceHandle, VIDIOCGWIN, &capture->captureWindow) < 0) {
      icvCloseCAM_V4L(capture);
      return;
   }

   /* Fill out each of the video_mmap structures with frame information */
   i = 0;
   while (i < capture->memoryBuffer.frames) {
      capture->mmaps[i].frame  = i;
      capture->mmaps[i].width  = capture->captureWindow.width;
      capture->mmaps[i].height = capture->captureWindow.height;
      capture->mmaps[i].format = capture->imageProperties.palette;
      ++i;
   }

   /* Set up Image data */
   capture->frame.imageSize = capture->captureWindow.width * capture->captureWindow.height
                            * ((capture->imageProperties.depth+7)>>3);
   cvInitImageHeader( &capture->frame,
                      cvSize( capture->captureWindow.width,
                              capture->captureWindow.height ),
                      IPL_DEPTH_8U, 3, IPL_ORIGIN_TL, 4 );
   /* Allocate space for RGBA data */
   if (capture->frame.imageData) cvFree((void **)&capture->frame.imageData);
   capture->frame.imageData = (char *)cvAlloc(capture->frame.imageSize);
}
 
static int icvSetPropertyCAM_V4L( CvCaptureCAM_V4L* capture, int property_id, double value ){
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

#else

/* Property things are stubbed V4L_SCALE_OPTION is false
   I have a newer vrersion of this that can scale the input if your capture card supports it */
static int    icvSetPropertyCAM_V4L( CvCaptureCAM_V4L* capture, int property_id, double value ){
   switch (property_id) {
       case CV_CAP_PROP_FRAME_WIDTH:
         printf("Not Done: Setting Width to %lf\n",value);
         break;
       case CV_CAP_PROP_FRAME_HEIGHT:
         printf("Not Done: Setting Width to %lf\n",value);
         break;
   }
   return 0;
};

#endif

static void icvCloseCAM_V4L( CvCaptureCAM_V4L* capture ){
   /* Deallocate space - Hopefully, no leaks */ 
   if (capture) {
      if (capture->mmaps) free(capture->mmaps);
      if (capture->memoryMap) munmap(capture->memoryMap, capture->memoryBuffer.size);
      if (capture->deviceHandle > 0) close(capture->deviceHandle);
      if (capture->frame.imageData) cvFree((void**)&capture->frame.imageData);
      //cvFree((void **)capture);
   }
};

#endif
