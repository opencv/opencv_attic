# ----------------------------------------------------------------------------
#  CMake file for C samples. See root CMakeLists.txt
#
# ----------------------------------------------------------------------------

if (BUILD_EXAMPLES)
    project(cpp_samples)

    if(CMAKE_COMPILER_IS_GNUCXX)
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-unused-function")
    endif()
    
    # ---------------------------------------------
    #      Define executable targets
    # ---------------------------------------------
    MACRO(MY_DEFINE_EXAMPLE name srcs)
    	add_executable(${name} ${srcs})
		set_target_properties(${name} PROPERTIES PROJECT_LABEL "(EXAMPLE) ${name}")
    	add_dependencies(${name} cv cxcore highgui ml cvaux)
    	target_link_libraries(${name} ${OPENCV_LINKER_LIBS} cv cxcore highgui ml cvaux)

        if(WIN32)
            install(TARGETS ${name}
                RUNTIME DESTINATION "samples/cpp" COMPONENT main)
        endif()
    ENDMACRO(MY_DEFINE_EXAMPLE)
    
    MY_DEFINE_EXAMPLE(connected_components	connected_components.cpp)
    MY_DEFINE_EXAMPLE(morphology2	morphology2.cpp)
endif(BUILD_EXAMPLES)

if (INSTALL_C_EXAMPLES AND NOT WIN32)
    file(GLOB C_SAMPLES *.c *.cpp *.jpg *.png *.data makefile.* build_all.sh *.dsp *.cmd )
    install(FILES ${C_SAMPLES}
            DESTINATION share/opencv/samples/cpp
            PERMISSIONS OWNER_READ GROUP_READ WORLD_READ)
endif ()

