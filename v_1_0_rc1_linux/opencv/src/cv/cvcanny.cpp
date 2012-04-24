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

icvCannyGetSize_t icvCannyGetSize_p = 0;
icvCanny_16s8u_C1R_t icvCanny_16s8u_C1R_p = 0;


static CvStatus CV_STDCALL
icvCanny( const short *dx, int dxstep, const short *dy, int dystep,
          uchar *dst, int dststep, CvSize roi, float low_threshold,
          float high_threshold, void *buffer, int flags )
{
    static const int sec_tab[] = { 1, 3, 0, 0, 2, 2, 2, 2 };
    int stack_count = 0;
    int low = cvRound( low_threshold );
    int high = cvRound( high_threshold );
    uchar* sector = (uchar*)buffer;
    int sectorstep = roi.width;
    int* mag = (int*)(sector + roi.width * roi.height);
    int magstep = roi.width + 2;
    void** stack = (void**)(mag + magstep*(roi.height + 2));
    int i, j;
    int l2_gradient = flags & CV_CANNY_L2_GRADIENT;
    int mshift[4];

    if( !dx || !dy || !dst || !stack  )
        return CV_NULLPTR_ERR;

    if( (roi.width <= 0) || (roi.height <= 0) )
        return CV_BADSIZE_ERR;

    memset( mag, 0, magstep * sizeof(mag[0]));
    memset( mag + magstep * (roi.height + 1), 0, magstep * sizeof(mag[0]));
    mag += magstep + 1;

    dxstep /= sizeof(dx[0]);
    dystep /= sizeof(dy[0]);

    /* sector numbers 
       (Top-Left Origin)

        1   2   3
         *  *  * 
          * * *  
        0*******0
          * * *  
         *  *  * 
        3   2   1
    */

    /* ///////////////////// calculate magnitude and angle of gradient ///////////////// */
    for( i = 0; i < roi.height; i++, dx += dxstep, dy += dystep,
                                     sector += sectorstep, mag += magstep )
    {
        mag[-1] = mag[roi.width] = 0;

        if( !l2_gradient )
            for( j = 0; j < roi.width; j++ )
                mag[j] = abs(dx[j]) + abs(dy[j]);
        else
            for( j = 0; j < roi.width; j++ )
            {
                int x = dx[j];
                int y = dy[j];
                mag[j] = cvRound(sqrt((double)x*x + (double)y*y));
            }

        for( j = 0; j < roi.width; j++ )
        {
            int x = dx[j];
            int y = dy[j];
            int s = x ^ y;
            int m = mag[j];

            x = abs(x);
            y = abs(y);

            /* estimating sector and magnitude */
            if( m > low )
            {
                #define CANNY_SHIFT 12
                #define TG22  (int)(0.4142135623730950488016887242097*(1<<CANNY_SHIFT) + 0.5)

                int tg22x = x * TG22;
                int tg67x = tg22x + ((x + x) << CANNY_SHIFT);

                y <<= CANNY_SHIFT;

                sector[j] = (uchar)sec_tab[(y > tg67x)*4 + (y < tg22x)*2 + (s < 0)];

                #undef CANNY_SHIFT
                #undef TG22
            }
            else
            {
                mag[j] = 0;
                sector[j] = 0;
            }
        }
    }

    mag -= magstep * roi.height;
    sector -= sectorstep * roi.height;

    #define PUSH2(d,m)                      \
    {                                       \
        stack[stack_count] = (void*)(d);    \
        stack[stack_count+1] = (void*)(m);  \
        stack_count += 2;                   \
        *(d)=255;                           \
    }

    #define POP2(d,m)                       \
    {                                       \
        stack_count -= 2;                   \
        (d) = (uchar*)stack[stack_count];   \
        (m) = (int*)stack[stack_count+1];   \
    }

    /////////////////////////// non-maxima suppresion /////////////////////////
    mshift[0] = 1;
    mshift[1] = magstep + 1;
    mshift[2] = magstep;
    mshift[3] = magstep - 1;

    for( i = 0; i < roi.height; i++, sector += sectorstep,
                           mag += magstep, dst += dststep )
    {
        memset( dst, 0, roi.width );
   
        for( j = 0; j < roi.width; j++ )
        {
            int* center = mag + j;
            int val = *center;
        
            if( val )
            {
                int sec = sector[j];
                int delta = mshift[sec];
                int b = center[delta];
                int c = center[-delta] & INT_MAX;
                
                if( val > c && (val > b ||
                    sec == 0 && val == b ||
                    sec == 2 && val == b) )
                {
                    if( val > high )
                        PUSH2( dst + j, mag + j );
                }
                else
                {
                    *center = val | INT_MIN;
                }
            }
        }
    }

    ///////////////////////  Hysteresis thresholding /////////////////////
    while( stack_count )
    {
        uchar* d;
        int* m;

        POP2( d, m );
    
        if( m[-1] > low && d[-1] == 0 )
            PUSH2( d - 1, m - 1 );
        if( m[1] > low && d[1] == 0 )
            PUSH2( d + 1, m + 1 );
        if( m[-magstep-1] > low && d[-dststep-1] == 0 )
            PUSH2( d - dststep - 1, m - magstep - 1 );
        if( m[-magstep] > low && d[-dststep] == 0 )
            PUSH2( d - dststep, m - magstep );
        if( m[-magstep+1] > low && d[-dststep+1] == 0 )
            PUSH2( d - dststep + 1, m - magstep + 1 );
        if( m[magstep-1] > low && d[dststep-1] == 0 )
            PUSH2( d + dststep - 1, m + magstep - 1 );
        if( m[magstep] > low && d[dststep] == 0 )
            PUSH2( d + dststep, m + magstep );
        if( m[magstep+1] > low && d[dststep+1] == 0 )
            PUSH2( d + dststep + 1, m + magstep + 1 );
    }
    #undef PUSH2
    #undef POP2

    return CV_OK;
}

CV_IMPL void
cvCanny( const void* srcarr, void* dstarr,
         double low_thresh, double high_thresh, int aperture_size )
{
    CvMat *dx = 0, *dy = 0;
    void *buffer = 0;

    CV_FUNCNAME( "cvCanny" );

    __BEGIN__;

    CvMat srcstub, *src = (CvMat*)srcarr;
    CvMat dststub, *dst = (CvMat*)dstarr;
    CvSize size;
    int flags = aperture_size;

    CV_CALL( src = cvGetMat( src, &srcstub ));
    CV_CALL( dst = cvGetMat( dst, &dststub ));

    if( CV_MAT_TYPE( src->type ) != CV_8UC1 ||
        CV_MAT_TYPE( dst->type ) != CV_8UC1 )
        CV_ERROR( CV_StsUnsupportedFormat, "" );

    if( !CV_ARE_SIZES_EQ( src, dst ))
        CV_ERROR( CV_StsUnmatchedSizes, "" );

    if( low_thresh > high_thresh )
    {
        double t;
        CV_SWAP( low_thresh, high_thresh, t );
    }

    aperture_size &= INT_MAX;
    if( (aperture_size & 1) == 0 || aperture_size < 3 || aperture_size > 7 )
        CV_ERROR( CV_StsBadFlag, "" );

    size = cvGetMatSize( src );

    dx = cvCreateMat( size.height, size.width, CV_16SC1 );
    dy = cvCreateMat( size.height, size.width, CV_16SC1 );
    cvSobel( src, dx, 1, 0, aperture_size );
    cvSobel( src, dy, 0, 1, aperture_size );

    if( icvCannyGetSize_p && icvCanny_16s8u_C1R_p && !(flags & CV_CANNY_L2_GRADIENT) )
    {
        int buf_size=  0;
        IPPI_CALL( icvCannyGetSize_p( size, &buf_size ));
        CV_CALL( buffer = cvAlloc( buf_size ));
        IPPI_CALL( icvCanny_16s8u_C1R_p( (short*)dx->data.ptr, dx->step,
                                     (short*)dy->data.ptr, dy->step,
                                     dst->data.ptr, dst->step,
                                     size, (float)low_thresh,
                                     (float)high_thresh, buffer ));
        EXIT;
    }

    CV_CALL( buffer = cvAlloc((size.height + 2)*(size.width + 2)*
            (sizeof(int) + sizeof(uchar) + sizeof(void*)*2)));

    icvCanny( (short*)dx->data.ptr, dx->step, (short*)dy->data.ptr, dy->step,
              dst->data.ptr, dst->step, size, (float)low_thresh,
              (float)high_thresh, buffer, flags );

    __END__;

    cvReleaseMat( &dx );
    cvReleaseMat( &dy );
    cvFree( &buffer );
}


/* End of file. */
