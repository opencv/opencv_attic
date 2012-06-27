find_package(OpenCL)


if (OPENCL_FOUND)
	set(HAVE_OPENCL 1)
	message(STATUS "OpenCL detected.")
	message(STATUS "OpenCL include dir: " ${OPENCL_INCLUDE_DIR})
	message(STATUS "OpenCL lib dir: " ${OPENCL_LIBRARY})
    

  file(GLOB CL_FILES "${CMAKE_CURRENT_SOURCE_DIR}/modules/ocl/src/kernel/*.cl")
  file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/modules/ocl/kernels.cpp "namespace cv{ namespace ocl{\n")
  foreach(CL_FILENAME ${CL_FILES})
    string(REGEX REPLACE "${CMAKE_CURRENT_SOURCE_DIR}/modules/ocl/src/kernel/" "" dd "${CL_FILENAME}") 
    string(REGEX REPLACE ".cl" "" dd "${dd}") 
    #message("${dd}")
    file(APPEND ${CMAKE_CURRENT_BINARY_DIR}/modules/ocl/kernels.cpp "const char *  ${dd} = \n" )
    unset(kernel_char)
    file(READ "${CL_FILENAME}" kernel_char)
    #string(REPLACE "HAVE_STD_HEADERS_H" ${HAVE_STD_HEADERS_H} "${kernel_char}" "")
    string(REGEX REPLACE "[\r\n]" "\\\\n\"\n\"" kernel_char "${kernel_char}")
    set(kernel_char "\"" ${kernel_char} "\\n\"")
    file(APPEND ${CMAKE_CURRENT_BINARY_DIR}/modules/ocl/kernels.cpp "${kernel_char}" )
    file(APPEND ${CMAKE_CURRENT_BINARY_DIR}/modules/ocl/kernels.cpp ";\n")
  endforeach()
  file(APPEND ${CMAKE_CURRENT_BINARY_DIR}/modules/ocl/kernels.cpp "}}")

endif()
