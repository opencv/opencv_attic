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

void InitAAbsDiff();
void InitAAcc();
void InitAApproxPoly();
void InitAArithmetic();
void InitAArrayIterator();
void InitABackProject();
void InitACalibration();
void InitACamShift();
void InitACanny();
void InitAChessCorners();
void InitAConDens();
void InitAContours();
void InitAConvert();
void InitAConvexHull();
void InitACorner();
void InitACvCalculate();
void InitADerv();
void InitADistanceTransform();
void InitADrawing();
void InitADrawingRegress();
void InitAEMD();
void InitAFitEllipse();
void InitAFitLine();
void InitAFloodFill();
void InitAHaar();
void InitAHistogram();
void InitACalcContrastHist();
void InitACalcOpticalFlowLK();
void InitACalcOpticalFlowHS(); 
void InitAHistogramOperations();
void InitAHistograms();
void InitAHoughLines();
void InitAImage();
void InitAImageStatistics();
void InitAImageToHMMObs();
void InitAKalman();
void InitAKMeans();
void InitALogic();
void InitAMaskAcc();
void InitAMatchTemplate();
void InitAMathUtils();
void InitAMatr();
void InitAMatr2();
void InitAMeanShift();
void InitAMinAreaRect();
void InitAMinEVal();
void InitAMoments();
void InitAMorphology();
void InitAMotionTemplates();
void InitAMotSeg();
void InitANodeIterator();
void InitAOptFlowPyrLK();
void InitAPixelAccess();
void InitAPyramids();
void InitASamplers();
void InitASequence();
void InitASnakes();
void InitAStorageArray();
void InitASubdiv();
void InitASVD();
void InitAThreshold();
void InitAAdaptThreshold();
void InitATree();
void InitATreeIterator();
void InitAFloodFill();
void InitAFloodFill8();
void InitAUnDistort();
void InitAEigenObjects();
void InitANorm();
void InitANormMask();
void InitAContourMoments();
void InitAMatchContours();
void InitACreateContourTree();
void InitAMatchContourTrees();
void InitAPreCorner();
void InitAPyrSegmentation();
void InitAGestureRecognition();
void InitAPOSIT();

/*============================== Algorithm Tests =============================*/
int main(int argC,char *argV[])
{
    char** argv = (char**)malloc( (argC + 4)*sizeof(argv[0]));
    argv[argC + 0] = "-l";
    argv[argC + 1] = "-s";
    argv[argC + 2] = "-m";
    argv[argC + 3] = "-B";
    memcpy( argv, argV, argC*sizeof(argv[0]));

#ifdef WIN32
    atsInitModuleTestData( argV[0], "../tests/cv/testdata" );
#else
    atsInitModuleTestData( argV[0], "../testdata" );
#endif
    atsLoadPrimitives( argC, argV );

    InitAAbsDiff();
    InitAAcc();
    // InitAAdaptThreshold(); // test is not up-to-date
    InitAApproxPoly();
    InitAArithmetic();
    InitAArrayIterator();
    InitABackProject();
    InitACalcOpticalFlowLK();
    InitACalcOpticalFlowHS();
    InitACalibration();
    InitACamShift();
    InitACanny();
    InitAChessCorners();
    InitAConDens();
    InitAContours();
    InitAContourMoments();
    InitAConvert();
    InitAConvexHull();
    InitACorner();
    InitACreateContourTree();
    InitACvCalculate();
    InitADerv();
    InitADistanceTransform();
    InitADrawing();
    InitADrawingRegress();
    InitAEigenObjects();
    InitAEMD();

    InitAFitEllipse();
    InitAFitLine();

    InitAFloodFill();
    InitAFloodFill8();
    InitAHaar();
    InitAHistogram();
    //InitACalcContrastHist(); // the function is not available
    InitAHistogramOperations();
    InitAHistograms();
    InitAHoughLines();
    InitAImage();
    InitAImageStatistics();
    //InitAImageToHMMObs(); // test uses IPL DCT
    InitAKalman();
    //InitAKMeans(); // test is not up-to-date
    InitALogic();
    InitAMaskAcc();
    InitAMatchContours();
    InitAMatchContourTrees();
    InitAMatchTemplate();
    InitAMathUtils();
    InitAMatr();
    InitAMatr2();

    InitAMeanShift();
    InitAMinAreaRect();
    InitAMoments();
    InitAMorphology();
    InitAMotionTemplates();
    // InitAMotSeg(); // test is not up-to-date
    InitAMinEVal ();

    InitANodeIterator();
    InitANorm();
    InitANormMask();
    InitAOptFlowPyrLK();

    InitAPixelAccess();

    InitAPOSIT();
    InitAPyramids();
    InitASamplers();
    InitASequence();
    InitASnakes();
    InitAStorageArray();
    InitASubdiv();
    InitASVD();
    InitAThreshold();
    InitATree();
    InitATreeIterator();
    InitAUnDistort();
    InitAPreCorner();
    InitAPyrSegmentation();
    //InitAGestureRecognition(); // some functionality has been removed

    trsRun(argC + 4,argv);
    printf("Passed\n");
    free( argv );
    return 0;
}
/* End of file. */
