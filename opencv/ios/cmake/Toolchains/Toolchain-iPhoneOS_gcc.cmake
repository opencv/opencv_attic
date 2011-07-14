message ( "Setting up iPhoneOS toolchain" )

set ( CMAKE_SYSTEM_NAME iPhoneOS )
set ( CMAKE_SYSTEM_PROCESSOR arm )
set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/ios/cmake/Modules")

set ( SDK_VERSION "4.3" )
set ( DEVELOPER_ROOT "/Developer/Platforms/iPhoneOS.platform/Developer" )
set ( SDK_ROOT "${DEVELOPER_ROOT}/SDKs/iPhoneOS${SDK_VERSION}.sdk" )
set ( ARCH "armv6 armv7" )
set ( CMAKE_OSX_SYSROOT "${SDK_ROOT}" )
set ( CMAKE_OSX_ARCHITECTURES "armv6" "armv7" )

# Skip the platform compiler checks for cross compiling 
set (CMAKE_CXX_COMPILER_WORKS TRUE)
set (CMAKE_C_COMPILER_WORKS TRUE)

set ( CMAKE_C_COMPILER "${DEVELOPER_ROOT}/usr/bin/gcc-4.2" )
set ( CMAKE_CXX_COMPILER "${DEVELOPER_ROOT}/usr/bin/g++-4.2" )

set ( CMAKE_C_FLAGS "-arch armv6 -arch armv7 -no-cpp-precomp -mthumb --sysroot=${SDK_ROOT} -miphoneos-version-min=${SDK_VERSION}" CACHE STRING "c flags" )
set ( CMAKE_CXX_FLAGS "-arch armv6 -arch armv7 -no-cpp-precomp -mthumb --sysroot=${SDK_ROOT} -miphoneos-version-min=${SDK_VERSION}" CACHE STRING "c++ flags" )
set ( CMAKE_SHARED_LINKER_FLAGS "-isysroot=${SDK_ROOT}" ${CMAKE_SHARED_LINKER_FLAGS} )

INCLUDE_DIRECTORIES(SYSTEM "${SDK_ROOT}/usr/include")
INCLUDE_DIRECTORIES(SYSTEM "${SDK_ROOT}/System/Library/Frameworks")

LINK_DIRECTORIES("${SDK_ROOT}/usr/lib")
LINK_DIRECTORIES("${SDK_ROOT}/usr/lib/system")
LINK_DIRECTORIES("${SDK_ROOT}/System/Library/Frameworks")


# search for programs in the build host directories
SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM BOTH)
# for libraries and headers in the target directories
SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

message ("iPhoneOS toolchain loaded ${CMAKE_SYSTEM_PROCESSOR}")