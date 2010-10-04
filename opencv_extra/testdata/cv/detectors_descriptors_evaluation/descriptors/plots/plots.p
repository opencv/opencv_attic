set terminal png
set size 0.7, 0.7
set xlabel "1-precision"
set ylabel "recall"
set xr[0:1]
set yr[0:1]

set key box
set key left top

set title 'bark'
set output 'bark.png'
plot 'Calonder_bark.csv' title 'Calonder' with lines, 'FERN_bark.csv' title 'FERN' with lines, 'SIFT_bark.csv' title 'SIFT' with lines, 'SURF_bark.csv' title 'SURF' with lines

set title 'bikes'
set output 'bikes.png'
plot 'Calonder_bikes.csv' title 'Calonder' with lines, 'FERN_bikes.csv' title 'FERN' with lines, 'SIFT_bikes.csv' title 'SIFT' with lines, 'SURF_bikes.csv' title 'SURF' with lines

set title 'boat'
set output 'boat.png'
plot 'Calonder_boat.csv' title 'Calonder' with lines, 'FERN_boat.csv' title 'FERN' with lines, 'SIFT_boat.csv' title 'SIFT' with lines, 'SURF_boat.csv' title 'SURF' with lines

set title 'graf'
set output 'graf.png'
plot 'Calonder_graf.csv' title 'Calonder' with lines, 'FERN_graf.csv' title 'FERN' with lines, 'SIFT_graf.csv' title 'SIFT' with lines, 'SURF_graf.csv' title 'SURF' with lines

set title 'leuven'
set output 'leuven.png'
plot 'Calonder_leuven.csv' title 'Calonder' with lines, 'FERN_leuven.csv' title 'FERN' with lines, 'SIFT_leuven.csv' title 'SIFT' with lines, 'SURF_leuven.csv' title 'SURF' with lines

set title 'trees'
set output 'trees.png'
plot 'Calonder_trees.csv' title 'Calonder' with lines, 'FERN_trees.csv' title 'FERN' with lines, 'SIFT_trees.csv' title 'SIFT' with lines, 'SURF_trees.csv' title 'SURF' with lines

set title 'ubc'
set output 'ubc.png'
plot 'Calonder_ubc.csv' title 'Calonder' with lines, 'FERN_ubc.csv' title 'FERN' with lines, 'SIFT_ubc.csv' title 'SIFT' with lines, 'SURF_ubc.csv' title 'SURF' with lines

set title 'wall'
set output 'wall.png'
plot 'Calonder_wall.csv' title 'Calonder' with lines, 'FERN_wall.csv' title 'FERN' with lines, 'SIFT_wall.csv' title 'SIFT' with lines, 'SURF_wall.csv' title 'SURF' with lines
