# ----------------------------------------------------------------------------
#  CMake file for C samples. See root CMakeLists.txt
#
# ----------------------------------------------------------------------------

project(c_samples)

if(CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-unused-function")
endif()

# ---------------------------------------------
#      Define executable targets
# ---------------------------------------------
MACRO(MY_DEFINE_EXAMPLE name srcs)
	add_executable(${name} ${srcs})
	add_dependencies(${name} cv cxcore highgui ml cvaux)
	target_link_libraries(${name} ${OPENCV_LINKER_LIBS} cv cxcore highgui ml cvaux)
ENDMACRO(MY_DEFINE_EXAMPLE)

MY_DEFINE_EXAMPLE(bgfg_codebook 	bgfg_codebook.cpp)
MY_DEFINE_EXAMPLE(bgfg_segm			bgfg_segm.cpp)
MY_DEFINE_EXAMPLE(blobtrack			blobtrack.cpp)
MY_DEFINE_EXAMPLE(camshiftdemo		camshiftdemo.c)
MY_DEFINE_EXAMPLE(contours			contours.c)
MY_DEFINE_EXAMPLE(convert_cascade	convert_cascade.c)
MY_DEFINE_EXAMPLE(convexhull		convexhull.c)
MY_DEFINE_EXAMPLE(delaunay			delaunay.c)
MY_DEFINE_EXAMPLE(demhist			demhist.c)
MY_DEFINE_EXAMPLE(dft				dft.c)
MY_DEFINE_EXAMPLE(distrans			distrans.c)
MY_DEFINE_EXAMPLE(drawing			drawing.c)
MY_DEFINE_EXAMPLE(edge				edge.c)
MY_DEFINE_EXAMPLE(facedetect		facedetect.c)
MY_DEFINE_EXAMPLE(ffilldemo			ffilldemo.c)
MY_DEFINE_EXAMPLE(find_obj			find_obj.cpp)
MY_DEFINE_EXAMPLE(fitellipse		fitellipse.cpp)
MY_DEFINE_EXAMPLE(houghlines		houghlines.c)
MY_DEFINE_EXAMPLE(image				image.cpp)
MY_DEFINE_EXAMPLE(inpaint			inpaint.cpp)
MY_DEFINE_EXAMPLE(kalman			kalman.c)
MY_DEFINE_EXAMPLE(kmeans			kmeans.c)
MY_DEFINE_EXAMPLE(laplace			laplace.c)
MY_DEFINE_EXAMPLE(letter_recog		letter_recog.cpp)
MY_DEFINE_EXAMPLE(lkdemo			lkdemo.c)
MY_DEFINE_EXAMPLE(minarea			minarea.c)
MY_DEFINE_EXAMPLE(morphology		morphology.c)
MY_DEFINE_EXAMPLE(motempl			motempl.c)
MY_DEFINE_EXAMPLE(mushroom			mushroom.cpp)
MY_DEFINE_EXAMPLE(pyramid_segmentation	pyramid_segmentation.c)
MY_DEFINE_EXAMPLE(squares			squares.c)
MY_DEFINE_EXAMPLE(stereo_calib		stereo_calib.cpp)
MY_DEFINE_EXAMPLE(watershed			watershed.cpp)

