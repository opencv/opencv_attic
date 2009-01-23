# ----------------------------------------------------------------------------
#  CMake file for cvaux. See root CMakeLists.txt
#
# ----------------------------------------------------------------------------
project(cvaux)

file(GLOB aux_srcs *.cpp)
file(GLOB aux_srcs_vs vs/*.cpp)
set(lib_srcs ${aux_srcs_vs} ${aux_srcs})
source_group(Src FILES ${aux_srcs})
source_group(Src\\VideoSurveillance FILES ${aux_srcs_vs})

set(aux_hdr_names cvaux.h cvaux.hpp cvmat.hpp cvvidsurv.hpp)
set(aux_hdrs_ext)
foreach(h ${aux_hdr_names})
    list(APPEND aux_hdrs_ext "${CMAKE_SOURCE_DIR}/include/opencv/${h}")
endforeach()

file(GLOB_RECURSE aux_hdrs_int *.h*)
set(lib_hdrs ${lib_hdrs} ${aux_hdrs_ext} ${aux_hdrs_int})

source_group("Include\\External" FILES ${aux_hdrs_ext})
source_group("Include\\Internal" FILES ${aux_hdrs_int})

include_directories("${CMAKE_SOURCE_DIR}/src/cv" ".")

if(WIN32 AND MSVC)
    set(pch_header "_cvaux.h")
    set(pch_src "precomp.cpp")
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
set(the_target "cvaux")

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

# Add the required libraries for linking:
target_link_libraries(${the_target} ${OPENCV_LINKER_LIBS} cxcore cv)

if(MSVC AND CMAKE_CROSSCOMPILING)
	set_target_properties(${the_target} PROPERTIES
		LINK_FLAGS "/NODEFAULTLIB:secchk"
		)
endif()

# Dependencies of this target:
add_dependencies(${the_target} cxcore cv)

install(TARGETS ${the_target}
	RUNTIME DESTINATION bin COMPONENT dev
	LIBRARY DESTINATION lib COMPONENT dev
	ARCHIVE DESTINATION lib COMPONENT dev)

install(FILES ${aux_hdrs_ext}
        DESTINATION include/opencv
        COMPONENT dev)


