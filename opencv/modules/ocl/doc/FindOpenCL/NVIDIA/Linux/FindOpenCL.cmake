set(ENV_OPENCLROOT $ENV{OPENCLROOT})
if(ENV_OPENCLROOT)
  find_path(
    OPENCL_INCLUDE_DIR
    NAMES CL/cl.h OpenCL/cl.h
    PATHS $ENV{OPENCLROOT}/inc
    NO_DEFAULT_PATH
    )

  if("${CMAKE_SYSTEM_NAME}" MATCHES "Linux")
    if(CMAKE_SIZEOF_VOID_P EQUAL 4)
      set(
        OPENCL_LIB_SEARCH_PATH
        ${OPENCL_LIB_SEARCH_PATH}
       	/usr/lib/
        )
    else(CMAKE_SIZEOF_VOID_P EQUAL 4)
      set(
        OPENCL_LIB_SEARCH_PATH
        ${OPENCL_LIB_SEARCH_PATH}
				/usr/lib64/
        )
    endif(CMAKE_SIZEOF_VOID_P EQUAL 4)
  endif("${CMAKE_SYSTEM_NAME}" MATCHES "Linux")
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
