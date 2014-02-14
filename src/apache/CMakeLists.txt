project (mod_cdnalizer)

# Apache ap_config.h thinks that our c++ compiler doesn't support C99. Here we tell it we do (because all our imports are inside of extern "C" blocks anyway).
add_definitions(-DAP_HAVE_C99)

include_directories(${APR_INCLUDE_DIR})
include_directories(${APRUTIL_INCLUDE_DIR})
include_directories(${APACHE_INCLUDE_DIR})

if (HAVE_APACHE_2_4)
    add_definitions(-DHAVE_APACHE_2_4)
endif()

if (STATIC_STDLIB_CXX)
    add_definitions(-static-libstdc++ -static-libgcc)
endif()

add_library(mod_cdnalizer SHARED mod_cdnalizer.cpp config.cpp $<TARGET_OBJECTS:base>)
target_link_libraries(mod_cdnalizer ${APR_LIBRARIES})
set_target_properties(mod_cdnalizer PROPERTIES PREFIX "")

INSTALL(
    TARGETS mod_cdnalizer
    DESTINATION ${MODULE_INSTALL_DIR}
)
