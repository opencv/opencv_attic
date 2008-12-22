#
#  CMake file for OpenCV docs
#

file(GLOB FILES_DOC *.htm *.txt *.jpg *.png *.pdf)
file(GLOB FILES_DOC_PAPERS papers/*.pdf papers/*.ps)
file(GLOB FILES_DOC_VS vidsurv/*.doc)
file(GLOB FILES_DOC_REF ref/*.htm ref/*.css)
file(GLOB FILES_DOC_REF_PICS ref/pics/*.png ref/pics/*.jpg)

install(FILES ${FILES_DOC} DESTINATION "${OPENCV_DOC_INSTALL_PATH}" COMPONENT doc)
install(FILES ${FILES_DOC_PAPERS} DESTINATION "${OPENCV_DOC_INSTALL_PATH}/papers" COMPONENT doc)
install(FILES ${FILES_DOC_VS} DESTINATION "${OPENCV_DOC_INSTALL_PATH}/vidsurv" COMPONENT doc)
install(FILES ${FILES_DOC_REF} DESTINATION "${OPENCV_DOC_INSTALL_PATH}/ref" COMPONENT doc)
install(FILES ${FILES_DOC_REF_PICS} DESTINATION "${OPENCV_DOC_INSTALL_PATH}/ref/pics" COMPONENT doc)
