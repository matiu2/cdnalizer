cmake_minimum_required(VERSION 2.8)

project (cdnalizer)

SET(VERSION 0.2)

set(CMAKE_MODULE_PATH $CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
include(CheckCXXCompilerFlag)

CHECK_CXX_COMPILER_FLAG(-std=c++11 HAVE_CPP11_FLAG)

if(HAVE_CPP11)
    enable_testing()
    include(get_bandit)
    add_definitions(-std=c++11 -DHAVE_CPP11)
endif()

add_definitions(-Wall -Wextra)

include_directories(${APR_INCLUDE_DIR})
include_directories(${APRUTIL_INCLUDE_DIR})
include_directories(${APACHE_INCLUDE_DIR})

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
