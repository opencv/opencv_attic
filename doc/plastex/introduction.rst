

************
Introduction
************

.. toctree::
   :maxdepth: 2

   c++_cheatsheet
   namespace_:cfunc:`cv()`_and_function_naming
   memory_management
   memory_management_part_ii._automatic_data_allocation
   algebraic_operations
   fast_element_access
   saturation_arithmetics
   error_handling
   threading_and_reenterability



Starting from OpenCV 2.0 the new modern C++ interface has been introduced. It is crisp (less typing is needed to code the same thing), type-safe (no more :ctype:`CvArr*` a.k.a. ``void*``) and, in general, more convenient to use. Here is a short example of what it looks like: 



  ::

      //
      // Simple retro-style photo effect done by adding noise to
      // the luminance channel and reducing intensity of the chroma channels
      //
      
      // include standard OpenCV headers, same as before
      #include "cv.h"
      #include "highgui.h"
      
      // all the new API is put into "cv" namespace. Export its content
      using namespace cv;
      
      // enable/disable use of mixed API in the code below.
      #define DEMO_MIXED_API_USE 1
      
      int main( int argc, char** argv )
      {
          const char* imagename = argc > 1 ? argv[1] : "lena.jpg";
      #if DEMO_MIXED_API_USE
          // Ptr<T> is safe ref-conting pointer class
          Ptr<IplImage> iplimg = cvLoadImage(imagename);
          
          // cv::Mat replaces the CvMat and IplImage, but it's easy to convert
          // between the old and the new data structures
          // (by default, only the header is converted and the data is shared)
          Mat img(iplimg); 
      #else
          // the newer cvLoadImage alternative with MATLAB-style name
          Mat img = imread(imagename);
      #endif
      
          if( !img.data ) // check if the image has been loaded properly
              return -1;
      
          Mat img_yuv;
          // convert image to YUV color space.
          // The output image will be allocated automatically
          cvtColor(img, img_yuv, CV_BGR2YCrCb); 
      
          // split the image into separate color planes
          vector<Mat> planes;
          split(img_yuv, planes);
      
          // another Mat constructor; allocates a matrix of the specified
      	// size and type
          Mat noise(img.size(), CV_8U);
          
          // fills the matrix with normally distributed random values;
          // there is also randu() for uniformly distributed random numbers. 
          // Scalar replaces CvScalar, Scalar::all() replaces cvScalarAll().
          randn(noise, Scalar::all(128), Scalar::all(20));
                                                           
          // blur the noise a bit, kernel size is 3x3 and both sigma's 
      	// are set to 0.5
          GaussianBlur(noise, noise, Size(3, 3), 0.5, 0.5);
      
          const double brightness_gain = 0;
          const double contrast_gain = 1.7;
      #if DEMO_MIXED_API_USE
          // it's easy to pass the new matrices to the functions that
          // only work with IplImage or CvMat:
          // step 1) - convert the headers, data will not be copied
          IplImage cv_planes_0 = planes[0], cv_noise = noise;
          // step 2) call the function; do not forget unary "&" to form pointers
          cvAddWeighted(&cv_planes_0, contrast_gain, &cv_noise, 1,
                       -128 + brightness_gain, &cv_planes_0);
      #else
          addWeighted(planes[0], constrast_gain, noise, 1,
                      -128 + brightness_gain, planes[0]);
      #endif
          const double color_scale = 0.5;
          // Mat::convertTo() replaces cvConvertScale.
          // One must explicitly specify the output matrix type
          // (we keep it intact, i.e. pass planes[1].type())
          planes[1].convertTo(planes[1], planes[1].type(),
                              color_scale, 128*(1-color_scale));
      
          // alternative form of convertTo if we know the datatype
          // at compile time ("uchar" here).
          // This expression will not create any temporary arrays
          // and should be almost as fast as the above variant
          planes[2] = Mat_<uchar>(planes[2]*color_scale + 128*(1-color_scale));
      
          // Mat::mul replaces cvMul(). Again, no temporary arrays are
          // created in the case of simple expressions.
          planes[0] = planes[0].mul(planes[0], 1./255);
      
          // now merge the results back
          merge(planes, img_yuv);
          // and produce the output RGB image
          cvtColor(img_yuv, img, CV_YCrCb2BGR);
      
          // this is counterpart for cvNamedWindow
          namedWindow("image with grain", CV_WINDOW_AUTOSIZE);
      #if DEMO_MIXED_API_USE
          // this is to demonstrate that img and iplimg really share the data -
          // the result of the above processing is stored to img and thus 
      	// in iplimg too.
          cvShowImage("image with grain", iplimg);
      #else
          imshow("image with grain", img);
      #endif
          waitKey();
      
          return 0;
          // all the memory will automatically be released
          // by vector<>, Mat and Ptr<> destructors.
      }



Following a summary "cheatsheet" below, the rest of the introduction will discuss the key features of the new interface in more detail. 
