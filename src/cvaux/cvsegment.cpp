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

#include "_cvaux.h"

typedef struct Seg
{
    int y;
    int l;
    int r;
    int Prevl;
    int Prevr;
    int fl;
}
Seg;

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


#define DIFF(p1,p2) ((unsigned)((p1)[0] - (p2)[0] + d_lw)<=Interval && \
                     (unsigned)((p1)[1] - (p2)[1] + d_lw)<=Interval && \
                     (unsigned)((p1)[2] - (p2)[2] + d_lw)<=Interval)


CvStatus  icvSegmFloodFill_Stage1( uchar* pImage, int step,
                                   uchar* pMask, int maskStep,
                                   CvSize /*roi*/, CvPoint seed,
                                   int* newVal, int d_lw, int d_up,
                                   CvConnectedComp * region,
                                   void *pStack )
{
    uchar* img = pImage + step * seed.y;
    uchar* mask = pMask + maskStep * (seed.y + 1);
    unsigned Interval = (unsigned) (d_up + d_lw);
    Seg *stack = (Seg*)pStack;
    int StIn = 0;
    int i, L, R; 
    int area = 0;
    int sum[] = { 0, 0, 0 };
    int XMin, XMax, YMin = seed.y, YMax = seed.y;
    int val0[3];

    L = seed.x;
    R = seed.x;
    img = pImage + seed.y*step;
    mask = pMask + seed.y*maskStep;
    mask[L] = 1;

    val0[0] = img[seed.x*3];
    val0[1] = img[seed.x*3 + 1];
    val0[2] = img[seed.x*3 + 2];

    while( DIFF( img + (R+1)*3, /*img + R*3*/val0 ) && !mask[R + 1] )
        mask[++R] = 2;

    while( DIFF( img + (L-1)*3, /*img + L*3*/val0 ) && !mask[L - 1] )
        mask[--L] = 2;

    XMax = R;
    XMin = L;
    PUSH( seed.y, L, R, R + 1, R, UP );

    while( StIn )
    {
        int k, YC, PL, PR, flag, curstep;

        POP( YC, L, R, PL, PR, flag );

        int data[][3] = { {-flag, L, R}, {flag, L, PL-1}, {flag,PR+1,R}};

        if( XMax < R )
            XMax = R;

        if( XMin > L )
            XMin = L;

        if( YMax < YC )
            YMax = YC;

        if( YMin > YC )
            YMin = YC;

        for( k = 0; k < 3; k++ )
        {
            flag = data[k][0];
            curstep = flag * step;
            img = pImage + (YC + flag) * step;
            mask = pMask + (YC + flag) * maskStep;
            int left = data[k][1];
            int right = data[k][2];

            for( i = left; i <= right; i++ )
            {
                if( !mask[i] && DIFF( img + i*3, /*img - curstep + i*3*/val0 ))
                {
                    int j = i;
                    mask[i] = 2;
                    while( !mask[j - 1] && DIFF( img + (j - 1)*3, /*img + j*3*/val0 ))
                        mask[--j] = 2;

                    while( !mask[i + 1] &&
                           (DIFF( img + (i+1)*3, /*img + i*3*/val0 ) ||
                           (DIFF( img + (i+1)*3, /*img + (i+1)*3 - curstep*/val0) && i < R)))
                        mask[++i] = 2;

                    PUSH( YC + flag, j, i, L, R, -flag );
                    i++;
                }
            }
        }
        
        img = pImage + YC * step;

        for( i = L; i <= R; i++ )
        {
            sum[0] += img[i*3];
            sum[1] += img[i*3 + 1];
            sum[2] += img[i*3 + 2];
        }

        area += R - L + 1;
    }
    
    region->area = area;
    region->rect.x = XMin;
    region->rect.y = YMin;
    region->rect.width = XMax - XMin + 1;
    region->rect.height = YMax - YMin + 1;
    region->value = 0;

    {
        double inv_area = area ? 1./area : 0;
        newVal[0] = cvRound( sum[0] * inv_area );
        newVal[1] = cvRound( sum[1] * inv_area );
        newVal[2] = cvRound( sum[2] * inv_area );
    }

    return CV_NO_ERR;
}


#undef PUSH
#undef POP
#undef DIFF


CvStatus  icvSegmFloodFill_Stage2( uchar* pImage, int step,
                                   uchar* pMask, int maskStep,
                                   CvSize /*roi*/, int* newVal,
                                   CvRect rect )
{
    uchar* img = pImage + step * rect.y + rect.x * 3;
    uchar* mask = pMask + maskStep * (rect.y + 1) + rect.x + 1;
    uchar uv[] = { (uchar)newVal[0], (uchar)newVal[1], (uchar)newVal[2] };
    int x, y;

    for( y = 0; y < rect.height; y++, img += step, mask += maskStep )
        for( x = 0; x < rect.width; x++ )
            if( mask[x] == 2 )
            {
                mask[x] = 1;
                img[x*3] = uv[0];
                img[x*3 + 1] = uv[1];
                img[x*3 + 2] = uv[2];
            }

    return CV_OK;
}


CV_IMPL void
cvSegmentImage( CvArr* srcarr, CvArr* dstarr,
                double canny_threshold, double ffill_threshold )
{
    CvMat* gray = 0;
    CvMat* canny = 0;
    void* stack = 0;
    
    CV_FUNCNAME( "cvSegmentImage" );

    __BEGIN__;

    CvMat srcstub, *src;
    CvMat dststub, *dst;
    CvMat* mask;
    CvSize size;
    CvPoint pt;
    int ffill_lw_up = cvRound( fabs(ffill_threshold) );

    CV_CALL( src = cvGetMat( srcarr, &srcstub ));
    CV_CALL( dst = cvGetMat( dstarr, &dststub ));

    if( src->data.ptr != dst->data.ptr )
    {
        CV_CALL( cvCopy( src, dst ));
        src = dst;
    }

    size = cvGetSize( src );

    CV_CALL( gray = cvCreateMat( size.height, size.width, CV_8UC1 ));
    CV_CALL( canny = cvCreateMat( size.height, size.width, CV_8UC1 ));

    CV_CALL( stack = cvAlloc( size.width * size.height * sizeof(Seg)));

    cvCvtColor( src, gray, CV_BGR2GRAY );
    cvCanny( gray, canny, 0, canny_threshold, 5 );

    mask = canny; // a new name for new role

    // make a non-zero border.
    cvRectangle( mask, cvPoint(0,0), cvPoint(size.width-1,size.height-1), 1, 1 );

    for( pt.y = 0; pt.y < size.height; pt.y++ )
    {
        for( pt.x = 0; pt.x < size.width; pt.x++ )
        {
            if( mask->data.ptr[mask->step*pt.y + pt.x] == 0 )
            {
                CvConnectedComp region;
                int avgVal[3] = { 0, 0, 0 };
                
                icvSegmFloodFill_Stage1( src->data.ptr, src->step,
                                         mask->data.ptr, mask->step,
                                         size, pt, avgVal,
                                         ffill_lw_up, ffill_lw_up,
                                         &region, stack );

                icvSegmFloodFill_Stage2( src->data.ptr, src->step,
                                         mask->data.ptr, mask->step,
                                         size, avgVal,
                                         region.rect );
            }
        }
    }

    __END__;

    cvReleaseMat( &gray );
    cvReleaseMat( &canny );
    cvFree( &stack );
}

/* End of file. */
