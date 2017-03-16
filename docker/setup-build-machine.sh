#!/bin/sh
#apt-get update
#apt-get install docker.io
alias d=docker
# Infrastructure
docker build -t matiu/postgres postgres
docker build -t cdnalizer/postgres-cdash postgres-cdash
docker build -t cdnalizer/cdash apache-cdash
# Builders
docker build -t cdnalizer/build-centos build-centos
docker build -t cdnalizer/build-fedora build-fedora
docker build -t cdnalizer/build-ubuntu-precise build-ubuntu-precise
docker build -t cdnalizer/build-ubuntu-trusty build-ubuntu-trusty
# Setup
docker run -d --name=postgres cdnalizer/postgres-cdash
docker run -p 80:80 -d --name=cdash --link=postgres:db cdnalizer/cdash
# TODO: git pulling machine with shared volumes
