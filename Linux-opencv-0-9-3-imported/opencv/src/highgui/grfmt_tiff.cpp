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
#include "grfmt_tiff.h"

static const char fmtDescrTiff[] = "TIFF (*.tiff;*.tif)";
static const char fmtSignTiffII[] = "II\x2a\x00";
static const char fmtSignTiffMM[] = "MM\x00\x2a";
static const int  tiffMask[] = { 0xff, 0xff, 0xffffffff, 0xffff, 0xffffffff };

/************************ TIFF reader *****************************/

GrFmtTiffReader::GrFmtTiffReader()
{
    m_sign_len = 4;
    m_signature = fmtSignTiffII;
    m_description = fmtDescrTiff;
    m_offsets = 0;
    m_maxoffsets = 0;
    m_strips = -1;
    m_max_pal_length = 0;
    m_temp_palette = 0;
}


GrFmtTiffReader::~GrFmtTiffReader()
{
    Close();

    delete m_offsets;
    delete m_temp_palette;
}

void  GrFmtTiffReader::Close()
{
    m_strm.Close();
}


bool  GrFmtTiffReader::CheckFormat( const char* signature )
{
    return memcmp( signature, fmtSignTiffII, 4 ) == 0 ||
           memcmp( signature, fmtSignTiffMM, 4 ) == 0;
}


int   GrFmtTiffReader::GetWordEx()
{
    int val = m_strm.GetWord();
    if( m_byteorder == TIFF_ORDER_MM )
        val = ((val)>>8)|(((val)&0xff)<<8);
    return val;
}


int   GrFmtTiffReader::GetDWordEx()
{
    int val = m_strm.GetDWord();
    if( m_byteorder == TIFF_ORDER_MM )
        val = BSWAP( val );
    return val;
}


void  GrFmtTiffReader::ReadTable( int offset, int count,
                                  TiffFieldType fieldType,
                                  int*& array, int& arraysize )
{
    int i;
    
    if( count < 0 )
        BAD_HEADER_ERR();
    
    if( fieldType != TIFF_TYPE_SHORT &&
        fieldType != TIFF_TYPE_LONG &&
        fieldType != TIFF_TYPE_BYTE )
        BAD_HEADER_ERR();

    if( count > arraysize )
    {
        delete array;
        arraysize = arraysize*3/2;
        if( arraysize < count )
            arraysize = count;
        array = new int[arraysize];
    }

    if( count > 1 )
    {
        int pos = m_strm.GetPos();
        m_strm.SetPos( offset );

        if( fieldType == TIFF_TYPE_LONG )
        {
            if( m_byteorder == TIFF_ORDER_MM )
                for( i = 0; i < count; i++ )
                    array[i] = ((RMByteStream&)m_strm).GetDWord();
            else
                for( i = 0; i < count; i++ )
                    array[i] = ((RLByteStream&)m_strm).GetDWord();
        }
        else if( fieldType == TIFF_TYPE_SHORT )
        {
            if( m_byteorder == TIFF_ORDER_MM )
                for( i = 0; i < count; i++ )
                    array[i] = ((RMByteStream&)m_strm).GetWord();
            else
                for( i = 0; i < count; i++ )
                    array[i] = ((RLByteStream&)m_strm).GetWord();
        }
        else // fieldType == TIFF_TYPE_BYTE
            for( i = 0; i < count; i++ )
                array[i] = m_strm.GetByte();

        m_strm.SetPos(pos);
    }
    else
    {
        assert( (offset & ~tiffMask[fieldType]) == 0 );
        array[0] = offset;
    }
}


bool  GrFmtTiffReader::ReadHeader()
{
    bool result = false;
    int  photometric = -1;
    int  channels = 1;
    int  pal_length = -1;

    const int MAX_CHANNELS = 4;
    int  bpp_arr[MAX_CHANNELS];

    assert( strlen(m_filename) != 0 );
    if( !m_strm.Open( m_filename )) return false;

    m_width = -1;
    m_height = -1;
    m_strips = -1;
    m_bpp = 1;
    m_compression = TIFF_UNCOMP;
    m_rows_per_strip = -1;

    try
    {
        m_byteorder = (TiffByteOrder)m_strm.GetWord();
        m_strm.Skip( 2 );
        int header_offset = GetDWordEx();

        m_strm.SetPos( header_offset );

        // read the first tag directory
        int i, j, count = GetWordEx();

        for( i = 0; i < count; i++ )
        {
            // read tag
            TiffTag tag = (TiffTag)GetWordEx();
            TiffFieldType fieldType = (TiffFieldType)GetWordEx();
            int count = GetDWordEx();
            int value = GetDWordEx();
            if( count == 1 )
            {
                if( m_byteorder == TIFF_ORDER_MM )
                {
                    if( fieldType == TIFF_TYPE_SHORT )
                        value = (unsigned)value >> 16;
                    else if( fieldType == TIFF_TYPE_BYTE )
                        value = (unsigned)value >> 24;
                }

                value &= tiffMask[fieldType];
            }

            switch( tag )
            {
            case  TIFF_TAG_WIDTH:
                m_width = value;
                break;

            case  TIFF_TAG_HEIGHT:
                m_height = value;
                break;

            case  TIFF_TAG_BITS_PER_SAMPLE:
                {
                    int* bpp_arr_ref = bpp_arr;

                    if( count > MAX_CHANNELS )
                        BAD_HEADER_ERR();

                    ReadTable( value, count, fieldType, bpp_arr_ref, count );
                
                    for( j = 1; j < count; j++ )
                    {
                        if( bpp_arr[j] != bpp_arr[0] )
                        {
                            BAD_HEADER_ERR();
                        }
                    }

                    m_bpp = bpp_arr[0];
                }

                break;

            case  TIFF_TAG_COMPRESSION:
                m_compression = (TiffCompression)value;
                if( m_compression != TIFF_UNCOMP &&
                    m_compression != TIFF_HUFFMAN &&
                    m_compression != TIFF_PACKBITS )
                    BAD_HEADER_ERR();
                break;

            case  TIFF_TAG_PHOTOMETRIC:
                photometric = value;
                if( (unsigned)photometric > 3 )
                    BAD_HEADER_ERR();
                break;

            case  TIFF_TAG_STRIP_OFFSETS:
                m_strips = count;
                ReadTable( value, count, fieldType, m_offsets, m_maxoffsets );
                break;

            case  TIFF_TAG_SAMPLES_PER_PIXEL:
                channels = value;
                if( channels != 1 && channels != 3 && channels != 4 )
                    BAD_HEADER_ERR();
                break;

            case  TIFF_TAG_ROWS_PER_STRIP:
                m_rows_per_strip = value;
                break;

            case  TIFF_TAG_PLANAR_CONFIG:
                {
                int planar_config = value;
                if( planar_config != 1 )
                    BAD_HEADER_ERR();
                }
                break;

            case  TIFF_TAG_COLOR_MAP:
                if( fieldType != TIFF_TYPE_SHORT || count < 2 )
                    BAD_HEADER_ERR();
                ReadTable( value, count, fieldType,
                           m_temp_palette, m_max_pal_length );
                pal_length = count / 3;
                if( pal_length > 256 )
                    BAD_HEADER_ERR();
                for( i = 0; i < pal_length; i++ )
                {
                    m_palette[i].r = (uchar)(m_temp_palette[i] >> 8);
                    m_palette[i].g = (uchar)(m_temp_palette[i + pal_length] >> 8);
                    m_palette[i].b = (uchar)(m_temp_palette[i + pal_length*2] >> 8);
                }
                break;
            case  TIFF_TAG_STRIP_COUNTS:
                break;
            }
        }

        if( m_strips == 1 && m_rows_per_strip == -1 )
            m_rows_per_strip = m_height;

        if( m_width > 0 && m_height > 0 && m_strips > 0 &&
            (m_height + m_rows_per_strip - 1)/m_rows_per_strip == m_strips )
        {
            switch( m_bpp )
            {
            case 1:
                if( photometric == 0 || photometric == 1 && channels == 1 )
                {
                    FillGrayPalette( m_palette, m_bpp, photometric == 0 );
                    result = true;
                    m_iscolor = false;
                }
                break;
            case 4:
            case 8:
                if( (photometric == 0 || photometric == 1 ||
                     photometric == 3 && pal_length == (1 << m_bpp)) &&
                    m_compression != TIFF_HUFFMAN && channels == 1 )
                {
                    if( pal_length < 0 )
                    {
                        FillGrayPalette( m_palette, m_bpp, photometric == 0 );
                        m_iscolor = false;
                    }
                    else
                    {
                        m_iscolor = IsColorPalette( m_palette, m_bpp );
                    }
                    result = true;
                }
                else if( photometric == 2 && pal_length < 0 &&
                         (channels == 3 || channels == 4) &&
                         m_compression == TIFF_UNCOMP )
                {
                    m_bpp = 8*channels;
                    m_iscolor = true;
                    result = true;
                }
                break;
            default:
                BAD_HEADER_ERR();
            }
        }
    }
    catch( int )
    {
    }

    if( !result )
    {
        m_strips = -1;
        m_width = m_height = -1;
        m_strm.Close();
    }

    return result;
}


bool  GrFmtTiffReader::ReadData( uchar* data, int step, int color )
{
    const  int buffer_size = 1 << 12;
    uchar  buffer[buffer_size];
    uchar  gray_palette[256];
    bool   result = false;
    uchar* src = buffer;
    int    src_pitch = (m_width*m_bpp + 7)/8;
    int    y = 0;

    if( m_strips < 0 || !m_strm.IsOpened())
        return false;
    
    if( src_pitch+32 > buffer_size )
        src = new uchar[src_pitch+32];

    if( !color )
        if( m_bpp <= 8 )
        {
            CvtPaletteToGray( m_palette, gray_palette, 1 << m_bpp );
        }

    try
    {
        for( int s = 0; s < m_strips; s++ )
        {
            int y_limit = m_rows_per_strip;

            y_limit += y;
            if( y_limit > m_height ) y_limit = m_height;

            m_strm.SetPos( m_offsets[s] );

            if( m_compression == TIFF_UNCOMP )
            {
                for( ; y < y_limit; y++, data += step )
                {
                    m_strm.GetBytes( src, src_pitch );
                    if( color )
                        switch( m_bpp )
                        {
                        case 1:
                            FillColorRow1( data, src, m_width, m_palette );
                            break;
                        case 4:
                            FillColorRow4( data, src, m_width, m_palette );
                            break;
                        case 8:
                            FillColorRow8( data, src, m_width, m_palette );
                            break;
                        case 24:
                            CvtRGBToBGR( src, data, m_width );
                            break;
                        case 32:
                            CvtRGBAToBGR( src, data, m_width );
                            break;
                        default:
                            assert(0);
                            goto bad_decoding_end;
                        }
                    else
                        switch( m_bpp )
                        {
                        case 1:
                            FillGrayRow1( data, src, m_width, gray_palette );
                            break;
                        case 4:
                            FillGrayRow4( data, src, m_width, gray_palette );
                            break;
                        case 8:
                            FillGrayRow8( data, src, m_width, gray_palette );
                            break;
                        case 24:
                            CvtRGBToGray( src, data, m_width );
                            break;
                        case 32:
                            CvtRGBAToGray( src, data, m_width );
                            break;
                        default:
                            assert(0);
                            goto bad_decoding_end;
                        }
                }
            }
            else
            {
            }

            result = true;

bad_decoding_end:

            ;
        }
    }
    catch( int )
    {
    }

    if( src != buffer ) delete src; 
    return result;
}


//////////////////////////////////////////////////////////////////////////////////////////

GrFmtTiffWriter::GrFmtTiffWriter()
{
    m_description = fmtDescrTiff;
}


GrFmtTiffWriter::~GrFmtTiffWriter()
{
}


void  GrFmtTiffWriter::WriteTag( TiffTag tag, TiffFieldType fieldType,
                                 int count, int value )
{
    m_strm.PutWord( tag );
    m_strm.PutWord( fieldType );
    m_strm.PutDWord( count );
    m_strm.PutDWord( value );
}


bool  GrFmtTiffWriter::WriteImage( const uchar* data, int step,
                                   int width, int height, bool isColor )
{
    bool result = false;
    int nch  = isColor ? 3 : 1;
    int fileStep = width*nch;

    assert( data && width > 0 && height > 0 && step >= fileStep);

    if( m_strm.Open( m_filename ) )
    {
        int rowsPerStrip = (1 << 13)/fileStep;

        if( rowsPerStrip < 1 )
            rowsPerStrip = 1;

        if( rowsPerStrip > height )
            rowsPerStrip = height;

        int i, stripCount = (height + rowsPerStrip - 1) / rowsPerStrip;
#if defined _DEBUG || !defined WIN32
        int uncompressedRowSize = rowsPerStrip * fileStep;
#endif
        int directoryOffset = 0;

        int* stripOffsets = new int[stripCount];
        short* stripCounts = new short[stripCount];
        uchar* buffer = new uchar[fileStep + 32];
        int  stripOffsetsOffset = 0;
        int  stripCountsOffset = 0;
        int  bitsPerSample = 8;
        int  y = 0;

        m_strm.PutBytes( fmtSignTiffII, 4 );
        m_strm.PutDWord( directoryOffset );

        // write an image data first (the most reasonable way
        // for compressed images)
        for( i = 0; i < stripCount; i++ )
        {
            int limit = y + rowsPerStrip;

            if( limit > height )
                limit = height;

            stripOffsets[i] = m_strm.GetPos();

            for( ; y < limit; y++, data += step )
            {
                if( isColor ) CvtRGBToBGR( data, buffer, width );
                m_strm.PutBytes( isColor ? buffer : data, fileStep );
            }

            stripCounts[i] = (short)(m_strm.GetPos() - stripOffsets[i]);
            assert( stripCounts[i] == uncompressedRowSize ||
                    stripCounts[i] < uncompressedRowSize &&
                    i == stripCount - 1);
        }

        if( stripCount > 1 )
        {
            stripOffsetsOffset = m_strm.GetPos();
            for( i = 0; i < stripCount; i++ )
                m_strm.PutDWord( stripOffsets[i] );

            stripCountsOffset = m_strm.GetPos();
            for( i = 0; i < stripCount; i++ )
                m_strm.PutWord( stripCounts[i] );
        }
        else
        {
            stripOffsetsOffset = stripOffsets[0];
            stripCountsOffset = stripCounts[0];
        }

        if( isColor )
        {
            bitsPerSample = m_strm.GetPos();
            m_strm.PutWord(8);
            m_strm.PutWord(8);
            m_strm.PutWord(8);
        }

        directoryOffset = m_strm.GetPos();

        // write header
        m_strm.PutWord( 9 );

        WriteTag( TIFF_TAG_WIDTH, TIFF_TYPE_LONG, 1, width );
        WriteTag( TIFF_TAG_HEIGHT, TIFF_TYPE_LONG, 1, height );
        WriteTag( TIFF_TAG_BITS_PER_SAMPLE,
                  TIFF_TYPE_SHORT, nch, bitsPerSample );
        WriteTag( TIFF_TAG_COMPRESSION, TIFF_TYPE_LONG, 1, TIFF_UNCOMP );
        WriteTag( TIFF_TAG_PHOTOMETRIC, TIFF_TYPE_SHORT, 1, isColor ? 2 : 1 );

        WriteTag( TIFF_TAG_SAMPLES_PER_PIXEL, TIFF_TYPE_SHORT, 1, nch );
        WriteTag( TIFF_TAG_ROWS_PER_STRIP, TIFF_TYPE_LONG, 1, rowsPerStrip );
        
        WriteTag( TIFF_TAG_STRIP_OFFSETS, TIFF_TYPE_LONG,
                  stripCount, stripOffsetsOffset );

        WriteTag( TIFF_TAG_STRIP_COUNTS,
                  stripCount > 1 ? TIFF_TYPE_SHORT : TIFF_TYPE_LONG,
                  stripCount, stripCountsOffset );

        m_strm.PutDWord(0);
        m_strm.Close();

        // write directory offset
        FILE* f = fopen( m_filename, "r+b" );
        buffer[0] = (uchar)directoryOffset;
        buffer[1] = (uchar)(directoryOffset >> 8);
        buffer[2] = (uchar)(directoryOffset >> 16);
        buffer[3] = (uchar)(directoryOffset >> 24);

        fseek( f, 4, SEEK_SET );
        fwrite( buffer, 1, 4, f );
        fclose(f);

        delete  stripOffsets;
        delete  stripCounts;

        result = true;
    }
    return result;
}


