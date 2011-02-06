#notice the "recycling" of CMAKE_C_FLAGS
#this is necessary to pick up android flags
set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -pedantic -fPIC" )

find_package(OpenCV REQUIRED)

if(NOT ANDROID)
#jni is available by default on android
find_package(JNI REQUIRED)
include_directories(${JNI_INCLUDE_DIRS})
FIND_PACKAGE(SWIG)
endif()

INCLUDE(${SWIG_USE_FILE})

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})

set( LIBRARY_DEPS ${OpenCV_LIBS})

if(ANDROID)
set( LIBRARY_DEPS ${LIBRARY_DEPS} GLESv2 log)
endif()

#this is compiled staticly, and linked into the swig module
add_library(androidcv STATIC
Calibration.cpp
gl_code.cpp
image_pool.cpp
yuv420sp2rgb.c)


if(ANDROID)
#this will set the output path for the java package
#and properly create the package declarations in generated java sources
SET_SWIG_JAVA_PACKAGE( com.opencv.jni ) #defined in the android toolchain
endif(ANDROID)

SET_SOURCE_FILES_PROPERTIES(android-cv.i PROPERTIES CPLUSPLUS ON)
SWIG_ADD_MODULE(android-opencv java android-cv.i)
target_link_libraries(android-opencv androidcv ${LIBRARY_DEPS} )
