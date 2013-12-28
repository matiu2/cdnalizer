project(mod_cdnalize)

# Apache and APR include files

find_path(APACHE_INCLUDE_DIR httpd.h)
if(${APACHE_INCLUDE_DIR} STREQUAL "APACHE_INCLUDE_DIR-NOTFOUND") 
        message(FATAL_ERROR "Couldn't find the apache include directory; try: sudo apt-get install apache2-dev")
endif()

find_path(APR_INCLUDE_DIR apr.h)
if(${APR_INCLUDE_DIR} STREQUAL "APR_INCLUDE_DIR-NOTFOUND") 
        message(FATAL_ERROR "Couldn't find the apr (Apache Portable Runtime) include directory; try: sudo apt-get install libapr1-dev")
endif()

include_directories(${APACHE_INCLUDE_DIR} ${APR_INCLUDE_DIR})

# Threads

# TODO(matiu): This might not be needed; I just brought it over from the apx + autotools + libtool build.
find_package(Threads REQUIRED) # Gives us CMAKE_THREAD_LIBS_INIT

# Build the library

add_library(cdnalize SHARED mod_cdnalize.cpp)
