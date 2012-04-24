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

#ifndef _UTILS_H_
#define _UTILS_H_

typedef unsigned char uchar;

struct PaletteEntry 
{
    unsigned char b, g, r, a;
};

#define WRITE_PIX( ptr, clr )     \
    (((uchar*)(ptr))[0] = (clr).b, \
     ((uchar*)(ptr))[1] = (clr).g, \
     ((uchar*)(ptr))[2] = (clr).r)

#define SWAP(a,b,t) ((t)=(a),(a)=(b),(b)=(t))

#define  descale(x,n)  (((x) + (1 << ((n)-1))) >> (n))
#define  saturate(x)   (uchar)(((x) & ~255) == 0 ? (x) : ~((x)>>31))

void  CvtBGRToGray( const uchar* bgr, uchar* gray, int len );
void  CvtGrayToBGR( const uchar* gray, uchar* bgr, int len );
void  CvtRGBToGray( const uchar* rgb, uchar* gray, int len );
void  CvtRGBToBGR( const uchar* rgb, uchar* bgr, int len );
void  CvtBGRAToBGR( const uchar* bgra, uchar* bgr, int len );
void  CvtRGBAToBGR( const uchar* rgba, uchar* bgr, int len );
void  CvtBGRAToGray( const uchar* bgr, uchar* gray, int len );
void  CvtRGBAToGray( const uchar* bgr, uchar* gray, int len );

void  FillGrayPalette( PaletteEntry* palette, int bpp, bool negative = false );
bool  IsColorPalette( PaletteEntry* palette, int bpp );

void  CvtPaletteToGray( const PaletteEntry* palette, uchar* grayPalette, int entries );

uchar* FillUniColor( uchar* data, uchar*& line_end, int step, int width3,
                     int& y, int height, int count3, PaletteEntry clr );

uchar* FillUniGray( uchar* data, uchar*& line_end, int step, int width3,
                     int& y, int height, int count3, uchar clr );

uchar* FillColorRow8( uchar* data, uchar* indices, int len, PaletteEntry* palette );

uchar* FillGrayRow8( uchar* data, uchar* indices, int len, uchar* palette );

uchar* FillColorRow4( uchar* data, uchar* indices, int len, PaletteEntry* palette );

uchar* FillGrayRow4( uchar* data, uchar* indices, int len, uchar* palette );

uchar* FillColorRow1( uchar* data, uchar* indices, int len, PaletteEntry* palette );

uchar* FillGrayRow1( uchar* data, uchar* indices, int len, uchar* palette );

inline int round( double x )
{
    static unsigned delta = 0x59c00000;
    x = (x + 1e-7) + (float&)delta;
    return (int&)x;
}

inline int myfloor( double x )
{
    static unsigned delta = 0x59c00000;
    x = (x - 0.4999999) + (float&)delta;
    return (int&)x;
}


inline int myceil( double x )
{
    static unsigned delta = 0x59c00000;
    x = (x + 0.4999999) + (float&)delta;
    return (int&)x;
}


#endif/*_UTILS_H_*/
