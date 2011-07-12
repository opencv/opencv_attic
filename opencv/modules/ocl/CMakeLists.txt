set(name "ocl")

set(the_target "opencv_${name}")
project(${the_target})

set(DEPS "opencv_core" "opencv_imgproc" "opencv_objdetect" "opencv_features2d" "opencv_flann" "opencv_calib3d") #"opencv_features2d" "opencv_flann" "opencv_objdetect" - only headers needed 
set(OPENCV_LINKER_LIBS ${OPENCV_LINKER_LIBS} opencv_ocl)

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/include"                    
					"${CMAKE_CURRENT_SOURCE_DIR}/src/ocl"                    
					"${CMAKE_CURRENT_SOURCE_DIR}/src"
					"${CMAKE_CURRENT_BINARY_DIR}")

file(GLOB lib_srcs "src/*.cpp")
file(GLOB lib_int_hdrs "src/*.h*")
file(GLOB lib_ocl "src/ocl/*.cl*")
file(GLOB lib_ocl_hdrs "src/ocl/*.h*")
source_group("Src\\Host" FILES ${lib_srcs} ${lib_int_hdrs})
source_group("Src\\Ocl" FILES ${lib_ocl} ${lib_ocl_hdrs})

file(GLOB lib_hdrs "include/opencv2/${name}/*.h*")
source_group("Include" FILES ${lib_hdrs})

if (HAVE_OPENCL)
    file(GLOB_RECURSE ocl_srcs "src/ocl/*.cpp")	
    file(GLOB_RECURSE ocl_opencl "src/ocl/*.cl")
    file(GLOB_RECURSE ocl_hdrs "src/ocl/*.hpp" "src/ocl/*.h")
    
endif()

if (HAVE_OPENCL)       
    include_directories(${OPENCL_INCLUDE_DIRS})


    string(REPLACE "/W4" "/W3" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    string(REPLACE "/W4" "/W3" CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}")
    string(REPLACE "/W4" "/W3" CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")

    if(MSVC)
        #string(REPLACE "/W4" "/W3" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
        #string(REPLACE "/W4" "/W3" CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}")
        #string(REPLACE "/W4" "/W3" CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4211 /wd4201 /wd4100 /wd4505 /wd4408")

        string(REPLACE "/EHsc-" "/EHs" CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
        string(REPLACE "/EHsc-" "/EHs" CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE}")
        string(REPLACE "/EHsc-" "/EHs" CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG}")
        string(REPLACE "/EHsc-" "/EHs" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
        string(REPLACE "/EHsc-" "/EHs" CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}")
        string(REPLACE "/EHsc-" "/EHs" CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
    endif()
	
 endif()

foreach(d ${DEPS})
	if(${d} MATCHES "opencv_")
		string(REPLACE "opencv_" "${CMAKE_CURRENT_SOURCE_DIR}/../" d_dir ${d})
		include_directories("${d_dir}/include")
	endif()
endforeach()

add_library(${the_target} ${lib_srcs} ${lib_hdrs} ${lib_int_hdrs} ${lib_ocl} ${lib_ocl_hdrs})

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

if(ENABLE_SOLUTION_FOLDERS)
	set_target_properties(${the_target} PROPERTIES FOLDER "modules")
endif()	
	
if (BUILD_SHARED_LIBS)
	if (MSVC)
		set_target_properties(${the_target} PROPERTIES DEFINE_SYMBOL CVAPI_EXPORTS)
	else()
		add_definitions(-DCVAPI_EXPORTS)	   
	endif()	
endif()

# Additional target properties
set_target_properties(${the_target} PROPERTIES
	DEBUG_POSTFIX "${OPENCV_DEBUG_POSTFIX}"
    ARCHIVE_OUTPUT_DIRECTORY ${LIBRARY_OUTPUT_PATH}
    RUNTIME_OUTPUT_DIRECTORY ${EXECUTABLE_OUTPUT_PATH}
    INSTALL_NAME_DIR lib
	)

# Add the required libraries for linking:
target_link_libraries(${the_target} ${OPENCV_LINKER_LIBS} ${DEPS} )

if (HAVE_OPENCL)
	target_link_libraries(${the_target} ${OPENCL_LIBRARIES})    
endif()

if(MSVC)
	if(CMAKE_CROSSCOMPILING)
		set_target_properties(${the_target} PROPERTIES LINK_FLAGS "/NODEFAULTLIB:secchk")
	endif()
    
	set_target_properties(${the_target} PROPERTIES LINK_FLAGS "/NODEFAULTLIB:libc")
endif()

# Dependencies of this target:
add_dependencies(${the_target} ${DEPS})

install(TARGETS ${the_target}
	RUNTIME DESTINATION bin COMPONENT main
	LIBRARY DESTINATION lib COMPONENT main
	ARCHIVE DESTINATION lib COMPONENT main)

install(FILES ${lib_hdrs}
	DESTINATION ${OPENCV_INCLUDE_PREFIX}/opencv2/${name}
	COMPONENT main)