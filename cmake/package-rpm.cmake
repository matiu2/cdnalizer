SET(CPACK_RPM_PACKAGE_REQUIRES "libstdc++ >= 4.8, httpd >= 2.4")
SET(CPACK_RPM_PACKAGE_URL "https://github.com/matiu2/cdnalizer")
SET(CPACK_RPM_PACKAGE_LICENSE "ASL 2.0")

# Copy postinst and postrm scripts
set(RPM_DIR ${CMAKE_CURRENT_SOURCE_DIR}/packages/redhat)
configure_file(${RPM_DIR}/postinst postinst)
file(COPY ${RPM_DIR}/01-cdnalizer.conf ${RPM_DIR}/postrm DESTINATION .)

# Get our conf file that loads the module
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/01-cdnalizer.conf DESTINATION /etc/httpd/conf.d/)

# Use postinst and postrm scripts
SET(CPACK_RPM_POST_INSTALL_SCRIPT_FILE ${CMAKE_CURRENT_BINARY_DIR}/postinst )
SET(CPACK_RPM_POST_UNINSTALL_SCRIPT_FILE ${CMAKE_CURRENT_BINARY_DIR}/postrm )
