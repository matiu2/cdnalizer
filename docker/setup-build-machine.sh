#!/bin/sh
apt-get update
apt-get install docker.io
alias d=docker.io
# Infrastructure
d build -t postgres postgres
d build -t pg-init postgres-init
d build -t cdash apache-cdash
# Builders
d build -t build-centos build-centos
d build -t build-ubuntu-precise build-ubuntu-precise
# Setup
d run -d -name=postgres postgres
d run -name=postgres-init --link=postgres:db postgres-init
d run -p 80:80 -d -name=cdash --link=postgres:db cdash
# TODO: git pulling machine with shared volumes
