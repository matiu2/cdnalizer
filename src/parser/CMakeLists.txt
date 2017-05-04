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

# Generate html.hpp source from the ragel template
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/html.hpp
    PRE_BUILD
    COMMAND ragel -C -o ${CMAKE_CURRENT_SOURCE_DIR}/html.hpp ${CMAKE_CURRENT_SOURCE_DIR}/html.hpp.rl;
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/html.hpp.rl
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/html.machine.rl
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

# Generate path.hpp source from the ragel template
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/path.hpp
    PRE_BUILD
    COMMAND ragel -C -o ${CMAKE_CURRENT_SOURCE_DIR}/path.hpp ${CMAKE_CURRENT_SOURCE_DIR}/path.hpp.rl;
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/path.hpp.rl
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/path.machine.rl
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)


add_custom_target(parser_code_generated
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/css.hpp
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/html.hpp
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/path.hpp
)

################################

set(BLOG_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../blog)

# Update the blog directory with the latest css state machine pic
add_custom_command(
    OUTPUT ${BLOG_DIR}/css_state_machine.png
    PRE_BUILD
    COMMAND ragel -Vp -M css ${CMAKE_CURRENT_SOURCE_DIR}/css.machine.rl | dot -Tpng -o ${BLOG_DIR}/css_state_machine.png;
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/css.machine.rl
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

# Update the blog directory with the latest html state machine pic
add_custom_command(
    OUTPUT ${BLOG_DIR}/html_state_machine.png
    PRE_BUILD
    COMMAND ragel -Vp -M html ${CMAKE_CURRENT_SOURCE_DIR}/html.machine.rl | dot -Tpng -o ${BLOG_DIR}/html_state_machine.png;
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/html.machine.rl
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

# HTML machine visualization, but just the attrib part
add_custom_command(
    OUTPUT ${BLOG_DIR}/html_attrib_state_machine.png
    PRE_BUILD
    COMMAND ragel -Vp -M attrib ${CMAKE_CURRENT_SOURCE_DIR}/html.machine.rl | dot -Tpng -o ${BLOG_DIR}/html_attrib_state_machine.png;
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/html.machine.rl
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)


# Update the blog directory with the latest path state machine pic
add_custom_command(
    OUTPUT ${BLOG_DIR}/path_state_machine.png
    PRE_BUILD
    COMMAND ragel -Vp -M path ${CMAKE_CURRENT_SOURCE_DIR}/path.machine.rl | dot -Tpng -o ${BLOG_DIR}/path_state_machine.png;
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/path.machine.rl
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)
 
add_custom_target(parser_visualization 
    DEPENDS ${BLOG_DIR}/css_state_machine.png
    DEPENDS ${BLOG_DIR}/html_state_machine.png
    DEPENDS ${BLOG_DIR}/html_attrib_state_machine.png
    DEPENDS ${BLOG_DIR}/path_state_machine.png
)

##################################
