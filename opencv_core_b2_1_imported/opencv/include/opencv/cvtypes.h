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
typedef long long _int64;
#endif

#ifndef __IPL_H__
typedef unsigned char uchar;
#endif

#define CV_SWAP(a,b,t) ((t) = (a), (a) = (b), (b) = (t))

#ifndef MIN
#define MIN(a,b)  ((a) > (b) ? (b) : (a))
#endif

#ifndef MAX
#define MAX(a,b)  ((a) < (b) ? (b) : (a))
#endif

/**********************************************************************************\
*                            Types of Matrix data                                  *
\**********************************************************************************/

#define CV_8U   0
#define CV_8S   1
#define CV_16S  2
#define CV_32S  3
#define CV_32F  4
#define CV_64F  5

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

#define IPL_DEPTH_64F  64

#define CV_AUTO_STEP  0x7fffffff
#define CV_WHOLE_ARR  cvSlice( 0, 0x3fffffff )

#define CV_ARR_CN_MASK        (3 << 3)
#define CV_ARR_CN(flags)      ((((flags) & CV_ARR_CN_MASK) >> 3) + 1)
#define CV_ARR_DEPTH_MASK     7
#define CV_ARR_DEPTH(flags)   ((flags) & CV_ARR_DEPTH_MASK)
#define CV_ARR_TYPE_MASK      31
#define CV_ARR_TYPE(flags)    ((flags) & CV_ARR_TYPE_MASK)
#define CV_ARR_FMT_MASK       511
#define CV_ARR_FMT(flags)     ((flags) & CV_ARR_FMT_MASK)
#define CV_ARR_CONT_FLAG_SHIFT 9
#define CV_ARR_CONT_FLAG      (1 << CV_ARR_CONT_FLAG_SHIFT)
#define CV_IS_ARR_CONT(flags) ((flags) & CV_ARR_CONT_FLAG) 

#define CV_ARR_MAGIC_MASK   0xFFFF0000 
#define CV_ARR_MAGIC_VAL    0x42240000

typedef struct CvMat
{
    int type;

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

    int step;
    union
    {
        float* fl;
        double* db;
        uchar* ptr;
    } data;
} CvMat;


#define _CV_IS_ARR(arr) \
    ((arr) != 0 && (((const CvMat*)(arr))->type & CV_ARR_MAGIC_MASK) == CV_ARR_MAGIC_VAL)

#define _CV_IS_IMAGE(img) \
    ((img) != 0 && ((const IplImage*)(img))->nSize == sizeof(IplImage))

#define CV_IS_ARR(arr) \
    (_CV_IS_ARR(arr) && ((const CvMat*)(arr))->data.ptr != 0)

#define CV_IS_IMAGE(img) \
    (_CV_IS_IMAGE(img) && ((IplImage*)img)->imageData != 0)

#define CV_IS_MASK_ARR(arr) \
    (((arr)->type & (CV_ARR_TYPE_MASK & ~CV_8SC1)) == 0)

#define CV_ARE_TYPES_EQ(arr1, arr2) \
    ( (((arr1)->type ^ (arr2)->type) & CV_ARR_TYPE_MASK) == 0 )

#define CV_ARE_CNS_EQ(arr1, arr2) \
    ( (((arr1)->type ^ (arr2)->type) & CV_ARR_CN_MASK) == 0 )

#define CV_ARE_DEPTHS_EQ(arr1, arr2) \
    ( (((arr1)->type ^ (arr2)->type) & CV_ARR_DEPTH_MASK) == 0 )

#define CV_ARE_SIZES_EQ(arr1, arr2) \
    ( (arr1)->height == (arr2)->height && (arr1)->width == (arr2)->width )

#define CV_IS_ARR_CONST(arr)  \
    ( ((arr)->height|(arr)->width) == 1 )

typedef void CvArr;

typedef struct CvRect
{
    int x;
    int y;
    int width;
    int height;
} CvRect;

typedef struct CvPoint
{
    int x;
    int y;
} CvPoint;

typedef struct {
    int width;
    int height;
} CvSize;


#define CV_TERMCRIT_ITER    1
#define CV_TERMCRIT_NUMB    CV_TERMCRIT_ITER
#define CV_TERMCRIT_EPS     2

typedef struct CvTermCriteria
{
    int   type;  /* may be combination of
                 CV_TERMCRIT_ITER
                 CV_TERMCRIT_EPS */
    int   maxIter;
    float epsilon;
} CvTermCriteria;


typedef struct CvPoint2D32f
{
    float x;
    float y;
} CvPoint2D32f;

typedef struct CvPoint3D32f
{
    float x;
    float y;
    float z;
} CvPoint3D32f;

typedef struct CvPoint2D64d
{
    double x;
    double y;
} CvPoint2D64d;

typedef struct CvPoint3D64d
{
    double x;
    double y;
    double z;
} CvPoint3D64d;

typedef struct CvSize2D32f
{
    float width;
    float height;
} CvSize2D32f;

typedef struct CvBox2D
{
    CvPoint2D32f center;
    CvSize2D32f  size;
    float angle;
} CvBox2D;

typedef enum CvCoeffType
{
    CV_VALUE = 1,
    CV_ARRAY = 2
} CvCoeffType;

typedef struct CvSlice
{
    int  startIndex, endIndex;
} CvSlice;

typedef struct CvScalar
{
    double val[4];
}
CvScalar;

CV_INLINE  int  cvPixSize( int matType );
CV_INLINE  int  cvPixSize( int matType )
{
    return (((0733100 >> CV_ARR_DEPTH( matType )*3) & 7) + 1)*CV_ARR_CN( matType );
}

CV_INLINE  CvMat  cvMat( int rows, int cols, int type, void* data CV_DEFAULT(0));
CV_INLINE  CvMat  cvMat( int rows, int cols, int type, void* data )
{
    CvMat m;

    assert( (unsigned)CV_ARR_DEPTH(type) <= CV_64F );
    m.type = CV_ARR_MAGIC_VAL | CV_ARR_CONT_FLAG | CV_ARR_TYPE(type);
    m.cols = cols;
    m.rows = rows;
    m.step = m.cols*cvPixSize( CV_ARR_TYPE(type) );
    m.data.ptr = (uchar*)data;

    return m; 
}


CV_INLINE  double  cvmGet( const CvMat* mat, int i, int j );
CV_INLINE  double  cvmGet( const CvMat* mat, int i, int j )
{
    int type;

    assert( (unsigned)i < (unsigned)(mat->rows) &&
            (unsigned)j < (unsigned)(mat->cols) );

    type = CV_ARR_TYPE(mat->type);
    if( type == CV_32FC1 )
        return ((float*)(mat->data.ptr + mat->step*i))[j];
    else
    {
        assert( type == CV_64FC1 );
        return ((double*)(mat->data.ptr + mat->step*i))[j];
    }
}


CV_INLINE  void  cvmSet( CvMat* mat, int i, int j, double val );
CV_INLINE  void  cvmSet( CvMat* mat, int i, int j, double val )
{
    int type;
    assert( (unsigned)i < (unsigned)(mat->rows) &&
            (unsigned)j < (unsigned)(mat->cols) );

    type = CV_ARR_TYPE(mat->type);
    if( type == CV_32FC1 )
        ((float*)(mat->data.ptr + mat->step*i))[j] = (float)val;
    else
    {
        assert( type == CV_64FC1 );
        ((double*)(mat->data.ptr + mat->step*i))[j] = val;
    }
}


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


CV_INLINE  CvSize  cvSize( int width, int height );
CV_INLINE  CvSize  cvSize( int width, int height )
{
    CvSize s;

    s.width = width;
    s.height = height;

    return s;
}


CV_INLINE  CvPoint  cvPoint( int x, int y );
CV_INLINE  CvPoint  cvPoint( int x, int y )
{
    CvPoint p;

    p.x = x;
    p.y = y;

    return p;
}


CV_INLINE  CvPoint2D32f  cvPoint2D32f( double x, double y );
CV_INLINE  CvPoint2D32f  cvPoint2D32f( double x, double y )
{
    CvPoint2D32f p;

    p.x = (float)x;
    p.y = (float)y;

    return p;
}

CV_INLINE  CvPoint3D32f  cvPoint3D32f( double x, double y, double z );
CV_INLINE  CvPoint3D32f  cvPoint3D32f( double x, double y, double z )
{
    CvPoint3D32f p;

    p.x = (float)x;
    p.y = (float)y;
    p.z = (float)z;

    return p;
}           

CV_INLINE  CvTermCriteria  cvTermCriteria( int type, int maxIter, double epsilon );
CV_INLINE  CvTermCriteria  cvTermCriteria( int type, int maxIter, double epsilon )
{
    CvTermCriteria t;

    t.type = type;
    t.maxIter = maxIter;
    t.epsilon = (float)epsilon;

    return t;
}


CV_INLINE  CvSlice  cvSlice( int start, int end );
CV_INLINE  CvSlice  cvSlice( int start, int end )
{
    CvSlice slice;
    slice.startIndex = start;
    slice.endIndex = end;

    return slice;
}

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


#define CV_WHOLE_SEQ  cvSlice(0, 0x3fffffff)

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
    CV_DIST_WELSCH       = 6 /* distance = c^2/2(1-exp(-(x/c)^2)), c = 2.9846 */
} CvDisType;


/* Filters used in pyramid decomposition */
typedef enum CvFilter
{
    CV_GAUSSIAN_5x5 = 7
} CvFilter;

#define CV_RGB( r, g, b )  (int)((uchar)(b) + ((uchar)(g) << 8) + ((uchar)(r) << 16))
#define CV_FILLED -1

/****************  segmented component's structure  *************************************/
typedef struct CvConnectedComp
{
    double area;          /*  area of the segmented component  */
    double value;          /*  gray scale value of the segmented component  */
    CvRect rect;        /*  ROI of the segmented component  */
} CvConnectedComp;

/****************************************************************************************\
*                         Histogram structures & defines                                 *
\****************************************************************************************/
#define CV_HIST_MAX_DIM 16

typedef enum CvHistType {
    CV_HIST_ARRAY = 0,
    CV_HIST_TREE  = 1
} CvHistType;

typedef enum CvCompareMethod {
    CV_COMP_CORREL = 0,
    CV_COMP_CHISQR = 1,
    CV_COMP_INTERSECT  = 2
} CvCompareMethod;

typedef enum CvHistFlag {
    CV_HIST_MEMALLOCATED    = 1,
    CV_HIST_HEADERALLOCATED = 2,
    CV_HIST_UNIFORM         = 4,
    CV_HIST_THRESHALLOCATED = 8
} CvHistFlag;

typedef struct CvHistogram
{
    int     header_size;    /* header's size */
    CvHistType type;           /* type of histogram */
    int     flags;
    int     c_dims;
    int     dims[CV_HIST_MAX_DIM];
    int     mdims[CV_HIST_MAX_DIM]; /* coefficients for fast calculation of number of element   */
                            /* &m[a,b,c] = m + a*mdims[0] + b*mdims[1] + c*mdims[2] */
    float*  thresh[CV_HIST_MAX_DIM];
    float*  array; /* all the histogram data, expanded into the single row */

    struct  CvNode* root;
    struct  CvSet*  set;
    int*    chdims[CV_HIST_MAX_DIM];
} CvHistogram;

/****************************************************************************************/
/*                                   Data structures                                    */
/****************************************************************************************/

/******************************** Memory storage ****************************************/

typedef struct CvMemBlock
{
    struct CvMemBlock*  prev;
    struct CvMemBlock*  next;
} CvMemBlock;

typedef struct CvMemStorage
{
    CvMemBlock* bottom;/* first allocated block */
    CvMemBlock* top;   /* current memory block - top of the stack */
    struct  CvMemStorage* parent; /* borrows new blocks from */
    int     block_size;  /* block size */
    int     free_space;  /* free space in the current block */
} CvMemStorage;


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
} CvSeqBlock;


/*
   Read/Write sequence.
   Elements can be dynamically inserted to or deleted from the sequence.
*/
#define CV_SEQUENCE_FIELDS()                                    \
    int       header_size;   /* size of sequence header */      \
    struct    CvSeq* h_prev; /* previous sequence */            \
    struct    CvSeq* h_next; /* next sequence */                \
    struct    CvSeq* v_prev; /* 2nd previous sequence */        \
    struct    CvSeq* v_next; /* 2nd next sequence */            \
    int       flags;          /* micsellaneous flags */                \
    int       total;          /* total number of elements */           \
    int       elem_size;      /* size of sequence element in bytes */  \
    char*     block_max;      /* maximal bound of the last block */    \
    char*     ptr;            /* current write pointer */              \
    int       delta_elems;    /* how many elements allocated when the seq grows */  \
    CvMemStorage* storage;  /* where the seq is stored */       \
    CvSeqBlock* free_blocks;  /* free blocks list */       \
    CvSeqBlock* first; /* pointer to the first sequence block */


typedef struct CvSeq
{
    CV_SEQUENCE_FIELDS()
} CvSeq;


/*************************************** Set ********************************************/
/*
  Set.
  Order isn't keeped. There can be gaps between sequence elements.
  After the element has been inserted it stays on the same place all the time.
  The LSB(least-significant bit) of the first field is 0 iff the element exists.
*/
#define CV_SET_ELEM_FIELDS()    \
    int*  aligned_ptr;

typedef struct CvSetElem
{
    CV_SET_ELEM_FIELDS()
}
CvSetElem;

#define CV_SET_FIELDS()      \
    CV_SEQUENCE_FIELDS()     \
    CvMemBlock* free_elems;

typedef struct CvSet
{
    CV_SET_FIELDS()
}
CvSet;

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
#define CV_GRAPH_EDGE_FIELDS()    \
    int flags;                    \
    struct CvGraphEdge* next[2];  \
    struct CvGraphVtx* vtx[2];
    

#define CV_GRAPH_VERTEX_FIELDS()  \
    int flags;                    \
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
} CvChain;

#define CV_CONTOUR_FIELDS()  \
    CV_SEQUENCE_FIELDS()     \
    CvRect rect;

typedef struct CvContour
{
    CV_CONTOUR_FIELDS()
} CvContour;

/****************************************************************************************\
*                                    Sequence types                                      *
\****************************************************************************************/

#define CV_SEQ_ELTYPE_BITS         5
#define CV_SEQ_ELTYPE_MASK         ((1 << CV_SEQ_ELTYPE_BITS) - 1)

#define CV_SEQ_ELTYPE_POINT          1  /* (x,y) */
#define CV_SEQ_ELTYPE_CODE           2  /* freeman code: 0..7 */
#define CV_SEQ_ELTYPE_PPOINT         3  /* &(x,y) */
#define CV_SEQ_ELTYPE_INDEX          4  /* #(x,y) */
#define CV_SEQ_ELTYPE_GRAPH_EDGE     5  /* &next_o, &next_d, &vtx_o, &vtx_d */
#define CV_SEQ_ELTYPE_GRAPH_VERTEX   6  /* first_edge, &(x,y) */
#define CV_SEQ_ELTYPE_TRIAN_ATR      7  /* vertex of the binary tree   */
#define CV_SEQ_ELTYPE_CONNECTED_COMP 8  /* connected component  */
#define CV_SEQ_ELTYPE_POINT3D        9  /* (x,y,z)  */

#define CV_SEQ_KIND_BITS      5
#define CV_SEQ_KIND_MASK       (((1 << CV_SEQ_KIND_BITS) - 1)<<CV_SEQ_ELTYPE_BITS)

#define CV_SEQ_KIND_SET        (0 << CV_SEQ_ELTYPE_BITS)
#define CV_SEQ_KIND_CURVE      (1 << CV_SEQ_ELTYPE_BITS)
#define CV_SEQ_KIND_BIN_TREE   (2 << CV_SEQ_ELTYPE_BITS)
#define CV_SEQ_KIND_GRAPH      (3 << CV_SEQ_ELTYPE_BITS)
#define CV_SEQ_KIND_SUBDIV2D   (4 << CV_SEQ_ELTYPE_BITS)

#define CV_SEQ_FLAG_SHIFT      (CV_SEQ_KIND_BITS + CV_SEQ_ELTYPE_BITS)

/* flags for curves */
#define CV_SEQ_FLAG_CLOSED     (1 << CV_SEQ_FLAG_SHIFT)
#define CV_SEQ_FLAG_SIMPLE     (2 << CV_SEQ_FLAG_SHIFT)
#define CV_SEQ_FLAG_CONVEX     (4 << CV_SEQ_FLAG_SHIFT)
#define CV_SEQ_FLAG_HOLE       (8 << CV_SEQ_FLAG_SHIFT)

/* flags for graphs */
#define CV_GRAPH_FLAG_ORIENTED (1 << CV_SEQ_FLAG_SHIFT)

/* flags for graph vertices and edges */
#define  CV_GRAPH_ITEM_VISITED_FLAG  (1 << 31)
#define  CV_IS_GRAPH_VERTEX_VISITED(vtx) \
    (((CvGraphVtx*)(vtx))->flags & CV_GRAPH_ITEM_VISITED_FLAG)
#define  CV_IS_GRAPH_EDGE_VISITED(edge) \
    (((CvGraphEdge*)(edge))->flags & CV_GRAPH_ITEM_VISITED_FLAG)

/* point sets */
#define CV_SEQ_POINT_SET       (CV_SEQ_KIND_SET    | CV_SEQ_ELTYPE_POINT)
#define CV_SEQ_POINT3D_SET     (CV_SEQ_KIND_SET    | CV_SEQ_ELTYPE_POINT3D)
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
#define CV_SEQ_CONNECTED_COMP  (CV_SEQ_KIND_SET  | CV_SEQ_ELTYPE_CONNECTED_COMP)

/* sequence of the integer numbers */
#define CV_SEQ_INDEX  (CV_SEQ_KIND_SET  | CV_SEQ_ELTYPE_INDEX)

/* type retrieving macros */
#define CV_SEQ_TYPE_BITS       CV_SEQ_FLAG_SHIFT
#define CV_SEQ_TYPE_MASK       ((1 << CV_SEQ_TYPE_BITS) - 1)

#define CV_SEQ_SUBTYPE_BITS    (CV_SEQ_TYPE_BITS + 10)
#define CV_SEQ_SUBTYPE_MASK    ((1 << (CV_SEQ_SUBTYPE_BITS) - 1)

#define CV_GET_SEQ_ELTYPE( seq )    ((seq)->flags & CV_SEQ_ELTYPE_MASK)
#define CV_GET_SEQ_KIND( seq )      ((seq)->flags & CV_SEQ_KIND_MASK )
#define CV_GET_SEQ_TYPE( seq )      ((seq)->flags & CV_SEQ_TYPE_MASK )
#define CV_GET_SEQ_SUBTYPE( seq )   ((seq)->flags & CV_SEQ_SUBTYPE_MASK )

/* flag checking */
#define CV_IS_SEQ_INDEX( seq )      ((CV_GET_SEQ_ELTYPE(seq) == CV_SEQ_ELTYPE_INDEX) && \
                                       (CV_GET_SEQ_KIND(seq) == CV_SEQ_KIND_SET))

#define CV_IS_SEQ_CURVE( seq )      (CV_GET_SEQ_KIND(seq) == CV_SEQ_KIND_CURVE)
#define CV_IS_SEQ_CLOSED( seq )     (((seq)->flags & CV_SEQ_FLAG_CLOSED) != 0)
#define CV_IS_SEQ_CONVEX( seq )     (((seq)->flags & CV_SEQ_FLAG_CONVEX) != 0)
#define CV_IS_SEQ_HOLE( seq )       (((seq)->flags & CV_SEQ_FLAG_HOLE) != 0)
#define CV_IS_SEQ_SIMPLE( seq )     ((((seq)->flags & CV_SEQ_FLAG_SIMPLE) != 0) || \
                                         CV_IS_CONVEX(seq)

/* type checking macros */
#define CV_IS_SEQ_POINT_SET( seq ) \
(CV_GET_SEQ_ELTYPE(seq) == CV_SEQ_ELTYPE_POINT)

#define CV_IS_SEQ_POINT_SUBSET( seq ) \
(CV_GET_SEQ_ELTYPE(seq) == CV_SEQ_ELTYPE_PPOINT)

#define CV_IS_SEQ_POLYLINE( seq )   \
(CV_GET_SEQ_TYPE(seq) == CV_SEQ_POLYLINE)

#define CV_IS_SEQ_POLYGON( seq )   \
(CV_IS_SEQ_POLYLINE(seq) && CV_IS_SEQ_CLOSED(seq))

#define CV_IS_SEQ_CHAIN( seq )   \
(CV_GET_SEQ_TYPE(seq) == CV_SEQ_CHAIN)

#define CV_IS_SEQ_CONTOUR( seq )   \
(CV_IS_SEQ_CLOSED(seq) && (CV_IS_SEQ_POLYLINE(seq) || CV_IS_SEQ_CHAIN(seq)))

#define CV_IS_SEQ_CHAIN_CONTOUR( seq ) \
(CV_IS_SEQ_CHAIN( seq ) && CV_IS_SEQ_CLOSED( seq ))

#define CV_IS_SEQ_POLYGON_TREE( seq ) \
    (CV_GET_SEQ_ELTYPE (seq) ==  CV_SEQ_ELTYPE_TRIAN_ATR &&    \
    CV_GET_SEQ_KIND( seq ) ==  CV_SEQ_KIND_BIN_TREE )

#define CV_IS_GRAPH( seq )    \
    ( CV_GET_SEQ_KIND(seq) == CV_SEQ_KIND_GRAPH )

#define CV_IS_GRAPH_ORIENTED( seq )   \
    ( CV_IS_GRAPH( seq ) && ((seq)->flags & CV_GRAPH_FLAG_ORIENTED) != 0 )

#define CV_IS_SUBDIV2D( seq )  \
    ( CV_GET_SEQ_KIND(seq) == CV_SEQ_KIND_SUBDIV2D )

/****************************************************************************************/
/*                            Sequence writer & reader                                  */
/****************************************************************************************/

#define CV_SEQ_WRITER_FIELDS()                                     \
    int          header_size;                                      \
    CvSeq*       seq;        /* sequence, beign written */         \
    CvSeqBlock*  block;      /* current block */                   \
    char*        ptr;        /* pointer to free space */           \
    char*        block_min;  /* address of end of block */         \
    char*        block_max;  /* address of end of block */

typedef struct CvSeqWriter
{
    CV_SEQ_WRITER_FIELDS()
    int  reserved[7]; /* some reserved fields */
} CvSeqWriter;


#define CV_SEQ_READER_FIELDS()                                      \
    int          header_size;                                       \
    CvSeq*       seq;        /* sequence, beign read */             \
    CvSeqBlock*  block;      /* current block */                    \
    char*        ptr;        /* pointer to element be read next */  \
    char*        block_max;  /* upper boundary of the block */      \
    char*        block_min;  /* lower boundary of the block */      \
    int          delta_index;/* = seq->first->start_index   */      \
    char*        prev_elem;  /* pointer to previous element */


typedef struct CvSeqReader
{
    CV_SEQ_READER_FIELDS()
    int  reserved[9];
} CvSeqReader;

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
    cvGetSeqElem( (CvSeq*)(seq), (index), 0 )))


/* macro that adds element to sequence */
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

/************ Set macros **************/
#define CV_IS_SET_ELEM_EXISTS( ptr )  (((*(long*)(ptr)) & 1) == 0)

#define CV_REMOVE_SET_ELEM( set, index, elem )                                 \
    (((CvMemBlock*)(elem))->prev = (CvMemBlock*)((long)((set)->free_elems)|1), \
    ((CvMemBlock*)(elem))->next = (CvMemBlock*)(index),                        \
    (set)->free_elems = (CvMemBlock*)(elem))

/************ Graph macros ************/

/* returns next graph edge for given vertex */
#define  CV_NEXT_GRAPH_EDGE( edge, vertex )                              \
     (assert((edge)->vtx[0] == (vertex) || (edge)->vtx[1] == (vertex)),  \
      (edge)->next[(edge)->vtx[1] == (vertex)])

/****************************************************************************************/
/*                                  Planar subdivisions                                 */
/****************************************************************************************/

typedef struct
{
    const char* file;
    int         line;
}CvStackRecord;                               


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
} CvStatus;



#endif /*_CVTYPES_H_*/

/* End of file. */
