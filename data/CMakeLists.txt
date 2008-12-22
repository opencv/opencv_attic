file(GLOB HAAR_CASCADES haarcascades/*.xml)

if(WIN32)
install(FILES ${HAAR_CASCADES} DESTINATION data/haarcascades COMPONENT dev)
else()
install(FILES ${HAAR_CASCADES} DESTINATION share/opencv/haarcascades COMPONENT dev)
endif()


