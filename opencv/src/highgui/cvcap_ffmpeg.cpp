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
}


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
        cvFree( (void**)&capture->rgb_picture.data[0] );

    memset( &capture->frame, 0, sizeof(capture->frame));
}


static int icvOpenAVI_FFMPEG( CvCaptureAVI_FFMPEG* capture, const char* filename )
{
    int err, valid = 0, video_index = -1, i;
    AVFormatContext *ic;

    capture->ic = NULL;
    capture->video_stream = -1;
    capture->video_st = NULL;
    /* register all codecs, demux and protocols */
    av_register_all();

    err = av_open_input_file(&ic, filename, NULL, 0, NULL);
    if (err < 0) {
    fprintf(stderr, "HIGHGUI ERROR: AVI: %s: could not open file\n", filename);
    goto exit_func;
    }
    capture->ic = ic;
    err = av_find_stream_info(ic);
    if (err < 0) {
    fprintf(stderr, "HIGHGUI ERROR: AVI: %s: could not find codec parameters\n", filename);
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

        capture->rgb_picture.data[0] = (uchar*)cvAlloc(
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


    if(video_index >= 0)
    valid = 1;

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


static int icvSetPropertyAVI_FFMPEG( CvCaptureAVI_FFMPEG* capture,
                                     int property_id, double value );

static double icvGetPropertyAVI_FFMPEG( CvCaptureAVI_FFMPEG* capture, int property_id )
{
    if( !capture || !capture->video_st || !capture->picture->data[0] )
    return 0;

    int64_t timestamp;
    timestamp = capture->picture_pts;

    switch( property_id )
    {
    case CV_CAP_PROP_POS_MSEC:
        if(capture->ic->start_time != static_cast<double>(AV_NOPTS_VALUE))
        return (double)(timestamp - capture->ic->start_time)*1000/(double)AV_TIME_BASE;
        break;
    case CV_CAP_PROP_POS_FRAMES:
    if(capture->video_st->cur_dts != static_cast<double>(AV_NOPTS_VALUE))
        return (double)capture->video_st->cur_dts-1;
    break;
    case CV_CAP_PROP_POS_AVI_RATIO:
    if(capture->ic->start_time != static_cast<double>(AV_NOPTS_VALUE) && capture->ic->duration != static_cast<double>(AV_NOPTS_VALUE))
        return (double)(timestamp-capture->ic->start_time)/(double)capture->ic->duration;
    break;
    case CV_CAP_PROP_FRAME_WIDTH:
        return capture->frame.width;
    break;
    case CV_CAP_PROP_FRAME_HEIGHT:
        return capture->frame.height;
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


static int icvSetPropertyAVI_FFMPEG( CvCaptureAVI_FFMPEG* capture,
                                     int property_id, double value )
{
    if( !capture || !capture->video_st || !capture->picture->data[0] )
    return 0;
    switch( property_id )
    {
#if 0    
    case CV_CAP_PROP_POS_MSEC:
    case CV_CAP_PROP_POS_FRAMES:
    case CV_CAP_PROP_POS_AVI_RATIO:
        {
        int64_t timestamp = AV_NOPTS_VALUE;
        switch( property_id )
            {
        case CV_CAP_PROP_POS_FRAMES:
        if(capture->ic->start_time != AV_NOPTS_VALUE) {
            value *= (double)capture->video_st->codec.frame_rate_base
            / (double)capture->video_st->codec.frame_rate;
            timestamp = capture->ic->start_time+(int64_t)(value*AV_TIME_BASE);
        }
        break;
        case CV_CAP_PROP_POS_MSEC:
        if(capture->ic->start_time != AV_NOPTS_VALUE)
            timestamp = capture->ic->start_time+(int64_t)(value*AV_TIME_BASE/1000);
        break;
        case CV_CAP_PROP_POS_AVI_RATIO:
        if(capture->ic->start_time != AV_NOPTS_VALUE && capture->ic->duration != AV_NOPTS_VALUE)
            timestamp = capture->ic->start_time+(int64_t)(value*capture->ic->duration);
        break;
        }
        if(timestamp != AV_NOPTS_VALUE) {
        //printf("timestamp=%g\n",(double)timestamp);
        int ret = av_seek_frame(capture->ic, -1, timestamp, 0);
        if (ret < 0) {
            fprintf(stderr, "HIGHGUI ERROR: AVI: could not seek to position %0.3f\n", 
                (double)timestamp / AV_TIME_BASE);
            return 0;
        }
        }
    }
        break;
#endif  
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
            cvReleaseCapture( (CvCapture**)&capture );
    }

    return (CvCapture*)capture;
}

#if 0

typedef struct CvAVI_FFMPEG_Writer
{
    AVCodec         * codec;
    AVCodecContext  * context;
    uint8_t         * outbuf;
    uint32_t          outbuf_size;
    FILE            * outfile;
    
    AVFrame         * picture;
    AVFrame         * rgb_picture;
    uint8_t         * picbuf;
} CvAVI_FFMPEG_Writer;


// shorthand for specifying correct four character code cookies
#ifndef MKTAG
#define MKTAG(a,b,c,d) (a | (b << 8) | (c << 16) | (d << 24))
#endif


/**
 * the following function is a modified version of code
 * found in ffmpeg-0.4.9-pre1/libavcodec/avcodec.c
 */
static AVCodec* icv_avcodec_find_by_fcc_FFMPEG(uint32_t fcc)
{
    // translation table
    static const struct fcc_to_avcodecid {
        enum CodecID codec;
        uint32_t list[4]; // maybe we could map more fcc to same codec
    } lc[] = {
    { CODEC_ID_H263,       { MKTAG('U', '2', '6', '3'), 0 } },
    { CODEC_ID_H263I,      { MKTAG('I', '2', '6', '3'), 0 } },
    { CODEC_ID_MSMPEG4V3,  { MKTAG('D', 'I', 'V', '3'), 0 } },
    { CODEC_ID_MPEG4,      { MKTAG('D', 'I', 'V', 'X'),  MKTAG('D', 'X', '5', '0'), 0 } },
    { CODEC_ID_MSMPEG4V2,  { MKTAG('M', 'P', '4', '2'), 0 } },
    { CODEC_ID_MJPEG,      { MKTAG('M', 'J', 'P', 'G'), 0 } },
    { CODEC_ID_MPEG1VIDEO, { MKTAG('P', 'I', 'M', '1'), 0 } },
    { CODEC_ID_AC3,        { 0x2000, 0 } },
    { CODEC_ID_MP2,        { 0x50, 0x55, 0 } },
    { CODEC_ID_FLV1,       { MKTAG('F', 'L', 'V', '1'), 0 } },
        
    { CODEC_ID_NONE, {0}}
    };
    const struct fcc_to_avcodecid* c;
    
    for (c = lc; c->codec != CODEC_ID_NONE; c++)
    {
        int i = 0;
        while (c->list[i] != 0)
            if (c->list[i++] == fcc)
        		//		return avcodec_find_decoder(c->codec); // original line
        		return avcodec_find_encoder(c->codec);
    }
    
    return NULL;
}

/**
 * the following function is a modified version of code
 * found in ffmpeg-0.4.9-pre1/output_example.c
 */
static AVFrame *icv_alloc_picture_FFMPEG(int pix_fmt, int width, int height)
{
    AVFrame * picture;
    uint8_t * picture_buf;
    int size;
    
    picture = avcodec_alloc_frame();
    if (!picture)
        return NULL;
    size = avpicture_get_size(pix_fmt, width, height);
    picture_buf = (uint8_t *) cvAlloc(size);
    if (!picture_buf) 
    {
        av_free(picture);
        return NULL;
    }
    avpicture_fill((AVPicture *)picture, picture_buf, 
                   pix_fmt, width, height);
    return picture;
}


/// Create a video writer object that uses FFMPEG
CV_IMPL CvVideoWriter* cvCreateVideoWriter( const char * filename, int fourcc,
                                            double fps, CvSize frameSize, int /*is_color*/ )
{
    
    CV_FUNCNAME( "cvCreateVideoWriter" );
    __BEGIN__;
    
    // check arguments
    CV_ASSERT (filename);
    CV_ASSERT (fps > 0);
    CV_ASSERT (frameSize.width > 0  &&  frameSize.height > 0);
    

    // allocate memory for structure...
    CvAVI_FFMPEG_Writer * writer = (CvAVI_FFMPEG_Writer *) cvAlloc( sizeof(CvAVI_FFMPEG_Writer));
    memset (writer, 0, sizeof (*writer));
    
    // tell FFMPEG to register codecs
    av_register_all();
    
    // lookup codec using the four character code
    writer->codec = icv_avcodec_find_by_fcc_FFMPEG( fourcc );
    if (!(writer->codec))
        CV_ERROR( CV_StsBadArg, "input_array or output_array are not valid matrices" );
    
    // alloc memory for context
    writer->context     = avcodec_alloc_context();
    CV_ASSERT (writer->context);
    
    // create / prepare rgb_picture (used for color conversion)    
    writer->rgb_picture = avcodec_alloc_frame();
    CV_ASSERT (writer->rgb_picture);
    
    // create / prepare picture (used for encoding)...    
    writer->picture     = icv_alloc_picture_FFMPEG(writer->context->pix_fmt, frameSize.width, frameSize.height);
    CV_ASSERT (writer->picture);
    
    // set parameters in context as desired...    
    writer->context->bit_rate         = 400000;      // TODO: BITRATE SETTINGS!
    writer->context->width            = frameSize.width;  
    writer->context->height           = frameSize.height;
    writer->context->frame_rate       = static_cast<int> (fps);
    writer->context->frame_rate_bas   =  1;
    writer->context->gop_size         = 10;
    writer->context->max_b_frames     =  0;          // TODO: WHAT TO DO WITH B-FRAMES IN OTHER CODECS?
    
    // try to open codec, exit if it fails
    if ( avcodec_open(writer->context, writer->codec) < 0)
    {
        fprintf(stderr, "HIGHGUI ERROR: Couldn't open codec\n");
        
        cvFree ( & writer->picture->data[0] );
        av_free(   writer->picture );
        av_free(   writer->rgb_picture );
        av_free(   writer->context );
        cvFree ( & writer );
        
        return 0;
    }
    
    // open output file
    writer->outfile = fopen(filename, "wb");
    if (!(writer->outfile))
    {
        fprintf(stderr, "HIGHGUI ERROR: Couldn't open file %s\n", filename);
        
        avcodec_close(writer->context);
        cvFree ( & writer->picture->data[0] );
        av_free(   writer->picture );
        av_free(   writer->rgb_picture );
        av_free(   writer->context );
        cvFree ( & writer );
        
        return 0;
    }
    
    // alloc image and output buffer
    writer->outbuf_size = avpicture_get_size (writer->context->pix_fmt, frameSize.width, frameSize.height);
    writer->outbuf      = (uint8_t *) cvAlloc (writer->outbuf_size);
    if (! (writer->outbuf))
    {
        fprintf(stderr, "HIGHGUI ERROR: Couldn't allocate memory for output buffer\n");
    
        fclose (writer->outfile);
        avcodec_close(writer->context);
        cvFree ( & writer->picture->data[0] );
        av_free(   writer->picture );
        av_free(   writer->rgb_picture );
        av_free(   writer->context );
        cvFree ( & writer );
        
        return 0;
    }
    
    __END__;
    
    // return what we got
    return (CvVideoWriter *) writer;
}

/// write a frame with FFMPEG
CV_IMPL int cvWriteFrame( CvVideoWriter * writer, const IplImage * image )
{
    
    CV_FUNCNAME ( "cvWriteFrame" );
    __BEGIN__;
    
    // check parameters
    CV_ASSERT ( image );
    CV_ASSERT ( image->nChannels == 3 );
    CV_ASSERT ( image->depth == IPL_DEPTH_8U );

    
    // check if buffer sizes match
    OPENCV_ASSERT ( image->imageSize == avpicture_get_size (PIX_FMT_BGR24, image->width, image->height),
                    "cvWriteFrame( CvVideoWriter *, const IplImage *)", "illegal image->imageSize");
    
    CvAVI_FFMPEG_Writer * mywriter = (CvAVI_FFMPEG_Writer*)writer;
    
    // let rgb_picture point to the raw data buffer of 'image'
    avpicture_fill((AVPicture *)mywriter->rgb_picture, (uint8_t *) image->imageData, 
                   PIX_FMT_BGR24, image->width, image->height);
    
    // convert to the color format needed by the codec
    img_convert((AVPicture *)mywriter->picture, mywriter->context->pix_fmt,
                (AVPicture *)mywriter->rgb_picture, PIX_FMT_BGR24, 
                image->width, image->height);
    
    // encode frame
    int outsize = avcodec_encode_video(mywriter->context, mywriter->outbuf,
                                       mywriter->outbuf_size, mywriter->picture);
    
    // write out data
    fwrite(mywriter->outbuf, 1, outsize, mywriter->outfile);
    
    __END__;
    
    return CV_StsOk;
}

/// close video output stream and free associated memory
CV_IMPL void cvReleaseVideoWriter( CvVideoWriter ** writer )
{
    // nothing to do if already released
    if ( !(*writer) )
        return;
    
    // release data structures in reverse order
    CvAVI_FFMPEG_Writer * mywriter = (CvAVI_FFMPEG_Writer*)(*writer);
    fclose(mywriter->outfile);
    avcodec_close(mywriter->context);
    cvFree ( & mywriter->picture->data[0] );
    av_free(   mywriter->picture );
    av_free(   mywriter->rgb_picture );
    av_free(   mywriter->context );
    cvFree ( writer );

    // mark as released
    (*writer) = 0;
}

#endif

typedef struct CvAVI_FFMPEG_Writer
{
    AVCodec         * codec;
    AVCodecContext  * context;
    uint8_t         * outbuf;
    uint32_t          outbuf_size;
    FILE            * outfile;
    
    AVFrame         * picture;
    AVFrame         * rgb_picture;
    uint8_t         * picbuf;
} CvAVI_FFMPEG_Writer;


// shorthand for specifying correct four character code cookies
#ifndef MKTAG
#define MKTAG(a,b,c,d) (a | (b << 8) | (c << 16) | (d << 24))
#endif


/**
 * the following function is a modified version of code
 * found in ffmpeg-0.4.9-pre1/libavcodec/avcodec.c
 */
static AVCodec* icv_avcodec_find_by_fcc_FFMPEG(uint32_t fcc)
{
    // translation table
    static const struct fcc_to_avcodecid {
        enum CodecID codec;
        uint32_t list[4]; // maybe we could map more fcc to same codec
    } lc[] = {
    { CODEC_ID_H263,       { MKTAG('U', '2', '6', '3'), 0 } },
    { CODEC_ID_H263I,      { MKTAG('I', '2', '6', '3'), 0 } },
    { CODEC_ID_MSMPEG4V3,  { MKTAG('D', 'I', 'V', '3'), 0 } },
    { CODEC_ID_MPEG4,      { MKTAG('D', 'I', 'V', 'X'),  MKTAG('D', 'X', '5', '0'), 0 } },
    { CODEC_ID_MSMPEG4V2,  { MKTAG('M', 'P', '4', '2'), 0 } },
    { CODEC_ID_MJPEG,      { MKTAG('M', 'J', 'P', 'G'), 0 } },
    { CODEC_ID_MPEG1VIDEO, { MKTAG('P', 'I', 'M', '1'), 0 } },
    { CODEC_ID_AC3,        { 0x2000, 0 } },
    { CODEC_ID_MP2,        { 0x50, 0x55, 0 } },
    { CODEC_ID_FLV1,       { MKTAG('F', 'L', 'V', '1'), 0 } },
        
    { CODEC_ID_NONE, {0}}
    };
    const struct fcc_to_avcodecid* c;
    
    for (c = lc; c->codec != CODEC_ID_NONE; c++)
    {
        int i = 0;
        while (c->list[i] != 0)
            if (c->list[i++] == fcc)
                //      return avcodec_find_decoder(c->codec); // original line
                return avcodec_find_encoder(c->codec);
    }
    
    return NULL;
}

/**
 * the following function is a modified version of code
 * found in ffmpeg-0.4.9-pre1/output_example.c
 */
static AVFrame *icv_alloc_picture_FFMPEG(int pix_fmt, int width, int height)
{
    AVFrame * picture;
    uint8_t * picture_buf;
    int size;
    
    picture = avcodec_alloc_frame();
    if (!picture)
        return NULL;
    size = avpicture_get_size(pix_fmt, width, height);
    picture_buf = (uint8_t *) cvAlloc(size);
    if (!picture_buf) 
    {
        av_free(picture);
        return NULL;
    }
    avpicture_fill((AVPicture *)picture, picture_buf, 
                   pix_fmt, width, height);
    return picture;
}


/// Create a video writer object that uses FFMPEG
CV_IMPL CvVideoWriter* cvCreateVideoWriter( const char * filename, int fourcc,
                                            double fps, CvSize frameSize, int /*is_color*/ )
{
    // check arguments
    assert (filename);
    assert (fps > 0);
    assert (frameSize.width > 0  &&  frameSize.height > 0);
    

    // allocate memory for structure...
    CvAVI_FFMPEG_Writer * writer = (CvAVI_FFMPEG_Writer *) cvAlloc( sizeof(CvAVI_FFMPEG_Writer));
    memset (writer, 0, sizeof (*writer));
    
    // tell FFMPEG to register codecs
    av_register_all ();
    
    // lookup codec using the four character code
    writer->codec = icv_avcodec_find_by_fcc_FFMPEG (fourcc);
    if (!(writer->codec))
    {
      fprintf(stderr, "HIGHGUI ERROR: Unsupported video codec.\n");
      return 0;
    }
    
    // alloc memory for context
    writer->context     = avcodec_alloc_context();
    assert (writer->context);
    
    // TODO: WHAT TO REALLY PUT HERE?
    writer->context->pix_fmt = PIX_FMT_YUV420P;
    
    // create / prepare rgb_picture (used for color conversion)    
    writer->rgb_picture = avcodec_alloc_frame();
    assert (writer->rgb_picture);
    
    // create / prepare picture (used for encoding)...    
    writer->picture     = icv_alloc_picture_FFMPEG(writer->context->pix_fmt, frameSize.width, frameSize.height);
    assert (writer->picture);
    
    // set parameters in context as desired...    
    writer->context->bit_rate         = 400000;      // TODO: BITRATE SETTINGS!
    writer->context->width            = frameSize.width;  
    writer->context->height           = frameSize.height;
#if LIBAVCODEC_BUILD > 4753
    writer->context->time_base.num    = 1;
    writer->context->time_base.den    = static_cast<int> (fps);
#else
    writer->context->frame_rate       = static_cast<int> (fps);
    writer->context->frame_rate_base  =  1;
#endif
    writer->context->gop_size         = 10;
    writer->context->max_b_frames     =  0;          // TODO: WHAT TO DO WITH B-FRAMES IN OTHER CODECS?
    
    // try to open codec, exit if it fails
    if ( avcodec_open(writer->context, writer->codec) < 0)
    {
        fprintf(stderr, "HIGHGUI ERROR: Couldn't open codec\n");
        
        cvFree ( (void **) & writer->picture->data[0] );
        av_free(   writer->picture );
        av_free(   writer->rgb_picture );
        av_free(   writer->context );
        cvFree ( (void **) & writer );
        
        return 0;
    }
    
    // open output file
    writer->outfile = fopen(filename, "wb");
    if (!(writer->outfile))
    {
        fprintf(stderr, "HIGHGUI ERROR: Couldn't open file %s\n", filename);
        
        avcodec_close(writer->context);
        cvFree ( (void **) & writer->picture->data[0] );
        av_free(   writer->picture );
        av_free(   writer->rgb_picture );
        av_free(   writer->context );
        cvFree ( (void **) & writer );
        
        return 0;
    }
    
    // alloc image and output buffer
    writer->outbuf_size = avpicture_get_size (writer->context->pix_fmt, frameSize.width, frameSize.height);
    writer->outbuf      = (uint8_t *) cvAlloc (writer->outbuf_size);
    if (! (writer->outbuf))
    {
        fprintf(stderr, "HIGHGUI ERROR: Couldn't allocate memory for output buffer\n");
    
        fclose (writer->outfile);
        avcodec_close(writer->context);
        cvFree ( (void **) & writer->picture->data[0] );
        av_free(   writer->picture );
        av_free(   writer->rgb_picture );
        av_free(   writer->context );
        cvFree ( (void **) & writer );
        
        return 0;
    }
    
    // return what we got
    return (CvVideoWriter *) writer;
}

/// write a frame with FFMPEG
CV_IMPL int cvWriteFrame( CvVideoWriter * writer, const IplImage * image )
{
    
    // check parameters
    assert ( image );
    assert ( image->nChannels == 3 );
    assert ( image->depth == IPL_DEPTH_8U );

    
    // check if buffer sizes match, i.e. image has expected format (size, channels, bitdepth, alignment)
    assert (image->imageSize == avpicture_get_size (PIX_FMT_BGR24, image->width, image->height));
    
    // typecast from opaque data type to implemented struct
    CvAVI_FFMPEG_Writer * mywriter = (CvAVI_FFMPEG_Writer*) writer;
    
    // let rgb_picture point to the raw data buffer of 'image'
    avpicture_fill((AVPicture *)mywriter->rgb_picture, (uint8_t *) image->imageData, 
                   PIX_FMT_BGR24, image->width, image->height);
    
    // convert to the color format needed by the codec
    img_convert((AVPicture *)mywriter->picture, mywriter->context->pix_fmt,
                (AVPicture *)mywriter->rgb_picture, PIX_FMT_BGR24, 
                image->width, image->height);
    
    // encode frame
    int outsize = avcodec_encode_video(mywriter->context, mywriter->outbuf,
                                       mywriter->outbuf_size, mywriter->picture);
    
    // write out data
    fwrite(mywriter->outbuf, 1, outsize, mywriter->outfile);
    
    
    return CV_StsOk;
}

/// close video output stream and free associated memory
CV_IMPL void cvReleaseVideoWriter( CvVideoWriter ** writer )
{
    // nothing to do if already released
    if ( !(*writer) )
        return;
    
    // release data structures in reverse order
    CvAVI_FFMPEG_Writer * mywriter = (CvAVI_FFMPEG_Writer*)(*writer);
    fclose(mywriter->outfile);
    avcodec_close(mywriter->context);
    cvFree ( (void **) & mywriter->picture->data[0] );
    av_free(   mywriter->picture );
    av_free(   mywriter->rgb_picture );
    av_free(   mywriter->context );
    cvFree ( (void **) writer );

    // mark as released
    (*writer) = 0;
}
