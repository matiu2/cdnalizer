############################################################
# Dockerfile to build cdnalizer for release on centos
############################################################
FROM goldmann/f20 # Need goldmann's repo to install httpd-devel. See: https://bugzilla.redhat.com/show_bug.cgi?id=1012952
MAINTAINER Matthew Sherborne <matt.sherborne@rackspace.com>
RUN yum install --nogpgcheck -y git make gcc gcc-c++ httpd-devel tar wget cmake
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
