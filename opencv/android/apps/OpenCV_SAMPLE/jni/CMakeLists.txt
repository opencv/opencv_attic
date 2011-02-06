#notice the "recycling" of CMAKE_C_FLAGS
#this is necessary to pick up android flags
set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -pedantic -fPIC" )

find_package(OpenCV REQUIRED)
FIND_PACKAGE(AndroidOpenCV REQUIRED )

if(NOT ANDROID)
#jni is available by default on android
find_package(JNI REQUIRED)
include_directories(${JNI_INCLUDE_DIRS})
FIND_PACKAGE(SWIG)
endif()

INCLUDE(${SWIG_USE_FILE})

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})

set( LIBRARY_DEPS ${AndroidOpenCV_LIBS} ${OpenCV_LIBS} )

if(ANDROID)
set( LIBRARY_DEPS ${LIBRARY_DEPS} log)
#this will set the output path for the java package
#and properly create the package declarations in generated java sources
SET_SWIG_JAVA_PACKAGE( com.OpenCV_SAMPLE.jni ) #defined in the android toolchain
endif(ANDROID)

#this add's the swig path for the opencv wrappers
SET(CMAKE_SWIG_FLAGS ${CMAKE_SWIG_FLAGS} "-I${AndroidOpenCV_SWIG_DIR}" )
SET_SOURCE_FILES_PROPERTIES(OpenCV_SAMPLE.i PROPERTIES CPLUSPLUS ON)
SWIG_ADD_MODULE(OpenCV_SAMPLE java 
                OpenCV_SAMPLE.i #swig file
                cvsample.cpp #cpp files can be compiled to
                )
                
target_link_libraries(OpenCV_SAMPLE ${LIBRARY_DEPS} )
