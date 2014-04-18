############################################################
# Dockerfile to run cdash
############################################################
FROM matiu/ubuntu:trusty
MAINTAINER Matthew Sherborne <matt.sherborne@rackspace.com>
RUN apt-get update
RUN DEBIAN_FRONTEND=noninteractive apt-get install -y apache2 wget unzip php5-gd php5-xsl php5-curl libapache2-mod-php5
RUN mkdir /downloads && cd /downloads && wget http://www.cdash.org/download/CDash-2.0.2.zip
RUN cd /var/www && unzip /downloads/CDash*.zip && mv CDash*/* . && rmdir CDash*
ADD apache-cdash.conf /etc/apache2/sites-available/cdash.conf
ADD config.local.php /var/www/cdash/config.local.php
RUN a2dissite 000-default
RUN a2ensite cdash
EXPOSE 80
ENV APACHE_RUN_USER www-data
ENV APACHE_RUN_GROUP www-data
ENV APACHE_LOG_DIR /var/log/apache2
VOLUME ["/var/log/apache2"]
ENTRYPOINT ["/usr/sbin/apache2"]
CMD ["-D", "FOREGROUND"]
