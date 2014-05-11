SET(CPACK_DEBIAN_PACKAGE_DEPENDS "libstdc++6 (>= 4.8), apache2 (>= 2.4)")
SET(CPACK_DEBIAN_PACKAGE_ENHANCES "apache2")
SET(CPACK_DEBIAN_PACKAGE_HOMEPAGE "https://github.com/matiu2/cdnalizer")

set(DEB_DIR packages/debian)

configure_file(${DEB_DIR}/mod_cdnalizer.load mod_cdnalizer.load)
file(COPY ${DEB_DIR}/mod_cdnalizer.conf ${DEB_DIR}/prerm DESTINATION .)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/mod_cdnalizer.load ${CMAKE_CURRENT_BINARY_DIR}/mod_cdnalizer.conf DESTINATION /etc/apache2/mods-available/)

configure_file(${DEB_DIR}/postinst postinst)
SET(CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA ${CMAKE_CURRENT_BINARY_DIR}/postinst;${CMAKE_CURRENT_BINARY_DIR}/prerm )
