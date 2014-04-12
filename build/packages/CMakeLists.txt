project(packages)

set(PACKAGE_TO_BUILD NONE CACHE STRING "Type of package to build when we run 'make package': DEB, YUM, or NONE")

#add_subdirectory(centos)
if(PACKAGE_TO_BUILD STREQUAL DEB)
    add_subdirectory(debian)
endif()
