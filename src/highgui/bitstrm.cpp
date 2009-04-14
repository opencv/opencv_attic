/*M///////////////////////////////////////////////////////////////////////////////////////
//
//  IMPORTANT: READ BEFORE DOWNLOADING, COPYING, INSTALLING OR USING.
//
//  By downloading, copying, installing or using the software you agree to this license.
//  If you do not agree to this license, do not download, install,
//  copy or use the software.
//
//
//                           License Agreement
//                For Open Source Computer Vision Library
//
// Copyright (C) 2000-2008, Intel Corporation, all rights reserved.
// Copyright (C) 2009, Willow Garage Inc., all rights reserved.
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
//   * The name of the copyright holders may not be used to endorse or promote products
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
#include "bitstrm.h"

namespace cv
{

const int BS_DEF_BLOCK_SIZE = 1<<15;

void bsBSwapBlock( uchar *start, uchar *end )
{
    ulong* data = (ulong*)start;
    int i, size = (int)(end - start+3)/4;

    for( i = 0; i < size; i++ )
    {
        ulong temp = data[i];
        temp = BSWAP( temp );
        data[i] = temp;
    }
}

bool  bsIsBigEndian( void )
{
    return (((const int*)"\0\x1\x2\x3\x4\x5\x6\x7")[0] & 255) != 0;
}

/////////////////////////  RBaseStream ////////////////////////////

bool  RBaseStream::isOpened()
{ 
    return m_is_opened;
}

void  RBaseStream::allocate()
{
    if( !m_start )
    {
        m_start = new uchar[m_block_size + m_ungetsize];
        m_start+= m_ungetsize;
    }
    m_end = m_start + m_block_size;
    m_current = m_end;
}


RBaseStream::RBaseStream()
{
    m_start = m_end = m_current = 0;
    m_file = 0;
    m_block_size = BS_DEF_BLOCK_SIZE;
    m_ungetsize = 4; // 32 bits
    m_is_opened = false;
}


RBaseStream::~RBaseStream()
{
    close();    // Close files
    release();  // free  buffers
}


void  RBaseStream::readBlock()
{
    size_t readed;
    assert( m_file != 0 );

    // copy unget buffer
    if( m_start )
        memcpy( m_start - m_ungetsize, m_end - m_ungetsize, m_ungetsize );

    setPos( getPos() ); // normalize position

    fseek( m_file, m_block_pos, SEEK_SET );
    readed = fread( m_start, 1, m_block_size, m_file );
    m_end = m_start + readed;
    m_current   -= m_block_size;
    m_block_pos += m_block_size;

    if( readed == 0 || m_current >= m_end )
        throw RBS_THROW_EOS;
}


bool  RBaseStream::open( const String& filename )
{
    close();
    allocate();
    
    m_file = fopen( filename.c_str(), "rb" );
    
    if( m_file )
    {
        m_is_opened = true;
        setPos(0);
    }
    return m_file != 0;
}

void  RBaseStream::close()
{
    if( m_file )
    {
        fclose( m_file );
        m_file = 0;
    }
    m_is_opened = false;
}


void  RBaseStream::release()
{
    if( m_start )
    {
        delete[] (m_start - m_ungetsize);
    }
    m_start = m_end = m_current = 0;
}


void  RBaseStream::setBlockSize( int block_size, int unGetsize )
{
    assert( unGetsize >= 0 && block_size > 0 &&
           (block_size & (block_size-1)) == 0 );

    if( m_start && block_size == m_block_size && unGetsize == m_ungetsize ) return;
    release();
    m_block_size = block_size;
    m_ungetsize = unGetsize;
    allocate();
}


void  RBaseStream::setPos( int pos )
{
    int offset = pos & (m_block_size - 1);
    int block_pos = pos - offset;
    
    assert( isOpened() && pos >= 0 );
    
    if( m_current < m_end && block_pos == m_block_pos - m_block_size )
    {
        m_current = m_start + offset;
    }
    else
    {
        m_block_pos = block_pos;
        m_current = m_start + m_block_size + offset;
    }
}


int  RBaseStream::getPos()
{
    assert( isOpened() );
    return m_block_pos - m_block_size + (int)(m_current - m_start);
}

void  RBaseStream::skip( int bytes )
{
    assert( bytes >= 0 );
    m_current += bytes;
}

/////////////////////////  RLByteStream ////////////////////////////

RLByteStream::~RLByteStream()
{
}

int  RLByteStream::getByte()
{
    uchar *current = m_current;
    int   val;

    if( current >= m_end )
    {
        readBlock();
        current = m_current;
    }

    val = *((uchar*)current);
    m_current = current + 1;
    return val;
}


void  RLByteStream::getBytes( void* buffer, int count, int* readed )
{
    uchar*  data = (uchar*)buffer;
    assert( count >= 0 );
    
    if( readed) *readed = 0;

    while( count > 0 )
    {
        int l;

        for(;;)
        {
            l = (int)(m_end - m_current);
            if( l > count ) l = count;
            if( l > 0 ) break;
            readBlock();
        }
        memcpy( data, m_current, l );
        m_current += l;
        data += l;
        count -= l;
        if( readed ) *readed += l;
    }
}


////////////  RLByteStream & RMByteStream <Get[d]word>s ////////////////

RMByteStream::~RMByteStream()
{
}


int  RLByteStream::getWord()
{
    uchar *current = m_current;
    int   val;

    if( current+1 < m_end )
    {
        val = current[0] + (current[1] << 8);
        m_current = current + 2;
    }
    else
    {
        val = getByte();
        val|= getByte() << 8;
    }
    return val;
}


int  RLByteStream::getDWord()
{
    uchar *current = m_current;
    int   val;

    if( current+3 < m_end )
    {
        val = current[0] + (current[1] << 8) +
              (current[2] << 16) + (current[3] << 24);
        m_current = current + 4;
    }
    else
    {
        val = getByte();
        val |= getByte() << 8;
        val |= getByte() << 16;
        val |= getByte() << 24;
    }
    return val;
}


int  RMByteStream::getWord()
{
    uchar *current = m_current;
    int   val;

    if( current+1 < m_end )
    {
        val = (current[0] << 8) + current[1];
        m_current = current + 2;
    }
    else
    {
        val = getByte() << 8;
        val|= getByte();
    }
    return val;
}


int  RMByteStream::getDWord()
{
    uchar *current = m_current;
    int   val;

    if( current+3 < m_end )
    {
        val = (current[0] << 24) + (current[1] << 16) +
              (current[2] << 8) + current[3];
        m_current = current + 4;
    }
    else
    {
        val = getByte() << 24;
        val |= getByte() << 16;
        val |= getByte() << 8;
        val |= getByte();
    }
    return val;
}

/////////////////////////// WBaseStream /////////////////////////////////

// WBaseStream - base class for output streams
WBaseStream::WBaseStream()
{
    m_start = m_end = m_current = 0;
    m_file = 0;
    m_block_size = BS_DEF_BLOCK_SIZE;
    m_is_opened = false;
}


WBaseStream::~WBaseStream()
{
    close();
    release();
}


bool  WBaseStream::isOpened()
{ 
    return m_is_opened;
}


void  WBaseStream::allocate()
{
    if( !m_start )
        m_start = new uchar[m_block_size];

    m_end = m_start + m_block_size;
    m_current = m_start;
}


void  WBaseStream::writeBlock()
{
    int size = (int)(m_current - m_start);
    assert( m_file != 0 );

    //fseek( m_file, m_block_pos, SEEK_SET );
    fwrite( m_start, 1, size, m_file );
    m_current = m_start;

    /*if( written < size ) throw RBS_THROW_EOS;*/
    
    m_block_pos += size;
}


bool  WBaseStream::open( const String& filename )
{
    close();
    allocate();
    
    m_file = fopen( filename.c_str(), "wb" );
    
    if( m_file )
    {
        m_is_opened = true;
        m_block_pos = 0;
        m_current = m_start;
    }
    return m_file != 0;
}


void  WBaseStream::close()
{
    if( m_file )
    {
        writeBlock();
        fclose( m_file );
        m_file = 0;
    }
    m_is_opened = false;
}


void  WBaseStream::release()
{
    if( m_start )
    {
        delete[] m_start;
    }
    m_start = m_end = m_current = 0;
}


void  WBaseStream::setBlockSize( int block_size )
{
    assert( block_size > 0 && (block_size & (block_size-1)) == 0 );

    if( m_start && block_size == m_block_size ) return;
    release();
    m_block_size = block_size;
    allocate();
}


int  WBaseStream::getPos()
{
    assert( isOpened() );
    return m_block_pos + (int)(m_current - m_start);
}


///////////////////////////// WLByteStream /////////////////////////////////// 

WLByteStream::~WLByteStream()
{
}

void WLByteStream::putByte( int val )
{
    *m_current++ = (uchar)val;
    if( m_current >= m_end )
        writeBlock();
}


void WLByteStream::putBytes( const void* buffer, int count )
{
    uchar* data = (uchar*)buffer;
    
    assert( data && m_current && count >= 0 );

    while( count )
    {
        int l = (int)(m_end - m_current);
        
        if( l > count )
            l = count;
        
        if( l > 0 )
        {
            memcpy( m_current, data, l );
            m_current += l;
            data += l;
            count -= l;
        }
        if( m_current == m_end )
            writeBlock();
    }
}


void WLByteStream::putWord( int val )
{
    uchar *current = m_current;

    if( current+1 < m_end )
    {
        current[0] = (uchar)val;
        current[1] = (uchar)(val >> 8);
        m_current = current + 2;
        if( m_current == m_end )
            writeBlock();
    }
    else
    {
        putByte(val);
        putByte(val >> 8);
    }
}


void WLByteStream::putDWord( int val )
{
    uchar *current = m_current;

    if( current+3 < m_end )
    {
        current[0] = (uchar)val;
        current[1] = (uchar)(val >> 8);
        current[2] = (uchar)(val >> 16);
        current[3] = (uchar)(val >> 24);
        m_current = current + 4;
        if( m_current == m_end )
            writeBlock();
    }
    else
    {
        putByte(val);
        putByte(val >> 8);
        putByte(val >> 16);
        putByte(val >> 24);
    }
}


///////////////////////////// WMByteStream /////////////////////////////////// 

WMByteStream::~WMByteStream()
{
}


void WMByteStream::putWord( int val )
{
    uchar *current = m_current;

    if( current+1 < m_end )
    {
        current[0] = (uchar)(val >> 8);
        current[1] = (uchar)val;
        m_current = current + 2;
        if( m_current == m_end )
            writeBlock();
    }
    else
    {
        putByte(val >> 8);
        putByte(val);
    }
}


void WMByteStream::putDWord( int val )
{
    uchar *current = m_current;

    if( current+3 < m_end )
    {
        current[0] = (uchar)(val >> 24);
        current[1] = (uchar)(val >> 16);
        current[2] = (uchar)(val >> 8);
        current[3] = (uchar)val;
        m_current = current + 4;
        if( m_current == m_end )
            writeBlock();
    }
    else
    {
        putByte(val >> 24);
        putByte(val >> 16);
        putByte(val >> 8);
        putByte(val);
    }
}

}
