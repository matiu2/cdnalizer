cmake_minimum_required(VERSION 2.8)

project (cdnalizer)

set(CTEST_USE_LAUNCHERS 1)
include( CTest )
enable_testing()

set(CMAKE_MODULE_PATH $CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
include(cmake/get_bandit.cmake)

add_definitions(-std=c++11 -Wall -Wextra)

# 1.x versions are for Apache 2.4 and built with c++11
SET(VERSION 1.3)

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
