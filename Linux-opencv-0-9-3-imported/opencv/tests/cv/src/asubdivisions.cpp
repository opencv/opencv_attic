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
#include <float.h>

#define DRAW_CONTOURS 0

#if DRAW_CONTOURS
    extern "C" void WINAPI Sleep(unsigned long);
#endif


static char* funcName[] = 
{
    "cvCreateSubdiv2D, cvSubdiv2DLocate, cvInitSubdivDelaunay2D, "
    "cvSubdivDelaunay2DInsert, cvCalcSubdivVoronoi2D" 
};

static char* testName[] = 
{
    "Calculating Delaunay/Voronoi"
};


static int img_size;
static int max_points;
static int base_iters;
static int init_subdiv_params = 0;


void read_subdiv_params( void )
{
    if( !init_subdiv_params )
    {
        trsiRead( &img_size, "307", "image width" );
        trsiRead( &max_points, "1000", "Maximum number of points in subdivision" );
        trsiRead( &base_iters, "100", "Number of iterations" );

        init_subdiv_params = 1;
    }
}


static int subdiv_test( void )
{
    int     seed = atsGetSeed();

    /* test parameters */
    int     i = 0, j = 0, k;
    int     code = TRS_OK;

    AtsRandState rng_state;
    CvMemStorage* storage;
    CvSize size;

    atsRandInit( &rng_state, 0, 1, seed );

    read_subdiv_params();

    size = cvSize( img_size, img_size*3/4 );

    storage = cvCreateMemStorage(0);

    for( i = 0; i < base_iters; i++ )
    {
        int count = atsRandPlain32s( &rng_state ) % max_points + 1;
        int real_count = count;

        CvSubdiv2D* subdiv = icvCreateSubdivDelaunay2D(
            cvRect( 0, 0, size.width, size.height ), storage );
        CvSeq* seq = cvCreateSeq( 0, sizeof(*seq), sizeof(CvPoint2D32f), storage );

        CvSeqWriter writer;

        cvStartAppendToSeq( seq, &writer );

        // insert random points
        for( j = 0; j < count; j++ )
        {
            CvPoint2D32f pt = atsRandPoint2D32f( &rng_state, size );
            CvSubdiv2DPoint* point;

            CvSubdiv2DPointLocation loc = 
                cvSubdiv2DLocate( subdiv, pt, 0, &point );

            if( loc == CV_PTLOC_VERTEX )
            {
                int index = cvSeqElemIdx( (CvSeq*)subdiv, point );
                CvPoint2D32f* pt1;

                cvFlushSeqWriter( &writer );

                pt1 = (CvPoint2D32f*)cvGetSeqElem( seq, index - 3 );

                if( !pt1 ||
                    fabs(pt1->x - pt.x) > FLT_EPSILON ||
                    fabs(pt1->y - pt.y) > FLT_EPSILON )
                {
                    code = TRS_FAIL;
                    goto test_exit;
                }

                real_count--;
            }

            point = cvSubdivDelaunay2DInsert( subdiv, pt );
            if( point->pt.x != pt.x || point->pt.y != pt.y )
            {
                code = TRS_FAIL;
                goto test_exit;
            }

            if( (j + 1) % 10 == 0 || j == count - 1 )
            {
                if( !icvSubdiv2DCheck( subdiv ))
                {
                    code = TRS_FAIL;
                    goto test_exit;
                }
            }
            
            if( loc != CV_PTLOC_VERTEX )
            {
                CV_WRITE_SEQ_ELEM( pt, writer );
            }
        }

        if( real_count != subdiv->total - 3 )
        {
            code = TRS_FAIL;
            goto test_exit;
        }

        cvCalcSubdivVoronoi2D( subdiv );
        seq = cvEndWriteSeq( &writer );

        if( !icvSubdiv2DCheck( subdiv ))
        {
            trsWrite( ATS_LST|ATS_CON, "subdiv is bad\n" );
            code = TRS_FAIL;
            goto test_exit;
        }

        for( j = 0; j < (count - 5)/10 + 5; j++ )
        {
            CvPoint2D32f pt = atsRandPoint2D32f( &rng_state, size );
            double minDistance;

            /*if( i == 30 && j == 13 )
            {
                putchar('.');
                printf("(x,y) = (%f, %f)\n", pt.x, pt.y );
                printf("rect = (xl,yl,xh,yh) = (%f,%f,%f,%f)\n",
                       subdiv->topleft.x, subdiv->topleft.y,
                       subdiv->bottomright.x, subdiv->bottomright.y );
            }*/

            CvSubdiv2DPoint* point = icvFindNearestPoint2D( subdiv, pt );
            CvSeqReader reader;

            if( !point )
            {
                trsWrite( ATS_LST|ATS_CON, "No nearest Point for #%d\n", j );
                code = TRS_FAIL;
                goto test_exit;
            }

            cvStartReadSeq( seq, &reader );

            minDistance = icvSqDist2D32f( pt, point->pt );

            for( k = 0; k < seq->total; k++ )
            {
                CvPoint2D32f ptt;

                CV_READ_SEQ_ELEM( ptt, reader );

                double distance = icvSqDist2D32f( ptt, pt );
                if( minDistance > distance )
                {
                    trsWrite( ATS_LST|ATS_CON, "%f, %f\n", minDistance, distance );
                    /*CvSubdiv2DPoint* temp_point = (CvSubdiv2DPoint*)cvGetSeqElem(
                                                     (CvSeq*)subdiv, k + 3 );*/
                    code = TRS_FAIL;
                    goto test_exit;
                }
            }
        }

        cvClearMemStorage( storage );
    }

test_exit:

    cvReleaseMemStorage( &storage );

    if( code == TRS_OK )
    {
        return trsResult( TRS_OK, "No errors" );
    }
    else
    {
        trsWrite( ATS_LST, "Fatal error at iter = %d, seed = %08x", i, seed );
        return trsResult( TRS_FAIL, "Test failed" );
    }
}


void InitASubdiv()
{
    trsReg( funcName[0], testName[0], atsAlgoClass, subdiv_test );
}

/* End of file. */

