#
#  CMake file for OpenCV docs
#

set(OPENCV_DOC_INSTALL_PATH ${CMAKE_INSTALL_PREFIX}/share/opencv/doc)

file(GLOB_RECURSE FILES_DOC *.htm *.css *.txt *.jpg *.png *.pdf *.ps)

install(FILES ${FILES_DOC} DESTINATION ${OPENCV_DOC_INSTALL_PATH})
