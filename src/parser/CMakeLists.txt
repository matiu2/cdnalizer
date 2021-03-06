project (parser)

# Function for generating a x.hpp file from an x.machine.rl file, plus a x.hpp.rl file
macro(add_ragel_file)
    set(one MAIN_FILE)
    set(multi EXTRA_RAGEL_FILES)
    cmake_parse_arguments(ADD_RAGEL "" "${one}" "${multi}" ${ARGN})

    set(extra_depends "")
    foreach(extra_file ${ADD_RAGEL_EXTRA_RAGEL_FILES})
        set(extra_depends ${extra_depends} DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${extra_file})
    endforeach()

    add_custom_command(
        OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/${ADD_RAGEL_MAIN_FILE}.hpp
        COMMAND ragel -C
                -o ${CMAKE_CURRENT_SOURCE_DIR}/${ADD_RAGEL_MAIN_FILE}.hpp 
                ${CMAKE_CURRENT_SOURCE_DIR}/${ADD_RAGEL_MAIN_FILE}.hpp.rl
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${ADD_RAGEL_MAIN_FILE}.hpp.rl
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${ADD_RAGEL_MAIN_FILE}.machine.rl
        ${extra_depends}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )
endmacro()

# Generate all the *.hpp from the ragel files 
add_ragel_file(MAIN_FILE css)
add_ragel_file(MAIN_FILE html)
add_ragel_file(MAIN_FILE path)

add_custom_target(parser_code_generated
  DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/css.hpp
  DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/html.hpp
  DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/path.hpp
)

################################

set(VISUALIZATION_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../docs/state-machines)

FILE(WRITE "${VISUALIZATION_DIR}/index.html" "<body style='font-size: xx-large; font-family: sans'><dl>")

function(add_machine_visualization)
    set(one MAIN_FILE
            MACHINE_NAME)
    set(multi DEPENDENCIES)
    cmake_parse_arguments(VISU "" "${one}" "${multi}" ${ARGN})
    
    set(extra_depends "")
    foreach(extra_file ${VISU_DEPENDENCIES})
        set(extra_depends ${extra_depends} DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${extra_file})
    endforeach()

    add_custom_command(
        OUTPUT ${VISUALIZATION_DIR}/${VISU_MACHINE_NAME}.svg
        COMMAND ragel -Vp -M ${VISU_MACHINE_NAME}
                ${CMAKE_CURRENT_SOURCE_DIR}/${VISU_MAIN_FILE}.machine.rl |
                dot -Tsvg -o ${VISUALIZATION_DIR}/${VISU_MACHINE_NAME}.svg
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${VISU_MAIN_FILE}.machine.rl
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )

file(APPEND "${VISUALIZATION_DIR}/index.html" 
            "<dt>${VISU_MACHINE_NAME}</dt><dd><a href=\"${VISU_MACHINE_NAME}.svg\"><img width=\"320\" height=\"160\" src=\"${VISU_MACHINE_NAME}.svg\" /></dd>")

endfunction()

# Update the blog directory with the latest state machine visualization
add_machine_visualization(MAIN_FILE css MACHINE_NAME css)
add_machine_visualization(MAIN_FILE path MACHINE_NAME path)
add_machine_visualization(MAIN_FILE js MACHINE_NAME js)

FILE(APPEND "${VISUALIZATION_DIR}/index.html" "</body></dl>")

add_custom_target(parser_visualization
    DEPENDS "${VISUALIZATION_DIR}/css.svg"
    DEPENDS "${VISUALIZATION_DIR}/path.svg"
    DEPENDS "${VISUALIZATION_DIR}/js.svg"
    DEPENDS "${VISUALIZATION_DIR}/index.html")

##################################

# Copy the test files for test_html
file(COPY
     ${CMAKE_CURRENT_SOURCE_DIR}/index.html
     ${CMAKE_CURRENT_SOURCE_DIR}/index.html.expected
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
