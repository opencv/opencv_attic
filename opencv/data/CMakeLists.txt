file(GLOB HAAR_CASCADES haarcascades/*.xml)

if(UNIX)
    install(FILES ${HAAR_CASCADES} DESTINATION ${CMAKE_INSTALL_PREFIX}/share/opencv/haarcascades)
endif()
