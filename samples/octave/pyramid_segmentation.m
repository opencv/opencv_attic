#! /usr/bin/env octave -q

cv
highgui

image =  [None, None]
image0 = None
image1 = None
threshold1 = 255
threshold2 = 30
l = level = 4;
block_size = 1000;
filter = CV_GAUSSIAN_5x5;
storage = None
min_comp = CvConnectedComp()

function set_thresh1( val )
  global threshold1
  threshold1 = val
  ON_SEGMENT()
endfunction

function set_thresh2( val )
  global threshold2
  threshold2 = val
  ON_SEGMENT()
endfunction

function ON_SEGMENT()
  global storage
  global min_comp
  comp = cvPyrSegmentation(image0, image1, storage, level, threshold1+1, threshold2+1);
  cvShowImage("Segmentation", image1);
endfunction

filename = "../c/fruits.jpg";
if (length(argv) == 2)
  filename = sys.argv(1)
endif
image[0] = cvLoadImage( filename, 1)
if (! image[0])
  printf("Error opening %s\n",filename);
  exit(-1)
endif

cvNamedWindow("Source", 0);
cvShowImage("Source", image[0]);
cvNamedWindow("Segmentation", 0);
storage = cvCreateMemStorage ( block_size );
image[0].width &= -(1<<level);
image[0].height &= -(1<<level);
image0 = cvCloneImage( image[0] );
image1 = cvCloneImage( image[0] );
# segmentation of the color image
l = 1;
threshold1 =255;
threshold2 =30;
ON_SEGMENT();
sthreshold1 = cvCreateTrackbar("Threshold1", "Segmentation", threshold1, 255, set_thresh1);
sthreshold2 = cvCreateTrackbar("Threshold2", "Segmentation",  threshold2, 255, set_thresh2);
cvShowImage("Segmentation", image1);
cvWaitKey(0);
cvDestroyWindow("Segmentation");
cvDestroyWindow("Source");
