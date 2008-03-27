#! /usr/bin/env octave -q

cv
highgui

marker_mask = None;
markers = None;
img0 = None
img = None
img_gray = None 
wshed = None
prev_pt = cvPoint(-1,-1)

function on_mouse( event, x, y, flags, param )
  global prev_pt
  if( ! img )
    return;
  endif
  if( event == CV_EVENT_LBUTTONUP or not (flags & CV_EVENT_FLAG_LBUTTON) )
    prev_pt = cvPoint(-1,-1);
  elseif( event == CV_EVENT_LBUTTONDOWN )
    prev_pt = cvPoint(x,y);
  elseif( event == CV_EVENT_MOUSEMOVE and (flags & CV_EVENT_FLAG_LBUTTON) )
    pt = cvPoint(x,y);
    if( prev_pt.x < 0 )
      prev_pt = pt;
    endif
    cvLine( marker_mask, prev_pt, pt, cvScalarAll(255), 5, 8, 0 );
    cvLine( img, prev_pt, pt, cvScalarAll(255), 5, 8, 0 );
    prev_pt = pt;
    cvShowImage( "image", img );
  endif
endfunction

filename = "../c/fruits.jpg"
if (length(argv)>1)
  filename = argv(1)
endif

rng = cvRNG(-1);
img0 = cvLoadImage(filename,1)
if (!img0)
  print "Error opening image '%s'" % filename
  exit(-1)
endif

printf("Hot keys:\n");
printf("\tESC - quit the program\n");
printf("\tr - restore the original image\n");
printf("\tw - run watershed algorithm\n");
printf("\t  (before that, roughly outline several markers on the image)\n");

cvNamedWindow( "image", 1 );
cvNamedWindow( "watershed transform", 1 );

img = cvCloneImage( img0 );
img_gray = cvCloneImage( img0 );
wshed = cvCloneImage( img0 );
marker_mask = cvCreateImage( cvGetSize(img), 8, 1 );
markers = cvCreateImage( cvGetSize(img), IPL_DEPTH_32S, 1 );

cvCvtColor( img, marker_mask, CV_BGR2GRAY );
cvCvtColor( marker_mask, img_gray, CV_GRAY2BGR );

cvZero( marker_mask );
cvZero( wshed );

cvShowImage( "image", img );
cvShowImage( "watershed transform", wshed );

cvSetMouseCallback( "image", on_mouse, None );
while (true)
  c = cvWaitKey(0);
  if (c=='\x1b')
    break;
  endif
  if (c == 'r')
    cvZero( marker_mask );
    cvCopy( img0, img );
    cvShowImage( "image", img );
  endif
  if (c == 'w')
    storage = cvCreateMemStorage(0);
    comp_count = 0;
#cvSaveImage( "wshed_mask.png", marker_mask );
#marker_mask = cvLoadImage( "wshed_mask.png", 0 );
    nb_cont, contours = cvFindContours( marker_mask, storage, sizeof_CvContour,
				       CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE );
    cvZero( markers );
    while (contours)
      cvDrawContours( markers, contours, cvScalarAll(comp_count+1),
                     cvScalarAll(comp_count+1), -1, -1, 8, cvPoint(0,0) );
      contours=contours.h_next
      comp_count+=1
    endwhile
    color_tab = cvCreateMat( comp_count, 1, CV_8UC3 );
    for i=0:comp_count-1,
      color_tab[i] = cvScalar( mod(cvRandInt(rng),180) + 50, 
                              mod(cvRandInt(rng),180) + 50, 
                              mod(cvRandInt(rng),180) + 50 );
    endfor
    t = cvGetTickCount();
    cvWatershed( img0, markers );
    t = cvGetTickCount() - t;
#print "exec time = %f" % t/(cvGetTickFrequency()*1000.)

    cvSet( wshed, cvScalarAll(255) );

# paint the watershed image
    for j=0:markers.height-1,
      for i=0:markers.width-1,
	idx = markers[j,i]
	if (idx==-1)
          continue
	endif
	idx = idx-1
	wshed[j,i] = color_tab[idx,0]
      endfor
    endfor

    cvAddWeighted( wshed, 0.5, img_gray, 0.5, 0, wshed );
    cvShowImage( "watershed transform", wshed );
    cvWaitKey();
  endif
endwhile
