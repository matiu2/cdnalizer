project (Rewriter)

enable_testing()

add_library(base STATIC Config.cpp)
set_property(TARGET base PROPERTY COMPILE_FLAGS -fPIC) # Because it gets loaded into shared object libraries later

add_subdirectory(stream)
add_subdirectory(standalone)
add_subdirectory(apache)

add_executable(test_rewriteHTML test_rewriteHTML.cpp)
target_link_libraries(test_rewriteHTML stream)
add_test(test_rewriteHTML test_rewriteHTML)
set_target_properties(test_rewriteHTML PROPERTIES DEPENDS bandit)

add_executable(test_config test_config.cpp)
target_link_libraries(test_config stream)
set_property(TARGET test_config PROPERTY COMPILE_FLAGS -fno-access-control) # Allows tests to examine internals of objects
set_target_properties(test_config PROPERTIES DEPENDS bandit)
add_test(test_config test_config)
