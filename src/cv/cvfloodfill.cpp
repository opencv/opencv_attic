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

#include "_cv.h"

typedef struct CvFFillSegment
{
    ushort y;
    ushort l;
    ushort r;
    ushort prevl;
    ushort prevr;
    short dir;
}
CvFFillSegment;

#define UP 1
#define DOWN -1             

#define ICV_PUSH( Y, L, R, PREV_L, PREV_R, DIR )\
{                                               \
    tail->y = (ushort)(Y);                      \
    tail->l = (ushort)(L);                      \
    tail->r = (ushort)(R);                      \
    tail->prevl = (ushort)(PREV_L);             \
    tail->prevr = (ushort)(PREV_R);             \
    tail->dir = (short)(DIR);                   \
    if( ++tail >= buffer_end )                  \
        tail = buffer;                          \
}


#define ICV_POP( Y, L, R, PREV_L, PREV_R, DIR ) \
{                                               \
    Y = head->y;                                \
    L = head->l;                                \
    R = head->r;                                \
    PREV_L = head->prevl;                       \
    PREV_R = head->prevr;                       \
    DIR = head->dir;                            \
    if( ++head >= buffer_end )                  \
        head = buffer;                          \
}


/****************************************************************************************\
*                             Simple (monochrome) Floodfill                              *
\****************************************************************************************/

static CvStatus
icvFloodFill_8u_C1IR( uchar* pImage, int step, CvSize roi, CvPoint seed,
                      uchar* _newVal, CvConnectedComp* region, int flags,
                      CvFFillSegment* buffer, int buffersize )
{
    uchar* img = pImage + step * seed.y;
    int i, L, R; 
    int area = 0;
    int val0 = 0;
    uchar newVal = _newVal[0];
    int XMin, XMax, YMin = seed.y, YMax = seed.y;
    int _8_connectivity = (flags & 255) == 8;
    CvFFillSegment* buffer_end = buffer + buffersize, *head = buffer, *tail = buffer;

    L = R = seed.x;
    val0 = img[L];

    if( val0 == newVal )
    {
        XMin = XMax = seed.x;
        goto exit_func;
    }

    img[L] = newVal;

    while( ++R < roi.width && img[R] == val0 )
        img[R] = newVal;

    while( --L >= 0 && img[L] == val0 )
        img[L] = newVal;

    XMax = --R;
    XMin = ++L;
    ICV_PUSH( seed.y, L, R, R + 1, R, UP );

    while( head != tail )
    {
        int k, YC, PL, PR, dir, curstep;
        ICV_POP( YC, L, R, PL, PR, dir );

        int data[][3] =
        {
            {-dir, L - _8_connectivity, R + _8_connectivity},
            {dir, L - _8_connectivity, PL - 1},
            {dir, PR + 1, R + _8_connectivity}
        };

        if( region )
        {
            area += R - L + 1;

            if( XMax < R ) XMax = R;
            if( XMin > L ) XMin = L;
            if( YMax < YC ) YMax = YC;
            if( YMin > YC ) YMin = YC;
        }

        for( k = (unsigned)(YC - dir) >= (unsigned)roi.height; k < 3; k++ )
        {
            dir = data[k][0];
            curstep = dir * step;
            img = pImage + (YC + dir) * step;
            int left = data[k][1];
            int right = data[k][2];

            for( i = left; i <= right; i++ )
            {
                if( (unsigned)i < (unsigned)roi.width && img[i] == val0 )
                {
                    int j = i;
                    img[i] = newVal;
                    while( --j >= 0 && img[j] == val0 )
                        img[j] = newVal;

                    while( ++i < roi.width && img[i] == val0 )
                        img[i] = newVal;

                    ICV_PUSH( YC + dir, j+1, i-1, L, R, -dir );
                }
            }
        }
    }

exit_func:
    if( region )
    {
        region->area = area;
        region->rect.x = XMin;
        region->rect.y = YMin;
        region->rect.width = XMax - XMin + 1;
        region->rect.height = YMax - YMin + 1;
        region->value = newVal;
    }

    return CV_NO_ERR;
}


#define ICV_EQ_C3( p1, p2 ) \
    ((p1)[0] == (p2)[0] && (p1)[1] == (p2)[1] && (p1)[2] == (p2)[2])

#define ICV_SET_C3( p, q ) \
    ((p)[0] = (q)[0], (p)[1] = (q)[1], (p)[2] = (q)[2])

static CvStatus
icvFloodFill_8u_C3IR( uchar* pImage, int step, CvSize roi, CvPoint seed,
                      uchar* _newVal, CvConnectedComp* region, int flags,
                      CvFFillSegment* buffer, int buffersize )
{
    uchar* img = pImage + step * seed.y;
    int i, L, R; 
    int area = 0;
    int val0[3] = { 0, 0, 0 };
    uchar newVal[3] = { _newVal[0], _newVal[1], _newVal[2] };
    int XMin, XMax, YMin = seed.y, YMax = seed.y;
    int _8_connectivity = (flags & 255) == 8;
    CvFFillSegment* buffer_end = buffer + buffersize, *head = buffer, *tail = buffer;

    L = R = seed.x;
    ICV_SET_C3( val0, img + L*3 );

    if( ICV_EQ_C3( val0, newVal ))
    {
        XMin = XMax = seed.x;
        goto exit_func;
    }

    ICV_SET_C3( img + L*3, newVal );
    
    while( --L >= 0 && ICV_EQ_C3( img + L*3, val0 ))
        ICV_SET_C3( img + L*3, newVal );
    
    while( ++R < roi.width && ICV_EQ_C3( img + R*3, val0 ))
        ICV_SET_C3( img + R*3, newVal );

    XMin = ++L;
    XMax = --R;
    ICV_PUSH( seed.y, L, R, R + 1, R, UP );

    while( head != tail )
    {
        int k, YC, PL, PR, dir, curstep;
        ICV_POP( YC, L, R, PL, PR, dir );

        int data[][3] =
        {
            {-dir, L - _8_connectivity, R + _8_connectivity},
            {dir, L - _8_connectivity, PL - 1},
            {dir, PR + 1, R + _8_connectivity}
        };

        if( region )
        {
            area += R - L + 1;

            if( XMax < R ) XMax = R;
            if( XMin > L ) XMin = L;
            if( YMax < YC ) YMax = YC;
            if( YMin > YC ) YMin = YC;
        }

        for( k = (unsigned)(YC - dir) >= (unsigned)roi.height; k < 3; k++ )
        {
            dir = data[k][0];
            curstep = dir * step;
            img = pImage + (YC + dir) * step;
            int left = data[k][1];
            int right = data[k][2];

            for( i = left; i <= right; i++ )
            {
                if( (unsigned)i < (unsigned)roi.width && ICV_EQ_C3( img + i*3, val0 ))
                {
                    int j = i;
                    ICV_SET_C3( img + i*3, newVal );
                    while( --j >= 0 && ICV_EQ_C3( img + j*3, val0 ))
                        ICV_SET_C3( img + j*3, newVal );

                    while( ++i < roi.width && ICV_EQ_C3( img + i*3, val0 ))
                        ICV_SET_C3( img + i*3, newVal );

                    ICV_PUSH( YC + dir, j+1, i-1, L, R, -dir );
                }
            }
        }
    }

exit_func:
    if( region )
    {
        region->area = area;
        region->rect.x = XMin;
        region->rect.y = YMin;
        region->rect.width = XMax - XMin + 1;
        region->rect.height = YMax - YMin + 1;
        region->value = CV_RGB( newVal[2], newVal[1], newVal[0] );
    }

    return CV_NO_ERR;
}


/* because all the operations on floats that are done during non-gradient floodfill
   are just copying and comparison on equality,
   we can do the whole op on 32-bit integers instead */
static CvStatus
icvFloodFill_32f_C1IR( int* pImage, int step, CvSize roi, CvPoint seed,
                       int* _newVal, CvConnectedComp* region, int flags,
                       CvFFillSegment* buffer, int buffersize )
{
    int* img = pImage + (step /= sizeof(pImage[0])) * seed.y;
    int i, L, R; 
    int area = 0;
    int val0 = 0;
    int newVal = _newVal[0];
    int XMin, XMax, YMin = seed.y, YMax = seed.y;
    int _8_connectivity = (flags & 255) == 8;
    CvFFillSegment* buffer_end = buffer + buffersize, *head = buffer, *tail = buffer;

    L = R = seed.x;
    val0 = img[L];

    if( val0 == newVal )
    {
        XMin = XMax = seed.x;
        goto exit_func;
    }

    img[L] = newVal;

    while( ++R < roi.width && img[R] == val0 )
        img[R] = newVal;

    while( --L >= 0 && img[L] == val0 )
        img[L] = newVal;

    XMax = --R;
    XMin = ++L;
    ICV_PUSH( seed.y, L, R, R + 1, R, UP );

    while( head != tail )
    {
        int k, YC, PL, PR, dir, curstep;
        ICV_POP( YC, L, R, PL, PR, dir );

        int data[][3] =
        {
            {-dir, L - _8_connectivity, R + _8_connectivity},
            {dir, L - _8_connectivity, PL - 1},
            {dir, PR + 1, R + _8_connectivity}
        };

        if( region )
        {
            area += R - L + 1;

            if( XMax < R ) XMax = R;
            if( XMin > L ) XMin = L;
            if( YMax < YC ) YMax = YC;
            if( YMin > YC ) YMin = YC;
        }

        for( k = (unsigned)(YC - dir) >= (unsigned)roi.height; k < 3; k++ )
        {
            dir = data[k][0];
            curstep = dir * step;
            img = pImage + (YC + dir) * step;
            int left = data[k][1];
            int right = data[k][2];

            for( i = left; i <= right; i++ )
            {
                if( (unsigned)i < (unsigned)roi.width && img[i] == val0 )
                {
                    int j = i;
                    img[i] = newVal;
                    while( --j >= 0 && img[j] == val0 )
                        img[j] = newVal;

                    while( ++i < roi.width && img[i] == val0 )
                        img[i] = newVal;

                    ICV_PUSH( YC + dir, j+1, i-1, L, R, -dir );
                }
            }
        }
    }

exit_func:
    if( region )
    {
        region->area = area;
        region->rect.x = XMin;
        region->rect.y = YMin;
        region->rect.width = XMax - XMin + 1;
        region->rect.height = YMax - YMin + 1;
        region->value = newVal;
    }

    return CV_NO_ERR;
}

/****************************************************************************************\
*                                   Gradient Floodfill                                   *
\****************************************************************************************/

#define DIFF_INT_C1(p1,p2) ((unsigned)((p1)[0] - (p2)[0] + d_lw)<= interval)

#define DIFF_INT_C3(p1,p2) ((unsigned)((p1)[0] - (p2)[0] + d_lw[0])<= interval[0] && \
                            (unsigned)((p1)[1] - (p2)[1] + d_lw[1])<= interval[1] && \
                            (unsigned)((p1)[2] - (p2)[2] + d_lw[2])<= interval[2])

#define DIFF_FLT_C1(p1,p2) (fabs((p1)[0] - (p2)[0] + d_lw)<= interval)

static CvStatus
icvFloodFill_Grad_8u_C1IR( uchar* pImage, int step, uchar* pMask, int maskStep,
                           CvSize /*roi*/, CvPoint seed, uchar* _newVal, uchar* _d_lw,
                           uchar* _d_up, CvConnectedComp* region, int flags,
                           CvFFillSegment* buffer, int buffersize )
{
    const int cn = 1;
    uchar* img = pImage + step*seed.y;
    uchar* mask = (pMask += maskStep + 1) + maskStep*seed.y;
    int i, L, R;
    int area = 0;
    int sum = 0, val0[1] = {0};
    uchar newVal = _newVal[0];
    int d_lw = _d_lw[0];
    int d_up = _d_up[0];
    unsigned interval = (unsigned) (d_up + d_lw);
    int XMin, XMax, YMin = seed.y, YMax = seed.y;
    int _8_connectivity = (flags & 255) == 8;
    int fixedRange = flags & CV_FLOODFILL_FIXED_RANGE;
    int fillImage = (flags & CV_FLOODFILL_MASK_ONLY) == 0;
    uchar newMaskVal = (uchar)(flags & 0xff00 ? flags >> 8 : 1);
    CvFFillSegment* buffer_end = buffer + buffersize, *head = buffer, *tail = buffer;

    L = R = seed.x;
    if( mask[L] )
        return CV_OK;

    mask[L] = newMaskVal;

    if( fixedRange )
    {
        val0[0] = img[seed.x];

        while( !mask[R + 1] && DIFF_INT_C1( img + (R+1)*cn, val0 ))
            mask[++R] = newMaskVal;

        while( !mask[L - 1] && DIFF_INT_C1( img + (L-1)*cn, val0 ))
            mask[--L] = newMaskVal;
    }
    else
    {
        while( !mask[R + 1] && DIFF_INT_C1( img + (R+1)*cn, img + R*cn ))
            mask[++R] = newMaskVal;

        while( !mask[L - 1] && DIFF_INT_C1( img + (L-1)*cn, img + L*cn ))
            mask[--L] = newMaskVal;
    }

    XMax = R;
    XMin = L;
    ICV_PUSH( seed.y, L, R, R + 1, R, UP );

    while( head != tail )
    {
        int k, YC, PL, PR, dir, curstep;
        ICV_POP( YC, L, R, PL, PR, dir );

        int data[][3] =
        {
            {-dir, L - _8_connectivity, R + _8_connectivity},
            {dir, L - _8_connectivity, PL - 1},
            {dir, PR + 1, R + _8_connectivity}
        };

        unsigned length = (unsigned)(R-L);

        if( region )
        {
            area += (int)length + 1;

            if( XMax < R ) XMax = R;
            if( XMin > L ) XMin = L;
            if( YMax < YC ) YMax = YC;
            if( YMin > YC ) YMin = YC;
        }

        for( k = 0; k < 3; k++ )
        {
            dir = data[k][0];
            curstep = dir * step;
            img = pImage + (YC + dir) * step;
            mask = pMask + (YC + dir) * maskStep;
            int left = data[k][1];
            int right = data[k][2];

            if( fixedRange )
            {
                for( i = left; i <= right; i++ )
                {
                    if( !mask[i] && DIFF_INT_C1( img + i*cn, val0 ))
                    {
                        int j = i;
                        mask[i] = newMaskVal;
                        while( !mask[--j] && DIFF_INT_C1( img + j*cn, val0 ))
                            mask[j] = newMaskVal;

                        while( !mask[++i] && DIFF_INT_C1( img + i*cn, val0 ))
                            mask[i] = newMaskVal;

                        ICV_PUSH( YC + dir, j+1, i-1, L, R, -dir );
                    }
                }
            }
            else if( !_8_connectivity )
            {
                for( i = left; i <= right; i++ )
                {
                    if( !mask[i] && DIFF_INT_C1( img + i*cn, img - curstep + i*cn ))
                    {
                        int j = i;
                        mask[i] = newMaskVal;
                        while( !mask[--j] && DIFF_INT_C1( img + j*cn, img + (j+1)*cn ))
                            mask[j] = newMaskVal;

                        while( !mask[++i] &&
                               (DIFF_INT_C1( img + i*cn, img + (i-1)*cn ) ||
                               (DIFF_INT_C1( img + i*cn, img + i*cn - curstep) && i <= R)))
                            mask[i] = newMaskVal;

                        ICV_PUSH( YC + dir, j+1, i-1, L, R, -dir );
                    }
                }
            }
            else
            {
                for( i = left; i <= right; i++ )
                {
                    int idx, val[1];
                    
                    if( !mask[i] &&
                        ((val[0] = img[i*cn],
                        (unsigned)(idx = i-L-1) <= length) &&
                        DIFF_INT_C1( val, img - curstep + (i-1)*cn ) ||
                        (unsigned)(++idx) <= length &&
                        DIFF_INT_C1( val, img - curstep + i*cn ) ||
                        (unsigned)(++idx) <= length &&
                        DIFF_INT_C1( val, img - curstep + (i+1)*cn )))
                    {
                        int j = i;
                        mask[i] = newMaskVal;
                        while( !mask[--j] && DIFF_INT_C1( img + j*cn, img + (j+1)*cn ))
                            mask[j] = newMaskVal;

                        while( !mask[++i] &&
                               ((val[0] = img[i*cn],
                               DIFF_INT_C1( val, img + (i-1)*cn )) ||
                               ((unsigned)(idx = i-L-1) <= length &&
                               DIFF_INT_C1( val, img - curstep + (i-1)*cn )) ||
                               (unsigned)(++idx) <= length &&
                               DIFF_INT_C1( val, img - curstep + i*cn ) ||
                               (unsigned)(++idx) <= length &&
                               DIFF_INT_C1( val, img - curstep + (i+1)*cn )))
                            mask[i] = newMaskVal;

                        ICV_PUSH( YC + dir, j+1, i-1, L, R, -dir );
                    }
                }
            }
        }

        img = pImage + YC * step;
        if( fillImage )
            for( i = L; i <= R; i++ )
                img[i] = newVal;
        else if( region )
            for( i = L; i <= R; i++ )
                sum += img[i];
    }
    
    if( region )
    {
        region->area = area;
        region->rect.x = XMin;
        region->rect.y = YMin;
        region->rect.width = XMax - XMin + 1;
        region->rect.height = YMax - YMin + 1;
    
        if( fillImage )
            region->value = newVal;
        else
            region->value = area ? ((double)sum)/area : 0;
    }

    return CV_NO_ERR;
}


static CvStatus
icvFloodFill_Grad_8u_C3IR( uchar* pImage, int step, uchar* pMask, int maskStep,
                           CvSize /*roi*/, CvPoint seed, uchar* _newVal, uchar* _d_lw,
                           uchar* _d_up, CvConnectedComp* region, int flags,
                           CvFFillSegment* buffer, int buffersize )
{
    const int cn = 3;
    uchar* img = pImage + step*seed.y;
    uchar* mask = (pMask += maskStep + 1) + maskStep*seed.y;
    int i, L, R;
    int area = 0;
    int sum[3] = { 0, 0, 0 }, val0[3] = { 0, 0, 0 };
    uchar newVal[3] = {_newVal[0], _newVal[1], _newVal[2]};
    int d_lw[3] = {_d_lw[0], _d_lw[1], _d_lw[2]};
    int d_up[3] = {_d_up[0], _d_up[1], _d_up[2]};
    unsigned interval[3] =
    { (unsigned) (d_up[0] + d_lw[0]),
      (unsigned) (d_up[1] + d_lw[1]),
      (unsigned) (d_up[2] + d_lw[2])};
    int XMin, XMax, YMin = seed.y, YMax = seed.y;
    int _8_connectivity = (flags & 255) == 8;
    int fixedRange = flags & CV_FLOODFILL_FIXED_RANGE;
    int fillImage = (flags & CV_FLOODFILL_MASK_ONLY) == 0;
    uchar newMaskVal = (uchar)(flags & 0xff00 ? flags >> 8 : 1);
    CvFFillSegment* buffer_end = buffer + buffersize, *head = buffer, *tail = buffer;

    L = R = seed.x;
    if( mask[L] )
        return CV_OK;

    mask[L] = newMaskVal;

    if( fixedRange )
    {
        val0[0] = img[seed.x*cn];
        val0[1] = img[seed.x*cn+1];
        val0[2] = img[seed.x*cn+2];

        while( DIFF_INT_C3( img + (R+1)*cn, val0 ) && !mask[R + 1] )
            mask[++R] = newMaskVal;

        while( DIFF_INT_C3( img + (L-1)*cn, val0 ) && !mask[L - 1] )
            mask[--L] = newMaskVal;
    }
    else
    {
        while( DIFF_INT_C3( img + (R+1)*cn, img + R*cn ) && !mask[R + 1] )
            mask[++R] = newMaskVal;

        while( DIFF_INT_C3( img + (L-1)*cn, img + L*cn ) && !mask[L - 1] )
            mask[--L] = newMaskVal;
    }

    XMax = R;
    XMin = L;
    ICV_PUSH( seed.y, L, R, R + 1, R, UP );

    while( head != tail )
    {
        int k, YC, PL, PR, dir, curstep;
        ICV_POP( YC, L, R, PL, PR, dir );

        int data[][3] =
        {
            {-dir, L - _8_connectivity, R + _8_connectivity},
            {dir, L - _8_connectivity, PL - 1},
            {dir, PR + 1, R + _8_connectivity}
        };

        unsigned length = (unsigned)(R-L);

        if( region )
        {
            area += (int)length + 1;

            if( XMax < R ) XMax = R;
            if( XMin > L ) XMin = L;
            if( YMax < YC ) YMax = YC;
            if( YMin > YC ) YMin = YC;
        }

        for( k = 0; k < 3; k++ )
        {
            dir = data[k][0];
            curstep = dir * step;
            img = pImage + (YC + dir) * step;
            mask = pMask + (YC + dir) * maskStep;
            int left = data[k][1];
            int right = data[k][2];

            if( fixedRange )
            {
                for( i = left; i <= right; i++ )
                {
                    if( !mask[i] && DIFF_INT_C3( img + i*cn, val0 ))
                    {
                        int j = i;
                        mask[i] = newMaskVal;
                        while( !mask[--j] && DIFF_INT_C3( img + j*cn, val0 ))
                            mask[j] = newMaskVal;

                        while( !mask[++i] && DIFF_INT_C3( img + i*cn, val0 ))
                            mask[i] = newMaskVal;

                        ICV_PUSH( YC + dir, j+1, i-1, L, R, -dir );
                    }
                }
            }
            else if( !_8_connectivity )
            {
                for( i = left; i <= right; i++ )
                {
                    if( !mask[i] && DIFF_INT_C3( img + i*cn, img - curstep + i*cn ))
                    {
                        int j = i;
                        mask[i] = newMaskVal;
                        while( !mask[--j] && DIFF_INT_C3( img + j*cn, img + (j+1)*cn ))
                            mask[j] = newMaskVal;

                        while( !mask[++i] &&
                               (DIFF_INT_C3( img + i*cn, img + (i-1)*cn ) ||
                               (DIFF_INT_C3( img + i*cn, img + i*cn - curstep) && i <= R)))
                            mask[i] = newMaskVal;

                        ICV_PUSH( YC + dir, j+1, i-1, L, R, -dir );
                    }
                }
            }
            else
            {
                for( i = left; i <= right; i++ )
                {
                    int idx, val[3];
                    
                    if( !mask[i] &&
                        ((val[0] = img[i*cn],
                          val[1] = img[i*cn+1],
                          val[2] = img[i*cn+2],
                        (unsigned)(idx = i-L-1) <= length) &&
                        DIFF_INT_C3( val, img - curstep + (i-1)*cn ) ||
                        (unsigned)(++idx) <= length &&
                        DIFF_INT_C3( val, img - curstep + i*cn ) ||
                        (unsigned)(++idx) <= length &&
                        DIFF_INT_C3( val, img - curstep + (i+1)*cn )))
                    {
                        int j = i;
                        mask[i] = newMaskVal;
                        while( !mask[--j] && DIFF_INT_C3( img + j*cn, img + (j+1)*cn ))
                            mask[j] = newMaskVal;

                        while( !mask[++i] &&
                               ((val[0] = img[i*cn],
                                 val[1] = img[i*cn+1],
                                 val[2] = img[i*cn+2],
                               DIFF_INT_C3( &val, img + (i-1)*cn )) ||
                               ((unsigned)(idx = i-L-1) <= length &&
                               DIFF_INT_C3( &val, img - curstep + (i-1)*cn )) ||
                               (unsigned)(++idx) <= length &&
                               DIFF_INT_C3( &val, img - curstep + i*cn ) ||
                               (unsigned)(++idx) <= length &&
                               DIFF_INT_C3( &val, img - curstep + (i+1)*cn )))
                            mask[i] = newMaskVal;

                        ICV_PUSH( YC + dir, j+1, i-1, L, R, -dir );
                    }
                }
            }
        }

        img = pImage + YC * step;
        if( fillImage )
            for( i = L; i <= R; i++ )
            {
                img[i*3] = newVal[0];
                img[i*3+1] = newVal[1];
                img[i*3+2] = newVal[2];
            }
        else if( region )
            for( i = L; i <= R; i++ )
            {
                sum[0] += img[i*3];
                sum[1] += img[i*3+1];
                sum[2] += img[i*3+2];
            }
    }
    
    if( region )
    {
        region->area = area;
        region->rect.x = XMin;
        region->rect.y = YMin;
        region->rect.width = XMax - XMin + 1;
        region->rect.height = YMax - YMin + 1;
    
        if( fillImage )
            region->value = CV_RGB(newVal[2],newVal[1],newVal[0]);
        else
        {
            region->value = 0;
            if( area )
            {
                double inv_area = 1./area;
                int b = cvRound(sum[0]*inv_area);
                int g = cvRound(sum[1]*inv_area);
                int r = cvRound(sum[2]*inv_area);
                b = CV_CAST_8U(b);
                g = CV_CAST_8U(g);
                r = CV_CAST_8U(r);
                region->value = CV_RGB( r, g, b );
            }
        }
    }

    return CV_NO_ERR;
}


static CvStatus
icvFloodFill_Grad_32f_C1IR( float* pImage, int step, uchar* pMask, int maskStep,
                            CvSize /*roiSize*/, CvPoint seed, float* _newVal, float* _d_lw,
                            float* _d_up, CvConnectedComp* region, int flags,
                            CvFFillSegment* buffer, int buffersize )
{
    const int cn = 1;
    float* img = pImage + (step /= sizeof(pImage[0]))*seed.y;
    uchar* mask = (pMask += maskStep + 1) + maskStep*seed.y;
    int i, L, R;
    int area = 0;
    double sum = 0;
    float val0[1] = {0};
    float newVal = _newVal[0];
    float interval = 0.5f*(_d_lw[0] + _d_up[0]);
    float d_lw = 0.5f*(_d_lw[0] - _d_up[0]);
    int XMin, XMax, YMin = seed.y, YMax = seed.y;
    int _8_connectivity = (flags & 255) == 8;
    int fixedRange = flags & CV_FLOODFILL_FIXED_RANGE;
    int fillImage = (flags & CV_FLOODFILL_MASK_ONLY) == 0;
    uchar newMaskVal = (uchar)(flags & 0xff00 ? flags >> 8 : 1);
    CvFFillSegment* buffer_end = buffer + buffersize, *head = buffer, *tail = buffer;

    L = R = seed.x;
    if( mask[L] )
        return CV_OK;

    mask[L] = newMaskVal;

    if( fixedRange )
    {
        val0[0] = img[seed.x];

        while( DIFF_FLT_C1( img + (R+1)*cn, val0 ) && !mask[R + 1] )
            mask[++R] = newMaskVal;

        while( DIFF_FLT_C1( img + (L-1)*cn, val0 ) && !mask[L - 1] )
            mask[--L] = newMaskVal;
    }
    else
    {
        while( DIFF_FLT_C1( img + (R+1)*cn, img + R*cn ) && !mask[R + 1] )
            mask[++R] = newMaskVal;

        while( DIFF_FLT_C1( img + (L-1)*cn, img + L*cn ) && !mask[L - 1] )
            mask[--L] = newMaskVal;
    }

    XMax = R;
    XMin = L;
    ICV_PUSH( seed.y, L, R, R + 1, R, UP );

    while( head != tail )
    {
        int k, YC, PL, PR, dir, curstep;
        ICV_POP( YC, L, R, PL, PR, dir );

        int data[][3] =
        {
            {-dir, L - _8_connectivity, R + _8_connectivity},
            {dir, L - _8_connectivity, PL - 1},
            {dir, PR + 1, R + _8_connectivity}
        };

        unsigned length = (unsigned)(R-L);

        if( region )
        {
            area += (int)length + 1;

            if( XMax < R ) XMax = R;
            if( XMin > L ) XMin = L;
            if( YMax < YC ) YMax = YC;
            if( YMin > YC ) YMin = YC;
        }

        for( k = 0; k < 3; k++ )
        {
            dir = data[k][0];
            curstep = dir * step;
            img = pImage + (YC + dir) * step;
            mask = pMask + (YC + dir) * maskStep;
            int left = data[k][1];
            int right = data[k][2];

            if( fixedRange )
            {
                for( i = left; i <= right; i++ )
                {
                    if( !mask[i] && DIFF_FLT_C1( img + i*cn, val0 ))
                    {
                        int j = i;
                        mask[i] = newMaskVal;
                        while( !mask[--j] && DIFF_FLT_C1( img + j*cn, val0 ))
                            mask[j] = newMaskVal;

                        while( !mask[++i] && DIFF_FLT_C1( img + i*cn, val0 ))
                            mask[i] = newMaskVal;

                        ICV_PUSH( YC + dir, j+1, i-1, L, R, -dir );
                    }
                }
            }
            else if( !_8_connectivity )
            {
                for( i = left; i <= right; i++ )
                {
                    if( !mask[i] && DIFF_FLT_C1( img + i*cn, img - curstep + i*cn ))
                    {
                        int j = i;
                        mask[i] = newMaskVal;
                        while( !mask[--j] && DIFF_FLT_C1( img + j*cn, img + (j+1)*cn ))
                            mask[j] = newMaskVal;

                        while( !mask[++i] &&
                               (DIFF_FLT_C1( img + i*cn, img + (i-1)*cn ) ||
                               (DIFF_FLT_C1( img + i*cn, img + i*cn - curstep) && i <= R)))
                            mask[i] = newMaskVal;

                        ICV_PUSH( YC + dir, j+1, i-1, L, R, -dir );
                    }
                }
            }
            else
            {
                for( i = left; i <= right; i++ )
                {
                    int idx;
                    float val[1];
                    
                    if( !mask[i] &&
                        ((val[0] = img[i*cn],
                        (unsigned)(idx = i-L-1) <= length) &&
                        DIFF_FLT_C1( val, img - curstep + (i-1)*cn ) ||
                        (unsigned)(++idx) <= length &&
                        DIFF_FLT_C1( val, img - curstep + i*cn ) ||
                        (unsigned)(++idx) <= length &&
                        DIFF_FLT_C1( val, img - curstep + (i+1)*cn )))
                    {
                        int j = i;
                        mask[i] = newMaskVal;
                        while( !mask[--j] && DIFF_FLT_C1( img + j*cn, img + (j+1)*cn ))
                            mask[j] = newMaskVal;

                        while( !mask[++i] &&
                               ((val[0] = img[i*cn],
                               DIFF_FLT_C1( val, img + (i-1)*cn )) ||
                               ((unsigned)(idx = i-L-1) <= length &&
                               DIFF_FLT_C1( val, img - curstep + (i-1)*cn )) ||
                               (unsigned)(++idx) <= length &&
                               DIFF_FLT_C1( val, img - curstep + i*cn ) ||
                               (unsigned)(++idx) <= length &&
                               DIFF_FLT_C1( val, img - curstep + (i+1)*cn )))
                            mask[i] = newMaskVal;

                        ICV_PUSH( YC + dir, j+1, i-1, L, R, -dir );
                    }
                }
            }
        }

        img = pImage + YC * step;
        if( fillImage )
            for( i = L; i <= R; i++ )
                img[i] = newVal;
        else if( region )
            for( i = L; i <= R; i++ )
                sum += img[i];
    }
    
    if( region )
    {
        region->area = area;
        region->rect.x = XMin;
        region->rect.y = YMin;
        region->rect.width = XMax - XMin + 1;
        region->rect.height = YMax - YMin + 1;
    
        if( fillImage )
            region->value = newVal;
        else
            region->value = area ? ((double)sum)/area : 0;
    }

    return CV_NO_ERR;
}


/****************************************************************************************\
*                                    External Functions                                  *
\****************************************************************************************/

typedef  CvStatus (CV_CDECL* CvFloodFillFunc)(
                           void* img, int step, CvSize size, CvPoint seed,
                           void* newval, CvConnectedComp* comp, int flags,
                           void* buffer, int buffersize );

typedef  CvStatus (CV_CDECL* CvFloodFillGradFunc)(
                           void* img, int step, uchar* mask, int maskStep, CvSize size,
                           CvPoint seed, void* newval, void* d_lw, void* d_up,
                           void* ccomp, int flags, void* buffer, int buffersize );

static  void  icvInitFloodFill( void** ffill_tab,
                                void** ffillgrad_tab )
{
    ffill_tab[0] = (void*)icvFloodFill_8u_C1IR;
    ffill_tab[1] = (void*)icvFloodFill_8u_C3IR;
    ffill_tab[2] = (void*)icvFloodFill_32f_C1IR;

    ffillgrad_tab[0] = (void*)icvFloodFill_Grad_8u_C1IR;
    ffillgrad_tab[1] = (void*)icvFloodFill_Grad_8u_C3IR;
    ffillgrad_tab[2] = (void*)icvFloodFill_Grad_32f_C1IR;
}


CV_IMPL void
cvFloodFill( CvArr* arr, CvPoint seed_point,
             double newVal, double lo_diff, double up_diff,
             CvConnectedComp* comp, int flags, CvArr* maskarr )
{
    static void* ffill_tab[3];
    static void* ffillgrad_tab[3];
    static int inittab = 0;

    CvMat* tempMask = 0;
    CvFFillSegment* buffer = 0;
    CV_FUNCNAME( "cvFloodFill" );

    __BEGIN__;

    int type, is_simple, idx;
    int connectivity = flags & 255;
    int buffersize;
    double nv_buf, ld_buf, ud_buf;
    CvSize size;
    CvMat stub, *img = (CvMat*)arr;
    CvMat maskstub, *mask = (CvMat*)maskarr;

    if( !inittab )
    {
        icvInitFloodFill( ffill_tab, ffillgrad_tab );
        inittab = 1;
    }

    CV_CALL( img = cvGetMat( img, &stub ));
    type = CV_MAT_TYPE( img->type );

    idx = type == CV_8UC1 ? 0 : type == CV_8UC3 ? 1 : type == CV_32FC1 ? 2 : -1;

    if( idx < 0 )
        CV_ERROR( CV_StsUnsupportedFormat, "" );

    if( connectivity == 0 )
        connectivity = 4;
    else if( connectivity != 4 && connectivity != 8 )
        CV_ERROR( CV_StsBadFlag, "Connectivity must be 4, 0(=4) or 8" );

    if( type != CV_8UC3 && (lo_diff < 0 || up_diff < 0) )
        CV_ERROR( CV_StsBadArg, "lo_diff and up_diff must be non-negative" );

    size = icvGetMatSize( img );

    if( (unsigned)seed_point.x >= (unsigned)size.width ||
        (unsigned)seed_point.y >= (unsigned)size.height )
        CV_ERROR( CV_StsOutOfRange, "Seed point is outside of image" );

    is_simple = lo_diff == 0 && up_diff == 0 && mask == 0 &&
                (flags & CV_FLOODFILL_MASK_ONLY) == 0;

    icvExtractColor( newVal, type, &nv_buf );
    buffersize = MAX( size.width, size.height )*2;
    CV_CALL( buffer = (CvFFillSegment*)cvAlloc( buffersize*sizeof(buffer[0])));

    if( is_simple )
    {
        CvFloodFillFunc func = (CvFloodFillFunc)ffill_tab[idx];
        if( !func )
            CV_ERROR( CV_StsUnsupportedFormat, "" );
        
        IPPI_CALL( func( img->data.ptr, img->step, size,
                         seed_point, &nv_buf, comp, flags,
                         buffer, buffersize ));
    }
    else
    {
        CvFloodFillGradFunc func = (CvFloodFillGradFunc)ffillgrad_tab[idx];
        if( !func )
            CV_ERROR( CV_StsUnsupportedFormat, "" );
        
        if( !mask )
        {
            /* created mask will be 8-byte aligned */
            tempMask = cvCreateMat( size.height + 2, (size.width + 9) & -8, CV_8UC1 );
            mask = tempMask;
        }
        else
        {
            CV_CALL( mask = cvGetMat( mask, &maskstub ));
            if( !CV_IS_MASK_ARR( mask ))
                CV_ERROR( CV_StsBadMask, "" );

            if( mask->width != size.width + 2 || mask->height != size.height + 2 )
                CV_ERROR( CV_StsUnmatchedSizes, "mask must be 2 pixel wider "
                                       "and 2 pixel taller than filled image" );
        }

        {
            int i, width = tempMask ? mask->step : size.width + 2;
            uchar* mask_row = mask->data.ptr + mask->step;
            memset( mask_row - mask->step, 1, width );

            for( i = 1; i <= size.height; i++, mask_row += mask->step )
            {
                if( tempMask )
                    memset( mask_row, 0, width );
                mask_row[0] = mask_row[size.width+1] = (uchar)1;
            }
            memset( mask_row, 1, width );
        }

        icvExtractColor( lo_diff, type, &ld_buf );
        icvExtractColor( up_diff, type, &ud_buf );

        IPPI_CALL( func( img->data.ptr, img->step, mask->data.ptr, mask->step,
                         size, seed_point, &nv_buf, &ld_buf, &ud_buf,
                         comp, flags, buffer, buffersize ));
    }

    __END__;

    cvFree( (void**)&buffer );
    cvReleaseMat( &tempMask );
}

/* End of file. */
