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
//    Name:    icvThreshold8uC1R
//    Purpose: Thresholding the source array
//    Context:
//    Parameters:
//      Src     - source array
//      roi     - size of picture in elements
//      srcStep - length of string
//      Thresh  - threshold parameter
//      Type    - thresholding type, must be one of
//                  CV_THRESH_BINARY       - val = (val > Thresh ? MAX    : 0)
//                  CV_THRESH_BINARY_INV   - val = (val > Thresh ? 0      : MAX)
//                  CV_THRESH_TRUNC        - val = (val > Thresh ? Thresh : val)
//                  CV_THRESH_TOZERO       - val = (val > Thresh ? val    : 0)
//                  CV_THRESH_TOZERO_INV   - val = (val > Thresh ? 0      : val)
//    Returns:
//    Notes:
//      The MAX constant for uchar is 255, for char is 127
//F*/
IPCVAPI_IMPL( CvStatus, icvThresh_8u_C1R, (const uchar * src,
                                           int src_step,
                                           uchar * dst,
                                           int dst_step,
                                           CvSize roi,
                                           int thresh, uchar maxval, CvThreshType type) )
{
    /* Some variables */
    int i, j;

    /* Check for bad arguments */
    assert( src && dst );
    if( thresh & ~255 )
        return CV_BADFACTOR_ERR;
    if( roi.width <= 0 || roi.height <= 0 )
        return CV_BADSIZE_ERR;
    if( roi.width > src_step )
        return CV_BADSIZE_ERR;
    if( roi.width > dst_step )
        return CV_BADSIZE_ERR;

    if( roi.width == src_step && roi.width == dst_step )
    {
        roi.width *= roi.height;
        roi.height = 1;
    }

    /* Calculating */
    switch (type)
    {
    case CV_THRESH_BINARY:
        for( i = 0; i < roi.height; i++, src += src_step, dst += dst_step )
            for( j = 0; j < roi.width; j++ )
                dst[j] = (uchar) (((thresh - src[j]) >> 8) & maxval);
        break;
    case CV_THRESH_BINARY_INV:
        for( i = 0; i < roi.height; i++, src += src_step, dst += dst_step )
            for( j = 0; j < roi.width; j++ )
                dst[j] = (uchar) (((src[j] - thresh - 1) >> 8) & maxval);
        break;
    case CV_THRESH_TRUNC:
        for( i = 0; i < roi.height; i++, src += src_step, dst += dst_step )
            for( j = 0; j < roi.width; j++ )
            {
                int temp = src[j] - thresh;

                dst[j] = (uchar) ((temp & (temp >> 31)) + thresh);
            }
        break;
    case CV_THRESH_TOZERO:
        for( i = 0; i < roi.height; i++, src += src_step, dst += dst_step )
            for( j = 0; j < roi.width; j++ )
            {
                int temp = src[j];

                dst[j] = (uchar) (((thresh - temp) >> 31) & temp);
            }
        break;
    case CV_THRESH_TOZERO_INV:
        for( i = 0; i < roi.height; i++, src += src_step, dst += dst_step )
            for( j = 0; j < roi.width; j++ )
            {
                int temp = src[j];

                dst[j] = (uchar) (((temp - thresh - 1) >> 31) & temp);
            }
        break;
    default:
        return CV_BADFLAG_ERR;
    }
    return CV_NO_ERR;
}


/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:    icvThreshold8sC1R
//    Purpose: Thresholding the source array
//    Context:
//    Parameters:
//      Src     - source array
//      roi     - size of picture in elements
//      srcStep - length of string
//      Thresh  - threshold parameter
//      Type    - thresholding type, must be one of
//                  CV_THRESH_BINARY       - val = (val > Thresh ? MAX    : 0)
//                  CV_THRESH_BINARY_INV   - val = (val > Thresh ? 0      : MAX)
//                  CV_THRESH_TRUNC        - val = (val > Thresh ? Thresh : val)
//                  CV_THRESH_TOZERO       - val = (val > Thresh ? val    : 0)
//                  CV_THRESH_TOZERO_INV   - val = (val > Thresh ? 0      : val)
//    Returns:
//    Notes:
//      The MAX constant for uchar is 255, for char is 127
//F*/
IPCVAPI_IMPL( CvStatus, icvThresh_8s_C1R, (const char *src,
                                           int src_step,
                                           char *dst,
                                           int dst_step,
                                           CvSize roi,
                                           int thresh, char maxval, CvThreshType type) )
{
    /* Some variables */
    int i, j;

    /* Check for bad arguments */
    if( !src || !dst )
        return CV_NULLPTR_ERR;
    if( thresh < -128 || thresh > 127 )
        return CV_BADFACTOR_ERR;
    if( roi.width <= 0 || roi.height <= 0 )
        return CV_BADSIZE_ERR;
    if( roi.width > src_step )
        return CV_BADSIZE_ERR;
    if( roi.width > dst_step )
        return CV_BADSIZE_ERR;

    if( roi.width == src_step && roi.width == dst_step )
    {
        roi.width *= roi.height;
        roi.height = 1;
    }

    /* Calculating */
    switch (type)
    {
    case CV_THRESH_BINARY:
        for( i = 0; i < roi.height; i++, src += src_step, dst += dst_step )
            for( j = 0; j < roi.width; j++ )
                dst[j] = (char) (((thresh - src[j]) >> 8) & maxval);
        break;
    case CV_THRESH_BINARY_INV:
        for( i = 0; i < roi.height; i++, src += src_step, dst += dst_step )
            for( j = 0; j < roi.width; j++ )
                dst[j] = (char) (((src[j] - thresh - 1) >> 8) & maxval);
        break;
    case CV_THRESH_TRUNC:
        for( i = 0; i < roi.height; i++, src += src_step, dst += dst_step )
            for( j = 0; j < roi.width; j++ )
            {
                int temp = src[j] - thresh;

                dst[j] = (char) ((temp & (temp >> 31)) + thresh);
            }
        break;
    case CV_THRESH_TOZERO:
        for( i = 0; i < roi.height; i++, src += src_step, dst += dst_step )
            for( j = 0; j < roi.width; j++ )
            {
                int temp = src[j];

                dst[j] = (char) (((thresh - temp) >> 31) & temp);
            }
        break;
    case CV_THRESH_TOZERO_INV:
        for( i = 0; i < roi.height; i++, src += src_step, dst += dst_step )
            for( j = 0; j < roi.width; j++ )
            {
                int temp = src[j];

                dst[j] = (char) (((temp - thresh - 1) >> 31) & temp);
            }
        break;
    default:
        return CV_BADFLAG_ERR;
    }
    return CV_NO_ERR;
}


/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:    icvThreshold32fC1R
//    Purpose: Thresholding the source array
//    Context:
//    Parameters:
//      Src     - source array
//      roi     - size of picture in elements
//      srcStep - length of string
//      Thresh  - threshold parameter
//      Type    - thresholding type, must be one of
//                  CV_THRESH_BINARY       - val = (val > Thresh ? MAX    : 0)
//                  CV_THRESH_BINARY_INV   - val = (val > Thresh ? 0      : MAX)
//                  CV_THRESH_TRUNC        - val = (val > Thresh ? Thresh : val)
//                  CV_THRESH_TOZERO       - val = (val > Thresh ? val    : 0)
//                  CV_THRESH_TOZERO_INV   - val = (val > Thresh ? 0      : val)
//    Returns:
//    Notes:
//      The MAX constant for uchar is 255, for char is 127
//F*/
IPCVAPI_IMPL( CvStatus, icvThresh_32f_C1R, (const float *src,
                                            int src_step,
                                            float *dst,
                                            int dst_step,
                                            CvSize roi,
                                            float thresh, float maxval, CvThreshType type) )
{
    /* Some variables */
    int i, j;
    int iThresh = CV_TOGGLE_FLT( (int &) thresh );
    int iMax = (int &) maxval;
    int *isrc = (int *) src;
    int *idst = (int *) dst;

    assert( sizeof( int ) == sizeof( float ));

    /* Check for bad arguments */
    if( !src || !dst )
        return CV_NULLPTR_ERR;
    if( roi.width < 0 || roi.height < 0 )
        return CV_BADSIZE_ERR;
    if( roi.width * sizeof_float > src_step )
        return CV_BADSIZE_ERR;
    if( roi.width * sizeof_float > dst_step )
        return CV_BADSIZE_ERR;
    if( (src_step & (sizeof_float - 1)) != 0 || (dst_step & (sizeof_float - 1)) != 0 )
        return CV_BADSIZE_ERR;

    src_step /= sizeof_float;
    dst_step /= sizeof_float;

    if( roi.width == src_step && roi.width == dst_step )
    {
        roi.width *= roi.height;
        roi.height = 1;
    }

    /* Calculating */
    switch (type)
    {
    case CV_THRESH_BINARY:
        for( i = 0; i < roi.height; i++, isrc += src_step, idst += dst_step )
        {
            for( j = 0; j < roi.width; j++ )
            {
                int temp = isrc[j];

                idst[j] = ((CV_TOGGLE_FLT( temp ) <= iThresh) - 1) & iMax;
            }
        }
        break;

    case CV_THRESH_BINARY_INV:
        for( i = 0; i < roi.height; i++, isrc += src_step, idst += dst_step )
        {
            for( j = 0; j < roi.width; j++ )
            {
                int temp = isrc[j];

                idst[j] = CV_TOGGLE_FLT( temp ) > iThresh ? 0 : iMax;
            }
        }
        break;

    case CV_THRESH_TRUNC:
        for( i = 0; i < roi.height; i++, src += src_step, dst += dst_step )
        {
            for( j = 0; j < roi.width; j++ )
            {
                float temp = src[j];

                if( temp > thresh )
                    temp = thresh;
                dst[j] = temp;
            }
        }
        break;

    case CV_THRESH_TOZERO:
        for( i = 0; i < roi.height; i++, isrc += src_step, idst += dst_step )
        {
            for( j = 0; j < roi.width; j++ )
            {
                int temp = isrc[j];

                idst[j] = ((CV_TOGGLE_FLT( temp ) <= iThresh) - 1) & temp;
            }
        }
        break;

    case CV_THRESH_TOZERO_INV:
        for( i = 0; i < roi.height; i++, isrc += src_step, idst += dst_step )
        {
            for( j = 0; j < roi.width; j++ )
            {
                int temp = isrc[j];

                idst[j] = CV_TOGGLE_FLT( temp ) > iThresh ? 0 : temp;
            }
        }
        break;

    default:
        return CV_BADFLAG_ERR;
    }

    return CV_OK;
}


CV_IMPL void
cvThreshold( const void* srcIm, void* dstIm, double thresh, double maxval, CvThreshType type )
{
    CvSize roi;
    CvSize roi_dst;
    int step;
    int step_dst;
    uchar *src_data = 0;
    uchar *dst_data = 0;
    CvMat src_stub, dst_stub;
    CvMat *src = 0, *dst = 0;

    CV_FUNCNAME( "cvThreshold" );

    __BEGIN__;

    CV_CALL( src = cvGetMat( srcIm, &src_stub ));
    CV_CALL( dst = cvGetMat( dstIm, &dst_stub ));

    if( !CV_ARE_DEPTHS_EQ( src, dst ))
        CV_ERROR( IPL_BadDepth, "inconsistent depths" );

    if( !CV_ARE_CNS_EQ( src, dst ) || CV_ARR_CN(src->type) != 1 )
        CV_ERROR( IPL_BadNumChannels, "bad channels numbers" );

    if( !CV_ARE_SIZES_EQ( src, dst ) )
    {
        CV_ERROR( IPL_BadImageSize, "unequal ROI's" );
    }

    cvGetRawData( src, &src_data, &step, &roi );
    cvGetRawData( dst, &dst_data, &step_dst, &roi_dst );

    switch( CV_ARR_DEPTH(src->type) )
    {
    case CV_8U:
        IPPI_CALL( icvThresh_8u_C1R( src_data, step, dst_data, step_dst, roi,
                                     cvRound(thresh), (uchar)cvRound( maxval ), type ));
        break;
    case CV_8S:
        IPPI_CALL( icvThresh_8s_C1R( (char*)src_data, step, (char*)dst_data, step_dst, roi,
                                     cvRound( thresh ), (char)cvRound( maxval ), type ));
        break;
    case CV_32F:
        IPPI_CALL( icvThresh_32f_C1R( (float*) src_data, step,
                                      (float*) dst_data, step_dst, roi,
                                      (float)thresh, (float)maxval, type ));
        break;
    default:
        CV_ERROR( IPL_BadDepth, icvUnsupportedFormat );
    }

    __END__;
}


/* End of file. */
