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

#endif /* __cplusplus */

#endif /* _CV_HPP */


/* End of file. */
