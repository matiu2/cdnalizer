cmake_minimum_required(VERSION 2.8)

project (cdnalizer)

# 0.x versions are for Apache 2.2 and depend only on the pre c++11 c++ runtimes
SET(VERSION 0.3)

set(CMAKE_MODULE_PATH $CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
include(CheckCXXCompilerFlag)

add_definitions(-Wall -Wextra)

include_directories(${APR_INCLUDE_DIR})
include_directories(${APRUTIL_INCLUDE_DIR})
include_directories(${APACHE_INCLUDE_DIR})

# Settings
set(STATIC_STDLIB_CXX Off
    CACHE BOOL "Statically link stdlibc++ (for the actual mod_cdnalizer only) (works with g++ only)")

# APR - Apache Portable Runtime
find_package(apr REQUIRED)
# Apache - include files
find_package(Apache REQUIRED)
if(${APACHE_VERSION} MATCHES 2.4*)
set(HAVE_APACHE_2_4)
endif()

file(COPY dev-tools DESTINATION .)
configure_file(dev-tools/apache/apache2.conf dev-tools/apache/apache2.conf)

add_subdirectory(src)

include(cmake/package.cmake)
