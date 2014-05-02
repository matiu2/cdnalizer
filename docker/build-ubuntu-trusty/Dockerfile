############################################################
# Dockerfile to build cdnalizer for release on ubuntu trusty
############################################################
FROM matiu/ubuntu:trusty
MAINTAINER Matthew Sherborne <matt.sherborne@rackspace.com>
RUN apt-get update
RUN apt-get install -y build-essential cmake apache2-dev g++ git
# cdnalizer
RUN git clone https://github.com/matiu2/cdnalizer.git
RUN mkdir cdnalizer/build
WORKDIR /cdnalizer/build
RUN cmake -DCMAKE_HOST_SYSTEM_NAME=fedora -DCMAKE_INSTALL_PREFIX=/usr -DCMAKE_BUILD_TYPE=Release -DPACKAGE_TO_BUILD=DEB -DCMAKE_CXX_FLAGS_RELEASE=-O6\ -DNDEBUG .. 
RUN make
RUN make package 
RUN ctest -D Continuous
WORKDIR /
ADD ../shared/build.sh build.sh
CMD /build.sh
