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

#include <string.h>

#if _MSC_VER>=1200 && !defined __ICL
    #define CV_UNREFERENCED(arg)  (arg)
#else
    #define CV_UNREFERENCED(arg)
#endif

#define CvMatType int
#define CvDisMaskType int
#define CvMatArray CvMat

#define CvThreshType int
#define CvAdaptiveThreshMethod int
#define CvCompareMethod int
#define CvFontFace int
#define CvPolyApproxMethod int
#define CvContoursMatchMethod int
#define CvContourTreesMatchMethod int
#define CvCoeffType int
#define CvRodriguesType int
#define CvElementShape int
#define CvMorphOp int
#define CvTemplMatchMethod int

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

#define cvIntegralImage     cvIntegral
#define cvMatchContours     cvMatchShapes

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


CV_INLINE double  cvSumPixels( const CvArr* image );
CV_INLINE double  cvSumPixels( const CvArr* image )
{
    CvScalar scalar = cvSum( image );
    return scalar.val[0];
}

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


CV_INLINE void cvbRand( CvRandState* state, float* dst, int len );
CV_INLINE void cvbRand( CvRandState* state, float* dst, int len )
{
    CvMat mat = cvMat( 1, len, CV_32F, (void*)dst );
    cvRand( state, &mat );
}


CV_INLINE void  cvbCartToPolar( const float* y, const float* x,
                                float* magnitude, float* angle, int len );
CV_INLINE void  cvbCartToPolar( const float* y, const float* x,
                                float* magnitude, float* angle, int len )
{
    CvMat mx = cvMat( 1, len, CV_32F, (void*)x );
    CvMat my = mx;
    CvMat mm = mx;
    CvMat ma = mx;

    my.data.fl = (float*)y;
    mm.data.fl = (float*)magnitude;
    ma.data.fl = (float*)angle;

    cvCartToPolar( &mx, &my, &mm, angle ? &ma : NULL, 1 );
}


CV_INLINE void  cvbFastArctan( const float* y, const float* x,
                               float* angle, int len );
CV_INLINE void  cvbFastArctan( const float* y, const float* x,
                               float* angle, int len )
{
    CvMat mx = cvMat( 1, len, CV_32F, (void*)x );
    CvMat my = mx;
    CvMat ma = mx;

    my.data.fl = (float*)y;
    ma.data.fl = (float*)angle;

    cvCartToPolar( &mx, &my, NULL, &ma, 1 );
}


CV_INLINE  void  cvbSqrt( const float* x, float* y, int len );
CV_INLINE  void  cvbSqrt( const float* x, float* y, int len )
{
    CvMat mx = cvMat( 1, len, CV_32F, (void*)x );
    CvMat my = mx;
    my.data.fl = (float*)y;

    cvPow( &mx, &my, 0.5 );
}


CV_INLINE  void  cvbInvSqrt( const float* x, float* y, int len );
CV_INLINE  void  cvbInvSqrt( const float* x, float* y, int len )
{
    CvMat mx = cvMat( 1, len, CV_32F, (void*)x );
    CvMat my = mx;
    my.data.fl = (float*)y;

    cvPow( &mx, &my, -0.5 );
}


CV_INLINE  void  cvbReciprocal( const float* x, float* y, int len );
CV_INLINE  void  cvbReciprocal( const float* x, float* y, int len )
{
    CvMat mx = cvMat( 1, len, CV_32F, (void*)x );
    CvMat my = mx;
    my.data.fl = (float*)y;

    cvPow( &mx, &my, -1 );
}


CV_INLINE  void  cvbFastExp( const float* x, double* y, int len );
CV_INLINE  void  cvbFastExp( const float* x, double* y, int len )
{
    CvMat mx = cvMat( 1, len, CV_32F, (void*)x );
    CvMat my = cvMat( 1, len, CV_64F, y );
    cvExp( &mx, &my );
}


CV_INLINE  void  cvbFastLog( const double* x, float* y, int len );
CV_INLINE  void  cvbFastLog( const double* x, float* y, int len )
{
    CvMat mx = cvMat( 1, len, CV_64F, (void*)x );
    CvMat my = cvMat( 1, len, CV_32F, y );
    cvLog( &mx, &my );
}


CV_INLINE  CvRect  cvContourBoundingRect( const void* point_set, int update CV_DEFAULT(0));
CV_INLINE  CvRect  cvContourBoundingRect( const void* point_set, int update )
{
    return cvBoundingRect( point_set, update );
}


CV_INLINE double cvPseudoInverse( const CvArr* src, CvArr* dst,
                                  int flags CV_DEFAULT(0));
CV_INLINE double cvPseudoInverse( const CvArr* src, CvArr* dst, int flags )
{
    CV_UNREFERENCED( flags );
    return cvInvert( src, dst, CV_SVD );
}

#define cvPseudoInv cvPseudoInverse

#define cvContourMoments( contour, moments ) \
    cvMoments( contour, moments, 0 )

#define cvGetPtrAt              cvPtr2D
#define cvGetAt                 cvGet2D
#define cvSetAt(arr,val,y,x)    cvSet2D((arr),(y),(x),(val))

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
#define cvReleaseMatHeader              cvReleaseMat

/* Calculates exact convex hull of 2d point set */
CV_INLINE void cvConvexHull( CvPoint* points, int num_points, CvRect* bound_rect,
                             int orientation, int* hull, int* hullsize );
CV_INLINE void cvConvexHull( CvPoint* points, int num_points, CvRect* bound_rect,
                             int orientation, int* hull, int* hullsize )
{
    CvMat points1 = cvMat( 1, num_points, CV_32SC2, points );
    CvMat hull1 = cvMat( 1, num_points, CV_32SC1, hull );

    CV_UNREFERENCED( bound_rect );
    
    cvConvexHull2( &points1, &hull1, orientation, 0 );
    *hullsize = hull1.cols;
}

/* Calculates exact convex hull of 2d point set stored in a sequence */
#define cvContourConvexHull( contour, orientation, storage ) \
    cvConvexHull2( contour, storage, orientation )

/* Calculates approximate convex hull of 2d point set */
#define cvConvexHullApprox( points, num_points, bound_rect, bandwidth,      \
                            orientation, hull, hullsize )                   \
cvConvexHull( points, num_points, bound_rect, orientation, hull, hullsize )

/* Calculates approximate convex hull of 2d point set stored in a sequence */
#define cvContourConvexHullApprox( contour, bandwidth, orientation, storage )   \
    cvConvexHull2( contour, storage, orientation )


CV_INLINE void cvMinAreaRect( CvPoint* points, int n,
                              int left, int bottom, int right, int top,
                              CvPoint2D32f* anchor,
                              CvPoint2D32f* vect1,
                              CvPoint2D32f* vect2 );
CV_INLINE void cvMinAreaRect( CvPoint* points, int n,
                              int left, int bottom, int right, int top,
                              CvPoint2D32f* anchor,
                              CvPoint2D32f* vect1,
                              CvPoint2D32f* vect2 )
{
    CvMat mat = cvMat( 1, n, CV_32SC2, points );
    CvBox2D box = cvMinAreaRect2( &mat, 0 );
    CvPoint2D32f pt[4];

    cvBoxPoints( box, pt );
    *anchor = pt[0];
    vect1->x = pt[1].x - pt[0].x;
    vect1->y = pt[1].y - pt[0].y;
    vect2->x = pt[3].x - pt[0].x;
    vect2->y = pt[3].y - pt[0].y;

    CV_UNREFERENCED( (left, bottom, right, top) );
}


CV_INLINE  void  cvFitLine3D( CvPoint3D32f* points, int count, CvDisType dist,
                              void *param, float reps, float aeps, float* line );
CV_INLINE  void  cvFitLine3D( CvPoint3D32f* points, int count, CvDisType dist,
                              void *param, float reps, float aeps, float* line )
{
    CvMat mat = cvMat( 1, count, CV_32FC3, points );
    float _param = param != NULL ? *(float*)param : 0.f;
    assert( dist != CV_DIST_USER );
    cvFitLine( &mat, dist, _param, reps, aeps, line );
}

/* Fits a line into set of 2d points in a robust way (M-estimator technique) */ 
CV_INLINE  void  cvFitLine2D( CvPoint2D32f* points, int count, CvDisType dist,
                              void *param, float reps, float aeps, float* line );
CV_INLINE  void  cvFitLine2D( CvPoint2D32f* points, int count, CvDisType dist,
                              void *param, float reps, float aeps, float* line )
{
    CvMat mat = cvMat( 1, count, CV_32FC2, points );
    float _param = param != NULL ? *(float*)param : 0.f;
    assert( dist != CV_DIST_USER );
    cvFitLine( &mat, dist, _param, reps, aeps, line );
}


CV_INLINE  void cvFitEllipse( const CvPoint2D32f* points, int count, CvBox2D* box );
CV_INLINE  void cvFitEllipse( const CvPoint2D32f* points, int count, CvBox2D* box )
{
    CvMat mat = cvMat( 1, count, CV_32FC2, (void*)points );
    *box = cvFitEllipse2( &mat );
}


/* Retrieves value of the particular bin
   of x-dimensional (x=1,2,3,...) histogram */
#define cvQueryHistValue_1D( hist, idx0 ) \
    ((float)cvGetReal1D( (hist)->bins, (idx0)))
#define cvQueryHistValue_2D( hist, idx0, idx1 ) \
    ((float)cvGetReal2D( (hist)->bins, (idx0), (idx1)))
#define cvQueryHistValue_3D( hist, idx0, idx1, idx2 ) \
    ((float)cvGetReal3D( (hist)->bins, (idx0), (idx1), (idx2)))
#define cvQueryHistValue_nD( hist, idx ) \
    ((float)cvGetRealND( (hist)->bins, (idx)))

/* Returns pointer to the particular bin of x-dimesional histogram.
   For sparse histogram the bin is created if it didn't exist before */
#define cvGetHistValue_1D( hist, idx0 ) \
    ((float*)cvPtr1D( (hist)->bins, (idx0), 0))
#define cvGetHistValue_2D( hist, idx0, idx1 ) \
    ((float*)cvPtr2D( (hist)->bins, (idx0), (idx1), 0))
#define cvGetHistValue_3D( hist, idx0, idx1, idx2 ) \
    ((float*)cvPtr3D( (hist)->bins, (idx0), (idx1), (idx2), 0))
#define cvGetHistValue_nD( hist, idx ) \
    ((float*)cvPtrND( (hist)->bins, (idx), 0))


#define CV_IS_SET_ELEM_EXISTS CV_IS_SET_ELEM


CV_INLINE  int  cvHoughLines( CvArr* image, double rho,
                              double theta, int threshold,
                              float* lines, int linesNumber );
CV_INLINE  int  cvHoughLines( CvArr* image, double rho,
                              double theta, int threshold,
                              float* lines, int linesNumber )
{
    CvMat linesMat = cvMat( 1, linesNumber, CV_32FC2, lines ); 
    cvHoughLines2( image, &linesMat, CV_HOUGH_STANDARD,
                   rho, theta, threshold, 0, 0 );

    return linesMat.rows;
}


CV_INLINE  int  cvHoughLinesP( CvArr* image, double rho,
                               double theta, int threshold,
                               int lineLength, int lineGap,
                               int* lines, int linesNumber );
CV_INLINE  int  cvHoughLinesP( CvArr* image, double rho,
                               double theta, int threshold,
                               int lineLength, int lineGap,
                               int* lines, int linesNumber )
{
    CvMat linesMat = cvMat( 1, linesNumber, CV_32SC4, lines ); 
    cvHoughLines2( image, &linesMat, CV_HOUGH_PROBABILISTIC,
                   rho, theta, threshold, lineLength, lineGap );

    return linesMat.rows;
}


CV_INLINE  int  cvHoughLinesSDiv( CvArr* image, double rho, int srn,
                                  double theta, int stn, int threshold,
                                  float* lines, int linesNumber );
CV_INLINE  int  cvHoughLinesSDiv( CvArr* image, double rho, int srn,
                                  double theta, int stn, int threshold,
                                  float* lines, int linesNumber )
{
    CvMat linesMat = cvMat( 1, linesNumber, CV_32FC2, lines ); 
    cvHoughLines2( image, &linesMat, CV_HOUGH_MULTI_SCALE,
                   rho, theta, threshold, srn, stn );

    return linesMat.rows;
}


/* Find fundamental matrix */
CV_INLINE  void  cvFindFundamentalMatrix( int* points1, int* points2,
                            int numpoints, int method, float* matrix );
CV_INLINE  void  cvFindFundamentalMatrix( int* points1, int* points2,
                            int numpoints, int method, float* matrix )
{
    CvMat* pointsMat1;
    CvMat* pointsMat2;
    CvMat fundMatr = cvMat(3,3,CV_32F,matrix);
    int i, curr = 0;
#ifndef __BORLANDC__
    method = method;
#endif

    pointsMat1 = cvCreateMat(3,numpoints,CV_64F);
    pointsMat2 = cvCreateMat(3,numpoints,CV_64F);

    for( i = 0; i < numpoints; i++ )
    {
        cvmSet(pointsMat1,0,i,points1[curr]);//x
        cvmSet(pointsMat1,1,i,points1[curr+1]);//y
        cvmSet(pointsMat1,2,i,1.0);

        cvmSet(pointsMat2,0,i,points2[curr]);//x
        cvmSet(pointsMat2,1,i,points2[curr+1]);//y
        cvmSet(pointsMat2,2,i,1.0);
        curr += 2;
    }

    cvFindFundamentalMat(pointsMat1,pointsMat2,&fundMatr,CV_FM_RANSAC,1,0.99,0);

    cvReleaseMat(&pointsMat1);
    cvReleaseMat(&pointsMat2);
}


CV_INLINE  float  cvCalcEMD( const float* signature1, int size1,
                             const float* signature2, int size2,
                             int dims, CvDisType dist_type CV_DEFAULT(CV_DIST_L2),
                             CvDistanceFunction dist_func CV_DEFAULT(0),
                             float* lower_bound CV_DEFAULT(0),
                             void* user_param CV_DEFAULT(0));
CV_INLINE  float  cvCalcEMD( const float* signature1, int size1,
                             const float* signature2, int size2,
                             int dims, CvDisType dist_type,
                             CvDistanceFunction dist_func,
                             float* lower_bound, void* user_param )
{
    CvMat sign1 = cvMat( size1, dims + 1, CV_32FC1, (void*)signature1 );
    CvMat sign2 = cvMat( size2, dims + 1, CV_32FC1, (void*)signature2 );

    return cvCalcEMD2( &sign1, &sign2, dist_type, dist_func, 0, 0, lower_bound, user_param ); 
}


CV_INLINE  void  cvKMeans( int num_clusters, float** samples,
                           int num_samples, int vec_size,
                           CvTermCriteria termcrit, int* cluster_idx );
CV_INLINE  void  cvKMeans( int num_clusters, float** samples,
                           int num_samples, int vec_size,
                           CvTermCriteria termcrit, int* cluster_idx )
{
    CvMat* samples_mat = cvCreateMat( num_samples, vec_size, CV_32FC1 );
    CvMat cluster_idx_mat = cvMat( num_samples, 1, CV_32SC1, cluster_idx );
    int i;
    for( i = 0; i < num_samples; i++ )
        memcpy( samples_mat->data.fl + i*vec_size, samples[i], vec_size*sizeof(float));
    cvKMeans2( samples_mat, num_clusters, &cluster_idx_mat, termcrit );
    cvReleaseMat( &samples_mat );
}

#define cvKalmanUpdateByTime  cvKalmanPredict
#define cvKalmanUpdateByMeasurement cvKalmanCorrect

/* Creates hand mask image given several points on the hand */
OPENCVAPI  void  cvCreateHandMask( CvSeq* hand_points,
                                   IplImage *img_mask, CvRect *roi);

/* Finds hand region in range image data */
OPENCVAPI  void  cvFindHandRegion (CvPoint3D32f* points, int count,
                                CvSeq* indexs,
                                float* line, CvSize2D32f size, int flag,
                                CvPoint3D32f* center,
                                CvMemStorage* storage, CvSeq **numbers);

/* Finds hand region in range image data (advanced version) */
OPENCVAPI  void  cvFindHandRegionA( CvPoint3D32f* points, int count,
                                CvSeq* indexs,
                                float* line, CvSize2D32f size, int jc,
                                CvPoint3D32f* center,
                                CvMemStorage* storage, CvSeq **numbers);

/****************************************************************************************\
*                                   Pixel Access Macros                                  *
\****************************************************************************************/

typedef struct _CvPixelPosition8u
{
    unsigned char*   currline;      /* pointer to the start of the current pixel line   */
    unsigned char*   topline;       /* pointer to the start of the top pixel line       */
    unsigned char*   bottomline;    /* pointer to the start of the first line           */
                                    /* which is below the image                         */
    int     x;                      /* current x coordinate ( in pixels )               */
    int     width;                  /* width of the image  ( in pixels )                */
    int     height;                 /* height of the image  ( in pixels )               */
    int     step;                   /* distance between lines ( in elements of single   */
                                    /* plane )                                          */
    int     step_arr[3];            /* array: ( 0, -step, step ). It is used for        */
                                    /* vertical moving                                  */
} CvPixelPosition8u;

/* this structure differs from the above only in data type */
typedef struct _CvPixelPosition8s
{
    char*   currline;
    char*   topline;
    char*   bottomline;
    int     x;
    int     width;
    int     height;
    int     step;
    int     step_arr[3];
} CvPixelPosition8s;

/* this structure differs from the CvPixelPosition8u only in data type */
typedef struct _CvPixelPosition32f
{
    float*  currline;
    float*  topline;
    float*  bottomline;
    int     x;
    int     width;
    int     height;
    int     step;
    int     step_arr[3];
} CvPixelPosition32f;


/* Initialize one of the CvPixelPosition structures.   */
/*  pos    - initialized structure                     */
/*  origin - pointer to the left-top corner of the ROI */
/*  step   - width of the whole image in bytes         */
/*  roi    - width & height of the ROI                 */
/*  x, y   - initial position                          */
#define CV_INIT_PIXEL_POS(pos, origin, _step, roi, _x, _y, orientation)    \
    (                                                                        \
    (pos).step = (_step)/sizeof((pos).currline[0]) * (orientation ? -1 : 1), \
    (pos).width = (roi).width,                                               \
    (pos).height = (roi).height,                                             \
    (pos).bottomline = (origin) + (pos).step*(pos).height,                   \
    (pos).topline = (origin) - (pos).step,                                   \
    (pos).step_arr[0] = 0,                                                   \
    (pos).step_arr[1] = -(pos).step,                                         \
    (pos).step_arr[2] = (pos).step,                                          \
    (pos).x = (_x),                                                          \
    (pos).currline = (origin) + (pos).step*(_y) )


/* Move to specified point ( absolute shift ) */
/*  pos    - position structure               */
/*  x, y   - coordinates of the new position  */
/*  cs     - number of the image channels     */
#define CV_MOVE_TO( pos, _x, _y, cs )                                                   \
((pos).currline = (_y) >= 0 && (_y) < (pos).height ? (pos).topline + ((_y)+1)*(pos).step : 0, \
 (pos).x = (_x) >= 0 && (_x) < (pos).width ? (_x) : 0, (pos).currline + (_x) * (cs) )

/* Get current coordinates                    */
/*  pos    - position structure               */
/*  x, y   - coordinates of the new position  */
/*  cs     - number of the image channels     */
#define CV_GET_CURRENT( pos, cs )  ((pos).currline + (pos).x * (cs))
 
/* Move by one pixel relatively to current position */
/*  pos    - position structure                     */
/*  cs     - number of the image channels           */
 
/* left */
#define CV_MOVE_LEFT( pos, cs ) \
 ( --(pos).x >= 0 ? (pos).currline + (pos).x*(cs) : 0 )
 
/* right */
#define CV_MOVE_RIGHT( pos, cs ) \
 ( ++(pos).x < (pos).width ? (pos).currline + (pos).x*(cs) : 0 )
 
/* up */
#define CV_MOVE_UP( pos, cs ) \
 (((pos).currline -= (pos).step) != (pos).topline ? (pos).currline + (pos).x*(cs) : 0 )
 
/* down */
#define CV_MOVE_DOWN( pos, cs ) \
 (((pos).currline += (pos).step) != (pos).bottomline ? (pos).currline + (pos).x*(cs) : 0 )
 
/* left up */
#define CV_MOVE_LU( pos, cs ) ( CV_MOVE_LEFT(pos, cs), CV_MOVE_UP(pos, cs))
 
/* right up */
#define CV_MOVE_RU( pos, cs ) ( CV_MOVE_RIGHT(pos, cs), CV_MOVE_UP(pos, cs))
 
/* left down */
#define CV_MOVE_LD( pos, cs ) ( CV_MOVE_LEFT(pos, cs), CV_MOVE_DOWN(pos, cs))
 
/* right down */
#define CV_MOVE_RD( pos, cs ) ( CV_MOVE_RIGHT(pos, cs), CV_MOVE_DOWN(pos, cs))
 
 
 
/* Move by one pixel relatively to current position with wrapping when the position     */
/* achieves image boundary                                                              */
/*  pos    - position structure                                                         */
/*  cs     - number of the image channels                                               */
 
/* left */
#define CV_MOVE_LEFT_WRAP( pos, cs ) \
 ((pos).currline + ( --(pos).x >= 0 ? (pos).x : ((pos).x = (pos).width-1))*(cs))
 
/* right */
#define CV_MOVE_RIGHT_WRAP( pos, cs ) \
 ((pos).currline + ( ++(pos).x < (pos).width ? (pos).x : ((pos).x = 0))*(cs) )
 
/* up */
#define CV_MOVE_UP_WRAP( pos, cs ) \
    ((((pos).currline -= (pos).step) != (pos).topline ? \
    (pos).currline : ((pos).currline = (pos).bottomline - (pos).step)) + (pos).x*(cs) )
 
/* down */
#define CV_MOVE_DOWN_WRAP( pos, cs ) \
    ((((pos).currline += (pos).step) != (pos).bottomline ? \
    (pos).currline : ((pos).currline = (pos).topline + (pos).step)) + (pos).x*(cs) )
 
/* left up */
#define CV_MOVE_LU_WRAP( pos, cs ) ( CV_MOVE_LEFT_WRAP(pos, cs), CV_MOVE_UP_WRAP(pos, cs))
/* right up */
#define CV_MOVE_RU_WRAP( pos, cs ) ( CV_MOVE_RIGHT_WRAP(pos, cs), CV_MOVE_UP_WRAP(pos, cs))
/* left down */
#define CV_MOVE_LD_WRAP( pos, cs ) ( CV_MOVE_LEFT_WRAP(pos, cs), CV_MOVE_DOWN_WRAP(pos, cs))
/* right down */
#define CV_MOVE_RD_WRAP( pos, cs ) ( CV_MOVE_RIGHT_WRAP(pos, cs), CV_MOVE_DOWN_WRAP(pos, cs))
 
/* Numeric constants which used for moving in arbitrary direction  */
#define CV_SHIFT_NONE   2
#define CV_SHIFT_LEFT   1
#define CV_SHIFT_RIGHT  3
#define CV_SHIFT_UP     6
#define CV_SHIFT_DOWN  10
#define CV_SHIFT_LU     5
#define CV_SHIFT_RU     7
#define CV_SHIFT_LD     9
#define CV_SHIFT_RD    11
 
/* Move by one pixel in specified direction                                     */
/*  pos    - position structure                                                 */
/*  shift  - direction ( it's value must be one of the CV_SHIFT_… constants ) */
/*  cs     - number of the image channels                                       */
#define CV_MOVE_PARAM( pos, shift, cs )                                           \
    ( (pos).currline += (pos).step_arr[(shift)>>2], (pos).x += ((shift)&3)-2,       \
    ((pos).currline != (pos).topline && (pos).currline != (pos).bottomline &&       \
    (pos).x >= 0 && (pos).x < (pos).width) ? (pos).currline + (pos).x*(cs) : 0 )
 
/* Move by one pixel in specified direction with wrapping when the               */
/* position achieves image boundary                                              */
/*  pos    - position structure                                                  */
/*  shift  - direction ( it's value must be one of the CV_SHIFT_… constants )  */
/*  cs     - number of the image channels                                        */
#define CV_MOVE_PARAM_WRAP( pos, shift, cs )                                      \
    ( (pos).currline += (pos).step_arr[(shift)>>2],                                 \
    (pos).currline = ((pos).currline == (pos).topline ?                             \
    (pos).bottomline - (pos).step :                                                 \
    (pos).currline == (pos).bottomline ?                                            \
    (pos).topline + (pos).step : (pos).currline),                                   \
                                                                                    \
    (pos).x += ((shift)&3)-2,                                                       \
    (pos).x = ((pos).x < 0 ? (pos).width-1 : (pos).x >= (pos).width ? 0 : (pos).x), \
                                                                                    \
    (pos).currline + (pos).x*(cs) )

#endif/*_CVCOMPAT_H_*/
