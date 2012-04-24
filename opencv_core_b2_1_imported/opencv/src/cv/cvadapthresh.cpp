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

/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:    _FindInitiThreshold
//    Purpose: Find initial threshold
//    Context:
//    Parameters:
//      src - pointer to source image data
//      size - size of scan window
//      MeanLine - buffer for keeping average of pixels in scan line
//      DispLine - buffer for keeping power of 2 of pixels in scan line
//
//    Returns:
//    Notes:
//F*/
void
_Read_Lines( uchar * src, int *MeanLine, int *DispLine, int width, int size )
{
    //////////// some variables //////////////
    int Disp;                   // dispersion
    int Mean;                   // mesan
    uchar *Beg;                 // pointer to first pixel on scan line
    uchar *End;                 // pointer to the pixel after last pixel of scan line
    uchar Val_1, Val_2;         // temp values ( value of depth in pixel )
    register int x;

    /////////// Init ///////////////
    Disp = Mean = 0;
    End = Beg = src;
    for( ; Beg < src + size; )
    {
        Val_1 = *Beg;
        Mean += Val_1;
        Disp += Val_1 * Val_1;

        Beg++;
    }                           // for(  ; Beg < src + size; )

    /////////////// calculating //////////////////////
    for(  /*register int */ x = 0; x < size; x++ )
    {

        Val_1 = *Beg;
        Mean += Val_1;
        Disp += Val_1 * Val_1;

        MeanLine[x] = Mean;
        DispLine[x] = Disp;

        Beg++;
    }                           // for( x = 0 ; x < size; x++ )

    for( ; x < (width - size); x++ )
    {
        Val_1 = *Beg;
        Val_2 = *End;

        Mean += (Val_1 - Val_2);
        Disp += (Val_1 * Val_1 - Val_2 * Val_2);

        MeanLine[x] = Mean;
        DispLine[x] = Disp;

        Beg++;
        End++;
    }                           // for(  ; x < ( width - size ); x++ )

    for( ; x < width; x++ )
    {
        Val_1 = *End;
        Mean -= Val_1;
        Disp -= Val_1 * Val_1;

        MeanLine[x] = Mean;
        DispLine[x] = Disp;

        End++;
    }                           // for( ; x < width; x++ )

}                               // CvStatus _Read_Lines( uchar* src, int* MeanLine, int* DispLine ,int y, int width , int size )

/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:    _FindInitiThreshold
//    Purpose: Find initial threshold
//    Context:
//    Parameters:
//      src - pointer to source image data
//      src_step - line step of src data
//      roi - image sizes
//      size - size of scan window
//      minDisp - min dispersion
//      thresh - pointer to thresholds
//
//    Returns:
//    Notes:
//F*/
CvStatus
_FindInitiThreshold( uchar * src, int src_step, CvSize roi,
                     int size, int minDisp, int *thresh )
{
    //////////////////////// Variables ////////////////////////
    uchar *cur_src_line;        // cyrent line of src image

    ///////////
    int height = roi.height;
    int width = roi.width;
    int y, x;                   // curent coordinates

    /////////////
    int Disp = 0;               // dispersion
    int Mean = 0;               // average
    int t1, t2, t3;             // temporaly variables

    /////////////
    int *Lines = NULL;          // pointer to allocated memory
    int *MeanLines;             // 2*size+1 lines buffer of intermediate result of average culculating
    int *DispLines;             // 2*size+1 lines buffer of intermediate result of dispersion culculating
    int *MeanLines_NextLine;    // pointer to second line in MeanLines buffer
    int *DispLines_NextLine;    // pointer to second line in DispLines buffer
    int *MeanLines_LastLine;    // pointer to last line in  MeanLines buffer
    int *DispLines_LastLine;    // pointer to last line in  DispLines buffer

    ///////////
    int *Means = NULL;          // buffer of calculating means ( for recoverage )
    int *Disps = NULL;          // buffer of calculating means ( for recoverage )

    ///////////
    int *p1, *p2;               // temporaly poiters

    ///////////
    int Y_Amount;               // amount pixels in y - cat of scan window

    /////////// Sizes
    int S_W;
    int S_W_W = (S_W = size * width) + width;
    int MemSize = 2 * (2 * S_W_W + width);
    int SizeOfLines = 2 * S_W_W;
    int LastLine;
    int SizeOfMove = (LastLine = 2 * S_W + width) * sizeof( int );

    /////////////// Allocating memory for lines
    Lines = new int[MemSize];

    if( Lines == NULL )
    {
        return CV_OUTOFMEM_ERR;

    }

    MeanLines = Lines;
    DispLines = Lines + SizeOfLines;

    MeanLines_NextLine = MeanLines + width;
    DispLines_NextLine = DispLines + width;

    MeanLines_LastLine = MeanLines + LastLine;
    DispLines_LastLine = DispLines + LastLine;

    Means = DispLines + SizeOfLines;
    Disps = Means + width;

    ////////////// Initialising ////////////////
    memset( Lines, 0, MemSize * sizeof( int ));

    cur_src_line = src;
    for( y = 0; y < S_W; y += width, cur_src_line += src_step )
    {
        _Read_Lines( cur_src_line, MeanLines + y, DispLines + y, width, size );
    }

    for( x = 0; x < width; x++ )
    {
        Mean = 0;
        Disp = 0;
        for( t1 = x; t1 < (x + S_W); t1 += width )
        {
            Mean += MeanLines[t1];
            Disp += DispLines[t1];
        }
        Means[x] = Mean;
        Disps[x] = Disp;
    }


    ////////////////// calculating /////////////////////////
    Y_Amount = size + 1;
    cur_src_line = src + size * src_step;
    t2 = height - size;
    t3 = width - size;
    for( y = 0; y < height; y++, cur_src_line += src_step )
    {
        /////////////// Move lines //////////////
        memmove( MeanLines_NextLine, MeanLines, SizeOfMove );
        memmove( DispLines_NextLine, DispLines, SizeOfMove );

        ////////////// Fill lines ///////////////
        if( y < t2 )
        {
            _Read_Lines( cur_src_line, MeanLines, DispLines, width, size );
        }                       // if( y < t2 )

        ///////////// calculating ///////////// 
        t1 = size * Y_Amount;
        p1 = Means;
        p2 = Disps;
        for( x = 0; x < width; x++, p1++, p2++ )
        {

            Mean = *p1;
            Disp = *p2;

            if( y < t2 )
            {
                Mean += MeanLines[x];
                Disp += DispLines[x];
            }                   // if( y < t2 )
            if( y > size )
            {

                Mean -= MeanLines_LastLine[x];
                Disp -= DispLines_LastLine[x];
            }                   // if( y > size )

            *p1 = Mean;
            *p2 = Disp;

            Mean = Mean / t1;
            Disp = (Disp / t1 - Mean * Mean);

            if( Disp > minDisp )
            {
                *thresh = Mean;
                delete[]Lines;
                return CV_NO_ERR;
            }                   // if( Disp > minDisp) 

            if( x < size )
            {
                t1 += Y_Amount;
            }                   // if( x < size )
            else if( x >= t3 )
            {
                t1 += Y_Amount;
            }                   //else if( x >=  t3  )

        }                       // for( x = 0 ; x < width ; x++ , p1++, p2++)

        if( y < size )
        {
            Y_Amount++;
        }                       // if( y < size )
        else if( y >= t2 )
        {
            Y_Amount--;
        }                       // else if( y >= t2 )

    }                           // for( y = 0; y < height; y++ , cur_src_line += src_step )

    delete[]Lines;
    return CV_NO_ERR;

}                               // CvStatus _FindInitiThreshold


CvStatus
_CalcDispMean( const CvMat* srcIm, CvMat* dispIm, CvMat* meanIm, int size )
{
    //////////// check input parametrs ///////////////////
    if( (srcIm == NULL) || (dispIm == NULL) || (meanIm == NULL) )
    {
        return CV_NULLPTR_ERR;
    }

    //////////// variables ///////////////
    uchar *src;
    int step_srcIm;
    int number = 2 * size + 1;
    CvSize roi_srcIm;

    IplImage*     AverageIm_f;
    IplImage*     Temp1_f;
    IplImage*     Temp2_f;
    _CvConvState* convState;

    /////////////// initialising /////////////////////////
    cvGetRawData( srcIm, &src, &step_srcIm, &roi_srcIm );

    //////////// creating images ///////////////////////
    Temp2_f = cvCreateImage( roi_srcIm, IPL_DEPTH_32F, 1 );
    AverageIm_f = cvCreateImage( roi_srcIm, IPL_DEPTH_32F, 1 );
    Temp1_f = cvCreateImage( roi_srcIm, IPL_DEPTH_32F, 1 );
    icvBlurInitAlloc( roi_srcIm.width, cv32f, number, &convState );

    if( (Temp2_f == NULL) || (AverageIm_f == NULL) || (Temp1_f == NULL) || !convState )
    {
        cvReleaseImage( &Temp2_f );
        cvReleaseImage( &AverageIm_f );
        cvReleaseImage( &Temp1_f );
        icvConvolFree( &convState );
        return CV_OUTOFMEM_ERR;
    }

    ////////////// calculating //////////////////////
    cvConvertScale( srcIm, Temp2_f, 1, 0 );
    icvBlur_32f_C1R( (float*)Temp2_f->imageData,     Temp2_f->widthStep,
                     (float*)AverageIm_f->imageData, AverageIm_f->widthStep,
                     &roi_srcIm, convState, 0 );
    cvMul( Temp2_f, Temp2_f, Temp1_f );      // square
    icvBlur_32f_C1R( (float*)Temp1_f->imageData, Temp1_f->widthStep,
                     (float*)Temp2_f->imageData, Temp2_f->widthStep,
                     &roi_srcIm, convState, 0 );

    cvSub( Temp2_f, AverageIm_f, Temp1_f );       // dispersion powered 2

    cvConvertScale( Temp2_f, dispIm, 1.0 / (number * number), 0 );    // convert to IPL_DEPTH_8U
    cvConvertScale( AverageIm_f, meanIm, 1.0 / (number * number), 0 );

    ///////////// relesing memory ///////////////////////////
    cvReleaseImage( &Temp1_f );
    cvReleaseImage( &Temp2_f );
    cvReleaseImage( &AverageIm_f );
    icvConvolFree( &convState );

    return CV_NO_ERR;

} // CvStatus _CalcDispMean( IplImage* srcIm, IplImage* dispIm, IplImage* meanIm , int size )


void
icvAdaptiveThreshold_StdDev( const CvMat* srcIm, CvMat* dstIm,
                             int maxValue, CvThreshType type,
                             int size, int epsilon )
{
    //////////////// Some variables  /////////////////////
    CvMat* avgIm = 0;
    CvMat* dispIm = 0;

    CvSize roi;

    uchar* src = 0;
    uchar* dst = 0;
    uchar* disp = 0;
    uchar* avg = 0;

    int  src_step;
    int  dst_step;
    int  disp_step;
    int  avg_step;

    int  thresh = 0;
    int  i, j;

    CV_FUNCNAME( "cvAdaptiveThreshold" );

    __BEGIN__;

    //////////////// Check for bad arguments ////////////////
    if( !srcIm || !dstIm )
        CV_ERROR( CV_StsNullPtr, "" );

    if( size < 1 || size > 4 )
        CV_ERROR( CV_StsBadSize, "" );

    cvGetRawData( srcIm, &src, &src_step, &roi );
    cvGetRawData( dstIm, &dst, &dst_step, 0 );

    CV_CALL( avgIm = cvCreateMat( roi.height, roi.width, CV_8UC1 ));
    CV_CALL( dispIm = cvCreateMat( roi.height, roi.width, CV_8UC1 ));

    // calc dispersion
    IPPI_CALL( _CalcDispMean( srcIm, avgIm, dispIm, size ));

    cvGetRawData( dispIm, &disp, &disp_step, 0 );
    cvGetRawData( avgIm, &avg, &avg_step, 0 );

    epsilon = epsilon * epsilon;

    _FindInitiThreshold( src, src_step, roi, size, epsilon, &thresh );

    switch (type)
    {
    case CV_THRESH_BINARY:
        for( i = 0; i < roi.height; i++, src += src_step,
             dst += dst_step, disp += disp_step, avg += avg_step )
        {
            for( j = 0; j < roi.width; j++ )
            {
                int tdisp = disp[j];

                if( tdisp > epsilon )
                    thresh = avg[j];

                dst[j] = (uchar)((thresh < src[j] ? -1 : 0) & maxValue);
            }
        }
        break;

    case CV_THRESH_BINARY_INV:
        for( i = 0; i < roi.height; i++, src += src_step,
             dst += dst_step, disp += disp_step, avg += avg_step )
        {
            for( j = 0; j < roi.width; j++ )
            {
                int tdisp = disp[j];

                if( tdisp > epsilon )
                    thresh = avg[j];

                dst[j] = (uchar)((src[j] < thresh ? -1 : 0) & maxValue);
            }
        }
        break;

    case CV_THRESH_TOZERO:
        for( i = 0; i < roi.height; i++, src += src_step,
             dst += dst_step, disp += disp_step, avg += avg_step )
        {
            for( j = 0; j < roi.width; j++ )
            {
                int t;
                int tdisp = disp[j];

                if( tdisp > epsilon )
                    thresh = avg[j];

                t = src[j];
                dst[j] = (uchar) ((thresh < t ? -1 : 0) & t);
            }
        }
        break;

    case CV_THRESH_TOZERO_INV:
        for( i = 0; i < roi.height; i++, src += src_step,
             dst += dst_step, disp += disp_step, avg += avg_step )
        {
            for( j = 0; j < roi.width; j++ )
            {
                int tdisp = disp[j];

                if( tdisp > epsilon )
                    thresh = avg[j];

                dst[j] = (uchar)((src[j] < thresh ? -1 : 0) & src[j]);
            }
        }
        break;

    default:
        CV_ERROR( CV_StsBadFlag, "" );
    }

    __END__;

    cvReleaseMat( &avgIm );
    cvReleaseMat( &dispIm );
}



/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:    cvAdaptiveThreshold
//    Purpose: Adaptive Thresholding the source image
//    Context:
//    Parameters:
//      srcIm     - source image
//      dstIm     - result thresholding image
//      maxValue       - the maximum value of the image pixel
//      method    - method for the adaptive threshold calculation 
//                  (now CV_STDDEF_ADAPTIVE_THRESH only)  
//      type      - thresholding type, must be one of
//                  CV_THRESH_BINARY       - val = (val > Thresh ? MAX    : 0)
//                  CV_THRESH_BINARY_INV   - val = (val > Thresh ? 0      : MAX)
//                  CV_THRESH_TOZERO       - val = (val > Thresh ? val    : 0)
//                  CV_THRESH_TOZERO_INV   - val = (val > Thresh ? 0      : val)
//      parameters - pointer to the input parameters (for the
//                   CV_STDDEF_ADAPTIVE_THRESH method parameters[0] is size of
//                   the neighborhood thresholding, (one of the 1-(3x3),2-(5x5),or
//                   3-(7x7)), parameters[1] is the value of the minimum variance
//    Returns:
//    Notes:
//F*/
CV_IMPL void
cvAdaptiveThreshold( const void *srcIm, void *dstIm,
                     double maxValue, CvAdaptiveThreshMethod method,
                     CvThreshType type, double *parameters )
{
    ///////////////// Some  variables /////////////////
    CvMat src_stub, dst_stub;
    CvMat *src = 0, *dst = 0;

    CV_FUNCNAME( "cvAdaptiveThreshold" );

    __BEGIN__;

    ///////////////// Checking /////////////////

    CV_CALL( src = cvGetMat( srcIm, &src_stub ));
    CV_CALL( dst = cvGetMat( dstIm, &dst_stub ));

    if( !CV_ARE_TYPES_EQ( src, dst ))
        CV_ERROR( CV_StsUnmatchedFormats, "" );

    if( CV_ARR_TYPE(src->type) != CV_8UC1 )
        CV_ERROR( CV_StsUnsupportedFormat, "" );

    if( !CV_ARE_SIZES_EQ( src, dst ) )
        CV_ERROR( CV_StsUnmatchedSizes, "" );

    switch (method)
    {
    case CV_STDDEV_ADAPTIVE_THRESH:
        icvAdaptiveThreshold_StdDev( src, dst, cvRound(maxValue),
                                     type, cvRound( parameters[0] ),
                                     cvRound( parameters[1] ));
        break;

    default:
        CV_ERROR_FROM_STATUS( CV_BADCOEF_ERR );
    }

    __END__;
}

/* End of file. */
