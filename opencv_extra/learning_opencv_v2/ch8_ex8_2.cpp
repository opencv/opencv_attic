//
// example 8-2
//
//
//

#include <opencv2/opencv.hpp>
#include <iostream>

using namespace cv;
using namespace std;

Mat g_gray, g_binary;
int g_thresh = 100;

void on_trackbar(int, void*) {
  threshold( g_gray, g_binary, g_thresh, 255, CV_THRESH_BINARY );
  vector<vector<Point> > contours;
  findContours(g_binary, contours, noArray(), CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE);
  g_binary = Scalar::all(0);
    
  drawContours(g_binary, contours, -1, Scalar::all(255));
  imshow( "Contours", g_binary );
}

int main( int argc, char** argv )
{
  if( argc != 2 || (g_gray = imread(argv[1], 0)).empty() )
      return -1;
  namedWindow( "Contours", 1 );
  createTrackbar( 
    "Threshold", 
    "Contours", 
    &g_thresh, 
    255, 
    on_trackbar
  );
  on_trackbar(0, 0);
  waitKey();
  return 0; 
}
