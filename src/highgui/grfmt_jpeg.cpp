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
#include "grfmt_jpeg.h"

#ifdef HAVE_JPEG

// JPEG filter factory

GrFmtJpeg::GrFmtJpeg()
{
    m_sign_len = 3;
    m_signature = "\xFF\xD8\xFF";
    m_description = "JPEG files (*.jpeg;*.jpg;*.jpe)";
}


GrFmtJpeg::~GrFmtJpeg()
{
}


GrFmtReader* GrFmtJpeg::NewReader( const char* filename )
{
    return new GrFmtJpegReader( filename );
}


GrFmtWriter* GrFmtJpeg::NewWriter( const char* filename )
{
    return new GrFmtJpegWriter( filename );
}


/****************************************************************************************\
    This part of the file implements JPEG codec on base of IJG libjpeg library,
    in particular, this is the modified example.doc from libjpeg package.
    See otherlibs/_graphics/readme.txt for copyright notice.
\****************************************************************************************/

#include <stdio.h>
#include <setjmp.h>

#ifdef WIN32

#define XMD_H // prevent redefinition of INT32
#undef FAR  // prevent FAR redefinition

#endif

#if defined WIN32 && defined __GNUC__
typedef unsigned char boolean;
#endif

extern "C" {
#include "jpeglib.h"
}

/////////////////////// Error processing /////////////////////

typedef struct GrFmtJpegErrorMgr
{
    struct jpeg_error_mgr pub;    /* "parent" structure */
    jmp_buf setjmp_buffer;        /* jump label */
}
GrFmtJpegErrorMgr;


METHODDEF(void)
error_exit( j_common_ptr cinfo )
{
    GrFmtJpegErrorMgr* err_mgr = (GrFmtJpegErrorMgr*)(cinfo->err);

    /* Return control to the setjmp point */
    longjmp( err_mgr->setjmp_buffer, 1 );
}


/////////////////////// GrFmtJpegReader ///////////////////


GrFmtJpegReader::GrFmtJpegReader( const char* filename ) : GrFmtReader( filename )
{
    m_cinfo = 0;
    m_f = 0;
}


GrFmtJpegReader::~GrFmtJpegReader()
{
}


void  GrFmtJpegReader::Close()
{
    if( m_f )
    {
        fclose( m_f );
        m_f = 0;
    }

    if( m_cinfo )
    {
        jpeg_decompress_struct* cinfo = (jpeg_decompress_struct*)m_cinfo;
        GrFmtJpegErrorMgr* jerr = (GrFmtJpegErrorMgr*)m_jerr;

        jpeg_destroy_decompress( cinfo );
        delete cinfo;
        delete jerr;
        m_cinfo = 0;
        m_jerr = 0;
    }
    GrFmtReader::Close();
}


bool  GrFmtJpegReader::ReadHeader()
{
    bool result = false;
    Close();

    jpeg_decompress_struct* cinfo = new jpeg_decompress_struct;
    GrFmtJpegErrorMgr* jerr = new GrFmtJpegErrorMgr;

    cinfo->err = jpeg_std_error(&jerr->pub);
    jerr->pub.error_exit = error_exit;

    m_cinfo = cinfo;
    m_jerr = jerr;

    if( setjmp( jerr->setjmp_buffer ) == 0 )
    {
        jpeg_create_decompress( cinfo );

        m_f = fopen( m_filename, "rb" );
        if( m_f )
        {
            jpeg_stdio_src( cinfo, m_f );
            jpeg_read_header( cinfo, TRUE );

            m_width = cinfo->image_width;
            m_height = cinfo->image_height;
            m_iscolor = cinfo->num_components > 1;

            result = true;
        }
    }

    if( !result )
        Close();

    return result;
}

/***************************************************************************
 * following code is for supporting MJPEG image files
 * based on a message of Laurent Pinchart on the video4linux mailing list
 ***************************************************************************/

/* JPEG DHT Segment for YCrCb omitted from MJPEG data */
static
unsigned char my_jpeg_odml_dht[0x1a4] = {
    0xff, 0xc4, 0x01, 0xa2,

    0x00, 0x00, 0x01, 0x05, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b,

    0x01, 0x00, 0x03, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
    0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b,

    0x10, 0x00, 0x02, 0x01, 0x03, 0x03, 0x02, 0x04, 0x03, 0x05, 0x05, 0x04,
    0x04, 0x00, 0x00, 0x01, 0x7d,
    0x01, 0x02, 0x03, 0x00, 0x04, 0x11, 0x05, 0x12, 0x21, 0x31, 0x41, 0x06,
    0x13, 0x51, 0x61, 0x07,
    0x22, 0x71, 0x14, 0x32, 0x81, 0x91, 0xa1, 0x08, 0x23, 0x42, 0xb1, 0xc1,
    0x15, 0x52, 0xd1, 0xf0,
    0x24, 0x33, 0x62, 0x72, 0x82, 0x09, 0x0a, 0x16, 0x17, 0x18, 0x19, 0x1a,
    0x25, 0x26, 0x27, 0x28,
    0x29, 0x2a, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x43, 0x44, 0x45,
    0x46, 0x47, 0x48, 0x49,
    0x4a, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5a, 0x63, 0x64, 0x65,
    0x66, 0x67, 0x68, 0x69,
    0x6a, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7a, 0x83, 0x84, 0x85,
    0x86, 0x87, 0x88, 0x89,
    0x8a, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9a, 0xa2, 0xa3,
    0xa4, 0xa5, 0xa6, 0xa7,
    0xa8, 0xa9, 0xaa, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6, 0xb7, 0xb8, 0xb9, 0xba,
    0xc2, 0xc3, 0xc4, 0xc5,
    0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6, 0xd7, 0xd8,
    0xd9, 0xda, 0xe1, 0xe2,
    0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9, 0xea, 0xf1, 0xf2, 0xf3, 0xf4,
    0xf5, 0xf6, 0xf7, 0xf8,
    0xf9, 0xfa,

    0x11, 0x00, 0x02, 0x01, 0x02, 0x04, 0x04, 0x03, 0x04, 0x07, 0x05, 0x04,
    0x04, 0x00, 0x01, 0x02, 0x77,
    0x00, 0x01, 0x02, 0x03, 0x11, 0x04, 0x05, 0x21, 0x31, 0x06, 0x12, 0x41,
    0x51, 0x07, 0x61, 0x71,
    0x13, 0x22, 0x32, 0x81, 0x08, 0x14, 0x42, 0x91, 0xa1, 0xb1, 0xc1, 0x09,
    0x23, 0x33, 0x52, 0xf0,
    0x15, 0x62, 0x72, 0xd1, 0x0a, 0x16, 0x24, 0x34, 0xe1, 0x25, 0xf1, 0x17,
    0x18, 0x19, 0x1a, 0x26,
    0x27, 0x28, 0x29, 0x2a, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x43, 0x44,
    0x45, 0x46, 0x47, 0x48,
    0x49, 0x4a, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5a, 0x63, 0x64,
    0x65, 0x66, 0x67, 0x68,
    0x69, 0x6a, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7a, 0x82, 0x83,
    0x84, 0x85, 0x86, 0x87,
    0x88, 0x89, 0x8a, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9a,
    0xa2, 0xa3, 0xa4, 0xa5,
    0xa6, 0xa7, 0xa8, 0xa9, 0xaa, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6, 0xb7, 0xb8,
    0xb9, 0xba, 0xc2, 0xc3,
    0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6,
    0xd7, 0xd8, 0xd9, 0xda,
    0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9, 0xea, 0xf2, 0xf3, 0xf4,
    0xf5, 0xf6, 0xf7, 0xf8,
    0xf9, 0xfa
};

/*
 * Parse the DHT table.
 * This code comes from jpeg6b (jdmarker.c).
 */
static
int my_jpeg_load_dht (struct jpeg_decompress_struct *info, unsigned char *dht,
              JHUFF_TBL *ac_tables[], JHUFF_TBL *dc_tables[])
{
    unsigned int length = (dht[2] << 8) + dht[3] - 2;
    unsigned int pos = 4;
    unsigned int count, i;
    int index;

    JHUFF_TBL **hufftbl;
    unsigned char bits[17];
    unsigned char huffval[256];

    while (length > 16)
    {
       bits[0] = 0;
       index = dht[pos++];
       count = 0;
       for (i = 1; i <= 16; ++i)
       {
           bits[i] = dht[pos++];
           count += bits[i];
       }
       length -= 17;

       if (count > 256 || count > length)
           return -1;

       for (i = 0; i < count; ++i)
           huffval[i] = dht[pos++];
       length -= count;

       if (index & 0x10)
       {
           index -= 0x10;
           hufftbl = &ac_tables[index];
       }
       else
           hufftbl = &dc_tables[index];

       if (index < 0 || index >= NUM_HUFF_TBLS)
           return -1;

       if (*hufftbl == NULL)
           *hufftbl = jpeg_alloc_huff_table ((j_common_ptr)info);
       if (*hufftbl == NULL)
           return -1;

       memcpy ((*hufftbl)->bits, bits, sizeof (*hufftbl)->bits);
       memcpy ((*hufftbl)->huffval, huffval, sizeof (*hufftbl)->huffval);
    }

    if (length != 0)
       return -1;

    return 0;
}

/***************************************************************************
 * end of code for supportting MJPEG image files
 * based on a message of Laurent Pinchart on the video4linux mailing list
 ***************************************************************************/

bool  GrFmtJpegReader::ReadData( uchar* data, int step, int color )
{
    bool result = false;

    color = color > 0 || (m_iscolor && color < 0);

    if( m_cinfo && m_jerr && m_width && m_height )
    {
        jpeg_decompress_struct* cinfo = (jpeg_decompress_struct*)m_cinfo;
        GrFmtJpegErrorMgr* jerr = (GrFmtJpegErrorMgr*)m_jerr;
        JSAMPARRAY buffer = 0;

        if( setjmp( jerr->setjmp_buffer ) == 0 )
        {
            /* check if this is a mjpeg image format */
            if ( cinfo->ac_huff_tbl_ptrs[0] == NULL &&
                cinfo->ac_huff_tbl_ptrs[1] == NULL &&
                cinfo->dc_huff_tbl_ptrs[0] == NULL &&
                cinfo->dc_huff_tbl_ptrs[1] == NULL )
            {
                /* yes, this is a mjpeg image format, so load the correct
                huffman table */
                my_jpeg_load_dht( cinfo,
                    my_jpeg_odml_dht,
                    cinfo->ac_huff_tbl_ptrs,
                    cinfo->dc_huff_tbl_ptrs );
            }

            if( color > 0 || (m_iscolor && color < 0) )
            {
                color = 1;
                if( cinfo->num_components != 4 )
                {
                    cinfo->out_color_space = JCS_RGB;
                    cinfo->out_color_components = 3;
                }
                else
                {
                    cinfo->out_color_space = JCS_CMYK;
                    cinfo->out_color_components = 4;
                }
            }
            else
            {
                color = 0;
                if( cinfo->num_components != 4 )
                {
                    cinfo->out_color_space = JCS_GRAYSCALE;
                    cinfo->out_color_components = 1;
                }
                else
                {
                    cinfo->out_color_space = JCS_CMYK;
                    cinfo->out_color_components = 4;
                }
            }

            jpeg_start_decompress( cinfo );

            buffer = (*cinfo->mem->alloc_sarray)((j_common_ptr)cinfo,
                                              JPOOL_IMAGE, m_width*4, 1 );

            for( ; m_height--; data += step )
            {
                jpeg_read_scanlines( cinfo, buffer, 1 );
                if( color )
                {
                    if( cinfo->out_color_components == 3 )
                        icvCvt_RGB2BGR_8u_C3R( buffer[0], 0, data, 0, cvSize(m_width,1) );
                    else
                        icvCvt_CMYK2BGR_8u_C4C3R( buffer[0], 0, data, 0, cvSize(m_width,1) );
                }
                else
                {
                    if( cinfo->out_color_components == 1 )
                        memcpy( data, buffer[0], m_width );
                    else
                        icvCvt_CMYK2Gray_8u_C4C1R( buffer[0], 0, data, 0, cvSize(m_width,1) );
                }
            }
            result = true;
            jpeg_finish_decompress( cinfo );
        }
    }

    Close();
    return result;
}


/////////////////////// GrFmtJpegWriter ///////////////////

GrFmtJpegWriter::GrFmtJpegWriter( const char* filename ) : GrFmtWriter( filename )
{
}


GrFmtJpegWriter::~GrFmtJpegWriter()
{
}


bool  GrFmtJpegWriter::WriteImage( const uchar* data, int step,
                                   int width, int height, int /*depth*/, int _channels )
{
    const int default_quality = 95;
    struct jpeg_compress_struct cinfo;
    GrFmtJpegErrorMgr jerr;

    bool result = false;
    FILE* f = 0;
    int channels = _channels > 1 ? 3 : 1;
    uchar* buffer = 0; // temporary buffer for row flipping

    cinfo.err = jpeg_std_error(&jerr.pub);
    jerr.pub.error_exit = error_exit;

    if( setjmp( jerr.setjmp_buffer ) == 0 )
    {
        jpeg_create_compress(&cinfo);
        f = fopen( m_filename, "wb" );

        if( f )
        {
            jpeg_stdio_dest( &cinfo, f );

            cinfo.image_width = width;
            cinfo.image_height = height;
            cinfo.input_components = channels;
            cinfo.in_color_space = channels > 1 ? JCS_RGB : JCS_GRAYSCALE;

            jpeg_set_defaults( &cinfo );
            jpeg_set_quality( &cinfo, default_quality,
                              TRUE /* limit to baseline-JPEG values */ );
            jpeg_start_compress( &cinfo, TRUE );

            if( channels > 1 )
                buffer = new uchar[width*channels];

            for( ; height--; data += step )
            {
                uchar* ptr = (uchar*)data;

                if( _channels == 3 )
                {
                    icvCvt_BGR2RGB_8u_C3R( data, 0, buffer, 0, cvSize(width,1) );
                    ptr = buffer;
                }
                else if( _channels == 4 )
                {
                    icvCvt_BGRA2BGR_8u_C4C3R( data, 0, buffer, 0, cvSize(width,1), 2 );
                    ptr = buffer;
                }

                jpeg_write_scanlines( &cinfo, &ptr, 1 );
            }

            jpeg_finish_compress( &cinfo );
            result = true;
        }
    }

    if(f) fclose(f);
    jpeg_destroy_compress( &cinfo );

    delete[] buffer;
    return result;
}

#endif

/* End of file. */
