
set(ENV_OPENCLROOT $ENV{CUDA_PATH}) 
if(ENV_OPENCLROOT)
  find_path(
    OPENCL_INCLUDE_DIR
    NAMES CL/cl.h OpenCL/cl.h
    PATHS $ENV{CUDA_PATH}/include
    NO_DEFAULT_PATH
    )

    if(CMAKE_SIZEOF_VOID_P EQUAL 4)
      set(
        OPENCL_LIB_SEARCH_PATH
        ${OPENCL_LIB_SEARCH_PATH}
       	$ENV{CUDA_PATH}/lib/Win32
        )
    else(CMAKE_SIZEOF_VOID_P EQUAL 4)
      set(
        OPENCL_LIB_SEARCH_PATH
        ${OPENCL_LIB_SEARCH_PATH}
		$ENV{CUDA_PATH}/lib/x64
        )
    endif(CMAKE_SIZEOF_VOID_P EQUAL 4)
 
  find_library(
    OPENCL_LIBRARY
    NAMES OpenCL
    PATHS ${OPENCL_LIB_SEARCH_PATH}
    NO_DEFAULT_PATH
    )
else(ENV_OPENCLROOT)
  find_path(
    OPENCL_INCLUDE_DIR
    NAMES CL/cl.h OpenCL/cl.h
    )

  find_library(
    OPENCL_LIBRARY
    NAMES OpenCL
    )
endif(ENV_OPENCLROOT)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  OPENCL
  DEFAULT_MSG
  OPENCL_LIBRARY OPENCL_INCLUDE_DIR
  )

if(OPENCL_FOUND)
  set(OPENCL_LIBRARIES ${OPENCL_LIBRARY})
else(OPENCL_FOUND)
  set(OPENCL_LIBRARIES)
endif(OPENCL_FOUND)


mark_as_advanced(
  OPENCL_INCLUDE_DIR
  OPENCL_LIBRARY
  )

