set(ENV_AMDAPPSDKROOT $ENV{AMDAPPSDKROOT})
if(ENV_AMDAPPSDKROOT)
  find_path(
    OPENCL_INCLUDE_DIR
    NAMES CL/cl.h OpenCL/cl.h
    PATHS $ENV{AMDAPPSDKROOT}/include
    NO_DEFAULT_PATH
    )

  
    if(CMAKE_SIZEOF_VOID_P EQUAL 4)
      set(
        OPENCL_LIB_SEARCH_PATH
        ${OPENCL_LIB_SEARCH_PATH}
        $ENV{AMDAPPSDKROOT}/lib/x86
        )
    else(CMAKE_SIZEOF_VOID_P EQUAL 4)
      set(
        OPENCL_LIB_SEARCH_PATH
        ${OPENCL_LIB_SEARCH_PATH}
        $ENV{AMDAPPSDKROOT}/lib/x86_64
        )
    endif(CMAKE_SIZEOF_VOID_P EQUAL 4)
  
  find_library(
    OPENCL_LIBRARY
    NAMES OpenCL
    PATHS ${OPENCL_LIB_SEARCH_PATH}
    NO_DEFAULT_PATH
    )
else(ENV_AMDAPPSDKROOT)
  find_path(
    OPENCL_INCLUDE_DIR
    NAMES CL/cl.h OpenCL/cl.h
    )

  find_library(
    OPENCL_LIBRARY
    NAMES OpenCL
    )
endif(ENV_AMDAPPSDKROOT)

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
