###########################################################
# Initializes postgres for cdash
############################################################
FROM matiu/ubuntu:trusty
MAINTAINER Matthew Sherborne <matt.sherborne@rackspace.com>

# Run it at the beginning so we can share some cache with other packages
RUN apt-get update
RUN DEBIAN_FRONTEND=noninteractive apt-get -y upgrade
RUN DEBIAN_FRONTEND=noninteractive apt-get install -y postgresql-client unzip
ENV PGPASSWORD docker
ADD setup.sql setup.sql
ADD http://www.cdash.org/download/CDash-2.0.2.zip CDash-2.0.2.zip
RUN unzip /downloads/CDash*.zip
ADD run.sh run.sh
# You need to docker --link something:db when running so we can get the DB_PORT_5432_TCP_ADDR env variable
ENTRYPOINT /run.sh
