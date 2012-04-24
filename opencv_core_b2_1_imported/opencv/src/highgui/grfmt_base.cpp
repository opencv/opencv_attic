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

#include "grfmt_base.h"
#include "bitstrm.h"

///////////////////// GrFmtFilter //////////////////////////

GrFmtFilter::GrFmtFilter() : m_description(0)
{
}

GrFmtFilter::~GrFmtFilter()
{
}

bool  GrFmtFilter::SetFile( const char* filename )
{
    int length;
    m_filename[0] = '\0';

    if( !filename )
        return false;

    length = strlen( filename );
    if( (unsigned)length >= sizeof(m_filename))
        return false;

    memcpy( m_filename, filename, length + 1 );

    return true;
}

///////////////////// GrFmtReader //////////////////////////

GrFmtReader::GrFmtReader()
{
    m_width = m_height = -1;
    m_sign_len    = 0;
    m_signature   = 0;
    m_filename[0] ='\0';
}


GrFmtReader::~GrFmtReader()
{
}


bool  GrFmtReader::CheckFormat( const char* signature )
{
    assert( signature != 0 );
    return memcmp( m_signature, signature, m_sign_len ) == 0;
}


///////////////////// GrFmtWriter //////////////////////////

GrFmtWriter::GrFmtWriter()
{
    m_parameter = 0;
}


GrFmtWriter::~GrFmtWriter()
{
}


bool GrFmtWriter::SetParameter( long parameter )
{
    m_parameter = parameter;
    return true;
}


static char* ExtractExtension( const char** str, char* buffer, int maxLen )
{
    const char* ext = strchr(*str, '.');
    ext = ext ? ext + 1 : *str;

    int len = 0;
    while( isalnum(ext[len]) && len < maxLen )
        buffer[len++] = (char)toupper(ext[len]);

    if( len >= maxLen )
        return 0;

    if( len != 0 )
    {
        buffer[len] = '\0';
        *str = ext + len;
    }
    else
    {
        buffer = 0;
        *str = 0;
    }

    return buffer;
}

bool  GrFmtWriter::CheckFormat( const char* format )
{
    const int MAX_EXT_LEN = 256;
    char formatBuf[MAX_EXT_LEN];
    char descrBuf[MAX_EXT_LEN];
    const char* formatExt = format;
    const char* descr = 0;

    if( !format || !m_description ) return false;

    // find the right-most extension of the passed format string
    for(;;)
    {
        const char* ext = strchr( formatExt, '.' );
        if( !ext ) break;
        formatExt = ext + 1;
    }

    if( !ExtractExtension( &formatExt, formatBuf, MAX_EXT_LEN ))
        return false;

    descr = strchr( m_description, '(' );

    while( descr )
    {
        if( ExtractExtension( &descr, descrBuf, MAX_EXT_LEN ) &&
            strcmp( descrBuf, formatBuf ) == 0 )
            return true;
    }

    return false;
}

///////////////////// GrFmtFilterList //////////////////////////

GrFmtFiltersList::GrFmtFiltersList()
{
    m_filters = 0;
    RemoveAll();
}


GrFmtFiltersList::~GrFmtFiltersList()
{
    RemoveAll();
}


void  GrFmtFiltersList::RemoveAll()
{
    if( m_filters )
    {
        for( int i = 0; i < m_curFilters; i++ ) delete m_filters[i];
        delete m_filters;
    }
    m_filters = 0;
    m_maxFilters = m_curFilters = 0;
}


bool  GrFmtFiltersList::AddFilter( GrFmtFilter* filter )
{
    assert( filter != 0 );
    if( m_curFilters == m_maxFilters )
    {
        // reallocate the filters pointers storage
        int newMaxFilters = 2*m_maxFilters;
        if( newMaxFilters < 16 ) newMaxFilters = 16;

        GrFmtFilter** newFilters = new GrFmtFilter*[newMaxFilters];

        for( int i = 0; i < m_curFilters; i++ ) newFilters[i] = m_filters[i];

        delete m_filters;
        m_filters = newFilters;
        m_maxFilters = newMaxFilters;
    }

    if( m_curFilters > 0 && filter->GetFilterType() != m_filters[0]->GetFilterType())
        return false;

    m_filters[m_curFilters++] = filter;
    return true;
}


ListPosition  GrFmtFiltersList::GetFirstFilterPos()
{
    return (ListPosition)m_filters;
}


GrFmtFilter* GrFmtFiltersList::GetNextFilter( ListPosition& pos )
{
    GrFmtFilter* filter = 0;
    GrFmtFilter** temp = (GrFmtFilter**)pos;

    assert( temp == 0 || (m_filters <= temp && temp < m_filters + m_curFilters));
    if( temp )
    {
        filter = *temp++;
        pos = (ListPosition)(temp < m_filters + m_curFilters ? temp : 0);
    }
    return filter;
}


int  GrFmtFiltersList::GetFiltersString( char* /*buffer*/, int /*maxlen*/ )
{
    return 0;
}


///////////////////// GrFmtReadersList //////////////////////////

GrFmtReadersList::GrFmtReadersList()
{
}


GrFmtReadersList::~GrFmtReadersList()
{
}


GrFmtFilter* GrFmtReadersList::FindFilter( const char* filename )
{
    int    i;
    FILE*  f = 0;
    char   signature[4096];
    int    sign_len = 0;
    GrFmtFilter* filter = 0;

    if( !filename ) return 0;

    for( i = 0; i < m_curFilters; i++ )
    {
        GrFmtReader* tempFilter = (GrFmtReader*)m_filters[i];
        int temp = tempFilter->GetSignatureLength();
        if( temp > sign_len ) sign_len = temp;
    }

    assert( sign_len <= sizeof(signature) );

    f = fopen( filename, "rb" );

    if( f )
    {
        sign_len = fread( signature, 1, sign_len, f );
        fclose(f);

        for( i = 0; i < m_curFilters; i++ )
        {
            GrFmtReader* tempFilter = (GrFmtReader*)m_filters[i];
            int temp = tempFilter->GetSignatureLength();
            if( temp <= sign_len && tempFilter->CheckFormat(signature))
            {
                filter = tempFilter;
                break;
            }
        }
    }

    return filter;
}


///////////////////// GrFmtWritersList //////////////////////////

GrFmtWritersList::GrFmtWritersList()
{
}


GrFmtWritersList::~GrFmtWritersList()
{
}


GrFmtFilter* GrFmtWritersList::FindFilter( const char* format )
{
    int    i;
    GrFmtFilter* filter = 0;

    if( !format ) return 0;

    for( i = 0; i < m_curFilters; i++ )
    {
        GrFmtWriter* tempFilter = (GrFmtWriter*)m_filters[i];
        if( tempFilter->CheckFormat( format ))
        {
            filter = tempFilter;
            break;
        }
    }

    return filter;
}

