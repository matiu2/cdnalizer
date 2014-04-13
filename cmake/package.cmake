set(PACKAGE_TO_BUILD NONE CACHE STRING "Type of package to build when we run 'make package': DEB, YUM, or NONE")
SET(CPACK_GENERATOR "${PACKAGE_TO_BUILD}" CACHE STRING "Don't set this, set PACKAGE_TO_BUILD" FORCE)

SET(CPACK_PACKAGE_NAME "cdnalizer")
SET(CPACK_PACKAGE_VERSION "${VERSION}")
SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Apache filter that rewrites outging HTML to refer to CDN links")
SET(CPACK_PACKAGE_CONTACT "Matthew Sherborne <matt.sherborne@rackspace.com>")
SET(CPACK_OUTPUT_FILE_PREFIX packages)

#add_subdirectory(centos)
if(PACKAGE_TO_BUILD STREQUAL DEB)
    include(cmake/package-deb.cmake)
endif()

INCLUDE(CPack)
