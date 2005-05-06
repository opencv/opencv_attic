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
/*                                Warnings Disabling                                    */
/****************************************************************************************/
#if _MSC_VER > 1000
#pragma warning(disable : 4514) /* unreferenced inline function has been */
                                /* removed                               */
#pragma warning(disable : 4127) /* conditional expression is constant    */
                                /* for no warnings in _ASSERT            */
#endif


/****************************************************************************************/
/*                       Helper Functions for New-Style Tests                           */
/****************************************************************************************/

void cvTsCalcSobelKernel2D( int dx, int dy, int _aperture_size, int origin, CvMat* kernel );

    
/****************************************************************************************/
/*                           Old API for RNG and Comparison                             */
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
   Fills the whole image or selected ROI by random numbers.
   Supports only 8u, 8s and 32f formats
*/
void       atsFillRandomImageEx( IplImage *img, AtsRandState* state );

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

int cvTsRodrigues( const CvMat* src, CvMat* dst, CvMat* jacobian=0 );
void cvTsConvertHomogenious( const CvMat* src, CvMat* dst );
void cvTsConvertHomogenious( const CvMat* src, CvMat* dst );

#ifdef __cplusplus
}
#endif

#endif /* _CVTEST_H_ */

/* End of file. */
