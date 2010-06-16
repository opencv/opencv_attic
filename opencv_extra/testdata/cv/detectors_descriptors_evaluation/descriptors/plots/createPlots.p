set terminal png

set size 0.7, 0.7
set title "A perfomance evaluation of local descriptors"
set xlabel "1-precision"
set ylabel "recall"
set xr[0:1]
set yr[0:0.3]

set key box

set output 'bark.png'
plot "surf_bark.csv" title 'surf' with lines, "sift_bark.csv" title 'sift' with lines, "fern_bark.csv" title 'fern' with lines, "one_way_bark.csv" title 'one_way' with lines

set output 'bikes.png'
plot "surf_bikes.csv" title 'surf' with lines, "sift_bikes.csv" title 'sift' with lines, "fern_bikes.csv" title 'fern' with lines, "one_way_bikes.csv" title 'one_way' with lines

set output 'boat.png'
plot "surf_boat.csv" title 'surf' with lines, "sift_boat.csv" title 'sift' with lines, "fern_boat.csv" title 'fern' with lines, "one_way_boat.csv" title 'one_way' with lines

set output 'graf.png'
plot "surf_graf.csv" title 'surf' with lines, "sift_graf.csv" title 'sift' with lines, "fern_graf.csv" title 'fern' with lines, "one_way_graf.csv" title 'one_way' with lines

set output 'leuven.png'
plot "surf_leuven.csv" title 'surf' with lines, "sift_leuven.csv" title 'sift' with lines, "fern_leuven.csv" title 'fern' with lines, "one_way_leuven.csv" title 'one_way' with lines

set output 'trees.png'
plot "surf_trees.csv" title 'surf' with lines, "sift_trees.csv" title 'sift' with lines, "fern_trees.csv" title 'fern' with lines, "one_way_trees.csv" title 'one_way' with lines

set output 'ubc.png'
plot "surf_ubc.csv" title 'surf' with lines, "sift_ubc.csv" title 'sift' with lines, "fern_ubc.csv" title 'fern' with lines, "one_way_ubc.csv" title 'one_way' with lines

set output 'wall.png'
plot "surf_wall.csv" title 'surf' with lines, "sift_wall.csv" title 'sift' with lines, "fern_wall.csv" title 'fern' with lines, "one_way_wall.csv" title 'one_way' with lines


