############################################################
# Dockerfile to run cdash
############################################################
FROM matiu/ubuntu:trusty
MAINTAINER Matthew Sherborne <matt.sherborne@rackspace.com>
RUN apt-get update
RUN DEBIAN_FRONTEND=noninteractive apt-get -y upgrade
RUN DEBIAN_FRONTEND=noninteractive apt-get install -y apache2 wget unzip php5-gd php5-xsl php5-curl libapache2-mod-php5 php-pear php5-dev php5-pgsql
RUN pecl install pdo pdo_pgsql
ADD http://www.cdash.org/download/CDash-2.0.2.zip CDash-2.0.2.zip
RUN mkdir /downloads && cd /downloads && wget http://www.cdash.org/download/CDash-2.0.2.zip
RUN cd /var/www && unzip /downloads/CDash*.zip && mv CDash*/* . && rmdir CDash*
ADD apache-cdash.conf /etc/apache2/sites-available/cdash.conf
ADD config.local.php /var/www/cdash/config.local.php
RUN chown -R www-data:www-data /var/www
RUN a2dissite 000-default
RUN a2ensite cdash
ADD run.sh run.sh
EXPOSE 80
VOLUME ["/var/log/apache2"]
CMD "/run.sh"
