FROM fedora:38
MAINTAINER Ben Boeckel <ben.boeckel@kitware.com>

# Install build dependencies for packages.
COPY install_deps.sh /root/install_deps.sh
RUN sh /root/install_deps.sh

COPY install_gcc.sh /root/install_gcc.sh
RUN sh /root/install_gcc.sh
