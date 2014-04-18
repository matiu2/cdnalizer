############################################################
# Initializes postgres for cdash
############################################################
FROM matiu/ubuntu:trusty
MAINTAINER Matthew Sherborne <matt.sherborne@rackspace.com>

# Run it at the beginning so we can share some cache with other packages
RUN apt-get update
RUN apt-get install postgresql-client
ENV PGPASSWORD=docker
ADD setup.sql setup.sql
ENTRYPOINT ["/usr/bin/psql"]
CMD ["setup.sql"]
# You need to docker --link something:db when running so we can get the DB_PORT_5432_TCP_ADDR env variable
ENTRYPOINT ["/usr/bin/psql", "-U docker", "-H $DB_PORT_5432_TCP_ADDR", "setup.sql"]
