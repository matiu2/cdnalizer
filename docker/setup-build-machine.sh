#!/bin/sh
#apt-get update
#apt-get install docker.io
alias d=docker
# Infrastructure
docker build --name matiu/postgres postgres
docker build --name cdnalizer/postgres-cdash postgres-cdash
docker build --name cdnalizer/cdash apache-cdash
# Builders
docker build --name cdnalizer/build-centos build-centos
docker build --name cdnalizer/build-fedora build-fedora
docker build --name cdnalizer/build-ubuntu-precise build-ubuntu-precise
docker build --namet cdnalizer/build-ubuntu-trusty build-ubuntu-trusty
# Setup
docker run -d --name=postgres cdnalizer/postgres-cdash
docker run -p 80:80 -d --name=cdash --link=postgres:db cdnalizer/cdash
# TODO: git pulling machine with shared volumes
