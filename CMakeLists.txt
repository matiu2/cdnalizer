cmake_minimum_required(VERSION 2.8)

project (cdnalizer)

enable_testing()

include_directories(dependencies/bandit)
add_definitions(-std=c++11 -Wall -Wextra)

add_subdirectory(src)
