if(UNIX)
    file(GLOB opencv_hdrs opencv/*.h*)
    install(FILES ${opencv_hdrs} DESTINATION "${CMAKE_INSTALL_PREFIX}/include/opencv")
endif()
