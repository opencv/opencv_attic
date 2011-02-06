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
set( LIBRARY_DEPS ${LIBRARY_DEPS} log)
endif()

#this is compiled staticly, and linked into the swig module
add_library(cvsample STATIC
cvsample.cpp)


if(ANDROID)
#this will set the output path for the java package
#and properly create the package declarations in generated java sources
SET_SWIG_JAVA_PACKAGE( com.OpenCV_SAMPLE.jni ) #defined in the android toolchain
endif(ANDROID)

set(android_opencv_dir ${CMAKE_SOURCE_DIR}/../../android-opencv CACHE PATH "the location of the android-opencv directory")
#this add's the swig path for the opencv wrappers
SET(CMAKE_SWIG_FLAGS ${CMAKE_SWIG_FLAGS} "-I${android_opencv_dir}/jni" )
SET_SOURCE_FILES_PROPERTIES(OpenCV_SAMPLE.i PROPERTIES CPLUSPLUS ON)
SWIG_ADD_MODULE(OpenCV_SAMPLE java OpenCV_SAMPLE.i)
target_link_libraries(OpenCV_SAMPLE cvsample ${LIBRARY_DEPS} )
