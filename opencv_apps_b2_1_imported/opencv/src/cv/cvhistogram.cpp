/*M///////////////////////////////////////////////////////////////////////////////////////
//
//  IMPORTANT: READ BEFORE DOWNLOADING, COPYING, INSTALLING OR USING.
//
//  By downloading, copying, installing or using the software you agree to this license.
//  If you do not agree to this license, do not download, install,
//  copy or use the software.
//
//
//			  Intel License Agreement
//		  For Open Source Computer Vision Library
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

#if _MSC_VER >= 1200
#pragma warning( disable : 4201 )	/* nonstandard extension used : nameless struct/union */
#endif

#define LEFT_LEFT   ((CV_LEFT	<< 2) + CV_LEFT)
#define RIGHT_LEFT  ((CV_RIGHT	<< 2) + CV_LEFT)
#define LEFT_RIGHT  ((CV_LEFT	<< 2) + CV_RIGHT)
#define RIGHT_RIGHT ((CV_RIGHT	<< 2) + CV_RIGHT)
#define CENTER_LEFT  ((CV_CENTER  << 2) + CV_LEFT)
#define CENTER_RIGHT ((CV_CENTER  << 2) + CV_RIGHT)

typedef enum
{
    CV_LEFT = 0,
    CV_RIGHT = 1,
    CV_CENTER = 2
}
CvNodeDirection;

/* Flags for histogram */
typedef struct CvNode
{
    union
    {
	struct CvNode *link[2];
	struct
	{
	    struct CvNode *left;
	    struct CvNode *right;
	}
	u;
    };
    struct CvNode *up;
    float value;
    int64 index;
}
CvNode;

#define _LINK(node) ((CvNode*)((long)(node) & ~3))
#define _BALANCE(node) ((long)(node) & 3)
#define _UP(link,balance) (CvNode*)(((long)(link) & ~3) + (balance))

static CvNode *
icvBeginIter( CvNode * root )
{
    if( !root )
	return 0;
    while( root->u.left )
	root = root->u.left;
    assert( !((int) root->u.left & 3) );
    return root;
}

static CvNode *
icvNextIter( CvNode * node )
{
    if( node->link[CV_RIGHT] )
	return icvBeginIter( node->link[CV_RIGHT] );

    while( _LINK( node->up ) && _LINK( node->up )->index < node->index )
	node = _LINK( node->up );
    return _LINK( node->up );
}


/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:	  icvFillHistHeader
//    Purpose:	  Filling histogram header (no allocate data)
//    Context:
//    Parameters:
//	c_dims - number of dimensions
//	dims   - dimensions
//	hist   - pointer to header
//    Returns:
//    Notes:
//F*/
static CvStatus
icvFillHistHeader( int c_dims, int *dims, CvHistType type, CvHistogram * hist )
{
    int i;
    int num = 1;

    if( !hist )
	return CV_NULLPTR_ERR;
    hist->header_size = sizeof( CvHistogram );
    hist->type = type;

    for( i = 0; i < CV_HIST_MAX_DIM; i++ )
	hist->dims[i] = hist->mdims[i] = 0;
    for( i = 0; i < c_dims; i++ )
    {
	hist->dims[i] = dims[i];
	hist->mdims[c_dims - i - 1] = num;
	num *= dims[c_dims - i - 1];
    }

    hist->c_dims = c_dims;

    return CV_NO_ERR;
}

/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:	  cvCreateHist
//    Purpose:	  Creating
//    Context:
//    Parameters:
//
//    Returns:
//    Notes:
//F*/
CvHistogram *
cvCreateHist( int c_dims, int *dims, CvHistType type, float** ranges, int uniform )
{
    CvHistogram *hist = 0;

    CV_FUNCNAME( "cvCreateHist" );
    __BEGIN__;

    if( !c_dims || c_dims > CV_HIST_MAX_DIM )
	CV_ERROR( IPL_BadOrder, "Too many dimensions" );
    if( !dims )
	CV_ERROR( IPL_HeaderIsNull, 0 );

    hist = (CvHistogram *) icvAlloc( sizeof( CvHistogram ));
    if( !hist )
	CV_ERROR( IPL_StsNoMem, "Out Of Memory" );

    memset( hist, 0, sizeof( *hist ));

    hist->flags = CV_HIST_MEMALLOCATED | CV_HIST_HEADERALLOCATED;

    IPPI_CALL( icvFillHistHeader( c_dims, dims, type, hist ) );

    if( type == CV_HIST_ARRAY )
    {
	hist->array = (float *) icvAlloc( (hist->mdims[0] * hist->dims[0] + 1) *
					  sizeof( hist->array ));
	if( !hist->array )
	    CV_ERROR( IPL_StsNoMem, "Out Of Memory" );
    }
    else
    {
	assert( type == CV_HIST_TREE );
	CvMemStorage *storage = cvCreateMemStorage( 0 );

	hist->set = (CvSet *) cvCreateSet( 0, sizeof( CvSet ), sizeof( CvNode ), storage );
    }

    if( ranges )
	cvSetHistBinRanges( hist, ranges, uniform );

    
    __END__;

    return hist;
}


/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:	  cvMakeHistHeaderForArray
//    Purpose:	  Initializes histogram header and sets
//		  its data pointer to given value
//    Context:
//    Parameters:
//	c_dims - number of dimension in the histogram
//	dims   - array, containing number of bins per each dimension
//	hist   - pointer to histogram structure. It will have CV_HIST_ARRAY type.
//	data   - histogram data
//    Returns:
//	CV_NO_ERR if ok, error code else
//    Notes:
//F*/
CV_IMPL void
cvMakeHistHeaderForArray( int c_dims, int *dims, CvHistogram *hist,
			  float *data, float **ranges, int uniform )
{
    CV_FUNCNAME( "cvMakeHistHeaderForArray" );
    __BEGIN__;

    if( !hist )
	CV_ERROR( IPL_HeaderIsNull, 0 );
    if( hist->type != CV_HIST_ARRAY )
	CV_ERROR( IPL_BadOrder, "Unsupported format" );
    hist->array = data;
    hist->flags = 0;
    CV_CALL( icvFillHistHeader( c_dims, dims, CV_HIST_ARRAY, hist ));

    if( ranges )
	cvSetHistBinRanges( hist, ranges, uniform );

    
    __END__;
}


/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:	  cvReleaseHist
//    Purpose:	  Releases histogram header and underlying data
//    Context:
//    Parameters:
//	hist - pointer to released histogram.
//    Returns:
//	CV_NO_ERR if ok, error code else
//    Notes:
//F*/
CV_IMPL void
cvReleaseHist( CvHistogram **hist )
{
    int i;

    CV_FUNCNAME( "cvReleaseHist" );
    __BEGIN__;

    if( !hist )
	CV_ERROR( IPL_HeaderIsNull, 0 );
    if( *hist )
    {
	if( (*hist)->type == CV_HIST_ARRAY )
	{
	    if( ((*hist)->flags & CV_HIST_MEMALLOCATED) && (*hist)->array )
		icvFree( &(*hist)->array );
	}
	else if( ((*hist)->flags & CV_HIST_MEMALLOCATED) && (*hist)->set )
	{
	    CvMemStorage *storage = (*hist)->set->storage;

	    cvReleaseMemStorage( &storage );
	}

	if( (*hist)->flags & CV_HIST_THRESHALLOCATED )
	    for( i = 0; i < (*hist)->c_dims; i++ )
		if( (*hist)->thresh[i] )
		    icvFree( &(*hist)->thresh[i] );
	if( ((*hist)->flags & CV_HIST_THRESHALLOCATED) && (*hist)->chdims[0] )
	    icvFree( &(*hist)->chdims[0] );
	if( (*hist)->flags & CV_HIST_HEADERALLOCATED )
	    icvFree( hist );
    }

    
    __END__;
}


/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:	icvThreshHist
//    Purpose:	Clears histogram bins that are below specified level
//    Context:
//    Parameters:
//	hist - pointer to histogram.
//	thresh - threshold level
//    Returns:
//	CV_NO_ERR or error code
//    Notes:
//F*/
CV_IMPL void
cvThreshHist( CvHistogram * hist, double thresh )
{
    int size;

    CV_FUNCNAME( "cvThreshHist" );
    __BEGIN__;

    if( !hist )
	CV_ERROR( IPL_HeaderIsNull, 0 );

    if( hist->type == CV_HIST_ARRAY )
    {
	size = hist->dims[0] * hist->mdims[0];
	CV_CALL( icvThresh_32f_C1R( hist->array, size * 4, hist->array, size * 4,
				     cvSize( size, 1 ), (float)thresh, 0, CV_THRESH_TOZERO ));
    }
    else
    {
	CvNode *node = 0;

	if( !hist->set || !hist->root )
	    return;
	node = icvBeginIter( hist->root );
	do
	{
	    if( node->value <= thresh )
		node->value = 0;
	}
	while( (node = icvNextIter( node )) != 0 );
    }
    
    __END__;
}


/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:	cvNormalizeHist
//    Purpose:	Normalizes histogram (such that sum of histogram bins becomes factor)
//    Context:
//    Parameters:
//	hist - pointer to normalized histogram.
//    Returns:
//	CV_NO_ERR or error code
//    Notes:
//F*/
CV_IMPL void
cvNormalizeHist( CvHistogram * hist, double factor )
{
    double sum = 0;

    CV_FUNCNAME( "cvNormalizeHist" );
    __BEGIN__;

    if( !hist )
	CV_ERROR( IPL_HeaderIsNull, 0 );

    if( hist->type == CV_HIST_ARRAY )
    {
	int size = hist->dims[0] * hist->mdims[0];

	sum = icvSum_32f( hist->array, size );

	if( !sum )
	    return;

	CV_CALL( icvScaleVector_32f
		 ( hist->array, hist->array, size, (float) (factor / sum) ));
    }
    else
    {
	CvNode *node = 0;
	CvNode *root = 0;

	sum = 0;
	if( !hist->set || !hist->root )
	    return;
	node = root = icvBeginIter( hist->root );
	do
	{
	    sum += node->value;
	}
	while( (node = icvNextIter( node )) != 0 );

	if( !sum )
	    EXIT;

	node = root;
	factor /= sum;

	do
	{
	    node->value = (float)(node->value*factor);
	}
	while( (node = icvNextIter( node )) != 0 );
    }

    
    __END__;
}

#define SET_NODE(node,left,right,_up,_value,index)  \
    (node)->link[CV_LEFT]  = left;		    \
    (node)->link[CV_RIGHT] = right;		    \
    (node)->up		     = _up;		    \
    (node)->value	     = _value;		    \
    (node)->index	     = index;


static CvNode *
icvInsertNode( CvHistogram * hist, int64 index )
{
    CvNode *root = hist->root;
    CvNode *node = root;
    CvNode *last = 0;

    assert( (int64) hist->dims[0] * hist->mdims[0] > index );

    if( !hist->root )
    {
	cvSetAdd( hist->set, 0, (CvSetElem **) & hist->root );
	SET_NODE( hist->root, 0, 0, _UP( 0, CV_CENTER ), 0, index );

	return hist->root;
    }

    /* Find place where insert new element */
    while( node && node->index != index )
    {
	last = node;
	node = node->link[index > node->index];
    }

    /* if element present then return it */
    if( node )
	return node;

    cvSetAdd( hist->set, 0, (CvSetElem **) & node );
    SET_NODE( node, 0, 0, _UP( last, CV_CENTER ), 0, index );
    last->link[index > last->index] = node;
    last = node;
    node = _LINK( node->up );

    /* Rool back while not find wrong place or root node */
    while( node )
    {
	int balance = _BALANCE( node->up );

	assert( node );
	switch ((balance << 2) + (index > node->index))
	{
	case LEFT_LEFT:
	    assert( _BALANCE( node->u.left->up ) != CV_CENTER );
	    if( _BALANCE( node->u.left->up ))	/* u.right balance */
	    {
		CvNode *a = node;
		CvNode *b = node->u.left;
		CvNode *c = b->u.right;

		assert( a );
		assert( b );
		assert( c );
		b->u.right = c->u.left;
		a->u.left = c->u.right;
		c->u.left = b;
		c->u.right = a;
		node = a->up;
		b->up = _UP( c, (_BALANCE( c->up ) != CV_RIGHT) << 1 );
		a->up = _UP( c, 2 - !_BALANCE( c->up ));
		c->up = _UP( node, CV_CENTER );
		if( a->u.left )
		    a->u.left->up = _UP( a, _BALANCE( a->u.left->up ));
		if( b->u.right )
		    b->u.right->up = _UP( b, _BALANCE( b->u.right->up ));
		if( !_LINK( c->up ))
		    hist->root = c;
		else
		{
		    node = _LINK( c->up );
		    node->link[c->index > node->index] = c;
		}
	    }
	    else		/* u.left balance */
	    {
		CvNode *a = node;
		CvNode *b = node->u.left;

		assert( a );
		assert( b );
		a->u.left = b->u.right;
		b->u.right = a;
		b->up = _UP( a->up, CV_CENTER );
		a->up = _UP( b, CV_CENTER );
		if( a->u.left )
		    a->u.left->up = _UP( a, _BALANCE( a->u.left->up ));
		if( !_LINK( b->up ))
		    hist->root = b;
		else
		{
		    node = _LINK( b->up );
		    node->link[b->index > node->index] = b;
		}
	    }
	    return last;

	case LEFT_RIGHT:
	case RIGHT_LEFT:
	    node->up = _UP( node->up, CV_CENTER );
	    return last;

	case CENTER_LEFT:
	    node->up = _UP( node->up, CV_LEFT );
	    break;

	case CENTER_RIGHT:
	    node->up = _UP( node->up, CV_RIGHT );
	    break;

	case RIGHT_RIGHT:
	    assert( _BALANCE( node->u.right->up ) != CV_CENTER );
	    if( !_BALANCE( node->u.right->up )) /* u.left balance */
	    {
		CvNode *a = node;
		CvNode *b = node->u.right;
		CvNode *c = b->u.left;

		assert( a );
		assert( b );
		assert( c );
		a->u.right = c->u.left;
		b->u.left = c->u.right;
		c->u.left = a;
		c->u.right = b;
		node = a->up;
		a->up = _UP( c, (_BALANCE( c->up ) != CV_RIGHT) << 1 );
		b->up = _UP( c, 2 - !_BALANCE( c->up ));
		c->up = _UP( node, CV_CENTER );
		if( a->u.right )
		    a->u.right->up = _UP( a, _BALANCE( a->u.right->up ));
		if( b->u.left )
		    b->u.left->up = _UP( b, _BALANCE( b->u.left->up ));
		if( !_LINK( c->up ))
		    hist->root = c;
		else
		{
		    node = _LINK( c->up );
		    node->link[c->index > node->index] = c;
		}
	    }
	    else		/* u.right balance */
	    {
		CvNode *a = node;
		CvNode *b = node->u.right;

		assert( a );
		assert( b );
		a->u.right = b->u.left;
		b->u.left = a;
		b->up = _UP( a->up, CV_CENTER );
		a->up = _UP( b, CV_CENTER );
		if( a->u.right )
		    a->u.right->up = _UP( a, _BALANCE( a->u.right->up ));
		if( !_LINK( b->up ))
		    hist->root = b;
		else
		{
		    node = _LINK( b->up );
		    node->link[b->index > node->index] = b;
		}
	    }
	    return last;

	default:
	    assert( 0 );
	    return last;
	}
	assert( _BALANCE( node->up ) != CV_CENTER );
	node = _LINK( node->up );
    }
    return last;
}


/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:	  cvGetHistValue....
//    Purpose:	  Returns pointer to histogram bin, given its cooridinates
//    Context:
//    Parameters:
//	hist - pointer to histogram.
//	idx0 - index for the 1st dimension
//	idx1 - index for the 2nd dimension
//	       ...
//	idx  - array of coordinates (for multi-dimensonal histogram).
//	       must have hist->c_dims elements.
//    Returns:
//	Pointer to histogram bin
//    Notes:
//	For non-array histogram function creates a new element if it is not exists.
//F*/
CV_IMPL float*
cvGetHistValue_1D( CvHistogram * hist, int idx0 )
{
    float* node = 0;

    CV_FUNCNAME( "cvGetHistValue_1D" );
    
    __BEGIN__;
    
    if( !hist )
	CV_ERROR( IPL_HeaderIsNull, 0 );
	
    if( hist->type == CV_HIST_ARRAY )
	node = &hist->array[idx0];
    else
	node = &icvInsertNode( hist, idx0 )->value;
    
    __END__;

    return node;
}

CV_IMPL float *
cvGetHistValue_2D( CvHistogram * hist, int idx0, int idx1 )
{
    float* node = 0;

    CV_FUNCNAME( "cvGetHistValue_2D" );
    
    __BEGIN__;
    
    if( !hist )
	CV_ERROR( IPL_HeaderIsNull, 0 );
    if( hist->type == CV_HIST_ARRAY )
	node = &hist->array[idx0 * hist->mdims[0] + idx1];
    else
	node = &icvInsertNode( hist, (int64) idx0 * hist->mdims[1] + idx1 )->value;
    
    __END__;
    
    return node;
}

CV_IMPL float *
cvGetHistValue_3D( CvHistogram * hist, int idx0, int idx1, int idx2 )
{
    float* node = 0;

    CV_FUNCNAME( "cvGetHistValue_3D" );
    
    __BEGIN__;
    
    if( !hist )
	CV_ERROR( IPL_HeaderIsNull, 0 );
	
    if( hist->type == CV_HIST_ARRAY )
	node = &hist->array[idx0 * hist->mdims[0] + idx1 * hist->mdims[1] + idx2];
    else
	node = &icvInsertNode( hist, (int64) idx0 * hist->mdims[0] +
			       idx1 * hist->mdims[1] + idx2 )->value;
    
    __END__;
    
    return node;
}

CV_IMPL float *
cvGetHistValue_nD( CvHistogram * hist, int *idx )
{
    float* node = 0;

    CV_FUNCNAME( "cvGetHistValue_nD" );
    __BEGIN__;
    int i;
    int64 address = 0;

    if( !hist )
	CV_ERROR( IPL_HeaderIsNull, 0 );
    if( !idx )
	CV_ERROR( IPL_HeaderIsNull, 0 );
    for( i = 0; i < hist->c_dims; i++ )
	address += (int64) hist->mdims[i] * (int64) idx[i];
    assert( address < (int64) hist->mdims[0] * (int64) hist->dims[0] );
    if( hist->type == CV_HIST_ARRAY )
	node = hist->array + address;
    else
	node = &icvInsertNode( hist, address )->value;
    
    __END__;
    
    return node;
}


static CvNode *
icvFindNode( CvNode * root, int64 index )
{
    while( root && root->index != index )
	root = root->link[index > root->index];
    return root;
}


/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:	  cvQueryHistValue....
//    Purpose:	  Returns value or histogram bin, given its cooridinates
//    Context:
//    Parameters:
//	hist - pointer to histogram.
//	idx0 - index for the 1st dimension
//	idx1 - index for the 2nd dimension
//	       ...
//	idx  - array of coordinates (for multi-dimensonal histogram)
//    Returns:
//	Value of histogram bin
//    Notes:
//	For non-array histogram function returns 0 if the specified element isn't present
//F*/
CV_IMPL float
cvQueryHistValue_1D( CvHistogram * hist, int idx0 )
{
    float val = 0;

    CV_FUNCNAME( "cvQueryHistValue_1D" );
    
    __BEGIN__;
    
    if( !hist )
	CV_ERROR( IPL_HeaderIsNull, 0 );
	
    if( hist->type == CV_HIST_ARRAY )
    {
	float *res = cvGetHistValue_1D( hist, idx0 );

	if( res )
	    val = *res;
    }
    else
    {
	CvNode *node = icvFindNode( hist->root, idx0 );

	if( node )
	    val = node->value;
    }
    
    __END__;
    
    return val;    
}

float
cvQueryHistValue_2D( CvHistogram * hist, int idx0, int idx1 )
{
    float val = 0;

    CV_FUNCNAME( "cvQueryHistValue_2D" );
    __BEGIN__;

    if( !hist )
	CV_ERROR( IPL_HeaderIsNull, 0 );
    if( hist->type == CV_HIST_ARRAY )
    {
	float *res = cvGetHistValue_2D( hist, idx0, idx1 );

	if( res )
	    val = *res;
    }
    else
    {
	CvNode *node = icvFindNode( hist->root, (int64) idx0 * hist->mdims[0] + idx1 );

	if( node )
	    val = node->value;
    }
    __END__;
    
    return val;
}

CV_IMPL float
cvQueryHistValue_3D( CvHistogram * hist, int idx0, int idx1, int idx2 )
{
    float val = 0;

    CV_FUNCNAME( "cvQueryHistValue_3D" );
    
    __BEGIN__;
    
    if( !hist )
	CV_ERROR( IPL_HeaderIsNull, 0 );
	
    if( hist->type == CV_HIST_ARRAY )
    {
	float *res = cvGetHistValue_3D( hist, idx0, idx1, idx2 );

	if( res )
	    val = *res;
    }
    else
    {
	CvNode *node = icvFindNode( hist->root, (int64) idx0 * hist->mdims[0] +
				    idx1 * hist->mdims[1] + idx2 );

	if( node )
	    val = node->value;
    }
    
    __END__;	
    return val;
}

CV_IMPL float
cvQueryHistValue_nD( CvHistogram * hist, int *idx )
{
    float val = 0;

    CV_FUNCNAME( "cvQueryHistValue_nD" );
    __BEGIN__;
    
    if( !hist )
	CV_ERROR( IPL_HeaderIsNull, 0 );
    if( hist->type == CV_HIST_ARRAY )
    {
	float *res = cvGetHistValue_nD( hist, idx );

	if( res )
	    val = *res;
    }
    else
    {
	int64 address = 0;
	int i;
	CvNode *node = 0;

	for( i = 0; i < hist->c_dims; i++ )
	    address += hist->mdims[i] * idx[i];
	node = icvFindNode( hist->root, address );
	if( node )
	    val = node->value;
    }
    
    __END__;
    return val;    
}


/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:	  icvGetMinMaxHistValue
//    Purpose:	  Finds coordinates and numerical values of minimum and maximum
//		  histogram bins
//    Context:
//    Parameters:
//	hist - pointer to histogram.
//	idx_min - pointer to array of coordinates for minimum.
//		  if not NULL, must have hist->c_dims elements.
//	value_min - pointer to minimum value of histogram (can be NULL).
//	idx_max - pointer to array of coordinates for maximum.
//		  if not NULL, must have hist->c_dims elements.
//	value_max - pointer to maximum value of histogram (can be NULL).
//    Returns:
//	CV_NO_ERR or error code
//    Notes:
//F*/
CV_IMPL void
cvGetMinMaxHistValue( CvHistogram * hist, float *value_min, float* value_max,
		      int *idx_min, int *idx_max )
{
    int i;
    int size;

    float minValue = 0, maxValue = 0;
    CvPoint minp, maxp;
    int64 i_min = 0, i_max = 0;

    CV_FUNCNAME("cvGetMinMaxHistValue");

    __BEGIN__;

    if( !hist )
	CV_ERROR_FROM_STATUS( CV_NULLPTR_ERR );

    if( hist->type == CV_HIST_ARRAY )
    {
	size = hist->dims[0] * hist->mdims[0];

	IPPI_CALL( icvMinMaxIndx_32f_C1R( hist->array, size * sizeof_float,
					  cvSize( size, 1 ), &minValue, &maxValue,
					  &minp, &maxp ));
	i_min = minp.x;
	i_max = maxp.x;
    }
    else
    {
	CvNode *node = icvBeginIter( hist->root );

	if( node )
	{
	    minValue = maxValue = node->value;
	    i_min = i_max = node->index;
	    while( (node = icvNextIter( node )) != 0 )
	    {
		if( node->value < minValue )
		{
		    minValue = node->value;
		    i_min = node->index;
		}
		if( node->value > maxValue )
		{
		    maxValue = node->value;
		    i_max = node->index;
		}
	    }
	}
    }

    if( value_min )
	*value_min = minValue;
    if( value_max )
	*value_max = maxValue;

    for( i = 0; i < hist->c_dims; i++ )
    {
	int _idx_min;
	int _idx_max;

	assert( hist->mdims[i] );

	_idx_min = (int) (i_min / hist->mdims[i]);
	_idx_max = (int) (i_max / hist->mdims[i]);

	if( idx_min )
	    idx_min[i] = _idx_min;
	if( idx_max )
	    idx_max[i] = _idx_max;

	i_min -= hist->mdims[i] * _idx_min;
	i_max -= hist->mdims[i] * _idx_max;
    }

    
    __END__;
}


CV_IMPL float
cvGetMeanHistValue( CvHistogram * hist )
{
    float sum = 0;
    
    CV_FUNCNAME( "cvGetMeanHistValue" );
    
    __BEGIN__;
    
    int size;

    if( !hist )
	CV_ERROR( IPL_HeaderIsNull, 0 );

    size = hist->mdims[0] * hist->dims[0];

    if( hist->type == CV_HIST_ARRAY )
    {
	int i;

	for( i = 0; i < size; i++ )
	    sum += hist->array[i];
    }
    else
    {
	CvNode *node = icvBeginIter( hist->root );

	while( node )
	{
	    sum += node->value;
	    node = icvNextIter( node );
	}
    }

    if( size )
	sum /= size;

    __END__;

    return sum;
}


/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:	cvCompareHist
//    Purpose:	compares two histograms using specified method
//    Context:
//    Parameters:
//	hist1 - first compared histogram.
//	hist2 - second compared histogram.
//	method - comparison method
//    Returns:
//	value, that characterizes similarity (or difference) of two histograms
//    Notes:
//F*/
CV_IMPL double
cvCompareHist( CvHistogram * hist1, CvHistogram * hist2, CvCompareMethod method )
{
    int i;
    int size;
    double sum = 0;

    CV_FUNCNAME( "cvCompareHist" );
    __BEGIN__;

    if( !hist1 || !hist2 )
	CV_ERROR( IPL_HeaderIsNull, 0 );
    if( hist1->type != hist2->type )
	CV_ERROR( IPL_BadOrder, "Types are not equals" );
    if( hist1->c_dims != hist2->c_dims )
	CV_ERROR( IPL_BadOrder, "Dimensions are not equals" );
    for( i = 0; i < hist1->c_dims; i++ )
	if( hist1->dims[i] != hist2->dims[i] )
	    CV_ERROR( IPL_BadOrder, "Dimensions are not equals" );

    size = hist1->dims[0] * hist1->mdims[0];

    switch (method)
    {
    case CV_COMP_CHISQR:
	if( hist1->type == CV_HIST_ARRAY )
	{
	    for( i = 0; i < size; i++ )
		if( hist1->array[i] != -hist2->array[i] )
		    sum += (hist1->array[i] - hist2->array[i]) *
			(hist1->array[i] - hist2->array[i]) /
			(hist1->array[i] + hist2->array[i]);
	}
	else
	{
	    CvNode *node1 = icvBeginIter( hist1->root );
	    CvNode *node2 = icvBeginIter( hist2->root );

	    while( node1 || node2 )
	    {
		if( node1 && node2 )
		{
		    if( node1->index > node2->index )
		    {
			sum += node2->value;
			node2 = icvNextIter( node2 );
		    }
		    else if( node1->index < node2->index )
		    {
			sum += node1->value;
			node1 = icvNextIter( node1 );
		    }
		    else
		    {
			if( node1->value != -node2->value )
			    sum += (node1->value - node2->value) *
				(node1->value - node2->value) / (node1->value + node2->value);
			node1 = icvNextIter( node1 );
			node2 = icvNextIter( node2 );
		    }
		}
		else if( node1 )
		{
		    sum += node1->value;
		    node1 = icvNextIter( node1 );
		}
		else
		{
		    sum += node2->value;
		    node2 = icvNextIter( node2 );
		}
	    }
	}
	break;

    case CV_COMP_CORREL:
	if( hist1->type == CV_HIST_ARRAY )
	{
	    double sum1 = 0, sum2 = 0, sum3 = 0;
	    double v1, v2;
	    float mean1 = cvGetMeanHistValue( hist1 );
	    float mean2 = cvGetMeanHistValue( hist2 );

	    for( i = 0; i < size; i++ )
	    {
		v1 = hist1->array[i] - mean1;
		v2 = hist2->array[i] - mean2;

		sum1 += v1 * v2;
		sum2 += v1 * v1;
		sum3 += v2 * v2;
	    }

	    assert( sum2 >= 0 && sum3 >= 0 );
	    sum2 *= sum3;

	    sum = sum2 == 0 ? 1 : sum1 / sqrt( sum2 );
	}
	else
	{
	    float mean1 = cvGetMeanHistValue( hist1 );
	    float mean2 = cvGetMeanHistValue( hist2 );
	    CvNode *node1 = icvBeginIter( hist1->root );
	    CvNode *node2 = icvBeginIter( hist2->root );
	    double sum1 = 0, sum2 = 0, sum3 = 0;
	    double v1, v2;

	    while( node1 || node2 )
	    {
		if( node1 && node2 )
		{
		    if( node1->index > node2->index )
			node2 = icvNextIter( node2 );
		    else if( node1->index < node2->index )
			node1 = icvNextIter( node1 );
		    else
		    {
			v1 = node1->value - mean1;
			v2 = node2->value - mean2;

			sum1 += v1 * v2;
			sum2 += v1 * v1;
			sum3 += v2 * v2;

			node1 = icvNextIter( node1 );
			node2 = icvNextIter( node2 );
		    }
		}
		else if( node1 )
		    node1 = icvNextIter( node1 );
		else
		    node2 = icvNextIter( node2 );
	    }

	    assert( sum2 >= 0 && sum3 >= 0 );
	    sum2 *= sum3;

	    sum = sum2 == 0 ? 1 : sum1 / sqrt( sum2 );
	}
	break;

    case CV_COMP_INTERSECT:
	if( hist1->type == CV_HIST_ARRAY )
	{
	    for( i = 0; i < size; i++ )
		sum += MIN( hist1->array[i], hist2->array[i] );
	}
	else
	{
	    CvNode *node1 = icvBeginIter( hist1->root );
	    CvNode *node2 = icvBeginIter( hist2->root );

	    while( node1 || node2 )
	    {
		if( node1 && node2 )
		{
		    if( node1->index > node2->index )
		    {
			sum += MIN( node2->value, 0 );
			node2 = icvNextIter( node2 );
		    }
		    else if( node1->index < node2->index )
		    {
			sum += MIN( node1->value, 0 );
			node1 = icvNextIter( node1 );
		    }
		    else
		    {
			sum += MIN( node1->value, node2->value );
			node1 = icvNextIter( node1 );
			node2 = icvNextIter( node2 );
		    }
		}
		else if( node1 )
		{
		    sum += MIN( node1->value, 0 );
		    node1 = icvNextIter( node1 );
		}
		else
		{
		    sum += MIN( node2->value, 0 );
		    node2 = icvNextIter( node2 );
		}
	    }
	}
	break;

    default:
	sum = DBL_MAX;
	CV_ERROR( IPL_BadOrder, "Wrong method" );
    }

    __END__;

    return sum;
}


/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:	  cvCopyHist
//    Purpose:	  Copying one histogram to another
//    Context:
//    Parameters:
//    Returns:
//    Notes:	  if second parameter is pointer to NULL (*dst == 0) then second
//		  histogram will be created.
//		  both histograms (if second histogram present) must be equal
//		  types & sizes
//F*/
CV_IMPL void
cvCopyHist( CvHistogram * src, CvHistogram ** dst )
{
    CV_FUNCNAME( "cvCopyHist" );
    __BEGIN__;

    CvHistogram *_dst;
    int i;
    int size;

    if( !src || !dst )
	CV_ERROR( IPL_HeaderIsNull, 0 );

    if( !*dst )
	*dst = _dst = cvCreateHist( src->c_dims, src->dims, src->type );
    else
    {
	_dst = *dst;
	if( src->type != _dst->type )
	    CV_ERROR( IPL_BadOrder, "Types are not equal" );
	if( src->c_dims != _dst->c_dims )
	    CV_ERROR( IPL_BadOrder, "Dimensions are not equal" );
	for( i = 0; i < src->c_dims; i++ )
	    if( src->dims[i] != _dst->dims[i] )
		CV_ERROR( IPL_BadOrder, "Dimensions are not equal" );
    }

    if( src->flags & CV_HIST_THRESHALLOCATED )
    {
	if( !_dst->thresh[0] )
	{
	    for( i = 0; i < _dst->c_dims; i++ )
		_dst->thresh[i] = (float *) icvAlloc( (_dst->dims[i] + 1) * sizeof( float ));

	    _dst->flags |= CV_HIST_THRESHALLOCATED;
	}
	for( i = 0; i < _dst->c_dims; i++ )
	    icvCopyVector( src->thresh[i], _dst->thresh[i], src->dims[i] + 1 );
	_dst->flags |= src->flags & CV_HIST_UNIFORM;
    }

    size = src->mdims[0] * src->dims[0];
    if( src->type == CV_HIST_ARRAY )
	icvCopyVector( src->array, _dst->array, size );
    else
    {
	CvNode *node = icvBeginIter( src->root );

	if( _dst->set )
	    cvClearSet( _dst->set );

	while( node )
	{
	    icvInsertNode( _dst, node->index )->value = node->value;
	    node = icvNextIter( node );
	}
    }

    if( src->chdims[0] )
    {
	if( _dst->chdims[0] )
	    icvFree( &_dst->chdims[0] );
	_dst->chdims[0] = (int *) icvAlloc( (128 + 256) * sizeof( int ) * _dst->c_dims );

	for( i = 1; i < _dst->c_dims; i++ )
	    _dst->chdims[i] = _dst->chdims[i - 1] + (128 + 256);
	icvCopyVector_32f( (float *) src->chdims[0], (128 + 256) * _dst->c_dims,
			    (float *) _dst->chdims[0] );
    }
    
    __END__;
}


/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:	  cvSetHistBinRanges
//    Purpose:	  Setting threshold values to histogram
//    Context:
//    Parameters:
//    Returns:
//    Notes:	  if even parameter is not NULL then thresh[i][0] - minimum value,
//		  thresh[i][1] - maximum value of thresholds for dimension i
//F*/
CV_IMPL void
cvSetHistBinRanges( CvHistogram* hist, float** ranges, int uniform )
{
    int i, j, k;

    CV_FUNCNAME( "cvSetHistBinRanges" );
    __BEGIN__;

    if( !hist || !ranges )
	CV_ERROR( IPL_HeaderIsNull, 0 );

    for( i = 0; i < hist->c_dims; i++ )
	if( !ranges[i] )
	    CV_ERROR( IPL_HeaderIsNull, 0 );

    hist->flags |= CV_HIST_THRESHALLOCATED;

    if( !uniform )
    {
	for( i = 0; i < hist->c_dims; i++ )
	{
	    if( !hist->thresh[i] )
		hist->thresh[i] =
		    (float *) icvAlloc( (hist->dims[i] + 1) * sizeof( **hist->thresh ));
	    for( j = 0; j <= hist->dims[i]; j++ )
		hist->thresh[i][j] = ranges[i][j];
	}
	hist->flags &= ~CV_HIST_UNIFORM;
    }
    else
    {
	for( i = 0; i < hist->c_dims; i++ )
	{
	    float step = (ranges[i][1] - ranges[i][0]) / (hist->dims[i]);

	    if( !hist->thresh[i] )
		hist->thresh[i] =
		    (float *) icvAlloc( (hist->dims[i] + 1) * sizeof( **hist->thresh ));
	    hist->thresh[i][0] = ranges[i][0];
	    hist->thresh[i][hist->dims[i]] = ranges[i][1];
	    for( j = 1; j < hist->dims[i]; j++ )
		hist->thresh[i][j] = step * j + ranges[i][0];
	}
	hist->flags |= CV_HIST_UNIFORM;
    }

    if( hist->chdims[0] )
	icvFree( &hist->chdims[0] );
    hist->chdims[0] = (int *) icvAlloc( (128 + 256) * sizeof( int ) * hist->c_dims );

    for( i = 1; i < hist->c_dims; i++ )
	hist->chdims[i] = hist->chdims[i - 1] + (128 + 256);

    for( k = -128; k < 256; k++ )
	for( i = 0; i < hist->c_dims; i++ )
	{
	    if( k < hist->thresh[i][0] || k >= hist->thresh[i][hist->dims[i]] )
		hist->chdims[i][k + 128] =
		    hist->c_dims == 1 ? hist->dims[0] :
		    hist->c_dims == 2 && i == 0 ? (hist->dims[1] + 1) * hist->dims[0] :
		    hist->c_dims == 2 && i == 1 ? hist->dims[1] : -1;
	    else
	    {
		for( j = 1; j < hist->dims[i]; j++ )
		    if( k < hist->thresh[i][j] )
			break;
		hist->chdims[i][k + 128] =
		    (hist->c_dims == 2 && i == 0) ? (hist->mdims[0] + 1) * (j - 1) :
		    (hist->c_dims == 2 && i == 1) ? (j - 1) : (j - 1) * hist->mdims[i];
	    }
	}

    
    __END__;
}


/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:	  icvCalcHist...C1R
//    Purpose:	  Calculating histogram from array of one-channel images
//    Context:
//    Parameters:
//    Returns:
//    Notes:	  if dont_clear parameter is NULL then histogram clearing before
//		  calculating (all values sets to NULL)
//F*/
IPCVAPI_IMPL( CvStatus, icvCalcHist8uC1R, (uchar ** img, int step, CvSize size,
					   CvHistogram * hist, int dont_clear) )
{
    int i, j, x = 0, y = 0;
    int dims;

    if( !hist || !img )
	return CV_NULLPTR_ERR;

    dims = hist->c_dims;

    for( i = 0; i < dims; i++ )
	if( !img[i] )
	    return CV_NULLPTR_ERR;

    for( i = 0; i < hist->c_dims; i++ )
    {
	if( !hist->thresh[i] )
	    return CV_NULLPTR_ERR;
	assert( hist->chdims[i] );
    }

    j = hist->dims[0] * hist->mdims[0];

    if( hist->type == CV_HIST_ARRAY )
    {
	if( !dont_clear )
	    for( i = 0; i < j; i++ )
		hist->array[i] = 0;

	switch (hist->c_dims)
	{
	case 1:
	    {
		uchar *data0 = img[0];
		int *array = (int *) hist->array;
		int *chdims = hist->chdims[0];

		for( i = 0; i < j; i++ )
		    array[i] = cvRound( hist->array[i] );

		for( y = 0; y < size.height; y++, data0 += step )
		{
		    for( x = 0; x <= size.width - 4; x += 4 )
		    {
			int val0 = chdims[data0[x] + 128];
			int val1 = chdims[data0[x + 1] + 128];

			array[val0]++;
			array[val1]++;
			val0 = chdims[data0[x + 2] + 128];
			val1 = chdims[data0[x + 3] + 128];
			array[val0]++;
			array[val1]++;
		    }
		    for( ; x < size.width; x++ )
			array[chdims[data0[x] + 128]]++;
		}
		for( i = 0; i < j; i++ )
		    hist->array[i] = (float) array[i];
	    }
	    break;
	case 2:
	    if( (hist->dims[1] + 1) * (hist->dims[0] + 1) < 4096 )
	    {
		uchar *data0 = img[0];
		uchar *data1 = img[1];
		int hstep = hist->dims[1] + 1;
		int array[4096];
		int chdims0[256];
		int chdims1[256];

		memcpy( chdims0, hist->chdims[0] + 128, 256 * sizeof( int ));
		memcpy( chdims1, hist->chdims[1] + 128, 256 * sizeof( int ));

		memset( array, 0, hstep * (hist->mdims[0] + 1) * sizeof( *array ));

		for( y = 0; y < size.height; y++, data0 += step, data1 += step )
		{
		    for( x = 0; x < size.width - 2; x += 2 )
		    {
			int val0 = chdims0[data0[x]];
			int val1 = chdims1[data1[x]];

			array[val0 + val1]++;
			val0 = chdims0[data0[x + 1]];
			val1 = chdims1[data1[x + 1]];
			array[val0 + val1]++;
		    }
		    for( ; x < size.width; x++ )
		    {
			int val0 = chdims0[data0[x]];
			int val1 = chdims1[data1[x]];

			array[val0 + val1]++;
		    }
		}
		for( y = 0, i = 0; y < hist->dims[0]; y++ )
		    for( x = 0; x < hist->dims[1]; x++, i++ )
			hist->array[i] += (float) array[y * hstep + x];
	    }
	    else
	    {
		uchar *data0 = img[0];
		uchar *data1 = img[1];
		int hstep = hist->dims[1] + 1;
		int *array =
		    (int *) icvAlloc( (hist->dims[1] + 1) * (hist->dims[0] + 1) *

				      sizeof( *array ));
		int chdims0[256];
		int chdims1[256];

		memcpy( chdims0, hist->chdims[0] + 128, 256 * sizeof( int ));
		memcpy( chdims1, hist->chdims[1] + 128, 256 * sizeof( int ));

		memset( array, 0, hstep * (hist->mdims[0] + 1) * sizeof( *array ));

		for( y = 0; y < size.height; y++, data0 += step, data1 += step )
		{
		    for( x = 0; x < size.width - 2; x += 2 )
		    {
			int val0 = chdims0[data0[x]];
			int val1 = chdims1[data1[x]];

			array[val0 + val1]++;
			val0 = chdims0[data0[x + 1]];
			val1 = chdims1[data1[x + 1]];
			array[val0 + val1]++;
		    }
		    for( ; x < size.width; x++ )
		    {
			int val0 = chdims0[data0[x]];
			int val1 = chdims1[data1[x]];

			array[val0 + val1]++;
		    }
		}
		for( y = 0, i = 0; y < hist->dims[0]; y++ )
		    for( x = 0; x < hist->dims[1]; x++, i++ )
			hist->array[i] += (float) array[y * hstep + x];
		icvFree( &array );
	    }
	    break;
	case 3:
	    {
		uchar *data0 = img[0];
		uchar *data1 = img[1];
		uchar *data2 = img[2];

		for( y = 0; y < size.height; y++, data0 += step, data1 += step, data2 += step )
		    for( x = 0; x < size.width; x++ )
		    {
			int val0 = hist->chdims[0][(int) data0[x] + 128];
			int val1 = hist->chdims[1][(int) data1[x] + 128];
			int val2 = hist->chdims[2][(int) data2[x] + 128];

			if( (val0 | val1 | val2) >= 0 )
			    hist->array[val0 + val1 + val2]++;
		    }
	    }
	    break;
	default:
	    {
		uchar *data[CV_HIST_MAX_DIM];

		for( i = 0; i < hist->c_dims; i++ )
		    data[i] = img[i];

		for( y = 0; y < size.height; y++ )
		{
		    for( x = 0; x < size.width; x++ )
		    {
			int addr = 0;

			for( i = 0; i < hist->c_dims; i++ )
			{
			    int val = hist->chdims[i][(int) data[i][x] + 128];

			    if( val >= 0 )
				addr += val;
			    else
				goto next;
			}
			hist->array[addr]++;
		      next:;
		    }
		    for( i = 0; i < hist->c_dims; i++ )
			data[i] += step;
		}
	    }
	    break;
	}
    }
    else
    {
	if( !dont_clear && hist->set )
	{
	    cvClearSet( hist->set );
	    hist->root = 0;
	}

	switch (hist->c_dims)
	{
	case 1:
	    {
		uchar *data0 = img[0];
		int *array = (int *) icvAlloc( j * sizeof( *array ));
		int *chdims = hist->chdims[0];

		memset( array, 0, j * sizeof( *array ));

		for( y = 0; y < size.height; y++, data0 += step )
		{
		    for( x = 0; x <= size.width - 4; x += 4 )
		    {
			int val0 = chdims[data0[x] + 128];
			int val1 = chdims[data0[x + 1] + 128];

			array[val0]++;
			array[val1]++;
			val0 = chdims[data0[x + 2] + 128];
			val1 = chdims[data0[x + 3] + 128];
			array[val0]++;
			array[val1]++;
		    }
		    for( ; x < size.width; x++ )
			array[chdims[data0[x] + 128]]++;
		}
		for( i = 0; i < j; i++ )
		    if( array[i] )
			icvInsertNode( hist, i )->value += (float) array[i];
		icvFree( &array );
	    }
	    break;
	case 2:
	    if( (hist->dims[1] + 1) * (hist->dims[0] + 1) < 4096 )
	    {
		uchar *data0 = img[0];
		uchar *data1 = img[1];
		int hstep = hist->dims[1] + 1;
		int array[4096];
		int chdims0[256];
		int chdims1[256];

		memcpy( chdims0, hist->chdims[0] + 128, 256 * sizeof( int ));
		memcpy( chdims1, hist->chdims[1] + 128, 256 * sizeof( int ));

		memset( array, 0, hstep * (hist->mdims[0] + 1) * sizeof( *array ));

		for( y = 0; y < size.height; y++, data0 += step, data1 += step )
		{
		    for( x = 0; x < size.width - 2; x += 2 )
		    {
			int val0 = chdims0[data0[x]];
			int val1 = chdims1[data1[x]];

			array[val0 + val1]++;
			val0 = chdims0[data0[x + 1]];
			val1 = chdims1[data1[x + 1]];
			array[val0 + val1]++;
		    }
		    for( ; x < size.width; x++ )
		    {
			int val0 = chdims0[data0[x]];
			int val1 = chdims1[data1[x]];

			array[val0 + val1]++;
		    }
		}
		for( y = 0, i = 0; y < hist->dims[0]; y++ )
		    for( x = 0; x < hist->dims[1]; x++, i++ )
		    {
			float val = (float) array[y * hstep + x];

			if( val )
			    icvInsertNode( hist, i )->value += val;
		    }
	    }
	    else
	    {
		uchar *data0 = img[0];
		uchar *data1 = img[1];
		int hstep = hist->dims[1] + 1;
		int *array =
		    (int *) icvAlloc( (hist->dims[1] + 1) * (hist->dims[0] + 1) *

				      sizeof( *array ));
		int chdims0[256];
		int chdims1[256];

		memcpy( chdims0, hist->chdims[0] + 128, 256 * sizeof( int ));
		memcpy( chdims1, hist->chdims[1] + 128, 256 * sizeof( int ));

		memset( array, 0, hstep * (hist->mdims[0] + 1) * sizeof( *array ));

		for( y = 0; y < size.height; y++, data0 += step, data1 += step )
		{
		    for( x = 0; x < size.width - 2; x += 2 )
		    {
			int val0 = chdims0[data0[x]];
			int val1 = chdims1[data1[x]];

			array[val0 + val1]++;
			val0 = chdims0[data0[x + 1]];
			val1 = chdims1[data1[x + 1]];
			array[val0 + val1]++;
		    }
		    for( ; x < size.width; x++ )
		    {
			int val0 = chdims0[data0[x]];
			int val1 = chdims1[data1[x]];

			array[val0 + val1]++;
		    }
		}
		for( y = 0, i = 0; y < hist->dims[0]; y++ )
		    for( x = 0; x < hist->dims[1]; x++, i++ )
		    {
			float val = (float) array[y * hstep + x];

			if( val )
			    icvInsertNode( hist, i )->value += val;
		    }
		icvFree( &array );
	    }
	    break;
	case 3:
	    {
		uchar *data0 = img[0];
		uchar *data1 = img[1];
		uchar *data2 = img[2];

		for( y = 0; y < size.height; y++, data0 += step, data1 += step, data2 += step )
		    for( x = 0; x < size.width; x++ )
		    {
			int val0 = hist->chdims[0][(int) data0[x] + 128];
			int val1 = hist->chdims[1][(int) data1[x] + 128];
			int val2 = hist->chdims[2][(int) data2[x] + 128];

			if( (val0 | val1 | val2) >= 0 )
			    icvInsertNode( hist, (int64) val0 + val1 + val2 )->value++;
		    }
	    }
	    break;
	default:
	    {
		uchar *data[CV_HIST_MAX_DIM];

		for( i = 0; i < hist->c_dims; i++ )
		    data[i] = img[i];

		for( y = 0; y < size.height; y++ )
		{
		    for( x = 0; x < size.width; x++ )
		    {
			int64 addr = 0;

			for( i = 0; i < hist->c_dims; i++ )
			{
			    int val = hist->chdims[i][(int) data[i][x] + 128];

			    if( val >= 0 )
				addr += val;
			    else
				goto next_tree;
			}
			icvInsertNode( hist, addr )->value++;
		      next_tree:;
		    }
		    for( i = 0; i < hist->c_dims; i++ )
			data[i] += step;
		}
	    }
	    break;
	}
    }

    return CV_NO_ERR;
}

/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:	  icvCalcHist...C1R
//    Purpose:	  Calculating histogram from array of one-channel images
//    Context:
//    Parameters:
//    Returns:
//    Notes:	  if dont_clear parameter is NULL then histogram clearing before
//		  calculating (all values sets to NULL)
//F*/
IPCVAPI_IMPL( CvStatus, icvCalcHist8sC1R, (char **img, int step, CvSize size,
					   CvHistogram * hist, int dont_clear) )
{
    int i, j, x = 0, y = 0;
    int dims;

    if( !hist || !img )
	return CV_NULLPTR_ERR;

    dims = hist->c_dims;

    for( i = 0; i < dims; i++ )
	if( !img[i] )
	    return CV_NULLPTR_ERR;

    for( i = 0; i < hist->c_dims; i++ )
    {
	if( !hist->thresh[i] )
	    return CV_NULLPTR_ERR;
	assert( hist->chdims[i] );
    }

    j = hist->dims[0] * hist->mdims[0];
    if( hist->type == CV_HIST_ARRAY )
    {
	if( !dont_clear )
	    for( i = 0; i < j; i++ )
		hist->array[i] = 0;

	switch (hist->c_dims)
	{
	case 1:
	    {
		char *data0 = img[0];
		int *array = (int *) hist->array;
		int *chdims = hist->chdims[0];

		for( i = 0; i < j; i++ )
		    array[i] = cvRound( hist->array[i] );

		for( y = 0; y < size.height; y++, data0 += step )
		{
		    for( x = 0; x <= size.width - 4; x += 4 )
		    {
			int val0 = chdims[data0[x] + 128];
			int val1 = chdims[data0[x + 1] + 128];

			array[val0]++;
			array[val1]++;
			val0 = chdims[data0[x + 2] + 128];
			val1 = chdims[data0[x + 3] + 128];
			array[val0]++;
			array[val1]++;
		    }
		    for( ; x < size.width; x++ )
			array[chdims[data0[x] + 128]]++;
		}
		for( i = 0; i < j; i++ )
		    hist->array[i] = (float) array[i];
	    }
	    break;
	case 2:
	    if( (hist->dims[1] + 1) * (hist->dims[0] + 1) < 4096 )
	    {
		char *data0 = img[0];
		char *data1 = img[1];
		int hstep = hist->dims[1] + 1;
		int array[4096];
		int chdims0[256];
		int chdims1[256];

		memcpy( chdims0, hist->chdims[0], 256 * sizeof( int ));
		memcpy( chdims1, hist->chdims[1], 256 * sizeof( int ));

		memset( array, 0, hstep * (hist->mdims[0] + 1) * sizeof( *array ));

		for( y = 0; y < size.height; y++, data0 += step, data1 += step )
		{
		    for( x = 0; x < size.width - 2; x += 2 )
		    {
			int val0 = chdims0[data0[x] + 128];
			int val1 = chdims1[data1[x] + 128];

			array[val0 + val1]++;
			val0 = chdims0[data0[x + 1] + 128];
			val1 = chdims1[data1[x + 1] + 128];
			array[val0 + val1]++;
		    }
		    for( ; x < size.width; x++ )
		    {
			int val0 = chdims0[data0[x] + 128];
			int val1 = chdims1[data1[x] + 128];

			array[val0 + val1]++;
		    }
		}
		for( y = 0, i = 0; y < hist->dims[0]; y++ )
		    for( x = 0; x < hist->dims[1]; x++, i++ )
			hist->array[i] += (float) array[y * hstep + x];
	    }
	    else
	    {
		char *data0 = img[0];
		char *data1 = img[1];
		int hstep = hist->dims[1] + 1;
		int *array =
		    (int *) icvAlloc( (hist->dims[1] + 1) * (hist->dims[0] + 1) *

				      sizeof( *array ));
		int chdims0[256];
		int chdims1[256];

		memcpy( chdims0, hist->chdims[0] + 128, 256 * sizeof( int ));
		memcpy( chdims1, hist->chdims[1] + 128, 256 * sizeof( int ));

		memset( array, 0, hstep * (hist->mdims[0] + 1) * sizeof( *array ));

		for( y = 0; y < size.height; y++, data0 += step, data1 += step )
		{
		    for( x = 0; x < size.width - 2; x += 2 )
		    {
			int val0 = chdims0[data0[x]];
			int val1 = chdims1[data1[x]];

			array[val0 + val1]++;
			val0 = chdims0[data0[x + 1]];
			val1 = chdims1[data1[x + 1]];
			array[val0 + val1]++;
		    }
		    for( ; x < size.width; x++ )
		    {
			int val0 = chdims0[data0[x]];
			int val1 = chdims1[data1[x]];

			array[val0 + val1]++;
		    }
		}
		for( y = 0, i = 0; y < hist->dims[0]; y++ )
		    for( x = 0; x < hist->dims[1]; x++, i++ )
			hist->array[i] += (float) array[y * hstep + x];
		icvFree( &array );
	    }
	    break;
	case 3:
	    {
		char *data0 = img[0];
		char *data1 = img[1];
		char *data2 = img[2];

		for( y = 0; y < size.height; y++, data0 += step, data1 += step, data2 += step )
		    for( x = 0; x < size.width; x++ )
		    {
			int val0 = hist->chdims[0][data0[x] + 128];
			int val1 = hist->chdims[1][data1[x] + 128];
			int val2 = hist->chdims[2][data2[x] + 128];

			if( (val0 | val1 | val2) >= 0 )
			    hist->array[val0 + val1 + val2]++;
		    }
	    }
	    break;
	default:
	    {
		char *data[CV_HIST_MAX_DIM];

		for( i = 0; i < hist->c_dims; i++ )
		    data[i] = img[i];

		for( y = 0; y < size.height; y++ )
		{
		    for( x = 0; x < size.width; x++ )
		    {
			int addr = 0;

			for( i = 0; i < hist->c_dims; i++ )
			{
			    int val = hist->chdims[i][data[i][x] + 128];

			    if( val >= 0 )
				addr += val;
			    else
				goto next;
			}
			hist->array[addr]++;
		      next:;
		    }
		    for( i = 0; i < hist->c_dims; i++ )
			data[i] += step;
		}
	    }
	    break;
	}
    }
    else
    {
	if( !dont_clear && hist->set )
	{
	    cvClearSet( hist->set );
	    hist->root = 0;
	}

	switch (hist->c_dims)
	{
	case 1:
	    {
		char *data0 = img[0];
		int *array = (int *) icvAlloc( j * sizeof( *array ));
		int *chdims = hist->chdims[0];

		memset( array, 0, j * sizeof( *array ));

		for( y = 0; y < size.height; y++, data0 += step )
		{
		    for( x = 0; x <= size.width - 4; x += 4 )
		    {
			int val0 = chdims[data0[x] + 128];
			int val1 = chdims[data0[x + 1] + 128];

			array[val0]++;
			array[val1]++;
			val0 = chdims[data0[x + 2] + 128];
			val1 = chdims[data0[x + 3] + 128];
			array[val0]++;
			array[val1]++;
		    }
		    for( ; x < size.width; x++ )
			array[chdims[data0[x] + 128]]++;
		}
		for( i = 0; i < j; i++ )
		    if( array[i] )
			icvInsertNode( hist, i )->value += (float) array[i];
		icvFree( &array );
	    }
	    break;
	case 2:
	    if( (hist->dims[1] + 1) * (hist->dims[0] + 1) < 4096 )
	    {
		char *data0 = img[0];
		char *data1 = img[1];
		int hstep = hist->dims[1] + 1;
		int array[4096];
		int chdims0[256];
		int chdims1[256];

		memcpy( chdims0, hist->chdims[0], 256 * sizeof( int ));
		memcpy( chdims1, hist->chdims[1], 256 * sizeof( int ));

		memset( array, 0, hstep * (hist->mdims[0] + 1) * sizeof( *array ));

		for( y = 0; y < size.height; y++, data0 += step, data1 += step )
		{
		    for( x = 0; x < size.width - 2; x += 2 )
		    {
			int val0 = chdims0[data0[x] + 128];
			int val1 = chdims1[data1[x] + 128];

			array[val0 + val1]++;
			val0 = chdims0[data0[x + 1] + 128];
			val1 = chdims1[data1[x + 1] + 128];
			array[val0 + val1]++;
		    }
		    for( ; x < size.width; x++ )
		    {
			int val0 = chdims0[data0[x] + 128];
			int val1 = chdims1[data1[x] + 128];

			array[val0 + val1]++;
		    }
		}
		for( y = 0, i = 0; y < hist->dims[0]; y++ )
		    for( x = 0; x < hist->dims[1]; x++, i++ )
		    {
			float val = (float) array[y * hstep + x];

			if( val )
			    icvInsertNode( hist, i )->value += val;
		    }
	    }
	    else
	    {
		char *data0 = img[0];
		char *data1 = img[1];
		int hstep = hist->dims[1] + 1;
		int *array =
		    (int *) icvAlloc( (hist->dims[1] + 1) * (hist->dims[0] + 1) *

				      sizeof( *array ));
		int chdims0[256];
		int chdims1[256];

		memcpy( chdims0, hist->chdims[0] + 128, 256 * sizeof( int ));
		memcpy( chdims1, hist->chdims[1] + 128, 256 * sizeof( int ));

		memset( array, 0, hstep * (hist->mdims[0] + 1) * sizeof( *array ));

		for( y = 0; y < size.height; y++, data0 += step, data1 += step )
		{
		    for( x = 0; x < size.width - 2; x += 2 )
		    {
			int val0 = chdims0[data0[x]];
			int val1 = chdims1[data1[x]];

			array[val0 + val1]++;
			val0 = chdims0[data0[x + 1]];
			val1 = chdims1[data1[x + 1]];
			array[val0 + val1]++;
		    }
		    for( ; x < size.width; x++ )
		    {
			int val0 = chdims0[data0[x]];
			int val1 = chdims1[data1[x]];

			array[val0 + val1]++;
		    }
		}
		for( y = 0, i = 0; y < hist->dims[0]; y++ )
		    for( x = 0; x < hist->dims[1]; x++, i++ )
		    {
			float val = (float) array[y * hstep + x];

			if( val )
			    icvInsertNode( hist, i )->value += val;
		    }
		icvFree( &array );
	    }
	    break;
	case 3:
	    {
		char *data0 = img[0];
		char *data1 = img[1];
		char *data2 = img[2];

		for( y = 0; y < size.height; y++, data0 += step, data1 += step, data2 += step )
		    for( x = 0; x < size.width; x++ )
		    {
			int val0 = hist->chdims[0][data0[x] + 128];
			int val1 = hist->chdims[1][data1[x] + 128];
			int val2 = hist->chdims[2][data2[x] + 128];

			if( (val0 | val1 | val2) >= 0 )
			    icvInsertNode( hist, (int64) val0 + val1 + val2 )->value++;
		    }
	    }
	    break;
	default:
	    {
		char *data[CV_HIST_MAX_DIM];

		for( i = 0; i < hist->c_dims; i++ )
		    data[i] = img[i];

		for( y = 0; y < size.height; y++ )
		{
		    for( x = 0; x < size.width; x++ )
		    {
			int64 addr = 0;

			for( i = 0; i < hist->c_dims; i++ )
			{
			    int val = hist->chdims[i][data[i][x] + 128];

			    if( val >= 0 )
				addr += val;
			    else
				goto next_tree;
			}
			icvInsertNode( hist, addr )->value++;
		      next_tree:;
		    }
		    for( i = 0; i < hist->c_dims; i++ )
			data[i] += step;
		}
	    }
	    break;
	}
    }

    return CV_NO_ERR;
}

/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:	  icvCalcHist...C1R
//    Purpose:	  Calculating histogram from array of one-channel images
//    Context:
//    Parameters:
//    Returns:
//    Notes:	  if dont_clear parameter is NULL then histogram clearing before
//		  calculating (all values sets to NULL)
//F*/
IPCVAPI_IMPL( CvStatus, icvCalcHist32fC1R, (float **img, int step, CvSize size,
					    CvHistogram * hist, int dont_clear) )
{
    int i, j, x = 0, y = 0;
    int dims;

    if( !hist || !img )
	return CV_NULLPTR_ERR;

    dims = hist->c_dims;

    for( i = 0; i < dims; i++ )
	if( !img[i] )
	    return CV_NULLPTR_ERR;

    for( i = 0; i < hist->c_dims; i++ )
    {
	if( !hist->thresh[i] )
	    return CV_NULLPTR_ERR;
	assert( hist->chdims[i] );
    }

    j = hist->dims[0] * hist->mdims[0];

    if( hist->type == CV_HIST_ARRAY )
    {
	if( !dont_clear )
	    for( i = 0; i < j; i++ )
		hist->array[i] = 0;

	if( !(hist->flags & CV_HIST_UNIFORM) )
	    switch (hist->c_dims)
	    {
	    case 1:
		{
		    float val;
		    float *data[CV_HIST_MAX_DIM];
		    int addr[CV_HIST_MAX_DIM];

		    data[0] = img[0];

		    for( y = 0; y < size.height; y++ )
		    {
			for( x = 0; x < size.width; x++ )
			{
			    val = data[0][x];
			    if( val >= hist->thresh[0][0] &&
				val < hist->thresh[0][hist->dims[0]] )
			    {
				for( j = 1; j < hist->dims[0]; j++ )
				{
				    assert( hist->thresh[0][j - 1] < hist->thresh[0][j] );
				    if( val <= hist->thresh[0][j] )
					break;
				}
				addr[0] = j - 1;
				hist->array[addr[0]]++;
			    }
			}
			data[0] = (float *) ((uchar *) data[0] + step);
		    }
		}
		break;
	    case 2:
		{
		    float val;
		    float *data[CV_HIST_MAX_DIM];
		    int addr[CV_HIST_MAX_DIM];

		    data[0] = img[0];
		    data[1] = img[1];

		    for( y = 0; y < size.height; y++ )
		    {
			for( x = 0; x < size.width; x++ )
			{
			    val = data[0][x];
			    if( val >= hist->thresh[0][0] &&
				val < hist->thresh[0][hist->dims[0]] )
			    {
				for( j = 1; j < hist->dims[0]; j++ )
				{
				    assert( hist->thresh[0][j - 1] < hist->thresh[0][j] );
				    if( val <= hist->thresh[0][j] )
					break;
				}
				addr[0] = j - 1;

				val = data[1][x];
				if( val >= hist->thresh[1][0] &&
				    val < hist->thresh[1][hist->dims[1]] )
				{
				    for( j = 1; j < hist->dims[1]; j++ )
				    {
					assert( hist->thresh[1][j - 1] < hist->thresh[1][j] );
					if( val <= hist->thresh[1][j] )
					    break;
				    }
				    addr[1] = j - 1;

				    hist->array[addr[0] * hist->mdims[0] + addr[1]]++;
				}
			    }
			}
			data[0] = (float *) ((uchar *) data[0] + step);
			data[1] = (float *) ((uchar *) data[1] + step);
		    }
		}
		break;
	    case 3:
		{
		    float val;
		    float *data[CV_HIST_MAX_DIM];
		    int addr[CV_HIST_MAX_DIM];

		    data[0] = img[0];
		    data[1] = img[1];
		    data[2] = img[2];

		    for( y = 0; y < size.height; y++ )
		    {
			for( x = 0; x < size.width; x++ )
			{
			    val = data[0][x];
			    if( val >= hist->thresh[0][0] &&
				val < hist->thresh[0][hist->dims[0]] )
			    {
				for( j = 1; j < hist->dims[0]; j++ )
				{
				    assert( hist->thresh[0][j - 1] < hist->thresh[0][j] );
				    if( val <= hist->thresh[0][j] )
					break;
				}
				addr[0] = j - 1;

				val = data[1][x];
				if( val >= hist->thresh[1][0] &&
				    val < hist->thresh[1][hist->dims[1]] )
				{
				    for( j = 1; j < hist->dims[1]; j++ )
				    {
					assert( hist->thresh[1][j - 1] < hist->thresh[1][j] );
					if( val <= hist->thresh[1][j] )
					    break;
				    }
				    addr[1] = j - 1;

				    val = data[2][x];
				    if( val >= hist->thresh[2][0] &&
					val < hist->thresh[2][hist->dims[2]] )
				    {
					for( j = 1; j < hist->dims[2]; j++ )
					{
					    assert( hist->thresh[2][j - 1] <
						    hist->thresh[2][j] );
					    if( val <= hist->thresh[2][j] )
						break;
					}
					addr[2] = j - 1;

					hist->array[addr[0] * hist->mdims[0] +
						    addr[1] * hist->mdims[1] + addr[2]]++;
				    }
				}
			    }
			}
			data[0] = (float *) ((uchar *) data[0] + step);
			data[1] = (float *) ((uchar *) data[1] + step);
			data[2] = (float *) ((uchar *) data[2] + step);
		    }
		}
		break;
	    default:
		{
		    float val;
		    float *data[CV_HIST_MAX_DIM];
		    int addr[CV_HIST_MAX_DIM];

		    for( i = 0; i < dims; i++ )
			data[i] = img[i];

		    for( y = 0; y < size.height; y++ )
		    {
			for( x = 0; x < size.width; x++ )
			{
			    for( i = 0; i < dims; i++ )
			    {
				val = data[i][x];
				if( val < hist->thresh[i][0] ||
				    val >= hist->thresh[i][hist->dims[i]] )
				    goto next_pointfloat;
				else
				{
				    for( j = 1; j < hist->dims[i]; j++ )
				    {
					assert( hist->thresh[i][j - 1] < hist->thresh[i][j] );
					if( val <= hist->thresh[i][j] )
					    break;
				    }
				}
				addr[i] = j - 1;
			    }
			    (*cvGetHistValue_nD( hist, addr ))++;
			  next_pointfloat:;
			}
			for( i = 0; i < dims; i++ )
			    data[i] = (float *) ((uchar *) data[i] + step);
		    }
		}
		break;
	    }
	else
	    switch (hist->c_dims)
	    {
	    case 1:
		{
		    float val;
		    float *data[CV_HIST_MAX_DIM];
		    int addr[CV_HIST_MAX_DIM];
		    float stp[CV_HIST_MAX_DIM];
		    float mn[CV_HIST_MAX_DIM];

		    data[0] = img[0];
		    mn[0] = hist->thresh[0][0];
		    stp[0] =
			hist->dims[0] / (hist->thresh[0][hist->dims[0]] - hist->thresh[0][0]);

		    for( y = 0; y < size.height; y++ )
		    {
			for( x = 0; x < size.width; x++ )
			{
			    val = data[0][x];
			    if( val >= hist->thresh[0][0] &&
				val < hist->thresh[0][hist->dims[0]] )
			    {
				addr[0] = cvFloor( ((float) val - mn[0]) * stp[0] );
				hist->array[addr[0]]++;
			    }
			}
			data[0] = (float *) ((uchar *) data[0] + step);
		    }
		}
		break;
	    case 2:
		{
		    float val;
		    float *data[CV_HIST_MAX_DIM];
		    int addr[CV_HIST_MAX_DIM];
		    float stp[CV_HIST_MAX_DIM];
		    float mn[CV_HIST_MAX_DIM];

		    data[0] = img[0];
		    mn[0] = hist->thresh[0][0];
		    stp[0] =
			hist->dims[0] / (hist->thresh[0][hist->dims[0]] - hist->thresh[0][0]);

		    data[1] = img[1];
		    mn[1] = hist->thresh[1][0];
		    stp[1] =
			hist->dims[1] / (hist->thresh[1][hist->dims[1]] - hist->thresh[1][0]);

		    for( y = 0; y < size.height; y++ )
		    {
			for( x = 0; x < size.width; x++ )
			{
			    val = data[0][x];
			    if( val >= hist->thresh[0][0] &&
				val < hist->thresh[0][hist->dims[0]] )
			    {
				addr[0] = cvFloor( ((float) val - mn[0]) * stp[0] );

				val = data[1][x];
				if( val >= hist->thresh[1][0] &&
				    val < hist->thresh[1][hist->dims[1]] )
				{
				    addr[1] = cvFloor( ((float) val - mn[1]) * stp[1] );

				    hist->array[addr[0] * hist->mdims[0] + addr[1]]++;
				}
			    }
			}
			data[0] = (float *) ((uchar *) data[0] + step);
			data[1] = (float *) ((uchar *) data[1] + step);
		    }
		}
		break;
	    case 3:
		{
		    float val;
		    float *data[CV_HIST_MAX_DIM];
		    int addr[CV_HIST_MAX_DIM];
		    float stp[CV_HIST_MAX_DIM];
		    float mn[CV_HIST_MAX_DIM];

		    data[0] = img[0];
		    mn[0] = hist->thresh[0][0];
		    stp[0] =
			hist->dims[0] / (hist->thresh[0][hist->dims[0]] - hist->thresh[0][0]);

		    data[1] = img[1];
		    mn[1] = hist->thresh[1][0];
		    stp[1] =
			hist->dims[1] / (hist->thresh[1][hist->dims[1]] - hist->thresh[1][0]);

		    data[2] = img[2];
		    mn[2] = hist->thresh[2][0];
		    stp[2] =
			hist->dims[2] / (hist->thresh[2][hist->dims[2]] - hist->thresh[2][0]);

		    for( y = 0; y < size.height; y++ )
		    {
			for( x = 0; x < size.width; x++ )
			{
			    val = data[0][x];
			    if( val >= hist->thresh[0][0] &&
				val < hist->thresh[0][hist->dims[0]] )
			    {
				addr[0] = cvFloor( ((float) val - mn[0]) * stp[0] );

				val = data[1][x];
				if( val >= hist->thresh[1][0] &&
				    val < hist->thresh[1][hist->dims[1]] )
				{
				    addr[1] = cvFloor( ((float) val - mn[1]) * stp[1] );

				    val = data[2][x];
				    if( val >= hist->thresh[2][0] &&
					val < hist->thresh[2][hist->dims[2]] )
				    {
					addr[2] = cvFloor( ((float) val - mn[2]) * stp[2] );

					hist->array[addr[0] * hist->mdims[0] +
						    addr[1] * hist->mdims[1] + addr[2]]++;
				    }
				}
			    }
			}
			data[0] = (float *) ((uchar *) data[0] + step);
			data[1] = (float *) ((uchar *) data[1] + step);
			data[2] = (float *) ((uchar *) data[2] + step);
		    }
		}
		break;
	    default:
		{
		    float val;
		    float *data[CV_HIST_MAX_DIM];
		    int addr[CV_HIST_MAX_DIM];
		    float stp[CV_HIST_MAX_DIM];
		    float mn[CV_HIST_MAX_DIM];

		    for( i = 0; i < dims; i++ )
		    {
			data[i] = img[i];
			mn[i] = hist->thresh[i][0];
			stp[i] =
			    hist->dims[i] / (hist->thresh[i][hist->dims[i]] -
					     hist->thresh[i][0]);
		    }

		    for( y = 0; y < size.height; y++ )
		    {
			for( x = 0; x < size.width; x++ )
			{
			    for( i = 0; i < dims; i++ )
			    {
				val = data[i][x];
				if( val < hist->thresh[i][0] ||
				    val >= hist->thresh[i][hist->dims[i]] )
				    goto next_pointefloat;
				else
				    addr[i] = cvFloor( ((float) val - mn[i]) * stp[i] );
			    }
			    (*cvGetHistValue_nD( hist, addr ))++;
			  next_pointefloat:;
			}
			for( i = 0; i < dims; i++ )
			    data[i] = (float *) ((uchar *) data[i] + step);
		    }
		}
		break;
	    }
    }
    else
    {
	if( !dont_clear && hist->set )
	{
	    cvClearSet( hist->set );
	    hist->root = 0;
	}

	if( !(hist->flags & CV_HIST_UNIFORM) )
	    switch (hist->c_dims)
	    {
	    case 1:
		{
		    float val;
		    float *data[CV_HIST_MAX_DIM];
		    int64 addr[CV_HIST_MAX_DIM];

		    data[0] = img[0];

		    for( y = 0; y < size.height; y++ )
		    {
			for( x = 0; x < size.width; x++ )
			{
			    val = data[0][x];
			    if( val >= hist->thresh[0][0] &&
				val < hist->thresh[0][hist->dims[0]] )
			    {
				for( j = 1; j < hist->dims[0]; j++ )
				{
				    assert( hist->thresh[0][j - 1] < hist->thresh[0][j] );
				    if( val <= hist->thresh[0][j] )
					break;
				}
				addr[0] = j - 1;
				icvInsertNode( hist, addr[0] )->value++;
			    }
			}
			data[0] = (float *) ((uchar *) data[0] + step);
		    }
		}
		break;
	    case 2:
		{
		    float val;
		    float *data[CV_HIST_MAX_DIM];
		    int addr[CV_HIST_MAX_DIM];

		    data[0] = img[0];
		    data[1] = img[1];

		    for( y = 0; y < size.height; y++ )
		    {
			for( x = 0; x < size.width; x++ )
			{
			    val = data[0][x];
			    if( val >= hist->thresh[0][0] &&
				val < hist->thresh[0][hist->dims[0]] )
			    {
				for( j = 1; j < hist->dims[0]; j++ )
				{
				    assert( hist->thresh[0][j - 1] < hist->thresh[0][j] );
				    if( val <= hist->thresh[0][j] )
					break;
				}
				addr[0] = j - 1;

				val = data[1][x];
				if( val >= hist->thresh[1][0] &&
				    val < hist->thresh[1][hist->dims[1]] )
				{
				    for( j = 1; j < hist->dims[1]; j++ )
				    {
					assert( hist->thresh[1][j - 1] < hist->thresh[1][j] );
					if( val <= hist->thresh[1][j] )
					    break;
				    }
				    addr[1] = j - 1;

				    icvInsertNode( hist, (int64) addr[0] * hist->mdims[0] +
						   addr[1] )->value++;
				}
			    }
			}
			data[0] = (float *) ((uchar *) data[0] + step);
			data[1] = (float *) ((uchar *) data[1] + step);
		    }
		}
		break;
	    case 3:
		{
		    float val;
		    float *data[CV_HIST_MAX_DIM];
		    int addr[CV_HIST_MAX_DIM];

		    data[0] = img[0];
		    data[1] = img[1];
		    data[2] = img[2];

		    for( y = 0; y < size.height; y++ )
		    {
			for( x = 0; x < size.width; x++ )
			{
			    val = data[0][x];
			    if( val >= hist->thresh[0][0] &&
				val < hist->thresh[0][hist->dims[0]] )
			    {
				for( j = 1; j < hist->dims[0]; j++ )
				{
				    assert( hist->thresh[0][j - 1] < hist->thresh[0][j] );
				    if( val <= hist->thresh[0][j] )
					break;
				}
				addr[0] = j - 1;

				val = data[1][x];
				if( val >= hist->thresh[1][0] &&
				    val < hist->thresh[1][hist->dims[1]] )
				{
				    for( j = 1; j < hist->dims[1]; j++ )
				    {
					assert( hist->thresh[1][j - 1] < hist->thresh[1][j] );
					if( val <= hist->thresh[1][j] )
					    break;
				    }
				    addr[1] = j - 1;

				    val = data[2][x];
				    if( val >= hist->thresh[2][0] &&
					val < hist->thresh[2][hist->dims[2]] )
				    {
					for( j = 1; j < hist->dims[2]; j++ )
					{
					    assert( hist->thresh[2][j - 1] <
						    hist->thresh[2][j] );
					    if( val <= hist->thresh[2][j] )
						break;
					}
					addr[2] = j - 1;

					icvInsertNode( hist, (int64) addr[0] * hist->mdims[0] +
						       addr[1] * hist->mdims[1] +
						       addr[2] )->value++;
				    }
				}
			    }
			}
			data[0] = (float *) ((uchar *) data[0] + step);
			data[1] = (float *) ((uchar *) data[1] + step);
			data[2] = (float *) ((uchar *) data[2] + step);
		    }
		}
		break;
	    default:
		{
		    float val;
		    float *data[CV_HIST_MAX_DIM];
		    int addr[CV_HIST_MAX_DIM];

		    for( i = 0; i < dims; i++ )
			data[i] = img[i];

		    for( y = 0; y < size.height; y++ )
		    {
			for( x = 0; x < size.width; x++ )
			{
			    for( i = 0; i < dims; i++ )
			    {
				val = data[i][x];
				if( val < hist->thresh[i][0] ||
				    val >= hist->thresh[i][hist->dims[i]] )
				    goto next_pointfloat_tree;
				else
				{
				    for( j = 1; j < hist->dims[i]; j++ )
				    {
					assert( hist->thresh[i][j - 1] < hist->thresh[i][j] );
					if( val <= hist->thresh[i][j] )
					    break;
				    }
				}
				addr[i] = j - 1;
			    }
			    (*cvGetHistValue_nD( hist, addr ))++;
			  next_pointfloat_tree:;
			}
			for( i = 0; i < dims; i++ )
			    data[i] = (float *) ((uchar *) data[i] + step);
		    }
		}
		break;
	    }
	else
	    switch (hist->c_dims)
	    {
	    case 1:
		{
		    float val;
		    float *data[CV_HIST_MAX_DIM];
		    int64 addr[CV_HIST_MAX_DIM];
		    float stp[CV_HIST_MAX_DIM];
		    float mn[CV_HIST_MAX_DIM];

		    data[0] = img[0];
		    mn[0] = hist->thresh[0][0];
		    stp[0] =
			hist->dims[0] / (hist->thresh[0][hist->dims[0]] - hist->thresh[0][0]);

		    for( y = 0; y < size.height; y++ )
		    {
			for( x = 0; x < size.width; x++ )
			{
			    val = data[0][x];
			    if( val >= hist->thresh[0][0] &&
				val < hist->thresh[0][hist->dims[0]] )
			    {
				addr[0] = cvFloor( ((float) val - mn[0]) * stp[0] );
				icvInsertNode( hist, addr[0] )->value++;
			    }
			}
			data[0] = (float *) ((uchar *) data[0] + step);
		    }
		}
		break;
	    case 2:
		{
		    float val;
		    float *data[CV_HIST_MAX_DIM];
		    int addr[CV_HIST_MAX_DIM];
		    float stp[CV_HIST_MAX_DIM];
		    float mn[CV_HIST_MAX_DIM];

		    data[0] = img[0];
		    mn[0] = hist->thresh[0][0];
		    stp[0] =
			hist->dims[0] / (hist->thresh[0][hist->dims[0]] - hist->thresh[0][0]);

		    data[1] = img[1];
		    mn[1] = hist->thresh[1][0];
		    stp[1] =
			hist->dims[1] / (hist->thresh[1][hist->dims[1]] - hist->thresh[1][0]);

		    for( y = 0; y < size.height; y++ )
		    {
			for( x = 0; x < size.width; x++ )
			{
			    val = data[0][x];
			    if( val >= hist->thresh[0][0] &&
				val < hist->thresh[0][hist->dims[0]] )
			    {
				addr[0] = cvFloor( ((float) val - mn[0]) * stp[0] );

				val = data[1][x];
				if( val >= hist->thresh[1][0] &&
				    val < hist->thresh[1][hist->dims[1]] )
				{
				    addr[1] = cvFloor( ((float) val - mn[1]) * stp[1] );

				    icvInsertNode( hist, (int64) addr[0] * hist->mdims[0] +
						   addr[1] )->value++;
				}
			    }
			}
			data[0] = (float *) ((uchar *) data[0] + step);
			data[1] = (float *) ((uchar *) data[1] + step);
		    }
		}
		break;
	    case 3:
		{
		    float val;
		    float *data[CV_HIST_MAX_DIM];
		    int addr[CV_HIST_MAX_DIM];
		    float stp[CV_HIST_MAX_DIM];
		    float mn[CV_HIST_MAX_DIM];

		    data[0] = img[0];
		    mn[0] = hist->thresh[0][0];
		    stp[0] =
			hist->dims[0] / (hist->thresh[0][hist->dims[0]] - hist->thresh[0][0]);

		    data[1] = img[1];
		    mn[1] = hist->thresh[1][0];
		    stp[1] =
			hist->dims[1] / (hist->thresh[1][hist->dims[1]] - hist->thresh[1][0]);

		    data[2] = img[2];
		    mn[2] = hist->thresh[2][0];
		    stp[2] =
			hist->dims[2] / (hist->thresh[2][hist->dims[2]] - hist->thresh[2][0]);

		    for( y = 0; y < size.height; y++ )
		    {
			for( x = 0; x < size.width; x++ )
			{
			    val = data[0][x];
			    if( val >= hist->thresh[0][0] &&
				val < hist->thresh[0][hist->dims[0]] )
			    {
				addr[0] = cvFloor( ((float) val - mn[0]) * stp[0] );

				val = data[1][x];
				if( val >= hist->thresh[1][0] &&
				    val < hist->thresh[1][hist->dims[1]] )
				{
				    addr[1] = cvFloor( ((float) val - mn[1]) * stp[1] );

				    val = data[2][x];
				    if( val >= hist->thresh[2][0] &&
					val < hist->thresh[2][hist->dims[2]] )
				    {
					addr[2] = cvFloor( ((float) val - mn[2]) * stp[2] );

					icvInsertNode( hist, (int64) addr[0] * hist->mdims[0] +
						       addr[1] * hist->mdims[1] +
						       addr[2] )->value++;
				    }
				}
			    }
			}
			data[0] = (float *) ((uchar *) data[0] + step);
			data[1] = (float *) ((uchar *) data[1] + step);
			data[2] = (float *) ((uchar *) data[2] + step);
		    }
		}
		break;
	    default:
		{
		    float val;
		    float *data[CV_HIST_MAX_DIM];
		    int addr[CV_HIST_MAX_DIM];
		    float stp[CV_HIST_MAX_DIM];
		    float mn[CV_HIST_MAX_DIM];

		    for( i = 0; i < dims; i++ )
		    {
			data[i] = img[i];
			mn[i] = hist->thresh[i][0];
			stp[i] =
			    hist->dims[i] / (hist->thresh[i][hist->dims[i]] -
					     hist->thresh[i][0]);
		    }

		    for( y = 0; y < size.height; y++ )
		    {
			for( x = 0; x < size.width; x++ )
			{
			    for( i = 0; i < dims; i++ )
			    {
				val = data[i][x];
				if( val < hist->thresh[i][0] ||
				    val >= hist->thresh[i][hist->dims[i]] )
				    goto next_pointefloat_tree;
				else
				    addr[i] = cvFloor( ((float) val - mn[i]) * stp[i] );
			    }
			    (*cvGetHistValue_nD( hist, addr ))++;
			  next_pointefloat_tree:;
			}
			for( i = 0; i < dims; i++ )
			    data[i] = (float *) ((uchar *) data[i] + step);
		    }
		}
		break;
	    }
    }
    return CV_NO_ERR;
}


/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:	  icvCalcHistMask...C1R
//    Purpose:	  Calculating histogram from array of one-channel images
//    Context:
//    Parameters:
//    Returns:
//    Notes:	  if dont_clear parameter is NULL then histogram clearing before
//		  calculating (all values sets to NULL)
//F*/
IPCVAPI_IMPL( CvStatus, icvCalcHistMask8uC1R, (uchar ** img, int step,
					       uchar * mask, int mask_step,
					       CvSize size,
					       CvHistogram * hist, int dont_clear) )
{
    int i, j, x = 0, y = 0;
    int dims;

    if( !hist || !img || !mask )
	return CV_NULLPTR_ERR;

    dims = hist->c_dims;

    for( i = 0; i < dims; i++ )
	if( !img[i] )
	    return CV_NULLPTR_ERR;

    for( i = 0; i < hist->c_dims; i++ )
    {
	if( !hist->thresh[i] )
	    return CV_NULLPTR_ERR;
	assert( hist->chdims[i] );
    }

    j = hist->dims[0] * hist->mdims[0];

    if( hist->type == CV_HIST_ARRAY )
    {
	if( !dont_clear )
	    for( i = 0; i < j; i++ )
		hist->array[i] = 0;

	switch (hist->c_dims)
	{
	case 1:
	    {
		uchar *data0 = img[0];
		int *array = (int *) hist->array;
		int *chdims = hist->chdims[0];

		for( i = 0; i < j; i++ )
		    array[i] = cvRound( hist->array[i] );

		for( y = 0; y < size.height; y++, data0 += step, mask += mask_step )
		{
		    for( x = 0; x <= size.width - 2; x += 2 )
		    {
			if( mask[x] )
			{
			    int val0 = chdims[data0[x] + 128];

			    array[val0]++;
			}
			if( mask[x + 1] )
			{
			    int val0 = chdims[data0[x + 1] + 128];

			    array[val0]++;
			}
		    }
		    for( ; x < size.width; x++ )
			if( mask[x] )
			    array[chdims[data0[x]] + 128]++;
		}
		for( i = 0; i < j; i++ )
		    hist->array[i] = (float) array[i];
	    }
	    break;
	case 2:
	    if( (hist->dims[1] + 1) * (hist->dims[0] + 1) < 4096 )
	    {
		uchar *data0 = img[0];
		uchar *data1 = img[1];
		int hstep = hist->dims[1] + 1;
		int array[4096];
		int chdims0[256];
		int chdims1[256];

		memcpy( chdims0, hist->chdims[0] + 128, 256 * sizeof( int ));
		memcpy( chdims1, hist->chdims[1] + 128, 256 * sizeof( int ));

		memset( array, 0, hstep * (hist->mdims[0] + 1) * sizeof( *array ));

		for( y = 0; y < size.height;
		     y++, data0 += step, data1 += step, mask += mask_step )
		{
		    for( x = 0; x < size.width - 2; x += 2 )
		    {
			if( mask[x] )
			{
			    int val0 = chdims0[data0[x]];
			    int val1 = chdims1[data1[x]];

			    array[val0 + val1]++;
			}
			if( mask[x + 1] )
			{
			    int val0 = chdims0[data0[x + 1]];
			    int val1 = chdims1[data1[x + 1]];

			    array[val0 + val1]++;
			}
		    }
		    for( ; x < size.width; x++ )
			if( mask[x] )
			{
			    int val0 = chdims0[data0[x]];
			    int val1 = chdims1[data1[x]];

			    array[val0 + val1]++;
			}
		}
		for( y = 0, i = 0; y < hist->dims[0]; y++ )
		    for( x = 0; x < hist->dims[1]; x++, i++ )
			hist->array[i] += (float) array[y * hstep + x];
	    }
	    else
	    {
		uchar *data0 = img[0];
		uchar *data1 = img[1];
		int hstep = hist->dims[1] + 1;
		int *array =
		    (int *) icvAlloc( (hist->dims[1] + 1) * (hist->dims[0] + 1) *

				      sizeof( *array ));
		int chdims0[256];
		int chdims1[256];

		memcpy( chdims0, hist->chdims[0] + 128, 256 * sizeof( int ));
		memcpy( chdims1, hist->chdims[1] + 128, 256 * sizeof( int ));

		memset( array, 0, hstep * (hist->mdims[0] + 1) * sizeof( *array ));

		for( y = 0; y < size.height;
		     y++, data0 += step, data1 += step, mask += mask_step )
		{
		    for( x = 0; x < size.width - 2; x += 2 )
		    {
			if( mask[x] )
			{
			    int val0 = chdims0[data0[x]];
			    int val1 = chdims1[data1[x]];

			    array[val0 + val1]++;
			}
			if( mask[x + 1] )
			{
			    int val0 = chdims0[data0[x + 1]];
			    int val1 = chdims1[data1[x + 1]];

			    array[val0 + val1]++;
			}
		    }
		    for( ; x < size.width; x++ )
			if( mask[x] )
			{
			    int val0 = chdims0[data0[x]];
			    int val1 = chdims1[data1[x]];

			    array[val0 + val1]++;
			}
		}
		for( y = 0, i = 0; y < hist->dims[0]; y++ )
		    for( x = 0; x < hist->dims[1]; x++, i++ )
			hist->array[i] += (float) array[y * hstep + x];
		icvFree( &array );
	    }
	    break;
	case 3:
	    {
		uchar *data0 = img[0];
		uchar *data1 = img[1];
		uchar *data2 = img[2];

		for( y = 0; y < size.height; y++, data0 += step, data1 += step,
		     data2 += step, mask += mask_step )
		    for( x = 0; x < size.width; x++ )
			if( mask[x] )
			{
			    int val0 = hist->chdims[0][data0[x] + 128];
			    int val1 = hist->chdims[1][data1[x] + 128];
			    int val2 = hist->chdims[2][data2[x] + 128];

			    if( (val0 | val1 | val2) >= 0 )
				hist->array[val0 + val1 + val2]++;
			}
	    }
	    break;
	default:
	    {
		uchar *data[CV_HIST_MAX_DIM];

		for( i = 0; i < hist->c_dims; i++ )
		    data[i] = img[i];

		for( y = 0; y < size.height; y++, mask += mask_step )
		{
		    for( x = 0; x < size.width; x++ )
			if( mask[x] )
			{
			    int addr = 0;

			    for( i = 0; i < hist->c_dims; i++ )
			    {
				int val = hist->chdims[i][(int) data[i][x] + 128];

				if( val >= 0 )
				    addr += val;
				else
				    goto next;
			    }
			    hist->array[addr]++;
			  next:;
			}
		    for( i = 0; i < hist->c_dims; i++ )
			data[i] += step;
		}
	    }
	    break;
	}
    }
    else
    {
	if( !dont_clear && hist->set )
	{
	    cvClearSet( hist->set );
	    hist->root = 0;
	}

	switch (hist->c_dims)
	{
	case 1:
	    {
		uchar *data0 = img[0];
		int *array = (int *) icvAlloc( j * sizeof( *array ));
		int *chdims = hist->chdims[0];

		memset( array, 0, j * sizeof( *array ));

		for( y = 0; y < size.height; y++, data0 += step, mask += mask_step )
		{
		    for( x = 0; x <= size.width - 2; x += 2 )
		    {
			if( mask[x] )
			{
			    int val0 = chdims[data0[x] + 128];

			    array[val0]++;
			}
			if( mask[x + 1] )
			{
			    int val0 = chdims[data0[x + 1] + 128];

			    array[val0]++;
			}
		    }
		    for( ; x < size.width; x++ )
			if( mask[x] )
			    array[chdims[data0[x]] + 128]++;
		}
		for( i = 0; i < j; i++ )
		    if( array[i] )
			icvInsertNode( hist, i )->value += (float) array[i];

		icvFree( &array );
	    }
	    break;
	case 2:
	    if( (hist->dims[1] + 1) * (hist->dims[0] + 1) < 4096 )
	    {
		uchar *data0 = img[0];
		uchar *data1 = img[1];
		int hstep = hist->dims[1] + 1;
		int array[4096];
		int chdims0[256];
		int chdims1[256];

		memcpy( chdims0, hist->chdims[0] + 128, 256 * sizeof( int ));
		memcpy( chdims1, hist->chdims[1] + 128, 256 * sizeof( int ));

		memset( array, 0, hstep * (hist->mdims[0] + 1) * sizeof( *array ));

		for( y = 0; y < size.height;
		     y++, data0 += step, data1 += step, mask += mask_step )
		{
		    for( x = 0; x < size.width - 2; x += 2 )
		    {
			if( mask[x] )
			{
			    int val0 = chdims0[data0[x]];
			    int val1 = chdims1[data1[x]];

			    array[val0 + val1]++;
			}
			if( mask[x + 1] )
			{
			    int val0 = chdims0[data0[x + 1]];
			    int val1 = chdims1[data1[x + 1]];

			    array[val0 + val1]++;
			}
		    }
		    for( ; x < size.width; x++ )
			if( mask[x] )
			{
			    int val0 = chdims0[data0[x]];
			    int val1 = chdims1[data1[x]];

			    array[val0 + val1]++;
			}
		}
		for( y = 0, i = 0; y < hist->dims[0]; y++ )
		    for( x = 0; x < hist->dims[1]; x++, i++ )
		    {
			float val = (float) array[y * hstep + x];

			if( val )
			    icvInsertNode( hist, i )->value += val;
		    }
	    }
	    else
	    {
		uchar *data0 = img[0];
		uchar *data1 = img[1];
		int hstep = hist->dims[1] + 1;
		int *array =
		    (int *) icvAlloc( (hist->dims[1] + 1) * (hist->dims[0] + 1) *

				      sizeof( *array ));
		int chdims0[256];
		int chdims1[256];

		memcpy( chdims0, hist->chdims[0] + 128, 256 * sizeof( int ));
		memcpy( chdims1, hist->chdims[1] + 128, 256 * sizeof( int ));

		memset( array, 0, hstep * (hist->mdims[0] + 1) * sizeof( *array ));

		for( y = 0; y < size.height;
		     y++, data0 += step, data1 += step, mask += mask_step )
		{
		    for( x = 0; x < size.width - 2; x += 2 )
		    {
			if( mask[x] )
			{
			    int val0 = chdims0[data0[x]];
			    int val1 = chdims1[data1[x]];

			    array[val0 + val1]++;
			}
			if( mask[x + 1] )
			{
			    int val0 = chdims0[data0[x + 1]];
			    int val1 = chdims1[data1[x + 1]];

			    array[val0 + val1]++;
			}
		    }
		    for( ; x < size.width; x++ )
			if( mask[x] )
			{
			    int val0 = chdims0[data0[x]];
			    int val1 = chdims1[data1[x]];

			    array[val0 + val1]++;
			}
		}
		for( y = 0, i = 0; y < hist->dims[0]; y++ )
		    for( x = 0; x < hist->dims[1]; x++, i++ )
		    {
			float val = (float) array[y * hstep + x];

			if( val )
			    icvInsertNode( hist, i )->value += val;
		    }
		icvFree( &array );
	    }
	    break;
	case 3:
	    {
		uchar *data0 = img[0];
		uchar *data1 = img[1];
		uchar *data2 = img[2];

		for( y = 0; y < size.height; y++, data0 += step, data1 += step,
		     data2 += step, mask += mask_step )
		    for( x = 0; x < size.width; x++ )
			if( mask[x] )
			{
			    int val0 = hist->chdims[0][data0[x] + 128];
			    int val1 = hist->chdims[1][data1[x] + 128];
			    int val2 = hist->chdims[2][data2[x] + 128];

			    if( (val0 | val1 | val2) >= 0 )
				icvInsertNode( hist, (int64) val0 + val1 + val2 )->value++;
			}
	    }
	    break;
	default:
	    {
		uchar *data[CV_HIST_MAX_DIM];

		for( i = 0; i < hist->c_dims; i++ )
		    data[i] = img[i];

		for( y = 0; y < size.height; y++, mask += mask_step )
		{
		    for( x = 0; x < size.width; x++ )
			if( mask[x] )
			{
			    int64 addr = 0;

			    for( i = 0; i < hist->c_dims; i++ )
			    {
				int val = hist->chdims[i][(int) data[i][x] + 128];

				if( val >= 0 )
				    addr += val;
				else
				    goto next_tree;
			    }
			    icvInsertNode( hist, addr )->value++;
			  next_tree:;
			}
		    for( i = 0; i < hist->c_dims; i++ )
			data[i] += step;
		}
	    }
	    break;
	}
    }
    return CV_NO_ERR;
}


/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:	  icvCalcHistMask...C1R
//    Purpose:	  Calculating histogram from array of one-channel images
//    Context:
//    Parameters:
//    Returns:
//    Notes:	  if dont_clear parameter is NULL then histogram clearing before
//		  calculating (all values sets to NULL)
//F*/
IPCVAPI_IMPL( CvStatus, icvCalcHistMask8sC1R, (char **img, int step,
					       uchar * mask, int mask_step,
					       CvSize size,
					       CvHistogram * hist, int dont_clear) )
{
    int i, j, x = 0, y = 0;
    int dims;

    if( !hist || !img || !mask )
	return CV_NULLPTR_ERR;

    dims = hist->c_dims;

    for( i = 0; i < dims; i++ )
	if( !img[i] )
	    return CV_NULLPTR_ERR;

    for( i = 0; i < hist->c_dims; i++ )
    {
	if( !hist->thresh[i] )
	    return CV_NULLPTR_ERR;
	assert( hist->chdims[i] );
    }

    j = hist->dims[0] * hist->mdims[0];

    if( hist->type == CV_HIST_ARRAY )
    {
	if( !dont_clear )
	    for( i = 0; i < j; i++ )
		hist->array[i] = 0;

	switch (hist->c_dims)
	{
	case 1:
	    {
		char *data0 = img[0];
		int *array = (int *) hist->array;
		int *chdims = hist->chdims[0];

		for( i = 0; i < j; i++ )
		    array[i] = cvRound( hist->array[i] );

		for( y = 0; y < size.height; y++, data0 += step, mask += mask_step )
		{
		    for( x = 0; x <= size.width - 2; x += 2 )
		    {
			if( mask[x] )
			{
			    int val0 = chdims[data0[x] + 128];

			    array[val0]++;
			}
			if( mask[x + 1] )
			{
			    int val0 = chdims[data0[x + 1] + 128];

			    array[val0]++;
			}
		    }
		    for( ; x < size.width; x++ )
			if( mask[x] )
			    array[chdims[data0[x] + 128]]++;
		}
		for( i = 0; i < j; i++ )
		    hist->array[i] = (float) array[i];
	    }
	    break;
	case 2:
	    if( (hist->dims[1] + 1) * (hist->dims[0] + 1) < 4096 )
	    {
		char *data0 = img[0];
		char *data1 = img[1];
		int hstep = hist->dims[1] + 1;
		int array[4096];
		int chdims0[256];
		int chdims1[256];

		memcpy( chdims0, hist->chdims[0], 256 * sizeof( int ));
		memcpy( chdims1, hist->chdims[1], 256 * sizeof( int ));

		memset( array, 0, hstep * (hist->mdims[0] + 1) * sizeof( *array ));

		for( y = 0; y < size.height; y++, data0 += step, data1 += step,
		     mask += mask_step )
		{
		    for( x = 0; x < size.width - 2; x += 2 )
		    {
			if( mask[x] )
			{
			    int val0 = chdims0[data0[x] + 128];
			    int val1 = chdims1[data1[x] + 128];

			    array[val0 + val1]++;
			}
			if( mask[x + 1] )
			{
			    int val0 = chdims0[data0[x + 1] + 128];
			    int val1 = chdims1[data1[x + 1] + 128];

			    array[val0 + val1]++;
			}
		    }
		    for( ; x < size.width; x++ )
			if( mask[x] )
			{
			    int val0 = chdims0[data0[x] + 128];
			    int val1 = chdims1[data1[x] + 128];

			    array[val0 + val1]++;
			}
		}
		for( y = 0, i = 0; y < hist->dims[0]; y++ )
		    for( x = 0; x < hist->dims[1]; x++, i++ )
			hist->array[i] += (float) array[y * hstep + x];
	    }
	    else
	    {
		char *data0 = img[0];
		char *data1 = img[1];
		int hstep = hist->dims[1] + 1;
		int *array =
		    (int *) icvAlloc( (hist->dims[1] + 1) * (hist->dims[0] + 1) *

				      sizeof( *array ));
		int chdims0[256];
		int chdims1[256];

		memcpy( chdims0, hist->chdims[0] + 128, 256 * sizeof( int ));
		memcpy( chdims1, hist->chdims[1] + 128, 256 * sizeof( int ));

		memset( array, 0, hstep * (hist->mdims[0] + 1) * sizeof( *array ));

		for( y = 0; y < size.height; y++, data0 += step, data1 += step,
		     mask += mask_step )
		{
		    for( x = 0; x < size.width - 2; x += 2 )
		    {
			if( mask[x] )
			{
			    int val0 = chdims0[data0[x] + 128];
			    int val1 = chdims1[data1[x] + 128];

			    array[val0 + val1]++;
			}
			if( mask[x + 1] )
			{
			    int val0 = chdims0[data0[x + 1] + 128];
			    int val1 = chdims1[data1[x + 1] + 128];

			    array[val0 + val1]++;
			}
		    }
		    for( ; x < size.width; x++ )
			if( mask[x] )
			{
			    int val0 = chdims0[data0[x] + 128];
			    int val1 = chdims1[data1[x] + 128];

			    array[val0 + val1]++;
			}
		}
		for( y = 0, i = 0; y < hist->dims[0]; y++ )
		    for( x = 0; x < hist->dims[1]; x++, i++ )
			hist->array[i] += (float) array[y * hstep + x];
		icvFree( &array );
	    }
	    break;
	case 3:
	    {
		char *data0 = img[0];
		char *data1 = img[1];
		char *data2 = img[2];

		for( y = 0; y < size.height; y++, data0 += step, data1 += step,
		     data2 += step, mask += mask_step )
		    for( x = 0; x < size.width; x++ )
			if( mask[x] )
			{
			    int val0 = hist->chdims[0][(int) data0[x] + 128];
			    int val1 = hist->chdims[1][(int) data1[x] + 128];
			    int val2 = hist->chdims[2][(int) data1[x] + 128];

			    if( (val0 | val1 | val2) >= 0 )
				hist->array[val0 + val1 + val2]++;
			}
	    }
	    break;
	default:
	    {
		char *data[CV_HIST_MAX_DIM];

		for( i = 0; i < hist->c_dims; i++ )
		    data[i] = img[i];

		for( y = 0; y < size.height; y++, mask += mask_step )
		{
		    for( x = 0; x < size.width; x++ )
			if( mask[x] )
			{
			    int addr = 0;

			    for( i = 0; i < hist->c_dims; i++ )
			    {
				int val = hist->chdims[i][(int) data[i][x] + 128];

				if( val >= 0 )
				    addr += val;
				else
				    goto next;
			    }
			    hist->array[addr]++;
			  next:;
			}
		    for( i = 0; i < hist->c_dims; i++ )
			data[i] += step;
		}
	    }
	    break;
	}
    }
    else
    {
	if( !dont_clear && hist->set )
	{
	    cvClearSet( hist->set );
	    hist->root = 0;
	}

	switch (hist->c_dims)
	{
	case 1:
	    {
		char *data0 = img[0];
		int *array = (int *) icvAlloc( j * sizeof( *array ));
		int *chdims = hist->chdims[0];

		memset( array, 0, j * sizeof( *array ));

		for( y = 0; y < size.height; y++, data0 += step, mask += mask_step )
		{
		    for( x = 0; x <= size.width - 2; x += 2 )
		    {
			if( mask[x] )
			{
			    int val0 = chdims[data0[x] + 128];

			    array[val0]++;
			}
			if( mask[x + 1] )
			{
			    int val0 = chdims[data0[x + 1] + 128];

			    array[val0]++;
			}
		    }
		    for( ; x < size.width; x++ )
			if( mask[x] )
			    array[chdims[data0[x] + 128]]++;
		}
		for( i = 0; i < j; i++ )
		    if( array[i] )
			icvInsertNode( hist, i )->value += (float) array[i];
		icvFree( &array );
	    }
	    break;
	case 2:
	    if( (hist->dims[1] + 1) * (hist->dims[0] + 1) < 4096 )
	    {
		char *data0 = img[0];
		char *data1 = img[1];
		int hstep = hist->dims[1] + 1;
		int array[4096];
		int chdims0[256];
		int chdims1[256];

		memcpy( chdims0, hist->chdims[0], 256 * sizeof( int ));
		memcpy( chdims1, hist->chdims[1], 256 * sizeof( int ));

		memset( array, 0, hstep * (hist->mdims[0] + 1) * sizeof( *array ));

		for( y = 0; y < size.height; y++, data0 += step, data1 += step,
		     mask += mask_step )
		{
		    for( x = 0; x < size.width - 2; x += 2 )
		    {
			if( mask[x] )
			{
			    int val0 = chdims0[data0[x] + 128];
			    int val1 = chdims1[data1[x] + 128];

			    array[val0 + val1]++;
			}
			if( mask[x + 1] )
			{
			    int val0 = chdims0[data0[x + 1] + 128];
			    int val1 = chdims1[data1[x + 1] + 128];

			    array[val0 + val1]++;
			}
		    }
		    for( ; x < size.width; x++ )
			if( mask[x] )
			{
			    int val0 = chdims0[data0[x] + 128];
			    int val1 = chdims1[data1[x] + 128];

			    array[val0 + val1]++;
			}
		}
		for( y = 0, i = 0; y < hist->dims[0]; y++ )
		    for( x = 0; x < hist->dims[1]; x++, i++ )
		    {
			float val = (float) array[y * hstep + x];

			if( val )
			    icvInsertNode( hist, i )->value += val;
		    }
	    }
	    else
	    {
		char *data0 = img[0];
		char *data1 = img[1];
		int hstep = hist->dims[1] + 1;
		int *array =
		    (int *) icvAlloc( (hist->dims[1] + 1) * (hist->dims[0] + 1) *

				      sizeof( *array ));
		int chdims0[256];
		int chdims1[256];

		memcpy( chdims0, hist->chdims[0] + 128, 256 * sizeof( int ));
		memcpy( chdims1, hist->chdims[1] + 128, 256 * sizeof( int ));

		memset( array, 0, hstep * (hist->mdims[0] + 1) * sizeof( *array ));

		for( y = 0; y < size.height; y++, data0 += step, data1 += step,
		     mask += mask_step )
		{
		    for( x = 0; x < size.width - 2; x += 2 )
		    {
			if( mask[x] )
			{
			    int val0 = chdims0[data0[x] + 128];
			    int val1 = chdims1[data1[x] + 128];

			    array[val0 + val1]++;
			}
			if( mask[x + 1] )
			{
			    int val0 = chdims0[data0[x + 1] + 128];
			    int val1 = chdims1[data1[x + 1] + 128];

			    array[val0 + val1]++;
			}
		    }
		    for( ; x < size.width; x++ )
			if( mask[x] )
			{
			    int val0 = chdims0[data0[x] + 128];
			    int val1 = chdims1[data1[x] + 128];

			    array[val0 + val1]++;
			}
		}
		for( y = 0, i = 0; y < hist->dims[0]; y++ )
		    for( x = 0; x < hist->dims[1]; x++, i++ )
		    {
			float val = (float) array[y * hstep + x];

			if( val )
			    icvInsertNode( hist, i )->value += val;
		    }
		icvFree( &array );
	    }
	    break;
	case 3:
	    {
		char *data0 = img[0];
		char *data1 = img[1];
		char *data2 = img[2];

		for( y = 0; y < size.height; y++, data0 += step, data1 += step,
		     data2 += step, mask += mask_step )
		    for( x = 0; x < size.width; x++ )
			if( mask[x] )
			{
			    int val0 = hist->chdims[0][(int) data0[x] + 128];
			    int val1 = hist->chdims[1][(int) data1[x] + 128];
			    int val2 = hist->chdims[2][(int) data1[x] + 128];

			    if( (val0 | val1 | val2) >= 0 )
				icvInsertNode( hist, (int64) val0 + val1 + val2 )->value++;
			}
	    }
	    break;
	default:
	    {
		char *data[CV_HIST_MAX_DIM];

		for( i = 0; i < hist->c_dims; i++ )
		    data[i] = img[i];

		for( y = 0; y < size.height; y++, mask += mask_step )
		{
		    for( x = 0; x < size.width; x++ )
			if( mask[x] )
			{
			    int64 addr = 0;

			    for( i = 0; i < hist->c_dims; i++ )
			    {
				int val = hist->chdims[i][(int) data[i][x] + 128];

				if( val >= 0 )
				    addr += val;
				else
				    goto next_tree;
			    }
			    icvInsertNode( hist, addr )->value++;
			  next_tree:;
			}
		    for( i = 0; i < hist->c_dims; i++ )
			data[i] += step;
		}
	    }
	    break;
	}
    }
    return CV_NO_ERR;
}

/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:	  icvCalcHist...C1R
//    Purpose:	  Calculating histogram from array of one-channel images
//    Context:
//    Parameters:
//    Returns:
//    Notes:	  if dont_clear parameter is NULL then histogram clearing before
//		  calculating (all values sets to NULL)
//F*/
IPCVAPI_IMPL( CvStatus, icvCalcHistMask32fC1R, (float **img, int step,
						uchar * mask, int mask_step,
						CvSize size,
						CvHistogram * hist, int dont_clear) )
{
    int i, j, x = 0, y = 0;
    int dims;

    if( !hist || !img )
	return CV_NULLPTR_ERR;

    dims = hist->c_dims;

    for( i = 0; i < dims; i++ )
	if( !img[i] )
	    return CV_NULLPTR_ERR;

    for( i = 0; i < hist->c_dims; i++ )
    {
	if( !hist->thresh[i] )
	    return CV_NULLPTR_ERR;
	assert( hist->chdims[i] );
    }

    j = hist->dims[0] * hist->mdims[0];

    if( hist->type == CV_HIST_ARRAY )
    {
	if( !dont_clear )
	    for( i = 0; i < j; i++ )
		hist->array[i] = 0;

	if( !(hist->flags & CV_HIST_UNIFORM) )
	    switch (hist->c_dims)
	    {
	    case 1:
		{
		    float val;
		    float *data[CV_HIST_MAX_DIM];
		    int addr[CV_HIST_MAX_DIM];

		    data[0] = img[0];

		    for( y = 0; y < size.height; y++, mask += mask_step )
		    {
			for( x = 0; x < size.width; x++ )
			{
			    if( mask[x] )
			    {
				val = data[0][x];
				if( val >= hist->thresh[0][0] &&
				    val < hist->thresh[0][hist->dims[0]] )
				{
				    for( j = 1; j < hist->dims[0]; j++ )
				    {
					assert( hist->thresh[0][j - 1] < hist->thresh[0][j] );
					if( val <= hist->thresh[0][j] )
					    break;
				    }
				    addr[0] = j - 1;
				    hist->array[addr[0]]++;
				}
			    }
			}
			data[0] = (float *) ((uchar *) data[0] + step);
		    }
		}
		break;
	    case 2:
		{
		    float val;
		    float *data[CV_HIST_MAX_DIM];
		    int addr[CV_HIST_MAX_DIM];

		    data[0] = img[0];
		    data[1] = img[1];

		    for( y = 0; y < size.height; y++, mask += mask_step )
		    {
			for( x = 0; x < size.width; x++ )
			{
			    if( mask[x] )
			    {
				val = data[0][x];
				if( val >= hist->thresh[0][0] &&
				    val < hist->thresh[0][hist->dims[0]] )
				{
				    for( j = 1; j < hist->dims[0]; j++ )
				    {
					assert( hist->thresh[0][j - 1] < hist->thresh[0][j] );
					if( val <= hist->thresh[0][j] )
					    break;
				    }
				    addr[0] = j - 1;

				    val = data[1][x];
				    if( val >= hist->thresh[1][0] &&
					val < hist->thresh[1][hist->dims[1]] )
				    {
					for( j = 1; j < hist->dims[1]; j++ )
					{
					    assert( hist->thresh[1][j - 1] <
						    hist->thresh[1][j] );
					    if( val <= hist->thresh[1][j] )
						break;
					}
					addr[1] = j - 1;

					hist->array[addr[0] * hist->mdims[0] + addr[1]]++;
				    }
				}
			    }
			}
			data[0] = (float *) ((uchar *) data[0] + step);
			data[1] = (float *) ((uchar *) data[1] + step);
		    }
		}
		break;
	    case 3:
		{
		    float val;
		    float *data[CV_HIST_MAX_DIM];
		    int addr[CV_HIST_MAX_DIM];

		    data[0] = img[0];
		    data[1] = img[1];
		    data[2] = img[2];

		    for( y = 0; y < size.height; y++, mask += mask_step )
		    {
			for( x = 0; x < size.width; x++ )
			{
			    if( mask[x] )
			    {
				val = data[0][x];
				if( val >= hist->thresh[0][0] &&
				    val < hist->thresh[0][hist->dims[0]] )
				{
				    for( j = 1; j < hist->dims[0]; j++ )
				    {
					assert( hist->thresh[0][j - 1] < hist->thresh[0][j] );
					if( val <= hist->thresh[0][j] )
					    break;
				    }
				    addr[0] = j - 1;

				    val = data[1][x];
				    if( val >= hist->thresh[1][0] &&
					val < hist->thresh[1][hist->dims[1]] )
				    {
					for( j = 1; j < hist->dims[1]; j++ )
					{
					    assert( hist->thresh[1][j - 1] <
						    hist->thresh[1][j] );
					    if( val <= hist->thresh[1][j] )
						break;
					}
					addr[1] = j - 1;

					val = data[2][x];
					if( val >= hist->thresh[2][0] &&
					    val < hist->thresh[2][hist->dims[2]] )
					{
					    for( j = 1; j < hist->dims[2]; j++ )
					    {
						assert( hist->thresh[2][j - 1] <
							hist->thresh[2][j] );
						if( val <= hist->thresh[2][j] )
						    break;
					    }
					    addr[2] = j - 1;

					    hist->array[addr[0] * hist->mdims[0] +
							addr[1] * hist->mdims[1] + addr[2]]++;
					}
				    }
				}
			    }
			}
			data[0] = (float *) ((uchar *) data[0] + step);
			data[1] = (float *) ((uchar *) data[1] + step);
			data[2] = (float *) ((uchar *) data[2] + step);
		    }
		}
		break;
	    default:
		{
		    float val;
		    float *data[CV_HIST_MAX_DIM];
		    int addr[CV_HIST_MAX_DIM];

		    for( i = 0; i < dims; i++ )
			data[i] = img[i];

		    for( y = 0; y < size.height; y++, mask += mask_step )
		    {
			for( x = 0; x < size.width; x++ )
			{
			    if( mask[x] )
			    {
				for( i = 0; i < dims; i++ )
				{
				    val = data[i][x];
				    if( val < hist->thresh[i][0] ||
					val >= hist->thresh[i][hist->dims[i]] )
					goto next_point_float;
				    else
				    {
					for( j = 1; j < hist->dims[i]; j++ )
					{
					    assert( hist->thresh[i][j - 1] <
						    hist->thresh[i][j] );
					    if( val <= hist->thresh[i][j] )
						break;
					}
				    }
				    addr[i] = j - 1;
				}
				(*cvGetHistValue_nD( hist, addr ))++;
			    }
			  next_point_float:;
			}
			for( i = 0; i < dims; i++ )
			    data[i] = (float *) ((uchar *) data[i] + step);
		    }
		}
		break;
	    }
	else
	    switch (hist->c_dims)
	    {
	    case 1:
		{
		    float val;
		    float *data[CV_HIST_MAX_DIM];
		    int addr[CV_HIST_MAX_DIM];
		    float stp[CV_HIST_MAX_DIM];
		    float mn[CV_HIST_MAX_DIM];

		    data[0] = img[0];
		    mn[0] = hist->thresh[0][0];
		    stp[0] =
			hist->dims[0] / (hist->thresh[0][hist->dims[0]] - hist->thresh[0][0]);

		    for( y = 0; y < size.height; y++, mask += mask_step )
		    {
			for( x = 0; x < size.width; x++ )
			{
			    if( mask[x] )
			    {
				val = data[0][x];
				if( val >= hist->thresh[0][0] &&
				    val < hist->thresh[0][hist->dims[0]] )
				{
				    addr[0] = cvFloor( ((float) val - mn[0]) * stp[0] );
				    hist->array[addr[0]]++;
				}
			    }
			}
			data[0] = (float *) ((uchar *) data[0] + step);
		    }
		}
		break;
	    case 2:
		{
		    float val;
		    float *data[CV_HIST_MAX_DIM];
		    int addr[CV_HIST_MAX_DIM];
		    float stp[CV_HIST_MAX_DIM];
		    float mn[CV_HIST_MAX_DIM];

		    data[0] = img[0];
		    mn[0] = hist->thresh[0][0];
		    stp[0] =
			hist->dims[0] / (hist->thresh[0][hist->dims[0]] - hist->thresh[0][0]);

		    data[1] = img[1];
		    mn[1] = hist->thresh[1][0];
		    stp[1] =
			hist->dims[1] / (hist->thresh[1][hist->dims[1]] - hist->thresh[1][0]);

		    for( y = 0; y < size.height; y++, mask += mask_step )
		    {
			for( x = 0; x < size.width; x++ )
			{
			    if( mask[x] )
			    {
				val = data[0][x];
				if( val >= hist->thresh[0][0] &&
				    val < hist->thresh[0][hist->dims[0]] )
				{
				    addr[0] = cvFloor( ((float) val - mn[0]) * stp[0] );

				    val = data[1][x];
				    if( val >= hist->thresh[1][0] &&
					val < hist->thresh[1][hist->dims[1]] )
				    {
					addr[1] = cvFloor( ((float) val - mn[1]) * stp[1] );

					hist->array[addr[0] * hist->mdims[0] + addr[1]]++;
				    }
				}
			    }
			}
			data[0] = (float *) ((uchar *) data[0] + step);
			data[1] = (float *) ((uchar *) data[1] + step);
		    }
		}
		break;
	    case 3:
		{
		    float val;
		    float *data[CV_HIST_MAX_DIM];
		    int addr[CV_HIST_MAX_DIM];
		    float stp[CV_HIST_MAX_DIM];
		    float mn[CV_HIST_MAX_DIM];

		    data[0] = img[0];
		    mn[0] = hist->thresh[0][0];
		    stp[0] =
			hist->dims[0] / (hist->thresh[0][hist->dims[0]] - hist->thresh[0][0]);

		    data[1] = img[1];
		    mn[1] = hist->thresh[1][0];
		    stp[1] =
			hist->dims[1] / (hist->thresh[1][hist->dims[1]] - hist->thresh[1][0]);

		    data[2] = img[2];
		    mn[2] = hist->thresh[2][0];
		    stp[2] =
			hist->dims[2] / (hist->thresh[2][hist->dims[2]] - hist->thresh[2][0]);

		    for( y = 0; y < size.height; y++ )
		    {
			for( x = 0; x < size.width; x++ )
			{
			    if( mask[x] )
			    {
				val = data[0][x];
				if( val >= hist->thresh[0][0] &&
				    val < hist->thresh[0][hist->dims[0]] )
				{
				    addr[0] = cvFloor( ((float) val - mn[0]) * stp[0] );

				    val = data[1][x];
				    if( val >= hist->thresh[1][0] &&
					val < hist->thresh[1][hist->dims[1]] )
				    {
					addr[1] = cvFloor( ((float) val - mn[1]) * stp[1] );

					val = data[2][x];
					if( val >= hist->thresh[2][0] &&
					    val < hist->thresh[2][hist->dims[2]] )
					{
					    addr[2] =
						cvFloor( ((float) val - mn[2]) * stp[2] );

					    hist->array[addr[0] * hist->mdims[0] +
							addr[1] * hist->mdims[1] + addr[2]]++;
					}
				    }
				}
			    }
			}
			data[0] = (float *) ((uchar *) data[0] + step);
			data[1] = (float *) ((uchar *) data[1] + step);
			data[2] = (float *) ((uchar *) data[2] + step);
		    }
		}
		break;
	    default:
		{
		    float val;
		    float *data[CV_HIST_MAX_DIM];
		    int addr[CV_HIST_MAX_DIM];
		    float stp[CV_HIST_MAX_DIM];
		    float mn[CV_HIST_MAX_DIM];

		    for( i = 0; i < dims; i++ )
		    {
			data[i] = img[i];
			mn[i] = hist->thresh[i][0];
			stp[i] =
			    hist->dims[i] / (hist->thresh[i][hist->dims[i]] -
					     hist->thresh[i][0]);
		    }

		    for( y = 0; y < size.height; y++, mask += mask_step )
		    {
			for( x = 0; x < size.width; x++ )
			{
			    if( mask[x] )
			    {
				for( i = 0; i < dims; i++ )
				{
				    val = data[i][x];
				    if( val < hist->thresh[i][0] ||
					val >= hist->thresh[i][hist->dims[i]] )
					goto next_pointe_float;
				    else
					addr[i] = cvFloor( ((float) val - mn[i]) * stp[i] );
				}
				(*cvGetHistValue_nD( hist, addr ))++;
			    }
			  next_pointe_float:;
			}
			for( i = 0; i < dims; i++ )
			    data[i] = (float *) ((uchar *) data[i] + step);
		    }
		}
		break;
	    }
    }
    else
    {
	if( !dont_clear && hist->set )
	{
	    cvClearSet( hist->set );
	    hist->root = 0;
	}

	if( !(hist->flags & CV_HIST_UNIFORM) )
	    switch (hist->c_dims)
	    {
	    case 1:
		{
		    float val;
		    float *data[CV_HIST_MAX_DIM];
		    int addr[CV_HIST_MAX_DIM];

		    data[0] = img[0];

		    for( y = 0; y < size.height; y++, mask += mask_step )
		    {
			for( x = 0; x < size.width; x++ )
			{
			    if( mask[x] )
			    {
				val = data[0][x];
				if( val >= hist->thresh[0][0] &&
				    val < hist->thresh[0][hist->dims[0]] )
				{
				    for( j = 1; j < hist->dims[0]; j++ )
				    {
					assert( hist->thresh[0][j - 1] < hist->thresh[0][j] );
					if( val <= hist->thresh[0][j] )
					    break;
				    }
				    addr[0] = j - 1;
				    icvInsertNode( hist, addr[0] )->value++;
				}
			    }
			}
			data[0] = (float *) ((uchar *) data[0] + step);
		    }
		}
		break;
	    case 2:
		{
		    float val;
		    float *data[CV_HIST_MAX_DIM];
		    int addr[CV_HIST_MAX_DIM];

		    data[0] = img[0];
		    data[1] = img[1];

		    for( y = 0; y < size.height; y++, mask += mask_step )
		    {
			for( x = 0; x < size.width; x++ )
			{
			    if( mask[x] )
			    {
				val = data[0][x];
				if( val >= hist->thresh[0][0] &&
				    val < hist->thresh[0][hist->dims[0]] )
				{
				    for( j = 1; j < hist->dims[0]; j++ )
				    {
					assert( hist->thresh[0][j - 1] < hist->thresh[0][j] );
					if( val <= hist->thresh[0][j] )
					    break;
				    }
				    addr[0] = j - 1;

				    val = data[1][x];
				    if( val >= hist->thresh[1][0] &&
					val < hist->thresh[1][hist->dims[1]] )
				    {
					for( j = 1; j < hist->dims[1]; j++ )
					{
					    assert( hist->thresh[1][j - 1] <
						    hist->thresh[1][j] );
					    if( val <= hist->thresh[1][j] )
						break;
					}
					addr[1] = j - 1;

					icvInsertNode( hist, addr[0] * hist->mdims[0] +
						       addr[1] )->value++;
				    }
				}
			    }
			}
			data[0] = (float *) ((uchar *) data[0] + step);
			data[1] = (float *) ((uchar *) data[1] + step);
		    }
		}
		break;
	    case 3:
		{
		    float val;
		    float *data[CV_HIST_MAX_DIM];
		    int addr[CV_HIST_MAX_DIM];

		    data[0] = img[0];
		    data[1] = img[1];
		    data[2] = img[2];

		    for( y = 0; y < size.height; y++, mask += mask_step )
		    {
			for( x = 0; x < size.width; x++ )
			{
			    if( mask[x] )
			    {
				val = data[0][x];
				if( val >= hist->thresh[0][0] &&
				    val < hist->thresh[0][hist->dims[0]] )
				{
				    for( j = 1; j < hist->dims[0]; j++ )
				    {
					assert( hist->thresh[0][j - 1] < hist->thresh[0][j] );
					if( val <= hist->thresh[0][j] )
					    break;
				    }
				    addr[0] = j - 1;

				    val = data[1][x];
				    if( val >= hist->thresh[1][0] &&
					val < hist->thresh[1][hist->dims[1]] )
				    {
					for( j = 1; j < hist->dims[1]; j++ )
					{
					    assert( hist->thresh[1][j - 1] <
						    hist->thresh[1][j] );
					    if( val <= hist->thresh[1][j] )
						break;
					}
					addr[1] = j - 1;

					val = data[2][x];
					if( val >= hist->thresh[2][0] &&
					    val < hist->thresh[2][hist->dims[2]] )
					{
					    for( j = 1; j < hist->dims[2]; j++ )
					    {
						assert( hist->thresh[2][j - 1] <
							hist->thresh[2][j] );
						if( val <= hist->thresh[2][j] )
						    break;
					    }
					    addr[2] = j - 1;

					    icvInsertNode( hist,
							   (int64) addr[0] * hist->mdims[0] +
							   addr[1] * hist->mdims[1] +
							   addr[2] )->value++;
					}
				    }
				}
			    }
			}
			data[0] = (float *) ((uchar *) data[0] + step);
			data[1] = (float *) ((uchar *) data[1] + step);
			data[2] = (float *) ((uchar *) data[2] + step);
		    }
		}
		break;
	    default:
		{
		    float val;
		    float *data[CV_HIST_MAX_DIM];
		    int addr[CV_HIST_MAX_DIM];

		    for( i = 0; i < dims; i++ )
			data[i] = img[i];

		    for( y = 0; y < size.height; y++, mask += mask_step )
		    {
			for( x = 0; x < size.width; x++ )
			{
			    if( mask[x] )
			    {
				for( i = 0; i < dims; i++ )
				{
				    val = data[i][x];
				    if( val < hist->thresh[i][0] ||
					val >= hist->thresh[i][hist->dims[i]] )
					goto next_point_float_tree;
				    else
				    {
					for( j = 1; j < hist->dims[i]; j++ )
					{
					    assert( hist->thresh[i][j - 1] <
						    hist->thresh[i][j] );
					    if( val <= hist->thresh[i][j] )
						break;
					}
				    }
				    addr[i] = j - 1;
				}
				(*cvGetHistValue_nD( hist, addr ))++;
			    }
			  next_point_float_tree:;
			}
			for( i = 0; i < dims; i++ )
			    data[i] = (float *) ((uchar *) data[i] + step);
		    }
		}
		break;
	    }
	else
	    switch (hist->c_dims)
	    {
	    case 1:
		{
		    float val;
		    float *data[CV_HIST_MAX_DIM];
		    int addr[CV_HIST_MAX_DIM];
		    float stp[CV_HIST_MAX_DIM];
		    float mn[CV_HIST_MAX_DIM];

		    data[0] = img[0];
		    mn[0] = hist->thresh[0][0];
		    stp[0] =
			hist->dims[0] / (hist->thresh[0][hist->dims[0]] - hist->thresh[0][0]);

		    for( y = 0; y < size.height; y++, mask += mask_step )
		    {
			for( x = 0; x < size.width; x++ )
			{
			    if( mask[x] )
			    {
				val = data[0][x];
				if( val >= hist->thresh[0][0] &&
				    val < hist->thresh[0][hist->dims[0]] )
				{
				    addr[0] = cvFloor( ((float) val - mn[0]) * stp[0] );
				    icvInsertNode( hist, addr[0] )->value++;
				}
			    }
			}
			data[0] = (float *) ((uchar *) data[0] + step);
		    }
		}
		break;
	    case 2:
		{
		    float val;
		    float *data[CV_HIST_MAX_DIM];
		    int addr[CV_HIST_MAX_DIM];
		    float stp[CV_HIST_MAX_DIM];
		    float mn[CV_HIST_MAX_DIM];

		    data[0] = img[0];
		    mn[0] = hist->thresh[0][0];
		    stp[0] =
			hist->dims[0] / (hist->thresh[0][hist->dims[0]] - hist->thresh[0][0]);

		    data[1] = img[1];
		    mn[1] = hist->thresh[1][0];
		    stp[1] =
			hist->dims[1] / (hist->thresh[1][hist->dims[1]] - hist->thresh[1][0]);

		    for( y = 0; y < size.height; y++, mask += mask_step )
		    {
			for( x = 0; x < size.width; x++ )
			{
			    if( mask[x] )
			    {
				val = data[0][x];
				if( val >= hist->thresh[0][0] &&
				    val < hist->thresh[0][hist->dims[0]] )
				{
				    addr[0] = cvFloor( ((float) val - mn[0]) * stp[0] );

				    val = data[1][x];
				    if( val >= hist->thresh[1][0] &&
					val < hist->thresh[1][hist->dims[1]] )
				    {
					addr[1] = cvFloor( ((float) val - mn[1]) * stp[1] );

					icvInsertNode( hist, addr[0] * hist->mdims[0] +
						       addr[1] )->value++;
				    }
				}
			    }
			}
			data[0] = (float *) ((uchar *) data[0] + step);
			data[1] = (float *) ((uchar *) data[1] + step);
		    }
		}
		break;
	    case 3:
		{
		    float val;
		    float *data[CV_HIST_MAX_DIM];
		    int addr[CV_HIST_MAX_DIM];
		    float stp[CV_HIST_MAX_DIM];
		    float mn[CV_HIST_MAX_DIM];

		    data[0] = img[0];
		    mn[0] = hist->thresh[0][0];
		    stp[0] =
			hist->dims[0] / (hist->thresh[0][hist->dims[0]] - hist->thresh[0][0]);

		    data[1] = img[1];
		    mn[1] = hist->thresh[1][0];
		    stp[1] =
			hist->dims[1] / (hist->thresh[1][hist->dims[1]] - hist->thresh[1][0]);

		    data[2] = img[2];
		    mn[2] = hist->thresh[2][0];
		    stp[2] =
			hist->dims[2] / (hist->thresh[2][hist->dims[2]] - hist->thresh[2][0]);

		    for( y = 0; y < size.height; y++ )
		    {
			for( x = 0; x < size.width; x++ )
			{
			    if( mask[x] )
			    {
				val = data[0][x];
				if( val >= hist->thresh[0][0] &&
				    val < hist->thresh[0][hist->dims[0]] )
				{
				    addr[0] = cvFloor( ((float) val - mn[0]) * stp[0] );

				    val = data[1][x];
				    if( val >= hist->thresh[1][0] &&
					val < hist->thresh[1][hist->dims[1]] )
				    {
					addr[1] = cvFloor( ((float) val - mn[1]) * stp[1] );

					val = data[2][x];
					if( val >= hist->thresh[2][0] &&
					    val < hist->thresh[2][hist->dims[2]] )
					{
					    addr[2] =
						cvFloor( ((float) val - mn[2]) * stp[2] );

					    icvInsertNode( hist, addr[0] * hist->mdims[0] +
							   addr[1] * hist->mdims[1] +
							   addr[2] )->value++;
					}
				    }
				}
			    }
			}
			data[0] = (float *) ((uchar *) data[0] + step);
			data[1] = (float *) ((uchar *) data[1] + step);
			data[2] = (float *) ((uchar *) data[2] + step);
		    }
		}
		break;
	    default:
		{
		    float val;
		    float *data[CV_HIST_MAX_DIM];
		    int addr[CV_HIST_MAX_DIM];
		    float stp[CV_HIST_MAX_DIM];
		    float mn[CV_HIST_MAX_DIM];

		    for( i = 0; i < dims; i++ )
		    {
			data[i] = img[i];
			mn[i] = hist->thresh[i][0];
			stp[i] =
			    hist->dims[i] / (hist->thresh[i][hist->dims[i]] -
					     hist->thresh[i][0]);
		    }

		    for( y = 0; y < size.height; y++, mask += mask_step )
		    {
			for( x = 0; x < size.width; x++ )
			{
			    if( mask[x] )
			    {
				for( i = 0; i < dims; i++ )
				{
				    val = data[i][x];
				    if( val < hist->thresh[i][0] ||
					val >= hist->thresh[i][hist->dims[i]] )
					goto next_pointe_float_tree;
				    else
					addr[i] = cvFloor( ((float) val - mn[i]) * stp[i] );
				}
				(*cvGetHistValue_nD( hist, addr ))++;
			    }
			  next_pointe_float_tree:;
			}
			for( i = 0; i < dims; i++ )
			    data[i] = (float *) ((uchar *) data[i] + step);
		    }
		}
		break;
	    }
    }

    return CV_NO_ERR;
}


/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:	  icvCalcBackProject...C1R
//    Purpose:	  Calculating back project of histogram
//    Context:
//    Parameters:
//    Returns:
//    Notes:
//F*/
IPCVAPI_IMPL( CvStatus, icvCalcBackProject8uC1R, (uchar ** img, int step,
						  uchar * dst, int dst_step,
						  CvSize size, CvHistogram * hist) )
{
    int i, dims, x, y;

    if( !img || !dst || !hist )
	return CV_NULLPTR_ERR;
    if( size.width <= 0 || size.height <= 0 )
	return CV_BADSIZE_ERR;
    if( size.width * (int) sizeof( **img ) > step )
	return CV_BADSIZE_ERR;
    if( size.width * (int) sizeof( **img ) > dst_step )
	return CV_BADSIZE_ERR;

    dims = hist->c_dims;
    for( i = 0; i < dims; i++ )
	if( !img[i] )
	    return CV_NULLPTR_ERR;

    for( i = 0; i < size.height; i++ )
	memset( dst + dst_step * i, 0, size.width );

    if( hist->type == CV_HIST_ARRAY )
    {
	switch (hist->c_dims)
	{
	case 1:
	    {
		uchar *data0 = img[0];
		uchar cache[256];
		int *chdims0 = hist->chdims[0];

		hist->array[hist->dims[0]] = 0;

		for( i = 0; i < 256; i++ )
		{
		    int v = chdims0[i + 128];

		    cache[i] = (uchar) hist->array[v];
		}

		for( y = 0; y < size.height; y++, data0 += step, dst += dst_step )
		{
		    for( x = 0; x <= size.width - 4; x += 4 )
		    {
			dst[x] = cache[data0[x]];
			dst[x + 1] = cache[data0[x + 1]];
			dst[x + 2] = cache[data0[x + 2]];
			dst[x + 3] = cache[data0[x + 3]];
		    }

		    for( ; x < size.width; x++ )
			dst[x] = cache[data0[x]];
		}
	    }
	    break;
	case 2:
	    if( (hist->mdims[0] + 1) * (hist->mdims[1] + 1) )
	    {
		uchar *data0 = img[0];
		uchar *data1 = img[1];
		int chdims0[256];
		int chdims1[256];
		int array[4096];
		int hstep = hist->mdims[0] + 1;

		for( y = 0, i = 0; y < hist->dims[0]; y++ )
		    for( x = 0; x < hist->dims[1]; x++, i++ )
			array[y * hstep + x] = cvRound( hist->array[i] );
		for( i = 0; i < hist->dims[1] + 1; i++ )
		    array[hist->dims[0] * hstep + i] = 0;
		for( i = 0; i < hist->dims[0]; i++ )
		    array[(i + 1) * hstep - 1] = 0;

		memcpy( chdims0, hist->chdims[0] + 128, 256 * sizeof( int ));
		memcpy( chdims1, hist->chdims[1] + 128, 256 * sizeof( int ));

		for( y = 0; y < size.height; y++, data0 += step, data1 += step,
		     dst += dst_step )
		{
		    for( x = 0; x < size.width - 2; x += 2 )
		    {
			int val0 = chdims0[data0[x]];
			int val1 = chdims1[data1[x]];

			dst[x] = (uchar) array[val0 + val1];
			val0 = chdims0[data0[x + 1]];
			val1 = chdims1[data1[x + 1]];
			dst[x + 1] = (uchar) array[val0 + val1];
		    }
		    for( ; x < size.width; x++ )
		    {
			int val0 = chdims0[data0[x]];
			int val1 = chdims1[data1[x]];

			dst[x] = (uchar) array[val0 + val1];
		    }
		}
	    }
	    else
	    {
		uchar *data0 = img[0];
		uchar *data1 = img[1];
		int chdims0[256];
		int chdims1[256];
		int hstep = hist->mdims[0] + 1;
		int *array = (int *) icvAlloc( hstep * (hist->dims[0] + 1) );

		for( y = 0, i = 0; y < hist->dims[0]; y++ )
		    for( x = 0; x < hist->dims[1]; x++, i++ )
			array[y * hstep + x] = cvRound( hist->array[i] );
		for( i = 0; i < hist->dims[1] + 1; i++ )
		    array[hist->dims[0] * hstep + i] = 0;
		for( i = 0; i < hist->dims[0]; i++ )
		    array[(i + 1) * hstep - 1] = 0;

		memcpy( chdims0, hist->chdims[0] + 128, 256 * sizeof( int ));
		memcpy( chdims1, hist->chdims[1] + 128, 256 * sizeof( int ));

		for( y = 0; y < size.height; y++, data0 += step, data1 += step,
		     dst += dst_step )
		{
		    for( x = 0; x < size.width - 2; x += 2 )
		    {
			int val0 = chdims0[data0[x]];
			int val1 = chdims1[data1[x]];

			dst[x] = (uchar) array[val0 + val1];
			val0 = chdims0[data0[x + 1]];
			val1 = chdims1[data1[x + 1]];
			dst[x + 1] = (uchar) array[val0 + val1];
		    }
		    for( ; x < size.width; x++ )
		    {
			int val0 = chdims0[data0[x]];
			int val1 = chdims1[data1[x]];

			dst[x] = (uchar) array[val0 + val1];
		    }
		}
		icvFree( &array );
	    }
	    break;
	case 3:
	    {
		uchar *data0 = img[0];
		uchar *data1 = img[1];
		uchar *data2 = img[2];

		for( y = 0; y < size.height; y++, data0 += step, data1 += step,
		     data2 += step, dst += dst_step )
		    for( x = 0; x < size.width; x++ )
		    {
			int val0 = hist->chdims[0][(int) data0[x] + 128];
			int val1 = hist->chdims[1][(int) data1[x] + 128];
			int val2 = hist->chdims[2][(int) data2[x] + 128];

			if( (val0 | val1 | val2) >= 0 )
			    dst[x] = (uchar) hist->array[val0 + val1 + val2];
		    }
	    }
	    break;
	default:
	    {
		uchar *data[CV_HIST_MAX_DIM];

		for( i = 0; i < hist->c_dims; i++ )
		    data[i] = img[i];

		for( y = 0; y < size.height; y++, dst += dst_step )
		{
		    for( x = 0; x < size.width; x++ )
		    {
			int addr = 0;

			for( i = 0; i < hist->c_dims; i++ )
			{
			    int val = hist->chdims[i][(int) data[i][x] + 128];

			    if( val >= 0 )
				addr += val;
			    else
				goto next;
			}
			dst[x] = (uchar) hist->array[addr];
		      next:;
		    }
		    for( i = 0; i < hist->c_dims; i++ )
			data[i] += step;
		}
	    }
	    break;
	}
    }
    else
    {
	switch (hist->c_dims)
	{
	case 1:
	    {
		uchar *data0 = img[0];
		uchar cache[256];
		int *chdims0 = hist->chdims[0];

		for( i = 0; i < 256; i++ )
		{
		    int v = chdims0[i + 128];
		    CvNode *node = icvFindNode( hist->root, v );

		    cache[i] = (uchar) (node ? node->value : 0);
		}

		for( y = 0; y < size.height; y++, data0 += step, dst += dst_step )
		{
		    for( x = 0; x <= size.width - 4; x += 4 )
		    {
			dst[x] = cache[data0[x]];
			dst[x + 1] = cache[data0[x + 1]];
			dst[x + 2] = cache[data0[x + 2]];
			dst[x + 3] = cache[data0[x + 3]];
		    }

		    for( ; x < size.width; x++ )
			dst[x] = cache[data0[x]];
		}
	    }
	    break;
	case 2:
	    if( (hist->mdims[0] + 1) * (hist->mdims[1] + 1) )
	    {
		uchar *data0 = img[0];
		uchar *data1 = img[1];
		int chdims0[256];
		int chdims1[256];
		int array[4096];
		int hstep = hist->mdims[0] + 1;

		for( y = 0, i = 0; y < hist->dims[0]; y++ )
		    for( x = 0; x < hist->dims[1]; x++, i++ )
		    {
			CvNode *node = icvFindNode( hist->root, i );

			array[y * hstep + x] = node ? cvRound( node->value ) : 0;
		    }
		for( i = 0; i < hist->dims[1] + 1; i++ )
		    array[hist->dims[0] * hstep + i] = 0;
		for( i = 0; i < hist->dims[0]; i++ )
		    array[(i + 1) * hstep - 1] = 0;

		memcpy( chdims0, hist->chdims[0] + 128, 256 * sizeof( int ));
		memcpy( chdims1, hist->chdims[1] + 128, 256 * sizeof( int ));

		for( y = 0; y < size.height; y++, data0 += step, data1 += step,
		     dst += dst_step )
		{
		    for( x = 0; x < size.width - 2; x += 2 )
		    {
			int val0 = chdims0[data0[x]];
			int val1 = chdims1[data1[x]];

			dst[x] = (uchar) array[val0 + val1];
			val0 = chdims0[data0[x + 1]];
			val1 = chdims1[data1[x + 1]];
			dst[x + 1] = (uchar) array[val0 + val1];
		    }
		    for( ; x < size.width; x++ )
		    {
			int val0 = chdims0[data0[x]];
			int val1 = chdims1[data1[x]];

			dst[x] = (uchar) array[val0 + val1];
		    }
		}
	    }
	    else
	    {
		uchar *data0 = img[0];
		uchar *data1 = img[1];
		int chdims0[256];
		int chdims1[256];
		int hstep = hist->mdims[0] + 1;
		int *array = (int *) icvAlloc( hstep * (hist->dims[0] + 1) );

		for( y = 0, i = 0; y < hist->dims[0]; y++ )
		    for( x = 0; x < hist->dims[1]; x++, i++ )
		    {
			CvNode *node = icvFindNode( hist->root, i );

			array[y * hstep + x] = node ? cvRound( node->value ) : 0;
		    }
		for( i = 0; i < hist->dims[1] + 1; i++ )
		    array[hist->dims[0] * hstep + i] = 0;
		for( i = 0; i < hist->dims[0]; i++ )
		    array[(i + 1) * hstep - 1] = 0;

		memcpy( chdims0, hist->chdims[0] + 128, 256 * sizeof( int ));
		memcpy( chdims1, hist->chdims[1] + 128, 256 * sizeof( int ));

		for( y = 0; y < size.height; y++, data0 += step, data1 += step,
		     dst += dst_step )
		{
		    for( x = 0; x < size.width - 2; x += 2 )
		    {
			int val0 = chdims0[data0[x]];
			int val1 = chdims1[data1[x]];

			dst[x] = (uchar) array[val0 + val1];
			val0 = chdims0[data0[x + 1]];
			val1 = chdims1[data1[x + 1]];
			dst[x + 1] = (uchar) array[val0 + val1];
		    }
		    for( ; x < size.width; x++ )
		    {
			int val0 = chdims0[data0[x]];
			int val1 = chdims1[data1[x]];

			dst[x] = (uchar) array[val0 + val1];
		    }
		}
		icvFree( &array );
	    }
	    break;
	case 3:
	    {
		uchar *data0 = img[0];
		uchar *data1 = img[1];
		uchar *data2 = img[2];

		for( y = 0; y < size.height; y++, data0 += step, data1 += step,
		     data2 += step, dst += dst_step )
		    for( x = 0; x < size.width; x++ )
		    {
			int val0 = hist->chdims[0][(int) data0[x] + 128];
			int val1 = hist->chdims[1][(int) data1[x] + 128];
			int val2 = hist->chdims[2][(int) data2[x] + 128];

			if( (val0 | val1 | val2) >= 0 )
			{
			    CvNode *node = icvFindNode( hist->root,
							(int64) val0 + val1 + val2 );

			    dst[x] = (uchar) (node ? node->value : 0);
			}
		    }
	    }
	    break;
	default:
	    {
		uchar *data[CV_HIST_MAX_DIM];

		for( i = 0; i < hist->c_dims; i++ )
		    data[i] = img[i];

		for( y = 0; y < size.height; y++, dst += dst_step )
		{
		    for( x = 0; x < size.width; x++ )
		    {
			int64 addr = 0;
			CvNode *node;

			for( i = 0; i < hist->c_dims; i++ )
			{
			    int val = hist->chdims[i][(int) data[i][x] + 128];

			    if( val >= 0 )
				addr += val;
			    else
				goto next_tree;
			}
			node = icvFindNode( hist->root, addr );
			dst[x] = (uchar) (node ? node->value : 0);
		      next_tree:;
		    }
		    for( i = 0; i < hist->c_dims; i++ )
			data[i] += step;
		}
	    }
	    break;
	}
    }
    return CV_NO_ERR;
}

/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:	  icvCalcBackProject...C1R
//    Purpose:	  Calculating back project of histogram
//    Context:
//    Parameters:
//    Returns:
//    Notes:
//F*/
IPCVAPI_IMPL( CvStatus, icvCalcBackProject8sC1R, (char **img, int step,
						  char *dst, int dst_step,
						  CvSize size, CvHistogram * hist) )
{
    int i, dims, x, y;

    if( !img || !dst || !hist )
	return CV_NULLPTR_ERR;
    if( size.width <= 0 || size.height <= 0 )
	return CV_BADSIZE_ERR;
    if( size.width * (int) sizeof( **img ) > step )
	return CV_BADSIZE_ERR;
    if( size.width * (int) sizeof( **img ) > dst_step )
	return CV_BADSIZE_ERR;

    dims = hist->c_dims;
    for( i = 0; i < dims; i++ )
	if( !img[i] )
	    return CV_NULLPTR_ERR;

    for( i = 0; i < size.height; i++ )
	memset( dst + dst_step * i, 0, size.width );

    if( hist->type == CV_HIST_ARRAY )
    {
	switch (hist->c_dims)
	{
	case 1:
	    {
		char *data0 = img[0];
		char cache[256];

		hist->array[hist->dims[0]] = 0;

		for( i = 0; i < 256; i++ )
		{
		    int v = hist->chdims[0][i + 128];

		    if( v >= 0 )
			cache[i] = (char) hist->array[v];
		    else
			cache[i] = 0;
		}

		for( y = 0; y < size.height; y++, data0 += step, dst += dst_step )
		{
		    for( x = 0; x <= size.width - 4; x += 4 )
		    {
			dst[x] = cache[data0[x]];
			dst[x + 1] = cache[data0[x + 1]];
			dst[x + 2] = cache[data0[x + 2]];
			dst[x + 3] = cache[data0[x + 3]];
		    }

		    for( ; x < size.width; x++ )
		    {
			dst[x] = cache[data0[x]];
		    }
		}
	    }
	    break;
	case 2:
	    if( (hist->mdims[0] + 1) * (hist->mdims[1] + 1) )
	    {
		char *data0 = img[0];
		char *data1 = img[1];
		int chdims0[256];
		int chdims1[256];
		int array[4096];
		int hstep = hist->mdims[0] + 1;

		for( y = 0, i = 0; y < hist->dims[0]; y++ )
		    for( x = 0; x < hist->dims[1]; x++, i++ )
			array[y * hstep + x] = cvRound( hist->array[i] );
		for( i = 0; i < hist->dims[1] + 1; i++ )
		    array[hist->dims[0] * hstep + i] = 0;
		for( i = 0; i < hist->dims[0]; i++ )
		    array[(i + 1) * hstep - 1] = 0;

		memcpy( chdims0, hist->chdims[0], 256 * sizeof( int ));
		memcpy( chdims1, hist->chdims[1], 256 * sizeof( int ));

		for( y = 0; y < size.height; y++, data0 += step, data1 += step,
		     dst += dst_step )
		{
		    for( x = 0; x < size.width; x += 2 )
		    {
			int val0 = chdims0[data0[x] + 128];
			int val1 = chdims1[data1[x] + 128];

			dst[x] = (uchar) array[val0 + val1];
			val0 = chdims0[data0[x + 1] + 128];
			val1 = chdims1[data1[x + 1] + 128];
			dst[x + 1] = (uchar) array[val0 + val1];
		    }
		    for( ; x < size.width; x++ )
		    {
			int val0 = chdims0[data0[x] + 128];
			int val1 = chdims1[data1[x] + 128];

			dst[x] = (uchar) array[val0 + val1];
		    }
		}
	    }
	    else
	    {
		char *data0 = img[0];
		char *data1 = img[1];
		int chdims0[256];
		int chdims1[256];
		int hstep = hist->mdims[0] + 1;
		int *array = (int *) icvAlloc( hstep * (hist->dims[0] + 1) );

		for( y = 0, i = 0; y < hist->dims[0]; y++ )
		    for( x = 0; x < hist->dims[1]; x++, i++ )
			array[y * hstep + x] = cvRound( hist->array[i] );
		for( i = 0; i < hist->dims[1] + 1; i++ )
		    array[hist->dims[0] * hstep + i] = 0;
		for( i = 0; i < hist->dims[0]; i++ )
		    array[(i + 1) * hstep - 1] = 0;

		memcpy( chdims0, hist->chdims[0], 256 * sizeof( int ));
		memcpy( chdims1, hist->chdims[1], 256 * sizeof( int ));

		for( y = 0; y < size.height; y++, data0 += step, data1 += step,
		     dst += dst_step )
		{
		    for( x = 0; x < size.width; x += 2 )
		    {
			int val0 = chdims0[data0[x] + 128];
			int val1 = chdims1[data1[x] + 128];

			dst[x] = (uchar) array[val0 + val1];
			val0 = chdims0[data0[x + 1] + 128];
			val1 = chdims1[data1[x + 1] + 128];
			dst[x + 1] = (uchar) array[val0 + val1];
		    }
		    for( ; x < size.width; x++ )
		    {
			int val0 = chdims0[data0[x] + 128];
			int val1 = chdims1[data1[x] + 128];

			dst[x] = (uchar) array[val0 + val1];
		    }
		}
		icvFree( &array );
	    }
	    break;
	case 3:
	    {
		char *data0 = img[0];
		char *data1 = img[1];
		char *data2 = img[2];

		for( y = 0; y < size.height; y++, data0 += step, data1 += step,
		     data2 += step, dst += dst_step )
		    for( x = 0; x < size.width; x++ )
		    {
			int val0 = hist->chdims[0][(int) data0[x] + 128];
			int val1 = hist->chdims[1][(int) data1[x] + 128];
			int val2 = hist->chdims[2][(int) data2[x] + 128];

			if( (val0 | val1 | val2) >= 0 )
			    dst[x] = (char) hist->array[val0 + val1 + val2];
		    }
	    }
	    break;
	default:
	    {
		char *data[CV_HIST_MAX_DIM];

		for( i = 0; i < hist->c_dims; i++ )
		    data[i] = img[i];

		for( y = 0; y < size.height; y++, dst += dst_step )
		{
		    for( x = 0; x < size.width; x++ )
		    {
			int addr = 0;

			for( i = 0; i < hist->c_dims; i++ )
			{
			    int val = hist->chdims[i][(int) data[i][x] + 128];

			    if( val >= 0 )
				addr += val;
			    else
				goto next;
			}
			dst[x] = (char) hist->array[addr];
		      next:;
		    }
		    for( i = 0; i < hist->c_dims; i++ )
			data[i] += step;
		}
	    }
	    break;
	}
    }
    else
    {
	switch (hist->c_dims)
	{
	case 1:
	    {
		char *data0 = img[0];
		char cache[256];

		for( i = 0; i < 256; i++ )
		{
		    int v = hist->chdims[0][i + 128];

		    if( v >= 0 )
		    {
			CvNode *node = icvFindNode( hist->root, v );

			cache[i] = (uchar) (node ? node->value : 0);
		    }
		    else
			cache[i] = 0;
		}

		for( y = 0; y < size.height; y++, data0 += step, dst += dst_step )
		{
		    for( x = 0; x <= size.width - 4; x += 4 )
		    {
			dst[x] = cache[data0[x]];
			dst[x + 1] = cache[data0[x + 1]];
			dst[x + 2] = cache[data0[x + 2]];
			dst[x + 3] = cache[data0[x + 3]];
		    }

		    for( ; x < size.width; x++ )
		    {
			dst[x] = cache[data0[x]];
		    }
		}
	    }
	    break;
	case 2:
	    if( (hist->mdims[0] + 1) * (hist->mdims[1] + 1) )
	    {
		char *data0 = img[0];
		char *data1 = img[1];
		int chdims0[256];
		int chdims1[256];
		int array[4096];
		int hstep = hist->mdims[0] + 1;

		for( y = 0, i = 0; y < hist->dims[0]; y++ )
		    for( x = 0; x < hist->dims[1]; x++, i++ )
		    {
			CvNode *node = icvFindNode( hist->root, i );

			array[y * hstep + x] = node ? cvRound( node->value ) : 0;
		    }
		for( i = 0; i < hist->dims[1] + 1; i++ )
		    array[hist->dims[0] * hstep + i] = 0;
		for( i = 0; i < hist->dims[0]; i++ )
		    array[(i + 1) * hstep - 1] = 0;

		memcpy( chdims0, hist->chdims[0], 256 * sizeof( int ));
		memcpy( chdims1, hist->chdims[1], 256 * sizeof( int ));

		for( y = 0; y < size.height; y++, data0 += step, data1 += step,
		     dst += dst_step )
		{
		    for( x = 0; x < size.width; x += 2 )
		    {
			int val0 = chdims0[data0[x] + 128];
			int val1 = chdims1[data1[x] + 128];

			dst[x] = (uchar) array[val0 + val1];
			val0 = chdims0[data0[x + 1] + 128];
			val1 = chdims1[data1[x + 1] + 128];
			dst[x + 1] = (uchar) array[val0 + val1];
		    }
		    for( ; x < size.width; x++ )
		    {
			int val0 = chdims0[data0[x] + 128];
			int val1 = chdims1[data1[x] + 128];

			dst[x] = (uchar) array[val0 + val1];
		    }
		}
	    }
	    else
	    {
		char *data0 = img[0];
		char *data1 = img[1];
		int chdims0[256];
		int chdims1[256];
		int hstep = hist->mdims[0] + 1;
		int *array = (int *) icvAlloc( hstep * (hist->dims[0] + 1) );

		for( y = 0, i = 0; y < hist->dims[0]; y++ )
		    for( x = 0; x < hist->dims[1]; x++, i++ )
		    {
			CvNode *node = icvFindNode( hist->root, i );

			array[y * hstep + x] = node ? cvRound( node->value ) : 0;
		    }
		for( i = 0; i < hist->dims[1] + 1; i++ )
		    array[hist->dims[0] * hstep + i] = 0;
		for( i = 0; i < hist->dims[0]; i++ )
		    array[(i + 1) * hstep - 1] = 0;

		memcpy( chdims0, hist->chdims[0], 256 * sizeof( int ));
		memcpy( chdims1, hist->chdims[1], 256 * sizeof( int ));

		for( y = 0; y < size.height; y++, data0 += step, data1 += step,
		     dst += dst_step )
		{
		    for( x = 0; x < size.width; x += 2 )
		    {
			int val0 = chdims0[data0[x] + 128];
			int val1 = chdims1[data1[x] + 128];

			dst[x] = (uchar) array[val0 + val1];
			val0 = chdims0[data0[x + 1] + 128];
			val1 = chdims1[data1[x + 1] + 128];
			dst[x + 1] = (uchar) array[val0 + val1];
		    }
		    for( ; x < size.width; x++ )
		    {
			int val0 = chdims0[data0[x] + 128];
			int val1 = chdims1[data1[x] + 128];

			dst[x] = (uchar) array[val0 + val1];
		    }
		}
		icvFree( &array );
	    }
	    break;
	case 3:
	    {
		char *data0 = img[0];
		char *data1 = img[1];
		char *data2 = img[2];

		for( y = 0; y < size.height; y++, data0 += step, data1 += step,
		     data2 += step, dst += dst_step )
		    for( x = 0; x < size.width; x++ )
		    {
			int val0 = hist->chdims[0][(int) data0[x] + 128];
			int val1 = hist->chdims[1][(int) data1[x] + 128];
			int val2 = hist->chdims[2][(int) data2[x] + 128];

			if( (val0 | val1 | val2) >= 0 )
			{
			    CvNode *node = icvFindNode( hist->root,
							(int64) val0 + val1 + val2 );

			    dst[x] = (char) (node ? node->value : 0);
			}
		    }
	    }
	    break;
	default:
	    {
		char *data[CV_HIST_MAX_DIM];

		for( i = 0; i < hist->c_dims; i++ )
		    data[i] = img[i];

		for( y = 0; y < size.height; y++, dst += dst_step )
		{
		    for( x = 0; x < size.width; x++ )
		    {
			int addr = 0;
			CvNode *node;

			for( i = 0; i < hist->c_dims; i++ )
			{
			    int val = hist->chdims[i][(int) data[i][x] + 128];

			    if( val >= 0 )
				addr += val;
			    else
				goto next_tree;
			}
			node = icvFindNode( hist->root, addr );
			dst[x] = (char) (node ? node->value : 0);
		      next_tree:;
		    }
		    for( i = 0; i < hist->c_dims; i++ )
			data[i] += step;
		}
	    }
	    break;
	}
    }

    return CV_NO_ERR;
}


/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:	  icvCalcBackProject...C1R
//    Purpose:	  Calculating back project of histogram
//    Context:
//    Parameters:
//    Returns:
//    Notes:
//F*/
IPCVAPI_IMPL( CvStatus, icvCalcBackProject32fC1R, (float **img, int step,
						   float *dst, int dst_step,
						   CvSize size, CvHistogram * hist) )
{
    int i, j, dims;

    if( !img || !dst || !hist )
	return CV_NULLPTR_ERR;
    if( size.width <= 0 || size.height <= 0 )
	return CV_BADSIZE_ERR;
    if( size.width * (int) sizeof( **img ) > step )
	return CV_BADSIZE_ERR;
    if( size.width * (int) sizeof( *dst ) > dst_step )
	return CV_BADSIZE_ERR;

    dims = hist->c_dims;
    for( i = 0; i < dims; i++ )
	if( !img[i] )
	    return CV_NULLPTR_ERR;

    for( i = 0; i < size.height; i++ )
	memset( (char *) dst + dst_step * i, 0, size.width * sizeof( *dst ));

    if( hist->flags & CV_HIST_UNIFORM )
    {
	float val;
	float *data[CV_HIST_MAX_DIM];
	int x, y;
	int addr[CV_HIST_MAX_DIM];
	float stp[CV_HIST_MAX_DIM];
	float mn[CV_HIST_MAX_DIM];

	for( i = 0; i < dims; i++ )
	{
	    data[i] = img[i];
	    mn[i] = hist->thresh[i][0];
	    stp[i] = hist->dims[i] / (hist->thresh[i][hist->dims[i]] - hist->thresh[i][0]);
	}

	for( y = 0; y < size.height; y++ )
	{
	    for( x = 0; x < size.width; x++ )
	    {
		for( i = 0; i < dims; i++ )
		{
		    val = data[i][x];
		    if( val < hist->thresh[i][0] || val > hist->thresh[i][hist->dims[i]] )
		    {
			dst[x] = 0;
			goto next_pointe_float;
		    }
		    else
			addr[i] = cvFloor( ((float) val - mn[i]) * stp[i] );
		}
		dst[x] = (float) cvQueryHistValue_nD( hist, addr );
	      next_pointe_float:;
	    }
	    for( i = 0; i < dims; i++ )
		data[i] = (float *) ((uchar *) data[i] + step);
	    dst = (float *) ((uchar *) dst + dst_step);
	}
    }
    else
    {
	float val;
	float *data[CV_HIST_MAX_DIM];
	int x, y;
	int addr[CV_HIST_MAX_DIM];

	for( i = 0; i < dims; i++ )
	    data[i] = img[i];

	for( y = 0; y < size.height; y++ )
	{
	    for( x = 0; x < size.width; x++ )
	    {
		for( i = 0; i < dims; i++ )
		{
		    val = data[i][x];
		    if( val < hist->thresh[i][0] || val > hist->thresh[i][hist->dims[i]] )
		    {
			dst[x] = 0;
			goto next_point_float;
		    }
		    else
		    {
			for( j = 1; j < hist->dims[i]; j++ )
			{
			    assert( hist->thresh[i][j - 1] < hist->thresh[i][j] );
			    if( val <= hist->thresh[i][j] )
				break;
			}
		    }
		    addr[i] = j - 1;
		}
		dst[x] = (float) cvQueryHistValue_nD( hist, addr );
	      next_point_float:;
	    }
	    for( i = 0; i < dims; i++ )
		data[i] = (float *) ((uchar *) data[i] + step);
	    dst = (float *) ((uchar *) dst + dst_step);
	}
    }

    return CV_NO_ERR;
}

#define _CHECK(func)		      \
{				      \
    CvStatus res = func;	      \
    if(res != CV_NO_ERR) return res;  \
}


/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:	  icvCalcBackProjectPatch...C1R
//    Purpose:	  Calculating back project patch of histogram
//    Context:
//    Parameters:
//    Returns:
//    Notes:
//F*/
IPCVAPI_IMPL( CvStatus, icvCalcBackProjectPatch8uC1R, (uchar ** src, int src_step,
						       float *dst, int dst_step,
						       CvSize roi,
						       CvSize range,
						       CvHistogram * hist,
						       CvCompareMethod method,
						       float norm_factor) )
{
    CvHistogram *model = 0;
    int i, x, y;
    uchar *img[CV_HIST_MAX_DIM];
    CvSize rng = cvSize( range.width * 2 + 1, range.height * 2 + 1 );

    if( !hist || !src || !dst )
	return CV_NULLPTR_ERR;
    if( roi.width < (range.width * 2 + 1) || roi.height < (range.height * 2 + 1) )
	return CV_BADSIZE_ERR;
    for( i = 0; i < hist->c_dims; i++ )
	if( (img[i] = src[i]) == 0 )
	    return CV_NULLPTR_ERR;

    cvCopyHist( hist, &model );

    /* top */
    for( y = 0; y < range.height; y++, dst = (float *) ((char *) dst + dst_step) )
    {
	/* u.left */
	for( x = 0; x < range.width; x++ )
	{
	    _CHECK( icvCalcHist8uC1R( img, src_step,
				      cvSize( range.width + x + 1, range.height + y + 1 ),
				      model, 0 ));
	    cvNormalizeHist( model, norm_factor );
	    dst[x] = (float) cvCompareHist( hist, model, method );
	}
	/* center */
	for( ; x < roi.width - range.width; x++ )
	{
	    _CHECK( icvCalcHist8uC1R( img, src_step,
				      cvSize( rng.width, range.height + y + 1 ), model, 0 ));
	    cvNormalizeHist( model, norm_factor );
	    dst[x] = (float) cvCompareHist( hist, model, method );
	    for( i = 0; i < hist->c_dims; i++ )
		img[i]++;
	}
	/* u.right */
	for( ; x < roi.width; x++ )
	{
	    _CHECK( icvCalcHist8uC1R( img, src_step,
				      cvSize( roi.width - x + range.width,
					      range.height + y + 1 ), model, 0 ));
	    cvNormalizeHist( model, norm_factor );
	    dst[x] = (float) cvCompareHist( hist, model, method );
	    for( i = 0; i < hist->c_dims; i++ )
		img[i]++;
	}
	for( i = 0; i < hist->c_dims; i++ )
	    img[i] = src[0];
    }

    /* center */
    for( ; y < roi.height - range.height; y++, dst = (float *) ((char *) dst + dst_step) )
    {
	/* u.left */
	for( x = 0; x < range.width; x++ )
	{
	    _CHECK( icvCalcHist8uC1R( img, src_step, cvSize( range.width + x + 1, rng.height ),
				      model, 0 ));
	    cvNormalizeHist( model, norm_factor );
	    dst[x] = (float) cvCompareHist( hist, model, method );
	}
	/* center */
	for( ; x < roi.width - range.width; x++ )
	{
	    _CHECK( icvCalcHist8uC1R( img, src_step, rng, model, 0 ));
	    cvNormalizeHist( model, norm_factor );
	    dst[x] = (float) cvCompareHist( hist, model, method );
	    for( i = 0; i < hist->c_dims; i++ )
		img[i]++;
	}
	/* u.right */
	for( ; x < roi.width; x++ )
	{
	    _CHECK( icvCalcHist8uC1R( img, src_step,
				      cvSize( roi.width - x + range.width, rng.height ),
				      model, 0 ));
	    cvNormalizeHist( model, norm_factor );
	    dst[x] = (float) cvCompareHist( hist, model, method );
	    for( i = 0; i < hist->c_dims; i++ )
		img[i]++;
	}
	for( i = 0; i < hist->c_dims; i++ )
	    img[i] += src_step - roi.width + range.width;
    }
    /* bottom */
    for( ; y < roi.height; y++, dst = (float *) ((char *) dst + dst_step) )
    {
	/* u.left */
	for( x = 0; x < range.width; x++ )
	{
	    _CHECK( icvCalcHist8uC1R( img, src_step,
				      cvSize( range.width + x + 1,
					      roi.height - y + range.height ), model, 0 ));
	    cvNormalizeHist( model, norm_factor );
	    dst[x] = (float) cvCompareHist( hist, model, method );
	}
	/* center */
	for( ; x < roi.width - range.width; x++ )
	{
	    _CHECK( icvCalcHist8uC1R( img, src_step,
				      cvSize( rng.width, roi.height - y + range.height ),
				      model, 0 ));
	    cvNormalizeHist( model, norm_factor );
	    dst[x] = (float) cvCompareHist( hist, model, method );
	    for( i = 0; i < hist->c_dims; i++ )
		img[i]++;
	}
	/* u.right */
	for( ; x < roi.width; x++ )
	{
	    _CHECK( icvCalcHist8uC1R( img, src_step,
				      cvSize( roi.width - x + range.width,
					      roi.height - y + range.height ), model, 0 ));
	    cvNormalizeHist( model, norm_factor );
	    dst[x] = (float) cvCompareHist( hist, model, method );
	    for( i = 0; i < hist->c_dims; i++ )
		img[i]++;
	}
	for( i = 0; i < hist->c_dims; i++ )
	    img[i] += src_step - roi.width + range.width;
    }

    cvReleaseHist( &model );
    return CV_NO_ERR;
}				/* icvCalcBackProjectPatch8uC1R */

/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:	  icvCalcBackProjectPatch...C1R
//    Purpose:	  Calculating back project patch of histogram
//    Context:
//    Parameters:
//    Returns:
//    Notes:
//F*/
IPCVAPI_IMPL( CvStatus, icvCalcBackProjectPatch8sC1R, (char **src, int src_step,
						       float *dst, int dst_step,
						       CvSize roi,
						       CvSize range,
						       CvHistogram * hist,
						       CvCompareMethod method,
						       float norm_factor) )
{
    CvHistogram *model = 0;
    int i, x, y;
    char *img[CV_HIST_MAX_DIM];
    CvSize rng = cvSize( range.width * 2 + 1, range.height * 2 + 1 );

    if( !hist || !src || !dst )
	return CV_NULLPTR_ERR;
    if( roi.width < (range.width * 2 + 1) || roi.height < (range.height * 2 + 1) )
	return CV_BADSIZE_ERR;
    for( i = 0; i < hist->c_dims; i++ )
	if( (img[i] = src[i]) == 0 )
	    return CV_NULLPTR_ERR;

    cvCopyHist( hist, &model );

    /* top */
    for( y = 0; y < range.height; y++, dst = (float *) ((char *) dst + dst_step) )
    {
	/* u.left */
	for( x = 0; x < range.width; x++ )
	{
	    _CHECK( icvCalcHist8sC1R( img, src_step,
				      cvSize( range.width + x + 1, range.height + y + 1 ),
				      model, 0 ));
	    cvNormalizeHist( model, norm_factor );
	    dst[x] = (float) cvCompareHist( hist, model, method );
	}
	/* center */
	for( ; x < roi.width - range.width; x++ )
	{
	    _CHECK( icvCalcHist8sC1R( img, src_step,
				      cvSize( rng.width, range.height + y + 1 ), model, 0 ));
	    cvNormalizeHist( model, norm_factor );
	    dst[x] = (float) cvCompareHist( hist, model, method );
	    for( i = 0; i < hist->c_dims; i++ )
		img[i]++;
	}
	/* u.right */
	for( ; x < roi.width; x++ )
	{
	    _CHECK( icvCalcHist8sC1R( img, src_step,
				      cvSize( roi.width - x + range.width,
					      range.height + y + 1 ), model, 0 ));
	    cvNormalizeHist( model, norm_factor );
	    dst[x] = (float) cvCompareHist( hist, model, method );
	    for( i = 0; i < hist->c_dims; i++ )
		img[i]++;
	}
	for( i = 0; i < hist->c_dims; i++ )
	    img[i] = src[0];
    }

    /* center */
    for( ; y < roi.height - range.height; y++, dst = (float *) ((char *) dst + dst_step) )
    {
	/* u.left */
	for( x = 0; x < range.width; x++ )
	{
	    _CHECK( icvCalcHist8sC1R( img, src_step, cvSize( range.width + x + 1, rng.height ),
				      model, 0 ));
	    cvNormalizeHist( model, norm_factor );
	    dst[x] = (float) cvCompareHist( hist, model, method );
	}
	/* center */
	for( ; x < roi.width - range.width; x++ )
	{
	    _CHECK( icvCalcHist8sC1R( img, src_step, rng, model, 0 ));
	    cvNormalizeHist( model, norm_factor );
	    dst[x] = (float) cvCompareHist( hist, model, method );
	    for( i = 0; i < hist->c_dims; i++ )
		img[i]++;
	}
	/* u.right */
	for( ; x < roi.width; x++ )
	{
	    _CHECK( icvCalcHist8sC1R( img, src_step,
				      cvSize( roi.width - x + range.width, rng.height ),
				      model, 0 ));
	    cvNormalizeHist( model, norm_factor );
	    dst[x] = (float) cvCompareHist( hist, model, method );
	    for( i = 0; i < hist->c_dims; i++ )
		img[i]++;
	}
	for( i = 0; i < hist->c_dims; i++ )
	    img[i] += src_step - roi.width + range.width;
    }
    /* bottom */
    for( ; y < roi.height; y++, dst = (float *) ((char *) dst + dst_step) )
    {
	/* u.left */
	for( x = 0; x < range.width; x++ )
	{
	    _CHECK( icvCalcHist8sC1R( img, src_step,
				      cvSize( range.width + x + 1,
					      roi.height - y + range.height ), model, 0 ));
	    cvNormalizeHist( model, norm_factor );
	    dst[x] = (float) cvCompareHist( hist, model, method );
	}
	/* center */
	for( ; x < roi.width - range.width; x++ )
	{
	    _CHECK( icvCalcHist8sC1R( img, src_step,
				      cvSize( rng.width, roi.height - y + range.height ),
				      model, 0 ));
	    cvNormalizeHist( model, norm_factor );
	    dst[x] = (float) cvCompareHist( hist, model, method );
	    for( i = 0; i < hist->c_dims; i++ )
		img[i]++;
	}
	/* u.right */
	for( ; x < roi.width; x++ )
	{
	    _CHECK( icvCalcHist8sC1R( img, src_step,
				      cvSize( roi.width - x + range.width,
					      roi.height - y + range.height ), model, 0 ));
	    cvNormalizeHist( model, norm_factor );
	    dst[x] = (float) cvCompareHist( hist, model, method );
	    for( i = 0; i < hist->c_dims; i++ )
		img[i]++;
	}
	for( i = 0; i < hist->c_dims; i++ )
	    img[i] += src_step - roi.width + range.width;
    }

    cvReleaseHist( &model );
    return CV_NO_ERR;
}				/* icvCalcBackProjectPatch8sC1R */

/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:	  icvCalcBackProjectPatch...C1R
//    Purpose:	  Calculating back project patch of histogram
//    Context:
//    Parameters:
//    Returns:
//    Notes:
//F*/
IPCVAPI_IMPL( CvStatus, icvCalcBackProjectPatch32fC1R, (float **src, int src_step,
							float *dst, int dst_step,
							CvSize roi,
							CvSize range,
							CvHistogram * hist,
							CvCompareMethod method,
							float norm_factor) )
{
    CvHistogram *model = 0;
    int i, x, y;
    float *img[CV_HIST_MAX_DIM];
    CvSize rng = cvSize( range.width * 2 + 1, range.height * 2 + 1 );

    if( !hist || !src || !dst )
	return CV_NULLPTR_ERR;
    if( roi.width < (range.width * 2 + 1) || roi.height < (range.height * 2 + 1) )
	return CV_BADSIZE_ERR;
    for( i = 0; i < hist->c_dims; i++ )
	if( (img[i] = src[i]) == 0 )
	    return CV_NULLPTR_ERR;

    cvCopyHist( hist, &model );

    /* top */
    for( y = 0; y < range.height; y++, dst = (float *) ((char *) dst + dst_step) )
    {
	/* u.left */
	for( x = 0; x < range.width; x++ )
	{
	    _CHECK( icvCalcHist32fC1R( img, src_step,
				       cvSize( range.width + x + 1, range.height + y + 1 ),
				       model, 0 ));
	    cvNormalizeHist( model, norm_factor );
	    dst[x] = (float) cvCompareHist( hist, model, method );
	}
	/* center */
	for( ; x < roi.width - range.width; x++ )
	{
	    _CHECK( icvCalcHist32fC1R( img, src_step,
				       cvSize( rng.width, range.height + y + 1 ), model, 0 ));
	    cvNormalizeHist( model, norm_factor );
	    dst[x] = (float) cvCompareHist( hist, model, method );
	    for( i = 0; i < hist->c_dims; i++ )
		img[i]++;
	}
	/* u.right */
	for( ; x < roi.width; x++ )
	{
	    _CHECK( icvCalcHist32fC1R( img, src_step,
				       cvSize( roi.width - x + range.width,
					       range.height + y + 1 ), model, 0 ));
	    cvNormalizeHist( model, norm_factor );
	    dst[x] = (float) cvCompareHist( hist, model, method );
	    for( i = 0; i < hist->c_dims; i++ )
		img[i]++;
	}
	for( i = 0; i < hist->c_dims; i++ )
	    img[i] = src[0];
    }

    /* center */
    for( ; y < roi.height - range.height; y++, dst = (float *) ((char *) dst + dst_step) )
    {
	/* u.left */
	for( x = 0; x < range.width; x++ )
	{
	    _CHECK( icvCalcHist32fC1R
		    ( img, src_step, cvSize( range.width + x + 1, rng.height ), model, 0 ));
	    cvNormalizeHist( model, norm_factor );
	    dst[x] = (float) cvCompareHist( hist, model, method );
	}
	/* center */
	for( ; x < roi.width - range.width; x++ )
	{
	    _CHECK( icvCalcHist32fC1R( img, src_step, rng, model, 0 ));
	    cvNormalizeHist( model, norm_factor );
	    dst[x] = (float) cvCompareHist( hist, model, method );
	    for( i = 0; i < hist->c_dims; i++ )
		img[i]++;
	}
	/* u.right */
	for( ; x < roi.width; x++ )
	{
	    _CHECK( icvCalcHist32fC1R( img, src_step,
				       cvSize( roi.width - x + range.width, rng.height ),
				       model, 0 ));
	    cvNormalizeHist( model, norm_factor );
	    dst[x] = (float) cvCompareHist( hist, model, method );
	    for( i = 0; i < hist->c_dims; i++ )
		img[i]++;
	}
	for( i = 0; i < hist->c_dims; i++ )
	    img[i] += src_step / sizeof( float ) - roi.width + range.width;
    }
    /* bottom */
    for( ; y < roi.height; y++, dst = (float *) ((char *) dst + dst_step) )
    {
	/* u.left */
	for( x = 0; x < range.width; x++ )
	{
	    _CHECK( icvCalcHist32fC1R( img, src_step,
				       cvSize( range.width + x + 1,
					       roi.height - y + range.height ), model, 0 ));
	    cvNormalizeHist( model, norm_factor );
	    dst[x] = (float) cvCompareHist( hist, model, method );
	}
	/* center */
	for( ; x < roi.width - range.width; x++ )
	{
	    _CHECK( icvCalcHist32fC1R( img, src_step,
				       cvSize( rng.width, roi.height - y + range.height ),
				       model, 0 ));
	    cvNormalizeHist( model, norm_factor );
	    dst[x] = (float) cvCompareHist( hist, model, method );
	    for( i = 0; i < hist->c_dims; i++ )
		img[i]++;
	}
	/* u.right */
	for( ; x < roi.width; x++ )
	{
	    _CHECK( icvCalcHist32fC1R( img, src_step,
				       cvSize( roi.width - x + range.width,
					       roi.height - y + range.height ), model, 0 ));
	    cvNormalizeHist( model, norm_factor );
	    dst[x] = (float) cvCompareHist( hist, model, method );
	    for( i = 0; i < hist->c_dims; i++ )
		img[i]++;
	}
	for( i = 0; i < hist->c_dims; i++ )
	    img[i] += src_step / sizeof( float ) - roi.width + range.width;
    }

    cvReleaseHist( &model );
    return CV_NO_ERR;
}				/* icvCalcBackProjectPatch32fC1R */


CvStatus
icvClearHist( CvHistogram * hist )
{
    assert( hist );
    if( hist->type == CV_HIST_ARRAY )
    {
	assert( hist->array );
	memset( hist->array, 0, hist->dims[0] * hist->mdims[0] * sizeof( *hist->array ));
    }
    else			/* tree histogram */
    {
	if( hist->set )
	    cvClearSet( hist->set );
	hist->root = 0;
    }
    return CV_NO_ERR;
}


/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name:	  icvCalcBayesianProb
//    Purpose:	  Calculating bayes probabilistic histograms
//    Context:
//    Parameters:
//    Returns:
//    Notes:
//F*/
void
cvCalcBayesianProb( CvHistogram ** src, int number, CvHistogram ** dst )
{
    int i, j;
    int size;

    CV_FUNCNAME( "cvCalcBayesianProb" );
    __BEGIN__;

    /* Check for bad args */
    if( !src || !dst )
	CV_ERROR( IPL_HeaderIsNull, "Pointer to histogram array is null" );
    if( number < 2 )
	CV_ERROR( IPL_BadOrder, "Too small number" );

    for( i = 0; i < number; i++ )
    {
	if( !src[i] || !dst[i] )
	    CV_ERROR( IPL_HeaderIsNull, "Pointer to histogram is null" );
	if( src[i]->type != src[0]->type || dst[i]->type != src[0]->type )
	    CV_ERROR( IPL_BadOrder, "Types are not equals" );
	if( src[i]->c_dims != src[0]->c_dims || dst[i]->c_dims != src[0]->c_dims )
	    CV_ERROR( IPL_BadOrder, "Dims are not equals" );
	for( j = 0; j < src[0]->c_dims; j++ )
	    if( src[i]->dims[j] != src[0]->dims[j] || dst[i]->dims[j] != src[0]->dims[j] )
		CV_ERROR( IPL_BadOrder, "Dims are not equals" );
    }

    cvClearHist( dst[0] );
    size = src[0]->dims[0] * src[0]->mdims[0];

    if( src[0]->type == CV_HIST_ARRAY )
    {
	/* using dst[0] histogram for temporary summ array */
	for( i = 0; i < number; i++ )
	    icvAddVector_32f( src[i]->array, dst[0]->array, dst[0]->array, size );
	for( i = 0; i < size; i++ )
	{
	    float val = dst[0]->array[i];

	    dst[0]->array[i] = val ? 1.0f / val : 0;
	}
	for( i = number - 1; i >= 0; i-- )
	    icvMulVectors_32f( src[i]->array, dst[0]->array, dst[i]->array, size );
    }
    else			/* tree histogram */
    {
	float _array[4096];
	float *array = _array;

	if( number > 4096 )
	    array = (float *) icvAlloc( number * sizeof( *array ));

	for( i = 1; i < number; i++ )
	    cvClearHist( dst[i] );
	for( i = 0; i < size; i++ )
	{
	    float sum = 0;

	    for( j = 0; j < number; j++ )
		sum += (array[j] = cvQueryHistValue_1D( src[j], i ));

	    if( sum )
	    {
		sum = 1.0f / sum;
		for( j = 0; j < number; j++ )
		    if( array[j] )
			icvInsertNode( dst[j], i )->value = array[j] * sum;
	    }
	}
	if( number > 4096 )
	    icvFree( &array );
    }
    
    __END__;
}


static void
icvCalcHistMask( IplImage ** img, IplImage * mask, CvHistogram * hist, int dont_clear )
{
    CV_FUNCNAME( "_cvCalcHistMask" );
    uchar *data[CV_HIST_MAX_DIM];
    uchar *mask_data;
    int step = 0;
    int mask_step = 0;
    CvSize roi = { 0, 0 };


    __BEGIN__;
    for( int i = 0; i < hist->c_dims; i++ )
	CV_CALL( CV_CHECK_IMAGE( img[i] ));
    CV_CALL( CV_CHECK_IMAGE( mask ));

    {
	for( int i = 0; i < hist->c_dims; i++ )
	    cvGetImageRawData( img[i], &data[i], &step, &roi );
    }
    cvGetImageRawData( mask, &mask_data, &mask_step, 0 );

    switch (img[0]->depth)
    {
    case IPL_DEPTH_8U:
	IPPI_CALL( icvCalcHistMask8uC1R( data, step, mask_data, mask_step,
					 roi, hist, dont_clear ));
	break;
    case IPL_DEPTH_8S:
	IPPI_CALL( icvCalcHistMask8sC1R( (char **) data, step, mask_data, mask_step,
					 roi, hist, dont_clear ));
	break;
    case IPL_DEPTH_32F:
	IPPI_CALL( icvCalcHistMask32fC1R( (float **) data, step, mask_data, mask_step,
					  roi, hist, dont_clear ));
	break;
    }

    
    __END__;
}


CV_IMPL void
cvCalcHist( IplImage ** img, CvHistogram * hist, int dont_clear, IplImage * mask )
{
    if( mask )
    {
	icvCalcHistMask( img, mask, hist, dont_clear );
	return;
    }

    CV_FUNCNAME( "cvCalcHist" );
    uchar *data[CV_HIST_MAX_DIM];
    int step = 0;
    CvSize roi = { 0, 0 };

    __BEGIN__;

    {
	for( int i = 0; i < hist->c_dims; i++ )
	    CV_CALL( CV_CHECK_IMAGE( img[i] ));
    }

    {
	for( int i = 0; i < hist->c_dims; i++ )
	    cvGetImageRawData( img[i], &data[i], &step, &roi );
    }

    switch (img[0]->depth)
    {
    case IPL_DEPTH_8U:
	IPPI_CALL( icvCalcHist8uC1R( data, step, roi, hist, dont_clear ));
	break;
    case IPL_DEPTH_8S:
	IPPI_CALL( icvCalcHist8sC1R( (char **) data, step, roi, hist, dont_clear ));
	break;
    case IPL_DEPTH_32F:
	IPPI_CALL( icvCalcHist32fC1R( (float **) data, step, roi, hist, dont_clear ));
	break;
    }

    
    __END__;
}


CV_IMPL void
cvCalcBackProject( IplImage ** img, IplImage * dst, CvHistogram * hist )
{
    CV_FUNCNAME( "cvCalcBackProject" );
    uchar *data[CV_HIST_MAX_DIM];
    uchar *dst_data;
    int step = 0;
    int dst_step = 0;
    CvSize roi = { 0, 0 };


    __BEGIN__;
    for( int i = 0; i < hist->c_dims; i++ )
	CV_CALL( CV_CHECK_IMAGE( img[i] ));
    CV_CALL( CV_CHECK_IMAGE( dst ));

    {
	for( int i = 0; i < hist->c_dims; i++ )
	    cvGetImageRawData( img[i], &data[i], &step, &roi );
    }
    cvGetImageRawData( dst, &dst_data, &dst_step, 0 );

    switch (img[0]->depth)
    {
    case IPL_DEPTH_8U:
	IPPI_CALL( icvCalcBackProject8uC1R( data, step, dst_data, dst_step, roi, hist ));
	break;
    case IPL_DEPTH_8S:
	IPPI_CALL( icvCalcBackProject8sC1R( (char **) data, step, (char *) dst_data, dst_step,
					    roi, hist ));
	break;
    case IPL_DEPTH_32F:
	IPPI_CALL( icvCalcBackProject32fC1R
		   ( (float **) data, step, (float *) dst_data, dst_step, roi, hist ));
	break;
    }

    
    __END__;
}


CV_IMPL void
cvCalcBackProjectPatch( IplImage ** img, IplImage * dst, CvSize range,
			CvHistogram * hist, CvCompareMethod method,
			double norm_factor )
{
    CV_FUNCNAME( "cvCalcBackProjectPatch" );
    uchar *data[CV_HIST_MAX_DIM];
    float *dst_data;
    int step = 0;
    int dst_step = 0;
    CvSize roi = { 0, 0 };


    __BEGIN__;
    for( int i = 0; i < hist->c_dims; i++ )
	CV_CALL( CV_CHECK_IMAGE( img[i] ));
    CV_CALL( CV_CHECK_IMAGE( dst ));

    {
	for( int i = 0; i < hist->c_dims; i++ )
	    cvGetImageRawData( img[i], &data[i], &step, &roi );
    }
    cvGetImageRawData( dst, (uchar **) & dst_data, &dst_step, 0 );

    switch (img[0]->depth)
    {
    case IPL_DEPTH_8U:
	IPPI_CALL( icvCalcBackProjectPatch8uC1R( data, step, dst_data, dst_step,
						 roi, range, hist, method, (float)norm_factor ));
	break;
    case IPL_DEPTH_8S:
	IPPI_CALL( icvCalcBackProjectPatch8sC1R( (char **) data, step, dst_data, dst_step,
						 roi, range, hist, method, (float)norm_factor ));
	break;
    case IPL_DEPTH_32F:
	IPPI_CALL( icvCalcBackProjectPatch32fC1R( (float **) data, step, dst_data, dst_step,
						  roi, range, hist, method, (float)norm_factor ));
	break;
    }

    
    __END__;
}


void
cvClearHist( CvHistogram * hist )
{

    CV_FUNCNAME( "cvClearHist" );

    __BEGIN__;

    IPPI_CALL( icvClearHist( hist ));

    
    __END__;
}
