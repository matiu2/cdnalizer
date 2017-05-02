project (parser)

# Generate css.hpp source from the ragel template
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/css.hpp
    PRE_BUILD
    COMMAND ragel -C -o ${CMAKE_CURRENT_SOURCE_DIR}/css.hpp ${CMAKE_CURRENT_SOURCE_DIR}/css.hpp.rl;
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/css.hpp.rl
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/css.machine.rl
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

add_custom_target(parser_code_generated
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/css.hpp)

################################

# Update the blog directory with the latest css state machine pic
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/../../blog/css_state_machine.png
    PRE_BUILD
    COMMAND ragel -Vp ${CMAKE_CURRENT_SOURCE_DIR}/css.machine.rl | dot -Tpng -o ${CMAKE_CURRENT_SOURCE_DIR}/../../blog/css_state_machine.png;
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/css.machine.rl
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

# Update the blog directory with the latest html state machine pic
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/../../blog/html_state_machine.png
    PRE_BUILD
    COMMAND ragel -Vp ${CMAKE_CURRENT_SOURCE_DIR}/html.machine.rl | dot -Tpng -o ${CMAKE_CURRENT_SOURCE_DIR}/../../blog/html_state_machine.png;
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/html.machine.rl
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)
 
add_custom_target(parser_visualization 
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/../../blog/css_state_machine.png
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/../../blog/html_state_machine.png
)

##################################
