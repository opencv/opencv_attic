project(traincascade)

link_libraries(cxcore cv cvhaartraining highgui)

set(traincascade_files traincascade.cpp
    cascadeclassifier.cpp cascadeclassifier.h
    boost.cpp boost.h features.cpp features.h
    haarfeatures.cpp haarfeatures.h
    lbpfeatures.cpp lbpfeatures.h
    imagestorage.cpp imagestorage.h)
    
add_executable(traincascade ${traincascade_files})

add_dependencies(traincascade cxcore cv ml)
set_target_properties(traincascade PROPERTIES
                      DEBUG_POSTFIX "${OPENCV_DEBUG_POSTFIX}"
                      OUTPUT_NAME "opencv_traincascade")

target_link_libraries(traincascade cxcore cv ml)

install(TARGETS traincascade RUNTIME DESTINATION bin COMPONENT main)
