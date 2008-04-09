#! /usr/bin/env octave -q

printf("OpenCV Octave version of lkdemo\n");

## import the necessary things for OpenCV
cv
highgui

#############################################################################
## some "constants"

win_size = 10
MAX_COUNT = 500

#############################################################################
## some "global" variables

image = None
pt = None
add_remove_pt = false
flags = 0
night_mode = false
need_to_init = false

#############################################################################
## the mouse callback

## the callback on the trackbar
function on_mouse (event, x, y, flags, param)

  ## we will use the global pt and add_remove_pt
  global pt
  global add_remove_pt
  
  if (swig_this(image) == 0)
    ## not initialized, so skip
    return
  endif

  if (image.origin != 0)
    ## different origin
    y = image.height - y
  endif

  if (event == highgui.CV_EVENT_LBUTTONDOWN)
    ## user has click, so memorize it
    pt = cv.cvPoint (x, y)
    add_remove_pt = true
  endif
endfunction

#############################################################################
## so, here is the main part of the program

try
  ## try to get the device number from the command line
  device = int32 (argv (1, :))

  ## got it ! so remove it from the arguments
  argv (1, :) = []
catch
  ## no device number on the command line, assume we want the 1st device
  device = 0
end_try_catch

if (size (argv, 1) == 1)
  ## no argument on the command line, try to use the camera
  capture = highgui.cvCreateCameraCapture (device)

else
  ## we have an argument on the command line,
  ## we can assume this is a file name, so open it
  capture = highgui.cvCreateFileCapture (argv (1, :))
endif

## check that capture device is OK
if (!capture)
  print "Error opening capture device"
  exit(1)
endif

## display a small howto use it
printf("Hot keys: \n" \
       "\tESC - quit the program\n" \
       "\tr - auto-initialize tracking\n" \
       "\tc - delete all the points\n" \
       "\tn - switch the \"night\" mode on/off\n" \
       "To add/remove a feature point click it\n");

## first, create the necessary windows
highgui.cvNamedWindow ('LkDemo', highgui.CV_WINDOW_AUTOSIZE)

## register the mouse callback
highgui.cvSetMouseCallback ('LkDemo', on_mouse, None)

while (1)
  ## do forever

  ## 1. capture the current image
  frame = highgui.cvQueryFrame (capture)
  if (swig_this(frame) == 0)
    ## no image captured... end the processing
    break
  endif

  if (swig_this(image) == 0),
    ## create the images we need
    image = cv.cvCreateImage (cv.cvGetSize (frame), 8, 3)
    image.origin = frame.origin
    grey = cv.cvCreateImage (cv.cvGetSize (frame), 8, 1)
    prev_grey = cv.cvCreateImage (cv.cvGetSize (frame), 8, 1)
    pyramid = cv.cvCreateImage (cv.cvGetSize (frame), 8, 1)
    prev_pyramid = cv.cvCreateImage (cv.cvGetSize (frame), 8, 1)
    points = {[], []}
  endif

  ## copy the frame, so we can draw on it
  cv.cvCopy (frame, image)

  ## create a grey version of the image
  cv.cvCvtColor (image, grey, cv.CV_BGR2GRAY)

  if (night_mode)
    ## night mode: only display the points
    cv.cvSetZero (image)
  endif

  if (need_to_init)
    ## we want to search all the good points

    ## create the wanted images
    eig = cv.cvCreateImage (cv.cvGetSize (grey), 32, 1)
    temp = cv.cvCreateImage (cv.cvGetSize (grey), 32, 1)

    ## the default parameters
    quality = 0.01
    min_distance = 10

    ## search the good points
    points [1] = cv.cvGoodFeaturesToTrack (
					   grey, eig, temp,
					   MAX_COUNT,
					   quality, min_distance, None, 3, 0, 0.04)

    ## refine the corner locations
    cv.cvFindCornerSubPix (
			   grey,
			   points [1],
			   cv.cvSize (win_size, win_size), cv.cvSize (-1, -1),
			   cv.cvTermCriteria (cv.CV_TERMCRIT_ITER | cv.CV_TERMCRIT_EPS,
					      20, 0.03))

    ## release the temporary images
    cv.cvReleaseImage (eig)
    cv.cvReleaseImage (temp)
    
  else (len (points [0]) > 0)
    ## we have points, so display them

    ## calculate the optical flow
    [points [1], status] = cv.cvCalcOpticalFlowPyrLK (
						    prev_grey, grey, prev_pyramid, pyramid,
						    points [0], len (points [0]),
						    cv.cvSize (win_size, win_size), 3,
						    len (points [0]),
						    None,
						    cv.cvTermCriteria (cv.CV_TERMCRIT_ITER|cv.CV_TERMCRIT_EPS,
								       20, 0.03),
						    flags)

    ## initializations
    point_counter = -1
    new_points = []
    
    for the_point in points (1),
      ## go trough all the points

      ## increment the counter
      point_counter += 1
      
      if (add_remove_pt)
	## we have a point to add, so see if it is close to
	## another one. If yes, don't use it
        dx = pt.x - the_point.x
        dy = pt.y - the_point.y
        if (dx * dx + dy * dy <= 25)
	  ## too close
          add_remove_pt = 0
          continue
	endif
      endif

      if (!status (point_counter))
	## we will disable this point
        continue
      endif

      ## this point is a correct point
      new_points.append (the_point)
      
      ## draw the current point
      cv.cvCircle (image,
                   [the_point.x, the_point.y],
                   3, cv.cvScalar (0, 255, 0, 0),
                   -1, 8, 0)
    endfor

    ## set back the points we keep
    points [1] = new_points
  endif
  
  if (add_remove_pt)
    ## we want to add a point
    points [1].append (cv.cvPointTo32f (pt))

    ## refine the corner locations
    points [1][-1] = cv.cvFindCornerSubPix (
					    grey,
					    [points [1][-1]],
					    cv.cvSize (win_size, win_size), cv.cvSize (-1, -1),
					    cv.cvTermCriteria (cv.CV_TERMCRIT_ITER | cv.CV_TERMCRIT_EPS,
							       20, 0.03))[0]

    ## we are no more in "add_remove_pt" mode
    add_remove_pt = false
  endif

  ## swapping
  prev_grey, grey = grey, prev_grey
  prev_pyramid, pyramid = pyramid, prev_pyramid
  points [0], points [1] = points [1], points [0]
  need_to_init = false
  
  ## we can now display the image
  highgui.cvShowImage ('LkDemo', image)

  ## handle events
  c = highgui.cvWaitKey (10)

  if (c == '\x1b')
    ## user has press the ESC key, so exit
    break
  endif

  ## processing depending on the character
  if (c in ['r', 'R'])
    need_to_init = true
  elseif (c in ['c', 'C'])
    points = [[], []]
  elseif (c in ['n', 'N'])
    night_mode = not night_mode
  endif
endwhile
