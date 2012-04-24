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
// Copyright( C) 2000, Intel Corporation, all rights reserved.
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
//(including, but not limited to, procurement of substitute goods or services;
// loss of use, data, or profits; or business interruption) however caused
// and on any theory of liability, whether in contract, strict liability,
// or tort(including negligence or otherwise) arising in any way out of
// the use of this software, even if advised of the possibility of such damage.
//
//M*/

/*
   This is a set of macros and definitions intended to provide backward compatibility
   with the previous versions of OpenCV
*/

#ifndef _CVCOMPAT_H_
#define _CVCOMPAT_H_

#define CvMatType int
#define CvDisMaskType int
#define CvMatArray CvMat

#define  CV_MAT32F      CV_32FC1
#define  CV_MAT3x1_32F  CV_32FC1
#define  CV_MAT4x1_32F  CV_32FC1
#define  CV_MAT3x3_32F  CV_32FC1
#define  CV_MAT4x4_32F  CV_32FC1

#define  CV_MAT64D      CV_64FC1
#define  CV_MAT3x1_64D  CV_64FC1
#define  CV_MAT4x1_64D  CV_64FC1
#define  CV_MAT3x3_64D  CV_64FC1
#define  CV_MAT4x4_64D  CV_64FC1

#define  IPL_GAUSSIAN_5x5   7
#define  CvBox2D32f     CvBox2D

/* allocation/deallocation macros */
#define cvCreateImageData   cvCreateData
#define cvReleaseImageData  cvReleaseData
#define cvSetImageData      cvSetData
#define cvGetImageRawData   cvGetRawData

#define cvmAlloc            cvCreateData
#define cvmFree             cvReleaseData
#define cvmAllocArray       cvCreateData
#define cvmFreeArray        cvReleaseData

CV_INLINE CvMat cvMatArray( int rows, int cols, int type,
                            int count, void* data CV_DEFAULT(0));
CV_INLINE CvMat cvMatArray( int rows, int cols, int type,
                            int count, void* data )
{
    return cvMat( rows*count, cols, type, data );    
}

#define cvUpdateMHIByTime  cvUpdateMotionHistory

#define cvAccMask cvAcc
#define cvSquareAccMask cvSquareAcc
#define cvMultiplyAccMask cvMultiplyAcc
#define cvRunningAvgMask(imgY, imgU, mask, alpha) cvRunningAvg(imgY, imgU, alpha, mask)

#define cvSetHistThresh  cvSetHistBinRanges
#define cvCalcHistMask(img, mask, hist, doNotClear) cvCalcHist(img, hist, doNotClear, mask)

CV_INLINE double cvMean( const CvArr* image, const CvArr* mask CV_DEFAULT(0));
CV_INLINE double cvMean( const CvArr* image, const CvArr* mask )
{
    CvScalar mean = cvAvg( image, mask );
    return mean.val[0];
}

#define cvSumPixels( img )  ((cvSum( img )).val[0])

CV_INLINE void  cvMean_StdDev( const CvArr* image, double* mean, double* sdv,
                               const CvArr* mask CV_DEFAULT(0));
CV_INLINE void  cvMean_StdDev( const CvArr* image, double* mean,
                               double* sdv, const CvArr* mask )
{
    CvScalar _mean, _sdv;
    cvAvgSdv( image, &_mean, &_sdv, mask );

    if( mean )
        *mean = _mean.val[0];

    if( sdv )
        *sdv = _sdv.val[0];
}


CV_INLINE void cvmPerspectiveProject( const CvArr* mat, const CvArr* src,
                                      CvArr* dst );
CV_INLINE void cvmPerspectiveProject( const CvArr* mat, const CvArr* src,
                                      CvArr* dst )
{
    CvMat tsrc, tdst;

    cvReshape( src, &tsrc, 3, 0 );
    cvReshape( dst, &tdst, 3, 0 );

    cvPerspectiveTransform( &tsrc, &tdst, mat );
}


CV_INLINE void cvFillImage( CvArr* mat, double color );
CV_INLINE void cvFillImage( CvArr* mat, double color )
{
    CvPoint left_top = { 0, 0 }, right_bottom = { 0, 0 };

    cvGetRawData( mat, 0, 0, (CvSize*)&right_bottom );
    right_bottom.x--;
    right_bottom.y--;

    cvRectangle( mat, left_top, right_bottom, color, CV_FILLED );
}


#define cvMeanMask  cvMean
#define cvMean_StdDevMask(img,mask,mean,sdv) cvMean_StdDev(img,mean,sdv,mask)

#define cvNormMask(imgA,imgB,mask,normType) cvNorm(imgA,imgB,normType,mask)

#define cvMinMaxLocMask(img, mask, min_val, max_val, min_loc, max_loc) \
        cvMinMaxLoc(img, min_val, max_val, min_loc, max_loc, mask)

#define cvRemoveMemoryManager  cvSetMemoryManager

#define cvmSetZero( mat )               cvSetZero( mat )
#define cvmSetIdentity( mat )           cvSetIdentity( mat )
#define cvmAdd( src1, src2, dst )       cvAdd( src1, src2, dst, 0 )
#define cvmSub( src1, src2, dst )       cvSub( src1, src2, dst, 0 )
#define cvmCopy( src, dst )             cvCopy( src, dst, 0 )
#define cvmMul( src1, src2, dst )       cvMatMulAdd( src1, src2, 0, dst )
#define cvmTranspose( src, dst )        cvT( src, dst )
#define cvmInvert( src, dst )           cvInv( src, dst )
#define cvmMahalanobis(vec1, vec2, mat) cvMahalanobis( vec1, vec2, mat )
#define cvmDotProduct( vec1, vec2 )     cvDotProduct( vec1, vec2 )
#define cvmCrossProduct(vec1, vec2,dst) cvCrossProduct( vec1, vec2, dst )
#define cvmTrace( mat )                 (cvTrace( mat )).val[0]
#define cvmMulTransposed( src, dst, order ) cvMulTransposed( src, dst, order )
#define cvmEigenVV( mat, evec, eval, eps)   cvEigenVV( mat, evec, eval, eps )
#define cvmDet( mat )                   cvDet( mat )
#define cvmScale( src, dst, scale )     cvScale( src, dst, scale )

#define cvCopyImage( src, dst )         cvCopy( src, dst, 0 )

#endif/*_CVCOMPAT_H_*/
