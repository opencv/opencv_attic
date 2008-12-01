# ----------------------------------------------------------------------------
#  CMake file for libjpeg. See root CMakeLists.txt 
#
# ----------------------------------------------------------------------------
project(libjpeg)

# List of C++ files:

include_directories(
	${CMAKE_CURRENT_SOURCE_DIR} 
	${CMAKE_SOURCE_DIR}/otherlibs/_graphics/include
	)

# The .cpp files:
file(GLOB lib_srcs *.c)
file(GLOB lib_hdrs *.h)

# ----------------------------------------------------------------------------------
# 				Define the library target:
# ----------------------------------------------------------------------------------

set(the_target "libjpeg")

add_library(${the_target} STATIC ${lib_srcs} ${lib_hdrs})

if(MSVC)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /W3")
    add_definitions(-DJAS_WIN_MSVC_BUILD)
endif()

set_target_properties(${the_target} 
	PROPERTIES OUTPUT_NAME "${the_target}" 
	DEBUG_POSTFIX "d"
	ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/otherlibs/_graphics/lib
	)

#install(TARGETS ${the_target}
#	RUNTIME DESTINATION bin
#	LIBRARY DESTINATION lib
#	ARCHIVE DESTINATION lib
#	)
