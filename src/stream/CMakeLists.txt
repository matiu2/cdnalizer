project (stream)

add_library(stream STATIC stream.cpp)
target_link_libraries(stream base)

add_executable(test_iterator test_iterator.cpp)
add_dependencies(test_iterator bandit)
set_target_properties(test_iterator PROPERTIES
                      INCLUDE_DIRECTORIES "${BANDIT_INCLUDE_DIR}")
target_link_libraries(test_iterator stream)
add_test(test_iterator, test_iterator)
set_target_properties(test_iterator PROPERTIES DEPENDS bandit)
