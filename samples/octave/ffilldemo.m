#! /usr/bin/env octave -q
cv;
highgui;

color_img0 = None;
mask = None;
color_img = None;
gray_img0 = None;
gray_img = None;
ffill_case = 1;
lo_diff = 20
up_diff = 20;
connectivity = 4;
is_color = 1;
is_mask = 0;
new_mask_val = 255;

function update_lo( pos )
  lo_diff = pos;
endfunction
function update_up( pos )
  up_diff = pos;
endfunction

function on_mouse( event, x, y, flags, param )

  if( ! color_img )
    return;
  endif

  if (event == CV_EVENT_LBUTTONDOWN)
    comp = CvConnectedComp();
    my_mask = None;
    seed = cvPoint(x,y);
    if (ffill_case==0)
      lo = 0;
      up = 0;
      flags = connectivity + (new_mask_val << 8);
    else
      lo = lo_diff;
      up = up_diff;
      flags = connectivity + (new_mask_val << 8) + \
	  CV_FLOODFILL_FIXED_RANGE;
    endif
    b = random.randint(0,255);
    g = random.randint(0,255);
    r = random.randint(0,255);

    if( is_mask )
      my_mask = mask;
      cvThreshold( mask, mask, 1, 128, CV_THRESH_BINARY );
    endif
    
    if( is_color )
      
      color = CV_RGB( r, g, b );
      cvFloodFill( color_img, seed, color, CV_RGB( lo, lo, lo ),
                  CV_RGB( up, up, up ), comp, flags, my_mask );
      cvShowImage( "image", color_img );
      
    else
      
      brightness = cvRealScalar((r*2 + g*7 + b + 5)/10);
      cvFloodFill( gray_img, seed, brightness, cvRealScalar(lo),
                  cvRealScalar(up), comp, flags, my_mask );
      cvShowImage( "image", gray_img );
    endif
    

    printf("%s pixels were repainted\n", comp.area);

    if( is_mask )
      cvShowImage( "mask", mask );
    endif
  endif
endfunction




filename = "../c/fruits.jpg";
if (size(argv, 1)>1)
  filename=argv(1, :);
endif

color_img0 = cvLoadImage(filename,1);
if (! color_img0)
  printf("Could not open %s\n",filename);
  exit(-1);
endif

printf("Hot keys:\n");
printf("\tESC - quit the program\n");
printf("\tc - switch color/grayscale mode\n");
printf("\tm - switch mask mode\n");
printf("\tr - restore the original image\n");
printf("\ts - use null-range floodfill\n");
printf("\tf - use gradient floodfill with fixed(absolute) range\n");
printf("\tg - use gradient floodfill with floating(relative) range\n");
printf("\t4 - use 4-connectivity mode\n");
printf("\t8 - use 8-connectivity mode\n");

color_img = cvCloneImage( color_img0 );
gray_img0 = cvCreateImage( cvSize(color_img.width, color_img.height), 8, 1 );
cvCvtColor( color_img, gray_img0, CV_BGR2GRAY );
gray_img = cvCloneImage( gray_img0 );
mask = cvCreateImage( cvSize(color_img.width + 2, color_img.height + 2), 8, 1 );

cvNamedWindow( "image", 1 );
cvCreateTrackbar( "lo_diff", "image", lo_diff, 255, update_lo);
cvCreateTrackbar( "up_diff", "image", up_diff, 255, update_up);

cvSetMouseCallback( "image", on_mouse );

while (true)
  if( is_color )
    cvShowImage( "image", color_img );
  else
    cvShowImage( "image", gray_img );
  endif

  c = cvWaitKey(0);
  if (c=='\x1b')
    print("Exiting ...");
    exit(0)
  elseif (c=='c')
    if( is_color )
      
      print("Grayscale mode is set");
      cvCvtColor( color_img, gray_img, CV_BGR2GRAY );
      is_color = 0;
      
    else
      
      print("Color mode is set");
      cvCopy( color_img0, color_img, None );
      cvZero( mask );
      is_color = 1;
    endif
    
  elseif (c=='m')
    if( is_mask )
      cvDestroyWindow( "mask" );
      is_mask = 0;
      
    else
      cvNamedWindow( "mask", 0 );
      cvZero( mask );
      cvShowImage( "mask", mask );
      is_mask = 1;
    endif
    
  elseif (c=='r')
    print("Original image is restored");
    cvCopy( color_img0, color_img, None );
    cvCopy( gray_img0, gray_img, None );
    cvZero( mask );
  elseif (c=='s')
    print("Simple floodfill mode is set");
    ffill_case = 0;
  elseif (c=='f')
    print("Fixed Range floodfill mode is set");
    ffill_case = 1;
  elseif (c=='g')
    print("Gradient (floating range) floodfill mode is set");
    ffill_case = 2;
  elseif (c=='4')
    print("4-connectivity mode is set");
    connectivity = 4;
  elseif (c=='8')
    print("8-connectivity mode is set");
    connectivity = 8;
  endif

endwhile
