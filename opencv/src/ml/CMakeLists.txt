# ----------------------------------------------------------------------------
#  CMake file for ml. See root CMakeLists.txt
# ----------------------------------------------------------------------------
project(ml)

file(GLOB lib_srcs "*.cpp")
source_group("Src" FILES ${lib_srcs})
set(lib_hdrs ${CMAKE_SOURCE_DIR}/include/opencv/ml.h)
source_group("Include\\External" FILES ${lib_hdrs})
file(GLOB lib_int_hdrs "*.h*")
source_group("Include\\Internal" FILES ${lib_int_hdrs})

if(WIN32 AND MSVC)
    set(pch_header "_ml.h")
    set(pch_src "ml.cpp")
    foreach(src_file ${lib_srcs})
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

set(the_target "ml")

add_library(${the_target} SHARED ${lib_srcs} ${lib_hdrs} ${lib_int_hdrs})

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
	DEFINE_SYMBOL "CVAPI_EXPORTS"
	)

# Add the required libraries for linking:
target_link_libraries(${the_target} ${OPENCV_LINKER_LIBS} cxcore)

# Dependencies of this target:
add_dependencies(${the_target} cxcore)

install(TARGETS ${the_target}
	RUNTIME DESTINATION bin COMPONENT dev
	LIBRARY DESTINATION lib COMPONENT dev
	ARCHIVE DESTINATION lib COMPONENT dev)

install(FILES ${lib_hdrs}
        DESTINATION include/opencv
        COMPONENT dev)

