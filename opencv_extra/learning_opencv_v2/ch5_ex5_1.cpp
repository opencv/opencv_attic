//
//Example 5-1. Doing something with each element in the sequence of connected components returned
//             by cvPyrSegmentation(
//
/* *************** License:**************************
   Oct. 3, 2008
   Right to use this code in any way you want without warrenty, support or any guarentee of it working.

   BOOK: It would be nice if you cited it:
   Learning OpenCV: Computer Vision with the OpenCV Library
     by Gary Bradski and Adrian Kaehler
     Published by O'Reilly Media, October 3, 2008
 
   AVAILABLE AT: 
     http://www.amazon.com/Learning-OpenCV-Computer-Vision-Library/dp/0596516134
     Or: http://oreilly.com/catalog/9780596516130/
     ISBN-10: 0596516134 or: ISBN-13: 978-0596516130    

   OTHER OPENCV SITES:
   * The source code is on sourceforge at:
     http://sourceforge.net/projects/opencvlibrary/
   * The OpenCV wiki page (As of Oct 1, 2008 this is down for changing over servers, but should come back):
     http://opencvlibrary.sourceforge.net/
   * An active user group is at:
     http://tech.groups.yahoo.com/group/OpenCV/
   * The minutes of weekly OpenCV development meetings are at:
     http://pr.willowgarage.com/wiki/OpenCV
   ************************************************** */

#include <opencv2/opencv.hpp>
#include <iostream>

using namespace cv;
using namespace std;

void f( 
  const Mat& src,
  Mat& dst
) {
  Ptr<CvMemStorage> storage = cvCreateMemStorage(0);
  Seq<CvConnectedComp> comp;

  dst.create(src.size(), src.type());
  IplImage c_src = src, c_dst = dst;
  cvPyrSegmentation( &c_src, &c_dst, storage, &comp.seq, 4, 200, 50 );
  int n_comp = comp.size();

  for( int i=0; i<n_comp; i++ ) {
	CvConnectedComp cc = comp[i];
    // do_something_with( cc );
	//rectangle(dst, cc.rect, Scalar(128, 255, 255), 1);  
  }
}

int main(int argc, char** argv)
{
  if(argc < 2) { cout << "specify input image" << endl; return -1; }
  // Create a named window with a the name of the file.
  namedWindow( argv[1], 1 );
  // Load the image from the given file name.
  Mat src = imread(argv[1]), dst;
  if(src.empty()) { cout << "Couldn't seem to open " << argv[1] << ", sorry\n"; return -1;}
  f( src, dst);

  // Show the image in the named window
  imshow( argv[1], dst );

  // Idle until the user hits any key.
  waitKey();
  return 0;	
}
