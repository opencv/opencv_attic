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

#include "cvtest.h"

/* Testing parameters */
static char TestName[] = "State estimation of linear system by means of Kalman Filtering";
static char TestClass[] = "Algorithm";
static int  Dim;
static int  Steps;

static int  read_param = 0;
static int  data_types = 0;
static double EPSILON = 1.000;

static int fcaKalman( void )
{
    AtsRandState noisegen; 
	AtsRandState dynam;
    double Error = 0;
	CvKalman* Kalm;
			
    /* Initialization global parameters */
    if( !read_param )
    {
        read_param = 1;
        /* Reading test-parameters */
        trsiRead( &Dim,"7","Dimension of dynamical system");
		trsiRead( &Steps,"100","Length of trajectory to track");
    }
	CvMat Sample = cvMat(Dim,1,CV_MAT32F,NULL);
	CvMat Temp = cvMat(Dim,1,CV_MAT32F,NULL);
	
	cvmAlloc(&Sample);
	cvmAlloc(&Temp);
	Kalm = cvCreateKalman(Dim, Dim);
	CvMat Dyn = cvMat(Dim,Dim,CV_MAT32F,Kalm->DynamMatr);
	CvMat Mes = cvMat(Dim,Dim,CV_MAT32F,Kalm->MeasurementMatr);
	CvMat PNC = cvMat(Dim,Dim,CV_MAT32F,Kalm->PNCovariance);
	CvMat MNC = cvMat(Dim,Dim,CV_MAT32F,Kalm->MNCovariance);
	CvMat PriErr = cvMat(Dim,Dim,CV_MAT32F,Kalm->PriorErrorCovariance);
	CvMat PostErr = cvMat(Dim,Dim,CV_MAT32F,Kalm->PosterErrorCovariance);
	CvMat PriState = cvMat(Dim,1,CV_MAT32F,Kalm->PriorState);
	CvMat PostState = cvMat(Dim,1,CV_MAT32F,Kalm->PosterState);
	cvmSetIdentity(&PNC);
	cvmSetIdentity(&PriErr);
	cvmSetIdentity(&PostErr);
	cvmSetZero(&MNC);
	cvmSetZero(&PriState);
	cvmSetZero(&PostState);
    cvmSetIdentity(&Mes);
	cvmSetIdentity(&Dyn);
	atsRandInit(&dynam,-1.0, 1.0, 1);
	atsRandInit(&noisegen,-0.1, 0.1, 2);
	//atsbRand32f(&dynam,Dyn.data.fl,Dim*Dim);
	atsbRand32f(&dynam,Sample.data.fl,Dim);
	cvKalmanUpdateByMeasurement(Kalm, &Sample);
	for(int i = 0; i<Steps; i++)
	{
		cvKalmanUpdateByTime(Kalm);
        int j;
		for(j = 0; j<Dim; j++)
		{
			float t = 0;
			for(int k=0; k<Dim; k++)
			{
				t += Dyn.data.fl[j*Dim+k]*Sample.data.fl[k];
			}
			Temp.data.fl[j]= t+atsRand32f(&noisegen);
		}
		for(j = 0; j<Dim; j++)
		{
			Sample.data.fl[j] = Temp.data.fl[j];
		}
		cvKalmanUpdateByMeasurement(Kalm,&Temp);
	}
	Error = atsCompSinglePrec(Sample.data.fl,Kalm->PriorState,Dim,EPSILON);
	cvmFree(&Sample);
	cvmFree(&Temp);
	cvReleaseKalman(&Kalm);
	if(Error>=EPSILON)return TRS_FAIL;
    return TRS_OK;
} /* fcaSobel8uC1R */


void InitAKalman(void)
{
    trsReg( "Kalman Filtering", TestName, TestClass, fcaKalman);
 
} /* InitASobel */

/* End of file. */
