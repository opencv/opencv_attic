#! /usr/bin/env octave -q
cv;
highgui;

inpaint_mask = None;
img0 = None;
img = None;
inpainted = None;
prev_pt = cvPoint(-1,-1);

function on_mouse( event, x, y, flags, param )
  global prev_pt
  if (!img)
    return;
  endif

  if (event == CV_EVENT_LBUTTONUP || ! (flags & CV_EVENT_FLAG_LBUTTON))
    prev_pt = cvPoint(-1,-1);
  elseif (event == CV_EVENT_LBUTTONDOWN)
    prev_pt = cvPoint(x,y);
  elseif (event == CV_EVENT_MOUSEMOVE && (flags & CV_EVENT_FLAG_LBUTTON))
    pt = cvPoint(x,y);
    if (prev_pt.x < 0)
      prev_pt = pt;
    endif
    cvLine( inpaint_mask, prev_pt, pt, cvScalarAll(255), 5, 8, 0 );
    cvLine( img, prev_pt, pt, cvScalarAll(255), 5, 8, 0 );
    prev_pt = pt;
    cvShowImage( "image", img );
  endif
endfunction

filename = "../c/fruits.jpg";
if (size(argv, 1)>=2)
  filename = argv(1, :);
endif

img0 = cvLoadImage(filename,-1);
if (!img0)
  printf("Can't open image '%s'\n", filename);
  exit(1);
endif

printf("Hot keys:\n");
printf("\tESC - quit the program\n");
printf("\tr - restore the original image\n");
printf("\ti or ENTER - run inpainting algorithm\n");
printf("\t\t(before running it, paint something on the image)\n");

cvNamedWindow( "image", 1 );

img = cvCloneImage( img0 );
inpainted = cvCloneImage( img0 );
inpaint_mask = cvCreateImage( cvGetSize(img), 8, 1 );

cvZero( inpaint_mask );
cvZero( inpainted );
cvShowImage( "image", img );
cvShowImage( "watershed transform", inpainted );
cvSetMouseCallback( "image", on_mouse, None );

while (true)
  c = cvWaitKey(0);

  if( c == '\x1b' || c=='q')
    break;
  endif

  if( c == 'r' )
    cvZero( inpaint_mask );
    cvCopy( img0, img );
    cvShowImage( "image", img );
  endif

  if( c == 'i' || c == '\012' )
    cvNamedWindow( "inpainted image", 1 );
    cvInpaint( img, inpaint_mask, inpainted, 3, CV_INPAINT_TELEA );
    cvShowImage( "inpainted image", inpainted );
  endif
endwhile

