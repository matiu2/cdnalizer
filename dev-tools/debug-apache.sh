#!/bin/sh
cd apache
cgdb --args /usr/sbin/apache2 -d . -X
