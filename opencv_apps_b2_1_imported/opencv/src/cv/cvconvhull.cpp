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
#include <limits.h>
#include "_cvutils.h"

/* operator >  for upper half,  <  for lower half */
#define IPCV_IMPLEMENT_SKLANSKY( func_name, _op_, _op2_ )                   \
int func_name(CvPoint* points, int* index, int start, int end, int* stack )\
{                                                                         \
    int pprev, pcur, pnext;                                               \
                                                                          \
    int stacksize = 3;                                                    \
                                                                          \
    int incr;                                                             \
                                                                          \
    if ( end > start )                                                    \
    {                                                                     \
        incr = 1;                                                         \
    }                                                                     \
    else incr = -1;                                                       \
                                                                          \
    if( start==end ||                                                     \
        ( points[index[start]].x == points[index[end]].x &&               \
          points[index[start]].y == points[index[end]].y ) )              \
        { stack[0] = start; return 1; }                                   \
                                                                          \
    /* prepare first triangle */                                          \
    pprev = start;                                                        \
    pcur = start+incr;                                                    \
    pnext = pcur+incr;                                                    \
                                                                          \
    stack[0] = pprev;                                                     \
    stack[1] = pcur;                                                      \
    stack[2] = pnext;                                                     \
                                                                          \
    while( pnext != end+incr )                                            \
    {                                                                     \
        /* check the angle p1,p2,p3 */                                    \
        int cury = points[index[pcur]].y;                                 \
        int nexty = points[index[pnext]].y;                               \
                                                                           \
        if (nexty _op2_ cury )                                              \
        {                                                                    \
            int ax = points[index[pcur]].x - points[index[pprev]].x;          \
            int bx = points[index[pnext]].x - points[index[pcur]].x;          \
                                                                              \
            int ay = cury - points[index[pprev]].y;                           \
            int by = nexty - points[index[pcur]].y;                           \
                                                                              \
            int convexity =  ay*bx - ax*by;   /* if >0 then convex angle */   \
                                                                              \
            if (convexity _op_ 0)                                             \
            {                                                                 \
                pprev = pcur;                                                 \
                pcur = pnext;                                                 \
                pnext+=incr;                                                  \
                stack[stacksize] = pnext;                                     \
                stacksize++;                                                  \
            }                                                                 \
            else                                                              \
            {                                                                 \
                if (pprev == start)                                           \
                {                                                             \
                    pcur = pnext;                                             \
                    stack[1] = pcur;                                          \
                    pnext+=incr;                                              \
                    stack[2] = pnext;                                         \
                }                                                             \
                else                                                          \
                {                                                             \
                    stack[stacksize-2] = pnext;                               \
                    pcur = pprev;                                             \
                    stacksize--;                                              \
                    pprev = stack[stacksize-3];                               \
                }                                                             \
            }                                                                 \
        }                                                                     \
        else                                                                 \
        {                                                                   \
            pnext+=incr;                                                   \
            stack[stacksize-1] = pnext;                                    \
        }                                                                  \
    }                                                                      \
    stacksize--;                                                           \
    return stacksize;                                                      \
}

IPCV_IMPLEMENT_SKLANSKY( icvSklanskyTL, >, >= );
IPCV_IMPLEMENT_SKLANSKY( icvSklanskyTR, <, >= );
IPCV_IMPLEMENT_SKLANSKY( icvSklanskyBL, <, <= );
IPCV_IMPLEMENT_SKLANSKY( icvSklanskyBR, >, <= );

/* algorithm for convex hull approximation. */
/* can give true hull                       */
 CvStatus
 icvConvexHull_Approx( CvPoint * points,
                       int num_points,
                       CvRect * bound_rect,
                       int bandwidth, int orientation, int *hullpoints, int *hullsize )
{
    /* bounds */
    int minx;
    int screenwidth;

    int maxy_ind = 0;
    int miny_ind = 0;

    int *buffer;
    int *uh;                    /* upper half pretenders  */
    int *lh;                    /* lower half pretenderts */

    int *flag;

    /* stack of previous points for Sklansky */
    int *stack;

    int i, hc;

    int numbands;
    int width = 0;

    /* this value store last point of upper half */
    int rest = -1;

    float ibw = 1.f / bandwidth;

    if( bound_rect == NULL )
    {
        int k;
        int maxx;

        maxx = minx = points[0].x;
        /* find minx and maxx */
        for( k = 1; k < num_points; k++ )
        {
            maxx = MAX( points[k].x, maxx );
            minx = MIN( points[k].x, minx );
        }
        screenwidth = maxx - minx + 1;
    }
    else
    {
        minx = bound_rect->x;
        screenwidth = bound_rect->width;
    }

    /* allocate memory for bands */
    numbands = screenwidth / bandwidth;
    numbands += (bandwidth * numbands < screenwidth);

    buffer = (int *) icvAlloc( 2 * numbands * sizeof( int ));

    if( buffer == NULL )
        return CV_OUTOFMEM_ERR;
    flag = (int *) icvAlloc( numbands * sizeof( int ) + 2 );

    if( flag == NULL )
    {
        icvFree( &buffer );
        return CV_OUTOFMEM_ERR;
    }

    uh = buffer;
    lh = uh + numbands;

    stack = flag;

    /* fill arrays */
    for( i = 0; i < numbands; i++ )
    {
        lh[i] = -1;
        uh[i] = -1;
        flag[i] = 0;
    }

    /* step two - finding maximal and minimal coordinate within every band */
    for( i = 0; i < num_points; i++ )
    {
        /* find band */
        int x = cvFloor( (points[i].x - minx) * ibw );
        int y = points[i].y;

        if( flag[x] == 0 )
        {
            width++;
            flag[x] = 1;
            lh[x] = uh[x] = i;
        }
        else
        {
            if( points[lh[x]].y >= y )
                lh[x] = i;
            if( points[uh[x]].y <= y )
                uh[x] = i;
        }
    }
    /* copy info from characteristic vector to solid array */
    /* with finding splitter */
    {
        int j = 0;

        for( i = 0; i < numbands; i++ )
        {
            if( flag[i] )
            {
                uh[j] = uh[i];
                if( points[uh[i]].y >= points[uh[maxy_ind]].y )
                    maxy_ind = j;

                lh[j] = lh[i];
                if( points[lh[i]].y <= points[lh[miny_ind]].y )
                    miny_ind = j;
                j++;
            }
        }
    }

    /*upper half */
    {
        int tl_count = icvSklanskyTL( points, uh, 0, maxy_ind, stack );
        int *tr_stack = stack + tl_count;
        int tr_count = icvSklanskyTR( points, uh, width - 1, maxy_ind, tr_stack );

        /* gather upper part of convex hull to output */
        hc = 0;
        if( orientation == CV_COUNTER_CLOCKWISE )
        {
            int i;

            for( i = 0; i < tr_count - 1; i++, hc++ )
            {
                hullpoints[hc] = uh[tr_stack[i]];
            }

            for( i = tl_count - 1; i > 0; i--, hc++ )
            {
                hullpoints[hc] = uh[stack[i]];
            }
            rest = uh[stack[0]];
        }
        else
        {
            int i;

            for( i = 0; i < tl_count - 1; i++, hc++ )
            {
                hullpoints[hc] = uh[stack[i]];
            }

            for( i = tr_count - 1; i > 0; i--, hc++ )
            {
                hullpoints[hc] = uh[tr_stack[i]];
            }
            rest = uh[tr_stack[0]];
        }
    }

    /* lower half */
    {
        int bl_count = icvSklanskyBL( points, lh, 0, miny_ind, stack );
        int *br_stack = stack + bl_count;
        int br_count = icvSklanskyBR( points, lh, width - 1, miny_ind, br_stack );

        if( orientation == CV_COUNTER_CLOCKWISE )
        {
            int i;

            if( lh[0] != uh[0] )
            {
                hullpoints[hc] = rest;
                hc++;
            }

            for( i = 0; i < bl_count - 1; i++, hc++ )
            {
                hullpoints[hc] = lh[stack[i]];
            }

            for( i = br_count - 1; i > 0; i--, hc++ )
            {
                hullpoints[hc] = lh[br_stack[i]];
            }

            if( lh[width - 1] != uh[width - 1] )
            {
                hullpoints[hc] = lh[br_stack[0]];
                hc++;
            }
        }
        else
        {
            int i;

            if( lh[width - 1] != uh[width - 1] )
            {
                hullpoints[hc] = rest;
                hc++;
            }

            for( i = 0; i < br_count - 1; i++, hc++ )
            {
                hullpoints[hc] = lh[br_stack[i]];
            }

            for( i = bl_count - 1; i > 0; i--, hc++ )
            {
                hullpoints[hc] = lh[stack[i]];
            }

            if( lh[0] != uh[0] )
            {
                hullpoints[hc] = lh[stack[0]];
                hc++;
            }
        }
    }

    *hullsize = hc;

    icvFree( &buffer );
    icvFree( &flag );

    return CV_OK;
}

/* _op_ >  for upper half,  <  for lower half */
/* _op2_ >  in ascendance of y ,  <  in descendance of y */

#define IPCV_IMPLEMENT_SKLANSKY2( func_name, _op_, _op2_ )                \
int func_name(CvPoint** points, int start, int end, int* stack )         \
{                                                                         \
    int pprev, pcur, pnext;                                               \
                                                                          \
    int stacksize = 3;                                                    \
                                                                          \
    int incr;                                                             \
                                                                          \
    if ( end > start )                                                    \
    {                                                                     \
        incr = 1;                                                         \
    }                                                                     \
    else incr = -1;                                                       \
                                                                          \
    if( start==end ||                                                     \
        ( points[start]->x == points[end]->x &&                           \
          points[start]->y == points[end]->y ) )                          \
        { stack[0] = start; return 1; }                                   \
                                                                          \
    /* prepare first triangle */                                          \
    pprev = start;                                                        \
    pcur = start+incr;                                                    \
    pnext = pcur+incr;                                                    \
                                                                          \
    stack[0] = pprev;                                                     \
    stack[1] = pcur;                                                      \
    stack[2] = pnext;                                                     \
                                                                          \
    end+=incr;/* make end = afterend */                                   \
                                                                          \
    while( pnext != end )                                                 \
    {                                                                     \
        CvPoint* ptcur = points[pcur];                                   \
        CvPoint* ptnext = points[pnext];                                 \
                                                                          \
        /* check the angle p1,p2,p3 */                                    \
        int cury = ptcur->y;                                              \
        int nexty = ptnext->y;                                            \
                                                                          \
        if (nexty _op2_ cury )                                            \
        {                                                                 \
            CvPoint* ptprev = points[pprev];                             \
                                                                          \
            int ax = ptcur->x - ptprev->x;                                \
            int bx = ptnext->x - ptcur->x;                                \
                                                                          \
            int ay = cury - (ptprev->y);                                  \
            int by = nexty - (ptcur->y);                                  \
                                                                          \
            int convexity =  ay*bx - ax*by;/* if >0 then convex angle */  \
                                                                          \
            if (convexity _op_ 0)                                         \
            {                                                             \
                pprev = pcur;                                             \
                pcur = pnext;                                             \
                pnext+=incr;                                              \
                stack[stacksize] = pnext;                                 \
                stacksize++;                                              \
            }                                                             \
            else                                                          \
            {                                                             \
                if (pprev == start)                                       \
                {                                                         \
                    pcur = pnext;                                         \
                    stack[1] = pcur;                                      \
                    pnext+=incr;                                          \
                    stack[2] = pnext;                                     \
                                                                          \
                }                                                         \
                else                                                      \
                {                                                         \
                    stack[stacksize-2] = pnext;                           \
                    pcur = pprev;                                         \
                    stacksize--;                                          \
                    pprev = stack[stacksize-3];                           \
                }                                                         \
            }                                                             \
        }                                                                 \
        else                                                              \
        {                                                                 \
            pnext+=incr;                                                  \
            stack[stacksize-1] = pnext;                                   \
        }                                                                 \
    }                                                                     \
    stacksize--;                                                          \
    return stacksize;                                                     \
}


#define IPCV_IMPLEMENT_SKLANSKY3( func_name, array_type , _op_ , _op2_ , _access_ ) \
int func_name(array_type* array, int start, int end, int* stack )         \
{                                                                         \
    int pprev, pcur, pnext;                                               \
                                                                          \
    int stacksize = 3;                                                    \
                                                                          \
    int incr;                                                             \
                                                                          \
    if ( end > start )                                                    \
    {                                                                     \
        incr = 1;                                                         \
    }                                                                     \
    else incr = -1;                                                       \
                                                                          \
                                                                          \
    if( start==end ||                                                     \
        ( array[start]_access_ x == array[end]_access_ x &&               \
          array[start]_access_ y == array[end]_access_ y ) )              \
        { stack[0] = start; return 1; }                                   \
                                                                          \
    /* prepare first triangle */                                          \
    pprev = start;                                                        \
    pcur = start+incr;                                                    \
    pnext = pcur+incr;                                                    \
                                                                          \
    stack[0] = pprev;                                                     \
    stack[1] = pcur;                                                      \
    stack[2] = pnext;                                                     \
                                                                          \
    end+=incr;/* make end = afterend */                                   \
                                                                          \
    while( pnext != end )                                                 \
    {                                                                     \
        /* check the angle p1,p2,p3 */                                    \
        int cury = array[pcur]_access_ y;                                 \
        int nexty = array[pnext]_access_ y;                               \
                                                                          \
        if (nexty _op2_ cury )                                            \
        {                                                                 \
            int ax = (array[pcur]_access_ x) - (array[pprev]_access_ x);  \
            int bx = (array[pnext]_access_ x) - (array[pcur]_access_ x);  \
                                                                          \
            int ay = cury - (array[pprev]_access_ y);                     \
            int by = nexty - cury;                                        \
                                                                          \
            int convexity =  ay*bx - ax*by;/* if >0 then convex angle */  \
                                                                          \
            if (convexity _op_ 0)                                         \
            {                                                             \
                pprev = pcur;                                             \
                pcur = pnext;                                             \
                pnext+=incr;                                              \
                stack[stacksize] = pnext;                                 \
                stacksize++;                                              \
            }                                                             \
            else                                                          \
            {                                                             \
                if (pprev == start)                                       \
                {                                                         \
                    pcur = pnext;                                         \
                    stack[1] = pcur;                                      \
                    pnext+=incr;                                          \
                    stack[2] = pnext;                                     \
                                                                          \
                }                                                         \
                else                                                      \
                {                                                         \
                    stack[stacksize-2] = pnext;                           \
                    pcur = pprev;                                         \
                    stacksize--;                                          \
                    pprev = stack[stacksize-3];                           \
                }                                                         \
            }                                                             \
        }                                                                 \
        else                                                              \
        {                                                                 \
            pnext+=incr;                                                  \
            stack[stacksize-1] = pnext;                                   \
        }                                                                 \
    }                                                                     \
    stacksize--;                                                          \
    return stacksize;                                                     \
}


/*
IPCV_IMPLEMENT_SKLANSKY2(icvSklanskyTL2, >, >= )
IPCV_IMPLEMENT_SKLANSKY2(icvSklanskyTR2, <, >= )
IPCV_IMPLEMENT_SKLANSKY2(icvSklanskyBL2, <, <= )
IPCV_IMPLEMENT_SKLANSKY2(icvSklanskyBR2, >, <= )
*/

IPCV_IMPLEMENT_SKLANSKY3( icvSklanskyTL2, CvPoint *, >, >=,-> )
IPCV_IMPLEMENT_SKLANSKY3( icvSklanskyTR2, CvPoint *, <, >=,-> )
IPCV_IMPLEMENT_SKLANSKY3( icvSklanskyBL2, CvPoint *, <, <=,-> )
IPCV_IMPLEMENT_SKLANSKY3( icvSklanskyBR2, CvPoint *, >, <=,-> )

CvStatus
icvConvexHull_Approx_Contour( CvSeq * contour,
                              int bandwidth,
                              int orientation, CvMemStorage * storage, CvSeq ** hull )
{
    CvStatus status = CV_OK;
    
    /* bounds */
    int minx;
    int screenwidth;

    int maxy_ind = 0;
    int miny_ind = 0;

/*int* uh; *//* upper half pretenders  */
/*int* lh; *//* lower half pretenderts */
    CvPoint **puh;              /* upper half pretenders  */
    CvPoint **plh;              /* upper half pretenders  */
    int *flag;

    /* stack of previous points for Sklansky */
    int *stack;

    int i, hc;

    int numbands;
    int width = 0;

    CvPoint *rest;

    CvSeqReader contour_reader;
    CvSeqWriter hull_writer;

    float ibw = 1.f / bandwidth;

    cvStartReadSeq( contour, &contour_reader, 0 );

    /*if (contour->bounds.height < 0 ) */
    {
        int k;
        CvPoint *pt;
        int maxx = INT_MIN;

        minx = INT_MAX;

        /* find minx and maxx */
        for( k = 0; k < contour->total; k++ )
        {
            pt = (CvPoint *) (contour_reader.ptr);
            CV_NEXT_SEQ_ELEM( sizeof( CvPoint ), contour_reader );
            /* find minx and maxx */
            maxx = MAX( pt->x, maxx );
            minx = MIN( pt->x, minx );
        }
        screenwidth = maxx - minx + 1;
    }
/*
    else
    {
        minx = contour->bounds.x;
        screenwidth = contour->bounds.width;
    }
*/

    /* allocate memory for bands */
    numbands = screenwidth / bandwidth;
    numbands += (bandwidth * numbands < screenwidth);

/*
    uh = (int*)icvAlloc( 2 * numbands* sizeof(int) );
    lh = uh + numbands;
*/
    puh = (CvPoint **) icvAlloc( 2 * numbands * sizeof( CvPoint * ));
    plh = puh + numbands;
    flag = (int *) icvAlloc( numbands * sizeof( int ) + 2 );

    if( !( /*uh && */ puh && flag) )
    {
        status = CV_OUTOFMEM_ERR;
        goto freemem;
    }

    stack = flag;
    /* fill arrays */
    for( i = 0; i < numbands; i++ )
    {
        flag[i] = 0;
    }

    /* step two - finding maximal and minimal coordinate within every band */
    for( i = 0; i < contour->total; i++ )
    {
        /* find band */
        int x, y;
        CvPoint *pt = (CvPoint *) (contour_reader.ptr);

        CV_NEXT_SEQ_ELEM( sizeof( CvPoint ), contour_reader );
        x = cvFloor( (pt->x - minx) * ibw );
        y = pt->y;

        if( flag[x] == 0 )
        {
            width++;
            flag[x] = 1;
            /*lh[x] = uh[x] = i; */
            plh[x] = puh[x] = pt;
        }
        else
        {
            if( plh[x]->y >= y )
            {                   /*lh[x] = i; */
                plh[x] = pt;
            }
            if( puh[x]->y <= y )
            {                   /*uh[x] = i; */
                puh[x] = pt;
            }
        }
    }
    /* copy info from characteristic vector to solid array */
    /* with finding splitter */
    {
        int j = 0;

        for( i = 0; i < numbands; i++ )
        {
            if( flag[i] )
            {
                /*uh[j] = uh[i]; */ puh[j] = puh[i];
                if( puh[i]->y >= puh[maxy_ind]->y )
                    maxy_ind = j;

                /*lh[j] = lh[i]; */ plh[j] = plh[i];
                if( plh[i]->y <= plh[miny_ind]->y )
                    miny_ind = j;
                j++;
            }
        }
    }

    cvStartWriteSeq( CV_SEQ_KIND_CURVE | CV_SEQ_ELTYPE_PPOINT | CV_SEQ_FLAG_CLOSED,
                     sizeof( CvSeq ), sizeof( CvPoint * ), storage, &hull_writer );

    /*upper half */
    {
        int tl_count = icvSklanskyTL2( puh, 0, maxy_ind, stack );
        int *tr_stack = stack + tl_count;
        int tr_count = icvSklanskyTR2( puh, width - 1, maxy_ind, tr_stack );

        /* gather upper part of convex hull to output */
        hc = 0;
        if( orientation == CV_COUNTER_CLOCKWISE )
        {
            int i;

            for( i = 0; i < tr_count - 1; i++, hc++ )
            {
                CV_WRITE_SEQ_ELEM( puh[tr_stack[i]], hull_writer );
            }

            for( i = tl_count - 1; i > 0; i--, hc++ )
            {
                CV_WRITE_SEQ_ELEM( puh[stack[i]], hull_writer );
            }
            /* memorize rest */
            rest = puh[stack[0]];
        }
        else
        {
            int i;

            for( i = 0; i < tl_count - 1; i++, hc++ )
            {
                CV_WRITE_SEQ_ELEM( puh[stack[i]], hull_writer );
            }

            for( i = tr_count - 1; i > 0; i--, hc++ )
            {
                CV_WRITE_SEQ_ELEM( puh[tr_stack[i]], hull_writer );
            }
            /* memorize rest */
            rest = puh[tr_stack[0]];
        }
    }
    /* lower half */
    {
        int bl_count = icvSklanskyBL2( plh, 0, miny_ind, stack );
        int *br_stack = stack + bl_count;
        int br_count = icvSklanskyBR2( plh, width - 1, miny_ind, br_stack );

        if( orientation == CV_COUNTER_CLOCKWISE )
        {
            int i;

            if( plh[0] != puh[0] )
            {
                CV_WRITE_SEQ_ELEM( rest, hull_writer );
            }

            for( i = 0; i < bl_count - 1; i++, hc++ )
            {
                CV_WRITE_SEQ_ELEM( plh[stack[i]], hull_writer );
            }

            for( i = br_count - 1; i > 0; i--, hc++ )
            {
                CV_WRITE_SEQ_ELEM( plh[br_stack[i]], hull_writer );
            }

            if( plh[width - 1] != puh[width - 1] )
            {
                CV_WRITE_SEQ_ELEM( plh[br_stack[0]], hull_writer );
            }
        }
        else
        {
            int i;

            if( plh[width - 1] != puh[width - 1] )
            {
                CV_WRITE_SEQ_ELEM( rest, hull_writer );
            }

            for( i = 0; i < br_count - 1; i++, hc++ )
            {
                CV_WRITE_SEQ_ELEM( plh[br_stack[i]], hull_writer );
            }

            for( i = bl_count - 1; i > 0; i--, hc++ )
            {
                CV_WRITE_SEQ_ELEM( plh[stack[i]], hull_writer );
            }
            if( plh[0] != puh[0] )
            {
                CV_WRITE_SEQ_ELEM( plh[stack[0]], hull_writer );
            }
        }
    }
    *hull = cvEndWriteSeq( &hull_writer );

  freemem:

    /*if (uh)   icvFree(&uh); */
    if( puh )
        icvFree( &puh );
    if( flag )
        icvFree( &flag );

    return status;
}


/* exact convex hull with points sorting */
#define cmp_pts( ind1, ind2 )\
( aux[ind1].x  < aux[ind2].x  ||\
  aux[ind1].x == aux[ind2].x && \
  aux[ind1].y  < aux[ind2].y   )

CV_IMPLEMENT2_QSORT( icvSortPoints, int, cmp_pts, CvPoint * );

CvStatus icvConvexHull_Exact( CvPoint * points,
                              int num_points,
                              int orientation, int *hullpoints, int *hullsize )
{
    int i;
    int maxy_ind = 0, miny_ind = 0;
    int hc = 0;

    int *index;
    int *stack;

    index = (int *) icvAlloc( num_points * sizeof( int * ));

    if( index == NULL )
        return CV_OUTOFMEM_ERR;

    stack = (int *) icvAlloc( (num_points + 2) * sizeof( int ));

    if( stack == NULL )
    {
        icvFree( &index );
        return CV_OUTOFMEM_ERR;
    }

    for( i = 0; i < num_points; i++ )
    {
        index[i] = i;
    }

    icvSortPoints( index, num_points, points );

    /* find top and bottom */
    for( i = 0; i < num_points; i++ )
    {
        maxy_ind = (points[index[maxy_ind]].y < points[index[i]].y) ? i : maxy_ind;
        miny_ind = (points[index[miny_ind]].y > points[index[i]].y) ? i : miny_ind;
    }

    /*upper half */
    {
        int *tl_stack = stack;
        int tl_count = icvSklanskyTL( points, index, 0, maxy_ind, tl_stack );
        int *tr_stack = tl_stack + tl_count;
        int tr_count = icvSklanskyTR( points, index, num_points - 1, maxy_ind, tr_stack );

        /* gather upper part of convex hull to output */
        if( orientation == CV_COUNTER_CLOCKWISE )
        {
            int i;

            for( i = 0; i < tr_count - 1; i++, hc++ )
            {
                hullpoints[hc] = index[tr_stack[i]];
            }

            for( i = tl_count - 1; i > 0; i--, hc++ )
            {
                hullpoints[hc] = index[tl_stack[i]];
            }
        }
        else
        {
            int i;

            for( i = 0; i < tl_count - 1; i++, hc++ )
            {
                hullpoints[hc] = index[tl_stack[i]];
            }

            for( i = tr_count - 1; i > 0; i--, hc++ )
            {
                hullpoints[hc] = index[tr_stack[i]];
            }
        }
    }
    /* lower half */
    {
        int *bl_stack = stack;
        int bl_count = icvSklanskyBL( points, index, 0, miny_ind, bl_stack );
        int *br_stack = stack + bl_count;
        int br_count = icvSklanskyBR( points, index, num_points - 1, miny_ind, br_stack );

        /*if (lh[0] == uh[0]) */
        /*{ bl_stack+=1; bl_count--; }
           if (lh[width - 1] == uh[width - 1]) */
        /*{ br_stack+=1; br_count--; } */

        if( orientation == CV_COUNTER_CLOCKWISE )
        {
            int i;

            for( i = 0; i < bl_count - 1; i++, hc++ )
            {
                hullpoints[hc] = index[bl_stack[i]];
            }

            for( i = br_count - 1; i > 0; i--, hc++ )
            {
                hullpoints[hc] = index[br_stack[i]];
            }
        }
        else
        {
            int i;

            for( i = 0; i < br_count - 1; i++, hc++ )
            {
                hullpoints[hc] = index[br_stack[i]];
            }

            for( i = bl_count - 1; i > 0; i--, hc++ )
            {
                hullpoints[hc] = index[bl_stack[i]];
            }
        }
    }

    *hullsize = hc;
    icvFree( &index );
    icvFree( &stack );

    return CV_OK;
}

/* _op_ >  for upper half,  <  for lower half */
/* _op2_ >  in ascendance of y ,  <  in descendance of y */



/* exact contour convex hull with points sorting */
/*typedef struct
{
    CvPoint* pointer;
    int       index;
} cvPointPosition;
*/
/*#define pt_access  .pointer->*/
/*
IPCV_IMPLEMENT_SKLANSKY3(icvSklanskyTL3, cvPointPosition, >, >= , .pointer-> )
IPCV_IMPLEMENT_SKLANSKY3(icvSklanskyTR3, cvPointPosition, <, >= , .pointer-> )
IPCV_IMPLEMENT_SKLANSKY3(icvSklanskyBL3, cvPointPosition, <, <= , .pointer-> )
IPCV_IMPLEMENT_SKLANSKY3(icvSklanskyBR3, cvPointPosition, >, <= , .pointer-> )
*/


/*
#define cmp_pts1( pos1, pos2 ) \
( pos1.pointer->x  < pos2.pointer->x  ||       \
  pos1.pointer->x == pos2.pointer->x &&       \
  pos1.pointer->y  < pos2.pointer->y  )

*/
/*IPCV_IMPLEMENT_QSORT( icvSortPointsByPosition, cvPointPosition, cmp_pts1 )*/

#define cmp_pts1( ptr1, ptr2 ) \
( ptr1->x  < ptr2->x  ||       \
  ptr1->x == ptr2->x &&       \
  ptr1->y  < ptr2->y  )

CV_IMPLEMENT_QSORT( icvSortPointsByPointers, CvPoint *, cmp_pts1 );

IPCV_IMPLEMENT_SKLANSKY3( icvSklanskyTL3, CvPoint *, >, >=,-> )
IPCV_IMPLEMENT_SKLANSKY3( icvSklanskyTR3, CvPoint *, <, >=,-> )
IPCV_IMPLEMENT_SKLANSKY3( icvSklanskyBL3, CvPoint *, <, <=,-> )
IPCV_IMPLEMENT_SKLANSKY3( icvSklanskyBR3, CvPoint *, >, <=,-> )

CvStatus
icvContourConvexHull_Exact( CvSeq * contour,
                            int orientation, CvMemStorage * storage, CvSeq ** hull )
{
    int i;
    int maxy_ind = 0, miny_ind = 0;
    int hc = 0;

    CvPoint **pointer;
    int *stack;

    CvSeqReader reader;
    CvSeqWriter writer;

    if( contour->total == 0 ) return CV_BADRANGE_ERR;

    pointer = (CvPoint **) icvAlloc( contour->total * sizeof( CvPoint * ));
    if( pointer == NULL )
        return CV_OUTOFMEM_ERR;

    stack = (int *) icvAlloc( (contour->total + 2) * sizeof( int ));

    if( stack == NULL )
    {
        icvFree( &pointer );
        return CV_OUTOFMEM_ERR;
    }

    cvStartReadSeq( contour, &reader, 0 );

    for( i = 0; i < contour->total; i++ )
    {
        pointer[i] = (CvPoint *) reader.ptr;
        CV_NEXT_SEQ_ELEM( sizeof( CvPoint ), reader );
    }

    icvSortPointsByPointers( pointer, contour->total, 0 );

    /* find top and bottom */
    for( i = 0; i < contour->total; i++ )
    {
        maxy_ind = (pointer[maxy_ind]->y < pointer[i]->y) ? i : maxy_ind;
        miny_ind = (pointer[miny_ind]->y > pointer[i]->y) ? i : miny_ind;
    }
    cvStartWriteSeq( CV_SEQ_KIND_CURVE | CV_SEQ_ELTYPE_PPOINT | CV_SEQ_FLAG_CLOSED,
                     sizeof( CvSeq ), sizeof( CvPoint * ), storage, &writer );


    /*upper half */
    {
        int *tl_stack = stack;
        int tl_count = icvSklanskyTL3( pointer, 0, maxy_ind, tl_stack );
        int *tr_stack = tl_stack + tl_count;
        int tr_count = icvSklanskyTR3( pointer, contour->total - 1, maxy_ind, tr_stack );

        /* gather upper part of convex hull to output */
        hc = 0;
        if( orientation == CV_COUNTER_CLOCKWISE )
        {
            int i;

            for( i = 0; i < tr_count - 1; i++, hc++ )
            {
                CV_WRITE_SEQ_ELEM( pointer[tr_stack[i]], writer );
            }

            for( i = tl_count - 1; i > 0; i--, hc++ )
            {
                CV_WRITE_SEQ_ELEM( pointer[tl_stack[i]], writer );
            }
        }
        else
        {
            int i;

            for( i = 0; i < tl_count - 1; i++, hc++ )
            {
                CV_WRITE_SEQ_ELEM( pointer[tl_stack[i]], writer );
            }

            for( i = tr_count - 1; i > 0; i--, hc++ )
            {
                CV_WRITE_SEQ_ELEM( pointer[tr_stack[i]], writer );
            }
        }
    }
    /* lower half */
    {
        int *bl_stack = stack;
        int bl_count = icvSklanskyBL3( pointer, 0, miny_ind, bl_stack );
        int *br_stack = stack + bl_count;
        int br_count = icvSklanskyBR3( pointer, contour->total - 1, miny_ind, br_stack );

        /*{ bl_stack+=1; bl_count--; } */
        /*{ br_stack+=1; br_count--; } */
        if( orientation == CV_COUNTER_CLOCKWISE )
        {
            int i;

            for( i = 0; i < bl_count - 1; i++, hc++ )
            {
                CV_WRITE_SEQ_ELEM( pointer[bl_stack[i]], writer );
            }

            for( i = br_count - 1; i > 0; i--, hc++ )
            {
                CV_WRITE_SEQ_ELEM( pointer[br_stack[i]], writer );
            }
        }
        else
        {
            int i;

            for( i = 0; i < br_count - 1; i++, hc++ )
            {
                CV_WRITE_SEQ_ELEM( pointer[br_stack[i]], writer );
            }

            for( i = bl_count - 1; i > 0; i--, hc++ )
            {
                CV_WRITE_SEQ_ELEM( pointer[bl_stack[i]], writer );
            }
        }
    }
    *hull = cvEndWriteSeq( &writer );

    icvFree( &pointer );
    icvFree( &stack );

    return CV_OK;
}



/*typedef struct
{
    CvPoint* start;
    CvPoint* end;
    CvPoint* depth_point;
    float depth;
}CvConvexityDefect;
*/

/* contour must be a simple polygon */
/* it must have more than 3 points  */
CvStatus
icvConvexityDefects( CvSeq * contour, CvSeq * convexhull,
                     CvMemStorage * storage, CvSeq ** defects )
{
    CvPoint *pos;
    int index, i;

    /* is orientation of hull different from contour one */
    int rev_orientation;

    CvSeqReader hull_reader;
    CvSeqReader contour_reader;
    CvSeqWriter writer;

    if( contour->total < 4 || convexhull->total < 3)
        return CV_BADSIZE_ERR;
    /* recognize co-orientation of contour and its hull */
    {
        int sign = 0;
        int index1, index2, index3;

        pos = *((CvPoint **) (CV_GET_SEQ_ELEM( CvPoint *, convexhull, 0 )));
        index1 = cvSeqElemIdx( contour, (char *) pos, NULL );

        pos = *((CvPoint **) (CV_GET_SEQ_ELEM( CvPoint *, convexhull, 1 )));
        index2 = cvSeqElemIdx( contour, (char *) pos, NULL );

        pos = *((CvPoint **) (CV_GET_SEQ_ELEM( CvPoint *, convexhull, 2 )));
        index3 = cvSeqElemIdx( contour, (char *) pos, NULL );

        sign += (index2 > index1) ? 1 : 0;
        sign += (index3 > index2) ? 1 : 0;
        sign += (index1 > index3) ? 1 : 0;

        rev_orientation = (sign == 2) ? 0 : 1;
    }

    cvStartReadSeq( contour, &contour_reader, 0 );
    cvStartReadSeq( convexhull, &hull_reader, rev_orientation );

    pos = *((CvPoint **) (hull_reader.prev_elem));
    index = cvSeqElemIdx( contour, (char *) pos, NULL );
    cvSetSeqReaderPos( &contour_reader, index );

    cvStartWriteSeq( CV_SEQ_KIND_CURVE | CV_SEQ_FLAG_CLOSED, sizeof( CvSeq ),
                     sizeof( CvConvexityDefect ), storage, &writer );

    /* cycle through contour and hull with computing defects */
    for( i = 0; i < convexhull->total; i++ )
    {
        CvConvexityDefect defect;
        int is_defect = 0;
        int dx0, dy0;
        float depth = 0;

        /*if (rev_orientation) 
           {        
           CV_REV_READ_SEQ_ELEM( pos, hull_reader );
           }
           else
           {
           CV_READ_SEQ_ELEM( pos, hull_reader );
           } */
        dx0 = (*(CvPoint **) (hull_reader.ptr))->x - pos->x;
        dy0 = (*(CvPoint **) (hull_reader.ptr))->y - pos->y;
        /* go through contour to achieve next hull point */
        CV_NEXT_SEQ_ELEM( sizeof( CvPoint ), contour_reader );

        defect.start = pos;
        defect.end = *((CvPoint **) (hull_reader.ptr));

        while( contour_reader.ptr != (char *) (*((CvPoint **) hull_reader.ptr)))
        {
            CvPoint *cur = (CvPoint *) (contour_reader.ptr);

            /* compute distance from current point to hull edge */
            int dx = cur->x - pos->x;
            int dy = cur->y - pos->y;

            /* compute depth */
            int prod = abs( -dy0 * dx + dx0 * dy );
            int norm = dx0 * dx0 + dy0 * dy0;
            float inorm = cvInvSqrt( (float) norm );
            float dist = prod * inorm;

            if( dist > depth )
            {
                depth = dist;
                defect.depth_point = cur;
                defect.depth = depth;
                is_defect = 1;
            }
            CV_NEXT_SEQ_ELEM( sizeof( CvPoint ), contour_reader );
        }
        if( is_defect )
        {
            CV_WRITE_SEQ_ELEM( defect, writer );
        }

        if( rev_orientation )
        {
            CV_REV_READ_SEQ_ELEM( pos, hull_reader );
        }
        else
        {
            CV_READ_SEQ_ELEM( pos, hull_reader );
        }
    }

    *defects = cvEndWriteSeq( &writer );

    return CV_OK;
}


CvStatus
icvConvexHull( CvPoint * points,
               int num_points, CvRect * /*bound_rect*/, int orientation, int *hull, int *hullsize )
{
    if( points == NULL )
        return CV_NULLPTR_ERR;
    if( num_points <= 0 )
        return CV_BADSIZE_ERR;
    if( hull == NULL )
        return CV_NULLPTR_ERR;
    if( hullsize == NULL )
        return CV_NULLPTR_ERR;

    return icvConvexHull_Exact( points, num_points, orientation, hull, hullsize );
}

CvStatus
icvContourConvexHull( CvSeq * contour, int orientation, CvMemStorage * storage, CvSeq ** hull )
{
    if( contour == NULL )
        return CV_NULLPTR_ERR;
    if( storage == NULL )
        return CV_NULLPTR_ERR;
    if( hull == NULL )
        return CV_NULLPTR_ERR;

    return icvContourConvexHull_Exact( contour, orientation, storage, hull );
}


int
icvCheckContourConvexity( CvSeq * contour )
{
    int i;
    int dx0, dy0;
    CvPoint *cur_pt;
    CvPoint *prev_pt;
    int orientation = 0;
    CvSeqReader reader;

    /* check arguments */
    if( contour == NULL )
        return ( int ) CV_NULLPTR_ERR;
    if( !CV_IS_SEQ_POLYGON( contour ))
        return ( int ) CV_BADFLAG_ERR;

    cvStartReadSeq( contour, &reader, 0 );
    prev_pt = (CvPoint *) reader.prev_elem;
    cur_pt = (CvPoint *) reader.ptr;
    dx0 = cur_pt->x - prev_pt->x;
    dy0 = cur_pt->y - prev_pt->y;

    for( i = 0; i < contour->total; i++ )
    {
        int dxdy0, dydx0;
        int dx, dy;

        /*int orient; */
        CV_NEXT_SEQ_ELEM( sizeof( CvPoint ), reader );
        prev_pt = cur_pt;
        cur_pt = (CvPoint *) reader.ptr;

        dx = cur_pt->x - prev_pt->x;
        dy = cur_pt->y - prev_pt->y;
        dxdy0 = dx * dy0;
        dydx0 = dy * dx0;

        /* find orientation */
        /*orient = -dy0 * dx + dx0 * dy;
           orientation |= (orient > 0) ? 1 : 2;
         */
        orientation |= (dydx0 > dxdy0) ? 1 : ((dydx0 < dxdy0) ? 2 : 3);

        if( orientation == 3 )
            return 0;

        dx0 = dx;
        dy0 = dy;
    }

    return 1;
}

/* end of file */
