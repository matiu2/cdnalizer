project (stream)

# Although, theoretically this should be possible on old compilers, to save time for now, we just compile apache module
if(HAVE_CPP11)
    add_library(stream STATIC stream.cpp)
    target_link_libraries(stream base)
endif()

# All tests need c++11, because of the 'bandit' testing suite
if(HAVE_CPP11)
    add_executable(test_iterator test_iterator.cpp)
    add_dependencies(test_iterator bandit)
    set_target_properties(test_iterator PROPERTIES
                          INCLUDE_DIRECTORIES "${BANDIT_INCLUDE_DIR}")
    target_link_libraries(test_iterator stream)
    add_test(test_iterator, test_iterator)
    set_target_properties(test_iterator PROPERTIES DEPENDS bandit)
endif()
