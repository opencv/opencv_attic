# ----------------------------------------------------------------------------
#  CMake file for highgui. See root CMakeLists.txt
#   Some parts taken from version of Hartmut Seichter, HIT Lab NZ.
#   Jose Luis Blanco, 2008
# ----------------------------------------------------------------------------
project(highgui)

if(OPENCV_BUILD_3RDPARTY_LIBS)
    include_directories("${CMAKE_CURRENT_SOURCE_DIR}/../../3rdparty/include")
    add_definitions(-DHAVE_JPEG -DHAVE_PNG -DHAVE_TIFF -DHAVE_JASPER)
endif()

if(MSVC)
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /NODEFAULTLIB:libcmt.lib")
endif()

file(GLOB grfmt_hdrs grfmt*.h)
file(GLOB grfmt_srcs grfmt*.cpp)
set(grfmt_hdrs bitstrm.h ${grfmt_hdrs})
set(grfmt_srcs bitstrm.cpp ${grfmt_srcs})

source_group("Src\\grfmts" FILES ${grfmt_hdrs} ${grfmt_srcs})

set(highgui_srcs
    cvcap.cpp
    cvcap_images.cpp
    image.cpp
    loadsave.cpp
    precomp.cpp
    utils.cpp
    window.cpp
    )

set(highgui_hdrs _highgui.h utils.h)

set(lib_hdr_names highgui.h highgui.hpp)
set(highgui_ext_hdrs)
foreach(h ${lib_hdr_names})
    list(APPEND highgui_ext_hdrs "${CMAKE_CURRENT_SOURCE_DIR}/../../include/opencv/${h}")
endforeach()

if(WIN32)
    set(highgui_srcs ${highgui_srcs} window_w32.cpp cvcap_vfw.cpp cvcap_cmu.cpp cvcap_w32.cpp cvcap_dshow.cpp)
    if(HAVE_MIL)
        set(highgui_srcs ${highgui_srcs} cvcap_mil.cpp)
    endif()
endif()

if(UNIX)
    if(HAVE_GTK)
        set(highgui_srcs ${highgui_srcs} window_gtk.cpp)
    endif()

    if(HAVE_XINE)
        set(highgui_srcs ${highgui_srcs} cvcap_xine.cpp)
    endif()

    if(HAVE_DC1394_2)
        set(highgui_srcs ${highgui_srcs} cvcap_dc1394_v2.cpp)
    endif()

    if(HAVE_DC1394)
        set(highgui_srcs ${highgui_srcs} cvcap_dc1394.cpp)
    endif()

    if(HAVE_FFMPEG)
        set(highgui_srcs ${highgui_srcs} cvcap_ffmpeg.cpp)
    endif()

    if(HAVE_GSTREAMER)
        set(highgui_srcs ${highgui_srcs} cvcap_gstreamer.cpp gstappsink.cpp)
        set(highgui_hdrs ${highgui_hdrs} gstappsink.h)
    endif()

    if(HAVE_UNICAP)
        set(highgui_srcs ${highgui_srcs} cvcap_unicap.cpp)
    endif()

	if(HAVE_LIBV4L)
		set(highgui_srcs ${highgui_srcs} cvcap_libv4l.cpp)
	else()
		set(highgui_srcs ${highgui_srcs} cvcap_v4l.cpp)
	endif()

    if(NOT APPLE AND NOT OPENCV_BUILD_3RDPARTY_LIBS)
        if(JPEG_FOUND)
            add_definitions(-DHAVE_JPEG)
            include_directories(${JPEG_INCLUDE_DIR})
        endif()
        if(PNG_FOUND)
            add_definitions(-DHAVE_PNG)
            add_definitions(${PNG_DEFINITIONS})
            include_directories(${PNG_INCLUDE_DIR})
        endif()
        if(TIFF_FOUND)
            add_definitions(-DHAVE_TIFF)
            include_directories(${TIFF_INCLUDE_DIR})
        endif()
        if(JASPER_FOUND)
            add_definitions(-DHAVE_JASPER)
            include_directories(${JASPER_INCLUDE_DIR})
        endif()
    endif()

    foreach(P ${HIGHGUI_INCLUDE_DIRS})
        include_directories(${P})
    endforeach()

    foreach(P ${HIGHGUI_LIBRARY_DIRS})
        link_directories(${P})
    endforeach()
endif()

if(APPLE)
    add_definitions(-DHAVE_QUICKTIME=1 -DHAVE_CARBON=1)
    if(NOT OPENCV_BUILD_3RDPARTY_LIBS)
        add_definitions(-DHAVE_IMAGEIO=1)
    endif()
    set(highgui_srcs ${highgui_srcs} window_carbon.cpp cvcap_qt.cpp)
endif(APPLE)

source_group("Src" FILES ${highgui_srcs} ${highgui_hdrs})
source_group("Include" FILES ${highgui_ext_hdrs})

if(OPENCV_BUILD_3RDPARTY_LIBS)
    if(WIN32)
        link_directories("${CMAKE_CURRENT_SOURCE_DIR}/../../3rdparty/lib")
    endif()
    link_directories(
        "${CMAKE_BINARY_DIR}/3rdparty/lib"
        "${CMAKE_BINARY_DIR}/3rdparty/lib/${ConfigurationName}"
        )
endif()

set(lib_srcs ${highgui_srcs} ${grfmt_srcs})

# ----------------------------------------------------------------------------------
# 				Define the library target:
# ----------------------------------------------------------------------------------
set(the_target "highgui")

add_definitions(-DHIGHGUI_EXPORTS -DCVAPI_EXPORTS)

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/../../include/opencv"
                    "${CMAKE_CURRENT_SOURCE_DIR}"
                    "${CMAKE_CURRENT_BINARY_DIR}")

add_library(${the_target} SHARED ${lib_srcs} ${highgui_hdrs} ${grfmt_hdrs} ${highgui_ext_hdrs})

if(PCHSupport_FOUND)
    set(pch_header ${CMAKE_CURRENT_SOURCE_DIR}/_highgui.h)
    if(${CMAKE_GENERATOR} MATCHES "Visual*" OR ${CMAKE_GENERATOR} MATCHES "Xcode*")
        if(${CMAKE_GENERATOR} MATCHES "Visual*")
            set(${the_target}_pch "precomp.cpp")
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
	LINK_INTERFACE_LIBRARIES ""
	)

if(MSVC)
	set_target_properties(${the_target} PROPERTIES LINK_FLAGS "/NODEFAULTLIB:atlthunk.lib")
endif(MSVC)

# Dependencies of this target:
add_dependencies(${the_target} cv cxcore)

# Add the required libraries for linking:
target_link_libraries(${the_target} ${OPENCV_LINKER_LIBS} cxcore cv ${JPEG_LIBRARIES} ${PNG_LIBRARIES} ${TIFF_LIBRARIES} ${JASPER_LIBRARIES} ${HIGHGUI_LIBRARIES})

if(OPENCV_BUILD_3RDPARTY_LIBS)
	add_dependencies(${the_target} libjasper libjpeg libpng libtiff zlib)
	target_link_libraries(${the_target} libjasper libjpeg libpng libtiff zlib)
endif()
if(WIN32)
	target_link_libraries(${the_target} comctl32 gdi32 ole32)
	
	if (MSVC)
		target_link_libraries(${the_target} vfw32)
	endif()

	if(MINGW)
		target_link_libraries(${the_target} vfw32 winmm videoInput strmiids)
	endif()
endif()

if(APPLE)
	target_link_libraries(${the_target} "-framework Carbon -framework CoreFoundation -framework QuickTime -framework QuartzCore")
endif()

install(TARGETS ${the_target}
	RUNTIME DESTINATION bin COMPONENT main
	LIBRARY DESTINATION lib COMPONENT main
	ARCHIVE DESTINATION lib COMPONENT main)

install(FILES ${highgui_ext_hdrs}
        DESTINATION include/opencv
        COMPONENT main)
