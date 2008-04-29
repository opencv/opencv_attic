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

extern "C" {
#include <ffmpeg/avformat.h>
#include <ffmpeg/avcodec.h>
#ifndef WIN32
#include <errno.h>
#endif
}


#ifdef NDEBUG
#define CV_WARN(message)
#else
#define CV_WARN(message) fprintf(stderr, "warning: %s (%s:%d)\n", message, __FILE__, __LINE__)
#endif


#ifndef MKTAG
#define MKTAG(a,b,c,d) (a | (b << 8) | (c << 16) | (d << 24))
#endif




char * FOURCC2str( int fourcc )
{
    char * mystr=(char*)malloc(5);
    mystr[0]=(fourcc    )&255;
    mystr[1]=(fourcc>> 8)&255;
    mystr[2]=(fourcc>>16)&255;
    mystr[3]=(fourcc>>24)&255;
    mystr[4]=0;
    return mystr;
}


// required to look up the correct codec ID depending on the FOURCC code,
// this is just a snipped from the file riff.c from ffmpeg/libavformat
typedef struct AVCodecTag {
    int id;
    unsigned int tag;
} AVCodecTag;

const AVCodecTag codec_bmp_tags[] = {
    { CODEC_ID_H264, MKTAG('H', '2', '6', '4') },
    { CODEC_ID_H264, MKTAG('h', '2', '6', '4') },
    { CODEC_ID_H264, MKTAG('X', '2', '6', '4') },
    { CODEC_ID_H264, MKTAG('x', '2', '6', '4') },
    { CODEC_ID_H264, MKTAG('a', 'v', 'c', '1') },
    { CODEC_ID_H264, MKTAG('V', 'S', 'S', 'H') },

    { CODEC_ID_H263, MKTAG('H', '2', '6', '3') },
    { CODEC_ID_H263P, MKTAG('H', '2', '6', '3') },
    { CODEC_ID_H263I, MKTAG('I', '2', '6', '3') }, /* intel h263 */
    { CODEC_ID_H261, MKTAG('H', '2', '6', '1') },

    /* added based on MPlayer */
    { CODEC_ID_H263P, MKTAG('U', '2', '6', '3') },
    { CODEC_ID_H263P, MKTAG('v', 'i', 'v', '1') },

    { CODEC_ID_MPEG4, MKTAG('F', 'M', 'P', '4') },
    { CODEC_ID_MPEG4, MKTAG('D', 'I', 'V', 'X') },
    { CODEC_ID_MPEG4, MKTAG('D', 'X', '5', '0') },
    { CODEC_ID_MPEG4, MKTAG('X', 'V', 'I', 'D') },
    { CODEC_ID_MPEG4, MKTAG('M', 'P', '4', 'S') },
    { CODEC_ID_MPEG4, MKTAG('M', '4', 'S', '2') },
    { CODEC_ID_MPEG4, MKTAG(0x04, 0, 0, 0) }, /* some broken avi use this */

    /* added based on MPlayer */
    { CODEC_ID_MPEG4, MKTAG('D', 'I', 'V', '1') },
    { CODEC_ID_MPEG4, MKTAG('B', 'L', 'Z', '0') },
    { CODEC_ID_MPEG4, MKTAG('m', 'p', '4', 'v') },
    { CODEC_ID_MPEG4, MKTAG('U', 'M', 'P', '4') },
    { CODEC_ID_MPEG4, MKTAG('W', 'V', '1', 'F') },
    { CODEC_ID_MPEG4, MKTAG('S', 'E', 'D', 'G') },

    { CODEC_ID_MPEG4, MKTAG('R', 'M', 'P', '4') },

    { CODEC_ID_MSMPEG4V3, MKTAG('D', 'I', 'V', '3') }, /* default signature when using MSMPEG4 */
    { CODEC_ID_MSMPEG4V3, MKTAG('M', 'P', '4', '3') },

    /* added based on MPlayer */
    { CODEC_ID_MSMPEG4V3, MKTAG('M', 'P', 'G', '3') },
    { CODEC_ID_MSMPEG4V3, MKTAG('D', 'I', 'V', '5') },
    { CODEC_ID_MSMPEG4V3, MKTAG('D', 'I', 'V', '6') },
    { CODEC_ID_MSMPEG4V3, MKTAG('D', 'I', 'V', '4') },
    { CODEC_ID_MSMPEG4V3, MKTAG('A', 'P', '4', '1') },
    { CODEC_ID_MSMPEG4V3, MKTAG('C', 'O', 'L', '1') },
    { CODEC_ID_MSMPEG4V3, MKTAG('C', 'O', 'L', '0') },

    { CODEC_ID_MSMPEG4V2, MKTAG('M', 'P', '4', '2') },

    /* added based on MPlayer */
    { CODEC_ID_MSMPEG4V2, MKTAG('D', 'I', 'V', '2') },

    { CODEC_ID_MSMPEG4V1, MKTAG('M', 'P', 'G', '4') },

    { CODEC_ID_WMV1, MKTAG('W', 'M', 'V', '1') },

    /* added based on MPlayer */
    { CODEC_ID_WMV2, MKTAG('W', 'M', 'V', '2') },
    { CODEC_ID_DVVIDEO, MKTAG('d', 'v', 's', 'd') },
    { CODEC_ID_DVVIDEO, MKTAG('d', 'v', 'h', 'd') },
    { CODEC_ID_DVVIDEO, MKTAG('d', 'v', 's', 'l') },
    { CODEC_ID_DVVIDEO, MKTAG('d', 'v', '2', '5') },
    { CODEC_ID_MPEG1VIDEO, MKTAG('m', 'p', 'g', '1') },
    { CODEC_ID_MPEG1VIDEO, MKTAG('m', 'p', 'g', '2') },
    { CODEC_ID_MPEG2VIDEO, MKTAG('m', 'p', 'g', '2') },
    { CODEC_ID_MPEG2VIDEO, MKTAG('M', 'P', 'E', 'G') },
    { CODEC_ID_MPEG1VIDEO, MKTAG('P', 'I', 'M', '1') },
    { CODEC_ID_MPEG1VIDEO, MKTAG('V', 'C', 'R', '2') },
    { CODEC_ID_MPEG1VIDEO, 0x10000001 },
    { CODEC_ID_MPEG2VIDEO, 0x10000002 },
    { CODEC_ID_MPEG2VIDEO, MKTAG('D', 'V', 'R', ' ') },
    { CODEC_ID_MPEG2VIDEO, MKTAG('M', 'M', 'E', 'S') },
    { CODEC_ID_MJPEG, MKTAG('M', 'J', 'P', 'G') },
    { CODEC_ID_MJPEG, MKTAG('L', 'J', 'P', 'G') },
    { CODEC_ID_LJPEG, MKTAG('L', 'J', 'P', 'G') },
    { CODEC_ID_MJPEG, MKTAG('J', 'P', 'G', 'L') }, /* Pegasus lossless JPEG */
    { CODEC_ID_MJPEG, MKTAG('M', 'J', 'L', 'S') }, /* JPEG-LS custom FOURCC for avi - decoder */
    { CODEC_ID_MJPEG, MKTAG('j', 'p', 'e', 'g') },
    { CODEC_ID_MJPEG, MKTAG('I', 'J', 'P', 'G') },
    { CODEC_ID_MJPEG, MKTAG('A', 'V', 'R', 'n') },
    { CODEC_ID_HUFFYUV, MKTAG('H', 'F', 'Y', 'U') },
    { CODEC_ID_FFVHUFF, MKTAG('F', 'F', 'V', 'H') },
    { CODEC_ID_CYUV, MKTAG('C', 'Y', 'U', 'V') },
    { CODEC_ID_RAWVIDEO, 0 },
    { CODEC_ID_RAWVIDEO, MKTAG('I', '4', '2', '0') },
    { CODEC_ID_RAWVIDEO, MKTAG('Y', 'U', 'Y', '2') },
    { CODEC_ID_RAWVIDEO, MKTAG('Y', '4', '2', '2') },
    { CODEC_ID_RAWVIDEO, MKTAG('Y', 'V', '1', '2') },
    { CODEC_ID_RAWVIDEO, MKTAG('U', 'Y', 'V', 'Y') },
    { CODEC_ID_RAWVIDEO, MKTAG('I', 'Y', 'U', 'V') },
    { CODEC_ID_RAWVIDEO, MKTAG('Y', '8', '0', '0') },
    { CODEC_ID_RAWVIDEO, MKTAG('H', 'D', 'Y', 'C') },
    { CODEC_ID_INDEO3, MKTAG('I', 'V', '3', '1') },
    { CODEC_ID_INDEO3, MKTAG('I', 'V', '3', '2') },
    { CODEC_ID_VP3, MKTAG('V', 'P', '3', '1') },
    { CODEC_ID_VP3, MKTAG('V', 'P', '3', '0') },
    { CODEC_ID_ASV1, MKTAG('A', 'S', 'V', '1') },
    { CODEC_ID_ASV2, MKTAG('A', 'S', 'V', '2') },
    { CODEC_ID_VCR1, MKTAG('V', 'C', 'R', '1') },
    { CODEC_ID_FFV1, MKTAG('F', 'F', 'V', '1') },
    { CODEC_ID_XAN_WC4, MKTAG('X', 'x', 'a', 'n') },
    { CODEC_ID_MSRLE, MKTAG('m', 'r', 'l', 'e') },
    { CODEC_ID_MSRLE, MKTAG(0x1, 0x0, 0x0, 0x0) },
    { CODEC_ID_MSVIDEO1, MKTAG('M', 'S', 'V', 'C') },
    { CODEC_ID_MSVIDEO1, MKTAG('m', 's', 'v', 'c') },
    { CODEC_ID_MSVIDEO1, MKTAG('C', 'R', 'A', 'M') },
    { CODEC_ID_MSVIDEO1, MKTAG('c', 'r', 'a', 'm') },
    { CODEC_ID_MSVIDEO1, MKTAG('W', 'H', 'A', 'M') },
    { CODEC_ID_MSVIDEO1, MKTAG('w', 'h', 'a', 'm') },
    { CODEC_ID_CINEPAK, MKTAG('c', 'v', 'i', 'd') },
    { CODEC_ID_TRUEMOTION1, MKTAG('D', 'U', 'C', 'K') },
    { CODEC_ID_MSZH, MKTAG('M', 'S', 'Z', 'H') },
    { CODEC_ID_ZLIB, MKTAG('Z', 'L', 'I', 'B') },
    { CODEC_ID_SNOW, MKTAG('S', 'N', 'O', 'W') },
    { CODEC_ID_4XM, MKTAG('4', 'X', 'M', 'V') },
    { CODEC_ID_FLV1, MKTAG('F', 'L', 'V', '1') },
    { CODEC_ID_SVQ1, MKTAG('s', 'v', 'q', '1') },
    { CODEC_ID_TSCC, MKTAG('t', 's', 'c', 'c') },
    { CODEC_ID_ULTI, MKTAG('U', 'L', 'T', 'I') },
    { CODEC_ID_VIXL, MKTAG('V', 'I', 'X', 'L') },
    { CODEC_ID_QPEG, MKTAG('Q', 'P', 'E', 'G') },
    { CODEC_ID_QPEG, MKTAG('Q', '1', '.', '0') },
    { CODEC_ID_QPEG, MKTAG('Q', '1', '.', '1') },
    { CODEC_ID_WMV3, MKTAG('W', 'M', 'V', '3') },
    { CODEC_ID_LOCO, MKTAG('L', 'O', 'C', 'O') },
    { CODEC_ID_THEORA, MKTAG('t', 'h', 'e', 'o') },
#if LIBAVCODEC_VERSION_INT>0x000409
    { CODEC_ID_WNV1, MKTAG('W', 'N', 'V', '1') },
    { CODEC_ID_AASC, MKTAG('A', 'A', 'S', 'C') },
    { CODEC_ID_INDEO2, MKTAG('R', 'T', '2', '1') },
    { CODEC_ID_FRAPS, MKTAG('F', 'P', 'S', '1') },
    { CODEC_ID_TRUEMOTION2, MKTAG('T', 'M', '2', '0') },
#endif
#if LIBAVCODEC_VERSION_INT>((50<<16)+(1<<8)+0)
    { CODEC_ID_FLASHSV, MKTAG('F', 'S', 'V', '1') },
    { CODEC_ID_JPEGLS,MKTAG('M', 'J', 'L', 'S') }, /* JPEG-LS custom FOURCC for avi - encoder */
    { CODEC_ID_VC1, MKTAG('W', 'V', 'C', '1') },
    { CODEC_ID_VC1, MKTAG('W', 'M', 'V', 'A') },
    { CODEC_ID_CSCD, MKTAG('C', 'S', 'C', 'D') },
    { CODEC_ID_ZMBV, MKTAG('Z', 'M', 'B', 'V') },
    { CODEC_ID_KMVC, MKTAG('K', 'M', 'V', 'C') },
#endif
#if LIBAVCODEC_VERSION_INT>((51<<16)+(11<<8)+0)
    { CODEC_ID_VP5, MKTAG('V', 'P', '5', '0') },
    { CODEC_ID_VP6, MKTAG('V', 'P', '6', '0') },
    { CODEC_ID_VP6, MKTAG('V', 'P', '6', '1') },
    { CODEC_ID_VP6, MKTAG('V', 'P', '6', '2') },
    { CODEC_ID_VP6F, MKTAG('V', 'P', '6', 'F') },
    { CODEC_ID_JPEG2000, MKTAG('M', 'J', '2', 'C') },
    { CODEC_ID_VMNC, MKTAG('V', 'M', 'n', 'c') },
#endif
#if LIBAVCODEC_VERSION_INT>=((51<<16)+(49<<8)+0)
// this tag seems not to exist in older versions of FFMPEG
    { CODEC_ID_TARGA, MKTAG('t', 'g', 'a', ' ') },
#endif
    { CODEC_ID_NONE, 0 },
};


typedef struct CvCaptureAVI_FFMPEG
{
    CvCaptureVTable   * vtable;

    AVFormatContext   * ic;
    int                 video_stream;
    AVStream          * video_st;
    AVFrame           * picture;
    int64_t             picture_pts;
    AVFrame             rgb_picture;
    IplImage            frame;
/*
   'filename' contains the filename of the videosource,
   'filename==NULL' indicates that ffmpeg's seek support works
   for the particular file.
   'filename!=NULL' indicates that the slow fallback function is used for seeking,
   and so the filename is needed to reopen the file on backward seeking.
*/
    char              * filename;
} CvCaptureAVI_FFMPEG;


static void icvCloseAVI_FFMPEG( CvCaptureAVI_FFMPEG* capture )
{
    //cvFree( (void**)&(capture->entries) );
    if( capture->picture )
    av_free(capture->picture);

    if( capture->video_st )
    {
#if LIBAVFORMAT_BUILD > 4628
        avcodec_close( capture->video_st->codec );
#else
        avcodec_close( &capture->video_st->codec );
#endif
        capture->video_st = NULL;
    }

    if( capture->ic )
    {
        av_close_input_file(capture->ic);
        capture->ic = NULL;
    }

    if( capture->rgb_picture.data[0] )
        cvFree( &capture->rgb_picture.data[0] );

    memset( &capture->frame, 0, sizeof(capture->frame));
}


/*
    Used to reopen a video if the slower fallback function for seeking is used.
*/
static int icvReopenFileAVI_FFMPEG( CvCaptureAVI_FFMPEG* capture )
{
    if ( capture->filename==NULL ) return 0;

#if LIBAVFORMAT_BUILD > 4628
    avcodec_close( capture->video_st->codec );
#else
    avcodec_close( &capture->video_st->codec );
#endif
    av_close_input_file(capture->ic);

    // reopen video
    av_open_input_file(&capture->ic, capture->filename, NULL, 0, NULL);
    av_find_stream_info(capture->ic);
#if LIBAVFORMAT_BUILD > 4628
    AVCodecContext *enc = capture->ic->streams[capture->video_stream]->codec;
#else
    AVCodecContext *enc = &capture->ic->streams[capture->video_stream]->codec;
#endif
    AVCodec *codec = avcodec_find_decoder(enc->codec_id);
    avcodec_open(enc, codec);
    capture->video_st = capture->ic->streams[capture->video_stream];

    // reset framenumber to zero
    capture->picture_pts=0;

    return 1;
}



// forward
static int icvCheckSeekAVI_FFMPEG( CvCaptureAVI_FFMPEG *capture);

static int icvOpenAVI_FFMPEG( CvCaptureAVI_FFMPEG* capture, const char* filename )
{
    int err, valid = 0, video_index = -1, i;
    AVFormatContext *ic;

    capture->ic = NULL;
    capture->video_stream = -1;
    capture->video_st = NULL;
    /* register all codecs, demux and protocols */
    av_register_all();

#ifndef _DEBUG
    // av_log_level = AV_LOG_QUIET;
#endif

    err = av_open_input_file(&ic, filename, NULL, 0, NULL);
    if (err < 0) {
	    CV_WARN("Error opening file");
	    goto exit_func;
    }
    capture->ic = ic;
    err = av_find_stream_info(ic);
    if (err < 0) {
	    CV_WARN("Could not find codec parameters");
	    goto exit_func;
    }
    for(i = 0; i < ic->nb_streams; i++) {
#if LIBAVFORMAT_BUILD > 4628
        AVCodecContext *enc = ic->streams[i]->codec;
#else
        AVCodecContext *enc = &ic->streams[i]->codec;
#endif
        AVCodec *codec;
    if( CODEC_TYPE_VIDEO == enc->codec_type && video_index < 0) {
        video_index = i;
        codec = avcodec_find_decoder(enc->codec_id);
        if (!codec ||
        avcodec_open(enc, codec) < 0)
        goto exit_func;
        capture->video_stream = i;
        capture->video_st = ic->streams[i];
        capture->picture = avcodec_alloc_frame();

        capture->rgb_picture.data[0] = (uint8_t*)cvAlloc(
                                avpicture_get_size( PIX_FMT_BGR24,
                                enc->width, enc->height ));
        avpicture_fill( (AVPicture*)&capture->rgb_picture, capture->rgb_picture.data[0],
                PIX_FMT_BGR24, enc->width, enc->height );

        cvInitImageHeader( &capture->frame, cvSize( enc->width,
                                   enc->height ), 8, 3, 0, 4 );
        cvSetData( &capture->frame, capture->rgb_picture.data[0],
                           capture->rgb_picture.linesize[0] );
        break;
    }
    }

    if(video_index >= 0) valid = 1;

    // perform check if source is seekable via ffmpeg's seek function av_seek_frame(...)
    err = av_seek_frame(capture->ic, capture->video_stream, 10, 0);
    if (err < 0)
    {
        int length=0;
        while ( filename[length] ) length++;
        // remark filename
        capture->filename=(char*)malloc(length+1);
        for ( int i=0; i<length+1; i++ ) capture->filename[i]=filename[i];
        // reopen videofile to 'seek' back to first frame
        icvReopenFileAVI_FFMPEG( capture );
    }
    else
    {
        // seek seems to work, so we don't need the filename,
        // but we still need to seek back to filestart
        capture->filename=NULL;
        av_seek_frame(capture->ic, capture->video_stream, 0, 0);
    }
exit_func:

    if( !valid )
        icvCloseAVI_FFMPEG( capture );

    return valid;
}




static int icvGrabFrameAVI_FFMPEG( CvCaptureAVI_FFMPEG* capture )
{
    int valid=0;
    static bool bFirstTime = true;
    static AVPacket pkt;
    int got_picture;

    // First time we're called, set packet.data to NULL to indicate it
    // doesn't have to be freed
    if (bFirstTime) {
        bFirstTime = false;
        pkt.data = NULL;
    }

    if( !capture || !capture->ic || !capture->video_st )
        return 0;

    // free last packet if exist
    if (pkt.data != NULL) {
        av_free_packet (&pkt);
    }

    // get the next frame
    while ((0 == valid) && (av_read_frame(capture->ic, &pkt) >= 0)) {
		if( pkt.stream_index != capture->video_stream ) continue;
#if LIBAVFORMAT_BUILD > 4628
        avcodec_decode_video(capture->video_st->codec,
                             capture->picture, &got_picture,
                             pkt.data, pkt.size);
#else
        avcodec_decode_video(&capture->video_st->codec,
                             capture->picture, &got_picture,
                             pkt.data, pkt.size);
#endif

        if (got_picture) {
            // we have a new picture, so memorize it
            capture->picture_pts = pkt.pts;
            valid = 1;
        }
    }

    // return if we have a new picture or not
    return valid;
}


static const IplImage* icvRetrieveFrameAVI_FFMPEG( CvCaptureAVI_FFMPEG* capture )
{
    if( !capture || !capture->video_st || !capture->picture->data[0] )
    return 0;


#if LIBAVFORMAT_BUILD > 4628
    img_convert( (AVPicture*)&capture->rgb_picture, PIX_FMT_BGR24,
                 (AVPicture*)capture->picture,
                 capture->video_st->codec->pix_fmt,
                 capture->video_st->codec->width,
                 capture->video_st->codec->height );
#else
    img_convert( (AVPicture*)&capture->rgb_picture, PIX_FMT_BGR24,
                 (AVPicture*)capture->picture,
                 capture->video_st->codec.pix_fmt,
                 capture->video_st->codec.width,
                 capture->video_st->codec.height );
#endif
    return &capture->frame;
}


static double icvGetPropertyAVI_FFMPEG( CvCaptureAVI_FFMPEG* capture, int property_id )
{
    // if( !capture || !capture->video_st || !capture->picture->data[0] ) return 0;
    if( !capture || !capture->video_st ) return 0;


    int64_t timestamp;
    timestamp = capture->picture_pts;

    switch( property_id )
    {
    case CV_CAP_PROP_POS_MSEC:
        // if(capture->ic->start_time != static_cast<double>(AV_NOPTS_VALUE))
        if(capture->ic->start_time != AV_NOPTS_VALUE)
        return (double)(timestamp - capture->ic->start_time)*1000/(double)AV_TIME_BASE;
        break;
    case CV_CAP_PROP_POS_FRAMES:
    //if(capture->video_st->cur_dts != static_cast<double>(AV_NOPTS_VALUE))
    if(capture->video_st->cur_dts != AV_NOPTS_VALUE)
        return (double)capture->video_st->cur_dts-1;
    break;
    case CV_CAP_PROP_POS_AVI_RATIO:
    //  if(capture->ic->start_time != static_cast<double>(AV_NOPTS_VALUE) && capture->ic->duration != static_cast<double>(AV_NOPTS_VALUE))
    if(capture->ic->start_time != AV_NOPTS_VALUE && capture->ic->duration != AV_NOPTS_VALUE)
        return (double)(timestamp-capture->ic->start_time)/(double)capture->ic->duration;
    break;
    case CV_CAP_PROP_FRAME_WIDTH:
        return (double)capture->frame.width;
    break;
    case CV_CAP_PROP_FRAME_HEIGHT:
        return (double)capture->frame.height;
    break;
    case CV_CAP_PROP_FPS:
#if LIBAVCODEC_BUILD > 4753
        return av_q2d (capture->video_st->r_frame_rate);
#else
        return (double)capture->video_st->codec.frame_rate
            / (double)capture->video_st->codec.frame_rate_base;
#endif
    break;
    case CV_CAP_PROP_FOURCC:
#if LIBAVFORMAT_BUILD > 4628
        return (double)capture->video_st->codec->codec_tag;
#else
        return (double)capture->video_st->codec.codec_tag;
#endif
    break;
    }
    return 0;
}



// this is a VERY slow fallback function, ONLY used if ffmpeg's av_seek_frame delivers no correct result!
static int icvSlowSeekAVI_FFMPEG( CvCaptureAVI_FFMPEG* capture, const int framenumber )
{
    if ( framenumber>capture->picture_pts )
    {
        while ( capture->picture_pts<framenumber )
            if ( icvGrabFrameAVI_FFMPEG( capture )<0 ) return 0;
    }
    else if ( framenumber<capture->picture_pts )
    {
        icvReopenFileAVI_FFMPEG(capture);
        while ( capture->picture_pts<framenumber )
            if ( icvGrabFrameAVI_FFMPEG( capture )<0 ) return 0;
    }
    return 1;
}




static int icvSetPropertyAVI_FFMPEG( CvCaptureAVI_FFMPEG* capture,
                                     int property_id, double value )
{
    if( !capture || !capture->video_st ) return 0;

    switch( property_id )
    {
    case CV_CAP_PROP_POS_MSEC:
    case CV_CAP_PROP_POS_FRAMES:
    case CV_CAP_PROP_POS_AVI_RATIO:
        {
            int64_t timestamp;
            AVRational time_base;
        switch( property_id )
            {
        case CV_CAP_PROP_POS_FRAMES:
                timestamp=(int64_t)value;
                if(capture->ic->start_time != AV_NOPTS_VALUE)
                    timestamp += capture->ic->start_time;
        break;

        case CV_CAP_PROP_POS_MSEC:
                time_base=capture->ic->streams[capture->video_stream]->time_base;
                timestamp=(int64_t)(value*(float(time_base.den)/float(time_base.num))/1000);
        if(capture->ic->start_time != AV_NOPTS_VALUE)
                    timestamp += capture->ic->start_time;
        break;

        case CV_CAP_PROP_POS_AVI_RATIO:
                timestamp=(int64_t)(value*capture->ic->duration);
        if(capture->ic->start_time != AV_NOPTS_VALUE && capture->ic->duration != AV_NOPTS_VALUE)
                    timestamp += capture->ic->start_time;
        break;
        }

            if ( capture->filename )
            {
                // ffmpeg's seek doesn't work...
                if (icvSlowSeekAVI_FFMPEG(capture, timestamp) < 0)
                {
                    fprintf(stderr, "HIGHGUI ERROR: AVI: could not (slow) seek to position %0.3f\n",
                        (double)timestamp / AV_TIME_BASE);
                    return 0;
                }
            }
            else
            {
                int ret = av_seek_frame(capture->ic, capture->video_stream, timestamp, 0);
                if (ret < 0)
                {
            fprintf(stderr, "HIGHGUI ERROR: AVI: could not seek to position %0.3f\n",
                (double)timestamp / AV_TIME_BASE);
            return 0;
        }
        }
            capture->picture_pts=value;
    }
        break;

    default:
        return 0;
    }

    return 1;
}



static CvCaptureVTable captureAVI_FFMPEG_vtable =
{
    6,
    (CvCaptureCloseFunc)icvCloseAVI_FFMPEG,
    (CvCaptureGrabFrameFunc)icvGrabFrameAVI_FFMPEG,
    (CvCaptureRetrieveFrameFunc)icvRetrieveFrameAVI_FFMPEG,
    (CvCaptureGetPropertyFunc)icvGetPropertyAVI_FFMPEG,
    (CvCaptureSetPropertyFunc)icvSetPropertyAVI_FFMPEG,
    (CvCaptureGetDescriptionFunc)0
};


CvCapture* cvCaptureFromFile_FFMPEG( const char* filename )
{
    CvCaptureAVI_FFMPEG* capture = 0;

    if( filename )
    {
        capture = (CvCaptureAVI_FFMPEG*)cvAlloc( sizeof(*capture));
        memset( capture, 0, sizeof(*capture));

        capture->vtable = &captureAVI_FFMPEG_vtable;

        if( !icvOpenAVI_FFMPEG( capture, filename ))
        {
            capture->vtable->close((CvCapture*)capture);
            cvFree( &capture );
        }
    }

    return (CvCapture*)capture;
}


///////////////// FFMPEG CvVideoWriter implementation //////////////////////////
typedef struct CvFFMPEGWriter
{
	CvVideoWriterVTable *vtable;

	AVOutputFormat *fmt;
	AVFormatContext *oc;
    uint8_t         * outbuf;
    uint32_t          outbuf_size;
    FILE            * outfile;
    AVFrame         * picture;
    AVFrame         * input_picture;
    uint8_t         * picbuf;
	AVStream        * video_st;
	int 			  input_pix_fmt;
    IplImage        * temp_image;
} CvFFMPEGWriter;

static const char * icvFFMPEGErrStr(int err)
{
    switch(err) {
    case AVERROR_NUMEXPECTED:
		return "Incorrect filename syntax";
    case AVERROR_INVALIDDATA:
		return "Invalid data in header";
    case AVERROR_NOFMT:
		return "Unknown format";
    case AVERROR_IO:
		return "I/O error occurred";
    case AVERROR_NOMEM:
		return "Memory allocation error";
    default:
		break;
    }
  	return "Unspecified error";
}

/* function internal to FFMPEG (libavformat/riff.c) to lookup codec id by fourcc tag*/
extern "C" {
	enum CodecID codec_get_bmp_id(unsigned int tag);
}

/**
 * the following function is a modified version of code
 * found in ffmpeg-0.4.9-pre1/output_example.c
 */
static AVFrame * icv_alloc_picture_FFMPEG(int pix_fmt, int width, int height, bool alloc)
{
	AVFrame * picture;
	uint8_t * picture_buf;
	int size;

	picture = avcodec_alloc_frame();
	if (!picture)
		return NULL;
	size = avpicture_get_size(pix_fmt, width, height);
	if(alloc){
		picture_buf = (uint8_t *) cvAlloc(size);
		if (!picture_buf)
		{
			av_free(picture);
			return NULL;
		}
		avpicture_fill((AVPicture *)picture, picture_buf,
				pix_fmt, width, height);
	}
	else {
	}
	return picture;
}

/* add a video output stream to the container */
static AVStream *icv_add_video_stream_FFMPEG(AVFormatContext *oc,
		                                     CodecID codec_id,
											 int w, int h, int bitrate,
											 double fps, int pixel_format)
{
	AVCodecContext *c;
	AVStream *st;
	int frame_rate, frame_rate_base;
	AVCodec *codec;


	st = av_new_stream(oc, 0);
	if (!st) {
		CV_WARN("Could not allocate stream");
		return NULL;
	}

#if LIBAVFORMAT_BUILD > 4628
	c = st->codec;
#else
	c = &(st->codec);
#endif

#if LIBAVFORMAT_BUILD > 4621
	c->codec_id = av_guess_codec(oc->oformat, NULL, oc->filename, NULL, CODEC_TYPE_VIDEO);
#else
	c->codec_id = oc->oformat->video_codec;
#endif

	if(codec_id != CODEC_ID_NONE){
		c->codec_id = codec_id;
	}

    //if(codec_tag) c->codec_tag=codec_tag;
	codec = avcodec_find_encoder(c->codec_id);

	c->codec_type = CODEC_TYPE_VIDEO;

	/* put sample parameters */
	c->bit_rate = bitrate;

	/* resolution must be a multiple of two */
	c->width = w;
	c->height = h;

	/* time base: this is the fundamental unit of time (in seconds) in terms
       of which frame timestamps are represented. for fixed-fps content,
       timebase should be 1/framerate and timestamp increments should be
       identically 1. */
	frame_rate=cvRound(fps);
	frame_rate_base=1;
	while (fabs((double)frame_rate/frame_rate_base) - fps > 0.001){
		frame_rate_base*=10;
		frame_rate=cvRound(fps*frame_rate_base);
	}
#if LIBAVFORMAT_BUILD > 4752
    c->time_base.den = frame_rate;
    c->time_base.num = frame_rate_base;
	/* adjust time base for supported framerates */
	if(codec && codec->supported_framerates){
		const AVRational *p= codec->supported_framerates;
        AVRational req = {frame_rate, frame_rate_base};
		const AVRational *best=NULL;
		AVRational best_error= {INT_MAX, 1};
		for(; p->den!=0; p++){
			AVRational error= av_sub_q(req, *p);
			if(error.num <0) error.num *= -1;
			if(av_cmp_q(error, best_error) < 0){
				best_error= error;
				best= p;
			}
		}
		c->time_base.den= best->num;
		c->time_base.num= best->den;
	}
#else
	c->frame_rate = frame_rate;
	c->frame_rate_base = frame_rate_base;
#endif

	c->gop_size = 12; /* emit one intra frame every twelve frames at most */
	c->pix_fmt = (PixelFormat) pixel_format;

	if (c->codec_id == CODEC_ID_MPEG2VIDEO) {
        c->max_b_frames = 2;
    }
    if (c->codec_id == CODEC_ID_MPEG1VIDEO || c->codec_id == CODEC_ID_MSMPEG4V3){
        /* needed to avoid using macroblocks in which some coeffs overflow
           this doesnt happen with normal video, it just happens here as the
           motion of the chroma plane doesnt match the luma plane */
		/* avoid FFMPEG warning 'clipping 1 dct coefficients...' */
        c->mb_decision=2;
    }
#if LIBAVCODEC_VERSION_INT>0x000409
    // some formats want stream headers to be seperate
    if(oc->oformat->flags & AVFMT_GLOBALHEADER)
    {
        c->flags |= CODEC_FLAG_GLOBAL_HEADER;
    }
#endif

    return st;
}

int icv_av_write_frame_FFMPEG( AVFormatContext * oc, AVStream * video_st, uint8_t * outbuf, uint32_t outbuf_size, AVFrame * picture ){
	CV_FUNCNAME("icv_av_write_frame_FFMPEG");

#if LIBAVFORMAT_BUILD > 4628
	AVCodecContext * c = video_st->codec;
#else
	AVCodecContext * c = &(video_st->codec);
#endif
	int out_size;
	int ret;

	__BEGIN__;

    if (oc->oformat->flags & AVFMT_RAWPICTURE) {
        /* raw video case. The API will change slightly in the near
           futur for that */
        AVPacket pkt;
        av_init_packet(&pkt);

        pkt.flags |= PKT_FLAG_KEY;
        pkt.stream_index= video_st->index;
        pkt.data= (uint8_t *)picture;
        pkt.size= sizeof(AVPicture);

        ret = av_write_frame(oc, &pkt);
    } else {
        /* encode the image */
        out_size = avcodec_encode_video(c, outbuf, outbuf_size, picture);
        /* if zero size, it means the image was buffered */
        if (out_size > 0) {
            AVPacket pkt;
            av_init_packet(&pkt);

#if LIBAVFORMAT_BUILD > 4752
            pkt.pts = av_rescale_q(c->coded_frame->pts, c->time_base, video_st->time_base);
#else
			pkt.pts = c->coded_frame->pts;
#endif
            if(c->coded_frame->key_frame)
                pkt.flags |= PKT_FLAG_KEY;
            pkt.stream_index= video_st->index;
            pkt.data= outbuf;
            pkt.size= out_size;

            /* write the compressed frame in the media file */
            ret = av_write_frame(oc, &pkt);
        } else {
            ret = 0;
        }
    }
    if (ret != 0) {
		CV_ERROR(CV_StsError, "Error while writing video frame");
	}

	__END__;
	return CV_StsOk;
}

/// write a frame with FFMPEG
static int icvWriteFrame_FFMPEG( CvVideoWriter * writer, const IplImage * image )
{
	int ret = 0;

	CV_FUNCNAME("cvWriteFrame");

	__BEGIN__;

	// typecast from opaque data type to implemented struct
	CvFFMPEGWriter * mywriter = (CvFFMPEGWriter*) writer;
#if LIBAVFORMAT_BUILD > 4628
    AVCodecContext *c = mywriter->video_st->codec;
#else
	AVCodecContext *c = &(mywriter->video_st->codec);
#endif

    if( c->codec_id == CODEC_ID_RAWVIDEO && image->origin != IPL_ORIGIN_BL )
    {
        if( !mywriter->temp_image )
            mywriter->temp_image = cvCreateImage( cvGetSize(image),
                                    image->depth, image->nChannels );
        cvFlip( image, mywriter->temp_image, 0 );
        image = mywriter->temp_image;
    }

    // check parameters
    if (mywriter->input_pix_fmt == PIX_FMT_BGR24) {
        if (image->nChannels != 3 || image->depth != IPL_DEPTH_8U) {
            CV_ERROR(CV_StsUnsupportedFormat, "cvWriteFrame() needs images with depth = IPL_DEPTH_8U and nChannels = 3.");
        }
    }
	else if (mywriter->input_pix_fmt == PIX_FMT_GRAY8) {
        if (image->nChannels != 1 || image->depth != IPL_DEPTH_8U) {
            CV_ERROR(CV_StsUnsupportedFormat, "cvWriteFrame() needs images with depth = IPL_DEPTH_8U and nChannels = 1.");
        }
    }
	else {
        assert(false);
    }

	// check if buffer sizes match, i.e. image has expected format (size, channels, bitdepth, alignment)
	assert (image->imageSize == avpicture_get_size( mywriter->input_pix_fmt, image->width, image->height ));

	if ( c->pix_fmt != mywriter->input_pix_fmt ) {
		assert( mywriter->input_picture );
		// let input_picture point to the raw data buffer of 'image'
		avpicture_fill((AVPicture *)mywriter->input_picture, (uint8_t *) image->imageData,
				mywriter->input_pix_fmt, image->width, image->height);

		// convert to the color format needed by the codec
		if( img_convert((AVPicture *)mywriter->picture, c->pix_fmt,
					(AVPicture *)mywriter->input_picture, mywriter->input_pix_fmt,
					image->width, image->height) < 0){
			CV_ERROR(CV_StsUnsupportedFormat, "FFMPEG::img_convert pixel format conversion from BGR24 not handled");
		}
	}
	else{
		avpicture_fill((AVPicture *)mywriter->picture, (uint8_t *) image->imageData,
				mywriter->input_pix_fmt, image->width, image->height);
	}

	ret = icv_av_write_frame_FFMPEG( mywriter->oc, mywriter->video_st, mywriter->outbuf, mywriter->outbuf_size, mywriter->picture);

	__END__;
	return ret;
}

/// close video output stream and free associated memory
static void icvCloseVideoWriter_FFMPEG( CvFFMPEGWriter* mywriter )
{
	int i;

	// nothing to do if already released
	if ( !writer )
		return;

	/* no more frame to compress. The codec has a latency of a few
	   frames if using B frames, so we get the last frames by
	   passing the same picture again */
	// TODO -- do we need to account for latency here?

	/* write the trailer, if any */
	av_write_trailer(mywriter->oc);

	// free pictures
#if LIBAVFORMAT_BUILD > 4628
	if( mywriter->video_st->codec->pix_fmt != mywriter->input_pix_fmt){
#else
	if( mywriter->video_st->codec.pix_fmt != mywriter->input_pix_fmt){
#endif
		cvFree(&(mywriter->picture->data[0]));
	}
	av_free(mywriter->picture);

    if (mywriter->input_picture) {
        av_free(mywriter->input_picture);
    }

	/* close codec */
#if LIBAVFORMAT_BUILD > 4628
	avcodec_close(mywriter->video_st->codec);
#else
	avcodec_close(&(mywriter->video_st->codec));
#endif

	av_free(mywriter->outbuf);

	/* free the streams */
	for(i = 0; i < mywriter->oc->nb_streams; i++) {
		av_freep(&mywriter->oc->streams[i]->codec);
		av_freep(&mywriter->oc->streams[i]);
	}

	if (!(mywriter->fmt->flags & AVFMT_NOFILE)) {
		/* close the output file */


#if LIBAVCODEC_VERSION_INT==((51<<16)+(49<<8)+0)
		url_fclose(mywriter->oc->pb);
#else
		url_fclose(&mywriter->oc->pb);
#endif

	}

	/* free the stream */
	av_free(mywriter->oc);

    cvReleaseImage( &mywriter->temp_image );

	/* free cvVideoWriter */
	cvFree ( writer );

	// mark as released
	(*writer) = 0;
}

static CvVideoWriterVTable videoWriter_FFMPEG_vtable =
{
    2,
    (CvVideoWriterCloseFunc)icvCloseVideoWriter_FFMPEG,
    (CvVideoWriterWriteFrameFunc)icvWriteFrame_FFMPEG
};

/// Create a video writer object that uses FFMPEG
CvVideoWriter* cvCreateVideoWriter_FFMPEG( const char * filename, int fourcc,
		double fps, CvSize frameSize, int is_color )
{
	CV_FUNCNAME("cvCreateVideoWriter");

	CvFFMPEGWriter* writer = NULL;
	CodecID codec_id = CODEC_ID_NONE;
	int err;

	__BEGIN__;

	// check arguments
	assert (filename);
	assert (fps > 0);
	assert (frameSize.width > 0  &&  frameSize.height > 0);

	// allocate memory for structure...
	writer = (CvFFMPEGWriter*)cvAlloc( sizeof(*writer));
	memset (writer, 0, sizeof (*writer));

	writer->vtable = &writerAVI_FFMPEG_vtable;

	// tell FFMPEG to register codecs
	av_register_all ();

	/* auto detect the output format from the name and fourcc code. */
	writer->fmt = guess_format(NULL, filename, NULL);
	if (!writer->fmt) {
		CV_ERROR( CV_StsUnsupportedFormat, "FFMPEG does not recognize the given file extension");
	}

	/* determine optimal pixel format */
    if (is_color) {
        writer->input_pix_fmt = PIX_FMT_BGR24;
    }
	else {
        writer->input_pix_fmt = PIX_FMT_GRAY8;
    }

	// alloc memory for context
	writer->oc = av_alloc_format_context();
	assert (writer->oc);

	/* set file name */
	writer->oc->oformat = writer->fmt;
	snprintf(writer->oc->filename, sizeof(writer->oc->filename), "%s", filename);

	/* set some options */
	writer->oc->max_delay = (int)(0.7*AV_TIME_BASE);  /* This reduces buffer underrun warnings with MPEG */

	/* Lookup codec_id for given fourcc */
	if(fourcc!=CV_FOURCC_DEFAULT){
#if LIBAVCODEC_VERSION_INT<((51<<16)+(49<<8)+0)
        if( (codec_id = codec_get_bmp_id( fourcc )) == CODEC_ID_NONE ){
			CV_ERROR( CV_StsUnsupportedFormat,
				"FFMPEG could not find a codec matching the given FOURCC code. Use fourcc=CV_FOURCC_DEFAULT for auto selection." );
		}
	}
#else
        if( (codec_id = av_codec_get_id((const AVCodecTag**)(&codec_bmp_tags), fourcc)) == CODEC_ID_NONE ){
			CV_ERROR( CV_StsUnsupportedFormat,
				"FFMPEG could not find a codec matching the given FOURCC code. Use fourcc=CV_FOURCC_DEFAULT for auto selection." );
		}
	}
#endif

    // set a few optimal pixel formats for lossless codecs of interest..
    int codec_pix_fmt;
    switch (codec_id) {
#if LIBAVCODEC_VERSION_INT>((50<<16)+(1<<8)+0)
    case CODEC_ID_JPEGLS:
        // BGR24 or GRAY8 depending on is_color...
        codec_pix_fmt = writer->input_pix_fmt;
        break;
#endif
    case CODEC_ID_FFV1:
        // no choice... other supported formats are YUV only
        codec_pix_fmt = PIX_FMT_RGBA32;
        break;
	case CODEC_ID_MJPEG:
	case CODEC_ID_LJPEG:
		codec_pix_fmt = PIX_FMT_YUVJ420P;
		break;
    case CODEC_ID_RAWVIDEO:
    default:
        // good for lossy formats, MPEG, etc.
        codec_pix_fmt = PIX_FMT_YUV420P;
        break;
    }

	// TODO -- safe to ignore output audio stream?
	writer->video_st = icv_add_video_stream_FFMPEG(writer->oc, codec_id,
			frameSize.width, frameSize.height, frameSize.width*frameSize.height*64,
            fps, codec_pix_fmt);


	/* set the output parameters (must be done even if no
       parameters). */
    if (av_set_parameters(writer->oc, NULL) < 0) {
		CV_ERROR(CV_StsBadArg, "Invalid output format parameters");
    }

    dump_format(writer->oc, 0, filename, 1);

    /* now that all the parameters are set, we can open the audio and
       video codecs and allocate the necessary encode buffers */
    if (!writer->video_st){
		CV_ERROR(CV_StsBadArg, "Couldn't open video stream");
	}

    AVCodec *codec;
    AVCodecContext *c;

#if LIBAVFORMAT_BUILD > 4628
    c = (writer->video_st->codec);
#else
    c = &(writer->video_st->codec);
#endif

    c->codec_tag = fourcc;
    /* find the video encoder */
    codec = avcodec_find_encoder(c->codec_id);
    if (!codec) {
		CV_ERROR(CV_StsBadArg, "codec not found");
    }

    /* open the codec */
    if ( (err=avcodec_open(c, codec)) < 0) {
		char errtext[256];
		sprintf(errtext, "Could not open codec '%s': %s", codec->name, icv_FFMPEG_ErrStr(err));
		CV_ERROR(CV_StsBadArg, errtext);
    }

    writer->outbuf = NULL;

    if (!(writer->oc->oformat->flags & AVFMT_RAWPICTURE)) {
        /* allocate output buffer */
		/* assume we will never get codec output with more than 4 bytes per pixel... */
		writer->outbuf_size = frameSize.width*frameSize.height*4;
        writer->outbuf = (uint8_t *) av_malloc(writer->outbuf_size);
    }

	bool need_color_convert;
	need_color_convert = (c->pix_fmt != writer->input_pix_fmt);

    /* allocate the encoded raw picture */
    writer->picture = icv_alloc_picture_FFMPEG(c->pix_fmt, c->width, c->height, need_color_convert);
    if (!writer->picture) {
		CV_ERROR(CV_StsNoMem, "Could not allocate picture");
    }

    /* if the output format is not our input format, then a temporary
       picture of the input format is needed too. It is then converted
	   to the required output format */
	writer->input_picture = NULL;
    if ( need_color_convert ) {
        writer->input_picture = icv_alloc_picture_FFMPEG(writer->input_pix_fmt, c->width, c->height, false);
        if (!writer->input_picture) {
			CV_ERROR(CV_StsNoMem, "Could not allocate picture");
        }
    }

	/* open the output file, if needed */
    if (!(writer->fmt->flags & AVFMT_NOFILE)) {
        if (url_fopen(&writer->oc->pb, filename, URL_WRONLY) < 0) {
			CV_ERROR(CV_StsBadArg, "Couldn't open output file for writing");
        }
    }

    /* write the stream header, if any */
    av_write_header( writer->oc );


	__END__;

	// return what we got
	return (CvVideoWriter *) writer;
}
