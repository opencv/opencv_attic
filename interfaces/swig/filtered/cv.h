# 1 "../../cv/include/cv.h"
# 1 "<eingebaut>"
# 1 "<Kommandozeile>"
# 1 "../../cv/include/cv.h"
# 58 "../../cv/include/cv.h"
# 1 "../../cxcore/include/cxcore.h" 1
# 75 "../../cxcore/include/cxcore.h"
# 1 "../../cxcore/include/cxtypes.h" 1
# 119 "../../cxcore/include/cxtypes.h"
typedef long long int64;
typedef unsigned long long uint64;



typedef unsigned char uchar;
typedef unsigned short ushort;





typedef void CvArr;
# 165 "../../cxcore/include/cxtypes.h"
static int cvRound( double value );
static int cvRound( double value )
{
# 183 "../../cxcore/include/cxtypes.h"
    double temp = value + 6755399441055744.0;
    return (int)*((uint64*)&temp);

}


static int cvFloor( double value );
static int cvFloor( double value )
{
    int temp = cvRound(value);
    float diff = (float)(value - temp);

    return temp - (*(int*)&diff < 0);
}


static int cvCeil( double value );
static int cvCeil( double value )
{
    int temp = cvRound(value);
    float diff = (float)(temp - value);

    return temp + (*(int*)&diff < 0);
}




static int cvIsNaN( double value );
static int cvIsNaN( double value )
{





    unsigned lo = (unsigned)*(uint64*)&value;
    unsigned hi = (unsigned)(*(uint64*)&value >> 32);
    return (hi & 0x7fffffff) + (lo != 0) > 0x7ff00000;

}


static int cvIsInf( double value );
static int cvIsInf( double value )
{





    unsigned lo = (unsigned)*(uint64*)&value;
    unsigned hi = (unsigned)(*(uint64*)&value >> 32);
    return (hi & 0x7fffffff) == 0x7ff00000 && lo == 0;

}




typedef uint64 CvRNG;

static CvRNG cvRNG( int64 seed );
static CvRNG cvRNG( int64 seed )
{
    CvRNG rng = (uint64)(seed ? seed : (int64)-1);
    return rng;
}


static unsigned cvRandInt( CvRNG* rng );
static unsigned cvRandInt( CvRNG* rng )
{
    uint64 temp = *rng;
    temp = (uint64)(unsigned)temp*1554115554 + (temp >> 32);
    *rng = temp;
    return (unsigned)temp;
}


static double cvRandReal( CvRNG* rng );
static double cvRandReal( CvRNG* rng )
{
    return cvRandInt(rng)*2.3283064365386962890625e-10 ;
}
# 310 "../../cxcore/include/cxtypes.h"
typedef struct _IplImage
{
    int nSize;
    int ID;
    int nChannels;
    int alphaChannel;
    int depth;

    char colorModel[4];
    char channelSeq[4];
    int dataOrder;

    int origin;

    int align;

    int width;
    int height;
    struct _IplROI *roi;
    struct _IplImage *maskROI;
    void *imageId;
    struct _IplTileInfo *tileInfo;
    int imageSize;


    char *imageData;
    int widthStep;
    int BorderMode[4];
    int BorderConst[4];
    char *imageDataOrigin;


}
IplImage;

typedef struct _IplTileInfo IplTileInfo;

typedef struct _IplROI
{
    int coi;
    int xOffset;
    int yOffset;
    int width;
    int height;
}
IplROI;

typedef struct _IplConvKernel
{
    int nCols;
    int nRows;
    int anchorX;
    int anchorY;
    int *values;
    int nShiftR;
}
IplConvKernel;

typedef struct _IplConvKernelFP
{
    int nCols;
    int nRows;
    int anchorX;
    int anchorY;
    float *values;
}
IplConvKernelFP;
# 478 "../../cxcore/include/cxtypes.h"
typedef struct CvMat
{
    int type;
    int step;


    int* refcount;

    union
    {
        uchar* ptr;
        short* s;
        int* i;
        float* fl;
        double* db;
    } data;
# 508 "../../cxcore/include/cxtypes.h"
    int rows;
    int cols;


}
CvMat;
# 547 "../../cxcore/include/cxtypes.h"
static CvMat cvMat( int rows, int cols, int type, void* data );
static CvMat cvMat( int rows, int cols, int type, void* data )
{
    CvMat m;

    assert( (unsigned)((type) & ((1 << 3) - 1)) <= 6 );
    type = ((type) & ((1 << 3)*4 - 1));
    m.type = 0x42420000 | (1 << 9) | type;
    m.cols = cols;
    m.rows = rows;
    m.step = rows > 1 ? m.cols*(((((type) & ((4 - 1) << 3)) >> 3) + 1) << ((((sizeof(size_t)/4+1)*16384|0x3a50) >> ((type) & ((1 << 3) - 1))*2) & 3)) : 0;
    m.data.ptr = (uchar*)data;
    m.refcount = NULL;

    return m;
}
# 577 "../../cxcore/include/cxtypes.h"
static double cvmGet( const CvMat* mat, int row, int col );
static double cvmGet( const CvMat* mat, int row, int col )
{
    int type;

    type = ((mat->type) & ((1 << 3)*4 - 1));
    assert( (unsigned)row < (unsigned)mat->rows &&
            (unsigned)col < (unsigned)mat->cols );

    if( type == ((5) + (((1)-1) << 3)) )
        return ((float*)(mat->data.ptr + (size_t)mat->step*row))[col];
    else
    {
        assert( type == ((6) + (((1)-1) << 3)) );
        return ((double*)(mat->data.ptr + (size_t)mat->step*row))[col];
    }
}


static void cvmSet( CvMat* mat, int row, int col, double value );
static void cvmSet( CvMat* mat, int row, int col, double value )
{
    int type;
    type = ((mat->type) & ((1 << 3)*4 - 1));
    assert( (unsigned)row < (unsigned)mat->rows &&
            (unsigned)col < (unsigned)mat->cols );

    if( type == ((5) + (((1)-1) << 3)) )
        ((float*)(mat->data.ptr + (size_t)mat->step*row))[col] = (float)value;
    else
    {
        assert( type == ((6) + (((1)-1) << 3)) );
        ((double*)(mat->data.ptr + (size_t)mat->step*row))[col] = (double)value;
    }
}


static int cvCvToIplDepth( int type )
{
    int depth = ((type) & ((1 << 3) - 1));
    return (((((depth) & ((4 - 1) << 3)) >> 3) + 1) << ((((sizeof(size_t)/4+1)*16384|0x3a50) >> ((depth) & ((1 << 3) - 1))*2) & 3))*8 | (depth == 1 || depth == 3 ||
           depth == 4 ? 0x80000000 : 0);
}
# 632 "../../cxcore/include/cxtypes.h"
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
    dim[32];
}
CvMatND;
# 670 "../../cxcore/include/cxtypes.h"
struct CvSet;

typedef struct CvSparseMat
{
    int type;
    int dims;
    int* refcount;
    struct CvSet* heap;
    void** hashtable;
    int hashsize;
    int valoffset;
    int idxoffset;
    int size[32];
}
CvSparseMat;
# 695 "../../cxcore/include/cxtypes.h"
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
# 717 "../../cxcore/include/cxtypes.h"
typedef int CvHistType;
# 733 "../../cxcore/include/cxtypes.h"
typedef struct CvHistogram
{
    int type;
    CvArr* bins;
    float thresh[32][2];
    float** thresh2;
    CvMatND mat;
}
CvHistogram;
# 763 "../../cxcore/include/cxtypes.h"
typedef struct CvRect
{
    int x;
    int y;
    int width;
    int height;
}
CvRect;

static CvRect cvRect( int x, int y, int width, int height );
static CvRect cvRect( int x, int y, int width, int height )
{
    CvRect r;

    r.x = x;
    r.y = y;
    r.width = width;
    r.height = height;

    return r;
}


static IplROI cvRectToROI( CvRect rect, int coi );
static IplROI cvRectToROI( CvRect rect, int coi )
{
    IplROI roi;
    roi.xOffset = rect.x;
    roi.yOffset = rect.y;
    roi.width = rect.width;
    roi.height = rect.height;
    roi.coi = coi;

    return roi;
}


static CvRect cvROIToRect( IplROI roi );
static CvRect cvROIToRect( IplROI roi )
{
    return cvRect( roi.xOffset, roi.yOffset, roi.width, roi.height );
}







typedef struct CvTermCriteria
{
    int type;


    int max_iter;
    double epsilon;
}
CvTermCriteria;

static CvTermCriteria cvTermCriteria( int type, int max_iter, double epsilon );
static CvTermCriteria cvTermCriteria( int type, int max_iter, double epsilon )
{
    CvTermCriteria t;

    t.type = type;
    t.max_iter = max_iter;
    t.epsilon = (float)epsilon;

    return t;
}




typedef struct CvPoint
{
    int x;
    int y;
}
CvPoint;


static CvPoint cvPoint( int x, int y );
static CvPoint cvPoint( int x, int y )
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


static CvPoint2D32f cvPoint2D32f( double x, double y );
static CvPoint2D32f cvPoint2D32f( double x, double y )
{
    CvPoint2D32f p;

    p.x = (float)x;
    p.y = (float)y;

    return p;
}


static CvPoint2D32f cvPointTo32f( CvPoint point );
static CvPoint2D32f cvPointTo32f( CvPoint point )
{
    return cvPoint2D32f( (float)point.x, (float)point.y );
}


static CvPoint cvPointFrom32f( CvPoint2D32f point );
static CvPoint cvPointFrom32f( CvPoint2D32f point )
{
    CvPoint ipt;
    ipt.x = cvRound(point.x);
    ipt.y = cvRound(point.y);

    return ipt;
}


typedef struct CvPoint3D32f
{
    float x;
    float y;
    float z;
}
CvPoint3D32f;


static CvPoint3D32f cvPoint3D32f( double x, double y, double z );
static CvPoint3D32f cvPoint3D32f( double x, double y, double z )
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




typedef struct
{
    int width;
    int height;
}
CvSize;

static CvSize cvSize( int width, int height );
static CvSize cvSize( int width, int height )
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


static CvSize2D32f cvSize2D32f( double width, double height );
static CvSize2D32f cvSize2D32f( double width, double height )
{
    CvSize2D32f s;

    s.width = (float)width;
    s.height = (float)height;

    return s;
}

typedef struct CvBox2D
{
    CvPoint2D32f center;
    CvSize2D32f size;
    float angle;

}
CvBox2D;



typedef struct CvSlice
{
    int start_index, end_index;
}
CvSlice;

static CvSlice cvSlice( int start, int end );
static CvSlice cvSlice( int start, int end )
{
    CvSlice slice;
    slice.start_index = start;
    slice.end_index = end;

    return slice;
}







typedef struct CvScalar
{
    double val[4];
}
CvScalar;

static CvScalar cvScalar( double val0, double val1 ,
                                double val2 , double val3 );
static CvScalar cvScalar( double val0, double val1, double val2, double val3 )
{
    CvScalar scalar;
    scalar.val[0] = val0; scalar.val[1] = val1;
    scalar.val[2] = val2; scalar.val[3] = val3;
    return scalar;
}


static CvScalar cvRealScalar( double val0 );
static CvScalar cvRealScalar( double val0 )
{
    CvScalar scalar;
    scalar.val[0] = val0;
    scalar.val[1] = scalar.val[2] = scalar.val[3] = 0;
    return scalar;
}

static CvScalar cvScalarAll( double val0123 );
static CvScalar cvScalarAll( double val0123 )
{
    CvScalar scalar;
    scalar.val[0] = scalar.val[1] = scalar.val[2] = scalar.val[3] = val0123;
    return scalar;
}







typedef struct CvMemBlock
{
    struct CvMemBlock* prev;
    struct CvMemBlock* next;
}
CvMemBlock;



typedef struct CvMemStorage
{
    int signature;
    CvMemBlock* bottom;
    CvMemBlock* top;
    struct CvMemStorage* parent;
    size_t block_size;
    size_t free_space;
}
CvMemStorage;






typedef struct CvMemStoragePos
{
    CvMemBlock* top;
    size_t free_space;
}
CvMemStoragePos;




typedef struct CvSeqBlock
{
    struct CvSeqBlock* prev;
    struct CvSeqBlock* next;
    int start_index;

    int count;
    char* data;
}
CvSeqBlock;
# 1116 "../../cxcore/include/cxtypes.h"
typedef struct CvSeq
{
    int flags; int header_size; struct CvSeq* h_prev; struct CvSeq* h_next; struct CvSeq* v_prev; struct CvSeq* v_next; int total; int elem_size; char* block_max; char* ptr; int delta_elems; CvMemStorage* storage; CvSeqBlock* free_blocks; CvSeqBlock* first;
}
CvSeq;
# 1136 "../../cxcore/include/cxtypes.h"
typedef struct CvSetElem
{
    int flags; struct CvSetElem* next_free;
}
CvSetElem;






typedef struct CvSet
{
    int flags; int header_size; struct CvSeq* h_prev; struct CvSeq* h_next; struct CvSeq* v_prev; struct CvSeq* v_next; int total; int elem_size; char* block_max; char* ptr; int delta_elems; CvMemStorage* storage; CvSeqBlock* free_blocks; CvSeqBlock* first; CvSetElem* free_elems; int active_count;
}
CvSet;
# 1189 "../../cxcore/include/cxtypes.h"
typedef struct CvGraphEdge
{
    int flags; float weight; struct CvGraphEdge* next[2]; struct CvGraphVtx* vtx[2];
}
CvGraphEdge;

typedef struct CvGraphVtx
{
    int flags; struct CvGraphEdge* first;
}
CvGraphVtx;

typedef struct CvGraphVtx2D
{
    int flags; struct CvGraphEdge* first;
    CvPoint2D32f* ptr;
}
CvGraphVtx2D;
# 1216 "../../cxcore/include/cxtypes.h"
typedef struct CvGraph
{
    int flags; int header_size; struct CvSeq* h_prev; struct CvSeq* h_next; struct CvSeq* v_prev; struct CvSeq* v_next; int total; int elem_size; char* block_max; char* ptr; int delta_elems; CvMemStorage* storage; CvSeqBlock* free_blocks; CvSeqBlock* first; CvSetElem* free_elems; int active_count; CvSet* edges;
}
CvGraph;





typedef struct CvChain
{
    int flags; int header_size; struct CvSeq* h_prev; struct CvSeq* h_next; struct CvSeq* v_prev; struct CvSeq* v_next; int total; int elem_size; char* block_max; char* ptr; int delta_elems; CvMemStorage* storage; CvSeqBlock* free_blocks; CvSeqBlock* first;
    CvPoint origin;
}
CvChain;







typedef struct CvContour
{
    int flags; int header_size; struct CvSeq* h_prev; struct CvSeq* h_next; struct CvSeq* v_prev; struct CvSeq* v_next; int total; int elem_size; char* block_max; char* ptr; int delta_elems; CvMemStorage* storage; CvSeqBlock* free_blocks; CvSeqBlock* first; CvRect rect; int color; int reserved[3];
}
CvContour;

typedef CvContour CvPoint2DSeq;
# 1383 "../../cxcore/include/cxtypes.h"
typedef struct CvSeqWriter
{
    int header_size; CvSeq* seq; CvSeqBlock* block; char* ptr; char* block_min; char* block_max;
    int reserved[4];
}
CvSeqWriter;
# 1402 "../../cxcore/include/cxtypes.h"
typedef struct CvSeqReader
{
    int header_size; CvSeq* seq; CvSeqBlock* block; char* ptr; char* block_min; char* block_max; int delta_index; char* prev_elem;
    int reserved[4];
}
CvSeqReader;
# 1523 "../../cxcore/include/cxtypes.h"
typedef struct CvFileStorage CvFileStorage;
# 1532 "../../cxcore/include/cxtypes.h"
typedef struct CvAttrList
{
    const char** attr;
    struct CvAttrList* next;
}
CvAttrList;

static CvAttrList cvAttrList( const char** attr ,
                                 CvAttrList* next );
static CvAttrList cvAttrList( const char** attr, CvAttrList* next )
{
    CvAttrList l;
    l.attr = attr;
    l.next = next;

    return l;
}

struct CvTypeInfo;
# 1586 "../../cxcore/include/cxtypes.h"
typedef struct CvString
{
    int len;
    char* ptr;
}
CvString;



typedef struct CvStringHashNode
{
    unsigned hashval;
    CvString str;
    struct CvStringHashNode* next;
}
CvStringHashNode;

typedef struct CvGenericHash CvFileNodeHash;


typedef struct CvFileNode
{
    int tag;
    struct CvTypeInfo* info;

    union
    {
        double f;
        int i;
        CvString str;
        CvSeq* seq;
        CvFileNodeHash* map;
    } data;
}
CvFileNode;




typedef int ( *CvIsInstanceFunc)( const void* struct_ptr );
typedef void ( *CvReleaseFunc)( void** struct_dblptr );
typedef void* ( *CvReadFunc)( CvFileStorage* storage, CvFileNode* node );
typedef void ( *CvWriteFunc)( CvFileStorage* storage, const char* name,
                                      const void* struct_ptr, CvAttrList attributes );
typedef void* ( *CvCloneFunc)( const void* struct_ptr );




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




typedef struct CvPluginFuncInfo
{
    void** func_addr;
    void* default_func_addr;
    const char* func_names;
    int search_modules;
    int loaded_from;
}
CvPluginFuncInfo;

typedef struct CvModuleInfo
{
    struct CvModuleInfo* next;
    const char* name;
    const char* version;
    CvPluginFuncInfo* func_tab;
}
CvModuleInfo;
# 76 "../../cxcore/include/cxcore.h" 2
# 1 "../../cxcore/include/cxerror.h" 1
# 47 "../../cxcore/include/cxerror.h"
typedef int CVStatus;
# 77 "../../cxcore/include/cxcore.h" 2
# 90 "../../cxcore/include/cxcore.h"
 void* cvAlloc( size_t size );







 void cvFree( void** ptr );


 IplImage* cvCreateImageHeader( CvSize size, int depth, int channels );


 IplImage* cvInitImageHeader( IplImage* image, CvSize size, int depth,
                                   int channels, int origin ,
                                   int align );


 IplImage* cvCreateImage( CvSize size, int depth, int channels );


 void cvReleaseImageHeader( IplImage** image );


 void cvReleaseImage( IplImage** image );


 IplImage* cvCloneImage( const IplImage* image );



 void cvSetImageCOI( IplImage* image, int coi );


 int cvGetImageCOI( const IplImage* image );


 void cvSetImageROI( IplImage* image, CvRect rect );


 void cvResetImageROI( IplImage* image );


 CvRect cvGetImageROI( const IplImage* image );


 CvMat* cvCreateMatHeader( int rows, int cols, int type );




 CvMat* cvInitMatHeader( CvMat* mat, int rows, int cols,
                              int type, void* data ,
                              int step );


 CvMat* cvCreateMat( int rows, int cols, int type );



 void cvReleaseMat( CvMat** mat );



static void cvDecRefData( CvArr* arr );
static void cvDecRefData( CvArr* arr )
{
    if( (((arr) != NULL && (((const CvMat*)(arr))->type & 0xFFFF0000) == 0x42420000) && ((const CvMat*)(arr))->data.ptr != NULL) || (((arr) != NULL && (((const CvMatND*)(arr))->type & 0xFFFF0000) == 0x42430000) && ((const CvMatND*)(arr))->data.ptr != NULL))
    {
        CvMat* mat = (CvMat*)arr;
        mat->data.ptr = NULL;
        if( mat->refcount != NULL && --*mat->refcount == 0 )
            cvFree( (void**)&mat->refcount );
        mat->refcount = NULL;
    }
}


static int cvIncRefData( CvArr* arr );
static int cvIncRefData( CvArr* arr )
{
    int refcount = 0;
    if( (((arr) != NULL && (((const CvMat*)(arr))->type & 0xFFFF0000) == 0x42420000) && ((const CvMat*)(arr))->data.ptr != NULL) || (((arr) != NULL && (((const CvMatND*)(arr))->type & 0xFFFF0000) == 0x42430000) && ((const CvMatND*)(arr))->data.ptr != NULL))
    {
        CvMat* mat = (CvMat*)arr;
        if( mat->refcount != NULL )
            refcount = ++*mat->refcount;
    }
    return refcount;
}



 CvMat* cvCloneMat( const CvMat* mat );




 CvMat* cvGetSubRect( const CvArr* arr, CvMat* submat, CvRect rect );




 CvMat* cvGetRows( const CvArr* arr, CvMat* submat,
                        int start_row, int end_row,
                        int delta_row );

static CvMat* cvGetRow( const CvArr* arr, CvMat* submat, int row );
static CvMat* cvGetRow( const CvArr* arr, CvMat* submat, int row )
{
    return cvGetRows( arr, submat, row, row + 1, 1 );
}




 CvMat* cvGetCols( const CvArr* arr, CvMat* submat,
                        int start_col, int end_col );

static CvMat* cvGetCol( const CvArr* arr, CvMat* submat, int col );
static CvMat* cvGetCol( const CvArr* arr, CvMat* submat, int col )
{
    return cvGetCols( arr, submat, col, col + 1 );
}





 CvMat* cvGetDiag( const CvArr* arr, CvMat* submat,
                            int diag );


 void cvScalarToRawData( const CvScalar* scalar, void* data, int type,
                              int extend_to_12 );

 void cvRawDataToScalar( const void* data, int type, CvScalar* scalar );


 CvMatND* cvCreateMatNDHeader( int dims, const int* sizes, int type );


 CvMatND* cvCreateMatND( int dims, const int* sizes, int type );


 CvMatND* cvInitMatNDHeader( CvMatND* mat, int dims, const int* sizes,
                                    int type, void* data );


static void cvReleaseMatND( CvMatND** mat );
static void cvReleaseMatND( CvMatND** mat )
{
    cvReleaseMat( (CvMat**)mat );
}


 CvMatND* cvCloneMatND( const CvMatND* mat );


 CvSparseMat* cvCreateSparseMat( int dims, const int* sizes, int type );


 void cvReleaseSparseMat( CvSparseMat** mat );


 CvSparseMat* cvCloneSparseMat( const CvSparseMat* mat );



 CvSparseNode* cvInitSparseMatIterator( const CvSparseMat* mat,
                                              CvSparseMatIterator* mat_iterator );


static CvSparseNode* cvGetNextSparseNode( CvSparseMatIterator* mat_iterator );
static CvSparseNode* cvGetNextSparseNode( CvSparseMatIterator* mat_iterator )
{
    if( mat_iterator->node->next )
        return mat_iterator->node = mat_iterator->node->next;
    else
    {
        int idx;
        for( idx = ++mat_iterator->curidx; idx < mat_iterator->mat->hashsize; idx++ )
        {
            CvSparseNode* node = (CvSparseNode*)mat_iterator->mat->hashtable[idx];
            if( node )
            {
                mat_iterator->curidx = idx;
                return mat_iterator->node = node;
            }
        }
        return NULL;
    }
}





typedef struct CvNArrayIterator
{
    int count;
    int dims;
    CvSize size;
    uchar* ptr[10];
    int stack[32];
    CvMatND* hdr[10];

}
CvNArrayIterator;
# 308 "../../cxcore/include/cxcore.h"
 int cvInitNArrayIterator( int count, CvArr** arrs,
                                 const CvArr* mask, CvMatND* stubs,
                                 CvNArrayIterator* array_iterator,
                                 int flags );


 int cvNextNArraySlice( CvNArrayIterator* array_iterator );




 int cvGetElemType( const CvArr* arr );



 int cvGetDims( const CvArr* arr, int* sizes );





 int cvGetDimSize( const CvArr* arr, int index );




 uchar* cvPtr1D( const CvArr* arr, int idx0, int* type );
 uchar* cvPtr2D( const CvArr* arr, int idx0, int idx1, int* type );
 uchar* cvPtr3D( const CvArr* arr, int idx0, int idx1, int idx2,
                      int* type );





 uchar* cvPtrND( const CvArr* arr, int* idx, int* type ,
                      int create_node ,
                      unsigned* precalc_hashval );


 CvScalar cvGet1D( const CvArr* arr, int idx0 );
 CvScalar cvGet2D( const CvArr* arr, int idx0, int idx1 );
 CvScalar cvGet3D( const CvArr* arr, int idx0, int idx1, int idx2 );
 CvScalar cvGetND( const CvArr* arr, int* idx );


 double cvGetReal1D( const CvArr* arr, int idx0 );
 double cvGetReal2D( const CvArr* arr, int idx0, int idx1 );
 double cvGetReal3D( const CvArr* arr, int idx0, int idx1, int idx2 );
 double cvGetRealND( const CvArr* arr, int* idx );


 void cvSet1D( CvArr* arr, int idx0, CvScalar value );
 void cvSet2D( CvArr* arr, int idx0, int idx1, CvScalar value );
 void cvSet3D( CvArr* arr, int idx0, int idx1, int idx2, CvScalar value );
 void cvSetND( CvArr* arr, int* idx, CvScalar value );


 void cvSetReal1D( CvArr* arr, int idx0, double value );
 void cvSetReal2D( CvArr* arr, int idx0, int idx1, double value );
 void cvSetReal3D( CvArr* arr, int idx0,
                        int idx1, int idx2, double value );
 void cvSetRealND( CvArr* arr, int* idx, double value );



 void cvClearND( CvArr* arr, int* idx );






 CvMat* cvGetMat( const CvArr* arr, CvMat* header,
                       int* coi ,
                       int allowND );


 IplImage* cvGetImage( const CvArr* arr, IplImage* image_header );
# 399 "../../cxcore/include/cxcore.h"
 CvArr* cvReshapeMatND( const CvArr* arr,
                             int sizeof_header, CvArr* header,
                             int new_cn, int new_dims, int* new_sizes );





 CvMat* cvReshape( const CvArr* arr, CvMat* header,
                        int new_cn, int new_rows );



 void cvRepeat( const CvArr* src, CvArr* dst );


 void cvCreateData( CvArr* arr );


 void cvReleaseData( CvArr* arr );




 void cvSetData( CvArr* arr, void* data, int step );




 void cvGetRawData( const CvArr* arr, uchar** data,
                         int* step ,
                         CvSize* roi_size );


 CvSize cvGetSize( const CvArr* arr );


 void cvCopy( const CvArr* src, CvArr* dst,
                     const CvArr* mask );



 void cvSet( CvArr* arr, CvScalar value,
                    const CvArr* mask );


 void cvSetZero( CvArr* arr );





 void cvSplit( const CvArr* src, CvArr* dst0, CvArr* dst1,
                      CvArr* dst2, CvArr* dst3 );



 void cvMerge( const CvArr* src0, const CvArr* src1,
                      const CvArr* src2, const CvArr* src3,
                      CvArr* dst );






 void cvConvertScale( const CvArr* src, CvArr* dst,
                             double scale ,
                             double shift );
# 478 "../../cxcore/include/cxcore.h"
 void cvConvertScaleAbs( const CvArr* src, CvArr* dst,
                                double scale ,
                                double shift );







 CvTermCriteria cvCheckTermCriteria( CvTermCriteria criteria,
                                           double default_eps,
                                           int default_max_iters );






 void cvAdd( const CvArr* src1, const CvArr* src2, CvArr* dst,
                    const CvArr* mask );


 void cvAddS( const CvArr* src, CvScalar value, CvArr* dst,
                     const CvArr* mask );


 void cvSub( const CvArr* src1, const CvArr* src2, CvArr* dst,
                    const CvArr* mask );


static void cvSubS( const CvArr* src, CvScalar value, CvArr* dst,
                         const CvArr* mask );
static void cvSubS( const CvArr* src, CvScalar value, CvArr* dst,
                         const CvArr* mask )
{
    cvAddS( src, cvScalar( -value.val[0], -value.val[1], -value.val[2], -value.val[3]),
            dst, mask );
}


 void cvSubRS( const CvArr* src, CvScalar value, CvArr* dst,
                      const CvArr* mask );



 void cvMul( const CvArr* src1, const CvArr* src2,
                    CvArr* dst, double scale );




 void cvDiv( const CvArr* src1, const CvArr* src2,
                    CvArr* dst, double scale );


 void cvScaleAdd( const CvArr* src1, CvScalar scale,
                         const CvArr* src2, CvArr* dst );



 void cvAddWeighted( const CvArr* src1, double alpha,
                            const CvArr* src2, double beta,
                            double gamma, CvArr* dst );


 double cvDotProduct( const CvArr* src1, const CvArr* src2 );


 void cvAnd( const CvArr* src1, const CvArr* src2,
                  CvArr* dst, const CvArr* mask );


 void cvAndS( const CvArr* src, CvScalar value,
                   CvArr* dst, const CvArr* mask );


 void cvOr( const CvArr* src1, const CvArr* src2,
                 CvArr* dst, const CvArr* mask );


 void cvOrS( const CvArr* src, CvScalar value,
                  CvArr* dst, const CvArr* mask );


 void cvXor( const CvArr* src1, const CvArr* src2,
                  CvArr* dst, const CvArr* mask );


 void cvXorS( const CvArr* src, CvScalar value,
                   CvArr* dst, const CvArr* mask );


 void cvNot( const CvArr* src, CvArr* dst );


 void cvInRange( const CvArr* src, const CvArr* lower,
                      const CvArr* upper, CvArr* dst );


 void cvInRangeS( const CvArr* src, CvScalar lower,
                       CvScalar upper, CvArr* dst );
# 592 "../../cxcore/include/cxcore.h"
 void cvCmp( const CvArr* src1, const CvArr* src2, CvArr* dst, int cmp_op );


 void cvCmpS( const CvArr* src, double value, CvArr* dst, int cmp_op );


 void cvMin( const CvArr* src1, const CvArr* src2, CvArr* dst );


 void cvMax( const CvArr* src1, const CvArr* src2, CvArr* dst );


 void cvMinS( const CvArr* src, double value, CvArr* dst );


 void cvMaxS( const CvArr* src, double value, CvArr* dst );


 void cvAbsDiff( const CvArr* src1, const CvArr* src2, CvArr* dst );


 void cvAbsDiffS( const CvArr* src, CvArr* dst, CvScalar value );
# 622 "../../cxcore/include/cxcore.h"
 void cvCartToPolar( const CvArr* x, const CvArr* y,
                            CvArr* magnitude, CvArr* angle ,
                            int angle_in_degrees );




 void cvPolarToCart( const CvArr* magnitude, const CvArr* angle,
                            CvArr* x, CvArr* y,
                            int angle_in_degrees );


 void cvPow( const CvArr* src, CvArr* dst, double power );




 void cvExp( const CvArr* src, CvArr* dst );





 void cvLog( const CvArr* src, CvArr* dst );


 float cvFastArctan( float y, float x );


 float cvCbrt( float value );







 int cvCheckArr( const CvArr* arr, int flags ,
                        double min_val , double max_val );




 void cvRandArr( CvRNG* rng, CvArr* arr, int dist_type,
                      CvScalar param1, CvScalar param2 );






 void cvCrossProduct( const CvArr* src1, const CvArr* src2, CvArr* dst );
# 684 "../../cxcore/include/cxcore.h"
 void cvGEMM( const CvArr* src1, const CvArr* src2, double alpha,
                     const CvArr* src3, double beta, CvArr* dst,
                     int tABC );




 void cvTransform( const CvArr* src, CvArr* dst,
                          const CvMat* transmat,
                          const CvMat* shiftvec );



 void cvPerspectiveTransform( const CvArr* src, CvArr* dst,
                                     const CvMat* mat );


 void cvMulTransposed( const CvArr* src, CvArr* dst, int order,
                            const CvArr* delta );


 void cvTranspose( const CvArr* src, CvArr* dst );






 void cvFlip( const CvArr* src, CvArr* dst ,
                     int flip_mode );
# 722 "../../cxcore/include/cxcore.h"
 void cvSVD( CvArr* A, CvArr* W, CvArr* U ,
                     CvArr* V , int flags );



 void cvSVBkSb( const CvArr* W, const CvArr* U,
                        const CvArr* V, const CvArr* B,
                        CvArr* X, int flags );





 double cvInvert( const CvArr* src, CvArr* dst,
                         int method );




 int cvSolve( const CvArr* src1, const CvArr* src2, CvArr* dst,
                     int method );


 double cvDet( const CvArr* mat );


 CvScalar cvTrace( const CvArr* mat );


 void cvEigenVV( CvArr* mat, CvArr* evects,
                        CvArr* evals, double eps );


 void cvSetIdentity( CvArr* mat, CvScalar value );
# 771 "../../cxcore/include/cxcore.h"
 void cvCalcCovarMatrix( const CvArr** vects, int count,
                                CvArr* cov_mat, CvArr* avg, int flags );


 double cvMahalanobis( const CvArr* vec1, const CvArr* vec2, CvArr* mat );







 CvScalar cvSum( const CvArr* arr );


 int cvCountNonZero( const CvArr* arr );


 CvScalar cvAvg( const CvArr* arr, const CvArr* mask );


 void cvAvgSdv( const CvArr* arr, CvScalar* mean, CvScalar* std_dev,
                       const CvArr* mask );


 void cvMinMaxLoc( const CvArr* arr, double* min_val, double* max_val,
                          CvPoint* min_loc ,
                          CvPoint* max_loc ,
                          const CvArr* mask );
# 817 "../../cxcore/include/cxcore.h"
 double cvNorm( const CvArr* arr1, const CvArr* arr2 ,
                       int norm_type ,
                       const CvArr* mask );
# 837 "../../cxcore/include/cxcore.h"
 void cvDFT( const CvArr* src, CvArr* dst, int flags,
                    int nonzero_rows );



 void cvMulSpectrums( const CvArr* src1, const CvArr* src2,
                             CvArr* dst, int flags );


 int cvGetOptimalDFTSize( int size0 );


 void cvDCT( const CvArr* src, CvArr* dst, int flags );






 int cvSliceLength( CvSlice slice, const CvSeq* seq );





 CvMemStorage* cvCreateMemStorage( int block_size );



 CvMemStorage* cvCreateChildMemStorage( CvMemStorage* parent );




 void cvReleaseMemStorage( CvMemStorage** storage );






 void cvClearMemStorage( CvMemStorage* storage );


 void cvSaveMemStoragePos( const CvMemStorage* storage, CvMemStoragePos* pos );


 void cvRestoreMemStoragePos( CvMemStorage* storage, CvMemStoragePos* pos );


 void* cvMemStorageAlloc( CvMemStorage* storage, size_t size );


 CvString cvMemStorageAllocString( CvMemStorage* storage, const char* ptr,
                                        int len );


 CvSeq* cvCreateSeq( int seq_flags, int header_size,
                            int elem_size, CvMemStorage* storage );



 void cvSetSeqBlockSize( CvSeq* seq, int delta_elems );



 char* cvSeqPush( CvSeq* seq, void* element );



 char* cvSeqPushFront( CvSeq* seq, void* element );



 void cvSeqPop( CvSeq* seq, void* element );



 void cvSeqPopFront( CvSeq* seq, void* element );





 void cvSeqPushMulti( CvSeq* seq, void* elements,
                             int count, int in_front );


 void cvSeqPopMulti( CvSeq* seq, void* elements,
                            int count, int in_front );



 char* cvSeqInsert( CvSeq* seq, int before_index,
                           void* element );


 void cvSeqRemove( CvSeq* seq, int index );





 void cvClearSeq( CvSeq* seq );





 char* cvGetSeqElem( const CvSeq* seq, int index );



 int cvSeqElemIdx( const CvSeq* seq, const void* element,
                         CvSeqBlock** block );


 void cvStartAppendToSeq( CvSeq* seq, CvSeqWriter* writer );



 void cvStartWriteSeq( int seq_flags, int header_size,
                              int elem_size, CvMemStorage* storage,
                              CvSeqWriter* writer );





 CvSeq* cvEndWriteSeq( CvSeqWriter* writer );




 void cvFlushSeqWriter( CvSeqWriter* writer );




 void cvStartReadSeq( const CvSeq* seq, CvSeqReader* reader,
                           int reverse );



 int cvGetSeqReaderPos( CvSeqReader* reader );




 void cvSetSeqReaderPos( CvSeqReader* reader, int index,
                                 int is_relative );


 void* cvCvtSeqToArray( const CvSeq* seq, void* elements,
                               CvSlice slice );




 CvSeq* cvMakeSeqHeaderForArray( int seq_type, int header_size,
                                       int elem_size, void* elements, int total,
                                       CvSeq* seq, CvSeqBlock* block );


 CvSeq* cvSeqSlice( const CvSeq* seq, CvSlice slice,
                         CvMemStorage* storage ,
                         int copy_data );

static CvSeq* cvCloneSeq( const CvSeq* seq, CvMemStorage* storage );
static CvSeq* cvCloneSeq( const CvSeq* seq, CvMemStorage* storage )
{
    return cvSeqSlice( seq, cvSlice(0, 0x3fffffff), storage, 1 );
}


 void cvSeqRemoveSlice( CvSeq* seq, CvSlice slice );


 void cvSeqInsertSlice( CvSeq* seq, int before_index, const CvArr* from_arr );


typedef int (* CvCmpFunc)(const void* a, const void* b, void* userdata );


 void cvSeqSort( CvSeq* seq, CvCmpFunc func, void* userdata );


 char* cvSeqSearch( CvSeq* seq, const void* elem, CvCmpFunc func,
                          int is_sorted, int* elem_idx,
                          void* userdata );


 void cvSeqInvert( CvSeq* seq );


 int cvSeqPartition( const CvSeq* seq, CvMemStorage* storage,
                            CvSeq** labels, CvCmpFunc is_equal, void* userdata );


 void cvChangeSeqBlock( CvSeqReader* reader, int direction );
 void cvCreateSeqBlock( CvSeqWriter* writer );



 CvSet* cvCreateSet( int set_flags, int header_size,
                            int elem_size, CvMemStorage* storage );


 int cvSetAdd( CvSet* set_header, CvSetElem* elem ,
                      CvSetElem** inserted_elem );


static CvSetElem* cvSetNew( CvSet* set_header );
static CvSetElem* cvSetNew( CvSet* set_header )
{
    CvSetElem* elem = set_header->free_elems;
    if( elem )
    {
        set_header->free_elems = elem->next_free;
        elem->flags = elem->flags & ((1 << 24) - 1);
        set_header->active_count++;
    }
    else
        cvSetAdd( set_header, NULL, (CvSetElem**)&elem );
    return elem;
}


static void cvSetRemoveByPtr( CvSet* set_header, void* elem );
static void cvSetRemoveByPtr( CvSet* set_header, void* elem )
{
    CvSetElem* _elem = (CvSetElem*)elem;
    assert( _elem->flags >= 0 );
    _elem->next_free = set_header->free_elems;
    _elem->flags = (_elem->flags & ((1 << 24) - 1)) | (1 << (sizeof(int)*8-1));
    set_header->free_elems = _elem;
    set_header->active_count--;
}


 void cvSetRemove( CvSet* set_header, int index );



static CvSetElem* cvGetSetElem( const CvSet* set_header, int index );
static CvSetElem* cvGetSetElem( const CvSet* set_header, int index )
{
    CvSetElem* elem = (CvSetElem*)cvGetSeqElem( (CvSeq*)set_header, index );
    return elem && (((CvSetElem*)(elem))->flags >= 0) ? elem : 0;
}


 void cvClearSet( CvSet* set_header );


 CvGraph* cvCreateGraph( int graph_flags, int header_size,
                                int vtx_size, int edge_size,
                                CvMemStorage* storage );


 int cvGraphAddVtx( CvGraph* graph, const CvGraphVtx* vtx ,
                           CvGraphVtx** inserted_vtx );



 int cvGraphRemoveVtx( CvGraph* graph, int index );
 int cvGraphRemoveVtxByPtr( CvGraph* graph, CvGraphVtx* vtx );






 int cvGraphAddEdge( CvGraph* graph,
                            int start_idx, int end_idx,
                            const CvGraphEdge* edge ,
                            CvGraphEdge** inserted_edge );

 int cvGraphAddEdgeByPtr( CvGraph* graph,
                               CvGraphVtx* start_vtx, CvGraphVtx* end_vtx,
                               const CvGraphEdge* edge ,
                               CvGraphEdge** inserted_edge );


 void cvGraphRemoveEdge( CvGraph* graph, int start_idx, int end_idx );
 void cvGraphRemoveEdgeByPtr( CvGraph* graph, CvGraphVtx* start_vtx,
                                     CvGraphVtx* end_vtx );


 CvGraphEdge* cvFindGraphEdge( const CvGraph* graph, int start_idx, int end_idx );
 CvGraphEdge* cvFindGraphEdgeByPtr( const CvGraph* graph,
                                           const CvGraphVtx* start_vtx,
                                           const CvGraphVtx* end_vtx );




 void cvClearGraph( CvGraph* graph );



 int cvGraphVtxDegree( const CvGraph* graph, int vtx_idx );
 int cvGraphVtxDegreeByPtr( const CvGraph* graph, const CvGraphVtx* vtx );
# 1175 "../../cxcore/include/cxcore.h"
typedef struct CvGraphScanner
{
    CvGraphVtx* vtx;
    CvGraphVtx* dst;
    CvGraphEdge* edge;

    CvGraph* graph;
    CvSeq* stack;
    int index;
    int mask;
}
CvGraphScanner;


 CvGraphScanner* cvCreateGraphScanner( CvGraph* graph,
                                             CvGraphVtx* vtx ,
                                             int mask );


 void cvReleaseGraphScanner( CvGraphScanner** scanner );


 int cvNextGraphItem( CvGraphScanner* scanner );


 CvGraph* cvCloneGraph( const CvGraph* graph, CvMemStorage* storage );
# 1222 "../../cxcore/include/cxcore.h"
 void cvLine( CvArr* img, CvPoint pt1, CvPoint pt2,
                     CvScalar color, int thickness ,
                     int line_type , int shift );



 void cvRectangle( CvArr* img, CvPoint pt1, CvPoint pt2,
                          CvScalar color, int thickness ,
                          int line_type ,
                          int shift );



 void cvCircle( CvArr* img, CvPoint center, int radius,
                       CvScalar color, int thickness ,
                       int line_type , int shift );




 void cvEllipse( CvArr* img, CvPoint center, CvSize axes,
                        double angle, double start_angle, double end_angle,
                        CvScalar color, int thickness ,
                        int line_type , int shift );

static void cvEllipseBox( CvArr* img, CvBox2D box, CvScalar color,
                               int thickness ,
                               int line_type , int shift );
static void cvEllipseBox( CvArr* img, CvBox2D box, CvScalar color,
                               int thickness, int line_type, int shift )
{
    CvSize axes;
    axes.width = cvRound(box.size.height*0.5);
    axes.height = cvRound(box.size.width*0.5);

    cvEllipse( img, cvPointFrom32f( box.center ), axes, box.angle*180/3.1415926535897932384626433832795,
               0, 360, color, thickness, line_type, shift );
}


 void cvFillConvexPoly( CvArr* img, CvPoint* pts, int npts, CvScalar color,
                               int line_type , int shift );


 void cvFillPoly( CvArr* img, CvPoint** pts, int* npts, int contours, CvScalar color,
                         int line_type , int shift );


 void cvPolyLine( CvArr* img, CvPoint** pts, int* npts, int contours,
                         int is_closed, CvScalar color, int thickness ,
                         int line_type , int shift );
# 1290 "../../cxcore/include/cxcore.h"
typedef struct CvFont
{
    int font_face;
    const int* ascii;
    const int* greek;
    const int* cyrillic;
    float hscale, vscale;
    float shear;
    int thickness;
    float dx;
    int line_type;
}
CvFont;


 void cvInitFont( CvFont* font, int font_face,
                         double hscale, double vscale,
                         double shear ,
                         int thickness ,
                         int line_type );



 void cvPutText( CvArr* img, const char* text, CvPoint org,
                        const CvFont* font, CvScalar color );


 void cvGetTextSize( const char* text_string, const CvFont* font,
                            CvSize* text_size, int* baseline );




 CvScalar cvColorToScalar( double packed_color, int arrtype );


 void cvDrawContours( CvArr *img, CvSeq* contour,
                            CvScalar external_color, CvScalar hole_color,
                            int max_level, int thickness ,
                            int line_type );



 void cvLUT( const CvArr* src, CvArr* dst, const CvArr* lut );



typedef struct CvTreeNodeIterator
{
    const void* node;
    int level;
    int max_level;
}
CvTreeNodeIterator;

 void cvInitTreeNodeIterator( CvTreeNodeIterator* tree_iterator,
                                   const void* first, int max_level );
 void* cvNextTreeNode( CvTreeNodeIterator* tree_iterator );
 void* cvPrevTreeNode( CvTreeNodeIterator* tree_iterator );




 void cvInsertNodeIntoTree( void* node, void* parent, void* frame );


 void cvRemoveNodeFromTree( void* node, void* frame );



 CvSeq* cvTreeToNodeSeq( const void* first, int header_size,
                              CvMemStorage* storage );



 void cvKMeans2( const CvArr* samples, int cluster_count,
                        CvArr* labels, CvTermCriteria termcrit );






 int cvRegisterModule( const CvModuleInfo* module_info );


 int cvUseOptimized( int on_off );


 void cvGetModuleInfo( const char* module_name,
                              const char** version,
                              const char** loaded_addon_plugins );


 int cvGetErrStatus( void );


 void cvSetErrStatus( int status );






 int cvGetErrMode( void );


 int cvSetErrMode( int mode );




 void cvError( int status, const char* func_name,
                    const char* err_msg, const char* file_name, int line );


 const char* cvErrorStr( int status );


 int cvGetErrInfo( const char** errcode_desc, const char** description,
                        const char** filename, int* line );


 int cvErrorFromIppStatus( int ipp_status );

typedef int ( *CvErrorCallback)( int status, const char* func_name,
                    const char* err_msg, const char* file_name, int line, void* userdata );


 CvErrorCallback cvRedirectError( CvErrorCallback error_handler,
                                       void* userdata ,
                                       void** prev_userdata );







 int cvNulDevReport( int status, const char* func_name, const char* err_msg,
                          const char* file_name, int line, void* userdata );

 int cvStdErrReport( int status, const char* func_name, const char* err_msg,
                          const char* file_name, int line, void* userdata );

 int cvGuiBoxReport( int status, const char* func_name, const char* err_msg,
                          const char* file_name, int line, void* userdata );

typedef void* ( *CvAllocFunc)(size_t size, void* userdata);
typedef int ( *CvFreeFunc)(void* pptr, void* userdata);



 void cvSetMemoryManager( CvAllocFunc alloc_func ,
                               CvFreeFunc free_func ,
                               void* userdata );


typedef IplImage* (* Cv_iplCreateImageHeader)
                            (int,int,int,char*,char*,int,int,int,int,int,
                            IplROI*,IplImage*,void*,IplTileInfo*);
typedef void (* Cv_iplAllocateImageData)(IplImage*,int,int);
typedef void (* Cv_iplDeallocate)(IplImage*,int);
typedef IplROI* (* Cv_iplCreateROI)(int,int,int,int,int);
typedef IplImage* (* Cv_iplCloneImage)(const IplImage*);


 void cvSetIPLAllocators( Cv_iplCreateImageHeader create_header,
                               Cv_iplAllocateImageData allocate_data,
                               Cv_iplDeallocate deallocate,
                               Cv_iplCreateROI create_roi,
                               Cv_iplCloneImage clone_image );
# 1474 "../../cxcore/include/cxcore.h"
 CvFileStorage* cvOpenFileStorage( const char* filename,
                                          CvMemStorage* memstorage,
                                          int flags );


 void cvReleaseFileStorage( CvFileStorage** fs );


 const char* cvAttrValue( const CvAttrList* attr, const char* attr_name );


 void cvStartWriteStruct( CvFileStorage* fs, const char* name,
                                int struct_flags, const char* type_name ,
                                CvAttrList attributes );


 void cvEndWriteStruct( CvFileStorage* fs );


 void cvWriteInt( CvFileStorage* fs, const char* name, int value );


 void cvWriteReal( CvFileStorage* fs, const char* name, double value );


 void cvWriteString( CvFileStorage* fs, const char* name,
                           const char* str, int quote );


 void cvWriteComment( CvFileStorage* fs, const char* comment,
                            int eol_comment );



 void cvWrite( CvFileStorage* fs, const char* name, const void* ptr,
                         CvAttrList attributes );


 void cvStartNextStream( CvFileStorage* fs );


 void cvWriteRawData( CvFileStorage* fs, const void* src,
                                int len, const char* dt );



 CvStringHashNode* cvGetHashedKey( CvFileStorage* fs, const char* name,
                                        int len ,
                                        int create_missing );



 CvFileNode* cvGetRootFileNode( const CvFileStorage* fs,
                                     int stream_index );



 CvFileNode* cvGetFileNode( CvFileStorage* fs, CvFileNode* map,
                                 const CvStringHashNode* key,
                                 int create_missing );


 CvFileNode* cvGetFileNodeByName( const CvFileStorage* fs,
                                       const CvFileNode* map,
                                       const char* name );

static int cvReadInt( const CvFileNode* node, int default_value );
static int cvReadInt( const CvFileNode* node, int default_value )
{
    return !node ? default_value :
        (((node->tag) & 7) == 1) ? node->data.i :
        (((node->tag) & 7) == 2) ? cvRound(node->data.f) : 0x7fffffff;
}


static int cvReadIntByName( const CvFileStorage* fs, const CvFileNode* map,
                         const char* name, int default_value );
static int cvReadIntByName( const CvFileStorage* fs, const CvFileNode* map,
                         const char* name, int default_value )
{
    return cvReadInt( cvGetFileNodeByName( fs, map, name ), default_value );
}


static double cvReadReal( const CvFileNode* node, double default_value );
static double cvReadReal( const CvFileNode* node, double default_value )
{
    return !node ? default_value :
        (((node->tag) & 7) == 1) ? (double)node->data.i :
        (((node->tag) & 7) == 2) ? node->data.f : 1e300;
}


static double cvReadRealByName( const CvFileStorage* fs, const CvFileNode* map,
                        const char* name, double default_value );
static double cvReadRealByName( const CvFileStorage* fs, const CvFileNode* map,
                        const char* name, double default_value )
{
    return cvReadReal( cvGetFileNodeByName( fs, map, name ), default_value );
}


static const char* cvReadString( const CvFileNode* node,
                        const char* default_value );
static const char* cvReadString( const CvFileNode* node,
                        const char* default_value )
{
    return !node ? default_value : (((node->tag) & 7) == 3) ? node->data.str.ptr : 0;
}


static const char* cvReadStringByName( const CvFileStorage* fs, const CvFileNode* map,
                        const char* name, const char* default_value );
static const char* cvReadStringByName( const CvFileStorage* fs, const CvFileNode* map,
                        const char* name, const char* default_value )
{
    return cvReadString( cvGetFileNodeByName( fs, map, name ), default_value );
}



 void* cvRead( CvFileStorage* fs, CvFileNode* node,
                        CvAttrList* attributes );


static void* cvReadByName( CvFileStorage* fs, const CvFileNode* map,
                              const char* name, CvAttrList* attributes );
static void* cvReadByName( CvFileStorage* fs, const CvFileNode* map,
                              const char* name, CvAttrList* attributes )
{
    return cvRead( fs, cvGetFileNodeByName( fs, map, name ), attributes );
}



 void cvStartReadRawData( const CvFileStorage* fs, const CvFileNode* src,
                               CvSeqReader* reader );


 void cvReadRawDataSlice( const CvFileStorage* fs, CvSeqReader* reader,
                               int count, void* dst, const char* dt );


 void cvReadRawData( const CvFileStorage* fs, const CvFileNode* src,
                          void* dst, const char* dt );


 void cvWriteFileNode( CvFileStorage* fs, const char* new_node_name,
                            const CvFileNode* node, int embed );


 const char* cvGetFileNodeName( const CvFileNode* node );



 void cvRegisterType( const CvTypeInfo* info );
 void cvUnregisterType( const char* type_name );
 CvTypeInfo* cvFirstType(void);
 CvTypeInfo* cvFindType( const char* type_name );
 CvTypeInfo* cvTypeOf( const void* struct_ptr );


 void cvRelease( void** struct_ptr );
 void* cvClone( const void* struct_ptr );


 void cvSave( const char* filename, const void* struct_ptr,
                    const char* name ,
                    const char* comment ,
                    CvAttrList attributes );
 void* cvLoad( const char* filename,
                     CvMemStorage* memstorage ,
                     const char* name ,
                     const char** real_name );





 int64 cvGetTickCount( void );
 double cvGetTickFrequency( void );
# 59 "../../cv/include/cv.h" 2
# 1 "../../cv/include/cvtypes.h" 1
# 51 "../../cv/include/cvtypes.h"
typedef struct CvMoments
{
    double m00, m10, m01, m20, m11, m02, m30, m21, m12, m03;
    double mu20, mu11, mu02, mu30, mu21, mu12, mu03;
    double inv_sqrt_m00;
}
CvMoments;


typedef struct CvHuMoments
{
    double hu1, hu2, hu3, hu4, hu5, hu6, hu7;
}
CvHuMoments;


typedef struct CvLineIterator
{
    uchar* ptr;
    int err;
    int plus_delta;
    int minus_delta;
    int plus_step;
    int minus_step;
}
CvLineIterator;



typedef struct CvConnectedComp
{
    double area;
    CvScalar value;
    CvRect rect;
    CvSeq* contour;

}
CvConnectedComp;





typedef struct _CvContourScanner* CvContourScanner;
# 111 "../../cv/include/cvtypes.h"
typedef struct CvChainPtReader
{
    int header_size; CvSeq* seq; CvSeqBlock* block; char* ptr; char* block_min; char* block_max; int delta_index; char* prev_elem;
    char code;
    CvPoint pt;
    char deltas[8][2];
    int reserved[2];
}
CvChainPtReader;
# 129 "../../cv/include/cvtypes.h"
typedef struct CvContourTree
{
    int flags; int header_size; struct CvSeq* h_prev; struct CvSeq* h_next; struct CvSeq* v_prev; struct CvSeq* v_next; int total; int elem_size; char* block_max; char* ptr; int delta_elems; CvMemStorage* storage; CvSeqBlock* free_blocks; CvSeqBlock* first;
    CvPoint p1;
    CvPoint p2;
}
CvContourTree;


typedef struct CvConvexityDefect
{
    CvPoint* start;
    CvPoint* end;
    CvPoint* depth_point;
    float depth;
}
CvConvexityDefect;



typedef size_t CvSubdiv2DEdge;
# 163 "../../cv/include/cvtypes.h"
typedef struct CvQuadEdge2D
{
    int flags; struct CvSubdiv2DPoint* pt[4]; CvSubdiv2DEdge next[4];
}
CvQuadEdge2D;

typedef struct CvSubdiv2DPoint
{
    int flags; CvSubdiv2DEdge first; CvPoint2D32f pt;
}
CvSubdiv2DPoint;
# 183 "../../cv/include/cvtypes.h"
typedef struct CvSubdiv2D
{
    int flags; int header_size; struct CvSeq* h_prev; struct CvSeq* h_next; struct CvSeq* v_prev; struct CvSeq* v_next; int total; int elem_size; char* block_max; char* ptr; int delta_elems; CvMemStorage* storage; CvSeqBlock* free_blocks; CvSeqBlock* first; CvSetElem* free_elems; int active_count; CvSet* edges; int quad_edges; int is_geometry_valid; CvSubdiv2DEdge recent_edge; CvPoint2D32f topleft; CvPoint2D32f bottomright;
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
    CV_NEXT_AROUND_ORG = 0x00,
    CV_NEXT_AROUND_DST = 0x22,
    CV_PREV_AROUND_ORG = 0x11,
    CV_PREV_AROUND_DST = 0x33,
    CV_NEXT_AROUND_LEFT = 0x13,
    CV_NEXT_AROUND_RIGHT = 0x31,
    CV_PREV_AROUND_LEFT = 0x20,
    CV_PREV_AROUND_RIGHT = 0x02
}
CvNextEdgeType;
# 229 "../../cv/include/cvtypes.h"
typedef enum CvFilter
{
    CV_GAUSSIAN_5x5 = 7
}
CvFilter;





typedef float* CvVect32f;
typedef float* CvMatr32f;
typedef double* CvVect64d;
typedef double* CvMatr64d;

typedef struct CvMatrix3
{
    float m[3][3];
}
CvMatrix3;






typedef float ( * CvDistanceFunction)( const float* a, const float* b, void* user_param );





typedef struct CvConDensation
{
    int MP;
    int DP;
    float* DynamMatr;
    float* State;
    int SamplesNum;
    float** flSamples;
    float** flNewSamples;
    float* flConfidence;
    float* flCumulative;
    float* Temp;
    float* RandomSample;
    struct CvRandState* RandS;
}
CvConDensation;







typedef struct CvKalman
{
    int MP;
    int DP;
    int CP;



    float* PosterState;
    float* PriorState;
    float* DynamMatr;
    float* MeasurementMatr;
    float* MNCovariance;
    float* PNCovariance;
    float* KalmGainMatr;
    float* PriorErrorCovariance;
    float* PosterErrorCovariance;
    float* Temp1;
    float* Temp2;


    CvMat* state_pre;

    CvMat* state_post;

    CvMat* transition_matrix;
    CvMat* control_matrix;

    CvMat* measurement_matrix;
    CvMat* process_noise_cov;
    CvMat* measurement_noise_cov;
    CvMat* error_cov_pre;

    CvMat* gain;

    CvMat* error_cov_post;

    CvMat* temp1;
    CvMat* temp2;
    CvMat* temp3;
    CvMat* temp4;
    CvMat* temp5;
}
CvKalman;
# 340 "../../cv/include/cvtypes.h"
typedef struct CvHaarFeature
{
    int tilted;
    struct
    {
        CvRect r;
        float weight;
    } rect[3];
}
CvHaarFeature;

typedef struct CvHaarClassifier
{
    int count;
    CvHaarFeature* haar_feature;
    float* threshold;
    int* left;
    int* right;
    float* alpha;
}
CvHaarClassifier;

typedef struct CvHaarStageClassifier
{
    int count;
    float threshold;
    CvHaarClassifier* classifier;

    int next;
    int child;
    int parent;
}
CvHaarStageClassifier;

typedef struct CvHidHaarClassifierCascade CvHidHaarClassifierCascade;

typedef struct CvHaarClassifierCascade
{
    int flags;
    int count;
    CvSize orig_window_size;
    CvSize real_window_size;
    double scale;
    CvHaarStageClassifier* stage_classifier;
    CvHidHaarClassifierCascade* hid_cascade;
}
CvHaarClassifierCascade;

typedef struct CvAvgComp
{
    CvRect rect;
    int neighbors;
}
CvAvgComp;
# 60 "../../cv/include/cv.h" 2
# 71 "../../cv/include/cv.h"
 void cvCopyMakeBorder( const CvArr* src, CvArr* dst, CvPoint offset,
                              int bordertype, CvScalar value );
# 81 "../../cv/include/cv.h"
 void cvSmooth( const CvArr* src, CvArr* dst,
                      int smoothtype ,
                      int param1 ,
                      int param2 ,
                      double param3 );


 void cvFilter2D( const CvArr* src, CvArr* dst, const CvMat* kernel,
                        CvPoint anchor );



 void cvIntegral( const CvArr* image, CvArr* sum,
                       CvArr* sqsum ,
                       CvArr* tilted_sum );






 void cvPyrDown( const CvArr* src, CvArr* dst,
                        int filter );






 void cvPyrUp( const CvArr* src, CvArr* dst,
                      int filter );
# 123 "../../cv/include/cv.h"
 void cvPyrSegmentation( IplImage* src,
                              IplImage* dst,
                              CvMemStorage* storage,
                              CvSeq** comp,
                              int level, double threshold1,
                              double threshold2 );







 void cvSobel( const CvArr* src, CvArr* dst,
                    int xorder, int yorder,
                    int aperture_size );


 void cvLaplace( const CvArr* src, CvArr* dst,
                      int aperture_size );
# 240 "../../cv/include/cv.h"
 void cvCvtColor( const CvArr* src, CvArr* dst, int code );
# 251 "../../cv/include/cv.h"
 void cvResize( const CvArr* src, CvArr* dst,
                       int interpolation );


 void cvWarpAffine( const CvArr* src, CvArr* dst, const CvMat* map_matrix,
                           int flags ,
                           CvScalar fillval );


 CvMat* cv2DRotationMatrix( CvPoint2D32f center, double angle,
                                   double scale, CvMat* map_matrix );


 void cvWarpPerspective( const CvArr* src, CvArr* dst, const CvMat* map_matrix,
                                int flags ,
                                CvScalar fillval );


 CvMat* cvWarpPerspectiveQMatrix( const CvPoint2D32f* src,
                                       const CvPoint2D32f* dst,
                                       CvMat* map_matrix );


 void cvRemap( const CvArr* src, CvArr* dst,
                      const CvArr* mapx, const CvArr* mapy,
                      int flags ,
                      CvScalar fillval );

 void cvLogPolar( const CvArr* src, CvArr* dst,
                         CvPoint2D32f center, double M,
                         int flags );







 IplConvKernel* cvCreateStructuringElementEx(
            int cols, int rows, int anchor_x, int anchor_y,
            int shape, int* values );


 void cvReleaseStructuringElement( IplConvKernel** element );




 void cvErode( const CvArr* src, CvArr* dst,
                      IplConvKernel* element ,
                      int iterations );



 void cvDilate( const CvArr* src, CvArr* dst,
                       IplConvKernel* element ,
                       int iterations );
# 316 "../../cv/include/cv.h"
 void cvMorphologyEx( const CvArr* src, CvArr* dst,
                             CvArr* temp, IplConvKernel* element,
                             int operation, int iterations );



 void cvMoments( const CvArr* arr, CvMoments* moments, int binary );


 double cvGetSpatialMoment( CvMoments* moments, int x_order, int y_order );
 double cvGetCentralMoment( CvMoments* moments, int x_order, int y_order );
 double cvGetNormalizedCentralMoment( CvMoments* moments,
                                             int x_order, int y_order );


 void cvGetHuMoments( CvMoments* moments, CvHuMoments* hu_moments );





 int cvInitLineIterator( const CvArr* image, CvPoint pt1, CvPoint pt2,
                                CvLineIterator* line_iterator,
                                int connectivity );
# 353 "../../cv/include/cv.h"
 int cvSampleLine( const CvArr* image, CvPoint pt1, CvPoint pt2, void* buffer,
                          int connectivity );




 void cvGetRectSubPix( const CvArr* src, CvArr* dst, CvPoint2D32f center );







 void cvGetQuadrangleSubPix( const CvArr* src, CvArr* dst,
                                    const CvMat* map_matrix,
                                    int fill_outliers ,
                                    CvScalar fill_value );
# 382 "../../cv/include/cv.h"
 void cvMatchTemplate( const CvArr* image, const CvArr* templ,
                              CvArr* result, int method );



 float cvCalcEMD2( const CvArr* signature1,
                          const CvArr* signature2,
                          int distance_type,
                          CvDistanceFunction distance_func ,
                          const CvArr* cost_matrix ,
                          CvArr* flow ,
                          float* lower_bound ,
                          void* userdata );







 int cvFindContours( CvArr* image, CvMemStorage* storage, CvSeq** first_contour,
                            int header_size ,
                            int mode ,
                            int method ,
                            CvPoint offset );







 CvContourScanner cvStartFindContours( CvArr* image, CvMemStorage* storage,
                            int header_size ,
                            int mode ,
                            int method ,
                            CvPoint offset );


 CvSeq* cvFindNextContour( CvContourScanner scanner );




 void cvSubstituteContour( CvContourScanner scanner, CvSeq* new_contour );



 CvSeq* cvEndFindContours( CvContourScanner* scanner );


 CvSeq* cvApproxChains( CvSeq* src_seq, CvMemStorage* storage,
                            int method ,
                            double parameter ,
                            int minimal_perimeter ,
                            int recursive );





 void cvStartReadChainPoints( CvChain* chain, CvChainPtReader* reader );


 CvPoint cvReadChainPoint( CvChainPtReader* reader );
# 456 "../../cv/include/cv.h"
 void cvCalcOpticalFlowLK( const CvArr* prev, const CvArr* curr,
                                  CvSize win_size, CvArr* velx, CvArr* vely );


 void cvCalcOpticalFlowBM( const CvArr* prev, const CvArr* curr,
                                  CvSize block_size, CvSize shift_size,
                                  CvSize max_range, int use_previous,
                                  CvArr* velx, CvArr* vely );


 void cvCalcOpticalFlowHS( const CvArr* prev, const CvArr* curr,
                                  int use_previous, CvArr* velx, CvArr* vely,
                                  double lambda, CvTermCriteria criteria );
# 479 "../../cv/include/cv.h"
 void cvCalcOpticalFlowPyrLK( const CvArr* prev, const CvArr* curr,
                                     CvArr* prev_pyr, CvArr* curr_pyr,
                                     const CvPoint2D32f* prev_features,
                                     CvPoint2D32f* curr_features,
                                     int count,
                                     CvSize win_size,
                                     int level,
                                     char* status,
                                     float* track_error,
                                     CvTermCriteria criteria,
                                     int flags );
# 516 "../../cv/include/cv.h"
 void cvUpdateMotionHistory( const CvArr* silhouette, CvArr* mhi,
                                      double timestamp, double duration );



 void cvCalcMotionGradient( const CvArr* mhi, CvArr* mask, CvArr* orientation,
                                     double delta1, double delta2,
                                     int aperture_size );




 double cvCalcGlobalOrientation( const CvArr* orientation, const CvArr* mask,
                                        const CvArr* mhi, double timestamp,
                                        double duration );



 CvSeq* cvSegmentMotion( const CvArr* mhi, CvArr* seg_mask,
                                CvMemStorage* storage,
                                double timestamp, double seg_thresh );




 void cvAcc( const CvArr* image, CvArr* sum,
                    const CvArr* mask );


 void cvSquareAcc( const CvArr* image, CvArr* sqsum,
                          const CvArr* mask );


 void cvMultiplyAcc( const CvArr* image1, const CvArr* image2, CvArr* acc,
                            const CvArr* mask );


 void cvRunningAvg( const CvArr* image, CvArr* acc, double alpha,
                           const CvArr* mask );
# 563 "../../cv/include/cv.h"
 int cvCamShift( const CvArr* prob_image, CvRect window,
                       CvTermCriteria criteria, CvConnectedComp* comp,
                       CvBox2D* box );



 int cvMeanShift( const CvArr* prob_image, CvRect window,
                        CvTermCriteria criteria, CvConnectedComp* comp );


 CvConDensation* cvCreateConDensation( int dynam_params,
                                             int measure_params,
                                             int sample_count );


 void cvReleaseConDensation( CvConDensation** condens );


 void cvConDensUpdateByTime( CvConDensation* condens);


 void cvConDensInitSampleSet( CvConDensation* condens, CvMat* lower_bound, CvMat* upper_bound );


 CvKalman* cvCreateKalman( int dynam_params, int measure_params,
                                int control_params );


 void cvReleaseKalman( CvKalman** kalman);


 const CvMat* cvKalmanPredict( CvKalman* kalman,
                                     const CvMat* control );



 const CvMat* cvKalmanCorrect( CvKalman* kalman, const CvMat* measurement );






 void cvInitSubdivDelaunay2D( CvSubdiv2D* subdiv, CvRect rect );


 CvSubdiv2D* cvCreateSubdiv2D( int subdiv_type, int header_size,
                                      int vtx_size, int quadedge_size,
                                      CvMemStorage* storage );




static CvSubdiv2D* cvCreateSubdivDelaunay2D( CvRect rect, CvMemStorage* storage );
static CvSubdiv2D* cvCreateSubdivDelaunay2D( CvRect rect, CvMemStorage* storage )
{
    CvSubdiv2D* subdiv = cvCreateSubdiv2D( (4 << 5), sizeof(*subdiv),
                         sizeof(CvSubdiv2DPoint), sizeof(CvQuadEdge2D), storage );

    cvInitSubdivDelaunay2D( subdiv, rect );
    return subdiv;
}



 CvSubdiv2DPoint* cvSubdivDelaunay2DInsert( CvSubdiv2D* subdiv, CvPoint2D32f pt);




 CvSubdiv2DPointLocation cvSubdiv2DLocate(
                               CvSubdiv2D* subdiv, CvPoint2D32f pt,
                               CvSubdiv2DEdge* edge,
                               CvSubdiv2DPoint** vertex );


 void cvCalcSubdivVoronoi2D( CvSubdiv2D* subdiv );



 void cvClearSubdivVoronoi2D( CvSubdiv2D* subdiv );



 CvSubdiv2DPoint* cvFindNearestPoint2D( CvSubdiv2D* subdiv, CvPoint2D32f pt );




static CvSubdiv2DEdge cvSubdiv2DNextEdge( CvSubdiv2DEdge edge );
static CvSubdiv2DEdge cvSubdiv2DNextEdge( CvSubdiv2DEdge edge )
{
    return (((CvQuadEdge2D*)((edge) & ~3))->next[(edge)&3]);
}


static CvSubdiv2DEdge cvSubdiv2DRotateEdge( CvSubdiv2DEdge edge, int rotate );
static CvSubdiv2DEdge cvSubdiv2DRotateEdge( CvSubdiv2DEdge edge, int rotate )
{
    return (edge & ~3) + ((edge + rotate) & 3);
}

static CvSubdiv2DEdge cvSubdiv2DSymEdge( CvSubdiv2DEdge edge );
static CvSubdiv2DEdge cvSubdiv2DSymEdge( CvSubdiv2DEdge edge )
{
    return edge ^ 2;
}

static CvSubdiv2DEdge cvSubdiv2DGetEdge( CvSubdiv2DEdge edge, CvNextEdgeType type );
static CvSubdiv2DEdge cvSubdiv2DGetEdge( CvSubdiv2DEdge edge, CvNextEdgeType type )
{
    CvQuadEdge2D* e = (CvQuadEdge2D*)(edge & ~3);
    edge = e->next[(edge + (int)type) & 3];
    return (edge & ~3) + ((edge + ((int)type >> 4)) & 3);
}


static CvSubdiv2DPoint* cvSubdiv2DEdgeOrg( CvSubdiv2DEdge edge );
static CvSubdiv2DPoint* cvSubdiv2DEdgeOrg( CvSubdiv2DEdge edge )
{
    CvQuadEdge2D* e = (CvQuadEdge2D*)(edge & ~3);
    return (CvSubdiv2DPoint*)e->pt[edge & 3];
}


static CvSubdiv2DPoint* cvSubdiv2DEdgeDst( CvSubdiv2DEdge edge );
static CvSubdiv2DPoint* cvSubdiv2DEdgeDst( CvSubdiv2DEdge edge )
{
    CvQuadEdge2D* e = (CvQuadEdge2D*)(edge & ~3);
    return (CvSubdiv2DPoint*)e->pt[(edge + 2) & 3];
}


static double cvTriangleArea( CvPoint2D32f a, CvPoint2D32f b, CvPoint2D32f c );
static double cvTriangleArea( CvPoint2D32f a, CvPoint2D32f b, CvPoint2D32f c )
{
    return (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
}
# 711 "../../cv/include/cv.h"
 CvSeq* cvApproxPoly( const void* src_seq,
                             int header_size, CvMemStorage* storage,
                             int method, double parameter,
                             int parameter2 );




 CvSeq* cvFindDominantPoints( CvSeq* contour, CvMemStorage* storage,
                                   int method ,
                                   double parameter1 ,
                                   double parameter2 ,
                                   double parameter3 ,
                                   double parameter4 );


 double cvArcLength( const void* curve,
                            CvSlice slice ,
                            int is_closed );




 CvRect cvBoundingRect( CvArr* points, int update );


 double cvContourArea( const CvArr* contour,
                              CvSlice slice );


 CvBox2D cvMinAreaRect2( const CvArr* points,
                                CvMemStorage* storage );


 int cvMinEnclosingCircle( const CvArr* points,
                                  CvPoint2D32f* center, float* radius );






 double cvMatchShapes( const void* object1, const void* object2,
                              int method, double parameter );


 CvContourTree* cvCreateContourTree( const CvSeq* contour,
                                            CvMemStorage* storage,
                                            double threshold );


 CvSeq* cvContourFromContourTree( const CvContourTree* tree,
                                         CvMemStorage* storage,
                                         CvTermCriteria criteria );




 double cvMatchContourTrees( const CvContourTree* tree1,
                                    const CvContourTree* tree2,
                                    int method, double threshold );


 void cvCalcPGH( const CvSeq* contour, CvHistogram* hist );





 CvSeq* cvConvexHull2( const CvArr* input,
                             void* hull_storage ,
                             int orientation ,
                             int return_points );


 int cvCheckContourConvexity( const CvArr* contour );


 CvSeq* cvConvexityDefects( const CvArr* contour, const CvArr* convexhull,
                                   CvMemStorage* storage );


 CvBox2D cvFitEllipse2( const CvArr* points );


 CvRect cvMaxRect( const CvRect* rect1, const CvRect* rect2 );


 void cvBoxPoints( CvBox2D box, CvPoint2D32f pt[4] );






 CvHistogram* cvCreateHist( int dims, int* sizes, int type,
                                   float** ranges ,
                                   int uniform );


 void cvSetHistBinRanges( CvHistogram* hist, float** ranges,
                                int uniform );


 CvHistogram* cvMakeHistHeaderForArray(
                            int dims, int* sizes, CvHistogram* hist,
                            float* data, float** ranges ,
                            int uniform );


 void cvReleaseHist( CvHistogram** hist );


 void cvClearHist( CvHistogram* hist );


 void cvGetMinMaxHistValue( const CvHistogram* hist,
                                   float* min_value, float* max_value,
                                   int* min_idx ,
                                   int* max_idx );




 void cvNormalizeHist( CvHistogram* hist, double factor );



 void cvThreshHist( CvHistogram* hist, double threshold );






 double cvCompareHist( const CvHistogram* hist1,
                              const CvHistogram* hist2,
                              int method);



 void cvCopyHist( const CvHistogram* src, CvHistogram** dst );




 void cvCalcBayesianProb( CvHistogram** src, int number,
                                CvHistogram** dst);


 void cvCalcArrHist( CvArr** arr, CvHistogram* hist,
                            int accumulate ,
                            const CvArr* mask );

static void cvCalcHist( IplImage** image, CvHistogram* hist,
                             int accumulate ,
                             const CvArr* mask );
static void cvCalcHist( IplImage** image, CvHistogram* hist,
                             int accumulate, const CvArr* mask )
{
    cvCalcArrHist( (CvArr**)image, hist, accumulate, mask );
}


 void cvCalcArrBackProject( CvArr** image, CvArr* dst,
                                   const CvHistogram* hist );





 void cvCalcArrBackProjectPatch( CvArr** image, CvArr* dst, CvSize range,
                                        CvHistogram* hist, int method,
                                        double factor );





 void cvCalcProbDensity( const CvHistogram* hist1, const CvHistogram* hist2,
                                CvHistogram* dst_hist, double scale );






 void cvSnakeImage( const IplImage* image, CvPoint* points,
                           int length, float* alpha,
                           float* beta, float* gamma,
                           int coeff_usage, CvSize win,
                           CvTermCriteria criteria, int calc_gradient );


 void cvCalcImageHomography( float* line, CvPoint3D32f* center,
                                    float* intrinsic, float* homography );





 void cvDistTransform( const CvArr* src, CvArr* dst,
                              int distance_type ,
                              int mask_size ,
                              const float* mask ,
                              CvArr* labels );
# 928 "../../cv/include/cv.h"
 void cvThreshold( const CvArr* src, CvArr* dst,
                          double threshold, double max_value,
                          int threshold_type );
# 940 "../../cv/include/cv.h"
 void cvAdaptiveThreshold( const CvArr* src, CvArr* dst, double max_value,
                                  int adaptive_method ,
                                  int threshold_type ,
                                  int block_size ,
                                  double param1 );





 void cvFloodFill( CvArr* image, CvPoint seed_point,
                          CvScalar new_val, CvScalar lo_diff ,
                          CvScalar up_diff ,
                          CvConnectedComp* comp ,
                          int flags ,
                          CvArr* mask );






 void cvCanny( const CvArr* image, CvArr* edges, double threshold1,
                      double threshold2, int aperture_size );




 void cvPreCornerDetect( const CvArr* image, CvArr* corners,
                              int aperture_size );



 void cvCornerEigenValsAndVecs( const CvArr* image, CvArr* eigenvv,
                                      int block_size, int aperture_size );



 void cvCornerMinEigenVal( const CvArr* image, CvArr* eigenval,
                                 int block_size, int aperture_size );



 void cvCornerHarris( const CvArr* image, CvArr* harris_responce,
                             int block_size, int aperture_size ,
                             double k );


 void cvFindCornerSubPix( const CvArr* image, CvPoint2D32f* corners,
                                 int count, CvSize win, CvSize zero_zone,
                                 CvTermCriteria criteria );



 void cvGoodFeaturesToTrack( const CvArr* image, CvArr* eig_image,
                                   CvArr* temp_image, CvPoint2D32f* corners,
                                   int* corner_count, double quality_level,
                                   double min_distance,
                                   const CvArr* mask ,
                                   int block_size ,
                                   int use_harris ,
                                   double k );
# 1014 "../../cv/include/cv.h"
 CvSeq* cvHoughLines2( CvArr* image, void* line_storage, int method,
                              double rho, double theta, int threshold,
                              double param1 , double param2 );


 void cvFitLine( const CvArr* points, int dist_type, double param,
                        double reps, double aeps, float* line );







 CvHaarClassifierCascade* cvLoadHaarClassifierCascade(
                    const char* directory, CvSize orig_window_size);

 void cvReleaseHaarClassifierCascade( CvHaarClassifierCascade** cascade );



 CvSeq* cvHaarDetectObjects( const CvArr* image,
                     CvHaarClassifierCascade* cascade,
                     CvMemStorage* storage, double scale_factor ,
                     int min_neighbors , int flags ,
                     CvSize min_size );


 void cvSetImagesForHaarClassifierCascade( CvHaarClassifierCascade* cascade,
                                                const CvArr* sum, const CvArr* sqsum,
                                                const CvArr* tilted_sum, double scale );


 int cvRunHaarClassifierCascade( CvHaarClassifierCascade* cascade,
                                      CvPoint pt, int start_stage );







 void cvUnDistortOnce( const CvArr* src, CvArr* dst,
                              const float* intrinsic_matrix,
                              const float* distortion_coeffs,
                              int interpolate );




 void cvUnDistortInit( const CvArr* src, CvArr* undistortion_map,
                              const float* intrinsic_matrix,
                              const float* distortion_coeffs,
                              int interpolate );



 void cvUnDistort( const CvArr* src, CvArr* dst, const CvArr* undistortion_map,
                          int interpolate );



 void cvConvertMap( const CvArr* src, const CvArr* map_xy,
                           CvArr* map_fast, int iterpolate );


 void cvCalibrateCamera( int image_count,
                                int* point_counts,
                                CvSize image_size,
                                CvPoint2D32f* image_points,
                                CvPoint3D32f* object_points,
                                CvVect32f distortion_coeffs,
                                CvMatr32f camera_matrix,
                                CvVect32f translation_vectors,
                                CvMatr32f rotation_matrixes,
                                int use_intrinsic_guess);


 void cvCalibrateCamera_64d( int image_count,
                                    int* point_counts,
                                    CvSize image_size,
                                    CvPoint2D64d* image_points,
                                    CvPoint3D64d* object_points,
                                    CvVect64d distortion_coeffs,
                                    CvMatr64d camera_matrix,
                                    CvVect64d translation_vectors,
                                    CvMatr64d rotation_matrixes,
                                    int use_intrinsic_guess);



 void cvFindExtrinsicCameraParams( int point_count,
                                          CvSize image_size,
                                          CvPoint2D32f* image_points,
                                          CvPoint3D32f* object_points,
                                          CvVect32f focal_length,
                                          CvPoint2D32f principal_point,
                                          CvVect32f distortion_coeffs,
                                          CvVect32f rotation_vector,
                                          CvVect32f translation_vector);


 void cvFindExtrinsicCameraParams_64d( int point_count,
                                              CvSize image_size,
                                              CvPoint2D64d* image_points,
                                              CvPoint3D64d* object_points,
                                              CvVect64d focal_length,
                                              CvPoint2D64d principal_point,
                                              CvVect64d distortion_coeffs,
                                              CvVect64d rotation_vector,
                                              CvVect64d translation_vector);







 void cvRodrigues( CvMat* rotation_matrix, CvMat* rotation_vector,
                          CvMat* jacobian, int conv_type);


 void cvProjectPoints( int point_count,
                              CvPoint3D64d* object_points,
                              CvVect64d rotation_vector,
                              CvVect64d translation_vector,
                              CvVect64d focal_length,
                              CvPoint2D64d principal_point,
                              CvVect64d distortion,
                              CvPoint2D64d* image_points,
                              CvVect64d deriv_points_rotation_matrix,
                              CvVect64d deriv_points_translation_vect,
                              CvVect64d deriv_points_focal,
                              CvVect64d deriv_points_principal_point,
                              CvVect64d deriv_points_distortion_coeffs);


 void cvProjectPointsSimple( int point_count,
                                  CvPoint3D64d * object_points,
                                  CvVect64d rotation_matrix,
                                  CvVect64d translation_vector,
                                  CvMatr64d camera_matrix,
                                  CvVect64d distortion,
                                  CvPoint2D64d* image_points);


 int cvFindChessBoardCornerGuesses( const CvArr* image, CvArr* thresh,
                                           CvMemStorage* storage, CvSize board_size,
                                           CvPoint2D32f* corners,
                                           int* corner_count );

typedef struct CvPOSITObject CvPOSITObject;


 CvPOSITObject* cvCreatePOSITObject( CvPoint3D32f* points, int point_count );




 void cvPOSIT( CvPOSITObject* posit_object, CvPoint2D32f* image_points,
                       double focal_length, CvTermCriteria criteria,
                       CvMatr32f rotation_matrix, CvVect32f translation_vector);


 void cvReleasePOSITObject( CvPOSITObject** posit_object );





 void cvMake2DPoints( CvMat* src, CvMat* dst );
 void cvMake3DPoints( CvMat* src, CvMat* dst );
 int cvSolveCubic( CvMat* coeffs, CvMat* roots );






 int cvFindFundamentalMat( CvMat* points1, CvMat* points2,
                                 CvMat* fundamental_matrix, int method,
                                 double param1, double param2,
                                 CvMat* status );




 void cvComputeCorrespondEpilines( const CvMat* points,
                                         int which_image,
                                         const CvMat* fundamental_matrix,
                                         CvMat* correspondent_lines );
