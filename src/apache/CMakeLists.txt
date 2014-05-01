project (mod_cdnalizer)

# Apache ap_config.h thinks that our c++ compiler doesn't support C99. Here we tell it we do (because all our imports are inside of extern "C" blocks anyway).
#add_definitions(-DAP_HAVE_C99 -DAP_HAVE_DESIGNATED_INITIALIZER)

if (HAVE_APACHE_2_4)
    add_definitions(-DHAVE_APACHE_2_4)
endif()

if (STATIC_STDLIB_CXX)
    add_definitions(-static-libstdc++ -static-libgcc)
endif()

add_library(${PROJECT_NAME} SHARED mod_cdnalizer.cpp config.cpp filter.cpp)
target_link_libraries(${PROJECT_NAME} base ${APR_LIBRARIES})
set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "")

INSTALL(
    TARGETS ${PROJECT_NAME}
    DESTINATION ${CMAKE_INSTALL_PREFIX}/share/cdnalizer/
)
