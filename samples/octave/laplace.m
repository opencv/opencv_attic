#! /usr/bin/env octave -q
cv
highgui

laplace = None
colorlaplace = None
planes = [ None, None, None ];
capture = None

if (length(argv)==1)
  capture = cvCreateCameraCapture( 0 )
elseif (length(argv)==2 && all(isdigit(argv(1))))
  capture = cvCreateCameraCapture( int(argv(1)) )
elseif (len(sys.argv)==2)
  capture = cvCreateFileCapture( argv(1) ); 
endif

if (! capture)
  print "Could not initialize capturing..."
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

  cvSplit( frame, planes[0], planes[1], planes[2], None );
  for plane in planes,
    cvLaplace( plane, laplace, 3 );
    cvConvertScaleAbs( laplace, plane, 1, 0 );
  endfor

  cvMerge( planes[0], planes[1], planes[2], None, colorlaplace );
  colorlaplace.origin = frame.origin;

  cvShowImage("Laplacian", colorlaplace );

  if (cvWaitKey(10) != -1)
    break;
  endif
endwhile

cvDestroyWindow("Laplacian");
