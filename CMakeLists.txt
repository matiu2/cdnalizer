cmake_minimum_required(VERSION 2.8)

project (cdnalizer)

enable_testing()

set(CMAKE_MODULE_PATH $CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/dependencies/bandit)

add_definitions(-std=c++11 -Wall -Wextra)

set(USE_SYSTEM_APACHE Off
    CACHE BOOL "Find apache on your system, or use the embeded 2.2 headers")
if (USE_SYSTEM_APACHE)
    # APR - Apache Portable Runtime
    find_package(apr REQUIRED)
    # Apache - include files
    find_package(Apache REQUIRED)
    if(${APACHE_VERSION} MATCHES 2.4*)
        set(HAVE_APACHE_2_4 True)
    else()
        set(HAVE_APACHE_2_4 True)
    endif()
else()
    set(APR_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include/apr-1.3.9)
    set(APRUTIL_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include/apr-util-1.3.9)
    set(APACHE_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include/httpd-2.2.26)
    MARK_AS_ADVANCED(
      APR_LIBRARY
      APRUTIL_INCLUDE_DIR
      APR_INCLUDE_DIR
      APACHE_INCLUDE_DIR
     )
    set(HAVE_APACHE_2_4 False)
endif()

file(COPY dev-tools DESTINATION .)
configure_file(dev-tools/apache/apache2.conf dev-tools/apache/apache2.conf)

add_subdirectory(src)
