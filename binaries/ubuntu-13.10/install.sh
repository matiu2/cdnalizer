#!/bin/sh
wget https://raw.github.com/matiu2/cdnalizer/master/binaries/ubuntu-13.10/mod_cdnalizer.so -o /usr/lib/apache2/modules/mod_cdnalizer.so
wget https://raw.github.com/matiu2/cdnalizer/master/binaries/ubuntu-13.10/mod_cdnalizer.load -o /etc/apache2/mods-available/mod_cdnalizer.load
a2enmod mod_cdnalizer
