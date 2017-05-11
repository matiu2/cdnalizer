project (Rewriter)

enable_testing()

add_library(base STATIC Config.cpp)
set_property(TARGET base PROPERTY COMPILE_FLAGS -fPIC) # Because it gets loaded into shared object libraries later
add_dependencies(base parser_code_generated)

add_subdirectory(parser)
add_subdirectory(stream)
add_subdirectory(standalone)
add_subdirectory(apache)

add_executable(test_config test_config.cpp)
target_link_libraries(test_config base)
set_property(TARGET test_config PROPERTY COMPILE_FLAGS -fno-access-control) # Allows tests to examine internals of objects
add_dependencies(test_config bandit)
set_target_properties(test_config PROPERTIES 
                      INCLUDE_DIRECTORIES "${BANDIT_INCLUDE_DIR}")
add_test(test_config test_config)

add_executable(test_rewriteHTML test_rewriteHTML.cpp)
target_link_libraries(test_rewriteHTML base)
add_dependencies(test_rewriteHTML bandit)
add_dependencies(test_rewriteHTML parser_code_generated)
set_target_properties(test_rewriteHTML PROPERTIES
                      INCLUDE_DIRECTORIES "${BANDIT_INCLUDE_DIR}")
add_test(test_rewriteHTML test_rewriteHTML)

add_executable(test_rewriteHTML_string test_rewriteHTML_string.cpp)
target_link_libraries(test_rewriteHTML_string base)
add_dependencies(test_rewriteHTML_string bandit)
set_target_properties(test_rewriteHTML_string PROPERTIES
                      INCLUDE_DIRECTORIES "${BANDIT_INCLUDE_DIR}")
add_test(test_rewriteHTML_string test_rewriteHTML_string)

add_executable(test_rewriteCSS test_rewriteCSS.cpp)
target_link_libraries(test_rewriteCSS base)
add_dependencies(test_rewriteCSS bandit)
set_target_properties(test_rewriteCSS PROPERTIES
                      INCLUDE_DIRECTORIES "${BANDIT_INCLUDE_DIR}")
add_test(test_rewriteCSS test_rewriteCSS)
