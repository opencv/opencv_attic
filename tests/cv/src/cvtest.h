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

#ifndef _CVTEST_H_
#define _CVTEST_H_

#ifdef WIN32
#include <windows.h>
#endif

#include <math.h>
#include "cv.h"
#include "cxmisc.h"
#include "cvaux.h"
#include "cxts.h"
#include "highgui.h"
#include "trsapi.h"

#ifdef __cplusplus
extern "C"{
#endif

#ifndef min
#define min(a,b) (a) > (b) ? (b) : (a)
#endif

#ifndef max
#define max(a,b) (a) < (b) ? (b) : (a)
#endif

#ifndef IPL_DEPTH_MASK
#define IPL_DEPTH_MASK 255
#endif

#define IPPI_CHECK( cvFun )                                                  \
  {                                                                          \
    CvStatus  result = cvFun;                                                \
    if( result != CV_NO_ERR )                                                 \
    {                                                                         \
      trsWrite(ATS_LST,                                                       \
               "The error code %d was returned by the function call\n"        \
               "%s\n"                                                         \
               "in file %s, line %d",                                         \
               result, #cvFun, __FILE__, __LINE__ );                          \
      return trsResult( TRS_FAIL,                                             \
                        "A function from OpenCV library returns error status" ); \
    }                                                                         \
  }

#define ATS_CHECK( atsFun )                                                   \
  {                                                                           \
    CvStatus  result = (CvStatus)atsFun;                                  \
    if( result != IPP_NO_ERR )                                                \
    {                                                                         \
      trsWrite(ATS_LST,                                                       \
               "The error code %d was returned by the function call\n"        \
               "%s\n"                                                         \
               "in file %s, line %d",                                         \
               result, #atsFun, __FILE__, __LINE__ );                         \
      return trsResult( TRS_FAIL,                                             \
                        "A function from ATS library returns error status" ); \
    }                                                                         \
  }


#ifdef WIN32
typedef unsigned char       uchar;
typedef unsigned short      ushort;
#endif

/* define 64-bit integers */
#ifdef WIN32
#if (_MSC_VER > 1000) || defined __BORLANDC__
    typedef __int64 int64;
#else
    typedef long long int64;
#endif
#endif


/****************************************************************************************/
/*                              Warnings Disabling                                      */
/****************************************************************************************/
#if _MSC_VER > 1000
#pragma warning(disable : 4514) /* unreferenced inline function has been */
                                /* removed                               */
#pragma warning(disable : 4127) /* conditional expression is constant    */
                                /* for no warnings in _ASSERT            */
#endif
/****************************************************************************************/
/*                              Finctions declaration                                   */
/****************************************************************************************/

double atsInitRandom( double Min, double Max );
void ats1bInitRandom( double Min, double Max, unsigned char* pDst, long lLen );
void ats1cInitRandom( double Min, double Max, char* pDst, long lLen );
void ats1iInitRandom( double Min, double Max, int* pDst, long lLen );
void ats1flInitRandom( double Min, double Max, float* pDst, long lLen );
void ats1flInitGradRandom( double Min, double Max, float* pDst, long lLen );


void atsbInitEllipse( uchar* Src, int width,  int height,
                      int step, int x, int y, int major, int minor,
                      float orientation, uchar value );

void atsfInitEllipse( float* Src, int width,  int height,
                      int step, int x, int y, int major, int minor,
                      float orientation, float value );

long atsCompSingle(float flFirst,
                   float flSecond,
                   double dbAcc);

long atsCompSinglePrec(float* flFirstArray, float* flSecondArray, long lLen, double dbAcc);

long atsCompDoublePrec(double* flFirstArray,
                       double* flSecondArray,
                       long lLen,
                       double dbAcc);

long atsCompare1Db(  uchar* ArrayAct, uchar* ArrayExp, long  lLen, int    Tol );
long atsCompare1Dc(  char*  ArrayAct, char*  ArrayExp, long  lLen, int    Tol );
long atsCompare1Dfl( float* ArrayAct, float* ArrayExp, long  lLen, double Tol );

CvPoint  atsFindFirstErr( IplImage* imgA, IplImage* imgB, double eps );

#ifndef WIN32
#define __inline static
#endif

/* Compare two angles in (0..360) */
__inline double atsCompareAngles( double angle1, double angle2 )
{
    double err = fabs(angle1 - angle2);
    double err1 = fabs(err - 360);
    return err < err1 ? err : err1;
}

void ats1flInitGrad( double Min, double Max, float* pDst, long lLen );

#define atsGetTickCount cvGetTickCount

void atsTimerStart( int timer );
void atsTimerStop( int timer );


extern char* atsTimingClass;  /* string "Timing" */
extern char* atsAlgoClass;    /* string "Algorithm" */
extern int   atsCPUFreq;      /* CPU frequency (MHz) */

#define  ATS_TICS_TO_USECS(tics)    (((double)(tics))/atsCPUFreq)

/******************************************************************************/
/*                     Extended random number generation                      */
/******************************************************************************/

typedef struct
{
    unsigned  seed;
    int       ia, ib;  /* for integer random numbers */
    float     fa, fb;  /* for float random numbers */
    int       shift;   /* if (upper - lower) is power of two */
    int       mask;    /* float mask */
} AtsRandState;

unsigned  atsGetSeed( void );  /* get seed using processor tick counter */

/* will generate random numbers in [lower,upper) */
void  atsRandInit( AtsRandState* state, double lower, double upper, unsigned seed );
void  atsRandSetBounds( AtsRandState* state, double lower, double upper );
void  atsRandSetFloatBits( AtsRandState* state, int bits );
float atsRand32f( AtsRandState* state );
void  atsbRand32f( AtsRandState* state, float* vect, int len );
int   atsRand32s( AtsRandState* state );
void  atsbRand32s( AtsRandState* state, int* vect, int len );
void  atsbRand16s( AtsRandState* state, short* vect, int len );
void  atsbRand8u( AtsRandState* state, uchar* vect, int len );
void  atsbRand8s( AtsRandState* state, char* vect, int len );
void  atsbRand64d( AtsRandState* state, double* vect, int len );

/* simply returns seed */
int   atsRandPlain32s( AtsRandState* state );
/* return float: 0..1 */
float atsRandPlane32f( AtsRandState* state );

int   atsIsNaN( double x );

/******************************************************************************/
/*                                Data input/output                           */
/******************************************************************************/

float*  atsReadMatrix( const char* filename, int* m, int* n );
void    atsWriteMatrix( const char* filename, int m, int n, float* data );

void atsInitModuleTestData( char* module, char* path_from_module );

char* atsGetTestDataPath( char* buffer, char* folder, char* filename, char* extention );

void atsLoadPrimitives( int flag );

/******************************************************************************/
/*                                 Defines                                    */
/******************************************************************************/

#define ATS_SWAP( a, b, temp )  ((temp) = (a), (a) = (b), (b) = temp)
#define ATS_RANGE( x, a, b )  ((a) <= (x) && (x) < (b))

/* min & max without jumps */
#define ATS_MIN(a, b)  ((a) ^ (((a)^(b)) & (((a) < (b)) - 1)))
#define ATS_MAX(a, b)  ((a) ^ (((a)^(b)) & (((a) > (b)) - 1)))

/* Converts float to 2-complement representation for integer comparing */
#define ATS_TOGGLE_FLT(x)  (((x)&0x7fffffff)^(((int)(x))>>31))

#define ATS_DIM(a)         (sizeof(a)/sizeof((a)[0]))


/* Increases the <value> by adding or multiplying by the <delta> */
#define ATS_INCREASE( value, delta_type, delta ) \
    ((value) = (delta_type) == 0 ? (value)+(delta) : (value)*(delta))


#define ATS_TIC_MAX  0x7fffffffffffffffI64

#define ATS_START_TIMING() int64 temp = atsGetTickCount();
#define ATS_END_TIMING()   temp = atsGetTickCount() - temp; tics = ATS_MIN( tics, temp );

#define ATS_MEASURE( act ) \
    ATS_START_TIMING()     \
    (act);                 \
    ATS_END_TIMING()

#define ATS_COOLMEASURE(func)                       \
{                                                   \
    int i,j;                                        \
    int64 time_min = 0;                           \
                                                    \
    atsTimerStart( 0 );                             \
    for(j=0;j<10;j++)                              \
    {                                               \
        int64 temp = atsGetTickCount();           \
        for(i=0;i<10;i++) func;                     \
        temp = atsGetTickCount() - temp;            \
        if(!time_min) time_min = temp;              \
        else time_min = ATS_MIN( time_min, temp );  \
    }                                               \
    atsTimerStop( 0 );                              \
                                                    \
    tics = (time_min) / 10;                         \
}


#define ATS_CON TW_CON | TW_RUN | TW_DEBUG | TW_RERUN
#define ATS_LST TW_LST | TW_RUN | TW_DEBUG | TW_RERUN
#define ATS_SUM TW_SUM | TW_RUN | TW_DEBUG | TW_RERUN

#define USECS 1000000
#define CPU   1000000

long atsCompare1Db(  uchar* ArrayAct, uchar* ArrayExp, long  lLen, int    Tol );
long atsCompare1Dc(  char*  ArrayAct, char*  ArrayExp, long  lLen, int    Tol );
long atsCompare1Dfl( float* ArrayAct, float* ArrayExp, long  lLen, double Tol );

long atsCompare2Db( uchar* ArrayAct, uchar* ArrayExp, CvSize size, int stride, int Tol );
long atsCompare2Dc( char* ArrayAct, char* ArrayExp, CvSize size, int stride, int Tol );
long atsCompare2Dfl( float* ArrayAct, float* ArrayExp, CvSize size, int stride, double Tol );


void atsConvert( IplImage* src, IplImage* dst );


/*
   Fills the whole image or selected ROI by random numbers.
   Supports only 8u, 8s and 32f formats
*/
void       atsFillRandomImage( IplImage *img, double low, double high );

CvPoint atsRandPoint( AtsRandState* rng_state, CvSize size );
CvPoint2D32f atsRandPoint2D32f( AtsRandState* rng_state, CvSize size );

/* Allocates/Deallocates the IPL image and (may be) clears it */
IplImage*  atsCreateImage( int w, int h, int depth, int nch, int clear_flag );
void atsReleaseImage( IplImage* img );

/* Extracts ROI data from the image and writes it in a single row */
void       atsGetDataFromImage( IplImage *img, void *data );

/* Writes linear data to the image ROI */
void       atsPutDataToImage( IplImage *img, void *data );


typedef void (*AtsBinArithmMaskFunc)( const CvArr* src1, const CvArr* src2,
                                     CvArr* dst, const CvArr* mask );

typedef void (*AtsUnArithmMaskFunc)( const CvArr* src, CvScalar scalar,
                                     CvArr* dst, const CvArr* mask );

typedef void (*AtsBinArithmFunc)( const CvArr* src1, const CvArr* src2, CvArr* dst );

void atsLinearFunc( const CvArr* src1arr, CvScalar alpha,
                    const CvArr* src2arr, CvScalar beta,
                    CvScalar gamma, CvArr* dstarr );

void atsMul( const CvArr* src1arr, const CvArr* src2arr, CvArr* dstarr );

#define ATS_LOGIC_AND  0
#define ATS_LOGIC_OR   1
#define ATS_LOGIC_XOR  2

void atsLogic( const CvArr* src1arr, const CvArr* src2arr, CvArr* dstarr, int op );
void atsLogicS( const CvArr* src1arr, CvScalar scalar, CvArr* dstarr, int op );

/*
   Retrieves various information about the image:
   *pData     - pointer to the whole image or ROI (if presents)
   *pStep     - distance between rows in bytes
   *pSz       - width and height of the whole image or ROI (if presents)
   *pDepth    - depth of image (in the IPL format: IPL_DEPTH_xxxx )
   *pChannels - number of channels
   *pBtPix    - bytes per pixel = ((depth & 255)>>3)*channels;

   Any of the destination pointers may be 0 if the appropriate parameter is'nt needed.
*/
void       atsGetImageInfo( IplImage* img, void** pData, int* pStep,
                            CvSize*  pSz, int*  pDepth, int* pChannels,
                            int* pBtPix );

/*
   The function applies min filter using specified structuring element.
*/
void       atsMinFilterEx( IplImage* src, IplImage* dst, IplConvKernel* B );

/*
   The function applies max filter using specified structuring element.
*/
void       atsMaxFilterEx( IplImage* src, IplImage* dst, IplConvKernel* B );


/*
   Create IplConvKernelFP for calclulating derivative
*/
IplConvKernelFP* atsCalcDervConvKernel( int Xorder, int Yorder, int apertureSize, int origin );

/*
   Replicates left and right ROI borders dx times,
   top and bottom ROI borders dy times.
*/
void atsReplicateBorders( IplImage* img, int dx, int dy );


/*
   The convolution function.
   Supports only 32fC1 images
*/
void atsConvolve( IplImage* src, IplImage* dst, IplConvKernelFP* ker );

void atsConvolveSep2D( IplImage* src, IplImage* dst,
                       IplConvKernelFP* kerX, IplConvKernelFP* kerY );

IplConvKernelFP* atsCreateConvKernelFP( int cols, int rows,
                                        int anchorX, int anchorY,
                                        float* data );

void atsDeleteConvKernelFP( IplConvKernelFP*& kernel );

IplConvKernel* atsCreateConvKernel( int cols, int rows,
                                    int anchorX, int anchorY,
                                    int* data, int shiftR );

void atsDeleteConvKernel( IplConvKernel*& kernel );

/* This function calculates  kernels for Sobel operators */ 
void atsCalcKernel( int   datatype,
                 int   Xorder,
                 int   Yorder,
                 int   apertureSize,
                 char* KerX,
                 char* KerY,
                 CvSize* kerLens,
                 int origin);

/*
   Fills the whole image or selected ROI by random numbers.
   Supports only 8u, 8s and 32f formats
*/
void       atsFillRandomImageEx( IplImage *img, AtsRandState* state );

/* dst(x,y) = scale*src(x,y) + shift */
void  atsScaleAddImage( IplImage* src, IplImage* dst, double scale, double shift );
/* dst(x,y) = abs(scale*src(x,y) + shift) */
void  atsScaleAddAbsImage( IplImage* src, IplImage* dst, double scale, double shift );

/******************************************************************************/
/*                             Matrix functions                               */
/******************************************************************************/

double atsDot( const CvMat* mat1, CvMat* mat2 );
void atsAXPY( double alpha, const CvMat* matX, CvMat* matY );
void atsMatMul( const CvMat* mat1, const CvMat* mat2, CvMat* mat3 );

/******************************************************************************/
/*                             Image statistics                               */
/******************************************************************************/
void       atsCalcImageStatistics( 
                    IplImage* img, IplImage* mask,
                    double* _min_val, double* _max_val,
                    CvPoint* _min_loc, CvPoint* _max_loc,
                    int* _non_zero, double* _sum,
                    double* _mean, double* _sdv,
                    double* _c_norm, double* _l1_norm, double* _l2_norm,
                    int* _mask_pix );

int       atsCannyStatistics(uchar* src,
                             CvSize roi,
                             int srcStep,
                             uchar* dst,
                             int dstStep,
                             int Sobord,
                             float lowThreshold,
                             float highThreshold,
                             int* zero_mag,
                             int* under_low,
                             int* above_high,
                             int* edges_in_nms,
                             int* components,
                             int* in_edges);


typedef struct
{
    /* spatial moments */
    double  m00;
    double  m10, m01;
    double  m20, m11, m02;
    double  m30, m21, m12, m03;

    /* central moments */
    double  mu20, mu11, mu02;
    double  mu30, mu21, mu12, mu03;

    /* normalized central moments */
    double  nu20, nu11, nu02;
    double  nu30, nu21, nu12, nu03;
}
AtsMomentState;


/*
  Function calculates spatial and central moments up to third order.
  <binary> mode means that pixels values treated as 1 if they are non zero and 0 if zero.
*/
void    atsCalcMoments( IplImage* img, AtsMomentState* state, int binary );

/*
  Convert internal representation to explicit form
*/
void  atsGetMoments( CvMoments* istate, AtsMomentState* astate );

/* calculates  sum (imgA(x,y) - deltaA)*(imgB(x,y) - deltaB) */
double atsCrossCorr( IplImage* imgA, IplImage* imgB, double deltaA, double deltaB );

/* creates contour which consist of convex hull vertices */
/* hull is CvSeq<CvPoint*>                               */
CvSeq* atsCvtHullToContour( CvSeq* hull, CvMemStorage* storage );


/******************************************************************************/
/*                                 Drawing                                    */
/******************************************************************************/

/* The function draws line in 8uC1/C3 image */
void  atsDrawLine( IplImage* img, float x1, float y1, float x2, float y2, int color );

/* The function draws ellipse arc in 8uC1/C3 image */
void  atsDrawEllipse( IplImage* img, float xc, float yc, float a, float b,
                      float angle, float arc0, float arc1, int color );

/* The function draws conic arc in 8uC1/C3 image */
void  atsDrawConic( IplImage* img, float xc, float yc, float mag, float e,
                    float angle, float arc0, float arc1, int color );

int   atsCalcQuadricCoeffs( double xc, double yc, double mag, double e,
                            double angle, double arc0, double arc1,
                            double* A, double* B, double* C, double* D, double* E,
                            CvPoint* pt1, CvPoint* pt2 );

/* make zero border in the image */
void  atsClearBorder( IplImage* img );

/* fills an 8uC1 image with blobs - rotated ellipses */
void  atsGenerateBlobImage( IplImage* img, int min_blob_size, int max_blob_size,
                            int blob_count, int min_brightness, int max_brightness,
                            AtsRandState* rng_state );

/******************************************************************************/
/*                             Display routines                               */ 
/******************************************************************************/

int   atsCreateWindow( const char* name, CvPoint wnd_org, CvSize wnd_size );
void  atsDisplayImage( IplImage* img, int window, CvPoint dst_org, CvSize dst_size );
void  atsDestroyWindow( int window );

/******************************************************************************/
/*                     Reading images from file                               */
/******************************************************************************/

/* Reads image from the disk and creates IplImage from it */  
IplImage* atsCreateImageFromFile( const char* filename );


/******************************************************************************/
/*                     Helper contour processing functions                    */
/******************************************************************************/

CvSeq* atsCreateRandomContours( IplImage* img, CvMemStorage* storage,
                                CvContourRetrievalMode mode,
                                CvChainApproxMethod approx,
                                AtsRandState* rng_state );

/******************************************************************************/
/*                                 Set of contours                            */
/******************************************************************************/

typedef CvSeq* ( *Contour )( CvMemStorage* storage );

/******************************************************************************/
/*                                 Defines                                    */
/******************************************************************************/

#define ATS_SWAP( a, b, temp )  ((temp) = (a), (a) = (b), (b) = temp)
#define ATS_RANGE( x, a, b )  ((a) <= (x) && (x) < (b))

/* min & max without jumps */
#define ATS_MIN(a, b)  ((a) ^ (((a)^(b)) & (((a) < (b)) - 1)))
#define ATS_MAX(a, b)  ((a) ^ (((a)^(b)) & (((a) > (b)) - 1)))

/* Converts float to 2-complement representation for integer comparing */
#define ATS_TOGGLE_FLT(x)  (((x)&0x7fffffff)^(((int)(x))>>31))

#define ATS_DIM(a)         (sizeof(a)/sizeof((a)[0]))


/* Increases the <value> by adding or multiplying by the <delta> */
#define ATS_INCREASE( value, delta_type, delta ) \
    ((value) = (delta_type) == 0 ? (value)+(delta) : (value)*(delta))


#ifndef ATS_TIC_MAX
#ifdef WIN32
#define ATS_TIC_MAX  0x7fffffffffffffffI64
#else
#define ATS_TIC_MAX  0x7fffffffffffffffLL
#endif
#endif

#define ATS_START_TIMING() int64 temp = atsGetTickCount();
#define ATS_END_TIMING()   temp = atsGetTickCount() - temp; tics = ATS_MIN( tics, temp );

#define ATS_MEASURE( act ) \
    ATS_START_TIMING()     \
    (act);                 \
    ATS_END_TIMING()


#define ATS_COMP_ERROR(Error,Func,ErrorExp,Message)          \
        {                                                    \
            IppiStatus  res;                                 \
            trsWrite( ATS_CON | ATS_LST, "%s...", Message ); \
            if( (res = Func) != ErrorExp )                   \
            {                                                \
                trsWrite( ATS_CON | ATS_LST,                 \
                          "error: act %d  exp %s\n",         \
                          res,                               \
                          #ErrorExp );                       \
                Error++;                                     \
            }                                                \
            else trsWrite( ATS_CON | ATS_LST, "ok\n" );      \
        }

#define _CV_C            1
#define _CV_L1           2
#define _CV_L2           4
#define _CV_RELATIVE     8

#define _CV_RELATIVE_C   (_CV_RELATIVE | _CV_C)
#define _CV_RELATIVE_L1  (_CV_RELATIVE | _CV_L1)
#define _CV_RELATIVE_L2  (_CV_RELATIVE | _CV_L2)

#define _CV_DIFF        16

#define _CV_DIFF_C   (_CV_DIFF | _CV_C)
#define _CV_DIFF_L1  (_CV_DIFF | _CV_L1)
#define _CV_DIFF_L2  (_CV_DIFF | _CV_L2)


#define CV_ORIGIN_TL  0
#define CV_ORIGIN_BL  1

void* icvAlloc_( int lSize );
void  icvFree_( void** ptr );

void* _dbgAlloc_( int size, const char* file, int line);
void  _dbgFree_( void** pptr,const char* file, int line);

#define icvAlloc( size ) icvAlloc_( size )
#define icvFree( ptr ) icvFree_( (void**)(ptr) )


#ifdef __cplusplus
}
#endif

#endif /* _CVTEST_H_ */

/* End of file. */
