/* Original code has been submitted by Liu Liu.
   ----------------------------------------------------------------------------------
   * Spill-Tree for Approximate KNN Search
   * Author: Liu Liu
   * mailto: liuliu.1987+opencv@gmail.com
   * Refer to Paper:
   * An Investigation of Practical Approximate Nearest Neighbor Algorithms
   * cvMergeSpillTree TBD
   *
   * Redistribution and use in source and binary forms, with or
   * without modification, are permitted provided that the following
   * conditions are met:
   * 	Redistributions of source code must retain the above
   * 	copyright notice, this list of conditions and the following
   * 	disclaimer.
   * 	Redistributions in binary form must reproduce the above
   * 	copyright notice, this list of conditions and the following
   * 	disclaimer in the documentation and/or other materials
   * 	provided with the distribution.
   * 	The name of Contributor may not be used to endorse or
   * 	promote products derived from this software without
   * 	specific prior written permission.
   *
   * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
   * CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
   * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
   * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
   * DISCLAIMED. IN NO EVENT SHALL THE CONTRIBUTORS BE LIABLE FOR ANY
   * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
   * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
   * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
   * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
   * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
   * TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
   * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
   * OF SUCH DAMAGE.
   */

#include "cv.h"
#include "_cvfeaturetree.h"

struct CvSpillTreeNode
{
  bool leaf; // is leaf or not (leaf is the point that have no more child)
  bool spill; // is not a non-overlapping point (defeatist search)
  CvSpillTreeNode* lc; // left child (<)
  CvSpillTreeNode* rc; // right child (>)
  int cc; // child count
  CvMat* u; // projection vector
  CvMat* center; // center
  int i; // original index
  double r; // radius of remaining feature point
  double ub; // upper bound
  double lb; // lower bound
  double mp; // mean point
  double p; // projection value
};

struct CvSpillTree
{
  CvSpillTreeNode* root;
  int naive; // under this value, we perform naive search
  double rho; // under this value, it is a spill tree
  double tau; // the overlapping buffer ratio
};

// find the farthest node in the "list" from "node"
static inline CvSpillTreeNode*
icvFarthestNode( CvSpillTreeNode* node,
		 CvSpillTreeNode* list,
		 int total )
{
  double farthest = -1.;
  CvSpillTreeNode* result = NULL;
  for ( int i = 0; i < total; i++ )
    {
      double norm = cvNorm( node->center, list->center );
      if ( norm > farthest )
	{
	  farthest = norm;
	  result = list;
	}
      list = list->rc;
    }
  return result;
}

// clone a new tree node
static inline CvSpillTreeNode*
icvCloneSpillTreeNode( CvSpillTreeNode* node )
{
  CvSpillTreeNode* result = (CvSpillTreeNode*)cvAlloc( sizeof(CvSpillTreeNode) );
  memcpy( result, node, sizeof(CvSpillTreeNode) );
  return result;
}

// append the link-list of a tree node
static inline void
icvAppendSpillTreeNode( CvSpillTreeNode* node,
			CvSpillTreeNode* append )
{
  if ( node->lc == NULL )
    {
      node->lc = node->rc = append;
      node->lc->lc = node->rc->rc = NULL;
    } else {
      append->lc = node->rc;
      append->rc = NULL;
      node->rc->rc = append;
      node->rc = append;
    }
  node->cc++;
}

static void
icvDFSInitSpillTreeNode( const CvSpillTree* tr,
			 const int d,
			 CvSpillTreeNode* node )
{
  if ( node->cc <= tr->naive )
    {
      // already get to a leaf, terminate the recursion.
      node->leaf = true;
      node->spill = false;
      return;
    }

  // random select a node, then find a farthest node from this one, then find a farthest from that one...
  // to approximate the farthest node-pair
  static CvRNG rng_state = cvRNG(0xdeadbeef);
  int rn = cvRandInt( &rng_state ) % node->cc;
  CvSpillTreeNode* lnode = NULL;
  CvSpillTreeNode* rnode = node->lc;
  for ( int i = 0; i < rn; i++ )
    rnode = rnode->rc;
  lnode = icvFarthestNode( rnode, node->lc, node->cc );
  rnode = icvFarthestNode( lnode, node->lc, node->cc );

  // u is the projection vector
  node->u = cvCreateMat( 1, d, CV_64FC1 );
  cvSub( lnode->center, rnode->center, node->u );
  cvNormalize( node->u, node->u );
	
  // find the center of node in hyperspace
  node->center = cvCreateMat( 1, d, CV_64FC1 );
  cvZero( node->center );
  CvSpillTreeNode* it = node->lc;
  for ( int i = 0; i < node->cc; i++ )
    {
      cvAdd( it->center, node->center, node->center );
      it = it->rc;
    }
  cvConvertScale( node->center, node->center, 1./node->cc );

  // project every node to "u", and find the mean point "mp"
  it = node->lc;
  node->r = -1.;
  node->mp = 0;
  for ( int i = 0; i < node->cc; i++ )
    {
      node->mp += ( it->p = cvDotProduct( it->center, node->u ) );
      double norm = cvNorm( node->center, it->center );
      if ( norm > node->r )
	node->r = norm;
      it = it->rc;
    }
  node->mp = node->mp / node->cc;

  // overlapping buffer and upper bound, lower bound
  double ob = (lnode->p-rnode->p)*tr->tau*.5;
  node->ub = node->mp+ob;
  node->lb = node->mp-ob;
  int l = 0;
  int r = 0;
  it = node->lc;
  for ( int i = 0; i < node->cc; i++ )
    {
      if ( it->p <= node->ub )
	l++;
      if ( it->p >= node->lb )
	r++;
      it = it->rc;
    }
  CvSpillTreeNode* lc = (CvSpillTreeNode*)cvAlloc( sizeof(CvSpillTreeNode) );
  CvSpillTreeNode* rc = (CvSpillTreeNode*)cvAlloc( sizeof(CvSpillTreeNode) );
  lc->lc = lc->rc = rc->lc = rc->rc = NULL;
  lc->cc = rc->cc = 0;
  int undo = cvRound(node->cc*tr->rho);
  if (( l > undo )||( r > undo ))
    {
      // it is not a spill point (defeatist search disabled)
      it = node->lc;
      for ( int i = 0; i < node->cc; i++ )
	{
	  CvSpillTreeNode* next = it->rc;
	  if ( it->p < node->mp )
	    icvAppendSpillTreeNode( lc, it );
	  else
	    icvAppendSpillTreeNode( rc, it );
	  it = next;
	}
      node->spill = false;
    } else {
      // a spill point
      it = node->lc;
      for ( int i = 0; i < node->cc; i++ )
	{
	  CvSpillTreeNode* next = it->rc;
	  if ( it->p < node->lb )
	    icvAppendSpillTreeNode( lc, it );
	  else if ( it->p > node->ub )
	    icvAppendSpillTreeNode( rc, it );
	  else {
	    CvSpillTreeNode* cit = icvCloneSpillTreeNode( it );
	    icvAppendSpillTreeNode( lc, it );
	    icvAppendSpillTreeNode( rc, cit );
	  }
	  it = next;
	}
      node->spill = true;
    }
  node->lc = lc;
  node->rc = rc;

  // recursion process
  icvDFSInitSpillTreeNode( tr, d, node->lc );
  icvDFSInitSpillTreeNode( tr, d, node->rc );
}

static CvSpillTree*
icvCreateSpillTree( const CvMat* raw_data,
		    const int naive,
		    const double rho,
		    const double tau )
{
  CvSpillTree* tr = (CvSpillTree*)cvAlloc( sizeof(CvSpillTree) );
  tr->root = (CvSpillTreeNode*)cvAlloc( sizeof(CvSpillTreeNode) );
  tr->naive = naive;
  tr->rho = rho;
  tr->tau = tau;

  int n = raw_data->rows;
  int d = raw_data->cols;

  // tie a link-list to the root node
  tr->root->lc = (CvSpillTreeNode*)cvAlloc( sizeof(CvSpillTreeNode) );
  tr->root->lc->center = cvCreateMatHeader( 1, raw_data->cols, CV_64FC1 );
  cvSetData( tr->root->lc->center, raw_data->data.db, raw_data->step );
  tr->root->lc->lc = NULL;
  tr->root->lc->leaf = true;
  tr->root->lc->i = 0;
  CvSpillTreeNode* node = tr->root->lc;
  for ( int i = 1; i < n; i++ )
    {
      CvSpillTreeNode* newnode = (CvSpillTreeNode*)cvAlloc( sizeof(CvSpillTreeNode) );
      switch ( CV_MAT_DEPTH( raw_data->type ) )
	{
	case CV_32F:
	  newnode->center = cvCreateMatHeader( 1, d, CV_32FC1 );
	  cvSetData( newnode->center, raw_data->data.fl+i*d, raw_data->step );
	  break;
	case CV_64F:
	  newnode->center = cvCreateMatHeader( 1, d, CV_64FC1 );
	  cvSetData( newnode->center, raw_data->data.db+i*d, raw_data->step );
	  break;
	default:
	  assert(0);
	}
      newnode->lc = node;
      newnode->i = i;
      newnode->leaf = true;
      newnode->rc = NULL;
      node->rc = newnode;
      node = newnode;
    }
  tr->root->rc = node;
  tr->root->cc = n;
  icvDFSInitSpillTreeNode( tr, d, tr->root );
  return tr;
}

static void
icvSpillTreeNodeHeapify( CvSpillTreeNode** heap,
			 int i,
			 const int k )
{
  if ( heap[i] == NULL )
    return;
  int l = 0;
  int r = 0;
  int largest = i;
  CvSpillTreeNode* inp;
  do {
    i = largest;
    l = ((i+1)<<1)-1;
    r = (i+1)<<1;
    if (( l < k )&&(( heap[l] == NULL )||( heap[l]->mp > heap[i]->mp )))
      largest = l;
    else if (( r < k )&&(( heap[r] == NULL )||( heap[r]->mp > heap[i]->mp )))
      largest = r;
    if ( largest != i )
      CV_SWAP( heap[largest], heap[i], inp );
  } while ( largest != i );
}

static void
icvSpillTreeDFSearch( CvSpillTreeNode* node,
		      CvSpillTreeNode** heap,
		      const CvMat* desc,
		      const int k )
{
  double dist, p;
  while ( node->spill )
    {
      // defeatist search
      p = cvDotProduct( node->u, desc );
      if ( p < node->lb )
	node = node->lc;
      else if ( p > node->ub )
	node = node->rc;
      else
	break;
      if ( NULL == node )
	return;
      if ( !node->leaf )
	p = cvDotProduct( node->u, desc );
    }
  if ( node->leaf )
    {
      // a leaf, naive search
      CvSpillTreeNode* it = node->lc;
      for ( int i = 0; i < node->cc; i++ )
	{
	  it->mp = cvNorm( it->center, desc );
	  if (( heap[0] == NULL)||( it->mp < heap[0]->mp ))
	    {
	      heap[0] = it;
	      icvSpillTreeNodeHeapify( heap, 0, k );
	    }
	  it = it->rc;
	}
      return;
    }
  dist = cvNorm( node->center, desc );
  // impossible case, skip
  if (( heap[0] != NULL )&&( dist-node->r > heap[0]->mp ))
    return;
  p = cvDotProduct( node->u, desc );
  // guided dfs
  if ( p < node->mp )
    {
      icvSpillTreeDFSearch( node->lc, heap, desc, k );
      icvSpillTreeDFSearch( node->rc, heap, desc, k );
    } else {
      icvSpillTreeDFSearch( node->rc, heap, desc, k );
      icvSpillTreeDFSearch( node->lc, heap, desc, k );
    }
}

static void
icvFindSpillTreeFeatures( CvSpillTree* tr,
			  const CvMat* desc,
			  CvMat* results,
			  CvMat* dist,
			  const int k )
{
  CvSpillTreeNode** heap = (CvSpillTreeNode**)cvAlloc( k*sizeof(heap[0]) );
  for ( int j = 0; j < desc->rows; j++ )
    {
      CvMat _desc;
      switch ( CV_MAT_DEPTH( desc->type ) )
	{
	case CV_32F:
	  _desc = cvMat( 1, desc->cols, CV_32F, desc->data.fl+j*desc->cols );
	  break;
	case CV_64F:
	  _desc = cvMat( 1, desc->cols, CV_64F, desc->data.db+j*desc->cols );
	  break;
	default:
	  assert(0);
	}
      for ( int i = 0; i < k; i++ )
	heap[i] = NULL;
      icvSpillTreeDFSearch( tr->root, heap, &_desc, k );
      CvSpillTreeNode* inp;
      for ( int i = k-1; i > 0; i-- )
	{
	  CV_SWAP( heap[i], heap[0], inp );
	  icvSpillTreeNodeHeapify( heap, 0, i );
	}
      int* rs = results->data.i+j*results->cols;
      double* dt = dist->data.db+j*dist->cols;
      for ( int i = 0; i < k; i++, rs++, dt++ )
	if ( heap[i] != NULL )
	  {
	    *rs = heap[i]->i;
	    *dt = heap[i]->mp;
	  } else
	    *rs = -1;
    }
  cvFree( &heap );
}

static void
icvDFSReleaseSpillTreeNode( CvSpillTreeNode* node )
{
  if ( node->leaf )
    {
      CvSpillTreeNode* it = node->lc;
      for ( int i = 0; i < node->cc; i++ )
	{
	  cvReleaseMat( &it->u );
	  cvReleaseMat( &it->center );
	  CvSpillTreeNode* s = it;
	  it = it->rc;
	  cvFree( &s );
	}
    } else {
      cvReleaseMat( &node->u );
      cvReleaseMat( &node->center );
      icvDFSReleaseSpillTreeNode( node->lc );
      icvDFSReleaseSpillTreeNode( node->rc );
      cvFree( &node );
    }
}

static void
icvReleaseSpillTree( CvSpillTree** tr )
{
  icvDFSReleaseSpillTreeNode( (*tr)->root );
  cvFree( tr );
}

class CvSpillTreeWrap : public CvFeatureTree {
  CvSpillTree* tr;
public:
  CvSpillTreeWrap(const CvMat* raw_data,
		  const int naive,
		  const double rho,
		  const double tau) {
    tr = icvCreateSpillTree(raw_data, naive, rho, tau);
  }
  ~CvSpillTreeWrap() {
    icvReleaseSpillTree(&tr);
  }

  void FindFeatures(CvMat* desc, int k, int emax, CvMat* results, CvMat* dist) {
    icvFindSpillTreeFeatures(tr, desc, results, dist, k);
  }
};

CvFeatureTree* cvCreateSpillTree( const CvMat* raw_data,
				  const int naive,
				  const double rho,
				  const double tau ) {
  return new CvSpillTreeWrap(raw_data, naive, rho, tau);
}
