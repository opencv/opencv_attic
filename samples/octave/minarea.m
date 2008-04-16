#! /usr/bin/env octave

cv;
highgui;

function ret = randint(a, b)
  ret = int32(rand() * (b - a) + a);
endfunction

function minarea_array(img, count)
  global CV_32SC2;
  global cvZero;
  pointMat = cvCreateMat( count, 1, CV_32SC2 );
  for i=0:count-1,
    pointMat(i) = cvPoint( randint(img.width/4, img.width*3/4), randint(img.height/4, img.height*3/4) );
  endfor

  box = cvMinAreaRect2( pointMat );
  box_vtx = cvBoxPoints( box );
  [success, center, radius] = cvMinEnclosingCircle( pointMat );
  cvZero( img );
  for i=0:count-1,
    cvCircle( img, cvGet1D(pointMat,i), 2, CV_RGB( 255, 0, 0 ), \
	     CV_FILLED, CV_AA, 0 );
  endfor

  box_vtx = {cvPointFrom32f(box_vtx{1}),
             cvPointFrom32f(box_vtx{2}),
             cvPointFrom32f(box_vtx{3}),
             cvPointFrom32f(box_vtx{4})};
  cvCircle( img, cvPointFrom32f(center), cvRound(radius), CV_RGB(255, 255, 0), 1, CV_AA, 0 );
  cvPolyLine( img, {box_vtx}, 1, CV_RGB(0,255,255), 1, CV_AA ) ;
endfunction


function minarea_seq(img, count, storage)
  global CV_SEQ_KIND_GENERIC;
  global CV_32SC2;
  global sizeof_CvContour;
  global sizeof_CvPoint;
  global CvSeq_CvPoint;
  global cvZero;
  ptseq = cvCreateSeq( bitor(CV_SEQ_KIND_GENERIC, CV_32SC2), sizeof_CvContour, sizeof_CvPoint, storage );
  ptseq = CvSeq_CvPoint.cast( ptseq );
  for i=0:count-1,
    pt0 = cvPoint( randint(img.width/4, img.width*3/4), randint(img.height/4, img.height*3/4) );
    cvSeqPush( ptseq, pt0 );
  endfor
  box = cvMinAreaRect2( ptseq );
  box_vtx = cvBoxPoints( box );
  [success, center, radius] = cvMinEnclosingCircle( ptseq );
  cvZero( img );
  for pt = ptseq,
    cvCircle( img, pt, 2, CV_RGB( 255, 0, 0 ), CV_FILLED, CV_AA, 0 );
  endfor

  box_vtx = {cvPointFrom32f(box_vtx{0}),
             cvPointFrom32f(box_vtx{1}),
             cvPointFrom32f(box_vtx{2}),
             cvPointFrom32f(box_vtx{3})};
  cvCircle( img, cvPointFrom32f(center), cvRound(radius), CV_RGB(255, 255, 0), 1, CV_AA, 0 );
  cvPolyLine( img, {box_vtx}, 1, CV_RGB(0,255,255), 1, CV_AA );
  cvClearMemStorage( storage );
endfunction

img = cvCreateImage( cvSize( 500, 500 ), 8, 3 );
storage = cvCreateMemStorage(0);

cvNamedWindow( "rect & circle", 1 );

use_seq=false;

while (true),
  count = randint(1,100);
  if (use_seq)
    minarea_seq(img, count, storage);
  else
    minarea_array(img, count);
  endif

  cvShowImage("rect & circle", img);
  key = cvWaitKey();
  if( key == '\x1b' );
    break;
  endif

  use_seq = !use_seq;
endwhile
