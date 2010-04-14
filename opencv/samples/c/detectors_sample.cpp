#include <cv.h>
#include <cvaux.h>
#include <highgui.h>
#include <iostream>

using namespace cv;
using namespace std;

struct Match
{
    int index1;      // index of the descriptor from the first set.
    int index2;      // index of the descriptor from the second set.
    double distance; // computed distance between them.

    Match(int index1, int index2, double distance)
        : index1(index1), index2(index2), distance(distance) {}
    operator double() const { return distance; }
};

void bruteForceMatche( const Mat& descriptors_1,
                       const Mat& descriptors_2,
                       const Mat& mask,
                       vector<Match>& matches )
{
    assert(mask.empty() || (mask.rows == descriptors_1.rows && mask.cols == descriptors_2.rows));
    assert(descriptors_1.cols == descriptors_2.cols);
    assert(descriptors_1.type() == CV_32F || descriptors_2.type() == CV_32F);

    matches.clear();
    matches.reserve(descriptors_1.rows);

    for (int i = 0; i < descriptors_1.rows; ++i)
    {
        Mat d1 = descriptors_1.row(i);
        int matchIndex = -1;
        float matchDistance = std::numeric_limits<float>::max();

        for (int j = 0; j < descriptors_2.rows; ++j)
        {
            if( mask.empty() || mask.at<char>(i,j) )
            {
                Mat d2 = descriptors_2.row(j);
                float distance = norm(d1, d2);
                if( distance < matchDistance )
                {
                    matchDistance = distance;
                    matchIndex = j;
                }
            }
        }

        if (matchIndex != -1)
            matches.push_back( Match(i, matchIndex, (double)matchDistance) );
    }
}

void drawCorrespondences( const Mat& img1, const Mat& img2, const Mat& transfMtr,
                          const vector<KeyPoint>& keypoints1, const vector<KeyPoint>& keypoints2,
                          const vector<Match>& matches, float maxDist, Mat& drawImg )
{
    Scalar RED = CV_RGB(255, 0, 0);
    Scalar PINK = CV_RGB(255,130,230);
    Scalar GREEN = CV_RGB(0, 255, 0);
    Scalar BLUE = CV_RGB(0, 0, 255);

    /* Output:
       red point - point without corresponding point;
       grean point - point having correct corresponding point;
       pink point - point having incorrect corresponding point, but excised by threshold of distance;
       blue point - point having incorrect corresponding point;
    */
    Size size(img1.cols + img2.cols, MAX(img1.rows, img2.rows));
    drawImg.create(size, CV_MAKETYPE(img1.depth(), 3));
    Mat drawImg1 = drawImg(Rect(0, 0, img1.cols, img1.rows));
    cvtColor(img1, drawImg1, CV_GRAY2RGB);
    Mat drawImg2 = drawImg(Rect(img1.cols, 0, img2.cols, img2.rows));
    cvtColor(img2, drawImg2, CV_GRAY2RGB);
    
    for(vector<KeyPoint>::const_iterator it = keypoints1.begin(); it < keypoints1.end(); ++it )
    {
        circle(drawImg, it->pt, 3, RED);
    }
    
    for(vector<KeyPoint>::const_iterator it = keypoints2.begin(); it < keypoints2.end(); ++it )
    {
		Point p = it->pt;
        circle(drawImg, Point(p.x+img1.cols, p.y), 3, RED);
    }
    
    Mat vec1(3, 1, CV_32FC1), vec2;
    float err = 3;
    for(vector<Match>::const_iterator it = matches.begin(); it < matches.end(); ++it )
    {
        Point pt1 = keypoints1[it->index1].pt, pt2 = keypoints2[it->index2].pt;
        vec1.at<float>(0,0) = pt1.x; vec1.at<float>(1,0) = pt1.y; vec1.at<float>(2,0) = 1.f;
        vec2 = transfMtr * vec1;
        vec2 /= vec2.at<float>(2,0);
        Point truePt2 = Point(cvRound(vec2.at<float>(0,0)), cvRound(vec2.at<float>(1,0))),
              diff = truePt2 - pt2;
        if( sqrt((double)(diff.x*diff.x + diff.y *diff.y)) < err )
        {
            circle(drawImg, pt1, 3, GREEN);
            circle(drawImg, Point(pt2.x+img1.cols, pt2.y), 3, GREEN);
            line(drawImg, pt1, Point(pt2.x+img1.cols, pt2.y), GREEN);
        }
        else
        {
            if( it->distance > maxDist )
            {
                circle(drawImg, pt1, 3, PINK);
                circle(drawImg, Point(pt2.x+img1.cols, pt2.y), 3, PINK);
            }
            else
            {
                circle(drawImg, pt1, 3, BLUE);
                circle(drawImg, Point(pt2.x+img1.cols, pt2.y), 3, BLUE);
                line(drawImg, pt1, Point(pt2.x+img1.cols, pt2.y), BLUE);
            }
        }
    }
}

FeatureDetector* createDetector( const string& detectorType )
{
	FeatureDetector* fd = 0;
	if( !strcmp( detectorType.c_str(), "FAST" ) )
	{
		fd = new FastFeatureDetector( 1/*threshold*/, true/*nonmax_suppression*/ );
	}
    else if( !strcmp( detectorType.c_str(), "STAR" ) )
	{
		fd = new StarFeatureDetector( 16/*max_size*/, 30/*response_threshold*/, 10/*line_threshold_projected*/,
									  8/*line_threshold_binarized*/, 5/*suppress_nonmax_size*/ );
	}
	else if( !strcmp( detectorType.c_str(), "SURF" ) )
	{
		fd = new SurfFeatureDetector( 400./*hessian_threshold*/, 3 /*octaves*/, 4/*octave_layers*/ );
	}
	else if( !strcmp( detectorType.c_str(), "MSER" ) )
	{
		fd = new MserFeatureDetector( 5/*delta*/, 60/*min_area*/, 14400/*_max_area*/, 0.25f/*max_variation*/,
				0.2/*min_diversity*/, 200/*max_evolution*/, 1.01/*area_threshold*/, 0.003/*min_margin*/, 
				5/*edge_blur_size*/ );
	}
	else if( !strcmp( detectorType.c_str(), "GFTT" ) )
	{
        fd = new GoodFeaturesToTrackDetector( 1000/*maxCorners*/, 0.01/*qualityLevel*/, 1./*minDistance*/,
                                              3/*int _blockSize*/, true/*useHarrisDetector*/, 0.04/*k*/ );
	}
	else
		fd = 0;
		
	return fd;
}

DescriptorExtractor* createDescExtractor( const string& descriptorType )
{
	DescriptorExtractor* de = 0;
	if( !strcmp( descriptorType.c_str(), "CALONDER" ) )
	{
		assert(0);
        //de = new CalonderDescriptorExtractor<float>("");
	}
	else if( !strcmp( descriptorType.c_str(), "SURF" ) )
	{
		de = new SurfDescriptorExtractor( 3/*octaves*/, 4/*octave_layers*/, false/*extended*/ );
	}
	else 
		de = 0;
	return de;
}

/*DescriptorMatcher* createDescMatcher( const string& matherType)
{
	return new BruteForceMatcher<L2<float> >();
}*/

const string DETECTOR_TYPE_STR = "detector_type";
const string DESCRIPTOR_TYPE_STR = "descriptor_type";

const string winName = "correspondences";

void iter( Ptr<FeatureDetector> detector, Ptr<DescriptorExtractor> descriptor,
           const Mat& img1, float maxDist, Mat& transfMtr, RNG* rng = 0 )
{
    if( transfMtr.empty() )
        transfMtr = Mat::eye(3, 3, CV_32FC1);
    if( rng )
    {
        transfMtr.at<float>(0,0) = rng->uniform( 0.7f, 1.3f);
        transfMtr.at<float>(0,1) = rng->uniform(-0.2f, 0.2f);
        transfMtr.at<float>(0,2) = rng->uniform(-0.1f, 0.1f)*img1.cols;
        transfMtr.at<float>(1,0) = rng->uniform(-0.2f, 0.2f);
        transfMtr.at<float>(1,1) = rng->uniform( 0.7f, 1.3f);
        transfMtr.at<float>(1,2) = rng->uniform(-0.1f, 0.3f)*img1.rows;
        transfMtr.at<float>(2,0) = rng->uniform( -1e-4f, 1e-4f);
        transfMtr.at<float>(2,1) = rng->uniform( -1e-4f, 1e-4f);
        transfMtr.at<float>(2,2) = rng->uniform( 0.7f, 1.3f);
    }

    Mat img2; warpPerspective( img1, img2, transfMtr, img1.size() );


    cout << endl << "< Extracting keypoints... ";
    vector<KeyPoint> keypoints1, keypoints2;
    detector->detect( img1, keypoints1 );
    detector->detect( img2, keypoints2 );
    cout << keypoints1.size() << " from first image and " << keypoints2.size() << " from second image >" << endl;
    if( keypoints1.empty() || keypoints2.empty() )
        cout << "end" << endl;

    cout << "< Computing  descriptors... ";
    Mat descs1, descs2;
    descriptor->compute( img1, keypoints1, descs1 );
    descriptor->compute( img2, keypoints2, descs2 );
    cout << ">" << endl;

    cout << "< Matching keypoints by descriptors... ";
    vector<Match> matches;
    bruteForceMatche(descs1, descs2, Mat(), matches);
    cout << ">" << endl;

    // TODO time

    Mat drawImg;
    drawCorrespondences(img1, img2, transfMtr, keypoints1, keypoints2, matches, maxDist, drawImg );
    imshow( winName, drawImg);
}

Ptr<FeatureDetector> detector;
Ptr<DescriptorExtractor> descriptor;
Mat img1;
Mat transfMtr;
RNG rng;
const float maxDistScale = 0.01f;
int maxDist;

void onMaxDistChange( int maxDist, void* )
{
    float realMaxDist = maxDist*maxDistScale;
    cout << "maxDist " <<  realMaxDist << endl;
    iter( detector, descriptor, img1, realMaxDist, transfMtr );
}

int main(int argc, char** argv)
{
    if( argc != 4 )
    {
        cout << "Format:" << endl;
        cout << "./" << argv[0] << " [detector_type] [descriptor_type] [image]" << endl;
        return 0;
    }
    
    cout << "< Creating detector, descriptor and matcher... ";
    detector = createDetector(argv[1]);
    descriptor = createDescExtractor(argv[2]);
    //Ptr<DescriptorMatcher> matcher = createDescMatcher(argv[3]);
    cout << ">" << endl;
    if( detector.empty() || descriptor.empty()/* || matcher.empty() */ )
    {
		cout << "Can not create detector or descriptor or matcher of given types" << endl;
		return 0;
	}
		
    cout << "< Reading the image... ";
    img1 = imread( argv[3], CV_LOAD_IMAGE_GRAYSCALE);
    cout << ">" << endl;
    if( img1.empty() )
    {
        cout << "Can not read image" << endl;
        return 0;
    }

    namedWindow(winName, 1);
    maxDist = 25;
    createTrackbar( "maxDist", winName, &maxDist, 100, onMaxDistChange );

    onMaxDistChange(maxDist, 0);
    for(;;)
    {
        char c = (char)cvWaitKey(0);
        if( c == '\x1b' ) // esc
        {
            cout << "Exiting ..." << endl;
            return 0;
        }
        else if( c == 'n' )
            iter(detector, descriptor, img1, maxDist*maxDistScale, transfMtr, &rng);
    }
    waitKey(0);
}
