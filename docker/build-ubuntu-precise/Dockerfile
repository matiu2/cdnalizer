############################################################
# Dockerfile to build cdnalizer for release on centos
############################################################
FROM ubuntu:precise
MAINTAINER Matthew Sherborne <matt.sherborne@rackspace.com>
RUN apt-get update
RUN apt-get install -y build-essential cmake apache2-dev g++ git
# cdnalizer
RUN git clone https://github.com/matiu2/cdnalizer.git -b old
RUN mkdir cdnalizer/build
# I don't know why cmake needs to be run twice .. it just does
RUN cd /cdnalizer/build && cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS_RELEASE=-O6\ -DNDEBUG .. && cmake .. && make && make package
