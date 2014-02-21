#!/bin/sh
curl https://raw.github.com/matiu2/cdnalizer/master/binaries/ubuntu-13.10/mod_cdnalizer.so > /usr/lib/apache2/modules/mod_cdnalizer.so
curl https://raw.github.com/matiu2/cdnalizer/master/binaries/ubuntu-13.10/mod_cdnalizer.load > /etc/apache2/mods-available/mod_cdnalizer.load
a2enmod mod_cdnalizer
