cmake_minimum_required(VERSION 2.8)

project (cdnalizer)

set(CTEST_USE_LAUNCHERS 1)
include( CTest )
enable_testing()

set(CMAKE_MODULE_PATH $CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
include(cmake/get_bandit.cmake)

add_definitions(-std=c++1z -Wall -Wextra)

# 1.4 is for Apache 2.4 and built with c++11
# 1.5 is for Apache 2.4 and built with c++14 - it does a straight search and replace, instead of looking for tags
SET(VERSION 1.5)

# Default to release build, as that's what most people use it for

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

include(cmake/package.cmake)
