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

int atsCheckConvexHull( CvPoint* Pts, int psize, int* hull, int hsize, int orient )
{
    int  i;
    int errors = 0;

    CvPoint* hullvect = (CvPoint*)icvAlloc( hsize * sizeof(CvSize) );

    for ( i = 1 ; i < hsize; i++ )
    {
        hullvect[i-1].x = Pts[hull[i]].x - Pts[hull[i-1]].x;
        hullvect[i-1].y = Pts[hull[i]].y - Pts[hull[i-1]].y;    
    }
    hullvect[hsize-1].x = Pts[hull[0]].x - Pts[hull[hsize-1]].x;
    hullvect[hsize-1].y = Pts[hull[0]].y - Pts[hull[hsize-1]].y;    

    /* check two consequtive vectors */
    for ( i = 0 ; i < hsize - 1; i++ )
    {
        int conv = hullvect[i].x * hullvect[i+1].y - hullvect[i].y * hullvect[i+1].x ;
        if (orient == CV_CLOCKWISE) conv = -conv;
        if (conv < 0)
            errors++;
    }
    {
        int conv = hullvect[hsize-1].x * hullvect[0].y - 
                   hullvect[hsize-1].y * hullvect[0].x ;
        if (orient == CV_CLOCKWISE) conv = -conv;
        if (conv < 0)
            errors++; 
    }
 


    for ( i = 0 ; i < psize; i++ )
    {
        int j;
        for( j = 0 ; j < hsize; j++ )
        {                
            int dx = Pts[i].x - Pts[hull[j]].x;
            int dy = Pts[i].y - Pts[hull[j]].y;

            int conv = dy*hullvect[j].x - dx*hullvect[j].y;
            
            /*if ( (i==hull[j])||(i==hull[j+1]) ) continue; */

            if (orient == CV_CLOCKWISE) conv = -conv;

            if ( conv < 0 ) 
                errors++;
        }
    }
    icvFree(&hullvect);
    /*assert( errors == 0); */
    return errors;    
}
int atsCheckConvexHullP( CvPoint* Pts, int psize, CvPoint** hull, int hsize, int orient )
{
    int  i;
    int errors = 0;

    CvPoint* hullvect = (CvPoint*)icvAlloc( hsize * sizeof(CvSize) );

    for ( i = 1 ; i < hsize; i++ )
    {
        hullvect[i-1].x = hull[i]->x - hull[i-1]->x;
        hullvect[i-1].y = hull[i]->y - hull[i-1]->y;    
    }
    hullvect[hsize-1].x = hull[0]->x - hull[hsize-1]->x;
    hullvect[hsize-1].y = hull[0]->y - hull[hsize-1]->y;    

    /* check two consequtive vectors */
    for ( i = 0 ; i < hsize - 1; i++ )
    {
        int conv = hullvect[i].x * hullvect[i+1].y - hullvect[i].y * hullvect[i+1].x ;
        if (orient == CV_CLOCKWISE) conv = -conv;
        if (conv < 0)
            errors++;
    }
    {
        int conv = hullvect[hsize-1].x * hullvect[0].y - 
                   hullvect[hsize-1].y * hullvect[0].x ;
        if (orient == CV_CLOCKWISE) conv = -conv;
        if (conv < 0)
            errors++; 
    }
 


    for ( i = 0 ; i < psize; i++ )
    {
        int j;
        for( j = 0 ; j < hsize; j++ )
        {                
            int dx = Pts[i].x - hull[j]->x;
            int dy = Pts[i].y - hull[j]->y;

            int conv = dy*hullvect[j].x - dx*hullvect[j].y;
            
            /*if ( (i==hull[j])||(i==hull[j+1]) ) continue; */

            if (orient == CV_CLOCKWISE) conv = -conv;

            if ( conv < 0 ) 
                errors++;
        }
    }
    icvFree(&hullvect);
    /*assert( errors == 0); */
    return errors;    
}


/* Testing parameters */
static char test_desc[] = "Convex hull";

static char TestClass[] = "Algorithm";
static char* func_name[4] = 
{
    "cvConvexHullApprox",
    "cvConvexHull",
    "cvContourConvexHullApprox",
    "cvContourConvexHull"
};

#define APPROX 0
#define EXACT  1 

static int lScreenSize;
static long lLoopsProp;
static long lNumPoints;   

static int fmaConvexHull(void* prm)
{ 
    long lErrors = 0;
    
    
    static int  read_param = 0;

    int            i,j;
    
    CvPoint* Pts;
    int* hull;
    int count = 0;

    CvRect rect;
   
    int minx = 1000000, maxx = -10000;
    int miny = 1000000, maxy = -10000;
    long lParam = (long)prm;
    
    if(!read_param)
    {
        read_param=1;
        
        /* Reading test-parameters */
        trslRead( &lNumPoints, "4096", "Maximal number of points" ); 
        trslRead( &lLoopsProp, "100", "Loops" ); 
    }
        
    /* Allocating image */
    Pts = (CvPoint*)icvAlloc( lNumPoints * sizeof(CvPoint) );
    hull = (int*)icvAlloc( lNumPoints * sizeof(int) );
    
    for( j = 0; j < lLoopsProp; j++ )
    {
        int numpts;  
        ats1iInitRandom( 5, lNumPoints, &numpts, 1 );
        
        /* init points */
        ats1iInitRandom( 5, 1024, &lScreenSize, 1 );
        ats1iInitRandom( 0, lScreenSize, (int*)Pts, 2*numpts ) ;
        
        for( i = 0; i < numpts ; i++ )
        {
            minx = MIN(Pts[i].x, minx );
            maxx = MAX(Pts[i].x, maxx );
            miny = MIN(Pts[i].y, miny );
            maxy = MAX(Pts[i].y, maxy );  
        }

        rect.x = minx;
        rect.y = miny;
        rect.width = maxx- minx + 1;
        rect.height = maxy- miny + 1;
    
        switch (lParam)
        {
        case APPROX:
            cvConvexHullApprox( Pts,
                                             numpts,
                                             &rect,
                                             1,
                                             CV_COUNTER_CLOCKWISE,
                                             hull, &count );   
            break;
        case EXACT:
            cvConvexHull( Pts,
                                       numpts,NULL,
                                       CV_COUNTER_CLOCKWISE,
                                       hull, &count );   
            break;

        }/*switch */

        /* check errors */
        lErrors += atsCheckConvexHull( Pts, numpts, hull, count, CV_COUNTER_CLOCKWISE );

    } /* for */

    
   if( lErrors == 0 ) return trsResult( TRS_OK, "No errors fixed for this test" );
   else return trsResult( TRS_FAIL, "Total fixed %d errors", lErrors );

}        

static int fmaConvexHullContour(void* prm)
{ 
    long lErrors = 0;     
    
    static int  read_param = 0;
    int i,j;
    
    CvRect rect;
   
    int minx = 1000000, maxx = -10000;
    int miny = 1000000, maxy = -10000;
    long lParam = (long)prm;

    CvPoint* points;
    CvPoint** pointers;
    
    CvSeqWriter writer;
    CvSeqReader reader;

    CvSeq* contour;
    CvSeq* hull = NULL;
    CvMemStorage* storage;

    if(!read_param)
    {
        read_param=1;
        
        /* Reading test-parameters */
        trslRead( &lNumPoints, "4096", "Maximal number of points" ); 
        trslRead( &lLoopsProp, "100", "Loops" ); 
    }

    storage = cvCreateMemStorage(0);
    cvClearMemStorage( storage );
   
    points =  (CvPoint*)icvAlloc( lNumPoints * sizeof(CvPoint) );
    pointers = (CvPoint**)icvAlloc( lNumPoints * sizeof(CvPoint*) );
    
    for( j = 0; j < lLoopsProp; j++ )
    {
        int numpts;
        /* Allocating points */
                
        cvStartWriteSeq( CV_SEQ_SIMPLE_POLYGON , sizeof(CvSeq),
                                sizeof(CvPoint), storage, &writer );

        ats1iInitRandom( 5, lNumPoints, &numpts, 1 );
        
        /* init points */
        ats1iInitRandom( 5, 1024, &lScreenSize, 1 );
        
        for( i = 0; i < numpts ; i++ )
        {
            CvPoint pt;
            ats1iInitRandom( 0, lScreenSize, (int*)&pt, 2 );
            CV_WRITE_SEQ_ELEM( pt, writer );
        
            minx = MIN(pt.x, minx );
            maxx = MAX(pt.x, maxx );
            miny = MIN(pt.y, miny );
            maxy = MAX(pt.y, maxy );  
        }
        contour = cvEndWriteSeq( &writer );

        rect.x = minx;
        rect.y = miny;
        rect.width = maxx- minx + 1;
        rect.height = maxy- miny + 1;
    
        switch (lParam)
        {
        case APPROX:
            hull = cvContourConvexHullApprox( contour, 1, CV_COUNTER_CLOCKWISE,
                                                    storage );   
            break;
        case EXACT:
            hull = cvContourConvexHull( contour,CV_COUNTER_CLOCKWISE,
                                                    storage );   
            break;
        
        }/*switch */

        /* check errors */
        
        cvStartReadSeq( contour, &reader, 0 );
        for( i = 0; i < contour->total; i++ )
        {      
            CV_READ_SEQ_ELEM( points[i], reader );
        }
        cvStartReadSeq( hull, &reader, 0 );
        for( i = 0; i < hull->total; i++ )
        {      
            CV_READ_SEQ_ELEM( pointers[i], reader );
            
        }

        cvClearMemStorage( storage );

        lErrors += atsCheckConvexHullP( points, contour->total, pointers, hull->total,
                                       CV_COUNTER_CLOCKWISE );
        
        
    } /* for */

    icvFree(&points);
    icvFree(&pointers);
    cvReleaseMemStorage(&storage);
    
   if( lErrors == 0 ) return trsResult( TRS_OK, "No errors fixed for this test" );
   else return trsResult( TRS_FAIL, "Total fixed %d errors", lErrors );

}        

void InitAConvexHull(void)
{
    /* Register test function */    
    trsRegArg( func_name[0], test_desc, atsAlgoClass, fmaConvexHull, APPROX );
    trsRegArg( func_name[1], test_desc, atsAlgoClass, fmaConvexHull, EXACT );
    trsRegArg( func_name[2], test_desc, atsAlgoClass, fmaConvexHullContour, APPROX );
    trsRegArg( func_name[3], test_desc, atsAlgoClass, fmaConvexHullContour, EXACT );
    

} /* InitAConvexHull */

