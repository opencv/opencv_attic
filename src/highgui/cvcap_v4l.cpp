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

Fourth Patch: Sept 7, 2005 Csaba Kertesz sign@freemail.hu
For Release:  OpenCV-Linux Beta5 OpenCV-0.9.7

I modified the following:
  - Additional Video4Linux2 support :)
  - Use mmap functions (v4l2)
  - New methods are internal:
    try_palette_v4l2 -> rewrite try_palette for v4l2
    mainloop_v4l2, read_image_v4l2 -> this methods are moved from official v4l2 capture.c example
    try_init_v4l -> device v4l initialisation
    try_init_v4l2 -> device v4l2 initialisation
    autosetup_capture_mode_v4l -> autodetect capture modes for v4l
    autosetup_capture_mode_v4l2 -> autodetect capture modes for v4l2
  - Modifications are according with Video4Linux old codes
  - Video4Linux handling is automatically if it does not recognize a Video4Linux2 device
  - Tested succesful with Logitech Quickcam Express (V4L), Creative Vista (V4L) and Genius VideoCam Notebook (V4L2)
  - Correct source lines with compiler warning messages
  - Information message from v4l/v4l2 detection

Fifth Patch: Sept 7, 2005 Csaba Kertesz sign@freemail.hu
For Release:  OpenCV-Linux Beta5 OpenCV-0.9.7

I modified the following:
  - SN9C10x chip based webcams support
  - New methods are internal:
    bayer2rgb24, sonix_decompress -> decoder routines for SN9C10x decoding from Takafumi Mizuno <taka-qce@ls-a.jp> with his pleasure :)
  - Tested succesful with Genius VideoCam Notebook (V4L2)

Sixth Patch: Sept 10, 2005 Csaba Kertesz sign@freemail.hu
For Release:  OpenCV-Linux Beta5 OpenCV-0.9.7

I added the following:
  - Add capture control support (hue, saturation, brightness, contrast, gain)
  - Get and change V4L capture controls (hue, saturation, brightness, contrast)
  - New method is internal:
    icvSetControl -> set capture controls
  - Tested succesful with Creative Vista (V4L)

make & enjoy!

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

#define CLEAR(x) memset (&(x), 0, sizeof (x))

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

#include <string.h>
#include <stdlib.h>
#include <asm/types.h>          /* for videodev2.h */
#include <assert.h>
#include <sys/stat.h>
#include <sys/ioctl.h>

#ifdef HAVE_CAMV4L2
#include <linux/videodev2.h>
#endif

/* Defaults - If your board can do better, set it here.  Set for the most common type inputs. */
#define DEFAULT_V4L_WIDTH  640
#define DEFAULT_V4L_HEIGHT 480

#define CHANNEL_NUMBER 1
#define MAX_CAMERAS 8

#define MAX_DEVICE_DRIVER_NAME 80

/* Device Capture Objects */

#ifdef HAVE_CAMV4L2

/* V4L2 structure */
struct buffer
{
  void *  start;
  size_t  length;
};

static unsigned int n_buffers = 0;

/* Additional V4L2 pixelformats support for Sonix SN9C10x base webcams */
#ifndef V4L2_PIX_FMT_SBGGR8
#define V4L2_PIX_FMT_SBGGR8  v4l2_fourcc('B','A','8','1') /* 8 BGBG.. GRGR.. */
#endif
#ifndef V4L2_PIX_FMT_SN9C10X
#define V4L2_PIX_FMT_SN9C10X  v4l2_fourcc('S','9','1','0') /* SN9C10x cmpr. */
#endif

#endif  /* HAVE_CAMV4L2 */

int  PALETTE_BGR24 = 0,
     PALETTE_YVU420 = 0,
     PALETTE_YUV411P = 0,
     PALETTE_SBGGR8 = 0,
     PALETTE_SN9C10X = 0;

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

#ifdef HAVE_CAMV4L2

   /* V4L2 variables */
   buffer buffers[10];
   struct v4l2_capability cap;
   struct v4l2_input inp;
   struct v4l2_format form;
   struct v4l2_crop crop;
   struct v4l2_cropcap cropcap;
   struct v4l2_requestbuffers req;
   struct v4l2_jpegcompression compr;
   struct v4l2_control control;
   enum v4l2_buf_type type;

#endif /* HAVE_CAMV4L2 */

}
CvCaptureCAM_V4L;

#ifdef HAVE_CAMV4L2

int V4L2_SUPPORT = 0;

#endif /* HAVE_CAMV4L2 */

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

static void
errno_exit                      (const char *           s)
{
        fprintf (stderr, "%s error %d, %s\n",
                 s, errno, strerror (errno));

        exit (EXIT_FAILURE);
}

#ifdef HAVE_CAMV4L2

// IOCTL handling for V4L2
static int xioctl( int fd, int request, void *arg)
{

  int r;


  do r = ioctl (fd, request, arg);
  while (-1 == r && EINTR == errno);

  return r;

}
 
#endif /* HAVE_CAMV4L2 */

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

#ifdef HAVE_CAMV4L2

int try_palette_v4l2(CvCaptureCAM_V4L* capture, unsigned long colorspace)
{
  CLEAR (capture->form);

  capture->form.type                = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  capture->form.fmt.pix.pixelformat = colorspace;
  capture->form.fmt.pix.field       = V4L2_FIELD_INTERLACED;
  
  if (-1 == xioctl (capture->deviceHandle, VIDIOC_S_FMT, &capture->form))
    errno_exit ("VIDIOC_S_FMT");

  
  if (colorspace != capture->form.fmt.pix.pixelformat)
    return -1;
  else
    return 0;
}

#endif /* HAVE_CAMV4L2 */

int try_init_v4l(CvCaptureCAM_V4L* capture, char *deviceName)
{

  // if detect = -1 then unable to open device
  // if detect = 0 then detected nothing
  // if detect = 1 then V4L device 
  int detect = 0;


  // Test device for V4L compability

  /* Test using an open to see if this new device name really does exists. */
  /* No matter what the name - it still must be opened! */
  capture->deviceHandle = open(deviceName, O_RDWR);


  if (capture->deviceHandle == 0)
  {
    detect = -1;

    icvCloseCAM_V4L(capture);
  }
  
  if (detect == 0)
  {
    /* Query the newly opened device for its capabilities */
    if (ioctl(capture->deviceHandle, VIDIOCGCAP, &capture->capability) < 0)
    {
      detect = 0;

      icvCloseCAM_V4L(capture);
    }
      else
    {
      detect = 1;
    }
  }
  
  return detect;

}

#ifdef HAVE_CAMV4L2

int try_init_v4l2(CvCaptureCAM_V4L* capture, char *deviceName)
{

  // if detect = -1 then unable to open device
  // if detect = 0 then detected nothing
  // if detect = 1 then V4L2 device 
  int detect = 0;


  // Test device for V4L2 compability

  /* Open and test V4L2 device */
  capture->deviceHandle = open (deviceName, O_RDWR /* required */ | O_NONBLOCK, 0);
  


  if (capture->deviceHandle == 0)
  {
    detect = -1;

    icvCloseCAM_V4L(capture);
  }

  if (detect == 0)
  {
    if (-1 == xioctl (capture->deviceHandle, VIDIOC_QUERYCAP, &capture->cap))
    {
      detect = 0;

      icvCloseCAM_V4L(capture);
    }
      else
    {
      capture->capability.type = capture->cap.capabilities;
     
      /* Query channels number */
      if (-1 != xioctl (capture->deviceHandle, VIDIOC_G_INPUT, &capture->capability.channels))
      {
        detect = 1;
      }
    }
  }

  return detect;

}

int autosetup_capture_mode_v4l2(CvCaptureCAM_V4L* capture)
{

  if (try_palette_v4l2(capture, V4L2_PIX_FMT_BGR24) == 0)
  {
    PALETTE_BGR24 = 1;
  }
  else
  if (try_palette_v4l2(capture, V4L2_PIX_FMT_YVU420) == 0)
  {
    PALETTE_YVU420 = 1;
  }
  else
  if (try_palette_v4l2(capture, V4L2_PIX_FMT_YUV411P) == 0)
  {
    PALETTE_YUV411P = 1;
  }
  else
  if (try_palette_v4l2(capture, V4L2_PIX_FMT_SBGGR8) == 0)
  {
    PALETTE_SBGGR8 = 1;
  }
  else
  if (try_palette_v4l2(capture, V4L2_PIX_FMT_SN9C10X) == 0)
  {
    PALETTE_SN9C10X = 1;

    if (-1 == xioctl (capture->deviceHandle, VIDIOC_G_JPEGCOMP, &capture->compr))
      errno_exit ("VIDIOC_G_JPEGCOMP");
	 
    capture->compr.quality = 0;

    if (-1 == xioctl (capture->deviceHandle, VIDIOC_S_JPEGCOMP, &capture->compr))
         errno_exit ("VIDIOC_S_JPEGCOMP");

  } else
  {

    fprintf( stderr, "HIGHGUI ERROR: V4L2: Unable to set palette.\n");

    icvCloseCAM_V4L(capture);
    
    return -1;

  }
  
  return 0;

}

#endif /* HAVE_CAMV4L2 */

int autosetup_capture_mode_v4l(CvCaptureCAM_V4L* capture)
{

  if(ioctl(capture->deviceHandle, VIDIOCGPICT, &capture->imageProperties) < 0) {
     fprintf( stderr, "HIGHGUI ERROR: V4L: Unable to determine size of incoming image\n");
     icvCloseCAM_V4L(capture);
     return -1;
  }

  /* Yet MORE things that might have to be changes with your frame capture card */
  /* This sets the scale to the center of a 2^16 number */
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
    return -1;
  }

  return 0;

}


CvCapture* icvOpenCAM_V4L( int index ) {
   static int autoindex=0;
   int detect_v4l = 0;

#ifdef HAVE_CAMV4L2
   int detect_v4l2 = 0;
#endif /* HAVE_CAMV4L2 */

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
   
#ifdef HAVE_CAMV4L2
   V4L2_SUPPORT = 0;
#endif /* HAVE_CAMV4L2 */

   detect_v4l = try_init_v4l(capture, deviceName);

#ifdef HAVE_CAMV4L2
   if (detect_v4l <= 0)
     detect_v4l2 = try_init_v4l2(capture, deviceName);
#endif /* HAVE_CAMV4L2 */

   //if (detect_v4l == 1)
   //{
   //  fprintf (stderr, "HIGHGUI Detect: V4L: device %s\n", deviceName);
   //}

   if ((detect_v4l == -1)
#ifdef HAVE_CAMV4L2
       && (detect_v4l2 == -1)
#endif /* HAVE_CAMV4L2 */
       )
   {
     fprintf (stderr, "HIGHGUI ERROR: V4L"
#ifdef HAVE_CAMV4L2
	      "/V4L2"
#endif /* HAVE_CAMV4L2 */
	      ": device %s: Unable to open for READ ONLY\n", deviceName);

     return NULL;
   }

   if ((detect_v4l <= 0)
#ifdef HAVE_CAMV4L2
       && (detect_v4l2 <= 0)
#endif /* HAVE_CAMV4L2 */
       )
   {
     fprintf (stderr, "HIGHGUI ERROR: V4L"
#ifdef HAVE_CAMV4L2
	      "/V4L2"
#endif /* HAVE_CAMV4L2 */
	      ": device %s: Unable to query number of channels\n", deviceName);

     return NULL;
   }
   
#ifdef HAVE_CAMV4L2
   if (detect_v4l2 == 1)
   {
     //fprintf (stderr, "HIGHGUI Detect: V4L2: device %s\n", deviceName);
     V4L2_SUPPORT = 1;
   }


   /* Can this device capture video to memory? */
   if (V4L2_SUPPORT == 1)
   {

     if ((capture->cap.capabilities & V4L2_CAP_VIDEO_CAPTURE) == 0) {
      /* Nope. */
      fprintf( stderr, "HIGHGUI ERROR: V4L2: device %s is unable to capture video memory.\n",deviceName);
      icvCloseCAM_V4L(capture);
      return NULL;
     }

   } else
#endif /* HAVE_CAMV4L2 */
   {
     if ((capture->capability.type & VID_TYPE_CAPTURE) == 0) {
       /* Nope. */
       fprintf( stderr, "HIGHGUI ERROR: V4L: "
		"device %s is unable to capture video memory.\n",deviceName);
       icvCloseCAM_V4L(capture);
       return NULL;
     }

   }


   /* The following code sets the CHANNEL_NUMBER of the video input.  Some video sources
   have sub "Channel Numbers".  For a typical V4L TV capture card, this is usually 1.
   I myself am using a simple NTSC video input capture card that uses the value of 1.
   If you are not in North America or have a different video standard, you WILL have to change
   the following settings and recompile/reinstall.  This set of settings is based on
   the most commonly encountered input video source types (like my bttv card) */

#ifdef HAVE_CAMV4L2

   if (V4L2_SUPPORT == 1)
   {

     if(capture->inp.index > 0) {
       capture->inp.index = CHANNEL_NUMBER;
       /* Set only channel number to CHANNEL_NUMBER */
       /* V4L2 have a status field from selected video mode */
       if (-1 == xioctl (capture->deviceHandle, VIDIOC_ENUMINPUT, &capture->inp))
       {
         fprintf (stderr, "HIGHGUI ERROR: V4L2: Aren't able to set channel number\n");
     
         exit (EXIT_FAILURE);
       }
     } /* End if */

   } else
#endif /* HAVE_CAMV4L2 */
   {

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

   }

#ifdef HAVE_CAMV4L2

   /* Find Window info */
   if (V4L2_SUPPORT == 1)
   {

     capture->form.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        
     if (-1 == xioctl (capture->deviceHandle, VIDIOC_G_FMT, &capture->form)) {
       fprintf( stderr, "HIGHGUI ERROR: V4L2: Could not obtain specifics of capture window.\n\n");
       icvCloseCAM_V4L(capture);
       return NULL;
     }

   } else
#endif /* HAVE_CAMV4L2 */
   {

     if(ioctl(capture->deviceHandle, VIDIOCGWIN, &capture->captureWindow) == -1) {
       fprintf( stderr, "HIGHGUI ERROR: V4L: "
		"Could not obtain specifics of capture window.\n\n");
       icvCloseCAM_V4L(capture);
       return NULL;
     }

   }

#ifdef HAVE_CAMV4L2

   if (V4L2_SUPPORT == 0)
   {
     /* Don't scale the image by default if there is hardware scaling */
     /* Else, choose the image size closest to DEFAULT_V4L_WIDTH x DEFAULT_V4L_HEIGHT */
     if(((capture->capability.type & VID_TYPE_SCALES) == 0) &&
        (capture->captureWindow.width < DEFAULT_V4L_WIDTH) &&
        (capture->captureWindow.height < DEFAULT_V4L_HEIGHT)) {
       /* If your card can open a bigger window, it will be attempted here.  I have a version
      of this program that returns the "Property" set ability, but it is buggy.  It also
      handles multiple cameras for stereo work.  Contact TW if you would like to use it */
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
         fprintf( stderr, "HIGHGUI ERROR: V4L/V4L2: Could not obtain specifics of capture window.\n\n");
         icvCloseCAM_V4L(capture);
         return NULL;
       }
     }

   }

#endif /* HAVE_CAMV4L2 */

#ifdef HAVE_CAMV4L2

   /* Find Picture info */
   if (V4L2_SUPPORT == 1)
   {
   
     if (autosetup_capture_mode_v4l2(capture) == -1)
       return NULL;

     icvSetVideoSize(capture, DEFAULT_V4L_WIDTH, DEFAULT_V4L_HEIGHT);

     unsigned int min;

     /* Buggy driver paranoia. */
     min = capture->form.fmt.pix.width * 2;

     if (capture->form.fmt.pix.bytesperline < min)
       capture->form.fmt.pix.bytesperline = min;

     min = capture->form.fmt.pix.bytesperline * capture->form.fmt.pix.height;
 
     if (capture->form.fmt.pix.sizeimage < min)
       capture->form.fmt.pix.sizeimage = min;

   } else
#endif /* HAVE_CAMV4L2 */
   {

     if (autosetup_capture_mode_v4l(capture) == -1)
       return NULL;

   }

#ifdef HAVE_CAMV4L2

   /* Setup mapped memory io */

   if (V4L2_SUPPORT == 1)
   {

     CLEAR (capture->req);

     capture->req.count               = 2;
     capture->req.type                = V4L2_BUF_TYPE_VIDEO_CAPTURE;
     capture->req.memory              = V4L2_MEMORY_MMAP;

     if (-1 == xioctl (capture->deviceHandle, VIDIOC_REQBUFS, &capture->req))
     {
       if (EINVAL == errno)
       {
         fprintf (stderr, "%s does not support memory mapping\n", deviceName);

         exit (EXIT_FAILURE);
       } else {
         errno_exit ("VIDIOC_REQBUFS");
       }
     }

     if (capture->req.count < 2)
     {
       fprintf (stderr, "Insufficient buffer memory on %s\n", deviceName);

       exit (EXIT_FAILURE);
     }

     for (n_buffers = 0; n_buffers < capture->req.count; ++n_buffers)
     {
       struct v4l2_buffer buf;

       CLEAR (buf);

       buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
       buf.memory = V4L2_MEMORY_MMAP;
       buf.index = n_buffers;

       if (-1 == xioctl (capture->deviceHandle, VIDIOC_QUERYBUF, &buf))
         errno_exit ("VIDIOC_QUERYBUF");

       capture->buffers[n_buffers].length = buf.length;
       capture->buffers[n_buffers].start =
         mmap (NULL /* start anywhere */,
               buf.length,
               PROT_READ | PROT_WRITE /* required */,
               MAP_SHARED /* recommended */,
               capture->deviceHandle, buf.m.offset);

       if (MAP_FAILED == capture->buffers[n_buffers].start)
         errno_exit ("mmap");
     }

   } else
#endif /* HAVE_CAMV4L2 */
   {

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

#ifdef HAVE_CAMV4L2

static int read_frame_v4l2(CvCaptureCAM_V4L* capture)
{
        struct v4l2_buffer buf;


        CLEAR (buf);

       	buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
       	buf.memory = V4L2_MEMORY_MMAP;

    		if (-1 == xioctl (capture->deviceHandle, VIDIOC_DQBUF, &buf)) {
        errno_exit ("VIDIOC_DQBUF");
            		switch (errno) {
            		case EAGAIN:
                    		return 0;

      case EIO:
        /* Could ignore EIO, see spec. */

        /* fall through */

      default:
        errno_exit ("VIDIOC_DQBUF");
      }
    }

   assert(buf.index < capture->req.count);
   
   capture->bufferIndex = buf.index;

   if (-1 == xioctl (capture->deviceHandle, VIDIOC_QBUF, &buf))
     errno_exit ("VIDIOC_QBUF");

   return 1;
}

static void mainloop_v4l2(CvCaptureCAM_V4L* capture)
{
  unsigned int count;

        count = 1;

        while (count-- > 0) {
                for (;;) {
                        fd_set fds;
                        struct timeval tv;
                        int r;

                        FD_ZERO (&fds);
                        FD_SET (capture->deviceHandle, &fds);

                        /* Timeout. */
                        tv.tv_sec = 2;
                        tv.tv_usec = 0;

                        r = select (capture->deviceHandle+1, &fds, NULL, NULL, &tv);

                        if (-1 == r) {
                                if (EINTR == errno)
                                        continue;

                                errno_exit ("select");
                        }

                        if (0 == r) {
                                fprintf (stderr, "select timeout\n");
//                                exit (EXIT_FAILURE);
                        }

      if (read_frame_v4l2 (capture))
                    		break;
  
      /* EAGAIN - continue select loop. */
                }
        }
}

#endif /* HAVE_CAMV4L2 */

static int icvGrabFrameCAM_V4L(CvCaptureCAM_V4L* capture) {

   if (capture->FirstCapture) { 
      /* Some general initialization must take place the first time through */
      capture->FirstCapture = 0;
      /* This is just a technicality, but all buffers must be filled up before any
         staggered SYNC is applied.  SO, filler up. (see V4L HowTo) */

#ifdef HAVE_CAMV4L2

      if (V4L2_SUPPORT == 1)
      {

        for (capture->bufferIndex = 0;
             capture->bufferIndex < ((int)capture->req.count-1);
             ++capture->bufferIndex)
        {

          struct v4l2_buffer buf;

          CLEAR (buf);

          buf.type        = V4L2_BUF_TYPE_VIDEO_CAPTURE;
          buf.memory      = V4L2_MEMORY_MMAP;
          buf.index       = (unsigned long)capture->bufferIndex;

          if (-1 == xioctl (capture->deviceHandle, VIDIOC_QBUF, &buf))
            errno_exit ("VIDIOC_QBUF");

          capture->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

          if (-1 == xioctl (capture->deviceHandle, VIDIOC_STREAMON, &capture->type))
            errno_exit ("VIDIOC_STREAMON");
        }

      } else
#endif /* HAVE_CAMV4L2 */
      {

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
   }

#ifdef HAVE_CAMV4L2

   if (V4L2_SUPPORT == 1)
   {

     mainloop_v4l2(capture);

   } else
#endif /* HAVE_CAMV4L2 */
   {
   
     capture->mmaps[capture->bufferIndex].frame  = capture->bufferIndex;
     capture->mmaps[capture->bufferIndex].width  = capture->captureWindow.width;
     capture->mmaps[capture->bufferIndex].height = capture->captureWindow.height;
     capture->mmaps[capture->bufferIndex].format = capture->imageProperties.palette;

     if (ioctl(capture->deviceHandle, VIDIOCMCAPTURE, &capture->mmaps[capture->bufferIndex]) == -1) {
        fprintf( stderr, "HIGHGUI ERROR: V4L: Unable to start next buffer cycle. %s\n", strerror(errno));
        return(0);

     }

     if (capture->bufferIndex == capture->memoryBuffer.frames) {
        capture->bufferIndex = 0;
     }

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


/*
 * BAYER2RGB24 ROUTINE TAKEN FROM:
 *
 * Sonix SN9C10x based webcam basic I/F routines
 * Takafumi Mizuno <taka-qce@ls-a.jp>
 *
 */

void bayer2rgb24(long int WIDTH, long int HEIGHT, unsigned char *src, unsigned char *dst)
{
    long int i;
    unsigned char *rawpt, *scanpt;
    long int size;

    rawpt = src;
    scanpt = dst;
    size = WIDTH*HEIGHT;

    for ( i = 0; i < size; i++ ) {
  if ( (i/WIDTH) % 2 == 0 ) {
      if ( (i % 2) == 0 ) {
    /* B */
    if ( (i > WIDTH) && ((i % WIDTH) > 0) ) {
        *scanpt++ = (*(rawpt-WIDTH-1)+*(rawpt-WIDTH+1)+
         *(rawpt+WIDTH-1)+*(rawpt+WIDTH+1))/4;	/* R */
        *scanpt++ = (*(rawpt-1)+*(rawpt+1)+
         *(rawpt+WIDTH)+*(rawpt-WIDTH))/4;	/* G */
        *scanpt++ = *rawpt;					/* B */
    } else {
        /* first line or left column */
        *scanpt++ = *(rawpt+WIDTH+1);		/* R */
        *scanpt++ = (*(rawpt+1)+*(rawpt+WIDTH))/2;	/* G */
        *scanpt++ = *rawpt;				/* B */
    }
      } else {
    /* (B)G */
    if ( (i > WIDTH) && ((i % WIDTH) < (WIDTH-1)) ) {
        *scanpt++ = (*(rawpt+WIDTH)+*(rawpt-WIDTH))/2;	/* R */
        *scanpt++ = *rawpt;					/* G */
        *scanpt++ = (*(rawpt-1)+*(rawpt+1))/2;		/* B */
    } else {
        /* first line or right column */
        *scanpt++ = *(rawpt+WIDTH);	/* R */
        *scanpt++ = *rawpt;		/* G */
        *scanpt++ = *(rawpt-1);	/* B */
    }
      }
  } else {
      if ( (i % 2) == 0 ) {
    /* G(R) */
    if ( (i < (WIDTH*(HEIGHT-1))) && ((i % WIDTH) > 0) ) {
        *scanpt++ = (*(rawpt-1)+*(rawpt+1))/2;		/* R */
        *scanpt++ = *rawpt;					/* G */
        *scanpt++ = (*(rawpt+WIDTH)+*(rawpt-WIDTH))/2;	/* B */
    } else {
        /* bottom line or left column */
        *scanpt++ = *(rawpt+1);		/* R */
        *scanpt++ = *rawpt;			/* G */
        *scanpt++ = *(rawpt-WIDTH);		/* B */
    }
      } else {
    /* R */
    if ( i < (WIDTH*(HEIGHT-1)) && ((i % WIDTH) < (WIDTH-1)) ) {
        *scanpt++ = *rawpt;					/* R */
        *scanpt++ = (*(rawpt-1)+*(rawpt+1)+
         *(rawpt-WIDTH)+*(rawpt+WIDTH))/4;	/* G */
        *scanpt++ = (*(rawpt-WIDTH-1)+*(rawpt-WIDTH+1)+
         *(rawpt+WIDTH-1)+*(rawpt+WIDTH+1))/4;	/* B */
    } else {
        /* bottom line or right column */
        *scanpt++ = *rawpt;				/* R */
        *scanpt++ = (*(rawpt-1)+*(rawpt-WIDTH))/2;	/* G */
        *scanpt++ = *(rawpt-WIDTH-1);		/* B */
    }
      }
  }
  rawpt++;
    }

}


#define CLAMP(x)	((x)<0?0:((x)>255)?255:(x))

typedef struct {
  int is_abs;
  int len;
  int val;
} code_table_t;


/* local storage */
static code_table_t table[256];
static int init_done = 0;


/*
  sonix_decompress_init
  =====================
    pre-calculates a locally stored table for efficient huffman-decoding.

  Each entry at index x in the table represents the codeword
  present at the MSB of byte x.

*/
void sonix_decompress_init(void)
{
  int i;
  int is_abs, val, len;

  for (i = 0; i < 256; i++) {
    is_abs = 0;
    val = 0;
    len = 0;
    if ((i & 0x80) == 0) {
      /* code 0 */
      val = 0;
      len = 1;
    }
    else if ((i & 0xE0) == 0x80) {
      /* code 100 */
      val = +4;
      len = 3;
    }
    else if ((i & 0xE0) == 0xA0) {
      /* code 101 */
      val = -4;
      len = 3;
    }
    else if ((i & 0xF0) == 0xD0) {
      /* code 1101 */
      val = +11;
      len = 4;
    }
    else if ((i & 0xF0) == 0xF0) {
      /* code 1111 */
      val = -11;
      len = 4;
    }
    else if ((i & 0xF8) == 0xC8) {
      /* code 11001 */
      val = +20;
      len = 5;
    }
    else if ((i & 0xFC) == 0xC0) {
      /* code 110000 */
      val = -20;
      len = 6;
    }
    else if ((i & 0xFC) == 0xC4) {
      /* code 110001xx: unknown */
      val = 0;
      len = 8;
    }
    else if ((i & 0xF0) == 0xE0) {
      /* code 1110xxxx */
      is_abs = 1;
      val = (i & 0x0F) << 4;
      len = 8;
    }
    table[i].is_abs = is_abs;
    table[i].val = val;
    table[i].len = len;
  }

  init_done = 1;
}


/*
  sonix_decompress
  ================
    decompresses an image encoded by a SN9C101 camera controller chip.

  IN	width
    height
    inp		pointer to compressed frame (with header already stripped)
  OUT	outp	pointer to decompressed frame

  Returns 0 if the operation was successful.
  Returns <0 if operation failed.

*/
int sonix_decompress(int width, int height, unsigned char *inp, unsigned char *outp)
{
  int row, col;
  int val;
  int bitpos;
  unsigned char code;
  unsigned char *addr;

  if (!init_done) {
    /* do sonix_decompress_init first! */
    return -1;
  }

  bitpos = 0;
  for (row = 0; row < height; row++) {

    col = 0;



    /* first two pixels in first two rows are stored as raw 8-bit */
    if (row < 2) {
      addr = inp + (bitpos >> 3);
      code = (addr[0] << (bitpos & 7)) | (addr[1] >> (8 - (bitpos & 7)));
      bitpos += 8;
      *outp++ = code;

      addr = inp + (bitpos >> 3);
      code = (addr[0] << (bitpos & 7)) | (addr[1] >> (8 - (bitpos & 7)));
      bitpos += 8;
      *outp++ = code;

      col += 2;
    }

    while (col < width) {
      /* get bitcode from bitstream */
      addr = inp + (bitpos >> 3);
      code = (addr[0] << (bitpos & 7)) | (addr[1] >> (8 - (bitpos & 7)));

      /* update bit position */
      bitpos += table[code].len;

      /* calculate pixel value */
      val = table[code].val;
      if (!table[code].is_abs) {
        /* value is relative to top and left pixel */
        if (col < 2) {
          /* left column: relative to top pixel */
          val += outp[-2*width];
        }
        else if (row < 2) {
          /* top row: relative to left pixel */
          val += outp[-2];
        }
        else {
          /* main area: average of left pixel and top pixel */
          val += (outp[-2] + outp[-2*width]) / 2;
        }
      }

      /* store pixel */
      *outp++ = CLAMP(val);
      col++;
    }
  }

  return 0;
}


static IplImage* icvRetrieveFrameCAM_V4L( CvCaptureCAM_V4L* capture) {

#ifdef HAVE_CAMV4L2
  if (V4L2_SUPPORT == 0)
#endif /* HAVE_CAMV4L2 */
  {

    /* [FD] this really belongs here */
    if (ioctl(capture->deviceHandle, VIDIOCSYNC, &capture->mmaps[capture->bufferIndex]) == -1) {
      fprintf( stderr, "HIGHGUI ERROR: V4L: Could not SYNC to video stream. %s\n", strerror(errno));
      return(0);
    }

  }

   /* Now get what has already been captured as a IplImage return */

   /* First, reallocate imageData if the frame sized changed */

#ifdef HAVE_CAMV4L2

  if (V4L2_SUPPORT == 1)
  {

    if(((unsigned long)capture->frame.width != capture->form.fmt.pix.width)
       || ((unsigned long)capture->frame.height != capture->form.fmt.pix.height)) {
        cvFree((void**)&capture->frame.imageData);
        cvInitImageHeader( &capture->frame,
              cvSize( capture->form.fmt.pix.width,
                  capture->form.fmt.pix.height ),
              IPL_DEPTH_8U, 3, IPL_ORIGIN_TL, 4 );
       capture->frame.imageData = (char *)cvAlloc(capture->frame.imageSize);
    }

  } else
#endif /* HAVE_CAMV4L2 */
  {

    if((capture->frame.width != capture->mmaps[capture->bufferIndex].width)
      || (capture->frame.height != capture->mmaps[capture->bufferIndex].height)) {
       cvFree((void**)&capture->frame.imageData);
       cvInitImageHeader( &capture->frame,
              cvSize( capture->captureWindow.width,
                  capture->captureWindow.height ),
              IPL_DEPTH_8U, 3, IPL_ORIGIN_TL, 4 );
       capture->frame.imageData = (char *)cvAlloc(capture->frame.imageSize);
    }
 
  }

#ifdef HAVE_CAMV4L2

  if (V4L2_SUPPORT == 1)
  {

    if (PALETTE_BGR24 == 1)
      memcpy((char *)capture->frame.imageData, 
             (char *)capture->buffers[capture->bufferIndex].start,
             capture->frame.imageSize);

    if (PALETTE_YVU420 == 1)
      yuv420p_to_rgb24(capture->form.fmt.pix.width,
                       capture->form.fmt.pix.height,
                       (unsigned char*)(capture->buffers[capture->bufferIndex].start),
                       (unsigned char*)capture->frame.imageData);

    if (PALETTE_YUV411P == 1)
      yuv411p_to_rgb24(capture->form.fmt.pix.width,
                       capture->form.fmt.pix.height,
                       (unsigned char*)(capture->buffers[capture->bufferIndex].start),
                       (unsigned char*)capture->frame.imageData);

    if (PALETTE_SBGGR8 == 1)
    {
      bayer2rgb24(capture->form.fmt.pix.width,
                  capture->form.fmt.pix.height,
                  (unsigned char*)capture->buffers[capture->bufferIndex].start,
                  (unsigned char*)capture->frame.imageData);
      
      /* Change BGR to RGB */
      unsigned char trash;
      
      for (int i = 0; i < capture->frame.imageSize; i++)
        if (i % 3 == 1)
        {
          trash = capture->frame.imageData[i+1];
         
          capture->frame.imageData[i+1] = capture->frame.imageData[i-1];
        
          capture->frame.imageData[i-1] = trash;
        }
    }

    if (PALETTE_SN9C10X == 1)
    {
      sonix_decompress_init();
      
      sonix_decompress(capture->form.fmt.pix.width,
                       capture->form.fmt.pix.height,
                       (unsigned char*)capture->buffers[capture->bufferIndex].start,
                       (unsigned char*)capture->buffers[(capture->bufferIndex+1) % capture->req.count].start);

      bayer2rgb24(capture->form.fmt.pix.width,
                  capture->form.fmt.pix.height,
                  (unsigned char*)capture->buffers[(capture->bufferIndex+1) % capture->req.count].start,
                  (unsigned char*)capture->frame.imageData);

      /* Change BGR to RGB */
      unsigned char trash;
      
      for (int i = 0; i < capture->frame.imageSize; i++)
        if (i % 3 == 1)
        {
          trash = capture->frame.imageData[i+1];
         
          capture->frame.imageData[i+1] = capture->frame.imageData[i-1];
        
          capture->frame.imageData[i-1] = trash;
        }
    }

  } else
#endif /* HAVE_CAMV4L2 */
  {

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
        fprintf( stderr,
		 "HIGHGUI ERROR: V4L: Cannot convert from palette %d to RGB\n",
		 capture->imageProperties.palette);

        return 0;
    }

  }

   return(&capture->frame);
}

static double icvGetPropertyCAM_V4L (CvCaptureCAM_V4L* capture,
				     int property_id ) {

  double retval = -1;

#ifdef HAVE_CAMV4L2

  if (V4L2_SUPPORT == 1)
  {

      if (-1 == xioctl (capture->deviceHandle, VIDIOC_G_FMT, &capture->form))
	  errno_exit ("VIDIOC_S_FMT");

      switch (property_id) {
      case CV_CAP_PROP_FRAME_WIDTH:
	  retval = capture->form.fmt.pix.width;
	  break;
      case CV_CAP_PROP_FRAME_HEIGHT:
	  retval = capture->form.fmt.pix.height;
	  break;
      }
  } else
#endif /* HAVE_CAMV4L2 */
  {

    if (ioctl (capture->deviceHandle,
	       VIDIOCGWIN, &capture->captureWindow) < 0) {
	fprintf (stderr,
		 "HIGHGUI ERROR: V4L: "
		 "Unable to determine size of incoming image\n");
	icvCloseCAM_V4L(capture);
	return -1;
    }

    if (ioctl (capture->deviceHandle,
	       VIDIOCGPICT, &capture->imageProperties) < 0)
    {
	fprintf (stderr,
		 "HIGHGUI ERROR: V4L: "
		 "Unable to get properties of device\n");
	icvCloseCAM_V4L(capture);
	return -1;
    }
    
    switch (property_id) {
    case CV_CAP_PROP_FRAME_WIDTH:
	retval = capture->captureWindow.width;
	break;
    case CV_CAP_PROP_FRAME_HEIGHT:
	retval = capture->captureWindow.height;
	break;
    case CV_CAP_PROP_BRIGHTNESS:
	retval = capture->imageProperties.brightness;
	break;
    case CV_CAP_PROP_CONTRAST:
	retval = capture->imageProperties.contrast;
	break;
    case CV_CAP_PROP_SATURATION:
	retval = capture->imageProperties.colour;
	break;
    case CV_CAP_PROP_HUE:
	retval = capture->imageProperties.hue;
	break;
    case CV_CAP_PROP_GAIN:
	fprintf(stderr,
		"HIGHGUI ERROR: V4L: Gain control in V4L is not supported\n");
	return -1;
	break;
    default:
	fprintf(stderr,
		"HIGHGUI ERROR: V4L: getting property #%d is not supported\n",
		property_id);

    }

    return retval;

  }

};

static int icvSetVideoSize( CvCaptureCAM_V4L* capture, int w, int h) {

#ifdef HAVE_CAMV4L2

  if (V4L2_SUPPORT == 1)
  {

    capture->crop.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    capture->crop.c.left       = 0; 
    capture->crop.c.top        = 0; 
    capture->crop.c.height     = h*24;
    capture->crop.c.width      = w*24;

    if (-1 == xioctl (capture->deviceHandle, VIDIOC_S_CROP, &capture->crop))
      errno_exit ("VIDIOC_S_CROP");

    capture->form.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    capture->form.fmt.pix.width = w; 
    capture->form.fmt.pix.height = h;
    capture->form.fmt.win.chromakey = 0;
    capture->form.fmt.win.field = V4L2_FIELD_ANY;
    capture->form.fmt.win.clips = 0;
    capture->form.fmt.win.clipcount = 0;
    capture->form.fmt.pix.field = V4L2_FIELD_NONE;
  
    if (-1 == xioctl (capture->deviceHandle, VIDIOC_S_FMT, &capture->form))
      errno_exit ("VIDIOC_S_FMT");

    /* Get window info again, to get the real value */
      
    if (-1 == xioctl (capture->deviceHandle, VIDIOC_G_FMT, &capture->form))
    {
      fprintf(stderr, "HIGHGUI ERROR: V4L/V4L2: Could not obtain specifics of capture window.\n\n");

      icvCloseCAM_V4L(capture);

      return 0;
    }

    return 0;

  } else
#endif /* HAVE_CAMV4L2 */
  {
  
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
   
  return 0;

}

static int icvSetControl (CvCaptureCAM_V4L* capture,
			  int property_id, double value) {
  
#ifdef HAVE_CAMV4L2

  if (V4L2_SUPPORT == 1)
  {

  } else
#endif /* HAVE_CAMV4L2 */
  {

    int v4l_value = (int)value;
    
    if (v4l_value < 0)
      v4l_value = 0;
    if (v4l_value > 0xffff)
      v4l_value = 0xffff;
  
    switch (property_id) {
    case CV_CAP_PROP_BRIGHTNESS:
      capture->imageProperties.brightness = v4l_value;
      break;
    case CV_CAP_PROP_CONTRAST:
      capture->imageProperties.contrast = v4l_value;
      break;
    case CV_CAP_PROP_SATURATION:
      capture->imageProperties.colour = v4l_value;
      break;
    case CV_CAP_PROP_HUE:
      capture->imageProperties.hue = v4l_value;
      break;
    case CV_CAP_PROP_GAIN:
	fprintf(stderr,
		"HIGHGUI ERROR: V4L: Gain control in V4L is not supported\n");
	return -1;
    default:
	fprintf(stderr,
		"HIGHGUI ERROR: V4L: property #%d is not supported\n",
		property_id);
    }
    
    if (ioctl(capture->deviceHandle, VIDIOCSPICT, &capture->imageProperties)
	< 0)
    {
       fprintf(stderr,
	       "HIGHGUI ERROR: V4L: Unable to set video informations\n");
       icvCloseCAM_V4L(capture);
       return -1;
    }
    
    return 0;
  }
   
  return 0;
}
 
static int icvSetPropertyCAM_V4L( CvCaptureCAM_V4L* capture,
				  int property_id, double value ){
    static int width = 0, height = 0;
    int retval;

    /* initialization */
    retval = 0;

    /* two subsequent calls setting WIDTH and HEIGHT will change
       the video size */
    /* the first one will return an error, though. */

    switch (property_id) {
    case CV_CAP_PROP_FRAME_WIDTH:
	width = cvRound(value);
	if(width !=0 && height != 0) {
	    retval = icvSetVideoSize( capture, width, height);
	    width = height = 0;
	}
	break;
    case CV_CAP_PROP_FRAME_HEIGHT:
	height = cvRound(value);
	if(width !=0 && height != 0) {
	    retval = icvSetVideoSize( capture, width, height);
	    width = height = 0;
	}
	break;
    case CV_CAP_PROP_BRIGHTNESS:
    case CV_CAP_PROP_CONTRAST:
    case CV_CAP_PROP_SATURATION:
    case CV_CAP_PROP_HUE:
    case CV_CAP_PROP_GAIN:
	retval = icvSetControl(capture, property_id, value);
	break;
    default:
	fprintf(stderr,
		"HIGHGUI ERROR: V4L: setting property #%d is not supported\n",
		property_id);
    }

    /* return the the status */
    return retval;
}

static void icvCloseCAM_V4L( CvCaptureCAM_V4L* capture ){
   /* Deallocate space - Hopefully, no leaks */ 

   if (capture)
   {

#ifdef HAVE_CAMV4L2
     if (V4L2_SUPPORT == 0)
#endif /* HAVE_CAMV4L2 */
     {

       if (capture->mmaps)
         free(capture->mmaps);
       if (capture->memoryMap)
         munmap(capture->memoryMap, capture->memoryBuffer.size);

     }
#ifdef HAVE_CAMV4L2
     else {

       for (unsigned int n_buffers = 0; n_buffers < capture->req.count; ++n_buffers)
       {
         if (-1 == munmap (capture->buffers[n_buffers].start, capture->buffers[n_buffers].length))
           errno_exit ("munmap");
       }

     }
#endif /* HAVE_CAMV4L2 */

     if (capture->deviceHandle > 0) close(capture->deviceHandle);

     if (capture->frame.imageData) cvFree((void**)&capture->frame.imageData);
      //cvFree((void **)capture);
   }
};

#endif
