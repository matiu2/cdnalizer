cmake_minimum_required(VERSION 2.8)

project (cdnalizer)

enable_testing()

set(CMAKE_MODULE_PATH $CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/dependencies/bandit)
add_definitions(-std=c++11 -Wall -Wextra)

# APR - Apache Portable Runtime
find_package(apr REQUIRED)
include_directories(${APR_INCLUDE_DIR})

# Apache - include files
find_package(Apache REQUIRED)
include_directories(${APACHE_INCLUDE_DIR})

file(COPY dev-tools DESTINATION .)
configure_file(dev-tools/apache/apache2.conf dev-tools/apache/apache2.conf)


add_subdirectory(src)
