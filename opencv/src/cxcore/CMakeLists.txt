# ----------------------------------------------------------------------------
#  CMake file for cxcore. See root CMakeLists.txt 
# ----------------------------------------------------------------------------
project(cxcore)

file(GLOB lib_srcs "src/*.cpp")
source_group("Src" FILES ${lib_srcs})
file(GLOB lib_hdrs "include/*.h*")
source_group("Include\\External" FILES ${lib_hdrs})
file(GLOB lib_int_hdrs "src/*.h*")
source_group("Include\\Internal" FILES ${lib_int_hdrs})

if(WIN32 AND MSVC)
    set(pch_header "_cxcore.h")
    set(pch_src "cxprecomp.cpp")
    foreach(src_file ${lib_srcs})
        if(${src_file} MATCHES "dummy.cpp")
            set(dummy ${src_file})
        elseif(${src_file} MATCHES ${pch_src})
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
    list(REMOVE_ITEM lib_srcs ${dummy})
endif()

set(the_target "cxcore")

add_library(${the_target} SHARED ${lib_srcs} ${lib_hdrs} ${lib_int_hdrs})

# For dynamic link numbering convenions
set_target_properties(${the_target} PROPERTIES
    VERSION ${OPENCV_VERSION}
    SOVERSION ${OPENCV_SOVERSION}
    OUTPUT_NAME "${the_target}${OPENCV_DLLVERSION}"
    )

# Additional target properties
set_target_properties(${the_target} PROPERTIES
	DEBUG_POSTFIX "d"
	ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib/"
	RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/"
	DEFINE_SYMBOL "CVAPI_EXPORTS"
	)

# Add the required libraries for linking:
target_link_libraries(${the_target} ${OPENCV_LINKER_LIBS})

install(TARGETS ${the_target}
	RUNTIME DESTINATION bin
	LIBRARY DESTINATION lib
	ARCHIVE DESTINATION lib
    )

if(UNIX)
    install(FILES ${lib_hdrs} DESTINATION "${CMAKE_INSTALL_PREFIX}/include/opencv")
endif()
    
