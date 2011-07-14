if (BUILD_EXAMPLES)
    project(opencl_samples)

    include_directories(
        "${CMAKE_SOURCE_DIR}/modules/core/include"
        "${CMAKE_SOURCE_DIR}/modules/flann/include"
        "${CMAKE_SOURCE_DIR}/modules/imgproc/include"
        "${CMAKE_SOURCE_DIR}/modules/video/include"
        "${CMAKE_SOURCE_DIR}/modules/highgui/include"
        "${CMAKE_SOURCE_DIR}/modules/ml/include"
        "${CMAKE_SOURCE_DIR}/modules/calib3d/include"
        "${CMAKE_SOURCE_DIR}/modules/features2d/include"
        "${CMAKE_SOURCE_DIR}/modules/objdetect/include"
        "${CMAKE_SOURCE_DIR}/modules/legacy/include"
        "${CMAKE_SOURCE_DIR}/modules/contrib/include"
        "${CMAKE_SOURCE_DIR}/modules/ocl/include"
    )   
    
    if(CMAKE_COMPILER_IS_GNUCXX)
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-unused-function")
    endif()
    
    # ---------------------------------------------
    #      Define executable targets
    # ---------------------------------------------
    MACRO(MY_DEFINE_EXAMPLE name srcs)
        set(the_target "example_ocl_${name}")
        add_executable(${the_target} ${srcs})
        set_target_properties(${the_target} PROPERTIES
            OUTPUT_NAME "${name}_ocl"
            PROJECT_LABEL "(EXAMPLE_OPENCL) ${name}")
        add_dependencies(${the_target} opencv_core opencv_flann opencv_imgproc opencv_highgui
            opencv_ml opencv_video opencv_objdetect opencv_features2d
            opencv_calib3d opencv_legacy opencv_contrib opencv_ocl)
        target_link_libraries(${the_target} ${OPENCV_LINKER_LIBS} opencv_core
            opencv_flann opencv_imgproc opencv_highgui opencv_ml opencv_video opencv_objdetect
            opencv_features2d opencv_calib3d opencv_legacy opencv_contrib opencv_ocl)
            
        if(ENABLE_SOLUTION_FOLDERS)
            set_target_properties(${the_target} PROPERTIES FOLDER "samples//opencl")
        endif()

        if(WIN32)
            if (MSVC AND NOT BUILD_SHARED_LIBS)
                set_target_properties(${the_target} PROPERTIES LINK_FLAGS "/NODEFAULTLIB:atlthunk.lib /NODEFAULTLIB:atlsd.lib /DEBUG")
            endif()
            install(TARGETS ${the_target}
                RUNTIME DESTINATION "samples/ocl" COMPONENT main)
        endif()
    ENDMACRO(MY_DEFINE_EXAMPLE)
    
    file(GLOB ocl_samples RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.cpp)
    
    foreach(sample_filename ${ocl_samples})
        get_filename_component(sample ${sample_filename} NAME_WE)
        MY_DEFINE_EXAMPLE(${sample}  ${sample_filename})
    endforeach()
endif(BUILD_EXAMPLES)

if (NOT WIN32)
    file(GLOB OCL_FILES *.c *.cpp *.jpg *.png *.data makefile.* build_all.sh *.dsp *.cmd *.avi)
    install(FILES ${OCL_FILES}
            DESTINATION share/opencv/samples/ocl
            PERMISSIONS OWNER_READ GROUP_READ WORLD_READ)
endif ()

