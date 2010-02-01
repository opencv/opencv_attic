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

using namespace std;
using namespace cv;

const float EVAL_BAD_THRESH = 1.f;
const int EVAL_TEXTURELESS_WIDTH = 9;
const float EVAL_TEXTURELESS_THRESH = 2.f;
const float EVAL_DISP_THRESH = 1.f;
const float EVAL_DISP_GAP = 2.f;
const int EVAL_DISCONT_WIDTH = 9;
const int EVAL_IGNORE_BORDER = 10;

const int ERROR_KINDS_COUNT = 6;

//============================== quality measuring functions =================================================

/*
  Calculate textureless regions of image: regions where the squared horizontal intensity gradient averaged over
  a square window of size=evalTexturelessWidth is below a threshold=evalTexturelessThresh.
*/
void computeTexturelessRegions( const Mat& img,  Mat& texturelessMask,
             int texturelessWidth = EVAL_TEXTURELESS_WIDTH, float texturelessThresh = EVAL_TEXTURELESS_THRESH )
{
    if( img.empty() )
        CV_Error( CV_StsBadArg, "img is empty" );
    if( img.type() != CV_8UC3 )
        CV_Error( CV_StsBadArg, "img.type() must be CV_8UC1" );
    Mat dxI; Sobel( img, dxI, CV_32F, 1, 0, 3 );
    Mat dxI2; pow( dxI / 8.f, 2, dxI2 );
    Mat tmp; cvtColor(dxI2, tmp, CV_BGR2GRAY); dxI2 = tmp;
    Mat avgDxI2; boxFilter( dxI2, avgDxI2, CV_32FC1, Size(texturelessWidth,texturelessWidth) );
    texturelessMask = avgDxI2 < texturelessThresh;
}

void checkDispMaps( const Mat& disp1, const Mat& disp2 )
{
    if( disp1.empty() || disp2.empty() )
        CV_Error( CV_StsBadArg, "disp1 or disp2 is empty" );
    if( disp1.depth() != CV_8U || disp2.depth() != CV_8U )
        CV_Error( CV_StsBadArg, "disp1.depth() and disp2.depth() must be CV_8U" );
    Size sz = disp1.size();
    if( disp1.cols != sz.width || disp2.rows != sz.height )
        CV_Error( CV_StsBadArg, "disp1 and disp2 must have the same size" );
}

/*
  Calculate occluded regions of reference image (left image): regions that are occluded in the matching image (right image),
  i.e., where the forward-mapped disparity lands at a location with a larger (nearer) disparity.
*/
void computeOccludedRegions( const Mat& leftDisp, const Mat& rightDisp, Mat& occludedMask,
                             float dispThresh = EVAL_DISP_THRESH )
{
    checkDispMaps(leftDisp, rightDisp);
    Mat grayLeftDisp; cvtColor(leftDisp, grayLeftDisp, CV_BGR2GRAY);
    Mat grayRightDisp; cvtColor(rightDisp, grayRightDisp, CV_BGR2GRAY);

    occludedMask.create(leftDisp.size(), CV_8UC1); occludedMask.setTo(Scalar::all(255));
    for( int leftY = 0; leftY < leftDisp.rows; leftY++ )
        for( int leftX = 0; leftX < leftDisp.cols; leftX++ )
        {
            int leftDispVal = grayLeftDisp.at<uchar>(leftY, leftX);
            int rightX = leftX - leftDispVal, rightY = leftY;
            if( rightX < 0 )
                occludedMask.at<uchar>(leftY, leftX) = 0;
            else
            {
                int rightDispVal = grayRightDisp.at<uchar>(rightY, rightX);
                if( abs(rightDispVal - leftDispVal) > dispThresh )
                    occludedMask.at<uchar>(leftY, leftX) = 0;
            }
        }
}

/*
  Calculate depth discontinuty regions: pixels whose neiboring disparities differ by more than
  dispGap, dilated by window of width discontWidth.
*/
void computeDepthDiscontRegions( const Mat& disp, Mat& depthDiscontMask,
                                 float dispGap = EVAL_DISP_GAP, int discontWidth = EVAL_DISCONT_WIDTH )
{
    if( disp.empty() )
        CV_Error( CV_StsBadArg, "disp is empty" );
    if( disp.depth() != CV_8U )
        CV_Error( CV_StsBadArg, "disp.depth() must be CV_8U" );
    Mat grayDisp; cvtColor(disp, grayDisp, CV_BGR2GRAY);
    Mat maxNeighbDisp; dilate(grayDisp, maxNeighbDisp, Mat(3, 3, CV_8UC1, Scalar(1)));
    Mat minNeighbDisp; erode(grayDisp, minNeighbDisp, Mat(3, 3, CV_8UC1, Scalar(1)));
    depthDiscontMask = max((Mat)(maxNeighbDisp-grayDisp), (Mat)(grayDisp-minNeighbDisp)) > dispGap;
    dilate(depthDiscontMask, depthDiscontMask, Mat(discontWidth, discontWidth, CV_8UC1, Scalar(1)));
}

/*
  Following functions are for getting evaluation masks excluding a border.
*/
Mat borderedAllMask( Size maskSize, int border = EVAL_IGNORE_BORDER )
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

void checkMask( const Mat& mask )
{
    if( mask.empty() )
        CV_Error( CV_StsBadArg, "mask is empty" );
    if( mask.type() != CV_8UC1 )
        CV_Error( CV_StsBadArg, "mask must have CV_8UC1 type" );
}

void checkMasks( const Mat& mask1, const Mat& mask2 )
{
    checkMask(mask1);
    checkMask(mask2);
    if( mask1.cols != mask2.cols || mask1.rows != mask2.rows )
        CV_Error( CV_StsBadArg, "mask1 and mask2 must have the same size" );
}

Mat borderedNoOcclMask( const Mat& occludedMask, int border = EVAL_IGNORE_BORDER )
{
    checkMask(occludedMask);
    return ~occludedMask & borderedAllMask(occludedMask.size(), border);
}

Mat borderedOcclMask( const Mat& occludedMask, int border = EVAL_IGNORE_BORDER )
{
    checkMask(occludedMask);
    return occludedMask & borderedAllMask(occludedMask.size(), border);
}

Mat borderedTexturedMask( const Mat& texturelessMask, const Mat& occludedMask, int border = EVAL_IGNORE_BORDER )
{
    checkMasks(texturelessMask, occludedMask);
    return ~texturelessMask & borderedAllMask(occludedMask.size(), border);
}

Mat borderedTexturelessMask( const Mat& texturelessMask, const Mat& occludedMask, int border = EVAL_IGNORE_BORDER )
{
    checkMasks(texturelessMask, occludedMask);
    return texturelessMask & borderedAllMask(occludedMask.size(), border);
}

Mat borderedDepthDiscontMask( const Mat& depthDiscontMask, const Mat& occludedMask, int border = EVAL_IGNORE_BORDER )
{
    checkMasks(depthDiscontMask, occludedMask);
    return depthDiscontMask & borderedAllMask(occludedMask.size(), border);
}

/*
  Calculate root-mean-squared error between the computed disparity map (computedDisp) and ground truth map (groundTruthDisp).
*/
float dispRMS( const Mat& computedDisp, const Mat& groundTruthDisp, const Mat& mask )
{
    checkDispMaps(computedDisp, groundTruthDisp);
    Mat grayComputedDisp; cvtColor(computedDisp, grayComputedDisp, CV_BGR2GRAY);
    Mat grayGroundTruthDisp; cvtColor(groundTruthDisp, grayGroundTruthDisp, CV_BGR2GRAY);

    int pointsCount = mask.empty() ? computedDisp.cols*computedDisp.rows : countNonZero(mask);
    return 1.f/sqrt((float)pointsCount) * norm(grayComputedDisp, grayGroundTruthDisp, NORM_L2, mask);
}

/*
  Calculate percentage of bad matching pixels.
*/
float badMatchPxlsPercentage( const Mat& computedDisp, const Mat& groundTruthDisp, const Mat& mask,
                              int badThresh = EVAL_BAD_THRESH )
{
    checkDispMaps(computedDisp, groundTruthDisp);
    Mat grayComputedDisp; cvtColor(computedDisp, grayComputedDisp, CV_BGR2GRAY);
    Mat grayGroundTruthDisp; cvtColor(groundTruthDisp, grayGroundTruthDisp, CV_BGR2GRAY);

    Mat badPxlsMap;
    absdiff(computedDisp, groundTruthDisp, badPxlsMap);
    badPxlsMap = badPxlsMap > badThresh;
    int pointsCount = computedDisp.cols*computedDisp.rows;
    if( !mask.empty() )
    {
        badPxlsMap = badPxlsMap & mask;
        pointsCount = countNonZero(mask);
    }
    return 1.f/pointsCount * countNonZero(badPxlsMap);
}

/*
  Calculate root-mean-squared errors for six kinds regions:
  bordered region, bordered non occluded region, bordered occluded region, bordered textured region,
  bordered textureless region, bordered depth discontinuty region.
*/
void calcRMSs( const Mat& computedDisp, const Mat& groundTruthDisp,
               const Mat& texturelessMask, const Mat& occludedMask, const Mat& depthDiscontMask,
               vector<float>& errors )
{
    errors.resize(ERROR_KINDS_COUNT);
    errors[0] = dispRMS( computedDisp, groundTruthDisp, borderedAllMask(computedDisp.size()) );
    errors[1] = dispRMS( computedDisp, groundTruthDisp, borderedNoOcclMask(occludedMask) );
    errors[2] = dispRMS( computedDisp, groundTruthDisp, borderedOcclMask(occludedMask) ),
    errors[3] = dispRMS( computedDisp, groundTruthDisp, borderedTexturedMask(texturelessMask, occludedMask) ),
    errors[4] = dispRMS( computedDisp, groundTruthDisp, borderedTexturelessMask(texturelessMask, occludedMask) ),
    errors[5] = dispRMS( computedDisp, groundTruthDisp, borderedDepthDiscontMask(depthDiscontMask, occludedMask) );
}

/*
  Calculate percentages of bad matching pixels for six kinds regions:
  bordered region, bordered non occluded region, bordered occluded region, bordered textured region,
  bordered textureless region, bordered depth discontinuty region.
*/
void calcBadMatchPxlsPercentages( const Mat& computedDisp, const Mat& groundTruthDisp,
                const Mat& texturelessMask, const Mat& occludedMask, const Mat& depthDiscontMask,
                vector<float>& errors, int badThresh = EVAL_BAD_THRESH )
{
    errors.resize(ERROR_KINDS_COUNT);
    errors[0] = badMatchPxlsPercentage( computedDisp, groundTruthDisp, borderedAllMask(computedDisp.size()), badThresh ),
    errors[1] = badMatchPxlsPercentage( computedDisp, groundTruthDisp, borderedNoOcclMask(occludedMask), badThresh ),
    errors[2] = badMatchPxlsPercentage( computedDisp, groundTruthDisp, borderedOcclMask(occludedMask), badThresh ),
    errors[3] = badMatchPxlsPercentage( computedDisp, groundTruthDisp, borderedTexturedMask(texturelessMask, occludedMask), badThresh ),
    errors[4] = badMatchPxlsPercentage( computedDisp, groundTruthDisp, borderedTexturelessMask(texturelessMask, occludedMask), badThresh ),
    errors[5] = badMatchPxlsPercentage( computedDisp, groundTruthDisp, borderedDepthDiscontMask(depthDiscontMask, occludedMask), badThresh );
}

//===================== regression test for stereo matching algorithms ==============================

const string RESULT_DIR = "test_results";
const string LEFT_IMG_NAME = "im2.ppm";
const string RIGHT_IMG_NAME = "im6.ppm";
const string TRUE_LEFT_DISP_NAME = "disp2.pgm";
const string TRUE_RIGHT_DISP_NAME = "disp6.pgm";

string DATASETS_NAMES[] = { "barn2", "bull", "cones", "poster", "sawtooth", "teddy", "venus" };
string ERROR_PREFIXES[] = { "borderedAll",
                            "borderedNoOccl",
                            "borderedOccl",
                            "borderedTextured",
                            "borderedTextureless",
                            "borderedDepthDiscont" }; // size of ERROR_KINDS_COUNT

const string RMS_STR = "RMS";
const string BAD_PXLS_PERCENTAGE_STR = "badPxlsPercentage";

class CV_StereoMatchingTest : public CvTest
{
public:
    CV_StereoMatchingTest(const char* testName, const char* testFuncs) :
            CvTest( testName, testFuncs ) {}
protected:
    // assumed that left image is a reference image
    virtual void runStereoMatchingFunc( const Mat& leftImg, const Mat& rigthImg,
                                        Mat& leftDisp, Mat& rightDisp ) = 0;

    void writeErrors( FileStorage fs, const string& errName, const vector<float>& errors );
    void readErrors( FileNode fn, const string& errName, vector<float>& errors );
    int compareErrors( const vector<float>& calcErrors, const vector<float>& validErrors,
                       const vector<float>& eps, const string& errName, const string& datasetName );
    int processStereoMatchingResults( FileStorage& fs, const string& datasetName, bool isWrite,
                  const Mat& leftImg, const Mat& rightImg,
                  const Mat& trueLeftDisp, const Mat& trueRightDisp,
                  const Mat& leftDisp, const Mat& rightDisp );
    void run( int );

    string resultFilename;
};

void CV_StereoMatchingTest::writeErrors( FileStorage fs, const string& errName, const vector<float>& errors )
{
    assert( (int)errors.size() == ERROR_KINDS_COUNT );
    vector<float>::const_iterator it = errors.begin();
    for( int i = 0; i < ERROR_KINDS_COUNT; i++, ++it )
        fs << ERROR_PREFIXES[i] + errName << *it;
}

void CV_StereoMatchingTest::readErrors( FileNode fn, const string& errName, vector<float>& errors )
{
    errors.resize( ERROR_KINDS_COUNT );
    vector<float>::iterator it = errors.begin();
    for( int i = 0; i < ERROR_KINDS_COUNT; i++, ++it )
        fn[ERROR_PREFIXES[i]+errName] >> *it;
}

int CV_StereoMatchingTest::compareErrors( const vector<float>& calcErrors, const vector<float>& validErrors,
                   const vector<float>& eps, const string& errName, const string& datasetName )
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
            ts->printf( CvTS::LOG, "bad accuracy of %s on dataset %s",
                        string(ERROR_PREFIXES[i]+errName).c_str(), datasetName.c_str() );
            return CvTS::FAIL_BAD_ACCURACY;
        }
    return CvTS::OK;
}

int CV_StereoMatchingTest::processStereoMatchingResults( FileStorage& fs, const string& datasetName, bool isWrite,
              const Mat& leftImg, const Mat& rightImg,
              const Mat& trueLeftDisp, const Mat& trueRightDisp,
              const Mat& leftDisp, const Mat& rightDisp )
{
    // rightDisp is not used in current test virsion
    int code = CvTS::OK;
    assert( fs.isOpened() );
    assert( !datasetName.empty() );

    // commpute three kinds masks
    Mat texturelessMask; computeTexturelessRegions( leftImg, texturelessMask );
    Mat occludedMask; computeOccludedRegions( trueLeftDisp, trueRightDisp, occludedMask );
    Mat depthDiscontMask; computeDepthDiscontRegions( trueLeftDisp, depthDiscontMask );
    assert( !texturelessMask.empty() && !occludedMask.empty() && !depthDiscontMask.empty() );

    // compute errors
    vector<float> rmss, badPxlsPercentages;
    calcRMSs( leftDisp, trueLeftDisp, texturelessMask, occludedMask, depthDiscontMask, rmss );
    calcBadMatchPxlsPercentages( leftDisp, trueLeftDisp,
              texturelessMask, occludedMask, depthDiscontMask, badPxlsPercentages );

    if( isWrite )
    {
        fs << datasetName << "{";
        writeErrors( fs, RMS_STR, rmss );
        writeErrors( fs, BAD_PXLS_PERCENTAGE_STR, badPxlsPercentages );
        fs << "}"; // datasetName
    }
    else // compare
    {
        FileNode fn = fs.getFirstTopLevelNode()[datasetName];
        vector<float> validRmss, validBadPxlsPercentages;

        readErrors( fn, RMS_STR, validRmss );
        int tempCode = compareErrors( rmss, validRmss,
                      vector<float>(ERROR_KINDS_COUNT, 0.01f), RMS_STR, datasetName );
        code = tempCode==CvTS::OK ? code : tempCode;

        readErrors( fn, BAD_PXLS_PERCENTAGE_STR, validBadPxlsPercentages );
        tempCode = compareErrors( badPxlsPercentages, validBadPxlsPercentages,
                      vector<float>(ERROR_KINDS_COUNT, 0.01f), BAD_PXLS_PERCENTAGE_STR, datasetName );
        code = tempCode==CvTS::OK ? code : tempCode;
    }
    return code;
}

void CV_StereoMatchingTest::run(int)
{
    int code = CvTS::OK;
    if( resultFilename.empty() )
    {
        ts->printf( CvTS::LOG, "resultFilename is empty" );
        ts->set_failed_test_info( CvTS::FAIL_BAD_ARG_CHECK );
        return;
    }
             
    string fullResultFilename = string(ts->get_data_path()) + "stereomatching/" + RESULT_DIR + "/" + resultFilename;
    bool isWrite = true; // write or compare results
    FileStorage fs( fullResultFilename, FileStorage::READ );
    if( fs.isOpened() )
        isWrite = false;
    else
    {
        fs.open( fullResultFilename, FileStorage::WRITE );
        if( !fs.isOpened() )
        {
            ts->printf( CvTS::LOG, "file named %s can not be read or written", fullResultFilename.c_str() );
            code = CvTS::FAIL_BAD_ARG_CHECK;
            return;
        }
        fs << "stereo_matching" << "{";
    }
    
    int datasetsCount = sizeof(DATASETS_NAMES)/sizeof(DATASETS_NAMES[0]);
    for( int dsi = 0; dsi < datasetsCount; dsi++)
    {
        string dataPath = string(ts->get_data_path()) + "stereomatching/datasets/" + DATASETS_NAMES[dsi] + "/";
        Mat leftImg = imread(dataPath + LEFT_IMG_NAME);
        Mat rightImg = imread(dataPath + RIGHT_IMG_NAME);
        Mat trueLeftDisp = imread(dataPath + TRUE_LEFT_DISP_NAME);
        Mat trueRightDisp = imread(dataPath + TRUE_RIGHT_DISP_NAME);
        if( leftImg.empty() || rightImg.empty() || trueLeftDisp.empty() || trueRightDisp.empty() )
        {
            ts->printf( CvTS::LOG, "images or true disparities of dataset %s can not be read", DATASETS_NAMES[dsi].c_str() );
            code = CvTS::FAIL_INVALID_TEST_DATA;
            continue;
        }

        Mat leftDisp, rightDisp;
        runStereoMatchingFunc( leftImg, rightImg, leftDisp, rightDisp );

        int tempCode = processStereoMatchingResults( fs, DATASETS_NAMES[dsi], isWrite,
                   leftImg, rightImg, trueLeftDisp, trueRightDisp, leftDisp, rightDisp);
        code = tempCode==CvTS::OK ? code : tempCode;
    }    

    if( isWrite )
        fs << "}"; // "stereo_matching"

    ts->set_failed_test_info( code );
}

//CV_StereoMatchingTest stereoMatchingTest;
