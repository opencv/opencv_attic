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
#include "_cvwrap.h"

static CvStatus  icvMulTransposed_32f( CvMatr32f srcMatr1,
                                       int matrWidth1,
                                       int matrHeight1,
                                       CvMatr32f srcMatr2,
                                       int matrWidth2,
                                       int matrHeight2, CvMatr32f destMatr )
{
    int i, j;

    for( i = 0; i < matrHeight1; i++ )
    {
        for( j = 0; j < matrHeight2; j++ )
        {
            destMatr[i * matrHeight2 + j] = (float)
                icvDotProduct_32f( srcMatr1 + i * matrWidth1,
                                srcMatr2 + j * matrWidth2,
                                matrWidth1 );
        }
    }
    return CV_NO_ERR;
}

/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:    cvCreateKalman
//    Purpose: Creating CvKalman structure and allocating memory for it
//    Context:
//    Parameters:
//      Kalman        - double pointer to CvKalman structure
//      DynParams     - dimension of the dynamical vector
//      MeasureParams - dimension of the measurement vector
//    Returns:
//    Notes:
//      
//F*/
CV_IMPL CvKalman*
cvCreateKalman( int DynamParams, int MeasureParams )
{
    CvKalman *Kalm = 0;

    CV_FUNCNAME( "cvCreateKalman" );
    __BEGIN__;
    
    if( DynamParams <= 0 || MeasureParams <= 0 )
        CV_ERROR( CV_StsOutOfRange, "" );
    
    /* allocating memory for the structure */
    CV_CALL( Kalm = (CvKalman *) cvAlloc( sizeof( CvKalman )));
    /* Setting struture fields */
    Kalm->DP = DynamParams;
    Kalm->MP = MeasureParams;
    /* allocating memory for the structure fields */
    CV_CALL( Kalm->DynamMatr = (float *) cvAlloc( sizeof( float ) * DynamParams * DynamParams ));
    CV_CALL( Kalm->MeasurementMatr =
        (float *) cvAlloc( sizeof( float ) * MeasureParams * DynamParams ));
    CV_CALL( Kalm->MNCovariance = (float *) cvAlloc( sizeof( float ) * MeasureParams * DynamParams ));
    CV_CALL( Kalm->PNCovariance = (float *) cvAlloc( sizeof( float ) * DynamParams * DynamParams ));
    CV_CALL( Kalm->PriorState = (float *) cvAlloc( sizeof( float ) * DynamParams ));
    CV_CALL( Kalm->PosterState = (float *) cvAlloc( sizeof( float ) * DynamParams ));
    CV_CALL( Kalm->PosterErrorCovariance =
        (float *) cvAlloc( sizeof( float ) * DynamParams * DynamParams ));
    CV_CALL( Kalm->PriorErrorCovariance =
        (float *) cvAlloc( sizeof( float ) * DynamParams * DynamParams ));
    CV_CALL( Kalm->KalmGainMatr = (float *) cvAlloc( sizeof( float ) * DynamParams * MeasureParams ));
    CV_CALL( Kalm->Temp1 = (float *) cvAlloc( sizeof( float ) * DynamParams * DynamParams ));
    CV_CALL( Kalm->Temp2 = (float *) cvAlloc( sizeof( float ) * DynamParams * DynamParams ));

    /* Returning created structure */
    __END__;

    return Kalm;
}

/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:    cvReleaseKalman
//    Purpose: Releases CvKalman structure and frees memory allocated for it
//    Context:
//    Parameters:
//      Kalman        - double pointer to CvKalman structure
//    Returns:
//    Notes:
//      
//F*/
CV_IMPL void
cvReleaseKalman( CvKalman ** Kalman )
{
    CvKalman *Kalm;

    CV_FUNCNAME( "cvReleaseKalman" );
    __BEGIN__;
    
    if( !Kalman )
        CV_ERROR( CV_StsNullPtr, "" );
    
    Kalm = *Kalman;
    
    /* freeing the memory */
    cvFree( (void**)&Kalm->PriorState );
    cvFree( (void**)&Kalm->PosterState );
    cvFree( (void**)&Kalm->DynamMatr );
    cvFree( (void**)&Kalm->MeasurementMatr );
    cvFree( (void**)&Kalm->PNCovariance );
    cvFree( (void**)&Kalm->MNCovariance );
    cvFree( (void**)&Kalm->PosterErrorCovariance );
    cvFree( (void**)&Kalm->PriorErrorCovariance );
    cvFree( (void**)&Kalm->KalmGainMatr );
    cvFree( (void**)&Kalm->Temp1 );
    cvFree( (void**)&Kalm->Temp2 );

    /* deallocating the structure */
    cvFree( (void**)Kalman );
    Kalman = NULL;
    __END__;
}

/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:    cvKalmanUpdateByTime
//    Purpose: Performing Time update routine for Kalman Filter
//    Context:
//    Parameters:
//      Kalman  - pointer to CvKalman structure
//    Returns:
//    Notes:
//      
//F*/

CV_IMPL void
cvKalmanUpdateByTime( CvKalman * Kalman )
{
    CV_FUNCNAME( "cvCreateKalman" );
    __BEGIN__;
    
    if( !Kalman )
        CV_ERROR( CV_StsNullPtr, "" );
    
    /*Updating the state */
    icvTransformVector_32f( Kalman->DynamMatr,
                             Kalman->PosterState, Kalman->PriorState, Kalman->DP, Kalman->DP );

    /*Updating the Error Covariances */
    icvMulTransposed_32f( Kalman->PosterErrorCovariance,
                          Kalman->DP,
                          Kalman->DP,
                          Kalman->DynamMatr, Kalman->DP, Kalman->DP, Kalman->Temp1 );
    icvMulMatrix_32f( Kalman->DynamMatr,
                      Kalman->DP,
                      Kalman->DP,
                      Kalman->Temp1, Kalman->DP, Kalman->DP, Kalman->PriorErrorCovariance );
    icvAddMatrix_32f( Kalman->PriorErrorCovariance,
                      Kalman->PNCovariance,
                      Kalman->PriorErrorCovariance, Kalman->DP, Kalman->DP );
    __END__;
}

/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:    cvKalmanUpdateByTMeasurement
//    Purpose: Performing Measurement update routine for Kalman Filter
//    Context:
//    Parameters:
//    Kalman  - pointer to CvKalman structure
//    Measurement - Current Measurement vector 
//    Returns:
//    Notes:
//      
//F*/
CV_IMPL void
cvKalmanUpdateByMeasurement( CvKalman * Kalman, CvMat * Measurement )
{
    float *Measure;

    CV_FUNCNAME( "cvCreateKalman" );
    __BEGIN__;
    
    if( !Kalman || !Measurement )
        CV_ERROR( CV_StsNullPtr, "" );

    if( CV_ARR_TYPE( Measurement->type ) != CV_32FC1 )
        CV_ERROR( CV_StsBadArg, "source  has not appropriate format" );

    if( (Measurement->cols != 1) || (Measurement->rows != Kalman->MP) )
        CV_ERROR( CV_StsBadArg, "source  has not appropriate size" );

    Measure = Measurement->data.fl;

    /* Calculating Kalman Gain Matrix */
    icvMulTransposed_32f( Kalman->PriorErrorCovariance,
                          Kalman->DP,
                          Kalman->DP,
                          Kalman->MeasurementMatr,
                          Kalman->DP,
                          Kalman->MP,
                          Kalman->Temp1 );

    icvMulMatrix_32f( Kalman->MeasurementMatr,
                      Kalman->DP,
                      Kalman->MP,
                      Kalman->Temp1, Kalman->MP, Kalman->DP, Kalman->KalmGainMatr );

    icvAddMatrix_32f( Kalman->KalmGainMatr,
                      Kalman->MNCovariance, Kalman->KalmGainMatr, Kalman->MP, Kalman->MP );

    icvInvertMatrix_32f( Kalman->KalmGainMatr, Kalman->MP, Kalman->Temp2 );

    icvMulMatrix_32f( Kalman->Temp1,
                      Kalman->MP,
                      Kalman->DP,
                      Kalman->Temp2, Kalman->MP, Kalman->MP, Kalman->KalmGainMatr );

    /* Update the estimation */
    icvTransformVector_32f( Kalman->MeasurementMatr,
                             Kalman->PriorState, Kalman->Temp1, Kalman->DP, Kalman->MP );
    icvSubVector_32f( Measure, Kalman->Temp1, Kalman->Temp2, Kalman->MP );
    icvTransformVector_32f( Kalman->KalmGainMatr,
                             Kalman->Temp2, Kalman->Temp1, Kalman->MP, Kalman->DP );
    icvAddVector_32f( Kalman->PriorState, Kalman->Temp1, Kalman->PosterState, Kalman->DP );

    /* Update the Error Covariance */
    icvMulMatrix_32f( Kalman->KalmGainMatr,
                      Kalman->MP,
                      Kalman->DP,
                      Kalman->MeasurementMatr,
                      Kalman->DP, Kalman->MP, Kalman->PosterErrorCovariance );
    icvSetIdentity_32f( Kalman->Temp1, Kalman->DP, Kalman->DP );
    icvSubMatrix_32f( Kalman->Temp1,
                      Kalman->PosterErrorCovariance, Kalman->Temp1, Kalman->DP, Kalman->DP );
    icvMulMatrix_32f( Kalman->Temp1,
                      Kalman->DP,
                      Kalman->DP,
                      Kalman->PriorErrorCovariance,
                      Kalman->DP, Kalman->DP, Kalman->PosterErrorCovariance );
    __END__;
}
