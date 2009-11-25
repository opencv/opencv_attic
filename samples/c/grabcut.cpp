#include <highgui.h>
#include <cv.h>
#include <iostream>

using namespace std;
using namespace cv;

const string winName = "image";
const Scalar RED = Scalar(0,0,255);
const Scalar BLUE = Scalar(255,0,0);

void getBinMask( const Mat& comMask, Mat& binMask )
{
    if( comMask.empty() || comMask.type()!=CV_8UC1 )
        CV_Error( CV_StsBadArg, "comMask is empty or has incorrect type (not CV_8UC1)" );
    if( binMask.empty() || binMask.rows!=comMask.rows || binMask.cols!=comMask.cols )
        binMask.create( comMask.size(), CV_8UC1 );
    binMask = comMask & 1;
}

void changeMask( Mat& mask, vector<Point> bgdPixels, vector<Point> fgdPixels )
{
    vector<Point>::const_iterator it = bgdPixels.begin(), itEnd = bgdPixels.end();
    for( ; it != itEnd; ++it )
        mask.at<uchar>(*it) = GC_BGD;
    it = fgdPixels.begin(), itEnd = fgdPixels.end();
    for( ; it != itEnd; ++it )
        mask.at<uchar>(*it) = GC_FGD;
}

void showImage( Mat& _img, Mat& _mask, vector<Point>& _bgdPxls, vector<Point>& _fgdPxls )
{
    Mat res;
    Mat binMask;
    if( _mask.empty() )
        _img.copyTo( res );
    else
    {
        getBinMask( _mask, binMask );
        _img.copyTo( res, binMask );
    }

    vector<Point>::const_iterator it;
    for( it = _bgdPxls.begin(); it != _bgdPxls.end(); ++it )
        circle( res, *it, 1, BLUE );
    for( it = _fgdPxls.begin(); it != _fgdPxls.end(); ++it )
        circle( res, *it, 1, RED );

    imshow( winName, res );
}

Mat image;
Mat mask;
Mat bgdModel, fgdModel;
Rect rect;

enum{ NOT_SET = 0, IN_PROCESS = 1, SET = 2 };
uchar rectState = NOT_SET;
uchar lblsState = NOT_SET;

vector<Point> fgdPxls, bgdPxls;
Point p1, p2;
int iterCount;

void on_mouse( int event, int x, int y, int flags, void* param )
{
    switch( event )
    {
    case CV_EVENT_LBUTTONDOWN:
        {
            bool isb = (flags & CV_EVENT_FLAG_ALTKEY) != 0,
                isf = (flags & CV_EVENT_FLAG_SHIFTKEY) != 0;
            if( rectState == NOT_SET && !isb && !isf )
            {
                rectState = IN_PROCESS;
                p1 = Point(x, y);
            }
            if ( (isb || isf) && rectState == SET )
            {
                lblsState = IN_PROCESS;
            }
        }
        break;
    case CV_EVENT_LBUTTONUP:
        if( rectState == IN_PROCESS )
        {
            p2 = Point(x, y);
            rect = Rect( p1, p2 );
            rectState = SET;
            grabCut( image, mask, rect, bgdModel, fgdModel, 0, GC_INIT_WITH_RECT );
            assert( bgdPxls.empty() && fgdPxls.empty() );
            showImage( image, mask, bgdPxls, fgdPxls );
        }
        if( lblsState == IN_PROCESS )
        {
            lblsState = NOT_SET;
            showImage( image, mask, bgdPxls, fgdPxls );
        }
        break;
    case CV_EVENT_MOUSEMOVE:
        if( rectState == IN_PROCESS )
        {
            Mat res;
            image.copyTo(res);
            rectangle(res, p1, Point(x,y), Scalar(0,255,0), 2);
            imshow(winName, res);
        }
        else if( lblsState == IN_PROCESS )
        {
            Point p = Point(x, y);
            if( flags & CV_EVENT_FLAG_ALTKEY )
            {
                bgdPxls.push_back(p);
                showImage( image, mask, bgdPxls, fgdPxls );
            }
            if( flags & CV_EVENT_FLAG_SHIFTKEY )
            {
                fgdPxls.push_back( Point(x, y) );
                showImage( image, mask, bgdPxls, fgdPxls );
            }
        }
        break;
    }
}

int main( int argc, char** argv )
{
    if( argc==1 )
        return 1;
    string filename = argv[1];
    if( filename.empty() )
        return 1;
    image = imread( filename, 1 );
    if( image.empty() )
        return 1;

    cout << "Hot keys: \n" << 
        "\tESC - quit the program\n"
        "First, select the rectangular area\n"
        "\tr - restore the original image\n"
        "\tn - next iteration\n"
        "\n\n"
        "\tleft mouse button - set rectangle\n"
        "\talt+left mouse button - set background pixels\n"
        "\tshift+left mouse button - set foreground pixels\n";

    cvNamedWindow( winName.c_str(), CV_WINDOW_AUTOSIZE );
    imshow( winName, image );

    cvSetMouseCallback( winName.c_str(), on_mouse, 0 );

    rectState = NOT_SET;
    lblsState = NOT_SET;
    iterCount = 0;
    for(;;)
    {
        int c = cvWaitKey(0);
        switch( (char) c )
        {
        case '\x1b':
            cout << "Exiting ..." << endl;
            goto exit_main;
        case 'r':
            rectState = NOT_SET;
            lblsState = NOT_SET;
            iterCount = 0;
            bgdPxls.clear(); fgdPxls.clear();
            mask.release();
            cout << endl;
            assert( bgdPxls.empty() && fgdPxls.empty() && mask.empty() );
            showImage( image, mask, bgdPxls, fgdPxls );
            break;
        case 'n':
            cout << "<" << iterCount << "... ";
            if( rectState == SET )
            {
                changeMask( mask, bgdPxls, fgdPxls );
                bgdPxls.clear(); fgdPxls.clear();
                grabCut( image, mask, rect, bgdModel, fgdModel, 1 );
                showImage( image, mask, bgdPxls, fgdPxls );
                cout << iterCount << ">" << endl;
                iterCount++;
            }
            else
                cout << "rect is not determined>" << endl;
            break;
        }
    }

exit_main:
    cvDestroyWindow( winName.c_str() );
    return 0;
}
