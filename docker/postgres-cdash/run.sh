#!/bin/sh
cat setup.sql | /usr/bin/psql -U docker -h $DB_PORT_5432_TCP_ADDR
cd /CDash/sql/pgsql/
export PGPASSWORD=cdash
cat cdash.sql | /usr/bin/psql -U cdash -h $DB_PORT_5432_TCP_ADDR
cat cdash.ext.sql | /usr/bin/psql -U cdash -h $DB_PORT_5432_TCP_ADDR
