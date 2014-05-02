#!/bin/sh
#apt-get update
#apt-get install docker.io
alias d=docker.io
# Infrastructure
d build -t matiu/postgres postgres
d build -t cdnalizer/postgres-cdash postgres-cdash
d build -t cdnalizer/cdash apache-cdash
# Builders
d build -t cdnalizer/build-centos build-centos
d build -t cdnalizer/build-fedora build-fedora
d build -t cdnalizer/build-ubuntu-precise build-ubuntu-precise
d build -t cdnalizer/build-ubuntu-trusty build-ubuntu-trusty
# Setup
d run -d --name=postgres cdnalizer/postgres-cdash
d run -p 80:80 -d --name=cdash --link=postgres:db cdnalizer/cdash
# TODO: git pulling machine with shared volumes
