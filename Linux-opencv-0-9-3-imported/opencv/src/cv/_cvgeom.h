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

#ifndef _IPCVGEOM_H_
#define _IPCVGEOM_H_

#define CV_CALIPERS_MAXHEIGHT      0
#define CV_CALIPERS_MINAREARECT    1
#define CV_CALIPERS_MAXDIST        2

CvStatus  icvConvexHull( CvPoint* points,
                         int num_points,
                         CvRect* bound_rect,
                         int orientation,
                         int* hull,
                         int* hullsize );

CvStatus icvContourConvexHull( CvSeq* contour,
                               int orientation,
                               CvMemStorage* storage,
                               CvSeq** hull );


int icvCheckContourConvexity( CvSeq* contour );


CvStatus  icvConvexHull_Approx( CvPoint* points,
                                  int num_points,
                                  CvRect* bound_rect,
                                  int bandwidth,
                                  int orientation, 
                                  int* hullpoints,
                                  int* hullsize );

CvStatus  icvConvexHull_Approx_Contour( CvSeq* contour,
                                          int bandwidth,
                                          int orientation,
                                          CvMemStorage* storage,
                                          CvSeq** hull);

CvStatus  icvConvexHull_Exact( CvPoint* points,
                               int num_points,
                               int orientation,
                               int* hullpoints,
                               int* hullsize );

CvStatus  icvConvexityDefects( CvSeq* contour, CvSeq* convexhull,
                               CvMemStorage* storage, CvSeq** defects );


/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:    icvRotatingCalipers
//    Purpose:
//      Rotating calipers algorithm with some applications
//
//    Context:
//    Parameters:
//      points      - convex hull vertices ( any orientation )
//      n           - number of vertices
//      mode        - concrete application of algorithm 
//                    can be  CV_CALIPERS_MAXDIST   or   
//                            CV_CALIPERS_MINAREARECT  
//      left, bottom, right, top - indexes of extremal points
//      out         - output info.
//                    In case CV_CALIPERS_MAXDIST it points to float value - 
                      maximal height of polygon.
                      In case CV_CALIPERS_MINAREARECT
                      ((CvPoint2D32f*)out)[0] - corner 
                      ((CvPoint2D32f*)out)[1] - vector1
                      ((CvPoint2D32f*)out)[0] - corner2
                        
                        ^
                        |
                vector2 |
                        |
                        |____________\
                      corner         /
                                 vector1
  
//    Returns:
//    Notes:
//F*/
CvStatus  icvRotatingCalipers( CvPoint* points, int n, int mode,
                                 int left, int bottom, int right, int top, char* out );


/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:    icvCalcContourPerimeter
//    Purpose:
//      Calculates contour perimeter, finds minimal edge and maximal egde lengths
//    Context:
//    Parameters:
//      contour          - source contour
//      _perimeter       - pointer to resultant perimeter
//      _min_edge_length - pointer to minimum edge length
//      _max_edge_length - pointer to maximum edge length
//
//    Returns:
//      IPP_NO_ERR if ok, error code else
//    Notes:
//F*/  
/*
CvStatus   icvCalcContourPerimeter( CvSeq* contour,
                                      double* _perimeter,
                                      double* _min_edge_length,
                                      double* _max_edge_length );
*/

/* Finds distance between two points */
CV_INLINE  float  icvDistanceL2_32s( CvPoint pt1, CvPoint pt2 )
{
    int dx = pt2.x - pt1.x;
    int dy = pt2.y - pt1.y;

    return cvSqrt( (float)(dx*dx + dy*dy));
}

/* Finds distance between two points */
CV_INLINE  float  icvDistanceL2_32f( CvPoint2D32f pt1, CvPoint2D32f pt2 )
{
    float dx = pt2.x - pt1.x;
    float dy = pt2.y - pt1.y;

    return cvSqrt( (float)(dx*dx + dy*dy));
}


CV_INLINE CvPoint2D32f icvCvtPoint32s_32f( CvPoint  pt )
{
    CvPoint2D32f res_pt;
    res_pt.x = (float)pt.x;
    res_pt.y = (float)pt.y;

    return res_pt;
}


CV_INLINE CvPoint icvCvtPoint32f_32s( CvPoint2D32f  pt )
{
    CvPoint res_pt;
    res_pt.x = cvRound( pt.x );
    res_pt.y = cvRound( pt.y );

    return res_pt;
}


CV_INLINE int icvIsPtInCircle( CvPoint2D32f  pt,
                               CvPoint2D32f  center,
                               float  radius )
{
    pt.x -= center.x;
    pt.y -= center.y;
    return  pt.x*pt.x + pt.y*pt.y <= radius*radius;
}


int icvIsPtInCircle3( CvPoint2D32f pt, CvPoint2D32f a,
                      CvPoint2D32f b, CvPoint2D32f c );


CV_INLINE CvPoint2D32f icvMidPoint( CvPoint2D32f pt1, CvPoint2D32f pt2 )
{
    CvPoint2D32f  mid_pt;
    mid_pt.x = (pt1.x + pt2.x)*0.5f;
    mid_pt.y = (pt1.y + pt2.y)*0.5f;
    return  mid_pt;
}


int  icvIntersectLines( double x1, double dx1, double y1, double dy1,
                        double x2, double dx2, double y2, double dy2,
                        double* t2 );


CvStatus   icvFindCircle( CvPoint2D32f pt0, CvPoint2D32f pt1,
                          CvPoint2D32f pt2, CvPoint2D32f* center,
                          float* radius );


CvStatus icvProject3D( CvPoint3D32f* points3D, int count,
                       CvPoint2D32f* points2D, int index1, int index2);


void icvCreateCenterNormalLine( CvSubdiv2DEdge edge, double* a, double* b, double* c );

void icvIntersectLines3( double* a0, double* b0, double* c0,
                         double* a1, double* b1, double* c1,
                         CvPoint2D32f* point );


#define _CV_BINTREE_LIST()                                          \
   struct _CvTrianAttr* prev_v;   /* pointer to the parent  element on the previous level of the tree  */    \
   struct _CvTrianAttr* next_v1;   /* pointer to the child  element on the next level of the tree  */        \
   struct _CvTrianAttr* next_v2;   /* pointer to the child  element on the next level of the tree  */        

typedef struct _CvTrianAttr
{
   CvPoint pt;    /* Coordinates x and y of the vertex  which don't lie on the base line LMIAT  */
   char sign;             /*  sign of the triangle   */
   double area;       /*   area of the triangle    */
   double r1;   /*  The ratio of the height of triangle to the base of the triangle  */
   double r2;  /*   The ratio of the projection of the left side of the triangle on the base to the base */
   _CV_BINTREE_LIST()    /* structure double list   */
}
_CvTrianAttr;


CvStatus  icvCalcTriAttr(CvSeq *contour_h,CvPoint t2,CvPoint t1,int n1,
						   CvPoint t3, int n3, double *s, double *s_c,
						   double *h, double *a, double *b);

CvStatus  icvMemCopy (double **buf1, double **buf2, double **buf3, int *b_max);

#endif /*_IPCVGEOM_H_*/

/* End of file. */
