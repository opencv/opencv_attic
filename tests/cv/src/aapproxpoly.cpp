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
#include <limits.h>
extern Contour Contours[];

static char cTestName[]  = "Contours Approximation";
static char cTestClass[] = "Algorithm";
static char cFuncName[]  = "cvApproxPoly";

bool Cmp( CvPoint Pt1, CvPoint Pt2 )
{
    if( ( Pt1.x == Pt2.x ) && ( Pt1.y == Pt2.y ) )
    {
        return true;
    }

    return false;
}


/*
bool GetContour(Contour Cont, CvSeq** Seq, int* d , CvMemStorage* storage )
{
    int Max_x, Max_y, Min_x, Min_y;
    int t1,t2;
    CvSeqReader  Reader;
    CvPoint Pt;

    if( ( Cont == NULL ) || ( Seq == NULL ) || ( d == NULL ) )
    {
        return false;
    }


    *Seq = NULL;
    *Seq = Cont( storage );
    if( *Seq == NULL ) 
    {
        return false;
    }

    Max_y = Max_x = INT_MIN;
    Min_y = Min_x = INT_MAX;
    cvStartReadSeq( *Seq, &Reader, 0 );
    for( int i = 0 ; i < (*Seq)->total ; i++ )
    {
        CV_READ_SEQ_ELEM( Pt, Reader );

        if( Pt.x > Max_x ) Max_x = Pt.x;
        else if ( Pt.x < Min_x ) Min_x = Pt.x;

        if( Pt.y > Max_y ) Max_y = Pt.y;
        else if ( Pt.y < Min_y ) Min_y = Pt.y;
    }

    t1 = Max_x - Min_x;
    t2 = Max_y - Min_y;
    *d = ( t1*t1 + t2*t2 );
    return true;
}
*/


bool GetContour( int /*type*/, CvSeq** Seq, int* d,
                 CvMemStorage* storage, AtsRandState* rng_state )
{
    int max_x = INT_MIN, max_y = INT_MIN, min_x = INT_MAX, min_y = INT_MAX;
    int i;
    CvSeq* seq;
    int total = atsRandPlain32s( rng_state ) % 1000 + 1;
    CvPoint center;
    int radius, angle;
    double deg_to_rad = CV_PI/180.;
    CvPoint pt;

    center.x = atsRandPlain32s( rng_state ) % 1000;
    center.y = atsRandPlain32s( rng_state ) % 1000;
    radius = atsRandPlain32s( rng_state ) % 1000;
    angle = atsRandPlain32s( rng_state ) % 360;

    seq = cvCreateSeq( CV_SEQ_POLYGON, sizeof(CvContour), sizeof(CvPoint), storage );

    for( i = 0; i < total; i++ )
    {
        int d_radius = atsRandPlain32s( rng_state ) % 10 - 5;
        int d_angle = atsRandPlain32s( rng_state ) % 10 - 5;
        pt.x = cvRound( center.x + radius*cos(angle*deg_to_rad));
        pt.y = cvRound( center.x - radius*sin(angle*deg_to_rad));
        radius += d_radius;
        angle += d_angle;
        cvSeqPush( seq, &pt );

        max_x = MAX( max_x, pt.x );
        max_y = MAX( max_y, pt.y );

        min_x = min( min_x, pt.x );
        min_y = min( min_y, pt.y );
    }

    *d = (max_x - min_x)*(max_x - min_x) + (max_y - min_y)*(max_y - min_y);
    *Seq = seq;
    return true;
}


int CheckSlice( CvPoint StartPt, CvPoint EndPt, CvSeqReader* SrcReader, float Eps ,int* j , int Count)
{
    ///////////
    CvPoint Pt;
    ///////////
    bool flag;
    double dy,dx;
    double A,B,C;
    double Sq;
    double sin_a = 0;
    double cos_a = 0;
    double d     = 0;
    double dist;    
    ///////////
    int TotallErrors = 0;

    ////////////////////////////////
    if( ( SrcReader == NULL ) || ( j == NULL ) )
    {
        assert( false );
        return 0;
    }

    ///////// init line ////////////
    flag = true;

    dx = (double)StartPt.x - (double)EndPt.x;
    dy = (double)StartPt.y - (double)EndPt.y;
    
    if( ( dx == 0 ) && ( dy == 0 ) ) flag = false;
    else
    {
        A = -dy;
        B = dx;
        C = dy * (double)StartPt.x - dx * (double)StartPt.y;
        Sq = sqrt( A*A + B*B );

        sin_a = B/Sq;
        cos_a = A/Sq;
        d     = C/Sq;
    }


    /////// find start point and check distance ////////
    for( ; (*j) < Count  ;  )
    {
        CV_READ_SEQ_ELEM( Pt, *SrcReader );
        (*j)++;
        if( Cmp( StartPt, Pt ) ) break;
        else
        {
            if( flag ) dist = sin_a * Pt.y + cos_a * Pt.x - d;
            else dist = sqrt( (EndPt.y - Pt.y)*(EndPt.y - Pt.y) + (EndPt.x - Pt.x)*(EndPt.x - Pt.x) );

            if( dist > Eps ) TotallErrors++;

        }
    } // for( int j = 0; ( j < SrcSeq->total ) && flag ; i++ )

    return TotallErrors;
}


int Check ( CvSeq* SrcSeq, CvSeq* DstSeq , float Eps )
{
    //////////
    CvSeqReader  DstReader;
    CvSeqReader  SrcReader;
    CvPoint StartPt, EndPt;
    ///////////
    int TotallErrors = 0;
    ///////////
    int Count;
    int i,j;




    if( ( SrcSeq == NULL ) || ( DstSeq == NULL ) )
    {
        assert( false );
        return 0;
    } // if( ( ScrSeq == NULL ) || ( DsrSeq == NULL ) )


    ////////// init ////////////////////
    Count = SrcSeq->total;

    cvStartReadSeq( DstSeq, &DstReader, 0 );
    cvStartReadSeq( SrcSeq, &SrcReader, 0 );

    CV_READ_SEQ_ELEM( StartPt, DstReader );
    for( i = 0 ; i < Count ;  )
    {
        CV_READ_SEQ_ELEM( EndPt, SrcReader );
        i++;
        if( Cmp( StartPt, EndPt ) ) break;
    }

    ///////// start ////////////////
    for( i = 1 , j = 0 ; i <= DstSeq->total ;  )
    {
        ///////// read slice ////////////
        EndPt.x = StartPt.x;
        EndPt.y = StartPt.y;
        CV_READ_SEQ_ELEM( StartPt, DstReader );
        i++;

        TotallErrors += CheckSlice( StartPt, EndPt, &SrcReader, Eps, &j, Count );

        if( j > Count ) 
        {
            TotallErrors++;
            return TotallErrors;
        } //if( !flag ) 

    } // for( int i = 0 ; i < DstSeq->total ; i++ )

    return 0;
}


static int aApproxPoly( void )
{

    ////////////// Variables ////////////////
    int IntervalsCount = 10;
    ///////////
    Contour Cont;
    CvSeq*  SrcSeq = NULL;
    CvSeq*  DstSeq;
    ///////////
    int     iDiam;
    float   dDiam, Eps, EpsStep;
    ///////////
    int Ret;
    int TotallErrors;
    ///////////

    unsigned seed = atsGetSeed();
    AtsRandState rng_state;
    

    atsRandInit( &rng_state, 0, 1, seed );

    TotallErrors = 0;
    for( int i = 0; NULL != ( Cont = Contours[i] ) ; i++ )
    {
		CvMemStorage* storage = 0;
        CvMemStoragePos pos;



        ///////////////////// init contour /////////
        dDiam = 0;
        while ( ( sqrt( dDiam ) / IntervalsCount ) == 0 )
        {

			if( storage != 0 ) 
			{
				cvReleaseMemStorage(&storage);				
			}

			storage = cvCreateMemStorage( 0 );

            if ( GetContour( 0, &SrcSeq, &iDiam, storage, &rng_state ) )
                dDiam = (float)iDiam;

        }
        dDiam = (float)sqrt( dDiam );

        storage = SrcSeq->storage;

        ////////////////// test /////////////
        EpsStep = dDiam / IntervalsCount ;
        for( Eps = EpsStep ; Eps < dDiam ; Eps += EpsStep )
        {
            cvSaveMemStoragePos( storage, &pos ); 


            
            //trsWrite( ATS_LST | ATS_CON, "\nContour %d accuracy %f : ", i , Eps );
            ////////// call function ////////////
            //trsWrite( ATS_LST | ATS_CON, "(((");
            DstSeq = cvApproxPoly( SrcSeq, SrcSeq->header_size, storage, 
                                   CV_POLY_APPROX_DP, Eps );
            //trsWrite( ATS_LST | ATS_CON, ")))");

            if( DstSeq == NULL ) 
            {
                trsWrite( ATS_LST | ATS_CON, "\tfunction return NULL ");
                TotallErrors++;
            } // if( DstSeq == NULL )
            if( DstSeq == NULL ) continue;

            //trsWrite( ATS_LST | ATS_CON, "{{{");
            Ret = Check( SrcSeq, DstSeq, Eps );
            //trsWrite( ATS_LST | ATS_CON, "}}}");
            if( Ret != 0 )
            {
                 trsWrite( ATS_LST | ATS_CON, "\tIncorrect result");
                 TotallErrors += Ret;
            } // if( Ret != 0 )
     
            cvRestoreMemStoragePos( storage, &pos );

        } // for( Eps = EpsStep ; Eps <= Diam ; Eps += EpsStep )

        ///////////// free memory  ///////////////////
       
		cvReleaseMemStorage(&storage);
    } // for( int i = 0; NULL != ( Cont = Contours[i] ) ; i++ )


    if( TotallErrors == 0 )   return TRS_OK;
    else
    {
        trsWrite( ATS_LST | ATS_CON, "\nTotall errors = %d ", TotallErrors);
        return TRS_FAIL;
    }
}

void InitAApproxPoly( void )
{
    trsReg(cFuncName,cTestName,cTestClass,aApproxPoly); 
    
} /* void InitAAproxPoly( void ) */
