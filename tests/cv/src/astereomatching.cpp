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
  This is a regression test for stereo matching algorithms. This test gets some quality metrics
  discribed in "A Taxonomy and Evaluation of Dense Two-Frame Stereo Correspondence Algorithms".
  Daniel Scharstein, Richard Szeliski
*/

#include "cvtest.h"
#include <limits>
#include <cstdio>

using namespace std;
using namespace cv;

const float EVAL_BAD_THRESH = 1.f;
const int EVAL_TEXTURELESS_WIDTH = 7;
const float EVAL_TEXTURELESS_THRESH = 2.f;
const float EVAL_DISP_THRESH = 1.f;
const float EVAL_DISP_GAP = 2.f;
const int EVAL_DISCONT_WIDTH = 9;
const int EVAL_IGNORE_BORDER = 10;

const int ERROR_KINDS_COUNT = 6;

//============================== quality measuring functions =================================================

/*
  Calculate textureless regions of image (regions where the squared horizontal intensity gradient averaged over
  a square window of size=evalTexturelessWidth is below a threshold=evalTexturelessThresh) and textured regions.
*/
void computeTextureBasedMasks( const Mat& img, Mat* texturelessMask, Mat* texturedMask,
             int texturelessWidth = EVAL_TEXTURELESS_WIDTH, float texturelessThresh = EVAL_TEXTURELESS_THRESH )
{
    if( !texturelessMask && !texturedMask )
        return;
    if( img.empty() )
        CV_Error( CV_StsBadArg, "img is empty" );

    Mat dxI; Sobel( img, dxI, CV_32F, 1, 0, 3 );
    Mat dxI2; pow( dxI / 8.f/*normalize*/, 2, dxI2 );
    if( dxI2.channels() > 1)
    {
        Mat tmp; cvtColor( dxI2, tmp, CV_BGR2GRAY ); dxI2 = tmp;
    }
    Mat avgDxI2; boxFilter( dxI2, avgDxI2, CV_32FC1, Size(texturelessWidth,texturelessWidth) );

    if( texturelessMask )
        *texturelessMask = avgDxI2 < texturelessThresh;
    if( texturedMask )
        *texturedMask = avgDxI2 >= texturelessThresh;
}

void checkSizeAndTypeOfDispMaps( const Mat& leftDispMap, const Mat& rightDispMap )
{
    if( leftDispMap.empty() )
        CV_Error( CV_StsBadArg, "leftDispMap is empty" );
    if( leftDispMap.type() != CV_32FC1 )
        CV_Error( CV_StsBadArg, "leftDispMap must have CV_32FC1 type" );

    if( !rightDispMap.empty() )
    {
        if( rightDispMap.type() != CV_32FC1 )
            CV_Error( CV_StsBadArg, "rightDispMap must have CV_32FC1 type" );
        if( rightDispMap.cols != leftDispMap.cols || rightDispMap.rows != leftDispMap.rows )
            CV_Error( CV_StsBadArg, "leftDispMap and rightDispMap must have the same size" );
    }
}

void checkSizeAndTypeOfMask( const Mat& mask, Size sz )
{
    if( mask.empty() )
        CV_Error( CV_StsBadArg, "mask is empty" );
    if( mask.type() != CV_8UC1 )
        CV_Error( CV_StsBadArg, "mask must have CV_8UC1 type" );
    if( mask.rows != sz.height || mask.cols != sz.width )
        CV_Error( CV_StsBadArg, "mask has incorrect size" );
}

void checkDispMapsAndUnknDispMasks( const Mat& leftDispMap, const Mat& rightDispMap,
                                    const Mat& leftUnknDispMask, const Mat& rightUnknDispMask )
{
    checkSizeAndTypeOfDispMaps( leftDispMap, rightDispMap );

    if( !leftUnknDispMask.empty() )
        checkSizeAndTypeOfMask( leftUnknDispMask, leftDispMap.size() );
    if( !rightUnknDispMask.empty() )
        checkSizeAndTypeOfMask( rightUnknDispMask, rightDispMap.size() );

    double leftMinVal = 0, rightMinVal = 0;
    if( leftUnknDispMask.empty() )
        minMaxLoc( leftDispMap, &leftMinVal );
    else
        minMaxLoc( leftDispMap, &leftMinVal, 0, 0, 0, ~leftUnknDispMask );
    if( !rightDispMap.empty() )
    {
        if( rightUnknDispMask.empty() )
            minMaxLoc( rightDispMap, &rightMinVal );
        else
            minMaxLoc( rightDispMap, &rightMinVal, 0, 0, 0, ~rightUnknDispMask );
    }
    if( leftMinVal < 0 || rightMinVal < 0)
        CV_Error( CV_StsBadArg, "known disparity values must be positive" );
}

/*
  Calculate occluded regions of reference image (left image) (regions that are occluded in the matching image (right image),
  i.e., where the forward-mapped disparity lands at a location with a larger (nearer) disparity) and non occluded regions.
*/
void computeOcclusionBasedMasks( const Mat& leftDisp, const Mat& _rightDisp,
                             Mat* occludedMask, Mat* nonOccludedMask,
                             const Mat& leftUnknDispMask = Mat(), const Mat& rightUnknDispMask = Mat(),
                             float dispThresh = EVAL_DISP_THRESH )
{
    const float dispDiff = 1.f;
    if( !occludedMask && !nonOccludedMask )
        return;
    checkDispMapsAndUnknDispMasks( leftDisp, _rightDisp, leftUnknDispMask, rightUnknDispMask );

    Mat rightDisp;
    if( _rightDisp.empty() )
    {
        if( !rightUnknDispMask.empty() )
           CV_Error( CV_StsBadArg, "rightUnknDispMask must be empty if _rightDisp is empty" );
        rightDisp.create(leftDisp.size(), CV_32FC1);
        rightDisp.setTo(Scalar::all(0) );
        for( int leftY = 0; leftY < leftDisp.rows; leftY++ )
        {
            for( int leftX = 0; leftX < leftDisp.cols; leftX++ )
            {
                if( !leftUnknDispMask.empty() && leftUnknDispMask.at<uchar>(leftY,leftX) )
                    continue;
                float leftDispVal = leftDisp.at<float>(leftY, leftX);
                int rightX = leftX - cvRound(leftDispVal), rightY = leftY;
                if( rightX >= 0)
                    rightDisp.at<float>(rightY,rightX) = max(rightDisp.at<float>(rightY,rightX), leftDispVal);
            }
        }
    }
    else
        _rightDisp.copyTo(rightDisp);

    if( occludedMask )
    {
        occludedMask->create(leftDisp.size(), CV_8UC1);
        occludedMask->setTo(Scalar::all(0) );
    }
    if( nonOccludedMask )
    {
        nonOccludedMask->create(leftDisp.size(), CV_8UC1);
        occludedMask->setTo(Scalar::all(0) );
    }
    for( int leftY = 0; leftY < leftDisp.rows; leftY++ )
    {
        for( int leftX = 0; leftX < leftDisp.cols; leftX++ )
        {
            if( !leftUnknDispMask.empty() && leftUnknDispMask.at<uchar>(leftY,leftX) )
                continue;
            float leftDispVal = leftDisp.at<float>(leftY, leftX);
            int rightX = leftX - cvRound(leftDispVal), rightY = leftY;
            if( rightX < 0 && occludedMask )
                occludedMask->at<uchar>(leftY, leftX) = 255;
            else
            {
                if( !rightUnknDispMask.empty() && rightUnknDispMask.at<uchar>(rightY,rightX) )
                    continue;
                float rightDispVal = rightDisp.at<float>(rightY, rightX);
                if( rightDispVal > leftDispVal + dispDiff )
                {
                    if( occludedMask )
                        occludedMask->at<uchar>(leftY, leftX) = 255;
                }
                else
                {
                    if( nonOccludedMask )
                        nonOccludedMask->at<uchar>(leftY, leftX) = 255;
                }
            }
        }
    }
}

/*
  Calculate depth discontinuty regions: pixels whose neiboring disparities differ by more than
  dispGap, dilated by window of width discontWidth.
*/
void computeDepthDiscontMask( const Mat& disp, Mat& depthDiscontMask, const Mat& unknDispMask = Mat(),
                                 float dispGap = EVAL_DISP_GAP, int discontWidth = EVAL_DISCONT_WIDTH )
{
    if( disp.empty() )
        CV_Error( CV_StsBadArg, "disp is empty" );
    if( disp.type() != CV_32FC1 )
        CV_Error( CV_StsBadArg, "disp must have CV_32FC1 type" );
    if( !unknDispMask.empty() )
        checkSizeAndTypeOfMask( unknDispMask, disp.size() );

    Mat curDisp; disp.copyTo( curDisp );
    if( !unknDispMask.empty() )
        curDisp.setTo( Scalar(numeric_limits<float>::min()), unknDispMask );
    Mat maxNeighbDisp; dilate( curDisp, maxNeighbDisp, Mat(3, 3, CV_8UC1, Scalar(1)) );
    if( !unknDispMask.empty() )
        curDisp.setTo( Scalar(numeric_limits<float>::max()), unknDispMask );
    Mat minNeighbDisp; erode( curDisp, minNeighbDisp, Mat(3, 3, CV_8UC1, Scalar(1)) );
    depthDiscontMask = max( (Mat)(maxNeighbDisp-disp), (Mat)(disp-minNeighbDisp) ) > dispGap;
    if( !unknDispMask.empty() )
        depthDiscontMask &= ~unknDispMask;
    dilate( depthDiscontMask, depthDiscontMask, Mat(discontWidth, discontWidth, CV_8UC1, Scalar(1)) );
}

/*
   Get evaluation masks excluding a border.
*/
Mat getBorderedMask( Size maskSize, int border = EVAL_IGNORE_BORDER )
{
    CV_Assert( border >= 0 );
    Mat mask(maskSize, CV_8UC1, Scalar(0));
    int w = maskSize.width - 2*border, h = maskSize.height - 2*border;
    if( w < 0 ||  h < 0 )
        mask.setTo(Scalar(0));
    else
        mask( Rect(Point(border,border),Size(w,h)) ).setTo(Scalar(255));
    return mask;
}

/*
  Calculate root-mean-squared error between the computed disparity map (computedDisp) and ground truth map (groundTruthDisp).
*/
float dispRMS( const Mat& computedDisp, const Mat& groundTruthDisp, const Mat& mask )
{
    checkSizeAndTypeOfDispMaps( computedDisp, groundTruthDisp );
    int pointsCount = computedDisp.cols*computedDisp.rows;
    if( !mask.empty() )
    {
        checkSizeAndTypeOfMask( mask, computedDisp.size() );
        pointsCount = countNonZero(mask);
    }
    return 1.f/sqrt((float)pointsCount) * norm(computedDisp, groundTruthDisp, NORM_L2, mask);
}

/*
  Calculate percentage of bad matching pixels.
*/
float badMatchPxlsPercentage( const Mat& computedDisp, const Mat& groundTruthDisp, const Mat& mask,
                              int badThresh = EVAL_BAD_THRESH )
{
    checkSizeAndTypeOfDispMaps( computedDisp, groundTruthDisp );
    Mat badPxlsMap;
    absdiff( computedDisp, groundTruthDisp, badPxlsMap );
    badPxlsMap = badPxlsMap > badThresh;
    int pointsCount = computedDisp.cols*computedDisp.rows;
    if( !mask.empty() )
    {
        checkSizeAndTypeOfMask( mask, computedDisp.size() );
        badPxlsMap = badPxlsMap & mask;
        pointsCount = countNonZero(mask);
    }
    return 1.f/pointsCount * countNonZero(badPxlsMap);
}

//===================== regression test for stereo matching algorithms ==============================

const string ALGORITHMS_DIR = "stereomatching/algorithms/";
const string DATASETS_DIR = "stereomatching/datasets/";
const string DATASETS_FILE = "datasets.xml";

const string RUN_PARAMS_FILE = "_params.xml";
const string RESULT_FILE = "_res.xml";

const string LEFT_IMG_NAME = "im2.png";
const string RIGHT_IMG_NAME = "im6.png";
const string TRUE_LEFT_DISP_NAME = "disp2.png";
const string TRUE_RIGHT_DISP_NAME = "disp6.png";

string ERROR_PREFIXES[] = { "borderedAll",
                            "borderedNoOccl",
                            "borderedOccl",
                            "borderedTextured",
                            "borderedTextureless",
                            "borderedDepthDiscont" }; // size of ERROR_KINDS_COUNT


const string RMS_STR = "RMS";
const string BAD_PXLS_PERCENTAGE_STR = "BadPxlsPercentage";

class CV_StereoMatchingTest : public CvTest
{
public:
    CV_StereoMatchingTest( const char* testName ) :
            CvTest( testName, "stereo-matching" ) { rmsEps = perEps = 0.01f; }
protected:
    // assumed that left image is a reference image
    virtual void runStereoMatchingAlgorithm( const Mat& leftImg, const Mat& rightImg,
                   Mat& leftDisp, Mat& rightDisp, FileStorage& paramsFS, const string& datasetName ) = 0;

    int readDatasetsInfo();
    void readDatasetRunParams( FileStorage& fs, const string datasetName ) {}
    void writeErrors( const string& errName, const vector<float>& errors, FileStorage* fs = 0 );
    void readErrors( FileNode& fn, const string& errName, vector<float>& errors );
    int compareErrors( const vector<float>& calcErrors, const vector<float>& validErrors,
                       const vector<float>& eps, const string& errName );
    int processStereoMatchingResults( FileStorage& fs, int datasetIdx, bool isWrite,
                  const Mat& leftImg, const Mat& rightImg,
                  const Mat& trueLeftDisp, const Mat& trueRightDisp,
                  const Mat& leftDisp, const Mat& rightDisp );
    void run( int );

    vector<string> datasetsNames;
    vector<int> dispScaleFactors;
    vector<int> dispUnknownVal;

    float rmsEps;
    float perEps;
};

void CV_StereoMatchingTest::run(int)
{
    string dataPath = ts->get_data_path();
    string algoritmName = name;
    assert( !algoritmName.empty() );
    if( dataPath.empty() )
    {
        ts->printf( CvTS::LOG, "dataPath is empty" );
        ts->set_failed_test_info( CvTS::FAIL_BAD_ARG_CHECK );
        return;
    }

    int code = readDatasetsInfo();
    if( code != CvTS::OK )
    {
        ts->set_failed_test_info( code );
        return;
    }

    string fullResultFilename = dataPath + ALGORITHMS_DIR + algoritmName + RESULT_FILE;
    bool isWrite = true; // write or compare results
    FileStorage runParamsFS( dataPath + ALGORITHMS_DIR + algoritmName + RUN_PARAMS_FILE, FileStorage::READ );
    FileStorage resFS( fullResultFilename, FileStorage::READ );
    if( resFS.isOpened() )
        isWrite = false;
    else
    {
        resFS.open( fullResultFilename, FileStorage::WRITE );
        if( !resFS.isOpened() )
        {
            ts->printf( CvTS::LOG, "file named %s can not be read or written\n", fullResultFilename.c_str() );
            ts->set_failed_test_info( CvTS::FAIL_BAD_ARG_CHECK );
            return;
        }
        resFS << "stereo_matching" << "{";
    }

    int progress = 0;
    for( int dsi = 0; dsi < (int)datasetsNames.size(); dsi++)
    {
        progress = update_progress( progress, dsi, (int)datasetsNames.size(), 0 );
        string datasetFullDirName = dataPath + DATASETS_DIR + datasetsNames[dsi] + "/";
        Mat leftImg = imread(datasetFullDirName + LEFT_IMG_NAME);
        Mat rightImg = imread(datasetFullDirName + RIGHT_IMG_NAME);
        Mat trueLeftDisp = imread(datasetFullDirName + TRUE_LEFT_DISP_NAME, 0);
        Mat trueRightDisp = imread(datasetFullDirName + TRUE_RIGHT_DISP_NAME, 0);

        if( leftImg.empty() || rightImg.empty() || trueLeftDisp.empty()/* || trueRightDisp.empty()*/ )
        {
            ts->printf( CvTS::LOG, "images or left ground-truth disparities of dataset %s can not be read", datasetsNames[dsi].c_str() );
            code = CvTS::FAIL_INVALID_TEST_DATA;
            continue;
        }
        Mat tmp;
        int dispScaleFactor = dispScaleFactors[dsi];
        trueLeftDisp.convertTo( tmp, CV_32FC1, 1.f/dispScaleFactor ); trueLeftDisp = tmp; tmp.release();
        if( !trueRightDisp.empty() )
            trueRightDisp.convertTo( tmp, CV_32FC1, 1.f/dispScaleFactor ); trueRightDisp = tmp; tmp.release();

        Mat leftDisp, rightDisp;
        runStereoMatchingAlgorithm( leftImg, rightImg, leftDisp, rightDisp, runParamsFS, datasetsNames[dsi] );
        leftDisp.convertTo( tmp, CV_32FC1 ); leftDisp = tmp; tmp.release();
        rightDisp.convertTo( tmp, CV_32FC1 ); rightDisp = tmp; tmp.release();

        int tempCode = processStereoMatchingResults( resFS, dsi, isWrite,
                   leftImg, rightImg, trueLeftDisp, trueRightDisp, leftDisp, rightDisp);
        code = tempCode==CvTS::OK ? code : tempCode;
    }

    if( isWrite )
        resFS << "}"; // "stereo_matching"

    ts->set_failed_test_info( code );
}

void calcErrors( const Mat& leftImg, const Mat& rightImg,
                 const Mat& trueLeftDisp, const Mat& trueRightDisp,
                 const Mat& trueLeftUnknDispMask, const Mat& trueRightUnknDispMask,
                 const Mat& calcLeftDisp, const Mat& calcRightDisp,
                 vector<float>& rms, vector<float>& badPxlsPercentages )
{
    Mat texturelessMask, texturedMask;
    computeTextureBasedMasks( leftImg, &texturelessMask, &texturedMask );
    Mat occludedMask, nonOccludedMask;
    computeOcclusionBasedMasks( trueLeftDisp, trueRightDisp, &occludedMask, &nonOccludedMask,
                                trueLeftUnknDispMask, trueRightUnknDispMask);
    Mat depthDiscontMask;
    computeDepthDiscontMask( trueLeftDisp, depthDiscontMask, trueLeftUnknDispMask);

    Mat borderedKnownMask = getBorderedMask( leftImg.size() ) & ~trueLeftUnknDispMask;

    nonOccludedMask &= borderedKnownMask;
    occludedMask &= borderedKnownMask;
    texturedMask &= nonOccludedMask; // & borderedKnownMask
    texturelessMask &= nonOccludedMask; // & borderedKnownMask
    depthDiscontMask &= nonOccludedMask; // & borderedKnownMask

    rms.resize(ERROR_KINDS_COUNT);
    rms[0] = dispRMS( calcLeftDisp, trueLeftDisp, borderedKnownMask );
    rms[1] = dispRMS( calcLeftDisp, trueLeftDisp, nonOccludedMask );
    rms[2] = dispRMS( calcLeftDisp, trueLeftDisp, occludedMask );
    rms[3] = dispRMS( calcLeftDisp, trueLeftDisp, texturedMask );
    rms[4] = dispRMS( calcLeftDisp, trueLeftDisp, texturelessMask );
    rms[5] = dispRMS( calcLeftDisp, trueLeftDisp, depthDiscontMask );

    badPxlsPercentages.resize(ERROR_KINDS_COUNT);
    badPxlsPercentages[0] = badMatchPxlsPercentage( calcLeftDisp, trueLeftDisp, borderedKnownMask );
    badPxlsPercentages[1] = badMatchPxlsPercentage( calcLeftDisp, trueLeftDisp, nonOccludedMask );
    badPxlsPercentages[2] = badMatchPxlsPercentage( calcLeftDisp, trueLeftDisp, occludedMask );
    badPxlsPercentages[3] = badMatchPxlsPercentage( calcLeftDisp, trueLeftDisp, texturedMask );
    badPxlsPercentages[4] = badMatchPxlsPercentage( calcLeftDisp, trueLeftDisp, texturelessMask );
    badPxlsPercentages[5] = badMatchPxlsPercentage( calcLeftDisp, trueLeftDisp, depthDiscontMask );
}

int CV_StereoMatchingTest::processStereoMatchingResults( FileStorage& fs, int datasetIdx, bool isWrite,
              const Mat& leftImg, const Mat& rightImg,
              const Mat& trueLeftDisp, const Mat& trueRightDisp,
              const Mat& leftDisp, const Mat& rightDisp )
{
    // rightDisp is not used in current test virsion
    int code = CvTS::OK;
    assert( fs.isOpened() );
    assert( trueLeftDisp.type() == CV_32FC1 && trueRightDisp.type() == CV_32FC1 );
    assert( leftDisp.type() == CV_32FC1 && rightDisp.type() == CV_32FC1 );

    // get masks for unknown ground truth disparity values
    Mat leftUnknMask, rightUnknMask;
    absdiff( trueLeftDisp, Scalar(dispUnknownVal[datasetIdx]), leftUnknMask );
    leftUnknMask = leftUnknMask < numeric_limits<float>::epsilon();
    if( !trueRightDisp.empty() )
    {
        absdiff( trueRightDisp, Scalar(dispUnknownVal[datasetIdx]), rightUnknMask );
        rightUnknMask = rightUnknMask < numeric_limits<float>::epsilon();
    }

    // calculate errors
    vector<float> rmss, badPxlsPercentages;
    calcErrors( leftImg, rightImg, trueLeftDisp, trueRightDisp, leftUnknMask, rightUnknMask,
                leftDisp, rightDisp, rmss, badPxlsPercentages );

    const string& datasetName = datasetsNames[datasetIdx];
    if( isWrite )
    {
        fs << datasetName << "{";
        cvWriteComment( fs.fs, RMS_STR.c_str(), 0 );
        writeErrors( RMS_STR, rmss, &fs );
        cvWriteComment( fs.fs, BAD_PXLS_PERCENTAGE_STR.c_str(), 0 );
        writeErrors( BAD_PXLS_PERCENTAGE_STR, badPxlsPercentages, &fs );
        fs << "}"; // datasetName
    }
    else // compare
    {
        ts->printf( CvTS::LOG, "\nquality on dataset %s\n", datasetName.c_str() );
        ts->printf( CvTS::LOG, "%s\n", RMS_STR.c_str() );
        writeErrors( RMS_STR, rmss );
        ts->printf( CvTS::LOG, "%s\n", BAD_PXLS_PERCENTAGE_STR.c_str() );
        writeErrors( BAD_PXLS_PERCENTAGE_STR, badPxlsPercentages );

        FileNode fn = fs.getFirstTopLevelNode()[datasetName];
        vector<float> validRmss, validBadPxlsPercentages;

        readErrors( fn, RMS_STR, validRmss );
        readErrors( fn, BAD_PXLS_PERCENTAGE_STR, validBadPxlsPercentages );
        int tempCode = compareErrors( rmss, validRmss,
                      vector<float>(ERROR_KINDS_COUNT, rmsEps), RMS_STR );
        code = tempCode==CvTS::OK ? code : tempCode;
        tempCode = compareErrors( badPxlsPercentages, validBadPxlsPercentages,
                      vector<float>(ERROR_KINDS_COUNT, perEps), BAD_PXLS_PERCENTAGE_STR );
        code = tempCode==CvTS::OK ? code : tempCode;
    }
    return code;
}

int CV_StereoMatchingTest::readDatasetsInfo()
{
    string datasetsFilename = string(ts->get_data_path()) + DATASETS_DIR + DATASETS_FILE;

    FileStorage fs( datasetsFilename, FileStorage::READ );
    if( !fs.isOpened() )
    {
        ts->printf( CvTS::LOG, "%s can not be read\n", datasetsFilename.c_str() );
        return CvTS::FAIL_INVALID_TEST_DATA;
    }
    FileNode fn = fs.getFirstTopLevelNode()["names_scale_unknown"];
    assert(fn.isSeq());
    datasetsNames.clear();
    dispScaleFactors.clear();
    dispUnknownVal.clear();
    for( int i = 0; i < (int)fn.size(); i+=3 )
    {
        string name = fn[i]; datasetsNames.push_back(name);
        string scale = fn[i+1]; dispScaleFactors.push_back(atoi(scale.c_str()));
        string unkn = fn[i+2]; dispUnknownVal.push_back(atoi(unkn.c_str()));
    }

    return CvTS::OK;
}

void CV_StereoMatchingTest::writeErrors( const string& errName, const vector<float>& errors, FileStorage* fs )
{
    assert( (int)errors.size() == ERROR_KINDS_COUNT );
    vector<float>::const_iterator it = errors.begin();
    if( fs )
        for( int i = 0; i < ERROR_KINDS_COUNT; i++, ++it )
            *fs << ERROR_PREFIXES[i] + errName << *it;
    else
        for( int i = 0; i < ERROR_KINDS_COUNT; i++, ++it )
            ts->printf( CvTS::LOG, "%s = %f\n", string(ERROR_PREFIXES[i]+errName).c_str(), *it );
}

void CV_StereoMatchingTest::readErrors( FileNode& fn, const string& errName, vector<float>& errors )
{
    errors.resize( ERROR_KINDS_COUNT );
    vector<float>::iterator it = errors.begin();
    for( int i = 0; i < ERROR_KINDS_COUNT; i++, ++it )
        fn[ERROR_PREFIXES[i]+errName] >> *it;
}

int CV_StereoMatchingTest::compareErrors( const vector<float>& calcErrors, const vector<float>& validErrors,
                   const vector<float>& eps, const string& errName )
{
    assert( (int)calcErrors.size() == ERROR_KINDS_COUNT );
    assert( (int)validErrors.size() == ERROR_KINDS_COUNT );
    assert( (int)eps.size() == ERROR_KINDS_COUNT );
    vector<float>::const_iterator calcIt = calcErrors.begin(),
                                  validIt = validErrors.begin(),
                                  epsIt = eps.begin();
    for( int i = 0; i < ERROR_KINDS_COUNT; i++, ++calcIt, ++validIt, ++epsIt )
        if( fabs(*calcIt - *validIt) > *epsIt )
        {
            ts->printf( CvTS::LOG, "bad accuracy of %s\n", string(ERROR_PREFIXES[i]+errName).c_str());
            return CvTS::FAIL_BAD_ACCURACY;
        }
    return CvTS::OK;
}

//----------------------------------- StereoBM test -----------------------------------------------------

class CV_StereoBMTest : public CV_StereoMatchingTest
{
public:
    CV_StereoBMTest() :
            CV_StereoMatchingTest( "stereobm" ) {}
protected:
    virtual void runStereoMatchingAlgorithm( const Mat& _leftImg, const Mat& _rightImg,
                   Mat& leftDisp, Mat& rightDisp, FileStorage& paramsFS, const string& datasetName )
    {
        int ndisp = 7;
        int winSize = 21;
        assert( !datasetName.empty() );
        if( paramsFS.isOpened() )
        {
            FileNodeIterator fni = paramsFS.getFirstTopLevelNode()[datasetName].begin();
            fni >> ndisp >> winSize;
        }
        else
            ts->printf( CvTS::LOG, "%s was tested with default params "
                        "(ndisp = 7, winSize = 21)\n", datasetName.c_str());

        assert( _leftImg.type() == CV_8UC3 && _rightImg.type() == CV_8UC3 );
        Mat leftImg; cvtColor( _leftImg, leftImg, CV_BGR2GRAY );
        Mat rightImg; cvtColor( _rightImg, rightImg, CV_BGR2GRAY );

        StereoBM bm( StereoBM::BASIC_PRESET, ndisp*16 );
        bm( leftImg, rightImg, leftDisp, CV_32F );
    }
};

CV_StereoBMTest stereoBM;

//----------------------------------- StereoGC test -----------------------------------------------------

class CV_StereoGCTest : public CV_StereoMatchingTest
{
public:
    CV_StereoGCTest() :
            CV_StereoMatchingTest( "stereogc" ) { rmsEps = 1.f; perEps = 0.08f; }
protected:
    virtual void runStereoMatchingAlgorithm( const Mat& _leftImg, const Mat& _rightImg,
                   Mat& leftDisp, Mat& rightDisp, FileStorage& paramsFS, const string& datasetName )
    {
        int ndisp = 20;
        int icount = 2;
        assert( !datasetName.empty() );
        if( paramsFS.isOpened() )
        {
            FileNodeIterator fni = paramsFS.getFirstTopLevelNode()[datasetName].begin();
            fni >> ndisp >> icount;
        }
        else
            ts->printf( CvTS::LOG, "%s was tested with default params "
                        "(ndisp = 20, icount = 2)\n", datasetName.c_str());

        assert( _leftImg.type() == CV_8UC3 && _rightImg.type() == CV_8UC3 );
        Mat leftImg, rightImg, tmp;
        cvtColor( _leftImg, leftImg, CV_BGR2GRAY );
        cvtColor( _rightImg, rightImg, CV_BGR2GRAY );

        leftDisp.create( leftImg.size(), CV_16SC1 );
        rightDisp.create( rightImg.size(), CV_16SC1 );

        CvMat _limg = leftImg, _rimg = rightImg, _ldisp = leftDisp, _rdisp = rightDisp;
        CvStereoGCState *state = cvCreateStereoGCState( ndisp, icount );
        cvFindStereoCorrespondenceGC( &_limg, &_rimg, &_ldisp, &_rdisp, state );
        cvReleaseStereoGCState( &state );

        leftDisp = - leftDisp;
    }

};

CV_StereoGCTest stereoGC;
