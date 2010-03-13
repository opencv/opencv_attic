# ----------------------------------------------------------------------------
#  CMake file for python support
# ----------------------------------------------------------------------------
project(python_support)

include_directories(${PYTHON_INCLUDE_PATH})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_BINARY_DIR})

if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W3")
endif()

file(GLOB lib_srcs "*.cpp")

add_custom_command(
   OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/generated0.i
   COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/gen.py ${CMAKE_CURRENT_SOURCE_DIR}
   DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/api
   DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/defs
   DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/gen.py
   )

set(the_target "cvpy")
add_library(${the_target} ${lib_srcs} ${lib_hdrs} ${lib_int_hdrs} ${CMAKE_CURRENT_BINARY_DIR}/generated0.i)
target_link_libraries(${the_target} ${PYTHON_LIBRARIES} cxcore cv cvaux highgui)
set_target_properties(${the_target} PROPERTIES PREFIX "")
set_target_properties(${the_target} PROPERTIES OUTPUT_NAME "cv")

execute_process(COMMAND ${PYTHON_EXECUTABLE} -c "import distutils.sysconfig; print distutils.sysconfig.get_config_var('SO')"
                RESULT_VARIABLE PYTHON_CVPY_PROCESS
                OUTPUT_VARIABLE CVPY_SUFFIX
                OUTPUT_STRIP_TRAILING_WHITESPACE)

set_target_properties(${the_target} PROPERTIES SUFFIX ${CVPY_SUFFIX})

set(cvpy_files cv${CVPY_SUFFIX})

if(WIN32)
    install(TARGETS ${the_target}
            RUNTIME DESTINATION "Python${PYTHON_VERSION_MAJOR_MINOR}/Lib/site-packages" COMPONENT main
            LIBRARY DESTINATION "Python${PYTHON_VERSION_MAJOR_MINOR}/Lib/site-packages" COMPONENT main
            ARCHIVE DESTINATION "Python${PYTHON_VERSION_MAJOR_MINOR}/Lib/site-packages" COMPONENT main
            )
else()
    install(FILES ${LIBRARY_OUTPUT_PATH}/cv.so DESTINATION ${PYTHON_PACKAGES_PATH})
endif()
