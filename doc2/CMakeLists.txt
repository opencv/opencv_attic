project(opencv_refman)

set(refman_m4_srcs CxCore.tex.m4 CvReference.tex.m4 HighGui.tex.m4)
set(refman_srcs CxCore-cpp.tex Cv-cpp.tex CvAux.tex HighGui-cpp.tex MachineLearning.tex opencv.tex opencv.sty helvetica.sty opencv.bib)
set(refman_copied_srcs)

foreach(f ${refman_m4_srcs})
    get_filename_component(basef ${f} NAME_WE)
    set(outputf ${CMAKE_CURRENT_BINARY_DIR}/${basef}-c.tex)
    add_custom_command(
        OUTPUT ${outputf}
        DEPENDS ${f}
        COMMAND m4
        ARGS -DTARGET_LANGUAGE=c ${f} > ${outputf}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        VERBATIM
        )
    list(APPEND refman_copied_srcs ${outputf})
endforeach()

foreach(f ${refman_srcs})
    set(outputf ${CMAKE_CURRENT_BINARY_DIR}/${f})
    add_custom_command(
        OUTPUT ${outputf}
        DEPENDS ${f}
        COMMAND ${CMAKE_COMMAND}
        ARGS -E copy ${f} ${outputf}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
    list(APPEND refman_copied_srcs ${outputf})
endforeach()

set(pdflatex_args -interaction=batchmode "'\\newcommand{\\genc}{true}\\newcommand{\\genpy}{true}\\input{opencv.tex}'")

add_custom_command(
  OUTPUT    ${CMAKE_CURRENT_BINARY_DIR}/opencv.aux
  DEPENDS   ${refman_copied_srcs}
  COMMAND   ${PDFLATEX_COMPILER}
  ARGS      ${pdflatex_args}
  COMMENT   "Latex (first pass)"
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
  )

add_custom_command(
  OUTPUT    ${CMAKE_CURRENT_BINARY_DIR}/opencv.bbl
  DEPENDS   ${CMAKE_CURRENT_BINARY_DIR}/opencv.aux
  COMMAND   ${BIBTEX_COMPILER}
  ARGS      ${CMAKE_CURRENT_BINARY_DIR}/opencv
  COMMENT   "Bibtex"
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
  )

add_custom_command(
  OUTPUT    ${CMAKE_CURRENT_BINARY_DIR}/opencv.pdf
  DEPENDS   ${CMAKE_CURRENT_BINARY_DIR}/opencv.bbl
  COMMAND   ${PDFLATEX_COMPILER}
  ARGS      ${pdflatex_args}
  COMMENT   "Latex (second pass)"
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
  )

add_custom_command(
  OUTPUT    ${CMAKE_CURRENT_BINARY_DIR}/opencv.log
  DEPENDS   ${CMAKE_CURRENT_BINARY_DIR}/opencv.bbl
            ${CMAKE_CURRENT_BINARY_DIR}/opencv.pdf
  COMMAND   ${PDFLATEX_COMPILER}
  ARGS      ${pdflatex_args}
  COMMENT   "Latex (third pass)"
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
  )

add_custom_target(MakePicsLink
   COMMAND ln -sf ${CMAKE_CURRENT_SOURCE_DIR}/pics ${CMAKE_CURRENT_BINARY_DIR}/pics
   )
  
# Eventually trigger the whole process
add_custom_target(ReferenceManual ALL echo
   DEPENDS   ${CMAKE_CURRENT_BINARY_DIR}/opencv.log
   )

add_dependencies(ReferenceManual MakePicsLink)
