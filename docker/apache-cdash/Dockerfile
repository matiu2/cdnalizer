############################################################
# Dockerfile to run cdash
############################################################
FROM ubuntu:latest
MAINTAINER Matthew Sherborne <matt.sherborne@rackspace.com>
RUN apt-get update
RUN DEBIAN_FRONTEND=noninteractive apt-get install -y apache2 wget unzip php5-gd php5-xsl php5-curl
RUN mkdir /downloads && cd /downloads && wget http://www.cdash.org/download/CDash-2.0.2.zip
RUN cd /var/www && rm index.html && unzip /downloads/CDash*.zip && mv CDash*/* . && rmdir CDash*
ADD apache-cdash.conf /etc/apache2/sites-available/cdash
RUN a2dissite default
RUN a2ensite cdash
EXPOSE 80
ENV APACHE_RUN_USER www-data
ENV APACHE_RUN_GROUP www-data
ENV APACHE_LOG_DIR /var/log/apache2
ENTRYPOINT ["/usr/sbin/apache2"]
CMD ["-D", "FOREGROUND"]
