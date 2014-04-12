set(CPACK_GENERATOR DEB FORCE)
configure_file(cdnalizer.load cdnalizer.load)
file(COPY cdnalizer.conf DESTINATION .)
install(FILES cdnalizer.load cdnalizer.conf DESTINATION /etc/apache2/mods-available/)
