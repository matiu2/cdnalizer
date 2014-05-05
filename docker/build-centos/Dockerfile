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
WORKDIR /cdnalizer/build
RUN cmake -DCMAKE_HOST_SYSTEM_NAME=centos -DCMAKE_INSTALL_PREFIX=/usr -DCMAKE_BUILD_TYPE=Release -DPACKAGE_TO_BUILD=DEB -DCMAKE_CXX_FLAGS_RELEASE=-O6\ -DNDEBUG .. 
RUN cmake .. # For some reason running it a second time makes it find the Apache libs
RUN make
RUN make package 
WORKDIR /
ADD ../shared/build-old.sh build.sh
CMD /build.sh