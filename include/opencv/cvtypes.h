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

#ifndef _CVTYPES_H_
#define _CVTYPES_H_

#include <assert.h>
#include <stdlib.h>

#ifndef WIN32
    #define CV_CDECL
    #define CV_STDCALL
#else
    #define CV_CDECL __cdecl
    #define CV_STDCALL __stdcall
#endif

#ifndef CV_EXTERN_C
    #ifdef __cplusplus
        #define CV_EXTERN_C extern "C"
        #define CV_DEFAULT(val) = val
    #else
        #define CV_EXTERN_C
        #define CV_DEFAULT(val)
    #endif
#endif

#ifndef CV_EXTERN_C_FUNCPTR
    #ifdef __cplusplus
        #define CV_EXTERN_C_FUNCPTR(x) extern "C" { typedef x; }
    #else
        #define CV_EXTERN_C_FUNCPTR(x) typedef x
    #endif
#endif

#if defined WIN32 && defined CV_DLL
    #define CV_DLL_ENTRY __declspec(dllexport)
#else
    #define CV_DLL_ENTRY
#endif

#ifndef OPENCVAPI
    #define OPENCVAPI CV_EXTERN_C CV_DLL_ENTRY
#endif

#ifndef CV_INLINE
#ifdef WIN32
    #define CV_INLINE __inline
#elif defined __cplusplus
    #define CV_INLINE inline
#else
    #define CV_INLINE static
#endif
#endif /* CV_INLINE */

#if defined _MSC_VER || defined __BORLANDC__
typedef __int64 int64;
typedef unsigned __int64 uint64;
#else
typedef long long int64;
typedef unsigned long long uint64;
#endif

#ifndef __IPL_H__
typedef unsigned char uchar;
#endif


/* CvArr is used to pass arbitrary array-like data structures
   into the functions and where the particular
   array type is recognized at runtime */ 
typedef void CvArr;


/****************************************************************************************\
*                                  Image type (IplImage)                                 *
\****************************************************************************************/

#ifndef HAVE_IPL

/*
 * The following definitions (until #endif)
 * is an extract from IPL headers.
 * Copyright (c) 1995 Intel Corporation.
 */
#define IPL_DEPTH_SIGN 0x80000000

#define IPL_DEPTH_1U     1
#define IPL_DEPTH_8U     8
#define IPL_DEPTH_16U   16
#define IPL_DEPTH_32F   32

#define IPL_DEPTH_8S  (IPL_DEPTH_SIGN| 8)
#define IPL_DEPTH_16S (IPL_DEPTH_SIGN|16)
#define IPL_DEPTH_32S (IPL_DEPTH_SIGN|32)

#define IPL_DATA_ORDER_PIXEL  0
#define IPL_DATA_ORDER_PLANE  1

#define IPL_ORIGIN_TL 0
#define IPL_ORIGIN_BL 1

#define IPL_ALIGN_4BYTES   4
#define IPL_ALIGN_8BYTES   8
#define IPL_ALIGN_16BYTES 16
#define IPL_ALIGN_32BYTES 32

#define IPL_ALIGN_DWORD   IPL_ALIGN_4BYTES
#define IPL_ALIGN_QWORD   IPL_ALIGN_8BYTES

typedef struct _IplImage {
    int  nSize;         /* sizeof(IplImage) */
    int  ID;            /* version (=0)*/
    int  nChannels;     /* Most of OpenCV functions support 1,2,3 or 4 channels */
    int  alphaChannel;  /* ignored by OpenCV */
    int  depth;         /* pixel depth in bits: IPL_DEPTH_8U, IPL_DEPTH_8S, IPL_DEPTH_16S,
                           IPL_DEPTH_32S, IPL_DEPTH_32F and IPL_DEPTH_64F are supported */
    char colorModel[4]; /* ignored by OpenCV */
    char channelSeq[4]; /* ditto */
    int  dataOrder;     /* 0 - interleaved color channels, 1 - separate color channels.
                           cvCreateImage can only create interleaved images */
    int  origin;        /* 0 - top-left origin,
                           1 - bottom-left origin (Windows bitmaps style) */
    int  align;         /* Alignment of image rows (4 or 8).
                           OpenCV ignores it and uses widthStep instead */
    int  width;         /* image width in pixels */
    int  height;        /* image height in pixels */
    struct _IplROI *roi;/* image ROI. if NULL, the whole image is selected */
    struct _IplImage *maskROI; /* must be NULL */
    void  *imageId;     /* ditto */
    struct _IplTileInfo *tileInfo; /* ditto */
    int  imageSize;     /* image data size in bytes
                           (==image->height*image->widthStep
                           in case of interleaved data)*/
    char *imageData;  /* pointer to aligned image data */
    int  widthStep;   /* size of aligned image row in bytes */
    int  BorderMode[4]; /* ignored by OpenCV */
    int  BorderConst[4]; /* ditto */
    char *imageDataOrigin; /* pointer to very origin of image data
                              (not necessarily aligned) -
                              needed for correct deallocation */
}
IplImage;

typedef struct _IplTileInfo IplTileInfo;

typedef struct _IplROI {
    int  coi; /* 0 - no COI (all channels are selected), 1 - 0th channel is selected ...*/
    int  xOffset;
    int  yOffset;
    int  width;
    int  height;
}
IplROI;

typedef struct _IplConvKernel
{
    int  nCols;
    int  nRows;
    int  anchorX;
    int  anchorY;
    int *values;
    int  nShiftR;
}
IplConvKernel;

typedef struct _IplConvKernelFP
{
    int  nCols;
    int  nRows;
    int  anchorX;
    int  anchorY;
    float *values;
}
IplConvKernelFP;

#define IPL_IMAGE_HEADER 1
#define IPL_IMAGE_DATA   2
#define IPL_IMAGE_ROI    4

#endif/*HAVE_IPL*/

#define IPL_IMAGE_MAGIC_VAL  ((int)sizeof(IplImage))

/* for file storages make the value independent from arch */
#define IPL_IMAGE_FILE_MAGIC_VAL  112

#define CV_IS_IMAGE_HDR(img) \
    ((img) != NULL && ((const IplImage*)(img))->nSize == sizeof(IplImage))

#define CV_IS_IMAGE(img) \
    (CV_IS_IMAGE_HDR(img) && ((IplImage*)img)->imageData != NULL)

#define IPL_DEPTH_64F  64 /* for storing double-precision
                             floating point data in IplImage's */

/* get pointer to pixel at (col,row),
   for multi-channel images (col) should be multiplied by number of channels */
#define CV_IMAGE_ELEM( image, elemtype, row, col )       \
    (((elemtype*)((image)->imageData + (image)->widthStep*(row))[(col)])


/****************************************************************************************\
*                                  Matrix type (CvMat)                                   *
\****************************************************************************************/

#define CV_8U   0
#define CV_8S   1
#define CV_16S  2
#define CV_32S  3
#define CV_32F  4
#define CV_64F  5
#define CV_USRTYPE1 6
#define CV_USRTYPE2 7

#define CV_8UC1 (CV_8U + 0*8)
#define CV_8UC2 (CV_8U + 1*8)
#define CV_8UC3 (CV_8U + 2*8)
#define CV_8UC4 (CV_8U + 3*8)

#define CV_8SC1 (CV_8S + 0*8)
#define CV_8SC2 (CV_8S + 1*8)
#define CV_8SC3 (CV_8S + 2*8)
#define CV_8SC4 (CV_8S + 3*8)

#define CV_16SC1 (CV_16S + 0*8)
#define CV_16SC2 (CV_16S + 1*8)
#define CV_16SC3 (CV_16S + 2*8)
#define CV_16SC4 (CV_16S + 3*8)

#define CV_32SC1 (CV_32S + 0*8)
#define CV_32SC2 (CV_32S + 1*8)
#define CV_32SC3 (CV_32S + 2*8)
#define CV_32SC4 (CV_32S + 3*8)

#define CV_32FC1 (CV_32F + 0*8)
#define CV_32FC2 (CV_32F + 1*8)
#define CV_32FC3 (CV_32F + 2*8)
#define CV_32FC4 (CV_32F + 3*8)

#define CV_64FC1 (CV_64F + 0*8)
#define CV_64FC2 (CV_64F + 1*8)
#define CV_64FC3 (CV_64F + 2*8)
#define CV_64FC4 (CV_64F + 3*8)

#define CV_AUTO_STEP  0x7fffffff
#define CV_WHOLE_ARR  cvSlice( 0, 0x3fffffff )

#define CV_MAT_CN_MASK          (3 << 3)
#define CV_MAT_CN(flags)        ((((flags) & CV_MAT_CN_MASK) >> 3) + 1)
#define CV_MAT_DEPTH_MASK       7
#define CV_MAT_DEPTH(flags)     ((flags) & CV_MAT_DEPTH_MASK)
#define CV_MAT_TYPE_MASK        31
#define CV_MAT_TYPE(flags)      ((flags) & CV_MAT_TYPE_MASK)
#define CV_MAT_FMT_MASK         511
#define CV_MAT_FMT(flags)       ((flags) & CV_MAT_FMT_MASK)
#define CV_MAT_CONT_FLAG_SHIFT  9
#define CV_MAT_CONT_FLAG        (1 << CV_MAT_CONT_FLAG_SHIFT)
#define CV_IS_MAT_CONT(flags)   ((flags) & CV_MAT_CONT_FLAG) 
#define CV_IS_CONT_MAT          CV_IS_MAT_CONT
#define CV_MAT_TEMP_FLAG_SHIFT  10
#define CV_MAT_TEMP_FLAG        (1 << CV_MAT_TEMP_FLAG_SHIFT)
#define CV_IS_TEMP_MAT(flags)   ((flags) & CV_MAT_TEMP_FLAG)

#define CV_MAGIC_MASK       0xFFFF0000
#define CV_MAT_MAGIC_VAL    0x42420000

typedef struct CvMat
{
    int type;
    int step;

    /* for internal use only */
    int* refcount;

    union
    {
        uchar* ptr;
        short* s;
        int* i;
        float* fl;
        double* db;
    } data;

#ifdef __cplusplus
    union
    {
        int rows;
        int height;
    };

    union
    {
        int cols;
        int width;
    };
#else
    int rows;
    int cols;
#endif

} CvMat;


#define CV_IS_MAT_HDR(mat) \
    ((mat) != NULL && (((const CvMat*)(mat))->type & CV_MAGIC_MASK) == CV_MAT_MAGIC_VAL)

#define CV_IS_MAT(mat) \
    (CV_IS_MAT_HDR(mat) && ((const CvMat*)(mat))->data.ptr != NULL)

#define CV_IS_MASK_ARR(mat) \
    (((mat)->type & (CV_MAT_TYPE_MASK & ~CV_8SC1)) == 0)

#define CV_ARE_TYPES_EQ(mat1, mat2) \
    ((((mat1)->type ^ (mat2)->type) & CV_MAT_TYPE_MASK) == 0)

#define CV_ARE_CNS_EQ(mat1, mat2) \
    ((((mat1)->type ^ (mat2)->type) & CV_MAT_CN_MASK) == 0)

#define CV_ARE_DEPTHS_EQ(mat1, mat2) \
    ((((mat1)->type ^ (mat2)->type) & CV_MAT_DEPTH_MASK) == 0)

#define CV_ARE_SIZES_EQ(mat1, mat2) \
    ((mat1)->height == (mat2)->height && (mat1)->width == (mat2)->width)

#define CV_IS_MAT_CONST(mat)  \
    (((mat)->height|(mat)->width) == 1)

#define CV_ELEM_SIZE(type) \
    (CV_MAT_CN(type) << ((0xe90 >> CV_MAT_DEPTH(type)*2) & 3))

/* inline constructor. No data is allocated internally!!!
   (use together with cvCreateData, or use cvCreateMat instead to
   get a matrix with allocated data */
CV_INLINE CvMat cvMat( int rows, int cols, int type, void* data CV_DEFAULT(NULL));
CV_INLINE CvMat cvMat( int rows, int cols, int type, void* data )
{
    CvMat m;

    assert( (unsigned)CV_MAT_DEPTH(type) <= CV_64F );
    type = CV_MAT_TYPE(type);
    m.type = CV_MAT_MAGIC_VAL | CV_MAT_CONT_FLAG | type;
    m.cols = cols;
    m.rows = rows;
    m.step = m.cols*CV_ELEM_SIZE(type);
    m.data.ptr = (uchar*)data;
    m.refcount = NULL;

    return m; 
}


#define CV_MAT_ELEM_PTR_FAST( mat, row, col, pix_size )  \
    (assert( (unsigned)(row) < (unsigned)(mat).rows &&   \
             (unsigned)(col) < (unsigned)(mat).cols ),   \
     (mat).data.ptr + (mat).step*(row) + (pix_size)*(col))

#define CV_MAT_ELEM_PTR( mat, row, col )                 \
    CV_MAT_ELEM_PTR_FAST( mat, row, col, CV_ELEM_SIZE((mat).type) )

#define CV_MAT_ELEM( mat, elemtype, row, col )           \
    (*(elemtype*)CV_MAT_ELEM_PTR_FAST( mat, row, col, sizeof(elemtype)))


CV_INLINE  double  cvmGet( const CvMat* mat, int i, int j );
CV_INLINE  double  cvmGet( const CvMat* mat, int i, int j )
{
    int type;

    type = CV_MAT_TYPE(mat->type);
    assert( (unsigned)i < (unsigned)mat->rows &&
            (unsigned)j < (unsigned)mat->cols );

    if( type == CV_32FC1 )
        return ((float*)(mat->data.ptr + i*mat->step))[j];
    else
    {
        assert( type == CV_64FC1 );
        return ((double*)(mat->data.ptr + i*mat->step))[j];
    }
}


CV_INLINE  void  cvmSet( CvMat* mat, int i, int j, double val );
CV_INLINE  void  cvmSet( CvMat* mat, int i, int j, double val )
{
    int type;
    type = CV_MAT_TYPE(mat->type);
    assert( (unsigned)i < (unsigned)mat->rows &&
            (unsigned)j < (unsigned)mat->cols );

    if( type == CV_32FC1 )
        ((float*)(mat->data.ptr + i*mat->step))[j] = (float)val;
    else
    {
        assert( type == CV_64FC1 );
        ((double*)(mat->data.ptr + i*mat->step))[j] = (double)val;
    }
}

/****************************************************************************************\
*                       Multi-dimensional dense array (CvMatND)                          *
\****************************************************************************************/

#define CV_MATND_MAGIC_VAL    0x42430000
#define CV_MAX_DIM 16

#define CV_MAT_LIKE_FLAG_SHIFT  11
#define CV_MAT_LIKE_FLAG        (1 << CV_MAT_LIKE_FLAG_SHIFT)

typedef struct CvMatND
{
    int type;
    int dims;

    int* refcount;
    union
    {
        uchar* ptr;
        float* fl;
        double* db;
        int* i;
        short* s;
    } data;

    struct
    {
        int size;
        int step;
    }
    dim[CV_MAX_DIM];
}
CvMatND;

#define CV_IS_MATND_HDR(mat) \
    ((mat) != NULL && (((const CvMatND*)(mat))->type & CV_MAGIC_MASK) == CV_MATND_MAGIC_VAL)

#define CV_IS_MATND(mat) \
    (CV_IS_MATND_HDR(mat) && ((const CvMatND*)(mat))->data.ptr != NULL)


/****************************************************************************************\
*                      Multi-dimensional sparse array (CvSparseMat)                      *
\****************************************************************************************/

#define CV_SPARSE_MAT_MAGIC_VAL    0x42440000

struct CvSet;

typedef struct CvSparseMat
{
    int type;
    int dims;
    int* refcount;
    struct CvSet* heap;
    void** hashtable;
    int hashsize;
    int total;
    int valoffset;
    int idxoffset;
    int size[CV_MAX_DIM];   
}
CvSparseMat;

#define CV_IS_SPARSE_MAT_HDR(mat) \
    ((mat) != NULL && \
    (((const CvSparseMat*)(mat))->type & CV_MAGIC_MASK) == CV_SPARSE_MAT_MAGIC_VAL)

#define CV_IS_SPARSE_MAT(mat) \
    CV_IS_SPARSE_MAT_HDR(mat)

/**************** iteration through a sparse array *****************/

typedef struct CvSparseNode
{
    unsigned hashval;
    struct CvSparseNode* next;
}
CvSparseNode;

typedef struct CvSparseMatIterator
{
    CvSparseMat* mat;
    CvSparseNode* node;
    int curidx;
}
CvSparseMatIterator;

#define CV_NODE_VAL(mat,node)   ((void*)((uchar*)(node) + (mat)->valoffset))
#define CV_NODE_IDX(mat,node)   ((int*)((uchar*)(node) + (mat)->idxoffset))

/****************************************************************************************\
*                                         Histogram                                      *
\****************************************************************************************/

typedef int CvHistType;

#define CV_HIST_MAGIC_VAL     0x42450000
#define CV_HIST_UNIFORM_FLAG  (1 << 10)

/* indicates whether bin ranges are set already or not */
#define CV_HIST_RANGES_FLAG   (1 << 11)

#define CV_HIST_ARRAY         0
#define CV_HIST_SPARSE        1
#define CV_HIST_TREE          CV_HIST_SPARSE

#define CV_HIST_UNIFORM       1 /* should be used as a parameter only,
                                   it turns to CV_HIST_UNIFORM_FLAG of hist->type */

typedef struct CvHistogram
{
    int     type;
    CvArr*  bins;
    float   thresh[CV_MAX_DIM][2]; /* for uniform histograms */
    float** thresh2; /* for non-uniform histograms */
    CvMatND mat; /* embedded matrix header for array histograms */
}
CvHistogram;

#define CV_IS_HIST( hist ) \
    ((hist) != NULL  && \
     (((CvHistogram*)(hist))->type & CV_MAGIC_MASK) == CV_HIST_MAGIC_VAL && \
     (hist)->bins != NULL)

#define CV_IS_UNIFORM_HIST( hist ) \
    (((hist)->type & CV_HIST_UNIFORM_FLAG) != 0)

#define CV_IS_SPARSE_HIST( hist ) \
    CV_IS_SPARSE_MAT((hist)->bins)

#define CV_HIST_HAS_RANGES( hist ) \
    (((hist)->type & CV_HIST_RANGES_FLAG) != 0) 

/****************************************************************************************\
*                      Other supplementary data type definitions                         *
\****************************************************************************************/

/* ************************************************************* *\
   substitutions for round(x), floor(x), ceil(x):
   the algorithm was taken from Agner Fog's optimization guide
   at http://www.agner.org/assem
\* ************************************************************* */
CV_INLINE  int  cvRound( double val );
CV_INLINE  int  cvRound( double val )
{
    double temp = val + 6755399441055744.0;
    return (int)*((uint64*)&temp);
}


CV_INLINE  int  cvFloor( double val );
CV_INLINE  int  cvFloor( double val )
{
    double temp = val + 6755399441055744.0;
    float diff = (float)(val - (int)*((uint64*)&temp));

    return (int)*((uint64*)&temp) - (*(int*)&diff < 0);
}


CV_INLINE  int  cvCeil( double val );
CV_INLINE  int  cvCeil( double val )
{
    double temp = val + 6755399441055744.0;
    float diff = (float)((int)*((uint64*)&temp) - val);

    return (int)*((uint64*)&temp) + (*(int*)&diff < 0);
}

/*************************************** CvRect *****************************************/

typedef struct CvRect
{
    int x;
    int y;
    int width;
    int height;
}
CvRect;

CV_INLINE  CvRect  cvRect( int x, int y, int width, int height );
CV_INLINE  CvRect  cvRect( int x, int y, int width, int height )
{
    CvRect r;

    r.x = x;
    r.y = y;
    r.width = width;
    r.height = height;

    return r;
}


CV_INLINE  IplROI  cvRectToROI( CvRect rect, int coi CV_DEFAULT(0));
CV_INLINE  IplROI  cvRectToROI( CvRect rect, int coi )
{
    IplROI roi;
    roi.xOffset = rect.x;
    roi.yOffset = rect.y;
    roi.width = rect.width;
    roi.height = rect.height;
    roi.coi = coi;

    return roi;
}


CV_INLINE  CvRect  cvROIToRect( IplROI roi );
CV_INLINE  CvRect  cvROIToRect( IplROI roi )
{
    return cvRect( roi.xOffset, roi.yOffset, roi.width, roi.height );
}

/*********************************** CvTermCriteria *************************************/

#define CV_TERMCRIT_ITER    1
#define CV_TERMCRIT_NUMB    CV_TERMCRIT_ITER
#define CV_TERMCRIT_EPS     2

typedef struct CvTermCriteria
{
    int    type;  /* may be combination of
                     CV_TERMCRIT_ITER
                     CV_TERMCRIT_EPS */
    int    maxIter;
    double epsilon;
}
CvTermCriteria;

CV_INLINE  CvTermCriteria  cvTermCriteria( int type, int maxIter, double epsilon );
CV_INLINE  CvTermCriteria  cvTermCriteria( int type, int maxIter, double epsilon )
{
    CvTermCriteria t;

    t.type = type;
    t.maxIter = maxIter;
    t.epsilon = (float)epsilon;

    return t;
}


/******************************* CvPoint and variants ***********************************/

typedef struct CvPoint
{
    int x;
    int y;
}
CvPoint;


CV_INLINE  CvPoint  cvPoint( int x, int y );
CV_INLINE  CvPoint  cvPoint( int x, int y )
{
    CvPoint p;

    p.x = x;
    p.y = y;

    return p;
}


typedef struct CvPoint2D32f
{
    float x;
    float y;
}
CvPoint2D32f;


CV_INLINE  CvPoint2D32f  cvPoint2D32f( double x, double y );
CV_INLINE  CvPoint2D32f  cvPoint2D32f( double x, double y )
{
    CvPoint2D32f p;

    p.x = (float)x;
    p.y = (float)y;

    return p;
}


CV_INLINE  CvPoint2D32f  cvPointTo32f( CvPoint pt );
CV_INLINE  CvPoint2D32f  cvPointTo32f( CvPoint pt )
{
    return cvPoint2D32f( (float)pt.x, (float)pt.y );    
}


CV_INLINE  CvPoint  cvPointFrom32f( CvPoint2D32f pt );
CV_INLINE  CvPoint  cvPointFrom32f( CvPoint2D32f pt )
{
    return cvPoint( cvRound(pt.x), cvRound(pt.y) );    
}


typedef struct CvPoint3D32f
{
    float x;
    float y;
    float z;
}
CvPoint3D32f;


CV_INLINE  CvPoint3D32f  cvPoint3D32f( double x, double y, double z );
CV_INLINE  CvPoint3D32f  cvPoint3D32f( double x, double y, double z )
{
    CvPoint3D32f p;

    p.x = (float)x;
    p.y = (float)y;
    p.z = (float)z;

    return p;
}           


typedef struct CvPoint2D64d
{
    double x;
    double y;
}
CvPoint2D64d;


typedef struct CvPoint3D64d
{
    double x;
    double y;
    double z;
}
CvPoint3D64d;


/******************************** CvSize's & CvBox **************************************/

typedef struct
{
    int width;
    int height;
}
CvSize;

CV_INLINE  CvSize  cvSize( int width, int height );
CV_INLINE  CvSize  cvSize( int width, int height )
{
    CvSize s;

    s.width = width;
    s.height = height;

    return s;
}

typedef struct CvSize2D32f
{
    float width;
    float height;
}
CvSize2D32f;


CV_INLINE  CvSize2D32f  cvSize2D32f( double width, double height );
CV_INLINE  CvSize2D32f  cvSize2D32f( double width, double height )
{
    CvSize2D32f s;

    s.width = (float)width;
    s.height = (float)height;

    return s;
}

typedef struct CvBox2D
{
    CvPoint2D32f center;  /* center of the box */
    CvSize2D32f  size;    /* box width and length */
    float angle;          /* angle between the horizontal axis
                             and the first side (i.e. length) in radians */
}
CvBox2D;

/************************************* CvSlice ******************************************/

typedef struct CvSlice
{
    int  startIndex, endIndex;
} CvSlice;

CV_INLINE  CvSlice  cvSlice( int start, int end );
CV_INLINE  CvSlice  cvSlice( int start, int end )
{
    CvSlice slice;
    slice.startIndex = start;
    slice.endIndex = end;

    return slice;
}

#define CV_WHOLE_SEQ  cvSlice(0, 0x3fffffff)


/************************************* CvScalar *****************************************/

typedef struct CvScalar
{
    double val[4];
}
CvScalar;


CV_INLINE  CvScalar  cvScalar( double a, double b CV_DEFAULT(0),
                               double c CV_DEFAULT(0), double d CV_DEFAULT(0));
CV_INLINE  CvScalar  cvScalar( double a, double b, double c, double d )
{
    CvScalar scalar;
    scalar.val[0] = a; scalar.val[1] = b;
    scalar.val[2] = c; scalar.val[3] = d;
    return scalar;
}


CV_INLINE  CvScalar  cvRealScalar( double a );
CV_INLINE  CvScalar  cvRealScalar( double a )
{
    CvScalar scalar;
    scalar.val[0] = a;
    scalar.val[1] = scalar.val[2] = scalar.val[3] = 0;
    return scalar;
}

CV_INLINE  CvScalar  cvScalarAll( double a );
CV_INLINE  CvScalar  cvScalarAll( double a )
{
    CvScalar scalar;
    scalar.val[0] = scalar.val[1] = scalar.val[2] = scalar.val[3] = a;
    return scalar;
}


/**************************** Connected Component  **************************************/

struct CvSeq;

typedef struct CvConnectedComp
{
    double area;  /* area of the connected component  */
    double value; /* average brightness of the connected component
                     (or packed RGB color) */
    CvRect rect;  /* ROI of the component  */
    struct CvSeq* contour; /* optional component boundary
                     (the contour might have child contours corresponding to the holes)*/
}
CvConnectedComp;


/*************** Utility definitions, macros and inline functions ***********************/

#define CV_PI   3.1415926535897932384626433832795

#define CV_SWAP(a,b,t) ((t) = (a), (a) = (b), (b) = (t))

#ifndef MIN
#define MIN(a,b)  ((a) > (b) ? (b) : (a))
#endif

#ifndef MAX
#define MAX(a,b)  ((a) < (b) ? (b) : (a))
#endif

/* min & max without jumps */
#define  CV_IMIN(a, b)  ((a) ^ (((a)^(b)) & (((a) < (b)) - 1)))

#define  CV_IMAX(a, b)  ((a) ^ (((a)^(b)) & (((a) > (b)) - 1)))

/* absolute value without jumps */
#define  CV_IABS(a)     (((a) ^ ((a) < 0 ? -1 : 0)) - ((a) < 0 ? -1 : 0))
#define  CV_SIGN(a)     (((a) < 0 ? -1 : 0) | ((a) > 0))

/* initializes 8-element array for fast access to 3x3 neighborhood of a pixel */
#define  CV_INIT_3X3_DELTAS( deltas, step, nch )            \
    ((deltas)[0] =  (nch),  (deltas)[1] = -(step) + (nch),  \
     (deltas)[2] = -(step), (deltas)[3] = -(step) - (nch),  \
     (deltas)[4] = -(nch),  (deltas)[5] =  (step) - (nch),  \
     (deltas)[6] =  (step), (deltas)[7] =  (step) + (nch))

/* ************************************************************************** *\
   Fast square root and inverse square root by
   Bruce W. Holloway, Jeremy M., James Van Buskirk, Vesa Karvonen and others.
   Taken from Paul Hsieh's site http://www.azillionmonkeys.com/qed/sqroot.html.
\* ************************************************************************** */
#define CV_SQRT_MAGIC  0xbe6f0000

CV_INLINE  float  cvInvSqrt( float arg );
CV_INLINE  float  cvInvSqrt( float arg )
{
    float x, y;
    unsigned iarg = *((unsigned*)&arg);
    *((unsigned*)&x) = (CV_SQRT_MAGIC - iarg)>>1;

    y = arg*0.5f;
    x*= 1.5f - y*x*x;
    x*= 1.5f - y*x*x;

    return x;
}


CV_INLINE  float  cvSqrt( float arg );
CV_INLINE  float  cvSqrt( float arg )
{
    float x, y;
    unsigned iarg = *((unsigned*)&arg);
    *((unsigned*)&x) = (CV_SQRT_MAGIC - iarg)>>1;

    y = arg*0.5f;
    x*= 1.5f - y*x*x;
    x*= 1.5f - y*x*x;

    return x*arg;
}


/* Defines for Distance Transform */
typedef enum CvDisType
{
    CV_DIST_USER         = -1, /* User defined distance */
    CV_DIST_L1           = 1, /* distance = |x1-x2| + |y1-y2| */
    CV_DIST_L2           = 2, /* the simple euclidean distance */
    CV_DIST_C            = 3, /* distance = max(|x1-x2|,|y1-y2|) */
    CV_DIST_L12          = 4, /* L1-L2 metric: distance = 2(sqrt(1+x*x/2) - 1)) */
    CV_DIST_FAIR         = 5, /* distance = c^2(|x|/c-log(1+|x|/c)), c = 1.3998 */
    CV_DIST_WELSCH       = 6, /* distance = c^2/2(1-exp(-(x/c)^2)), c = 2.9846 */
    CV_DIST_HUBER        = 7  /* distance = |x|<c ? x^2/2 : c(|x|-c/2), c=1.345 */
}
CvDisType;


/* Filters used in pyramid decomposition */
typedef enum CvFilter
{
    CV_GAUSSIAN_5x5 = 7
}
CvFilter;

/****************************************************************************************/
/*                                   Data structures                                    */
/****************************************************************************************/

/******************************** Memory storage ****************************************/

typedef struct CvMemBlock
{
    struct CvMemBlock*  prev;
    struct CvMemBlock*  next;
}
CvMemBlock;

#define CV_STORAGE_MAGIC_VAL    0x42890000

typedef struct CvMemStorage
{
    int     signature;
    CvMemBlock* bottom;/* first allocated block */
    CvMemBlock* top;   /* current memory block - top of the stack */
    struct  CvMemStorage* parent; /* borrows new blocks from */
    int     block_size;  /* block size */
    int     free_space;  /* free space in the current block */
}
CvMemStorage;

#define CV_IS_STORAGE(storage)  \
    ((storage) != NULL &&       \
    (((CvMemStorage*)(storage))->signature & CV_MAGIC_MASK) == CV_STORAGE_MAGIC_VAL)


typedef struct CvMemStoragePos
{
    CvMemBlock* top;
    int  free_space;
}
CvMemStoragePos;


/*********************************** Sequence *******************************************/

typedef struct CvSeqBlock
{
    struct CvSeqBlock*  prev; /* previous sequence block */
    struct CvSeqBlock*  next; /* next sequence block */
    int    start_index;       /* index of the first element in the block +
                                 sequence->first->start_index */
    int    count;             /* number of elements in the block */
    char*  data;              /* pointer to the first element of the block */
}
CvSeqBlock;


#define CV_TREE_NODE_FIELDS(node_type)                          \
    int       flags;         /* micsellaneous flags */          \
    int       header_size;   /* size of sequence header */      \
    struct    node_type* h_prev; /* previous sequence */        \
    struct    node_type* h_next; /* next sequence */            \
    struct    node_type* v_prev; /* 2nd previous sequence */    \
    struct    node_type* v_next; /* 2nd next sequence */

/*
   Read/Write sequence.
   Elements can be dynamically inserted to or deleted from the sequence.
*/
#define CV_SEQUENCE_FIELDS()                                            \
    CV_TREE_NODE_FIELDS(CvSeq)                                          \
    int       total;          /* total number of elements */            \
    int       elem_size;      /* size of sequence element in bytes */   \
    char*     block_max;      /* maximal bound of the last block */     \
    char*     ptr;            /* current write pointer */               \
    int       delta_elems;    /* how many elements allocated when the seq grows */  \
    CvMemStorage* storage;    /* where the seq is stored */             \
    CvSeqBlock* free_blocks;  /* free blocks list */                    \
    CvSeqBlock* first; /* pointer to the first sequence block */


typedef struct CvSeq
{
    CV_SEQUENCE_FIELDS()
}
CvSeq;


/*************************************** Set ********************************************/
/*
  Set.
  Order isn't keeped. There can be gaps between sequence elements.
  After the element has been inserted it stays on the same place all the time.
  The MSB(most-significant or sign bit) of the first field is 0 iff the element exists.
*/
#define CV_SET_ELEM_FIELDS(elem_type)   \
    int  flags;                         \
    struct elem_type* next_free;

typedef struct CvSetElem
{
    CV_SET_ELEM_FIELDS(CvSetElem)
}
CvSetElem;

#define CV_SET_FIELDS()      \
    CV_SEQUENCE_FIELDS()     \
    CvSetElem* free_elems;

typedef struct CvSet
{
    CV_SET_FIELDS()
}
CvSet;


#define CV_SET_ELEM_IDX_MASK   ((1 << 24) - 1)
#define CV_SET_ELEM_FREE_FLAG  (1 << (sizeof(int)*8-1))

/* Checks whether the element pointed by ptr belongs to a set or not */
#define CV_IS_SET_ELEM( ptr )  (((CvSetElem*)(ptr))->flags >= 0)

/************************************* Graph ********************************************/

/*
  Graph is represented as a set of vertices.
  Vertices contain their adjacency lists (more exactly, pointers to first incoming or
  outcoming edge (or 0 if isolated vertex)). Edges are stored in another set.
  There is a single-linked list of incoming/outcoming edges for each vertex.

  Each edge consists of:
    two pointers to the starting and the ending vertices (vtx[0] and vtx[1],
    respectively). Graph may be oriented or not. In the second case, edges between
    vertex i to vertex j are not distingueshed (during the search operations).

    two pointers to next edges for the starting and the ending vertices.
    next[0] points to the next edge in the vtx[0] adjacency list and
    next[1] points to the next edge in the vtx[1] adjacency list.
*/
#define CV_GRAPH_EDGE_FIELDS()      \
    int flags;                      \
    float weight;                   \
    struct CvGraphEdge* next[2];    \
    struct CvGraphVtx* vtx[2];
    

#define CV_GRAPH_VERTEX_FIELDS()    \
    int flags;                      \
    struct CvGraphEdge* first;
    

typedef struct CvGraphEdge
{
    CV_GRAPH_EDGE_FIELDS()
}
CvGraphEdge;

typedef struct CvGraphVtx
{
    CV_GRAPH_VERTEX_FIELDS()
}
CvGraphVtx;

typedef struct CvGraphVtx2D
{
    CV_GRAPH_VERTEX_FIELDS()
    CvPoint2D32f* ptr;
}
CvGraphVtx2D;

/*
   Graph is "derived" from the set (this is set a of vertices)
   and includes another set (edges)
*/
#define  CV_GRAPH_FIELDS()   \
    CV_SET_FIELDS()          \
    CvSet* edges;

typedef struct CvGraph
{
    CV_GRAPH_FIELDS()
}
CvGraph;

/*********************************** Chain/Countour *************************************/

typedef struct CvChain
{
    CV_SEQUENCE_FIELDS()
    CvPoint  origin;
}
CvChain;

#define CV_CONTOUR_FIELDS()  \
    CV_SEQUENCE_FIELDS()     \
    CvRect rect;             \
    int color;               \
    int reserved[3];

typedef struct CvContour
{
    CV_CONTOUR_FIELDS()
}
CvContour;

typedef CvContour CvPoint2DSeq;

/****************************************************************************************\
*                                    Sequence types                                      *
\****************************************************************************************/

#define CV_SEQ_MAGIC_VAL             0x42990000
#define CV_IS_SEQ(seq) \
    ((seq) != NULL && (((CvSeq*)(seq))->flags & CV_MAGIC_MASK) == CV_SEQ_MAGIC_VAL)

#define CV_SET_MAGIC_VAL             0x42980000
#define CV_IS_SET(set) \
    ((set) != NULL && (((CvSeq*)(set))->flags & CV_MAGIC_MASK) == CV_SET_MAGIC_VAL)

#define CV_SEQ_ELTYPE_BITS           5
#define CV_SEQ_ELTYPE_MASK           ((1 << CV_SEQ_ELTYPE_BITS) - 1)

#define CV_SEQ_ELTYPE_POINT          CV_32SC2  /* (x,y) */
#define CV_SEQ_ELTYPE_CODE           CV_8UC1   /* freeman code: 0..7 */
#define CV_SEQ_ELTYPE_GENERIC        0
#define CV_SEQ_ELTYPE_PTR            CV_USRTYPE1 
#define CV_SEQ_ELTYPE_PPOINT         CV_SEQ_ELTYPE_PTR  /* &(x,y) */
#define CV_SEQ_ELTYPE_INDEX          CV_32SC1  /* #(x,y) */
#define CV_SEQ_ELTYPE_GRAPH_EDGE     0  /* &next_o, &next_d, &vtx_o, &vtx_d */
#define CV_SEQ_ELTYPE_GRAPH_VERTEX   0  /* first_edge, &(x,y) */
#define CV_SEQ_ELTYPE_TRIAN_ATR      0  /* vertex of the binary tree   */
#define CV_SEQ_ELTYPE_CONNECTED_COMP 0  /* connected component  */
#define CV_SEQ_ELTYPE_POINT3D        CV_32FC3  /* (x,y,z)  */

#define CV_SEQ_KIND_BITS        5
#define CV_SEQ_KIND_MASK        (((1 << CV_SEQ_KIND_BITS) - 1)<<CV_SEQ_ELTYPE_BITS)

/* types of sequences */
#define CV_SEQ_KIND_GENERIC     (0 << CV_SEQ_ELTYPE_BITS)
#define CV_SEQ_KIND_CURVE       (1 << CV_SEQ_ELTYPE_BITS)
#define CV_SEQ_KIND_BIN_TREE    (2 << CV_SEQ_ELTYPE_BITS)

/* types of sparse sequences (sets) */
#define CV_SEQ_KIND_GRAPH       (3 << CV_SEQ_ELTYPE_BITS)
#define CV_SEQ_KIND_SUBDIV2D    (4 << CV_SEQ_ELTYPE_BITS)

#define CV_SEQ_FLAG_SHIFT       (CV_SEQ_KIND_BITS + CV_SEQ_ELTYPE_BITS)

/* flags for curves */
#define CV_SEQ_FLAG_CLOSED     (1 << CV_SEQ_FLAG_SHIFT)
#define CV_SEQ_FLAG_SIMPLE     (2 << CV_SEQ_FLAG_SHIFT)
#define CV_SEQ_FLAG_CONVEX     (4 << CV_SEQ_FLAG_SHIFT)
#define CV_SEQ_FLAG_HOLE       (8 << CV_SEQ_FLAG_SHIFT)

/* flags for graphs */
#define CV_GRAPH_FLAG_ORIENTED (1 << CV_SEQ_FLAG_SHIFT)

#define CV_GRAPH               CV_SEQ_KIND_GRAPH
#define CV_ORIENTED_GRAPH      (CV_SEQ_KIND_GRAPH|CV_GRAPH_FLAG_ORIENTED)

/* point sets */
#define CV_SEQ_POINT_SET       (CV_SEQ_KIND_GENERIC| CV_SEQ_ELTYPE_POINT)
#define CV_SEQ_POINT3D_SET     (CV_SEQ_KIND_GENERIC| CV_SEQ_ELTYPE_POINT3D)
#define CV_SEQ_POLYLINE        (CV_SEQ_KIND_CURVE  | CV_SEQ_ELTYPE_POINT)
#define CV_SEQ_POLYGON         (CV_SEQ_FLAG_CLOSED | CV_SEQ_POLYLINE )
#define CV_SEQ_CONTOUR         CV_SEQ_POLYGON
#define CV_SEQ_SIMPLE_POLYGON  (CV_SEQ_FLAG_SIMPLE | CV_SEQ_POLYGON  )

/* chain-coded curves */
#define CV_SEQ_CHAIN           (CV_SEQ_KIND_CURVE  | CV_SEQ_ELTYPE_CODE)
#define CV_SEQ_CHAIN_CONTOUR   (CV_SEQ_FLAG_CLOSED | CV_SEQ_CHAIN)

/* binary tree for the contour */
#define CV_SEQ_POLYGON_TREE    (CV_SEQ_KIND_BIN_TREE  | CV_SEQ_ELTYPE_TRIAN_ATR)

/* sequence of the connected components */
#define CV_SEQ_CONNECTED_COMP  (CV_SEQ_KIND_GENERIC  | CV_SEQ_ELTYPE_CONNECTED_COMP)

/* sequence of the integer numbers */
#define CV_SEQ_INDEX           (CV_SEQ_KIND_GENERIC  | CV_SEQ_ELTYPE_INDEX)

#define CV_SEQ_ELTYPE( seq )   ((seq)->flags & CV_SEQ_ELTYPE_MASK)
#define CV_SEQ_KIND( seq )     ((seq)->flags & CV_SEQ_KIND_MASK )

/* flag checking */
#define CV_IS_SEQ_INDEX( seq )      ((CV_SEQ_ELTYPE(seq) == CV_SEQ_ELTYPE_INDEX) && \
                                     (CV_SEQ_KIND(seq) == CV_SEQ_KIND_GENERIC))

#define CV_IS_SEQ_CURVE( seq )      (CV_SEQ_KIND(seq) == CV_SEQ_KIND_CURVE)
#define CV_IS_SEQ_CLOSED( seq )     (((seq)->flags & CV_SEQ_FLAG_CLOSED) != 0)
#define CV_IS_SEQ_CONVEX( seq )     (((seq)->flags & CV_SEQ_FLAG_CONVEX) != 0)
#define CV_IS_SEQ_HOLE( seq )       (((seq)->flags & CV_SEQ_FLAG_HOLE) != 0)
#define CV_IS_SEQ_SIMPLE( seq )     (((seq)->flags & CV_SEQ_FLAG_SIMPLE) != 0) || \
                                    CV_IS_SEQ_CONVEX(seq))

/* type checking macros */
#define CV_IS_SEQ_POINT_SET( seq ) \
    ((CV_SEQ_ELTYPE(seq) == CV_32SC2 || CV_SEQ_ELTYPE(seq) == CV_32FC2))

#define CV_IS_SEQ_POINT_SUBSET( seq ) \
    (CV_IS_SEQ_INDEX( seq ) || CV_SEQ_ELTYPE(seq) == CV_SEQ_ELTYPE_PPOINT)

#define CV_IS_SEQ_POLYLINE( seq )   \
    (CV_SEQ_KIND(seq) == CV_SEQ_KIND_CURVE && CV_IS_SEQ_POINT_SET(seq))

#define CV_IS_SEQ_POLYGON( seq )   \
    (CV_IS_SEQ_POLYLINE(seq) && CV_IS_SEQ_CLOSED(seq))

#define CV_IS_SEQ_CHAIN( seq )   \
    (CV_SEQ_KIND(seq) == CV_SEQ_KIND_CURVE && (seq)->elem_size == 1)

#define CV_IS_SEQ_CONTOUR( seq )   \
    (CV_IS_SEQ_CLOSED(seq) && (CV_IS_SEQ_POLYLINE(seq) || CV_IS_SEQ_CHAIN(seq)))

#define CV_IS_SEQ_CHAIN_CONTOUR( seq ) \
    (CV_IS_SEQ_CHAIN( seq ) && CV_IS_SEQ_CLOSED( seq ))

#define CV_IS_SEQ_POLYGON_TREE( seq ) \
    (CV_SEQ_ELTYPE (seq) ==  CV_SEQ_ELTYPE_TRIAN_ATR &&    \
    CV_SEQ_KIND( seq ) ==  CV_SEQ_KIND_BIN_TREE )

#define CV_IS_GRAPH( seq )    \
    (CV_IS_SET(seq) && CV_SEQ_KIND((CvSet*)(seq)) == CV_SEQ_KIND_GRAPH)

#define CV_IS_GRAPH_ORIENTED( seq )   \
    (((seq)->flags & CV_GRAPH_FLAG_ORIENTED) != 0)

#define CV_IS_SUBDIV2D( seq )  \
    (CV_IS_SET(seq) && CV_SEQ_KIND((CvSet*)(seq)) == CV_SEQ_KIND_SUBDIV2D)

/****************************************************************************************/
/*                            Sequence writer & reader                                  */
/****************************************************************************************/

#define CV_SEQ_WRITER_FIELDS()                                     \
    int          header_size;                                      \
    CvSeq*       seq;        /* the sequence written */            \
    CvSeqBlock*  block;      /* current block */                   \
    char*        ptr;        /* pointer to free space */           \
    char*        block_min;  /* pointer to the beginning of block*/\
    char*        block_max;  /* pointer to the end of block */

typedef struct CvSeqWriter
{
    CV_SEQ_WRITER_FIELDS()
    int  reserved[4]; /* some reserved fields */
}
CvSeqWriter;


#define CV_SEQ_READER_FIELDS()                                      \
    int          header_size;                                       \
    CvSeq*       seq;        /* sequence, beign read */             \
    CvSeqBlock*  block;      /* current block */                    \
    char*        ptr;        /* pointer to element be read next */  \
    char*        block_min;  /* pointer to the beginning of block */\
    char*        block_max;  /* pointer to the end of block */      \
    int          delta_index;/* = seq->first->start_index   */      \
    char*        prev_elem;  /* pointer to previous element */


typedef struct CvSeqReader
{
    CV_SEQ_READER_FIELDS()
    int  reserved[4];
}
CvSeqReader;

/****************************************************************************************/
/*                                Operations on sequences                               */
/****************************************************************************************/

#define  CV_GET_SEQ_ELEM( elem_type, seq, index )                \
/* assert gives some guarantee that <seq> parameter is valid */  \
(   assert(sizeof((seq)->first[0]) == sizeof(CvSeqBlock) &&      \
    (seq)->elem_size == sizeof(elem_type)),                      \
    (elem_type*)((seq)->first && (unsigned)index <               \
    (unsigned)((seq)->first->count) ?                            \
    (seq)->first->data + (index) * sizeof(elem_type) :           \
    cvGetSeqElem( (CvSeq*)(seq), (index), NULL )))


/* macro that adds element to sequence */
#define CV_WRITE_SEQ_ELEM_VAR( elem_ptr, writer )     \
{                                                     \
    if( (writer).ptr >= (writer).block_max )          \
    {                                                 \
        cvCreateSeqBlock( &writer);                   \
    }                                                 \
    memcpy((writer).ptr, elem_ptr, (writer).seq->elem_size);\
    (writer).ptr += (writer).seq->elem_size;          \
}

#define CV_WRITE_SEQ_ELEM( elem, writer )             \
{                                                     \
    assert( (writer).seq->elem_size == sizeof(elem)); \
    if( (writer).ptr >= (writer).block_max )          \
    {                                                 \
        cvCreateSeqBlock( &writer);                   \
    }                                                 \
    assert( (writer).ptr <= (writer).block_max - sizeof(elem));\
    memcpy((writer).ptr, &elem, sizeof(elem));        \
    (writer).ptr += sizeof(elem);                     \
}


/* move reader position forward */
#define CV_NEXT_SEQ_ELEM( elem_size, reader )                 \
{                                                             \
    if( ((reader).ptr += (elem_size)) >= (reader).block_max ) \
    {                                                         \
        cvChangeSeqBlock( &(reader), 1 );                     \
    }                                                         \
}


/* move reader position backward */
#define CV_PREV_SEQ_ELEM( elem_size, reader )                \
{                                                            \
    if( ((reader).ptr -= (elem_size)) < (reader).block_min ) \
    {                                                        \
        cvChangeSeqBlock( &(reader), -1 );                   \
    }                                                        \
}

/* read element and move read position forward */
#define CV_READ_SEQ_ELEM( elem, reader )                       \
{                                                              \
    assert( (reader).seq->elem_size == sizeof(elem));          \
    memcpy( &(elem), (reader).ptr, sizeof((elem)));            \
    CV_NEXT_SEQ_ELEM( sizeof(elem), reader )                   \
}

/* read element and move read position backward */
#define CV_REV_READ_SEQ_ELEM( elem, reader )                     \
{                                                                \
    assert( (reader).seq->elem_size == sizeof(elem));            \
    memcpy(&(elem), (reader).ptr, sizeof((elem)));               \
    CV_PREV_SEQ_ELEM( sizeof(elem), reader )                     \
}


#define CV_READ_CHAIN_POINT( _pt, reader )                              \
{                                                                       \
    (_pt) = (reader).pt;                                                \
    if( (reader).ptr )                                                  \
    {                                                                   \
        CV_READ_SEQ_ELEM( (reader).code, (*((CvSeqReader*)&(reader)))); \
        assert( ((reader).code & ~7) == 0 );                            \
        (reader).pt.x += (reader).deltas[(reader).code][0];             \
        (reader).pt.y += (reader).deltas[(reader).code][1];             \
    }                                                                   \
}

#define CV_CURRENT_POINT( reader )  (*((CvPoint*)((reader).ptr)))
#define CV_PREV_POINT( reader )     (*((CvPoint*)((reader).prev_elem)))

#define CV_READ_EDGE( pt1, pt2, reader )               \
{                                                      \
    assert( sizeof(pt1) == sizeof(CvPoint) &&          \
            sizeof(pt2) == sizeof(CvPoint) &&          \
            reader.seq->elem_size == sizeof(CvPoint)); \
    (pt1) = CV_PREV_POINT( reader );                   \
    (pt2) = CV_CURRENT_POINT( reader );                \
    (reader).prev_elem = (reader).ptr;                 \
    CV_NEXT_SEQ_ELEM( sizeof(CvPoint), (reader));      \
}

/************ Graph macros ************/

/* returns next graph edge for given vertex */
#define  CV_NEXT_GRAPH_EDGE( edge, vertex )                              \
     (assert((edge)->vtx[0] == (vertex) || (edge)->vtx[1] == (vertex)),  \
      (edge)->next[(edge)->vtx[1] == (vertex)])

/****************************************************************************************/

/**** For error processing and debugging purposes ******/ 
typedef struct
{
    const char* file;
    int         line;
}
CvStackRecord;                               


/****************************************************************************************/
/*                                    Older definitions                                 */
/****************************************************************************************/

typedef float*   CvVect32f;
typedef float*   CvMatr32f;
typedef double*  CvVect64d;
typedef double*  CvMatr64d;

typedef struct CvMatrix3
{
    float m[3][3];
}
CvMatrix3;

typedef enum CvStatus
{         
    CV_INPLACE_NOT_SUPPORTED_ERR= -112,
    CV_UNMATCHED_ROI_ERR        = -111,
    CV_NOTFOUND_ERR             = -110,
    CV_BADCONVERGENCE_ERR       = -109,

    CV_BADDEPTH_ERR             = -107,
    CV_BADROI_ERR               = -106,
    CV_BADHEADER_ERR            = -105,
    CV_UNMATCHED_FORMATS_ERR    = -104,
    CV_UNSUPPORTED_COI_ERR      = -103,
    CV_UNSUPPORTED_CHANNELS_ERR = -102,
    CV_UNSUPPORTED_DEPTH_ERR    = -101,
    CV_UNSUPPORTED_FORMAT_ERR   = -100,

                       
    CV_BADARG_ERR      = -49,  //ipp comp
    CV_NOTDEFINED_ERR  = -48,  //ipp comp

    CV_BADCHANNELS_ERR = -47,  //ipp comp
    CV_BADRANGE_ERR    = -44,  //ipp comp
    CV_BADSTEP_ERR     = -29,  //ipp comp

    CV_BADFLAG_ERR     =  -12,
    CV_DIV_BY_ZERO_ERR =  -11, //ipp comp
    CV_BADCOEF_ERR     =  -10,

    CV_BADFACTOR_ERR   =  -7,
    CV_BADPOINT_ERR    =  -6,
    CV_BADSCALE_ERR    =  -4,
    CV_OUTOFMEM_ERR    =  -3,
    CV_NULLPTR_ERR     =  -2,
    CV_BADSIZE_ERR     =  -1,
    CV_NO_ERR          =   0,
    CV_OK              =   CV_NO_ERR
}
CvStatus;

#endif /*_CVTYPES_H_*/

/* End of file. */
