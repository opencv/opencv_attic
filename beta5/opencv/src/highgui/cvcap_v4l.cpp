/* This is the contributed code:

File:             cvcap_v4l.cpp
Current Location: ../opencv-0.9.6/otherlibs/highgui

Original Version: 2003-03-12  Magnus Lundin lundin@mlu.mine.nu
Original Comments:

ML:This set of files adds support for firevre and usb cameras.
First it tries to install a firewire camera,
if that fails it tries a v4l/USB camera
It has been tested with the motempl sample program
 
First Patch:  August 24, 2004 Travis Wood   TravisOCV@tkwood.com
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
http://pages.cpsc.ucalgary.ca/~sayles/VFL_HowTo
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

Second Patch:   August 28, 2004 Sfuncia Fabio fiblan@yahoo.it
For Release:  OpenCV-Linux Beta4 Opencv-0.9.6

FS: this patch fix not sequential index of device (unplugged device), and real numCameras.
    for -1 index (icvOpenCAM_V4L) i dont use /dev/video but real device available, because 
    if /dev/video is a link to /dev/video0 and i unplugged device on /dev/video0, /dev/video 
    is a bad link. I search the first available device with indexList. 

Third Patch:   December 9, 2004 Frederic Devernay Frederic.Devernay@inria.fr
For Release:  OpenCV-Linux Beta4 Opencv-0.9.6

[FD] I modified the following:
 - handle YUV420P, YUV420, and YUV411P palettes (for many webcams) without using floating-point
 - cvGrabFrame should not wait for the end of the first frame, and should return quickly
   (see highgui doc)
 - cvRetrieveFrame should in turn wait for the end of frame capture, and should not
   trigger the capture of the next frame (the user choses when to do it using GrabFrame)
   To get the old behavior, re-call cvRetrieveFrame just after cvGrabFrame.
 - having global bufferIndex and FirstCapture variables makes the code non-reentrant
 (e.g. when using several cameras), put these in the CvCapture struct.
 - according to V4L HowTo, incrementing the buffer index must be done before VIDIOCMCAPTURE.
 - the VID_TYPE_SCALES stuff from V4L HowTo is wrong: image size can be changed
   even if the hardware does not support scaling (e.g. webcams can have several
   resolutions available). Just don't try to set the size at 640x480 if the hardware supports
   scaling: open with the default (probably best) image size, and let the user scale it
   using SetProperty.
 - image size can be changed by two subsequent calls to SetProperty (for width and height)
 - bug fix: if the image size changes, realloc the new image only when it is grabbed
 - issue errors only when necessary, fix error message formatting.
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

/* Defaults - If your board can do better, set it here.  Set for the most common type inputs. */
#define DEFAULT_V4L_WIDTH  640
#define DEFAULT_V4L_HEIGHT 480

#define CHANNEL_NUMBER 1
#define MAX_CAMERAS 8

#define MAX_DEVICE_DRIVER_NAME 80

/* Device Capture Objects */

typedef struct CvCaptureCAM_V4L
{
    CvCaptureVTable* vtable;
    int deviceHandle;
    int bufferIndex;
    int FirstCapture;
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

static int icvSetVideoSize( CvCaptureCAM_V4L* capture, int w, int h);

/***********************   Implementations  ***************************************/

static int numCameras = 0;
static int indexList = 0; 
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
    // add indexList
    indexList|=(1 << CameraNumber);
        numCameras++;
    }        
    close(deviceHandle);
      /* Set up to test the next /dev/video source in line */
      CameraNumber++;
   } /* End while */
      
}; /* End icvInitCapture_V4L */

int
try_palette(int fd,
            struct video_picture *cam_pic,
        int pal,
            int depth)
{
  cam_pic->palette = pal;
  cam_pic->depth = depth;
  if (ioctl(fd, VIDIOCSPICT, cam_pic) < 0)
    return 0;
  if (ioctl(fd, VIDIOCGPICT, cam_pic) < 0)
    return 0;
  if (cam_pic->palette == pal)
    return 1;
  return 0;
}

CvCapture* icvOpenCAM_V4L( int index ) {
   static int autoindex=0;
   struct video_channel selectedChannel;
   char deviceName[MAX_DEVICE_DRIVER_NAME];
   if (!numCameras)
      icvInitCapture_V4L(); /* Havent called icvInitCapture yet - do it now! */
   if (!numCameras)
     return NULL; /* Are there any /dev/video input sources? */
   //search index in indexList
   if ( (index>-1) && ! ((1 << index) & indexList) ) 
   {
     fprintf( stderr, "HIGHGUI ERROR: V4L: index %d is not correct!\n",index);
     return NULL; /* Did someone ask for not correct video source number? */
   }
   /* Allocate memory for this humongus CvCaptureCAM_V4L structure that contains ALL
      the handles for V4L processing */
   CvCaptureCAM_V4L * capture = (CvCaptureCAM_V4L*)cvAlloc(sizeof(CvCaptureCAM_V4L));
   if (!capture) {
      fprintf( stderr, "HIGHGUI ERROR: V4L: Could not allocate memory for capture process.\n");
      return NULL;
   }
   /* Select camera, or rather, V4L video source */
   if (index<0) { // Asking for the first device available 
     for (; autoindex<MAX_CAMERAS;autoindex++)
    if (indexList & (1<<autoindex))
        break;
     if (autoindex==MAX_CAMERAS)
    return NULL; 
     index=autoindex;
     autoindex++;// i can recall icvOpenCAM_V4l with index=-1 for next camera
   }
   /* Print the CameraNumber at the end of the string with a width of one character */
   sprintf(deviceName, "/dev/video%1d", index);
   
   /* w/o memset some parts  arent initialized - AKA: Fill it with zeros so it is clean */
   memset(capture,0,sizeof(CvCaptureCAM_V4L));
   /* Present the routines needed for V4L funtionality.  They are inserted as part of
      the standard set of cv calls promoting transparency.  "Vector Table" insertion. */
   capture->vtable = &captureCAM_V4L_vtable;
   capture->FirstCapture = 1;

   /* Test using an open to see if this new device name really does exists. */
   /* No matter what the name - it still must be opened! */
   capture->deviceHandle = open(deviceName, O_RDWR);
   if (capture->deviceHandle == 0) {
      fprintf( stderr, "HIGHGUI ERROR: V4L: device %s: Unable to open for READ ONLY\n",deviceName);
      icvCloseCAM_V4L(capture);
      return NULL;
   }

   /* Query the newly opened device for its capabilities */
   if (ioctl(capture->deviceHandle, VIDIOCGCAP, &capture->capability) < 0) {
      fprintf( stderr, "HIGHGUI ERROR: V4L: device %s: Unable to query its capability.\n",deviceName);
      icvCloseCAM_V4L(capture);
      return NULL;
   }

   /* Can this device capture video to memory? */
   if ((capture->capability.type & VID_TYPE_CAPTURE) == 0) {
      /* Nope. */
      fprintf( stderr, "HIGHGUI ERROR: V4L: device %s is unable to capture video to memory.\n",deviceName);
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
           //printf("\n%d, %s not NTSC capable.\n",selectedChannel.channel, selectedChannel.name);
        } /* End if */
     } /* End if */ 
   } /* End if */

   /* Find Window info */
   if(ioctl(capture->deviceHandle, VIDIOCGWIN, &capture->captureWindow) == -1) {
      fprintf( stderr, "HIGHGUI ERROR: V4L: Could not obtain specifics of capture window.\n\n");
      icvCloseCAM_V4L(capture);
      return NULL;
   }
   /* Don't scale the image by default if there is hardware scaling */
   /* Else, chose the image size closest to DEFAULT_V4L_WIDTH x DEFAULT_V4L_HEIGHT */
   if(((capture->capability.type & VID_TYPE_SCALES) == 0) &&
      (capture->captureWindow.width < DEFAULT_V4L_WIDTH) &&
      (capture->captureWindow.height < DEFAULT_V4L_HEIGHT)) {
     /* If your card can open a bigger window, it will be attempted here.  I have a version
    of this program that returns the "Property" set ability, but it is buggy.  It also
    handles multiple camers for stero work.  Contact TW if you would like to use it */
     //printf("trying to get a %dx%d image.\n", DEFAULT_V4L_WIDTH, DEFAULT_V4L_HEIGHT);
     capture->captureWindow.x = 0;
     capture->captureWindow.y = 0;
     capture->captureWindow.width  = DEFAULT_V4L_WIDTH;
     capture->captureWindow.height = DEFAULT_V4L_HEIGHT;
     capture->captureWindow.chromakey = 0;
     capture->captureWindow.flags = 0;
     capture->captureWindow.clips = 0;
     capture->captureWindow.clipcount = 0;
     if (ioctl(capture->deviceHandle, VIDIOCSWIN, &capture->captureWindow) == -1) {
     //printf("cannot get a %dx%d image.\n", DEFAULT_V4L_WIDTH, DEFAULT_V4L_HEIGHT);
     }
     /* Get window info again, to get the real value */
     if(ioctl(capture->deviceHandle, VIDIOCGWIN, &capture->captureWindow) == -1) {
       fprintf( stderr, "HIGHGUI ERROR: V4L: Could not obtain specifics of capture window.\n\n");
       icvCloseCAM_V4L(capture);
       return NULL;
     }
   }

   /* Find Picture info */
   if(ioctl(capture->deviceHandle, VIDIOCGPICT, &capture->imageProperties) < 0) {
      fprintf( stderr, "HIGHGUI ERROR: V4L: Unable to determine size of incoming image\n");
      icvCloseCAM_V4L(capture);
      return NULL;
   }

   /* Yet MORE things that might have to be changes with your frame capture card */
   /* This sets the scale to the center of a 2^16 number */
   capture->imageProperties.brightness= 65535/2;
   capture->imageProperties.contrast=   65535/2;
   capture->imageProperties.colour=     65535/2;
   capture->imageProperties.hue=        65535/2;
   if (try_palette(capture->deviceHandle, &capture->imageProperties, VIDEO_PALETTE_RGB24, 24)) {
       //printf("negotiated palette RGB24\n");
   }
   else if (try_palette(capture->deviceHandle, &capture->imageProperties, VIDEO_PALETTE_YUV420P, 16)) {
       //printf("negotiated palette YUV420P\n");
   }
   else if (try_palette(capture->deviceHandle, &capture->imageProperties, VIDEO_PALETTE_YUV420, 16)) {
       //printf("negotiated palette YUV420\n");
   }
   else if (try_palette(capture->deviceHandle, &capture->imageProperties, VIDEO_PALETTE_YUV411P, 16)) {
       //printf("negotiated palette YUV420P\n");
   }
   else {
     fprintf( stderr, "HIGHGUI ERROR: V4L: Unable to set Brightness, Contrast, Color, Hue, Depth or Palette.\n");
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
      fprintf( stderr, "HIGHGUI ERROR: V4L: Mapping Memmory from video source error: %s\n", strerror(errno));
      icvCloseCAM_V4L(capture);
   }

   /* Set up video_mmap structure pointing to this memory mapped area so each image may be
      retrieved from an index value */
   capture->mmaps = (struct video_mmap *)
                 (malloc(capture->memoryBuffer.frames * sizeof(struct video_mmap)));
   if (!capture->mmaps) {
      fprintf( stderr, "HIGHGUI ERROR: V4L: Could not memory map video frames.\n");
      icvCloseCAM_V4L(capture);
      return NULL;
   }

   /* Set up Image data */
   cvInitImageHeader( &capture->frame,
                      cvSize( capture->captureWindow.width,
                              capture->captureWindow.height ),
                      IPL_DEPTH_8U, 3, IPL_ORIGIN_TL, 4 );
   /* Allocate space for RGBA data */
   capture->frame.imageData = (char *)cvAlloc(capture->frame.imageSize);

   return (CvCapture *)capture;
}; /* End icvOpenCAM_V4L */

static int icvGrabFrameCAM_V4L( CvCaptureCAM_V4L* capture) {
   if (capture->FirstCapture) { 
      /* Some general initialization must take place the first time through */
      capture->FirstCapture = 0;
      /* This is just a technicality, but all buffers must be filled up before any
         staggered SYNC is applied.  SO, filler up. (see V4L HowTo) */
      for (capture->bufferIndex = 0;
       capture->bufferIndex < (capture->memoryBuffer.frames-1);
       ++capture->bufferIndex) {
      capture->mmaps[capture->bufferIndex].frame  = capture->bufferIndex;
      capture->mmaps[capture->bufferIndex].width  = capture->captureWindow.width;
      capture->mmaps[capture->bufferIndex].height = capture->captureWindow.height;
      capture->mmaps[capture->bufferIndex].format = capture->imageProperties.palette;
         if (ioctl(capture->deviceHandle, VIDIOCMCAPTURE, &capture->mmaps[capture->bufferIndex]) == -1) {
            fprintf( stderr, "HIGHGUI ERROR: V4L: Initial Capture Error: Unable to load initial memory buffers.\n");
            return 0;
         }
      }
   }
   
   capture->mmaps[capture->bufferIndex].frame  = capture->bufferIndex;
   capture->mmaps[capture->bufferIndex].width  = capture->captureWindow.width;
   capture->mmaps[capture->bufferIndex].height = capture->captureWindow.height;
   capture->mmaps[capture->bufferIndex].format = capture->imageProperties.palette;
   if (ioctl(capture->deviceHandle, VIDIOCMCAPTURE, &capture->mmaps[capture->bufferIndex]) == -1) {
      fprintf( stderr, "HIGHGUI ERROR: V4L: Unable to start next buffer cycle. %s\n", strerror(errno));
      return(0);
   }
   ++capture->bufferIndex;
   if (capture->bufferIndex == capture->memoryBuffer.frames) {
      /* Do not let buffer index past the last buffer - reset it */
      capture->bufferIndex = 0;
   }

   return(1);
}

/*
 * Turn a YUV4:2:0 block into an RGB block
 *
 * Video4Linux seems to use the blue, green, red channel
 * order convention-- rgb[0] is blue, rgb[1] is green, rgb[2] is red.
 *
 * Color space conversion coefficients taken from the excellent
 * http://www.inforamp.net/~poynton/ColorFAQ.html
 * In his terminology, this is a CCIR 601.1 YCbCr -> RGB.
 * Y values are given for all 4 pixels, but the U (Pb)
 * and V (Pr) are assumed constant over the 2x2 block.
 *
 * To avoid floating point arithmetic, the color conversion
 * coefficients are scaled into 16.16 fixed-point integers.
 * They were determined as follows:
 *
 *  double brightness = 1.0;  (0->black; 1->full scale) 
 *  double saturation = 1.0;  (0->greyscale; 1->full color)
 *  double fixScale = brightness * 256 * 256;
 *  int rvScale = (int)(1.402 * saturation * fixScale);
 *  int guScale = (int)(-0.344136 * saturation * fixScale);
 *  int gvScale = (int)(-0.714136 * saturation * fixScale);
 *  int buScale = (int)(1.772 * saturation * fixScale);
 *  int yScale = (int)(fixScale);   
 */

/* LIMIT: convert a 16.16 fixed-point value to a byte, with clipping. */
#define LIMIT(x) ((x)>0xffffff?0xff: ((x)<=0xffff?0:((x)>>16)))

static inline void
move_420_block(int yTL, int yTR, int yBL, int yBR, int u, int v, 
           int rowPixels, unsigned char * rgb)
{
    const int rvScale = 91881;
    const int guScale = -22553;
    const int gvScale = -46801;
    const int buScale = 116129;
    const int yScale  = 65536;
    int r, g, b;

    g = guScale * u + gvScale * v;
//  if (force_rgb) {
//      r = buScale * u;
//      b = rvScale * v;
//  } else {
        r = rvScale * v;
        b = buScale * u;
//  }

    yTL *= yScale; yTR *= yScale;
    yBL *= yScale; yBR *= yScale;

    /* Write out top two pixels */
    rgb[0] = LIMIT(b+yTL); rgb[1] = LIMIT(g+yTL);
    rgb[2] = LIMIT(r+yTL);

    rgb[3] = LIMIT(b+yTR); rgb[4] = LIMIT(g+yTR);
    rgb[5] = LIMIT(r+yTR);

    /* Skip down to next line to write out bottom two pixels */
    rgb += 3 * rowPixels;
    rgb[0] = LIMIT(b+yBL); rgb[1] = LIMIT(g+yBL);
    rgb[2] = LIMIT(r+yBL);

    rgb[3] = LIMIT(b+yBR); rgb[4] = LIMIT(g+yBR);
    rgb[5] = LIMIT(r+yBR);
}

static inline void
move_411_block(int yTL, int yTR, int yBL, int yBR, int u, int v, 
           int rowPixels, unsigned char * rgb)
{
    const int rvScale = 91881;
    const int guScale = -22553;
    const int gvScale = -46801;
    const int buScale = 116129;
    const int yScale  = 65536;
    int r, g, b;

    g = guScale * u + gvScale * v;
//  if (force_rgb) {
//      r = buScale * u;
//      b = rvScale * v;
//  } else {
        r = rvScale * v;
        b = buScale * u;
//  }

    yTL *= yScale; yTR *= yScale;
    yBL *= yScale; yBR *= yScale;

    /* Write out top two first pixels */
    rgb[0] = LIMIT(b+yTL); rgb[1] = LIMIT(g+yTL);
    rgb[2] = LIMIT(r+yTL);

    rgb[3] = LIMIT(b+yTR); rgb[4] = LIMIT(g+yTR);
    rgb[5] = LIMIT(r+yTR);

    /* Write out top two last pixels */
    rgb += 6;
    rgb[0] = LIMIT(b+yBL); rgb[1] = LIMIT(g+yBL);
    rgb[2] = LIMIT(r+yBL);

    rgb[3] = LIMIT(b+yBR); rgb[4] = LIMIT(g+yBR);
    rgb[5] = LIMIT(r+yBR);
}

// Consider a YUV420P image of 8x2 pixels.
//
// A plane of Y values    A B C D E F G H
//                        I J K L M N O P
//
// A plane of U values    1   2   3   4 
// A plane of V values    1   2   3   4 ....
//
// The U1/V1 samples correspond to the ABIJ pixels.
//     U2/V2 samples correspond to the CDKL pixels.
//
/* Converts from planar YUV420P to RGB24. */
static void 
yuv420p_to_rgb24(int width, int height,
           unsigned char *pIn0, unsigned char *pOut0)
{
    const int numpix = width * height;
    const int bytes = 24 >> 3;
    int i, j, y00, y01, y10, y11, u, v;
    unsigned char *pY = pIn0;
    unsigned char *pU = pY + numpix;
    unsigned char *pV = pU + numpix / 4;
    unsigned char *pOut = pOut0;

    for (j = 0; j <= height - 2; j += 2) {
        for (i = 0; i <= width - 2; i += 2) {
            y00 = *pY;
            y01 = *(pY + 1);
            y10 = *(pY + width);
            y11 = *(pY + width + 1);
            u = (*pU++) - 128;
            v = (*pV++) - 128;

            move_420_block(y00, y01, y10, y11, u, v,
                       width, pOut);
    
            pY += 2;
            pOut += 2 * bytes;

        }
        pY += width;
        pOut += width * bytes;
    }
}

// Consider a YUV420 image of 6x2 pixels.
//
// A B C D U1 U2
// I J K L V1 V2
//
// The U1/V1 samples correspond to the ABIJ pixels.
//     U2/V2 samples correspond to the CDKL pixels.
//
/* Converts from interlaced YUV420 to RGB24. */
/* [FD] untested... */
static void 
yuv420_to_rgb24(int width, int height,
        unsigned char *pIn0, unsigned char *pOut0)
{
    const int numpix = width * height;
    const int bytes = 24 >> 3;
    int i, j, y00, y01, y10, y11, u, v;
    unsigned char *pY = pIn0;
    unsigned char *pU = pY + 4;
    unsigned char *pV = pU + width;
    unsigned char *pOut = pOut0;

    for (j = 0; j <= height - 2; j += 2) {
        for (i = 0; i <= width - 4; i += 4) {
            y00 = *pY;
            y01 = *(pY + 1);
            y10 = *(pY + width);
            y11 = *(pY + width + 1);
            u = (*pU++) - 128;
            v = (*pV++) - 128;

            move_420_block(y00, y01, y10, y11, u, v,
                       width, pOut);
    
            pY += 2;
            pOut += 2 * bytes;

            y00 = *pY;
            y01 = *(pY + 1);
            y10 = *(pY + width);
            y11 = *(pY + width + 1);
            u = (*pU++) - 128;
            v = (*pV++) - 128;

            move_420_block(y00, y01, y10, y11, u, v,
                       width, pOut);
    
            pY += 4; // skip UV
            pOut += 2 * bytes;

        }
        pY += width;
        pOut += width * bytes;
    }
}

// Consider a YUV411P image of 8x2 pixels.
//
// A plane of Y values as before.
//
// A plane of U values    1       2
//                        3       4
//
// A plane of V values    1       2
//                        3       4
//
// The U1/V1 samples correspond to the ABCD pixels.
//     U2/V2 samples correspond to the EFGH pixels.
//
/* Converts from planar YUV411P to RGB24. */
/* [FD] untested... */
static void 
yuv411p_to_rgb24(int width, int height,
           unsigned char *pIn0, unsigned char *pOut0)
{
    const int numpix = width * height;
    const int bytes = 24 >> 3;
    int i, j, y00, y01, y10, y11, u, v;
    unsigned char *pY = pIn0;
    unsigned char *pU = pY + numpix;
    unsigned char *pV = pU + numpix / 4;
    unsigned char *pOut = pOut0;

    for (j = 0; j <= height; j++) {
        for (i = 0; i <= width - 4; i += 4) {
            y00 = *pY;
            y01 = *(pY + 1);
            y10 = *(pY + 2);
            y11 = *(pY + 3);
            u = (*pU++) - 128;
            v = (*pV++) - 128;

            move_411_block(y00, y01, y10, y11, u, v,
                       width, pOut);
    
            pY += 4;
            pOut += 4 * bytes;

        }
    }
}


static IplImage* icvRetrieveFrameCAM_V4L( CvCaptureCAM_V4L* capture ) {
   /* [FD] this really belongs here */
   if (ioctl(capture->deviceHandle, VIDIOCSYNC, &capture->mmaps[capture->bufferIndex]) == -1) {
     fprintf( stderr, "HIGHGUI ERROR: V4L: Could not SYNC to video stream. %s\n", strerror(errno));
     return(0);
   }

   /* Now get what has already been captured as a IplImage return */

   /* First, reallocate imageData if the frame sized changed */
   if((capture->frame.width != capture->mmaps[capture->bufferIndex].width)
      || (capture->frame.height != capture->mmaps[capture->bufferIndex].height)) {
       cvFree((void**)&capture->frame.imageData);
       cvInitImageHeader( &capture->frame,
              cvSize( capture->captureWindow.width,
                  capture->captureWindow.height ),
              IPL_DEPTH_8U, 3, IPL_ORIGIN_TL, 4 );
       capture->frame.imageData = (char *)cvAlloc(capture->frame.imageSize);
   }
  switch(capture->imageProperties.palette) {
  case VIDEO_PALETTE_RGB24:
    memcpy((char *)capture->frame.imageData, 
       (char *)(capture->memoryMap + capture->memoryBuffer.offsets[capture->bufferIndex]),
       capture->frame.imageSize);
    break;
  case VIDEO_PALETTE_YUV420P:
    yuv420p_to_rgb24(capture->captureWindow.width,
             capture->captureWindow.height,
             (unsigned char*)(capture->memoryMap + capture->memoryBuffer.offsets[capture->bufferIndex]),
             (unsigned char*)capture->frame.imageData);
    break;
  case VIDEO_PALETTE_YUV420:
    yuv420_to_rgb24(capture->captureWindow.width,
          capture->captureWindow.height,
          (unsigned char*)(capture->memoryMap + capture->memoryBuffer.offsets[capture->bufferIndex]),
          (unsigned char*)capture->frame.imageData);
    break;
  case VIDEO_PALETTE_YUV411P:
    yuv411p_to_rgb24(capture->captureWindow.width,
          capture->captureWindow.height,
          (unsigned char*)(capture->memoryMap + capture->memoryBuffer.offsets[capture->bufferIndex]),
          (unsigned char*)capture->frame.imageData);
    break;
  default:
     fprintf( stderr, "HIGHGUI ERROR: V4L: Cannot convert from palette %d to RGB\n");
     return 0;
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

static int icvSetVideoSize( CvCaptureCAM_V4L* capture, int w, int h) {
  int i;

  if (capture==0) return 0;
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
      return 0;
   }

   if (ioctl(capture->deviceHandle, VIDIOCGWIN, &capture->captureWindow) < 0) {
      icvCloseCAM_V4L(capture);
      return 0;
   }
}
 
static int icvSetPropertyCAM_V4L( CvCaptureCAM_V4L* capture, int property_id, double value ){
    static int width = 0, height = 0;
    int retval;

    /* two subsequent calls setting WIDTH and HEIGHT will change the video size */
    /* the first one will return an error, though. */
   switch (property_id) {
       case CV_CAP_PROP_FRAME_WIDTH:
       width = cvRound(value);
       if(width !=0 && height != 0) {
           retval = icvSetVideoSize( capture, width, height);
           width = height = 0;
           return retval;
       }
       break;
       case CV_CAP_PROP_FRAME_HEIGHT:
       height = cvRound(value);
       if(width !=0 && height != 0) {
           retval = icvSetVideoSize( capture, width, height);
           width = height = 0;
           return retval;
       }
       break;
   }
   return 0;
};

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

