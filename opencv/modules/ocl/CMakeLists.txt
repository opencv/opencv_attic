include_directories(${OPENCL_INCLUDE_DIRS})
message(STATUS "OPENCL_INCLUDE_DIRS: ${OPENCL_INCLUDE_DIRS}")
message(STATUS "OPENCL_LIBRARIES: ${OPENCL_LIBRARIES}")

define_opencv_module(ocl opencv_core opencv_imgproc opencv_highgui ${OPENCL_LIBRARIES})
