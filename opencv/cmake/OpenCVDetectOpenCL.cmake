	find_package(OpenCL REQUIRED)

	if (OPENCL_FOUND)
		set(HAVE_OPENCL 1)
		message(STATUS "OpenCL detected.")
		message(STATUS "OpenCL include dir: " ${OPENCL_INCLUDE_DIR})
		message(STATUS "OpenCL lib dir: " ${OPENCL_LIBRARY})
	endif()
