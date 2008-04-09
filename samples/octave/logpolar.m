#! /usr/bin/env octave -q
cv
highgui

src=None
dst=None
src2=None

function on_mouse( event, x, y, flags, param )

  if(! src )
    return;
  endif

  if (event==CV_EVENT_LBUTTONDOWN)
    cvLogPolar( src, dst, cvPoint2D32f(x,y), 40, CV_INTER_LINEAR+CV_WARP_FILL_OUTLIERS );
    cvLogPolar( dst, src2, cvPoint2D32f(x,y), 40, CV_INTER_LINEAR+CV_WARP_FILL_OUTLIERS+CV_WARP_INVERSE_MAP );
    cvShowImage( "log-polar", dst );
    cvShowImage( "inverse log-polar", src2 );
  endif
endfunction

filename = "../c/fruits.jpg"
if (size(argv, 1)>1)
  filename=argv(1, :)
endif

src = cvLoadImage(filename,1)
if (!src)
  print "Could not open %s" % filename
  exit(-1)
endif

cvNamedWindow( "original",1 );
cvNamedWindow( "log-polar", 1 );
cvNamedWindow( "inverse log-polar", 1 );


dst = cvCreateImage( cvSize(256,256), 8, 3 );
src2 = cvCreateImage( cvGetSize(src), 8, 3 );

cvSetMouseCallback( "original", on_mouse );
on_mouse( CV_EVENT_LBUTTONDOWN, src.width/2, src.height/2, None, None)

cvShowImage( "original", src );
cvWaitKey();
