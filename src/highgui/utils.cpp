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
#include "utils.h"

#define  SCALE  14
#define  cR  (int)(0.299*(1 << SCALE) + 0.5)
#define  cG  (int)(0.587*(1 << SCALE) + 0.5)
#define  cB  ((1 << SCALE) - cR - cG)


void CvtBGRToGray( const uchar* bgr, uchar* gray, int len )
{
    int i;
    for( i = 0; i < len; i++, bgr += 3 )
    {
        int t = descale( bgr[0]*cB + bgr[1]*cG + bgr[2]*cR, SCALE );
        assert( (unsigned)t <= 255 );
        gray[i] = (uchar)t;
    }
}


void CvtRGBToGray( const uchar* rgb, uchar* gray, int len )
{
    int i;
    for( i = 0; i < len; i++, rgb += 3 )
    {
        int t = descale( rgb[2]*cB + rgb[1]*cG + rgb[0]*cR, SCALE );
        assert( (unsigned)t <= 255 );
        gray[i] = (uchar)t;
    }
}


void CvtBGRAToGray( const uchar* bgra, uchar* gray, int len )
{
    int i;
    for( i = 0; i < len; i++, bgra += 4 )
    {
        int t = descale( bgra[0]*cB + bgra[1]*cG + bgra[2]*cR, SCALE );
        assert( (unsigned)t <= 255 );
        gray[i] = (uchar)t;
    }
}


void CvtRGBAToGray( const uchar* rgba, uchar* gray, int len )
{
    int i;
    for( i = 0; i < len; i++, rgba += 4 )
    {
        int t = descale( rgba[2]*cB + rgba[1]*cG + rgba[0]*cR, SCALE );
        assert( (unsigned)t <= 255 );
        gray[i] = (uchar)t;
    }
}


void CvtRGBToBGR( const uchar* rgb, uchar* bgr, int len )
{
    int i;

    for( i = 0; i < len; i++, rgb += 3, bgr += 3 )
    {
        uchar b = rgb[2], g = rgb[1], r = rgb[0];
        bgr[0] = b;
        bgr[1] = g;
        bgr[2] = r;
    }
}


void  CvtBGRAToBGR( const uchar* bgra, uchar* bgr, int len )
{
    for( int i = 0; i < len; i++, bgra += 4, bgr += 3 )
    {
        uchar b = bgra[0], g = bgra[1], r = bgra[2];
        bgr[0] = b;
        bgr[1] = g;
        bgr[2] = r;
    }
}


void  CvtRGBAToBGR( const uchar* rgba, uchar* bgr, int len )
{
    for( int i = 0; i < len; i++, rgba += 4, bgr += 3 )
    {
        uchar b = rgba[2], g = rgba[1], r = rgba[0];
        bgr[0] = b;
        bgr[1] = g;
        bgr[2] = r;
    }
}


void CvtPaletteToGray( const PaletteEntry* palette, uchar* grayPalette, int entries )
{
    int i;
    for( i = 0; i < entries; i++ )
    {
        CvtBGRToGray( (uchar*)(palette + i), grayPalette + i, 1 );
    }
}


void  FillGrayPalette( PaletteEntry* palette, int bpp, bool negative )
{
    int i, length = 1 << bpp;
    int xor_mask = negative ? 255 : 0;

    for( i = 0; i < length; i++ )
    {
        int val = (i * 255/(length - 1)) ^ xor_mask;
        palette[i].b = palette[i].g = palette[i].r = (uchar)val;
        palette[i].a = 0;
    }
}


bool  IsColorPalette( PaletteEntry* palette, int bpp )
{
    int i, length = 1 << bpp;

    for( i = 0; i < length; i++ )
    {
        if( palette[i].b != palette[i].g ||
            palette[i].b != palette[i].r )
            return true;
    }

    return false;
}


void CalcShifts( uchar* data, uchar* line_end, int width3,
                 int y, int height, int& x_shift3, int& y_shift )
{
    int x3 = data - (line_end - width3);
    int new_x3 = x3 + x_shift3;
    
    y_shift = new_x3 / width3;
    new_x3 -= y_shift * width3;

    if( new_x3 == 0 )
    {
        y_shift--;
        new_x3 = width3;
    }

    x_shift3 = new_x3 - x3;

    if( y + y_shift >= height )
    {
        if( y + y_shift > height )
            y_shift = height - y;
        if( width3 - (line_end - data) + x_shift3 > 0 )
            x_shift3 = (line_end - data) - width3;
    }
}


uchar* FillUniColor( uchar* data, uchar*& line_end,
                     int step, int width3,
                     int& y, int height,
                     int count3, PaletteEntry clr )
{
    do
    {
        uchar* end = data + count3;

        if( end > line_end )
            end = line_end;

        count3 -= end - data;
        
        for( ; data < end; data += 3 )
        {
            WRITE_PIX( data, clr );
        }

        if( data >= line_end )
        {
            line_end += step;
            data = line_end - width3;
            if( ++y >= height  ) break;
        }
    }
    while( count3 > 0 );

    return data;
}


uchar* FillUniGray( uchar* data, uchar*& line_end,
                    int step, int width,
                    int& y, int height,
                    int count, uchar clr )
{
    do
    {
        uchar* end = data + count;

        if( end > line_end )
            end = line_end;

        count -= end - data;
        
        for( ; data < end; data++ )
        {
            *data = clr;
        }

        if( data >= line_end )
        {
            line_end += step;
            data = line_end - width;
            if( ++y >= height  ) break;
        }
    }
    while( count > 0 );

    return data;
}


uchar* FillColorRow8( uchar* data, uchar* indices, int len, PaletteEntry* palette )
{
    uchar* end = data + len*3;
    while( (data += 3) < end )
    {
        *((PaletteEntry*)(data-3)) = palette[*indices++];
    }
    PaletteEntry clr = palette[indices[0]];
    WRITE_PIX( data - 3, clr );
    return data;
}
                       

uchar* FillGrayRow8( uchar* data, uchar* indices, int len, uchar* palette )
{
    int i;
    for( i = 0; i < len; i++ )
    {
        data[i] = palette[indices[i]];
    }
    return data + len;
}


uchar* FillColorRow4( uchar* data, uchar* indices, int len, PaletteEntry* palette )
{
    uchar* end = data + len*3;

    while( (data += 6) < end )
    {
        int idx = *indices++;
        *((PaletteEntry*)(data-6)) = palette[idx >> 4];
        *((PaletteEntry*)(data-3)) = palette[idx & 15];
    }

    int idx = indices[0];
    PaletteEntry clr = palette[idx >> 4];
    WRITE_PIX( data - 6, clr );

    if( data == end )
    {
        clr = palette[idx & 15];
        WRITE_PIX( data - 3, clr );
    }
    return end;
}


uchar* FillGrayRow4( uchar* data, uchar* indices, int len, uchar* palette )
{
    uchar* end = data + len;
    while( (data += 2) < end )
    {
        int idx = *indices++;
        data[-2] = palette[idx >> 4];
        data[-1] = palette[idx & 15];
    }

    int idx = indices[0];
    uchar clr = palette[idx >> 4];
    data[-2] = clr;

    if( data == end )
    {
        clr = palette[idx & 15];
        data[-1] = clr;
    }
    return end;
}


uchar* FillColorRow1( uchar* data, uchar* indices, int len, PaletteEntry* palette )
{
    uchar* end = data + len*3;

    while( (data += 24) < end )
    {
        int idx = *indices++;
        *((PaletteEntry*)(data - 24)) = palette[(idx & 128) != 0];
        *((PaletteEntry*)(data - 21)) = palette[(idx & 64) != 0];
        *((PaletteEntry*)(data - 18)) = palette[(idx & 32) != 0];
        *((PaletteEntry*)(data - 15)) = palette[(idx & 16) != 0];
        *((PaletteEntry*)(data - 12)) = palette[(idx & 8) != 0];
        *((PaletteEntry*)(data - 9)) = palette[(idx & 4) != 0];
        *((PaletteEntry*)(data - 6)) = palette[(idx & 2) != 0];
        *((PaletteEntry*)(data - 3)) = palette[(idx & 1) != 0];
    }
    
    int idx = indices[0] << 24;
    for( data -= 24; data < end; data += 3, idx += idx )
    {
        PaletteEntry clr = palette[idx < 0];
        WRITE_PIX( data, clr );
    }

    return data;
}


uchar* FillGrayRow1( uchar* data, uchar* indices, int len, uchar* palette )
{
    uchar* end = data + len;

    while( (data += 8) < end )
    {
        int idx = *indices++;
        *((uchar*)(data - 8)) = palette[(idx & 128) != 0];
        *((uchar*)(data - 7)) = palette[(idx & 64) != 0];
        *((uchar*)(data - 6)) = palette[(idx & 32) != 0];
        *((uchar*)(data - 5)) = palette[(idx & 16) != 0];
        *((uchar*)(data - 4)) = palette[(idx & 8) != 0];
        *((uchar*)(data - 3)) = palette[(idx & 4) != 0];
        *((uchar*)(data - 2)) = palette[(idx & 2) != 0];
        *((uchar*)(data - 1)) = palette[(idx & 1) != 0];
    }
    
    int idx = indices[0] << 24;
    for( data -= 8; data < end; data++, idx += idx )
    {
        data[0] = palette[idx < 0];
    }

    return data;
}

