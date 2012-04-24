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

/*
    Partially based on Yossi Rubner code:
    =========================================================================
    emd.c

    Last update: 3/14/98

    An implementation of the Earth Movers Distance.
    Based of the solution for the Transportation problem as described in
    "Introduction to Mathematical Programming" by F. S. Hillier and 
    G. J. Lieberman, McGraw-Hill, 1990.

    Copyright (C) 1998 Yossi Rubner
    Computer Science Department, Stanford University
    E-Mail: rubner@cs.stanford.edu   URL: http://vision.stanford.edu/~rubner
    ==========================================================================
*/
#include "_cv.h"
#include "_cvwrap.h"

#define MAX_ITERATIONS 500
#define CV_EMD_INF   ((float)1e20)
#define CV_EMD_EPS   ((float)1e-5)

/* CvNode1D is used for lists, representing 1D sparse array */
typedef struct CvNode1D
{
    float val;
    struct CvNode1D *next;
}
CvNode1D;

/* CvNode2D is used for lists, representing 2D sparse matrix */
typedef struct CvNode2D
{
    float val;
    struct CvNode2D *next[2];  /* next row & next column */
    int i, j;
}
CvNode2D;


typedef struct CvEMDState
{
    int ssize, dsize;

    float **cost;
    CvNode2D *_x;
    CvNode2D *end_x;
    CvNode2D *enter_x;
    char **is_x;

    CvNode2D **rows_x;
    CvNode2D **cols_x;

    CvNode1D *u;
    CvNode1D *v;

    /* find_loop buffers */
    CvNode2D **loop;
    char *is_used;

    /* russel buffers */
    float *s;
    float *d;
    float **delta;

    float weight, max_cost;
    char *buffer;
}
CvEMDState;

/* static function declaration */
static CvStatus icvInitEMD( const float *signature1, int size1,
                            const float *signature2, int size2,
                            int dims,
                            CvDistanceFunction dist_func,
                            void *user_param,
                            CvEMDState * state, float *lower_bound,
                            char *local_buffer, int local_buffer_size );

static CvStatus icvFindBasicVariables( float **cost, char **is_x,
                                       CvNode1D * u, CvNode1D * v, int ssize, int dsize );

static float icvIsOptimal( float **cost, char **is_x,
                           CvNode1D * u, CvNode1D * v,
                           int ssize, int dsize, CvNode2D * enter_x );

static void icvRussel( CvEMDState * state );


static CvStatus icvNewSolution( CvEMDState * state );
static int icvFindLoop( CvEMDState * state );

static void icvAddBasicVariable( CvEMDState * state,
                                 int min_i, int min_j,
                                 CvNode1D * prev_u_min_i,
                                 CvNode1D * prev_v_min_j,
                                 CvNode1D * u_head );

static float icvDistL2( const float *x, const float *y, void *user_param );
static float icvDistL1( const float *x, const float *y, void *user_param );
static float icvDistC( const float *x, const float *y, void *user_param );

/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:       cvCalcEMD
//    Purpose:    Computes Earth mover distance (and/or lower bound of it) for given pair
//                of signatures. By default, ground distance is calculated as
//                L1, L2 or C distance using features' coordinates.
//    Context:
//    Parameters:
//      signature1  - first signature - array of size1 * (dims + 1) elements
//      signature2  - second signature - array of size2 * (dims + 1) elements
//      dims        - number of dimensions in feature space. If 0, then
//                    signature1 and signature2 are considered as simple 1D histograms,
//                    else both signatures must look as follows:
//                    (weight_i0, x0_i0, x1_i0, ..., x(dims-1)_i0,
//                     weight_i1, x0_i1, x1_i1, ..., x(dims-1)_i1,
//                     ...
//                     weight_(size1-1),x0_(size1-1),x1_(size1-1,...,x(dims-1)_(size1-1))
//
//                     where weight_ik - weight of ik cluster.
//                     x0_ik,...,x(dims-1)_ik - coordinates of ik cluster.
//
//      weights     - array of weights for each coordinate,
//                    which is used when wieghted euclidian distance is calculated.
//                    if 0, then all weights are set to 1s.
//      emd         - pointer to calculated emd distance
//      lower_bound - pointer to calculated lower bound.
//                    if 0, this quantity is not calculated (only emd is calculated).
//                    else if calculated lower bound is greater or equal to the value,
//                    stored at this pointer, then the true emd is not calculated, but
//                    is set to that lower_bound.
//    Returns:
//      CV_NO_ERR if ok, error code else
//    Notes:
//F*/
CV_IMPL float
cvCalcEMD( const float *signature1, int size1,
           const float *signature2, int size2,
           int dims, CvDisType dist_type,
           CvDistanceFunction dist_func,
           float *lower_bound,
           void *user_param )
{
    char local_buffer[16384];
    char *local_buffer_ptr = (char *)icvAlignPtr(local_buffer,16);
    CvEMDState state;
    float emd = 0;

    CV_FUNCNAME( "cvCalcEMD" );

    memset( &state, 0, sizeof(state));

    __BEGIN__;
    int i, j, itr;
    float total_cost = 0;
    
    CvStatus result = CV_NO_ERR;
    float eps, min_delta;
    CvNode2D *xp = 0;

    if( !signature1 || !signature2 )
        CV_ERROR( CV_StsNullPtr, "Null pointers to signatures" );
    if( size1 <= 0 || size2 <= 0 || dims < 0 )
        CV_ERROR( CV_StsBadSize,
        "size1 <= 0 or size2 <= 0 or number_of_dimensions < 0" );

    if( dist_type < 0 )
    {
        if( !dist_func )
            CV_ERROR( CV_StsNullPtr, "Distance function is undefined" );
    }
    else
    {
        if( dims == 0 )
            CV_ERROR( CV_StsBadSize,
            "Number of dimensions can be 0 only if a user-defined metric is used" );
        user_param = (void *) dims;
        switch (dist_type)
        {
        case CV_DIST_L1:
            dist_func = icvDistL1;
            break;
        case CV_DIST_L2:
            dist_func = icvDistL2;
            break;
        case CV_DIST_C:
            dist_func = icvDistC;
            break;
        default:
            CV_ERROR( CV_StsBadFlag, "Bad or unsupported metric type" );
        }
    }

    IPPI_CALL( result = icvInitEMD( signature1, size1, signature2, size2,
                                    dims, dist_func, user_param,
                                    &state, lower_bound, local_buffer_ptr,
                                    sizeof( local_buffer ) - 16 ));

    if( result > 0 && lower_bound )
    {
        emd = *lower_bound;
        EXIT;
    }

    eps = CV_EMD_EPS * state.max_cost;

    /* if ssize = 1 or dsize = 1 then we are done, else ... */
    if( state.ssize > 1 && state.dsize > 1 )
    {
        for( itr = 1; itr < MAX_ITERATIONS; itr++ )
        {
            /* find basic variables */
            result = icvFindBasicVariables( state.cost, state.is_x,
                                            state.u, state.v, state.ssize, state.dsize );
            if( result < 0 )
                break;

            /* check for optimality */
            min_delta = icvIsOptimal( state.cost, state.is_x,
                                      state.u, state.v,
                                      state.ssize, state.dsize, state.enter_x );

            if( min_delta == CV_EMD_INF )
            {
                CV_ERROR( CV_StsNoConv, "" );
            }

            /* if no negative deltamin, we found the optimal solution */
            if( min_delta >= -eps )
                break;

            /* improve solution */
            IPPI_CALL( icvNewSolution( &state ));
        }
    }

    /* compute the total flow */
    for( xp = state._x; xp < state.end_x; xp++ )
    {
        float val = xp->val;

        i = xp->i;
        j = xp->j;

        if( xp != state.enter_x && i < size1 && j < size2 && val != 0 )
        {
            total_cost += val * state.cost[i][j];
        }
    }

    emd = (float) (total_cost / state.weight);

    __END__;

    if( state.buffer && state.buffer != local_buffer_ptr )
        icvFree( &(state.buffer) );

    return emd;
}


/************************************************************************************\
*          initialize structure, allocate buffers and generate initial golution      *
\************************************************************************************/
static CvStatus
icvInitEMD( const float* signature1, int size1,
            const float* signature2, int size2,
            int dims,
            CvDistanceFunction dist_func, void* user_param,
            CvEMDState* state, float* lower_bound,
            char* local_buffer, int local_buffer_size )
{
    float s_sum = 0, d_sum = 0, diff;
    float *ptr;
    int i, j, ci = 0, cj = 0, ssize, dsize;
    int equal_sums = 1;
    int buffer_size;
    float *cost_matrix;
    float max_cost = 0;
    char *buffer, *buffer_end;
    float *xs, *xd;

    ssize = 0;
    dsize = 0;

    memset( state, 0, sizeof( *state ));

    /* sum up the supply and demand */
    for( i = 0; i < size1; i++ )
    {
        float temp = signature1[i * (dims + 1)];

        if( temp > 0 )
        {
            s_sum += temp;
            ssize++;
        }
        else if( temp < 0 )
            return CV_BADRANGE_ERR;
    }

    for( i = 0; i < size2; i++ )
    {
        float temp = signature2[i * (dims + 1)];

        if( temp > 0 )
        {
            d_sum += temp;
            dsize++;
        }
        else if( temp < 0 )
            return CV_BADRANGE_ERR;
    }

    if( ssize == 0 || dsize == 0 )
        return CV_BADRANGE_ERR;

    /* if supply different than the demand, add a zero-cost dummy cluster */
    diff = s_sum - d_sum;
    if( fabs( diff ) >= CV_EMD_EPS * s_sum )
    {
        equal_sums = 0;
        if( diff < 0 )
            ssize++;
        else
            dsize++;
    }

    state->weight = s_sum > d_sum ? s_sum : d_sum;

    /* calculate buffer size */
    buffer_size = ssize * dsize * (sizeof( float ) +    /* cost */
                                   sizeof( char ) +     /* is_x */
                                   sizeof( float )) +   /* delta matrix */
        (ssize + dsize) * (sizeof( CvNode2D ) + /* _x */
                           sizeof( CvNode2D * ) +       /* cols_x & rows_x */
                           sizeof( CvNode1D ) + /* u & v */
                           sizeof( float )) +   /* s & d */
        ssize * (sizeof( float * ) + sizeof( char * ) + /* rows pointers for */
                 sizeof( float * )) + 256;      /*  cost, is_x and delta */

    if( buffer_size < (int) (dims * 2 * sizeof( float )))
    {
        buffer_size = dims * 2 * sizeof( float );
    }

    /* allocate buffers */
    if( local_buffer != 0 && local_buffer_size >= buffer_size )
    {
        buffer = local_buffer;
    }
    else
    {
        buffer = (char *) icvAlloc( buffer_size );
        if( !buffer )
            return CV_OUTOFMEM_ERR;
    }

    state->buffer = buffer;
    buffer_end = buffer + buffer_size;
    xs = (float *) buffer;
    xd = xs + dims;

    if( lower_bound && equal_sums )     /* check lower bound */
    {
        int sz1 = size1 * (dims + 1), sz2 = size2 * (dims + 1);
        float lb = 0;

        for( i = 0; i < dims; i++ )
        {
            float s = 0, d = 0;

            for( j = 0; j < sz1; j += dims + 1 )
            {
                s += signature1[j + i + 1] * signature1[j];
            }

            for( j = 0; j < sz2; j += dims + 1 )
            {
                d += signature2[j + i + 1] * signature2[j];
            }

            xs[i] = s;
            xd[i] = d;
        }

        lb = dist_func( xs, xd, user_param ) / state->weight;
        i = *lower_bound <= lb;
        *lower_bound = lb;
        if( i )
            return ( CvStatus ) 1;
    }

    /* assign pointers */
    state->is_used = (char *) buffer;
    /* init delta matrix */
    state->delta = (float **) buffer;
    buffer += ssize * sizeof( float * );

    for( i = 0; i < ssize; i++ )
    {
        state->delta[i] = (float *) buffer;
        buffer += dsize * sizeof( float );
    }

    state->loop = (CvNode2D **) buffer;
    state->s = (float *) buffer;
    buffer += (ssize + 1) * sizeof( float );

    state->d = (float *) buffer;
    buffer += (dsize + 1) * sizeof( float );

    state->_x = (CvNode2D *) buffer;
    buffer += (ssize + dsize) * sizeof( CvNode2D );

    memset( buffer, 0, buffer_end - buffer );

    state->rows_x = (CvNode2D **) buffer;
    buffer += ssize * sizeof( CvNode2D * );

    state->cols_x = (CvNode2D **) buffer;
    buffer += dsize * sizeof( CvNode2D * );

    state->u = (CvNode1D *) buffer;
    buffer += ssize * sizeof( CvNode1D );

    state->v = (CvNode1D *) buffer;
    buffer += dsize * sizeof( CvNode1D );

    /* init cost matrix */
    state->cost = (float **) buffer;
    buffer += ssize * sizeof( float * );

    /* compute the distance matrix */
    for( i = 0; i < size1; i++ )
    {
        state->cost[i] = (float *) buffer;
        buffer += dsize * sizeof( float );

        if( signature1[i * (dims + 1)] != 0 )
        {
            for( j = 0, cj = 0; j < size2; j++ )
            {
                if( signature2[j] != 0 )
                {
                    state->cost[ci][cj++] = dist_func( signature1 + i * (dims + 1) + 1,
                                                       signature2 + j * (dims + 1) + 1,
                                                       user_param );
                }
            }
            ci++;
        }
    }

    /* init is_x matrix */
    state->is_x = (char **) buffer;
    buffer += ssize * sizeof( char * );

    for( i = 0; i < ssize; i++ )
    {
        state->is_x[i] = buffer;
        buffer += dsize;
    }

    /* find maximum of the cost matrix */
    max_cost = 0;
    cost_matrix = state->cost[0];
    for( i = 0; i < ssize * dsize; i++ )
    {
        float temp = cost_matrix[i];

        if( temp < 0 )
            return CV_BADRANGE_ERR;
        if( max_cost < temp )
            max_cost = temp;
    }

    state->ssize = ssize;
    state->dsize = dsize;
    state->max_cost = max_cost;

    state->end_x = state->_x;

    for( i = 0, ptr = state->s; i < size1; i++ )
    {
        float temp = signature1[i * (dims + 1)];

        if( temp != 0 )
            *ptr++ = temp;
    }

    for( i = 0, ptr = state->d; i < size2; i++ )
    {
        float temp = signature2[i * (dims + 1)];

        if( temp != 0 )
            *ptr++ = temp;
    }

    if( !equal_sums )
    {
        if( diff < 0 )
            state->s[ssize - 1] = -diff;
        else
            state->d[dsize - 1] = diff;
    }

    icvRussel( state );

    state->enter_x = (state->end_x)++;
    return CV_NO_ERR;
}


/****************************************************************************************\
*                              icvFindBasicVariables                                   *
\****************************************************************************************/
static CvStatus
icvFindBasicVariables( float **cost, char **is_x,
                       CvNode1D * u, CvNode1D * v, int ssize, int dsize )
{
    int i, j, found;
    int u_cfound, v_cfound;
    CvNode1D u0_head, u1_head, *cur_u, *prev_u;
    CvNode1D v0_head, v1_head, *cur_v, *prev_v;

    /* initialize the rows list (u) and the columns list (v) */
    u0_head.next = u;
    for( i = 0; i < ssize; i++ )
    {
        u[i].next = u + i + 1;
    }
    u[ssize - 1].next = 0;
    u1_head.next = 0;

    v0_head.next = ssize > 1 ? v + 1 : 0;
    for( i = 1; i < dsize; i++ )
    {
        v[i].next = v + i + 1;
    }
    v[dsize - 1].next = 0;
    v1_head.next = 0;

    /* there are ssize+dsize variables but only ssize+dsize-1 independent equations,
       so set v[0]=0 */
    v[0].val = 0;
    v1_head.next = v;
    v1_head.next->next = 0;

    /* loop until all variables are found */
    u_cfound = v_cfound = 0;
    while( u_cfound < ssize || v_cfound < dsize )
    {
        found = 0;
        if( v_cfound < dsize )
        {
            /* loop over all marked columns */
            prev_v = &v1_head;

            for( found |= (cur_v = v1_head.next) != 0; cur_v != 0; cur_v = cur_v->next )
            {
                float cur_v_val = cur_v->val;

                j = cur_v - v;
                /* find the variables in column j */
                prev_u = &u0_head;
                for( cur_u = u0_head.next; cur_u != 0; )
                {
                    i = cur_u - u;
                    if( is_x[i][j] )
                    {
                        /* compute u[i] */
                        cur_u->val = cost[i][j] - cur_v_val;
                        /* ...and add it to the marked list */
                        prev_u->next = cur_u->next;
                        cur_u->next = u1_head.next;
                        u1_head.next = cur_u;
                        cur_u = prev_u->next;
                    }
                    else
                    {
                        prev_u = cur_u;
                        cur_u = cur_u->next;
                    }
                }
                prev_v->next = cur_v->next;
                v_cfound++;
            }
        }

        if( u_cfound < ssize )
        {
            /* loop over all marked rows */
            prev_u = &u1_head;
            for( found |= (cur_u = u1_head.next) != 0; cur_u != 0; cur_u = cur_u->next )
            {
                float cur_u_val = cur_u->val;
                float *_cost;
                char *_is_x;

                i = cur_u - u;
                _cost = cost[i];
                _is_x = is_x[i];
                /* find the variables in rows i */
                prev_v = &v0_head;
                for( cur_v = v0_head.next; cur_v != 0; )
                {
                    j = cur_v - v;
                    if( _is_x[j] )
                    {
                        /* compute v[j] */
                        cur_v->val = _cost[j] - cur_u_val;
                        /* ...and add it to the marked list */
                        prev_v->next = cur_v->next;
                        cur_v->next = v1_head.next;
                        v1_head.next = cur_v;
                        cur_v = prev_v->next;
                    }
                    else
                    {
                        prev_v = cur_v;
                        cur_v = cur_v->next;
                    }
                }
                prev_u->next = cur_u->next;
                u_cfound++;
            }
        }

        if( !found )
        {
            return CV_NOTDEFINED_ERR;
        }
    }

    return CV_NO_ERR;
}


/****************************************************************************************\
*                                   icvIsOptimal                                       *
\****************************************************************************************/
static float
icvIsOptimal( float **cost, char **is_x,
              CvNode1D * u, CvNode1D * v, int ssize, int dsize, CvNode2D * enter_x )
{
    float delta, min_delta = CV_EMD_INF;
    int i, j, min_i = 0, min_j = 0;

    /* find the minimal cij-ui-vj over all i,j */
    for( i = 0; i < ssize; i++ )
    {
        float u_val = u[i].val;
        float *_cost = cost[i];
        char *_is_x = is_x[i];

        for( j = 0; j < dsize; j++ )
        {
            if( !_is_x[j] )
            {
                delta = _cost[j] - u_val - v[j].val;
                if( min_delta > delta )
                {
                    min_delta = delta;
                    min_i = i;
                    min_j = j;
                }
            }
        }
    }

    enter_x->i = min_i;
    enter_x->j = min_j;

    return min_delta;
}

/****************************************************************************************\
*                                   icvNewSolution                                     *
\****************************************************************************************/
static CvStatus
icvNewSolution( CvEMDState * state )
{
    int i, j;
    float min_val = CV_EMD_INF;
    int steps;
    CvNode2D head, *cur_x, *next_x, *leave_x = 0;
    CvNode2D *enter_x = state->enter_x;
    CvNode2D **loop = state->loop;

    /* enter the new basic variable */
    i = enter_x->i;
    j = enter_x->j;
    state->is_x[i][j] = 1;
    enter_x->next[0] = state->rows_x[i];
    enter_x->next[1] = state->cols_x[j];
    enter_x->val = 0;
    state->rows_x[i] = enter_x;
    state->cols_x[j] = enter_x;

    /* find a chain reaction */
    steps = icvFindLoop( state );

    if( steps == 0 )
        return CV_NOTDEFINED_ERR;

    /* find the largest value in the loop */
    for( i = 1; i < steps; i += 2 )
    {
        float temp = loop[i]->val;

        if( min_val > temp )
        {
            leave_x = loop[i];
            min_val = temp;
        }
    }

    /* update the loop */
    for( i = 0; i < steps; i += 2 )
    {
        float temp0 = loop[i]->val + min_val;
        float temp1 = loop[i + 1]->val - min_val;

        loop[i]->val = temp0;
        loop[i + 1]->val = temp1;
    }

    /* remove the leaving basic variable */
    i = leave_x->i;
    j = leave_x->j;
    state->is_x[i][j] = 0;

    head.next[0] = state->rows_x[i];
    cur_x = &head;
    while( (next_x = cur_x->next[0]) != leave_x )
    {
        cur_x = next_x;
        assert( cur_x );
    }
    cur_x->next[0] = next_x->next[0];
    state->rows_x[i] = head.next[0];

    head.next[1] = state->cols_x[j];
    cur_x = &head;
    while( (next_x = cur_x->next[1]) != leave_x )
    {
        cur_x = next_x;
        assert( cur_x );
    }
    cur_x->next[1] = next_x->next[1];
    state->cols_x[j] = head.next[1];

    /* set enter_x to be the new empty slot */
    state->enter_x = leave_x;

    return CV_NO_ERR;
}



/****************************************************************************************\
*                                    icvFindLoop                                       *
\****************************************************************************************/
static int
icvFindLoop( CvEMDState * state )
{
    int i, steps = 1;
    CvNode2D *new_x;
    CvNode2D **loop = state->loop;
    CvNode2D *enter_x = state->enter_x, *_x = state->_x;
    char *is_used = state->is_used;

    memset( is_used, 0, state->ssize + state->dsize );

    new_x = loop[0] = enter_x;
    is_used[enter_x - _x] = 1;
    steps = 1;

    do
    {
        if( (steps & 1) == 1 )
        {
            /* find an unused x in the row */
            new_x = state->rows_x[new_x->i];
            while( new_x != 0 && is_used[new_x - _x] )
                new_x = new_x->next[0];
        }
        else
        {
            /* find an unused x in the column, or the entering x */
            new_x = state->cols_x[new_x->j];
            while( new_x != 0 && is_used[new_x - _x] && new_x != enter_x )
                new_x = new_x->next[1];
            if( new_x == enter_x )
                break;
        }

        if( new_x != 0 )        /* found the next x */
        {
            /* add x to the loop */
            loop[steps++] = new_x;
            is_used[new_x - _x] = 1;
        }
        else                    /* didn't find the next x */
        {
            /* backtrack */
            do
            {
                i = steps & 1;
                new_x = loop[steps - 1];
                do
                {
                    new_x = new_x->next[i];
                }
                while( new_x != 0 && is_used[new_x - _x] );

                if( new_x == 0 )
                {
                    is_used[loop[--steps] - _x] = 0;
                }
            }
            while( new_x == 0 && steps > 0 );

            is_used[loop[steps - 1] - _x] = 0;
            loop[steps - 1] = new_x;
            is_used[new_x - _x] = 1;
        }
    }
    while( steps > 0 );

    return steps;
}



/****************************************************************************************\
*                                        icvRussel                                     *
\****************************************************************************************/
static void
icvRussel( CvEMDState * state )
{
    int i, j, min_i = -1, min_j = -1;
    float min_delta, diff;
    CvNode1D u_head, *cur_u, *prev_u;
    CvNode1D v_head, *cur_v, *prev_v;
    CvNode1D *prev_u_min_i = 0, *prev_v_min_j = 0, *remember;
    CvNode1D *u = state->u, *v = state->v;
    int ssize = state->ssize, dsize = state->dsize;
    float eps = CV_EMD_EPS * state->max_cost;
    float **cost = state->cost;
    float **delta = state->delta;

    /* initialize the rows list (ur), and the columns list (vr) */
    u_head.next = u;
    for( i = 0; i < ssize; i++ )
    {
        u[i].next = u + i + 1;
    }
    u[ssize - 1].next = 0;

    v_head.next = v;
    for( i = 0; i < dsize; i++ )
    {
        v[i].val = -CV_EMD_INF;
        v[i].next = v + i + 1;
    }
    v[dsize - 1].next = 0;

    /* find the maximum row and column values (ur[i] and vr[j]) */
    for( i = 0; i < ssize; i++ )
    {
        float u_val = -CV_EMD_INF;
        float *cost_row = cost[i];

        for( j = 0; j < dsize; j++ )
        {
            float temp = cost_row[j];

            if( u_val < temp )
                u_val = temp;
            if( v[j].val < temp )
                v[j].val = temp;
        }
        u[i].val = u_val;
    }

    /* compute the delta matrix */
    for( i = 0; i < ssize; i++ )
    {
        float u_val = u[i].val;
        float *delta_row = delta[i];
        float *cost_row = cost[i];

        for( j = 0; j < dsize; j++ )
        {
            delta_row[j] = cost_row[j] - u_val - v[j].val;
        }
    }

    /* find the basic variables */
    do
    {
        /* find the smallest delta[i][j] */
        min_i = -1;
        min_delta = CV_EMD_INF;
        prev_u = &u_head;
        for( cur_u = u_head.next; cur_u != 0; cur_u = cur_u->next )
        {
            int i = cur_u - u;
            float *delta_row = delta[i];

            prev_v = &v_head;
            for( cur_v = v_head.next; cur_v != 0; cur_v = cur_v->next )
            {
                int j = cur_v - v;

                if( min_delta > delta_row[j] )
                {
                    min_delta = delta_row[j];
                    min_i = i;
                    min_j = j;
                    prev_u_min_i = prev_u;
                    prev_v_min_j = prev_v;
                }
                prev_v = cur_v;
            }
            prev_u = cur_u;
        }

        if( min_i < 0 )
            break;

        /* add x[min_i][min_j] to the basis, and adjust supplies and cost */
        remember = prev_u_min_i->next;
        icvAddBasicVariable( state, min_i, min_j, prev_u_min_i, prev_v_min_j, &u_head );

        /* update the necessary delta[][] */
        if( remember == prev_u_min_i->next )    /* line min_i was deleted */
        {
            for( cur_v = v_head.next; cur_v != 0; cur_v = cur_v->next )
            {
                int j = cur_v - v;

                if( cur_v->val == cost[min_i][j] )      /* column j needs updating */
                {
                    float max_val = -CV_EMD_INF;

                    /* find the new maximum value in the column */
                    for( cur_u = u_head.next; cur_u != 0; cur_u = cur_u->next )
                    {
                        float temp = cost[cur_u - u][j];

                        if( max_val < temp )
                            max_val = temp;
                    }

                    /* if needed, adjust the relevant delta[*][j] */
                    diff = max_val - cur_v->val;
                    cur_v->val = max_val;
                    if( fabs( diff ) < eps )
                    {
                        for( cur_u = u_head.next; cur_u != 0; cur_u = cur_u->next )
                        {
                            delta[cur_u - u][j] += diff;
                        }
                    }
                }
            }
        }
        else                    /* column min_j was deleted */
        {
            for( cur_u = u_head.next; cur_u != 0; cur_u = cur_u->next )
            {
                int i = cur_u - u;

                if( cur_u->val == cost[i][min_j] )      /* row i needs updating */
                {
                    float max_val = -CV_EMD_INF;

                    /* find the new maximum value in the row */
                    for( cur_v = v_head.next; cur_v != 0; cur_v = cur_v->next )
                    {
                        float temp = cost[i][cur_v - v];

                        if( max_val < temp )
                            max_val = temp;
                    }

                    /* if needed, adjust the relevant delta[i][*] */
                    diff = max_val - cur_u->val;
                    cur_u->val = max_val;

                    if( fabs( diff ) < eps )
                    {
                        for( cur_v = v_head.next; cur_v != 0; cur_v = cur_v->next )
                        {
                            delta[i][cur_v - v] += diff;
                        }
                    }
                }
            }
        }
    }
    while( u_head.next != 0 || v_head.next != 0 );
}



/****************************************************************************************\
*                                   icvAddBasicVariable                                *
\****************************************************************************************/
static void
icvAddBasicVariable( CvEMDState * state,
                     int min_i, int min_j,
                     CvNode1D * prev_u_min_i, CvNode1D * prev_v_min_j, CvNode1D * u_head )
{
    float temp;
    CvNode2D *end_x = state->end_x;

    if( state->s[min_i] < state->d[min_j] + state->weight * CV_EMD_EPS )
    {                           /* supply exhausted */
        temp = state->s[min_i];
        state->s[min_i] = 0;
        state->d[min_j] -= temp;
    }
    else                        /* demand exhausted */
    {
        temp = state->d[min_j];
        state->d[min_j] = 0;
        state->s[min_i] -= temp;
    }

    /* x(min_i,min_j) is a basic variable */
    state->is_x[min_i][min_j] = 1;

    end_x->val = temp;
    end_x->i = min_i;
    end_x->j = min_j;
    end_x->next[0] = state->rows_x[min_i];
    end_x->next[1] = state->cols_x[min_j];
    state->rows_x[min_i] = end_x;
    state->cols_x[min_j] = end_x;
    state->end_x = end_x + 1;

    /* delete supply row only if the empty, and if not last row */
    if( state->s[min_i] == 0 && u_head->next->next != 0 )
        prev_u_min_i->next = prev_u_min_i->next->next;  /* remove row from list */
    else
        prev_v_min_j->next = prev_v_min_j->next->next;  /* remove column from list */
}


/****************************************************************************************\
*                                  standard  metrics                                     *
\****************************************************************************************/
static float
icvDistL1( const float *x, const float *y, void *user_param )
{
    int i, dims = (int)(long)user_param;
    double s = 0;

    for( i = 0; i < dims; i++ )
    {
        double t = x[i] - y[i];

        s += fabs( t );
    }
    return (float)s;
}

static float
icvDistL2( const float *x, const float *y, void *user_param )
{
    int i, dims = (int)(long)user_param;
    double s = 0;

    for( i = 0; i < dims; i++ )
    {
        double t = x[i] - y[i];

        s += t * t;
    }
    return cvSqrt( (float)s );
}

static float
icvDistC( const float *x, const float *y, void *user_param )
{
    int i, dims = (int)(long)user_param;
    double s = 0;

    for( i = 0; i < dims; i++ )
    {
        double t = fabs( x[i] - y[i] );

        if( s < t )
            s = t;
    }
    return (float)s;
}

/* End of file. */

