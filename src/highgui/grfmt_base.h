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

#ifndef _GRFMT_BASE_H_
#define _GRFMT_BASE_H_

#if _MSC_VER >= 1200
    #pragma warning( disable: 4514 )
    #pragma warning( disable: 4711 )
#endif

#include "utils.h"
#include "bitstrm.h"

#define  RBS_BAD_HEADER   -125  /* invalid image header */
#define  BAD_HEADER_ERR()     throw RBS_BAD_HEADER

#ifndef _MAX_PATH
    #define _MAX_PATH    1024
#endif


///////////////////////////// type of graphic format filter //////////////////////////////
enum GrFmtFilterType
{
    GR_FMT_READER = 0,
    GR_FMT_WRITER = 1
};


//////////////////////////// base graphic format filter class ////////////////////////////
class   GrFmtFilter
{
public:

    GrFmtFilter();
    virtual ~GrFmtFilter();

    const char*  GetDescription() { return m_description; };
    virtual bool CheckFormat( const char* ) = 0;
    virtual GrFmtFilterType GetFilterType() = 0;
    virtual bool  SetFile( const char* filename );

protected:
    const char* m_description;
           // graphic format description in form:
           // <Some textual description>( *.<extension1> [; *.<extension2> ...]).
           // the textual description can not contain symbols '(', ')'
           // and may be, some others. It is safe to use letters, digits and spaces only.
           // e.g. "Targa (*.tga)",
           // or "Portable Graphic Format (*.pbm;*.pgm;*.ppm)"

    char m_filename[_MAX_PATH];
};


/////////////////////////// base graphic format reader class /////////////////////////////
class   GrFmtReader : public GrFmtFilter
{
public:
    
    GrFmtReader();
    ~GrFmtReader();

    GrFmtFilterType GetFilterType() { return GR_FMT_READER; };

    int           GetSignatureLength() { return m_sign_len; };
    int           GetWidth()  { return m_width; };
    int           GetHeight() { return m_height; };
    bool          IsColor()   { return m_iscolor; };
    
    virtual bool  CheckFormat( const char* signature );
    virtual bool  ReadHeader() = 0;
    virtual bool  ReadData( uchar* data, int step, int color ) = 0;
    virtual void  Close() = 0;

protected:
    
    bool         m_iscolor;     // 1 means color, 0 means grayscale
    int          m_width;       // width  of an image ( filled by ReadHeader )
    int          m_height;      // height of an image ( filled by ReadHeader )
    int          m_sign_len;    // length of the signature of the format
    const char*  m_signature;   // signature of the format
};


/////////////////////////// base graphic format writer class /////////////////////////////
class   GrFmtWriter : public GrFmtFilter
{
public:

    GrFmtWriter();
    ~GrFmtWriter();

    GrFmtFilterType GetFilterType() { return GR_FMT_WRITER; };

    virtual bool  CheckFormat( const char* format );
    virtual bool  SetParameter( long parameter );
    long          GetParameter() { return m_parameter; }
    virtual bool  WriteImage( const uchar* data, int step,
                              int width, int height, bool isColor ) = 0;
protected:
    
    long         m_parameter;   // parameter for writing process customization
};


/////////////////////////// list of graphic format filters ///////////////////////////////

typedef struct _ListPosition { int stub; }* ListPosition;

class   GrFmtFiltersList
{
public:

    GrFmtFiltersList();
    virtual ~GrFmtFiltersList();
    void  RemoveAll();
    bool  AddFilter( GrFmtFilter* filter );
    int  FiltersCount() { return m_curFilters; };
    ListPosition  GetFirstFilterPos();
    GrFmtFilter*  GetNextFilter( ListPosition& pos );
    virtual GrFmtFilter*  FindFilter( const char* filename ) = 0;
    int  GetFiltersString( char* buffer, int maxlen );

protected:

    GrFmtFilter** m_filters;
    int  m_maxFilters;
    int  m_curFilters;
};


/////////////////////////// list of graphic format readers ///////////////////////////////
class   GrFmtReadersList : public GrFmtFiltersList
{
public:

    GrFmtReadersList();
    virtual ~GrFmtReadersList();

    GrFmtFilter* FindFilter( const char* filename );
};


/////////////////////////// list of graphic format writers ///////////////////////////////
class   GrFmtWritersList : public GrFmtFiltersList
{
public:

    GrFmtWritersList();
    virtual ~GrFmtWritersList();

    GrFmtFilter* FindFilter( const char* filename );
};

#endif/*_GRFMT_BASE_H_*/
