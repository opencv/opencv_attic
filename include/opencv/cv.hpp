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

//#ifdef WIN32

#ifndef _CV_HPP_
#define _CV_HPP_

#include "cv.h"

#ifdef __cplusplus

#if _MSC_VER >= 1200

#pragma warning(disable : 4710) /* function not inlined */
#pragma warning(disable : 4711)
#pragma warning(disable : 4514)

#endif

/* high-level C interface */
#include <assert.h>

#if defined _MSC_VER || defined __ICL || defined __BORLANDC__

#define CVH_DECLARE_STORAGE
#include "cvstorage.hpp"


/****************************************************************************************\
*                          Template classes for dynamic data structures                  *
\****************************************************************************************/

template<class Val> class CvArrayIterator
{
public:
    typedef  Val  value_type;
    typedef  int  idx_type;
    typedef  CvArrayIterator<value_type>  iterator;

    value_type& operator *() const {return *current;}
    idx_type    get_idx() const {return current - begin;}
    
    iterator& operator ++() { current++; return *this; }
    const iterator operator ++(int) { iterator temp = *this; current++; return temp; }

    operator value_type*() {return *current;}

    CvArrayIterator() {current = begin = 0;}
    CvArrayIterator( value_type* _begin ) {begin = current = _begin;}
    
    CvArrayIterator( value_type* _begin, value_type* _current )
        {begin = _begin; current = _current;}

    CvArrayIterator( const iterator& another ) {*this = another;}

    void init( value_type* _begin, value_type* _current )
        {begin = _begin; current = _current;}

    bool operator == ( const iterator& another ) const
        {return (current == another.current) && (begin == another.begin);}
    bool operator != ( const iterator& another ) const
        {return (current != another.current) || (begin != another.begin);}

    value_type* get_begin(){ return begin; }
    value_type* get_current(){ return current; }

protected:
    value_type*  current;
    value_type*  begin;
};


template<class Node> class CvTreeIterator
{
public:
    typedef  Node  node_type;
    typedef  typename node_type::value_type value_type;
    typedef  typename node_type::idx_type   idx_type;
    typedef  CvTreeIterator<node_type>  iterator;

    value_type& operator *() { assert( node != 0 ); return node->val; }
    idx_type    get_idx()    { assert( node != 0 ); return node->idx; }
    node_type*  get_node()   { assert( node != 0 ); return node; }
    
    iterator   operator ++();
    iterator   operator ++(int);
    iterator&  operator =( const iterator& _iterator )
        { node = _iterator.node; return *this; }

    CvTreeIterator() { node = 0; }
    CvTreeIterator( const iterator& another ) { node = another.node; }
    CvTreeIterator( node_type* root_node ) { node = root_node; }

    void init( node_type* _node ) { node = _node; }

    bool operator==( const iterator& another ) { return node == another.node; }
    bool operator!=( const iterator& another ) { return node != another.node; }

protected:
    node_type*  node;
    node_type*  next();
};

template<class Node> class _CvNodeManager
{
public:
    typedef  Node  node_type;
    typedef  node_type::value_type value_type;
    typedef  node_type::idx_type idx_type;
    typedef  _CvNodeBlock<node_type>  block_type;
    typedef  CvNodeIterator<node_type> iterator;
    typedef  _CvNodeManager<node_type> manager_type;

    _CvNodeManager();
    ~_CvNodeManager() { destroy(); }

    void  destroy();
    void  clear();
    node_type* new_node();
    void  release_node( node_type* _node );

    iterator     begin() const { return iterator( first_block ); }
    iterator     end() const { return iterator( last_block, block_type::block_nodes - 1 ); }

protected:
    node_type*   first_free;
    block_type*  first_block;
    block_type*  last_block;
    node_type*   allocate_new_block();
};


/************************* tree class *******************************/ 
template<class Val, class Idx = int> class CvTree
{
public:

    typedef  Val  value_type;
    typedef  Idx  idx_type;
    typedef  _CvTreeNode<value_type, idx_type> node_type;
    typedef  _CvNodeManager<node_type>  node_manager;
    typedef  CvNodeIterator<node_type>  raw_iterator;
    typedef  CvTreeIterator<node_type>  iterator;
    typedef  CvTree<value_type,idx_type> storage;

    value_type query( idx_type idx ) const;

    value_type& operator []( idx_type idx )
        { assert( size == 0 || idx < size ); return create_node( idx )->val; }

    void  remove( idx_type idx );
    void  clear() { root = 0; size = 0; manager.clear(); }
    void  destroy() { root = 0; size = 0; manager.destroy(); }

    idx_type  get_size() const { return size; }
    
    iterator  begin() const;
    iterator  end() const;

    raw_iterator raw_begin() const { return manager.begin(); }
    raw_iterator raw_end() const { return manager.end(); }

    void  set_size( idx_type _size ) { size = _size; }

    CvTree() {root = 0; size = 0;}
    CvTree( idx_type _size ) { root = 0; size = _size; }
    CvTree( const storage& another );

    storage& operator = ( const storage& another );

    template<class Op> double operate_with( const storage& another, Op operation ) const
    {
        iterator iter1 = begin();
        iterator iter2 = another.begin();
        iterator  end1 = end();
        iterator  end2 = another.end();

        Op::result_type s = 0.0;
        value_type val = 0;

        do{
            if( iter1.get_idx() > iter2.get_idx() )
                s = operation( s, val, *iter2++ );
            else if( iter1.get_idx() < iter2.get_idx() )
                s = operation( s, *iter1++, (value_type)0 );
            else if( iter1.get_idx() == iter2.get_idx() )
                s = operation( s, *iter1++, *iter2++ );
        }while( iter1 != end1 && iter2 != end2 );
        if( iter1.get_idx() == iter2.get_idx() ) s = operation( s, *iter1, *iter2 );
        return s;
    }

    node_type* get_root() const { return root; }
    
protected:

    node_type*    root;
    node_manager  manager;
    idx_type      size;

    node_type* create_node( idx_type idx );
};


/************************* array class *******************************/ 
template<class Val> class CvArray
{
public:

    typedef  Val   value_type;
    typedef  int   idx_type;
    typedef  CvArrayIterator<value_type>  iterator;
    typedef  iterator  raw_iterator;
    typedef  CvArray<value_type>  storage;
    
    value_type   query( idx_type idx ) const
        { assert( size == 0 || idx < size );
          return array[idx]; }
    
    value_type&  operator []( idx_type idx )
        { assert( size == 0 || idx < size ); return array[idx]; }

    void  remove( idx_type idx );
    void  clear();
    void  destroy();

    idx_type    get_size() const { return size; };
    idx_type    get_capacity(){ return capacity; };
    value_type* get_array() const { return array; }
    
    iterator  begin() const;
    iterator  end() const;

    raw_iterator raw_begin() const;
    raw_iterator raw_end() const;

    void  set_size( idx_type _size );
    void  set_capacity( idx_type _capacity );

    CvArray() { array = 0; size = capacity = 0; }
    CvArray( idx_type _size );
    CvArray( idx_type _size, idx_type _capacity );
    CvArray( const storage& another );

    ~CvArray() { if( array != 0 ) delete array; }

    storage& operator = ( const storage& another );

    template<class Op> double operate_with( const storage& another, Op operation ) const
    {
        Op::result_type s = 0.0;
        for( idx_type i = 0; i < size; i++ )
            s = operation( s, array[i], another.array[i] );
        return s;
    }

protected:

    value_type* array;
    idx_type    size;
    idx_type    capacity;
};

/****************************************************************************************\
*                             Multi-dimensional histogram                                *
\****************************************************************************************/

template <class Storage = CvArray<class Val> > class CVHistogram
{
public:

    typedef  Storage  storage_type;
    typedef  storage_type::value_type value_type;  // type of histogram bins
    typedef  storage_type::idx_type   idx_type;    // type of bin indices
    typedef  typename storage_type::iterator iterator;
    typedef  typename storage_type::raw_iterator  raw_iterator;
    typedef  CVHistogram<storage_type> histogram; 

    void     create( int bins0, int bins1 = 0, int bins2 = 0,
                     int bins3 = 0, int bins4 = 0, int bins5 = 0 );
    void     clear() { storage.clear(); } // clear histogram
    void     destroy() { storage.destroy(); dims = 0; } // clear and free memory

    //  lookup operations  
    value_type  query( int i0 ) const { return storage.query( i0 ); }
    value_type  query( int i0, int i1 ) const
        { return storage.query( i0 + i1 * mbins[1] ); }
    value_type  query( int i0, int i1, int i2 ) const
        { return storage.query( i0 + i1 * mbins[1] + i2 * mbins[2] ); }
    value_type  query( int i0, int i1, int i2, int i3, int i4 = 0, int i5 = 0 ) const
        { return storage.query( get_idx( i0, i1, i2, i3, i4, i5 ) ); }
    value_type  query( const int* idxs ) const;

    // retrieving references to bins
    value_type&  operator []( int i0 ) { return storage[i0]; }
    value_type&  operator ()( int i0 ) { return storage[i0]; }
    value_type&  operator ()( int i0, int i1 ) { return storage[i0 + i1 * mbins[1]]; }
    value_type&  operator ()( int i0, int i1, int i2 )
        { return storage[i0 + i1 * mbins[1] + i2 * mbins[2]]; }
    value_type&  operator ()( int i0, int i1, int i2, int i3, int i4 = 0, int i5 = 0 )
        { return storage[get_idx( i0, i1, i2, i3, i4, i5 )]; }
    value_type&  operator ()( const int* idxs );

    // hi-level iterators
    iterator  begin() const { return storage.begin(); }
    iterator  end() const { return storage.end(); }
    
    // low-level iterators
    raw_iterator raw_begin() const { return storage.raw_begin(); }
    raw_iterator raw_end() const { return storage.raw_end(); }
    
    // normalizing
    void  normalize( value_type norm_factor = 1 );
    void  normalize( histogram& result, value_type norm_factor = 1 ) const;
    // thresholding
    void  threshold( value_type threshold );
    void  threshold( value_type threshold, histogram& result );

    histogram& operator -= (value_type val);
    histogram& operator *= (value_type val);
    double mean() const;
    
    // some other operations: -=val, *=val, blur, mean ...
    
    // constructors/destructor 
    CVHistogram() { dims = 0; }
    CVHistogram( int bins0, int bins1 = 0, int bins2 = 0, 
                  int bins3 = 0, int bins4 = 0, int bins5 = 0 )
    { create( bins0, bins1, bins2, bins3, bins4, bins5 ); }
    CVHistogram( const histogram& another );
    ~CVHistogram() { destroy(); }

    // copy operation
    histogram& operator = (const histogram& another);
    
    int   get_dims() const { return dims; }
    int   get_dim_size(int n = 0) const { return bins[n]; };

   // helper template method for histograms comparing
    template<class Op> double operate_with( const histogram& another, Op operation ) const
    {
        return storage.operate_with( another.storage, operation );
    }

    bool  check_size_equality( const histogram& another )
    {
        if( dims != another.dims ) return false;
        for( int i = 0; i < dims; i++ ) if( bins[i] != another.bins[i] ) return false;
        return true;
    }

    enum { max_dims = 6 };

protected:

    int           dims;
    int           bins[max_dims];
    int           mbins[max_dims];
    storage_type  storage;

    idx_type get_idx( int bin0, int bin1 = 0, int bin2 = 0,
                      int bin3 = 0, int bin4 = 0, int bin5 = 0 ) const
        { return bin0 * mbins[0] + bin1 * mbins[1] + bin2 * mbins[2] + bin3 * mbins[3]
               + bin4 * mbins[4] + bin5 * mbins[5]; }
};


template <class Hist> inline double calc_histogram_intersection( const Hist& hist1,
                                                                 const Hist& hist2 );

template <class Hist> inline double calc_histogram_chi_square( const Hist& hist1,
                                                               const Hist& hist2 );

template <class Hist> inline double calc_histogram_correlation( const Hist& hist1,
                                                                const Hist& hist2 );

template<class Histogram, class SrcType, class ThreshType> void
    calc_histogram_from_images( Histogram& hist, SrcType** src,
                                CvSize roi, int  step, ThreshType** thresh );

template<class Histogram, class SrcType, class ThreshType, class DstType> void 
    calc_back_project_from_images( Histogram&   Hmodel,
                                   SrcType*     src,
                                   CvSize      roi,
                                   int          src_step,
                                   ThreshType** thresh,
                                   DstType*     measure,
                                   int          dst_step,
                                   DstType      threshold = 0 );

#define CvBackProject calc_back_project_from_images
#define CvCalculateC1 calc_histogram_from_images


#define CVH_IMPLEMENT_STORAGE
#include "cvstorage.hpp"


#endif /* #if defined _MSC_VER || defined __ICL || defined __BORLANDC__ */

/****************************************************************************************\
*                                      Image class                                       *
\****************************************************************************************/

struct CV_DLL_ENTRY CvImage : public IplImage
{
    CvImage();
    CvImage( CvSize size, int depth, int channels );
    ~CvImage();

    uchar* image_data();
    const uchar* image_data() const;

    CvSize image_roi_size() const;
    int    byte_per_pixel() const;

    CvImage& operator = ( const CvImage& another )
    { return copy( another ); }
    
    CvImage& copy( const CvImage& another );
};


class CV_DLL_ENTRY CvImageGroup
{
public:
    enum{ max_count = 7 };
    
    CvImageGroup( int _count = 0 )
    { assert( _count < max_count ); clear(); operator=(_count); }


    ~CvImageGroup() { destroy(); }
    
    CvImage& operator[]( int count ) { return (CvImage&)*image[count]; }

    CvImageGroup& operator=( const CvImageGroup& another )
    { return copy( another ); }

    CvImageGroup& operator=( const CvImage& another )
    { return copy( another ); }

    CvImageGroup& copy( const CvImageGroup& another );

    CvImageGroup& copy( const CvImage& another );
    CvImageGroup& operator=( int _count );

    int get_count() const { return count; }

    void destroy();
    void clear() { for( int i = 0; i < max_count; i++ ) image[i] = 0; }
    IplImage** get_group() { return &image[0]; };

protected:
    int       count;
    IplImage* image[max_count];
};

class CV_DLL_ENTRY CvCamShiftTracker
{
public:
    
    // constructor
    CvCamShiftTracker();
    // destructor
    virtual ~CvCamShiftTracker();
    
    // get- properties
    
    // Characteristics of the object, 
    // which are calculated by track_object method
    float   get_orientation()  // orientation of the object in degrees 
    { return orientation; }
    float   get_length()       // the larger linear size of the object
    { return length; }
    float   get_width()        // the smaller linear size of the object
    { return width; }
    CvRect get_window()       // bounding rectangle for the object
    { return window; }
    
    // Tracking parameters
    int     get_threshold()  // thresholding value that applied to back project
    { return threshold; }
    int     get_hist_dims( int* dims = 0 ); // returns number of histogram dimensions and sets
    // dims[0] to number of bins on 1st dimension,
    // dims[1] -||- on 2nd dimension etc. 
    // (if dims pointer is not 0).
    
    int     get_min_ch_val( int channel )  // Given channel index, returns 
    { return min_ch_val[channel]; }        // the minimum value of that channel, 
                                           // starting from which the pixel
                                           // is counted during histogram calculation.
    
    int     get_max_ch_val( int channel )  // Get maximum channel value.
    { return max_ch_val[channel]; }
    
    
    // Background differencing parameters
    
    // set- properties
    // Object characteristics
    bool    set_window( CvRect _window) // set initial bounding rectangle for the object
    { window = _window; return true; }
    
    // Tracking parameters
    bool    set_threshold( int _threshold ) // threshold level that applied 
    { threshold = _threshold; return true; }

    // to back project.
    bool    set_hist_dims( int c_dims, int* dims );// histogram dimensions.
    
    bool    set_min_ch_val( int channel, int val ) // Given channel index, sets 
    { min_ch_val[channel] = val; return true; }    // the minimum value of that channel, 
                                                   // starting from which the pixel
                                                   // is counted during histogram calculation.
    
    bool    set_max_ch_val( int channel, int val ) // Set maximum value for the channel.
    { max_ch_val[channel] = val; return true; }

    bool    set_thresh( int channel, int min, int max );
    
    bool    set_hist_mapping( int* /*channels*/ )
    { return 0; }
    // selects the channels from
    // resulting image 
    // (before histogram/back prj calculation)
    // for using them in histogram/back prj
    // calculation. 
    // That is, channel #(channels[0]) is 
    // corresponds to first histogram 
    // dimension, channel #(channels[1])
    // to 2nd etc.
    
    // Backgournd differencing ...
    
    // can be used (and overrided) if histogram is built from several frames
    virtual void  reset_histogram() { cvClearHist( hist ); }
    
    // main pipeline for object tracking 
    virtual void  track_object( CvImage* src_image )
    {
        trackobj_find( calc_back_project( trackobj_post_color(
            trackobj_color_transform( trackobj_pre_color( src_image )))));
    }
    
    // main pipeline for histogram calculation
    virtual void update_histogram( CvImage* src_image )
    {
        calc_histogram( hist_post_color(
            hist_color_transform( hist_pre_color( src_image ))));
    }


    virtual CvImage* get_back_project()
    { return &calc_back_project_image; }


    virtual int get_shift_parameter()
    { return shift; }


    virtual bool set_shift_parameter(int _shift)
    { shift = _shift; return true; }


    virtual int query( int bin )
    { return cvRound(cvQueryHistValue_1D( hist, bin )); }
    
protected:
    typedef CvHistogram  hist_type;

    hist_type* hist;

    float      orientation;
    float      width;
    float      length;
    CvRect     window;

    int        min_ch_val[6];
    int        max_ch_val[6];

    int        shift;

    float*     thresh[CvImageGroup::max_count];
    float      thresh_buf[CvImageGroup::max_count*2];
    int        threshold;

    CvImageGroup   color_transform_image_group;
    CvImage        trackobj_pre_color_image;
    CvImage        calc_back_project_image;
    
    // Internal pipeline functions
    
    // Common
    
    // common color transform
    virtual CvImageGroup* color_transform( CvImage* src_image );
    
    // Specific for object tracking
    // preprocessing before color transformation
    virtual CvImage* trackobj_pre_color( CvImage* src_image );
    
    // color transformation. do common transform by default
    virtual CvImageGroup* trackobj_color_transform( CvImage* src_image )
    { return color_transform( src_image ); }
    
    // postprocessing after color transformation before back project
    virtual CvImageGroup* trackobj_post_color( CvImageGroup* src_image );
    
    // calculation of back project
    virtual CvImage* calc_back_project( CvImageGroup* src_image );
    
    // apply camshift algorithm to calculate object parameters
    virtual void  trackobj_find( CvImage* src_image );
    
    
    // Specific for histogram calculation   
    
    // preprocessing before color transformation
    virtual CvImage* hist_pre_color( CvImage* src_image )
    { return trackobj_pre_color( src_image ); }

    // color transformation. do common transform by default
    virtual CvImageGroup* hist_color_transform( CvImage* src_image )
    { return color_transform( src_image ); }
    
    // postprocessing after color transformation before back project
    virtual CvImageGroup* hist_post_color( CvImageGroup* src_image )
    { return trackobj_post_color( src_image ); }
    
    // histogram calculation
    virtual void calc_histogram( CvImageGroup* src_image );
    
    
};

#include "cvstorage.hpp"

#endif /* __cplusplus */

#endif /* _CV_HPP */


/* End of file. */
