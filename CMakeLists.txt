cmake_minimum_required(VERSION 2.8)

project (cdnalizer)

enable_testing()

set(CMAKE_MODULE_PATH $CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
include(cmake/get_bandit.cmake)

add_definitions(-std=c++11 -Wall -Wextra)

SET(VERSION 0.1)

# Default to release build, as that's what most people use it for

# Installation settings
set(BUILD_DISTRO DEB
    CACHE STRING "DEB or YUM - Debian/Ubuntu or centos. Currently only Ubuntu >= 13.10 is supported")
if (BUILD_DEB STREQUAL "DEB")
    set(MODULE_INSTALL_DIR "etc/apache2/mods-available")
    set(MODULE_CONF_FILE "${MODULE_INSTALL_DIR}/mod_cdnalizer.conf")
else()
    set(MODULE_INSTALL_DIR "etc/httpd/modules")
    set(MODULE_CONF_FILE "etc/httpd/conf/httpd.conf")
endif()

# Settings
set(STATIC_STDLIB_CXX Off
    CACHE BOOL "Statically link stdlibc++ (for the actual mod_cdnalizer only) (works with g++ only)")
set(USE_SYSTEM_APACHE On
    CACHE BOOL "Find apache on your system, or use the embeded 2.2 headers")

if (USE_SYSTEM_APACHE)
    # APR - Apache Portable Runtime
    find_package(apr REQUIRED)
    # Apache - include files
    find_package(Apache REQUIRED)
    if(${APACHE_VERSION} MATCHES 2.4*)
        set(HAVE_APACHE_2_4)
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
endif()

file(COPY dev-tools DESTINATION .)
configure_file(dev-tools/apache/apache2.conf dev-tools/apache/apache2.conf)

add_subdirectory(src)

include(cmake/package.cmake)
