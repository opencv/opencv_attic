project(haartraining)

IF(WIN32)
link_directories(${CMAKE_SOURCE_DIR}/lib ${CMAKE_SOURCE_DIR}/bin .)
ENDIF(WIN32)
link_libraries(cxcore cv cvhaartraining highgui)

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
set_target_properties(haartraining PROPERTIES
                      DEBUG_POSTFIX "${OPENCV_DEBUG_POSTFIX}"
                      OUTPUT_NAME "opencv_haartraining")

# -----------------------------------------------------------
#  createsamples
# -----------------------------------------------------------

add_executable(createsamples cvhaartraining.h createsamples.cpp)
add_dependencies(createsamples cxcore cv cvhaartraining)
set_target_properties(createsamples PROPERTIES
                      DEBUG_POSTFIX "${OPENCV_DEBUG_POSTFIX}"
                      OUTPUT_NAME "opencv_createsamples")

# -----------------------------------------------------------
#  performance
# -----------------------------------------------------------
add_executable(performance performance.cpp)
add_dependencies(performance cxcore cv highgui cvhaartraining)
set_target_properties(performance PROPERTIES
                      DEBUG_POSTFIX "${OPENCV_DEBUG_POSTFIX}"
                      OUTPUT_NAME "opencv_performance")

# -----------------------------------------------------------
#  Install part
# -----------------------------------------------------------

install(TARGETS haartraining RUNTIME DESTINATION bin COMPONENT main)
install(TARGETS createsamples RUNTIME DESTINATION bin COMPONENT main)
install(TARGETS performance RUNTIME DESTINATION bin COMPONENT main)
