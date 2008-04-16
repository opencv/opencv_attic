#! /usr/bin/env octave
cv
highgui

laplace = []
colorlaplace = []
planes = [ [], [], [] ];
capture = []

if (size(argv, 1)==1)
  capture = cvCreateCameraCapture( 0 )
elseif (size(argv, 1)==2 && all(isdigit(argv(1, :))))
  capture = cvCreateCameraCapture( int32(argv(1, :)) )
elseif (size(argv, 1)==2)
  capture = cvCreateFileCapture( argv(1, :) ); 
endif

if (! capture)
  printf("Could not initialize capturing...\n");
  exit(-1)
endif

cvNamedWindow( "Laplacian", 1 );

while (true),
  frame = cvQueryFrame( capture );
  if (!frame)
    break
  endif

  if (!laplace)
    for i=0:len(planes)-1,
      planes[i] = cvCreateImage( \
				cvSize(frame.width,frame.height), \
				8, 1 );
    endfor
    laplace = cvCreateImage( cvSize(frame.width,frame.height), IPL_DEPTH_16S, 1 );
    colorlaplace = cvCreateImage( \
				 cvSize(frame.width,frame.height), \
				 8, 3 );
  endif

  cvSplit( frame, planes[0], planes[1], planes[2], [] );
  for plane in planes,
    cvLaplace( plane, laplace, 3 );
    cvConvertScaleAbs( laplace, plane, 1, 0 );
  endfor

  cvMerge( planes[0], planes[1], planes[2], [], colorlaplace );
  colorlaplace.origin = frame.origin;

  cvShowImage("Laplacian", colorlaplace );

  if (cvWaitKey(10) != -1)
    break;
  endif
endwhile

cvDestroyWindow("Laplacian");
