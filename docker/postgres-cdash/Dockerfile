################################################################################
## A postgres install set up for cdash
################################################################################
FROM matiu/postgres
MAINTAINER Matthew Sherborne <matt.sherborne@rackspace.com>

USER root
RUN DEBIAN_FRONTEND=noninteractive apt-get install -y unzip

# Get all the files we need
ADD http://www.cdash.org/download/CDash-2.0.2.zip CDash-2.0.2.zip
RUN unzip /CDash-2.0.2.zip

# Add the cdash user
# Setup the DBs as cdash
USER postgres
    
RUN service postgresql start &&\
    psql --command "CREATE USER cdash WITH PASSWORD 'cdash';" &&\
    createdb -O cdash cdash &&\
    psql --command "GRANT ALL on DATABASE cdash TO cdash;" &&\
    service postgresql stop

ENV PGPASSWORD cdash

RUN service postgresql start &&\
    cat /CDash-2-0-2/sql/pgsql/cdash.sql /CDash-2-0-2/sql/pgsql/cdash.ext.sql | /usr/bin/psql -U cdash -h 127.0.0.1 cdash &&\
    service postgresql stop

# Expose the PostgreSQL port
EXPOSE 5432

# Run postgres on boot (comes from matiu/postgres)
CMD ["/usr/lib/postgresql/9.3/bin/postgres", "-D", "/var/lib/postgresql/9.3/main", "-c", "config_file=/etc/postgresql/9.3/main/postgresql.conf"]

# Add VOLUMEs to allow backup of config, logs and databases .. needs to be run at the end, as we need to fill them up in the build before exporting them
VOLUME  ["/etc/postgresql", "/var/log/postgresql", "/var/lib/postgresql"]
