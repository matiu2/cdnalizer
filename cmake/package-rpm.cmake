SET(CPACK_RPM_PACKAGE_REQUIRES "libstdc++ >= 4.4, httpd >= 2.2")
SET(CPACK_RPM_PACKAGE_URL "https://github.com/matiu2/cdnalizer")
SET(CPACK_RPM_PACKAGE_LICENSE "ASL 2.0")

# Get our conf file that loads the module
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/01-cdnalizer.conf DESTINATION /etc/httpd/conf.d/)

# Copy postinst and postrm scripts
set(RPM packages/redhat)
configure_file(${RPM_DIR}/postinst postinst)
file(COPY ${RPM_DIR}/01-cdnalizer.conf ${RPM_DIR}/postrm DESTINATION .)

# Use postinst and postrm scripts
SET(CPACK_RPM_POST_INSTALL_SCRIPT_FILE ${CMAKE_CURRENT_BINARY_DIR}/postinst )
SET(CPACK_RPM_POST_UNINSTALL_SCRIPT_FILE ${CMAKE_CURRENT_BINARY_DIR}/postrm )
