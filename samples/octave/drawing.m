#! /usr/bin/env octave -q

printf("OpenCV Octave version of drawing\n");

# import the necessary things for OpenCV
cv
highgui

function ret=random_color (random)
  """
  Return a random color
  """
  icolor = random.randint (0, 0xFFFFFF)
  ret = cv.cvScalar (icolor & 0xff, (icolor >> 8) & 0xff, (icolor >> 16) & 0xff)
endfunction


# some "constants"
width = 1000
height = 700
window_name = "Drawing Demo"
number = 100
delay = 5
line_type = cv.CV_AA  # change it to 8 to see non-antialiased graphics

# create the source image
image = cv.cvCreateImage (cv.cvSize (width, height), 8, 3)

# create window and display the original picture in it
highgui.cvNamedWindow (window_name, 1)
cv.cvSetZero (image)
highgui.cvShowImage (window_name, image)

# create the random number
random = Random ()

# draw some lines
for i=0:number-1,
  pt1 = cv.cvPoint (random.randrange (-width, 2 * width),
                    random.randrange (-height, 2 * height))
  pt2 = cv.cvPoint (random.randrange (-width, 2 * width),
                    random.randrange (-height, 2 * height))
  cv.cvLine (image, pt1, pt2,
             random_color (random),
             random.randrange (0, 10),
             line_type, 0)
  
  highgui.cvShowImage (window_name, image)
  highgui.cvWaitKey (delay)
endfor

# draw some rectangles
for i=0:number-1,
  pt1 = cv.cvPoint (random.randrange (-width, 2 * width),
                    random.randrange (-height, 2 * height))
  pt2 = cv.cvPoint (random.randrange (-width, 2 * width),
                    random.randrange (-height, 2 * height))
  cv.cvRectangle (image, pt1, pt2,
                  random_color (random),
                  random.randrange (-1, 9),
                  line_type, 0)
  
  highgui.cvShowImage (window_name, image)
  highgui.cvWaitKey (delay)
endfor

# draw some ellipes
for i=0:number-1,
  pt1 = cv.cvPoint (random.randrange (-width, 2 * width),
                    random.randrange (-height, 2 * height))
  sz = cv.cvSize (random.randrange (0, 200),
                  random.randrange (0, 200))
  angle = random.randrange (0, 1000) * 0.180
  cv.cvEllipse (image, pt1, sz, angle, angle - 100, angle + 200,
                random_color (random),
                random.randrange (-1, 9),
                line_type, 0)
  
  highgui.cvShowImage (window_name, image)
  highgui.cvWaitKey (delay)
endfor

# init the list of polylines
nb_polylines = 2
polylines_size = 3
pt = [0,] * nb_polylines
for a=0:nb_polylines-1,
  pt [a] = [0,] * polylines_size
endfor

# draw some polylines
for i=0:number-1,
  for a=0:nb_polylines-1,
    for b=0:polylines_size-1,
      pt [a][b] = cv.cvPoint (random.randrange (-width, 2 * width),
                              random.randrange (-height, 2 * \
						height))
    endfor
    cv.cvPolyLine (image, pt, 1,
                   random_color (random),
                   random.randrange (1, 9),
                   line_type, 0)

    highgui.cvShowImage (window_name, image)
    highgui.cvWaitKey (delay)
  endfor
endfor

# draw some filled polylines
for i=0:number-1,
  for a=0:nb_polylines-1,
    for b=0:polylines_size-1,
      pt [a][b] = cv.cvPoint (random.randrange (-width, 2 * width),
                              random.randrange (-height, 2 * \
						height))
    endfor
    cv.cvFillPoly (image, pt,
                   random_color (random),
                   line_type, 0)

    highgui.cvShowImage (window_name, image)
    highgui.cvWaitKey (delay)
  endfor
endfor

# draw some circles
for i=0:number-1,
  pt1 = cv.cvPoint (random.randrange (-width, 2 * width),
                    random.randrange (-height, 2 * height))
  cv.cvCircle (image, pt1, random.randrange (0, 300),
               random_color (random),
               random.randrange (-1, 9),
               line_type, 0)
  
  highgui.cvShowImage (window_name, image)
  highgui.cvWaitKey (delay)
endfor

# draw some text
for i=0:number-1,
  pt1 = cv.cvPoint (random.randrange (-width, 2 * width),
                    random.randrange (-height, 2 * height))
  font = cv.cvInitFont (random.randrange (0, 8),
                        random.randrange (0, 100) * 0.05 + 0.01,
                        random.randrange (0, 100) * 0.05 + 0.01,
                        random.randrange (0, 5) * 0.1,
                        random.randrange (0, 10),
                        line_type)

  cv.cvPutText (image, "Testing text rendering!",
                pt1, font,
                random_color (random))
  
  highgui.cvShowImage (window_name, image)
  highgui.cvWaitKey (delay)
endfor

# prepare a text, and get it's properties
font = cv.cvInitFont (cv.CV_FONT_HERSHEY_COMPLEX,
                      3, 3, 0.0, 5, line_type)
text_size, ymin = cv.cvGetTextSize ("OpenCV forever!", font)
pt1.x = (width - text_size.width) / 2
pt1.y = (height + text_size.height) / 2
image2 = cv.cvCloneImage(image)

# now, draw some OpenCV pub ;-)
for i=0:255-1,
  cv.cvSubS (image2, cv.cvScalarAll (i), image, None)
  cv.cvPutText (image, "OpenCV forever!",
                pt1, font, cv.cvScalar (255, i, i))
  highgui.cvShowImage (window_name, image)
  highgui.cvWaitKey (delay)
endfor

# wait some key to end
highgui.cvWaitKey (0)
