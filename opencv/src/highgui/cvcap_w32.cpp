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

static int ffmpeg_initialized = 0;
static CvCaptureFromFile icvCaptureFromFile_FFMPEG_p = 0;
static CvCreateVideoWriter icvCreateVideoWriter_FFMPEG_p = 0;
static CvWriteFrame icvWriteFrame_FFMPEG_p = 0;
static CvReleaseVideoWriter icvReleaseVideoWriter_FFMPEG_p = 0;
static HMODULE ffopencv = 0;

static void
icvInitFFMPEG(void)
{
    if( !ffmpeg_initialized )
    {
#ifdef _DEBUG
#define ffopencv_suffix_dbg "d"
#else
#define ffopencv_suffix_dbg ""
#endif
#if defined EM64T
#define ffopencv_suffix "_64"
#else
#define ffopencv_suffix ""
#endif

#define ffopencv_name_m2(a,b,c) "ffopencv" #a #b #c ffopencv_suffix ffopencv_suffix_dbg ".dll"
#define ffopencv_name_m(a,b,c) ffopencv_name_m2(a,b,c)
        const char* ffopencv_name =
            ffopencv_name_m(CV_MAJOR_VERSION,CV_MINOR_VERSION,CV_SUBMINOR_VERSION);

        ffopencv = LoadLibrary( ffopencv_name );
        if( ffopencv )
        {
            icvCaptureFromFile_FFMPEG_p =
                (CvCaptureFromFile)GetProcAddress(ffopencv, "cvCaptureFromFile_FFMPEG");
            icvCreateVideoWriter_FFMPEG_p =
                (CvCreateVideoWriter)GetProcAddress(ffopencv, "cvCreateVideoWriter_FFMPEG");
            icvWriteFrame_FFMPEG_p =
                (CvWriteFrame)GetProcAddress(ffopencv, "cvWriteFrame_FFMPEG");
            icvReleaseVideoWriter_FFMPEG_p =
                (CvReleaseVideoWriter)GetProcAddress(ffopencv, "cvReleaseVideoWriter_FFMPEG");
        }

        ffmpeg_initialized = 1;
    }
}

CvCapture * cvCaptureFromFile_Win32 (const char * filename)
{
    CvCapture* result = 0;
    
    icvInitFFMPEG();

    if( icvCaptureFromFile_FFMPEG_p )
        result = icvCaptureFromFile_FFMPEG_p(filename);

    if( !result )
        result = cvCaptureFromFile_VFW(filename);

    return result;
}


CV_IMPL CvVideoWriter* cvCreateVideoWriter( const char* filename, int fourcc,
                                            double fps, CvSize frameSize, int is_color )
{
    CvVideoWriter* result = 0;
    
    icvInitFFMPEG();

    // as video writer does not have virtual functions, we should be
    // careful and do not mix FFMPEG & VFW. Therefore, use only one of the interfaces
    if( icvCreateVideoWriter_FFMPEG_p )
        result = icvCreateVideoWriter_FFMPEG_p(filename, fourcc, fps, frameSize, is_color);
    else
        result = cvCreateVideoWriter_VFW(filename, fourcc, fps, frameSize, is_color);

    return result;
}

CV_IMPL int cvWriteFrame( CvVideoWriter* writer, const IplImage* image )
{
    int result;
    
    if( icvCreateVideoWriter_FFMPEG_p && icvWriteFrame_FFMPEG_p )
        result = icvWriteFrame_FFMPEG_p( writer, image );
    else
        result = cvWriteFrame_VFW( writer, image );

    return result;
}

CV_IMPL void cvReleaseVideoWriter( CvVideoWriter** writer )
{
    if( icvCreateVideoWriter_FFMPEG_p && icvReleaseVideoWriter_FFMPEG_p )
        icvReleaseVideoWriter_FFMPEG_p( writer );
    else
        cvReleaseVideoWriter_VFW( writer );
}
