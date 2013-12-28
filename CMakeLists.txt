cmake_minimum_required(VERSION 2.8)

project (cdnalizer)

enable_testing()

set(CMAKE_MODULE_PATH $CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

include_directories(dependencies/bandit)
add_definitions(-std=c++11 -Wall -Wextra)

add_subdirectory(src)
