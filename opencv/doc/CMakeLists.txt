#
#  CMake file for OpenCV docs
#

set(OPENCV_DOC_INSTALL_PATH ${CMAKE_INSTALL_PREFIX}/share/opencv/doc)

file(GLOB FILES_DOC *.htm *.txt *.jpg *.png *.pdf)
file(GLOB FILES_DOC_PAPERS papers/*.pdf papers/*.ps)
file(GLOB FILES_DOC_REF ref/*.htm ref/*.css)
file(GLOB FILES_DOC_REF_PICS ref/pics/*.png ref/pics/*.jpg)

install(FILES ${FILES_DOC} DESTINATION ${OPENCV_DOC_INSTALL_PATH})
install(FILES ${CMAKE_SOURCE_DIR}/apps/haartraining/doc/haartraining.htm DESTINATION ${OPENCV_DOC_INSTALL_PATH})
install(FILES ${FILES_DOC_PAPERS} DESTINATION ${OPENCV_DOC_INSTALL_PATH}/papers)
install(FILES ${FILES_DOC_REF} DESTINATION ${OPENCV_DOC_INSTALL_PATH}/ref)
install(FILES ${FILES_DOC_REF_PICS} DESTINATION ${OPENCV_DOC_INSTALL_PATH}/ref/pics)
