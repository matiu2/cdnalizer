#!/bin/sh
cd /etc/httpd/conf.d/
wget https://raw.githubusercontent.com/matiu2/cdnalizer/old/binaries/centos-6/cdnalizer.conf
cd /etc/httpd/modules/
wget https://raw.githubusercontent.com/matiu2/cdnalizer/old/binaries/centos-6/mod_cdnalizer.so

echo Restart apache to finalize installation:
echo httpd -S
echo service httpd restart

echo

echo To use, in a <VirtualHost> or the site root's .htaccess enter (for example):
echo CDN_URL /uploads/ http://myCDN.ukraine.hippo/uploads/
echo
echo Then restart Apache again
