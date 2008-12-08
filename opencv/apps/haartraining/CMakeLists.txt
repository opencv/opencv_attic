project(haartraining)

include_directories("${CMAKE_CURRENT_SOURCE_DIR}")

# -----------------------------------------------------------
#  Library
# -----------------------------------------------------------
set(cvhaartraining_lib_src
    _cvcommon.h
    cvclassifier.h
    _cvhaartraining.h
    cvhaartraining.h
    cvboost.cpp
    cvcommon.cpp
    cvhaarclassifier.cpp
    cvhaartraining.cpp
    cvsamples.cpp
    )

add_library(cvhaartraining STATIC ${cvhaartraining_lib_src})

# -----------------------------------------------------------
#  haartraining
# -----------------------------------------------------------

add_executable(haartraining cvhaartraining.h haartraining.cpp)
add_dependencies(haartraining cxcore cv cvhaartraining)
target_link_libraries(haartraining cxcore cv cvhaartraining highgui)
set_target_properties(haartraining PROPERTIES
                      DEBUG_POSTFIX "d"
                      OUTPUT_NAME "opencv-haartraining")

# -----------------------------------------------------------
#  createsamples
# -----------------------------------------------------------

add_executable(createsamples cvhaartraining.h createsamples.cpp)
add_dependencies(createsamples cxcore cv cvhaartraining)
target_link_libraries(createsamples cxcore cv cvhaartraining highgui)
set_target_properties(createsamples PROPERTIES
                      DEBUG_POSTFIX "d"
                      OUTPUT_NAME "opencv-createsamples")

# -----------------------------------------------------------
#  performance
# -----------------------------------------------------------
add_executable(performance performance.cpp)
add_dependencies(performance cxcore cv highgui cvhaartraining)
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

if(UNIX)
    install(FILES ${HAAR_CASCADES} DESTINATION ${CMAKE_INSTALL_PREFIX}/share/opencv/haarcascades)
endif()
