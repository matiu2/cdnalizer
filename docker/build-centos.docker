############################################################
# Dockerfile to build cdnalizer for release on centos
############################################################
FROM centos
MAINTAINER Matthew Sherborne <matt.sherborne@rackspace.com>
RUN yum install -y git make gcc gcc-c++ httpd-devel tar wget
# CMAKE first
RUN wget http://www.cmake.org/files/v2.8/cmake-2.8.12.2.tar.gz
RUN tar zxf cmake-2.8.12.2.tar.gz 
RUN cd /cmake* && ./bootstrap && make && make install
# cdnalizer
RUN git clone https://github.com/matiu2/cdnalizer.git -b old
RUN mkdir cdnalizer/build
# I don't know why cmake needs to be run twice .. it just does
RUN cd /cdnalizer/build && cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS_RELEASE=-O6\ -DNDEBUG .. && cmake .. && make && make package
