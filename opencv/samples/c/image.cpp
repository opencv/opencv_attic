// The short example shows how to use new-style image classes declared in cxcore.hpp.
// There is also a very similar matrix class (CvMatrix) - a wrapper for CvMat
#include "cv.h"
#include "highgui.h"

int main( int argc, char** argv )
{
    // load image in constructor: the image can be loaded either from bitmap (see cvLoadImage),
    // or from XML/YAML (see cvLoad)
    CvImage img(argc > 1 ? argv[1] : "lena.jpg", 0, CV_LOAD_IMAGE_COLOR),
        img_yuv, y, noise;
    CvRNG rng = cvRNG(-1);

    if( !img.data() ) // check if the image has been loaded properly
        return -1;

    img_yuv = img.clone(); // clone the image
                           // (although, later the content will be replaced with cvCvtColor,
                           // clone() is used for simplicity and for the illustration)
    cvCvtColor( img, img_yuv, CV_BGR2YCrCb ); // simply call OpenCV functions and pass the class instances there

    y.create( img.size(), IPL_DEPTH_8U, 1 ); // another method to create an image - from scratch
    noise.create( img.size(), IPL_DEPTH_32F, 1 );

    cvSplit( img_yuv, y, 0, 0, 0 );
    cvRandArr( &rng, noise, CV_RAND_NORMAL, cvScalarAll(0), cvScalarAll(20) );
    cvSmooth( noise, noise, CV_GAUSSIAN, 5, 5, 1, 1 );
    cvAcc( y, noise );
    cvConvert( noise, y );
    cvMerge( y, 0, 0, 0, img_yuv );
    cvCvtColor( img_yuv, img, CV_YCrCb2BGR );

    cvNamedWindow( "image with grain", CV_WINDOW_AUTOSIZE );
    img.show( "image with grain" ); // .show method is the conveninient form of cvShowImage
    cvWaitKey();

    return 0;
    // all the images will be released automatically
}




