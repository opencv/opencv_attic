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

//
//  Loading and saving IPL images.
//

#include "_highgui.h"
#include "grfmts.h"

/****************************************************************************************\
*                              Path class (list of search folders)                       *
\****************************************************************************************/

class  CvvPath
{
public:
    CvvPath();
    ~CvvPath();

    // preprocess folder or file name - calculate its length,
    // check for invalid symbols in the name and substitute
    // all backslashes with simple slashes.
    // the result is put into the specified buffer
    static int Preprocess( const char* filename, char* buffer );
    
    // add folder to the path
    bool Add( const char* path );

    // clear the path
    void Clear();

    // return the path - string, where folders are separated by ';'
    const char* Get() const { return m_path; };

    // find the file in the path
    const char* Find( const char* filename, char* buffer ) const;

    // return the first folder from the path
    // the returned string is not terminated by '\0'!!!
    // its length is returned via len parameter
    const char* First( int& len ) const;

    // return the folder, next in the path after the specified folder.
    // see also note to First() method
    const char* Next( const char* folder, int& len ) const;

protected:

    char* m_path;
    int m_maxsize;
    int m_len;
};


void CvvPath::Clear()
{
    delete m_path;
    m_maxsize = m_len = 0;
}


CvvPath::CvvPath()
{
    m_path = 0;
    m_maxsize = m_len = 0;
}


CvvPath::~CvvPath()
{
    Clear();
}


bool  CvvPath::Add( const char* path )
{
    char buffer[_MAX_PATH + 1];
    int len = Preprocess( path, buffer );

    if( len < 0 )
        return false;

    if( m_len + len + 3 // +1 for one more ';',
                        // another +1 for possible additional '/',
                        // and the last +1 is for '\0'
                      > m_maxsize )
    {
        int new_size = (m_len + len + 3 + 1023) & -1024;
        char* new_path = new char[new_size];
        
        if( m_path )
        {
            memcpy( new_path, m_path, m_len );
            delete m_path;
        }

        m_path = new_path;
        m_maxsize = new_size;
    }

    m_path[m_len++] = ';';
    memcpy( m_path + m_len, buffer, len );
    m_len += len;

    if( m_path[m_len] != '/' )
        m_path[m_len++] = '/';

    m_path[m_len] = '\0'; // '\0' is not counted in m_len.

    return true;
}


const char* CvvPath::First( int& len ) const
{
    const char* path = (const char*)(m_path ? m_path : "");
    const char* path_end = path;

    while( *path_end && *path_end != ';' )
        path_end++;

    len = path_end - path;
    return path;
}


const char* CvvPath::Next( const char* folder, int& len ) const
{
    if( !folder || folder < m_path || folder >= m_path + m_len )
        return 0;
    
    folder = strchr( folder, ';' );
    if( folder )
    {
        const char* folder_end = ++folder;
        while( *folder_end && *folder_end != ';' )
            folder_end++;

        len = folder_end - folder;
    }

    return folder;
}


const char* CvvPath::Find( const char* filename, char* buffer ) const
{
    char path0[_MAX_PATH + 1];
    int len = Preprocess( filename, path0 );
    int folder_len = 0;
    const char* folder = First( folder_len );
    char* name_only = 0;
    char* name = path0;
    FILE* f = 0;

    if( len < 0 )
        return 0;

    do
    {
        if( folder_len + len <= _MAX_PATH )
        {
            memcpy( buffer, folder, folder_len );
            strcpy( buffer + folder_len, name );
        
            f = fopen( buffer, "rb" );
            if( f )
                break;
        }

        if( name != name_only )
        {
            name_only = strrchr( path0, '/' );
            if( !name_only )
                name_only = path0;
            else
                name_only++;
            len = strlen( name_only );
            name = name_only;
        }
    }
    while( (folder = Next( folder, folder_len )) != 0 );

    filename = 0;

    if( f )
    {
        filename = (const char*)buffer;
        fclose(f);
    }

    return filename;
}


int CvvPath::Preprocess( const char* str, char* buffer )
{
    int i;

    if( !str || !buffer )
        return -1;

    for( i = 0; i <= _MAX_PATH; i++ )
    {
        buffer[i] = str[i];
        
        if( isalnum(str[i])) // fast check to skip most of characters
            continue;
        
        if( str[i] == '\0' )
            break;

        if( str[i] == '\\' ) // convert back slashes to simple slashes
                             // (for Win32-*NIX compatibility)
            buffer[i] = '/';

        if( str[i] == '*' || str[i] == ';' || str[i] == ',' || str[i] == '%' ||
            str[i] == '?' || str[i] == '\"' || str[i] == '>' || str[i] == '<' ||
            str[i] == '|' )
            return -1;
    }

    return i <= _MAX_PATH ? i : -1;
}



/****************************************************************************************\
*                              Image Readers & Writers Class                             *
\****************************************************************************************/

class  CvvImageFilters
{
public:

    CvvImageFilters();
    ~CvvImageFilters();

    GrFmtReader* FindReader( const char* filename ) const;
    GrFmtWriter* FindWriter( const char* filename ) const;
    
    bool AddPath( const char* path ) { return m_path.Add( path ); };
    const CvvPath& Path() const { return (const CvvPath&)m_path; };
    CvvPath& Path() { return m_path; };

protected:

    GrFmtFactoriesList*  m_factories;
    CvvPath m_path;
};


CvvImageFilters::CvvImageFilters()
{
    m_factories = new GrFmtFactoriesList;

    m_factories->AddFactory( new GrFmtBmp() );
    m_factories->AddFactory( new GrFmtJpeg() );
    m_factories->AddFactory( new GrFmtSunRaster() );
    m_factories->AddFactory( new GrFmtPxM() );
    m_factories->AddFactory( new GrFmtTiff() );
#ifdef HAVE_PNG
    m_factories->AddFactory( new GrFmtPng() );
#endif
}


CvvImageFilters::~CvvImageFilters()
{
    delete m_factories;
}


GrFmtReader* CvvImageFilters::FindReader( const char* filename ) const
{
    char buffer[_MAX_PATH + 1];
    GrFmtReader* reader = 0;

    filename = m_path.Find( filename, buffer );
    reader = filename ? m_factories->FindReader( filename ) : 0;

    return reader;
}


GrFmtWriter* CvvImageFilters::FindWriter( const char* filename ) const
{
    char buffer[_MAX_PATH + 1];
    GrFmtWriter* writer = CvvPath::Preprocess( filename, buffer ) ?
                            m_factories->FindWriter( buffer ) : 0;
    return writer;
}

/****************************************************************************************\
*                         HighGUI loading & saving function implementation               *
\****************************************************************************************/

// global image I/O filters
CvvImageFilters  g_Filters;

HIGHGUI_IMPL void
cvAddSearchPath( const char* path )
{
    CV_FUNCNAME( "cvAddSearchPath" );

    __BEGIN__;

    if( !path || strlen(path) == 0 )
        CV_ERROR( CV_StsNullPtr, "Null path" );
    
    g_Filters.AddPath( path );

    __END__;
}


HIGHGUI_IMPL IplImage*
cvLoadImage( const char* filename, int iscolor )
{
    GrFmtReader* reader = 0;
    IplImage* image = 0;

    CV_FUNCNAME( "cvLoadImage" );

    __BEGIN__;

    if( !filename || strlen(filename) == 0 )
        CV_ERROR( CV_StsNullPtr, "null filename" );

    reader = g_Filters.FindReader( filename );
    if( !reader )
        EXIT;

    if( !reader->ReadHeader() )
        EXIT;

    {
        CvSize size;
        size.width = reader->GetWidth();
        size.height = reader->GetHeight();

        iscolor = iscolor > 0 || (iscolor < 0 && reader->IsColor());

        CV_CALL( image = cvCreateImage( size, IPL_DEPTH_8U, iscolor ? 3 : 1 ));
        
        if( !reader->ReadData( (unsigned char*)(image->imageData),
                               image->widthStep, iscolor ))
        {
            cvReleaseImage( &image );
            EXIT;
        }
    }
    
    __END__;

    delete reader;

    if( cvGetErrStatus() < 0 )
        cvReleaseImage( &image );

    return image;
}


HIGHGUI_IMPL int
cvSaveImage( const char* filename, const CvArr* arr )
{
    int origin = 0;
    int did_flip = 0;
    GrFmtWriter* writer = 0;
    
    CV_FUNCNAME( "cvSaveImage" );

    __BEGIN__;
    
    CvMat stub, *image;
    int channels;
    
    if( !filename || strlen(filename) == 0 )
        CV_ERROR( CV_StsNullPtr, "null filename" );

    CV_CALL( image = cvGetMat( arr, &stub ));

    if( CV_IS_IMAGE( arr ))
        origin = ((IplImage*)arr)->origin;

    channels = CV_MAT_CN( image->type );
    if( channels != 1 && channels != 3 )
        CV_ERROR( CV_BadNumChannels, "" );

    writer = g_Filters.FindWriter( filename );
    if( !writer )
        CV_ERROR( CV_StsError, "could not find a filter for the specified extension" );

    if( origin )
    {
        CV_CALL( cvFlip( image, image, 0 ));
        did_flip = 1;
    }

    if( !writer->WriteImage( image->data.ptr, image->step, image->width,
                             image->height, channels > 1 ))
        CV_ERROR( CV_StsError, "could not save the image" );

    __END__;

    delete writer;

    if( origin && did_flip )
        cvFlip( arr, (void*)arr, 0 );

    return cvGetErrStatus() >= 0;
}


HIGHGUI_IMPL void
cvConvertImage( const CvArr* srcarr, CvArr* dstarr, int flip )
{
    CvMat* temp = 0;
    
    CV_FUNCNAME( "cvConvertImage" );
    
    __BEGIN__;

    CvMat srcstub, *src;
    CvMat dststub, *dst;
    int src_cn;

    CV_CALL( src = cvGetMat( srcarr, &srcstub ));
    CV_CALL( dst = cvGetMat( dstarr, &dststub ));

    src_cn = CV_MAT_CN( src->type );

    if( src_cn != 1 && src_cn != 3 && src_cn != 4 )
        CV_ERROR( CV_BadNumChannels, "Source image must have 1, 3 or 4 channels" );

    if( CV_MAT_DEPTH( dst->type ) != CV_8U )
        CV_ERROR( CV_BadDepth, "Destination image must be 8u" );
    
    if( !CV_ARE_DEPTHS_EQ( src, dst ))
    {
        temp = cvCreateMat( src->height, src->width,
                            (src->type & CV_MAT_CN_MASK)|(dst->type & CV_MAT_DEPTH_MASK));
        cvScale( src, temp, CV_MAT_DEPTH(src->type) >= CV_32F ? 255 : 1, 0 );
        src = temp;
    }

    if( !CV_ARE_CNS_EQ( src, dst ))
    {
        static const int cvt_table[] = {
            /*    \ dst 
              src  \    */
            0, CV_GRAY2BGR565, CV_GRAY2BGR, CV_GRAY2BGRA,
            0, 0, 0, 0,
            CV_BGR2GRAY, CV_BGR2BGR565, 0, CV_BGR2BGRA,
            CV_BGRA2GRAY, CV_BGRA2BGR565, CV_BGRA2BGR, 0
        };

        CV_CALL( cvCvtColor( src, dst, cvt_table[(src_cn-1)*4 + CV_MAT_CN(dst->type) - 1] ));

        if( flip )
        {
            CV_CALL( cvFlip( dst, dst, 0 ));
        }
    }
    else if( flip )
    {
        CV_CALL( cvFlip( src, dst, 0 ));
    }
    else
    {
        CV_CALL( cvCopy( src, dst ));
    }

    __END__;

    cvReleaseMat( &temp );
}

/* End of file. */
