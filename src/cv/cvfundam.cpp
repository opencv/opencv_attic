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
#include "_cv.h"

/* Valery Mosyagin */

/*=====================================================================================*/
/* new version of fundamental matrix functions */
/*=====================================================================================*/

static int icvComputeFundamental7Point(CvMat* points1,CvMat* points2,
                                     CvMat* fundMatr);

static int icvComputeFundamental8Point(CvMat* points1,CvMat* points2,
                                     CvMat* fundMatr);


static int icvComputeFundamentalRANSAC(   CvMat* points1,CvMat* points2,
                                        CvMat* fundMatr,
                                        double threshold,/* threshold for good point. Distance from epipolar line */
                                        double p,/* probability of good result. Usually = 0.99 */
                                        CvMat* status);

static int icvComputeFundamentalLMedS(   CvMat* points1,CvMat* points2,
                                        CvMat* fundMatr,
                                        double threshold,/* threshold for good point. Distance from epipolar line */
                                        double p,/* probability of good result. Usually = 0.99 */
                                        CvMat* status);

static void icvMakeFundamentalSingular(CvMat* fundMatr);

static void icvNormalizeFundPoints(    CvMat* points,
                                CvMat* transfMatr);

static void icvMake2DPoints(CvMat* srcPoint,CvMat* dstPoint);

static void icvMake3DPoints(CvMat* srcPoint,CvMat* dstPoint);

static int icvCubicV( double a2, double a1, double a0, double *squares );

static void cvComputeCorrespondEpilines(CvMat* points,int pointImageID,CvMat* fundMatr,CvMat* corrLines);

/*=====================================================================================*/

/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:    cvFindFundamentalMat
//    Purpose: find fundamental matrix for given points using 
//    Context:
//    Parameters:
//      points1  - points on first image. Size of matrix 2xN or 3xN
//      points2  - points on second image Size of matrix 2xN or 3xN
//      fundMatr - found fundamental matrix (or matrixes for 7-point). Size 3x3 or 9x3
//      method   - method for computing fundamental matrix
//                 CV_FM_7POINT - for 7-point algorithm. Number of points == 7
//                 CV_FM_8POINT - for 8-point algorithm. Number of points >= 8
//                 CV_FM_RANSAC - for RANSAC  algorithm. Number of points >= 8
//                 CV_FM_LMEDS  - for LMedS   algorithm. Number of points >= 8
//      param1 and param2 uses for RANSAC method
//        param1 - threshold distance from point to epipolar line.
//                 If distance less than threshold point is good.
//        param2 - probability. Usually = 0.99
//        status - array, every element of which will be set to 1 if the point was good
//                 and used for computation. 0 else. (for RANSAC and LMedS only)
//                 (it is optional parameter, can be NULL)
//
//    Returns:
//      number of found matrixes 
//F*/
CV_IMPL int
cvFindFundamentalMat( CvMat* points1,
                      CvMat* points2,
                      CvMat* fundMatr,
                      int    method,
                      double param1,
                      double param2,
                      CvMat* status )
{
    int result = -1;

    CvMat* wpoints[2]={0,0};
    CvMat* tmpPoints[2]={0,0};
    
    CV_FUNCNAME( "icvComputeFundamentalMat" );
    __BEGIN__;

    tmpPoints[0] = points1;
    tmpPoints[1] = points2;
    int numRealPoints[2];
    int numPoints = 0;

    /* work with points */
    {
        int i;
        for( i = 0; i < 2; i++ )
        {
            int realW,realH;
            realW = tmpPoints[i]->cols;
            realH = tmpPoints[i]->rows;

            int goodW,goodH;
            goodW = realW > realH ? realW : realH;
            goodH = realW < realH ? realW : realH;

            if( goodH != 2 && goodH != 3 )
            {
                CV_ERROR(CV_StsBadPoint,"Number of coordinates of points must be 2 or 3");
            }

            wpoints[i] = cvCreateMat(2,goodW,CV_64F);

            numRealPoints[i] = goodW;

            /* Test for transpose point matrix */
            if( realW != goodW )
            {/* need to transpose point matrix */
                CvMat* tmpPointMatr = 0;
                tmpPointMatr = cvCreateMat(goodH,goodW,CV_64F);
                cvTranspose(tmpPoints[i],tmpPointMatr);
                cvMake2DPoints(tmpPointMatr,wpoints[i]);
                cvReleaseMat(&tmpPointMatr);
            }
            else
            {
                cvMake2DPoints(tmpPoints[i],wpoints[i]);
            }

        }

        if( numRealPoints[0] != numRealPoints[1] )
        {
            CV_ERROR(CV_StsBadPoint,"Number of points must be the same");
        }

        numPoints = numRealPoints[0];
    }

    /* work with status if use functions which don't compute it */
    if( status && (method == CV_FM_7POINT || method == CV_FM_8POINT ))
    {
        
        if( !CV_IS_MAT(status) )
        {
            CV_ERROR(CV_StsBadPoint,"status is not a matrix");
        }


        if( !CV_IS_MAT(points1))
        {
            CV_ERROR(CV_StsBadPoint,"Points1 not a matrix");
        }

        if( status->cols != numPoints || status->rows != 1 )
        {
            CV_ERROR(CV_StsBadPoint,"Size of status must be 1xN");
        }

        int i;
        for( i = 0; i < status->cols; i++)
        {
            cvmSet(status,0,i,1.0);
        }

    }


    switch( method )
    {
        case CV_FM_7POINT: result = icvComputeFundamental7Point(wpoints[1], wpoints[0], fundMatr);break;

        case CV_FM_8POINT: result = icvComputeFundamental8Point(wpoints[1],wpoints[0], fundMatr);break;

        case CV_FM_LMEDS : result = icvComputeFundamentalLMedS(   wpoints[1],wpoints[0], fundMatr,
                                        param1,param2,status);break;

        case CV_FM_RANSAC: result = icvComputeFundamentalRANSAC(   wpoints[1],wpoints[0], fundMatr,
                                        param1,param2,status);break;

        //default:return -1/*ERROR*/;
    }

    __END__;

    cvReleaseMat(&wpoints[0]);
    cvReleaseMat(&wpoints[1]);

    return result;
}

/*=====================================================================================*/

/* Computes 1 or 3 fundamental matrixes using 7-point algorithm */
static int icvComputeFundamental7Point(CvMat* points1, CvMat* points2, CvMat* fundMatr)
{

    CvMat* squares = 0;
    int numberRoots = 0;
    
    CV_FUNCNAME( "icvComputeFundamental7Point" );
    __BEGIN__;
    
    /* Test correct of input data */

    if( !CV_IS_MAT(points1) || !CV_IS_MAT(points2)|| !CV_IS_MAT(fundMatr))
    {
        CV_ERROR(CV_StsBadPoint,"Not a matrixes");
    }

    if( !CV_ARE_TYPES_EQ( points1, points2 ))
    {
        CV_ERROR( CV_StsUnmatchedSizes, "Data types of points unmatched" );    
    }

    int numPoint;
    numPoint = points1->cols;
    
    /*int type;
    type = points1->type;*/
    
    if( numPoint != points2->cols )
    {
        CV_ERROR( CV_StsBadSize, "Number of points not equal" );
    }
    if( numPoint != 7 )
    {
        CV_ERROR( CV_StsBadSize, "Number of points must be 7" );
    }

    if( points1->rows != 2 && points1->rows != 3 )
    {
        CV_ERROR( CV_StsBadSize, "Number of coordinates of points1 must be 2 or 3" );
    }

    if( points2->rows != 2 && points2->rows != 3 )
    {
        CV_ERROR( CV_StsBadSize, "Number of coordinates of points2 must be 2 or 3" );
    }

    if( ( fundMatr->rows != 3 && fundMatr->rows != 9 )|| fundMatr->cols != 3 )
    {
        CV_ERROR( CV_StsBadSize, "Size of fundMatr must be 3x3 or 9x3" );
    }

    squares = cvCreateMat(2,3,CV_64F);


    /* Make it Normalize points if need */
    double wPoints1_dat[2*7];
    double wPoints2_dat[2*7];
    CvMat wPoints1;
    CvMat wPoints2;
    wPoints1 = cvMat(2,7,CV_64F,wPoints1_dat);
    wPoints2 = cvMat(2,7,CV_64F,wPoints2_dat);

    icvMake2DPoints(points1,&wPoints1);
    icvMake2DPoints(points2,&wPoints2);

    /* fill matrix U */

    int currPoint;
    CvMat matrU;
    double matrU_dat[7*9];
    matrU = cvMat(7,9,CV_64F,matrU_dat);

    double* currDataLine;
    currDataLine = matrU_dat;
    for( currPoint = 0; currPoint < 7; currPoint++ )
    {
        double x1,y1,x2,y2;
        x1 = cvmGet(&wPoints1,0,currPoint);
        y1 = cvmGet(&wPoints1,1,currPoint);
        x2 = cvmGet(&wPoints2,0,currPoint);
        y2 = cvmGet(&wPoints2,1,currPoint);

        currDataLine[0] = x1*x2;
        currDataLine[1] = x1*y2;
        currDataLine[2] = x1;
        currDataLine[3] = y1*x2;
        currDataLine[4] = y1*y2;
        currDataLine[5] = y1;
        currDataLine[6] = x2;
        currDataLine[7] = y2;
        currDataLine[8] = 1;

        currDataLine += 9;
    }

    CvMat matrUU;
    CvMat matrSS;
    CvMat matrVV;
    double matrUU_dat[7*7];
    double matrSS_dat[7*9];
    double matrVV_dat[9*9];
    matrUU = cvMat(7,7,CV_64F,matrUU_dat);
    matrSS = cvMat(7,9,CV_64F,matrSS_dat);
    matrVV = cvMat(9,9,CV_64F,matrVV_dat);

    cvSVD( &matrU, &matrSS, &matrUU, &matrVV, 0/*CV_SVD_V_T*/ );/* get transposed matrix V */

    double F111,F112,F113;
    double F121,F122,F123;
    double F131,F132,F133;

    double F211,F212,F213;
    double F221,F222,F223;
    double F231,F232,F233;

    F111=cvmGet(&matrVV,0,7);
    F112=cvmGet(&matrVV,1,7);
    F113=cvmGet(&matrVV,2,7);
    F121=cvmGet(&matrVV,3,7);
    F122=cvmGet(&matrVV,4,7);
    F123=cvmGet(&matrVV,5,7);
    F131=cvmGet(&matrVV,6,7);
    F132=cvmGet(&matrVV,7,7);
    F133=cvmGet(&matrVV,8,7);
    
    F211=cvmGet(&matrVV,0,8);
    F212=cvmGet(&matrVV,1,8);
    F213=cvmGet(&matrVV,2,8);
    F221=cvmGet(&matrVV,3,8);
    F222=cvmGet(&matrVV,4,8);
    F223=cvmGet(&matrVV,5,8);
    F231=cvmGet(&matrVV,6,8);
    F232=cvmGet(&matrVV,7,8);
    F233=cvmGet(&matrVV,8,8);

    double a,b,c,d;

    a =   F231*F112*F223 + F231*F212*F123 - F231*F212*F223 + F231*F113*F122 -
          F231*F113*F222 - F231*F213*F122 + F231*F213*F222 - F131*F112*F223 -
          F131*F212*F123 + F131*F212*F223 - F131*F113*F122 + F131*F113*F222 +
          F131*F213*F122 - F131*F213*F222 + F121*F212*F133 - F121*F212*F233 +
          F121*F113*F132 - F121*F113*F232 - F121*F213*F132 + F121*F213*F232 +
          F221*F112*F133 - F221*F112*F233 - F221*F212*F133 + F221*F212*F233 -
          F221*F113*F132 + F221*F113*F232 + F221*F213*F132 - F221*F213*F232 +
          F121*F112*F233 - F111*F222*F133 + F111*F222*F233 - F111*F123*F132 +
          F111*F123*F232 + F111*F223*F132 - F111*F223*F232 - F211*F122*F133 +
          F211*F122*F233 + F211*F222*F133 - F211*F222*F233 + F211*F123*F132 -
          F211*F123*F232 - F211*F223*F132 + F211*F223*F232 + F111*F122*F133 -
          F111*F122*F233 - F121*F112*F133 + F131*F112*F123 - F231*F112*F123;
    
    b =   2*F231*F213*F122 - 3*F231*F213*F222 + F231*F112*F123   - 2*F231*F112*F223 -
          2*F231*F212*F123 + 3*F231*F212*F223 - F231*F113*F122   + 2*F231*F113*F222 +
          F131*F212*F123   - 2*F131*F212*F223 - F131*F113*F222   - F131*F213*F122   +
          2*F131*F213*F222 + F121*F113*F232   + F121*F213*F132   - 2*F121*F213*F232 -
          F221*F112*F133   + 2*F221*F112*F233 + 2*F221*F212*F133 - 3*F221*F212*F233 +
          F221*F113*F132   - 2*F221*F113*F232 - 2*F221*F213*F132 + 3*F221*F213*F232 +
          F131*F112*F223   - 2*F211*F122*F233 - 2*F211*F222*F133 + 3*F211*F222*F233 -
          F211*F123*F132   + 2*F211*F123*F232 + 2*F211*F223*F132 - 3*F211*F223*F232 -
          F121*F112*F233   - F121*F212*F133   + 2*F121*F212*F233 - 2*F111*F222*F233 -
          F111*F123*F232   - F111*F223*F132   + 2*F111*F223*F232 + F111*F122*F233   +
          F111*F222*F133   + F211*F122*F133;
    
    c =   F231*F112*F223   + F231*F212*F123   - 3*F231*F212*F223 - F231*F113*F222   -
          F231*F213*F122   + 3*F231*F213*F222 + F131*F212*F223   - F131*F213*F222   +
          F121*F213*F232   - F221*F112*F233   - F221*F212*F133   + 3*F221*F212*F233 +
          F221*F113*F232   + F221*F213*F132   - 3*F221*F213*F232 + F211*F122*F233   +
          F211*F222*F133   - 3*F211*F222*F233 - F211*F123*F232   - F211*F223*F132   +
          3*F211*F223*F232 - F121*F212*F233   + F111*F222*F233   - F111*F223*F232;
    
    d =   F221*F213*F232 - F211*F223*F232 + F211*F222*F233 - F221*F212*F233 +
          F231*F212*F223 - F231*F213*F222;

    /* find root */
    double coeffs_dat[4];
    CvMat coeffs;
    coeffs = cvMat(1,4,CV_64F,coeffs_dat);

    cvmSet(&coeffs,0,0,a);
    cvmSet(&coeffs,0,1,b);
    cvmSet(&coeffs,0,2,c);
    cvmSet(&coeffs,0,3,d);

    int numCubRoots;
    numCubRoots = cvSolveCubic(&coeffs,squares);

    /* take real solution */
    /* Need test all roots */

    int i;
    for( i = 0; i < numCubRoots; i++ )
    {
        if( fabs(cvmGet(squares,1,i)) < 1e-8 )
        {//It is real square. (Im==0)

            double sol;
            sol = cvmGet(squares,0,i);
            //F=sol*F1+(1-sol)*F2;

            int t;
            for( t = 0; t < 9; t++ )
            {
                double f1,f2;
                f1 = cvmGet(&matrVV,t,7);
                f2 = cvmGet(&matrVV,t,8);

                double s = f1 * sol + (1-sol) * f2;
                cvmSet(fundMatr,numberRoots*3 + t/3,t%3,s);
            }
            numberRoots++;

            if( fundMatr->rows == 3 )/* not enough space to store more than one root */
                break;
        }
    }

    /* scale fundamental matrix */
    for( i = 0; i < numberRoots; i++ )
    {

        double fsc;
        fsc = cvmGet(fundMatr,i*3+2,2);
        if( fabs(fsc) > 1e-8 )
        {
            CvMat subFund;
            cvGetSubArr( fundMatr, &subFund, cvRect(0,i*3,3,3) );
            cvmScale(&subFund,&subFund,1.0/fsc);
        }
    }

    __END__;

    cvReleaseMat(&squares);


    return numberRoots;
}    


/*=====================================================================================*/

static int icvComputeFundamental8Point(CvMat* points1,CvMat* points2, CvMat* fundMatr)
{
    CvMat* wpoints[2]={0,0};
    CvMat* preFundMatr = 0;
    CvMat* sqdists = 0;
    CvMat* matrA = 0;
    CvMat* matrU = 0;
    CvMat* matrW = 0;
    CvMat* matrV = 0;

    int numFundMatrs = 0;

    CV_FUNCNAME( "icvComputeFundamental8Point" );
    __BEGIN__;
    
    /* Test correct of input data */

    if( !CV_IS_MAT(points1) || !CV_IS_MAT(points2)|| !CV_IS_MAT(fundMatr))
    {
        CV_ERROR(CV_StsBadPoint,"Not a matrixes");
    }

    if( !CV_ARE_TYPES_EQ( points1, points2 ))
    {
        CV_ERROR( CV_StsUnmatchedSizes, "Data types of points unmatched" );    
    }

    int numPoint;
    numPoint = points1->cols;
    
    /*int type;
    type = points1->type;*/
    
    if( numPoint != points2->cols )
    {
        CV_ERROR( CV_StsBadSize, "Number of points not equal" );
    }
    if( numPoint < 8 )
    {
        CV_ERROR( CV_StsBadSize, "Number of points must be at least 8" );
    }

    if( points1->rows > 3 || points1->rows < 2 )
    {
        CV_ERROR( CV_StsBadSize, "Number of coordinates of points1 must be 2 or 3" );
    }

    if( points2->rows > 3 || points2->rows < 2 )
    {
        CV_ERROR( CV_StsBadSize, "Number of coordinates of points2 must be 2 or 3" );
    }

    if( fundMatr->rows != 3 || fundMatr->cols != 3 )
    {
        CV_ERROR( CV_StsBadSize, "Size of fundMatr must be 3x3" );
    }

    /* allocate data */
    CV_CALL( wpoints[0] = cvCreateMat(2,numPoint,CV_64F) );
    CV_CALL( wpoints[1] = cvCreateMat(2,numPoint,CV_64F) );
    CV_CALL( matrA = cvCreateMat(numPoint,9,CV_64F) );
    CV_CALL( preFundMatr = cvCreateMat(3,3,CV_64F) );
    CV_CALL( matrU = cvCreateMat(numPoint,numPoint,CV_64F) );
    CV_CALL( matrW = cvCreateMat(numPoint,9,CV_64F) );
    CV_CALL( matrV = cvCreateMat(9,9,CV_64F) );
    CV_CALL( sqdists = cvCreateMat(1,numPoint,CV_64F) );

    /* Create working points with just x,y */

    CvMat transfMatr[2];
    double transfMatr1_dat[9];
    double transfMatr2_dat[9];
    transfMatr[0] = cvMat(3,3,CV_64F,transfMatr1_dat);
    transfMatr[1] = cvMat(3,3,CV_64F,transfMatr2_dat);
    
    {/* transform to x,y.  tranform point and compute transform matrixes */
        icvMake2DPoints(points1,wpoints[0]);
        icvMake2DPoints(points2,wpoints[1]);

        icvNormalizeFundPoints( wpoints[0],&transfMatr[0]);
        icvNormalizeFundPoints( wpoints[1],&transfMatr[1]);
        
        /* we have normalized working points wpoints[0] and wpoints[1] */
        /* build matrix A from points coordinates */

        int currPoint;
        for( currPoint = 0; currPoint < numPoint; currPoint++ )
        {
            CvMat rowA;
            double x1,y1;
            double x2,y2;

            x1 = cvmGet(wpoints[0],0,currPoint);
            y1 = cvmGet(wpoints[0],1,currPoint);
            x2 = cvmGet(wpoints[1],0,currPoint);
            y2 = cvmGet(wpoints[1],1,currPoint);

            cvGetRow(matrA,&rowA,currPoint);
            rowA.data.db[0] = x1*x2;
            rowA.data.db[1] = x1*y2;
            rowA.data.db[2] = x1;

            rowA.data.db[3] = y1*x2;
            rowA.data.db[4] = y1*y2;
            rowA.data.db[5] = y1;

            rowA.data.db[6] = x2;
            rowA.data.db[7] = y2;
            rowA.data.db[8] = 1;
        }
    }

    /* We have matrix A. Compute svd(A). We need last column of V */

    
    cvSVD( matrA, matrW, matrU, matrV, CV_SVD_V_T );/* get transposed matrix V */

    /* Compute number of non zero eigen values */
    int numEig;
    numEig = 0;
    {
        int i;
        for( i = 0; i < 8; i++ )
        {
            if( cvmGet(matrW,i,i) < 1e-8 )
            {
                break;
            }
        }

        numEig = i;

    }

    if( numEig < 5 )
    {/* Bad points */
        numFundMatrs = 0;
    }
    else
    {
        numFundMatrs = 1;

        /* copy last row of matrix V to precomputed fundamental matrix */

        CvMat preF;
        cvGetRow(matrV,&preF,8);
        cvReshape(&preF,preFundMatr,1,3);

        /* Apply singularity condition */
        icvMakeFundamentalSingular(preFundMatr);
    
        /* Denormalize fundamental matrix */
        /* Compute transformation for normalization */

        CvMat wfundMatr;
        double wfundMatr_dat[9];
        wfundMatr = cvMat(3,3,CV_64F,wfundMatr_dat);
    
        {/*  Freal = T1'*Fpre*T2 */
            double tmpMatr_dat[9];
            double tmptransfMatr_dat[9];
        
            CvMat tmpMatr = cvMat(3,3,CV_64F,tmpMatr_dat);
            CvMat tmptransfMatr = cvMat(3,3,CV_64F,tmptransfMatr_dat);

            cvTranspose(&transfMatr[0],&tmptransfMatr);
        
            cvmMul(&tmptransfMatr,preFundMatr,&tmpMatr);
            cvmMul(&tmpMatr,&transfMatr[1],&wfundMatr);
        }

        /* scale fundamental matrix */
        double fsc;
        fsc = cvmGet(&wfundMatr,2,2);
        if( fabs(fsc) > 1.0e-8 )
        {
            cvmScale(&wfundMatr,&wfundMatr,1.0/fsc);
        }
    
        /* copy result fundamental matrix */
        cvConvert( &wfundMatr, fundMatr );

    }


    __END__;
    
    cvReleaseMat(&matrU);
    cvReleaseMat(&matrW);
    cvReleaseMat(&matrV);
    cvReleaseMat(&preFundMatr);
    cvReleaseMat(&matrA);
    cvReleaseMat(&wpoints[0]);
    cvReleaseMat(&wpoints[1]);
    cvReleaseMat(&sqdists);

    return numFundMatrs;
}

/*=====================================================================================*/

/* Computes fundamental matrix using RANSAC method */
/*  */
static int icvComputeFundamentalRANSAC(   CvMat* points1,CvMat* points2,
                                        CvMat* fundMatr,
                                        double threshold,/* Threshold for good points */
                                        double p,/* Probability of good result. */
                                        CvMat* status)    
{
    CvMat* wpoints1 = 0;
    CvMat* wpoints2 = 0;
    CvMat* corrLines1 = 0;
    CvMat* corrLines2 = 0;
    CvMat* bestPoints1 = 0;
    CvMat* bestPoints2 = 0;

    int* flags = 0;
    int* bestFlags = 0;
    int numFundMatr = 0;
    
    CV_FUNCNAME( "icvComputeFundamentalRANSAC" );
    __BEGIN__;

    /* Test correct of input data */

    if( !CV_IS_MAT(points1) || !CV_IS_MAT(points2)|| !CV_IS_MAT(fundMatr))
    {
        CV_ERROR(CV_StsBadPoint,"points1 or points2 or funMatr are not a matrixes");
    }

    int numPoint;
    numPoint = points1->cols;
    
    /*int type;
    type = points1->type;*/
    
    if( numPoint != points2->cols )
    {
        CV_ERROR( CV_StsBadSize, "Number of points not equals" );
    }
    if( numPoint < 8 )
    {
        CV_ERROR( CV_StsBadSize, "Number of points must be >= 8" );
    }

    if( points1->rows != 2 && points1->rows != 3 )
    {
        CV_ERROR( CV_StsBadSize, "Number of coordinates of points1 must be 2 or 3" );
    }

    if( points2->rows != 2 && points2->rows != 3 )
    {
        CV_ERROR( CV_StsBadSize, "Number of coordinates of points2 must be 2 or 3" );
    }

    if( fundMatr->rows != 3 || fundMatr->cols != 3 )
    {
        CV_ERROR( CV_StsBadSize, "Size of fundMatr must be 3x3" );
    }

    if( status )
    {/* status is present test it */
        if( !CV_IS_MAT(status) )
        {
            CV_ERROR(CV_StsBadPoint,"status is not a matrix");
        }

        if( status->cols != numPoint || status->rows != 1 )
        {
            CV_ERROR(CV_StsBadPoint,"Size of status must be 1xN");
        }
    }
       
    /* We will choose adaptive number of N (samples) */

    /* Convert points to 64F working points */

    CV_CALL( flags = (int*)cvAlloc(numPoint * sizeof(int)) );
    CV_CALL( bestFlags = (int*)cvAlloc(numPoint * sizeof(int)) );
    CV_CALL( wpoints1 = cvCreateMat(3,numPoint,CV_64F) );
    CV_CALL( wpoints2 = cvCreateMat(3,numPoint,CV_64F) );
    CV_CALL( corrLines1 = cvCreateMat(3,numPoint,CV_64F));
    CV_CALL( corrLines2 = cvCreateMat(3,numPoint,CV_64F));
    CV_CALL( bestPoints1 = cvCreateMat(3,numPoint,CV_64F));
    CV_CALL( bestPoints2 = cvCreateMat(3,numPoint,CV_64F));

    icvMake3DPoints(points1,wpoints1);
    icvMake3DPoints(points2,wpoints2);
        
    {
        int wasCount = 0;  //count of choosing samples
        int maxGoodPoints = 0;
        int numGoodPoints = 0;

        double bestFund_dat[9];
        CvMat  bestFund;
        bestFund = cvMat(3,3,CV_64F,bestFund_dat);

        /* choosen points */        
        int NumSamples = 500;/* Initial need number of samples */
        while( wasCount < NumSamples )
        {
            /* select samples */
            int randNumbs[7];
            int i;
            int newnum;
            int pres;
            for( i = 0; i < 7; i++ )
            {
                do
                {
                    newnum = rand()%numPoint;

                    /* test this number */
                    pres = 0;
                    int test;
                    for( test = 0; test < i; test++ )
                    {
                        if( randNumbs[test] == newnum )
                        {
                            pres = 1;
                            break;
                        }
                    }
                }
                while( pres );
                randNumbs[i] = newnum;
            }
            /* random numbers of points was generated */
            /* select points */

            double selPoints1_dat[2*7];
            double selPoints2_dat[2*7];
            CvMat selPoints1;
            CvMat selPoints2;
            selPoints1 = cvMat(2,7,CV_64F,selPoints1_dat);
            selPoints2 = cvMat(2,7,CV_64F,selPoints2_dat);
            /* copy points */
            int t;
            for( t = 0; t < 7; t++ )
            {
                double x,y;
                x = cvmGet(wpoints1,0,randNumbs[t]);
                y = cvmGet(wpoints1,1,randNumbs[t]);
                cvmSet(&selPoints1,0,t,x);
                cvmSet(&selPoints1,1,t,y);
                
                x = cvmGet(wpoints2,0,randNumbs[t]);
                y = cvmGet(wpoints2,1,randNumbs[t]);
                cvmSet(&selPoints2,0,t,x);
                cvmSet(&selPoints2,1,t,y);
            }

            /* Compute fundamental matrix using 7-points algorithm */

            double fundTriple_dat[27];
            CvMat fundTriple;
            fundTriple = cvMat(9,3,CV_64F,fundTriple_dat);

            int numFund = icvComputeFundamental7Point(&selPoints1,&selPoints2,&fundTriple);

            //double fund7_dat[9];
            CvMat fund7;
            //fund7 = cvMat(3,3,CV_64F,fund7_dat);

            /* get sub matrix */
            for( int currFund = 0; currFund < numFund; currFund++ )
            {
                cvGetSubArr(&fundTriple,&fund7,cvRect(0,currFund*3,3,3));
                {
                    /* Compute inliers for this fundamental matrix */
                    /* Compute distances for points and correspond lines */
                    {
                        /* Create corresponde lines */
                    
                        cvComputeCorrespondEpilines(wpoints1,2,&fund7,corrLines2);
                        cvComputeCorrespondEpilines(wpoints2,1,&fund7,corrLines1);
                        /* compute distances for points and number of good points */
                        int i;
                        numGoodPoints = 0;
                        for( i = 0; i < numPoint; i++ )
                        {
                            CvMat pnt1,pnt2;
                            CvMat lin1,lin2;
                            cvGetCol(wpoints1,&pnt1,i);
                            cvGetCol(corrLines1,&lin1,i);
                            cvGetCol(wpoints2,&pnt2,i);
                            cvGetCol(corrLines2,&lin2,i);
                            double dist1,dist2;
                            dist1 = fabs(cvDotProduct(&pnt1,&lin1));
                            dist2 = fabs(cvDotProduct(&pnt2,&lin2));
                            flags[i] = ( dist1 < threshold && dist2 < threshold )?1:0;
                            numGoodPoints += flags[i];
                        }
                    }
                }

                if( numGoodPoints > maxGoodPoints )
                {/* good matrix */
                    cvCopy(&fund7,&bestFund);
                    maxGoodPoints = numGoodPoints;
                    /* copy best flags */
                    int i;
                    for(i=0;i<numPoint;i++)
                    {
                        bestFlags[i] = flags[i];
                    }

                    /* Recompute new number of need steps */

                    /* Adaptive number of samples to count*/
                    double ep = 1 - (double)numGoodPoints / (double)numPoint;
                    if( ep == 1 )
                    {
                        ep = 0.5;//if there is not good points set ration of outliers to 50%
                    }
            
                    double newNumSamples = log(1-p);
                    newNumSamples /= log(1-pow(1-ep,7));

                    if( newNumSamples < (double)NumSamples )
                    {
                        NumSamples = cvRound(newNumSamples);
                    }

                }
            }
            
            wasCount++;
        }

        /* we have best 7-point fundamental matrix. */
        /* and best points */
        /* use these points to improve matrix */

        /* collect best points */
        /* copy points */
        
        /* Test number of points. And if number >=8 improove found fundamental matrix  */

        if( maxGoodPoints < 7 )
        {
            /* Fundamental matrix not found */
            numFundMatr = 0;
        }
        else
        {
            if( maxGoodPoints > 7 )
            {
                /* Found >= 8 point. Improove matrix */
                int i;
                int currPnt;
                currPnt = 0;

                for( i = 0; i < numPoint; i++ )
                {
                    if( bestFlags[i] )
                    {
                        CvMat wPnt;
                        CvMat bestPnt;
                        cvGetCol( wpoints1,&wPnt, i );
                        cvGetCol( bestPoints1,&bestPnt, currPnt );
                        cvCopy(&wPnt,&bestPnt);
                        cvGetCol( wpoints2,&wPnt, i );
                        cvGetCol( bestPoints2,&bestPnt, currPnt );
                        cvCopy(&wPnt,&bestPnt);
                        currPnt++;
                    }
                }

                CvMat wbestPnts1;
                CvMat wbestPnts2;

                cvGetSubArr( bestPoints1, &wbestPnts1, cvRect(0,0,maxGoodPoints,3) );
                cvGetSubArr( bestPoints2, &wbestPnts2, cvRect(0,0,maxGoodPoints,3) );

                /* best points was collectet. Improve fundamental matrix */
                /* Just use 8-point algorithm */
                double impFundMatr_dat[9];
                CvMat impFundMatr;
                impFundMatr = cvMat(3,3,CV_64F,impFundMatr_dat);
                numFundMatr = icvComputeFundamental8Point(&wbestPnts1,&wbestPnts2,&impFundMatr);

                cvConvert(&impFundMatr,fundMatr);        
                //cvConvert(&bestFund,fundMatr); // This line must be deleted
            }
            else
            {
                /* 7 point. Just copy to result */
                cvConvert(&bestFund,fundMatr);
                numFundMatr = 1;
            }

            /* copy flag to status if need */
            if( status )
            {
                for( int i = 0; i < numPoint; i++)
                {
                    cvmSet(status,0,i,(double)bestFlags[i]);
                }
            }
        }


    }

    __END__;

    /* free allocated memory */
    
    cvReleaseMat(&corrLines1);
    cvReleaseMat(&corrLines2);
    cvReleaseMat(&wpoints1);
    cvReleaseMat(&wpoints2);
    cvReleaseMat(&bestPoints1);
    cvReleaseMat(&bestPoints2);
    cvFree((void**)&flags);
    cvFree((void**)&bestFlags);

    return numFundMatr;
}//icvComputeFundamentalRANSAC

/*=====================================================================================*/

static void icvCompPointLineDists(CvMat* points,CvMat* lines,CvMat* distances)
{/* Line must be normalized */
    
    int numPoints;
    numPoints = points->cols;
    if( numPoints != lines->cols && numPoints != distances->cols )
    {
        //printf("Size if arrays not equals\n");
        return;//error
    }

    int i;
    for( i = 0; i < numPoints; i++ )
    {
        CvMat pnt;
        CvMat lin;
        cvGetCol(points,&pnt,i);
        cvGetCol(lines,&lin,i);
        double dist;
        dist = fabs(cvDotProduct(&pnt,&lin));
        cvmSet(distances,0,i,dist);
    }

    return;
}

/*=====================================================================================*/



#define _compVals( v1, v2 )  ((v1) < (v2))

/* Create function to sort vector */
static CV_IMPLEMENT_QSORT( _SortCvMatVect, double, _compVals )

/*=====================================================================================*/
static int icvComputeFundamentalLMedS(    CvMat* points1,CvMat* points2,
                                    CvMat* fundMatr,
                                    double threshold,/* Threshold for good points */
                                    double p,/* Probability of good result. */
                                    CvMat* status)    
{
    CvMat* wpoints1 = 0;
    CvMat* wpoints2 = 0;
    CvMat* corrLines1 = 0;
    CvMat* corrLines2 = 0;
    CvMat* bestPoints1 = 0;
    CvMat* bestPoints2 = 0;
    CvMat* dists1 = 0;
    CvMat* dists2 = 0;
    CvMat* distsSq1 = 0;
    CvMat* distsSq2 = 0;
    CvMat* allDists = 0;

    int* flags = 0;
    int* bestFlags = 0;
    int numFundMatr = 0;
    
    CV_FUNCNAME( "icvComputeFundamentalLMedS" );
    __BEGIN__;

    /* Test correct of input data */

    if( !CV_IS_MAT(points1) || !CV_IS_MAT(points2)|| !CV_IS_MAT(fundMatr))
    {
        CV_ERROR(CV_StsBadPoint,"Not a matrixes");
    }

    int numPoint;
    numPoint = points1->cols;
    
    /*int type;
    type = points1->type;*/
    
    if( numPoint != points2->cols )
    {
        CV_ERROR( CV_StsBadSize, "Number of points not equal" );
    }
    if( numPoint < 8 )
    {
        CV_ERROR( CV_StsBadSize, "Number of points must be >= 8" );
    }

    if( points1->rows != 2 && points1->rows != 3 )
    {
        CV_ERROR( CV_StsBadSize, "Number of coordinates of points1 must be 2 or 3" );
    }

    if( points2->rows != 2 && points2->rows != 3 )
    {
        CV_ERROR( CV_StsBadSize, "Number of coordinates of points2 must be 2 or 3" );
    }

    if( fundMatr->rows != 3 || fundMatr->cols != 3 )
    {
        CV_ERROR( CV_StsBadSize, "Size of fundMatr must be 3x3" );
    }

    if( status )
    {/* status is present test it */
        if( !CV_IS_MAT(status) )
        {
            CV_ERROR(CV_StsBadPoint,"status is not a matrix");
        }

        if( status->cols != numPoint || status->rows != 1 )
        {
            CV_ERROR(CV_StsBadPoint,"Size of status must be 1xN");
        }
    }
       
    /* We will choose adaptive number of N (samples) */

    /* Convert points to 64F working points */

    CV_CALL( flags = (int*)cvAlloc(numPoint * sizeof(int)) );
    CV_CALL( bestFlags = (int*)cvAlloc(numPoint * sizeof(int)) );
    CV_CALL( wpoints1 = cvCreateMat(3,numPoint,CV_64F) );
    CV_CALL( wpoints2 = cvCreateMat(3,numPoint,CV_64F) );
    CV_CALL( corrLines1 = cvCreateMat(3,numPoint,CV_64F));
    CV_CALL( corrLines2 = cvCreateMat(3,numPoint,CV_64F));
    CV_CALL( bestPoints1 = cvCreateMat(3,numPoint,CV_64F));
    CV_CALL( bestPoints2 = cvCreateMat(3,numPoint,CV_64F));
    CV_CALL( dists1 = cvCreateMat(1,numPoint,CV_64F));
    CV_CALL( dists2 = cvCreateMat(1,numPoint,CV_64F));
    CV_CALL( distsSq1 = cvCreateMat(1,numPoint,CV_64F));
    CV_CALL( distsSq2 = cvCreateMat(1,numPoint,CV_64F));
    CV_CALL( allDists = cvCreateMat(1,numPoint,CV_64F));

    icvMake3DPoints(points1,wpoints1);
    icvMake3DPoints(points2,wpoints2);
    
    {
        int NumSamples = 500;//Maximux number of steps
        int wasCount = 0;  //count of choosing samples

        double goodMean = FLT_MAX;
        double currMean;

        int numGoodPoints = 0;

        double bestFund_dat[9];
        CvMat  bestFund;
        bestFund = cvMat(3,3,CV_64F,bestFund_dat);

        /* choosen points */        
        while( wasCount < NumSamples )
        {
            /* select samples */
            int randNumbs[7];
            int i;
            int newnum;
            int pres;
            for( i = 0; i < 7; i++ )
            {
                do
                {
                    newnum = rand()%numPoint;

                    /* test this number */
                    pres = 0;
                    int test;
                    for( test = 0; test < i; test++ )
                    {
                        if( randNumbs[test] == newnum )
                        {
                            pres = 1;
                            break;
                        }
                    }
                }
                while( pres );
                randNumbs[i] = newnum;
            }
            /* random numbers of points was generated */
            /* select points */

            double selPoints1_dat[2*7];
            double selPoints2_dat[2*7];
            CvMat selPoints1;
            CvMat selPoints2;
            selPoints1 = cvMat(2,7,CV_64F,selPoints1_dat);
            selPoints2 = cvMat(2,7,CV_64F,selPoints2_dat);
            /* copy points */
            int t;
            for( t = 0; t < 7; t++ )
            {
                double x,y;
                x = cvmGet(wpoints1,0,randNumbs[t]);
                y = cvmGet(wpoints1,1,randNumbs[t]);
                cvmSet(&selPoints1,0,t,x);
                cvmSet(&selPoints1,1,t,y);
                
                x = cvmGet(wpoints2,0,randNumbs[t]);
                y = cvmGet(wpoints2,1,randNumbs[t]);
                cvmSet(&selPoints2,0,t,x);
                cvmSet(&selPoints2,1,t,y);
            }
            /* Compute fundamental matrix using 7-points algorithm */

            double fundTriple_dat[27];
            CvMat fundTriple;
            fundTriple = cvMat(9,3,CV_64F,fundTriple_dat);

            int numFund = icvComputeFundamental7Point(&selPoints1,&selPoints2,&fundTriple);

            //double fund7_dat[9];
            CvMat fund7;
            //fund7 = cvMat(3,3,CV_64F,fund7_dat);

            /* get sub matrix */
            for( int currFund = 0; currFund < numFund; currFund++ )
            {

                cvGetSubArr(&fundTriple,&fund7,cvRect(0,currFund*3,3,3));
                {
                    /* Compute median error for this matrix */
                    {
                        cvComputeCorrespondEpilines(wpoints1,2,&fund7,corrLines2);
                        cvComputeCorrespondEpilines(wpoints2,1,&fund7,corrLines1);

                        icvCompPointLineDists(wpoints1,corrLines1,dists1);
                        icvCompPointLineDists(wpoints2,corrLines2,dists2);

                        /* add distances for points (d1*d1+d2*d2) */
                        cvMul(dists1,dists1,distsSq1);
                        cvMul(dists2,dists2,distsSq2);

                        cvAdd(distsSq1,distsSq2,allDists);

                        /* sort distances */
                        _SortCvMatVect(allDists->data.db,numPoint,0);

                        /* get median error */
                        currMean = allDists->data.db[numPoint/2];
                    }
                }

                if( currMean < goodMean )
                {/* good matrix */
                    cvCopy(&fund7,&bestFund);
                    goodMean = currMean;

                    /* Compute number of good points using threshold */
                    int i;
                    numGoodPoints = 0;
                    for( i = 0 ; i < numPoint; i++ )
                    {
                        if( dists1->data.db[i] < threshold && dists2->data.db[i] < threshold )
                        {
                            numGoodPoints++;
                        }
                    }


                    /* Compute adaptive number of steps */
                    double ep = 1 - (double)numGoodPoints / (double)numPoint;
                    if( ep == 1 )
                    {
                        ep = 0.5;//if there is not good points set ration of outliers to 50%
                    }

                    double newNumSamples = log(1-p);
                    newNumSamples /= log(1-pow(1-ep,7));
                    if( newNumSamples < (double)NumSamples )
                    {
                        NumSamples = cvRound(newNumSamples);
                    }
                }
            }

            wasCount++;
        }

        /* Select just good points using threshold */
        /* Compute distances for all points for best fundamental matrix */

        /* Test if we have computed fundamental matrix*/
        if( goodMean == FLT_MAX )
        {
            numFundMatr = 0;

        }
        else
        {/* we have computed fundamental matrix */
            {
                cvComputeCorrespondEpilines(wpoints1,2,&bestFund,corrLines2);
                cvComputeCorrespondEpilines(wpoints2,1,&bestFund,corrLines1);

                icvCompPointLineDists(wpoints1,corrLines1,dists1);
                icvCompPointLineDists(wpoints2,corrLines2,dists2);

                /* test dist for each point and set status for each point if need */
                int i;
                int currPnt = 0;
                for( i = 0; i < numPoint; i++ )
                {
                    if( dists1->data.db[i] < threshold && dists2->data.db[i] < threshold )
                    {
                        CvMat wPnt;
                        CvMat bestPnt;
                        cvGetCol( wpoints1,&wPnt, i );
                        cvGetCol( bestPoints1,&bestPnt, currPnt );
                        cvCopy(&wPnt,&bestPnt);
                        cvGetCol( wpoints2,&wPnt, i );
                        cvGetCol( bestPoints2,&bestPnt, currPnt );
                        cvCopy(&wPnt,&bestPnt);
                        currPnt++;
                                        
                        if( status )
                            cvmSet(status,0,i,1.0);
                    }
                    else
                    {
                        if( status )
                            cvmSet(status,0,i,0.0);
                    }

                }
                numGoodPoints = currPnt;
            }

            /* we have best 7-point fundamental matrix. */
            /* and best points */
            /* use these points to improove matrix */

            /* Test number of points. And if number >=8 improove found fundamental matrix  */

            if( numGoodPoints < 7 )
            {
                /* Fundamental matrix not found */
                numFundMatr = 0;
            }
            else
            {
                if( numGoodPoints > 7 )
                {
                    /* Found >= 8 point. Improove matrix */

                    CvMat wbestPnts1;
                    CvMat wbestPnts2;

                    cvGetSubArr( bestPoints1, &wbestPnts1, cvRect(0,0,numGoodPoints,3) );
                    cvGetSubArr( bestPoints2, &wbestPnts2, cvRect(0,0,numGoodPoints,3) );

                    /* best points was collectet. Improve fundamental matrix */
                    /* Just use 8-point algorithm */
                    double impFundMatr_dat[9];
                    CvMat impFundMatr;
                    impFundMatr = cvMat(3,3,CV_64F,impFundMatr_dat);
                    numFundMatr = icvComputeFundamental8Point(&wbestPnts1,&wbestPnts2,&impFundMatr);

                    cvConvert(&impFundMatr,fundMatr);        
                }
                else
                {
                    /* 7 point. Just copy to result */
                    cvConvert(&bestFund,fundMatr);
                    numFundMatr = 1;
                }
            }

        }
    }

    __END__;

    /* free allocated memory */
    
    cvReleaseMat(&corrLines1);
    cvReleaseMat(&corrLines2);
    cvReleaseMat(&wpoints1);
    cvReleaseMat(&wpoints2);
    cvReleaseMat(&bestPoints1);
    cvReleaseMat(&bestPoints2);
    cvReleaseMat(&dists1);
    cvReleaseMat(&dists2);
    cvReleaseMat(&distsSq1);
    cvReleaseMat(&distsSq2);
    cvReleaseMat(&allDists);
    cvFree((void**)&flags);
    cvFree((void**)&bestFlags);

    return numFundMatr;
}//icvComputeFundamentalLMedS

/*=====================================================================================*/



static void icvMakeFundamentalSingular(CvMat* fundMatr)
{
    CV_FUNCNAME( "icvFundSingular" );
    __BEGIN__;

    if( !CV_IS_MAT(fundMatr) )
    {
        CV_ERROR(CV_StsBadPoint,"Input data is not matrix");
    }
    
    if( fundMatr->rows != 3 || fundMatr->cols != 3 )
    {
        CV_ERROR( CV_StsBadSize, "Size of fundametal matrix must be 3x3" );
    }

    
    {/* Apply singularity condition */
        CvMat matrFU;
        CvMat matrFW;
        CvMat matrFVt;
        CvMat tmpMatr;
        CvMat preFundMatr;
        double matrFU_dat[9];
        double matrFW_dat[9];
        double matrFVt_dat[9];
        double tmpMatr_dat[9];
        double preFundMatr_dat[9];

        
        matrFU  = cvMat(3,3,CV_64F,matrFU_dat);
        matrFW  = cvMat(3,3,CV_64F,matrFW_dat);
        matrFVt = cvMat(3,3,CV_64F,matrFVt_dat);
        tmpMatr = cvMat(3,3,CV_64F,tmpMatr_dat);
        preFundMatr = cvMat(3,3,CV_64F,preFundMatr_dat);

        cvConvert(fundMatr,&preFundMatr);
        cvSVD( &preFundMatr, &matrFW, &matrFU, &matrFVt, CV_SVD_V_T );
        cvmSet(&matrFW,2,2,0);
        /* multiply U*W*V' */

        cvmMul(&matrFU,&matrFW,&tmpMatr);
        cvmMul(&tmpMatr,&matrFVt,&preFundMatr);
        cvConvert(&preFundMatr,fundMatr);
    }
    

    __END__;
}


/*=====================================================================================*/
/* Normalize points for computing fundamental matrix */
/* and compute transform matrix */
/* Points:  2xN  */
/* Matrix:  3x3 */
/* place centroid of points to (0,0) */
/* set mean distance from (0,0) by sqrt(2) */

static void icvNormalizeFundPoints( CvMat* points, CvMat* transfMatr )
{
    CvMat* subwpointsx = 0;
    CvMat* subwpointsy = 0;
    CvMat* sqdists     = 0;
    CvMat* pointsxx    = 0;
    CvMat* pointsyy    = 0;

    int numPoint;
    double shiftx,shifty; 
    double meand;
    double scale;

    CvMat tmpwpointsx;
    CvMat tmpwpointsy;

    CvScalar sumx;
    CvScalar sumy;

    CV_FUNCNAME( "icvNormalizeFundPoints" );
    __BEGIN__;
    
    /* Test for correct input data */

    if( !CV_IS_MAT(points) || !CV_IS_MAT(transfMatr) )
    {
        CV_ERROR(CV_StsBadPoint,"Input data is not matrixes");
    }
    
    numPoint = points->cols;
    
    if( numPoint < 1 )
    {
        CV_ERROR( CV_StsBadSize, "Number of points must be at least 1" );
    }

    if( points->rows != 2 )
    {
        CV_ERROR( CV_StsBadSize, "Number of coordinates of points1 must be 2" );
    }

    if( transfMatr->rows != 3 || transfMatr->cols != 3 )
    {
        CV_ERROR( CV_StsBadSize, "Size of transform matrix must be 3x3" );
    }

    CV_CALL( subwpointsx  =  cvCreateMat(1,numPoint,CV_64F) );
    CV_CALL( subwpointsy  =  cvCreateMat(1,numPoint,CV_64F) );
    CV_CALL( sqdists      =  cvCreateMat(1,numPoint,CV_64F) );
    CV_CALL( pointsxx     =  cvCreateMat(1,numPoint,CV_64F) );
    CV_CALL( pointsyy     =  cvCreateMat(1,numPoint,CV_64F) );

    /* get x,y coordinates of points */
    
    {
        cvGetRow( points, &tmpwpointsx, 0 );
        cvGetRow( points, &tmpwpointsy, 1 );

        /* Copy to working data 64F */
        cvConvert(&tmpwpointsx,subwpointsx);
        cvConvert(&tmpwpointsy,subwpointsy);
    }

    /* Compute center of points */

    sumx = cvSum(subwpointsx);
    sumy = cvSum(subwpointsy);

    sumx.val[0] /= (double)numPoint;
    sumy.val[0] /= (double)numPoint;

    shiftx = sumx.val[0];
    shifty = sumy.val[0];

    /* Shift points center to 0 */

    cvSubS( subwpointsx, sumx, subwpointsx);
    cvSubS( subwpointsy, sumy, subwpointsy);

    /* Compute x*x and y*y */        

    cvMul(subwpointsx,subwpointsx,pointsxx);
    cvMul(subwpointsy,subwpointsy,pointsyy);
    
    /* add  */
    cvAdd( pointsxx, pointsyy, sqdists);

    /* compute sqrt of each component*/
    
    cvPow(sqdists,sqdists,0.5);

    /* in vector sqdists we have distances */
    /* compute mean value and scale */
    
    meand = cvAvg(sqdists).val[0];
    
    if( fabs(meand) > 1e-8  )
    {
        scale = 0.70710678118654752440084436210485/meand;
    }
    else
    {
        scale = 1.0;
    }

    /* scale working points */    
    cvScale(subwpointsx,subwpointsx,scale);
    cvScale(subwpointsy,subwpointsy,scale);

    /* copy output data */
    {
        cvGetRow( points, &tmpwpointsx, 0 );
        cvGetRow( points, &tmpwpointsy, 1 );

        /* Copy to output data 64F */
        cvConvert(subwpointsx,&tmpwpointsx);
        cvConvert(subwpointsy,&tmpwpointsy);
    }

    /* Set transform matrix */
    
    cvmSet(transfMatr,0,0, scale);
    cvmSet(transfMatr,0,1, 0);
    cvmSet(transfMatr,0,2, -scale*shiftx);

    cvmSet(transfMatr,1,0, 0);
    cvmSet(transfMatr,1,1, scale);
    cvmSet(transfMatr,1,2, -scale*shifty);

    cvmSet(transfMatr,2,0, 0);
    cvmSet(transfMatr,2,1, 0);
    cvmSet(transfMatr,2,2, 1);

    __END__;
    
    /* Free data */
    cvReleaseMat(&subwpointsx);
    cvReleaseMat(&subwpointsy);
    cvReleaseMat(&sqdists);
    cvReleaseMat(&pointsxx);
    cvReleaseMat(&pointsyy);

}

/*=====================================================================================*/
// Solve cubic equation and returns number of roots
// Also returns 0 if all values are possible
// Test for very big coefficients
// Input params 1x3 or 1x4
CV_IMPL int cvSolveCubic(CvMat* coeffs,CvMat* result)
{/* solve a*x^3 + b+x^2 + c*x + d = 0 */
    /* coeffs a,b,c,d or b,c,d if a== 1*/
    /* test input params */
    /* result 2x3  */

    int numRoots = 0;

    CV_FUNCNAME( "icvSolveCubic" );
    __BEGIN__;
    
    /* Test correct of input data */

    if( !CV_IS_MAT(coeffs) || !CV_IS_MAT(result) )
    {
        CV_ERROR(CV_StsBadPoint,"Not a matrixes");
    }

    if( !(coeffs->rows == 1 && (coeffs->cols == 3 || coeffs->cols == 4) ))
    {
        CV_ERROR( CV_StsBadSize, "Number of coeffs must be 3 or 4" );
    }


    double squares[6];
    double cc[4];
    cc[0] = cvmGet(coeffs,0,0);
    cc[1] = cvmGet(coeffs,0,1);
    cc[2] = cvmGet(coeffs,0,2);

    if( fabs(cc[0]) > FLT_MAX || fabs(cc[1]) > FLT_MAX || fabs(cc[2]) > FLT_MAX )
    {
        return 0;//Coeffs too big
    }

    double a0,a1,a2;
    if( coeffs->cols == 3 )
    {
        a0 = cc[0];
        a1 = cc[1];
        a2 = cc[2];
        numRoots = icvCubicV(a0,a1,a2,squares);
    }
    else
    {// We have for coeffs
        /* Test for very big coeffs */
        cc[3] = cvmGet(coeffs,0,3);

        if( fabs(cc[3]) > FLT_MAX )
        {
            return 0;//Coeffs too big
        }

        double a = cc[0];
        if( fabs(a) > FLT_MIN)
        {
            a = 1. / a;
            a0 = cc[1] * a;
            a1 = cc[2] * a;
            a2 = cc[3] * a;
            numRoots = icvCubicV(a0,a1,a2,squares);
        }
        else
        {// It's a square eqaution.
            double a,b,c;
            a = cc[1];
            b = cc[2];
            c = cc[3];
            if( fabs(a) > 1e-8 )
            {
                double D;
                D = b*b-4*a*c;
                if( D > FLT_MIN )
                {// Two roots
                    numRoots = 2;
                    squares[0] = (-b + sqrt(D))/(2*a);
                    squares[1] = 0;
                    squares[2] = (-b - sqrt(D))/(2*a);
                    squares[3] = 0;
                }
                else
                {
                    if( D < FLT_MIN  )
                    {/* Two Im values */
                        numRoots = 2;
                        squares[0] = (-b)/(2*a);
                        squares[1] = (  sqrt(-D))/(2*a);

                        squares[2] = (-b)/(2*a);
                        squares[3] = ( -sqrt(-D))/(2*a);
                    }
                    else
                    {/* D==0 */
                        numRoots = 2;
                        squares[0] = (-b)/(2*a);
                        squares[1] = 0;
                        squares[2] = (-b)/(2*a);
                        squares[3] = 0;
                    }
                }
            }
            else
            {// Linear equation
                if( fabs(b) > FLT_MIN )
                {
                    squares[0] = -c/b;
                    squares[1] = 0;
                    numRoots = 1;
                }
                else
                {
                    if( fabs(c) > FLT_MIN)
                    {
                        numRoots = 0;
                    }
                    else
                    {
                        /* All values are posible */
                        numRoots = 0;// !!!
                        //cvmSet(result,0,0,0);
                        //cvmSet(result,1,0,0);
                    }
                }
            }
        }
    }

    /* copy result  */
    int i;

    for( i=0;i<numRoots;i++ )
    {
        cvmSet(result,0,i,squares[i*2]);
        cvmSet(result,1,i,squares[i*2+1]);
    }
    __END__;

    return numRoots;
}

/*=====================================================================================*/
void cvMake2DPoints(CvMat* srcPoint,CvMat* dstPoint)
{
    icvMake2DPoints(srcPoint,dstPoint);
    return;
}

/* 
  Convert 2D or 3D points to 2D points

  for 3D: x = x/z;
          y = y/z
  for 2D just copy and maybe convert type

  Source and destiantion may be the same and in this case src must be 2D
*/
static void icvMake2DPoints(CvMat* srcPoint,CvMat* dstPoint)
{
    CvMat* submatx = 0;
    CvMat* submaty = 0;
    CvMat* submatz = 0;

    CV_FUNCNAME( "icvMake2DPoints" );
    __BEGIN__;
    
    if( !CV_IS_MAT(srcPoint) || !CV_IS_MAT(dstPoint) )
    {
        CV_ERROR(CV_StsBadPoint,"Not a matrixes");
    }

    int numPoint;
    numPoint = srcPoint->cols;
        
    if( numPoint != dstPoint->cols )
    {
        CV_ERROR( CV_StsBadSize, "Number of points not equal" );
    }
    if( numPoint < 1 )
    {
        CV_ERROR( CV_StsBadSize, "Number of points must > 0" );
    }

    if( srcPoint->rows > 3 || srcPoint->rows < 2 )
    {
        CV_ERROR( CV_StsBadSize, "Number of coordinates of srcPoint must be 2 or 3" );
    }

    if( dstPoint->rows != 2 )
    {
        CV_ERROR( CV_StsBadSize, "Number of coordinates of dstPoint must be 2" );
    }

    CV_CALL( submatx = cvCreateMat(1,numPoint,CV_64F) );
    CV_CALL( submaty = cvCreateMat(1,numPoint,CV_64F) );
    CV_CALL( submatz = cvCreateMat(1,numPoint,CV_64F) );

    CvMat subwpointsx;
    CvMat subwpointsy;
    
    CvMat tmpSubmatx;
    CvMat tmpSubmaty;
    CvMat tmpSubmatz;
    
    cvGetRow( dstPoint, &subwpointsx, 0 );
    cvGetRow( dstPoint, &subwpointsy, 1 );
    
    cvGetRow( srcPoint, &tmpSubmatx, 0 );
    cvGetRow( srcPoint, &tmpSubmaty, 1 );
    
    cvConvert(&tmpSubmatx,submatx);
    cvConvert(&tmpSubmaty,submaty);

    if( srcPoint->rows == 3 )
    {
        cvGetRow( srcPoint, &tmpSubmatz, 2 );
        cvConvert(&tmpSubmatz,submatz);
        
        cvDiv( submatx, submatz, &subwpointsx);
        cvDiv( submaty, submatz, &subwpointsy);
    }
    else
    {
        cvConvert(submatx,&subwpointsx);
        cvConvert(submaty,&subwpointsy);
    }

    __END__;

    cvReleaseMat(&submatx);
    cvReleaseMat(&submaty);
    cvReleaseMat(&submatz);
}

/*=====================================================================================*/

void cvMake3DPoints(CvMat* srcPoint,CvMat* dstPoint)
{
    icvMake3DPoints(srcPoint,dstPoint);
    return;
}


/* 
  Convert 2D or 3D points to 3D points

  for 2D: x = x;
          y = y;
          z = 1;
          
  for 3D: x = x;
          y = y;
          z = z;
          
  Source and destiantion may be the same and in this case src must be 2D
*/
static void icvMake3DPoints(CvMat* srcPoint,CvMat* dstPoint)
{
    CvMat* tmpSubmatz = 0;

    CV_FUNCNAME( "icvMake3DPoints" );
    __BEGIN__;
    
    if( !CV_IS_MAT(srcPoint) || !CV_IS_MAT(dstPoint) )
    {
        CV_ERROR(CV_StsBadPoint,"Not a matrixes");
    }

    int numPoint;
    numPoint = srcPoint->cols;
        
    if( numPoint != dstPoint->cols )
    {
        CV_ERROR( CV_StsBadSize, "Number of points not equal" );
    }
    if( numPoint < 1 )
    {
        CV_ERROR( CV_StsBadSize, "Number of points must > 0" );
    }

    if( srcPoint->rows > 3 || srcPoint->rows < 2 )
    {
        CV_ERROR( CV_StsBadSize, "Number of coordinates of srcPoint must be 2 or 3" );
    }

    if( dstPoint->rows != 3 )
    {
        CV_ERROR( CV_StsBadSize, "Number of coordinates of dstPoint must be 3" );
    }

    CV_CALL( tmpSubmatz = cvCreateMat(1,numPoint,CV_64F) );
    
    if( srcPoint->rows == 3 )
    {
        /* Just copy all points */
        cvConvert(srcPoint,dstPoint);
    }
    else
    {
        CvMat subwpointsx;
        CvMat subwpointsy;
        CvMat subwpointsz;
        
        cvGetRow( dstPoint, &subwpointsx, 0 );
        cvGetRow( dstPoint, &subwpointsy, 1 );
        cvGetRow( dstPoint, &subwpointsz, 2 );
        
        CvMat tmpSubmatx;
        CvMat tmpSubmaty;
        
        cvGetRow( srcPoint, &tmpSubmatx, 0 );
        cvGetRow( srcPoint, &tmpSubmaty, 1 );

        cvConvert( &tmpSubmatx, &subwpointsx );
        cvConvert( &tmpSubmaty, &subwpointsy );
        
        /* fill z by 1 */
        int i;
        for( i = 0; i < numPoint; i++ )
        {
            cvmSet(&subwpointsz,0,i,1.0);
        }
    }

    __END__;

    cvReleaseMat(&tmpSubmatz);
}

/*=====================================================================================*/
static void cvComputeCorrespondEpilines(CvMat* points,int pointImageID,CvMat* fundMatr,CvMat* corrLines)
{

    CvMat* wpoints = 0;
    CvMat* wcorrLines = 0;

    pointImageID = 3-pointImageID;

    CV_FUNCNAME( "icvComputeCorrespondEpilines" );
    __BEGIN__;
    
    /* Test correct of input data */

    if( !CV_IS_MAT(points) || !CV_IS_MAT(fundMatr)|| !CV_IS_MAT(corrLines))
    {
        CV_ERROR(CV_StsBadPoint,"Not a matrixes");
    }

    /*  */

    int numPoint;
    numPoint = points->cols;
    
    if( numPoint != corrLines->cols )
    {
        CV_ERROR( CV_StsBadSize, "Number of points and lines are not equal" );
    }

    if( numPoint < 1 )
    {
        CV_ERROR( CV_StsBadSize, "Number of points must > 0" );
    }

    if( points->rows != 2 && points->rows != 3 )
    {
        CV_ERROR( CV_StsBadSize, "Number of coordinates of points1 must be 2 or 3" );
    }

    if( corrLines->rows != 3 )
    {
        CV_ERROR( CV_StsBadSize, "Number of coordinates of corrLines must be 3" );
    }

    if( fundMatr->rows != 3 || fundMatr->cols != 3 )
    {
        CV_ERROR( CV_StsBadSize, "Size of fundMatr must be 3x3" );
    }

    double wfundMatr_dat[9];
    CvMat wfundMatr;
    wfundMatr = cvMat(3,3,CV_64F,wfundMatr_dat);
    cvConvert(fundMatr,&wfundMatr);

    if( pointImageID == 1 )
    {// get transformed fundamental matrix
        double tmpMatr_dat[9];
        CvMat tmpMatr;
        tmpMatr = cvMat(3,3,CV_64F,tmpMatr_dat);
        cvConvert(fundMatr,&tmpMatr);
        cvTranspose(&tmpMatr,&wfundMatr);
    }
    else if( pointImageID != 2 )
    {
        CV_ERROR( CV_StsBadArg, "Image ID must be 1 or 2" );
    }
    /* if wfundMatr we have good fundamental matrix */
    /* compute corr epi line for given points */

    CV_CALL( wpoints = cvCreateMat(3,numPoint,CV_64F) );
    CV_CALL( wcorrLines = cvCreateMat(3,numPoint,CV_64F) );


    /* if points has 2 coordinates trandform them to 3D */
    icvMake3DPoints(points,wpoints);

    cvmMul(&wfundMatr,wpoints,wcorrLines);

    /* normalise line coordinates */
    int i;
    for( i = 0; i < numPoint; i++ )
    {
        CvMat line;
        cvGetCol(wcorrLines,&line,i);
        double a,b;
        a = cvmGet(&line,0,0);
        b = cvmGet(&line,1,0);
        double nv;
        nv = sqrt(a*a+b*b);
        cvConvertScale(&line,&line,1.0 / nv);        
    }
    cvConvert(wcorrLines,corrLines);


    __END__;

    cvReleaseMat(&wpoints);
    cvReleaseMat(&wcorrLines);

}

/*=====================================================================================*/

#define SIGN(x) ( (x)<0 ? -1:((x)>0?1:0 ) )
//#define REAL_ZERO(x) ( (x) < EPSILON && (x) > -EPSILON)
#define REAL_ZERO(x) ( (x) < 1e-8 && (x) > -1e-8)

/* function return squares for cubic equation. 6 params - two for each square (Re,Im) */
static int
icvCubicV( double a2, double a1, double a0, double *squares )
{
    double p, q, D, c1, c2, b1, b2, ro1, ro2, fi1, fi2;
    double x[6][3];
    int i, j, t;

    if( !squares )
        return CV_BADFACTOR_ERR;

    if( fabs(a0) > FLT_MAX || fabs(a1) > FLT_MAX || fabs(a2) > FLT_MAX )
    {
        return 0;//Coeffs too big
    }


    p = a1 - a2 * a2 / 3;
    q = (9 * a1 * a2 - 27 * a0 - 2 * a2 * a2 * a2) / 27;
    D = q * q / 4 + p * p * p / 27;

    if( fabs(p) > FLT_MAX || fabs(q) > FLT_MAX || fabs(D) > FLT_MAX )
    {
        return 0;//Coeffs too big
    }
    
    if( D < 0 )
    {

        c1 = q / 2;
        c2 = c1;
        b1 = sqrt( -D );
        b2 = -b1;

        ro1 = sqrt( c1 * c1 - D );
        ro2 = ro1;

        fi1 = atan2( b1, c1 );
        fi2 = -fi1;
    }
    else
    {

        c1 = q / 2 + sqrt( D );
        c2 = q / 2 - sqrt( D );
        b1 = 0;
        b2 = 0;

        ro1 = fabs( c1 );
        ro2 = fabs( c2 );
        fi1 = CV_PI * (1 - SIGN( c1 )) / 2;
        fi2 = CV_PI * (1 - SIGN( c2 )) / 2;
    }                           /* if */

    for( i = 0; i < 6; i++ )
    {

        x[i][0] = -a2 / 3;
        x[i][1] = 0;
        x[i][2] = 0;

        squares[i] = x[i][i % 2];
    }                           /* for */

    if( !REAL_ZERO( ro1 ))
    {
        c1 = SIGN( ro1 ) * pow( fabs( ro1 ), 1. / 3 );
        c1 -= SIGN( ro1 ) * p / 3. * pow( fabs( ro1 ), -1. / 3 );

        c2 = SIGN( ro1 ) * pow( fabs( ro1 ), 1. / 3 );
        c2 += SIGN( ro1 ) * p / 3. * pow( fabs( ro1 ), -1. / 3 );
    }                           /* if */

    if( !REAL_ZERO( ro2 ))
    {
        b1 = SIGN( ro2 ) * pow( fabs( ro2 ), 1. / 3 );
        b1 -= SIGN( ro2 ) * p / 3. * pow( fabs( ro2 ), -1. / 3 );

        b2 = SIGN( ro2 ) * pow( fabs( ro2 ), 1. / 3 );
        b2 += SIGN( ro2 ) * p / 3. * pow( fabs( ro2 ), -1. / 3 );
    }

    for( i = 0; i < 6; i++ )
    {

        if( i < 3 )
        {

            if( !REAL_ZERO( ro1 ))
            {

                x[i][0] = cos( fi1 / 3. + 2 * CV_PI * (i % 3) / 3. ) * c1 - a2 / 3;
                x[i][1] = sin( fi1 / 3. + 2 * CV_PI * (i % 3) / 3. ) * c2;
            }
            else
            {

                //x[i][2] = 1;!!!
            }                   /* if */
        }
        else
        {

            if( !REAL_ZERO( ro2 ))
            {

                x[i][0] = cos( fi2 / 3. + 2 * CV_PI * (i % 3) / 3. ) * b1 - a2 / 3;
                x[i][1] = sin( fi2 / 3. + 2 * CV_PI * (i % 3) / 3. ) * b2;
            }
            else
            {

                //x[i][2] = 1;!!!
            }                   /* if */
        }                       /* if */
    }                           /* for */

    t = 0;

    //int numRoots = 6;
    for( i = 0; i < 6 && t < 6; i++ )
    {

        if( !x[i][2] )
        {

            squares[t++] = x[i][0];
            squares[t++] = x[i][1];
            x[i][2] = 1;

            for( j = i + 1; j < 6; j++ )
            {/* delete equal root from rest */

                if( !x[j][2] && REAL_ZERO( x[i][0] - x[j][0] )
                    && REAL_ZERO( x[i][1] - x[j][1] ))
                {

                    x[j][2] = 1;
                    break;
                }               /* if */
            }                   /* for */
        }                       /* if */
    }                           /* for */
    return 3;
}                               /* icvCubic */

/*=====================================================================================*/

