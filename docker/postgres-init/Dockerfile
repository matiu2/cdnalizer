############################################################
# Initializes postgres for cdash
############################################################
FROM matiu/ubuntu:trusty
MAINTAINER Matthew Sherborne <matt.sherborne@rackspace.com>

# Run it at the beginning so we can share some cache with other packages
RUN apt-get update
RUN apt-get install psql
ADD setup.sql
ENTRYPOINT ["/usr/bin/psql", setup.sql]
