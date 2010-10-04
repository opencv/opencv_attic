set terminal png

set size 0.7, 0.7
#set title "A perfomance evaluation of local descriptors"
set xlabel "1-precision"
set ylabel "recall"
set xr[0:1]
set yr[0:1]

set key box
set key left top

set title "Bark"
set output 'bark.png'
plot "SURF_bark.csv" title 'SURF' with lines, "SIFT_bark.csv" title 'SIFT' with lines, "fern_bark.csv" title 'fern' with lines, "one_way_bark.csv" title 'one_way' with lines

set title "Bikes"
set output 'bikes.png'
plot "SURF_bikes.csv" title 'SURF' with lines, "SIFT_bikes.csv" title 'SIFT' with lines, "fern_bikes.csv" title 'fern' with lines, "one_way_bikes.csv" title 'one_way' with lines

set title "Boat"
set output 'boat.png'
plot "SURF_boat.csv" title 'SURF' with lines, "SIFT_boat.csv" title 'SIFT' with lines, "fern_boat.csv" title 'fern' with lines, "one_way_boat.csv" title 'one_way' with lines

set title "Graf"
set output 'graf.png'
plot "SURF_graf.csv" title 'SURF' with lines, "SIFT_graf.csv" title 'SIFT' with lines, "fern_graf.csv" title 'fern' with lines, "one_way_graf.csv" title 'one_way' with lines

set title "Leuven"
set output 'leuven.png'
plot "SURF_leuven.csv" title 'SURF' with lines, "SIFT_leuven.csv" title 'SIFT' with lines, "fern_leuven.csv" title 'fern' with lines, "one_way_leuven.csv" title 'one_way' with lines

set title "Trees"
set output 'trees.png'
plot "SURF_trees.csv" title 'SURF' with lines, "SIFT_trees.csv" title 'SIFT' with lines, "fern_trees.csv" title 'fern' with lines, "one_way_trees.csv" title 'one_way' with lines

set title "Ubc"
set output 'ubc.png'
plot "SURF_ubc.csv" title 'SURF' with lines, "SIFT_ubc.csv" title 'SIFT' with lines, "fern_ubc.csv" title 'fern' with lines, "one_way_ubc.csv" title 'one_way' with lines

set title "Wall"
set output 'wall.png'
plot "SURF_wall.csv" title 'SURF' with lines, "SIFT_wall.csv" title 'SIFT' with lines, "fern_wall.csv" title 'fern' with lines, "one_way_wall.csv" title 'one_way' with lines


