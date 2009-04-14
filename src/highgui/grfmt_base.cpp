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

namespace cv
{

BaseImageDecoder::BaseImageDecoder()
{
    m_width = m_height = 0;
    m_type = -1;
}

void BaseImageDecoder::setSource( const String& filename )
{
    m_filename = filename;
    m_buf.release();
}

void BaseImageDecoder::setSource( const Vector<uchar>& buf )
{
    m_filename = String();
    m_buf = buf;
}

size_t BaseImageDecoder::signatureLength() const
{
    return m_signature.size();
}

bool BaseImageDecoder::checkSignature( const String& signature ) const
{
    size_t len = signatureLength();
    return signature.size() >= len && memcmp( signature.c_str(), m_signature.c_str(), len ) == 0;
}

ImageDecoder BaseImageDecoder::newDecoder() const
{
    return ImageDecoder();
}

bool  BaseImageEncoder::isFormatSupported( int depth ) const
{
    return depth == CV_8U;
}

String BaseImageEncoder::getDescription() const
{
    return m_description;
}

ImageEncoder BaseImageEncoder::newEncoder() const
{
    return ImageEncoder();
}

bool BaseImageEncoder::encode( const Mat& img, Vector<uchar>& buf, const Vector<int>& params )
{
    char fnamebuf[L_tmpnam];
    const char* filename = tmpnam(fnamebuf);

    if( !write( filename, img, params ))
        return false;
    FILE* f = fopen( filename, "rb" );
    CV_Assert(f != 0);
    fseek( f, 0, SEEK_END );
    long pos = ftell(f);
    buf.resize((size_t)pos);
    buf.resize(fread( &buf[0], 1, buf.size(), f ));
    fclose(f);
    unlink(filename);

    return true;
}

}

/* End of file. */
