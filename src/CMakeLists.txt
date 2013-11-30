project (Rewriter)

enable_testing()

add_subdirectory(stream)
add_subdirectory(standalone)

add_executable(test_rewriteHTML test_rewriteHTML.cpp $<TARGET_OBJECTS:stream>)
add_test(test_rewriteHTML test_rewriteHTML)
