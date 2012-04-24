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

#include "_cvaux.h"
#include "_cvwrap.h"

#include "cvtypes.h"
#include <float.h>
#include <limits.h>

#define EPS64D 0.000000001

//CvPoint3D64d exPointA[1000];
//CvPoint3D64d exPointB[1000];
//CvPoint3D64d exPointC[1000];

//int numberABCPoints = 0;


CvStatus icvGetSymPoint3D(  CvPoint3D64d pointCorner,
                            CvPoint3D64d point1,
                            CvPoint3D64d point2,
                            CvPoint3D64d *pointSym2)
{
    double len1,len2;
    double alpha;
    icvGetPieceLength3D(pointCorner,point1,&len1);
    if( len1 < EPS64D )
    {
        return CV_BADARG_ERR;
    }
    icvGetPieceLength3D(pointCorner,point2,&len2);
    alpha = len2 / len1;

    pointSym2->x = pointCorner.x + alpha*(point1.x - pointCorner.x);
    pointSym2->y = pointCorner.y + alpha*(point1.y - pointCorner.y);
    pointSym2->z = pointCorner.z + alpha*(point1.z - pointCorner.z);
    return CV_NO_ERR;
}

CvContourOrientation FindPrincipalAxes(CvSeq* contour)
{
    CvContourOrientation orient;
    CvMoments mom;
    cvContourMoments(contour, &mom);
    float matrix[4];
    matrix[0] = (float)(mom.mu20/mom.m00);
    matrix[1] = (float)(mom.mu11/mom.m00);
    matrix[2] = (float)(mom.mu11/mom.m00);
    matrix[3] = (float)(mom.mu02/mom.m00);
    CvMat mat = cvMat(2, 2, CV_MAT32F, matrix);
    CvMat evalues = cvMat(2, 1, CV_MAT32F, &orient.egvals);
    CvMat evectors = cvMat(2, 2, CV_MAT32F, &orient.egvects);
    cvEigenVV(&mat, &evectors, &evalues, 1e-6);

    CvSeqReader reader;
    cvStartReadSeq(contour, &reader, 0);
    orient.max = -FLT_MAX;
    orient.min = FLT_MAX;
    float* line = orient.egvals[0] > orient.egvals[1] ? &orient.egvects[0] : &orient.egvects[2];
    for(int i = 0; i < contour->total; i++)
    {
        CvPoint p;
        CV_READ_SEQ_ELEM(p, reader);
        float t = p.x*line[0] + p.y*line[1];
        if(t > orient.max)
        {
            orient.max = t;
            orient.imax = i;
        }
            
        if(t < orient.min)
        {
            orient.min = t;
            orient.imin = i;
        }
    }

    return orient;
}

void DrawEdges(IplImage* image, CvSeq* seq)
{
    CvSeqReader reader;
    cvStartReadSeq(seq, &reader);

    CvPoint p1, p2;
    CV_READ_SEQ_ELEM(p1, reader);
    for(int i = 0; i < seq->total; i++)
    {
        CV_READ_SEQ_ELEM(p2, reader);
        cvLine(image, p1, p2, CV_RGB(rand()%255, rand()%255, rand()%255), 1);
        p1 = p2;
    }
}

/* FindLineOnImage */
CvStatus icvFindLineOnImage(IplImage* image,CvPoint* point1,CvPoint* point2,int *num)
{

    int width  = image->width;
    int height = image->height;
    int y,x;
    
    IplImage* hsv = cvCreateImage(cvSize(width,height), IPL_DEPTH_8U, 3);

    IplImage* gray = cvCreateImage(cvSize(width,height), IPL_DEPTH_8U, 1);
    unsigned char* graydata = (unsigned char*)gray->imageData;

    int stepGray = gray->widthStep;

    cvCvtColor(image,hsv,CV_BGR2HSV);
    uchar* data1 = (uchar*)hsv->imageData;
    int widthStep1 = hsv->widthStep;
    //memcpy(image->imageData,hsv->imageData,width*height*3);
    /* Processing images */
    
    //int baseR = 107,baseG = 110,baseB = 107;
    //int sR = 10, sG = 40, sB = 10;

    //int* xObj1 = (int*)calloc(height,sizeof(int));
    //int* xObj2 = (int*)calloc(height,sizeof(int));

    for( y = 0; y < height; y++ )
    {
        //xObj1[y] = 0;
        for( x = 0; x < width; x++ )
        {
            int B = data1[y * widthStep1 + x * 3 + 0];
            int G = data1[y * widthStep1 + x * 3 + 1];
            //int R = data1[y * widthStep1 + x * 3 + 2];
            int H, S;

            H = B;
            S = G;

            if( H > 90 && H < 140 &&
                S > 75)
            {
                graydata[y*stepGray+x] = 255;
            }
            else
            {
                graydata[y*stepGray+x] = 0;                
            }
        }
    }

    /*  */
    CvMemStorage* storage = cvCreateMemStorage(0);
    //IplImage* temp = iplCloneImage(cameraImage1.GetImage());
    CvSeq* contour;

    cvFindContours(gray, storage, &contour, sizeof(CvContour), CV_RETR_CCOMP, CV_CHAIN_APPROX_NONE);
    //CvSeq* longest = contour;
    *num = 0;
    for(CvSeq* seq = contour; seq && *num == 0; seq = seq->h_next)
    {
        //CvSeq* _approx = cvApproxPoly(seq, sizeof(CvContour), storage, CV_POLY_APPROX_DP, 2, 0);
        //iplSet(temp, 0);
        //DrawEdges(temp, _approx);

        CvContourOrientation orient = FindPrincipalAxes(seq);

        if( orient.egvals[0]>0.00001 || orient.egvals[0]<-0.00001 &&
            orient.egvals[1]>0.00001 || orient.egvals[1]<-0.00001 && *num == 0)
        {
            if( (orient.egvals[0]/orient.egvals[1] > 8)  ||
                (orient.egvals[1]/orient.egvals[0] > 8))
            {
                //DrawEdges(cameraImage1.GetImage(), _approx);
                /* Get length */
                CvPoint wing_p1 = *(CvPoint*)cvGetSeqElem(seq, orient.imin);
                CvPoint wing_p2 = *(CvPoint*)cvGetSeqElem(seq, orient.imax);
                float dist = (float)(wing_p1.x-wing_p2.x)*(float)(wing_p1.x-wing_p2.x) + 
                             (float)(wing_p1.y-wing_p2.y)*(float)(wing_p1.y-wing_p2.y);

                if( dist > 60*60 )
                {
                    *point1 = wing_p1;
                    *point2 = wing_p2;
                    *num = 1;

                    //cvCircle(image, wing_p1, 3, CV_RGB(0, 255, 0), 1);
                    //cvCircle(image, wing_p2, 3, CV_RGB(0, 255, 0), 1);
                    //cvLineAA(image,wing_p1,wing_p2,CV_RGB(0,255,0,));
                }
            }
        }
    }

    cvReleaseMemStorage(&storage );
    //free(xObj1);
    //free(xObj2);

    cvReleaseImage(&hsv);
    cvReleaseImage(&gray);

    return CV_NO_ERR;
}



/* Compute 3D point for scanline and alpha betta */
CvStatus icvCompute3DPoint(    double alpha,double betta,
                            StereoLineCoeff* coeffs,
                            CvPoint3D64d* point)
{
    double partX;
    double partY;
    double partZ;
    double partAll;
    double invPartAll;

    double alphabetta = alpha*betta;

    partAll = coeffs->Apart        + coeffs->ApartA *alpha +
              coeffs->ApartB*betta + coeffs->ApartAB*alphabetta;

    partX   = coeffs->Xpart        + coeffs->XpartA *alpha +
              coeffs->XpartB*betta + coeffs->XpartAB*alphabetta;

    partY   = coeffs->Ypart        + coeffs->YpartA *alpha +
              coeffs->YpartB*betta + coeffs->YpartAB*alphabetta;
    
    partZ   = coeffs->Zpart        + coeffs->ZpartA *alpha +
              coeffs->ZpartB*betta + coeffs->ZpartAB*alphabetta;

    invPartAll = 1.0 / partAll;
    
    point->x = partX * invPartAll;
    point->y = partY * invPartAll;
    point->z = partZ * invPartAll;

    return CV_NO_ERR;
}

/*--------------------------------------------------------------------------------------*/

/* Compute rotate matrix and trans vector for change system */
CvStatus icvCreateConvertMatrVect( CvMatr64d     rotMatr1,
                                CvMatr64d     transVect1,
                                CvMatr64d     rotMatr2,
                                CvMatr64d     transVect2,
                                CvMatr64d     convRotMatr,
                                CvMatr64d     convTransVect)
{
    double invRotMatr2[9];
    double tmpVect[3];


    icvInvertMatrix_64d(rotMatr2,3,invRotMatr2);
    /* Test for error */

    icvMulMatrix_64d(   rotMatr1,
                        3,3,
                        invRotMatr2,
                        3,3,
                        convRotMatr);

    icvMulMatrix_64d(   convRotMatr,
                        3,3,
                        transVect2,
                        1,3, 
                        tmpVect);
    
    icvSubVector_64d(transVect1,tmpVect,convTransVect,3);

    
    return CV_NO_ERR;
}

/*--------------------------------------------------------------------------------------*/

/* Compute point coordinates in other system */
CvStatus icvConvertPointSystem(CvPoint3D64d  M2,
                            CvPoint3D64d* M1,
                            CvMatr64d     rotMatr,
                            CvMatr64d     transVect
                            )
{
    double tmpVect[3];

    icvMulMatrix_64d(   rotMatr,
                        3,3,
                        (double*)&M2,
                        1,3, 
                        tmpVect);

    icvAddVector_64d(tmpVect,transVect,(double*)M1,3);
    
    return CV_NO_ERR;
}
/*--------------------------------------------------------------------------------------*/
CvStatus icvComputeCoeffForStereo( double quad1[4][2],
                                double quad2[4][2],
                                int    numScanlines,
                                CvMatr64d    camMatr1,
                                CvMatr64d    rotMatr1,
                                CvMatr64d    transVect1,
                                CvMatr64d    camMatr2,
                                CvMatr64d    rotMatr2,
                                CvMatr64d    transVect2,
                                StereoLineCoeff*    startCoeffs)
{
    /* For each pair */

    CvPoint2D64d point1;
    CvPoint2D64d point2;
    CvPoint2D64d point3;
    CvPoint2D64d point4;

    int currLine;
    for( currLine = 0; currLine < numScanlines; currLine++ )
    {
        /* Compute points */
        double alpha = ((double)currLine)/((double)(numScanlines)); /* maybe - 1 */

        point1.x = (1.0 - alpha) * quad1[0][0] + alpha * quad1[3][0];
        point1.y = (1.0 - alpha) * quad1[0][1] + alpha * quad1[3][1];

        point2.x = (1.0 - alpha) * quad1[1][0] + alpha * quad1[2][0];
        point2.y = (1.0 - alpha) * quad1[1][1] + alpha * quad1[2][1];
        
        point3.x = (1.0 - alpha) * quad2[0][0] + alpha * quad2[3][0];
        point3.y = (1.0 - alpha) * quad2[0][1] + alpha * quad2[3][1];

        point4.x = (1.0 - alpha) * quad2[1][0] + alpha * quad2[2][0];
        point4.y = (1.0 - alpha) * quad2[1][1] + alpha * quad2[2][1];

        /* We can compute coeffs for this line */
        icvComCoeffForLine(    point1,
                            point2,
                            point3,
                            point4,
                            camMatr1,
                            rotMatr1,
                            transVect1,
                            camMatr2,
                            rotMatr2,
                            transVect2,
                            &startCoeffs[currLine]);
    }
    return CV_NO_ERR;    
}
/*--------------------------------------------------------------------------------------*/
CvStatus icvComCoeffForLine(   CvPoint2D64d point1,
                            CvPoint2D64d point2,
                            CvPoint2D64d point3,
                            CvPoint2D64d point4,
                            CvMatr64d    camMatr1,
                            CvMatr64d    rotMatr1,
                            CvMatr64d    transVect1,
                            CvMatr64d    camMatr2,
                            CvMatr64d    rotMatr2,
                            CvMatr64d    transVect2,
                            StereoLineCoeff* coeffs)
{
    /* Get direction for all points */
    /* Direction for camera 1 */
    
    double direct1[3];
    double direct2[3];
    double camPoint1[3];
    
    double directS3[3];
    double directS4[3];
    double direct3[3];
    double direct4[3];
    double camPoint2[3];
    
    icvGetDirectionForPoint(   point1,
                            camMatr1,
                            (CvPoint3D64d*)direct1);
    
    icvGetDirectionForPoint(   point2,
                            camMatr1,
                            (CvPoint3D64d*)direct2);

    /* Direction for camera 2 */

    icvGetDirectionForPoint(   point3,
                            camMatr2,
                            (CvPoint3D64d*)directS3);
    
    icvGetDirectionForPoint(   point4,
                            camMatr2,
                            (CvPoint3D64d*)directS4);

    /* Create convertion for camera 2: two direction and camera point */
    
    double convRotMatr[9];
    double convTransVect[3];

    icvCreateConvertMatrVect(  rotMatr1,
                            transVect1,
                            rotMatr2,
                            transVect2,
                            convRotMatr,
                            convTransVect);

    double zeroVect[3];
    zeroVect[0] = zeroVect[1] = zeroVect[2] = 0.0;
    camPoint1[0] = camPoint1[1] = camPoint1[2] = 0.0;
    
    icvConvertPointSystem(*((CvPoint3D64d*)directS3),(CvPoint3D64d*)direct3,convRotMatr,convTransVect);
    icvConvertPointSystem(*((CvPoint3D64d*)directS4),(CvPoint3D64d*)direct4,convRotMatr,convTransVect);
    icvConvertPointSystem(*((CvPoint3D64d*)zeroVect),(CvPoint3D64d*)camPoint2,convRotMatr,convTransVect);

    /* We can compute cross of all directions: t.e. xA,yA,zA,xB, ..., cZ */

    double pointA[3];
    double pointB[3];
    double pointC[3];

    /* Compute point A: xA,yA,zA */
    icvGetCrossLines(*((CvPoint3D64d*)camPoint1),*((CvPoint3D64d*)direct1),
                  *((CvPoint3D64d*)camPoint2),*((CvPoint3D64d*)direct3),
                  (CvPoint3D64d*)pointA);

    /* Compute point B: xB,yB,zB */
    icvGetCrossLines(*((CvPoint3D64d*)camPoint1),*((CvPoint3D64d*)direct2),
                  *((CvPoint3D64d*)camPoint2),*((CvPoint3D64d*)direct3),
                  (CvPoint3D64d*)pointB);
                            
    /* Compute point C: xC,yC,zC */
    icvGetCrossLines(*((CvPoint3D64d*)camPoint1),*((CvPoint3D64d*)direct2),
                  *((CvPoint3D64d*)camPoint2),*((CvPoint3D64d*)direct4),
                  (CvPoint3D64d*)pointC);

    /* Add new point */


    /* !!! MEW VERSION WITH CORRECTION 3D POINTS !!! */
    /* correction points */
    CvPoint3D64d pointNewA; pointNewA.x = pointNewA.y = 0;
    CvPoint3D64d pointNewC;

    icvGetSymPoint3D(   *((CvPoint3D64d*)camPoint1),
                        *((CvPoint3D64d*)pointA),
                        *((CvPoint3D64d*)pointB),
                        &pointNewA);

    icvGetSymPoint3D(   *((CvPoint3D64d*)camPoint2),
                        *((CvPoint3D64d*)pointC),
                        *((CvPoint3D64d*)pointB),
                        &pointNewC);

    coeffs->pointA = pointNewA;
    coeffs->pointB = *((CvPoint3D64d*)pointB);
    coeffs->pointC = pointNewC;
    coeffs->pointCam1 = *((CvPoint3D64d*)camPoint1);
    coeffs->pointCam2 = *((CvPoint3D64d*)camPoint2);

    icvComputeStereoLineCoeffs( pointNewA,
                                *((CvPoint3D64d*)pointB),
                                pointNewC,
                                *((CvPoint3D64d*)camPoint1),
                                *((CvPoint3D64d*)camPoint2),
                                coeffs);

    return CV_NO_ERR;


}


/*--------------------------------------------------------------------------------------*/

CvStatus icvGetDirectionForPoint(  CvPoint2D64d point,
                                CvMatr64d camMatr,
                                CvPoint3D64d* direct)
{
    /*  */
    double invMatr[9];
    
    /* Invert matrix */

    icvInvertMatrix_64d(camMatr,3,invMatr);
    /* TEST FOR ERRORS */


    double vect[3];
    vect[0] = point.x;
    vect[1] = point.y;
    vect[2] = 1;

    /* Mul matr */
    
    icvMulMatrix_64d(   invMatr,
                        3,3,
                        vect,
                        1,3, 
                        (double*)direct);

    return CV_NO_ERR;    
}


/*--------------------------------------------------------------------------------------*/

CvStatus icvGetCrossLines(CvPoint3D64d point11,CvPoint3D64d point12,
                       CvPoint3D64d point21,CvPoint3D64d point22,
                       CvPoint3D64d* midPoint)
{
    double xM,yM,zM;
    double xN,yN,zN;

    double xA,yA,zA;
    double xB,yB,zB;

    double xC,yC,zC;
    double xD,yD,zD;

    xA = point11.x;
    yA = point11.y;
    zA = point11.z;

    xB = point12.x;
    yB = point12.y;
    zB = point12.z;

    xC = point21.x;
    yC = point21.y;
    zC = point21.z;

    xD = point22.x;
    yD = point22.y;
    zD = point22.z;

    double a11,a12,a21,a22;
    double b1,b2;

    a11 =  (xB-xA)*(xB-xA)+(yB-yA)*(yB-yA)+(zB-zA)*(zB-zA);
    a12 = -(xD-xC)*(xB-xA)-(yD-yC)*(yB-yA)-(zD-zC)*(zB-zA);
    a21 =  (xB-xA)*(xD-xC)+(yB-yA)*(yD-yC)+(zB-zA)*(zD-zC);
    a22 = -(xD-xC)*(xD-xC)-(yD-yC)*(yD-yC)-(zD-zC)*(zD-zC);
    b1  = -( (xA-xC)*(xB-xA)+(yA-yC)*(yB-yA)+(zA-zC)*(zB-zA) );
    b2  = -( (xA-xC)*(xD-xC)+(yA-yC)*(yD-yC)+(zA-zC)*(zD-zC) );

    double delta;
    double deltaA,deltaB;
    double alpha,betta;

    delta  = a11*a22-a12*a21;
    
    if( fabs(delta) < EPS64D )
    {
        //return ERROR;
    }

    deltaA = b1*a22-b2*a12;
    deltaB = a11*b2-b1*a21;

    alpha = deltaA / delta;
    betta = deltaB / delta;

    xM = xA+alpha*(xB-xA);
    yM = yA+alpha*(yB-yA);
    zM = zA+alpha*(zB-zA);

    xN = xC+betta*(xD-xC);
    yN = yC+betta*(yD-yC);
    zN = zC+betta*(zD-zC);

    /* Compute middle point */

    midPoint->x = (xM + xN) * 0.5;
    midPoint->y = (yM + yN) * 0.5;
    midPoint->z = (zM + zN) * 0.5;

    return CV_NO_ERR;
}

/*--------------------------------------------------------------------------------------*/

/* Compute  */

/* Computation */
CvStatus icvComputeStereoLineCoeffs(   CvPoint3D64d pointA,
                                    CvPoint3D64d pointB,
                                    CvPoint3D64d pointC,
                                    CvPoint3D64d pointCam1,
                                    CvPoint3D64d pointCam2,
                                    StereoLineCoeff*    coeffs)
{
    double x1,y1,z1;
    double x2,y2,z2;

    x1 = pointCam1.x;
    y1 = pointCam1.y;
    z1 = pointCam1.z;

    x2 = pointCam2.x;
    y2 = pointCam2.y;
    z2 = pointCam2.z;

    double xA,yA,zA;
    double xB,yB,zB;
    double xC,yC,zC;

    xA = pointA.x;
    yA = pointA.y;
    zA = pointA.z;

    xB = pointB.x;
    yB = pointB.y;
    zB = pointB.z;

    xC = pointC.x;
    yC = pointC.y;
    zC = pointC.z;

    /* Pre pre end part */
    /* Compute A,B,C */
    double deltaA,deltaB,deltaC,deltaD;
    double delta;

    deltaA = -(yA*zB + yB*zC + zA*yC - zB*yC - zA*yB - yA*zC);
    deltaB =  (xA*zB + xB*zC + zA*xC - zB*xC - zA*xB - xA*zC);
    deltaC = -(xA*yB + xB*yC + yA*xC - yB*xC - yA*xB - xA*yC);
    deltaD =   xA*yB*zC + zA*xB*yC + yA*zB*xC - zA*yB*xC - yA*xB*zC - xA*zB*yC;

    delta  = deltaA + deltaB + deltaC + deltaD;

    double A,B,C,D;

    A = deltaA / delta;
    B = deltaB / delta;
    C = deltaC / delta;
    D = deltaD / delta;

    /* Pre end part */
    double a11,a12,a21,a22;
    double b11,b12,b21,b22;
    double c11,c12,c21,c22;
    double d11,d12,d21,d22;

    a11 = yA*C - zA*B - y1*C + z1*B;
    a12 = (yB-yA)*C - (zB-zA)*B;
    a21 = yB*C - zB*B - y2*C + z2*B;
    a22 = (yC-yB)*C - (zC-zB)*B;

    b11 = -xA*C + zA*A + x1*C - z1*A;
    b12 = -(xB-xA)*C + (zB-zA)*A;
    b21 = -xB*C + zB*A + x2*C - z2*A;
    b22 = -(xC-xB)*C + (zC-zB)*A;

    c11 = xA*B - yA*A - x1*B + y1*A;
    c12 = (xB-xA)*B - (yB-yA)*A;
    c21 = xB*B - yB*A - x2*B + y2*A;
    c22 = (xC-xB)*B - (yC-yB)*A;

    d11 = xA*(y1*C-z1*B)-yA*(x1*C-z1*A)+zA*(x1*B-y1*A);
    d12 = (xB-xA)*(y1*C-z1*B)-(yB-yA)*(x1*C-z1*A)+(zB-zA)*(x1*B-y1*A);
    d21 = xB*(y2*C-z2*B)-yB*(x2*C-z2*A)+zB*(x2*B-y2*A);
    d22 = (xC-xB)*(y2*C-z2*B)-(yC-yB)*(x2*C-z2*A)+(zC-zB)*(x2*B-y2*A);

    /* End part */
    double Detpart;
    double DetpartA;
    double DetpartB;
    double DetpartAB;

    double DetXpart;
    double DetXpartA;
    double DetXpartB;
    double DetXpartAB;

    double DetYpart;
    double DetYpartA;
    double DetYpartB;
    double DetYpartAB;

    double DetZpart;
    double DetZpartA;
    double DetZpartB;
    double DetZpartAB;

    Detpart   = A*b11*c21 + C*a11*b21 + B*c11*a21 - C*b11*a21 - B*a11*c21 - A*c11*b21;
    DetpartA  = A*b12*c21 + C*a12*b21 + B*c12*a21 - C*b12*a21 - B*a12*c21 - A*c12*b21;
    DetpartB  = A*b11*c22 + C*a11*b22 + B*c11*a22 - C*b11*a22 - B*a11*c22 - A*c11*b22;
    DetpartAB = A*b12*c22 + C*a12*b22 + B*c12*a22 - C*b12*a22 - B*a12*c22 - A*c12*b22;

    DetXpart   = - D*b11*c21 - C*d11*b21 - B*c11*d21 + C*b11*d21 + B*d11*c21 + D*c11*b21;
    DetXpartA  = - D*b12*c21 - C*d12*b21 - B*c12*d21 + C*b12*d21 + B*d12*c21 + D*c12*b21;
    DetXpartB  = - D*b11*c22 - C*d11*b22 - B*c11*d22 + C*b11*d22 + B*d11*c22 + D*c11*b22;
    DetXpartAB = - D*b12*c22 - C*d12*b22 - B*c12*d22 + C*b12*d22 + B*d12*c22 + D*c12*b22;
    
    DetYpart   = - A*d11*c21 - C*a11*d21 - D*c11*d21 + C*d11*a21 + D*a11*c21 + A*c11*d21;
    DetYpartA  = - A*d12*c21 - C*a12*d21 - D*c12*d21 + C*d12*a21 + D*a12*c21 + A*c12*d21;
    DetYpartB  = - A*d11*c22 - C*a11*d22 - D*c11*d22 + C*d11*a22 + D*a11*c22 + A*c11*d22;
    DetYpartAB = - A*d12*c22 - C*a12*d22 - D*c12*d22 + C*d12*a22 + D*a12*c22 + A*c12*d22;

    DetZpart   = - A*b11*d21 - D*a11*b21 - B*d11*a21 + D*b11*a21 + B*a11*d21 + A*d11*b21;
    DetZpartA  = - A*b12*d21 - D*a12*b21 - B*d12*a21 + D*b12*a21 + B*a12*d21 + A*d12*b21;
    DetZpartB  = - A*b11*d22 - D*a11*b22 - B*d11*a22 + D*b11*a22 + B*a11*d22 + A*d11*b22;
    DetZpartAB = - A*b12*d22 - D*a12*b22 - B*d12*a22 + D*b12*a22 + B*a12*d22 + A*d12*b22;
    
    coeffs->Apart   = Detpart;
    coeffs->ApartA  = DetpartA;
    coeffs->ApartB  = DetpartB;
    coeffs->ApartAB = DetpartAB;

    coeffs->Xpart   = DetXpart;
    coeffs->XpartA  = DetXpartA;
    coeffs->XpartB  = DetXpartB;
    coeffs->XpartAB = DetXpartAB;

    coeffs->Ypart   = DetYpart;
    coeffs->YpartA  = DetYpartA;
    coeffs->YpartB  = DetYpartB;
    coeffs->YpartAB = DetYpartAB;

    coeffs->Zpart   = DetZpart;
    coeffs->ZpartA  = DetZpartA;
    coeffs->ZpartB  = DetZpartB;
    coeffs->ZpartAB = DetZpartAB;

    return CV_NO_ERR;
}
/*--------------------------------------------------------------------------------------*/


/* Find fundamental matrix */
CvStatus icvComputeFundMatrEpipoles ( CvMatr64d camMatr1, 
                                    CvMatr64d     rotMatr1, 
                                    CvVect64d     transVect1,
                                    CvMatr64d     camMatr2,
                                    CvMatr64d     rotMatr2,
                                    CvVect64d     transVect2,
                                    CvPoint2D64d* epipole1,
                                    CvPoint2D64d* epipole2,
                                    CvMatr64d     fundMatr) 
{
    double matrP1[9], matrP2[9];
    double invMatrP1[9], invMatrP2[9];
    double tmpMatr[9], tmpVect[3];
    double tmpEp1[3], tmpEp2[3], tmpF1[9];
    double vectp1[3], vectp2[3];
    //double vectTempF1[3];

    /* P1 = CM1 * R1  */
    icvMulMatrix_64d(   camMatr1,3,3,
                        rotMatr1,3,3,
                        matrP1);

    /* P2 = CM2 * R2  */
    icvMulMatrix_64d(   camMatr2,3,3,
                        rotMatr2,3,3,
                        matrP2);

    /* p1 = CM1 * T1  */
    icvMulMatrix_64d(   camMatr1,3,3,
                        transVect1,1,3,
                        vectp1);

    /* p2 = CM2 * T2  */
    icvMulMatrix_64d(   camMatr2,3,3,
                        transVect2,1,3,
                        vectp2);

    /* inv(P1), inv(P2) */
    {
        CvMat p1s = cvMat( 3, 3, CV_64FC1, matrP1 );
        CvMat p1d = cvMat( 3, 3, CV_64FC1, invMatrP1 );
        CvMat p2s = cvMat( 3, 3, CV_64FC1, matrP2 );
        CvMat p2d = cvMat( 3, 3, CV_64FC1, invMatrP2 );

        cvPseudoInv( &p1s, &p1d );
        cvPseudoInv( &p2s, &p2d );
    }

    //icvInvertMatrix_64d(matrP1,3,invMatrP1);
    //icvInvertMatrix_64d(matrP2,3,invMatrP2);

    /* P1*inv(P2) */
    icvMulMatrix_64d(matrP1,3,3,invMatrP2,3,3,tmpMatr);
    /* P1*inv(P2)*p2 */
    icvMulMatrix_64d(tmpMatr,3,3,vectp2,1,3,tmpVect);
    /* p1-P1*inv(P2)*p2 */
    icvSubVector_64d(vectp1,tmpVect,tmpEp1,3);

    /* P2*inv(P1) */
    icvMulMatrix_64d(matrP2,3,3,invMatrP1,3,3,tmpMatr);
    /* P2*inv(P1)*p1 */
    icvMulMatrix_64d(tmpMatr,3,3,vectp1,1,3,tmpVect);
    /* p2-P2*inv(P1)*p1 */
    icvSubVector_64d(vectp2,tmpVect,tmpEp2,3);

    tmpF1[0] = 0;
    tmpF1[1] = -tmpEp2[2];
    tmpF1[2] =  tmpEp2[1];

    tmpF1[3] =  tmpEp2[2];
    tmpF1[4] =  0;
    tmpF1[5] = -tmpEp2[0];

    tmpF1[6] = -tmpEp2[1];
    tmpF1[7] =  tmpEp2[0];
    tmpF1[8] =  0;

    /* Precompute fundamental matrix */
    icvMulMatrix_64d(tmpF1,3,3,tmpMatr,3,3,fundMatr);

    /* Scale fundamental matrix */
    icvScaleMatrix_64d( fundMatr, fundMatr, 3, 3, 1.0/fundMatr[8] );

    /* Set result epipoles */
    epipole1->x = tmpEp1[0] / tmpEp1[2];
    epipole1->y = tmpEp1[1] / tmpEp1[2];

    epipole2->x = tmpEp2[0] / tmpEp2[2];
    epipole2->y = tmpEp2[1] / tmpEp2[2];

    /*
    icvDeleteMatrix(matrP1);
    icvDeleteMatrix(matrP2);

    icvDeleteMatrix(invMatrP1);
    icvDeleteMatrix(invMatrP2);

    icvDeleteMatrix(tmpMatr);
    icvDeleteVector(tmpVect);
    icvDeleteMatrix(tmpF1);

    icvDeleteVector(vectp1);
    icvDeleteVector(vectp2);
    
    icvDeleteVector(vectTempF1);*/

    return CV_NO_ERR;
}

/*---------------------------------------------------------------------------------------*/

/* Finds fundamental matrix and epipoles */

/*
void cvComputeFundMatrEpipoles( CvMatr64d camMatr1, 
                                CvMatr64d rotMatr1, 
                                CvVect64d transVect1,
                                CvMatr64d camMatr2,
                                CvMatr64d rotMatr2,
                                CvVect64d transVect2,
                                CvPoint2D64d* epipole1,
                                CvPoint2D64d* epipole2,
                                CvMatr64d fundMatr)
{
    CV_FUNCNAME( "cvComputeFundMatrEpipoles" );
    __BEGIN__;

    IPPI_CALL( icvComputeFundMatrEpipoles(  camMatr1, 
                                 rotMatr1, 
                                 transVect1,
                                 camMatr2,
                                 rotMatr2,
                                 transVect2,
                                 epipole1,
                                 epipole2,
                                 fundMatr));
    __CLEANUP__;
    __END__;
}
*/
/*---------------------------------------------------------------------------------------*/

/* This function get minimum angle started at point which contains rect */
int icvGetAngleLine( CvPoint2D64d startPoint, CvSize imageSize,CvPoint2D64d *point1,CvPoint2D64d *point2)
{
    /* Get crosslines with image corners */

    /* Find four lines */

    CvPoint2D64d pa,pb,pc,pd;
    
    pa.x = 0;
    pa.y = 0;

    pb.x = imageSize.width;
    pb.y = 0;

    pd.x = imageSize.width;
    pd.y = imageSize.height;

    pc.x = 0;
    pc.y = imageSize.height;
    
    /* We can compute points for angle */
    /* Test for place section */
    
    if( startPoint.x < 0 )
    {/* 1,4,7 */
        if( startPoint.y < 0)
        {/* 1 */
            *point1 = pb;
            *point2 = pc;
        }
        else if( startPoint.y > imageSize.height )
        {/* 7 */
            *point1 = pa;
            *point2 = pd;
        }
        else
        {/* 4 */
            *point1 = pa;
            *point2 = pc;
        }
    }
    else if ( startPoint.x > imageSize.width )
    {/* 3,6,9 */
        if( startPoint.y < 0 )
        {/* 3 */
            *point1 = pa;
            *point2 = pd;
        }
        else if ( startPoint.y > imageSize.height )
        {/* 9 */
            *point1 = pb;
            *point2 = pc;
        }
        else
        {/* 6 */
            *point1 = pb;
            *point2 = pd;
        }
    }
    else
    {/* 2,5,8 */
        if( startPoint.y < 0 )
        {/* 2 */
            if( startPoint.x < imageSize.width/2 )
            {
                *point1 = pb;
                *point2 = pa;
            }
            else
            {
                *point1 = pa;
                *point2 = pb;
            }
        }
        else if( startPoint.y > imageSize.height )
        {/* 8 */
            if( startPoint.x < imageSize.width/2 )
            {
                *point1 = pc;
                *point2 = pd;
            }
            else
            {
                *point1 = pd;
                *point2 = pc;
            }
        }
        else
        {/* 5 - point in the image */
            return 2;
        }
    }
    return 0;
}/* GetAngleLine */

/*---------------------------------------------------------------------------------------*/

void icvGetCoefForPiece(   CvPoint2D64d p_start,CvPoint2D64d p_end,
                        double *a,double *b,double *c,
                        int* result)
{
    double det;
    double detA,detB,detC;

    det = p_start.x*p_end.y+p_end.x+p_start.y-p_end.y-p_start.y*p_end.x-p_start.x;
    if( fabs(det) < EPS64D)/* Error */
    {
        *result = 0;
        return;
    }

    detA = p_start.y - p_end.y;
    detB = p_end.x - p_start.x;
    detC = p_start.x*p_end.y - p_end.x*p_start.y;

    double invDet = 1.0 / det;
    *a = detA * invDet;
    *b = detB * invDet;
    *c = detC * invDet;

    *result = 1;
    return;
}

/*---------------------------------------------------------------------------------------*/

/* Get common area of rectifying */
void icvGetCommonArea( CvSize imageSize,
                    CvPoint2D64d epipole1,CvPoint2D64d epipole2,
                    CvMatr64d fundMatr,
                    CvVect64d coeff11,CvVect64d coeff12,
                    CvVect64d coeff21,CvVect64d coeff22,
                    int* result)
{
    CvPoint2D64d point11;
    CvPoint2D64d point12;
    CvPoint2D64d point21;
    CvPoint2D64d point22;

    double corr11[3];
    double corr12[3];
    double corr21[3];
    double corr22[3];

    double pointW11[3];
    double pointW12[3];
    double pointW21[3];
    double pointW22[3];


    double transFundMatr[3*3];
    /* Compute transpose of fundamental matrix */
    icvTransposeMatrix_64d( fundMatr, 3, 3, transFundMatr );
    
    int stat = icvGetAngleLine( epipole1, imageSize,&point11,&point12);
    if( stat == 2 )
    {
        /* No angle */
        *result = 0;
        return;
    }

    stat = icvGetAngleLine( epipole2, imageSize,&point21,&point22);
    if( stat == 2 )
    {
        /* No angle */
        *result = 0;
        return;
    }

    /* ============= Computation for line 1 ================ */
    /* Find correspondence line for angle points11 */
    /* corr21 = Fund*p1 */

    pointW11[0] = point11.x;
    pointW11[1] = point11.y;
    pointW11[2] = 1.0;

    icvTransformVector_64d( fundMatr,
                            pointW11, 
                            corr21,
                            3,3);

    /* Find crossing of line with image 2 */
    CvPoint2D64d start;
    CvPoint2D64d end;
    int res;
    icvGetCrossRectDirect( imageSize,
                        corr21[0],corr21[1],corr21[2],
                        &start,&end,
                        &res);
    
    if( res == 0 )
    {/* We have not cross */
        /* We must define new angle */

        pointW21[0] = point21.x;
        pointW21[1] = point21.y;
        pointW21[2] = 1.0;

        /* Find correspondence line for this angle points */
        /* We know point and try to get corr line */
        /* For point21 */
        /* corr2 = Fund' * p1 */

        icvTransformVector_64d( transFundMatr,
                                pointW21, 
                                corr11,
                                3,3);

        /* We have cross. And it's result cross for up line. Set result coefs */

        /* Set coefs for line 1 image 1 */
        coeff11[0] = corr11[0];
        coeff11[1] = corr11[1];
        coeff11[2] = corr11[2];
        
        /* Set coefs for line 1 image 2 */
        int res;

        icvGetCoefForPiece(    epipole2,point21,
                            &coeff21[0],&coeff21[1],&coeff21[2],
                            &res);
        if( res == 0 )
        {
            *result = 0;
            return;/* Error */
        }
    }
    else
    {/* Line 1 cross image 2 */
        /* Set coefs for line 1 image 1 */
        int res;

        icvGetCoefForPiece(    epipole1,point11,
                            &coeff11[0],&coeff11[1],&coeff11[2],
                            &res);
        if( res == 0 )
        {
            *result = 0;
            return;/* Error */
        }
        
        /* Set coefs for line 1 image 2 */
        coeff21[0] = corr21[0];
        coeff21[1] = corr21[1];
        coeff21[2] = corr21[2];
        
    }

    /* ============= Computation for line 2 ================ */
    /* Find correspondence line for angle points11 */
    /* corr22 = Fund*p2 */

    pointW12[0] = point12.x;
    pointW12[1] = point12.y;
    pointW12[2] = 1.0;

    icvTransformVector_64d( fundMatr,
                            pointW12, 
                            corr22,
                            3,3);

    /* Find crossing of line with image 2 */
    //CvPoint2D64d start;
    //CvPoint2D64d end;
    //int res;
    icvGetCrossRectDirect( imageSize,
                        corr22[0],corr22[1],corr22[2],
                        &start,&end,
                        &res);
    
    if( res == 0 )
    {/* We have not cross */
        /* We must define new angle */

        pointW22[0] = point22.x;
        pointW22[1] = point22.y;
        pointW22[2] = 1.0;

        /* Find correspondence line for this angle points */
        /* We know point and try to get corr line */
        /* For point21 */
        /* corr2 = Fund' * p1 */

        icvTransformVector_64d( transFundMatr,
                                pointW22, 
                                corr12,
                                3,3);

        
        /* We have cross. And it's result cross for down line. Set result coefs */

        /* Set coefs for line 2 image 1 */
        coeff12[0] = corr12[0];
        coeff12[1] = corr12[1];
        coeff12[2] = corr12[2];
        
        /* Set coefs for line 1 image 2 */
        int res;

        icvGetCoefForPiece(    epipole2,point22,
                            &coeff22[0],&coeff22[1],&coeff22[2],
                            &res);
        if( res == 0 )
        {
            *result = 0;
            return;/* Error */
        }
    }
    else
    {/* Line 2 cross image 2 */
        /* Set coefs for line 2 image 1 */
        int res;

        icvGetCoefForPiece(    epipole1,point12,
                            &coeff12[0],&coeff12[1],&coeff12[2],
                            &res);
        if( res == 0 )
        {
            *result = 0;
            return;/* Error */
        }
        
        /* Set coefs for line 1 image 2 */
        coeff22[0] = corr22[0];
        coeff22[1] = corr22[1];
        coeff22[2] = corr22[2];
        
    }

    /* Now we know common area */

    return;

}/* GetCommonArea */

/* Function finds correspondence points. Algorithm based on LK */
/* and filter them according fundamental matrix information */
void icvFindCorrPointsFundamentLK(IplImage* image1,IplImage* image2,
                               CvMatr64d /*fundMatr*/,
                               CvPoint2D64d* points1,
                               CvPoint2D64d* points2,
                               int* count
                               )
{
    /* Test */
    if( image1 ==0 || image2 == 0 )
    {
        return;//Error bad pointer
    }

    /* Test image sizes  */
    if( image1->width != image2->width || image1->height != image2->height )
    {
        return;// Error bad sizes
    }

    CvSize size;
    size.width  = image1->width;
    size.height = image1->height;
    
    int i, k;
    //bool  result  = false;
    int   m_count = 100;
    char* status  = (char*)calloc(sizeof(char),m_count);

    IplImage* m_left_pyr;
    IplImage* m_right_pyr;

    IplImage* gray1;
    IplImage* gray2;

    IplImage* pyr1;
    IplImage* pyr2;

    CvRect rect;
    rect = cvRect(0,0,size.width,size.height);

    gray1 = cvCreateImage( size, 8, 1 );
    cvSetImageROI( gray1, rect );

    gray2 = cvCreateImage( size, 8, 1 );
    cvSetImageROI( gray2, rect );

    pyr1 = cvCreateImage( size, 8, 1 );
    cvSetImageROI( pyr1, rect );

    pyr2 = cvCreateImage( size, 8, 1 );
    cvSetImageROI( pyr2, rect );

    IplImage* m_left_gray_img  = 0;
    IplImage* m_right_gray_img = 0;

    m_left_pyr  = 0;
    m_right_pyr = 0;

    int m_realcount = 0;
    //CvSize m_win_size = cvSize(20,20);

    int m_pyr_level = 3;
    //CvTermCriteria m_criteria = cvTermCriteria(CV_TERMCRIT_ITER|CV_TERMCRIT_EPS,20,0.1);
    
    
    m_left_gray_img = icvCreateIsometricImage( image1, m_left_gray_img,
                                            image1->depth, 1 );
    m_right_gray_img = icvCreateIsometricImage( image2, m_right_gray_img,
                                             image2->depth, 1 );

    m_left_pyr = icvCreateIsometricImage( image1, m_left_pyr,
                                       IPL_DEPTH_32F, 1 );
    m_right_pyr = icvCreateIsometricImage( image2, m_right_pyr,
                                        IPL_DEPTH_32F, 1 );

    cvCvtColor( image1, m_left_gray_img, CV_BGR2GRAY );
    cvCvtColor( image2, m_right_gray_img, CV_BGR2GRAY );

    m_left_gray_img->roi->width &= -(1 << m_pyr_level);
    m_left_gray_img->roi->height &= -(1 << m_pyr_level);
    m_right_gray_img->roi = m_left_gray_img->roi;
/***********************************/
#if 0
    {/* ----- Create points ----- */
    
        m_realcount = m_count;

        cvGoodFeaturesToTrack( m_left_gray_img, m_left_pyr, m_right_pyr, points1,
                               &m_realcount, 0.01, 5 );

        cvFindCornerSubPix( m_left_gray_img, points1, m_realcount,
                            m_win_size, cvSize(-1,-1), m_criteria );

        /* ----- end create points ----- */
    }


    cvCalcOpticalFlowPyrLK( m_left_gray_img, m_right_gray_img,
                            m_left_pyr, m_right_pyr, points1, points2,
                            m_realcount, m_win_size, m_pyr_level, status, 0,
                            m_criteria,0);
#endif
/*****************************/    
    m_right_gray_img->roi = m_left_gray_img->roi = image1->roi;

    for( i = 0, k = 0; i < m_realcount; i++ )
    {
        if( status[i] &&
            points1[i].x > 1 && points1[i].x < size.width - 2 &&
            points1[i].y > 1 && points1[i].y < size.height - 2  &&
            points2[i].x > 1 && points2[i].x < size.width - 2 &&
            points2[i].y > 1 && points2[i].y < size.height - 2 )
        {
            if( k < i )
            {
                points1[k] = points1[i];
                points2[k] = points2[i];
            }
            k++;
        }
    }

    free(status);

    int numPoints;
    numPoints = m_realcount;
    numPoints = k;
    
    *count = numPoints;/* Return number of points */
    return;
}/* FindCorrPointsFundamentLK */


/*---------------------------------------------------------------------------------------*/

/* Get cross for direction1 and direction2 */
/*  Result = 1 - cross */
/*  Result = 2 - parallel and not equal */
/*  Result = 3 - parallel and equal */

void icvGetCrossDirectDirect(  CvVect64d direct1,CvVect64d direct2,
                            CvPoint2D64d *cross,int* result)
{
    double det  = direct1[0]*direct2[1] - direct2[0]*direct1[1];
    double detx = -direct1[2]*direct2[1] + direct1[1]*direct2[2];

    if( fabs(det) > EPS64D )
    {/* Have cross */
        cross->x = detx/det;
        cross->y = (-direct1[0]*direct2[2] + direct2[0]*direct1[2])/det;
        *result = 1;
    }
    else
    {/* may be parallel */
        if( fabs(detx) > EPS64D )
        {/* parallel and not equal */
            *result = 2;
        }
        else
        {/* equals */
            *result = 3;
        }
    }

    return;
}

/*---------------------------------------------------------------------------------------*/

/* Get cross for piece p1,p2 and direction a,b,c */
/*  Result = 0 - no cross */
/*  Result = 1 - cross */
/*  Result = 2 - parallel and not equal */
/*  Result = 3 - parallel and equal */

void icvGetCrossPieceDirect(   CvPoint2D64d p_start,CvPoint2D64d p_end,
                            double a,double b,double c,
                            CvPoint2D64d *cross,int* result)
{

    if( (a*p_start.x + b*p_start.y + c) * (a*p_end.x + b*p_end.y + c) <= 0 )
    {/* Have cross */
        double det;
        double detxc,detyc;
        
        det = a * (p_end.x - p_start.x) + b * (p_end.y - p_start.y);
        
        if( fabs(det) < EPS64D )
        {/* lines are parallel and may be equal or line is point */
            if(  fabs(a*p_start.x + b*p_start.y + c) < EPS64D )
            {/* line is point or not diff */
                *result = 3;
                return;
            }
            else
            {
                *result = 2;                
            }
            return;
        }

        detxc = b*(p_end.y*p_start.x - p_start.y*p_end.x) + c*(p_start.x - p_end.x);
        detyc = a*(p_end.x*p_start.y - p_start.x*p_end.y) + c*(p_start.y - p_end.y);

        cross->x = detxc / det;
        cross->y = detyc / det;
        *result = 1;

    }
    else
    {
        *result = 0;
    }
    return;
}
/*--------------------------------------------------------------------------------------*/

void icvGetCrossPiecePiece( CvPoint2D64d p1_start,CvPoint2D64d p1_end,
                            CvPoint2D64d p2_start,CvPoint2D64d p2_end,
                            CvPoint2D64d* cross,
                            int* result)
{
    double ex1,ey1,ex2,ey2;
    double px1,py1,px2,py2;
    double del;
    double delA,delB,delX,delY;
    double alpha,betta;//,X,Y;

    ex1 = p1_start.x;
    ey1 = p1_start.y;
    ex2 = p1_end.x;
    ey2 = p1_end.y;

    px1 = p2_start.x;
    py1 = p2_start.y;
    px2 = p2_end.x;
    py2 = p2_end.y;

    del = (py1-py2)*(ex1-ex2)-(px1-px2)*(ey1-ey2);
    if( fabs(del) <= EPS64D )
    {/* May be it's parallel !!! */
        *result = 0;
        return;
    }

    delA =  (ey1-ey2)*(ex1-px1) + (ex1-ex2)*(py1-ey1);
    delB =  (py1-py2)*(ex1-px1) + (px1-px2)*(py1-ey1);

    alpha = delA / del;
    betta = delB / del;

    if( alpha < 0 || alpha > 1.0 || betta < 0 || betta > 1.0)
    {
        *result = 0;
        return;
    }

    delX =  (px1-px2)*(ey1*(ex1-ex2)-ex1*(ey1-ey2))+
            (ex1-ex2)*(px1*(py1-py2)-py1*(px1-px2));

    delY =  (py1-py2)*(ey1*(ex1-ex2)-ex1*(ey1-ey2))+
            (ey1-ey2)*(px1*(py1-py2)-py1*(px1-px2));

    cross->x = delX / del;
    cross->y = delY / del;
    
    *result = 1;
    return;
}


/*---------------------------------------------------------------------------------------*/

void icvGetPieceLength(CvPoint2D64d point1,CvPoint2D64d point2,double* dist)
{
    double dx = point2.x - point1.x;
    double dy = point2.y - point1.y;
    *dist = sqrt( dx*dx + dy*dy );
    return;
}

/*---------------------------------------------------------------------------------------*/

void icvGetPieceLength3D(CvPoint3D64d point1,CvPoint3D64d point2,double* dist)
{
    double dx = point2.x - point1.x;
    double dy = point2.y - point1.y;
    double dz = point2.z - point1.z;
    *dist = sqrt( dx*dx + dy*dy + dz*dz );
    return;
}

/*---------------------------------------------------------------------------------------*/

/* Find line from epipole which cross image rect */
/* Find points of cross 0 or 1 or 2. Return number of points in cross */
void icvGetCrossRectDirect(    CvSize imageSize,
                            double a,double b,double c,
                            CvPoint2D64d *start,CvPoint2D64d *end,
                            int* result)
{
    CvPoint2D64d frameBeg;
    CvPoint2D64d frameEnd;
    CvPoint2D64d cross[4];
    int     haveCross[4];
    
    haveCross[0] = 0;
    haveCross[1] = 0;
    haveCross[2] = 0;
    haveCross[3] = 0;

    frameBeg.x = 0;
    frameBeg.y = 0;
    frameEnd.x = imageSize.width;
    frameEnd.y = 0;

    icvGetCrossPieceDirect(frameBeg,frameEnd,a,b,c,&cross[0],&haveCross[0]);    
    
    frameBeg.x = imageSize.width;
    frameBeg.y = 0;
    frameEnd.x = imageSize.width;
    frameEnd.y = imageSize.height;
    icvGetCrossPieceDirect(frameBeg,frameEnd,a,b,c,&cross[1],&haveCross[1]);    

    frameBeg.x = imageSize.width;
    frameBeg.y = imageSize.height;
    frameEnd.x = 0;
    frameEnd.y = imageSize.height;
    icvGetCrossPieceDirect(frameBeg,frameEnd,a,b,c,&cross[2],&haveCross[2]);    

    frameBeg.x = 0;
    frameBeg.y = imageSize.height;
    frameEnd.x = 0;
    frameEnd.y = 0;
    icvGetCrossPieceDirect(frameBeg,frameEnd,a,b,c,&cross[3],&haveCross[3]);    

    double maxDist;

    int maxI=0,maxJ=0;


    int i,j;

    maxDist = -1.0;
    
    double distance;

    for( i = 0; i < 3; i++ )
    {
        if( haveCross[i] == 1 )
        {
            for( j = i + 1; j < 4; j++ )
            {
                if( haveCross[j] == 1)
                {/* Compute dist */
                    icvGetPieceLength(cross[i],cross[j],&distance);
                    if( distance > maxDist )
                    {
                        maxI = i;
                        maxJ = j;
                        maxDist = distance;
                    }
                }
            }
        }
    }

    if( maxDist >= 0 )
    {/* We have cross */
        *start = cross[maxI];
        *result = 1;
        if( maxDist > 0 )
        {
            *end   = cross[maxJ];
            *result = 2;
        }
    }
    else
    {
        *result = 0;
    }

    return;
}/* GetCrossRectDirect */

/*---------------------------------------------------------------------------------------*/
void icvProjectPointToImage(   CvPoint3D64d point,
                            CvMatr64d camMatr,CvMatr64d rotMatr,CvVect64d transVect,
                            CvPoint2D64d* projPoint)
{

    double tmpVect1[3];
    double tmpVect2[3];
    
    //double tmpMatr[9];

    icvMulMatrix_64d (  rotMatr,
                        3,3,
                        (double*)&point,
                        1,3,
                        tmpVect1);

    icvAddVector_64d ( tmpVect1, transVect,tmpVect2, 3);

    icvMulMatrix_64d (  camMatr,
                        3,3,
                        tmpVect2,
                        1,3,
                        tmpVect1);

    projPoint->x = tmpVect1[0] / tmpVect1[2];
    projPoint->y = tmpVect1[1] / tmpVect1[2];
   
    return;
}


/*---------------------------------------------------------------------------------------*/
/* Get quads for transform images */
#if 1
void icvGetQuadsTransform( CvSize        imageSize,
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
                        )
{
    /* First compute fundamental matrix and epipoles */

    //CvPoint2D64d epipole1;
    //CvPoint2D64d epipole2;
    //double fundMatr[9];

    int res;

#ifdef _DEBUG
    CvStatus status =
#endif
    icvComputeFundMatrEpipoles( camMatr1,
                        rotMatr1,
                        transVect1,
                        camMatr2,
                        rotMatr2,
                        transVect2,
                        epipole1,
                        epipole2,
                        fundMatr);
    assert( status >= 0 );

    double coeff11[3];
    double coeff12[3];
    double coeff21[3];
    double coeff22[3];

    icvGetCommonArea(   imageSize,
                        *epipole1,*epipole2,
                        fundMatr,
                        coeff11,coeff12,
                        coeff21,coeff22,
                        &res);

    CvPoint2D64d point11, point12,point21, point22;
    double width1,width2;
    double height1,height2;
    double tmpHeight1,tmpHeight2;

    /* ----- Image 1 ----- */
    icvGetCutPiece( coeff11,coeff12,
                *epipole1,
                imageSize,
                &point11,&point12,
                &point21,&point22,
                &res);

    /* Compute distance */
    icvGetPieceLength(point11,point21,&width1);
    icvGetPieceLength(point11,point12,&tmpHeight1);
    icvGetPieceLength(point21,point22,&tmpHeight2);
    height1 = MAX(tmpHeight1,tmpHeight2);

    quad1[0][0] = point11.x;
    quad1[0][1] = point11.y;

    quad1[1][0] = point21.x;
    quad1[1][1] = point21.y;

    quad1[2][0] = point22.x;
    quad1[2][1] = point22.y;

    quad1[3][0] = point12.x;
    quad1[3][1] = point12.y;

    /* ----- Image 2 ----- */
    icvGetCutPiece( coeff21,coeff22,
                *epipole2,
                imageSize,
                &point11,&point12,
                &point21,&point22,
                &res);

    /* Compute distance */
    icvGetPieceLength(point11,point21,&width2);
    icvGetPieceLength(point11,point12,&tmpHeight1);
    icvGetPieceLength(point21,point22,&tmpHeight2);
    height2 = MAX(tmpHeight1,tmpHeight2);

    quad2[0][0] = point11.x;
    quad2[0][1] = point11.y;

    quad2[1][0] = point21.x;
    quad2[1][1] = point21.y;

    quad2[2][0] = point22.x;
    quad2[2][1] = point22.y;

    quad2[3][0] = point12.x;
    quad2[3][1] = point12.y;

    double warpWidth,warpHeight;

    warpWidth  = MAX(width1,width2);
    warpHeight = MAX(height1,height2);

    warpSize->width  = (int)warpWidth;
    warpSize->height = (int)warpHeight;

    return;
}
#endif
/*---------------------------------------------------------------------------------------*/

/* Get cut line for one image */
void icvGetCutPiece(   CvVect64d areaLineCoef1,CvVect64d areaLineCoef2,
                    CvPoint2D64d epipole,
                    CvSize imageSize,
                    CvPoint2D64d* point11,CvPoint2D64d* point12,
                    CvPoint2D64d* point21,CvPoint2D64d* point22,
                    int* result)
{
    /* Compute nearest cut line to epipole */
    /* Get corners inside sector */
    /* Collect all candidate point */

    CvPoint2D64d candPoints[8];
    CvPoint2D64d midPoint;
    int numPoints = 0;
    int res;
    int i;

    double cutLine1[3];
    double cutLine2[3];


    

    /* Find middle line of sector */
    double midLine[3];
#if 0
    double norm1 = 1.0/sqrt(areaLineCoef1[0]*areaLineCoef1[0]+areaLineCoef1[1]*areaLineCoef1[1]+areaLineCoef1[2]*areaLineCoef1[2]);
    double norm2 = 1.0/sqrt(areaLineCoef2[0]*areaLineCoef2[0]+areaLineCoef2[1]*areaLineCoef2[1]+areaLineCoef2[2]*areaLineCoef2[2]);
    
    midLine[0] =   (areaLineCoef1[0]*norm1 + areaLineCoef2[0]*norm2)*0.5;
    midLine[1] =   (areaLineCoef1[1]*norm1 + areaLineCoef2[1]*norm2)*0.5;
    midLine[2] = - (midLine[0]*epipole.x + midLine[1]*epipole.y);
#endif


    /* Different way  */
    CvPoint2D64d pointOnLine1;  pointOnLine1.x = pointOnLine1.y = 0;
    CvPoint2D64d pointOnLine2;  pointOnLine2.x = pointOnLine2.y = 0;

    CvPoint2D64d start1,end1;

    icvGetCrossRectDirect( imageSize,
                        areaLineCoef1[0],areaLineCoef1[1],areaLineCoef1[2],
                        &start1,&end1,&res);
    if( res > 0 )
    {
        pointOnLine1 = start1;
    }

    icvGetCrossRectDirect( imageSize,
                        areaLineCoef2[0],areaLineCoef2[1],areaLineCoef2[2],
                        &start1,&end1,&res);
    if( res > 0 )
    {
        pointOnLine2 = start1;
    }

    icvGetMiddleAnglePoint(epipole,pointOnLine1,pointOnLine2,&midPoint);

    icvGetCoefForPiece(epipole,midPoint,&midLine[0],&midLine[1],&midLine[2],&res);

    /* Test corner points */
    CvPoint2D64d cornerPoint;
    CvPoint2D64d tmpPoints[2];

    cornerPoint.x = 0;
    cornerPoint.y = 0;
    icvTestPoint( cornerPoint, areaLineCoef1, areaLineCoef2, epipole, &res);
    if( res == 1 )
    {/* Add point */
        candPoints[numPoints] = cornerPoint;
        numPoints++;
    }

    cornerPoint.x = imageSize.width;
    cornerPoint.y = 0;
    icvTestPoint( cornerPoint, areaLineCoef1, areaLineCoef2, epipole, &res);
    if( res == 1 )
    {/* Add point */
        candPoints[numPoints] = cornerPoint;
        numPoints++;
    }
    
    cornerPoint.x = imageSize.width;
    cornerPoint.y = imageSize.height;
    icvTestPoint( cornerPoint, areaLineCoef1, areaLineCoef2, epipole, &res);
    if( res == 1 )
    {/* Add point */
        candPoints[numPoints] = cornerPoint;
        numPoints++;
    }

    cornerPoint.x = 0;
    cornerPoint.y = imageSize.height;
    icvTestPoint( cornerPoint, areaLineCoef1, areaLineCoef2, epipole, &res);
    if( res == 1 )
    {/* Add point */
        candPoints[numPoints] = cornerPoint;
        numPoints++;
    }

    /* Find cross line 1 with image border */
    icvGetCrossRectDirect( imageSize,
                        areaLineCoef1[0],areaLineCoef1[1],areaLineCoef1[2],
                        &tmpPoints[0], &tmpPoints[1],
                        &res);
    for( i = 0; i < res; i++ )
    {
        candPoints[numPoints++] = tmpPoints[i];
    }

    /* Find cross line 2 with image border */
    icvGetCrossRectDirect( imageSize,
                        areaLineCoef2[0],areaLineCoef2[1],areaLineCoef2[2],
                        &tmpPoints[0], &tmpPoints[1],
                        &res);
    
    for( i = 0; i < res; i++ )
    {
        candPoints[numPoints++] = tmpPoints[i];
    }

    if( numPoints < 2 )
    {
        *result = 0;
        return;/* Error. Not enought points */
    }
    /* Project all points to middle line and get max and min */

    CvPoint2D64d projPoint;
    CvPoint2D64d minPoint; minPoint.x = minPoint.y = FLT_MAX;
    CvPoint2D64d maxPoint; maxPoint.x = maxPoint.y = -FLT_MAX;


    double dist;
    double maxDist = 0;
    double minDist = 10000000;

    
    for( i = 0; i < numPoints; i++ )
    {
        icvProjectPointToDirect(candPoints[i], midLine, &projPoint);
        icvGetPieceLength(epipole,projPoint,&dist);
        if( dist < minDist)
        {
            minDist = dist;
            minPoint = projPoint;
        }

        if( dist > maxDist)
        {
            maxDist = dist;
            maxPoint = projPoint;
        }
    }

    /* We know maximum and minimum points. Now we can compute cut lines */
    
    icvGetNormalDirect(midLine,minPoint,cutLine1);
    icvGetNormalDirect(midLine,maxPoint,cutLine2);

    /* Test for begin of line. */
    CvPoint2D64d tmpPoint2;

    /* Get cross with */
    icvGetCrossDirectDirect(areaLineCoef1,cutLine1,point11,&res);
    icvGetCrossDirectDirect(areaLineCoef2,cutLine1,point12,&res);

    icvGetCrossDirectDirect(areaLineCoef1,cutLine2,point21,&res);
    icvGetCrossDirectDirect(areaLineCoef2,cutLine2,point22,&res);

    if( epipole.x > imageSize.width * 0.5 )
    {/* Need to change points */
        tmpPoint2 = *point11;
        *point11 = *point21;
        *point21 = tmpPoint2;

        tmpPoint2 = *point12;
        *point12 = *point22;
        *point22 = tmpPoint2;
    }

    return;
}
/*---------------------------------------------------------------------------------------*/
/* Get middle angle */
void icvGetMiddleAnglePoint(   CvPoint2D64d basePoint,
                            CvPoint2D64d point1,CvPoint2D64d point2,
                            CvPoint2D64d* midPoint)
{/* !!! May be need to return error */
    
    double dist1;
    double dist2;
    icvGetPieceLength(basePoint,point1,&dist1);
    icvGetPieceLength(basePoint,point2,&dist2);
    CvPoint2D64d pointNew1;
    CvPoint2D64d pointNew2;
    double alpha = dist2/dist1;

    pointNew1.x = basePoint.x + (1.0/alpha) * ( point2.x - basePoint.x );
    pointNew1.y = basePoint.y + (1.0/alpha) * ( point2.y - basePoint.y );

    pointNew2.x = basePoint.x + alpha * ( point1.x - basePoint.x );
    pointNew2.y = basePoint.y + alpha * ( point1.y - basePoint.y );

    int res;
    icvGetCrossPiecePiece(point1,point2,pointNew1,pointNew2,midPoint,&res);

    return;
}



/*---------------------------------------------------------------------------------------*/
/* Get normal direct to direct in line */
void icvGetNormalDirect(CvVect64d direct,CvPoint2D64d point,CvVect64d normDirect)
{
    normDirect[0] =   direct[1];
    normDirect[1] = - direct[0];
    normDirect[2] = -(normDirect[0]*point.x + normDirect[1]*point.y);  
    return;
}

/*---------------------------------------------------------------------------------------*/
CV_IMPL double icvGetVect(CvPoint2D64d basePoint,CvPoint2D64d point1,CvPoint2D64d point2)
{
    return  (point1.x - basePoint.x)*(point2.y - basePoint.y) -
            (point2.x - basePoint.x)*(point1.y - basePoint.y);
}
/*---------------------------------------------------------------------------------------*/
/* Test for point in sector           */
/* Return 0 - point not inside sector */
/* Return 1 - point inside sector     */
void icvTestPoint( CvPoint2D64d testPoint,
                CvVect64d line1,CvVect64d line2,
                CvPoint2D64d basePoint,
                int* result)
{
    CvPoint2D64d point1,point2;

    icvProjectPointToDirect(testPoint,line1,&point1);
    icvProjectPointToDirect(testPoint,line2,&point2);

    double sign1 = icvGetVect(basePoint,point1,point2);
    double sign2 = icvGetVect(basePoint,point1,testPoint);
    if( sign1 * sign2 > 0 )
    {/* Correct for first line */
        sign1 = - sign1;
        sign2 = icvGetVect(basePoint,point2,testPoint);
        if( sign1 * sign2 > 0 )
        {/* Correct for both lines */
            *result = 1;
        }
        else
        {
            *result = 0;
        }
    }
    else
    {
        *result = 0;
    }
    
    return;
}

/*---------------------------------------------------------------------------------------*/
/* Project point to line */
void icvProjectPointToDirect(  CvPoint2D64d point,CvVect64d lineCoeff,
                            CvPoint2D64d* projectPoint)
{
    double a = lineCoeff[0];
    double b = lineCoeff[1];
    
    double det =  1.0 / ( a*a + b*b );
    double delta =  a*point.y - b*point.x;

    projectPoint->x = ( -a*lineCoeff[2] - b * delta ) * det;
    projectPoint->y = ( -b*lineCoeff[2] + a * delta ) * det ;

    return;
}

/*---------------------------------------------------------------------------------------*/
/* Get distance from point to direction */
void icvGetDistanceFromPointToDirect( CvPoint2D64d point,CvVect64d lineCoef,double*dist)
{
    CvPoint2D64d tmpPoint;
    icvProjectPointToDirect(point,lineCoef,&tmpPoint);
    double dx = point.x - tmpPoint.x;
    double dy = point.y - tmpPoint.y;
    *dist = sqrt(dx*dx+dy*dy);
    return;
}
/*---------------------------------------------------------------------------------------*/

CV_IMPL IplImage* icvCreateIsometricImage( IplImage* src, IplImage* dst,
                                       int desired_depth, int desired_num_channels )
{
    CvSize src_size ;
    src_size.width = src->width;
    src_size.height = src->height;
    
    CvSize dst_size = src_size;

    if( dst )
    {
        dst_size.width = dst->width;
        dst_size.height = dst->height;
    }

    if( !dst || dst->depth != desired_depth ||
        dst->nChannels != desired_num_channels ||
        dst_size.width != src_size.width ||
        dst_size.height != dst_size.height )
    {
        cvReleaseImage( &dst );
        dst = cvCreateImage( src_size, desired_depth, desired_num_channels );
        CvRect rect = cvRect(0,0,src_size.width,src_size.height);
        cvSetImageROI( dst, rect );

    }

    return dst;
};

CvStatus
icvCvt_32f_64d( float *src, double *dst, int size )
{
    int t;

    if( !src || !dst )
        return CV_NULLPTR_ERR;
    if( size <= 0 )
        return CV_BADRANGE_ERR;

    for( t = 0; t < size; t++ )
    {
        dst[t] = (double) (src[t]);
    }

    return CV_OK;
}

/*======================================================================================*/
/* Type conversion double -> float */
CvStatus
icvCvt_64d_32f( double *src, float *dst, int size )
{
    int t;

    if( !src || !dst )
        return CV_NULLPTR_ERR;
    if( size <= 0 )
        return CV_BADRANGE_ERR;

    for( t = 0; t < size; t++ )
    {
        dst[t] = (float) (src[t]);
    }

    return CV_OK;
}


/* --------------------------- Prototipes ------------------------------ */
/*=========================================================================
===========================================================================
===========================================================================
===========================================================================
=========================================================================*/
/* ---------------------   Geometric functions   ----------------------- */
/* ---------------------  Some useful  funcions  ----------------------- */




/*---------------------------------------------------------------------------------------*/
#if 0
int FindCorrespondLine( CvMatr32f fundMatr,/* Fundamental matrix */
                        CvSize imageSize,/* Image size */
                        CvPoint2D32f epipole1,/* Epipole point on first image */
                        CvPoint2D32f epipole2,/* Epipole point on second image */
                        CvPoint2D32f &anglePoint1,/* image point on first image */
                        CvPoint2D32f &anglePoint2)/* image point on second image */
{/* For point from first image finds line (point) on second image */

    /* Apply matrix to first point */

    CvPoint3D32f pointM;
    CvPoint3D32f tmp3;
    CvPoint2D32f testAnglePoint1;
    CvPoint2D32f testAnglePoint2;

    pointM.x = anglePoint1->x;
    pointM.y = anglePoint1->y;
    pointM.z = 1;
    

    _cvMulMatrix_32f (  fundMatr,
                        3,3,
                        pointM,
                        3,1,
                        tmp3);

    /* Normalize point */    
    testAnglePoint2.x = tmp3.x / tmp3.z;
    testAnglePoint2.y = tmp3.y / tmp3.z;

    /* Now we can compute crossing with image rectangle */
    int cross;

    CvPoint2D32f start;
    CvPoint2D32f end;
    
    cross = FindLine(epipole2,imageSize,testAnglePoint2,&start,&end);
    if( cross > 0 )
    {/* have cross. It's OK */
        *anglePoint2 = testAnglePoint2;
    }
    else
    {/* Not cross use point from second image as base */
        /* Transpose Fundamental matrix */
        
        /* take the up point from second image */

        //CvPoint2D32f basePoint1;/* base point on first image */
        float transFund[9];
        
        _cvTransposeMatrix_32f (fundMatr,
                                3,3,
                                transFund);

        /* Compute corr point */
        
        pointM.x = anglePoint2->x;
        pointM.y = anglePoint2->y;
        pointM.z = 1;
        
        _cvMulMatrix_32f (  transFund,
                            3,3,
                            pointM,
                            3,1,
                            tmp3);

        /* Normalize point */    
        testAnglePoint1.x = tmp3.x / tmp3.z;
        testAnglePoint1.y = tmp3.y / tmp3.z;
        /* We compute new angle point on first image */
        /* Test it for cross */
        cross = FindLine(epipole1,imageSize,testAnglePoint1,&start,&end);

        if( cross == 0 )
        {
            /* Bad case. No crossing */
            return -1;
        }
        else
        {
            *anglePoint1 = testAnglePoint1;            
        }
    }
    return 0;    
}

/*--------------------------------------------------------------------------------------*/

/* Rectifing the image */
void cvRectifyImage(IplImage* srcimage,CvPoint2D32f epipole,IplImage* dstImage)
{

    return;
}
#endif

/*----------------------------------------------------------------------------------*/
    
#if 0
void CCalib3DWindow::SetPoint(float x, float y)
{
    /* Calculate position */
    CvMat matr    = cvMat( 3, 3, CV_MAT32F, m_camera.matrix );
    CvMat invMatr = cvMat( 3, 3, CV_MAT32F, 0 );
    CvMat tmpVect = cvMat( 3, 1, CV_MAT32F, 0 );

    cvmAlloc( &invMatr );
    cvmAlloc( &tmpVect );
    
    cvmInvert(&matr,&invMatr);
    
    float vectd[3];
    vectd[0] = x;
    vectd[1] = y;
    vectd[2] = 1;

    CvMat vect = cvMat(3,1,CV_MAT32F,vectd);

    cvmMul(&invMatr,&vect,&tmpVect);

    lookX = tmpVect.data.fl[0];
    lookY = tmpVect.data.fl[1];
    lookZ = tmpVect.data.fl[2];

    cvmFree( &invMatr );
    cvmFree( &tmpVect );
    
//    _cvInvertMatrix_32f(m_camera.matrix,3,invMatr);
//    _cvMulMatrix_32f(invMatr,3,3,vect,1,3,resVect);

}

/*----------------------------------------------------------------------------------*/


/* Find distance from point to line */
CvStatus DistanceToPiece2D( CvPoint2D32f beg,
                            CvPoint2D32f end,
                            CvPoint2D32f point,
                            float *distance)
{
    float d1;
    float d2;
    float dist;
    float delta,delta1,delta2;
    float t1,t2;
    float dist12;
    float crossX,crossY;
    float tmpDist;

    /* Compute distance to ends of piece */
    d1 = sqrt( (beg.x - point.x) * (beg.x - point.x) + (beg.y - point.y) * (beg.y - point.y));
    d2 = sqrt( (end.x - point.x) * (end.x - point.x) + (end.y - point.y) * (end.y - point.y));

    dist = ( d1 < d2 ? d1 : d2 );

    /* compute distance to line (and inside ends) */

    dist12 = (beg.x - end.x) * (beg.x - end.x) + (beg.y - end.y) * (beg.y - end.y);

    if( dist12 < 0.00001 )
    {
        *distance = dist;
        return CV_OK;
    }
    
    delta  = - dist12;
    delta1 = (point.x - beg.x)*(beg.x-end.x) - (end.y-beg.y)*(point.y-beg.y);
    delta2 = (end.x - beg.x)*(point.y-beg.y) - (point.x-beg.x)*(end.y-beg.y);

    t1 = delta1 / delta;
    t2 = delta2 / delta;

    if( t1 > 0 && t1 < 1.0f )
    {/* Cross inside piece */
        crossX = point.x + t2 * (beg.y-end.y);
        crossY = point.x + t2 * (beg.x-end.x);
        tmpDist = sqrt((point.x-crossX)*(point.x-crossX) + (point.y-crossY)*(point.y-crossY));
        *distance = ( dist < tmpDist ? dist : tmpDist );
    }
    else
    {
        *distance = dist;
    }


    return CV_OK;
}


/*----------------------------------------------------------------------------------*/


int GetAngleLine( CvPoint2D32f epipole, CvSize imageSize,CvPoint2D32f *point1,CvPoint2D32f *point2)
{
    float epix = epipole.x;
    float epiy = epipole.y;

    float width  = imageSize.width;
    float height = imageSize.height;

    /* Get crosslines with image corners */

    /* Find four lines */

    CvPoint2D32f pa,pb,pc,pd;
    //CvPoint2D32f point1,point2;
    
    pa.x = 0;
    pa.y = 0;

    pb.x = width;
    pb.y = 0;

    pd.x = width;
    pd.y = height;

    pc.x = 0;
    pc.y = height;
    
    /* We can compute points for angle */
    /* Test for place section */

    float x,y;
    x = epipole.x;
    y = epipole.y;
    
    if( x < 0 )
    {/* 1,4,7 */
        if( y < 0)
        {/* 1 */
            *point1 = pb;
            *point2 = pc;
        }
        else if( y > height )
        {/* 7 */
            *point1 = pa;
            *point2 = pd;
        }
        else
        {/* 4 */
            *point1 = pa;
            *point2 = pc;
        }
    }
    else if ( x > width )
    {/* 3,6,9 */
        if( y < 0 )
        {/* 3 */
            *point1 = pa;
            *point2 = pd;
        }
        else if ( y > height )
        {/* 9 */
            *point1 = pb;
            *point2 = pc;
        }
        else
        {/* 6 */
            *point1 = pb;
            *point2 = pd;
        }
    }
    else
    {/* 2,5,8 */
        if( y < 0 )
        {/* 2 */
            if( x < width/2 )
            {
                *point1 = pb;
                *point2 = pa;
            }
            else
            {
                *point1 = pa;
                *point2 = pb;
            }
        }
        else if( y > height )
        {/* 8 */
            if( x < width/2 )
            {
                *point1 = pc;
                *point2 = pd;
            }
            else
            {
                *point1 = pd;
                *point2 = pc;
            }
        }
        else
        {/* 5 - point in the image */
            return 2;
        }
    }
    return 0;
}

/*----------------------------------------------------------------------------------*/



int FindCorrespondLine( CvMatr32f fundMatr,/* Fundamental matrix */
                        CvSize imageSize,/* Image size */
                        CvPoint2D32f epipole1,/* Epipole point on first image */
                        CvPoint2D32f epipole2,/* Epipole point on second image */
                        CvPoint2D32f &anglePoint1,/* image point on first image */
                        CvPoint2D32f &anglePoint2)/* image point on second image */
{/* For point from first image finds line (point) on second image */

    /* Apply matrix to first point */

    CvPoint3D32f pointM;
    CvPoint3D32f tmp3;
    CvPoint2D32f testAnglePoint1;
    CvPoint2D32f testAnglePoint2;

    pointM.x = anglePoint1->x;
    pointM.y = anglePoint1->y;
    pointM.z = 1;
    

    _cvMulMatrix_32f (  fundMatr,
                        3,3,
                        pointM,
                        3,1,
                        tmp3);

    /* Normalize point */    
    testAnglePoint2.x = tmp3.x / tmp3.z;
    testAnglePoint2.y = tmp3.y / tmp3.z;

    /* Now we can compute crossing with image rectangle */
    int cross;

    CvPoint2D32f start;
    CvPoint2D32f end;
    
    cross = FindLine(epipole2,imageSize,testAnglePoint2,&start,&end);
    if( cross > 0 )
    {/* have cross. It's OK */
        *anglePoint2 = testAnglePoint2;
    }
    else
    {/* Not cross use point from second image as base */
        /* Transpose Fundamental matrix */
        
        /* take the up point from second image */

        //CvPoint2D32f basePoint1;/* base point on first image */
        float transFund[9];
        
        _cvTransposeMatrix_32f (fundMatr,
                                3,3,
                                transFund);

        /* Compute corr point */
        
        pointM.x = anglePoint2->x;
        pointM.y = anglePoint2->y;
        pointM.z = 1;
        
        _cvMulMatrix_32f (  transFund,
                            3,3,
                            pointM,
                            3,1,
                            tmp3);

        /* Normalize point */    
        testAnglePoint1.x = tmp3.x / tmp3.z;
        testAnglePoint1.y = tmp3.y / tmp3.z;
        /* We compute new angle point on first image */
        /* Test it for cross */
        cross = FindLine(epipole1,imageSize,testAnglePoint1,&start,&end);

        if( cross == 0 )
        {
            /* Bad case. No crossing */
            return -1;
        }
        else
        {
            *anglePoint1 = testAnglePoint1;            
        }
    }
    return 0;    
}


/*----------------------------------------------------------------------------------*/

int FindCommonArea( CvPoint2D32f angle1Point1,
                    CvPoint2D32f angle1Point2,
                    CvPoint2D32f angle2Point1,
                    CvPoint2D32f angle2Point1,
                    CvPoint2D32f epipole1,
                    CvPoint2D32f epipole2,
                    CvMatr32f    fundMatr,
                    CvSize       imageSize)
{
    /* Test up part of angles (Point1) */
    int result = FindCorrespondLine(    fundMatr,     /* Fundamental matrix */
                                        imageSize,    /* Image size */
                                        epipole1,     /* Epipole point on first image */
                                        epipole2,     /* Epipole point on second image */
                                        &angle1Point1,/* image point on first image */
                                        &angle2Point1);/* image point on second image */

    if( result == -1 )
    {
        /* This is error No up part */
        return -1;
    }

    /* Test down part of angles (Point2) */
    int result = FindCorrespondLine(    fundMatr,     /* Fundamental matrix */
                                        imageSize,    /* Image size */
                                        epipole1,     /* Epipole point on first image */
                                        epipole2,     /* Epipole point on second image */
                                        &angle1Point2,/* image point on first image */
                                        &angle2Point2);/* image point on second image */

    if( result == -1 )
    {
        /* This is error No up part */
        return -1;
    }

    /* Now we know common part of angles */
    /* We must determine number of scanlines */
    


    return 0;
}


/*----------------------------------------------------------------------------------*/


int GetCrossLineDirect(CvPoint2D32f p1,CvPoint2D32f p2,float a,float b,float c,CvPoint2D32f* cross)
{
    double del;
    double delX,delY,delA;

    double px1,px2,py1,py2;
    double X,Y,alpha;

    px1 = p1.x;
    py1 = p1.y;

    px2 = p2.x;
    py2 = p2.y;

    del = a * (px2 - px1) + b * (py2-py1);
    if( del == 0 )
    {
        return -1;
    }

    delA = - c - a*px1 - b*py1;
    alpha = delA / del;

    if( alpha < 0 || alpha > 1.0 )
    {
        return -1;/* no cross */
    }

    delX = b * (py1*(px1-px2) - px1*(py1-py2)) + c * (px1-px2);
    delY = a * (px1*(py1-py2) - py1*(px1-px2)) + c * (py1-py2);

    X = delX / del;
    Y = delY / del;

    cross->x = X;
    cross->y = Y;
    
    return 1;
}

/*--------------------------------------------------------------------------------------*/
int GetCrossPieceVector(CvPoint2D32f p1_start,CvPoint2D32f p1_end,CvPoint2D32f v2_start,CvPoint2D32f v2_end,CvPoint2D32f *cross)
{
    double ex1,ey1,ex2,ey2;
    double px1,py1,px2,py2;
    double del;
    double delA,delB,delX,delY;
    double alpha,betta,X,Y;

    ex1 = p1_start.x;
    ey1 = p1_start.y;
    ex2 = p1_end.x;
    ey2 = p1_end.y;

    px1 = v2_start.x;
    py1 = v2_start.y;
    px2 = v2_end.x;
    py2 = v2_end.y;

    //del = 2*(ex1-ex2)*(py1-py2);
    del = (ex1-ex2)*(py2-py1)+(ey2-ey1)*(px2-px1);
    if( del == 0)
    {
        return -1;
    }

    delA =  (px1-ex1)*(py1-py2) + (ey1-py1)*(px1-px2);
    delB =  (ex1-px1)*(ey1-ey2) + (py1-ey1)*(ex1-ex2);

    alpha =  delA / del;
    betta = -delB / del;

    if( alpha < 0 || alpha > 1.0 )
    {
        return -1;
    }

    delX =  (ex1-ex2)*(py1*(px1-px2)-px1*(py1-py2))+
            (px1-px2)*(ex1*(ey1-ey2)-ey1*(ex1-ex2));

    delY =  (ey1-ey2)*(px1*(py1-py2)-py1*(px1-px2))+
            (py1-py2)*(ey1*(ex1-ex2)-ex1*(ey1-ey2));

    cross->x =  delX / del;
    cross->y = -delY / del;
    return 1;
}


/*----------------------------------------------------------------------------------*/


/* Cross lines */
int GetCrossLines(CvPoint2D32f p1_start,CvPoint2D32f p1_end,CvPoint2D32f p2_start,CvPoint2D32f p2_end,CvPoint2D32f *cross)
{
    double ex1,ey1,ex2,ey2;
    double px1,py1,px2,py2;
    double del;
    double delA,delB,delX,delY;
    double alpha,betta,X,Y;

    ex1 = p1_start.x;
    ey1 = p1_start.y;
    ex2 = p1_end.x;
    ey2 = p1_end.y;

    px1 = p2_start.x;
    py1 = p2_start.y;
    px2 = p2_end.x;
    py2 = p2_end.y;

    del = (ex1-ex2)*(py2-py1)+(ey2-ey1)*(px2-px1);
    if( del == 0)
    {
        return -1;
    }

    delA =  (px1-ex1)*(py1-py2) + (ey1-py1)*(px1-px2);
    delB =  (ex1-px1)*(ey1-ey2) + (py1-ey1)*(ex1-ex2);

    alpha =  delA / del;
    betta = -delB / del;

    if( alpha < 0 || alpha > 1.0 || betta < 0 || betta > 1.0)
    {
        return -1;
    }

    delX =  (ex1-ex2)*(py1*(px1-px2)-px1*(py1-py2))+
            (px1-px2)*(ex1*(ey1-ey2)-ey1*(ex1-ex2));

    delY =  (ey1-ey2)*(px1*(py1-py2)-py1*(px1-px2))+
            (py1-py2)*(ey1*(ex1-ex2)-ex1*(ey1-ey2));

    cross->x =  delX / del;
    cross->y = -delY / del;
    return 1;
}


/*----------------------------------------------------------------------------------*/



/* Find line which cross frame by line(a,b,c) */
void FindLineForEpiline(    CvSize imageSize,
                            float a,float b,float c,
                            CvPoint2D32f *start,CvPoint2D32f *end,
                            int* result)
{
    CvPoint2D32f frameBeg;
    CvPoint2D32f frameEnd;
    CvPoint2D32f cross[4];
    int     haveCross[4];
    float   dist;

    haveCross[0] = 0;
    haveCross[1] = 0;
    haveCross[2] = 0;
    haveCross[3] = 0;

    frameBeg.x = 0;
    frameBeg.y = 0;
    frameEnd.x = imageSize.width;
    frameEnd.y = 0;
    haveCross[0] = GetCrossLineDirect(frameBeg,frameEnd,a,b,c,&cross[0]);
    
    frameBeg.x = imageSize.width;
    frameBeg.y = 0;
    frameEnd.x = imageSize.width;
    frameEnd.y = imageSize.height;
    haveCross[1] = GetCrossLineDirect(frameBeg,frameEnd,a,b,c,&cross[1]);

    frameBeg.x = imageSize.width;
    frameBeg.y = imageSize.height;
    frameEnd.x = 0;
    frameEnd.y = imageSize.height;
    haveCross[2] = GetCrossLineDirect(frameBeg,frameEnd,a,b,c,&cross[2]);
    
    frameBeg.x = 0;
    frameBeg.y = imageSize.height;
    frameEnd.x = 0;
    frameEnd.y = 0;
    haveCross[3] = GetCrossLineDirect(frameBeg,frameEnd,a,b,c,&cross[3]);

    int n;
    float minDist = INT_MAX;
    float maxDist = INT_MIN;

    int maxN = -1;
    int minN = -1;

    double midPointX = imageSize.width  / 2.0;
    double midPointY = imageSize.height / 2.0;

    for( n = 0; n < 4; n++ )
    {
        if( haveCross[n] > 0 )
        {
            dist =  (midPointX - cross[n].x)*(midPointX - cross[n].x) +
                    (midPointY - cross[n].y)*(midPointY - cross[n].y);

            if( dist < minDist )
            {
                minDist = dist;
                minN = n;
            }

            if( dist > maxDist )
            {
                maxDist = dist;
                maxN = n;
            }
        }
    }

    if( minN >= 0 && maxN >= 0 && (minN != maxN) )
    {
        *start = cross[minN];
        *end   = cross[maxN];
    }
    else
    {
        start->x = 0;
        start->y = 0;
        end->x = 0;
        end->y = 0;
    }

    return;
    
}


/*----------------------------------------------------------------------------------*/





/*----------------------------------------------------------------------------------*/

int GetAngleLinee( CvPoint2D32f epipole, CvSize imageSize,CvPoint2D32f point1,CvPoint2D32f point2)
{
    float epix = epipole.x;
    float epiy = epipole.y;

    float width  = imageSize.width;
    float height = imageSize.height;

    /* Get crosslines with image corners */

    /* Find four lines */

    CvPoint2D32f pa,pb,pc,pd;
    //CvPoint2D32f point1,point2;
    
    pa.x = 0;
    pa.y = 0;

    pb.x = width;
    pb.y = 0;

    pd.x = width;
    pd.y = height;

    pc.x = 0;
    pc.y = height;

    /* We can compute points for angle */
    /* Test for place section */

    float x,y;
    x = epipole.x;
    y = epipole.y;
    
    if( x < 0 )
    {/* 1,4,7 */
        if( y < 0)
        {/* 1 */
            point1 = pb;
            point2 = pc;
        }
        else if( y > height )
        {/* 7 */
            point1 = pa;
            point2 = pd;
        }
        else
        {/* 4 */
            point1 = pa;
            point2 = pc;
        }
    }
    else if ( x > width )
    {/* 3,6,9 */
        if( y < 0 )
        {/* 3 */
            point1 = pa;
            point2 = pd;
        }
        else if ( y > height )
        {/* 9 */
            point1 = pc;
            point2 = pb;
        }
        else
        {/* 6 */
            point1 = pb;
            point2 = pd;
        }
    }
    else
    {/* 2,5,8 */
        if( y < 0 )
        {/* 2 */
            point1 = pa;
            point2 = pb;
        }
        else if( y > height )
        {/* 8 */
            point1 = pc;
            point2 = pd;
        }
        else
        {/* 5 - point in the image */
            return 2;
        }

        
    }
    

    return 0;
}

#endif


CV_IMPL  void
cvInitRectify( const CvArr* srcImage, const CvCamera* params, CvArr* rectMap )
{
    CvMat* tempMap = 0;

    CV_FUNCNAME( "cvInitRectify" );

    __BEGIN__;

    double A[64];
    double b[8];
    double c[8];
    CvPoint2D32f pt[4];
    CvMat  mapstub, *map;
    CvSize size;
    int i, j;

    if( !params )
        CV_ERROR( CV_StsNullPtr, "" );

    CV_CALL( map = cvGetMat( rectMap, &mapstub ));

    if( CV_ARR_TYPE( map->type ) != CV_32SC3 )
        CV_ERROR( CV_StsUnsupportedFormat, "" );

    size.width = cvRound(params->imgSize[0]);
    size.height = cvRound(params->imgSize[1]);

    if( map->width != size.width || map->height != size.height )
        CV_ERROR( CV_StsUnmatchedSizes, "" );

    pt[0] = cvPoint2D32f( 0, 0 );
    pt[1] = cvPoint2D32f( size.width, 0 );
    pt[2] = cvPoint2D32f( size.width, size.height );
    pt[3] = cvPoint2D32f( 0, size.height );

    for( i = 0; i < 4; i++ )
    {
#if 0
        double x = params->quad[i].x;
        double y = params->quad[i].y;
        double X = pt[i].x;
        double Y = pt[i].y;
#else
        double x = pt[i].x;
        double y = pt[i].y;
        double X = params->quad[i].x;
        double Y = params->quad[i].y;
#endif
        double* a = A + i*16;
        
        a[0] = x;
        a[1] = y;
        a[2] = 1;
        a[3] = 0;
        a[4] = 0;
        a[5] = 0;
        a[6] = -X*x;
        a[7] = -X*y;

        a += 8;

        a[0] = 0;
        a[1] = 0;
        a[2] = 0;
        a[3] = x;
        a[4] = y;
        a[5] = 1;
        a[6] = -Y*x;
        a[7] = -Y*y;

        b[i*2] = X;
        b[i*2 + 1] = Y;
    }

    {
    double invA[64];
    CvMat matA = cvMat( 8, 8, CV_64F, A );
    CvMat matInvA = cvMat( 8, 8, CV_64F, invA );
    CvMat matB = cvMat( 8, 1, CV_64F, b );
    CvMat matX = cvMat( 8, 1, CV_64F, c );

    CV_CALL( cvPseudoInverse( &matA, &matInvA ));
    CV_CALL( cvMatMulAdd( &matInvA, &matB, 0, &matX ));
    }

    tempMap = cvCreateMat( size.height, size.width, CV_32FC2 );

    for( i = 0; i < size.height; i++ )
    {
        CvPoint2D32f* tptr = (CvPoint2D32f*)(tempMap->data.ptr + tempMap->step*i);
        for( j = 0; j < size.width; j++ )
        {
            double w = 1./(c[6]*j + c[7]*i + 1.);
            double x = (c[0]*j + c[1]*i + c[2])*w;
            double y = (c[3]*j + c[4]*i + c[5])*w;

            tptr[j].x = (float)x;
            tptr[j].y = (float)y;
        }
    }

    CV_CALL( cvConvertMap( srcImage, tempMap, rectMap, 1 ));

    __END__;

    cvReleaseMat( &tempMap );
}
