INCLUDE(CPack)

SET(CPACK_PACKAGE_NAME "cdnalizer")
SET(CPACK_PACKAGE_VERSION "${VERSION}")
SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Apache filter that rewrites outging HTML to refer to CDN links")
SET(CPACK_GENERATOR "DEB")
SET(CPACK_PACKAGE_CONTACT "Matthew Sherborne <matt.sherborne@rackspace.com>")
SET(CPACK_DEBIAN_PACKAGE_DEPENDS "libc6 (>= 2.17), libgcc1 (>= 1:4.8.1) apache2 (>= 2.4)")
SET(CPACK_DEBIAN_PACKAGE_ENHANCES "apache2")
SET(CPACK_DEBIAN_PACKAGE_HOMEPAGE "https://github.com/matiu2/cdnalizer")


INSTALL(CODE "execute_process(COMMAND a2enmod mod_cdnalizer)")
