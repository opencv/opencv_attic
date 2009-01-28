project(opencv_ffmpeg)

include_directories("${CMAKE_CURRENT_SOURCE_DIR}")
include_directories("${CMAKE_SOURCE_DIR}/3rdparty/include")
include_directories("${CMAKE_SOURCE_DIR}/src/highgui")
link_directories("${CMAKE_SOURCE_DIR}/3rdparty/lib")

set(the_target opencv_ffmpeg)
add_library(${the_target} SHARED ffopencv.cpp ffopencv.h)

add_dependencies(${the_target} cxcore)
target_link_libraries(${the_target} cxcore) #libavcodec libavformat libavutil libgcc_mingw libmingwex)

set_target_properties(${the_target} PROPERTIES
                      DEBUG_POSTFIX "${OPENCV_DEBUG_POSTFIX}"
                      VERSION ${OPENCV_VERSION}
                      OUTPUT_NAME "${the_target}${OPENCV_DLLVERSION}"
                      DEFINE_SYMBOL "CVAPI_EXPORTS"
                      ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib/"
                      RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/"
                      )

install(TARGETS ${the_target}
	RUNTIME DESTINATION bin COMPONENT dev
	LIBRARY DESTINATION lib COMPONENT dev
    )
