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
//    Name:    icvUpdateMHIByTime32fC1R
//    Purpose: The function updates motion history image.
//    Context:
//    Parameters:
//        silIm         - silhouette image
//        silStep       - its step
//        mhiIm         - motion history image
//        mhiStep       - its step
//        size          - size of both images (in pixels)
//        timestamp     - current system time (in seconds)
//        mhi_duration  - maximal duration of motion track before it will
//                        be removed (in seconds too)
//    Returns:
//        CV_OK or error code:
//           CV_NULLPTR_ERR - silIm or mhiIm pointer are null
//           CV_BADSIZE_ERR   - width or height is negative or steps less than width
//           CV_BADFACTOR_ERR - mhi_duration is non positive
//    Notes:
//F*/
IPCVAPI_IMPL( CvStatus, icvUpdateMotionHistory_8u32f_C1IR,
    (const uchar * silIm, int silStep, float *mhiIm, int mhiStep,
     CvSize size, float timestamp, float mhi_duration),
     (silIm, silStep, mhiIm, mhiStep, size, timestamp, mhi_duration) )
{
    int y;
    int delbound;

    /* function processes floating-point images using integer arithmetics */
    int ts = *((int *) &timestamp);
    int *mhi = (int *) mhiIm;

    if( !silIm || !mhiIm )
        return CV_NULLPTR_ERR;

    if( size.height <= 0 || size.width <= 0 ||
        silStep < size.width || mhiStep < size.width * CV_SIZEOF_FLOAT ||
        (mhiStep & (CV_SIZEOF_FLOAT - 1)) != 0 )
        return CV_BADSIZE_ERR;

    if( mhi_duration < 0 )
        return CV_BADFACTOR_ERR;

    mhi_duration = timestamp - mhi_duration;
    delbound = CV_TOGGLE_FLT( (*(int *) &mhi_duration) );

    mhiStep /= CV_SIZEOF_FLOAT;

    if( mhiStep == size.width && silStep == size.width )
    {
        size.width *= size.height;
        size.height = 1;
    }

    if( delbound > 0 )
        for( y = 0; y < size.height; y++, silIm += silStep, mhi += mhiStep )
        {
            int x;

            for( x = 0; x < size.width; x++ )
            {
                int val = mhi[x];

                /* val = silIm[x] ? ts : val < delbound ? 0 : val; */
                val &= (val < delbound) - 1;
                val ^= (ts ^ val) & ((silIm[x] == 0) - 1);
                mhi[x] = val;
            }
        }
    else
        for( y = 0; y < size.height; y++, silIm += silStep, mhi += mhiStep )
        {
            int x;

            for( x = 0; x < size.width; x++ )
            {
                int val = mhi[x];

                /* val = silIm[x] ? ts : val < delbound ? 0 : val; */
                val &= (CV_TOGGLE_FLT( val ) < delbound) - 1;
                val ^= (ts ^ val) & ((silIm[x] == 0) - 1);
                mhi[x] = val;
            }
        }

    return CV_OK;
}


/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:    icvCalcMotionGradient32fC1R
//    Purpose: The functions calculates motion gradient and mask where it is valid
//    Context:
//    Parameters:
//       mhi         - motion history image
//       mhiStep     - its step
//       mask        - mask(out): indicates where <orient> data is valid
//       maskStep    - its step
//       orient      - image containing gradient orientation(out) (in degrees)
//       orientStep  - its step
//       size        - size of the all images in pixels
//       aperture_size - size of the filters for Dx & Dy
//       maxTDelta   - gradient bounds.
//       minTDelta   _/
//       origin      - 0 - image data start from geometrical top (y coordinate increases)
//                     1 - image data start from geometrical bottom (y decreases)
//    Returns:
//        CV_OK or error code:
//        CV_NULLPTR_ERR - orient or mask or mhi pointers are null
//        CV_BADSIZE_ERR   - width or height is negative or steps less than width
//        CV_BADFACTOR_ERR - if sobel_order parameter is invalid (<2 or >8 or uneven) or
//                            minDelta >= maxDelta.
//    Notes:
//F*/
static CvStatus
icvCalcMotionGradient32fC1R( float *mhi, int mhiStep,
                             uchar * mask, int maskStep,
                             float *orient, int orientStep,
                             CvSize roi, int apertureSize,
                             float maxTDelta, float minTDelta, int origin )
{
    float gradientEps = 1e-4f * apertureSize;
    int iGradientEps = (int &) gradientEps;
    int iMinDelta = *(int *) &minTDelta;
    int iMaxDelta = *(int *) &maxTDelta;
    int x, y;
    CvMorphState *morph_filter;
    float *drvX_min;
    float *drvY_max;
    int tempStep;
    int tempSize;
    _CvConvState *pX;
    _CvConvState *pY;

    CvStatus result = CV_OK;

    /* check parameters */
    if( !orient || !mask || !mhi )
        return CV_NULLPTR_ERR;

    if( orient == mhi )
        return CV_INPLACE_NOT_SUPPORTED_ERR;

    if( roi.height <= 0 || roi.width <= 0 || orientStep < roi.width * 4 ||
        maskStep < roi.width || mhiStep < roi.width * 4 )
        return CV_BADSIZE_ERR;
    if( ((mhiStep | orientStep) & 3) != 0 )
        return CV_BADSIZE_ERR;

    tempStep = cvAlign(roi.width,2) * sizeof( float );

    tempSize = tempStep * roi.height;

    drvX_min = (float *) cvAlloc( tempSize );
    drvY_max = (float *) cvAlloc( tempSize );
    if( !drvX_min || !drvY_max )
    {
        result = CV_OUTOFMEM_ERR;
        goto func_exit;
    }

    {
    CvSize element_size = { apertureSize, apertureSize };
    CvPoint element_anchor = { apertureSize/2, apertureSize/2 };
    icvMorphologyInitAlloc( roi.width, cv32f, 1, element_size, element_anchor,
                            CV_SHAPE_RECT, 0, &morph_filter );
    }

    /* calc Dx and Dy */
    icvSobelInitAlloc( roi.width, cv32f, apertureSize, origin, 1, 0, &pX );
    result = icvSobel_32f_C1R( mhi, mhiStep, drvX_min, tempStep, &roi, pX, 0 );
    icvFilterFree( &pX );

    if( result < 0 )
        goto func_exit;
    icvSobelInitAlloc( roi.width, cv32f, apertureSize, origin, 0, 1, &pY );
    result = icvSobel_32f_C1R( mhi, mhiStep, drvY_max, tempStep, &roi, pY, 0 );
    icvFilterFree( &pY );

    if( result < 0 )
        goto func_exit;

    /* calc gradient */
    for( y = 0; y < roi.height; y++, drvX_min += tempStep / sizeof( float ),
         drvY_max += tempStep / sizeof( float ), orient += orientStep / sizeof( float ))
    {
        cvbFastArctan( drvY_max, drvX_min, orient, roi.width );

        /* make orientation zero where the gradient is very small */
        for( x = 0; x < roi.width; x++ )
        {
            int dY = ((int *) drvY_max)[x] & 0x7fffffff;
            int dX = ((int *) drvX_min)[x] & 0x7fffffff;

            ((int *) orient)[x] &= ((dX < iGradientEps) && (dY < iGradientEps)) - 1;
        }
    }

    drvX_min -= tempSize / sizeof( float );
    drvY_max -= tempSize / sizeof( float );
    orient -= (orientStep / sizeof( float )) * roi.height;

    result = icvErodeStrip_Rect_32f_C1R( (int*)mhi, mhiStep, (int*)drvX_min, tempStep,
                                         &roi, morph_filter, 0 );
    if( result < 0 )
        goto func_exit;

    result = icvDilateStrip_Rect_32f_C1R( (int*)mhi, mhiStep, (int*)drvY_max, tempStep,
                                           &roi, morph_filter, 0 );
    if( result < 0 )
        goto func_exit;

    /* mask off pixels which have little motion difference in their neighborhood */
    for( y = 0; y < roi.height; y++, drvX_min += tempStep / sizeof( float ),
         drvY_max += tempStep / sizeof( float ),
         orient += orientStep / sizeof( float ), mask += maskStep )
    {
        for( x = 0; x <= roi.width - 4; x += 4 )
        {
            float d0 = drvY_max[x] - drvX_min[x];
            float d1 = drvY_max[x + 1] - drvX_min[x + 1];
            float d2 = drvY_max[x + 2] - drvX_min[x + 2];
            float d3 = drvY_max[x + 3] - drvX_min[x + 3];

            mask[x] = (uchar) ((*(int *) &d0 >= iMinDelta) && (*(int *) &d0 <= iMaxDelta));
            mask[x + 1] = (uchar) ((*(int *) &d1 >= iMinDelta) && (*(int *) &d1 <= iMaxDelta));
            mask[x + 2] = (uchar) ((*(int *) &d2 >= iMinDelta) && (*(int *) &d2 <= iMaxDelta));
            mask[x + 3] = (uchar) ((*(int *) &d3 >= iMinDelta) && (*(int *) &d3 <= iMaxDelta));
            /*((int*)orient)[x] &= (mask[x] == 0) - 1;
               ((int*)orient)[x + 1] &= (mask[x + 1] == 0) - 1;
               ((int*)orient)[x + 2] &= (mask[x + 2] == 0) - 1;
               ((int*)orient)[x + 3] &= (mask[x + 3] == 0) - 1; */
        }

        for( ; x < roi.width; x++ )
        {
            float delta = drvY_max[x] - drvX_min[x];

            mask[x] = (uchar) ((delta >= minTDelta) && (delta <= maxTDelta));
            /*((int*)orient)[x] &= (mask[x] == 0) - 1; */
        }
    }


    drvX_min -= tempSize / sizeof( float );
    drvY_max -= tempSize / sizeof( float );

  func_exit:

    icvMorphologyFree( &morph_filter );
    cvFree( (void**)&drvX_min );
    cvFree( (void**)&drvY_max );

    return result;
}


/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:    icvCalcGlobalOrientation32fC1R
//    Purpose: The function calculates general motion direction in the selected region.
//    Context:
//    Parameters:
//         orient             - orientation image
//         orientStep         - its step
//         mask               - region mask
//         maskStep           - its step
//         mhi                - motion history image
//         mhiStep            - its step
//         size               - size of the all images in pixels
//         curr_mhi_timestamp - the last timestamp when mhi was updated
//         mhi_duration       - maximal motion track duration.
//         angle              - result: motion direction of the region
//    Returns:
//        CV_OK or error code:
//           CV_NULLPTR_ERR - orient or mask or mhi pointers are null
//           CV_BADSIZE_ERR   - width or height is negative or steps less than width
//           CV_BADFACTOR_ERR - mhi_duration is non positive
//    Notes:
//F*/
static CvStatus
icvCalcGlobalOrientation32fC1R( float *orient, int orientStep,
                                uchar * mask, int maskStep,
                                float *mhi, int mhiStep,
                                CvSize size,
                                float curr_mhi_timestamp, float mhi_duration, float *angle )
{
#define _CV_MT2_HIST_SIZE  12
    const double hist_scale = _CV_MT2_HIST_SIZE / 360.;
    float delbound = curr_mhi_timestamp - mhi_duration;
    int orient_hist[_CV_MT2_HIST_SIZE];
    int y;
    int base_orient = 0;

    if( !orient || !mask || !mhi || !angle )
        return CV_NULLPTR_ERR;

    if( orient == mhi )
        return CV_INPLACE_NOT_SUPPORTED_ERR;

    if( size.height <= 0 || size.width <= 0 ||
        orientStep < size.width * 4 || maskStep < size.width || mhiStep < size.width * 4 )
        return CV_BADSIZE_ERR;
    if( mhi_duration <= 0 )
        return CV_BADFACTOR_ERR;

    orientStep /= 4;
    mhiStep /= 4;

    memset( orient_hist, 0, sizeof( orient_hist ));

    /* 1. Calc orientation histogram */
    for( y = 0; y < size.height; y++ )
    {
        int x;

        for( x = 0; x < size.width; x++ )
            if( mask[x] != 0 )
            {
                int bin = cvRound( orient[x] * hist_scale );

                bin = CV_IMAX( bin, 0 );
                bin = CV_IMIN( bin, _CV_MT2_HIST_SIZE - 1 );
                orient_hist[bin]++;
            }
        orient += orientStep;
        mask += maskStep;
    }

    /* 2. find historgam maximum */
    {
        int max_bin_val = orient_hist[0];

        for( y = 1; y < _CV_MT2_HIST_SIZE; y++ )
        {
            int bin_val = orient_hist[y];
            int max_mask = (max_bin_val >= bin_val) - 1;

            /*
               if( !(max_bin_val > bin_val) )
               max_bin_val = bin_val, base_orient = y;
             */
            max_bin_val ^= (max_bin_val ^ bin_val) & max_mask;
            base_orient ^= (base_orient ^ y) & max_mask;
        }

        base_orient *= 30;
    }


    /* 3. find the shift as weighted sum of relative angles */
    {
        double shift_orient = 0, shift_weight = 0;
        double a = (254. / 255.) / mhi_duration;
        double b = 1. - curr_mhi_timestamp * a;
        double fbase_orient = base_orient;

        /*
           a = 254/(255*dt)
           b = 1 - t*a = 1 - 254*t/(255*dur) =
           (255*dt - 254*t)/(255*dt) =
           (dt - (t - dt)*254)/(255*dt);
           --------------------------------------------------------
           ax + b = 254*x/(255*dt) + (dt - (t - dt)*254)/(255*dt) =
           (254*x + dt - (t - dt)*254)/(255*dt) =
           ((x - (t - dt))*254 + dt)/(255*dt) =
           (((x - (t - dt))/dt)*254 + 1)/255 = (((x - low_time)/dt)*254 + 1)/255
         */

        orient -= size.height * orientStep;
        mask -= size.height * maskStep;

        for( y = 0; y < size.height; y++ )
        {
            int x;

            for( x = 0; x < size.width; x++ )
                if( mask[x] != 0 && mhi[x] > delbound )
                {
                    /*
                       orient in 0..360, base_orient in 0..360
                       -> (rel_angle = orient - base_orient) in -360..360.
                       rel_angle is translated to -180..180
                     */
                    double weight = mhi[x] * a + b;
                    int rel_angle = cvRound( orient[x] - fbase_orient );

                    rel_angle += (rel_angle < -180 ? 360 : 0);
                    rel_angle += (rel_angle > 180 ? -360 : 0);
                    shift_orient += weight * rel_angle;
                    shift_weight += weight;
                }

            orient += orientStep;
            mask += maskStep;
            mhi += mhiStep;
        }

        /* 4. add base and shift orientations and normalize result */

        /* set lowest mantissa's bit to avoid division by 0 */
        *((int *) &shift_weight) |= 1;

        base_orient = base_orient + cvRound( shift_orient / shift_weight );
    }

    base_orient -= (base_orient < 360 ? 0 : 360);
    base_orient += (base_orient >= 0 ? 0 : 360);

    *angle = (float) base_orient;
    return CV_OK;

#undef _CV_MT2_HIST_SIZE
}


/* motion templates */
CV_IMPL void
cvUpdateMotionHistory( const void* silhouette, void* mhimg,
                       double timestamp, double mhi_duration )
{
    CvSize size;
    CvMat  silhstub, *silh = (CvMat*)silhouette;
    CvMat  mhistub, *mhi = (CvMat*)mhimg;
    int mhi_step, silh_step;

    CV_FUNCNAME( "cvUpdateMHIByTime" );

    __BEGIN__;

    CV_CALL( silh = cvGetMat( silh, &silhstub ));
    CV_CALL( mhi = cvGetMat( mhi, &mhistub ));

    if( !CV_IS_MASK_ARR( silh ))
        CV_ERROR( CV_StsBadMask, "" );

    if( CV_MAT_CN( mhi->type ) > 1 )
        CV_ERROR( CV_BadNumChannels, "" );

    if( CV_MAT_DEPTH( mhi->type ) != CV_32F )
        CV_ERROR( CV_BadDepth, "" );

    if( !CV_ARE_SIZES_EQ( mhi, silh ))
        CV_ERROR( CV_StsUnmatchedSizes, "" );

    size = cvGetMatSize( mhi );

    mhi_step = mhi->step;
    silh_step = silh->step;

    if( CV_IS_MAT_CONT( mhi->type & silh->type ))
    {
        size.width *= size.height;
        mhi_step = silh_step = CV_STUB_STEP;
        size.height = 1;
    }

    IPPI_CALL( icvUpdateMotionHistory_8u32f_C1IR( (const uchar*)(silh->data.ptr), silh_step,
                                                  mhi->data.fl, mhi_step, size,
                                                  (float)timestamp, (float)mhi_duration ));
    __END__;
}


CV_IMPL void
cvCalcMotionGradient( const void* mhiimg, void* maskimg,
                      void* orientation,
                      double maxTDelta, double minTDelta,
                      int apertureSize )
{
    CvSize  size;
    CvMat  mhistub, *mhi = (CvMat*)mhiimg;
    CvMat  maskstub, *mask = (CvMat*)maskimg;
    CvMat  orientstub, *orient = (CvMat*)orientation;

    CV_FUNCNAME( "cvCalcMotionGradient" );

    __BEGIN__;

    CV_CALL( mhi = cvGetMat( mhi, &mhistub ));
    CV_CALL( mask = cvGetMat( mask, &maskstub ));
    CV_CALL( orient = cvGetMat( orient, &orientstub ));

    if( !CV_IS_MASK_ARR( mask ))
        CV_ERROR( CV_StsBadMask, "" );

    if( apertureSize < 3 || apertureSize > 7 || (apertureSize & 1) == 0 )
        CV_ERROR( CV_StsOutOfRange, "apertureSize must be 3, 5 or 7" );

    if( minTDelta <= 0 || maxTDelta <= 0 )
        CV_ERROR( CV_StsOutOfRange, "both delta's must be positive" );

    if( CV_MAT_CN( mhi->type ) != 1 || CV_MAT_CN( orient->type ) != 1 )
        CV_ERROR( CV_BadNumChannels, "" );

    if( CV_MAT_DEPTH( mhi->type ) != CV_32F ||
        CV_MAT_DEPTH( orient->type ) != CV_32F )
        CV_ERROR( CV_BadDepth, "" );

    if( !CV_ARE_SIZES_EQ( mhi, mask ) || !CV_ARE_SIZES_EQ( orient, mhi ))
        CV_ERROR( CV_StsUnmatchedSizes, "" );

    if( minTDelta > maxTDelta )
    {
        double t;
        CV_SWAP( minTDelta, maxTDelta, t );
    }

    size = cvGetMatSize( mhi );

    IPPI_CALL( icvCalcMotionGradient32fC1R( mhi->data.fl, mhi->step,
                                            (uchar*)(mask->data.ptr), mask->step,
                                            orient->data.fl, orient->step,
                                            size, apertureSize,
                                            (float) maxTDelta, (float) minTDelta,
                                            CV_IS_IMAGE_HDR(orientation) ?
                                            ((IplImage*)orientation)->origin : 0 ));
    __END__;
}


CV_IMPL double
cvCalcGlobalOrientation( const void* orientation, const void* maskimg, const void* mhiimg,
                         double curr_mhi_timestamp, double mhi_duration )
{
    CvSize  size;
    float  angle = 0;
    CvMat  mhistub, *mhi = (CvMat*)mhiimg;
    CvMat  maskstub, *mask = (CvMat*)maskimg;
    CvMat  orientstub, *orient = (CvMat*)orientation;
    int  mhi_step, orient_step, mask_step;

    CV_FUNCNAME( "cvCalcGlobalOrientation" );

    __BEGIN__;

    CV_CALL( mhi = cvGetMat( mhi, &mhistub ));
    CV_CALL( mask = cvGetMat( mask, &maskstub ));
    CV_CALL( orient = cvGetMat( orient, &orientstub ));

    if( !CV_IS_MASK_ARR( mask ))
        CV_ERROR( CV_StsBadMask, "" );

    if( CV_MAT_CN( mhi->type ) != 1 || CV_MAT_CN( orient->type ) != 1 )
        CV_ERROR( CV_BadNumChannels, "" );

    if( CV_MAT_DEPTH( mhi->type ) != CV_32F ||
        CV_MAT_DEPTH( orient->type ) != CV_32F )
        CV_ERROR( CV_BadDepth, "" );

    if( !CV_ARE_SIZES_EQ( mhi, mask ) || !CV_ARE_SIZES_EQ( orient, mhi ))
        CV_ERROR( CV_StsUnmatchedSizes, "" );

    size = cvGetMatSize( mhi );

    mhi_step = mhi->step;
    mask_step = mask->step;
    orient_step = orient->step;

    if( CV_IS_MAT_CONT( mhi->type & mask->type & orient->type ))
    {
        size.width *= size.height;
        mhi_step = mask_step = orient_step = CV_STUB_STEP;
        size.height = 1;
    }

    IPPI_CALL( icvCalcGlobalOrientation32fC1R( orient->data.fl, orient_step,
                                               (uchar*)(mask->data.ptr), mask_step,
                                               mhi->data.fl, mhi_step, size,
                                               (float)curr_mhi_timestamp,
                                               (float)mhi_duration, &angle ));
    __END__;

    return angle;
}


CV_IMPL CvSeq*
cvSegmentMotion( const CvArr* mhiimg, CvArr* segmask, CvMemStorage* storage,
                 double timestamp, double seg_thresh )
{
    CvSeq* components = 0;
    CvMat* mask8u = 0;

    CV_FUNCNAME( "cvSegmentMotion" );

    __BEGIN__;

    CvMat  mhistub, *mhi = (CvMat*)mhiimg;
    CvMat  maskstub, *mask = (CvMat*)segmask;
    float  ts = (float)timestamp;
    float  comp_idx = 1;
    float  stub_val = FLT_MAX*0.1f;
    int x, y;

    if( !storage )
        CV_ERROR( CV_StsNullPtr, "NULL memory storage" );

    CV_CALL( mhi = cvGetMat( mhi, &mhistub ));
    CV_CALL( mask = cvGetMat( mask, &maskstub ));

    if( CV_MAT_TYPE( mhi->type ) != CV_32FC1 || CV_MAT_TYPE( mask->type ) != CV_32FC1 )
        CV_ERROR( CV_BadDepth, "Both MHI and the destination mask" );

    if( !CV_ARE_SIZES_EQ( mhi, mask ))
        CV_ERROR( CV_StsUnmatchedSizes, "" );

    CV_CALL( mask8u = cvCreateMat( mhi->rows + 2, mhi->cols + 2, CV_8UC1 ));
    cvZero( mask8u );
    cvZero( mask );
    CV_CALL( components = cvCreateSeq( CV_SEQ_KIND_GENERIC, sizeof(CvSeq),
                                       sizeof(CvConnectedComp), storage ));

    for( y = 0; y < mhi->rows; y++ )
    {
        int* mhi_row = (int*)(mhi->data.ptr + y*mhi->step);
        for( x = 0; x < mhi->cols; x++ )
        {
            if( mhi_row[x] == 0 )
                mhi_row[x] = (int&)stub_val;
        }
    }

    for( y = 0; y < mhi->rows; y++ )
    {
        int* mhi_row = (int*)(mhi->data.ptr + y*mhi->step);
        uchar* mask8u_row = mask8u->data.ptr + (y+1)*mask8u->step + 1;

        for( x = 0; x < mhi->cols; x++ )
        {
            if( mhi_row[x] == (int&)ts && mask8u_row[x] == 0 )
            {
                CvConnectedComp comp;
                int x1, y1;
                CvScalar _seg_thresh = cvRealScalar(seg_thresh);
                CvPoint seed = cvPoint(x,y);

                CV_CALL( cvFloodFill( mhi, seed, cvRealScalar(0), _seg_thresh, _seg_thresh,
                                      &comp, CV_FLOODFILL_MASK_ONLY + 2*256 + 4, mask8u ));

                for( y1 = 0; y1 < comp.rect.height; y1++ )
                {
                    int* mask_row1 = (int*)(mask->data.ptr +
                                    (comp.rect.y + y1)*mask->step) + comp.rect.x;
                    uchar* mask8u_row1 = mask8u->data.ptr +
                                    (comp.rect.y + y1+1)*mask8u->step + comp.rect.x+1;

                    for( x1 = 0; x1 < comp.rect.width; x1++ )
                    {
                        if( mask8u_row1[x1] > 1 )
                        {
                            mask8u_row1[x1] = 1;
                            mask_row1[x1] = (int&)comp_idx;
                        }
                    }
                }
                comp_idx++;
                cvSeqPush( components, &comp );
            }
        }
    }

    for( y = 0; y < mhi->rows; y++ )
    {
        int* mhi_row = (int*)(mhi->data.ptr + y*mhi->step);
        for( x = 0; x < mhi->cols; x++ )
        {
            if( mhi_row[x] == (int&)stub_val )
                mhi_row[x] = 0;
        }
    }

    __END__;

    cvReleaseMat( &mask8u );

    return components;
}

/* End of file. */
