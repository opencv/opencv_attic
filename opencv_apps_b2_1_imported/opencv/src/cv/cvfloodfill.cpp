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
#include "_cvwrap.h"

typedef struct
{
    int y;
    int l;
    int r;
    int Prevl;
    int Prevr;
    int fl;
} Seg;

#define UP 1
#define DOWN -1             

#define PUSH(Y,IL,IR,IPL,IPR,FL) {  stack[StIn].y=Y; \
                                    stack[StIn].l=IL; \
                                    stack[StIn].r=IR; \
                                    stack[StIn].Prevl=IPL; \
                                    stack[StIn].Prevr=IPR; \
                                    stack[StIn].fl=FL; \
                                    StIn++; }

#define POP(Y,IL,IR,IPL,IPR,FL)  {  StIn--; \
                                    Y=stack[StIn].y; \
                                    IL=stack[StIn].l; \
                                    IR=stack[StIn].r;\
                                    IPL=stack[StIn].Prevl; \
                                    IPR=stack[StIn].Prevr; \
                                    FL=stack[StIn].fl; }


/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name: CvFloodFillGetSize, CvFloodFillGetSize_Grad
//    Purpose: The functions calculate size of buffer necessary for calling CvFloodFill
//    Context:
//    Parameters:  
//                 roi        - size of image ROI,
//                 bufferSize - size of buffer in bytes output parameter.
//
//    Returns: CV_NO_ERR or error code
//    Notes:   
//F*/

IPCVAPI_IMPL( CvStatus, icvFloodFillGetSize, (CvSize roi, int *bufferSize) )
{
    if( roi.width <= 0 || roi.height <= 0 )
        return CV_BADSIZE_ERR;
    if( !bufferSize )
        return CV_NULLPTR_ERR;
    (*bufferSize) = roi.height * roi.width * (sizeof( Seg )) >> 2;
    return CV_NO_ERR;
}

IPCVAPI_IMPL( CvStatus, icvFloodFillGetSize_Grad, (CvSize roi, int *bufferSize) )
{
    if( roi.width <= 0 || roi.height <= 0 )
        return CV_BADSIZE_ERR;
    if( !bufferSize )
        return CV_NULLPTR_ERR;
    
        (*bufferSize) =
        (roi.width + 2) * (roi.height + 2) + (roi.height * roi.width * sizeof( Seg )) / 4;
    return CV_NO_ERR;
}

/*F///////////////////////////////////////////////////////////////////////////////////////
//    Names: icvFloodFill_8uC1R,  icvFloodFill_32fC1R,
//           icvFloodFill8_8u_C1R, icvFloodFill8_32f_C1R
//    Purpose: The functions fill the seed pixel environs inside which all pixel
//             values are not far from each other. 
//    Context:
//    Parameters:  img        - pointer to ROI of initial image (in the beginning)
//                              which is "repainted" during the function action,
//                 step       - full string length of initial image (in bytes),
//                 roi        - size of image ROI,
//                 initPoint  - coordinates of the seed point inside image ROI,
//                 nv         - new value of repainted area pixels,
//                 d_lw, d_up - maximal lower and upper differences of the values of
//                              appurtenant to repainted area pixel and one of its
//                              neighbour,
//                 comp       - pointer to connected component structure of the
//                              repainted area
//
//    Notes:   The 1 and 2 functions look for 4-connected environs,
//             the 3 and 4 ones - 8-connected.
//F*/
/*--------------------------------------------------------------------------------------*/

IPCVAPI_IMPL( CvStatus, icvFloodFill_4Con_8u_C1IR, (uchar * pImage, int step,
                                                    CvSize roi, CvPoint seed,
                                                    int* _newVal, CvConnectedComp * region,
                                                    void *pBuffer) )
{
    uchar *Temp = pImage + step * seed.y;
    uchar defcol = Temp[seed.x];
    uchar uv = (uchar)*_newVal;
    Seg *stack;
    int StIn = 0;
    unsigned int hght = (unsigned int) (roi.height);
    int i, j, YC, L, R, PL, PR, flag;
    int Sum = 0;
    int XMin, XMax, YMin = seed.y, YMax = seed.y;

    if( pImage == NULL )
        return CV_NULLPTR_ERR;
    if( roi.width <= 0 || roi.height <= 0 )
        return CV_BADSIZE_ERR;
    if( roi.width > step )
        return CV_BADSIZE_ERR;
    if( seed.x < 0 || seed.x >= roi.width )
        return CV_BADPOINT_ERR;
    if( seed.y < 0 || seed.y >= roi.height )
        return CV_BADPOINT_ERR;

    if( defcol == uv )
        return CV_NO_ERR;
    stack = (Seg *) pBuffer;
    L = seed.x;
    R = seed.x;
    YC = seed.y;
    Temp[R] = uv;
    while( (R < roi.width - 1) && (Temp[R + 1] == defcol) )
    {
        Temp[++R] = uv;
    }
    XMax = R;
    while( L && (Temp[L - 1] == defcol) )
    {
        Temp[--L] = uv;
    }
    XMin = L;
    Sum += R - L + 1;
    flag = (YC != roi.height - 1) ? UP : DOWN;
    PUSH( YC, L, R, R + 1, R, flag );

    while( StIn )
    {
        POP( YC, L, R, PL, PR, flag );
        XMax = MAX( XMax, R );
        XMin = MIN( XMin, L );
        YMax = MAX( YMax, YC );
        YMin = MIN( YMin, YC );
        if( ((unsigned) (YC - flag)) < hght )
        {
            Temp = pImage + (YC - flag) * step;
            for( i = L; i < R + 1; i++ )
            {
                if( Temp[i] == defcol )
                {
                    j = i;
                    Temp[i] = uv;
                    while( j && (Temp[j - 1] == defcol) )
                    {
                        Temp[--j] = uv;
                    }
                    while( (i < roi.width - 1) && (Temp[i + 1] == defcol) )
                    {
                        Temp[++i] = uv;
                    }
                    PUSH( YC - flag, j, i++, L, R, flag );
                    Sum += i - j;
                }
            }
        }
        Temp = pImage + (YC + flag) * step;
        for( i = L; i < PL; i++ )
        {
            if( Temp[i] == defcol )
            {
                j = i;
                Temp[i] = uv;
                while( j && (Temp[j - 1] == defcol) )
                {
                    Temp[--j] = uv;
                }
                while( (i < roi.width - 1) && (Temp[i + 1] == defcol) )
                {
                    Temp[++i] = uv;
                }
                PUSH( YC + flag, j, i++, L, R, -flag );
                Sum += i - j;
            }
        }
        for( i = PR + 1; i < R + 1; i++ )
        {
            if( Temp[i] == defcol )
            {
                j = i;
                Temp[i] = uv;
                while( j && (Temp[j - 1] == defcol) )
                {
                    Temp[--j] = uv;
                }
                while( (i < roi.width - 1) && (Temp[i + 1] == defcol) )
                {
                    Temp[++i] = uv;
                }
                PUSH( YC + flag, j, i++, L, R, -flag );
                Sum += i - j;
            }
        }
    }
    region->area = Sum;
    region->rect.x = XMin;
    region->rect.y = YMin;
    region->rect.width = XMax - XMin + 1;
    region->rect.height = YMax - YMin + 1;
    region->value = (float)*_newVal;
    return CV_NO_ERR;
}

IPCVAPI_IMPL( CvStatus, icvFloodFill_4Con_32f_C1IR, (float *pImage, int step,
                                                     CvSize roi, CvPoint seed,
                                                     float* newVal,
                                                     CvConnectedComp * region, void *pBuffer) )
{
    int ownstep = step / 4;
    float *Temp = pImage + ownstep * seed.y;
    Seg *stack;
    int StIn = 0;
    int i, j, YC, L, R, PL, PR, flag;
    float defcol = Temp[seed.x];
    float uv = *newVal;
    unsigned int hght = (unsigned int) (roi.height);
    int Sum = 0;
    int XMin, XMax, YMin = seed.y, YMax = seed.y;

    if( pImage == NULL )
        return CV_NULLPTR_ERR;
    if( roi.width < 1 || roi.height < 1 )
        return CV_BADSIZE_ERR;
    if( roi.width > step )
        return CV_BADSIZE_ERR;
    if( seed.x < 0 || seed.x >= roi.width )
        return CV_BADPOINT_ERR;
    if( seed.y < 0 || seed.y >= roi.height )
        return CV_BADPOINT_ERR;
    if( defcol == uv )
        return CV_NO_ERR;
    stack = (Seg *) pBuffer;
    L = seed.x;
    R = seed.x;
    YC = seed.y;
    Temp[R] = uv;
    while( (R < roi.width - 1) && (Temp[R + 1] == defcol) )
    {
        Temp[++R] = uv;
    }
    XMax = R;
    while( L && (Temp[L - 1] == defcol) )
    {
        Temp[--L] = uv;
    }
    XMin = L;
    Sum += R - L + 1;
    flag = (YC != roi.height - 1) ? UP : DOWN;
    PUSH( YC, L, R, R + 1, R, flag );

    while( StIn )
    {
        POP( YC, L, R, PL, PR, flag );
        XMax = MAX( XMax, R );
        XMin = MIN( XMin, L );
        YMax = MAX( YMax, YC );
        YMin = MIN( YMin, YC );
        if( ((unsigned) (YC - flag)) < hght )
        {
            Temp = pImage + (YC - flag) * ownstep;
            for( i = L; i < R + 1; i++ )
            {
                if( Temp[i] == defcol )
                {
                    j = i;
                    Temp[i] = uv;
                    while( j && (Temp[j - 1] == defcol) )
                    {
                        Temp[--j] = uv;
                    }
                    while( (i < roi.width - 1) && (Temp[i + 1] == defcol) )
                    {
                        Temp[++i] = uv;
                    }
                    PUSH( YC - flag, j, i++, L, R, flag );
                    Sum += i - j;
                }
            }
        }
        Temp = pImage + (YC + flag) * ownstep;
        for( i = L; i < PL; i++ )
        {
            if( Temp[i] == defcol )
            {
                j = i;
                Temp[i] = uv;
                while( j && (Temp[j - 1] == defcol) )
                {
                    Temp[--j] = uv;
                }
                while( (i < roi.width - 1) && (Temp[i + 1] == defcol) )
                {
                    Temp[++i] = uv;
                }
                PUSH( YC + flag, j, i++, L, R, -flag );
                Sum += i - j;
            }
        }
        for( i = PR + 1; i < R + 1; i++ )
        {
            if( Temp[i] == defcol )
            {
                j = i;
                Temp[i] = uv;
                while( j && (Temp[j - 1] == defcol) )
                {
                    Temp[--j] = uv;
                }
                while( (i < roi.width - 1) && (Temp[i + 1] == defcol) )
                {
                    Temp[++i] = uv;
                }
                PUSH( YC + flag, j, i++, L, R, -flag );
                Sum += i - j;
            }
        }
    }
    region->area = Sum;
    region->rect.x = XMin;
    region->rect.y = YMin;
    region->rect.width = XMax - XMin + 1;
    region->rect.height = YMax - YMin + 1;
    region->value = uv;
    return CV_NO_ERR;
}

IPCVAPI_IMPL( CvStatus, icvFloodFill_8Con_8u_C1IR, (uchar * pImage, int step,
                                                    CvSize roi, CvPoint seed,
                                                    int* _newVal, CvConnectedComp* region,
                                                    void *pBuffer) )
{
    uchar *Temp = pImage + step * seed.y;
    uchar *NL;
    uchar defcol = Temp[seed.x];
    uchar uv = (uchar)*_newVal;
    int LB, RB;
    unsigned int hght = (unsigned int) (roi.height);
    Seg *stack;
    int StIn = 0;
    int i, j, YC, L, R, PL, PR, flag;
    int Sum = 0;
    int XMin, XMax, YMin = seed.y, YMax = seed.y;

    if( pImage == NULL )
        return CV_NULLPTR_ERR;
    if( roi.width <= 0 || roi.height <= 0 )
        return CV_BADSIZE_ERR;
    if( roi.width > step )
        return CV_BADSIZE_ERR;
    if( seed.x < 0 || seed.x >= roi.width )
        return CV_BADPOINT_ERR;
    if( seed.y < 0 || seed.y >= roi.height )
        return CV_BADPOINT_ERR;


    if( defcol == uv )
        return CV_NO_ERR;
    stack = (Seg *) pBuffer;
    L = seed.x;
    R = seed.x;
    YC = seed.y;
    Temp[R] = uv;
    while( (R < roi.width - 1) && (Temp[R + 1] == defcol) )
    {
        Temp[++R] = uv;
    }
    XMax = R;
    while( L && (Temp[L - 1] == defcol) )
    {
        Temp[--L] = uv;
    }
    XMin = L;
    Sum += R - L + 1;
    PL = MIN( R + 2, roi.width );
    flag = (YC != roi.height - 1) ? UP : DOWN;
    PUSH( YC, L, R, R + 1, R, flag );

    while( StIn )
    {
        POP( YC, L, R, PL, PR, flag );
        XMax = MAX( XMax, R );
        XMin = MIN( XMin, L );
        YMax = MAX( YMax, YC );
        YMin = MIN( YMin, YC );
        if( ((unsigned) (YC - flag)) < hght )
        {


            NL = pImage + (YC - flag) * step;
            LB = MAX( 0, L - 1 );
            RB = MIN( roi.width, R + 2 );
            for( i = LB; i < RB; i++ )
            {
                if( NL[i] == defcol )
                {
                    j = i;
                    NL[i] = uv;
                    while( j && (NL[j - 1] == defcol) )
                    {
                        NL[--j] = uv;
                    }
                    while( (i < roi.width - 1) && (NL[i + 1] == defcol) )
                    {
                        NL[++i] = uv;
                    }
                    PUSH( YC - flag, j, i++, L, R, flag );
                    Sum += i - j;
                }
            }
        }
        NL = pImage + (YC + flag) * step;
        LB = MAX( L - 1, 0 );
        for( i = LB; i < PL; i++ )
        {
            if( NL[i] == defcol )
            {
                j = i;
                NL[i] = uv;
                while( j && (NL[j - 1] == defcol) )
                {
                    NL[--j] = uv;
                }
                while( (i < roi.width - 1) && (NL[i + 1] == defcol) )
                {
                    NL[++i] = uv;
                }
                PUSH( YC + flag, j, i++, L, R, -flag );
                Sum += i - j;
            }
        }
        RB = MIN( roi.width, R + 2 );
        for( i = PR + 1; i < RB; i++ )
        {
            if( NL[i] == defcol )
            {
                j = i;
                NL[i] = uv;
                while( j && (NL[j - 1] == defcol) )
                {
                    NL[--j] = uv;
                }
                while( (i < roi.width - 1) && (NL[i + 1] == defcol) )
                {
                    NL[++i] = uv;
                }
                PUSH( YC + flag, j, i++, L, R, -flag );
                Sum += i - j;
            }
        }
    }

    region->area = Sum;
    region->rect.x = XMin;
    region->rect.y = YMin;
    region->rect.width = XMax - XMin + 1;
    region->rect.height = YMax - YMin + 1;
    region->value = (float)*_newVal;
    return CV_NO_ERR;
}

IPCVAPI_IMPL( CvStatus, icvFloodFill_8Con_32f_C1IR, (float *pImage,
                                                     int step,
                                                     CvSize roi,
                                                     CvPoint seed,
                                                     float* _newVal,
                                                     CvConnectedComp * region, void *pBuffer) )
{
    int ownstep = step / 4;
    float *Temp = pImage + ownstep * seed.y;
    float *NL;
    unsigned int hght = (unsigned int) (roi.height);
    float defcol = Temp[seed.x];
    float newVal = *_newVal;
    Seg *stack;
    int StIn = 0;
    int i, j, YC, L, R, PL, PR, flag;
    int Sum = 0;
    int LB, RB;
    int XMin, XMax, YMin = seed.y, YMax = seed.y;

    if( pImage == NULL )
        return CV_NULLPTR_ERR;
    if( roi.width < 1 || roi.height < 1 )
        return CV_BADSIZE_ERR;
    if( roi.width > step )
        return CV_BADPOINT_ERR;
    if( seed.x < 0 || seed.x >= roi.width )
        return CV_BADPOINT_ERR;
    if( seed.y < 0 || seed.y >= roi.height )
        return CV_BADPOINT_ERR;
    if( defcol == *_newVal )
        return CV_NO_ERR;
    stack = (Seg *) pBuffer;
    L = seed.x;
    R = seed.x;
    YC = seed.y;
    Temp[R] = newVal;
    while( (R < roi.width - 1) && (Temp[R + 1] == defcol) )
    {
        Temp[++R] = newVal;
    }
    XMax = R;
    while( L && (Temp[L - 1] == defcol) )
    {
        Temp[--L] = newVal;
    }
    XMin = L;
    Sum += R - L + 1;
    PL = MIN( roi.width, R + 2 );
    flag = (YC != roi.height - 1) ? UP : DOWN;
    PUSH( YC, L, R, R + 1, R, flag );

    while( StIn )
    {
        POP( YC, L, R, PL, PR, flag );
        XMax = MAX( XMax, R );
        XMin = MIN( XMin, L );
        YMax = MAX( YMax, YC );
        YMin = MIN( YMin, YC );
        if( ((unsigned) (YC - flag)) < hght )
        {
            NL = pImage + (YC - flag) * ownstep;
            LB = MAX( 0, L - 1 );
            RB = MIN( R + 2, roi.width );
            for( i = LB; i < RB; i++ )
            {
                if( NL[i] == defcol )
                {
                    j = i;
                    NL[i] = newVal;
                    while( j && (NL[j - 1] == defcol) )
                    {
                        NL[--j] = newVal;
                    }
                    while( (i < roi.width - 1) && (NL[i + 1] == defcol) )
                    {
                        NL[++i] = newVal;
                    }
                    PUSH( YC - flag, j, i++, L, R, flag );
                    Sum += i - j;
                }
            }
        }
        NL = pImage + (YC + flag) * ownstep;
        LB = MAX( L - 1, 0 );
        for( i = LB; i < PL; i++ )
        {
            if( NL[i] == defcol )
            {
                j = i;
                NL[i] = newVal;
                while( j && (NL[j - 1] == defcol) )
                {
                    NL[--j] = newVal;
                }
                while( (i < roi.width - 1) && (NL[i + 1] == defcol) )
                {
                    NL[++i] = newVal;
                }
                PUSH( YC + flag, j, i++, L, R, -flag );
                Sum += i - j;
            }
        }
        RB = MIN( roi.width, R + 2 );
        for( i = PR + 1; i < RB; i++ )
        {
            if( NL[i] == defcol )
            {
                j = i;
                NL[i] = newVal;
                while( j && (NL[j - 1] == defcol) )
                {
                    NL[--j] = newVal;
                }
                while( (i < roi.width - 1) && (NL[i + 1] == defcol) )
                {
                    NL[++i] = newVal;
                }
                PUSH( YC + flag, j, i++, L, R, -flag );
                Sum += i - j;
            }
        }
    }

    region->area = Sum;
    region->rect.x = XMin;
    region->rect.y = YMin;
    region->rect.width = XMax - XMin + 1;
    region->rect.height = YMax - YMin + 1;
    region->value = *_newVal;
    return CV_NO_ERR;
}

/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name: ippiFloodFill_8uC1R, ippiFloodFill_32fC1R
//    Purpose: The function fills the seed pixel enewValirons inside which all pixel
//             values are not far from each other. 
//    Context:
//    Parameters:  pImage     - pointer to ROI of initial image (in the beginning)
//                              which is "repainted" during the function action,
//                 step       - full string length of initial image (in bytes),
//                 roi        - size of image ROI,
//                 seed  - coordinates of the seed point inside image ROI,
//                 newVal         - new value of repainted area pixels,
//                 d_lw, d_up - maximal lower and upper differences of the values of
//                              appurtenant to repainted area pixel and one of its
//                              neighbour,
//                 pPainted   - pointer to counter of repainted pixels number,
//                 rect       - pointer to rectangle circumscribed about the
//                              repainted area
//
//    Returns: CV_NO_ERR or error code
//    Notes:   This function uses a rapid non-recursive algorithm.
//F*/

IPCVAPI_IMPL( CvStatus, icvFloodFill_Grad4Con_8u_C1IR, (uchar * pImage,
                                                        int step,
                                                        CvSize roi,
                                                        CvPoint seed,
                                                        int* _newVal,
                                                        int* _d_lw,
                                                        int* _d_up,
                                                        CvConnectedComp * region,
                                                        void *pBuffer) )
{
    uchar *Temp = pImage + step * seed.y;
    uchar *TempRP;
    uchar *RP;
    uchar uv = (uchar)*_newVal;
    int d_lw = *_d_lw;
    int d_up = *_d_up;
    Seg *stack;
    int StIn = 0;
    int i, j, YC, L, R, PL, PR, flag;
    int Sum = 0;
    int curstep;
    unsigned int Interval = (unsigned int) (d_up + d_lw);
    int stepr = roi.width + 2;
    int XMin, XMax, YMin = seed.y, YMax = seed.y;

    if( pImage == NULL )
        return CV_NULLPTR_ERR;
    if( roi.width <= 0 || roi.height <= 0 )
        return CV_BADSIZE_ERR;
    if( roi.width > step )
        return CV_BADSIZE_ERR;
    if( seed.x < 0 || seed.x >= roi.width )
        return CV_BADPOINT_ERR;
    if( seed.y < 0 || seed.y >= roi.height )
        return CV_BADPOINT_ERR;
    if( d_lw < 0 || d_up < 0 )
        return CV_BADPOINT_ERR;

    RP = (uchar *) pBuffer;
    stack = (Seg *) ((long) pBuffer + (roi.width + 2) * (roi.height + 2));
    memset( RP, 1, stepr );
    memset( RP + (roi.height + 1) * stepr, 1, stepr );
    for( i = 0; i < roi.height; i++ )
    {
        TempRP = RP + (i + 1) * stepr;
        TempRP[0] = 1;
        TempRP[stepr - 1] = 1;
        TempRP++;
        memset( TempRP, 0, stepr - 2 );
    }
    L = seed.x;
    R = seed.x;
    YC = seed.y;
    TempRP = RP + (YC + 1) * stepr + 1;
    TempRP[L] = 1;
    while( ((((unsigned) (Temp[R + 1] - Temp[R] + d_lw)) <= Interval)) && (!TempRP[R + 1]) )
    {
        TempRP[++R] = 1;
    }
    XMax = R;
    while( ((((unsigned) (Temp[L - 1] - Temp[L] + d_lw)) <= Interval)) && (!TempRP[L - 1]) )
    {

        TempRP[--L] = 1;
    }
    XMin = L;
    PUSH( YC, L, R, R + 1, R, UP );

    while( StIn )
    {
        POP( YC, L, R, PL, PR, flag );
        XMax = MAX( XMax, R );
        XMin = MIN( XMin, L );
        YMax = MAX( YMax, YC );
        YMin = MIN( YMin, YC );
        curstep = flag * step;
        Temp = pImage + YC * step - curstep;
        TempRP = RP + (YC + 1 - flag) * stepr + 1;
        for( i = L; i < R + 1; i++ )
        {
            if( (!TempRP[i]) &&
                (((unsigned) (Temp[i] - Temp[i + curstep] + d_lw)) <= Interval) )
            {
                TempRP[i] = 1;
                j = i;
                while( (!TempRP[j - 1]) &&
                       (((unsigned) (Temp[j - 1] - Temp[j] + d_lw)) <= Interval) )
                {
                    TempRP[--j] = 1;
                }
                while( (!TempRP[i + 1]) &&
                       ((((unsigned) (Temp[i + 1] - Temp[i] + d_lw)) <= Interval) ||
                        (((unsigned) (Temp[i + 1] - Temp[i + 1 + curstep] + d_lw) <= Interval)
                         && (i < R))))
                {
                    TempRP[++i] = 1;
                }

                PUSH( YC - flag, j, i++, L, R, flag );

            }
        }
        Temp = pImage + YC * step + curstep;
        TempRP = RP + (YC + 1 + flag) * stepr + 1;
        for( i = L; i < PL; i++ )
        {
            if( (!TempRP[i]) &&
                (((unsigned) (Temp[i] - Temp[i - curstep] + d_lw)) <= Interval) )
            {
                TempRP[i] = 1;
                j = i;
                //moving left
                while( (!TempRP[j - 1]) &&
                       (((unsigned) (Temp[j - 1] - Temp[j] + d_lw)) <= Interval) )
                {
                    TempRP[--j] = 1;
                }
                while( (!TempRP[i + 1]) &&
                       ((((unsigned) (Temp[i + 1] - Temp[i] + d_lw)) <= Interval) ||
                        (((unsigned) (Temp[i + 1] - Temp[i + 1 - curstep] + d_lw) <= Interval)
                         & (i < R))))
                {
                    TempRP[++i] = 1;
                }
                PUSH( YC + flag, j, i++, L, R, -flag );
            }
        }
        for( i = PR + 1; i < R + 1; i++ )
        {
            if( (!TempRP[i]) &&
                (((unsigned) (Temp[i] - Temp[i - curstep] + d_lw)) <= Interval) )
            {
                TempRP[i] = 1;
                j = i;
                //moving left
                while( (!TempRP[j - 1]) &&
                       (((unsigned) (Temp[j - 1] - Temp[j] + d_lw)) <= Interval) )
                {
                    TempRP[--j] = 1;
                }
                while( (!TempRP[i + 1]) &&
                       ((((unsigned) (Temp[i + 1] - Temp[i] + d_lw)) <= Interval) ||
                        (((unsigned) (Temp[i + 1] - Temp[i + 1 - curstep] + d_lw) <= Interval)
                         && (i < R))))
                {
                    TempRP[++i] = 1;
                }
                PUSH( YC + flag, j, i++, L, R, -flag );
            }
        }
        for( i = L; i < R + 1; i++ )
            pImage[YC * step + i] = uv;
        Sum += (R - L + 1);
    }
    region->area = Sum;
    region->rect.x = XMin;
    region->rect.y = YMin;
    region->rect.width = XMax - XMin + 1;
    region->rect.height = YMax - YMin + 1;
    region->value = (float)*_newVal;
    return CV_NO_ERR;
}

/****************************************************************************************/
/******************************** 32fC1 flavor ******************************************/
/****************************************************************************************/
IPCVAPI_IMPL( CvStatus, icvFloodFill_Grad4Con_32f_C1IR, (float *pImage,
                                                         int step,
                                                         CvSize roi,
                                                         CvPoint seed,
                                                         float* _newVal,
                                                         float* _d_lw,
                                                         float* _d_up,
                                                         CvConnectedComp * region,
                                                         void *pBuffer) )
{
    int ownstep = step / 4;
    float *Temp = pImage + ownstep * seed.y;
    uchar *RP;
    uchar *TempRP;
    float newVal = *_newVal;
    float d_lw = *_d_lw;
    float d_up = *_d_up;
    Seg *stack;
    int StIn = 0;
    int i, j, YC, L, R, PL, PR, flag;
    int Sum = 0;
    int stepr = roi.width + 2;
    int curstep;
    float Addit = (d_lw - d_up) / 2;
    float Interval = (d_up + d_lw) / 2;
    int XMin, XMax, YMin = seed.y, YMax = seed.y;

    L = seed.x;
    R = seed.x;
    YC = seed.y;
    RP = (uchar *) pBuffer;
    memset( RP, 1, stepr );
    memset( RP + (roi.height + 1) * stepr, 1, stepr );
    for( i = 0; i < roi.height; i++ )
    {
        TempRP = RP + (i + 1) * stepr;
        TempRP[0] = 1;
        TempRP[stepr - 1] = 1;
        TempRP++;
        memset( TempRP, 0, stepr - 2 );

    }
    stack = (Seg *) ((long) pBuffer + (roi.width + 2) * (roi.height + 2));
    TempRP = RP + (YC + 1) * stepr + 1;
    TempRP[L] = 1;
    while( (!TempRP[R + 1]) && (((fabs( Temp[R + 1] - Temp[R] + Addit )) <= Interval)))
    {
        TempRP[++R] = 1;
    }
    XMax = R;
    while( (!TempRP[L - 1]) && (((fabs( Temp[L - 1] - Temp[L] + Addit )) <= Interval)))
    {

        TempRP[--L] = 1;
    }
    XMin = L;
    PUSH( YC, L, R, R + 1, R, UP );

    while( StIn )
    {
        POP( YC, L, R, PL, PR, flag );
        XMax = MAX( XMax, R );
        XMin = MIN( XMin, L );
        YMin = MIN( YC, YMin );
        YMax = MAX( YC, YMax );
        curstep = flag * ownstep;
        Temp = pImage + YC * ownstep - curstep;
        TempRP = RP + (YC + 1 - flag) * stepr + 1;
        for( i = L; i < R + 1; i++ )
        {
            if( (!TempRP[i]) && ((fabs( Temp[i] - Temp[i + curstep] + Addit )) <= Interval) )
            {

                TempRP[i] = 1;
                j = i;
                while( (!TempRP[j - 1]) &&
                       ((fabs( Temp[j - 1] - Temp[j] + Addit )) <= Interval) )
                {
                    TempRP[--j] = 1;
                }

                while( (!TempRP[i + 1]) &&
                       (((fabs( Temp[i + 1] - Temp[i] + Addit )) <= Interval) ||
                        ((fabs( Temp[i + 1] - Temp[i + 1 + curstep] + Addit ) <= Interval) &&
                         (i < R))))
                {
                    TempRP[++i] = 1;
                }

                PUSH( YC - flag, j, i++, L, R, flag );

            }
        }

        Temp = pImage + YC * ownstep + curstep;
        TempRP = RP + (YC + 1 + flag) * stepr + 1;
        for( i = L; i < PL; i++ )
        {
            if( (!TempRP[i]) && ((fabs( Temp[i] - Temp[i - curstep] + Addit )) <= Interval) )
            {

                TempRP[i] = 1;
                j = i;
                //moving left
                while( (!TempRP[j - 1]) &&
                       ((fabs( Temp[j - 1] - Temp[j] + Addit )) <= Interval) )
                {
                    TempRP[--j] = 1;
                }
                while( (!TempRP[i + 1]) &&
                       (((fabs( Temp[i + 1] - Temp[i] + Addit )) <= Interval) ||
                        ((fabs( Temp[i + 1] - Temp[i + 1 - curstep] + Addit ) <= Interval) &&
                         (i < R))))
                {
                    TempRP[++i] = 1;
                }
                PUSH( YC + flag, j, i++, L, R, -flag );
            }
        }
        for( i = PR + 1; i < R + 1; i++ )
        {
            if( (!TempRP[i]) && ((fabs( Temp[i] - Temp[i - curstep] + Addit )) <= Interval) )
            {
                TempRP[i] = 1;
                j = i;
                //moving left
                while( (!TempRP[j - 1]) &&
                       ((fabs( Temp[j - 1] - Temp[j] + Addit )) <= Interval) )
                {
                    TempRP[--j] = 1;
                }
                while( (!TempRP[i + 1]) &&
                       (((fabs( Temp[i + 1] - Temp[i] + Addit )) <= Interval) ||
                        ((fabs( Temp[i + 1] - Temp[i + 1 - curstep] + Addit ) <= Interval) &
                         (i < R))))
                {
                    TempRP[++i] = 1;
                }
                PUSH( YC + flag, j, i++, L, R, -flag );
            }
        }


        for( i = L; i < R + 1; i++ )
            pImage[YC * ownstep + i] = newVal;
        Sum += (R - L + 1);
    }
    region->area = Sum;
    region->rect.x = XMin;
    region->rect.y = YMin;
    region->rect.width = XMax - XMin + 1;
    region->rect.height = YMax - YMin + 1;
    region->value = (float) newVal;
    return CV_NO_ERR;
}

/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name: ippiFloodFill8_8u_C1R, ippiFloodFill8_32f_C1R
//    Purpose: The function fills the seed pixel enewValirons inside which all pixel
//             values are not far from each other (8-connected variant). 
//    Context:
//    Parameters:  pImage     - pointer to ROI of initial image (in the beginning)
//                              which is "repainted" during the function action,
//                 step       - full string length of initial image (in bytes),
//                 roi        - size of image ROI,
//                 seed  - coordinates of the seed point inside image ROI,
//                 newVal         - new value of repainted area pixels,
//                 d_lw, d_up - maximal lower and upper differences of the values of
//                              appurtenant to repainted area pixel and one of its
//                              neighbour,
//                 pPainted   - pointer to counter of repainted pixels number.
//
//    Returns: CV_NO_ERR or error code
//    Notes:   This function uses a rapid non-recursive algorithm.
//F*/
IPCVAPI_IMPL( CvStatus, icvFloodFill_Grad8Con_8u_C1IR, (uchar * pImage,
                                                        int step,
                                                        CvSize roi,
                                                        CvPoint seed,
                                                        int* _newVal,
                                                        int* _d_lw,
                                                        int* _d_up,
                                                        CvConnectedComp * region,
                                                        void *pBuffer) )
{
    uchar *Temp = pImage + step * seed.y;
    uchar *NL;
    uchar *TempRP;
    uchar *NLR;
    uchar *RP;
    uchar uv = (uchar)*_newVal;
    int d_lw = *_d_lw;
    int d_up = *_d_up;
    uchar TV;
    unsigned int Diap;
    Seg *stack;
    int StIn = 0;
    int i, j, YC, L, R, PL, PR, flag, k;
    int Sum = 0;
    unsigned int Interval = (unsigned int) (d_up + d_lw);
    int stepr = roi.width + 2;
    int XMin, XMax, YMin = seed.y, YMax = seed.y;

    if( pImage == NULL )
        return CV_NULLPTR_ERR;
    if( roi.width < 1 || roi.height < 1 )
        return CV_BADSIZE_ERR;
    if( roi.width > step )
        return CV_BADPOINT_ERR;
    if( seed.x < 0 || seed.x >= roi.width )
        return CV_BADPOINT_ERR;
    if( seed.y < 0 || seed.y >= roi.height )
        return CV_BADPOINT_ERR;
    if( d_lw < 0 || d_up < 0 )
        return CV_BADPOINT_ERR;
    if( *_newVal < 0 || *_newVal > 255 )
        return CV_BADPOINT_ERR;

    RP = (uchar *) pBuffer;
    stack = (Seg *) ((long) pBuffer + (roi.width + 2) * (roi.height + 2));
    memset( RP, 1, stepr );
    memset( RP + (roi.height + 1) * stepr, 1, stepr );
    for( i = 0; i < roi.height; i++ )
    {
        TempRP = RP + (i + 1) * stepr;
        TempRP[0] = 1;
        TempRP[stepr - 1] = 1;
        TempRP++;
        memset( TempRP, 0, stepr - 2 );

    }
    L = seed.x;
    R = seed.x;
    YC = seed.y;
    TempRP = RP + (YC + 1) * stepr + 1;
    TempRP[L] = 1;
    while( ((((unsigned) (Temp[R + 1] - Temp[R] + d_lw)) <= Interval)) && (!TempRP[R + 1]) )
    {
        TempRP[++R] = 1;
    }
    XMax = R;
    while( ((((unsigned) (Temp[L - 1] - Temp[L] + d_lw)) <= Interval)) && (!TempRP[L - 1]) )
    {

        TempRP[--L] = 1;
    }
    XMin = L;
    PL = MIN( R + 2, roi.width );
    PUSH( YC, L, R, PL, R + 2, UP );

    while( StIn )
    {
        POP( YC, L, R, PL, PR, flag );
        XMax = MAX( XMax, R );
        XMin = MIN( XMin, L );
        YMax = MAX( YMax, YC );
        YMin = MIN( YMin, YC );
        Diap = (unsigned) (R - L);
        Temp = pImage + YC * step;
        TempRP = RP + (YC + 1) * stepr + 1;
        NLR = TempRP - flag * stepr;
        NL = Temp - flag * step;
        for( i = L - 1; i < R + 2; i++ )
        {
            if( !(NLR[i]) )
            {
                TV = NL[i];
                if( 
                    (((unsigned) ((k = i - L) - 1) <= Diap) &&
                     (((unsigned) (TV - Temp[i - 1] + d_lw)) <= Interval)) ||
                    (((unsigned) (k) <= Diap) &&
                     (((unsigned) (TV - Temp[i] + d_lw)) <= Interval)) ||
                    (((unsigned) (++k) <= Diap) &&
                     (((unsigned) (TV - Temp[i + 1] + d_lw)) <= Interval)))
                {
                    NLR[i] = 1;
                    j = i;
                    while( (!NLR[j - 1]) &&
                           (((unsigned) (NL[j - 1] - NL[j] + d_lw)) <= Interval) )
                    {
                        NLR[--j] = 1;
                    }
                    while( (!NLR[++i]) &&
                           ((((unsigned) ((TV = NL[i]) - NL[i - 1] + d_lw)) <= Interval) ||
                            (((unsigned) ((k = i - L) - 1) <= Diap) &&
                             (((unsigned) (TV - Temp[i - 1] + d_lw)) <= Interval)) ||
                            (((unsigned) (k) <= Diap) &&
                             (((unsigned) (TV - Temp[i] + d_lw)) <= Interval)) ||
                            (((unsigned) (++k) <= Diap) &&
                             (((unsigned) (TV - Temp[i + 1] + d_lw)) <= Interval))))
                    {
                        NLR[i] = 1;
                    }
                    PUSH( YC - flag, j, i - 1, L, R, flag );
                }
            }
        }
        NLR = TempRP + flag * stepr;
        NL = Temp + flag * step;
        for( i = L - 1; i < PL; i++ )
        {
            if( !(NLR[i]) )
            {
                TV = NL[i];
                if( 
                    (((unsigned) ((k = i - L) - 1) <= Diap) &&
                     (((unsigned) (TV - Temp[i - 1] + d_lw)) <= Interval)) ||
                    (((unsigned) (k) <= Diap) &&
                     (((unsigned) (TV - Temp[i] + d_lw)) <= Interval)) ||
                    (((unsigned) (++k) <= Diap) &&
                     (((unsigned) (TV - Temp[i + 1] + d_lw)) <= Interval)))
                {
                    NLR[i] = 1;
                    j = i;
                    while( (!NLR[j - 1]) &&
                           (((unsigned) (NL[j - 1] - NL[j] + d_lw)) <= Interval) )
                    {
                        NLR[--j] = 1;
                    }

                    while( (!NLR[++i]) &&
                           ((((unsigned) ((TV = NL[i]) - NL[i - 1] + d_lw)) <= Interval) ||
                            (((unsigned) ((k = i - L) - 1) <= Diap) &&
                             (((unsigned) (TV - Temp[i - 1] + d_lw)) <= Interval)) ||
                            (((unsigned) (k) <= Diap) &&
                             (((unsigned) (TV - Temp[i] + d_lw)) <= Interval)) ||
                            (((unsigned) (++k) <= Diap) &&
                             (((unsigned) (TV - Temp[i + 1] + d_lw)) <= Interval))))
                    {
                        NLR[i] = 1;
                    }
                    PUSH( YC + flag, j, i - 1, L, R, -flag );
                }
            }
        }
        for( i = PR + 1; i < R + 2; i++ )
        {
            if( !(NLR[i]) )
            {
                TV = NL[i];
                if( 
                    (((unsigned) ((k = i - L) - 1) <= Diap) &&
                     (((unsigned) (TV - Temp[i - 1] + d_lw)) <= Interval)) ||
                    (((unsigned) (k) <= Diap) &&
                     (((unsigned) (TV - Temp[i] + d_lw)) <= Interval)) ||
                    (((unsigned) (++k) <= Diap) &&
                     (((unsigned) (TV - Temp[i + 1] + d_lw)) <= Interval)))
                {
                    NLR[i] = 1;
                    j = i;
                    while( (!NLR[j - 1]) &&
                           (((unsigned) (NL[j - 1] - NL[j] + d_lw)) <= Interval) )
                    {
                        NLR[--j] = 1;
                    }

                    while( (!NLR[++i]) &&
                           ((((unsigned) ((TV = NL[i]) - NL[i - 1] + d_lw)) <= Interval) ||
                            (((unsigned) ((k = i - L) - 1) <= Diap) &&
                             (((unsigned) (TV - Temp[i - 1] + d_lw)) <= Interval)) ||
                            (((unsigned) (k) <= Diap) &&
                             (((unsigned) (TV - Temp[i] + d_lw)) <= Interval)) ||
                            (((unsigned) (++k) <= Diap) &&
                             (((unsigned) (TV - Temp[i + 1] + d_lw)) <= Interval))))
                    {
                        NLR[i] = 1;
                    }
                    PUSH( YC + flag, j, i - 1, L, R, -flag );
                }
            }
        }
        for( i = L; i < R + 1; i++ )
            pImage[YC * step + i] = uv;
        Sum += (R - L + 1);
    }
    region->area = Sum;
    region->rect.x = XMin;
    region->rect.y = YMin;
    region->rect.width = XMax - XMin + 1;
    region->rect.height = YMax - YMin + 1;
    region->value = (float)*_newVal;
    return CV_NO_ERR;
}

/****************************************************************************************/
/*******************************************32fC1 flavor*********************************/
/****************************************************************************************/
IPCVAPI_IMPL( CvStatus, icvFloodFill_Grad8Con_32f_C1IR, (float *pImage,
                                                         int step,
                                                         CvSize roi,
                                                         CvPoint seed,
                                                         float* _newVal,
                                                         float* _d_lw,
                                                         float* _d_up,
                                                         CvConnectedComp * region,
                                                         void *pBuffer) )
{
    int ownstep = step / 4;
    float *Temp = pImage + ownstep * seed.y;
    float *NL;
    uchar *TempRP;
    uchar *NLR;
    uchar *RP;
    float newVal = *_newVal;
    float d_lw = *_d_lw;
    float d_up = *_d_up;
    float TV;
    float Addit = (d_lw - d_up) / 2;
    float Interval = (d_up + d_lw) / 2;
    Seg *stack;
    int StIn = 0;
    int i, j, YC, L, R, PL, PR, flag, k;
    int Sum = 0;
    int stepr = roi.width + 2;
    unsigned int Diap;
    int XMin, XMax, YMin = seed.y, YMax = seed.y;

    if( pImage == NULL )
        return CV_NULLPTR_ERR;
    if( roi.width < 1 || roi.height < 1 )
        return CV_BADSIZE_ERR;
    if( roi.width > step )
        return CV_BADPOINT_ERR;
    if( seed.x < 0 || seed.x >= roi.width )
        return CV_BADPOINT_ERR;
    if( seed.y < 0 || seed.y >= roi.height )
        return CV_BADPOINT_ERR;
    if( d_lw < 0 || d_up < 0 )
        return CV_BADPOINT_ERR;
    RP = (uchar *) pBuffer;
    stack = (Seg *) ((long) pBuffer + (roi.width + 2) * (roi.height + 2));
    memset( RP, 1, stepr );
    memset( RP + (roi.height + 1) * stepr, 1, stepr );
    for( i = 0; i < roi.height; i++ )
    {
        TempRP = RP + (i + 1) * stepr;
        TempRP[0] = 1;
        TempRP[stepr - 1] = 1;
        TempRP++;
        memset( TempRP, 0, stepr - 2 );

    }
    L = seed.x;
    R = seed.x;
    YC = seed.y;
    TempRP = RP + (YC + 1) * stepr + 1;
    TempRP[L] = 1;
    while( (fabs( Temp[R + 1] - Temp[R] + Addit ) <= Interval) && (!TempRP[R + 1]) )
    {
        TempRP[++R] = 1;
    }
    XMax = R;
    while( (fabs( Temp[L - 1] - Temp[L] + Addit ) <= Interval) && (!TempRP[L - 1]) )
    {

        TempRP[--L] = 1;
    }
    XMin = L;
    PL = MIN( R + 2, roi.width );
    PUSH( YC, L, R, PL, R + 2, UP );

    while( StIn )
    {
        POP( YC, L, R, PL, PR, flag );
        XMax = MAX( XMax, R );
        XMin = MIN( XMin, L );
        YMax = MAX( YMax, YC );
        YMin = MIN( YMin, YC );
        Diap = (unsigned) (R - L);
        Temp = pImage + YC * ownstep;
        TempRP = RP + (YC + 1) * stepr + 1;
        NLR = TempRP - flag * stepr;
        NL = Temp - flag * ownstep;
        for( i = L - 1; i < R + 2; i++ )
        {
            if( !(NLR[i]) )
            {
                TV = NL[i];
                if( 
                    (((unsigned) ((k = i - L) - 1) <= Diap) &&
                     (fabs( TV - Temp[i - 1] + Addit ) <= Interval)) ||
                    (((unsigned) (k) <= Diap) && (fabs( TV - Temp[i] + Addit ) <= Interval)) ||
                    (((unsigned) (++k) <= Diap) &&
                     (fabs( TV - Temp[i + 1] + Addit ) <= Interval)))
                {
                    NLR[i] = 1;
                    j = i;
                    while( (!NLR[j - 1]) && (fabs( NL[j - 1] - NL[j] + Addit ) <= Interval) )
                    {
                        NLR[--j] = 1;
                    }
                    while( (!NLR[++i]) &&
                           ((fabs( (TV = NL[i]) - NL[i - 1] + Addit ) <= Interval) ||
                            (((unsigned) ((k = i - L) - 1) <= Diap) &&
                             (fabs( TV - Temp[i - 1] + Addit ) <= Interval)) ||
                            (((unsigned) (k) <= Diap) &&
                             (fabs( TV - Temp[i] + Addit ) <= Interval)) ||
                            (((unsigned) (++k) <= Diap) &&
                             (fabs( TV - Temp[i + 1] + Addit ) <= Interval))))
                    {
                        NLR[i] = 1;
                    }
                    PUSH( YC - flag, j, i - 1, L, R, flag );
                }
            }
        }
        NLR = TempRP + flag * stepr;
        NL = Temp + flag * ownstep;
        for( i = L - 1; i < PL; i++ )
        {
            if( !(NLR[i]) )
            {
                TV = NL[i];
                if( 
                    (((unsigned) ((k = i - L) - 1) <= Diap) &&
                     (fabs( TV - Temp[i - 1] + Addit ) <= Interval)) ||
                    (((unsigned) (k) <= Diap) && (fabs( TV - Temp[i] + Addit ) <= Interval)) ||
                    (((unsigned) (++k) <= Diap) &&
                     (fabs( TV - Temp[i + 1] + Addit ) <= Interval)))
                {
                    NLR[i] = 1;
                    j = i;
                    while( (!NLR[j - 1]) && (fabs( NL[j - 1] - NL[j] + Addit ) <= Interval) )
                    {
                        NLR[--j] = 1;
                    }

                    while( (!NLR[++i]) &&
                           ((fabs( (TV = NL[i]) - NL[i - 1] + Addit ) <= Interval) ||
                            (((unsigned) ((k = i - L) - 1) <= Diap) &&
                             (fabs( TV - Temp[i - 1] + Addit ) <= Interval)) ||
                            (((unsigned) (k) <= Diap) &&
                             (fabs( TV - Temp[i] + Addit ) <= Interval)) ||
                            (((unsigned) (++k) <= Diap) &&
                             (fabs( TV - Temp[i + 1] + Addit ) <= Interval))))
                    {
                        NLR[i] = 1;
                    }
                    PUSH( YC + flag, j, i - 1, L, R, -flag );
                }
            }
        }
        for( i = PR + 1; i < R + 2; i++ )
        {
            if( !(NLR[i]) )
            {
                TV = NL[i];
                if( 
                    (((unsigned) ((k = i - L) - 1) <= Diap) &&
                     (fabs( TV - Temp[i - 1] + Addit ) <= Interval)) ||
                    (((unsigned) (k) <= Diap) && (fabs( TV - Temp[i] + Addit ) <= Interval)) ||
                    (((unsigned) (++k) <= Diap) &&
                     (fabs( TV - Temp[i + 1] + Addit ) <= Interval)))
                {
                    NLR[i] = 1;
                    j = i;
                    while( (!NLR[j - 1]) && (fabs( NL[j - 1] - NL[j] + Addit ) <= Interval) )
                    {
                        NLR[--j] = 1;
                    }

                    while( (!NLR[++i]) &&
                           ((fabs( (TV = NL[i]) - NL[i - 1] + Addit ) <= Interval) ||
                            (((unsigned) ((k = i - L) - 1) <= Diap) &&
                             (fabs( TV - Temp[i - 1] + Addit ) <= Interval)) ||
                            (((unsigned) (k) <= Diap) &&
                             (fabs( TV - Temp[i] + Addit ) <= Interval)) ||
                            (((unsigned) (++k) <= Diap) &&
                             (fabs( TV - Temp[i + 1] + Addit ) <= Interval))))
                    {
                        NLR[i] = 1;
                    }
                    PUSH( YC + flag, j, i - 1, L, R, -flag );
                }
            }
        }
        Sum += (R - L + 1);
        for( i = L; i < R + 1; i++ )
            pImage[YC * ownstep + i] = newVal;

    }
    region->area = Sum;
    region->rect.x = XMin;
    region->rect.y = YMin;
    region->rect.width = XMax - XMin + 1;
    region->rect.height = YMax - YMin + 1;
    region->value = newVal;
    return CV_NO_ERR;
}


/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name: cvFloodFill, cvFloodFill_8
//    Purpose: The functions fill the seed pixel environs inside which all pixel
//             values are not far from each other. 
//    Context:
//    Parameters:  img        - initial image (in the beginning)
//                              which is "repainted" during the function action,
//                 initPoint  - coordinates of the seed point inside image ROI,
//                 nv         - new value of repainted area pixels,
//                 lo_diff, up_diff - maximal lower and upper differences of the values of
//                              appurtenant to repainted area pixel and one of its
//                              neighbour,
//                 comp       - pointer to connected component structure of the
//                              repainted area
//
//    Notes:   The 1st function looks for 4-connected environs, the 2nd one - 8-connected.
//F*/


typedef  CvStatus (CV_STDCALL* CvFloodFillFunc)( void* img, int step, CvSize size,
                                                 CvPoint seed, void* newval,
                                                 CvConnectedComp* comp,
                                                 void* buffer );

typedef  CvStatus (CV_STDCALL* CvFloodFillGradFunc)( void* img, int step, CvSize size,
                                                     CvPoint seed, void* newval,
                                                     void* lo_diff, void* hi_diff,
                                                     CvConnectedComp* comp,
                                                     void* buffer );


static  void  icvInitFloodFill( void** ffill_tab,
                                void** ffillgrad_tab )
{
    ffill_tab[0] = (void*)icvFloodFill_4Con_8u_C1IR;
    ffill_tab[1] = (void*)icvFloodFill_8Con_8u_C1IR;
    ffill_tab[2] = (void*)icvFloodFill_4Con_32f_C1IR;
    ffill_tab[3] = (void*)icvFloodFill_8Con_32f_C1IR;

    ffillgrad_tab[0] = (void*)icvFloodFill_Grad4Con_8u_C1IR;
    ffillgrad_tab[1] = (void*)icvFloodFill_Grad8Con_8u_C1IR;
    ffillgrad_tab[2] = (void*)icvFloodFill_Grad4Con_32f_C1IR;
    ffillgrad_tab[3] = (void*)icvFloodFill_Grad8Con_32f_C1IR;
}


CV_IMPL void
cvFloodFill( void* arr, CvPoint seed_point,
             double newval, double lo_diff, double up_diff,
             CvConnectedComp *comp, int connectivity )
{
    static void* ffill_tab[4];
    static void* ffillgrad_tab[4];
    static int inittab = 0;

    void *buffer = 0;

    CV_FUNCNAME( "cvFloodFill" );

    __BEGIN__;

    int buf_size, is_simple, idx;
    double buf[12];
    void *nv_ptr = 0, *lo_diff_ptr = 0, *up_diff_ptr = 0;
    CvSize img_size;
    CvMat stub, *img = (CvMat*)arr;

    if( !inittab )
    {
        icvInitFloodFill( ffill_tab, ffillgrad_tab );
        inittab = 1;
    }

    CV_CALL( img = cvGetMat( img, &stub ));

    if( CV_ARR_TYPE( img->type ) != CV_8UC1 &&
        CV_ARR_TYPE( img->type ) != CV_32FC1 )
        CV_ERROR( CV_StsUnsupportedFormat, "" );

    if( connectivity != 4 && connectivity != 8 )
        CV_ERROR( CV_StsBadFlag, "Connectivity must be 4 or 8" );

    if( lo_diff < 0 || up_diff < 0 )
        CV_ERROR( CV_StsBadArg, "lo_diff and up_diff must be non-negative" );

    img_size = icvGetMatSize( img );

    if( lo_diff == 0 && up_diff == 0 )
    {
        is_simple = 1;
        IPPI_CALL( icvFloodFillGetSize( img_size, &buf_size ));
    }
    else
    {
        is_simple = 0;
        IPPI_CALL( icvFloodFillGetSize_Grad( img_size, &buf_size ));
    }
    
    CV_CALL( buffer = cvAlloc( buf_size ));

    idx = (CV_ARR_DEPTH( img->type ) == CV_32F)*2 + (connectivity == 8);

    if( CV_ARR_DEPTH( img->type ) < CV_32F )
    {
        int* ibuf = (int*)buf;
        int  val = cvRound( newval );
        nv_ptr = ibuf;
        ibuf[0] = val;

        val = cvRound( lo_diff );
        lo_diff_ptr = ibuf + 1;
        ibuf[1] = val;

        val = cvRound( up_diff );
        up_diff_ptr = ibuf + 2;
        ibuf[2] = val;
    }
    else
    {
        float* ibuf = (float*)buf;
        float  val = (float)newval;
        nv_ptr = ibuf;
        ibuf[0] = val;

        val = (float)lo_diff;
        lo_diff_ptr = ibuf + 1;
        ibuf[1] = val;

        val = (float)up_diff;
        up_diff_ptr = ibuf + 2;
        ibuf[2] = val;
    }

    if( is_simple )
    {
        CvFloodFillFunc func = (CvFloodFillFunc)(ffill_tab[idx]);
        assert( func != 0 );

        IPPI_CALL( func( img->data.ptr, img->step, img_size,
                         seed_point, nv_ptr, comp, buffer ));
    }
    else
    {
        CvFloodFillGradFunc func = (CvFloodFillGradFunc)(ffillgrad_tab[idx]);
        assert( func != 0 );

        IPPI_CALL( func( img->data.ptr, img->step, img_size,
                         seed_point, nv_ptr, lo_diff_ptr, up_diff_ptr, comp, buffer ));
    }

    __END__;

    cvFree( &buffer );
}

/* End of file. */
