project (mod_cdnalizer)

# Apache ap_config.h thinks that our c++ compiler doesn't support C99. Here we tell it we do (because all our imports are inside of extern "C" blocks anyway).
add_definitions(-DAP_HAVE_C99)

include_directories(${APR_INCLUDE_DIR})
include_directories(${APRUTIL_INCLUDE_DIR})
include_directories(${APACHE_INCLUDE_DIR})

if (STATIC_STDLIB_CXX)
    add_definitions(-static-libstdc++ -static-libgcc)
endif()

add_library(${PROJECT_NAME} SHARED mod_cdnalizer.cpp config.cpp filter.cpp)
target_link_libraries(${PROJECT_NAME} base ${APR_LIBRARIES})
set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "")

add_executable(test_block_iterator test_block_iterator.cpp)
add_test(test_block_iterator test_block_iterator)

INSTALL(
    TARGETS ${PROJECT_NAME}
    DESTINATION ${CMAKE_INSTALL_PREFIX}/share/cdnalizer/
)
