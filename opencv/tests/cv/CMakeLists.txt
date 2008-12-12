# ----------------------------------------------------------------------------
#  CMake file for cvtest. See root CMakeLists.txt
#
# ----------------------------------------------------------------------------
project(cvtest)

file(GLOB test_srcs "src/*.cpp")
source_group("Src" FILES ${test_srcs})
file(GLOB test_hdrs "src/*.h*")
source_group("Include" FILES ${test_hdrs})

if(WIN32 AND MSVC)
    set(pch_header "cvtest.h")
    set(pch_src "cvtest.cpp")
    foreach(src_file ${test_srcs})
        if(${src_file} MATCHES ${pch_src})
            set_source_files_properties(
                ${src_file}
                PROPERTIES
                COMPILE_FLAGS "/Yc${pch_header}"
                )
        else()
            set_source_files_properties(
                ${src_file}
                PROPERTIES
                COMPILE_FLAGS "/Yu${pch_header}"
                )
        endif()
    endforeach()
endif()

include_directories(../cxts)

set(the_target "cvtest")

add_executable(${the_target} ${test_srcs} ${test_hdrs})

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
