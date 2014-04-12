cmake_minimum_required(VERSION 2.8)

project (cdnalizer)

enable_testing()

set(CMAKE_MODULE_PATH $CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
include(cmake/get_bandit.cmake)

add_definitions(-std=c++11 -Wall -Wextra)

SET(VERSION 0.1)

# Default to release build, as that's what most people use it for

# Settings
set(STATIC_STDLIB_CXX Off
    CACHE BOOL "Statically link stdlibc++ (for the actual mod_cdnalizer only) (works with g++ only)")
set(USE_SYSTEM_APACHE On
    CACHE BOOL "Find apache on your system, or use the embeded 2.2 headers")

# APR - Apache Portable Runtime
find_package(apr REQUIRED)
# Apache - include files
find_package(Apache REQUIRED)
if(NOT ${APACHE_VERSION} MATCHES 2.4*)
    message(FATAL_ERROR "Cannot find Apache 2.4 - Found: ${APACHE_VERSION}")
endif()

file(COPY dev-tools DESTINATION .)
configure_file(dev-tools/apache/apache2.conf dev-tools/apache/apache2.conf)

add_subdirectory(src)

add_subdirectory(packages)
