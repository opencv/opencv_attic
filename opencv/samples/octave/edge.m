
printf("OpenCV Octave version of edge\n");

# import the necessary things for OpenCV
cv
highgui

# some definitions
win_name = "Edge"
trackbar_name = "Threshold"

# the callback on the trackbar
function on_trackbar (position)

  cv.cvSmooth (gray, edge, cv.CV_BLUR, 3, 3, 0)
  cv.cvNot (gray, edge)

# run the edge dector on gray scale
  cv.cvCanny (gray, edge, position, position * 3, 3)

# reset
  cv.cvSetZero (col_edge)

# copy edge points
  cv.cvCopy (image, col_edge, edge)
  
# show the image
  highgui.cvShowImage (win_name, col_edge)
endfunction

filename = "../c/fruits.jpg"

if (length(argv)>1)
  filename = argv(1)
endif

# load the image gived on the command line
image = highgui.cvLoadImage (filename);

if (!image)
  print "Error loading image '%s'" % filename
  exit(-1)
endif

# create the output image
col_edge = cv.cvCreateImage (cv.cvSize (image.width, image.height), 8, 3)

# convert to grayscale
gray = cv.cvCreateImage (cv.cvSize (image.width, image.height), 8, 1)
edge = cv.cvCreateImage (cv.cvSize (image.width, image.height), 8, 1)
cv.cvCvtColor (image, gray, cv.CV_BGR2GRAY)

# create the window
highgui.cvNamedWindow (win_name, highgui.CV_WINDOW_AUTOSIZE)

# create the trackbar
highgui.cvCreateTrackbar (trackbar_name, win_name, 1, 100, on_trackbar)

# show the image
on_trackbar (0)

# wait a key pressed to end
highgui.cvWaitKey (0)
