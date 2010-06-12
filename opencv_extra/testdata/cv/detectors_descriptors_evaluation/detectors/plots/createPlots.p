set terminal png

set key box outside bottom

set size 1, 0.7
set title "Detectors evaluation under scale changes (bark dataset)"
set xlabel "dataset image index (increasing zoom+rotation)"
set ylabel "repeatability, %"
set output 'bark_repeatability.png'
set xr[2:6]
set yr[0:100]
plot "fast_bark_repeatability.csv" title 'fast' with linespoints, "gftt_bark_repeatability.csv" title 'gftt' with linespoints, "harris_bark_repeatability.csv" title 'harris' with linespoints,  "mser_bark_repeatability.csv" title 'mser' with linespoints, "star_bark_repeatability.csv" title 'star' with linespoints, "sift_bark_repeatability.csv" title 'sift' with linespoints, "surf_bark_repeatability.csv" title 'surf' with linespoints

set size 1, 1
set ylabel "correspondences count"
set output 'bark_correspondenceCount.png'
set yr[0:2000]
plot "fast_bark_correspondenceCount.csv" title 'fast' with linespoints,  "gftt_bark_correspondenceCount.csv" title 'gftt' with linespoints, "harris_bark_correspondenceCount.csv" title 'harris' with linespoints,  "mser_bark_correspondenceCount.csv" title 'mser' with linespoints, "star_bark_correspondenceCount.csv" title 'star' with linespoints, "sift_bark_correspondenceCount.csv" title 'sift' with linespoints, "surf_bark_correspondenceCount.csv" title 'surf' with linespoints

set size 1, 0.7
set title "Detectors evaluation under blur changes (bike dataset)"
set xlabel "dataset image index (increasing blur)"
set ylabel "repeatability, %"
set output 'bikes_repeatability.png'
set xr[2:6]
set yr[0:100]
plot "fast_bikes_repeatability.csv" title 'fast' with linespoints, "gftt_bikes_repeatability.csv" title 'gftt' with linespoints, "harris_bikes_repeatability.csv" title 'harris' with linespoints,  "mser_bikes_repeatability.csv" title 'mser' with linespoints, "star_bikes_repeatability.csv" title 'star' with linespoints, "sift_bikes_repeatability.csv" title 'sift' with linespoints, "surf_bikes_repeatability.csv" title 'surf' with linespoints

set size 1, 1
set ylabel "correspondences count"
set output 'bikes_correspondenceCount.png'
set yr[0:1200]
plot "fast_bikes_correspondenceCount.csv" title 'fast' with linespoints,  "gftt_bikes_correspondenceCount.csv" title 'gftt' with linespoints, "harris_bikes_correspondenceCount.csv" title 'harris' with linespoints,  "mser_bikes_correspondenceCount.csv" title 'mser' with linespoints, "star_bikes_correspondenceCount.csv" title 'star' with linespoints, "sift_bikes_correspondenceCount.csv" title 'sift' with linespoints, "surf_bikes_correspondenceCount.csv" title 'surf' with linespoints

set size 1, 0.7
set title "Detectors evaluation under size changes (boat dataset)"
set xlabel "dataset image index (increasing zoom+rotation)"
set ylabel "repeatability, %"
set output 'boat_repeatability.png'
set xr[2:6]
set yr[0:100]
plot "fast_boat_repeatability.csv" title 'fast' with linespoints, "gftt_boat_repeatability.csv" title 'gftt' with linespoints, "harris_boat_repeatability.csv" title 'harris' with linespoints,  "mser_boat_repeatability.csv" title 'mser' with linespoints, "star_boat_repeatability.csv" title 'star' with linespoints, "sift_boat_repeatability.csv" title 'sift' with linespoints, "surf_boat_repeatability.csv" title 'surf' with linespoints

set size 1, 1
set ylabel "correspondences count"
set output 'boat_correspondenceCount.png'
set yr[0:3500]
plot "fast_boat_correspondenceCount.csv" title 'fast' with linespoints,  "gftt_boat_correspondenceCount.csv" title 'gftt' with linespoints, "harris_boat_correspondenceCount.csv" title 'harris' with linespoints,  "mser_boat_correspondenceCount.csv" title 'mser' with linespoints, "star_boat_correspondenceCount.csv" title 'star' with linespoints, "sift_boat_correspondenceCount.csv" title 'sift' with linespoints, "surf_boat_correspondenceCount.csv" title 'surf' with linespoints

set size 1, 0.7
set title "Detectors evaluation under viewpoint changes (graf dataset)"
set xlabel "viewpoint angle"
set ylabel "repeatability, %"
set output 'graf_repeatability.png'
set xr[20:60]
set yr[0:100]
plot "fast_graf_repeatability.csv" title 'fast' with linespoints, "gftt_graf_repeatability.csv" title 'gftt' with linespoints, "harris_graf_repeatability.csv" title 'harris' with linespoints,  "mser_graf_repeatability.csv" title 'mser' with linespoints, "star_graf_repeatability.csv" title 'star' with linespoints, "sift_graf_repeatability.csv" title 'sift' with linespoints, "surf_graf_repeatability.csv" title 'surf' with linespoints

set size 1, 1
set ylabel "correspondences count"
set output 'graf_correspondenceCount.png'
set yr[0:2000]
plot "fast_graf_correspondenceCount.csv" title 'fast' with linespoints,  "gftt_graf_correspondenceCount.csv" title 'gftt' with linespoints, "harris_graf_correspondenceCount.csv" title 'harris' with linespoints,  "mser_graf_correspondenceCount.csv" title 'mser' with linespoints, "star_graf_correspondenceCount.csv" title 'star' with linespoints, "sift_graf_correspondenceCount.csv" title 'sift' with linespoints, "surf_graf_correspondenceCount.csv" title 'surf' with linespoints

set size 1, 0.7
set title "Detectors evaluation under light changes (leuven dataset)"
set xlabel "dataset image index (decreasing light)"
set ylabel "repeatability, %"
set output 'leuven_repeatability.png'
set xr[2:6]
set yr[0:100]
plot "fast_leuven_repeatability.csv" title 'fast' with linespoints, "gftt_leuven_repeatability.csv" title 'gftt' with linespoints, "harris_leuven_repeatability.csv" title 'harris' with linespoints,  "mser_leuven_repeatability.csv" title 'mser' with linespoints, "star_leuven_repeatability.csv" title 'star' with linespoints, "sift_leuven_repeatability.csv" title 'sift' with linespoints, "surf_leuven_repeatability.csv" title 'surf' with linespoints

set size 1, 1
set ylabel "correspondences count"
set output 'leuven_correspondenceCount.png'
set yr[0:1500]
plot "fast_leuven_correspondenceCount.csv" title 'fast' with linespoints,  "gftt_leuven_correspondenceCount.csv" title 'gftt' with linespoints, "harris_leuven_correspondenceCount.csv" title 'harris' with linespoints,  "mser_leuven_correspondenceCount.csv" title 'mser' with linespoints, "star_leuven_correspondenceCount.csv" title 'star' with linespoints, "sift_leuven_correspondenceCount.csv" title 'sift' with linespoints, "surf_leuven_correspondenceCount.csv" title 'surf' with linespoints

set size 1, 0.7
set title "Detectors evaluation under blur changes (trees)"
set xlabel "dataset image index (increasing blur)"
set ylabel "repeatability, %"
set output 'trees_repeatability.png'
set xr[2:6]
set yr[0:100]
plot "fast_trees_repeatability.csv" title 'fast' with linespoints, "gftt_trees_repeatability.csv" title 'gftt' with linespoints, "harris_trees_repeatability.csv" title 'harris' with linespoints,  "mser_trees_repeatability.csv" title 'mser' with linespoints, "star_trees_repeatability.csv" title 'star' with linespoints, "sift_trees_repeatability.csv" title 'sift' with linespoints, "surf_trees_repeatability.csv" title 'surf' with linespoints

set size 1, 1
set ylabel "correspondences count"
set output 'trees_correspondenceCount.png'
set yr[0:6000]
plot "fast_trees_correspondenceCount.csv" title 'fast' with linespoints,  "gftt_trees_correspondenceCount.csv" title 'gftt' with linespoints, "harris_trees_correspondenceCount.csv" title 'harris' with linespoints,  "mser_trees_correspondenceCount.csv" title 'mser' with linespoints, "star_trees_correspondenceCount.csv" title 'star' with linespoints, "sift_trees_correspondenceCount.csv" title 'sift' with linespoints, "surf_trees_correspondenceCount.csv" title 'surf' with linespoints

set size 1, 0.7
set title "Detectors evaluation under JPEG compression (ubc dataset)"
set xlabel "JPEG compression, %"
set ylabel "repeatability, %"
set output 'ubc_repeatability.png'
set xr[60:98]
set yr[0:100]
plot "fast_ubc_repeatability.csv" title 'fast' with linespoints, "gftt_ubc_repeatability.csv" title 'gftt' with linespoints, "harris_ubc_repeatability.csv" title 'harris' with linespoints,  "mser_ubc_repeatability.csv" title 'mser' with linespoints, "star_ubc_repeatability.csv" title 'star' with linespoints, "sift_ubc_repeatability.csv" title 'sift' with linespoints, "surf_ubc_repeatability.csv" title 'surf' with linespoints

set size 1, 1
set ylabel "correspondences count"
set output 'ubc_correspondenceCount.png'
set yr[0:3000]
plot "fast_ubc_correspondenceCount.csv" title 'fast' with linespoints,  "gftt_ubc_correspondenceCount.csv" title 'gftt' with linespoints, "harris_ubc_correspondenceCount.csv" title 'harris' with linespoints,  "mser_ubc_correspondenceCount.csv" title 'mser' with linespoints, "star_ubc_correspondenceCount.csv" title 'star' with linespoints, "sift_ubc_correspondenceCount.csv" title 'sift' with linespoints, "surf_ubc_correspondenceCount.csv" title 'surf' with linespoints

set size 1, 0.7
set title "Detectors evaluation under viewpoint changes (wall dataset)"
set xlabel "viewpoint angle"
set ylabel "repeatability, %"
set output 'wall_repeatability.png'
set xr[20:60]
set yr[0:100]
plot "fast_wall_repeatability.csv" title 'fast' with linespoints, "gftt_wall_repeatability.csv" title 'gftt' with linespoints, "harris_wall_repeatability.csv" title 'harris' with linespoints,  "mser_wall_repeatability.csv" title 'mser' with linespoints, "star_wall_repeatability.csv" title 'star' with linespoints, "sift_wall_repeatability.csv" title 'sift' with linespoints, "surf_wall_repeatability.csv" title 'surf' with linespoints

set size 1, 1
set ylabel "correspondences count"
set output 'wall_correspondenceCount.png'
set yr[0:5000]
plot "fast_wall_correspondenceCount.csv" title 'fast' with linespoints,  "gftt_wall_correspondenceCount.csv" title 'gftt' with linespoints, "harris_wall_correspondenceCount.csv" title 'harris' with linespoints,  "mser_wall_correspondenceCount.csv" title 'mser' with linespoints, "star_wall_correspondenceCount.csv" title 'star' with linespoints, "sift_wall_correspondenceCount.csv" title 'sift' with linespoints, "surf_wall_correspondenceCount.csv" title 'surf' with linespoints

