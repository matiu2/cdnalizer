cmake_minimum_required(VERSION 2.8)

project (cdnalizer)

add_definitions(-std=c++11 -Wall -Wextra)

add_subdirectory(src)
add_subdirectory(apache)
