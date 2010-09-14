#include <highgui.h>
#include "opencv2/features2d/features2d.hpp"
#include <iostream>

using namespace cv;
using namespace std;

void maskMatchesByTrainImgIdx( const vector<DMatch>& matches, int trainImgIdx, vector<char>& mask )
{
    mask.resize( matches.size(), 0 );
    for( size_t i = 0; i < matches.size(); i++ )
    {
        if( matches[i].trainImgIdx == trainImgIdx )
            mask[i] = 1;
    }
}

int main(int argc, char** argv)
{
    if( argc != 7 )
    {
        cout << "Format:" << endl;
        cout << argv[0] << "[detector_type] [descriptor_type] [matcher_type] [query_image] [train_image1] [train_image2]" << endl;
        return -1;
    }

    cout << "< Creating feature detector, descriptor extractor and descriptor matcher ..." << endl;
    Ptr<FeatureDetector> detector = createFeatureDetector( argv[1] );
    Ptr<DescriptorExtractor> descriptorExtractor = createDescriptorExtractor( argv[2] );
    Ptr<DescriptorMatcher> descriptorMatcher = createDescriptorMatcher( argv[3] );
    cout << ">" << endl;
    if( detector.empty() || descriptorExtractor.empty() || descriptorMatcher.empty()  )
    {
        cout << "Can not create feature detector or descriptor exstractor or descriptor matcher of given types" << endl;
        return -1;
	}

    Mat queryImg;
    vector<KeyPoint> queryPoints;
    Mat queryDescs;

    vector<Mat> trainImgCollection( 2 );
    vector<vector<KeyPoint> > trainPointCollection;
    vector<Mat> trainDescCollection;

    vector<DMatch> matches;

    cout << "< Reading the images..." << endl;
    queryImg = imread( argv[4], CV_LOAD_IMAGE_GRAYSCALE);
    trainImgCollection[0] = imread( argv[5], CV_LOAD_IMAGE_GRAYSCALE );
    trainImgCollection[1] = imread( argv[6], CV_LOAD_IMAGE_GRAYSCALE );
    cout << ">" << endl;
    if( queryImg.empty() || trainImgCollection[0].empty() || trainImgCollection[1].empty() )
    {
        cout << "Can not read images" << endl;
        return -1;
    }

    cout << endl << "< Extracting keypoints from images..." << endl;
    detector->detect( queryImg, queryPoints );
    detector->detect( trainImgCollection, trainPointCollection );
    cout << ">" << endl;

    cout << "< Computing descriptors for keypoints from first image..." << endl;
    descriptorExtractor->compute( queryImg, queryPoints, queryDescs );
    descriptorExtractor->compute( trainImgCollection, trainPointCollection, trainDescCollection );
    cout << ">" << endl;

    cout << "< Set train descriptors collection in the matcher and match query descriptors to them..." << endl;
    descriptorMatcher->add( trainDescCollection );
    descriptorMatcher->match( queryDescs, matches );
    cout << ">" << endl;

    string name0 = "Matches with first train image";
    string name1 = "Matches with second train image";

    Mat drawImg0, drawImg1;
    vector<char> mask;

    maskMatchesByTrainImgIdx( matches, 0, mask);
    drawMatches( queryImg, queryPoints,
                 trainImgCollection[0], trainPointCollection[0],
                 matches, drawImg0, Scalar::all(-1), Scalar::all(-1), mask );

    maskMatchesByTrainImgIdx( matches, 1, mask);
    drawMatches( queryImg, queryPoints,
                 trainImgCollection[1], trainPointCollection[1],
                 matches, drawImg1, Scalar::all(-1), Scalar::all(-1), mask );

    namedWindow( name0, 1);
    namedWindow( name1, 1);

    imshow( name0, drawImg0 );
    imshow( name1, drawImg1 );

    waitKey(0);
    return 0;
}
