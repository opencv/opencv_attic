project(opencv_ffmpeg)

include_directories("${CMAKE_CURRENT_SOURCE_DIR}")
include_directories("${CMAKE_SOURCE_DIR}/3rdparty/include")
include_directories("${CMAKE_SOURCE_DIR}/src/highgui")
link_directories("${CMAKE_SOURCE_DIR}/3rdparty/lib")

set(the_target opencv_ffmpeg)
add_library(${the_target} SHARED ffopencv.cpp ffopencv.h)

if (MSVC)
add_dependencies(${the_target} cxcore)
target_link_libraries(${the_target} cxcore)
else ()
add_dependencies(${the_target} cxcore)
target_link_libraries(${the_target} cxcore 
                      ${CMAKE_SOURCE_DIR}/3rdparty/lib/libgcc_.a
                      ${CMAKE_SOURCE_DIR}/3rdparty/lib/libmingwex_.a
                      ${CMAKE_SOURCE_DIR}/3rdparty/lib/libavformat.a
                      ${CMAKE_SOURCE_DIR}/3rdparty/lib/libavcodec.a
                      ${CMAKE_SOURCE_DIR}/3rdparty/lib/libavutil.a
                      ${CMAKE_SOURCE_DIR}/3rdparty/lib/libwsock32_.a)
endif ()

set_target_properties(${the_target} PROPERTIES
                      DEBUG_POSTFIX "${OPENCV_DEBUG_POSTFIX}"
                      VERSION ${OPENCV_VERSION}
                      OUTPUT_NAME "${the_target}${OPENCV_DLLVERSION}"
                      DEFINE_SYMBOL "CVAPI_EXPORTS"
                      ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib/"
                      RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/"
                      )

install(TARGETS ${the_target}
	RUNTIME DESTINATION bin COMPONENT main
	LIBRARY DESTINATION lib COMPONENT main
    )
