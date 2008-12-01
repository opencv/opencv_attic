# ----------------------------------------------------------------------------
#  CMake file for python support
#
# ----------------------------------------------------------------------------
project(python_support)

if(WIN32)
    set(PY_SRC_SUFFIX "_win32")
    set(PY_MODULE_SUFFIX ".pyd")
    if(MSVC)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W3")
        set(CMAKE_CXX_FLAGS_DEBUG ${CMAKE_CXX_FLAGS_RELEASE})
        #add_definitions(-DMS_NO_COREDLL)
        #add_definitions(-DPy_BUILD_CORE)
    endif()
endif()

if(UNIX)
    set(PY_MODULE_SUFFIX ".so")
endif()

if(APPLE)
    set(PY_MODULE_SUFFIX ".dylib")
endif()

# List of C++ files:
set(pcv_src 
    _cv${PY_SRC_SUFFIX}.cpp 
    error.cpp 
    error.h 
    pyhelpers.cpp 
    pyhelpers.h 
    pycvseq.hpp 
    cvshadow.cpp 
    cvshadow.h 
#	cv.i 
#	imagedata.i 
#	cvarr.i 
#	nointpb.i 
#	pytypemaps.i 
#	cvseq.i 
#	cvshadow.i
	)

set(pml_src 
    _ml${PY_SRC_SUFFIX}.cpp 
    pyhelpers.cpp 
    pyhelpers.h 
    )

set(phighgui_src 
    _highgui${PY_SRC_SUFFIX}.cpp 
    pyhelpers.cpp 
    pyhelpers.h 
    #highgui.i
    )

add_definitions(-DCV_NO_BACKWARD_COMPATIBILITY)

foreach(P "${PYTHON_INCLUDE_PATH}")            
    include_directories(${P})
endforeach()

# ----------------------------------------------------------------------------------
# 				Cv wrapper
# ----------------------------------------------------------------------------------
add_library(_cv SHARED ${pcv_src})

set_target_properties(_cv PROPERTIES
    OUTPUT_NAME "_cv"
    PREFIX ""
    SUFFIX ${PY_MODULE_SUFFIX}
    )

set_target_properties(_cv PROPERTIES
	ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib/"
	RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/"
#	DEFINE_SYMBOL "CVAPI_EXPORTS"
#	LINK_FLAGS "-fPIC -Bsymbolic-functions -export-dynamic -Bsymbolic-functions -Wl -export-dynamic"
	)

add_dependencies(_cv cxcore cv highgui ml cvaux)
target_link_libraries(_cv ${OPENCV_LINKER_LIBS} ${PYTHON_LIBRARIES})

# ----------------------------------------------------------------------------------
# 				HighGui wrapper
# ----------------------------------------------------------------------------------
add_library(_highgui SHARED ${phighgui_src})

set_target_properties(_highgui PROPERTIES
    OUTPUT_NAME "_highgui${PY_MODULE_SUFFIX}"	
    PREFIX ""
    )

set_target_properties(_highgui PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib/" 
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/" 
#   LINK_FLAGS "-fPIC -Bsymbolic-functions -export-dynamic -Bsymbolic-functions -Wl -export-dynamic "
    )

add_dependencies(_highgui cxcore cv highgui)
target_link_libraries(_highgui ${OPENCV_LINKER_LIBS} ${PYTHON_LIBRARIES})

# ----------------------------------------------------------------------------------
#           ML wrapper
# ----------------------------------------------------------------------------------
add_library(_ml SHARED ${pml_src})

set_target_properties(_ml PROPERTIES
    OUTPUT_NAME "_ml${PY_MODULE_SUFFIX}"
    PREFIX ""
    )

set_target_properties(_ml PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib/"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/"
#   LINK_FLAGS "-fPIC -Bsymbolic-functions -export-dynamic -Bsymbolic-functions -Wl -export-dynamic"
    )

add_dependencies(_ml cxcore ml)
target_link_libraries(_ml ${OPENCV_LINKER_LIBS} ${PYTHON_LIBRARIES})

if(UNIX)  
    get_target_property(LOC_CV _cv LOCATION)
    get_target_property(LOC_ML _ml LOCATION)
    get_target_property(LOC_HG _highgui LOCATION)
    install(FILES ${LOC_CV} ${LOC_ML} ${LOC_HG} DESTINATION ${PYTHON_PLUGIN_INSTALL_PATH})
endif(UNIX)
