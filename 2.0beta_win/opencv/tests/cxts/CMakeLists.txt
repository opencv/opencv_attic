# ----------------------------------------------------------------------------
#  CMake file for cxts. See root CMakeLists.txt
#
# ----------------------------------------------------------------------------
project(cxts)

file(GLOB lib_srcs "*.cpp")
source_group("Src" FILES ${lib_srcs})
file(GLOB lib_hdrs "*.h*")
source_group("Include" FILES ${lib_hdrs})

include_directories(.)

if(WIN32 AND MSVC)
    set(pch_header "_cxts.h")
    set(pch_src "precomp.cpp")
    list(REMOVE_ITEM lib_srcs ${CMAKE_CURRENT_SOURCE_DIR}/${pch_src})
    set(lib_srcs ${CMAKE_CURRENT_SOURCE_DIR}/${pch_src} ${lib_srcs})
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

# ----------------------------------------------------------------------------------
# 				Define the library target:
# ----------------------------------------------------------------------------------
set(the_target "cxts")

add_library(${the_target} SHARED ${lib_srcs} ${lib_hdrs})

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

add_dependencies(${the_target} cxcore)

# Add the required libraries for linking:
target_link_libraries(${the_target} ${OPENCV_LINKER_LIBS} cxcore)

if(WIN32)
install(TARGETS ${the_target}
    RUNTIME DESTINATION bin COMPONENT main
	LIBRARY DESTINATION lib COMPONENT main
	ARCHIVE DESTINATION lib COMPONENT main)
endif()
