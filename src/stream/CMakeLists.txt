project (stream)

add_library(stream OBJECT stream.cpp)

add_executable(test_iterator test_iterator.cpp $<TARGET_OBJECTS:base> $<TARGET_OBJECTS:stream>)
add_test(test_iterator, test_iterator)
set_target_properties(test_iterator PROPERTIES DEPENDS bandit)
