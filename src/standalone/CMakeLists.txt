project (standalone)

add_executable(rewiter main.cpp $<TARGET_OBJECTS:base> $<TARGET_OBJECTS:stream>)
