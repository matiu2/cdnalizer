project (standalone)


# Although, theoretically this should be possible on old compilers, to save time for now, we just compile apache module
if(HAVE_CPP11)
    add_executable(rewiter main.cpp)
    target_link_libraries(rewiter stream)
endif()
