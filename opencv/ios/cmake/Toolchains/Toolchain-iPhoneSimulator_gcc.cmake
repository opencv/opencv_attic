message ( "Setting up iPhoneSimulator toolchain" )

set ( CMAKE_SYSTEM_NAME iPhoneSimulator )
set ( CMAKE_SYSTEM_PROCESSOR i386)
set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/ios/cmake/Modules")

set_property ( GLOBAL PROPERTY TARGET_SUPPORTS_SHARED_LIBS FALSE )

set ( SDK_VERSION "4.3" )
set ( DEVELOPER_ROOT "/Developer/Platforms/iPhoneSimulator.platform/Developer" )
set ( SDK_ROOT "${DEVELOPER_ROOT}/SDKs/iPhoneSimulator${SDK_VERSION}.sdk" )
set ( ARCH "i386" )
set ( CMAKE_OSX_SYSROOT "${SDK_ROOT}" )
set ( CMAKE_OSX_ARCHITECTURES "${ARCH}" )


set ( CMAKE_C_COMPILER "${DEVELOPER_ROOT}/usr/bin/gcc-4.2" )
set ( CMAKE_CXX_COMPILER "${DEVELOPER_ROOT}/usr/bin/g++-4.2" )

# set ( CMAKE_C_FLAGS "-m32 ${CMAKE_C_FLAGS}" CACHE STRING "c flags" )
# set ( CMAKE_CXX_FLAGS "-m32 ${CMAKE_CXX_FLAGS}" CACHE STRING "c++ flags" )

ADD_DEFINITIONS("-m32")
ADD_DEFINITIONS("-arch i386")
ADD_DEFINITIONS("-no-cpp-precomp")
ADD_DEFINITIONS("--sysroot=${SDK_ROOT}")
ADD_DEFINITIONS("-miphoneos-version-min=${SDK_VERSION}")

INCLUDE_DIRECTORIES(SYSTEM "${SDK_ROOT}/usr/include")
INCLUDE_DIRECTORIES(SYSTEM "${SDK_ROOT}/System/Library/Frameworks")

LINK_DIRECTORIES("${SDK_ROOT}/usr/lib")
LINK_DIRECTORIES("${SDK_ROOT}/System/Library/Frameworks")

SET (CMAKE_FIND_ROOT_PATH "${SDK_ROOT}")
SET (CMAKE_FIND_ROOT_PATH_MODE_PROGRAM BOTH)
SET (CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
SET (CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)


message ("iPhoneSimulator toolchain loaded")