SET(CPACK_RPM_PACKAGE_REQUIRES "libstdc++ >= 4.8, httpd >= 2.4")
SET(CPACK_RPM_PACKAGE_URL "https://github.com/matiu2/cdnalizer")
SET(CPACK_RPM_PACKAGE_LICENSE "ASL 2.0")

set(YUM_DIR packages/redhat)

file(COPY ${YUM_DIR}/01-cdnalizer.conf ${YUM_DIR}/postrm DESTINATION .)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/cdnalizer.conf DESTINATION /etc/httpd/conf.modules.d/)

configure_file(${YUM_DIR}/postinst postinst)
SET(CPACK_RPM_POST_INSTALL_SCRIPT_FILE ${CMAKE_CURRENT_BINARY_DIR}/postinst )
SET(CPACK_RPM_POST_UNINSTALL_SCRIPT_FILE ${CMAKE_CURRENT_BINARY_DIR}/postrm )
