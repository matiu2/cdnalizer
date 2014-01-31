project (mod_cdnalizer)

# Apache ap_config.h thinks that our c++ compiler doesn't support C99. Here we tell it we do (because all our imports are inside of extern "C" blocks anyway).
add_definitions(-DAP_HAVE_C99)

add_library(mod_cdnalizer SHARED mod_cdnalizer.cpp config.cpp $<TARGET_OBJECTS:base>)
target_link_libraries(mod_cdnalizer ${APR_LIBRARIES})
set_target_properties(mod_cdnalizer PROPERTIES PREFIX "")
