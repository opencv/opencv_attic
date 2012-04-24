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
#ifdef WIN32
#include <conio.h>
#endif

static char *cTestName[] = 
{
    "Camera Calibration Tests",
};

static char cTestClass[] = "Algorithm";

static char *cFuncName[] = 
{
    "cvCameraCalibration",
};


static int calibrationTest(void *)
{
    char            filepath[100];
    char            filename[100];
    char            datafilesname[100];
    
    CvSize          imageSize;
    CvSize          etalonSize;
    int             numImages;
    
    CvPoint2D64d*   imagePoints;
    CvPoint3D64d*   objectPoints;
    CvPoint2D64d*   reprojectPoints;

    CvVect64d       transVects;
    CvMatr64d       rotMatrs;

    CvVect64d       goodTransVects;
    CvMatr64d       goodRotMatrs;

    double          cameraMatrix[3*3];
    double          distortion[4];

    double          goodDistortion[4];

    int*            numbers;
    FILE*           file;
    FILE*           datafile;
    int             i,j;
    int             currImage;
    int             currPoint;

    int             useIntrinsicGuess;
    char            i_dat_file[100];

    const char*     i_datafiles = "datafiles.txt";

    int             Errors = 0;
    
    int             numPoints;

    imagePoints     = 0;
    objectPoints    = 0;
    reprojectPoints = 0;
    numbers         = 0;

    transVects      = 0;
    rotMatrs        = 0;
    goodTransVects  = 0;
    goodRotMatrs    = 0;
    
    atsGetTestDataPath( filepath, "cameracalibration", 0, 0 );

    strcpy( datafilesname, filepath );
    strcat( datafilesname, i_datafiles );

    datafile = fopen(datafilesname,"r");
    if( datafile == 0 ) 
    {
        trsWrite( ATS_CON | ATS_SUM,
                  "Can't open file with list of test files: %s\n",datafilesname);
        Errors++;
        goto test_exit;
    }

    int numTests;
    int currTest;
    fscanf(datafile,"%d",&numTests);

    for( currTest = 0; currTest < numTests; currTest++ )
    {
        fscanf(datafile,"%s",i_dat_file);
        strcpy(filename,filepath);
        strcat(filename,i_dat_file);
        file = fopen(filename,"r");

        if( file == 0 )
        {
            trsWrite( ATS_CON | ATS_SUM,
                      "Can't open current test file: %s\n",i_dat_file);
            Errors++;
            continue;
        }

        trsWrite( ATS_CON | ATS_SUM, "Calibration test # %d\n",currTest+1);
        //trsWrite( ATS_CON | ATS_SUM, "Begin read testing data...\n");

        fscanf(file,"%d %d\n",&(imageSize.width),&(imageSize.height));
        if( imageSize.width <= 0 || imageSize.height <= 0 )
        {
            trsWrite( ATS_CON | ATS_SUM, "Image size in test file is incorrect\n");
            Errors++;
        }

        /* Read etalon size */
        fscanf(file,"%d %d\n",&(etalonSize.width),&(etalonSize.height));
        if( etalonSize.width <= 0 || etalonSize.height <= 0 )
        {
            trsWrite( ATS_CON | ATS_SUM,"Etalon size in test file is incorrect\n");
            Errors++;
        }

        numPoints = etalonSize.width * etalonSize.height;

        /* Read number of images */
        fscanf(file,"%d\n",&numImages);
        if( numImages <=0 )
        {
            trsWrite( ATS_CON | ATS_SUM, "Number of images in test file is incorrect\n");
            Errors++;
        }


        /* Need to allocate memory */
        imagePoints     = (CvPoint2D64d*)trsmAlloc( numPoints *
                                                    numImages * sizeof(CvPoint2D64d));
        
        objectPoints    = (CvPoint3D64d*)trsmAlloc( numPoints *
                                                    numImages * sizeof(CvPoint3D64d));

        reprojectPoints = (CvPoint2D64d*)trsmAlloc( numPoints *
                                                    numImages * sizeof(CvPoint2D64d));

        /* Alloc memory for numbers */
        numbers = (int*)trsmAlloc( numImages * sizeof(int));

        /* Fill it by numbers of points of each image*/
        for( currImage = 0; currImage < numImages; currImage++ )
        {
            numbers[currImage] = etalonSize.width * etalonSize.height;
        }

        /* Allocate memory for translate vectors and rotmatrixs*/
        transVects     = (CvVect64d)trsmAlloc(3 * 1 * numImages * sizeof(double));
        rotMatrs       = (CvMatr64d)trsmAlloc(3 * 3 * numImages * sizeof(double));

        goodTransVects = (CvVect64d)trsmAlloc(3 * 1 * numImages * sizeof(double));
        goodRotMatrs   = (CvMatr64d)trsmAlloc(3 * 3 * numImages * sizeof(double));

        /* Read object points */
        i = 0;/* shift for current point */
        for( currImage = 0; currImage < numImages; currImage++ )
        {
            for( currPoint = 0; currPoint < numPoints; currPoint++ )
            {
                double x,y,z;
                fscanf(file,"%lf %lf %lf\n",&x,&y,&z);

                (objectPoints+i)->x = x;
                (objectPoints+i)->y = y;
                (objectPoints+i)->z = z;
                i++;
            }
        }

        /* Read image points */
        i = 0;/* shift for current point */
        for( currImage = 0; currImage < numImages; currImage++ )
        {
            for( currPoint = 0; currPoint < numPoints; currPoint++ )
            {
                double x,y;
                fscanf(file,"%lf %lf\n",&x,&y);

                (imagePoints+i)->x = x;
                (imagePoints+i)->y = y;
                i++;
            }
        }

        /* Read good data computed before */

        /* Focal lengths */
        double goodFcx,goodFcy;
        fscanf(file,"%lf %lf",&goodFcx,&goodFcy);

        /* Principal points */
        double goodCx,goodCy;
        fscanf(file,"%lf %lf",&goodCx,&goodCy);

        /* Read distortion */

        fscanf(file,"%lf",goodDistortion+0);
        fscanf(file,"%lf",goodDistortion+1);
        fscanf(file,"%lf",goodDistortion+2);
        fscanf(file,"%lf",goodDistortion+3);

        /* Read good Rot matrixes */
        for( currImage = 0; currImage < numImages; currImage++ )
        {
            for( i = 0; i < 3; i++ )
            {
                for( j = 0; j < 3; j++ )
                {
                    fscanf(file, "%lf", goodRotMatrs + currImage * 9 + j * 3 + i);
                }
            }
        }

        /* Read good Trans vectors */
        for( currImage = 0; currImage < numImages; currImage++ )
        {
            for( i = 0; i < 3; i++ )
            {
                fscanf(file, "%lf", goodTransVects + currImage * 3 + i);
            }
        }
        
        useIntrinsicGuess = 0;
        
        /* Now we can calibrate camera */
        cvCalibrateCamera_64d(  numImages,
                                numbers,
                                imageSize,
                                imagePoints,
                                objectPoints,
                                distortion,
                                cameraMatrix,
                                transVects,
                                rotMatrs,
                                useIntrinsicGuess );

        /* ---- Reproject points to the image ---- */
        for( currImage = 0; currImage < numImages; currImage++ )
        {
            int numPoints = etalonSize.width * etalonSize.height;
            cvProjectPointsSimple(  numPoints,
                                    objectPoints + currImage * numPoints,
                                    rotMatrs + currImage * 9,
                                    transVects + currImage * 3,
                                    cameraMatrix,
                                    distortion,
                                    reprojectPoints + currImage * numPoints);
        }


        /* ----- Compute reprojection error ----- */
        i = 0;
        double dx,dy;
        double rx,ry;
        double meanDx,meanDy;
        double maxDx = 0.0;
        double maxDy = 0.0;

        meanDx = 0;
        meanDy = 0;
        for( currImage = 0; currImage < numImages; currImage++ )
        {
            for( currPoint = 0; currPoint < etalonSize.width * etalonSize.height; currPoint++ )
            {
                rx = reprojectPoints[i].x;
                ry = reprojectPoints[i].y;
                dx = rx - imagePoints[i].x;
                dy = ry - imagePoints[i].y;

                meanDx += dx;
                meanDy += dy;

                dx = fabs(dx);
                dy = fabs(dy);

                if( dx > maxDx )
                    maxDx = dx;
                
                if( dy > maxDy )
                    maxDy = dy;
                i++;
            }
        }

        meanDx /= numImages * etalonSize.width * etalonSize.height;
        meanDy /= numImages * etalonSize.width * etalonSize.height;

        if( maxDx > 1.0 )
        {
            trsWrite( ATS_CON | ATS_SUM,
                      "Error in reprojection maxDx=%f > 1.0\n",maxDx);
            Errors++;
        }

        if( maxDy > 1.0 )
        {
            trsWrite( ATS_CON | ATS_SUM,
                      "Error in reprojection maxDy=%f > 1.0\n",maxDy);
            Errors++;
        }


        /* Compare max error */

        /* Compute error */
        
        
        /* ========= Compare parameters ========= */

        /* ----- Compare focal lengths ----- */
        if( atsCompDoublePrec(cameraMatrix+0,&goodFcx,1,0.01) != 0 )
        {
            printf("Error in focal length x\n");
        }

        if( atsCompDoublePrec(cameraMatrix+4,&goodFcy,1,0.01) != 0 )
        {
            printf("Error in focal length y\n");
        }            

        /* ----- Compare principal points ----- */
        if( atsCompDoublePrec(cameraMatrix+2,&goodCx,1,0.01) != 0 )
        {
            printf("Error in principal point x\n");
        }

        if( atsCompDoublePrec(cameraMatrix+5,&goodCy,1,0.01) != 0 )
        {
            printf("Error in principal point y\n");
        }            

        /* ----- Compare distortion ----- */
        if( atsCompDoublePrec(distortion,goodDistortion,4,0.001) != 0 )
        {
            printf("Error in distortion\n");
        }

        /* ----- Compare rot matrixs ----- */
        if( atsCompDoublePrec(rotMatrs,goodRotMatrs, 9*numImages,0.001) != 0 )
        {
            printf("Error in Rot Matrixes\n");
        }

        /* ----- Compare rot matrixs ----- */
        if( atsCompDoublePrec(rotMatrs,goodRotMatrs, 3*numImages,0.001) != 0 )
        {
            printf("Error in Trans Vectors\n");
        }

        fclose(file);
    }
    fclose(datafile);

test_exit:

    /* Free all allocated memory */

    trsFree(imagePoints);
    trsFree(objectPoints);
    trsFree(reprojectPoints);
    trsFree(numbers);

    trsFree(transVects);
    trsFree(rotMatrs);
    trsFree(goodTransVects);
    trsFree(goodRotMatrs);

    if( Errors == 0 )
    {
        return trsResult( TRS_OK, "No errors fixed for this text" );
    }
    else
    {
        return trsResult( TRS_FAIL, "Total fixed %d errors", Errors );
    }

}


void InitACalibration( void )
{
/* Test Registartion */
    trsRegArg(cFuncName[0],cTestName[0],cTestClass,calibrationTest, 0); 
} /* InitACalibration */


