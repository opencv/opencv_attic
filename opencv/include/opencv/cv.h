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


#ifndef _CV_H_
#define _CV_H_

#ifdef __IPL_H__
#define HAVE_IPL /* On Windows IPL is needed by default */
#endif

#if defined(_CH_)
#include <dlfcn.h>
void *_ChCv_handle = _dlopen("libcv.dl", RTLD_LAZY);
if(_ChCv_handle == NULL) {
   fprintf(_stderr, "Error: dlopen(): %s\n", dlerror());
   fprintf(_stderr, "       cannot get _ChCv_handle in cv.h\n");
   exit(-1);
} 
void _dlclose_libcv(void) {
  dlclose(_ChCv_handle);
}
_atexit(_dlclose_libcv);
#endif


#ifdef HAVE_IPL
#ifndef _INC_WINDOWS
    #define CV_PRETEND_WINDOWS
    #define _INC_WINDOWS
    typedef struct tagBITMAPINFOHEADER BITMAPINFOHEADER;
    typedef int BOOL;
#endif
#include "ipl.h"
#ifdef CV_PRETEND_WINDOWS
    #undef _INC_WINDOWS
#endif
#endif

#include "cvtypes.h"
#include "cverror.h"

#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************************************\
*                                     Allocation/deallocation                            *
\****************************************************************************************/

/* <malloc> wrapper.
   If there is no enough memory the function
   (as well as other OpenCV functions that call cvAlloc)
   raises an error. */
OPENCVAPI  void*  cvAlloc( int size );

/* <free> wrapper.
   Here and further all the memory releasing functions
   (that all call cvFree) take double pointer which is used
   to clear user pointer to the data after releasing it.
   Passing pointer to NULL pointer is Ok: nothing happens in this case
*/
OPENCVAPI  void   cvFree( void** ptr );

/* Allocates and initializes IplImage header */
OPENCVAPI  IplImage*  cvCreateImageHeader( CvSize size, int depth, int channels );

/* Inializes IplImage header */
OPENCVAPI IplImage* cvInitImageHeader( IplImage* image, CvSize size, int depth,
                                       int channels, int origin CV_DEFAULT(0),
                                       int align CV_DEFAULT(4));

/* Creates IPL image (header and data) */
OPENCVAPI  IplImage*  cvCreateImage( CvSize size, int depth, int channels );

/* Releases (i.e. deallocates) IPL image header */
OPENCVAPI  void  cvReleaseImageHeader( IplImage** image );

/* Releases IPL image header and data */
OPENCVAPI  void  cvReleaseImage( IplImage** image );

/* Creates a copy of IPL image (widthStep may differ) */
OPENCVAPI IplImage* cvCloneImage( const IplImage* image );

/* Sets a Channel Of Interest (only a few functions support COI) - 
   use cvCopy to extract the selected channel and/or put it back */
OPENCVAPI  void  cvSetImageCOI( IplImage* image, int coi );

/* Retrieves image Channel Of Interest */
OPENCVAPI  int  cvGetImageCOI( IplImage* image );

/* Sets image ROI (region of interest) (COI is not changed) */
OPENCVAPI  void  cvSetImageROI( IplImage* image, CvRect rect );

/* Resets image ROI and COI */
OPENCVAPI  void  cvResetImageROI( IplImage* image );

/* Retrieves image ROI */
OPENCVAPI  CvRect cvGetImageROI( const IplImage* image );

/* Allocates and initalizes CvMat header */
OPENCVAPI  CvMat*  cvCreateMatHeader( int rows, int cols, int type );

#define CV_AUTOSTEP  0x7fffffff

/* Initializes CvMat header */
OPENCVAPI CvMat* cvInitMatHeader( CvMat* mat, int rows, int cols,
                                  int type, void* data CV_DEFAULT(NULL),
                                  int step CV_DEFAULT(CV_AUTOSTEP) );

/* Allocates and initializes CvMat header and allocates data */
OPENCVAPI  CvMat*  cvCreateMat( int rows, int cols, int type );

/* Releases CvMat header and deallocates matrix data
   (reference counting is used for data) */
OPENCVAPI  void  cvReleaseMat( CvMat** mat );

/* Decrements CvMat data reference counter and deallocates the data if
   it reaches 0 */
CV_INLINE  void  cvDecRefData( CvArr* arr );
CV_INLINE  void  cvDecRefData( CvArr* arr )
{
    if( CV_IS_MAT( arr ) || CV_IS_MATND( arr ))
    {
        CvMat* mat = (CvMat*)arr; /* the first few fields of CvMat and CvMatND are the same */
        mat->data.ptr = NULL;
        if( mat->refcount != NULL && --*mat->refcount == 0 )
        {
            uchar* data = (uchar*)mat->refcount + 2*sizeof(mat->refcount);
            cvFree( (void**)&data );
        }
        mat->refcount = NULL;
    }
}

/* Increments CvMat data reference counter */
CV_INLINE  int  cvIncRefData( CvArr* arr );
CV_INLINE  int  cvIncRefData( CvArr* arr )
{
    int refcount = 0;
    if( CV_IS_MAT( arr ) || CV_IS_MATND( arr ))
    {
        CvMat* mat = (CvMat*)arr;
        if( mat->refcount != NULL )
            refcount = ++*mat->refcount;
    }
    return refcount;
}


/* Creates an exact copy of the input matrix (except, may be, step value) */
OPENCVAPI CvMat* cvCloneMat( const CvMat* mat );


/* Makes a new matrix from <rect> subrectangle of input array.
   No data is copied */
OPENCVAPI CvMat* cvGetSubRect( const CvArr* arr, CvMat* submat, CvRect rect );
#define cvGetSubArr cvGetSubRect

/* Partial case of the cvGetSubArr:
    row span of the input array is selected
    (end_row is not included into the span). */
OPENCVAPI CvMat* cvGetRows( const CvArr* arr, CvMat* submat,
                            int start_row, int end_row );

CV_INLINE  void  cvGetRow( const CvArr* arr, CvMat* submat, int row );
CV_INLINE  void  cvGetRow( const CvArr* arr, CvMat* submat, int row )
{
    cvGetRows( arr, submat, row, row + 1 );
}


/* Partial case of the cvGetSubArr:
    column span of the input array is selected
    (end_col is not included into the span) */
OPENCVAPI CvMat* cvGetCols( const CvArr* arr, CvMat* submat,
                               int start_col, int end_col );

CV_INLINE  void  cvGetCol( const CvArr* arr, CvMat* submat, int col );
CV_INLINE  void  cvGetCol( const CvArr* arr, CvMat* submat, int col )
{
    cvGetCols( arr, submat, col, col + 1 );
}

/* Partial case of the cvGetSubArr:
    a single diagonal of the input array is selected
   (diag = 0 means main diagonal, >0 means some diagonal above the main one,
   <0 - below the main one).
   The diagonal will be represented as a column (nx1 matrix). */
OPENCVAPI CvMat* cvGetDiag( const CvArr* arr, CvMat* submat,
                            int diag CV_DEFAULT(0));

/* low-level scalar <-> raw data conversion functions */
OPENCVAPI void cvScalarToRawData( const CvScalar* scalar, void* data, int type,
                                  int extend_to_12 CV_DEFAULT(0) );

OPENCVAPI void cvRawDataToScalar( const void* data, int type, CvScalar* scalar );

/* Allocates and initializes CvMatND header */
OPENCVAPI  CvMatND*  cvCreateMatNDHeader( int dims, const int* sizes, int type );

/* Allocates and initializes CvMatND header and allocates data */
OPENCVAPI  CvMatND*  cvCreateMatND( int dims, const int* sizes, int type );

/* Initializes preallocated CvMatND header */
OPENCVAPI  CvMatND*  cvInitMatNDHeader( CvMatND* mat, int dims, const int* sizes,
                                        int type, void* data CV_DEFAULT(NULL) );

/* Releases CvMatND */
CV_INLINE  void  cvReleaseMatND( CvMatND** mat );
CV_INLINE  void  cvReleaseMatND( CvMatND** mat )
{
    cvReleaseMat( (CvMat**)mat );
}

/* Creates a copy of CvMatND (except, may be, steps) */
OPENCVAPI  CvMatND* cvCloneMatND( const CvMatND* mat );

/* Allocates and initializes CvSparseMat header and allocates data */
OPENCVAPI  CvSparseMat*  cvCreateSparseMat( int dims, const int* sizes, int type );

/* Releases CvSparseMat */
OPENCVAPI  void  cvReleaseSparseMat( CvSparseMat** mat );

/* Creates a copy of CvSparseMat (except, may be, zero items) */
OPENCVAPI  CvSparseMat* cvCloneSparseMat( const CvSparseMat* mat );

/* Initializes sparse array iterator
   (returns the first node or NULL if the array is empty) */
OPENCVAPI  CvSparseNode* cvInitSparseMatIterator( const CvSparseMat* mat,
                                                  CvSparseMatIterator* matIterator );

// returns next sparse array node (or NULL if there is no more nodes)
CV_INLINE CvSparseNode* cvGetNextSparseNode( CvSparseMatIterator* matIterator );
CV_INLINE CvSparseNode* cvGetNextSparseNode( CvSparseMatIterator* matIterator )
{
    if( matIterator->node->next )
        return matIterator->node = matIterator->node->next;
    else
    {
        int idx;
        for( idx = ++matIterator->curidx; idx < matIterator->mat->hashsize; idx++ )
        {
            CvSparseNode* node = (CvSparseNode*)matIterator->mat->hashtable[idx];
            if( node )
            {
                matIterator->curidx = idx;
                return matIterator->node = node;
            }
        }
        return NULL;
    }
}

/* Returns type of array elements:
   CV_8UC1 ... CV_64FC4 ... */
OPENCVAPI  int cvGetElemType( const CvArr* arr );

/* Retrieves number of an array dimensions and
   optionally sizes of the dimensions */
OPENCVAPI  int cvGetDims( const CvArr* arr, int* sizes CV_DEFAULT(NULL) );


/* Retrieves size of a particular array dimension.
   For 2d arrays cvGetDimSize(arr,0) returns number of rows (image height)
   and cvGetDimSize(arr,1) returns number of columns (image width) */
OPENCVAPI  int cvGetDimSize( const CvArr* arr, int index );


/* ptr = &arr(idx1,idx2,...). All indexes are zero-based,
   the major dimensions go first (e.g. (y,x) for 2D, (z,y,x) for 3D */
OPENCVAPI uchar* cvPtr1D( const CvArr* arr, int idx1, int* type CV_DEFAULT(NULL));
OPENCVAPI uchar* cvPtr2D( const CvArr* arr, int idx1, int idx2, int* type CV_DEFAULT(NULL) );
OPENCVAPI uchar* cvPtr3D( const CvArr* arr, int idx1, int idx2, int idx3,
                          int* type CV_DEFAULT(NULL));

/* For CvMat or IplImage number of indices should be 2
   (row index (y) goes first, column index (x) goes next).
   For CvMatND or CvSparseMat number of infices should match number of <dims> and
   indices order should match the array dimension order. */
OPENCVAPI uchar* cvPtrND( const CvArr* arr, int* idx, int* type CV_DEFAULT(NULL) );

/* value = arr(idx1,idx2,...) */
OPENCVAPI CvScalar cvGet1D( const CvArr* arr, int idx1 );
OPENCVAPI CvScalar cvGet2D( const CvArr* arr, int idx1, int idx2 );
OPENCVAPI CvScalar cvGet3D( const CvArr* arr, int idx1, int idx2, int idx3 );
OPENCVAPI CvScalar cvGetND( const CvArr* arr, int* idx );

/* for 1-channel arrays */
OPENCVAPI double cvGetReal1D( const CvArr* arr, int idx1 );
OPENCVAPI double cvGetReal2D( const CvArr* arr, int idx1, int idx2 );
OPENCVAPI double cvGetReal3D( const CvArr* arr, int idx1, int idx2, int idx3 );
OPENCVAPI double cvGetRealND( const CvArr* arr, int* idx );

/* arr(idx1,idx2,...) = value */
OPENCVAPI void cvSet1D( CvArr* arr, int idx1, CvScalar value );
OPENCVAPI void cvSet2D( CvArr* arr, int idx1, int idx2, CvScalar value );
OPENCVAPI void cvSet3D( CvArr* arr, int idx1, int idx2, int idx3, CvScalar value );
OPENCVAPI void cvSetND( CvArr* arr, int* idx, CvScalar value );

/* for 1-channel arrays */
OPENCVAPI void cvSetReal1D( CvArr* arr, int idx1, double value );
OPENCVAPI void cvSetReal2D( CvArr* arr, int idx1, int idx2, double value );
OPENCVAPI void cvSetReal3D( CvArr* arr, int idx1,
                            int idx2, int idx3, double value );
OPENCVAPI void cvSetRealND( CvArr* arr, int* idx, double value );

/* Converts CvArr (IplImage or CvMat,...) to CvMat.
   If the last parameter is non-zero, function can
   convert multi(>2)-dimensional array to CvMat as long as
   the last array's dimension is continous. The resultant
   matrix will be have appropriate (a huge) number of rows */
OPENCVAPI CvMat* cvGetMat( const CvArr* src, CvMat* header,
                           int* coi CV_DEFAULT(NULL),
                           int allowND CV_DEFAULT(0));

/* Converts CvArr (IplImage or CvMat) to IplImage */
OPENCVAPI IplImage* cvGetImage( const CvArr* array, IplImage* img );


/* Changes a shape of multi-dimensional array.
   new_cn == 0 means that number of channels remains unchanged.
   new_dims == 0 means that number and sizes of dimensions remain the same
   (unless they need to be changed to set the new number of channels)
   if new_dims == 1, there is no need to specify new dimension sizes
   The resultant configuration should be achievable w/o data copying.
   If the resultant array is sparse, CvSparseMat header should be passed
   to the function else if the result is 1 or 2 dimensional,
   CvMat header should be passed to the function
   else CvMatND header should be passed */
OPENCVAPI CvArr* cvReshapeMatND( const CvArr* array,
                                 int sizeof_header, CvArr* header,
                                 int new_cn, int new_dims, int* new_sizes );

#define cvReshapeND( arr, header, new_cn, new_dims, new_sizes )   \
      cvReshapeMatND( (arr), sizeof(*(header)), (header),         \
                      (new_cn), (new_dims), (new_sizes))

OPENCVAPI CvMat* cvReshape( const CvArr* array, CvMat* header,
                            int new_cn, int new_rows CV_DEFAULT(0) );

/* Repeats source 2d array several times in both horizontal and
   vertical direction to fit destination array */
OPENCVAPI void cvRepeat( const CvArr* src, CvArr* dst );

/* Allocates array data */
OPENCVAPI  void  cvCreateData( CvArr* array );

/* Releases array data */
OPENCVAPI  void  cvReleaseData( CvArr* array );

/* Attaches user data to the array header. The step is reffered to
   the pre-last dimension. That is, all the planes of the array
   must be joint (w/o gaps) */
OPENCVAPI  void  cvSetData( CvArr* array, void* data, int step );

/* Retrieves raw data of CvMat, IplImage or CvMatND.
   In the latter case the function raises an error if
   the array can not be represented as a matrix */
OPENCVAPI void cvGetRawData( const CvArr* array, uchar** data,
                             int* step CV_DEFAULT(NULL),
                             CvSize* roi_size CV_DEFAULT(NULL));

/* Returns width and height of array in elements */
OPENCVAPI  CvSize cvGetSize( const CvArr* arr );

/* Copies source array to destination array */
OPENCVAPI  void  cvCopy( const CvArr* src, CvArr* dst,
                         const CvArr* mask CV_DEFAULT(NULL) );

/* Sets all or "masked" elements of input array
   to the same <scalar> value*/
OPENCVAPI  void  cvSet( CvArr* arr, CvScalar scalar,
                        const CvArr* mask CV_DEFAULT(NULL) );

/* Clears all the array elements (sets them to 0) */
OPENCVAPI  void  cvSetZero( CvArr* mat );
#define cvZero  cvSetZero


/* Splits a multi-channel array into the set of single-channel arrays or
   extracts particular [color] plane */
OPENCVAPI  void  cvCvtPixToPlane( const void *src, void *dst0, void *dst1,
                                  void *dst2, void *dst3 );

/* Merges a set of single-channel arrays into the single multi-channel array
   or inserts one particular [color] plane to the array */
OPENCVAPI  void  cvCvtPlaneToPix( const void *src0, const void *src1,
                                  const void *src2, const void *src3,
                                  void *dst );

/* Performs linear transformation on every source array element:
   dst(x,y,c) = scale*src(x,y,c)+shift.
   Arbitrary combination of input and output array depths are allowed
   (number of channels must be the same), thus the function can be used
   for depth conversion */
OPENCVAPI  void  cvConvertScale( const CvArr *src, CvArr *dst,
                                 double scale CV_DEFAULT(1),
                                 double shift CV_DEFAULT(0) );
#define cvCvtScale cvConvertScale
#define cvScale  cvConvertScale
#define cvConvert( src, dst )  cvConvertScale( (src), (dst), 1, 0 )


/* Performs linear transformation on every source array element,
   stores absolute value of the result:
   dst(x,y,c) = abs(scale*src(x,y,c)+shift).
   destination array must have 8u type.
   In other cases one may use cvConvertScale + cvAbsDiffS */
OPENCVAPI  void  cvConvertScaleAbs( const void *src, void *dst,
                                    double scale CV_DEFAULT(1),
                                    double shift CV_DEFAULT(0) );
#define cvCvtScaleAbs  cvConvertScaleAbs


/* Finds minimum rectangle containing two given rectangles */
OPENCVAPI  CvRect  cvMaxRect( const CvRect* rect1, const CvRect* rect2 );

/* Finds coordinates of the box vertices */
OPENCVAPI  void cvBoxPoints( CvBox2D box, CvPoint2D32f pt[4] );


/****************************************************************************************\
*                   Arithmetic, logic and comparison operations                          *
\****************************************************************************************/

/* dst(mask) = srcA(mask) + srcB(mask) */
OPENCVAPI  void  cvAdd( const CvArr* srcA, const CvArr* srcB, CvArr* dst,
                        const CvArr* mask CV_DEFAULT(NULL));

/* dst(mask) = src(mask) + value */
OPENCVAPI  void  cvAddS( const CvArr* src, CvScalar value, CvArr* dst,
                         const CvArr* mask CV_DEFAULT(NULL));

/* dst(mask) = srcA(mask) - srcB(mask) */
OPENCVAPI  void  cvSub( const CvArr* srcA, const CvArr* srcB, CvArr* dst,
                        const CvArr* mask CV_DEFAULT(NULL));

/* dst(mask) = src(mask) - value = src(mask) + (-value) */
CV_INLINE  void  cvSubS( const CvArr* src, CvScalar value, CvArr* dst,
                         const CvArr* mask CV_DEFAULT(NULL));
CV_INLINE  void  cvSubS( const CvArr* src, CvScalar value, CvArr* dst,
                         const CvArr* mask )
{
    cvAddS( src, cvScalar( -value.val[0], -value.val[1], -value.val[2], -value.val[3]),
            dst, mask );
}

/* dst(mask) = value - src(mask) */
OPENCVAPI  void  cvSubRS( const CvArr* src, CvScalar value, CvArr* dst,
                          const CvArr* mask CV_DEFAULT(NULL));

/* dst(idx) = srcA(idx) * srcB(idx) * scale (element-wise multiplication with scale) */
OPENCVAPI  void  cvMul( const CvArr* srcA, const CvArr* srcB,
                        CvArr* dst, double scale CV_DEFAULT(1) );

/* element-wise division/inversion with scaling: 
    dst(idx) = srcA(idx) * scale / srcB(idx)
    or dst(idx) = scale / srcB(idx) if srcA == 0 */
OPENCVAPI  void  cvDiv( const CvArr* srcA, const CvArr* srcB,
                        CvArr* dst, double scale CV_DEFAULT(1));

/* dst = srcA * scale + srcB */
OPENCVAPI  void  cvScaleAdd( const CvArr* srcA, CvScalar scale,
                             const CvArr* srcB, CvArr* dst );

/* dst = srcA * alpha + srcB * beta + gamma */
OPENCVAPI  void  cvAddWeighted( const CvArr* srcA, double alpha,
                                const CvArr* srcB, double beta,
                                double gamma, CvArr* dst );

/* result = sum(srcA(i) * srcB*(i))  (srcB is conjugated)
             i                                           */
OPENCVAPI  double  cvDotProduct( const CvArr* srcA, const CvArr* srcB );


/* dst(idx) = src1(idx) & src2(idx) */
OPENCVAPI void cvAnd( const CvArr* src1, const CvArr* src2,
                      CvArr* dst, const CvArr* mask CV_DEFAULT(NULL));

/* dst(idx) = src(idx) & value */
OPENCVAPI void cvAndS( const CvArr* src, CvScalar value,
                       CvArr* dst, const CvArr* mask CV_DEFAULT(NULL));

/* dst(idx) = src1(idx) | src2(idx) */
OPENCVAPI void cvOr( const CvArr* src1, const CvArr* src2,
                     CvArr* dst, const CvArr* mask CV_DEFAULT(NULL));

/* dst(idx) = src(idx) | value */
OPENCVAPI void cvOrS( const CvArr* src, CvScalar value,
                      CvArr* dst, const CvArr* mask CV_DEFAULT(NULL));

/* dst(idx) = src1(idx) ^ src2(idx) */
OPENCVAPI void cvXor( const CvArr* src1, const CvArr* src2,
                      CvArr* dst, const CvArr* mask CV_DEFAULT(NULL));

/* dst(idx) = src(idx) ^ value */
OPENCVAPI void cvXorS( const CvArr* src, CvScalar value,
                       CvArr* dst, const CvArr* mask CV_DEFAULT(NULL));

/* dst(idx) = ~src(idx) */
OPENCVAPI void cvNot( const CvArr* src, CvArr* dst );

/* dst(idx) = lower(idx) <= src(idx) < upper(idx) */
OPENCVAPI void cvInRange( const CvArr* src, const CvArr* lower,
                          const CvArr* upper, CvArr* dst );

/* dst(idx) = lower <= src(idx) < upper */
OPENCVAPI void cvInRangeS( const CvArr* src, CvScalar lower,
                           CvScalar upper, CvArr* dst );

#define CV_CMP_EQ   0
#define CV_CMP_GT   1
#define CV_CMP_GE   2
#define CV_CMP_LT   3
#define CV_CMP_LE   4
#define CV_CMP_NE   5

/* The comparison operation support single-channel arrays only.
   Destination image should be 8uC1 or 8sC1 */

/* dst(idx) = src1(idx) _cmp_op_ src2(idx) */
OPENCVAPI void cvCmp( const CvArr* src1, const CvArr* src2, CvArr* dst, int cmpOp );

/* dst(idx) = src1(idx) _cmp_op_ scalar */
OPENCVAPI void cvCmpS( const CvArr* src1, double scalar, CvArr* dst, int cmpOp );

/* dst(idx) = min(src1(idx),src2(idx)) */
OPENCVAPI void cvMin( const CvArr* src1, const CvArr* src2, CvArr* dst );

/* dst(idx) = max(src1(idx),src2(idx)) */
OPENCVAPI void cvMax( const CvArr* src1, const CvArr* src2, CvArr* dst );

/* dst(idx) = min(src(idx),scalar) */
OPENCVAPI void cvMinS( const CvArr* src, double scalar, CvArr* dst );

/* dst(idx) = max(src(idx),scalar) */
OPENCVAPI void cvMaxS( const CvArr* src, double scalar, CvArr* dst );


/****************************************************************************************\
*                                Math operations                                         *
\****************************************************************************************/

/* Does cartesian->polar coordinates conversion.
   Either of output components (magnitude or angle) is optional */
OPENCVAPI  void  cvCartToPolar( const CvArr* x, const CvArr* y,
                                CvArr* magnitude, CvArr* angle CV_DEFAULT(NULL),
                                int angle_in_degrees CV_DEFAULT(0));

/* Does polar->cartesian coordinates conversion.
   Either of output components (magnitude or angle) is optional.
   If magnitude is missing it is assumed to be all 1's */
OPENCVAPI  void  cvPolarToCart( const CvArr* magnitude, const CvArr* angle,
                                CvArr* x, CvArr* y,
                                int angle_in_degrees CV_DEFAULT(0));

/* Does powering: dst(idx) = src(idx)^power */
OPENCVAPI  void  cvPow( const CvArr* src, CvArr* dst, double power );

/* Does exponention: dst(idx) = exp(src(idx)).
   Overflow is not handled yet. Underflow is handled.
   Maximal relative error is ~7e-6 */
OPENCVAPI  void  cvExp( const CvArr* src, CvArr* dst );

/* Calculates natural logarithms: dst(idx) = log(abs(src(idx))).
   Logarithm of 0 gives large negative number(~-700)
   Maximal relative error is ~3e-7
*/
OPENCVAPI  void  cvLog( const CvArr* src, CvArr* dst );

/* Checks array values for NaNs, Infs or simply for too large numbers
   (if CV_CHECK is set). If CV_CHECK_QUIET is set,
   no runtime errors is raised (function returns zero value in case of "bad" values).
   Otherwise cvError is called */ 
#define  CV_CHECK_RANGE    1
#define  CV_CHECK_QUIET    2
OPENCVAPI  int  cvCheckArr( const CvArr* arr, int flags CV_DEFAULT(0),
                            double minVal CV_DEFAULT(0), double maxVal CV_DEFAULT(0));
#define cvCheckArray cvCheckArr

/* RNG state */
typedef struct CvRandState
{
    uint64    state;    /* RNG state (the current seed and carry)*/
    int       disttype; /* distribution type */
    CvScalar  param[2]; /* parameters of RNG */
}
CvRandState;

/* Initalized RNG state */
#define CV_RAND_UNI      0
#define CV_RAND_NORMAL   1
OPENCVAPI  void  cvRandInit( CvRandState* state, double param1,
                             double param2, int seed,
                             int disttype CV_DEFAULT(CV_RAND_UNI));

/* Changes RNG range while preserving RNG state */
OPENCVAPI  void  cvRandSetRange( CvRandState* state, double param1, double param2,
                                 int index CV_DEFAULT(-1));

/* Fills array with random numbers */
OPENCVAPI  void  cvRand( CvRandState* state, CvArr* arr );

/* Returns 32-bit random number (ranges are not used)
   and updates RNG state */
CV_INLINE  unsigned  cvRandNext( CvRandState* state );
CV_INLINE  unsigned  cvRandNext( CvRandState* state )
{
    uint64 temp = 0;

    if( state )
    {
        temp = state->state;
        temp = (uint64)(unsigned)temp*1554115554 + (temp >> 32);
        state->state = temp;
    }

    return (unsigned)temp;
}


/****************************************************************************************\
*                                Matrix operations                                       *
\****************************************************************************************/

/* Calculates cross product of two 3d vectors */
OPENCVAPI  void  cvCrossProduct( const CvArr* srcA, const CvArr* srcB, CvArr* dst );

/* Matrix transform: dst = A*B + C, C is optional */
OPENCVAPI  void  cvMatMulAdd( const CvArr* srcA, const CvArr* srcB,
                              const CvArr* srcC, CvArr* dst );
#define cvMatMul( srcA, srcB, dst )  cvMatMulAdd( (srcA), (srcB), NULL, (dst))

#define CV_GEMM_A_T 1
#define CV_GEMM_B_T 2
#define CV_GEMM_C_T 4
/* Extended matrix transform:
   dst = alpha*op(A)*op(B) + beta*op(C), where op(X) is X or X^T */
OPENCVAPI  void  cvGEMM( const CvArr* srcA, const CvArr* srcB, double alpha,
                         const CvArr* srcC, double beta, CvArr* dst,
                         int tABC CV_DEFAULT(0));
#define cvMatMulAddEx cvGEMM

/* Transforms each element of source array and stores
   resultant vectors in destination array */
OPENCVAPI  void  cvMatMulAddS( const CvArr* src, CvArr* dst,
                               const CvMat* transform,
                               const CvMat* shiftvec CV_DEFAULT(NULL));
#define cvTransform cvMatMulAddS

/* Calculates A*A^T (order=0) or A^T*A (order=1) */
OPENCVAPI void cvMulTransposed( const CvArr* srcarr,
                                CvArr* dstarr, int order );

/* Tranposes matrix. Square matrices can be transposed in-place */
OPENCVAPI  void  cvTranspose( const CvArr* src, CvArr* dst );
#define cvT cvTranspose


/* Mirror array data around horizontal (flip=0),
   vertical (flip=1) or both(flip=-1) axises:
   cvFlip(src) flips images vertically and sequences horizontally (inplace) */
OPENCVAPI  void  cvFlip( const CvArr* src, CvArr* dst CV_DEFAULT(NULL),
                         int flip_mode CV_DEFAULT(0));
#define cvMirror cvFlip


#define CV_SVD_MODIFY_A   1
#define CV_SVD_U_T        2
#define CV_SVD_V_T        4

/* Performs Singular Value Decomposition of a matrix */
OPENCVAPI  void   cvSVD( CvArr* A, CvArr* W CV_DEFAULT(NULL),
                         CvArr* U CV_DEFAULT(NULL),
                         CvArr* V CV_DEFAULT(NULL),
                         int flags CV_DEFAULT(0));

/* Performs Singular Value Back Substitution:
   flags must be the same as in cvSVD */
OPENCVAPI  void   cvSVBkSb( const CvArr* warr, const CvArr* uarr,
                            const CvArr* varr, const CvArr* barr,
                            CvArr* xarr, int flags );

#define CV_LU  0
#define CV_SVD 1
/* Inverts matrix */
OPENCVAPI  double  cvInvert( const CvArr* src, CvArr* dst,
                             int method CV_DEFAULT(CV_LU));
#define cvInv cvInvert

/* Solves linear system Ax = b
   (returns 0 if A is singular) */
OPENCVAPI  int  cvSolve( const CvArr* A, const CvArr* b, CvArr* x,
                         int method CV_DEFAULT(CV_LU));

/* Calculates determinant of input matrix */
OPENCVAPI  double cvDet( const CvArr* mat );

/* Calculates trace of the matrix (sum of elements on the main diagonal) */
OPENCVAPI  CvScalar cvTrace( const CvArr* mat );

/* Finds eigen values and vectors of a _symmetric_ matrix */
OPENCVAPI  void  cvEigenVV( CvArr* src, CvArr* evects, CvArr* evals, double eps );

/* Makes an identity matrix (mat_ij = i == j) */
OPENCVAPI  void  cvSetIdentity( CvArr* mat, CvScalar value CV_DEFAULT(cvScalar(1)) );

/* Does perspective transform on every element of input array */
OPENCVAPI  void  cvPerspectiveTransform( const CvArr* src, CvArr* dst, const CvArr* mat );

/* Calculates covariation matrix for a set of vectors */
OPENCVAPI  void  cvCalcCovarMatrix( const CvArr** vects, CvArr* covarMatrix, CvArr* avg );

/* Calculates Mahalanobis(weighted) distance */
OPENCVAPI  double  cvMahalanobis( const CvArr* srcA, const CvArr* srcB, CvArr* mat );
#define cvMahalonobis  cvMahalanobis

/****************************************************************************************\
*                                    Array Statistics                                    *
\****************************************************************************************/

/* Finds sum of array elements */
OPENCVAPI  CvScalar  cvSum( const CvArr* array );


/* Calculates number of non-zero pixels */
OPENCVAPI  int  cvCountNonZero( const CvArr* array );


/* Calculates mean value of array elements */
OPENCVAPI  CvScalar  cvAvg( const CvArr* array, const CvArr* mask CV_DEFAULT(NULL) );

/* Calculates mean and standard deviation of pixel values */
OPENCVAPI  void  cvAvgSdv( const CvArr* array, CvScalar* mean, CvScalar* std_dev,
                           const CvArr* mask CV_DEFAULT(NULL) );

/* Finds global minimum, maximum among the input array elements and positions
   of the extremums */
OPENCVAPI  void  cvMinMaxLoc( const CvArr* array, double* min_val, double* max_val,
                              CvPoint* min_loc CV_DEFAULT(NULL),
                              CvPoint* max_loc CV_DEFAULT(NULL),
                              const CvArr* mask CV_DEFAULT(NULL) );

/* spatial and central moments */
typedef struct CvMoments
{
    double  m00, m10, m01, m20, m11, m02, m30, m21, m12, m03; /* spatial moments */
    double  mu20, mu11, mu02, mu30, mu21, mu12, mu03; /* central moments */
    double  inv_sqrt_m00; /* m00 != 0 ? 1/sqrt(m00) : 0 */
} CvMoments;

/* Calculates all spatial and central moments up to the 3rd order */
OPENCVAPI void cvMoments( const CvArr* array, CvMoments* moments, int binary CV_DEFAULT(0));

/* Retrieve particular spatial, central or normalized central moments */
OPENCVAPI  double  cvGetSpatialMoment( CvMoments* moments, int x_order, int y_order );
OPENCVAPI  double  cvGetCentralMoment( CvMoments* moments, int x_order, int y_order );
OPENCVAPI  double  cvGetNormalizedCentralMoment( CvMoments* moments,
                                                 int x_order, int y_order );

/* Hu invariants */
typedef struct CvHuMoments
{
    double hu1, hu2, hu3, hu4, hu5, hu6, hu7; /* Hu invariants */
} CvHuMoments;

/* Calculates 7 Hu's invariants from precalculated spatial and central moments */
OPENCVAPI void cvGetHuMoments( CvMoments*  moments, CvHuMoments*  hu_moments );

/* types of array norm */
#define CV_C            1
#define CV_L1           2
#define CV_L2           4
#define CV_NORM_MASK    7
#define CV_RELATIVE     8
#define CV_DIFF         16

#define CV_DIFF_C       (CV_DIFF | CV_C)
#define CV_DIFF_L1      (CV_DIFF | CV_L1)
#define CV_DIFF_L2      (CV_DIFF | CV_L2)
#define CV_RELATIVE_C   (CV_RELATIVE | CV_C)
#define CV_RELATIVE_L1  (CV_RELATIVE | CV_L1)
#define CV_RELATIVE_L2  (CV_RELATIVE | CV_L2)

/* Finds norm, difference norm or relative difference norm for an array (two arrays) */
OPENCVAPI  double  cvNorm( const CvArr* imgA, const CvArr* imgB CV_DEFAULT(NULL),
                           int normType CV_DEFAULT(CV_L2),
                           const CvArr* mask CV_DEFAULT(NULL) );

/****************************************************************************************\
*                              Dynamic data structures                                   *
\****************************************************************************************/

/* Creates new memory storage.
   block_size == 0 means that default,
   somewhat optimal size, is used (currently, it is 64K) */
OPENCVAPI  CvMemStorage*  cvCreateMemStorage( int block_size CV_DEFAULT(0));


/* Creates a memory storage that will borrow memory blocks from parent storage */
OPENCVAPI  CvMemStorage*  cvCreateChildMemStorage( CvMemStorage* parent );


/* Releases memory storage. All the children of a parent must be released before
   the parent. A child storage returns all the blocks to parent when it is released */
OPENCVAPI  void  cvReleaseMemStorage( CvMemStorage** storage );


/* Clears memory storage. This is the only way(!!!) (besides cvRestoreMemStoragePos)
   to reuse memory allocated for the storage - cvClearSeq,cvClearSet ...
   do not free any memory.
   A child storage returns all the blocks to the parent when it is cleared */
OPENCVAPI  void  cvClearMemStorage( CvMemStorage* storage );

/* Remember a storage "free memory" position */
OPENCVAPI  void  cvSaveMemStoragePos( const CvMemStorage* storage, CvMemStoragePos* pos );

/* Restore a storage "free memory" position */
OPENCVAPI  void  cvRestoreMemStoragePos( CvMemStorage* storage, CvMemStoragePos* pos );

/* Allocates continuous buffer of the specified size in the storage */
OPENCVAPI  void* cvMemStorageAlloc( CvMemStorage* storage, int size );

/* Creates new empty sequence that will reside in the specified storage */
OPENCVAPI  CvSeq*  cvCreateSeq( int seq_flags, int header_size,
                             int elem_size, CvMemStorage* storage );

/* Changes default size (granularity) of sequence blocks.
   The default size is ~1Kbyte */
OPENCVAPI  void  cvSetSeqBlockSize( CvSeq* seq, int delta_elements );


/* Adds new element to the end of sequence. Returns pointer to the element */
OPENCVAPI  char*  cvSeqPush( CvSeq* seq, void* element CV_DEFAULT(NULL));


/* Adds new element to the beginning of sequence. Returns pointer to it */
OPENCVAPI  char*  cvSeqPushFront( CvSeq* seq, void* element CV_DEFAULT(NULL));


/* Removes the last element from sequence and optionally saves it */
OPENCVAPI  void  cvSeqPop( CvSeq* seq, void* element CV_DEFAULT(NULL));


/* Removes the first element from sequence and optioanally saves it */
OPENCVAPI  void  cvSeqPopFront( CvSeq* seq, void* element CV_DEFAULT(NULL));


#define CV_FRONT 1
#define CV_BACK 0
/* Adds several new elements to the end of sequence */
OPENCVAPI  void  cvSeqPushMulti( CvSeq* seq, void* elements,
                                 int count, int in_front CV_DEFAULT(0) );

/* Removes several elements from the end of sequence and optionally saves them */
OPENCVAPI  void  cvSeqPopMulti( CvSeq* seq, void* elements,
                                int count, int in_front CV_DEFAULT(0) );

/* Inserts a new element in the middle of sequence.
   cvSeqInsert(seq,0,elem) == cvSeqPushFront(seq,elem) */
OPENCVAPI  char*  cvSeqInsert( CvSeq* seq, int before_index,
                               void* element CV_DEFAULT(NULL));

/* Removes specified sequence element */
OPENCVAPI  void  cvSeqRemove( CvSeq* seq, int index );


/* Removes all the elements from the sequence. The freed memory
   can be reused later only by the same sequence unless cvClearMemStorage
   or cvRestoreMemStoragePos is called */
OPENCVAPI  void  cvClearSeq( CvSeq* seq );


/* Retrives pointer to specified sequence element.
   Negative indices are supported and mean counting from the end
   (e.g -1 means the last sequence element) */
OPENCVAPI  char*  cvGetSeqElem( CvSeq* seq, int index, CvSeqBlock** block CV_DEFAULT(NULL) );


/* Calculates index of the specified sequence element.
   Returns -1 if element does not belong to the sequence */
OPENCVAPI int  cvSeqElemIdx( const CvSeq* seq, const void* element,
                             CvSeqBlock** block CV_DEFAULT(NULL) );

/* Initializes sequence writer. The new elements will be added to the end of sequence */
OPENCVAPI  void  cvStartAppendToSeq( CvSeq* seq, CvSeqWriter* writer );


/* Combination of cvCreateSeq and cvStartAppendToSeq */
OPENCVAPI  void  cvStartWriteSeq( int seq_flags, int header_size,
                                  int elem_size, CvMemStorage* storage,
                                  CvSeqWriter* writer );

/* Closes sequence writer, updates sequence header and returns pointer
   to the resultant sequence
   (which may be useful if the sequence was created using cvStartWriteSeq))
*/
OPENCVAPI  CvSeq*  cvEndWriteSeq( CvSeqWriter* writer );


/* Updates sequence header. May be useful to get access to some of previously
   written elements via cvGetSeqElem or sequence reader */
OPENCVAPI  void   cvFlushSeqWriter( CvSeqWriter* writer );


/* Initializes sequence reader.
   The sequence can be read in forward or backward direction */
OPENCVAPI void cvStartReadSeq( const CvSeq* seq, CvSeqReader* reader,
                               int reverse CV_DEFAULT(0) );


/* Returns current sequence reader position (currently observed sequence element) */
OPENCVAPI  int  cvGetSeqReaderPos( CvSeqReader* reader );


/* Changes sequence reader position. It may seek to an absolute or
   to relative to the current position */
OPENCVAPI  void   cvSetSeqReaderPos( CvSeqReader* reader, int index,
                                     int is_relative CV_DEFAULT(0));

/* Copies sequence content to an array */
OPENCVAPI  void*  cvCvtSeqToArray( CvSeq* seq, CvArr* array,
                                   CvSlice slice CV_DEFAULT(CV_WHOLE_SEQ) );

/* Creates sequence header for array.
   After that all the operations on sequences that do not alter the content
   can be applied to the resultant sequence */
OPENCVAPI  CvSeq* cvMakeSeqHeaderForArray( int seq_type, int header_size,
                                           int elem_size, void* elements, int total,
                                           CvSeq* seq, CvSeqBlock* block );

/* Extracts sequence slice (with or without copying sequence elements */
OPENCVAPI CvSeq* cvSeqSlice( CvSeq* seq, CvSlice slice,
                             CvMemStorage* storage CV_DEFAULT(NULL),
                             int copy_data CV_DEFAULT(0));

CV_INLINE CvSeq* cvCloneSeq( CvSeq* seq, CvMemStorage* storage CV_DEFAULT(NULL));
CV_INLINE CvSeq* cvCloneSeq( CvSeq* seq, CvMemStorage* storage )
{
    return cvSeqSlice( seq, CV_WHOLE_SEQ, storage, 1 );
}

/* Removes sequence slice */
OPENCVAPI  void  cvSeqRemoveSlice( CvSeq* seq, CvSlice slice );

/* Inserts a sequence or array into another sequence */
OPENCVAPI  void  cvSeqInsertSlice( CvSeq* seq, int index, const CvArr* from_arr );

/* a < b ? -1 : a > b ? 1 : 0 */
CV_EXTERN_C_FUNCPTR( int (CV_CDECL* CvCmpFunc)
                     (const void* a, const void* b, void* userdata ));

/* Sorts sequence in-place given element comparison function */
OPENCVAPI  void cvSeqSort( CvSeq* seq, CvCmpFunc func, void* userdata );

/* Reverses order of sequence elements in-place */
OPENCVAPI  void cvSeqInvert( CvSeq* seq );

/* Splits sequence into set of equivalency classes
   using specified equivalency criteria */
OPENCVAPI  int  cvPartitionSeq( CvSeq* seq, CvMemStorage* storage, CvSeq** comps,
                                CvCmpFunc is_equal, void* userdata, int is_set );

/************ Internal sequence functions ************/
OPENCVAPI  void  cvChangeSeqBlock( CvSeqReader* reader, int direction );
OPENCVAPI  void  cvCreateSeqBlock( CvSeqWriter* writer );


/* Creates a new set */
OPENCVAPI  CvSet*  cvCreateSet( int set_flags, int header_size,
                                int elem_size, CvMemStorage* storage );

/* Adds new element to the set and returns pointer to it */
OPENCVAPI  int  cvSetAdd( CvSet* set_header, CvSetElem* element CV_DEFAULT(NULL),
                          CvSetElem** inserted_element CV_DEFAULT(NULL) );

/* Fast variant of cvSetAdd */
CV_INLINE  CvSetElem* cvSetNew( CvSet* set_header );
CV_INLINE  CvSetElem* cvSetNew( CvSet* set_header )
{
    CvSetElem* elem = set_header->free_elems;
    if( elem )
    {
        set_header->free_elems = elem->next_free;
        elem->flags = elem->flags & CV_SET_ELEM_IDX_MASK;
    }
    else
        cvSetAdd( set_header, NULL, (CvSetElem**)&elem );
    return elem;
}

/* Removes set element given its pointer */
CV_INLINE  void cvSetRemoveByPtr( CvSet* set_header, void* _elem );
CV_INLINE  void cvSetRemoveByPtr( CvSet* set_header, void* _elem )
{
    CvSetElem* elem = (CvSetElem*)_elem;
    assert( elem->flags >= 0 && (elem->flags & CV_SET_ELEM_IDX_MASK) < set_header->total );
    elem->next_free = set_header->free_elems;
    elem->flags = (elem->flags & CV_SET_ELEM_IDX_MASK) | CV_SET_ELEM_FREE_FLAG;
    set_header->free_elems = elem;
}


/* Removes element from the set by its index  */
OPENCVAPI  void   cvSetRemove( CvSet* set_header, int index );


/* Returns a set element by index. If the element doesn't belong to the set,
   NULL is returned */
OPENCVAPI  CvSetElem*  cvGetSetElem( CvSet* set_header, int index );


/* Removes all the elements from the set */
OPENCVAPI  void  cvClearSet( CvSet* set_header );


/* Creates new graph */
OPENCVAPI  CvGraph*   cvCreateGraph( int graph_flags, int header_size,
                                  int vtx_size, int edge_size,
                                  CvMemStorage* storage );

/* Adds new vertex to the graph */
OPENCVAPI  int  cvGraphAddVtx( CvGraph* graph, CvGraphVtx* vertex CV_DEFAULT(NULL),
                               CvGraphVtx** inserted_vertex CV_DEFAULT(NULL) );


/* Removes vertex from the graph together with all incident edges */
OPENCVAPI  void   cvGraphRemoveVtx( CvGraph* graph, int index );
OPENCVAPI  void   cvGraphRemoveVtxByPtr( CvGraph* graph, CvGraphVtx* vtx );


/* Link two vertices specifed by indices or pointers if they
   are not connected or return pointer to already existing edge
   connecting the vertices.
   Functions return 1 if a new edge was created, 0 otherwise */
OPENCVAPI  int  cvGraphAddEdge( CvGraph* graph,
                                int start_idx, int end_idx,
                                CvGraphEdge* edge CV_DEFAULT(NULL),
                                CvGraphEdge** inserted_edge CV_DEFAULT(NULL) );

OPENCVAPI  int  cvGraphAddEdgeByPtr( CvGraph* graph,
                               CvGraphVtx* start_vtx, CvGraphVtx* end_vtx,
                               CvGraphEdge* edge CV_DEFAULT(NULL),
                               CvGraphEdge** inserted_edge CV_DEFAULT(NULL) );

/* Remove edge connecting two vertices */
OPENCVAPI  void  cvGraphRemoveEdge( CvGraph* graph, int start_idx, int end_idx );
OPENCVAPI  void  cvGraphRemoveEdgeByPtr( CvGraph* graph, CvGraphVtx* start_vtx,
                                         CvGraphVtx* end_vtx );

/* Find edge connecting two vertices */
OPENCVAPI  CvGraphEdge*  cvFindGraphEdge( CvGraph* graph, int start_idx, int end_idx );
OPENCVAPI  CvGraphEdge*  cvFindGraphEdgeByPtr( CvGraph* graph, CvGraphVtx* start_vtx,
                                               CvGraphVtx* end_vtx );
#define cvGraphFindEdge cvFindGraphEdge
#define cvGraphFindEdgeByPtr cvFindGraphEdgeByPtr

/* Remove all vertices and edges from the graph */
OPENCVAPI  void  cvClearGraph( CvGraph* graph );


/* Count number of edges incident to the vertex */
OPENCVAPI  int  cvGraphVtxDegree( CvGraph* graph, int vtx_idx );
OPENCVAPI  int  cvGraphVtxDegreeByPtr( CvGraph* graph, CvGraphVtx* vtx );


/* Retrieves graph vertex by given index */
#define cvGetGraphVtx( graph, idx ) (CvGraphVtx*)cvGetSetElem((CvSet*)(graph), (idx))

/* Retrieves index of a graph vertex given its pointer */
#define cvGraphVtxIdx( graph, vtx ) ((vtx)->flags & CV_SET_ELEM_IDX_MASK)

/* Retrieves index of a graph edge given its pointer */
#define cvGraphEdgeIdx( graph, edge ) ((edge)->flags & CV_SET_ELEM_IDX_MASK)


#define  CV_GRAPH_VERTEX        1
#define  CV_GRAPH_TREE_EDGE     2
#define  CV_GRAPH_BACK_EDGE     4
#define  CV_GRAPH_FORWARD_EDGE  8
#define  CV_GRAPH_CROSS_EDGE    16
#define  CV_GRAPH_ANY_EDGE      30
#define  CV_GRAPH_NEW_TREE      32
#define  CV_GRAPH_BACKTRACKING  64
#define  CV_GRAPH_OVER          -1

#define  CV_GRAPH_ALL_ITEMS    -1

/* flags for graph vertices and edges */
#define  CV_GRAPH_ITEM_VISITED_FLAG  (1 << 30)
#define  CV_IS_GRAPH_VERTEX_VISITED(vtx) \
    (((CvGraphVtx*)(vtx))->flags & CV_GRAPH_ITEM_VISITED_FLAG)
#define  CV_IS_GRAPH_EDGE_VISITED(edge) \
    (((CvGraphEdge*)(edge))->flags & CV_GRAPH_ITEM_VISITED_FLAG)
#define  CV_GRAPH_SEARCH_TREE_NODE_FLAG   (1 << 29)
#define  CV_GRAPH_FORWARD_EDGE_FLAG       (1 << 28)

typedef struct CvGraphScanner
{
    CvGraphVtx* vtx;       /* current graph vertex (or current edge origin) */
    CvGraphVtx* dst;       /* current graph edge destination vertex */
    CvGraphEdge* edge;     /* current edge */

    CvGraph* graph;        /* the graph */
    CvSeq*   stack;        /* the graph vertex stack */
    int      index;        /* the lower bound of certainly visited vertices */
    int      mask;         /* event mask */
}
CvGraphScanner;

/* Initializes graph traversal process.
   <mask> indicates what events one wants to handle. */
OPENCVAPI void  cvStartScanGraph( CvGraph* graph, CvGraphScanner* scanner,
                                  CvGraphVtx* vtx CV_DEFAULT(NULL),
                                  int mask CV_DEFAULT(CV_GRAPH_ALL_ITEMS));

/* Initializes graph traversal process.
   <mask> indicates what events one wants to handle. */
OPENCVAPI void  cvEndScanGraph( CvGraphScanner* scanner );

/* Get next graph element */
OPENCVAPI int  cvNextGraphItem( CvGraphScanner* scanner );

/* Creates a copy of graph */
OPENCVAPI CvGraph* cvCloneGraph( const CvGraph* graph, CvMemStorage* storage );

/****************************************************************************************\
*                                    Image Processing                                    *
\****************************************************************************************/

/* Does look-up transformation. Elements of the source array
   (that should be 8uC1 or 8sC1) are used as indexes in lutarr 256-element table */
OPENCVAPI  void cvLUT( const CvArr* srcarr, CvArr* dstarr, const CvArr* lutarr );


/* Smoothes array (remove noise) */
#define CV_BLUR_NO_SCALE 0
#define CV_BLUR  1
#define CV_GAUSSIAN  2
#define CV_MEDIAN 3
#define CV_BILATERAL 4

OPENCVAPI  void cvSmooth( const CvArr* srcarr, CvArr* dstarr,
                          int smoothtype CV_DEFAULT(CV_GAUSSIAN),
                          int param1 CV_DEFAULT(3),
                          int param2 CV_DEFAULT(0));

/* Finds integral image: SUM(X,Y) = sum(x<X,y<Y)I(x,y) */
OPENCVAPI void cvIntegral( const CvArr* image, CvArr* sumImage,
                           CvArr* sumSqImage CV_DEFAULT(NULL),
                           CvArr* tiltedSumImage CV_DEFAULT(NULL));

/*
   Down-samples image with prior gaussian smoothing.
   dst_width = floor(src_width/2)[+1],
   dst_height = floor(src_height/2)[+1]
*/
OPENCVAPI  void  cvPyrDown( const CvArr* src, CvArr* dst,
                            int filter CV_DEFAULT(CV_GAUSSIAN_5x5) );

/* 
   Up-samples image with posterior gaussian smoothing.
   dst_width = src_width*2,
   dst_height = src_height*2
*/
OPENCVAPI  void  cvPyrUp( const CvArr* src, CvArr* dst,
                          int filter CV_DEFAULT(CV_GAUSSIAN_5x5) );


/* Builds the whole pyramid at once. Output array of CvMat headers (levels[*])
   is initialized with the headers of subsequent pyramid levels */
/*OPENCVAPI  void  cvCalcPyramid( const CvArr* src, CvArr* container,
                                CvMat* levels, int levelCount,
                                int filter CV_DEFAULT(CV_GAUSSIAN_5x5) );*/


/* Segments image using son-father links (modification of Burt's algorithm).
   CvSeq<CvConnectedComp*> is returned to *comp */
OPENCVAPI void cvPyrSegmentation( IplImage* src,
                               IplImage* dst,
                               CvMemStorage *storage,
                               CvSeq **comp,
                               int level, double threshold1,
                               double threshold2 );


#define CV_SCHARR -1

/* calculates some image derivative using Sobel (apertureSize = 1,3,5,7)
   or Scharr (apertureSize = -1) operator.
   Scharr can be used only for the first dx or dy derivative */
OPENCVAPI void cvSobel( const CvArr* src, CvArr* dst,
                        int xorder, int yorder,
                        int apertureSize CV_DEFAULT(3));

/* Calculates Laplace operator: (d2/dx + d2/dy)I */
OPENCVAPI void cvLaplace( const CvArr* src, CvArr* dst,
                          int apertureSize CV_DEFAULT(3) );

/* Constants for color conversion */
#define  CV_BGR2BGRA    0
#define  CV_RGB2RGBA    CV_BGR2BGRA

#define  CV_BGRA2BGR    1
#define  CV_RGBA2RGB    CV_BGRA2BGR

#define  CV_BGR2RGBA    2
#define  CV_RGB2BGRA    CV_BGR2RGBA

#define  CV_RGBA2BGR    3
#define  CV_BGRA2RGB    CV_RGBA2BGR

#define  CV_BGR2RGB     4
#define  CV_RGB2BGR     CV_BGR2RGB

#define  CV_BGRA2RGBA   5
#define  CV_RGBA2BGRA   CV_BGRA2RGBA

#define  CV_BGR2GRAY    6
#define  CV_RGB2GRAY    7
#define  CV_GRAY2BGR    8
#define  CV_GRAY2RGB    CV_GRAY2BGR
#define  CV_GRAY2BGRA   9
#define  CV_GRAY2RGBA   CV_GRAY2BGRA
#define  CV_BGRA2GRAY   10
#define  CV_RGBA2GRAY   11

#define  CV_BGR2BGR565  12
#define  CV_RGB2BGR565  13
#define  CV_BGR5652BGR  14
#define  CV_BGR5652RGB  15
#define  CV_BGRA2BGR565 16
#define  CV_RGBA2BGR565 17
#define  CV_BGR5652BGRA 18
#define  CV_BGR5652RGBA 19

#define  CV_GRAY2BGR565 20
#define  CV_BGR5652GRAY 21

#define  CV_BGR2XYZ     22
#define  CV_RGB2XYZ     23
#define  CV_XYZ2BGR     24
#define  CV_XYZ2RGB     25

#define  CV_BGR2YCrCb   26
#define  CV_RGB2YCrCb   27
#define  CV_YCrCb2BGR   28
#define  CV_YCrCb2RGB   29

#define  CV_BGR2HSV     30
#define  CV_RGB2HSV     31

#define  CV_BGR2Lab     34
#define  CV_RGB2Lab     35

#define  CV_BayerBG2BGR 40
#define  CV_BayerGB2BGR 41
#define  CV_BayerRG2BGR 42
#define  CV_BayerGR2BGR 43

#define  CV_BayerBG2RGB CV_BayerRG2BGR
#define  CV_BayerGB2RGB CV_BayerGR2BGR
#define  CV_BayerRG2RGB CV_BayerBG2BGR
#define  CV_BayerGR2RGB CV_BayerGB2BGR

#define  CV_COLORCVT_MAX  48

/* Converts input array from one color space to another.
   Only 8-bit images are supported now */
OPENCVAPI  void  cvCvtColor( const CvArr* src, CvArr* dst, int colorCvtCode );


#define  CV_INTER_NN        0
#define  CV_INTER_LINEAR    1
/*#define  CV_INTER_CUBIC     2 - not implemented yet */

/* Resizes 1D-2D array. Destination size is determined by the size of destination array */
OPENCVAPI  void  cvResize( const CvArr* src, CvArr* dst,
                           int method CV_DEFAULT( CV_INTER_LINEAR ));


#define  CV_SHAPE_RECT      0
#define  CV_SHAPE_CROSS     1
#define  CV_SHAPE_ELLIPSE   2
#define  CV_SHAPE_CUSTOM    100

/* creates structuring element used for morphological operations */
OPENCVAPI  IplConvKernel*  cvCreateStructuringElementEx(
            int  cols, int  rows, int  anchorX, int  anchorY,
            int shape, int* values CV_DEFAULT(NULL) );

/* releases structuring element */
OPENCVAPI  void  cvReleaseStructuringElement( IplConvKernel** element );


/* erodes input image (applies minimum filter) one or more times.
   If element pointer is NULL, 3x3 rectangular element is used */
OPENCVAPI  void  cvErode( const CvArr* src, CvArr* dst,
                          IplConvKernel* element CV_DEFAULT(NULL),
                          int iterations CV_DEFAULT(1) );

/* dilates input image (applies maximum filter) one or more times.
   If element pointer is NULL, 3x3 rectangular element is used */
OPENCVAPI  void  cvDilate( const CvArr* src, CvArr* dst,
                           IplConvKernel* element CV_DEFAULT(NULL),
                           int iterations CV_DEFAULT(1) );

#define CV_MOP_OPEN         2
#define CV_MOP_CLOSE        3
#define CV_MOP_GRADIENT     4
#define CV_MOP_TOPHAT       5
#define CV_MOP_BLACKHAT     6

/* performs complex morphological transformation */
OPENCVAPI  void  cvMorphologyEx( const CvArr* src, CvArr* dst,
                                 CvArr* temp, IplConvKernel* element,
                                 int operation, int iterations CV_DEFAULT(1) );


/****************************************************************************************\
*                                     Drawing                                            *
\****************************************************************************************/

/****************************************************************************************\
*       Drawing functions work with arbitrary 8-bit images or single-channel images      *
*       with larger depth: 16s, 32s, 32f, 64f                                            *
*       All the functions include parameter color that means rgb value (that may be      *
*       constructed with CV_RGB macro) for color images and brightness                   *
*       for grayscale images.                                                            *
*       If a drawn figure is partially or completely outside the image, it is clipped.   *
\****************************************************************************************/

#define CV_RGB( r, g, b )  (int)((uchar)(b) + ((uchar)(g) << 8) + ((uchar)(r) << 16))
#define CV_FILLED -1

/* Draws 4-connected or 8-connected line segment connecting two points */
OPENCVAPI  void  cvLine( CvArr* array, CvPoint pt1, CvPoint pt2,
                         double color, int thickness CV_DEFAULT(1),
                         int connectivity CV_DEFAULT(8) );

/* Draws 8-connected line segment connecting two points with antialiazing.
   Ending coordinates may be specified with sub-pixel accuracy
   (scale is number of fractional bits in the coordinates) */
OPENCVAPI  void  cvLineAA( CvArr* array, CvPoint pt1, CvPoint pt2,
                           double color, int scale CV_DEFAULT(0));

/* Draws a rectangle given two opposite corners of the rectangle (pt1 & pt2),
   if thickness<0 (e.g. thickness == CV_FILLED), the filled box is drawn */
OPENCVAPI  void  cvRectangle( CvArr* array, CvPoint pt1, CvPoint pt2,
                              double color, int thickness CV_DEFAULT(1));

/* Draws a circle with specified center and radius.
   Thickness works in the same way as with cvRectangle */
OPENCVAPI  void  cvCircle( CvArr* array, CvPoint center, int radius,
                           double color, int thickness CV_DEFAULT(1));

/* Draws antialiazed circle with specified center and radius.
   Both the center and radius can be specified with sub-pixel accuracy */
OPENCVAPI  void  cvCircleAA( CvArr* array, CvPoint center, int radius,
                             double color, int scale CV_DEFAULT(0) );

/* Draws ellipse outline, filled ellipse, elliptic arc or filled elliptic sector,
   depending on <thickness>, <startAngle> and <endAngle> parameters. The resultant figure
   is rotated by <angle>. All the angles are in degrees */
OPENCVAPI  void  cvEllipse( CvArr* array, CvPoint center, CvSize axes,
                            double angle, double startAngle, double endAngle,
                            double color, int thickness CV_DEFAULT(1));

CV_INLINE  void  cvEllipseBox( CvArr* array, CvBox2D box,
                               double color, int thickness CV_DEFAULT(1));
CV_INLINE  void  cvEllipseBox( CvArr* array, CvBox2D box,
                               double color, int thickness )
{
    cvEllipse( array, cvPointFrom32f( box.center ),
               cvSize( cvRound(box.size.height*0.5),
                       cvRound(box.size.width*0.5)),
               box.angle*180/CV_PI, 0, 360, color, thickness );
}


/* Draws the whole ellipse or elliptic arc with antialiazing */
OPENCVAPI  void  cvEllipseAA( CvArr* array, CvPoint center, CvSize axes,
                              double angle, double startAngle,
                              double endAngle, double color,
                              int scale CV_DEFAULT(0) );

/* Fills convex or monotonous (every horizontal line intersects the polygon twice at the most,
   except, may be, horizontal sides) polygon. Connectivity or monotony is not checked */
OPENCVAPI  void  cvFillConvexPoly( CvArr* array, CvPoint* pts, int npts, double color );


/* Fills an area bounded by one or more arbitrary polygons (with possible intersections or
   self-intersections */
OPENCVAPI  void  cvFillPoly( CvArr* array, CvPoint** pts,
                             int* npts, int contours, double color );

/* Draws one or more polygonal curves */
OPENCVAPI  void  cvPolyLine( CvArr* array, CvPoint** pts, int* npts, int contours,
                             int closed, double color,
                             int thickness CV_DEFAULT(1),
                             int connectivity CV_DEFAULT(8));

/* Draws one or more antialiazed polygonal curves */
OPENCVAPI  void  cvPolyLineAA( CvArr* array, CvPoint** pts, int* npts, int contours,
                               int closed, double color, int scale CV_DEFAULT(0) );

/* Font metrics and structure */
#define CV_FONT_VECTOR0  0

typedef struct CvFont
{
    const int*  data; /* font data and metrics */
    CvSize      size; /* horizontal and vertical scale factors,
                         (8:8) fix-point numbers */
    int         italic_scale; /* slope coefficient: 0 - normal, >0 - italic */
    int         thickness; /* letters thickness */
    int         dx; /* horizontal interval between letters */
} CvFont;

/* Initializes font structure used further in cvPutText */
OPENCVAPI  void  cvInitFont( CvFont* font, int font_face,
                             double hscale, double vscale,
                             double italic_scale CV_DEFAULT(0),
                             int thickness CV_DEFAULT(1) );

/* Renders text stroke with specified font and color at specified location.
   CvFont should be initialized with cvInitFont */
OPENCVAPI  void  cvPutText( CvArr* array, const char* text, CvPoint org,
                            CvFont* font, double color );

/* Calculates bounding box of text stroke (useful for alignment) */
OPENCVAPI  void  cvGetTextSize( const char* text_string, CvFont* font,
                                CvSize* text_size, int* ymin );


/*********************************** data sampling **************************************/

/* Line iterator state */
typedef struct CvLineIterator
{
    uchar* ptr;
    int  err;
    int  plus_delta;
    int  minus_delta;
    int  plus_step;
    int  minus_step;
} CvLineIterator;

/* Initializes line iterator. Initially ptr will point to pt1 location in the array.
   Returns the number of points on the line between the endings. */
OPENCVAPI  int  cvInitLineIterator( const CvArr* array, CvPoint pt1, CvPoint pt2,
                                    CvLineIterator* lineIterator,
                                    int connectivity CV_DEFAULT(8));

/* Moves iterator to the next line point */
#define CV_NEXT_LINE_POINT( iterator )                                          \
{                                                                               \
    int mask =  (iterator).err < 0 ? -1 : 0;                                    \
    (iterator).err += (iterator).minus_delta + ((iterator).plus_delta & mask);  \
    (iterator).ptr += (iterator).minus_step + ((iterator).plus_step & mask);    \
}

/* Grabs the raster line data into the destination buffer.
   Returns the number of retrieved points. */
OPENCVAPI  int  cvSampleLine( const CvArr* array, CvPoint pt1, CvPoint pt2, void* buffer,
                              int connectivity CV_DEFAULT(8));

/* Retrieves the rectangular image region with specified center from the input array.
 dst(x,y) <- src(x + center.x - dst_width/2, y + center.y - dst_height/2).
 Values of pixels with fractional coordinates are retrieved using bilinear interpolation*/
OPENCVAPI  void  cvGetRectSubPix( const CvArr* src, CvArr* dst, CvPoint2D32f center );


/* Retrieves quadrangle from the input array.
    matrixarr = ( a11  a12 | b1 )   dst(x,y) <- src(A[x y]' + b)
                ( a21  a22 | b2 )   (bilinear interpolation is used to retrieve pixels
                                     with fractional coordinates)
*/
OPENCVAPI  void  cvGetQuadrangleSubPix( const CvArr* src, CvArr* dstarr,
                                        const CvArr* matrixarr,
                                        int fillOutliers CV_DEFAULT(0),
                                        CvScalar fillvalue CV_DEFAULT(cvScalarAll(0)));

/* Methods for comparing two array */
#define  CV_TM_SQDIFF        0
#define  CV_TM_SQDIFF_NORMED 1
#define  CV_TM_CCORR         2
#define  CV_TM_CCORR_NORMED  3
#define  CV_TM_CCOEFF        4
#define  CV_TM_CCOEFF_NORMED 5

/* Measures similarity between template and overlapped windows in the source image
   and fills the resultant image with the measurements */
OPENCVAPI  void  cvMatchTemplate( const CvArr* array, const CvArr* templ,
                                  CvArr* result, int method );

CV_EXTERN_C_FUNCPTR( float (CV_CDECL * CvDistanceFunction)
                     ( const float* a, const float* b, void* user_param ));

/* Computes earth mover distance between two weigted point sets
   (called signatures in image retrieval terminology) */
OPENCVAPI  float  cvCalcEMD2( const CvArr* signature1,
                              const CvArr* signature2,
                              CvDisType dist_type,
                              CvDistanceFunction dist_func CV_DEFAULT(0),
                              const CvArr* cost_matrix CV_DEFAULT(0),
                              CvArr* flow CV_DEFAULT(0),
                              float* lower_bound CV_DEFAULT(0),
                              void* user_param CV_DEFAULT(0));

/****************************************************************************************\
*                              Contours retrieving                                       *
\****************************************************************************************/

/*
Internal structure that is used for sequental retrieving contours from the image.
It supports both hierarchical and plane variants of Suzuki algorithm.
*/
typedef struct _CvContourScanner* CvContourScanner;

typedef enum CvContourRetrievalMode
{
    CV_RETR_EXTERNAL = 0,
    CV_RETR_LIST     = 1,
    CV_RETR_CCOMP    = 2,
    CV_RETR_TREE     = 3
}
CvContourRetrievalMode;

typedef enum CvChainApproxMethod
{
    CV_CHAIN_CODE             = 0,
    CV_CHAIN_APPROX_NONE      = 1,
    CV_CHAIN_APPROX_SIMPLE    = 2,
    CV_CHAIN_APPROX_TC89_L1   = 3,
    CV_CHAIN_APPROX_TC89_KCOS = 4,
    CV_LINK_RUNS              = 5
} CvChainApproxMethod;


/* Retrieves outer and possibly inner boundaries of white (non-zero) connected
   components on the black (zero) background */
OPENCVAPI  int  cvFindContours( CvArr* array, CvMemStorage* storage,
                           CvSeq**  firstContour,
                           int  headerSize CV_DEFAULT(sizeof(CvContour)),
                           CvContourRetrievalMode mode CV_DEFAULT( CV_RETR_LIST ),
                           CvChainApproxMethod method CV_DEFAULT(CV_CHAIN_APPROX_SIMPLE));


/* Initalizes contour retrieving process.
   Call cvStartFindContours.
   Call cvFindNextContour until null pointer is returned
   or some other condition becomes true.
   Call cvEndFindContours at the end. */
OPENCVAPI  CvContourScanner   cvStartFindContours( CvArr* array, CvMemStorage* storage,
                                        int header_size, CvContourRetrievalMode mode,
                                        CvChainApproxMethod method );

/* Retrieves next contour */
OPENCVAPI  CvSeq*  cvFindNextContour( CvContourScanner scanner );


/* Substitutes the last retrieved contour with the new one
   (if the substitutor is null, the last retrieved contour is removed from the tree) */
OPENCVAPI  void   cvSubstituteContour( CvContourScanner scanner, CvSeq* newContour );


/* Releases contour scanner and returns pointer to the first outer contour */
OPENCVAPI  CvSeq*  cvEndFindContours( CvContourScanner* scanner );

/* Draws contour outlines or filled interiors on the image */
OPENCVAPI void  cvDrawContours( CvArr *img, CvSeq* contour,
                                double external_color, double hole_color,
                                int max_level, int thickness CV_DEFAULT(1),
                                int connectivity CV_DEFAULT(8));

/******************* Iteration through the sequence tree *****************/
typedef struct CvTreeNodeIterator
{
    const void* node;
    int level;
    int maxLevel;
}
CvTreeNodeIterator;

OPENCVAPI void cvInitTreeNodeIterator( CvTreeNodeIterator* treeIterator,
                                   const void* first, int maxLevel );
OPENCVAPI void* cvNextTreeNode( CvTreeNodeIterator* treeIterator );
OPENCVAPI void* cvPrevTreeNode( CvTreeNodeIterator* treeIterator );

/* Inserts sequence into tree with specified "parent" sequence.
   If parent is equal to frame (e.g. the most external contour),
   then added contour will have null pointer to parent. */
OPENCVAPI void cvInsertNodeIntoTree( void* node, void* parent, void* frame );

/* Removes contour from tree (together with the contour children). */
OPENCVAPI void cvRemoveNodeFromTree( void* node, void* frame );

/* Gathers pointers to all the sequences,
   accessible from the <first>, to the single sequence */
OPENCVAPI CvSeq* cvTreeToNodeSeq( const void* first, int header_size,
                                  CvMemStorage* storage );

/* Approximates a single Freeman chain or a tree of chains to polygonal curves */
OPENCVAPI  CvSeq* cvApproxChains( CvSeq* src_seq, CvMemStorage* storage,
                            CvChainApproxMethod method CV_DEFAULT(CV_CHAIN_APPROX_SIMPLE),
                            double parameter CV_DEFAULT(0),
                            int  minimal_perimeter CV_DEFAULT(0),
                            int  recursive CV_DEFAULT(0));


/* Freeman chain reader state */
typedef struct CvChainPtReader
{
    CV_SEQ_READER_FIELDS()
    char      code;
    CvPoint  pt;
    char      deltas[8][2];
    int       reserved[2];
} CvChainPtReader;

/* Initalizes Freeman chain reader.
   The reader is used to iteratively get coordinates of all the chain points.
   If the original codes should be read, a simple sequence reader can be used */
OPENCVAPI  void  cvStartReadChainPoints( CvChain* chain, CvChainPtReader* reader );

/* Retrieve the next chain point */
OPENCVAPI  CvPoint   cvReadChainPoint( CvChainPtReader* reader );


/****************************************************************************************\
*                                  Motion Analysis                                       *
\****************************************************************************************/

/********************************** change detection ************************************/

/* Finds absolute difference between to arrays
   dst(x,y,c) = abs(srcA(x,y,c) - srcB(x,y,c)) */
OPENCVAPI  void  cvAbsDiff( const CvArr* srcA, const CvArr* srcB, CvArr* dst );


/* Finds absolute difference between an array and scalar
   dst(x,y,c) = abs(srcA(x,y,c) - value(c)) */
OPENCVAPI  void  cvAbsDiffS( const CvArr* src, CvArr* dst, CvScalar value );
#define cvAbs( src, dst ) cvAbsDiffS( (src), (dst), cvScalarAll(0))

/************************************ optical flow ***************************************/

/* Calculates optical flow for 2 images using classical Lucas & Kanade algorithm */
OPENCVAPI  void  cvCalcOpticalFlowLK( const CvArr* srcA, const CvArr* srcB,
                                      CvSize winSize, CvArr* velx, CvArr* vely );

/* Calculates optical flow for 2 images using block matching algorithm */
OPENCVAPI  void  cvCalcOpticalFlowBM( const CvArr* srcA, const CvArr* srcB,
                                      CvSize blockSize, CvSize shiftSize,
                                      CvSize maxRange, int usePrevious,
                                      CvArr* velx, CvArr* vely );

/* Calculates Optical flow for 2 images using Horn & Schunck algorithm */
OPENCVAPI  void  cvCalcOpticalFlowHS( const CvArr* srcA, const CvArr* srcB,
                                      int usePrevious, CvArr* velx, CvArr* vely,
                                      double lambda, CvTermCriteria criteria );

#define  CV_LKFLOW_PYR_A_READY       1
#define  CV_LKFLOW_PYR_B_READY       2
#define  CV_LKFLOW_INITIAL_GUESSES   4

/* It is Lucas & Kanade method, modified to use pyramids.
   Also it does several iterations to get optical flow for
   every point at every pyramid level.
   Calculates optical flow between two images for certain set of points (i.e.
   it is a "sparse" optical flow, which is opposite to the previous 3 methods) */
OPENCVAPI  void  cvCalcOpticalFlowPyrLK( const CvArr*  imgA, const CvArr*  imgB,
                                         CvArr*  pyrA, CvArr*  pyrB,
                                         CvPoint2D32f* featuresA,
                                         CvPoint2D32f* featuresB,
                                         int       count,
                                         CvSize    winSize,
                                         int       level,
                                         char*     status,
                                         float*    error,
                                         CvTermCriteria criteria,
                                         int       flags );


/* Modification of a previous sparse optical flow algorithm to calculate
   affine flow */
OPENCVAPI  void  cvCalcAffineFlowPyrLK( const CvArr*  imgA, const CvArr*  imgB,
                                        CvArr*  pyrA, CvArr*  pyrB,
                                        CvPoint2D32f* featuresA,
                                        CvPoint2D32f* featuresB,
                                        float*  matrices, int  count,
                                        CvSize  winSize, int  level,
                                        char*  status, float* error,
                                        CvTermCriteria criteria, int flags );

/********************************* motion templates *************************************/

/****************************************************************************************\
*        All the motion template functions work only with single channel images.         *
*        Silhouette image must have depth IPL_DEPTH_8U or IPL_DEPTH_8S                   *
*        Motion history image must have depth IPL_DEPTH_32F,                             *
*        Gradient mask - IPL_DEPTH_8U or IPL_DEPTH_8S,                                   *
*        Motion orientation image - IPL_DEPTH_32F                                        *
*        Segmentation mask - IPL_DEPTH_32F                                               *
*        All the angles are in degrees, all the times are in milliseconds                *
\****************************************************************************************/

/* Updates motion history image given motion silhouette */
OPENCVAPI  void    cvUpdateMotionHistory( const CvArr* silhouette, CvArr* mhi,
                                          double timestamp, double mhiDuration );

/* Calculates gradient of the motion history image and fills
   a mask indicating where the gradient is valid */
OPENCVAPI  void    cvCalcMotionGradient( const CvArr* mhi, CvArr* mask, CvArr* orientation,
                                         double delta1, double delta2,
                                         int aperture_size CV_DEFAULT(3));

/* Calculates average motion direction within a selected motion region 
   (region can be selected by setting ROIs and/or by composing a valid gradient mask
   with the region mask) */
OPENCVAPI  double  cvCalcGlobalOrientation( const CvArr* orientation, const CvArr* mask,
                                            const CvArr* mhi, double curr_mhi_timestamp,
                                            double mhi_duration );

/* Splits a motion history image into a few parts corresponding to separate independent motions
   (e.g. left hand, right hand) */
OPENCVAPI  CvSeq*  cvSegmentMotion( const CvArr* mhi, CvArr* seg_mask,
                                    CvMemStorage* storage,
                                    double timestamp, double seg_thresh );

/*********************** Background statistics accumulation *****************************/

/* Adds image to accumulator */
OPENCVAPI  void  cvAcc( const CvArr* image, CvArr* sum,
                        const CvArr* mask CV_DEFAULT(NULL) );

/* Adds squared image to accumulator */
OPENCVAPI  void  cvSquareAcc( const CvArr* image, CvArr* sqSum,
                              const CvArr* mask CV_DEFAULT(NULL) );

/* Adds a product of two images to accumulator */
OPENCVAPI  void  cvMultiplyAcc( const CvArr* imgA, const CvArr* imgB, CvArr* acc,
                                const CvArr* mask CV_DEFAULT(NULL) );

/* Adds image to accumulator with weights: imgU = imgU*(1-alpha) + imgY*alpha */
OPENCVAPI  void  cvRunningAvg( const CvArr* imgY, CvArr* imgU, double alpha,
                               const CvArr* mask CV_DEFAULT(NULL) );


/****************************************************************************************\
*                                       Tracking                                         *
\****************************************************************************************/

/* Implements CAMSHIFT algorithm - determines object position, size and orientation
   from the object histogram back project (extension of meanshift) */
OPENCVAPI int  cvCamShift( const CvArr* imgProb, CvRect  windowIn,
                           CvTermCriteria criteria, CvConnectedComp* out,
                           CvBox2D* box );

/* Implements MeanShift algorithm - determines object position
   from the object histogram back project */
OPENCVAPI int  cvMeanShift( const CvArr* imgProb, CvRect  windowIn,
                            CvTermCriteria criteria, CvConnectedComp* out );

typedef struct CvConDensation
{
    int MP;
    int DP;
    float* DynamMatr;       /* Matrix of the linear Dynamics system  */
    float* State;           /* Vector of State                       */
    int SamplesNum;         /* Number of the Samples                 */
    float** flSamples;      /* array of the Sample Vectors           */
    float** flNewSamples;   /* temporary array of the Sample Vectors */
    float* flConfidence;    /* Confidence for each Sample            */
    float* flCumulative;    /* Cumulative confidence                 */
    float* Temp;            /* Temporary vector                      */
    float* RandomSample;    /* RandomVector to update sample set     */
    CvRandState* RandS;     /* Array of structures to generate random vectors */
} CvConDensation;

/* Creates ConDensation filter state */
OPENCVAPI CvConDensation*  cvCreateConDensation( int DP, int MP, int SamplesNum);

/* Releases ConDensation filter state */
OPENCVAPI void  cvReleaseConDensation( CvConDensation** ConDensation);

/* Updates ConDensation filter by time (predict future state of the system) */
OPENCVAPI void  cvConDensUpdateByTime( CvConDensation* ConDens);

/* Initializes ConDensation filter samples  */
OPENCVAPI void  cvConDensInitSampleSet( CvConDensation* conDens, CvMat* lowerBound,CvMat* upperBound);

/*
standard Kalman filter (in G. Welch' and G. Bishop's notation):

  x(k)=A*x(k-1)+B*u(k)+w(k)  p(w)~N(0,Q)
  z(k)=H*x(k)+v(k),   p(v)~N(0,R)
*/
typedef struct CvKalman
{
    int MP;                     /* number of measurement vector dimensions */
    int DP;                     /* number of state vector dimensions */
    int CP;                     /* number of control vector dimensions */

    /* backward compatibility fields */
#if 1
    float* PosterState;         /* =state_pre->data.fl */
    float* PriorState;          /* =state_post->data.fl */
    float* DynamMatr;           /* =transition_matrix->data.fl */
    float* MeasurementMatr;     /* =measurement_matrix->data.fl */
    float* MNCovariance;        /* =measurement_noise_cov->data.fl */
    float* PNCovariance;        /* =process_noise_cov->data.fl */
    float* KalmGainMatr;        /* =gain->data.fl */
    float* PriorErrorCovariance;/* =error_cov_pre->data.fl */
    float* PosterErrorCovariance;/* =error_cov_post->data.fl */
    float* Temp1;               /* temp1->data.fl */
    float* Temp2;               /* temp2->data.fl */
#endif

    CvMat* state_pre;           /* predicted state (x'(k)):
                                    x(k)=A*x(k-1)+B*u(k) */
    CvMat* state_post;          /* corrected state (x(k)):
                                    x(k)=x'(k)+K(k)*(z(k)-H*x'(k)) */
    CvMat* transition_matrix;   /* state transition matrix (A) */
    CvMat* control_matrix;      /* control matrix (B)
                                   (it is not used if there is no control)*/
    CvMat* measurement_matrix;  /* measurement matrix (H) */
    CvMat* process_noise_cov;   /* process noise covariance matrix (Q) */
    CvMat* measurement_noise_cov; /* measurement noise covariance matrix (R) */
    CvMat* error_cov_pre;       /* priori error estimate covariance matrix (P'(k)):
                                    P'(k)=A*P(k-1)*At + Q)*/
    CvMat* gain;                /* Kalman gain matrix (K(k)):
                                    K(k)=P'(k)*Ht*inv(H*P'(k)*Ht+R)*/
    CvMat* error_cov_post;      /* posteriori error estimate covariance matrix (P(k)):
                                    P(k)=(I-K(k)*H)*P'(k) */
    CvMat* temp1;               /* temporary matrices */
    CvMat* temp2;
    CvMat* temp3;
    CvMat* temp4;
    CvMat* temp5;

}
CvKalman;

/* Creates Kalman filter and sets A, B, Q, R and state to some initial values */
OPENCVAPI CvKalman* cvCreateKalman( int dynamParams, int measureParams,
                                    int controlParams CV_DEFAULT(0));

/* Releases Kalman filter state */
OPENCVAPI void  cvReleaseKalman( CvKalman** Kalman);

/* Updates Kalman filter by time (predicts future state of the system) */
OPENCVAPI const CvMat*  cvKalmanPredict( CvKalman* Kalman,
                                         const CvMat* control CV_DEFAULT(NULL));

/* Updates Kalman filter by measurement
   (corrects state of the system and internal matrices) */
OPENCVAPI const CvMat*  cvKalmanCorrect( CvKalman* Kalman, const CvMat* measurement );

/****************************************************************************************\
*                              Planar subdivisions                                       *
\****************************************************************************************/

/************ Data structures and related enumerations ************/

typedef long CvSubdiv2DEdge;

#define CV_QUADEDGE2D_FIELDS()     \
    int flags;                     \
    struct CvSubdiv2DPoint* pt[4]; \
    CvSubdiv2DEdge  next[4];

#define CV_SUBDIV2D_POINT_FIELDS()\
    int            flags;      \
    CvSubdiv2DEdge first;      \
    CvPoint2D32f   pt;

#define CV_SUBDIV2D_VIRTUAL_POINT_FLAG (1 << 30)

typedef struct CvQuadEdge2D
{
    CV_QUADEDGE2D_FIELDS()
}
CvQuadEdge2D;

typedef struct CvSubdiv2DPoint
{
    CV_SUBDIV2D_POINT_FIELDS()
}
CvSubdiv2DPoint;

#define CV_SUBDIV2D_FIELDS()    \
    CV_GRAPH_FIELDS()           \
    int  quad_edges;            \
    int  is_geometry_valid;     \
    CvSubdiv2DEdge recent_edge; \
    CvPoint2D32f  topleft;      \
    CvPoint2D32f  bottomright;
    
typedef struct CvSubdiv2D
{
    CV_SUBDIV2D_FIELDS()
}
CvSubdiv2D;


typedef enum CvSubdiv2DPointLocation
{
    CV_PTLOC_ERROR = -2,
    CV_PTLOC_OUTSIDE_RECT = -1,
    CV_PTLOC_INSIDE = 0,
    CV_PTLOC_VERTEX = 1,
    CV_PTLOC_ON_EDGE = 2
}
CvSubdiv2DPointLocation;

typedef enum CvNextEdgeType
{
    CV_NEXT_AROUND_ORG   = 0x00,
    CV_NEXT_AROUND_DST   = 0x22,
    CV_PREV_AROUND_ORG   = 0x11,
    CV_PREV_AROUND_DST   = 0x33,
    CV_NEXT_AROUND_LEFT  = 0x13,
    CV_NEXT_AROUND_RIGHT = 0x31,
    CV_PREV_AROUND_LEFT  = 0x20,
    CV_PREV_AROUND_RIGHT = 0x02
}
CvNextEdgeType;

/* get the next edge with the same origin point (counterwise) */
#define  CV_SUBDIV2D_NEXT_EDGE( edge )  (((CvQuadEdge2D*)((edge) & ~3))->next[(edge)&3])


/* Initializes Delaunay triangulation */
OPENCVAPI  void  cvInitSubdivDelaunay2D( CvSubdiv2D* subdiv, CvRect rect );

/* Creates new subdivision */
OPENCVAPI  CvSubdiv2D*  cvCreateSubdiv2D( int subdiv_type, int header_size,
                                       int vtx_size, int quadedge_size,
                                       CvMemStorage* storage );

/************************* high-level subdivision functions ***************************/

/* Simplified Delaunay diagram creation */
CV_INLINE  CvSubdiv2D* cvCreateSubdivDelaunay2D( CvRect rect, CvMemStorage* storage );
CV_INLINE  CvSubdiv2D* cvCreateSubdivDelaunay2D( CvRect rect, CvMemStorage* storage )
{
    CvSubdiv2D* subdiv = cvCreateSubdiv2D( CV_SEQ_KIND_SUBDIV2D, sizeof(*subdiv),
                         sizeof(CvSubdiv2DPoint), sizeof(CvQuadEdge2D), storage );

    cvInitSubdivDelaunay2D( subdiv, rect );
    return subdiv;
}


/* Inserts new point to the Delaunay triangulation */
OPENCVAPI  CvSubdiv2DPoint*  cvSubdivDelaunay2DInsert( CvSubdiv2D* subdiv, CvPoint2D32f pt);

/* Locates a point within the Delaunay triangulation (finds the edge
   the point is left to or belongs to, or the triangulation point the given
   point coinsides with */
OPENCVAPI  CvSubdiv2DPointLocation  cvSubdiv2DLocate(
                               CvSubdiv2D* subdiv, CvPoint2D32f pt,
                               CvSubdiv2DEdge *_edge,
                               CvSubdiv2DPoint** _point CV_DEFAULT(NULL) );

/* Calculates Voronoi tesselation (i.e. coordinates of Voronoi points) */
OPENCVAPI  void  cvCalcSubdivVoronoi2D( CvSubdiv2D* subdiv );


/* Removes all Voronoi points from the tesselation */
OPENCVAPI  void  cvClearSubdivVoronoi2D( CvSubdiv2D* subdiv );


/* Finds the nearest to the given point vertex in subdivision. */
OPENCVAPI CvSubdiv2DPoint* cvFindNearestPoint2D( CvSubdiv2D* subdiv, CvPoint2D32f pt );


/************ Basic quad-edge navigation and operations ************/

CV_INLINE  CvSubdiv2DEdge  cvSubdiv2DNextEdge( CvSubdiv2DEdge edge );
CV_INLINE  CvSubdiv2DEdge  cvSubdiv2DNextEdge( CvSubdiv2DEdge edge )
{
    return  CV_SUBDIV2D_NEXT_EDGE(edge);
}


CV_INLINE  CvSubdiv2DEdge  cvSubdiv2DRotateEdge( CvSubdiv2DEdge edge, int rotate );
CV_INLINE  CvSubdiv2DEdge  cvSubdiv2DRotateEdge( CvSubdiv2DEdge edge, int rotate )
{
    return  (edge & ~3) + ((edge + rotate) & 3);
}

CV_INLINE  CvSubdiv2DEdge  cvSubdiv2DSymEdge( CvSubdiv2DEdge edge );
CV_INLINE  CvSubdiv2DEdge  cvSubdiv2DSymEdge( CvSubdiv2DEdge edge )
{
    return edge ^ 2;
}

CV_INLINE  CvSubdiv2DEdge  cvSubdiv2DGetEdge( CvSubdiv2DEdge edge, CvNextEdgeType type );
CV_INLINE  CvSubdiv2DEdge  cvSubdiv2DGetEdge( CvSubdiv2DEdge edge, CvNextEdgeType type )
{
    CvQuadEdge2D* e = (CvQuadEdge2D*)(edge & ~3);
    edge = e->next[(edge + (int)type) & 3];
    return  (edge & ~3) + ((edge + ((int)type >> 4)) & 3);
}


CV_INLINE  CvSubdiv2DPoint*  cvSubdiv2DEdgeOrg( CvSubdiv2DEdge edge );
CV_INLINE  CvSubdiv2DPoint*  cvSubdiv2DEdgeOrg( CvSubdiv2DEdge edge )
{
    CvQuadEdge2D* e = (CvQuadEdge2D*)(edge & ~3);
    return (CvSubdiv2DPoint*)e->pt[edge & 3];
}


CV_INLINE  CvSubdiv2DPoint*  cvSubdiv2DEdgeDst( CvSubdiv2DEdge edge );
CV_INLINE  CvSubdiv2DPoint*  cvSubdiv2DEdgeDst( CvSubdiv2DEdge edge )
{
    CvQuadEdge2D* e = (CvQuadEdge2D*)(edge & ~3);
    return (CvSubdiv2DPoint*)e->pt[(edge + 2) & 3];
}


CV_INLINE  double  cvTriangleArea( CvPoint2D32f a, CvPoint2D32f b, CvPoint2D32f c );
CV_INLINE  double  cvTriangleArea( CvPoint2D32f a, CvPoint2D32f b, CvPoint2D32f c )
{
    return (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
}


/****************************************************************************************\
*                            Contour Processing and Shape Analysis                       *
\****************************************************************************************/

#define CV_POLY_APPROX_DP 0

/* Approximates a single polygonal curve (contour) or
   a tree of polygonal curves (contours) */
OPENCVAPI  CvSeq*  cvApproxPoly( const void* src_seq,
                                 int header_size, CvMemStorage* storage,
                                 int method, double parameter,
                                 int parameter2 CV_DEFAULT(0));

/* Calculates perimeter of a contour or a part of contour */
OPENCVAPI  double  cvArcLength( const void* curve,
                                CvSlice slice CV_DEFAULT(CV_WHOLE_SEQ),
                                int is_closed CV_DEFAULT(-1));
#define cvContourPerimeter( contour ) cvArcLength( contour, CV_WHOLE_SEQ, 1 )

/* Calculates contour boundning rectangle (update=1) or
   just retrieves pre-calculated rectangle (update=0) */
OPENCVAPI  CvRect  cvBoundingRect( const void* points, int update CV_DEFAULT(0) );

/* Calculates area of a contour or contour segment */
OPENCVAPI  double  cvContourArea( const void* contour,
                                  CvSlice slice CV_DEFAULT(CV_WHOLE_SEQ));

/* Finds minimum area rotated rectangle bounding a set of points */
OPENCVAPI  CvBox2D  cvMinAreaRect2( const void* points,
                                    CvMemStorage* storage CV_DEFAULT(NULL));

/* Finds minimum enclosing circle for a set of points */
OPENCVAPI  void  cvMinEnclosingCircle( const void* points,
                                       CvPoint2D32f* center, float* radius );

#define CV_CONTOURS_MATCH_I1  1
#define CV_CONTOURS_MATCH_I2  2
#define CV_CONTOURS_MATCH_I3  3

/* Compares two contours by matching their moments */
OPENCVAPI  double  cvMatchShapes( const void* contour1, const void* contour2,
                                  int method, double parameter CV_DEFAULT(0));

/* Contour tree header */
typedef struct CvContourTree
{
    CV_SEQUENCE_FIELDS()
    CvPoint p1;            /* the first point of the binary tree root segment */
    CvPoint p2;            /* the last point of the binary tree root segment */
} CvContourTree;

/* Builds hierarhical representation of a contour */
OPENCVAPI  CvContourTree*   cvCreateContourTree( CvSeq* contour, CvMemStorage* storage,
                                                 double threshold );

/* Reconstruct (completelly or partially) contour a from contour tree */
OPENCVAPI  CvSeq*  cvContourFromContourTree( CvContourTree *tree,
                                          CvMemStorage* storage,
                                          CvTermCriteria criteria );

/* Compares two contour trees */
#define  CV_CONTOUR_TREES_MATCH_I1  1

OPENCVAPI  double  cvMatchContourTrees( CvContourTree *tree1,
                                        CvContourTree *tree2,
                                        int method, double threshold );

/* Calculates histogram of a contour */
OPENCVAPI  void  cvCalcPGH( const CvSeq* contour, CvHistogram* hist );

#define CV_CLOCKWISE         1
#define CV_COUNTER_CLOCKWISE 2

/* Calculates exact convex hull of 2d point set */
OPENCVAPI CvSeq* cvConvexHull2( const CvArr* input,
                                void* hull_storage CV_DEFAULT(NULL),
                                int orientation CV_DEFAULT(CV_CLOCKWISE),
                                int return_points CV_DEFAULT(0));

/* Checks whether the contour is convex or not (returns 1 if convex, 0 if not) */
OPENCVAPI  int  cvCheckContourConvexity( const CvArr* contour );

/* Finds a sequence of convexity defects of given contour */
typedef struct CvConvexityDefect
{
    CvPoint* start; /* point of the contour where the defect begins */
    CvPoint* end; /* point of the contour where the defect ends */
    CvPoint* depth_point; /* the farthest from the convex hull point within the defect */
    float depth; /* distance between the farthest point and the convex hull */
} CvConvexityDefect;


/* Finds convexity defects for the contour */
OPENCVAPI CvSeq*  cvConvexityDefects( const CvArr* contour, const CvArr* convexhull,
                                      CvMemStorage* storage CV_DEFAULT(NULL));

/* Fits ellipse into a set of 2d points */
OPENCVAPI CvBox2D cvFitEllipse2( const CvArr* points );

/****************************************************************************************\
*                                  Histogram functions                                   *
\****************************************************************************************/

/* Creates new histogram */
OPENCVAPI  CvHistogram*  cvCreateHist( int dims, int* sizes, int type,
                                       float** ranges CV_DEFAULT(NULL),
                                       int uniform CV_DEFAULT(1));

/* Assignes histogram bin ranges */
OPENCVAPI void  cvSetHistBinRanges( CvHistogram* hist, float** ranges,
                                    int uniform CV_DEFAULT(1));

/* Creates histogram header for array */
OPENCVAPI  CvHistogram*  cvMakeHistHeaderForArray(
                            int  dims, int* sizes, CvHistogram* hist,
                            float* data, float** ranges CV_DEFAULT(NULL),
                            int uniform CV_DEFAULT(1));

/* Releases histogram */
OPENCVAPI  void  cvReleaseHist( CvHistogram** hist );

/* Clears all the histogram bins */
OPENCVAPI  void  cvClearHist( CvHistogram* hist );

/* Finds indices and values of minimum and maximum histogram bins */
OPENCVAPI  void  cvGetMinMaxHistValue( const CvHistogram* hist,
                                    float* value_min, float* value_max,
                                    int* idx_min CV_DEFAULT(NULL),
                                    int* idx_max CV_DEFAULT(NULL));


/* Normalizes histogram by dividing all bins by sum of the bins, multiplied by <factor>.
   After that sum of histogram bins is equal to <factor> */
OPENCVAPI  void  cvNormalizeHist( CvHistogram* hist, double factor );


/* Clear all histogram bins that are below the threshold */
OPENCVAPI  void  cvThreshHist( CvHistogram* hist, double thresh );

#define CV_COMP_CORREL      0
#define CV_COMP_CHISQR      1
#define CV_COMP_INTERSECT   2

/* Compares two histogram */
OPENCVAPI  double  cvCompareHist( const CvHistogram* hist1,
                                  const CvHistogram* hist2,
                                  int method);

/* Copies one histogram to another. Destination histogram is created if
   the destination pointer is NULL */
OPENCVAPI void  cvCopyHist( const CvHistogram* src, CvHistogram** dst );


/* Calculates bayesian probabilistic histograms
   (each or src and dst is an array of <number> histograms */
OPENCVAPI void  cvCalcBayesianProb( CvHistogram** src, int number,
                                    CvHistogram** dst);

/* Calculates array histogram */
OPENCVAPI  void  cvCalcArrHist( CvArr** arr, CvHistogram* hist,
                                int doNotClear CV_DEFAULT(0),
                                const CvArr* mask CV_DEFAULT(NULL) );

CV_INLINE  void  cvCalcHist( IplImage** img, CvHistogram* hist,
                             int doNotClear CV_DEFAULT(0),
                             const CvArr* mask CV_DEFAULT(NULL) );
CV_INLINE  void  cvCalcHist( IplImage** img, CvHistogram* hist,
                             int doNotClear, const CvArr* mask )
{
    cvCalcArrHist( (CvArr**)img, hist, doNotClear, mask );
}

/* Calculates contrast histogram */
OPENCVAPI  void  cvCalcContrastHist( CvArr** img, CvHistogram* hist,
                                     int doNotClear, IplImage* mask );

/* Calculates back project */
OPENCVAPI  void  cvCalcArrBackProject( CvArr** img, CvArr* dst,
                                       const CvHistogram* hist );
#define  cvCalcBackProject(img, dst, hist) cvCalcArrBackProject((CvArr**)img, dst, hist)


/* Does some sort of template matching but compares histograms of
   template and each window location */
OPENCVAPI  void  cvCalcArrBackProjectPatch( CvArr** img, CvArr* dst, CvSize range,
                                            CvHistogram* hist, int method,
                                            double normFactor );
#define  cvCalcBackProjectPatch( img, dst, range, hist, method, normFactor ) \
     cvCalcArrBackProjectPatch( (CvArr**)img, dst, range, hist, method, normFactor )


/* calculates probabilistic density (divides one histogram by another) */
OPENCVAPI  void  cvCalcProbDensity( const CvHistogram* hist, const CvHistogram* hist_mask,
                                    CvHistogram* hist_dens, double scale CV_DEFAULT(255) );


#define  CV_VALUE  1
#define  CV_ARRAY  2
/* Updates active contour in order to minimize its cummulative
   (internal and external) energy. */
OPENCVAPI  void  cvSnakeImage( const IplImage* src, CvPoint* points,
                            int  length, float* alpha,
                            float* beta, float* gamma,
                            int coeffUsage, CvSize  win,
                            CvTermCriteria criteria, int calcGradient CV_DEFAULT(1));

/* Calculates the cooficients of the homography matrix */
OPENCVAPI  void  cvCalcImageHomography(float *line, CvPoint3D32f* center,
                                     float* intrinsic, float* homography);

#define CV_DIST_MASK_3   3
#define CV_DIST_MASK_5   5 

/* Applies distance transform to binary image */
OPENCVAPI  void  cvDistTransform( const CvArr* src, CvArr* dst,
                                  CvDisType disType CV_DEFAULT(CV_DIST_L2),
                                  int maskSize CV_DEFAULT(3),
                                  const float* mask CV_DEFAULT(NULL));


/* Defines for Threshold functions */
#define CV_THRESH_BINARY      0  /* val = (val>thresh? MAX:0)      */
#define CV_THRESH_BINARY_INV  1  /* val = (val>thresh? 0:MAX)      */
#define CV_THRESH_TRUNC       2  /* val = (val>thresh? thresh:val) */
#define CV_THRESH_TOZERO      3  /* val = (val>thresh? val:0)      */
#define CV_THRESH_TOZERO_INV  4  /* val = (val>thresh? 0:val)      */

/* Applies fixed-level threshold to grayscale image. This is the basic operation
   to be performed before retrieving contours */
OPENCVAPI  void  cvThreshold( const CvArr*  src, CvArr*  dst,
                              double  thresh, double  maxValue, int type );

#define CV_ADAPTIVE_THRESH_MEAN_C  0
#define CV_ADAPTIVE_THRESH_GAUSSIAN_C  1

/* Applies adaptive threshold to grayscale image.
   The two parameters for methods CV_ADAPTIVE_THRESH_MEAN_C and
   CV_ADAPTIVE_THRESH_GAUSSIAN_C are:
   neighborhood size (3, 5, 7 etc.),
   and a constant subtracted from mean (...,-3,-2,-1,0,1,2,3,...) */
OPENCVAPI  void  cvAdaptiveThreshold( const CvArr* src, CvArr* dst, double maxValue,
                                      int method CV_DEFAULT(CV_ADAPTIVE_THRESH_MEAN_C),
                                      int type CV_DEFAULT(CV_THRESH_BINARY),
                                      int blockSize CV_DEFAULT(3),
                                      double param1 CV_DEFAULT(5));

#define CV_FLOODFILL_FIXED_RANGE (1 << 16)
#define CV_FLOODFILL_MASK_ONLY   (1 << 17)

/* Fills the connected component until the color difference gets large enough */
OPENCVAPI  void  cvFloodFill( CvArr* array, CvPoint seedPoint,
                              double newVal, double loDiff CV_DEFAULT(0),
                              double upDiff CV_DEFAULT(0),
                              CvConnectedComp* comp CV_DEFAULT(NULL),
                              int flags CV_DEFAULT(4),
                              CvArr* mask CV_DEFAULT(NULL));

/****************************************************************************************\
*                                  Feature detection                                     *
\****************************************************************************************/

/* Runs canny edge detector */
OPENCVAPI  void  cvCanny( const CvArr* src, CvArr* dst, double low_threshold,
                          double high_threshold, int  aperture_size CV_DEFAULT(3) );

/* Calculates constraint image for corner detection
   Dx^2 * Dyy + Dxx * Dy^2 - 2 * Dx * Dy * Dxy.
   Applying threshold to the result gives coordinates of corners */
OPENCVAPI void cvPreCornerDetect( const CvArr* src, CvArr* dst,
                                  int aperture_size CV_DEFAULT(3) );

/* Calculates eigen values and vectors of 2x2
   gradient matrix at every image pixel */
OPENCVAPI void  cvCornerEigenValsAndVecs( const CvArr* src, CvArr* eigenvv,
                                          int blockSize,
                                          int aperture_size CV_DEFAULT(3) );

/* Calculates minimal eigenvalue for 2x2 gradient matrix at
   every image pixel */
OPENCVAPI void  cvCornerMinEigenVal( const CvArr* src, CvArr* eigenval,
                                     int blockSize, int aperture_size CV_DEFAULT(3) );

/* Adjust corner position using some sort of gradient search */
OPENCVAPI  void  cvFindCornerSubPix( const CvArr* src,CvPoint2D32f*  corners,
                                     int count, CvSize win,CvSize zero_zone,
                                     CvTermCriteria  criteria );

/* Finds a sparse set of points within the selected region
   that seem to be easy to track */
OPENCVAPI void  cvGoodFeaturesToTrack( const CvArr* image, CvArr* eig_image,
                                       CvArr* temp_image, CvPoint2D32f* corners,
                                       int* corner_count, double  quality_level,
                                       double  min_distance,
                                       const CvArr* mask CV_DEFAULT(NULL));

#define CV_HOUGH_STANDARD 0
#define CV_HOUGH_PROBABILISTIC 1
#define CV_HOUGH_MULTI_SCALE 2

/* Finds lines on binary image using one of several methods.
   lineStorage is either memory storage or 1 x maxNumberOfLines CvMat, its
   number of columns is changed by the function.
   method is one of CV_HOUGH_*;
   rho, theta and threshold are used for each of those methods;
   param1 ~ lineLength, param2 ~ lineGap - for probabilistic,
   param1 ~ srn, param2 ~ stn - for multi-scale */
OPENCVAPI  CvSeq*  cvHoughLines2( CvArr* image, void* line_storage, int method, 
                                  double rho, double theta, int threshold,
                                  double param1 CV_DEFAULT(0), double param2 CV_DEFAULT(0));

/* Projects 2d points to one of standard coordinate planes
   (i.e. removes one of coordinates) */
OPENCVAPI  void  cvProject3D( CvPoint3D32f* points3D, int count,
                              CvPoint2D32f* points2D, int xIndx, int yIndx );

/* Fits a line into set of 2d or 3d points in a robust way (M-estimator technique) */
OPENCVAPI  void  cvFitLine( const CvArr* points, CvDisType dist, double param,
                            double reps, double aeps, float* line );


#define CV_EIGOBJ_NO_CALLBACK     0
#define CV_EIGOBJ_INPUT_CALLBACK  1
#define CV_EIGOBJ_OUTPUT_CALLBACK 2
#define CV_EIGOBJ_BOTH_CALLBACK   3


CV_EXTERN_C_FUNCPTR(CvStatus (CV_CDECL * CvCallback)
                    (int index, void* buffer, void* userData));

typedef union
{
    CvCallback callback;
    void* data;
}
CvInput;

/* Calculates covariation matrix of a set of arrays */
OPENCVAPI  void  cvCalcCovarMatrixEx( int nObjects, void* input, int ioFlags,
                                      int ioBufSize, uchar* buffer, void* userData,
                                      IplImage* avg, float* covarMatrix );

/* Calculates eigen values and vectors of covariation matrix of a set of
   arrays */
OPENCVAPI  void  cvCalcEigenObjects( int nObjects, void* input, void* output,
                                    int ioFlags, int ioBufSize, void* userData,
                                    CvTermCriteria* calcLimit, IplImage* avg,
                                    float* eigVals );

/* Calculates dot product (obj - avg) * eigObj (i.e. projects image to eigen vector) */
OPENCVAPI  double  cvCalcDecompCoeff( IplImage* obj, IplImage* eigObj, IplImage* avg );

/* Projects image to eigen space (finds all decomposion coefficients */
OPENCVAPI  void  cvEigenDecomposite( IplImage* obj, int nEigObjs, void* eigInput,
                                    int ioFlags, void* userData, IplImage* avg,
                                    float* coeffs );

/* Projects original objects used to calculate eigen space basis to that space */
OPENCVAPI  void  cvEigenProjection( void* eigInput, int nEigObjs, int ioFlags,
                                   void* userData, float* coeffs, IplImage* avg,
                                   IplImage* proj );

/*********************************** HMM structures *************************************/
typedef struct CvEHMMState
{
    int num_mix;      /*number of mixtures in this state*/
    float* mu;        /*mean vectors corresponding to each mixture*/
    float* inv_var; /* square root of inversed variances corresp. to each mixture*/
    float* log_var_val; /* sum of 0.5 (LN2PI + ln(variance[i]) ) for i=1,n */
    float* weight;   /*array of mixture weights. Summ of all weights in state is 1. */

} CvEHMMState;

typedef struct CvEHMM
{
    int level; /* 0 - lowest(i.e its states are real states), ..... */
    int num_states; /* number of HMM states */
    float*  transP;/*transition probab. matrices for states */
    float** obsProb; /* if level == 0 - array of brob matrices corresponding to hmm
                        if level == 1 - martix of matrices */
    union
    {
        CvEHMMState* state; /* if level == 0 points to real states array,
                               if not - points to embedded hmms */
        struct CvEHMM* ehmm; /* pointer to an embedded model or NULL, if it is a leaf */
    } u;

} CvEHMM;

/* Creates 2D HMM */
OPENCVAPI  CvEHMM*  cvCreate2DHMM( int* stateNumber, int* numMix, int obsSize );


/* Releases HMM */
OPENCVAPI  void  cvRelease2DHMM( CvEHMM** hmm );



#define CV_COUNT_OBS(roi, win, delta, numObs )                                       \
{                                                                                    \
   (numObs)->width  =((roi)->width  -(win)->width  +(delta)->width)/(delta)->width;  \
   (numObs)->height =((roi)->height -(win)->height +(delta)->height)/(delta)->height;\
}

typedef struct CvImgObsInfo
{
    int obs_x;
    int obs_y;
    int obs_size;
    float* obs;//consequtive observations

    int* state;/* array of pairs superstate/state to which observation belong */
    int* mix;  /* number of mixture to which observation belong */

} CvImgObsInfo;/*struct for 1 image*/

/* Creates storage for observation vectors */
OPENCVAPI  CvImgObsInfo*  cvCreateObsInfo( CvSize numObs, int obsSize );

/* Releases storage for observation vectors */
OPENCVAPI  void  cvReleaseObsInfo( CvImgObsInfo** obs_info );


/* The function takes an image on input and and returns the sequnce of observations
   to be used with an embedded HMM; Each observation is top-left block of DCT
   coefficient matrix */
OPENCVAPI  void  cvImgToObs_DCT( const CvArr* array, float* obs, CvSize dctSize,
                                 CvSize obsSize, CvSize delta );


/* Uniformly segments all observation vectors extracted from image */
OPENCVAPI  void  cvUniformImgSegm( CvImgObsInfo* obs_info, CvEHMM* ehmm );

/* Does mixture segmentation of the states of embedded HMM */
OPENCVAPI  void  cvInitMixSegm( CvImgObsInfo** obs_info_array,
                               int num_img, CvEHMM* hmm );

/* Function calculates means, variances, weights of every Gaussian mixture
   of every low-level state of embedded HMM */
OPENCVAPI  void  cvEstimateHMMStateParams( CvImgObsInfo** obs_info_array,
                                        int num_img, CvEHMM* hmm );

/* Function computes transition probability matrices of embedded HMM
   given observations segmentation */
OPENCVAPI  void  cvEstimateTransProb( CvImgObsInfo** obs_info_array,
                                   int num_img, CvEHMM* hmm );

/* Function computes probabilities of appearing observations at any state
   (i.e. computes P(obs|state) for every pair(obs,state)) */
OPENCVAPI  void  cvEstimateObsProb( CvImgObsInfo* obs_info,
                                   CvEHMM* hmm );

/* Runs Viterbi algorithm for embedded HMM */
OPENCVAPI  float  cvEViterbi( CvImgObsInfo* obs_info, CvEHMM* hmm );


/* Function clusters observation vectors from several images
   given observations segmentation.
   Euclidean distance used for clustering vectors.
   Centers of clusters are given means of every mixture */
OPENCVAPI  void  cvMixSegmL2( CvImgObsInfo** obs_info_array,
                             int num_img, CvEHMM* hmm );

/* The function implements the K-means algorithm for clustering an array of sample
   vectors in a specified number of classes */
OPENCVAPI  void  cvKMeans2( const CvArr* samples, int cluster_count,
                            CvArr* cluster_idx, CvTermCriteria termcrit );

/****************************************************************************************\
*                     Camera Calibration and Rectification functions                     *
\****************************************************************************************/

/* The function corrects radial and tangential image distortion using known
   matrix of the camera intrinsic parameters and distortion coefficients */
OPENCVAPI  void  cvUnDistortOnce( const CvArr* srcImage, CvArr* dstImage,
                                  const float* intrMatrix,
                                  const float* distCoeffs,
                                  int interpolate CV_DEFAULT(1) );

/* The function calculates map of distorted points indices and
   interpolation coefficients for cvUnDistort function using known
   matrix of the camera intrinsic parameters and distortion coefficients */
OPENCVAPI  void  cvUnDistortInit( const CvArr* srcImage, CvArr* undistMap,
                                  const float* intrMatrix,
                                  const float* distCoeffs,
                                  int interpolate CV_DEFAULT(1) );

/* The function corrects radial and tangential image distortion
   using previousely calculated (via cvUnDistortInit) map */
OPENCVAPI  void  cvUnDistort( const CvArr* srcImage, CvArr* dstImage,
                              const CvArr* undistMap, int interpolate CV_DEFAULT(1));
#define cvRemap cvUnDistort


/* The function converts floating-point pixel coordinate map to
   faster fixed-point map, used by cvUnDistort (cvRemap) */
OPENCVAPI  void  cvConvertMap( const CvArr* srcImage, const CvArr* flUndistMap,
                               CvArr* undistMap, int iterpolate CV_DEFAULT(1) );

/* Calibrates camera using multiple views of calibration pattern */
OPENCVAPI  void  cvCalibrateCamera( int           numImages,
                                    int*          numPoints,
                                    CvSize        imageSize,
                                    CvPoint2D32f* imagePoints32f,
                                    CvPoint3D32f* objectPoints32f,
                                    CvVect32f     distortion32f,
                                    CvMatr32f     cameraMatrix32f,
                                    CvVect32f     transVects32f,
                                    CvMatr32f     rotMatrs32f,
                                    int           useIntrinsicGuess);

/* Variant of the previous function that takes double-precision parameters */
OPENCVAPI  void  cvCalibrateCamera_64d( int           numImages,
                                       int*          numPoints,
                                       CvSize        imageSize,
                                       CvPoint2D64d* imagePoints,
                                       CvPoint3D64d* objectPoints,
                                       CvVect64d     distortion,
                                       CvMatr64d     cameraMatrix,
                                       CvVect64d     transVects,
                                       CvMatr64d     rotMatrs,
                                       int           useIntrinsicGuess );

/* Find 3d position of object given intrinsic camera parameters,
   3d model of the object and projection of the object into view plane */
OPENCVAPI  void  cvFindExtrinsicCameraParams( int           numPoints,
                                             CvSize        imageSize,
                                             CvPoint2D32f* imagePoints32f,
                                             CvPoint3D32f* objectPoints32f,
                                             CvVect32f     focalLength32f,
                                             CvPoint2D32f  principalPoint32f,
                                             CvVect32f     distortion32f,
                                             CvVect32f     rotVect32f,
                                             CvVect32f     transVect32f);

/* Variant of the previous function that takes double-precision parameters */
OPENCVAPI  void  cvFindExtrinsicCameraParams_64d( int           numPoints,
                                                 CvSize        imageSize,
                                                 CvPoint2D64d* imagePoints,
                                                 CvPoint3D64d* objectPoints,
                                                 CvVect64d     focalLength,
                                                 CvPoint2D64d  principalPoint,
                                                 CvVect64d     distortion,
                                                 CvVect64d     rotVect,
                                                 CvVect64d     transVect);


/* Rodrigues transform */
#define CV_RODRIGUES_M2V  0
#define CV_RODRIGUES_V2M  1

/* Converts rotation matrix to rotation vector or vice versa */
OPENCVAPI  void  cvRodrigues( CvMat* rotMatrix, CvMat* rotVector,
                              CvMat* jacobian, int convType);

/* Does reprojection of 3d object points to the view plane */
OPENCVAPI  void  cvProjectPoints( int             numPoints,
                                 CvPoint3D64d*   objectPoints,
                                 CvVect64d       rotVect,
                                 CvVect64d       transVect,
                                 CvVect64d       focalLength,
                                 CvPoint2D64d    principalPoint,
                                 CvVect64d       distortion,
                                 CvPoint2D64d*   imagePoints,
                                 CvVect64d       derivPointsRot,
                                 CvVect64d       derivPointsTrans,
                                 CvVect64d       derivPointsFocal,
                                 CvVect64d       derivPointsPrincipal,
                                 CvVect64d       derivPointsDistort);

/* Simpler version of the previous function */
OPENCVAPI void cvProjectPointsSimple(  int numPoints,
                                    CvPoint3D64d * objectPoints,
                                    CvVect64d rotMatr,
                                    CvVect64d transVect,
                                    CvMatr64d cameraMatrix,
                                    CvVect64d distortion,
                                    CvPoint2D64d* imagePoints);
                                    
/* Detects corners on a chess-board - "brand" OpenCV calibration pattern */
OPENCVAPI  int  cvFindChessBoardCornerGuesses( const CvArr* array, CvArr* thresh,
                                               CvMemStorage* storage, CvSize etalon_size,
                                               CvPoint2D32f* corners,
                                               int *corner_count CV_DEFAULT(NULL));


typedef struct CvPOSITObject CvPOSITObject;

/* Allocates and initializes CvPOSITObject structure before doing cvPOSIT */
OPENCVAPI  CvPOSITObject*  cvCreatePOSITObject( CvPoint3D32f* points, int numPoints );


/* Runs POSIT (POSe from ITeration) algorithm for determining 3d position of
   an object given its model and projection in a weak-perspective case */
OPENCVAPI  void  cvPOSIT(  CvPOSITObject* pObject, CvPoint2D32f* imagePoints,
                           double focalLength, CvTermCriteria criteria,
                           CvMatr32f rotation, CvVect32f translation);

/* Releases CvPOSITObject structure */
OPENCVAPI  void  cvReleasePOSITObject( CvPOSITObject**  ppObject );


/****************************************************************************************\
*                                      ViewMorphing                                      *
\****************************************************************************************/
OPENCVAPI void cvMake2DPoints(CvMat* srcPoint,CvMat* dstPoint);
OPENCVAPI void cvMake3DPoints(CvMat* srcPoint,CvMat* dstPoint);
OPENCVAPI int cvSolveCubic(CvMat* coeffs,CvMat* result);

/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:    cvFindFundamentalMat
//    Purpose: find fundamental matrix for given points using different methods
//    Context:
//    Parameters:
//      points1  - points on first image. Size of matrix 2xN or 3xN
//      points2  - points on second image Size of matrix 2xN or 3xN
//      fundMatr - found fundamental matrixes. Size 3x3. Or 9x3 for 7-point algorithm only.
//                 (7-point algorith can returns 3 fundamental matrixes)
//      method   - method for computing fundamental matrix
//                 CV_FM_7POINT - for 7-point algorithm. Number of points == 7
//                 CV_FM_8POINT - for 8-point algorithm. Number of points >= 8
//                 CV_FM_RANSAC - for RANSAC  algorithm. Number of points >= 8
//                 CV_FM_LMEDS  - for LMedS   algorithm. Number of points >= 8
//      param1 and param2 uses for RANSAC and LMedS method.
//         param1 - threshold distance from point to epipolar line.
//                  If distance less than threshold point is good.
//         param2 - probability. Usually = 0.99
//         status - array, every element of which will be set to 1 if the point was good,
//                  0 else. (for RANSAC and LMedS only)
//                  For other methods all points status set to 1)
//                  (it is optional parameter, can be NULL)
//
//    Returns:
//      number of found fundamental matrixes
//F*/
#define CV_FM_7POINT 1
#define CV_FM_8POINT 2
#define CV_FM_RANSAC 3
#define CV_FM_LMEDS  4 

OPENCVAPI  int cvFindFundamentalMat(    CvMat* points1,
                                CvMat* points2,
                                CvMat* fundMatr,
                                int    method,
                                double param1,
                                double param2,
                                CvMat* status);

/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:    cvComputeCorrespondEpilines
//    Purpose: computes correspondence piline for given point and fundamental matrix
//    Context:
//    Parameters:
//      points  - points on image. Size of matrix 2xN or 3xN
//      pointImageID - image on which points are located. 1 or 2 
//      fundMatr - fundamental matrix
//      corrLines - found correspondence lines for each point. Size of matrix 3xN,
//                  Each line given by a,b,c. (ax+by+c=0)
//
//    Returns:
//     
//F*/
OPENCVAPI  void cvComputeCorrespondEpilines(CvMat* points,
                                            int pointImageID,
                                            CvMat* fundMatr,
                                            CvMat* corrLines);

/* The order of the function corresponds to the order they should appear in
   the view morphing pipeline */ 

/* Finds ending points of scanlines on left and right images of stereo-pair */
OPENCVAPI  void  cvMakeScanlines( const CvMatrix3* matrix,
                                CvSize     imgSize,
                                int*       scanlines_1,
                                int*       scanlines_2,
                                int*       lens_1,
                                int*       lens_2,
                                int*       numlines);

/* Grab pixel values from scanlines and stores them sequentially
   (some sort of perspective image transform) */
OPENCVAPI  void  cvPreWarpImage( int       numLines,
                               IplImage* img,
                               uchar*    dst,
                               int*      dst_nums,
                               int*      scanlines);

/* Approximate each grabbed scanline by a sequence of runs
   (lossy run-length compression) */
OPENCVAPI  void  cvFindRuns( int    numLines,
                           uchar* prewarp_1,
                           uchar* prewarp_2,
                           int*   line_lens_1,
                           int*   line_lens_2,
                           int*   runs_1,
                           int*   runs_2,
                           int*   num_runs_1,
                           int*   num_runs_2);

/* Compares two sets of compressed scanlines */
OPENCVAPI  void  cvDynamicCorrespondMulti( int  lines,
                                         int* first,
                                         int* first_runs,
                                         int* second,
                                         int* second_runs,
                                         int* first_corr,
                                         int* second_corr);

/* Finds scanline ending coordinates for some intermediate "virtual" camera position */
OPENCVAPI  void  cvMakeAlphaScanlines( int*  scanlines_1,
                                     int*  scanlines_2,
                                     int*  scanlines_a,
                                     int*  lens,
                                     int   numlines,
                                     float alpha);

/* Blends data of the left and right image scanlines to get
   pixel values of "virtual" image scanlines */
OPENCVAPI  void  cvMorphEpilinesMulti( int    lines,
                                     uchar* first_pix,
                                     int*   first_num,
                                     uchar* second_pix,
                                     int*   second_num,
                                     uchar* dst_pix,
                                     int*   dst_num,
                                     float  alpha,
                                     int*   first,
                                     int*   first_runs,
                                     int*   second,
                                     int*   second_runs,
                                     int*   first_corr,
                                     int*   second_corr);

/* Does reverse warping of the morphing result to make
   it fill the destination image rectangle */
OPENCVAPI  void  cvPostWarpImage( int       numLines,
                                uchar*    src,
                                int*      src_nums,
                                IplImage* img,
                                int*      scanlines);

/* Deletes Moire (missed pixels that appear due to discretization) */
OPENCVAPI  void  cvDeleteMoire( IplImage*  img);

/****************************************************************************************\
*                                    System functions                                    *
\****************************************************************************************/

/* Loads optimized libraries (with manual and automatical processor type specification) */
OPENCVAPI  int  cvLoadPrimitives( const char* proc_type CV_DEFAULT(NULL) );

/* Exports low-level functions from OpenCV */
OPENCVAPI  int  cvFillInternalFuncsTable(void* table);

/* Retrieves information about OpenCV and loaded optimized primitives */
OPENCVAPI  void  cvGetLibraryInfo( const char** version, int* loaded,
                                   const char** loaded_modules );

/* Get current OpenCV error status */
OPENCVAPI CVStatus cvGetErrStatus( void );

/* Sets error status silently */
OPENCVAPI void cvSetErrStatus( CVStatus status );


/* Retrives current error processing mode */
OPENCVAPI int  cvGetErrMode( void );

/* Sets error processing mode */
OPENCVAPI void cvSetErrMode( int mode );

/* Sets error status and performs some additonal actions (error message box,
   writing message to stderr, terminate application etc.)
   depending on the current error mode */
OPENCVAPI CVStatus cvError( CVStatus code, const char *func,
                         const char *context, const char *file, int line);

/* Retrieves textual description of the error given its code */
OPENCVAPI const char* cvErrorStr( CVStatus status );


/* Assigns a new error-handling function */
OPENCVAPI CVErrorCallBack cvRedirectError(CVErrorCallBack cvErrorFunc);


/*
    Output to:
        cvNulDevReport - nothing
        cvStdErrReport - console(printf)
        cvGuiBoxReport - MessageBox(WIN32)
*/
OPENCVAPI CVStatus cvNulDevReport( CVStatus status, const char *funcName,
                                const char *context, const char *file, int line );

OPENCVAPI CVStatus cvStdErrReport( CVStatus status, const char *funcName,
                                const char *context, const char *file, int line );

OPENCVAPI CVStatus cvGuiBoxReport( CVStatus status, const char *funcName,
                                const char *context, const char *file, int line);

/* Get call stack */
OPENCVAPI void cvGetCallStack(CvStackRecord** stack, int* size);

/* Push the record to the call stack */
OPENCVAPI void cvStartProfile( const char* call, const char* file, int line );

/* Pop the record from the stack */
OPENCVAPI void cvEndProfile( const char* file, int line );

CV_EXTERN_C_FUNCPTR(void (CV_CDECL* CvStartProfileFunc)(const char*,const char*,int));
CV_EXTERN_C_FUNCPTR(void (CV_CDECL* CvEndProfileFunc)(const char*,int));

/* management functions */
OPENCVAPI void cvSetProfile( CvStartProfileFunc startProfile,
                             CvEndProfileFunc endProfile );
 
OPENCVAPI void cvRemoveProfile();                  


CV_EXTERN_C_FUNCPTR(void* (CV_STDCALL *CvAllocFunc)(int, const char*, int));
CV_EXTERN_C_FUNCPTR(int (CV_STDCALL *CvFreeFunc)(void**, const char*, int));

/* Set user-defined memory managment functions (substitutors for malloc and free) that
   will be called by cvAlloc, cvFree and higher-level functions (e.g. cvCreateImage) */
OPENCVAPI void cvSetMemoryManager( CvAllocFunc allocFunc CV_DEFAULT(0),
                                   CvFreeFunc freeFunc CV_DEFAULT(0));


CV_EXTERN_C_FUNCPTR(IplImage* (CV_STDCALL* Cv_iplCreateImageHeader)
                            (int,int,int,char*,char*,int,int,int,int,int,
                            IplROI*,IplImage*,void*,IplTileInfo*));
CV_EXTERN_C_FUNCPTR(void (CV_STDCALL* Cv_iplAllocateImageData)(IplImage*,int,int));

CV_EXTERN_C_FUNCPTR(void (CV_STDCALL* Cv_iplDeallocate)(IplImage*,int));

CV_EXTERN_C_FUNCPTR(IplROI* (CV_STDCALL* Cv_iplCreateROI)(int,int,int,int,int));

CV_EXTERN_C_FUNCPTR(IplImage* (CV_STDCALL* Cv_iplCloneImage)(const IplImage*));


/* Makes OpenCV use IPL functions for IplImage allocation/deallocation */
OPENCVAPI void
cvSetIPLAllocators( Cv_iplCreateImageHeader createHeader,
                    Cv_iplAllocateImageData allocateData,
                    Cv_iplDeallocate deallocate,
                    Cv_iplCreateROI createROI,
                    Cv_iplCloneImage cloneImage );

#define CV_TURN_ON_IPL_COMPATIBILITY()                                  \
    cvSetIPLAllocators( iplCreateImageHeader, iplAllocateImage,         \
                        iplDeallocate, iplCreateROI, iplCloneImage )

/****************************************************************************************\
*                                    Data Persistence                                    *
\****************************************************************************************/

/********************************** High-level functions ********************************/

/* "black box" file storage */
typedef struct CvFileStorage CvFileStorage;

/* storage flags */
#define CV_STORAGE_READ          0
#define CV_STORAGE_WRITE_TEXT    1
#define CV_STORAGE_WRITE_BINARY  2

/* write flags */
#define CV_WRITE_TREE      2 /* flag for storing sequence trees */

/* opens existing or creates new file storage */
OPENCVAPI  CvFileStorage*  cvOpenFileStorage( const char* filename,
                                              CvMemStorage* storage,
                                              int flags );

/* closes file storage and deallocates buffers */
OPENCVAPI  void cvReleaseFileStorage( CvFileStorage** storage );

/* list of attributes */
typedef struct CvAttrList
{
    char** attr; /* NULL-terminated array of (attribute_name,attribute_value) pairs */
    struct CvAttrList* next; /* pointer to next chunk of the attributes list */
}
CvAttrList;

CV_INLINE CvAttrList cvAttrList( char** attr CV_DEFAULT(NULL),
                                 CvAttrList* next CV_DEFAULT(NULL) );
CV_INLINE CvAttrList cvAttrList( char** attr, CvAttrList* next )
{
    CvAttrList list;
    list.attr = attr;
    list.next = next;

    return list;
}

OPENCVAPI const char* cvAttrValue( const CvAttrList* attr, const char* attr_name );

struct CvTypeInfo;

typedef struct CvFileNode
{
    CV_TREE_NODE_FIELDS(CvFileNode)
    const char* tagname;
    const char* name;
    CvAttrList* attr;
    struct CvFileNode* hash_next;
    unsigned hash_val;
    int elem_size;
    struct CvTypeInfo* typeinfo;
    const char* body;
    const void* content;
}
CvFileNode;


/* writes matrix, image, sequence, graph etc. */
OPENCVAPI  void cvWrite( CvFileStorage* storage, const char* name,
                         const void* struct_ptr,
                         CvAttrList attributes CV_DEFAULT(cvAttrList()),
                         int flags CV_DEFAULT(0));

/* writes opening tag of a compound object (used internally by cvWrite) */
OPENCVAPI  void cvStartWriteStruct( CvFileStorage* storage, const char* name,
                                    const char* type_name CV_DEFAULT(NULL),
                                    const void* struct_ptr CV_DEFAULT(NULL),
                                    CvAttrList attributes CV_DEFAULT(cvAttrList()));

/* writes closing tag of a compound object (used internally by cvWrite) */
OPENCVAPI  void cvEndWriteStruct( CvFileStorage* storage );

/* writes a basic type value or a C structure of such values */
OPENCVAPI  void cvWriteElem( CvFileStorage* storage,
                             const char* name,
                             const char* elem_spec,
                             const void* data_ptr );

/* finds the specified noe of file storage */
OPENCVAPI  CvFileNode* cvGetFileNode( CvFileStorage* storage, const char* name );

/* reads matrix, image, sequence, graph etc. */
OPENCVAPI  const void* cvReadFileNode( CvFileStorage* storage, CvFileNode* node,
                                       CvAttrList** list CV_DEFAULT(NULL));

CV_INLINE  const void* cvRead( CvFileStorage* storage, const char* name,
                               CvAttrList** list CV_DEFAULT(NULL) );
CV_INLINE  const void* cvRead( CvFileStorage* storage, const char* name, CvAttrList** list )
{
    return cvReadFileNode( storage, cvGetFileNode( storage, name ), list );
}

/* read a basic type value or a C structure of such values */
OPENCVAPI  int cvReadElem( CvFileStorage* storage, const char* name, void* data_ptr );

/*********************************** Adding own types ***********************************/

CV_EXTERN_C_FUNCPTR(int (CV_CDECL *CvIsInstanceFunc)(const void* struct_ptr));
CV_EXTERN_C_FUNCPTR(void (CV_CDECL *CvReleaseFunc)(void** struct_dblptr));
CV_EXTERN_C_FUNCPTR(void* (CV_CDECL *CvReadFunc)( CvFileStorage* storage,
                                                  CvFileNode* node ));
CV_EXTERN_C_FUNCPTR(void (CV_CDECL *CvWriteFunc)( CvFileStorage* storage,
                                                  const char* name,
                                                  const void* struct_ptr,
                                                  CvAttrList attributes,
                                                  int flags ));
CV_EXTERN_C_FUNCPTR(void* (CV_CDECL *CvCloneFunc)( const void* struct_ptr));

typedef struct CvTypeInfo
{
    int flags;
    int header_size;
    struct CvTypeInfo* prev;
    struct CvTypeInfo* next;
    const char* type_name;
    CvIsInstanceFunc is_instance;
    CvReleaseFunc release;
    CvReadFunc read;
    CvWriteFunc write;
    CvCloneFunc clone;
}
CvTypeInfo;

OPENCVAPI  CvTypeInfo* cvRegisterType( CvTypeInfo* info_data );
OPENCVAPI  void        cvUnregisterType( const char* type_name );
OPENCVAPI  CvTypeInfo* cvFirstType(void);
OPENCVAPI  CvTypeInfo* cvFindType( const char* type_name );
OPENCVAPI  CvTypeInfo* cvTypeOf( const void* struct_ptr );

/* universal functions */
OPENCVAPI void cvRelease( void** struct_ptr );
OPENCVAPI void* cvClone( const void* struct_ptr );

/****************************************************************************************\
*                                 Backward compatibility                                 *
\****************************************************************************************/

#ifdef __cplusplus
}
#endif


#ifndef _CV_NO_BACKWARD_COMPATIBILITY
#include "cvcompat.h"
#endif

#endif /*_CV_H_*/
