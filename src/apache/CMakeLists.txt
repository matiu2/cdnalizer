project (mod_cdnalizer)

add_library(mod_cdnalizer SHARED mod_cdnalizer.cpp)
target_link_libraries(mod_cdnalizer ${APR_LIBRARIES})
set_target_properties(mod_cdnalizer PROPERTIES PREFIX "")
