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

extern "C"
{
typedef CvCapture* (*CvCreateFileCapture_Plugin)( const char* filename );
typedef CvCapture* (*CvCreateCameraCapture_Plugin)( int index );
typedef void (*CvReleaseCapture_Plugin)( CvCapture** capture );
typedef CvVideoWriter* (*CvCreateVideoWriter_Plugin)( const char* filename, int fourcc,
                                                      double fps, CvSize frameSize, int isColor );
typedef void (*CvReleaseVideoWriter_Plugin)( CvVideoWriter** writer );
}

static HMODULE icvFFOpenCV = 0;
static CvCreateFileCapture_Plugin icvCreateFileCapture_FFMPEG_p = 0;
static CvReleaseCapture_Plugin icvReleaseCapture_FFMPEG_p = 0;
static CvCreateVideoWriter_Plugin icvCreateVideoWriter_FFMPEG_p = 0;
static CvReleaseVideoWriter_Plugin icvReleaseVideoWriter_FFMPEG_p = 0;


static void
icvInitFFMPEG(void)
{
    static int ffmpegInitialized = 0;
    if( !ffmpegInitialized )
    {
#ifdef _DEBUG
//#define ffopencv_suffix_dbg "d"
#define ffopencv_suffix_dbg ""
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

        icvFFOpenCV = LoadLibrary( ffopencv_name );
        if( icvFFOpenCV )
        {
            icvCreateFileCapture_FFMPEG_p =
                (CvCreateFileCapture_Plugin)GetProcAddress(icvFFOpenCV, "cvCreateFileCapture_FFMPEG");
            icvCreateVideoWriter_FFMPEG_p =
                (CvCreateVideoWriter_Plugin)GetProcAddress(icvFFOpenCV, "cvCreateVideoWriter_FFMPEG");
            icvReleaseCapture_FFMPEG_p =
                (CvReleaseCapture_Plugin)GetProcAddress(icvFFOpenCV, "cvReleaseCapture_FFMPEG");
            icvReleaseVideoWriter_FFMPEG_p =
                (CvReleaseVideoWriter_Plugin)GetProcAddress(icvFFOpenCV, "cvReleaseVideoWriter_FFMPEG");
        }
        ffmpegInitialized = 1;
    }
}


class CvCapture_FFMPEG_proxy : public CvCapture
{
public:
    CvCapture_FFMPEG_proxy() { ffmpegCapture = 0; }
    virtual ~CvCapture_FFMPEG_proxy() { close(); }

    virtual double getProperty(int propId)
    {
        return ffmpegCapture ? ffmpegCapture->getProperty(propId) : 0;
    }
    virtual bool setProperty(int propId, double value)
    {
        return ffmpegCapture ? ffmpegCapture->setProperty(propId, value) : false;
    }
    virtual bool grabFrame()
    {
        return ffmpegCapture ? ffmpegCapture->grabFrame() : false;
    }
    virtual IplImage* retrieveFrame()
    {
        return ffmpegCapture ? ffmpegCapture->retrieveFrame() : 0;
    }
    virtual bool open( const char* filename )
    {
        close();

        icvInitFFMPEG();
        if( !icvCreateFileCapture_FFMPEG_p )
            return false;
        ffmpegCapture = icvCreateFileCapture_FFMPEG_p( filename );
        return ffmpegCapture != 0;
    }
    virtual void close()
    {
        if( ffmpegCapture && icvReleaseCapture_FFMPEG_p )
            icvReleaseCapture_FFMPEG_p( &ffmpegCapture );
        assert( ffmpegCapture == 0 );
        ffmpegCapture = 0;
    }

protected:
    CvCapture* ffmpegCapture;
};


CvCapture* cvCreateFileCapture_Win32(const char * filename)
{
    CvCapture_FFMPEG_proxy* result = new CvCapture_FFMPEG_proxy;
    if( result->open( filename ))
        return result;
    delete result;
    return cvCreateFileCapture_VFW(filename);
}


class CvVideoWriter_FFMPEG_proxy : public CvVideoWriter
{
public:
    CvVideoWriter_FFMPEG_proxy() { ffmpegWriter = 0; }
    virtual ~CvVideoWriter_FFMPEG_proxy() { close(); }

    virtual bool writeFrame( const IplImage* image )
    {
        return ffmpegWriter ? ffmpegWriter->writeFrame(image) : false;
    }
    virtual bool open( const char* filename, int fourcc, double fps, CvSize frameSize, bool isColor )
    {
        close();
        icvInitFFMPEG();
        if( !icvCreateVideoWriter_FFMPEG_p )
            return false;
        ffmpegWriter = icvCreateVideoWriter_FFMPEG_p( filename, fourcc, fps, frameSize, isColor );
        return ffmpegWriter != 0;
    }

    virtual void close()
    {
        if( ffmpegWriter && icvReleaseVideoWriter_FFMPEG_p )
            icvReleaseVideoWriter_FFMPEG_p( &ffmpegWriter );
        assert( ffmpegWriter == 0 );
        ffmpegWriter = 0;
    }

protected:
    CvVideoWriter* ffmpegWriter;
};


CvVideoWriter* cvCreateVideoWriter_Win32( const char* filename, int fourcc,
                                          double fps, CvSize frameSize, int isColor )
{
    CvVideoWriter_FFMPEG_proxy* result = new CvVideoWriter_FFMPEG_proxy;

    if( result->open( filename, fourcc, fps, frameSize, isColor != 0 ))
        return result;
    delete result;
    
    return cvCreateVideoWriter_VFW(filename, fourcc, fps, frameSize, isColor);
}
