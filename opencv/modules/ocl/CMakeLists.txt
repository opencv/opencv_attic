set(name "ocl")
set(DEPS "opencv_core" "opencv_imgproc" "opencv_objdetect" "opencv_features2d" "opencv_flann" "opencv_calib3d")

set(OPENCV_LINKER_LIBS ${OPENCV_LINKER_LIBS} opencv_ocl)

set(the_target "opencv_${name}")

project(${the_target})

add_definitions(-DCVAPI_EXPORTS)

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/include"
					"${CMAKE_CURRENT_SOURCE_DIR}/src/kernel"
					"${CMAKE_CURRENT_SOURCE_DIR}/src"
					"${CMAKE_CURRENT_BINARY_DIR}")

foreach(d ${DEPS})
	if(${d} MATCHES "opencv_")
		string(REPLACE "opencv_" "${CMAKE_CURRENT_SOURCE_DIR}/../" d_dir ${d})
		include_directories("${d_dir}/include")
	endif()
endforeach()

file(GLOB lib_srcs "src/*.cpp" "src/kernel/*.cpp")
file(GLOB lib_int_hdrs "src/*.h*" "src/kernel/*.h*")
source_group("Src\\Host" FILES ${lib_srcs} ${lib_int_hdrs})

file(GLOB lib_hdrs "include/opencv2/${name}/*.h*")
source_group("Include" FILES ${lib_hdrs})

if (HAVE_OPENCL)
  include_directories(${OPENCL_INCLUDE_DIR})
endif()

add_library(${the_target} ${lib_srcs} ${lib_hdrs} ${lib_int_hdrs} ${lib_cuda} ${lib_cuda_hdrs} ${cuda_objs})

if(PCHSupport_FOUND)
	set(pch_header ${CMAKE_CURRENT_SOURCE_DIR}/src/precomp.hpp)
	if(${CMAKE_GENERATOR} MATCHES "Visual*" OR ${CMAKE_GENERATOR} MATCHES "Xcode*")
		if(${CMAKE_GENERATOR} MATCHES "Visual*")
			set(${the_target}_pch "src/precomp.cpp")
		endif()
		add_native_precompiled_header(${the_target} ${pch_header})
	elseif(CMAKE_COMPILER_IS_GNUCXX AND ${CMAKE_GENERATOR} MATCHES ".*Makefiles")
		add_precompiled_header(${the_target} ${pch_header})
	endif()
endif()

# For dynamic link numbering convenions
set_target_properties(${the_target} PROPERTIES
	VERSION ${OPENCV_VERSION}
	SOVERSION ${OPENCV_SOVERSION}
	OUTPUT_NAME "${the_target}${OPENCV_DLLVERSION}"
	)

# Additional target properties
set_target_properties(${the_target} PROPERTIES
	DEBUG_POSTFIX "${OPENCV_DEBUG_POSTFIX}"
	ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib/"
	RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/"
	INSTALL_NAME_DIR "${CMAKE_INSTALL_PREFIX}/lib"
	)

# Add the required libraries for linking:
target_link_libraries(${the_target} ${OPENCV_LINKER_LIBS} ${IPP_LIBS} ${DEPS})

if (HAVE_OPENCL)
	target_link_libraries(${the_target} ${OPENCL_LIBRARIES})
endif()

if(MSVC)
	if(CMAKE_CROSSCOMPILING)
		set_target_properties(${the_target} PROPERTIES
			LINK_FLAGS "/NODEFAULTLIB:secchk"
			)
	endif()
	set_target_properties(${the_target} PROPERTIES
		LINK_FLAGS "/NODEFAULTLIB:libc"
		)
endif()

# Dependencies of this target:
add_dependencies(${the_target} ${DEPS})

install(TARGETS ${the_target}
	RUNTIME DESTINATION bin COMPONENT main
	LIBRARY DESTINATION lib COMPONENT main
	ARCHIVE DESTINATION lib COMPONENT main)

install(FILES ${lib_hdrs}
	DESTINATION include/opencv2/${name}
	COMPONENT main)


