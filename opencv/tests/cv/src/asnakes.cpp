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

#define SCAN  0
#define CHECK 1

#include <stdio.h>
#include <string.h>

/* Testing parameters */
static char test_desc[] = "Snakes regression test";

/*  This is regression test for Snakes functions of OpenCV.
//  This test will generate fixed figure, read initial position 
//  of snake from file, run OpenCV function and compare result 
//  position of snake with position(from file) which must be resulting.
//  
//  Test is considered to be succesfull if resultant positions 
//  are identical.
*/    
static char* func_name[] = 
{
    "cvSnakeImage"
};

static const int numfig_image = 1;
static const int numfig_grad  = 1;

static char* file_name[] = 
{
    "ring",
    "square"
}; 
              

#ifndef _MAX_PATH
#define _MAX_PATH 1000
#endif

static int data_type = 0;
static int fmaSnakes( void* arg )
{
    int lParam = (int)(size_t)arg;
    FILE* file;
    char abs_file_name[_MAX_PATH];
    char rel_path[_MAX_PATH];

    int i,j;

    /* source image */
    IplImage* iplSrc = NULL;
    CvSize win;
    int length;
    
    float alpha,beta,gamma;
    CvTermCriteria criteria;

    long lErrors = 0;

    static int  read_param = 0;

    atsGetTestDataPath( rel_path, "snakes", 0, 0 );

    /* Initialization global parameters */
    if( !read_param )
    {
        read_param = 1;
        /* Determine which test are needed to run */
        trsCaseRead( &data_type,"/u/s/a", "u",
                     "a - all, 8u - unsigned char, 8s - char" );
    }

    criteria.type = CV_TERMCRIT_ITER;
    win.height = win.width = 3;
        
    for ( i = 0; i < numfig_image + numfig_grad; i++ )
    {           
        CvPoint* Pts;
        CvPoint* resPts;
    
        int num_pos;
        int k;

        char tmp[_MAX_PATH];
        /* create full name of bitmap file */
        strcpy(tmp, rel_path);
        strcat(tmp, file_name[i]);
        strcpy( abs_file_name, tmp ); 
        strcat( abs_file_name, ".bmp" );

        /* read bitmap with 8u image */
        iplSrc = atsCreateImageFromFile( abs_file_name );
        
        if (!iplSrc) 
            return trsResult( TRS_FAIL, "can't open BMP" );
        if (iplSrc->nChannels != 1 ) 
            return trsResult( TRS_FAIL, "BMP has no 8u format" );
        
        /* init snake reading file with snake */
        strcpy(tmp, rel_path);
        strcat(tmp, file_name[i]);
        strcpy( abs_file_name, tmp ); 
        strcat( abs_file_name, ".txt" );
        
        file = fopen( abs_file_name, "r+" );

        if (!file)
        {
            return trsResult( TRS_FAIL, "Can't open file %s",
                                        abs_file_name );
        }
    
        /* read snake parameters */
        fscanf(file, "%d", &length );
        fscanf(file, "%f", &alpha );
        fscanf(file, "%f", &beta );
        fscanf(file, "%f", &gamma );
        
        /* allocate memory for snakes */
        Pts = (CvPoint*)malloc( length * sizeof(CvPoint) ); 
        resPts = (CvPoint*)malloc( length * sizeof(CvPoint) ); 

        /* get number of snake positions */
        fscanf(file, "%d", &num_pos );

        /* get number iterations between two positions */
        fscanf(file, "%d", &criteria.max_iter ); 

        /* read initial snake position */
        for ( j = 0; j < length; j++ )
        {
            fscanf(file, "%d%d", &Pts[j].x, &Pts[j].y );
        }
        
        for ( k = 0; k < num_pos; k++ )
        {
            /* Run CVL function to check it */
            if(i<numfig_image)
            {
                 cvSnakeImage( iplSrc, Pts, length, 
                           &alpha, &beta, &gamma, CV_VALUE, win, criteria, 0 );
            }
            else
            {
                cvSnakeImage( iplSrc, Pts, length, 
                           &alpha, &beta, &gamma, CV_VALUE, win, criteria, 1 /*usegrad*/ );
            }

            if ( lParam & CHECK )
            {
                for ( j = 0; j < length; j++ )
                {
                    fscanf(file, "%d%d", &resPts[j].x, &resPts[j].y );
                     
                    lErrors += (Pts[j].x != resPts[j].x);
                    lErrors += (Pts[j].y != resPts[j].y);
                }
            }
            else
            {
                fseek( file, 0, SEEK_CUR ); 
                fprintf(file, "\n");
                for ( j = 0; j < length; j++ )
                {
                    fprintf(file, "\n%d %d", Pts[j].x, Pts[j].y );                          
                }                              
            }            
        }
        fclose(file);
        free((void*)Pts);
        free((void*)resPts); 
        atsReleaseImage(iplSrc);
    }    

   if( lErrors == 0 ) return trsResult( TRS_OK, "No errors fixed for this text" );
   else return trsResult( TRS_FAIL, "Total fixed %d errors", lErrors );

}        

void InitASnakes(void)
{
    /* Register test function */
    /*trsRegArg( func_name[0], test_desc, atsAlgoClass, fmaSnakes, SCAN );*/
    trsRegArg( func_name[0], test_desc, atsAlgoClass, fmaSnakes, CHECK );
} /* InitASnakes */
