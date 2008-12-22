project(opencv_ipp)

include_directories("${CMAKE_CURRENT_SOURCE_DIR}")
include_directories("${CMAKE_SOURCE_DIR}/3rdparty/include")
include_directories("${CMAKE_SOURCE_DIR}/src/highgui")
include_directories("${IPP_PATH}/../include")
link_directories("${IPP_PATH}/../lib")

set(the_target opencv_ipp)
if(MSVC)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /W3")
endif()

add_library(${the_target} SHARED dllmain.c opencvipp_funclist.h export51.def)

target_link_libraries(${the_target} ippsmerged ippsemerged
    ippimerged ippiemerged ippcvmerged ippcvemerged
    ippccmerged ippccemerged ippvmmerged ippvmemerged ippcorel)

set_target_properties(${the_target} PROPERTIES
                    DEBUG_POSTFIX "${OPENCV_DEBUG_POSTFIX}"
                    VERSION ${OPENCV_VERSION}
                    OUTPUT_NAME "${the_target}${OPENCV_DLLVERSION}"
                    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib/"
                    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/"
                    )

install(TARGETS ${the_target}
	RUNTIME DESTINATION bin COMPONENT dev
	LIBRARY DESTINATION lib COMPONENT dev
    )
