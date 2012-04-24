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

OPENCVAUXAPI CvSeq* cvSegmentImage( const CvArr* srcarr, CvArr* dstarr,
                                    double canny_threshold,
                                    double ffill_threshold,
                                    CvMemStorage* storage );

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

/*****************************************************************************************/

#define CV_CURRENT_INT( reader ) (*((int *)(reader).ptr))
#define CV_PREV_INT( reader ) (*((int *)(reader).prev_elem))

#define  CV_GRAPH_WEIGHTED_VERTEX_FIELDS() CV_GRAPH_VERTEX_FIELDS()\
    float weight;

#define  CV_GRAPH_WEIGHTED_EDGE_FIELDS() CV_GRAPH_EDGE_FIELDS()

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

typedef enum CvGraphWeightType
{
    CV_NOT_WEIGHTED,
    CV_WEIGHTED_VTX,
    CV_WEIGHTED_EDGE,
    CV_WEIGHTED_ALL
} CvGraphWeightType;


/*****************************************************************************************/


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

#define CLIQUE_TIME_OFF 2
#define CLIQUE_FOUND 1
#define CLIQUE_END   0

OPENCVAUXAPI void cvStartFindCliques( CvGraph* graph, CvCliqueFinder* finder, int reverse, 
                                   int weighted CV_DEFAULT(0),  int weighted_edges CV_DEFAULT(0));
OPENCVAUXAPI int cvFindNextMaximalClique( CvCliqueFinder* finder, int* clock_rest CV_DEFAULT(0) ); 
OPENCVAUXAPI void cvEndFindCliques( CvCliqueFinder* finder );

OPENCVAUXAPI void cvBronKerbosch( CvGraph* graph );                 



/*F///////////////////////////////////////////////////////////////////////////////////////
//
//    Name:    cvSubgraphWeight
//    Purpose: finds weight of subgraph in a graph
//    Context:
//    Parameters:
//      graph - input graph.
//      subgraph - sequence of pairwise different ints.  These are indices of vertices of subgraph.
//      weight_type - describes the way we measure weight.
//            one of the following:
//            CV_NOT_WEIGHTED - weight of a clique is simply its size
//            CV_WEIGHTED_VTX - weight of a clique is the sum of weights of its vertices
//            CV_WEIGHTED_EDGE - the same but edges
//            CV_WEIGHTED_ALL - the same but both edges and vertices
//      weight_vtx - optional vector of floats, with size = graph->total.
//            If weight_type is either CV_WEIGHTED_VTX or CV_WEIGHTED_ALL
//            weights of vertices must be provided.  If weight_vtx not zero
//            these weights considered to be here, otherwise function assumes
//            that vertices of graph are inherited from CvGraphWeightedVtx.
//      weight_edge - optional matrix of floats, of width and height = graph->total.
//            If weight_type is either CV_WEIGHTED_EDGE or CV_WEIGHTED_ALL
//            weights of edges ought to be supplied.  If weight_edge is not zero
//            function finds them here, otherwise function expects
//            edges of graph to be inherited from CvGraphWeightedEdge.
//            If this parameter is not zero structure of the graph is determined from matrix
//            rather than from CvGraphEdge's.  In particular, elements corresponding to
//            absent edges should be zero.
//    Returns:
//      weight of subgraph.
//    Notes:
//F*/
OPENCVAUXAPI float cvSubgraphWeight( CvGraph *graph, CvSeq *subgraph,
                                  CvGraphWeightType weight_type CV_DEFAULT(CV_NOT_WEIGHTED),
                                  CvVect32f weight_vtx CV_DEFAULT(0),
                                  CvMatr32f weight_edge CV_DEFAULT(0) );


/*F///////////////////////////////////////////////////////////////////////////////////////
//
//    Name:    cvFindCliqueEx
//    Purpose: tries to find clique with maximum possible weight in a graph
//    Context:
//    Parameters:
//      graph - input graph.
//      storage - memory storage to be used by the result.
//      is_complementary - optional flag showing whether function should seek for clique
//            in complementary graph.
//      weight_type - describes our notion about weight.
//            one of the following:
//            CV_NOT_WEIGHTED - weight of a clique is simply its size
//            CV_WEIGHTED_VTX - weight of a clique is the sum of weights of its vertices
//            CV_WEIGHTED_EDGE - the same but edges
//            CV_WEIGHTED_ALL - the same but both edges and vertices
//      weight_vtx - optional vector of floats, with size = graph->total.
//            If weight_type is either CV_WEIGHTED_VTX or CV_WEIGHTED_ALL
//            weights of vertices must be provided.  If weight_vtx not zero
//            these weights considered to be here, otherwise function assumes
//            that vertices of graph are inherited from CvGraphWeightedVtx.
//      weight_edge - optional matrix of floats, of width and height = graph->total.
//            If weight_type is either CV_WEIGHTED_EDGE or CV_WEIGHTED_ALL
//            weights of edges ought to be supplied.  If weight_edge is not zero
//            function finds them here, otherwise function expects
//            edges of graph to be inherited from CvGraphWeightedEdge.
//            Note that in case of CV_WEIGHTED_EDGE or CV_WEIGHTED_ALL
//            nonzero is_complementary implies nonzero weight_edge.
//      start_clique - optional sequence of pairwise different ints.  They are indices of
//            vertices that shall be present in the output clique.
//      subgraph_of_ban - optional sequence of (maybe equal) ints.  They are indices of
//            vertices that shall not be present in the output clique.
//      clique_weight_ptr - optional output parameter.  Weight of found clique stored here.
//      num_generations - optional number of generations in evolutionary part of algorithm,
//            zero forces to return first found clique.
//      quality - optional parameter determining degree of required quality/speed tradeoff.
//            Must be in the range from 0 to 9.
//            0 is fast and dirty, 9 is slow but hopefully yields good clique.
//    Returns:
//      sequence of pairwise different ints.
//      These are indices of vertices that form found clique.
//    Notes:
//      in cases of CV_WEIGHTED_EDGE and CV_WEIGHTED_ALL weights should be nonnegative.
//      start_clique has a priority over subgraph_of_ban.
//F*/
OPENCVAUXAPI CvSeq *cvFindCliqueEx( CvGraph *graph, CvMemStorage *storage,
                                 int is_complementary CV_DEFAULT(0),
                                 CvGraphWeightType weight_type CV_DEFAULT(CV_NOT_WEIGHTED),
                                 CvVect32f weight_vtx CV_DEFAULT(0),
                                 CvMatr32f weight_edge CV_DEFAULT(0),
                                 CvSeq *start_clique CV_DEFAULT(0),
                                 CvSeq *subgraph_of_ban CV_DEFAULT(0),
                                 float *clique_weight_ptr CV_DEFAULT(0),
                                 int num_generations CV_DEFAULT(3),
                                 int quality CV_DEFAULT(2) );


#define CV_UNDEF_SC_PARAM         12345 //default value of parameters

#define CV_IDP_BIRCHFIELD_PARAM1  25    
#define CV_IDP_BIRCHFIELD_PARAM2  5
#define CV_IDP_BIRCHFIELD_PARAM3  12
#define CV_IDP_BIRCHFIELD_PARAM4  15
#define CV_IDP_BIRCHFIELD_PARAM5  25


#define  CV_DISPARITY_BIRCHFIELD  0    


/*F///////////////////////////////////////////////////////////////////////////
//
//    Name:    cvFindStereoCorrespondence
//    Purpose: find stereo correspondence on stereo-pair
//    Context:
//    Parameters:
//      leftImage - left image of stereo-pair (format 8uC1).
//      rightImage - right image of stereo-pair (format 8uC1).
//   mode - mode of correspondence retrieval (now CV_DISPARITY_BIRCHFIELD only)
//      dispImage - destination disparity image
//      maxDisparity - maximal disparity 
//      param1, param2, param3, param4, param5 - parameters of algorithm
//    Returns:
//    Notes:
//      Images must be rectified.
//      All images must have format 8uC1.
//F*/
OPENCVAUXAPI void 
cvFindStereoCorrespondence( 
                   const  CvArr* leftImage, const  CvArr* rightImage,
                   int     mode,
                   CvArr*  dispImage,
                   int     maxDisparity,                                
                   double  param1 CV_DEFAULT(CV_UNDEF_SC_PARAM), 
                   double  param2 CV_DEFAULT(CV_UNDEF_SC_PARAM), 
                   double  param3 CV_DEFAULT(CV_UNDEF_SC_PARAM), 
                   double  param4 CV_DEFAULT(CV_UNDEF_SC_PARAM), 
                   double  param5 CV_DEFAULT(CV_UNDEF_SC_PARAM) );

/*****************************************************************************************/
/************ Epiline functions *******************/



typedef struct CvStereoLineCoeff
{
    double Xcoef;
    double XcoefA;
    double XcoefB;
    double XcoefAB;

    double Ycoef;
    double YcoefA;
    double YcoefB;
    double YcoefAB;

    double Zcoef;
    double ZcoefA;
    double ZcoefB;
    double ZcoefAB;
}CvStereoLineCoeff;


typedef struct CvCamera
{
    float   imgSize[2]; /* size of the camera view, used during calibration */
    float   matrix[9]; /* intinsic camera parameters:  [ fx 0 cx; 0 fy cy; 0 0 1 ] */
    float   distortion[4]; /* distortion coefficients - two coefficients for radial distortion
                              and another two for tangential: [ k1 k2 p1 p2 ] */
    float   rotMatr[9];
    float   transVect[3]; /* rotation matrix and transition vector relatively
                             to some reference point in the space. */
}
CvCamera;

typedef struct CvStereoCamera
{
    CvCamera* camera[2]; /* two individual camera parameters */
    float fundMatr[9]; /* fundamental matrix */

    /* New part for stereo */
    CvPoint3D32f epipole[2];
    CvPoint2D32f quad[2][4]; /* coordinates of destination quadrangle after
                                epipolar geometry rectification */
    double coeffs[2][3][3];/* coefficients for transformation */
    CvPoint2D32f border[2][4];
    CvSize warpSize;
    CvStereoLineCoeff* lineCoeffs;
    int needSwapCameras;/* flag set to 1 if need to swap cameras for good reconstruction */
    float rotMatrix[9];
    float transVector[3];
}
CvStereoCamera;


typedef struct CvContourOrientation
{
    float egvals[2];
    float egvects[4];

    float max, min; // minimum and maximum projections
    int imax, imin;
} CvContourOrientation;

#define CV_CAMERA_TO_WARP 1
#define CV_WARP_TO_CAMERA 2

OPENCVAUXAPI CvStatus icvConvertWarpCoordinates(double coeffs[3][3],
                                CvPoint2D32f* cameraPoint,
                                CvPoint2D32f* warpPoint,
                                int direction);

OPENCVAUXAPI CvStatus icvGetSymPoint3D(  CvPoint3D64d pointCorner,
                            CvPoint3D64d point1,
                            CvPoint3D64d point2,
                            CvPoint3D64d *pointSym2);

OPENCVAUXAPI void icvGetPieceLength3D(CvPoint3D64d point1,CvPoint3D64d point2,double* dist);

OPENCVAUXAPI CvStatus icvCompute3DPoint(    double alpha,double betta,
                            CvStereoLineCoeff* coeffs,
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

OPENCVAUXAPI CvStatus icvComputeCoeffForStereo(  CvStereoCamera* stereoCamera);

OPENCVAUXAPI int icvGetCrossPieceVector(CvPoint2D32f p1_start,CvPoint2D32f p1_end,CvPoint2D32f v2_start,CvPoint2D32f v2_end,CvPoint2D32f *cross);
OPENCVAUXAPI int icvGetCrossLineDirect(CvPoint2D32f p1,CvPoint2D32f p2,float a,float b,float c,CvPoint2D32f* cross);
OPENCVAUXAPI float icvDefinePointPosition(CvPoint2D32f point1,CvPoint2D32f point2,CvPoint2D32f point);
OPENCVAUXAPI CvStatus icvStereoCalibration( int numImages,
                            int* nums,
                            CvSize imageSize,
                            CvPoint2D32f* imagePoints1,
                            CvPoint2D32f* imagePoints2,
                            CvPoint3D32f* objectPoints,
                            CvStereoCamera* stereoparams
                           );


OPENCVAUXAPI CvStatus icvComputeRestStereoParams(CvStereoCamera *stereoparams);

OPENCVAUXAPI void cvComputePerspectiveMap(const double coeffs[3][3], CvArr* rectMap);

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
                            CvStereoLineCoeff*    coeffs,
                            int* needSwapCameras);

OPENCVAUXAPI CvStatus icvGetDirectionForPoint(  CvPoint2D64d point,
                                CvMatr64d camMatr,
                                CvPoint3D64d* direct);

OPENCVAUXAPI CvStatus icvGetCrossLines(CvPoint3D64d point11,CvPoint3D64d point12,
                       CvPoint3D64d point21,CvPoint3D64d point22,
                       CvPoint3D64d* midPoint);

OPENCVAUXAPI CvStatus icvComputeStereoLineCoeffs(   CvPoint3D64d pointA,
                                    CvPoint3D64d pointB,
                                    CvPoint3D64d pointCam1,
                                    double gamma,
                                    CvStereoLineCoeff*    coeffs);

OPENCVAUXAPI CvStatus icvComputeFundMatrEpipoles ( CvMatr64d camMatr1, 
                                    CvMatr64d     rotMatr1, 
                                    CvVect64d     transVect1,
                                    CvMatr64d     camMatr2,
                                    CvMatr64d     rotMatr2,
                                    CvVect64d     transVect2,
                                    CvPoint2D64d* epipole1,
                                    CvPoint2D64d* epipole2,
                                    CvMatr64d     fundMatr);

OPENCVAUXAPI void
icvSolveCubic(CvMat* coeffs,CvMat* result);

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

OPENCVAUXAPI void icvComputeeInfiniteProject1(CvMatr64d    rotMatr,
                                     CvMatr64d    camMatr1,
                                     CvMatr64d    camMatr2,
                                     CvPoint2D32f point1,
                                     CvPoint2D32f *point2);

OPENCVAUXAPI void icvComputeeInfiniteProject2(CvMatr64d    rotMatr,
                                     CvMatr64d    camMatr1,
                                     CvMatr64d    camMatr2,
                                     CvPoint2D32f* point1,
                                     CvPoint2D32f point2);

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
                        CvPoint3D64d* epipole1,
                        CvPoint3D64d* epipole2
                        );

OPENCVAUXAPI void icvGetQuadsTransformStruct(  CvStereoCamera* stereoCamera);

OPENCVAUXAPI void icvComputeStereoParamsForCameras(CvStereoCamera* stereoCamera);

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

OPENCVAUXAPI void icvProjectPointToDirect(  CvPoint2D64d point,CvVect64d lineCoeff,
                            CvPoint2D64d* projectPoint);

OPENCVAUXAPI void icvGetDistanceFromPointToDirect( CvPoint2D64d point,CvVect64d lineCoef,double*dist);

OPENCVAUXAPI IplImage* icvCreateIsometricImage( IplImage* src, IplImage* dst,
                              int desired_depth, int desired_num_channels );

OPENCVAUXAPI CvStatus icvCvt_32f_64d( float *src, double *dst, int size );
OPENCVAUXAPI CvStatus icvCvt_64d_32f( double *src, float *dst, int size );

OPENCVAUXAPI void cvDeInterlace( IplImage* frame, IplImage* fieldEven, IplImage* fieldOdd );



OPENCVAUXAPI CvStatus icvSelectBestRt(           int           numImages,
                                    int*          numPoints,
                                    CvSize        imageSize,
                                    CvPoint2D32f* imagePoints1,
                                    CvPoint2D32f* imagePoints2,
                                    CvPoint3D32f* objectPoints,

                                    CvMatr32f     cameraMatrix1,
                                    CvVect32f     distortion1,
                                    CvMatr32f     rotMatrs1,
                                    CvVect32f     transVects1,

                                    CvMatr32f     cameraMatrix2,
                                    CvVect32f     distortion2,
                                    CvMatr32f     rotMatrs2,
                                    CvVect32f     transVects2,

                                    CvMatr32f     bestRotMatr,
                                    CvVect32f     bestTransVect
                                    );

/****************************************************************************************\
*                                   Contour Morphing                                     *
\****************************************************************************************/

/* finds correspondence between two contours */
CvSeq* cvCalcContoursCorrespondence( const CvSeq* contour1,
                                     const CvSeq* contour2, 
                                     CvMemStorage* storage);

/* morphs contours using the pre-calculated correspondence:
   alpha=0 ~ contour1, alpha=1 ~ contour2 */
CvSeq* cvMorphContours( const CvSeq* contour1, const CvSeq* contour2,
                        CvSeq* corr, double alpha,
                        CvMemStorage* storage );

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
*                             Haar-like object detection                                 *
\****************************************************************************************/

#define CV_HAAR_FEATURE_MAX  3

typedef struct CvHaarFeature
{
    int  tilted;
    struct
    {
        CvRect r;
        float weight;
    } rect[CV_HAAR_FEATURE_MAX];
}
CvHaarFeature;


typedef struct CvHaarClassifier
{
    int count;
    CvHaarFeature* haarFeature;
    float* threshold;
    int* left;
    int* right;
    float* alpha;
}
CvHaarClassifier;


typedef struct CvHaarStageClassifier
{
    int  count;
    float threshold;
    CvHaarClassifier* classifier;
}
CvHaarStageClassifier;


typedef struct CvHaarClassifierCascade
{
    int  count;
    CvSize origWindowSize;
    CvHaarStageClassifier* stageClassifier;
}
CvHaarClassifierCascade;


typedef struct CvHidHaarClassifierCascade CvHidHaarClassifierCascade;


/* create faster internal representation of haar classifier cascade */
OPENCVAUXAPI CvHidHaarClassifierCascade*
cvCreateHidHaarClassifierCascade( CvHaarClassifierCascade* cascade,
                                  const CvArr* sumImage CV_DEFAULT(0),
                                  const CvArr* sqSumImage CV_DEFAULT(0),
                                  const CvArr* tiltedSumImage CV_DEFAULT(0),
                                  double scale CV_DEFAULT(1));

OPENCVAUXAPI double
cvGetHaarClassifierCascadeScale( CvHidHaarClassifierCascade* cascade );

OPENCVAUXAPI CvSize
cvGetHaarClassifierCascadeWindowSize( CvHidHaarClassifierCascade* cascade );


OPENCVAUXAPI void
cvSetImagesForHaarClassifierCascade( CvHidHaarClassifierCascade* cascade,
                                     const CvArr* sumImage,
                                     const CvArr* sqSumImage,
                                     const CvArr* tiltedImage,
                                     double scale );

OPENCVAUXAPI int
cvRunHaarClassifierCascade( CvHidHaarClassifierCascade* cascade,
                            CvPoint pt, int startStage CV_DEFAULT(0));

OPENCVAUXAPI void
cvReleaseHidHaarClassifierCascade( CvHidHaarClassifierCascade** cascade );

typedef struct CvAvgComp
{
    CvRect rect;
    int neighbors;
}
CvAvgComp;

#define CV_HAAR_DO_CANNY_PRUNING 1

OPENCVAUXAPI CvSeq*
cvHaarDetectObjects( const IplImage* img,
                     CvHidHaarClassifierCascade* hid_cascade,
                     CvMemStorage* storage, double scale_factor CV_DEFAULT(1.1),
                     int min_neighbors CV_DEFAULT(3), int flags CV_DEFAULT(0));

OPENCVAUXAPI CvHaarClassifierCascade*
cvLoadHaarClassifierCascade( const char* directory CV_DEFAULT("<default_face_cascade>"),
                             CvSize origWindowSize CV_DEFAULT(cvSize(24,24)));

OPENCVAUXAPI void
cvReleaseHaarClassifierCascade( CvHaarClassifierCascade** cascade );


/****************************************************************************************\
*                                  Face eyes&mouth tracking                              *
\****************************************************************************************/


typedef struct CvFaceTracker CvFaceTracker;

#define CV_NUM_FACE_ELEMENTS    3 
enum CV_FACE_ELEMENTS
{
    CV_FACE_MOUTH = 0,
    CV_FACE_LEFT_EYE = 1,
    CV_FACE_RIGHT_EYE = 2
};

OPENCVAUXAPI CvFaceTracker* cvInitFaceTracker(CvFaceTracker* pFaceTracking, const IplImage* imgGray,
                                                CvRect* pRects, int nRects);
OPENCVAUXAPI int cvTrackFace( CvFaceTracker* pFaceTracker, IplImage* imgGray,
                              CvRect* pRects, int nRects,
                              CvPoint* ptRotate, double* dbAngleRotate);
OPENCVAUXAPI void cvReleaseFaceTracker(CvFaceTracker** ppFaceTracker);


typedef struct CvFace
{
    CvRect MouthRect;
    CvRect LeftEyeRect;
    CvRect RightEyeRect;
} CvFaceData;

CvSeq * cvFindFace(IplImage * Image,CvMemStorage* storage);
CvSeq * cvPostBoostingFindFace(IplImage * Image,CvMemStorage* storage);


/****************************************************************************************\
*                                         3D Tracker                                     *
\****************************************************************************************/

typedef unsigned char CvBool;

typedef struct
{
    int id;
    CvPoint p;
} Cv3dTracker2dTrackedObject;

CV_INLINE Cv3dTracker2dTrackedObject cv3dTracker2dTrackedObject(int id, CvPoint p);
CV_INLINE Cv3dTracker2dTrackedObject cv3dTracker2dTrackedObject(int id, CvPoint p)
{
    Cv3dTracker2dTrackedObject r;
    r.id = id;
    r.p = p;
    return r;
}

typedef struct
{
    int id;
    CvPoint3D32f p;             // location of the tracked object
} Cv3dTrackerTrackedObject;

CV_INLINE Cv3dTracker2dTrackedObject cv3dTracker2dTrackedObject(int id, CvPoint p);
CV_INLINE Cv3dTrackerTrackedObject cv3dTrackerTrackedObject(int id, CvPoint3D32f p)
{
    Cv3dTrackerTrackedObject r;
    r.id = id;
    r.p = p;
    return r;
}

typedef struct
{
    CvBool valid;
    float mat[4][4];              /* maps camera coordinates to world coordinates */
    CvPoint2D32f principal_point; /* copied from intrinsics so this structure */
                                  /* has all the info we need */
} Cv3dTrackerCameraInfo;

typedef struct
{
    CvPoint2D32f principal_point;
    float focal_length[2];
    float distortion[4];
} Cv3dTrackerCameraIntrinsics;

OPENCVAUXAPI CvBool cv3dTrackerCalibrateCameras(int num_cameras,
                     const Cv3dTrackerCameraIntrinsics camera_intrinsics[], /* size is num_cameras */
                     CvSize etalon_size,
                     float square_size,
                     IplImage *samples[],                                   /* size is num_cameras */
                     Cv3dTrackerCameraInfo camera_info[]);                  /* size is num_cameras */

OPENCVAUXAPI int  cv3dTrackerLocateObjects(int num_cameras, int num_objects,
                   const Cv3dTrackerCameraInfo camera_info[],        /* size is num_cameras */
                   const Cv3dTracker2dTrackedObject tracking_info[], /* size is num_objects*num_cameras */
                   Cv3dTrackerTrackedObject tracked_objects[]);      /* size is num_objects */
/****************************************************************************************
 tracking_info is a rectangular array; one row per camera, num_objects elements per row.
 The id field of any unused slots must be -1. Ids need not be ordered or consecutive. On
 completion, the return value is the number of objects located; i.e., the number of objects
 visible by more than one camera. The id field of any unused slots in tracked objects is
 set to -1.
****************************************************************************************/


/****************************************************************************************\
*                           Skeletons and Linear-Contour Models                          *
\****************************************************************************************/

typedef enum CvLeeParameters
{
    CV_LEE_INT = 0,
    CV_LEE_FLOAT = 1,
    CV_LEE_DOUBLE = 2,
    CV_LEE_AUTO = -1,
    CV_LEE_ERODE = 0,
    CV_LEE_ZOOM = 1,
    CV_LEE_NON = 2
} CvLeeParameters;

#define CV_NEXT_VORONOISITE2D( SITE ) ((SITE)->edge[0]->site[((SITE)->edge[0]->site[0] == (SITE))])
#define CV_PREV_VORONOISITE2D( SITE ) ((SITE)->edge[1]->site[((SITE)->edge[1]->site[0] == (SITE))])
#define CV_FIRST_VORONOIEDGE2D( SITE ) ((SITE)->edge[0])
#define CV_LAST_VORONOIEDGE2D( SITE ) ((SITE)->edge[1])
#define CV_NEXT_VORONOIEDGE2D( EDGE, SITE ) ((EDGE)->next[(EDGE)->site[0] != (SITE)])
#define CV_PREV_VORONOIEDGE2D( EDGE, SITE ) ((EDGE)->next[2 + ((EDGE)->site[0] != (SITE))])
#define CV_VORONOIEDGE2D_BEGINNODE( EDGE, SITE ) ((EDGE)->node[((EDGE)->site[0] != (SITE))])
#define CV_VORONOIEDGE2D_ENDNODE( EDGE, SITE ) ((EDGE)->node[((EDGE)->site[0] == (SITE))])
#define CV_TWIN_VORONOISITE2D( SITE, EDGE ) ( (EDGE)->site[((EDGE)->site[0] == (SITE))]) 

#define CV_VORONOISITE2D_FIELDS()    \
    struct CvVoronoiNode2D *node[2]; \
    struct CvVoronoiEdge2D *edge[2];

typedef struct CvVoronoiSite2D
{
    CV_VORONOISITE2D_FIELDS()
    struct CvVoronoiSite2D *next[2];
} CvVoronoiSite2D;

#define CV_VORONOIEDGE2D_FIELDS()    \
    struct CvVoronoiNode2D *node[2]; \
    struct CvVoronoiSite2D *site[2]; \
    struct CvVoronoiEdge2D *next[4];

typedef struct CvVoronoiEdge2D
{
    CV_VORONOIEDGE2D_FIELDS()
} CvVoronoiEdge2D;

#define CV_VORONOINODE2D_FIELDS()       \
    CV_SET_ELEM_FIELDS(CvVoronoiNode2D) \
    CvPoint2D32f pt;                    \
    float radius;

typedef struct CvVoronoiNode2D
{
    CV_VORONOINODE2D_FIELDS()
} CvVoronoiNode2D;

#define CV_VORONOIDIAGRAM2D_FIELDS() \
    CV_GRAPH_FIELDS()                \
    CvSet *sites;

typedef struct CvVoronoiDiagram2D
{
    CV_VORONOIDIAGRAM2D_FIELDS()
} CvVoronoiDiagram2D;

/* Computes Voronoi Diagram for given polygons with holes */
OPENCVAUXAPI int  cvVoronoiDiagramFromContour(CvSeq* ContourSeq,
                                           CvVoronoiDiagram2D** VoronoiDiagram,
                                           CvMemStorage* VoronoiStorage,
                                           CvLeeParameters contour_type CV_DEFAULT(CV_LEE_INT),
                                           int contour_orientation CV_DEFAULT(-1),
                                           int attempt_number CV_DEFAULT(10));

/* Computes Voronoi Diagram for domains in given image */
OPENCVAUXAPI int  cvVoronoiDiagramFromImage(IplImage* pImage,
                                         CvSeq** ContourSeq,
                                         CvVoronoiDiagram2D** VoronoiDiagram,
                                         CvMemStorage* VoronoiStorage,
                                         CvLeeParameters regularization_method CV_DEFAULT(CV_LEE_NON),
                                         float approx_precision CV_DEFAULT(CV_LEE_AUTO));

/* Deallocates the storage */
OPENCVAUXAPI void cvReleaseVoronoiStorage(CvVoronoiDiagram2D* VoronoiDiagram,
                                          CvMemStorage** pVoronoiStorage);

/*********************** Linear-Contour Model ****************************/

struct CvLCMEdge;
struct CvLCMNode;

typedef struct CvLCMEdge
{
    CV_GRAPH_EDGE_FIELDS() 
    CvSeq* chain;
    float width;
    int index1;
    int index2;
} CvLCMEdge;

typedef struct CvLCMNode
{
    CV_GRAPH_VERTEX_FIELDS()
    CvContour* contour; 
} CvLCMNode;


/* Computes hybrid model from Voronoi Diagram */
OPENCVAUXAPI CvGraph* cvLinearContorModelFromVoronoiDiagram(CvVoronoiDiagram2D* VoronoiDiagram,
                                                         float maxWidth);

/* Releases hybrid model storage */
OPENCVAUXAPI int cvReleaseLinearContorModelStorage(CvGraph** Graph);

/****************************************************************************************\
*                                   Calibration engine                                   *
\****************************************************************************************/

OPENCVAUXAPI void cvInitPerspectiveTransform( CvSize size, const CvPoint2D32f vertex[4], double matrix[3][3],
                                              CvArr* rectMap );

OPENCVAUXAPI void cvInitStereoRectification( CvStereoCamera* params,
                                             CvArr* rectMap1, CvArr* rectMap2,
                                             int do_undistortion );

#ifdef __cplusplus

typedef enum CvCalibEtalonType
{
    CV_CALIB_ETALON_USER = -1,
    CV_CALIB_ETALON_CHESSBOARD = 0,
    CV_CALIB_ETALON_CHECKERBOARD = CV_CALIB_ETALON_CHESSBOARD
}
CvCalibEtalonType;

class CVAUX_DLL_ENTRY CvCalibFilter
{
public:
    /* Constructor & destructor */
    CvCalibFilter();
    virtual ~CvCalibFilter();

    /* Sets etalon type - one for all cameras.
       etalonParams is used in case of pre-defined etalons (such as chessboard).
       Number of elements in etalonParams is determined by etalonType.
       E.g., if etalon type is CV_ETALON_TYPE_CHESSBOARD then:
         etalonParams[0] is number of squares per one side of etalon
         etalonParams[1] is number of squares per another side of etalon
         etalonParams[2] is linear size of squares in the board in arbitrary units.
       pointCount & points are used in case of
       CV_CALIB_ETALON_USER (user-defined) etalon. */
    virtual bool
        SetEtalon( CvCalibEtalonType etalonType, double* etalonParams,
                   int pointCount = 0, CvPoint2D32f* points = 0 );

    /* Retrieves etalon parameters/or and points */
    virtual CvCalibEtalonType
        GetEtalon( int* paramCount = 0, const double** etalonParams = 0,
                   int* pointCount = 0, const CvPoint2D32f** etalonPoints = 0 ) const;

    /* Sets number of cameras calibrated simultaneously. It is equal to 1 initially */
    virtual void SetCameraCount( int cameraCount );

    /* Retrieves number of cameras */
    int GetCameraCount() const { return cameraCount; }

    /* Starts cameras calibration */
    virtual bool SetFrames( int totalFrames );
    
    /* Stops cameras calibration */
    virtual void Stop( bool calibrate = false );

    /* Retrieves number of cameras */
    bool IsCalibrated() const { return isCalibrated; }

    /* Feeds another serie of snapshots (one per each camera) to filter.
       Etalon points on these images are found automatically.
       If the function can't locate points, it returns false */
    virtual bool FindEtalon( IplImage** imgs );

    /* The same but takes matrices */
    virtual bool FindEtalon( CvMat** imgs );

    /* Lower-level function for feeding filter with already found etalon points.
       Array of point arrays for each camera is passed. */
    virtual bool Push( const CvPoint2D32f** points = 0 );

    /* Returns total number of accepted frames and, optionally,
       total number of frames to collect */
    virtual int GetFrameCount( int* framesTotal = 0 ) const;

    /* Retrieves camera parameters for specified camera.
       If camera is not calibrated the function returns 0 */
    virtual const CvCamera* GetCameraParams( int idx = 0 ) const;

    virtual const CvStereoCamera* GetStereoParams() const;

    /* Sets camera parameters for all cameras */
    virtual bool SetCameraParams( CvCamera* params );

    /* Saves all camera parameters to file */
    virtual bool SaveCameraParams( const char* filename );
    
    /* Loads all camera parameters from file */
    virtual bool LoadCameraParams( const char* filename );

    /* Undistorts images using camera parameters. Some of src pointers can be NULL. */
    virtual bool Undistort( IplImage** src, IplImage** dst );

    /* Undistorts images using camera parameters. Some of src pointers can be NULL. */
    virtual bool Undistort( CvMat** src, CvMat** dst );

    /* Returns array of etalon points detected/partally detected
       on the latest frame for idx-th camera */
    virtual bool GetLatestPoints( int idx, CvPoint2D32f** pts,
                                                  int* count, bool* found );

    /* Draw the latest detected/partially detected etalon */
    virtual void DrawPoints( IplImage** dst );

    /* Draw the latest detected/partially detected etalon */
    virtual void DrawPoints( CvMat** dst );

    virtual bool Rectify( IplImage** srcarr, IplImage** dstarr );
    virtual bool Rectify( CvMat** srcarr, CvMat** dstarr );

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

    /* Added by Valery */
    //CvStereoCamera stereoParams;

    int     maxPoints;
    int     framesTotal;
    int     framesAccepted;
    bool    isCalibrated;
};

#endif


/****************************************************************************************\
*                                    Utility Functions                                   *
\****************************************************************************************/

/* helper functions for RNG initialization and accurate time measurement: x86 only */
OPENCVAUXAPI int64  cvGetTickCount( void );
OPENCVAUXAPI double cvGetTickFrequency(); 

#ifdef __cplusplus
#include "cvaux.hpp"
#include "cvmat.hpp"
#endif

#endif

/* End of file. */
