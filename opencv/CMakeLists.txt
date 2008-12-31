# ----------------------------------------------------------------------------
#  Root CMake file for OpenCV
#
#    From the off-tree build directory, invoke:
#      $ cmake <PATH_TO_OPENCV_ROOT>
#
#
#   - OCT-2008: Initial version <joseluisblancoc@gmail.com>
#
# ----------------------------------------------------------------------------

# it _must_ go before PROJECT(OpenCV) in order to work
set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "Configs" FORCE)
set(CMAKE_C_FLAGS_MINSIZEREL "" CACHE INTERNAL "" FORCE)
set(CMAKE_C_FLAGS_RELWITHDEBINFO "" CACHE INTERNAL "" FORCE)
set(CMAKE_CXX_FLAGS_MINSIZEREL "" CACHE INTERNAL "" FORCE)
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "" CACHE INTERNAL "" FORCE)
set(CMAKE_EXE_LINKER_FLAGS_MINSIZEREL "" CACHE INTERNAL "" FORCE)
set(CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO "" CACHE INTERNAL "" FORCE)
set(CMAKE_MODULE_LINKER_FLAGS_MINSIZEREL "" CACHE INTERNAL "" FORCE)
set(CMAKE_MODULE_LINKER_FLAGS_RELWITHDEBINFO "" CACHE INTERNAL "" FORCE)
set(CMAKE_SHARED_LINKER_FLAGS_MINSIZEREL "" CACHE INTERNAL "" FORCE)
set(CMAKE_SHARED_LINKER_FLAGS_RELWITHDEBINFO "" CACHE INTERNAL "" FORCE)

project(OpenCV)

cmake_minimum_required(VERSION 2.6)

set(CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS true)
if(MSVC)
    set(CMAKE_USE_RELATIVE_PATHS ON CACHE INTERNAL "" FORCE)
endif()

# --------------------------------------------------------------
# Indicate CMake 2.7 and above that we don't want to mix relative
#  and absolute paths in linker lib lists.
# Run "cmake --help-policy CMP0003" for more information.
# --------------------------------------------------------------
if(COMMAND cmake_policy)
    cmake_policy(SET CMP0003 NEW)
endif()

# ----------------------------------------------------------------------------
#  Current version number:
# ----------------------------------------------------------------------------
set(OPENCV_VERSION "1.1.0")

string(REGEX MATCHALL "[0-9]" OPENCV_VERSION_PARTS "${OPENCV_VERSION}")

list(GET OPENCV_VERSION_PARTS 0 OPENCV_VERSION_MAJOR)
list(GET OPENCV_VERSION_PARTS 1 OPENCV_VERSION_MINOR)
list(GET OPENCV_VERSION_PARTS 2 OPENCV_VERSION_PATCH)

set(OPENCV_SOVERSION "${OPENCV_VERSION_MAJOR}.${OPENCV_VERSION_MINOR}")

if(WIN32)
	# Postfix of DLLs:
	set(OPENCV_DLLVERSION "${OPENCV_VERSION_MAJOR}${OPENCV_VERSION_MINOR}${OPENCV_VERSION_PATCH}")
	set(OPENCV_DEBUG_POSTFIX d)
else()
	# Postfix of so's:
	#set(OPENCV_DLLVERSION "${OPENCV_VERSION_MAJOR}${OPENCV_VERSION_MINOR}${OPENCV_VERSION_PATCH}")
	set(OPENCV_DLLVERSION "")
	set(OPENCV_DEBUG_POSTFIX)
endif()


# ----------------------------------------------------------------------------
#  Variables for cvconfig.h.cmake
# ----------------------------------------------------------------------------
set(PACKAGE "opencv")
set(PACKAGE_BUGREPORT "opencvlibrary-devel@lists.sourceforge.net")
set(PACKAGE_NAME "opencv")
set(PACKAGE_STRING "${PACKAGE} ${OPENCV_VERSION}")
set(PACKAGE_TARNAME "${PACKAGE}")
set(PACKAGE_VERSION "${OPENCV_VERSION}")

# ----------------------------------------------------------------------------
#  Autodetect if we are in a SVN repository
# ----------------------------------------------------------------------------
find_program(SVNVERSION_PATH svnversion)
mark_as_advanced(force SVNVERSION_PATH)
if(SVNVERSION_PATH)
    message(STATUS "Extracting svn version, please wait...")
    execute_process(COMMAND ${SVNVERSION_PATH} -n ${CMAKE_SOURCE_DIR} OUTPUT_VARIABLE SVNVERSION_RESULT)

    if(SVNVERSION_RESULT MATCHES "exported")
       # This is NOT a svn repository:
       set(OPENCV_SVNVERSION "")
    else()
        set(OPENCV_SVNVERSION " svn:${SVNVERSION_RESULT}")
    endif()

    message(STATUS "SVNVERSION: ${OPENCV_SVNVERSION}")
else()
    # We don't have svnversion:
    set(OPENCV_SVNVERSION "")
endif()

# ----------------------------------------------------------------------------
# Detect GNU version:
# ----------------------------------------------------------------------------
if(CMAKE_COMPILER_IS_GNUCXX)
    execute_process(COMMAND ${CMAKE_CXX_COMPILER} --version
                  OUTPUT_VARIABLE CMAKE_OPENCV_GCC_VERSION_FULL
                  OUTPUT_STRIP_TRAILING_WHITESPACE)

    # Typical output in CMAKE_OPENCV_GCC_VERSION_FULL: "c+//0 (whatever) 4.2.3 (...)"
    #  Look for the version number
    string(REGEX MATCH "[0-9].[0-9].[0-9]" CMAKE_GCC_REGEX_VERSION "${CMAKE_OPENCV_GCC_VERSION_FULL}")

    # Split the three parts:
    string(REGEX MATCHALL "[0-9]" CMAKE_OPENCV_GCC_VERSIONS "${CMAKE_GCC_REGEX_VERSION}")

    list(GET CMAKE_OPENCV_GCC_VERSIONS 0 CMAKE_OPENCV_GCC_VERSION_MAJOR)
    list(GET CMAKE_OPENCV_GCC_VERSIONS 1 CMAKE_OPENCV_GCC_VERSION_MINOR)

    set(CMAKE_OPENCV_GCC_VERSION ${CMAKE_OPENCV_GCC_VERSION_MAJOR}${CMAKE_OPENCV_GCC_VERSION_MINOR})
    message(STATUS "Detected version of GNU GCC: ${CMAKE_OPENCV_GCC_VERSION}")
endif()

# ----------------------------------------------------------------------------
# 		CHECK FOR SYSTEM LIBRARIES, OPTIONS, ETC..
# ----------------------------------------------------------------------------

# Build (or not) some apps:
# ===================================================
set(BUILD_EXAMPLES OFF CACHE BOOL "Build all examples")

# Build tests:
# ===================================================
set(BUILD_TESTS ON CACHE BOOL "Build tests")

if(UNIX)
    include(FindPkgConfig OPTIONAL)
    include(CheckFunctionExists)
    include(CheckIncludeFile)
endif()

set(OPENCV_ENABLE_OPENMP OFF CACHE BOOL "")

if(CMAKE_COMPILER_IS_GNUCXX)
    set(OPENCV_ENABLE_PROFILING OFF CACHE BOOL "Enable profiling in the GCC compiler (Add flags: -g -pg)")
	if (NOT MINGW AND NOT ${CMAKE_SYSTEM_PROCESSOR} MATCHES arm*)
		set(OPENCV_USE_SSE2 ON CACHE BOOL "Enable SSE2 for GCC")
	endif (NOT MINGW AND NOT ${CMAKE_SYSTEM_PROCESSOR} MATCHES arm*)
endif()

# ===================================================
# Macros that checks if module have been installed.
# After it adds module to build and define
# constants passed as second arg
# ===================================================

macro(CHECK_MODULE module_name define)
    set(${define} 0)
    if(PKG_CONFIG_FOUND)
        set(ALIAS               ALIASOF_${module_name})
        set(ALIAS_FOUND                 ${ALIAS}_FOUND)
        set(ALIAS_INCLUDE_DIRS   ${ALIAS}_INCLUDE_DIRS)
        set(ALIAS_LIBRARY_DIRS   ${ALIAS}_LIBRARY_DIRS)
        set(ALIAS_LIBRARIES         ${ALIAS}_LIBRARIES)

        PKG_CHECK_MODULES(${ALIAS} ${module_name})

        if (${ALIAS_FOUND})
            set(${define} 1)
            foreach(P "${ALIAS_INCLUDE_DIRS}")
                list(APPEND HIGHGUI_INCLUDE_DIRS ${${P}})
            endforeach()

            foreach(P "${ALIAS_LIBRARY_DIRS}")
                list(APPEND HIGHGUI_LIBRARY_DIRS ${${P}})
            endforeach()

            list(APPEND HIGHGUI_LIBRARIES ${${ALIAS_LIBRARIES}})
        endif()
    endif()
endmacro()

if(UNIX)
    if(NOT APPLE)
        CHECK_MODULE(gtk+-2.0 HAVE_GTK)
        CHECK_MODULE(gthread-2.0 HAVE_GTHREAD)
        CHECK_MODULE(gstreamer-base-0.10 HAVE_GSTREAMER)
    endif()

    CHECK_MODULE(libunicap HAVE_UNICAP_)
    CHECK_MODULE(libucil HAVE_UNICAP_UCIL)
    if(HAVE_UNICAP_ AND HAVE_UNICAP_UCIL)
        set(HAVE_UNICAP 1)
    endif()

    CHECK_MODULE(libavcodec HAVE_FFMPEG_CODEC)
    CHECK_MODULE(libavformat HAVE_FFMPEG_FORMAT)
    CHECK_MODULE(libavutil HAVE_FFMPEG_UTIL)
    if(HAVE_FFMPEG_CODEC AND HAVE_FFMPEG_FORMAT AND HAVE_FFMPEG_UTIL)
        set(HAVE_FFMPEG 1)
    endif()

    CHECK_MODULE(libdc1394-2 HAVE_DC1394_2)
    if(NOT HAVE_DC1394_2)
        CHECK_MODULE(libdc1394 HAVE_DC1394)
    endif()

    if(NOT APPLE)
        CHECK_INCLUDE_FILE(alloca.h HAVE_ALLOCA_H)
        CHECK_FUNCTION_EXISTS(alloca HAVE_ALLOCA)
        CHECK_INCLUDE_FILE(unistd.h HAVE_UNISTD_H)
        CHECK_INCLUDE_FILE(pthread.h HAVE_LIBPTHREAD)

        CHECK_MODULE(libxine HAVE_XINE)
        CHECK_INCLUDE_FILE(linux/videodev.h HAVE_CAMV4L)
        CHECK_INCLUDE_FILE(linux/videodev2.h HAVE_CAMV4L2)

        include(FindPNG)
        include(FindJPEG)
        include(FindTIFF)
        include(FindJasper)

        set(OPENCV_LINKER_LIBS dl m pthread)
    else()
        add_definitions(-DHAVE_ALLOCA -DHAVE_ALLOCA_H -DHAVE_LIBPTHREAD -DHAVE_UNISTD_H)
    endif()
endif()

set(BUILD_PYTHON_SUPPORT OFF CACHE BOOL "Build with Python support")
set(BUILD_OCTAVE_SUPPORT OFF CACHE BOOL "Build with Octave support")

find_package(SWIG)
if(SWIG_FOUND)

find_package(PythonInterp)
find_package(PythonLibs)

# Build with python if installes:
# ===================================================
if(PYTHONLIBS_FOUND AND BUILD_PYTHON_SUPPORT)
    execute_process(COMMAND ${PYTHON_EXECUTABLE} --version
              ERROR_VARIABLE PYTHON_VERSION_FULL
              OUTPUT_STRIP_TRAILING_WHITESPACE)

    string(REGEX MATCH "[0-9].[0-9]" PYTHON_VERSION_MAJOR_MINOR "${PYTHON_VERSION_FULL}")
    if(UNIX)
        set(PYTHON_PLUGIN_INSTALL_PATH lib/python${PYTHON_VERSION_MAJOR_MINOR}/site-packages/opencv)
    endif()
    if(WIN32)
        get_filename_component(PYTHON_PATH "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Python\\PythonCore\\${PYTHON_VERSION_MAJOR_MINOR}\\InstallPath]" ABSOLUTE CACHE)
        set(PYTHON_PLUGIN_INSTALL_PATH "${PYTHON_PATH}/Lib/site-packages/opencv")
    endif()
endif()

endif()

if(MSVC)

set(IPP_FOUND)
foreach(v "6.1" "6.0" "5.3" "5.2" "5.1")
    if(NOT IPP_FOUND)
        find_path(IPP_PATH "ippi-${v}.dll"
            PATHS ${CMAKE_PROGRAM_PATH} ${CMAKE_SYSTEM_PROGRAM_PATH}
            DOC "The path to IPP DLLs")
        if(IPP_PATH)
            file(GLOB IPP_HDRS "${IPP_PATH}/../include")
            if(IPP_HDRS)
                set(IPP_FOUND TRUE)
            endif()
        endif()
    endif()
endforeach()

message(STATUS "IPP detected: ${IPP_FOUND}")
set(BUILD_CUSTOM_IPP_LIB ${IPP_FOUND} CACHE BOOL "Build custom opencv_ipp*.dll")

endif()

# ----------------------------------------------------------------------------
#   				UPDATE CONFIG FILES & SCRIPTS:
#
#  CONFIGURE_FILE(InputFile OutputFile [COPYONLY] [ESCAPE_QUOTES] [@ONLY])
# If @ONLY is specified, only variables of the form @VAR@ will be
#  replaces and ${VAR} will be ignored.
#
#  A directory will be created for each platform so the "cvconfig.h" file is
#   not overwritten if cmake generates code in the same path.
# ----------------------------------------------------------------------------
add_definitions(-DHAVE_CONFIG_H)

set(OPENCV_CONFIG_FILE_INCLUDE_DIR "${CMAKE_BINARY_DIR}/" CACHE PATH "Where to create the platform-dependant cvconfig.h")

message(STATUS "Parsing 'cvconfig.h.cmake'")
configure_file("${CMAKE_SOURCE_DIR}/cvconfig.h.cmake" "${OPENCV_CONFIG_FILE_INCLUDE_DIR}/cvconfig.h")

# ---------------------------------------------------------------------------
# The C+//0 include & link directories:
# ---------------------------------------------------------------------------
include_directories("."
    "${OPENCV_CONFIG_FILE_INCLUDE_DIR}"
    "${CMAKE_SOURCE_DIR}/include"
    "${CMAKE_SOURCE_DIR}/include/opencv"
    )

# ----------------------------------------------------------------------------
# 			Set the maximum level of warnings:
# ----------------------------------------------------------------------------
# Should be set to true for development
set(OPENCV_WARNINGS_ARE_ERRORS OFF CACHE BOOL "Treat warnings as errors")

# Whole program optimization?
set(OPENCV_WHOLE_PROGRAM_OPTIMIZATION OFF CACHE BOOL "Flags for whole program optimization.")

set(EXTRA_C_FLAGS "")
set(EXTRA_C_FLAGS_RELEASE "")
set(EXTRA_C_FLAGS_DEBUG "")
set(EXTRA_EXE_LINKER_FLAGS "")
set(EXTRA_EXE_LINKER_FLAGS_RELEASE "")
set(EXTRA_EXE_LINKER_FLAGS_DEBUG "")

if(MSVC)
    set(EXTRA_C_FLAGS "${EXTRA_C_FLAGS} /W4 /D _CRT_SECURE_NO_DEPRECATE /D _CRT_NONSTDC_NO_DEPRECATE")
    # 64-bit portability warnings, in MSVC8
    if(MSVC80)
        set(EXTRA_C_FLAGS "${OPENCV_EXTRA_CPP_FLAGS} /Wp64")
    endif()

    set(EXTRA_EXE_LINKER_FLAGS_RELEASE "${EXTRA_EXE_LINKER_FLAGS_RELEASE} /debug")

    # Whole program optimization
    if(OPENCV_WHOLE_PROGRAM_OPTIMIZATION AND MSVC80)
        set(EXTRA_C_FLAGS_RELEASE "${EXTRA_C_FLAGS_RELEASE} /GL")
        set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS} /LTCG")
        set(CMAKE_MODULE_LINKER_FLAGS_RELEASE "${CMAKE_MODULE_LINKER_FLAGS} /LTCG")
    endif()

    if(OPENCV_ENABLE_OPENMP)
        set(EXTRA_C_FLAGS_RELEASE "${EXTRA_C_FLAGS} /openmp")
    endif()

    # Remove unreferenced functions: function level linking
    set(EXTRA_C_FLAGS "${EXTRA_C_FLAGS} /Gy")
    set(EXTRA_C_FLAGS_RELEASE "${EXTRA_C_FLAGS_RELEASE} /Zi")
endif()

if(CMAKE_COMPILER_IS_GNUCXX)
    # High level of warnings.
    # The -Wno-long-long is required in 64bit systems when including sytem headers.
    set(EXTRA_C_FLAGS "${EXTRA_C_FLAGS} -Wall")

    # We need pthread's
    if(UNIX)
        set(EXTRA_C_FLAGS "${EXTRA_C_FLAGS} -pthread")
    endif()

    if(OPENCV_WARNINGS_ARE_ERRORS)
        set(EXTRA_C_FLAGS "${EXTRA_C_FLAGS} -Werror")
    endif()

    # GCC optimization for floating math
    set(EXTRA_C_FLAGS_RELEASE "${EXTRA_C_FLAGS_RELEASE} -ffast-math")

    # Whole program optimization
    if(OPENCV_WHOLE_PROGRAM_OPTIMIZATION)
        set(EXTRA_C_FLAGS_RELEASE "${EXTRA_C_FLAGS_RELEASE} -fwhole-program --combine")
    endif()

    # Profiling?
    if(OPENCV_ENABLE_PROFILING)
        set(EXTRA_C_FLAGS_RELEASE "${EXTRA_C_FLAGS_RELEASE} -pg -g")
    else()
        # Remove unreferenced functions: function level linking
        if(NOT APPLE)
            set(EXTRA_C_FLAGS "${EXTRA_C_FLAGS} -ffunction-sections")
        endif()
    endif()

    # Parallel mode
    if(OPENCV_ENABLE_OPENMP)
        set(EXTRA_C_FLAGS "${EXTRA_C_FLAGS} -D_GLIBCXX_PARALLEL -fopenmp")
    endif()

    set(EXTRA_C_FLAGS_RELEASE "${EXTRA_C_FLAGS_RELEASE} -DNDEBUG")
    set(EXTRA_C_FLAGS_DEBUG "${EXTRA_C_FLAGS_DEBUG} -O0 -DDEBUG -D_DEBUG")

    if(OPENCV_USE_SSE2)
	   set(EXTRA_C_FLAGS "${EXTRA_C_FLAGS} -msse2")
    endif()
endif()

# Add user supplied extra options (optimization, etc...)
# ==========================================================
set(OPENCV_EXTRA_C_FLAGS ${EXTRA_C_FLAGS} CACHE STRING "Extra compiler options")
set(OPENCV_EXTRA_C_FLAGS_RELEASE ${EXTRA_C_FLAGS_RELEASE} CACHE STRING "Extra compiler options for Release build")
set(OPENCV_EXTRA_C_FLAGS_DEBUG ${EXTRA_C_FLAGS_DEBUG} CACHE STRING "Extra compiler options for Debug build")
set(OPENCV_EXTRA_EXE_LINKER_FLAGS ${EXTRA_EXE_LINKER_FLAGS} CACHE STRING "Extra linker flags")
set(OPENCV_EXTRA_EXE_LINKER_FLAGS_RELEASE ${EXTRA_EXE_LINKER_FLAGS_RELEASE} CACHE STRING "Extra linker flags for Release build")
set(OPENCV_EXTRA_EXE_LINKER_FLAGS_DEBUG ${EXTRA_EXE_LINKER_FLAGS_DEBUG} CACHE STRING "Extra linker flags for Debug build")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OPENCV_EXTRA_C_FLAGS}")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OPENCV_EXTRA_C_FLAGS}")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} ${OPENCV_EXTRA_C_FLAGS_RELEASE}")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} ${OPENCV_EXTRA_C_FLAGS_RELEASE}")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ${OPENCV_EXTRA_C_FLAGS_DEBUG}")
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} ${OPENCV_EXTRA_C_FLAGS_DEBUG}")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OPENCV_EXTRA_EXE_LINKER_FLAGS}")
set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} ${OPENCV_EXTRA_EXE_LINKER_FLAGS_RELEASE}")
set(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} ${OPENCV_EXTRA_EXE_LINKER_FLAGS_DEBUG}")

if("${CMAKE_CONFIGURE_LDFLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${CMAKE_CONFIGURE_LDFLAGS}")
endif("${CMAKE_CONFIGURE_LDFLAGS}")

# ----------------------------------------------------------------------------
#      					PROCESS SUBDIRECTORIES:
# ----------------------------------------------------------------------------
# Save libs and executables in the same place
set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/lib CACHE PATH "Output directory for libraries" )
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin CACHE PATH "Output directory for applications" )

if(WIN32)
set(OPENCV_DOC_INSTALL_PATH doc)
else()
set(OPENCV_DOC_INSTALL_PATH share/opencv/doc)
endif()

#-----------------------------------
# Subdirectories:
#-----------------------------------
add_subdirectory(src)
add_subdirectory(apps)
add_subdirectory(doc)
add_subdirectory(data)

if(BUILD_EXAMPLES)
    add_subdirectory(samples)
endif()

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

add_subdirectory(interfaces)

if(WIN32)
    add_subdirectory(3rdparty)
endif()

# --------------------------------------------------------------------------------------------
#   Installation for CMake Module:  OpenCVConfig.cmake
#  Part 1/2: ${BIN_DIR}/OpenCVConfig.cmake              -> For use *without* "make install"
#  Part 2/2: ${BIN_DIR}/unix-install/OpenCVConfig.cmake -> For use with "make install"
# -------------------------------------------------------------------------------------------

# -------------------------------------------------------------------------------------------
#  Part 1/2: ${BIN_DIR}/OpenCVConfig.cmake              -> For use *without* "make install"
# -------------------------------------------------------------------------------------------

# Name of libraries is: libcv.so.1.1.0, etc...
#  OPENCV_DLLVERSION must be set to "110", etc..
# Already done above.

# Set CMAKE_INCLUDE_DIRS_CONFIGCMAKE to the list of include directories:
set(CMAKE_INCLUDE_DIRS_CONFIGCMAKE)
list(APPEND CMAKE_INCLUDE_DIRS_CONFIGCMAKE "${OPENCV_CONFIG_FILE_INCLUDE_DIR}")
list(APPEND CMAKE_INCLUDE_DIRS_CONFIGCMAKE "${CMAKE_SOURCE_DIR}/include")
list(APPEND CMAKE_INCLUDE_DIRS_CONFIGCMAKE "${CMAKE_SOURCE_DIR}/include/opencv")

set(CMAKE_LIB_DIRS_CONFIGCMAKE ${LIBRARY_OUTPUT_PATH})

configure_file("${CMAKE_SOURCE_DIR}/OpenCVConfig.cmake.in" "${CMAKE_BINARY_DIR}/OpenCVConfig.cmake" IMMEDIATE)

# --------------------------------------------------------------------------------------------
#  Part 2/2: ${BIN_DIR}/unix-install/OpenCVConfig.cmake -> For use *with* "make install"
# -------------------------------------------------------------------------------------------
# Set CMAKE_INCLUDE_DIRS_CONFIGCMAKE to the list of include directories:

set(CMAKE_INCLUDE_DIRS_CONFIGCMAKE "${CMAKE_INSTALL_PREFIX}/include/opencv")
set(CMAKE_LIB_DIRS_CONFIGCMAKE "${CMAKE_INSTALL_PREFIX}/lib")

exec_program(${CMAKE_COMMAND} ARGS "-E make_directory \"${CMAKE_BINARY_DIR}/unix-install/\"")
configure_file("${CMAKE_SOURCE_DIR}/OpenCVConfig.cmake.in" "${CMAKE_BINARY_DIR}/unix-install/OpenCVConfig.cmake" IMMEDIATE)

if(UNIX)
    # For a command "FIND_PACKAGE(FOO)", CMake will look at the directory /usr/share|lib/FOO/FOOConfig.cmake, so:
    install(FILES ${CMAKE_BINARY_DIR}/unix-install/OpenCVConfig.cmake DESTINATION ${CMAKE_INSTALL_PREFIX}/share/opencv/)
endif()


# --------------------------------------------------------------------------------------------
#  Part 2/2: ${BIN_DIR}/unix-install/opencv.pc -> For use *with* "make install"
# prefix=/usr
# exec_prefix=${prefix}
# libdir=${exec_prefix}/lib
# includedir=${prefix}/include/opencv
# -------------------------------------------------------------------------------------------
set(prefix 		${CMAKE_INSTALL_PREFIX})
set(exec_prefix		"\${prefix}")
set(libdir		"\${exec_prefix}/lib")
set(includedir		"\${prefix}/include")
set(VERSION             ${OPENCV_VERSION})

configure_file("${CMAKE_SOURCE_DIR}/opencv.pc.cmake.in" "${CMAKE_BINARY_DIR}/unix-install/opencv.pc" @ONLY IMMEDIATE)

if(UNIX)
    install(FILES ${CMAKE_BINARY_DIR}/unix-install/opencv.pc DESTINATION lib/pkgconfig)
endif()


# ----------------------------------------------------------------------------
#   Uninstall target, for "make uninstall"
# ----------------------------------------------------------------------------
CONFIGURE_FILE(
  "${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
  IMMEDIATE @ONLY)

ADD_CUSTOM_TARGET(uninstall
  "${CMAKE_COMMAND}" -P "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake")


# ----------------------------------------------------------------------------
#   CPack target
# ----------------------------------------------------------------------------

option(BUILD_PACKAGE "Build a installer with the SDK")

if(BUILD_PACKAGE)

	configure_file(
	Package.cmake.in
	${CMAKE_BINARY_DIR}/.cpack/Package.cmake
	@ONLY
	)
	
	install(FILES 
		AUTHORS
		ChangeLog
		COPYING
		INSTALL
		NEWS
		README
		THANKS
		TODO
		DESTINATION ${OPENCV_DOC_INSTALL_PATH}
        COMPONENT doc
		)
		
	include(${CMAKE_BINARY_DIR}/.cpack/Package.cmake)
	
endif()

# ----------------------------------------------------------------------------
#   Sumary:
# ----------------------------------------------------------------------------
message(STATUS "")
message(STATUS "General configuration for opencv ${OPENCV_VERSION} =====================================")
message(STATUS "")
message(STATUS "    Compiler:                  ${CMAKE_COMPILER}")
message(STATUS "    C++ flags (Release):       ${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_RELEASE}")
message(STATUS "    C++ flags (Debug):         ${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_DEBUG}")
if(WIN32)
message(STATUS "    Linker flags (Release):    ${CMAKE_EXE_LINKER_FLAGS} ${CMAKE_EXE_LINKER_FLAGS_RELEASE}")
message(STATUS "    Linker flags (Debug):      ${CMAKE_EXE_LINKER_FLAGS} ${CMAKE_EXE_LINKER_FLAGS_DEBUG}")
else()
message(STATUS "    Linker flags (Release):    ${CMAKE_SHARED_LINKER_FLAGS} ${CMAKE_SHARED_LINKER_FLAGS_RELEASE}")
message(STATUS "    Linker flags (Debug):      ${CMAKE_SHARED_LINKER_FLAGS} ${CMAKE_SHARED_LINKER_FLAGS_DEBUG}")
endif()

if(UNIX)
if(NOT APPLE)
message(STATUS "")
message(STATUS "  GUI: ")
message(STATUS "    GTK+ 2.x:                  ${HAVE_GTK}")
message(STATUS "    GThread:                   ${HAVE_GTHREAD}")
message(STATUS "")
message(STATUS "  Image I/O: ")
message(STATUS "    JPEG:                      ${JPEG_FOUND}")
message(STATUS "    PNG:                       ${PNG_FOUND}")
message(STATUS "    TIFF:                      ${TIFF_FOUND}")
message(STATUS "    JASPER:                    ${JASPER_FOUND}")
endif()
message(STATUS "")
message(STATUS "  Video I/O: ")
message(STATUS "    DC1394 1.x:                ${HAVE_DC1394}")
message(STATUS "    DC1394 2.x:                ${HAVE_DC1394_2}")
message(STATUS "    FFMPEG:                    ${HAVE_FFMPEG}")
message(STATUS "    GStreamer:                 ${HAVE_GSTREAMER}")
message(STATUS "    UniCap:                    ${HAVE_UNICAP}")
message(STATUS "    V4L/V4L2:                  ${HAVE_CAMV4L}/${HAVE_CAMV4L2}")
message(STATUS "    Xine:                      ${HAVE_XINE}")
endif()
message(STATUS "")
message(STATUS "  Interfaces: ")
if(PYTHONLIBS_FOUND AND BUILD_PYTHON_SUPPORT)
message(STATUS "    Python:                    1")
else()
message(STATUS "    Python:                    0")
endif()
if(IPP_FOUND AND BUILD_CUSTOM_IPP_LIB)
message(STATUS "    Custom IPP Lib:            1")
else()
message(STATUS "    Custom IPP Lib:            0")
endif()
message(STATUS "")
message(STATUS "    Install path:              ${CMAKE_INSTALL_PREFIX}")
message(STATUS "")
message(STATUS "    cvconfig.h is in:          ${OPENCV_CONFIG_FILE_INCLUDE_DIR}")
message(STATUS "-----------------------------------------------------------------")
message(STATUS "")

#----------------------------------------------------------------------------
#   Generate the OpenCVConfig.cmake file for unix
#      installation in CMAKE_INSTALL_PREFIX
#----------------------------------------------------------------------------
