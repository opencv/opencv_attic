# Will be modified later 
if(NOT HAVE_OPENCL)
  ocv_module_disable(ocl)
endif()

set(the_description "OpenCL-accelerated Computer Vision")
ocv_add_module(ocl opencv_core  opencv_imgproc opencv_calib3d opencv_objdetect opencv_video opencv_nonfree)

ocv_module_include_directories()

file(GLOB lib_hdrs     "include/opencv2/${name}/*.hpp" "include/opencv2/${name}/*.h")
file(GLOB lib_srcs     "src/*.cpp" "src/kernel/*.cpp" "${CMAKE_CURRENT_BINARY_DIR}/kernels.cpp")
file(GLOB lib_int_hdrs "src/*.h*" "src/kernel/*.h*")

source_group("Include"   FILES ${lib_hdrs})
source_group("Src\\Host" FILES ${lib_srcs} ${lib_int_hdrs})

if (HAVE_OPENCL)
  set(ocl_link_libs ${OPENCL_LIBRARIES})
  ocv_include_directories(${OPENCL_INCLUDE_DIR})
endif()

ocv_set_module_sources(
    HEADERS ${lib_hdrs}
    SOURCES ${lib_int_hdrs} ${lib_srcs}
    )
ocv_create_module(${ocl_link_libs})

install(FILES ${lib_hdrs}
    DESTINATION include/opencv2/${name}
    COMPONENT main)

ocv_add_precompiled_headers(${the_module})

#ADD_CUSTOM_COMMAND(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/kernels.cpp
#     COMMAND ${CMAKE_BINARY_DIR}/bin/streamlineCL ${CMAKE_CURRENT_SOURCE_DIR}/src/kernel > ${CMAKE_CURRENT_BINARY_DIR}/kernels.cpp
#)

################################################################################################################
################################      OpenCL Module Tests     ##################################################
################################################################################################################
file(GLOB test_srcs "test/*.cpp")
file(GLOB test_hdrs "test/*.hpp" "test/*.h")

ocv_add_accuracy_tests(FILES "Include" ${test_hdrs}
                       FILES "Src" ${test_srcs})
################################################################################################################
################################   OpenCL Module Performance  ##################################################
################################################################################################################
file(GLOB perf_srcs "perf/*.cpp")
file(GLOB perf_hdrs "perf/*.hpp" "perf/*.h")

ocv_add_perf_tests(FILES "Include" ${perf_hdrs}
                       FILES "Src" ${perf_srcs})
