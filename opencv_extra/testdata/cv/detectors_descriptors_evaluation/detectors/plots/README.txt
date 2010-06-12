This folder contains data (csv files) and script createPlots.p need to plot graphics of quality detectors. 2 metrics are supported: repeatability and correspondences count. Detectors are evaluated on each image dataset (8) independently. Each dataset is used to assess detectors quality under some transformation: rotation+zoom, viewpoint changes, blur, light changes or JPEG compression.

To plot the graphics run gnuplot on createPlots.p script, i.e. type
gnuplot createPlots.p

To add new detector evaluation in graphics modify createPlots.p.

