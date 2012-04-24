# ----------------------------------------------------------------------------
#  CMake file for cvtest. See root CMakeLists.txt
#
# ----------------------------------------------------------------------------
project(cvtest)

file(GLOB test_srcs "src/*.cpp")
source_group("Src" FILES ${test_srcs})
file(GLOB test_hdrs "src/*.h*")
source_group("Include" FILES ${test_hdrs})

include_directories("${CMAKE_SOURCE_DIR}/include/opencv"
                    "${CMAKE_CURRENT_SOURCE_DIR}/src"
                    "${CMAKE_CURRENT_BINARY_DIR}")

include_directories(../cxts)

set(the_target "cvtest")

add_executable(${the_target} ${test_srcs} ${test_hdrs})

if(PCHSupport_FOUND)
    set(pch_header ${CMAKE_SOURCE_DIR}/tests/cv/src/cvtest.h)
    if(${CMAKE_GENERATOR} MATCHES "Visual*" OR ${CMAKE_GENERATOR} MATCHES "Xcode*")
        if(${CMAKE_GENERATOR} MATCHES "Visual*")
            set(${the_target}_pch "src/cvtest.cpp")
        endif()            
        add_native_precompiled_header(${the_target} ${pch_header})
    elseif(CMAKE_COMPILER_IS_GNUCXX AND ${CMAKE_GENERATOR} MATCHES ".*Makefiles")
        add_precompiled_header(${the_target} ${pch_header})
    endif()
endif()

# Additional target properties
set_target_properties(${the_target} PROPERTIES
	DEBUG_POSTFIX "${OPENCV_DEBUG_POSTFIX}"
	RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/"
	)


add_dependencies(${the_target} cxts cxcore cv cvaux highgui)

# Add the required libraries for linking:
target_link_libraries(${the_target} ${OPENCV_LINKER_LIBS} cxcore cv cvaux highgui cxts)

enable_testing()
get_target_property(LOC ${the_target} LOCATION)
add_test(cvtest "${LOC}")

if(WIN32)
install(TARGETS ${the_target} RUNTIME DESTINATION bin COMPONENT main)
endif()
