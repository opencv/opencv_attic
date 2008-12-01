# ----------------------------------------------------------------------------
#  CMake file for zlib. See root CMakeLists.txt 
#
# ----------------------------------------------------------------------------
PROJECT(zlib)

# List of C++ files:

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR} 
    ${CMAKE_SOURCE_DIR}/otherlibs/_graphics/include
    )

file(GLOB lib_srcs *.c)
file(GLOB lib_hdrs *.h)

set(the_target "zlib")

add_library(${the_target} STATIC ${lib_srcs} ${lib_hdrs})

if(MSVC)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /W3")
endif()

set_target_properties(${the_target} 
    PROPERTIES 
        OUTPUT_NAME "${the_target}" 
        DEBUG_POSTFIX "d"
        ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/otherlibs/_graphics/lib
    )

#install(TARGETS ${the_target}
#	RUNTIME DESTINATION bin
#	LIBRARY DESTINATION lib
#	ARCHIVE DESTINATION lib
#	)
