project (Rewriter)

if(HAVE_CPP11)
    enable_testing()
endif()

add_library(base STATIC Config.cpp)
set_property(TARGET base PROPERTY COMPILE_FLAGS -fPIC) # Because it gets loaded into shared object libraries later

add_subdirectory(stream)
add_subdirectory(standalone)
add_subdirectory(apache)

if(HAVE_CPP11)
    add_executable(test_rewriteHTML test_rewriteHTML.cpp)
    target_link_libraries(test_rewriteHTML stream)
    add_dependencies(test_rewriteHTML bandit)
    set_target_properties(test_rewriteHTML PROPERTIES
                  INCLUDE_DIRECTORIES "${BANDIT_INCLUDE_DIR}")
    add_test(test_rewriteHTML test_rewriteHTML)
endif()

if(HAVE_CPP11)
    add_executable(test_config test_config.cpp)
    target_link_libraries(test_config stream)
    set_property(TARGET test_config PROPERTY COMPILE_FLAGS -fno-access-control) # Allows tests to examine internals of objects
    add_dependencies(test_config bandit)
    set_target_properties(test_config PROPERTIES 
                  INCLUDE_DIRECTORIES "${BANDIT_INCLUDE_DIR}")
    add_test(test_config test_config)
endif()
