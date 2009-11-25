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
using namespace cv;
using namespace std;

//#define GET_STAT

#define DIST_E              "distE"
#define S_E                 "sE"
#define NO_PAIR_E           "noPairE"
#define TOTAL_NO_PAIR_E     "totalNoPairE"

#define DETECTOR_NAMES      "detector_names"
#define DETECTOR_FILENAMES  "detector_filenames"
#define IMAGE_FILENAMES     "image_filenames"
#define VALIDATION          "validation"

class CV_DetectorTest : public CvTest
{
public:
    CV_DetectorTest( const char* test_name );
    virtual int init( CvTS* system );
protected:
    virtual int prepareData( FileStorage& fs );
    virtual void run( int startFrom );
    virtual string& getValidationFilename();

    int runTestCase( int detectorIdx, vector<vector<Rect> >& objects );
    virtual void detectMultiScale( const string& detectorPath, const Mat& img, vector<Rect>& objects ) = 0;
    int validate( int detectorIdx, vector<vector<Rect> >& objects );

    struct
    {
        float dist;
        float s;
        float noPair;
        float totalNoPair;
    } eps;
    vector<string> detectorNames;
    vector<string> detectorFilenames;
    vector<string> imageFilenames;
    vector<Mat> images;
    string validationFilename;
    FileStorage validationFS;
};

CV_DetectorTest::CV_DetectorTest( const char* test_name ) : CvTest( test_name, "detectMultiScale" )
{
}

int CV_DetectorTest::init(CvTS *system)
{
    clear();
    ts = system;
    string dataPath = ts->get_data_path();
    validationFS.open( dataPath + getValidationFilename(), FileStorage::READ );
    return prepareData( validationFS );
}

string& CV_DetectorTest::getValidationFilename()
{
    return validationFilename;
}

int CV_DetectorTest::prepareData( FileStorage& _fs )
{
    if( !_fs.isOpened() )
        test_case_count = -1;
    else
    {
        FileNode fn = _fs.getFirstTopLevelNode();

        fn[DIST_E] >> eps.dist;
        fn[S_E] >> eps.s;
        fn[NO_PAIR_E] >> eps.noPair;
        fn[TOTAL_NO_PAIR_E] >> eps.totalNoPair;

        // read detectors names and filenames
        if( fn[DETECTOR_NAMES].node->data.seq != 0 )
        {
            FileNodeIterator it = fn[DETECTOR_NAMES].begin();
            for( ; it != fn[DETECTOR_NAMES].end(); )
            {
                string name, filename;
                it >> name; 
                detectorNames.push_back(name);
                if( fn[DETECTOR_FILENAMES][name].node->data.str.ptr != 0 )
                    fn[DETECTOR_FILENAMES][name] >> filename;
                detectorFilenames.push_back(filename);
            }
        }
        test_case_count = (int)detectorNames.size();

        // read images filenames and images
        string dataPath = ts->get_data_path();
        if( fn[IMAGE_FILENAMES].node->data.seq != 0 )
        {
            for( FileNodeIterator it = fn[IMAGE_FILENAMES].begin(); it != fn[IMAGE_FILENAMES].end(); )
            {
                string filename;
                it >> filename;
                imageFilenames.push_back(filename);
                Mat img = imread( dataPath+filename, 1 );
                images.push_back( img );
            }
        }
    }
    return CvTS::OK;
}

void CV_DetectorTest::run( int start_from )
{
    int code = CvTS::OK;
    start_from = 0;

#ifdef GET_STAT
    validationFS.release();
    string filename = ts->get_data_path();
    filename += getValidationFilename();
    validationFS.open( filename, FileStorage::WRITE );
    validationFS << FileStorage::getDefaultObjectName(validationFilename) << "{";

    validationFS << DIST_E << eps.dist;
    validationFS << S_E << eps.s;
    validationFS << NO_PAIR_E << eps.noPair;
    validationFS << TOTAL_NO_PAIR_E << eps.totalNoPair;

    // write detector names
    validationFS << DETECTOR_NAMES << "[";
    vector<string>::const_iterator nit = detectorNames.begin();
    for( ; nit != detectorNames.end(); ++nit )
    {
        validationFS << *nit;
    }
    validationFS << "]"; // DETECTOR_NAMES

    // write detector filenames
    validationFS << DETECTOR_FILENAMES << "{";
    assert( detectorNames.size() == detectorFilenames.size() );
    nit = detectorNames.begin();
    vector<string>::const_iterator fnit = detectorFilenames.begin();
    for( ; fnit != detectorFilenames.end(); ++nit, ++fnit )
    {
        validationFS << *nit << *fnit;
    }
    validationFS << "}"; // DETECTOR_FILENAMES

    // write image filenames
    validationFS << IMAGE_FILENAMES << "[";
    vector<string>::const_iterator it = imageFilenames.begin();
    for( int ii = 0; it != imageFilenames.end(); ++it, ii++ )
    {
        char buf[10];
        sprintf( buf, "%s%d", "img_", ii );
        cvWriteComment( validationFS.fs, buf, 0 );
        validationFS << *it;
    }
    validationFS << "]"; // IMAGE_FILENAMES

    validationFS << VALIDATION << "{";
#endif

    for( int di = 0; di < test_case_count; di++ )
    {
#ifdef GET_STAT
        validationFS << detectorNames[di] << "{";
#endif
        vector<vector<Rect> > objects;
        int temp_code = runTestCase( di, objects );
#ifndef GET_STAT
        if (temp_code == CvTS::OK)
            temp_code = validate( di, objects );
#endif
        if (temp_code != CvTS::OK)
            code = temp_code;
#ifdef GET_STAT
        validationFS << "}"; // detectorNames[di]
#endif
    }

#ifdef GET_STAT
    validationFS << "}"; // VALIDATION
    validationFS << "}"; // getDefaultObjectName
#endif
    if ( test_case_count <= 0 || imageFilenames.size() <= 0 )
    {
        ts->printf( CvTS::LOG, "validation file is not determined or not correct" );
        code = CvTS::FAIL_INVALID_TEST_DATA;
    }
    ts->set_failed_test_info( code );
}

int CV_DetectorTest::runTestCase( int detectorIdx, vector<vector<Rect> >& objects )
{
    string dataPath = ts->get_data_path(), detectorFilename;
    if( !detectorFilenames[detectorIdx].empty() )
        detectorFilename = dataPath + detectorFilenames[detectorIdx];

    for( int ii = 0; ii < (int)imageFilenames.size(); ++ii )
    {
        vector<Rect> imgObjects;
        Mat image = images[ii];
        if( image.empty() )
        {
            char msg[30];
            sprintf( msg, "%s %d %s", "image ", ii, " can not be read" );
            ts->printf( CvTS::LOG, msg );
            return CvTS::FAIL_INVALID_TEST_DATA;
        }
        detectMultiScale( detectorFilename, image, imgObjects );
        objects.push_back( imgObjects );

#ifdef GET_STAT
        char buf[10];
        sprintf( buf, "%s%d", "img_", ii );
        string imageIdxStr = buf;
        validationFS << imageIdxStr << "[:";
        for( vector<Rect>::const_iterator it = imgObjects.begin();
                it != imgObjects.end(); ++it )
        {
            validationFS << it->x << it->y << it->width << it->height;
        }
        validationFS << "]"; // imageIdxStr
#endif
    }
    return CvTS::OK;
}

int CV_DetectorTest::validate( int detectorIdx, vector<vector<Rect> >& objects )
{
    assert( imageFilenames.size() == objects.size() );
    int imageIdx = 0;
    int totalNoPair = 0, totalValRectCount = 0;

    for( vector<vector<Rect> >::const_iterator it = objects.begin();
        it != objects.end(); ++it, imageIdx++ ) // for image
    {
        Size imgSize = images[imageIdx].size();
        float dist = min(imgSize.height, imgSize.width) * eps.dist;
        float wDiff = imgSize.width * eps.s;
        float hDiff = imgSize.height * eps.s;

        int noPair = 0;

        // read validation rectangles
        char buf[10];
        sprintf( buf, "%s%d", "img_", imageIdx );
        string imageIdxStr = buf;
        FileNode node = validationFS.getFirstTopLevelNode()[VALIDATION][detectorNames[detectorIdx]][imageIdxStr];
        vector<Rect> valRects;
        if( node.node->data.seq != 0 )
        {
            for( FileNodeIterator it = node.begin(); it != node.end(); )
            {
                Rect r;
                it >> r.x >> r.y >> r.width >> r.height;
                valRects.push_back(r);
            }
        }
        totalValRectCount += (int)valRects.size();
                
        // compare rectangles
        for( vector<Rect>::const_iterator cr = it->begin();
            cr != it->end(); ++cr )
        {
            // find nearest rectangle
            Point2f cp1 = Point2f( cr->x + (float)cr->width/2.0f, cr->y + (float)cr->height/2.0f );
            int minIdx = -1, vi = 0;
            float minDist = (float)norm( Point(imgSize.width, imgSize.height) );
            for( vector<Rect>::const_iterator vr = valRects.begin();
                vr != valRects.end(); ++vr, vi++ )
            {
                Point2f cp2 = Point2f( vr->x + (float)vr->width/2.0f, vr->y + (float)vr->height/2.0f );
                float curDist = (float)norm(cp1-cp2);
                if( curDist < minDist )
                {
                    minIdx = vi;
                    minDist = curDist;
                }
            }
            if( minIdx == -1 )
            {
                noPair++;
            }
            else
            {
                Rect vr = valRects[minIdx];
                if( (minDist > dist) || (abs(cr->width - vr.width) > wDiff) ||
                    (abs(cr->height - vr.height) > hDiff) )
                {
                    noPair++;
                }
            }
        }
        noPair = (int)valRects.size() - (int)it->size() + 2*noPair;
        totalNoPair += noPair;
        if( noPair > valRects.size()*eps.noPair+1 )
            break;
    }
    if( imageIdx < (int)imageFilenames.size() )
    {
        char msg[100];
        sprintf( msg, "%s %d%s", "overrated count of rectangles without pair on ", imageIdx, "-image" );
        ts->printf( CvTS::LOG, msg );
        return CvTS::FAIL_BAD_ACCURACY;
    }
    if ( totalNoPair > totalValRectCount*eps.totalNoPair+1 )
    {
        ts->printf( CvTS::LOG, "overrated count of rectangles without pair on all images set" );
        return CvTS::FAIL_BAD_ACCURACY;
    }

    return CvTS::OK;
}

//----------------------------------------------- CascadeDetectorTest -----------------------------------
class CV_CascadeDetectorTest : public CV_DetectorTest
{
public:
    CV_CascadeDetectorTest( const char* test_name );
protected:
    virtual void detectMultiScale( const string& detectorPath, const Mat& img, vector<Rect>& objects );
};

CV_CascadeDetectorTest::CV_CascadeDetectorTest(const char *test_name)
    : CV_DetectorTest( test_name )
{
    validationFilename = "cascadeandhog/cascade.xml";
}

void CV_CascadeDetectorTest::detectMultiScale(const string& detectorPath, const Mat& img,
                                              vector<Rect>& objects)
{
    CascadeClassifier cascade( detectorPath );
    Mat grayImg;
    cvtColor( img, grayImg, CV_BGR2GRAY );
    equalizeHist( grayImg, grayImg );
    cascade.detectMultiScale( grayImg, objects );
}

//----------------------------------------------- HOGDetectorTest -----------------------------------
class CV_HOGDetectorTest : public CV_DetectorTest
{
public:
    CV_HOGDetectorTest( const char* test_name );
protected:
    virtual void detectMultiScale( const string& detectorPath, const Mat& img, vector<Rect>& objects );
};

CV_HOGDetectorTest::CV_HOGDetectorTest(const char *test_name)
: CV_DetectorTest( test_name )
{
    validationFilename = "cascadeandhog/hog.xml";
}

void CV_HOGDetectorTest::detectMultiScale(const string& detectorPath, const Mat& img,
                                              vector<Rect>& objects)
{
    HOGDescriptor hog;
    if( detectorPath.empty() )
        hog.setSVMDetector(HOGDescriptor::getDefaultPeopleDetector());
    else
        assert(0);
    hog.detectMultiScale(img, objects);
}

CV_CascadeDetectorTest cascadeTest("cascade-detector");
CV_HOGDetectorTest hogTest("hog-detector");
