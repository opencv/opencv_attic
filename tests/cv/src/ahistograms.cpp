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

static int foaHistGetQueryValue(void* _type)
{
    CvHistType type = (CvHistType)(int)_type;
    static int c_dimss;
    static int dims;
    static int init = 0;

    CvHistogram* hist;
    int d[CV_HIST_MAX_DIM + 1];
    int i, c_dims;
    float num;
    float* ptr;
    int errors = 0;

    if( !init )
    {
        init = 1;
        trsiRead( &c_dimss, "6", "Number of dimensions" );
        trsiRead( &dims, "10", "Size of every dimension" );
    }
    
    for( c_dims = 1; c_dims <= c_dimss; c_dims++ )
    {
        /* Filling dimension's array */
        for( i = 0; i < c_dims; i++ ) d[i] = dims;
        
        /*Creating histogram*/
        hist = cvCreateHist( c_dims, d, type );
        
        /*Initializing histogram*/
        num = 0;
        for( i = 0; i <= CV_HIST_MAX_DIM; i++ ) d[i] = 0;
        for( i = 0; i < c_dims; )
        {
            ptr = cvGetHistValue_nD( hist, d );
            *ptr = num;
            for( i = 0; d[i]++, d[i] >= dims && i < CV_HIST_MAX_DIM; i++ ) d[i] = 0;
        }

        /*Checking values*/
        num = 0;
        for( i = 0; i < c_dims; i++ ) d[i] = 0;
        for( i = 0; i < c_dims; )
        {
            if( cvQueryHistValue_nD( hist, d ) != num )
            {
                trsWrite( ATS_CON | ATS_LST, "Error in Get/Set nD\n" );
                errors++;
            }
            if( c_dims == 1 && cvQueryHistValue_1D( hist, d[0] ) != num )
            {
                trsWrite( ATS_CON | ATS_LST, "Error in Query1D/Set nD\n" );
                errors++;
            }
            if( c_dims == 2 && cvQueryHistValue_2D( hist, d[0], d[1] ) != num )
            {
                trsWrite( ATS_CON | ATS_LST, "Error in Query2D/Set nD\n" );
                errors++;
            }
            if( c_dims == 3 && cvQueryHistValue_3D( hist, d[0], d[1], d[2] ) != num )
            {
                trsWrite( ATS_CON | ATS_LST, "Error in Query3D/Set nD\n" );
                errors++;
            }
            if( c_dims == 1 && *cvGetHistValue_1D( hist, d[0] ) != num )
            {
                trsWrite( ATS_CON | ATS_LST, "Error in Get1D/Set nD\n" );
                errors++;
            }
            if( c_dims == 2 && *cvGetHistValue_2D( hist, d[0], d[1] ) != num )
            {
                trsWrite( ATS_CON | ATS_LST, "Error in Get2D/Set nD\n" );
                errors++;
            }
            if( c_dims == 3 && *cvGetHistValue_3D( hist, d[0], d[1], d[2] ) != num )
            {
                trsWrite( ATS_CON | ATS_LST, "Error in Get3D/Set nD\n" );
                errors++;
            }
            for( i = 0; d[i]++, d[i] >= dims && i < CV_HIST_MAX_DIM; i++ ) d[i] = 0;
        }
        cvReleaseHist( &hist );
    }
    return errors ? trsResult( TRS_FAIL, "Fixed %d errors", errors ) : TRS_OK;
}

static int foaHistMinMaxValue(void* _type)
{
    CvHistType type = (CvHistType)(int)_type;
    static int c_dimss;
    static int dims;
    static int init = 0;

    CvHistogram* hist;
    int d[CV_HIST_MAX_DIM + 1];
    int dmax[CV_HIST_MAX_DIM + 1];
    int dmin[CV_HIST_MAX_DIM + 1];
    int dmaxc[CV_HIST_MAX_DIM + 1];
    int dminc[CV_HIST_MAX_DIM + 1];
    int i, j, c_dims;
    float min_val = 101, max_val = -101, cmin, cmax;
    float v;
    float* ptr;
    int errors = 0;

    if( !init )
    {
        init = 1;
        trsiRead( &c_dimss, "6", "Number of dimensions" );
        trsiRead( &dims, "10", "Size of every dimension" );
    }

    for( c_dims = 1; c_dims <= c_dimss; c_dims++ )
    {
        min_val = 101;
        max_val = -101;

        trsWrite(ATS_CON | ATS_LST, "Dimension %d\n", c_dims);

        /* Filling dimension's array */
        for( i = 0; i < c_dims; i++ ) d[i] = dims;
        
        /*Creating histogram*/
        hist = cvCreateHist( c_dims, d, type );
        
        /*Initializing histogram*/
        for( i = 0; i <= CV_HIST_MAX_DIM; i++ ) d[i] = 0;
        for( i = 0; i < c_dims; )
        {
            ptr = cvGetHistValue_nD( hist, d );
            do{
                v = (float)atsInitRandom( -100, 100 );
            }while( v == min_val || v == max_val );
            if( v > max_val )
            {
                max_val = v;
                for( j = 0; j < c_dims; j++ ) dmax[j] = d[j];
            }
            if( v < min_val )
            {
                min_val = v;
                for( j = 0; j < c_dims; j++ ) dmin[j] = d[j];
            }
            *ptr = v;
            for( i = 0; d[i]++, d[i] >= dims && i < CV_HIST_MAX_DIM; i++ ) d[i] = 0;
        }

        /*Checking values*/
        cvGetMinMaxHistValue( hist, &cmin, &cmax, dminc, dmaxc );
        if( cmin != min_val )
        {
            trsWrite( ATS_LST | ATS_CON, "Wrong minimum value: act %f    exp %f\n", cmin, min_val );
            errors++;
        }
        if( cmax != max_val )
        {
            trsWrite( ATS_LST | ATS_CON, "Wrong maximum value: act %f    exp %f\n", cmax, max_val );
            errors++;
        }

        if( min_val != cvQueryHistValue_nD(hist, dminc) )
        {
            trsWrite( ATS_LST | ATS_CON, "Wrong minimum pointer: act     exp \n" );
            for( i = 0; i < c_dims; i++ )
                trsWrite( ATS_LST | ATS_CON, "%d   %d\n", dminc[i], dmin[i] );
            errors++;
        }

        if( max_val != cvQueryHistValue_nD(hist, dmaxc) )
        {
            trsWrite( ATS_LST | ATS_CON, "Wrong maximum value: act     exp \n" );
            for( i = 0; i < c_dims; i++ )
                trsWrite( ATS_LST | ATS_CON, "%d   %d\n", dmaxc[i], dmax[i] );
            errors++;
        }

        cvReleaseHist( &hist );

    }

    /*assert(!errors);*/
    return errors ? trsResult( TRS_FAIL, "Fixed %d errors", errors ) : TRS_OK;
}


static int foaNormalizeHist(void* _type)
{
    CvHistType type = (CvHistType)(int)_type;
    static int c_dimss;
    static int dims;
    static int init = 0;
    static float thresh;

    CvHistogram* hist1;
    CvHistogram* hist2;
    int d[CV_HIST_MAX_DIM + 1];
    int i, j, count, c_dims;
    float sum, v;
    int errors = 0;

    if( !init )
    {
        init = 1;
        trsiRead( &c_dimss, "6", "Number of dimensions" );
        trsiRead( &dims, "10", "Size of every dimension" );
        trssRead( &thresh, "0.000001", "Max error value" );
    }

    for( c_dims = 1; c_dims <= c_dimss; c_dims++ )
    {
        count = 0;
        sum = 0;
        
        /* Filling dimension's array */
        for( i = 0; i <= CV_HIST_MAX_DIM; i++ ) d[i] = dims;
        
        /*Creating histogram*/
        hist1 = cvCreateHist( c_dims, d, type );
        hist2 = cvCreateHist( c_dims, d, type );
        
        /*Initializing histogram*/
        for( i = 0; i < c_dims; i++ ) d[i] = 0;
        for( i = 0; i < c_dims; )
        {
            v = (float)atsInitRandom( 0, 100 );
            *cvGetHistValue_nD( hist1, d ) = v;
            *cvGetHistValue_nD( hist2, d ) = v;
            count++;
            sum += v;
            for( i = 0; d[i]++, d[i] >= dims && i < CV_HIST_MAX_DIM; i++ ) d[i] = 0;
        }

        /*Checking values*/
        cvNormalizeHist( hist1, 1 );

        for( i = 0; i < c_dims; i++ ) d[i] = 0;
        for( i = 0; i < c_dims; )
        {
            if( fabs( cvQueryHistValue_nD( hist1, d ) - cvQueryHistValue_nD( hist2, d ) / sum ) >
                thresh )
            {
                trsWrite( ATS_CON | ATS_LST, "Error: act %f   exp %f address  ",
                          cvQueryHistValue_nD( hist1, d ), cvQueryHistValue_nD( hist2, d ) / sum );
                for( j = 0; j < c_dims; j++ ) trsWrite(ATS_CON | ATS_LST, "%d  ", d[j] );
                trsWrite(ATS_CON | ATS_LST, "\n");
                errors++;
            }
            for( i = 0; d[i]++, d[i] >= dims && i < CV_HIST_MAX_DIM; i++ ) d[i] = 0;
        }

        cvReleaseHist( &hist1 );
        cvReleaseHist( &hist2 );

    }

    return errors ? trsResult( TRS_FAIL, "Fixed %d errors", errors ) : TRS_OK;
}


static int foaThreshHist(void* _type)
{
    CvHistType type = (CvHistType)(int)_type;
    static int c_dimss;
    static int dims;
    static int init = 0;
    static float thresh;

    CvHistogram* hist1;
    CvHistogram* hist2;
    int d[CV_HIST_MAX_DIM + 1];
    int i, j, count, c_dims;
    float sum, v;
    int errors = 0;

    if( !init )
    {
        init = 1;
        trsiRead( &c_dimss, "1", "Number of dimensions" );
        trsiRead( &dims, "10", "Size of every dimension" );
        trssRead( &thresh, "50", "Threshold value" );
    }

    for( c_dims = 1; c_dims <= c_dimss; c_dims++ )
    {
        count = 0;
        sum = 0;
        
        /* Filling dimension's array */
        for( i = 0; i <= CV_HIST_MAX_DIM; i++ ) d[i] = dims;
        
        /*Creating histogram*/
        hist1 = cvCreateHist( c_dims, d, type );
        hist2 = cvCreateHist( c_dims, d, type );
        
        /*Initializing histogram*/

        for( i = 0; i < CV_HIST_MAX_DIM; i++ ) d[i] = 0;
        for( i = 0; i < c_dims; )
        {
            v = (float)atsInitRandom( 0, 100 );
            *cvGetHistValue_nD( hist1, d ) = v;
            *cvGetHistValue_nD( hist2, d ) = v;
            count++;
            sum += v;
            for( i = 0; d[i]++, d[i] >= dims && i < CV_HIST_MAX_DIM; i++ ) d[i] = 0;
        }

        /*Checking values*/
        cvThreshHist( hist1, thresh );
        for( i = 0; i < c_dims; i++ ) d[i] = 0;
        for( i = 0; i < c_dims; )
        {
            if( cvQueryHistValue_nD( hist1, d ) != 0 && cvQueryHistValue_nD( hist2, d ) < thresh ||
                cvQueryHistValue_nD( hist1, d ) != cvQueryHistValue_nD( hist2, d ) &&
                cvQueryHistValue_nD( hist2, d ) >= thresh )
            {
                trsWrite( ATS_CON | ATS_LST, "Error: act %f   exp %f address  ",
                          cvQueryHistValue_nD( hist1, d ),
                          cvQueryHistValue_nD( hist2, d ) >= thresh ?
                          cvQueryHistValue_nD( hist2, d ) : 0.0f );
                for( j = 0; j < c_dims; j++ ) trsWrite(ATS_CON | ATS_LST, "%d  ", d[j] );
                trsWrite(ATS_CON | ATS_LST, "\n");
                errors++;
            }
            for( i = 0; d[i]++, d[i] >= dims && i < CV_HIST_MAX_DIM; i++ ) d[i] = 0;
        }

        cvReleaseHist( &hist1 );
        cvReleaseHist( &hist2 );

    }

    return errors ? trsResult( TRS_FAIL, "Fixed %d errors", errors ) : TRS_OK;
}


static int foaHistCompare(void* _type)
{
    CvHistType type = (CvHistType)(int)_type;
    static int c_dimss;
    static int dims;
    static int init = 0;
    static float thresh;

    CvHistogram* hist1;
    CvHistogram* hist2;

    int c_dims;
    int d[CV_HIST_MAX_DIM + 1];
    double intersect, exp_intersect;
    double correl, exp_correl;
    double chisqr, exp_chisqr;
    double m1, m2, m3, mn1, mn2;
    int size;
    int i;
    
    int errors = 0;

    if( !init )
    {
        init = 1;
        trsiRead( &c_dimss, "6", "Number of dimensions" );
        trsiRead( &dims, "10", "Size of every dimension" );
        trssRead( &thresh, "0.0001", "Max error value" );
    }

    for( c_dims = 1; c_dims <= c_dimss; c_dims++ )
    {
        /*Creating histograms*/
        for( i = 0; i < c_dims; i++ ) d[i] = dims;
        hist1 = cvCreateHist( c_dims, d, type );
        hist2 = cvCreateHist( c_dims, d, type );

        /*Filling histograms*/
        /*hist1: y = x / size*/
        /*hist2: y = 1 - x / size*/
        size = hist1->mdims[0] * hist1->dims[0];
        for( i = 0, mn1 = mn2 = 0; i < size; i++ )
        {
            *cvGetHistValue_1D(hist1, i) = (float)i / (size - 1);
            *cvGetHistValue_1D(hist2, i) = 1 - (float)i / (size - 1);
            mn1 += cvQueryHistValue_1D(hist1, i);
            mn2 += cvQueryHistValue_1D(hist2, i);
        }

        mn1 /= size;
        mn2 /= size;

        intersect = cvCompareHist( hist1, hist2, CV_COMP_INTERSECT );
        correl = cvCompareHist( hist1, hist2, CV_COMP_CORREL );
        chisqr = cvCompareHist( hist1, hist2, CV_COMP_CHISQR );

        for( i = 0, exp_intersect = 0, exp_chisqr = 0,
             m1 = m2 = m3 = 0; i < size; i++ )
        {
            exp_intersect += MIN( cvQueryHistValue_1D(hist1, i), cvQueryHistValue_1D(hist2, i) );
            exp_chisqr += (cvQueryHistValue_1D(hist1, i) - cvQueryHistValue_1D(hist2, i)) *
                          (cvQueryHistValue_1D(hist1, i) - cvQueryHistValue_1D(hist2, i)) /
                          (cvQueryHistValue_1D(hist1, i) + cvQueryHistValue_1D(hist2, i));
            m1 += (cvQueryHistValue_1D(hist1, i) - mn1) * (cvQueryHistValue_1D(hist2, i) - mn2);
            m2 += (cvQueryHistValue_1D(hist2, i) - mn2) * (cvQueryHistValue_1D(hist2, i) - mn2);
            m3 += (cvQueryHistValue_1D(hist1, i) - mn1) * (cvQueryHistValue_1D(hist1, i) - mn1);
        }

        exp_correl = m1 / sqrt( m2 * m3 );

        if( fabs( intersect - exp_intersect ) > thresh * exp_intersect )
        {
            trsWrite( ATS_CON | ATS_LST, "Intersection error: act %f   exp %f\n",
                      (float)intersect, (float)exp_intersect );
            trsWrite( ATS_CON | ATS_LST, "Size% " );
            for( i = 0; i < c_dims; i++ ) trsWrite( ATS_CON | ATS_LST, "%d  ", d[i] );
            trsWrite( ATS_CON | ATS_LST, "\n" );
            errors++;
        }

        if( fabs( correl - exp_correl ) > thresh )
        {
            trsWrite( ATS_CON | ATS_LST, "Correlation error: act %f   exp %f\n",
                      (float)correl, (float)exp_correl );
            trsWrite( ATS_CON | ATS_LST, "Size% " );
            for( i = 0; i < c_dims; i++ ) trsWrite( ATS_CON | ATS_LST, "%d  ", d[i] );
            trsWrite( ATS_CON | ATS_LST, "\n" );
            errors++;
        }

        if( fabs( chisqr - exp_chisqr ) > thresh )
        {
            trsWrite( ATS_CON | ATS_LST, "ChiSqr error: act %f   exp %f\n",
                      (float)chisqr, (float)exp_chisqr );
            trsWrite( ATS_CON | ATS_LST, "Size% " );
            for( i = 0; i < c_dims; i++ ) trsWrite( ATS_CON | ATS_LST, "%d  ", d[i] );
            trsWrite( ATS_CON | ATS_LST, "\n" );
            errors++;
        }

        cvReleaseHist( &hist1 );
        cvReleaseHist( &hist2 );
    }

    return errors ? trsResult( TRS_FAIL, "Total fixed %d errors", errors ) : TRS_OK;
}


static int foaCopyHist(void* _type)
{
    CvHistType type = (CvHistType)(int)_type;
    static int c_dimss;
    static int dims;
    static int init = 0;
    static float thresh;

    CvHistogram* hist1;
    CvHistogram* hist2;

    int c_dims;
    int d[CV_HIST_MAX_DIM + 1];
    int size;
    int i;
    
    int errors = 0;

    if( !init )
    {
        init = 1;
        trsiRead( &c_dimss, "6", "Number of dimensions" );
        trsiRead( &dims, "10", "Size of every dimension" );
        trssRead( &thresh, "0.00001", "Max error value" );
    }

    for( c_dims = 1; c_dims <= c_dimss; c_dims++ )
    {
        /*Creating histograms*/
        for( i = 0; i < c_dims; i++ ) d[i] = dims;
        hist1 = cvCreateHist( c_dims, d, type );
        hist2 = 0;

        /*Filling histograms*/
        /*hist1: y = x / size*/
        size = hist1->mdims[0] * hist1->dims[0];
        for( i = 0; i < size; i++ )
            *cvGetHistValue_1D(hist1, i) = (float)i / (size - 1);
        cvCopyHist( hist1, &hist2 );

        if( cvCompareHist( hist1, hist2, CV_COMP_CHISQR ) != 0 )
        {
            trsWrite( ATS_CON | ATS_LST, "Intersection error: act %f   exp 0 - "
                                         "histograms not equals\n",
                      (float)cvCompareHist( hist1, hist2, CV_COMP_CHISQR ) );
            trsWrite( ATS_CON | ATS_LST, "Size% " );
            for( i = 0; i < c_dims; i++ ) trsWrite( ATS_CON | ATS_LST, "%d  ", d[i] );
            trsWrite( ATS_CON | ATS_LST, "\n" );
            errors++;
        }

        cvReleaseHist( &hist1 );
        cvReleaseHist( &hist2 );
    }

    return errors ? trsResult( TRS_FAIL, "Total fixed %d errors", errors ) : TRS_OK;
}


static int foaCalcHist(void* _type)
{
    CvHistType type = (CvHistType)(int)_type;
    static int c_dimss;
    static int dims;
    static int init = 0;
    static int width;
    static int height;

    CvHistogram* hist1;
    CvHistogram* hist2;
    CvHistogram* hist3;

    int    c_dims;
    int    d[CV_HIST_MAX_DIM + 1];
    float* thresh[CV_HIST_MAX_DIM];
    float* threshe[CV_HIST_MAX_DIM];
    int    i, j, x, y, fl;
    IplImage* src8u[CV_HIST_MAX_DIM];
    IplImage* src8s[CV_HIST_MAX_DIM];
    IplImage* src32f[CV_HIST_MAX_DIM];
    int    step8u, step8s, step32f;
    
    int errors = 0;

    if( !init )
    {
        init = 1;
        trsiRead( &c_dimss, "6", "Number of dimensions" );
        trsiRead( &dims, "10", "Size of every dimension" );
        trsiRead( &width, "160", "Width of source picture" );
        trsiRead( &height, "160", "Height of source picture" );
    }

    for( c_dims = 1; c_dims <= c_dimss; c_dims++ )
    {
        trsWrite( ATS_CON, "Dimension %d\n", c_dims );
        /*Creating & filling histograms*/
        for( i = 0; i < c_dims; i++ ) d[i] = dims;
        hist1 = cvCreateHist( c_dims, d, type );
        hist2 = cvCreateHist( c_dims, d, type );
        hist3 = cvCreateHist( c_dims, d, type );

        for( i = 0; i < c_dims; i++ )
        {
            thresh[i] = (float*)icvAlloc( (hist1->dims[i] + 1) * sizeof(**hist1->thresh));
            threshe[i] = (float*)icvAlloc( 2 * sizeof(**hist1->thresh));
            thresh[i][0] = threshe[i][0] = 0.5;
            threshe[i][1] = thresh[i][hist1->dims[i]] = 0.5f + hist1->dims[i];
            for( j = 1; j < hist1->dims[i]; j++ ) thresh[i][j] = 0.5f + j;
        }

        cvSetHistThresh( hist2, thresh, 0 );
        cvSetHistThresh( hist3, threshe, 1 );

        for( i = 0; i < c_dims; i++ )
        {
            src8u[i] = cvCreateImage(cvSize(width, height), IPL_DEPTH_8U, 1);
            src8s[i] = cvCreateImage(cvSize(width, height), IPL_DEPTH_8S, 1);
            src32f[i] = cvCreateImage(cvSize(width, height), IPL_DEPTH_32F, 1);
        }
        
        step8u = src8u[0]->widthStep;
        step8s = src8s[0]->widthStep;
        step32f = src32f[0]->widthStep;

        for( i = 0; i < c_dims; i++ )
            for( y = 0; y < height; y++ )
                for( x = 0; x < width; x++ )
                {
                    ((float*)(src32f[i]->imageData))[y * step32f / 4 + x] =
                        src8u[i]->imageData[y * step8s + x] =
                        src8s[i]->imageData[y * step8u + x] = (char)x;
                }

        /* Filling control histogram */
        for( i = 0; i <= CV_HIST_MAX_DIM; i++ ) d[i] = 0;
        for( i = 0; i < c_dims; )
        {
            for( j = 1, fl = 1; j < c_dims; j++ ) fl &= (d[j-1] == d[j]);
            (*cvGetHistValue_nD( hist1, d )) = (float)(fl * height);
            for( i = 0; d[i]++, d[i] >= dims && i < CV_HIST_MAX_DIM; i++ ) d[i] = 0;
        }

        /* Clearing histograms */
        if(type == CV_HIST_ARRAY)
        {
            memset(hist2->array, 1, hist2->dims[0] * hist2->mdims[0] * sizeof(*hist2->array));
            memset(hist3->array, 1, hist2->dims[0] * hist2->mdims[0] * sizeof(*hist2->array));
        }

        /* Calculating histograms 8u */
        cvCalcHist( src8u, hist2, 0 );
        cvCalcHist( src8u, hist3, 0 );
        /* Checking histogram */
        if( cvCompareHist( hist1, hist2, CV_COMP_CHISQR ) != 0 )
        {
            trsWrite( ATS_CON | ATS_LST, "Non uniform 8u calc, Intersection error: act %f   exp 0 - "
                                         "histograms not equals\n",
                      (float)cvCompareHist( hist1, hist2, CV_COMP_CHISQR ) );
            trsWrite( ATS_CON | ATS_LST, "Size " );
            for( i = 0; i < c_dims; i++ )
                trsWrite( ATS_CON | ATS_LST, "%d  ", hist1->dims[i] );
            trsWrite( ATS_CON | ATS_LST, "\n" );
            errors++;
        }
        if( cvCompareHist( hist1, hist3, CV_COMP_CHISQR ) != 0 )
        {
            trsWrite( ATS_CON | ATS_LST, "Uniform 8u calc, Intersection error: act %f   exp 0 - "
                                         "histograms not equals\n",
                      (float)cvCompareHist( hist1, hist3, CV_COMP_CHISQR ) );
            trsWrite( ATS_CON | ATS_LST, "Size " );
            for( i = 0; i < c_dims; i++ )
                trsWrite( ATS_CON | ATS_LST, "%d  ", hist1->dims[i] );
            trsWrite( ATS_CON | ATS_LST, "\n" );
            errors++;
        }

        /* Clearing histograms */
        if(type == CV_HIST_ARRAY)
        {
            memset(hist2->array, 1, hist2->dims[0] * hist2->mdims[0] * sizeof(*hist2->array));
            memset(hist3->array, 1, hist2->dims[0] * hist2->mdims[0] * sizeof(*hist2->array));
        }

        /* Calculating histograms 8s */
        cvCalcHist( src8s, hist2, 0 );
        cvCalcHist( src8s, hist3, 0 );
        /* Checking histogram */
        if( cvCompareHist( hist1, hist2, CV_COMP_CHISQR ) != 0 )
        {
            trsWrite( ATS_CON | ATS_LST, "Non uniform 8s calc, Intersection error: act %f   exp 0 - "
                                         "histograms not equals\n",
                      (float)cvCompareHist( hist1, hist2, CV_COMP_CHISQR ) );
            trsWrite( ATS_CON | ATS_LST, "Size " );
            for( i = 0; i < c_dims; i++ )
                trsWrite( ATS_CON | ATS_LST, "%d  ", hist1->dims[i] );
            trsWrite( ATS_CON | ATS_LST, "\n" );
            errors++;
        }
        if( cvCompareHist( hist1, hist3, CV_COMP_CHISQR ) != 0 )
        {
            trsWrite( ATS_CON | ATS_LST, "Uniform 8s calc, Intersection error: act %f   exp 0 - "
                                         "histograms not equals\n",
                      (float)cvCompareHist( hist1, hist3, CV_COMP_CHISQR ) );
            trsWrite( ATS_CON | ATS_LST, "Size " );
            for( i = 0; i < c_dims; i++ )
                trsWrite( ATS_CON | ATS_LST, "%d  ", hist1->dims[i] );
            trsWrite( ATS_CON | ATS_LST, "\n" );
            errors++;
        }

        /* Clearing histograms */
        if(type == CV_HIST_ARRAY)
        {
            memset(hist2->array, 1, hist2->dims[0] * hist2->mdims[0] * sizeof(*hist2->array));
            memset(hist3->array, 1, hist2->dims[0] * hist2->mdims[0] * sizeof(*hist2->array));
        }

        /* Calculating histograms 32f */
        cvCalcHist( src32f, hist2, 0 );
        cvCalcHist( src32f, hist3, 0 );
        /* Checking histogram */
        if( cvCompareHist( hist1, hist2, CV_COMP_CHISQR ) != 0 )
        {
            trsWrite( ATS_CON | ATS_LST, "Non uniform 32f calc, Intersection error: act %f   exp 0 - "
                                         "histograms not equals\n",
                      (float)cvCompareHist( hist1, hist2, CV_COMP_CHISQR ) );
            trsWrite( ATS_CON | ATS_LST, "Size " );
            for( i = 0; i < c_dims; i++ )
                trsWrite( ATS_CON | ATS_LST, "%d  ", hist1->dims[i] );
            trsWrite( ATS_CON | ATS_LST, "\n" );
            errors++;
        }
        if( cvCompareHist( hist1, hist3, CV_COMP_CHISQR ) != 0 )
        {
            trsWrite( ATS_CON | ATS_LST, "Uniform 32f calc, Intersection error: act %f   exp 0 - "
                                         "histograms not equals\n",
                      (float)cvCompareHist( hist1, hist3, CV_COMP_CHISQR ) );
            trsWrite( ATS_CON | ATS_LST, "Size " );
            for( i = 0; i < c_dims; i++ )
                trsWrite( ATS_CON | ATS_LST, "%d  ", hist1->dims[i] );
            trsWrite( ATS_CON | ATS_LST, "\n" );
            errors++;
        }

        for( i = 0; i < c_dims; i++ )
        {
            cvReleaseImage( &src8u[i] );
            cvReleaseImage( &src8s[i] );
            cvReleaseImage( &src32f[i] );
            icvFree( &thresh[i] );
            icvFree( &threshe[i] );
        }
        cvReleaseHist( &hist1 );
        cvReleaseHist( &hist2 );
        cvReleaseHist( &hist3 );

    }
    return errors ? trsResult( TRS_FAIL, "Total fixed %d errors", errors ) : TRS_OK;
}


static int foaCalcHistMask(void* _type)
{
    CvHistType type = (CvHistType)(int)_type;
    static int c_dimss;
    static int dims;
    static int init = 0;
    static int width;
    static int height;

    CvHistogram* hist1;
    CvHistogram* hist2;
    CvHistogram* hist3;

    int    c_dims;
    int    d[CV_HIST_MAX_DIM + 1];
    float* thresh[CV_HIST_MAX_DIM];
    float* threshe[CV_HIST_MAX_DIM];
    int    i, j, x, y, fl;
    IplImage* src8u[CV_HIST_MAX_DIM];
    IplImage* src8s[CV_HIST_MAX_DIM];
    IplImage* src32f[CV_HIST_MAX_DIM];
    int    step8u, step8s, step32f;
    IplImage* mask;
    int    mask_step;

    int errors = 0;

    if( !init )
    {
        init = 1;
        trsiRead( &c_dimss, "6", "Number of dimensions" );
        trsiRead( &dims, "10", "Size of every dimension" );
        trsiRead( &width, "16", "Width of source picture" );
        trsiRead( &height, "16", "Height of source picture" );
    }

    mask = cvCreateImage(cvSize(width, height), IPL_DEPTH_8U, 1);
    mask_step = mask->widthStep;
    for(i = 0; i < height * mask_step; i++) mask->imageData[i] = (uchar)(i & 1);

    for( c_dims = 1; c_dims <= c_dimss; c_dims++ )
    {
        trsWrite( ATS_CON, "Dimension %d\n", c_dims );
        /*Creating & filling histograms*/
        for( i = 0; i < c_dims; i++ ) d[i] = dims;
        hist1 = cvCreateHist( c_dims, d, type );
        hist2 = cvCreateHist( c_dims, d, type );
        hist3 = cvCreateHist( c_dims, d, type );

        for( i = 0; i < c_dims; i++ )
        {
            thresh[i] = (float*)icvAlloc( (hist1->dims[i] + 1) * sizeof(**hist1->thresh));
            threshe[i] = (float*)icvAlloc( 2 * sizeof(**hist1->thresh));
            thresh[i][0] = threshe[i][0] = 0.5;
            threshe[i][1] = thresh[i][hist1->dims[i]] = 0.5f + hist1->dims[i];
            for( j = 1; j < hist1->dims[i]; j++ ) thresh[i][j] = 0.5f + j;
        }

        cvSetHistThresh( hist2, thresh, 0 );
        cvSetHistThresh( hist3, threshe, 1 );

        for( i = 0; i < c_dims; i++ )
        {
            src8u[i] = cvCreateImage(cvSize(width, height), IPL_DEPTH_8U, 1);
            src8s[i] = cvCreateImage(cvSize(width, height), IPL_DEPTH_8S, 1);
            src32f[i] = cvCreateImage(cvSize(width, height), IPL_DEPTH_32F, 1);
        }
        
        step8u = src8u[0]->widthStep;
        step8s = src8s[0]->widthStep;
        step32f = src32f[0]->widthStep;

        for( i = 0; i < c_dims; i++ )
            for( y = 0; y < height; y++ )
                for( x = 0; x < width; x++ )
                {
                    ((float*)(src32f[i]->imageData))[y * step32f / 4 + x] =
                        src8u[i]->imageData[y * step8s + x] =
                        src8s[i]->imageData[y * step8u + x] = (char)x;
                }

        /* Filling control histogram */
        for( i = 0; i <= CV_HIST_MAX_DIM; i++ ) d[i] = 0;
        for( i = 0; i < c_dims; )
        {
            for( j = 1, fl = 1; j < c_dims; j++ ) fl &= (d[j-1] == d[j]);
            (*cvGetHistValue_nD( hist1, d )) = (float)(fl * height * !(d[0] & 1));
            for( i = 0; d[i]++, d[i] >= dims && i < CV_HIST_MAX_DIM; i++ ) d[i] = 0;
        }

        /* Clearing histograms */
        if(type == CV_HIST_ARRAY)
        {
            memset(hist2->array, 1, hist2->dims[0] * hist2->mdims[0] * sizeof(*hist2->array));
            memset(hist3->array, 1, hist2->dims[0] * hist2->mdims[0] * sizeof(*hist2->array));
        }

        /* Calculating histograms 8u */
        cvCalcHistMask( src8u, mask, hist2, 0 );
        cvCalcHistMask( src8u, mask, hist3, 0 );
        /* Checking histogram */
        if( cvCompareHist( hist1, hist2, CV_COMP_CHISQR ) != 0 )
        {
            trsWrite( ATS_CON | ATS_LST, "Non uniform 8u calc, Intersection error: act %f   exp 0 - "
                                         "histograms not equals\n",
                      (float)cvCompareHist( hist1, hist2, CV_COMP_CHISQR ) );
            trsWrite( ATS_CON | ATS_LST, "Size " );
            for( i = 0; i < c_dims; i++ )
                trsWrite( ATS_CON | ATS_LST, "%d  ", hist1->dims[i] );
            trsWrite( ATS_CON | ATS_LST, "\n" );
            errors++;
        }
        if( cvCompareHist( hist1, hist3, CV_COMP_CHISQR ) != 0 )
        {
            trsWrite( ATS_CON | ATS_LST, "Uniform 8u calc, Intersection error: act %f   exp 0 - "
                                         "histograms not equals\n",
                      (float)cvCompareHist( hist1, hist3, CV_COMP_CHISQR ) );
            trsWrite( ATS_CON | ATS_LST, "Size " );
            for( i = 0; i < c_dims; i++ )
                trsWrite( ATS_CON | ATS_LST, "%d  ", hist1->dims[i] );
            trsWrite( ATS_CON | ATS_LST, "\n" );
            errors++;
        }

        /* Clearing histograms */
        if(type == CV_HIST_ARRAY)
        {
            memset(hist2->array, 1, hist2->dims[0] * hist2->mdims[0] * sizeof(*hist2->array));
            memset(hist3->array, 1, hist2->dims[0] * hist2->mdims[0] * sizeof(*hist2->array));
        }

        /* Calculating histograms 8s */
        cvCalcHistMask( src8s, mask, hist2, 0 );
        cvCalcHistMask( src8s, mask, hist3, 0 );
        /* Checking histogram */
        if( cvCompareHist( hist1, hist2, CV_COMP_CHISQR ) != 0 )
        {
            trsWrite( ATS_CON | ATS_LST, "Non uniform 8s calc, Intersection error: act %f   exp 0 - "
                                         "histograms not equals\n",
                      (float)cvCompareHist( hist1, hist2, CV_COMP_CHISQR ) );
            trsWrite( ATS_CON | ATS_LST, "Size " );
            for( i = 0; i < c_dims; i++ )
                trsWrite( ATS_CON | ATS_LST, "%d  ", hist1->dims[i] );
            trsWrite( ATS_CON | ATS_LST, "\n" );
            errors++;
        }
        if( cvCompareHist( hist1, hist3, CV_COMP_CHISQR ) != 0 )
        {
            trsWrite( ATS_CON | ATS_LST, "Uniform 8s calc, Intersection error: act %f   exp 0 - "
                                         "histograms not equals\n",
                      (float)cvCompareHist( hist1, hist3, CV_COMP_CHISQR ) );
            trsWrite( ATS_CON | ATS_LST, "Size " );
            for( i = 0; i < c_dims; i++ )
                trsWrite( ATS_CON | ATS_LST, "%d  ", hist1->dims[i] );
            trsWrite( ATS_CON | ATS_LST, "\n" );
            errors++;
        }

        /* Clearing histograms */
        if(type == CV_HIST_ARRAY)
        {
            memset(hist2->array, 1, hist2->dims[0] * hist2->mdims[0] * sizeof(*hist2->array));
            memset(hist3->array, 1, hist2->dims[0] * hist2->mdims[0] * sizeof(*hist2->array));
        }

        /* Calculating histograms 32f */
        cvCalcHistMask( src32f, mask, hist2, 0 );
        cvCalcHistMask( src32f, mask, hist3, 0 );
        /* Checking histogram */
        if( cvCompareHist( hist1, hist2, CV_COMP_CHISQR ) != 0 )
        {
            trsWrite( ATS_CON | ATS_LST, "Non uniform 32f calc, Intersection error: act %f   exp 0 - "
                                         "histograms not equals\n",
                      (float)cvCompareHist( hist1, hist2, CV_COMP_CHISQR ) );
            trsWrite( ATS_CON | ATS_LST, "Size " );
            for( i = 0; i < c_dims; i++ )
                trsWrite( ATS_CON | ATS_LST, "%d  ", hist1->dims[i] );
            trsWrite( ATS_CON | ATS_LST, "\n" );
            errors++;
        }
        if( cvCompareHist( hist1, hist3, CV_COMP_CHISQR ) != 0 )
        {
            trsWrite( ATS_CON | ATS_LST, "Uniform 32f calc, Intersection error: act %f   exp 0 - "
                                         "histograms not equals\n",
                      (float)cvCompareHist( hist1, hist3, CV_COMP_CHISQR ) );
            trsWrite( ATS_CON | ATS_LST, "Size " );
            for( i = 0; i < c_dims; i++ )
                trsWrite( ATS_CON | ATS_LST, "%d  ", hist1->dims[i] );
            trsWrite( ATS_CON | ATS_LST, "\n" );
            errors++;
        }

        for( i = 0; i < c_dims; i++ )
        {
            cvReleaseImage( &src8u[i] );
            cvReleaseImage( &src8s[i] );
            cvReleaseImage( &src32f[i] );
            icvFree( &thresh[i] );
            icvFree( &threshe[i] );
        }
        cvReleaseHist( &hist1 );
        cvReleaseHist( &hist2 );
        cvReleaseHist( &hist3 );

    }
    cvReleaseImage(&mask);
    return errors ? trsResult( TRS_FAIL, "Total fixed %d errors", errors ) : TRS_OK;
}


static int foaBackProject(void* _type)
{
    CvHistType type = (CvHistType)(int)_type;
    static int c_dimss;
    static int dims;
    static int init = 0;
    static int width;
    static int height;

    CvHistogram* hist2;
    CvHistogram* hist3;

    int    c_dims;
    int    d[CV_HIST_MAX_DIM + 1];
    float* thresh[CV_HIST_MAX_DIM];
    float* threshe[CV_HIST_MAX_DIM];
    int    i, j, x, y, fl;
    IplImage* src8u[CV_HIST_MAX_DIM];
    IplImage* src8s[CV_HIST_MAX_DIM];
    IplImage* src32f[CV_HIST_MAX_DIM];
    IplImage* dst8u;
    IplImage* dst8s;
    IplImage* dst32f;
    int    step8u, step8s, step32f;
    
    int errors = 0;

    if( !init )
    {
        init = 1;
        trsiRead( &c_dimss, "6", "Number of dimensions" );
        trsiRead( &dims, "10", "Size of every dimension" );
        trsiRead( &width, "16", "Width of source picture" );
        trsiRead( &height, "16", "Height of source picture" );
    }

    for( c_dims = 1; c_dims <= c_dimss; c_dims++ )
    {
        trsWrite( ATS_CON | ATS_LST, "Dimension %d\n", c_dims );
        /*Creating & filling histograms*/
        for( i = 0; i < c_dims; i++ ) d[i] = dims;
        hist2 = cvCreateHist( c_dims, d, type );
        hist3 = cvCreateHist( c_dims, d, type );

        for( i = 0; i < c_dims; i++ )
        {
            thresh[i] = (float*)icvAlloc( (hist2->dims[i] + 1) * sizeof(**hist2->thresh));
            threshe[i] = (float*)icvAlloc( 2 * sizeof(**hist2->thresh));
            thresh[i][0] = threshe[i][0] = 0.5;
            threshe[i][1] = thresh[i][hist2->dims[i]] = 0.5f + hist2->dims[i];
            for( j = 1; j < hist2->dims[i]; j++ ) thresh[i][j] = 0.5f + j;
        }

        cvSetHistThresh( hist2, thresh, 0 );
        cvSetHistThresh( hist3, threshe, 1 );

        for( i = 0; i < c_dims; i++ )
        {
            src8u[i] = cvCreateImage( cvSize(width, height), IPL_DEPTH_8U, 1 );
            src8s[i] = cvCreateImage( cvSize(width, height), IPL_DEPTH_8S, 1 );
            src32f[i] = cvCreateImage( cvSize(width, height), IPL_DEPTH_32F, 1 );
        }

        step8u = src8u[0]->widthStep;
        step8s = src8s[0]->widthStep;
        step32f = src32f[0]->widthStep;

        dst8u = cvCreateImage( cvSize(width, height), IPL_DEPTH_8U, 1 );
        dst8s = cvCreateImage( cvSize(width, height), IPL_DEPTH_8S, 1 );
        dst32f = cvCreateImage( cvSize(width, height), IPL_DEPTH_32F, 1 );
        
        for( i = 0; i < c_dims; i++ )
            for( y = 0; y < height; y++ )
                for( x = 0; x < width; x++ )
                {
                    ((float*)(src32f[i]->imageData))[y * step32f / 4 + x] =
                        src8u[i]->imageData[y * step8s + x] =
                        src8s[i]->imageData[y * step8u + x] = (char)x;
                }

        /* Filling control histograms */
        for( i = 0; i <= CV_HIST_MAX_DIM; i++ ) d[i] = 0;
        for( i = 0; i < c_dims; )
        {
            for( j = 1, fl = 1; j < c_dims; j++ ) fl &= (d[j-1] == d[j]);
            (*cvGetHistValue_nD( hist2, d )) = (*cvGetHistValue_nD( hist3, d )) =
                (float)(fl * height);
            for( i = 0; d[i]++, d[i] >= dims && i < CV_HIST_MAX_DIM; i++ ) d[i] = 0;
        }

        /* Calculating back project 8u non uniform */
        cvCalcBackProject( src8u, dst8u, hist2 );
        /* Checking results */
        for( y = 0; y < height; y++ )
        {
            for( x = 0; x < width; x++ )
            {
                if( (x > dims || x < 1) && dst8u->imageData[y * step8u + x] ||
                    (x <= dims && x >= 1) && dst8u->imageData[y * step8u + x] != height )
                {
                    trsWrite( ATS_CON | ATS_LST, "Bad destination image 8u (non uniform) act: %d, exp: %d,"
                                                 " x=%d  y=%d\n",
                                                 (int)dst8u->imageData[y * step8u + x],
                                                 x > dims || x < 1 ? 0 : height,
                                                 x, y );
                    errors++;
                }
            }
        }

        /* Calculating back project 8u uniform */
        cvCalcBackProject( src8u, dst8u, hist3 );
        /* Checking results */
        for( y = 0; y < height; y++ )
        {
            for( x = 0; x < width; x++ )
            {
                if( (x > dims || x < 1) && dst8u->imageData[y * step8u + x] ||
                    (x <= dims && x >= 1) && dst8u->imageData[y * step8u + x] != height )
                {
                    trsWrite( ATS_CON | ATS_LST, "Bad destination image 8u (uniform) act: %d, exp: %d,"
                                                 " x=%d  y=%d\n",
                                                 (int)dst8u->imageData[y * step8u + x],
                                                 x > dims || x < 1 ? 0 : height,
                                                 x, y );
                    errors++;
                }
            }
        }

        /* Calculating back project 8s non uniform */
        cvCalcBackProject( src8s, dst8s, hist2 );
        /* Checking results */
        for( y = 0; y < height; y++ )
        {
            for( x = 0; x < width; x++ )
            {
                if( (x > dims || x < 1) && dst8s->imageData[y * step8s + x] ||
                    (x <= dims && x >= 1) && dst8s->imageData[y * step8s + x] != height )
                {
                    trsWrite( ATS_CON | ATS_LST, "Bad destination image 8s (non uniform) act: %d, exp: %d,"
                                                 " x=%d  y=%d\n",
                                                 (int)dst8s->imageData[y * step8s + x],
                                                 x > dims || x < 1 ? 0 : height,
                                                 x, y );
                    errors++;
                }
            }
        }

        /* Calculating back project 8s uniform */
        cvCalcBackProject( src8s, dst8s, hist3 );
        /* Checking results */
        for( y = 0; y < height; y++ )
        {
            for( x = 0; x < width; x++ )
            {
                if( (x > dims || x < 1) && dst8s->imageData[y * step8s + x] ||
                    (x <= dims && x >= 1) && dst8s->imageData[y * step8s + x] != height )
                {
                    trsWrite( ATS_CON | ATS_LST, "Bad destination image 8s (uniform) act: %d, exp: %d,"
                                                 " x=%d  y=%d\n",
                                                 (int)dst8s->imageData[y * step8s + x],
                                                 x > dims || x < 1 ? 0 : height,
                                                 x, y );
                    errors++;
                }
            }
        }

        /* Calculating back project 32f non uniform */
        cvCalcBackProject( src32f, dst32f, hist2 );
        /* Checking results */
        for( y = 0; y < height; y++ )
        {
            for( x = 0; x < width; x++ )
            {
                if( (x > dims || x < 1) && ((float*)(dst32f->imageData))[y * step32f / 4 + x] ||
                    (x <= dims && x >= 1) && ((float*)(dst32f->imageData))[y * step32f / 4 + x] != height )
                {
                    trsWrite( ATS_CON | ATS_LST, "Bad destination image 32f (non uniform) act: %d, exp: %d,"
                                                 " x=%d  y=%d\n",
                                                 (int)((float*)(dst32f->imageData))[y * step32f / 4 + x],
                                                 x > dims || x < 1 ? 0 : height,
                                                 x, y );
                    errors++;
                }
            }
        }

        /* Calculating back project 32f uniform */
        cvCalcBackProject( src32f, dst32f, hist3 );
        /* Checking results */
        for( y = 0; y < height; y++ )
        {
            for( x = 0; x < width; x++ )
            {
                if( (x > dims || x < 1) && ((float*)(dst32f->imageData))[y * step32f / 4 + x] ||
                    (x <= dims && x >= 1) && ((float*)(dst32f->imageData))[y * step32f / 4 + x] != height )
                {
                    trsWrite( ATS_CON | ATS_LST, "Bad destination image 32f (uniform) act: %d, exp: %d,"
                                                 " x=%d  y=%d\n",
                                                 (int)((float*)(dst32f->imageData))[y * step32f / 4 + x],
                                                 x > dims || x < 1 ? 0 : height,
                                                 x, y );
                    errors++;
                }
            }
        }

        for( i = 0; i < c_dims; i++ )
        {
            cvReleaseImage( &src8u[i] );
            cvReleaseImage( &src8s[i] );
            cvReleaseImage( &src32f[i] );
            icvFree( &thresh[i] );
            icvFree( &threshe[i] );
        }

        cvReleaseImage( &dst8u );
        cvReleaseImage( &dst8s );
        cvReleaseImage( &dst32f );

        cvReleaseHist( &hist2 );
        cvReleaseHist( &hist3 );

    }
    return errors ? trsResult( TRS_FAIL, "Total fixed %d errors", errors ) : TRS_OK;
}/* foaBackProject */


static int myBackProjectPatch(IplImage** src8u, IplImage** src8s, IplImage** src32f,
                              CvHistogram* hist, IplImage* _dst8u, IplImage* _dst8s, IplImage* _dst32f,
                              float norm_factor, CvCompareMethod method,
                              CvSize range)
{
    CvSize roi = cvSize(src8u[0]->width, src8u[0]->height);
    int    step = (((roi.width + range.width * 2 + 1) * 4) | 15) + 1;

    uchar* _test8u[CV_HIST_MAX_DIM];
    char*  _test8s[CV_HIST_MAX_DIM];
    float* _test32f[CV_HIST_MAX_DIM];
    uchar* test8u[CV_HIST_MAX_DIM];
    char*  test8s[CV_HIST_MAX_DIM];
    float* test32f[CV_HIST_MAX_DIM];

    IplImage** img8u;
    IplImage** img8s;
    IplImage** img32f;

    float* dst8u = (float*)_dst8u->imageData;
    float* dst8s = (float*)_dst8s->imageData;
    float* dst32f = (float*)_dst32f->imageData;

    int    dst_step = _dst8u->widthStep;

    int i, x, y;
    CvHistogram* model = 0;

    img8u = (IplImage**)icvAlloc( hist->c_dims * sizeof(img8u[0]));
    img8s = (IplImage**)icvAlloc( hist->c_dims * sizeof(img8s[0]));
    img32f= (IplImage**)icvAlloc( hist->c_dims * sizeof(img32f[0]));

    for(i = 0; i < hist->c_dims; i++)
    {
        CvSize img_size = cvSize(range.width * 2 + 1, range.height * 2 + 1);
        
        _test8u[i] = (uchar*)icvAlloc(step * (roi.height + range.height * 2 + 1));
        _test8s[i] = (char*)icvAlloc(step * (roi.height + range.height * 2 + 1));
        _test32f[i] = (float*)icvAlloc(step * (roi.height + range.height * 2 + 1));

        memset(_test8u[i], 0, step * (roi.height + range.height * 2 + 1));
        memset(_test8s[i], 0, step * (roi.height + range.height * 2 + 1));
        memset(_test32f[i], 0, step * (roi.height + range.height * 2 + 1));

        img8u[i] = cvCreateImageHeader( img_size, IPL_DEPTH_8U, 1 );
        img8s[i] = cvCreateImageHeader( img_size, IPL_DEPTH_8S, 1 );
        img32f[i] = cvCreateImageHeader( img_size, IPL_DEPTH_32F, 1 );


    }

    for(i = 0; i < hist->c_dims; i++)
    {
        for(y = 0; y < roi.height; y++)
            for(x = 0; x < roi.width; x++)
            {
                _test8u[i][(y + range.height) * step + x + range.width] =
                    src8u[i]->imageData[y * src8u[i]->widthStep + x];

                _test8s[i][(y + range.height) * step + x + range.width] =
                    src8s[i]->imageData[y * src8s[i]->widthStep + x];

                _test32f[i][(y + range.height) * step / 4 + x + range.width] =
                    ((float*)(src32f[i]->imageData))[y * src32f[i]->widthStep / 4 + x];
            }
        test8u[i] = _test8u[i];
        test8s[i] = _test8s[i];
        test32f[i] = _test32f[i];
    }

    cvCopyHist(hist, &model);

    for(y = 0; y < roi.height; y++, dst8u += dst_step / 4, dst8s += dst_step / 4,
                                    dst32f += dst_step / 4)
    {
        for(x = 0; x < roi.width; x++)
        {
            for(i = 0; i < hist->c_dims; i++)
            {
                cvSetImageData( img8u[i], test8u[i], step );
                cvSetImageData( img8s[i], test8s[i], step );
                cvSetImageData( img32f[i], test32f[i], step );
            }
            
            cvCalcHist( img8u, model, 0);
            cvNormalizeHist(model, norm_factor);
            dst8u[x] = (float)cvCompareHist(hist, model, method);
            cvCalcHist( img8s, model, 0);
            cvNormalizeHist(model, norm_factor);
            dst8s[x] = (float)cvCompareHist(hist, model, method);
            cvCalcHist( img32f, model, 0);
            cvNormalizeHist(model, norm_factor);
            dst32f[x] = (float)cvCompareHist(hist, model, method);

            for(i = 0; i < hist->c_dims; i++)
            {
                test8u[i]++; test8s[i]++; test32f[i]++;
            }
        }
        for(i = 0; i < hist->c_dims; i++)
        {
            test8u[i] += step - roi.width;
            test8s[i] += step - roi.width;
            test32f[i] += step / 4 - roi.width;
        }
    }
    cvReleaseHist(&model);
    for(i = 0; i < hist->c_dims; i++)
    {
        icvFree(&_test8u[i]);
        icvFree(&_test8s[i]);
        icvFree(&_test32f[i]);
        cvReleaseImageHeader( img8u + i );
        cvReleaseImageHeader( img8s + i );
        cvReleaseImageHeader( img32f + i );
    }

    icvFree( &img8u );
    icvFree( &img8s );
    icvFree( &img32f );

    return TRS_OK;
}


static int foaBackProjectPatch(void* _type)
{
    CvHistType type = (CvHistType)(int)_type;
    static int c_dimss;
    static int dims;
    static int init = 0;
    static int width;
    static int height;
    static int range_width;
    static int range_height;

    CvHistogram* hist;

    int    c_dims;
    int    d[CV_HIST_MAX_DIM + 1];
    float* thresh[CV_HIST_MAX_DIM];
    int    i, j, x, y, fl;
    IplImage* src8u[CV_HIST_MAX_DIM];
    IplImage* src8s[CV_HIST_MAX_DIM];
    IplImage* src32f[CV_HIST_MAX_DIM];
    IplImage* dst8u;
    IplImage* dst8s;
    IplImage* dst32f;
    IplImage* _dst8u;
    IplImage* _dst8s;
    IplImage* _dst32f;
    
    int errors = 0;
    int l_err;

    if( !init )
    {
        init = 1;
        trsiRead( &c_dimss, "4", "Number of dimensions" );
        trsiRead( &dims, "10", "Size of every dimension" );
        trsiRead( &width, "16", "Width of source picture" );
        trsiRead( &height, "16", "Height of source picture" );
        trsiRead( &range_width, "3", "Width of range" );
        trsiRead( &range_height, "3", "Height of range" );
    }

    for( c_dims = 1; c_dims <= c_dimss; c_dims++ )
    {
        trsWrite( ATS_CON | ATS_LST, "Dimension %d\n", c_dims );
        /*Creating & filling histograms*/
        for( i = 0; i < c_dims; i++ ) d[i] = dims;
        hist = cvCreateHist( c_dims, d, type );

        for( i = 0; i < c_dims; i++ )
        {
            thresh[i] = (float*)icvAlloc( (hist->dims[i] + 1) * sizeof(**hist->thresh));
            thresh[i][0] = 0.5;
            for( j = 1; j <= hist->dims[i]; j++ ) thresh[i][j] = 0.5f + j;
        }

        cvSetHistThresh( hist, thresh, 0 );

        for( i = 0; i < c_dims; i++ )
        {
            src8u[i] = cvCreateImage(cvSize(width, height), IPL_DEPTH_8U, 1);
            src8s[i] = cvCreateImage(cvSize(width, height), IPL_DEPTH_8S, 1);
            src32f[i] = cvCreateImage(cvSize(width, height), IPL_DEPTH_32F, 1);
        }

        dst8u = cvCreateImage(cvSize(width, height), IPL_DEPTH_32F, 1);
        dst8s = cvCreateImage(cvSize(width, height), IPL_DEPTH_32F, 1);
        dst32f = cvCreateImage(cvSize(width, height), IPL_DEPTH_32F, 1);

        _dst8u = cvCreateImage(cvSize(width, height), IPL_DEPTH_32F, 1);
        _dst8s = cvCreateImage(cvSize(width, height), IPL_DEPTH_32F, 1);
        _dst32f = cvCreateImage(cvSize(width, height), IPL_DEPTH_32F, 1);
        
        for( i = 0; i < c_dims; i++ )
            for( y = 0; y < height; y++ )
                for( x = 0; x < width; x++ )
                {
                    ((float*)(src32f[i]->imageData))[y * src32f[i]->widthStep / 4 + x] =
                        src8u[i]->imageData[y * src8u[i]->widthStep + x] =
                        src8s[i]->imageData[y * src8s[i]->widthStep + x] = (char)x;
                }

        /* Filling control histograms */
        for( i = 0; i <= CV_HIST_MAX_DIM; i++ ) d[i] = 0;
        for( i = 0; i < c_dims; )
        {
            for( j = 1, fl = 1; j < c_dims; j++ ) fl &= (d[j-1] == d[j]);
            (*cvGetHistValue_nD( hist, d )) = (float)(fl * height);
            for( i = 0; d[i]++, d[i] >= dims && i < CV_HIST_MAX_DIM; i++ ) d[i] = 0;
        }

        myBackProjectPatch(src8u, src8s, src32f, hist, _dst8u, _dst8s, _dst32f, 
                           1, CV_COMP_CHISQR,
                           cvSize(range_width,range_height));

        cvCalcBackProjectPatch( src8u, dst8u, cvSize(range_width,range_height),
                                hist, CV_COMP_CHISQR, 1 );
        /* Checking results */
        l_err = atsCompare2Dfl((float*)dst8u->imageData, (float*)_dst8u->imageData,
                               cvSize(width, height), dst8u->widthStep, 0);
        errors += l_err;
        if(l_err)
            trsWrite( ATS_CON | ATS_LST, "cvCalcBackProjectPatch(8u) - error" );

        cvCalcBackProjectPatch( src8s, dst8s, cvSize(range_width,range_height),
                                hist, CV_COMP_CHISQR, 1 );
        /* Checking results */
        l_err = atsCompare2Dfl((float*)dst8s->imageData, (float*)_dst8s->imageData,
                               cvSize(width, height), dst8s->widthStep, 0);
        errors += l_err;
        if(l_err)
            trsWrite( ATS_CON | ATS_LST, "cvCalcBackProjectPatch(8s) - error" );

        cvCalcBackProjectPatch( src32f, dst32f, cvSize(range_width,range_height),
                                hist, CV_COMP_CHISQR, 1 );
        /* Checking results */
        l_err = atsCompare2Dfl((float*)dst32f->imageData, (float*)_dst32f->imageData,
                               cvSize(width, height), dst32f->widthStep, 0);
        errors += l_err;
        if(l_err)
            trsWrite( ATS_CON | ATS_LST, "cvCalcBackProjectPatch(32f) - error" );

        for( i = 0; i < c_dims; i++ )
        {
            cvReleaseImage( &src8u[i] );
            cvReleaseImage( &src8s[i] );
            cvReleaseImage( &src32f[i] );
            icvFree( &thresh[i] );
        }

        cvReleaseImage( &_dst8u );
        cvReleaseImage( &_dst8s );
        cvReleaseImage( &_dst32f );
        cvReleaseImage( &dst8u );
        cvReleaseImage( &dst8s );
        cvReleaseImage( &dst32f );

        cvReleaseHist( &hist );

    }
    return errors ? trsResult( TRS_FAIL, "Total fixed %d errors", errors ) : TRS_OK;
}/* foaBackProjectPatch */


static int foaBayesianProb(void* _type)
{
    int errors = 0;

    CvHistType type = (CvHistType)(int)_type;
    static int c_dimss;
    static int dims;
    static int init = 0;
    static int width;
    static int height;
    static int range_width;
    static int range_height;
    static double acc;

    int    c_dims;
    int    d[CV_HIST_MAX_DIM + 1];
    CvHistogram* src[10];
    CvHistogram* dst[10];
    int    i, j;
    float* sum = 0;

    if( !init )
    {
        init = 1;
        trsiRead( &c_dimss, "5", "Number of dimensions" );
        trsiRead( &dims, "10", "Size of every dimension" );
        trsdRead( &acc, "0.01", "Accuracy" );
    }

    AtsRandState state;
    int tickcount = -809287112;//(int)atsGetTickCount();
    atsRandInit( &state, -100, 100, tickcount );

    trsWrite( ATS_CON | ATS_LST, "seed = %d\n", tickcount );

    for( c_dims = 1; c_dims <= c_dimss; c_dims++ )
    {
        trsWrite( ATS_CON | ATS_LST, "Dimension %d\n", c_dims );
        /*Creating & filling histograms*/
        for( i = 0; i < c_dims; i++ ) d[i] = dims;

        for( i = 0; i < 10; i++ )
        {
            src[i] = cvCreateHist( c_dims, d, type );
            dst[i] = cvCreateHist( c_dims, d, type );
        }

        int size = src[0]->dims[0] * src[0]->mdims[0];

        sum = (float*)malloc( size * sizeof(float) );
        memset( sum, 0, size * sizeof(float) );

        for( i = 0; i < size; i++ )
            for( j = 0; j < 10; j++ )
            {
                float val = atsRand32f( &state );
                *cvGetHistValue_1D( src[j], i ) = val;
                sum[i] += val;
            }

        // run function
        cvCalcBayesianProb( src, 10, dst );

        // check results
        for( i = 0; i < size; i++ )
            for( j = 0; j < 10; j++ )
            {
                float src_val = cvQueryHistValue_1D( src[j], i );
                float dst_val = cvQueryHistValue_1D( dst[j], i );
                if( atsCompSingle( dst_val * sum[i],
                                   sum[i] ? src_val : 0,
                                   acc ) )
                {
                    trsWrite( ATS_CON | ATS_LST, "error: hist %d pos %d, act %f  exp %f\n",
                              j, i,
                              dst_val * sum[i],
                              sum[i] ? src_val : 0 );
                    errors++;
                }
            }


        free( sum );
        for( i = 0; i < 10; i++ )
        {
            cvReleaseHist( &src[i] );
            cvReleaseHist( &dst[i] );
        }
    }
    return errors ? trsResult( TRS_FAIL, "Total fixed %d errors", errors ) : TRS_OK;
}/* foaBayesianProb */


void InitAHistograms()
{
    trsRegArg( "cvGetHistValue_1D, cvGetHistValue_2D, cvGetHistValue_3D, cvGetHistValue_nD, "
               "cvQueryHistValue_1D, cvQueryHistValue_2D, cvQueryHistValue_3D, cvQueryHistValue_nD",
               "Histogram Get/Query functions algorithm test",
               "Algorithm", foaHistGetQueryValue, CV_HIST_ARRAY );
    trsRegArg( "cvGetHistValue_1D, cvGetHistValue_2D, cvGetHistValue_3D, cvGetHistValue_nD, "
               "cvQueryHistValue_1D, cvQueryHistValue_2D, cvQueryHistValue_3D, cvQueryHistValue_nD",
               "Histogram Get/Query functions algorithm test",
               "Algorithm", foaHistGetQueryValue, CV_HIST_TREE );

    trsRegArg( "cvGetMinMaxHistValue",
               "Histogram cvGetMinMaxHistValue function algorithm test",
               "Algorithm", foaHistMinMaxValue, CV_HIST_ARRAY );
    trsRegArg( "cvGetMinMaxHistValue",
               "Histogram cvGetMinMaxHistValue function algorithm test",
               "Algorithm", foaHistMinMaxValue, CV_HIST_TREE );

    trsRegArg( "cvNormalizeHist",
               "Histogram cvNormalizeHist function algorithm test",
               "Algorithm", foaNormalizeHist, CV_HIST_ARRAY );
    trsRegArg( "cvNormalizeHist",
               "Histogram cvNormalizeHist function algorithm test",
               "Algorithm", foaNormalizeHist, CV_HIST_TREE );

    trsRegArg( "cvThreshHist",
               "Histogram cvThreshHist function algorithm test",
               "Algorithm", foaThreshHist, CV_HIST_ARRAY );
    trsRegArg( "cvThreshHist",
               "Histogram cvThreshHist function algorithm test",
               "Algorithm", foaThreshHist, CV_HIST_TREE );

    trsRegArg( "cvCompareHist",
               "Histogram comparing function algorithm test",
               "Algorithm", foaHistCompare, CV_HIST_ARRAY );
    trsRegArg( "cvCompareHist",
               "Histogram comparing function algorithm test",
               "Algorithm", foaHistCompare, CV_HIST_TREE );

    trsRegArg( "cvCopyHist",
               "Histogram copying function algorithm test",
               "Algorithm", foaCopyHist, CV_HIST_ARRAY );
    trsRegArg( "cvCopyHist",
               "Histogram copying function algorithm test",
               "Algorithm", foaCopyHist, CV_HIST_TREE );

    trsRegArg( "cvCalcHist",
               "Histogram calculation function algorithm test",
               "Algorithm", foaCalcHist, CV_HIST_ARRAY );
    trsRegArg( "cvCalcHist8uC1R",
               "Histogram calculation function algorithm test",
               "Algorithm", foaCalcHist, CV_HIST_TREE );

    trsRegArg( "cvCalcHistMask",
               "Histogram calculation function algorithm test",
               "Algorithm", foaCalcHistMask, CV_HIST_ARRAY );
    trsRegArg( "cvCalcHistMask8uC1R",
               "Histogram calculation function algorithm test",
               "Algorithm", foaCalcHistMask, CV_HIST_TREE );

    trsRegArg( "cvCalcBackProject",
               "Back project calculation function algorithm test",
               "Algorithm", foaBackProject, CV_HIST_ARRAY );
    trsRegArg( "cvCalcBackProject",
               "Back project calculation function algorithm test",
               "Algorithm", foaBackProject, CV_HIST_TREE );

    trsRegArg( "cvCalcBackProjectPatch",
               "Back project patch calculation function algorithm test",
               "Algorithm", foaBackProjectPatch, CV_HIST_ARRAY );
    trsRegArg( "cvCalcBackProjectPatch",
               "Back project patch calculation function algorithm test",
               "Algorithm", foaBackProjectPatch, CV_HIST_TREE );

    trsRegArg( "cvCalcBayesianProb",
               "Bayesian Prob calculation function algorithm test",
               "Algorithm", foaBayesianProb, CV_HIST_ARRAY );
    trsRegArg( "cvCalcBayesianProb",
               "Bayesian Prob calculation function algorithm test",
               "Algorithm", foaBayesianProb, CV_HIST_TREE );
}


/* End Of File */
