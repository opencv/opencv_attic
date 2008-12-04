project(haartraining)

include_directories(src)
include_directories(include)

# -----------------------------------------------------------
#  Library 
# -----------------------------------------------------------
set(cvhaartraining_lib_src
    src/_cvcommon.h
    src/cvclassifier.h    
    src/_cvhaartraining.h
    include/cvhaartraining.h
    src/cvboost.cpp
    src/cvcommon.cpp
    src/cvhaarclassifier.cpp
    src/cvhaartraining.cpp
    src/cvsamples.cpp
    )

add_library(cvhaartraining STATIC ${cvhaartraining_lib_src})

# -----------------------------------------------------------
#  haartraining 
# -----------------------------------------------------------

add_executable(haartraining include/cvhaartraining.h src/haartraining.cpp)
add_dependencies(haartraining cxcore cv cvhaartraining)
target_link_libraries(haartraining cxcore cv cvhaartraining highgui)
set_target_properties(haartraining PROPERTIES
                      DEBUG_POSTFIX "d"
                      OUTPUT_NAME "opencv-haartraining")

# -----------------------------------------------------------
#  createsamples 
# -----------------------------------------------------------

add_executable(createsamples include/cvhaartraining.h src/createsamples.cpp)
add_dependencies(createsamples cxcore cv cvhaartraining)
target_link_libraries(createsamples cxcore cv cvhaartraining highgui)
set_target_properties(createsamples PROPERTIES
                      DEBUG_POSTFIX "d"
                      OUTPUT_NAME "opencv-createsamples")

# -----------------------------------------------------------
#  performance 
# -----------------------------------------------------------
add_executable(performance src/performance.cpp)
add_dependencies(performance cxcore cv cvhaartraining)
target_link_libraries(performance cxcore cv cvhaartraining highgui)
set_target_properties(performance PROPERTIES
                      DEBUG_POSTFIX "d"
                      OUTPUT_NAME "opencv-performance")

# -----------------------------------------------------------
#  Install part
# -----------------------------------------------------------

install(TARGETS haartraining RUNTIME DESTINATION bin)
install(TARGETS createsamples RUNTIME DESTINATION bin)
install(TARGETS performance RUNTIME DESTINATION bin)

file(GLOB HAAR_CASCADES ${CMAKE_SOURCE_DIR}/data/haarcascades/*.xml)

install(FILES ${HAAR_CASCADES} DESTINATION ${CMAKE_INSTALL_PREFIX}/share/opencv/haarcascades)
