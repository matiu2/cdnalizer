#!/bin/sh
apt-get update
apt-get install docker.io
alias d=docker.io
# Infrastructure
d build -t ipostgres postgres
d build -t ipg-init postgres-init
d build -t icdash apache-cdash
# Builders
d build -t build-centos build-centos
d build -t build-ubuntu-precise build-ubuntu-precise
# Setup
d run -d --name=postgres ipostgres
d run --name=postgres-init --link=postgres:db ipostgres-init
d run -p 80:80 -d --name=cdash --link=postgres:db icdash
# TODO: git pulling machine with shared volumes
