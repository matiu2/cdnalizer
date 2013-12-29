project (mod_cdnalizer)

# APR - Apache Portable Runtime
find_package(apr REQUIRED)
include_directories(${APR_INCLUDE_DIR})

# Apache - include files
find_package(Apache REQUIRED)
include_directories(${APACHE_INCLUDE_DIR})

add_library(mod_cdnalizer SHARED mod_cdnalizer.cpp)
target_link_libraries(mod_cdnalizer ${APR_LIBRARIES})
set_target_properties(mod_cdnalizer PROPERTIES PREFIX "")
