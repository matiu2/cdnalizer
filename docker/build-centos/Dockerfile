############################################################
# Dockerfile to build cdnalizer for release on centos
############################################################
FROM centos
MAINTAINER Matthew Sherborne <matt.sherborne@rackspace.com>
RUN yum update -y
RUN yum install -y git make gcc gcc-c++ httpd-devel tar wget cmake rpm-build python-setuptools
# cdnalizer
RUN git clone https://github.com/matiu2/cdnalizer.git
RUN mkdir cdnalizer/build
WORKDIR /cdnalizer/build
RUN cmake -DCMAKE_HOST_SYSTEM_NAME=centos -DCMAKE_INSTALL_PREFIX=/usr -DCMAKE_BUILD_TYPE=Release -DPACKAGE_TO_BUILD=RPM -DCMAKE_CXX_FLAGS_RELEASE=-O6\ -DNDEBUG .. 
RUN cmake .. # For some reason running it a second time makes it find the Apache libs
RUN make
RUN make package 
WORKDIR /
ADD build.sh build.sh
RUN /usr/bin/easy_install pip 
RUN /usr/bin/pip install awscli
CMD /build.sh && aws s3 cp cdnalizer/build/packages/*rpm s3://cdnalizer-packages/centos/
