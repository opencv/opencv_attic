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

#ifndef _CV_H_
#define _CV_H_

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
#include "cvpixelaccess.h"
#include "cvtypes.h"
#include "cverror.h"

/****************************************************************************************\
*                                    Function definition                                 *
\****************************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:    cvCreateImageHeader
//    Purpose: allocates IplImage structure, initializes and returns it
//    Context:
//    Parameters:
//      size - image size(width and height)
//      depth- image depth
//      channels - number of channels.
//    Returns:
//      created image header
//    Notes:
//      this call is short form of
//         iplCreateImageHeader( channels, 0, depth, channels == 1 ? "GRAY" :
//                               channels == 3 || channels == 4 ? "RGB" : "",
//                               channels == 1 ? "GRAY" : channels == 3 ? "BGR" :
//                               channels == 4 ? "BGRA" : "",
//                               IPL_DATA_ORDER_PIXEL, IPL_ORIGIN_TL, 4,
//                               size.width, size.height,
//                               0,0,0,0);
//F*/
OPENCVAPI  IplImage*  cvCreateImageHeader( CvSize size, int depth, int channels );


/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:    cvInitImageHeader
//    Purpose: initializes image header structure without memory allocation
//    Context:
//    Parameters:
//      image - image header. User allocates it manually(e.g. on the stack)
//      size  - width and height of the image
//      depth - image depth
//      channels - number of channels
//      origin - IPL_ORIGIN_TL or IPL_ORIGIN_BL.
//      align - alignment for raster lines
//      clear - if 1, header is cleared before it is initialized.
//    Returns:
//      initialized header
//F*/
OPENCVAPI IplImage* cvInitImageHeader( IplImage* image, CvSize size, int depth,
                                       int channels, int origin CV_DEFAULT(0),
                                       int align CV_DEFAULT(4), int clear CV_DEFAULT(1));


/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:    cvCreateImage
//    Purpose: creates image header and allocates data
//    Context:
//    Parameters:
//      size - image size(width and height)
//      depth- image depth
//      channels - number of channels.
//    Returns:
//      created image
//    Notes:
//      this call is short form of
//         header = cvCreateImageHeader(size,depth,channels);
//         cvCreateData(header);
//F*/
OPENCVAPI  IplImage*  cvCreateImage( CvSize size, int depth, int channels );


/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:    cvReleaseImageHeader
//    Purpose: releases header
//    Context:
//    Parameters:
//        image - released image header
//    Returns:
//      this call is short form of
//         if( image )
//         {
//              iplDeallocate( *image, IPL_IMAGE_HEADER | IPL_IMAGE_ROI );
//              *image = 0;
//         }
//F*/
OPENCVAPI  void  cvReleaseImageHeader( IplImage** image );


/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:    cvReleaseImage
//    Purpose: releases header and image data
//    Context:
//    Parameters:
//      image - released image
//    Returns:
//      this call is short form of
//         if( image && *image )
//         {
//              iplDeallocate( *image, IPL_IMAGE_ALL );
//              *image = 0;
//         }
//F*/
OPENCVAPI  void  cvReleaseImage( IplImage** image );


/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:    cvCloneImage
//    Purpose: creates a whole copy of the image
//    Context:
//    Parameters:
//      image - source image
//    Returns:
//    Notes:
//F*/
OPENCVAPI IplImage* cvCloneImage( const IplImage* image );


/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:    cvSetImageCOI
//    Purpose: set channel of interest to given value.
//    Context:
//    Parameters:
//      image - image header
//      coi   - channel of interest
//    Returns:
//    Notes:
//      If roi is NULL and coi != 0, roi is allocated.
//F*/
OPENCVAPI  void  cvSetImageCOI( IplImage* image, int coi );


/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:    cvGetImageCOI
//    Purpose: retrieves channel of interest
//    Context:
//    Parameters:
//      image - image header
//    Returns:
//      COI
//F*/
OPENCVAPI  int  cvGetImageCOI( IplImage* image );


/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:    cvSetImageROI
//    Purpose: set image ROI to given rectangle
//    Context:
//    Parameters:
//      image - image header
//      rect  - ROI rectangle
//    Returns:
//    Notes:
//       If roi is NULL and rect is not equal to a whole image, roi is allocated.
//F*/
OPENCVAPI  void  cvSetImageROI( IplImage* image, CvRect rect );


/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:    cvResetImageROI
//    Purpose: deletes image ROI
//    Context:
//    Parameters:
//      image - image header
//    Returns:
//    Notes:
//F*/
OPENCVAPI  void  cvResetImageROI( IplImage* image );


/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:    cvGetImageROI
//    Purpose: return region of interest (ROI) for given image or
//             (0,0,image->width,image->height) if ROI is not set
//    Context:
//    Parameters:
//      image - image header
//    Returns:
//    Notes:
//F*/
OPENCVAPI  CvRect cvGetImageROI( const IplImage* image );


#define CV_AUTOSTEP  0x7fffffff

/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:    cvCreateMatHeader
//    Purpose: allocates CvMat structure, initializes and returns it
//    Context:
//    Parameters:
//      rows - number of matrix rows
//      cols - number of matrix columns
//      type - matrix type
//      step - matrix step (or stride) - an optional parameter.
//    Returns:
//      created matrix header
//F*/
OPENCVAPI  CvMat*  cvCreateMatHeader( int rows, int cols, int type );


/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:    cvInitMatHeader
//    Purpose: initializes matrix header structure without memory allocation
//    Context:
//    Parameters:
//      mat   - matrix header. User allocates it manually(e.g. on the stack)
//      rows  - number of matrix rows
//      cols  - number of matrix columns
//      type  - matrix type
//      step  - matrix step (optional)
//    Returns:
//      initalized matrix header
//F*/
OPENCVAPI CvMat* cvInitMatHeader( CvMat* mat, int rows, int cols,
                                  int type, void* data CV_DEFAULT(0),
                                  int step CV_DEFAULT(CV_AUTOSTEP) );


/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:    cvCreateMat
//    Purpose: creates matrix header and allocates data
//    Context:
//    Parameters:
//      rows  - number of matrix rows
//      cols  - number of matrix columns
//      type  - matrix type
//      step  - matrix step (optional)
//    Returns:
//      created matrix
//F*/
OPENCVAPI  CvMat*  cvCreateMat( int rows, int cols, int type );


/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:    cvReleaseMatHeader
//    Purpose: releases matrix header
//    Context:
//    Parameters:
//        mat - released matrix header
//    Returns:
//      nothing
//F*/
OPENCVAPI  void  cvReleaseMatHeader( CvMat** mat );


/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:    cvReleaseMat
//    Purpose: releases matrix header and underlying data
//    Context:
//    Parameters:
//      matrix - released matrix
//    Returns:
//      nothing
//    Notes:
//F*/
OPENCVAPI  void  cvReleaseMat( CvMat** mat );


/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:    cvCloneMat
//    Purpose: creates a whole copy of the matrix
//    Context:
//    Parameters:
//      mat - the cloned matrix
//    Returns:
//F*/
OPENCVAPI CvMat* cvCloneMat( const CvMat* mat );


/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:    cvGetSubArr
//    Purpose: returns sub-matrix w.o copying data
//    Context:
//    Parameters:
//      arr - the original matrix (or IplImage)
//      submat - pointer to sub-matrix stucture
//      rect - extracted rectange
//    Returns:
//      filled header of submatrix (i.e., &submat)
//F*/
OPENCVAPI CvMat* cvGetSubArr( const CvArr* arr, CvMat* submat, CvRect rect );


/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:    cvGetRow
//    Purpose: The function is analogous to the previous, but returns a single row
//    Context:
//    Parameters:
//      arr - an original matrix (or IplImage)
//      submat - pointer to sub-matrix stucture
//      row - index of the row
//    Returns:
//      filled header of sub-matrix (i.e., &submat)
//F*/
OPENCVAPI CvMat* cvGetRow( const CvArr* arr, CvMat* submat, int row );


/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:    cvGetCol
//    Purpose: The function is analogous to the previous, but returns a single column
//    Context:
//    Parameters:
//      arr - an original matrix (or IplImage)
//      submat - pointer to sub-matrix stucture
//      column - index of the column
//    Returns:
//      filled header of sub-matrix (i.e., &submat)
//F*/
OPENCVAPI CvMat* cvGetCol( const CvArr* arr, CvMat* submat, int column );


/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:    cvGetCol
//    Purpose: The function is analogous to the previous, but returns a single diagonal
//    Context:
//    Parameters:
//      arr - an original matrix (or IplImage)
//      submat - pointer to sub-matrix stucture
//      diag - index of the diagonal ( 0 corresponds to the main diagonal,
//                positive number - to some upper diagonal, negative number - to
//                some lower diagonal ), as shown below:
//             0  1  2  3  4
//            -1  0  1  2  3
//            -2 -1  0  1  2
//            -3 -2 -1  0  1
//            -4 -3 -2 -1  0
//            -5 -4 -3 -2 -1
//
//    Returns:
//      filled header of sub-matrix (i.e., &submat)
//F*/
OPENCVAPI CvMat* cvGetDiag( const CvArr* arr, CvMat* submat,
                            int diag CV_DEFAULT(0));


/* ptr = &arr(idx) */
OPENCVAPI uchar* cvGetPtrAt( const CvArr* arr, int y, int x CV_DEFAULT(0));

/* value = arr(idx) */
OPENCVAPI CvScalar cvGetAt( const CvArr* arr, int y, int x CV_DEFAULT(0)); 

/* arr(idx) = value */
OPENCVAPI void cvSetAt( CvArr* arr, CvScalar value,
                        int y, int x CV_DEFAULT(0)); 


/* Converts CvArr (IplImage or CvMat) to CvMat */
OPENCVAPI CvMat* cvGetMat( const CvArr* src, CvMat* header, int* coi CV_DEFAULT(0));

/* Converts CvArr (IplImage or CvMat) to IplImage */
OPENCVAPI IplImage* cvGetImage( const CvArr* array, IplImage* img );


/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:    cvReshape
//    Purpose: reshapes the matrix
//    Context:
//    Parameters:
//      array  - image or matrix
//      header - the output matrix header (may be the same as input)
//      new_cn - the new channel number. The original array width multiplied by the
//               original number of channels should be divisible
//               by the new number of channels.
//      new_rows - the new number of rows in the matrix. 0 means do not change it if
//               it is not neccessary. Number of rows can be changed only if the matrix
//               continuous.
//    Returns:
//    Notes:
//      All the output parameters are optional
//F*/
OPENCVAPI CvMat* cvReshape( const CvArr* array, CvMat* header,
                            int new_cn, int new_rows CV_DEFAULT(0));

/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:    cvCreateData
//    Purpose: allocates image or matrix data
//    Context:
//    Parameters:
//        array - image or matrix header
//F*/
OPENCVAPI  void  cvCreateData( CvArr* array );


/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:    cvReleaseData
//    Purpose: releases image data
//    Context:
//    Parameters:
//      array - image or matrix header
//F*/
OPENCVAPI  void  cvReleaseData( CvArr* array );


/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:    cvSetData
//    Purpose: sets pointer to data and step parameter to given values
//    Context:
//    Parameters:
//      array - image or matrix header
//      data  - user data
//      step  - full width or data (distance between successive rows)
//    Returns:
//F*/
OPENCVAPI  void  cvSetData( CvArr* array, void* data, int step );

/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:    cvGetRawData
//    Purpose: fills output variables with image parameters
//    Context:
//    Parameters:
//      array - image or matrix
//      data  - pointer to top-left corner of ROI
//      step  - is set to <widthStep> field in case of IplImage or
//              to <step> field in case of CvMat
//      roi_size - width and height of ROI
//    Returns:
//    Notes:
//      All the output parameters are optional
//F*/
OPENCVAPI void cvGetRawData( const CvArr* array, uchar** data,
                             int* step, CvSize* roi_size );

/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:    cvGetSize
//    Purpose: returns size of matrix or image ROI.
//             in case of matrix size.width == number_of_columns,
//                               size.height == number_of_rows.
//    Context:
//    Parameters:
//      arr - image or matrix
//    Returns:
//    Notes:
//F*/
OPENCVAPI  CvSize cvGetSize( const CvArr* arr );

/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:    cvCopy
//    Purpose: copies image ROI or matrix
//    Context:
//    Parameters:
//      src - source array
//      dst - destination array
//      mask - optional mask
//F*/
OPENCVAPI  void  cvCopy( const CvArr* src, CvArr* dst,
                         const CvArr* mask CV_DEFAULT(0) );


/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:    cvSet
//    Purpose: sets image ROI or matrix to given value
//    Context:
//    Parameters:
//      arr - array
//      scalar - value to set to
//      mask - optional mask
//F*/
OPENCVAPI  void  cvSet( CvArr* arr, CvScalar scalar,
                        const CvArr* mask CV_DEFAULT(0) );


/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:    cvLUT
//    Purpose: Performs lookup-table transform: dst(x,y) = lut[src(x,y)]
//    Parameters:
//      srcarr - the source array: 8u or 8s type
//      dstarr - the destination array of arbitrary type,
//      lutarr - the LUT array. The same type as the destination array,
//               contains 256 entries.
//    Note:
//      if the source array has 8s type, the modified formula is used:
//      dst(x,y) = lut[src(x,y) + 128]
//F*/
OPENCVAPI  void cvLUT( const CvArr* srcarr, CvArr* dstarr, const CvArr* lutarr );


/****************************************************************************************\
*                                     Pyramids                                           *
\****************************************************************************************/

/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:    cvPyrUp
//    Purpose: performs factor-2 upsampling of the image with subsequent
//             Gaussian smoothing.
//    Context:
//    Parameters:
//      src - source image
//      dst - destination image(must have twice larger width and height than source image)
//      filter - filter applied. Only IPL_GAUSSIAN_5x5 is allowed.
//    Returns:
//F*/
OPENCVAPI  void  cvPyrUp( const CvArr* src, CvArr* dst,
                          int filter CV_DEFAULT(IPL_GAUSSIAN_5x5) );

/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:    cvPyrDown
//    Purpose: performs factor-2 downsampling of the image
//             with prior Gaussian smoothing
//    Context:
//    Parameters:
//        src - source image
//        dst - destination image(must have twice smaller width and height than
//                                 source image)
//        filter - filter applied. Only IPL_GAUSSIAN_5x5 is allowed.
//    Returns:
//F*/
OPENCVAPI  void  cvPyrDown( const CvArr* src, CvArr* dst,
                            int filter CV_DEFAULT(IPL_GAUSSIAN_5x5) );


/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:    cvPyrSegmentation
//    Purpose: segments image using iterative multi-scale algorithm 
//    Context:
//    Parameters:
//      src - source image
//      dst - destination image
//      storage - pointer to the memory storage
//      comp - pointer to the output sequence of the connected components
//      level - number of level to the pyramid costruction
//      threshold1 - the first segmentation threshold
//      threshold2 - the second segmentation threshold
//    Notes:
//      Source and destination image must be equal types and planes
//F*/
OPENCVAPI void cvPyrSegmentation( IplImage* src,
                               IplImage* dst,
                               CvMemStorage *storage,
                               CvSeq **comp,
                               int level, double threshold1,
                               double threshold2 );


/****************************************************************************************\
*                              Derivative calculation                                    *
\****************************************************************************************/

/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:    cvSobel
//    Purpose: calculates an image derivative d^(xorder+yorder)I/((dx)^xorder)*(dy^yoder))
//             by convolving the image with extended Sobel operators.
//             No scaling is performed.
//
//             |-1 -2 -1|     |-1 0 1|
//             | 0  0  0| and |-2 0 2| are partial cases of it.
//             | 1  2  1|     |-1 0 1|
//
//             First one corresponds to xorder = 0, yorder = 1, apertureSize = 3,
//             And the second corresponds to xorder = 1, yorder = 0, apertureSize = 3.
//    Context:
//    Parameters:
//      src    - source image
//      dst    - destination derivative image
//      xorder - order of x derivative
//      yorder - order of y derivative
//      apertureSize - size of colvolution operator. Must be odd: 3, 5, ... or
//                                        | -3  0  3 |    | -3 -10 -3 |
//                     CV_SCHARR (-1) for | -10 0 10 | or |  0   0  0 | kernels.
//                                        | -3  0  3 |    |  3  10  3 |
//    Returns:
//    Notes:
//      The function uses replicatation border mode.
//      In case of yorder is odd, the image origin is taken into account.
//      (for matrices the top-left origin is assumed)
//F*/
#define CV_SCHARR -1

OPENCVAPI void cvSobel( const CvArr* src, CvArr* dst,
                        int xorder, int yorder,
                        int apertureSize CV_DEFAULT(3));

/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:     cvLaplace
//    Purpose:  Calculates Laplacian of the image: deltaI = d^2(I)/dx^2 + d^2(I)/dy^2.
//              Sobel operator is used for calculating derivatives.
//    Context:
//    Parameters:
//      src - source image
//      dst - destination image
//      apertureSize - size of applied aperture
//    Returns:
//    Notes:
//F*/
OPENCVAPI void cvLaplace( const CvArr* src, CvArr* dst,
                          int apertureSize CV_DEFAULT(3) );

/****************************************************************************************\
*                                    Morphology                                          *
\****************************************************************************************/

/*F///////////////////////////////////////////////////////////////////////////////////////
//
//    Name:    cvCreateStructuringElementEx
//    Purpose:
//      Allocates and fills IplConvKernel structure
//      which can be used as a structuring element in following morphological operations
//    Context:
//    Parameters:
//        cols   - number of columns in the kernel
//        rows   - number of rows in the kernel
//        anchorX - x-coordinate of anchor point(0..cols-1)
//        anchorY - y-coordinate of anchor point(0..rows-1)
//        shape   - shape of the structuring element
//              CV_SHAPE_RECT - rectangular element
//              CV_SHAPE_CROSS - cross-shaped element
//              CV_SHAPE_ELLIPSE - elliptic element
//              CV_SHAPE_CUSTOM - arbitrary element.
//              <values> array determines mask
//        values  - mask array. non-zero pixels determine shape of the element
//    Returns:
//        structuring element
//F*/
typedef enum CvElementShape
{
    CV_SHAPE_RECT    = 0,
    CV_SHAPE_CROSS   = 1,
    CV_SHAPE_ELLIPSE = 2,
    CV_SHAPE_CUSTOM  = 100
}
CvElementShape;

OPENCVAPI  IplConvKernel*  cvCreateStructuringElementEx( int  cols,    int rows,
                                                         int  anchorX, int anchorY,
                                                         CvElementShape shape,
                                                         int* values CV_DEFAULT(0) );


/*F///////////////////////////////////////////////////////////////////////////////////////
//
//    Name:    cvReleaseStructuringElement
//    Purpose:
//      Releases structuring element and clears pointer
//    Context:
//    Parameters:
//        element - double pointer to structuring element
//    Returns:
//F*/
OPENCVAPI  void  cvReleaseStructuringElement( IplConvKernel** element );


/*F///////////////////////////////////////////////////////////////////////////////////////
//
//    Name:    cvErode
//    Purpose:
//      Applies minimum filter to the source image. Structuring element specifies
//      a shape of a pixel neigborhood, over which the minimum is calculated
//    Context:
//    Parameters:
//        src    - source image
//        dst    - destination image, may be the same as source one.
//        element - structuring element.
//                  If the pointer is 0, 3x3 rectangular element is used.
//        iterations - how many times the erosion needs to be applied
//    Returns:
//F*/
OPENCVAPI  void  cvErode( const CvArr* src, CvArr* dst,
                          IplConvKernel* element CV_DEFAULT(0),
                          int iterations CV_DEFAULT(1) );

/*F///////////////////////////////////////////////////////////////////////////////////////
//
//    Name:    cvDilate
//    Purpose:
//      Applies maximum filter to the source image. Structuring element specifies
//      a shape of a pixel neigborhood, over which the maximum is calculated
//    Context:
//    Parameters:
//        src    - source image
//        dst    - destination image, may be the same as source one.
//        element - structuring element.
//                  If the pointer is 0, 3x3 rectangular element is used.
//        iterations - how many times the dilation needs to be applied
//    Returns:
//F*/
OPENCVAPI  void  cvDilate( const CvArr* src, CvArr* dst,
                           IplConvKernel* element CV_DEFAULT(0),
                           int iterations CV_DEFAULT(1) );

/*F///////////////////////////////////////////////////////////////////////////////////////
//
//    Name:    cvMorphologyEx
//    Purpose:
//      Applies one of the extended morphological operations that are based on
//      erosion and dilation.
//    Context:
//    Parameters:
//        src    - source image
//        dst    - destination image, may be the same as source one.
//        temp   - temporary image. The parameter must be non-zero
//                 if operation is CV_MOP_TOPHAT or CV_MOP_BLACKHAT and src == dst, or
//                 if operation is CV_MOP_GRADIENT
//        element - structuring element.
//                  If the pointer is 0, 3x3 rectangular element is used.
//        operation - one of the following:
//               (let's nB = "<element>, applied <iterations> times")
//                CV_MOP_OPEN:   dst = dilate(erode(src,nB),nB);
//                CV_MOP_CLOSE:  dst = erode(dilate(src,nB),nB);
//                CV_MOP_GRADIENT: dst = dilate(src,nB)-erode(src,nB)
//                CV_MOP_TOPHAT:   dst = src - erode(src,nB)
//                CV_MOP_BLACKHAT: dst = dilate(src,nB) - src
//        iterations - how many times the erosion/dilation needs to be applied
//    Returns:
//F*/
typedef enum CvMorphOp
{
    CV_MOP_OPEN = 2,
    CV_MOP_CLOSE = 3,
    CV_MOP_GRADIENT = 4,
    CV_MOP_TOPHAT = 5,
    CV_MOP_BLACKHAT = 6
} CvMorphOp;

OPENCVAPI  void  cvMorphologyEx( const CvArr* src, CvArr* dst,
                                 CvArr* temp, IplConvKernel* element,
                                 CvMorphOp operation, int iterations CV_DEFAULT(1) );

/****************************************************************************************\
*                                  Image Statistics                                      *
\****************************************************************************************/

/****************************************************************************************\
*      Image statistics functions support the next image formats:                        *
*         single-channel: IPL_DEPTH_8U, IPL_DEPTH_8S, IPL_DEPTH_32F                      *
*         three-channel: IPL_DEPTH_8U, IPL_DEPTH_8S, IPL_DEPTH_32F (COI must be != 0)    *
\****************************************************************************************/

/*F///////////////////////////////////////////////////////////////////////////////////////
//
//    Name:    cvSum
//    Purpose:
//      Sums all the elements in the image ROI or in the matrix
//    Context:
//    Parameters:
//        array - image or matrix.
//    Returns:
//        sum of every channel.
//    Note:
//        In case of COI is set the sum over the selected channel is saved
//        if the 0-th element of the returned CvScalar structure.
//F*/
OPENCVAPI  CvScalar  cvSum( const CvArr* array );


/*F///////////////////////////////////////////////////////////////////////////////////////
//
//    Name:    cvCountNonZero
//    Purpose:
//        Counts all the non-zero elements in the image ROI or in the matrix
//    Context:
//    Parameters:
//        array - image or matrix.
//    Returns:
//        count of non-zero pixels
//    Note:
//        For multi-channel images COI must be set.
//F*/
OPENCVAPI  int  cvCountNonZero( const CvArr* array );


/*F///////////////////////////////////////////////////////////////////////////////////////
//
//    Name:    cvAvg
//    Purpose:
//      Calculates average value of the image region or the matrix.
//    Context:
//    Parameters:
//        array - input image or matrix.
//        mask - optional parameter: 8uC1 or 8sC1 array that specifies
//               the processed region of the input array
//    Returns:
//        average value for every channel. In case of COI is set, average value
//        of the selected COI is stored into 0-th element
//        of the returned CvScalar structure
//F*/
OPENCVAPI  CvScalar  cvAvg( const CvArr* array, const CvArr* mask CV_DEFAULT(0) );

/*F///////////////////////////////////////////////////////////////////////////////////////
//
//    Name:    cvAvgSdv
//    Purpose:
//      Calculates mean and standard deviation of pixels in the image region
//    Context:
//    Parameters:
//        img - input image.
//        mean - mean value
//        std_dev - standard deviation
//        mask - mask(byte-depth, single channel)
//    Returns:
//
//F*/
OPENCVAPI  void  cvAvgSdv( const CvArr* array, CvScalar* mean, CvScalar* std_dev,
                           const CvArr* mask CV_DEFAULT(0) );

/*F///////////////////////////////////////////////////////////////////////////////////////
//
//    Name:    cvMinMaxLoc
//    Purpose:
//      Finds minimum and maximum pixel values in the image region
//      and determines their locations.
//    Context:
//    Parameters:
//        img - input image.
//        minVal - minimum value
//        maxVal - maximum value
//        minLoc - location of the minimum
//        maxLoc - location of the maximum
//        mask - mask(byte-depth, single channel)
//    Returns:
//    Note:
//      If there are several global minimums and/or maximums,
//      function returns the most top-left extremums.
//F*/
OPENCVAPI  void  cvMinMaxLoc( const CvArr* array, double* min_val, double* max_val,
                              CvPoint* min_loc CV_DEFAULT(0),
                              CvPoint* max_loc CV_DEFAULT(0),
                              const CvArr* mask CV_DEFAULT(0) );


/****************************************************************************************\
*                                         Moments                                        *
\****************************************************************************************/

/*F///////////////////////////////////////////////////////////////////////////////////////
//
//    Name:    cvMoments
//    Purpose:
//      Calculates moments(up to third order) of the image ROI.
//      It fills moments state and after that, it is possible to
//      return concrete moments using
//        cvGetSpatialMoment, cvGetCentralMoment or
//        cvGetNormalizedCentralMoment
//    Context:
//    Parameters:
//        img - input image
//        moments - output moments state.
//        binary - if non zero, function treats non-zero pixels as 1s.
//    Returns:
//F*/
typedef struct CvMoments
{
    /* spatial moments */
    double  m00, m10, m01, m20, m11, m02, m30, m21, m12, m03;
    /* central moments */
    double  mu20, mu11, mu02, mu30, mu21, mu12, mu03;
    /* m00 != 0 ? 1/sqrt(m00) : 0 */
    double  inv_sqrt_m00;
} CvMoments;

OPENCVAPI void cvMoments( const CvArr* array, CvMoments* moments, int binary CV_DEFAULT( 0 ));


/*F///////////////////////////////////////////////////////////////////////////////////////
//
//    Name:    cvGetSpatialMoment, cvGetCentralMoment, cvGetCentralNormalizedMoment
//    Purpose:
//      Returns different moments(up to third order) from moments state.
//        for raster image, these moments are defined as:
//        mij = spatial_moment(i,j) = sum(y=0,H-1) sum(x=0,W-1) [I(x,y) *(x^i) *(y^j)]
//       (where I(x,y) means pixel value at point(x,y). x^y means x power y).
//
//        muij = central_moment(i,j) = sum(y=0,H-1) sum(x=0,W-1)
//                                     [I(x,y) *(x-mean_x)^i) *((y-mean_y)^j)]
//       (where mean_x = m10/m00, mean_y = m01/m00.
//         it's easy to see that mu00 = m00, mu10 = mu01 = 0)
//
//        nu_ij = central_normalized_moment(i,j) = muij/(m00^((i+j)/2+1))
//    Context:
//    Parameters:
//        moments - moment state( filled by cvMoments or cvContourMoments )
//        x_order - x order of the moment
//        y_order - y order of the moment.
//        The following condition has to be satifsied:
//          0 <= x_order + y_order <= 3
//    Returns:
//        Required moment
//F*/
OPENCVAPI  double  cvGetSpatialMoment( CvMoments* moments, int x_order, int y_order );
OPENCVAPI  double  cvGetCentralMoment( CvMoments* moments, int x_order, int y_order );
OPENCVAPI  double  cvGetNormalizedCentralMoment( CvMoments* moments,
                                              int x_order, int y_order );


/*F///////////////////////////////////////////////////////////////////////////////////////
//
//    Name:    cvGetHuMoments
//    Purpose:
//      Calculates seven Hu invariants from normalized moments
//    Context:
//    Parameters:
//        moments - moments state.
//        hu_moments - Hu moments
//    Returns:
//F*/
typedef struct CvHuMoments
{
    double hu1, hu2, hu3, hu4, hu5, hu6, hu7; /* Hu invariants */
} CvHuMoments;

OPENCVAPI void cvGetHuMoments( CvMoments*  moments, CvHuMoments*  hu_moments );


/*F///////////////////////////////////////////////////////////////////////////////////////
//
//    Name:    cvNorm, cvNormMask
//    Purpose:
//      Calculates different types of norm for single or a pair of images
//    Context:
//    Parameters:
//        imgA - first input image
//        imgB - second input image
//        mask - determine pixels that are considered in norm calculation
//        norm_type - type of the norm.
//                                imgB == 0           imgB != 0
//         ---------------------------------------------------------------------------
//          CV_C:               ||imgA||_inf      ||imgA - imgB||_inf
//          CV_L1:              ||imgA||_L1       ||imgA - imgB||_L1
//          CV_L2:              ||imgA||_L2       ||imgA - imgB||_L2
//         ---------------------------------------------------------------------------
//          CV_RELATIVE_C:       forbidden       ||imgA - imgB||_inf/||imgB||_inf
//          CV_RELATIVE_L1:      forbidden       ||imgA - imgB||_L1/||imgB||_L1
//          CV_RELATIVE_L2:      forbidden       ||imgA - imgB||_L2/||imgB||_L2
//    Returns:
//      required norm
//F*/
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

OPENCVAPI  double  cvNorm( const CvArr* imgA, const CvArr* imgB, int normType,
                           const CvArr* mask CV_DEFAULT(0) );

/****************************************************************************************\
*                                     Drawing                                            *
\****************************************************************************************/

/****************************************************************************************\
*       Drawing functions work with the following formats:                               *
*           single channel: IPL_DEPTH_8U, IPL_DEPTH_8S                                   *
*           three channels: IPL_DEPTH_8U, IPL_DEPTH_8S, coi must be == 0                 *
*       All the functions include parameter color that means rgb value for three-channel *
*       images(and may be constructed with CV_RGB macro) and brightness                 *
*      (least-significant byte of color) for grayscale images.                          *
*       If drawn figure is partially or completely outside the image, it is clipped.     *
\****************************************************************************************/

/*F///////////////////////////////////////////////////////////////////////////////////////
//
//    Name:    cvLine
//    Purpose:
//      Draws line on the image ROI between two points
//    Context:
//    Parameters:
//        img  - image where the line is drawn.
//        pt1  - starting point
//        pt2  - ending point
//        color - line color(or brightness)
//        thickness - line thickness. 1 means simple line.
//                    if line is thick, function draws the line with round endings.
//        connectivity - line connectivity (4 or 8)
//    Returns:
//F*/
OPENCVAPI  void  cvLine( CvArr* array, CvPoint pt1, CvPoint pt2,
                         double color, int thickness CV_DEFAULT(1),
                         int connectivity CV_DEFAULT(8) );


/*F///////////////////////////////////////////////////////////////////////////////////////
//
//    Name:    cvLineAA
//    Purpose:
//      Draws thin antialiazed line on the image ROI between two points
//    Context:
//    Parameters:
//        img  - image where the line is drawn.
//        pt1  - starting point
//        pt2  - ending point
//        scale - number of fractional bits in point coordinates.
//                That is, line can be drawn with sub-pixel accuracy
//        color - line color(or brightness)
//    Returns:
//F*/
OPENCVAPI  void  cvLineAA( CvArr* array, CvPoint pt1, CvPoint pt2,
                           double color, int scale CV_DEFAULT(0));

/*F///////////////////////////////////////////////////////////////////////////////////////
//
//    Name:    cvRectangle
//    Purpose:
//      Draws rectangle on the image ROI
//    Context:
//    Parameters:
//        img  - image where the rectangle is drawn.
//        pt1  - one of the rectangle corners
//        pt2  - opposite corner of the rectangle
//        thickness - thickness of the lines that made up rectangle.
//        color - line color(or brightness)
//    Returns:
//F*/
OPENCVAPI  void  cvRectangle( CvArr* array, CvPoint pt1, CvPoint pt2,
                              double color, int thickness CV_DEFAULT(1));

/*F///////////////////////////////////////////////////////////////////////////////////////
//
//    Name:    cvCircle
//    Purpose:
//      Draws circle on the image ROI
//    Context:
//    Parameters:
//        img  - image.
//        center - circle center
//        radius - circle radius(must be >= 0)
//        color - circle color(or brightness)
//        thickenss - thickness of drawn circle. <0 means filled circle.
//    Returns:
//F*/
OPENCVAPI  void  cvCircle( CvArr* array, CvPoint center, int radius,
                           double color, int thickness CV_DEFAULT(1));


/*F///////////////////////////////////////////////////////////////////////////////////////
//
//    Name:    cvCircleAA
//    Purpose:
//      Draws circle on the image ROI
//    Context:
//    Parameters:
//        img  - image.
//        center - circle center
//        radius - circle radius(must be >= 0)
//        color - circle color(or brightness)
//        scale - number of fractional bits in point coordinates.
//                That is, circle can be drawn with sub-pixel accuracy
//    Returns:
//F*/
OPENCVAPI  void  cvCircleAA( CvArr* array, CvPoint center, int radius,
                             double color, int scale CV_DEFAULT(0) );


/*F///////////////////////////////////////////////////////////////////////////////////////
//
//    Name:    cvEllipse
//    Purpose:
//      Draws elliptic arc
//    Context:
//    Parameters:
//        img  - image.
//        center - ellipse center
//        axes - half axes of the ellipse
//        angle - ellipse angle
//        startAngle - starting angle of elliptic arc
//        endAngle - ending angle of elliptic arc
//        thickness - arc thickness
//        color - ellipse color(or brightness)
//    Returns:
//F*/
OPENCVAPI  void  cvEllipse( CvArr* array, CvPoint center, CvSize axes,
                            double angle, double startAngle, double endAngle,
                            double color, int thickness CV_DEFAULT(1));

/*F///////////////////////////////////////////////////////////////////////////////////////
//
//    Name:    cvEllipseAA
//    Purpose:
//      Draws antialiazed elliptic arc
//    Context:
//    Parameters:
//        img  - image.
//        center - ellipse center
//        axes - half axes of the ellipse
//        angle - ellipse angle
//        startAngle - starting angle of elliptic arc
//        endAngle - ending angle of elliptic arc
//        scale - number of fractioanl bits in center coordinates and axes sizes.
//        color - ellipse color(or brightness)
//    Returns:
//F*/
OPENCVAPI  void  cvEllipseAA( CvArr* array, CvPoint center, CvSize axes,
                              double angle, double startAngle,
                              double endAngle, double color,
                              int scale CV_DEFAULT(0) );

/*F///////////////////////////////////////////////////////////////////////////////////////
//
//    Name:    cvFillConvexPoly
//    Purpose:
//      Fills convex polygon
//    Context:
//    Parameters:
//        img  - image.
//        pts  - array of polygon vertices
//        ntps - number of vertices in the polygon
//        color - polygon color(or brightness)
//    Returns:
//    Notes:
//        fucntion automatically closes the contour -
//        adds edge between first and last vertices.
//        function doesn't check that input polygon is convex.
//F*/
OPENCVAPI  void  cvFillConvexPoly( CvArr* array, CvPoint* pts, int npts, double color );


/*F///////////////////////////////////////////////////////////////////////////////////////
//
//    Name:    cvFillPoly
//    Purpose:
//      Fills arbitrary regions, bounded by several polygonal contours.
//    Context:
//    Parameters:
//        img  - image.
//        contours - number of contours
//        pts  - array of pointers to polygonal contours
//        ntps - array of vertices counters for the contours
//        color - polygons color(or brightness)
//    Returns:
//    Notes:
//        function automatically closes each polygonal contour.
//        If some contours are overlapped, they are added modulo 2.
//        That is, pixel is filled, if it belongs to odd number of polygonal contours.
//F*/
OPENCVAPI  void  cvFillPoly( CvArr* array, CvPoint** pts,
                             int* npts, int contours, double color );

/*F///////////////////////////////////////////////////////////////////////////////////////
//
//    Name:    cvPolyLine
//    Purpose:
//      Draws polygons outline with simple or thick lines.
//    Context:
//    Parameters:
//        img  - image.
//        contours - number of contours
//        pts  - array of pointers to polygonal contours
//        ntps - array of vertices counters for the contours
//        closed - if non-zero, function closes each contour.
//        thickness - line thickness
//        color - polygons color(or brightness)
//    Returns:
//F*/
OPENCVAPI  void  cvPolyLine( CvArr* array, CvPoint** pts, int* npts, int contours,
                             int closed, double color,
                             int thickness CV_DEFAULT(1),
                             int connectivity CV_DEFAULT(8));

/*F///////////////////////////////////////////////////////////////////////////////////////
//
//    Name:    cvPolyLineAA
//    Purpose:
//      Draws polygons outline with antialiazes lines.
//    Context:
//    Parameters:
//        img  - image.
//        contours - number of contours
//        pts  - array of pointers to polygonal contours
//        ntps - array of vertices counters for the contours
//        closed - if non-zero, function closes each contour.
//        scale - number of fractioanl bits in vertex coordinates
//        color - polygons color(or brightness)
//    Returns:
//F*/
OPENCVAPI  void  cvPolyLineAA( CvArr* array, CvPoint** pts, int* npts, int contours,
                               int closed, double color, int scale CV_DEFAULT(0) );


/*F///////////////////////////////////////////////////////////////////////////////////////
//
//    Name:    cvInitFont
//    Purpose:
//      Initializes font to use it in following text rendering operations
//    Context:
//    Parameters:
//        font - pointer to initialized font structure.
//        font_face - font family. There is a single font,
//                    supported now - CV_FONT_VECTOR0.
//        hscale - multiplier for horizontal letter sizes.
//                 If 1 then the original size is used,
//                 if 2 - twice wider, if 0.5 - twice thinner etc.
//        vscale - multiplier for vertical letter sizes.
//                 If 1 then the original size is used,
//                 if 2 - twice longer, if 0.5 - twice shorter etc.
//        italic_scale - tangent of letter slope, 0 means no slope,
//                       1 - 45 degree slope
//        thickness - letter thickness
//    Returns:
//F*/
typedef enum CvFontFace
{
    CV_FONT_VECTOR0 = 0
} CvFontFace;

typedef struct CvFont
{
    const int*  data; /* font data and metrics */
    CvSize      size; /* horizontal and vertical scale factors,
                         (8:8) fix-point numbers */
    int         italic_scale; /* slope coefficient: 0 - normal, >0 - italic */
    int         thickness; /* letters thickness */
    int         dx; /* horizontal interval between letters */
} CvFont;

OPENCVAPI  void  cvInitFont( CvFont* font, CvFontFace font_face,
                             double hscale, double vscale,
                             double italic_scale CV_DEFAULT(0),
                             int thickness CV_DEFAULT(1) );

/*F///////////////////////////////////////////////////////////////////////////////////////
//
//    Name:    cvPutText
//    Purpose:
//      Draws text string on the image with given font
//    Context:
//    Parameters:
//        img  - image.
//        text - text string
//        org  - left-bottom corner of output text string
//        font - text font
//        color - polygons color(or brightness)
//    Returns:
//F*/
OPENCVAPI  void  cvPutText( CvArr* array, const char* text, CvPoint org,
                            CvFont* font, double color );


/*F///////////////////////////////////////////////////////////////////////////////////////
//
//    Name:    cvGetTextSize
//    Purpose:
//      Calculates bounding rectangle for given text string and font.
//    Context:
//    Parameters:
//        text - text string
//        font - font to draw the string with
//        text_size - output parameter. width and height of bounding box
//                   (not including part of the text below base line)
//        ymin - output parameter. negative value or zero - minus height of
//               text part below base line
//    Returns:
//F*/
OPENCVAPI  void  cvGetTextSize( const char* text_string, CvFont* font,
                                CvSize* text_size, int* ymin );


/****************************************************************************************\
*                                 Color Transforms                                       *
\****************************************************************************************/

#define  CV_BGR2BGRA    0
#define  CV_RGB2RGBA    CV_BGR2BGRA

#define  CV_BGRA2BGR    1
#define  CV_RGBA2RGB    CV_BGRA2BGR

#define  CV_BGR2RGBA    2
#define  CV_RGB2BGRA    CV_BGR2RGBA

#define  CV_RGBA2BGR    3
#define  CV_BGRA2RGB    CV_RGBA2BGR

#define  CV_BGR2GRAY    4
#define  CV_RGB2GRAY    5

#define  CV_GRAY2BGR    6
#define  CV_GRAY2RGB    CV_GRAY2BGR

#define  CV_BGR2BGR565  7
#define  CV_RGB2BGR565  8
#define  CV_BGR5652BGR  9
#define  CV_BGR5652RGB  10

#define  CV_BGR2RGB     11
#define  CV_RGB2BGR     CV_BGR2RGB

#define  CV_BGR2XYZ     12
#define  CV_RGB2XYZ     13
#define  CV_XYZ2BGR     14
#define  CV_XYZ2RGB     15

#define  CV_BGR2YCrCb   16
#define  CV_RGB2YCrCb   17
#define  CV_YCrCb2BGR   18
#define  CV_YCrCb2RGB   19

#define  CV_BGR2HSV     20
#define  CV_RGB2HSV     21

#define  CV_BGR2Lab     22
#define  CV_RGB2Lab     23

#define  CV_GRAY2BGR565 24
#define  CV_GRAY2BGRA   25
#define  CV_GRAY2RGBA   CV_GRAY2BGRA

#define  CV_BGR5652GRAY 26
#define  CV_BGRA2GRAY   27
#define  CV_RGBA2GRAY   28

#define  CV_BGRA2BGR565 29
#define  CV_RGBA2BGR565 30

#define  CV_COLORCVT_MAX  32

OPENCVAPI  void  cvCvtColor( const CvArr* src, CvArr* dst, int colorCvtCode );


/****************************************************************************************\
*                                 Geometrical Transforms                                 *
\****************************************************************************************/

#define  CV_INTER_NN        0
#define  CV_INTER_LINEAR    1
/*#define  CV_INTER_CUBIC     2*/

OPENCVAPI  void  cvResize( const CvArr* src, CvArr* dst,
                           int method CV_DEFAULT( CV_INTER_LINEAR ));

/****************************************************************************************\
*                                       Utilities                                        *
\****************************************************************************************/

/*F///////////////////////////////////////////////////////////////////////////////////////
//
//    Name:    cvInitLineIterator
//    Purpose:
//      Initializes iterator that gets all the pixels, lying on the raster line between
//      two given points
//    Context:
//    Parameters:
//        img  - image.
//        pt1  - starting point
//        pt2  - ending point. Both points must be inside the image
//        lineIterator - pointer to initialized iterator state
//    Returns:
//        number of pixels between pt1 and pt2.
//        It is equal to max( abs(pt1.x - pt2.x), abs(pt1.y - pt2.y))
//F*/
typedef struct CvLineIterator
{
    uchar* ptr;
    int  err;
    int  plus_delta;
    int  minus_delta;
    int  plus_step;
    int  minus_step;
} CvLineIterator;

OPENCVAPI  int  cvInitLineIterator( const CvArr* array, CvPoint pt1, CvPoint pt2,
                                    CvLineIterator* lineIterator,
                                    int connectivity CV_DEFAULT(8));

/* Move to the next line point */
#define CV_NEXT_LINE_POINT( iterator )                                          \
{                                                                               \
    int mask =  (iterator).err < 0 ? -1 : 0;                                    \
    (iterator).err += (iterator).minus_delta + ((iterator).plus_delta & mask);  \
    (iterator).ptr += (iterator).minus_step + ((iterator).plus_step & mask);    \
}

/*F///////////////////////////////////////////////////////////////////////////////////////
//
//    Name:    cvSampleLine
//    Purpose:
//      Fetch all the pixel, lying on the raster line between two given points and
//      writes them to the buffer
//    Context:
//    Parameters:
//        img  - image.
//        pt1  - starting point
//        pt2  - ending point. Both points must be inside the image
//        buffer - pointer to destination buffer.
//    Returns:
//        number of pixels stored.
//        It is equal to max( abs(pt1.x - pt2.x), abs(pt1.y - pt2.y))
//F*/
OPENCVAPI  int  cvSampleLine( const CvArr* array, CvPoint pt1, CvPoint pt2, void* buffer,
                              int connectivity CV_DEFAULT(8));


/*F///////////////////////////////////////////////////////////////////////////////////////
//
//    Name:    cvGetRectSubPix
//    Purpose:
//      Retrieves rectangle from the image with sub-pixel accuracy
//    Context:
//    Parameters:
//        src  - source image.
//        dst  - destination image.
//        center - center point of the extracted rectangle.
//                 Size of extracted rectangle is equal to
//                 desination image ROI size.
//    Returns:
//F*/
OPENCVAPI  void  cvGetRectSubPix( const CvArr* src, CvArr* dst, CvPoint2D32f center );


/*F///////////////////////////////////////////////////////////////////////////////////////
//
//    Name:    cvGetQuandrangleSubPix
//    Purpose:
//      Retrieves rectangle from the image with sub-pixel accuracy
//    Context:
//    Parameters:
//        src  - source image.
//        dst  - destination image.
//        matrix - transformation matrix (2 rows x 3 columns).
//                 ( a11  a12 | b1 )      dst([x,y]') = src(A[x y]' + b)
//                 ( a21  a22 | b2 )      (bilinear interpolation is used)
//        fillOutliers - fill outlier pixels with some constant value or take them from
//                       the nearest boundary
//        fillValue - constant value to fill with
//    Returns:
//F*/
OPENCVAPI  void  cvGetQuadrangleSubPix( const CvArr* src, CvArr* dstarr,
                                        const CvArr* matrixarr,
                                        int fillOutliers CV_DEFAULT(0),
                                        CvScalar fillvalue CV_DEFAULT(cvScalarAll(0)));


/*F///////////////////////////////////////////////////////////////////////////////////////
//
//    Name:    cvCvtPixToPlane
//    Purpose:
//      Splits source image into several separate planes
//    Context:
//    Parameters:
//        src  - source image. Must have 3 or 4 channels.
//        dst0, dst1, dst2, dst3  - destination images. Must have single channel.
//               if src has 3 channels, dst3 must be NULL.
//               if one of the destination images is not NULL,
//               the corresponding channel is extracted from source image.
//               Else, all 3 or 4 images must be non NULL and all the source image
//               channels are written to destination images.
//    Returns:
//F*/
OPENCVAPI  void  cvCvtPixToPlane( const void *src, void *dst0, void *dst1,
                                  void *dst2, void *dst3 );


/*F///////////////////////////////////////////////////////////////////////////////////////
//
//    Name:    cvCvtPlaneToPix
//    Purpose:
//      Composes destination image from separate single-channel images
//    Context:
//    Parameters:
//        src0, src1, src2, src3  - source images. Must have single channel.
//              if destination image has 3 channels, src3 must be NULL, else must be
//              non NULL. Other images must always be non NULL.
//        dst - destination image. Must have 3 or 4 channels.
//    Returns:
//F*/
OPENCVAPI  void  cvCvtPlaneToPix( const void *src0, const void *src1,
                                  const void *src2, const void *src3,
                                  void *dst );


/*F///////////////////////////////////////////////////////////////////////////////////////
//
//    Name:    cvConvertScale
//    Purpose:
//      Converts image from one depth to another with linear transform
//    Context:
//    Parameters:
//        src - source image.
//        dst - destination image.
//        scale - multiplier
//        shift - delta. That is, dst(x,y) = src(x,y)*scale + shift.
//    Returns:
//    Notes:
//        only float->uchar and uchar->float are supported by now.
//F*/
OPENCVAPI  void  cvConvertScale( const CvArr *src, CvArr *dst,
                                 double scale CV_DEFAULT(1),
                                 double shift CV_DEFAULT(0) );
#define cvCvtScale cvConvertScale
#define cvScale  cvConvertScale
#define cvConvert( src, dst )  cvConvertScale( (src), (dst), 1, 0 )


/*F///////////////////////////////////////////////////////////////////////////////////////
//
//    Name:    cvConvertScaleAbs
//    Purpose:
//      Converts image from one depth to another
//    Context:
//    Parameters:
//        src - source image.
//        dst - destination image.
//        scale - multiplier
//        shift - delta. That is, dst(x,y) = abs(src(x,y)*scale + shift).
//    Returns:
//    Notes:
//F*/
OPENCVAPI  void  cvConvertScaleAbs( const void *src, void *dst,
                                    double scale CV_DEFAULT(1),
                                    double shift CV_DEFAULT(0) );
#define cvCvtScaleAbs  cvConvertScaleAbs

/*F///////////////////////////////////////////////////////////////////////////////////////
//
//    Name:    cvAbsDiff
//    Purpose:
//      Finds per-pixel absolute difference between two images
//    Context:
//    Parameters:
//        srcA - first source image.
//        srcB - second source image
//        dst  - destination image, May be equal to srcA or srcB
//    Returns:
//F*/
OPENCVAPI  void  cvAbsDiff( const CvArr* srcA, const CvArr* srcB, CvArr* dst );


/*F///////////////////////////////////////////////////////////////////////////////////////
//
//    Name:    cvAbsDiffS
//    Purpose:
//      Finds per-pixel absolute difference between image and scalar value
//    Context:
//    Parameters:
//        src - source image.
//        dst - destination image, May be equal to srcA or srcB
//        value - scalar value to compare with
//    Returns:
//F*/
OPENCVAPI  void  cvAbsDiffS( const CvArr* src, CvArr* dst, CvScalar value );


/*F///////////////////////////////////////////////////////////////////////////////////////
//
//    Name: cvMatchTemplate
//    Purpose:
//      measures similarity between template and overlapped windows in the source image
//      and fills the resultant image with the measurements.
//    Context:
//    Parameters:
//      img     - source image
//      templ   - template to find
//      result  - resultant image. its ROI must have size:
//                     (img_width - templ_width + 1, img_height - templ_height + 1)
//      method  - comparison method:
//---------------------------------------------------------------------------------------
//             CV_TM_SQDIFF:  res0(i,j)=sum(y=0,TH-1) sum(x=0,TW-1)[I(i+x,j+y)-T(x,y)]^2
//                    (where  TW - template width, TH - template height
//                          res0(i,j) - pixel value of result at location(i,j)
//                                     (zero-th method)
//                          Iij(x,y) - pixel value of source image at location(i+x,j+y)
//                                     Iij alone means window of source image
//                                     with top-left corner(i,j) and template size.
//                          T(x,y) - pixel value of template at location(x,y)
//                                   T alone means template.
//---------------------------------------------------------------------------------------
//             CV_TM_SQDIFF_NORMED:  res1(i,j) = res0(i,j)/
//                                             (l2_norm(Iij)*l2_norm(templ);
//                      where  l2_norm(A) = sqrt(
//                                     sum(y=0,A_height-1) sum(x=0,A_width-1) A(x,y)^2);
//---------------------------------------------------------------------------------------
//             CV_TM_CCORR:  res2(i,j)=sum(y=0,TH-1) sum(x=0,TW-1)[Iij(x,y)*T(x,y)]
//---------------------------------------------------------------------------------------
//             CV_TM_CCORR_NORMED:  res3(i,j) = res2(i,j)/[l2_norm(Iij)*l2_norm(templ)];
//---------------------------------------------------------------------------------------
//             CV_TM_CCOEFF:  res4(i,j)=sum(y=0,TH-1) sum(x=0,TW-1) [I'ij(x,y)*T'(x,y)]
//                   where A'(x,y) = A(x,y)-1/(A_width*A_height)*
//                                   sum(l=0,A_height-1) sum(k=0,A_width-1)A(k,l)
//---------------------------------------------------------------------------------------
//             CV_TM_CCOEFF_NORMED:
//                   res5(i,j)=res4(i,j)/[l2_norm(I'ij)*l2_norm(T')]
//---------------------------------------------------------------------------------------
//    Returns:
//F*/
/* method for comparing two images */
typedef enum CvTemplMatchMethod
{
    CV_TM_SQDIFF        = 0,
    CV_TM_SQDIFF_NORMED = 1,
    CV_TM_CCORR         = 2,
    CV_TM_CCORR_NORMED  = 3,
    CV_TM_CCOEFF        = 4,
    CV_TM_CCOEFF_NORMED = 5
}
CvTemplMatchMethod;

OPENCVAPI  void  cvMatchTemplate( const CvArr* array, const CvArr* templ,
                                  CvArr* result, CvTemplMatchMethod method );

/*F///////////////////////////////////////////////////////////////////////////////////////
//
//    Name:    cvbFastArctan
//    Purpose:
//      Calculates arctangent for arrays of abscissas and ordinates
//    Context:
//    Parameters:
//        y - array of abscissas
//        x - array of ordinates
//        angle - array of results: array[i] = arctan(y[i]/x[i])
//        len - number of elements in arrays
//    Returns:
//    Notes:
//      The function takes into account signs of both argument, so it is similar
//      to atan2, but it returns angle in degrees(from 0 to 359.999 degrees)
//      Maximal error is ~0.1 degreee.
//F*/
OPENCVAPI  void  cvbFastArctan( const float* y, const float* x, float* angle, int len );


/*F///////////////////////////////////////////////////////////////////////////////////////
//
//    Name:    cvbCartToPolar
//    Purpose:
//      Converts input arrays of abscissas and ordinates to polar form
//    Context:
//    Parameters:
//        y - array of abscissas
//        x - array of ordinates
//        magnitude - array of magnitudes: mag[i] = sqrt(y[i]*y[i] + x[i]*x[i])
//        angle - array of angles: array[i] = arctan(y[i]/x[i])
//        len - number of elements in arrays
//    Returns:
//    Notes:
//      The function calculates angle(similar to cvbFastArctan) and magnitude for
//      every 2D vector(x[i],y[i]). Both output arguments are optional. If some
//      output parameter is absent, corresponding part is not calculated
//F*/
OPENCVAPI  void  cvbCartToPolar( const float* y, const float* x,
                                float* magnitude, float* angle, int len );


/*F///////////////////////////////////////////////////////////////////////////////////////
//
//    Name:    cvbSqrt
//    Purpose:
//      Calculates square root for array of floats
//    Context:
//    Parameters:
//        x - array of arguments
//        sqrt_x - array of results
//        len - number of elements in arrays
//    Returns:
//    Notes:
//      Elements of input array must be non-negative, else the result is not defined.
//      Maximal relative error is ~3e-7
//F*/
OPENCVAPI  void  cvbSqrt( const float* x, float* sqrt_x, int len );


/*F///////////////////////////////////////////////////////////////////////////////////////
//
//    Name:    cvCheckArr
//    Purpose:
//      Checks array for bad elements (NaNs, Infinities or just too big
//                                     positive or negative values)
//    Context:
//    Parameters:
//      arr - input array
//      flags - operation flags, that may be zero or combination of the following values:
//               CV_CHECK_RANGE - the function checks that the array elements are
//                                within [minVal,maxVal) range. By default, only NaNs
//                                and Infinities are checked.
//               CV_CHECK_QUIET - do not raise error if some elements is out of
//                                range. It is not a default mode.
//    Returns:
//      1 if array is ok, 0 otherwise. If CV_CHECK_QUIET is not set, function
//      raises the CV_StsOutOfRange error in the latter case.
//F*/
#define  CV_CHECK_RANGE    1
#define  CV_CHECK_QUIET    2
OPENCVAPI  int  cvCheckArr( const CvArr* arr, int flags CV_DEFAULT(0),
                            double minVal CV_DEFAULT(0), double maxVal CV_DEFAULT(0));
#define cvCheckArray cvCheckArr

/*F///////////////////////////////////////////////////////////////////////////////////////
//
//    Name:    cvbInvSqrt
//    Purpose:
//      Calculates inverse square root for array of floats
//    Context:
//    Parameters:
//        x - array of arguments
//        sqrt_x - array of results
//        len - number of elements in arrays
//    Returns:
//    Notes:
//      Elements of input array must be positive, else the result is not defined.
//      Maximal relative error is ~2e-7
//F*/
OPENCVAPI  void  cvbInvSqrt( const float* x, float* inv_sqrt_x, int len );


/*F///////////////////////////////////////////////////////////////////////////////////////
//
//    Name:    cvbReciprocal
//    Purpose:
//      Calculates inverse value(1/x) for array of floats
//    Context:
//    Parameters:
//        x - array of arguments
//        inv_x - array of results
//        len - number of elements in arrays
//    Returns:
//    Notes:
//      For zero elements result is 0.
//      Maximal relative error is <2e-7
//F*/
OPENCVAPI  void  cvbReciprocal( const float* x, float* inv_x, int len );


/*F///////////////////////////////////////////////////////////////////////////////////////
//
//    Name:    cvbFastExp
//    Purpose:
//      Calculates fast exponent approximation for array of floats
//    Context:
//    Parameters:
//        x - array of arguments
//        exp_x - array of results
//        len - number of elements in arrays
//    Returns:
//    Notes:
//      Overflow is not handled yet. Underflow is handled.
//      Maximal relative error is ~7e-6
//F*/
OPENCVAPI  void  cvbFastExp( const float* x, double* exp_x, int len );


/*F///////////////////////////////////////////////////////////////////////////////////////
//
//    Name:    cvbFastLog
//    Purpose:
//      Calculates fast logarithm approximation for array of doubles
//    Context:
//    Parameters:
//        x - array of arguments
//        log_x - array of logarithms of absolute values of arguments
//        len - number of elements in arrays
//    Returns:
//    Notes:
//      Negative values are negated before logarithm is taken.
//      Logarithm of 0 gives large negative number(~700)
//      Maximal relative error is ~3e-7
//F*/
OPENCVAPI  void  cvbFastLog( const double* x, float* log_x, int len );


/* RNG state */
typedef struct CvRandState
{
    uint64    state;    /* RNG state (the current seed and carry)*/
    CvScalar  param[2]; /* parameters of RNG */
}
CvRandState;

/*F///////////////////////////////////////////////////////////////////////////////////////
//
//    Name:    cvRandInit
//    Purpose:
//      Initializes random number generator(RNG)
//    Context:
//    Parameters:
//      state - pointer to initialized RNG state
//      lower - lower bound of random values
//      upper - upper bound of random values.
//              Generated random numbers belong to range [lower,upper)
//      seed  - initializing 32-bit integer for RNG
//    Returns:
//F*/
OPENCVAPI  void  cvRandInit( CvRandState* state, double lower, double upper, int seed );

/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:    cvRandSetRange
//    Purpose: sets range of generated random numbers without reinitializing RNG
//    Context:
//    Parameters:
//      state - pointer to state structure
//      lower - lower bound
//      upper - upper bound
//      dim  - optional parameter.
//             Index of the dimension to set the range for (0th, 1st etc.)
//             -1 means to set the same range for all dimensions.
//    Returns:
//      CV_OK or error code if:
//         state pointer is zero or
//         lower bound greater than upper bound.
//    Notes:
//F*/
OPENCVAPI  void  cvRandSetRange( CvRandState* state, double lower, double upper,
                                 int index CV_DEFAULT(-1));


/*F///////////////////////////////////////////////////////////////////////////////////////
//
//    Name:    cvbRand
//    Purpose:
//      Fills array of floats with random numbers and updates RNG state
//    Context:
//    Parameters:
//      state - RNG state
//      dst   - destination floating-point array
//      len   - number of elements in the array.
//    Returns:
//F*/
OPENCVAPI  void  cvbRand( CvRandState* state, float* dst, int len );


/*F///////////////////////////////////////////////////////////////////////////////////////
//
//    Name:    cvRand
//    Purpose:
//      Fills an array with random numbers and updates RNG state
//    Context:
//    Parameters:
//      state - RNG state
//      arr   - the destination array
//    Returns:
//F*/
OPENCVAPI  void  cvRand( CvRandState* state, CvArr* arr );


/*F///////////////////////////////////////////////////////////////////////////////////////
//
//    Name:    cvRandNext
//    Purpose:
//      Updates RNG state and returns 32-bit random number
//    Context:
//    Parameters:
//      state - RNG state
//    Returns:
//      random number
//F*/
OPENCVAPI  unsigned  cvRandNext( CvRandState* state );


/****************************************************************************************\
*                               Motion templates                                         *
\****************************************************************************************/

/****************************************************************************************\
*        All the motion template functions work only with single channel images.         *
*        Silhouette image must have depth IPL_DEPTH_8U or IPL_DEPTH_8S                   *
*        Motion history image must have depth IPL_DEPTH_32F,                             *
*        Gradient mask - IPL_DEPTH_8U or IPL_DEPTH_8S,                                   *
*        Motion orientation image - IPL_DEPTH_32F                                        *
*        Segmentation mask - IPL_DEPTH_32F                                               *
*        All the angles are in degrees, all the times are in milliseconds                *
\****************************************************************************************/

/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:    cvUpdateMotionHistory
//    Purpose: updates motion history image.
//    Context:
//    Parameters:
//        silhouette  - silhouette image
//        mhi         - motion history image
//        timestamp   - current system time
//        mhiDuration - maximal duration of motion track before it will be removed
//    Returns:
//    Notes:
//      Motion history image is changed by the following algorithm:
//         for every point(x,y) in the mhi do
//             if( silhouette(x,y) != 0 )
//             {
//                 mhi(x,y) = timestamp;
//             }
//             else if( mhi(x,y) < timestamp - mhi_duration )
//             {
//                 mhi(x,y) = 0;
//             }
//             // else mhi(x,y) remains unchanged
//F*/
OPENCVAPI  void    cvUpdateMotionHistory( const CvArr* silhouette, CvArr* mhi,
                                          double timestamp, double mhiDuration );

/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:    cvCalcMotionGradient
//    Purpose: calculates motion gradient and mask where it is valid
//    Context:
//    Parameters:
//       mhi         - motion history image
//       mask        -(output image) indicates where <orientation> data is valid
//       orientation -(output image) contains gradient orientation in degrees
//       aperture_size - size of the filters for x & y derivatives
//
//       maxTDelta   - gradient bounds.
//       minTDelta   _/
//    Returns:
//    Notes:
//      Function handles both top-left and bottom-left origins of orientation image
//F*/
OPENCVAPI  void    cvCalcMotionGradient( const CvArr* mhi, CvArr* mask, CvArr* orientation,
                                         double maxTDelta, double minTDelta,
                                         int aperture_size CV_DEFAULT(3));

/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:    cvCalcGlobalOrientation
//    Purpose: calculates general motion direction in the selected region.
//    Context:
//    Parameters:
//         orient       - orientation image
//         mask         - region mask
//         mhi          - motion history image
//         timestamp    - the last timestamp when mhi was updated
//         mhi_duration - maximal motion track duration.
//    Returns:
//      direction of selected region in degrees
//F*/
OPENCVAPI  double  cvCalcGlobalOrientation( const CvArr* orientation, const CvArr* mask,
                                            const CvArr* mhi, double curr_mhi_timestamp,
                                            double mhi_duration );

/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:    cvSegmentMotion
//    Purpose: splits motion history image into several regions that
//             move in different directions.
//    Context:
//    Parameters:
//        mhi        - motion history image
//        seg_mask   - segmentation mask image. It is marked with different values
//                    (1,2,3...) for every motion component
//        storage    - where to store motion components
//        timestamp  - the last timestamp when mhi was updated
//        seg_thresh - threshold, which is used to split motion components(regions)
//                     the bigger threshold, the coarse segmentation is.
//    Returns:
//      sequence of connected components
//    Notes:
//F*/
OPENCVAPI  CvSeq*  cvSegmentMotion( CvArr* mhi, CvArr* seg_mask,
                                    CvMemStorage* storage,
                                    double timestamp, double seg_thresh );

/****************************************************************************************\
*                               Background Differencing                                  *
\****************************************************************************************/

OPENCVAPI  void  cvAcc( const CvArr* image, CvArr* sum,
                        const CvArr* mask CV_DEFAULT(0) );

OPENCVAPI  void  cvSquareAcc( const CvArr* image, CvArr* sqSum,
                              const CvArr* mask CV_DEFAULT(0) );

OPENCVAPI  void  cvMultiplyAcc( const CvArr* imgA, const CvArr* imgB, CvArr* acc,
                                const CvArr* mask CV_DEFAULT(0) );

OPENCVAPI  void  cvRunningAvg( const CvArr* imgY, CvArr* imgU, double alpha,
                               const CvArr* mask CV_DEFAULT(0) );

/****************************************************************************************\
*                              Dynamic data structures                                   *
\****************************************************************************************/

/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:    cvCreateMemStorage
//    Purpose: creates memory storage
//    Context:
//    Parameters:
//         block_size - size of memory storage blocks.
//                      If 0, default size( Currently 64K) is set
//    Returns:
//      memory storage
//F*/
OPENCVAPI  CvMemStorage*  cvCreateMemStorage( int block_size CV_DEFAULT(0));


/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:    cvCreateChildMemStorage
//    Purpose: creates child memory storage
//            (storage that borrows memory blocks from parent)
//    Context:
//    Parameters:
//         parent - parent memory storage
//    Returns:
//      memory storage
//F*/
OPENCVAPI  CvMemStorage*  cvCreateChildMemStorage( CvMemStorage* parent );


/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:    cvReleaseMemStorage
//    Purpose: releases memory storage.
//    Context:
//    Parameters:
//         storage - double pointer to memory storage
//    Returns:
//    Notes:
//      if memory storage is simple, all its blocks are released,
//      else(memory storage is child) all its blocks are returned to parent
//F*/
OPENCVAPI  void  cvReleaseMemStorage( CvMemStorage** storage );


/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:    cvClearMemStorage
//    Purpose: clears memory storage.
//    Context:
//    Parameters:
//         storage - memory storage
//    Returns:
//    Notes:
//      if memory storage is is child, all its blocks are returned to parent,
//      else the top of the storage is reset
//F*/
OPENCVAPI  void  cvClearMemStorage( CvMemStorage* storage );

/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:    cvSaveMemStoragePos
//    Purpose: saves current top of the storage.
//    Context:
//    Parameters:
//         storage - memory storage
//         pos - position structure
//    Returns:
//    Notes:
//F*/
OPENCVAPI  void  cvSaveMemStoragePos( CvMemStorage* storage, CvMemStoragePos* pos );

/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:    cvRestoreMemStoragePos
//    Purpose: restores top of the storage.
//    Context:
//    Parameters:
//         storage - memory storage
//         pos - position structure that was filled with cvSaveMemStoragePos
//    Returns:
//    Notes:
//F*/
OPENCVAPI  void  cvRestoreMemStoragePos( CvMemStorage* storage, CvMemStoragePos* pos );


/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:    cvCreateSeq
//    Purpose: creates sequence, located on the storage
//    Context:
//    Parameters:
//         seq_flags - flags of created sequence
//         header_size - size of sequence header. Must be non-less than sizeof(CvSeq)
//         elem_size - size of sequence elements
//         storage - memory storage
//    Returns:
//      created sequence
//    Notes:
//F*/
OPENCVAPI  CvSeq*  cvCreateSeq( int seq_flags, int header_size,
                             int elem_size, CvMemStorage* storage );


/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:    cvSetSeqBlockSize
//    Purpose: adjusts granularity of memory allocation for sequence
//    Context:
//    Parameters:
//         seq - sequence pointer
//         delta_elements - how many elements to allocate when there is no free space
//                          in the sequence.
//    Returns:
//    Notes:
//      If this function is not called after sequence is created,
//      delta_elements is set to ~1K/elem_size
//F*/
OPENCVAPI  void  cvSetSeqBlockSize( CvSeq* seq, int delta_elements );


/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:    cvSeqPush
//    Purpose: adds element in the end of sequence
//    Context:
//    Parameters:
//         seq - sequence pointer
//         element - added element
//    Returns:
//F*/
OPENCVAPI  char*  cvSeqPush( CvSeq* seq, void* element CV_DEFAULT(0));


/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:    cvSeqPushFront
//    Purpose: adds element in the beginning of sequence
//    Context:
//    Parameters:
//         seq     - sequence pointer
//         element - added element
//    Returns:
//F*/
OPENCVAPI  char*  cvSeqPushFront( CvSeq* seq, void* element CV_DEFAULT(0));


/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:    cvSeqPop
//    Purpose: removes element from the end of sequence
//    Context:
//    Parameters:
//         seq     - sequence pointer
//         element - optional parameter. If pointer is not NULL,
//                   removed element is copied there.
//    Returns:
//F*/
OPENCVAPI  void  cvSeqPop( CvSeq* seq, void* element );


/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:    cvSeqPopFront
//    Purpose: removes element from the beginning of sequence
//    Context:
//    Parameters:
//         seq     - sequence pointer
//         element - optional parameter. If pointer is not NULL,
//                   removed element is copied there.
//    Returns:
//F*/
OPENCVAPI  void  cvSeqPopFront( CvSeq* seq, void* element );


/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:    cvSeqPushMulti
//    Purpose: adds several elements in the end of sequence
//    Context:
//    Parameters:
//         seq      - sequence pointer
//         count    - number of added elements
//         elements - array of added elements.
//    Returns:
//F*/
OPENCVAPI  void  cvSeqPushMulti( CvSeq* seq, void* elements, int count );


/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:    cvSeqPopMulti
//    Purpose: removes several elements from the end of sequence
//    Context:
//    Parameters:
//         seq      - sequence pointer
//         count    - number of removed elements
//         elements - optional parameter. If not NULL, removed elements are copied there
//    Returns:
//F*/
OPENCVAPI  void  cvSeqPopMulti( CvSeq* seq, void* elements, int count );


/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:    cvSeqInsert
//    Purpose: inserts element in the middle of the sequence
//    Context:
//    Parameters:
//         sequence     - sequence pointer
//         before_index - index of element, before which the element is inserted
//         element      - inserted element
//    Returns:
//F*/
OPENCVAPI  char*  cvSeqInsert( CvSeq* seq, int before_index,
                               void* element CV_DEFAULT(0));

/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:    cvSeqRemove
//    Purpose: removes element from the middle of the sequence
//    Context:
//    Parameters:
//         seq      - sequence pointer
//         index    - index of removed element
//    Returns:
//F*/
OPENCVAPI  void  cvSeqRemove( CvSeq* seq, int index );


/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:    cvClearSeq
//    Purpose: clears sequence(removes all sequence elements)
//    Context:
//    Parameters:
//         seq - sequence pointer
//    Returns:
//F*/
OPENCVAPI  void  cvClearSeq( CvSeq* seq );


/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:    cvGetSeqElem
//    Purpose: finds sequence element by its index
//    Context:
//    Parameters:
//         seq - sequence pointer
//         index - element index
//         block - optional output parameter. Sequence block, containing found element
//    Returns:
//         pointer to found element or NULL.
//    Notes:
//         index == -1 means last sequence element, -2 - prelast element etc.
//F*/
OPENCVAPI  char*  cvGetSeqElem( CvSeq* seq, int index, CvSeqBlock** block CV_DEFAULT(0) );


/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:    cvSeqElemIdx
//    Purpose: calculates element index from its address
//    Context:
//    Parameters:
//         seq - sequence pointer
//         element - sequence element
//         block - optional output parameter. Sequence block, containing found element.
//    Returns:
//         index of sequence element
//F*/
OPENCVAPI int  cvSeqElemIdx( CvSeq* seq, void* element, CvSeqBlock** block CV_DEFAULT(0) );


/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:    cvStartAppendToSeq
//    Purpose: initializes writer state for further writing to sequence
//    Context:
//    Parameters:
//         seq - sequence pointer
//         writer - pointer to initialized writer state
//    Returns:
//F*/
OPENCVAPI  void  cvStartAppendToSeq( CvSeq* seq, CvSeqWriter* writer );


/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:    cvStartWriteSeq
//    Purpose: creates new sequence and initializes writer for it
//    Context:
//    Parameters:
//         seq_flags - flags of created sequence
//         header_size - size of sequence header. Must be non-less than sizeof(CvSeq)
//         elem_size - size of sequence elements
//         storage - memory storage, where the sequence will be located
//         writer - pointer to initialized writer state
//    Returns:
//F*/
OPENCVAPI  void  cvStartWriteSeq( int seq_flags, int header_size,
                               int elem_size, CvMemStorage* storage,
                               CvSeqWriter* writer );

/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:    cvEndWriteSeq
//    Purpose: ends writing process and closes writer
//    Context:
//    Parameters:
//         writer - writer state
//    Returns:
//         written sequence
//F*/
OPENCVAPI  CvSeq*  cvEndWriteSeq( CvSeqWriter* writer );


/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:    cvFlushSeqWriter
//    Purpose: updates sequence headers, but don't close writer
//    Context:
//    Parameters:
//         writer - writer state
//    Returns:
//F*/
OPENCVAPI  void   cvFlushSeqWriter( CvSeqWriter* writer );


/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:    cvStartReadSeq
//    Purpose: initializes sequence reader
//    Context:
//    Parameters:
//         seq - sequence pointer
//         reader - pointer to initialized reader state
//         reverse - if not 0, function moves read position to the end of sequence
//    Returns:
//F*/
OPENCVAPI void cvStartReadSeq( CvSeq* seq, CvSeqReader* reader, int reverse CV_DEFAULT(0) );


/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:    cvGetSeqReaderPos
//    Purpose: returns read position
//    Context:
//    Parameters:
//         reader - reader state
//    Returns:
//         read position
//F*/
OPENCVAPI  int    cvGetSeqReaderPos( CvSeqReader* reader );


/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:    cvSetSeqReaderPos
//    Purpose: moves read position
//    Context:
//    Parameters:
//         index  - new read position
//         is_relative - if not 0, index is offset from current position
//                      (else it is absolute position). Position is changed cyclically
//         reader - reader state
//    Returns:
//F*/
OPENCVAPI  void   cvSetSeqReaderPos( CvSeqReader* reader, int index,
                                  int is_relative CV_DEFAULT(0));


/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:    cvCvtSeqToArray
//    Purpose: copies sequence to array
//    Context:
//    Parameters:
//         seq - source sequence
//         array - destination array. Must have capacity at least
//                 seq->total*seq->elem_siz bytes
//    Returns:
//         pointer to array.
//F*/
OPENCVAPI  void*  cvCvtSeqToArray( CvSeq* seq, CvArr* array,
                                   CvSlice slice CV_DEFAULT(CV_WHOLE_SEQ) );


/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:    cvMakeSeqHeaderForArray
//    Purpose: wraps array with sequence(without copying data)
//    Context:
//    Parameters:
//         seq_flags - flags of sequence
//         header_size - size of sequence header. Must be non-less than sizeof(CvSeq)
//         elem_size - size of sequence elements
//         array - source array.
//         total - total number of elements in array
//         seq   - pointer to local structure CvSeq
//         block - pointer to local structure CvSeqBlock
//    Returns:
//F*/
OPENCVAPI  void  cvMakeSeqHeaderForArray( int seq_type, int header_size,
                                          int elem_size, CvArr* array, int total,
                                          CvSeq* seq, CvSeqBlock* block );

/************ Internal sequence functions ************/
OPENCVAPI  void  cvChangeSeqBlock( CvSeqReader* reader, int direction );
OPENCVAPI  void  cvCreateSeqBlock( CvSeqWriter* writer );


/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:    cvCreateSet
//    Purpose: creates new set
//    Context:
//    Parameters:
//         set_flags - flags of set
//         header_size - size of set header. Must be non-less than sizeof(CvSet)
//         elem_size - size of set elements.
//                     Must be non-less than 8 bytes, divisible by 4.
//                     Least significant bit of first 4-byte field of set elements must
//                     be zero.
//         storage   - memory storage, where the set will be located
//    Returns:
//         created set
//F*/
OPENCVAPI  CvSet*   cvCreateSet( int set_flags, int header_size,
                              int elem_size, CvMemStorage* storage );


/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:    cvSetAdd
//    Purpose: adds new element to the set
//    Context:
//    Parameters:
//         set - set
//         element - optional input parameter. If non NULL, it is copied to inserted
//                   element(starting from second 4-byte field)
//         inserted_element - optional output parameter. If non NULL, address of inserted
//                   element is stored there
//    Returns:
//         index of added element
//F*/
OPENCVAPI  int   cvSetAdd( CvSet* set_struct, CvSetElem* element CV_DEFAULT(0),
                           CvSetElem** inserted_element CV_DEFAULT(0) );


/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:    cvSetRemove
//    Purpose: removes element from the set
//    Context:
//    Parameters:
//         set - set
//         index - index of removed element
//    Returns:
//F*/
OPENCVAPI  void   cvSetRemove( CvSet* set_struct, int index );


/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:    cvGetSetElem
//    Purpose: finds set element by its index
//    Context:
//    Parameters:
//         set - set
//         index - element index
//    Returns:
//         pointer to element or 0 if index is out of range or element at this index
//         isn't in the set
//F*/
OPENCVAPI  CvSetElem*  cvGetSetElem( CvSet* set_struct, int index );


/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:    cvClearSet
//    Purpose: clear set(removes all elements from the set)
//    Context:
//    Parameters:
//         set - set
//    Returns:
//F*/
OPENCVAPI  void   cvClearSet( CvSet* set_struct );


/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:    cvCreateGraph
//    Purpose: creates graph
//    Context:
//    Parameters:
//         graph_flags - flags of created graph. CV_SEQ_KIND_GRAPH must be set,
//                       CV_GRAPH_FLAG_ORIENTED(if set) means oriented graph.
//         header_size - size of graph header. Must be non-less than sizeof(CvGraph)
//         vtx_size - size of graph vertices. Must be GREATER than sizeof(CvGraphVtx).
//                   (for example, sizeof(CvGraphVtx2D) can be used
//                     for simple graphs on the plane)
//         edge_size - size of graph edges. Must be non-less than sizeof(CvGraphEdge)
//         storage   - memory storage, where the graph will be located
//    Returns:
//         created graph
//F*/
OPENCVAPI  CvGraph*   cvCreateGraph( int graph_flags, int header_size,
                                  int vtx_size, int edge_size,
                                  CvMemStorage* storage );

/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:    cvGraphAddVtx
//    Purpose: adds vertex to graph
//    Context:
//    Parameters:
//         graph - graph
//         vertex - optional input parameter. If pointer to vertex is not NULL,
//                  it is copied to inserted vertex
//                 (first sizeof(CvGraphVtx) bytes aren't copied)
//         inserted_vertex - optional output parameter. If not NULL, pointer to inserted
//                  vertex is stored there
//    Returns:
//         index of inserted vertex
//F*/
OPENCVAPI  int   cvGraphAddVtx( CvGraph* graph, CvGraphVtx* vertex CV_DEFAULT(0),
                                CvGraphVtx** inserted_vertex CV_DEFAULT(0) );


/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:    cvGraphRemoveVtx, cvGraphRemoveVtxByPtr
//    Purpose: removes vertex from the graph
//    Context:
//    Parameters:
//         graph - graph
//         index - index of removed vertex
//         vtx - pointer to removed vertex
//    Returns:
//    Notes:
//      Vertex is removed with all the incident edges
//F*/
OPENCVAPI  void   cvGraphRemoveVtx( CvGraph* graph, int index );
OPENCVAPI  void   cvGraphRemoveVtxByPtr( CvGraph* graph, CvGraphVtx* vtx );


/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:    cvGraphAddEdge, cvGraphAddEdgeByPtr
//    Purpose: add edge to graph
//    Context:
//    Parameters:
//         graph - graph
//         start_idx, end_idx - indices of starting and ending vertices
//         start_vtx, end_vtx - pointers to starting and ending vertices
//         edge - optional input parameter. If not NULL, the edge is copied to
//                inserted edge(first sizeof(CvGraphEdge) bytes aren't copied
//         inserted_edge - optional output parameter. Points to inserted edge.
//    Returns:
//    ... 1 if the edge is inserted, 0 if the vertices were connected already,
//    -1 if a critical error occured (normally, an error message box appears in this case)
//    Notes:
//       starting vertex must differ from ending one.
//F*/
OPENCVAPI  int  cvGraphAddEdge( CvGraph* graph,
                                int start_idx, int end_idx,
                                CvGraphEdge* edge CV_DEFAULT(0),
                                CvGraphEdge** inserted_edge CV_DEFAULT(0) );

OPENCVAPI  int  cvGraphAddEdgeByPtr( CvGraph* graph,
                               CvGraphVtx* start_vtx, CvGraphVtx* end_vtx,
                               CvGraphEdge* edge CV_DEFAULT(0),
                               CvGraphEdge** inserted_edge CV_DEFAULT(0) );

/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:    cvGraphRemoveEdge, cvGraphRemoveEdgeByPtr
//    Purpose: removes edge to graph
//    Context:
//    Parameters:
//         graph - graph
//         start_idx, end_idx - indices of starting and ending vertices
//         start_vtx, end_vtx - pointers to starting and ending vertices
//    Returns:
//F*/
OPENCVAPI  void   cvGraphRemoveEdge( CvGraph* graph, int start_idx, int end_idx );
OPENCVAPI  void   cvGraphRemoveEdgeByPtr( CvGraph* graph, CvGraphVtx* start_vtx,
                                          CvGraphVtx* end_vtx );

/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:    cvFindGraphEdge, cvFindGraphEdgeByPtr
//    Purpose: finds edge, connecting two vertices. If graph is orientation, order
//             of input vertices is taken into account
//    Context:
//    Parameters:
//         graph - graph
//         start_idx, end_idx - indices of starting and ending vertices
//         start_vtx, end_vtx - pointers to starting and ending vertices
//    Returns:
//F*/
OPENCVAPI  CvGraphEdge*  cvFindGraphEdge( CvGraph* graph, int start_idx, int end_idx );
OPENCVAPI  CvGraphEdge*  cvFindGraphEdgeByPtr( CvGraph* graph, CvGraphVtx* start_vtx,
                                               CvGraphVtx* end_vtx );


/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:    cvClearGraph
//    Purpose: clear graph(removes all the edges and vertices from the graph)
//    Context:
//    Parameters:
//         graph - graph
//    Returns:
//F*/
OPENCVAPI  void  cvClearGraph( CvGraph* graph );


/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:    cvGraphVtxDegree, cvGraphVtxDegreeByPtr
//    Purpose: counts edges, incident to given vertex
//    Context:
//    Parameters:
//         graph - graph
//         vtx_idx - vertex index
//         vtx - pointer to vertex
//    Returns:
//      number of incident edges
//F*/
OPENCVAPI  int  cvGraphVtxDegree( CvGraph* graph, int vtx_idx );
OPENCVAPI  int  cvGraphVtxDegreeByPtr( CvGraph* graph, CvGraphVtx* vtx );


/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:    cvGetGraphVtx
//    Purpose: finds graph vertex by its index
//    Context:
//    Parameters:
//         graph - graph
//         idx - vertex index
//    Returns:
//      pointer to vertex
//F*/
#define cvGetGraphVtx( graph, idx ) (CvGraphVtx*)cvGetSetElem((CvSet*)(graph), (idx))


/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:    cvGraphVtxIdx
//    Purpose: calculates index of the graph vertex
//    Context:
//    Parameters:
//         graph - graph
//         vtx - pointer to vertex
//    Returns:
//      vertex index
//F*/
#define cvGraphVtxIdx( graph, vtx ) cvSeqElemIdx((CvSeq*)(graph),(vtx),0)

/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:    cvGraphEdgeIdx
//    Purpose: calculates index of the graph edge
//    Context:
//    Parameters:
//         graph - graph
//         edge - pointer to graph edge
//    Returns:
//      edge index
//F*/
#define cvGraphEdgeIdx( graph, edge ) cvSeqElemIdx((CvSeq*)((graph)->edges),(edge),0)


/****************************************************************************************\
*                              Planar subdivisions                                       *
\****************************************************************************************/

/************ Data structures and related enumerations ************/

typedef long CvSubdiv2DEdge;

#define CV_QUADEDGE2D_FIELDS()     \
    struct CvSubdiv2DPoint* pt[4]; \
    CvSubdiv2DEdge  next[4];

#define CV_SUBDIV2D_VIRTUAL_POINT  2

#define CV_SUBDIV2D_POINT_FIELDS()\
    int            is_virtual; \
    CvSubdiv2DEdge first;      \
    CvPoint2D32f   pt;

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

#define  CV_SUBDIV2D_NEXT_EDGE( edge )  (((CvQuadEdge2D*)((edge) & ~3))->next[(edge)&3])

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
    return e->pt[edge & 3];
}


CV_INLINE  CvSubdiv2DPoint*  cvSubdiv2DEdgeDst( CvSubdiv2DEdge edge );
CV_INLINE  CvSubdiv2DPoint*  cvSubdiv2DEdgeDst( CvSubdiv2DEdge edge )
{
    CvQuadEdge2D* e = (CvQuadEdge2D*)(edge & ~3);
    return e->pt[(edge + 2) & 3];
}


CV_INLINE  double  cvTriangleArea( CvPoint2D32f a, CvPoint2D32f b, CvPoint2D32f c );
CV_INLINE  double  cvTriangleArea( CvPoint2D32f a, CvPoint2D32f b, CvPoint2D32f c )
{
    return (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
}

/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:    cvCreateSubdiv2D
//    Purpose: creates initially empty planar subdivision structure
//    Context:
//    Parameters:
//      subdiv_type - type of subdivision
//      header_size - size of header(>= sizeof(CvSubdiv2D))
//      quadedge_size - size of quad-edges(>= sizeof(CvQuadEdge2D))
//      vtx_size - size of vertices(>= sizeof(CvSubdiv2DPoint))
//      storage  - size of memory storage
//    Returns:
//      created subdivision
//F*/
OPENCVAPI  CvSubdiv2D*  cvCreateSubdiv2D( int subdiv_type, int header_size,
                                       int vtx_size, int quadedge_size,
                                       CvMemStorage* storage );


/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:    cvSubdiv2DMakeEdge
//    Purpose: creates new isolated quad-edge
//    Context:
//    Parameters:
//      subdiv - subdivision - owner of the quadedge
//    Returns:
//      first edge of quad-edge.
//F*/
OPENCVAPI  CvSubdiv2DEdge  cvSubdiv2DMakeEdge( CvSubdiv2D* subdiv );


/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:    cvSubdiv2DAddPoint
//    Purpose:  basic topological operation: breaks or combines edge rings
//    Context:
//    Parameters:
//      edgeA - first edge
//      edgeB - second edge
//F*/
OPENCVAPI  CvSubdiv2DPoint*   cvSubdiv2DAddPoint( CvSubdiv2D* subdiv,
                                                  CvPoint2D32f pt, int is_virtual );


/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:    cvSubdiv2DSplice
//    Purpose:  basic topological operation: breaks or combines edge rings
//    Context:
//    Parameters:
//      edgeA - first edge
//      edgeB - second edge
//F*/
OPENCVAPI  void  cvSubdiv2DSplice( CvSubdiv2DEdge  edgeA,  CvSubdiv2DEdge  edgeB );


/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:    cvSubdiv2DSetEdgePoints
//    Purpose:  assigns edge origin and desination points
//    Context:
//    Parameters:
//      edge - edge
//      org_pt - point to origin vertex
//      dst_pt - point to destination vertex
//F*/
OPENCVAPI  void  cvSubdiv2DSetEdgePoints( CvSubdiv2DEdge edge,
                                          CvSubdiv2DPoint* org_pt,
                                          CvSubdiv2DPoint* dst_pt );

/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:    cvSubdiv2DDeleteEdge
//    Purpose:  deletes edge from subdivision.
//    Context:
//    Parameters:
//      subdiv - subdivison
//      edge - deleted edge
//F*/
OPENCVAPI  void  cvSubdiv2DDeleteEdge( CvSubdiv2D* subdiv, CvSubdiv2DEdge edge );


/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:    cvSubdiv2DConnectEdges
//    Purpose:  connect destination point of the first edge with
//              origin point of the second edge
//    Context:
//    Parameters:
//      subdiv - subdivison
//      edgeA - first edge
//      edgeB - second edge
//F*/
OPENCVAPI  CvSubdiv2DEdge  cvSubdiv2DConnectEdges( CvSubdiv2D* subdiv,
                                                   CvSubdiv2DEdge edgeA,
                                                   CvSubdiv2DEdge edgeB );


/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:    cvSubdiv2DSwapEdges
//    Purpose:  swap diagonal in two connected Delaunay facets
//    Context:
//    Parameters:
//      subdiv - subdivison
//      edge - sudivision edge
//F*/
OPENCVAPI  void  cvSubdiv2DSwapEdges( CvSubdiv2DEdge edge );


/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:    cvSubdiv2DLocate
//    Purpose:  finds location of the point within the Delaunay triangulation
//              origin point of the second edge
//    Context:
//    Parameters:
//      subdiv - subdivison
//      pt     - searched point
//      _edge  - bounding edge for facet, containing the point
//      _point - vertex(if searched point coincides with the vertex)
//F*/
OPENCVAPI  CvSubdiv2DPointLocation  cvSubdiv2DLocate(
                               CvSubdiv2D* subdiv, CvPoint2D32f pt,
                               CvSubdiv2DEdge *_edge,
                               CvSubdiv2DPoint** _point CV_DEFAULT(0) );


/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:    cvInitSubdivDelaunay2D
//    Purpose:  connect destination point of the first edge with
//              origin point of the second edge
//    Context:
//    Parameters:
//      subdiv - subdivison
//      pt     - searched point
//      _edge  - bounding edge for facet, containing the point
//      _point - vertex(if searched point coincides with the vertex)
//F*/
OPENCVAPI  void  cvInitSubdivDelaunay2D( CvSubdiv2D* subdiv, CvRect rect );


/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:    cvSubdivDelaunay2DInsert
//    Purpose:  insert the point into the triangulation
//    Context:
//    Parameters:
//      subdiv - subdivison
//      pt     - inserted point
//F*/
OPENCVAPI  CvSubdiv2DPoint*  cvSubdivDelaunay2DInsert( CvSubdiv2D* subdiv, CvPoint2D32f pt);


/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:    cvCalcSubdivVoronoi2D
//    Purpose:  calculates Voronoi tesselation( Coordinates of Voronoi points)
//    Context:
//    Parameters:
//      subdiv - subdivison
//    Note:
//      Before calculations the function checks the flag, indicating that
//      the Voronoi tesselation is already calculated.
//      If subdivision is modified(some points have been inserted), the flag is cleared.
//F*/
OPENCVAPI  void  cvCalcSubdivVoronoi2D( CvSubdiv2D* subdiv );


/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:    cvClearSubdivVoronoi2D
//    Purpose:  removes all Voronoi points from the tesselation.
//    Context:
//    Parameters:
//      subdiv - subdivison
//    Note:
//      The function is called implicitly from the cvCalcSubdivVoronoi2D
//      before Voronoi tesselation is calculated.
//F*/
OPENCVAPI  void  cvClearSubdivVoronoi2D( CvSubdiv2D* subdiv );


/****************************************************************************************\
*                              Contours procceding                                       *
\****************************************************************************************/

/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:    cvFindContours
//    Purpose: finds contours on the binary image
//    Context:
//    Parameters:
//         img - binary image(depth is IPL_DEPTH_8U or IPL_DEPTH_8S, single channel).
//         storage - memory storage where the contours will be stored
//         firstContour - output parameter. First contour on the highest level.
//         headerSize - size of contours headers.
//         mode - contour retrieving mode.
//                  CV_RETR_EXTERNAL - get only the most external contours(list).
//                  CV_RETR_LIST     - get all the contours without any hierarchical links
//                                    (list).
//                  CV_RETR_CCOMP    - get all the contours and make holes as child
//                                     contours of corresponding external contour
//                                    (two-level hierarchical structure)
//                  CV_RETR_TREE     - get all the contours and build all
//                                     hierarchical links(tree).
//         method - approximation method
//                CV_CHAIN_CODE    - output contours in chain-coded form(Freeman code).
//                              The rest of methods approximate chain code with polyline
//                CV_CHAIN_APPROX_NONE - no compression. Every point of digital curve
//                                       is coded
//                CV_CHAIN_APPROX_SIMPLE - horizontal, vertical and diagonal segments are
//                                         are coded with ending vertices (by default).
//                CV_CHAIN_APPROX_TC89_L1 - Teh-Chin algorithm, L1 curvature
//                CV_CHAIN_APPROX_TC89_KCOS - Teh-Chin algorithm, k-cosine curvature
//    Returns:
//      Number of contours found.
//F*/
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
    CV_CHAIN_APPROX_TC89_KCOS = 4
} CvChainApproxMethod;

OPENCVAPI  int  cvFindContours( CvArr* array, CvMemStorage* storage,
                           CvSeq**  firstContour,
                           int  headerSize CV_DEFAULT(sizeof(CvContour)),
                           CvContourRetrievalMode mode CV_DEFAULT( CV_RETR_LIST ),
                           CvChainApproxMethod method CV_DEFAULT(CV_CHAIN_APPROX_SIMPLE));


/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:    cvStartFindContours
//    Purpose: starts iterrative process of contours retrieving
//    Context:
//    Parameters:
//         img - binary image(depth is IPL_DEPTH_8U or IPL_DEPTH_8S, single channel).
//         storage - memory storage where the contours will be stored
//         header_size - size of contours headers.
//         mode - contour retrieving mode(see cvFindContours description)
//         method - approximation method(see cvFindContours description)
//    Returns:
//      contour scanner state.
//F*/
OPENCVAPI  CvContourScanner   cvStartFindContours( CvArr* array, CvMemStorage* storage,
                                        int header_size, CvContourRetrievalMode mode,
                                        CvChainApproxMethod method );

/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:    cvFindNextContour
//    Purpose: finds next contour on the image
//    Context:
//    Parameters:
//         scanner - contour scanner state
//    Returns:
//      next contour or NULL, if no more contours on the image
//F*/
OPENCVAPI  CvSeq*  cvFindNextContour( CvContourScanner scanner );


/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:    cvSubstituteContour
//    Purpose: substitutes retrived contour with another one.
//    Context:
//    Parameters:
//         scanner - contour scanner state
//         newContour - substituting contour
//                      (or NULL, if retrived contour should be rejected)
//    Returns:
//    Notes:
//      The function may be called immediately after contour is retrived
//     (may be, after some processing) before cvFindNextContour is called next time.
//      It replaces found contour with processed contour, or even rejects it.
//F*/
OPENCVAPI  void   cvSubstituteContour( CvContourScanner scanner, CvSeq* newContour );


/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:    cvEndFindContours
//    Purpose: finishes process of contours retrieving
//    Context:
//    Parameters:
//         scanner - contour scanner state
//    Returns:
//      pointer to first contour on the highest hierarchical level
//F*/
OPENCVAPI  CvSeq*  cvEndFindContours( CvContourScanner* scanner );


/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:    cvApproxChains
//    Purpose: approximates single(all) chains with polylines.
//    Context:
//    Parameters:
//         src_seq - pointer to chain(which can reffer to other chains).
//         storage - where to place resultant polylines.
//         dst_seq - double pointer to first resultant polyline.
//         method  - approximation method(see cvFindContours description)
//         parameter - method parameter(is not used now).
//         minimal_perimeter - approximates only those contours which perimeter is
//                             not less than <minimal_perimeter>. Other chains
//                             are removed from resultant structure
//         recursive - if not 0, approximate all the chains, which can be accessed
//                     from src_seq. if 0, approximate a single chain
//    Returns:
//F*/
OPENCVAPI  CvSeq* cvApproxChains( CvSeq* src_seq, CvMemStorage* storage,
                            CvChainApproxMethod method CV_DEFAULT(CV_CHAIN_APPROX_SIMPLE),
                            double parameter CV_DEFAULT(0),
                            int  minimal_perimeter CV_DEFAULT(0),
                            int  recursive CV_DEFAULT(0));


/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:    cvStartReadChainPoints
//    Purpose: starts read successive points of the chain-coded curve
//    Context:
//    Parameters:
//         chain   - chain
//         reader  - chain reader state
//    Returns:
//F*/
typedef struct CvChainPtReader
{
    CV_SEQ_READER_FIELDS()
    char      code;
    CvPoint  pt;
    char      deltas[8][2];
    int       reserved[2];
} CvChainPtReader;

OPENCVAPI  void  cvStartReadChainPoints( CvChain* chain, CvChainPtReader* reader );

/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:    cvReadChainPoint
//    Purpose: read current point of the chain and moves read position to the next code
//    Context:
//    Parameters:
//         reader - chain reader state
//    Returns:
//         current point of the chain
//F*/
OPENCVAPI  CvPoint   cvReadChainPoint( CvChainPtReader* reader );


/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:    cvApproxPoly
//    Purpose: approximates polygonal curve (either closed or not)
//             with another polygonal curve with desired accuracy
//    Context:
//    Parameters:
//         src_seq - source contour
//         header_size - size of destination contour header
//         storage - memory storage for result
//         dst_seq - destination contour
//         method  - approximation method. Only a single method is implemented now.
//                   CV_POLY_APPROX_DP - Douglas-Peucker method.
//         parameter - depends on method. For CV_POLY_APPROX_DP it is a desired accuracy.
//         recursive - if not 0, the function approximates all the contours that
//                     are next to or below the initial contour, otherwise the single
//                     contour is approximated
//    Returns:
//F*/
typedef enum CvPolyApproxMethod
{
    CV_POLY_APPROX_DP = 0
}
CvPolyApproxMethod;

OPENCVAPI  CvSeq*  cvApproxPoly( CvSeq* src_seq, int  header_size, CvMemStorage* storage,
                                 CvPolyApproxMethod  method, double parameter,
                                 int recursive CV_DEFAULT(0));

/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:    cvContourPerimeter
//    Purpose:
//      Calculates contour perimeter, finds minimal edge and maximal egde lengths
//    Context:
//    Parameters:
//      contour  - source contour
//      slice    - optional parameter. ending and starting indices of contour section  
//    Returns:
//      contour section perimeter
//      when a part of contour is selected, the function doesn't add
//      length of chord, connecting starting and ending points
//F*/
OPENCVAPI  double  cvContourPerimeter( CvSeq* contour,
                                       CvSlice slice CV_DEFAULT(CV_WHOLE_SEQ) );


/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:    cvContourBoundingRect
//    Purpose:
//      calculates contour bounding rebox
//    Context:
//    Parameters:
//      contour  - pointer to the source contour
//      update   - attribute of contour bounding box updating
//                 (if update = 0 the bounding box isn't updated)
//    Returns:
//      bounding rectangle
//F*/
OPENCVAPI  CvRect  cvContourBoundingRect( CvSeq* contour, int update CV_DEFAULT(0) );


/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name: cvContourMoments
//    Purpose:
//      Calculates spatial and central moments of the contour up to order 3
//    Context:
//    Parameters:
//      contour - the source contour
//      moments - output parameter. Pointer to the calculated moments
//
//F*/
OPENCVAPI  void  cvContourMoments( CvSeq* contour, CvMoments* moments);

/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name: cvContourArea
//    Purpose:
//      Calculates area within the contour
//    Context:
//    Parameters:
//      contour - pointer to input contour object.
//      slice  - optional parameter. ending and starting indices of contour section  
//    Returns:
//      Contour section area
//F*/
OPENCVAPI  double  cvContourArea( CvSeq* contour,
                               CvSlice slice CV_DEFAULT(CV_WHOLE_SEQ));

/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name: cvMatchContours
//    Purpose:
//      Compares two contours
//    Context:
//    Parameters:
//      contour1 - pointer to the first input contour object.
//      contour2 - pointer to the second input contour object.
//      method - method for the matching calculation
//     (now CV_CONTOURS_MATCH_I1, CV_CONTOURS_MATCH_I2 or
//      CV_CONTOURS_MATCH_I3 only  )
//      parameter - method-specific parameter (is used now)
//    Returns:
//      Comparison result
//F*/
typedef enum CvContoursMatchMethod
{
    CV_CONTOURS_MATCH_I1 = 1,
    CV_CONTOURS_MATCH_I2 = 2,
    CV_CONTOURS_MATCH_I3 = 3
}
CvContoursMatchMethod;

OPENCVAPI  double  cvMatchContours( CvSeq* contour1, CvSeq* contour2,
                                 CvContoursMatchMethod method,
                                 long parameter CV_DEFAULT(0));

/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name: cvCreateContourTree
//    Purpose:
//      Creates binary tree representation for the contour
//    Context:
//    Parameters:
//      contour - input contour
//      storage - storage
//      tree    - output pointer to the binary tree representation
//      threshold - threshold for the binary tree building
//    Returns:
//      Binary tree
//F*/
typedef struct CvContourTree
{
    CV_SEQUENCE_FIELDS()
    CvPoint p1;            /* the first point of the binary tree root segment */
    CvPoint p2;            /* the last point of the binary tree root segment */
} CvContourTree;

OPENCVAPI  CvContourTree*   cvCreateContourTree( CvSeq* contour, CvMemStorage* storage,
                                                 double threshold );

/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name: cvContourFromContourTree
//    Purpose:
//      Reconstructs contour from binary tree representation
//    Context:
//    Parameters:
//      tree   -  input binary tree representation
//      storage - memory storage
//      criteria - criteria for the definition threshold value
//                 for the contour reconstruction(level or precision)
//    Returns:
//      Created contour
//F*/
OPENCVAPI  CvSeq*  cvContourFromContourTree( CvContourTree *tree,
                                          CvMemStorage* storage,
                                          CvTermCriteria criteria );

/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name: cvMatchContourTrees
//    Purpose:
//      Compares two contour trees
//    Context:
//    Parameters:
//      tree1 - pointer to the first input contour tree object.
//      tree2 - pointer to the second input contour tree object.
//      method - method for the matching calculation
//     (now CV_CONTOUR_TREES_MATCH_I1 only  )
//      threshold - threshold for the contour trees matching
//    Returns:
//      comparison result
//F*/
typedef enum CvContourTreesMatchMethod
{
    CV_CONTOUR_TREES_MATCH_I1 = 1
}
CvContourTreesMatchMethod;

OPENCVAPI  double  cvMatchContourTrees( CvContourTree *tree1,
                                     CvContourTree *tree2,
                                     CvContourTreesMatchMethod method,
                                     double threshold );

/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name: cvDrawContours
//    Purpose:
//      Draws one or more contours outlines or their interiors on the image
//    Context:
//    Parameters:
//      img      - destination three-channel image
//      contour  - pointer to drawn contour(s).
//      external_color - color to draw external contours with
//      hole_color - color to draw hole contours with
//      max_level  - max level of the tree(starting from contour pointer) to draw.
//                   if it is 0, draw single contour, if 1 - draw the contour and
//                   other contours at the same level, 2 - draw two levels etc.
//      thickness - thickness of lines the contours are drawn with. If it is
//                  equal to CV_FILLED (-1), the contour(s) interior is filled.
//    Returns:
//F*/
OPENCVAPI void  cvDrawContours( void *img, CvSeq* contour,
                                double external_color, double hole_color,
                                int max_level, int thickness CV_DEFAULT(1),
                                int connectivity CV_DEFAULT(8));

/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name: cvCalcPGH
//    Purpose:
//      Calculates PGH(pairwise geometric histogram) for given contour.
//    Context:
//    Parameters:
//      contour  - input contour.
//      pgh      - output histogram(must be two-dimensional)
//    Returns:
//F*/
OPENCVAPI  void  cvCalcPGH( CvSeq* contour, CvHistogram* hist );

/****************************************************************************************\
*                          Computational Geometry functions                              *
\****************************************************************************************/

#define CV_CLOCKWISE         1
#define CV_COUNTER_CLOCKWISE 2

/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name: cvConvexHull
//    Purpose:
//      Finds convex hull of points set
//    Context:
//    Parameters:
//      points       - array of points
//      num_points   - number of input points
//      bound_rect   - pointer to bounding rectangle(if computed), can be NULL
//      orientation  - orientation of convex hull you want to get
//                     can be CV_CLOCKWISE or CV_COUNTER_CLOCKWISE
//      hull         - pointer to output array
//      hullsize     - pointer to output value, which is number of convex hull vertices
//
//    Returns:
//    Notes: Function computes convex hull and stores result in "hull" array,
//           where every vertex of convex hull is represented by index in input array.
//
//F*/
OPENCVAPI void cvConvexHull( CvPoint* points, int num_points, CvRect* bound_rect,
                          int orientation, int* hull, int* hullsize );

/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name: cvContourConvexHull
//    Purpose:
//      Finds convex hull of contour
//    Context:
//    Parameters:
//      contour      - pointer to CvSeq structure, which elements are CvPoints
//      orientation  - orientation of convex hull you want to get
//                     can be CV_CLOCKWISE or CV_COUNTER_CLOCKWISE
//      storage      - pointer to memory storage, where output sequence will be stored
//    Returns:
//      Convex hull
//    Notes: Function computes convex hull and returns it.
//           Every vertex of convex hull is represented by pointer to original point,
//           stored in input sequence, i.e. result is CvSeq which elements
//           have type CvPoint*
//F*/
OPENCVAPI CvSeq*  cvContourConvexHull( CvSeq* contour, int orientation,
                                    CvMemStorage* storage );


/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name: cvConvexHullApprox
//    Purpose:
//      Finds approximate convex hull of points set
//    Context:
//    Parameters:
//      points       - array of points
//      num_points   - number of input points
//      bound_rect   - pointer to bounding rectangle(if computed), can be NULL
//      bandwidth    - width of band, used in algorithm
//      orientation  - orientation of convex hull you want to get
//                     can be CV_CLOCKWISE or CV_COUNTER_CLOCKWISE
//      hullpoints   - pointer to output array
//      hullsize     - pointer to output value, which is number of convex hull vertices
//
//    Returns:
//    Notes: Function computes approximate convex hull and stores result in "hull" array,
//           where every vertex of convex hull is represented by index in input array.
//           If bandwidth == 1, then exact convex hull is computed.
//
//F*/
OPENCVAPI void  cvConvexHullApprox( CvPoint* points, int num_points,
                                  CvRect* bound_rect, int bandwidth,
                                  int orientation, int* hullpoints, int* hullsize );

/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name: cvContourConvexHullApprox
//    Purpose:
//      Finds approximate convex hull of contour
//    Context:
//    Parameters:
//      contour      - pointer to CvSeq structure, which elements are CvPoints
//      bandwidth    - width of band, used in algorithm
//      orientation  - orientation of convex hull you want to get
//                     can be CV_CLOCKWISE or CV_COUNTER_CLOCKWISE
//      storage      - pointer to memory storage, where output sequence will be stored
//    Returns:
//    Notes: Function computes approximate convex hull and returns it.
//           Every vertex of convex hull is represented by pointer to original point,
//           stored in input sequence, i.e. result is CvSeq which elements
//           have type CvPoint*
//F*/
OPENCVAPI CvSeq*  cvContourConvexHullApprox( CvSeq* contour, int bandwidth,
                                          int orientation, CvMemStorage* storage );

/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name: cvCheckContourConvexity
//    Purpose:
//      Checks if contour is convex or not
//    Context:
//    Parameters:
//      contour - input contour
//
//    Returns: 0 - contour is not convex
//             1 - contour is convex
//    Notes:
//F*/
OPENCVAPI int  cvCheckContourConvexity( CvSeq* contour );

/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name: cvConvexityDefects
//    Purpose:
//      Computes convexity defects of contour
//    Context:
//    Parameters:
//      contour      - pointer to CvSeq structure, which elements are CvPoints
//      convexhull   - pointer to convex hull of input contour
//      storage      - pointer to memory storage, where output sequence will be stored
//    Returns:
//      sequence of convexity defects
//      (i.e. the resultant sequence elements have type CvConvexityDefect).
//    Notes:
//F*/
typedef struct CvConvexityDefect
{
    CvPoint* start;
    CvPoint* end;
    CvPoint* depth_point;
    float depth;
} CvConvexityDefect;

OPENCVAPI CvSeq*  cvConvexityDefects( CvSeq* contour, CvSeq* convexhull,
                                      CvMemStorage* storage );


/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name: cvFitEllipse
//    Purpose:
//      Fits "least-square optimal" ellipse into the set of points
//    Context:
//    Parameters:
//      points       - array of 2D points with float coordinates
//      n            - number of input points
//      box          - output structure which contents center of ellipse
//                     full sizes of ellipse axis and angle to horisont
//    Returns:
//    Notes:
//F*/
OPENCVAPI void  cvFitEllipse( CvPoint2D32f* points, int n, CvBox2D* box );

/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name: cvMinAreaRect
//    Purpose:
//      For set of points finds enclosing rectangle which has minimal area among all such
//      rectangles. Function uses Toussaint algorithm(rotating calipers)
//    Context:
//    Parameters:
//       points - input points
//       n      - number of points
//       left,
//       bottom,
//       right,
//       top    - indices in input array of most left, bottom, right and top points
//       anchor - coordinates of one of corners of output rectangle
//       vect1,
//       vect2  - two vectors, which represents sides of rectangle which are incident
//                to anchor
//    Returns:
//    Notes:
//F*/
OPENCVAPI void  cvMinAreaRect( CvPoint* points, int n,
                             int left, int bottom, int right, int top,
                             CvPoint2D32f* anchor,
                             CvPoint2D32f* vect1,
                             CvPoint2D32f* vect2 );

/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name: cvMinEnclosingCircle
//    Purpose:
//      Finds minimal enclosing circle for point set
//    Context:
//    Parameters:
//      seq      - sequence of points
//      center   - center of min enclosing circle
//      radius   - radius of enclosing circle
//    Returns:
//    Notes:
//F*/
OPENCVAPI  void  cvMinEnclosingCircle( CvSeq* seq, CvPoint2D32f* center, float* radius );

/****************************************************************************************\
*                                  Histogram functions                                   *
\****************************************************************************************/

/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:       cvCalcEMD
//    Purpose:    Computes Earth mover distance(and/or lower bound of it) for given pair
//                of signatures. Ground distance can calculated as
//                L1, L2 or C distance between features' coordinate vectors or
//                using user-defined distance function.
//    Context:
//    Parameters:
//      signature1  - first signature - array of size1 *(dims + 1) elements
//      signature2  - second signature - array of size2 *(dims + 1) elements
//      dims        - number of dimensions in feature space. If 0, then
//                    signature1 and signature2 are considered as simple 1D histograms,
//                    else both signatures must look as follows:
//                   (weight_i0, x0_i0, x1_i0, ..., x(dims-1)_i0,
//                     weight_i1, x0_i1, x1_i1, ..., x(dims-1)_i1,
//                     ...
//                     weight_(size1-1),x0_(size1-1),x1_(size1-1,...,x(dims-1)_(size1-1))
//
//                     where weight_ik - weight of ik cluster.
//                     x0_ik,...,x(dims-1)_ik - coordinates of ik cluster.
//
//      dist_type   - CV_DIST_L1, CV_DIST_L2, CV_DIST_C mean one of standard metrics.
//                   ((CvDisType)-1) means user-defined distance function, which is
//                    passes two coordinate vectors and user parameter, and which returns
//                    distance between those feature points.
//      emd         - pointer to calculated emd distance
//      lower_bound - pointer to calculated lower bound.
//                    if 0, this quantity is not calculated(only emd is calculated).
//                    else if calculated lower bound is greater or equal to the value,
//                    stored at this pointer, then the true emd is not calculated, but
//                    is set to that lower_bound.
//    Returns:
//    Notes:
//F*/
CV_EXTERN_C_FUNCPTR( float (CV_CDECL * CvDistanceFunction)
                     ( const float* a, const float* b, void* user_param ));

OPENCVAPI  float  cvCalcEMD( const float* signature1, int size1,
                             const float* signature2, int size2,
                             int dims, CvDisType dist_type,
                             CvDistanceFunction dist_func,
                             float* lower_bound,
                             void* user_param );

/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:       cvCreateHist
//    Purpose:    Creates histogram
//    Context:
//    Parameters:
//      c_dims - number of dimension in the histogram
//      dims   - array, containing number of bins per each dimension
//      type   - type of histogram. Now, CV_HIST_ARRAY is only supported type.
//      ranges - array of bin ranges.
//      uniform - flag; non 0 if histogram bins are evenly spaced.
//    Returns:
//      Created histogram.
//F*/
OPENCVAPI  CvHistogram*  cvCreateHist( int c_dims, int* dims,
                                    CvHistType type,
                                    float** ranges CV_DEFAULT(0),
                                    int uniform CV_DEFAULT(1));


/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:       cvSetHistBinRanges
//    Purpose:    Sets histogram bins' ranges
//    Context:
//    Parameters:
//      ranges - array of bin ranges.
//      uniform - flag; non 0 if histogram bins are evenly spaced.
//    Returns:
//      nothing
//    Notes:      if uniform parameter is not NULL then thresh[i][0] - minimum value,
//                thresh[i][1] - maximum value of thresholds for dimension i
//F*/
OPENCVAPI void  cvSetHistBinRanges( CvHistogram* hist, float** ranges,
                                 int uniform CV_DEFAULT(1));

/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:       cvMakeHistHeaderForArray
//    Purpose:    Initializes histogram header and sets
//                its data pointer to given value
//    Context:
//    Parameters:
//      c_dims - number of dimension in the histogram
//      dims   - array, containing number of bins per each dimension
//      hist   - pointer to histogram structure. It will have CV_HIST_ARRAY type.
//      data   - histogram data
//    Returns:
//F*/
OPENCVAPI  void  cvMakeHistHeaderForArray( int  c_dims, int* dims, CvHistogram* hist,
                                           float* data, float** ranges CV_DEFAULT(0),
                                           int uniform CV_DEFAULT(1));


/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:       cvReleaseHist
//    Purpose:    Releases histogram header and underlying data
//    Context:
//    Parameters:
//      hist - pointer to released histogram.
//    Returns:
//F*/
OPENCVAPI  void  cvReleaseHist( CvHistogram** hist );


/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:       cvClearHist
//    Purpose:    Clears histogram(sets all bins to zero)
//    Context:
//    Parameters:
//      hist - pointer to cleared histogram.
//    Returns:
//F*/
OPENCVAPI  void  cvClearHist( CvHistogram* hist);


/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:       cvQueryHistValue....
//    Purpose:    Returns value or histogram bin, given its cooridinates
//    Context:
//    Parameters:
//      hist - pointer to histogram.
//      idx0 - index for the 1st dimension
//      idx1 - index for the 2nd dimension
//             ...
//      idx  - array of coordinates(for multi-dimensonal histogram)
//    Returns:
//      Value of histogram bin
//    Notes:
//      For non-array histogram function returns 0 if the specified element isn't present
//F*/
OPENCVAPI  float  cvQueryHistValue_1D( CvHistogram* hist, int idx0 );
OPENCVAPI  float  cvQueryHistValue_2D( CvHistogram* hist, int idx0, int idx1 );
OPENCVAPI  float  cvQueryHistValue_3D( CvHistogram* hist, int idx0, int idx1, int idx2 );
OPENCVAPI  float  cvQueryHistValue_nD( CvHistogram* hist, int* idx );


/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:       cvGetHistValue....
//    Purpose:    Returns pointer to histogram bin, given its cooridinates
//    Context:
//    Parameters:
//      hist - pointer to histogram.
//      idx0 - index for the 1st dimension
//      idx1 - index for the 2nd dimension
//             ...
//      idx  - array of coordinates(for multi-dimensonal histogram).
//             must have hist->c_dims elements.
//    Returns:
//      Pointer to histogram bin
//    Notes:
//      For non-array histogram function creates a new element if it is not exists.
//F*/
OPENCVAPI  float*  cvGetHistValue_1D( CvHistogram* hist, int idx0 );
OPENCVAPI  float*  cvGetHistValue_2D( CvHistogram* hist, int idx0, int idx1 );
OPENCVAPI  float*  cvGetHistValue_3D( CvHistogram* hist, int idx0, int idx1, int idx2 );
OPENCVAPI  float*  cvGetHistValue_nD( CvHistogram* hist, int* idx );


/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:       cvGetMinMaxHistValue
//    Purpose:    Finds coordinates and numerical values of minimum and maximum
//                histogram bins
//    Context:
//    Parameters:
//      hist - pointer to histogram.
//      idx_min - pointer to array of coordinates for minimum.
//                if not NULL, must have hist->c_dims elements.
//      value_min - pointer to minimum value of histogram( Can be NULL).
//      idx_max - pointer to array of coordinates for maximum.
//                if not NULL, must have hist->c_dims elements.
//      value_max - pointer to maximum value of histogram( Can be NULL).
//    Returns:
//F*/
OPENCVAPI  void  cvGetMinMaxHistValue( CvHistogram* hist,
                                    float* value_min, float* value_max,
                                    int* idx_min CV_DEFAULT(0), 
                                    int* idx_max CV_DEFAULT(0));

/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:     cvNormalizeHist
//    Purpose:  Normalizes histogram(such that sum of histogram bins becomes factor)
//    Context:
//    Parameters:
//      hist - pointer to normalized histogram.
//    Returns:
//F*/
OPENCVAPI  void  cvNormalizeHist( CvHistogram* hist, double factor );


/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:     cvThreshHist
//    Purpose:  Clears histogram bins that are below specified level
//    Context:
//    Parameters:
//      hist - pointer to histogram.
//      thresh - threshold level
//    Returns:
//F*/
OPENCVAPI  void  cvThreshHist( CvHistogram* hist, double thresh );


/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:     cvCompareHist
//    Purpose:  compares two histograms using specified method
//    Context:
//    Parameters:
//      hist1 - first compared histogram.
//      hist2 - second compared histogram.
//      method - comparison method
//    Returns:
//      value, that characterizes similarity(or difference) of two histograms
//    Notes:
//F*/
OPENCVAPI  double  cvCompareHist( CvHistogram*  hist1,
                               CvHistogram*  hist2,
                               CvCompareMethod method);


/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:       cvCopyHist
//    Purpose:    Copying one histogram to another
//    Context:
//    Parameters:
//      src - source histogram
//      dst - destination histogram
//    Returns:
//    Notes:      if second parameter is pointer to NULL(*dst == 0) then second
//                histogram will be created.
//                both histograms(if second histogram present) must be equal
//                types & sizes
//F*/
OPENCVAPI void  cvCopyHist( CvHistogram* src, CvHistogram** dst );


/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:       cvCalcBayesianProb
//    Purpose:    Calculates bayesian probabilistic histograms
//    Context:
//    Parameters:
//      src - array of source histograms
//      number - number of source/destination histograms 
//      dst - array of destination histograms
//    Returns:
//    Notes:
//F*/
OPENCVAPI void  cvCalcBayesianProb( CvHistogram** src, int number,
                                  CvHistogram** dst);


/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:       cvCalcHist
//    Purpose:    Calculating histogram from array of one-channel images
//    Context:
//    Parameters:
//      img - array of single-channel images
//      hist - histogram to be calculated. It must have as many dimensions as number of
//             images in <img> array.
//      doNotClear - if not 0, the histogram is not cleared before calculations.
//      mask - optional mask that determines pixels that participate in histogram
//             accumulation.
//    Returns:
//    Notes:      if doNotClear parameter is NULL then histogram clearing before
//                calculating(all values sets to NULL)
//F*/
OPENCVAPI  void  cvCalcHist( IplImage** img, CvHistogram* hist,
                          int doNotClear CV_DEFAULT(0),
                          IplImage* mask CV_DEFAULT(0) );

/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:       cvCalcContrastHist
//    Purpose:    Calculates contrast histogram from array of one-channel images
//    Context:
//    Parameters:
//    Returns:
//    Notes:      if dont_clear parameter is NULL then histogram clearing before
//                calculating(all values sets to NULL)
//F*/
OPENCVAPI  void   cvCalcContrastHist( IplImage** img, CvHistogram* hist,
                                   int doNotClear, IplImage* mask );

/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:       cvCalcBackProject
//    Purpose:    Calculates back project of histogram
//      img - array of input single-channel images
//      dst - destination single-channel image
//      hist - histogram, used for calculating back project. It must have as many
//             dimensions as the number of images in the <img> array.
//    Context:
//    Parameters:
//    Returns:
//    Notes:
//F*/
OPENCVAPI  void  cvCalcBackProject( IplImage** img, IplImage* dst,
                                 CvHistogram* hist );


/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:       cvCalcBackProjectPatch
//    Purpose:    Calculating back project patch of histogram
//    Context:
//    Parameters:
//    Returns:
//    Notes:
//F*/
OPENCVAPI  void  cvCalcBackProjectPatch( IplImage** img, IplImage* dst, CvSize range,
                                      CvHistogram* hist, CvCompareMethod method,
                                      double normFactor );


/****************************************************************************************\
*                                  Active contours                                       *
\****************************************************************************************/


/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:       cvSnakeImage
//    Purpose:    Updates active contour in order to minimize its cummulative (internal
//                and external) energy.
//    Context:
//    Parameters:
//      src - source image that represent external energy.
//      points - array of points in the snake.
//      length - number of points
//      alpha, beta, gamma - weights of different energy components
//      coeffUsage - if it is CV_ARRAY then previous three parameters are array of 
//                   <length> elements, otherwise each of them is a pointer to
//                   scalar values.
//      win - half-size of search window. 
//      criteria - termination criteria.
//      calcGradient - if not 0, the function uses magnitude of the source image gradient
//                     as external energy, otherwise the source image pixel values
//                     are just used for this purpose.  
//    Returns:
//    Notes:
//F*/
OPENCVAPI  void  cvSnakeImage( IplImage* src, CvPoint* points,
                            int  length, float* alpha,
                            float* beta, float* gamma,
                            CvCoeffType coeffUsage, CvSize  win,
                            CvTermCriteria criteria, int calcGradient CV_DEFAULT(1));

/****************************************************************************************\
*                              Gesture recognition                                      *
\****************************************************************************************/

/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:     cvFindHandRegion
//    Purpose:  finds hand region in range image data
//    Context:   
//    Parameters: 
//      points - pointer to the input point's set.
//      count  - the number of the input points.
//      indexs - pointer to the input sequence of the point's indexes
//      line   - pointer to the 3D-line
//      size   - size of the hand in meters 
//      flag   - hand direction's flag (0 - left, -1 - right, 
                 otherwise j-index of the initial image center)
//      center - pointer to the output hand center
//      storage - pointer to the memory storage  
//      numbers - pointer to the output sequence of the point's indexes inside
//                hand region                
//      
//    Notes:
//F*/
OPENCVAPI  void  cvFindHandRegion (CvPoint3D32f* points, int count,
                                CvSeq* indexs,
                                float* line, CvSize2D32f size, int flag,
                                CvPoint3D32f* center,
                                CvMemStorage* storage, CvSeq **numbers);



/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:     cvFindHandRegionA
//    Purpose:  finds hand region in range image data
//    Context:
//    Parameters:
//      points - pointer to the input point's set.
//      count  - the number of the input points.
//      indexs - pointer to the input sequence of the point's indexes
//      line   - pointer to the 3D-line
//      size   - size of the hand in meters
//      jc - j-index of the initial image center
//      center - pointer to the output hand center
//      storage - pointer to the memory storage
//      numbers - pointer to the output sequence of the point's indexes inside
//                hand region
//
//    Notes:
//F*/
OPENCVAPI  void  cvFindHandRegionA( CvPoint3D32f* points, int count,
                                CvSeq* indexs,
                                float* line, CvSize2D32f size, int jc,
                                CvPoint3D32f* center,
                                CvMemStorage* storage, CvSeq **numbers);

/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:     cvCalcImageHomography
//    Purpose:  calculates the cooficients of the homography matrix
//    Context:
//    Parameters:
//      line   - pointer to the input 3D-line
//      center - pointer to the input hand center
//      intrinsic - intrinsic camera parameters matrix
//      homography - result homography matrix
//
//    Notes:
//F*/
OPENCVAPI  void  cvCalcImageHomography(float *line, CvPoint3D32f* center,
                                     float intrinsic[3][3], float homography[3][3]);

/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:     cvCreateHandMask
//    Purpose:  creates hand mask image
//    Context:
//    Parameters:
//      numbers - pointer to the input sequence of the point's indexes inside
//                hand region
//      img_mask - pointer to the result mask image
//      roi      - result hand mask ROI
//
//    Notes:
//F*/
OPENCVAPI  void  cvCreateHandMask( CvSeq* numbers,
                                IplImage *img_mask, CvRect *roi);

/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:     cvCalcProbDensity
//    Purpose:  calculates hand mask probability density
//    Context:
//    Parameters:
//      hist      - pointer to the input image histogram
//      hist_mask - pointer to the input image mask histogram
//      hist_dens - pointer to the result probability density histogram
//
//    Notes:
//F*/
OPENCVAPI  void  cvCalcProbDensity( CvHistogram* hist, CvHistogram* hist_mask,
                                 CvHistogram* hist_dens);

/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:     cvMaxRect
//    Purpose:  calculates maximum rectangle
//    Context:
//    Parameters:
//      rect1      - pointer to the first input rectangle
//      rect2      - pointer to the second input rectangle
//      max_rect   - pointer to the result maximum rectangle
//
//    Notes:
//F*/
OPENCVAPI  void  cvMaxRect( CvRect *rect1, CvRect *rect2, CvRect *max_rect );

/****************************************************************************************\
*                                  Distance Transform                                    *
\****************************************************************************************/

/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:     cvDistTransform
//    Purpose:  calculates distance transform of binary image 
//    Context:
//    Parameters:
//      src - source binary image
//      dst - output floating-point image, whose pixel values are distances from
//            the correspondend pixel in the source image to the nearest 0-pixel.
//      disType - type of metric used
//      maskSize - size of discrete aperture that approximates the metric; can be 3 or 5.
//      mask - array of 2 (for 3x3 mask) or 3 numbers (for 5x5 mask) that characterizes
//             metric if disType is CV_DIST_USER (user-defined metric)
//    Notes:
//F*/
#define CV_DIST_MASK_3   3
#define CV_DIST_MASK_5   5 

OPENCVAPI  void  cvDistTransform( const CvArr* src, CvArr* dst,
                                  CvDisType disType CV_DEFAULT(CV_DIST_L2),
                                  int maskSize CV_DEFAULT(3),
                                  const float* mask CV_DEFAULT(0));


/****************************************************************************************\
*                                      Thresholds                                        *
\****************************************************************************************/

/* Defines for Threshold functions */
typedef enum CvThreshType
{
    CV_THRESH_BINARY     = 0,  /* val = (val>thresh? MAX:0)      */
    CV_THRESH_BINARY_INV = 1,  /* val = (val>thresh? 0:MAX)      */
    CV_THRESH_TRUNC      = 2,  /* val = (val>thresh? thresh:val) */
    CV_THRESH_TOZERO     = 3,  /* val = (val>thresh? val:0)      */
    CV_THRESH_TOZERO_INV = 4   /* val = (val>thresh? 0:val)      */
} CvThreshType;

typedef enum CvAdaptiveThreshMethod
{
    CV_STDDEV_ADAPTIVE_THRESH  = 0   /*  method for the defining local adaptive threshold  */
}
CvAdaptiveThreshMethod;

/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:    cvThreshold
//    Purpose: Applies fixed threshold to the grayscale image
//    Context:
//    Parameters:
//      src     - source image
//      dst     - destination image (can be the same as the source image)
//      threshold - threshold value
//      maxValue  - the maximum value of the image pixel
//      type      - thresholding type, must be one of
//                  CV_THRESH_BINARY       - val =(val > Thresh ? maxValue : 0)
//                  CV_THRESH_BINARY_INV   - val =(val > Thresh ? 0   : maxValue)
//                  CV_THRESH_TOZERO       - val =(val > Thresh ? val : 0)
//                  CV_THRESH_TOZERO_INV   - val =(val > Thresh ? 0   : val)
//    Returns:
//    Notes:
//F*/
OPENCVAPI  void  cvThreshold( const CvArr*  src, CvArr*  dst,
                              double  thresh,  double  maxValue,
                              CvThreshType type );

/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:    cvAdaptiveThreshold
//    Purpose: Applies adaptive threshold to the grayscale image
//    Context:
//    Parameters:
//      src     - source image
//      dst     - destination image
//      maxValue  - the maximum value of the image pixel
//      method    - method for the adaptive threshold calculation
                   (now CV_STDDEF_ADAPTIVE_THRESH only)
//      type      - thresholding type, must be one of
//                  CV_THRESH_BINARY       - val =(val > Thresh ? MAX    : 0)
//                  CV_THRESH_BINARY_INV   - val =(val > Thresh ? 0      : MAX)
//                  CV_THRESH_TOZERO       - val =(val > Thresh ? val    : 0)
//                  CV_THRESH_TOZERO_INV   - val =(val > Thresh ? 0      : val)
//      parameters - pointer to the input parameters(for the
//                   CV_STDDEF_ADAPTIVE_THRESH method parameters[0] is size of
//                   the neighborhood thresholding,(one of the 1-(3x3),2-(5x5),or
//                   3-(7x7)), parameters[1] is the value of the minimum variance
//    Returns:
//    Notes:
//F*/
OPENCVAPI  void  cvAdaptiveThreshold( const CvArr* src, CvArr* dst, double maxValue,
                                      CvAdaptiveThreshMethod method, CvThreshType type,
                                      double* parameters );

/****************************************************************************************\
*                                     Flood fill                                         *
\****************************************************************************************/

/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name: cvFloodFill, cvFloodFill
//    Purpose: The function fills the connected domain, starting from seed pixel
//             while the pixel values in this domain are not far from each other.
//    Context:
//    Parameters:
//      img        - initial image(in the beginning)
//                   which is "repainted" during the function action,
//      seedPoint  - coordinates of the seed point inside image ROI,
//      newVal     - new value of repainted area pixels,
//      loDiff, upDiff - maximal lower and upper differences of the values of
//                   appurtenant to repainted area pixel and one of its
//                   neighbour,
//      comp       - pointer to connected component structure of the
//                   repainted area
//      connectivity - if it is 4, the function looks for 4-connected neighbors,
//                     otherwise it looks for 8-connected neighbors.
//    Notes:
//F*/
OPENCVAPI  void  cvFloodFill( CvArr* array, CvPoint seedPoint,
                              double newVal, double loDiff, double upDiff,
                              CvConnectedComp* comp, int connectivity CV_DEFAULT(4) );

/****************************************************************************************\
*                                     CAMSHIFT                                           *
\****************************************************************************************/

/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:    cvCamShift
//    Purpose:
//      Implements CAMSHIFT algorithm - determines object position, size and orientation
//      from the object histogram back project.
//    Context:
//    Parameters:
//      imgProb - back project of the object histogram
//      windowIn - initial search window
//      criteria - iterative search termination criteria 
//      out    - output parameter. Final position of search window and object area
//      box    - width and height (i.e. length) of the object, its center and orientation
//    Returns:
//      Number of iterations made
//F*/
OPENCVAPI int  cvCamShift( const CvArr* imgProb, CvRect  windowIn,
                           CvTermCriteria criteria, CvConnectedComp* out,
                           CvBox2D* box );

/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:    cvMeanShift
//    Purpose:
//      Implements MeanShift algorithm - determines object position
//      from the object histogram back project.
//    Context:
//    Parameters:
//      imgProb - back project of the object histogram
//      windowIn - initial search window
//      criteria - iterative search termination criteria 
//      out - output parameter. Final position of search window and object area
//    Returns:
//      Number of iterations made
//    Notes:
//F*/
OPENCVAPI int  cvMeanShift( const CvArr* imgProb, CvRect  windowIn,
                            CvTermCriteria criteria, CvConnectedComp* out );

/****************************************************************************************\
*                                  Feature detection                                     *
\****************************************************************************************/

/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:    cvCanny
//    Purpose: Canny edge detection
//    Context:
//    Parameters:
//      src - source byte-depth, single channel image,
//      dst - destination byte-depth, single channel image with edges,
//      apertureSize - size of Sobel operator aperture,
//      lowThreshold,
//      highThreshold - tresholds, applied in hysteresis thresholding
//    Returns:
//    Notes: image gradient magnitude has scale factor 2^(2*apertureSize-3)
//           so user must choose appropriate lowThreshold and highThreshold
//           i.e. if real gradient magnitude is 1, then 3x3 Sobel used in this function
//           will output 8 for apertureSize == 3.
//F*/
OPENCVAPI  void  cvCanny( const CvArr* src, CvArr* dst, double lowThreshold,
                          double highThreshold, int  apertureSize CV_DEFAULT(3) );

/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:     cvPreCornerDetect
//    Purpose:  Calculating constraint image for corner detection
//              Dx^2 * Dyy + Dxx * Dy^2 - 2 * Dx * Dy * Dxy
//    Context:
//    Parameters:
//      src - source image
//      dst - destination feature image
//      apertureSize - Sobel operator aperture size
//    Returns:
//F*/
OPENCVAPI void cvPreCornerDetect( const CvArr* src, CvArr* dst,
                                  int apertureSize CV_DEFAULT(3) );

/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:    cvCalcCornerEigenValsAndVecs
//    Purpose:  Calculates eigen values and vectors of 2x2
//              gradient matrix at every image pixel
//    Context:
//    Parameters:
//      src      - pointer to the source image
//      eigenvv  - destination image, containing two eigen values and
//                 components of two eigen vectors for each raster point
//               ( i.e., this image is 6 times wider than source image )
//      apertureSize - Sobel operator aperture size
//      blockSize  - size of block for summation(averaging block)
//    Returns:
//F*/
OPENCVAPI void  cvCornerEigenValsAndVecs( const CvArr* src, CvArr* eigenvv,
                                          int blockSize,
                                          int apertureSize CV_DEFAULT(3) );

/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:    cvCornerMinEigenVal
//    Purpose:  Calculates minimal eigenvalue for 2x2 gradient matrix at
//              every image pixel
//    Context:
//    Parameters:
//      src        - source image
//      eigenval   - minimal eigen value for each point of the source image
//      apertureSize - Sobel operator aperture size
//      blockSize  - size of block for summation(averaging block)
//    Returns:
//F*/
OPENCVAPI void  cvCornerMinEigenVal( const CvArr* src, CvArr* eigenval,
                                     int blockSize, int apertureSize CV_DEFAULT(3) );

/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:    cvFindCornerSubPix
//    Purpose:
//      Finds corners on the image with sub-pixel accuracy given
//      initial guesses for those corners.
//    Context:
//    Parameters:
//      src        - source image
//      corners    - initial coordinates of corners on input, adjusted coordinates
//                   on output
//      count      - number of corner points
//      win        - search window size for each corner.
//                   actually, for each corner(x,y), the window
//                  (x - win.width .. x + win.width,y - win.height .. y + win_height)
//                   is used.(window  moves with the point after every iteration)
//      zeroZone   - size of zero zone in the middle of the mask.
//      criteria   - This parameter specifies, how many times iterate and what precision
//                   is required.
//    Returns:
//      Nothing
//    Notes:
//      Size of destination ROI is not passed into the function, because
//      it assumes dst ROI size:
//      =(src_size.width - 2, src_size.height - 2) if both kernels are used
//      =(src_size.width - 2, src_size.height)     if horizontal kernel != 0 only.
//      =(src_size.width, src_size.height - 2)     if vertical kernel != 0 only.
F*/
OPENCVAPI  void  cvFindCornerSubPix( const CvArr* src,CvPoint2D32f*  corners,
                                     int count, CvSize win,CvSize zeroZone,
                                     CvTermCriteria  criteria );

/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:    cvGoodFeaturesToTrack
//    Purpose:
//      Finds strong corners on the image
//    Context:
//    Parameters:
//      image    - input image(IPL_DEPTH_8U,IPL_DEPTH_8S or IPL_DEPTH_32F,single channel)
//      eigImage - temporary image(IPL_DEPTH_32F,single channel),
//                 which will contain minimal eigen value for each point
//      tempImage- temporary image(IPL_DEPTH_32F,single channel),
//                 which is used in non-maxima suppression.
//      corners  - output corners
//      corner_count - number of output corners
//      quality_level - only those corners are selected, which minimal eigen value is
//                      non-less than maximum of minimal eigen values on the image,
//                      multiplied by quality_level. For example, quality_level = 0.1
//                      means that selected corners must be at least 1/10 as good as
//                      the best corner.
//      min_distance - The selected corners(after thresholding using quality_level)
//                     are rerified such that pair-wise distance between them is
//                     non-less than min_distance
//    Returns:
F*/
OPENCVAPI void  cvGoodFeaturesToTrack( const CvArr* image, CvArr* eigImage,
                                       CvArr* tempImage, CvPoint2D32f* corners,
                                       int* corner_count, double  quality_level,
                                       double  min_distance,
                                       const CvArr* mask CV_DEFAULT(0));

/****************************************************************************************\
*                                     Hough Transform                                    *
\****************************************************************************************/

/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:    cvHoughLines
//    Purpose:
//      Function detects lines on a binary raster image
//    Context:
//    Parameters:
//      image       - input image
//      rho         - resolution in rho(the minimum difference between two values)
//      theta       - resolution in theta(the minimum difference between two values)
//      threshold   - the pixels number which is enough to plot a line through
//      lines       - output parameters of a line
//                    i line is rho = lines[2*i], theta = lines[2*i + 1]
//      linesNumber - 2*linesNumber is the size of the lines buffer
//    Returns:
//    Notes:
//      the Standard Hough Transform is used in the function
//F*/
OPENCVAPI  int  cvHoughLines( IplImage* image, double rho, double theta, int threshold,
                              float* lines, int linesNumber );

/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:    cvHoughLinesP
//    Purpose:
//      Function detects lines on a binary raster image
//    Context:
//    Parameters:
//      image       - input image
//      rho         - resolution in rho(the minimum difference between two values)
//      theta       - resolution in theta(the minimum difference between two values)
//      threshold   - the pixels number which is enough to plot a line through
//      lineLength  - the minimum accepted length of lines
//      lineGap     - the maximum accepted gap in a line(in pixels)
//      lines       - output parameters of a line
//                      the i line starts in x1 = lines[4*i], y1 = lines[4*i + 1] and
//                      finishes in x2 = lines[4*i + 2], y2 = lines[4*i + 3]
//      linesNumber - 4*linesNumber is the size of lines buffer
//      linesToFind - the maximum number of lines to detect
//    Returns:
//      The number of found lines
//    Notes:
//    The Progressive Probabilistic Hough Transform is implemented in the function. It
//      searches for linesToFind number of lines, taking only those that contain more than
//      lineLength pixels and return. Effectively detects long lines on an image with
//      strong noise.
//F*/
OPENCVAPI  int  cvHoughLinesP( IplImage* image, double rho, double theta, int threshold,
                            int lineLength, int lineGap, int* lines, int linesNumber );

/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:    cvHoughLinesSDiv
//    Purpose:
//      Function detects lines on a binary raster image
//    Context:
//    Parameters:
//      image       - input image
//      rho         - rough resolution in rho(the minimum difference between two values)
//      srn         - the scale factor of a rough rho resolution to a high one
//      theta       - rough resolution in theta(the minimum difference between two values)
//      stn         - the scale factor of a rough theta resolution to a high one
//      threshold   - the pixels number which is enough to plot a line through
//      lines       - output parameters of a line
//                      i line is rho = lines[2*i], theta = lines[2*i + 1]
//      linesNumber - 2*linesNumber is the size of the lines buffer
//    Returns:
//      the number of lines found
//    Notes:
//    the Standard Hough Transform is used in the function
//F*/
OPENCVAPI  int  cvHoughLinesSDiv( IplImage* image, double rho, int srn,
                                  double theta, int stn, int threshold,
                                  float* lines, int lines_number );

/****************************************************************************************\
*                              Geometry functions                                        *
\****************************************************************************************/

OPENCVAPI  void  cvProject3D( CvPoint3D32f* points3D, int count,
                              CvPoint2D32f* points2D, int xIndx, int yIndx );

OPENCVAPI  void  cvFitLine3D( CvPoint3D32f* points, int count, CvDisType dist,
                              void *param, float reps, float aeps, float* line );

OPENCVAPI  void  cvFitLine2D( CvPoint2D32f* points, int count, CvDisType dist,
                              void *param, float reps, float aeps, float* line );


/****************************************************************************************\
*                              Optical Flow functions                                    *
\****************************************************************************************/
/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name: cvCalcOpticalFlowLK( Lucas & Kanade method )
//    Purpose: calculate Optical flow for 2 images using Lucas & Kanade algorithm
//    Context:
//    Parameters:
//            srcA,         // first image
//            srcB,         // second image
//            winSize,      // size of the averaging window used for grouping
//            velx,         //  horizontal
//            vely          //  vertical components of optical flow
//
//    Returns:
//
//    Notes:  1.Optical flow to be computed for every pixel in ROI
//            2.For calculating spatial derivatives we use 3x3 Sobel operator.
//            3.We use the following border mode.
//              The last row or column is replicated for the border
//            ( IPL_BORDER_REPLICATE in IPL ).
//
//F*/
OPENCVAPI  void  cvCalcOpticalFlowLK( const CvArr* srcA, const CvArr* srcB,
                                      CvSize winSize, CvArr* velx, CvArr* vely );

/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name: cvCalcOpticalFlowBM
//    Purpose: calculate Optical flow for 2 images using block matching algorithm
//    Context:
//    Parameters:
//            srcA,         // first image
//            srcB,         // second image
//            blockSize,    // size of basic blocks which are compared
//            shiftSize,    // coordinates increments.
//            maxRange,     // size of the scanned neighborhood.
//            usePrevious,  // use previous(input) velocity field.
//            velx,         //  horizontal
//            vely          //  vertical components of optical flow
//
//    Returns:
//    Notes:
//F*/
OPENCVAPI  void  cvCalcOpticalFlowBM( const CvArr* srcA, const CvArr* srcB,
                                      CvSize blockSize, CvSize shiftSize,
                                      CvSize maxRange, int usePrevious,
                                      CvArr* velx, CvArr* vely );

/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name: cvCalcOpticalFlowHS(Horn & Schunck method )
//    Purpose: calculate Optical flow for 2 images using Horn & Schunck algorithm
//    Context:
//    Parameters:
//            srcA,         // first image
//            srcB,         // second image
//            int usePrevious, // use previous(input) velocity field.
//            velx,         //  horizontal
//            vely          //  vertical components of optical flow
//            double lambda, // Lagrangian multiplier
//            criteria       // criteria of process termination
//
//    Returns:
//
//    Notes:  1.Optical flow to be computed for every pixel in ROI
//            2.For calculating spatial derivatives we use 3x3 Sobel operator.
//            3.We use the following border mode.
//              The first and last rows and columns are replicated for the border
//            ( IPL_BORDER_REPLICATE in IPL ).
//F*/
OPENCVAPI  void  cvCalcOpticalFlowHS( const CvArr* srcA, const CvArr* srcB,
                                      int usePrevious, CvArr* velx, CvArr* vely,
                                      double lambda, CvTermCriteria criteria );


/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name: cvCalcOpticalFlowPyrLK
//    Purpose:
//      It is Lucas & Kanade method, modified to use pyramids.
//      Also it does several iterations to get optical flow for
//      every point at every pyramid level.
//      Calculates optical flow between two images for certain set of points.
//    Context:
//    Parameters:
//            imgA     - first frame(time t)
//            imgB     - second frame(time t+1)
//            pyrA     - buffer for pyramid for the first frame.
//                       if the pointer is not NULL, the buffer must have size enough to
//                       store pyramid(from level 1 to level #<level>(see below))
//                      (total size of(imgSize.width+8)*imgSize.height/3
//                        bytes will be enough)).
//            pyrB     - similar to pyrA, but for the second frame.
//
//                       for both parameters above the following rules work:
//                           If image pointer is 0, the function allocates the buffer
//                           internally, calculates pyramid and releases the buffer after
//                           processing.
//                           Else(image should be large enough then) the function calculates
//                           pyramid and stores it in the buffer unless the
//                           CV_LKFLOW_PYR_A[B]_READY flag is set. After function call
//                           both pyramids are calculated and ready-flag for corresponding
//                           image can be set.
//
//            count    - number of feature points
//            winSize  - size of search window on each pyramid level
//            level    - maximal pyramid level number
//                        (if 0, pyramids are not used(single level),
//                          if 1, two levels are used etc.)
//
//            next parameters are arrays of <count> elements.
//            ------------------------------------------------------
//            featuresA - array of points, for which the flow needs to be found
//            featuresB - array of 2D points, containing calculated
//                       new positions of input features(in the second image).
//            status   - array, every element of which will be set to 1 if the flow for the
//                       corresponding feature has been found, 0 else.
//            error    - array of double numbers, containing difference between
//                       patches around the original and moved points
//                      (it is optional parameter, can be NULL).
//            ------------------------------------------------------
//            criteria   - specifies when to stop the iteration process of finding flow
//                         for each point on each pyramid level
//
//            flags      - miscellaneous flags:
//                            CV_LKFLOW_PYR_A_READY - pyramid for the first frame
//                                                    is precalculated before call
//                            CV_LKFLOW_PYR_B_READY - pyramid for the second frame
//                                                    is precalculated before call
//                            CV_LKFLOW_INITIAL_GUESSES - featuresB array holds initial
//                                                        guesses about new features'
//                                                        locations before function call.
//    Returns:
//    Notes:  For calculating spatial derivatives 3x3 Sharr operator is used.
//            The values of pixels beyond the image are determined using border
//            replication.
//F*/
#define  CV_LKFLOW_PYR_A_READY       1
#define  CV_LKFLOW_PYR_B_READY       2
#define  CV_LKFLOW_INITIAL_GUESSES   4

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


/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name: cvCalcAffineFlowPyrLK
//    Purpose:
//      It is Lucas & Kanade affine tracking method, modified to use pyramids.
//      Also it does several iterations to find flow for
//      every point at every pyramid level.
//      Calculates affine flow between two images for certain set of points.
//    Context:
//    Parameters:
//            imgA     - first frame(time t)
//            imgB     - second frame(time t+1)
//            pyrA     - buffer for pyramid for the first frame.
//                       if the pointer is not NULL, the buffer must have size enough to
//                       store pyramid(from level 1 to level #<level>(see below))
//                      (total size of(imgSize.width+8)*imgSize.height/3
//                        bytes will be enough)).
//            pyrB     - similar to pyrA, but for the second frame.
//
//                       for both parameters above the following rules work:
//                           If image pointer is 0, the function allocates the buffer
//                           internally, calculates pyramid and releases the buffer after
//                           processing.
//                           Else(image should be large enough then) the function calculates
//                           pyramid and stores it in the buffer unless the
//                           CV_LKFLOW_PYR_A[B]_READY flag is set. After function call
//                           both pyramids are calculated and ready-flag for corresponding
//                           image can be set.
//
//            count    - number of feature points
//            winSize  - size of search window on each pyramid level
//            level    - maximal pyramid level number
//                        (if 0, pyramids are not used(single level),
//                          if 1, two levels are used etc.)
//
//            next parameters are arrays of <count> elements.
//            ------------------------------------------------------
//            featuresA - array of points, for which the flow needs to be found
//            featuresB - array of 2D points, containing calculated
//                       new positions of input features(in the second image).
//            matrices - affine transformation matrices,
//            status   - array, every element of which will be set to 1 if the flow for the
//                       corresponding feature has been found, 0 else.
//            error    - array of double numbers, containing difference between
//                       patches around the original and moved points
//                      (it is optional parameter, can be NULL).
//            ------------------------------------------------------
//            criteria   - specifies when to stop the iteration process of finding flow
//                         for each point on each pyramid level
//
//            flags      - miscellaneous flags:
//                            CV_LKFLOW_PYR_A_READY - pyramid for the first frame
//                                                    is precalculated before call
//                            CV_LKFLOW_PYR_B_READY - pyramid for the second frame
//                                                    is precalculated before call
//                            CV_LKFLOW_INITIAL_GUESSES - featuresB array holds initial
//                                                        guesses about new features'
//                                                        locations before function call,
//                                                        matrices array contains guesses
//                                                        about local transformations in
//                                                        the features' neighborhoods.
//    Returns:
//    Notes:  For calculating spatial derivatives 3x3 Sharr operator is used.
//            The values of pixels beyond the image are determined using border
//            replication.
//F*/
OPENCVAPI  void  cvCalcAffineFlowPyrLK( const CvArr*  imgA, const CvArr*  imgB,
                                        CvArr*  pyrA, CvArr*  pyrB,
                                        CvPoint2D32f* featuresA,
                                        CvPoint2D32f* featuresB,
                                        float*  matrices, int  count,
                                        CvSize  winSize, int  level,
                                        char*  status, float* error,
                                        CvTermCriteria criteria, int flags );


/****************************************************************************************\
*                              Eigen objects functions                                   *
\****************************************************************************************/

#define CV_EIGOBJ_NO_CALLBACK     0
#define CV_EIGOBJ_INPUT_CALLBACK  1
#define CV_EIGOBJ_OUTPUT_CALLBACK 2
#define CV_EIGOBJ_BOTH_CALLBACK   3

typedef CvStatus (CV_CDECL * CvCallback)( int index, void* buffer, void* userData );

typedef union
{
    CvCallback callback;
    void* data;
}
CvInput;

/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name: cvCalcCovarMatrixEx
//    Purpose: The function calculates a covariance matrix for a group of input objects
//            (images, vectors, etc.).
//    Context:
//    Parameters:  nObjects    - number of source objects
//                 input       - pointer either to array of input objects
//                               or to read callback function(depending on ioFlags)
//                 ioFlags     - input/output flags(see Notes to
//                               cvCalcEigenObjects function)
//                 ioBufSize   - input/output buffer size
//                 userData    - pointer to the structure which contains all necessary
//                               data for the callback functions
//                 avg         - averaged object
//                 covarMatrix - covariance matrix(output parameter; must be allocated
//                               before call)
//
//    Notes:  See Notes to cvCalcEigenObjects function
//F*/
OPENCVAPI  void  cvCalcCovarMatrixEx( int nObjects, void* input, int ioFlags,
                                      int ioBufSize, uchar* buffer, void* userData,
                                      IplImage* avg, float* covarMatrix );

/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name: cvCalcEigenObjects
//    Purpose: The function calculates an orthonormal eigen basis and a mean(averaged)
//             object for a group of input objects(images, vectors, etc.).
//    Context:
//    Parameters: nObjects  - number of source objects
//                input     - pointer either to array of input objects
//                            or to read callback function(depending on ioFlags)
//                output    - pointer either to output eigen objects
//                            or to write callback function(depending on ioFlags)
//                ioFlags   - input/output flags(see Notes)
//                ioBufSize - input/output buffer size
//                userData  - pointer to the structure which contains all necessary
//                            data for the callback functions
//                calcLimit - determines the calculation finish conditions
//                avg       - averaged object(has the same size as ROI)
//                eigVals   - pointer to corresponding eigen values(array of <nObjects>
//                            elements in descending order)
//
//    Notes: 1. input/output data(that is, input objects and eigen ones) may either
//              be allocated in the RAM or be read from/written to the HDD(or any
//              other device) by read/write callback functions. It depends on the
//              value of ioFlags paramater, which may be the following:
//                  CV_EIGOBJ_NO_CALLBACK, or 0;
//                  CV_EIGOBJ_INPUT_CALLBACK;
//                  CV_EIGOBJ_OUTPUT_CALLBACK;
//                  CV_EIGOBJ_BOTH_CALLBACK, or
//                            CV_EIGOBJ_INPUT_CALLBACK | CV_EIGOBJ_OUTPUT_CALLBACK.
//              The callback functions as well as the user data structure must be
//              developed by the user.
//
//           2. If ioBufSize = 0, or it's too large, the function dermines buffer size
//              itself.
//
//           3. Depending on calcLimit parameter, calculations are finished either if
//              eigenfaces number comes up to certain value or the relation of the
//              current eigenvalue and the largest one comes down to certain value
//             (or any of the above conditions takes place). The calcLimit->type value
//              must be CV_TERMCRIT_NUMB, CV_TERMCRIT_EPS or
//              CV_TERMCRIT_NUMB | CV_TERMCRIT_EPS. The function returns the real
//              values calcLimit->maxIter and calcLimit->epsilon.
//
//           4. eigVals may be equal to NULL(if you don't need eigen values in further).
//
//F*/
OPENCVAPI  void  cvCalcEigenObjects( int nObjects, void* input, void* output,
                                    int ioFlags, int ioBufSize, void* userData,
                                    CvTermCriteria* calcLimit, IplImage* avg,
                                    float* eigVals );

/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name: cvCalcDecompCoeff
//    Purpose: The function calculates one decomposition coefficient of input object
//             using previously calculated eigen object and the mean(averaged) object
//    Context:
//    Parameters:  obj     - input object
//                 eigObj  - eigen object
//                 avg     - averaged object
//
//    Returns: decomposition coefficient value or large negative value(if error)
//
//    Notes:
//F*/
OPENCVAPI  double  cvCalcDecompCoeff( IplImage* obj, IplImage* eigObj, IplImage* avg );

/*F///////////////////////////////////////////////////////////////////////////////////////
//    Names: cvEigenDecomposite
//    Purpose: The function calculates all decomposition coefficients for input object
//             using previously calculated eigen objects basis and the mean(averaged)
//             object
//
//    Parameters:  obj         - input object
//                 nEigObjs    - number of eigen objects
//                 eigInput    - pointer either to array of pointers to eigen objects
//                               or to read callback function(depending on ioFlags)
//                 ioFlags     - input/output flags
//                 userData    - pointer to the structure which contains all necessary
//                               data for the callback function
//                 avg         - averaged object
//                 coeffs      - calculated coefficients(output data)
//
//    Notes:   see notes to cvCalcEigenObjects function
//F*/
OPENCVAPI  void  cvEigenDecomposite( IplImage* obj, int nEigObjs, void* eigInput,
                                    int ioFlags, void* userData, IplImage* avg,
                                    float* coeffs );

/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name: cvEigenProjection
//    Purpose: The function calculates object projection to the eigen sub-space(restores
//             an object) using previously calculated eigen objects basis, mean(averaged)
//             object and decomposition coefficients of the restored object
//    Context:
//    Parameters:  nEigObjs    - number of eigen objects
//                 eigInput    - pointer either to array of pointers to eigen objects
//                               or to read callback function(depending on ioFlags)
//                 ioFlags     - input/output flags
//                 userData    - pointer to the structure which contains all necessary
//                               data for the callback function
//                 coeffs      - array of decomposition coefficients
//                 avg         - averaged object
//                 proj        - object projection(output data)
//
//    Notes:   see notes for cvCalcEigenObjects function
//F*/
OPENCVAPI  void  cvEigenProjection( void* eigInput, int nEigObjs, int ioFlags,
                                   void* userData, float* coeffs, IplImage* avg,
                                   IplImage* proj );

/****************************************************************************************\
*                              HMM(Hidden Markov Models)                                *
\****************************************************************************************/

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

//*F//////////////////////////////////////////////////////////////////////////////////////
//    Name: cvCreate2DHMM
//    Purpose: The function allocates memory for 2-dimensional embedded HMM model
//             after you finish work with created HMM you must free memory
//             by calling cvRelease2DHMM function
//    Context:
//    Parameters: stateNumber - array of hmm sizes(size of array == state_number[0]+1 )
//                numMix - number of gaussian mixtures in low-level HMM states
//                          size of array is defined by previous array values
//                obsSize - length of observation vectors
//
//    Returns:
//      Created 2D HMM.
//    Notes: stateNumber[0] - number of states in external HMM.
//           stateNumber[i] - number of states in embedded HMM
//
//           example for face recognition: state_number = { 5 3 6 6 6 3 },
//                                         length of num_mix array = 3+6+6+6+3 = 24
//
//F*/
OPENCVAPI  CvEHMM*  cvCreate2DHMM( int* stateNumber, int* numMix, int obsSize );


//*F///////////////////////////////////////////////////////////////////////////////////////
//    Name: cvRelease2DHMM
//    Purpose: The function free memory used by CvEHMM structure
//    Context:
//    Parameters: hmm - address of pointer to CvEHMM structure
//    Returns:
//    Notes:  function set *hmm = 0
//F*/
OPENCVAPI  void  cvRelease2DHMM( CvEHMM** hmm );


//*F///////////////////////////////////////////////////////////////////////////////////////
//    Name: cvCreateObsInfo
//    Purpose: The function allocates memory for CvImgObsInfo structure
//             after you finish working with allocated structure - destroy it
//             by  cvReleaseObsInfo
//
//    Context:
//    Parameters: numObs  - number of horizontal and vertical observations.
//                          Total number of allocated observation vectors
//                          will be   num_obs.width*num_obs.height
//                obsSize - length of observation vector
//
//    Returns:  Parameter obs_info is filled.
//
//    Notes: If you extract observations from an image, use CV_COUNT_OBS macro
//           to compute "numObs" parameter:
//
//           CV_COUNT_OBS( &roi, &obs, &delta, &numObs),
//
//                          where CvSize roi   - image ROI
//                                CvSize obs   - size of image block (a single observation)
//                                CvSize delta - horizontal and vertical shift
//                                           (i.e. because observation blocks overlap if
//                                            delta.width < obs.width or
//                                            delta.height < obs.height )
//                                CvSize numObs - output parameter to be computed
//
//F*/
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

OPENCVAPI  CvImgObsInfo*  cvCreateObsInfo( CvSize numObs, int obsSize );

//*F///////////////////////////////////////////////////////////////////////////////////////
//    Name: cvReleaseObsInfo
//    Purpose: The function free memory used by CvImgObsInfo structure
//
//    Context:
//    Parameters: obs_info - address of pointer to CvImgObsInfo structure
//    Returns:
//    Notes:  function sets *obs_info = 0
//F*/
OPENCVAPI  void  cvReleaseObsInfo( CvImgObsInfo** obs_info );


//*F//////////////////////////////////////////////////////////////////////////////////////
//    Name: cvImgToObs_DCT
//    Purpose: The function takes as input an image and returns the sequnce of observations
//             to be used with an embedded HMM; Each observation is top-left block of DCT
//             coefficient matrix.
//    Context:
//    Parameters: img     - pointer to the original image
//                obs     - pointer to resultant observation vectors
//                dctSize - size of the block for which DCT is calculated
//                obsSize - size of top-left block of DCT coeffs matrix, which is treated
//                          as observation. Each observation vector consists of
//                          obsSize.width * obsSize.height floats.
//                          The following conditions should be satisfied:
//                          0 < objSize.width <= dctSize.width,
//                          0 < objSize.height <= dctSize.height.
//                delta   - dctBlocks are overlapped and this parameter specifies horizontal
//                          and vertical shift.
//    Returns:
//
//    Notes:
//      The algorithm is following:
//          1. First, number of observation vectors per row and per column are calculated:
//
//             Nx = floor((roi.width - dctSize.width + delta.width)/delta.width);
//             Ny = floor((roi.height - dctSize.height + delta.height)/delta.height);
//
//             So, total number of observation vectors is Nx*Ny, and total size of
//             array obs must be >= Nx*Ny*obsSize.width*obsSize.height*sizeof(float).
//          2. Observation vectors are calculated in the following loop
//             ( actual implementation may be different ), where
//               I[x1:x2,y1:y2] means block of pixels from source image with
//               x1 <= x < x2, y1 <= y < y2,
//               D[x1:x2,y1:y2] means sub matrix of DCT matrix D.
//               O[x,y] means observation vector that corresponds to position
//              (x*delta.width,y*delta.height) in the source image
//             ( all indices are counted from 0 ).
//
//               for( y = 0; y < Ny; y++ )
//               {
//                   for( x = 0; x < Nx; x++ )
//                   {
//                       D = DCT(I[x*delta.width : x*delta.width + dctSize.width,
//                                  y*delta.height : y*delta.height + dctSize.height]);
//                       O[x,y] = D[0:obsSize.width, 0:obsSize.height];
//                   }
//               }
//F*/
OPENCVAPI  void  cvImgToObs_DCT( const CvArr* array, float* obs, CvSize dctSize,
                                 CvSize obsSize, CvSize delta );


//*F//////////////////////////////////////////////////////////////////////////////////////
//    Name: cvUniformImgSegm
//    Purpose: The uniformly segments all observation vectors extracted from image
//    Context:
//    Parameters: obs_info - observations structure
//                hmm      - 2D embedded HMM structure
//
//    Returns:
//    Notes:
//F*/
OPENCVAPI  void  cvUniformImgSegm( CvImgObsInfo* obs_info, CvEHMM* ehmm );

/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name: cvInitMixSegm
//    Purpose: The function implements the mixture segmentation of the states
//             of the embedded HMM
//
//    Context: used with the Viterbi training of the embedded HMM
//             Function uses K-Means algorithm for clustering.
//
//    Parameters:  obs_info_array - array of pointers to image observations
//                 num_img - length of above array
//                 hmm - pointer to HMM structure
//
//    Returns:
//    Notes:
//F*/
OPENCVAPI  void  cvInitMixSegm( CvImgObsInfo** obs_info_array,
                               int num_img, CvEHMM* hmm );

/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name: cvEstimHMMStateParams
//    Purpose: function calculates means, variances, weights of every Gaussian mixture
//             of every low-level state of embedded HMM
//    Context:
//    Parameters:  obs_info_array - array of pointers to observation structures
//                 num_img  - length of above array
//                 hmm      - hmm structure
//
//    Returns:
//
//    Notes:
//F*/
OPENCVAPI  void  cvEstimateHMMStateParams( CvImgObsInfo** obs_info_array,
                                        int num_img, CvEHMM* hmm );

/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name: cvEstimateTransProb
//    Purpose: function computes transition probability matrices of embedded HMM
//             given observations segmentation
//
//    Context:
//    Parameters:  obs_info_array - array of pointers to observation structures
//                 num_img  - length of above array
//                 hmm      - hmm structure
//
//    Returns:
//
//    Notes:
//F*/
OPENCVAPI  void  cvEstimateTransProb( CvImgObsInfo** obs_info_array,
                                   int num_img, CvEHMM* hmm );

/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name: cvEstimateTransProb
//    Purpose: function computes probabilities of appearing observations at any state
//           ( i.e. compute P(obs|state) for every pair(obs,state) )
//    Context:
//    Parameters:  obs_info - observations structure
//                 hmm      - hmm structure
//
//    Returns:
//    Notes:
//F*/
OPENCVAPI  void  cvEstimateObsProb( CvImgObsInfo* obs_info,
                                   CvEHMM* hmm );

/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name: cvEViterbi  (Embedded Viterbi)
//    Purpose: The function calculates the embedded Viterbi algorithm
//    Context:
//    Parameters:
//                obs_info - observations structure
//                hmm      - hmm structure
//
//    Returns: the Embedded Viterbi logarithmic probability.
//             Observations, stored in of obs_info structure are segmented
//           ( but new segmentation does not affect mixture segmentation or
//               states parameters )
//    Notes:
//F*/
OPENCVAPI  float  cvEViterbi( CvImgObsInfo* obs_info, CvEHMM* hmm );


/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name: cvMixSegmL2
//    Purpose: function clusters observation vectors from several images
//             given observations segmentation.
//             Euclidean distance used for clustering vectors.
//             Centers of clusters are given means of every mixture
//
//    Context: in HMM face recognition used after Viterbi segmentation
//    Parameters:  obs_info_array - array of pointers to observation structures
//                 num_img  - length of above array
//                 hmm      - hmm structure
//
//    Returns:
//    Notes:
//F*/
OPENCVAPI  void  cvMixSegmL2( CvImgObsInfo** obs_info_array,
                             int num_img, CvEHMM* hmm );

/* end of HMM functions*/

/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name: KMeans
//    Purpose: The function implements the K means algorithm, to cluster an array of sample
//             vectors in a number of classes specified by numClusters
//    Context:
//    Parameters:  numClunster - the number of clusters
//                 samples - the array of sample vectors
//                 numSamples - the number of samples
//                 VecSize - the size of each sample vector
//                 termcrit.eps - the convergence error; the iterations to find the best cluster
//                         centers will stop, when the value of the cost function at consecutive
//                         iterations falls below this threshold
//                 cluster - characteristic array. for every input vector indicates cluster
//
//    Returns: error code
//
//    Notes:
//F*/
OPENCVAPI  void  cvKMeans( int num_clusters, CvVect32f* samples, int num_samples,
                          int vec_size, CvTermCriteria termcrit, int* cluster  );


/****************************************************************************************\
*                               Undistortion functions                                  *
\****************************************************************************************/

/*F//////////////////////////////////////////////////////////////////////////////////////
//    Name: cvUnDistortOnce
//    Purpose: The function corrects radial and tangential image distortion using known
//             matrix of the camera intrinsic parameters and distortion coefficients
//    Context:
//    Parameters:  srcImage    - source(distorted) image
//                 dstImage    - output(undistorted) image
//                 intrMatrix  - matrix of the camera intrinsic parameters
//                 distCoeffs  - vector of the distortion coefficients(k1, k2, p1 and p2)
//                 interpolate - interpolation flag (turned on by default)
//F*/
OPENCVAPI  void  cvUnDistortOnce( const CvArr* srcImage, CvArr* dstImage,
                                  const float* intrMatrix,
                                  const float* distCoeffs,
                                  int interpolate CV_DEFAULT(1) );

/*F//////////////////////////////////////////////////////////////////////////////////////
//    Name: cvUnDistortInit
//    Purpose: The function calculates arrays of distorted points indices and
//             interpolation coefficients for cvUnDistort function using known
//             matrix of the camera intrinsic parameters and distortion coefficients
//    Context:
//    Parameters:  srcImage    - source(distorted) image
//                 intrMatrix  - matrix of the camera intrinsic parameters
//                 distCoeffs  - vector of the distortion coefficients(k1, k2, p1 and p2)
//                 undistMap   - distortion data array (CV_32SC1)
//                 interpolate - interpolation flag (turned on by default)
//F*/
OPENCVAPI  void  cvUnDistortInit( const CvArr* srcImage, CvArr* undistMap,
                                  const float* intrMatrix,
                                  const float* distCoeffs,
                                  int interpolate CV_DEFAULT(1) );

/*F//////////////////////////////////////////////////////////////////////////////////////
//    Name: cvUnDistort
//    Purpose: The function corrects radial and tangential distortion in the frame
//             using previousely calculated arrays of distorted points indices and
//             undistortion coefficients. The function can be also used
//             for arbitrary pre-calculated geometrical transformation.
//             The function processes as following:
//                 for (x,y) in dstImage:
//                    dstImage(x,y) = srcImage[ undistMap(x,y) ].
//    Context:
//    Parameters:  srcImage  - source(distorted) image (width x height x 8uC1/8uC3)
//                 dstImage  - output(undistorted) image (width x height x 8uC1/8uC3)
//                 undistMap - distortion data array:
//                                 (width x height x 32sC3) or
//                                 (width*3 x height x 32sC1) if interpolation is enabled;
//                                 (width x height x 32sC1) if interpolation is disabled;
//                             This array can be calculated from camera
//                             lens distortion parameters using cvUnDistortInit or
//                             from arbitrary floating-point map using cvConvertMap
//                 interpolate - interpolation flag (turned on by default)
//F*/
OPENCVAPI  void  cvUnDistort( const CvArr* srcImage, CvArr* dstImage,
                              const CvArr* undistMap, int interpolate CV_DEFAULT(1));
#define cvRemap cvUnDistort


/*F//////////////////////////////////////////////////////////////////////////////////////
//    Name: cvConvertMap
//    Purpose: The function converts floating-point pixel coordinate map to
//             faster fixed-point map, used within cvUnDistort (cvRemap)
//    Context:
//    Parameters:  srcImage  - sample image which header parameters (step and datatype)
//                             are used to prepare map.
//                 flUndistMap - source map: (width x height x 32fC2) or
//                                           (width*2 x height x 32fC1).
//                               each pair are pixel coordinates (x,y) in a source image.
//                 undistMap - resultant map: (width x height x 32sC3) or
//                                 (width*3 x height x 32sC1) if interpolation is enabled;
//                                 (width x height x 32sC1) if interpolation is disabled;
//                 interpolate - interpolation flag (turned on by default)
//F*/
OPENCVAPI  void  cvConvertMap( const CvArr* srcImage, const CvArr* flUndistMap,
                               CvArr* undistMap, int iterpolate CV_DEFAULT(1) );

/****************************************************************************************\
*                               Calibration functions                                   *
\****************************************************************************************/
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


OPENCVAPI  void  cvFindExtrinsicCameraParams( int           numPoints,
                                             CvSize        imageSize,
                                             CvPoint2D32f* imagePoints32f,
                                             CvPoint3D32f* objectPoints32f,
                                             CvVect32f     focalLength32f,
                                             CvPoint2D32f  principalPoint32f,
                                             CvVect32f     distortion32f,
                                             CvVect32f     rotVect32f,
                                             CvVect32f     transVect32f);


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
typedef enum CvRodriguesType
{
    CV_RODRIGUES_M2V = 0,
    CV_RODRIGUES_V2M = 1
}
CvRodriguesType;

OPENCVAPI  void  cvRodrigues( CvMat* rotMatrix, CvMat* rotVector,
                              CvMat* jacobian, CvRodriguesType convType);

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

OPENCVAPI void cvProjectPointsSimple(  int numPoints,
                                    CvPoint3D64d * objectPoints,
                                    CvVect64d rotMatr,
                                    CvVect64d transVect,
                                    CvMatr64d cameraMatrix,
                                    CvVect64d distortion,
                                    CvPoint2D64d* imagePoints);
                                    
/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:    cvFindChessBoardCornerGuesses
//    Purpose:
//      Function finds first approximation of internal corners on the chess board.
//    Context:
//    Parameters:
//      img      - source halftone image
//      thresh   - temporary image where the thresholded source image will be stored.
//      etalon_size - number of corners per each column and each row
//      corners  - pointer to found points array
//                 (must have at least etalon_size.width*etalon.height element).
//      corner_count - number of found corners
//    Returns:
//
//F*/
OPENCVAPI  int  cvFindChessBoardCornerGuesses( const CvArr* array, CvArr* thresh,
                                               CvMemStorage* storage, CvSize etalon_size,
                                               CvPoint2D32f* corners,
                                               int *corner_count CV_DEFAULT(0));


/****************************************************************************************\
*                                      POSIT(POse from ITeration)                       *
\****************************************************************************************/

/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:       cvCreatePOSITObject
//    Purpose:    Allocate and Initialize CvPOSITObject structure
//                before process cvPOSIT
//    Context:
//    Parameters:
//                  points - pointer to source object points given in
//                           object related coordinate system
//                  numPoints - number of object points
//                  ppObject - address of pointer to CvPOSITObject(returned)
//    Returns:
//    Notes:
//F*/
typedef struct CvPOSITObject CvPOSITObject;

OPENCVAPI  CvPOSITObject*  cvCreatePOSITObject( CvPoint3D32f* points, int numPoints );


/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:       cvPOSIT
//    Purpose:    performs POSIT algorithm
//
//    Context:
//    Parameters:
//                  pObject - pointer to CvPOSITObject filled with prev. function
//                  imagePoints - pointer to source object image points given in
//                                camera related coordinate system
//                  focalLength - focal length of camera
//                  criteria - stop criteria.
//                  rotation - rotation matrix
//                  translation - translation vector(from camera to
//                                first point of object )
//    Returns:
//    Notes:
//F*/
OPENCVAPI  void  cvPOSIT(  CvPOSITObject* pObject, CvPoint2D32f* imagePoints,
                           double focalLength, CvTermCriteria criteria,
                           CvMatr32f rotation, CvVect32f translation);

/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:       cvReleasePOSITObject
//    Purpose:    free CvPOSITObject structure
//    Context:
//    Parameters:
//      ppObject - address of pointer to CvPOSITObject
//    Returns:
//    Notes:
//F*/
OPENCVAPI  void  cvReleasePOSITObject( CvPOSITObject**  ppObject );


/****************************************************************************************\
*                                      ViewMorphing                                      *
\****************************************************************************************/

OPENCVAPI  void  cvFindFundamentalMatrix( int*       points1,
                                        int*       points2,
                                        int        numpoints,
                                        int        method,
                                        float* matrix );

OPENCVAPI  void  cvMakeScanlines( const CvMatrix3* matrix,
                                CvSize     imgSize,
                                int*       scanlines_1,
                                int*       scanlines_2,
                                int*       lens_1,
                                int*       lens_2,
                                int*       numlines);

OPENCVAPI  void  cvPreWarpImage( int       numLines,
                               IplImage* img,
                               uchar*    dst,
                               int*      dst_nums,
                               int*      scanlines);

OPENCVAPI  void  cvFindRuns( int    numLines,
                           uchar* prewarp_1,
                           uchar* prewarp_2,
                           int*   line_lens_1,
                           int*   line_lens_2,
                           int*   runs_1,
                           int*   runs_2,
                           int*   num_runs_1,
                           int*   num_runs_2);

OPENCVAPI  void  cvDynamicCorrespondMulti( int  lines,
                                         int* first,
                                         int* first_runs,
                                         int* second,
                                         int* second_runs,
                                         int* first_corr,
                                         int* second_corr);


OPENCVAPI  void  cvMakeAlphaScanlines( int*  scanlines_1,
                                     int*  scanlines_2,
                                     int*  scanlines_a,
                                     int*  lens,
                                     int   numlines,
                                     float alpha);

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

OPENCVAPI  void  cvDeleteMoire( IplImage*  img);

OPENCVAPI  void  cvPostWarpImage( int       numLines,
                                uchar*    src,
                                int*      src_nums,
                                IplImage* img,
                                int*      scanlines);


/****************************************************************************************\
*                                      Matrix Functions                                  *
\****************************************************************************************/


/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:       cvAdd
//    Purpose:    Adds one array to another
//    Context:
//    Parameters:
//      srcA - first source array
//      srcB - second source array
//      dst  - destination array: dst = srcA + srcB
//      mask - optional mask
//    Returns:
//F*/
OPENCVAPI  void  cvAdd( const CvArr* srcA, const CvArr* srcB, CvArr* dst,
                        const CvArr* mask CV_DEFAULT(0));


/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:       cvAddS
//    Purpose:    Adds array to scalar
//    Context:
//    Parameters:
//      src  - source array
//      value - added scalar
//      dst  - destination array: dst = src + value
//      mask - optional mask
//    Returns:
//F*/
OPENCVAPI  void  cvAddS( const CvArr* src, CvScalar value, CvArr* dst,
                         const CvArr* mask CV_DEFAULT(0));


/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:       cvSub
//    Purpose:    Subtracts one array from another
//    Context:
//    Parameters:
//      srcA - first source array
//      srcB - second source array 
//      dst  - destination array: dst = srcA - srcB
//    Returns:
//F*/
OPENCVAPI  void  cvSub( const CvArr* srcA, const CvArr* srcB, CvArr* dst,
                        const CvArr* mask CV_DEFAULT(0));


/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:       cvSubS
//    Purpose:    Subtracts a scalar value from array
//    Context:
//    Parameters:
//      src - source array
//      value - subtracted scalar
//      dst  - destination array: dst = src - value
//      mask - optional mask
//    Returns:
//F*/
CV_INLINE  void  cvSubS( const CvArr* src, CvScalar value, CvArr* dst,
                         const CvArr* mask CV_DEFAULT(0));
CV_INLINE  void  cvSubS( const CvArr* src, CvScalar value, CvArr* dst,
                         const CvArr* mask )
{
    cvAddS( src, cvScalar( -value.val[0], -value.val[1], -value.val[2], -value.val[3]),
            dst, mask );
}


/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:       cvSubRS
//    Purpose:    Subtracts a scalar value from array
//    Context:
//    Parameters:
//      src - source array
//      value - scalar to subtract from
//      dst  - destination array: dst = value - src
//      mask - optional mask
//    Returns:
//F*/
OPENCVAPI  void  cvSubRS( const CvArr* src, CvScalar value, CvArr* dst,
                          const CvArr* mask CV_DEFAULT(0));


/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:       cvMul
//    Purpose:    Multiplies two arrays
//    Context:
//    Parameters:
//      srcA - first source array
//      srcB - second source array
//      dst  - destination array: dst(x,y) = srcA(x,y) * srcB(x,y)
//    Returns:
//F*/
OPENCVAPI  void  cvMul( const CvArr* srcA, const CvArr* srcB, CvArr* dst);

/* dst(idx) = lower(idx) <= src(idx) < upper(idx) */
OPENCVAPI void cvInRange( const CvArr* src, const CvArr* lower,
                          const CvArr* upper, CvArr* dst );

/* dst(idx) = lower <= src(idx) < upper */
OPENCVAPI void cvInRangeS( const CvArr* src, CvScalar lower,
                           CvScalar upper, CvArr* dst );

/****************************************************************************************\
*                                      Logic Operations                                  *
\****************************************************************************************/

/* dst(idx) = src1(idx) & src2(idx) */
OPENCVAPI void cvAnd( const CvArr* src1, const CvArr* src2,
                      CvArr* dst, const CvArr* mask CV_DEFAULT(0));

/* dst(idx) = src(idx) & value */
OPENCVAPI void cvAndS( const CvArr* src, CvScalar value,
                       CvArr* dst, const CvArr* mask CV_DEFAULT(0));

/* dst(idx) = src1(idx) | src2(idx) */
OPENCVAPI void cvOr( const CvArr* src1, const CvArr* src2,
                     CvArr* dst, const CvArr* mask CV_DEFAULT(0));

/* dst(idx) = src(idx) | value */
OPENCVAPI void cvOrS( const CvArr* src, CvScalar value,
                      CvArr* dst, const CvArr* mask CV_DEFAULT(0));

/* dst(idx) = src1(idx) ^ src2(idx) */
OPENCVAPI void cvXor( const CvArr* src1, const CvArr* src2,
                      CvArr* dst, const CvArr* mask CV_DEFAULT(0));

/* dst(idx) = src(idx) ^ value */
OPENCVAPI void cvXorS( const CvArr* src, CvScalar value,
                       CvArr* dst, const CvArr* mask CV_DEFAULT(0));


/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:       cvScaleAdd
//    Purpose:    Multiplies all array elements by a scalar value and
//                adds another scalar to the scaled array
//    Context:
//    Parameters:
//      src - source array
//      scale - scale factor
//      delta - shift value
//      dst - destination array
//    Returns:
//    Notes:
//F*/
OPENCVAPI  void  cvScaleAdd( const CvArr* src1, CvScalar scale,
                             const CvArr* src2, CvArr* dst );
#define cvMulAddS cvScaleAdd

/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:       cvDotProduct
//    Purpose:    Evaluates dot product of two vectors
//    Context:
//    Parameters:
//      srcA - first source array
//      srcB - second source array
//
//    Returns:
//      Dot product of srcA and srcB:  sum(srcA(i,j)*srcB(i,j))
//                                     i,j
//F*/
OPENCVAPI  double  cvDotProduct( const CvArr* srcA, const CvArr* srcB );


/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:       cvCrossProduct
//    Purpose:    Evaluates cross product of two 3d vectors
//    Context:
//    Parameters: srcA - first source vector
//                srcB - second source vector
//                dst  - destination vector
//    Returns:
//
//F*/
OPENCVAPI  void  cvCrossProduct( const CvArr* srcA, const CvArr* srcB, CvArr* dst );


/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:       cvMatMulAdd
//    Purpose:    Evaluates product of two matrices and
//                adds the third matrix to the product
//    Context:
//    Parameters:
//      srcA - first source matrix
//      srcB - second source matrix
//      srcC - added matrix
//      dst  - destination matrix
//    Returns:
//
//F*/
OPENCVAPI  void  cvMatMulAdd( const CvArr* srcA, const CvArr* srcB,
                              const CvArr* srcC, CvArr* dst );
#define cvMatMul( srcA, srcB, dst )  cvMatMulAdd( (srcA), (srcB), 0, (dst))


/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:       cvMatMulAddS
//    Purpose:    Performs matrix transformation for every vector of the source array
//    Context:
//    Parameters:
//      srcr - source array
//      dst  - destination array
//      transform - transformation matrix
//      shiftvec - optional shift (may be encoded in the matrix as well)
//    Returns:
//
//F*/
OPENCVAPI  void  cvMatMulAddS( const CvArr* src, CvArr* dst,
                               const CvArr* transform,
                               const CvArr* shiftvec CV_DEFAULT(0));


/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:       cvMulTransposed
//    Purpose:    Evaluates product of matix by itself transposed
//    Context:
//    Parameters:
//      srcarr - the source matrix
//      dstarr - the destination matrix
//      order - determines the order of multiplication
//              if order = 0, function evaluates A*At
//              if order = 1, function evaluates At*A
//    Returns:
//    Notes:
//F*/
OPENCVAPI void cvMulTransposed( const CvArr* srcarr,
                                CvArr* dstarr, int order );

/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:       cvTranspose
//    Purpose:    Transposes matrix
//    Context:
//    Parameters:
//      src - source matrix
//      dst - destination matrix
//    Returns:
//    Notes:
//      square matrices can be transposed in-place.
//F*/
OPENCVAPI  void  cvTranspose( const CvArr* src, CvArr* dst );
#define cvT cvTranspose


/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:       cvFlip
//    Purpose:    Mirrors the matrix around vertical or horizontal axis
//    Context:
//    Parameters:
//      src - source matrix
//      dst - destination matrix
//      flipAxis - 0: horizontal axis
//                 1: vertical axis
//                -1: both axis
//    Returns:
//    Notes:
//F*/
OPENCVAPI  void  cvFlip( const CvArr* src, CvArr* dst, int flip_mode );
#define cvMirror cvFlip

/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:       cvInvert
//    Purpose:    Inverts Matrix using LU decomposition
//    Context:
//    Parameters:
//      src - source matrix
//      dst - destination matrix
//    Returns:
//      1 if the matrix inverted and 0 if it is a singular (or very close to it)
//    Notes:
//F*/
OPENCVAPI  int  cvInvert( const CvArr* src, CvArr* dst );
#define cvInv cvInvert

/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:       cvSolve
//    Purpose:    Solves linear system Ax = b using LU decomposition
//    Context:
//    Parameters:
//      A - the matrix
//      b - the "right side" of the system
//      x - destination array (solution of the system)
//    Returns:
//      1 if the system is solved and 0 if the matrix is a singular (or very close to it)
//    Notes:
//F*/
OPENCVAPI  int  cvSolve( const CvArr* A, const CvArr* b, CvArr* x );

/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:       cvDet
//    Purpose:    Calculates determinant of the matrix
//    Context:
//    Parameters:
//      mat - source matrix
//    Returns:
//      Matrix determinant
//    Notes:
//F*/
OPENCVAPI  double cvDet( const CvArr* mat );

/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:       cvTrace
//    Purpose:    Calculates trace of the matrix (sum of elements on the main diagonal) 
//    Context:
//    Parameters:
//      mat - source matrix
//    Returns:
//      Matrix determinant
//    Notes:
//F*/
OPENCVAPI  CvScalar cvTrace( const CvArr* mat );

/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:       cvSVD
//    Purpose:    Calculates Singular Value Decomposition for the input matrix: 
//       A = U W V',   U & V are orthogonal, W is diagonal matrix that can be
//                     either real matrix (the same size as A) or a vector of
//                     size min(A->rows,A->cols).
//       U & V are optional,
//       flags:  0th bit, reset to 0, means that A is copyied before processing,
//                        otherwise it is modified during the processing, which is
//                        faster.
//               1st bit, reset to 0, means that U is returned normal, otherwise it
//                        is returned transposed, which is faster.
//               2nd bit, reset to 0, means that V is returned normal, otherwise it
//                        is returned transposed, which is faster.
//F*/
#define CV_SVD_MODIFY_A   1
#define CV_SVD_U_T        2
#define CV_SVD_V_T        4

OPENCVAPI  void   cvSVD( CvArr* A, CvArr* W CV_DEFAULT(0),
                         CvArr* U CV_DEFAULT(0),
                         CvArr* V CV_DEFAULT(0),
                         int flag CV_DEFAULT(0));


/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:       cvPseudoInv
//    Purpose:    Calculates inverse or pseudo-inverse matrix for the input matrix.
//       if A is (m x n) matrix, B will be (n x m) matrix, such that
//       AB = I(m x m), BA = I(n x n).
//
//       flags:  0th bit, reset to 0, means that A is copyied before processing,
//                        otherwise it is modified during the processing, which is
//                        faster.
//    Return value:
//       The function returns condition number or DBL_MAX if the matrix is signular.
//F*/
OPENCVAPI  double  cvPseudoInverse( CvArr* A, CvArr* B,
                                    int flags CV_DEFAULT(0));
#define cvPseudoInv cvPseudoInverse

/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:       cvEigenVV
//    Purpose:    Finds eigenvalues & eigenvectors of a symmetric matrix:
//    Context:
//    Parameters:
//      src - source symmetric matrix,
//      evects - matrix of its eigenvectors
//               (i-th row is an i-th eigenvector),
//      evals - vector of its eigenvalues
//              (i-th element is an i-th eigenvalue),
//      eps - accuracy of diagonalization.
//    Returns:
//    Notes:
//F*/
OPENCVAPI  void  cvEigenVV( CvArr* src, CvArr* evects, CvArr* evals, double eps );


/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:       cvSetZero
//    Purpose:    Clears all the matrix elements (sets them to 0)
//    Context:
//    Parameters: mat  - matrix
//    Returns:
//    Notes:
//F*/
OPENCVAPI  void  cvSetZero( CvArr* mat );
#define cvZero  cvSetZero

/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:       cvSetIdentity
//    Purpose:    Fills identity matrix
//    Context:
//    Parameters:
//      mat - matrix
//    Returns:
//    Notes:
//F*/
OPENCVAPI  void  cvSetIdentity( CvArr* mat );


/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:       cvMahalanobis
//    Purpose:    Calculates Mahalanobis(weighted) distance.
//    Context:
//    Parameters:
//      srcA - first source vector
//      srcB - second source vector
//      matr - covariance matrix
//    Returns:
//      Mahalanobis distance
//    Notes:
//F*/
OPENCVAPI  double  cvMahalanobis( const CvArr* srcA, const CvArr* srcB, CvArr* mat );
#define cvMahalonobis  cvMahalanobis

/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:       cvPerspectiveTransform
//    Purpose:    Applies perspective transform to the array of vectors
//    Context:
//    Parameters: mat - matrix
//                src - source array
//                dst - destination array
//    Returns:
//    Notes:
//F*/

OPENCVAPI  void  cvPerspectiveTransform( const CvArr* src, CvArr* dst, const CvArr* mat );

/****************************************************************************************\
*                    CONditional DENsity PropogaTION tracking                            *
\****************************************************************************************/

typedef struct 
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

OPENCVAPI CvConDensation*  cvCreateConDensation( int DP, int MP, int SamplesNum);
OPENCVAPI void  cvReleaseConDensation( CvConDensation** ConDensation);
OPENCVAPI void  cvConDensUpdateByTime( CvConDensation* ConDens);
OPENCVAPI void  cvConDensInitSampleSet( CvConDensation* conDens, CvMat* lowerBound,CvMat* upperBound);

/****************************************************************************************\
*                                      Kalman Filtering                                  *
\****************************************************************************************/

typedef struct 
{
    int MP;
    int DP;
    float* PosterState;          /* Vector of State of the System in k-th step  */
    float* PriorState;           /* Vector of State of the System in (k-1)-th step */
    float* DynamMatr;            /* Matrix of the linear Dynamics system */
                                 /* (Must be updated by LinearizedDynamics function on each step*/
                                 /*  for nonlinear systems)*/
    float* MeasurementMatr;      /* Matrix of linear measurement (Must be updated by */
                                 /* LinearizedMeasurement function on each step*/
                                 /* for nonlinear measurements)*/
    float* MNCovariance;         /* Matrix of measurement noice covariance*/
                                 /* Initializes to Zero matrix, or sets by SetMeasureNoiseCov*/
                                 /* method  */
    float* PNCovariance;         /* Matrix of process noice covariance*/
                                 /* Initializes to Identity matrix, or sets by SetProcessNoiseCov*/
                                 /* method */
    float* KalmGainMatr;         /* Kalman Gain Matrix*/
    float* PriorErrorCovariance; /*Prior Error Covariance matrix*/
    float* PosterErrorCovariance;/*Poster Error Covariance matrix*/
    float* Temp1;                 /* Temporary Matrix */
    float* Temp2;

} CvKalman;

OPENCVAPI CvKalman* cvCreateKalman( int DynamParams, int MeasureParams);
OPENCVAPI void  cvReleaseKalman( CvKalman** Kalman);
OPENCVAPI void  cvKalmanUpdateByTime( CvKalman* Kalman);
OPENCVAPI void  cvKalmanUpdateByMeasurement( CvKalman* Kalman, CvMat* Measurement);

/*F//////////////////////////////////////////////////////////////////////////////////////
//    Name: cvLoadPrimitives
//    Purpose: The function loads primitives
//    Context:
//    Parameters:  dllName        - name of dll to be loaded(without prefix) or NULL
//                                  for default dll
//                 processor_type - needep processor type or NULL for automatic processor
//                                  detection
//    Return value: number of loaded functions
//    Notes:   full dll name is consists from dllName + processor_type.dll(or
//             dllName + processor_type + "d".dll for debug configuration)
//F*/
OPENCVAPI  int  cvLoadPrimitives( const char* proc_type CV_DEFAULT(0) );
OPENCVAPI  int  cvFillInternalFuncsTable(void* table);


/*F//////////////////////////////////////////////////////////////////////////////////////
//    Name: cvAlloc and cvFree
//    Purpose: The functions allocate/deallocate plain memory buffers
//F*/
OPENCVAPI  void*  cvAlloc( int size );
OPENCVAPI  void   cvFree( void** ptr );


/*F//////////////////////////////////////////////////////////////////////////////////////
//    Name: cvGetLibraryInfo
//    Purpose: The function returns information about current version of library and
//             loaded/non loaded primitives dll
//    Context:
//    Parameters:  version  - pointer to pointer to version of OpenCV - build date
//                           (or NULL if not needed)
//                 loaded   - pointer to flag of loaded primitives, nonzero value
//                            after returning from function indicates that primitives
//                            are loaded(NULL if not needed)
//                 loaded_modules - comma-separated list of loaded optimized dlls
//                           (NULL if not needed)
//
//    Notes:   
//F*/
OPENCVAPI  void  cvGetLibraryInfo( const char** version, int* loaded,
                                   const char** loaded_modules );

/* **************************** Error handling ************************* */

/* /////////////////////////////////////////////////////////////////////////
// Name:       cvGetErrStatus
// Purpose:    Gets last error status
// Returns:
// Parameters:
//
// Notes:
*/
OPENCVAPI CVStatus cvGetErrStatus( void );

/* /////////////////////////////////////////////////////////////////////////
// Name:       cvSetErrStatus
// Purpose:    Sets error status
// Returns:
// Parameters:
//
// Notes:
*/
OPENCVAPI void cvSetErrStatus( CVStatus status );


/* /////////////////////////////////////////////////////////////////////////
// Name:       cvGetErrMode, cvSetErrMode
// Purpose:    gets/sets error mode
// Returns:
// Parameters:
//
// Notes:
*/
OPENCVAPI int  cvGetErrMode( void );
OPENCVAPI void cvSetErrMode( int mode );

/* /////////////////////////////////////////////////////////////////////////
// Name:       cvError
// Purpose:    performs basic error handling
// Returns:    last status
// Parameters:
//
// Notes:
*/

OPENCVAPI CVStatus cvError( CVStatus code, const char *func,
                         const char *context, const char *file, int line);

/* /////////////////////////////////////////////////////////////////////////
// Name:       cvErrorStr
// Purpose:    translates an error status code into a textual description
// Returns:
// Parameters:
//
// Notes:
*/
OPENCVAPI const char* cvErrorStr( CVStatus status );


/* /////////////////////////////////////////////////////////////////////////
// Name:       cvRedirectError
// Purpose:    assigns a new error-handling function
// Returns:    old error-handling function
// Parameters: new error-handling function
//
// Notes:
*/

OPENCVAPI CVErrorCallBack cvRedirectError(CVErrorCallBack cvErrorFunc);


/*-----------------  Predefined error-handling functions  -----------------*/

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

OPENCVAPI void cvGetCallStack(CvStackRecord** stack, int* size);

OPENCVAPI void cvStartProfile( const char* call, const char* file, int line );
OPENCVAPI void cvEndProfile( const char* file, int line );

/* management functions */
OPENCVAPI void cvSetProfile( void (CV_CDECL *startprofile_f)(const char*, const char*, int),
                          void (CV_CDECL *endprofile_f)(const char*, int) ); 
 
OPENCVAPI void cvRemoveProfile();                  

OPENCVAPI void cvSetMemoryManager( void* (CV_STDCALL *allocFunc)(int, const char*, int) CV_DEFAULT(0),
                           int (CV_STDCALL *freeFunc)(void**, const char*, int) CV_DEFAULT(0));

OPENCVAPI void cvGetCallStack(CvStackRecord** stack, int* size);



CV_EXTERN_C_FUNCPTR(IplImage* (CV_STDCALL* Cv_iplCreateImageHeader)
                            (int,int,int,char*,char*,int,int,int,int,int,
                            IplROI*,IplImage*,void*,IplTileInfo*));
CV_EXTERN_C_FUNCPTR(void (CV_STDCALL* Cv_iplAllocateImageData)(IplImage*,int,int));

CV_EXTERN_C_FUNCPTR(void (CV_STDCALL* Cv_iplDeallocate)(IplImage*,int));

CV_EXTERN_C_FUNCPTR(IplROI* (CV_STDCALL* Cv_iplCreateROI)(int,int,int,int,int));

CV_EXTERN_C_FUNCPTR(IplImage* (CV_STDCALL* Cv_iplCloneImage)(const IplImage*));


/* //////////////////////////////////////////////////////////////////////////////////
// Name:    cvSetIPLAllocators
// Purpose:  Makes OpenCV to use IPL functions for image allocation/deallocation
// Returns:
// Parameters:
//
// Notes:
*/
OPENCVAPI void
cvSetIPLAllocators( Cv_iplCreateImageHeader createHeader,
                    Cv_iplAllocateImageData allocateData,
                    Cv_iplDeallocate deallocate,
                    Cv_iplCreateROI createROI,
                    Cv_iplCloneImage cloneImage );

#define CV_TURN_ON_IPL_COMPATIBILITY()                                  \
    cvSetIPLAllocators( iplCreateImageHeader, iplAllocateImageData,     \
                        iplDeallocate, iplCreateROI, iplCloneImage )

/****************************************************************************************\
*                                 Backward compatibility                                 *
\****************************************************************************************/

#ifndef _CV_NO_BACKWARD_COMPATIBILITY
#include "cvcompat.h"
#endif

#ifdef __cplusplus
}
#endif

#endif /*_CV_H_*/
