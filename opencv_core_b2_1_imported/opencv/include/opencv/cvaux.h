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

#ifndef __CVAUX__H__
#define __CVAUX__H__

#include <cv.h>

#if defined WIN32 && defined CVAUX_DLL
    #define CVAUX_DLL_ENTRY __declspec(dllexport)
#else
    #define CVAUX_DLL_ENTRY
#endif

#ifndef OPENCVAUXAPI
    #define OPENCVAUXAPI CV_EXTERN_C CVAUX_DLL_ENTRY
#endif

/****************************************************************************************\
*                               1D HMM  experimental                                     *
\****************************************************************************************/

typedef CvImgObsInfo Cv1DObsInfo;


OPENCVAUXAPI CvStatus  icvCreate1DHMM( CvEHMM** this_hmm,
                                   int state_number, int* num_mix, int obs_size );

OPENCVAUXAPI CvStatus  icvRelease1DHMM( CvEHMM** phmm );

OPENCVAUXAPI CvStatus  icvUniform1DSegm( Cv1DObsInfo* obs_info, CvEHMM* hmm );

OPENCVAUXAPI CvStatus  icvInit1DMixSegm( Cv1DObsInfo** obs_info_array, int num_img, CvEHMM* hmm);

OPENCVAUXAPI CvStatus  icvEstimate1DHMMStateParams( CvImgObsInfo** obs_info_array, int num_img, CvEHMM* hmm);

OPENCVAUXAPI CvStatus  icvEstimate1DObsProb( CvImgObsInfo* obs_info, CvEHMM* hmm );

OPENCVAUXAPI CvStatus  icvEstimate1DTransProb( Cv1DObsInfo** obs_info_array,
                                           int num_seq,
                                           CvEHMM* hmm );

OPENCVAUXAPI float  icvViterbi( Cv1DObsInfo* obs_info, CvEHMM* hmm);

OPENCVAUXAPI CvStatus  icv1DMixSegmL2( CvImgObsInfo** obs_info_array, int num_img, CvEHMM* hmm );


/****************************************************************************************\
*                           Additional operations on Subdivisions                        *
\****************************************************************************************/

// paints voronoi diagram: just demo function
OPENCVAUXAPI void  icvDrawMosaic( CvSubdiv2D* subdiv, IplImage* src, IplImage* dst );

// checks planar subdivision for correctness. It is not an absolute check,
// but it verifies some relations between quad-edges
OPENCVAUXAPI int   icvSubdiv2DCheck( CvSubdiv2D* subdiv );

// finds the nearest to the given point vertex in subdivision.
OPENCVAUXAPI CvSubdiv2DPoint* icvFindNearestPoint2D( CvSubdiv2D* subdiv, CvPoint2D32f pt );

// simplified Delanay diagram creation
CV_INLINE  CvSubdiv2D* icvCreateSubdivDelaunay2D( CvRect rect, CvMemStorage* storage )
{
    CvSubdiv2D* subdiv = cvCreateSubdiv2D( CV_SEQ_KIND_SUBDIV2D, sizeof(*subdiv),
                         sizeof(CvSubdiv2DPoint), sizeof(CvQuadEdge2D), storage );

    cvInitSubdivDelaunay2D( subdiv, rect );

    return subdiv;
}

// returns squared distance between two 2D points with floating-point coordinates.
CV_INLINE double icvSqDist2D32f( CvPoint2D32f pt1, CvPoint2D32f pt2 )
{
    double dx = pt1.x - pt2.x;
    double dy = pt1.y - pt2.y;

    return dx*dx + dy*dy;
}


/****************************************************************************************\
*                           More operations on sequences                                 *
\****************************************************************************************/

//
//  Convert slice from (start,end) format to (start1,length) format,
//  where length is the number of elements in the slice and start1 is normalized
//  start index of the slice (i.e. 0 <= start1 < seq->total).
//////////////////////////////////////////////////////////////////////
OPENCVAUXAPI CvSlice icvNormalizeSlice( CvSlice slice, CvSeq* seq );

// Iteration through the sequence tree
///////////////////////////////////////////////////////////
typedef struct CvSeqTreeIterator
{
    CvSeq* seq;
    int level;
    int maxLevel;
}
CvSeqTreeIterator;

OPENCVAUXAPI void icvInitSeqTreeIterator( CvSeqTreeIterator* seqIterator, CvSeq* first, int maxLevel );
OPENCVAUXAPI CvSeq* icvNextSeq( CvSeqTreeIterator* seqIterator );
OPENCVAUXAPI CvSeq* icvPrevSeq( CvSeqTreeIterator* seqIterator );

////////////////////////////////////////////////////////////

// allows to work with part of the sequence as with an ordinary sequence.
// Doesn't copies any data - only creates neccessary sequence and block headers.
// By default (storage == 0), the new sequence is resided in the same storage as the original one.
// !!! NOTE !!! All the operations that affect sequence may affect all the sub-sequences and vice versa.
OPENCVAUXAPI CvSeq* icvSeqSlice( CvSeq* seq, CvSlice slice, CvMemStorage* storage CV_DEFAULT(0));

// Makes a copy of a sequence or a part of the sequence. Returns pointer to new sequence.
// By default (storage == 0), the new sequence is resided in the same storage as the original one.
OPENCVAUXAPI CvSeq* icvCopySeq( CvSeq* seq, CvSlice slice CV_DEFAULT(CV_WHOLE_SEQ),
                             CvMemStorage* storage CV_DEFAULT(0));

// Removes several elements from the middle of the sequence
OPENCVAUXAPI void icvSeqRemoveSlice( CvSeq* seq, CvSlice slice );

// Inserts a new sequence into the middle of another sequence
OPENCVAUXAPI void icvSeqInsertSlice( CvSeq* seq, int index, CvSeq* from );

// Inverts the sequence in-place - 0-th element becomes last, 1-st becomes pre-last etc.
OPENCVAUXAPI void icvSeqInvert( CvSeq* seq );

// Sort the sequence using user-specified comparison function.
// Semantics is the same as in qsort function
OPENCVAUXAPI void icvSeqSort( CvSeq* seq, int(CV_CDECL*)(const void*,const void*,void*),
                           void* userdata);

// Removes several first elements from sequence and, optionaly, put them to the buffer
OPENCVAUXAPI void icvSeqPopFrontMulti( CvSeq* seq, void* elements, int count );

// Gathers pointers to all the sequences, accessible from the <first>, to the single sequence.
OPENCVAUXAPI CvSeq* icvSeqTreeToSeq( CvSeq* first, int header_size CV_DEFAULT(sizeof(CvSeq)),
                                  CvMemStorage* storage CV_DEFAULT(0) );


// Insert contour into tree given certain parent sequence.
// If parent is equal to frame (the most external contour),
// then added contour will have null pointer to parent.
OPENCVAUXAPI void icvInsertContourIntoTree( CvSeq* contour, CvSeq* parent, CvSeq* frame );


// Removes contour from tree (together with the contour children).
OPENCVAUXAPI void icvRemoveContourFromTree( CvSeq* contour, CvSeq* frame );

// Applies affine transformation to every point of the sequence
OPENCVAUXAPI void icvWarpAffineSeq( CvSeq* seq, double matrix[3][2] );

/*****************************************************************************************/

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

#define  CV_GRAPH_SEARCH_TREE_NODE_FLAG   (1 << 30)
#define  CV_GRAPH_FORWARD_EDGE_FLAG       (1 << 30)

#define  CV_GRAPH_WEIGHTED_VERTEX_FIELDS() CV_GRAPH_VERTEX_FIELDS()\
    float weight;

#define  CV_GRAPH_WEIGHTED_EDGE_FIELDS() CV_GRAPH_EDGE_FIELDS()\
    float weight;

typedef struct CvGraphWeightedVtx
{
    CV_GRAPH_WEIGHTED_VERTEX_FIELDS()
}
CvGraphWeightedVtx;

typedef struct CvGraphWeightedEdge
{
    CV_GRAPH_WEIGHTED_EDGE_FIELDS()
}
CvGraphWeightedEdge;


typedef struct CvGraphScanner
{
    CvGraphVtx* vtx;       // current graph vertex (or current edge origin)
    CvGraphVtx* dst;       // current graph edge destination vertex
    CvGraphEdge* edge;     // current edge
    
    CvGraph* graph;        // the graph
    CvSeq*   stack;        // the graph vertex stack
    int      index;        // the lower bound of certainly visited vertices
    int      mask;         // event mask
}
CvGraphScanner;

//
// mask indicates what events one wants to handle
//
OPENCVAUXAPI void icvStartScanGraph( CvGraph* graph, CvGraphScanner* scanner,
                                  CvGraphVtx* vtx CV_DEFAULT(0),
                                  int mask CV_DEFAULT(CV_GRAPH_ALL_ITEMS));

OPENCVAUXAPI void icvEndScanGraph( CvGraphScanner* scanner );

//
// returns type of the current element (see CV_GRAPH_* above).
// type is:
//    CV_GRAPH_VERTEX, when <scanner.vtx> vertex is visited for the first time,
//    CV_GRAPH_FORWARD_EDGE, when <scanner.edge> edge 
//                 i.e. from <scanner.vtx> to <scanner.dst>) is seen for the time and
//                 it leads to unvisited vertex (<scanner.dst>)
//    CV_GRAPH_BACK_EDGE, when <scanner.edge> edge (from <scanner.vtx> to <scanner.dst>)
//                 is seen for the first time, but it leads to previously visited vertex
//                 <scanner.dst> in the same search tree branch.
//    CV_GRAPH_CROSS_EDGE - (can happen only in oriented graphs). The same as previous, but
//                          the <scanner.dst> vertex was visited in another branch of the
//                          search tree.
//    CV_GRAPH_COMPONENT, when a new connected component is met.
//    CV_GRAPH_OVER (==-1), when all the vertices and all the edges of the graph were visited.
//
OPENCVAUXAPI int  icvNextGraphItem( CvGraphScanner* scanner );


/*****************************************************************************************/


// Bilateral filter
///////////////////////////////////////////////////////////
OPENCVAUXAPI void icvBilateralFiltering(IplImage* in, IplImage* out, int thresh_space, int thresh_color);

///////////////////////////////////////////////////////////
// Triangulation
OPENCVAUXAPI void cvDecompPoly( CvContour* cont, CvSubdiv2D** subdiv, CvMemStorage* storage );
///////////////////////////////////////////////////////////

/*******************************Stereo correspondence*************************************/
OPENCVAUXAPI void icvDrawFilledSegments( CvSeq* seq, IplImage* img, int part );

OPENCVAUXAPI CvSeq* cvExtractSingleEdges( IplImage* image, //bw image
                      CvMemStorage* storage );



typedef struct CvCliqueFinder
{   
    CvGraph* graph;
    int**    adj_matr;
    int N; //graph size

    // stacks, counters etc/
    int k; //stack size
    int* current_comp;
    int** All;
    
    int* ne;
    int* ce;
    int* fixp; //node with minimal disconnections
    int* nod;
    int* s; //for selected candidate
    int status;
    int best_score;
    int weighted;
    int weighted_edges;    
    float best_weight;
    float* edge_weights;
    float* vertex_weights;
    float* cur_weight;
    float* cand_weight;

} CvCliqueFinder;

#define CLIQUE_FOUND 1
#define CLIQUE_END   0

OPENCVAUXAPI void cvStartFindCliques( CvGraph* graph, CvCliqueFinder* finder, int reverse, 
                                   int weighted CV_DEFAULT(0),  int weighted_edges CV_DEFAULT(0));
OPENCVAUXAPI int cvFindNextMaximalClique( CvCliqueFinder* finder ); 
OPENCVAUXAPI void cvEndFindCliques( CvCliqueFinder* finder );

OPENCVAUXAPI void cvBronKerbosch( CvGraph* graph );                 


/*****************************************************************************************/
/************ Epiline functions *******************/



typedef struct StereoLineCoeff
{
    double Apart;
    double ApartA;
    double ApartB;
    double ApartAB;

    double Xpart;
    double XpartA;
    double XpartB;
    double XpartAB;

    double Ypart;
    double YpartA;
    double YpartB;
    double YpartAB;

    double Zpart;
    double ZpartA;
    double ZpartB;
    double ZpartAB;

    CvPoint3D64d pointA;
    CvPoint3D64d pointB;
    CvPoint3D64d pointC;

    CvPoint3D64d pointCam1;
    CvPoint3D64d pointCam2;

}StereoLineCoeff;


typedef struct CvContourOrientation
{
    float egvals[2];
    float egvects[4];

    float max, min; // minimim and maximum projections
    int imax, imin;
} CvContourOrientation;


OPENCVAUXAPI CvStatus icvGetSymPoint3D(  CvPoint3D64d pointCorner,
                            CvPoint3D64d point1,
                            CvPoint3D64d point2,
                            CvPoint3D64d *pointSym2);

OPENCVAUXAPI void icvGetPieceLength3D(CvPoint3D64d point1,CvPoint3D64d point2,double* dist);

OPENCVAUXAPI CvContourOrientation FindPrincipalAxes(CvSeq* contour);
//OPENCVAUXAPI void DrawEdges(IplImage* image, CvSeq* seq);

OPENCVAUXAPI CvStatus icvFindLineOnImage(IplImage* image,CvPoint* point1,CvPoint* point2,int* num);

OPENCVAUXAPI CvStatus icvCompute3DPoint(    double alpha,double betta,
                            StereoLineCoeff* coeffs,
                            CvPoint3D64d* point);

OPENCVAUXAPI CvStatus icvCreateConvertMatrVect( CvMatr64d     rotMatr1,
                                CvMatr64d     transVect1,
                                CvMatr64d     rotMatr2,
                                CvMatr64d     transVect2,
                                CvMatr64d     convRotMatr,
                                CvMatr64d     convTransVect);

OPENCVAUXAPI CvStatus icvConvertPointSystem(CvPoint3D64d  M2,
                            CvPoint3D64d* M1,
                            CvMatr64d     rotMatr,
                            CvMatr64d     transVect
                            );

OPENCVAUXAPI CvStatus icvComputeCoeffForStereo( double quad1[4][2],
                                double quad2[4][2],
                                int    numScanlines,
                                CvMatr64d    camMatr1,
                                CvMatr64d    rotMatr1,
                                CvMatr64d    transVect1,
                                CvMatr64d    camMatr2,
                                CvMatr64d    rotMatr2,
                                CvMatr64d    transVect2,
                                StereoLineCoeff*    startCoeffs);

OPENCVAUXAPI CvStatus icvComCoeffForLine(   CvPoint2D64d point1,
                            CvPoint2D64d point2,
                            CvPoint2D64d point3,
                            CvPoint2D64d point4,
                            CvMatr64d    camMatr1,
                            CvMatr64d    rotMatr1,
                            CvMatr64d    transVect1,
                            CvMatr64d    camMatr2,
                            CvMatr64d    rotMatr2,
                            CvMatr64d    transVect2,
                            StereoLineCoeff*    coeffs);

OPENCVAUXAPI CvStatus icvGetDirectionForPoint(  CvPoint2D64d point,
                                CvMatr64d camMatr,
                                CvPoint3D64d* direct);

OPENCVAUXAPI CvStatus icvGetCrossLines(CvPoint3D64d point11,CvPoint3D64d point12,
                       CvPoint3D64d point21,CvPoint3D64d point22,
                       CvPoint3D64d* midPoint);

OPENCVAUXAPI CvStatus icvComputeStereoLineCoeffs(   CvPoint3D64d pointA,
                                    CvPoint3D64d pointB,
                                    CvPoint3D64d pointC,
                                    CvPoint3D64d pointCam1,
                                    CvPoint3D64d pointCam2,
                                    StereoLineCoeff*    coeffs);

OPENCVAUXAPI CvStatus icvComputeFundMatrEpipoles ( CvMatr64d camMatr1, 
                                    CvMatr64d     rotMatr1, 
                                    CvVect64d     transVect1,
                                    CvMatr64d     camMatr2,
                                    CvMatr64d     rotMatr2,
                                    CvVect64d     transVect2,
                                    CvPoint2D64d* epipole1,
                                    CvPoint2D64d* epipole2,
                                    CvMatr64d     fundMatr);
/*
void cvComputeFundMatrEpipoles( CvMatr64d camMatr1, 
                                CvMatr64d rotMatr1, 
                                CvVect64d transVect1,
                                CvMatr64d camMatr2,
                                CvMatr64d rotMatr2,
                                CvVect64d transVect2,
                                CvPoint2D64d* epipole1,
                                CvPoint2D64d* epipole2,
                                CvMatr64d fundMatr);
*/
OPENCVAUXAPI int icvGetAngleLine( CvPoint2D64d startPoint, CvSize imageSize,CvPoint2D64d *point1,CvPoint2D64d *point2);

OPENCVAUXAPI void icvGetCoefForPiece(   CvPoint2D64d p_start,CvPoint2D64d p_end,
                        double *a,double *b,double *c,
                        int* result);

OPENCVAUXAPI void icvGetCommonArea( CvSize imageSize,
                    CvPoint2D64d epipole1,CvPoint2D64d epipole2,
                    CvMatr64d fundMatr,
                    CvVect64d coeff11,CvVect64d coeff12,
                    CvVect64d coeff21,CvVect64d coeff22,
                    int* result);

OPENCVAUXAPI void icvFindCorrPointsFundamentLK(IplImage* image1,IplImage* image2,
                               CvMatr64d fundMatr,
                               CvPoint2D64d* points1,
                               CvPoint2D64d* points2,
                               int* count
                               );

OPENCVAUXAPI void icvGetCrossDirectDirect(  CvVect64d direct1,CvVect64d direct2,
                            CvPoint2D64d *cross,int* result);

OPENCVAUXAPI void icvGetCrossPieceDirect(   CvPoint2D64d p_start,CvPoint2D64d p_end,
                            double a,double b,double c,
                            CvPoint2D64d *cross,int* result);

OPENCVAUXAPI void icvGetCrossPiecePiece( CvPoint2D64d p1_start,CvPoint2D64d p1_end,
                            CvPoint2D64d p2_start,CvPoint2D64d p2_end,
                            CvPoint2D64d* cross,
                            int* result);
                            
OPENCVAUXAPI void icvGetPieceLength(CvPoint2D64d point1,CvPoint2D64d point2,double* dist);

OPENCVAUXAPI void icvGetCrossRectDirect(    CvSize imageSize,
                            double a,double b,double c,
                            CvPoint2D64d *start,CvPoint2D64d *end,
                            int* result);

OPENCVAUXAPI void icvProjectPointToImage(   CvPoint3D64d point,
                            CvMatr64d camMatr,CvMatr64d rotMatr,CvVect64d transVect,
                            CvPoint2D64d* projPoint);

OPENCVAUXAPI void icvGetQuadsTransform( CvSize        imageSize,
                        CvMatr64d     camMatr1,
                        CvMatr64d     rotMatr1,
                        CvVect64d     transVect1,
                        CvMatr64d     camMatr2,
                        CvMatr64d     rotMatr2,
                        CvVect64d     transVect2,
                        CvSize*       warpSize,
                        double quad1[4][2],
                        double quad2[4][2],
                        CvMatr64d     fundMatr,
                        CvPoint2D64d* epipole1,
                        CvPoint2D64d* epipole2
                        );

OPENCVAUXAPI void icvGetCutPiece(   CvVect64d areaLineCoef1,CvVect64d areaLineCoef2,
                    CvPoint2D64d epipole,
                    CvSize imageSize,
                    CvPoint2D64d* point11,CvPoint2D64d* point12,
                    CvPoint2D64d* point21,CvPoint2D64d* point22,
                    int* result);

OPENCVAUXAPI void icvGetMiddleAnglePoint(   CvPoint2D64d basePoint,
                            CvPoint2D64d point1,CvPoint2D64d point2,
                            CvPoint2D64d* midPoint);

OPENCVAUXAPI void icvGetNormalDirect(CvVect64d direct,CvPoint2D64d point,CvVect64d normDirect);

OPENCVAUXAPI double icvGetVect(CvPoint2D64d basePoint,CvPoint2D64d point1,CvPoint2D64d point2);

OPENCVAUXAPI void icvTestPoint( CvPoint2D64d testPoint,
                CvVect64d line1,CvVect64d line2,
                CvPoint2D64d basePoint,
                int* result);

OPENCVAUXAPI void icvProjectPointToDirect(  CvPoint2D64d point,CvVect64d lineCoeff,
                            CvPoint2D64d* projectPoint);

OPENCVAUXAPI void icvGetDistanceFromPointToDirect( CvPoint2D64d point,CvVect64d lineCoef,double*dist);

OPENCVAUXAPI IplImage* icvCreateIsometricImage( IplImage* src, IplImage* dst,
                              int desired_depth, int desired_num_channels );

OPENCVAUXAPI CvStatus icvCvt_32f_64d( float *src, double *dst, int size );
OPENCVAUXAPI CvStatus icvCvt_64d_32f( double *src, float *dst, int size );

OPENCVAUXAPI void cvDeInterlace( IplImage* frame, IplImage* fieldEven, IplImage* fieldOdd );


/****************************************************************************************\
*                                    Texture Descriptors                                 *
\****************************************************************************************/

#define CV_GLCM_OPTIMIZATION_NONE                   -2
#define CV_GLCM_OPTIMIZATION_LUT                    -1
#define CV_GLCM_OPTIMIZATION_HISTOGRAM              0

#define CV_GLCMDESC_OPTIMIZATION_ALLOWDOUBLENEST    10
#define CV_GLCMDESC_OPTIMIZATION_ALLOWTRIPLENEST    11
#define CV_GLCMDESC_OPTIMIZATION_HISTOGRAM          4

#define CV_GLCMDESC_ENTROPY                         0
#define CV_GLCMDESC_ENERGY                          1
#define CV_GLCMDESC_HOMOGENITY                      2
#define CV_GLCMDESC_CONTRAST                        3
#define CV_GLCMDESC_CLUSTERTENDENCY                 4
#define CV_GLCMDESC_CLUSTERSHADE                    5
#define CV_GLCMDESC_CORRELATION                     6
#define CV_GLCMDESC_CORRELATIONINFO1                7
#define CV_GLCMDESC_CORRELATIONINFO2                8
#define CV_GLCMDESC_MAXIMUMPROBABILITY              9

#define CV_GLCM_ALL                                 0
#define CV_GLCM_GLCM                                1
#define CV_GLCM_DESC                                2

typedef struct CvGLCM CvGLCM;

OPENCVAUXAPI CvGLCM* cvCreateGLCM( const IplImage* srcImage,
                                int stepMagnitude,
                                const int* stepDirections CV_DEFAULT(0),
                                int numStepDirections CV_DEFAULT(0),
                                int optimizationType CV_DEFAULT(CV_GLCM_OPTIMIZATION_NONE));

OPENCVAUXAPI void cvReleaseGLCM( CvGLCM** GLCM, int flag CV_DEFAULT(CV_GLCM_ALL));

OPENCVAUXAPI void cvCreateGLCMDescriptors( CvGLCM* destGLCM,
                                        int descriptorOptimizationType
                                        CV_DEFAULT(CV_GLCMDESC_OPTIMIZATION_ALLOWDOUBLENEST));

OPENCVAUXAPI double cvGetGLCMDescriptor( CvGLCM* GLCM, int step, int descriptor );

OPENCVAUXAPI void cvGetGLCMDescriptorStatistics( CvGLCM* GLCM, int descriptor,
                                              double* average, double* standardDeviation );

OPENCVAUXAPI IplImage* cvCreateGLCMImage( CvGLCM* GLCM, int step );


/****************************************************************************************\
*                                   Calibration engine                                   *
\****************************************************************************************/

typedef struct CvCamera
{
    float   imgSize[2]; /* size of the camera view, used during calibration */

    float   focalLength[2]; /* focal length of the camera: (fx, fy) */
    float   principalPoint[2]; /* coordinates of principal point: (cx, cy) */
    float   matrix[9]; /* intinsic camera parameters:  [ fx 0 cx; 0 fy cy; 0 0 1 ] */

    float   distortion[4]; /* distortion coefficients - two coefficients for radial distortion
                              and another two for tangential: [ k1 k2 p1 p2 ] */
    float   rotMatr[9];
    float   transVect[3]; /* rotation matrix and transition vector relatively
                             to some reference point in the space. */

    /* part, valid only for stereo */
    CvPoint2D32f epipole; /* coordinates of epipole */
    CvPoint2D32f quad[4]; /* coordinates of destination quadrangle after
                             epipolar geometry rectification */
}
CvCamera;

typedef struct CvStereoCamera
{
    CvCamera* camera[2]; /* two individual camera parameters */
    double fundMatr[9]; /* fundamental matrix */
}
CvStereoCamera;

OPENCVAUXAPI void cvInitRectify( const CvArr* srcImage,
                                 const CvCamera* params,
                                 CvArr* rectMap );

OPENCVAUXAPI void cvSegmentImage( CvArr* srcarr, CvArr* dstarr,
                                  double canny_threshold,
                                  double ffill_threshold );

#ifdef __cplusplus

typedef enum CvCalibEtalonType
{
    CV_CALIB_ETALON_USER = -1,
    CV_CALIB_ETALON_CHESSBOARD = 0,
    CV_CALIB_ETALON_CHECKERBOARD = CV_CALIB_ETALON_CHESSBOARD
}
CvCalibEtalonType;

class CvCalibFilter
{
public:
    /* Constructor & destructor */
    CVAUX_DLL_ENTRY CvCalibFilter();
    CVAUX_DLL_ENTRY virtual ~CvCalibFilter();

    /* Sets etalon type - one for all cameras.
       etalonParams is used in case of pre-defined etalons (such as chessboard).
       Number of elements in etalonParams is determined by etalonType.
       E.g., if etalon type is CV_ETALON_TYPE_CHESSBOARD then:
         etalonParams[0] is number of squares per one side of etalon
         etalonParams[1] is number of squares per another side of etalon
         etalonParams[2] is linear size of squares in the board in arbitrary units.
       pointCount & points are used in case of
       CV_CALIB_ETALON_USER (user-defined) etalon. */
    CVAUX_DLL_ENTRY virtual bool
        SetEtalon( CvCalibEtalonType etalonType, double* etalonParams,
                   int pointCount = 0, CvPoint2D32f* points = 0 );

    /* Retrieves etalon parameters/or and points */
    CVAUX_DLL_ENTRY virtual CvCalibEtalonType
        GetEtalon( int* paramCount = 0, const double** etalonParams = 0,
                   int* pointCount = 0, const CvPoint2D32f** etalonPoints = 0 ) const;

    /* Sets number of cameras calibrated simultaneously. It is equal to 1 initially */
    CVAUX_DLL_ENTRY virtual void SetCameraCount( int cameraCount );

    /* Retrieves number of cameras */
    CVAUX_DLL_ENTRY int GetCameraCount() const { return cameraCount; }

    /* Starts cameras calibration */
    CVAUX_DLL_ENTRY virtual bool SetFrames( int totalFrames );
    
    /* Stops cameras calibration */
    CVAUX_DLL_ENTRY virtual void Stop( bool calibrate = false );

    /* Retrieves number of cameras */
    CVAUX_DLL_ENTRY bool IsCalibrated() const { return isCalibrated; }

    /* Feeds another serie of snapshots (one per each camera) to filter.
       Etalon points on these images are found automatically.
       If the function can't locate points, it returns false */
    CVAUX_DLL_ENTRY virtual bool FindEtalon( IplImage** imgs );

    /* The same but takes matrices */
    CVAUX_DLL_ENTRY virtual bool FindEtalon( CvMat** imgs );

    /* Lower-level function for feeding filter with already found etalon points.
       Array of point arrays for each camera is passed. */
    CVAUX_DLL_ENTRY virtual bool Push( const CvPoint2D32f** points = 0 );

    /* Returns total number of accepted frames and, optionally,
       total number of frames to collect */
    CVAUX_DLL_ENTRY virtual int GetFrameCount( int* framesTotal = 0 ) const;

    /* Retrieves camera parameters for specified camera.
       If camera is not calibrated the function returns 0 */
    CVAUX_DLL_ENTRY virtual const CvCamera* GetCameraParams( int idx = 0 ) const;

    CVAUX_DLL_ENTRY virtual const CvStereoCamera* GetStereoParams() const;

    /* Sets camera parameters for all cameras */
    CVAUX_DLL_ENTRY virtual bool SetCameraParams( CvCamera* params );

    /* Saves all camera parameters to file */
    CVAUX_DLL_ENTRY virtual bool SaveCameraParams( const char* filename );
    
    /* Loads all camera parameters from file */
    CVAUX_DLL_ENTRY virtual bool LoadCameraParams( const char* filename );

    /* Calculates epipolar geometry parameters using camera parameters */
    CVAUX_DLL_ENTRY virtual bool CalcEpipolarGeometry();

    /* Undistorts images using camera parameters. Some of src pointers can be NULL. */
    CVAUX_DLL_ENTRY virtual bool Undistort( IplImage** src, IplImage** dst );

    /* Undistorts images using camera parameters. Some of src pointers can be NULL. */
    CVAUX_DLL_ENTRY virtual bool Undistort( CvMat** src, CvMat** dst );

    /* Returns array of etalon points detected/partally detected
       on the latest frame for idx-th camera */
    CVAUX_DLL_ENTRY virtual bool GetLatestPoints( int idx, CvPoint2D32f** pts,
                                                  int* count, bool* found );

    /* Draw the latest detected/partially detected etalon */
    CVAUX_DLL_ENTRY virtual void DrawPoints( IplImage** dst );

    /* Draw the latest detected/partially detected etalon */
    CVAUX_DLL_ENTRY virtual void DrawPoints( CvMat** dst );

    CVAUX_DLL_ENTRY virtual bool Rectify( IplImage** srcarr, IplImage** dstarr );
    CVAUX_DLL_ENTRY virtual bool Rectify( CvMat** srcarr, CvMat** dstarr );

protected:

    enum { MAX_CAMERAS = 3 };

    /* etalon data */
    CvCalibEtalonType  etalonType;
    int     etalonParamCount;
    double* etalonParams;
    int     etalonPointCount;
    CvPoint2D32f* etalonPoints;
    CvSize  imgSize;
    CvMat*  grayImg;
    CvMat*  tempImg;
    CvMemStorage* storage;

    /* camera data */
    int     cameraCount;
    CvCamera cameraParams[MAX_CAMERAS];
    CvStereoCamera stereo;
    CvPoint2D32f* points[MAX_CAMERAS];
    CvMat*  undistMap[MAX_CAMERAS];
    CvMat*  undistImg;
    int     latestCounts[MAX_CAMERAS];
    CvPoint2D32f* latestPoints[MAX_CAMERAS];
    CvMat*  rectMap[MAX_CAMERAS];

    int     maxPoints;
    int     framesTotal;
    int     framesAccepted;
    bool    isCalibrated;
};

#endif

#endif

/* End of file. */
