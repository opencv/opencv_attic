# Will be modified later 
if(NOT HAVE_OPENCL)
  ocv_module_disable(ocl)
endif()

set(the_description "OpenCL-accelerated Computer Vision")
ocv_add_module(ocl opencv_core  opencv_imgproc opencv_calib3d opencv_objdetect opencv_video opencv_nonfree)

ocv_module_include_directories()

file(GLOB lib_hdrs     "include/opencv2/${name}/*.hpp" "include/opencv2/${name}/*.h")
file(GLOB lib_srcs     "src/*.cpp" "src/kernel/*.cpp")
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
if(UNIX)
add_custom_command(TARGET ${the_module}
PRE_BUILD
COMMAND ../../../modules/ocl/streamlineCL
)
else()
add_custom_command(TARGET ${the_module}
PRE_BUILD
COMMAND  cd ../../../modules/ocl
COMMAND streamlineCL
)
endif()


################################################################################################################
################################      OpenCL Module Tests     ##################################################
################################################################################################################
file(GLOB test_srcs "test/*.cpp")
file(GLOB test_hdrs "test/*.hpp" "test/*.h")

ocv_add_accuracy_tests(FILES "Include" ${test_hdrs}
                       FILES "Src" ${test_srcs})
