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

/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:      ippiCanny8uC1R     
//    Purpose:   finds edges on image using J.Canny algorithm
//    Context:   
//    Parameters:
//               src - source image,
//               srcStep - its step,
//               dst - destination binary image with edges,
//               dstStep - its step,
//               roi - size of ROI,
//               opSize - size of Sobel operator aperture, 
//               lowThreshold, 
//               highThreshold - tresholds for hysteresis thresholding  
//                 
//    Returns:   
//    Notes: image gradient magnitude has scale factor 2^(2*opSize-1)
//           so user must choose appropriate lowThreshold and highThreshold 
//           i.e. if real gradient magnitude is 1, then 3x3 Sobel used in this function 
//           will compute 8 //opSize is 2//
//F*/

IPCVAPI_IMPL( CvStatus, icvCannyGetSize, (CvSize roi, int *bufferSize) )
{
    if( (roi.width <= 0) && (roi.height <= 0) )
        return CV_BADSIZE_ERR;
    if( !bufferSize )
        return CV_NULLPTR_ERR;
    (*bufferSize) = (roi.height + 2) * (roi.width + 2) *
                    (sizeof(int) + sizeof(uchar) + sizeof(void*));
    return CV_NO_ERR;
}


IPCVAPI_IMPL( CvStatus,
icvCanny_16s8u_C1R, ( const short *pDX, int dxStep,
                      const short *pDY, int dyStep,
                      uchar *dst, int dststep,
                      CvSize roi, float lowThreshold,
                      float highThreshold, void *buffer ))
{
    static const int sec_tab[] = { 1, 3, 0, 0, 2, 2, 2, 2 };

    int stack_count = 0;
    int low = cvRound( lowThreshold );
    int high = cvRound( highThreshold );
    uchar* sector = (uchar*)buffer;
    int* mag = (int*)(sector + roi.width * roi.height);
    int magstep = roi.width + 2;
    void** stack = (void**)(mag + magstep*(roi.height + 2));
    int i, j;

    /* Check Bad Arguments */
    if( !pDX || !pDY || !dst || !stack  )
        return CV_NULLPTR_ERR;

    if( (roi.width <= 0) || (roi.height <= 0) )
        return CV_BADSIZE_ERR;

    memset( mag, 0, magstep * sizeof(mag[0]));
    memset( mag + magstep * (roi.height + 1), 0, magstep * sizeof(mag[0]));
    mag++;

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
    for( i = 0; i < roi.height; i++, (char*&)pDX += dxStep,
                                     (char*&)pDY += dyStep,
                                     sector += roi.width )
    {
        mag += magstep;
        mag[-1] = mag[roi.width] = 0;

        for( j = 0; j < roi.width; j++ )
        {
            int x = pDX[j];
            int y = pDY[j];
            int s = x ^ y;

            int m = (x >= 0) - 1;
            x = (x ^ m) - m;

            m = (y >= 0) - 1;
            y = (y ^ m) - m;

            m = x + y;

            /* estimating sector and magnitude */
            if( m > low )
            {
                #define CANNY_SHIFT 12
                #define TG22  (int)(0.4142135623730950488016887242097*(1<<CANNY_SHIFT) + 0.5)

                mag[j] = m;
                
                int tg22x = x * TG22;
                int tg67x = tg22x + ((x + x) << CANNY_SHIFT);

                y <<= CANNY_SHIFT;

                sector[j] = (uchar)sec_tab[(y > tg67x)*4 + (y < tg22x)*2 + (s < 0)];

                #undef CANNY_SHIFT
                #undef TG22
            }
            else
            {
                mag[j+1] = 0;
                sector[j] = 0;
            }
        }
    }

    mag -= magstep * roi.height;
    sector -= roi.width * roi.height;

    #define PUSH2(d,m)                      \
    {                                       \
        stack[stack_count] = (void*)(d);    \
        stack[stack_count+1] = (void*)(m);  \
        stack_count += 2;                   \
    }

    #define POP2(d,m)                       \
    {                                       \
        stack_count -= 2;                   \
        (d) = (uchar*)stack[stack_count];   \
        (m) = (int*)stack[stack_count+1];   \
    }

    /* /////////////////////////// non-maxima suppresion ///////////////////////// */
    {
        int shift[4];
        
        /* shift array init */
        shift[0] = 1;
        shift[1] = magstep + 1;
        shift[2] = magstep;
        shift[3] = magstep - 1;

        for( i = 0; i < roi.height; i++, sector += roi.width, dst += dststep )
        {
            mag += magstep;
            memset( dst, 0, roi.width );
       
            for( j = 0; j < roi.width; j++ )
            {
                int* center = mag + j;
                int val = *center;
            
                if( val )
                {
                    int delta = shift[sector[j]];

                    if( val > center[delta] && val > (center[-delta] & INT_MAX))
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
    }

    dst -= dststep * roi.height;
    mag -= magstep * roi.height;
    sector -= roi.width * roi.height;

    ///////////////////////  Hysteresis thresholding /////////////////////
    while( stack_count )
    {
        uchar* d;
        int* m;

        POP2( d, m );
        
        if( *d == 0 )
        {
            *d = 255;

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
    _CvConvState *pX = 0;
    _CvConvState *pY = 0;

    CV_FUNCNAME( "cvCanny" );

    __BEGIN__;

    CvMat srcstub, *src = (CvMat*)srcarr;
    CvMat dststub, *dst = (CvMat*)dstarr;
    CvSize src_size;
    int buf_size, origin = 0;

    CV_CALL( src = cvGetMat( src, &srcstub ));
    CV_CALL( dst = cvGetMat( dst, &dststub ));

    if( _CV_IS_IMAGE( srcarr ))
    {
        origin = ((IplImage*)srcarr)->origin;
    }

    if( CV_ARR_TYPE( src->type ) != CV_8UC1 ||
        CV_ARR_TYPE( dst->type ) != CV_8UC1 )
        CV_ERROR( CV_StsUnsupportedFormat, "" );

    if( !CV_ARE_SIZES_EQ( src, dst ))
        CV_ERROR( CV_StsUnmatchedSizes, "" );

    if( low_thresh > high_thresh )
        CV_ERROR( CV_StsBadFlag, "" );

    if( (aperture_size & 1) == 0 || aperture_size < 3 || aperture_size > 7 )
        CV_ERROR( CV_StsBadFlag, "" );

    src_size = icvGetMatSize( src );

    dx = cvCreateMat( src_size.height, src_size.width, CV_16SC1 );
    dy = cvCreateMat( src_size.height, src_size.width, CV_16SC1 );

    IPPI_CALL( icvCannyGetSize( src_size, &buf_size ));
    CV_CALL( buffer = cvAlloc( buf_size ));

    IPPI_CALL( icvSobelInitAlloc( src_size.width, cv8u,
                                  aperture_size, origin, 1, 0, &pX ));
    IPPI_CALL( icvSobelInitAlloc( src_size.width, cv8u,
                                  aperture_size, origin, 0, 1, &pY ));

    IPPI_CALL( icvSobel_8u16s_C1R( src->data.ptr, src->step,
                                   (short*)dx->data.ptr, dx->step,
                                   &src_size, pX, 0 ));
    IPPI_CALL( icvSobel_8u16s_C1R( src->data.ptr, src->step,
                                   (short*)dy->data.ptr, dy->step,
                                   &src_size, pY, 0 ));

    IPPI_CALL( icvCanny_16s8u_C1R( (short*)dx->data.ptr, dx->step,
                                   (short*)dy->data.ptr, dy->step,
                                   dst->data.ptr, dst->step,
                                   src_size, (float)low_thresh,
                                   (float)high_thresh, buffer ));

    __END__;

    cvReleaseMat( &dx );
    cvReleaseMat( &dy );
    cvFree( &buffer );
    
    icvConvolFree( &pX );
    icvConvolFree( &pY );
}


/* End of file. */
